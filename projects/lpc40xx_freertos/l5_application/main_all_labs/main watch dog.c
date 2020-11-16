// #include "FreeRTOS.h"
// #include "acceleration.h"
// #include "ff.h"
// #include "lpc40xx.h"
// #include "queue.h"
// #include "task.h"
// #include <stdio.h>
// #include <string.h>

// /* -------------------------------------------------------------------------- */
// /* -------------------------------------------------------------------------- */
// /* -------------------------------------------------------------------------- */
// /* -------------------------------------------------------------------------- */
// /* -------------------------------------------------------------------------- */

// #include "cli_handlers.h"
// #include "event_groups.h"
// EventGroupHandle_t object;
// QueueHandle_t sd_card_Q;

// void Producer(void) {
//   int data_send;
//   int sample_100[100];
//   acceleration__axis_data_s acc_object;
//   while (1) {
//     for (int i = 0; i < 100; i++) {
//       acc_object = acceleration__get_data();
//       sample_100[i] = acc_object.z;
//       data_send += sample_100[i];
//     }
//     data_send = data_send / 100;
//     if (xQueueSend(sd_card_Q, &data_send, 0)) {
//       data_send = 0; // finished sending data on the queue. reset the average value to 0
//     }
//     // puts("Producer ready to check-in with 0x04\n");
//     xEventGroupSetBits(object, 0x04);
//     vTaskDelay(500);
//   }
// }

// void Consumer(void) {
//   int data_receive;
//   char string[64];
//   const char *filename = "output.txt";
//   FIL file; // File handle
//   UINT bytes_written = 0;
//   FRESULT result = f_open(&file, filename, (FA_WRITE | FA_CREATE_ALWAYS));
//   while (1) {
//     if (xQueueReceive(sd_card_Q, &data_receive, portMAX_DELAY)) {
//       if (FR_OK == result) {
//         // char string[64];
//         int time = xTaskGetTickCount();
//         sprintf(string, "%d, %d\n", data_receive, time);
//         if (FR_OK == f_write(&file, string, strlen(string), &bytes_written)) {
//         } else {
//           fprintf(stderr, "ERROR: Failed to write data to file\n");
//         }
//         f_sync(&file);
//       } else {
//         fprintf(stderr, "ERROR: Failed to open: %s\n", filename);
//       }
//     }
//     // puts("Consumer ready to check-in with 0x08\n");
//     f_close(&file);
//     xEventGroupSetBits(object, 0x08);
//   }
// }
// void Watchdog(void) {
//   while (1) {
//     vTaskDelay(1000);                                    // wait for all the tasks to set all the bits
//     uint8_t check_in = xEventGroupWaitBits(object,       // The event group being tested.
//                                            0x0C,         // The bits within the event group to wait for.
//                                            1,            // Bit from Producer and Consumer should be clear0
//                                            0,            // Using AND logic
//                                            0);           // We already waited for 1second using vTaskDelay
//     uint8_t is_bit_cleared = xEventGroupWaitBits(object, // The event group being tested.
//                                                  0x0C,   // The bits within the event group to wait for.
//                                                  1,      // Bit from Producer and Consumer should be clear0
//                                                  1,      // Using AND logic
//                                                  0);     // We already waited for 1second using vTaskDelay
//     if (check_in == 0x0C) {
//       puts("The two tasks are fine\n");
//     }
//     if (check_in == 0x04) {
//       puts("Consumer with 0x08 is dead\n");
//     }
//     if (check_in == 0x08) {
//       puts("Producer with 0x04 is dead\n");
//     }
//     if (check_in == 0x00) {
//       puts("Both tasks are dead\n");
//     }
//     check_in = 0x00;
//   }
// }

// void main(void) {
//   acceleration__init();
//   sj2_cli__init();

//   xTaskCreate(Producer, "Producer", 2048 / sizeof(void *), NULL, PRIORITY_MEDIUM, NULL);
//   xTaskCreate(Consumer, "Consumer", 2048 / sizeof(void *), NULL, PRIORITY_MEDIUM, NULL);
//   xTaskCreate(Watchdog, "Watchdog", 2048 / sizeof(void *), NULL, PRIORITY_HIGH, NULL);
//   object = xEventGroupCreate();
//   sd_card_Q = xQueueCreate(1, sizeof(int));
//   vTaskStartScheduler();
// }
