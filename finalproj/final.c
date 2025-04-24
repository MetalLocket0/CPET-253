#include "msp.h"
#include <stdint.h>
#include "../inc/Clock.h"
#include "../inc/SSD1306.h"
#include "../inc/Init_Ports.h"
#include "../inc/Init_Timers.h"

#define TRIGGER 0x04  
#define ECHO    0x08  
#define MAX_DIST 100  
#define MAX_PIXELS 128 

int freeze = 0;
uint32_t pulseIn(void);


uint16_t getDistance(void) {
    P6OUT |= TRIGGER;
    Clock_Delay1us(10);
    P6OUT &= ~TRIGGER;

    uint32_t time = pulseIn();
    uint16_t dist = time * 0.034 / 2;

    if (dist > MAX_DIST) {
    return MAX_DIST;
    }
    else {
    return dist;
    }
}

uint32_t pulseIn(void) {
    uint32_t start, end;

    TA2CTL |= MC_2; 
    TA2R = 0;
    while ((P6IN & ECHO) == 0);  
    start = TA2R;
    while ((P6IN & ECHO));      
    end = TA2R;
    TA2CTL &= ~MC_3;             

    return (end - start) / 1.5; 
}

void beep() {
    TA0CTL |= MC_1;           // Start timer in up mode
    __delay_cycles(300000);   // Delay for tone duration (~300ms)
    TA0CTL &= ~MC_1;          // Stop timer
    P2->OUT &= ~BIT6;         // Turn off buzzer pin
}

void PORT1_IRQHandler(void) {
    if (P1IFG & BIT1) {
        freeze ^= 1;
        beep();
        P1IFG &= ~BIT1; 
    }
}


void main(void) {
	WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD;		// stop watchdog timer
	Clock_Init48MHz();  // makes bus clock 48 MHz
    Port1_Init();
    Port2_Init();
    Port6_Init();
    TimerA0_Init();
    TimerA2_Init();
    SSD1306_Init(SSD1306_SWITCHCAPVCC);

    uint16_t distance;

    while (1) {

            distance = getDistance();
           if(!freeze) {
            SSD1306_Clear();
            SSD1306_SetCursor(0, 0);
            SSD1306_OutString("Distance:");
            SSD1306_OutUDec(distance);
            SSD1306_OutString(" cm");

            if (distance >= 100){
                SSD1306_SetCursor(0, 3);
                SSD1306_OutString("TOO FAR!");
            }
           }

           else if (freeze) {
               SSD1306_SetCursor(0,7);
               SSD1306_OutString("Frozen");
               while (freeze);
           }


        Clock_Delay1ms(200);
    }
}
