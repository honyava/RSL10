/* ----------------------------------------------------------------------------
 * Copyright (c) 2015-2018 Semiconductor Components Industries, LLC (d/b/a
 * ON Semiconductor), All Rights Reserved
 *
 * Copyright (C) RivieraWaves 2009-2016
 *
 * This module is derived in part from example code provided by RivieraWaves
 * and as such the underlying code is the property of RivieraWaves [a member
 * of the CEVA, Inc. group of companies], together with additional code which
 * is the property of ON Semiconductor. The code (in whole or any part) may not
 * be redistributed in any form without prior written permission from
 * ON Semiconductor.
 *
 * The terms of use and warranty for this code are covered by contractual
 * agreements between ON Semiconductor and the licensee.
 *
 * This is Reusable Code.
 *
 * ----------------------------------------------------------------------------
 * ble_hrps.h
 * - Bluetooth heart rate server service header
 * ------------------------------------------------------------------------- */

#ifndef BLE_PROFILES_INCLUDE_BLE_HRPS_H_
#define BLE_PROFILES_INCLUDE_BLE_HRPS_H_


/* ----------------------------------------------------------------------------
 * If building with a C++ compiler, make all of the definitions in this header
 * have a C binding.
 * ------------------------------------------------------------------------- */
#ifdef __cplusplus
extern "C"
{
#endif    /* ifdef __cplusplus */

/* ----------------------------------------------------------------------------
 * Include files
 * --------------------------------------------------------------------------*/
#include <hrps.h>
#include <ke_timer.h>
/* ----------------------------------------------------------------------------
 * Defines
 * --------------------------------------------------------------------------*/

/** Advertising minimum interval for HRP reduced power mode - 1s */
#define HRPS_ADV_INT_MIN_HRP_REDUCED_POWER_MS              1000

/** Advertising maximum interval for HRP reduced power mode - 2.5s */
#define HRPS_ADV_INT_MAX_HRP_REDUCED_POWER_MS              2500

/** Advertising minimum interval for HRP fast connection mode - 20ms */
#define HRPS_ADV_INT_MIN_HRP_FAST_CONNECTION_MS            20

/** Advertising maximum interval for HRP reduced power mode - 30ms */
#define HRPS_ADV_INT_MAX_HRP_FAST_CONNECTION_MS            30

/** If the server should send notifications after a client connected */
#define HRPS_DEFAULT_NOTIFICATION       0x01

/** Fast connect/energy saving timeout time in seconds. */
#define HRPS_FAST_CONN_TIMEOUT_S     30

/** Skin contact timeout in seconds.
 * The peripheral will send a disconnect request after this time. */
#define HRPS_SKIN_CONTACT_TIMEOUT_S     15

/** Heart rate inactivity timeout.
 * The peripheral will send a disconnect request after this time. */
#define HRPS_HR_INACTIVITY_TIMEOUT_S    15

/** Heart rate UUID */
#define HRPS_SERVICE_UUID               {0x0D, 0x18}

/* ----------------------------------------------------------------------------
 * Global variables and types
 * --------------------------------------------------------------------------*/

/** Structure used to receive the measured values from the application */
struct HRPS_Measure_t
{
    /** measured values */
    struct hrs_hr_meas measurement;
    /** skin contact sensor value */
    uint8_t skin_contact;
};

typedef struct
{
    /********** Heart Rate **********/

    /** Heart rate notification configuration */
    uint16_t hrp_ntf_cfg[BLE_CONNECTION_MAX];

    /** Flag for reduced power mode */
    uint8_t reduced_power_mode;

    /** Flag for connection termination due to skin contact loss (for 15s) */
    uint8_t send_connection_termination_skin_contact;

    /** Heart rate value storing previous value and current value to compare and check
     * changing heart rate values to detect heart rate activity and terminate connection if
     * here is no heart rate activity upon timer expiry */
    uint16_t hrp_heartrate_prev[2];

    /** Temp variable to store previous energy expended value to be displayed without
     * updating to the current value (update for energy expended value is every 10s) */
    uint16_t hrp_energy_expended_prev;

    /** Index for hrp_heartrate_prev array */
    uint8_t hrp_heartrate_cnt;

    /** Flag for connection termination due to lack of heart rate activity (for 15s) */
    uint8_t send_connection_termination_activity;

    /** The flag that indicates that service has been added */
    bool serviceAdded;

    /** The flag that indicates that service has been enabled */
    bool enable;

    /** Database configuration */
    uint8_t features;
    /** Body Sensor Location */
    uint8_t body_sensor_loc;

    /** Time between notifications */
    uint16_t notification_timeout;

    /** Advertisement command configuration */
    struct gapm_start_advertise_cmd advertiseCmd;

    /** Pointer to an application function that returns the heart rate measurement */
    const struct HRPS_Measure_t*(*readHeartRateCallback)(void);

    /** Pointer to an application function that resets the energy expended measurement. */
    void (*energyExpendedResetCallback)(void);

} HRPS_Env_t;

enum hrps_app_msg_id
{
    /** Notification timer */
    HRPS_TIMEOUT_HEART_RATE_NTF = TASK_FIRST_MSG(TASK_ID_HRPS) + 50,
    /** Connection mode timeout */
    HRPS_TIMEOUT_ADVERTISE_MODE,
};

/* ----------------------------------------------------------------------------
 * Function prototype definitions
 * --------------------------------------------------------------------------*/
int HRPS_Initialize(uint16_t notification_timeout,
        uint8_t features,
        uint8_t body_sensor_loc,
        union gapm_adv_info *info,
        const struct HRPS_Measure_t*(*readHeartRateCallback)(void),
        void (*energyExpendedResetCallback)(void));

void HRPS_ProfileTaskAddCmd(void);

void HRPS_EnableReq(uint8_t conidx);

void HRPS_EnableRsp(struct hrps_enable_rsp const *param);

void HRPS_DisconnectInd( uint8_t conidx );

HRPS_Env_t* HRPS_GetEnv(void);

bool HRPS_IsAdded(void);

void HRPS_CfgIndNtfInd( const struct hrps_cfg_indntf_ind *param,
        uint16_t *hrp_ntf_cfg );

void HRPS_MeasSendReq( const struct hrs_hr_meas *meas_val );

bool HRPS_CheckNtfEnbl(uint16_t *hrp_ntf_cfg);

void HRPS_StartAdvertisement( void );

int HRPS_CheckAndSendNotifications(void);

void HRPS_MsgHandler(ke_msg_id_t const msg_id, void const *param,
                     ke_task_id_t const dest_id, ke_task_id_t const src_id);

/* ----------------------------------------------------------------------------
 * Close the 'extern "C"' block
 * ------------------------------------------------------------------------- */
#ifdef __cplusplus
}
#endif    /* ifdef __cplusplus */

#endif /* BLE_PROFILES_INCLUDE_BLE_HRPS_H_ */
