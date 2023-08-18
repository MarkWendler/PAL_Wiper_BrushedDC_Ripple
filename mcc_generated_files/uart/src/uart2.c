/**
 * UART2 Generated Driver Source File
 * 
 * @file        uart2.c
 *  
 * @ingroup     uartdriver
 *  
 * @brief       This is the generated driver source file for the UART2 driver
 *            
 * @skipline @version     Firmware Driver Version 1.6.1
 *
 * @skipline @version     PLIB Version 1.4.1
 *
 * @skipline    Device : dsPIC33CDVL64MC106
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

// Section: Included Files
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <xc.h>
#include "../uart2.h"

// Section: Macro Definitions
#define UART2_CLOCK 100000000U
#define UART2_BAUD_TO_BRG_WITH_FRACTIONAL(x) (UART2_CLOCK/(x))
#define UART2_BAUD_TO_BRG_WITH_BRGH_1(x) (UART2_CLOCK/(4U*(x))-1U)
#define UART2_BAUD_TO_BRG_WITH_BRGH_0(x) (UART2_CLOCK/(16U*(x))-1U)
#define UART2_BRG_TO_BAUD_WITH_FRACTIONAL(x) (UART2_CLOCK/(x))
#define UART2_BRG_TO_BAUD_WITH_BRGH_1(x) (UART2_CLOCK/(4U*((x)+1U)))
#define UART2_BRG_TO_BAUD_WITH_BRGH_0(x) (UART2_CLOCK/(16U*((x)+1U)))

#define UART2_MIN_ACHIEVABLE_BAUD_WITH_FRACTIONAL 95U
#define UART2_MIN_ACHIEVABLE_BAUD_WITH_BRGH_1 24U

// Section: Driver Interface

const struct UART_INTERFACE UART2_Drv = {
    .Initialize = &UART2_Initialize,
    .Deinitialize = &UART2_Deinitialize,
    .Read = &UART2_Read,
    .Write = &UART2_Write,
    .IsRxReady = &UART2_IsRxReady,
    .IsTxReady = &UART2_IsTxReady,
    .IsTxDone = &UART2_IsTxDone,
    .TransmitEnable = &UART2_TransmitEnable,
    .TransmitDisable = &UART2_TransmitDisable,
    .AutoBaudSet = &UART2_AutoBaudSet,
    .AutoBaudQuery = &UART2_AutoBaudQuery,
    .AutoBaudEventEnableGet = &UART2_AutoBaudEventEnableGet,
    .BRGCountSet = &UART2_BRGCountSet,
    .BRGCountGet = &UART2_BRGCountGet,
    .BaudRateSet = &UART2_BaudRateSet,
    .BaudRateGet = &UART2_BaudRateGet,
    .ErrorGet = &UART2_ErrorGet,
    .RxCompleteCallbackRegister = &UART2_RxCompleteCallbackRegister,
    .TxCompleteCallbackRegister = &UART2_TxCompleteCallbackRegister,
    .TxCollisionCallbackRegister = &UART2_TxCollisionCallbackRegister,
    .FramingErrorCallbackRegister = &UART2_FramingErrorCallbackRegister,
    .OverrunErrorCallbackRegister = &UART2_OverrunErrorCallbackRegister,
    .ParityErrorCallbackRegister = &UART2_ParityErrorCallbackRegister,
};

// Section: Private Variable Definitions

static volatile bool softwareBufferEmpty = true;
static union
{
    struct
    {
        uint16_t frammingError :1;
        uint16_t parityError :1;
        uint16_t overrunError :1;
        uint16_t txCollisionError :1;
        uint16_t autoBaudOverflow :1;
        uint16_t reserved :11;
    };
    size_t status;
} uartError;

// Section: Data Type Definitions

/**
 @ingroup  uartdriver
 @static   UART Driver Queue Status
 @brief    Defines the object required for the status of the queue
*/
static uint8_t * volatile rxTail;
static uint8_t *rxHead;
static uint8_t *txTail;
static uint8_t * volatile txHead;
static bool volatile rxOverflowed;

/**
 @ingroup  uartdriver
 @brief    Defines the length of the Transmit and Receive Buffers
*/

/* We add one extra byte than requested so that we don't have to have a separate
 * bit to determine the difference between buffer full and buffer empty, but
 * still be able to hold the amount of data requested by the user.  Empty is
 * when head == tail.  So full will result in head/tail being off by one due to
 * the extra byte.
 */
#define UART2_CONFIG_TX_BYTEQ_LENGTH (8+1)
#define UART2_CONFIG_RX_BYTEQ_LENGTH (8+1)

/**
 @ingroup  uartdriver
 @static   UART Driver Queue
 @brief    Defines the Transmit and Receive Buffers
*/
static uint8_t txQueue[UART2_CONFIG_TX_BYTEQ_LENGTH];
static uint8_t rxQueue[UART2_CONFIG_RX_BYTEQ_LENGTH];

