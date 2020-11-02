#include "FreeRTOS.h"
#include "SSD1306.h"
#include "acceleration.h"
#include "cli_handlers.h"
#include "clock.h"
#include "delay.h"
#include "event_groups.h"
#include "ff.h"
#include "gpio.h"
#include "gpio_isr.h"
#include "gpio_lab.h"
#include "i2c2_slave_functions.h"
#include "i2c_slave_init.h"
#include "lpc40xx.h"
#include "lpc_peripherals.h"
#include "mp3.h"
#include "queue.h"
#include "sj2_cli.h"
#include "song_list.h"
#include "task.h"
#include "uart_printf.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// tasks
void mp3_reader_task(void *p);
void mp3_player_task(void *p);
void volumeup_task(void *p);
void volumedwn_task(void *p);
void select_song_task(void *p);
void pass_song_name_task(void *p);
void accelerometer_bass_treble_control(void *p);
void mp3_screen_control_task(void *p);
void mp3_screen_control_task2(void *p);
void bass_down_task(void *p);
void bass_up_task(void *p);
void test_task(void *p);
void treble_up_task(void *p);
void treble_down_task(void *p);
void pause_task(void *p);
void menu_task(void *p);

void lcd_menu_switch_init();
void print_lcd_screen(int i);
void print_menu_screen();

// isrs
void volumedown_isr(void);
void volumeup_isr(void);
void pass_song_isr(void); // step 1 declare isr
void move_up_isr(void);
void move_down_isr(void); // in case we split in 2
void bass_up_isr(void);
void bass_down_isr(void);
void treble_up_isr(void);
void treble_down_isr(void);
void pause_isr(void);
void menu_isr(void);
void accelerometer_isr(void);

void setup_volume_ctrl_sws();
void treble_bass_switch_init();

// global variable
volatile int cursor = 0;
volatile uint8_t bass_level = 1;
volatile uint8_t treble_level = 1;
volatile uint8_t current_track_number = 0;
volatile bool play = true;
// volatile bool play =
// volatile int song_index = 0; //in case we split the screen control function in 2

QueueHandle_t Q_songdata;
QueueHandle_t Q_trackname;
SemaphoreHandle_t volumeup_semaphore;
SemaphoreHandle_t volumedwn_semaphore;
SemaphoreHandle_t pass_song_semaphore; // step 2 declare semaphore handle
SemaphoreHandle_t move_up_semaphore;
SemaphoreHandle_t move_down_semaphore;
SemaphoreHandle_t bass_up_semaphore;
SemaphoreHandle_t bass_down_semaphore;
SemaphoreHandle_t treble_up_semaphore;
SemaphoreHandle_t treble_down_semaphore;
SemaphoreHandle_t pause_semaphore;
SemaphoreHandle_t accelerometer_semaphore;
SemaphoreHandle_t menu_semaphore;

TaskHandle_t player_handle;

