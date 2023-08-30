/*******************************************************************************
(c) 2022 Microchip Technology Inc. and its subsidiaries

Subject to your compliance with these terms, you may use Microchip software and 
any derivatives exclusively with Microchip products. You're responsible for 
complying with 3rd party license terms applicable to your use of 3rd party 
software (including open source software) that may accompany Microchip software. 
SOFTWARE IS "AS IS." NO WARRANTIES, WHETHER EXPRESS, IMPLIED OR STATUTORY, 
APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED WARRANTIES OF NON-INFRINGEMENT, 
MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT WILL MICROCHIP 
BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE, INCIDENTAL OR CONSEQUENTIAL LOSS, 
DAMAGE, COST OR EXPENSE OF ANY KIND WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER 
CAUSED, EVEN IF MICROCHIP HAS BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE 
FORESEEABLE. TO THE FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY 
ON ALL CLAIMS RELATED TO THE SOFTWARE WILL NOT EXCEED AMOUNT OF FEES, IF ANY, 
YOU PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
 ******************************************************************************/

#include <xc.h>
#include "../MCP802X_task.h"
#include "../mcp8021.h"
#include "../../system/pins.h"

#define SEC_MCP802X __attribute__((section(".MCP802X")))

extern void MCP8021_Initialize(void); //!< link to the initialization function

static struct
{
    uint16_t DE2LastCmdSent;
    uint16_t u16IntervalCnt;
    uint16_t u16CmdInterval;
//    MCP802X_SCHED_MODE eOperationMode;
    uint16_t u16FaultCnt;
    MCP802X_FAULT_FLAGS eFaults;
    uint16_t u16Warnings;
    uint16_t u16DevID;
    MCP802X_STATE sCurrentState;
    uint8_t __attribute__((aligned(2u))) u8LastFaultStatus[2u];
} sVarMCPTask;

/*
 * 0 = run init schedule until configuration complete
 * 1 = read status periodically 
 * 2 = request auto baud
 */
static struct
{
    uint16_t u16Idx; //!< schedule index
    MCP802X_SCHED_MODE sReq; //!< which mode is requested on next schedule
    MCP802X_SCHED_MODE sAct; //!< actual configured mode
    MCP802X_SCHED_MODE sPrev;//!< last configuration mde
} sMcpSchedule;

#define MCP_INIT_TIME_DIV           (200u) /* cmd is handled every TMR task time */
#define MCP_SCHED_TIME_DIV          (10u) /* cmd is handled every 10 x TMR task time */

#define NB_OF_GET_DEVID_MSG         (1u)
#define NB_OF_SET_CONFIG_MSG        (2u)
#define NB_OF_GET_CONFIG_MSG        (2u)
#define NB_OF_STATUS_MSG            (2u)

#define NB_OF_MSG   ((NB_OF_SET_CONFIG_MSG) + (NB_OF_GET_CONFIG_MSG) + (NB_OF_STATUS_MSG)+ (NB_OF_GET_DEVID_MSG))

#define CONFIG_SET_CFG_BEGIN_IDX    (1u)
#define CONFIG_SET_CFG_END_IDX      (2u)

#define CONFIG_GET_CFG_BEGIN_IDX    (3u)
#define CONFIG_GET_CFG_END_IDX      (4u)

#define STATUS_BEGIN_IDX            (5u)
#define STATUS_END_IDX              (6u)

static const uint8_t au8CmdSchedule[NB_OF_MSG] = {
    GET_REVID,
    SET_CFG_0, SET_CFG_2,
    GET_CFG_0, GET_CFG_2,
    GET_STATUS_0, GET_STATUS_1,
};

