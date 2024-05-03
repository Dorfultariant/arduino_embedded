#ifndef _TIMER1_H
#define _TIMER1_H

// Macros for prescaler values
#define PS_1 1
#define PS_8 8
#define PS_64 64
#define PS_256 256
#define PS_1024 1024

#include <stdint.h>

/*
 * Initialize UNO timer 1 to mode 9 which is PWM, Phase and frequency correct,
 * OCR1A
 *
 * @param None
 * @returns Void
 */
void timer1_init_mode_9();

/*
 * Clear UNO timer 1 registers
 *
 * @param None
 * @returns void
 */
void timer1_clear();

/*
 * Set target TOP value of UNO Timer 1
 *
 * @param uint16_t value of target TOP.
 * @returns Void
 */
void timer1_set_target(uint16_t value);

/*
 * Set UNO Timer 1 prescaler value 1; 8; 64; 256; 1024
 *
 * @param None
 * @returns Void
 */
void timer1_set_prescaler(uint16_t prescaler);

#endif // _TIMER1_H
