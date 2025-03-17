int unpbcd(ival)
int ival;
{
#include "nistime.h"
#ifdef IBMPC
/*
        this routine receives a packed BCD value in ival (as read
        from the CMOS clock) and returns the integer value.
        it is the inverse function to cnvbcd, which produces a packed
        BCD value from an integer input

	note that this function is not used in the SUN version and
	will be compiled as a do-nothing if IBMPC is not defined
*/
        return(10*(ival >> 4) + (ival & 0xf) );
#endif
}
