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

struct State {
    uint8_t output;
    uint16_t leftwheel;
    uint16_t rightwheel;
    uint8_t delay;
    const struct State *next_state[9];
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
//#define Stop &FSM[7]
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
State_t FSM[7] ={

<<<<<<< HEAD
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
=======
     // lost
    {WHITE,5500,5250,0,{Lost, Slow_Forward, Forward, Small_Left, Small_Right, Left_Turn, Right_Turn}},

    // forward
    {GREEN,5500,5250, 15,{Lost, Forward, Slow_Forward, Left_Turn, Right_Turn, Small_Left, Small_Right}},

    // slow forward
    {GREEN,5500,5250,10,{Lost, Forward, Forward, Left_Turn, Right_Turn, Small_Left, Small_Right}},

    // left turn
    {YELLOW,5750,7000,15,{Lost, Forward, Slow_Forward, Left_Turn, Small_Right, Small_Left, Slow_Forward}},

    // right turn
    {PURPLE,7000,5250,15,{Lost, Forward, Slow_Forward, Small_Left, Right_Turn, Slow_Forward, Small_Right}},

    // small left
    {YELLOW,3550,3850,10,{Lost, Forward, Slow_Forward, Left_Turn, Right_Turn, Small_Left, Small_Right}},

    // small right
    {BLUE,4150,3850,10,{Lost, Forward, Slow_Forward, Left_Turn, Right_Turn, Small_Left, Small_Right}},

    // stop
    //{RED,0,0,10,{Stop, Forward, Slow_Forward, Left_Turn, Right_Turn, Small_Left, Small_Right}}
>>>>>>> refs/heads/Beta-1.2.2
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
void (*lookup_table[8])(uint16_t, uint16_t) = {
    Motor_Forward, Motor_Forward, Motor_Forward, Motor_Left, Motor_Right, Motor_Left, Motor_Right
};

void SysTick_Handler(void) {
    if (MainCount == 6) {
        Reflectance_Start();
    }
    else if (MainCount == 7) {
        IRSensorInput = Reflectance_End();

        // sensor output
        index = decision(IRSensorInput);
        sensor_state = sensor_state->next_state[index];
        P2->OUT = sensor_state->output;
<<<<<<< HEAD
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
=======
        lookup_table[index](sensor_state->leftwheel, sensor_state->rightwheel);
>>>>>>> refs/heads/Beta-1.2.2
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
<<<<<<< HEAD
    PWM_Init34(14999,0,0);
    sensor_state = stop;
=======
    PWM_Init34(14999, 0, 0);
    sensor_state = Lost;
>>>>>>> refs/heads/Beta-1.2.2
    EnableInterrupts();

    while(1) {
      WaitForInterrupt();
    }
}
