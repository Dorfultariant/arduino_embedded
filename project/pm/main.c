#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <util/delay.h>

#include "keypad.h"
#include "timer3.h"
#include "uart.h"

#define F_CPU 16000000UL
#define BAUD 9600
#define MYUBRR F_CPU / 16 / BAUD - 1

#define CODE_ARRAY_LENGTH 5
#define DATA_SIZE 16

#define SLAVE_ADDRESS 170

/*
 * g_State machine g_states:
 *
 * 0 PIR Sense
 * 1 TIMER ON
 * 2 KEY_INSERTION
 * 3 PIR TIMER ALARM OFF
 */
#define PIR_SENSE 0
#define TIMER_ON 1
#define KEY_INSERTION 2
#define PIR_TIMER_ALARM_OFF 3

// Countdown in seconds
#define ALARM_TIMER 10

// Signals to cause an alarm on UNO
const char wrong_code[] = "W";
const char times_up[] = "T";

// Correct signal
const char correct_code[] = "C";

// Timer is started signal
const char movement[] = "M";

// Reset signal
const char rearm[] = "R";

// All pins that are used on the Mega
const int PIR_SIGNAL = PE3;
const int REARM_BTN = PG5;
const int ALARM_LED = PH3;
const int I2C_ERROR = PH4;
const int I2C_OK = PH5;

// State machine state
volatile int8_t g_state = 0;

/*
 * Keypad code reading and verification.
 */
volatile uint8_t g_is_code_valid = 0;

/*
 Timer 3 counter for 10 second countdown from PIR sense.
 */
volatile uint16_t g_second_counter = 0;

/*
 * I2C / TWI transmission initialization with 400 kHz clock.
 * @param None
 *
 * @returns void
 */
static void i2c_init();

/*
 * I2C / TWI Transmission from Master to Slave address with data.
 * @param uint8_t address of the slave to transmit to.
 * @param char *data to be sent to the slave.
 *
 * @returns void
 */
static void i2c_transmit(uint8_t address, const char *data);

/*
 * Keypad code reading and verification.
 */
static int8_t read_keypad_code(char *dest, uint8_t code_len);
static int verify_code(char *to_be_checked, char *correct);

int main(void)
{
    // Define the correct keycode.
    char correct_code[CODE_ARRAY_LENGTH] = "0423";

    // Initialize empty code given by user.
    char users_code[CODE_ARRAY_LENGTH] = {'\0'};

    /*
     * Data which is send to the UNO, this has the following format:
     * [ #CODE ] or [ # ] where # represents g_states W, C, T, R, M declared as
     * global constants.
     */
    char transfer_data[DATA_SIZE] = {'\0'};

    // Output demo for alarm buzzer (currently RED LED)
    DDRH |= (1 << ALARM_LED) | (1 << I2C_ERROR) | (1 << I2C_OK);

    // PIR sensor input upon movement
    DDRE &= ~(1 << PIR_SIGNAL);

    // Input pin for rearming the system.
    DDRG &= ~(1 << REARM_BTN);

    // Initialize connection through USB for debugging.
    usart_init(MYUBRR);
    stdin = &mystdin;
    stdout = &mystdout;

    // Keypad initialization for getting users input from keypad.
    KEYPAD_Init();

    // Main logic loop, g_state machine.
    while (1) {
        switch (g_state) {
        case PIR_SENSE:
            // If PIR senses movement move to TIMER_ON g_state and sent g_state
            // information to UNO
            if (PINE & (1 << PIR_SIGNAL)) {
                g_state = TIMER_ON;
                i2c_init();
                i2c_transmit(SLAVE_ADDRESS, movement);
            }
            break;

        case TIMER_ON:
            // Initializing timer 3
            timer3_init_ctc();

            // Setting the timer 3 to interrupt every second
            timer3_set_interval_second();

            // Go wait for correct user input.
            g_state = KEY_INSERTION;
            break;

        case KEY_INSERTION:
            // Get keycode from user
            read_keypad_code(users_code, CODE_ARRAY_LENGTH - 1);

            // Verify the codes correctness
            g_is_code_valid = verify_code(users_code, correct_code);

            // Case correct code
            if (g_is_code_valid) {
                // Clear timer, just to be sure
                timer3_clear();

                // Clear array so no unintended bytes are sent
                for (uint8_t idx = 0; DATA_SIZE > idx; idx++) {
                    transfer_data[idx] = '\0';
                }

                // Concatenate the data to be sent
                strcat(transfer_data, correct_code);
                strcat(transfer_data, users_code);

                // Turn off alarm led
                PORTH &= ~(1 << ALARM_LED);

                // Transmit information to Slave
                i2c_init();
                i2c_transmit(SLAVE_ADDRESS, transfer_data);

                // Move to the final g_state
                g_state = PIR_TIMER_ALARM_OFF;
            }

            // Case wrong code
            else {

                // Clear array of unintended bytes
                for (uint8_t idx = 0; DATA_SIZE > idx; idx++) {
                    transfer_data[idx] = '\0';
                }

                // Turn the Alarm led On
                PORTH |= (1 << ALARM_LED);

                // Concatenate the data to be sent
                strcat(transfer_data, wrong_code);
                strcat(transfer_data, users_code);

                // Initialize connection and Send data
                i2c_init();
                i2c_transmit(SLAVE_ADDRESS, transfer_data);
            }
            break;

        case PIR_TIMER_ALARM_OFF: // Idle g_state
            // Waiting for system rearming.
            /*
             * We decided to allow rearming only in the case where user gives
             * correct keycode.
             */
            if (PING & (1 << REARM_BTN)) {
                // Resetting variables
                g_state = PIR_SENSE;
                g_is_code_valid = 0;
                g_second_counter = 0;

                // Clear transfer_data
                for (uint8_t idx = 0; DATA_SIZE > idx; idx++) {
                    transfer_data[idx] = '\0';
                }

                // Send g_state information to UNO
                i2c_init();
                i2c_transmit(SLAVE_ADDRESS, rearm);
            }
            break;
        }
    }

    return 0;
}

