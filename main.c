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
#include "UART_debug.h"
#include "mcc_generated_files/system/system.h"
#include "mcc_generated_files/mcp802x/MCP802X_task.h"
#include "mcc_generated_files/timer/tmr1.h"
#include "mcc_generated_files/system/pins.h"
#include "mcc_generated_files/adc/adc1.h"
#include "mcc_generated_files/pwm_hs/pwm.h"
#include "mcc_generated_files/uart/uart1.h"
#include "mcc_generated_files/i2c_host/i2c1.h"
#include "mcc_generated_files/i2c_host/i2c_host_types.h"
#if (X2CSCOPE_ENABLE == 1)
	#include "X2Cscope/X2Cscope.h"
#endif // X2CSCOPE_ENABLE

#define HYSTERESIS_UPDATE_INTERVAL   80
#define SPEED_MEASUREMENT_INTERVAL   25


#define MAX_DUTY_CYCLE               4900
#define MIN_DUTY_CYCLE               1000

#define MOTOR_CURRENT_0              2048

enum DEMO_STATE_MACHINE
{
    DEMO_STOP = 0,
    DEMO_RUN_LEFT,
    DEMO_RUN_RIGHT
};

enum MOTOR_COMMAND
{
    MOTOR_COMMAND_STOP = 0,
    MOTOR_COMMAND_RUN_LEFT,
    MOTOR_COMMAND_RUN_RIGHT
};

#define START_TIMER_MS(tmr_counter, x) tmr_counter = x;
#define IS_TIMER_MS_ELAPSED(tmr_counter) (tmr_counter == 0)


#define FCY 200000000  // System Clock
#include <libpic30.h>
#include <stdio.h>


void User_ADC1_ChannelCallback(enum ADC_CHANNEL channel, uint16_t adcVal);
void User_TMR1_TimeoutCallback();
void User_I2C1_Callback();


//--------- speed measurement variables --------------------
volatile uint16_t hysteresis_update_counter = 0; 
volatile uint16_t speed_measurement_counter = 0; 
volatile uint16_t current_min = 4095; 
volatile uint16_t current_max = 0;
volatile uint16_t current_pp = 4095; 
volatile uint16_t ripple_counter = 0; 
volatile uint16_t hyst_thresh_hi = 0; 
volatile uint16_t hyst_thresh_lo = 0; 
volatile uint16_t thrsh_HI_multiplier = 0;
volatile uint16_t thrsh_HI_divider    = 0;
volatile uint16_t thrsh_LO_multiplier = 0;
volatile uint16_t thrsh_LO_divider    = 0;
volatile uint16_t digital_comparator_output = 0; // debug & verification variable
volatile uint16_t debug_thresh_hi = 0; // debug & verification variable
volatile uint16_t debug_thresh_lo = 0; // debug & verification variable

volatile uint16_t current_average_index = 0; 
volatile uint16_t current_average5      = 0; 
volatile uint16_t current_buff[5]; 
volatile uint16_t i     = 0; 

volatile uint16_t new_speed_flag = 0;

//--------- gate driver status reading implementation --------------------
volatile MCP802X_STATE              mcp8021_communicate_status;
         MCP802X_POWER_STATUS_FLAGS mcp8021_power_status;
volatile bool                       mcp8021_power_status_event;
volatile uint16_t                   mcp8021_faults = 0;
volatile uint16_t                   mcp8021_warnings;
volatile uint16_t                   mcp8021_fault_pin_state = 0; 
uint8_t u8PowerStatus;
//---------   --------------------
extern uint16_t adcVal;

//---------   --------------------
volatile uint16_t motor_current                = 0;
volatile uint16_t motor_speed                  = 0; 

volatile uint16_t DC_bus_voltage       = 0;
volatile uint16_t Speed1_digital_input = 0;
volatile uint16_t Brake_digital_input  = 0;
//---------------------------------------------

volatile uint16_t cmd_motor_speed_rpm = 0;
volatile uint16_t cmd_motor_stop      = 0;

volatile uint16_t cmd_motor_voltage_duty_cycle = 0; 
volatile uint16_t motor_voltage_duty_cycle     = 0; 

//volatile uint16_t new_duty_cycle = 0;

volatile enum MOTOR_COMMAND dir_pwm            = MOTOR_COMMAND_STOP;
volatile enum MOTOR_COMMAND last_motor_command = MOTOR_COMMAND_STOP;
volatile uint16_t pwm_enabled    = 0;

