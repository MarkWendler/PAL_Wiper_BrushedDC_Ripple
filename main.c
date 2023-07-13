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
#include "mcc_generated_files/X2Cscope/X2Cscope.h"
#include "mcc_generated_files/system/system.h"
#include "mcc_generated_files/mcp802x/mcp8021.h"
#include "mcc_generated_files/mcp802x/MCP802X_task.h"
#include "mcc_generated_files/timer/tmr1.h"
#include "mcc_generated_files/adc/adc1.h"
#include "uart_debug.h"

// Specify bootstrap charging time in Seconds (mention at least 10mSecs)
#define BOOTSTRAP_CHARGING_TIME_SECS 0.01
  /* Specify PWM Period in seconds, (1/ PWMFREQUENCY_HZ) */
#define LOOPTIME_SEC            0.00005

// Calculate Bootstrap charging time in number of PWM Half Cycles
#define BOOTSTRAP_CHARGING_COUNTS (uint16_t)((BOOTSTRAP_CHARGING_TIME_SECS/LOOPTIME_SEC )* 2)

// MC PWM MODULE Related Definitions
#define INVERTERA_PWM_PDC1      PG1DC
#define INVERTERA_PWM_PDC2      PG2DC
#define INVERTERA_PWM_PDC3      PG3DC
    
/* Specify PWM Period in micro seconds */
#define LOOPTIME_MICROSEC       50
/* Specify dead time in micro seconds */
#define DEADTIME_MICROSEC       1

#define FOSC_MHZ 20000000

// loop time in terms of PWM clock period
#define LOOPTIME_TCY            (uint16_t)(((LOOPTIME_MICROSEC*FOSC_MHZ)/2)-1)
       
#define DDEADTIME               (uint16_t)(DEADTIME_MICROSEC*FOSC_MHZ)

#define MCP802X_ENABLE_SetHigh()          (_LATC13 = 1)
#define MCP802X_ENABLE_SetLow()           (_LATC13 = 0)

#define MAX_BUF  1750 //3500

MCP802X_STATE mcp8021_CommState;

#define FCY 200000000
#include <libpic30.h>
#include <stdio.h>
/*
    Main application
*/
void PWMInitialization2(void);
void ChargeBootstarpCapacitors(void);
volatile uint16_t faultHappen = 0;
volatile uint16_t mcp802x_warnings = 0;
volatile uint16_t mcp802x_mode = 0;

volatile uint16_t sum = 0;
volatile uint16_t avg = 0;
volatile uint16_t cnt_speed = 0;
volatile uint16_t cnt_1sec = 20000;

extern uint16_t adcVal;
//volatile float current = 1;
volatile int16_t current = 1;

volatile uint16_t current_buff[MAX_BUF];
volatile uint16_t output_buff[MAX_BUF];
volatile uint16_t index_acq = 0;
volatile uint16_t index_send_data = 0;
volatile uint16_t start_acq = 0; 
volatile uint16_t start_send_data = 0; 
volatile uint16_t prev_io_state = 0; 
volatile uint16_t debounce = 0; 

volatile uint16_t speed_cnt = 0; 
volatile uint16_t speed_cnt_2 = 0; 
volatile uint16_t current_min = 4095; 
volatile uint16_t current_max = 0;
volatile uint16_t current_pp = 4095; 
volatile uint16_t speed = 0; 
volatile uint16_t ripple = 0; 
volatile uint16_t hyst_thresh_hi = 0; 
volatile uint16_t hyst_thresh_lo = 0; 
volatile uint16_t cmp_output = 0; 
volatile uint16_t new_speed_flag = 0; 

void print_CommState( MCP802X_STATE CommState )
{

    switch(CommState)
    {
        case MCP802X_UNINITIALIZED:
            MAIN_PRINTF("CommState = UNINITIALIZED" );
            break;
        case MCP802X_INITIALIZATION:
            MAIN_PRINTF("CommState = INITIALIZATION" );
            break;
        case MCP802X_READY:
            MAIN_PRINTF("nCommState = READY" );
            break;
        case MCP802X_ERROR:
            MAIN_PRINTF("nCommState = ERROR" );
            break;
        default:
        {
            MAIN_PRINTF("CommState = UNKNOWN" );
        }
    }
}
    
void mcp8x_clear_fault()
{
    MCP802X_ENABLE_SetLow(); //disable the gate driver 
    __delay_ms(1);
    MCP802X_ENABLE_SetHigh(); //enable the gate driver 
}

