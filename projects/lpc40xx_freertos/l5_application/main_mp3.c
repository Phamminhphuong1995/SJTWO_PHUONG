#include "FreeRTOS.h"
#include "cli_handlers.h"
#include "decoder.h"
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
  char Year[4];
  uint8_t genre;
} mp3_meta_data;
/* -------------------------------------------------------------------------- */
/*                              SEMAPHORE SECTION                             */
/* -------------------------------------------------------------------------- */

SemaphoreHandle_t pause_semaphore;
SemaphoreHandle_t next_song;
SemaphoreHandle_t previous_song;
SemaphoreHandle_t get_current_song_name;
SemaphoreHandle_t volume_up;
SemaphoreHandle_t volume_down;

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
void volume_up_task();
void volume_down_task();

/* -------------------------------------------------------------------------- */
/*                           GLOBAL VARIABLE SECTION                          */
/* -------------------------------------------------------------------------- */

volatile bool play_pause = true;
uint16_t cursor_main = 0;
char *song_name_without_dot_mp3;
volatile bool metamp3 = true;
volatile uint8_t volume_level = 4;

/* -------------------------------------------------------------------------- */
/*                     INTERRUPT SERVICE ROUNTINE SECTION                     */
/* -------------------------------------------------------------------------- */

void pause_isr(void);
void next_song_isr(void);
void volume_control_isr(void);
/* -------------------------------------------------------------------------- */
/*                             TASKHANDLE SECTION                             */
/* -------------------------------------------------------------------------- */

TaskHandle_t player_handle;

/* ---------------------------------- MAIN ---------------------------------- */

int main() {

  /* ----------------------- utility and initialization ----------------------- */
  populate_list_song();
  GPIO__set_as_input(1, 15);
  gpio__construct_with_function(0, 6, GPIO__FUNCITON_0_IO_PIN);
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
  volume_up = xSemaphoreCreateBinary();
  volume_down = xSemaphoreCreateBinary();

  /* -------------------------------- Interrupt ------------------------------- */

  lpc_peripheral__enable_interrupt(LPC_PERIPHERAL__GPIO, gpio0__interrupt_dispatcher, "isr for port 0");
  gpio0__attach_interrupt(30, GPIO_INTR__FALLING_EDGE, pause_isr);
  gpio0__attach_interrupt(29, GPIO_INTR__FALLING_EDGE, next_song_isr);
  gpio0__attach_interrupt(6, GPIO_INTR__FALLING_EDGE, volume_control_isr);

  /* ------------------------------ Task creation ----------------------------- */

  xTaskCreate(reader_task, "reader", (2024 * 4) / sizeof(void *), NULL, PRIORITY_MEDIUM, NULL);
  xTaskCreate(player_task, "player", (3096 * 4) / sizeof(void *), NULL, PRIORITY_MEDIUM, &player_handle);
  xTaskCreate(pause_task, "pause", (1024 * 4) / sizeof(void *), NULL, PRIORITY_MEDIUM, NULL);
  xTaskCreate(previous_song_task, "previous_song", (1024 * 2) / sizeof(void *), NULL, PRIORITY_MEDIUM, NULL);
  xTaskCreate(next_song_task, "next_song", (1024 * 2) / sizeof(void *), NULL, PRIORITY_MEDIUM, NULL);
  xTaskCreate(volume_up_task, "volume up", (1024 * 2) / sizeof(void *), NULL, PRIORITY_MEDIUM, NULL);
  xTaskCreate(volume_down_task, "volume down", (1024 * 2) / sizeof(void *), NULL, PRIORITY_MEDIUM, NULL);
  vTaskStartScheduler();
}

/* -------------------------------------------------------------------------- */
/*                           TASK DEFINATION SECTION                          */
/* -------------------------------------------------------------------------- */

