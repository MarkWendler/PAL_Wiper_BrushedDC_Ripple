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
#include "mcc_generated_files/system/system.h"
#include "mcc_generated_files/mcp802x/MCP802X_task.h"
#include "mcc_generated_files/timer/tmr1.h"
#include "mcc_generated_files/system/pins.h"
#include "mcc_generated_files/adc/adc1.h"
#include "mcc_generated_files/pwm_hs/pwm.h"
#include "mcc_generated_files/uart/uart1.h"
#include "mcc_generated_files/i2c_host/i2c1.h"
#include "mcc_generated_files/i2c_host/i2c_host_types.h"
#include "X2Cscope/X2Cscope.h"
#include "UART_debug.h"

void User_ADC1_ChannelCallback(enum ADC_CHANNEL channel, uint16_t adcVal);

#define HYSTERESIS_UPDATE_INTERVAL   80
#define SPEED_MEASUREMENT_INTERVAL   25


#define MAX_DUTY_CYCLE               4500
#define MIN_DUTY_CYCLE               500

//uint16_t postscaler = POSTSCALER;

#define FCY 200000000  // System Clock
#include <libpic30.h>
#include <stdio.h>

//volatile uint16_t faultHappen = 0;
//volatile uint16_t mcp802x_warnings = 0;
//volatile uint16_t mcp802x_mode = 0;

//volatile uint16_t sum = 0;
//volatile uint16_t avg = 0;
//volatile uint16_t cnt_speed = 0;
//volatile uint16_t cnt_1sec = 20000;


//volatile uint16_t index_acq = 0;
//volatile uint16_t index_send_data = 0;
//volatile uint16_t start_acq = 0; 
//volatile uint16_t start_send_data = 0; 
//volatile uint16_t prev_io_state = 0; 
//volatile uint16_t debounce = 0; 

//--------- speed measurement variables --------------------
volatile uint16_t hysteresis_update_counter = 0; 
volatile uint16_t speed_measurement_counter = 0; 
volatile uint16_t current_min = 4095; 
volatile uint16_t current_max = 0;
volatile uint16_t current_pp = 4095; 
volatile uint16_t ripple_counter = 0; 
volatile uint16_t hyst_thresh_hi = 0; 
volatile uint16_t hyst_thresh_lo = 0; 
volatile uint16_t digital_comparator_output = 0; 
volatile uint16_t new_speed_flag = 0;

//--------- gate driver status reading implementation --------------------
volatile MCP802X_STATE              mcp8021_CommState;
volatile MCP802X_STATE              mcp8021_communicate_status;
         MCP802X_POWER_STATUS_FLAGS mcp8021_power_status;
volatile bool                       mcp8021_power_status_event;
volatile uint16_t                   mcp8021_faults = 0;
volatile uint16_t                   mcp8021_warnings;
volatile uint16_t                   mcp8021_fault_pin_state = 0; 
uint8_t u8PowerStatus;
extern uint16_t adcVal;

volatile uint16_t dir_cmd = 0;
volatile uint16_t dir_old = 0;
volatile uint16_t start_old = 0;
volatile uint16_t start_cmd = 0;

volatile uint16_t cmd_motor_speed_rpm = 0;
volatile uint16_t cmd_motor_direction = 0;
volatile uint16_t cmd_motor_stop      = 0;

volatile uint16_t motor_current                = 0;
volatile uint16_t motor_speed                  = 0; 
volatile uint16_t cmd_motor_voltage_duty_cycle = 0; 
volatile uint16_t motor_voltage_duty_cycle     = 0; 
volatile uint16_t motor_voltage_slope_counter  = 0; 

volatile uint16_t DC_bus_voltage       = 0;
volatile uint16_t Speed1_digital_input = 0;
volatile uint16_t Brake_digital_input  = 0;

volatile uint16_t new_duty_cycle = 0;

volatile uint16_t old_direction = 0;
volatile uint16_t is_motor_run = 0;

uint16_t prev_io_state = 1;
uint16_t io_debounce = 0;
uint16_t test_speed = 0;
uint16_t test_direction = 0;

// 1 milliseconds task variables
#define TASK_EXECUTION_PERIOD          20 // 20 * PWM_PERIOD = 20 * 50us = 1 ms
volatile uint16_t task_counter       = TASK_EXECUTION_PERIOD;
volatile uint16_t task_execute_flag  = 0;


