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
void User_TMR1_TimeoutCallback();
void User_I2C1_Callback();

#define HYSTERESIS_UPDATE_INTERVAL   80
#define SPEED_MEASUREMENT_INTERVAL   25


#define MAX_DUTY_CYCLE               4500
#define MIN_DUTY_CYCLE               500

enum DEMO_STATE_MACHINE
{
    DEMO_STOP = 0,
    DEMO_RUN_LEFT,
    DEMO_RUN_RIGHT
};

#define START_TIMER_MS(tmr_counter, x) tmr_counter = x;
#define IS_TIMER_MS_ELAPSED(tmr_counter) (tmr_counter == 0)


#define FCY 200000000  // System Clock
#include <libpic30.h>
#include <stdio.h>



//--------- speed measurement variables --------------------
volatile uint16_t hysteresis_update_counter = 0; 
volatile uint16_t speed_measurement_counter = 0; 
volatile uint16_t current_min = 4095; 
volatile uint16_t current_max = 0;
volatile uint16_t current_pp = 4095; 
volatile uint16_t ripple_counter = 0; 
volatile uint16_t hyst_thresh_hi = 0; 
volatile uint16_t hyst_thresh_lo = 0; 
volatile uint16_t digital_comparator_output = 0; // debug & verification variable
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
//---------   --------------------

volatile uint16_t cmd_motor_speed_rpm = 0;
volatile uint16_t cmd_motor_direction = 0;
volatile uint16_t cmd_motor_stop      = 0;

volatile uint16_t cmd_motor_voltage_duty_cycle = 0; 
volatile uint16_t motor_voltage_duty_cycle     = 0; 
volatile uint16_t motor_voltage_slope_counter  = 0; 

//---------   --------------------

volatile uint16_t new_duty_cycle = 0;

volatile uint16_t old_direction = 0;
volatile uint16_t is_motor_run  = 0;

volatile uint16_t prev_io_state = 1;
volatile uint16_t io_debounce = 0;
volatile uint16_t test_voltage_duty_cycle = 0;
volatile uint16_t test_direction = 0;
//---------   --------------------

// 1 milliseconds task variables
//#define TASK_EXECUTION_PERIOD          500 // 500 ms
//volatile uint16_t task_counter       = TASK_EXECUTION_PERIOD;
volatile uint16_t task_1ms_execute_flag  = 0;


//---------  --------------------
volatile uint16_t task_200ms_execute_flag  = 0;
volatile uint16_t task_200ms_counter  = 0;

//--------- Hall Sensor --------------------
volatile uint16_t hall_sensor_addr     = 0; //0x35; // 7-bit address; 8-bit address = 0x6A/0x6B
volatile bool     hall_sensor_detected = false;
uint8_t  hall_sensor_read_data[10];

volatile bool IsSensorReadFinished   = false;
volatile bool IsSensorReadInProgress = false;

volatile struct I2C_TRANSFER_SETUP HallSensor_Setup;
volatile uint32_t sysClk = FCY;
//---------  --------------------



volatile enum DEMO_STATE_MACHINE  demo_status = DEMO_STOP;
volatile enum DEMO_STATE_MACHINE  demo_status_old = DEMO_STOP;
volatile uint16_t demo_counter = 40100;


float K_dc_bus  = 0.00886230469f; //   V/bit  = 3.3V / (4096 * (1.8Kohms / (18 Kohms + 1.8 Kohms)))
float K_current = 0.01708984375f; //   A/bit  = 70 A / 4096




/////////////////////////////////////////



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

void motor_command( uint16_t volt_duty_cycle, uint16_t direction)
{
    new_duty_cycle = volt_duty_cycle;
    
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
    }
    else
    {
        PG1CONLbits.ON = 0;
        PG2CONLbits.ON = 0;

        __delay_us(200);        
    }
