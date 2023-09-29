#include <main.h>

static const struct ke_msg_handler appm_default_state[] =
{
    { KE_MSG_DEFAULT_HANDLER, (ke_msg_func_t)MsgHandler_Notify }
};

/* Use the state and event handler definition for all states. */
static const struct ke_state_handler appm_default_handler
    = KE_STATE_HANDLER(appm_default_state);

/* Defines a place holder for all task instance's state */
ke_state_t appm_state[APP_IDX_MAX];

static const struct ke_task_desc TASK_DESC_APP = {
    .state_handler = NULL,
    .default_handler = &appm_default_handler,
    .state = appm_state,
    .state_max = 0,
    .idx_max = APP_IDX_MAX
};


/* Device Information structure initialization, includes length and data string */
const struct DISS_DeviceInfo_t deviceInfo =
{
    .MANUFACTURER_NAME  = {.len = APP_DIS_MANUFACTURER_NAME_LEN, .data = (uint8_t*) APP_DIS_MANUFACTURER_NAME},
    .MODEL_NB_STR       = {.len = APP_DIS_MODEL_NB_STR_LEN, .data = (uint8_t*) APP_DIS_MODEL_NB_STR},
    .SERIAL_NB_STR      = {.len = APP_DIS_SERIAL_NB_STR_LEN, .data = (uint8_t*) APP_DIS_SERIAL_NB_STR},
    .FIRM_REV_STR       = {.len = APP_DIS_FIRM_REV_STR_LEN, .data = (uint8_t*) APP_DIS_FIRM_REV_STR},
    .SYSTEM_ID          = {.len = APP_DIS_SYSTEM_ID_LEN, .data = (uint8_t*) APP_DIS_SYSTEM_ID},
    .HARD_REV_STR       = {.len = APP_DIS_HARD_REV_STR_LEN, .data = (uint8_t*) APP_DIS_HARD_REV_STR},
    .SW_REV_STR         = {.len = APP_DIS_SW_REV_STR_LEN, .data = (uint8_t*) APP_DIS_SW_REV_STR},
    .IEEE               = {.len = APP_DIS_IEEE_LEN, .data = (uint8_t*) APP_DIS_IEEE},
    .PNP                = {.len = APP_DIS_PNP_ID_LEN, .data = (uint8_t*) APP_DIS_PNP_ID},
};

struct gapm_set_dev_config_cmd devConfigCmd =
{
    .operation = GAPM_SET_DEV_CONFIG,
    .role = GAP_ROLE_PERIPHERAL,
    .renew_dur = GAPM_DEFAULT_RENEW_DUR,
    .addr.addr = APP_BD_ADDRESS,
    .irk.key = APP_IRK,
    .addr_type = APP_BD_ADDRESS_TYPE,
#ifdef SECURE_CONNECTION
    .pairing_mode = (GAPM_PAIRING_SEC_CON | GAPM_PAIRING_LEGACY),
#else
    .pairing_mode = GAPM_PAIRING_LEGACY,
#endif
    .gap_start_hdl = GAPM_DEFAULT_GAP_START_HDL,
    .gatt_start_hdl = GAPM_DEFAULT_GATT_START_HDL,
	/* Remove GAPM_WRITE_UNAUTH to disable writing to the device name */
    .att_and_ext_cfg = GAPM_DEFAULT_ATT_CFG | GAPM_WRITE_UNAUTH,
    .sugg_max_tx_octets = GAPM_DEFAULT_TX_OCT_MAX,
    .sugg_max_tx_time = GAPM_DEFAULT_TX_TIME_MAX,
    .max_mtu = GAPM_DEFAULT_MTU_MAX,
    .max_mps = GAPM_DEFAULT_MPS_MAX,
    .max_nb_lecb = GAPM_DEFAULT_MAX_NB_LECB,
    .audio_cfg = GAPM_DEFAULT_AUDIO_CFG,
    .tx_pref_rates = GAP_RATE_ANY,
    .rx_pref_rates = GAP_RATE_ANY
};

/* ADV_DATA and SCAN_RSP data are set in APP_BLE_Initialize() */
union gapm_adv_info app_adv_info;

static uint8_t devInfoCfmName[sizeof(uint16_t)+APP_DEVICE_NAME_MAXLEN];
union gapc_dev_info_val *getDevInfoCfmName = (union gapc_dev_info_val*) devInfoCfmName;

