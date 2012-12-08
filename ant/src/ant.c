

// testing with code from sparkfun
/*
    2-8-2008
    Copyright Spark Fun Electronics© 2008
    Nathan Seidle
    nathan at sparkfun.com
    
  Key FOB transmitter based on the nRF24L01
  
  2-4uA average current
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include "id.c"

#define sbi(var, mask)   ((var) |= (uint8_t)(1 << mask))
#define cbi(var, mask)   ((var) &= (uint8_t)~(1 << mask))

/* EEPROM UUID
// START_UP_STRING_CODE is a code that we can check EEPROM for to see if we have 
// already saved a UID in EEPROM. We save this code to memory just after
// receiving the UID. The reason for it is 
// because our EEPROM isn't guaranteed to be uninitualized. Therefore,
// we can't just check a certain point in EEPROM memory for a UID
// because it might be trash. Instead, we will check the EEPROM for this
// code at a certain point in memory that basically tells us that 
// we have already saved a UID. 
*/
#define START_UP_STRING_CODE "STORED"
// Where we are saving the code in EEPROM
#define START_UP_STRING_CODE_ADDRESS 0
// The first address that the UUID is saved at
#define START_UUID_ADDRESS (sizeof(START_UP_STRING_CODE) + 1)
// The size, in bytes, of the UUID
#define UUID_SIZE  4
// The pin register associated with the nRF24 IRQ pin
#define NRF_IRQ 0

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

#include "nRF24L01-tx.c"
#include "nRF24L01-rx.c"

//======================
ISR(PCINT1_vect)
{
  // wake up from sleep mode
  // 1) read payload through SPI
  while( ping_pong() & 0x40) {
    // 2) clear RX_DR IRQ
    tx_send_command(0x27, (1 << RX_DR) | (1 << TX_DS) | (1 << MAX_RT));
    // 3) read FIFO_STATUS to check if there are more payloads available in RX FIFO
    // 4) if there are more data in RX FIFO, repeat from step 1)
  }
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

      PORTA = PORTA ^ (1<<LED ); // flash an LED

      // get ready to pong back a message
      configure_transmitter(data_pipe);

      data_array[0] = data_received[0];
      data_array[1] = data_received[1];
      data_array[2] = data_received[2];
      data_array[3] = data_received[3];
      data_array[4] = UUID[0];
      data_array[5] = UUID[1];
      data_array[6] = UUID[2];
      data_array[7] = UUID[3];

      delay_ms((UUID[0] * 1000) & 255);
      
      // Send over theping
      transmit_data(data_array);

      // // wait for transmitting to be done
      delay_ms(10);
      // go back to receiving
      configure_receiver(data_pipe);
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
    ping_pong();
       PORTA = PORTA ^(1 << 2);
   sleep_cpu(); //Sleep until a ping wakes us up on interrupt
   // sleep_disable();  
      PORTA = PORTA ^(1 << 3);
  }
  
  return(0);
}

void ioinit (void)
{
  //1 = Output, 0 = Input
  DDRA = 0xFF & ~(1<<TX_MISO);//| 1<<BUTTON0 | 1<<BUTTON1 | 1<<BUTTON2 | 1<<BUTTON3 | 1<<BUTTON4);
  DDRB = 0b00000110; //(CE on PB1) (CS on PB2)

  PORTA = 0b10011110;

  cbi(PORTB, TX_CE); //Stand by mode
  
  //Init Timer0 for delay_us
  TCCR0B = (1<<CS00); //Set Prescaler to No Prescaling (assume we are running at internal 1MHz). CS00=1 

  // configure_transmitter(data_pipe);

  configure_receiver(data_pipe);

  GIFR = (1 << PCIF1); // Enable logic interrupt on PCINT 11:8
  GIMSK = (1 << PCIE1); // Enable pin change interrupt request on PCINT 11:8
  PCMSK1 = (1 << NRF_IRQ); 
  MCUCR = (1<<SM1)|(1<<SE); //Setup Power-down mode and enable sleep
  
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
