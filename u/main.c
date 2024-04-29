#include <stdint.h>
#define F_CPU 16000000UL
#define SLAVE_ADDRESS 85
#define BAUD 9600
#define MYUBRR F_CPU / 16 / BAUD - 1
#define DATA_SIZE 16

#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdio.h>
#include <stdlib.h>
#include <util/delay.h>
#include <util/setbaud.h>

#include "uart.h"

const int BUILTIN_LED = PB5;

const int SDA = PC4;
const int SCL = PC5;

//  32 Bytes worth of data
typedef struct tr_data {
  char tranText[DATA_SIZE];
  char recvText[DATA_SIZE];
} TR_Data;

void TWI_Init(uint8_t address);
void TWI_Receive(char *dest);

int main(void) {

  TR_Data data = {"Hello Master\n", ""};

  USART_Init(MYUBRR);

  stdin = &mystdin;
  stdout = &mystdout;

  printf("Hello There!\n");

  TWI_Init(170);

  while (1) {
    TWI_Receive(data.recvText);
  }
  return 0;
}

/*
 Function to setup the UNO as slave:
 */
void TWI_Init(uint8_t address) {
  // LSB --> 170
  TWAR = address;

  // Slave receiver mode setup from documentation:
  TWCR |= (1 << TWEA) | (1 << TWEN);
  TWCR &= ~(1 << TWSTA) & ~(1 << TWSTO);
}

/*
 Function to receive data from Master:
 */
void TWI_Receive(char *received) {

  uint8_t twi_stat, twi_idx = 0;

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
    received[twi_idx] = TWDR;
    twi_idx++;
  } else if ((twi_stat == 0x88) || (twi_stat == 0x98)) {
    received[twi_idx] = TWDR;
    twi_idx++;
  } else if ((twi_stat == 0xA0)) { // STOP signal
    TWCR |= (1 << TWINT);
  }
  if (DATA_SIZE <= twi_idx) {
    printf("%s", received);
    twi_idx = 0;
  }
}
