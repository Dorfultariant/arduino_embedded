#include "own_eeprom.h"

void write_string(unsigned int ui_address, char *str, uint8_t str_size) {
  unsigned int i = 0;
  while (i < str_size) {
    EEPROM_write(i + ui_address, str[i]);
    i++;
  }
  EEPROM_write(i, '\0');
}

char *read_string(char *dest, unsigned int ui_address, uint8_t str_size) {
  unsigned int i = 0;
  while (i < str_size) {
    dest[i] = EEPROM_read(i + ui_address);
    i++;
  }
  dest[i] = '\0';
  return dest;
}

void EEPROM_write(unsigned int ui_address, unsigned char uc_data) {
  while (EECR & (1 << EEPE))
    ;
  EEAR = ui_address;
  EEDR = uc_data;
  EECR |= (1 << EEMPE);
  EECR |= (1 << EEPE);
}

unsigned char EEPROM_read(unsigned int ui_address) {
  while (EECR & (1 << EEPE))
    ;
  EEAR = ui_address;
  EECR |= (1 << EERE);
  return EEDR;
}
