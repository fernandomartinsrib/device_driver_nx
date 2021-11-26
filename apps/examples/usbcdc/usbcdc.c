#include <nuttx/config.h>
#include <stdio.h>

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

#include "drivers/buttons.h"
#include "drivers/pinout.h"

#include <pthread.h>
#include <semaphore.h>
#include <sched.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

//#include <nuttx/semaphore.h>


#include <arch/board/driverlib/usb.h>
#include <arch/board/driverlib/rom.h>
#include <arch/board/usblib/usblib.h>
#include <arch/board/usblib/usbcdc.h>
#include <arch/board/usblib/usb-ids.h>
#include <arch/board/usblib/device/usbdevice.h>
#include <arch/board/usblib/device/usbdcdc.h>
#include "USB/usb_serial_structs.h"

//#include "USB/cdc/virtual_com.h"
#include "USB/cdc/usb_terminal.h"
#include "USB/cdc/usb_terminal_commands.h"


//#include <siginfo.h>

#define GPIO_PD6_USB0EPEN       0x00031805
#define SEM_PRIO_NONE             0
#define SEM_PRIO_INHERIT          1
#define SEM_PRIO_PROTECT          2

/****************************************************************************
 * Public Functions
 ****************************************************************************/

static int tiva_usb_int(int irq, FAR void *context, FAR void *arg)
{
  // printf("Entrou tiva_usb_init");
  USB0DeviceIntHandler();

  return 0;
}

// static int usbtask(int argc, char *pvarg[])
static int usbtask()
{
  usb_terminal_init();
	(void)usb_terminal_add_cmd((command_t*)&usb_ver_cmd);

  printf("usb_ver_cmd registered\r\n");
	//(void)usb_terminal_add_cmd((command_t*)&usb_top_cmd);
	//(void)usb_terminal_add_cmd((command_t*)&usb_rst_cmd);
	//(void)usb_terminal_add_cmd((command_t*)&echo_cmd);
	//(void)usb_terminal_add_cmd((command_t*)&usb_ipconfig_cmd);

  //usb_terminal_process();

	while(1)
	{
		/* Call the application task */
    //printf("usb_terminal_process\r\n");
    usb_terminal_process();
  }
}

static int ledtask(int argc, char *pvarg[])
{
  int sig_num;
  //struct siginfo ret_signal;

  /*
   * Setup for wakeup signal
   */
 //(void)sigemptyset(&g_modbus.mbs_set);
 //(void)sigaddset(&g_modbus.mbs_set, MBS_WAKEUP_SIGNAL);
  struct sched_param scheduler;

  printf("ladtask initialized!\r\n");

  scheduler.sched_priority = 80;
  (void)sched_setscheduler (0, SCHED_RR, &scheduler);

  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
  GPIOPadConfigSet(GPIO_PORTF_BASE, GPIO_PIN_0, GPIO_STRENGTH_8MA_SC, GPIO_PIN_TYPE_STD);
  GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_0);

  GPIOPadConfigSet(GPIO_PORTF_BASE, GPIO_PIN_4, GPIO_STRENGTH_8MA_SC, GPIO_PIN_TYPE_STD);
  GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_4);

  while(1){
    //board_userled(0, true);
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0, GPIO_PIN_0);
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_4, GPIO_PIN_4);
    usleep(100000);
    //board_userled(0, false);
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0, 0);
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_4, 0);
    usleep(100000);
  }
}


sem_t sKEYB;
static int tiva_keyb_int(int irq, FAR void *context, FAR void *arg)
{
  MAP_GPIOIntClear(BUTTONS_GPIO_BASE, ALL_BUTTONS);
  MAP_GPIOIntDisable(BUTTONS_GPIO_BASE, ALL_BUTTONS);

  // Call the keyboard analysis task
  (void)sem_post(&sKEYB);

  return 0;
}


