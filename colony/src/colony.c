
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <avr/wdt.h>
#include "colony_id.c"

#define sbi(var, mask)   ((var) |= (uint8_t)(1 << mask))
#define cbi(var, mask)   ((var) &= (uint8_t)~(1 << mask))

// The size, in bytes, of the UUID
#define UUID_SIZE  4
// The pin register associated with the nRF24 IRQ pin
#define NRF_IRQ 0

#define MINUTES_TO_RESET 5

// The receiver frequency. Should match ant receive frequency.
// Frequency = 2400 + RF_CH [MHz]
#define TRANSMIT_FREQ 0x03 // RF_CH = 3

//Define functions
//======================
void ioinit(void);      //Initializes IO
void delay_ms(uint16_t x); //General purpose delay
void delay_us(uint8_t x);
void ping(void);
// TX address. RX address needs to be the same as this
// goes from least significant to most significant
uint8_t data_pipe[5] = {0xE7, 0xE7, 0xE7, 0xE7, 0xE7};
// first 4 bytes are the ID, last byte is button press
uint8_t data_array[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
// ID of the colony
char UUID[UUID_SIZE] = { ID_1, ID_2, ID_3, ID_4 } ;

// ~8 resets per minute to get total number of resets
// uint32_t interrupts_remaining = 8 * MINUTES_TO_RESET;

#include "nRF24L01-tx.c"
#include "nRF24L01-rx.c"

//===========

ISR(WDT_vect) {
  cli();
  PORTA = PORTA ^ (1 << LED);

  tx_send_command(0x27, (1 << RX_DR) | (1 << TX_DS) | (1 << MAX_RT));

  ping();
  delay_ms(50);
  
  PORTA = PORTA ^ (1 << LED);
  sei();
}

void ping() {
  // stick on colony ID
  data_array[0] = UUID[0];
  data_array[1] = UUID[1];
  data_array[2] = UUID[2];
  data_array[3] = UUID[3];

  // pad where the antID should go with 0s
  data_array[4] = 0x00;
  data_array[5] = 0x00;
  data_array[6] = 0x00;
  data_array[7] = 0x00;
  
  transmit_data(data_array);
  delay_ms(5);
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
   sleep_cpu(); // sleep until we need to ping out
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

  configure_transmitter(data_pipe, TRANSMIT_FREQ);

  GIFR = (1 << PCIF1); // Enable logic interrupt on PCINT 11:8
  GIMSK = (1 << PCIE1); // Enable pin change interrupt request on PCINT 11:8
  PCMSK1 = (1 << NRF_IRQ); 
  
  MCUCR = (1<<SM1)|(1<<SE); //Setup Power-down mode and enable sleep

  //prescaler set to 2 seconds
  WDTCSR |= (1 << WDCE) | (1 << WDP2) | (1 << WDP1) | (1 << WDP0);// |  (1 << WDP0);

  // Set the watchdog to interrupt instead of reset
  WDTCSR |= (1 << WDIE);
  delay_ms(200);
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
