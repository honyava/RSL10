#include "main.h"


void DIO0_IRQHandler(void)
{
  if(PWM_CFG->PWM_HIGH_BYTE > 254)
    PWM_CFG->PWM_HIGH_BYTE = 0;
  else
    PWM_CFG->PWM_HIGH_BYTE += 10;
}

/* Configuration pre-set in app_config.c */
extern struct gapm_set_dev_config_cmd  devConfigCmd;
extern union  gapc_dev_info_val* getDevInfoCfm[];

/* ----------------------------------------------------------------------------
 * Function      : void APP_WhitelistTimerHandler(
 *                                     ke_msg_id_t const msg_id,
                                       void const *param,
                                       ke_task_id_t const dest_id,
                                       ke_task_id_t const src_id)
 * ----------------------------------------------------------------------------
 * Description   : Handle the APP_BATT_LEVEL_LOW event triggered by the
 * Inputs        : - msg_id     - Kernel message ID number
 *                 - param      - Message parameter
 *                 - dest_id    - Destination task ID number
 *                 - src_id     - Source task ID number
 * Outputs       : None
 * Assumptions   : None
 * ------------------------------------------------------------------------- */
void APP_WhitelistTimerHandler(ke_msg_id_t const msg_id,
                                   void const *param,
                                   ke_task_id_t const dest_id,
                                   ke_task_id_t const src_id)
{
    switch (msg_id)
    {
        case APP_TIMEOUT_WHITELIST:
        {
            //("\n\r APP_TIMEOUT_WHITELIST \n\r");
            APP_WhitelistSetInactive();
        }
        break;
    }
}


/* ----------------------------------------------------------------------------
 * Function      : void APP_GAPM_CMP_EVT_Handler(uint8_t conidx,
 *                  const struct gapm_cmp_evt *p)
 * ----------------------------------------------------------------------------
 * Description   : Handle the GAPM_CMP_EVT, setting the device config and
 *                  handling the whitelist
 * Inputs        : - conidx     - Client index
 *                 - param      - Data from the kernel API
 * Outputs       : None
 * Assumptions   : None
 * ------------------------------------------------------------------------- */
void APP_GAPM_CMP_EVT_Handler(uint8_t conidx, const struct gapm_cmp_evt *param)
{
    /* Reset completed. Apply device configuration. */
    if(param->operation == GAPM_RESET)
    {
        GAPM_SetDevConfigCmd(&devConfigCmd);
        /* Trigger a GAPM_CMP_EVT / GAPM_SET_DEV_CONFIG when finished. */
    }
    else if((param->operation == GAPM_RESOLV_ADDR) && /* IRK not found for address */
            (param->status == GAP_ERR_NOT_FOUND))
    {
        if( APP_WhitelistIsActive() == true )
        {
            //("\n\r Whitelist active, IRK not found for address, REJECTING \n\r");
            GAPC_DisconnectCmd(conidx, CO_ERROR_REMOTE_USER_TERM_CON);
        }
        else
        {
            struct gapc_connection_cfm cfm;
            APP_SetConnectionCfmParams(conidx, &cfm);
            GAPC_ConnectionCfm(conidx, &cfm); /* Confirm connection without LTK. */
            //("\n\rGAPM_CMP_EVT / GAPM_RESOLV_ADDR. conidx=%d Status = NOT FOUND", conidx);
        }
    }
}

/* ----------------------------------------------------------------------------
 * Function      : void APP_GAPM_ADDR_SOLVED_IND_Handler(uint8_t conidx)
 * ----------------------------------------------------------------------------
 * Description   : Handle the GAPM_ADDR_SOLVED_IND message, triggered by a
 *                  successfully resolving a private address
 * Inputs        : - conidx     - Client index
 * Outputs       : None
 * Assumptions   : None
 * ------------------------------------------------------------------------- */
