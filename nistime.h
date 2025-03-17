/*
	include file nistime.h

	this file is used to define the constants
	that govern the compilation of program NISTIME.

	if the constant IBMPC is defined then the
	program is intended to run on an IBM-PC
	(or equivalent machine) under MSDOS.

	if the constant SUN is defined then the
	program is intended to run on a SUN work
	station.  This version is as non-specific
	as possible so that it should run with only
	minor modifications in other unix environments.

	the DOS version of the program assumes that the 
	declaration "int" implies a 16-bit quantity and 
	that a "long int" is 32-bits long.  some work-station
	environments use 32-bit quantities for both "int"
	and "long int" with 16-bit quantities defined as
	"short int". This should not be a problem in most
	cases, since there is usually no harm is using a
	32-bit integer when a 16-bit one would have been
	sufficient.
	The code may not work, however, if the machine uses
	64 bits for a "long int" and 32-bits for an "int"
	(e.g., DEC Alpha).  The long-int declarations will
	then produce 64 bit quantities which will not be
	correct in many cases (mostly when the quantity
	is passed by reference to a system subroutine which
	expects a parameter that is 32 bits long, but possibly
	in other situations as well).

	therefore --
	if SUN is defined and if the machine uses 16 or 32 bits 
	for an "int" and 32 bits for a "long int" then define
	B64 as 0.
	if SUN is defined and if the machine uses 64 bits for a 
	"long int" then define B64 as 1.  The only common machine
	currently in this category is a DEC Alpha-based system.
	if MSDOS is defined then B64 should be 0 since a long
	int is 32 bits in this environment.

	these two definitions are in the include file sizint.h,
	and that file should be edited appropriately by
	removing the definitions from the comment block
	(note that some
	compilers require that pre-processor directives of 
	this type start in col 1.)
*/
/*	#define IBMPC  1*/
#define SUN   1
#define LINUX
#include "sizint.h"
/*
	There are two MSDOS versions of the program:

	if the constant BIOS is defined, then the
	program will access the RS-232 port via calls
	to the system BIOS.

	if the constant BIOS is not defined, then the
	program uses direct input/output via inport and outport.

	if the following line is not a comment, then input/output
	will be done via calls to the BIOS.
*/
/*
	#define BIOS 1
*/
/*
	if IBMPC is defined, then an MSDOS version is being
	generated.  In that case, if direct input/output is
	to be used, the following constants define the
	offsets from the port address for the various
	registers of the rs-232 device.
*/
#ifdef IBMPC
#ifndef BIOS
#define txbuf  0        /* transmit buffer offset */
#define rxbuf  0        /* receiver buffer offset */
#define divlsb 0        /* divisor latch, low part */
#define divmsb 1        /* divisor latch, high part */
#define intena 1        /* interrupt enable register */
#define intid  2        /* interrupt ident. register */
#define lcreg  3        /* line control register */
#define modreg 4        /* modem control register */
#define lsreg  5        /* line status register   */
#define msreg  6        /* modem status register  */
#endif
#endif
/*
	if this is the UNIX version then parameter tickus
	gives the time interval between ticks in microseconds.
	typical values are 20 milliseconds for a SUN,
	10 milliseconds for a VAX and 3906 milliseconds for
	a DEC ULTRIX/RISC machine.
	this value is used in the interpolation routines
	interp.c, arcdif.c and parset.c.  If this parameter
	is specified as 0 then the interpolation between ticks
	is disabled and the comparison will use the integer
	tick value as read from the system clock.
	this constant is not needed in the MS-DOS environment
	since all of those machines use a constant value of
	18.2 ticks/second or about 55 milliseconds between ticks.
*/
#ifdef SUN
long int tickus = 3906;
#endif
#ifdef LINUX
#define B64 1
#endif	// LINUX