static void (*UART2_RxCompleteHandler)(void);
static void (*UART2_TxCompleteHandler)(void);
static void (*UART2_TxCollisionHandler)(void);
static void (*UART2_FramingErrorHandler)(void);
static void (*UART2_OverrunErrorHandler)(void);
static void (*UART2_ParityErrorHandler)(void);

// Section: Driver Interface

void UART2_Initialize(void)
{
    IEC1bits.U2TXIE = 0;
    IEC1bits.U2RXIE = 0;
    IEC11bits.U2EVTIE = 0;

    // URXEN ; RXBIMD ; UARTEN disabled; MOD Asynchronous 8-bit UART; UTXBRK ; BRKOVR ; UTXEN ; USIDL ; WAKE ; ABAUD ; BRGH ; 
    U2MODE = 0x0;
    // STSEL 1 Stop bit sent, 1 checked at RX; BCLKMOD enabled; SLPEN ; FLO ; BCLKSEL FOSC/2; C0EN ; RUNOVF ; UTXINV ; URXINV ; HALFDPLX ; 
    U2MODEH = 0x800;
    // OERIE ; RXBKIF ; RXBKIE ; ABDOVF ; OERR ; TXCIE ; TXCIF ; FERIE ; TXMTIE ; ABDOVE ; CERIE ; CERIF ; PERIE ; 
    U2STA = 0x80;
    // URXISEL ; UTXBE ; UTXISEL TX_SEVEN_WORDS; URXBE ; STPMD ; TXWRE ; 
    U2STAH = 0x702E;
    // BaudRate 9599.69; Frequency 100000000 Hz; BRG 10417; 
    U2BRG = 0x28B1;
    // BRG 0; 
    U2BRGH = 0x0;
    
    txHead = txQueue;
    txTail = txQueue;
    rxHead = rxQueue;
    rxTail = rxQueue;
   
    rxOverflowed = false;
    
    UART2_RxCompleteCallbackRegister(&UART2_RxCompleteCallback);
    UART2_TxCompleteCallbackRegister(&UART2_TxCompleteCallback);
    UART2_TxCollisionCallbackRegister(&UART2_TxCollisionCallback);
    UART2_FramingErrorCallbackRegister(&UART2_FramingErrorCallback);
    UART2_OverrunErrorCallbackRegister(&UART2_OverrunErrorCallback);
    UART2_ParityErrorCallbackRegister(&UART2_ParityErrorCallback);

    // UART Frame error interrupt
//    U2STAbits.FERIE = 1;
    // UART Parity error interrupt
//    U2STAbits.PERIE = 1;
    // UART Receive Buffer Overflow interrupt
//    U2STAbits.OERIE = 1;
    // UART Transmit collision interrupt
//    U2STAbits.TXCIE = 1;
    // UART Auto-Baud Overflow interrupt
//    U2STAbits.ABDOVE = 1;  
    // UART Receive Interrupt
    IEC1bits.U2RXIE = 1;
    // UART Event interrupt
//    IEC11bits.U2EVTIE = 1;
    // UART Error interrupt
//    IEC3bits.U2EIE    = 1;
    
    //Make sure to set LAT bit corresponding to TxPin as high before UART initialization
    U2MODEbits.UARTEN = 1;   // enabling UART ON bit
    U2MODEbits.UTXEN = 1;
    U2MODEbits.URXEN = 1;
}

void UART2_Deinitialize(void)
{
    // UART Transmit interrupt
    IFS1bits.U2TXIF = 0;
    IEC1bits.U2TXIE = 0;
    
    // UART Receive Interrupt
    IFS1bits.U2RXIF = 0;
    IEC1bits.U2RXIE = 0;
    
    // UART Event interrupt
    IFS11bits.U2EVTIF = 0;
    IEC11bits.U2EVTIE = 0;
    
    // UART Error interrupt
    IFS3bits.U2EIF = 0;
    IEC3bits.U2EIE    = 0;
    
    U2MODE = 0x0;
    U2MODEH = 0x0;
    U2STA = 0x80;
    U2STAH = 0x2E;
    U2BRG = 0x0;
    U2BRGH = 0x0;
}

uint8_t UART2_Read(void)
{
    uint8_t data = 0;

    if(rxHead != rxTail)
	{
		data = *rxHead;

		rxHead++;

		if (rxHead == &rxQueue[UART2_CONFIG_RX_BYTEQ_LENGTH])
		{
			rxHead = rxQueue;
		}
	}
    return data;
}

