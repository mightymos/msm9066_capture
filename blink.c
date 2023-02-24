/*
    blink for OB38S003
 */
//#include "project-defs.h"
#include <8052.h>
#include <stdbool.h>

//#include <delay.h>

// just do not have much more space for HAL with 1KB flash code size
//#include <eeprom-hal.h>
//#include <gpio-hal.h>
//#include <power-hal.h>
//#include <timer-hal.h>

//#ifndef MCU_HAS_WAKE_UP_TIMER
//    // Shouldn't happen, unless using an STC12.
//    #error "The selected MCU doesn't have a power-down wake-up timer."
//#endif // MCU_HAS_WAKE_UP_TIMER


#define LED_PIN P3_0

__sfr __at (0xdb) P3M1;
__sfr __at (0xda) P3M0;


// led is controlled by transistor which essentially inverts pin output
// (so low level enables transistor and then LED is on)
inline void led_on(void)
{
    LED_PIN = 0;
}

inline void led_off(void)
{
    LED_PIN = 1;
}


void delay_ms(unsigned int msCount)
{
    unsigned int i,j;
    for(i = 0; i < msCount; i++)
    {
        for(j = 0; j < 1000; j++);
    }
}



void main(void)
{
    // as per HAL instructions
    //INIT_EXTENDED_SFR();
    P3M1 &= ~0x01;
    P3M0 |= 0x01;
    
    while (true)
    {
        // pulse LED at startup because we delay next anyway
        led_on();
        
        // give the microcontroller time to stabilize
        delay_ms(500);
        
        led_off();
        
        // give the microcontroller time to stabilize
        delay_ms(500);
    }

}