void start_acquisition()
{
    if(start_acq == 0)
    {
        printf("\r\n START ACQUISITION \r\n");
        start_send_data = 0;
        index_acq = 0;
        start_acq = 1;
    }
}
void acq_send_data()
{
    if((start_send_data == 1) && (index_send_data < MAX_BUF))
    {
        CURRENT_PRINTF_VAR("%d; ", index_send_data);
        CURRENT_PRINTF_VAR("%d; ", current_buff[ index_send_data ]);
        CURRENT_PRINTF_VAR("%d;\r\n", output_buff[ index_send_data++ ]);
    }
    else
    {
        start_send_data = 0;
        if(new_speed_flag)
        {
            CURRENT_PRINTF_VAR("Speed = %d\r\n", speed);
            new_speed_flag = 0;
        }
    }
}

void mcp8021_service()
{
    //mcp8021_CommState = MCP802X_eHandleCommunication();

    MAIN_PRINTF("\r\n");
    print_CommState( mcp8021_CommState );

    if ( PORTDbits.RD1 == 0 )
    {
        MAIN_PRINTF(" fault_pin = FAULT" );
    }
    else
    {
        MAIN_PRINTF(" fault_pin = no fault" );
    }

    faultHappen = MCP802X_u16GetLastMcpFaults();

    if (faultHappen != 0)
    {
        MAIN_PRINTF_VAR(" FAULT = %04X", faultHappen );
    }
    mcp802x_warnings = MCP802X_u16GetWarnings();
    MAIN_PRINTF_VAR(" WARNINGS = %04X", mcp802x_warnings );

    mcp802x_mode = MCP802X_sGetMode();
    MAIN_PRINTF_VAR(" MODE = %04X", mcp802x_mode );

    if ( PORTDbits.RD1 == 0 ) // read FAULT pin
    {
        // button pressed
        mcp8x_clear_fault();
        MAIN_PRINTF("  CLEAR FAULT  " );
    }
//    SPEED_PRINTF_VAR("speed = %d ", speed );
//    SPEED_PRINTF_VAR("duty = %d ", PG1DC );
//    SPEED_PRINTF_VAR("current = %d ", current );
//    SPEED_PRINTF_VAR("ripple = %d ", ripple );
//    SPEED_PRINTF_VAR("counter 2 = %d ", speed_cnt_2 );
////    SPEED_PRINTF_VAR("The counter 1 is %d ", speed_cnt);
//    SPEED_PRINTF_VAR("diff =  %d ", current_pp );
//    SPEED_PRINTF_VAR("min = %d ", current_min );
//    SPEED_PRINTF_VAR("max = %d ", current_max );
//    SPEED_PRINTF("\r\n");
////    SPEED_PRINTF("\r\n");
    
}

void change_direction()
{
    //disable PWM generators
        PG1CONLbits.ON = 0;
        PG2CONLbits.ON = 0;
    
        __delay_ms(150);
    //change the mode of the operation    
        PG1IOCONL = PG1IOCONL ^ PG2IOCONL;
        PG2IOCONL = PG1IOCONL ^ PG2IOCONL;
        PG1IOCONL = PG1IOCONL ^ PG2IOCONL;
        
        __delay_ms(150);
    //enable PWM generators
        PG1CONLbits.ON = 1;
        PG2CONLbits.ON = 1;       
        
        MAIN_PRINTF(" DIRECTION CHANGED " );
}

int main(void)
{
    SYSTEM_Initialize();
    __delay_ms(300);
    __delay_ms(300);
    __delay_ms(300);

 
    printf("\r\n *** START ***** \r\n");
    __delay_ms(100);
    
    MCP802X_u8ConfigSet(config1);
    //ChargeBootstarpCapacitors();
    MCP802X_ENABLE_SetHigh(); //enable the gate driver 
    
    do
    {
        mcp8021_service();
        if ( PORTDbits.RD13 == 0 ) // read SW2
        {
        // button pressed
            change_direction();
        }
        
        if ( PORTDbits.RD8 == 0 && prev_io_state == 1 && debounce == 0) // read SW1
        {
        // button pressed
            
            start_acquisition();
            
            prev_io_state = 0;
            debounce = 100;
        }
        else
        {
            prev_io_state = 1;
            if(debounce>0)
            {
                debounce--;
            }
        }
    
        acq_send_data();
    
        //MAIN_PRINTF_VAR("\r\n Motor current = %d\r\n", current);
        //MAIN_PRINTF("\r\n Motor crt = ");
        //printf(0xAA);
        //MAIN_PRINTF_VAR("%i", current);
        //printf(0x55);
        //MAIN_PRINTF("\r\n");
        //MAIN_PRINTF_VAR("\r\n Motor current = %04X\r\n", adcVal);
        //CURRENT_PRINTF_VAR("%d\r\n", current);
        //__delay_ms(5);
        
        //X2CScope_Update();
        
        X2CScope_Communicate();
    }
    while( mcp8021_CommState != MCP802X_READY );
    
    TMR1_Start();
    MAIN_PRINTF("\r\nCommState = MCP802X_READY" );

    //__delay_ms(50);
    
    while(1)
    {
        X2CScope_Communicate();
    }    
}