//    old_direction = direction;
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

    if ( demo_counter == 0 )
    {
        demo_counter = 40100;  // restart counting
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

int main(void)
{
    SYSTEM_Initialize();
    X2Cscope_Init();
    
    // register the callback functions
    TMR1_TimeoutCallbackRegister( User_TMR1_TimeoutCallback );
    ADC1_ChannelCallbackRegister( User_ADC1_ChannelCallback );
    I2C1_CallbackRegister( User_I2C1_Callback );

    HallSensor_Setup.clkSpeed = 100000; //100kHz
    //I2C1_TransferSetup( &HallSensor_Setup, sysClk );
    
    motor_voltage_duty_cycle = MIN_DUTY_CYCLE; //set the initial duty cycle
    
    TMR1_Start();

    // Manually enable PWM to trigger ADC channels as MCC does not support by default
    #if (RUN_ON_EVAL_BOARD == 1)
        PWM_Trigger1Enable( PWM_GENERATOR_2,PWM_TRIGGER_COMPARE_A ); 
    #else
        PWM_Trigger1Enable( PWM_GENERATOR_2,PWM_TRIGGER_COMPARE_A ); 
    #endif // RUN_ON_EVAL_BOARD
   
    PG2TRIGB = PG2PER; // sampling point for the motor current at middle of PWM pulse
    PWM_Trigger2Enable( PWM_GENERATOR_2, PWM_TRIGGER_COMPARE_B ); //enable the sampling point for the motor current 
    
    MCP802X_u8ConfigSet(config1); //Set the basic configuration to the gate drive - Mandatory
    MCP802X_ENABLE_SetHigh();     //enable the gate driver
    
    printf("START\r\n");
    
    //while( mcp8021_communicate_status != MCP802X_READY ){} //wait until the gate driver is in the READY state
    
    demo_status = DEMO_RUN_LEFT;

    while(1)
    {
        //LATBbits.LATB0 = 1;
        X2CScope_Communicate(); //handles the communication with the MPLAB X plugin on the PC side
        //LATBbits.LATB0 = 0;

        //////////////////// TASK - 1 ms
        if(task_1ms_execute_flag)
        {
            task_1ms_execute_flag = 0;
            
            mcp8021_task_1ms();
            
            app_task_1ms();
            
            duty_cycle_slope_task_1ms();
        }
        //////////////////// 
        
        #if (RUN_ON_EVAL_BOARD == 1)

        #else
            //I2C1_task();
            I2C1_scan_addresses_task();
        #endif // RUN_ON_EVAL_BOARD
        
        //////////////////// 

        if( demo_status != demo_status_old )
        {
            switch( demo_status )
            {
                case DEMO_STOP:
                        motor_command( 0, 1 );
                        break;

                case DEMO_RUN_LEFT:
                        motor_command( MAX_DUTY_CYCLE, 0 );
                        break;

                case DEMO_RUN_RIGHT:
                        motor_command( MAX_DUTY_CYCLE, 1 );
                        break;

                default:
                        break;
            }
            demo_status_old = demo_status;
        }
        
        //////////////////// TASK - 200 ms
        if( task_200ms_execute_flag )
        {
            task_200ms_execute_flag = 0;
            
            printf("\r\nMCP_Status= %04X,", mcp8021_communicate_status );
            printf(" MCP_FAULTS= %04X,",    mcp8021_faults );
            printf(" MCP_fault_PIN= %04X,", mcp8021_fault_pin_state );
            printf(" MCP_WARNS= %04X,",     mcp8021_warnings );
            printf(" Demo_status= %d,",     demo_status );
            printf(" Motor_Voltage= %u,",   motor_voltage_duty_cycle );
            printf(" Speed= %u,",           motor_speed );
            printf(" Current = %.2f A,",    (motor_current * K_current) - 35.0f );
            printf(" DC_bus = %.2f V,",     DC_bus_voltage * K_dc_bus  );
            printf(" Speed1_IN= %d,",       Speed1_digital_input );
            printf(" Brake_IN= %d",         Brake_digital_input );
            #if (RUN_ON_EVAL_BOARD == 1)
            #else
                if( hall_sensor_detected )
                {
                    printf(" Hall detected, Address= 0x%02X", hall_sensor_addr );
                }
                else
                {
                    printf(" Hall scanning..." );
                }
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
        
        if(hysteresis_update_counter < HYSTERESIS_UPDATE_INTERVAL) // Stage 1: Detect the minimum and maximum motor current values
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
            // Stage 2: Update the hysteresis thresholds
            speed_measurement_counter++;
            current_pp = current_max - current_min; // current peak-peak
            
            // hyst_thresh_hi = current_min + 80% * current_pp   ( empirical determination! )
            hyst_thresh_hi = 32*current_pp;
            hyst_thresh_hi = hyst_thresh_hi/40;
            hyst_thresh_hi = hyst_thresh_hi + current_min;
            
            // hyst_thresh_lo = current_min + 25% * current_pp   ( empirical determination! )
            hyst_thresh_lo = current_pp;
            hyst_thresh_lo = hyst_thresh_lo/4;
            hyst_thresh_lo = hyst_thresh_lo + current_min;
            
            hysteresis_update_counter = 0;           

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
}

