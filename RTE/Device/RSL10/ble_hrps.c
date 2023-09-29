/* ----------------------------------------------------------------------------
 * Copyright (c) 2018 Semiconductor Components Industries, LLC (d/b/a
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
 *  This is Reusable Code.
 *
 * ----------------------------------------------------------------------------
 * ble_hrps.c
 * - Bluetooth heart rate server service functions. This file deals with the
 *      heart rate profile communication behavior as defined on the
 *      HEART RATE PROFILE (v10r00).
 * ------------------------------------------------------------------------- */
#include <rsl10.h>
#include <ble_hrps.h>
#include <ble_gap.h>
#include <msg_handler.h>
#include <prf_utils.h>
#include <hrps.h>

/* Global variable definition */
static HRPS_Env_t hrps_env;

/* ----------------------------------------------------------------------------
 * Function      : int HRPS_Initialize(uint16_t notification_timeout,
                    uint8_t features,
                    uint8_t body_sensor_loc,
                    union gapm_adv_info *info,
                    const struct HRPS_Measure_t*(*readHeartRateCallback)(void),
                    void (*energyExpendedResetCallback)(void))
 * ----------------------------------------------------------------------------
 * Description   : Initialize device information server service environment and
 *                 configure message handlers
 * Inputs        : notification_timeout : time between notifications, in time units
 *                 features - Database features: HRPS_BODY_SENSOR_LOC_CHAR_SUP,
 *                  HRPS_ENGY_EXP_FEAT_SUP, HRPS_HR_MEAS_NTF_CFG
 *                 body_sensor_loc - Body sensor location (HRS_LOC_*)
 *                 info - Application advertisement information. Only the
 *                  advertisement data and the scan response data are copied.
 *                 readHeartRateCallback - Pointer to the measurement callback function
 *                 energyExpendedResetCallback - Pointer to the reset expended energy
 *                 callback function
 * Outputs       : 0 if the initialization was successful, < 0 otherwise
 * Assumptions   : None
 * ------------------------------------------------------------------------- */
/**
 * @brief	Initialize heart rate server service environment and configure
 *  		message handlers
 *
 * Assumptions   : None
 *
 * @param[in] notification_timeout time between notifications, in time units.
 * @param[in] features - Database features: HRPS_BODY_SENSOR_LOC_CHAR_SUP,
 *                  HRPS_ENGY_EXP_FEAT_SUP, HRPS_HR_MEAS_NTF_CFG
 * @param[in] body_sensor_loc - Body sensor location (HRS_LOC_*)
 * @param[in] info Application advertisement information. Only the
 *                  advertisement data and the scan response data are copied.
 * @param[in] readHeartRateCallback Pointer to the measurement callback function
 * @param[in] energyExpendedResetCallback Pointer to the reset expended energy callback function
 * @return	0 if the initialization was successful, < 0 otherwise
 */
