#include <stdint.h>
#define F_CPU 16000000UL
#define SLAVE_ADDRESS 85

#include <avr/io.h>
#include <stdlib.h>
#include <util/delay.h>

#include <avr/interrupt.h>

const int BUILTIN_LED = PB5;

const int SDA PC4;
const int SCL = PC5;

const uint8_t DATA_SIZE = 16;
// 32 Bytes worth of data
typedef struct tr_data {
  char tranText[DATA_SIZE];
  char recvText[DATA_SIZE];
} TR_Data;

int main(void) {

  DDRC &= ~(1 << SDA) & ~(1 << SDL);

  // // Turn off led
  DDRB &= ~(1 << BUILTIN_LED);

  TR_Data data = {"Hello my Master\n", ""};

  uint8_t twi_idx = 0;
  uint8_t twi_stat = 0;

  // LSB --> 170
  TWAR = 0b10101010;
  // Slave receiver mode setup from documentation:
  TWCR |= (1 << TWEA) | (1 << TWEN);
  TWCR &= ~(1 << TWSTA) & ~(1 << TWSTO);

  while (1) {

    // wait for transmission:
    while (!(TWCR & (1 << TWINT)))
      ;

    // Set status
    twi_stat = (TWSR & 0xF8);

    // Reset the TWEA and TWEN and create ACK
    TWCR |= (1 << TWINT) | (1 << TWEA) | (1 << TWEN);

    // Waiting for TWINT to set:
    while (!(TWCR & (1 << TWINT)))
      ;

    // Set status
    twi_stat = (TWSR & 0xF8);
    // HEX values can be found in atmega 2560 doc page: 255, table: 24-4
    // Condition check of twi_status if previous was response of either slave
    // address or general call and NOT ACK return
    if ((twi_stat == 0x80) || (twi_stat == 0x90)) {
      data.recvText[twi_idx] = TWDR;
      twi_idx++;
    } else if ((twi_stat == 0x88) || (twi_stat == 0x98)) {
      twi_receive_data[twi_idx] = TWDR;
      twi_idx++;
    } else if ((twi_stat == 0xA0)) { // STOP signal
      TWCR |= (1 << TWINT);
    }
    if (DATA_SIZE <= twi_idx) {
      // Turn the led on
      PORTB |= (1 << BUILTIN_LED);
      twi_idx = 0;
    }
  }
  return 0;
}
