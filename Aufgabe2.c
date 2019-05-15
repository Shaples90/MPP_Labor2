// Laborversuch 2 Aufgabe 2
// Vorlesung Seite 96 - 99

#include "tm4c1294ncpdt.h"
#include "stdio.h"
#include "stdint.h"

void wait(unsigned long time)
{
   unsigned long i;
   for (i = 0; i < time; i++);             // wait-loop
}

void main(int argc, char const *argv[])
{
   unsigned long ulVal1, ulVal2;
   double timeMicroSeconds, timeMilliSeconds;
   unsigned long measureDistance;

   // configure PD(0) and PL(4)
   SYSCTL_RCGCGPIO_R |= ((1 << 3) | (1 << 10));          // clock enable port D and L
   while(!(SYSCTL_PRGPIO_R & ((1 << 3) | (1 << 10))));   // wait for port D clock

   GPIO_PORTD_AHB_DEN_R |= (1 << 0);                     // PD(0) enable
   GPIO_PORTD_AHB_DIR_R |= (1 << 0);                     // PD(0) define output
   GPIO_PORTL_DEN_R |= (1 << 4);                         // PL(4) enable
   GPIO_PORTL_AFSEL_R |= (1 << 4);                       // PL(4) alternate function
   GPIO_PORTL_PCTL_R |= 0x00030000;                      // PL(4) connected to Timer0A

   // configure Timer 0
   SYSCTL_RCGCTIMER_R |= (1 << 0);        // clock enable Timer0
   while(!(SYSCTL_PRTIMER_R & (1 << 0))); // wait for Timer0 clock
   TIMER0_CTL_R &= ~0x01;                 // disable Timer0 for config
   TIMER0_CFG_R = 0x04;                   // 2 x 16-bit mode
   TIMER0_TAMR_R |= (1 << 2) | 0x03;      // edge time mode, capture mode
   TIMER0_TAILR_R = 0xFFFF;             	// ILR
   TIMER0_CTL_R |= 0x0C;                  // both edges
   TIMER0_ICR_R |= 0x001F;                // clear all flags Timer0A

   while (1)
   {
      TIMER0_CTL_R |= 0x01;                                          // enable Timer0A
      while((TIMER0_RIS_R & (1 << 2)) == 0)                          // wait for capture event
      {
    	  GPIO_PORTD_AHB_DATA_R |= 0x1;              						// PD(0) to HIGH for measure trigger
    	  GPIO_PORTD_AHB_DATA_R &= ~0x1;                               // PD(0) to LOW for measure trigger
      }
      ulVal1 = TIMER0_TAR_R;                                         // save first timer-value at capture event
      TIMER0_ICR_R |= (1 << 2);                                      // clear Timer0a capture event flag
      TIMER0_CTL_R |= 0x01;                                          // re-enable Timer0A
      while((TIMER0_RIS_R & (1 << 2)) == 0);                         // wait for capture event
      ulVal2 = TIMER0_TAR_R;                                         // save second-timer value at capture event
      TIMER0_ICR_R |= (1 << 2);			                             // clear capture event flag
      TIMER0_CTL_R &= ~0x01;                                         // disable Timer0A

      if(ulVal1 > ulVal2)
      {
		  timeMicroSeconds = ((unsigned short) ((0xFFFF - ulVal2) - (0xFFFF - ulVal1)) / 32);
		  timeMilliSeconds = timeMicroSeconds * 0.001;
		  measureDistance = timeMilliSeconds * 34.4;
		  printf("measure distance is: %d\n", measureDistance);
      }
   }
}
