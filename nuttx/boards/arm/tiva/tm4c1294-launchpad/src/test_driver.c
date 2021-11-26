#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <debug.h>
#include <errno.h>
#include <sched.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <inttypes.h>
#include <fcntl.h>

#include <nuttx/config.h>
#include <nuttx/board.h>

#include "tiva_gpio.h"
#include "tm4c1294-launchpad.h"
#include <mqueue.h>

#include <arch/board/inc/hw_ints.h>
#include <arch/board/inc/hw_memmap.h>
#include <arch/board/inc/hw_types.h>
#include <arch/board/inc/hw_uart.h>

#include <arch/board/driverlib/rom.h>
#include <arch/board/driverlib/rom_map.h>
#include <arch/board/driverlib/pin_map.h>
#include <arch/board/driverlib/timer.h>
#include <arch/board/driverlib/sysctl.h>
#include <arch/board/driverlib/interrupt.h>
#include <arch/board/driverlib/watchdog.h>
#include <arch/board/driverlib/gpio.h>
#include <arch/board/driverlib/uart.h>
#include <arch/board/driverlib/usb.h>
#include <arch/board/driverlib/rom.h>
#include <arch/board/driverlib/debug.h>

#include <arch/board/usblib/usblib.h>
#include <arch/board/usblib/usbcdc.h>
#include <arch/board/usblib/usb-ids.h>
#include <arch/board/usblib/device/usbdevice.h>
#include <arch/board/usblib/device/usbdcdc.h>

#include "drivers/buttons.h"
#include "drivers/pinout.h"

#include "USB/usb_serial_structs.h"
#include "USB/cdc/virtual_com.h"

#define GPIO_PD6_USB0EPEN       0x00031805
#define SEM_PRIO_NONE             0
#define SEM_PRIO_INHERIT          1
#define SEM_PRIO_PROTECT          2

#define QUEUE_NAME	"/usb_queue"

FAR sem_t sRead;
FAR sem_t sWrite;

static int tiva_usb_int(int irq, FAR void *context, FAR void *arg)
{
  USB0DeviceIntHandler();
  return 0;
}

static void usbtask(void)
{
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOL);
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOQ);
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_USB0);

    //  
    // Configure the device pins.
    //
    //HWREG(GPIO_PORTD_BASE + GPIO_O_LOCK) = GPIO_LOCK_KEY;
    //HWREG(GPIO_PORTD_BASE + GPIO_O_CR) = 0xff;

    MAP_GPIOPinConfigure(GPIO_PD6_USB0EPEN);
    MAP_GPIOPinTypeUSBAnalog(GPIO_PORTB_BASE, GPIO_PIN_0 | GPIO_PIN_1);
    MAP_GPIOPinTypeUSBDigital(GPIO_PORTD_BASE, GPIO_PIN_6);
    MAP_GPIOPinTypeUSBAnalog(GPIO_PORTL_BASE, GPIO_PIN_6 | GPIO_PIN_7);
    MAP_GPIOPinTypeGPIOInput(GPIO_PORTQ_BASE, GPIO_PIN_4);

    //
    // Not configured initially.
    //
    g_bUSBConfigured = false;
    irq_attach(INT_USB0_TM4C129, tiva_usb_int, NULL);

    USBBufferInit(&g_sTxBuffer);
    USBBufferInit(&g_sRxBuffer);

    USBStackModeSet(0, eUSBModeForceDevice, 0);
    USBDCDCInit(0, &g_sCDCDevice);
    Virtual_Comm_Init();
}

/****************************************************************************
 * LEDs: Fileops Prototypes and Structures
 ****************************************************************************/

typedef FAR struct file file_t;

static int      cdc_open(file_t *filep);
static int      cdc_close(file_t *filep);
static ssize_t  cdc_read(file_t *filep, void *buffer, size_t buflen);
static ssize_t  cdc_write(file_t *filep, void *buf, size_t buflen);
static int      cdc_ioctl(file_t *filep, int request, unsigned long arg);

static const struct file_operations usb_cdc_ops = {
    cdc_open,      /* open */
    cdc_close,     /* close */
    cdc_read,      /* read */
    cdc_write,     /* write */
    0,             /* seek */
    cdc_ioctl,     /* ioctl */
};

/****************************************************************************
 * LEDs: Fileops
 ****************************************************************************/

