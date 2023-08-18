/**
 * PINS Generated Driver Header File 
 * 
 * @file      pins.h
 *            
 * @defgroup  pinsdriver Pins Driver
 *            
 * @brief     The Pin Driver directs the operation and function of 
 *            the selected device pins using dsPIC MCUs.
 *
 * @skipline @version   Firmware Driver Version 1.0.2
 *
 * @skipline @version   PLIB Version 1.3.0
 *
 * @skipline  Device : dsPIC33CDVL64MC106
*/

/*
© [2023] Microchip Technology Inc. and its subsidiaries.

    Subject to your compliance with these terms, you may use Microchip 
    software and any derivatives exclusively with Microchip products. 
    You are responsible for complying with 3rd party license terms  
    applicable to your use of 3rd party software (including open source  
    software) that may accompany Microchip software. SOFTWARE IS ?AS IS.? 
    NO WARRANTIES, WHETHER EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS 
    SOFTWARE, INCLUDING ANY IMPLIED WARRANTIES OF NON-INFRINGEMENT,  
    MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT 
    WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE, 
    INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY 
    KIND WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF 
    MICROCHIP HAS BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE 
    FORESEEABLE. TO THE FULLEST EXTENT ALLOWED BY LAW, MICROCHIP?S 
    TOTAL LIABILITY ON ALL CLAIMS RELATED TO THE SOFTWARE WILL NOT 
    EXCEED AMOUNT OF FEES, IF ANY, YOU PAID DIRECTLY TO MICROCHIP FOR 
    THIS SOFTWARE.
*/

#ifndef PINS_H
#define PINS_H
// Section: Includes
#include <xc.h>

// Section: Device Pin Macros

/**
 * @ingroup  pinsdriver
 * @brief    Sets the RB0 GPIO Pin which has a custom name of OUT_RB0 to High
 * @pre      The RB0 must be set as Output Pin             
 * @param    none
 * @return   none  
 */
#define OUT_RB0_SetHigh()          (_LATB0 = 1)

/**
 * @ingroup  pinsdriver
 * @brief    Sets the RB0 GPIO Pin which has a custom name of OUT_RB0 to Low
 * @pre      The RB0 must be set as Output Pin
 * @param    none
 * @return   none  
 */
#define OUT_RB0_SetLow()           (_LATB0 = 0)

/**
 * @ingroup  pinsdriver
 * @brief    Toggles the RB0 GPIO Pin which has a custom name of OUT_RB0
 * @pre      The RB0 must be set as Output Pin
 * @param    none
 * @return   none  
 */
#define OUT_RB0_Toggle()           (_LATB0 ^= 1)

/**
 * @ingroup  pinsdriver
 * @brief    Reads the value of the RB0 GPIO Pin which has a custom name of OUT_RB0
 * @param    none
 * @return   none  
 */
#define OUT_RB0_GetValue()         _RB0

/**
 * @ingroup  pinsdriver
 * @brief    Configures the RB0 GPIO Pin which has a custom name of OUT_RB0 as Input
 * @param    none
 * @return   none  
 */
#define OUT_RB0_SetDigitalInput()  (_TRISB0 = 1)

/**
 * @ingroup  pinsdriver
 * @brief    Configures the RB0 GPIO Pin which has a custom name of OUT_RB0 as Output
 * @param    none
 * @return   none  
 */
#define OUT_RB0_SetDigitalOutput() (_TRISB0 = 0)

/**
 * @ingroup  pinsdriver
 * @brief    Sets the RC13 GPIO Pin which has a custom name of MCP802X_ENABLE to High
 * @pre      The RC13 must be set as Output Pin             
 * @param    none
 * @return   none  
 */
#define MCP802X_ENABLE_SetHigh()          (_LATC13 = 1)

/**
 * @ingroup  pinsdriver
 * @brief    Sets the RC13 GPIO Pin which has a custom name of MCP802X_ENABLE to Low
 * @pre      The RC13 must be set as Output Pin
 * @param    none
 * @return   none  
 */
#define MCP802X_ENABLE_SetLow()           (_LATC13 = 0)

/**
 * @ingroup  pinsdriver
 * @brief    Toggles the RC13 GPIO Pin which has a custom name of MCP802X_ENABLE
 * @pre      The RC13 must be set as Output Pin
 * @param    none
 * @return   none  
 */
#define MCP802X_ENABLE_Toggle()           (_LATC13 ^= 1)

/**
 * @ingroup  pinsdriver
 * @brief    Reads the value of the RC13 GPIO Pin which has a custom name of MCP802X_ENABLE
 * @param    none
 * @return   none  
 */
#define MCP802X_ENABLE_GetValue()         _RC13

/**
 * @ingroup  pinsdriver
 * @brief    Configures the RC13 GPIO Pin which has a custom name of MCP802X_ENABLE as Input
 * @param    none
 * @return   none  
 */
#define MCP802X_ENABLE_SetDigitalInput()  (_TRISC13 = 1)

/**
 * @ingroup  pinsdriver
 * @brief    Configures the RC13 GPIO Pin which has a custom name of MCP802X_ENABLE as Output
 * @param    none
 * @return   none  
 */
#define MCP802X_ENABLE_SetDigitalOutput() (_TRISC13 = 0)

/**
 * @ingroup  pinsdriver
 * @brief    Sets the RD1 GPIO Pin which has a custom name of MCP802X_FAULT to High
 * @pre      The RD1 must be set as Output Pin             
 * @param    none
 * @return   none  
 */
#define MCP802X_FAULT_SetHigh()          (_LATD1 = 1)

