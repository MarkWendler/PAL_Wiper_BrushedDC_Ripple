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
 * @version   Firmware Driver Version 1.0.1
 *
 * @version   PLIB Version 1.1.0
 *
 * @skipline  Device : dsPIC33CDVL64MC106
*/

/*
� [2023] Microchip Technology Inc. and its subsidiaries.

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
 * @brief    Sets the RA3 GPIO Pin which has a custom name of IO_RA3 to High
 * @pre      The RA3 must be set as Output Pin             
 * @return   none  
 */
#define IO_RA3_SetHigh()          (_LATA3 = 1)

/**
 * @ingroup  pinsdriver
 * @brief    Sets the RA3 GPIO Pin which has a custom name of IO_RA3 to Low
 * @pre      The RA3 must be set as Output Pin
 * @return   none  
 */
#define IO_RA3_SetLow()           (_LATA3 = 0)

/**
 * @ingroup  pinsdriver
 * @brief    Toggles the RA3 GPIO Pin which has a custom name of IO_RA3
 * @pre      The RA3 must be set as Output Pin
 * @return   none  
 */
#define IO_RA3_Toggle()           (_LATA3 ^= 1)

/**
 * @ingroup  pinsdriver
 * @brief    Reads the value of the RA3 GPIO Pin which has a custom name of IO_RA3
 * @return   none  
 */
#define IO_RA3_GetValue()         _RA3

/**
 * @ingroup  pinsdriver
 * @brief    Configures the RA3 GPIO Pin which has a custom name of IO_RA3 as Input
 * @return   none  
 */
#define IO_RA3_SetDigitalInput()  (_TRISA3 = 1)

/**
 * @ingroup  pinsdriver
 * @brief    Configures the RA3 GPIO Pin which has a custom name of IO_RA3 as Output
 * @return   none  
 */
#define IO_RA3_SetDigitalOutput() (_TRISA3 = 0)

/**
 * @ingroup  pinsdriver
 * @brief    Sets the RB0 GPIO Pin which has a custom name of IO_RB0 to High
 * @pre      The RB0 must be set as Output Pin             
 * @return   none  
 */
#define IO_RB0_SetHigh()          (_LATB0 = 1)

/**
 * @ingroup  pinsdriver
 * @brief    Sets the RB0 GPIO Pin which has a custom name of IO_RB0 to Low
 * @pre      The RB0 must be set as Output Pin
 * @return   none  
 */
#define IO_RB0_SetLow()           (_LATB0 = 0)

/**
 * @ingroup  pinsdriver
 * @brief    Toggles the RB0 GPIO Pin which has a custom name of IO_RB0
 * @pre      The RB0 must be set as Output Pin
 * @return   none  
 */
#define IO_RB0_Toggle()           (_LATB0 ^= 1)

/**
 * @ingroup  pinsdriver
 * @brief    Reads the value of the RB0 GPIO Pin which has a custom name of IO_RB0
 * @return   none  
 */
#define IO_RB0_GetValue()         _RB0

/**
 * @ingroup  pinsdriver
 * @brief    Configures the RB0 GPIO Pin which has a custom name of IO_RB0 as Input
 * @return   none  
 */
#define IO_RB0_SetDigitalInput()  (_TRISB0 = 1)

/**
 * @ingroup  pinsdriver
 * @brief    Configures the RB0 GPIO Pin which has a custom name of IO_RB0 as Output
 * @return   none  
 */
#define IO_RB0_SetDigitalOutput() (_TRISB0 = 0)

/**
 * @ingroup  pinsdriver
 * @brief    Sets the RC13 GPIO Pin which has a custom name of MCP802X_ENABLE to High
 * @pre      The RC13 must be set as Output Pin             
 * @return   none  
 */
#define MCP802X_ENABLE_SetHigh()          (_LATC13 = 1)

/**
 * @ingroup  pinsdriver
 * @brief    Sets the RC13 GPIO Pin which has a custom name of MCP802X_ENABLE to Low
 * @pre      The RC13 must be set as Output Pin
 * @return   none  
 */
#define MCP802X_ENABLE_SetLow()           (_LATC13 = 0)

/**
 * @ingroup  pinsdriver
 * @brief    Toggles the RC13 GPIO Pin which has a custom name of MCP802X_ENABLE
 * @pre      The RC13 must be set as Output Pin
 * @return   none  
 */
#define MCP802X_ENABLE_Toggle()           (_LATC13 ^= 1)

