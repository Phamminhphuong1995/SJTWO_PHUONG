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

char *remove_dot_mp3(char *current_song) {
  trackname_t song_name_copy;
  char *copy_song;
  copy_song = song_name_copy;
  //   display(song_name_copy);
  strcpy(song_name_copy, current_song);
  //   display(song_name_copy);
  copy_song = strtok(song_name_copy, ".");
  return copy_song;
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

/* -------------------------------------------------------------------------- */
/*                                 GENRE TABLE                                */
/* -------------------------------------------------------------------------- */

char *genre_decoder(uint8_t genre) {
  char *ger;
  switch (genre) {
  case 0:
  case 1:
  case 2:
  case 3:
  case 4:
  case 5:
  case 6:
  case 7:
  case 8:
  case 9:
  case 10:
  case 11:
    ger = "Genre: Pop";
    break;
  case 12:
  case 13:
  case 14:
  case 15:
  case 16:
  case 17:
  case 18:
  case 19:
  case 20:
  case 21:
  case 22:
  case 23:
  case 24:
  case 25:
  case 26:
  case 27:
  case 28:
  case 29:
  case 30:
  case 31:
  case 32:
  case 33:
  case 34:
  case 35:
  case 36:
  case 37:
  case 38:
  case 39:
  case 40:
  case 41:
  case 42:
  case 43:
  case 44:
  case 45:
  case 46:
  case 47:
  case 48:
  case 49:
  case 50:
  case 51:
  case 52:
  case 53:
  case 54:
  case 55:
  case 56:
  case 57:
  case 58:
  case 59:
  case 60:
  case 61:
  case 62:
  case 63:
  case 64:
  case 65:
  case 66:
  case 67:
  case 68:
  case 69:
  case 70:
  case 71:
  case 72:
  case 74:
  case 75:
  case 76:
  case 77:
  case 78:
  case 79:
  case 80:
  case 81:
  case 82:
  case 83:
  case 84:
  case 85:
  case 86:
  case 87:
  case 88:
  case 89:
  case 90:
  case 91:
  case 92:
  case 93:
  case 94:
  case 95:
  case 96:
  case 97:
  case 98:
  case 99:
  case 100:
  case 101:
  case 102:
  case 103:
  case 104:
  case 105:
  case 106:
  case 107:
  case 108:
  case 109:
  case 110:
  case 111:
  case 112:
  case 113:
  case 114:
  case 115:
  case 116:
    ger = "Ballad";
    break;
  default:
    ger = "No Genre";
    break;
  }
  return ger;
}