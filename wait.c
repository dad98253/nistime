void wait()
{
/*
	this subroutine delays by 1 second each time
	it is called.  the delay is only approximate and
	is in fact 18 ticks of the clock for an IBMPC.  for
	a SUN version, the program waits until the low part
	of the time wraps around to the same value again
*/
#include "nistime.h"
#include <stdio.h>
#ifdef IBMPC
#include <dos.h>
#endif
#ifdef SUN
#include <sys/time.h>
struct timeval tvv,*tp;
LONG old,dly;
#endif
int j;
int k,kk;
/*
	get current hundredths of time for IBMPC or current
	microseconds for sun
*/
#ifdef IBMPC
	_AH=0x2c;
	geninterrupt(0x21);
	k=_DL;
#endif
#ifdef SUN
	tp= &tvv;
	gettimeofday(tp,0);
	old=tp->tv_usec;
#endif
#ifdef IBMPC
/*
	for IBMPC version, wait until tick changes 18 times --
	that is approximately 1 second, although the change
	of the first tick is likely to happen too soon and
	the number of ticks/sec is slightly more than 18 --
	both of these will cause the delay to be a bit too
	short.
*/
	for(j=0; j<18; j++)
	{
	do
	{
	_AH=0x2c;
	geninterrupt(0x21);
	kk=_DL;
	} while (kk == k);
	k=kk;
	}
#endif
#ifdef SUN
/*
	for SUN version, wait until new value of microseconds is
	just slightly less than old value.
*/
	do
	{
	gettimeofday(tp,0);
	dly=tp->tv_usec - old;
	if(dly < 0) dly += 1000000;
	for(k=0; k<20; k++)  ;   /* wait a bit */
	} while (dly <= 970000);
#endif
}