void APP_GAPM_ADDR_SOLVED_IND_Handler(uint8_t conidx)
{
    struct gapc_connection_cfm cfm;
    APP_SetConnectionCfmParams(conidx, &cfm);
    GAPC_ConnectionCfm(conidx, &cfm); /* Send connection confirmation with LTK */
    //("\n\r Whitelist inactive or previously bonded, ACCEPTING \n\r");
}

/* ----------------------------------------------------------------------------
 * Function      : void APP_GAPM_GATTM_Handler(ke_msg_id_t const msg_id,
 *                                     void const *param,
 *                                     ke_task_id_t const dest_id,
 *                                     ke_task_id_t const src_id)
 * ----------------------------------------------------------------------------
 * Description   : Handle GAPM/GATTM messages that need application action
 * Inputs        : - msg_id     - Kernel message ID number
 *                 - param      - Message parameter
 *                 - dest_id    - Destination task ID number
 *                 - src_id     - Source task ID number
 * Outputs       : None
 * Assumptions   : None
 * ------------------------------------------------------------------------- */
void APP_GAPM_GATTM_Handler(ke_msg_id_t const msg_id, void const *param,
                            ke_task_id_t const dest_id,
                            ke_task_id_t const src_id)
{
	//("\n\rAPP_GAPM_GATTM_Handler: %d\r\n", msg_id);

	uint8_t conidx = KE_IDX_GET(dest_id);

    switch(msg_id)
    {
        case GAPM_CMP_EVT:
        {
            APP_GAPM_CMP_EVT_Handler(conidx, (const struct gapm_cmp_evt*) param);
        }
        break;

        case GAPM_ADDR_SOLVED_IND: /* Private address resolution was successful */
        {
            //("\n\rGAPM_ADDR_SOLVED_IND");
            APP_GAPM_ADDR_SOLVED_IND_Handler(conidx);
        }
        break;

        case GATTM_ADD_SVC_RSP:
        case GAPM_PROFILE_ADDED_IND:
        {
            /* If all expected profiles/services have been added */
            if(GAPM_GetProfileAddedCount() == APP_NUM_STD_PRF &&
               GATTM_GetServiceAddedCount() == APP_NUM_CUSTOM_SVC)
            {
                /* Start advertising with the default configuration.
                 * The advertisement period will change after 30 s. */
                HRPS_StartAdvertisement();

                /* Start the LED periodic timer. LED blinks according to the
                 * number of connected peers. See APP_LED_Timeout_Handler. */
                ke_timer_set(APP_LED_TIMEOUT, TASK_APP, TIMER_SETTING_MS(200));

                /* Setting kernel timer to call APP_BASS_ReadBatteryLevel_Handler
                 *  after 1 s */
                ke_timer_set(APP_BATT_LEVEL_READ_TIMEOUT, TASK_APP, TIMER_SETTING_S(1));

                /* Start the whitelist timer, this will disable the whitelist
                 * mechanism after APP_WHITELIST_PERIOD_S, i.e. the server will
                 * accept connections from any client after this period. */
                ke_timer_set(APP_TIMEOUT_WHITELIST, TASK_APP,
                        TIMER_SETTING_S(APP_WHITELIST_PERIOD_S));
            }
        }
        break;
    }
}

