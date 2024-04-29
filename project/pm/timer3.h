#ifndef _TIMER3_H
#define _TIMER3_H

// Macros for prescaler values
#define PS_1 1
#define PS_8 8
#define PS_64 64
#define PS_256 256
#define PS_1024 1024

// TOP = F_CPU / (prescaler * frequency) - 1
#define SECOND_1024 15624

#include <stdint.h>

/*
 Initialize Mega timer 3 to mode 4 (CTC)
 *
 * @param None
 * @returns void
 */
void TIMER3_Init_CTC();

/*
 * Clear Mega timer 3 registers
 *
 * @param None
 * @returns void
 */
void TIMER3_Clear();

/*
 * Set target TOP value of Mega Timer 3
 *
 * @param uint16_t TOP value to be set to OCR3A register.
 * @returns void
 */
void TIMER3_SetTarget(uint16_t value);

/*
 * Set Mega Timer 3 prescaler value 1; 8; 64; 256; 1024
 *
 * @param uint16_t prescaler value to be used for timer
 * @returns void
 */
void TIMER3_SetPrescaler(uint16_t prescaler);

/*
 * Function to set the CTC timer 3 interval to every second.
 *
 * @param None
 * @returns void
 */
void TIMER3_SetIntervalSecond();

#endif // _TIMER3_H
