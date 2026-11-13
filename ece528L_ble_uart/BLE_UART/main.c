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

    // Buffer for commands received from the BLE module
    char BLE_UART_Buffer[BLE_UART_BUFFER_SIZE] = {0};

    // Enable interrupts
    EnableInterrupts();

    // Allow initialization to finish, then reset the BLE module
    Clock_Delay1ms(1000);
    BLE_UART_Reset();

    // Test the BLE UART connection
    BLE_UART_OutString("BLE UART Active\r\n");
    Clock_Delay1ms(1000);

    while (1)
    {
        int string_size = BLE_UART_InString(
            BLE_UART_Buffer,
            BLE_UART_BUFFER_SIZE
        );

        printf("BLE UART Data: ");

        for (int i = 0; i < string_size; i++)
        {
            printf("%c", BLE_UART_Buffer[i]);
        }

        printf("\n");

        Process_BLE_UART_Data(BLE_UART_Buffer);
    }
}
