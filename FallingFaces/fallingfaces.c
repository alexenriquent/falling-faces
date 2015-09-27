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
#include "usb_serial.h"

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
void status(char buff[]);
void reset_game();
void draw_player();
void draw_faces();
void update_player();
void update_player_pot();
void update_player_usb();
void update_faces();
void update_status();
int player_collision();
int faces_collision(Sprite* face1, Sprite* face2);
int bounce_collision(Sprite* face1, Sprite* face2);
void wrap_around();
void randomise_faces();
void randomise_directions();
void spawn_faces();
void bounce();
void edge_bounce();
void check_speed();
int finish();
void send_str(char* str);

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
    0b01011000, 0b00000010,
    0b10011000, 0b00000001,
    0b10000000, 0b00011001,
    0b10000000, 0b00011001,
    0b10000000, 0b00000001,
    0b10000111, 0b11110001,
    0b10000001, 0b10000001,
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
    0b01001000, 0b00010010,
    0b10000100, 0b00100001,
    0b10000010, 0b01000001,
    0b10000000, 0b00000001,
    0b10000000, 0b00000001,
    0b10000011, 0b11000001,
    0b10000100, 0b00100001,
    0b01001000, 0b00010010,
    0b01000000, 0b00000010,
    0b00100000, 0b00000100,
    0b00011000, 0b00011000,
    0b00000111, 0b11100000
    }
};

Sprite player;
Sprite faces[NUM_FACES];
int start = 0;
int lives = 3;
int scores = 0;
int finish_round = 0;
int speed = 0;
int serial_port = 0;

