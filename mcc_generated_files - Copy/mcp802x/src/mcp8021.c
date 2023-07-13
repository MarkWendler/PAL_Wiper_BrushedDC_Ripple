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

#include "../../system/system.h"
#include "../mcp8021.h"
#include "../MCP802X_task.h"
#include "../../uart/uart1.h"
#include "../../system/pins.h"
#include "../../../uart_debug.h"

/* ********************************************************************	*/
/* ********************************************************************	*/

#define SEC_MCP802X __attribute__((section(".MCP802X")))

#define UART_DE2    UART1_Drv

/**
 * @ingroup     mcpdriver
 * @brief       initializes the driver related variables and services for UART 
 * @param[in]   void
 * @return      void
 * @sequence    on demand
 * @note
 */
extern void MCP8021_Initialize(void);

/* 
 * typedefs
 */

/**
 @ingroup  mcpdriver
 @union   _uCFG0
 @brief    union containing bit representation for CFG0 register
 */
typedef union
{
    uint8_t b;

    struct
    {
        uint8_t EXTOC : 2u; //!< external MOSFET over current level
        uint8_t EXTSC : 1u; //!< external MOSFET Short-Circuit Detection
        uint8_t EXTUVLO : 1u; //!< external MOSFET under voltage lockout
        uint8_t nu1 : 1u;
        uint8_t SLEEP : 1u; //!< enable SLEEP mode when OE pin set low
        uint8_t nu2 : 1u;
        uint8_t nu3 : 1u;
    };
} _uCFG0;

/**
 @ingroup  mcpdriver
 @union   _uCFG2
 @brief    union containing bit representation for CFG2 register
 */
typedef union
{
    uint8_t b;

    struct
    {
        uint8_t DRVBL : 2u; //!< blanking time selection
        uint8_t DRVDT : 3u; //!< dead time selection
        uint8_t nu1 : 3u;
    };
} _uCFG2;

/**
 @ingroup  mcpdriver
 @union   _uSTATUS0
 @brief    union containing bit representation of STATUS0 register
 */
typedef union
{
    uint8_t b;

    struct
    {
        uint8_t FAULT : 1u; //!< general fault flag  
        uint8_t OTPW : 1u; //!< overtemperature warning
        uint8_t OTPF : 1u; //!< device shutoff while overtemperature
        uint8_t UVLOF : 1u; //!< supply voltage below defined minimum
        uint8_t OVLOF : 1u; //!< supply voltage above defined maximum
        uint8_t DHTDN : 3u; //!< Power Control Status (configuration lost if non-zero value)
    };
} _uSTATUS0;

/**
 @ingroup  mcpdriver
 @union   _uSTATUS1
 @brief    union containing bit representation of STATUS1 register
 */
typedef union
{
    uint8_t b;

    struct
    {
        uint8_t VREGUVF : 1u; //!< VREG output undervoltage
        uint8_t nu1 : 1u;
        uint8_t XUVLOF : 1u; //!< bootstrap voltage below threshold
        uint8_t XOCPF : 1u; //!< external MOSFET overcurrent 
        uint8_t nu2 : 4u;
    };
} _uSTATUS1;

/* ********************************************************************	*/
/* register bit definition of MCP8021							        */
/* ********************************************************************	*/

/**
 @ingroup  mcpdriver
 @stuct    sVarDE2 
 @brief    structure containing variables needed for communication protocol
 *         between controller and bridge driver
 */
volatile struct
{
    /* communication buffer for receive and transmission */
    uint8_t aRxData[4u]; //!< data reception buffer
    uint8_t aTxData[2u]; //!< command transmission buffer to client
    uint16_t u16LastKnownBrg; //!< contains last BRG value if autobaud is requested

    union
    {
        uint8_t b;

        struct
        {
            uint8_t WaitForResponse : 1u; //!< host sent command frame awaiting now feedback 
            uint8_t TxPending : 1u;//1< protector against unexpected transmission override
            uint8_t GotAck : 1u; //!< a command sent by host has successfully acknowledged   
            uint8_t SendCmd : 1u;//!< host shall send a command
            uint8_t Error : 2u; //!< error in protocol handling detected
            uint8_t AutoBaud : 1u; //!< autobaud is activated, baudrate generator became a counter
            uint8_t nu : 1u;
        };
    } uflags; /* DE2 communication flags */
    uint8_t u8FrameCompleted; //!< a frame with command and response had been handled
    uint8_t u8TxReqCnt; //!< contain the size of a command ( 1 or 2 bytes )
    uint8_t u8TxCnt; //!< counter for numbers of bytes sent
    uint8_t u8RxCnt; //1!< counter for bytes reeived
    uint8_t u8AutoBaudActive; //!< indicates that autobaud is in progress
} sVarDE2;

