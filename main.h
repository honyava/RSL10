#include <my_func.h>
#include <rsl10.h>
#include <rsl10_protocol.h>
#include <ble_gap.h>
#include <ble_gatt.h>
#include <msg_handler.h>
#include <ble_bass.h>
#include <ble_diss.h>
#include <ble_hrps.h>
#include "app_bass.h"
/* DIO number that is used for easy re-flashing (recovery mode) */
#define RECOVERY_DIO 12
#define LED_DIO 6
/* DIO number that is connected to push button of EVB */
#define BUTTON_DIO 5
#define RADIO_CLOCK_ACCURACY 20 /* RF Oscillator accuracy in ppm */
#define ADC_BUFFER_SIZE 100
#define UART_BUFFER_SIZE 100



/* APP Task messages */
enum appm_msg
{
    APPM_DUMMY_MSG = TASK_FIRST_MSG(TASK_ID_APP),
    APP_LED_TIMEOUT,
    APP_BATT_LEVEL_LOW,
    APP_BATT_LEVEL_READ_TIMEOUT,
    APP_TIMEOUT_WHITELIST /* Timeout to accept connections from not previously bonded clients */
};

#define APP_IDX_MAX                     BLE_CONNECTION_MAX /* Number of APP Task Instances */

#define APP_BLE_DEV_PARAM_SOURCE        FLASH_PROVIDED_or_DFLT /* or APP_PROVIDED  */

/* If APP_BD_ADDRESS_TYPE == GAPM_CFG_ADDR_PUBLIC and APP_DEVICE_PARAM_SRC == FLASH_PROVIDED_or_DFLT
 * the bluetooth address is loaded from FLASH NVR3. Otherwise, this address is used. */
#define APP_BD_ADDRESS_TYPE              GAPM_CFG_ADDR_PRIVATE /* or GAPM_CFG_ADDR_PUBLIC*/
#define APP_BD_ADDRESS                   { 0x94, 0x11, 0x22, 0xff, 0xbb, 0xAA }
#define APP_NB_PEERS                     4 /* 1-4 */

/* The number of standard profiles and custom services added in this application */
#define APP_NUM_STD_PRF                 2
#define APP_NUM_CUSTOM_SVC              0

#define LED_DIO_NUM                     6  /* DIO number that is connected to LED of EVB */
#define OUTPUT_POWER_DBM                0  /* RF output power in dBm */
#define RADIO_CLOCK_ACCURACY            20 /* RF Oscillator accuracy in ppm */

/* DIO number that is used for easy re-flashing (recovery mode) */
#define RECOVERY_DIO                    12

/* Timer setting in units of 10ms (kernel timer resolution) */
#define TIMER_SETTING_MS(MS)            (MS / 10)
#define TIMER_SETTING_S(S)              (S * 100)

/* Advertising data is composed by device name and company id */
#define APP_DEVICE_NAME_DEFAULT         "Peripheral_HeartRate"
/* Maximum length for the device name.
 * This is limited by GAP_MAX_NAME_SIZE during the stack build. */
#define APP_DEVICE_NAME_MAXLEN          0x20

/* Manufacturer info (ON SEMICONDUCTOR Company ID) */
#define APP_COMPANY_ID                  {0x62, 0x3}
#define APP_COMPANY_ID_LEN              2

#define APP_DEVICE_APPEARANCE_DEFAULT   0
#define APP_PREF_SLV_MIN_CON_INTERVAL   8
#define APP_PREF_SLV_MAX_CON_INTERVAL   10
#define APP_PREF_SLV_LATENCY            0
#define APP_PREF_SLV_SUP_TIMEOUT        200

/* --------------------------------------------------------------------------
 *  Device Information used for Device Information Server Service (DISS)
 * ----------------------------------------------------------------------- */
/* Manufacturer Name Value */
#define APP_DIS_MANUFACTURER_NAME       ("Peripheral_HeartRate")
#define APP_DIS_MANUFACTURER_NAME_LEN   (20)

/* Model Number String Value */
#define APP_DIS_MODEL_NB_STR            ("RW-BLE-1.0")
#define APP_DIS_MODEL_NB_STR_LEN        (10)

/* Serial Number */
#define APP_DIS_SERIAL_NB_STR           ("1.0.0.0-LE")
#define APP_DIS_SERIAL_NB_STR_LEN       (10)

/* Firmware Revision */
#define APP_DIS_FIRM_REV_STR            ("6.1.2")
#define APP_DIS_FIRM_REV_STR_LEN        (5)

/* System ID Value - LSB -> MSB */
#define APP_DIS_SYSTEM_ID               ("\x12\x34\x56\xFF\xFE\x9A\xBC\xDE")
#define APP_DIS_SYSTEM_ID_LEN           (8)

/* Hardware Revision String */
#define APP_DIS_HARD_REV_STR            ("1.0.0")
#define APP_DIS_HARD_REV_STR_LEN        (5)

