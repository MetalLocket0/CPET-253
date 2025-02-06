/****************************************************************************************
         CPET253 Lab3 - PWM and Motor Drivers

 Jeanne Christman
 original version 6/1/2022

 This program uses a state machine to control the TI-RSLK robot to drive
 in a pattern of forward, right turn, backward, left turn, forward, right turn, ….. 

 To control the motors on the TI-RSLK robot, there are three outputs that need
 to be driven.
    :Pin    :Description            :Notes
    :=======:=======================:=========================
    : P5.5  : Right motor direction : 0=forwards, 1=backwards
    : P3.6  : Right motor sleep     : 0=sleep, 1=awake
    : P2.6  : Right motor PWM       : 0=stop, PWM signal = go
    : P5.4  : Left motor direction  : 0=forwards, 1=backwards
    : P3.7  : Left motor sleep      : 0=sleep, 1= awake
    : P2.7  : Left motor PWM        : 0=stop, PWM signal = go

 Functions in this code:
     -Clock_Init48MHz() - function provided by TI to set system clock
     -Clock_Delay1ms(time) - built in function that delays time ms
     -Port2_Init();
     -Port3_Init();
     -Port5_Init();
     -TimerA0_Init();
     -MotorForward(volatile uint16_t rightDuty, volatile uint16_t leftDuty ); 
     -MotorBackward(volatile uint16_t rightDuty, volatile uint16_t leftDuty ); 
     -MotorTurnRight(volatile uint16_t rightDuty, volatile uint16_t leftDuty ); 
     -MotorTurnLeft(volatile uint16_t rightDuty, volatile uint16_t leftDuty ); 

The state machine has 4 states; forward, right, left, backward
use FSM to make a pattern: Forward, right turn 90 degrees, backwards, left turn 90, forward...
*******************************************************************************************/

#include "msp.h"
#include <stdint.h>
#include <stdbool.h>
#include "..\inc\Clock.h"
#include "..\inc\CortexM.h"
#include "..\inc\Init_Ports.h"
#include "..\inc\Init_Timers.h"
#include "..\inc\motor.h"


void main(void)
{


       WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD;     // stop watchdog timer
       Clock_Init48MHz();  // makes bus clock 48 MHz
       //Call the appropriate functions from Init_Ports.c
       Port2_Init();
       Port3_Init();
       Port5_Init();
       //Call the appropriate functions from Init_Timers.c
       TimerA0_Init();
       //These are the four states of the state machine
       enum motor_states {drive_forward,turn_right,turn_left,drive_backward, } state,prevState;

       state = drive_forward;                   //start state
       prevState = !drive_forward;               //used to know when the state has changed
       uint16_t stateTimer;       //used to stay in a state
       bool isNewState;           //true when the state has switched




       while(1)

       {
            isNewState = (state != prevState);
           prevState = state;  //save state for next time

          switch (state) {
          //each case below should have entry housekeeping, state business and exit housekeeping
          //remember to reset the stateTimer each time you enter a new state
          //you must assign a new state when stateTimer reaches the correct value
          case drive_forward:
              if (isNewState){
              stateTimer = 0;
              Motor_Forward(7599, 7599);
              }
              stateTimer++;
              if (stateTimer >= 100){
                  state = turn_right;
              }
                  break;
          case turn_right:
              if (isNewState){
              stateTimer = 0;
              Motor_Right(7599, 7599);
              }
              stateTimer++;
              if (stateTimer >= 100){
                  state = turn_left;
              }
                  break;
          case turn_left:
              if (isNewState){
              stateTimer = 0;
              Motor_Left(7599, 7599);
              }
              stateTimer++;
              if (stateTimer >= 100){
                  state = drive_backward;
              }
                  break;
          case drive_backward:
              if (isNewState){
              stateTimer = 0;
              Motor_Backward(7599, 7599);
              }
              stateTimer++;
              if (stateTimer >= 100){
                  state = drive_forward;
              }
                  break;
          } //switch

          Clock_Delay1ms(10);  //10ms delay so that each increment of statetimer is 10ms
       } //while(1)
   } //main()
