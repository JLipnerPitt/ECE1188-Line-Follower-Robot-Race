// BumpInt.c
// Runs on MSP432, interrupt version
// Provide low-level functions that interface bump switches on the robot.


// Negative logic bump sensors
// P4.7 Bump5, left side of robot
// P4.6 Bump4
// P4.5 Bump3
// P4.3 Bump2
// P4.2 Bump1
// P4.0 Bump0, right side of robot

#include <stdint.h>
#include "msp.h"
#include "../inc/Motor.h"

// Global variable and semaphore for bump sensor result
volatile uint8_t bumpSensorResult;
volatile uint8_t bumpSensorSemaphore;

// Function pointer for user-supplied collision handling function
/*typedef void (*CollisionHandler)(uint8_t);
static CollisionHandler collisionHandler = 0;*/

// Initialize Bump sensors
// Make six Port 4 pins inputs
// Activate interface pullup
// pins 7,6,5,3,2,0
// Interrupt on falling edge (on touch)
//void BumpInt_Init(void(*task)(uint8_t)){
void BumpInt_Init(void){
    // Configure bump sensor pins as inputs with internal pull-up resistors
    P4->SEL0 = 0x00;  // configure P4.7, P4.6, P4.5, P4.3, P4.2, P4.0 as GPIO
    P4->SEL1 = 0x00;  // configure P4.7, P4.6, P4.5, P4.3, P4.2, P4.0 as GPIO
    P4->DIR &= ~0xED;   // set P4.7, P4.6, P4.5, P4.3, P4.2, P4.0 as inputs
    P4->REN = 0xED;    // enable internal pull-up/pull-down resistors
    P4->OUT = 0xED;    //pins are pull up

    // Save the user-supplied collision handling function
    //collisionHandler = task;

    // Configure and enable interrupts for bump sensors
    P4->IES = 0xED;    // Interrupt on falling edge for P4.7, P4.6, P4.5, P4.3, P4.2, P4.0
    P4->IFG = 0x00;    // Clear interrupt flags
    P4->IE = 0xED;     // Enable interrupts for P4.7, P4.6, P4.5, P4.3, P4.2, P4.0

    //NVIC_EnableIRQ(PORT4_IRQn);  // Enable the interrupt in NVIC
    NVIC->IP[9] = (NVIC->IP[9]&0xFF00FFFF)|0x00400000; // priority 2
    NVIC->ISER[1] = 0x00000040;        // enable interrupt 35 in NVIC

    //bumpSensorResult = 0;


    //creating LED indicator for trigger
    P1->SEL0 &= ~0x01;
    P1->SEL1 &= ~0x01;                 // configure built-in LED1 as GPIO
    P1->DIR |= 0x01;                   // make built-in LED1 out
    P1->OUT &= ~0x01;



}
// Read current state of 6 switches
// Returns a 6-bit positive logic result (0 to 63)
// bit 5 Bump5
// bit 4 Bump4
// bit 3 Bump3
// bit 2 Bump2
// bit 1 Bump1
// bit 0 Bump0
uint8_t Bump_Read(void){
    // write this as part of Lab 14
    uint32_t B1, B2, B3, B4, B5 ,B6;

    // Read the state of the bump sensors and store the result
    B1 = (~(P4->IN)& 0x01);
    B2 = (~(P4->IN)& 0x04)>>1;
    B3 = (~(P4->IN)& 0x08)>>1;
    B4 = (~(P4->IN)& 0x20)>>2;
    B5 = (~(P4->IN)& 0x40)>>2;
    B6 = (~(P4->IN)& 0x80)>>2;

    //bumpSensorResult = ( B1 | B2 | B3 | B4 | B5 | B6 );

    // Set semaphore to indicate new data is available
    //bumpSensorSemaphore = 1;

    //return bumpSensorResult;
    return 0;
}

// we do not care about critical section/race conditions
// triggered on touch, falling edge
void PORT4_IRQHandler(void){
    // Read the 6-bit value from the sensors
    uint8_t status;

    status = P4->IV;

    if(status != 0x00){
        //bumpSensorResult = 1;
        Motor_Stop();
        //P1->OUT ^= 0x01;  //switching light for debugging
    }
    //bumpSensorResult = 0;
}