void APP_GAPC_ConnectionReqInd(uint8_t conidx,
        const struct gapc_connection_req_ind* param)
{
    //("\n\rGAPC_CONNECTION_REQ_IND: ");
    //("\n\r  ADDR: ");
    for (uint8_t i = 0;i < GAP_BD_ADDR_LEN; i++)
        //("%d ", param->peer_addr.addr[i]);
    //("\n\r  ADDR_TYPE: %d", param->peer_addr_type);

#if CFG_BOND_LIST_IN_NVR2
    if(GAP_IsAddrPrivateResolvable(param->peer_addr.addr, param->peer_addr_type) &&
       BondList_Size() > 0)
    {
        //("\n\r    Starting GAPM_ResolvAddrCmd\n\r");
        GAPM_ResolvAddrCmd(conidx, param->peer_addr.addr, 0, NULL);
    }
    else
#endif
    {
        /* While the whitelist is active, only previously bonded devices can
         * connect */
        if( GAPC_IsBonded(conidx) == false && APP_WhitelistIsActive() == true )
        {
            //("\n\r Whitelist active, not previously bonded, REJECTING \n\r");
            GAPC_DisconnectCmd(conidx, CO_ERROR_REMOTE_USER_TERM_CON);
        }
        else
        {
            struct gapc_connection_cfm cfm;
            APP_SetConnectionCfmParams(conidx, &cfm);
            GAPC_ConnectionCfm(conidx, &cfm); /* Send connection confirmation */
            //("\n\r Whitelist inactive or previously bonded, ACCEPTING \n\r");
        }
    }

    /* If not yet connected to all peers, keep advertising */
    if((GAPC_GetConnectionCount() < APP_NB_PEERS))
    {
        HRPS_StartAdvertisement();
    }
}

/* ----------------------------------------------------------------------------
 * Function      : void APP_GAPC_MsgHandler(ke_msg_id_t const msg_id,
 *                                     void const *param,
 *                                     ke_task_id_t const dest_id,
 *                                     ke_task_id_t const src_id)
 * ----------------------------------------------------------------------------
 * Description   : Handle GAPC messages that need application action
 * Inputs        : - msg_id     - Kernel message ID number
 *                 - param      - Message parameter
 *                 - dest_id    - Destination task ID number
 *                 - src_id     - Source task ID number
 * Outputs       : None
 * Assumptions   : None
 * ------------------------------------------------------------------------- */