int HRPS_Initialize(uint16_t notification_timeout,
        uint8_t features,
        uint8_t body_sensor_loc,
        union gapm_adv_info *info,
        const struct HRPS_Measure_t*(*readHeartRateCallback)(void),
        void (*energyExpendedResetCallback)(void))
{
    /* Reset the application manager environment */
    memset(&hrps_env, 0, sizeof(HRPS_Env_t));

    hrps_env.readHeartRateCallback = readHeartRateCallback;

    hrps_env.energyExpendedResetCallback = energyExpendedResetCallback;

    hrps_env.serviceAdded = false;

    hrps_env.enable = false;

    hrps_env.features = features;

    hrps_env.body_sensor_loc = body_sensor_loc;

    hrps_env.notification_timeout = notification_timeout;

    hrps_env.advertiseCmd.op.code = GAPM_ADV_UNDIRECT;

    hrps_env.advertiseCmd.op.addr_src = GAPM_STATIC_ADDR;

    hrps_env.advertiseCmd.op.state = 0;

    // Start in fast advertisement mode
    hrps_env.advertiseCmd.intv_min =
            MS_TO_BLE_TICKS(HRPS_ADV_INT_MIN_HRP_FAST_CONNECTION_MS);
    hrps_env.advertiseCmd.intv_max =
            MS_TO_BLE_TICKS(HRPS_ADV_INT_MAX_HRP_FAST_CONNECTION_MS);
    hrps_env.advertiseCmd.channel_map = GAPM_DEFAULT_ADV_CHMAP;

    memcpy(&hrps_env.advertiseCmd.info, info,
            sizeof(union gapm_adv_info));

    hrps_env.advertiseCmd.info.host.mode = GAP_GEN_DISCOVERABLE;

    hrps_env.advertiseCmd.info.host.adv_filt_policy =
            ADV_ALLOW_SCAN_ANY_CON_ANY;

    /*
     * Add HRPS message handlers
     */
    /* General handler */
    MsgHandler_Add(TASK_ID_HRPS, HRPS_MsgHandler);
    /* GAP event (operation == GAPM_SET_DEV_CONFIG) needed to add the service */
    MsgHandler_Add(GAPM_CMP_EVT, HRPS_MsgHandler);
    /* Confirmation that the service has been added */
    MsgHandler_Add(GAPM_PROFILE_ADDED_IND, HRPS_MsgHandler);
    /* Starts service after this */
    MsgHandler_Add(GAPC_CONNECTION_REQ_IND, HRPS_MsgHandler);
    /* Manages notification config */
    MsgHandler_Add(GAPC_DISCONNECT_IND, HRPS_MsgHandler);

    return 0;
}

/* ----------------------------------------------------------------------------
 * Function      : void HRPS_ProfileTaskAddCmd(void)
 * ----------------------------------------------------------------------------
 * Description   : Send request to add the device information service in kernel and DB
 * Inputs        : None
 * Outputs       : None
 * Assumptions   : None
 * ------------------------------------------------------------------------- */
/**
 * @brief Send request to add the device information service in kernel and DB
 *
 * <a href="https://www.bluetooth.com/specifications/gatt/viewer?attributeXmlFile=org.bluetooth.service.heart_rate.xml&u=org.bluetooth.service.heart_rate.xml">Heart Rate Service</a>
 */
void HRPS_ProfileTaskAddCmd(void)
{
    struct hrps_db_cfg db_cfg;

    /* Configure HRPS database parameters  */
    db_cfg.features = hrps_env.features;

    db_cfg.body_sensor_loc = hrps_env.body_sensor_loc;

    /* Add HRPS profile task to the stack. A GAPM_PROFILE_ADDED_IND event
     * will be generated when finished. */
    GAPM_ProfileTaskAddCmd(PERM(SVC_AUTH, DISABLE), TASK_ID_HRPS, TASK_APP, 0,
            (uint8_t*) &db_cfg, sizeof(struct hrps_db_cfg));
}

/* ----------------------------------------------------------------------------
 * Function      : void HRPS_EnableReq(uint8_t conidx)
 * ----------------------------------------------------------------------------
 * Description   : Send request for enabling heart rate profile to hrp server
 * Inputs        : conidx      - connection index
 * Outputs       : None
 * Assumptions   : None
 * ------------------------------------------------------------------------- */
/**
 * @brief Send request for enabling heart rate profile to hrp server
 *
 * Assumptions   : None
 *
 * @param[in] conidx The connection index
 */
void HRPS_EnableReq(uint8_t conidx)
{
    struct hrps_enable_req *req;
    struct hrps_env_tag *env;
    env = PRF_ENV_GET(HRPS, hrps);

    /* Restores the original notification state */
    hrps_env.hrp_ntf_cfg[conidx] = HRPS_DEFAULT_NOTIFICATION;

    req = KE_MSG_ALLOC(HRPS_ENABLE_REQ,
            prf_src_task_get(&(env->prf_env), 0),
            KE_BUILD_ID(TASK_APP, conidx), hrps_enable_req);

    req->conidx = conidx;
    req->hr_meas_ntf = hrps_env.hrp_ntf_cfg[conidx];

    ke_msg_send(req);
}

