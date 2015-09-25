#include <stdio.h>
#include <stdlib.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "lcd.h"
#include "cpu_speed.h"
#include "graphics.h"

int main() {

  // set clock speed
  set_clock_speed(CPU_8MHz);

  // initialise the LCD screen
  LCDInitialise(LCD_DEFAULT_CONTRAST);

  // prescaler = 128 (beween 40 and 160)
  // 8MHz / 50kHz = 800 / 5 = 160
  // 8MHz / 200kHz = 80 / 2 = 40
  ADCSRA |= (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);

  // ADC result register
  ADMUX |= (1 << ADLAR);

  // voltage reference
  ADMUX |= (1 << REFS0);

  // enable ADC interrupt
  ADCSRA |= (1 << ADIE);

  // enable ADC
  ADCSRA |= (1 << ADEN);

  // enable global interrupt
  sei();

  // start conversion
  ADCSRA |= (1 << ADSC);

  while (1) {
    clear_screen();
    show_screen();
  }

  return 0;
}

ISR(ADC_vect) {
  char adc_result[4];

  // ADCH - actual result value
  // convert integer to string
  itoa(ADCH, adc_result, 10);
  draw_string(0, 0, adc_result);

  // start another conversion
  ADCSRA |= (1 << ADSC);
}

