// TimerA1.c
// Runs on MSP432
// Use Timer A1 in periodic mode to request interrupts at a particular
// period.

#include <stdint.h>
#include "msp.h"

void (*TimerA1Task)(void);   // user function

// ***************** TimerA1_Init ****************
// Activate Timer A1 interrupts to run user task periodically
// Inputs:  task is a pointer to a user function
//          period in units (24/SMCLK), 16 bits
// Outputs: none
// With SMCLK 12 MHz, period has units 2us
void TimerA1_Init(void(*task)(void), uint16_t period){

    TimerA1Task = task; // Save the user task

    __disable_irq(); // Disable interrupts while initializing

    // Configure Timer A1
    TIMER_A1->CTL = TIMER_A_CTL_SSEL__SMCLK |   // Use SMCLK as the clock source
                    TIMER_A_CTL_MC__UP |        // Use Up mode
                    TIMER_A_CTL_ID_0;           // No divider

    TIMER_A1->CCR[0] = period - 1; // Set the period
    TIMER_A1->CCTL[0] = TIMER_A_CCTLN_CCIE; // Enable Timer A1 interrupt

    __enable_irq(); // Enable interrupts globally
}

// ------------TimerA1_Stop------------
// Deactivate the interrupt running a user task periodically.
// Input: none
// Output: none
void TimerA1_Stop(void){

    __disable_irq(); // Disable interrupts while stopping

    TIMER_A1->CTL = 0; // Clear Timer A1 control register
    TIMER_A1->CCTL[0] &= ~TIMER_A_CCTLN_CCIE; // Disable Timer A1 interrupt

    __enable_irq(); // Enable interrupts globally
}

void TA1_0_IRQHandler(void){

    TIMER_A1->CCTL[0] &= ~TIMER_A_CCTLN_CCIFG; // Clear the interrupt flag
    (*TimerA1Task)(); // Run the user task
}
