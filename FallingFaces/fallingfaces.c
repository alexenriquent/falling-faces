#include <avr/io.h>
#include <util/delay.h>
#include <math.h>

#include "lcd.h"
#include "cpu_speed.h"
#include "graphics.h"
#include "sprite.h"

void init();
void welcome();
int buttons_pressed();
int left_button_pressed();
int right_button_pressed();

int main() {

  int opt = 0;

  set_clock_speed(CPU_8MHz);

  init();

  while (1) {
    clear_screen();
    welcome();
    show_screen();

    if (buttons_pressed()) {
      while (1) {
        clear_screen();

        if (right_button_pressed()) {
          opt++;
          opt = opt % 3;
        }

        while (opt == 0) {
          clear_screen();

          if (right_button_pressed()) {
            opt++;
            opt = opt % 3;
          }

          draw_string(0, 0, "Level 1");
          show_screen();
        }

        while (opt == 1) {
          clear_screen();

          if (right_button_pressed()) {
            opt++;
            opt = opt % 3;
          }

          draw_string(0, 0, "Level 2");
          show_screen();
        }

        while (opt == 2) {
          clear_screen();

          if (right_button_pressed()) {
            opt++;
            opt = opt % 3;
          }

          draw_string(0, 0, "Level 3");
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