union gapc_dev_info_val getDevInfoCfmAppearance =
{
    .appearance = APP_DEVICE_APPEARANCE_DEFAULT
};

union gapc_dev_info_val getDevInfoCfmSlvParams =
{
    .slv_params = {APP_PREF_SLV_MIN_CON_INTERVAL,
                   APP_PREF_SLV_MAX_CON_INTERVAL,
                   APP_PREF_SLV_LATENCY,
                   APP_PREF_SLV_SUP_TIMEOUT}
};

union gapc_dev_info_val* getDevInfoCfm[] =
{
    [GAPC_DEV_NAME] = (union gapc_dev_info_val*) devInfoCfmName,
    [GAPC_DEV_APPEARANCE] = &getDevInfoCfmAppearance,
    [GAPC_DEV_SLV_PREF_PARAMS] = &getDevInfoCfmSlvParams
};

/** Flag indication is white list mode is active */
bool whitelist_active = true;

void vInitXTAL(void)
{
   
 /* Test DIO12 to pause the program to make it easy to re-flash */
    DIO->CFG[RECOVERY_DIO] = DIO_MODE_INPUT  | DIO_WEAK_PULL_UP |
                             DIO_LPF_DISABLE | DIO_6X_DRIVE;
    while (DIO_DATA->ALIAS[RECOVERY_DIO] == 0);

    /* Prepare the 48 MHz crystal
     * Start and configure VDDRF */
    ACS_VDDRF_CTRL->ENABLE_ALIAS = VDDRF_ENABLE_BITBAND;
    ACS_VDDRF_CTRL->CLAMP_ALIAS  = VDDRF_DISABLE_HIZ_BITBAND;

    /* Wait until VDDRF supply has powered up */
    while (ACS_VDDRF_CTRL->READY_ALIAS != VDDRF_READY_BITBAND);

    /* Enable RF power switches */
    SYSCTRL_RF_POWER_CFG->RF_POWER_ALIAS   = RF_POWER_ENABLE_BITBAND;

    /* Remove RF isolation */
    SYSCTRL_RF_ACCESS_CFG->RF_ACCESS_ALIAS = RF_ACCESS_ENABLE_BITBAND;

    /* Start the 48 MHz oscillator without changing the other register bits */
    RF->XTAL_CTRL = ((RF->XTAL_CTRL & ~XTAL_CTRL_DISABLE_OSCILLATOR) |
                     XTAL_CTRL_REG_VALUE_SEL_INTERNAL);
                     
    RF_REG2F->CK_DIV_1_6_CK_DIV_1_6_BYTE = CK_DIV_1_6_PRESCALE_1_BYTE;

    /* Wait until 48 MHz oscillator is started */
    while (RF_REG39->ANALOG_INFO_CLK_DIG_READY_ALIAS !=
           ANALOG_INFO_CLK_DIG_READY_BITBAND);

    Sys_Clocks_SystemClkConfig(JTCK_PRESCALE_1   |
                               EXTCLK_PRESCALE_1 |
                               SYSCLK_CLKSRC_RFCLK);
    
        /* Configure clock dividers */
    CLK->DIV_CFG0 = (SLOWCLK_PRESCALE_8 | BBCLK_PRESCALE_2 | //User = 48/1 = 48MHz
                     USRCLK_PRESCALE_1);                     //SLOW = 48/8 = 6MHz 
                                                             // BBCLK = 24MHz
    BBIF->CTRL = (BB_CLK_ENABLE | BBCLK_DIVIDER_24 | BB_WAKEUP); // Clk for BLE = 1MHz
    CLK->DIV_CFG2 = (CPCLK_PRESCALE_6 | DCCLK_PRESCALE_2);
    __set_PRIMASK(PRIMASK_ENABLE_INTERRUPTS);
}

void vInitPWM(void)
{
  PWM_CFG->PWM_PERIOD_BYTE = 254;
  PWM_CFG->PWM_HIGH_BYTE = 1;
  PWM_CTRL->PWM0_ENABLE_ALIAS = PWM0_ENABLE_BITBAND;
}
void vInitButton(void)
{
  Sys_DIO_Config(BUTTON_DIO, DIO_MODE_GPIO_IN_0 | DIO_WEAK_PULL_UP |
                 DIO_LPF_DISABLE);
  Sys_DIO_IntConfig(0,
                    DIO_EVENT_RISING_EDGE | DIO_SRC_DIO_5 |
                    DIO_DEBOUNCE_ENABLE,
                    DIO_DEBOUNCE_SLOWCLK_DIV1024, 49);
  NVIC_EnableIRQ(DIO0_IRQn);
  
  
}

