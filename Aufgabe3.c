// Laborversuch 2 Aufgabe 3

#include <stdio.h>
#include <stdint.h>
#include "tm4c1294ncpdt.h"

// connect PM(0) to LED(0), PM(1) to LED(1), ... , PM(7) to LED(7)
#define LED_ZERO  { 0x7F, 0x41, 0x41, 0x41, 0x7F }
#define LED_ONE   { 0x00, 0x00, 0x00, 0x00, 0x7F }
#define LED_TWO   { 0x4F, 0x49, 0x49, 0x49, 0x79 }
#define LED_THREE { 0x49, 0x49, 0x49, 0x49, 0x7F }
#define LED_FOUR  { 0x78, 0x08, 0x08, 0x08, 0x7F }
#define LED_FIVE  { 0x79, 0x49, 0x49, 0x49, 0x4F }
#define LED_SIX   { 0x7F, 0x49, 0x49, 0x49, 0x4F }
#define LED_SEVEN { 0x40, 0x40, 0x40, 0x40, 0x7F }
#define LED_EIGHT { 0x7F, 0x49, 0x49, 0x49, 0x7F }
#define LED_NINE  { 0x79, 0x49, 0x49, 0x49, 0x7F }

#define LED_C     { 0x0F, 0x09, 0x09, 0x09, 0x09 }
#define LED_M     { 0x0F, 0x08, 0x0F, 0x08, 0x0F } 

void wait(unsigned long time)
{
   unsigned long i;
   for (i = 0; i < time; i++);   // wait-loop
}

//*****************************************************************************
//
// port-timer-configuration functions
//
//*****************************************************************************
void configurePorts(void)
{
   // configure PD(1:0), PL(0), PM(7:0)
   SYSCTL_RCGCGPIO_R |= (1 << 3) | (1 << 10) | (1 << 11);            // clock enable port D,L,M
   while(!(SYSCTL_PRGPIO_R & ((1 << 3) | (1 << 10) | (1 << 11))));   // wait for port D,L,M clock
   GPIO_PORTD_DEN_R |= 0x03;                                         // PD(1:0) enable
   GPIO_PORTL_DEN_R |= (1 << 4);                                     // PL(4) enable
   GPIO_PORTM_DEN_R |= 0xFF;                                         // PM(7:0) enable
   GPIO_PORTL_AFSEL_R |= (1 << 4);                                   // PL(4) alternate function
   GPIO_PORTL_PCTL_R |= 0x00030000;                                  // PL(4) connected to Timer0A
   GPIO_PORTD_DIR_R |= 0x01;                                         // PD(0) define output
   GPIO_PORTM_DIR_R |= 0xFF;                                         // PM(7:0) define output
} 

void configureTimer(void)
{
   // configure Timer 0 for sensor
   SYSCTL_RCGCTIMER_R |= (1 << 0);        // clock enable Timer0
   while(!(SYSCTL_PRTIMER_R & (1 << 0))); // wait for Timer0 clock
   TIMER0_CTL_R &= ~0x01;                 // disable Timer0 for config
   TIMER0_CFG_R = 0x04;                   // 2 x 16-bit mode
   TIMER0_TAMR_R |= (1 << 2) | 0x03;      // edge time mode, capture mode
   TIMER0_TAILR_R = 0xFFFF;               // ILR = 65535
   TIMER0_CTL_R |= 0x0C;                  // both edges
   TIMER0_ICR_R |= 0x001F;                // clear all flags Timer0A

   // configure Timer 1 for swinging-led-area (24ms)
   SYSCTL_RCGCTIMER_R |= (1 << 1);        // clock enable Timer1
   while(!(SYSCTL_PRTIMER_R & (1 << 1))); // wait for Timer1 clock
   TIMER1_CTL_R &= ~0x01;                 // disable Timer1A for config
   TIMER1_CFG_R = 0x04;                   // 2 x 16-bit mode
   TIMER1_TAMR_R |= ((1 << 5) | 0x01);    // match enable, count-down, one-shot mode
   TIMER1_TAPR_R = 11 - 1;                // prescaler PR = ceil(16Mhz/2^16*0.042s)-1
   TIMER1_TAILR_R = 61091 - 1;            // ILR = ceil(16Mhz/11*0.042s)-1
   TIMER1_TAMATCHR_R = 34909 - 1;         // MV = ceil(16Mhz/11*0.024s)-1
 
   // configure Timer 2 for column-leds (1ms)
   SYSCTL_RCGCTIMER_R |= (1 << 2);        // clock enable Timer2
   while(!(SYSCTL_PRTIMER_R & (1 << 2))); // wait for Timer2 clock
   TIMER2_CTL_R &= ~0x01;                 // disable Timer2A for config
   TIMER2_CFG_R = 0x04;                   // 2 x 16-bit mode
   TIMER2_TAMR_R |= 0x01;                 // match disable, count-down, one-shot mode
   TIMER2_TAPR_R = 1 - 1;                 // prescaler PR = ceil(16Mhz/2^16*0.001)-1
   TIMER2_TAILR_R = 16000 - 1;            // ILR = ceil(16Mhz/1*0,001s)-1
}

