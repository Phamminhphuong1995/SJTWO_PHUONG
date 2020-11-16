// #include "i2c.h"
// #include "lpc40xx.h"
// #include "sj2_cli.h"

// const uint32_t i2c_speed_hz = UINT32_C(400) * 1000;
// void loop_back_set_for_i2c_0(uint8_t slave_address) {
//   LPC_IOCON->P0_1 |= (1 << 10);
//   LPC_IOCON->P0_0 |= (1 << 10);
//   gpio__construct_with_function(0, 1, 3);
//   gpio__construct_with_function(0, 0, 3);

//   i2c__initialize(I2C__1, i2c_speed_hz, clock__get_peripheral_clock_hz());
//   i2c_slave_init_loopback(slave_address);
//   puts("Checking again for i2c loop back devices\nAfter the peripheral init\n");
//   for (unsigned slave_address = 2; slave_address <= 254; slave_address += 2) {
//     if (i2c__detect(I2C__2, slave_address)) {
//       printf("I2C 1 slave detected at address: 0x%02X\n", slave_address);
//     }
//   }
// }

// void main() {
//   loop_back_set_for_i2c_0(0x84);
//   sj2_cli__init();

//   vTaskStartScheduler();
// }