static int keyboard_handler(int argc, char *pvarg[]){
  // task setup
  unsigned char  key  = NO_KEY;
  unsigned int   read = 0;

  int ret = 0;
  ret = sem_init(&sKEYB, 0, 0);

  if(ret < 0){
    printf("Error to init sem\r\n");
  }

  sem_setprotocol(&sKEYB, SEM_PRIO_NONE);

  printf("Keyboard task created!\n\r");

  // #if 0
  //     sKEYB = xSemaphoreCreateBinary();

  //     if( sKEYB == NULL ){
  //         /* There was insufficient FreeRTOS heap available for the semaphore to
  //         be created successfully. */
  //         vTaskSuspend(NULL);
  //     }
  //     else{
  //         qKEYB = xQueueCreate(128, sizeof(char));

  //         if( qKEYB == NULL ){
  //             /* There was insufficient FreeRTOS heap available for the queue to
  //             be created successfully. */
  //             vTaskSuspend(NULL);
  //         }else{
  //             ButtonsInit();
  //         }
  //     }
  // #endif

  //irq_attach(TIVA_IRQ_GPIOJ, tiva_keyb_int, NULL);
  ButtonsInit();
  irq_attach(BUTTONS_GPIO_INT, tiva_keyb_int, NULL);

  printf("Keyboard task configured!\n\r");
  // task main loop
  int teste = 0;
  for (;;){
      //#if 1
      // Wait for a keyboard interrupt
      //xSemaphoreTake(sKEYB,portMAX_DELAY);
      printf("no aguardo!\n\r");
      (void)sem_wait(&sKEYB);

      //teste = sem_wait(&sKEYB);

      printf("no aguardo pos wait %d!\n\r", teste);
      printf("Utilizando sleep");
      //vTaskDelay(50);
      usleep(50000);

      printf("no aguardo pos sleep!\n\r");

      read = MAP_GPIOPinRead(BUTTONS_GPIO_BASE, ALL_BUTTONS);

      printf("no aguardo pos read!\n\r");
      // Find out which key was pressed
      key = (unsigned char)read;

      printf("Key: %c\n\r", key);
      
      // Copy the key to the keyboard buffer
      if(key != NO_KEY){
          printf("Tecla pressionada!\n\r");
          //(void)execl ("/ps", "ps", (char *)0);
          system("ps");
          //xQueueSendToBack(qKEYB, &key,portMAX_DELAY);
      }

      key = NO_KEY;
      //#endif
      usleep(50000);
      // Enable interrupt to the next key detection
      MAP_GPIOIntEnable(BUTTONS_GPIO_BASE, ALL_BUTTONS);
  }
}
/****************************************************************************
 * usbcdc_main
 ****************************************************************************/

//#if defined(BUILD_MODULE)
#ifdef CONFIG_BUILD_KERNEL
int main(int argc, FAR char *argv[])
#else
int usbcdc_main(int argc, char *argv[])
#endif
{
  printf("usbcdc works!!\n");

  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPION);
  GPIOPadConfigSet(GPIO_PORTN_BASE, GPIO_PIN_1, GPIO_STRENGTH_8MA_SC, GPIO_PIN_TYPE_STD);
  GPIOPinTypeGPIOOutput(GPIO_PORTN_BASE, GPIO_PIN_1);
  GPIOPadConfigSet(GPIO_PORTN_BASE, GPIO_PIN_0, GPIO_STRENGTH_8MA_SC, GPIO_PIN_TYPE_STD);
  GPIOPinTypeGPIOOutput(GPIO_PORTN_BASE, GPIO_PIN_0);

  int ret_state = 0;

  //ret_state = task_create("keyb handler", 200,  PTHREAD_STACK_DEFAULT, keyboard_handler,  NULL);
  //ret_state = task_create("ledtask",      200,  PTHREAD_STACK_DEFAULT, ledtask,           NULL);
  //ret_state = task_create("usb console",  200,  PTHREAD_STACK_DEFAULT, usbtask,           NULL);

  int ret = 0;
  ret = usbtask();

  //board_userled_initialize();
  // while(1){
  //   //board_userled(0, true);
  //   GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0, 0);
  //   GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_1, 0);
  //   GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0, GPIO_PIN_0);
  //   GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_4, GPIO_PIN_4);
  //   usleep(100000);
  //   //board_userled(0, false);
  //   GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0, GPIO_PIN_0);
  //   GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_1, GPIO_PIN_1);
  //   GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0, 0);
  //   GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_4, 0);
  //   usleep(100000);
  // }

  return 0;
}
