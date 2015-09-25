#
# CAB202 Microprocessors and Digital Systems
#
#	B.Talbot, September 2015
#	Queensland University of Technology
#

SRC=pot
CAB202_LIB_DIR=../cab202_teensy

FLAGS=-mmcu=atmega32u4 -Os -DF_CPU=8000000UL -std=gnu99
LIBS=-Wl,-u,vfprintf -lprintf_flt -lcab202_teensy -lm

all:
	avr-gcc $(SRC).c $(FLAGS) -I$(CAB202_LIB_DIR) -L$(CAB202_LIB_DIR) $(LIBS) -o $(SRC).o
	avr-objcopy -O ihex $(SRC).o $(SRC).hex
