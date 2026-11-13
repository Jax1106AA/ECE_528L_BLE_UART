/**
 * @file main.c
 *
 * @brief Main source code for the BLE_UART program.
 *
 * This file contains the main entry point for the BLE_UART program,
 * which is used to demonstrate the BLE_UART driver.
 *
 * It interfaces with the Adafruit Bluefruit LE UART Friend Bluetooth Low Energy (BLE) module, which uses the UART communication protocol.
 *  - Product Link: https://www.adafruit.com/product/2479
 *
 * @note For more information regarding the Enhanced Universal Serial Communication Interface (eUSCI),
 * refer to the MSP432Pxx Microcontrollers Technical Reference Manual
 *
 * @author Aolany Acosta & Hernan Zapien Robles
 *
 */

#include <stdio.h>
#include <math.h>
#include "msp.h"

#include "inc/Clock.h"
#include "inc/CortexM.h"
#include "inc/GPIO.h"
#include "inc/EUSCI_A0_UART.h"
#include "inc/Motor.h"
#include "inc/BLE_UART.h"

#define PWM_NOMINAL 3000

void Process_BLE_UART_Data(char BLE_UART_Buffer[])
{
    if (Check_BLE_UART_Data(BLE_UART_Buffer, "RGB LED GREEN"))
    {
        LED2_Output(RGB_LED_GREEN);
    }
    else if (Check_BLE_UART_Data(BLE_UART_Buffer, "RGB LED OFF"))
    {
        LED2_Output(RGB_LED_OFF);
    }
    else
    {
        printf("BLE UART Command Not Found\n");
    }
}

/**
 * @brief Continuously transmits four fixed UART test bytes.
 *
 * The delay between bytes allows each UART frame to be captured
 * separately with an oscilloscope.
 */
void UART_Fixed_Byte_Test(void)
{
    while (1)
    {
        BLE_UART_OutChar(0xAA);
        Clock_Delay1ms(500);

        BLE_UART_OutChar(0x55);
        Clock_Delay1ms(500);

        BLE_UART_OutChar(0xFF);
        Clock_Delay1ms(500);

        BLE_UART_OutChar(0x00);
        Clock_Delay1ms(500);
    }
}

/**
 * @brief Transmits a UART byte selected by the PMOD SWT switches.
 *
 * SW1 transmits 0xAA, SW2 transmits 0x10, SW3 transmits
 * 0xF0, and SW4 transmits 0x0F. Any other switch
 * combination transmits 0x00.
 */
void UART_PMOD_Switch_Test(void)
{
    uint8_t switch_status;
    uint8_t uart_byte;

    PMOD_SWT_Init();

    while (1)
    {
        switch_status = Get_PMOD_SWT_Status();

        switch (switch_status)
        {
            case 0x01:
                uart_byte = 0xAA;
                break;

            case 0x02:
                uart_byte = 0x10;
                break;

            case 0x04:
                uart_byte = 0xF0;
                break;

            case 0x08:
                uart_byte = 0x0F;
                break;

            default:
                uart_byte = 0x00;
                break;
        }

        BLE_UART_OutChar(uart_byte);
        Clock_Delay1ms(500);
    }
}

/**
 * @brief Decodes a Bluefruit Controller button packet and controls the motors.
 *
 * Valid packets begin with "!B". Buttons 5-8 represent the four
 * directional arrows. A pressed state moves the robot, while any
 * arrow-button release stops both motors.
 *
 * @param packet Five-byte Bluefruit Controller packet
 */
void Decode_Controller_Packet(char packet[])
{
    char button;
    char state;

    // Verify the packet header and Button packet type
    if ((packet[0] != '!') || (packet[1] != 'B'))
    {
        return;
    }

    button = packet[2];
    state = packet[3];

    printf("Button: %c, State: %c\n", button, state);

    // Stop immediately whenever an arrow button is released
    if (state == '0')
    {
        Motor_Stop();
        return;
    }

    // Ignore invalid button states
    if (state != '1')
    {
        return;
    }

    switch (button)
    {
        case '5':
            Motor_Forward(PWM_NOMINAL, PWM_NOMINAL);
            break;

        case '6':
            Motor_Backward(PWM_NOMINAL, PWM_NOMINAL);
            break;

        case '7':
            Motor_Left(PWM_NOMINAL, PWM_NOMINAL);
            break;

        case '8':
            Motor_Right(PWM_NOMINAL, PWM_NOMINAL);
            break;

        default:
            Motor_Stop();
            break;
    }
}

int main(void)
{
    // Disable interrupts during initialization
    DisableInterrupts();

    // Initialize the 48 MHz clock
    Clock_Init48MHz();

    // Initialize the RGB LED
    LED2_Init();

    // Initialize the computer serial terminal UART
    EUSCI_A0_UART_Init_Printf();

    // Initialize the BLE UART module
    BLE_UART_Init();

    // Initialize the DC motors
    Motor_Init();

    // Buffer for one 5-byte Controller packet
    char BLE_UART_Buffer[BLE_UART_BUFFER_SIZE] = {0};

    // Enable interrupts
    EnableInterrupts();

    // Allow initialization to finish, then reset the BLE module
    Clock_Delay1ms(1000);
    BLE_UART_Reset();

    // Confirm that the BLE UART connection is active
    BLE_UART_OutString("BLE UART Active\r\n");
    Clock_Delay1ms(1000);

    while (1)
    {
        int packet_size = BLE_UART_InPacket(
            BLE_UART_Buffer,
            BLE_UART_BUFFER_SIZE
        );

        if (packet_size == 5)
        {
            Decode_Controller_Packet(BLE_UART_Buffer);
        }
    }
}
