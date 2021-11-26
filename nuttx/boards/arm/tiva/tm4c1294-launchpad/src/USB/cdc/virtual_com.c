#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <inttypes.h>
#include <fcntl.h>

#include "virtual_com.h"
#include <mqueue.h>

#include <arch/board/inc/hw_memmap.h>
#include <arch/board/inc/hw_types.h>
#include <arch/board/inc/hw_uart.h>
#include <arch/board/driverlib/sysctl.h>
#include <arch/board/driverlib/rom_map.h>



#define QUEUE_NAME	"/usb_queue"

//*****************************************************************************
//
// Global flag indicating that a USB configuration has been set.
//
//*****************************************************************************
volatile bool g_bUSBConfigured = false;
volatile unsigned long g_ulFlags = 0;
volatile char *g_pcStatus;
uint32_t g_ui32SysClock = 0;


//*****************************************************************************
//
// This function is called whenever serial data is received from the UART.
// It is passed the accumulated error flags from each character received in
// this interrupt and determines from them whether or not an interrupt
// notification to the host is required.
//
// If a notification is required and the control interrupt endpoint is idle,
// we send the notification immediately.  If the endpoint is not idle, we
// accumulate the errors in a global variable which will be checked on
// completion of the previous notification and used to send a second one
// if necessary.
//
//*****************************************************************************






//*****************************************************************************
//
// Set the state of the RS232 RTS and DTR signals.
//
//*****************************************************************************
void SetControlLineState(unsigned short usState)
{
}

//*****************************************************************************
//
// Set the communication parameters to use on the UART.
//
//*****************************************************************************
bool SetLineCoding(tLineCoding *psLineCoding)
{
    uint32_t ui32Config;
    bool bRetcode;

    //
    // Assume everything is OK until we detect any problem.
    //
    bRetcode = true;

    //printf("\nrecebido: %d\n", psLineCoding->ui8Databits);
    //
    // Word length.  For invalid values, the default is to set 8 bits per
    // character and return an error.
    //
    switch(psLineCoding->ui8Databits)
    {
        case 5:
        {
            ui32Config = UART_CONFIG_WLEN_5;
            break;
        }

        case 6:
        {
            ui32Config = UART_CONFIG_WLEN_6;
            break;
        }

        case 7:
        {
            ui32Config = UART_CONFIG_WLEN_7;
            break;
        }

        case 8:
        {
            ui32Config = UART_CONFIG_WLEN_8;
            break;
        }

        default:
        {
            ui32Config = UART_CONFIG_WLEN_8;
            bRetcode = false;
            break;
        }
    }

    //
    // Parity.  For any invalid values, we set no parity and return an error.
    //
    switch(psLineCoding->ui8Parity)
    {
        case USB_CDC_PARITY_NONE:
        {
            ui32Config |= UART_CONFIG_PAR_NONE;
            break;
        }

        case USB_CDC_PARITY_ODD:
        {
            ui32Config |= UART_CONFIG_PAR_ODD;
            break;
        }

        case USB_CDC_PARITY_EVEN:
        {
            ui32Config |= UART_CONFIG_PAR_EVEN;
            break;
        }

        case USB_CDC_PARITY_MARK:
        {
            ui32Config |= UART_CONFIG_PAR_ONE;
            break;
        }

        case USB_CDC_PARITY_SPACE:
        {
            ui32Config |= UART_CONFIG_PAR_ZERO;
            break;
        }
        

        default:
        {
            ui32Config |= UART_CONFIG_PAR_NONE;
            bRetcode = false;
            break;
        }
    }

    //
    // Stop bits.  Our hardware only supports 1 or 2 stop bits whereas CDC
    // allows the host to select 1.5 stop bits.  If passed 1.5 (or any other
    // invalid or unsupported value of ui8Stop, we set up for 1 stop bit but
    // return an error in case the caller needs to Stall or otherwise report
    // this back to the host.
    //
    switch(psLineCoding->ui8Stop)
    {
        //
        // One stop bit requested.
        //
        case USB_CDC_STOP_BITS_1:
        {
            ui32Config |= UART_CONFIG_STOP_ONE;
            break;
        }

        //
        // Two stop bits requested.
        //
        case USB_CDC_STOP_BITS_2:
        {
            ui32Config |= UART_CONFIG_STOP_TWO;
            break;
        }

        //
        // Other cases are either invalid values of ui8Stop or values that we
        // cannot support so set 1 stop bit but return an error.
        //
        default:
        {
            ui32Config |= UART_CONFIG_STOP_ONE;
            bRetcode = false;
            break;
        }
    }

    //
    // Set the UART mode appropriately.
    //

    // UARTConfigSetExpClk(UARTA_BASE, SysCtlLowSpeedClockGet(SYSTEM_CLOCK_SPEED),
    //                         readusb32_t(&(psLineCoding->ui32Rate)), ui32Config);

    //
    // Let the caller know if we had a problem or not.
    //
    return(bRetcode);
}

