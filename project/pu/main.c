#include "timer1.h"
#define F_CPU 16000000UL
#define SLAVE_ADDRESS 170
#define BAUD 9600
#define MYUBRR F_CPU / 16 / BAUD - 1

#define NOTE_B0  31
#define NOTE_C1  33
#define NOTE_CS1 35
#define NOTE_D1  37
#define NOTE_DS1 39
#define NOTE_E1  41
#define NOTE_F1  44
#define NOTE_FS1 46
#define NOTE_G1  49
#define NOTE_GS1 52
#define NOTE_A1  55
#define NOTE_AS1 58
#define NOTE_B1  62
#define NOTE_C2  65
#define NOTE_CS2 69
#define NOTE_D2  73
#define NOTE_DS2 78
#define NOTE_E2  82
#define NOTE_F2  87
#define NOTE_FS2 93
#define NOTE_G2  98
#define NOTE_GS2 104
#define NOTE_A2  110
#define NOTE_AS2 117
#define NOTE_B2  123
#define NOTE_C3  131
#define NOTE_CS3 139
#define NOTE_D3  147
#define NOTE_DS3 156
#define NOTE_E3  165
#define NOTE_F3  175
#define NOTE_FS3 185
#define NOTE_G3  196
#define NOTE_GS3 208
#define NOTE_A3  220
#define NOTE_AS3 233
#define NOTE_B3  247
#define NOTE_C4  262
#define NOTE_CS4 277
#define NOTE_D4  294
#define NOTE_DS4 311
#define NOTE_E4  330
#define NOTE_F4  349
#define NOTE_FS4 370
#define NOTE_G4  392
#define NOTE_GS4 415
#define NOTE_A4  440
#define NOTE_AS4 466
#define NOTE_B4  494
#define NOTE_C5  523
#define NOTE_CS5 554
#define NOTE_D5  587
#define NOTE_DS5 622
#define NOTE_E5  659
#define NOTE_F5  698
#define NOTE_FS5 740
#define NOTE_G5  784
#define NOTE_GS5 831
#define NOTE_A5  880
#define NOTE_AS5 932
#define NOTE_B5  988
#define NOTE_C6  1047
#define NOTE_CS6 1109
#define NOTE_D6  1175
#define NOTE_DS6 1245
#define NOTE_E6  1319
#define NOTE_F6  1397
#define NOTE_FS6 1480
#define NOTE_G6  1568
#define NOTE_GS6 1661
#define NOTE_A6  1760
#define NOTE_AS6 1865
#define NOTE_B6  1976
#define NOTE_C7  2093
#define NOTE_CS7 2217
#define NOTE_D7  2349
#define NOTE_DS7 2489
#define NOTE_E7  2637
#define NOTE_F7  2794
#define NOTE_FS7 2960
#define NOTE_G7  3136
#define NOTE_GS7 3322
#define NOTE_A7  3520
#define NOTE_AS7 3729
#define NOTE_B7  3951
#define NOTE_C8  4186
#define NOTE_CS8 4435
#define NOTE_D8  4699
#define NOTE_DS8 4978

#define badApple = [NOTE_DS4, NOTE_ES4, NOTE_FS4, NOTE_GS4, NOTE_AS4, NOTE_AS4, NOTE_DS5, NOTE_CS5, NOTE_AS4, NOTE_AS4, NOTE_DS4, NOTE_DS4, NOTE_AS4, NOTE_GS4, NOTE_FS4, NOTE_ES4, NOTE_DS4, NOTE_ES4, NOTE_FS4, NOTE_GS4, NOTE_AS4, NOTE_AS4, NOTE_GS4, NOTE_FS4, NOTE_ES4, NOTE_DS4, NOTE_ES4, NOTE_FS4, NOTE_ES4, NOTE_DS4, NOTE_D4, NOTE_ES4, NOTE_DS4, NOTE_ES4, NOTE_FS4, NOTE_GS4, NOTE_AS4, NOTE_AS4, NOTE_DS5, NOTE_DS5, NOTE_AS4, NOTE_AS4, NOTE_DS4, NOTE_DS4, NOTE_AS4, NOTE_GS4, NOTE_FS4, NOTE_ES4, NOTE_DS4, NOTE_ES4, NOTE_FS4, NOTE_GS4, NOTE_AS4, NOTE_AS4, NOTE_GS4, NOTE_FS4, NOTE_ES4, NOTE_ES4, NOTE_FS4, NOTE_FS4, NOTE_GS4, NOTE_GS4, NOTE_AS4, NOTE_AS4]

#define DATA_SIZE 16

#define PIR_SENSE 0
#define ALARM_ON 1
#define ALARM_OFF 2
#define WAIT_CORRECT_KEY 3
#define IDLE 4

#define NOTE_64_Cs3 906
#define NOTE_64_C5 856
#define NOTE_64_200 625

#include <avr/interrupt.h>
// #include <avr/io.h>
#include <stdint.h>
#include <stdio.h>
// #include <stdlib.h>
#include <util/delay.h>

#include "lcd.h"
#include "timer1.h"
#include "uart.h"

// State machine variable
volatile uint8_t state = 0;
volatile uint8_t data_incoming = 0;
volatile uint8_t is_buzz_on = 0;
volatile uint8_t isCodeCorrect = 0;

// Find out what is the system state.
void getSystemState(char *data);

// Compare two char arrays.
int8_t strcomp(char *a, char *b);

/*
 * I2C / TWI communication initialization with Master.
 */
void I2C_InitSlaveReceiver(uint8_t address);

/*
 * I2C / TWI receive from Master.
 */
