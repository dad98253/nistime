void wrtbuf(obuf)
char obuf[];
{
/*
        this subroutine writes the characters in obuf to
        a com port. if BIOS is defined, then the operation
        is done via calls to the BIOS routines.  if BIOS is
        not defined, then the operation is performed using
        direct input output operations on the port whose hardware
        address is defined by global cmadr.
        cmadr was initialized by the subroutine inilin based on the
        user input parameter cmport.

        in either case:
        the operation continues until a 0 byte is found in the input buffer
        and the port should have been initialized by somebody else.
*/
#include "nistime.h"
#include <stdio.h>
#ifdef IBMPC
#include <dos.h>
#endif
int j;
#ifdef IBMPC
unsigned char stat;
char cc;
#ifdef BIOS
extern int cmport;
#endif
#ifndef BIOS
extern int cmadr;
#endif
#endif
#ifdef SUN
extern int cmport;
#endif
int t;
#ifdef IBMPC
int tlimit= -20000;
#endif
extern int debug;
#ifdef SUN
	for(j=0; obuf[j] != 0; j++) ;   /* find number of bytes */
	t=write(cmport,&obuf[0],j);
	if(t < 1)
	   {
	   printf("\n write failed, %d chars. written",t);
	   abort();
	   }
	return;
#endif
#ifdef IBMPC
        t=tlimit;
        for(j=0; (cc=obuf[j]) != 0; j++)
        {
/*
        check status and wait until ready
*/
        do
           {
           #ifdef BIOS
           _AH=3;
           _DX=cmport;
           geninterrupt(0x14);
           stat=_AH;
	   #endif
	   #ifndef BIOS
           stat=inportb(cmadr+5);
           #endif
           t++;
           if(t == 0)
              {
              printf("\n Write failed. Modem not ready.");
              return;
              }
           }   while( (stat & 0x20) == 0 );
        #ifdef BIOS
        _AL=cc;
        _AH=1;
        _DX=cmport;
        geninterrupt(0x14);
	#endif
	#ifndef BIOS
        outportb(cmadr,cc);
        #endif
        t=tlimit;
        }
#endif
}