void APP_GAPC_Handler(ke_msg_id_t const msg_id, void const *param,
                      ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
    uint8_t conidx = KE_IDX_GET(src_id);

    switch(msg_id)
    {
        case GAPC_CONNECTION_REQ_IND:
            APP_GAPC_ConnectionReqInd(conidx, (struct gapc_connection_req_ind*) param);
            break;

        case GAPC_GET_DEV_INFO_REQ_IND:
        {
            const struct gapc_get_dev_info_req_ind* p = param;
            GAPC_GetDevInfoCfm(conidx, p->req, getDevInfoCfm[p->req]);
        }
        break;

        case GAPC_SET_DEV_INFO_REQ_IND:
        {
        	const struct gapc_set_dev_info_req_ind* p = param;
        	//("\n\rGAPC_SET_DEV_INFO_REQ_IND\r\n");
        	GAPC_SetDevInfoCfm(conidx, p, getDevInfoCfm[p->req], APP_DEVICE_NAME_MAXLEN);
        }
        break;

        case GAPC_PARAM_UPDATE_REQ_IND:
        {
            GAPC_ParamUpdateCfm(conidx, true, 0xFFFF, 0xFFFF);
            //("\n\rGAPC_PARAM_UPDATE_REQ_IND");
        }
        break;

#if CFG_BOND_LIST_IN_NVR2
        case GAPC_ENCRYPT_REQ_IND:
        {
            const struct gapc_encrypt_req_ind* p = param;
            /* Accept request if bond information is valid & EDIV/RAND match */
            bool found = (GAPC_IsBonded(conidx) &&
                          p->ediv == GAPC_GetBondInfo(conidx)->EDIV &&
                          !memcmp(p->rand_nb.nb, GAPC_GetBondInfo(conidx)->RAND, GAP_RAND_NB_LEN));

            //("\n\rGAPC_ENCRYPT_REQ_IND: bond information %s", (found ? "FOUND" : "NOT FOUND"));
            //("\n\r    GAPC_isBonded=%d GAPC_EDIV=%d  GAPC_rand= %d %d %d %d   ",
             //       GAPC_IsBonded(conidx), GAPC_GetBondInfo(conidx)->EDIV,
             //       GAPC_GetBondInfo(conidx)->RAND[0], GAPC_GetBondInfo(conidx)->RAND[1],
             //       GAPC_GetBondInfo(conidx)->RAND[2], GAPC_GetBondInfo(conidx)->RAND[3]);
            //("\n\r    p_EDIV=%d  p_rand= %d %d %d %d   ", p->ediv,
                   //p->rand_nb.nb[0], p->rand_nb.nb[1], p->rand_nb.nb[2], p->rand_nb.nb[3]);

            GAPC_EncryptCfm(conidx, found, GAPC_GetBondInfo(conidx)->LTK);
        }
        break;

        case GAPC_ENCRYPT_IND:
        {
            //("\n\rGAPC_ENCRYPT_IND: Link encryption is ON");
        }
        break;

        case GAPC_BOND_REQ_IND:
        {
            const struct gapc_bond_req_ind* p = param;
            switch (p->request)
            {
                case GAPC_PAIRING_REQ:
                {
                    bool accept = BondList_Size() < APP_BONDLIST_SIZE;
                    union gapc_bond_cfm_data pairingRsp =
                    {
                        .pairing_feat =
                        {
                            .iocap = GAP_IO_CAP_NO_INPUT_NO_OUTPUT,
                            .oob = GAP_OOB_AUTH_DATA_NOT_PRESENT,
                            .key_size = KEY_LEN,
                            .ikey_dist = (GAP_KDIST_IDKEY | GAP_KDIST_SIGNKEY),
                            .rkey_dist = (GAP_KDIST_ENCKEY | GAP_KDIST_IDKEY | GAP_KDIST_SIGNKEY),
                        }
                    };
#ifdef SECURE_CONNECTION
                    if (p->data.auth_req & GAP_AUTH_SEC_CON)
                    {
                        pairingRsp.pairing_feat.auth = GAP_AUTH_REQ_SEC_CON_BOND;
                        pairingRsp.pairing_feat.sec_req = GAP_SEC1_SEC_CON_PAIR_ENC;
                    }
                    else
#endif    /* ifdef SECURE_CONNECTION */
                    {
                        pairingRsp.pairing_feat.auth = GAP_AUTH_REQ_NO_MITM_BOND;
                        pairingRsp.pairing_feat.sec_req = GAP_NO_SEC;
                    }
                    //("\n\rGAPC_BOND_REQ_IND / GAPC_PAIRING_REQ: accept = %d conidx=%d", accept, conidx);
                    GAPC_BondCfm(conidx, GAPC_PAIRING_RSP, accept, &pairingRsp);
                }
                break;

                case GAPC_LTK_EXCH: /* Prepare and send random LTK (legacy only) */
                {
                    //("\n\rGAPC_BOND_REQ_IND / GAPC_LTK_EXCH");
                    union gapc_bond_cfm_data ltkExch;
                    ltkExch.ltk.ediv = co_rand_hword();
                    for(uint8_t i = 0, i2 = GAP_RAND_NB_LEN; i < GAP_RAND_NB_LEN; i++, i2++)
                    {
                        ltkExch.ltk.randnb.nb[i] = co_rand_byte();
                        ltkExch.ltk.ltk.key[i] = co_rand_byte();
                        ltkExch.ltk.ltk.key[i2] = co_rand_byte();
                    }
                    GAPC_BondCfm(conidx, GAPC_LTK_EXCH, true, &ltkExch); /* Send confirmation */
                }
                break;

                case GAPC_TK_EXCH: /* Prepare and send TK */
                {
                    //("\n\rGAPC_BOND_REQ_IND / GAPC_TK_EXCH");
                    /* IO Capabilities are set to GAP_IO_CAP_NO_INPUT_NO_OUTPUT in this application.
                     * Therefore TK exchange is NOT performed. It is always set to 0 (Just Works algorithm). */
                }
                break;

                case GAPC_IRK_EXCH:
                {
                    //("\n\rGAPC_BOND_REQ_IND / GAPC_IRK_EXCH");
                    union gapc_bond_cfm_data irkExch;
                    memcpy(irkExch.irk.addr.addr.addr, GAPM_GetDeviceConfig()->addr.addr, GAP_BD_ADDR_LEN);
                    irkExch.irk.addr.addr_type = GAPM_GetDeviceConfig()->addr_type;
                    memcpy(irkExch.irk.irk.key, GAPM_GetDeviceConfig()->irk.key, GAP_KEY_LEN);
                    GAPC_BondCfm(conidx, GAPC_IRK_EXCH, true, &irkExch); /* Send confirmation */
                }
                break;

                case GAPC_CSRK_EXCH:
                {
                    //("\n\rGAPC_BOND_REQ_IND / GAPC_CSRK_EXCH");
                    union gapc_bond_cfm_data csrkExch;
                    Device_Param_Read(PARAM_ID_CSRK, csrkExch.csrk.key);
                    GAPC_BondCfm(conidx, GAPC_CSRK_EXCH, true, &csrkExch); /* Send confirmation */
                }
                break;
            }
        }
        break;
#endif /* CFG_BOND_LIST_IN_NVR2 */
    }
}

