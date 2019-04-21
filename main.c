// The MIT License (MIT)

// Copyright (c) 2019 Dae-Jin Seon

//  Permission is hereby granted, free of charge, to any person obtaining a
//  copy of this software and associated documentation files (the "Software"),
//  to deal in the Software without restriction, including without limitation
//  the rights to use, copy, modify, merge, publish, distribute, sublicense,
//  and/or sell copies of the Software, and to permit persons to whom the
//  Software is furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in
//  all copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
//  OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
//  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
//  DEALINGS IN THE SOFTWARE.

/* code */


#include "tm4c1294ncpdt.h"
#include <stdio.h>
#include <stdint.h>

int main(int argc, char const *argv[])
{     
      SYSCTL_RCGCGPIO_R |= (1<<3);                    // clock enable port D
      while((SYSCTL_PRGPIO_R & (1<<3)) == 0);         // clock when port D is available
      GPIO_PORTD_AHB_DEN_R = 0x03;                    // digital I/O pins enabled
      GPIO_PORTD_AHB_DIR_R = 0x01;                    // define output direction I/O pins
      printf("Hello World\n");
      return 0;
}
