void inilin()
{
/*
        this subroutine initializes the com port defined by
        global cmport (0=com1, 1=com2, ... etc).
        the initialization parameters are set using param

        if BIOS is not defined, do not use bios calls but access the
        hardware directly.  this is done by determining the hardware
        address of the comport by looking in low core locations.
*/
#include <stdio.h>
#include "nistime.h"
#ifdef IBMPC
#include <dos.h>
char param;
#endif
extern int cmport; /* which comport to use ? */
extern int hs;     /* which speed to use ?   */
extern int debug;  /* debug is on if = 1 */
#ifdef IBMPC
void wait();		/*wait for DSR to come true */
extern int lpadr;   /* address of line printer -- determined here */
#ifndef BIOS
extern int cmadr;   /* hardware address of comport -- determined here*/
#endif
extern int lpt;     /* pulse line printer? 1=yes, 0= no */
#endif
extern char number[];   /* which number to dial, first char = mode*/
int j;
#ifdef SUN
#include <sgtty.h>
struct sgttyb blah,*arg;     /* parameters used for ioctl calls */
	arg= &blah;           /* blah is structure */
	if(hs == 1)
	   {
	    arg->sg_ispeed=B1200;
	    arg->sg_ospeed=B1200;
	   }
	else
	   {
	   arg->sg_ispeed=B300;
	   arg->sg_ospeed=B300;
	   }
	arg->sg_erase='\177';
	arg->sg_kill='\025';
	arg->sg_flags=RAW;
	j=ioctl(cmport,TIOCSETP,arg);
	if(j != 0) printf("\n Watch out! ioctl status=%d",j);
#endif
#ifdef IBMPC
unsigned char stat;
#endif
/*
	operation of the IBMPC version:

        if this is the direct access version of the program, then the
        hardware address of the comport must be either specified in the
        configuration file or determined by looking in low memory.  if
        it was specified in the configuration file, then cmport was set
        to -1 there and cmadr was read from line 6.  in that case we
        don't have to do anything here.  if cmadr is positive, then
        the value must be determined here by looking at the table starting
        at address 400 (hex).
        following variables and code are used to look in low memory
        to get the address of the comport requested in the config. file
        if the address in low memory is zero, then exit with an error.
        since the line printer addresses are stored just after the tty
        port addresses, these variables are also used for getting the
        hardware address of lpt1 (see below)

	operation of the SUN version:

	the port was opened in setcfg and this subroutine sets the
	speed and other parameters based on values in the config.
	file.
*/
#ifdef IBMPC
unsigned int ttyseg = 0x40;
unsigned int ttyoff =    0;
int far *cmptr;
#ifndef BIOS
/*
        first get hardware address of requested comport if necessary
        these address are stored starting at 400.  if cmport is -1,
        then the address was already gotten by setcfg from the configuration
        file and we don't have to do that here.
*/
        if(cmport >= 0)
        {
        ttyoff=2*cmport;
        cmptr=MK_FP(ttyseg,ttyoff);
        cmadr=*cmptr;
        if(cmadr == 0)
           {
           printf("\n Port COM%d not present in configuration.",cmport+1
);
           exit();
           }
        }
/*
        disable interrupts, turn on DTR and RTS, then
        set 8 bits, no parity, 1 stop bit, Div. Latch Acc Bits on
*/
        param=0;
        outportb(cmadr+intena,param);
        param=3;
        outportb(cmadr+modreg,param);
        param='\x83';
        outportb(cmadr+lcreg,param);
/*
        now set requested speed using two halves of divisor
        from tables in interface manual
*/
        if(hs == 1)
           {
           param=96;
           outportb(cmadr+divlsb,param);
           param=0;
           outportb(cmadr+divmsb,param);
           }
        else
           {
           param='\x80';
           outportb(cmadr+divlsb,param);
           param=1;
           outportb(cmadr+divmsb,param);
           }
/*
        now turn off divisor latch access bits
*/
        param=3;
        outportb(cmadr+lcreg,param);
        wait();
        wait();
        wait();		/*wait for ROLM system to wake up*/
/*
        get modem status
*/
        stat=inportb(cmadr+msreg);
#endif
#ifdef BIOS
/*
        if we get here then we are going to do the same stuff using
        BIOS calls. the parameters are:

        300 baud        no parity       1 stop bit      8 bits
        0 1 0           0 0             0               1 1

        1200 baud       no parity       1 stop bit      8 bits
        1 0 0           0 0             0               1 1

        9600 baud       no parity       1 stop bit      8 bits
        1 1 1           0 0             0               1 1
*/
        if(hs == 1)param='\x83';
        else       param='\x43';
        _AL=param;
        _AH=0;
        _DX=cmport;
        geninterrupt(0x14);
        stat=_AL;           /* return with modem status  */
#endif
/*
        if this is automatic dialing using either manual or pulse mode
        then be sure that the modem is ready to go.  check DSR
*/
        if( (number[3] == 't') || (number[3] == 'T') ||
            (number[3] == 'p') || (number[3] == 'P') )
        {
        if( (stat & 0x20) == 0)
           {
           printf("\nWarning!  modem does not ");
           printf("show data set ready (DSR).");
           }
        }
/*
        now set line printer address if lpt is set to pulse it. if
        lpt is zero so that pulsing is disabled, then return now.

        get lpt1 address by looking in low core at address 408
*/
        if(lpt == 0) return;
        ttyoff=8;
        cmptr=MK_FP(ttyseg,ttyoff);
        lpadr=*cmptr;
        if(lpadr == 0)
        {
        printf("\n printer-port not installed in system.");
        printf("\n pulsing selection disabled.");
        lpt=0;
        return;
        }
        outportb(lpadr,1);
        for(j=0; j<200; j++);   /*wait a bit*/
        if( (inportb(lpadr) & 0x1f) == 1)
        {
        outportb(lpadr,0);
        return;
        }
        printf("\n printer-port does not respond to pulse.");
        printf("\n pulsing selection will be disabled.");
        lpt=0;
#endif
}