/*
 * I2C / TWI transmission initialization with 400 kHz clock.
 * @param None
 *
 * @returns void
 */
static void i2c_init()
{
    // Bit Rate generator setup to 400 000 Hz -> F_CPU / (16 + 2 * TWBR *
    // 4^(TWSR):
    TWSR = 0x00;         // Prescaler to 1
    TWBR = 0x03;         // 3x multiplier to achieve 400 kHz
    TWCR |= (1 << TWEN); // TWI enable
}

/*
 * I2C / TWI Transmission from Master to Slave address with data.
 * @param uint8_t address of the slave to transmit to.
 * @param char *data to be sent to the slave.
 *
 * @returns void
 */
static void i2c_transmit(uint8_t address, const char *data)
{
    uint8_t twi_stat = 0;

    // Start transmission:
    TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);

    while (!(TWCR & (1 << TWINT))) {
        ;
    }

    // Read status from TWI status register
    twi_stat = (TWSR & 0xF8);

    // Slave address
    TWDR = address;

    // Clear TWINT to start transmit to slave + write
    TWCR = (1 << TWINT) | (1 << TWEN);

    // Wait TWINT to set
    while (!(TWCR & (1 << TWINT))) {
        ;
    }

    twi_stat = (TWSR & 0xF8);

    // Check if the connection to Slave is successful (Slave returns ACK)
    if ((twi_stat != 0x18) && (twi_stat != 0x40)) {
        // Set error led ON
        PORTH |= (1 << I2C_ERROR);
        // Verify that OK led is OFF
        PORTH &= ~(1 << I2C_OK);
        return;
    }

    // Set ok led ON
    PORTH |= (1 << I2C_OK);
    // Set error led OFF
    PORTH &= ~(1 << I2C_ERROR);

    // Send data byte at a time until either 16 bytes has been sent or data
    // array has reached its end.
    for (uint8_t twi_d_idx = 0;
         (DATA_SIZE > twi_d_idx) && ('\0' != data[twi_d_idx]); twi_d_idx++) {
        TWDR = data[twi_d_idx];

        // Reset TWINT to transmit data
        TWCR = (1 << TWINT) | (1 << TWEN);

        // Wait for TWINT to set
        while (!(TWCR & (1 << TWINT))) {
            ;
        }

        twi_stat = (TWSR & 0xF8);
    }

    // STOP transmission
    TWCR = (1 << TWINT) | (1 << TWSTO) | (1 << TWEN);
}

/*
 * Stores users given key code from the keypad to the destination array to be
 * verified. When user has given code_len amount of digits (only last ones are
 * stored) and user gives [A]ccept the loop ends and the code is verified.
 * User can also give [D]elete to remove previous digit from storage.
 *
 * @param char *dest        Destination array
 * @param uint8_t code_len  Destination array length
 *
 * @returns 0 for success
 */
static int8_t read_keypad_code(char *dest, uint8_t code_len)
{
    int index = 0;
    char chr = 0;

    // Ensure that we are working with empty array:
    for (uint8_t idx = 0; idx < code_len; idx++) {
        dest[idx] = '\0';
    }

    // Get users input until the user presses [A]ccept on the keypad and
    // user has given long enough password.
    for (;;) {
        chr = KEYPAD_GetKey();

        // Check for digit in range 0 - 9
        if (('0' <= chr) && ('9' >= chr)) {
            // We want to store the last code_len amount of digits
            if (index >= code_len) {
                memmove(dest, dest + 1, code_len - 1);
                index--;
            }
            dest[index++] = chr;
        }

        // Allow the [D]eletion of previous char if it exists
        else if (chr == 'D' && index > 0) {
            index--;
        }

        // End point to exit function if enough digits given and [A]ccept
        else if (('A' == chr) && (index == code_len)) {
            break;
        }
    }

    // Make sure that the dest ends.
    dest[code_len] = '\0';
    return 0;
}

/*
 * This function compares two given arrays until either of them are
 * checked to '\0'. Also strcmp() could be used since we are including string.h
 * anyways, but this works just fine.
 *
 * @param char *to_be_checked is the array to be compared
 * @param char *correct is the array to be compared against
 *
 * @returns int 1 for arrays being identical in the shortest arrays scope and 0
 * if the arrays do not match in the shortest arrays scope.
 */
static int verify_code(char *to_be_checked, char *correct)
{
    uint8_t idx = 0;
    while (('\0' != to_be_checked[idx]) && ('\0' != correct[idx])) {
        if (to_be_checked[idx] != correct[idx]) {
            return 0;
        }
        idx++;
    }
    return 1;
}

/*
 * Interrupt Service Routine for Timer 3.
 * Causes alarm if 10 seconds have passed.
 */
ISR(TIMER3_COMPA_vect)
{
    TCNT3 = 0;
    g_second_counter++;
    if (ALARM_TIMER <= g_second_counter) {
        // Led indicating ALARM is ON
        PORTH |= (1 << ALARM_LED);

        // Timer is cleared so this interrupt does not fire again until reset
        timer3_clear();
        g_second_counter = 0;

        // Send system g_state information to UNO
        i2c_init();
        i2c_transmit(SLAVE_ADDRESS, times_up);
    }
}

/* EOF */
