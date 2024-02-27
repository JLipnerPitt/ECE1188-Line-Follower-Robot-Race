#include "Reflectance.h"
#include <msp.h>
#include "Clock.h"
#include <stdbool.h>

void Reflectance_Init(void){

    // P5.3 configured as output
    P5->SEL0 &= ~0x08;
    P5->SEL1 &= ~0x08;
    P5->DIR |= 0x08;
    P5->OUT &= ~0x08;

    // P9.2 configured as output
    P9->SEL0 &= ~0x04;
    P9->SEL1 &= ~0x04;
    P9->DIR |= 0x04;
    P9->OUT &= ~0x04;

    // sensor line configured as input
    P7->SEL0 &= ~0xFF;
    P7->SEL1 &= ~0xFF;
    P7->DIR &= ~0xFF;
    P7->OUT &= ~0xFF;
}

void Reflectance_Start(void) {
    P5->OUT |= 0x08; // turns on even IR sensors
    P9->OUT |= 0x04; // turns on odd IR sensors

    P7->DIR |= 0xFF; // makes P7 output
    P7->OUT |= 0xFF; // starts charging capacitors

    Clock_Delay1us(10); // wait 10 us
    P7->DIR &= ~0xFF; // makes P7 input
    //Clock_Delay1us(1000);
    //IRSensorInput = P7->IN;
}

uint8_t Reflectance_End(void) {
    uint8_t input = P7->IN;
    P5->OUT &= ~0x08; // turns off even IR sensors
    P9->OUT &= ~0x04; // turns on even IR sensors
    return input;
}

// checks how many sensors are activated
/*uint8_t Num_Of_On_Sensors(uint8_t input) {
    uint8_t i;
    uint8_t bit_number;
    uint8_t count = 0;
    bool flag = false;

    for (i=0;i<8;i++) {
        if ((input >> i) & 0x01 == 0) {
            bit_number = i;
            count++;
        }
    }
    if (count == 0) {
        return 0;
    }
    flag = true;
    return bit_number;
}*/

uint8_t decision(uint8_t input) {
    switch (input) {
      case 0xFF: return 0;

      case 0xC3: return 1;

      case 0xE3:
      case 0xC7: return 2;

      case 0xE7:
      case 0xCF:
      case 0xF3: return 3;
      //case 0x81:return 3;

      case 0x9F:
      case 0x8F:
      case 0x87: return 4;

      case 0x3F:
      case 0x1F: return 5;

      case 0x0F: return 6;

      case 0x07: return 7;

      case 0xF9:
      case 0xF1:
      case 0xE1: return 8;

      case 0xFC:
      case 0xF8: return 9;

      case 0xF0: return 10;

      case 0xE0: return 11;

      default: return 0;
    }
}
