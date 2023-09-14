#include <main.h>



void vTaskCode_ONE(void * pvParameters)
{
  for(;;)
  {
    Sys_GPIO_Toggle(LED_DIO); 
    vTaskDelay(5500 / portTICK_PERIOD_MS);
    
  }
}

void vTaskCode_TWO(void * pvParameters)
{
  for(;;)
  {
    Sys_GPIO_Toggle(LED_DIO); 
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}


int main(void)
{
    vInitXTAL();
    Sys_DIO_Config(LED_DIO, DIO_MODE_GPIO_OUT_0);
//    Sys_DIO_Config(LED_DIO, DIO_MODE_PWM0);
    vInitPWM();
    vInitButton();
    xTaskCreate(vTaskCode_ONE, "Task1", 50, NULL, 1, NULL);
    xTaskCreate(vTaskCode_TWO, "Task2", 50, NULL, 1, NULL);

    vTaskStartScheduler();
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