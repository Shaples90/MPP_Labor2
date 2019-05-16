// Laborversuch 2 Aufgabe 3

#include <stdio.h>
#include <stdint.h>
#include "tm4c1294ncpdt.h"
#include <time.h>

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
   unsigned long j;
   for (j = 0; j < time; j++);   // wait-loop
}

//*****************************************************************************
//
// port-timer-configuration functions
//
//*****************************************************************************
void configurePorts(void)
{
   // configure PD(1:0), PL(0), PM(7:0)
   SYSCTL_RCGCGPIO_R |= ((1 << 3) | (1 << 10) | (1 << 11));            // clock enable port D,K,L,M
   while(!(SYSCTL_PRGPIO_R & ((1 << 3) | (1 << 10) | (1 << 11))));   // wait for port D,L,M clock
   GPIO_PORTD_AHB_DEN_R |= 0x01;                                     // PD(0) enable
   GPIO_PORTL_DEN_R |= ((1 << 4) | (1 << 0));                                     // PL(4) enable PL(0)
   GPIO_PORTM_DEN_R |= 0xFF;                                         // PM(7:0) enable
   GPIO_PORTL_AFSEL_R |= (1 << 4);                                   // PL(4) alternate function
   GPIO_PORTL_PCTL_R |= 0x00030000;                                  // PL(4) connected to Timer0A
   GPIO_PORTD_AHB_DIR_R |= 0x01;                                     // PD(0) define input
   GPIO_PORTM_DIR_R |= 0xFF;                                         // PM(7:0) define output
   GPIO_PORTL_DIR_R |= 0x00;										 // PL(0) define input
}

void configureTimer(void)
{
   // configure Timer 0 (capture echo signal)
   SYSCTL_RCGCTIMER_R |= (1 << 0);        // clock enable Timer0
   while(!(SYSCTL_PRTIMER_R & (1 << 0))); // wait for Timer0 clock
   TIMER0_CTL_R &= ~0x01;                 // disable Timer0 for config
   TIMER0_CFG_R = 0x04;                   // 2 x 16-bit mode
   TIMER0_TAMR_R |= (1 << 2) | 0x03;      // edge time mode, capture mode
   TIMER0_TAILR_R = 0xFFFF;               // ILR = 65535
   TIMER0_CTL_R |= 0x0C;                  // both edges
   TIMER0_ICR_R |= 0x001F;                // clear all flags Timer0A

   // configure Timer 1 for swinging-led-area (18ms)
   SYSCTL_RCGCTIMER_R |= (1 << 1);        // clock enable Timer1
   while(!(SYSCTL_PRTIMER_R & (1 << 1))); // wait for Timer1 clock
   TIMER1_CTL_R &= ~0x01;                 // disable Timer1A for config
   TIMER1_CFG_R = 0x04;                   // 2 x 16-bit mode
   TIMER1_TAMR_R |=  0x01;                // match disable, count-down, one-shot mode
   TIMER1_TAPR_R = 5 - 1;                 // prescaler
   TIMER1_TAILR_R = 57600 - 1;            // ILR = ceil(16Mhz/11*0.042s)-1
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
	unsigned long measureDistance;

   // synchronize to next edge
	TIMER0_CTL_R |= 0x01;                    // enable Timer0A
   while((TIMER0_RIS_R & (1 << 2)) == 0)    // wait for capture event
   {
      GPIO_PORTD_AHB_DATA_R |= 0x1;          // PD(0) to HIGH for measure trigger
      GPIO_PORTD_AHB_DATA_R &= ~0x1;         // PD(0) to LOW for measure trigger
   }
   ulVal1 = TIMER0_TAR_R;                    // save first timer-value at capture event
   TIMER0_ICR_R |= (1 << 2);                 // clear Timer0a capture event flag
   TIMER0_CTL_R |= 0x01;                     // re-enable Timer0A
   while((TIMER0_RIS_R & (1 << 2)) == 0);    // wait for capture event
   ulVal2 = TIMER0_TAR_R;                    // save second-timer value at capture event
   TIMER0_ICR_R |= (1 << 2);			     // clear capture event flag
   TIMER0_CTL_R &= ~0x01;                    // disable Timer0A

   if(ulVal1 > ulVal2)
   {
      timeMicroSeconds = ((unsigned short) ((0xFFFF - ulVal2) - (0xFFFF - ulVal1)) / 32);
      timeMilliSeconds = timeMicroSeconds * 0.001;
      measureDistance = timeMilliSeconds * 34.4;
      printf("%d\n", measureDistance);
   }
   return measureDistance;
}

