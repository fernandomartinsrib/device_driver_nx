#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <nuttx/config.h>

 
#include <arch/board/inc/hw_memmap.h>
#include <arch/board/driverlib/debug.h>
#include <arch/board/driverlib/gpio.h>
#include <arch/board/driverlib/sysctl.h>


#ifdef CONFIG_BUILD_KERNEL
int main(int argc, char *argv[])
#else
int blinky_main(int argc, FAR char *argv[])
#endif
{
    printf("Blinky works\n");

    volatile uint32_t ui32Loop;

    //
    // Enable the GPIO port that is used for the on-board LED.
    //
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPION);

    //
    // Check if the peripheral access is enabled.
    //
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPION))
    {
    }

    //
    // Enable the GPIO pin for the LED (PN0).  Set the direction as output, and
    // enable the GPIO pin for digital function.
    //
    GPIOPinTypeGPIOOutput(GPIO_PORTN_BASE, GPIO_PIN_0);

    //
    // Loop forever.
    //
    while(1)
    {
        //
        // Turn on the LED.
        //
        GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0, GPIO_PIN_0);

        //
        // Delay for a bit.
        //
        for(ui32Loop = 0; ui32Loop < 800000; ui32Loop++)
        {
        }

        //
        // Turn off the LED.
        //
        GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0, 0x0);

        //
        // Delay for a bit.
        //
        for(ui32Loop = 0; ui32Loop < 800000; ui32Loop++)
        {
        }
    }
}
