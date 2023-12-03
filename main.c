#include <main.h>

uint16_t adc_buffer[ADC_BUFFER_SIZE];
uint8_t uart_buffer[UART_BUFFER_SIZE];


extern const struct DISS_DeviceInfo_t deviceInfo;
/* ----------------------------------------------------------------------------
 * Function      : void BASS_Setup(void)
 * ----------------------------------------------------------------------------
 * Description   : Configure the Battery Service Server
 * Inputs        : None
 * Outputs       : None
 * Assumptions   : None
 * ------------------------------------------------------------------------- */
static void BASS_Setup(void)
{
    BASS_Initialize(APP_BAS_NB, APP_BASS_ReadBatteryLevel);
    BASS_NotifyOnBattLevelChange(TIMER_SETTING_S(2));     /* Periodically monitor the battery level. Only notify changes */
    BASS_NotifyOnTimeout(TIMER_SETTING_S(6));             /* Periodically notify the battery level to connected peers */
    APP_BASS_SetBatMonAlarm(BATMON_SUPPLY_THRESHOLD_CFG); /* BATMON alarm configuration */
}

static void DISS_Setup(void)
{
    DISS_Initialize(APP_DIS_FEATURES, (const struct DISS_DeviceInfo_t*) &deviceInfo);
}
static void HRPS_Setup(void)
{
    HRPS_Initialize(TIMER_SETTING_S(1),
            HRPS_BODY_SENSOR_LOC_CHAR_SUP | HRPS_ENGY_EXP_FEAT_SUP | HRPS_HR_MEAS_NTF_CFG,
            HRS_LOC_CHEST,
            &app_adv_info,
            APP_HRPS_HeartRateMeasurementUpdate,
            APP_HRPS_EnergyExpResetInd);
    APP_HRPS_Initialize();
}
void vADCTask(void *pvParameters)
{
    for(;;)
    {

    }
}



int main(void)
{
    vInitXTAL();
    Sys_DIO_Config(LED_DIO, DIO_MODE_GPIO_OUT_0);
//    Sys_DIO_Config(LED_DIO, DIO_MODE_PWM0);
    vInitPWM();
    vInitButton();
    //vInitADC();
    vInitBLE();
      /* Configure application-specific advertising data and scan response data. */
    APP_SetAdvScanData();

    APP_InitDevInfo();
    BASS_Setup();
    DISS_Setup();
  
      /* Add application message handlers */
    MsgHandler_Add(TASK_ID_GAPM, APP_GAPM_GATTM_Handler);
    MsgHandler_Add(GATTM_ADD_SVC_RSP, APP_GAPM_GATTM_Handler);
    MsgHandler_Add(TASK_ID_GAPC, APP_GAPC_Handler);
    MsgHandler_Add(APP_LED_TIMEOUT, APP_LED_Timeout_Handler);
    MsgHandler_Add(APP_BATT_LEVEL_LOW, APP_BASS_BattLevelLow_Handler);
    MsgHandler_Add(APP_TIMEOUT_WHITELIST, APP_WhitelistTimerHandler);
    MsgHandler_Add(APP_BATT_LEVEL_READ_TIMEOUT, APP_BASS_ReadBatteryLevel_Handler);

    /* Reset the GAP manager. Trigger GAPM_CMP_EVT / GAPM_RESET when finished. See APP_GAPM_GATTM_Handler */
    GAPM_ResetCmd();

    while (1)
    {
      
        Kernel_Schedule();    /* Dispatch all events in Kernel queue */
        Sys_Watchdog_Refresh();
        SYS_WAIT_FOR_EVENT;    /* Wait for an event before re-executing the scheduler */
      
    }
}