void I2C_Receive(char *dest);

// Annoying sound maker
void singASong();
void tone(uint16_t freq_top);

// LCD Display PINS WARNING remember to change from lcd.h also
const int LCD_RS = PB2;
const int LCD_RW = PB3;
const int LCD_EN = PB4;

const int LCD_D4 = PD4;
const int LCD_D5 = PD5;
const int LCD_D6 = PD6;
const int LCD_D7 = PD7;

// Buzzer
const int BUZZER = PB1;

void enableExternalInterrupt() {
  // External interrupt Control Register for when Master tries to transmit data
  // to slave. UNO Doc. 70-72 table 13-1 and 2
  EICRA |= (1 << ISC01);
  EIMSK |= (1 << INT0);
}

void disableExternalInterrupt() {
  EICRA = 0;
  EIMSK = 0;
}
// Interrupt routine for timer
ISR(TIMER1_COMPA_vect) { TCNT1 = 0; }

// Interrupt routine for I2C transfer
ISR(INT0_vect) {
  disableExternalInterrupt();
  _delay_ms(1);
  data_incoming = 1;
}

int main(void) {
  // // LCD PINS
  // OUTPUTS CONTROL
  DDRB |= (1 << LCD_RS) | (1 << LCD_RW) | (1 << LCD_EN);
  // OUTPUTS DATA
  DDRD |= (1 << LCD_D4) | (1 << LCD_D5) | (1 << LCD_D6) | (1 << LCD_D7);

  // Buzzer OUTPUT
  DDRB |= (1 << BUZZER);

  // Initialize empty recv char array
  char recv[DATA_SIZE] = {'\0'};

  // Init debug communication Through USB
  USART_Init(MYUBRR);
  stdin = &mystdin;
  stdout = &mystdout;

  sei();
  enableExternalInterrupt();

  // Init LCD display
  lcd_init(LCD_DISP_ON);
  lcd_clrscr();

  lcd_puts("Welcome!");
  // Test print to putty
  printf("Hello There!\n");

  // Setup TWI communication with Master
  I2C_InitSlaveReceiver(SLAVE_ADDRESS);
  // lcd_clrscr();

  while (1) {
    if (data_incoming) {
      I2C_Receive(recv);

      data_incoming = 0;
      enableExternalInterrupt();
    }

    lcd_clrscr();
    lcd_puts(recv);
    _delay_ms(10);
    // getSystemState(recv);
    //  if (is_buzz_on == 1) {
    //    singASong();
    //  }
  }

  return 0;
}

/*
 * Check the system status based on received data
 *
 * @param char *data
 * @returns void
 */
void getSystemState(char *data) {
  if (data[0] == '0') {
    is_buzz_on = 0;

  } else if (data[0] == '1') {
    is_buzz_on = 1;

  } else if (data[0] == '2') {
    isCodeCorrect = 0;

  } else if (data[0] == '3') {
    isCodeCorrect = 1;
  }
}

int8_t strcomp(char *a, char *b) {
  uint8_t i = 0;
  while ((a[i] != '\0') && (b[i] != '\0')) {
    if (a[i] != b[i]) {
      return 0;
    }
    i++;
  }
  return 1;
}

/*
 * Function to setup device as slave receiver.
 *
 * Follows closely Atmel Mega 2560 document of which page 253 - 254 contain
 * relevant information. Figure 24-15.
 */
void I2C_InitSlaveReceiver(uint8_t address) {
  // Devices own Slave Address
  TWAR = address;

  // Slave receiver mode setup.
  TWCR |= (1 << TWEA) | (1 << TWEN);
  // Explicitly set to 0
  TWCR &= ~(1 << TWSTA) & ~(1 << TWSTO);
  // Eqv: TWCR = 0b01000100;
}

/*
 Function to receive data from Master:
 */
void I2C_Receive(char *received) {
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
  while ((twi_stat == 0x80) || (twi_stat == 0x90)) {
    received[twi_idx] = TWDR;
    twi_idx++;

    if ((received[twi_idx - 1] == '\0') || (twi_idx >= DATA_SIZE)) {
      break;
    }

    // Reset the TWEA and TWEN, create ACK
    TWCR |= (1 << TWINT) | (1 << TWEA) | (1 << TWEN);

    // Wait for TWINT to set
    while (!(TWCR & (1 << TWINT)))
      ;

    // Status update
    twi_stat = (TWSR & 0xF8);
  }

  // check for NOT ACK or general Call
  if ((twi_stat == 0x88) || (twi_stat == 0x98)) {
    received[twi_idx] = TWDR;
    twi_idx++;
  } else if ((twi_stat == 0xA0)) { // STOP signal
    TWCR |= (1 << TWINT);
  }
  if (strcomp(received, "ALARM ON\0")) {
    is_buzz_on = 1;
  }
}

/*
 * A very annoying tone sequence repeated for the time being 10 times...
 *
 */
void singASong() {
  TIMER1_Init_Mode_9();
  TIMER1_SetPrescaler(PS_64);
  for (int i = 0; i < 5; i++) {
    tone(NOTE_64_Cs3);
    tone(NOTE_64_C5);
    tone(NOTE_64_Cs3);
    tone(NOTE_64_C5);
    tone(NOTE_64_Cs3);
    tone(NOTE_64_200);
    tone(NOTE_64_Cs3);
    tone(NOTE_64_200);
  }
  TIMER1_Clear();
}

void tone(uint16_t freq_top) {
  if (!is_buzz_on || data_incoming) {
    return;
  }
  TIMER1_SetTarget(freq_top);
  _delay_ms(200);
}
