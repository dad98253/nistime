void parset(buf)
char buf[280];
{
#include "nistime.h"
#include <stdio.h>
#ifdef IBMPC
#include <dos.h>
#endif
char c;
int j,yr,mo,day,hr,min,sec,is,dst;
#ifdef SUN
#include <sys/time.h>
#endif
#ifdef IBMPC
extern int utcdif;           /* local time - utc in hours */
extern int dsflag;           /* daylight saving time? 1=yes, 0=no */
extern int atflag;           /* AT-type machine? 1=yes, 0=no */
int lday[13] = {0,31,28,31,30,31,30,31,31,30,31,30,31}; /*last day of month*/
char cnvbcd();
char centc,yrc,moc,dayc,hrc,minc,secc;
int dsdone = 0;
#endif
#ifdef SUN
LONG mjd;
LONG mjd0 = 40587;     /* mjd of 1/1/70 -- origin of SUN time */
struct timeval tvv,*tp;
extern int hs;             /* 1 if dialing at 1200, 0 if at 300   */
LONG cvt2jd();         /* computes MJD from yr-mo-dy          */
#endif
/*
	this subroutine receives a time string in character
	array buf.  it is parsed and used to set the system
	clock.  the origin of the parser is the - character between
	the year and the month so that leading stuff will simply be
	ignored.

	this subroutine uses global variables dsflag to check for
	daylight savings time

	for the IBMPC version, the subroutine converts to local
	time if requested by the configuration file parameters
	(including a conversion to daylight savings time if needed.
	the SUN version sets the time directly in UTC by converting
	the received time to seconds since 1/1/70.  to simplify
	this conversion, the mjd, rather than the year-month-day
	is used.
*/
	for(j=0; (buf[j] != 0) && (buf[j] != '-') ; j++) ;
	sscanf(&buf[j-2],"%2d-%2d-%2d %2d:%2d:%2d %d",&yr,&mo,&day,
	&hr,&min,&sec,&dst);
	if(yr < 80) yr +=100;	/*number of years since 1900*/
#ifdef SUN
/*
	transmission at 1200 contains MJD and it can be read directly
	from NIST.  transmission at 300 does not contain MJD and it must
	be computed from the yr-mo-day values
*/
	if(hs==1) sscanf(&buf[j-8],"%5ld",&mjd);
	else      mjd=cvt2jd(yr,mo,day);
/*
	convert received day number to seconds since 1/1/70 and
	add in hour minute and second.
*/
	tp= &tvv;
	mjd= 86400*(mjd - mjd0);
	mjd= mjd + 3600*hr + 60*min + sec;
	tp->tv_sec=mjd;
	tp->tv_usec=0;
	settimeofday(tp,0);
#endif
#ifdef IBMPC
/*      
	if IBMPC version is being generated, we must now convert
	to local time (including daylight savings time as needed
*/
/*
	make standard-time portion of dst flag contiguous
*/
	if(dst == 0) dst = 100;
/*
	convert from utc to local time. note that although minute
	and second are already correct, there is no BIOS call to
	set just those parameters so we have to wait until the hour
	is corrected and then set them all at once.

	note that daylight savings time flag must also be updated
	if conversion to local time changes the day

	**      following correction is for version of 12 May 88:
	**      this correction compiled for final testing 31 may
	daylight savings time correction is done in two parts:

	if we are solidly in daylight savings time, then do correction
	immediately since it may change the day, and therefore it must
	be done before the check for hour/day overflow.

	if this is a transition day, then correction cannot be done now
	since the hour/day overflow may change the daylight savings flag
	dst thereby changing whether or not the correction is needed.
	thus correction must be postponed.

	parameter dsdone makes sure that the correction is not done twice
	if we are  moved into a transition day by the hour/day overflow */
	if( (yr & 3) == 0 ) lday[2]=29;  /* 29 days for Feb in leap year */
	hr += utcdif;             /* convert hour to local standard time */
/*
	if daylight saving time is selected and it is not a
	transition day, then add 1 hour and set flag to
	show conversion is completed
*/
	if( (dsflag !=0) && (dst <= 50) && (dst > 1) )
	  {
	  hr++;
	  dsdone=1;
	}
	if(hr < 0)
	  {
	  hr += 24;
	  day--;
	  dst++;         /* update daylight savings flag for change of day*/
	  if(day < 1)
	    {
	    mo--;
	    if(mo < 1)
	      {
	      mo=12;
	      yr--;
	    }
	    day=lday[mo];
	  }
	}
	if(hr > 23)
	  {
	  hr -= 24;
	  day++;
	  dst--;        /* update daylight saving flag for change of day */
	  if(day > lday[mo])
	   {
	   day=1;
	   mo++;
	   if(mo > 12)
	      {
	      mo=1;
	      yr++;
	   }
	  }
	}
/*
	now finish up daylight savings time if enabled
	only thing left to do is to deal with the transition days
	flag dsdone makes sure that transition day is not done twice
*/
	if( (dsflag != 0) && (dsdone == 0) )
	   {
	   if( (dst == 51) && (hr >= 2) ) hr++;
	   if( (dst ==  1) && (hr <  2) ) hr++;
	}
	_AH=0x2d;
	_CH=hr;
	_DH=sec;
	_CL=min;
	_DL=10;       /* approximate machine-dependent latency correction*/
	geninterrupt(0x21);
	yr += 1900;       /* add 1900 to number of years since 1900 above*/
	_AH=0x2b;
	_CX=yr;
	_DH=mo;
	_DL=day;
	geninterrupt(0x21);
	printf("\n Computer clock set.");
/*
	code to set AT CMOS clock. note that CMOS clock requires
	time to be converted to BCD digits, packed two to a byte. this
	is done using routine cnvbcd.
*/
	if(atflag != 0)
	{
	centc=cnvbcd(yr/100);
	yrc=cnvbcd(yr%100);
	moc=cnvbcd(mo);
	dayc=cnvbcd(day);
	hrc=cnvbcd(hr);
	minc=cnvbcd(min);
	secc=cnvbcd(sec);
	_AH=3;
	_CH=hrc;
	_CL=minc;
	_DH=secc;
	_DL=dsflag;             /* copy daylight savings to CMOS clock */
	geninterrupt(0x1a);
	_AH=5;
	_CH=centc;
	_CL=yrc;
	_DH=moc;
	_DL=dayc;
	geninterrupt(0x1a);
	printf("\n CMOS clock set.");
	}
#endif
}
