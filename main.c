#include <stdint.h>
#include "msp.h"
#include "CortexM.h"
#include "SysTickInts.h"
#include "Clock.h"
#include "Reflectance.h"

/**
 * main.c
 */

struct State {
    uint32_t output;
    uint32_t delay;
    const struct State *next_state[12];
};
typedef const struct State State_t; // lets us create state structures with the State_t initializer

// all possible next states
#define FSM[0] stop;
#define FSM[1] forward;
#define FSM[2] slow_forward;
#define FSM[3] fast_forward;
#define FSM[4] left;
#define FSM[5] slow_left;
#define FSM[6] 45degree_left;
#define FSM[7] 22degree_left;
#define FSM[8] right;
#define FSM[9] slow_right;
#define FSM[10] 45degree_right;
#define FSM[11] 22degree_right;

#define RED 0x01; // stop
#define GREEN 0x02; // forward
#define PINK 0x05; // left
#define SKYBLUE 0x06; // right


// global variables
uint8_t IRSensorInput, MainCount, index;
State_t *sensor_state;  // pointer to ir_sensor fsm

void Port2_Init(void) {
 P2->SEL0 &= ~0x07;
 P2->SEL1 &= ~0x07;
 P2->DIR |= 0x07;
 P2->DS |= 0x07;
 P2->OUT &= ~0x07;
}

void SysTick_Handler(void) {
    if (MainCount == 9) {
        Reflectance_Start();
    }
    else if (MainCount == 10) {
        IRSensorInput = Reflectance_End();

        // sensor output
        index = Num_Of_On_Sensors(IRSensorInput);
        sensor_state = sensor_state->next_state[index];
        P2->OUT = sensor_state->output;
        Clock_Delay1us(sensor_state->delay);
        MainCount = 0;
        return;
    }
    MainCount++;
}

int main(void)
{
    Clock_Init48MHz();      // running on crystal
    MainCount = 0; // semaphore
    SysTick_Init(48000,2);  // set up SysTick for 1000 Hz interrupts
    Reflectance_Init();
    Port2_Init();
    //sensor_state = slow_forward;
    EnableInterrupts();
}