int main(void) {

  setup_volume_ctrl_sws();
  treble_bass_switch_init();
  song_list__populate();
  delay__ms(10);
  SSD1306_Init();
  print_lcd_screen(0);

  Q_trackname = xQueueCreate(1, sizeof(trackname_t));
  Q_songdata = xQueueCreate(1, 512);
  volumedwn_semaphore = xSemaphoreCreateBinary();
  volumeup_semaphore = xSemaphoreCreateBinary();
  pass_song_semaphore = xSemaphoreCreateBinary(); // step 3 create semaphore binary
  move_up_semaphore = xSemaphoreCreateBinary();
  move_down_semaphore = xSemaphoreCreateBinary();
  bass_up_semaphore = xSemaphoreCreateBinary();
  bass_down_semaphore = xSemaphoreCreateBinary();
  treble_down_semaphore = xSemaphoreCreateBinary();
  treble_up_semaphore = xSemaphoreCreateBinary();
  pause_semaphore = xSemaphoreCreateBinary();
  menu_semaphore = xSemaphoreCreateBinary();
  accelerometer_semaphore = xSemaphoreCreateBinary();

  // lpc_peripheral__enable_interrupt(LPC_PERIPHERAL__GPIO, gpio0__interrupt_dispatcher);
  // gpio0__attach_interrupt(29, GPIO_INTR__FALLING_EDGE, volumeup_isr);
  // gpio0__attach_interrupt(30, GPIO_INTR__FALLING_EDGE, volumedown_isr); // step 3.5 attach the interrupt
  // gpio0__attach_interrupt(7, GPIO_INTR__FALLING_EDGE, pass_song_isr);
  // gpio0__attach_interrupt(6, GPIO_INTR__FALLING_EDGE, move_up_isr);
  // gpio0__attach_interrupt(8, GPIO_INTR__FALLING_EDGE, move_down_isr);
  // gpio0__attach_interrupt(25, GPIO_INTR__FALLING_EDGE, bass_down_isr);
  // gpio0__attach_interrupt(26, GPIO_INTR__FALLING_EDGE, bass_up_isr);
  // gpio0__attach_interrupt(0, GPIO_INTR__FALLING_EDGE, treble_up_isr);
  // gpio0__attach_interrupt(1, GPIO_INTR__FALLING_EDGE, treble_down_isr);
  // gpio0__attach_interrupt(16, GPIO_INTR__FALLING_EDGE, pause_isr);
  // gpio0__attach_interrupt(22, GPIO_INTR__FALLING_EDGE, menu_isr);
  // gpio0__attach_interrupt(9, GPIO_INTR__FALLING_EDGE, accelerometer_isr);

  // enable GPIO interrupt
  NVIC_EnableIRQ(GPIO_IRQn);
  init_SPI();
  mp3_setup();
  // lcd_menu_switch_init();
  // acceleration__init();

  sj2_cli__init();

  xTaskCreate(mp3_reader_task, "reader", (2024 * 4) / sizeof(void *), NULL, PRIORITY_MEDIUM, NULL);
  // xTaskCreate(volumeup_task, "volumeup", (1024 * 4) / sizeof(void *), NULL, PRIORITY_MEDIUM, NULL);
  // xTaskCreate(volumedwn_task, "volumedwn", (1024 * 4) / sizeof(void *), NULL, PRIORITY_MEDIUM, NULL);
  xTaskCreate(mp3_player_task, "player", (3096 * 4) / sizeof(void *), NULL, PRIORITY_MEDIUM,
              &player_handle); // made a change here
  // xTaskCreate(mp3_screen_control_task, "screen controls", (1024 * 4) / sizeof(void *), NULL, PRIORITY_MEDIUM, NULL);
  // xTaskCreate(mp3_screen_control_task2, "move arrow down", (1024 * 4) / sizeof(void *), NULL, PRIORITY_MEDIUM, NULL);
  // xTaskCreate(pass_song_name_task, "pass", (1024 * 4) / sizeof(void *), NULL, PRIORITY_MEDIUM, NULL);
  // xTaskCreate(bass_up_task, "bass up", (1024 * 4) / sizeof(void *), NULL, PRIORITY_MEDIUM, NULL);
  // xTaskCreate(bass_down_task, "bass down", (1024 * 4) / sizeof(void *), NULL, PRIORITY_MEDIUM, NULL);
  // xTaskCreate(treble_down_task, "treble down", (1024 * 4) / sizeof(void *), NULL, PRIORITY_MEDIUM, NULL);
  // xTaskCreate(treble_up_task, "treble up", (1024 * 4) / sizeof(void *), NULL, PRIORITY_MEDIUM, NULL);
  // xTaskCreate(pause_task, "pause", (1024 * 4) / sizeof(void *), NULL, PRIORITY_MEDIUM, NULL);
  // xTaskCreate(menu_task, "menu", (1024 * 4) / sizeof(void *), NULL, PRIORITY_MEDIUM, NULL);
  // // xTaskCreate(test_task, "pause", (3048 * 4) / sizeof(void *), NULL, PRIORITY_MEDIUM, NULL);
  // xTaskCreate(accelerometer_bass_treble_control, "accelerometer bass and treble", (1024 * 4) / sizeof(void *), NULL,
  //             PRIORITY_MEDIUM, NULL);

  vTaskStartScheduler();
  return 0;
}