//--------- Tasks variables  --------------------
volatile uint16_t task_1ms_execute_flag  = 0;
volatile uint16_t task_200ms_execute_flag  = 0;
volatile uint16_t task_200ms_counter  = 0;

//--------- Hall Sensor --------------------
volatile uint16_t hall_sensor_addr     = 0; //0x35; // 7-bit address; 8-bit address = 0x6A/0x6B
volatile bool     hall_sensor_detected = false;
uint8_t  hall_sensor_read_data[10];
volatile bool IsSensorReadFinished   = false;
volatile bool IsSensorReadInProgress = false;
volatile struct I2C_TRANSFER_SETUP HallSensor_Setup;
volatile uint32_t sysClk = FCY/2;

//--------- scaling factors --------------------
float    K_dc_bus  = 0.00886230469f; //   V/bit   = 3.3V / (4096 * (1.8Kohms / (18 Kohms + 1.8 Kohms)))
float    K_current = 0.01708984375f; //   A/bit   = 70 A / 4096
float    K_duty_cycle = 100.0f / 5000.0f; //   %
uint16_t K_speed   = 60*10/12;     //  50 rpm/ripple;  12 ripples / motor revolution
//---------------------------------------------

//--------- DEMO variables --------------------
volatile enum DEMO_STATE_MACHINE  demo_status = DEMO_STOP;
volatile enum DEMO_STATE_MACHINE  demo_status_old = DEMO_STOP;
volatile uint16_t demo_counter = 40100;
//---------------------------------------------

/////////////////////////////////////////

bool firstStart = 0;

void mcp8x_clear_fault()
{
    MCP802X_ENABLE_SetLow(); //disable the gate driver 
    __delay_ms(1);
    MCP802X_ENABLE_SetHigh(); //enable the gate driver 
}

void pwm_stop( void )
{
    PG1CONLbits.ON = 0;
    PG2CONLbits.ON = 0;
    pwm_enabled = 0;

    __delay_us(200);
    
    motor_current = MOTOR_CURRENT_0;
    motor_speed   = 0;
}


void set_speed_measurement_thresholds( enum MOTOR_COMMAND mot_cmd )
{
    switch( mot_cmd )
    {
        case MOTOR_COMMAND_RUN_LEFT:
            {
                thrsh_HI_multiplier = 24; // 24/40 = 60%
                thrsh_HI_divider    = 40;
                thrsh_LO_multiplier = 1;  // 1/4 = 25%
                thrsh_LO_divider    = 4;
            }
            break;
            
        case MOTOR_COMMAND_RUN_RIGHT:
            {
                thrsh_HI_multiplier = 24; // 24/40 = 60%
                thrsh_HI_divider    = 40;
                thrsh_LO_multiplier = 1;  // 1/4 = 25%
                thrsh_LO_divider    = 4;
            }
            break;
            
        case MOTOR_COMMAND_STOP:
                // Do not change anything
            break;
    }
}

void pwm_set_direction( enum MOTOR_COMMAND mot_cmd )
{
    //if ( old_direction != new_direction )
    {
        PG1CONLbits.ON = 0;
        PG2CONLbits.ON = 0;
        pwm_enabled = 0;

        __delay_us(200);
        
        motor_current = MOTOR_CURRENT_0;
        motor_speed   = 0;
        
        if( mot_cmd == MOTOR_COMMAND_RUN_LEFT )
        {
            dir_pwm = mot_cmd;
            PG1IOCONLbits.OVRENH = 0;
            PG1IOCONLbits.OVRENL = 0;

            PG2IOCONLbits.OVRENH = 1;
            PG2IOCONLbits.OVRENL = 1;     
        }
        
        if( mot_cmd == MOTOR_COMMAND_RUN_RIGHT )
        {
            dir_pwm = mot_cmd;
            PG1IOCONLbits.OVRENH = 1;
            PG1IOCONLbits.OVRENL = 1;

            PG2IOCONLbits.OVRENH = 0;
            PG2IOCONLbits.OVRENL = 0;      
        }
        
        __delay_us(200);

        PG1CONLbits.ON = 1;
        PG2CONLbits.ON = 1;
        pwm_enabled = 1;
    }
}

