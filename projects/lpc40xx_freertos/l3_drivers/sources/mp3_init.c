#include "mp3_init.h"

void mp3_configured_pin() {
  /* -------------------------------------------------------------------------- */
  /*                        INITALIZE PIN FOR SPI DECODER                       */
  /* -------------------------------------------------------------------------- */

  // sck: p0.15
  gpio__construct_with_function(GPIO__PORT_0, 15, GPIO__FUNCTION_2);

  // mis0: p0.17
  gpio__construct_with_function(GPIO__PORT_0, 17, GPIO__FUNCTION_2);

  // mosi: p0.18
  gpio__construct_with_function(GPIO__PORT_0, 18, GPIO__FUNCTION_2);

  /* -------------------------------------------------------------------------- */
  /*                             INITIALIZE GPIO PIN                            */
  /* -------------------------------------------------------------------------- */
  // setup DREQ
  gpio__construct_with_function(GPIO__PORT_2, 0, GPIO__FUNCITON_0_IO_PIN);
  GPIO__set_as_input(2, 0);

  // setup CS (low)
  gpio__construct_with_function(GPIO__PORT_2, 1, GPIO__FUNCITON_0_IO_PIN);
  GPIO__set_as_output(2, 1);

  // setup XDCS (low)
  gpio__construct_with_function(GPIO__PORT_2, 2, GPIO__FUNCITON_0_IO_PIN);
  GPIO__set_as_output(2, 2);

  // setup Reset (low)
  gpio__construct_with_function(GPIO__PORT_2, 4, GPIO__FUNCITON_0_IO_PIN);
  GPIO__set_as_output(2, 4);
}

void deactive_reset_decoder() { GPIO__set_high(2, 4); }
void active_reset_decoder() { GPIO__set_low(2, 4); }

void ds_decoder() { GPIO__set_high(2, 1); }
void cs_decoder() { GPIO__set_low(2, 1); }

void xdcs_decoder_high() { GPIO__set_high(2, 2); }
void xdcs_decoder_low() { GPIO__set_low(2, 2); }

uint16_t mp3_read(uint8_t address) {
  while (!GPIO__get_level(2, 0)) {
    ; // waiting for DREQ
  }
  uint16_t data1;
  uint16_t data2;
  cs_decoder();
  xdcs_decoder_low();
  {
    ssp0__exchange_byte_lab(0x03); // sending opcode for read
    ssp0__exchange_byte_lab(address);
    data1 = ssp0__exchange_byte_lab(0xFF);
    while (!GPIO__get_level(2, 0)) {
      ; // waiting for DREQ
    }
    data2 = ssp0__exchange_byte_lab(0xFF);
    while (!GPIO__get_level(2, 0)) {
      ; // waiting for DREQ
    }
  }
  xdcs_decoder_high();
  ds_decoder();
  uint16_t data = data1 << 8;
  data |= data2;
  return data;
}

void set_Volume(uint16_t volume) { mp3_write(SCI_VOL, volume); }
void mp3_write(uint8_t address_register, uint16_t high_low_byte_data) {
  while (!GPIO__get_level(2, 0)) {
    ; // waiting for DREQ
  }
  cs_decoder();
  xdcs_decoder_low();
  {
    ssp0__exchange_byte_lab(0x02); // opcode for write operation
    ssp0__exchange_byte_lab(address_register);
    decoder_flash_send_data(high_low_byte_data);
    while (!GPIO__get_level(2, 0)) {
      ; // waiting for DREQ
    }
  }
  xdcs_decoder_high();
  ds_decoder();
}

void send_data_to_decoder(uint8_t data_byte) {
  while (!GPIO__get_level(2, 0)) {
    ; // waiting for DREQ
  }
  xdcs_decoder_low();
  ssp0__exchange_byte_lab(data_byte);
  xdcs_decoder_high();
}
void mp3_init() {

  /* -------------------------------------------------------------------------- */
  /*                            CONFIGURATION FOR MP3                           */
  /* -------------------------------------------------------------------------- */

  mp3_configured_pin();
  deactive_reset_decoder();
  ssp0__init_mp3(3 * 1000 * 1000);
  ssp0__exchange_byte_lab(0xFF);
  ds_decoder();
  xdcs_decoder_high();
  set_Volume(0x2525);

  /* -------------------------------------------------------------------------- */
  /*                      TESTING FOR THE STATE OF DECODER                      */
  /* -------------------------------------------------------------------------- */

  uint16_t MP3Status = mp3_read(SCI_STATUS);
  uint16_t vsVersion = (MP3Status >> 4) & 0x000F;
  printf("Version of VS1053: %d\n", vsVersion);

  uint16_t MP3_CLK = mp3_read(SCI_CLOCKF);
  printf("SCK: %x\n", MP3_CLK);
  delay__ms(100);
  mp3_write(SCI_CLOCKF, 0x6000);
  uint16_t MP3_CLK_2 = mp3_read(SCI_CLOCKF);
  printf("SCK_2: %x\n", MP3_CLK_2);

  uint16_t MP3Mode = mp3_read(SCI_MODE);
  printf("Mode: %x\n", MP3Mode);

  /* -------------------------------------------------------------------------- */
  /*                        END READING STATE OF DECODER                        */
  /* -------------------------------------------------------------------------- */

  /* ------------------- Setting the decoder to run at 4Mhz ------------------- */
  decoder_clock(4000);
  uint16_t MP3_CLK_3 = mp3_read(SCI_CLOCKF);
  printf("SCK_3: %x\n", MP3_CLK_3);
}