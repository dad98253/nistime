char cnvbcd(val)
int val;
{
#include "nistime.h"
#ifdef IBMPC
/*
        this subroutine converts the integer value passed
        in val into two 4-bit BCD digits.  the two digits
        are packed into a byte and are returned by the
        function.  this routine is used to format the
        values for setting the CMOS clock.

	this routine is only used in the IBMPC version and
	will compile as a do-nothing if IBMPC is not defined
*/
int tendig,unidig;
        tendig=val/10;
        unidig=val - 10*tendig;
        return(16*tendig + unidig);
#endif
}