void motor_command( uint16_t volt_duty_cycle, enum MOTOR_COMMAND mot_cmd )
{
volatile uint16_t new_duty_cycle = volt_duty_cycle;
    
    if( new_duty_cycle >= 0)
    {
//        PG1CONLbits.ON = 1;
//        PG2CONLbits.ON = 1;
        
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
    }
    else
    {
//        PG1CONLbits.ON = 0;
//        PG2CONLbits.ON = 0;

        __delay_us(200);
        
        motor_current = MOTOR_CURRENT_0;
        motor_speed   = 0;
    }
    
    switch( mot_cmd )
    {
        case MOTOR_COMMAND_RUN_LEFT:
        case MOTOR_COMMAND_RUN_RIGHT:
            {
                set_speed_measurement_thresholds( mot_cmd );
                pwm_set_direction( mot_cmd );
            }
            break;

        case MOTOR_COMMAND_STOP:
            break;

        default:
                break;
    }
    
    last_motor_command = mot_cmd;
    
//    if( mot_cmd != -1) // stop
//    {
//        pwm_set_direction( direction );
//    }
}

void mcp8021_task_1ms()
{
    
    mcp8021_communicate_status = MCP802X_eHandleCommunication(); 
    
    //mcp8021_power_status = MCP802X_u16GetLastMcpFaults();
    mcp8021_power_status_event = MCP802X_PowerStatusGet( &mcp8021_power_status );
    
    if(mcp8021_faults == 0 ) //do not override the gate drive fault, store the first error
    { 
        mcp8021_faults = MCP802X_u16GetLastMcpFaults();
    }
    
    mcp8021_warnings = MCP802X_u16GetWarnings();
    
    if ( PORTDbits.RD1 == 0 )
    {
        mcp8021_fault_pin_state = 0; // FAULT
    }
    else
    {
        mcp8021_fault_pin_state = 1;
    }
}

void app_task_1ms()
{
    demo_counter--;
    if ( demo_counter == 0 )
    {
        demo_counter = 40100;  // restart counting
    }
    
    // DEMO 
    // for 10 seconds the MOTOR RUN LEFT direction
    // for next 10 seconds the MOTOR STOPS
    // for next 10 seconds the MOTOR RUN RIGHT direction
    // for next 10 seconds the MOTOR STOPS

    switch( demo_counter )
    {
        case 40000:
            demo_status = DEMO_RUN_LEFT;
            break;

        case 30000:
            demo_status = DEMO_STOP;
            break;

        case 20000:
            demo_status = DEMO_RUN_RIGHT;
            break;

        case 10000:
            demo_status = DEMO_STOP;
            break;

        default:
                break;
    }

#if (RUN_ON_EVAL_BOARD == 1)
//    if ( PORTDbits.RD8 == 0 )
//    {
//        // sw1 button pressed
//        if ( prev_io_state == 0 )
//        {
//            if ( io_debounce == 0 )
//            {
//                test_voltage_duty_cycle += 1000;
//
//                if( test_voltage_duty_cycle > 4500 )
//                {
//                    test_voltage_duty_cycle = MIN_DUTY_CYCLE;
//                    test_direction = !test_direction;
//                }
//
//                motor_command( test_voltage_duty_cycle, test_direction );
//            }
//
//            if( io_debounce >= 0)
//                io_debounce--;
//        }
//        else
//        {
//            io_debounce = 100;
//        }
//        prev_io_state = 0;
//    }
//    else
//    {
//        prev_io_state = 1;
//    }
#endif // RUN_ON_EVAL_BOARD


    #if (RUN_ON_EVAL_BOARD == 1)

    #else
        Speed1_digital_input = PORTDbits.RD8; //check if the Speed1 is connected
        Brake_digital_input = PORTDbits.RD13; //check if the Brake is connected
    #endif // RUN_ON_EVAL_BOARD


}

void duty_cycle_slope_task_1ms()
{
    if (motor_voltage_duty_cycle < cmd_motor_voltage_duty_cycle)
        motor_voltage_duty_cycle += 4;
    else
    {
        if (motor_voltage_duty_cycle > cmd_motor_voltage_duty_cycle)
            motor_voltage_duty_cycle -= 4;
    }

    if( motor_voltage_duty_cycle == MIN_DUTY_CYCLE ) // ignore the last 2 bits when compare because the duty increment is 4 !
    {
        if( last_motor_command == MOTOR_COMMAND_STOP )
        {
            if( pwm_enabled )
                pwm_stop();
        }
    }
}

