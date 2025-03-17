int rdbuf(ibuf,ie1,ie2,ie3,tmo)
char ibuf[],ie1,ie2,ie3;
int tmo;
{
/*
	this subroutine reads characters from the serial line.  the
	SUN version uses a loop on the system read call.  for the
	IBMPC version if BIOS is defined, the read is done using
	interrupt calls to the port defined by cmport.  if BIOS is not
	defined, direct calls are made to the hardware using inport and
	outport at the hardware address cmadr.  the characters are stored
	in ibuf.  the operation continues until any one of the three line
	terminators ie1, ie2 or ie3 is found.  the operation also ends
	on the 280th character if no line terminator is found before that.
	the input line is terminated by a zero byte; the last
	character before the zero byte is the terminator.

	the parameter tmo is a time-out count.  if no character is
	received before tmo goes to zero, the subroutine returns with
	a count of zero (a time-out on a partial line will result in
	the line being thrown away).  in this version (7 jan 92), the
	time-out parameter is in units of ticks of the clock and the
	elapsed time is measured directly by reading the clock rather
	than by counting a variable that is scaled to the cpu speed
	as in the previous versions.  the idea is to generate a time-
	out loop that is less sensitive to the details of the cpu
	speed estimator, which does not work consistently for many
	of the 386/486 configurations

	the comparison against the terminating characters is made after
	the 8th bit is cleared in the received character.  Thus if a
	terminating character has its 8th bit on (i.e. is negative),
	then it will never be matched and is effectively removed from
	the comparison.
*/
#include "nistime.h"
#include <stdio.h>
#ifdef IBMPC
#include <dos.h>
#endif
#ifdef SUN
#include <sys/ioctl.h>
#endif
extern int debug;
int j;
#ifdef IBMPC
unsigned char stat,mstat;
int k,kk;
#endif
char cc;
#ifdef IBMPC
#ifdef BIOS
extern int cmport;
#endif
#ifndef BIOS
extern int cmadr;
#endif
#endif
int tcount;             /*used in time-out loop */
#ifdef SUN
extern int cmport;
LONG k;
LONG tlimit = -10000;
#endif
	j=0;            /*initialize buffer index */
#ifdef SUN
	tcount=tlimit;
/*
	loop until character is ready or tcount goes to 0
*/
	do
	{
	   do
	   {
	      ioctl(cmport,FIONREAD,&k);
	      tcount++;
	   } while( (tcount < 0 ) && (k < 1) ) ;
	   if(tcount == 0)
	      {
	      tcount=tlimit;
	      tmo++;
	      if(tmo == 0)
		{
		if(debug != 0)  /*show whatever we got ... */
		   {
		   ibuf[j]='\0';
		   printf("\n timeout in rdbuf, %d chars read. \n %s",
			j,ibuf);
		   return (0);
		}
	      }
	   }
	   else
	      {
	      read(cmport,&cc,1);
	      ibuf[j]= cc = cc & 0x7f;
	      j++;
	      tcount=tlimit;
	      }
	} while( (j < 280) && (cc != ie1) && (cc != ie2) && (cc != ie3) );
#endif
#ifdef IBMPC
	do                      /*loop over input character stream*/
	  {
	  tcount=tmo;           /*initialize time-out counter */
	  _AH=0;
	  geninterrupt(0x1a);
	  kk=_DX;                       /*get lower half of time*/
	  do
	   {
/*
	   loop until character is available or tcount goes to 0.
*/
	   #ifdef BIOS
	   _AH=3;
	   _DX=cmport;
	   geninterrupt(0x14);
	   stat=_AH;
	   mstat=_AL;
	   #endif
	   #ifndef BIOS
	   stat=inportb(cmadr+lsreg);     /* read line status*/
	   mstat=inportb(cmadr+msreg);    /* read modem status*/
	   #endif
	   _AH=0;
	   geninterrupt(0x1a);
	   k=_DX;               /*get current lower half of time*/
	   if(k != kk)          /*if tick has changed */
	      {         /*increment counter, exit with time-out if it is 0*/
	      if(++tcount >= 0)
		{
		if(debug != 0)  /*show partial output, if any*/
		   {
		   ibuf[j]= '\0';
		   printf("\n timeout in rdbuf, %d chars rec'd.,\n %s",
		      j,ibuf);
		}
		return(0);
	      }
	      kk=k;             /*otherwise save new value of tick */
	   }
	   }  while ( (stat & 1) == 0);    /*while buffer is empty */
/*      if we came out of previous loop, a character is ready
	to be read
*/
	   #ifdef BIOS
	   _AH=2;
	   _DX=cmport;
	   geninterrupt(0x14);
	   cc=_AL;
	   #endif
	   #ifndef BIOS
	   cc=inportb(cmadr);
	   #endif
	   ibuf[j++]= cc= cc & '\x7f';    /* turn off ms bit */
/*
	if clear to send and carrier detect have been lost then
	the fact that a character is ready to be read is an error
	terminate line here. how can the hardware do this over
	and over again ??
*/
	   if( (mstat && 0xb0) == 0)
	      {
	      ibuf[j]=0;
	      if(debug != 0) printf("\nCTS or DCD lost.");
	      return(j);
	   }
/*
	continue loop until one of terminators is found or until
	280th character is read.  exit if any of these conditions
	is satisfied, add terminating NULL to buffer and return
	count of number of characters read
*/
	} while ( (j < 280) && (cc != ie1) && (cc != ie2) && (cc != ie3));
#endif
	ibuf[j]=0;
	return(j);
}
