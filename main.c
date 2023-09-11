#include <main.h>


int main(void)
{
    vInitXTAL();
//    Sys_DIO_Config(LED_DIO, DIO_MODE_GPIO_OUT_0);
    Sys_DIO_Config(LED_DIO, DIO_MODE_PWM0);
    vInitPWM();
    vInitButton();
  
    int t = 0;
    while (1)
    {
      t++;
      if (t == 100000) 
      {
//          Sys_GPIO_Toggle(LED_DIO); 
          t = 0;
      }
    }
}