/* generated based on MCC Graphical Interface settings */
static const MCP802X_CONFIG_STRUCT_t MCP802X_Config[MCP802X_MAX_CONFIG] = {
    {
        .SLEEP = 0u,
        .EXTUVLO = 0u,
        .EXTSC = 0u,
        .EXTOC = MCP802X_OVERCURRENT_1000V,
        .DRVBL = MCP802X_BLANKING_4000,
        .DEADTIME = MCP802X_DEADTIME_500
    },
};
/* static function prototypes */
static void MCP802X_vidRunAllCmds(void);
static void MCP802X_vidRunStatusCmds(void);
static uint8_t MCP802X_u8CheckRegisterState(void);

/*
 * description set new schedule state which should act next  after 
 *              completion of actual schedule
 * param       MCP802X_SCHED_MODE  CMD_INITIALIZR
 *                                  CMD_READ_STATUS
 *                                  CMD_BITRATE_ADJ
 *                                  CMD_IDLE
 * return      void
 * sequence    on demand
 * note
 */
uint8_t SEC_MCP802X MCP802X_u8SetMode(MCP802X_SCHED_MODE sNewMode)
{
    uint8_t u8retval;

    u8retval = 0xFFu;

    if (CND_WRONG_IDX > sNewMode)
    {
        sMcpSchedule.sReq = sNewMode;
        u8retval = 0u;
    }
    else
    {
        u8retval = 55u; /* schedule not known */
    }

    return u8retval;
}

/*
 * description    returns the current mode configured
 * param          void
 * return         MCP802X_SCHED_MODE
 * sequence       on demand
 * note
 */
MCP802X_SCHED_MODE SEC_MCP802X MCP802X_sGetMode(void)
{
    return sMcpSchedule.sAct;
}

/*
 * description update the actual local configuration for MCP802X and  
 *             update of this specific register if requested
 * param       MCP802X_CONFIG_ENUM_t sMcpConfigIdx  index of requested configuration
 * return      void
 * sequence    on demand
 * note        generated by MCC
 *             check range of index provided
 */
uint8_t SEC_MCP802X MCP802X_u8ConfigSet(const MCP802X_CONFIG_ENUM_t sMcpConfigIdx)
{
    uint8_t u8retval;

    /* check if index is in expected range */
    if (sMcpConfigIdx < MCP802X_MAX_CONFIG)
    {
        const MCP802X_CONFIG_STRUCT_t * psMcpConfig = &MCP802X_Config[sMcpConfigIdx];

        uint8_t regSET_CFG_0;
        uint8_t regSET_CFG_2;

        regSET_CFG_0 = (uint8_t) (psMcpConfig->SLEEP << (uint8_t) CFG0_SLEEP_SHIFT);
        regSET_CFG_0 |= (uint8_t) (psMcpConfig->EXTUVLO << (uint8_t) CFG0_EXTUVLO_SHIFT);
        regSET_CFG_0 |= (uint8_t) (psMcpConfig->EXTSC << (uint8_t) CFG0_EXTSC_SHIFT);
        regSET_CFG_0 |= (uint8_t) ((uint8_t) psMcpConfig->EXTOC << (uint8_t) CFG0_EXTOC_SHIFT);

        regSET_CFG_2 = (uint8_t) ((uint8_t) psMcpConfig->DEADTIME << (uint8_t) CFG2_DEADTIME_SHIFT);
        regSET_CFG_2 |= (uint8_t) ((uint8_t) psMcpConfig->DRVBL << (uint8_t) CFG2_DRVBL_SHIFT);

        MCP8021_SetRegister(SET_CFG_0, regSET_CFG_0);
        MCP8021_SetRegister(SET_CFG_2, regSET_CFG_2);

        /* request automatic update */
        MCP802X_u8SetMode(CMD_INITIALIZE);

        u8retval = 0u; /* configuration available */
    }
    else
    {
        /* 
         * provided index is not in the range do nothing  
         */
        u8retval = 34u;
    }
    return u8retval;
}

/*
 * description initialize the variables used for scheduling the communication 
 *             between uC and MCP802x and setup, local configuration variables
 *             and trigger full schedule on data exchange
 * param       void
 * return      void
 * sequence    on demand
 * note
 */