//--------- gate driver status reading implementation --------------------

volatile uint16_t timer_ms_counter = 0;

uint16_t device_addr  = 0x35;
uint8_t write_data[1]   = {0x13}; 
uint8_t read_data[1]    = {0x00};

bool IsSensorReadFinished   = false;
bool IsSensorReadInProgress = false;

uint32_t sysClk = FCY;

#define START_TIMER_MS(tmr_counter, x) tmr_counter = x;
#define IS_TIMER_MS_ELAPSED(tmr_counter) (tmr_counter == 0)


void mcp8x_clear_fault()
{
    MCP802X_ENABLE_SetLow(); //disable the gate driver 
    __delay_ms(1);
    MCP802X_ENABLE_SetHigh(); //enable the gate driver 
}

//void change_direction()
//{
//    //disable PWM generators
//        PG1CONLbits.ON = 0;
//        PG2CONLbits.ON = 0;
//    
//        __delay_us(10);
//    //change the mode of the operation    
//        PG1IOCONL = PG1IOCONL ^ PG2IOCONL;
//        PG2IOCONL = PG1IOCONL ^ PG2IOCONL;
//        PG1IOCONL = PG1IOCONL ^ PG2IOCONL;
//        
//        __delay_us(10);
//    //enable PWM generators
//        PG1CONLbits.ON = 1;
//        PG2CONLbits.ON = 1;       
//}
void pwm_stop( void )
{
    PG1CONLbits.ON = 0;
    PG2CONLbits.ON = 0;

    __delay_us(200);
}

void pwm_change_direction( uint16_t new_direction )
{
    if ( old_direction != new_direction )
    {
        PG1CONLbits.ON = 0;
        PG2CONLbits.ON = 0;

        __delay_us(200);
        
        if( new_direction == 0 )
        {
            PG1IOCONLbits.OVRENH = 0;
            PG1IOCONLbits.OVRENL = 0;

            PG2IOCONLbits.OVRENH = 1;
            PG2IOCONLbits.OVRENL = 1;     
        }
        else
        {
            PG1IOCONLbits.OVRENH = 1;
            PG1IOCONLbits.OVRENL = 1;

            PG2IOCONLbits.OVRENH = 0;
            PG2IOCONLbits.OVRENL = 0;      
        }
        
        __delay_us(200);

        PG1CONLbits.ON = 1;
        PG2CONLbits.ON = 1;
    }
    
    old_direction = new_direction;
}

void motor_command( uint16_t speed_rpm, uint16_t direction)
{
    new_duty_cycle = speed_rpm;  // scaling 1 to 1 (4500 rpm = 4500 duty)
    
    if( new_duty_cycle >= 0)
    {
        PG1CONLbits.ON = 1;
        PG2CONLbits.ON = 1;
        
        is_motor_run = 1;
        
        if( new_duty_cycle > MAX_DUTY_CYCLE )
        {
            cmd_motor_voltage_duty_cycle = MAX_DUTY_CYCLE;
        }
        else
        {
            if( new_duty_cycle < MIN_DUTY_CYCLE )
                cmd_motor_voltage_duty_cycle = MIN_DUTY_CYCLE;
            else
                cmd_motor_voltage_duty_cycle = new_duty_cycle;
        }
        
        
//        if( direction == 1 && old_direction != direction)
//        {
//            PG1CONLbits.ON = 0;
//            PG2CONLbits.ON = 0;
//
//            __delay_us(200);
//        
//            PG1IOCONLbits.OVRENH = 0;
//            PG1IOCONLbits.OVRENL = 0;
//
//            PG2IOCONLbits.OVRENH = 1;
//            PG2IOCONLbits.OVRENL = 1;     
//            
//            __delay_us(200);
//            
//            PG1CONLbits.ON = 1;
//            PG2CONLbits.ON = 1;
//        }
//        if( direction == 0 && old_direction != direction)
//        {
//            PG1CONLbits.ON = 0;
//            PG2CONLbits.ON = 0;
//
//            __delay_us(200);
//        
//            PG1IOCONLbits.OVRENH = 1;
//            PG1IOCONLbits.OVRENL = 1;
//
//            PG2IOCONLbits.OVRENH = 0;
//            PG2IOCONLbits.OVRENL = 0;      
//            
//            __delay_us(200);
//            
//            PG1CONLbits.ON = 1;
//            PG2CONLbits.ON = 1;
//        }
//        __delay_us(200);
       
    }
    else
    {
        PG1CONLbits.ON = 0;
        PG2CONLbits.ON = 0;

        __delay_us(200);        
    }
//    old_direction = direction;
}

