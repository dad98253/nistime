void diftim(buf)
char buf[280];
{
#include "nistime.h"
#include <stdio.h>
#include <time.h>
#ifdef IBMPC
#include <dos.h>
#endif
char c;
float xx,interp();                /* holds tick fraction */
int j,yr,mo,day,hr,min,sec,dst;   /* holds parsed NIST time */
int yribm,moibm,dayibm,hribm,minibm,secibm,hunibm; /* holds computer time */
#ifdef IBMPC
int yrat,moat,dayat,hrat,minat,secat;/* holds CMOS time for AT*/
#endif
int iterr = 0;
float diff;                  /* holds time difference */
float elapse;                  /* days since last comparison */
float ddiff;                   /* difference of difference in sec */
#ifdef IBMPC
float adiff;                   /* same for IBMPC CMOS clock */
#endif
#ifdef IBMPC
extern int utcdif;           /* local time - utc in hours */
extern int dsflag;           /* daylight saving time? 1=yes, 0=no */
extern int atflag;           /* AT-type machine? 1=yes, 0=no */
int lday[13] = {0,31,28,31,30,31,30,31,31,30,31,30,31}; /*last day of month*/
#endif
static int tday[13] = {0,0,31,59,90,120,151,181,212,243,273,304,334};
int done = 0;         /* flag to show when comparison is complete */
extern int wrtdif;    /*1=write dif to file, 0=don't 2=get rate too */
extern int debug;     /* 1=debug on, 0=debug off */
char *ptrslw,*ptrfas,*ptrrat;   /*output format strings*/
#ifdef IBMPC
int unpbcd();
int dsdone=0;                /* flag to show conversion for daylight sav.*/
#endif
#ifdef SUN
#include <sys/time.h>
struct timeval tvv,*tp;
struct tm gvv,*gv;
extern int hs;               /*1 if dialing at 1200, 0 if at 300 */
LONG cvt2jd();           /*computes MJD from yr-mo-dy*/
#endif
/*
	the following structure holds the values computed the last
	time the program was run as read to estimate the rate offset
	of the clock.  the values are read by getlst() from the last
	line in file nistime.dif if "A" mode was selected.  if "a" mode
	was selected, this structure is not used.
*/
extern struct tmprev
	{
	int yrprev;     /* time of previous comparison in first 6 values*/
	int moprev;
	int dyprev;
	int hrprev;
	int mnprev;
	int scprev;
	float dffprv;   /* value of difference followed by unts*/
	char unprev;
#ifdef IBMPC
	float datprv;   /* difference and units for IBMPC CMOS clock*/
	char uatprv;
#endif
} tmpp;
	ptrslw="\n Computer clock slow by %.3f %s\n";
	ptrfas="\n Computer clock fast by %.3f %s\n";
	ptrrat="\n Approximate rate offset= %.3f sec/day.\n";
/*
	this subroutine receives a time string in character
	array buf.  it is parsed and compared with the system
	clock.  the origin of the parser is the - character between
	the year and the month so that leading stuff will simply be
	ignored.

	the IBMPC version performs the comparison using local time
	this subroutine uses global variables dsflag to check for
	daylight savings time and utcdif to convert from UTC to local
	time

	the SUN version performs the comparison using UTC directly
	since that is available from the system


	begin by getting computer time now.
*/
#ifdef IBMPC
	_AH=0x2c;
	geninterrupt(0x21);
	hribm=_CH;
	minibm=_CL;
	secibm=_DH;
	hunibm=_DL;
/*
	also get tick fraction now for interpolating later
*/
	xx=interp();
	_AH=0x2a;
	geninterrupt(0x21);
	yribm=_CX;
	moibm=_DH;
	dayibm=_DL;
	yribm -= 1900; /*number of years since 1900*/
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
	gv= &gvv;
	gettimeofday(tp,0);
/*
	get tick fraction from interp
*/
	xx=interp();
	gv=gmtime(tp);
	yribm=gv->tm_year;
	moibm=gv->tm_mon + 1;
	dayibm=gv->tm_mday;
	hribm=gv->tm_hour;
	minibm=gv->tm_min;
	secibm=gv->tm_sec;
	hunibm=tp->tv_usec/10000;
#endif
/*
	now parse line from NIST transmission
	the year is expressed as the number of years since 1900.
	if the received value is less than 80 then assume that it 
	signals a year after 2000 since the ACTS systems didn't
	exist in 1980.
*/
	for(j=0; (buf[j] != 0) && (buf[j] != '-') ; j++) ;
	sscanf(&buf[j-2],"%2d-%2d-%2d %2d:%2d:%2d %d",&yr,&mo,&day,
	&hr,&min,&sec,&dst);
	if(yr < 80) yr += 100;	/*number of years since 1900*/
/*
	for IBMPC version, convert to local time and
	daylight savings time if necessary

	note that this conversion is not necessary for the
	SUN which performs the comparison directly on UTC
*/
#ifdef IBMPC
/*
	make standard-time portion of dst flag contiguous
*/
	if(dst == 0) dst = 100;
/*
	convert from utc to local time. note that minute and second
	are already correct.
	daylight savings time flag must also be updated
	if conversion to local time changes the day

*/
	if( (yr & 3) == 0 ) lday[2]=29;  /* 29 days for Feb in leap year */
	hr += utcdif;                    /* convert hour to local time */
/*
	also do first part of conversion to daylight savings time
	see parset and arcdif where same problem is dealt with
*/
	if( (dsflag != 0) && (dst <= 50) && (dst > 1) )
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
*/
	if( (dsflag != 0) && (dsdone == 0) )
	{
	   if( (dst == 51) && (hr >= 2) ) hr++;
	   if( (dst ==  1) && (hr <  2) ) hr++;
	}
#endif
/*
	ready to begin comparing the clocks
	first print out both times
*/
	printf("\n Computer time= %2d-%2.2d-%2.2d %2.2d:%2.2d:%2.2d.%2.2d",
	      yribm%100,moibm,dayibm,hribm,minibm,secibm,hunibm);
	printf("\n NIST time    = %2d-%2.2d-%2.2d %2.2d:%2.2d:%2.2d.00",
	      yr%100,mo,day,hr,min,sec);
	diff=365*(yribm - yr) + tday[moibm] - tday[mo] + dayibm - day;
	if( ( (yribm & 3) == 0)  && (moibm > 2) ) diff++;
	if( ( (yr    & 3) == 0)  && (mo    > 2) ) diff--;
/*
	compute time since last comparison if "A" mode was selected
	time is computed as days and fractions
*/
	if(wrtdif == 2)
	   {
	   elapse=365*(yr - tmpp.yrprev) +tday[mo] - tday[tmpp.moprev]
		+ day - tmpp.dyprev;
	   if( ( (yr & 3) == 0)  && (mo  >2) ) elapse++;
	   if( ( (tmpp.yrprev & 3) == 0) && (tmpp.moprev > 2) )elapse--;
	   elapse += ( (float) (hr - tmpp.hrprev) )/24. +
		     ( (float) (min -tmpp.mnprev) )/1440. +
		     ( (float) (sec -tmpp.scprev) )/86400.;
	   if(debug != 0) printf("\n time since last comparison=%f",elapse);
/*
	convert difference last time to seconds if necessary
*/
	switch(tmpp.unprev)
	      {
	      case 'd':
		ddiff = 86400.*tmpp.dffprv;
		break;
	      case 'h':
		ddiff = 3600.*tmpp.dffprv;
		break;
	      case 'm':
		ddiff = 60.*tmpp.dffprv;
		break;
	      case 's':
		ddiff=tmpp.dffprv;
		break;
	      default:
		printf("\n Units of difference= %c, not d,h,m,s.",
			tmpp.unprev);
		break;
	      }
	if(debug != 0) printf("\n previous difference= %f sec.",ddiff);
	   }
	if(diff > 1)
	{
	printf(ptrfas,diff,"days.");
	done=1;
	}
	if(diff < -1)
	{
	printf(ptrslw,-diff,"days.");
	done=1;
	}
/*
	if difference has been printed as days and rate estimate
	is selected, print value now
*/
	if( (wrtdif == 2) && (done == 1) )
	   {
	   ddiff = (86400.*diff - ddiff)/elapse;
	   printf(ptrrat,ddiff);
	   }
	if(done == 0)
	{
	   diff= 24*diff + hribm - hr;
	   if(diff > 2)
	   {
	      printf(ptrfas,diff,"hours.");
	      done=1;
	   }
	   if(diff < -2)
	   {
	      printf(ptrslw,-diff,"hours.");
	      done=1;
	   }
	   if( (wrtdif == 2) && (done == 1) )
		{
		ddiff=(3600.*diff - ddiff)/elapse;
		printf(ptrrat,ddiff);
		}
	}
	if(done == 0)
	{
	   diff=60*diff + minibm - min;
	   if(diff > 10)
	   {
	   printf(ptrfas,diff,"minutes.");
	   done=1;
	   }
	   if(diff < -10)
	   {
	   printf(ptrslw,-diff,"minutes.");
	   done=1;
	   }
	   if( (wrtdif == 2) && (done == 1) )
		{
		ddiff=(60.*diff - ddiff)/elapse;
		printf(ptrrat,ddiff);
		}
	}
	if(done == 0)
	{
	   diff=60*diff + secibm - sec +  0.01*(float)hunibm;
/*
	Add the fraction of a tick computed by interp.  this value
	tends to make the local clock later, since this value must be
	added to the integer values read from the system.
	If this is a SUN system, the interval between ticks is defined 
	in the header file nistime.h.  It is typically 20 msec on a Sun, 
	10 msec on a VAX and 3906 usec on a DEC RISC machine.  This number
	is always given in microseconds and is converted to seconds
	here.  Note that the value in paretheses is a tick fraction
	expressed as a percent.  All MS DOS machines uses the same clock
	rate of roughly 18.2 ticks/second or about 55 msec betweek
	ticks.
*/
#ifdef IBMPC
	   diff +=  0.000549*(100-xx);
#endif
#ifdef SUN
	   diff += ( (double) tickus) *(100-xx)*1e-8;
#endif
	   if(diff > 0)
	      {
	      printf(ptrfas,diff,"seconds.");
#ifdef IBMPC
	      iterr= 18.2*diff;
#endif
	      }
	   if(diff < 0)
	      {
	      printf(ptrslw,-diff,"seconds.");
#ifdef IBMPC
	      iterr= -18.2*diff;
#endif
	      }
#ifdef IBMPC
	   if(iterr == 0)
	      printf("\nClocks agree to within 1 computer clock tick.\n");
	   else
	      printf("\nDifference is %d computer clock tick(s). \n",iterr);
#endif
#ifdef SUN
	   if(diff == 0)
		printf("\n Clocks agree to within 10 msec. \n");
#endif
	   if(wrtdif == 2)
		{
		ddiff= (diff - ddiff)/elapse;
		printf(ptrrat,ddiff);
		}
	}
#ifdef IBMPC
	if(atflag == 0)return;
	printf("\n\nCompare time of CMOS (Battery-run) Clock with NIST.\n");
/*
	begin comparison of CMOS time -- first convert from packed
	BCD to normal binary, then compare as above.
*/
	yrat=unpbcd(yrat);
	if(yrat < 80) yrat += 100;		/*years since 1900*/
	moat=unpbcd(moat);
	dayat=unpbcd(dayat);
	hrat=unpbcd(hrat);
	minat=unpbcd(minat);
	secat=unpbcd(secat);
	printf("\nCMOS time= %2.2d-%2.2d-%2.2d %2.2d:%2.2d:%2.2d",
	yrat%100,moat,dayat,hrat,minat,secat);
	printf("\nNIST time= %2.2d-%2.2d-%2.2d %2.2d:%2.2d:%2.2d",
	yr%100,mo,day,hr,min,sec);
	diff=365*(yrat - yr) + tday[moat] - tday[mo] + dayat - day;
	if( ( (yrat & 3) == 0) && (moat > 2) )  diff++;
	if( ( (yr   & 3) == 0) && (mo   > 2) )  diff--;
/*
	if rate estimate was selected, convert previous difference
	to seconds if necessary
*/
	if(wrtdif == 2)
	   {
	   switch(tmpp.uatprv)
		{
		case 'd':
		   adiff=86400.*tmpp.datprv;
		   break;
		case 'h':
		   adiff=3600.*tmpp.datprv;
		   break;
		case 'm':
		   adiff=60.*tmpp.datprv;
		   break;
		case 's':
		   adiff=tmpp.datprv;
		   break;
		default:
		   printf("\n units of difference=%c, not d,h,m,s.",
			tmpp.uatprv);
		}
	   }
	done=0;
	if(diff > 1)
	{
	   printf(ptrfas,diff,"days.");
	   done=1;
	}
	if(diff < -1)
	{
	   printf(ptrslw,-diff,"days.");
	   done=1;
	}
	if ( (wrtdif == 2) && (done == 1) )
	   {
	   adiff=(86400.*diff - adiff)/elapse;
	   printf(ptrrat,adiff);
	   }
	if(done == 0)
	{
	diff=24*diff + hrat - hr;
	if(diff > 2)
	   {
	   printf(ptrfas,diff,"hours.");
	   done=1;
	   }
	if(diff < -2)
	   {
	   printf(ptrslw,-diff,"hours.");
	   done=1;
	   }
	if( (wrtdif == 2) && (done == 1) )
	   {
	   ddiff=(3600.*diff - adiff)/elapse;
	   printf(ptrrat,adiff);
	   }
	}
	if(done == 0)
	{
	diff=60*diff + minat - min;
	if(diff > 10)
	   {
	   printf(ptrfas,diff,"minutes.");
	   done=1;
	   }
	if(diff < -10)
	   {
	   printf(ptrslw,-diff,"minutes.");
	   done=1;
	   }
	if( (wrtdif == 2) && (done == 1) )
	   {
	   adiff=(60.*diff - adiff)/elapse;
	   printf(ptrrat,adiff);
	   }
	}
	if(done == 0)
	{
	diff=60*diff +secat - sec;
	if(diff > 0)  printf(ptrfas,diff,"seconds.");
	if(diff < 0)  printf(ptrslw,-diff,"seconds.");
	if(diff == 0) printf("\n Time agrees to within 1 second.\n");
	if(wrtdif == 2)
	   {
	   adiff=(diff - adiff)/elapse;
	   printf(ptrrat,adiff);
	   }
	}
#endif
}
