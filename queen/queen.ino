#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "printf.h"

#define QUEEN_ID 0x01

// listens on channel 10
// Set up nRF24L01 radio on SPI bus plus pins 9 & 10 

RF24 radio(9,10);

const int PAYLOAD_SIZE = 30;

// Open a reading address. This address needs to match the address that the ant is transmitting at
const uint64_t pipes[1] = { 0xE7E7E7E7E7LL };

void setup(void)
{

  Serial.begin(57600);
  printf_begin();
  printf("\n\rQueen\n\r");
  printf("ROLE: Receiver\n\r");

  // Setup and configure rf radio
  radio.begin();
  radio.setChannel(10);
  radio.setPayloadSize(PAYLOAD_SIZE);
  radio.setAutoAck(false);
  radio.setCRCLength(RF24_CRC_8);
//  radio.setRetries(15,15);
//  radio.setPALevel(RF24_PA_MIN);
  
  // optionally, increase the delay between retries & # of retries
//  radio.setRetries(15,15);
  // radio.setPALevel(RF24_PA_MID);
//  radio.setDataRate(RF24_250KBPS);
  // optionally, reduce the payload size.  seems to
  // improve reliability
  // radio.setPayloadSize(8);

  // read at the pipe
//  radio.openReadingPipe(0,pipes[0]);
  radio.openReadingPipe(1,0xE7E7E7E7E7LL);
  radio.openWritingPipe(0xE7E7E7E7E7LL);
  
  radio.startListening();
//  transmit_init();
  Serial.println("Here comes the details");
  radio.printDetails();
  Serial.println("hey all done here");
}

void loop(void)
{
  if ( radio.available() )
  {
    // Dump the payloads until we've gotten everything
    byte received_data[PAYLOAD_SIZE];
    bool done = false;
    while (!done)
    {
      done = radio.read( &received_data, PAYLOAD_SIZE );
      
      // check that it's an ant
//      if ( (received_data[4] | received_data[5] | 
//        received_data[6] | received_data[7]) != 0x0) {
        Serial.print("Got payload: ");
        if (QUEEN_ID <= 0xF)
          Serial.print("0");
        Serial.print(QUEEN_ID, HEX);
        for (int i = 0; i < PAYLOAD_SIZE; i++) {
          if (received_data[i] <= 0xF)
            Serial.print("0");
          Serial.print(received_data[i], HEX); 
        }
        Serial.println();
//      }      
    } 
  }
    
}