void SEC_MCP802X MCP802X_vidInitDriver(void)
{
    /* just in case the tsak scheduler init is need to get called 
     * also re-init the MCP802X driver
     */
    MCP8021_Initialize();

    uint16_t u16Idx;
    uint8_t * pu8Dst;

    /* default reset all variables related to MCPTask */
    pu8Dst = (uint8_t*) & sVarMCPTask.DE2LastCmdSent;
    for (u16Idx = 0u; u16Idx < (sizeof (sVarMCPTask)); u16Idx++)
    {
        *pu8Dst = 0u;
        pu8Dst++;
    }

    sMcpSchedule.sReq = CMD_IDLE;
    sMcpSchedule.sAct = CMD_IDLE;
    sMcpSchedule.sPrev = 0u;
    sMcpSchedule.u16Idx = 0u;

    sVarMCPTask.sCurrentState = MCP802X_UNINITIALIZED;
    sVarMCPTask.u16DevID = 0xFFu;
    sVarMCPTask.u16CmdInterval = MCP_INIT_TIME_DIV; /* interval used to call required commands */
}

/*
 * Gets the status of faults from the MCP802X software driver and maintains
 * a list of mutually non-exclusive fault flags representing every fault
 * mode supported by the MCP802X gate driver device. 
 * Fault flag variable is hosted by the callee and is meant to be used
 * for record-keeping purposes only. Fault flags have no impact on the fault
 * state of the MCP802X software driver or the MCP802X device.
 * Fault flags are only set by this function corresponding to fault events 
 * read from the MCP802X software driver. Fault flags are not automatically 
 * cleared when faults in the MCP802X driver or the device cease to exist.
 * @param faultFlags fault flag variable
 * @return one or more active faults were read out from the MCP802X software driver
 */
static MCP802X_FAULT_FLAGS SEC_MCP802X MCP802X_FaultStatusGet(void)
{
    MCP802X_FAULT_FLAGS faultFlags;

    faultFlags = MCP802X_NO_FAULT;

    uint8_t mcpStatus0 = MCP8021_GetRegister(GET_STATUS_0);

    uint8_t mcpStatus1 = MCP8021_GetRegister(GET_STATUS_1);


    /* adjust STATUS0 to fault flags */
    if (0u != (mcpStatus0)) /* copy findings if something detected */
    {
        /* to make sure that no recurring faults unexpected interpreted the received data is reset */
        MCP8021_SetRegister(GET_STATUS_0, 0u);

        /* copy detected fault into local buffer to make available to application */
        sVarMCPTask.u8LastFaultStatus[1u] = mcpStatus1 & 0x0Du;
        sVarMCPTask.u8LastFaultStatus[0u] = mcpStatus0 & 0xFCu; /* exclude the warning */

        uint16_t u16temp = ((uint16_t) mcpStatus0 >> 1u);

        /* check if over temperature warning is set  */
        if (MCP802X_OVER_TEMPERATURE_WARNING == (u16temp & MCP802X_OVER_TEMPERATURE_WARNING))
        {
            /* if warning is present then mark it as it  */
            sVarMCPTask.u16Warnings |= MCP802X_OVER_TEMPERATURE_WARNING;
            /* remove the warning to not trigger a fault */
            u16temp = u16temp & (uint16_t) (~MCP802X_OVER_TEMPERATURE_WARNING);
        }
        else
        {
            /* no over temperature warning detected reset flag */
            sVarMCPTask.u16Warnings = sVarMCPTask.u16Warnings & (uint16_t) (~MCP802X_OVER_TEMPERATURE_WARNING);
        }

        faultFlags = faultFlags | (MCP802X_FAULT_FLAGS) (u16temp);
    }

    /* decode STATUS1 content */
    if ((mcpStatus1 & 0x0Du) != 0u)
    {
        /* to make sure that no recurring faults unexpected interpreted the received data is reset */
        MCP8021_SetRegister(GET_STATUS_1, 0u);

        /* copy detected fault into local buffer to make available to application */
        sVarMCPTask.u8LastFaultStatus[1u] = mcpStatus1 & 0x0Du;

        uint16_t u16temp;
        u16temp = 0u;

        if (0u != (mcpStatus1 & (1u << 3u)))
        {
            u16temp |= MCP802X_MOSFET_OVERCURRENT_FAULT;
        }
        if (0u != (mcpStatus1 & (1u << 2u)))
        {
            u16temp |= MCP802X_MOSFET_UNDERVOLTAGE_LOCKOUT_FAULT;
        }
        if (1u == (mcpStatus1 & (1u << 0u)))
        {
            u16temp |= MCP802X_VREG_LDO_UNDERVOLTAGE_FAULT;
        }
        faultFlags = faultFlags | (MCP802X_FAULT_FLAGS) u16temp;
    }
    return faultFlags;
}

