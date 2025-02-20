/****************************************************************************************
         CPET253 Lab4 - Servo 

 Jeanne Christman
 original version 1/19/2025

 This program uses a software state machine to control the Texas Instrument's RSLK robot to drive forward
 for 1 second, back up for 1/4 second, stops and then control a servo motor to sweep 90 degrees right then 
 90 degrees left. After sweeping in both directions, the robot turns right 90 degrees and returns to 
 forward motion.

 The servo motor is controlled by a PWM signal from TimerA3. The period of the PWM signal is 20ms
 and the pulse width ranges from 1 ms to 2 ms to control the sweep. The PWM signal is output on
 pin P9.2

 Functions in this code:
     -Clock_Init48MHz() - function provided by TI to set system clock
     -Clock_Delay1ms(time) - built in function that delays time ms
     -Port2_Init();
     -Port3_Init();
     -Port5_Init();
     -Port9_Init();
     -TimerA0_Init();
     -TimerA3_Init();
     -Motor_Forward(volatile uint16_t rightDuty, volatile uint16_t leftDuty );
     -Motor_Backward(volatile uint16_t rightDuty, volatile uint16_t leftDuty );
     -Motor_Right(volatile uint16_t rightDuty, volatile uint16_t leftDuty );
     -Motor_Left(volatile uint16_t rightDuty, volatile uint16_t leftDuty );
     -Motor_Stop();

The state machine has 5 states; forward, tun right, backward, sweep right, sweep left
*******************************************************************************************/

#include "msp.h"
#include <msp432.h>
#include <stdint.h>
#include <stdbool.h>
#include "../inc/Clock.h"
#include "../inc/CortexM.h"
#include "../inc/motor.h"
#include "../inc/Init_Ports.h"
#include "../inc/Init_Timers.h"

void Servo(uint16_t angle_count);

void ServoInit(void)  //This function initializes the servo to be centered (0 degrees)
{
     //call Servo() function to center servo
     Servo();
     //delay here to give servo time to move - can use built in timer function
     Clock_Delay1ms(20);
     //stop the timer
     return;
}
void Servo(uint16_t angle_count) // this function moves the servo.
//input: angle_count should be in terms of clock counts to create the 
//desired pulse width in the PWM signal (1-2 ms)
{
    //set period for 20ms
        TA3CCR0 = 59999;
    //set high time for the input angle using angle_count
        TA3CCR3 = angle_count;
    //set timer for up mode
        TA3CTL = 0x0290;
    return;
}

void main(void)
{

	WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD;		// stop watchdog timer
	Clock_Init48MHz();  // makes bus clock 48 MHz
	//call all the port initialization functions
    Port2_Init();
    Port3_Init();
    Port5_Init();
    Port9_Init();
	//call all the timer initialization functions
    TimerA0_Init();
    TimerA3_Init();
	//center the servo using the ServoInit() function
	//These are the states of the state machine
	enum motor_states {forward,backward,turn_right,sweep_right,sweep_left} state, prevState;

	state = FORWARD;          //start in FORWARD state
	prevState = !FORWARD;   //used to know when the state has changed
	uint16_t stateTimer = 0;           //used to stay in a state
	bool isNewState;              //true when the state has switched


	while(1) {

	    isNewState = (state != prevState);
            prevState = state;
	    
	    switch (state) {

        case forward:
        if (isNewState){
            stateTimer = 0;
            Motor_Forward(7599,7599);
        }
	    stateTimer++;
        if (stateTimer >= 100)
        {
            state = backward;
        }
        break;
       
        case backward:
        if (isNewState){
            stateTimer = 0;
            Motor_Backward(7599,7599);
        }
        stateTimer++;
        if (stateTimer >= 25)
        {
            state = sweep_right;
        }
        break;

        case sweep_right:
        if (isNewState){
            stateTimer = 0;
            Motor_Stop(0,0);
            Servo(5999);
        }
        stateTimer++;
        if (stateTimer >= 2)
        {
            state = sweep_left;
        }
        break;

	     case sweep_left:
        if (isNewState){
            stateTimer = 0;
            Motor_Stop(0,0);
            Servo(12000);
        }
        stateTimer++;
        if (stateTimer >= 3)
        {
            state = turn_right;
        }
        break;

        case turn_right:
        if (isNewState){
            stateTimer = 0;
            Motor_Right(7599,7599);
        }
        stateTimer++;
        if (stateTimer >= 200)
        {
            state = turn_right;
        }
        break;
	    } //switch 
        Clock_Delay1ms(10);
	}  //while
}
