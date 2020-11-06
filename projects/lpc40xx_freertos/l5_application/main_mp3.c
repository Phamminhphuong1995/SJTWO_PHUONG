#include "FreeRTOS.h"
#include "cli_handlers.h"
#include "ff.h"
#include "gpio_isr.h"
#include "gpio_lab.h"
#include "list_song.h"
#include "lpc_peripherals.h"
#include "mp3_init.h"
#include "oled.h"
#include "queue.h"
#include "sj2_cli.h"
#include "task.h"
#include <string.h>

/* -------------------------------------------------------------------------- */
/*                              SEMAPHORE SECTION                             */
/* -------------------------------------------------------------------------- */

SemaphoreHandle_t pause_semaphore;
SemaphoreHandle_t next_song;

/* -------------------------------------------------------------------------- */
/*                                QUEUE SECTION                               */
/* -------------------------------------------------------------------------- */

QueueHandle_t Q_trackname;
QueueHandle_t Q_songdata;

/* -------------------------------------------------------------------------- */
/*                          TASK DECLARATION SECTION                          */
/* -------------------------------------------------------------------------- */

void reader_task();
void player_task();
void pause_task();
void get_current_playing_song_name();

/* -------------------------------------------------------------------------- */
/*                           GLOBAL VARIABLE SECTION                          */
/* -------------------------------------------------------------------------- */

volatile bool play_pause = true;
uint16_t cursor_main = 0;
char *song_name_without_dot_mp3;

/* -------------------------------------------------------------------------- */
/*                     INTERRUPT SERVICE ROUNTINE SECTION                     */
/* -------------------------------------------------------------------------- */

void pause_isr(void);
void next_song_isr(void);

/* -------------------------------------------------------------------------- */
/*                             TASKHANDLE SECTION                             */
/* -------------------------------------------------------------------------- */

TaskHandle_t player_handle;

/* ---------------------------------- MAIN ---------------------------------- */

int main() {

  /* ----------------------- utility and initialization ----------------------- */
  populate_list_song();
  fprintf(stderr, "\ntotal song: %d", total_of_songs());
  mp3_init();
  turn_on_oled();
  clear();
  update();
  sj2_cli__init();

  /* --------------------------- Queue and Semaphore -------------------------- */

  Q_trackname = xQueueCreate(1, sizeof(trackname_t));
  Q_songdata = xQueueCreate(1, 512);
  pause_semaphore = xSemaphoreCreateBinary();
  next_song = xSemaphoreCreateBinary();

  /* -------------------------------- Interrupt ------------------------------- */

  lpc_peripheral__enable_interrupt(LPC_PERIPHERAL__GPIO, gpio0__interrupt_dispatcher, "isr for port 0");
  gpio0__attach_interrupt(30, GPIO_INTR__FALLING_EDGE, pause_isr);
  gpio0__attach_interrupt(29, GPIO_INTR__FALLING_EDGE, next_song_isr);

  /* ------------------------------ Task creation ----------------------------- */

  xTaskCreate(reader_task, "reader", (2024 * 4) / sizeof(void *), NULL, PRIORITY_MEDIUM, NULL);
  xTaskCreate(player_task, "player", (3096 * 4) / sizeof(void *), NULL, PRIORITY_MEDIUM, &player_handle);
  xTaskCreate(pause_task, "pause", (1024 * 4) / sizeof(void *), NULL, PRIORITY_MEDIUM, NULL);
  xTaskCreate(get_current_playing_song_name, "get_song_name", (1024 * 4) / sizeof(void *), NULL, PRIORITY_MEDIUM, NULL);
  vTaskStartScheduler();
}

/* -------------------------------------------------------------------------- */
/*                           TASK DEFINATION SECTION                          */
/* -------------------------------------------------------------------------- */

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

void reader_task() {
  trackname_t song_name;
  uint8_t byte_512[512];
  // binary
  while (1) {
    if (xQueueReceive(Q_trackname, song_name, portMAX_DELAY)) {
      /* -------------------------------- OPEN FILE ------------------------------- */
      UINT br;
      const char *filename = song_name;
      fprintf(stderr, "Song name is: %s\n", filename);
      FIL file; // create object file
      FRESULT result = f_open(&file, filename, (FA_READ));
      // fprintf(stderr, "Status of result and FROK %d  %d\n", result, FR_OK);
      // fprintf(stderr, "BR is: %x\n", br);
      if (FR_OK == result) {
        f_read(&file, byte_512, sizeof(byte_512), &br);
        while (br != 0) {
          f_read(&file, byte_512, sizeof(byte_512), &br);
          xQueueSend(Q_songdata, byte_512, portMAX_DELAY);
          if (uxQueueMessagesWaiting(Q_trackname)) {
            printf("New play song request\n");
            break;
          }
          // fprintf(stderr, "Does it play\n");
        }

        /* --------------------------- Auto play next song -------------------------- */
        if (br == 0) {
          xSemaphoreGive(next_song);
        }
        f_close(&file);
      } else {
        fprintf(stderr, "Failed to open the file");
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
      }
    }
  }
}

void get_current_playing_song_name() {
  while (1) {
    if (xSemaphoreTake(next_song, portMAX_DELAY)) {
      int total = total_of_songs();
      if (cursor_main >= total) {
        cursor_main = 0;
      }
      // white_Out(OLED__PAGE1, OLED_SINGLE_PAGE);
      // white_Out(OLED__PAGE0, OLED_SINGLE_PAGE);
      white_Out(OLED__PAGE6, OLED_ALL_PAGES);
      char *song = get_songs_name(cursor_main);
      xQueueSend(Q_trackname, song, portMAX_DELAY);
      display("Playing: ");
      song_name_without_dot_mp3 = remove_dot_mp3(song);
      display(song);
      // display_at_page(song_name_without_dot_mp3, OLED__PAGE5);
      // horizontal_scrolling(OLED__PAGE1);
      // white_Out(OLED__PAGE6, OLED_ALL_PAGES);
      cursor_main++;
    }
  }
}

/* -------------------------------------------------------------------------- */
/*                      INTERRUPT SERVICE ROUTINE SECTION                     */
/* -------------------------------------------------------------------------- */

void pause_isr() { xSemaphoreGiveFromISR(pause_semaphore, NULL); }
void next_song_isr() { xSemaphoreGiveFromISR(next_song, NULL); }