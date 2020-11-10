#include "decoder.h"

/* -------------------------------------------------------------------------- */
/* ---------------------------- Private function ---------------------------- */
/* -------------------------------------------------------------------------- */

/* ------------------------------- Clock setup ------------------------------ */
static void decoder_ssp0__init(uint32_t SPI_clock_mhz) {
  SPI_clock_mhz = SPI_clock_mhz * 1000 * 1000;
  /* a) Power on Peripheral */
  lpc_peripheral__turn_on_power_to(LPC_PERIPHERAL__SSP0);

  /* b) Setup control registers CR0 and CR1 */
  LPC_SSP0->CR0 = 7;        // 8-Bit transfer
  LPC_SSP0->CR1 = (1 << 1); // SSP Control Enable

  /* c) Setup prescalar register to be <= SPI_clock-(Input) */
  const uint32_t CPU_CLK = clock__get_core_clock_hz(); // 96-MHz
  for (uint8_t divider = 2; divider <= 254; divider += 2) {
    if ((CPU_CLK / divider) <= SPI_clock_mhz) {
      // fprintf(stderr, "Pre_Scale: %d \n", divider);
      break;
    }
    /* Setup PreScale Control[7:0] */
    LPC_SSP0->CPSR = divider;
  }
}

static void decoder_max_clock(uint32_t SPI_clock_mhz) {
  SPI_clock_mhz = SPI_clock_mhz * 1000 * 1000;

  /* c) Setup prescalar register to be <= SPI_clock-(Input) */
  const uint32_t CPU_CLK = clock__get_core_clock_hz(); // 96-MHz
  for (uint8_t divider = 2; divider <= 254; divider += 2) {
    if ((CPU_CLK / divider) <= SPI_clock_mhz) {
      // fprintf(stderr, "Pre_Scale: %d \n", divider);
      break;
    }
    /* Setup PreScale Control[7:0] */
    LPC_SSP0->CPSR = divider;
  }
}

/* -------------------------------- Pincongig ------------------------------- */
static void decoder_ssp0_PINconfig() {
  /* -------------------------------- SPI0 PIN  */
  gpio__construct_with_function(0, 15, GPIO__FUNCTION_2); // CLK
  gpio__construct_with_function(0, 17, GPIO__FUNCTION_2); // MISO
  gpio__construct_with_function(0, 18, GPIO__FUNCTION_2); // MOSI

  /* ------------------------------ GPIO Control */
  gpio__construct_with_function(2, 0, GPIO__FUNCITON_0_IO_PIN);
  GPIO__set_as_input(2, 0); // setup DREQ

  gpio__construct_with_function(2, 1, GPIO__FUNCITON_0_IO_PIN);
  GPIO__set_as_output(2, 1); // setup CS (low active)

  gpio__construct_with_function(2, 2, GPIO__FUNCITON_0_IO_PIN);
  GPIO__set_as_output(2, 2); // setup DCS or XDCS (low active)

  gpio__construct_with_function(2, 4, GPIO__FUNCITON_0_IO_PIN);
  GPIO__set_as_output(2, 4); // setup Reset (low active)
}

/* ------------------------------ Transmit Data ----------------------------- */
static uint8_t decoder_ssp0_transferByte(uint8_t data_transfer) {
  /* 16-Bits Data Register [15:0] */
  LPC_SSP0->DR = data_transfer;

  /* Status Register-BUSY[4] */
  while (LPC_SSP0->SR & (1 << 4)) {
    ; /* Wait while it is busy(1), else(0) BREAK */
  }
  /* READ 16-Bits Data Register [15:0] */
  return (uint8_t)(LPC_SSP0->DR & 0xFF);
}

/* -------------------------------------------------------------------------- */
/* ----------------------------- Public Function ---------------------------- */
/* -------------------------------------------------------------------------- */

/* (P2_0) Input */
bool get_DREQ_HighActive() { return LPC_GPIO2->PIN & (1 << 0) ? true : false; }
/* (P2_1) Chip Select */
void set_CS_LowActive() { LPC_GPIO2->CLR |= (1 << 1); }
void set_CS_HighActive() { LPC_GPIO2->SET |= (1 << 1); }
/* (P2_2) MP3 Data Control  */
void set_XDCS_LowActive() { LPC_GPIO2->CLR |= (1 << 2); }
void set_XDCS_HighActive() { LPC_GPIO2->SET |= (1 << 2); }
/* (P2_4) Reset  */
void set_RESET_LowActive() { LPC_GPIO2->CLR |= (1 << 4); }
void set_RESET_HighActive() { LPC_GPIO2->SET |= (1 << 4); }