/* ----------------------------------------------------------------------------
 * Function      : void APP_LED_Timeout_Handler(ke_msg_idd_t const msg_id,
 *                                              void const *param,
 *                                              ke_task_id_t const dest_id,
 *                                              ke_task_id_t const src_id)
 * ----------------------------------------------------------------------------
 * Description   : Control GPIO "LED_DIO_NUM" behavior using a timer.
 *                 Possible LED behaviors:
 *                     - If the device is advertising but it has not connected
 *                       to any peer: the LED blinks every 200 ms.
 *                     - If the device is advertising and it is connecting to
 *                       fewer than BLE_CONNECTION_MAX peers: the LED blinks
 *                       every 2 seconds according to the number of connected
 *                       peers (i.e., blinks once if one peer is connected,
 *                       twice if two peers are connected, etc.).
 *                     - If the device is connected to BLE_CONNECTION_MAX peers
 *                       the LED is steady on.
 * Inputs        : - msg_id     - Kernel message ID number
 *                 - param      - Message parameter (unused)
 *                 - dest_id    - Destination task ID number
 *                 - src_id     - Source task ID number
 * Outputs       : None
 * Assumptions   : None
 * ------------------------------------------------------------------------- */
void APP_LED_Timeout_Handler(ke_msg_id_t const msg_id, void const *param,
                             ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
    static uint8_t toggle_cnt = 0;
    uint8_t connectionCount = GAPC_GetConnectionCount();

    /* Blink LED according to the number of connections */
    switch (connectionCount)
    {
        case 0:
        {
            ke_timer_set(APP_LED_TIMEOUT, TASK_APP, TIMER_SETTING_MS(200));
            Sys_GPIO_Toggle(LED_DIO_NUM); /* Toggle LED_DIO_NUM every 200ms */
            toggle_cnt = 0;
        }
        break;

        case APP_NB_PEERS:
        {
            ke_timer_set(APP_LED_TIMEOUT, TASK_APP, TIMER_SETTING_MS(200));
            Sys_GPIO_Set_High(LED_DIO_NUM); /* LED_DIO_NUM steady high */
            toggle_cnt = 0;
        }
        break;

        default: /* connectionCount is between 1 and APP_NB_PEERS (exclusive) */
        {
            if (toggle_cnt >= connectionCount * 2)
            {
                toggle_cnt = 0;
                ke_timer_set(APP_LED_TIMEOUT, TASK_APP, TIMER_SETTING_S(2)); /* Schedule timer for a long 2s break */
                Sys_GPIO_Set_High(LED_DIO_NUM); /* LED_DIO_NUM steady high until next 2s blinking period */
            }
            else
            {
                toggle_cnt++;
                Sys_GPIO_Toggle(LED_DIO_NUM);
                ke_timer_set(APP_LED_TIMEOUT, TASK_APP, TIMER_SETTING_MS(200));
            }
        }
    }
}

