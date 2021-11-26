/*
 * usb_terminal_commands.h
 *
 *  Created on: 12/05/2011
 *      Author: gustavo
 */

#ifndef USB_TERMINAL_COMMANDS_H_
#define USB_TERMINAL_COMMANDS_H_

#include "usb_terminal.h"

extern char text[384];

// BRTOS version Command
void usb_cmd_ver(char *param);
extern const command_t usb_ver_cmd;


// TOP Command (similar to the linux command)
void usb_cmd_top(char *param);
extern const command_t usb_top_cmd;


// Print a string in the terminal
void cmd_echo(char *param);
extern const command_t echo_cmd;
void echo (char *string, char Terminalbackup);

// Reason of Reset Command
void usb_cmd_rst(char *param);
extern const command_t usb_rst_cmd;

// Show ip config
void usb_cmd_ipconfig(char *param);
extern const command_t usb_ipconfig_cmd;


#endif /* USB_TERMINAL_COMMANDS_H_ */
