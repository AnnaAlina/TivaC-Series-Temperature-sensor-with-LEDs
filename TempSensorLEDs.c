#define TEMP_ADDR 0x4F        // Address for Temp Sensor
#define PART_TM4C123GH6PM    // Define needed for pin_map.h

#include <stdbool.h>
#include <stdint.h>
#include "inc/tm4c123gh6pm.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "inc/hw_i2c.h"
#include "driverlib/i2c.h"
#include "inc/hw_ints.h"
#include "driverlib/interrupt.h"
#include "driverlib/timer.h"

unsigned char start_screen[31] = "\n\n\r ATE Lab 9 Final Project \n\n\r";
unsigned char log[18] = "\n\n\r Temp reading: ";
unsigned char start_temp = 0x00;        // starting temp

void Print_header();                // Prints Header
unsigned short int Read_temp(unsigned char *data);    // Read Temperature sensor
void Set_Temp_Start();                // Set starting value

void main(void) {
    
    unsigned char temp_data[10] = "00.0 C \n\n\r";        // Temp format to be edited by read
    unsigned short int i = 0;                 // general counter
    unsigned short int LED_value = 0;            // LED Power switch

    // Setup the I2C see lab 7 ****************************************************************************************************
    SysCtlClockSet(SYSCTL_SYSDIV_1 | SYSCTL_USE_OSC | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);     //setup clock

    SysCtlPeripheralEnable(SYSCTL_PERIPH_I2C0);        // Enable I2C hardware
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);    // Enable Pin hardware

    GPIOPinConfigure(GPIO_PB3_I2C0SDA);            // Configure GPIO pin for I2C Data line
    GPIOPinConfigure(GPIO_PB2_I2C0SCL);            // Configure GPIO Pin for I2C clock line

    GPIOPinTypeI2C(GPIO_PORTB_BASE, GPIO_PIN_2 | GPIO_PIN_3); // Set Pin Type

    GPIOPadConfigSet(GPIO_PORTB_BASE, GPIO_PIN_2, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD);        // SDA MUST BE STD
    GPIOPadConfigSet(GPIO_PORTB_BASE, GPIO_PIN_3, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_OD);        // SCL MUST BE OPEN DRAIN
    I2CMasterInitExpClk(I2C0_BASE, SysCtlClockGet(), false);                         
// The False sets the controller to 100kHz communication
    I2CMasterSlaveAddrSet(I2C0_BASE, TEMP_ADDR, true);     // false means transmit
    //******************************************************************************************************

    // Setup the UART see lab 6 *******************************************************************************************************

    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);        // Enable UART hardware
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);        // Enable Pin hardware

    GPIOPinConfigure(GPIO_PA0_U0RX);        // Configure GPIO pin for UART RX line
    GPIOPinConfigure(GPIO_PA1_U0TX);        // Configure GPIO Pin for UART TX line
    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1); // Set Pins for UART

    UARTConfigSetExpClk(UART0_BASE, SysCtlClockGet(), 115200,    
                            // Configure UART to 8N1 at 115200bps
        (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE));
    //******************************************************************************************************

    // Setup the LEDs see lab 2-5 ********************************************************************************************************    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);                 //PORTC
    GPIOPinTypeGPIOOutput(GPIO_PORTC_BASE, GPIO_PIN_6|GPIO_PIN_7);     // LED 1 LED 2

    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);             //PORTB
    GPIOPinTypeGPIOOutput(GPIO_PORTB_BASE, GPIO_PIN_5);         // LED 4

    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);             //PORT D
    GPIOPinTypeGPIOOutput(GPIO_PORTD_BASE, GPIO_PIN_6);     // LED 3
    //******************************************************************************************************

    Print_header();            // Print Header
    Set_Temp_Start();

    while(1){

        LED_value = Read_temp(temp_data);        // Read Data from Temp Sensor
        SysCtlDelay(6000000);            // Delay
        for(i=0;i<10;i++){                // Loop to print out data string
            UARTCharPut(UART0_BASE, temp_data[i]);
        }

        if(LED_value == 1){
        GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_6|GPIO_PIN_7, 0x40); // LED 1 ON, LED 2 OFF
        GPIOPinWrite(GPIO_PORTD_BASE, GPIO_PIN_6, 0x00);             // LED 3 OFF
        GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_5, 0x00);             // LED 4 OFF
            }

            else if (LED_value == 2){
                GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_6|GPIO_PIN_7, 0xFF);
                                // LED 1 OFF, LED 2 ON
                GPIOPinWrite(GPIO_PORTD_BASE, GPIO_PIN_6, 0x00);    // LED 3 OFF
                GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_5, 0x00);    // LED 4 OFF
            }

            else if (LED_value == 3){
                GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_6|GPIO_PIN_7, 0xFF);
                                        // LED 1 OFF, LED 2 OFF
                GPIOPinWrite(GPIO_PORTD_BASE, GPIO_PIN_6, 0xFF);    // LED 3 ON
                GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_5, 0x00);    // LED 4 OFF
            }
            else if(LED_value == 4){
                GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_6|GPIO_PIN_7, 0xFF);
                                        // LED 1 OFF, LED 2 OFF
                GPIOPinWrite(GPIO_PORTD_BASE, GPIO_PIN_6, 0xFF);    // LED 3 OFF
                GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_5, 0x20);    // LED 4 ON
            }
    }
}
void Print_header(){        // Print Header at start of program

    int i = 0;             // general counter

    for(i=0;i<31;i++){        // Print Header at start of program
        UARTCharPut(UART0_BASE, start_screen[i]);
    }
}
unsigned short int Read_temp(unsigned char *data){    // Read Temperature sensor

    unsigned char temp[2];            // storage for data
    signed int temp_diff = 0;            // difference between start and new data

    I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_RECEIVE_START);    // Start condition
    SysCtlDelay(20000);                            // Delay
    
    temp[0] = I2CMasterDataGet(I2C0_BASE);                    // Read first char
    SysCtlDelay(20000);                            // Delay
    I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_RECEIVE_CONT);    // Push second Char
    SysCtlDelay(20000);                            // Delay
    temp[1] = I2CMasterDataGet(I2C0_BASE);                    // Read second char
    I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_RECEIVE_FINISH);    // Stop Condition

    data[0] = (temp[0] / 10) + 0x30;                // convert 10 place to ASCII
    data[1] = (temp[0] - ((temp[0] / 10)*10)) + 0x30;            // Convert 1's place to ASCII
    if(temp[1] == 0x80){                     // Test for .5 accuracy
        data[3] = 0x35;
    }
    else{
        data[3] = 0x30;
    }

    temp_diff = temp[0] - start_temp;        // get difference between new data and boot

    if(temp_diff <= 1){                // return LED switch value
        return 1;
    }

    else if ((temp_diff > 1) && (temp_diff <= 2)){
        return 2;
    }

    else if ((temp_diff > 2) && (temp_diff <= 3)){
        return 3;
    }
    else if(temp_diff > 3){
        return 4;
    }
}
void Set_Temp_Start(){

    unsigned char temp[2];                 // storage for data

    I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_RECEIVE_START);    // Start condition
    SysCtlDelay(20000);                            // Delay
    temp[0] = I2CMasterDataGet(I2C0_BASE);                    // Read first char
    SysCtlDelay(20000);                            // Delay
    I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_RECEIVE_CONT);    // Push second Char
    SysCtlDelay(20000);                            // Delay
    temp[1] = I2CMasterDataGet(I2C0_BASE);                    // Read second char
    I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_RECEIVE_FINISH);    // Stop Condition

    start_temp = temp[0];                            // assign starting value
}
