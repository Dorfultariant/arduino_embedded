# Parameters:
F_CPU=16000000UL

MCU=atmega328p # With atmega 2560 MCU -> atmega2560

PROGRAMMER=arduino

PORT=/dev/ttyACM0
BAUD=115200
TARGET=main


# Compiler
CC=avr-gcc

# -g debug, -Os optimization, -mmcu chip, -DF_CPU is the speed of chip
CFLAGS=-g -Os -mmcu=$(MCU) -DF_CPU=$(F_CPU)


# AVRDUUDE
AVRDUDE=avrdude -c $(PROGRAMMER) -p $(MCU) -P $(PORT) -b $(BAUD)

# Target for all: upload
all: $(TARGET).hex upload clean

# Compilation process
$(TARGET).hex:$(TARGET).c uart.o
	$(CC) $(CFLAGS) -o $(TARGET).elf $(TARGET).c uart.o
	avr-objcopy -O ihex -R .eeprom $(TARGET).elf $(TARGET).hex

# Bit banging
upload: $(TARGET).hex
	$(AVRDUDE) -U flash:w:$(TARGET).hex:i

# Tidying folder
clean:
	rm -f $(TARGET).elf $(TARGET).hex uart.o


# NOTE: ADDITIONAL LIBRARIES
uart.o: uart.c uart.h
	$(CC) $(CFLAGS) -c uart.c -o uart.o


# run "make all" to run compilation, upload and clean

