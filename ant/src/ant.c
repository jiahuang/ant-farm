

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

#define UUID_SIZE = 0x05;
// Note that there are only 128 bytes of EEPROM mem
#define UUID_EEPROM_START_ADDR = 0;

//Define functions
//======================
void ioinit(void);      //Initializes IO
void delay_ms(uint16_t x); //General purpose delay
void delay_us(uint8_t x);
void EEPROM_write(unsigned int ucAddress, uint8_t ucData);
uint8_t EEPROM_read(unsigned int ucAddress);


// TX address. RX address needs to be the same as this
// goes from least significant to most significant
uint8_t data_pipe[5] = {0xE7, 0xE7, 0xE7, 0xE7, 0xE7};
// first 4 bytes are the ID, last byte is button press
uint8_t data_array[4] = {ID_1, ID_2, ID_3, 0x00};

#include "nRF24L01.c"
//======================
ISR(PCINT0_vect)
{
  //This vector is only here to wake unit up from sleep mode
}

int main (void)
{
  uint16_t button_presses = 0;
  
  ioinit();

  transmit_data(data_array); //Send one packet when we turn on
  data_array[3] = 0x00; // reset

  // Write 2 at addr 10
  EEPROM_write(10, 02);
  // Read from addr 10
  data_array[0] = EEPROM_read(10);
  // Send byte from addr 10
  transmit_data(data_array);
  // Turn on a light
  PORTA = PORTA | (1<<LED );
    
  while(1)
  {
    
    if( (PINA & 0x8D) != 0x8D )
    {
      // turn on the LED
      PORTA = PORTA | (1<<LED );
      data_array[3] = 0x01;
    }
    
    transmit_data(data_array);

    delay_ms(200);
    // tx_send_command(0x20, 0x00); //Power down RF

    // cbi(PORTB, TX_CE); //Go into standby mode
    // sbi(PORTB, TX_CSN); //Deselect chip
    
    // ACSR = (1<<ACD); //Turn off Analog Comparator - this removes about 1uA
    // PRR = 0x0F; //Reduce all power right before sleep
    // asm volatile ("sleep");
    //Sleep until a button wakes us up on interrupt

    
    
    // delay_ms(3000);
    PORTA = PORTA & ~(1<<LED );
  }
  
  return(0);
}

void ioinit (void)
{
  //1 = Output, 0 = Input
  DDRA = 0xFF & ~(1<<TX_MISO | 1 << LIKE_BUTTON);//| 1<<BUTTON0 | 1<<BUTTON1 | 1<<BUTTON2 | 1<<BUTTON3 | 1<<BUTTON4);
  DDRB = 0b00000110; //(CE on PB1) (CS on PB2)

  //Enable pull-up resistors (page 74)
  // PORTA = 0b10001110; //Pulling up a pin that is grounded will cause 90uA current leak
  PORTA = 0b10001101;
  cbi(PORTB, TX_CE); //Stand by mode
  
  //Init Timer0 for delay_us
  TCCR0B = (1<<CS00); //Set Prescaler to No Prescaling (assume we are running at internal 1MHz). CS00=1 

  configure_transmitter(data_pipe);

  GIFR = (1<<PCIF0); //Enable the Pin Change interrupts to monitor button presses
  GIMSK = (1<<PCIE0); //Enable Pin Change Interrupt Request
  PCMSK0 = (1<<BUTTON0)|(1<<BUTTON1)|(1<<BUTTON2)|(1<<BUTTON3)|(1<<BUTTON4);
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


void EEPROM_write(unsigned int ucAddress, uint8_t ucData)
{
  /* Wait for completion of previous write */
  while(EECR & (1<<EEPE));
  /* Set Programming mode */
  EECR = (0<<EEPM1)|(0<<EEPM0);
  /* Set up address and data registers */
  EEAR = ucAddress;
  EEDR = ucData;
  /* Write logical one to EEMPE */
  EECR |= (1<<EEMPE);
  /* Start eeprom write by setting EEPE */
  EECR |= (1<<EEPE);
}

uint8_t EEPROM_read(unsigned int ucAddress)
{
/* Wait for completion of previous write */
while(EECR & (1<<EEPE))
;
/* Set up address register */
EEAR = ucAddress;
/* Start eeprom read by writing EERE */
EECR |= (1<<EERE);
/* Return data from data register */
return EEDR;
}

