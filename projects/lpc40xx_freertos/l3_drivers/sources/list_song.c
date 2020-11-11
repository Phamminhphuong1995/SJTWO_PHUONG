#include "list_song.h"

typedef char song_storage_location[128];

static song_storage_location list_song[32];
static uint16_t total_song;

static void song_list__handle_filename(const char *filename) {
  if (NULL != strstr(filename, ".mp3")) {
    strncpy(list_song[total_song], filename, sizeof(song_storage_location) - 1);
    ++total_song;
  }
}

void populate_list_song() {
  FRESULT res;
  static FILINFO file_info;
  const char *root_path = "/";

  DIR dir;
  res = f_opendir(&dir, root_path);

  if (res == FR_OK) {
    for (;;) {
      res = f_readdir(&dir, &file_info);
      if (res != FR_OK || file_info.fname[0] == 0) {
        break;
      }

      if (file_info.fattrib & AM_DIR) {
      } else {
        song_list__handle_filename(file_info.fname);
      }
    }
    f_closedir(&dir);
  }
}

uint16_t total_of_songs() { return total_song; }

char *get_songs_name(uint16_t index) {
  char *song_name = "";
  if (index >= total_song) {
    song_name = "";
  } else {
    song_name = list_song[index];
  }
  return song_name;
}