void accelerometer_bass_treble_control(void *p) {
  while (1) {
    if (xSemaphoreTake(accelerometer_semaphore, portMAX_DELAY)) {
      // acceleration__init();
      // printf("in accelerometer task \n");
      acceleration__axis_data_s sensor_data;
      float average_z = 0;
      float average_x = 0;

      for (int i = 0; i < 10; i++) {
        sensor_data = acceleration__get_data();
        average_z += sensor_data.z;
        average_x += sensor_data.x;
        // printf("accerleration data x= %f, z = %f \n", average_x, average_z);
      }
      average_z = average_z / 10; /// 10;
      average_x = average_x / 10;

      // set bass level
      if (average_z < 1000) {
        // bass setting 1
        bass_level = 1;
        setBassLevel(bass_level);
      } else if (average_z > 1000 && average_z < 2000) {
        // bass setting 2
        bass_level = 2;
        setBassLevel(bass_level);
      } else if (average_z > 2000 && average_z < 3000) {
        // bass setting 3
        bass_level = 3;
        setBassLevel(bass_level);
      } else if (average_z > 3000 && average_z < 4000) {
        // bass setting 4
        bass_level = 4;
        setBassLevel(bass_level);
      } else {
        bass_level = 5;
        setBassLevel(bass_level);
      }

      // set treble level
      if (average_x < 1000) {
        // treble setting 1
        treble_level = 1;
        setTrebleLevel(treble_level);
      } else if (average_x > 1000 && average_x < 2000) {
        // treble setting 2
        treble_level = 2;
        setTrebleLevel(treble_level);
      } else if (average_x > 2000 && average_x < 3000) {
        // treble setting 3
        treble_level = 3;
        setTrebleLevel(treble_level);
      } else if (average_x > 3000 && average_x < 4000) {
        // treble setting 4
        treble_level = 4;
        setTrebleLevel(treble_level);
      } else {
        // treble setting 5
        treble_level = 5;
        setTrebleLevel(treble_level);
      }
    }
  }
}
void treble_bass_switch_init() {
  // bass down p0.25
  // bass up p0.26
  // treble up p0.0
  // treble down p0.1
  // treble control via accelerometer p0.9
  gpio__construct_with_function(GPIO__PORT_0, 25, GPIO__FUNCITON_0_IO_PIN);
  gpio__lab__set_as_input(0, 25);
  gpio__construct_with_function(GPIO__PORT_0, 26, GPIO__FUNCITON_0_IO_PIN);
  gpio__lab__set_as_input(0, 26);

  gpio__construct_with_function(GPIO__PORT_0, 0, GPIO__FUNCITON_0_IO_PIN);
  gpio__lab__set_as_input(0, 0);
  gpio__construct_with_function(GPIO__PORT_0, 1, GPIO__FUNCITON_0_IO_PIN);
  gpio__lab__set_as_input(0, 1);

  gpio__construct_with_function(GPIO__PORT_0, 9, GPIO__FUNCITON_0_IO_PIN);
  gpio__lab__set_as_input(0, 9);
}

void lcd_menu_switch_init() {
  // p0.6 -> menu up
  // p0.8 -> menu down
  // p0.7 -> select song
  // p0.16 -> pause/play
  // p0.22 -> show menu
  gpio__construct_with_function(GPIO__PORT_0, 6, GPIO__FUNCITON_0_IO_PIN);
  gpio__lab__set_as_input(0, 6);
  // LPC_IOCON->P1_16 &= ~(3 << 3);
  // LPC_IOCON->P0_16 |= (1 << 3);

  gpio__construct_with_function(GPIO__PORT_0, 8, GPIO__FUNCITON_0_IO_PIN);
  gpio__lab__set_as_input(0, 8);
  // LPC_IOCON->P0_17 &= ~(3 << 3);
  // LPC_IOCON->P0_17 |= (1 << 3);

  gpio__construct_with_function(GPIO__PORT_0, 7, GPIO__FUNCITON_0_IO_PIN);
  gpio__lab__set_as_input(0, 7);

  gpio__construct_with_function(GPIO__PORT_0, 16, GPIO__FUNCITON_0_IO_PIN);
  gpio__lab__set_as_input(0, 16);
  LPC_IOCON->P0_16 &= ~(3 << 3);
  LPC_IOCON->P0_16 |= (1 << 3);

  gpio__construct_with_function(GPIO__PORT_0, 22, GPIO__FUNCITON_0_IO_PIN);
  gpio__lab__set_as_input(0, 22);
}

