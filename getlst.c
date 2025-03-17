int getlst()
{
/*
	this subroutine reads the last value in the difference file
	and stores it in the global array tmprev.  this is a structure
	of integers giving the year, month,day, hour, minute and second
	of the most recent time difference followed by the value of
	that time difference.

	since an IBMPC may have two clocks, there are two time differences
	in the structure if IBMPC is defined.  the first is the previous
	difference of the RAM clock and the second is for the CMOS clock
	of AT-type machines.
*/
#include "nistime.h"
#include <stdio.h>
extern struct tmprev
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
extern FILE *jop;
FILE *fopen();
extern int debug;
int jj,num;
#ifdef SUN
	num= 8;
#endif
#ifdef IBMPC
	num= 10;
#endif
/*
	open difference file and skip to last line
	if open fails signal that to caller
*/
	if( (jop=fopen("nistime.dif","r+t")) == 0) return (0);
	do
	{
	jj=fscanf(jop," %2d-%2d-%2d %2d:%2d:%2d%9f%c",
	&tmpp.yrprev,&tmpp.moprev,&tmpp.dyprev,&tmpp.hrprev,
	&tmpp.mnprev,&tmpp.scprev,&tmpp.dffprv,&tmpp.unprev);
#ifdef IBMPC
	jj += fscanf(jop," %9f%c",&tmpp.datprv,&tmpp.uatprv);
#endif
	} while (jj == num);
	if(debug != 0)
	   {
	   printf("\n previous time difference\
	   \n%2d %2d %2d %2d %2d %2d %f  units= %c",
	   tmpp.yrprev,tmpp.moprev,tmpp.dyprev,tmpp.hrprev,
	   tmpp.mnprev,tmpp.scprev,tmpp.dffprv,tmpp.unprev);
#ifdef IBMPC
	   printf("%f %c \n",tmpp.datprv,tmpp.uatprv);
#endif
	   }
	if(tmpp.yrprev < 80) tmpp.yrprev += 100;	/*years since 1900*/
	return (1);
}
