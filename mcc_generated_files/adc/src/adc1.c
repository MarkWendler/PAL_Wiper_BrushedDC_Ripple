/**
 * ADC1 Generated Driver Source File
 * 
 * @file      adc1.c
 *            
 * @ingroup   adcdriver
 *            
 * @brief     This is the generated driver source file for ADC1 driver
 *            
 * @skipline @version   Firmware Driver Version 1.4.4
 *
 * @skipline @version   PLIB Version 2.4.2
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

// Section: Included Files
#include <stddef.h>
#include "../adc1.h"

// Section: File specific functions

static void (*ADC1_CommonHandler)(void) = NULL;
static void (*ADC1_ChannelHandler)(enum ADC_CHANNEL channel, uint16_t adcVal) = NULL;
static void (*ADC1_ComparatorHandler)(enum ADC_CMP comparator) = NULL;

// Section: File specific data type definitions

/**
 @ingroup  adcdriver
 @enum     ADC_PWM_TRIG_SRCS
 @brief    Defines the PWM ADC TRIGGER sources available for the module to use.
*/
enum ADC_PWM_TRIG_SRCS {
    PWM4_TRIGGER2 = 0xb, 
    PWM4_TRIGGER1 = 0xa, 
    PWM3_TRIGGER2 = 0x9, 
    PWM3_TRIGGER1 = 0x8, 
    PWM2_TRIGGER2 = 0x7, 
    PWM2_TRIGGER1 = 0x6, 
    PWM1_TRIGGER2 = 0x5, 
    PWM1_TRIGGER1 = 0x4, 
};

//Defines an object for ADC_MULTICORE.
const struct ADC_MULTICORE adc1Multicore = {
    .ChannelTasks                           = &ADC1_ChannelTasks, 
    .ComparatorTasks                        = NULL,
    .IndividualChannelInterruptEnable       = &ADC1_IndividualChannelInterruptEnable,
    .IndividualChannelInterruptDisable      = &ADC1_IndividualChannelInterruptDisable,
    .IndividualChannelInterruptFlagClear    = &ADC1_IndividualChannelInterruptFlagClear,
    .IndividualChannelInterruptPrioritySet  = &ADC1_IndividualChannelInterruptPrioritySet,
    .ChannelCallbackRegister                = &ADC1_ChannelCallbackRegister,
    .ComparatorCallbackRegister             = &ADC1_ComparatorCallbackRegister,
    .CorePowerEnable                        = NULL,
    .SharedCorePowerEnable                  = &ADC1_SharedCorePowerEnable,
    .PWMTriggerSourceSet                    = &ADC1_PWMTriggerSourceSet
};

//Defines an object for ADC_INTERFACE.

const struct ADC_INTERFACE ADC1 = {
    .Initialize             = &ADC1_Initialize,
    .Deinitialize           = &ADC1_Deinitialize,
    .Enable                 = &ADC1_Enable,
    .Disable                = &ADC1_Disable,
    .SoftwareTriggerEnable  = &ADC1_SoftwareTriggerEnable,
    .SoftwareTriggerDisable = &ADC1_SoftwareTriggerDisable,
    .ChannelSelect          = &ADC1_ChannelSelect, 
    .ConversionResultGet    = &ADC1_ConversionResultGet,
    .IsConversionComplete   = &ADC1_IsConversionComplete,
    .ResolutionSet          = &ADC1_ResolutionSet,
    .InterruptEnable        = &ADC1_InterruptEnable,
    .InterruptDisable       = &ADC1_InterruptDisable,
    .InterruptFlagClear     = &ADC1_InterruptFlagClear,
    .InterruptPrioritySet   = &ADC1_InterruptPrioritySet,
    .CommonCallbackRegister = &ADC1_CommonCallbackRegister,
    .Tasks                  = &ADC1_Tasks,
    .adcMulticoreInterface = &adc1Multicore,
};

// Section: Driver Interface Function Definitions

