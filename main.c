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
    //uint32_t leftwheel;
    //uint32_t rightwheel;
    uint32_t delay;
    const struct State *next_state[12];
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
#define slow_right &FSM[8]
#define right &FSM[9]
#define right_45degrees &FSM[10]
#define right_22degrees &FSM[11]

#define RED 0x01 // stop
#define GREEN 0x02 // forward
#define BLUE 0x04 // left
#define PINK 0x05 // right

State_t FSM[12] ={
  {0x01,10,{stop,fast_forward,forward,slow_forward,slow_left,left,left_45degrees,left_22degrees,slow_right,right,right_45degrees,right_22degrees}},
  {0x02,10,{stop,fast_forward,forward,slow_forward,slow_left,left,left_45degrees,left_22degrees,slow_right,right,right_45degrees,right_22degrees}},
  {0x02,10,{stop,fast_forward,forward,slow_forward,slow_left,left,left_45degrees,left_22degrees,slow_right,right,right_45degrees,right_22degrees}},
  {0x02,10,{stop,fast_forward,forward,slow_forward,slow_left,left,left_45degrees,left_22degrees,slow_right,right,right_45degrees,right_22degrees}},
  {0x05,10,{stop,fast_forward,forward,slow_forward,slow_left,left,left_45degrees,left_22degrees,slow_right,right,right_45degrees,right_22degrees}},
  {0x05,10,{stop,fast_forward,forward,slow_forward,slow_left,left,left_45degrees,left_22degrees,slow_right,right,right_45degrees,right_22degrees}},
  {0x05,10,{stop,fast_forward,forward,slow_forward,slow_left,left,left_45degrees,left_22degrees,slow_right,right,right_45degrees,right_22degrees}},
  {0x05,10,{stop,fast_forward,forward,slow_forward,slow_left,left,left_45degrees,left_22degrees,slow_right,right,right_45degrees,right_22degrees}},
  {0x04,10,{stop,fast_forward,forward,slow_forward,slow_left,left,left_45degrees,left_22degrees,slow_right,right,right_45degrees,right_22degrees}},
  {0x04,10,{stop,fast_forward,forward,slow_forward,slow_left,left,left_45degrees,left_22degrees,slow_right,right,right_45degrees,right_22degrees}},
  {0x04,10,{stop,fast_forward,forward,slow_forward,slow_left,left,left_45degrees,left_22degrees,slow_right,right,right_45degrees,right_22degrees}},
  {0x04,10,{stop,fast_forward,forward,slow_forward,slow_left,left,left_45degrees,left_22degrees,slow_right,right,right_45degrees,right_22degrees}}
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
            uint32_t i, j, temp, length;
            uint32_t a[100]={5000,5308,5614,5918,6219,6514,
            6804,7086,7361,7626,7880,8123,8354,8572,8776,8964,9137,
            9294,9434,9556,9660,9746,9813,9861,9890,9900,9890,9861,
            9813,9746,9660,9556,9434,9294,9137,8964,8776,8572,8354,
            8123,7880,7626, 7361,7086,6804,6514,6219,5918,5614,
            5308,5000,4692,4386,4082,3781,3486,3196,2914,2639,2374,
            2120,1877,1646,1428,1224,1036,863,706,566,444,340,254,
            187,139,110,100,110,139,187,254,340,444,566,706,863,
            1036, 1224, 1428, 1646,1877,2120,2374,2639,2914,
            3196,3486,3781,4082,4386,4692};
            length = 100;
            for (i = 0; i < length; i++) {
                for (j = 0; j < length - i - 1; j++) {
                    if (a[j + 1] < a[j]) {
                        temp = a[j];
                        a[j] = a[j + 1];
                        a[j + 1] = temp;
                    }
                }
            }
        }
    }
