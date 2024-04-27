#ifndef _OWN_EEPROM_H
#define _OWN_EEPROM_H

#include <avr/io.h>
#include <stdlib.h>

// Function to read data from EEPROM memory
unsigned char EEPROM_read(unsigned int ui_address);

// Function to write data to EEPROM
void EEPROM_write(unsigned int ui_address, unsigned char uc_data);

// Reading string from address to destination
char *read_string(char *dest, unsigned int ui_address, uint8_t str_size);

// Writing to address from source:
void write_string(unsigned int ui_address, char *str, uint8_t str_size);

#endif // _OWN_EEPROM_H