void ChargeBootstarpCapacitors(void)
{
    uint16_t i = BOOTSTRAP_CHARGING_COUNTS;
    uint16_t prevStatusCAHALF = 0,currStatusCAHALF = 0;
    uint16_t k = 0;
    
    // Enable PWMs only on PWMxL ,to charge bootstrap capacitors at the beginning
    // Hence PWMxH is over-ridden to "LOW"
    PG3IOCONLbits.OVRDAT = 0;  // 0b00 = State for PWM3H,L, if Override is Enabled
    PG2IOCONLbits.OVRDAT = 0;  // 0b00 = State for PWM2H,L, if Override is Enabled
    PG1IOCONLbits.OVRDAT = 0;  // 0b00 = State for PWM1H,L, if Override is Enabled

    PG3IOCONLbits.OVRENH = 1;  // 1 = OVRDAT<1> provides data for output on PWM3H
    PG2IOCONLbits.OVRENH = 1;  // 1 = OVRDAT<1> provides data for output on PWM2H
    PG1IOCONLbits.OVRENH = 1;  // 1 = OVRDAT<1> provides data for output on PWM1H

    PG3IOCONLbits.OVRENL = 1;  // 1 = OVRDAT<0> provides data for output on PWM3L
    PG2IOCONLbits.OVRENL = 1;  // 1 = OVRDAT<0> provides data for output on PWM2L
    PG1IOCONLbits.OVRENL = 1;  // 1 = OVRDAT<0> provides data for output on PWM1L

    // PDCx: PWMx GENERATOR DUTY CYCLE REGISTER
    // Initialize the PWM duty cycle for charging
    INVERTERA_PWM_PDC3 = LOOPTIME_TCY - (DDEADTIME/2 + 5);
    INVERTERA_PWM_PDC2 = LOOPTIME_TCY - (DDEADTIME/2 + 5);
    INVERTERA_PWM_PDC1 = LOOPTIME_TCY - (DDEADTIME/2 + 5);
    
    MAIN_PRINTF("start\r\n");
    
    while(i)
    {
        //driver status
        mcp8021_CommState = MCP802X_eHandleCommunication(); //call in a low priority timer interrupt
        MAIN_PRINTF_VAR("status= %04X\r\n", mcp8021_CommState);
        
        prevStatusCAHALF = currStatusCAHALF;
        currStatusCAHALF = PG3STATbits.CAHALF;
        if (prevStatusCAHALF != currStatusCAHALF)
        {
            if (currStatusCAHALF == 0)
            {
                i--; 
                k++;
                if (i == (BOOTSTRAP_CHARGING_COUNTS - 50))
                {
                    // 0 = PWM generator provides data for PWM1L pin
                    PG1IOCONLbits.OVRENL = 0;
                }
                else if (i == (BOOTSTRAP_CHARGING_COUNTS - 150))
                {
                    // 0 = PWM generator provides data for PWM2L pin
                    PG2IOCONLbits.OVRENL = 0;  
                }
                else if (i == (BOOTSTRAP_CHARGING_COUNTS - 250))
                {
                    // 0 = PWM generator provides data for PWM3L pin
                    PG3IOCONLbits.OVRENL = 0;  
                }
                if (k > 25)
                {
                    if (PG3IOCONLbits.OVRENL == 0)
                    {
                        if (INVERTERA_PWM_PDC3 > 2)
                        {
                            INVERTERA_PWM_PDC3 -= 2;
                        }
                        else
                        {
                           INVERTERA_PWM_PDC3 = 0; 
                        }
                    }
                    if (PG2IOCONLbits.OVRENL == 0)
                    {
                        if (INVERTERA_PWM_PDC2 > 2)
                        {
                            INVERTERA_PWM_PDC2 -= 2;
                        }
                        else
                        {
                            INVERTERA_PWM_PDC2 = 0; 
                        }
                    }
                    if (PG1IOCONLbits.OVRENL == 0)
                    {
                        if (INVERTERA_PWM_PDC1 > 2)
                        {
                            INVERTERA_PWM_PDC1 -= 2;
                        }
                        else
                        {
                            INVERTERA_PWM_PDC1 = 0; 
                        }
                    }
                    k = 0;
                } 
            }
        }
    }
    // PDCx: PWMx GENERATOR DUTY CYCLE REGISTER
    // Initialize the PWM duty cycle for charging
    INVERTERA_PWM_PDC3 = 0;
    INVERTERA_PWM_PDC2 = 0;
    INVERTERA_PWM_PDC1 = 0;

    PG1IOCONLbits.OVRENH = 1;  // 0 = PWM generator provides data for PWM1H pin
    PG1IOCONLbits.OVRDAT = 1;  // 0b00 = State for PWM3H,L, if Override is Enabled
    PG2IOCONLbits.OVRENH = 0;  // 0 = PWM generator provides data for PWM1H pin
    PG2IOCONLbits.OVRDAT = 0;  // 0b00 = State for PWM2H,L, if Override is Enabled
    //PG2IOCONLbits.OVRDAT = 0;  // 0b00 = State for PWM3H,L, if Override is Enabled
}

