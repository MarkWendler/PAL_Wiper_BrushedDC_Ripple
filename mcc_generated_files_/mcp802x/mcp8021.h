/**
 * MCP Generated Driver Interface Header File
 * 
 * @file      mcp8021.h
 *            
 * @defgroup  mcpdriver MCP802X Driver
 *            
 * @brief     software driver for single wire (DE2) communication used on MCP802x devices.
 *            handles dedicated data exchange as well as unsolicited messages from
 *            bridge driver in case of detected fault.
 *            driver is linked to selected UART module.
 *            Pin configuration decides if single pin ( RX/TX on same ) or dual pin 
 *            shall be used. Driver automatically handles both configurations
 *            
 * @version   Driver Version 1.0.0
 *             
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
#ifndef MCP8021_H
#define	MCP8021_H

#include <stdint.h> 
#include "../system/clock.h"

#define DE2BRG_TYP (CLOCK_PeripheralFrequencyGet()/9600UL)
#define DE2BRG_MAX (uint16_t)(((uint16_t)DE2BRG_TYP * 1.05f))
#define DE2BRG_MIN (uint16_t)(((uint16_t)DE2BRG_TYP * 0.95f))
#define DE2BRG_BREAK (uint16_t)(1.75f * DE2BRG_TYP)


/* 
 * define commands for Host to MCP8021
 */

/**
 * @struct  contain the command IDs for possible actions on MCP802x
 *          for more details refer to actual datasheet section
 */
typedef enum
{
    SET_CFG_0 = 0x81u,//!< update CFG0 register at client
    GET_CFG_0 = 0x82u,//!< refresh the local copy of CFG0 at host
    GET_STATUS_0 = 0x85u,//!< refresh local copy of STATUS_0 at host
    GET_STATUS_1 = 0x86u,//!< refresh local copy of STATUS_1 at host
    SET_CFG_2 = 0x87u, //!< update CFG2 register at client
    GET_CFG_2 = 0x88u, //!< refresh local copy of CFG2 at host
    RUN_AUTOBAUD = 0x3Cu,//!< request client to send 0x55 for bitrate measurement
    GET_REVID = 0x90u //!< read the client specific device information
} _eDE2CMD;

/*
 * @enum    possible error codes responded back 
 */
typedef enum
{
    DE2_OK = 0u,    //!< communication is still okay
    DE2_TRMT_MISMATCH,  //!< transmitted data not received back with expected value
    DE2_GENERAL_RCV_NOK,    //!< general reception issue
    DE2_RCV_FRAMESIZE_MISMATCH  //!< data package size from client wrong
} _eDE2Error;

typedef enum
{
    STATUS0_FAULT_MASK = 0x01u,
    STATUS0_OTPW_MASK = 0x02u,
    STATUS0_OTPF_MASK = 0x04u,
    STATUS0_UVLOF_MASK = 0x08u,
    STATUS0_OVLOF_MASK = 0x10u,
    STATUS0_DHTDN_MASK = 0xE0u,
} _uSTATUS0_MASK;

typedef enum
{
    STATUS0_FAULT_SHIFT = 0x00u,
    STATUS0_OTPW_SHIFT = 0x01u,
    STATUS0_OTPF_SHIFT = 0x02u,
    STATUS0_UVLOF_SHIFT = 0x03u,
    STATUS0_OVLOF_SHIFT = 0x04u,
    STATUS0_DHTDN_SHIFT = 0x05u,
} _uSTATUS0_BITS_SHIFT;

typedef enum
{
    STATUS1_VREGUVF_MASK = 0x01,
    STATUS1_XUVLOF_MASK = 0x04,
    STATUS1_XOCPF_MASK = 0x08
} _uSTATUS1_MASK;

typedef enum
{
    STATUS1_VREGUVF_SHIFT = 0x00,
    STATUS1_XUVLOF_SHIFT = 0x02,
    STATUS1_XOCPF_SHIFT = 0x03
} _uSTATUS1_BITS_SHIFT;

typedef enum
{
    CFG0_EXTOC_MASK = 0x03,
    CFG0_EXTSC_MASK = 0x04,
    CFG0_EXTUVLO_MASK = 0x08,
    CFG0_SLEEP_MASK = 0x20,
} _uCFG0_MASK;

typedef enum
{
    CFG0_EXTOC_SHIFT = 0x00,
    CFG0_EXTSC_SHIFT = 0x02,
    CFG0_EXTUVLO_SHIFT = 0x03,
    CFG0_SLEEP_SHIFT = 0x05,
} _uCFG0_BITS_SHIFT;

typedef enum
{
    CFG2_DRVBL_MASK = 0x03,
    CFG2_DEADTIME_MASK = 0x1C,
} _uCFG2_MASK;

typedef enum
{
    CFG2_DRVBL_SHIFT = 0x00,
    CFG2_DEADTIME_SHIFT = 0x02,
} _uCFG2_BITS_SHIFT;

/* ********************************************************************	*/

/**
 * @ingroup     mcpdriver
 * @brief       check if single wire bus protocol is completed
 * @param[in]   void
 * @return      return actual state of protocol handler
 */
extern uint8_t MCP8021_CommunicationIdle(void);

/**
 * @ingroup     mcpdriver
 * @brief       set the local backup registers which will be transfered to bridge driver
 *              when schedule is changed to CMD_INITIALIZE
 * @param[in]   reg - register index 
 * @param[in]   value - expected register value
 * @return      void
 */
extern void MCP8021_SetRegister(uint8_t reg, uint8_t value);

/**
 * @ingroup     mcpdriver
 * @brief       get the local backup register value updated via DE2 from bridge driver
 * @param[in]   reg - addressed register index
 * @return      value of local backup location
 */
extern uint8_t MCP8021_GetRegister(uint8_t reg);

/**
 * @ingroup     mcpdriver
 * @brief       initializes the data transfer from host to client. As DE2 is 
 *              a single wire RX_Handler will get the data to act then according 
 *              the received command
 * @param[in]   uint8_t u8Cmd as defined in enum _eDE2CMD
 * @return      uint8_t 0 = not able to send
 * @sequence    on demand
 * @note
 */
extern uint8_t MCP8021_SendCmd(uint8_t u8Cmd); 

#endif