void User_I2C1_Callback()
{
    IsSensorReadFinished = true;
}

void I2C1_scan_addresses_task()
{
static uint16_t addr = 0;
    
    if( addr < 0x7F)
    {
        if( IsSensorReadInProgress )
        {
            if( IsSensorReadFinished == true )
            {
                //printf("\r\nScan Address: %02X ", addr );
                
                if( I2C_HOST_ERROR_NONE == I2C1_ErrorGet() )
                {
                    hall_sensor_detected = 1;
                    hall_sensor_addr = addr;
                    //printf(" FOUND" );
                }
                else
                {
                    hall_sensor_detected = 0;
                    //printf(" not found" );
                }

                IsSensorReadInProgress = false;
            }
        }
        else
        {
            IsSensorReadFinished = false;
            IsSensorReadInProgress = true;

            addr++;
            I2C1_Read( addr, hall_sensor_read_data , 1);   
        }
    }
}

void I2C1_task()
{
    //////////////////// check read
    if( IsSensorReadInProgress )
    {
        if( IsSensorReadFinished == true )
        {
            if( I2C_HOST_ERROR_NONE == I2C1_ErrorGet() )
            {
                //printf("  Hall Sensor Present" );
                //printf("  Hall Data: %02X", HallSensor_read_data[0] );
                hall_sensor_detected = 1;
            }
            else
            {
                //printf( "  I2C error!" );
                hall_sensor_detected = 0;
            }

            IsSensorReadInProgress = false;
        }
    }
    else
    {
        IsSensorReadFinished = false;
        IsSensorReadInProgress = true;
        
        I2C1_Read( hall_sensor_addr, hall_sensor_read_data , 0 );   
    }
}

