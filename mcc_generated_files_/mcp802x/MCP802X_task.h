/**
 * MCP Generated Driver Interface Header File
 * 
 * @file      mcp8021_task.h
 *            
 * @ingroup  mcpdriver MCP802X TaskHandler
 *            
 * @brief     protocol handler between MCP802x and controller
 *            
 * @version   Driver Version 1.0.0
*/

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

// This is a guard condition so that contents of this file are not included
// more than once.
#ifndef MCP802X_TASK_H
#define MCP802X_TASK_H
#include "mcp8021.h"

/************************************/
/*       Constants                  */
/************************************/

/************************************/
/*       Typedefs                   */
/************************************/

/*
 * 0 = run init schedule until configuration complete
 * 1 = read status periodically 
 * 2 = request autobaud
 * 3 = no communication on DE2 from master expected
 */
typedef enum
{
    CMD_INITIALIZE = 0u, //!< used to update all MCP registers
    CMD_READ_STATUS = 1u, //!< read both status registers
    CMD_BITRATE_ADJ = 2u, //!< request client to respond with 0x55 to measure actual bitrate
    CMD_IDLE = 3u, //!< bus shall b in idle state, wait for unsoicited messages */
    CND_WRONG_IDX //!< out of usable index indicator
} MCP802X_SCHED_MODE;

typedef enum
{
    MCP802X_UNINITIALIZED = 0u, //!< no communication had been issued unti now
    MCP802X_INITIALIZATION = 1u, //!< System Initialization Stage
    MCP802X_READY = 2u, //!< System is Ready
    MCP802X_ERROR = 0xFFu
} MCP802X_STATE;

typedef enum tagMCP802X_FAULT_FLAGS
{
    MCP802X_NO_FAULT = 0u,
    MCP802X_OVER_TEMPERATURE_WARNING = 1u,
    MCP802X_OVER_TEMPERATURE_FAULT = 2u,
    MCP802X_INPUT_UNDER_VOLTAGE_FAULT = 4u,
    MCP802X_INPUT_OVER_VOLTAGE_FAULT = 8u,
    MCP802X_MOSFET_OVERCURRENT_FAULT = 16u,
    MCP802X_MOSFET_UNDERVOLTAGE_LOCKOUT_FAULT = 32u,
    MCP802X_VREG_LDO_UNDERVOLTAGE_FAULT = 64u,
    MCP802X_FAULT_PIN_FAIL = 128u,
    MCP802X_REGS_MISMATCH = 256u
} MCP802X_FAULT_FLAGS;

typedef enum tagMCP802X_POWER_STATUS_FLAGS
{
    MCP802X_NORMAL_OPERATION = 0u,
    MCP802X_OVER_TEMPERATURE_SHUTDOWN = 1u,
    MCP802X_INPUT_UNDER_VOLTAGE_SHUTDOWN = 2u,
    MCP802X_INPUT_OVER_VOLTAGE_SHUTDOWN = 4u,
    MCP802X_SLEEP_EVENT = 8u,
    MCP802X_POWER_ON_RESET_EVENT = 16u
} MCP802X_POWER_STATUS_FLAGS;

/**********************************/
/*       Configuring MCP802X      */
/**********************************/

typedef enum
{
    MCP802X_OVERCURRENT_0250V = 0b00, //00 = Overcurrent limit 0.250V
    MCP802X_OVERCURRENT_0500V = 0b01, //01 = Overcurrent limit 0.500V
    MCP802X_OVERCURRENT_0750V = 0b10, //10 = Overcurrent limit 0.750V
    MCP802X_OVERCURRENT_1000V = 0b11  //11 = Overcurrent limit 1.000V
} MCP802X_OVERCURRENT_LIMIT_t;

typedef enum
{
    MCP802X_DEADTIME_2000 = 0b000,     //2000 ns
    MCP802X_DEADTIME_1750 = 0b001,     //1750 ns
    MCP802X_DEADTIME_1500 = 0b010,     //1500 ns
    MCP802X_DEADTIME_1250 = 0b011,     //1250 ns
    MCP802X_DEADTIME_1000 = 0b100,     //1000 ns
    MCP802X_DEADTIME_750  = 0b101,     // 750 ns
    MCP802X_DEADTIME_500  = 0b110,     // 500 ns
    MCP802X_DEADTIME_250  = 0b111      // 250 ns
} MCP802X_DEADTIME_t;