/**
 * @description     MCP8021Reg contain the local copy of client corresponding registers
 * @brief           this is the footprint of MCP8021 register set to make 
 *                  comparison about expected and received value.
 *                  There are two sets of registers:
 *                  <br>1. for SET 
 *                  <br>2. for GET
 *                  This prevents from overwriting when updating the register on 
 *                  device or reading values from device.
 */

static union 
{
    uint8_t b[9u];

    struct
    {
        uint8_t RevId; //!< local copy of client revision ID
        _uCFG0 SetCFG0; //!< local copy of CFG0 with data need to be transfered to client
        _uCFG0 GetCFG0; //!< local copy of CFG0 read from client
        uint8_t nu[2u]; //!< not implemented
        _uSTATUS0 STATUS0; //!< local copy of STATUS_0 read from client
        _uSTATUS1 STATUS1; //!< local copy of STATUS_1 read from client
        _uCFG2 SetCFG2; //!< local copy of CFG2 with data need to be transfered to client
        _uCFG2 GetCFG2; //!< local copy of CFG2 read from client
    };
} MCP8021Reg; //!< local storage of client registers


/*
 * brief       set the local backup registers which will be transfered to bridge driver
 *              when schedule is changed to CMD_INITIALIZE
 * param[in]   reg - register index 
 * param[in]   value - expected register value
 * return      void
 */

void MCP8021_SetRegister(uint8_t reg, uint8_t value)
{
    uint8_t temp;

    temp = reg & 0x0FU; /* extract the index out of CMD */
    if (temp < 9U) /* CFG_2 is last command with highest index of 8 */
    {
        MCP8021Reg.b[temp] = value;
    }
}

/*
 * brief       get the local backup register value updated via DE2 from bridge driver
 * param[in]   reg - addressed register index
 * return      value of local backup location
 */

uint8_t MCP8021_GetRegister(uint8_t reg)
{
    return MCP8021Reg.b[reg & 0x0Fu];
}

/**
 * @ingroup     mcpdriver
 * @brief       handles the data need to be sent from host to client
 * @param[in]   void
 * @return      void
 * @sequence    on event
 * @note        called from receive handler
 */

static void SEC_MCP802X MCP8021_TxHandler(void)
{
    if (0u == sVarDE2.uflags.TxPending)
    {
        /*
         * all expected bytes sent ? 
         * If so then we need to wait for the response
         */
        if (sVarDE2.u8TxCnt >= sVarDE2.u8TxReqCnt)
        {
            sVarDE2.uflags.WaitForResponse = 1u;
            sVarDE2.uflags.SendCmd = 0u;
            sVarDE2.u8TxReqCnt = 0u;
            UART_DE2.TransmitDisable(); /* release TX pin */
        }
        else
        {
            /* flag the actual transmission as CMD frame */
            sVarDE2.uflags.SendCmd = 1u;
            UART_DE2.TransmitEnable(); /* make sure XMT channel is active */
            UART_DE2.Write(sVarDE2.aTxData[sVarDE2.u8TxCnt]); /* send data */
        }
    }
}

/* ********************************************************************	*/
/* 
 * Callback function to notify upper layer in case of unsolicited message
 * parameter : 
 *          1 status_0 came through
 *          2 status_1 came through
 */

/* ********************************************************************	*/

uint8_t __attribute__((weak)) SEC_MCP802X Status_Notification(_eDE2CMD idx)
{
    return 0u;
}

/*
 * description  check all received bytes according the protocol flow
 * param        void
 * return       void
 * sequence     on event
 * note
 */

