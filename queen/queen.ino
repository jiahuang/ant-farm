// modified from RF24's getting started example

/*
 Copyright (C) 2011 J. Coliz <maniacbug@ymail.com>

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 version 2 as published by the Free Software Foundation.
 */

/**
 * Example for Getting Started with nRF24L01+ radios. 
 *
 * This is an example of how to use the RF24 class.  Write this sketch to two 
 * different nodes.  Put one of the nodes into 'transmit' mode by connecting 
 * with the serial monitor and sending a 'T'.  The ping node sends the current 
 * time to the pong node, which responds by sending the value back.  The ping 
 * node can then see how long the whole cycle took.
 */

#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "printf.h"

// how often we transmit an id
#define PING_INCREMENT 20
// Hardware configuration
//

unsigned long last_ping = 0;

// Set up nRF24L01 radio on SPI bus plus pins 9 & 10 
RF24 radio(9,10);

// Set up nRF24L01 radio on SPI bus plus pins 9 & 10 
RF24 radio2(6,7);

int TRANSMIT_TIME = 30000;
int transmit_time_counter = 0;
//
// Topology
//

// Open a reading address. This address needs to match the address that the ant is transmitting at
const uint64_t pipes[1] = { 0xE7E7E7E7E7LL };

void setup(void)
{
  //
  // Print preamble
  //

  Serial.begin(57600);
  printf_begin();
  printf("\n\rQueen\n\r");
  printf("ROLE: Receiver\n\r");

  //
  // Setup and configure rf radio
  //
  Serial.println("Hey");
  radio.begin();
  Serial.println("It has begined");
  radio.setChannel(2);
  radio.setPayloadSize(8);
  radio.setAutoAck(false);
  radio.setCRCLength(RF24_CRC_8);
  radio.setRetries(15,15);
  
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
 
  //
  // Start listening
  //
  Serial.println("About to start listening");
//  radio.startListening();

  //
  // Dump the configuration of the rf unit for debugging
  //
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
  if ((millis()/1000.0 - last_ping)  >= PING) {
      Serial.println("Yep, 20 seconds occurred");
      last_ping = millis();
//    transmit_init();
//      byte queen_id[8] = {30,30,30,30, 0, 0, 0, 0};
//      boolean okay = transmit(queen_id);
//      if (okay) {
//       transmit_time_counter = 0;
//       receive_init();
//      } else {
//       Serial.println("OOOOHHHH NOOOO"); 
//      }
  } else {
//    transmit_time_counter++;
////    Serial.println("about to receive");
//    receive_data();
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

//void receive_data(void) {
// if ( radio.available() )
//  {
//    // Dump the payloads until we've gotten everything
//    byte received_data[8];
//    bool done = false;
//    while (!done)
//    {
//      // Fetch the payload, and see if this was the last one.
//      done = radio.read( &received_data, 8 );
//
//      if (payload_is_from_ant(received_data)) {
//          serial_out_payload(received_data);
//      }
//      
//      // Delay just a little bit to let the other unit
//      // make the transition to receiver
//      delay(20);
//    } 
//  }
//}

//void payload_is_from_ant(byte[] received_data) {
//  return ( (received_data[4] | received_data[5] | 
//        received_data[6] | received_data[7]) != 0x0);
//}
//
//void serial_out_payload(byte[] received_data) {
//  Serial.print("Got payload: ");
//  for (int i = 0; i < 8; i++) {
//      if (received_data[i] <= 0xF) Serial.print("0");
//      Serial.print(received_data[i], HEX); 
//      }
//  Serial.println();
//}