/* ----------------------------------------------------------------------------
 * Function      : int HRPS_EnableRsp(struct hrps_enable_rsp const *param)
 * ----------------------------------------------------------------------------
 * Description   : Receive heart rate profile service enable from hrp task
 * Inputs        : - param      - Message parameters in format of
 *                                struct hrps_enable_rsp
 * Outputs       : None
 * Assumptions   : None
 * ------------------------------------------------------------------------- */
/**
 * @brief Receive heart rate profile service enable from hrp task
 *
 * Assumptions   : None
 *
 * @param[in] param Message parameters in format of struct hrps_enable_rsp
 */
void HRPS_EnableRsp(struct hrps_enable_rsp const *param)
{
    if( (param->status == GAP_ERR_NO_ERROR)
            || (param->status == PRF_ERR_REQ_DISALLOWED) ) /* Already enabled */
    {
        hrps_env.enable = true;
    }
}

/* ----------------------------------------------------------------------------
 * Function      : void HRPS_DisconnectReq( uint8_t conidx, HRPS_Env_t *support_env )
 * ----------------------------------------------------------------------------
 * Description   : Handles the GAPC_DISCONNECT_IND event.
 * Inputs        : conidx      - connection index
 * Outputs       : None
 * Assumptions   : None
 * ------------------------------------------------------------------------- */
/**
 * @brief Handles the GAPC_DISCONNECT_IND event. Now it just set the original
 * notification state and checks if the timers should be stopped.
 *
 * @param[in] conidx Disconnected client connection index
 * @param[in] support_env HRPS state structure
 */
void HRPS_DisconnectInd(uint8_t conidx)
{
    /* Clears notification on disconnect */
    hrps_env.hrp_ntf_cfg[conidx] = 0;

    /* 5.1.3 Link Loss Reconnection Procedure
     * When a connection is terminated due to link loss, a Heart Rate Sensor
     * should attempt to reconnect to the Collector by entering a GAP
     * Connectable Mode using the recommended advertising interval values shown
     * in Table 5.1. */

    /* Disable the reduced power mode after disconnection (the application will
     * switch to the fast connection mode with time-out duration defined by 
     * HRPS_FAST_CONN_TIMEOUT_S). */
    hrps_env.reduced_power_mode = 0;

    /* Start timer, fast connection mode */
    ke_timer_set(HRPS_TIMEOUT_ADVERTISE_MODE, TASK_APP,
            HRPS_FAST_CONN_TIMEOUT_S*100);

    /* White list device filtering is initially activated. */
//    whitelist_info.wl_filtering_activated = true;
    /* When the Heart Rate Sensor is disconnected by the Collector and it is
     * ready for connection (e.g., when in contact with the body or when
     * commanded by the user), the Heart Rate Sensor should reinitiate the
     * connection procedure. */

    /* Cancel the advertising and upon the reception of the operation cancel in
     * GAPM_CMP_EVT, restart the advertising */
    GAPM_CancelCmd();
}

/* ----------------------------------------------------------------------------
 * Function      : void HRPS_GetEnv(void)
 * ----------------------------------------------------------------------------
 * Description   : Return a reference to the internal environment structure.
 * Inputs        : None
 * Outputs       : Pointer to BASS_Env_t
 * Assumptions   : None
 * ------------------------------------------------------------------------- */
/**
 * @brief Return a reference to the internal environment structure.
 * @return Pointer to BASS_Env_t
 */
HRPS_Env_t* HRPS_GetEnv(void)
{
    return &hrps_env;
}

/* ----------------------------------------------------------------------------
 * Function      : bool HRPS_IsAdded(void)
 * ----------------------------------------------------------------------------
 * Description   : Return a boolean indication is the HRPS server has been
 *                  successfully added to the services database.
 * Inputs        : None
 * Outputs       : Return value: true if the service as added, false if not
 * Assumptions   : None
 * ------------------------------------------------------------------------- */
