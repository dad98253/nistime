void setcfg()
{
/*
        this subroutine opens the configuration file named
        nistime.cfg and sets the various global parameters
        based on the contents.


        if the file does not exist
        the globals are not altered and the default configuration
        is used.  the global parameters are defined in the main
        program preamble.
	the configuration file is slight different for the different
	versions of the program.
	the first line always contains the telephone number.
	the second line contains the port specifier at the start.
	for the ibm pc version, this is a single digit or h (for
	a hardware address to be specified on line 6). for the
	UNIX version, if the first two character are letters or
	numbers (nn), they define the port address as /dev/tty<nn>.
	if the first is a letter or number (n) and  the second is - then
	the port address is /dev/tty<n>. if both positions are -, then
	the port is specified by its full path on the last line.
*/
#include "nistime.h"
#include <stdio.h>
#ifdef SUN
#include <sys/file.h>
#include <fcntl.h>		/*this include not needed in some systems */
#endif
#include <ctype.h>
FILE *fopen();
FILE *iop;
int getlst();
extern FILE *jop;
extern int debug;       /* turn on debug mode if = 1 */
#ifdef SUN
char dv[20]; /*device path name*/
#endif
int j,eol,icol;
char c;
char *peof,*perr;
/*
	the IBMPC version can convert the received time
	from UTC to local time using the following two flags.
	the SUN version always sets the time directly in
	UTC seconds since 1970 since that is the UNIX standard
*/
#ifdef IBMPC
extern int utcdif;         /* local time - utc in hours */
extern int dsflag;         /* daylight savings time? 1=yes, 0=no */
#endif
/*
	for the IBMPC version, cmport specifies the comport, 0=com1,
	1=com2, etc.
	for the SUN version, cmport is the file handle returned when
	the comport is opened at the end of this routine.
*/
extern int cmport;
#ifdef IBMPC
extern int atflag;         /* 1=AT-type machine with CMOS clock, 0= not */
#endif
extern char number[];      /* number to dial with trailing <cr> and 0 */
extern int echo;           /* expect echoes from modem? 1=yes, 0=no */
extern int hs;             /* 1 = use 1200 baud, 0=use 300 baud     */
#ifdef IBMPC
extern int lpt;             /* 1=  pulse lp on time, 0= do not pulse */
#endif
extern int setclk;         /*  1= set computer clock, 0= do not set */
/*
	wrtdif = 1      compute current time difference and write to file
	wrtdif = 0      do not write to file
	wrtdif = 2      write current difference to file and estimate rate
	 		 using second difference divided by elapsed time
*/
extern int wrtdif;
#ifdef IBMPC
#ifndef BIOS
extern int cmadr;          /* hardware address of port for _d version*/
#endif
#endif
extern int retry;	   /* number of times to retry if BUSY or NO ANSWER*/
        perr="\n Syntax error in config. file ";
        peof=" -- premature EOF \n ";
        iop=fopen("nistime.cfg","rt");
        if(iop == 0)
        {
        printf("\n can't open file nistime.cfg, use default configuration.");
        return;
        }
/*
        first line of file is full telephone number. If this line
        begins with either T or P then we have an old-style command
        file.  read the characters and store then with a leading
        ATD which starts any command to the modem.
        if this line begins with A then it is a more complex
        string of modem commands which we may not understand.
        read the string in this case without interpretation --
        it will be passed by the dial subroutine directly to
        the modem.  If there is a problem here, we may not be
        able to detect it until the modem returns some kind of
        error status.
*/
        eol=0;
        for(j=3; (j<256) && (eol==0); j++)
          {
          c=getc(iop);
          switch(c)
           {
           case EOF:
              printf("%s %s",perr,peof);
              abort();
              break;
           case '\n':
              number[j]='\r';        /*terminate with carriage return*/
              number[j+1]=0;        /* and then a null */
              eol=1;
              break;
           default:
              number[j]=c;
              break;
          }
        }
        if(eol == 0)
          {
          printf("%s -- telephone # too long. \n",perr);
          abort();
        }
/*
	for ibm version:
	next line contains comport, 1 = com1, 2 = com2 in col 1, etc.
        if first line contains h, then comport hardware address will be
        specified on sixth line of file.  if comport is specified by
	number, sixth line of file is not used.
        second char sets echo: B=0, anything else =1,
        third character is baud rate, h=1200, l=300;
        fourth character is line printer y=yes, n=no.
        fifth character is set clocks, s=set, d=disable set.
        rest of line is ignored and may be used for a comment

	for sun version:
	if first two characters are alphanumeric, they specify port.
	if first character is alphanumeric and second is -, first is port.
	if first is - then port is specified by its full address.
	note that length of port specifier may be 1 or 2 chars so that
	the word next below depends on how the port was specified
	next character is echo as above.
	next is baud rate as above
	next is set/ no set as above.
*/
	icol=0;			/* keep track of what col. we are in*/
        c=getc(iop);
	icol++;
        switch (c)
        {
        case EOF:
           printf("%s %s",perr,peof);
           abort();
           break;
#ifdef IBMPC
        case '1':
           cmport=0;
           break;
        case '2':
           cmport=1;
           break;
        #ifndef BIOS
        case '3':
           cmport=2;
           break;
        case '4':
           cmport=3;
           break;
        case 'h':
        case 'H':
           cmport= -1;
           break;
        #endif
        default:
           #ifdef BIOS
           printf("%s -- comport not 1 or 2 \n",perr);
	   #endif
	   #ifndef BIOS
           printf("%s -- comport must be 1,2 3,4 or h \n",perr);
           #endif
           abort();
           break;
#endif
#ifdef SUN
	case '-':           /* port to be specified by path name */
	   cmport= -1;
	   break;
	default:
	   if( (isalpha(c) == 0) && (isdigit(c) == 0) )
		{
		printf("%s -- port address must be alphanumeric \n",perr);
		abort();
		break;
		}
/*
	name begins with /dev/tty then add chars just read
*/
	   sprintf(&dv[0],"%s","/dev/tty");
	   dv[8]=c;
	   c=getc(iop);
	   icol++;
	   if(c == '-')             /* comport is only 1 char long*/
		{
		dv[9]='\0';         /*terminate string with null*/
		cmport=1;           /*show port specified ok.   */
		break;
		}
	   if( (isalpha(c) == 0) && (isdigit(c) == 0) )
		{
		printf("%s -- port address must be alphanumeric \n",perr);
		abort();
		break;
		}
	   dv[9]=c;            /* get second character of comport*/
	   dv[10]='\0';        /* and terminate with null byte */
	   cmport= 1;          /* show port specified ok.      */
	   break;
#endif
        }
        c=getc(iop);
	icol++;
        switch(c)
        {
        case EOF:
           printf("%s %s", perr,peof);
           abort();
           break;
        case 'b':
        case 'B':
           echo=0;
           break;
        case 'e':
        case 'E':
           echo=1;
           break;
        default:
           printf("\nWarning, Line 2 Char %d not e or b -- e assumed.",icol);
           echo=1;
           break;
        }
        c=getc(iop);
	icol++;
        switch (c)
        {
        case EOF:
           printf("%s %s",perr,peof);
           abort();
        case 'h':
        case 'H':
           hs=1;
           break;
        case 'l':
        case 'L':
           hs=0;
           break;
        default:
           printf("\n Warning, Line 2 Char %d not h or l -- h assumed.",icol);
           hs=1;
           break;
        }
#ifdef IBMPC
        c=getc(iop);
	icol++;
        switch (c)
        {
        case EOF:
           printf("%s %s",perr,peof);
           abort();
        case 'y':
        case 'Y':
           lpt=1;
           break;
        case 'n':
        case 'N':
           lpt=0;
           break;
        default:
           printf("\n Warning, Line 2 Char %d not y or n -- n assumed.",icol);
           lpt=0;
           break;
        }
#endif
        c=getc(iop);
	icol++;
        switch (c)
        {
        case EOF:
           printf("%s %s",perr,peof);
           abort();
        case 's':
        case 'S':
           setclk=1;
           break;
        case 'd':
        case 'D':
           setclk=0;
           break;
        default:
           printf("\n Warning, Line 2 Char %d not s or d -- d assumed.",icol);
           setclk=0;
           break;
        }
        c=getc(iop);
	icol++;
        switch (c)
        {
        case EOF:
           printf("%s %s",perr,peof);
           abort();
/*
	the next character specifies if the time difference is to be
	stored in the archive file.  a response of "a" means store difference
	and a response of "n" means do not store it.  any other response is
	taken to be "n" and a diagnostic is printed.  In addition, an upper
	case response "A" also means estimate the rate offset of the clock
	by comparing the last difference with the current difference and
	dividing the second difference by the elapsed time.  this value is
	the rate offset with respect to UTC(NIST).
*/
	case 'A':
	   if( getlst() == 0)
	      {
	      printf("\n Can't read previous difference for rate estimate.");
	      wrtdif=0;
	      }
	   else
	      {
	      wrtdif=2;
	      break;
	      }
        case 'a':
           wrtdif=1;
           if(  (jop=fopen("nistime.dif","at")) == 0)
              {
              printf("\n Cannot open file for writing time difference");
              wrtdif=0;
              }
           break;
        case 'n':
        case 'N':
           wrtdif=0;
           break;
        default:
           printf("\n Warning, Line 2 Char %d not a or n -- n assumed.",icol);
           wrtdif=0;
           break;
        }
/*
	the next character specifies how to deal with BUSY or NO ANSWER
	from the modem.  If the next character is R, then parameter retry
	is set to 6, which will retry to dialing sequence up to 6
	additional times; r specifies up to 3 additional times; m is
	manual mode -- ask the operator before each retry and do as many
	as we are told and anything else means no retries at all. Note
	anything else includes a newline, so that older versions of the
	configuration file that did not support retries will result in
	the retry feature being disabled which is consistent with the
	old action.
*/
	c=getc(iop); 	/*read next character and advance col.  counter*/
	icol++;
	switch (c)
	{
	case EOF:	/* Error, premature EOF */
	  printf("%s %s",perr,peof);
	  abort();
	  break;
	case 'R':
	  retry=6;
	  break;
	case 'r':
	  retry=3;
	  break;
	case 'm':
	case 'M':
	  retry= -1;	/*signal manual mode */
	  break;
	case '\n':
	  retry=0;	/*newline means no retries */
	  goto ss;	/*bypass skip past newline below */
	default:
	  retry=0;	/*anything else also means no retries */
	  break;
	}	/* end of switch statement */
        while( ( (c=getc(iop)) != '\n') && (c != EOF) );/*skip rest of line*/
        if(c == EOF)
        {
        printf("%s %s",perr,peof);
        abort();
        }
ss:		/* continue reading next line */
#ifdef IBMPC
/*
        next line contains utcdif as number for hours or symbol for zone
	note that this is not used for SUN since conversion is to UTC
	directly.
*/
        c=getc(iop);
        switch (c)
        {
        case 'p':
        case 'P':
           utcdif= -8;
           break;
        case 'm':
        case 'M':
           utcdif= -7;
           break;
        case 'c':
        case 'C':
           utcdif= -6;
           break;
        case 'e':
        case 'E':
           utcdif= -5;
           break;
        case 'z':
        case 'Z':
           utcdif=0;
           break;
        case '-':
        case '+':
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
           ungetc(c,iop);
           fscanf(iop,"%d",&utcdif);
           break;
        default:
           printf("\n %s -- syntax error in Line 3 -- utc dif.\n",perr);
           abort();
           break;
        }
        while( ( (c=getc(iop) ) != '\n') && (c != EOF));/*skip to next line*/
        if(c == EOF)
        {
        printf("%s %s",perr,peof);
        abort();
        }
        j=fscanf(iop,"%d \n%d",&dsflag,&atflag);
        if(j == EOF)
        {
        printf("%s %s",perr,peof);
        abort();
        }
        if(j != 2)
        {
        printf("%s -- syntax error in AT flag or ds flag \n",perr);
        abort();
        }
#endif
/*
        if the comport is to be specified by its hardware address,
        then the first character on line 2 was 'h' for the IBMPC
	version and '-' for the SUN version -- comport is now -1.
        if so, read the hardware address from the next line.  if comport
        is positive or zero, then next line is ignored.

	note that line is read differently depending on which version
	is being generated.  for IBMPC is is read as a hex address
	while for  the SUN it is read as a full path name.
	
	also for the IBMPC the last read was for the AT and ds flags
	and the newline must be skipped at the end.  for the sun
	the last read was for line 2 and the newline at the end
	of line 2 has already been skipped there

	also note that the IBMPC version of the code is only compiled
	if the non-BIOS version is being generated.  The bios version
	does not used the hardware address of the port
*/
        if(cmport < 0)
        {
#ifdef IBMPC
#ifndef BIOS
        while( ( (c=getc(iop)) != '\n') && (c != EOF) ) ;/*skip to next line*/
        if(c == EOF)
           {
           printf("%s %s",perr,peof);
           printf("\n while trying to read comport hardware address.");
           abort();
           }
        j=fscanf(iop,"\n%x",&cmadr);
#endif
#endif
#ifdef SUN
	j=fscanf(iop,"%s",&dv[0]);
#endif
        if(j < 1)
           {
           printf("\n Syntax error reading comport hardware address.");
           abort();
           }
	}
#ifdef SUN
/*
	now try to open port using path name either read above
	or constructed from template
*/
	if(debug != 0) printf("\n port= %s",dv);
	cmport=open(dv, O_RDWR | O_NDELAY);
	if(debug != 0) printf("\n port file descriptor=%d",cmport);
	if(cmport < 0 )
	   {
	   printf("\n error opening port %s",dv);
	   abort();
	   }
#endif
        fclose(iop);
}
