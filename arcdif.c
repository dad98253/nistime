void arcdif(buf)
char buf[280];
{
#include "nistime.h"
#include <stdio.h>
#ifdef IBMPC
#include <dos.h>
#endif
#ifdef SUN
#include <sys/time.h>
#include <math.h>
#endif
char c;
int j,yr,mo,day,hr,min,sec,dst;   /* holds parsed NIST time */
#ifdef IBMPC
int yribm,moibm,dayibm,hribm,minibm,secibm,hunibm; /* holds computer time*/
int yrat,moat,dayat,hrat,minat,secat;/* holds CMOS time for AT*/
#endif
#ifdef SUN
LONG mjd;
LONG mjd0 = 40587;     /* mjd of 1/1/70 */
struct timeval tvv,*tp;    /* holds time in SUN format */
extern int hs;             /* 1 if dialing at 1200, 0 if at 300 */
LONG cvt2jd();         /* convert yr-mo-day to MJD for sun  */
#endif
double diff;
#ifdef IBMPC
extern int utcdif;           /* local time - utc in hours */
extern int dsflag;           /* daylight saving time? 1=yes, 0=no */
extern int atflag;           /* AT-type machine? 1=yes, 0=no */
#endif
extern FILE *jop;            /* file handle for writing difference*/
#ifdef IBMPC
int lday[13] = {0,31,28,31,30,31,30,31,31,30,31,30,31}; /*last day of month*/
int tday[13] = {0,0,31,59,90,120,151,181,212,243,273,304,334};
int unpbcd();
int dsdone = 0;              /* flag to show day. sav. time corr. done */
#endif
int done   = 0;              /* flag to show when comparison is done */
float xx,interp();           /* interpolate fraction of a tick */
/*
	this subroutine receives a time string in character
	array buf.  it is parsed and compared with the system
	clock.  the origin of the parser is the - character between
	the year and the month so that leading stuff (possible a lf)
	will simply be ignored.

	if IBMPC is true, then the comparison is made using local
	time as defined by global variables dsflag to check for
	daylight savings time and utcdif to convert from UTC to local
	time. if SUN is true then the conversion is done using UTC
	directly.

	begin by getting computer time now. also get tick fraction
	via interp.
*/
#ifdef IBMPC
	_AH=0x2c;
	geninterrupt(0x21);
	hribm=_CH;
	minibm=_CL;
	secibm=_DH;
	hunibm=_DL;
	xx=interp();         /* get tick fraction now */
	_AH=0x2a;
	geninterrupt(0x21);
	yribm=_CX;
	moibm=_DH;
	dayibm=_DL;
	yribm -= 1900;	/*number of years since 1900*/
/*
	if this is an at-type machine, read CMOS clock too
*/
	if(atflag != 0)
	{
	_AH=2;
	geninterrupt(0x1a);
	hrat=_CH;
	minat=_CL;
	secat=_DH;
	_AH=4;
	geninterrupt(0x1a);
	yrat=_CL;
	moat=_DH;
	dayat=_DL;
	}
#endif
#ifdef SUN
	tp= &tvv;
	gettimeofday(tp,0);
	xx=interp();
#endif
/*
	now parse line from NIST transmission

	first find - between year and month
*/
	for(j=0; (buf[j] != 0) && (buf[j] != '-') ; j++) ;
	sscanf(&buf[j-2],"%2d-%2d-%2d %2d:%2d:%2d %d",&yr,&mo,&day,
	&hr,&min,&sec,&dst);
	if(yr < 90) yr += 100;	/*number of years since 1900*/
#ifdef SUN
/*
	1200 bit/s transmission includes MJD -- therefore parse it
	from input line.  300 bit/s transmission does not include
	MJD and we must get it from yr-mo-day via a call to cvt2jd
*/
	if(hs == 1) sscanf(&buf[j-8],"%5ld",&mjd);   /* also get mjd for SUN */
	else        mjd=cvt2jd(yr,mo,day);
/*
	convert nist time to number of seconds since 1/1/70
*/
	mjd= 86400*(mjd - mjd0);
	mjd= mjd + 3600*hr + 60*min + sec;
#endif
#ifdef IBMPC
/*
	for the IBMPC version, the comparison is made against
	local time -- thus nist time must be converted to local
	time using utcdif and dst flags.
*/
/*
	make standard-time portion of dst flag contiguous
*/
	if(dst == 0) dst = 100;
/*
	convert from utc to local time. note that minute and second
	are already correct.
	daylight savings time flag must also be updated
	if conversion to local time changes the day

	**
	** version of 12 may -- compiled and tested 31 may
	daylight savings time correction must be done in two
	parts since adding hour may cause hour/day to overflow.
	therefore deal with part of it now -- if day changes then
	correction flag dst will change and therefore correction
	for transition days must be done after final day is known.
	also see subroutine parset where same problem appears.
*/
	if( (yr & 3) == 0 ) lday[2]=29;  /* 29 days for Feb in leap year*/
	hr += utcdif;              /* convert hour to local standard time */
/*
	see comments in parset.c regarding the following code, which
	is basically copied from there
*/
	if( (dsflag != 0) && (dst <= 50) && (dst > 1) )
	{
	hr++;
	dsdone=1;
	}              /* do first part of daylight savings time */
	if( (dsflag != 0) && (dst == 1) && (hr < 1)  )
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
	note that the correction when dst=1 uses the test
	hr < 1 even though the transition is at 0200 because
	the hour has been set to standard time above by default.
	also see comments in parset.c
*/
	if ( (dsflag != 0) && (dsdone == 0) )
	{
	   if( (dst == 51) && (hr >= 2) ) hr++;
	   if( (dst ==  1) && (hr <  1) ) hr++;
	}
#endif
/*
	ready to begin comparing the clocks
	note that we only write a 2-digit year here,
	but that yr actually is the number of years since
	1900.
*/
	fprintf(jop,"%2d-%2.2d-%2.2d %2.2d:%2.2d:%2.2d",
	      yr%100,mo,day,hr,min,sec);
#ifdef SUN
/*
	sun comparison is done directly as difference
	of two longs giving number of seconds since 1/1/70
*/
	diff= tp->tv_sec - mjd;
	done=0;
	if(fabs(diff) > 100000.)
	   {
	   fprintf(jop," %8.2fd\n",diff/86400.);
	   done=1;
	   }
	if( (done == 0) && (fabs(diff) > 10000.) )
	   {
	   fprintf(jop," %8.2fh\n",diff/3600.);
	   done=1;
	   }
	if( (done == 0) && (fabs(diff) > 1000.) )
	   {
	   fprintf(jop," %8.2fm\n",diff/60.);
	   done=1;
	   }
	if( (done == 0) && (fabs(diff) <= 1000.) )
	   {
	   diff += (float) tp->tv_usec/1000000.;
/*
	add the fraction of a tick computed by interp.  this value
	tends to make the sun clock later, since this value must be
	added to the integer values read from the system.
	The interval between ticks is defined in the header file
	nistime.h.  It is typically 20 msec on a Sun, 10 msec on a
	VAX and 3906 usec on a DEC RISC machine.  This number is
	always given in microseconds and is converted to seconds
	here.  Note that the value in paretheses is a tick fraction
	expressed as a percent.
*/
	   diff += ( (double) tickus) *(100-xx)*1e-8;

	   fprintf(jop," %9.4fs\n",diff);
	   }
#endif
#ifdef IBMPC
/*
	ibm comparison is done bit by bit to preserve
	resolution
*/
	diff=365*(yribm - yr) + tday[moibm] - tday[mo] + dayibm - day;
	if( ( (yribm & 3) == 0)  && (moibm > 2) ) diff++;
	if( ( (yr    & 3) == 0)  && (mo    > 2) ) diff--;
	if( (diff > 1) || (diff < -1) )
	{
	fprintf(jop," %8.2fd",diff);
	done=1;
	}
	if(done == 0)
	{
	   diff= 24*diff + hribm - hr;
	   if( (diff > 2) || (diff < -2) )
	   {
	      fprintf(jop," %8.2fh",diff);
	      done=1;
	   }
	}
	if(done == 0)
	{
	   diff=60*diff + minibm - min;
	   if( (diff > 10) || (diff < -10) )
	   {
	   fprintf(jop," %8.2fm",diff);
	   done=1;
	   }
	}
	if(done == 0)
	{
/*
	compute difference, adding tick fraction computed by interp
*/
	   diff=60*diff + secibm - sec +  0.01*(float)hunibm
		 +  0.000549*(100-xx);
	   fprintf(jop," %8.3fs",diff);
	}
	if(atflag == 0)
	{
	fprintf(jop,"     0.00\n");
	fclose(jop);
	return;
	}
/*
	begin comparison of CMOS time -- first convert from packed
	BCD to normal binary, then compare as above.
*/
	yrat=unpbcd(yrat);
	if(yrat < 80) yrat += 100;	/*number of years since 1900*/
	moat=unpbcd(moat);
	dayat=unpbcd(dayat);
	hrat=unpbcd(hrat);
	minat=unpbcd(minat);
	secat=unpbcd(secat);
	diff=365*(yrat - yr) + tday[moat] - tday[mo] + dayat - day;
	if( ( (yrat & 3) == 0) && (moat > 2) )  diff++;
	if( ( (yr   & 3) == 0) && (mo   > 2) )  diff--;
	done=0;
	if( (diff > 1) || (diff < -1) )
	{
	   fprintf(jop," %8.2fd\n",diff);
	   done=1;
	}
	if(done == 0)
	{
	diff=24*diff + hrat - hr;
	if( (diff > 2) || (diff < -2) )
	   {
	   fprintf(jop," %8.2fh\n",diff);
	   done=1;
	   }
	}
	if(done == 0)
	{
	diff=60*diff + minat - min;
	if( (diff > 10) || (diff < -10) )
	   {
	   fprintf(jop," %8.2fm\n",diff);
	   done=1;
	   }
	}
	if(done == 0)
	{
	diff=60*diff +secat - sec;
	fprintf(jop," %8.2fs\n",diff);
	}
#endif
	fclose(jop);
}

