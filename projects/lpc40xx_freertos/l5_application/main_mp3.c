#include "FreeRTOS.h"
#include "cli_handlers.h"
#include "ff.h"
#include "gpio_isr.h"
#include "gpio_lab.h"
#include "lpc_peripherals.h"
#include "mp3_init.h"
#include "oled.h"
#include "queue.h"
#include "sj2_cli.h"
#include "task.h"
#include <string.h>

SemaphoreHandle_t pause_semaphore;
QueueHandle_t Q_trackname;
QueueHandle_t Q_songdata;
void reader_task();
void player_task();
void pause_task();
volatile bool play_pause = true;
void pause_isr(void);
TaskHandle_t player_handle;
int main() {
  puts("Before MP3_init");
  mp3_init();
  turn_on_oled();
  clear();
  update();
  puts("End MP3_init\n\n");
  puts("Begin doing reader and player");
  sj2_cli__init();
  Q_trackname = xQueueCreate(1, sizeof(trackname_t));
  Q_songdata = xQueueCreate(1, 512);
  pause_semaphore = xSemaphoreCreateBinary();
  puts("check\n");
  lpc_peripheral__enable_interrupt(LPC_PERIPHERAL__GPIO, gpio0__interrupt_dispatcher, "isr for port 0");
  puts("check1\n");
  gpio0__attach_interrupt(30, GPIO_INTR__FALLING_EDGE, pause_isr);
  //   puts("check2\n");
  xTaskCreate(reader_task, "reader", (2024 * 4) / sizeof(void *), NULL, PRIORITY_MEDIUM, NULL);
  xTaskCreate(player_task, "player", (3096 * 4) / sizeof(void *), NULL, PRIORITY_MEDIUM, &player_handle);
  xTaskCreate(pause_task, "pause", (1024 * 4) / sizeof(void *), NULL, PRIORITY_MEDIUM, NULL);
  vTaskStartScheduler();
}
void pause_isr() { xSemaphoreGiveFromISR(pause_semaphore, NULL); }
void pause_task() {
  while (1) {
    if (xSemaphoreTake(pause_semaphore, portMAX_DELAY)) {
      if (play_pause) {
        vTaskSuspend(player_handle);
        play_pause = false;
      } else {
        vTaskResume(player_handle);
        play_pause = true;
      }
    }
  }
}
char *song_name_without_dot_mp3;
void reader_task() {
  trackname_t song_name;
  uint8_t byte_512[512];
  UINT br; // binary
  while (1) {
    if (xQueueReceive(Q_trackname, song_name, portMAX_DELAY)) {
      printf("Song name is: %s", song_name);
      display("Playing\n");
      song_name_without_dot_mp3 = remove_dot_mp3(song_name);
      display(song_name_without_dot_mp3);
      horizontal_scrolling();
      /* -------------------------------- OPEN FILE ------------------------------- */
      const char *filename = song_name;
      FIL file; // create object file
      FRESULT result = f_open(&file, filename, (FA_READ));
      if (FR_OK == result) {
        while (br != 0) {
          f_read(&file, byte_512, sizeof(byte_512), &br);
          xQueueSend(Q_songdata, byte_512, portMAX_DELAY);
          if (uxQueueMessagesWaiting(Q_trackname)) {
            printf("New play song request\n");
            break;
          }
        }
        f_close(&file);
      } else {
        puts("Failed to open mp3 file\n");
      }
    }
  }
}

void player_task() {
  while (1) {
    uint8_t byte_512[512];
    if (xQueueReceive(Q_songdata, byte_512, portMAX_DELAY)) {
      for (int i = 0; i < 512; i++) {
        while (!GPIO__get_level(2, 0)) {
          vTaskDelay(1); // waiting for DREQ
        }
        send_data_to_decoder(byte_512[i]);
        // printf("%x ", byte_512[i]);
      }
    }
  }
}