void ADC1_Initialize (void)
{
    // ADSIDL disabled; ADON enabled; 
    ADCON1L = (uint16_t)0x8000 & (uint16_t)0x7FFF; //Disabling ADON bit
    // SHRRES 12-bit resolution; FORM Integer; 
    ADCON1H = 0x60;
    // SHRADCS 2; SHREISEL Early interrupt is generated 1 TADCORE clock prior to data being ready; EIEN disabled; REFERCIE disabled; REFCIE disabled; 
    ADCON2L = 0x0;
    // SHRSAMC 48; 
    ADCON2H = 0x30;
    // CNVCHSEL AN0; SWCTRG disabled; SWLCTRG disabled; SHRSAMP disabled; SUSPCIE disabled; SUSPEND disabled; REFSEL disabled; 
    ADCON3L = 0x0;
    // SHREN enabled; CLKDIV 1; CLKSEL FVCO/4; 
    ADCON3H = (uint16_t)0xC080 & (uint16_t)0xFF00; //Disabling C0EN, C1EN, C2EN, C3EN and SHREN bits
    // SIGN0 disabled; DIFF0 disabled; SIGN1 disabled; DIFF1 disabled; SIGN2 disabled; DIFF2 disabled; SIGN3 disabled; DIFF3 disabled; SIGN4 disabled; DIFF4 disabled; SIGN5 disabled; DIFF5 disabled; SIGN6 disabled; DIFF6 disabled; SIGN7 disabled; DIFF7 disabled; 
    ADMOD0L = 0x0;
    // SIGN8 disabled; DIFF8 disabled; SIGN9 disabled; DIFF9 disabled; SIGN10 disabled; DIFF10 disabled; SIGN11 disabled; DIFF11 disabled; SIGN12 disabled; DIFF12 disabled; SIGN13 disabled; DIFF13 disabled; SIGN14 disabled; DIFF14 disabled; SIGN15 disabled; DIFF15 disabled; 
    ADMOD0H = 0x0;
    // SIGN16 disabled; DIFF16 disabled; SIGN17 disabled; DIFF17 disabled; 
    ADMOD1L = 0x0;
    // IE0 disabled; IE1 disabled; IE2 enabled; IE3 disabled; IE4 disabled; IE5 disabled; IE6 disabled; IE7 enabled; IE8 disabled; IE9 disabled; IE10 disabled; IE11 disabled; IE12 enabled; IE13 disabled; IE14 disabled; IE15 enabled; 
    ADIEL = 0x9084;
    // IE16 disabled; IE17 disabled; 
    ADIEH = 0x0;
    // 
    ADSTATL = 0x0;
    // 
    ADSTATH = 0x0;
    // CMPEN0 disabled; CMPEN1 disabled; CMPEN2 disabled; CMPEN3 disabled; CMPEN4 disabled; CMPEN5 disabled; CMPEN6 disabled; CMPEN7 disabled; CMPEN8 disabled; CMPEN9 disabled; CMPEN10 disabled; CMPEN11 disabled; CMPEN12 disabled; CMPEN13 disabled; CMPEN14 disabled; CMPEN15 disabled; 
    ADCMP0ENL = 0x0;
    // CMPEN0 disabled; CMPEN1 disabled; CMPEN2 disabled; CMPEN3 disabled; CMPEN4 disabled; CMPEN5 disabled; CMPEN6 disabled; CMPEN7 disabled; CMPEN8 disabled; CMPEN9 disabled; CMPEN10 disabled; CMPEN11 disabled; CMPEN12 disabled; CMPEN13 disabled; CMPEN14 disabled; CMPEN15 disabled; 
    ADCMP1ENL = 0x0;
    // CMPEN0 disabled; CMPEN1 disabled; CMPEN2 disabled; CMPEN3 disabled; CMPEN4 disabled; CMPEN5 disabled; CMPEN6 disabled; CMPEN7 disabled; CMPEN8 disabled; CMPEN9 disabled; CMPEN10 disabled; CMPEN11 disabled; CMPEN12 disabled; CMPEN13 disabled; CMPEN14 disabled; CMPEN15 disabled; 
    ADCMP2ENL = 0x0;
    // CMPEN0 disabled; CMPEN1 disabled; CMPEN2 disabled; CMPEN3 disabled; CMPEN4 disabled; CMPEN5 disabled; CMPEN6 disabled; CMPEN7 disabled; CMPEN8 disabled; CMPEN9 disabled; CMPEN10 disabled; CMPEN11 disabled; CMPEN12 disabled; CMPEN13 disabled; CMPEN14 disabled; CMPEN15 disabled; 
    ADCMP3ENL = 0x0;
    // CMPEN16 disabled; CMPEN17 disabled; 
    ADCMP0ENH = 0x0;
    // CMPEN16 disabled; CMPEN17 disabled; 
    ADCMP1ENH = 0x0;
    // CMPEN16 disabled; CMPEN17 disabled; 
    ADCMP2ENH = 0x0;
    // CMPEN16 disabled; CMPEN17 disabled; 
    ADCMP3ENH = 0x0;
    // CMPLO 0x0; 
    ADCMP0LO = 0x0;
    // CMPLO 0x0; 
    ADCMP1LO = 0x0;
    // CMPLO 0x0; 
    ADCMP2LO = 0x0;
    // CMPLO 0x0; 
    ADCMP3LO = 0x0;
    // CMPHI 0x0; 
    ADCMP0HI = 0x0;
    // CMPHI 0x0; 
    ADCMP1HI = 0x0;
    // CMPHI 0x0; 
    ADCMP2HI = 0x0;
    // CMPHI 0x0; 
    ADCMP3HI = 0x0;
    // 
    ADFL0DAT = 0x0;
    // 
    ADFL1DAT = 0x0;
    // 
    ADFL2DAT = 0x0;
    // 
    ADFL3DAT = 0x0;
    // FLCHSEL AN0; IE disabled; OVRSAM 4x; MODE Oversampling Mode; FLEN disabled; 
    ADFL0CON = 0x400;
    // FLCHSEL AN0; IE disabled; OVRSAM 4x; MODE Oversampling Mode; FLEN disabled; 
    ADFL1CON = 0x400;
    // FLCHSEL AN0; IE disabled; OVRSAM 4x; MODE Oversampling Mode; FLEN disabled; 
    ADFL2CON = 0x400;
    // FLCHSEL AN0; IE disabled; OVRSAM 4x; MODE Oversampling Mode; FLEN disabled; 
    ADFL3CON = 0x400;
    // LOLO disabled; LOHI disabled; HILO disabled; HIHI disabled; BTWN disabled; IE disabled; CMPEN disabled; 
    ADCMP0CON = 0x0;
    // LOLO disabled; LOHI disabled; HILO disabled; HIHI disabled; BTWN disabled; IE disabled; CMPEN disabled; 
    ADCMP1CON = 0x0;
    // LOLO disabled; LOHI disabled; HILO disabled; HIHI disabled; BTWN disabled; IE disabled; CMPEN disabled; 
    ADCMP2CON = 0x0;
    // LOLO disabled; LOHI disabled; HILO disabled; HIHI disabled; BTWN disabled; IE disabled; CMPEN disabled; 
    ADCMP3CON = 0x0;
    // LVLEN0 disabled; LVLEN1 disabled; LVLEN2 disabled; LVLEN3 disabled; LVLEN4 disabled; LVLEN5 disabled; LVLEN6 disabled; LVLEN7 disabled; LVLEN8 disabled; LVLEN9 disabled; LVLEN10 disabled; LVLEN11 disabled; LVLEN12 disabled; LVLEN13 disabled; LVLEN14 disabled; LVLEN15 disabled; 
    ADLVLTRGL = 0x0;
    // LVLEN16 disabled; LVLEN17 disabled; 
    ADLVLTRGH = 0x0;
    // EIEN0 disabled; EIEN1 disabled; EIEN2 disabled; EIEN3 disabled; EIEN4 disabled; EIEN5 disabled; EIEN6 disabled; EIEN7 disabled; EIEN8 disabled; EIEN9 disabled; EIEN10 disabled; EIEN11 disabled; EIEN12 disabled; EIEN13 disabled; EIEN14 disabled; EIEN15 disabled; 
    ADEIEL = 0x0;
    // EIEN16 disabled; EIEN17 disabled; 
    ADEIEH = 0x0;
    // EISTAT0 disabled; EISTAT1 disabled; EISTAT2 disabled; EISTAT3 disabled; EISTAT4 disabled; EISTAT5 disabled; EISTAT6 disabled; EISTAT7 disabled; EISTAT8 disabled; EISTAT9 disabled; EISTAT10 disabled; EISTAT11 disabled; EISTAT12 disabled; EISTAT13 disabled; EISTAT14 disabled; EISTAT15 disabled; 
    ADEISTATL = 0x0;
    // EISTAT16 disabled; EISTAT17 disabled; 
    ADEISTATH = 0x0;
    // SHRCIE disabled; WARMTIME 16 Source Clock Periods; 
    ADCON5H = (uint16_t)0x400 & (uint16_t)0xF0FF; //Disabling WARMTIME bit
    // 
    ADCBUF0 = 0x0;
    // 
    ADCBUF1 = 0x0;
    // 
    ADCBUF2 = 0x0;
    // 
    ADCBUF3 = 0x0;
    // 
    ADCBUF4 = 0x0;
    // 
    ADCBUF5 = 0x0;
    // 
    ADCBUF6 = 0x0;
    // 
    ADCBUF7 = 0x0;
    // 
    ADCBUF8 = 0x0;
    // 
    ADCBUF9 = 0x0;
    // 
    ADCBUF10 = 0x0;
    // 
    ADCBUF11 = 0x0;
    // 
    ADCBUF12 = 0x0;
    // 
    ADCBUF13 = 0x0;
    // 
    ADCBUF14 = 0x0;
    // 
    ADCBUF15 = 0x0;
    // 
    ADCBUF16 = 0x0;
    // 
    ADCBUF17 = 0x0;
        
    ADC1_CommonCallbackRegister(&ADC1_CommonCallback);
    ADC1_ChannelCallbackRegister(&ADC1_ChannelCallback);
    ADC1_ComparatorCallbackRegister(&ADC1_ComparatorCallback);
    
    // Clearing Channel_AN2 interrupt flag.
    IFS5bits.ADCAN2IF = 0;
    // Enabling Channel_AN2 interrupt.
    IEC5bits.ADCAN2IE = 1;
    // Clearing Channel_AN7 interrupt flag.
    IFS6bits.ADCAN7IF = 0;
    // Enabling Channel_AN7 interrupt.
    IEC6bits.ADCAN7IE = 1;
    // Clearing Channel_AN12 interrupt flag.
    IFS6bits.ADCAN12IF = 0;
    // Enabling Channel_AN12 interrupt.
    IEC6bits.ADCAN12IE = 1;
    // Clearing Channel_AN15 interrupt flag.
    IFS6bits.ADCAN15IF = 0;
    // Enabling Channel_AN15 interrupt.
    IEC6bits.ADCAN15IE = 1;

    // Setting WARMTIME bit
    ADCON5Hbits.WARMTIME = 0xF;
    // Enabling ADC Module
    ADCON1Lbits.ADON = 0x1;
    // Enabling Power for the Shared Core
    ADC1_SharedCorePowerEnable();

    //TRGSRC0 None; TRGSRC1 None; 
    ADTRIG0L = 0x0;
    //TRGSRC2 PWM2 Trigger1; TRGSRC3 None; 
    ADTRIG0H = 0x6;
    //TRGSRC4 None; TRGSRC5 None; 
    ADTRIG1L = 0x0;
    //TRGSRC6 None; TRGSRC7 PWM2 Trigger2; 
    ADTRIG1H = 0x700;
    //TRGSRC8 None; TRGSRC9 None; 
    ADTRIG2L = 0x0;
    //TRGSRC10 None; TRGSRC11 None; 
    ADTRIG2H = 0x0;
    //TRGSRC12 PWM2 Trigger1; TRGSRC13 None; 
    ADTRIG3L = 0x6;
    //TRGSRC14 None; TRGSRC15 PWM2 Trigger1; 
    ADTRIG3H = 0x600;
    //TRGSRC16 None; TRGSRC17 None; 
    ADTRIG4L = 0x0;
}

