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

struct State {
    uint8_t output;
    uint16_t leftwheel;
    uint16_t rightwheel;
    uint8_t delay;
    const struct State *next_state[7];
};
typedef const struct State State_t; // allows us to create state structures with the State_t initializer

// all possible next states

#define Lost &FSM[0]
#define Forward &FSM[1]
#define Slow_Forward &FSM[2]
#define Left_Turn &FSM[3]
#define Right_Turn &FSM[4]
#define Small_Left &FSM[5]
#define Small_Right &FSM[6]

#define RED 0x01
#define GREEN 0x02
#define YELLOW 0x03
#define PURPLE 0x05
#define BLUE 0x04
#define WHITE 0x07
#define SKYBLUE 0x06

//50% duty cycle is 7500
State_t FSM[7] ={
     // lost
    {WHITE,5400,5375,2.5,{Lost, Slow_Forward, Forward, Small_Left, Small_Right, Left_Turn, Right_Turn}},

    // forward
    {GREEN,5400,5375,15,{Lost, Forward, Slow_Forward, Left_Turn, Right_Turn, Small_Left, Small_Right}},

    // slow forward
    {GREEN,5400,5375,10,{Lost, Forward, Forward, Left_Turn, Right_Turn, Small_Left, Small_Right}},

    // left turn
    {YELLOW,5750,7000,15,{Lost, Forward, Slow_Forward, Left_Turn, Small_Right, Small_Left, Slow_Forward}},

    // right turn
    {PURPLE,7000,5250,15,{Lost, Forward, Slow_Forward, Small_Left, Right_Turn, Slow_Forward, Small_Right}},

    // small left
    {YELLOW,3550,3850,10,{Lost, Forward, Slow_Forward, Left_Turn, Right_Turn, Small_Left, Small_Right}},

    // small right
    {BLUE,4150,3850,10,{Lost, Forward, Slow_Forward, Left_Turn, Right_Turn, Small_Left, Small_Right}},
};


// global variables
uint8_t IRSensorInput, MainCount, index;
State_t *sensor_state;  // pointer to ir_sensor fsm

void Port2_Init(void) {
 P2->SEL0 &= ~0x07;
 P2->SEL1 &= ~0x07;
 P2->DIR |= 0x07;
 //P2->DS &= ~0x07;
 P2->OUT &= ~0x07;
}

// stores each motor action for a given state
void (*lookup_table[7])(uint16_t, uint16_t) = {
    Motor_Forward, Motor_Forward, Motor_Forward, Motor_Left, Motor_Right, Motor_Left, Motor_Right
};

void SysTick_Handler(void) {
    if (MainCount == 4) {
        Reflectance_Start();
    }
    else if (MainCount == 5) {
        IRSensorInput = Reflectance_End();
        index = decision(IRSensorInput);
        sensor_state = sensor_state->next_state[index];
        lookup_table[index](sensor_state->leftwheel, sensor_state->rightwheel);
        P2->OUT &= ~0x07;
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
    Port2_Init();
    Reflectance_Init();
    Motor_Init();
    //BumpInt_Init();
    PWM_Init34(14999, 0, 0);
    sensor_state = Lost;
    EnableInterrupts();

    while(1) {
      WaitForInterrupt();
    }
}