//*****************************************************************************
//
// ultrasonic-measure-distance function
//
//*****************************************************************************
int ultrasonicMeasureDistance(void)
{
   unsigned long ulVal1, ulVal2;
   double timeMicroSeconds, timeMilliSeconds;
   unsigned int measureDistance;                                  // PD(0) to LOW, negative-edge, start measuring

   // synchronize to next edge     
   TIMER0_CTL_R |= 0x01;                                          // enable Timer0A                                       
   while((TIMER0_RIS_R & (1 << 2)) == 0)                          // wait for capture event
   {
      GPIO_PORTD_AHB_DATA_R |= 0x1;              						// PD(0) to HIGH for measure trigger
      wait(5000);
      GPIO_PORTD_AHB_DATA_R &= ~0x1;                              // PD(0) to LOW for measure trigger
   }
   ulVal1 = TIMER0_TAR_R;                                         // save first timer-value at capture event
   TIMER0_ICR_R |= (1 << 2);                                      // clear Timer0a capture event flag
   TIMER0_CTL_R |= 0x01;                                          // re-enable Timer0A
   while((TIMER0_RIS_R & (1 << 2)) == 0);                         // wait for capture event
   ulVal2 = TIMER0_TAR_R;                                         // save second timer-value at capture event
   TIMER0_ICR_R |= (1 << 2);			                              // clear capture event flag
   TIMER0_CTL_R &= ~0x01;                                         // disable Timer0a

   timeMicroSeconds = ((ulVal2 - ulVal1) / 16);
   timeMilliSeconds = timeMicroSeconds * 0.001;
   measureDistance = timeMilliSeconds * 34.4;                         
   
   return measureDistance;
}