//*****************************************************************************
//
// LED-number-output function
//
//*****************************************************************************
void ledOutputDigit(int digit)
{
	int i = 0;
   if (0 == digit)
   {
      unsigned char arr[5] = LED_ZERO;
      for(i = 4; i < 0; i--)
      {
         GPIO_PORTM_DATA_R |= arr[i];
         wait(200);
         GPIO_PORTM_DATA_R &= ~0xFF;
         wait(200);
      }
   }
   else if (1 == digit)
   {
      unsigned char arr[5] = LED_ONE;
      for(i = 4; i < 0; i--)
      {
         GPIO_PORTM_DATA_R |= arr[i];
         wait(200);
         GPIO_PORTM_DATA_R &= ~0xFF;
         wait(200);
      }
   }
   else if(2 == digit)
   {
      unsigned char arr[5] = LED_TWO;
      for(i = 4; i < 0; i--)
      {
         GPIO_PORTM_DATA_R |= arr[i];
         wait(200);
         GPIO_PORTM_DATA_R &= ~0xFF;
         wait(200);
      }
   }
   else if(3 == digit)
   {
      unsigned char arr[5] = LED_THREE;
      for(i = 4; i < 0; i--)
      {
         GPIO_PORTM_DATA_R |= arr[i];
         wait(200);
         GPIO_PORTM_DATA_R &= ~0xFF;
         wait(200);
      }
   }
   else if(4 == digit)
   {
      unsigned char arr[5] = LED_FOUR;
      for(i = 4; i < 0; i--)
      {
         GPIO_PORTM_DATA_R |= arr[i];
         wait(200);
         GPIO_PORTM_DATA_R &= ~0xFF;
         wait(200);
      }
   }
   else if(5 == digit)
   {
      unsigned char arr[5] = LED_FIVE;
      for(i = 4; i < 0; i--)
      {
         GPIO_PORTM_DATA_R |= arr[i];
         wait(200);
         GPIO_PORTM_DATA_R &= ~0xFF;
         wait(200);
      }
   }
   else if(6 == digit)
   {
      unsigned char arr[5] = LED_SIX;
      for(i = 4; i < 0; i--)
      {
         GPIO_PORTM_DATA_R |= arr[i];
         wait(200);
         GPIO_PORTM_DATA_R &= ~0xFF;
         wait(200);
      }
   }
   else if(7 == digit)
   {
      unsigned char arr[5] = LED_SEVEN;
      for(i = 4; i < 0; i--)
      {
         GPIO_PORTM_DATA_R |= arr[i];
         wait(200);
         GPIO_PORTM_DATA_R &= ~0xFF;
         wait(200);
      }
   }
   else if(8 == digit)
   {
      unsigned char arr[5] = LED_EIGHT;
      for(i = 4; i < 0; i--)
      {
         GPIO_PORTM_DATA_R |= arr[i];
         wait(200);
         GPIO_PORTM_DATA_R &= ~0xFF;
         wait(200);
      }
   }
   else if(9 == digit)
   {
      unsigned char arr[5] = LED_NINE;
      for(i = 4; i < 0; i--)
      {
         GPIO_PORTM_DATA_R |= arr[i];
         wait(200);
         GPIO_PORTM_DATA_R &= ~0xFF;
         wait(200);
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
   wait(400);
}

//*****************************************************************************
//
// LED-letter-output functions
//
//*****************************************************************************
void ledOutputLetterC(void)
{
   int i = 0;
   unsigned char arrc[5] = LED_C;
   for(i = 4; i < 0; i--)
   {
      GPIO_PORTM_DATA_R |= arrc[i]; // PM(7:0) for LED_C
      wait(200);
      GPIO_PORTM_DATA_R &= ~0xFF;
      wait(200);
   }

}
void ledOutputLetterM(void)
{
   int i = 4;
   unsigned char arrm[5] = LED_M;
   while(i < 0)
   {
      GPIO_PORTM_DATA_R |= 0xFF; // PM(7:0) for LED_M
      wait(6000);                    // würde es erstmal ohne wait-funktionversuchen und dann langsam hoch arbeiten warscheinlich ist 200 zu hoch
      //GPIO_PORTM_DATA_R &= ~0xFF;
      wait(160);
      i--;
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
   while((TIMER1_RIS_R & (1 << 0)) == 0); // time-out value after 18ms
   TIMER1_ICR_R |= (1 << 4);              // clear Timer1A time-out flag
   TIMER1_CTL_R &= ~0x01;                 // disable Timer1A
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
   int i = 4;

   configurePorts();
   configureTimer();

   while (1)
   {
      // positive edge Pendulum-LED
      newPendulumInput = GPIO_PORTL_DATA_R;                                  // PK(0) read input edge
      if((oldPendulumInput != newPendulumInput) && (newPendulumInput == 0x01))   // positive-edge-signal
      {
    	  GPIO_PORTM_DATA_R &= ~0xFF;
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
    	  GPIO_PORTM_DATA_R &= ~0xFF;
    	  wait(25000);

      }

	  else if((oldPendulumInput == newPendulumInput) && (newPendulumInput == 0x00)) // negative-edge-signal
	  {
		        unsigned char arrm[5] = LED_M;
		        unsigned char arrc[5] = LED_C;
		        unsigned char arr9[5] = LED_NINE;
		        unsigned char arr5[5] = LED_FIVE;

		        GPIO_PORTM_DATA_R |= arrm[4]; // PM(7:0) for LED_M
				wait(1000);                    // würde es erstmal ohne wait-funktionversuchen und dann langsam hoch arbeiten warscheinlich ist 200 zu hoch
				GPIO_PORTM_DATA_R &= ~0xFF;
				wait(200);

    	        GPIO_PORTM_DATA_R |= arrm[3]; // PM(7:0) for LED_M
				wait(1000);                    // würde es erstmal ohne wait-funktionversuchen und dann langsam hoch arbeiten warscheinlich ist 200 zu hoch
				GPIO_PORTM_DATA_R &= ~0xFF;
				wait(200);

				GPIO_PORTM_DATA_R |= arrm[2]; // PM(7:0) for LED_M
				wait(1000);                    // würde es erstmal ohne wait-funktionversuchen und dann langsam hoch arbeiten warscheinlich ist 200 zu hoch
				GPIO_PORTM_DATA_R &= ~0xFF;
				wait(200);

				GPIO_PORTM_DATA_R |= arrm[1]; // PM(7:0) for LED_M
				wait(1000);                    // würde es erstmal ohne wait-funktionversuchen und dann langsam hoch arbeiten warscheinlich ist 200 zu hoch
				GPIO_PORTM_DATA_R &= ~0xFF;
				wait(200);

				GPIO_PORTM_DATA_R |= arrm[0]; // PM(7:0) for LED_M
				wait(1000);                    // würde es erstmal ohne wait-funktionversuchen und dann langsam hoch arbeiten warscheinlich ist 200 zu hoch
				GPIO_PORTM_DATA_R &= ~0xFF;
				wait(805);

				GPIO_PORTM_DATA_R |= arrc[4]; // PM(7:0) for LED_M
				wait(1000);                    // würde es erstmal ohne wait-funktionversuchen und dann langsam hoch arbeiten warscheinlich ist 200 zu hoch
				GPIO_PORTM_DATA_R &= ~0xFF;
				wait(200);

				GPIO_PORTM_DATA_R |= arrc[3]; // PM(7:0) for LED_M
				wait(1000);                    // würde es erstmal ohne wait-funktionversuchen und dann langsam hoch arbeiten warscheinlich ist 200 zu hoch
				GPIO_PORTM_DATA_R &= ~0xFF;
				wait(200);

				GPIO_PORTM_DATA_R |= arrc[2]; // PM(7:0) for LED_M
				wait(1000);                    // würde es erstmal ohne wait-funktionversuchen und dann langsam hoch arbeiten warscheinlich ist 200 zu hoch
				GPIO_PORTM_DATA_R &= ~0xFF;
				wait(200);

				GPIO_PORTM_DATA_R |= arrc[1]; // PM(7:0) for LED_M
				wait(1000);                    // würde es erstmal ohne wait-funktionversuchen und dann langsam hoch arbeiten warscheinlich ist 200 zu hoch
				GPIO_PORTM_DATA_R &= ~0xFF;
				wait(200);

				GPIO_PORTM_DATA_R |= arrc[0]; // PM(7:0) for LED_M
				wait(1000);                    // würde es erstmal ohne wait-funktionversuchen und dann langsam hoch arbeiten warscheinlich ist 200 zu hoch
				GPIO_PORTM_DATA_R &= ~0xFF;
				wait(12000);

				GPIO_PORTM_DATA_R |= arr9[4]; // PM(7:0) for LED_M
				wait(1000);                    // würde es erstmal ohne wait-funktionversuchen und dann langsam hoch arbeiten warscheinlich ist 200 zu hoch
				GPIO_PORTM_DATA_R &= ~0xFF;
				wait(200);

				GPIO_PORTM_DATA_R |= arr9[3]; // PM(7:0) for LED_M
				wait(1000);                    // würde es erstmal ohne wait-funktionversuchen und dann langsam hoch arbeiten warscheinlich ist 200 zu hoch
				GPIO_PORTM_DATA_R &= ~0xFF;
				wait(200);

				GPIO_PORTM_DATA_R |= arr9[2]; // PM(7:0) for LED_M
				wait(1000);                    // würde es erstmal ohne wait-funktionversuchen und dann langsam hoch arbeiten warscheinlich ist 200 zu hoch
				GPIO_PORTM_DATA_R &= ~0xFF;
				wait(200);

				GPIO_PORTM_DATA_R |= arr9[1]; // PM(7:0) for LED_M
				wait(1000);                    // würde es erstmal ohne wait-funktionversuchen und dann langsam hoch arbeiten warscheinlich ist 200 zu hoch
				GPIO_PORTM_DATA_R &= ~0xFF;
				wait(200);

				GPIO_PORTM_DATA_R |= arr9[0]; // PM(7:0) for LED_M
				wait(1000);                    // würde es erstmal ohne wait-funktionversuchen und dann langsam hoch arbeiten warscheinlich ist 200 zu hoch
				GPIO_PORTM_DATA_R &= ~0xFF;
				wait(805);

				GPIO_PORTM_DATA_R |= arr5[4]; // PM(7:0) for LED_M
				wait(1000);                    // würde es erstmal ohne wait-funktionversuchen und dann langsam hoch arbeiten warscheinlich ist 200 zu hoch
				GPIO_PORTM_DATA_R &= ~0xFF;
				wait(200);

				GPIO_PORTM_DATA_R |= arr5[3]; // PM(7:0) for LED_M
				wait(1000);                    // würde es erstmal ohne wait-funktionversuchen und dann langsam hoch arbeiten warscheinlich ist 200 zu hoch
				GPIO_PORTM_DATA_R &= ~0xFF;
				wait(200);

				GPIO_PORTM_DATA_R |= arr5[2]; // PM(7:0) for LED_M
				wait(1000);                    // würde es erstmal ohne wait-funktionversuchen und dann langsam hoch arbeiten warscheinlich ist 200 zu hoch
				GPIO_PORTM_DATA_R &= ~0xFF;
				wait(200);

				GPIO_PORTM_DATA_R |= arr5[1]; // PM(7:0) for LED_M
				wait(1000);                    // würde es erstmal ohne wait-funktionversuchen und dann langsam hoch arbeiten warscheinlich ist 200 zu hoch
				GPIO_PORTM_DATA_R &= ~0xFF;
				wait(200);

				GPIO_PORTM_DATA_R |= arr5[0]; // PM(7:0) for LED_M
				wait(1000);                    // würde es erstmal ohne wait-funktionversuchen und dann langsam hoch arbeiten warscheinlich ist 200 zu hoch
				GPIO_PORTM_DATA_R &= ~0xFF;
				while((oldPendulumInput == newPendulumInput) && (newPendulumInput == 0x00));

	  }


        /* ledOutputLetterM();
         ledOutputSpace();
         ledOutputLetterC();
         ledOutputSpace();
         ledOutputSpace();
         ledOutputDigit(secondDigit);
         ledOutputSpace();
         ledOutputDigit(firstDigit);
         oneSideBlackBar();*/


      oldPendulumInput = newPendulumInput;   // store new signal to old input signal
   }
}