/**
 * @ingroup  pinsdriver
 * @brief    Reads the value of the RC13 GPIO Pin which has a custom name of MCP802X_ENABLE
 * @return   none  
 */
#define MCP802X_ENABLE_GetValue()         _RC13

/**
 * @ingroup  pinsdriver
 * @brief    Configures the RC13 GPIO Pin which has a custom name of MCP802X_ENABLE as Input
 * @return   none  
 */
#define MCP802X_ENABLE_SetDigitalInput()  (_TRISC13 = 1)

/**
 * @ingroup  pinsdriver
 * @brief    Configures the RC13 GPIO Pin which has a custom name of MCP802X_ENABLE as Output
 * @return   none  
 */
#define MCP802X_ENABLE_SetDigitalOutput() (_TRISC13 = 0)

/**
 * @ingroup  pinsdriver
 * @brief    Sets the RD1 GPIO Pin which has a custom name of MCP802X_FAULT to High
 * @pre      The RD1 must be set as Output Pin             
 * @return   none  
 */
#define MCP802X_FAULT_SetHigh()          (_LATD1 = 1)

/**
 * @ingroup  pinsdriver
 * @brief    Sets the RD1 GPIO Pin which has a custom name of MCP802X_FAULT to Low
 * @pre      The RD1 must be set as Output Pin
 * @return   none  
 */
#define MCP802X_FAULT_SetLow()           (_LATD1 = 0)

/**
 * @ingroup  pinsdriver
 * @brief    Toggles the RD1 GPIO Pin which has a custom name of MCP802X_FAULT
 * @pre      The RD1 must be set as Output Pin
 * @return   none  
 */
#define MCP802X_FAULT_Toggle()           (_LATD1 ^= 1)

/**
 * @ingroup  pinsdriver
 * @brief    Reads the value of the RD1 GPIO Pin which has a custom name of MCP802X_FAULT
 * @return   none  
 */
#define MCP802X_FAULT_GetValue()         _RD1

/**
 * @ingroup  pinsdriver
 * @brief    Configures the RD1 GPIO Pin which has a custom name of MCP802X_FAULT as Input
 * @return   none  
 */
#define MCP802X_FAULT_SetDigitalInput()  (_TRISD1 = 1)

/**
 * @ingroup  pinsdriver
 * @brief    Configures the RD1 GPIO Pin which has a custom name of MCP802X_FAULT as Output
 * @return   none  
 */
#define MCP802X_FAULT_SetDigitalOutput() (_TRISD1 = 0)

/**
 * @ingroup  pinsdriver
 * @brief    Sets the RD13 GPIO Pin which has a custom name of IO_RD13 to High
 * @pre      The RD13 must be set as Output Pin             
 * @return   none  
 */
#define IO_RD13_SetHigh()          (_LATD13 = 1)

/**
 * @ingroup  pinsdriver
 * @brief    Sets the RD13 GPIO Pin which has a custom name of IO_RD13 to Low
 * @pre      The RD13 must be set as Output Pin
 * @return   none  
 */
#define IO_RD13_SetLow()           (_LATD13 = 0)

/**
 * @ingroup  pinsdriver
 * @brief    Toggles the RD13 GPIO Pin which has a custom name of IO_RD13
 * @pre      The RD13 must be set as Output Pin
 * @return   none  
 */
#define IO_RD13_Toggle()           (_LATD13 ^= 1)

/**
 * @ingroup  pinsdriver
 * @brief    Reads the value of the RD13 GPIO Pin which has a custom name of IO_RD13
 * @return   none  
 */
#define IO_RD13_GetValue()         _RD13

/**
 * @ingroup  pinsdriver
 * @brief    Configures the RD13 GPIO Pin which has a custom name of IO_RD13 as Input
 * @return   none  
 */
#define IO_RD13_SetDigitalInput()  (_TRISD13 = 1)

/**
 * @ingroup  pinsdriver
 * @brief    Configures the RD13 GPIO Pin which has a custom name of IO_RD13 as Output
 * @return   none  
 */
#define IO_RD13_SetDigitalOutput() (_TRISD13 = 0)

/**
 * @ingroup  pinsdriver
 * @brief    Initializes the PINS module
 * @return   none  
 */
void PINS_Initialize(void);

/**
 * @ingroup  pinsdriver
 * @brief    This function is callback for MCP802X_FAULT Pin
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