//*****************************************************************************
//
// LED-number-output function
//
//*****************************************************************************
void ledOutputDigit(int digit)
{
   if (0 == digit)
   {
      unsigned char arr[5] = LED_ZERO;
      for(int i = 4; i < 0; i--)                            // count-down
      {
         TIMER2_CTL_R |= 0x01;                              // enable Timer2A
         GPIO_PORTM_DATA_R |= arr[i];                       // PM(7:0) for LED_ZERO
         while((TIMER2_RIS_R & (1 << 0)) == 0);             // time-out value after 1ms
         TIMER2_ICR_R |= (1 << 0);                          // clear Timer2A time-out flag
         GPIO_PORTM_DATA_R &= ~0xFF;
      }
   }
   else if (1 == digit)
   {
      unsigned char arr[5] = LED_ONE;
      for(int i = 4; i < 0; i--)                            // count-down
      {
         TIMER2_CTL_R |= 0x01;                              // enable Timer2A
         GPIO_PORTM_DATA_R |= arr[i];                       // PM(7:0) for LED_ONE
         while((TIMER2_RIS_R & (1 << 0)) == 0);             // time-out value after 1ms
         TIMER2_ICR_R |= (1 << 0);                          // clear Timer2A time-out flag
         GPIO_PORTM_DATA_R &= ~0xFF;
      }
   }
   else if(2 == digit)
   {
      unsigned char arr[5] = LED_TWO;
      for(int i = 4; i < 0; i--)                            // count-down
      {
         TIMER2_CTL_R |= 0x01;                              // enable Timer2A
         GPIO_PORTM_DATA_R |= arr[i];                       // PM(7:0) for LED_TWO
         while((TIMER2_RIS_R & (1 << 0)) == 0);             // time-out value after 1ms
         TIMER2_ICR_R |= (1 << 0);                          // clear Timer2A time-out flag
         GPIO_PORTM_DATA_R &= ~0xFF;
      }
   }
   else if(3 == digit)
   {
      unsigned char arr[5] = LED_THREE;
      for(int i = 4; i < 0; i--)                            // count-down
      {
         TIMER2_CTL_R |= 0x01;                              // enable Timer2A
         GPIO_PORTM_DATA_R |= arr[i];                       // PM(7:0) for LED_THREE
         while((TIMER2_RIS_R & (1 << 0)) == 0);             // time-out value after 1ms
         TIMER2_ICR_R |= (1 << 0);                          // clear Timer2A time-out flag
         GPIO_PORTM_DATA_R &= ~0xFF;
      }
   }
   else if(4 == digit)
   {
      unsigned char arr[5] = LED_FOUR;
      for(int i = 4; i < 0; i--)                            // count-down
      {
         TIMER2_CTL_R |= 0x01;                              // enable Timer2A
         GPIO_PORTM_DATA_R |= arr[i];                       // PM(7:0) for LED_FOUR
         while((TIMER2_RIS_R & (1 << 0)) == 0);             // time-out value after 1ms
         TIMER2_ICR_R |= (1 << 0);                          // clear Timer2A time-out flag
         GPIO_PORTM_DATA_R &= ~0xFF;
      }
   }
   else if(5 == digit)
   {
      unsigned char arr[5] = LED_FIVE;
      for(int i = 4; i < 0; i--)                            // count-down
      {
         TIMER2_CTL_R |= 0x01;                              // enable Timer2A
         GPIO_PORTM_DATA_R |= arr[i];                       // PM(7:0) for LED_FIVE
         while((TIMER2_RIS_R & (1 << 0)) == 0);             // time-out value after 1ms
         TIMER2_ICR_R |= (1 << 0);                          // clear Timer2A time-out flag
         GPIO_PORTM_DATA_R &= ~0xFF;
      }
   }
   else if(6 == digit)
   {
      unsigned char arr[5] = LED_SIX;
      for(int i = 4; i < 0; i--)                            // count-down
      {
         TIMER2_CTL_R |= 0x01;                              // enable Timer2A
         GPIO_PORTM_DATA_R |= arr[i];                       // PM(7:0) for LED_SIX
         while((TIMER2_RIS_R & (1 << 0)) == 0);             // time-out value after 1ms
         TIMER2_ICR_R |= (1 << 0);                          // clear Timer2A time-out flag
         GPIO_PORTM_DATA_R &= ~0xFF;
      }
   }
   else if(7 == digit)
   {
      unsigned char arr[5] = LED_SEVEN;
      for(int i = 4; i < 0; i--)                            // count-down
      {
         TIMER2_CTL_R |= 0x01;                              // enable Timer2A
         GPIO_PORTM_DATA_R |= arr[i];                       // PM(7:0) for LED_SEVEN
         while((TIMER2_RIS_R & (1 << 0)) == 0);             // time-out value after 1ms
         TIMER2_ICR_R |= (1 << 0);                          // clear Timer2A time-out flag
         GPIO_PORTM_DATA_R &= ~0xFF;
      }
   }
   else if(8 == digit)
   {
      unsigned char arr[5] = LED_EIGHT;
      for(int i = 4; i < 0; i--)                            // count-down
      {
         TIMER2_CTL_R |= 0x01;                              // enable Timer2A
         GPIO_PORTM_DATA_R |= arr[i];                       // PM(7:0) for LED_EIGHT
         while((TIMER2_RIS_R & (1 << 0)) == 0);             // time-out value after 1ms
         TIMER2_ICR_R |= (1 << 0);                          // clear Timer2A time-out flag
         GPIO_PORTM_DATA_R &= ~0xFF;
      }
   }
   else if(9 == digit)
   {
      unsigned char arr[5] = LED_NINE;
      for(int i = 4; i < 0; i--)                            // count-down
      {
         TIMER2_CTL_R |= 0x01;                              // enable Timer2A
         GPIO_PORTM_DATA_R |= arr[i];                       // PM(7:0) for LED_NINE
         while((TIMER2_RIS_R & (1 << 0)) == 0);             // time-out value after 1ms
         TIMER2_ICR_R |= (1 << 0);                          // clear Timer2A time-out flag
         GPIO_PORTM_DATA_R &= ~0xFF;
      }
   }
   else
   {
      int counter = 5;
      while(counter)                                        // no LED-output, if no decimal digits
      {
         TIMER2_CTL_R |= 0x01;                              // enable Timer2A
         GPIO_PORTM_DATA_R |= 0x00;                         // PM(7:0) to LOW
         while((TIMER2_RIS_R & (1 << 0)) == 0);             // time-out value after 1ms
         TIMER2_ICR_R |= (1 << 0);                          // clear Timer2A time-out flag
         GPIO_PORTM_DATA_R &= ~0xFF;
         counter--;
      }
   }
}