void ADC1_Deinitialize (void)
{
    uint16_t dummy; //buffers has to be read before clearing interrupt flags
    ADCON1Lbits.ADON = 0;
    
    dummy = ADCBUF2;
    IFS5bits.ADCAN2IF = 0;
    IEC5bits.ADCAN2IE = 0;
    
    dummy = ADCBUF7;
    IFS6bits.ADCAN7IF = 0;
    IEC6bits.ADCAN7IE = 0;
    
    dummy = ADCBUF12;
    IFS6bits.ADCAN12IF = 0;
    IEC6bits.ADCAN12IE = 0;
    
    dummy = ADCBUF15;
    IFS6bits.ADCAN15IF = 0;
    IEC6bits.ADCAN15IE = 0;
    
    ADCON1L = 0x0;
    ADCON1H = 0x60;
    ADCON2L = 0x0;
    ADCON2H = 0x0;
    ADCON3L = 0x0;
    ADCON3H = 0x0;
    ADMOD0L = 0x0;
    ADMOD0H = 0x0;
    ADMOD1L = 0x0;
    ADIEL = 0x0;
    ADIEH = 0x0;
    ADSTATL = 0x0;
    ADSTATH = 0x0;
    ADCMP0ENL = 0x0;
    ADCMP1ENL = 0x0;
    ADCMP2ENL = 0x0;
    ADCMP3ENL = 0x0;
    ADCMP0ENH = 0x0;
    ADCMP1ENH = 0x0;
    ADCMP2ENH = 0x0;
    ADCMP3ENH = 0x0;
    ADCMP0LO = 0x0;
    ADCMP1LO = 0x0;
    ADCMP2LO = 0x0;
    ADCMP3LO = 0x0;
    ADCMP0HI = 0x0;
    ADCMP1HI = 0x0;
    ADCMP2HI = 0x0;
    ADCMP3HI = 0x0;
    ADFL0DAT = 0x0;
    ADFL1DAT = 0x0;
    ADFL2DAT = 0x0;
    ADFL3DAT = 0x0;
    ADFL0CON = 0x0;
    ADFL1CON = 0x0;
    ADFL2CON = 0x0;
    ADFL3CON = 0x0;
    ADTRIG0L = 0x0;
    ADTRIG0H = 0x0;
    ADTRIG1L = 0x0;
    ADTRIG1H = 0x0;
    ADTRIG2L = 0x0;
    ADTRIG2H = 0x0;
    ADTRIG3L = 0x0;
    ADTRIG3H = 0x0;
    ADTRIG4L = 0x0;
    ADCMP0CON = 0x0;
    ADCMP1CON = 0x0;
    ADCMP2CON = 0x0;
    ADCMP3CON = 0x0;
    ADLVLTRGL = 0x0;
    ADLVLTRGH = 0x0;
    ADEIEL = 0x0;
    ADEIEH = 0x0;
    ADEISTATL = 0x0;
    ADEISTATH = 0x0;
    ADCON5L = 0x0;
    ADCON5H = 0x0;
    ADCBUF0 = 0x0;
    ADCBUF1 = 0x0;
    ADCBUF2 = 0x0;
    ADCBUF3 = 0x0;
    ADCBUF4 = 0x0;
    ADCBUF5 = 0x0;
    ADCBUF6 = 0x0;
    ADCBUF7 = 0x0;
    ADCBUF8 = 0x0;
    ADCBUF9 = 0x0;
    ADCBUF10 = 0x0;
    ADCBUF11 = 0x0;
    ADCBUF12 = 0x0;
    ADCBUF13 = 0x0;
    ADCBUF14 = 0x0;
    ADCBUF15 = 0x0;
    ADCBUF16 = 0x0;
    ADCBUF17 = 0x0;
}


