/****************************************************************************************
* 
*   FILE        :    usb_terminal_commands.c
*   DATE CREATED:    
*   DESCRIPTION	:    
*
*   Author		:     
*  	Location	:    
*   Purpose		:
*   Copyright	:    
*                                                  
* UPDATED HISTORY:
*
* REV   YYYY.MM.DD  AUTHOR          	DESCRIPTION OF CHANGE
* ---   ----------  ------          	--------------------- 
* 1.0   2011.05.12  Gustavo Denardin	Initial version
*
****************************************************************************************/

/*
 * usb_terminal_commands.c
 *
 *  Created on: 12/05/2011
 *      Author: gustavo
 */

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "usb_terminal.h"
#include "usb_terminal_commands.h"
//#include "virtual_com.h"

uint8_t entradas[CONSOLE_BUFFER_SIZE]; //vetor para a entrada de dados

// BRTOS version Command
void usb_cmd_ver(char *param)
{
  (void)*param;
  printf_usb("\n\r");
  printf_usb("Nuttx ver. 7.5");
  printf_usb("\n\r");

}

const command_t usb_ver_cmd = {
  "ver", usb_cmd_ver, "Nuttx Version"
};

