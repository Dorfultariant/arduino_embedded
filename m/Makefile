# Parameters:
F_CPU=16000000UL

MCU=atmega2560 # With atmega 2560 MCU -> atmega2560

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
$(TARGET).hex:$(TARGET).c lcd.o
	$(CC) $(CFLAGS) -o $(TARGET).elf $(TARGET).c lcd.o
	avr-objcopy -OD ihex -R .eeprom $(TARGET).elf $(TARGET).hex

# Bit banging
upload: $(TARGET).hex
	$(AVRDUDE) -U flash:w:$(TARGET).hex:i

# Tidying folder
clean:
	rm -f $(TARGET).elf $(TARGET).hex lcd.o


# NOTE: ADDITIONAL LIBRARIES
lcd.o: lcd.c lcd.h
	$(CC) $(CFLAGS) -c lcd.c -o lcd.o

# run "make all" to run compilation, upload and clean
