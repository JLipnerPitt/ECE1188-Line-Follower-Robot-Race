#ifndef REFLECTANCE_H
#define REFLECTANCE_H

#include <stdint.h>

uint8_t IRSensorInput;
void Reflectance_Init(void);
void Reflectance_Start(void);
uint8_t Reflectance_End(void);
uint8_t Num_Of_On_Sensors(uint8_t);

#endif
