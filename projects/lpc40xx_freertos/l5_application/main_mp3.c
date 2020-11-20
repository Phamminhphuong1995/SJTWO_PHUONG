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
#include <stdlib.h>
#include <string.h>
#include <time.h>

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
SemaphoreHandle_t playlist;
SemaphoreHandle_t volume_down;
SemaphoreHandle_t main_menu;
SemaphoreHandle_t move_down;
SemaphoreHandle_t favorite_song;
SemaphoreHandle_t favorite_song_sender;

/* -------------------------------------------------------------------------- */
/*                                QUEUE SECTION                               */
/* -------------------------------------------------------------------------- */

QueueHandle_t Q_trackname;
QueueHandle_t Q_songdata;
QueueHandle_t Q_song_favorite;

/* -------------------------------------------------------------------------- */
/*                          TASK DECLARATION SECTION                          */
/* -------------------------------------------------------------------------- */

void reader_task();
void player_task();
void pause_task();
void pause_test();
void next_song_task();
void previous_song_task();
void playlist_control_task();
void menu_control_task();

void volume_up_task();
void volume_down_task();
void play_list_display_task();
void favorite_songs_list();
void favorite_song_sender_task();
/* -------------------------------------------------------------------------- */
/*                              UTILITY FUNCTION SECTION                      */
/* -------------------------------------------------------------------------- */

void display_current_volume();
void display_list_of_song();
void menu();
void clear_number_of_page(uint8_t number_of_page);
void populate_song_no_mp3();
void clear_cursor_and_screen();
void read_meta(char *byte_128);
void get_current_playing_song_name();
void debounce_reduce(uint8_t port, uint8_t pin);
void loading_screen();
void menu_section_task();
void play_list_navigation();
void favorite_songs_list();
void read_favorite();
void display_favorite();
void clear_favorite();
/* -------------------------------------------------------------------------- */
/*                           GLOBAL VARIABLE SECTION                          */
/* -------------------------------------------------------------------------- */

volatile bool play_pause = true;
uint16_t cursor_main = 0;
char *song_name_without_dot_mp3;
volatile bool metamp3 = true;
volatile uint8_t volume_level = 4;
char list_song_without_mp3[32][128];
uint8_t cursor_menu = 0;
volatile bool menu_switch = true;
uint8_t favorite_counter = 0;
uint8_t favorite_counter_token = 0;
uint16_t favorite[8] = {0};
/* -------------------------------------------------------------------------- */
/*                     INTERRUPT SERVICE ROUNTINE SECTION                     */
/* -------------------------------------------------------------------------- */

void pause_isr(void);
void next_song_isr(void);
void menu_control_task_isr(void);
void playlist_control_task_isr(void);
void menu_control_task_isr(void);
/* -------------------------------------------------------------------------- */
/*                             TASKHANDLE SECTION                             */
/* -------------------------------------------------------------------------- */

TaskHandle_t player_handle;
TaskHandle_t playlist_control_task_handle;
TaskHandle_t volume_up_handle;
TaskHandle_t volume_down_handle;

/* ---------------------------------- MAIN ---------------------------------- */

