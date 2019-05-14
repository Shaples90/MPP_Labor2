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
   unsigned long ulVal;
   double timeMicroSeconds, timeMilliSeconds;
   unsigned int measureDistance;

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
   TIMER0_TAILR_R = 0xFFFF;               // ILR = 65535
   TIMER0_CTL_R |= 0x0C;                  // both edges
   TIMER0_ICR_R |= 0x001F;                // clear all flags Timer0A
   
   // configure Timer 1 for 20ms
   SYSCTL_RCGCTIMER_R |= (1 << 1);        // clock enable Timer1
   while(!(SYSCTL_PRTIMER_R & (1 << 1))); // wait for Timer1 clock
   TIMER1_CTL_R &= ~0x01;                 // disable Timer1A for config
   TIMER1_CFG_R = 0x04;                   // 2 x 16-bit mode
   TIMER1_TAMR_R |= 0x01;                 // match disable, count-down, one-shot mode
   TIMER1_TAPR_R = 5 - 1;                 // prescaler PR = 5-1
   TIMER1_TAILR_R = 64000 - 1;            // ILR = 64000 - 1
   

   while (1)
   {
      // synchronize to next edge     
      TIMER0_CTL_R |= 0x01;                                          // enable Timer0A                                       
      while((TIMER0_RIS_R & (1 << 2)) == 0)                          // wait for capture event
      {
    	  GPIO_PORTD_AHB_DATA_R |= 0x1;              						// PD(0) to HIGH for measure trigger
    	  wait(5000);
    	  GPIO_PORTD_AHB_DATA_R &= ~0x1;                               // PD(0) to LOW for measure trigger
      }
      TIMER0_ICR_R |= (1 << 2);                                      // clear Timer0a capture event flag
      TIMER0_CTL_R |= 0x01;                                          // re-enable Timer0A
      while((TIMER0_RIS_R & (1 << 2)) == 0);                         // wait for capture event
      ulVal = TIMER0_TAR_R;                                          // save imter value at capture event
      timeMicroSeconds = ((unsigned short) (0xFFFF - ulVal) / 16);
      timeMilliSeconds = timeMicroSeconds * 0.001;
      measureDistance = timeMilliSeconds * 34.4;                         
      printf("measure distance is: %d\n", measureDistance);
      TIMER0_ICR_R |= (1 << 2);			                              // clear capture event flag
      TIMER1_CTL_R |= 0x01;                                          // enable Timer1A (20ms)
      while((TIMER1_RIS_R & (1 << 0)) == 0);                         // flag, wenn time-out
      TIMER0_ICR_R |= (1 << 0);                                      // clear time-out flag
   }
}