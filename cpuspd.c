int cpuspd()
{
/*
  this subroutine estimates the speed of the cpu by
  counting the number of increments that can be performed
  in roughly 0.25 seconds.  this number is used to scale
  the various time-out parameters so that faster cpu-s
  don't time-out prematurely

  this version of the subroutine is changed for force variable
  k to be a memory location rather than a register variable.  this is
  to better match the comparison code in rdbuf where the timeout variable
  is stored in memory.  the difference is important for -286 machines
  which can perform memory references much faster and which therefore
  would time-out even after having been calibrated by this routine.
*/
#include "nistime.h"
#include <stdio.h>
#ifdef IBMPC
#include <dos.h>
#endif
#ifdef SUN
#include <sys/time.h>
struct timeval tvv,*tp;
long int old,dly;
#endif
int j;
int kk;
static int k = 0;   /* force k to be a non-register variable */
long int mm;
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
#endif
#ifdef IBMPC
/*
  for IBMPC version, wait until tick changes 5 times --
  that is approximately 0.25 second, although the change
  of the first tick is likely to happen too soon and this
  calculation is only intended to be a relative speed number

  start out by waiting until the tick has just changed to
  try to get 5 full ticks.
  when k is a register variable,
  the "standard" machine returns mm 6298 at low speed
  and 10854 at "turbo" speed.  10854/6298=1.72 and ratio of
  cpu clock speeds is 10 MHz/6 MHz = 1.67.
  this version is changed by making k static and making mm long
  to more accurately measure the speed ratio of 386 machines where
  a static variable reference is faster than a register reference
  mm returns the speed ratio as a percent relative to the 6 MHz
  machine.  note that this is only approximate!
*/
	do
	  {
	  _AH=0;
	  geninterrupt(0x1a);
	  kk=_DX;
	} while (kk==k);
	mm=0;
	k=kk;
        for(j=0; j<5; j++)
        {
        do
        {
        mm++;
        _AH=0;
        geninterrupt(0x1a);
        kk=_DX;
        } while (kk == k);
        k=kk;
        }
        return(mm/63);
#endif
#ifdef SUN
/*
	for SUN version, wait until new value of microseconds has
	advanced by 12 ticks (0.24 sec).

	as with IBM PC version, wait until the tick has just
	changed to get 12 full ticks.

	the SUN 3/50 yields a value of mm of about 1100 although
	the fluctuations are quite large.  as with IBM PC, return
	mm as percentage of 3/50 speed
*/
	do
	  {
	  gettimeofday(tp,0);
	  dly=tp->tv_usec - old;
	  if(dly < 0) dly += 1000000;
	} while(dly < 19000);
	old=tp->tv_usec;
	mm=0;
	do
	  {
	  mm++;
	  gettimeofday(tp,0);
	  dly=tp->tv_usec - old;
	  if(dly < 0) dly += 1000000;
	} while (dly <= 230000);
	return(mm/11);
#endif
}
