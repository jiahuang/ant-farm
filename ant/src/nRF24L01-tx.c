/*
    2-8-2008
    Copyright Spark Fun Electronics© 2008
  
  Basic routines for nRF24L01
*/

#define TX_PORT   PORTA
#define TX_PORT_PIN PINA
#define TX_PORT_DD  DDRA

#define TX_SCK  4 //Output
#define TX_MISO 5 //Input
#define TX_MOSI 6 //Output

#define TX_CE 1 //Output
#define TX_CSN  2 //Output

//#define RF_DELAY  5
#define RF_DELAY  55

#define LIKE_BUTTON 0
#define LED 1
// #define LED2 2
#define ANALOG_VOLTAGE 3

#define RX_DR 6
#define TX_DS 5
#define MAX_RT 4

#define W_REGISTER    0x20
#define W_TX_PAYLOAD  0xA0

//2.4G Configuration - Transmitter
uint8_t configure_transmitter(uint8_t * address);
//Sends command to nRF
uint8_t tx_send_byte(uint8_t cmd);
//Basic SPI to nRF
uint8_t tx_send_command(uint8_t cmd, uint8_t data);
//Sends the 4 bytes of payload
void tx_send_payload(uint8_t cmd, uint8_t size, uint8_t * data);
//This sends out the data stored in the data_array
void transmit_data(uint8_t * data);
//Basic SPI to nRF
uint8_t tx_spi_byte(uint8_t outgoing);

//TX Functions
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

//This sends out the data stored in the data_array
//data_array must be setup before calling this function
void transmit_data(uint8_t * data)
{
  tx_send_command(0x27, 0x7E);  //Clear any interrupts
  
//  tx_send_command(0x20,0x7E); //Power up and be a transmitter. 2-byte CRC.
  tx_send_command(0x20,0b01111010);//Power up and be a transmitter. 1-byte CRC.

  tx_send_byte(0xE1);   //Clear TX Fifo
  
  tx_send_payload(0xA0, 8, data);  //Clock in 4 byte payload of data_array to TX pipe

    sbi(PORTB, TX_CE); //Pulse CE to start transmission
    delay_ms(3);
    cbi(PORTB, TX_CE);

  // tx_send_command(0x27, 0x7E); //Clear any interrupts
  
  // tx_send_command(W_REGISTER, 0x7A); //Power up and be a transmitter

  // tx_send_byte(0xE1); //Clear TX Fifo
  
  // tx_send_payload(W_TX_PAYLOAD); //Clock in 4 byte payload of data_array

  // sbi(PORTB, TX_CE); //Pulse CE to start transmission
  // delay_ms(3);
  // cbi(PORTB, TX_CE);
}

//2.4G Configuration - Transmitter
//This sets up one RF-24G for shockburst transmission
uint8_t configure_transmitter(uint8_t * address)
{
  cbi(PORTB, TX_CE); //Go into standby mode
  
  tx_send_command(W_REGISTER, 0x78); //CRC enabled, be a transmitter //0x7E);

  tx_send_command(0x21, 0x00); //Disable auto acknowledge on all pipes
  // tx_send_command(0x21, 0x03); //enable auto acknowledge on pipes 0, 1

  // tx_send_command(0x22, 0x00); // disable recieving

  tx_send_command(0x23, 0x03); //Set address width to 5bytes (default, not really needed)

  tx_send_command(0x24, 0x00); //Disable auto-retransmit
  // tx_send_command(0x24, 0x05); // retransmit 5 times w/ a 250uS delay each time

  tx_send_command(0x25, 0x02); //RF Channel 2

  tx_send_command(0x26, 0x07); //Air data rate 1Mbit, 0dBm, Setup LNA
  // tx_send_command(0x26, 0x01); //Air data rate 1Mbit, -18dBm, Setup LNA
  // tx_send_command(0x26, 0x00); //Air data rate 1Mbit, -18dBm, Setup LNA

  tx_send_payload(0x30, 5, address); //Set TX address
  
  tx_send_command(0x20, 0x7A); //Power up, be a transmitter

  return(tx_send_byte(0xFF));
}

//Sends the 4 bytes of payload
void tx_send_payload(uint8_t cmd, uint8_t size, uint8_t * data)
{
  uint8_t i;

  cbi(PORTB, TX_CSN); //Select chip
  tx_spi_byte(cmd);
  
  for(i = 0 ; i < size ; i++)
    tx_spi_byte(data[i]);

  sbi(PORTB, TX_CSN); //Deselect chip
}


//Sends command to nRF
uint8_t tx_send_command(uint8_t cmd, uint8_t data)
{
  uint8_t status;

  cbi(PORTB, TX_CSN); //Select chip
  tx_spi_byte(cmd);
  status = tx_spi_byte(data);
  sbi(PORTB, TX_CSN); //Deselect chip

  return(status);
}

//Sends one byte to nRF
uint8_t tx_send_byte(uint8_t cmd)
{
  uint8_t status;
  
  cbi(PORTB, TX_CSN); //Select chip
  status = tx_spi_byte(cmd);
  sbi(PORTB, TX_CSN); //Deselect chip
  
  return(status);
}

//Basic SPI to nRF
uint8_t tx_spi_byte(uint8_t outgoing)
{
    uint8_t i, incoming;
  incoming = 0;

    //Send outgoing byte
    for(i = 0 ; i < 8 ; i++)
    {
      if(outgoing & 0b10000000)
        sbi(TX_PORT, TX_MOSI);
      else
        cbi(TX_PORT, TX_MOSI);
      
      sbi(TX_PORT, TX_SCK); //TX_SCK = 1;
      delay_us(RF_DELAY);

      //MISO bit is valid after clock goes going high
      incoming <<= 1;
      if( TX_PORT_PIN & (1<<TX_MISO) ) incoming |= 0x01;

      cbi(TX_PORT, TX_SCK); //TX_SCK = 0; 
      delay_us(RF_DELAY);
      
      outgoing <<= 1;
    }

  return(incoming);
}