/* Software Revision String */
#define APP_DIS_SW_REV_STR              ("6.3.0")
#define APP_DIS_SW_REV_STR_LEN          (5)

/* IEEE */
#define APP_DIS_IEEE                    ("\xFF\xEE\xDD\xCC\xBB\xAA")
#define APP_DIS_IEEE_LEN                (6)

/**
 * PNP ID Value - LSB -> MSB
 *      Vendor ID Source : 0x02 (USB Implementer�s Forum assigned Vendor ID value)
 *      Vendor ID : 0x1057      (ON Semiconductor)
 *      Product ID : 0x0040
 *      Product Version : 0x0300
 */

#define APP_DIS_PNP_ID               ("\x02\x57\x10\x40\x00\x00\x03")
#define APP_DIS_PNP_ID_LEN           (7)
#define APP_DIS_FEATURES             (DIS_ALL_FEAT_SUP)

/* Application-provided IRK */
#define APP_IRK                         { 0x01, 0x23, 0x45, 0x68, 0x78, 0x9a, \
                                          0xbc, 0xde, 0x01, 0x23, 0x45, 0x68, \
                                          0x78, 0x9a, 0xbc, 0xde }

/* Application-provided CSRK */
#define APP_CSRK                        { 0x01, 0x23, 0x45, 0x68, 0x78, 0x9a, \
                                          0xbc, 0xde, 0x01, 0x23, 0x45, 0x68, \
                                          0x78, 0x9a, 0xbc, 0xde }

/* Application-provided private key */
#define APP_PRIVATE_KEY                 { 0xEC, 0x89, 0x3C, 0x11, 0xBB, 0x2E, \
                                          0xEB, 0x5C, 0x80, 0x88, 0x63, 0x57, \
                                          0xCC, 0xE2, 0x05, 0x17, 0x20, 0x75, \
                                          0x5A, 0x26, 0x3E, 0x8D, 0xCF, 0x26, \
                                          0x63, 0x1D, 0x26, 0x0B, 0xCE, 0x4D, \
                                          0x9E, 0x07 }

/* Application-provided public key X */
#define APP_PUBLIC_KEY_X                { 0x56, 0x09, 0x79, 0x1D, 0x5A, 0x5F, \
                                          0x4A, 0x5C, 0xFE, 0x89, 0x56, 0xEC, \
                                          0xE6, 0xF7, 0x92, 0x21, 0xAC, 0x93, \
                                          0x99, 0x10, 0x51, 0x82, 0xF4, 0xDD, \
                                          0x84, 0x07, 0x50, 0x99, 0xE7, 0xC2, \
                                          0xF1, 0xC8 }

/* Application-provided public key Y */
#define APP_PUBLIC_KEY_Y                { 0x40, 0x84, 0xB4, 0xA6, 0x08, 0x67, \
                                          0xFD, 0xAC, 0x81, 0x5D, 0xB0, 0x41, \
                                          0x27, 0x75, 0x9B, 0xA7, 0x92, 0x57, \
                                          0x0C, 0x44, 0xB1, 0x57, 0x7C, 0x76, \
                                          0x5B, 0x56, 0xF0, 0xBA, 0x03, 0xF4, \
                                          0xAA, 0x67 }

/* PEriod that only clients that were previously bonded can connect */
#define APP_WHITELIST_PERIOD_S          10


void vInitXTAL(void);
void vInitPWM(void);
void vInitButton(void);
void vInitADC(void);



void DIO0_IRQHandler(void);
void ADC_BATMON_IRQHandler(void);
void vInitBLE(void);
void APP_InitDevInfo(void);
void APP_InitDevInfo(void);
                                          
    void APP_GAPM_GATTM_Handler(ke_msg_id_t const msg_id, void const *param,
                            ke_task_id_t const dest_id,
                            ke_task_id_t const src_id);
    void APP_GAPM_GATTM_Handler(ke_msg_id_t const msg_id, void const *param,
                            ke_task_id_t const dest_id,
                            ke_task_id_t const src_id);
    void APP_GAPC_Handler(ke_msg_id_t const msg_id, void const *param,
                            ke_task_id_t const dest_id,
                            ke_task_id_t const src_id);
    void APP_LED_Timeout_Handler(ke_msg_id_t const msg_id, void const *param,
                            ke_task_id_t const dest_id,
                            ke_task_id_t const src_id);
    void APP_WhitelistTimerHandler(ke_msg_id_t const msg_id, void const *param,
                            ke_task_id_t const dest_id,
                            ke_task_id_t const src_id);

                                          

extern int GAPM_CmpEvt(ke_msg_id_t const msgid,
                       struct gapm_cmp_evt const *param,
                       ke_task_id_t const dest_id,
                       ke_task_id_t const src_id);

extern int GAPM_PubKeyInd(ke_msg_id_t const msg_id,
                          struct gapm_pub_key_ind const *param,
                          ke_task_id_t const dest_id,
                          ke_task_id_t const src_id);