void ADC1_SharedCorePowerEnable (void) 
{
    ADCON5Lbits.SHRPWR = 1;   
    while(ADCON5Lbits.SHRRDY == 0)
    {
    }
    ADCON3Hbits.SHREN = 1;   
}

static uint16_t ADC1_TriggerSourceValueGet(enum ADC_PWM_INSTANCE pwmInstance, enum ADC_PWM_TRIGGERS triggerNumber)
{
    uint16_t adcTriggerSourceValue = 0x0U;
    switch(pwmInstance)
    {
        case 1:
                if(triggerNumber == ADC_PWM_TRIGGER_1)
                {
                    adcTriggerSourceValue = PWM1_TRIGGER1;
                }
                else if(triggerNumber == ADC_PWM_TRIGGER_2)
                {
                    adcTriggerSourceValue = PWM1_TRIGGER2;
                }
                break;
        case 2:
                if(triggerNumber == ADC_PWM_TRIGGER_1)
                {
                    adcTriggerSourceValue = PWM2_TRIGGER1;
                }
                else if(triggerNumber == ADC_PWM_TRIGGER_2)
                {
                    adcTriggerSourceValue = PWM2_TRIGGER2;
                }
                break;
        case 3:
                if(triggerNumber == ADC_PWM_TRIGGER_1)
                {
                    adcTriggerSourceValue = PWM3_TRIGGER1;
                }
                else if(triggerNumber == ADC_PWM_TRIGGER_2)
                {
                    adcTriggerSourceValue = PWM3_TRIGGER2;
                }
                break;
        case 4:
                if(triggerNumber == ADC_PWM_TRIGGER_1)
                {
                    adcTriggerSourceValue = PWM4_TRIGGER1;
                }
                else if(triggerNumber == ADC_PWM_TRIGGER_2)
                {
                    adcTriggerSourceValue = PWM4_TRIGGER2;
                }
                break;
         default:
                break;
    }
    return adcTriggerSourceValue;
}