int main() {

  /* ----------------------- utility and initialization ----------------------- */
  srand(time(0));
  populate_list_song();
  GPIO__set_as_input(1, 15);
  GPIO__set_as_input(1, 19);
  gpio__construct_with_function(0, 6, GPIO__FUNCITON_0_IO_PIN);
  gpio__construct_with_function(0, 8, GPIO__FUNCITON_0_IO_PIN);
  fprintf(stderr, "\ntotal song: %d\n", total_of_songs());
  mp3_init();
  turn_on_oled();
  clear();
  update();
  populate_song_no_mp3();
  loading_screen();
  // display_list_of_song();
  menu();
  fprintf(stderr, "Calling\n");
  // read_favorite();
  sj2_cli__init();

  /* --------------------------- Queue and Semaphore -------------------------- */

  Q_trackname = xQueueCreate(1, sizeof(trackname_t));
  Q_songdata = xQueueCreate(1, 512);
  Q_song_favorite = xQueueCreate(1, sizeof(uint16_t));
  pause_semaphore = xSemaphoreCreateBinary();
  next_song = xSemaphoreCreateBinary();
  previous_song = xSemaphoreCreateBinary();
  main_menu = xSemaphoreCreateBinary();
  move_down = xSemaphoreCreateBinary();
  playlist = xSemaphoreCreateBinary();
  favorite_song = xSemaphoreCreateMutex();
  favorite_song_sender = xSemaphoreCreateBinary();

  /* -------------------------------- Interrupt ------------------------------- */

  lpc_peripheral__enable_interrupt(LPC_PERIPHERAL__GPIO, gpio0__interrupt_dispatcher, "isr for port 0");
  gpio0__attach_interrupt(30, GPIO_INTR__FALLING_EDGE, pause_isr);
  gpio0__attach_interrupt(29, GPIO_INTR__FALLING_EDGE, menu_control_task_isr);
  gpio0__attach_interrupt(6, GPIO_INTR__FALLING_EDGE, next_song_isr);
  gpio0__attach_interrupt(8, GPIO_INTR__FALLING_EDGE, playlist_control_task_isr);

  /* ------------------------------ Task creation ----------------------------- */

  xTaskCreate(reader_task, "reader", (2024 * 4) / sizeof(void *), NULL, PRIORITY_MEDIUM, NULL);
  xTaskCreate(favorite_songs_list, "favorite_songs_list", (2024 * 4) / sizeof(void *), NULL, PRIORITY_HIGH, NULL);
  xTaskCreate(player_task, "player", (3096 * 4) / sizeof(void *), NULL, PRIORITY_MEDIUM, &player_handle);
  xTaskCreate(pause_task, "pause", (1024 * 4) / sizeof(void *), NULL, PRIORITY_MEDIUM, NULL);

  xTaskCreate(previous_song_task, "previous_song", (1024 * 2) / sizeof(void *), NULL, PRIORITY_MEDIUM, NULL);
  xTaskCreate(next_song_task, "next_song", (1024 * 2) / sizeof(void *), NULL, PRIORITY_MEDIUM, NULL);
  xTaskCreate(playlist_control_task, "playlist_control", (1024 * 2) / sizeof(void *), NULL, PRIORITY_MEDIUM,
              &playlist_control_task_handle);

  xTaskCreate(menu_control_task, "menu", (1024 * 2) / sizeof(void *), NULL, PRIORITY_MEDIUM, NULL);
  xTaskCreate(volume_up_task, "volume up", (1024) / sizeof(void *), NULL, PRIORITY_MEDIUM, &volume_up_handle);
  xTaskCreate(volume_down_task, "volume down", (1024) / sizeof(void *), NULL, PRIORITY_MEDIUM, &volume_down_handle);

  xTaskCreate(play_list_display_task, "display playlist", (1024) / sizeof(void *), NULL, PRIORITY_MEDIUM, NULL);
  xTaskCreate(favorite_song_sender_task, "sender fav", (1024) / sizeof(void *), NULL, PRIORITY_MEDIUM, NULL);
  vTaskSuspend(volume_up_handle);
  vTaskSuspend(volume_down_handle);
  vTaskStartScheduler();
}

/* -------------------------------------------------------------------------- */
/*                           TASK DEFINATION SECTION                          */
/* -------------------------------------------------------------------------- */