void app_task_1ms()
{
//    #if (RUN_ON_EVAL_BOARD == 1)
//            LATBbits.LATB0 = 1;
//    #endif

    mcp8021_communicate_status = MCP802X_eHandleCommunication(); 
    
    //mcp8021_power_status = MCP802X_u16GetLastMcpFaults();
    mcp8021_power_status_event = MCP802X_PowerStatusGet( &mcp8021_power_status );
    
    if(mcp8021_faults == 0 ) //do not override the gate drive fault, store the first error
    { 
        mcp8021_faults = MCP802X_u16GetLastMcpFaults();
    }
    
    mcp8021_warnings = MCP802X_u16GetWarnings();

    //motor_command( 1000, 0 );
    
    if ( PORTDbits.RD8 == 0 )
    {
        // sw1 button pressed
        if ( prev_io_state == 0 )
        {
            if ( io_debounce == 0 )
            {
                test_speed += 1000;

                if( test_speed > 4500 )
                {
                    test_speed = MIN_DUTY_CYCLE;
                    test_direction = !test_direction;
                }

                motor_command( test_speed, test_direction );
            }

            if( io_debounce >= 0)
                io_debounce--;
        }
        else
        {
            io_debounce = 100;
        }
        prev_io_state = 0;
    }
    else
    {
        prev_io_state = 1;
    }
    
    
//    if( (motor_voltage_slope_counter %2) == 0)
//    {
        if (motor_voltage_duty_cycle < cmd_motor_voltage_duty_cycle)
            motor_voltage_duty_cycle += 4;
        else
        {
            if (motor_voltage_duty_cycle > cmd_motor_voltage_duty_cycle)
                motor_voltage_duty_cycle -= 4;
        }
//    }
    
    motor_voltage_slope_counter++;
    
    if( motor_voltage_duty_cycle == MIN_DUTY_CYCLE )
    {
        pwm_change_direction( test_direction );
        
        if( is_motor_run )
        {
            pwm_stop();
            is_motor_run = 0;
        }
    }


    
//    switch(mcp8021_communicate_status)
//    {
//        case MCP802X_UNINITIALIZED: //!< no communication had been issued unti now
//            break;
//            
//        case MCP802X_INITIALIZATION: //!< System Initialization Stage
//            
//            if(dir_cmd == 1)
//            {
//                //dir_old = ~dir_old;
//                dir_cmd = 0;
//                //change_direction();
//                
//                PG1CONLbits.ON = 0;
//                PG2CONLbits.ON = 0;
//                
//                __delay_ms(10);
//                
//                PG1IOCONLbits.OVRENH = ~PG1IOCONLbits.OVRENH;
//                PG1IOCONLbits.OVRENL = ~PG1IOCONLbits.OVRENL;
//                
//                PG2IOCONLbits.OVRENH = ~PG2IOCONLbits.OVRENH;
//                PG2IOCONLbits.OVRENL = ~PG2IOCONLbits.OVRENL;     
//                
//                __delay_ms(10);
//                
//                PG1CONLbits.ON = start_cmd; //to prevent accidentally change in direction when the motor is stopped
//                PG2CONLbits.ON = start_cmd;
//                //change_direction();
//            }
//            
//            if(start_cmd == 1)
//            {
//                //start_old = ~start_old;
//                start_cmd = 0;
//                PG1CONLbits.ON = 1;
//                PG2CONLbits.ON = 1;
//            } 
//            
//            break;
//            
//        case MCP802X_READY:  //!< System is Ready
//            
//            if(dir_cmd == 1)
//            {
//                //dir_old = ~dir_old;
//                dir_cmd--;
//                //change_direction();
//                
//                PG1CONLbits.ON = 0;
//                PG2CONLbits.ON = 0;
//                
//                __delay_ms(10);
//                
//                PG1IOCONLbits.OVRENH = ~PG1IOCONLbits.OVRENH;
//                PG1IOCONLbits.OVRENL = ~PG1IOCONLbits.OVRENL;
//                
//                PG2IOCONLbits.OVRENH = ~PG2IOCONLbits.OVRENH;
//                PG2IOCONLbits.OVRENL = ~PG2IOCONLbits.OVRENL;     
//                
//                __delay_ms(10);
//                
//                PG1CONLbits.ON = start_cmd; //to prevent accidentally change in direction when the motor is stopped
//                PG2CONLbits.ON = start_cmd;
//                //change_direction();
//            }
//            
//            if(start_cmd == 1)
//            {
//                //start_old = ~start_old;
//                start_cmd--;
//                PG1CONLbits.ON = start_cmd;
//                PG2CONLbits.ON = start_cmd;
//            } 
//            
//            break;
//            
//        case MCP802X_ERROR:
//            break; 
//    }
    
    #if (RUN_ON_EVAL_BOARD == 1)

    #else
        Speed1_digital_input = PORTDbits.RD8; //check if the Speed1 is connected
        Brake_digital_input = PORTDbits.RD13; //check if the Brake is connected
    #endif // RUN_ON_EVAL_BOARD

//    #if (RUN_ON_EVAL_BOARD == 1)
//            LATBbits.LATB0 = 0;
//    #endif
}

