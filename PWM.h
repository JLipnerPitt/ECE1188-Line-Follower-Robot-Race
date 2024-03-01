
#ifndef PWM_H_
#define PWM_H_


void PWM_Init12(uint16_t period, uint16_t duty1, uint16_t duty2);

void PWM_Init1(uint16_t period, uint16_t duty);

void PWM_Duty1(uint16_t duty1);

void PWM_Duty2(uint16_t duty2);

void PWM_Init34(uint16_t period);

void PWM_Duty34(uint16_t duty3, uint16_t duty4);

#endif /* PWM_H_ */