void ADC1_PWMTriggerSourceSet(enum ADC_CHANNEL channel, enum ADC_PWM_INSTANCE pwmInstance, enum ADC_PWM_TRIGGERS triggerNumber)
{
    uint16_t adcTriggerValue;
    adcTriggerValue= ADC1_TriggerSourceValueGet(pwmInstance, triggerNumber);
    switch(channel)
    {
        case Channel_AN2:
                ADTRIG0Hbits.TRGSRC2 = adcTriggerValue;
                break;
        case Channel_AN7:
                ADTRIG1Hbits.TRGSRC7 = adcTriggerValue;
                break;
        case Channel_AN12:
                ADTRIG3Lbits.TRGSRC12 = adcTriggerValue;
                break;
        case Channel_AN15:
                ADTRIG3Hbits.TRGSRC15 = adcTriggerValue;
                break;
        default:
                break;
    }
}

void ADC1_CommonCallbackRegister(void(*callback)(void))
{
    if(NULL != callback)
    {
        ADC1_CommonHandler = callback;
    }
}

void __attribute__ ((weak)) ADC1_CommonCallback (void)
{ 

} 


void __attribute__ ( ( __interrupt__ , auto_psv, weak ) ) _ADCInterrupt ( void )
{
    uint16_t adcVal;
    if(NULL != ADC1_CommonHandler)
    {
        (*ADC1_CommonHandler)();
    }
    
    if(IFS5bits.ADCAN2IF == 1)
    {
        //Read the ADC value from the ADCBUF before clearing interrupt
        adcVal = ADCBUF2;
        if(NULL != ADC1_ChannelHandler)
        {
            (*ADC1_ChannelHandler)(Channel_AN2, adcVal);
        }
        IFS5bits.ADCAN2IF = 0;
    }
    if(IFS6bits.ADCAN7IF == 1)
    {
        //Read the ADC value from the ADCBUF before clearing interrupt
        adcVal = ADCBUF7;
        if(NULL != ADC1_ChannelHandler)
        {
            (*ADC1_ChannelHandler)(Channel_AN7, adcVal);
        }
        IFS6bits.ADCAN7IF = 0;
    }
    if(IFS6bits.ADCAN12IF == 1)
    {
        //Read the ADC value from the ADCBUF before clearing interrupt
        adcVal = ADCBUF12;
        if(NULL != ADC1_ChannelHandler)
        {
            (*ADC1_ChannelHandler)(Channel_AN12, adcVal);
        }
        IFS6bits.ADCAN12IF = 0;
    }
    if(IFS6bits.ADCAN15IF == 1)
    {
        //Read the ADC value from the ADCBUF before clearing interrupt
        adcVal = ADCBUF15;
        if(NULL != ADC1_ChannelHandler)
        {
            (*ADC1_ChannelHandler)(Channel_AN15, adcVal);
        }
        IFS6bits.ADCAN15IF = 0;
    }
        
    // clear the ADC1 interrupt flag
    IFS5bits.ADCIF = 0;
}