static void SEC_MCP802X MCP8021_RxHandler(void)
{
    uint16_t u16ActRcvStatus;
    uint8_t u8ActRcvData;
    uint16_t u16BRGtemp;
    uint8_t u8Cmd;

    /* 
     * get data and reset potential failure 
     * status during reception  
     */
    u16ActRcvStatus = (uint16_t) UART_DE2.ErrorGet() & 0x001Fu; /* get relevant error states */

    if (0u == (uint8_t) UART_DE2.AutoBaudEventEnableGet())
    {
        u8ActRcvData = UART_DE2.Read(); /* get received value */
    }
    else
    {
        UART_DE2.AutoBaudSet(false); /*disable Auto baud */
        u8ActRcvData = 0u;
    }

    /* check if errors occurred during receipt */
    if (0U != u16ActRcvStatus)
    {
        sVarDE2.uflags.b &= 0x80u;
        sVarDE2.uflags.Error = (_eDE2Error) DE2_GENERAL_RCV_NOK;
        UART_DE2.TransmitDisable();/*disconnect UART from physical pin */
        MCP8021_PRINTF(" DE2 ERR1," );
    }
    else
    {
        if ((0U == u8ActRcvData) && /* received a zero frame */
                (0U == sVarDE2.u8AutoBaudActive) &&
                (1U == sVarDE2.uflags.AutoBaud)) /* autobaud had been requested */
        {
            UART_DE2.TransmitDisable(); /* release TX pin */
            sVarDE2.u8AutoBaudActive = 1;
            UART_DE2.AutoBaudSet(true); /* enable Autobaud */
        }
        else if ((1u == sVarDE2.uflags.AutoBaud) && (0U != sVarDE2.u8AutoBaudActive)) /* autobaud had been requested */
        {
            sVarDE2.uflags.AutoBaud = 0; /* reset the marker */
            UART_DE2.AutoBaudSet(false); /* disable Autobaud */
            u16BRGtemp = UART_DE2.BRGCountGet(); /*  get captured value */
            
            /* check valid range of captured bit rate and if out of range 
             * then re-init with last known value */
            /* [misra-c2012-10.4] Required: Both operands of an operator in 
             * which the usual arithmetic conversions are performed shall 
             * have the same essential type category
             * exception decision : code inspection 
             */
            if (((u16BRGtemp > (uint16_t) DE2BRG_MAX)) || (u16BRGtemp < (int16_t) DE2BRG_MIN))/*cppcheck-suppress [misra-c2012-10.4] */
            {
                /* rewrite BRG with last known good value in case of out of range */
                UART_DE2.BRGCountSet(sVarDE2.u16LastKnownBrg);
            }
            sVarDE2.u8AutoBaudActive = 0u;
        }
        else
        {
            /* double check the bit rate to prevent stuck in case of transmit collision */
            u16BRGtemp = (uint16_t)UART_DE2.BRGCountGet();           
            if (((u16BRGtemp > (uint16_t) DE2BRG_MAX)) || (u16BRGtemp < (int16_t) DE2BRG_MIN))/*cppcheck-suppress [misra-c2012-10.4] */
            {
                UART_DE2.BRGCountSet(DE2BRG_TYP);
            }
            
            /* expected back receive from transmission while command had been sent */
            if ((0U == sVarDE2.uflags.TxPending) && (1u == sVarDE2.uflags.SendCmd))
            {
                /* check if received byte is identical with sent one */
                if ((u8ActRcvData == sVarDE2.aTxData[sVarDE2.u8TxCnt]) && (sVarDE2.u8TxCnt < 2u))
                {
                    sVarDE2.u8TxCnt++;
                    MCP8021_TxHandler();
                }
                else
                {
                    /* abort schedule as data had been corrupted */
                    sVarDE2.uflags.b &= 0x80u;
                    sVarDE2.uflags.Error = (_eDE2Error) DE2_TRMT_MISMATCH;
                    MCP8021_PRINTF(" DE2 ERR2," );
                }
                sVarDE2.u8RxCnt = 0u;
            }
            else
            {
                if (sVarDE2.u8RxCnt < 2u) /* only 2 bytes expected for reception */
                {
                    if (0u == sVarDE2.u8RxCnt) /* first byte is the CMD from MCP */
                    {
                        if ((u8ActRcvData & 0x0Fu) < 9u) /* check CMD range */
                        {
                            /* check if data could used as CMD is ACK or unsolicited */
                            if (0u != (u8ActRcvData & 0xC0u))
                            {
                                sVarDE2.aRxData[sVarDE2.u8RxCnt] = u8ActRcvData & 0x0Fu; /* get the index */
                                sVarDE2.u8RxCnt++;

                                if (0U != (u8ActRcvData & 0x40u))
                                {
                                    sVarDE2.uflags.GotAck = 1u;
                                }
                                else
                                {
                                    sVarDE2.uflags.GotAck = 0u;
                                }
                            }
                            else
                            {
                                sVarDE2.uflags.b &= 0x80u;/* discard the received data */
                                sVarDE2.uflags.Error = (_eDE2Error) DE2_GENERAL_RCV_NOK;
                                sVarDE2.u8RxCnt = 0u; /* reset counter in case of fault */
                                MCP8021_PRINTF(" DE2 ERR3," );
                            }
                        }
                    }
                    else
                    {
                        /* 
                         * reload the received command temporarily to prevent overwriting the SET config registers 
                         * from response. These responses are stored in the associated GET registers to make 
                         * verify possible
                         */
                        u8Cmd = sVarDE2.aRxData[0u] & 0x0Fu;

                        if ((((uint8_t) SET_CFG_0 & 0x0Fu) == u8Cmd) || \
                                (((uint8_t) SET_CFG_2 & 0x0Fu) == u8Cmd))
                        {
                            u8Cmd = (uint8_t) (u8Cmd + 1u);
                        }
                        else
                        {
                            /* do nothing */
                        }
                        /* copy received value into dedicated buffer */
                        MCP8021Reg.b[u8Cmd] = (uint8_t) u8ActRcvData;
                        /* reset all com flags except ACK  */
                        sVarDE2.uflags.b &= 0x88u;
                        sVarDE2.u8RxCnt = 0u; /* reset counter in case two bytes received */
                        sVarDE2.u8FrameCompleted = 0x55u;
                        /*
                         * check if received command is STATUS_0 or STATUS_1 
                         */

                        if (0x80u == (sVarDE2.aRxData[0u] & 0x80u)) //If unsolicited message arrives notify upper layer
                        {
                            (void) Status_Notification((_eDE2CMD) sVarDE2.aRxData[0u]);
                        }
                    }
                }
                else
                {
                    sVarDE2.u8RxCnt = 0u;
                    sVarDE2.uflags.b &= 0x80u;
                    sVarDE2.uflags.Error = (_eDE2Error) DE2_RCV_FRAMESIZE_MISMATCH;
                    MCP8021_PRINTF(" DE2 ERR4," );
                }
            }
        }
    }
}

