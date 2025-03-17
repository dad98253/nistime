main (argc,argv)
int argc;
char *argv[];
{
/*
	program sleep loops for the amount of time requested
	on the command line and then exits.  it can be used
	to delay execution of a batch file, for example.
*/
#include <stdio.h>
int sw();        /* following variables are used to parse command */
char s;
long int v;
long int secs;   /* delay in seconds */
	while(sw(argc,argv,&s,&v) != 0)
	   {
	   switch(s)
	      {
	      case 'h':
		secs=3600*v;
		break;
	      case 's':
		secs=v;
		break;
	      default:
		fprintf(stderr,"\n unknown switch ,%c %d \n",s,v);
		break;
	   }
	   argc--;
	   argv++;
	}
	do
	  {
/*	  printf("\n sec remaining= %ld",secs);      */
	  wait();
	  secs--;
	} while (secs > 0);
}
