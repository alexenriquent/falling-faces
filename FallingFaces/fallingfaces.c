#include <stdio.h>
#include <avr/io.h>
#include <util/delay.h>
#include <math.h>

#include "lcd.h"
#include "cpu_speed.h"
#include "graphics.h"
#include "sprite.h"

#define NUM_FACES 3
#define BYTE_PER_PLAYER 8
#define BYTE_PER_FACE 32

void init();
void welcome();
int buttons_pressed();
int left_button_pressed();
int right_button_pressed();
int check_option(int option);

unsigned char player_bitmaps[BYTE_PER_PLAYER] = {
  0b00000000,
  0b10001000,
  0b10001000,
  0b10001000,
  0b11111000,
  0b10001000,
  0b10001000,
  0b10001000
};

unsigned char faces_bitmaps[NUM_FACES][BYTE_PER_FACE] = {
  {
    0b00000111, 0b11100000,
    0b00011000, 0b00011000,
    0b00100000, 0b00000100,
    0b01000000, 0b00000010,
    0b01011000, 0b00011010,
    0b10011000, 0b00011001,
    0b10000000, 0b00000001,
    0b10000000, 0b00000001,
    0b10010000, 0b00001001,
    0b10010000, 0b00001001,
    0b10001000, 0b00010001,
    0b01000111, 0b11100010,
    0b01000000, 0b00000010,
    0b00100000, 0b00000100,
    0b00011000, 0b00011000,
    0b00000111, 0b11100000
    }, 
    {
    0b00000111, 0b11100000,
    0b00011000, 0b00011000,
    0b00100000, 0b00000100,
    0b01000000, 0b00000010,
    0b01011000, 0b00011010,
    0b10011000, 0b00011001,
    0b10000000, 0b00000001,
    0b10000000, 0b00000001,
    0b10000000, 0b00000001,
    0b10001111, 0b11110001,
    0b10110000, 0b00001101,
    0b01000000, 0b00000010,
    0b01000000, 0b00000010,
    0b00100000, 0b00000100,
    0b00011000, 0b00011000,
    0b00000111, 0b11100000
    },
    {
    0b00000111, 0b11100000,
    0b00011000, 0b00011000,
    0b00100000, 0b00000100,
    0b01000000, 0b00000010,
    0b01011000, 0b00011010,
    0b10011000, 0b00011001,
    0b10000000, 0b00000001,
    0b10000000, 0b00000001,
    0b10000111, 0b11100001,
    0b10001000, 0b00010001,
    0b10010000, 0b00001001,
    0b01010000, 0b00001010,
    0b01000000, 0b00000010,
    0b00100000, 0b00000100,
    0b00011000, 0b00011000,
    0b00000111, 0b11100000
    }
};

int main() {

  int opt = 0;
  int lives = 3;
  int scores = 0;
  char buff[80];

  Sprite player;
  Sprite faces[NUM_FACES];

  init_sprite(&player, 42, 40, 8, 8, player_bitmaps);

  for (int i = 0; i < NUM_FACES; i++) {
    init_sprite(&faces[i], 0, 0, 16, 16, faces_bitmaps[i]);
  }

  set_clock_speed(CPU_8MHz);

  init();

  while (1) {
    clear_screen();
    welcome();
    show_screen();

    if (buttons_pressed()) {
      while (1) {
        clear_screen();
        opt = check_option(opt);

        while (opt == 0) {
          clear_screen();
          draw_string(0, 0, "Level 1");
          draw_string(0, 8, "Basic");
          draw_string(0, 24, "Left: Play");
          draw_string(0, 32, "Right: Skip");
          opt = check_option(opt);

          if (left_button_pressed()) {
            while (lives > 0) {
              clear_screen();
              draw_string(0, 0, "L:");
              sprintf(buff, "%d", lives);
              draw_string(12, 0, buff);
              draw_string(17, 0, ", S:");
              sprintf(buff, "%d", scores);
              draw_string(40, 0, buff);
              draw_line(0, 8, 84, 8);
              draw_sprite(&player);

              int next_left = (int) round(player.x - 1);
              int next_right = (int) round(player.x + 5);

              if (left_button_pressed()) {
                player.x -= 3;
                if (next_left <= 0) {
                  player.x = 3;
                }
              } else if (right_button_pressed()) {
                player.x += 3;
                if (next_right >= 83) {
                  player.x = 79;
                }
              }

              show_screen();
            }
          }

          show_screen();
        }

        while (opt == 1) {
          clear_screen();
          draw_string(0, 0, "Level 2");
          draw_string(0, 8, "Potentiometers");
          draw_string(0, 24, "Left: Play");
          draw_string(0, 32, "Right: Skip");
          opt = check_option(opt);
          show_screen();
        }

        while (opt == 2) {
          clear_screen();
          draw_string(0, 0, "Level 3");
          draw_string(0, 8, "Serial Port");
          draw_string(0, 24, "Left: Play");
          draw_string(0, 32, "Right: Skip");
          opt = check_option(opt);
          show_screen();
        }

        show_screen();
      }
    }
  }

  return 0;
}

void init() {
  LCDInitialise(LCD_DEFAULT_CONTRAST);

  DDRB &= ~((1 << PB0) | (1 << PB1));
}

void welcome() {
  draw_string(0, 0, "Thanat");
  draw_string(0, 8, "Chokwijitkul");
  draw_string(0, 16, "n9234900");
  draw_string(0, 32, "Press any button");
}

int buttons_pressed() {
  if (left_button_pressed() || 
    right_button_pressed()) {
    return 1;
  }
  return 0;
}

int left_button_pressed() {
  static uint8_t button_state = 0;
  uint8_t current_state = PINB & (1 << PB0);

  if (current_state != button_state) {
    button_state = current_state;
    if (current_state == 0) {
      return 1;
    }
  }
  return 0;
}

int right_button_pressed() {
  static uint8_t button_state = 0;
  uint8_t current_state = PINB & (1 << PB1);

  if (current_state != button_state) {
    button_state = current_state;
    if (current_state == 0) {
      return 1;
    }
  }
  return 0;
}

int check_option(int option) {
  if (right_button_pressed()) {
    option++;
    option = option % 3;
  }
  return option;
}