struct I2C_TRANSFER_SETUP mySetup;

int main(void)
{
    
    mySetup.clkSpeed = 100000; //100kHz
    
    SYSTEM_Initialize();
    ADC1_ChannelCallbackRegister(&User_ADC1_ChannelCallback);

    
    motor_voltage_duty_cycle = MIN_DUTY_CYCLE; //set the initial duty cycle
    
    // Manually enable PWM to trigger ADC channels as MCC does not support by default
    #if (RUN_ON_EVAL_BOARD == 1)
        PWM_Trigger1Enable(PWM_GENERATOR_2,PWM_TRIGGER_COMPARE_A); 
        
//    TRISBbits.TRISB0   = 0; // B0 = output
//    ANSELBbits.ANSELB0 = 0; // B0 = digital function

    #else

    #endif // RUN_ON_EVAL_BOARD
   
    //////////////////// read the MSB temperature register from the Hall sensor       
//    I2C1_TransferSetup(&mySetup, sysClk);
    
    
    
    PG2TRIGB = PG2PER; // sampling point for the motor current at middle of PWM pulse
    PWM_Trigger2Enable(PWM_GENERATOR_2,PWM_TRIGGER_COMPARE_B); //enable the sampling point for the motor current 
    
    MCP802X_u8ConfigSet(config1); //Set the basic configuration to the gate drive - Mandatory
    MCP802X_ENABLE_SetHigh();     //enable the gate driver
    
    //while(gatedrive_communicate_status != MCP802X_READY ){} //wait until the gate driver is in the READY state

    //printf("START\r\n");

//    TRISBbits.TRISB9 = 0;
//    LATBbits.LATB9 = 1;
//    LATBbits.LATB9 = 0;

    while(1)
    {
        //LATBbits.LATB0 = 1;
        X2CScope_Communicate(); //handles the communication with the MPLAB X plugin on the PC side
        //LATBbits.LATB0 = 0;

        //////////////////// TASK - 1 ms
        if(task_execute_flag)
        {
            task_execute_flag = 0;
            app_task_1ms();
        }
        //////////////////// 

        
        //////////////////// check read
        if( IsSensorReadInProgress )
        {
            if( IsSensorReadFinished == true )
            {
                if( I2C_HOST_ERROR_NONE == I2C1_ErrorGet() )
                {
                    //printf("\r\nSensor Data: %02X", read_data[0] );
                }
                else
                {
                    //printf( "\r\nI2C error" );
                }

                IsSensorReadInProgress = false;
            }
        }
        else
        {
            IsSensorReadFinished = false;
            IsSensorReadInProgress = true;

            I2C1_Read(device_addr, read_data , 1);   
            //I2C1_WriteRead(device_addr, write_data, 1, read_data , 1);   
        }
        //////////////////// 
    }    
}


/////////////// Timer1 handles the DE2 gate driver communication
void TMR1_TimeoutCallback()
{
    
//    mcp8021_communicate_status = MCP802X_eHandleCommunication(); 
//    
//    //mcp8021_power_status = MCP802X_u16GetLastMcpFaults();
//    mcp8021_power_status_event = MCP802X_PowerStatusGet( &mcp8021_power_status );
//    
//    if(mcp8021_faults == 0 ) //do not override the gate drive fault, store the first error
//    { 
//        mcp8021_faults = MCP802X_u16GetLastMcpFaults();
//    }
//    
//    mcp8021_warnings = MCP802X_u16GetWarnings();
    
    //////////////////// TASK - 1 ms
//    task_counter--;
//    if(task_counter== 0)
//    {
        task_execute_flag = 1;
//        task_counter      = TASK_EXECUTION_PERIOD;
//    }
    //////////////////// 
    
    if( timer_ms_counter > 0 )
    {
        timer_ms_counter--;
    }
}

