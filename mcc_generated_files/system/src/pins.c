/**
 * PINS Generated Driver Source File 
 * 
 * @file      pins.c
 *            
 * @ingroup   pinsdriver
 *            
 * @brief     This is the generated driver source file for PINS driver.
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

// Section: Includes
#include <xc.h>
#include <stddef.h>
#include "../pins.h"

// Section: File specific functions
static void (*MCP802X_FAULT_InterruptHandler)(void) = NULL;

// Section: Driver Interface Function Definitions
void PINS_Initialize(void)
{
    /****************************************************************************
     * Setting the Output Latch SFR(s)
     ***************************************************************************/
    LATA = 0x0000;
    LATB = 0x0000;
    LATC = 0x0000;
    LATD = 0x0000;

    /****************************************************************************
     * Setting the GPIO Direction SFR(s)
     ***************************************************************************/
    TRISA = 0x0007;
    TRISB = 0x03FF;
    TRISC = 0x1F7F;
    TRISD = 0x2502;


    /****************************************************************************
     * Setting the Weak Pull Up and Weak Pull Down SFR(s)
     ***************************************************************************/
    CNPUA = 0x0000;
    CNPUB = 0x0000;
    CNPUC = 0x1000;
    CNPUD = 0x0002;
    CNPDA = 0x0000;
    CNPDB = 0x0000;
    CNPDC = 0x0000;
    CNPDD = 0x0000;


    /****************************************************************************
     * Setting the Open Drain SFR(s)
     ***************************************************************************/
    ODCA = 0x0000;
    ODCB = 0x0000;
    ODCC = 0x0000;
    ODCD = 0x0000;


    /****************************************************************************
     * Setting the Analog/Digital Configuration SFR(s)
     ***************************************************************************/
    ANSELA = 0x001F;
    ANSELB = 0x009F;
    ANSELC = 0x004F;
    ANSELD = 0x0000;

    /****************************************************************************
     * Set the PPS
     ***************************************************************************/
     __builtin_write_RPCON(0x0000); // unlock PPS

        RPINR19bits.U2RXR = 0x003C; //RC12->UART2:U2RX;
        RPINR18bits.U1RXR = 0x004A; //RD10->UART1:U1RX;
        RPOR14bits.RP60R = 0x0003;  //RC12->UART2:U2TX;
        RPOR11bits.RP55R = 0x0001;  //RC7->UART1:U1TX;

     __builtin_write_RPCON(0x0800); // lock PPS

    /*******************************************************************************
    * Interrupt On Change: negative
    *******************************************************************************/
    CNEN1Dbits.CNEN1D1 = 1; //Pin : RD1; 

    /****************************************************************************
     * Interrupt On Change: flag
     ***************************************************************************/
    CNFDbits.CNFD1 = 0;    //Pin : MCP802X_FAULT

    /****************************************************************************
     * Interrupt On Change: config
     ***************************************************************************/
    CNCONDbits.CNSTYLE = 1; //Config for PORTD
    CNCONDbits.ON = 1; //Config for PORTD

    /* Initialize IOC Interrupt Handler*/
    MCP802X_FAULT_SetInterruptHandler(&MCP802X_FAULT_CallBack);

    /****************************************************************************
     * Interrupt On Change: Interrupt Enable
     ***************************************************************************/
    IFS4bits.CNDIF = 0; //Clear CNDI interrupt flag
    IEC4bits.CNDIE = 1; //Enable CNDI interrupt
}

void __attribute__ ((weak)) MCP802X_FAULT_CallBack(void)
{

}

void MCP802X_FAULT_SetInterruptHandler(void (* InterruptHandler)(void))
{ 
    IEC4bits.CNDIE = 0; //Disable CNDI interrupt
    MCP802X_FAULT_InterruptHandler = InterruptHandler; 
    IEC4bits.CNDIE = 1; //Enable CNDI interrupt
}

/* Interrupt service function for the CNDI interrupt. */
void __attribute__ (( interrupt, no_auto_psv )) _CNDInterrupt (void)
{
    if(CNFDbits.CNFD1 == 1)
    {
        if(MCP802X_FAULT_InterruptHandler != NULL) 
        { 
            MCP802X_FAULT_InterruptHandler(); 
        }
        
        CNFDbits.CNFD1 = 0;  //Clear flag for Pin - MCP802X_FAULT
    }
    
    // Clear the flag
    IFS4bits.CNDIF = 0;
}

