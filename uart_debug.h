/* Microchip Technology Inc. and its subsidiaries.  You may use this software 
 * and any derivatives exclusively with Microchip products. 
 * 
 * THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS".  NO WARRANTIES, WHETHER 
 * EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED 
 * WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A 
 * PARTICULAR PURPOSE, OR ITS INTERACTION WITH MICROCHIP PRODUCTS, COMBINATION 
 * WITH ANY OTHER PRODUCTS, OR USE IN ANY APPLICATION. 
 *
 * IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE, 
 * INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND 
 * WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS 
 * BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE.  TO THE 
 * FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS 
 * IN ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF 
 * ANY, THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
 *
 * MICROCHIP PROVIDES THIS SOFTWARE CONDITIONALLY UPON YOUR ACCEPTANCE OF THESE 
 * TERMS. 
 */

/* 
 * File:   
 * Author: 
 * Comments:
 * Revision history: 
 */

// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef MCP8021_DEBUG_H
#define	MCP8021_DEBUG_H

#include <xc.h> // include processor files - each processor file is guarded.  
#include <stdio.h>
#include "./mcc_generated_files/uart/uart2.h"

#define MCP8021_ENABLE_DEBUG    0
#define MAIN_ENABLE_DEBUG       0
#define CURRENT_ENABLE_DEBUG    1
#define SPEED_ENABLE_DEBUG      1

#if (MCP8021_ENABLE_DEBUG == 1)
    #define MCP8021_PRINTF(x)  printf(x)
    #define MCP8021_PRINTF_VAR(x,y)  printf(x,y)
#else
    #define MCP8021_PRINTF(x)  ;
    #define MCP8021_PRINTF_VAR(x,y)  ;

#endif // MCP8021_ENABLE_DEBUG


#if (MAIN_ENABLE_DEBUG == 1)
    #define MAIN_PRINTF(x)  printf(x)
    #define MAIN_PRINTF_VAR(x,y)  printf(x,y)
#else
    #define MAIN_PRINTF(x)  ;
    #define MAIN_PRINTF_VAR(x,y)  ;

#endif // MAIN_ENABLE_DEBUG


#if (CURRENT_ENABLE_DEBUG == 1)
    #define CURRENT_PRINTF(x)  printf(x)
    #define CURRENT_PRINTF_VAR(x,y)  printf(x,y)

#else
    #define CURRENT_PRINTF(x)  ;
    #define CURRENT_PRINTF_VAR(x,y)  ;

#endif // CURRENT_ENABLE_DEBUG

#if (SPEED_ENABLE_DEBUG == 1)
    #define SPEED_PRINTF(x)  printf(x)  ;
    #define SPEED_PRINTF_VAR(x,y)  printf(x,y)  ;
#else
    #define SPEED_PRINTF(x)  
    #define SPEED_PRINTF_VAR(x,y)  
#endif // SPEED_ENABLE_DEBUG

#ifdef	__cplusplus
extern "C" {
#endif /* __cplusplus */

    // TODO If C++ is being used, regular C code needs function names to have C 
    // linkage so the functions can be used by the c code. 

#ifdef	__cplusplus
}
#endif /* __cplusplus */

#endif	/* MCP8021_DEBUG_H */

