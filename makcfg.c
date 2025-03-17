main()
{
/*
        This is program MAKCFG.  it is used to make
        a configuration file for use by program nistime.
        it builds the file based on the answers to a series
        of questions.
	this program can be used to build different versions
	of the configuration file depending on which system
	nistime will be used.  note that not all of the following
	variables are used by all versions
*/
#include <stdio.h>
#include <ctype.h>
#include "nistime.h"
char number[256];     /* holds telephone number */
char path[40];        /* holds path name for sun system port*/
char dv1,dv2;         /* sun system port alphanumeric chars */
char cmport;          /* holds comport, 1,2 (or 3,4 for non-bios calls */
char cmadr[4];         /* holds com port hardware address if necessary */
int bios;            /* 1 for bios i/o, 0 for non-bios calls */
char echo;            /* e for echo, b for blind*/
char hs;             /* h for 1200, l for 300 */
char lpt;            /* y pulse lpt port, n do not pulse */
char setclk;         /* s, set clocks, d do not set them */
char utcdif[4];          /* local time - utc in hours */
int dsflag;          /* 1 for daylight savings time, 0 for no */
int atflag;          /* 1 for AT with CMOS clock, 0 for no    */
char wrtdif;         /* a to write time difference to NISTIME.DIF, n=don't)*/
char retry;	     /* r=retry 3 times, R=retry 7 times, m=manual, n=no*/
char buf[30],cc,dd;     /* temporary register for answers  */
char *help0,*help1,*help2,*help3,*help4,*help5,*help6,*help7,*help8;
char *help9,*helpa,*helpb,*helpc,*helpd,*helpe,*helpf,*helpg,*helph;
int stat;            /* 1 means reply ok, 0 means reply in error */
int mach;     /* 1=IBMPC, 2=SUN, 3=RSX11M, 4=VAX/VMS */
int j,k;
FILE *fopen();
FILE *iop;
char *fname;
/*
	the sun version of C does not recognize the __DATE__
	macro so define it here as a simple string.  the pre-processor
	in Turbo C will assign the compilation date to this string
	automatically
*/
#ifdef  SUN
char *__DATE__;
	__DATE__="01-20-93";
#endif
/*
        following are texts of help messages
*/
        help0="\n This program uses your answers to generate a file\
               \n named nistime.cfg in your current default directory.\
               \n If a file by this name already exists there,\
               \n this question allows you to stop before the existing\
               \n file is overwritten, which will happen if you select\
               \n the Continue option.  If you select Rename, the\
               \n existing file is renamed to config.old and then\
               \n a new file named nistime.cfg is produced.\
               \n The rename will fail if config.old already exists.";
        help1="\n This question is used to generate responses for the\
               \n first line of the configuration file.  Tone dialing\
               \n uses Touch Tone frequencies to dial, pulse dialing\
               \n simulates a rotary dial telephone and manual dialing\
               \n means that the user will make the connection outside\
               \n of the program. \
               \n If you wish to enter additional commands to configure\
               \n the modem before dialing ACTS, enter A here.  You may\
               \n have to edit the configuration file after MAKCFG\
               \n finishes. See file NISTIME.DES for additional\
               \n format information.";
        help2="\n Enter n <return> here if you are directly connected\
               \n to an outside line.  If you need to dial a digit to\
               \n be connected to an outside line, enter it here.\
               \n Typical responses are 9 or 8.";
        help3="\n If you are in the Denver Metro calling area, then\
               \n no long distance access is required so answer n.\
               \n If you are outside of Denver Metro but in area 303\
               \n then usually only the digit 1 is required.\
               \n If you are outside of area 303 then some systems\
               \n require 1 303, while others require only 303.\
               \n If your system is none of the above, respond with\
               \n o and you will be prompted for the required string.";
        help4="\n If a billing number or other code is required AFTER\
               \n the number is dialed, enter it here, terminated with\
               \n <return>. Use a comma in the string if a 2-second\
               \n pause if necessary.  If billing number is not required\
               \n just enter <return>.  The string is not checked.";
        help5="\n Enter the port that will be used to connect the\
               \n computer to the external modem.  Only COM1 or COM2\
               \n may be used if input/output via BIOS calls was\
               \n selected above.  If direct i/o was selected, COM3 or\
               \n COM4 may also be used.  If an internal modem is used,\
               \n it must be configured as the selected COM port.  If\
               \n direct i/o was selected you may specify the port using\
               \n its hardware address, and you may have to do so if you\
               \n plan to use COM3 or COM4 and if the hardware address\
               \n of the port is not stored in the equipment table in\
               \n memory.  Typical responses are 3f8, 2f8, 3e8 and 2e8\
               \n for ports 1 - 4 respectively, but your response will\
               \n depend on your hardware configuration.  The addresses\
               \n can usually be found in your hardware manuals.";
        help6="\n If the modem echoes commands followed by OK when it\
               \n is in command mode, then respond E to this query.\
               \n If the modem does not echo the commands then respond\
               \n B (for blind dialing).  This capability is usually set\
               \n by means of switches inside the modem.  If you enter\
               \n E and the modem in fact does not respond as expected\
               \n then the program will halt with a message that the\
               \n modem did not respond.  If you enter B then the\
               \n program will wait for a response, but will simply\
               \n continue if no response is received within about\
               \n 3 seconds.  If you are in doubt, you should use\
               \n B, but this will bypass some of the error detection\
               \n capabilities of the program.";
        help7="\n The program converts from Universal Time as received\
               \n from NIST to the local time zone of the user using\
               \n this answer. The conversion is disabled for z, and\
               \n the computer will be set to Universal (Greenwich)\
               \n time.  For non-US time zones, the correction may be\
               \n specified numerically by first responding n and then\
               \n giving the difference (in hours) between local time\
               \n and Universal (Greenwich) Time.  Locations West of\
               \n Greenwich will have negative offsets, while those\
               \n East of Greenwich will be positive.  You may also\
               \n specify your time zone by letter.  To do so, respond\
               \n with a * here.";
        help8="\n The program will set the computer to Daylight\
               \n Savings Time based on the Daylight Savings Time\
               \n flag from the received message and your answer to\
               \n this question.  If you answer y(es), the flag in the\
               \n received message will be tested and the conversion\
               \n will be applied if necessary.  If you answer n(o)\
               \n the received flag will be ignored and only the time\
               \n zone correction will be applied.";
        help9="\n The program will always set or check the computer\
               \n clock stored in memory and read by the 'time'\
               \n and 'date' commands.  If you have an AT-type machine\
               \n or equivalent, it may also have a second clock that\
               \n runs from a battery even when the main power is off.\
               \n If you answer y(es), the program will also set or check\
               \n this clock.  If you answer n(o), the battery clock will\
               \n not be set.  Since the time will only be set in the\
               \n clock that is stored in the volatile computer memory\
               \n in this case, the time will be lost when the power\
               \n is turned off.";
        helpa="\n You can select transmission at 300 bits/s using 103-type\
               \n modems or 1200 bits/s using 212A modems.  The lower speed\
               \n does not provide the Modified Julian Day (MJD) and sends\
               \n the time string only on the even second.  It is less\
               \n affected by modem delays than the 1200 bit/s service.\
               \n The higher speed service provides the full time and date\
               \n every second with an accuracy that is usually adequate\
               \n for most users.  On an IBM PC, output pulses may be\
               \n selected in either mode.";
        helpb="\n If you answer yes to this question, a negative pulse\
               \n will be sent to the line-printer port every second\
               \n when the on-time character is received from NIST.\
               \n The pulse will appear on pin 1 of the female DB25\
               \n connector used for the line printer.  Pin 25 of the\
               \n same connector is ground.  You must have a parallel\
               \n printer adapter configured as LPT1. The program\
               \n will attempt to ascertain its hardware address\
               \n and will print a diagnostic if it fails to do so.\
               \n You should disconnect the line printer if you intend\
               \n to use this capability. If you answer No, the pulse\
               \n generating logic is disabled.";
        helpc="\n If you answer s (or S) to this question, and if you\
               \n select 1200 bits/s communications, then the time of your\
               \n computer clock will be set to the NIST time (converted\
               \n to local time, if necessary).  If you answwer d (or D)\
               \n the computer clocks will not be set.  The difference\
               \n between NIST time and computer time is displayed in\
               \n both cases.  Note that neither setting nor comparison\
               \n are performed at 300 bits/s, since neither the date nor\
               \n the Daylight Savings Time Flag are transmitted in that\
               \n mode.";
        helpd="\n If you answer yes to this question, the time\
               \n differences between the computer clocks and NIST will be\
               \n written to file NISTIME.DIF.  If the file exists, the\
               \n values will be appended to the end of the file.  If the\
               \n file does not exist, it will be created.  Each line\
               \n begins with the date and time of the comparison.  After\
               \n that comes the offset of the clock stored in memory and\
               \n finally offset of the IBMPC CMOS clock if it exists.  If a\
               \n clock is fast with respect to NIST, its difference is\
               \n positive; if slow it is negative.  The letters d, h, m,\
               \n or s used after each value indicate days, hours,\
               \n minutes or seconds, respectively.  If you answer r\
	       \n then in addition to storing the current time difference\
	       \n as just described, the program estimates the rate offset\
	       \n of the clock by subtracting the current time difference\
	       \n from the last difference read from the file NISTIME.DIF\
	       \n and dividing by the elapsed time in days between the\
	       \n two comparisons.  The rate estimate is always given\
	       \n in units of sec/day. A positive value indicates that\
	       \n the computer clock is fast with respect to NIST.";
        helpe="\n If you answer 1, NISTIME will call the resident BIOS\
               \n routines for input and output operations.  Only COM1\
               \n and COM2 may be specified (a BIOS restriction), but\
               \n the program is isolated from the detailed hardware\
               \n configuration by the BIOS, and is therefore more\
               \n likely to work with third-party hardware.  If you\
               \n answer 0, input and output use direct commands to the\
               \n hardware.  Any of the four COM ports may be used\
               \n and the BIOS restriction that clear-to-send (CTS)\
               \n must be true is removed.  The hardware registers\
               \n must conform to the technical description in the\
               \n IBM technical manuals, and the program may not work\
               \n with third-party systems or with future versions of\
               \n the personal-computer hardware.  The comport may be\
               \n specified either by number as 1, 2, 3, or 4 or by\
               \n its hardware address.";
	helpf="\n The configuration files for the different versions\
	       \n of the program differ, and the answer to this question\
	       \n is used to select the questions you will be asked\
	       \n and the acceptable responses.";	
	helpg="\n For the sun system, the communication port may be\
	       \n specified using either one or two alphanumeric\
		\n characters that are appended to the string /dev/tty\
		\n to produce the device name.  Legal responses are the\
	       \n digits 0 - 9 and the letters a -z or A - Z.  The case\
	       \n of an alphabetic response is significant and must match\
	       \n the requirements of the target system.  If only one\
	       \n character is needed, the second should be entered as -.\
	       \n You may also specify the port by its full path name if\
	       \n the form /dev/ttyxx is not suitable.  In that case,\
	       \n respond - to this question.  The program will then\
		\n ask for the path name of the port to be used.  Your\
		\n response is not parsed or validated and an error\
		\n will only be detected when the program NISTIME is run.";
	helph="\nThis switch specifies how the program will respond to  \
 	       \na call whose status is BUSY or NO ANSWER.  If you enter\
	       \nR, the program will re-try the number up to 6 times, \
	       \nr will retry up to 3 times and m will print a message on \
	       \n the screen before each attempt and retry as many times\
	       \n as you wish.  If you enter n then no retries will be \
	       \n attempted.  The program will exit immediately\
	       \nif you select n and the call does not complete.";
        printf("\nProgram to Generate Configuration File for NISTIME.");
        printf("\nVersion of %s",__DATE__);
        printf("\nAll responses must end with <return>");
        printf("\n Answer ? <return> at any time for more information");
/*
        first see if configuration file already exists
*/
        fname="nistime.cfg";
        if( (iop=fopen(fname,"rt") ) != 0)
        {
        do
        {
        printf("\n\n\n A configuration file named %s already exists.",fname);
        printf("\n If you want to");
        printf("\n Continue and overwite the file, answer C <return>");
        printf("\n Stop and think about it,        answer S <return>");
        printf("\n Rename old file and continue,   answer R <return>");
        printf("\n ans= ");
        cc=getchar();
        switch (cc)
           {
           case '?':
              printf("%s",help0);
              stat=0;
              break;
           case 's':
           case 'S':
              exit();
              break;
           case 'R':
           case 'r':
              if( fclose(iop) !=0 )
                 {
                 printf("\n close before rename failed.\n\n");
                 exit();
                 }
              if( rename("nistime.cfg","config.old") != 0)
                 {
                 printf("\n attempt to rename failed\n\n");
                 exit();
                 }
              stat=1;
              break;
           case 'c':
           case 'C':
              if( fclose(iop) != 0)
                 {
                 printf("\n file close failed.\n\n");
                 exit();
                 }
              stat=1;
              break;
           default:
              printf("\n I don't understand %c",cc);
              stat=0;
           }
        while( (cc=getchar() ) != '\n') ;
        } while (stat == 0);
        }  /* end of if file already existed open code */
/*
        now open it for real
*/
        if( (iop=fopen(fname,"wt") )== 0)
        {
        printf("\n open for write failed.\n\n");
        abort();
        }
/*
	find out what the target machine will be
*/
	do
	{
	printf("\n\n Which machine will you be using? \n Please enter:");
	printf("\n i <return> for an IBMPC (or compatible) machine");
	printf("\n s <return> for a SUN workstation.");
	printf("\n v <return> for a VAX/VMS system.");
	printf("\n r <return> for a PDP-11/RSX11M system.");
	printf("\n ans=");
	cc=getchar();
	switch (cc)
	   {
	   case '?':
		printf("%s",helpf);
		stat=0;
		break;
	   case 'i':
	   case 'I':
		mach=1;
		stat=1;
		break;
	   case 's':
	   case 'S':
		mach=2;
		stat=1;
		break;
	   case 'r':
	   case 'R':
		mach=3;
	   case 'v':
	   case 'V':
		mach=4;
		printf("\n makcfg does not support these versions yet.");
		stat = 0;
		break;
	   default:
		printf("\n I don't understand %c",cc);
		stat=0;
	   }
	while ( (cc=getchar() ) != '\n') ; /*skip newline*/
	} while (stat == 0);
/*
        ask if bios calls will be used. if yes only com1 and com2
        if no then com3 and com4 can also be used and the address
        can be specified as a hardware port number
*/
   if(mach == 1)    /* this question only for IBMPC version */
    {
        do
        {
        printf("\n  Please enter:");
        printf("\n  1 <return> if input/output will use BIOS");
        printf("\n  0 <return> if input/output via direct access");
        printf("\n ans= ");
        cc=getchar();
        switch (cc)
           {
           case '1':
              bios=1;
              stat=1;
              break;
           case '0':
              bios=0;
              stat=1;
              break;
           case '?':
              printf("%s",helpe);
              stat=0;
              break;
           default:
              printf("\n I don't understand %c",cc);
              stat=0;
              break;
           }
        while( (cc = getchar() ) != '\n' )   ; /*skip to newline*/
        } while (stat == 0);
    }
        do
        {
        printf("\n1a.  Dialing Mode.  Please enter:");
        printf("\n       T <return> for tone (push-button) dialing");
        printf("\n       P <return> for pulse (rotary) dialing");
        printf("\n       M <return> for manual dialing");
        printf("\n       A <return> to enter commands that will");
        printf("\n         configure the modem before calling ACTS.");
        printf("\n   ans= ");
        cc=getchar();
        switch (cc)
           {
           case 'a':
           case 'A':
              printf("\nEnter complete modem commands on one line");
              printf("\nand terminate with <return>");
              printf("\nCommand=");
              scanf(" %s",number);
              stat=1;
              break;
           case 't':
           case 'T':
              number[0]='T';
              stat=1;
              break;
           case 'm':
           case 'M':
              strcpy(number,"Manual Dialing");
              k=14;
              stat=1;
              break;
           case 'p':
           case 'P':
              number[0]='P';
              stat=1;
              break;
           case '?':
              printf("%s",help1);
              stat=0;
              break;
           default:
              printf("\n I don't understand %c",cc);
              stat=0;
              break;
           }
        while( (cc=getchar() ) != '\n') ;   /* skip to newline character*/
        } while (stat == 0);
/*
        continue unless manual dialing selected or if
        the full command was entered above
*/
        if( (number[0] == 'T') || (number[0] == 'P') )
        {
        do
        {
        printf("\n\n1b. Outside Line Access ?");
        printf("\n  n <return> if not required");
        printf("\n  otherwise enter digit <return>.");
        printf("\n  ans = ");
        cc=getchar();
        switch (cc)
           {
           case '?':
              printf("%s",help2);
              stat=0;
              break;
           case 'n':
           case 'N':
              number[1]=' ';
              k=2;
              stat=1;
              break;
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
              number[1]=cc;
              number[2]=',';
              number[3]=' ';
              k=4;
              stat=1;
              break;
           default:
              printf("\nI don't understand %c",cc);
              stat=0;
              break;
           }
           while( (cc=getchar() ) != '\n') ;  /*skip to newline character*/
           } while (stat == 0);
        do
        {
        printf("\n\n1c. Long Distance Access, Enter");
        printf("\n   n <return> if not required");
        printf("\n   1 <return> if only 1 is required");
        printf("\n   2 <return> if only 303 area code required");
        printf("\n   3 <return> if 1 and 303 both required");
        printf("\n   o <return> if some other access required");
        printf("\n  ans =  ");
        cc=getchar();
        switch (cc)
           {
           case '?':
              printf("%s",help3);
              stat=0;
              break;
           case 'n':
           case 'N':
              stat=1;
              break;
           case '1':
              number[k]='1';
              k++;
              stat=1;
              break;
           case '3':
              number[k]='1';
              k++;
           case '2':
              number[k]='3';
              k++;
              number[k]='0';
              k++;
              number[k]='3';
              k++;
              stat=1;
              break;
           case 'o':
           case 'O':
              while( (cc=getchar() ) != '\n') ; /* skip <return> from o ans*/
              printf("\n Enter Long Distance Access, end with <return>");
              printf("\n ans=");
              while( (cc=getchar() ) != '\n') number[k++]=cc;
              ungetc('\n',stdin);     /* push this <return> back */
              stat=1;
              break;
           default:
              printf("\n I don't understand %c",cc);
              stat=0;
              break;
           }
        while( (cc=getchar() ) != '\n') ;  /* skip to end of line*/
        } while (stat == 0);
        strcpy(&number[k]," 494 4774");
        k +=9;
        do
        {
        printf("\n\n1d. If Accounting codes are necessary after the");
        printf(" telephone number, Enter \n");
        printf("\n      <return> if not needed");
        printf("\n      or codes <return>");
        printf("\n ans = ");
        gets(buf);
        if( buf[0] == '?')
           {
           printf("%s",help4);
           stat=0;
           }
        else
           {
           if(buf[0] != '\0') number[k++]=',';  /*add 1 pause by default*/
           for(j=0; buf[j] != 0; j++) number[k++]=buf[j];
           stat=1;
           }
        } while (stat == 0);
        }
        number[k]='\0';
        do
        {
        printf("\n\n2a. Which Serial Port will be used? \n    Enter:");
   if(mach == 1)
      {
        printf("\n 1 <return> for COM1");
        printf("\n 2 <return> for COM2");
        if(bios == 0)
           {
           printf("\n 3 <return> for COM3");
           printf("\n 4 <return> for COM4");
           printf("\n h <return> to specify the port by its hardware address");
           }
      }
   if(mach == 2)
      {
	printf("\n a- <return> to specify port /dev/ttya, or");
	printf("\n b- <return> to specify port /dev/ttyb, etc. or");
	printf("\n\n xx <return> to specify port /dev/tty<x><x>");
	printf("\n where xx is a pair of numbers or digits, e.g.");
	printf("\n h2 to specify port /dev/ttyh2, etc. or");
	printf("\n\n -  <return> to specify port later by its path name.");
      }
        printf("\n ans =");
        cc=getchar();
   if(mach == 2)
      {
       switch (cc)
	{
	case '?':
	   printf("%s",helpg);
	   stat=0;
	   break;
	case '-':
	   dv1=cc;
	   while( (cc=getchar() ) != '\n') ; /* skip pending newline*/
	   printf("\n path name of port=");
	   scanf("%s",path);
	   stat=1;
	   break;
	default:
	   dv1=cc;
	   dv2=getchar();
	   if( isalnum(dv1) && ( (dv2 == '-') || isalnum(dv2) ) )
		{
		stat=1;
		break;
		}
	   else
		{
		printf("\n response must be either a letter or a digit");
#ifdef SUN		/* SUN uses \ to escape the " */
		printf("\n followed by a \"-\" or two letters or digits.");
#else			/* PC uses "" to escape the " */
		printf("\n followed by a ""-"" or two letters or digits.");
#endif
		stat=0;
		break;
		}
	  }
      }
   if(mach == 1)
      {
        switch (cc)
           {
           case '?':
              printf("%s",help5);
              stat=0;
              break;
           case '1':
           case '2':
              cmport=cc;
              stat=1;
              break;
           case '3':
           case '4':
              cmport=cc;
              stat=1 - bios;      /* only legal if bios == 0 */
              if(stat == 0) printf("\n COM%c not supported by BIOS",cc);
              break;
           case 'h':
           case 'H':
              cmport='h';
              stat=1 - bios;      /* only legal if bios == 0 */
              if(stat == 1)
              {
                 while( (cc=getchar() ) != '\n'); /* skip pending newline*/
                 printf("\n Hardware address of port=");
                 for(j=0; j<3; j++)
                 {
                    cc=getchar();
                    if(isxdigit(cc) != 0) cmadr[j]=cc;
                    else
                    {
                       printf("\n address must have only hex digits.");
                       stat=0;
                       break;
                    }
                 }
              cmadr[3]='\0';
              }
              else
                 printf("\n cannot use hardware address in BIOS mode");
              break;
           default:
              printf("\n I don't understand %c",cc);
              stat=0;
              break;
           }
      }
        while( (cc=getchar() ) != '\n') ; /*skip rest of line */
        } while (stat == 0);
/*
	echo/blind question does not matter if manual dialing
	was selected
*/
        if( (number[0] == 'm') || (number[0] == 'M') )
           {
           echo='b';
           goto skipeb;
        }
        do
        {
        printf("\n2b. Specify Modem Responses to Commands, Enter");
        printf("\n E if modem echoes commands\n B if it does not");
        printf("\n ans =");
        cc=getchar();
        switch (cc)
           {
           case '?':
              printf("%s",help6);
              stat=0;
              break;
           case 'e':
           case 'E':
              echo='e';
              stat=1;
              break;
           case 'b':
           case 'B':
              echo='b';
              stat=1;
              break;
           default:
              printf("\n I don't understand %c",cc);
              stat=0;
              break;
           }
        while( (cc=getchar() ) != '\n') ;
        } while (stat == 0);
skipeb:			/*echo/blind don't matter for manual dialing*/
        do
        {
        printf("\n\n2c. Specify Transmission Speed and protocol");
        printf("\n\n    Enter");
        printf("\n\n    h <return> for 1200 bits/s, 212A Modem");
        printf("  \n    l <return> for  300 bits/s, 103  Modem");
        printf("  \n ans= ");
        cc=getchar();
        switch (cc)
           {
           case '?':
              printf("%s",helpa);
              stat=0;
              break;
           case 'h':
           case 'H':
           case 'l':
           case 'L':
              hs=cc;
              stat=1;
              break;
           default:
              printf("\n I don't understand %c",cc);
              stat=0;
           }
        while( (cc=getchar() ) != '\n'); /* skip to newline */
        } while (stat == 0);
   if(mach == 1)    /* only IBMPC version can pulse line printer port*/
     {
        do
        {
        printf("\n\n2d. Pulse Line-Printer Port Each Second?");
        printf("\n\n    Enter");
        printf("\n\n    y <return> for yes");
        printf("  \n    n <return> for no");
        printf("\n\n    ans= ");
        cc=getchar();
        switch (cc)
           {
           case '?':
             printf("%s",helpb);
             stat=0;
             break;
           case 'y':
           case 'Y':
           case 'n':
           case 'N':
              lpt=cc;
              stat=1;
              break;
           default:
              printf("\n I don't understand %c",cc);
              stat=0;
              break;
           }
        while( (cc=getchar() ) != '\n') ;
        } while (stat == 0);
     }
        do
        {
        printf("\n\n2e. Set computer clocks to received time?");
        printf("  \n     Enter");
        printf("  \n     s <return> for set enable");
        printf("  \n     d <return> for set disable");
        printf("  \n     ans= ");
        cc=getchar();
        switch (cc)
           {
           case '?':
              printf("%s",helpc);
              stat=0;
              break;
           case 's':
           case 'S':
           case 'd':
           case 'D':
              setclk=cc;
              stat=1;
              break;
           default:
              printf("\nI don't understand %c",cc);
              stat=0;
              break;
           }
        while( (cc=getchar()) != '\n')  ;
        }  while (stat == 0);
        do
        {
        printf("\n\n2f. Write Time Differences to file NISTIME.DIF ?");
        printf("  \n    Enter");
        printf("  \n    y <return> for yes");
        printf("  \n    n <return> for no");
	printf("  \n    r <return> for yes and estimate rate too");
        printf("\n\n    ans= ");
        cc=getchar();
        switch (cc)
           {
           case '?':
              printf("%s",helpd);
              stat=0;
              break;
           case 'y':
           case 'Y':
              wrtdif='a';
              stat=1;
              break;
           case 'n':
           case 'N':
              wrtdif='n';
              stat=1;
              break;
	   case 'r':
	   case 'R':
	      wrtdif='A';
	      stat=1;
	      break;
           default:
              printf("\nI don't understand %c",cc);
              stat=0;
              break;
           }
        while( (cc=getchar() ) != '\n') ;
        } while (stat == 0);
	do
	{
	printf("\n\n 2g. Re-dial if Busy or No Answer ?");
	printf("  \n     Enter");
	printf("  \n     R <return> to re-try up to 6 times");
	printf("  \n     r <return> to re-try up to 3 times");
	printf("  \n     m <return> to manually select each re-try");
	printf("  \n     n <return> to disable retries.");
	printf("  \n     ans= ");
	cc=getchar();
	switch (cc)
	   {
	   case '?':
	    printf("%s",helph);
	    stat=0;
	    break;
	   case 'r':
	   case 'R':
	   case 'M':
	   case 'm':
	   case 'n':
	   case 'N':
	     retry=cc;
	     stat=1;
	     break;
	   default:
	     printf("\nI don't understand %c",cc);
	     stat=0;
	     break;
	}		/* end of switch statement*/
	while( (cc=getchar() ) != '\n');   /* skip to end of line */
	} while (stat == 0);
   if(mach == 1)
     {
        do
        {
        printf("\n3. Specify time zone for conversion to local time:");
        printf("\n e <return> for U. S. Eastern Time");
        printf("\n c <return> for U. S. Central Time");
        printf("\n m <return> for U. S. Mountain Time");
        printf("\n p <return> for U. S. Pacific Time");
        printf("\n z <return> for Universal (Greenwich) Time");
        printf("\n n <return> to enter time offset numerically");
        printf("\n * <return> to enter time zone alphabetically");
        printf("\n ans= ");
        cc=getchar();
        switch (cc)
           {
           case '?':
              printf("%s",help7);
              stat=0;
              break;
           case 'e':
           case 'E':
           case 'c':
           case 'C':
           case 'm':
           case 'M':
           case 'p':
           case 'P':
           case 'z':
           case 'Z':
              utcdif[0]=cc;
              utcdif[1]='\0';
              stat=1;
              break;
           case '*':
              while( (cc=getchar() ) != '\n');  /* discard pending newline*/
              printf("\n Time Zone letter (Any except j) =");
/*
        read character, convert to upper case
        then signal error if J or non-letter
*/
              cc=toupper( getchar() );
              if( (cc == 'J') || (isalpha(cc) == 0) )
                {
                printf("\n Response must be any letter except j.");
                stat=0;
                break;
                }
/*
        remove ASCII offset of letters
        we now have A=1, B=2, etc.
*/
              k= (int) cc - 64;
/*
        since j is not used, k, l, m have one
        value too high
*/
              if( (k == 11) || (k == 12) || (k == 13) ) k--;
/*
        if letter was n or later, then value is negative
*/
              if(k >= 14) k= 13 - k;
/*
        special case for z, which is 0
*/
              if(k == -13) k=0;
              sprintf(&utcdif[0],"%+02.2i",k);
              stat=1;
              break;
           case 'n':
           case 'N':
              while( (cc=getchar() ) != '\n') ; /* discard pending newline*/
              printf("\n local time - Universal time in hours=");
              j=0;
              do
                 {
                 cc=getchar();
                 if( (cc != '\n') && (j< 3) )utcdif[j++]=cc;
                 else  utcdif[j]='\0';
                 } while(cc != '\n');
              stat=1;
              ungetc('\n',stdin);
              for(j=0; (cc=utcdif[j]) != 0 ; j++)
              if( (isdigit(cc) == 0) && (cc != '+') && (cc != '-') )
                 {
                 printf("\n response must be a numerical value");
                 stat=0;
                 break;
                 }
              break;
           default:
              printf("\n I don't understand %c",cc);
              stat=0;
              break;
           }
        while( (cc=getchar() ) != '\n') ;  /* skip to new line char */
        } while (stat == 0);
        do
        {
        printf("\n\n4. Test for Daylight Savings Time? \n    Enter");
        printf("  \n   y <return> for yes");
        printf("  \n   n <return> for no ");
        printf("\n   ans = ");
        cc=getchar();
        switch (cc)
           {
           case '?':
              printf("%s",help8);
              stat=0;
              break;
           case 'y':
           case 'Y':
              dsflag=1;
              stat=1;
              break;
           case 'n':
           case 'N':
              dsflag=0;
              stat=1;
              break;
           default:
              printf("\n I don't understand %c",cc);
              stat=0;
              break;
           }
        while( (cc=getchar() ) != '\n');
        } while (stat == 0);
        do
        {
        printf("\n\n5.  Do you have a CMOS battery clock ?\n   Enter\n");
        printf("  \n    y <return> for yes");
        printf("  \n    n <return> for no");
        printf("  \n   ans= ");
        cc=getchar();
        switch (cc)
           {
           case '?':
              printf("%s",help9);
              stat=0;
              break;
           case 'y':
           case 'Y':
              atflag=1;
              stat=1;
              break;
          case 'n':
          case 'N':
              atflag=0;
              stat=1;
              break;
          default:
              printf("\n I don't understand %c",cc);
              stat=0;
              break;
           }
        while( (cc=getchar() ) !='\n');  /*skip to newline*/
        } while (stat == 0);
    }
	fprintf(iop,"%s\n",number);
	if(mach == 1)fprintf(iop,"%c",cmport);
	if(mach == 2)
	   {
	   fprintf(iop,"%c",dv1);
	   if(isalnum(dv1)) fprintf(iop,"%c",dv2);
	   }
        fprintf(iop,"%c%c",echo,hs);
	if(mach == 1) fprintf(iop,"%c",lpt);
	fprintf(iop,"%c%c%c\n",setclk,wrtdif,retry);
        if(mach == 1)fprintf(iop,"%s\n%1d\n%1d\n",utcdif,dsflag,atflag);
        if( (mach == 1) && (cmport == 'h') ) fprintf(iop,"%s\n",cmadr);
	if( (mach == 2) && (dv1 == '-') )fprintf(iop,"%s\n",path);
        fclose(iop);
}