void __attribute__ ((weak)) ADC1_Tasks ( void )
{
    if(IFS5bits.ADCIF == 1)
    {
        if(NULL != ADC1_CommonHandler)
        {
            (*ADC1_CommonHandler)();
        }

        // clear the ADC1 interrupt flag
        IFS5bits.ADCIF = 0;
    }
}

void ADC1_ChannelCallbackRegister(void(*callback)(enum ADC_CHANNEL channel, uint16_t adcVal))
{
    if(NULL != callback)
    {
        ADC1_ChannelHandler = callback;
    }
}

void __attribute__ ((weak)) ADC1_ChannelCallback (enum ADC_CHANNEL channel, uint16_t adcVal)
{ 
    (void)channel;
    (void)adcVal;
} 


void __attribute__ ( ( __interrupt__ , auto_psv, weak ) ) _ADCAN2Interrupt ( void )
{
    uint16_t valChannel_AN2;
    //Read the ADC value from the ADCBUF
    valChannel_AN2 = ADCBUF2;

    if(NULL != ADC1_ChannelHandler)
    {
        (*ADC1_ChannelHandler)(Channel_AN2, valChannel_AN2);
    }

    //clear the Channel_AN2 interrupt flag
    IFS5bits.ADCAN2IF = 0;
}

void __attribute__ ( ( __interrupt__ , auto_psv, weak ) ) _ADCAN7Interrupt ( void )
{
    uint16_t valChannel_AN7;
    //Read the ADC value from the ADCBUF
    valChannel_AN7 = ADCBUF7;

    if(NULL != ADC1_ChannelHandler)
    {
        (*ADC1_ChannelHandler)(Channel_AN7, valChannel_AN7);
    }

    //clear the Channel_AN7 interrupt flag
    IFS6bits.ADCAN7IF = 0;
}