/* ********************************************************************	*/

/* ********************************************************************	*/

static uint8_t SEC_MCP802X MCP8021_CheckFrameCompletion(void)
{
    uint8_t u8retval;

    if (0x55U == sVarDE2.u8FrameCompleted)
    {
        u8retval = 0x55u;
        sVarDE2.u8FrameCompleted = 0u;
    }
    else
    {
        u8retval = 0u;
    }
    return u8retval;
}

/*
 * brief        is used to provide information if an ACK had been received
 *              from client to identify that message had been accepted
 * param[in]    void
 * return       uint8_t 0 -> no ACK, 0x1 -> ACK
 * sequence    
 * note
 */

static uint8_t SEC_MCP802X MCP8021_CheckACKResponse(void)
{
    uint8_t temp;

    temp = 0U;

    if (0U != sVarDE2.uflags.GotAck)
    {
        temp = 1U;
        sVarDE2.uflags.GotAck = 0U;
    }
    return temp;
}

/*
 * brief       initializes the data transfer from host to client. As DE2 is 
 *             a single wire RX_Handler will get the data to act then according 
 *             the received command
 * param[in]   uint8_t u8Cmd as defined in enum _eDE2CMD
 * return      uint8_t 0 = not able to send
 * sequence    on demand
 * note
 */