void print_lcd_screen(int song_index) {

  SSD1306_Clear();
  SSD1306_InvertDisplay(true);
  int i = 0;
  while (i < 8 && song_index < (int)song_list__get_item_count) {
    if (i == 0) {
      SSD1306_PrintString("->");
      SSD1306_SetPageStartAddr(i);
      SSD1306_SetColStartAddr(15);
    } else {
      SSD1306_SetPageStartAddr(i);
      SSD1306_SetColStartAddr(0);
    }
    char song_name[24];
    strncpy(song_name, song_list__get_name_for_item(song_index), 23);
    // printf("Song name for index %d = %s\n", song_index, song_list__get_name_for_item(song_index));
    // printf("Strncpy version = %s \n \n", song_name);
    SSD1306_PrintString(song_name);
    i++; // made a change here
    song_index++;
  }
}

void mp3_screen_control_task(void *p) {
  while (1) {
    if (xSemaphoreTake(move_up_semaphore, portMAX_DELAY)) {
      // up
      if (cursor < (int)song_list__get_item_count) {
        cursor++;
        print_lcd_screen(cursor);
      }
    }
  }
}

// if  we end up having to split the screen control task in 2 this is the move down task void
void mp3_screen_control_task2(void *p) {
  while (1) {
    if (xSemaphoreTake(move_down_semaphore, portMAX_DELAY)) {
      // down
      if (cursor > 0) {
        cursor--;
        print_lcd_screen(cursor);
      }
    }
  }
}

void setup_volume_ctrl_sws() {
  // p0.29 (SW3) -> volume up
  gpio__construct_with_function(GPIO__PORT_0, 29, GPIO__FUNCITON_0_IO_PIN);
  gpio__lab__set_as_input(0, 29);

  // p0.30 (SW2) -> volume down
  gpio__construct_with_function(GPIO__PORT_0, 30, GPIO__FUNCITON_0_IO_PIN);
  gpio__lab__set_as_input(0, 30);
}

void mp3_reader_task(void *p) {
  trackname_t name;
  char bytes_512[512];
  UINT br;
  while (1) {
    xQueueReceive(Q_trackname, name, portMAX_DELAY);
    // printf("Received song to play: %s\n", name);

    // open file
    const char *filename = name; // check if accurate
    FIL file;
    FRESULT result = f_open(&file, filename, (FA_READ));
    if (FR_OK == result) {
      while (br != 0) {
        // read 512 bytes from file to bytes_512
        f_read(&file, bytes_512, sizeof(bytes_512), &br);
        xQueueSend(Q_songdata, bytes_512, portMAX_DELAY);

        if (uxQueueMessagesWaiting(Q_trackname)) {
          // printf("New play song request\n");
          break;
        }
      }
      // close file
      f_close(&file);
    } else {
      // printf("Failed to open mp3 file \n");
    }
  }
}

void mp3_player_task(void *p) {
  int counter = 1;
  while (1) {
    char bytes_512[512];
    xQueueReceive(Q_songdata, bytes_512, portMAX_DELAY);
    for (int i = 0; i < 512; i++) {
      // printf("%x \t", bytes_512[i]);
      while (!gpio__lab_get_level(2, 0)) {
        vTaskDelay(1);
      }
      SPI_send_mp3_data(bytes_512[i]);
    }
    // printf("Received 512 bytes : %d\n", counter);
    counter++;
  }
}

void volumeup_task(void *p) {
  while (1) {
    if (xSemaphoreTake(volumeup_semaphore, portMAX_DELAY)) {
      // read current volume
      uint16_t current_volume = Mp3ReadRegister(SCI_VOL);
      uint8_t current_volume_highbyte = (current_volume >> 8) & 0xFF;
      uint8_t current_volume_lowbyte = current_volume & 0xFF;
      if (current_volume > MAX_VOLUME) {
        Mp3WriteRegister(SCI_VOL, current_volume_highbyte - 25, current_volume_lowbyte - 25);
      }
    }
  }
}

void volumedwn_task(void *p) {
  while (1) {
    if (xSemaphoreTake(volumedwn_semaphore, portMAX_DELAY)) {
      // read current volume
      uint16_t current_volume = Mp3ReadRegister(SCI_VOL);
      uint8_t current_volume_highbyte = (current_volume >> 8) & 0xFF;
      uint8_t current_volume_lowbyte = current_volume & 0xFF;
      if (current_volume < MIN_VOLUME) {
        Mp3WriteRegister(SCI_VOL, current_volume_highbyte + 25, current_volume_lowbyte + 25);
      }
    }
  }
}