uint16_t SEC_MCP802X MCP802X_u16GetWarnings(void)
{
    return sVarMCPTask.u16Warnings;
}

/*
 * Gets the status of power events from the MCP802X software driver and maintains
 * a list of mutually non-exclusive flags representing every power event
 * detected by the MCP802X gate driver device. 
 * Power status flag variable is hosted by the callee and is meant to be used
 * for record-keeping purposes only. Power status flags have no impact on the
 * state of MCP802X software driver or the MCP802X device.
 * Power status flags are only set by this function corresponding to events 
 * read from the MCP802X software driver. They are not automatically 
 * cleared when the power status in MCP802X driver or the device is cleared.
 * @param pointer to powerStatus status variable
 * @return one or more active power status events were read out from the MCP802X 
 * software driver
 */
/*inline static*/ bool SEC_MCP802X MCP802X_PowerStatusGet(MCP802X_POWER_STATUS_FLAGS * powerStatus)
{
    bool powerEvent = false;

    uint8_t * pu8PowerStatus;
    uint8_t u8PowerStatus;

    pu8PowerStatus = (uint8_t*) powerStatus;
    u8PowerStatus = *pu8PowerStatus;

    const uint8_t mcpStatus0 = MCP8021_GetRegister((uint8_t) GET_STATUS_0 & 15u);
    uint8_t mcpPowerState = ((mcpStatus0 & 0xE0u) >> 5u);

    switch (mcpPowerState)
    {
    case 1u:
        u8PowerStatus = (u8PowerStatus | (uint8_t) MCP802X_POWER_ON_RESET_EVENT);
        powerEvent = true;
        break;

    case 2u:
        u8PowerStatus = (u8PowerStatus | (uint8_t) MCP802X_INPUT_UNDER_VOLTAGE_SHUTDOWN);
        powerEvent = true;
        break;

    case 3u:
        u8PowerStatus = (u8PowerStatus | (uint8_t) MCP802X_SLEEP_EVENT);
        powerEvent = true;
        break;

    case 4u:
        u8PowerStatus = (u8PowerStatus | (uint8_t) MCP802X_INPUT_OVER_VOLTAGE_SHUTDOWN);
        powerEvent = true;
        break;

    case 5u:
        u8PowerStatus = (u8PowerStatus | (uint8_t) MCP802X_OVER_TEMPERATURE_SHUTDOWN);
        powerEvent = true;
        break;

    default:
        break;
    }

    *pu8PowerStatus = u8PowerStatus;

    return powerEvent;
}

/*
 * description return the actual state of driver activity
 * param       void
 * return      MCP802X_STATE MCP802X_UNINITIALIZED   -> at startup, MCP with default configuration
 *                            MCP802X_INITIALIZATION  -> MCP802x is in progress to get new configuration
 *                            MCP802X_READY,          -> MCP802x re-configured 
 *                            MCP802X_ERROR           -> issues found in communication
 * sequence    on demand
 * note
 */