uint8_t SEC_MCP802X MCP8021_SendCmd(uint8_t u8Cmd)
{
    uint8_t u8RetVal;

    u8RetVal = 0u;

    (void) MCP8021_CheckFrameCompletion(); /* used to reset Frame completion flag */

    sVarDE2.uflags.b &= 0x80u; /* reset all flags */

    /* check if autobaud is still active */
    if (0u != sVarDE2.uflags.AutoBaud)
    {
        sVarDE2.uflags.AutoBaud = 0u;
        /* default reset of autobaud active mode */
        sVarDE2.u8AutoBaudActive = 0u;
        UART_DE2.AutoBaudSet(false); /* clear autobaud mode */
        UART_DE2.BRGCountSet(sVarDE2.u16LastKnownBrg); /* set baudrate to last measured value */

    }
    if ((u8Cmd & 0x0Fu) < 9u)
    {
        sVarDE2.aTxData[0u] = u8Cmd;
        sVarDE2.u8TxCnt = 0u;
        sVarDE2.u8RxCnt = 0u;

        /* check if 2 bytes need to be transfered, this is relevant for SET commands 	*/

        if (((uint8_t) SET_CFG_0 == u8Cmd) || ((uint8_t) SET_CFG_2 == u8Cmd))
        {
            /* copy data from local register map into transmission buffer */

            sVarDE2.aTxData[1u] = MCP8021Reg.b[u8Cmd & 0x0Fu];
            sVarDE2.u8TxReqCnt = 2u; /* two bytes need to be transmitted */
            sVarDE2.uflags.WaitForResponse = 0u; /* so no response request at this time */
        }
        else
        {
            sVarDE2.uflags.WaitForResponse = 1u;
            sVarDE2.u8TxReqCnt = 1u;
        }

        /* check if DE2 line is idle before start transmission */
        /* start if bus is idle otherwise sign up to start after last reception */
        MCP8021_TxHandler();

        u8RetVal = (uint8_t) sVarDE2.uflags.TxPending;
    }
    else
    {
        /* check if autobaud had been requested */
        if ((uint8_t) RUN_AUTOBAUD == u8Cmd)
        {

            sVarDE2.aTxData[0u] = u8Cmd;
            sVarDE2.uflags.AutoBaud = 1u;
            /* 
             * set the BREAK bit and send a 0 causing 13TBIT
             * dominant frame. The autoBREAK feature is not used as with 
             * interrupt this would cause blocking inside the ISR for 3bit times
             * so the bit rate is changed to meet the 1.29 .. 2 ms window
             * based on nominal bit rate
             */
            sVarDE2.u16LastKnownBrg = UART_DE2.BRGCountGet();
            /* [misra-c2012-10.4] Required: Both operands of an operator in 
             * which the usual arithmetic conversions are performed shall 
             * have the same essential type category
             * exception disission : code inspection to check correcness of value
             */
            UART_DE2.BRGCountSet((uint16_t) DE2BRG_BREAK);/*cppcheck-suppress [misra-c2012-10.4]*/
            UART_DE2.TransmitEnable();
            UART_DE2.Write(0u);
        }
        else
        {
            u8RetVal = 0xFFu;
        }
    }

    return u8RetVal;
}

/*
 * brief       check if single wire bus protocol is completed
 * param[in]   void
 * return      return actual state of protocol handler
 */
uint8_t MCP8021_CommunicationIdle(void) /*cppcheck-suppress [misra-c2012-8.4]*/
{
    /* ignore the auto baud active flag , handled through SendCmd() */
    return (uint8_t) (sVarDE2.uflags.b & 0x15U);
}

/* ********************************************************************	*/

/* ********************************************************************	*/

static void MCP8021_ClearError(void)
{
    UART_DE2.Deinitialize();
    UART_DE2.Initialize();
}

/*
 * brief       initializes the driver related variables and services for UART 
 * param[in]   void
 * return      void
 * sequence    on demand
 * note
 */
void SEC_MCP802X MCP8021_Initialize(void)
{
    UART_DE2.TxCollisionCallbackRegister(0);
    UART_DE2.FramingErrorCallbackRegister(0);
    UART_DE2.OverrunErrorCallbackRegister(0);
    UART_DE2.ParityErrorCallbackRegister(0);

    /* link RX handler to RX event */
    UART_DE2.RxCompleteCallbackRegister(MCP8021_RxHandler);

    UART_DE2.AutoBaudSet(false); /* disable Auto baud */

    uint16_t u16Idx;
    uint8_t * pu8Dst;
    
    /* default reset all variables related to MCP8021 */
    pu8Dst = (uint8_t*) & MCP8021Reg.b[0u];
    for (u16Idx = 0u; u16Idx < (sizeof(MCP8021Reg)); u16Idx++)
    {
        *pu8Dst = 0u;
        pu8Dst++;
    }
    /* default reset all variables related to MCP8021 */
    pu8Dst = (uint8_t*) & sVarDE2.aRxData[0u];
    for (u16Idx = 0u; u16Idx < (sizeof (sVarDE2)); u16Idx++)
    {
        *pu8Dst = 0u;
        pu8Dst++;
    }
}

/* ********************************************************************	*/

/*
 * end of file
 */