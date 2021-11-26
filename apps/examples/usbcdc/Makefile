include $(APPDIR)/Make.defs

EXTRADEFINES += -DPART_TM4C1294NCPDT -DTARGET_IS_TM4C129_RA0 -Dgcc

# Hello, World! built-in application info

PROGNAME  = $(CONFIG_EXAMPLES_USBCDC_PROGNAME)
PRIORITY  = $(CONFIG_EXAMPLES_USBCDC_PRIORITY)
STACKSIZE = $(CONFIG_EXAMPLES_USBCDC_STACKSIZE)
MODULE    = $(CONFIG_EXAMPLES_USBCDC)

# Hello, World! Example

MAINSRC = usbcdc.c
CSRCS = drivers/buttons.c drivers/pinout.c USB/usb_serial_structs.c USB/cdc/usb_terminal.c USB/cdc/usb_terminal_commands.c

include $(APPDIR)/Application.mk
