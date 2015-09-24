#include <stdio.h>
#include <stdlib.h>
#include <avr/io.h>
#include <avr/interrupt.h>
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
void menu(int level, char desc[], char buff[]);
void status(int lives, int scores, char buff[]);
void update_player(Sprite* player);
int faces_collision(Sprite* face1, Sprite* face2);

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

Sprite player;
Sprite faces[NUM_FACES];
int start = 0;

int main() {

  int opt = 0;
  int lives = 3;
  int scores = 0;
  char buff[80];
  int random = 0;

  init_sprite(&player, 39, 40, 8, 8, player_bitmaps);
  init_sprite(&faces[0], 10, 10, 16, 16, faces_bitmaps[0]);
  init_sprite(&faces[1], 33, 10, 16, 16, faces_bitmaps[1]);
  init_sprite(&faces[2], 56, 10, 16, 16, faces_bitmaps[2]);

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
          menu(1, "Basic", buff);
          opt = check_option(opt);

          if (left_button_pressed()) {
            start = 1;
            while (lives > 0) {
              clear_screen();
              status(lives, scores, buff);
              draw_sprite(&player);
              update_player(&player);

              for (int i = 0; i < NUM_FACES; i++) {
                draw_sprite(&faces[i]);
              }

              for (int i = 0; i < NUM_FACES; i++) {
                int next_y = (int) round(faces[i].y + 1);

                if (next_y >= 49) {
                  faces[i].y = 10;
                  random = rand() % 66;
                  faces[i].x = random;

                  while (faces_collision(&faces[i], &faces[(i + 1) % 3]) ||
                    faces_collision(&faces[i], &faces[(i + 2) % 3])) {
                    random = rand() % 66;
                    faces[i].x = random;
                  }

                  faces[i].x = random;
                  faces[i].y = 10;
                }
              }

              show_screen();
            }
          }

          show_screen();
        }

        while (opt == 1) {
          clear_screen();
          menu(2, "Potentiometers", buff);
          opt = check_option(opt);
          show_screen();
        }

        while (opt == 2) {
          clear_screen();
          menu(3, "Serial Port", buff);
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

  TCCR1B &= ~(1 << WGM12);

  TCCR1B |= ((1 << CS11) | (1 << CS10));
  TCCR1B &= ~(1 << CS12);

  TIMSK1 = (1 << TOIE1);

  sei();
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

void menu(int level, char desc[], char buff[]) {
  draw_string(0, 0, "Level");
  sprintf(buff, "%d", level);
  draw_string(28, 0, buff);
  draw_string(0, 8, desc);
  draw_string(0, 24, "Left: Play");
  draw_string(0, 32, "Right: Skip");
}

void status(int lives, int scores, char buff[]) {
  draw_string(0, 0, "L:");
  sprintf(buff, "%d", lives);
  draw_string(12, 0, buff);
  draw_string(17, 0, ", S:");
  sprintf(buff, "%d", scores);
  draw_string(40, 0, buff);
  draw_line(0, 8, 84, 8);
}

void update_player(Sprite* player) {
  int next_left = (int) round(player->x - 1);
  int next_right = (int) round(player->x + 5);

  if (left_button_pressed()) {
    player->x -= 3;
    if (next_left <= 0) {
      player->x = 0;
    }
  } else if (right_button_pressed()) {
    player->x += 3;
    if (next_right >= 83) {
        player->x = 79;
    }
  }
}

int faces_collision(Sprite* face1, Sprite* face2) {
  return ((abs(face1->x - face2->x) * 2) - 10 < 32);
  // return (face1->x < face2->x + 23 &&
  //   face1->x + 23 > face2->x &&
  //   face1->y < face2->y + 16 &&
  //   face1->y + 16 > face2->y);
}

ISR(TIMER1_OVF_vect) {
  if (start) {
    for (int i = 0; i < NUM_FACES; i++) {
      faces[i].y += 2;
    }
  }
}


