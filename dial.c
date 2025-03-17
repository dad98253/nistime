int dial()
{
/*
        this subroutine dials a hayes modem connected
        to the com port using calls to wrtbuf and rdbuf.

        if the first character in the dialing sequence (after
        the ATD) is M (or m), then "manual" dialing is assumed.
        in that case, just return and wait for received message.

        if echo is set to zero by the configuration file, then
        echoes are not expected from the modem.  if echo is not zero
        then echoes are expected and a fatal error exists if
        echoes are not received within the time out period.

        this version of the program (7 jan. 92) uses time-out values
        that are specified in units of clock ticks.  this method
        eliminates the need for time-out values that are scaled by
        the CPU speed and is intended to be better suited to the
        faster 386 and 486 configurations where cpuspd sometimes does
        not give the correct relative speed value so that the
        time-outs are wrong (usually too short).
*/
#include "nistime.h"
#include <stdio.h>
#ifdef IBMPC
#include <dos.h>
#endif
void wrtbuf(),wait();
int rdbuf();
char *ptr;
char ans[30],cc;
char ie1 = 'K';
char ie2 = '\n';
char ie3 = '\r';
char ie4 = 'Y';        /* response is BUSY */
char ie5 = 'T';        /* response is CONNECT */
char ie6 = 'R';        /* response is NO ANSWER or NO CARRIER */
char ie7 =  -120;
int j,k,kk,stat;
#ifdef IBMPC
int tmo  =  -200;     /* time-out limit, roughly 10 seconds */
int ttmo =  -900;    /* wait up to 50 sec for number to be dialed */
#endif
#ifdef SUN
int tmo = -10;
int ttmo= -200;     /* this is a LONG time-out. */
#endif
extern char number[];
extern int echo;
extern int debug;
extern int retry;	/*how to handle BUSY or NO ANSWER */
char tnum[42];		/*temporary buffer for telephone commands */
        if( (number[3] == 'm') || (number[3] == 'M') )
        {
        printf("\n Ready for manual dialing. Please dial now. \n enter");
        printf(" y <return> if connected, n <return> otherwise\n");
        scanf("%c",&cc);
        if( (cc == 'y') || (cc == 'Y')) return(1);
        return(0);
        }
again:		/*try to dial the number from the start */
        ptr="ATZ\r";
        wrtbuf(ptr);
        if( (rdbuf(ans,ie1,ie7,ie7,tmo) == 0) && (echo != 0) )
        {
        printf("\n No response from modem. -1\n");
	printf("\n ATZ sent and OK expected.");
        abort();
        }
        if( (rdbuf(ans,ie2,ie7,ie7,tmo) == 0) && (echo != 0) )
        {
        printf("\n No response from modem. -2\n");
	printf("\n ATZ send and OK or newline expected.");
        abort();
        }
        wait();
/*
        set: echo on, full duplex, speaker on, basic word responses on
*/
        ptr="AT E1 M1 Q0 V1\r";
        wrtbuf(ptr);
        if( (rdbuf(ans,ie1,ie7,ie7,tmo) == 0) && (echo !=0) )
        {
        printf("\n No response from modem. -3\n");
	printf("\n Configuration commands sent, OK expected.");
        abort();
        }
        wait();
/*
	if number[3] is T/t or P/p, then we have an original-style
	telephone number of the form ATDT ... or ATDP ...
	in that case we just send the whole thing off.

	if number[3] is A/a then there is a more complex command
	string to the modem which begins with AT in space 3.  Send
	that string off without interpretation.
	The modem buffer can only hold 40 characters, but this may
	not be long enough to accept a complex command plus a
	telephone number in one swallow.  The character | in the
	modem string will act as a temporary terminator.
*/
        if( (number[3] == 'T') || (number[3] == 't') ||
            (number[3] == 'P') || (number[3] == 'p') ) wrtbuf(number);
        else	/*more complex command is present*/
           {
           j=3;
mrenum:
           kk=1;		/*show null terminator found */
           for(k=0; k<40; k++)
              {
              tnum[k]=number[j++];
              if(tnum[k] == 0) goto sndnum;	/*final terminator found*/
              if(tnum[k] =='|')		/*send this bit off and repeat*/
                 {
                 tnum[k++]='\r';	/*add return and terminating null*/
                 tnum[k]='\0';
                 kk=0;			/*show not done yet */
                 goto sndnum;		/*send this part to modem*/
              }		/*end of intermediate terminator found*/
           }		/*end of for loop over temporary buffer*/
sndnum:
	   wrtbuf(tnum);
	   fprintf(stderr,"\n Sending Modem Command: %s \n",tnum);
	   wait();
	   wait();
	   if(kk == 0) goto mrenum;	/*more to do*/
	}		/*end of more complex command present*/
        if( (rdbuf(ans,ie3,ie7,ie7,tmo) == 0) && (echo !=0) )
        {
        printf("\n No response from modem. -4\n");
	printf("\n Telephone number sent, number echo expected.");
        abort();
        }
/*
	now wait for response from modem.  there are several
	possibilities as follows:
	1.  modem does not response at all.  this is almost
	    certainly an error -- flag immediately if echo
	    mode enabled. otherwise parse response.

	2.  read terminates on R from RINGING -- only some modems
	    can do this -- go back and read some more. do not
	    reset buffer pointer here since response is only
	    partial.

	3.  read terminates on R from NO ANSWER or NO CARRIER --
	    that's all folks.  Same thing if read terminates on Y
	    from BUSY.

	4.  read terminates on T from CONNECT -- the kind of
	    stuff we like to see.

	5.  read terminates on T from AT or ATDT -- throw
	    it away and read some more.  this should only
	    happen on SUN machines where the reads are buffered
	    on IBMPC where the input port is not buffered the
	    ATDT string should have been overwritten by the
	    digits of the number.
	the variable stat is 0 until we are finished and is
	1 when the final modem status has been received
	kk points to the origin of the buffer for the current
	read.  it is zero unless a partial answer has been
	received (NO CAR for example).
*/
	stat=0;
	kk=0;
	do
	{
/*
	   read some stuff, abort if timeout and no response
	   (modem should have given status of call).

	   when rdbuf returns, k points to terminator relative to
	   origin which is kk.
*/
           if( (k=rdbuf(&ans[kk],ie4,ie5,ie6,ttmo)) == 0 )
              {
              printf("\n No response from modem. -5\n");
	      printf("\n Command to dial sent, call status expected.");
              abort();
              }
/*
	   if we read more than one character, and
	   if end of message is ER, we have final modem status.
	   answer is either NO ANSWER or NO CARRIER. bad news
	   either way but we are finished.
	   likewise if the message ends in SY, then the call
	   status is BUSY.  Also bad news, but we are finished
*/
           if( (k  > 1) && (ans[kk+k-1] == 'R') && (ans[kk+k-2] == 'E'))
	      stat=1;
	   if( (k  > 1) && (ans[kk+k-1] == 'Y') && (ans[kk+k-2] == 'S'))
	      stat=1;
/*
	   if we read more than one character and
	   if end of message is CT we have final modem status.
	   answer is CONNECT (possibly CONNECT 1200).
*/
	   if( (k > 1) && (ans[kk+k-1] == 'T') && (ans[kk+k-2] == 'C') )
	      stat=1;
/*
	   if we terminated on an R and it is the first character, then
	   it is the R of RINGING. if we read more than one character
	   and we ended with an R and the previous character was not E,
	   then we are in the midst of NO CARRIER  -- keep on reading.
*/
	   if( (kk+k > 1) && (ans[kk+k-1] == 'R') && (ans[kk+k-2] != 'E') )
	      kk += k;
	   if( (kk+k == 1) && (ans[0] == 'R') )
	      kk += k;
	} while (stat == 0);
/*
	since modem response was set to words, skip leading
	extraneous stuff including remnants of number and possible
	leading newline
*/
	for(j=0; ( (j < k+kk) && (ans[j] < 'A')) ; j++)  ;
        printf("\n modem answer= %s\n",&ans[j]);
        for(k=j; (cc=ans[k]) != 0; k++) if(cc == 'T') return(1); /*connect !*/
/*
	if this attempt did not work and retries are either
	exhausted or disabled then too bad.  Return to caller
	with a failure status

	else if manual retries have been requested, then prompt
	operator and either retry or exit depending on response.

	else if automatic retries were selected try again. Decrement
	retry to keep track of how many times we have done this.
	Eventually there will be no more left and then
	we will return to caller with a failure status
*/
        if(retry == 0) return(0);
	if(retry < 0)
	   {
	   printf("\n Try again?");
	   printf("\n Please respond with y or n followed by <return>");
	   printf("\n Ans=    ");
	   scanf("%c",&cc);
	   if( (cc == 'y') || (cc == 'Y') )
	    {
	    while( (cc=getchar()) != '\n')  ; /*skip trailing newline */
	    goto tryit;
	   }
	   exit();
	}
/*
	if we are here then retry > 0 and automatic retries are
	selected and available.  So decrement the count and then
	go around again.  This loop will either eventually connect
	or retry will go to 0 and we will return a failed status
	above.
*/
	printf("\n Dialing again, %d re-tries left.",retry);
	retry--;
tryit:		/*   wait a few seconds and then try it again */
	wait();
	wait();
	goto again;
}