//*****************************************************************************
//
// Get the communication parameters in use on the UART.
//
//*****************************************************************************
void GetLineCoding(tLineCoding *psLineCoding)
{
    uint32_t ui32Config;
    //uint32_t ui32Rate;

    //
    // Get the current line coding set in the UART.
    //
    ui32Config = DEFAULT_UART_CONFIG;
    psLineCoding->ui32Rate = DEFAULT_BIT_RATE;

    //
    // Translate the configuration word length field into the format expected
    // by the host.
    //
    switch(ui32Config & UART_CONFIG_WLEN_MASK)
    {
        case UART_CONFIG_WLEN_8:
        {
            psLineCoding->ui8Databits = 8;
            break;
        }

        case UART_CONFIG_WLEN_7:
        {
            psLineCoding->ui8Databits = 7;
            break;
        }

        case UART_CONFIG_WLEN_6:
        {
            psLineCoding->ui8Databits = 6;
            break;
        }

        case UART_CONFIG_WLEN_5:
        {
            psLineCoding->ui8Databits = 5;
            break;
        }
    }

    //
    // Translate the configuration parity field into the format expected
    // by the host.
    //
    switch(ui32Config & UART_CONFIG_PAR_MASK)
    {
        case UART_CONFIG_PAR_NONE:
        {
            psLineCoding->ui8Parity = USB_CDC_PARITY_NONE;
            break;
        }

        case UART_CONFIG_PAR_ODD:
        {
            psLineCoding->ui8Parity = USB_CDC_PARITY_ODD;
            break;
        }

        case UART_CONFIG_PAR_EVEN:
        {
            psLineCoding->ui8Parity = USB_CDC_PARITY_EVEN;
            break;
        }

        case UART_CONFIG_PAR_ONE:
        {
            psLineCoding->ui8Parity = USB_CDC_PARITY_MARK;
            break;
        }

        case UART_CONFIG_PAR_ZERO:
        {
            psLineCoding->ui8Parity = USB_CDC_PARITY_SPACE;
            break;
        }
    }

    //
    // Translate the configuration stop bits field into the format expected
    // by the host.
    //
    switch(ui32Config & UART_CONFIG_STOP_MASK)
    {
        case UART_CONFIG_STOP_ONE:
        {
            psLineCoding->ui8Stop = USB_CDC_STOP_BITS_1;
            break;
        }

        case UART_CONFIG_STOP_TWO:
        {
            psLineCoding->ui8Stop = USB_CDC_STOP_BITS_2;
            break;
        }
    }
}

//*****************************************************************************
//
// This function sets or clears a break condition on the redirected UART RX
// line.  A break is started when the function is called with \e bSend set to
// \b true and persists until the function is called again with \e bSend set
// to \b false.
//
//*****************************************************************************
//*****************************************************************************
//
// Flag indicating whether or not we are currently sending a Break condition.
//
//*****************************************************************************
//static bool g_bSendingBreak = false;
void SendBreak(bool bSend)
{
    //
    // Are we being asked to start or stop the break condition?
    //
    if(!bSend)
    {
        //
        // Remove the break condition on the line.
        //
        //ROM_UARTBreakCtl(USB_UART_BASE, false);
        //g_bSendingBreak = false;
    }
    else
    {
        //
        // Start sending a break condition on the line.
        //
        //ROM_UARTBreakCtl(USB_UART_BASE, true);
        //g_bSendingBreak = true;
    }
}

