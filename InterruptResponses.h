/*
 * Interrupt functions
 *
 *
 * @file        interruptResponses.h
 * @author      hustGs
 * @copyright   hustGs
 *
 */

#ifndef _INTERRUPTRESPONSES_H
#define _INTERRUPTRESPONSES_H

#include <stdint.h>

#define RX_MSG_OBJ_ID      (32)

extern uint16_t cpuTimer0IntCount;
extern uint16_t cpuTimer1IntCount;

__interrupt void canISR(void);

__interrupt void cpuTimer0ISR(void);

__interrupt void cpuTimer1ISR(void);

#endif