void vInitADC(void)
{
    Sys_ADC_Set_Config(ADC_VBAT_DIV2_NORMAL | ADC_NORMAL | ADC_PRESCALE_6400);
    
  
    Sys_ADC_Set_BATMONIntConfig(INT_EBL_ADC |
                                ADC_INT_CH1 |
                                INT_EBL_BATMON_ALARM);
    Sys_ADC_InputSelectConfig(1, ADC_POS_INPUT_DIO1 |
                              ADC_NEG_INPUT_GND);
    Sys_DIO_Config(1, DIO_MODE_GPIO_IN_1);
  
  
    NVIC_EnableIRQ(ADC_BATMON_IRQn);
    /* Unmask all interrupts */
    __set_PRIMASK(PRIMASK_ENABLE_INTERRUPTS);
}
void vInitBLE(void)
{
      /* Configure DIOs */
    Sys_DIO_Config(LED_DIO_NUM, DIO_MODE_GPIO_OUT_0);

    /* Seed the random number generator */
    srand(1);

    /* Initialize the kernel and Bluetooth stack */
    Kernel_Init(0);
    BLE_InitNoTL(0);
    
	/* Set radio output power of RF */
    Sys_RFFE_SetTXPower(OUTPUT_POWER_DBM);
	
    /* Create the application task handler */
    ke_task_create(TASK_APP, &TASK_DESC_APP);

    /* Enable Bluetooth related interrupts */
    NVIC->ISER[1] = (NVIC_BLE_CSCNT_INT_ENABLE      |
                     NVIC_BLE_SLP_INT_ENABLE        |
                     NVIC_BLE_RX_INT_ENABLE         |
                     NVIC_BLE_EVENT_INT_ENABLE      |
                     NVIC_BLE_CRYPT_INT_ENABLE      |
                     NVIC_BLE_ERROR_INT_ENABLE      |
                     NVIC_BLE_GROSSTGTIM_INT_ENABLE |
                     NVIC_BLE_FINETGTIM_INT_ENABLE  |
                     NVIC_BLE_SW_INT_ENABLE);

    /* Stop masking interrupts */
    __set_PRIMASK(PRIMASK_ENABLE_INTERRUPTS);
    __set_FAULTMASK(FAULTMASK_ENABLE_INTERRUPTS);
}

/* ----------------------------------------------------------------------------
 * Function      : void APP_InitDevInfo(void)
 * ----------------------------------------------------------------------------
 * Description   : Initializes the device info data structure
 * Inputs        : None
 * Outputs       : None
 * Assumptions   : None
 * ------------------------------------------------------------------------- */
/**
 * @brief Initializes the device info data structure
 *
 * Assumptions   : None
 */
void APP_InitDevInfo(void)
{
    getDevInfoCfmName->name.length = sizeof(APP_DEVICE_NAME_DEFAULT)-1;
    memcpy(getDevInfoCfmName->name.value, APP_DEVICE_NAME_DEFAULT,
            sizeof(APP_DEVICE_NAME_DEFAULT)-1);
}

/* ----------------------------------------------------------------------------
 * Function      : void APP_SetAdvScanData(void)
 * ----------------------------------------------------------------------------
 * Description   : Initializes the advertisement data structure
 * Inputs        : None
 * Outputs       : None
 * Assumptions   : None
 * ------------------------------------------------------------------------- */
/**
 * @brief Initializes the advertisement data structure
 *
 * Assumptions   : None
 */
