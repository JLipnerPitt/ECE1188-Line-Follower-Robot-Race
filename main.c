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
    uint8_t output;
    uint16_t leftwheel;
    uint16_t rightwheel;
    uint8_t delay;
    const struct State *next_state[14];
};
typedef const struct State State_t; // lets us create state structures with the State_t initializer

// all possible next states
#define stop &FSM[0]
#define fast_forward &FSM[1]
#define forward &FSM[2]
#define slow_forward &FSM[3]
#define slow_left &FSM[4]
#define left &FSM[5]
#define left_45degrees &FSM[6]
#define left_22degrees &FSM[7]
#define slight_left &FSM[8]
#define slow_right &FSM[9]
#define right &FSM[10]
#define right_45degrees &FSM[11]
#define right_22degrees &FSM[12]
#define slight_right &FSM[13]

#define RED 0x01 // stop
#define GREEN 0x02 // forward
#define YELLOW 0x03 // left
#define PURPLE 0x05 // right

//50% duty cycle is 7500
State_t FSM[14] ={
  {RED,0,0,10,{stop,fast_forward,forward,slow_forward,slow_left,left,left_45degrees,left_22degrees,slight_left,slow_right,right,right_45degrees,right_22degrees,slight_right}},
  {GREEN,7500,7500,10,{stop,fast_forward,forward,slow_forward,slow_left,left,left_45degrees,left_22degrees,slight_left,slow_right,right,right_45degrees,right_22degrees,slight_right}},
  {GREEN,4000,4000,10,{stop,fast_forward,forward,slow_forward,slow_left,left,left_45degrees,left_22degrees,slight_left,slow_right,right,right_45degrees,right_22degrees,slight_right}},
  {GREEN,1500,1500,10,{stop,fast_forward,forward,slow_forward,slow_left,left,left_45degrees,left_22degrees,slight_left,slow_right,right,right_45degrees,right_22degrees,slight_right}},
  {YELLOW,1500,4000,10,{stop,fast_forward,forward,slow_forward,slow_left,left,left_45degrees,left_22degrees,slight_left,slow_right,right,right_45degrees,right_22degrees,slight_right}},
  {YELLOW,6000,6000,10,{stop,fast_forward,forward,slow_forward,slow_left,left,left_45degrees,left_22degrees,slight_left,slow_right,right,right_45degrees,right_22degrees,slight_right}},
  {YELLOW,1500,6000,10,{stop,fast_forward,forward,slow_forward,slow_left,left,left_45degrees,left_22degrees,slight_left,slow_right,right,right_45degrees,right_22degrees,slight_right}},
  {YELLOW,1500,4500,10,{stop,fast_forward,forward,slow_forward,slow_left,left,left_45degrees,left_22degrees,slight_left,slow_right,right,right_45degrees,right_22degrees,slight_right}},
  {YELLOW,6500,7500,10,{stop,fast_forward,forward,slow_forward,slow_left,left,left_45degrees,left_22degrees,slight_left,slow_right,right,right_45degrees,right_22degrees,slight_right}},
  {PURPLE,4000,1500,10,{stop,fast_forward,forward,slow_forward,slow_left,left,left_45degrees,left_22degrees,slight_left,slow_right,right,right_45degrees,right_22degrees,slight_right}},
  {PURPLE,6000,6000,10,{stop,fast_forward,forward,slow_forward,slow_left,left,left_45degrees,left_22degrees,slight_left,slow_right,right,right_45degrees,right_22degrees,slight_right}},
  {PURPLE,6000,1500,10,{stop,fast_forward,forward,slow_forward,slow_left,left,left_45degrees,left_22degrees,slight_left,slow_right,right,right_45degrees,right_22degrees,slight_right}},
  {PURPLE,4500,1500,10,{stop,fast_forward,forward,slow_forward,slow_left,left,left_45degrees,left_22degrees,slight_left,slow_right,right,right_45degrees,right_22degrees,slight_right}},
  {PURPLE,7500,6500,10,{stop,fast_forward,forward,slow_forward,slow_left,left,left_45degrees,left_22degrees,slight_left,slow_right,right,right_45degrees,right_22degrees,slight_right}}
};

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
        index = decision(IRSensorInput);
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
    sensor_state = stop;
    EnableInterrupts();

    while(1) {
      WaitForInterrupt();

    }
}
