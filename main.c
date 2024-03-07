#include <stdint.h>
#include "msp.h"
#include "CortexM.h"
#include "SysTickInts.h"
#include "Clock.h"
#include "Reflectance.h"
#include "Motor.h"
#include "PWM.h"
#include "BumpInt.h"

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
//#define backwards &FSM[14]
//#define back_left &FSM[15]
//#define back_right &FSM[16]

#define RED 0x01 // stop
#define GREEN 0x02 // forward
#define YELLOW 0x03 // left
#define PURPLE 0x05 // right
#define BLUE 0x04
#define WHITE 0x07
#define SKYBLUE 0x06

void (*motor_stop)(void) = &Motor_Stop;
void (*motor_forward)(uint16_t, uint16_t) = &Motor_Forward;
void (*motor_left)(uint16_t, uint16_t) = &Motor_Left;
void (*motor_right)(uint16_t, uint16_t) = &Motor_Right;
void (*motor_backward)(uint16_t, uint16_t) = &Motor_Backward;

//50% duty cycle is 7500
State_t FSM[14] ={
  {RED,0,0,10,{stop,fast_forward,forward,slow_forward,slow_left,left,left_45degrees,left_22degrees,slight_left,slow_right,right,right_45degrees,right_22degrees,slight_right}},

  // fast forward
  {GREEN,4500,4000,15,{stop,fast_forward,forward,slow_forward,slow_left,left,left_45degrees,left_22degrees,slight_left,slow_right,right,right_45degrees,right_22degrees,slight_right}},

  // forward
  {GREEN,4500,4000,15,{stop,fast_forward,forward,slow_forward,slow_left,left,left_45degrees,left_22degrees,slight_left,slow_right,right,right_45degrees,right_22degrees,slight_right}},

  // slow forward
  {GREEN,3250,2750,5,{stop,fast_forward,forward,slow_forward,slow_left,left,left_45degrees,left_22degrees,slight_left,slow_right,right,right_45degrees,right_22degrees,slight_right}},

  // slow left
  {BLUE,4500,4000,10,{stop,fast_forward,forward,slow_forward,slow_left,left,left_45degrees,left_22degrees,slight_left,slow_right,right,right_45degrees,right_22degrees,slight_right}},

  // left
  {YELLOW,5750,7000,10,{stop,fast_forward,forward,slow_forward,slow_left,left,left_45degrees,left_22degrees,slight_left,slow_right,right,right_45degrees,right_22degrees,slight_right}},

  // 45degree left
  {BLUE,2500,7250,10,{stop,fast_forward,forward,slow_forward,slow_left,left,left_45degrees,left_22degrees,slight_left,slow_right,right,right_45degrees,right_22degrees,slight_right}},

  // 22.5degree left
  {YELLOW,1750,3000,10,{stop,fast_forward,forward,slow_forward,slow_left,left,left_45degrees,left_22degrees,slight_left,slow_right,right,right_45degrees,right_22degrees,slight_right}},

  // slight left
  {YELLOW,2250,2500,10,{stop,fast_forward,forward,slow_forward,slow_left,left,left_45degrees,left_22degrees,slight_left,slow_right,right,right_45degrees,right_22degrees,slight_right}},

  // slow right
  {SKYBLUE,4750,4000,10,{stop,fast_forward,forward,slow_forward,slow_left,left,left_45degrees,left_22degrees,slight_left,slow_right,right,right_45degrees,right_22degrees,slight_right}},

  // right
  {PURPLE,7000,5250,10,{stop,fast_forward,forward,slow_forward,slow_left,left,left_45degrees,left_22degrees,slight_left,slow_right,right,right_45degrees,right_22degrees,slight_right}},

  // 45degree right
  {SKYBLUE,7500,2250,10,{stop,fast_forward,forward,slow_forward,slow_left,left,left_45degrees,left_22degrees,slight_left,slow_right,right,right_45degrees,right_22degrees,slight_right}},

  // 22.5degree right
  {PURPLE,3500,1750,10,{stop,fast_forward,forward,slow_forward,slow_left,left,left_45degrees,left_22degrees,slight_left,slow_right,right,right_45degrees,right_22degrees,slight_right}},

  // slight right
  {PURPLE,2750,2000,10,{stop,fast_forward,forward,slow_forward,slow_left,left,left_45degrees,left_22degrees,slight_left,slow_right,right,right_45degrees,right_22degrees,slight_right}}
};

// global variables
uint8_t IRSensorInput, MainCount, index;
State_t *sensor_state;  // pointer to ir_sensor fsm

void Port2_Init(void) {
 P2->SEL0 &= ~0x07;
 P2->SEL1 &= ~0x07;
 P2->DIR |= 0x07;
 //P2->DS |= 0x07;
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
        //my edits
        if (index == 0) {
            Motor_Stop();
        }
        //
        if (index > 0 & index < 4) {
            Motor_Forward(sensor_state->leftwheel,sensor_state->rightwheel);
        }
        else if (index > 3 && index < 9) {
            Motor_Left(sensor_state->leftwheel,sensor_state->rightwheel);
        }
        else if (index > 9 && index < 14) {
            Motor_Right(sensor_state->leftwheel,sensor_state->rightwheel);
        }
        else {
            //my edit
            Motor_Stop();
            //Motor_Forward(1750, 2000);
            //
        }
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
    Port2_Init();
    Reflectance_Init();
    Motor_Init();
    BumpInt_Init();
    PWM_Init34(14999,0,0);
    sensor_state = stop;
    EnableInterrupts();

    while(1) {
      WaitForInterrupt();
    }
}