void pause_task() {
  while (1) {
    if (xSemaphoreTake(pause_semaphore, portMAX_DELAY)) {
      if (play_pause) {
        white_Out(OLED__PAGE0, OLED_SINGLE_PAGE);
        display_at_page("Paused", OLED__PAGE0);
        vTaskSuspend(player_handle);
        deactivate_horizontal_scrolling();
        play_pause = false;
      } else {
        white_Out(OLED__PAGE0, OLED_SINGLE_PAGE);
        display_at_page("Playing...", OLED__PAGE0);
        vTaskResume(player_handle);
        horizontal_scrolling(OLED__PAGE1);
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
    xQueueReceive(Q_songdata, byte_512, portMAX_DELAY);
    for (int i = 0; i < 512; i++) {
      while (!get_DREQ_HighActive()) {
        vTaskDelay(1); // waiting for DREQ
      }
      decoder_send_mp3Data(byte_512[i]);
    }
  }
}
volatile bool debounce = true;
void next_song_task() {
  while (1) {
    if (xSemaphoreTake(next_song, portMAX_DELAY)) {
      if (!GPIO__get_level(1, 15)) {
        metamp3 = true;
        int total = total_of_songs();
        if (cursor_main == total) {
          cursor_main = 0;
        }
        char *song = get_songs_name(cursor_main);
        cursor_main++;
        xQueueSend(Q_trackname, song, portMAX_DELAY);

        white_Out(OLED__PAGE0, OLED_SINGLE_PAGE);
        display_at_page("Playing...", OLED__PAGE0);
      } else {
        white_Out(OLED__PAGE0, OLED_SINGLE_PAGE);
        display_at_page("Playing...", OLED__PAGE0);
        xSemaphoreGive(previous_song);
      }
      vTaskDelay(1000);
    }
  }
}

void previous_song_task() {
  while (1) {
    if (xSemaphoreTake(previous_song, portMAX_DELAY)) {
      metamp3 = true;
      int total = total_of_songs();
      if (cursor_main == 0) {
        cursor_main = total;
      }
      cursor_main--;
      char *song = get_songs_name(cursor_main);
      white_Out(OLED__PAGE0, OLED_SINGLE_PAGE);
      display_at_page("Playing...", OLED__PAGE0);

      xQueueSend(Q_trackname, song, portMAX_DELAY);
    }
    vTaskDelay(100);
  }
}

void volume_up_task() {
  while (1) {
    if (xSemaphoreTake(volume_up, portMAX_DELAY)) {
      if (!GPIO__get_level(1, 15)) {
        volume_level++;
        if (volume_level == 10) {
          volume_level = 0;
        }
        set_Volume(volume_level);
        white_Out(OLED__PAGE7, OLED_SINGLE_PAGE);
        display_at_page("Vol: ", OLED__PAGE7);
        for (int i = 0; i < volume_level; i++) {
          display("=");
        }
        horizontal_scrolling(OLED__PAGE1);
      } else {
        xSemaphoreGive(volume_down);
      }
    }
  }
}

void volume_down_task() {
  while (1) {
    if (xSemaphoreTake(volume_down, portMAX_DELAY)) {
      volume_level--;
      if (volume_level == 0) {
        volume_level = 9;
      }
      set_Volume(volume_level);
      white_Out(OLED__PAGE7, OLED_SINGLE_PAGE);
      display_at_page("Vol: ", OLED__PAGE7);
      for (int i = 0; i < volume_level; i++) {
        display("=");
      }
      horizontal_scrolling(OLED__PAGE1);
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

  white_Out(OLED__PAGE1, OLED_SINGLE_PAGE);
  white_Out(OLED__PAGE2, OLED_SINGLE_PAGE);
  white_Out(OLED__PAGE3, OLED_SINGLE_PAGE);
  white_Out(OLED__PAGE4, OLED_SINGLE_PAGE);
  mp3_meta_data meta_data_mp3 = {0};
  for (int i = 0; i < 128; i++) {
    if ((((int)(byte_128[i]) > 47) && ((int)(byte_128[i]) < 58)) ||
        (((int)(byte_128[i]) > 64) && ((int)(byte_128[i]) < 91)) ||
        (((int)(byte_128[i]) > 96) && ((int)(byte_128[i]) < 123)) || ((int)(byte_128[i])) == 32) {
      char c = (int)(byte_128[i]);
      if (i < 3) {
        meta_data_mp3.Tag[i] = c;
      } else if (i > 2 && i < 33) {
        meta_data_mp3.Title[i - 3] = c;
      } else if (i > 32 && i < 63) {
        meta_data_mp3.Artist[i - 33] = c;
      } else if (i > 62 && i < 93) {
        meta_data_mp3.Album[i - 63] = c;
      } else if (i > 92 && i < 97) {
        meta_data_mp3.Year[i - 93] = c;
      } else if (i == 127) {
        meta_data_mp3.genre = (int)(byte_128[i]);
      }
    }
  }
  display_at_page(meta_data_mp3.Title, OLED__PAGE1);
  display_at_page(meta_data_mp3.Artist, OLED__PAGE2);
  display_at_page(genre_decoder(meta_data_mp3.genre), OLED__PAGE3);
  display_at_page(meta_data_mp3.Year, OLED__PAGE4);
  horizontal_scrolling(OLED__PAGE1);
}
/* -------------------------------------------------------------------------- */
/*                      INTERRUPT SERVICE ROUTINE SECTION                     */
/* -------------------------------------------------------------------------- */

void pause_isr() { xSemaphoreGiveFromISR(pause_semaphore, NULL); }
void next_song_isr() { xSemaphoreGiveFromISR(next_song, NULL); }
void volume_control_isr() { xSemaphoreGiveFromISR(volume_up, NULL); }