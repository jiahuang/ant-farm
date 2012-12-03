
//2.4G Configuration - Receiver
void configure_receiver(uint8_t * address);
//Sends one byte to nRF
// uint8_t rx_send_byte(uint8_t cmd);
//Sends command to nRF
// uint8_t rx_send_command(uint8_t cmd, uint8_t data);
//Sends the 4 bytes of payload
// void rx_send_payload(uint8_t cmd);
//Basic SPI to nRF
// uint8_t rx_spi_byte(uint8_t outgoing);


void init_nRF_pins(void)
{
  //1 = Output, 0 = Input
  //TX_PORT_DD = 0xFF & ~(1<<TX_MISO);// | 1<<TX_IRQ);
  TX_PORT_DD = 0xFF & ~(1<<TX_MISO);// | 1<<RX_IRQ);

  //Enable pull-up resistors (page 74)
  //TX_PORT = 0b11111111;
  TX_PORT = 0b11111111;

  cbi(PORTB, TX_CE); //Stand by mode
  //cbi(TX_PORT, TX_CE); //Stand by mode
}


//Reads the current RX buffer into the data array
//Forces an RX buffer flush
void receive_data(void)
{
  cbi(PORTB, TX_CSN); //Stand by mode
    tx_spi_byte(0x61); //Read RX Payload
  data_array[0] = tx_spi_byte(0xFF);
  data_array[1] = tx_spi_byte(0xFF);
  data_array[2] = tx_spi_byte(0xFF);
  data_array[3] = tx_spi_byte(0xFF);
  sbi(PORTB, TX_CSN);
  
  tx_send_byte(0xE2); //Flush RX FIFO
  
  tx_send_command(0x27, 0x40); //Clear RF FIFO interrupt

  sbi(PORTB, TX_CE); //Go back to receiving!
}

//2.4G Configuration - Receiver
//This setups up a RF-24G for receiving at 1mbps
void configure_receiver(uint8_t * address)
{
  cbi(PORTB, TX_CE); //Go into standby mode

  tx_send_command(0x20, 0x39); //Enable RX IRQ, CRC Enabled, be a receiver

  tx_send_command(0x21, 0x00); //Disable auto-acknowledge

  tx_send_command(0x23, 0x03); //Set address width to 5bytes (default, not really needed)

  tx_send_command(0x26, 0x07); //Air data rate 1Mbit, 0dBm, Setup LNA

  tx_send_command(0x31, 0x04); //4 byte receive payload

  tx_send_command(0x25, 0x02); //RF Channel 2 (default, not really needed)

  // data_array[0] = 0xE7;
  // data_array[1] = 0xE7;
  // data_array[2] = 0xE7;
  // data_array[3] = 0xE7;
  tx_send_payload(0x2A, 5, address); //Set RX pipe 0 address

  tx_send_command(0x20, 0x3B); //RX interrupt, power up, be a receiver

  sbi(PORTB, TX_CE); //Start receiving!
  
  delay_us(500);
}    

//Sends the 4 bytes of payload
// void rx_send_payload(uint8_t cmd)
// {
//   uint8_t i;

//   cbi(RX_PORT, RX_CSN); //Select chip
//   rx_spi_byte(cmd);
  
//   for(i = 0 ; i < 4 ; i++)
//     rx_spi_byte(data_array[i]);

//   sbi(RX_PORT, RX_CSN); //Deselect chip
// }

//Sends command to nRF
// uint8_t rx_send_command(uint8_t cmd, uint8_t data)
// {
//   uint8_t status;

//   cbi(TX_PORT, TX_CE); //Stand by mode

//   cbi(TX_PORT, TX_CSN); //Select chip
//   rx_spi_byte(cmd);
//   status = rx_spi_byte(data);
//   sbi(TX_PORT, TX_CSN); //Deselect chip
  
//   return(status);
// }

//Sends one byte to nRF
// uint8_t rx_send_byte(uint8_t cmd)
// {
//   uint8_t status;

//   cbi(RX_PORT, RX_CSN); //Select chip
//   status = rx_spi_byte(cmd);
//   sbi(RX_PORT, RX_CSN); //Deselect chip
  
//   return(status);
// }

// //Basic SPI to nRF
// uint8_t rx_spi_byte(uint8_t outgoing)
// {
//     uint8_t i, incoming;
//   incoming = 0;

//     //Send outgoing byte
//     for(i = 0 ; i < 8 ; i++)
//     {
//       if(outgoing & 0b10000000)
//         sbi(RX_PORT, RX_MOSI);
//       else
//         cbi(RX_PORT, RX_MOSI);

//       sbi(RX_PORT, RX_SCK); //RX_SCK = 1;
//       delay_us(RF_DELAY);


//       incoming <<= 1;
//       if( RX_PORT_PIN & (1<<RX_MISO) ) incoming |= 0x01;
      
//       cbi(RX_PORT, RX_SCK); //RX_SCK = 0; 
//       delay_us(RF_DELAY);

//       outgoing <<= 1;
//     }

//   return(incoming);
// }