static MCP802X_STATE SEC_MCP802X MCP802X_sGetState(void)
{
    return sVarMCPTask.sCurrentState;
}

/*
 * description check if the expected time interval had been elapsed
 *             otherwise decrement the counter
 * param       void
 * return      uint8_t    0 -> counter not matching
 * sequence    on demand
 * note        function need the be called periodical to ensure
 *             equidistant decrement
 */
static uint8_t SEC_MCP802X MCP802X_u8CheckIntervalElapsed(void)
{
    uint8_t u8retval;

    if (sVarMCPTask.u16IntervalCnt >= sVarMCPTask.u16CmdInterval)
    {
        u8retval = 0x12u;
        sVarMCPTask.u16IntervalCnt = 0u;
    }
    else
    {
        sVarMCPTask.u16IntervalCnt++;
        u8retval = 0u;
    }

    return u8retval;
}

/*
 * description if there is a pending schedule change then prepare
 *             necessary registers setup
 * param       void
 * return      uint8_t    actual schedule state
 * sequence    on demand
 * note
 */
static uint8_t SEC_MCP802X MCP802X_u8CheckStateChange(void)
{
    uint16_t u8retval;

    /* if state had been changed update schedule */
    if (sMcpSchedule.sAct != sMcpSchedule.sReq)
    {
        /* save actual schedule */
        sMcpSchedule.sPrev = sMcpSchedule.sAct;
        sMcpSchedule.sAct = sMcpSchedule.sReq;
        sVarMCPTask.u16CmdInterval = MCP_SCHED_TIME_DIV;
        sMcpSchedule.u16Idx = 0;
        /* inform about changed state in case */
        u8retval = 0xFFu;
    }
    else
    {
        /* nothing changed */
        u8retval = 0u;
    }
    return u8retval;
}

/*
 * description return value of detected faults
 * param        void
 * return       MCP802X_FAULT_FLAGS -> catalog of detected faults
 * sequence    on demand
 * note
 */
static MCP802X_FAULT_FLAGS SEC_MCP802X MCP802X_sGetTaskFaultFlag(void)
{
    return sVarMCPTask.eFaults;
}

/*
 * description  update detected internal faults 
 * param        MCP802X_FAULT_FLAGS efault
 * return       void
 * sequence    on demand
 * note
 */
static void SEC_MCP802X MCP802X_vidSetTaskFaultFlag(MCP802X_FAULT_FLAGS efault)
{
    sVarMCPTask.eFaults = efault;
}

/*
 * description add a defined value to a counter and checks result against
 *             expected limit. If limit is reached then a flag is returned
 *             and counter is clamped to limit
 * param       uin16_t u16Inc      value to be added
 *             uint16_t u16Limit   limitation value
 * return      uint8_t             0u      -> counter did not reach limit
 *                                 != 0u   -> limit reached
 * sequence    periodic
 * note
 */
static uint8_t SEC_MCP802X MCP802X_u8FaultCountIncAndElapsed(uint16_t u16Inc, uint16_t u16Limit)
{
    uint8_t u8retval;

    sVarMCPTask.u16FaultCnt += u16Inc; /* increment fault counter */
    if (sVarMCPTask.u16FaultCnt >= u16Limit) /* fault count above limit ? */
    {
        sVarMCPTask.u16FaultCnt = u16Limit; /* limit the counter to not overflow */
        u8retval = 0x34u;
    }
    else
    {
        u8retval = 0u;
    }
    return u8retval;
}

/*
 * description counter is decremented automatically and limited to 0
 * param       void
 * return      uint8_t != 0u ->  zero not reached
 * sequence    periodic
 * note
 */
