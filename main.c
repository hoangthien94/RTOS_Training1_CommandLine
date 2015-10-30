#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "driverlib/debug.h"
#include "driverlib/eeprom.h"
#include "driverlib/fpu.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "driverlib/rom_map.h"
#include "utils/ustdlib.h"
#include "utils/uartstdio.h"
#include "utils/cmdline.h"

// define process status constant
#define 	PROCESS_CMD_SUCCESS			0
#define		PROCESS_CMD_FAIL				-1

// Input buffer for commands

uint8_t ui8InputBuffer[128];

//*****************************************************************************
//
// The error routine that is called if the driver library encounters an error.
//
//*****************************************************************************
#ifdef DEBUG
void
__error__(char *pcFilename, uint32_t ui32Line)
{
    UARTprintf("Error at line %d of %s\n", ui32Line, pcFilename);
    while(1)
    {
    }
}
#endif

//*****************************************************************************
//
// Command: help
//
// Description: Print the help strings for all commands.
// Param: none
// Return: 
//*****************************************************************************
int CMD_help(int argc, char **argv)
{
    int32_t i32Index;

    (void)argc;
    (void)argv;

    //
    // Start at the beginning of the command table
    //
    i32Index = 0;

    //
    // Get to the start of a clean line on the serial output.
    //
    UARTprintf("\nAvailable Commands\n------------------\n\n");

    //
    // Display strings until we run out of them.
    //
    while(g_psCmdTable[i32Index].pcCmd)
    {
      UARTprintf("%17s %s\n", g_psCmdTable[i32Index].pcCmd,
                 g_psCmdTable[i32Index].pcHelp);
      i32Index++;
    }

    //
    // Leave a blank line after the help strings.
    //

    UARTprintf("\n");

    return PROCESS_CMD_SUCCESS;
}

//*****************************************************************************
//
// Command: echo
//
//*****************************************************************************
int
CMD_echo(int argc, char **argv) {
  int i;
  for (i = 0 ; i < argc; i++) 
    UARTprintf("%s%s", i ? " " : "",argv[i]);
  UARTprintf("\n");
  return PROCESS_CMD_SUCCESS;
}

//*****************************************************************************
//
// Command: Toggle led
//
//*****************************************************************************
int CMD_led(int argc, char **argv) 
{
  if (argc < 2)
    return (CMDLINE_TOO_FEW_ARGS);

  if (argc > 2)
    return (CMDLINE_TOO_MANY_ARGS);

  if (strcmp(argv[1],"on") == 0)  {
    ROM_GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, GPIO_PIN_2);
  }
  else if (strcmp(argv[1],"off") == 0 ) {
    ROM_GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, 0);
  }
  else
    return (CMDLINE_INVALID_ARG);
  return PROCESS_CMD_SUCCESS;
}
//*****************************************************************************
//
// Table of valid command strings, callback functions and help messages.
//
//*****************************************************************************
tCmdLineEntry g_psCmdTable[] =
{
    {"help",     CMD_help,      " : Display list of commands" },
		{"echo",     CMD_echo,      " : Echo Arguments"},
    {"led",      CMD_led,       " : set led [on|off]"},
    {0,0,0}
};

//*****************************************************************************
//
// Configure the UART and its pins.  This must be called before UARTprintf().
//
//*****************************************************************************
void
ConfigureUART(void)
{
	    //
    // Enable the GPIO Peripheral used by the UART.
    //
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);

    //
    // Enable UART0
    //
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);

    //
    // Configure GPIO Pins for UART mode.
    //
    ROM_GPIOPinConfigure(GPIO_PA0_U0RX);
    ROM_GPIOPinConfigure(GPIO_PA1_U0TX);
    ROM_GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    //
    // Use the internal 16MHz oscillator as the UART clock source.
    //
    ROM_UARTClockSourceSet(UART0_BASE, UART_CLOCK_PIOSC);

    //
    // Initialize the UART for console I/O.
    //
    UARTStdioConfig(0, 115200, 16000000);
}

int
main(void) 
{
    // Enable lazy stacking for interrupt handlers.

    ROM_FPUEnable();
    ROM_FPULazyStackingEnable();

    // Set the clocking to run directly from the crystal.

    ROM_SysCtlClockSet(SYSCTL_SYSDIV_1 | SYSCTL_USE_OSC | SYSCTL_OSC_MAIN |
                       SYSCTL_XTAL_16MHZ);

    // Enable the GPIO port and PF2 

    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    ROM_GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_2);
	
    // Enable UART

    ConfigureUART();

		// Create Console
		UARTprintf("\033[2J");		// clear screen
		UARTprintf("\033[0;0H");	// set cursor to 0,0
//	UARTprintf("\033[10B");  // move cursor down 10 lines
//	UARTprintf("\033[5A");  // move cursor up 5 lines
		UARTprintf("**************************************************\n");
    UARTprintf("**** RTOS Training 1: Command line for Tiva C ****\n"  );
		UARTprintf("**************************************************\n");

    while(1) 
		{

      int32_t i32CommandStatus;

      UARTprintf("\n>");

      while(UARTPeek('\r') == CMDLINE_BAD_CMD) {
            ROM_SysCtlDelay(ROM_SysCtlClockGet() / (1000 / 3));
      }
			
      UARTgets((char *)ui8InputBuffer,sizeof(ui8InputBuffer));
      i32CommandStatus = CmdLineProcess((char *)ui8InputBuffer);

      switch (i32CommandStatus) {
      case CMDLINE_BAD_CMD:
					UARTprintf("Bad command!\n");
					break;
      case CMDLINE_TOO_MANY_ARGS:
					UARTprintf("Too many arguments for command processor!\n");
					break;
      case CMDLINE_TOO_FEW_ARGS:
					UARTprintf("Too few arguments for command processor!\n");
					break;
      case CMDLINE_INVALID_ARG:
					UARTprintf("Invalid argument for command processor!\n");
					break;
      }
    }
}

