int cmplst(buf,count,osec,obuf)
char buf[];
int *count;
int *osec;
char obuf[];
{
/*
        this subroutine compares the current received date,time and dst
        flag as part of the line received in buf
        with the previously received values in the array obuf and osec
        which hold the old time string and seconds, respectively.
        the current values are then copied into array obuf to get ready
        for the next comparison.
        in the current version, the operation is different at 300 and
        1200 since there is only a transmission every other second at
        300.  thus two consecutive transmissions will differ by 1 at
        1200 but by 2 at 300.  the wrap problem at 300 is the same as
        at 1200. if the two lines are consecutive (dates equal and
        seconds differ by 1 at 1200 or 2 at 300) then cmplst returns 1.

	if this is the first comparison then cmplst copies the line into
        the buffer and returns 0. if the value of the second is < 58 then
        count is incremented so that a comparison will be made on the next
        call. Note that the transmission before 58 is 57 at 1200 and 56 at
        300.  if the second is >= 58 count is left alone since the next
        second may wrap to the next minute making comparisons
        very difficult. if the comparison fails, cmplst returns -1.
*/
#include <stdio.h>
char nbuf[25];       /* current time string*/
int nsec;            /* current seconds value*/
int  j,k,l;          /* used for loop control */
int err;             /* used to hold detected error */
extern int hs;       /* 1 if dialing at 1200, 0 if at 300 */
/*
        if first time through just do copy of old to new.
        extract current seconds and store in osec
        if current sec is 58, 59 or 60 leave count at zero so
        that next entry will be treated as first one to prevent a
        comparison across a possible wrap
        -> set seconds field to spaces to prevent comparison error
        on change in seconds value
*/
        if(*count == 0)
           {
           for(j=0; (buf[j] != 0) && (buf[j] != '-'); j++) ; /*find - */
/*
	if previous loop ended on the zero byte then there must be
	a format error on the line.  just throw this line away and
	wait for the next one -- leave count alone and return 0
*/
	   if(buf[j] == 0) return (0);
           l=0;
           for(k=j-2; k<j+18; k++) obuf[l++]=buf[k];
           *osec=10*(obuf[15] - '0') + (obuf[16] - '0');
           obuf[15]=obuf[16]=' ';
           obuf[21]='\0';
           if(*osec < 58) (*count)++;
           return(0);
           }
/*
        store corresponding segment of current line in nbuf, compute
        current second and compare both
*/
        for(j=0; (buf[j] != 0) && (buf[j] != '-'); j++)  ;
        l=0;
        for(k=j-2; k<j+18; k++) nbuf[l++]=buf[k];
        nsec=10*(nbuf[15] - '0') + (nbuf[16] - '0');
        (*osec)++;                /*advance previous to next second*/
        if(hs == 0) (*osec)++;	  /*difference is 2 seconds at 300 */
        nbuf[15]=nbuf[16]=' ';
        err=1;
        for(k=0; k<20; k++) if(nbuf[k] != obuf[k]) err= -1;
        if(nsec != *osec) err= -1;
        if(err == -1)
        {
        nbuf[20]=obuf[20]='\0';
        printf("\n sequence error");
        printf("\n first line=%s, sec=%d",obuf,*osec);
        printf("\nsecond line=%s, sec=%d",nbuf, nsec);
        *count=0;
        }
        return(err);
}