static int cdc_open(file_t *filep)
{   
    return 1;
}

static int cdc_close(file_t *filep)
{
    return 1;
}

static ssize_t cdc_read(file_t *filep, void *buf, size_t buflen)
{
    ssize_t len = 0;
    mqd_t  mq;
    ssize_t ulRead = 0;

    if(buf == NULL || buflen < 1)
    {
        return -EINVAL;
    }
        
    (void)nxsem_wait(&sRead);
    mq = mq_open(QUEUE_NAME, O_RDONLY | O_NONBLOCK); 
    ulRead = mq_receive(mq, buf, buflen, NULL);
    (void)nxsem_post(&sRead);
    
    mq_close(mq);

    return ulRead;
}

static ssize_t cdc_write(file_t *filep, void *buf, size_t buflen)
{
    unsigned char buf_aux[buflen];
    int status = 0;
    ssize_t nbytes = 0;
    mqd_t  mq;

    char carac = ((char *) buf) [0];    

    (void)nxsem_wait(&sWrite); 
    uint32_t written = 0;
    
    for(int i = 0; i < buflen; i++){
        buf_aux[i] = ((char *) buf) [i];
        nbytes += USBBufferWrite((tUSBBuffer *)&g_sTxBuffer, &buf_aux[i], 1);
    }

    (void)nxsem_post(&sWrite);

    //mq_close(mq);

    return nbytes;
}

static int cdc_ioctl(file_t *filep, int request, unsigned long arg){
    switch(request)
    {
        case 1:
        {
            tLineCoding psLineCoding;
            
            psLineCoding.ui8Databits = 7;
            psLineCoding.ui8Parity = USB_CDC_PARITY_ODD;
            psLineCoding.ui8Stop = USB_CDC_STOP_BITS_1;

            uint32_t ui32Event = USBD_CDC_EVENT_SET_LINE_CODING;
            uint32_t ui32MsgValue = 0;
            uint32_t ret = ControlHandler(0, ui32Event, ui32MsgValue, &psLineCoding);

            if(!ret){
                printf("\nError to use SetLineCoding\n");
            }

            tLineCoding rsLineCoding;

            GetLineCoding(&rsLineCoding);
            
            printf("\rsLineCoding->ui32Rate: %d\n", rsLineCoding.ui32Rate);
            printf("\rsLineCoding->ui8Databits: %d\n", rsLineCoding.ui8Databits);
            printf("\rsLineCoding->ui8Parity: %d\n", rsLineCoding.ui8Parity);
            printf("\rsLineCoding->ui8Stop: %d\n", rsLineCoding.ui8Stop);
        }
        break;
        case 2:
        {
            tLineCoding psLineCoding;

            GetLineCoding(&psLineCoding);
            
            printf("\npsLineCoding->ui32Rate: %d\n", psLineCoding.ui32Rate);
            printf("\npsLineCoding->ui8Databits: %d\n", psLineCoding.ui8Databits);
            printf("\npsLineCoding->ui8Parity: %d\n", psLineCoding.ui8Parity);
            printf("\npsLineCoding->ui8Stop: %d\n", psLineCoding.ui8Stop);
        }
        break;
        case 3:
        {
            
        }
    }
    return 0;
}

/****************************************************************************
 * Initialize device, add /dev/... nodes
 ****************************************************************************/

void up_usbcdc(void){

    int ret = 0;
    ret = nxsem_init(&sRead,0,0);
    (void)nxsem_post(&sRead);

    int ret2 = 0;
    ret2 = nxsem_init(&sWrite,0,0);
    (void)nxsem_post(&sWrite);

    //sem_init(&sem, 0, 0);
    sem_setprotocol(&sRead, SEM_PRIO_NONE);
    sem_setprotocol(&sWrite, SEM_PRIO_NONE);

    usbtask();

    tiva_configgpio(GPIO_LED_D1);
    tiva_configgpio(GPIO_LED_D2);
    tiva_configgpio(GPIO_LED_D3);
    tiva_configgpio(GPIO_LED_D4);

    int reg;

    reg = register_driver("/dev/usb0", &usb_cdc_ops, 0644, NULL);
    if (reg < 0)
    {
        printf("\nusb_cdc driver register_driver failed: %d\n", reg);
    }
    printf("\nusb_cdc driver initialized successfully!\n");
}

 