int main(void) {
    SYSTEM_Initialize();
    #if (X2CSCOPE_ENABLE == 1)
        X2Cscope_Init();
    #endif // X2CSCOPE_ENABLE

    // register the callback functions
    TMR1_TimeoutCallbackRegister(User_TMR1_TimeoutCallback);
    ADC1_ChannelCallbackRegister(User_ADC1_ChannelCallback);
    I2C1_CallbackRegister(User_I2C1_Callback);

    HallSensor_Setup.clkSpeed = 100000; //100kHz
    //I2C1_TransferSetup( &HallSensor_Setup, sysClk );

    motor_voltage_duty_cycle = MIN_DUTY_CYCLE; //set the initial duty cycle

#if (RUN_ON_EVAL_BOARD == 1)
    // pin B0 used as output test pin, only on evaluation board
    TRISBbits.TRISB0 = 0; // pin B0 = OUTPUT
    ANSELBbits.ANSELB0 = 0; // pin B0 = ANALOG
#else
#endif // RUN_ON_EVAL_BOARD

    TMR1_Start();

    // Manually enable PWM to trigger ADC channels as MCC does not support by default
#if (RUN_ON_EVAL_BOARD == 1)
    PWM_Trigger1Enable(PWM_GENERATOR_2, PWM_TRIGGER_COMPARE_A);
#else
    PWM_Trigger1Enable(PWM_GENERATOR_2, PWM_TRIGGER_COMPARE_A);
#endif // RUN_ON_EVAL_BOARD

    PG2TRIGB = PG2PER; // sampling point for the motor current at middle of PWM pulse
    PWM_Trigger2Enable(PWM_GENERATOR_2, PWM_TRIGGER_COMPARE_B); //enable the sampling point for the motor current 


    MCP802X_ENABLE_SetHigh();
    __delay_us(100);
    MCP802X_ENABLE_SetLow();
    __delay_us(100);
    MCP802X_ENABLE_SetHigh();
    __delay_us(100);
    MCP802X_u8ConfigSet(config1); //Set the basic configuration to the gate drive - Mandatory

    #if (X2CSCOPE_ENABLE == 1)
    #else
        printf("START\r\n");
    #endif // X2CSCOPE_ENABLE

    //while( mcp8021_communicate_status != MCP802X_READY ){} //wait until the gate driver is in the READY state

    demo_status = DEMO_STOP;

    while (1) {
        //////////////////// TASK - 1 ms
        if (task_1ms_execute_flag) {
            task_1ms_execute_flag = 0;

            mcp8021_task_1ms();

            app_task_1ms();

            duty_cycle_slope_task_1ms();
        }
        //////////////////// 

    #if (X2CSCOPE_ENABLE == 1)
        X2CScope_Communicate(); //handles the communication with the MPLAB X plugin on the PC side
    #endif // X2CSCOPE_ENABLE

#if (RUN_ON_EVAL_BOARD == 1)

#else
        //I2C1_task();
        I2C1_scan_addresses_task();
#endif // RUN_ON_EVAL_BOARD

        //////////////////// 
        if ((mcp8021_communicate_status == MCP802X_READY &&
                (firstStart == 0)
                )) {
            firstStart = 1;
            mcp8021_faults = 0;
            //motor_command(MAX_DUTY_CYCLE, MOTOR_COMMAND_RUN_LEFT );
            
        }
        if((mcp8021_faults == 0) && 
            (mcp8021_communicate_status == MCP802X_READY)){

            if ((demo_status != demo_status_old)
                    ) {
                switch (demo_status) {
                    case DEMO_STOP:
                        motor_command( 0, MOTOR_COMMAND_STOP);
                        break;

                    case DEMO_RUN_LEFT:
                        motor_command( MAX_DUTY_CYCLE, MOTOR_COMMAND_RUN_RIGHT );
                        break;

                    case DEMO_RUN_RIGHT:
                        motor_command( MAX_DUTY_CYCLE, MOTOR_COMMAND_RUN_RIGHT );
                        break;

                    default:
                        break;
                }
                demo_status_old = demo_status;
            }
        }
        else
        {
            // motor_command(0, -1); // stop
        }
         



        //////////////////// TASK - 200 ms
        if (task_200ms_execute_flag) {
            task_200ms_execute_flag = 0;

            #if (X2CSCOPE_ENABLE == 1)

            #else
                printf("\r\nMCP_Status= %04X,", mcp8021_communicate_status);
                printf(" MCP_FAULTS= %04X,", mcp8021_faults);
                printf(" MCP_fault_PIN= %04X,", mcp8021_fault_pin_state);
                printf(" MCP_WARNS= %04X,", mcp8021_warnings);
                printf(" Demo_status= %d,", demo_status);
                printf(" PWM= %d",  pwm_enabled);
                if( pwm_enabled )
                    printf(" Motor_DutyCycle= %4.1f %%,", motor_voltage_duty_cycle * K_duty_cycle);
                else
                    printf(" Motor_DutyCycle= ---- %%," );
                printf(" Speed= %4u rpm,", motor_speed * K_speed);
                printf(" Current = %6.2f A,", (motor_current * K_current) - ((2048 * K_current)));
                printf(" DC_bus = %5.2f V,", DC_bus_voltage * K_dc_bus);
                printf(" Speed1_IN= %d,", Speed1_digital_input);
                printf(" Brake_IN= %d", Brake_digital_input);
    //            printf(" mot_cmd= %d",  last_motor_command );
                printf(" dir_pwm= %d", dir_pwm);
            #endif // X2CSCOPE_ENABLE
#if (RUN_ON_EVAL_BOARD == 1)
#else
            #if (X2CSCOPE_ENABLE == 1)
            #else
                if (hall_sensor_detected) {
                    //printf(" Hall detected, Address= 0x%02X", hall_sensor_addr);
                } else {
                    //printf(" Hall scanning...");
            }
            #endif // X2CSCOPE_ENABLE
#endif // RUN_ON_EVAL_BOARD
        }
        //////////////////// 
    }
}


/////////////// Timer1 handles the DE2 gate driver communication
void User_TMR1_TimeoutCallback()
{
    task_1ms_execute_flag = 1;

    if( task_200ms_counter == 0 )
    {
        task_200ms_counter = 200;
        
        task_200ms_execute_flag = 1;
    }
    
    task_200ms_counter--;
}