void APP_SetAdvScanData(void)
{
    uint8_t companyID[] = APP_COMPANY_ID;
    uint8_t devName[]   = APP_DEVICE_NAME_DEFAULT;
    uint8_t hrp_uuid[]  = HRPS_SERVICE_UUID;

    /* Set advertising data as device name */
    app_adv_info.host.adv_data_len = 0;
    GAPM_AddAdvData(GAP_AD_TYPE_COMPLETE_NAME, devName,
                    sizeof(APP_DEVICE_NAME_DEFAULT)-1, app_adv_info.host.adv_data,
                    &app_adv_info.host.adv_data_len);

    /* Set scan response data as heart rate UUID and company ID */
    app_adv_info.host.scan_rsp_data_len = 0;
    GAPM_AddAdvData(GAP_AD_TYPE_SERVICE_16_BIT_DATA, hrp_uuid,
                    sizeof(hrp_uuid), app_adv_info.host.scan_rsp_data,
                    &app_adv_info.host.scan_rsp_data_len);
    GAPM_AddAdvData(GAP_AD_TYPE_MANU_SPECIFIC_DATA, companyID,
                    APP_COMPANY_ID_LEN, app_adv_info.host.scan_rsp_data,
                    &app_adv_info.host.scan_rsp_data_len);
}

void APP_SetConnectionCfmParams(uint8_t conidx, struct gapc_connection_cfm* cfm)
{
    cfm->svc_changed_ind_enable = 0;
    cfm->ltk_present = false;
#ifdef SECURE_CONNECTION
        cfm->pairing_lvl = GAP_PAIRING_BOND_SECURE_CON;
#else
    cfm->pairing_lvl = GAP_PAIRING_BOND_UNAUTH;
#endif

#if CFG_BOND_LIST_IN_NVR2
    if(GAPC_IsBonded(conidx))
    {
        cfm->ltk_present = true;
        Device_Param_Read(PARAM_ID_CSRK, cfm->lcsrk.key);
        memcpy(cfm->rcsrk.key, GAPC_GetBondInfo(conidx)->CSRK, KEY_LEN);
        cfm->lsign_counter = 0xFFFFFFFF;
        cfm->rsign_counter = 0;
		cfm->pairing_lvl = GAPC_GetBondInfo(conidx)->PAIRING_LVL;
    }
#endif
}

/* ----------------------------------------------------------------------------
 * Function      : Device_Param_Prepare(struct app_device_param * param)
 * ----------------------------------------------------------------------------
 * Description   : This function allows the application to overwrite a few BLE
 *                 parameters (BD address and keys) without having to write
 *                 data into RSL10 flash (NVR3). This function is called by the
 *                 stack and it's useful for debugging and testing purposes.
 * Inputs        : - param    - pointer to the parameters to be configured
 * Outputs       : None
 * Assumptions   : None
 * ------------------------------------------------------------------------- */
void Device_Param_Prepare(app_device_param_t *param)
{
    param->device_param_src_type = APP_BLE_DEV_PARAM_SOURCE;

    if(param->device_param_src_type == APP_PROVIDED)
    {
        uint8_t temp_bleAddress[6] = APP_BD_ADDRESS;
        uint8_t temp_irk[16] = APP_IRK;
        uint8_t temp_csrk[16] = APP_CSRK;
        uint8_t temp_privateKey[32] = APP_PRIVATE_KEY;
        uint8_t temp_publicKey_x[32] = APP_PUBLIC_KEY_X;
        uint8_t temp_publicKey_y[32] = APP_PUBLIC_KEY_Y;

        memcpy(param->bleAddress, temp_bleAddress, 6);
        memcpy(param->irk, temp_irk, 16);
        memcpy(param->csrk, temp_csrk, 16);
        memcpy(param->privateKey, temp_privateKey, 32);
        memcpy(param->publicKey_x, temp_publicKey_x, 32);
        memcpy(param->publicKey_y, temp_publicKey_y, 32);
    }
}

/* ----------------------------------------------------------------------------
 * Function      : bool APP_WhitelistIsActive(void)
 * ----------------------------------------------------------------------------
 * Description   : Returns the current state of the whitelist flag
 * Inputs        : None
 * Outputs       : Return value: true is the whitelist is active, false if it is inactive.
 * Assumptions   : None
 * ------------------------------------------------------------------------- */
bool APP_WhitelistIsActive(void)
{
    return whitelist_active;
}

/* ----------------------------------------------------------------------------
 * Function      : void APP_WhitelistSetInactive(void)
 * ----------------------------------------------------------------------------
 * Description   : Sets the whitelist flag to false
 * Inputs        : None
 * Outputs       : None
 * Assumptions   : None
 * ------------------------------------------------------------------------- */
void APP_WhitelistSetInactive(void)
{
    whitelist_active = false;
}





