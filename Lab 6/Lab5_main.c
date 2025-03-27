/****************************************************************************************
         CPET253 Lab5 - Servos and Ultrasonic Sensing

 Jeanne Christman
 original version 1/19/2025

 This program uses an ultrasonic sensor to determine when there is an object in front of a
 forward moving robot. Once it is determined that the robot is approaching an object, it backs up,
 stops and then controls a servo motor to sweep the sensor 90 degrees right then 90 degrees left.
 A distance measurement is taken on each sweep. The robot then turns in the direction that is "more
 clear" and continues forward again.

 The servo motor is controlled by a PWM signal from TimerA3. The period of the PWM signal is 20ms
 and the pulse width ranges from 1 ms to 2 ms to control the sweep. The PWM signal is output on
 pin P9.2

 The ultrasonic sensor trigger is connected to pin P6.2 and the echo is connected to pin P6.3. The
 distance measurement is calculated using the width of the pulse returned from the sensor. TimerA2
 is used to determine the duration of the pulse from the sensor.

 Functions in this code:
     -Clock_Init48MHz() - function provided by TI to set system clock
     -Clock_Delay1ms(time) - built in function that delays time ms
     -Clock_Delay1us(time) - built in function that delays time us
     -Port2_Init();
     -Port3_Init();
     -Port5_Init();
     -Port6_Init();
     -Port9_Init();
     -TimerA0_Init();
     -TimerA2_Init();
     -TimerA3_Init();
     -Motor_Forward(volatile uint16_t rightDuty, volatile uint16_t leftDuty );
     -Motor_Backward(volatile uint16_t rightDuty, volatile uint16_t leftDuty );
     -Motor_Right(volatile uint16_t rightDuty, volatile uint16_t leftDuty );
     -Motor_Left(volatile uint16_t rightDuty, volatile uint16_t leftDuty );
     -Motor_Stop();

The state machine has 6 states; forward, turn right, turn left, backward, sweep right, sweep left
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

#define TRIGGER 0x04
#define ECHO 0x08
#define COLLISION_DISTANCE 20  // cm
#define BACKUP_TIME 25         // ~500ms (25*20ms)
#define TURN_TIME 100          // ~2 seconds (100*20ms)

#define microsecondsToClockCycles(a) ( (a) * 1.5 )       //assume 12Mhz clock divided by 8
#define clockCyclesToMicroseconds(a) ( (a) / 1.5 )       // 1.5 clock cycles = 1us

void Servo(uint16_t angle);
uint32_t pulseIn (void);


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
//desired pulse width in the PWM signal
{
    //set period for 20ms
    TA3CCR0 = 59999;
    //set high time for the input angle using angle_count
        TA3CCR3 = angle_count;
    //set timer for up mode
        TA3CTL = 0x0290;
    return;
}
uint16_t distanceInCm(void) {  //this function measures and returns the distance to the nearest object
    uint16_t distance;

    P6OUT |= TRIGGER; //drive trigger pin high
    Clock_Delay1us(10);//wait 10 us - can use built-in timer function
    P6OUT &= ~TRIGGER; //drive trigger pin low
    uint32_t pulseWidth = pulseIn(); //calculate distance using s=t * 0.034/2. t comes from pulseIn() function
    // if no echo (distance = 0), assume object is at farthest distance
    if (pulseWidth == 0) {
        return 400;
    //return the distance
}
uint32_t pulseIn (void)  //this function returns the width of the return pulse
//from the ultrasonic sensor in terms of microseconds
{
    uint16_t width = 0;   //will be in clock counts
    uint16_t time = 0;    //the result of converting clock counts to microseconds
    uint16_t maxcount = 56999;  //max count for 38 ms (timeout)

    TA2CTL |= MC_2; //set timer for continuous mode

    TA2R = 0;//reset the count register
    //wait for the pulse to start (while Echo is low)
    //if count is greater than maxcount return 0
    while (!(P6IN & ECHO)) {  
        if (TA2R > maxcount) return 0; 
    }

    TA2R = 0; //reset the count register
    //wait for the pulse to finish (while Echo is high)
    //if count is greater than maxcount return 0
    while (P6IN & ECHO) {  
        if (TA2R > maxcount) return 0;  
    }
    width = TA2R; //read the count (width of the return pulse)
    TA2CTL = 0;//stop the timer
    //convert the reading to microseconds.
    //return the microsecond reading
    return clockCyclesToMicroseconds(width);
}

void main(void)
{

  

	WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD;		// stop watchdog timer
	Clock_Init48MHz();  // makes bus clock 48 MHz
	//call all the port initialization functions
    Port2_Init();
    Port3_Init();
    Port5_Init();
    Port6_Init();
    Port9_Init();
	//call all the timer initialization functions
    TimerA0_Init();
    TimerA3_Init();
    TimerA2_Init();
	//center the servo using the ServoInit() function
    ServoInit();
	//These are the states of the state machine
	typedef enum {
        FORWARD,
        BACKWARD,
        TURN_RIGHT,
        TURN_LEFT
    } State;
    State state = FORWARD;
    State prevState = !FORWARD;
    uint16_t stateTimer = 0;
    bool isNewState;
    uint16_t distance = 0;
    uint8_t collisionSide = 0; // 0=front, 1=right, 2=left
    


	while(1) {

	    isNewState = (state != prevState);
        prevState = state;
        
        switch (state) {
            case FORWARD:
                if (isNewState) {
                    stateTimer = 0;
                    Motor_Forward(7599, 7599);
                }
                
                distance = distanceInCm();
                if (distance < COLLISION_DISTANCE) {
                    collisionSide = 0; // Front collision
                    state = BACKWARD;
                }
                break;
                
            case BACKWARD:
                if (isNewState) {
                    stateTimer = 0;
                    Motor_Backward(7599, 7599);
                }
                
                stateTimer++;
                if (stateTimer >= BACKUP_TIME) {
                    // After backing up, turn opposite collision side
                    state = (collisionSide == 1) ? TURN_LEFT : TURN_RIGHT;
                }
                break;
                
            case TURN_RIGHT:
                if (isNewState) {
                    stateTimer = 0;
                    Motor_Right(7599, 7599);
                }
                
                stateTimer++;
                if (stateTimer >= TURN_TIME) {
                    state = FORWARD;
                }
                break;
                
            case TURN_LEFT:
                if (isNewState) {
                    stateTimer = 0;
                    Motor_Left(7599, 7599);
                }
                
                stateTimer++;
                if (stateTimer >= TURN_TIME) {
                    state = FORWARD;
                }
                break;
        }
        
        Clock_Delay1ms(20);
    }
}
