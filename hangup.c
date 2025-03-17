void hangup()
{
/*
        this subroutine hangs up the modem.
        hayes protocol is used.
        first send a % character to get the distant end to stop
        sending. then put the local modem in the command mode
        and hang it up.
        Note: if manual mode was selected at dialing then we assume
        that we can not hangup the telephone because it is not a Hayes
        compatible.  so send the % character and then return.

        this routine is slightly dependent on whether or not BIOS calls
        are used to communicate with the modem.  the non-bios calls are
        faster and different time-out values are needed.
*/
#include "nistime.h"
#ifdef IBMPC
int tmo = -80;	/* about 4 seconds */
#endif
#ifdef SUN
int tmo = -5;
#endif
extern char number[];
extern int debug;
char *ptr;
char ans[400];
char ie1 = 'K';		/* response is ... OK */
char ie2 = '+';		/* response is +++ since already in command mode*/
char ie2a = '\r';	/* echo of transmitted carriage return */
char ie3 = '0';		/* response is 0 instead of OK */
char ie3a = 'R';	/* response is ... NO CARRIER ... */
void wrtbuf(),wait();
int rdbuf();
int j;
        ptr="%";
        wrtbuf(ptr);
        if(debug != 0) printf("\n Start hangup, send %");
/*
	NIST should hang up immediately and the local modem
	should drop back to command mode when it detects the
	loss of carrier.  This may take a few seconds to happen,
	and the modem may respond with "NO CARRIER" as a result.
*/
        for(j=0; j<4; j++) wait();
/*
        return now if manual mode dialing
*/
        if( (number[3] == 'm') || (number[3] == 'M') ) return;
        if(debug != 0) printf("\n Send +++ to Modem.");
        ptr="+";
        wrtbuf(ptr);
        wait();
        wrtbuf(ptr);
        wait();
        wrtbuf(ptr);	/* send + + + 1 second apart */
/*
	if the modem returned to command mode following the %
	character, then it should echo the + + +.  If the modem
	returned as a result of the + + + then it should now
	echo OK.  Some modems may wait quite a while before they
	decide that the carrier is lost.
*/
        rdbuf(ans,ie1,ie2,ie3a,tmo);
        wait();
        if(debug != 0) printf("\n Send ATH0 to Modem.");
        ptr="ATH0\r";	/*hang up modem */
        wrtbuf(ptr);
        rdbuf(ans,ie1,ie2a,ie3,tmo);
	wait();
	if(debug != 0) printf("\n send ATZ to Modem.");
        ptr="ATZ\r";	/* reset modem */
        wrtbuf(ptr);
        rdbuf(ans,ie1,ie2a,ie3,tmo);
        if(debug != 0) printf("\n Hangup Completed.");
}
