// BumpInt.c
// Runs on MSP432, interrupt version
// Provide low-level functions that interface bump switches on the robot.
// Daniel Valvano and Jonathan Valvano
// July 11, 2019

/* This example accompanies the book
   "Embedded Systems: Introduction to Robotics,
   Jonathan W. Valvano, ISBN: 9781074544300, copyright (c) 2019
 For more information about my classes, my research, and my books, see
 http://users.ece.utexas.edu/~valvano/

Simplified BSD License (FreeBSD License)
Copyright (c) 2019, Jonathan Valvano, All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

The views and conclusions contained in the software and documentation are
those of the authors and should not be interpreted as representing official
policies, either expressed or implied, of the FreeBSD Project.
*/

// Negative logic bump sensors
// P4.7 Bump5, left side of robot
// P4.6 Bump4
// P4.5 Bump3
// P4.3 Bump2
// P4.2 Bump1
// P4.0 Bump0, right side of robot

#include <stdint.h>
#include "msp.h"
// Initialize Bump sensors
// Make six Port 4 pins inputs
// Activate interface pullup
// pins 7,6,5,3,2,0
// Interrupt on falling edge (on touch)
volatile int32_t bump_count = 0; // Running count based on bumper presses
void BumpInt_Init(void){
    // write this as part of Lab 5
    P4DIR &= ~0xED;  // 0b11101101 (Only these bits are set to 0 for input)

    // Enable pull-up resistors
    P4REN |= 0xED;   // Enable resistor on the selected pins
    P4OUT |= 0xED;   // Set pull-up mode

    // Enable falling edge interrupt (trigger on press)
    P4IE  |= 0xED;   // Enable interrupts for bump sensor pins
    P4IES |= 0xED;   // Set to trigger on falling edge (active low)
    P4IFG &= ~0xED;  // Clear any pending interrupt flags

    // Enable Port 4 interrupts in NVIC
    NVIC->ISER[1] = 1 << ((PORT4_IRQn) & 31);  // Enable Port 4 interrupt in NVIC
}

// triggered on touch, falling edge
void PORT4_IRQHandler(void){
    // write this as part of Lab 5
    switch (P4IV) {  // Read P4IV register to determine which pin caused the interrupt
        case 0x02:  // P4.0 (Bump0)
            bump_count += 3;
            break;
        case 0x06:  // P4.2 (Bump1)
            bump_count += 2;
            break;
        case 0x08:  // P4.3 (Bump2)
            bump_count += 1;
            break;
        case 0x0A:  // P4.5 (Bump3)
            bump_count -= 1;
            break;
        case 0x0C:  // P4.6 (Bump4)
            bump_count -= 2;
            break;
        case 0x0E:  // P4.7 (Bump5)
            bump_count -= 3;
            break;
        default:
            break;
    }
}