//*****************************************************************************
//
// Handles CDC driver notifications related to control and setup of the device.
//
// \param pvCBData is the client-supplied callback pointer for this channel.
// \param ulEvent identifies the event we are being notified about.
// \param ulMsgValue is an event-specific value.
// \param pvMsgData is an event-specific pointer.
//
// This function is called by the CDC driver to perform control-related
// operations on behalf of the USB host.  These functions include setting
// and querying the serial communication parameters, setting handshake line
// states and sending break conditions.
//
// \return The return value is event-specific.
//
//*****************************************************************************
uint32_t
ControlHandler(void *pvCBData, uint32_t ui32Event,
               uint32_t ui32MsgValue, void *pvMsgData)
{
	static int flag = 0;

    //
    // Which event are we being asked to process?
    //
    switch(ui32Event)
    {
        //
        // We are connected to a host and communication is now possible.
        //
        case USB_EVENT_CONNECTED:
            g_bUSBConfigured = true;

            //
            // Flush our buffers.
            //
            USBBufferFlush(&g_sTxBuffer);
            USBBufferFlush(&g_sRxBuffer);

            //
            // Tell the main loop to update the display.
            //
            //OSEnterCritical();
            g_pcStatus = "Connected";
            g_ulFlags |= COMMAND_STATUS_UPDATE;
            //OSExitCritical();
            break;

        //
        // The host has disconnected.
        //
        case USB_EVENT_DISCONNECTED:
            g_bUSBConfigured = false;
            g_pcStatus = "Disconnected";
            g_ulFlags |= COMMAND_STATUS_UPDATE;
            break;

        //
        // Return the current serial communication parameters.
        //
        case USBD_CDC_EVENT_GET_LINE_CODING:
            GetLineCoding(pvMsgData);
            break;

        //
        // Set the current serial communication parameters.
        //
        case USBD_CDC_EVENT_SET_LINE_CODING:
            SetLineCoding(pvMsgData);
        	if (flag == 0)
        	{
        		flag = 1;
                // char start_message = ">> Nuttx RTOS Started!\n\r>";
        		//USBBufferWrite((tUSBBuffer *)&g_sTxBuffer, (unsigned char *)">> Nuttx RTOS Started!\n\r>", sizeof(start_message));
        	}else
        	{
        		flag = 0;
        	}
            break;

        //
        // Set the current serial communication parameters.
        //
        case USBD_CDC_EVENT_SET_CONTROL_LINE_STATE:
            SetControlLineState((unsigned short)ui32MsgValue);
            break;

        //
        // Send a break condition on the serial line.
        //
        case USBD_CDC_EVENT_SEND_BREAK:
            SendBreak(true);
            break;

        //
        // Clear the break condition on the serial line.
        //
        case USBD_CDC_EVENT_CLEAR_BREAK:
            SendBreak(false);
            break;

        //
        // Ignore SUSPEND and RESUME for now.
        //
        case USB_EVENT_SUSPEND:
        case USB_EVENT_RESUME:
            break;

        //
        // We don't expect to receive any other events.  Ignore any that show
        // up in a release build or hang in a debug build.
        //
        default:
#ifdef DEBUG
            while(1);
#else
            break;
#endif

    }
    return(0);
}

//*****************************************************************************
//
// Handles CDC driver notifications related to the transmit channel (data to
// the USB host).
//
// \param ulCBData is the client-supplied callback pointer for this channel.
// \param ulEvent identifies the event we are being notified about.
// \param ulMsgValue is an event-specific value.
// \param pvMsgData is an event-specific pointer.
//
// This function is called by the CDC driver to notify us of any events
// related to operation of the transmit data channel (the IN channel carrying
// data to the USB host).
//
// \return The return value is event-specific.
//
//*****************************************************************************
uint32_t
TxHandler(void *pvCBData, uint32_t ui32Event, uint32_t ui32MsgValue,
          void *pvMsgData)
{
    //
    // Which event have we been sent?
    //
    switch(ui32Event)
    {
        case USB_EVENT_TX_COMPLETE:
            //
            // Since we are using the USBBuffer, we don't need to do anything
            // here.
            //
            break;

        //
        // We don't expect to receive any other events.  Ignore any that show
        // up in a release build or hang in a debug build.
        //
        default:
#ifdef DEBUG
        while(1);
#else
        break;
#endif

    }
    return(0);
}

