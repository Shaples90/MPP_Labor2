// Laborversuch 2 Aufgabe 2
// Vorlesung Seite 96 - 99

#include <stdio.h>
#include <stdint.h>
#include "tm4c1294ncpdt.h"

void wait(unsigned long time)
{
   unsigned long i;
   for (i = 0; i < time; i++);             // wait-loop
}

void main(int argc, char const *argv[])
{
   unsigned short timeMicroSeconds, timeMilliSeconds, measureDistance;

   // configure PD(0) and PD(1)
   SYSCTL_RCGCGPIO_R |= (1 << 3);            // clock enable port D
   while(!(SYSCTL_PRGPIO_R & (1 << 3)));     // wait for port D clock
   GPIO_PORTD_DEN_R |= (1 << 0) | (1 << 1);  // PD(0) and PD(1) enable
   GPIO_PORTD_AFSEL_R |= (1 << 0);           // PD(0) alternate function
   GPIO_PORTD_PCTL_R |= 0x03;                // PD(0) connected to Timer0A
   GPIO_PORTD_DIR_R |= 0x02;                 // PD(1) define output
   GPIO_PORTD_DATA_R |= 0x02;                // PD(1) to High

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
      GPIO_PORTD_DATA_R &= 0x00;                                           // PD(1) to Low
      while((TIMER0_RIS_R & (1 << 2)) == 0);                               // wait for capture event
      TIMER0_ICR_R |= (1 << 2);                                            // clear Timer0a capture event flag
      TIMER0_CTL_R |= 0x01;                                                // re-enable Timer0A
      while((TIMER0_RIS_R & (1 << 2)) == 0);                               // wait for capture event
      timeMicroSeconds = ((unsigned short) (0xFFFF - TIMER0_TAR_R)) / 16;  // measured time in micro seconds
      timeMilliSeconds = timeMicroSeconds * 0.001;                         // measured time in milli seconds
      measureDistance = timeMilliSeconds * 34,4;                           // measured Distance in cm
      printf("measure distance is: %d\n", measureDistance);                // print measureDistance on console
      TIMER0_ICR_R |= (1 << 2);                                            // clear capture event flag
      GPIO_PORTD_DATA_R |= 0x02;                                           // PD(1) to High
   }
}
