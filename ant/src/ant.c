
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <avr/wdt.h>
#include "id.c"

#define sbi(var, mask)   ((var) |= (uint8_t)(1 << mask))
#define cbi(var, mask)   ((var) &= (uint8_t)~(1 << mask))

#define UUID_SIZE  4
// The pin register associated with the nRF24 IRQ pin
#define NRF_IRQ 0

#define MINUTES_TO_RESET 5

// The receiver frequency. Should match colony transmit frequency.
// Frequency = 2400 + RF_CH [MHz]
#define RECEIVE_FREQ 3 // RF_CH = 3
// The transmit frequency. Should match the queen receive frequency.
#define TRANSMIT_FREQ 10 // RF_CH = 10

//Define functions
//======================
void ioinit(void);      //Initializes IO
void delay_ms(uint16_t x); //General purpose delay
void delay_us(uint8_t x);
uint8_t ping_pong(void);

// TX address. RX address needs to be the same as this
// goes from least significant to most significant
uint8_t data_pipe[5] = {0xE7, 0xE7, 0xE7, 0xE7, 0xE7};
// first 4 bytes are the ID, last byte is button press
uint8_t data_array[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

uint8_t data_received[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
// The UUID itself. It can be loaded with load_UUID()
char UUID[UUID_SIZE] = { ID_1, ID_2, ID_3, ID_4 } ;

// ~8 resets per minute to get total number of resets
uint16_t interrupts_remaining = 8 * MINUTES_TO_RESET;

uint16_t ping_id = 0x00;

#include "nRF24L01-tx.c"
#include "nRF24L01-rx.c"

//======================
ISR(PCINT1_vect)
{
  cli();
  while( ping_pong() & 0x40) { }
  sei();
}

ISR(WDT_vect) {

    cli();
    PORTA = PORTA ^ (1<<2 ); // flash an LED
    tx_send_command(0x27, (1 << RX_DR) | (1 << TX_DS) | (1 << MAX_RT));

    if (--interrupts_remaining <= 0) {

      configure_receiver(data_pipe, RECEIVE_FREQ);

      delay_ms(20);
      
      interrupts_remaining = 8 * MINUTES_TO_RESET;

    }
    sei();
}


uint8_t ping_pong(void) 
{
  uint8_t incoming = 0;
  incoming = tx_send_byte(0xFF); //Get status register
  if (incoming & 0x40)
  {

    //We have data!
    receive_data(data_received, 8);

    // make sure that it's a queen's ping
    if ( (data_received[4] | data_received[5] |
      data_received[6] | data_received[7]) == 0x00 ) {

      // get ready to pong back a message
      configure_transmitter(data_pipe, TRANSMIT_FREQ);

      data_array[0] = data_received[0];
      data_array[1] = data_received[1];
      data_array[2] = UUID[0];
      data_array[3] = UUID[1];
      data_array[4] = UUID[2];
      data_array[5] = UUID[3];
      data_array[6] = ((ping_id >> 8) & 0xFF);
      data_array[7] = (ping_id & 0xFF);

      if (ping_id == 0xFFFF) ping_id = 0;
      else ping_id++;

      // delay some more for randomness
      delay_ms(5);
      delay_ms(MS_DELAY * 5); // space out at at least 5 ms
      // delay_ms((UUID[0] * 1000) & 255);
      
      // Send over the ping
      transmit_data(data_array);
      // // wait for transmitting to be done
      delay_ms(5);

      PORTA = PORTA ^ (1<<LED ); // flash an LED
      delay_ms(2);
      // go back to receiving
      configure_receiver(data_pipe, RECEIVE_FREQ);
      // delay for some fun
      delay_ms(5);
      PORTA = PORTA ^ (1<<LED ); // flash an LED
    }
  }

  // delay_ms(100);
  return incoming;
}

int main (void)
{  
  // Set up all the input and output ports
  ioinit();

  // Turn off the led when it's loaded
  PORTA = PORTA & ~(1 << LED);

  ACSR = (1<<ACD); //Turn off Analog Comparator - this removes about 1uA

  PRR = 0x0B; //Reduce everything except timer0
    
  while(1)
  {
   sleep_cpu(); //Sleep until a ping wakes us up on interrupt
  }
  
  return(0);
}

void ioinit (void)
{
  cli();
  //1 = Output, 0 = Input
  DDRA = 0xFF & ~(1<<TX_MISO);
  DDRB = 0b00000110; //(CE on PB1) (CS on PB2)

  PORTA = 0b10011010;

  /* Clear WDRF in MCUSR */
  MCUSR = 0x00;
  /* Write logical one to WDCE and WDE */ 
  WDTCSR |= (1 << WDCE) | (1 << WDE);
  /* Turn off WDT */
  WDTCSR = 0x00;

  cbi(PORTB, TX_CE); //Stand by mode
  
  //Init Timer0 for delay_us
  TCCR0B = (1<<CS00); //Set Prescaler to No Prescaling (assume we are running at internal 1MHz). CS00=1 

  configure_receiver(data_pipe, RECEIVE_FREQ);

  GIFR = (1 << PCIF1); // Enable logic interrupt on PCINT 11:8
  GIMSK = (1 << PCIE1); // Enable pin change interrupt request on PCINT 11:8
  PCMSK1 = (1 << NRF_IRQ); 
  MCUCR = (1<<SM1)|(1<<SE); //Setup Power-down mode and enable sleep

  //prescaler set to 8 seconds
  WDTCSR |= (1 << WDCE) | (1 << WDP3) |  (1 << WDP0);

  // Set the watchdog to interrupt instead of reset
  WDTCSR |= (1 << WDIE);
  
  sei(); //Enable interrupts
}

//General short delays
void delay_ms(uint16_t x)
{
  for (; x > 0 ; x--)
  {
    delay_us(250);
    delay_us(250);
    delay_us(250);
    delay_us(250);
  }
}

//General short delays
void delay_us(uint8_t x)
{
  TIFR0 = 0x01; //Clear any interrupt flags on Timer2
  
    TCNT0 = 256 - x; //256 - 125 = 131 : Preload timer 2 for x clicks. Should be 1us per click

  while( (TIFR0 & (1<<TOV0)) == 0);
}
