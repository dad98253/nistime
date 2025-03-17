#include "nistime.h"
/*
        following is the block of global variables
        used by all of the subroutines
        these variables are defined here with their default
        values which are modified by reading a configuration
        file named settim.cfg.  this is done by subroutine setcfg.

	the global variables depend on which version this is.  the
	ibm-pc version converts from received UTC to local time
	based on the values in the configuration file, while UNIX
	versions store the time in UTC
*/
#include <stdio.h>
#ifdef IBMPC
int utcdif = -7;   /* local standard time - utc in hours */
int dsflag = 1;    /* daylight savings time? 1 = yes, 0 = no */
#endif
int cmport = 0;    /* 0 for com1 port, 1 for com2 port, etc. */
char number[256] = { 'A','T','D','T','9',',','4','9','4','4','7','7','4',
                   '\r', 0 , 0 , 0 , 0 , 0 };  /* telephone number to dial */
#ifdef IBMPC
int atflag = 1;    /* AT-type machine? 1=yes, 0=no */
#endif
int debug  = 0;    /* list diagnostic messages? 1=yes, 0=no */
int echo   = 1;    /* expect echoes from modem? 1=yes, 0=no  */
int hs     = 1;    /* 1200 baud dialing? 1=yes, 0=no, use 300*/
int retry  = 0;	   /* number of times to retry if busy/no answer */
#ifdef IBMPC
int lpt    = 0;    /* pulse lpt1 on time? 1=yes, 0=no */
int lpadr  = 0x37a; /* default command register for line printer */
#ifndef BIOS
int cmadr  = 0; /* command register for COM port, NO DEFAULT !!  */
#endif
#endif
int setclk   = 1;     /* 1=set clock, 0=check time only          */
int wrtdif   = 0;     /* 1=archive time dif, 0= do not 2=get rate too  */
FILE *jop;            /* file handle for writing difference */
/*
	following structure is used to read
	previous time difference from file if
	"A" mode is selected

	structure contains time of previous
	difference (year, month, day, hour,
	minute and second) followed by difference
	and units of difference.  note that the
	IBMPC version has two such pairs -- one for
	the RAM clock and one for the CMOS clock
*/
struct tmprev
	{
	int yrprev;
	int moprev;
	int dyprev;
	int hrprev;
	int mnprev;
	int scprev;
	float dffprv;
	char unprev;
#ifdef IBMPC
	float datprv;
	char uatprv;
#endif
} tmpp;
main(argc,argv)
int argc;
char *argv[];
{
/*
        this is the main program named rdline for task nistime.
*/
#ifdef IBMPC
#include <dos.h>
#endif
char buf[280],ichar;
void wrtbuf(),inilin(),hangup(),sndptr(),diftim(),arcdif();
int rdbuf(),cmplst();
void wait(),parset(),setcfg();
int dial();
int j,param;
int len;
int lim;
int tmo  = 0;
char ie1 = '*';
char ie2 = '#';
char ie3 = -120;
char ie3a= '>';    /* changed to allow titles to terminate with > after OTM */
int jm;
/*
        following constant sets timeout value for read.
        this version (7 jan 92) uses a time-out value that is
        specified directly in ticks and is not scaled by
        the cpu speed for the PC version.  the value is
        therefore independent of BIOS/non-BIOS access and
        machine speed.
        the new higher speed modems often take several seconds to
        start transmitting after the connect message is received.
        the time-out is set longer on the first line for that
        reason.
*/
#ifdef IBMPC
int ttmo = -72;		/*72 ticks is about 4 seconds */
int ttmoo = -360;	/*about 20 seconds -- used for first line only*/
#endif
#ifdef SUN
int ttmo = -20;
int ttmoo = -100;
#endif
/*
        following variables are used in cmplst and are defined
        here so that they remain defined across consecutive
        calls.
        osec is the previous value of the second
        obuf holds the time string of the previous transmission
        count is 0 until a line has been read
*/
int count = 0;
int osec  = 0;
char obuf[25];
/*
        see if there is a switch on the command line
*/
        if( (argc > 1) && (*argv[1] == '-') )
        {
        argv[1]++;
        if(*argv[1] == 'd') debug = 1;
        }
#ifdef IBMPC
        printf("\nNISTIME/ACTS Version of %s \n",__DATE__);
#endif
#ifdef SUN
	printf("\n Version of 28 October 1992 for SUN system \n");
#endif

/*
        open configuration file and set global parameters
*/
        setcfg();
/*
        following code sets number of times first loop is done.
        previous versions had a different limit for 300 bits/s, but
        there is no difference in this version since the time can
        now be set at either speed.
*/
        lim=12;
        if(debug != 0)
        {
        printf("\n input configuration \n %s",number);
#ifdef IBMPC
        printf("\n utcdif= %d",utcdif);
        printf("\n cmport= COM%1d",cmport+1);
        #ifdef BIOS
        printf("\n i/o via calls to BIOS routines.");
	#endif
        #ifndef BIOS
        printf("\n i/o via direct operations on port.");
        #endif
        printf("\n atflag= %d",atflag);
        printf("\n dsflag= %d",dsflag);
#endif
#ifdef SUN
	printf("\n port file descriptor=%d",cmport);
#endif
        printf("\n echo  = %d",echo);
        printf("\n hs    = %d",hs);
#ifdef IBMPC
        printf("\n lpt1  = %d",lpt);
        printf("\n lp adr= %x",lpadr);
#endif
        printf("\n set clk ?, (1=y)= %d",setclk);
        printf("\n wrt to file (1=y), (2=y & rate)=%d",wrtdif);
	if(retry == 0)
	  printf("\n Program will not retry if BUSY or NO ANSWER");
	if(retry < 0) printf("\n Program will ask before each re-try.");
	if(retry > 0)
	 printf("\n Up to %d retries if BUSY or NO ANSWER", retry);
        }  /* end of debug printing */
        if(setclk == 0)
              printf("\n Computer clock will not be set.");
/*
        initialize com line
*/
	if(debug != 0) printf("\n Initializing COM port.");
        inilin();
	if(debug != 0) printf("\n initialization complete.");
/*
        if debug is turned on and if direct calls are used, print
        effective address which was determined by inilin
*/
#ifdef IBMPC
#ifndef BIOS
        if(debug != 0) printf("\n hex. address of port=%x",cmadr);
#endif
#endif
/*
        now dial number -- continue if dial status shows connect
*/
        if(dial() == 1)
        {
/*
        now start reading from line
        note that 6 lines will be read followed by a time set.
        note that this version does not distinguish between
        300 and 1200.
*/
        for(j=0; j<lim; j++)
           {
           len=rdbuf(buf,ie1,ie2,ie3a,ttmoo);
           if( (len > 0) && (buf[0] > 0) )
              {
#ifdef IBMPC
              if(lpt != 0) sndptr();  /* pulse strobe of lpt1*/
#endif
              wrtbuf(&buf[len-1]);    /* echo terminating character */
#ifdef IBMPC
              printf("\n %s",buf);
#endif
#ifdef SUN
	      printf("%s",buf);       /* SUN version uses no newline*/
#endif	
           }
           else
              {
              tmo=1;     /* show exit on time out */
              if(debug != 0) printf("\n time out in main loop.");
              break;     /* get out of loop on a time-out*/
           }
         }
/*
        write difference to archive file if enabled
        and if the previous loop did not end on a time-out.
        if it did end on a time-out then skip this part since
        the connection has been lost
*/
        if( (wrtdif != 0) &&  (tmo == 0) )
           {
           len=rdbuf(buf,ie1,ie2,ie3,ttmo);
           if( (len > 0) && (buf[0] > 0) )
              {
#ifdef IBMPC
              if(lpt != 0) sndptr();
#endif
              wrtbuf(&buf[len-1]);
              arcdif(buf);
           }
           else
              {
              tmo = 1;
              if(debug != 0) printf("\n time out in arcdif.");
           }
        }
/*
	if clock setting has been requested and if the previous
	operations did not sense a time out then read the next
        two lines and be sure they are sequential in time.
        If they are, parse second line and set time.
        if sequential comparison fails, print message
        in comparison subroutine and try again.

        inner while loop continues until two consecutive lines pass
        the test or until the read fails on a time-out and tmo =1.
*/
        if( (setclk == 1) && (tmo == 0) )
          {
          do
           {
           len=rdbuf(buf,ie1,ie2,ie3,ttmo);
           if(debug != 0) printf("\n len=%d \n %s",len,buf);
           if( (len > 0) && (buf[0] > 0) )
              {
#ifdef IBMPC
              if(lpt != 0) sndptr();
#endif
              wrtbuf(&buf[len-1]);
              jm=cmplst(buf,&count,&osec,obuf);
              if(debug != 0)
                 printf("\n count=%d osec=%d \n %s",count,osec,obuf);
              if(jm == 1) parset(buf); /* jm=1 means times are sequential*/
           }
           else
              {
              tmo=1;
              if(debug != 0) printf("\n time out after parset.");
           }
          } while ( (jm < 1) && (tmo == 0) );
        }   /* end of if setclk == 1 && tmo == 0 */
 /*
        now read next line and then check the time
*/
        if(tmo == 0)
          {
          len=rdbuf(buf,ie1,ie2,ie3,ttmo);
          if( (len > 0) && (buf[0] > 0) )
           {
#ifdef IBMPC
           if(lpt != 0) sndptr();
#endif
           wrtbuf(&buf[len-1]);
           diftim(buf);
          }
          else
           {
           tmo = 1;
           if(debug != 0) printf("\n timeout after diftim.");
          }
        }
/*
        if lpt is on, then continue reading lines until ended
        by a timeout and if we have not had a timeout yet
*/
#ifdef IBMPC
        if( (lpt != 0) && (tmo == 0) )
        {
           do
              {
              len=rdbuf(buf,ie1,ie2,ie3,ttmo);
              if( (len > 0) && (buf[0] > 0) )
                 {
                 sndptr();
                 wrtbuf(&buf[len-1]);
                 printf("\n %s",buf);
                 }
               } while (len > 0);     /* loop until nothing more read */
        }
#endif
        hangup();
        }
}