/**
 * @ingroup  pinsdriver
 * @brief    Sets the RD1 GPIO Pin which has a custom name of MCP802X_FAULT to Low
 * @pre      The RD1 must be set as Output Pin
 * @param    none
 * @return   none  
 */
#define MCP802X_FAULT_SetLow()           (_LATD1 = 0)

/**
 * @ingroup  pinsdriver
 * @brief    Toggles the RD1 GPIO Pin which has a custom name of MCP802X_FAULT
 * @pre      The RD1 must be set as Output Pin
 * @param    none
 * @return   none  
 */
#define MCP802X_FAULT_Toggle()           (_LATD1 ^= 1)

/**
 * @ingroup  pinsdriver
 * @brief    Reads the value of the RD1 GPIO Pin which has a custom name of MCP802X_FAULT
 * @param    none
 * @return   none  
 */
#define MCP802X_FAULT_GetValue()         _RD1

/**
 * @ingroup  pinsdriver
 * @brief    Configures the RD1 GPIO Pin which has a custom name of MCP802X_FAULT as Input
 * @param    none
 * @return   none  
 */
#define MCP802X_FAULT_SetDigitalInput()  (_TRISD1 = 1)

/**
 * @ingroup  pinsdriver
 * @brief    Configures the RD1 GPIO Pin which has a custom name of MCP802X_FAULT as Output
 * @param    none
 * @return   none  
 */
#define MCP802X_FAULT_SetDigitalOutput() (_TRISD1 = 0)

/**
 * @ingroup  pinsdriver
 * @brief    Sets the RD8 GPIO Pin which has a custom name of IO_SW1_RD8 to High
 * @pre      The RD8 must be set as Output Pin             
 * @param    none
 * @return   none  
 */
#define IO_SW1_RD8_SetHigh()          (_LATD8 = 1)

/**
 * @ingroup  pinsdriver
 * @brief    Sets the RD8 GPIO Pin which has a custom name of IO_SW1_RD8 to Low
 * @pre      The RD8 must be set as Output Pin
 * @param    none
 * @return   none  
 */
#define IO_SW1_RD8_SetLow()           (_LATD8 = 0)

/**
 * @ingroup  pinsdriver
 * @brief    Toggles the RD8 GPIO Pin which has a custom name of IO_SW1_RD8
 * @pre      The RD8 must be set as Output Pin
 * @param    none
 * @return   none  
 */
#define IO_SW1_RD8_Toggle()           (_LATD8 ^= 1)

/**
 * @ingroup  pinsdriver
 * @brief    Reads the value of the RD8 GPIO Pin which has a custom name of IO_SW1_RD8
 * @param    none
 * @return   none  
 */
#define IO_SW1_RD8_GetValue()         _RD8

/**
 * @ingroup  pinsdriver
 * @brief    Configures the RD8 GPIO Pin which has a custom name of IO_SW1_RD8 as Input
 * @param    none
 * @return   none  
 */
#define IO_SW1_RD8_SetDigitalInput()  (_TRISD8 = 1)

/**
 * @ingroup  pinsdriver
 * @brief    Configures the RD8 GPIO Pin which has a custom name of IO_SW1_RD8 as Output
 * @param    none
 * @return   none  
 */
#define IO_SW1_RD8_SetDigitalOutput() (_TRISD8 = 0)

/**
 * @ingroup  pinsdriver
 * @brief    Sets the RD13 GPIO Pin which has a custom name of IO_SW2_RD13 to High
 * @pre      The RD13 must be set as Output Pin             
 * @param    none
 * @return   none  
 */
#define IO_SW2_RD13_SetHigh()          (_LATD13 = 1)

/**
 * @ingroup  pinsdriver
 * @brief    Sets the RD13 GPIO Pin which has a custom name of IO_SW2_RD13 to Low
 * @pre      The RD13 must be set as Output Pin
 * @param    none
 * @return   none  
 */
#define IO_SW2_RD13_SetLow()           (_LATD13 = 0)

/**
 * @ingroup  pinsdriver
 * @brief    Toggles the RD13 GPIO Pin which has a custom name of IO_SW2_RD13
 * @pre      The RD13 must be set as Output Pin
 * @param    none
 * @return   none  
 */
#define IO_SW2_RD13_Toggle()           (_LATD13 ^= 1)

/**
 * @ingroup  pinsdriver
 * @brief    Reads the value of the RD13 GPIO Pin which has a custom name of IO_SW2_RD13
 * @param    none
 * @return   none  
 */
#define IO_SW2_RD13_GetValue()         _RD13

/**
 * @ingroup  pinsdriver
 * @brief    Configures the RD13 GPIO Pin which has a custom name of IO_SW2_RD13 as Input
 * @param    none
 * @return   none  
 */
#define IO_SW2_RD13_SetDigitalInput()  (_TRISD13 = 1)

/**
 * @ingroup  pinsdriver
 * @brief    Configures the RD13 GPIO Pin which has a custom name of IO_SW2_RD13 as Output
 * @param    none
 * @return   none  
 */
#define IO_SW2_RD13_SetDigitalOutput() (_TRISD13 = 0)

/**
 * @ingroup  pinsdriver
 * @brief    Initializes the PINS module
 * @param    none
 * @return   none  
 */
void PINS_Initialize(void);

/**
 * @ingroup  pinsdriver
 * @brief    This function is callback for MCP802X_FAULT Pin
 * @param    none
 * @return   none   
 */
void MCP802X_FAULT_CallBack(void);


/**
 * @ingroup    pinsdriver
 * @brief      This function assigns a function pointer with a callback address
 * @param[in]  InterruptHandler - Address of the callback function 
 * @return     none  
 */
void MCP802X_FAULT_SetInterruptHandler(void (* InterruptHandler)(void));


#endif