/* ------------------------------ SETUP Decoder ----------------------------- */
void decoder_setup() {

  /* Setup SPI_pin + GPIO (DREQ, CS, DCS, RESET) */
  decoder_ssp0_PINconfig();

  /* Deactivate reset */
  set_RESET_HighActive();

  /* Init SPI0(CLK): 1 MHZ */
  decoder_ssp0__init(1);

  /* Send dummy byte */
  decoder_ssp0_transferByte(0xFF);

  /* Deselect --> CS & XDCS */
  set_CS_HighActive();
  set_XDCS_HighActive();

  /* Set default Volume */
  decoder_write_register(SCI_VOL, 25, 25);

  /* Set Bass + Treble */
  set_BassLevel(1);
  set_TrebleLevel(1);

  /* --------------------- Decoder Version + MODE + CLOCK */
  uint16_t MP3Status = decoder_read_register(SCI_STATUS);
  int vsVersion = (MP3Status >> 4) & 0x000F; // four version bits
  printf("VS1053 Ver %d\n", vsVersion);

  uint16_t MP3Mode = decoder_read_register(SCI_MODE);
  printf("SCI_MODE = 0x%x\n", MP3Mode);

  /*****************************************************************
   *Now that we have the VS1053_ON + RUNNING                       *
   *----> Increase the VS1053 internal clock multiplier.           *
   *----> Up our SPI Speed                                         *
   *----> Please Read DataSheet Page.12(max SCI reads are CLKI/72) *
   *---->  Frequency = safe < 4MHz (Tested)                        *
   ****************************************************************/
  /* Set multiplier to 3.0x */
  decoder_write_register(SCI_CLOCKF, 0x60, 0x00);
  /* Set SPI bus speed to 4MHz (16MHz / 4 = 4MHz) */
  decoder_max_clock(4);

  uint16_t MP3Clock = decoder_read_register(SCI_CLOCKF);
  printf("SCI_CLK = 0x%x\n", MP3Clock);
}

/*!!!!! Need to check variable type !!! Caution */
/* -------------------------------- READ REG -------------------------------- */
uint16_t decoder_read_register(uint16_t register_address) {
  while (!get_DREQ_HighActive()) {
    ; // wait
  }
  set_CS_LowActive(); // ON

  /* Instruction (8bits) + address(8bits) + data(16bits) */
  decoder_ssp0_transferByte(0x03); //  OP_code for READ
  decoder_ssp0_transferByte(register_address);

  uint8_t first_8bits = decoder_ssp0_transferByte(0xFF); // Read the 1st byte
  while (!get_DREQ_HighActive()) {
    ; // wait
  }
  uint8_t second_8bits = decoder_ssp0_transferByte(0xFF); // Read the 2nd byte
  while (!get_DREQ_HighActive()) {
    ; // wait
  }
  set_CS_HighActive(); // OFF

  uint16_t finalvalue = 0;
  finalvalue |= ((first_8bits << 8) | (second_8bits << 0));
  return finalvalue;
}

/* -------------------------------- WRITE REG ------------------------------- */
void decoder_write_register(uint16_t register_address, uint8_t MSB_byte, uint8_t LSB_byte) {
  while (!get_DREQ_HighActive()) {
    ; // Wait
  }
  set_CS_LowActive(); // ON

  /* Instruction (8bits) + address(8bits) + data(16bits) */
  decoder_ssp0_transferByte(0x02); //  OP_code for WRITE
  decoder_ssp0_transferByte(register_address);

  /* Write */
  decoder_ssp0_transferByte(MSB_byte);
  decoder_ssp0_transferByte(LSB_byte);
  while (!get_DREQ_HighActive()) {
    ; // Wait
  }
  set_CS_HighActive(); // OFF
}

/* --------------------------- SEND MP3 DATA Byte --------------------------- */
void decoder_send_mp3Data(uint8_t data) {
  set_XDCS_LowActive();            // Select
  decoder_ssp0_transferByte(data); // Send SPI Data
  set_XDCS_HighActive();           // Deselect
}

/* -------------------------- Bass Effect function -------------------------- */
void set_Bass(uint8_t amplitude, uint8_t frequency) {
  /* Amplitude + Frequency Boundary */
  if ((amplitude <= 15) && (frequency >= 2) && (frequency <= 15)) {
    /* SCI_BASS(W/R)---Amp[7:4]-Freq[3:0]--BassEffect */
    uint8_t setup_Bass = ((amplitude << 4) | (frequency << 0));
    uint16_t current_Bass = decoder_read_register(SCI_BASS);
    decoder_write_register(SCI_BASS, (current_Bass >> 8), setup_Bass);
  }
}

void set_BassLevel(uint8_t level) {
  switch (level) {
  case 1:
    set_Bass(3, 6);
    break;
  case 2:
    set_Bass(6, 6);
  case 3:
    set_Bass(9, 6);
  case 4:
    set_Bass(12, 6);
  case 5:
    set_Bass(15, 6);
  default:
    set_Bass(0, 6);
    break;
  }
}

/* ------------------------- Treble Effect Function ------------------------- */
void set_Treble(int8_t amplitude, uint8_t frequency) {
  if (((amplitude >= -8) && (amplitude <= 7)) && ((frequency >= 1) && (frequency <= 15))) {
    /* SCI_BASS(W/R)---Amp[15:12]-Freq[11:8]--BassEffect */
    uint8_t setup_Treble = ((amplitude << 12) | (frequency << 8));
    uint16_t current_Treble = decoder_read_register(SCI_BASS);
    decoder_write_register(SCI_BASS, setup_Treble, (current_Treble & 0xff));
  }
}

void set_TrebleLevel(uint8_t level) {
  switch (level) {
  case 1:
    set_Treble(-5, 5);
    break;
  case 2:
    set_Treble(-2, 5);
  case 3:
    set_Treble(1, 5);
  case 4:
    set_Treble(4, 5);
  case 5:
    set_Treble(7, 5);
  default:
    set_Treble(0, 5);
    break;
  }
}