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

int main() {

  int opt;

  set_clock_speed(CPU_8MHz);

  init();

  while (1) {
    clear_screen();
    welcome();
    show_screen();

    if (buttons_pressed()) {
      clear_screen();
      while (1) {
        clear_screen();
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
  if (PINB & (1 << PB0) ||
    PINB & (1 << PB1)) {
    return 1;
  }
  return 0;
}



