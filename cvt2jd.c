#include "sizint.h"
LONG cvt2jd(yr,mo,day)
int yr;
int mo;
int day;
{
LONG mjd;  /*holds result*/
int ilp;       /*number of leap years from 1900 not counting current*/
static int dmo[12] = {0,31,59,90,120,151,181,212,243,273,304,334};
/*
	this subroutine receives a date in the form year, month and day
	and returns the MJD corresponding to that day.  the year may
	be specified as 90 or 1990.
*/
	if(yr > 1900) yr -= 1900;    /* convert to years since 1900 */
	ilp=(yr-1)/4;                /* number of leap years since 1900*/
/*
	compute number of days since 1900 + 1 day for each leap year
	+ number of days since start of this year
*/
	mjd = 365*(LONG) yr + (LONG) (ilp + dmo[mo-1] + day - 1);
	mjd += 15020;      /* add MJD of 1900 */
/*
	if current month is jan or feb then it does not matter if
	current year is a leap year
*/
	if(mo < 3) return(mjd);
/*
	if current month is march or later then must add 1 day if
	current year is divisible by 4
*/
	if( (yr & 3) == 0) mjd++;
	return(mjd);
}
