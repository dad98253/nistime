/*
	include file sizint.h

	the DOS version of the program assumes that the 
	declaration "int" implies a 16-bit quantity and 
	that a "long int" is 32-bits long.  some work-station
	environments use 32-bit quantities for both "int"
	and "long int" with 16-bit quantities defined as
	"short int". This should not be a problem in most
	cases, since there is usually no harm is using a
	32-bit integer when a 16-bit one would have been
	sufficient.
	The code may not work, however, if the machine uses
	64 bits for a "long int" and 32-bits for an "int"
	(e.g., DEC Alpha).  The long-int declarations will
	then produce 64 bit quantities which will not be
	correct in many cases (mostly when the quantity
	is passed by reference to a system subroutine which
	expects a parameter that is 32 bits long, but possibly
	in other situations as well).
	this problem is addressed using the pseudo type LONG,
	which is defined below so as to produce a 32 bit quantity
	in the target environment.  The pseudo type LONG is
	defined as "long int" if a standard int would be 16 bits
	and as "int" if an unqualified int is 32 bits and a long
	int is 64 bits.


	therefore --
	if SUN is defined and if the machine uses 16 or 32 bits 
	for an "int" and 32 bits for a "long int" then define
	B64 as 0.
	if SUN is defined and if the machine uses 64 bits for a 
	"long int" then define B64 as 1.  The only common machine
	currently in this category is a DEC Alpha-based system.
	if IBMPC is defined then a long int is 32 bits in this 
	environment and B64 is not needed.
	(note that some compilers require that pre-processor 
	directives of this type start in col 1.)
*/
#define B64   1
/*      #define B64   0 */
#ifdef IBMPC
#define LONG long int
#else
#if B64 == 1
#define LONG int
#else
#define LONG long int
#endif
#endif     
