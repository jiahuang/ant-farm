#include <inttypes.h>

void uuid_create(uint8_t* uuidLocation);

int randomBitRaw(void) {
  uint8_t copyAdmux, copyAdcsra, copyAdcsrb, copyPorta, copyDdra;
  uint16_t i;
  uint8_t bit;
  uint8_t dummy;
  
//   // Store all the registers we'll be playing with
//   copyAdmux = ADMUX;
//   copyAdcsra = ADCSRA;
//   copyAdcsrb = ADCSRB;
//   copyPorta = PORTA;
//   copyDdra = DDRA;
  
//   // Perform a conversion on Analog0, using the Vcc reference
//   ADMUX = _BV(REFS0);
  
// #if F_CPU > 16000000
//   // ADC is enabled, divide by 32 prescaler
//   ADCSRA = _BV(ADEN) | _BV(ADPS2) | _BV(ADPS0);
// #elif F_CPU > 8000000
//   // ADC is enabled, divide by 16 prescaler
//   ADCSRA = _BV(ADEN) | _BV(ADPS2);
// #else
//   // ADC is enabled, divide by 8 prescaler
//   ADCSRA = _BV(ADEN) | _BV(ADPS1) | _BV(ADPS0);
// #endif

//   // Autotriggering disabled
//   ADCSRB = 0;

//   // Pull Analog0 to ground
//   PORTA &=~_BV(0);
//   DDRA |= _BV(0);
//   // Release Analog0, apply internal pullup
//   DDRA &= ~_BV(0);
//   PORTA |= _BV(1);
//   // Immediately start a sample conversion on Analog0
//   ADCSRA |= _BV(ADSC);
//   // Wait for conversion to complete
//   while (ADCSRA & _BV(ADSC)) PORTA ^= _BV(0);
//   // Xor least significant bits together
//   bit = ADCL;
//   // We're ignoring the high bits, but we have to read them before the next conversion
//   dummy = ADCH;

//   // Restore register states
//   ADMUX = copyAdmux;
//   ADCSRA = copyAdcsra;
//   ADCSRB = copyAdcsrb;
//   PORTA = copyPorta;
//   DDRA = copyDdra;

//   return bit & 1;
  return 1;
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
  // for(;;) {
  //   int a = randomBitRaw2() | (randomBitRaw2()<<1);
  //   if (a==1) return 0; // 1 to 0 transition: log a zero bit
  //   if (a==2) return 1; // 0 to 1 transition: log a one bit
  //   // For other cases, try again.
  // }
  return 1;
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

void uuid_create(uint8_t* uuidLocation) {
  // Generate a Version 4 UUID according to RFC4122
  memfill((char*)uuidLocation,16);
  // Although the UUID contains 128 bits, only 122 of those are random.
  // The other 6 bits are fixed, to indicate a version number.
  uuidLocation[6] = 0x40 | (0x0F & uuidLocation[6]); 
  uuidLocation[8] = 0x80 | (0x3F & uuidLocation[8]);
}