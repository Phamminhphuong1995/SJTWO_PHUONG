#include "gpio_lab.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
void turn_off_annoy_4_red_leds() {
  GPIO__set_high(1, 18);
  GPIO__set_high(1, 24);
  GPIO__set_high(1, 26);
  GPIO__set_high(2, 3);
}

/* -------------------------------------------------------------------------- */
/*                                MENU SECTION                                */
/* -------------------------------------------------------------------------- */

static char menu_array[8][128];
void set_menu_array() {
  strncpy(menu_array[0], "List Song", 127);
  strncpy(menu_array[1], "Random", 127);
  strncpy(menu_array[2], "Favorite", 127);
  strncpy(menu_array[3], "Clear Favorite", 127);
  strncpy(menu_array[4], "Current Play", 127);
}

char *get_menu_from_menu_array(uint8_t choice) {
  char *menu = menu_array[choice];
  return menu;
}

static bool menu = true;
bool is_menu_pressed() { return menu; }

void set_menu_signal(bool menu_setter) { menu = menu_setter; }

/* -------------------------------------------------------------------------- */
/*                              LIST SONG SECTION                             */
/* -------------------------------------------------------------------------- */

static bool list_song_favorite = true;
bool are_we_in_list_of_favorite() { return list_song_favorite; }

void set_list_favorite(bool favorite_setter) { list_song_favorite = favorite_setter; }

/* -------------------------------------------------------------------------- */
/*                           CURRENT PLAYING SECTION                          */
/* -------------------------------------------------------------------------- */

static bool current_playing = true;
bool are_we_in_current_playing() { return current_playing; }

void set_current_play_signal(bool current_play_setter) { current_playing = current_play_setter; }