void volumeup_isr(void) { xSemaphoreGiveFromISR(volumeup_semaphore, NULL); }
void volumedown_isr(void) { xSemaphoreGiveFromISR(volumedwn_semaphore, NULL); } // step 4 isr to give semaphor
void pass_song_isr(void) { xSemaphoreGiveFromISR(pass_song_semaphore, NULL); }
void move_up_isr(void) { xSemaphoreGiveFromISR(move_up_semaphore, NULL); }
void move_down_isr(void) { xSemaphoreGiveFromISR(move_down_semaphore, NULL); }
void bass_up_isr(void) { xSemaphoreGiveFromISR(bass_up_semaphore, NULL); }
void bass_down_isr(void) { xSemaphoreGiveFromISR(bass_down_semaphore, NULL); }
void treble_up_isr(void) { xSemaphoreGiveFromISR(treble_up_semaphore, NULL); }
void treble_down_isr(void) { xSemaphoreGiveFromISR(treble_down_semaphore, NULL); }
void menu_isr(void) { xSemaphoreGiveFromISR(menu_semaphore, NULL); }
void accelerometer_isr(void) { xSemaphoreGiveFromISR(accelerometer_semaphore, NULL); }

void pause_isr(void) {
  // uart_printf__polled(UART__0, "pause task sending semaphore \n");
  xSemaphoreGiveFromISR(pause_semaphore, NULL);
}

void pass_song_name_task(void *p) { // using pin 0_7
  while (1) {
    if (xSemaphoreTake(pass_song_semaphore, portMAX_DELAY)) { // step 5 receive semaphor clause
      current_track_number = cursor;
      xQueueSend(Q_trackname, song_list__get_name_for_item(cursor), portMAX_DELAY);
    }
  }
}

void bass_down_task(void *p) {
  while (1) {
    if (xSemaphoreTake(bass_down_semaphore, portMAX_DELAY)) {
      // printf("in bass down task \n");
      if (bass_level > 1) {
        bass_level--;
        setBassLevel(bass_level);
      }
    }
  }
}

void bass_up_task(void *p) {
  while (1) {
    if (xSemaphoreTake(bass_up_semaphore, portMAX_DELAY)) {
      // check to make sure we dont try to set bass level to something more than 5
      // printf("in bass up task \n");
      if (bass_level < 5) {
        bass_level++;
        setBassLevel(bass_level);
      }
    }
  }
}

void treble_up_task(void *p) {
  while (1) {
    if (xSemaphoreTake(treble_up_semaphore, portMAX_DELAY)) {
      // printf("in treble up task \n");
      if (treble_level < 5) {
        treble_level++;
        setTrebleLevel(treble_level);
      }
    }
  }
}

void treble_down_task(void *p) {
  while (1) {
    if (xSemaphoreTake(treble_down_semaphore, portMAX_DELAY)) {
      // printf("in treble down task \n");
      if (treble_level > 1) {
        treble_level--;
        setTrebleLevel(treble_level);
      }
    }
  }
}

void test_task(void *p) {
  while (1) {
    if (gpio__lab_get_level(0, 16)) {
      printf("button is high \n");
    } else if (gpio__lab_get_level(0, 26)) {
      printf("button2 is high \n");
    }
  }
}

void pause_task(void *p) { // using button 0_16
  while (1) {
    if (xSemaphoreTake(pause_semaphore, portMAX_DELAY)) {
      // printf("in pause_task \n");
      if (play) {
        vTaskSuspend(player_handle);
        play = false;
      } else {
        vTaskResume(player_handle);
        play = true;
      }
    }
  }
}
void menu_task(void *p) {
  while (1) {
    if (xSemaphoreTake(menu_semaphore, portMAX_DELAY)) {
      print_menu_screen();
    }
  }
}
void print_menu_screen() {
  SSD1306_Clear();
  SSD1306_InvertDisplay(true);
  // printf("in pause_task \n");
  SSD1306_PrintString("Current song playing: \n");
  // get current song
  char current_song[24];
  strncpy(current_song, song_list__get_name_for_item(current_track_number), 23);
  SSD1306_PrintString(current_song);
  SSD1306_PrintString("\n");
  SSD1306_PrintString("\n");
  SSD1306_PrintString("Bass Level: \n");
  // convert bass level int
  char str[1];
  itoa(bass_level, str, 10);
  SSD1306_PrintString(str);
  SSD1306_PrintString("\n");
  SSD1306_PrintString("\n");
  SSD1306_PrintString("Treble Level: \n");
  itoa(treble_level, str, 10);
  SSD1306_PrintString(str);
}