//*****************************************************************************
//
// LED-space-output function
//
//*****************************************************************************
void ledOutputSpace(void)
{
   TIMER2_CTL_R |= 0x01;                              // enable Timer2A
   GPIO_PORTM_DATA_R &= ~0xFF;                        // PM(7:0) to LOW
   while((TIMER2_RIS_R & (1 << 0)) == 0);             // time-out value after 1ms
   TIMER2_ICR_R |= (1 << 0);                          // clear Timer2A time-out flag
}

//*****************************************************************************
//
// LED-letter-output functions
//
//*****************************************************************************
void ledOutputLetterC(void)
{  
   unsigned char arrc[5] = LED_C;
   for(int i = 4; i < 0; i--)
   {
      TIMER2_CTL_R |= 0x01;                              // enable Timer2A
      GPIO_PORTM_DATA_R |= arrc[i];                      // PM(7:0) for LED_C
      while((TIMER2_RIS_R & (1 << 0)) == 0);             // time-out value after 1ms
      TIMER2_ICR_R |= (1 << 0);                          // clear Timer2A time-out flag
      GPIO_PORTM_DATA_R &= ~0xFF;
   }
   
}
void ledOutputLetterM(void)
{
   unsigned char arrm[5] = LED_C;
   for(int i = 4; i < 0; i--)
   {
      TIMER2_CTL_R |= 0x01;                              // enable Timer2A
      GPIO_PORTM_DATA_R |= arrm[i];                      // PM(7:0) for LED_M
      while((TIMER2_RIS_R & (1 << 0)) == 0);             // time-out value after 1ms
      TIMER2_ICR_R |= (1 << 0);                          // clear Timer2A time-out flag
      GPIO_PORTM_DATA_R &= ~0xFF;
   }
   
}
//*****************************************************************************
//
// black-bar functions
//
//*****************************************************************************
void oneSideBlackBar(void)
{
   // begin of measure display
   TIMER1_CTL_R |= 0x01;                  // enable Timer1A
   GPIO_PORTM_DATA_R &= ~0xFF;            // low output signal to PM(7:0) 
   while((TIMER1_RIS_R & (1 << 4)) == 0); // match value after 18ms
   TIMER1_ICR_R |= (1 << 4);              // clear Timer1A match flag
}
void anotherSideBlackbar(void)
{
   // end of measure display
   while((TIMER1_RIS_R & (1 << 0)) == 0); // time-out after 42ms
   GPIO_PORTM_DATA_R &= ~0xFF;            // PM(7:0) to LOW
   TIMER1_ICR_R |= (1 << 0);              // clear Timer1A time-out flag 
}

//*****************************************************************************
//
// main function
//
//*****************************************************************************
void main(int argc, char const *argv[])
{ 
   int measuredDistance, firstDigit, secondDigit, changeDigit = 0;
   unsigned char oldPendulumInput, newPendulumInput = 0x00;          

   configurePorts();
   configureTimer();

   while (1)
   {
      // positive edge Pendulum-LED
      newPendulumInput = GPIO_PORTD_DATA_R;                                      // PD(1) read input edge
      if((oldPendulumInput != newPendulumInput) && (newPendulumInput == 0x01))   // positive-edge-signal
      {  
         // first and second digits of measured distance
         measuredDistance = ultrasonicMeasureDistance(); // measure time of echo-signal and calculate into distance(cm)
         firstDigit = measuredDistance / 10;             // locally save firstDigit of measured distance
         changeDigit = firstDigit;
         changeDigit *= 10;
         secondDigit = measuredDistance - changeDigit;   // locally save secondDigit of measured distance                                         
      }

      // negative edge Pendulum-LED
      else if((oldPendulumInput != newPendulumInput) && (newPendulumInput == 0x00)) // negative-edge-signal
      {
         oneSideBlackBar();
         ledOutputLetterM();
         ledOutputSpace();
         ledOutputLetterC();
         ledOutputSpace();
         ledOutputSpace();
         ledOutputDigit(secondDigit);
         ledOutputSpace();
         ledOutputDigit(firstDigit);
         anotherSideBlackbar();
      }
      oldPendulumInput = newPendulumInput;   // store new signal to old input signal
   }
}