void __attribute__ ( ( __interrupt__ , auto_psv, weak ) ) _ADCAN12Interrupt ( void )
{
    uint16_t valChannel_AN12;
    //Read the ADC value from the ADCBUF
    valChannel_AN12 = ADCBUF12;

    if(NULL != ADC1_ChannelHandler)
    {
        (*ADC1_ChannelHandler)(Channel_AN12, valChannel_AN12);
    }

    //clear the Channel_AN12 interrupt flag
    IFS6bits.ADCAN12IF = 0;
}

void __attribute__ ( ( __interrupt__ , auto_psv, weak ) ) _ADCAN15Interrupt ( void )
{
    uint16_t valChannel_AN15;
    //Read the ADC value from the ADCBUF
    valChannel_AN15 = ADCBUF15;

    if(NULL != ADC1_ChannelHandler)
    {
        (*ADC1_ChannelHandler)(Channel_AN15, valChannel_AN15);
    }

    //clear the Channel_AN15 interrupt flag
    IFS6bits.ADCAN15IF = 0;
}



void __attribute__ ((weak)) ADC1_ChannelTasks (enum ADC_CHANNEL channel)
{
    uint16_t adcVal;
    
    switch(channel)
    {   
        case Channel_AN2:
            if((bool)ADSTATLbits.AN2RDY == 1)
            {
                //Read the ADC value from the ADCBUF
                adcVal = ADCBUF2;

                if(NULL != ADC1_ChannelHandler)
                {
                    (*ADC1_ChannelHandler)(channel, adcVal);
                }
            }
            break;
        case Channel_AN7:
            if((bool)ADSTATLbits.AN7RDY == 1)
            {
                //Read the ADC value from the ADCBUF
                adcVal = ADCBUF7;

                if(NULL != ADC1_ChannelHandler)
                {
                    (*ADC1_ChannelHandler)(channel, adcVal);
                }
            }
            break;
        case Channel_AN12:
            if((bool)ADSTATLbits.AN12RDY == 1)
            {
                //Read the ADC value from the ADCBUF
                adcVal = ADCBUF12;

                if(NULL != ADC1_ChannelHandler)
                {
                    (*ADC1_ChannelHandler)(channel, adcVal);
                }
            }
            break;
        case Channel_AN15:
            if((bool)ADSTATLbits.AN15RDY == 1)
            {
                //Read the ADC value from the ADCBUF
                adcVal = ADCBUF15;

                if(NULL != ADC1_ChannelHandler)
                {
                    (*ADC1_ChannelHandler)(channel, adcVal);
                }
            }
            break;
        default:
            break;
    }            
}

void ADC1_ComparatorCallbackRegister(void(*callback)(enum ADC_CMP comparator))
{
    if(NULL != callback)
    {
        ADC1_ComparatorHandler = callback;
    }
}

void __attribute__ ((weak)) ADC1_ComparatorCallback (enum ADC_CMP comparator)
{ 
    (void)comparator;
} 



