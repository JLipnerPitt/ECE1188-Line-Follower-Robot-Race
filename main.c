#include <stdint.h>
#include "msp.h"
#include "CortexM.h"
#include "SysTickInts.h"
#include "Clock.h"
#include "Reflectance.h"
#include "Motor.h"
#include "PWM.h"

/**
 * main.c
 */

/*void (*motor_stop)(uint16_t, uint16_t) = &Motor_Stop;
void (*motor_forward)(uint16_t, uint16_t) = &Motor_Forward;
void (*motor_left)(uint16_t, uint16_t) = &Motor_Left;
void (*motor_right)(uint16_t, uint16_t) = &Motor_Right;
void (*motor_backward)(uint16_t, uint16_t) = &Motor_Backward;*/

// stores each motor action for a given state
void (*lookup_table[9])(uint16_t, uint16_t) = {
    Motor_Stop, Motor_Forward, Motor_Forward, Motor_Left, Motor_Right, Motor_Left, Motor_Right, Motor_Left, Motor_Right// ... more function pointers ...
};

struct State {
    uint8_t output;
    uint16_t leftwheel;
    uint16_t rightwheel;
    uint8_t delay;
    const struct State *next_state[4];
};
typedef const struct State State_t; // allows us to create state structures with the State_t initializer

// all possible next states
#define Stop &FSM[0]
#define Forward &FSM[1]
#define Slow_Forward &FSM[2]
#define Left_Turn &FSM[3]
#define Right_Turn &FSM[4]
#define Hard_Left &FSM[5]
#define Hard_Right &FSM[6]
#define Small_Left &FSM[7]
#define Small_Right &FSM[8]
/*#define Backward &FSM[9]
#define Back_Left &FSM[10]
#define Back_Right_45degrees &FSM[11]*/

#define RED 0x01 // stop
#define GREEN 0x02 // forward
#define YELLOW 0x03 // left
#define PURPLE 0x05 // right
#define BLUE 0x04
#define WHITE 0x07
#define SKYBLUE 0x06

//50% duty cycle is 7500
State_t FSM[9] ={

  // lost
  {RED,0,0,10,{Slow_Forward,Left_Turn,Right_Turn,Stop}},

  // forward
  {GREEN,4250,4500,15,{Forward,Left_Turn,Right_Turn,Stop}},

  // slow forward
  {GREEN,2750,3000,10,{Forward,Left_Turn,Right_Turn,Stop}},

  // left turn
  {YELLOW,5750,7000,15,{Forward,Small_Right,Small_Left,Small_Right}},

  // right turn
  {PURPLE,7000,5250,15,{Forward,Small_Right,Small_Left,Small_Left}},

  // hard left
  {YELLOW,2500,7250,20,{Slow_Forward,Right_Turn,Left_Turn,Stop}},

  // hard right
  {PURPLE,2250,7500,20,{Slow_Forward,Right_Turn,Left_Turn,Stop}},

  // small left
  {YELLOW,2250,2500,10,{Slow_Forward,Small_Right,Small_Left,Right_Turn}},

  // small right
  {PURPLE,2750,2000,10,{Slow_Forward,Small_Right,Small_Left,Left_Turn}}
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
        lookup_table[index](sensor_state->leftwheel, sensor_state->rightwheel);
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
    PWM_Init34(14999);
    sensor_state = Stop;
    EnableInterrupts();

    while(1) {
      WaitForInterrupt();
    }
}