void ADC1_ChannelCallback(enum ADC_CHANNEL channel, uint16_t adcVal)
{
    if(channel == Channel_AN15)
    {
        PG1DC = adcVal + 500;
        PG2DC = adcVal + 500;
//        PG1DC = 1500;
//        PG2DC = 1500;
//        LATAbits.LATA3 = ~LATAbits.LATA3;
    }

    if(channel == Channel_AN7)
    {
        LATBbits.LATB0 = 1;
        //current = ((((float)adcVal - 2048)/4096)*3.3)*100;
        current = adcVal;
        //PG2TRIGA = 4500;//PG2DC * 1.5;
        PG2TRIGB = PG2PER;
        PG2TRIGA = 500;//0;
        //LATAbits.LATA3 = ~LATAbits.LATA3;
        //CURRENT_PRINTF_VAR("%d/n", current);
        //CURRENT_PRINTF("A");
        
        if((start_acq==1) && (index_acq < MAX_BUF))
        {
            current_buff[index_acq] =  current;
            output_buff[index_acq] =  cmp_output;
        
            index_acq++;
        }
        else
        {
            if(start_acq)
            {
                index_acq = 0;
                start_acq = 0;
                index_send_data = 0;
                start_send_data = 1;
            }
        }
        
//        sum += current;
//        sum -= avg;
//        avg = sum >> 4;
//        cnt_1sec--;
//        
//        if(current > avg)
//        {
//            //LATBbits.LATB0 = 1;
//            cnt_speed++;
//        }
//        else
//        {
//            //LATBbits.LATB0 = 0;
//        }
//        if(cnt_1sec == 0)
//        {
//            speed = cnt_speed;
//            cnt_speed = 0;
//            MAIN_PRINTF_VAR("/r/n Speed is = %d", speed);
//            MAIN_PRINTF_VAR(" Current average is = %d/r/n", avg);
//            cnt_1sec = 20000;
//        }
        
        if(speed_cnt < 80) //verify if the first loop is done
        {
            speed_cnt++;
            if(current < current_min)
            {
                current_min = current;
            }
            
            if(current > current_max)
            {
                current_max = current;
            }            
            
            if((current > hyst_thresh_hi) && cmp_output == 0) 
            {
                //ripple++;
                cmp_output = 1; //reset the state of the comparator
            }
            
            if(current < hyst_thresh_lo && cmp_output == 1)
            {
                ripple++;
                cmp_output = 0;
            }
            
        }
        else
        {
            speed_cnt_2++;
            current_pp = current_max - current_min;
            
            hyst_thresh_hi = 32*current_pp;
            hyst_thresh_hi = hyst_thresh_hi/40;
            hyst_thresh_hi = hyst_thresh_hi + current_min;
            
            hyst_thresh_lo = current_pp;
            hyst_thresh_lo = hyst_thresh_lo/4;
            hyst_thresh_lo = hyst_thresh_lo + current_min;
            
            speed_cnt = 0;           
            //ripple = 0;
            current_max = 0;
            current_min = 4095;
            cmp_output = 0;
        }
        
        if(speed_cnt_2 == 25) //verify if the second loop is done
        {
            speed = ripple;
            ripple = 0;
            speed_cnt_2 = 0;
            current_pp = 4095; //reset the peak-to-peak value of the current
            new_speed_flag = 1;
        }

        
        //X2CScope_Update();
        
        LATBbits.LATB0 = 0;

    }
}