void User_ADC1_ChannelCallback(enum ADC_CHANNEL channel, uint16_t adcVal)
{
    #if (RUN_ON_EVAL_BOARD == 1)
        if(channel == Channel_AN15)
        {
            //PG1DC = adcVal * 1.1; // set duty cycle from external potentiometer value;  scale the value from 4096 ==>> 4500 (90% duty cycle)
            PG1DC = motor_voltage_duty_cycle; // set duty cycle from variable
            PG2DC = PG1DC;
        }       
        
        if( channel == Channel_AN12)
        {
            DC_bus_voltage = adcVal; //read the bus voltage
        }  
        
    #else

        PG1DC = motor_voltage_duty_cycle; // set duty cycle from variable
        PG2DC = PG1DC;
            
        if( channel == Channel_AN2 )
        {
            DC_bus_voltage = adcVal; //read the bus voltage
        }  

    #endif // RUN_ON_EVAL_BOARD
    

    if(channel == Channel_AN7)
    {
        motor_current = adcVal; // read the value of the motor current
        
        #if (RUN_ON_EVAL_BOARD == 1)
//            LATBbits.LATB0 = 1; // toggle this pin to check the interrupt duration
            PG2TRIGA = 500; // trigger point for DC bus voltage measurement and external potentiometer
        #else
            PG2TRIGA = 500; // trigger point for DC bus voltage measurement
        #endif // RUN_ON_EVAL_BOARD



        //////////////////// SPEED MEASUREMENT - START
        
        current_buff[ current_average_index ] = motor_current;
        current_average_index++;

        if( current_average_index == 5 )
        {
            current_average_index = 0;
            current_average5     = 0; 

            for( i=0; i<5; i++)
            {
                current_average5 += current_buff[ i ];
            }
            current_average5 = current_average5/5;
        }
            
        if(hysteresis_update_counter < HYSTERESIS_UPDATE_INTERVAL) // Stage 1: Detect the minimum and maximum motor current values
        {
            // Update Hysteresis Thresholds Period = HYSTERESIS_UPDATE_INTERVAL * PWM_PERIOD = 4000 us
            // Detect the minimum and maximum motor current values
            hysteresis_update_counter++;
            if(current_average5 < current_min)
            {
                current_min = current_average5;
            }
            
            if(current_average5 > current_max)
            {
                current_max = current_average5;
            }            
            
            if((current_average5 > hyst_thresh_hi) && digital_comparator_output == 0) 
            {
                digital_comparator_output = 1; //reset the state of the comparator
            }
            
            if(current_average5 < hyst_thresh_lo && digital_comparator_output == 1)
            {
                // complete pulse on comparator output
                ripple_counter++;   // count a new current ripple
                digital_comparator_output = 0;
            }
        }
        else
        {
            // Stage 2: Update the hysteresis thresholds
            speed_measurement_counter++;
            current_pp = current_max - current_min; // current peak-peak

            // hyst_thresh_hi = current_min + 80% * current_pp   ( empirical determination! )
            hyst_thresh_hi = thrsh_HI_multiplier*current_pp;
            hyst_thresh_hi = hyst_thresh_hi/thrsh_HI_divider;
            hyst_thresh_hi = hyst_thresh_hi + current_min;
            
            // hyst_thresh_lo = current_min + 25% * current_pp   ( empirical determination! )
            hyst_thresh_lo = thrsh_LO_multiplier*current_pp;
            hyst_thresh_lo = hyst_thresh_lo/thrsh_LO_divider;
            hyst_thresh_lo = hyst_thresh_lo + current_min;
            
            hysteresis_update_counter = 0;           

            current_max = 0;
            current_min = 4095;
            digital_comparator_output = 0;
            debug_thresh_hi = hyst_thresh_hi;
            debug_thresh_lo = hyst_thresh_lo;
        }
        
        if(speed_measurement_counter == SPEED_MEASUREMENT_INTERVAL) //
        {
            // Speed Measurement Period = SPEED_MEASUREMENT_INTERVAL * HYSTERESIS_UPDATE_INTERVAL * PWM_PERIOD = 25 * 80 * 50us = 100 ms
            // Speed measurement, save the counted ripples
            if (motor_current > (2048 + 17)) // measure the speed only if the current > 300mA
            {
                motor_speed = ripple_counter;
            }
            
            ripple_counter = 0;
            speed_measurement_counter = 0;
            current_pp = 4095; //reset the peak-to-peak value of the current
            new_speed_flag = 1;
        }

        //////////////////// SPEED MEASUREMENT - END
        

    #if (X2CSCOPE_ENABLE == 1)
        X2CScope_Update(); //This is the sample point of the scope at every 50us
    #endif // X2CSCOPE_ENABLE
        
    #if (RUN_ON_EVAL_BOARD == 1)
//            LATBbits.LATB0 = 0;
    #else

    #endif // RUN_ON_EVAL_BOARD

    }
}

