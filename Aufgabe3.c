// Laborversuch 2 Aufgabe 3

#include <stdio.h>
#include <stdint.h>
#include "tm4c1294ncpdt.h"

void main(int argc, char const *argv[])
{
   unsigned long measureDistance;         // local measureDistance storage

   // configure PD(0) and PK(7:0)
   SYSCTL_RCGCGPIO_R |= (1 << 3) | (1 << 9);          // clock enable port D and K
   while(!(SYSCTL_PRGPIO_R & ((1 << 3)|(1 << 9))));   // wait for port P  and port K clock
   GPIO_PORTD_DEN_R |= (1 << 0);                      // PD(0) enable
   GPIO_PORTD_AFSEL_R |= (1 << 0);                    // PD(0) alternate function
   GPIO_PORTD_PCTL_R |= 0x03;                         // PD(0) connected to Timer0A
   GPIO_PORTK_DEN_R |= 0xFF;                          // PK(7:0) enable
   GPIO_PORTK_DIR_R |= 0xFF;                          // define output direction for PK(7:0)

   // configure Timer 0
   SYSCTL_RCGCTIMER_R |= (1 << 0);        // clock enable Timer0
   while(!(SYSCTL_PRTIMER_R & (1 << 0))); // wait for Timer0 clock
   TIMER0_CTL_R &= ~0x01;                 // disable Timer0 for config
   TIMER0_CFG_R = 0x04;                   // 2 x 16-bit mode
   TIMER0_TAMR_R |= (1 << 2) | 0x03;      // edge time mode, capture mode
   TIMER0_TAILR_R = 0xFFFF;               // ILR = 65535
   TIMER0_CTL_R |= 0x000C;                // both edges
   TIMER0_ICR_R |= 0x001F;                // clear all flags Timer0A
   TIMER0_CTL_R |= 0x01;                  // enable Timer0A

   while (1)
   {
      // synchronize to next edge
      while((TIMER0_RIS_R & (1 << 2)) == 0);                   // wait for capture event
      TIMER0_ICR_R |= (1 << 2);                                // clear Timer0a capture event flag
      TIMER0_CTL_R |= 0x01;                                    // re-enable Timer0A
      while((TIMER0_RIS_R & (1 << 2)) == 0);                   // wait for capture event
      measureDistance = ((((unsigned short) (0xFFFF - TIMER0_TAR_R)) / 16) * 0.001) * 34.3;         // calculate measureDistance in cm
      printf("measure distance is: %d\n", measureDistance);    // print measureDistance on console
      TIMER0_ICR_R |= (1 << 2);                                // clear capture event flag
   }
}