/**
 * @brief Return a boolean indication is the HRPS server has been successfully
 *          added to the services database.
 *
 * Assumptions   : None
 *
 * @return true if the service as added, false if not
 */
bool HRPS_IsAdded(void)
{
    return hrps_env.serviceAdded;
}

/* ----------------------------------------------------------------------------
 * Function      : void HRPS_CfgIndNtfInd( const struct hrps_cfg_indntf_ind const *param,
 *                      uint16_t *hrp_ntf_cfg )
 * ----------------------------------------------------------------------------
 * Description   : Receive notification config indication for heart rate profile
 * Inputs        : - param      - Message parameters in format of
 *                                struct hrps_cfg_indntf_ind
 *                 - hrp_ntf_cfg- Heart rate notification configuration
 * Outputs       : None
 * Assumptions   : None
 * ------------------------------------------------------------------------- */
/**
 * @brief Receive notification config indication for heart rate profile
 *
 * Assumptions   : None
 *
 * @param[in] param Message parameters
 * @param[in] hrp_ntf_cfg Heart rate notification configuration
 */
void HRPS_CfgIndNtfInd(const struct hrps_cfg_indntf_ind *param,
        uint16_t *hrp_ntf_cfg)
{
    if( param->conidx >= BLE_CONNECTION_MAX )
        return;

    hrp_ntf_cfg[param->conidx] = param->cfg_val;
}

/* ----------------------------------------------------------------------------
 * Function      : void HRPS_MeasSendReq(struct hrs_hr_meas meas_val)
 * ----------------------------------------------------------------------------
 * Description   : Send request for sending a heart rate measurements to hrp
 *                 server
 * Inputs        : meas_val - heart rate measurement structure
 * Outputs       : None
 * Assumptions   : None
 * ------------------------------------------------------------------------- */
/**
 * @brief Send request for sending a heart rate measurements to hrp server
 *
 * Assumptions   : None
 *
 * @param[in] meas_val heart rate measurement structure
 */
void HRPS_MeasSendReq(const struct hrs_hr_meas *meas_val)
{
    struct hrps_meas_send_req *req;
    struct hrps_env_tag *env;
    env = PRF_ENV_GET(HRPS, hrps);

    req = KE_MSG_ALLOC(HRPS_MEAS_SEND_REQ,
            prf_src_task_get(&(env->prf_env), 0), TASK_APP,
            hrps_meas_send_req);

    /* Send the measurement */
    req->meas_val = *meas_val;

    ke_msg_send(req);
}

/* ----------------------------------------------------------------------------
 * Function      : bool HRPS_CheckNtfEnbl()
 * ----------------------------------------------------------------------------
 * Description   : Check if HRPS notifications are enabled for any peer central
 *                 device.
 * Inputs        : None
 * Outputs       : None
 * Assumptions   : None
 * ------------------------------------------------------------------------- */
bool HRPS_CheckNtfEnbl(uint16_t *hrp_ntf_cfg)
{
    if( GAPC_GetConnectionCount() <= 0 )
        return false;

    for( signed int i = 0; i < BLE_CONNECTION_MAX; i++ )
    {
        if( hrp_ntf_cfg[i] )
        {
            return true;
        }
    }
    return false;
}

/* ----------------------------------------------------------------------------
 * Function      : void HRPS_StartAdvertisement( HRPS_Env_t *support_env,
 *                      struct gapm_start_advertise_cmd *adv_cmd )
 * ----------------------------------------------------------------------------
 * Description   : Starts advertisement considering the current power mode
 * Inputs        : None
 * Outputs       : None
 * Assumptions   : None
 * ------------------------------------------------------------------------- */
/**
 * @brief Starts advertisement considering the current power mode
 *
 * Assumptions   : None
 *
 */
