#include <RSL10.h>



/* DIO number that is used for easy re-flashing (recovery mode) */
#define RECOVERY_DIO 12
#define LED_DIO 6
/* DIO number that is connected to push button of EVB */
#define BUTTON_DIO 5


void vInitXTAL(void);
void vInitPWM(void);
void vInitButton(void);
void DIO0_IRQHandler(void);