static uint8_t SEC_MCP802X MCP802X_u8FaultCountDecrement(void)
{
    uint8_t u8retval;

    if (sVarMCPTask.u16FaultCnt > 0u)/* as long something to decrement */
    {
        sVarMCPTask.u16FaultCnt -= 1u; /* reduce fault counter */
        u8retval = 0x34u;
    }
    else
    {
        u8retval = 0u;
    }
    return u8retval;
}

/*
 * description as controller only contain copy of the registers and with 
 *             a SET_CFG command the registers reported back with actual state
 *             using the DE2 this value is stored in the GetCfg register for
 *             correctness check. 
 *             If both values are identical it is assumed MCP is set correct
 * param       void
 * return      uint16_t u16RetVal -> 0u = same content on MCP and temporary location
 * sequence    periodic
 * note
 */
static uint8_t SEC_MCP802X MCP802X_u8CheckRegisterState(void)
{
    uint8_t u8retval;

    u8retval = 0u;

    if (MCP8021_GetRegister(SET_CFG_0) != MCP8021_GetRegister(GET_CFG_0))
    {
        u8retval |= 1u;
    }
    else
    {
    }

    if (MCP8021_GetRegister(SET_CFG_2) != MCP8021_GetRegister(GET_CFG_2))
    {
        u8retval |= 4u;
    }
    else
    {
    }

    return u8retval;
}

/*
 * description handles the different schedules like initialization,
 *             status read or auto-baud.
 * param       void
 * return      void
 * sequence    periodic
 * note
 */
static void SEC_MCP802X MCP802X_vidTaskHandler(void)
{
    if (sMcpSchedule.sAct != CMD_IDLE)
    {
        /* 
         * check first if communication is ongoing before throwing new message
         */

        if (MCP802X_UNINITIALIZED == sVarMCPTask.sCurrentState)
        {
            sVarMCPTask.sCurrentState = MCP802X_INITIALIZATION;
        }


        /* check the last commands handled on the bus when DE2 bus is idle */

        if (0U == MCP8021_CommunicationIdle())
        {
            /* example to check content of MCP registers updated */
            //If communication established, the ID can be obtained from the driver:
            if ((uint16_t) GET_REVID == sVarMCPTask.DE2LastCmdSent)
            {
                sVarMCPTask.u16DevID = MCP8021_GetRegister(GET_REVID); //0 is the ID register
            }

            /* 
             * run the initialization until the MCP registers are updated
             * and contain expected values
             */
            if (0U == MCP802X_u8CheckRegisterState())
            {
                /* reset fault state */
                MCP802X_vidSetTaskFaultFlag((uint16_t) MCP802X_sGetTaskFaultFlag() & (~(uint16_t) MCP802X_REGS_MISMATCH));
            }
            else
            {
                if ((MCP802X_STATE) MCP802X_READY == sVarMCPTask.sCurrentState)
                {
                    MCP802X_vidSetTaskFaultFlag((uint16_t) MCP802X_sGetTaskFaultFlag() | (uint16_t) MCP802X_REGS_MISMATCH);
                }
                else
                {
                    /* do nothing */
                }
            }
        }

        /* check what schedule type is running */
        if (CMD_BITRATE_ADJ == sMcpSchedule.sAct)
        {
            /*
             * if the auto-baud had been requested run that command once
             * and if completed then switch back to last state
             */
            (void) MCP8021_SendCmd((uint8_t) RUN_AUTOBAUD);

            /*
             * this is preparation for next schedule task to switch
             * back to previous schedule when auto-baud had been 
             * requested in between 
             */
            sMcpSchedule.sAct = sMcpSchedule.sPrev;
            sMcpSchedule.sReq = sMcpSchedule.sAct;

        }
        else if (CMD_READ_STATUS == sMcpSchedule.sAct)
        {
            /* read MCP status registers and update in temporary ones */
            MCP802X_vidRunStatusCmds();

        }
        else if (CMD_INITIALIZE == sMcpSchedule.sAct)
        {
            /* refresh all MCP registers */
            MCP802X_vidRunAllCmds();
        }
        else
        {
            /* do nothing */
        }
    }
    else
    {
        /* do nothing */
    }
    /*
     * all other modes do not cause active transmission
     * so check if state change requested
     */
    (void) MCP802X_u8CheckStateChange();

}