//*****************************************************************************
//
// Handles CDC driver notifications related to the receive channel (data from
// the USB host).
//
// \param ulCBData is the client-supplied callback data value for this channel.
// \param ulEvent identifies the event we are being notified about.
// \param ulMsgValue is an event-specific value.
// \param pvMsgData is an event-specific pointer.
//
// This function is called by the CDC driver to notify us of any events
// related to operation of the receive data channel (the OUT channel carrying
// data from the USB host).
//
// \return The return value is event-specific.
//
//*****************************************************************************
void USBUARTPrimeTransmit(void)
{
    mqd_t  mq;
    unsigned long ulRead;
    unsigned long read_bytes;
    unsigned char ucChar[32];

    // mq = mq_open(QUEUE_NAME, O_WRONLY);
    // printf("\nmq - mq_open driver usbcdc: %d", mq);

    // struct mq_attr attr;
    
    // if (mq_getattr(mq, &attr) == -1) {
    //     perror("mq_getattr");
    //     return 1;
    // }
    // printf("\nmq_flags %ld\n",  attr.mq_flags);
    // printf("mq_maxmsg %ld\n", attr.mq_maxmsg);
    // printf("mq_msgsize %ld\n",attr.mq_msgsize);
    // printf("mq_curmsgs %ld\n",attr.mq_curmsgs);
    // Analisar aqui
    //
    // Get a character from the buffer.
    //

	ulRead = USBBufferRead((tUSBBuffer *)&g_sRxBuffer, (unsigned char *)&ucChar, 32);

    uint32_t written = 0;
    int status = 0;
	if(ulRead)
	{
        

		read_bytes = 0;
		do
		{
            mq = mq_open(QUEUE_NAME, O_EXCL | O_RDWR);

            status = mq_send(mq,&ucChar[read_bytes],1,0);

            mq_close(mq);

            if(status == -1){
                perror("mq_send failure on mqfd");
            }
            written = USBBufferWrite((tUSBBuffer *)&g_sTxBuffer, &ucChar[read_bytes], 1);

			read_bytes++;
			ulRead--;
		}while(ulRead);

        
		#if 0
		if (ucChar[0] == '1') USBBufferWrite((tUSBBuffer *)&g_sTxBuffer, (unsigned char *)&txChar1, 30);
		if (ucChar[0] == '2') USBBufferWrite((tUSBBuffer *)&g_sTxBuffer, (unsigned char *)&txChar2, 30);
		#endif
	}
}


uint32_t
RxHandler(void *pvCBData, uint32_t ui32Event, uint32_t ui32MsgValue,
          void *pvMsgData)
{
    //uint32_t ui32Count;

    //
    // Which event are we being sent?
    //
    switch(ui32Event)
    {
        //
        // A new packet has been received.
        //
        case USB_EVENT_RX_AVAILABLE:
        {
            //
            // Feed some characters into the UART TX FIFO and enable the
            // interrupt so we are told when there is more space.
            //
            USBUARTPrimeTransmit();
            break;
        }

        //
        // We are being asked how much unprocessed data we have still to
        // process. We return 0 if the UART is currently idle or 1 if it is
        // in the process of transmitting something. The actual number of
        // bytes in the UART FIFO is not important here, merely whether or
        // not everything previously sent to us has been transmitted.
        //
        case USB_EVENT_DATA_REMAINING:
        {
            //
            // Get the number of bytes in the buffer and add 1 if some data
            // still has to clear the transmitter.
            //
            //ui32Count = ROM_UARTBusy(USB_UART_BASE) ? 1 : 0;
            //return(ui32Count);
        }

        //
        // We are being asked to provide a buffer into which the next packet
        // can be read. We do not support this mode of receiving data so let
        // the driver know by returning 0. The CDC driver should not be sending
        // this message but this is included just for illustration and
        // completeness.
        //
        case USB_EVENT_REQUEST_BUFFER:
        {
            return(0);
        }

        //
        // We don't expect to receive any other events.  Ignore any that show
        // up in a release build or hang in a debug build.
        //
        default:
#ifdef DEBUG
            while(1);
#else
            break;
#endif
    }

    return(0);
}



void putchar_usb(char c)
{
	uint32_t written = 0;
	do
	{
		written = USBBufferWrite((tUSBBuffer *)&g_sTxBuffer, (unsigned char *)&c, 1);
	}while(written != 1);
}
/*****************************************************************************
 * Name:
 *    print
 * In:
 *    s: string
 * Out:
 *    n/a
 *
 * Description:
 *    Print the specified string.
 * Assumptions:
 *
 *****************************************************************************/
void printf_usb(char *s)
{
	uint32_t count = 0;
	uint32_t written = 0;
	uint32_t char_to_written = 0;
	uint32_t written_char = 0;
	char  *string = s;

	while(*string)
	{
		count++;
		string++;
	}

	char_to_written = count;
	do
	{
		written = USBBufferWrite((tUSBBuffer *)&g_sTxBuffer, (unsigned char *)&s[written_char], char_to_written);
		written_char += written;
		char_to_written -= written;
	}while(written_char != count);
}



void Virtual_Comm_Init(void)
{
    struct mq_attr attr;
    mqd_t mq;

    // Initialize queue attributes
    attr.mq_flags = 0; //0;
    attr.mq_maxmsg = 64;
    attr.mq_msgsize = 1;
    attr.mq_curmsgs = 0;

    // Cria fila de mensagens
    //mq = mq_open(QUEUE_NAME, O_CREAT | O_RDONLY, 0644, &attr);
    //mq = mq_open(QUEUE_NAME, O_CREAT | O_RDWR, 0644, &attr);
    mq = mq_open(QUEUE_NAME, O_CREAT, 0644, &attr);
    mq_close(mq);
}