void User_ADC1_ChannelCallback(enum ADC_CHANNEL channel, uint16_t adcVal)
{
    LATBbits.LATB0 = 1;
    
    #if (RUN_ON_EVAL_BOARD == 1)
        if(channel == Channel_AN15)
        {
            // set duty cycle from external potentiometer value
            // scale the value from 4096 ==>> 4500 (90% duty cycle)
            //PG1DC = adcVal * 1.1;
            PG1DC = motor_voltage_duty_cycle;
            PG2DC = PG1DC;
        }       
        
        if(channel == Channel_AN12)
        {
            DC_bus_voltage = adcVal; //read the bus voltage
        }  
        
    #else

        PG1DC = motor_voltage_duty_cycle;
        PG2DC = PG1DC;
            
        if(channel == Channel_AN2)
        {
            DC_bus_voltage = adcVal; //read the bus voltage
        }  

    #endif // RUN_ON_EVAL_BOARD
    

    if(channel == Channel_AN7)
    {
        motor_current = adcVal; // read the value of the motor current
        
        #if (RUN_ON_EVAL_BOARD == 1)
//            LATBbits.LATB0 = 1; // toggle this pin to check the interrupt duration
            PG2TRIGA = 500; // trigger point for external potentiometer reading
        #else
        #endif // RUN_ON_EVAL_BOARD



        //////////////////// SPEED MEASUREMENT - START
        
        if(hysteresis_update_counter < HYSTERESIS_UPDATE_INTERVAL) //verify if the first loop is done
        {
            // Update Hysteresis Thresholds Period = HYSTERESIS_UPDATE_INTERVAL * PWM_PERIOD = 4000 us
            // Detect the minimum and maximum motor current values
            hysteresis_update_counter++;
            if(motor_current < current_min)
            {
                current_min = motor_current;
            }
            
            if(motor_current > current_max)
            {
                current_max = motor_current;
            }            
            
            if((motor_current > hyst_thresh_hi) && digital_comparator_output == 0) 
            {
                digital_comparator_output = 1; //reset the state of the comparator
            }
            
            if(motor_current < hyst_thresh_lo && digital_comparator_output == 1)
            {
                // complete pulse on comparator output
                ripple_counter++;   // count a new current ripple
                digital_comparator_output = 0;
            }
        }
        else
        {
            // Update the hysteresis thresholds
            speed_measurement_counter++;
            current_pp = current_max - current_min;
            
            hyst_thresh_hi = 32*current_pp;
            hyst_thresh_hi = hyst_thresh_hi/40;
            hyst_thresh_hi = hyst_thresh_hi + current_min;
            
            hyst_thresh_lo = current_pp;
            hyst_thresh_lo = hyst_thresh_lo/4;
            hyst_thresh_lo = hyst_thresh_lo + current_min;
            
            hysteresis_update_counter = 0;           
            //ripple = 0;
            current_max = 0;
            current_min = 4095;
            digital_comparator_output = 0;
        }
        
        if(speed_measurement_counter == SPEED_MEASUREMENT_INTERVAL) //verify if the second loop is done
        {
            // Speed Measurement Period = SPEED_MEASUREMENT_INTERVAL * HYSTERESIS_UPDATE_INTERVAL * PWM_PERIOD = 100 ms
            // Speed measurement, save the counted ripples
            motor_speed = ripple_counter;
            ripple_counter = 0;
            speed_measurement_counter = 0;
            current_pp = 4095; //reset the peak-to-peak value of the current
            new_speed_flag = 1;
        }

        //////////////////// SPEED MEASUREMENT - END
        

        X2CScope_Update(); //This is the sample point of the scope at every 50us
        
    #if (RUN_ON_EVAL_BOARD == 1)
//            LATBbits.LATB0 = 0;
    #else

    #endif // RUN_ON_EVAL_BOARD

    }
    LATBbits.LATB0 = 0;
}

void I2C1_Callback()
{
    IsSensorReadFinished = true;
}