void pause_task() {
  while (1) {
    if (xSemaphoreTake(pause_semaphore, portMAX_DELAY)) {
      if (GPIO__get_level(1, 19)) {
        display_at_page("Loved", OLED__PAGE6);
        // favorite_counter_token++;
        horizontal_scrolling(OLED__PAGE1);
        xSemaphoreGive(favorite_song_sender);
      } else {
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
}

void reader_task() {
  trackname_t song_name;
  uint8_t byte_512[512];
  UINT br; // binary
  while (1) {
    if (xQueueReceive(Q_trackname, song_name, portMAX_DELAY)) {
      white_Out(OLED__PAGE0, OLED_SINGLE_PAGE);
      display_at_page("Playing...", OLED__PAGE0);
      /* -------------------------------- OPEN FILE ------------------------------- */
      const char *filename = song_name;
      FIL file; // create object file
      FRESULT result = f_open(&file, filename, (FA_READ));

      /* ----------------------------- READ META_DATA ----------------------------- */
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
      while (!GPIO__get_level(2, 0)) {
        vTaskDelay(1); // waiting for DREQ
      }
      send_data_to_decoder(byte_512[i]);
    }
  }
}

void next_song_task() {
  while (1) {
    if (xSemaphoreTake(next_song, portMAX_DELAY)) {
      vTaskResume(player_handle);
      vTaskResume(volume_up_handle);
      vTaskResume(volume_down_handle);
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
      vTaskDelay(100);
    }
  }
}

void previous_song_task() {
  while (1) {
    if (xSemaphoreTake(previous_song, portMAX_DELAY)) {
      vTaskResume(player_handle);
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
    if (GPIO__get_level(1, 15)) {
      if (volume_level == 10) {
        volume_level = 0;
      }
      volume_level++;
      set_Volume(volume_level);
      display_current_volume();
      vTaskDelay(1000);
      srand(time(0));
      white_Out(OLED__PAGE7, OLED_SINGLE_PAGE);
      horizontal_scrolling(OLED__PAGE1);
    }
    vTaskDelay(750);
  }
}

void volume_down_task() {
  while (1) {
    if (GPIO__get_level(1, 19)) {
      volume_level--;
      if (volume_level == 0) {
        volume_level = 9;
      }
      set_Volume(volume_level);
      display_current_volume();
      vTaskDelay(1000);
      white_Out(OLED__PAGE7, OLED_SINGLE_PAGE);
      horizontal_scrolling(OLED__PAGE1);
    }
    vTaskDelay(750);
  }
}

uint8_t cursor_for_scrolling = 0;
void playlist_control_task() {
  while (1) {
    if (xSemaphoreTake(move_down, portMAX_DELAY)) {
      if (menu_switch) {
        if (cursor_menu > 4) {
          cursor_menu = 0;
        }
        horizontal_scrolling(cursor_menu);
        menu_section_task();
      } else {
        if (cursor_for_scrolling == 8) {
          cursor_for_scrolling = 0;
          white_Out(0, OLED_ALL_PAGES);
          display_list_of_song();
        }
        play_list_navigation();
      }
    }
  }
}

void menu_control_task() {
  while (1) {
    if (xSemaphoreTake(main_menu, portMAX_DELAY)) {
      vTaskSuspend(volume_up_handle);
      vTaskSuspend(volume_down_handle);
      white_Out(0, OLED_ALL_PAGES);
      menu();
      cursor_menu = 0;
      menu_switch = true;
    }
  }
}

void play_list_display_task() {
  while (1) {
    if (xSemaphoreTake(playlist, portMAX_DELAY)) {
      clear_cursor_and_screen();
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

void read_meta(char *byte_128) {
  white_Out(OLED__PAGE1, OLED_SINGLE_PAGE);
  white_Out(OLED__PAGE2, OLED_SINGLE_PAGE);
  white_Out(OLED__PAGE3, OLED_SINGLE_PAGE);
  white_Out(OLED__PAGE4, OLED_SINGLE_PAGE);
  white_Out(OLED__PAGE5, OLED_SINGLE_PAGE);
  white_Out(OLED__PAGE6, OLED_SINGLE_PAGE);
  white_Out(OLED__PAGE7, OLED_SINGLE_PAGE);
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
  if (strlen(meta_data_mp3.Title) < 1) {
    char *song = get_songs_name(cursor_main - 1); // index go back by 1
    song_name_without_dot_mp3 = remove_dot_mp3(song);
    display_at_page(song_name_without_dot_mp3, OLED__PAGE1);
  } else {
    display_at_page(meta_data_mp3.Title, OLED__PAGE1);
  }
  display_at_page(meta_data_mp3.Artist, OLED__PAGE2);
  display_at_page(genre_decoder(meta_data_mp3.genre), OLED__PAGE3);
  display_at_page(meta_data_mp3.Year, OLED__PAGE4);
  horizontal_scrolling(OLED__PAGE1);
}

void display_current_volume() {
  white_Out(OLED__PAGE7, OLED_SINGLE_PAGE);
  display_at_page("Vol: ", OLED__PAGE7);
  for (int i = 0; i < volume_level; i++) {
    display("=");
  }
}

void display_list_of_song() {

  uint8_t oled_page_counter = 0;
  for (int i = cursor_main; i < cursor_main + 8; i++) {
    if (i == total_of_songs()) {
      break;
    }
    char *song = list_song_without_mp3[i];
    display_at_page(song, oled_page_counter);
    oled_page_counter++;
  }
}

void populate_song_no_mp3() {
  for (int i = 0; i < total_of_songs(); i++) {
    char *song = get_songs_name(i);
    song_name_without_dot_mp3 = remove_dot_mp3(song);
    strncpy(list_song_without_mp3[i], song_name_without_dot_mp3, 127);
  }
}

void clear_number_of_page(uint8_t number_of_page) {
  for (int i = 0; i < number_of_page; i++) {
    white_Out(i, OLED_SINGLE_PAGE);
  }
}

void clear_cursor_and_screen() {
  cursor_main = 0;
  cursor_for_scrolling = 0;
  clear_number_of_page(8);
  display_list_of_song();
}

void menu() {
  display_at_page("List song", OLED__PAGE0);
  display_at_page("Random song", OLED__PAGE1);
  display_at_page("Favorite songs", OLED__PAGE2);
  display_at_page("Clean Fav", OLED__PAGE3);
}

void menu_section_task() {
  if (GPIO__get_level(1, 19) && cursor_menu == 1) {
    vTaskSuspend(volume_up_handle);
    vTaskSuspend(volume_down_handle);
    clear_cursor_and_screen();
    display_list_of_song();
    menu_switch = false;
  }
  if (GPIO__get_level(1, 19) && cursor_menu == 2) {
    int rand_number;
    white_Out(0, OLED_ALL_PAGES);
    rand_number = rand() % total_of_songs();
    cursor_main = rand_number;
    vTaskResume(volume_up_handle);
    vTaskResume(volume_down_handle);
    xSemaphoreGive(next_song);
    menu_switch = false;
  }
  // TODO: working on list of favorite songs
  if (GPIO__get_level(1, 19) && cursor_menu == 3) {
    vTaskSuspend(volume_up_handle);
    vTaskSuspend(volume_down_handle);
    white_Out(0, OLED_ALL_PAGES);
    read_favorite();
    display_favorite();
    menu_switch = false;
  }
  if (GPIO__get_level(1, 19) && cursor_menu == 4) {
    vTaskSuspend(volume_up_handle);
    vTaskSuspend(volume_down_handle);
    white_Out(0, OLED_ALL_PAGES);
    display("Clearing favorite");
    clear_favorite();
    read_favorite();
    menu_switch = false;
  }
  cursor_menu++;
}
// FIXME

void read_favorite() {
  fprintf(stderr, "checking read\n");
  UINT br; // binary
  const char *filename = "favorite.txt";
  FIL file; // create object file
  FRESULT result = f_open(&file, filename, (FA_OPEN_EXISTING | FA_READ));
  char index[128] = {0};
  if (FR_OK == result) {
    f_read(&file, index, sizeof(index), &br);
    while (br != 0) {
      f_read(&file, index, sizeof(index), &br);
      f_close(&file);
    }
  } else {
    fprintf(stderr, "failed at read_favor");
  }
  fprintf(stderr, "data: %c\n", index[0]);
  char *token;
  int index_of_favorite_song = 0;
  favorite_counter_token = 0;
  token = strtok(index, " ");
  while (token != NULL) {
    favorite[index_of_favorite_song] = atoi(token);
    fprintf(stderr, "%d: ", favorite[index_of_favorite_song]);
    token = strtok(NULL, " ");
    index_of_favorite_song++;
    favorite_counter_token++;
  }
}

void favorite_song_sender_task() {
  uint16_t data;
  while (1) {
    if (xSemaphoreTake(favorite_song_sender, portMAX_DELAY)) {
      data = cursor_main - 1;
      xQueueSend(Q_song_favorite, &data, 0);
    }
  }
}

void favorite_songs_list() {
  char string[8];
  uint16_t index_song;
  const char *filename = "favorite.txt";
  FIL file; // File handle
  UINT bytes_written = 0;
  FRESULT result = f_open(&file, filename, (FA_OPEN_APPEND | FA_WRITE));
  while (1) {
    if (xQueueReceive(Q_song_favorite, &index_song, portMAX_DELAY)) {
      if (FR_OK == result) {
        sprintf(string, "%d ", index_song);
        if (FR_OK == f_write(&file, string, strlen(string), &bytes_written)) {
        } else {
          fprintf(stderr, "ERROR: Failed to write data to file\n");
        }
        f_sync(&file);
      } else {
        fprintf(stderr, "ERROR: Failed to open: %s\n", filename);
      }
    }
    // f_close(&file);
  }
}
void play_list_navigation() {
  if (cursor_main == total_of_songs()) {
    clear_cursor_and_screen();
  }
  horizontal_scrolling(cursor_for_scrolling);
  if (GPIO__get_level(1, 19)) {
    /**
     * cause this cursor will be move down again when we press the button combination
     * a hacky way is to deduct by 1 to move back to our intended location
     * */
    cursor_main--;
    char *song = get_songs_name(cursor_main);
    vTaskResume(volume_down_handle);
    vTaskResume(volume_up_handle);
    xQueueSend(Q_trackname, song, portMAX_DELAY);
  }
  /* -------------------------------------------------------------------------- */
  cursor_for_scrolling++;
  cursor_main++;

  if (GPIO__get_level(1, 15)) {
    vTaskSuspend(volume_up_handle);
    vTaskSuspend(volume_down_handle);
    clear_cursor_and_screen();
  }
}

void debounce_reduce(uint8_t port, uint8_t pin) {
  uint8_t delay = 0;
  while (GPIO__get_level(port, pin)) {
    delay += 10;
  }
  while (!GPIO__get_level(port, pin) && (delay != 0)) {
    delay--;
  }
}

void loading_screen() {
  display("Initializing....\n");
  for (int i = 0; i < 10; i++) {
    display("......");
    delay__ms(100);
  }
  delay__ms(1000);
  white_Out(0, OLED_ALL_PAGES);
}

void display_favorite() {
  uint8_t oled_page_counter = 0;
  for (int i = 0; i < favorite_counter_token; i++) {
    char *song = list_song_without_mp3[favorite[i]];
    fprintf(stderr, "\n%s\n", song);
    display_at_page(song, oled_page_counter);
    oled_page_counter++;
  }
}

void clear_favorite() {
  const char *filename = "favorite.txt";
  FIL file; // create object file
  FRESULT result = f_open(&file, filename, FA_CREATE_ALWAYS);
  f_close(&file);
  memset(favorite, 0, sizeof(favorite));
}

/* -------------------------------------------------------------------------- */
/*                      INTERRUPT SERVICE ROUTINE SECTION                     */
/* -------------------------------------------------------------------------- */

void pause_isr() {
  debounce_reduce(0, 30);
  xSemaphoreGiveFromISR(pause_semaphore, NULL);
}
void next_song_isr() { xSemaphoreGiveFromISR(next_song, NULL); }
void menu_control_task_isr() {
  debounce_reduce(0, 29);
  xSemaphoreGiveFromISR(main_menu, NULL);
}
void playlist_control_task_isr() { xSemaphoreGiveFromISR(move_down, NULL); }