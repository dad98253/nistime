	The  files in this directory may be used to build a program
to access the ACTS system.  

     If you are going to run the program in a Windows 3.1 or Windows 95
environment then you should use program winacts.exe.  The corresponding 
program for a 32-bit environment (the later versions of Win 95, Windows 98
and Win NT) is winacts-32bit.exe.  Both of these programs are ready to run 
as is.  

    The files may be copied (using binary/image mode) to any directory and 
installed using the standard Windows procedure (using the File Manager New 
Program command in Win 3.x or dragging the file to your desktop using the
newer versions).  Note that the 32-bit version also requires the 4 dll files 
in this directory. These are cdrvdl32.dll, cdrvhf32.dll, cdrvxf32.dll and 
commsc32.dll.

   Once it is installed, either program may be run at any time by clicking on 
its icon as with any Windows program.  There are a number of help boxes that 
explain the various options, including selecting the COM port and speed and
whether the program is to dial once or dial repeatedly at an interval
that you can select.  If you add the word "Once" to the command line (without 
the quotes and in any combination of upper- and lower-case letters)
then the program will call the ACTS server once each time it is started, set
the local clock and exit after the clock has been set.  Note that the "Once"
must be added to the command line -- not as a response to the periodic query
dialog. If you have copied the program icon to your desktop, the command line may
be edited by right-clicking on the icon and opening the properties of the
short cut. Click on "HELP | Getting Started" for more information.

     If you are going to run the program in DOS on an IBM-PC (or compatible 
machine), then you should copy files nistimed.exe, nistimeb.exe and makcfg.exe 
to the target directory on your PC.  These files must be copied in binary/image
mode.  After you have coped the files, change to that directory and run makcfg.
If you are going to use directory time on disk c:, for example, then

>c:
>cd \time
>makcfg

This will create a file named nistime.cfg based on your answers to a 
series of questions.  You should rename either nistimed.exe or 
nistimeb.exe as nistime.exe:

rename nistimeb.exe nistime.exe

or

rename nistimed.exe nistime.exe

We suggest using nistimed.exe first if you are in doubt about which one 
to use and if your hardware is fully IBM-PC compatible.  The differences
between these two files are discussed in the file nistime.des. 

You may then run the program by typing:

>nistime

For more detailed information and other installation options, examine or 
print files NISTIME.INT, NISTIME.FMT and NISTIME.DES, which are located in 
this directory.

The complete source code is in this directory if you wish to rebuild the 
program.  First you should edit nistime.h following the comments in
that file.  You can then make both nistime and makcfg using the makefile
in this directory.  You can also use this source code to build a version 
that will run in many standard UNIX environments.  See file nistime.h for
details.  

    The FORTRAN source code for a VAX/VMS/OpenVMS version of this program 
is in directory vax.  This code can also be modified to run in other 
environments as described in the comments in the program text.

Program PCTIME is a second program that can be used to get the time
from the ACTS system in a DOS environment.  It will display time information 
in several formats, including the current time in all of the US time zones 
and in many major US cities.  To use the program, copy file pctime.exe 
in binary/image mode and start the program by typing the command 
pctime <return>.  The capabilities of the program are outlined in 
various menu and help screens; additional information can be found 
in the text file pctime.doc which is also in this directory.
Note that the source code for PCTIME is not available and that the
version in this directory will only work on IBM-PCs and compatibles.

   Although both PCTIME and NISTIMED/B are designed to work in a
pure DOS environment, they may work in a DOS shell of Windows or
another operating system (such as OS/2).  This mode of operation
does not work in all configurations, and you should run them in
"pure" DOS if possible.

Questions or comments may be directed to:

ACTS Software
Time and Frequency Division/847
NIST
325 Broadway
Boulder, Colorado 80303

e-mail:  time@time.nist.gov