/*
 * description this function handles all configured commands which are 
 *             linked to ID table MCP802xSchedule one after the other
 * param       void
 * return      void
 * sequence    on demand / periodic
 * note
 */
static void SEC_MCP802X MCP802X_vidRunAllCmds(void)
{
    /* get next command which is expected to be sent and backup this ID */
    sVarMCPTask.DE2LastCmdSent = au8CmdSchedule[sMcpSchedule.u16Idx];

    /* 
     * once all IDs had been transfered to run startup initialization
     * of MCP802x then switch to potential new schedule type 
     * and schedule time if MCP had been init okay
     */

    if (sMcpSchedule.u16Idx >= NB_OF_MSG)
    {
        sMcpSchedule.u16Idx = 0u; /* start from scratch */

        if (0U == MCP802X_u8CheckRegisterState())
        {
            sVarMCPTask.sCurrentState = MCP802X_READY; /* now MCP802X is ready to use */
        }

        /* 
         * check if MCP had been initialized completely and no issues
         * had been detected on configuration
         */
        if (MCP802X_READY == sVarMCPTask.sCurrentState)
        {
            if (sVarMCPTask.u16CmdInterval != MCP_SCHED_TIME_DIV)
            {
                /* now switch to normal operation schedule timing */
                sVarMCPTask.u16CmdInterval = MCP_SCHED_TIME_DIV;
            }
        }

            MCP802X_u8SetMode(CMD_IDLE); /* transfer completed */


    }
    else
    {
        sMcpSchedule.u16Idx++;

        /* start communication frame */
        (void) MCP8021_SendCmd(sVarMCPTask.DE2LastCmdSent);
    }
}

/*
 * description run only other STATUS command checking periodically if 
 *             register get changed while FAULT is not triggered
 * param       void
 * return      void
 * sequence    on demand
 * note        reading the status registers while a fault had been detected
 *             a second read will reset the marked event in status registers
 *             therefor after completed both requests scheduler is switched 
 *             back into idle mode
 */
static void SEC_MCP802X MCP802X_vidRunStatusCmds(void)
{
    if ((sMcpSchedule.u16Idx < (uint16_t) STATUS_BEGIN_IDX) ||
            (sMcpSchedule.u16Idx > (uint16_t) STATUS_END_IDX))
    {
        sMcpSchedule.u16Idx = STATUS_BEGIN_IDX;
    }

    sVarMCPTask.u16CmdInterval = MCP_SCHED_TIME_DIV;

    sVarMCPTask.DE2LastCmdSent = au8CmdSchedule[sMcpSchedule.u16Idx];
    sMcpSchedule.u16Idx++;

    (void) MCP8021_SendCmd(sVarMCPTask.DE2LastCmdSent);

    /* 
     * all status had been read , now check for changes
     * on the schedule type command
     */
    if (sMcpSchedule.u16Idx >= NB_OF_MSG)
    {
        MCP802X_u8SetMode(CMD_IDLE); /* transfer completed */
        sMcpSchedule.u16Idx = 0u; /* start with GET_STATUS_0 command */
    }
}

/*
 * description  returns the last detected and temporary stored faults since 
 *              last calling. Internal state is reset after read
 * param        void
 * return       uint16_t collective fault states from STATUS_1 and STATUS_1
 * sequence     on demand
 * note
 */
uint16_t SEC_MCP802X MCP802X_u16GetLastMcpFaults(void)
{
    uint16_t u16retval;

    u16retval = (uint16_t) sVarMCPTask.u8LastFaultStatus[0u];
    u16retval |= (uint16_t) (sVarMCPTask.u8LastFaultStatus[1u] << 8u);

    /* reset the fault container to prevent re-activation */
    sVarMCPTask.u8LastFaultStatus[0u] = 0u;
    sVarMCPTask.u8LastFaultStatus[1u] = 0u;

    return u16retval;
}

