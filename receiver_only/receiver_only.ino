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

//
// Hardware configuration
//

// Set up nRF24L01 radio on SPI bus plus pins 9 & 10 

RF24 radio(9,10);

//
// Topology
//

// Open a reading address. This address needs to match the address that the ant is transmitting at
const uint64_t pipes[1] = { 0xE7E7E7E7E7LL };

//
// Role management
//
// Set up role.  This sketch uses the same software for all the nodes
// in this system.  Doing so greatly simplifies testing.  
//

// The various roles supported by this sketch
// typedef enum { role_ping_out = 1, role_pong_back } role_e;

// The debug-friendly names of those roles
// const char* role_friendly_name[] = { "invalid", "Ping out", "Pong back"};

// The role of the current running sketch
//role_e role = role_pong_back;

void setup(void)
{
  //
  // Print preamble
  //

  Serial.begin(57600);
  printf_begin();
  printf("\n\rsoldier\n\r");
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
  radio.openReadingPipe(1,0xE7E7E7E7E7LL);
  // optionally, increase the delay between retries & # of retries
//  radio.setRetries(15,15);
  // radio.setPALevel(RF24_PA_MID);
//  radio.setDataRate(RF24_250KBPS);
  // optionally, reduce the payload size.  seems to
  // improve reliability
  // radio.setPayloadSize(8);

  // read at the pipe
//  radio.openReadingPipe(0,pipes[0]);
 
  //
  // Start listening
  //
  Serial.println("About to start listening");
  radio.startListening();

  //
  // Dump the configuration of the rf unit for debugging
  //
  Serial.println("Here comes the details");
  radio.printDetails();
  Serial.println("hey all done here");
}

void loop(void)
{
  // if there is data ready
  //printf("" + radio.available());
    if ( radio.available() )
    {
      //Serial.print("Hi");
      //printf("got something");
      // Dump the payloads until we've gotten everything
      byte received_data[8];
      bool done = false;
      while (!done)
      {
//        Serial.print("inner loop ");
        // Fetch the payload, and see if this was the last one.
        done = radio.read( &received_data, 8 );
//        Serial.println("DONE" + done);
        // Spew it
        Serial.print("Got payload: ");
        
//        Serial.println(received_data);
        for (int i = 0; i < 8; i++) {
          if (received_data[i] <= 0xF)
            Serial.print("0");
          Serial.print(received_data[i], HEX); 
        }
        Serial.println();

        // Delay just a little bit to let the other unit
        // make the transition to receiver
        delay(20);
      }

      // // First, stop listening so we can talk
      // radio.stopListening();

      // // Send the final one back.
      // radio.write( &got_time, sizeof(unsigned long) );
      // printf("Sent response.\n\r");

      // // Now, resume listening so we catch the next packets.
      // radio.startListening();
    }
}
