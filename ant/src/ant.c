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
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
  

#define sbi(var, mask)   ((var) |= (uint8_t)(1 << mask))
#define cbi(var, mask)   ((var) &= (uint8_t)~(1 << mask))

//Define functions
//======================
void ioinit(void);      //Initializes IO
void delay_ms(uint16_t x); //General purpose delay
void delay_us(uint8_t x);

uint8_t data_array[5];

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
  // PORTA = 0b11111111; 
  transmit_data(); //Send one packet when we turn on

  while(1)
  {
    
    // if( (PINA & 0x8F) != 0x8F )
    // {
    //   button_presses++;
      
    //   data_array[0] = PINA & 0x0F;
    //   data_array[0] |= (PINA & 0x80) >> 3;
      
    //   data_array[1] = button_presses >> 8;
    //   data_array[2] = button_presses & 0xFF;

    //   data_array[3] = 0;

    //   transmit_data();
    // }
    
    // tx_send_command(0x20, 0x00); //Power down RF

    // cbi(PORTB, TX_CE); //Go into standby mode
    // sbi(PORTB, TX_CSN); //Deselect chip
    
    // ACSR = (1<<ACD); //Turn off Analog Comparator - this removes about 1uA
    // PRR = 0x0F; //Reduce all power right before sleep
    // asm volatile ("sleep");
    //Sleep until a button wakes us up on interrupt
    
    // sleep 5 sec then transmit
    delay_ms(200);
     
    transmit_data();
    data_array[0] = 1;
    PORTA = PORTA ^ (1<<BUTTON0 ); // flash an LED
  }
  
    return(0);
}

void ioinit (void)
{
  //1 = Output, 0 = Input
  DDRA = 0xFF & ~(1<<TX_MISO );//| 1<<BUTTON0 | 1<<BUTTON1 | 1<<BUTTON2 | 1<<BUTTON3 | 1<<BUTTON4);
  DDRB = 0b00000110; //(CE on PB1) (CS on PB2)

  //Enable pull-up resistors (page 74)
  PORTA = 0b10001110; //Pulling up a pin that is grounded will cause 90uA current leak
  // PORTA = 0b11111111; 

  cbi(PORTB, TX_CE); //Stand by mode
  
    //Init Timer0 for delay_us
    TCCR0B = (1<<CS00); //Set Prescaler to No Prescaling (assume we are running at internal 1MHz). CS00=1 

  /*
  DDRA = 0xFF;
  DDRB = 0xFF;
  while(1)
  {
    PORTA = 0xFF;
    PORTB = 0xFF;
    delay_ms(3000);

    PORTA = 0x00;
    PORTB = 0x00;
    delay_ms(3000);
  }
  */

  configure_transmitter();

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
