#include "main.h"


void DIO0_IRQHandler(void)
{
  if(PWM_CFG->PWM_HIGH_BYTE > 254)
    PWM_CFG->PWM_HIGH_BYTE = 0;
  else
    PWM_CFG->PWM_HIGH_BYTE += 10;
}
  