// Laborversuch 2 Aufgabe 1

#include <stdio.h>
#include <stdint.h>
#include "tm4c1294ncpdt.h"

void wait(unsigned long time)
{
   for (unsigned long i = 0; i < time; i++);                // wait-loop
}

void main(int argc, char const *argv[])
{
   unsigned char old_input, new_input = 0x00;               // local input signal storage

   SYSCTL_RCGCGPIO_R |= ((1 << 10) | (1 << 11));            // clock enable port L & M
   while((SYSCTL_PRGPIO_R & ((1 << 10) | (1 << 11))) == 0); // control peripheral ready bits
   GPIO_PORTL_DEN_R = 0x01;                                 // digital I/O pin PL(0)
   GPIO_PORTM_DEN_R = 0xFF;                                 // digital I/O pins PM(0:7)
   GPIO_PORTL_DIR_R = 0x00;                                 // define input direction for PL(0)
   GPIO_PORTM_DIR_R = 0xFF;                                 // define output direction for PM(0:7)

   while(1)
   {
      new_input = GPIO_PORTL_DATA_R;                        // read new input signal
      if(old_input != new_input)                            // compare old with new input signal
      {
         GPIO_PORTM_DATA_R |= 0xFF;                         // high output signal to PM(0:7)
         wait(100);                                         // wait output signal until it reachess the LEDs
      }
      else
      {
         GPIO_PORTM_DATA_R &= 0x00;                         // low output signal to PM(0:7)
         wait(100);                                         // wait output signals until they arrive
      }
      old_input = new_input;                                // store new signal to old input signal
   }
}
