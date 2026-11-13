/**
 * @file BLE_UART.c
 *
 * @brief Source code for the BLE_UART driver.
 *
 * This file contains the function definitions for the BLE_UART driver.
 *
 * It interfaces with the Adafruit Bluefruit LE UART Friend Bluetooth Low Energy (BLE) module, which uses the UART communication protocol.
 *  - Product Link: https://www.adafruit.com/product/2479
 *
 * The following connections must be made:
 *  - BLE UART MOD  (Pin 1)     <-->  MSP432 LaunchPad Pin P1.6
 *  - BLE UART CTS  (Pin 2)     <-->  MSP432 LaunchPad GND
 *  - BLE UART TXO  (Pin 3)     <-->  MSP432 LaunchPad Pin P9.6 (PM_UCA3RXD)
 *  - BLE UART RXI  (Pin 4)     <-->  MSP432 LaunchPad Pin P9.7 (PM_UCA3TXD)
 *  - BLE UART VIN  (Pin 5)     <-->  MSP432 LaunchPad VCC (3.3V)
 *  - BLE UART RTS  (Pin 6)     <-->  Not Connected
 *  - BLE UART GND  (Pin 7)     <-->  MSP432 LaunchPad GND
 *  - BLE UART DFU  (Pin 8)     <-->  Not Connected
 *
 * @note For more information regarding the Enhanced Universal Serial Communication Interface (eUSCI),
 * refer to the MSP432Pxx Microcontrollers Technical Reference Manual
 *
 * @author
 *
 */

#include "../inc/BLE_UART.h"
void BLE_UART_Init()
{
    // Configure P9.6 (RX) and P9.7 (TX) for the EUSCI_A3 peripheral
    P9->SEL0 |= 0xC0;
    P9->SEL1 &= ~0xC0;

    // Configure P1.6 as an output GPIO for the BLE MOD pin
    P1->SEL0 &= ~0x40;
    P1->SEL1 &= ~0x40;
    P1->DIR |= 0x40;

    // Hold EUSCI_A3 in reset while it is configured
    EUSCI_A3->CTLW0 |= 0x01;

    // Clear the modulation control register
    EUSCI_A3->MCTLW &= ~0xFF;

    // Disable parity and select odd parity if parity were enabled
    EUSCI_A3->CTLW0 &= ~0x8000;
    EUSCI_A3->CTLW0 &= ~0x4000;

    // Use LSB first, 8-bit data, and one stop bit
    EUSCI_A3->CTLW0 &= ~0x2000;
    EUSCI_A3->CTLW0 &= ~0x1000;
    EUSCI_A3->CTLW0 &= ~0x0800;

    // Select UART asynchronous mode
    EUSCI_A3->CTLW0 &= ~0x0600;
    EUSCI_A3->CTLW0 &= ~0x0100;

    // Select SMCLK as the clock source
    EUSCI_A3->CTLW0 |= 0x00C0;

    // 12 MHz / 9600 baud = 1250
    EUSCI_A3->BRW = 1250;

    // Disable transmit-complete and start-bit interrupts
    EUSCI_A3->IE &= ~0x0C;

    // Enable transmit and receive interrupts
    EUSCI_A3->IE |= 0x03;

    // Release EUSCI_A3 from reset
    EUSCI_A3->CTLW0 &= ~0x01;
}

uint8_t BLE_UART_InChar()
{
    // Wait until a complete character has been received
    while ((EUSCI_A3->IFG & 0x01) == 0)
    {
    }

    // Reading RXBUF clears the receive flag
    return EUSCI_A3->RXBUF;
}

void BLE_UART_OutChar(uint8_t data)
{
    // Wait until the transmit buffer is ready
    while ((EUSCI_A3->IFG & 0x02) == 0)
    {
    }

    // Write the character to the transmit buffer
    EUSCI_A3->TXBUF = data;
}

int BLE_UART_InString(char *buffer_pointer, uint16_t buffer_size)
{
    int length = 0;
    int string_size = 0;
    char character = BLE_UART_InChar();

    // Continue until a line-feed character is received
    while (character != LF)
    {
        // Remove the previous character when backspace is received
        if (character == BS)
        {
            if (length > 0)
            {
                buffer_pointer--;
                length--;
                string_size--;
                BLE_UART_OutChar(BS);
            }
        }
        // Leave one space available for the null terminator
        else if (length < (buffer_size - 1))
        {
            *buffer_pointer = character;
            buffer_pointer++;
            length++;
            string_size++;
        }

        character = BLE_UART_InChar();
    }

    // Terminate the received string
    *buffer_pointer = '\0';

    return string_size;
}

void BLE_UART_OutString(char *pt)
{
    while (*pt)
    {
        BLE_UART_OutChar(*pt);
        pt++;
    }
}

uint8_t Check_BLE_UART_Data(char BLE_UART_Data_Buffer[], char *data_string)
{
    if (strstr(BLE_UART_Data_Buffer, data_string) != NULL)
    {
        return 0x01;
    }
    else
    {
        return 0x00;
    }
}


void BLE_UART_Reset()
{
    // Switch the BLE module to command mode
    P1->OUT |= 0x40;
    Clock_Delay1ms(1000);

    // Send the system-reset command
    BLE_UART_OutString("ATZ\r\n");
    Clock_Delay1ms(3000);

    // Return the BLE module to data mode
    P1->OUT &= ~0x40;
}