/*
 * description reset the detected faults while executing low high transmission
 *             on OE pin of MCP802x.
 * param       void
 * return      void
 * sequence    on demand
 * note        Events will only be reset if situation causing the issue went away
 */
static void SEC_MCP802X MCP802X_vidFaultClear(void)
{
    uint16_t u16Delay;

    /* get the state of driver enable pin */
    uint16_t u16EnableState = MCP802X_ENABLE_GetValue();

    /* driver enable pin low */
    MCP802X_ENABLE_SetLow();

    /* while loop requires for cycles */
    u16Delay = (uint16_t) (5e-6 * CLOCK_InstructionFrequencyGet()) / 4u;

    while (0u != u16Delay)
    {
        Nop(); /* needed to keep while loop not removed from optimizer */
        u16Delay--;
    }

    /* to prevent any unexpected activation check the received pin state before */
    if (0u != u16EnableState)
    {
        /* rearm driver enable pin to allow MCP output being active */
        MCP802X_ENABLE_SetHigh();
    }
    /* reset the fault condition for now */
    MCP802X_vidSetTaskFaultFlag((uint16_t) MCP802X_sGetTaskFaultFlag() & (~(uint16_t) MCP802X_FAULT_PIN_FAIL));
}

/*
 * description checks the potential fault condition on /FAULT input and
 *             unsolicited status message from MCP. Depending on findings
 *             the fault reset sequence is started to release MCP from 
 *             halt condition
 * param       void
 * return      void
 * sequence    periodic
 * note
 */
static void SEC_MCP802X MCP802X_vidCheckFaultState(void)
{
    /* collect the actual fault state received via DE2 and store in task fault state */
    const MCP802X_FAULT_FLAGS u8FaultState = MCP802X_FaultStatusGet();

    /* 
     * check periodically if FAULT input is low and if fault is detected 
     * then increment fault counter and check potential receipt on
     * unsolicited message from DE2
     */
    if ((0u == (uint8_t) MCP802X_FAULT_GetValue()) && (0u != u8FaultState))
    {
        /* just as a time keeper , if FAULT went low then at least 20/9600sec 
         * later a fault should be received by solicited message */
        const uint8_t u8Count = MCP802X_u8FaultCountIncAndElapsed(1u, 10u);

        if ((0u != (uint8_t) u8FaultState) || (0u != u8Count))
        {
            MCP802X_vidFaultClear(); /* reset the fault condition via physical pin manipulation */

            /* set the indicator that fault pin cannot reset */
            if (0u == (uint8_t) MCP802X_FAULT_GetValue())
            {
                MCP802X_vidSetTaskFaultFlag((uint16_t) MCP802X_sGetTaskFaultFlag() | (uint16_t) (MCP802X_FAULT_PIN_FAIL));
            }
        }
        else
        {
            /* do nothing */
        }
    }
    else
    {
        (void) MCP802X_u8FaultCountDecrement();
    }
}

/*
 * description check the status of MCP8021
 * param       void
 * return      MCP802X_STATE
 * sequence    periodic
 * note
 */
MCP802X_STATE SEC_MCP802X MCP802X_eHandleCommunication(void)
{
    MCP802X_STATE eState;

    eState = MCP802X_sGetState();

    /* check if periodic time stamp elapsed to take next action */
    if (0u != MCP802X_u8CheckIntervalElapsed())
    {
        MCP802X_vidTaskHandler();
    }
    else
    {
        /* do nothing */
    }

    /* 
     * check every ms if FAULT input is low 
     * if fault is detected then increment fault counter
     */
    MCP802X_vidCheckFaultState();

    return eState;
}

/*
 * end of file
 */