void UART2_Write(uint8_t byte)
{
    // if software buffer is empty and hardware FIFO is not full write to TXREG directly
    if((softwareBufferEmpty == true) && (U2STAHbits.UTXBF == 0))
    {
        U2TXREG = byte;
    }
    else
    {
        while(UART2_IsTxReady() == 0)
        {
        }

        *txTail = byte;

        txTail++;
        
        if (txTail == &txQueue[UART2_CONFIG_TX_BYTEQ_LENGTH])
        {
            txTail = txQueue;
        }

        IEC1bits.U2TXIE = 1;
        softwareBufferEmpty = false;
    }
}

bool UART2_IsRxReady(void)
{    
    return !(rxHead == rxTail);
}

bool UART2_IsTxReady(void)
{
    uint16_t size;
    uint8_t *snapshot_txHead = (uint8_t*)txHead;
    
    if (txTail < snapshot_txHead)
    {
        size = (snapshot_txHead - txTail - 1);
    }
    else
    {
        size = ( UART2_CONFIG_TX_BYTEQ_LENGTH - (txTail - snapshot_txHead) - (uint16_t)1 );
    }
    
    return (size != (uint16_t)0);
}

bool UART2_IsTxDone(void)
{
    bool status = false;
    
    if(txTail == txHead)
    {
        status = (bool)(U2STAbits.TRMT && U2STAHbits.UTXBE);
    }
    
    return status;
}

void UART2_TransmitEnable(void)
{
    U2MODEbits.UTXEN = 1;
}

void UART2_TransmitDisable(void)
{
    U2MODEbits.UTXEN = 0;
}

void UART2_AutoBaudSet(bool enable)
{
    U2INTbits.ABDIF = 0U;
    U2INTbits.ABDIE = enable;
    U2MODEbits.ABAUD = enable;
}

bool UART2_AutoBaudQuery(void)
{
    return U2MODEbits.ABAUD;
}

bool UART2_AutoBaudEventEnableGet(void)
{ 
    return U2INTbits.ABDIE; 
}


void UART2_BRGCountSet(uint32_t brgValue)
{
    U2BRG = brgValue & 0xFFFFU;
    U2BRGH = (brgValue >>16U) & 0x000FU;
}

uint32_t UART2_BRGCountGet(void)
{
    uint32_t brgValue;
    
    brgValue = U2BRGH;
    brgValue = (brgValue << 16U) | U2BRG;
    
    return brgValue;
}

void UART2_BaudRateSet(uint32_t baudRate)
{
    uint32_t brgValue;
    
    if((baudRate >= UART2_MIN_ACHIEVABLE_BAUD_WITH_FRACTIONAL) && (baudRate != 0))
    {
        U2MODEHbits.BCLKMOD = 1;
        U2MODEbits.BRGH = 0;
        brgValue = UART2_BAUD_TO_BRG_WITH_FRACTIONAL(baudRate);
    }
    else if(baudRate >= UART2_MIN_ACHIEVABLE_BAUD_WITH_BRGH_1)
    {
        U2MODEHbits.BCLKMOD = 0;
        U2MODEbits.BRGH = 1;
        brgValue = UART2_BAUD_TO_BRG_WITH_BRGH_1(baudRate);
    }
    else
    {
        U2MODEHbits.BCLKMOD = 0;
        U2MODEbits.BRGH = 0;
        brgValue = UART2_BAUD_TO_BRG_WITH_BRGH_0(baudRate);
    }
    U2BRG = brgValue & 0xFFFFU;
    U2BRGH = (brgValue >>16U) & 0x000FU;
}

uint32_t UART2_BaudRateGet(void)
{
    uint32_t brgValue;
    uint32_t baudRate;
    
    brgValue = UART2_BRGCountGet();
    if((U2MODEHbits.BCLKMOD == 1) && (brgValue != 0))
    {
        baudRate = UART2_BRG_TO_BAUD_WITH_FRACTIONAL(brgValue);
    }
    else if(U2MODEbits.BRGH == 1)
    {
        baudRate = UART2_BRG_TO_BAUD_WITH_BRGH_1(brgValue);
    }
    else
    {
        baudRate = UART2_BRG_TO_BAUD_WITH_BRGH_0(brgValue);
    }
    return baudRate;
}

size_t UART2_ErrorGet(void)
{
    size_t fetchUartError = uartError.status;
    uartError.status = 0;
    return fetchUartError;
}

void UART2_RxCompleteCallbackRegister(void (*handler)(void))
{
    if(NULL != handler)
    {
        UART2_RxCompleteHandler = handler;
    }
}

void __attribute__ ((weak)) UART2_RxCompleteCallback(void)
{ 

} 

void UART2_TxCompleteCallbackRegister(void (*handler)(void))
{
    if(NULL != handler)
    {
        UART2_TxCompleteHandler = handler;
    }
}

void __attribute__ ((weak)) UART2_TxCompleteCallback(void)
{ 

} 