void HRPS_StartAdvertisement(void)
{
    /* Advertising for reduced power mode */
    if( hrps_env.reduced_power_mode != 0 )
    {
        hrps_env.advertiseCmd.intv_min =
                MS_TO_BLE_TICKS(HRPS_ADV_INT_MIN_HRP_REDUCED_POWER_MS);
        hrps_env.advertiseCmd.intv_max =
                MS_TO_BLE_TICKS(HRPS_ADV_INT_MAX_HRP_REDUCED_POWER_MS);
    }
    /* Advertising for fast connection mode */
    else
    {
        hrps_env.advertiseCmd.intv_min =
                MS_TO_BLE_TICKS(HRPS_ADV_INT_MIN_HRP_FAST_CONNECTION_MS);
        hrps_env.advertiseCmd.intv_max =
                MS_TO_BLE_TICKS(HRPS_ADV_INT_MAX_HRP_FAST_CONNECTION_MS);
    }

    GAPM_StartAdvertiseCmd(&hrps_env.advertiseCmd); /* Start advertising */
}

/* ----------------------------------------------------------------------------
 * Function      : int HRPS_CheckAndSendNotifications(HRPS_Env_t *support_env)
 * ----------------------------------------------------------------------------
 * Description   : Sends the heart rate measurement notifications if necessary
 * Inputs        : HRPS_Env_t support_env Heart rate application state environment
 * Outputs       : return value - 0 if successful, < 0 otherwise
 * Assumptions   : None.
 * ------------------------------------------------------------------------- */
/**
 * @brief Sends the heart rate measurement notifications if necessary
 *
 * Assumptions   : None
 *
 * @param[in] support_env Heart rate application state environment
 * @return 0 if successful, < 0 otherwise
 */
int HRPS_CheckAndSendNotifications(void)
{
    const struct HRPS_Measure_t *meas =
            hrps_env.readHeartRateCallback();

    /* Store current heart rate value */
    hrps_env.hrp_heartrate_prev[hrps_env.hrp_heartrate_cnt] =
            meas->measurement.heart_rate;

    hrps_env.hrp_heartrate_cnt++;
    if( hrps_env.hrp_heartrate_cnt
            >= (sizeof(hrps_env.hrp_heartrate_prev)
                    / sizeof(hrps_env.hrp_heartrate_prev[0])) )
    {
        hrps_env.hrp_heartrate_cnt = 0;
    }

    /* When the Heart Rate Sensor is no longer in contact with the skin or no
     * longer senses heart rate activity for several seconds
     * (e.g., 10 to 20 seconds), the Heart Rate Sensor should perform the GAP
     * Terminate Connection procedure. */

    /* This needs to be updates either by pooling or by an interrupt event,
     * should reflect the current state of the skin contact sensor */
    if( meas->skin_contact == 0 )
        hrps_env.send_connection_termination_skin_contact++;
    else
        hrps_env.send_connection_termination_skin_contact = 0;

    /* Checks for heart rate inactivity */
    if( hrps_env.hrp_heartrate_prev[0]
            == hrps_env.hrp_heartrate_prev[1] )
        hrps_env.send_connection_termination_activity++;
    else
        hrps_env.send_connection_termination_activity = 0;

    if( hrps_env.send_connection_termination_skin_contact
            >= HRPS_SKIN_CONTACT_TIMEOUT_S
            || hrps_env.send_connection_termination_activity
                    >= HRPS_HR_INACTIVITY_TIMEOUT_S )
    {
        GAPC_DisconnectAll(CO_ERROR_REMOTE_USER_TERM_CON);
        return 0;
    }

    if( HRPS_CheckNtfEnbl(hrps_env.hrp_ntf_cfg) == true )
        HRPS_MeasSendReq(&meas->measurement);

    return 0;
}

/* ----------------------------------------------------------------------------
 * Function      : void HRPS_MsgHandler(ke_msg_id_t const msg_id,
 *                                      void const *param,
 *                                      ke_task_id_t const dest_id,
 *                                      ke_task_id_t const src_id)
 * ----------------------------------------------------------------------------
 * Description   : 	Handle all events related to the heart rate service server
 * Inputs        : - msg_id     - Kernel message ID number
 *                 - param      - Message parameter
 *                 - dest_id    - Destination task ID number
 *                 - src_id     - Source task ID number
 * Outputs       : None
 * Assumptions   : None
 * ------------------------------------------------------------------------- */