int main() {

  int opt = 0;
  char buff[80];

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
            reset_game();

            while (lives > 0 && scores < 20) {
              clear_screen();
              status(buff);
              update_player();
              update_faces();
              check_speed();
              show_screen();
            }
            opt = finish();
          }
          show_screen();
        }

        while (opt == 1) {
          clear_screen();
          menu(2, "Potentiometers", buff);
          opt = check_option(opt);

          if (left_button_pressed()) {
            reset_game();

            while (lives > 0) {
              clear_screen();
              status(buff);
              update_player_pot();
              update_faces();
              check_speed();
              show_screen();
            }
            opt = finish();
          }
          show_screen();
        }

        while (opt == 2) {
          clear_screen();
          menu(3, "Serial Port", buff);
          opt = check_option(opt);

          if (left_button_pressed()) {
            reset_game();
            start = 0;

            clear_screen();
            draw_string(14, 17, "Waiting for");
            draw_string(9, 24, "the player...");
            show_screen();

            while (!usb_configured() || 
              !usb_serial_get_control());

            for (int i = 0; i < NUM_FACES; i++) {
              faces[i].x = 0;
              faces[i].y = 0;
            }

            spawn_faces();
            randomise_directions();
            serial_port = 1;

            while (lives > 0) {
              clear_screen();
              status(buff);
              update_player_usb();
              draw_faces();
              edge_bounce();
              bounce();
              check_speed();
              show_screen();
            }
            opt = finish();
          }
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

  ADCSRA |= (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);

  ADMUX |= (1 << ADLAR);

  ADMUX |= (1 << REFS0);

  ADCSRA |= (1 << ADEN);

  usb_init();

  sei();

  ADCSRA |= (1 << ADSC);
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

void status(char buff[]) {
  draw_string(0, 0, "L:");
  sprintf(buff, "%d", lives);
  draw_string(12, 0, buff);
  draw_string(17, 0, ", S:");
  sprintf(buff, "%d", scores);
  draw_string(40, 0, buff);
  // sprintf(buff, "%d", speed);
  // draw_string(78, 0, buff);
  // sprintf(buff, "%d", ADCH);
  // draw_string(60, 0, buff);
  draw_line(0, 8, 84, 8);
}

void reset_game() {
  start = 1;
  lives = 3;
  scores = 0;
  finish_round = 0;
  speed = 0;
  serial_port = 0;
  player.x = 39;

  for (int i = 0; i < NUM_FACES; i++) {
    faces[i].y = 10;
    faces[i].dx = 0;
    faces[i].dy = 0;
  }
}

void draw_player() {
  draw_sprite(&player);
}

void draw_faces() {
  for (int i = 0; i < NUM_FACES; i++) {
    if (faces[i].y >= 10) {
      draw_sprite(&faces[i]);
    }
  }
}

void update_player() {
  draw_player();

  int next_left = (int) round(player.x - 1);
  int next_right = (int) round(player.x + 5);

  if (left_button_pressed()) {
    player.x -= 3;
    if (next_left <= 0) {
      player.x = 0;
    }
  } else if (right_button_pressed()) {
    player.x += 3;
    if (next_right >= 83) {
        player.x = 79;
    }
  }
}

void update_player_pot() {
  draw_player();
  player.x = 79 - floor(ADCH / 3.2);
  ADCSRA |= (1 << ADSC);
}

void update_player_usb() {
  char key;
  int next;

  draw_player();
  key = usb_serial_getchar();

  switch (key) {
    case 'a':
      next = (int) round(player.x - 1);
      player.x -= 3;
      if (next <= 0) {
        player.x = 0;
      }
      break;
    case 'd':
      next = (int) round(player.x + 5);
      player.x += 3;
      if (next >= 83) {
        player.x = 79;
      }
      break;
    case 'w':
      next = (int) round(player.y - 1);
      player.y -= 3;
      if (next <= 10) {
        player.y = 10;
      }
      break;
    case 's':
      next = (int) round(player.y + 7);
      player.y += 3;
      if (next >= 40) {
        player.y = 40;
      }
    break;
  }
}

void update_faces() {
  draw_faces();
  wrap_around();
  randomise_faces();
  update_status();
}

void update_status() {
  if (!finish_round) {
    if (player_collision() >= 0) {
      switch (player_collision()) {
        case 0:
          scores += 2;
          break;
        case 1:
          speed++;
          if (speed >= 2) {
            speed = 2;
          }
          break;
        case 2:
          lives -= 1;
          break;
      }
      finish_round = 1;
    }
  }
}

int player_collision() {
  for (int i = 0; i < NUM_FACES; i++) {
    if (player.x + 4 >= faces[i].x &&
      player.x <= faces[i].x + 15 &&
      player.y + 7 >= faces[i].y &&
      player.y <= faces[i].y + 15) {
      return i;
    }
  }
  return -1;
}

int faces_collision(Sprite* face1, Sprite* face2) {
  return (abs(face1->x - face2->x) <= 21) &&
    (abs(face1->y - face2->y) <= 21);
}

int bounce_collision(Sprite* face1, Sprite* face2) {
  // return (abs(face1->x - face2->x) <= 16) &&
  //   (abs(face1->y - face2->y) <= 16);
  return (face1->x + 16 >= face2->x &&
      face1->x <= face2->x + 16 &&
      face1->y + 16 >= face2->y &&
      face1->y <= face2->y + 16);
}

void wrap_around() {
  int random = 0;

  for (int i = 0; i < NUM_FACES; i++) {
    int next_y = (int) round(faces[i].y + 15);

    if (next_y >= 48) {
      if (speed == 1 || speed == 2) {
        faces[i].y = 7;
      } else {
        faces[i].y = 8;
      } 
      random = rand() % 66;
      faces[i].x = random;
      finish_round = 0;
    }
  }
}

void randomise_faces() {
  int random = 0;

  for (int i = 0; i < NUM_FACES; i++) {
    while (faces_collision(&faces[i], &faces[(i + 1) % 3])) {
      random = rand() % 66;
      faces[i].x = random;
      if (speed == 1 || speed == 2) {
        faces[i].y = 7;
      } else {
        faces[i].y = 8;
      } 
    }
  } 
}

void randomise_directions() {
  int rand_x;
  int rand_y;

  for (int i = 0; i < NUM_FACES; i++) {
    rand_x = rand() % 2;
    rand_y = rand() % 2;
    if (rand_x == 0) {
      faces[i].dx = -2;
    } else if (rand_x == 1) {
      faces[i].dx = 2;
    }
    if (rand_y == 0) {
      faces[i].dy = -2;
    } else if (rand_y == 1) {
      faces[i].dy = 2;
    }
  }
}

void spawn_faces() {
  int rand_x = 0;
  int rand_y = 0;

  for (int i = 0; i < NUM_FACES; i++) {
    rand_x = rand() % 68;
    rand_y = (rand() % 22) + 10;
    faces[i].x = rand_x;
    faces[i].y = rand_y;
    while (faces_collision(&faces[i], &faces[(i + 1) % 3]) ||
      faces_collision(&faces[i], &faces[(i + 2) % 3])) {
      rand_x = rand() % 68;
      rand_y = (rand() % 22) + 10;
      faces[i].x = rand_x;
      faces[i].y = rand_y;
    }
  }
}

void bounce() {
  for (int i = 0; i < NUM_FACES; i++) {
    if (bounce_collision(&faces[i], &faces[(i + 1) % 3])) {
      if (faces[i].dx == -2) {
        faces[i].dx = 2;
        faces[i].x += 2;
        faces[(i + 1) % 3].dx = -2;
        faces[(i + 1) % 3].x -= 2;
        break;
      } else if (faces[i].dx == 2) {
        faces[i].dx = -2;
        faces[i].x -= 2;
        faces[(i + 1) % 3].dx = 2;
        faces[(i + 1) % 3].x += 2;
        break;
      } else if (faces[i].dy == -2) {
        faces[i].dy = 2;
        faces[i].y += 2;
        faces[(i + 1) % 3].dy = -2;
        faces[(i + 1) % 3].y -= 2;
        break;
      } else if (faces[i].dy == 2) {
        faces[i].dy = -2;
        faces[i].x -= 2;
        faces[(i + 1) % 3].dy = 2;
        faces[(i + 1) % 3].y += 2;
        break;
      }
    }
  }
}

void edge_bounce() {
  for (int i = 0; i < NUM_FACES; i++) {
    if (faces[i].x <= 0) {
      faces[i].x = 0;
      faces[i].dx = 2;
    } else if (faces[i].x >= 68) {
      faces[i].x = 68;
      faces[i].dx = -2;
    }
    if (faces[i].y <= 10) {
      faces[i].y = 10;
      faces[i].dy = 2;
    } else if (faces[i].y >= 32) {
      faces[i].y = 32;
      faces[i].dy = -2;
    } 
  }
}

void check_speed() {
  if (speed == 1) {
    if (TCNT1 >= 45000) {
      TCNT1 = 0xffff;
    }
  } else if (speed == 2) {
    if (TCNT1 >= 30000) {
      TCNT1 = 0xffff;
    }
  } 
}

int finish() {
  clear_screen();
  start = 0;
  if (scores == 20) {
    draw_string(22, 20, "You win!");
    show_screen();
    _delay_ms(3000);
    return 1;
  } else if (lives == 0) {
    draw_string(22, 20, "You lose!");
    show_screen();
    _delay_ms(3000);
    return 0;
  } 
}

void send_str(char* str) {
  while (*str != '\0') {
    usb_serial_putchar(*str);
    str++;
  }

  usb_serial_putchar('\r');
  usb_serial_putchar('\n');
}

ISR(TIMER1_OVF_vect) {
  if (start) {
    for (int i = 0; i < NUM_FACES; i++) {
      faces[i].y += 2;
    }
  } else if (serial_port) {
    for (int i = 0; i < NUM_FACES; i++) {
      faces[i].x += faces[i].dx;
      faces[i].y += faces[i].dy;
    }
  }
}
