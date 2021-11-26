#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <inttypes.h>

#include <arch/board/inc/hw_types.h>
#include <arch/board/inc/hw_gpio.h>
#include <arch/board/inc/hw_uart.h>
#include <arch/board/inc/hw_sysctl.h>
#include <arch/board/driverlib/gpio.h>
#include <arch/board/driverlib/pin_map.h>
#include <arch/board/driverlib/uart.h>
#include <arch/board/driverlib/usb.h>
#include <arch/board/driverlib/rom.h>
#include <arch/board/usblib/usblib.h>
#include <arch/board/usblib/usbcdc.h>
#include <arch/board/usblib/usb-ids.h>
#include <arch/board/usblib/device/usbdevice.h>
#include <arch/board/usblib/device/usbdcdc.h>
#include "../usb_serial_structs.h"
// #include "usb_terminal.h"


//*****************************************************************************
//
// Default line coding settings for the redirected UART.
//
//*****************************************************************************
#define DEFAULT_BIT_RATE        115200
#define DEFAULT_UART_CONFIG     (UART_CONFIG_WLEN_8 | UART_CONFIG_PAR_NONE | UART_CONFIG_STOP_ONE)

//*****************************************************************************
//
// GPIO peripherals and pins muxed with the redirected UART.  These will depend
// upon the IC in use and the UART selected in USB_UART_BASE.  Be careful that
// these settings all agree with the hardware you are using.
//
//*****************************************************************************

//*****************************************************************************
//
// Flags used to pass commands from interrupt context to the main loop.
//
//*****************************************************************************
#define COMMAND_PACKET_RECEIVED 0x00000001
#define COMMAND_STATUS_UPDATE   0x00000002

extern volatile unsigned long g_ulFlags;
extern volatile char *g_pcStatus;
extern volatile bool g_bUSBConfigured;


//*****************************************************************************
//
// Internal function prototypes.
//
//*****************************************************************************
void SetControlLineState(unsigned short usState);
bool SetLineCoding(tLineCoding *psLineCoding);
void GetLineCoding(tLineCoding *psLineCoding);
void SendBreak(bool bSend);
void putchar_usb(char c);
void printf_usb(char *s);
void Virtual_Comm_Init(void);