void UART2_TxCollisionCallbackRegister(void (*handler)(void))
{
    if(NULL != handler)
    {
        UART2_TxCollisionHandler = handler;
    }
}

void __attribute__ ((weak)) UART2_TxCollisionCallback(void)
{ 

} 

void UART2_FramingErrorCallbackRegister(void (*handler)(void))
{
    if(NULL != handler)
    {
        UART2_FramingErrorHandler = handler;
    }
}

void __attribute__ ((weak)) UART2_FramingErrorCallback(void)
{ 

} 

void UART2_OverrunErrorCallbackRegister(void (*handler)(void))
{
    if(NULL != handler)
    {
        UART2_OverrunErrorHandler = handler;
    }
}

void __attribute__ ((weak)) UART2_OverrunErrorCallback(void)
{ 

} 

void UART2_ParityErrorCallbackRegister(void (*handler)(void))
{
    if(NULL != handler)
    {
        UART2_ParityErrorHandler = handler;
    }
}

void __attribute__ ((weak)) UART2_ParityErrorCallback(void)
{ 

} 

void __attribute__ ( ( interrupt, no_auto_psv ) ) _U2TXInterrupt(void)
{

    if(txHead == txTail)
    {
        IEC1bits.U2TXIE = 0;
        softwareBufferEmpty = true;
    }
    else
    {
        IFS1bits.U2TXIF = 0;

        while(!(U2STAHbits.UTXBF == 1))
        {
            U2TXREG = *txHead;
            txHead++;

            if(txHead == &txQueue[UART2_CONFIG_TX_BYTEQ_LENGTH])
            {
                txHead = txQueue;
            }

            // Are we empty?
            if(txHead == txTail)
            {
				if(NULL != UART2_TxCompleteHandler)
					{
						(*UART2_TxCompleteHandler)();
					}
                break;
            }
        }
    }
}

void __attribute__ ( ( interrupt, no_auto_psv ) ) _U2RXInterrupt(void)
{
    size_t rxQueueSize = 0;
    uint8_t *rxTailPtr = NULL;
    
    IFS1bits.U2RXIF = 0;
    
    while(!(U2STAHbits.URXBE == 1))
    {
        *rxTail = U2RXREG;

        rxQueueSize = UART2_CONFIG_RX_BYTEQ_LENGTH - 1;
        rxTailPtr = rxTail;
        rxTailPtr++;
        // Will the increment not result in a wrap and not result in a pure collision?
        // This is most often condition so check first
        if ((rxTail != &rxQueue[rxQueueSize]) && (rxTailPtr != rxHead))
        {
            rxTail++;
        } 
        else if ( (rxTail == &rxQueue[rxQueueSize]) &&
                  (rxHead !=  rxQueue) )
        {
            // Pure wrap no collision
            rxTail = rxQueue;
        } 
        else // must be collision
        {
            rxOverflowed = true;
        }
    }
	
    if(NULL != UART2_RxCompleteHandler)
    {
        (*UART2_RxCompleteHandler)();
    }
}

void __attribute__ ( ( interrupt, no_auto_psv ) ) _U2EInterrupt(void)
{
    if (U2STAbits.ABDOVF == 1)
    {
        uartError.status = uartError.status|UART_ERROR_AUTOBAUD_OVERFLOW_MASK;
        U2STAbits.ABDOVF = 0;
    }
    
    if (U2STAbits.TXCIF == 1)
    {
        uartError.status = uartError.status|UART_ERROR_TX_COLLISION_MASK;
        if(NULL != UART2_TxCollisionHandler)
        {
            (*UART2_TxCollisionHandler)();
        }
        
        U2STAbits.TXCIF = 0;
    }
    
    if (U2STAbits.OERR == 1)
    {
        uartError.status = uartError.status|UART_ERROR_RX_OVERRUN_MASK;
        if(NULL != UART2_OverrunErrorHandler)
        {
            (*UART2_OverrunErrorHandler)();
        }
        
        U2STAbits.OERR = 0;
    }
    
    if (U2STAbits.PERR == 1)
    {
        uartError.status = uartError.status|UART_ERROR_PARITY_MASK;
        if(NULL != UART2_ParityErrorHandler)
        {
            (*UART2_ParityErrorHandler)();
        }
    }
    
    if (U2STAbits.FERR == 1)
    {
        uartError.status = uartError.status|UART_ERROR_FRAMING_MASK;
        if(NULL != UART2_FramingErrorHandler)
        {
            (*UART2_FramingErrorHandler)();
        }
    }
        
    IFS3bits.U2EIF = 0;
}

/* ISR for UART Event Interrupt */
void __attribute__ ( ( interrupt, no_auto_psv ) ) _U2EVTInterrupt(void)
{
    U2INTbits.ABDIF = false;
    IFS11bits.U2EVTIF = false;
}
