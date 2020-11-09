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

typedef struct {
  char Tag[3];
  char Title[30];
  char Artist[30];
  char Album[30];
  uint8_t genre;
} mp3_meta_data;
/* -------------------------------------------------------------------------- */
/*                              SEMAPHORE SECTION                             */
/* -------------------------------------------------------------------------- */

SemaphoreHandle_t pause_semaphore;
SemaphoreHandle_t pause_semaphore_test;
SemaphoreHandle_t next_song;
SemaphoreHandle_t previous_song;
SemaphoreHandle_t get_current_song_name;

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
void pause_test();
void get_current_playing_song_name();
void next_song_task();
void previous_song_task();
void read_meta(char *byte_128);

/* -------------------------------------------------------------------------- */
/*                           GLOBAL VARIABLE SECTION                          */
/* -------------------------------------------------------------------------- */

volatile bool play_pause = true;
uint16_t cursor_main = 0;
char *song_name_without_dot_mp3;
volatile bool metamp3 = true;

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
  fprintf(stderr, "\ntotal song: %d\n", total_of_songs());
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
  previous_song = xSemaphoreCreateBinary();

  /* -------------------------------- Interrupt ------------------------------- */

  lpc_peripheral__enable_interrupt(LPC_PERIPHERAL__GPIO, gpio0__interrupt_dispatcher, "isr for port 0");
  gpio0__attach_interrupt(30, GPIO_INTR__FALLING_EDGE, pause_isr);
  gpio0__attach_interrupt(29, GPIO_INTR__FALLING_EDGE, next_song_isr);

  /* ------------------------------ Task creation ----------------------------- */

  xTaskCreate(reader_task, "reader", (2024 * 4) / sizeof(void *), NULL, PRIORITY_MEDIUM, NULL);
  xTaskCreate(player_task, "player", (3096 * 4) / sizeof(void *), NULL, PRIORITY_MEDIUM, &player_handle);
  xTaskCreate(pause_task, "pause", (1024 * 4) / sizeof(void *), NULL, PRIORITY_MEDIUM, NULL);
  xTaskCreate(previous_song_task, "get_song_name", (1024 * 2) / sizeof(void *), NULL, PRIORITY_MEDIUM, NULL);
  xTaskCreate(next_song_task, "get_song_name", (1024 * 2) / sizeof(void *), NULL, PRIORITY_MEDIUM, NULL);
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
  UINT br; // binary
  while (1) {
    if (xQueueReceive(Q_trackname, song_name, portMAX_DELAY)) {
      /* -------------------------------- OPEN FILE ------------------------------- */
      const char *filename = song_name;
      FIL file; // create object file
      FRESULT result = f_open(&file, filename, (FA_READ));

      /* ----------------------------- READ META_DATA ----------------------------- */
      // FIXME
      char byte_128[128];
      f_lseek(&file, f_size(&file) - (sizeof(char) * 128));
      f_read(&file, byte_128, sizeof(byte_128), &br);
      fprintf(stderr, "Song name: %s\n", filename);
      read_meta(byte_128);
      f_lseek(&file, 0);
      /* ----------------------------- READ SONG_DATA ----------------------------- */

      if (FR_OK == result) {
        f_read(&file, byte_512, sizeof(byte_512), &br);
        while (br != 0) {
          f_read(&file, byte_512, sizeof(byte_512), &br);
          xQueueSend(Q_songdata, byte_512, portMAX_DELAY);
          if (uxQueueMessagesWaiting(Q_trackname)) {
            printf("New play song request\n");
            break;
          }
        }
        /* --------------------------- Auto play next song -------------------------- */
        if (br == 0) {
          metamp3 = true; // read meta
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

void next_song_task() {
  while (1) {
    if (xSemaphoreTake(next_song, portMAX_DELAY)) {
      vTaskDelay(100);
      metamp3 = true;
      int total = total_of_songs();
      if (cursor_main == total) {
        cursor_main = 0;
      }
      char *song = get_songs_name(cursor_main);
      get_current_playing_song_name();
      fprintf(stderr, "before next %d\n", cursor_main);
      cursor_main++;
      fprintf(stderr, "after next %d\n", cursor_main);
      xQueueSend(Q_trackname, song, portMAX_DELAY);
    }
  }
}

void previous_song_task() {
  while (1) {
    if (xSemaphoreTake(previous_song, portMAX_DELAY)) {
      vTaskDelay(100);
      metamp3 = true;
      int total = total_of_songs();
      if (cursor_main == 0) {
        cursor_main = total;
      }
      fprintf(stderr, "before previous %d\n", cursor_main);
      cursor_main--;
      fprintf(stderr, "after previous %d\n", cursor_main);
      char *song = get_songs_name(cursor_main);
      get_current_playing_song_name();

      xQueueSend(Q_trackname, song, portMAX_DELAY);
    }
  }
}

/* -------------------------------------------------------------------------- */
/*                                UTILITY TASK                                */
/* -------------------------------------------------------------------------- */

void get_current_playing_song_name() {
  white_Out(OLED__PAGE7, OLED_SINGLE_PAGE);
  char *song = get_songs_name(cursor_main);
  song_name_without_dot_mp3 = remove_dot_mp3(song);
  display_at_page(song_name_without_dot_mp3, OLED__PAGE7);
}

/**
 * FIXME
 */
void read_meta(char *byte_128) {
  white_Out(OLED__PAGE0, OLED_SINGLE_PAGE);
  white_Out(OLED__PAGE1, OLED_SINGLE_PAGE);
  white_Out(OLED__PAGE2, OLED_SINGLE_PAGE);
  white_Out(OLED__PAGE3, OLED_SINGLE_PAGE);
  white_Out(OLED__PAGE4, OLED_SINGLE_PAGE);
  white_Out(OLED__PAGE5, OLED_SINGLE_PAGE);
  white_Out(OLED__PAGE6, OLED_SINGLE_PAGE);
  int meta_index = 0;
  char meta_data[128] = {"0"};
  mp3_meta_data meta_data_mp3 = {0};
  for (int i = 0; i < 128; i++) {
    if ((((int)(byte_128[i]) > 47) && ((int)(byte_128[i]) < 58)) ||
        (((int)(byte_128[i]) > 64) && ((int)(byte_128[i]) < 91)) ||
        (((int)(byte_128[i]) > 96) && ((int)(byte_128[i]) < 123)) || ((int)(byte_128[i])) == 32) {
      char c = (int)(byte_128[i]);
      if (i < 3) {
        meta_data_mp3.Tag[i] = c;
      } else if (i > 2 && i < 33) {
        meta_data_mp3.Artist[i - 3] = c;
      } else if (i > 32 && i < 63) {
        meta_data_mp3.Album[i - 33] = c;
      } else if (i == 127) {
        meta_data_mp3.genre = (int)(byte_128[i]);
      }
      meta_data[meta_index] = c;
      meta_index++;
    }
  }
  display_at_page(meta_data_mp3.Tag, OLED__PAGE1);
  display_at_page(meta_data_mp3.Artist, OLED__PAGE2);
  display_at_page(meta_data_mp3.Album, OLED__PAGE3);
  display_at_page(genre_decoder(meta_data_mp3.genre), OLED__PAGE4);
  horizontal_scrolling(OLED__PAGE7);
}
/* -------------------------------------------------------------------------- */
/*                      INTERRUPT SERVICE ROUTINE SECTION                     */
/* -------------------------------------------------------------------------- */

void pause_isr() { xSemaphoreGiveFromISR(previous_song, NULL); }
void next_song_isr() { xSemaphoreGiveFromISR(next_song, NULL); }