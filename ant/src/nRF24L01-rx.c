/*
  Modified from Sparkfun code at https://www.sparkfun.com/products/8602
*/

//2.4G Configuration - Receiver
void configure_receiver(uint8_t * address, uint8_t rf_channel);
void receive_data(uint8_t * data, uint8_t size);

//Reads the current RX buffer into the data array
//Forces an RX buffer flush
void receive_data(uint8_t * data, uint8_t size)
{
  cbi(PORTB, TX_CSN); //Stand by mode

  tx_spi_byte(0x61); //Read RX Payload

  for (int i = 0; i < size; i++) {
    data[i] = tx_spi_byte(0xFF);
  }

  sbi(PORTB, TX_CSN);
  
  tx_send_byte(0xE2); //Flush RX FIFO
  tx_send_command(0x27, (1 << RX_DR) | (1 << TX_DS) | (1 << MAX_RT));

  sbi(PORTB, TX_CE); //Go back to receiving!
}

//2.4G Configuration - Receiver
//This setups up a RF-24G for receiving at 1mbps
void configure_receiver(uint8_t * address, uint8_t rf_channel)
{
  cbi(PORTB, TX_CE); //Go into standby mode

  tx_send_command(0x20, 0x39); //Enable RX IRQ, CRC Enabled, be a receiver

  tx_send_command(0x21, 0x00); //Disable auto-acknowledge

  tx_send_command(0x23, 0x03); //Set address width to 5bytes (default, not really needed)

  tx_send_command(0x26, 0x00); //Air data rate 1Mbit, 0dBm, Setup LNA

  tx_send_command(0x31, 0x08); //8 byte receive payload

  tx_send_command(0x25, rf_channel); //RF Channel

  tx_send_payload(0x2A, 5, address); //Set RX pipe 0 address

  tx_send_command(0x20, 0x3B); //RX interrupt, power up, be a receiver

  sbi(PORTB, TX_CE); //Start receiving!
  
} 