/**
 * @brief Handle all events related to the heart rate service server
 *
 * Assumptions   : None
 *
 * @param[in] msg_id Kernel message ID number
 * @param[in] param Message parameter
 * @param[in] dest_id Destination task ID number
 * @param[in] src_id Source task ID number
 */
void HRPS_MsgHandler(ke_msg_id_t const msg_id, void const *param,
        ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
    uint8_t conidx = KE_IDX_GET(src_id);

    switch (msg_id)
    {
        case GAPM_CMP_EVT:  // GAP initialized, add the Device Information
        {			        // Service will receive a GAPM_PROFILE_ADDED_IND
                            // event as response
            struct gapm_cmp_evt *gap_event = (struct gapm_cmp_evt*) param;
            switch (gap_event->operation)
            {
                case GAPM_SET_DEV_CONFIG:
                {
                    HRPS_ProfileTaskAddCmd();
                }
                break;

                case (GAPM_ADV_UNDIRECT):
                {
                    /* If advertisement was cancelled, restart it in low power mode */
                    if( gap_event->status == GAP_ERR_CANCELED )
                        HRPS_StartAdvertisement();
                }
                break;
            }
        }
        break;

        case GAPM_PROFILE_ADDED_IND:
        {
            struct gapm_profile_added_ind *ind =
                    (struct gapm_profile_added_ind*) param;
            if( ind->prf_task_id == TASK_ID_HRPS )
            {
                hrps_env.serviceAdded = true;

                ke_timer_set(HRPS_TIMEOUT_HEART_RATE_NTF, TASK_APP,
                        hrps_env.notification_timeout);

                /* Start timer, fast connection mode */
                ke_timer_set(HRPS_TIMEOUT_ADVERTISE_MODE, TASK_APP,
                        HRPS_FAST_CONN_TIMEOUT_S*100);
            }
        }
        break;

        case GAPC_CONNECTION_CFM:
        {
        }
        break;

        case GAPC_CONNECTION_REQ_IND:
        {
            HRPS_EnableReq(conidx);
        }
        break;

        case HRPS_ENABLE_RSP:
        {
            HRPS_EnableRsp((struct hrps_enable_rsp*) param);
        }
        break;

        case GAPC_DISCONNECT_IND:
        {
            HRPS_DisconnectInd(conidx);
        }
        break;

            /* Notification indication changed by client */
        case HRPS_CFG_INDNTF_IND:
        {
            HRPS_CfgIndNtfInd((struct hrps_cfg_indntf_ind*) param,
                    hrps_env.hrp_ntf_cfg);
        }
        break;

        case HRPS_ENERGY_EXP_RESET_IND:
        {
            hrps_env.energyExpendedResetCallback();
        }
        break;

        case HRPS_MEAS_SEND_RSP:
        {
        }
        break;

        case HRPS_TIMEOUT_HEART_RATE_NTF:
        {
            HRPS_CheckAndSendNotifications();

            /* Restart timer */

            ke_timer_set(HRPS_TIMEOUT_HEART_RATE_NTF, TASK_APP,
                    hrps_env.notification_timeout);
        }
        break;

        case HRPS_TIMEOUT_ADVERTISE_MODE:
        {
            /* Enable the reduced power mode since no connection has been established 
             * during time-out duration defined by HRPS_FAST_CONN_TIMEOUT_S. */
            hrps_env.reduced_power_mode = 1;

            /* Cancel the advertising and upon the reception of the operation cancel in
             * GAPM_CMP_EVT, restart the advertising (if reduced power mode flag is set)
             * for reduced power mode. */
            GAPM_CancelCmd();
        }
        break;

        default:
        break;
    }
}
