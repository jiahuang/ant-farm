// modified from RF24's getting started example

#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "printf.h"

//
// Hardware configuration
//

// Set up nRF24L01 radio on SPI bus plus pins 9 & 10 

RF24 radio(9,10);

int TRANSMIT_TIME = 30000;//65535;
int MULTIPLE = 5;
int transmit_time_counter = 0;
int multiple_counter = 0;
//
// Topology
//

// Open a reading address. This address needs to match the address that the ant is transmitting at
const uint64_t pipes[1] = { 0xE7E7E7E7E7LL };

void setup(void)
{

  Serial.begin(57600);
  printf_begin();
  printf("\n\rsoldier\n\r");
  printf("ROLE: Receiver\n\r");

  // Setup and configure rf radio
  Serial.println("Hey");
  radio.begin();
  Serial.println("It has begined");
  radio.setChannel(10);
  radio.setPayloadSize(8);
  radio.setAutoAck(false);
  radio.setCRCLength(RF24_CRC_8);
  radio.setRetries(15,15);
  radio.setPALevel(RF24_PA_MIN);
  
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
 
  transmit_init();
  Serial.println("Here comes the details");
  radio.printDetails();
  Serial.println("hey all done here");
}

void receive_init() {
  radio.startListening();
}

void transmit_init() {
  radio.stopListening();
}

void loop(void)
{
  // if there is data ready
  //printf("" + radio.available());
  if (transmit_time_counter >= TRANSMIT_TIME) {
    transmit_init();
//    delay(200);
      byte queen_id[8] = {30,30,30,30, 0, 0, 0, 0};
      boolean okay = transmit(queen_id);
      if (okay) {
       transmit_time_counter = 0;
       receive_init();
//       delay(200);
      } else {
       Serial.println("OOOOHHHH NOOOO"); 
      }
  } else {
    if (multiple_counter >= MULTIPLE) {
      multiple_counter = 0;
      transmit_time_counter++;
    } else {
      multiple_counter++;
    }
    receive_data();
  }
    
}

boolean transmit(byte * data) {
  
  boolean okay = radio.write(data, 8);
  if (okay) {
    Serial.println("Serial transmit okay");
  } else {
   Serial.println("PROLLEM WITH TRANSMIT"); 
  }
  return okay;
}

void receive_data(void) {
 if ( radio.available() )
  {
    // Dump the payloads until we've gotten everything
    byte received_data[8];
    bool done = false;
    while (!done)
    {
      done = radio.read( &received_data, 8 );
      
      // check that it's a queen
      if ( (received_data[4] | received_data[5] | 
        received_data[6] | received_data[7]) != 0x0) {
        Serial.print("Got payload: ");
        for (int i = 0; i < 8; i++) {
          if (received_data[i] <= 0xF)
            Serial.print("0");
          Serial.print(received_data[i], HEX); 
        }
        Serial.println();
      }
      
      // Delay just a little bit to let the other unit
      // make the transition to receiver
      delay(20);
    } 
  }
}
