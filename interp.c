float interp()
{
/*
  this subroutine estimates the fraction of a tick between
  when it is called and the next time the clock changes.
  it does this by incrementing a variable until the tick
  changes and then repeating the operation until it changes
  again.  the first number is a partial tick and the second
  one is a full tick and the ratio is thus the fraction
  from when the routine is called until the next tick.

  For the Sun system, the interval betweek ticks in microseconds
  must be specified in the file nistime.h as parameter tickus.
  If this parameter is set to 0, then the interpolator is 
  effectively disabled.
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
long int mm,mmm;
float xx;
/*
	get lower portion of time for IBMPC or current microseconds for sun
*/
#ifdef IBMPC
	_AH=0;
	geninterrupt(0x1a);
	k=_DX;
#endif
#ifdef SUN
	tp= &tvv;
	gettimeofday(tp,0);
	old=tp->tv_usec;
	if(tickus == 0) return (100.);  /*interpolation disabled */
#endif
#ifdef IBMPC
/*
  now wait until tick changes
*/
	mmm=0;
	do
	  {
	  mmm++;
	  _AH=0;
	  geninterrupt(0x1a);
	  kk=_DX;
	} while (kk==k);
	mm=0;
	k=kk;
	do
	  {
	  mm++;
	  _AH=0;
	   geninterrupt(0x1a);
	   kk=_DX;
	} while (kk == k);
	xx=mmm;
	xx=100*xx/(float) mm;
	if(xx < 0) return (0.);
	if(xx > 100) return(100.);
	return(xx);
#endif
#ifdef SUN
/*
	first increment counter until tick changes  by the 
	time interval in microseconds specified in the
	header file as parameter tickus. -- see nistime.h
*/
	mmm=0;
	do
	  {
	  mmm++;
	  gettimeofday(tp,0);
	  dly=tp->tv_usec - old;
	  if(dly < 0) dly += 1000000;  /*if rolled over to next second*/
	} while(dly < tickus);
	old=tp->tv_usec;
	mm=0;
	do
	  {
	  mm++;
	  gettimeofday(tp,0);
	  dly=tp->tv_usec - old;
	  if(dly < 0) dly += 1000000;
	} while (dly < tickus);
	xx=mmm;
	xx=100*xx/(float)mm;
	if(xx < 0) return(0.);
	if(xx > 100) return(100.);
	return(xx);
#endif
}
