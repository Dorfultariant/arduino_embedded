# Parameters:
F_CPU=16000000UL

# With atmega 2560 MCU -> atmega2560
MCU=atmega2560

PROGRAMMER=wiring

PORT=/dev/ttyACM0
BAUD=115200
TARGET=main


# Compiler
CC=avr-gcc

# -g debug, -Os optimization, -mmcu chip, -DF_CPU is the speed of chip
CFLAGS=-g -Os -mmcu=$(MCU) -DF_CPU=$(F_CPU)


# AVRDUUDE
AVRDUDE=avrdude -c $(PROGRAMMER) -p $(MCU) -P $(PORT) -b $(BAUD)

# Target for all:
all: $(TARGET).hex upload clean

# Compilation process
$(TARGET).hex:$(TARGET).c uart.o keypad.o delay.o
	$(CC) $(CFLAGS) -o $(TARGET).elf $(TARGET).c uart.o keypad.o delay.o
	avr-objcopy -O ihex -R .eeprom $(TARGET).elf $(TARGET).hex

# Bit banging
upload: $(TARGET).hex
	$(AVRDUDE) -DU flash:w:$(TARGET).hex:i

# Tidying folder
clean:
	rm -f $(TARGET).elf $(TARGET).hex uart.o keypad.o delay.o


# NOTE: ADDITIONAL LIBRARIES
uart.o: uart.c uart.h
	$(CC) $(CFLAGS) -c uart.c -o uart.o

keypad.o: keypad.c keypad.h stdutils.h
	$(CC) $(CFLAGS) -c keypad.c -o keypad.o

delay.o: delay.c delay.h
	$(CC) $(CFLAGS) -c delay.c -o delay.o

# run "make all" to run compilation, upload and clean
