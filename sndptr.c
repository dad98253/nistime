void sndptr()
{
#include "nistime.h"
#ifdef IBMPC
#include <dos.h>
/*
        pulses strobe line on printer

	note -- only used for IBMPC version. if IBMPC is not
	defined, this is a do - nothing
*/
extern int lpadr;
int j;
        outportb(lpadr,1);
        for(j=0; j<3; j++) ;
        outportb(lpadr,0);
#endif
}