typedef enum
{
    MCP802X_BLANKING_4000 = 0b00, //4000 ns
    MCP802X_BLANKING_2000 = 0b01, //2000 ns
    MCP802X_BLANKING_1000 = 0b10, //1000 ns
    MCP802X_BLANKING_500  = 0b11, //500 ns
}MCP802X_BLANKINGTIME_t;


typedef struct
{
    uint8_t SLEEP;     //!<Sleep Mode (1 = System enters Sleep mode when OE = 0, 0 = System enters Standby mode when OE = 0)
    uint8_t EXTUVLO;   //!<External MOSFET Undervoltage Lockout
    uint8_t EXTSC;     //!<External MOSFET Short-Circuit Detection
    MCP802X_OVERCURRENT_LIMIT_t EXTOC; //!<External MOSFET Overcurrent Limit Value
    MCP802X_DEADTIME_t DEADTIME; //!<Driver Dead-Time Selection
    MCP802X_BLANKINGTIME_t DRVBL; //!<Driver Blanking Time Selection
} MCP802X_CONFIG_STRUCT_t;

/* available configuration names from MCC Graphical Interface */
typedef enum tag_MCP802X_CONFIG_ENUM_t
{
    config1  =  (0u),
    MCP802X_MAX_CONFIG = (1u)
}MCP802X_CONFIG_ENUM_t;

#define MCP802X_DEFAULT_CONFIG  config1

/************************************/
/*       Function prototypes        */
/************************************/

 /**
 * @ingroup     mcpdriver
 * @brief       handles the communication triggered by a timer
 * @param[in]   void
 * @return      void
 */ 
extern MCP802X_STATE        MCP802X_eHandleCommunication(void);

/**
 * @ingroup     mcpdriver
 * @brief       initializes the necessary variables and the driver interface
 * @param[in]   void
 * @return      void
 */ 
extern void                 MCP802X_vidInitDriver(void);

/**
 * @ingroup     mcpdriver
 * @brief       collect the last information from both status registers of bridge driver
 * @param[in]   void
 * @return      value of STATUS0 ( low byte)  and STATUS1 ( high byte )
 */
extern uint16_t             MCP802X_u16GetLastMcpFaults(void);

/**
 * @ingroup     mcpdriver
 * @brief       update the local backup registers depending on selected configuration
 * @param[in]   sMcpConfigIdx - index of configuration needed
 * @return      0 if configuration was possible to set
 */
extern uint8_t              MCP802X_u8ConfigSet(const MCP802X_CONFIG_ENUM_t sMcpConfigIdx);

/**
 * @ingroup     mcpdriver
 * @brief       sets the action for next schedule scheme between controller and
 *              bridge driver. After completing the schedule scheme is set to
 *              CMD_IDLE. This is done to just act on potential fault based 
 *              unsolicited messages. Possible configurations are
 *                  CMD_INITIALIZE
 *                  CMD_READ_STATUS
 *                  CMD_BITRATE_ADJ
 *                  CMD_IDLE (default )
 * 
 * @param[in]   sMcpConfigIdx - index of configuration needed
 * @return      0 if configuration was possible
 */
extern uint8_t              MCP802X_u8SetMode(MCP802X_SCHED_MODE sNewMode);

/**
 * @ingroup     mcpdriver
 * @brief       return the actual configured mode
 * @param[in]   void
 * @return      actual configured mode
 */
extern MCP802X_SCHED_MODE   MCP802X_sGetMode(void);

/**
 * @ingroup     mcpdriver
 * @brief       request actual detected warnings
 * @param[in]   void
 * @return      detected warnings
 */
extern uint16_t MCP802X_u16GetWarnings(void);


extern bool MCP802X_PowerStatusGet(MCP802X_POWER_STATUS_FLAGS * powerStatus);

#endif

/*
 * end of file
 */