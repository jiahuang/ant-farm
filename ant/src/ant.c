

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
void UUID_init(void); // creates or loads UUID as appropriate
void create_uuid(void); // creates a uuid
void memfill(char* location, int size); // helper, don't use it.
void reset_uuid(void); // resets the uuid (for debugging)
int UUID_already_created(void); // boolean to check for start up code
void write_start_code(void); // write the start up code
void load_UUID(void); // load the uuid from eeprom
void clear_UUID_from_EEPROM(void); // clears uuid from eeprom
void clear_START_CODE_from_EEPROM(void); // clears start code from uuid
void store_UUID_to_EEPROM(void); // writes the uuid to eeprom
void EEPROM_write(unsigned int ucAddress, uint8_t ucData); // general function for writing a byte to eeprom
uint8_t EEPROM_read(unsigned int ucAddress); // general function for reading a byte from eeprom

uint8_t random_int_in_range(uint8_t low_range, uint8_t high_range) ;

// TX address. RX address needs to be the same as this
// goes from least significant to most significant
uint8_t data_pipe[5] = {0xE7, 0xE7, 0xE7, 0xE7, 0xE7};
// first 4 bytes are the ID, last byte is button press
uint8_t data_array[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

uint8_t data_received[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
// The UUID itself. It can be loaded with load_UUID()
char UUID[UUID_SIZE];

#include "nRF24L01-tx.c"
#include "nRF24L01-rx.c"

//======================
ISR(PCINT1_vect)
{
  //This vector is only here to wake unit up from sleep mode
}

int main (void)
{
  uint8_t incoming;

  uint16_t button_presses = 0;    
  
  ioinit();
  clear_UUID_from_EEPROM();
  clear_START_CODE_from_EEPROM();
  UUID_init();
    
  while(1)
  {

    incoming = tx_send_byte(0xFF); //Get status register
    if (incoming & 0x40)
    {
      //We have data!
      receive_data(data_received);
      PORTA = PORTA ^ (1<<LED ); // flash an LED

      // get ready to pong back a message
      configure_transmitter(data_pipe);
      // easy boy
      delay_ms(10);

      data_array[0] = data_received[0];
      data_array[1] = data_received[1];
      data_array[2] = data_received[2];
      data_array[3] = data_received[3];
      data_array[4] = UUID[0];
      data_array[5] = UUID[1];
      data_array[6] = UUID[2];
      data_array[7] = UUID[3];

      // Send over theping
      transmit_data(data_array);

      // // wait for transmitting to be done
      delay_ms(20);
      // transmit = 1;

      // go back to receiving
      configure_receiver(data_pipe);
    }


    // Delay a rando amount of time (the UUID)
    delay_ms(UUID[0]);
    // configure_receiver(data_pipe);
    // tx_send_command(0x20, 0x00); //Power down RF

    // cbi(PORTB, TX_CE); //Go into standby mode
    // sbi(PORTB, TX_CSN); //Deselect chip
    
    ACSR = (1<<ACD); //Turn off Analog Comparator - this removes about 1uA
    PRR = 0x0F; //Reduce all power right before sleep
    asm volatile ("sleep");
    //Sleep until a button wakes us up on interrupt
  }
  
  return(0);
}

void ioinit (void)
{
  //1 = Output, 0 = Input
  DDRA = 0xFF & ~(1<<TX_MISO | 1 << LIKE_BUTTON);//| 1<<BUTTON0 | 1<<BUTTON1 | 1<<BUTTON2 | 1<<BUTTON3 | 1<<BUTTON4);
  DDRB = 0b00000110; //(CE on PB1) (CS on PB2)
  // init_nRF_pins();

  //Enable pull-up resistors (page 74)
  // PORTA = 0b10001110; //Pulling up a pin that is grounded will cause 90uA current leak
  PORTA = 0b10001101;
  cbi(PORTB, TX_CE); //Stand by mode
  
  //Init Timer0 for delay_us
  TCCR0B = (1<<CS00); //Set Prescaler to No Prescaling (assume we are running at internal 1MHz). CS00=1 

  // configure_transmitter(data_pipe);

  configure_receiver(data_pipe);

  GIFR = (1 << PCIF1); // Enable logic interrupt on PCINT 11:8
  GIMSK = (1 << PCIE1); // Enable pin change interrupt request on PCINT 11:8
  PCMSK1 = (1 << NRF_IRQ); 
  // GIFR = (1<<PCIF0); //Enable the Pin Change interrupts to monitor button presses
  // GIMSK = (1<<PCIE0); //Enable Pin Change Interrupt Request
  // PCMSK0 = (1<<BUTTON0)|(1<<BUTTON1)|(1<<BUTTON2)|(1<<BUTTON3)|(1<<BUTTON4);
  MCUCR = (1<<SM1)|(1<<SE); //Setup Power-down mode and enable sleep
  
  sei(); //Enable interrupts
}

void UUID_init(void) {

  // Check for a start code to see if we have created a UUID yet
  if (!UUID_already_created()) {

    // If not, Create it (also loads it into UUID)
    create_uuid();

    // Save it to EEPROM
    store_UUID_to_EEPROM();

    // Write start code since we've written the id now
    write_start_code();
  } else {
    // We've already created the UUID so just load it
    load_UUID();
  }
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


void create_uuid(void) {
  // Generate a Version 4 UUID according to RFC4122
  memfill((char*)UUID, 4);
}

void reset_uuid(void) {
  clear_UUID_from_EEPROM();
  clear_START_CODE_from_EEPROM();
}
void write_start_code() {

  for (int i = 0; i < sizeof(START_UP_STRING_CODE); i++) {
    EEPROM_write(START_UP_STRING_CODE_ADDRESS + i, START_UP_STRING_CODE[i]);
  }
}
int UUID_already_created() {

  for (int i = 0; i < sizeof(START_UP_STRING_CODE); i++) {
      if (EEPROM_read(START_UP_STRING_CODE_ADDRESS + i) != START_UP_STRING_CODE[i]) {
        return 0;
      }
  }
  return 1;
}

void load_UUID(void) {
  for (int i = 0; i < UUID_SIZE; i++) {
    UUID[i] = EEPROM_read(START_UUID_ADDRESS + i);
  }
}

void clear_UUID_from_EEPROM(void) {
  for (int i = 0; i < UUID_SIZE; i++) {
    EEPROM_write(START_UUID_ADDRESS + i, '\0');
  }
}

void clear_START_CODE_from_EEPROM(void) {
  for (int i = 0; i < sizeof(START_UP_STRING_CODE); i++) {
    EEPROM_write(START_UP_STRING_CODE_ADDRESS + i, '\0');
  }
}

void store_UUID_to_EEPROM(void) {
  for (int i = 0; i < UUID_SIZE; i++) {
    EEPROM_write(START_UUID_ADDRESS + i, UUID[i]);
  }
}
int randomBitRaw(void) {
  uint8_t copyAdmux, copyAdcsra, copyAdcsrb, copyPorta, copyDdra;
  uint8_t bit;
  uint8_t dummy;

  // Store all the registers we'll be playing with
  copyAdmux = ADMUX;
  copyAdcsra = ADCSRA;
  copyAdcsrb = ADCSRB;
  copyPorta = PORTA;
  copyDdra = DDRA;
  
  // Perform a conversion on Analog0, using the Vcc reference
  ADMUX =
(0 << ADLAR)| //10bit precision
(1 << MUX1)| //use PB4 as input pin
(0 << REFS0)| //set refs0 and refs1 to 0 to use Vcc as Vref
(0 << REFS1);
  
#if F_CPU > 16000000
  // ADC is enabled, divide by 32 prescaler
  ADCSRA = _BV(ADEN) | _BV(ADPS2) | _BV(ADPS0);
#elif F_CPU > 8000000
  // ADC is enabled, divide by 16 prescaler
  ADCSRA = _BV(ADEN) | _BV(ADPS2);
#else
  // ADC is enabled, divide by 8 prescaler
  ADCSRA = _BV(ADEN) | _BV(ADPS1) | _BV(ADPS0);
#endif

  
  // Autotriggering disabled
  ADCSRB = 0;

  //Turn on the third pin of PORTA
  PORTA |= _BV(ANALOG_VOLTAGE);


  PORTA |= _BV(1); //xxxx1x

  // Immediately start a sample conversion on Analog0
  ADCSRA |= _BV(ADSC);

  // Wait for conversion to complete
  while (ADCSRA & _BV(ADSC));

  // Xor least significant bits together
  bit = ADCL;

  // We're ignoring the high bits, but we have to read them before the next conversion
  dummy = ADCH;

  // Restore register states
  ADMUX = copyAdmux;
  ADCSRA = copyAdcsra;
  ADCSRB = copyAdcsrb;
  PORTA = copyPorta;
  DDRA = copyDdra;

  return (bit & 1);

}

int randomBitRaw2(void) {
  // Software whiten bits using Von Neumann algorithm
  //
  // von Neumann, John (1951). "Various techniques used in connection
  // with random digits". National Bureau of Standards Applied Math Series
  // 12:36.
  //
  for(;;) {
    int a = randomBitRaw() | (randomBitRaw()<<1);
    if (a==1) return 0; // 1 to 0 transition: log a zero bit
    if (a==2) return 1; // 0 to 1 transition: log a one bit
    // For other cases, try again.
  }
}

int randomBit(void) {
  // Software whiten bits using Von Neumann algorithm
  //
  // von Neumann, John (1951). "Various techniques used in connection
  // with random digits". National Bureau of Standards Applied Math Series
  // 12:36.
  //
  for(;;) {
    int a = randomBitRaw2() | (randomBitRaw2()<<1);
    if (a==1) return 0; // 1 to 0 transition: log a zero bit
    if (a==2) return 1; // 0 to 1 transition: log a one bit
    // For other cases, try again.
  }
}

char randomByte(void) {
  char result;
  uint8_t i;
  result = 0;
  for (i=8; i--;) result += result + randomBit();
  return result;
}

void memfill(char* location, int size) {
  for (;size--;) *location++ = randomByte();
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

uint8_t random_int_in_range(uint8_t low_range, uint8_t high_range) {
  return low_range + ((high_range - low_range) & randomByte());
}
