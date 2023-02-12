
/************************************************************************************
 * @file  : shift.c
 *          shift.exe Tool
 * @brief : 
 *  - 2008/12/05 : Attached Header Created.
 *  - 2022/11/20 : rename attachHdr.c to shift.c
 *
 * How to build :
 *     1) Install cygwin
 *     2) command as below:
 *        gcc -o AttachHdr -mno-cygwin AttachHdr.c  or
 *        gcc-3  -o AttachHdr -mno-cygwin AttachHdr.c 
 *        x86_64-w64-mingw32-gcc  -o AttachHdr AttachHdr.c
 *
 * ** NEW compiler : x86_64-w64-mingw32-gcc -o shift shift.c
 *
 * $ x86_64-w64-mingw32-gcc --version
 * x86_64-w64-mingw32-gcc (GCC) 11.3.0
 * Copyright (C) 2021 Free Software Foundation, Inc.
 * This is free software; see the source for copying conditions.	There is NO
 * warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 *
 * How to profiling
 *     1) gcc -pg -g -o AttachHdr -mno-cygwin AttachHdr.c 
 *     2) AttachHdr  -> running and then create file, gmon.out.
 *     3) gprof -p -b AttachHdr.exe
 *     4) result is as below:
 *         Flat profile:
 *
 * @version : 2.0
 * @date	: 2022/11/20 - first for Shift Quality Data Shorting 
 * @author	: Yoon-Hwan Joo (tp.joo@daum.net or toppy_joo@naver.com) 
 *
 ************************************************************************************/


/*--------------------------------------------------------------------------
                     INCLUDE FILES 
---------------------------------------------------------------------------*/

//#define __USE_MINGW_ANSI_STDIO 		1
//#define _UNICODE

#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h> /* for getopt() */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <io.h>
#include <time.h> 
#include <math.h>  
#include <assert.h>	/* assert() */
#include <getopt.h> /* getopt_long() */
#include <memory.h> 
#include <stdarg.h>
#include <direct.h> // mkdir, rename
#include <locale.h>
#include <wchar.h>

//#include <windows.h> /* 2022-11-22 */


#include "common.h" 
#include "feature.h"
#include "shift.h"


#include "hash.c"
#include "mjd.c"
#include "hex2bin.c"
#include "elfinfo.c"
#include "common.c"




/*--------------------------------------------------------------------------
                     MACRO
---------------------------------------------------------------------------*/



void beep (int frequency, int duration)
{
#if 0
#define ESC 	27

	fprintf(stderr,"%c[10;%d]%c[11;%d]\a", ESC, frequency, ESC, duration);
#endif
}


int istextfile(char *txtstr)
{
	while( (NULL != txtstr) && (*txtstr != '.') )
		txtstr++;

	if( 0 == strncmp(txtstr, ".exe", 4) ) return 0; /* no text file */
	if( 0 == strncmp(txtstr, ".EXE", 4) ) return 0; /* no text file */
	if( 0 == strncmp(txtstr, ".com", 4) ) return 0; /* no text file */
	if( 0 == strncmp(txtstr, ".COM", 4) ) return 0; /* no text file */

	return 1; /* text file */
}



void 	FileCounter(char *txt)
{
	struct 	_finddata_t 	root_files;
	//struct 	_finddata_t 	subd_files;
	long 	roothFile; //, subdhFile;
	unsigned int LineCounter = 0, AccLineCounter = 0 ;
	int 	c, subdlen, itmp;
	//unsigned char sub_txt[255];
	char strtmp[255];
	
	/* Find first .c file in current directory */
	if( (roothFile = _findfirst( "*.*", &root_files )) == -1L )
	{
		printf("No files in current directory! \n");
	}
	else
	{

		if( 0 == strncmp(txt,"all", 3) )
		{
			//printf( "Listing of files\n\n" );
			printf( "\nR H S A d FILE                                              DATE %24c SIZE\n", ' ' );
			printf(   "- - - - - ------------------------------------------------  ------------------------ %4c ----\n", ' ' );
		}
		else if( 0 == strncmp(txt,"line", 4) )
		{
			printf( "\nFILENAME                                                 LINES\n");
			printf(   "-------------------------------------------------------  ------\n");
		}
		else 
		{ 
			printf("Check option...\n");
			return; 
		}
	
		/* Find the rest of the .c files */
		do
		{
			if( 0 == strncmp(txt,"all", 3) )
			{
				printf( ( root_files.attrib & _A_RDONLY ) ? "Y " : "- " );
				printf( ( root_files.attrib & _A_SYSTEM ) ? "Y " : "- " );
				printf( ( root_files.attrib & _A_HIDDEN ) ? "Y " : "- " );
				printf( ( root_files.attrib & _A_ARCH )   ? "Y " : "- " );
				printf( ( root_files.attrib & _A_SUBDIR ) ? "Y " : "- " );

				if( !(root_files.attrib & _A_SUBDIR) )
					printf( " %-48s %.24s %9ld", root_files.name, ctime( &( root_files.time_write ) ), root_files.size );
				else
				{
					memset( strtmp, 0x00, sizeof(strtmp) );
					subdlen = (48-5-strlen(root_files.name));
					for(itmp=0; itmp<subdlen; itmp++) strtmp[itmp] = 0x20; /* fill SPACE */
					
					printf( " *[%s]* %s %.24s ", root_files.name, strtmp, ctime( &( root_files.time_write ) ) );
				}
			}
			else if( 0 == strncmp(txt,"line", 4) )
			{
				//printf( " %-32s %.24s ", root_files.name, ctime( &( root_files.time_write ) ) );
				if( !(root_files.attrib & _A_SUBDIR) )
					printf( " %-48s ", root_files.name );
				else
					printf( " *[%s]* ", root_files.name );
			}
			
			
			LineCounter = 0;
			if( !(root_files.attrib & _A_SUBDIR) )
			{
				if( istextfile(root_files.name) )
				{
					if( NULL == (inpfile = fopen( root_files.name, "r")) ) 
					{
						printf("\n[++LineCounter++ERROR++] Can not open input file (%s). \n",root_files.name );
						if(inpfile) { fclose(inpfile); inpfile=NULL; }
						_findclose( roothFile );
						beep(700,100);
						return;
					}
		
					while( EOF != (c=fgetc(inpfile)) )
					{
						if('\n' == c || '\r' == c )
							LineCounter++;
					}
					if(inpfile) { fclose(inpfile); inpfile=NULL; }
					
					AccLineCounter += LineCounter;

					printf( "   %10u\n", LineCounter );
				}
				else
				{
					printf(" No text file \n");
				}
			}
			else
			{
		#if 1
				printf("   -> Folder\n");
		#else
				printf(" Folder. \n"); /// [%s] \n", root_files.name);
	
				subdlen = strlen(root_files.name);

				if( subdlen <= 2 && ( 0==strncmp(root_files.name,".", 1) || 0==strncmp(root_files.name,"..", 2) ) )
				{
					printf(" Folder..\n");
				} 
				else
				{
					LineCounter = 0;
					memset( sub_txt, 0x00, sizeof(sub_txt) );  
					sprintf( sub_txt, "%s\\*.*", root_files.name );

					if( (subdhFile = _findfirst( sub_txt, &subd_files )) == -1L )
					{
						printf( "No files in sub directory! \n" );
					}		

					if( !(subd_files.attrib & _A_SUBDIR) )
					{
						if( istextfile(subd_files.name) )
						{
							if( NULL == (inpfile = fopen( subd_files.name, "r")) ) 
							{
								printf("\n[++LineCounter++ERROR++] Can not open input file (%s). \n", subd_files.name );
								fclose(inpfile);
								_findclose( roothFile );
								_findclose( subdhFile );
								beep(700,100);
								return;
							}
				
							while( EOF != (c=fgetc(inpfile)) )
							{
								if('\n' == c || '\r' == c )
									LineCounter++;
							}
							fclose(inpfile);
							
							AccLineCounter += LineCounter;

							printf( "   %10ld\n", LineCounter );		
						}
						else { printf(">>>\n\n"); }
						_findclose( subdhFile );
					}
				}
		#endif
			}
			
		}while( _findnext( roothFile, &root_files ) == 0 );
		_findclose( roothFile );

		printf( "-------------------------------------------------------  ------\n" );
		printf( " Total Line Counter ===>                             %10u\n", AccLineCounter );


	}
}



/* -----------------------------------------------------------------
 * double   val   : Number to be converted
 * int      round : Number of digits after decimal point
 * int      *dp   : Pointer to decimal-point position
 * int      *sign : Pointer to stored sign indicator
 * -----------------------------------------------------------------
 */
char *commify(double val, char *buf, int round) 
{
	static char *result;
	char *nmr;
	int dp, sign;

	result = buf;

	/* Be sure round-off is positive  */
	if (round < 0)
		round = -round;

	nmr = malloc( COMMA_BUF_SIZE*sizeof(unsigned char) );

	/* Convert number to a string */
	nmr = fcvt(val, round, &dp, &sign); 

	/* Prefix minus sign if negative  */
	if (sign)
		*buf++ = '-';

	/* Check if number is less than 1 */
	if (dp <= 0)
	{
		/* Set dp to max(dp, -round) */
		if (dp < -round)
			dp = -round;

		/* Prefix with "0." */
		*buf++ = '0';
		*buf++ = '.';

		/* Write zeros following decimal  */
		while (dp++)
		{
			/* point */
			*buf++ = '0';
		}
	}
	else 
	{
		/* Number is >= 1, commify it */
		while (dp--)
		{
			*buf++ = *nmr++;
			if (dp % 3 == 0)
			{
				/// *buf++ = dp ? ',' : '.';
				*buf++ = dp ? ',' : (round ? '.' : 0);
			}
		}
	}

	/* Append rest of digits */
	strcpy(buf, nmr);
	
	if(nmr) free(nmr);
	return result;
}



void help_brief(void)
{
#if SHIFT_QUALITY_DATA_SORTING /* 2023-01-24 */
	fprintf(stderr,"---------------------------------------------------------------\n" );
	fprintf(stderr," shift.exe %s by TOP.JOO (%s)/ %s \n", AttVersion, EmailText, __DATE__ );
	fprintf(stderr,"---------------------------------------------------------------\n" );
	
#else

	fprintf(stderr,"---------------------------------------------------------------\n" );
	fprintf(stderr," ah.exe %s by TOP.JOO (%s)/ %s \n", AttVersion, EmailText, __DATE__ );
	fprintf(stderr,"---------------------------------------------------------------\n" );
#endif
}


void help(void)
{
#if SHIFT_QUALITY_DATA_SORTING /* 2022-11-13 */

	printf("\nUsage: shift.exe [Version %s, Build:%s] by TOP.JOO. \r\n"

#else

    printf("\nUsage: ah.exe [Version %s, Build:%s] by TOP.JOO. \r\n"
           "--[ Helps ]----------------------------- -------------------------------------------------------------------------\n"
           "  -h or --help [intel|motorola]          intel    : Intel hex format help.  \n"
           "                                         motorola : Motorola hex format help. \n"
           " Ex) ah.exe --help motorola \n"
           "     ah.exe -h intel \n"
           "\n"
           "--[ Input / Output filename ]----------- -------------------------------------------------------------------------\n"
		   "  -i or --input [FILE]                   Input file. \n"
		   "  -o or --output [FILE]                  Output file. (new create) \n"
		   "  -a or --append [FILE]                  Output file. (add to the specific file) \n"
		   "\n"
		   "--[ Detach Header ]--------------------- -------------------------------------------------------------------------\n"
		   "  -d or --detach [byte number(in decimal)]  \n"
		   "        --detach 0|default               Delete the default size (16Bytes X 4Lines) Bytes. \n"
		   "        --detach 54                      Delete the specific bytes number, 54 Bytes in decimal. \n"
		   "        --detach 2048 | 16384 | 1048576  Delete the specific bytes number, 2048/ 16384/ 1048576 Bytes. in decimal.\n"
		   "        --detach 2kB | 16kB | 1MB        Delete the specific bytes number, 2kB, 16kB, 1MB. \n"
		   "\n"
		   " Ex) ah.exe -i LAN1234_nk.LGE -o nk.bin -d 0 \n"
		   "     ah.exe --input LAN1234_nk.LGE --output nk.bin --detach 128 \n"
		   "     ah.exe --input LAN1234_nk.LGE --output nk.bin --detach 2kB \n"
           "\n"
		   "--[ Attach Header and Order ]----------- -------------------------------------------------------------------------\n"
		   "  -b or --board [CPU/PCB NAME]           CPU/PCB/Module Name (Max. 16 bytes)\n"	 /* PI2000, Max 16 Characters */
		   "  -m or --model [MODEL NAME]             Model Name (Max. 16 bytes)\n"	 /* GMPT43, Max 16 Characters  */
           "  -v or --version [VERSION NAME]         Version Name (Max. 16 bytes) or \n"
           "  -p or --mcu [VERSION NAME]             MCU Version Header (Max 32bytes) \n" /* Building Date, Max 128 Characters  */
		   "  -c or --cinfo date                     Building Date/Time(size is 16bytes)\n" /* Building Date, Max 128 Characters  */
		   "        --cinfo crc16|crc16c|crc32       Insert checksum-lowercase in front of output file. (Max. 16Bytes) \n"  
		   "        --cinfo crc16ksc                 Insert checksum-lowercase in front of output file. (Max. 16Bytes) \n"  
		   "        --cinfo crc64|adler32|joaat      Insert checksum-lowercase in front of output file. (Max. 16Bytes) \n"  
		   "        --cinfo CRC16|CRC16C|CRC32       Insert checksum-uppercase in front of output file. (Max. 16Bytes) \n"  
		   "        --cinfo CRC64|ADLER32            Insert checksum-uppercase in front of output file. (Max. 16Bytes) \n"  
		   "        --cinfo CRC64ISC                 Insert CRC64 ISC(Internet Systems Consortium) (Max. 16Bytes) \n"  
		   "        --cinfo sha224|SHA224            Insert SHA2 (lower/uppercase) in front of output file. (Size is 56Bytes)\n"
		   "        --cinfo md5|md6                  Insert Message Digest5/6 in front of output file. (Size is 32/64Bytes)\n"  
		   "        --cinfo md2|md4                  Insert Message Digest2/4 in front of output file. (Size is 32/32Bytes)\n"  
           "        --cinfo sha1                     Insert SHA1 in front of output file. (Size is 40Bytes)\n"  
           "        --cinfo sha224|sha256            Insert SHA2 in front of output file. (Size is 56/64Bytes)\n"	
           "        --cinfo sha384|sha512            Insert SHA2 in front of output file. (Size is 96/128Bytes)\n" 
           "        --cinfo sha3-224|SHA3-256        Insert SHA3 in front of output file. (Size is 56/64Bytes)\n" 
           "        --cinfo sha3-384|SHA3-512        Insert SHA3 in front of output file. (Size is 96/128Bytes)\n" 
           "        --cinfo shake128|SHAKE256        Insert Shake in front of output file. (Size is 64/128Bytes)\n" 
           "        --cinfo RMD128|RMD160            Insert RipeMD128/RipeMD160in front of output file. (Size is 32/40Bytes)\n" 
           "\n"
           " Ex) ah.exe -b AU1354 -m LAN1234 -c date  -i nk.bin -o LAN1234_nk.JOO --version 5.0.00(13A) \n"
           "     ah.exe -b AU1354 -m LAN1234 --cinfo crc16  -i nk.bin -o LAN1234_nk.JOO -v 5.0.00(13A) \n"
           "     ah.exe -b AU1354 -m LAN1234 --cinfo crc64  --input nk.bin --output LAN1234_nk.LGE -v 5.0.00(13A) \n"
           "     ah.exe --board AU1354 -v DH12-01 -i nk.bin -o LAN1234_nk.LGE \n"
           "     ah.exe --cinfo sha1  --input nk.bin --output LAN1234_nk.LGE  \n"
           "     ah.exe --version BLTMCAM1.0 --input nk.bin --output LAN1234_nk.LGE  \n"
           "     ah.exe --mcu 5.0.00(13E) --cinfo sha256 -i nk.bin -o LAN1234_nk.LGE  \n"
           "     ah.exe --model LAN1234 --mcu 5.0.00(13E) --cinfo sha256 -i nk.bin -o LAN1234_nk.LGE  \n"
           "     ah.exe -p M-LGCNS-K8HXX -i aig300lgcns.bin -o mcu.dat \n"
           "     ah.exe --mcu M-LGCNS-K8HXX -i aig300lgcns.bin -o mcu.dat \n"
           "\n"
		   "--[ Convert Float number to Hexa ]------ -------------------------------------------------------------------------\n"
		   "  -g both -f or --ignore both --float [0.8543]     -g both -f 0.866 \n" 
		   "  -g both -f or --ignore both --float [sin|cos|tan 60]  -g both -f sin 60 \n" 
		   "  --ignore both --float [float number]        --ignore both --float 0.866 \n" 
		   "  --ignore both --float [sin|cos|tan 60]      --ignore both --float sin 60 \n" 
		   "  -i -o -f file access                   -i [input] -o [output] -f f2h \n" 
		   "  --input --output --float file access   --input [in-file] --output [out-file] --float f2h \n" 
		   " Ex) ah.exe -g both --float 0.8543 \n"
		   "     ah.exe -g both --float sin 60 \n"
		   "     ah.exe --input ff.dat --output ff.txt --float f2h \n"
		   "\n"
           "--[ Hex2Bin ]--------------------------- -------------------------------------------------------------------------\n"
		   "  -L or --intel                          Convert INTEL hexa format to Bin. (Max Size:%u MB) \n"
		   "  -A or --motorola                       Convert MOTOROLA hexa format to Bin. (Max Size:%u MB) \n"
		   "                                           . S19-style (16-bit) addressing format. \n"
		   "                                           . S28-style (24-bit) addressing format. \n"
		   "                                           . S37-style (32-bit) addressing format. \n"
		   "                                           . checksum \n"
           "  -S or --startaddr                      Starting address in hexa value. (with -A or -L option) \n"
	#if 0
           "  -k or --crctype [0|1|2|3|4|5]          Select checksum type (default:Checksum 8-bit) \n"
           "                                           0: Checksum 8-bit \n"
           "                                           1: Checksum 16-bit (adds 16-bit words into a 16-bit sum, data and result BE or LE)\n"
           "                                           2: CRC8 \n"
           "                                           3: CRC16 \n"
           "                                           4: CRC32 \n"
           "                                           5: Checksum 16-bit (adds bytes into a 16-bit sum, result BE or LE) \n"
	#endif
           "  -l or --length [1000]                  0x1000 in hexa value for bin's file length . (ex: 8000 for 0x8000)\n"
           "        --length [a0000]                 0xa0000 in hexa value for bin's file length 640kByte \n"
           "        --length [640kB]                 For bin's file length 640kBytes \n"
           "        --length [1MB]                   For bin's file length 1MBytes \n"
           "  -n or --alignword                      Address alignment word (--intel option only) \n"
           "  -E or --endian [little|big]            Endian for checksum/CRC, 0: little, 1: big \n"
           "  -P or --padbyte [hexa-value]           Pad-byte value in hex, example -P ee (default: 0xff)\n"
           "  -k or --allpadarea                     Padding byte for ALL empty area. --padbyte ab option \n"
           "  -Z or --zeroforced                     Because the real address of hexa file is very large, so \n"
           "                                           it is created binary a based on zero address.\n"
           "\n"
           " Ex) ah.exe -i [hexa-file] -o [out bin file] -A or -L \n"
           "     ah.exe -i [hexa-file] -o [out bin file] -L --length 1000 \n"
           "     ah.exe -i [hexa-file] -o [out bin file] --intel \n"
           "     ah.exe --input [hexa-file] --output [out bin file] -A or -L -S A00 \n"
           "     ah.exe --input romp.hex -o BIN\\ROMP.bin --intel --startaddr A00 \n"
           "     ah.exe --input romp.hex --output BIN\\ROMP.bin --intel --length A0000 \n"
           "     ah.exe --input romp.hex --output BIN\\ROMP.bin --motorola \n"
           "     ah.exe --input romp.hex --output BIN\\ROMP.bin --motorola \n"
           "     ah.exe --input romp.hex --output BIN\\ROMP.bin --motorola --length 10000 \n"
           "     ah.exe --input romp.hex --output BIN\\ROMP.bin --motorola --length 10000 --padbyte ab \n"
           "\n"
           "--[ WinCE Record ]---------------------- -------------------------------------------------------------------------\n"
           "  -N or --nk [nb0|none]                   NK.BIN (WinCE Kernel) Record information \n"
           " Ex) ah.exe -i nk.bin -o NK.LOG -N none \n"
           "     ah.exe --input nk.bin --output NK.LOG -nk nb0 \n"
           "\n"
           "--[ Files Merge/Join ]------------------ -------------------------------------------------------------------------\n"
           "  -j or --join [hexa value]               Specify the 1st file's total size in hexa. \n"
           "        --join [640kB/320kB]              Specify the 1st file's total size 640kB/320kB in size unit no-space. \n"
           "        --join [1MB/2MB]                  Specify the 1st file's total size 1MB/2MB in size unit no-space. \n"
           "  -E or --endian [little|big]             Calculate the 2nd file size in hexa (4byte) \n"
           "  -P or --padbyte [hexa value]            Pad byte in hexa (1byte). default is 0xFF \n"
           "\n"
           " Ex) ah.exe -j 3000       -i [1st-file] [2nd-file] -o [merged output file] \n"
           "     ah.exe --join 0x8000 -i [1st-file] [2nd-file] -o [merged output file] \n"
           "     ah.exe --join 2800   -i STM_mcu_Boot.bin  STM_mcu_302K8_App.bin -o  STM_mcu_TotalQD21.bin --padbyte ab \n"
           "     ah.exe --join 10kB   -i STM_mcu_Boot.bin  STM_mcu_302K8_App.bin -o  STM_mcu_TotalQD21.bin --padbyte ab \n"
           "     ah.exe --join 0x2800 -E little -i STM_mcu_Boot.bin	STM_mcu_302K8_App.bin -o STM_mcu_TotalQD21.bin \n"
           "     ah.exe --join 2800   --endian little -i STM_mcu_Boot.bin  STM_mcu_302K8_App.bin -o STM_mcu_TotalQD21.bin \n"
           "     ah.exe --join 0x2800 --endian big    -i STM_mcu_Boot.bin  STM_mcu_302K8_App.bin -o STM_mcu_TotalQD21.bin \n"
           "\n"
           "--[ Files Encapsulation/Extract ]------- -------------------------------------------------------------------------\n"
           "  -y or --merge [1st file 2nd file .....] Specify the file lists to be merging to 1 file. Max count is 100 files.\n"
           "  -x or --extract [filename]              Specify the merged file name (upto 100 files). \n"
           "\n"
           " Ex) ah.exe --merge a100.bin a200.bin b300.bin c400.bin c600.bin --model BLTNCAM --version CI02-02 --output total.bin \n"
           "     ah.exe --model BLTNCAM --version CI02-02 --merge a100.bin a200.bin b300.bin c400.bin c600.bin --output total.bin \n"
           "     ah.exe --merge a100.bin a200.bin b300.bin c400.bin c600.bin --version CI02-02 --output total.bin \n"
           "     ah.exe --merge a100.bin a200.bin b300.bin c400.bin c600.bin --output total.bin \n"
           "\n"
           "     ah.exe --extract total.bin \n"
           "     ah.exe -x total.bin \n"
           "\n"
	#if MODIFIED_JULIAN_DATE 
           "--[ MJD (Modified Julian Date) ]-------- -------------------------------------------------------------------------\n"
           "  -J or --mjd test                        Modified JD test and create test-file.\n" 
           "        --mjd current                     Convert the current System Time to MJD \n" 
           "        --mjd mjd                         Convert input DATE file(-i) to MJD \n" 
           "        --mjd date                        Convert input MJD file(-i) to DATE  \n" 
           "\n"
           " Ex) ah.exe --mjd test \n"
           "     ah.exe --mjd current \n"
           "     ah.exe --input number.mjd --output out.txt --mjd date \n"
           "     ah.exe --input date.mjd   --append out.txt --mjd mjd \n"
           "\n"
	#endif

           "--[ Others ]---------------------------- -------------------------------------------------------------------------\n"
           "  -I or --fileinform [all|line]           Searching all files. -I all , -I lines \n"
           "  Ex) ah.exe -I all \n"
           "\n"
           "  -F or --fillsize [size in hexa]         Fill Pad Byte. (default:0xFF). The fillsize means total file length. \n"
           "        --fillsize [a0000]                hexa value 0xa0000 for length 640kByte \n"
           "        --fillsize [640kB]                For total length 640kBytes \n"
           "        --fillsize [1MB]                  For example, output file total length is 1MBytes \n"
           "  -z or --verbose 0|1|2|3|4               display message\n"
           "        --verbose date|size|datesize      display message\n"
           "\n"
           " Ex) ah.exe --input STM32.bin --output mcuTOT_STM32.bin --fillsize a0000  \n"
           "     ah.exe --input STM32.bin --output mcuTOT_STM32.bin --fillsize 640kB --padbyte CC  \n"
           "\n"
           "--[ Message Digest / SHA / Checksum ]--- -------------------------------------------------------------------------\n"
           "  -M or --checksum [crc16|crc16c|crc16ksc Generate CRC16/CRC16CCITT/KSC-CRC16  \n" 
           "                    crc32|adler32|joaat            CRC32/ADLER32  \n"
           "                    crc64|crc64isc                 CRC64/CRC64 ISC(Internet Systems Consortium)  \n"
           "        --checksum [MD2|MD4|MD5|MD6]      Generate MD2 (32Bytes) Message Digest Algorithm-2 \n" 
           "                                                   MD4 (32Bytes) Message Digest Algorithm-4 \n" 
           "                                                   MD5 (32Bytes) Message Digest Algorithm-5 \n" 
           "                                                   MD6 (64Bytes) Message Digest Algorithm-6 \n" 
           "        --checksum [SHA1|SHA224|          Generate SHA1 (40Bytes) Secure Hash Algorithm \n" 
           "                    SHA256|SHA384|SHA512]          SHA2-224 (56Bytes) Secure Hash Algorithm \n" 
           "                                                   SHA2-256 (64Bytes) Secure Hash Algorithm \n" 
           "                                                   SHA2-384 (96Bytes) Secure Hash Algorithm \n" 
           "                                                   SHA2-512 (128Bytes) Secure Hash Algorithm \n" 
           "        --checksum [SHA3-224|SHA3-256|    Generate SHA3-Keccak-224 (56Bytes) Secure Hash Algorithm \n" 
           "                    SHA3-384|SHA3-512|             SHA3-Keccak-256 (64Bytes) Secure Hash Algorithm \n" 
           "                    SHAKE128|SHAKE256]             SHA3-Keccak-384 (96Bytes) Secure Hash Algorithm \n" 
           "                                                   SHA3-Keccak-512 (128Bytes) Secure Hash Algorithm \n" 
           "                                                   SHAKE128 (64Bytes) Secure Hash Algorithm \n" 
           "                                                   SHAKE256 (128Bytes) Secure Hash Algorithm \n" 
           "        --checksum [RMD128|RMD160]        Generate RipeMD128/RipeMD160 (32/40Bytes) Hash Algorithm \n" 
	#if MD5_MULTI_INPUT_FILES 
           " \n"
           " Ex) ah.exe -i @.zip         -a MD5sum.txt        --checksum MD5 --verbose datesize \n"
           "     ah.exe -i $.zip         -o SHA384sum.txt     -M sha224      -z size \n"
           "     ah.exe --input @.zip    --output SHA2sum.txt -M SHA512      --verbose date \n"
           "     ah.exe --input @.@      --append CRCsum.txt  --checksum CRC16C \n"
           "     ah.exe --input @.zip    --append SHA2sum.txt --checksum SHA256 --verbose date \n"
           "     ah.exe --input Mem@.zip --append SHA1sum.txt --checksum SHA1   --verbose datesize \n"
           "     ah.exe --input @.zip    --checksum sha256 \n"
           "     ah.exe --input $.zip    --checksum rmd160 \n"
    #endif
#endif


	#if SHIFT_QUALITY_DATA_SORTING /* 2022-11-13 */
           " \n"
           "--[ Shift Quality Data Sorting ]-------- -------------------------------------------------------------------------\n"
           "  -U or --upshift [ModeID] [Cont#1] [Jerk#1] [Nt-pos#] [PWR#1] [Tol#2] [APS#1] [APS#2] [APS#3]\n" 
           "             ModeID : ECO, SPT, NOR and you can add option like .last \n"
           "                      ECO.last, SPT.last, nor.last  \n"
           "             Cont#1 : SB/SP point decision continuous count. (default 3 times) \n"
           "             Jerk#1 : Reverse time length at SB point for Jerk1 calcution. (default 200msec) \n"
           "             Nt-pos#: Before or after time position for Nt-Max/Nt-min position (default: +/-50msec) \n"
           "             PWR#1  : APS Power On low level as 3, or 4.5. -> Power On decision level \n"
           "             Tol#2  : APS Power tolerance as +/-1%%, or +/-2%% (default +/-2%%) \n"
           "             APS#1  : APS Power Start value. \n"
           "             APS#2  : APS Power End value. \n"
           "             APS#3  : APS Power Step. \n"
           "\n"
           " Ex) shift.exe --input 5ms_16select.tsv --output 5ms_eco.txt --upshift eco.last 3 200 50 3 1.5 \n"
           "     shift.exe --input 5ms_16select.tsv --output 5ms_spt.txt --upshift spt      4 200 50 3 1.5 \n"
           "     shift.exe --input 5ms_16select.tsv --output 5ms_nor.txt --upshift NOR.g1   4 200 50 3 2  \n"
           "     shift.exe --input 5ms_16select.tsv --output 5ms_nor.txt --upshift spt.last.g2 4 200 50 3.5 2  \n"
           "        -> Sorted result output : 5ms_eco.txt, 5ms_eco.gil and 5ms_eco.rpt \n"
           "\n" 
           "  -D or --downshift [ModeID] [Pos#1] [Cout#1] [Jerk#1] [VS#1] [VS#2] [VS#3] \n" 
           "\n"
           " Ex) shift.exe --input 5ms_16select.tsv --output 5ms_spt.txt --downshift SPT 5 300 50 3 2 160 60 -10 \n"
           "        -> Sorted result output : 5ms_spt.txt, 5ms_spt.gil and 5ms_eco.rpt \n"
           "\n"
           " [ModeID]: HOT WUP MNL DN2 DN1 UP1 UP2 UP3 NOR ECO ECODN2 ECODN1 ECOUP1 ECOUP2 ECOUP3 \n" 
           "           CRZ CRZUP1 CRZUP2 BRK1 BRK2 HUP1 HUP2 HUP3 SPTDN2 SPTDN1 SPT SPTUP1 SPTUP2 \n" 
           "           SPTUP3 SS_XECO SS_ECODN2 SS_ECODN1 SS_ECO SS_ECOUP1 SS_ECOUP2 SS_ECOUP3  \n"
           "           SS_DN2 SS_DN1 SS_NOR SS_UP1 SS_UP2 SS_UP3 SS_SPTDN2 SS_SPTDN1 SS_SPT SS_SPTUP1 \n" 
           "           SS_SPTUP2 SS_SPTUP3 SS_XSPTDN2 SS_XSPTDN1 SS_XSPT SS_XSPTUP1 SS_XSPTUP2  \n" 
           "           SS_XSPTUP3 L4W COLD SNOW SNOWUP1 SNOWUP2 SAND SANDUP1 SANDUP2 MUD MUDUP1 MUDUP2 \n" 
           "           TOWDN2 TOWDN1 TOW TOWUP1 TOWUP2 TOWUP3 \n" 
	#endif
	

	#if CONVERT_BMP2C
           "--[ Convert bmp image to C text ]------- -------------------------------------------------------------------------\n"
           "  -B or --bmp                             Convert BMP file to C text file. \n" 
           "      888                RGB 888 \n" 
           "      444                RGB 444 \n" 
           "      555                RGB 555 \n"
           "      565                RGB 565 \n"
	#endif /// CONVERT_BMP2C
           "------------------------------------------------------------------------------------------------------------------\n", 
           		AttVersion, __DATE__ , (INT2BIN_MEMORY_SIZE>>20), (MOT2BIN_MEMORY_SIZE>>20) );


	#if 1
	//exit(0);
	#else
	printf("Press [ENTER]...\r");
	#endif
}




#if 0
int arg_sum(int num_args, ...)
{
	int arg=0;
	int result=0;
	int i=0;
	
	va_list ap;
	va_start(ap, num_args);

	//for(i=0; i<num_args; i++)
	result = 0;
	for(;;)
	{
		arg = va_arg(ap, int);
		if(0==arg) break;
		result += arg;
	}
	va_end(ap);
	return result;
}
//printf(" ==== %d  ==== ", arg_sum(3,3,3,3,3) );
#endif

#if 0
#include <sys/ioctl.h>
#include <net/if.h>

int ttmain(void)
{
    int sock;
    struct ifreq ifr;
    unsigned char *mac = NULL;

    memset(&ifr, 0x00, sizeof(ifr));
    strcpy(ifr.ifr_name, "eth0");

    int fd=socket(AF_UNIX, SOCK_DGRAM, 0);

    if((sock=socket(AF_UNIX, SOCK_DGRAM, 0))<0){
        perror("socket ");
        return 1;
    }

    if(ioctl(fd,SIOCGIFHWADDR,&ifr)<0){
        perror("ioctl ");
        return 1;
    }

    mac = ifr.ifr_hwaddr.sa_data;

    printf("%s: %02x:%02x:%02x:%02x:%02x:%02x\n", ifr.ifr_name, mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
    close(sock);
    return 0;
}
#endif


int isFileExist(const char *file, int disp)
{
	struct	_finddatai64_t fexist;
	long ret = 0L;
	int  retValue = 0;

	if(0==strcmp(file,"") || NULL==file) return 0;

	if( (ret = _findfirsti64( file, &fexist )) == -1L )
	{
		if(disp) printf(" ------ [ %s ] is Non file ", file);
		retValue = 0;
	}
	else
	{
		if(disp) printf(" FILE : [ %s ] ", file);
		retValue = 1;
	}

	_findclose( ret );

	if(disp) printf(" RETURN VALUE : %d  \n", retValue );
	return retValue;
}


struct tm *current_time(void)
{
	time_t	cur_time;
	struct  tm* pLocalTime;

	time( &cur_time );
    pLocalTime = localtime(&cur_time);

	#if 0
    fprintf(stdout, "+++++ %04d/%02d/%02d %02d:%02d:%02d ++++ \n", 
			pLocalTime->tm_year + 1900, 
			pLocalTime->tm_mon + 1, 
			pLocalTime->tm_mday, 
			pLocalTime->tm_hour, pLocalTime->tm_min, pLocalTime->tm_sec);
	#endif

	pLocalTime->tm_year += 1900;
	pLocalTime->tm_mon  += 1;

	return pLocalTime;
}


void AllFilesClosed(void)
{
	if(inpfile) { fclose(inpfile);	inpfile=NULL; }
	if(outfile) { fclose(outfile);	outfile=NULL; }
	if(data_buf){ free(data_buf);	data_buf=NULL; }
}



#if SHIFT_QUALITY_DATA_SORTING /* 2022-11-13 */

/* -------------------------------------------------------
   [ 변속의 정의] / 2022-12-06
 SS : Shfit Signal (변속 신호)
 SB : Shift Begin (변속 개시점)
 SP : Shift Synchronise (변속 회전수 동기)
 SF : Shift Finish (변속 종료)
 --------------------------------------------------------- */

#define MAX_TABLE_SIZ 			1000 //2000
#define SB_DECISION_TIMES 		3
#define SB_DECISION_MAX_TIMES 	10 /* upto 9 */
#define SB_DECISION_NUM 		(SB_DECISION_MAX_TIMES*2) /* SB decision times buffer */
#define SP_DECISION_NUM 		(SB_DECISION_MAX_TIMES*2) /* SP decision times buffer */

#define NtMax_DECISION_NUM 		(SB_DECISION_MAX_TIMES*2) /* NtMax decision times buffer */
#define Ntmin_DECISION_NUM 		(SB_DECISION_MAX_TIMES*2) /* Ntmin decision times buffer */

#define gMax_Before_SB_POINT 	0 /* 5msec * 1 SB point + 5msec lines number */
#define gmin_Before_SB_POINT 	1 /* 5msec * 1 SB point + 5msec lines number */
#define JERK_TIME_mSec 			200 /* unit: msec, GMax point (msec) before SB point */
#define JERK_min_TIME_mSec 		100 /* unit: msec, min 100 msec */
#define JERK_MAX_TIME_mSec 		2000 /* unit: msec, max 2sec */

#define NtMax_Before_mSec 		50 /* unit: msec, Nt-Max Begin point (msec) before SB point */
#define Ntmin_After_mSec 		50 /* unit: msec, Nt-min Begin point (msec) after SP point */

#define Nt_min_TIME_mSec 		50 /* unit: msec, min 50msec */
#define Nt_MAX_TIME_mSec 		200 /* unit: msec, max 200msec  */


#define JERK_TIME_SCALE 		10 /* just scale */
#define SS2SP_OVER_TIME 		2000UL /* 2000 msec -> so ignored */
#define SP2SF_OVER_TIME 		1000UL /* 1000 msec -> so ignored */

#define JERK_LPFfactor 			(0.102f) /* 10.2% */

#define JERK1_INVALID_VALUE1 	(-10) /* -10G/sec -> invalid */
#define JERK1_INVALID_VALUE2 	(0)   /* 0G/sec -> invalid */
#define JERK1_T_INVALID 		(100) /* t = abs 100msec under -> invalid */

#define MAX_RPT_BUF_SIZ 		(8)
#define GRAPH_Y_BUF_SIZ 		10

#define FUNC_SPC 				14
#define SPACE_ONE 				60
#define SPACE_ONE_UNIT 			10



#define MODE_NOR 		8  /* 8: (md_NOR) */
#define MODE_ECO 		9  /* 9: (md_ECO) */
#define MODE_SPT 		25 /* 25 : (md_SPT) */
#define MODE_UNKOWN 	-1
#define MODE_FILE_SAVE 	999 /* Mode ID?? file saving */

	
#define SHIFT_UP 		11 /* SEQ_UP, sequential up shift */
#define SHIFT_DN 		12 /* SEQ_DN */
#define SHIFT_SKIP_UP 	21 /* SKIP_UP skip up shift */
#define SHIFT_SKIP_DN 	22 /* SKIP_DN */
#define SHIFT_PNDR 		33 /* P/N-D/R : (P-R), (P-D), (N-R), N-D) */


#define SHI_STATIC 		10 /* Static */
#define SHI_PWR_ON 		11 /* Power On */
#define SHI_PWR_OFF 	12 /* Power Off */
#define SHI_N_STOP_DN 	15 /* Near Stop Down Shift */


#define TYPE_MAX_NT 	0x11
#define TYPE_MAX_NE 	0x12
#define TYPE_Min_NT 	0x13
#define TYPE_Min_NE 	0x14
#define TYPE_MAX_G 		0x22
#define TYPE_Min_G 		0x23
#define TYPE_MinLAST_G 	(0x80 | TYPE_Min_G)
#define TYPE_MAXLAST_G 	(0x80 | TYPE_MAX_G)


#define LEN_POS 		15
#define TXT_SSTIME 			"(SS)"  /* Shift Start : Upshift case> tg : 1 -> 2 */
#define TXT_SBTIME 			"(SB)"  /* Shift Begin */
#define TXT_SBSWING0 		"(SB0)" /* SB re-point because of SB level swing */

#define TXT_SPTIME 			"(SP)"  /* Synchronization Point */
#define TXT_SPSWING0 		"(SP0)" /* SP re-point because of SP level swing */

#define TXT_SFTIME 			"(SF)"  /* Shift Finish : Upshift case> cg : 1 -> 2 */


#define TXT_NtMaxTIME 		"(NX)" // "(NMx)"
#define TXT_NtMaxTIMENONE 	"(N*X)" // "(NMx)"

#define TXT_NtMaxSWING0 	"(NMx0)" /* NtMax re-point because of Nt Max level swing */

#define TXT_NtminTIME 		"(Nm)" // "(Nmn)"
#define TXT_NtminTIMENONE 	"(N*m)" // "(Nmn)"
#define TXT_NtminSWING0 	"(Nmn0)" /* Ntmin re-point because of Nt min level swing */

#define TXT_UNKNOWN 		"(***)"
#define TXT_UPCASE 			"(*u*)"
#define TXT_DNCASE 			"(*d*)"
#define TXT_MaxNt 			"(MXNt)"
#define TXT_MaxNe 			"(MXNe)"


#define TXT_SBMXNt 			"(SB)MXNt"
#define TXT_SBMXNe 			"(SB)MXNe"
#define TXT_SBMXgX 			"(SB)gX"
#define TXT_SBmigm 			"(SB)gm"

#define TXT_SBMXNtNe 		"(SB)MXNtNe"
#define TXT_SBMXNtgX 		"(SB)MXNtgX"
#define TXT_SBMXNegX 		"(SB)MXNegX"
#define TXT_SBMXNtgm 		"(SB)MXNtgm"
#define TXT_SBMXNegm 		"(SB)MXNegm"

#define TXT_SB0_MaxNt 		"(SB0)MXNt" 
#define TXT_SB0_MaxNe 		"(SB0)MXNe" 
#define TXT_SB0_MaxgX 		"(SB0)MXgX" 
#define TXT_SB0_Mingm 		"(SB0)migm" 

#define TXT_MXNtgX 			"(MXNtgX)"
#define TXT_MXNtgm 			"(MXNtgm)"
#define TXT_MXNtNe 			"(MXNtNe)"
#define TXT_MXNegX 			"(MXNegX)"
#define TXT_MXNegm 			"(MXNegm)"

#define TXT_MXNtNegX 		"(MXNtNe)gX"
#define TXT_MXNtNegm 		"(MXNtNe)gm"

#define TXT_gMAX 			"(gX)" // "(gMx)"
#define TXT_gMAX_NONE 		"(g*X)" // "(gMx)"

#define TXT_gmin 			"(gm)" // "(gmn)"
#define TXT_gmin_NONE 		"(g*m)" // "(gmn)"



#define TXT_NEW_MaxNt 		"(xNt)"
#define TXT_NEW_MaxNe 		"(xNe)"
#define TXT_NEW_gMAX 		"(gMax)"
#define TXT_NEW_gmin 		"(gmin)"


#define TXT_SB1_MaxNt 		"(SB1)MXNt" 
#define TXT_SB1_MaxNe 		"(SB1)MXNe" 
#define TXT_SB1_MaxgX 		"(SB1)MXgX" 
#define TXT_SB1_Mingm 		"(SB1)migm" 

#define TXT_SB2_MaxNt 		"(SB2)MXNt" 
#define TXT_SB2_MaxNe 		"(SB2)MXNe" 
#define TXT_SB2_MaxgX 		"(SB2)MXgX" 
#define TXT_SB2_Mingm 		"(SB2)migm" 

#define TXT_SB3_MaxNt 		"(SB3)MXNt" 
#define TXT_SB3_MaxNe 		"(SB3)MXNe" 
#define TXT_SB3_MaxgX 		"(SB3)MXgX" 
#define TXT_SB3_Mingm 		"(SB3)migm" 

#define TXT_SB4_MaxNt 		"(SB4)MXNt" 
#define TXT_SB4_MaxNe 		"(SB4)MXNe" 
#define TXT_SB4_MaxgX 		"(SB4)MXgX" 
#define TXT_SB4_Mingm 		"(SB4)migm" 

#define TXT_SB5_MaxNt 		"(SB5)MXNt" 
#define TXT_SB5_MaxNe 		"(SB5)MXNe" 
#define TXT_SB5_MaxgX 		"(SB5)MXgX" 
#define TXT_SB5_Mingm 		"(SB5)migm" 

#define TXT_SB6_MaxNt 		"(SB6)MXNt" 
#define TXT_SB6_MaxNe 		"(SB6)MXNe" 
#define TXT_SB6_MaxgX 		"(SB6)MXgX" 
#define TXT_SB6_Mingm 		"(SB6)migm" 

#define TXT_SB7_MaxNt 		"(SB7)MXNt" 
#define TXT_SB7_MaxNe 		"(SB7)MXNe" 
#define TXT_SB7_MaxgX 		"(SB7)MXgX" 
#define TXT_SB7_Mingm 		"(SB7)migm" 

#define TXT_SB8_MaxNt 		"(SB8)MXNt" 
#define TXT_SB8_MaxNe 		"(SB8)MXNe" 
#define TXT_SB8_MaxgX 		"(SB8)MXgX" 
#define TXT_SB8_Mingm 		"(SB8)migm" 

#define TXT_SB9_MaxNt 		"(SB9)MXNt" 
#define TXT_SB9_MaxNe 		"(SB9)MXNe" 
#define TXT_SB9_MaxgX 		"(SB9)MXgX" 
#define TXT_SB9_Mingm 		"(SB9)migm" 


#define SB_SWING_LEVEL 		0.5f /* SB level point swing : when 0.5 gear ratio */
#define UP_SB_PNT_RPM 		(-30.0f) /* unit: rpm */
#define UP_SP_PNT_RPM 		(30.0f)  /* unit: rpm */

#define UP_NtMax_PNT_RPM 	(-5.0f) /* 1-DAN gear ratio, unit: rpm */
#define UP_Ntmin_PNT_RPM 	(5.0f)  /* 2-DAN gear ratio, unit: rpm */

#define SB_PNT_RPM_DNS 		(30.0f) /* unit: rpm */
#define SP_PNT_RPM_DNS 		(-30.0f) /* unit: rpm */

#define PRINT_TXT_SB_SEQ_FIRST 	(TXT_SBTIME "." TXT_SBSWING0 ".." TXT_SBSWING0)
#define PRINT_TXT_SB_SEQ_LAST 	(TXT_SBSWING0 ".." TXT_SBSWING0 "." TXT_SBTIME)

//unsigned int iSBnewPoint[500] = {0,};


const double gearTable[11] = {
		0.000f,
		4.717f, /* 1-DAN */
		2.906f, /* 2-DAN */
		1.864f, /* 3-DAN */
		1.423f, /* 4-DAN */
		1.224f, /* 5-DAN */
		1.000f, /* 6-DAN */
		0.790f, /* 7-DAN */
		0.635f, /* 8-DAN */
		3.239f, /* Rear */
		3.510f, /* ??? */
};


/* ------------------------------------------------------ */
/* ------------------------------------------------------ */
/* 1) Power ON  + UP Shift -> APS Table                -- */
/* ------------------------------------------------------ */
/* ------------------------------------------------------ */

#define APS_PWR_ON_VAL 		(3.0f) /* APS >= 3%, 4% or 5% and then POWER On, else POWER OFF */
#define APS_TOLENANCE 		(2.0f)

#define APS_TABLE_NUM 		14
static double  ApsTble[APS_TABLE_NUM] = {
		 10.0f, /*  0 */
		 20.0f, /*  1 */
		 30.0f, /*  2 */
		 50.0f, /*  3 */
		 70.0f, /*  4 */
		100.0f, /*  5 */
		  0.0f, /*  6 */
		  0.0f, /*  7 */
		  0.0f, /*  8 */
		  0.0f, /*  9 */
		  0.0f, /* 10 */
		  0.0f, /* 11 */
		  0.0f, /* 12 */
		  0.0f, /* 13 */
		};			



/* ------------------------------------------------------ */
/* ------------------------------------------------------ */
/* 2) Power ON  + Down Shift -> VSkph Table            -- */
/* 3) Power OFF + UP Shift   -> VSkph Table            -- */
/* 4) Power OFF + Down Shift -> VSkph Table            -- */
/* ------------------------------------------------------ */
/* ------------------------------------------------------ */

#define VS_TABLE_NUM 		10 

/* Power On/Off + Down shift table and Index */
#define DNShi_tg76 		0 /* When Down shift & tgtGear 7 and 6, VS(Vehicle Speed) index in VSkphTblDN */
#define DNShi_tg54 		1 /* When Down shift & tgtGear 5 and 4, VS(Vehicle Speed) index in VSkphTblDN */
#define DNShi_tg3 		2 /* When Down shift & tgtGear 3, VS(Vehicle Speed) index in VSkphTblDN */
#define DNShi_tg2 		3 /* When Down shift & tgtGear 2, VS(Vehicle Speed) index in VSkphTblDN */
#define DNShi_tg1 		5 /* When Down shift & tgtGear 1, VS(Vehicle Speed) index in VSkphTblDN */

#define DNShi_NUM 		5 /* Down shift numner : 5ea*/
static short  VSkphTblDN[VS_TABLE_NUM] = {
		160, /* 0 <- Down shift Start position>> tgtGear : 7,6 (87Z, 86Z, 76Z), ------------------- (manual:87S, 76S) */
		140, /* 1 <- Down shift Start position>> tgtGear : 5,4 (85Z, 75Z, 65Z, 84Z, 74Z, 64Z, 54Z), (manual:65S) */
		120, /* 2 <- Down shift Start position>> tgtGear : 3   (83Z, 73Z, 63Z, 53Z, 43Z), --------- (manual:54S) */
		100, /* 3 <- Down shift Start position>> tgtGear : 2   (82Z, 72Z, 62Z, 52Z, 42Z, 32Z), ---- (manual:43S) */
 		 80, /* 4 <- Down shift Start position>>               ------------------------------------ (manual:32S) */
		 60, /* 5 <- Down shift Start position>> tgtGear : 1   (41Z, 31Z, 21Z), ------------------- (manual:21S) */
		 40, /* 6 */
		 30, /* 7 */
		 20, /* 8 */
		 10, /* 9 */
		};	

/* Power Off + Up shift table and Index */
#define POff_UPShi_tg2 		0 /* When Power Off + Up shift & tgtGear 2, VS(Vehicle Speed) index in VSkphTblUP */
#define POff_UPShi_tg3 		1 /* When Power Off + Up shift & tgtGear 3, VS(Vehicle Speed) index in VSkphTblUP */
#define POff_UPShi_tg4 		2 /* When Power Off + Up shift & tgtGear 4, VS(Vehicle Speed) index in VSkphTblUP */
#define POff_UPShi_tg5 		3 /* When Power Off + Up shift & tgtGear 5, VS(Vehicle Speed) index in VSkphTblUP */
#define POff_UPShi_tg6 		4 /* When Power Off + Up shift & tgtGear 6, VS(Vehicle Speed) index in VSkphTblUP */
#define POff_UPShi_tg78 	5 /* When Power Off + Up shift & tgtGear 7 and 8, VS(Vehicle Speed) index in VSkphTblUP */

#define POff_UPShi_NUM 		5 /* Power Off Up shift numner : 5ea*/
static short  VSkphTblUP[VS_TABLE_NUM] = {
		 10, /* 0 <- Up shift Start position>> (manual: 12S) */
		 20, /* 1 <- Up shift Start position>> (manual: 23S) */
		 30, /* 2 <- Up shift Start position>> (manual: 34S) */
		 40, /* 3 <- Up shift Start position>> (manual: 45S) */
 		 60, /* 4 <- Up shift Start position>> (manual: 56S) */
		 80, /* 5 <- Up shift Start position>> (manual: 67S, 78S) */
		100, /* 6 */
		120, /* 7 */
		140, /* 8 */
		160, /* 9 */
		};	



/* ------------------------------------------------------ */
/* ------------------------------------------------------ */
/* 5) Static Shift -> Shift & A/C(On/Off) & Temperature - */
/*                  P-R    A/C ON        Low (20deg)    - */
/*                         A/C ON        Middle (20~60) - */
/*                         A/C ON        High (60~)     - */
/*                         A/C OFF       Low            - */
/*                         A/C OFF       Middle         - */
/*                         A/C OFF       High           - */
/*                  P-D    A/C ON        Low            - */
/*                         A/C ON        Middle         - */
/*                         A/C ON        High           - */
/*                         A/C OFF       Low            - */
/*                         A/C OFF       Middle         - */
/*                         A/C OFF       High           - */
/*                  N-R    A/C ON        Low            - */
/*                         A/C ON        Middle         - */
/*                         A/C ON        High           - */
/*                         A/C OFF       Low            - */
/*                         A/C OFF       Middle         - */
/*                         A/C OFF       High           - */
/*                  N-D    A/C ON        Low            - */
/*                         A/C ON        Middle         - */
/*                         A/C ON        High           - */
/*                         A/C OFF       Low            - */
/*                         A/C OFF       Middle         - */
/*                         A/C OFF       High           - */
/* ------------------------------------------------------ */
/* ------------------------------------------------------ */


/* ------------------------------------------------------ */
/* ------------------------------------------------------ */
/* 6) Near to Stop Down Shift -> A/C(On/Off) & G Table -- */
/*                         87R : A/C ON      0  ~0.2   -- */
/*                               A/C OFF     0.2~0.5   -- */
/*                               A/C ON      0  ~0.2   -- */
/*                               A/C OFF     0.2~0.5   -- */
/*                         76R : A/C ON      0  ~0.2   -- */
/*                               A/C OFF     0.2~0.5   -- */
/*                               A/C ON      0  ~0.2   -- */
/*                               A/C OFF     0.2~0.5   -- */
/*                         65R : A/C ON      0  ~0.2   -- */
/*                               A/C OFF     0.2~0.5   -- */
/*                               A/C ON      0  ~0.2   -- */
/*                               A/C OFF     0.2~0.5   -- */
/*                         54R : A/C ON      0  ~0.2   -- */
/*                               A/C OFF     0.2~0.5   -- */
/*                               A/C ON      0  ~0.2   -- */
/*                               A/C OFF     0.2~0.5   -- */
/*                         43R : A/C ON      0  ~0.2   -- */
/*                               A/C OFF     0.2~0.5   -- */
/*                               A/C ON      0  ~0.2   -- */
/*                               A/C OFF     0.2~0.5   -- */
/*                         32R : A/C ON      0  ~0.2   -- */
/*                               A/C OFF     0.2~0.5   -- */
/*                               A/C ON      0  ~0.2   -- */
/*                               A/C OFF     0.2~0.5   -- */
/*                         21R : A/C ON      0  ~0.2   -- */
/*                               A/C OFF     0.2~0.5   -- */
/*                               A/C ON      0  ~0.2   -- */
/*                               A/C OFF     0.2~0.5   -- */
/* ------------------------------------------------------ */
/* ------------------------------------------------------ */







static double fAPSpwrLvl   = APS_PWR_ON_VAL;
static double fAPStol      = APS_TOLENANCE;  /* APS tolerance */
static short  iSBdecision  = SB_DECISION_TIMES;
static int    iJerkTimeLen = JERK_TIME_mSec;
static int    iNtTimeLen = NtMax_Before_mSec;

#define PWR_ON_UP_SHIFT_SSPOINT_START 		1

#define GMAX_RVALUE 	99.990f

typedef struct _sqdata_ {
	double  Time01;
	int     OTS02;
	double  VSP03;
	int     TqStnd04;
	int     iPATs05;
	int     EngTemp06;
	double  NetEng_Acor; /* New items - 2023.02.04 */
	double  tqi07;
	int     curGear08;
	double  APS09;
	double  No10;
	int     tgtGear11;
	int 	MSs_Ntg;     /* New items - 2023.02.04 */
	int     ShiTy12;
	double  TqFr13;
	int     ShiPh14;     /* New items - 2023.02.04 - FORMAT4 */
	double  acor_Nm;
	double  Ne15;
	double	Nt16;
	double	LAcc17;
} sqd_type;



	



typedef struct _sqdata2nd_ {
	double  Time01;
	int     OTS02;
	double  VSP03;
	int     TqStnd04;
	int     iPATs05;
	char 	iPATs05S[20];
	int     EngTemp06;
	double  tqi07;
	int     curGear08;
	double  APS09;
	double  No10;
	int     tgtGear11;
	int 	ShiNew12;
	int     ShiTy12;
	char 	arrGear[10];
	double  TqFr13;
	int     ShiPh14;
	double  Ne15;
	double	Nt16;
	double	LAcc17;
	char 	sTimePos[LEN_POS]; /* (***), (SS), (SB), (SP), (*u*), (*d*) */
	int     sPosNum;
	double 	Gsum;
	char 	sTimeNtPos[LEN_POS]; /* (***), (NtMx) (NtMn) (*u*), (*d*) */
	double 	gearRat;
	double 	gearShiOne;
	double 	gearShiTwo;
	double 	fJerk0;
	double 	MaxAcc; // g_Max
	double 	minAcc; // g_min
	double  LPFAcc; /* Low Pass Filtered LACC */
	//int     sqvalid; // SS/SB/SP pair check : 1:valid, 0: NG,
} sqd2nd_type;


typedef struct _sqdata3rd_ {
	double  Time01;
	int     OTS02;
	double  VSP03;
	int     TqStnd04;
	int     iPATs05;
	char 	iPATs05S[20];
	int     EngTemp06;
	double  tqi07;
	int     curGear08;
	double  APS09;
	double  No10;
	int     tgtGear11;
	int 	ShiNew12;
	int     ShiTy12;
	char 	arrGear[10];
	double  TqFr13;
	int     ShiPh14;
	double  Ne15;
	double	Nt16;
	double	LAcc17;
	double  DiffTime;
	char 	sTimePos[LEN_POS]; /* (***), (SS), (SB), (SP), (*u*), (*d*) */
	int     sPosNum;
	double 	Gsum;
	char 	sTimeNtPos[LEN_POS]; /* (***), (NtMx) (NtMn) (*u*), (*d*) */
	char 	sTimeGpos[LEN_POS]; /* (***), Gmax Gmin */
	double 	gearRat;
	double 	gearShiOne;
	double 	gearShiTwo;
	double 	fJerk0;
	double 	fJerk1;

	double  MaxAcc; // g_Max
	double  minAcc; // g_min
	double  LPFAcc; /* Low Pass Filtered LACC */
} sqd3rd_type;



typedef struct _sqdataFlt_ {
	double  Time01;
	int     OTS02;
	double  VSP03;
	int     TqStnd04;
	int     iPATs05;
	char 	iPATs05S[20];
	int     EngTemp06;
	double  tqi07;
	int     curGear08;
	double  APS09;
	double  No10;
	int     tgtGear11;
	int 	ShiNew12;
	int     ShiTy12;
	char 	arrGear[10];
	double  TqFr13;
	int     ShiPh14;
	double  Ne15;
	double	Nt16;
	double	LAcc17;
	double  DiffTime;
	char 	sTimePos[LEN_POS]; /* (***), (SS), (SB), (SP), (*u*), (*d*) */
	int     sPosNum;
	double 	Gavg; /* Gsum or Gavg */
	char 	sTimeNtPos[LEN_POS]; /* (***), (NtMx) (NtMn) (*u*), (*d*) */
	char 	sTimeGpos[LEN_POS]; /* (***), Gmax Gmin */
	double 	gearRat;
	double 	gearShiOne;
	double 	gearShiTwo;
	double 	fJerk0;
	double 	fJerk1;

	double  fJerk2;
	int 	deltams; // delta msec
	double  LPFAcc; /* Low Pass Filtered LACC */
} sqdflt_type;


typedef struct _sqdataAPS_ {
	double  Time01;
	int     OTS02;
	double  VSP03;
	int     TqStnd04;
	int     iPATs05;
	char 	iPATs05S[20];
	int     EngTemp06;
	double  tqi07;
	int     curGear08;
	double  APS09;
	double  No10;
	int     tgtGear11;
	int 	ShiNew12;
	int     ShiTy12;
	char 	arrGear[10];
	double  TqFr13;
	int     ShiPh14;
	double  Ne15;
	double	Nt16;
	double	LAcc17;
	double  DiffTime;
	char 	sTimePos[LEN_POS]; /* (***), (SS), (SB), (SP), (*u*), (*d*) */
	int     sPosNum;
	double 	Gavg; /* Gsum or Gavg */
	char 	sTimeNtPos[LEN_POS]; /* (***), (NtMx) (NtMn) (*u*), (*d*) */
	char 	sTimeGpos[LEN_POS]; /* (***), Gmax Gmin */
	double 	gearRat;
	double 	gearShiOne;
	double 	gearShiTwo;
	double 	fJerk0;
	double 	fJerk1;
	double  fJerk2;
	int 	deltams; // delta msec

	int   	apsIdx;
	double  LPFAcc; /* Low Pass Filtered LACC */	
} sqdAps_type;


/* ------- original tsv text file read format ----------------------- */
#define 	RD_TIME01 		"%lf"
#define 	RD_OTS02 		"%d"
#define 	RD_VSP03 		"%lf"
#define 	RD_TQSTND04 	"%d"
#define 	RD_iPATS05 		"%d"
#define 	RD_iPATS05S 	"%s"
#define 	RD_EngTemp06 	"%d"
#define 	RD_NetEng_Acor 	"%lf" /* New items - 2023.02.04 - SQD_INFMT3 */
#define 	RD_MSs_Ntg 		"%d"  /* New items - 2023.02.04 - SQD_INFMT3 */
#define 	RD_acor_Nm 		"%lf" /* New items - 2023.02.04 - SQD_INFMT4 */
#define 	RD_TQI07 		"%lf"
#define 	RD_curGear08 	"%d"
#define 	RD_APS09 		"%lf"
#define 	RD_No10 		"%lf"
#define 	RD_tgtGear11 	"%d"
#define 	RD_ShiType12 	"%d"
#define 	RD_ShiNew12 	"%d"
#define 	RD_Shi12S  		"%s"
#define 	RD_tQFr13 		"%lf"
#define 	RD_ShiftPh14 	"%d"
#define 	RD_Ne15 		"%lf"
#define 	RD_Nt16 		"%lf"
#define 	RD_LAcc17 		"%lf"


enum {
	USE_IN_CASE0_NONE = 0,
	USE_IN_CASE1_17 = 1,
	USE_IN_CASE2_15,
	USE_IN_NEW_CASE_3_15,
	USE_IN_NEW_CASE_4_15,


	USE_IN_MAX = 9	
}; // eUseCase_type;

/* first format */
#define 	SQD_INFORMAT 	(RD_TIME01 " " RD_OTS02 " " RD_VSP03 " " RD_TQSTND04 " " RD_iPATS05 " " RD_EngTemp06 " " \
							RD_TQI07 " " RD_curGear08 " " RD_APS09 " " RD_No10 " " RD_tgtGear11 " " RD_ShiType12 " " \
							RD_tQFr13 " " RD_ShiftPh14 " " RD_Ne15 " " RD_Nt16 " " RD_LAcc17)

/* other format */
#define 	SQD_INFMT2 		(RD_TIME01 " " RD_TQSTND04 " " RD_OTS02 " " RD_EngTemp06 " " RD_VSP03 " " RD_iPATS05 " "\
							RD_tgtGear11 " " RD_APS09 " " RD_curGear08 " " RD_ShiftPh14 " " RD_LAcc17 " " RD_ShiType12 " "\
							RD_Nt16 " " RD_No10 " " RD_Ne15)


/* other format */
#define 	SQD_INFMT3_15 	(RD_TIME01 " " RD_OTS02 " " RD_VSP03 " " RD_iPATS05 " " RD_EngTemp06 " " RD_NetEng_Acor " "\
							RD_curGear08 " " RD_APS09 " " RD_No10 " " RD_tgtGear11 " " RD_MSs_Ntg " " RD_ShiType12 " "\
							RD_Ne15 " " RD_Nt16 " " RD_LAcc17)


#define 	SQD_INFMT4_15 	(RD_TIME01 " " RD_OTS02 " " RD_MSs_Ntg " " RD_EngTemp06 " " RD_VSP03 " " RD_iPATS05 " "\
							RD_tgtGear11 " " RD_APS09 " " RD_curGear08 " " RD_LAcc17 " " RD_ShiType12 " " RD_Nt16 " "\
							RD_No10 " " RD_acor_Nm " " RD_Ne15)








#define 	RD_TimePos 	 	"%s"
#define 	RD_TimePosNum 	"%d"
#define 	RD_Gsum 		"%lf"
#define 	RD_Gavg 		"%lf"
#define 	RD_TimeNtPos 	"%s"
#define 	RD_TimeGpos 	"%s"
#define 	RD_GearRatio  	"%lf"
#define 	RD_GearSOne  	"%lf"
#define 	RD_GearSTwo  	"%lf"
#define 	RD_Jerk0  		"%lf"
#define 	RD_Jerk1st  	"%lf"
#define 	RD_MaxNe  		"%lf"
#define 	RD_MaxNt  		"%lf"
#define 	RD_MaxAcc  		"%lf" /* g_Max */
#define 	RD_minAcc  		"%lf" /* g_min */
#define   	RD_LPFAcc       "%lf" /* Low Pass Filter LAcc */
#define 	RD_SQValid 		"%d" 
#define 	RD_TimeDiff  	"%lf"
#define 	RD_TimeTxt 		"%s"
#define 	RD_Jerk2nd  	"%lf"
#define 	RD_DeltaTime  	"%d"
#define 	RD_APSIndex  	"%d"


#define 	RD_MaxNe_D  	"%d" /* x100 scale up */
#define 	RD_MaxNt_D  	"%d" /* x100 scale up */
#define 	RD_MaxAcc_D  	"%d" /* x1000, g_Max */
#define 	RD_minAcc_D  	"%d" /* x1000, g_min */

/* ------- original tsv text file read format ----------------------- */



char shift_file[MAX_CHARS*LENGTH_OF_FILENAME+1];
char shift_in[MAX_CHARS*LENGTH_OF_FILENAME+1];

/* timeShift, iPATs05, arrPATs_ModeID[iPATs05].ModeID, tqi, curGear08, Aps09, sNo10, tgtGear11, 
iShift, iShiType12, arrGear[iShiType12].sGear, TqFr, ShiftPh, sNe, sNt16, Long_Accel, gearRatio, rpmG */

#define SAVEMODE 					1
#define LOW_PASS_FILTER_GACC 		1 /* 2023-02-09, Low Pass Filter for LAcc */


/* ------- 1st saved text file write format ----------------------- */
#define 	WR_TIME01 		" %11.5lf"
#define 	WR_OTS02 		"[@%d"
#define 	WR_VSP03 		" %6.1lf"
#define 	WR_TQSTND04 	"[=%d"
#define 	WR_iPATS05 		" %3d"
#define 	WR_iPATS05S 	" %-10s"
#define 	WR_EngTemp06 	"[/%d"
#define 	WR_TQI07 		" %7.2lf"
#define 	WR_curGear08 	" %2d"
#define 	WR_APS09 		" %8.2lf"
#define 	WR_No10 		" %8.2lf"
#define 	WR_tgtGear11 	" %2d"
#define 	WR_ShiNew12   	" %2d"
#define 	WR_ShiType12 	" %3d"
#define 	WR_Shi12S 	 	" %-8s"
#define 	WR_tQFr13 		" %7.2lf"
#define 	WR_ShiftPh14 	" %2d"
#define 	WR_Ne15 		" %8.2lf"
#define 	WR_Nt16 		" %8.2lf"
#define 	WR_LAcc17 		" %7.4lf"
#define 	WR_TimeDiff 	" %8.3lf"
#define 	WR_TimePos 	 	" %-15s"
#define 	WR_TimePosNum 	" %4d"
#define 	WR_Gsum 		" %10.4lf"
#define 	WR_Gavg 		" %7.4lf"
#define 	WR_TimeNtPos 	" %-7s"
#define 	WR_TimeGpos 	" %-7s"
#define 	WR_GearRatio  	" %5.3lf"
#define 	WR_GearSOne  	" %9.2lf"
#define 	WR_GearSTwo  	" %9.2lf"
#define 	WR_Jerk0  		" %8.2lf"
#define 	WR_Jerk1  		" %8.2lf"
#define 	WR_MaxNe  		" %8.2lf"
#define 	WR_MaxNt  		" %8.2lf"
#define 	WR_MaxAcc  		" %7.4lf"
#define 	WR_minAcc  		" %7.4lf"
#define 	WR_LPFilterAcc  " %7.4lf"
#define 	WR_TimeTxt 		" %-2s"
#define 	WR_Jerk2  		" %8.2lf"
#define 	WR_mSec  		" %5d"
#define 	WR_APSIdx  		" %2d"
#define 	WR_SQValid 		" %2d" /* 2023-01-09 */

#define 	WR_MaxNe_DEC    " %6d"
#define 	WR_MaxNt_DEC  	" %6d"
#define 	WR_MaxAcc_DEC  	" %5d"
#define 	WR_minAcc_DEC  	" %5d"

#define 	WR_APS_D 		" %3d"
#define 	WR_APS_F 		" %5.1lf"

#define 	WR_MAX_POS 		"%s"
#define 	WR_MAX_DIFF 	" (%+4d)"


//#define SAVEFMT  "%10.4lf  %3d %-10s %6.1lf %7.2lf  %2d %8.2lf %8.2lf %2d %03d %3d %-8s %7.2lf %2d %8.2lf %8.2lf %6.3lf %-6s %4.2lf %9.2lf %9.2lf %7.2lf %9.2lf %9.2lf"

#define SAVEFMT 	(WR_TIME01 WR_iPATS05 WR_iPATS05S WR_VSP03 WR_TQI07 WR_curGear08 WR_APS09 WR_No10 \
					WR_tgtGear11 WR_ShiNew12 WR_ShiType12 WR_Shi12S WR_tQFr13 WR_ShiftPh14 WR_Ne15 WR_Nt16 \
					WR_LAcc17 WR_LPFilterAcc WR_TimePos WR_TimeNtPos WR_GearRatio WR_GearSOne WR_GearSTwo WR_Jerk0 WR_MaxAcc WR_minAcc)

#define SAVEFMT2 	(WR_TIME01 WR_iPATS05 WR_iPATS05S WR_VSP03 WR_TQI07 WR_ShiNew12 WR_ShiType12 WR_Shi12S \
					WR_APS09 WR_TimeDiff WR_TimeTxt WR_LAcc17 WR_Nt16 WR_Ne15)

/* ------- 1st saved text file write format ----------------------- */

#define RD_2ndFMT 	(RD_TIME01 " " RD_iPATS05 " " RD_iPATS05S " " RD_VSP03 " " RD_TQI07 " " RD_curGear08 " " RD_APS09 " " RD_No10 " " RD_tgtGear11 " "\
					RD_ShiNew12 " " RD_ShiType12 " " RD_Shi12S " " RD_tQFr13 " " RD_ShiftPh14 " " RD_Ne15 " " RD_Nt16 " " RD_LAcc17 " " RD_LPFAcc " "\
					RD_TimePos " "\
					RD_TimeNtPos " " RD_GearRatio " " RD_GearSOne " " RD_GearSTwo " " RD_Jerk0 " " RD_MaxAcc " " RD_minAcc)


#define RD_23YNW 	(RD_TIME01 " " RD_iPATS05 " " RD_iPATS05S " " RD_VSP03 " " RD_TQI07 " " RD_curGear08 " " RD_APS09 " " RD_No10 " " RD_tgtGear11 " "\
					RD_ShiNew12 " " RD_ShiType12 " " RD_Shi12S " " RD_tQFr13 " " RD_ShiftPh14 " " RD_Ne15 " " RD_Nt16 " " RD_LAcc17 " " RD_LPFAcc " "\
					RD_TimePos " "\
					RD_TimePosNum " " RD_Gsum " " RD_TimeNtPos " " RD_GearRatio " " RD_GearSOne " " RD_GearSTwo " " RD_Jerk0 " " RD_MaxAcc " " RD_minAcc)


#define RD_NewFM 	(RD_TIME01 " " RD_iPATS05 " " RD_iPATS05S " " RD_VSP03 " " RD_TQI07 " " RD_curGear08 " " RD_APS09 " " RD_No10 " " RD_tgtGear11 " "\
					RD_ShiNew12 " " RD_ShiType12 " " RD_Shi12S " " RD_tQFr13 " " RD_ShiftPh14 " " RD_Ne15 " " RD_Nt16 " " RD_LAcc17 " " RD_LPFAcc " "\
					RD_TimePos " "\
					RD_TimePosNum " " RD_Gsum " " RD_TimeNtPos " " RD_TimeGpos "" RD_GearRatio " " RD_GearSOne " " RD_GearSTwo " " RD_Jerk0 " "\
					RD_MaxAcc " " RD_minAcc)


#define SAVE_2ndFMT (WR_TIME01 WR_iPATS05 WR_iPATS05S WR_curGear08 WR_tgtGear11 WR_VSP03 WR_TQI07 WR_APS09 WR_No10 \
					 WR_ShiNew12 WR_ShiType12 WR_Shi12S WR_tQFr13 WR_ShiftPh14 WR_Ne15 WR_Nt16 WR_LAcc17 WR_TimeDiff \
					 WR_TimePos WR_GearRatio WR_Jerk1 WR_MaxNe WR_MaxNt WR_MaxAcc WR_minAcc)

#define SAVE_ChkFMT (WR_TIME01 WR_iPATS05 WR_iPATS05S WR_VSP03 WR_TQI07 WR_curGear08 WR_APS09 WR_No10 WR_tgtGear11 \
					 WR_ShiNew12 WR_ShiType12 WR_Shi12S WR_tQFr13 WR_ShiftPh14 WR_Ne15 WR_Nt16 WR_LAcc17 WR_LPFilterAcc WR_TimePos \
					 WR_TimePosNum WR_Gsum WR_TimeNtPos WR_GearRatio WR_GearSOne WR_GearSTwo WR_Jerk0 WR_MaxAcc WR_minAcc)


#define SAVE_23YNW 	(WR_TIME01 WR_iPATS05 WR_iPATS05S WR_VSP03 WR_TQI07 WR_curGear08 WR_APS09 WR_No10 WR_tgtGear11 \
					 WR_ShiNew12 WR_ShiType12 WR_Shi12S WR_tQFr13 WR_ShiftPh14 WR_Ne15 WR_Nt16 WR_LAcc17 WR_LPFilterAcc WR_TimePos \
					 WR_TimePosNum WR_Gsum WR_TimeNtPos WR_TimeGpos WR_GearRatio WR_GearSOne WR_GearSTwo WR_Jerk0 \
					 WR_MaxAcc WR_minAcc)



#define SAVE_2NWFMT (WR_TIME01 WR_iPATS05 WR_iPATS05S WR_curGear08 WR_tgtGear11 WR_VSP03 WR_TQI07 WR_APS09 WR_No10 \
					 WR_ShiNew12 WR_ShiType12 WR_Shi12S WR_tQFr13 WR_ShiftPh14 WR_Ne15 WR_Nt16 WR_LAcc17 WR_TimeDiff \
					 WR_TimePos WR_GearRatio WR_Jerk1 WR_MaxNe_DEC WR_MaxNt_DEC WR_MaxAcc_DEC WR_minAcc_DEC)


#define RD_3rdFMT 	(RD_TIME01 " " RD_iPATS05 " " RD_iPATS05S " " RD_curGear08 " " RD_tgtGear11 " " RD_VSP03 " " RD_TQI07 " " RD_APS09 " " RD_No10 " "\
					RD_ShiNew12 " " RD_ShiType12 " " RD_Shi12S " " RD_tQFr13 " " RD_ShiftPh14 " " RD_Ne15 " " RD_Nt16 " " RD_LAcc17 " " RD_TimeDiff " "\
					RD_TimePos " " RD_GearRatio " " RD_Jerk1st " " RD_MaxNe " " RD_MaxNt " " RD_MaxAcc " " RD_minAcc)


#define RD_FltFMT 	(RD_TIME01 " " RD_iPATS05 " " RD_iPATS05S " " RD_curGear08 " " RD_tgtGear11 " " RD_VSP03 " " RD_TQI07 " " RD_APS09 " " RD_No10 " "\
					RD_ShiNew12 " " RD_ShiType12 " " RD_Shi12S " " RD_tQFr13 " " RD_ShiftPh14 " " RD_Ne15 " " RD_Nt16 " " RD_LAcc17 " " RD_TimeDiff " "\
					RD_TimePos " " RD_GearRatio " " RD_Jerk1st " " RD_Jerk2nd " " RD_DeltaTime)


#define RD_APS_FMT 	(RD_TIME01 " " RD_iPATS05 " " RD_iPATS05S " " RD_curGear08 " " RD_tgtGear11 " " RD_VSP03 " " RD_TQI07 " " RD_APS09 " " RD_No10 " "\
					RD_ShiNew12 " " RD_ShiType12 " " RD_Shi12S " " RD_tQFr13 " " RD_ShiftPh14 " " RD_Ne15 " " RD_Nt16 " " RD_LAcc17 " " RD_LPFAcc " "\
					RD_TimeDiff " "\
					RD_TimePos " " RD_TimePosNum " " RD_Gavg " " RD_TimeNtPos " " RD_TimeGpos " " RD_GearRatio " " RD_Jerk0 " " RD_Jerk1st " "\
					RD_DeltaTime " " RD_APSIndex)


#define SAVE_3rdFMT (WR_TIME01 WR_iPATS05 WR_iPATS05S WR_curGear08 WR_tgtGear11 WR_VSP03 WR_TQI07 WR_APS09 WR_No10 \
					WR_ShiNew12 WR_ShiType12 WR_Shi12S WR_tQFr13 WR_ShiftPh14 WR_Ne15 WR_Nt16 WR_LAcc17 WR_TimeDiff \
					WR_TimePos WR_GearRatio WR_Jerk1 WR_MaxNe WR_MaxNt WR_MaxAcc WR_minAcc)


#define RD_3NWFMT 	(RD_TIME01 " " RD_iPATS05 " " RD_iPATS05S " " RD_curGear08 " " RD_tgtGear11 " " RD_VSP03 " " RD_TQI07 " " RD_APS09 " " RD_No10 " "\
					RD_ShiNew12 " " RD_ShiType12 " " RD_Shi12S " " RD_tQFr13 " " RD_ShiftPh14 " " RD_Ne15 " " RD_Nt16 " " RD_LAcc17 " " RD_TimeDiff " "\
					RD_TimePos " " RD_GearRatio " " RD_Jerk1st " " RD_MaxNe_D " " RD_MaxNt_D " " RD_MaxAcc_D " " RD_minAcc_D)


#define SAVE_3NWFMT (WR_TIME01 WR_iPATS05 WR_iPATS05S WR_curGear08 WR_tgtGear11 WR_VSP03 WR_TQI07 WR_APS09 WR_No10 \
					WR_ShiNew12 WR_ShiType12 WR_Shi12S WR_tQFr13 WR_ShiftPh14 WR_Ne15 WR_Nt16 WR_LAcc17 WR_TimeDiff \
					WR_TimePos WR_GearRatio WR_Jerk1 WR_MaxNe_DEC WR_MaxNt_DEC WR_MaxAcc_DEC WR_minAcc_DEC)


#define SAVE_APSFMT (WR_TIME01 WR_iPATS05 WR_iPATS05S WR_curGear08 WR_tgtGear11 WR_VSP03 WR_TQI07 WR_APS09 WR_No10 \
					WR_ShiNew12 WR_ShiType12 WR_Shi12S WR_tQFr13 WR_ShiftPh14 WR_Ne15 WR_Nt16 WR_LAcc17 WR_LPFilterAcc WR_TimeDiff \
					WR_TimePos WR_TimePosNum WR_Gavg WR_TimeNtPos WR_TimeGpos WR_GearRatio WR_Jerk0 WR_Jerk1 WR_mSec WR_APSIdx)


#define SAVE_FltFMT (WR_TIME01 WR_iPATS05 WR_iPATS05S WR_curGear08 WR_tgtGear11 WR_VSP03 WR_TQI07 WR_APS09 WR_No10 \
					WR_ShiNew12 WR_ShiType12 WR_Shi12S WR_tQFr13 WR_ShiftPh14 WR_Ne15 WR_Nt16 WR_LAcc17 WR_LPFilterAcc WR_TimeDiff \
					WR_TimePos WR_TimePosNum WR_Gsum WR_TimeNtPos WR_TimeGpos WR_GearRatio WR_Jerk0 WR_Jerk1 WR_mSec)


#define SAVE_SortFMT (WR_TIME01 WR_iPATS05 WR_iPATS05S WR_curGear08 WR_tgtGear11 WR_VSP03 WR_TQI07 WR_APS09 WR_No10 \
					WR_ShiNew12 WR_ShiType12 WR_Shi12S WR_tQFr13 WR_ShiftPh14 WR_Ne15 WR_Nt16 WR_LAcc17 WR_TimeDiff \
					WR_TimePos WR_GearRatio WR_Jerk1 WR_Jerk2 WR_mSec)



#define RD_TxtFMT 	(RD_TIME01 " " RD_iPATS05 " " RD_iPATS05S " " RD_curGear08 " " RD_tgtGear11 " " RD_VSP03 " " RD_TQI07 " " RD_APS09 " " RD_No10 " "\
					RD_ShiNew12 " " RD_ShiType12 " " RD_Shi12S " " RD_tQFr13 " " RD_ShiftPh14 " " RD_Ne15 " " RD_Nt16 " " RD_LAcc17 " " RD_LPFAcc " "\
					RD_TimeDiff " "\
					RD_TimePos " " RD_TimePosNum " " RD_Gavg " " RD_TimeNtPos " " RD_TimeGpos " " RD_GearRatio " " RD_Jerk0 " " RD_Jerk1st " " RD_DeltaTime)




#define NtMAX_FORMAT	("%5d" WR_TIME01 WR_MaxNt)
#define NeMAX_FORMAT	("%5d" WR_TIME01 WR_MaxNe)
#define gMAX_FORMAT		("%5d" WR_TIME01 WR_MaxAcc)
#define gmin_FORMAT		("%5d" WR_TIME01 WR_minAcc)

#define WR_NcurGear 	"%2d"
#define WR_NtgtGear 	"%2d"

#define WRP_TimeDiff 	" %6.3lf"
#define WRP_mSec  		" %4d"

#define SAVE_PRT_01 	(WR_NcurGear WR_NtgtGear WR_Shi12S WR_APS_D WR_APS_F)
#define SAVE_PRT_02 	(WRP_TimeDiff WRP_TimeDiff WRP_TimeDiff WR_LAcc17 WR_LAcc17 WR_LAcc17 WR_LAcc17)
#define SAVE_PRT_03 	(WR_Jerk1 WRP_mSec WR_Nt16 WR_Nt16 WR_Nt16)
#define SAVE_PRT_04 	(WR_Nt16 WR_Nt16 WR_MAX_POS WR_Ne15 WR_Ne15 WR_MAX_POS)
#define SAVE_PRT_05 	(WR_Nt16 WR_Ne15 WR_Ne15)

#define SAVE_PRT_10 	(" 3(F)_300msec : continuous 3 times and first poistion. Before 300msec postion from SB position. \n")
#define SAVE_PRT_11 	(" APSt : APS Table values \n")
#define SAVE_PRT_12 	(" APSv : APS values from vehicle at SS position \n")
#define SAVE_PRT_13 	(" t1   : SS~SB time (sec) \n")
#define SAVE_PRT_14 	(" t2   : SB~SP time (sec) \n")
#define SAVE_PRT_15 	(" Jerk : calculated Jerk value \n")
#define SAVE_PRT_16 	(" msec : Time duration(min and Max) used to calculate Jerk (msec) \n")


#define SAVE_PRT_20 	("SS : Shfit Signal (변속 신호)")
#define SAVE_PRT_21 	("SB : Shift Begin (변속 개시점)")
#define SAVE_PRT_22 	("SP : Shift Synchronise (변속 회전수 동기)")
#define SAVE_PRT_23 	("SF : Shift Finish (변속 종료)")



/* 2022-12-18, to buffer */
#define MAX_DATA_FILE_SAVED 	0 /* NOT file saved, Used buffer */

//#if (0==MAX_DATA_FILE_SAVED)
typedef struct _minMax_buf_ {
	int iIndex; 
	double	msTime;
	double	mValue;
	int SBnumber;
	int SPnumber;
} tminMax_type;
//#endif


typedef struct _tGear_ {
		int 	iGear;
		char 	sGear[10];
} tGear_type;


#define ERROR_NUM_LIMIT 		10

#define PWR_ON_12Z 		15
#define PWR_ON_23Z 		16
#define PWR_ON_34Z 		17
#define PWR_ON_45Z 		18
#define PWR_ON_56Z 		19
#define PWR_ON_67Z 		20
#define PWR_ON_78Z 		21

#define PWR_OFF_12S 	22
#define PWR_OFF_23S 	23
#define PWR_OFF_34S 	24
#define PWR_OFF_45S 	25
#define PWR_OFF_56S 	26
#define PWR_OFF_67S 	27
#define PWR_OFF_78S 	28

#define GEAR_SHI_NUMS 			162

const tGear_type arrGear[GEAR_SHI_NUMS] = {
		{  0, 	"(sN)"    },
		{  1,	"(s1)"    },
		{  2,	"(s2)"    },
		{  3,	"(s3)"    },
		{  4,	"(s4)"    },
		{  5,	"(s5)"    },
		{  6,	"(s6)"    },
		{  7,	"(s7)"    },
		{  8,	"(s8)"    },
		{  9,	"(sP)"    },
		{ 10,	"(sR)"    },
		{ 11,	"(s1_Y)"  },
		{ 12,	"(s2_Y)"  },
		{ 13,	"(s1_J)"  },
		{ 14,	"(s2_J)"  },
 
		{ 15,	"(s12Z)"  }, /* Power On */
		{ 16,	"(s23Z)"  }, /* Power On */
		{ 17,	"(s34Z)"  }, /* Power On */
		{ 18,	"(s45Z)"  }, /* Power On */
		{ 19,	"(s56Z)"  }, /* Power On */
		{ 20,	"(s67Z)"  }, /* Power On */
		{ 21,	"(s78Z)"  }, /* Power On */

		{ 22,	"(s12S)"  }, /* Power Off */
		{ 23,	"(s23S)"  }, /* Power Off */
		{ 24,	"(s34S)"  }, /* Power Off */
		{ 25,	"(s45S)"  }, /* Power Off */
		{ 26,	"(s56S)"  }, /* Power Off */
		{ 27,	"(s67S)"  }, /* Power Off */
		{ 28,	"(s78S)"  }, /* Power Off */

		{ 29,	"(s12MZ)" },
		{ 30,	"(s23MZ)" },
		{ 31,	"(s34MZ)" },
		{ 32,	"(s45MZ)" },
		{ 33,	"(s56MZ)" },
		{ 34,	"(s67MZ)" },	
		{ 35,	"(s78MZ)" },	

		{ 36,	"(s21Z)"  },
		{ 37,	"(s32Z)"  },
		{ 38,	"(s43Z)"  },
		{ 39,	"(s54Z)"  },
		{ 40,	"(s65Z)"  },
		{ 41,	"(s76Z)"  },
		{ 42,	"(s87Z)"  },
 
		{ 43,	"(s21S)"  },
		{ 44,	"(s32S)"  },
		{ 45,	"(s43S)"  },
		{ 46,	"(s54S)"  },
		{ 47,	"(s65S)"  },
		{ 48,	"(s76S)"  },
		{ 49,	"(s87S)"  },
 
		{ 50,	"(s21MZ)" },	
		{ 51,	"(s32MZ)" },
		{ 52,	"(s43MZ)" },	
		{ 53,	"(s54MZ)" },	
		{ 54,	"(s65MZ)" },	
		{ 55,	"(s76MZ)" },	
		{ 56,	"(s87MZ)" },	

		{ 57,	"(s31Z)"  }, /* Skip Down Shift */
		{ 58,	"(s41Z)"  }, /* Skip Down Shift */
		{ 59,	"(s42Z)"  }, /* Skip Down Shift */
		{ 60,	"(s52Z)"  }, /* Skip Down Shift */
		{ 61,	"(s53Z)"  }, /* Skip Down Shift */
		{ 62,	"(s62Z)"  }, /* Skip Down Shift */
		{ 63,	"(s63Z)"  }, /* Skip Down Shift */
		{ 64,	"(s64Z)"  }, /* Skip Down Shift */
		{ 65,	"(s72Z)"  }, /* Skip Down Shift */
		{ 66,	"(s73Z)"  }, /* Skip Down Shift */
		{ 67,	"(s74Z)"  }, /* Skip Down Shift */
		{ 68,	"(s75Z)"  }, /* Skip Down Shift */
		{ 69,	"(s82Z)"  }, /* Skip Down Shift */
		{ 70,	"(s83Z)"  }, /* Skip Down Shift */
		{ 71,	"(s84Z)"  }, /* Skip Down Shift */
		{ 72,	"(s85Z)"  }, /* Skip Down Shift */
		{ 73,	"(s86Z)"  }, /* Skip Down Shift */
 
		{ 74,	"(s31S)"  },
		{ 75,	"(s42S)"  },
		{ 76,	"(s53S)"  },
		{ 77,	"(s64S)"  },
		{ 78,	"(s75S)"  },
		{ 79,	"(s86S)"  },
		{ 80,	"(s31MZ)" },
		{ 81,	"(s42MZ)" },	
		{ 82,	"(s53MZ)" },	
		{ 83,	"(s64MZ)" },	
		{ 84,	"(s75MZ)" },	
		{ 85,	"(s86MZ)" },	
		{ 86,	"(s21R)"  },
		{ 87,	"(s31R)"  },
		{ 88,	"(s32R)"  },
		{ 89,	"(s42R)"  },
		{ 90,	"(s43R)"  },
		{ 91,	"(s53R)"  },
		{ 92,	"(s54R)"  },
		{ 93,	"(s64R)"  },
		{ 94,	"(s65R)"  },
		{ 95,	"(s75R)"  },
		{ 96,	"(s76R)"  },
		{ 97,	"(s86R)"  },
		{ 98,	"(s87R)"  },
		{ 99,	"(s121)"  },
		{100,	"(s232)"  },
		{101,	"(s343)"  },	
		{102,	"(s454)"  },	
		{103,	"(s565)"  },	
		{104,	"(s676)"  },	
		{105,	"(s787)"  },	
		{106,	"(s212)"  },	
		{107,	"(s313)"  },	
		{108,	"(s414)"  },	
		{109,	"(s424)"  },	
		{110,	"(s525)"  },	
		{111,	"(s535)"  },	
		{112,	"(s626)"  },	
		{113,	"(s636)"  },	
		{114,	"(s646)"  },	
		{115,	"(s727)"  },	
		{116,	"(s737)"  },
		{117,	"(s747)"  },	
		{118,	"(s757)"  },	
		{119,	"(s828)"  },	
		{120,	"(s838)"  },	
		{121,	"(s848)"  },	
		{122,	"(s858)"  },	
		{123,	"(s868)"  },
		{124,	"(sN1)"   },
		{125,	"(sN2)"   },
		{126,	"(sN3)"   },
		{127,	"(sN4)"   },
		{128,	"(sN5)"   },
		{129,	"(sN6)"   },
		{130,	"(sN7)"   },
		{131,	"(sN8)"   },
		{132,	"(sXN)"   },
		{133,	"(sDN)"   },
		{134,	"(sD2N)"  },
		{135,	"(sRN)"   },
		{136,	"(sY1N)"  },	
		{137,	"(sY2N)"  },	
		{138,	"(s1Y)"   },
		{139,	"(sNY)"   },
		{140,	"(s1J)"   },
		{141,	"(s2J)"   },
		{142,	"(sJ1N)"  },	
		{143,	"(sJ2N)"  },	
		{144,	"(sDR)"   },
		{145,	"(sD2R)"  },	
		{146,	"(sRDR)"  },
		{147,	"(sNR)"   },
		{148,	"(sRD)"   },
		{149,	"(sRD2)"  },
		{150,	"(sRRD)"  },
		{151,	"(sRRD2)" },
		{152,	"(sND1)"  },
		{153,	"(sND2)"  },
		{154,	"(sY1)"   },
		{155,	"(sYR)"   },
		{156,	"(sRD2R)" },
		{157,	"(sJ1)"   },
		{158,	"(sJ2)"   },
		{159,	"(sJ1R)"  },
		{160,	"(sJ2R)"  },
		{161,   "(UNKNOWN)" },
	};	

#define MODE_ID_NUMS 			72

typedef struct _tPATs_ {
		int 	iPATs;
		char	ModeID[20];
		char 	ModeNm[20];
		char    FileNm[20];
} tPATs_ModeType;

/* File Extension */
#define FILE_EXT_gMAX 		"gMX" /* g_Max FIle extenstion */
#define FILE_EXT_gMin 		"gmn" /* g_Min FIle extenstion */

//#define FILE_EXT_CHK 		"chk" /* g_Min FIle extenstion */

const tPATs_ModeType arrPATs_ModeID[MODE_ID_NUMS] = {
		{  0,   "(md_HOT)"       ,  "HOT"       ,  "zHOT"       },
		{  1,   "(md_WUP)"       ,  "WUP"       ,  "zWUP"       },
		{  2,   "(md_MNL)"       ,  "MNL"       ,  "zMNL"       },
		{  3,   "(md_DN2)"       ,  "DN2"       ,  "zDN2"       },
		{  4,   "(md_DN1)"       ,  "DN1"       ,  "zDN1"       },
		{  5,   "(md_UP1)"       ,  "UP1"       ,  "zUP1"       },
		{  6,   "(md_UP2)"       ,  "UP2"       ,  "zUP2"       },
		{  7,   "(md_UP3)"       ,  "UP3"       ,  "zUP3"       },
		{  8,	"(md_NOR)"       ,  "NOR"       ,  "zNOR"       }, /* use case */
		{  9,	"(md_ECO)"       ,  "ECO"       ,  "zECO"       }, /* use case */
		{ 10,	"(md_ECODN2)"    ,  "ECODN2"    ,  "zECODN2"    },		
		{ 11,	"(md_ECODN1)"    ,  "ECODN1"    ,  "zECODN1"    },		
		{ 12,	"(md_ECOUP1)"    ,  "ECOUP1"    ,  "zECOUP1"    },
		{ 13,	"(md_ECOUP2)"    ,  "ECOUP2"    ,  "zECOUP2"    },
		{ 14,	"(md_ECOUP3)"    ,  "ECOUP3"    ,  "zECOUP3"    },
		{ 15,	"(md_CRZ)"       ,  "CRZ"       ,  "zCRZ"       },
		{ 16,	"(md_CRZUP1)"    ,  "CRZUP1"    ,  "zCRZUP1"    },  
		{ 17,	"(md_CRZUP2)"    ,  "CRZUP2"    ,  "zCRZUP2"    },  
		{ 18,	"(md_BRK1)"	     ,  "BRK1"      ,  "zBRK1"      },
		{ 19,	"(md_BRK2)"	     ,  "BRK2"      ,  "zBRK2"      },
		{ 20,	"(md_HUP1)"	     ,  "HUP1"      ,  "zHUP1"      },
		{ 21,	"(md_HUP2)"	     ,  "HUP2"      ,  "zHUP2"      },
		{ 22,	"(md_HUP3)"	     ,  "HUP3"      ,  "zHUP3"      },
 		{ 23,	"(md_SPTDN2)"    ,  "SPTDN2"    ,  "zSPTDN2"    },		
 		{ 24,	"(md_SPTDN1)"    ,  "SPTDN1"    ,  "zSPTDN1"    },
		{ 25,	"(md_SPT)"       ,  "SPT"       ,  "zSPT"       }, /* use case */
		{ 26,	"(md_SPTUP1)"    ,  "SPTUP1"    ,  "zSPTUP1"    },
		{ 27,	"(md_SPTUP2)"    ,  "SPTUP2"    ,  "zSPTUP2"    },
		{ 28,	"(md_SPTUP3)"    ,  "SPTUP3"    ,  "zSPTUP3"    },
		{ 29,	"(md_SS_XECO)"   ,  "SS_XECO"   ,  "zSS_XECO"   },
		{ 30,	"(md_SS_ECODN2)" ,  "SS_ECODN2" ,  "zSS_ECODN2" },
		{ 31,	"(md_SS_ECODN1)" ,  "SS_ECODN1" ,  "zSS_ECODN1" },
		{ 32,	"(md_SS_ECO)"    ,  "SS_ECO"    ,  "zSS_ECO"    },
		{ 33,	"(md_SS_ECOUP1)" ,  "SS_ECOUP1" ,  "zSS_ECOUP1" },
		{ 34,	"(md_SS_ECOUP2)" ,  "SS_ECOUP2" ,  "zSS_ECOUP2" },
		{ 35,	"(md_SS_ECOUP3)" ,  "SS_ECOUP3" ,  "zSS_ECOUP3" },
		{ 36,	"(md_SS_DN2)"    ,  "SS_DN2"    ,  "zSS_DN2"    },
		{ 37,	"(md_SS_DN1)"    ,  "SS_DN1"    ,  "zSS_DN1"    },
		{ 38,	"(md_SS_NOR)"    ,  "SS_NOR"    ,  "zSS_NOR"    },
		{ 39,	"(md_SS_UP1)"    ,  "SS_UP1"    ,  "zSS_UP1"    },
		{ 40,	"(md_SS_UP2)"    ,  "SS_UP2"    ,  "zSS_UP2"    },
		{ 41,	"(md_SS_UP3)"    ,  "SS_UP3"    ,  "zSS_UP3"    },
		{ 42,	"(md_SS_SPTDN2)" ,  "SS_SPTDN2" ,  "zSS_SPTDN2" },
		{ 43,	"(md_SS_SPTDN1)" ,  "SS_SPTDN1" ,  "zSS_SPTDN1" },
		{ 44,	"(md_SS_SPT)"    ,  "SS_SPT"    ,  "zSS_SPT"    },
		{ 45,	"(md_SS_SPTUP1)" ,  "SS_SPTUP1" ,  "zSS_SPTUP1" },
		{ 46,	"(md_SS_SPTUP2)" ,  "SS_SPTUP2" ,  "zSS_SPTUP2" },
		{ 47,	"(md_SS_SPTUP3)" ,  "SS_SPTUP3" ,  "zSS_SPTUP3" },
		{ 48,	"(md_SS_XSPTDN2)",  "SS_XSPTDN2",  "zSS_XSPTDN2"},
		{ 49,	"(md_SS_XSPTDN1)",  "SS_XSPTDN1",  "zSS_XSPTDN1"},
		{ 50,	"(md_SS_XSPT)"   ,  "SS_XSPT"   ,  "zSS_XSPT"   },
		{ 51,	"(md_SS_XSPTUP1)",  "SS_XSPTUP1",  "zSS_XSPTUP1"},
		{ 52,	"(md_SS_XSPTUP2)",  "SS_XSPTUP2",  "zSS_XSPTUP2"},
		{ 53,	"(md_SS_XSPTUP3)",  "SS_XSPTUP3",  "zSS_XSPTUP3"},
		{ 54,	"(md_L4W)"       ,  "L4W"       ,  "zL4W"       },
		{ 55,	"(md_COLD)"      ,  "COLD"      ,  "zCOLD"      },
		{ 56,	"(md_SNOW)"      ,  "SNOW"      ,  "zSNOW"      },
		{ 57,	"(md_SNOWUP1)"   ,  "SNOWUP1"   ,  "zSNOWUP1"   },	
		{ 58,	"(md_SNOWUP2)"   ,  "SNOWUP2"   ,  "zSNOWUP2"   },	
		{ 59,	"(md_SAND)"      ,  "SAND"      ,  "zSAND"      },
		{ 60,	"(md_SANDUP1)"   ,  "SANDUP1"   ,  "zSANDUP1"   },
		{ 61,	"(md_SANDUP2)"   ,  "SANDUP2"   ,  "zSANDUP2"   },
		{ 62,	"(md_MUD)"       ,  "MUD"       ,  "zMUD"       },
		{ 63,	"(md_MUDUP1)"    ,  "MUDUP1"    ,  "zMUDUP1"    },
		{ 64,	"(md_MUDUP2)"    ,  "MUDUP2"    ,  "zMUDUP2"    },
		{ 65,	"(md_TOWDN2)"    ,  "TOWDN2"    ,  "zTOWDN2"    },
		{ 66,	"(md_TOWDN1)"    ,  "TOWDN1"    ,  "zTOWDN1"    },
		{ 67,	"(md_TOW)"       ,  "TOW"       ,  "zTOW"       },
		{ 68,	"(md_TOWUP1)"    ,  "TOWUP1"    ,  "zTOWUP1"    },
		{ 69,	"(md_TOWUP2)"    ,  "TOWUP2"    ,  "zTOWUP2"    },
		{ 70,	"(md_TOWUP3)"    ,  "TOWUP3"    ,  "zTOWUP3"    },
		{ 71,   "(md_UNKNOWN)"   ,  "Unknown"   ,  "Unknown"    },
	}; 


#define FILTER_EXT_DTA 		"dta"
#define FILTER_EXT_GIL 		"gil"
#define FILTER_EXT_RPT 		"rpt"


#define TITLE_SHI0	"    time   iPAT  ModeID     vsp    tqi    cg   Aps      No    tg ct iShi txt        TqFr ShiPh Ne       Nt       LAccel  LPFAcc TimPos       TimNtPos  Ratio     Shi1One  Shi2Two   Jerk0  g_Max   g_min"

#define TITLE_SHI1	"    time   iPAT  ModeID     vsp    tqi    cg   Aps      No    tg ct iShi txt        TqFr ShiPh Ne       Nt       LAccel  LPFAcc TimPos       PosNum     Gsum   TimNtPos Ratio    Shi1One  Shi2Two   Jerk0  g_Max   g_min"

#define TITLE_SHIGG	"    time   iPAT  ModeID     vsp    tqi    cg   Aps      No    tg ct iShi txt        TqFr ShiPh Ne       Nt       LAccel  LPFAcc TimPos       PosNum     Gsum   TimNtPos Gpos   Ratio     Shi1One  Shi2Two   Jerk0  g_Max   g_min"

#define TITLE_TXT   "    time   iPAT  ModeID    cG tG   vsp    tqi      Aps      No    ct iShi txt        TqFr ShiPh Ne       Nt     LAccel  LPFAcc DiffTime TimPos(%d)%s   posNum    Gavg  NtPos   Gpos   Ratio    Jerk_%dms  Jerk1 (msec)_%dms"

#define REPORT_TXT  "cg tg Shift   APSt APSv  t1     t2     t3     G(SS)   G(SB)   G(SP)   G(dt)     Jerk1 msec  Ntmin    NtMax    Nt(dt)   Nt(SB)   Ne(SB)   NeMax "

#define SEPER_DASH 		144

#endif /* SHIFT_QUALITY_DATA_SORTING */


#if SHIFT_QUALITY_DATA_SORTING /* 2022-11-13 */

#define IGN_BECAUSE_GENENAL 		1 /* Ignored because of General */
#define IGN_BECAUSE_NTMAX_NONE 		2 /* Ignored because of NtMax NONE */
#define IGN_BECAUSE_NTMIN_NONE 		3 /* Ignored because of Ntmin NONE */
#define IGN_BECAUSE_GMAX_NONE 		4 /* Ignored because of GMax NONE */
#define IGN_BECAUSE_GMIN_NONE 		5 /* Ignored because of Gmin NONE */

#define IGN_BECAUSE_NOPAIR 		9 /* Ignored because of No pair, that is SS>SB, SS>SP */
#define IGN_BECAUSE_OT_SS2SP	10 /* Ignored because of Over time SS~SP */
#define IGN_BECAUSE_OT_SP2SF	11 /* Ignored because of Over time SP~SF */


typedef struct _SBTimePosition_ {
	unsigned int Index;
	long SSTime;
	long NtMaxBegin; /* Case Upshift : SB-50msec */
	long NtMaxTime;
	double Gavg0; /* G average for SS~Sb */
	long SBTime;
	long gMx1Begin; /* Defalt Value : before 200 msec */
	long gmn2Begin; /* Defalt Value : before 300 msec */
	long gMx2End; /* Defalt Value : before 300 msec */
	long gmn1End; /* Defalt Value : before 300 msec */
	double Gavg1; /* G average for SB~SP */
	long SPTime;
	long NtminTime;
	long NtminEnd; /* Case Upshift : SP+50msec */
	double Gavg2; /* G average for SP~SF */
	long SFTime; /* Shift Finish Time */
	short ignored; /* SS ~ SP time >= 2000 msec -> ignored */
} tSBtimePos_type;

tSBtimePos_type  gMaxTbl[MAX_TABLE_SIZ]; /* gMax Start Time Position for Jerk1 Calculation */



typedef struct _SBdecision_ {
	short FixCount;
	unsigned long long SBTime;
	unsigned long long gmMxBegin; /* example, Defalt Value : before 200 msec */
} tSBdec_type;

typedef struct _SPdecision_ {
	short FixCount;
	unsigned long long SPTime;
	unsigned long long gmMxBegin; /* example, Defalt Value : before 200 msec */
} tSPdec_type;


typedef struct _NtMaxdecision_ {
	short FixCount;
	unsigned long long NtMaxTime;
	unsigned long long gmMxBegin; /* example, Defalt Value : before 200 msec */
} tNtMaxdec_type;

typedef struct _Ntmindecision_ {
	short FixCount;
	unsigned long long NtminTime;
	unsigned long long gmMxBegin; /* example, Defalt Value : before 200 msec */
} tNtmindec_type;


#define WIDE_CHAR_SYMBOL 	0

typedef struct _graphDB_ {
	int  gearIdx;
	char gearTxt[10];
	int  value;
	int  YvalNum;
	#if WIDE_CHAR_SYMBOL
	wchar_t symbol[2];
	#else
	char symbol[2];
	#endif
} graphData_type;


#if WIDE_CHAR_SYMBOL
static wchar_t gSymbol[20][2];

const char	mbSym[][2] = {
		"",  // 0
		"■" , // 2
		"◈" , // 9
		"♡" , // 10
		"♤" , // 11
		"●" , // 3
		"◆" , // 4
		"▲" , // 5
		"♠" , // 6
		"♥" , // 7
		"♣" , // 8
		"○" , // 12
		"◇" , // 13
		"★", // 1
		"△" , // 14
		"α" , // 15
		"β" , // 16
		"γ" , // 17
		"δ" , // 18
		""
	};
#else
const char	gSymbol[20][2] = {
		"0",  // 0
		"a", // 1
		"b", // 2
		"c", // 3
		"d", // 4
		"e", // 5
		"f", // 6
		"g", // 7
		"h", // 8
		"i", // 9
		"j", // 10
		"k", // 11
		"m", // 12
		"n", // 13
		"o", // 14
		"p", // 15
		"q", // 16
		"r", // 17
		"s", // 18
		""
	};
#endif


				 
void mb2wc(void)
{
#if WIDE_CHAR_SYMBOL
	int length, temp;
	int ii;
	char *loc = NULL;

	/* --------------------------------------------------------------------------
	mbtowc() ? 멀티바이트 문자를 와이드 문자로 변환
	wcslen() ? 와이드 문자 스트링 길이 계산
	wcrtomb() ? 멀티바이트 문자로 와이드 문자 변환(재시작 가능)
	wcstombs() ? 멀티바이트 스트링으로 와이드 문자 스트링 변환
	wcsrtombs() ? 와이드 문자 스트링을 멀티바이트 스트링으로 변환(재시작 가능)
	---------------------------------------------------------------------------- */

	/* initialize internal state variable */
	//temp = mbtowc(gSymbol, NULL, 0); 		   
	memset(gSymbol, 0x00, 20*sizeof(wchar_t) );


	//loc = setlocale(LC_ALL, "ko_KR.utf8" );
	//loc = setlocale(LC_ALL, "ko_KR.UTF-8");
	loc = setlocale(LC_ALL, "");
	//loc = setlocale(LC_ALL, "korean" ); 
	//loc = setlocale(LC_ALL,".949");
	//loc = setlocale(LC_ALL,"Korean_Korea.65001");
	//loc = setlocale(LC_ALL,"Korean_Korea.949");
	//loc = setlocale(LC_ALL,"en_US.UTF-8");//	CP_UTF8
	//loc = setlocale(LC_ALL, "/QSYS.LIB/EN_US.LOCALE" );
	//loc = setlocale(LC_ALL, "");
	if( NULL == loc )
	{
		fprintf(stderr,"set locale failed.. [%s] \r\n\n", loc );
	}
	else
	{
		//fprintf(stderr,"LC_ALL = [%s] \n", loc);
		//printf("LC_CTYPE = [%s]\n", setlocale(LC_CTYPE, NULL));
	}

	/* Set string to point to a multibyte character. */
	for(ii=0; ii<20; ii++)
	{
		length = mblen(mbSym[ii], MB_CUR_MAX);
		temp = mbtowc(gSymbol[ii], mbSym[ii], length);
		//temp = mbstowcs(gSymbol[ii], mbSym[ii], length); 
		gSymbol[ii][1] = L'\0';
		//fwprintf(stderr, L"wide character string: %d/%d -> [%4ls] \n",length, temp, gSymbol[ii] );
	}

	//fprintf(outfile,"■★●◆▲♠♥♣ ◎※℃∇ αβγδ ◈○◇□△♡♤\n");
#endif
}


short apsTableIndex(void)
{
	short aps_index = APS_TABLE_NUM;
	short kk = 0;

	for(kk=0; kk<APS_TABLE_NUM; kk++)
	{
		if( ApsTble[kk] <= 0.0f )
		{
			aps_index = kk;
			break;
		}
	}

	fprintf(stderr,">>APS Table Nums   : %2d (", aps_index );
	for(kk=0; kk<aps_index-1; kk++)
	{
		fprintf(stderr,"%d, ", (int)ApsTble[kk] );
	}
	fprintf(stderr,"%d) \n", (int)ApsTble[kk] );
	
	return aps_index;
}


short vsKPHTableIndex(void)
{
	short vs_index = VS_TABLE_NUM;
	short kk = 0;

	for(kk=0; kk<VS_TABLE_NUM; kk++)
	{
		if( VSkphTblDN[kk] <= 0.0f )
		{
			vs_index = kk;
			break;
		}
	}

	fprintf(stderr,">>VS Table Nums    : %2d (", vs_index );
	for(kk=0; kk<vs_index-1; kk++)
	{
		fprintf(stderr,"%d, ", VSkphTblDN[kk] );
	}
	fprintf(stderr,"%d) \n", VSkphTblDN[kk]);	
	
	return vs_index;
}


typedef enum _gear_shift_type_ {
	GEAR_SHI_NONE = 0,
		
	PWR_ON_UP_SHIFT = 1,
	PWR_ON_DOWN_SHIFT,
	
	POWER_OFF_UP_SHIFT ,
	POWER_OFF_DOWN_SHIFT ,
	
	GEAR_PWRON_MAN , // GEAR_POWER_ON_MANUAL 
	GEAR_NEAR2STOP ,
	GEAR_RETURN_SHI,
	
	GEAR_P,
	GEAR_N,
	GEAR_R,
	GEAR_D,
	GEAR_OTHER     ,

	GEAR_MAX_NUM = 0x7f
} eGearShi_enumType;

typedef struct _gearShiKind_ {
	short 	eGearMode;
	char 	szModeTxt[10];
	short 	iStart1;
	short 	iEnd1;
	short 	iStart2;
	short 	iEnd2;
	short 	iStart3;
	short 	iEnd3;
	short 	iStart4;
	short 	iEnd4;
	unsigned long long lSum;
} tGearShi_type;

tGearShi_type gMode[13] = {
	{ GEAR_SHI_NONE,		"(****)",	  0, -1,   0, -1,   0, -1,  0, -1, 0ULL },
	{ PWR_ON_UP_SHIFT, 		"(s**Z)",    15, 21,   0, -1,   0, -1,  0, -1, 0ULL }, /* Power On	(s**Z), 15~21  */
	{ PWR_ON_DOWN_SHIFT,	"(s**Z)",	 36, 42,  57, 73,	0, -1,  0, -1, 0ULL }, /* Power On_Down shift (s**Z), 36~42,	57~73 */
	{ POWER_OFF_UP_SHIFT, 	"(s**S)",    22, 28,   0, -1,   0, -1,  0, -1, 0ULL }, /* Power Off	(s**S), 22~28,  43~49,  74~79 */
	{ POWER_OFF_DOWN_SHIFT,	"(s**S)",	 43, 49,  74, 79,	0, -1,  0, -1, 0ULL }, /* Power Off (s**S), 22~28,	43~49,	74~79 */
	{ GEAR_PWRON_MAN, 		"(s**MZ)",   29, 35,  50, 56,  80, 85,  0, -1, 0ULL }, /* Power On Manual (s**MZ) 29~35, 80~85, 50~56 */
	{ GEAR_NEAR2STOP,		"(s**R)",    86, 98,   0, -1,   0, -1,  0, -1, 0ULL }, /* Near to Stop (s**R) 86~98 */
	{ GEAR_RETURN_SHI,		"(s***)",    99,123,   0, -1,   0, -1,  0, -1, 0ULL }, /* Return Shift (s???) 99~123 */
	{ GEAR_P, 				"(sP)", 	  9,  9,   0, -1,	0, -1,	0, -1, 0ULL }, /* Parking (sP) */
	{ GEAR_N,				"(sN)", 	  0,  0,   0, -1,	0, -1,	0, -1, 0ULL }, /* N (sN) */
	{ GEAR_R,				"(sR)", 	 10, 10,   0, -1,	0, -1,	0, -1, 0ULL }, /* R (sR) */
	{ GEAR_D,				"(s*)", 	  1,  8,   0, -1,	0, -1,	0, -1, 0ULL }, /* D (s?) 1~8 */
	{ GEAR_OTHER,			"(s@^#)",	124,160,  11, 14,   0, -1,  0, -1, 0ULL }, /* Return Shift (s???) 99~123 */
};


typedef struct _SQData_pair_ {
	unsigned int SSnum;
	unsigned int SBnum;
	unsigned int SPnum;
	unsigned int SFnum;
	
	unsigned int SStot;
	unsigned int SBtot;
	unsigned int SPtot;
	unsigned int SFtot;

	unsigned int SSend;
	unsigned int SBend;
	unsigned int SPend;
	unsigned int SFend;
} tSQData_PairCheck_type;


int tempFileDeleteIt(short YesNo, short iModeID, unsigned int iSBchk);

/* -------------------------------------------- */
/* qsort() down sorting ----------------------- */
/* -------------------------------------------- */
int doubleCompDown(const void *a, const void *b) 
{
	tminMax_type *pa = (tminMax_type *)a;
	tminMax_type *pb = (tminMax_type *)b;

	if( pa->mValue < pb->mValue )
		return 1;
	else if( pa->mValue > pb->mValue ) 
		return -1;
	else
		return 0;  
}


/* -------------------------------------------- */
/* qsort() Up sorting ----------------------- */
/* -------------------------------------------- */
int doubleCompUp(const void *a, const void *b) 
{
	tminMax_type *pa = (tminMax_type *)a;
	tminMax_type *pb = (tminMax_type *)b;

	if( pa->mValue < pb->mValue )
		return -1;
	else if( pa->mValue > pb->mValue ) 
		return 1;
	else
		return 0;  
}


int intCompUp(const void *a,const void *b) 
{
	int ia = *(int *)a;
	int ib = *(int *)b;

	if( ia < ib )
		return -1;
	else if( ia > ib ) 
		return 1;
	else
		return 0;  
}


int QsortCompareUp(const void *a, const void *b) 
{
    graphData_type *pa = (graphData_type*)a;
    graphData_type *pb = (graphData_type*)b;

    if(pa->value < pb->value)
        return -1;
    else if(pa->value > pb->value)
        return 1;
    else
	    return 0;
}


int QsortCompare(const void *a, const void *b)  /* 내림차순 정렬, from small data ~ */
{
    graphData_type *pa = (graphData_type*)a;
    graphData_type *pb = (graphData_type*)b;

    if(pa->value < pb->value)
        return 1;
    else if(pa->value > pb->value)
        return -1;
    else
	    return 0;
}


unsigned long long Find_GearMode( unsigned long long *gShift, short iChoice, unsigned int iavgTime )
{
	short ii=0;
	unsigned int lTotalNums = 0;
	unsigned int lPowerOn_UpNums = 0;
	unsigned int lPowerOn_DownNums = 0;
	unsigned int lPowerOff_UpNums = 0;
	unsigned int lPowerOff_DownNums = 0;
	unsigned int lPwrOnManualNums = 0;
	unsigned int lNear2StopNums = 0;
	unsigned int lReturnShiNums = 0;
	unsigned int lGearPNums = 0;
	unsigned int lGearNNums = 0;
	unsigned int lGearRNums = 0;
	unsigned int lGearDNums = 0;
	unsigned int lotherNums = 0;
	
	lTotalNums = 0U;
	for(ii=0; ii<GEAR_SHI_NUMS; ii++) lTotalNums += gShift[ii];
	
	lPowerOn_UpNums = 0U;
	for(ii=gMode[PWR_ON_UP_SHIFT].iStart1; ii<=gMode[PWR_ON_UP_SHIFT].iEnd1; ii++) lPowerOn_UpNums += gShift[ii];
	for(ii=gMode[PWR_ON_UP_SHIFT].iStart2; ii<=gMode[PWR_ON_UP_SHIFT].iEnd2; ii++) lPowerOn_UpNums += gShift[ii];
	for(ii=gMode[PWR_ON_UP_SHIFT].iStart3; ii<=gMode[PWR_ON_UP_SHIFT].iEnd3; ii++) lPowerOn_UpNums += gShift[ii];
	gMode[PWR_ON_UP_SHIFT].lSum = lPowerOn_UpNums;

	lPowerOn_DownNums = 0U;
	for(ii=gMode[PWR_ON_DOWN_SHIFT].iStart1; ii<=gMode[PWR_ON_DOWN_SHIFT].iEnd1; ii++) lPowerOn_DownNums += gShift[ii];
	for(ii=gMode[PWR_ON_DOWN_SHIFT].iStart2; ii<=gMode[PWR_ON_DOWN_SHIFT].iEnd2; ii++) lPowerOn_DownNums += gShift[ii];
	for(ii=gMode[PWR_ON_DOWN_SHIFT].iStart3; ii<=gMode[PWR_ON_DOWN_SHIFT].iEnd3; ii++) lPowerOn_DownNums += gShift[ii];
	gMode[PWR_ON_DOWN_SHIFT].lSum = lPowerOn_DownNums;
	

	lPowerOff_UpNums = 0U;
	for(ii=gMode[POWER_OFF_UP_SHIFT].iStart1; ii<=gMode[POWER_OFF_UP_SHIFT].iEnd1; ii++) lPowerOff_UpNums += gShift[ii];
	for(ii=gMode[POWER_OFF_UP_SHIFT].iStart2; ii<=gMode[POWER_OFF_UP_SHIFT].iEnd2; ii++) lPowerOff_UpNums += gShift[ii];
	for(ii=gMode[POWER_OFF_UP_SHIFT].iStart3; ii<=gMode[POWER_OFF_UP_SHIFT].iEnd3; ii++) lPowerOff_UpNums += gShift[ii];
	gMode[POWER_OFF_UP_SHIFT].lSum = lPowerOff_UpNums;


	lPowerOff_DownNums = 0U;
	for(ii=gMode[POWER_OFF_DOWN_SHIFT].iStart1; ii<=gMode[POWER_OFF_DOWN_SHIFT].iEnd1; ii++) lPowerOff_DownNums += gShift[ii];
	for(ii=gMode[POWER_OFF_DOWN_SHIFT].iStart2; ii<=gMode[POWER_OFF_DOWN_SHIFT].iEnd2; ii++) lPowerOff_DownNums += gShift[ii];
	for(ii=gMode[POWER_OFF_DOWN_SHIFT].iStart3; ii<=gMode[POWER_OFF_DOWN_SHIFT].iEnd3; ii++) lPowerOff_DownNums += gShift[ii];
	gMode[POWER_OFF_DOWN_SHIFT].lSum = lPowerOff_DownNums;

	
	lPwrOnManualNums = 0U;
	for(ii=gMode[GEAR_PWRON_MAN].iStart1; ii<=gMode[GEAR_PWRON_MAN].iEnd1; ii++) lPwrOnManualNums += gShift[ii];
	for(ii=gMode[GEAR_PWRON_MAN].iStart2; ii<=gMode[GEAR_PWRON_MAN].iEnd2; ii++) lPwrOnManualNums += gShift[ii];
	for(ii=gMode[GEAR_PWRON_MAN].iStart3; ii<=gMode[GEAR_PWRON_MAN].iEnd3; ii++) lPwrOnManualNums += gShift[ii];
	gMode[GEAR_PWRON_MAN].lSum = lPwrOnManualNums;
	
	lNear2StopNums = 0U;
	for(ii=gMode[GEAR_NEAR2STOP].iStart1; ii<=gMode[GEAR_NEAR2STOP].iEnd1; ii++) lNear2StopNums += gShift[ii];
	for(ii=gMode[GEAR_NEAR2STOP].iStart2; ii<=gMode[GEAR_NEAR2STOP].iEnd2; ii++) lNear2StopNums += gShift[ii];
	for(ii=gMode[GEAR_NEAR2STOP].iStart3; ii<=gMode[GEAR_NEAR2STOP].iEnd3; ii++) lNear2StopNums += gShift[ii];
	gMode[GEAR_NEAR2STOP].lSum = lNear2StopNums;
	
	lReturnShiNums = 0U;
	for(ii=gMode[GEAR_RETURN_SHI].iStart1; ii<=gMode[GEAR_RETURN_SHI].iEnd1; ii++) lReturnShiNums += gShift[ii];
	for(ii=gMode[GEAR_RETURN_SHI].iStart2; ii<=gMode[GEAR_RETURN_SHI].iEnd2; ii++) lReturnShiNums += gShift[ii];
	for(ii=gMode[GEAR_RETURN_SHI].iStart3; ii<=gMode[GEAR_RETURN_SHI].iEnd3; ii++) lReturnShiNums += gShift[ii];
	gMode[GEAR_RETURN_SHI].lSum = lReturnShiNums;

	lGearPNums = 0U;
	for(ii=gMode[GEAR_P].iStart1; ii<=gMode[GEAR_P].iEnd1; ii++) lGearPNums += gShift[ii];
	for(ii=gMode[GEAR_P].iStart2; ii<=gMode[GEAR_P].iEnd2; ii++) lGearPNums += gShift[ii];
	for(ii=gMode[GEAR_P].iStart3; ii<=gMode[GEAR_P].iEnd3; ii++) lGearPNums += gShift[ii];
	gMode[GEAR_P].lSum = lGearPNums;

	lGearNNums = 0U;
	for(ii=gMode[GEAR_N].iStart1; ii<=gMode[GEAR_N].iEnd1; ii++) lGearNNums += gShift[ii];
	for(ii=gMode[GEAR_N].iStart2; ii<=gMode[GEAR_N].iEnd2; ii++) lGearNNums += gShift[ii];
	for(ii=gMode[GEAR_N].iStart3; ii<=gMode[GEAR_N].iEnd3; ii++) lGearNNums += gShift[ii];
	gMode[GEAR_N].lSum = lGearNNums;

	lGearRNums = 0U;
	for(ii=gMode[GEAR_R].iStart1; ii<=gMode[GEAR_R].iEnd1; ii++) lGearRNums += gShift[ii];
	for(ii=gMode[GEAR_R].iStart2; ii<=gMode[GEAR_R].iEnd2; ii++) lGearRNums += gShift[ii];
	for(ii=gMode[GEAR_R].iStart3; ii<=gMode[GEAR_R].iEnd3; ii++) lGearRNums += gShift[ii];
	gMode[GEAR_R].lSum = lGearRNums;

	lGearDNums = 0U;
	for(ii=gMode[GEAR_D].iStart1; ii<=gMode[GEAR_D].iEnd1; ii++) lGearDNums += gShift[ii];
	for(ii=gMode[GEAR_D].iStart2; ii<=gMode[GEAR_D].iEnd2; ii++) lGearDNums += gShift[ii];
	for(ii=gMode[GEAR_D].iStart3; ii<=gMode[GEAR_D].iEnd3; ii++) lGearDNums += gShift[ii];
	gMode[GEAR_D].lSum = lGearDNums;

	lotherNums = 0U;
	for(ii=gMode[GEAR_OTHER].iStart1; ii<=gMode[GEAR_OTHER].iEnd1; ii++) lotherNums += gShift[ii];
	for(ii=gMode[GEAR_OTHER].iStart2; ii<=gMode[GEAR_OTHER].iEnd2; ii++) lotherNums += gShift[ii];
	for(ii=gMode[GEAR_OTHER].iStart3; ii<=gMode[GEAR_OTHER].iEnd3; ii++) lotherNums += gShift[ii];
	gMode[GEAR_OTHER].lSum = lotherNums;


	fprintf(stderr,"  Total Quality Shift Records -: %9u lines, %6.1f min \n", lTotalNums, (double)(lTotalNums*iavgTime)/1000.0/60 );
	fprintf(stderr,"    Pwr On Upshift--%-8s   : %9llu lines, %6.1f min \n", gMode[PWR_ON_UP_SHIFT].szModeTxt, gMode[PWR_ON_UP_SHIFT].lSum, (double)(gMode[PWR_ON_UP_SHIFT].lSum*iavgTime)/1000.0/60 );
	fprintf(stderr,"    Pwr On Downshift--%-8s : %9llu lines, %6.1f min \n", gMode[PWR_ON_DOWN_SHIFT].szModeTxt, gMode[PWR_ON_DOWN_SHIFT].lSum, (double)(gMode[PWR_ON_DOWN_SHIFT].lSum*iavgTime)/1000.0/60 );
	fprintf(stderr,"    Pwr Off Upshift--%-8s  : %9llu lines, %6.1f min \n", gMode[POWER_OFF_UP_SHIFT].szModeTxt, gMode[POWER_OFF_UP_SHIFT].lSum, (double)(gMode[POWER_OFF_UP_SHIFT].lSum*iavgTime)/1000/60 );
	fprintf(stderr,"    Pwr Off Downshift--%-8s: %9llu lines, %6.1f min \n", gMode[POWER_OFF_DOWN_SHIFT].szModeTxt, gMode[POWER_OFF_DOWN_SHIFT].lSum, (double)(gMode[POWER_OFF_DOWN_SHIFT].lSum*iavgTime)/1000/60 );
	fprintf(stderr,"    Power On Manual--%-8s  : %9llu lines, %6.1f min \n", gMode[GEAR_PWRON_MAN].szModeTxt, gMode[GEAR_PWRON_MAN].lSum, (double)(gMode[GEAR_PWRON_MAN].lSum*iavgTime)/1000/60 );
	fprintf(stderr,"    Near2Stop Shift--%-8s  : %9llu lines, %6.1f min \n", gMode[GEAR_NEAR2STOP].szModeTxt, gMode[GEAR_NEAR2STOP].lSum, (double)(gMode[GEAR_NEAR2STOP].lSum*iavgTime)/1000/60 );
	fprintf(stderr,"    Return Shift--%-8s     : %9llu lines, %6.1f min \n", gMode[GEAR_RETURN_SHI].szModeTxt, gMode[GEAR_RETURN_SHI].lSum, (double)(gMode[GEAR_RETURN_SHI].lSum*iavgTime)/1000/60 );
	fprintf(stderr,"    P Gear status--%-8s    : %9llu lines, %6.1f min \n", gMode[GEAR_P].szModeTxt, gMode[GEAR_P].lSum, (double)(gMode[GEAR_P].lSum*iavgTime)/1000/60 );
	fprintf(stderr,"    N Gear status--%-8s    : %9llu lines, %6.1f min \n", gMode[GEAR_N].szModeTxt, gMode[GEAR_N].lSum, (double)(gMode[GEAR_N].lSum*iavgTime)/1000/60 );
	fprintf(stderr,"    R Gear status--%-8s    : %9llu lines, %6.1f min \n", gMode[GEAR_R].szModeTxt, gMode[GEAR_R].lSum, (double)(gMode[GEAR_R].lSum*iavgTime)/1000/60 );
	fprintf(stderr,"    D Gear status--%-8s    : %9llu lines, %6.1f min \n", gMode[GEAR_D].szModeTxt, gMode[GEAR_D].lSum, (double)(gMode[GEAR_D].lSum*iavgTime)/1000/60 );
	fprintf(stderr,"    Others Gear -- %-8s    : %9llu lines, %6.1f min \n", gMode[GEAR_OTHER].szModeTxt, gMode[GEAR_OTHER].lSum, (double)(gMode[GEAR_OTHER].lSum*iavgTime)/1000.0/60 );
	fprintf(stderr,"----------------------------------------------------------------------------------\n" );

	if(iChoice>GEAR_SHI_NONE && iChoice<=GEAR_OTHER)
		return(gMode[iChoice].lSum);
	else
		return 0;
}

double LPFilter(double factor, double fltLAcc, double currLAcc)
{
	double dLPF;

	/* --------------------------------------- */
	/* Low Pass Filter ----------------------- */
	/* --------------------------------------- */
	dLPF = (factor*currLAcc) + (1.0 - factor)*fltLAcc;
	return dLPF;
}




unsigned int ShiftQualData(short aiPATs05, int iShiftType, int shiDir03, short SBposDecision, tSQData_PairCheck_type *SPoint, unsigned int *SBswingcnt)
{
	FILE *shiFile=NULL;

	unsigned short iINPUT_NUMS = QUAL_TSV_DATA_ITEM_NUM;
	sqd_type sq[2];
	short	useInputCase = USE_IN_CASE1_17;
	unsigned int NGformatMODECnt = 0;
	unsigned int NGformatGEARCnt = 0;
	unsigned int NGformatCG = 0;
	unsigned int NGformatTG = 0;
	unsigned int iErrorVSP = 0;
	unsigned int iErrorAPS = 0;
	unsigned int iErrorLAcc = 0;
	unsigned int iErrorRPM = 0;
	unsigned int iErrorTemp = 0;
	
	unsigned int iPreShTime = 0;
	double avgTime = 0.0f;
	unsigned long long avgCount = 0ULL;
	unsigned int iavgTime = 0;
	char shift_out[MAX_CHARS*LENGTH_OF_FILENAME+1];
	unsigned int ii=0;
	short iItemCurOK = 0; /* 1: OK, 0: NG */
	short iItemPreOK = 0;
	short iSave = 0, iFirstItem = 0;
	short iShift = 0;

	double LPFilteredLAcc = 0.0f;
	double g_SBtime = -1.0f;
	double g_SPtime = -1.0f;
	double g_SFtime = -1.0f;

	double g_NtMaxtime = -1.0f;
	double g_Ntmintime = -1.0f;

	double gearShift1One = -1.0f; //, pre_gearShift1One = -1.0f;
	double gearShift2Two = -1.0f;

	/* line inputted from file */
	char QualData[QUAL_DATA_MAX_SIZE] = {0,}; 
	int result;
	unsigned long long RecordCnt=0ULL;

	unsigned long long chkPATs_ModeID[MODE_ID_NUMS] = {0ULL,};
	unsigned int totChkModeID = 0;
	unsigned long long gearShift[GEAR_SHI_NUMS] = {0ULL,};

	unsigned int iNGcount = 0;
	unsigned int iOKcount = 0;
	unsigned int iSortCount = 0;

	double gearRatio = 0.0f;
	int isNaming = 0;
	char sTimePos[LEN_POS] = TXT_UNKNOWN;
	char sTimeNtPos[LEN_POS] = TXT_UNKNOWN;
	short isFindSS = 1;
	int curPreGear = -1, tgtPreGear = 1;
	double fJerk0 = 0.0f;
	double preLAcc = 0.0f;
	int curSFPreGear = -1, tgtSFPreGear = 1;
	unsigned int DiffTime = 0;
	unsigned int preTime = 0;
	unsigned int iSScount = 0U;
	unsigned int iSBcount = 0U;
	unsigned int iSPcount = 0U;
	unsigned int iSFcount = 0U;

	unsigned int iSxIgnCnt = 0;
	
	unsigned int iNtMaxcount = 0U;
	unsigned int iNtmincount = 0U;

	unsigned int iSBswingCnt = 0U;
	unsigned int iSPswingCnt = 0U;
	unsigned int iNtMaxswingCnt = 0U;
	unsigned int iNtminswingCnt = 0U;
	
	short iOnce_SS = 0; /* */
	short iOnce_SB = 1; /* Shift Begin */
	short iOnce_SP = 1; /* Synchronizing Point */
	short iOnce_SF = 0; /* Shift Finish */

	short iExistSBpoint = 0;
	short iExistSPpoint = 0;

	short iOnce_NtMax = 1; /* NtMax */
	short iOnce_Ntmin = 1; /* Ntmin */

	double MaxAcc = 0.0f;
	double minAcc = GMAX_RVALUE;
	short iAPSNum = APS_TABLE_NUM;
	short ivsKPHNum = VS_TABLE_NUM;

	tSBdec_type SBdecision[SB_DECISION_NUM];
	unsigned short iSBdecCnt = 0;
	short iSBpntFix = 0;
	short iSBstart = 0;
	short iSBTimeSum = 0;
	short ide=0;
	unsigned int igMxTime = 0;
	long avg_t1=0ULL, avg_t2 = 0ULL, avg_t3 = 0ULL;
	long dif_t1=0ULL, dif_t2 = 0ULL, dif_t3 = 0ULL;

	tSPdec_type SPdecision[SP_DECISION_NUM];
	unsigned short iSPdecCnt = 0;
	short iSPstart = 0;
	short iSPTimeSum = 0;
	short iSPpntFix = 0;
	int ignoredRecord = 0;

	tNtMaxdec_type NtMaxdecision[SP_DECISION_NUM];
	unsigned short iNtMaxdecCnt = 0;
	short iNtMaxpntFix = 0;
	short iNtMaxstart = 0;
	short iNtMaxTimeSum = 0;

	tNtmindec_type Ntmindecision[SP_DECISION_NUM];
	unsigned short iNtmindecCnt = 0;
	short iNtminpntFix = 0;
	short iNtminstart = 0;
	short iNtminTimeSum = 0;
	short is2File = 0;
	short curSmPreGear = -1;
	short tgtSmPreGear = 0;
	unsigned int iCurrTime = 0;


	memset(QualData, 0x00, QUAL_DATA_MAX_SIZE*sizeof(char) );
	memset(sq, 0x00, sizeof(sq) );
	memset(chkPATs_ModeID, 0x00, sizeof(chkPATs_ModeID) );
	memset(gearShift, 0x00, GEAR_SHI_NUMS*sizeof(unsigned long long) );

	memset(SBdecision, 0x00, SB_DECISION_NUM*sizeof(tSBdec_type) );
	memset(SPdecision, 0x00, SP_DECISION_NUM*sizeof(tSPdec_type) );
	memset(gMaxTbl, 0x00, MAX_TABLE_SIZ*sizeof(tSBtimePos_type) );

	memset(NtMaxdecision, 0x00, SB_DECISION_NUM*sizeof(tNtMaxdec_type) );
	memset(Ntmindecision, 0x00, SP_DECISION_NUM*sizeof(tNtmindec_type) );


	iSBdecCnt  = 0;
	iSBstart   = 0;
	iSBTimeSum = 0;
	iSBpntFix  = 0;

	iOnce_SS	= 0;
	iOnce_SB	= TRUE;
	iOnce_SP	= TRUE;

	iOnce_NtMax = TRUE; /* NtMax */
	iOnce_Ntmin = TRUE; /* Ntmin */

	iSPdecCnt  = 0;
	iSPstart   = 0;
	iSPTimeSum = 0;
	iSPpntFix  = 0;

	iSortCount  = 0;
	iFirstItem  = 0;

	g_SBtime    = -1.0f;
	g_SPtime    = -1.0f;
	g_SFtime    = -1.0f;

	g_NtMaxtime = -1.0f;
	g_Ntmintime = -1.0f;

	RecordCnt  = 0ULL;
	iNGcount   = 0ULL;
	iOKcount   = 0ULL;
	iSortCount = 0ULL;

	avgCount   = 0ULL;


	if( -1==aiPATs05 )
	{
		fprintf(stderr,"\n");
		fprintf(stderr,">>PATs-ModeID -> Unknown ModeID, check please... \r\n" );
		// FAIL
		AllFilesClosed();
		exit(0);
		return 0;
	}

	if( fAPSpwrLvl < 0.0f ) 
	{
		fAPSpwrLvl = APS_PWR_ON_VAL; /* default value setting */
		fprintf(stderr,">>APS default lvl : %.2lf%% -- default(3\%~5\%) \n", fAPSpwrLvl );
	}

	iAPSNum   = apsTableIndex();
	//ivsKPHNum = vsKPHTableIndex();


	/* ===================================================================================== */
	/* get file  */
	//fprintf(stderr,"\n");
	//fprintf(stderr,">>ModeID %s Sorting... \n", arrPATs_ModeID[aiPATs05].ModeID );
	fprintf(stderr,">>Shift Type       : %s \n", (iShiftType==SHI_PWR_ON?"PWR On":(iShiftType==SHI_PWR_OFF?"PWR Off":(iShiftType==SHI_STATIC?"Static":(iShiftType==SHI_N_STOP_DN?"Stop Dn":"Unknown")))) );
	fprintf(stderr,">>Shift Direction  : Shift %s \n", (shiDir03==SHIFT_UP?"UP":(shiDir03==SHIFT_DN?"DOWN": \
		(shiDir03==SHIFT_SKIP_DN?"SkipDown":(shiDir03==SHIFT_SKIP_UP?"SkipUp":"Unknown"))) ));			

	/* ===================================================================================== */

	memset(shift_out, 0x00, sizeof(shift_out)); // 2022.11.22
	strcpy(shift_out, shift_file);
	if( (aiPATs05>=0 && aiPATs05<MODE_ID_NUMS-1) && (strlen(shift_out)>0) )
	{
		isNaming = 0;
		for(ii=strlen(shift_out)-1; ii>0; ii--)
		{
			if( shift_out[ii]=='.' ) 
			{
				shift_out[ii+1] = '\0';
				strcat(shift_out, arrPATs_ModeID[aiPATs05].ModeNm);
				isNaming = 1;
				break;
			}
		}

		if( 0==isNaming )
		{
			strcat(shift_out, ".");
			strcat(shift_out, arrPATs_ModeID[aiPATs05].ModeNm);
		}

		// mkdir OK
		// File extension : *.ECO, *.HOT, *SPT
		if( NULL == (shiFile = fopen( shift_out, "wb")) )	
		{
			// FAIL
			printf("\n\nCan not create output file(%s).. check plz... \n\n", shift_out );
			AllFilesClosed();
			exit(0);
		}
	}	
	/* ===================================================================================== */


#if SAVEMODE
	if(shiFile)
	{
		fprintf(shiFile, TITLE_SHI0 "\n");
	}
#endif


	iNGcount = 0;
	iOKcount = 0;

	iSScount = 0;
	iSBcount = 0;
	iSPcount = 0;

	iNtMaxcount = 0;
	iNtmincount = 0;


	fprintf(stderr,">>Sorting and analyzing SQ Data... Format...  (%d) \n", useInputCase);

	do
	{
		unsigned int i=0;

		/* Read a line from input file. */
		memset( QualData, 0x00, sizeof(QualData) );

		if( NULL == fgets( QualData, QUAL_DATA_MAX_SIZE, inpfile ) )
		{
			//fclose(shiFile);
			//fprintf(stderr,">>PATs-ModeID %s Sorting 1st step completed!!! \r\n", arrPATs_ModeID[aiPATs05].ModeID ); 			
			break;
		}

		RecordCnt++;

		/* Remove carriage return/line feed at the end of line. */
		i = strlen(QualData);
		if(i >= QUAL_DATA_MAX_SIZE)
		{
			fprintf(stderr,"ShiftLine%6lld :ERROR: Not enough Buffer length(%d) \r\n", RecordCnt, i );
		}

		if (--i > 0)
		{
			if (QualData[i] == '\n') QualData[i] = '\0';
			if (QualData[i] == '\r') QualData[i] = '\0'; 

			/* Input Case1 : 17 items */
			if( USE_IN_CASE1_17==useInputCase )
			{
				result = sscanf(QualData, SQD_INFORMAT,
							&sq[0].Time01,    &sq[0].OTS02,     &sq[0].VSP03,     &sq[0].TqStnd04,   &sq[0].iPATs05,   &sq[0].EngTemp06, 
							&sq[0].tqi07,     &sq[0].curGear08, &sq[0].APS09,     &sq[0].No10,       &sq[0].tgtGear11, &sq[0].ShiTy12, 
							&sq[0].TqFr13,    &sq[0].ShiPh14,   &sq[0].Ne15,      &sq[0].Nt16,       &sq[0].LAcc17 );

				iINPUT_NUMS = QUAL_TSV_CASE1_17_ITEM_NUM;
			}
			/* Input Case2 : 15 items */
			else if( USE_IN_CASE2_15==useInputCase )
			{
				result = sscanf(QualData, SQD_INFMT2,
							&sq[0].Time01,    &sq[0].TqStnd04,  &sq[0].OTS02,      &sq[0].EngTemp06,  &sq[0].VSP03,   &sq[0].iPATs05,
							&sq[0].tgtGear11, &sq[0].APS09,     &sq[0].curGear08,  &sq[0].ShiPh14,    &sq[0].LAcc17,  &sq[0].ShiTy12,
							&sq[0].Nt16,      &sq[0].No10,      &sq[0].Ne15 );  


				iINPUT_NUMS = QUAL_TSV_CASE2_15_ITEM_NUM;
			}
			else if( USE_IN_NEW_CASE_3_15==useInputCase )
			{
				result = sscanf(QualData, SQD_INFMT3_15,
							&sq[0].Time01,    &sq[0].OTS02,     &sq[0].VSP03,     &sq[0].iPATs05,    &sq[0].EngTemp06,  &sq[0].NetEng_Acor,
							&sq[0].curGear08, &sq[0].APS09,     &sq[0].No10,      &sq[0].tgtGear11,  &sq[0].MSs_Ntg,    &sq[0].ShiTy12,
							&sq[0].Ne15,      &sq[0].Nt16,      &sq[0].LAcc17 );  

				iINPUT_NUMS = QUAL_TSV_CASE3_15_NEW_ITEMS;

			}
			else if( USE_IN_NEW_CASE_4_15==useInputCase )
			{
				result = sscanf(QualData, SQD_INFMT4_15,
							&sq[0].Time01,    &sq[0].OTS02,     &sq[0].MSs_Ntg,    &sq[0].EngTemp06,  &sq[0].VSP03,     &sq[0].iPATs05,    
							&sq[0].tgtGear11, &sq[0].APS09,     &sq[0].curGear08,  &sq[0].LAcc17,     &sq[0].ShiTy12,   &sq[0].Nt16,       
							&sq[0].No10,      &sq[0].acor_Nm,   &sq[0].Ne15 );  

				iINPUT_NUMS = QUAL_TSV_CASE4_15_NEW_ITEMS;

			}


			/* === 1 STEP : record (17 items check) =============== */
			iSave = 0;
			iItemCurOK = 0; /* NG - FAIL */

			//if(QUAL_TSV_DATA_ITEM_NUM==result)
			if(iINPUT_NUMS == result) /* QUAL_TSV_DATA_ITEM_NUM */
			{
				iItemCurOK = 1; /* OK */
				iOKcount ++;
			}
			else if( (iINPUT_NUMS != result) && (result!=-1 && i>0) )  /* QUAL_TSV_DATA_ITEM_NUM */
			{
				iNGcount ++;
				continue; /* reading Next item because of FAIL item */
			}
			else
			{
				iNGcount ++;
				fprintf(stderr,"\n[%s]: ++ERROR++ result = %d, NGcount = %u\n",__FUNCTION__, result, iNGcount );
				continue; /* reading Next item because of FAIL item */
			}
			/* === 1 STEP : record (17 items check) =============== */


		#if 1 /* Check tsv FILE FORMAT */
			if( (sq[0].iPATs05 < MODE_ID_NUMS-1) && (sq[0].iPATs05 >= 0) )
			{
				chkPATs_ModeID[ sq[0].iPATs05 ] ++; /* All ModeID Counts */
			}
			else
			{
				//fprintf(stderr,"++ERROR++ ModeID errors > iPATs05: %d, OKline:%u \n", sq[0].iPATs05, iOKcount );
				NGformatMODECnt ++;

				if( (NGformatMODECnt > ERROR_NUM_LIMIT) && (useInputCase>USE_IN_CASE0_NONE) && (useInputCase < USE_IN_MAX) )
				{
					RecordCnt = 0LL;
					iOKcount = 0;
					iNGcount = 0;
					memset(chkPATs_ModeID, 0x00, sizeof(chkPATs_ModeID) );
					memset(gearShift, 0x00, GEAR_SHI_NUMS*sizeof(unsigned long long) );

					NGformatMODECnt = 0; 
					NGformatGEARCnt = 0;
					NGformatCG = 0;
					NGformatTG = 0;
					iErrorVSP = 0;
					iErrorAPS = 0;
					iErrorLAcc = 0;
					iErrorRPM = 0;
					iErrorTemp = 0;

					useInputCase ++;  /* Next Input Format... */

					fprintf(stderr,">>Checking Input file FORMAT and SEQUENCE.1.  (%d) \n", useInputCase );

					rewind(inpfile);
					continue;
				}


			}


			if( (sq[0].ShiTy12 < GEAR_SHI_NUMS) && (sq[0].ShiTy12 >= 0) )
			{
				gearShift[ sq[0].ShiTy12 ] ++;    /* Shift Type Counts */
			}
			else
			{
				NGformatGEARCnt ++;
				//fprintf(stderr,"++ERROR++ ShiftType errors > ShiTy12 : %u, OKline:%u \n", sq[0].ShiTy12, iOKcount );

				if( (NGformatGEARCnt > ERROR_NUM_LIMIT) && (useInputCase>USE_IN_CASE0_NONE) && (useInputCase < USE_IN_MAX) )
				{
					RecordCnt = 0LL;
					iOKcount = 0;
					iNGcount = 0;
					memset(chkPATs_ModeID, 0x00, sizeof(chkPATs_ModeID) );
					memset(gearShift, 0x00, GEAR_SHI_NUMS*sizeof(unsigned long long) );

					NGformatMODECnt = 0; 
					NGformatGEARCnt = 0;
					NGformatCG = 0;
					NGformatTG = 0;
					iErrorVSP = 0;
					iErrorAPS = 0;
					iErrorLAcc = 0;
					iErrorRPM = 0;
					iErrorTemp = 0;

					useInputCase ++;  /* Next Input Format... */

					fprintf(stderr,">>Checking Input file FORMAT and SEQUENCE.2.  (%d) \n", useInputCase );

					rewind(inpfile);
					continue;
				}

			}


			/* ------------------------------------------------------------ */
			/* N(0), D(1~8), P(9), R(10) ---------------------------------  */
			/* 11, 12 : s1_Y, s2_Y       ---------------------------------  */
			/* 13, 14 : s1_J, s2_J       ---------------------------------  */
			/* ------------------------------------------------------------ */
			if( sq[0].curGear08 > 14 || sq[0].curGear08 < 0 )
			{
				/* currGear OK : 0~14 */
				NGformatCG++;
				//fprintf(stderr,"++ERROR++ curGear errors > %d , %u \n", sq[0].curGear08, NGformatCG );

				if( (NGformatCG > ERROR_NUM_LIMIT) && (useInputCase>USE_IN_CASE0_NONE) && (useInputCase < USE_IN_MAX) )
				{
					RecordCnt = 0LL;
					iOKcount = 0;
					iNGcount = 0;
					memset(chkPATs_ModeID, 0x00, sizeof(chkPATs_ModeID) );
					memset(gearShift, 0x00, GEAR_SHI_NUMS*sizeof(unsigned long long) );

					NGformatMODECnt = 0; 
					NGformatGEARCnt = 0;
					NGformatCG = 0;
					NGformatTG = 0;
					iErrorVSP = 0;
					iErrorAPS = 0;
					iErrorLAcc = 0;
					iErrorRPM = 0;
					iErrorTemp = 0;

					useInputCase ++;  /* Next Input Format... */

					fprintf(stderr,">>Checking Input file FORMAT and SEQUENCE.3.  (%d) \n", useInputCase );

					rewind(inpfile);
					continue;
				}

			}

			if( sq[0].tgtGear11 > 14 || sq[0].tgtGear11 < 0 )
			{
				/* tgtGear OK : 0~14 */
				NGformatTG++;
				//fprintf(stderr,"++ERROR++ tgtGear errors > %d , %u \n", sq[0].tgtGear11 );

				if( (NGformatTG > ERROR_NUM_LIMIT) && (useInputCase>USE_IN_CASE0_NONE) && (useInputCase < USE_IN_MAX) )
				{
					RecordCnt = 0LL;
					iOKcount = 0;
					iNGcount = 0;
					memset(chkPATs_ModeID, 0x00, sizeof(chkPATs_ModeID) );
					memset(gearShift, 0x00, GEAR_SHI_NUMS*sizeof(unsigned long long) );

					NGformatMODECnt = 0; 
					NGformatGEARCnt = 0;
					NGformatCG = 0;
					NGformatTG = 0;
					iErrorVSP = 0;
					iErrorAPS = 0;
					iErrorLAcc = 0;
					iErrorRPM = 0;
					iErrorTemp = 0;

					useInputCase ++;  /* Next Input Format... */

					fprintf(stderr,">>Checking Input file FORMAT and SEQUENCE.4.  (%d) \n", useInputCase );

					rewind(inpfile);
					continue;
				}

			}
		#endif

			if( (sq[0].VSP03 < 0.0f) || (sq[0].VSP03 > 256.0f) )
			{
				/* OK: 0 <= VS <= 256 */
				iErrorVSP ++;
				if( (iErrorVSP > ERROR_NUM_LIMIT) && (useInputCase>USE_IN_CASE0_NONE) && (useInputCase < USE_IN_MAX) )
				{
					RecordCnt = 0LL;
					iOKcount = 0;
					iNGcount = 0;
					memset(chkPATs_ModeID, 0x00, sizeof(chkPATs_ModeID) );
					memset(gearShift, 0x00, GEAR_SHI_NUMS*sizeof(unsigned long long) );

					NGformatMODECnt = 0; 
					NGformatGEARCnt = 0;
					NGformatCG = 0;
					NGformatTG = 0;
					iErrorVSP = 0;
					iErrorAPS = 0;
					iErrorLAcc = 0;
					iErrorRPM = 0;
					iErrorTemp = 0;

					useInputCase ++;  /* Next Input Format... */

					fprintf(stderr,">>Checking Input file FORMAT and SEQUENCE.5.  (%d) \n", useInputCase );

					rewind(inpfile);
					continue;
				}
			}


			if( (sq[0].APS09 < 0.0f) || (sq[0].APS09 > 128.0f) )
			{
				/* OK: 0 <= APS <= 128 */
				iErrorAPS ++;
				if( (iErrorAPS > ERROR_NUM_LIMIT) && (useInputCase>USE_IN_CASE0_NONE) && (useInputCase < USE_IN_MAX) )
				{
					RecordCnt = 0LL;
					iOKcount = 0;
					iNGcount = 0;
					memset(chkPATs_ModeID, 0x00, sizeof(chkPATs_ModeID) );
					memset(gearShift, 0x00, GEAR_SHI_NUMS*sizeof(unsigned long long) );
			
					NGformatMODECnt = 0; 
					NGformatGEARCnt = 0;
					NGformatCG = 0;
					NGformatTG = 0;
					iErrorVSP = 0;
					iErrorAPS = 0;
					iErrorLAcc = 0;
					iErrorRPM = 0;
					iErrorTemp = 0;

					useInputCase ++;  /* Next Input Format... */
			
					fprintf(stderr,">>Checking Input file FORMAT and SEQUENCE.6.  (%d) \n", useInputCase );
			
					rewind(inpfile);
					continue;
				}
			}


			if( (sq[0].LAcc17 < -16.0f) || (sq[0].LAcc17 > 16.0f) )
			{
				/* OK: -16 <= LAcc17 <= 16 */
				iErrorLAcc ++;
				if( (iErrorLAcc > ERROR_NUM_LIMIT) && (useInputCase>USE_IN_CASE0_NONE) && (useInputCase < USE_IN_MAX) )
				{
					RecordCnt = 0LL;
					iOKcount = 0;
					iNGcount = 0;
					memset(chkPATs_ModeID, 0x00, sizeof(chkPATs_ModeID) );
					memset(gearShift, 0x00, GEAR_SHI_NUMS*sizeof(unsigned long long) );
			
					NGformatMODECnt = 0; 
					NGformatGEARCnt = 0;
					NGformatCG = 0;
					NGformatTG = 0;
					iErrorVSP = 0;
					iErrorAPS = 0;
					iErrorLAcc = 0;
					iErrorRPM = 0;
					iErrorTemp = 0;

					useInputCase ++;  /* Next Input Format... */
			
					fprintf(stderr,">>Checking Input file FORMAT and SEQUENCE.7.  (%d) \n", useInputCase );
			
					rewind(inpfile);
					continue;
				}
			}

			if( (sq[0].No10 < 0.0f) || (sq[0].No10 > 16384.0f) ||
				(sq[0].Nt16 < 0.0f) || (sq[0].Nt16 > 16384.0f) ||
				(sq[0].Ne15 < 0.0f) || (sq[0].Ne15 > 16384.0f) )
			{
				/* OK: 0 <= No10 <= 16384 */
				/* OK: 0 <= Nt16 <= 16384 */
				/* OK: 0 <= Ne15 <= 16384 */

				iErrorRPM ++;
				if( (iErrorRPM > ERROR_NUM_LIMIT) && (useInputCase>USE_IN_CASE0_NONE) && (useInputCase < USE_IN_MAX) )
				{
					RecordCnt = 0LL;
					iOKcount = 0;
					iNGcount = 0;
					memset(chkPATs_ModeID, 0x00, sizeof(chkPATs_ModeID) );
					memset(gearShift, 0x00, GEAR_SHI_NUMS*sizeof(unsigned long long) );
			
					NGformatMODECnt = 0; 
					NGformatGEARCnt = 0;
					NGformatCG = 0;
					NGformatTG = 0;
					iErrorVSP = 0;
					iErrorAPS = 0;
					iErrorLAcc = 0;
					iErrorRPM = 0;
					iErrorTemp = 0;

					useInputCase ++;  /* Next Input Format... */
			
					fprintf(stderr,">>Checking Input file FORMAT and SEQUENCE.8.  (%d) \n", useInputCase );
			
					rewind(inpfile);
					continue;
				}
			}


			if( (sq[0].EngTemp06 < -255) || (sq[0].EngTemp06 > 255) )
			{
				/* OK: -255 <= EngTemp06 <= 255 */
			
				iErrorTemp ++;
				if( (iErrorTemp > ERROR_NUM_LIMIT) && (useInputCase>USE_IN_CASE0_NONE) && (useInputCase < USE_IN_MAX) )
				{
					RecordCnt = 0LL;
					iOKcount = 0;
					iNGcount = 0;
					memset(chkPATs_ModeID, 0x00, sizeof(chkPATs_ModeID) );
					memset(gearShift, 0x00, GEAR_SHI_NUMS*sizeof(unsigned long long) );
			
					NGformatMODECnt = 0; 
					NGformatGEARCnt = 0;
					NGformatCG = 0;
					NGformatTG = 0;
					iErrorVSP = 0;
					iErrorAPS = 0;
					iErrorLAcc = 0;
					iErrorRPM = 0;
					iErrorTemp = 0;
			
					useInputCase ++;  /* Next Input Format... */
			
					fprintf(stderr,">>Checking Input file FORMAT and SEQUENCE.9.  (%d) \n", useInputCase );
			
					rewind(inpfile);
					continue;
				}
			}



			/* === 2 STEP : SKIP record check ===================== */
			if(iItemCurOK)
			{
				iSave = 1;
			
				if(iItemCurOK && (iItemPreOK != iItemCurOK) ) iFirstItem = 1; /* First OK -> print */
			}
			iItemPreOK = iItemCurOK;
			/* === 2 STEP : SKIP record check ===================== */

			if(iFirstItem)
			{
				iFirstItem = 0;
			}


			/* ----------------------------------------------------- */
			/* Find Time Period in this data						 */
			/* ----------------------------------------------------- */
			if(iPreShTime)
			{
				avgTime  += ((unsigned int)(sq[0].Time01 * 1000) - iPreShTime);
				avgCount ++;
			}
			iPreShTime = (unsigned int)(sq[0].Time01 * 1000);
			/* ----------------------------------------------------- */
			/* Find Time Period in this data						 */
			/* ----------------------------------------------------- */


		#if 0
			/* === 3 STEP : ModeID check ===================== */
			if( !iSave ) 
			{
				continue; /* abnormal -> next record reading */
			}

			if( aiPATs05 != sq[0].iPATs05 )
			{
				continue; /* reading Next item because of no same ModeID item, example  ECO, SPT, NOR, ... */
			}
		#endif

		
		#if 0
			if(sq[0].curGear08 == sq[0].tgtGear11)
			{
				continue; /* reading Next item because of same gear number, while shift gear.. 1->2, 2->3, 3->4 , ...  2->1, 3->2 */
			}
		#endif
			/* === 3 STEP : ModeID check ===================== */


		#if 0
			if( iShiftType == SHI_PWR_ON )
			{
				if( sq[0].APS09 < fAPSpwrLvl ) continue; /* APS power level check and ignored */
			}
		#endif

			if( (sq[0].iPATs05 == aiPATs05) && (sq[0].curGear08 != sq[0].tgtGear11) )
			{

				//fprintf(stderr,"Quality Shift Data - OK (%12u) / NG (%12u) / Total (%12lld) \r", iOKcount, iNGcount, RecordCnt );

			}

		}


		/* --------------------------------------------------------------------------------------- */
		/* Input data file *.tsv, FORAMT, SEQUENCE checking -------------------------------------- */
		/* --------------------------------------------------------------------------------------- */
		if( (iOKcount>1000) && (useInputCase>USE_IN_CASE0_NONE) && (useInputCase < USE_IN_MAX) )
		{
			//fprintf(stderr,"Reading....  (%lld) \r", RecordCnt);
			if( (chkPATs_ModeID[aiPATs05] < 1) && (NGformatMODECnt || NGformatGEARCnt || NGformatCG || NGformatTG) )
			{

				RecordCnt = 0LL;
				iOKcount = 0;
				iNGcount = 0;
				memset(chkPATs_ModeID, 0x00, sizeof(chkPATs_ModeID) );
				memset(gearShift, 0x00, GEAR_SHI_NUMS*sizeof(unsigned long long) );

				NGformatMODECnt = 0; 
				NGformatGEARCnt = 0;
				NGformatCG = 0;
				NGformatTG = 0;

				iErrorVSP = 0;
				iErrorAPS = 0;
				iErrorLAcc = 0;
				iErrorRPM = 0;
				iErrorTemp = 0;

				useInputCase ++;  /* Next Input Format... */

				fprintf(stderr,">>Checking Input file FORMAT and SEQUENCE...  (%d) \n", useInputCase );

				rewind(inpfile);
			}
		}
		/* --------------------------------------------------------------------------------------- */
		/* Input data file *.tsv, FORAMT, SEQUENCE checking -------------------------------------- */
		/* --------------------------------------------------------------------------------------- */


	}
	while (!feof (inpfile));
	/* ---------- SS, SB, SP pair check ------------------ */


	/* --------------------------------------------------------------------------- */
	/* INPUT File *.tsv FORMAT --------------------------------------------------- */
	/* --------------------------------------------------------------------------- */

	if( chkPATs_ModeID[aiPATs05] < 1 )
	{
		char shi_ori[MAX_CHARS*LENGTH_OF_FILENAME+1];
		char shi_out[MAX_CHARS*LENGTH_OF_FILENAME+1];

		memset(shi_ori, 0x00, (MAX_CHARS*LENGTH_OF_FILENAME+1)*sizeof(char) );
		memset(shi_out, 0x00, (MAX_CHARS*LENGTH_OF_FILENAME+1)*sizeof(char) );

		fprintf(stderr,"  Total Quality Shift Records  : %9llu lines \n", RecordCnt );
		fprintf(stderr,"  Error Shift Records (NG) ----: %9u lines <- invalid shift data record \n", iNGcount );
		fprintf(stderr,"  Quality Shift Records (OK) --: %9u lines, %6.1lf min \n", iOKcount, (iOKcount*iavgTime)/1000.0/60 );
		fprintf(stderr,"\n");
		fprintf(stderr,"  %-11s - Shift Quality Data is NONE... %llu lines \n", arrPATs_ModeID[aiPATs05].ModeID, chkPATs_ModeID[aiPATs05] );
		fprintf(stderr,"  or *.tsv input file check (order, contents,...) \n");
		//fprintf(stderr,"  Time, OTS, VSP, TqStnd, iPATs, EngTemp, TQI, cg, APS, No, tg, ShiType, TQFR, ShiPH, Ne, Nt, LAcc \n\n");
		fprintf(stderr,"  There is NONE Quality Shift Data %-11s -> So STOP!!!! \n", arrPATs_ModeID[aiPATs05].ModeID);
		fprintf(stderr,"----------------------------------------------------------------------------------\n" );


		AllFilesClosed();
		fclose(shiFile);


		strcpy(shi_ori, shift_file);
		isNaming = 0;
		for(ii=strlen(shi_ori)-1; ii>0; ii--)
		{
			if( shi_ori[ii]=='.' ) 
			{
				shi_ori[ii+1] = '\0';
				isNaming = 1;
				break;
			}
		}
		if( 0==isNaming ) strcat(shi_ori, ".");
		
		
		// -------------------------------------------------------------
		strcpy(shi_out, shi_ori);
		strcat(shi_out, arrPATs_ModeID[aiPATs05].ModeNm);
		if( isFileExist(shi_out, 0) )
		{
			ii = remove( shi_out );
			if( 0 != ii )
				fprintf(stderr,"  Deleted temp file [%s] -> Failed(%d) \n", shi_out, ii );
		}

		// -------------------------------------------------------------
		strcpy(shi_out, shift_file);
		if( isFileExist(shi_out, 0) )
		{
			ii = remove( shi_out );
			if( 0 != ii )
				fprintf(stderr,"  Deleted temp file [%s] -> Failed(%d) \n", shi_out, ii );
		}

		exit(0);
		return 0;
	}



	/* --------------------------------------------------------------------------- */
	/* 1st step ------------------------------------------------------------------ */
	/* SS point, SB point, SP point, SF point Searching and Fixed ---------------- */
	/* Standby : SB-50msec ~ SB point for Nt-Max, Nt-min  ------------------------ */
	/* --------------------------------------------------------------------------- */

	rewind(inpfile);

	memset(sq, 0x00, sizeof(sq) );
	memset(chkPATs_ModeID, 0x00, sizeof(chkPATs_ModeID) );
	memset(gearShift, 0x00, GEAR_SHI_NUMS*sizeof(unsigned long long) );
	NGformatMODECnt = 0; 
	NGformatGEARCnt = 0;
	NGformatCG = 0;
	NGformatTG = 0;
	
	iErrorVSP = 0;
	iErrorAPS = 0;
	iErrorLAcc = 0;
	iErrorRPM = 0;
	iErrorTemp = 0;

	iSBdecCnt  = 0;
	iSBstart   = 0;
	iSBTimeSum = 0;
	iSBpntFix  = 0;

	iOnce_SS	= 0;
	iOnce_SB	= TRUE;
	iOnce_SP	= TRUE;
	iOnce_SF    = 0;

	iSPdecCnt  = 0;
	iSPstart   = 0;
	iSPTimeSum = 0;
	iSPpntFix  = 0;

	/* Nt Max decision initialized */
	iNtMaxdecCnt  = 0;
	iNtMaxstart   = 0;
	iNtMaxTimeSum = 0;
	iNtMaxpntFix  = 0;
	
	/* Nt min decision initialized */
	iNtmindecCnt  = 0;
	iNtminstart   = 0;
	iNtminTimeSum = 0;
	iNtminpntFix  = 0;
	
	isFindSS = 0;

	iSortCount  = 0;
	iFirstItem  = 0;

	g_SBtime    = -1.0f;
	g_SPtime    = -1.0f;
	g_SFtime    = -1.0f;

	g_NtMaxtime = -1.0f;
	g_Ntmintime = -1.0f;

	RecordCnt  = 0ULL;
	iNGcount   = 0ULL;
	iOKcount   = 0ULL;
	iSortCount = 0ULL;

	//avgCount   = 0ULL;
	iExistSBpoint = 0; 
	iExistSPpoint = 0;

	iSScount = 0;
	iSBcount = 0;
	iSPcount = 0;
	iSFcount = 0;

	iNtMaxcount = 0;
	iNtmincount = 0;
	is2File = 0;
	memset(gMaxTbl, 0x00, MAX_TABLE_SIZ*sizeof(tSBtimePos_type) );

	avgCount   = 0ULL;
	iavgTime   = 0;
	avgTime    = 0.0f;
	iPreShTime = 0;

	do
	{
		unsigned int i=0;

		/* Read a line from input file. */
		memset( QualData, 0x00, sizeof(QualData) );

		if( NULL == fgets( QualData, QUAL_DATA_MAX_SIZE, inpfile ) )
		{
			//fclose(shiFile);

		#if DEBUG_MSG_1ST_POINT_TIME
			fprintf(stderr,"\n");
		#endif

			fprintf(stderr,">>PATs-ModeID %s Sorting 1st step is completed!!! \r\n", arrPATs_ModeID[aiPATs05].ModeID ); 			
			break;
		}

		RecordCnt++;

		/* Remove carriage return/line feed at the end of line. */
		i = strlen(QualData);
		if(i >= QUAL_DATA_MAX_SIZE)
		{
			fprintf(stderr,"ShiftLine%6lld :ERROR: Not enough Buffer length(%d) \r\n", RecordCnt, i );
		}

		if (--i > 0)
		{
			if (QualData[i] == '\n') QualData[i] = '\0';
			if (QualData[i] == '\r') QualData[i] = '\0'; 


			if( USE_IN_CASE1_17==useInputCase )
			{
				result = sscanf(QualData, SQD_INFORMAT,
							&sq[0].Time01, &sq[0].OTS02,       &sq[0].VSP03,     &sq[0].TqStnd04,   &sq[0].iPATs05,   &sq[0].EngTemp06, 
							&sq[0].tqi07,  &sq[0].curGear08,   &sq[0].APS09,     &sq[0].No10,       &sq[0].tgtGear11, &sq[0].ShiTy12, 
							&sq[0].TqFr13, &sq[0].ShiPh14,     &sq[0].Ne15,      &sq[0].Nt16,       &sq[0].LAcc17 );

				iINPUT_NUMS = QUAL_TSV_CASE1_17_ITEM_NUM;
			}
			else if( USE_IN_CASE2_15==useInputCase )
			{
				result = sscanf(QualData, SQD_INFMT2,
							&sq[0].Time01,	  &sq[0].TqStnd04,	&sq[0].OTS02,	   &sq[0].EngTemp06,  &sq[0].VSP03,   &sq[0].iPATs05,
							&sq[0].tgtGear11, &sq[0].APS09, 	&sq[0].curGear08,  &sq[0].ShiPh14,	  &sq[0].LAcc17,  &sq[0].ShiTy12,
							&sq[0].Nt16,	  &sq[0].No10,		&sq[0].Ne15 );	
			
				iINPUT_NUMS = QUAL_TSV_CASE2_15_ITEM_NUM;
			}
			else if( USE_IN_NEW_CASE_3_15==useInputCase )
			{
				result = sscanf(QualData, SQD_INFMT3_15,
							&sq[0].Time01,    &sq[0].OTS02,     &sq[0].VSP03,   &sq[0].iPATs05,    &sq[0].EngTemp06,  &sq[0].NetEng_Acor,
							&sq[0].curGear08, &sq[0].APS09,     &sq[0].No10,    &sq[0].tgtGear11,  &sq[0].MSs_Ntg,    &sq[0].ShiTy12,
							&sq[0].Ne15,      &sq[0].Nt16,      &sq[0].LAcc17 );  

				iINPUT_NUMS = QUAL_TSV_CASE3_15_NEW_ITEMS;
			}
			else if( USE_IN_NEW_CASE_4_15==useInputCase )
			{
				result = sscanf(QualData, SQD_INFMT4_15,
							&sq[0].Time01,    &sq[0].OTS02,     &sq[0].MSs_Ntg,    &sq[0].EngTemp06,  &sq[0].VSP03,     &sq[0].iPATs05,    
							&sq[0].tgtGear11, &sq[0].APS09,     &sq[0].curGear08,  &sq[0].LAcc17,     &sq[0].ShiTy12,   &sq[0].Nt16,       
							&sq[0].No10,      &sq[0].acor_Nm,   &sq[0].Ne15 );  

				iINPUT_NUMS = QUAL_TSV_CASE4_15_NEW_ITEMS;
			}




			/* === 1 STEP : record (17 items check) =============== */
			iSave = 0;
			iItemCurOK = 0; /* NG - FAIL */
			if(iINPUT_NUMS == result)
			{
				iItemCurOK = 1; /* OK */
				iOKcount ++;
			}
			else if( (iINPUT_NUMS != result) && (result!=-1 && i>0) ) 
			{
				iNGcount ++;
				continue; /* reading Next item because of FAIL item */
			}
			else
			{
				iNGcount ++;
				fprintf(stderr,"\n[%s]: ++ERROR++ result = %d, NGcount = %u\n",__FUNCTION__, result, iNGcount );
				continue; /* reading Next item because of FAIL item */
			}
			/* === 1 STEP : record (17 items check) =============== */

			if( (sq[0].iPATs05 < MODE_ID_NUMS) && (sq[0].iPATs05 >= 0) )
			{
				chkPATs_ModeID[ sq[0].iPATs05 ] ++; /* ModeID Counts */
			}
			else
			{
				fprintf(stderr,"++ERROR++ ModeID errors = %d, line:%u \n", sq[0].iPATs05, iOKcount );
				NGformatMODECnt ++; 
			}


			if( (sq[0].ShiTy12 < GEAR_SHI_NUMS) && (sq[0].ShiTy12 >= 0) )
			{
				gearShift[ sq[0].ShiTy12 ] ++;    /* Shift Type Counts */
			}
			else
			{
				fprintf(stderr,"++ERROR++ ShiftType errors = %u, line:%u \n", sq[0].ShiTy12, iOKcount );
				NGformatGEARCnt ++;				
			}

		
			/* ------------------------------------------------------------ */
			/* N(0), D(1~8), P(9), R(10) ---------------------------------	*/
			/* 11, 12 : s1_Y, s2_Y		 ---------------------------------	*/
			/* 13, 14 : s1_J, s2_J		 ---------------------------------	*/
			/* ------------------------------------------------------------ */
			if( sq[0].curGear08 > 14 || sq[0].curGear08 < 0 )
			{
				NGformatCG++;
				fprintf(stderr,"++ERROR++ curGear errors > %d, %u \n", sq[0].curGear08, NGformatCG );
			}
		
			if( sq[0].tgtGear11 > 14 || sq[0].tgtGear11 < 0 )
			{
				NGformatTG++;
				fprintf(stderr,"++ERROR++ tgtGear errors > %d, %u \n", sq[0].tgtGear11, NGformatTG );		
			}
		
			if( (sq[0].VSP03 < 0.0f) || (sq[0].VSP03 > 256.0f) )
			{
				/* OK: 0 <= VS <= 256 */
				iErrorVSP ++;
				fprintf(stderr,"++ERROR++ VSP errors > %lf, %u \n", sq[0].VSP03, iErrorVSP );		
			}
		
		
			if( (sq[0].APS09 < 0.0f) || (sq[0].APS09 > 128.0f) )
			{
				/* OK: 0 <= APS <= 128 */
				iErrorAPS ++;
				fprintf(stderr,"++ERROR++ ASP errors > %lf, %u \n", sq[0].APS09, iErrorAPS );		
			}
		
		
			if( (sq[0].LAcc17 < -16.0f) || (sq[0].LAcc17 > 16.0f) )
			{
				/* OK: -16 <= LAcc17 <= 16 */
				iErrorLAcc ++;
				fprintf(stderr,"++ERROR++ LAcc errors > %lf, %u \n", sq[0].LAcc17, iErrorLAcc );		
			}
		
			if( (sq[0].No10 < 0.0f) || (sq[0].No10 > 16384.0f) ||
				(sq[0].Nt16 < 0.0f) || (sq[0].Nt16 > 16384.0f) ||
				(sq[0].Ne15 < 0.0f) || (sq[0].Ne15 > 16384.0f) )
			{
				/* OK: 0 <= No10 <= 16384 */
				/* OK: 0 <= Nt16 <= 16384 */
				/* OK: 0 <= Ne15 <= 16384 */
		
				iErrorRPM ++;
				fprintf(stderr,"++ERROR++ rpm errors > No:%lf, Nt:%lf, Ne:%lf, %u \n", sq[0].No10, sq[0].Nt16, sq[0].Ne15, iErrorRPM );						
			}
		

			if( (sq[0].EngTemp06 < -255) || (sq[0].EngTemp06 > 255) )
			{
				/* OK: -255 <= Temp <= 255 */
				iErrorTemp ++;
				fprintf(stderr,"++ERROR++ Temp errors > %d, %u \n", sq[0].EngTemp06, iErrorTemp );		
			}




			/* === 2 STEP : SKIP record check ===================== */
			if(iItemCurOK)
			{
				iSave = 1;
			
				if(iItemCurOK && (iItemPreOK != iItemCurOK) ) iFirstItem = 1; /* First OK -> print */
			}
			iItemPreOK = iItemCurOK;
			/* === 2 STEP : SKIP record check ===================== */


			/* ----------------------------------------------------- */
			/* Find Time Period     								 */
			/* ----------------------------------------------------- */
			if(iPreShTime)
			{
				avgTime  += ((unsigned int)(sq[0].Time01 * 1000) - iPreShTime);
				avgCount ++;
			}
			iPreShTime = (unsigned int)(sq[0].Time01 * 1000);
			/* ----------------------------------------------------- */
			/* Find Time Period     								 */
			/* ----------------------------------------------------- */

			
			/* === 3 STEP : ModeID check ===================== */
			if( !iSave ) 
			{
				continue; /* abnormal -> next record reading */
			}
			
			if( aiPATs05 != sq[0].iPATs05 )
			{
				continue; /* reading Next item because of no same ModeID item, example	ECO, SPT, NOR, ... */
			}

			#if 0
			if(sq[0].curGear08 == sq[0].tgtGear11)
			{
				continue; /* reading Next item because of same gear number, while shift gear.. 1->2, 2->3, 3->4 , ...  2->1, 3->2 */
			}
			#endif
			/* === 3 STEP : ModeID check ===================== */
			
			
			if( iShiftType == SHI_PWR_ON )
			{
				if( sq[0].APS09 < fAPSpwrLvl ) continue; /* APS power level check and ignored */
			}

			//if( (SHIFT_UP==shiDir03) && (sq[0].curGear08 != sq[0].tgtGear11) && (sq[0].curGear08+1 == sq[0].tgtGear11+0) )
			{
				if( 0 == sq[0].curGear08 ) continue; /* 0-DAN ignored */
				/* -- else OK -- */
			}
			//else if( (SHIFT_DN==shiDir03) && (sq[0].curGear08 != sq[0].tgtGear11) && (sq[0].curGear08+0 == sq[0].tgtGear11+1) )
			{
				if( 0 == sq[0].tgtGear11 ) continue; /* 0-DAN ignored */
				/* -- else OK -- */
			}
			//else if( (SHIFT_SKIP_DN==shiDir03) && (sq[0].curGear08 != sq[0].tgtGear11) && (sq[0].curGear08+2 >= sq[0].tgtGear11) )
			//{
			//	if( 0 == sq[0].tgtGear11 ) continue; /* 0-DAN ignored */
			//	/* -- else OK -- */
			//}
			//else 
			//{
			//	continue;
			//}


			/* ------------------------------------------------------------------- */
			/* -- NOT saving ----------------------------------------------------- */
			/* ------------------------------------------------------------------- */
			is2File = 0;

			
			/* ------------------------------------------------------------------- */
			/* ------------------------------------------------------------------- */
			if( (sq[0].iPATs05 == aiPATs05) && (sq[0].curGear08 == sq[0].tgtGear11) && (curSmPreGear+1==sq[0].curGear08) )
			{
				is2File = 0;

				/* There are SB, SP point. exist!!! */
				if( iExistSBpoint && iExistSPpoint ) 
				{
					if( curSmPreGear+1 == sq[0].curGear08 ) /* curGr == tgtGr */
					{
						//if( 0L == gMaxTbl[iSScount-1].SFTime )
						{
						strcpy(sTimePos, TXT_SFTIME);

						gMaxTbl[iSScount-1].SFTime = (unsigned int)( (sq[0].Time01)*1000*JERK_TIME_SCALE );

					#if DEBUG_MSG_1ST_POINT_TIME
						fprintf(stderr,"aaa(%3d)-> SF:%9ld : SF-SP:%9.4lf, SP-SS:%9.4lf ", iSScount-1, gMaxTbl[iSScount-1].SFTime, 
								(double)(gMaxTbl[iSScount-1].SFTime - gMaxTbl[iSScount-1].SPTime)/1000/JERK_TIME_SCALE,
								(double)(gMaxTbl[iSScount-1].SPTime - gMaxTbl[iSScount-1].SSTime)/1000/JERK_TIME_SCALE );
					#endif

						iSFcount ++;

						is2File = 1;
						iOnce_SF = 1;
						isFindSS = 0; /* goto first SS */

						iExistSBpoint = 0;
						iExistSPpoint = 0;
						}

					}
				}
				else if( (curSmPreGear+1==sq[0].curGear08) && (tgtSmPreGear==sq[0].tgtGear11) )
				{
					iSFcount ++;

					is2File = 1;
					iOnce_SF = 1;
					strcpy(sTimePos, TXT_SFTIME);

					isFindSS = 0; /* goto first SS */
					
					gMaxTbl[iSScount-1].SFTime = (unsigned int)( (sq[0].Time01)*1000*JERK_TIME_SCALE );

				}
				else
				{
					/* reading Next item because of same gear number, while shift gear.. 1->2, 2->3, 3->4 , ...  2->1, 3->2 */
					//continue;
				}

				/* CASE : (sq[0].curGear08 != sq[0].tgtGear11) -> initialized */
				//// -------- curPreGear = -1;
				//// -------- tgtPreGear = 0;
			}
			/* ------------------------------------------------------------------- */
			/* -- Power On / Up Shift -------------------------------------------- */
			/* ------------------------------------------------------------------- */
			else if( (sq[0].iPATs05 == aiPATs05) && (sq[0].curGear08 +1 == sq[0].tgtGear11) )
			{
				//printf(" DIFF -> cg=%d, tg=%d   local: cg(%d), tg (%d) global : cg (%d) tg (%d) \n", sq[0].curGear08, sq[0].tgtGear11, curPreGear, tgtPreGear , curSmPreGear, tgtSmPreGear );

				is2File = 1;

				iSortCount ++;

				iShift = sq[0].curGear08*10 + sq[0].tgtGear11; /* 12..23..34..45..56..67..78.. */ 
											 					/* 21..32..43..54..65..76..87.. */
				gearRatio = 0.0f;
				gearShift1One = 0.0f;
				gearShift2Two = 0.0f;
				if( (sq[0].No10>0.0 || sq[0].No10<0.0) )
				{
					gearRatio = (sq[0].Nt16/sq[0].No10); // gearTable[sq[0].curGear08];
				}
				gearShift1One  = (sq[0].Nt16 - sq[0].No10 * gearTable[sq[0].curGear08]);
				gearShift2Two  = (sq[0].Nt16 - sq[0].No10 * gearTable[sq[0].tgtGear11]);


				/* ----------------------------------------------------- */
				/* Calc Jerk0 (per 5msec) 								 */
				/* ----------------------------------------------------- */
				fJerk0 = 0.0f;
				if(preTime)
				{
					DiffTime  = ((unsigned int)(sq[0].Time01 * 1000 * JERK_TIME_SCALE) - preTime);
					if(DiffTime)
					{
						fJerk0 = (sq[0].LAcc17 * 1000 - preLAcc)*JERK_TIME_SCALE/(DiffTime); /* UNIT: m2/sec/msec */
					}
				}
				preTime = (unsigned int)(sq[0].Time01 * 1000 * JERK_TIME_SCALE);
				preLAcc = (sq[0].LAcc17 * 1000);
				/* ----------------------------------------------------- */
				/* Calc Jerk0 (per 5msec) 								 */
				/* ----------------------------------------------------- */



				
				/* ----------------------------------------------------- */
				/* Find SSTime Position                                  */
				/* ----------------------------------------------------- */
				if(0==isFindSS)
				{
					if( (curPreGear != sq[0].curGear08) && (tgtPreGear != sq[0].tgtGear11) ) 
					//if( (sq[0].curGear08 +1 == sq[0].tgtGear11) && (curSmPreGear==sq[0].curGear08) && (curSmPreGear==tgtSmPreGear) )
					{
						isFindSS = 1;
					}
					else if( iOnce_SF && (curPreGear == sq[0].curGear08) && (tgtPreGear == sq[0].tgtGear11) )
					{
						isFindSS = 1;
					}

					curPreGear = sq[0].curGear08;
					tgtPreGear = sq[0].tgtGear11;
				}


				if( isFindSS ) // && (sq[0].curGear08 != sq[0].tgtGear11) )
				{

					ignoredRecord = 0; /* PASS(OK)  at SS point */
					g_SBtime	= -1.0f;
					g_SPtime	= -1.0f;
					g_SFtime	= -1.0f;


					isFindSS = 0;
					strcpy(sTimePos, TXT_SSTIME);
					strcpy(sTimeNtPos, TXT_UPCASE); /* Up case default STRING -- */


					if( iSScount >= MAX_TABLE_SIZ )
					{
						fprintf(stderr,"++ERROR+%d+ gMaxTbl[%d] Table index over!! \n", __LINE__, iSScount);
					} 
					else
					{
					gMaxTbl[iSScount].SSTime = (unsigned int)( (sq[0].Time01)*1000*JERK_TIME_SCALE );
					gMaxTbl[iSScount].SFTime = 0L;

				#if DEBUG_MSG_1ST_POINT_TIME
						fprintf(stderr,"\nZZZ(%3d)-> SS:%9ld ", iSScount, gMaxTbl[iSScount].SSTime );
				#endif

					}

					iSScount ++;

					iOnce_SS = 1;

					/* initialized */
					iOnce_SB = 1;
					iOnce_SP = 1;
					iOnce_SF = 0;

					iExistSBpoint = 0;
					iExistSPpoint = 0;

					iOnce_NtMax = 0; /* Power On & Upshift -> NtMax */
					iOnce_Ntmin = 0; /* Power On & Upshift -> Ntmin */
					
					MaxAcc = 0.0f;
					minAcc = GMAX_RVALUE;
					

					/* SB decision initialized */
					memset(SBdecision, 0x00, SB_DECISION_NUM*sizeof(tSBdec_type) );
					iSBdecCnt  = 0;
					iSBstart   = 0;
					iSBTimeSum = 0;
					iSBpntFix  = 0;
					iSBswingCnt = 0;

					/* SP decision initialized */
					memset(SPdecision, 0x00, SP_DECISION_NUM*sizeof(tSPdec_type) );
					iSPdecCnt  = 0;
					iSPstart   = 0;
					iSPTimeSum = 0;
					iSPpntFix  = 0;
					iSPswingCnt = 0;

					/* Nt Max decision initialized */
					memset(NtMaxdecision, 0x00, NtMax_DECISION_NUM*sizeof(tNtMaxdec_type) );
					iNtMaxdecCnt  = 0;
					iNtMaxstart   = 0;
					iNtMaxTimeSum = 0;
					iNtMaxpntFix  = 0;
					iNtMaxswingCnt = 0;

					/* Nt min decision initialized */
					memset(Ntmindecision, 0x00, Ntmin_DECISION_NUM*sizeof(tNtmindec_type) );
					iNtmindecCnt  = 0;
					iNtminstart   = 0;
					iNtminTimeSum = 0;
					iNtminpntFix  = 0;
					iNtminswingCnt = 0;

				}
				else
				{
					/* initialized */
					strcpy(sTimePos, TXT_UPCASE);
					strcpy(sTimeNtPos, TXT_UPCASE);

					if( SHIFT_UP==shiDir03 ) /* Power On, Power Off case */
					{

						/* -------------------------------------------------------------------------------
						※	1-2 : Shift Begin (SB) 점 판단식 : Nt - No x 1단 기어비 ≤-30RPM 되는 싯점 
								  Synch Point (SP) 점 판단식 : Nt - No x 2단 기어비 ≤ 30RPM 되는 싯점	 
								  Nt Max 점 판단식			 : Nt - No x 1단 기어비 ≤0RPM 되는 싯점 
								  Nt Max 점 판단식			 : Nt - No x 2단 기어비 ≤0RPM 되는 싯점 
						---------------------------------------------------------------------------------- */

					#if 0 // SB_POINT_SWING1
						/* SB 튀는 시점 발생 -> */
						if( (iOnce_SS) && (0==iOnce_SB) && (gearShift1One > UP_SB_PNT_RPM) && (pre_gearShift1One < (gearShift1One-fSBswingLvl /*SB_SWING_LEVEL*/)) )
						{
							iOnce_SB = 1; // re-SB point
							iSBpntFix = 1;
							pre_gearShift1One = gearShift1One;

							iSBnewPoint[iSBswingCnt] = iSBcount;
							iSBswingCnt ++;
							fprintf(stderr,"  SB re-point because of rpm ratio (%.1f) swing... SBcount:%3u -> %u : *\n", fSBswingLvl, iSBswingCnt, iSBcount );
						}
					#endif

					/* =============================================================== */
					/* 1) Shift Begin (SB) decision checking, contunous 3 times  ----- */
					/* =============================================================== */
						if( 0==iSBpntFix && (gearShift1One <= UP_SB_PNT_RPM) ) /* Shift Begin (SB) -30 rpm under */
						{	
							if(iSBdecCnt < SB_DECISION_NUM)
							{
								SBdecision[iSBdecCnt].FixCount = 1; // 1 times
								SBdecision[iSBdecCnt].SBTime = (unsigned int)( (sq[0].Time01)*1000*JERK_TIME_SCALE ); // 1 times
								iSBstart = 1;
								iOnce_SB = 1; // re-enterance
								//fprintf(stderr,"  SB decision SBdecision: %d:(%d) \n", iSBdecCnt, SBdecision[iSBdecCnt].FixCount );
							}
							else
								fprintf(stderr, "  SB decision point too many points... %d \n", iSBdecCnt );
						}
						else
						{
							iSBdecCnt  = 0;
							iSBstart   = 0;
							iSBTimeSum = 0;
							memset(SBdecision, 0x00, SB_DECISION_NUM*sizeof(tSBdec_type) );
						}

						if(iSBstart)
							iSBdecCnt ++;

						iSBTimeSum = 0; /* clear before summation */
						for(ide=0; ide<iSBdecCnt; ide++)
							iSBTimeSum += SBdecision[ide].FixCount;

						if( iSBTimeSum >= iSBdecision /* SB_DECISION_TIMES */) 
						{
							iSBpntFix=1;
							//fprintf(stderr,"  SB decision ---- OK (%d) \n", iSBTimeSum );
						}
						/* =============================================================== */
						/* 1) Shift Begin (SB) decision checking, contunous 3 times  ----- */
						/* =============================================================== */
						

						/* ====================================================================== */
						/* 2) Synchronizing Point (SP) decision checking, , contunous 3 times --- */
						/* ====================================================================== */
						if( (1==iSBpntFix) && (0==iSPpntFix) && (gearShift2Two <= UP_SP_PNT_RPM) ) /* SP after SB */
						{	
							if(iSPdecCnt < SP_DECISION_NUM)
							{
								SPdecision[iSPdecCnt].FixCount = 1; // 1 times
								SPdecision[iSPdecCnt].SPTime = (unsigned int)( (sq[0].Time01)*1000*JERK_TIME_SCALE ); // 1 times
								iSPstart = 1;
								iOnce_SP = 1;
								//fprintf(stderr,"  SP decision SPdecision: %d:(%d) \n", iSPdecCnt, SPdecision[iSPdecCnt].FixCount );
							}
							else
								fprintf(stderr, "  SP decision point too many points... %d \n", iSPdecCnt );
						}
						else
						{
							iSPdecCnt  = 0;
							iSPstart   = 0;
							iSPTimeSum = 0;
							memset(SPdecision, 0x00, SP_DECISION_NUM*sizeof(tSPdec_type) );
						}

						if(iSPstart)
							iSPdecCnt ++;

						iSPTimeSum = 0; /* clear before summation */
						for(ide=0; ide<iSPdecCnt; ide++)
							iSPTimeSum += SPdecision[ide].FixCount;

						if( iSPTimeSum >= iSBdecision /* SP_DECISION_TIMES */) 
						{
							iSPpntFix=1;
							//fprintf(stderr,"  SP decision ---- OK (%d) \n", iSPTimeSum );
						}
						/* ====================================================================== */
						/* 2) Synchronizing Point (SP) decision checking, contunous 3 times ----- */
						/* ====================================================================== */


					#if 0
						/* ====================================================================== */
						/* 3) NtMax decision checking, continous 3 times ------------------------ */
						/* ====================================================================== */
						if( 0==iNtMaxpntFix && (gearShift1One <= UP_NtMax_PNT_RPM) ) /* NtMax gear1 <= 0 rpm under */
						{	
							if(iNtMaxdecCnt < NtMax_DECISION_NUM)
							{
								NtMaxdecision[iNtMaxdecCnt].FixCount = 1; // 1 times
								NtMaxdecision[iNtMaxdecCnt].NtMaxTime = (unsigned int)( (sq[0].Time01)*1000*JERK_TIME_SCALE ); // 1 times
								iNtMaxstart = 1;
								iOnce_NtMax = 1; // re-enterance
								//fprintf(stderr,"  iNtMaxdecCnt NtMaxdecision: %d:(%d) \n", iNtMaxdecCnt, NtMaxdecision[iNtMaxdecCnt].FixCount );
							}
							else
								fprintf(stderr, "  iNtMaxdecCnt point too many points... %d \n", iNtMaxdecCnt );
						}
						else
						{
							iNtMaxdecCnt  = 0;
							iNtMaxstart   = 0;
							iNtMaxTimeSum = 0;

							memset(NtMaxdecision, 0x00, NtMax_DECISION_NUM*sizeof(tNtMaxdec_type) );
						}

						if(iNtMaxstart)
							iNtMaxdecCnt ++;

						iNtMaxTimeSum = 0; /* clear before summation */
						for(ide=0; ide<iNtMaxdecCnt; ide++)
							iNtMaxTimeSum += NtMaxdecision[ide].FixCount;

						if( iNtMaxTimeSum >= iSBdecision /* NtMax_DECISION_NUM */) 
						{
							iNtMaxpntFix=1;
							//fprintf(stderr,"  iNtMaxTimeSum ---- OK (%d) \n", iNtMaxTimeSum );
						}
						/* ====================================================================== */
						/* 3) NtMax decision checking, continous 3 times ------------------------ */
						/* ====================================================================== */
						

						/* ====================================================================== */
						/* 4) Ntmin decision checking, continous 3 times ------------------------ */
						/* ====================================================================== */

						/* Nt-min Value : SP point ~ SP + 50msec point 사이 */
						//iCurrTime = (unsigned int)( (sq[0].Time01)*1000*JERK_TIME_SCALE );
						//if( gMaxTbl[iSScount-1].SPTime <= iCurrTime && gMaxTbl[iSScount-1].NtminEnd >= iCurrTime )
						{

							if( 0==iNtminpntFix && (gearShift2Two <= UP_Ntmin_PNT_RPM) ) /* Ntmin gear2 <= 0 rpm under */
							{	
								if(iNtmindecCnt < Ntmin_DECISION_NUM)
								{
									Ntmindecision[iNtmindecCnt].FixCount = 1; // 1 times
									Ntmindecision[iNtmindecCnt].NtminTime = (unsigned int)( (sq[0].Time01)*1000*JERK_TIME_SCALE ); // 1 times
									iNtminstart = 1;
									iOnce_Ntmin = 1; // re-enterance
									//fprintf(stderr,"  iNtmindecCnt Ntmindecision: %d:(%d) \n", iNtmindecCnt, Ntmindecision[iNtmindecCnt].FixCount );
								}
								else
									fprintf(stderr, "  iNtmindecCnt point too many points... %d \n", iNtmindecCnt );
							}
							else
							{
								iNtmindecCnt  = 0;
								iNtminstart   = 0;
								iNtminTimeSum = 0;
								memset(Ntmindecision, 0x00, Ntmin_DECISION_NUM*sizeof(tNtmindec_type) );
							}

							if(iNtminstart)
								iNtmindecCnt ++;

							iNtminTimeSum = 0; /* clear before summation */
							for(ide=0; ide<iNtmindecCnt; ide++)
								iNtminTimeSum += Ntmindecision[ide].FixCount;

							if( iNtminTimeSum >= iSBdecision /* Ntmin_DECISION_NUM */) 
							{
								iNtminpntFix=1;
								//fprintf(stderr,"  iNtminTimeSum ---- OK (%d) \n", iNtminTimeSum );
							}

						}
						/* ====================================================================== */
						/* 4) Ntmin decision checking, continous 3 times ------------------------ */
						/* ====================================================================== */
					#endif



						/* =============================================================== */
						/* STRING Fix ---------------------------------------------------- */
						/* Shift Begin (SB) -30 rpm under decision ----------------------- */
						/* =============================================================== */
						if( iOnce_SB && (gearShift1One <= UP_SB_PNT_RPM) ) /* Shift Begin (SB) -30 rpm under */
						{
							iOnce_SB = FALSE;

							if( iSBpntFix ) 
							{
								iExistSBpoint = 1;

								strcpy(sTimePos, TXT_SBTIME); 
								//strcpy(sTimePos, TXT_SBSWING0);

								if( iSBcount >= MAX_TABLE_SIZ )
								{
									fprintf(stderr,"++ERROR+%d+ gMaxTbl[%d] Table index over!! \n", __LINE__, iSBcount);
								} 
								else
								{
								gMaxTbl[iSScount-1].Index      = iSBcount;
								//gMaxTbl[iSBcount].SBTime     = (sq[0].Time01)*1000*JERK_TIME_SCALE;  /* Last One Choice */
								//gMaxTbl[iSBcount].SBTime     = SBdecision[0].SBTime;  /* first One time choice */ 
								gMaxTbl[iSScount-1].SBTime     = SBdecision[SBposDecision].SBTime;  /* first posistion or Last Position time choice */ 

								gMaxTbl[iSScount-1].gMx1Begin  = gMaxTbl[iSScount-1].SBTime - iJerkTimeLen*JERK_TIME_SCALE;  /* JERK_TIME_mSec */
								gMaxTbl[iSScount-1].gmn1End    = gMaxTbl[iSScount-1].SBTime; 

								gMaxTbl[iSScount-1].NtMaxBegin = gMaxTbl[iSScount-1].SBTime - iNtTimeLen*JERK_TIME_SCALE;	/* JERK_TIME_mSec */

						#if DEBUG_MSG_1ST_POINT_TIME
									fprintf(stderr,"aaa(%3d)-> SB:%9ld ", iSScount-1, gMaxTbl[iSScount-1].SBTime );
						#endif

								}


								iSBcount ++;
							}
							else
							{
								strcpy(sTimePos, TXT_SBSWING0);
								iSBswingCnt++;
							}

							g_SBtime = sq[0].Time01;

						}

						/* =============================================================== */
						/* Synchronizing Poing (SP) 30 rpm under decision ----------------- */
						/* =============================================================== */
						if( iOnce_SP && (gearShift2Two <= UP_SP_PNT_RPM) ) /* Synchronizing Point (SP) 30 rpm under */
						{
							iOnce_SP = FALSE;

							if( iSPpntFix ) 
							{
								iExistSPpoint = 1;
								strcpy(sTimePos, TXT_SPTIME);
								//strcpy(sTimePos, TXT_SPSWING0);

								if( iSPcount >= MAX_TABLE_SIZ )
								{
									fprintf(stderr,"++ERROR+%d+ gMaxTbl[%d] Table index over!! \n", __LINE__, iSPcount);
								} 
								else
								{
								//gMaxTbl[iSPcount].Index     = iSPcount;
								//gMaxTbl[iSPcount].SPTime    = SPdecision[0].SBTime;  /* first One time choice */ 
								gMaxTbl[iSScount-1].SPTime    = SPdecision[SBposDecision].SPTime;  /* first One time choice */ 
								//gMaxTbl[iSPcount].gmMxBegin = gMaxTbl[iSPcount].SPTime - iJerkTimeLen*JERK_TIME_SCALE;  /* JERK_TIME_mSec */

								gMaxTbl[iSScount-1].NtminEnd  = gMaxTbl[iSScount-1].SPTime + iNtTimeLen*JERK_TIME_SCALE;	/* Nt-min end time */

							#if DEBUG_MSG_1ST_POINT_TIME
									fprintf(stderr,"aaa(%3d)-> SP:%9ld ", iSScount-1, gMaxTbl[iSScount-1].SPTime );
							#endif

								}
								
								iSPcount ++;
							}
							else
							{
								strcpy(sTimePos, TXT_SPSWING0);
								iSPswingCnt++;
							}


							g_SPtime = sq[0].Time01;
							iOnce_SS = 0;
						}


					#if 0 // 2nd step as below
						/* =============================================================== */
						/* Nt Max gear1 <= 0 rpm under decision -------------------------- */
						/* =============================================================== */
						if( iOnce_NtMax && (gearShift1One <= UP_NtMax_PNT_RPM) ) /* NtMax gear1 0 rpm under */
						{
							iOnce_NtMax = FALSE;

							if( iNtMaxpntFix ) 
							{

								strcpy(sTimeNtPos, TXT_NtMaxTIME);

								if( iNtMaxcount >= MAX_TABLE_SIZ )
								{
									fprintf(stderr,"++ERROR+%d+ gMaxTbl[%d] Table index over!! \n", __LINE__, iNtMaxcount);
								} 
								else
								{
								//gMaxTbl[iNtMaxcount].Index	 = iNtMaxcount;
								gMaxTbl[iSScount-1].NtMaxTime = NtMaxdecision[SBposDecision].NtMaxTime;	/* first One time choice */ 
								}
								iNtMaxcount ++;
							}
							else
							{
								strcpy(sTimeNtPos, TXT_NtMaxSWING0);
								iNtMaxswingCnt++;
							}

							g_NtMaxtime = sq[0].Time01;

						}


						/* =============================================================== */
						/* Nt min gear2 <= 0 rpm under decision -------------------------- */
						/* =============================================================== */
						if( iOnce_Ntmin && (gearShift2Two <= UP_Ntmin_PNT_RPM) ) /* Ntmin gear2 0 rpm under */
						{
							iOnce_Ntmin = FALSE;

							if( iNtminpntFix ) 
							{

								strcpy(sTimeNtPos, TXT_NtminTIME);

								if( iNtmincount >= MAX_TABLE_SIZ )
								{
									fprintf(stderr,"++ERROR+%d+ gMaxTbl[%d] Table over!! \n", __LINE__, iNtmincount);
								} 
								else
								{
								//gMaxTbl[iNtmincount].Index	 = iNtmincount;
								gMaxTbl[iSScount-1].NtminTime = Ntmindecision[SBposDecision].NtminTime;	/* first One time choice */ 
								}

								iNtmincount ++;
							}
							else
							{
								strcpy(sTimeNtPos, TXT_NtminSWING0);
								iNtminswingCnt++;
							}

							g_Ntmintime = sq[0].Time01;

						}
					#endif


					}
					else if( SHIFT_DN==shiDir03 ) /* Power On, Power Off case */
					{
						/* ------------------------------------------------------------------------------
						※ 2-1 : Shift Begin (SB) 점 판단식 : Nt - No x 1단 기어비 ≥ 30RPM 되는 싯점 
								 Synch Point (SP) 점 판단식 : Nt - No x 2단 기어비 ≥-30RPM 되는 싯점	 
						--------------------------------------------------------------------------------- */

						if( iOnce_SB && (gearShift1One >= SB_PNT_RPM_DNS) ) /* Shift Begin (SB) -30 rpm under */
						{
							iOnce_SB = FALSE;
							strcpy(sTimePos, TXT_SBTIME);
							iSBcount ++;

							g_SBtime = sq[0].Time01;
						}
						else if( iOnce_SP && (gearShift2Two >= SP_PNT_RPM_DNS) ) /* 30 rpm under */
						{
							iOnce_SP = FALSE;
							strcpy(sTimePos, TXT_SPTIME);
							iSPcount ++;

							g_SPtime = sq[0].Time01;
						}
						else
						{
							strcpy(sTimePos, TXT_DNCASE);
						}

					}
					else if( SHIFT_SKIP_DN==shiDir03 )
					{


					}
					else if( SHIFT_PNDR==shiDir03 ) /* Static shift */
					{
						/* ---------------------------------------------------------------------------------
							P/N-D/R : Shift Begin  (SB) 점 판단식 : Nt - Nt_max ≤ -30RPM 되는 싯점 
						              Shift Finish (FF) 점 판단식 : Nt ≤ 30RPM 되는 싯점	 
					      ---------------------------------------------------------------------------------- */


					}
					else
					{
						strcpy(sTimePos, TXT_UNKNOWN);
					}
				}
				/* ----------------------------------------------------- */
				/* Find SSTime Position                                  */
				/* ----------------------------------------------------- */


				/* --------------------------------------------------------------
				Power ON UP-shift CASE >> 
				 (1) Nt_max와 Ne_max는 SB 시점값이고, 
				 (2) Nt_min과 Ne_min은 FF 시점값을 취합니다.

				 (3) G_max와G_min는 SS에서 SB 구간 중 최대값과 최소값입니다.
				 (4) △G 는 (SB~FF의 G_평균값과 FF~SF의 G_평균값)의 차이값입니다.

				Power ON DOWN-shift CASE >>
				 (1) Nt_min와 Ne_min은 SB 시점값이고, 
				 (2) Nt_max과 Ne_max는 FF 시점값을 취합니다.
				 (3) j1 >> G_max는 SB~FF사이에서 G_최대값을 취하고, 
				     G_min은 G_최대값 직후에 오는 G_최소값을 취합니다.
				 (4) j2를 구하기 위한 G_min은 FF~SF사이에서 G_최소값을 취하고, 
				     G_max는 G_최소값 직후에 오는 G_최대값을 취합니다.
				----------------------------------------------------------------- */

				/* -------------------------------------------------------------- */
				/* g_Max, g_min  ------------------------------------------------ */
				/* -------------------------------------------------------------- */

				if( MaxAcc <= sq[0].LAcc17 ) MaxAcc = sq[0].LAcc17; /* Find g_Max */
				if( minAcc >= sq[0].LAcc17 ) minAcc = sq[0].LAcc17; /* Find g_mix */

				
				/* -------------------------------------------------------------- */
				/* g_Max, g_min  ------------------------------------------------ */
				/* -------------------------------------------------------------- */


			}


			curSmPreGear = sq[0].curGear08;
			tgtSmPreGear = sq[0].tgtGear11;

		#if 0 //SAVEMODE
			if(shiFile && is2File)
			{
			fprintf(shiFile, SAVEFMT,
					sq[0].Time01,	 sq[0].iPATs05, arrPATs_ModeID[sq[0].iPATs05].ModeID, sq[0].VSP03, sq[0].tqi07, 
					sq[0].curGear08, sq[0].APS09,	sq[0].No10, sq[0].tgtGear11, 
					iShift, 		 sq[0].ShiTy12, arrGear[sq[0].ShiTy12].sGear, sq[0].TqFr13, sq[0].ShiPh14, 
					sq[0].Ne15, 	 sq[0].Nt16,    sq[0].LAcc17, 
					sTimePos, sTimeNtPos, gearRatio, gearShift1One, gearShift2Two, fJerk0, MaxAcc, minAcc );

			fprintf(shiFile, "\n");
			}
		#endif


		}
	}
	while (!feof (inpfile));




	/* --------------------------------------------------------------------------- */
	/* 2nd step ------------------------------------------------------------------ */
	/* SS point, SB point, SP point, SF point Searching and Fixed ---------------- */
	/* Standby : SB-50msec ~ SB point for Nt-Max, Nt-min  ------------------------ */
	/* Low Pass Filter for LAcc value  ------------------------------------------- */
	/* --------------------------------------------------------------------------- */

	rewind(inpfile);

	memset(sq, 0x00, sizeof(sq) );
	memset(chkPATs_ModeID, 0x00, sizeof(chkPATs_ModeID) );
	memset(gearShift, 0x00, GEAR_SHI_NUMS*sizeof(unsigned long long) );
	NGformatMODECnt = 0; 
	NGformatGEARCnt = 0;
	NGformatCG = 0;
	NGformatTG = 0;
	iErrorVSP = 0;
	iErrorAPS = 0;
	iErrorLAcc = 0;
	iErrorRPM = 0;

	iSBdecCnt  = 0;
	iSBstart   = 0;
	iSBTimeSum = 0;
	iSBpntFix  = 0;

	iOnce_SS	= 0;
	iOnce_SB	= TRUE;
	iOnce_SP	= TRUE;
	iOnce_SF    = 0;

	iSPdecCnt  = 0;
	iSPstart   = 0;
	iSPTimeSum = 0;
	iSPpntFix  = 0;

	/* Nt Max decision initialized */
	iNtMaxdecCnt  = 0;
	iNtMaxstart   = 0;
	iNtMaxTimeSum = 0;
	iNtMaxpntFix  = 0;
	
	/* Nt min decision initialized */
	iNtmindecCnt  = 0;
	iNtminstart   = 0;
	iNtminTimeSum = 0;
	iNtminpntFix  = 0;
	
	isFindSS = 0;

	iSortCount  = 0;
	iFirstItem  = 0;

	g_SBtime    = -1.0f;
	g_SPtime    = -1.0f;
	g_SFtime    = -1.0f;

	g_NtMaxtime = -1.0f;
	g_Ntmintime = -1.0f;

	RecordCnt  = 0ULL;
	iNGcount   = 0ULL;
	iOKcount   = 0ULL;
	iSortCount = 0ULL;

	//avgCount   = 0ULL;
	iExistSBpoint = 0; 
	iExistSPpoint = 0;

	iSScount = 0;
	iSBcount = 0;
	iSPcount = 0;
	iSFcount = 0;

	iNtMaxcount = 0;
	iNtmincount = 0;
	is2File = 0;
	//memset(gMaxTbl, 0x00, MAX_TABLE_SIZ*sizeof(tSBtimePos_type) );

	avgCount   = 0ULL;
	iavgTime   = 0;
	avgTime    = 0.0f;
	iPreShTime = 0;

	do
	{
		unsigned int i=0;

		/* Read a line from input file. */
		memset( QualData, 0x00, sizeof(QualData) );

		if( NULL == fgets( QualData, QUAL_DATA_MAX_SIZE, inpfile ) )
		{
			fclose(shiFile);

	#if DEBUG_MSG_1ST_POINT_TIME
			fprintf(stderr,"\n");
	#endif

			fprintf(stderr,">>PATs-ModeID %s Sorting Nt-min/Nt-Max start and end points are searched!!! \r\n", arrPATs_ModeID[aiPATs05].ModeID ); 			
			break;
		}

		RecordCnt++;

		/* Remove carriage return/line feed at the end of line. */
		i = strlen(QualData);
		if(i >= QUAL_DATA_MAX_SIZE)
		{
			fprintf(stderr,"ShiftLine%6lld :ERROR: Not enough Buffer length(%d) \r\n", RecordCnt, i );
		}

		if (--i > 0)
		{
			if (QualData[i] == '\n') QualData[i] = '\0';
			if (QualData[i] == '\r') QualData[i] = '\0'; 


			if( USE_IN_CASE1_17==useInputCase )
			{
				result = sscanf(QualData, SQD_INFORMAT,
							&sq[0].Time01, &sq[0].OTS02,     &sq[0].VSP03, &sq[0].TqStnd04, &sq[0].iPATs05,   &sq[0].EngTemp06, 
							&sq[0].tqi07,  &sq[0].curGear08, &sq[0].APS09, &sq[0].No10,     &sq[0].tgtGear11, &sq[0].ShiTy12, 
							&sq[0].TqFr13, &sq[0].ShiPh14,   &sq[0].Ne15,  &sq[0].Nt16,     &sq[0].LAcc17 );

				iINPUT_NUMS = QUAL_TSV_CASE1_17_ITEM_NUM;
			}
			else if( USE_IN_CASE2_15==useInputCase )
			{
				result = sscanf(QualData, SQD_INFMT2,
							&sq[0].Time01,	  &sq[0].TqStnd04,	&sq[0].OTS02,	   &sq[0].EngTemp06,  &sq[0].VSP03,   &sq[0].iPATs05,
							&sq[0].tgtGear11, &sq[0].APS09, 	&sq[0].curGear08,  &sq[0].ShiPh14,	  &sq[0].LAcc17,  &sq[0].ShiTy12,
							&sq[0].Nt16,	  &sq[0].No10,		&sq[0].Ne15 );	
			
				iINPUT_NUMS = QUAL_TSV_CASE2_15_ITEM_NUM;
			}
			else if( USE_IN_NEW_CASE_3_15==useInputCase )
			{
				result = sscanf(QualData, SQD_INFMT3_15,
							&sq[0].Time01,    &sq[0].OTS02,     &sq[0].VSP03,   &sq[0].iPATs05,    &sq[0].EngTemp06,  &sq[0].NetEng_Acor,
							&sq[0].curGear08, &sq[0].APS09,     &sq[0].No10,    &sq[0].tgtGear11,  &sq[0].MSs_Ntg,    &sq[0].ShiTy12,
							&sq[0].Ne15,      &sq[0].Nt16,      &sq[0].LAcc17 );  

				iINPUT_NUMS = QUAL_TSV_CASE3_15_NEW_ITEMS;
			}
			else if( USE_IN_NEW_CASE_4_15==useInputCase )
			{
				result = sscanf(QualData, SQD_INFMT4_15,
							&sq[0].Time01,    &sq[0].OTS02,     &sq[0].MSs_Ntg,    &sq[0].EngTemp06,  &sq[0].VSP03,     &sq[0].iPATs05,    
							&sq[0].tgtGear11, &sq[0].APS09,     &sq[0].curGear08,  &sq[0].LAcc17,     &sq[0].ShiTy12,   &sq[0].Nt16,       
							&sq[0].No10,      &sq[0].acor_Nm,   &sq[0].Ne15 );  

				iINPUT_NUMS = QUAL_TSV_CASE4_15_NEW_ITEMS;
			}




			/* === 1 STEP : record (17 items check) =============== */
			iSave = 0;
			iItemCurOK = 0; /* NG - FAIL */
			if(iINPUT_NUMS == result)
			{
				iItemCurOK = 1; /* OK */
				iOKcount ++;
			}
			else if( (iINPUT_NUMS != result) && (result!=-1 && i>0) ) 
			{
				iNGcount ++;
				continue; /* reading Next item because of FAIL item */
			}
			else
			{
				iNGcount ++;
				fprintf(stderr,"\n[%s]: ++ERROR++ result = %d, NGcount = %u\n",__FUNCTION__, result, iNGcount );
				continue; /* reading Next item because of FAIL item */
			}
			/* === 1 STEP : record (17 items check) =============== */

			if( (sq[0].iPATs05 < MODE_ID_NUMS) && (sq[0].iPATs05 >= 0) )
			{
				chkPATs_ModeID[ sq[0].iPATs05 ] ++; /* ModeID Counts */
			}
			else
			{
				fprintf(stderr,"++ERROR++ ModeID errors = %d, line:%u \n", sq[0].iPATs05, iOKcount );
				NGformatMODECnt ++; 
			}


			if( (sq[0].ShiTy12 < GEAR_SHI_NUMS) && (sq[0].ShiTy12 >= 0) )
			{
				gearShift[ sq[0].ShiTy12 ] ++;    /* Shift Type Counts */
			}
			else
			{
				fprintf(stderr,"++ERROR++ ShiftType errors = %u, line:%u \n", sq[0].ShiTy12, iOKcount );
				NGformatGEARCnt ++;				
			}


			/* === 2 STEP : SKIP record check ===================== */
			if(iItemCurOK)
			{
				iSave = 1;
			
				if(iItemCurOK && (iItemPreOK != iItemCurOK) ) iFirstItem = 1; /* First OK -> print */
			}
			iItemPreOK = iItemCurOK;
			/* === 2 STEP : SKIP record check ===================== */

			if(iFirstItem)
			{
				iFirstItem = 0;

		#ifdef LOW_PASS_FILTER_GACC /* 2023-02-09, Low Pass Filter for LAcc */
				/* ------------------------------------------------------------------- */
				/* -- Low Pass Filter initial Value for LAcclel ---------------------- */
				/* ------------------------------------------------------------------- */
				LPFilteredLAcc = sq[0].LAcc17;
		#endif

		#if SAVEMODE
				if(shiFile)
				{
					fprintf(shiFile, SAVEFMT,
							sq[0].Time01,    sq[0].iPATs05,                 arrPATs_ModeID[sq[0].iPATs05].ModeID, sq[0].VSP03,      sq[0].tqi07, 
							sq[0].curGear08, sq[0].APS09,                   sq[0].No10,                           sq[0].tgtGear11,  iShift,
							sq[0].ShiTy12,   arrGear[sq[0].ShiTy12].sGear,  sq[0].TqFr13,                         sq[0].ShiPh14,    sq[0].Ne15,
							sq[0].Nt16,      sq[0].LAcc17,                  LPFilteredLAcc,
							sTimePos, sTimeNtPos, gearRatio, gearShift1One, gearShift2Two, fJerk0, MaxAcc, minAcc );

					fprintf(shiFile, "\n");
				}
		#endif
			}


			/* ----------------------------------------------------- */
			/* Find Time Period     								 */
			/* ----------------------------------------------------- */
			if(iPreShTime)
			{
				avgTime  += ((unsigned int)(sq[0].Time01 * 1000) - iPreShTime);
				avgCount ++;
			}
			iPreShTime = (unsigned int)(sq[0].Time01 * 1000);
			/* ----------------------------------------------------- */
			/* Find Time Period     								 */
			/* ----------------------------------------------------- */

			
			/* === 3 STEP : ModeID check ===================== */
			if( !iSave ) 
			{
				continue; /* abnormal -> next record reading */
			}
			
			if( aiPATs05 != sq[0].iPATs05 )
			{
				continue; /* reading Next item because of no same ModeID item, example	ECO, SPT, NOR, ... */
			}

		#if 0
			if(sq[0].curGear08 == sq[0].tgtGear11)
			{
				continue; /* reading Next item because of same gear number, while shift gear.. 1->2, 2->3, 3->4 , ...  2->1, 3->2 */
			}
		#endif
			/* === 3 STEP : ModeID check ===================== */
			
			
			if( iShiftType == SHI_PWR_ON )
			{
				if( sq[0].APS09 < fAPSpwrLvl ) continue; /* APS power level check and ignored */
			}

			//if( (SHIFT_UP==shiDir03) && (sq[0].curGear08 != sq[0].tgtGear11) && (sq[0].curGear08+1 == sq[0].tgtGear11+0) )
			{
				if( 0 == sq[0].curGear08 ) continue; /* 0-DAN ignored */
				/* -- else OK -- */
			}
			//else if( (SHIFT_DN==shiDir03) && (sq[0].curGear08 != sq[0].tgtGear11) && (sq[0].curGear08+0 == sq[0].tgtGear11+1) )
			{
				if( 0 == sq[0].tgtGear11 ) continue; /* 0-DAN ignored */
				/* -- else OK -- */
			}
			//else if( (SHIFT_SKIP_DN==shiDir03) && (sq[0].curGear08 != sq[0].tgtGear11) && (sq[0].curGear08+2 >= sq[0].tgtGear11) )
			//{
			//	if( 0 == sq[0].tgtGear11 ) continue; /* 0-DAN ignored */
			//	/* -- else OK -- */
			//}
			//else 
			//{
			//	continue;
			//}


			/* ------------------------------------------------------------------- */
			/* -- NOT saving ----------------------------------------------------- */
			/* ------------------------------------------------------------------- */
			is2File = 0;



			/* ------------------------------------------------------------------- */
			/* ------------------------------------------------------------------- */
			if( (sq[0].iPATs05 == aiPATs05) && (sq[0].curGear08 == sq[0].tgtGear11) && (curSmPreGear+1==sq[0].curGear08) )
			{
				is2File = 0;

				/* There are SB, SP point. exist!!! */
				if( iExistSBpoint && iExistSPpoint ) 
				{
					if( curSmPreGear+1 == sq[0].curGear08 ) /* curGr == tgtGr */
					{
						//if( 0L == gMaxTbl[iSScount-1].SFTime )
						{
						strcpy(sTimePos, TXT_SFTIME);

						//gMaxTbl[iSScount-1].SFTime = (unsigned int)( (sq[0].Time01)*1000*JERK_TIME_SCALE );

				#if 0 //DEBUG_MSG_1ST_POINT_TIME
						fprintf(stderr,"aaa(%3d)-> SF:%9ld : SF-SP:%9.4lf, SP-SS:%9.4lf ", iSScount-1, gMaxTbl[iSScount-1].SFTime, 
								(double)(gMaxTbl[iSScount-1].SFTime - gMaxTbl[iSScount-1].SPTime)/1000/JERK_TIME_SCALE,
								(double)(gMaxTbl[iSScount-1].SPTime - gMaxTbl[iSScount-1].SSTime)/1000/JERK_TIME_SCALE );
				#endif

						iSFcount ++;

						is2File = 1;
						iOnce_SF = 1;
						isFindSS = 0; /* goto first SS */

						iExistSBpoint = 0;
						iExistSPpoint = 0;
						}

					}
				}
				else if( (curSmPreGear+1==sq[0].curGear08) && (tgtSmPreGear==sq[0].tgtGear11) )
				{
					iSFcount ++;

					is2File = 1;
					iOnce_SF = 1;
					strcpy(sTimePos, TXT_SFTIME);

					isFindSS = 0; /* goto first SS */
					
					//gMaxTbl[iSScount-1].SFTime = (unsigned int)( (sq[0].Time01)*1000*JERK_TIME_SCALE );

				}
				else
				{
					/* reading Next item because of same gear number, while shift gear.. 1->2, 2->3, 3->4 , ...  2->1, 3->2 */
					//continue;
				}

				/* CASE : (sq[0].curGear08 != sq[0].tgtGear11) -> initialized */
				//// -------- curPreGear = -1;
				//// -------- tgtPreGear = 0;
			}
			/* ------------------------------------------------------------------- */
			/* -- Power On / Up Shift -------------------------------------------- */
			/* ------------------------------------------------------------------- */
			else if( (sq[0].iPATs05 == aiPATs05) && (sq[0].curGear08 +1 == sq[0].tgtGear11) )
			{
				//printf(" DIFF -> cg=%d, tg=%d   local: cg(%d), tg (%d) global : cg (%d) tg (%d) \n", sq[0].curGear08, sq[0].tgtGear11, curPreGear, tgtPreGear , curSmPreGear, tgtSmPreGear );

				is2File = 1;

				iSortCount ++;

				iShift = sq[0].curGear08*10 + sq[0].tgtGear11; /* 12..23..34..45..56..67..78.. */ 
											 					/* 21..32..43..54..65..76..87.. */
				gearRatio = 0.0f;
				gearShift1One = 0.0f;
				gearShift2Two = 0.0f;
				if( (sq[0].No10>0.0 || sq[0].No10<0.0) )
				{
					gearRatio = (sq[0].Nt16/sq[0].No10); // gearTable[sq[0].curGear08];
				}
				gearShift1One  = (sq[0].Nt16 - sq[0].No10 * gearTable[sq[0].curGear08]);
				gearShift2Two  = (sq[0].Nt16 - sq[0].No10 * gearTable[sq[0].tgtGear11]);


		#ifdef LOW_PASS_FILTER_GACC /* 2023-02-09, Low Pass Filter for LAcc */
				/* -------------------------------------------------------------- */
				/* Low Pass Filter factor (0.102 = 10.2%) for JERK -------------- */
				/* curr(10.2%) + previous(89.8%) -------------------------------- */
				/* -------------------------------------------------------------- */
				LPFilteredLAcc = (JERK_LPFfactor * sq[0].LAcc17) + (1.0f - JERK_LPFfactor)*LPFilteredLAcc;
		#endif


		#ifndef LOW_PASS_FILTER_GACC /* 2023-02-09, Low Pass Filter for LAcc */

				/* ----------------------------------------------------- */
				/* Calc Jerk0 (per 5msec) 								 */
				/* ----------------------------------------------------- */
				fJerk0 = 0.0f;
				if(preTime)
				{
					DiffTime  = ((unsigned int)(sq[0].Time01 * 1000 * JERK_TIME_SCALE) - preTime);
					if(DiffTime)
					{
						fJerk0 = (sq[0].LAcc17 * 1000 - preLAcc)*JERK_TIME_SCALE/(DiffTime); /* UNIT: m2/sec/msec */
					}
				}
				preTime = (unsigned int)(sq[0].Time01 * 1000 * JERK_TIME_SCALE);
				preLAcc = (sq[0].LAcc17 * 1000);
				/* ----------------------------------------------------- */
				/* Calc Jerk0 (per 5msec) 								 */
				/* ----------------------------------------------------- */
		#else
				/* ----------------------------------------------------- */
				/* Calc Jerk0 (per 5msec) Low Pass Filtered  ----------- */
				/* ----------------------------------------------------- */
				fJerk0 = 0.0f;
				if(preTime)
				{
					DiffTime  = ((unsigned int)(sq[0].Time01 * 1000 * JERK_TIME_SCALE) - preTime);
					if(DiffTime)
					{
						fJerk0 = (LPFilteredLAcc * 1000 - preLAcc)*JERK_TIME_SCALE/(DiffTime); /* UNIT: m2/sec/msec */
					}
				}
				preTime = (unsigned int)(sq[0].Time01 * 1000 * JERK_TIME_SCALE);
				preLAcc = (LPFilteredLAcc * 1000);
				/* ----------------------------------------------------- */
				/* Calc Jerk0 (per 5msec)								 */
				/* ----------------------------------------------------- */

		#endif /* LOW_PASS_FILTER_GACC */



				
				/* ----------------------------------------------------- */
				/* Find SSTime Position                                  */
				/* ----------------------------------------------------- */
				if(0==isFindSS)
				{
					if( (curPreGear != sq[0].curGear08) && (tgtPreGear != sq[0].tgtGear11) ) 
					//if( (sq[0].curGear08 +1 == sq[0].tgtGear11) && (curSmPreGear==sq[0].curGear08) && (curSmPreGear==tgtSmPreGear) )
					{
						isFindSS = 1;
					}
					else if( iOnce_SF && (curPreGear == sq[0].curGear08) && (tgtPreGear == sq[0].tgtGear11) )
					{
						isFindSS = 1;
					}

					curPreGear = sq[0].curGear08;
					tgtPreGear = sq[0].tgtGear11;
				}


				if( isFindSS ) // && (sq[0].curGear08 != sq[0].tgtGear11) )
				{

					ignoredRecord = 0; /* PASS(OK)  at SS point */
					g_SBtime	= -1.0f;
					g_SPtime	= -1.0f;
					g_SFtime	= -1.0f;

			#ifdef LOW_PASS_FILTER_GACC /* 2023-02-09, Low Pass Filter for LAcc */
					LPFilteredLAcc = sq[0].LAcc17;
			#endif

			
					isFindSS = 0;
					strcpy(sTimePos, TXT_SSTIME);
					strcpy(sTimeNtPos, TXT_UPCASE); /* Up case default STRING -- */


					if( iSScount >= MAX_TABLE_SIZ )
					{
						fprintf(stderr,"++ERROR+%d+ gMaxTbl[%d] Table index over!! \n", __LINE__, iSScount);
					} 
					else
					{
					//gMaxTbl[iSScount].SSTime = (unsigned int)( (sq[0].Time01)*1000*JERK_TIME_SCALE );
					//gMaxTbl[iSScount].SFTime = 0L;

				#if 0 //DEBUG_MSG_1ST_POINT_TIME
						fprintf(stderr,"\nZZZ(%3d)-> SS:%9ld ", iSScount, gMaxTbl[iSScount].SSTime );
				#endif

					}

					iSScount ++;

					iOnce_SS = 1;

					/* initialized */
					iOnce_SB = 1;
					iOnce_SP = 1;
					iOnce_SF = 0;

					iExistSBpoint = 0;
					iExistSPpoint = 0;

					iOnce_NtMax = 0; /* Power On & Upshift -> NtMax */
					iOnce_Ntmin = 0; /* Power On & Upshift -> Ntmin */
					
					MaxAcc = 0.0f;
					minAcc = GMAX_RVALUE;
					

					/* SB decision initialized */
					memset(SBdecision, 0x00, SB_DECISION_NUM*sizeof(tSBdec_type) );
					iSBdecCnt  = 0;
					iSBstart   = 0;
					iSBTimeSum = 0;
					iSBpntFix  = 0;
					iSBswingCnt = 0;

					/* SP decision initialized */
					memset(SPdecision, 0x00, SP_DECISION_NUM*sizeof(tSPdec_type) );
					iSPdecCnt  = 0;
					iSPstart   = 0;
					iSPTimeSum = 0;
					iSPpntFix  = 0;
					iSPswingCnt = 0;

					/* Nt Max decision initialized */
					memset(NtMaxdecision, 0x00, NtMax_DECISION_NUM*sizeof(tNtMaxdec_type) );
					iNtMaxdecCnt  = 0;
					iNtMaxstart   = 0;
					iNtMaxTimeSum = 0;
					iNtMaxpntFix  = 0;
					iNtMaxswingCnt = 0;

					/* Nt min decision initialized */
					memset(Ntmindecision, 0x00, Ntmin_DECISION_NUM*sizeof(tNtmindec_type) );
					iNtmindecCnt  = 0;
					iNtminstart   = 0;
					iNtminTimeSum = 0;
					iNtminpntFix  = 0;
					iNtminswingCnt = 0;

				}
				else
				{
					/* initialized string */
					strcpy(sTimePos, TXT_UPCASE);
					strcpy(sTimeNtPos, TXT_UPCASE);

					if( SHIFT_UP==shiDir03 ) /* Power On, Power Off case */
					{

						/* -------------------------------------------------------------------------------
						※	1-2 : Shift Begin (SB) 점 판단식 : Nt - No x 1단 기어비 ≤-30RPM 되는 싯점 
								  Synch Point (SP) 점 판단식 : Nt - No x 2단 기어비 ≤ 30RPM 되는 싯점	 
								  Nt Max 점 판단식			 : Nt - No x 1단 기어비 ≤-5RPM 되는 싯점 
								  Nt min 점 판단식			 : Nt - No x 2단 기어비 ≤5RPM 되는 싯점 
						---------------------------------------------------------------------------------- */

				#if 0 // SB_POINT_SWING1
						/* SB 튀는 시점 발생 -> */
						if( (iOnce_SS) && (0==iOnce_SB) && (gearShift1One > UP_SB_PNT_RPM) && (pre_gearShift1One < (gearShift1One-fSBswingLvl /*SB_SWING_LEVEL*/)) )
						{
							iOnce_SB = 1; // re-SB point
							iSBpntFix = 1;
							pre_gearShift1One = gearShift1One;

							iSBnewPoint[iSBswingCnt] = iSBcount;
							iSBswingCnt ++;
							fprintf(stderr,"  SB re-point because of rpm ratio (%.1f) swing... SBcount:%3u -> %u : *\n", fSBswingLvl, iSBswingCnt, iSBcount );
						}
				#endif

					/* =============================================================== */
					/* 1) Shift Begin (SB) decision checking, contunous 3 times  ----- */
					/* =============================================================== */
						if( 0==iSBpntFix && (gearShift1One <= UP_SB_PNT_RPM) ) /* Shift Begin (SB) -30 rpm under */
						{	
							if(iSBdecCnt < SB_DECISION_NUM)
							{
								SBdecision[iSBdecCnt].FixCount = 1; // 1 times
								SBdecision[iSBdecCnt].SBTime = (unsigned int)( (sq[0].Time01)*1000*JERK_TIME_SCALE ); // 1 times
								iSBstart = 1;
								iOnce_SB = 1; // re-enterance
								//fprintf(stderr,"  SB decision SBdecision: %d:(%d) \n", iSBdecCnt, SBdecision[iSBdecCnt].FixCount );
							}
							else
								fprintf(stderr, "  SB decision point too many points... %d \n", iSBdecCnt );
						}
						else
						{
							iSBdecCnt  = 0;
							iSBstart   = 0;
							iSBTimeSum = 0;
							memset(SBdecision, 0x00, SB_DECISION_NUM*sizeof(tSBdec_type) );
						}

						if(iSBstart)
							iSBdecCnt ++;

						iSBTimeSum = 0; /* clear before summation */
						for(ide=0; ide<iSBdecCnt; ide++)
							iSBTimeSum += SBdecision[ide].FixCount;

						if( iSBTimeSum >= iSBdecision /* SB_DECISION_TIMES */) 
						{
							iSBpntFix=1;
							//fprintf(stderr,"  SB decision ---- OK (%d) \n", iSBTimeSum );
						}
						/* =============================================================== */
						/* 1) Shift Begin (SB) decision checking, contunous 3 times  ----- */
						/* =============================================================== */
						

						/* ====================================================================== */
						/* 2) Synchronizing Point (SP) decision checking, , contunous 3 times --- */
						/* ====================================================================== */
						if( (1==iSBpntFix) && (0==iSPpntFix) && (gearShift2Two <= UP_SP_PNT_RPM) ) /* SP after SB */
						{	
							if(iSPdecCnt < SP_DECISION_NUM)
							{
								SPdecision[iSPdecCnt].FixCount = 1; // 1 times
								SPdecision[iSPdecCnt].SPTime = (unsigned int)( (sq[0].Time01)*1000*JERK_TIME_SCALE ); // 1 times
								iSPstart = 1;
								iOnce_SP = 1;
								//fprintf(stderr,"  SP decision SPdecision: %d:(%d) \n", iSPdecCnt, SPdecision[iSPdecCnt].FixCount );
							}
							else
								fprintf(stderr, "  SP decision point too many points... %d \n", iSPdecCnt );
						}
						else
						{
							iSPdecCnt  = 0;
							iSPstart   = 0;
							iSPTimeSum = 0;
							memset(SPdecision, 0x00, SP_DECISION_NUM*sizeof(tSPdec_type) );
						}

						if(iSPstart)
							iSPdecCnt ++;

						iSPTimeSum = 0; /* clear before summation */
						for(ide=0; ide<iSPdecCnt; ide++)
							iSPTimeSum += SPdecision[ide].FixCount;

						if( iSPTimeSum >= iSBdecision /* SP_DECISION_TIMES */) 
						{
							iSPpntFix=1;
							//fprintf(stderr,"  SP decision ---- OK (%d) \n", iSPTimeSum );
						}
						/* ====================================================================== */
						/* 2) Synchronizing Point (SP) decision checking, contunous 3 times ----- */
						/* ====================================================================== */


				#if 1
						/* ====================================================================== */
						/* 3) NtMax decision checking, continous 3 times ------------------------ */
						/* ====================================================================== */
						iCurrTime = (unsigned int)( (sq[0].Time01)*1000*JERK_TIME_SCALE );
						if( gMaxTbl[iSScount-1].SBTime >= iCurrTime && gMaxTbl[iSScount-1].NtMaxBegin <= iCurrTime )
						{
							if( 0==iNtMaxpntFix && (gearShift1One <= UP_NtMax_PNT_RPM) ) /* NtMax gear1 <= 0 rpm under */
							{	
								if(iNtMaxdecCnt < NtMax_DECISION_NUM)
								{
									NtMaxdecision[iNtMaxdecCnt].FixCount = 1; // 1 times
									NtMaxdecision[iNtMaxdecCnt].NtMaxTime = (unsigned int)( (sq[0].Time01)*1000*JERK_TIME_SCALE );
									iNtMaxstart = 1;
									iOnce_NtMax = 1; // re-enterance
									//fprintf(stderr,"  iNtMaxdecCnt NtMaxdecision: %d:(%d) \n", iNtMaxdecCnt, NtMaxdecision[iNtMaxdecCnt].FixCount );
								}
								else
									fprintf(stderr, "  iNtMaxdecCnt point too many points... %d \n", iNtMaxdecCnt );
							}
							else
							{
								iNtMaxdecCnt  = 0;
								iNtMaxstart   = 0;
								iNtMaxTimeSum = 0;

								memset(NtMaxdecision, 0x00, NtMax_DECISION_NUM*sizeof(tNtMaxdec_type) );
							}

							if(iNtMaxstart)
								iNtMaxdecCnt ++;

							iNtMaxTimeSum = 0; /* clear before summation */
							for(ide=0; ide<iNtMaxdecCnt; ide++)
								iNtMaxTimeSum += NtMaxdecision[ide].FixCount;

							if( iNtMaxTimeSum >= iSBdecision /* NtMax_DECISION_NUM */ ) 
							{
								iNtMaxpntFix=1;
								//fprintf(stderr,"  iNtMaxTimeSum ---- OK (%d) \n", iNtMaxTimeSum );
							}
						}
						/* ====================================================================== */
						/* 3) NtMax decision checking, continous 3 times ------------------------ */
						/* ====================================================================== */
						

						/* ====================================================================== */
						/* 4) Ntmin decision checking, continous 3 times ------------------------ */
						/* ====================================================================== */

						/* Nt-min Value : SP point ~ SP + 50msec point 사이 */
						iCurrTime = (unsigned int)( (sq[0].Time01)*1000*JERK_TIME_SCALE );
						if( gMaxTbl[iSScount-1].SPTime <= iCurrTime && gMaxTbl[iSScount-1].NtminEnd >= iCurrTime )
						{

							if( 0==iNtminpntFix && (gearShift2Two <= UP_Ntmin_PNT_RPM) ) /* Ntmin gear2 <= 0 rpm under */
							{	
								if(iNtmindecCnt < Ntmin_DECISION_NUM)
								{
									Ntmindecision[iNtmindecCnt].FixCount = 1; // 1 times
									Ntmindecision[iNtmindecCnt].NtminTime = (unsigned int)( (sq[0].Time01)*1000*JERK_TIME_SCALE );
									iNtminstart = 1;
									iOnce_Ntmin = 1; // re-enterance
									//fprintf(stderr,"  iNtmindecCnt Ntmindecision: %d:(%d) \n", iNtmindecCnt, Ntmindecision[iNtmindecCnt].FixCount );
								}
								else
									fprintf(stderr, "  iNtmindecCnt point too many points... %d \n", iNtmindecCnt );
							}
							else
							{
								iNtmindecCnt  = 0;
								iNtminstart   = 0;
								iNtminTimeSum = 0;
								memset(Ntmindecision, 0x00, Ntmin_DECISION_NUM*sizeof(tNtmindec_type) );
							}

							if(iNtminstart)
								iNtmindecCnt ++;

							iNtminTimeSum = 0; /* clear before summation */
							for(ide=0; ide<iNtmindecCnt; ide++)
								iNtminTimeSum += Ntmindecision[ide].FixCount;

							if( iNtminTimeSum >= iSBdecision /* Ntmin_DECISION_NUM */ ) 
							{
								iNtminpntFix=1;
								//fprintf(stderr,"  iNtminTimeSum ---- OK (%d) \n", iNtminTimeSum );
							}

						}
						/* ====================================================================== */
						/* 4) Ntmin decision checking, continous 3 times ------------------------ */
						/* ====================================================================== */
				#endif



						/* =============================================================== */
						/* STRING Fix ---------------------------------------------------- */
						/* Shift Begin (SB) -30 rpm under decision ----------------------- */
						/* =============================================================== */
						if( iOnce_SB && (gearShift1One <= UP_SB_PNT_RPM) ) /* Shift Begin (SB) -30 rpm under */
						{
							iOnce_SB = FALSE;

							if( iSBpntFix ) 
							{
								iExistSBpoint = 1;

								strcpy(sTimePos, TXT_SBTIME); 
								//strcpy(sTimePos, TXT_SBSWING0);

								if( iSBcount >= MAX_TABLE_SIZ )
								{
									fprintf(stderr,"++ERROR+%d+ SB gMaxTbl[%d] Table index over!! \n", __LINE__, iSBcount);
								} 
								else
								{

					#if 0
								gMaxTbl[iSScount-1].Index      = iSBcount;
								//gMaxTbl[iSBcount].SBTime     = (sq[0].Time01)*1000*JERK_TIME_SCALE;  /* Last One Choice */
								//gMaxTbl[iSBcount].SBTime     = SBdecision[0].SBTime;  /* first One time choice */ 
								gMaxTbl[iSScount-1].SBTime     = SBdecision[SBposDecision].SBTime;  /* first posistion or Last Position time choice */ 

								gMaxTbl[iSScount-1].gMx1Begin  = gMaxTbl[iSScount-1].SBTime - iJerkTimeLen*JERK_TIME_SCALE;  /* JERK_TIME_mSec */
								gMaxTbl[iSScount-1].gmn1End    = gMaxTbl[iSScount-1].SBTime; 

								gMaxTbl[iSScount-1].NtMaxBegin = gMaxTbl[iSScount-1].SBTime - NtMax_Before_mSec*JERK_TIME_SCALE;	/* JERK_TIME_mSec */

							#if 0 //DEBUG_MSG_1ST_POINT_TIME
									fprintf(stderr,"aaa(%3d)-> SB:%9ld ", iSScount-1, gMaxTbl[iSScount-1].SBTime );
							#endif
					#endif
					
								}


								iSBcount ++;
							}
							else
							{
								strcpy(sTimePos, TXT_SBSWING0);
								iSBswingCnt++;
							}

							g_SBtime = sq[0].Time01;

						}

						/* =============================================================== */
						/* Synchronizing Poing (SP) 30 rpm under decision ----------------- */
						/* =============================================================== */
						if( iOnce_SP && (gearShift2Two <= UP_SP_PNT_RPM) ) /* Synchronizing Point (SP) 30 rpm under */
						{
							iOnce_SP = FALSE;

							if( iSPpntFix ) 
							{
								iExistSPpoint = 1;
								strcpy(sTimePos, TXT_SPTIME);
								//strcpy(sTimePos, TXT_SPSWING0);

								if( iSPcount >= MAX_TABLE_SIZ )
								{
									fprintf(stderr,"++ERROR+%d+ SP gMaxTbl[%d] Table index over!! \n", __LINE__, iSPcount);
								} 
								else
								{

					#if 0
								//gMaxTbl[iSPcount].Index     = iSPcount;
								//gMaxTbl[iSPcount].SPTime    = SPdecision[0].SBTime;  /* first One time choice */ 
								gMaxTbl[iSScount-1].SPTime    = SPdecision[SBposDecision].SPTime;  /* first One time choice */ 
								//gMaxTbl[iSPcount].gmMxBegin = gMaxTbl[iSPcount].SPTime - iJerkTimeLen*JERK_TIME_SCALE;  /* JERK_TIME_mSec */

								gMaxTbl[iSScount-1].NtminEnd  = gMaxTbl[iSScount-1].SPTime + Ntmin_After_mSec*JERK_TIME_SCALE;	/* Nt-min end time */

							#if DEBUG_MSG_1ST_POINT_TIME
									fprintf(stderr,"aaa(%3d)-> SP:%9ld ", iSScount-1, gMaxTbl[iSScount-1].SPTime );
							#endif
					#endif
								}
								
								iSPcount ++;
							}
							else
							{
								strcpy(sTimePos, TXT_SPSWING0);
								iSPswingCnt++;
							}


							g_SPtime = sq[0].Time01;
							iOnce_SS = 0;
						}


				#if 1
						/* =============================================================== */
						/* Nt Max gear1 <= 0 rpm under decision -------------------------- */
						/* =============================================================== */
						if( gMaxTbl[iSScount-1].SBTime >= iCurrTime && gMaxTbl[iSScount-1].NtMaxBegin <= iCurrTime )
						{
							if( iOnce_NtMax && (gearShift1One <= UP_NtMax_PNT_RPM) ) /* NtMax gear1 0 rpm under */
							{
								iOnce_NtMax = FALSE;

								if( iNtMaxpntFix ) 
								{

									strcpy(sTimeNtPos, TXT_NtMaxTIME);

									if( iNtMaxcount >= MAX_TABLE_SIZ )
									{
										fprintf(stderr,"++ERROR+%d+ NtMax gMaxTbl[%d] Table index over!! \n", __LINE__, iNtMaxcount);
									} 
									else
									{
									//gMaxTbl[iNtMaxcount].Index	 = iNtMaxcount;
									gMaxTbl[iSScount-1].NtMaxTime = NtMaxdecision[SBposDecision].NtMaxTime;	/* first One time choice */ 
									}
									iNtMaxcount ++;
								}
								else
								{
									strcpy(sTimeNtPos, TXT_NtMaxSWING0);
									iNtMaxswingCnt++;
								}

								g_NtMaxtime = sq[0].Time01;

							}
						}

						/* =============================================================== */
						/* Nt min gear2 <= 0 rpm under decision -------------------------- */
						/* =============================================================== */
						if( gMaxTbl[iSScount-1].SPTime <= iCurrTime && gMaxTbl[iSScount-1].NtminEnd >= iCurrTime )
						{
							if( iOnce_Ntmin && (gearShift2Two <= UP_Ntmin_PNT_RPM) ) /* Ntmin gear2 0 rpm under */
							{
								iOnce_Ntmin = FALSE;

								if( iNtminpntFix ) 
								{

									strcpy(sTimeNtPos, TXT_NtminTIME);

									if( iNtmincount >= MAX_TABLE_SIZ )
									{
										fprintf(stderr,"++ERROR+%d+ Ntmin gMaxTbl[%d] Table over!! \n", __LINE__, iNtmincount);
									} 
									else
									{
									//gMaxTbl[iNtmincount].Index	 = iNtmincount;
									gMaxTbl[iSScount-1].NtminTime = Ntmindecision[SBposDecision].NtminTime;	/* first One time choice */ 
									}

									iNtmincount ++;
								}
								else
								{
									strcpy(sTimeNtPos, TXT_NtminSWING0);
									iNtminswingCnt++;
								}

								g_Ntmintime = sq[0].Time01;

							}
						}
				#endif


					}
					else if( SHIFT_DN==shiDir03 ) /* Power On, Power Off case */
					{
						/* ------------------------------------------------------------------------------
						※ 2-1 : Shift Begin (SB) 점 판단식 : Nt - No x 1단 기어비 ≥ 30RPM 되는 싯점 
								 Synch Point (SP) 점 판단식 : Nt - No x 2단 기어비 ≥-30RPM 되는 싯점	 
						--------------------------------------------------------------------------------- */

						if( iOnce_SB && (gearShift1One >= SB_PNT_RPM_DNS) ) /* Shift Begin (SB) -30 rpm under */
						{
							iOnce_SB = FALSE;
							strcpy(sTimePos, TXT_SBTIME);
							iSBcount ++;

							g_SBtime = sq[0].Time01;
						}
						else if( iOnce_SP && (gearShift2Two >= SP_PNT_RPM_DNS) ) /* 30 rpm under */
						{
							iOnce_SP = FALSE;
							strcpy(sTimePos, TXT_SPTIME);
							iSPcount ++;

							g_SPtime = sq[0].Time01;
						}
						else
						{
							strcpy(sTimePos, TXT_DNCASE);
						}

					}
					else if( SHIFT_SKIP_DN==shiDir03 )
					{


					}
					else if( SHIFT_PNDR==shiDir03 ) /* Static shift */
					{
						/* ---------------------------------------------------------------------------------
							P/N-D/R : Shift Begin  (SB) 점 판단식 : Nt - Nt_max ≤ -30RPM 되는 싯점 
						              Shift Finish (FF) 점 판단식 : Nt ≤ 30RPM 되는 싯점	 
					      ---------------------------------------------------------------------------------- */


					}
					else
					{
						strcpy(sTimePos, TXT_UNKNOWN);
					}
				}
				/* ----------------------------------------------------- */
				/* Find SSTime Position                                  */
				/* ----------------------------------------------------- */


				/* --------------------------------------------------------------
				Power ON UP-shift CASE >> 
				 (1) Nt_max와 Ne_max는 SB 시점값이고, 
				 (2) Nt_min과 Ne_min은 FF 시점값을 취합니다.

				 (3) G_max와G_min는 SS에서 SB 구간 중 최대값과 최소값입니다.
				 (4) △G 는 (SB~FF의 G_평균값과 FF~SF의 G_평균값)의 차이값입니다.

				Power ON DOWN-shift CASE >>
				 (1) Nt_min와 Ne_min은 SB 시점값이고, 
				 (2) Nt_max과 Ne_max는 FF 시점값을 취합니다.
				 (3) j1 >> G_max는 SB~FF사이에서 G_최대값을 취하고, 
				     G_min은 G_최대값 직후에 오는 G_최소값을 취합니다.
				 (4) j2를 구하기 위한 G_min은 FF~SF사이에서 G_최소값을 취하고, 
				     G_max는 G_최소값 직후에 오는 G_최대값을 취합니다.
				----------------------------------------------------------------- */


				/* -------------------------------------------------------------- */
				/* g_Max, g_min  ------------------------------------------------ */
				/* -------------------------------------------------------------- */

			#ifndef LOW_PASS_FILTER_GACC /* 2023-02-09, Low Pass Filter for LAcc */
				if( MaxAcc <= sq[0].LAcc17 ) MaxAcc = sq[0].LAcc17; /* Find g_Max */
				if( minAcc >= sq[0].LAcc17 ) minAcc = sq[0].LAcc17; /* Find g_mix */
			#else
				/* Low Pass Filtered data for LAcc */
				if( MaxAcc <= LPFilteredLAcc ) MaxAcc = LPFilteredLAcc; /* Find g_Max */
				if( minAcc >= LPFilteredLAcc ) minAcc = LPFilteredLAcc; /* Find g_mix */
			#endif
			
				/* -------------------------------------------------------------- */
				/* g_Max, g_min  ------------------------------------------------ */
				/* -------------------------------------------------------------- */



			#if 0 // SAVEMODE
				if(shiFile)
				{
				fprintf(shiFile, SAVEFMT,
						sq[0].Time01,	 sq[0].iPATs05, arrPATs_ModeID[sq[0].iPATs05].ModeID, sq[0].VSP03, sq[0].tqi07, 
						sq[0].curGear08, sq[0].APS09,	sq[0].No10, sq[0].tgtGear11, 
						iShift, 		 sq[0].ShiTy12, arrGear[sq[0].ShiTy12].sGear, sq[0].TqFr13, sq[0].ShiPh14, 
						sq[0].Ne15, 	 sq[0].Nt16,    sq[0].LAcc17, 
						sTimePos, sTimeNtPos, gearRatio, gearShift1One, gearShift2Two, fJerk0, MaxAcc, minAcc );

				fprintf(shiFile, "\n");
				}
			#endif


			}


			curSmPreGear = sq[0].curGear08;
			tgtSmPreGear = sq[0].tgtGear11;

	#if SAVEMODE
			if(shiFile && is2File)
			{
			fprintf(shiFile, SAVEFMT,
					sq[0].Time01,	 sq[0].iPATs05, arrPATs_ModeID[sq[0].iPATs05].ModeID, sq[0].VSP03, sq[0].tqi07, 
					sq[0].curGear08, sq[0].APS09,	sq[0].No10, sq[0].tgtGear11, 
					iShift, 		 sq[0].ShiTy12, arrGear[sq[0].ShiTy12].sGear, sq[0].TqFr13, sq[0].ShiPh14, 
					sq[0].Ne15, 	 sq[0].Nt16,    sq[0].LAcc17,    LPFilteredLAcc,
					sTimePos, sTimeNtPos, gearRatio, gearShift1One, gearShift2Two, fJerk0, MaxAcc, minAcc );

			fprintf(shiFile, "\n");
			}
	#endif


		}
	}
	while (!feof (inpfile));



	if(shiFile) { fclose(shiFile); shiFile=NULL; }
	if(inpfile) { fclose(inpfile); inpfile=NULL; }


	avg_t1 = 0L;
	avg_t2 = 0L;
	avg_t3 = 0L;
	iSxIgnCnt = 0;

	//fprintf(stderr, "%d %d %d \n", iSScount, iSBcount, iSPcount );
	for(ii=0; ii<iSScount; ii++)
	{
		if( gMaxTbl[ii].SSTime && gMaxTbl[ii].SBTime && gMaxTbl[ii].SPTime && gMaxTbl[ii].SFTime )
		{
		dif_t1 = (gMaxTbl[ii].SBTime - gMaxTbl[ii].SSTime)/JERK_TIME_SCALE;
		dif_t2 = (gMaxTbl[ii].SPTime - gMaxTbl[ii].SBTime)/JERK_TIME_SCALE;

		dif_t3 = (gMaxTbl[ii].SFTime - gMaxTbl[ii].SPTime)/JERK_TIME_SCALE;
		}
		else
		{
			iSxIgnCnt ++;
			gMaxTbl[ii].ignored = IGN_BECAUSE_NOPAIR;
	#if DEBUG_MSG_1ST_POINT_TIME
			fprintf(stderr," NONE [%3d]: SS(%10ld) -SB(%10ld) -SP(%10ld) -SF(%10ld) \n", 
							ii, gMaxTbl[ii].SSTime, gMaxTbl[ii].SBTime, gMaxTbl[ii].SPTime, gMaxTbl[ii].SFTime );
	#endif
		}


		#if 0
		/* Ignored Case checking -- SS~SP durtion */
		if( (dif_t1 + dif_t2) > SS2SP_OVER_TIME )
			gMaxTbl[ii].ignored = 1;

		/* Ignored Case checking -- SP~SF durtion */
		if(  dif_t3 > SP2SF_OVER_TIME )
			gMaxTbl[ii].ignored = 1;
		#endif

		
	#if 0
		if( (dif_t1 + dif_t2) < JERK_min_TIME_mSec )
		{
			fprintf(stderr, "  SS~SP under time : %5d -> %9lld msec = SS(%.4lf), SB(%.4lf), SP(%.4lf) sec \n", 
				ii, (dif_t1+dif_t2) , gMaxTbl[ii].SSTime/1000.0/JERK_TIME_SCALE, gMaxTbl[ii].SBTime/1000.0/JERK_TIME_SCALE, gMaxTbl[ii].SPTime/1000.0/JERK_TIME_SCALE  );
		}

		if( (dif_t1 + dif_t2) > JERK_MAX_TIME_mSec )
		{
			fprintf(stderr, "  SS~SP over time  : %5d -> %9lld msec = SS(%.4lf), SB(%.4lf), SP(%.4lf) sec \n", 
				ii, (dif_t1+dif_t2) , gMaxTbl[ii].SSTime/1000.0/JERK_TIME_SCALE, gMaxTbl[ii].SBTime/1000.0/JERK_TIME_SCALE, gMaxTbl[ii].SPTime/1000.0/JERK_TIME_SCALE  );
		}
	#endif
	
		avg_t1 += dif_t1;
		avg_t2 += dif_t2;
		avg_t3 += dif_t3;

		//fprintf(stderr, " %2d -> %10lld  %10lld  \n", ii, avg_t1, avg_t1 );
 	}


	if(iSScount+1-iSxIgnCnt)
	{
		avg_t1 /= (iSScount+1-iSxIgnCnt);
		avg_t2 /= (iSScount+1-iSxIgnCnt);
		avg_t3 /= (iSScount+1-iSxIgnCnt);
	}
	
	if(avgCount)
		iavgTime = (unsigned int)(avgTime/avgCount);

	
	//fprintf(stderr, "** %2d -> %10llu  %10llu \n", ii, avg_t1, avg_t2 );
	
	//fprintf(stderr,"----------------------------------------------------------------------------------\n" );
	//fprintf(stderr,">>ModeID %s files are saved. \n", arrPATs_ModeID[aiPATs05].ModeID);
	fprintf(stderr,"  ModeID %s, 1st step sorting file: %s  \n", arrPATs_ModeID[aiPATs05].ModeID, shift_out );
	//fprintf(stderr,"  ModeID file: %s \n", shift_file );


	//fprintf(stderr,"----------------------------------------------------------------------------------\n" );
	fprintf(stderr,"==================================================================================\n" );
	fprintf(stderr,">>Quality Shift Data Sorting Summary... %s \n", arrPATs_ModeID[aiPATs05].ModeID );
	if(iSBdecision>=3)
	{
		if(0==SBposDecision)
			fprintf(stderr,"  cont. SB/SP decision Number  : %d times, such as %s ", iSBdecision, PRINT_TXT_SB_SEQ_FIRST );
		else
			fprintf(stderr,"  cont. SB/SP decision Number  : %d times, such as %s ", iSBdecision, PRINT_TXT_SB_SEQ_LAST );

		if(SBposDecision) fprintf(stderr,"- last position \n"); 
		else fprintf(stderr,"- first position \n"); 
	}
	else if(iSBdecision==2)
	{
		if(0==SBposDecision)
			fprintf(stderr,"  cont. SB/SP decision Number  : %d times, such as %s.%s ", iSBdecision, TXT_SBTIME, TXT_SBSWING0 );
		else
			fprintf(stderr,"  cont. SB/SP decision Number  : %d times, such as %s.%s ", iSBdecision, TXT_SBSWING0, TXT_SBTIME );

		if(SBposDecision) fprintf(stderr,"- last position \n"); 
		else fprintf(stderr,"- first position \n"); 
	}
	else if(iSBdecision<=1)
		fprintf(stderr,"  cont. SB/SP decision Number  : %d (Just One) - SB/SP-point can be determined wrongly. \n", iSBdecision );

	fprintf(stderr,"  Jerk Time length (gMax/gmin) : -%d msec ~ (SB point) ~ %d msec \n", iJerkTimeLen, iavgTime*gMax_Before_SB_POINT );
	fprintf(stderr,"  Nt-Max/Nt-min Time length    : -%d msec ~ (SB point) / (SP point) ~ +%d msec \n", iNtTimeLen, iNtTimeLen );

	fprintf(stderr,"  Shift Data Time Period ------: %9u msec \n", iavgTime );
	fprintf(stderr,"  Shift Average Time(t1:SS~SB) : %9ld msec <- 1st average time \n", avg_t1 );
	fprintf(stderr,"  Shift Average Time(t2:SB~SP) : %9ld msec <- 1st average time \n", avg_t2 );
	fprintf(stderr,"  Shift Average Time(t3:SP~SF) : %9ld msec <- 1st average time \n", avg_t3 );
	fprintf(stderr,"  Total Quality Shift Records  : %9llu lines \n", RecordCnt );
	fprintf(stderr,"  Error Shift Records (NG) ----: %9u lines <- invalid shift data record \n", iNGcount );
	fprintf(stderr,"  Quality Shift Records (OK) --: %9u lines, %6.1lf min \n", iOKcount, (iOKcount*iavgTime)/1000.0/60 );


	totChkModeID = 0;
	for(ii=0; ii<MODE_ID_NUMS-1 ;ii++)
	{
		if( chkPATs_ModeID[ii] > 0UL )
		{
			fprintf(stderr,"    %3u:%-22s : %9llu lines, %6.1lf min  %s \n", ii, arrPATs_ModeID[ii].ModeID, chkPATs_ModeID[ii], (double)(chkPATs_ModeID[ii]*iavgTime)/1000.0/60, (aiPATs05==ii?"* Sorting":" ") );
			totChkModeID += chkPATs_ModeID[ii];
		}
	}
	fprintf(stderr,"  Quality Sum Records ---------: %9u lines \n", totChkModeID );
	fprintf(stderr,"  Shift Sorted Rec %-11s : %9u/ %llu lines \n", arrPATs_ModeID[aiPATs05].ModeID, iSortCount, chkPATs_ModeID[aiPATs05] );
	fprintf(stderr,"     SS Point Counts--%-8s : %9u points \n", (shiDir03==SHIFT_UP?"Up":(shiDir03==SHIFT_DN?"Down":(shiDir03==SHIFT_SKIP_DN?"SkipDn":"Unknown"))), iSScount );
	fprintf(stderr,"     SB Point Counts--%-8s : %9u points \n", (shiDir03==SHIFT_UP?"Up":(shiDir03==SHIFT_DN?"Down":(shiDir03==SHIFT_SKIP_DN?"SkipDn":"Unknown"))), iSBcount );
	fprintf(stderr,"     SP Point Counts--%-8s : %9u points \n", (shiDir03==SHIFT_UP?"Up":(shiDir03==SHIFT_DN?"Down":(shiDir03==SHIFT_SKIP_DN?"SkipDn":"Unknown"))), iSPcount );
	fprintf(stderr,"     SF Point Counts--%-8s : %9u points \n", (shiDir03==SHIFT_UP?"Up":(shiDir03==SHIFT_DN?"Down":(shiDir03==SHIFT_SKIP_DN?"SkipDn":"Unknown"))), iSFcount );
	fprintf(stderr,"----------------------------------------------------------------------------------\n" );



	Find_GearMode( gearShift, PWR_ON_UP_SHIFT, iavgTime );


	/* When Same -> */
	SPoint->SSnum = iSScount;
	SPoint->SBnum = iSBcount;
	SPoint->SPnum = iSPcount;
	SPoint->SFnum = iSFcount;

	/* When NOT same -> */
	SPoint->SStot = iSScount;
	SPoint->SBtot = iSBcount;
	SPoint->SPtot = iSPcount;
	SPoint->SFtot = iSFcount;

	*SBswingcnt = iSBdecCnt; /* One checking */
	

	return iavgTime;
}



int SSnNtPointFix(short aiPATs05, short SBposDecision, tSQData_PairCheck_type SPoint, tSQData_PairCheck_type *SRPoint, unsigned int *iSBchk)
{
	FILE *fp2chk = NULL;
	char chk_out[MAX_CHARS*LENGTH_OF_FILENAME+1];
	char shi_inp[MAX_CHARS*LENGTH_OF_FILENAME+1];
	char shi_out[MAX_CHARS*LENGTH_OF_FILENAME+1];

	unsigned int ii, iSBck=0;
	int isNaming=0;
	sqd2nd_type sq2[2];  /* 0:SS, 1:SB, 2:MaxNt, 3:MaxNe, 4:g_Max, 5:g_min, 6:SP */
	int  ierr = -1;

	/* line inputted from file */
	char QualData[QUAL_DATA_MAX_SIZE]; 
	int result;
	unsigned long long RecordCnt=0ULL;
	short iItemCurOK = 0; /* 1: OK, 0: NG */
	short iSave = 0;

	unsigned int iSScount = 0U;
	unsigned int iSBcount = 0U;
	unsigned int iSPcount = 0U;
	unsigned int iSFcount = 0U;
	
	unsigned int sTimeIgnoredCnt = 0;

	unsigned int iNtMaxcount = 0U;
	unsigned int iNtmincount = 0U;
	
	unsigned long long iNGcount = 0ULL;
	unsigned long long iOKcount = 0ULL;

	unsigned int iSxnumbering = 0;
	int		isSSarea = 0;
	int		isSBarea = 0;
	int		isSParea = 0;
	int 	isSFarea = 0;

	long avg_t1=0L, avg_t2 = 0L;
	long dif_t1=0L, dif_t2 = 0L;
	long dif_t3=0L;

	short   isSBok = 0;
	short   isSPok = 0;
	
	double MaxAcc = 0.0f;
	double minAcc = GMAX_RVALUE;
	unsigned int iCurrTime = 0;
	unsigned int ignoredCount = 0;
	unsigned int ignored2ndCnt = 0;

	// =========================================================
	// =========================================================
	memset(shi_inp, 0x00, sizeof(shi_inp));  
	memset(chk_out, 0x00, sizeof(chk_out));  
	memset(shi_out, 0x00, sizeof(shi_out));  


	strcpy(shi_inp, shift_file);
	strcpy(chk_out, shift_file);
	strcpy(shi_out, shift_file);
	
	isNaming = 0;
	for(ii=strlen(chk_out)-1; ii>0; ii--)
	{
		if( shi_inp[ii]=='.' ) 
		{
			shi_inp[ii+1] = '\0';
			chk_out[ii+1] = '\0';
			shi_out[ii+1] = '\0';

			strcat(shi_inp, arrPATs_ModeID[aiPATs05].ModeNm);
	
			strcat(chk_out, "c" );
			strcat(chk_out, arrPATs_ModeID[aiPATs05].ModeNm);

			strcat(shi_out, "b" ); /* Final SB decision file */
			strcat(shi_out, arrPATs_ModeID[aiPATs05].ModeNm);
			isNaming = 1;
			break;			
		}
	}
	
	if( 0==isNaming )
	{
		strcat(shi_inp, "."); 
		strcat(shi_inp, arrPATs_ModeID[aiPATs05].ModeNm);
	
		strcat(chk_out, ".c"); 
		strcat(chk_out, arrPATs_ModeID[aiPATs05].ModeNm);

		strcat(shi_out, ".b" ); /* Final SB decision file */
		strcat(shi_out, arrPATs_ModeID[aiPATs05].ModeNm);
	}
	// =========================================================
	// =========================================================

	if( (SPoint.SSnum==SPoint.SBnum) && (SPoint.SBnum==SPoint.SPnum) && (SPoint.SSnum==SPoint.SPnum) )
	{
		fprintf(stderr,">>Counter %s ...... Origin > SS(%d), SB(%d), SP(%d), SF(%d) - Same OK!!! \n", arrPATs_ModeID[aiPATs05].ModeID, SPoint.SSnum, SPoint.SBnum, SPoint.SPnum, SPoint.SFnum );
	}
	else if( (SPoint.SStot>SPoint.SBtot) || (SPoint.SStot>SPoint.SPtot) )
	//else if( (SPoint.SStot>SPoint.SBtot && SPoint.SBtot==SPoint.SPtot && SPoint.SStot>SPoint.SPtot) )
	{
		fprintf(stderr,">>Counter %s ...... Adjust > SS(%d), SB(%d), SP(%d), SF(%d) - SS More!!! \n", arrPATs_ModeID[aiPATs05].ModeID, SPoint.SStot, SPoint.SBtot, SPoint.SPtot, SPoint.SFtot );

		for(ii=0; ii<SPoint.SStot; ii++)
		{
			gMaxTbl[ii].ignored = 0; /* saved to File !! */

			if( 0L==gMaxTbl[ii].SBTime ) 
			{
				gMaxTbl[ii].ignored = IGN_BECAUSE_NOPAIR;

				fprintf(stderr," %5d (SS time: %12.4lf) : SB index NONE value \n", ii, gMaxTbl[ii].SSTime/1000.0/JERK_TIME_SCALE );
			}

			if( 0L==gMaxTbl[ii].SPTime ) 
			{
				gMaxTbl[ii].ignored = IGN_BECAUSE_NOPAIR;

				fprintf(stderr," %5d (SS time: %12.4lf) : SP index NONE value \n", ii, gMaxTbl[ii].SSTime/1000.0/JERK_TIME_SCALE );
			}

			if( 0L==gMaxTbl[ii].SFTime ) 
			{
				gMaxTbl[ii].ignored = IGN_BECAUSE_NOPAIR;

				fprintf(stderr," %5d (SS time: %12.4lf) : SF index NONE value \n", ii, gMaxTbl[ii].SSTime/1000.0/JERK_TIME_SCALE );
			}


			if( (0L==gMaxTbl[ii].SBTime) || (0L==gMaxTbl[ii].SPTime) || (0L==gMaxTbl[ii].SFTime) ) 
			{
				ignoredCount ++;
			}
			
		}

	}

	//else if( (SPoint.SStot>SPoint.SBtot || SPoint.SStot>SPoint.SPtot) )
	//{
	//}
	
#if DEBUG_MSG_1ST_POINT_TIME
	else
	{
		fprintf(stderr,"  SS,SB,SP,SF num/tot >> %d,%d,%d,%d = %d %d %d %d \n\n", 
			SPoint.SSnum, SPoint.SBnum, SPoint.SPnum, SPoint.SFnum, SPoint.SStot, SPoint.SBtot, SPoint.SPtot, SPoint.SFtot );
	}
#endif

	if(	ignoredCount )
		fprintf(stderr,"----------------------------------------------------------------------------------\n" );

	if(fp2chk) { fclose(fp2chk); fp2chk=NULL; }
	if(inpfile) { fclose(inpfile); inpfile=NULL; }


	// ========================================================================================
	// ========================================================================================
	// ========================================================================================
	// Real Average Time (SS ~ SB)
	// Real Average Time (SB ~ SP)
	// read file OK
	if(iSBck)
	{
		if( NULL == (inpfile = fopen( chk_out, "rb")) ) /* *.cECO */
		{
			// FAIL
			fprintf(stderr,"\r\n++ERROR++[%s]:Can not read file (%s) \n\n", __FUNCTION__, chk_out );
			AllFilesClosed();
			exit(0);
		}
	}
	else
	{
		if( NULL == (inpfile = fopen( shi_inp, "rb")) ) 
		{
			// FAIL
			fprintf(stderr,"\r\n++ERROR++[%s]:Can not read file (%s) \n\n", __FUNCTION__, shi_inp );
			AllFilesClosed();
			exit(0);
		}
	}
	
	// SB decision write file OK
	if( NULL == (fp2chk = fopen( shi_out, "wb")) )	/* *.bECO*/
	{
		// FAIL
		fprintf(stderr,"\r\n++ERROR++[%s]:Can not SB decision write file (%s) \n\n", __FUNCTION__, shi_out );
		AllFilesClosed();
		exit(0);
	}


#if SAVEMODE
	if(fp2chk)
		fprintf(fp2chk, TITLE_SHI1 "\n");
#endif


	fprintf(stderr,">>Checking an invalid time... \n");
	fprintf(stderr,"  Invalid if SS~SP time exceeds (%ld)msec, so ignored it. \n", SS2SP_OVER_TIME );
	fprintf(stderr,"  Invalid if SP~SF time exceeds (%ld)msec, so ignored it. \n", SP2SF_OVER_TIME );

	{
		long avg_t1=0L, avg_t2 = 0L, avg_t3 = 0L;
		double Gsum = 0.0f; /* sum of SS~SB, or SB~SP, or SP~SF */
		
		iSScount = 0U;
		iSBcount = 0U;
		iSPcount = 0U;
		iSFcount = 0U;

		iNtMaxcount = 0U;
		iNtmincount = 0U;

		iNGcount   = 0ULL;
		iOKcount   = 0ULL;
		sTimeIgnoredCnt = 0;

		iSxnumbering = 0;

		isSSarea = 0;
		isSBarea = 0;
		isSParea = 0;
		isSFarea = 0;


	#if 0
		/* Nt Max decision initialized */
		memset(NtMaxdecision, 0x00, NtMax_DECISION_NUM*sizeof(tNtMaxdec_type) );
		iNtMaxdecCnt  = 0;
		iNtMaxstart   = 0;
		iNtMaxTimeSum = 0;
		iNtMaxpntFix  = 0;
		
		/* Nt min decision initialized */
		memset(Ntmindecision, 0x00, Ntmin_DECISION_NUM*sizeof(tNtmindec_type) );
		iNtmindecCnt  = 0;
		iNtminstart   = 0;
		iNtminTimeSum = 0;
		iNtminpntFix  = 0;
	#endif


		do
		{
			unsigned int i=0;

			/* Read a line from input file. */
			memset( QualData, 0x00, QUAL_DATA_MAX_SIZE*sizeof(char) );

			if( NULL == fgets( QualData, QUAL_DATA_MAX_SIZE, inpfile ) )
			{
				ierr = ferror(inpfile);
				//fprintf(stderr,">>Duplica~~~~~~~~te record check completed %s -- [%s] \r\n", arrPATs_ModeID[aiPATs05].ModeID, shi_inp );			
				//fclose(inpfile);
				break;
			}

			RecordCnt++;


			/* Remove carriage return/line feed at the end of line. */
			i = strlen(QualData);
			
			if(i >= QUAL_DATA_MAX_SIZE)
			{
				fprintf(stderr,"ERROR:%6lld: Not enough Buffer length (%d) \r\n", RecordCnt, i );
			}

			if (--i > 0)
			{
				if (QualData[i] == '\n') QualData[i] = '\0';
				if (QualData[i] == '\r') QualData[i] = '\0'; 

				result = sscanf(QualData, RD_2ndFMT,
						&sq2[0].Time01,     &sq2[0].iPATs05,    &sq2[0].iPATs05S,   &sq2[0].VSP03,      &sq2[0].tqi07,    &sq2[0].curGear08, 
						&sq2[0].APS09,	    &sq2[0].No10,       &sq2[0].tgtGear11,  &sq2[0].ShiNew12,   &sq2[0].ShiTy12,  &sq2[0].arrGear, 
						&sq2[0].TqFr13,     &sq2[0].ShiPh14,    &sq2[0].Ne15,       &sq2[0].Nt16,       &sq2[0].LAcc17,   &sq2[0].LPFAcc, 
						&sq2[0].sTimePos, 
						&sq2[0].sTimeNtPos, &sq2[0].gearRat,    &sq2[0].gearShiOne, &sq2[0].gearShiTwo, &sq2[0].fJerk0,	
						&sq2[0].MaxAcc,     &sq2[0].minAcc );
				

				/* === 1 STEP : record (17 items check) =============== */
				
				iSave = 0;
				iItemCurOK = 0; /* NG - FAIL */
				if(QUAL_2ND_DATA_ITEM_NUM==result)
				{
					iItemCurOK = 1; /* OK */
					iOKcount ++;
				}
				else if( (QUAL_2ND_DATA_ITEM_NUM!=result) && (result!=-1 && i>0) ) 
				{
					iNGcount ++;
					continue; /* reading Next item because of FAIL item */
				}
				/* === 1 STEP : record (17 items check) =============== */


			

				if( 0==strcmp( sq2[0].sTimePos, TXT_SSTIME) ) 
				{ 
					gMaxTbl[iSScount].SSTime = (unsigned int)( (sq2[0].Time01)*1000*JERK_TIME_SCALE );
					gMaxTbl[iSScount].SFTime = 0L;

				#if DEBUG_MSG_OVER_TIME
					fprintf(stderr,"\nBBB(%3d)-> SS:%9ld  ", iSScount, gMaxTbl[iSScount].SSTime );
				#endif
		
					gMaxTbl[iSScount].Gavg0 = 0.0;
					gMaxTbl[iSScount].Gavg1 = 0.0;
					gMaxTbl[iSScount].Gavg2 = 0.0;

					iSScount++;

					isSSarea = 1;
					isSBarea = 0;
					isSParea = 0;
					isSFarea = 0;

					iSxnumbering = 0;
					Gsum = 0.0f;

			#if 0
					iOnce_NtMax = 0; /* Power On & Upshift -> NtMax */
					iOnce_Ntmin = 0; /* Power On & Upshift -> Ntmin */

					/* Nt Max decision initialized */
					memset(NtMaxdecision, 0x00, NtMax_DECISION_NUM*sizeof(tNtMaxdec_type) );
					iNtMaxdecCnt  = 0;
					iNtMaxstart   = 0;
					iNtMaxTimeSum = 0;
					iNtMaxpntFix  = 0;
					
					/* Nt min decision initialized */
					memset(Ntmindecision, 0x00, Ntmin_DECISION_NUM*sizeof(tNtmindec_type) );
					iNtmindecCnt  = 0;
					iNtminstart   = 0;
					iNtminTimeSum = 0;
					iNtminpntFix  = 0;
			#endif
			
				}


			#if 0
				/* -------------------------------------------------------- */
				/* Adjust Case -> ignored --------------------------------- */
				/* NOT Pair, that is  SS > SB, SS > SP,... SS more ...  --- */
				/* -------------------------------------------------------- */
				if(iSScount>0)
				{
					if( gMaxTbl[iSScount-1].ignored ) /* IGN_BECAUSE_NOPAIR */
					{
						//fprintf(stderr," 1st +++++++++++ deleted \n");
						continue; /* continue -> NOT saved!!! */
					}
				}
				/* -------------------------------------------------------- */
				/* Adjust Case -> ignored --------------------------------- */
				/* NOT Pair, that is  SS > SB, SS > SP,... SS more ...  --- */
				/* -------------------------------------------------------- */
			#endif
				

				if( 0==strcmp( sq2[0].sTimePos, TXT_SFTIME) ) 
				{ 
					if( 0L==gMaxTbl[iSScount-1].SFTime )
					{
						gMaxTbl[iSScount-1].SFTime = (unsigned int)( (sq2[0].Time01)*1000*JERK_TIME_SCALE );

					#if DEBUG_MSG_OVER_TIME
						fprintf(stderr,"bbb(%3d)-> SF:%9ld : SF-SP:%9.4lf, SP-SS:%9.4lf ", iSScount-1, gMaxTbl[iSScount-1].SFTime, 
								(double)(gMaxTbl[iSScount-1].SFTime - gMaxTbl[iSScount-1].SPTime)/1000/JERK_TIME_SCALE,
								(double)(gMaxTbl[iSScount-1].SPTime - gMaxTbl[iSScount-1].SSTime)/1000/JERK_TIME_SCALE );
					#endif


						if(iSxnumbering)
						{
							gMaxTbl[iSScount-1].Gavg2 = (double)(Gsum/iSxnumbering); /* SP~SF : G-average2 */
						}
						else
						{
							fprintf(stderr,"++ERROR++ iSxnumbering = %d \n", iSxnumbering );
						}
						//fprintf(stderr, " SF Time = %3d  -> %12.4lf  \n", iSScount-1, (double)( gMaxTbl[iSScount-1].SFTime/1000.0/JERK_TIME_SCALE ) );
						iSFcount++; /* NEVER NOT used!!! just counter */

						iSxnumbering = 0;
						Gsum = 0.0f;
					}
					else
					{
						strcpy( sq2[0].sTimePos, TXT_UNKNOWN ); /* change : SF -> (***) Unknown */
				#if DEBUG_MSG_OVER_TIME
						fprintf(stderr,"bbb(%3d)-> SF:%9ld  +++Unknown+++ \n", iSScount-1, gMaxTbl[iSScount-1].SFTime );
				#endif
					}					
				}



				/* ======================================================== */
				/* ======================================================== */
				/* ======================================================== */

				if( 0==SBposDecision ) /* VERSY IMPORTANT!!! first Position decision ++++++++ */
				{
					/* -------------------------------------------------------------- */
					/* -- first or LAST SB point ------------------------------------ */
					/* -------------------------------------------------------------- */					
					if( 0==strcmp( sq2[0].sTimePos, TXT_SBSWING0) )
					{ 

						if( (unsigned int)(gMaxTbl[iSScount-1].SBTime/JERK_TIME_SCALE) == (unsigned int)((sq2[0].Time01)*1000) )
						{
							strcpy( sq2[0].sTimePos, TXT_SBTIME); /* SB point fix */
							gMaxTbl[iSScount-1].SBTime    = (unsigned int)( (sq2[0].Time01)*1000*JERK_TIME_SCALE );

						#if DEBUG_MSG_OVER_TIME
							fprintf(stderr,"bbb(%3d)-> SB:%9ld ", iSScount-1, gMaxTbl[iSScount-1].SBTime );
						#endif

							gMaxTbl[iSScount-1].gMx1Begin  = gMaxTbl[iSScount-1].SBTime - iJerkTimeLen*JERK_TIME_SCALE;	/* JERK_TIME_mSec */
							gMaxTbl[iSScount-1].gmn1End    = gMaxTbl[iSScount-1].SBTime;
							
							gMaxTbl[iSScount-1].NtMaxBegin = gMaxTbl[iSScount-1].SBTime - iNtTimeLen*JERK_TIME_SCALE; /* JERK_TIME_mSec */


							//fprintf(stderr, "gMx begin/end %3d = %12lld  %12lld  \n", iSBcount, gMaxTbl[iSBcount].gMx1Begin, gMaxTbl[iSBcount].gmn1End );

							gMaxTbl[iSScount-1].Gavg0 = (double)(Gsum/iSxnumbering); /* SS~SB : G-average0 */

							iSBcount++;
							iSxnumbering = 0;
							Gsum = 0.0f;
						}
						else //if( (gMaxTbl[iSBcount].SBTime)*1000*JERK_TIME_SCALE != (unsigned int)( (sq2[0].Time01)*1000*JERK_TIME_SCALE) )
						{
							strcpy( sq2[0].sTimePos, TXT_UPCASE); 
						}
					}

					else if( 0==strcmp( sq2[0].sTimePos, TXT_SBTIME) )
					{
						if( iSBdecision > 1 )
						{
							strcpy( sq2[0].sTimePos, TXT_UPCASE );  /* Last SB0(3) -> (*u*) */
						}
						else
						{
							// SB -> SB Keeping
							iSxnumbering = 0;
							Gsum = 0.0f;
						}
					}

					/* -------------------------------------------------------------- */
					/* first or LAST SP point --------------------------------------- */
					/* -------------------------------------------------------------- */					
					if( 0==strcmp( sq2[0].sTimePos, TXT_SPSWING0 ) )
					{ 
						if( (unsigned int)(gMaxTbl[iSScount-1].SPTime/JERK_TIME_SCALE)	== (unsigned int)((sq2[0].Time01)*1000) )
						{
							strcpy( sq2[0].sTimePos, TXT_SPTIME); /* SP point fix */
							gMaxTbl[iSScount-1].SPTime = (unsigned int)( (sq2[0].Time01)*1000*JERK_TIME_SCALE );
						
							gMaxTbl[iSScount-1].NtminEnd  = gMaxTbl[iSScount-1].SPTime + Ntmin_After_mSec*JERK_TIME_SCALE;	/* Nt-min end time */

						#if DEBUG_MSG_OVER_TIME
							fprintf(stderr,"bbb(%3d)-> SP:%9ld ", iSScount-1, gMaxTbl[iSScount-1].SPTime );
						#endif

							/* ---- SS~SP time : 2000msec case : ignored case ---- */
							if( (gMaxTbl[iSScount-1].SPTime - gMaxTbl[iSScount-1].SSTime) >  SS2SP_OVER_TIME*JERK_TIME_SCALE )
							{
								sTimeIgnoredCnt ++;
								gMaxTbl[iSScount-1].ignored = IGN_BECAUSE_OT_SS2SP; /* ignored */
								fprintf(stderr," %5d (SS time: %12.4lf) : SS~SP time over!! (%4.1lf sec) <- invalid time. \n", 
									iSPcount, (double)((gMaxTbl[iSScount-1].SSTime)/1000.0/JERK_TIME_SCALE), (double)((gMaxTbl[iSScount-1].SPTime - gMaxTbl[iSScount-1].SSTime)/1000.0/JERK_TIME_SCALE) );
							}
							else
							{
								gMaxTbl[iSScount-1].ignored = 0; /* saved to File !! */
							}

							gMaxTbl[iSScount-1].Gavg1 = (double)(Gsum/iSxnumbering); /* SB~SP : G-average1 */

							iSxnumbering = 0;
							Gsum = 0.0f;

							iSPcount++;
						}
						else //if( (gMaxTbl[iSPcount].SPTime)*1000*JERK_TIME_SCALE != (unsigned int)( (sq2[0].Time01)*1000*JERK_TIME_SCALE) )
						{
							strcpy( sq2[0].sTimePos, TXT_UPCASE); 
						}
					}
					
					else if( 0==strcmp( sq2[0].sTimePos, TXT_SPTIME) )
					{
						if( iSBdecision > 1 )
						{
							strcpy( sq2[0].sTimePos, TXT_UPCASE );	/* Last SP0(3) -> (*u*) */
						}
						else
						{
							// SP -> SP Keeping
							iSxnumbering = 0;
							Gsum = 0.0f;
						}
					}


					
					/* -------------------------------------------------------------- */
					/* Nt Max point ------------------------------------------------- */
					/* -------------------------------------------------------------- */					
					if( 0==strcmp( sq2[0].sTimeNtPos, TXT_NtMaxSWING0 ) )
					{ 
						if( (unsigned int)(gMaxTbl[iSScount-1].NtMaxTime/JERK_TIME_SCALE) == (unsigned int)((sq2[0].Time01)*1000) )
						{
							strcpy( sq2[0].sTimeNtPos, TXT_NtMaxTIME); /* NtMax point fix */
					
							iNtMaxcount++;
						}
						else //if( (gMaxTbl[iSPcount].SPTime)*1000*JERK_TIME_SCALE != (unsigned int)( (sq2[0].Time01)*1000*JERK_TIME_SCALE) )
						{
							strcpy( sq2[0].sTimeNtPos, TXT_UPCASE); 
						}
					}
					
					else if( 0==strcmp( sq2[0].sTimeNtPos, TXT_NtMaxTIME) )
					{
						if( iSBdecision > 1 )
						{
							strcpy( sq2[0].sTimeNtPos, TXT_UPCASE );	/* Last SP0(3) -> (*u*) */
						}
						else
						{
							// NtMax -> NtMax Keeping 
						}
					}

					
					/* -------------------------------------------------------------- */
					/* Nt min point ------------------------------------------------- */
					/* -------------------------------------------------------------- */					
					if( 0==strcmp( sq2[0].sTimeNtPos, TXT_NtminSWING0 ) )
					{ 
						if( (unsigned int)(gMaxTbl[iSScount-1].NtminTime/JERK_TIME_SCALE) == (unsigned int)((sq2[0].Time01)*1000) )
						{
							strcpy( sq2[0].sTimeNtPos, TXT_NtminTIME); /* Ntmin point fix */
					
							iNtmincount++;
						}
						else //if( (gMaxTbl[iSPcount].SPTime)*1000*JERK_TIME_SCALE != (unsigned int)( (sq2[0].Time01)*1000*JERK_TIME_SCALE) )
						{
							strcpy( sq2[0].sTimeNtPos, TXT_UPCASE); 
						}
					}
					
					else if( 0==strcmp( sq2[0].sTimeNtPos, TXT_NtminTIME) )
					{
						if( iSBdecision > 1 )
						{
							strcpy( sq2[0].sTimeNtPos, TXT_UPCASE );	/* Last SP0(3) -> (*u*) */
						}
						else
						{
							// Ntmin -> Ntmin Keeping 
						}
					}

				}
				else //if( SBposDecision>1 ) /* Last position */
				{
					/* -------------------------------------------------------------- */
					/* SB point ----------------------------------------------------- */
					/* -------------------------------------------------------------- */					
					if( 0==strcmp( sq2[0].sTimePos, TXT_SBSWING0) )
					{ 
						strcpy( sq2[0].sTimePos, TXT_UPCASE); 
					}
					else if( 0==strcmp( sq2[0].sTimePos, TXT_SBTIME) )
					{

						gMaxTbl[iSScount-1].Gavg0 = (double)(Gsum/iSxnumbering); /* SS~SB : G-average0 */

						iSBcount++;
						// SB -> SB Keeping

						iSxnumbering = 0;
						Gsum = 0.0f;
					}


					/* -------------------------------------------------------------- */
					/* SP point ----------------------------------------------------- */
					/* -------------------------------------------------------------- */					
					if( 0==strcmp( sq2[0].sTimePos, TXT_SPSWING0) )
					{ 
						strcpy( sq2[0].sTimePos, TXT_UPCASE); 
					}
					else if( 0==strcmp( sq2[0].sTimePos, TXT_SPTIME) )
					{

						gMaxTbl[iSScount-1].Gavg1 = (double)(Gsum/iSxnumbering); /* SB~SP : G-average1 */

						iSxnumbering = 0;
						Gsum = 0.0f;

						iSPcount++;
						// SP -> SP Keeping
					}

					/* -------------------------------------------------------------- */
					/* Nt Max point ----------------------------------------------------- */
					/* -------------------------------------------------------------- */					
					if( 0==strcmp( sq2[0].sTimeNtPos, TXT_NtMaxSWING0) )
					{ 
						strcpy( sq2[0].sTimeNtPos, TXT_UPCASE); 
					}
					else if( 0==strcmp( sq2[0].sTimeNtPos, TXT_NtMaxTIME) )
					{
						iNtMaxcount++;
						// NtMax -> NtMax Keeping					
					}

					/* -------------------------------------------------------------- */
					/* Nt min point ----------------------------------------------------- */
					/* -------------------------------------------------------------- */					
					if( 0==strcmp( sq2[0].sTimeNtPos, TXT_NtminSWING0) )
					{ 
						strcpy( sq2[0].sTimeNtPos, TXT_UPCASE); 
					}
					else if( 0==strcmp( sq2[0].sTimeNtPos, TXT_NtminTIME) )
					{
						iNtmincount++;
						// Ntmin -> Ntmin Keeping
					}

				}
				/* ======================================================== */
				/* ======================================================== */
				/* ======================================================== */


			#if 0
				if( 0==strncmp( sq2[0].sTimePos, TXT_SPTIME, 4) ) 
				{ 
					gMaxTbl[iSPcount].SPTime = (unsigned int)( (sq2[0].Time01)*1000*JERK_TIME_SCALE );

					if( (gMaxTbl[iSPcount].SPTime - gMaxTbl[iSPcount].SSTime) >  SS2SP_OVER_TIME*JERK_TIME_SCALE )
					{
						gMaxTbl[iSPcount].ignored = 1; /* ignored */
						fprintf(stderr," %5d (SS time: %12.4lf) : %4.1lf sec (SS~SP diff time) <- ignored. \n", 
							iSPcount, (double)((gMaxTbl[iSPcount].SSTime)/1000.0/JERK_TIME_SCALE), (double)((gMaxTbl[iSPcount].SPTime - gMaxTbl[iSPcount].SSTime)/1000.0/JERK_TIME_SCALE) );
					}
					else
					{
						gMaxTbl[iSPcount].ignored = 0; /* saved !! */
					}

					iSPcount++;
				}
			#endif


				/* ----------------------------------------- */
				/* All Area (SS, SB, SP) counts ------------ */
				/* ----------------------------------------- */
				if( 0==strcmp(sq2[0].sTimePos, TXT_SSTIME) )
				{
					isSSarea = 1;
					isSBarea = 0;
					isSParea = 0;
					isSFarea = 0;
				}

				/* SB position checking ~~ upto SP*/
				if( 0==strncmp(sq2[0].sTimePos, TXT_SBTIME, 4) )
				{
					isSBarea = 1;
				}

				/* SP position checking ~~ upto next new SS */
				if( 0==strncmp(sq2[0].sTimePos, TXT_SPTIME, 4) )
				{
					isSSarea = 0;
					isSParea = 1;
				}

				/* SF position checking ~~ upto next new SS */
				if( 0==strncmp(sq2[0].sTimePos, TXT_SFTIME, 4) )
				{
					isSSarea = 0;
					isSBarea = 0;
					isSParea = 0;
					isSFarea = 1;
				}


				/* ----------------------------------------- */
				/* All Area (SS, SB, SP) counts ------------ */
				/* ----------------------------------------- */

				/* gMax & gmin - SS point ~ SB point 사이 */
				iCurrTime = (unsigned int)( (sq2[0].Time01)*1000*JERK_TIME_SCALE );
				if( gMaxTbl[iSScount-1].gMx1Begin <= iCurrTime )
				{
			#ifndef LOW_PASS_FILTER_GACC /* 2023-02-09, Low Pass Filter for LAcc */
					if( MaxAcc <= sq2[0].LAcc17 ) MaxAcc = sq2[0].LAcc17; /* Find gMax */
					if( minAcc >= sq2[0].LAcc17 ) minAcc = sq2[0].LAcc17; /* Find g_mix */
			#else
					if( MaxAcc <= sq2[0].LPFAcc ) MaxAcc = sq2[0].LPFAcc; /* Find gMax with LPFiltered data */
					if( minAcc >= sq2[0].LPFAcc ) minAcc = sq2[0].LPFAcc; /* Find g_mix with LPFiltered data */
			#endif
				
				}
				else
				{
					/* upto gMax */
					MaxAcc = 0.0f;

					/* upto gmin */
					/* SB + 1 point */
					if( isSBarea && (0==isSParea) && (gmin_Before_SB_POINT > iSxnumbering) ) 
					{
				#ifndef LOW_PASS_FILTER_GACC /* 2023-02-09, Low Pass Filter for LAcc */
						if( minAcc >= sq2[0].LAcc17 ) minAcc = sq2[0].LAcc17; /* valid g_min point */
				#else
						if( minAcc >= sq2[0].LPFAcc ) minAcc = sq2[0].LPFAcc; /* valid g_min point, with LPFiltered data  */
				#endif
					}
					else
					{
						minAcc = GMAX_RVALUE;
					}

				}


				iSxnumbering ++;

			#ifndef LOW_PASS_FILTER_GACC /* 2023-02-09, Low Pass Filter for LAcc */
				Gsum += sq2[0].LAcc17;
			#else			
				Gsum += sq2[0].LPFAcc; /* with LPFiltered data  */
			#endif




			#if 1
				/* -------------------------------------------------------- */
				/* Adjust Case -> ignored --------------------------------- */
				/* NOT Pair, that is  SS > SB, SS > SP,... SS more ...	--- */
				/* -------------------------------------------------------- */
				if(iSScount>0)
				{
					if( gMaxTbl[iSScount-1].ignored ) /* IGN_BECAUSE_NOPAIR */
					{
						continue; /* continue -> NOT saved!!! */
					}
				}
				/* -------------------------------------------------------- */
				/* Adjust Case -> ignored --------------------------------- */
				/* NOT Pair, that is  SS > SB, SS > SP,... SS more ...	--- */
				/* -------------------------------------------------------- */
			#endif


			#if SAVEMODE
				if(fp2chk) // && is2File)
				{
					fprintf(fp2chk, SAVE_ChkFMT,
							sq2[0].Time01,     sq2[0].iPATs05,    sq2[0].iPATs05S,  sq2[0].VSP03,      sq2[0].tqi07,      sq2[0].curGear08, 
							sq2[0].APS09,      sq2[0].No10,	      sq2[0].tgtGear11, sq2[0].ShiNew12,   sq2[0].ShiTy12,    sq2[0].arrGear, 
							sq2[0].TqFr13,     sq2[0].ShiPh14,    sq2[0].Ne15,      sq2[0].Nt16,       sq2[0].LAcc17,     sq2[0].LPFAcc,
							sq2[0].sTimePos,   iSxnumbering,
							Gsum,              sq2[0].sTimeNtPos, sq2[0].gearRat,   sq2[0].gearShiOne, sq2[0].gearShiTwo, sq2[0].fJerk0,  
							MaxAcc,     minAcc /* sq2[0].minAcc */ );

					fprintf(fp2chk, "\n");
				}
			#endif

			}

		}
		while (!feof (inpfile));

		if(fp2chk) fclose(fp2chk);
		if(inpfile) fclose(inpfile);

		if( !sTimeIgnoredCnt )
		{
		#if DEBUG_MSG_OVER_TIME
			fprintf(stderr,"\n");
		#endif

			fprintf(stderr,"  SS~SP Time over is NONE!!! \n");
		}

	#if DEBUG_MSG_OVER_TIME
		fprintf(stderr,"\n");
	#endif
		fprintf(stderr,"----------------------------------------------------------------------------------\n" );
		

		avg_t1 = 0L;
		avg_t2 = 0L;
		avg_t3 = 0L;
		ignoredCount = 0;
		ignored2ndCnt = 0;

		//fprintf(stderr, "%d %d %d \n", iSScount, iSBcount, iSPcount );
		for(ii=0; ii<iSScount; ii++)
		{

			if( gMaxTbl[ii].ignored )
			{
				ignored2ndCnt ++; 
			}
			else
			{
				dif_t1 = (gMaxTbl[ii].SBTime - gMaxTbl[ii].SSTime)/JERK_TIME_SCALE ;
				dif_t2 = (gMaxTbl[ii].SPTime - gMaxTbl[ii].SBTime)/JERK_TIME_SCALE ;
				
				dif_t3 = (gMaxTbl[ii].SFTime - gMaxTbl[ii].SPTime)/JERK_TIME_SCALE ;
			}			

			if( dif_t1 < 0 ) dif_t1 = 0L;
			if( dif_t2 < 0 ) dif_t2 = 0L;
			if( dif_t3 < 0 ) dif_t3 = 0L;

		#if 0
			/* Ignored Case checking -- SS~SP durtion */
			if( (dif_t1 + dif_t2) > SS2SP_OVER_TIME )
			{
				ignoredCount ++;

				gMaxTbl[ii].ignored = IGN_BECAUSE_OT_SS2SP;

				fprintf(stderr," %5d (SS time: %12.4lf) : SS~SP time over!! (%4.1lf sec) <- ignored... \n", 
					ii, (double)((gMaxTbl[ii].SSTime)/1000.0/JERK_TIME_SCALE), (double)((gMaxTbl[ii].SPTime - gMaxTbl[ii].SSTime)/1000.0/JERK_TIME_SCALE) );
			}
		#endif


			
			/* Ignored Case checking -- SP~SF durtion */
			if( (dif_t3 > SP2SF_OVER_TIME) )
			{
				ignoredCount ++;

				gMaxTbl[ii].ignored = IGN_BECAUSE_OT_SP2SF;

				fprintf(stderr," %5d (SS time: %12.4lf) : SP~SF time over!! (%4.1lf sec) <- invalid time.. \n", 
					ii, (double)((gMaxTbl[ii].SSTime)/1000.0/JERK_TIME_SCALE), (double)((gMaxTbl[ii].SFTime - gMaxTbl[ii].SPTime)/1000.0/JERK_TIME_SCALE) );
			}


			//fprintf(stderr, " ii=%3d -> Gavg0 = %10.4lf   Gavg1 = %10.4lf   Gavg2 = %10.4lf  \n", ii, gMaxTbl[ii].Gavg0, gMaxTbl[ii].Gavg1, gMaxTbl[ii].Gavg2 );
			
			avg_t1 += dif_t1;
			avg_t2 += dif_t2;
			avg_t3 += dif_t3;

			//fprintf(stderr, " %2d -> %10lld  %10lld  \n", ii, avg_t1, avg_t2 );
	 	}


		if(ignoredCount)
		{
	#if DEBUG_MSG_OVER_TIME
			fprintf(stderr,"\n");
	#endif
			fprintf(stderr,"----------------------------------------------------------------------------------\n" );
		}


		if(iSScount-ignored2ndCnt)
		{
			avg_t1 /= (iSScount-ignored2ndCnt);
			avg_t2 /= (iSScount-ignored2ndCnt);
			avg_t3 /= (iSScount-ignored2ndCnt);
		}
		fprintf(stderr,">>2nd Average Time Measurement --- SS(%d), SB(%d), SP(%d), SF(%d) Records.. %s \n", 
			iSScount-ignored2ndCnt, iSBcount-ignored2ndCnt, iSPcount-ignored2ndCnt, iSFcount-ignored2ndCnt, arrPATs_ModeID[aiPATs05].ModeID );
		fprintf(stderr,"  Shift Average Time(t1:SS~SB) : %9ld msec <- 2nd average time  \n", avg_t1 );
		fprintf(stderr,"  Shift Average Time(t2:SB~SP) : %9ld msec <- 2nd average time  \n", avg_t2 );
		fprintf(stderr,"  Shift Average Time(t3:SP~SF) : %9ld msec <- 2nd average time  \n", avg_t3 );
		fprintf(stderr,"----------------------------------------------------------------------------------\n" );

	}



	SRPoint->SStot = iSScount-ignored2ndCnt;
	SRPoint->SBtot = iSBcount-ignored2ndCnt;
	SRPoint->SPtot = iSPcount-ignored2ndCnt;
	SRPoint->SFtot = iSFcount-ignored2ndCnt;

	SRPoint->SSnum = iSScount-ignored2ndCnt;
	SRPoint->SBnum = iSBcount-ignored2ndCnt;
	SRPoint->SPnum = iSPcount-ignored2ndCnt;
	SRPoint->SFnum = iSFcount-ignored2ndCnt;

	*iSBchk = ignoredCount; /* Because of Last action : delete files, TRUE or FALSE */

	return ignoredCount;
}




int ignored_QSData(short aiPATs05, short SBposDecision, tSQData_PairCheck_type *SPoint)
{
	FILE *fp2inp = NULL;
	FILE *fp2out = NULL;
	char shi_inp[MAX_CHARS*LENGTH_OF_FILENAME+1];
	char shi_out[MAX_CHARS*LENGTH_OF_FILENAME+1];

	int ignoredRecord = 0;
	unsigned int ii, iSBck=0;
	int isNaming=0;
	sqd2nd_type sq2[2];  /* 0:SS, 1:SB, 2:MaxNt, 3:MaxNe, 4:g_Max, 5:g_min, 6:SP */
	int  ierr = -1;

	/* line inputted from file */
	char QualData[QUAL_DATA_MAX_SIZE]; 
	int result;
	unsigned long long RecordCnt=0ULL;
	short iItemCurOK = 0; /* 1: OK, 0: NG */
	short iSave = 0;

	unsigned int iSScount = 0U;
	unsigned int iSBcount = 0U;
	unsigned int iSPcount = 0U;
	unsigned int iSFcount = 0U;
	
	short is2File = 1;
	unsigned int ignoredCount = 0;
	unsigned long long iNGcount = 0ULL;
	unsigned long long iOKcount = 0ULL;

	int		isSSarea = 0;
	int		isSBarea = 0;
	int		isSParea = 0;
	int 	isSFarea = 0;

	long avg_t1=0L, avg_t2 = 0L;
	long dif_t1=0L, dif_t2 = 0L;
	long dif_t3=0L;

	
	// =========================================================
	// =========================================================
	memset(shi_inp, 0x00, sizeof(shi_inp));  
	memset(shi_out, 0x00, sizeof(shi_out));  


	strcpy(shi_inp, shift_file);
	strcpy(shi_out, shift_file);
	
	isNaming = 0;
	for(ii=strlen(shi_out)-1; ii>0; ii--)
	{
		if( shi_inp[ii]=='.' ) 
		{
			shi_inp[ii+1] = '\0';
			shi_out[ii+1] = '\0';
	
			strcat(shi_inp, "b" );
			strcat(shi_inp, arrPATs_ModeID[aiPATs05].ModeNm);

			strcat(shi_out, "i" ); /* Final SB decision file */
			strcat(shi_out, arrPATs_ModeID[aiPATs05].ModeNm);
			isNaming = 1;
			break;			
		}
	}
	
	if( 0==isNaming )
	{
		strcat(shi_inp, ".b"); 
		strcat(shi_inp, arrPATs_ModeID[aiPATs05].ModeNm);

		strcat(shi_out, ".i" ); /* Final SB decision file */
		strcat(shi_out, arrPATs_ModeID[aiPATs05].ModeNm);
	}
	// =========================================================
	// =========================================================

	// ========================================================================================
	// ========================================================================================

	if( NULL == (fp2inp = fopen( shi_inp, "rb")) ) /* *.bECO*/
	{
		// FAIL
		fprintf(stderr,"\r\n++ERROR++[%s]:Can not read file (%s) \n\n", __FUNCTION__, shi_inp );
		AllFilesClosed();
		exit(0);
	}
	
	// SB decision write file OK
	if( NULL == (fp2out = fopen( shi_out, "wb")) )	/* *.iECO*/
	{
		// FAIL
		fprintf(stderr,"\r\n++ERROR++[%s]:Can not SB decision write file (%s) \n\n", __FUNCTION__, shi_out );
		AllFilesClosed();
		exit(0);
	}


#if SAVEMODE
	if(fp2out)
		fprintf(fp2out, TITLE_SHI1 "\n");
#endif


	//fprintf(stderr,">>Checking if SS~SP time (%dmsec) is exceeded, and then ignored... \n", SS2SP_OVER_TIME );

	{
		long avg_t1=0L, avg_t2 = 0L, avg_t3 = 0L;

		iSScount = 0U;
		iSBcount = 0U;
		iSPcount = 0U;
		iSFcount = 0U;

		RecordCnt  = 0ULL;
		iNGcount   = 0ULL;
		iOKcount   = 0ULL;


		isSSarea = 0;
		isSBarea = 0;
		isSParea = 0;
		isSFarea = 0;


		is2File = 1; /* Record Saved!! */

		do
		{
			unsigned int i=0;

			/* Read a line from input file. */
			memset( QualData, 0x00, sizeof(QualData) );

			if( NULL == fgets( QualData, QUAL_DATA_MAX_SIZE, inpfile ) )
			{
				ierr = ferror(inpfile);
				//fprintf(stderr,">>Duplica~~~~~~~~te record check completed %s -- [%s] \r\n", arrPATs_ModeID[aiPATs05].ModeID, shi_inp );			
				//fclose(inpfile);
				break;
			}

			RecordCnt++;


			/* Remove carriage return/line feed at the end of line. */
			i = strlen(QualData);
			
			if(i >= QUAL_DATA_MAX_SIZE)
			{
				fprintf(stderr,"ERROR:%6lld: Not enough Buffer length (%d) \r\n", RecordCnt, i );
			}

			if (--i > 0)
			{
				if (QualData[i] == '\n') QualData[i] = '\0';
				if (QualData[i] == '\r') QualData[i] = '\0'; 

				result = sscanf(QualData, RD_23YNW,
						&sq2[0].Time01,     &sq2[0].iPATs05,    &sq2[0].iPATs05S,   &sq2[0].VSP03,      &sq2[0].tqi07,      &sq2[0].curGear08, 
						&sq2[0].APS09,	    &sq2[0].No10,       &sq2[0].tgtGear11,  &sq2[0].ShiNew12,   &sq2[0].ShiTy12,    &sq2[0].arrGear, 
						&sq2[0].TqFr13,     &sq2[0].ShiPh14,    &sq2[0].Ne15,       &sq2[0].Nt16,       &sq2[0].LAcc17,     &sq2[0].LPFAcc,
						&sq2[0].sTimePos, 
						&sq2[0].sPosNum,    &sq2[0].Gsum,       &sq2[0].sTimeNtPos, &sq2[0].gearRat,    &sq2[0].gearShiOne, &sq2[0].gearShiTwo, 
						&sq2[0].fJerk0,	    &sq2[0].MaxAcc,     &sq2[0].minAcc );
				

				/* === 1 STEP : record (17 items check) =============== */
				
				iSave = 0;
				iItemCurOK = 0; /* NG - FAIL */
				if(QUAL_2ND_NEWDT_ITEM_NUM==result)
				{
					iItemCurOK = 1; /* OK */
					iOKcount ++;
				}
				else if( (QUAL_2ND_NEWDT_ITEM_NUM!=result) && (result!=-1 && i>0) ) 
				{
					iNGcount ++;
					continue; /* reading Next item because of FAIL item */
				}
				/* === 1 STEP : record (17 items check) =============== */




				/* ----------------------------------------- */
				/* All Area (SS, SB, SP) counts ------------ */
				/* ----------------------------------------- */
				if( 0==strcmp(sq2[0].sTimePos, TXT_SSTIME) )
				{
					if( gMaxTbl[iSScount].ignored ) 
					{
						is2File = 0; /* NOT saved!! */
					}
					else
					{
						is2File = 1; /* Record Saved!! */
					}

					isSSarea = 1;
					isSBarea = 0;
					isSParea = 0;
					isSFarea = 0;

					iSScount ++;

				}

				/* SB position checking ~~ upto SP*/
				if( 0==strncmp(sq2[0].sTimePos, TXT_SBTIME, 4) )
				{
					isSSarea = 0;
					isSBarea = 1;

					iSBcount ++;
				}

				/* SP position checking ~~ upto next new SS */
				if( 0==strncmp(sq2[0].sTimePos, TXT_SPTIME, 4) )
				{
					isSSarea = 0;
					isSBarea = 0;
					isSParea = 1;

					iSPcount ++;
				}

				/* SF position checking ~~ upto next new SS */
				if( 0==strncmp(sq2[0].sTimePos, TXT_SFTIME, 4) )
				{
					isSFarea = 1;

					iSFcount ++;
				}


				/* -------------------------------------------- */
				/* ignored checked  --------------------------- */
				/* -------------------------------------------- */
				if(iSScount)
				{
					if( gMaxTbl[iSScount-1].ignored ) 
					{
						is2File=0;

						if( 0==strcmp(sq2[0].sTimePos, TXT_SSTIME) ) ignoredRecord ++;
					}
				}


			#if SAVEMODE
				if(fp2out && is2File)
				{
					fprintf(fp2out, SAVE_ChkFMT,
							sq2[0].Time01,     sq2[0].iPATs05,    sq2[0].iPATs05S,   sq2[0].VSP03,       sq2[0].tqi07,      sq2[0].curGear08, 
							sq2[0].APS09,      sq2[0].No10,	      sq2[0].tgtGear11,  sq2[0].ShiNew12,	 sq2[0].ShiTy12,    sq2[0].arrGear, 
							sq2[0].TqFr13,     sq2[0].ShiPh14,    sq2[0].Ne15,       sq2[0].Nt16,	     sq2[0].LAcc17,     sq2[0].LPFAcc,
							sq2[0].sTimePos, 
							sq2[0].sPosNum,    sq2[0].Gsum,       sq2[0].sTimeNtPos, sq2[0].gearRat,     sq2[0].gearShiOne,  sq2[0].gearShiTwo, 
							sq2[0].fJerk0,     sq2[0].MaxAcc,     sq2[0].minAcc );

					fprintf(fp2out, "\n");
				}
			#endif

			}

		}
		while (!feof (inpfile));

		if(fp2out) fclose(fp2out);
		if(fp2inp) fclose(fp2inp);

		//fprintf(stderr,"----------------------------------------------------------------------------------\n" );


		avg_t1 = 0L;
		avg_t2 = 0L;
		avg_t3 = 0L;
		ignoredCount = 0;
		//fprintf(stderr, "%d %d %d \n", iSScount, iSBcount, iSPcount );
		for(ii=0; ii<iSScount; ii++)
		{
			dif_t1 = (gMaxTbl[ii].SBTime - gMaxTbl[ii].SSTime)/JERK_TIME_SCALE ;
			dif_t2 = (gMaxTbl[ii].SPTime - gMaxTbl[ii].SBTime)/JERK_TIME_SCALE ;
			
			dif_t3 = (gMaxTbl[ii].SFTime - gMaxTbl[ii].SPTime)/JERK_TIME_SCALE ;


			/* Ignored Case checking -- SS~SP durtion */
			if( (dif_t1 + dif_t2) > SS2SP_OVER_TIME )
			{
				ignoredCount ++;
				//gMaxTbl[ii].ignored = 1;

				/* zero Clear because of over time */
				gMaxTbl[ii].SSTime = 0UL;
				gMaxTbl[ii].SBTime = 0UL;
				gMaxTbl[ii].SPTime = 0UL;
				gMaxTbl[ii].SFTime = 0UL;

				gMaxTbl[ii].Gavg0 = 0.0f;
				gMaxTbl[ii].Gavg1 = 0.0f;
				gMaxTbl[ii].Gavg2 = 0.0f;

				gMaxTbl[ii].gMx1Begin = 0UL;
				gMaxTbl[ii].gmn2Begin = 0UL;
				gMaxTbl[ii].gMx2End   = 0UL;
				gMaxTbl[ii].gmn1End   = 0UL;

				gMaxTbl[ii].NtMaxTime = 0UL;
				gMaxTbl[ii].NtminTime = 0UL;

			}

			/* Ignored Case checking -- SP~SF durtion */
			else if(  dif_t3 > SP2SF_OVER_TIME )
			{
				ignoredCount ++;
				//gMaxTbl[ii].ignored = 1;

				/* zero Clear because of over time */
				gMaxTbl[ii].SSTime = 0UL;
				gMaxTbl[ii].SBTime = 0UL;
				gMaxTbl[ii].SPTime = 0UL;
				gMaxTbl[ii].SFTime = 0UL;
				gMaxTbl[ii].SFTime = 0UL;

				gMaxTbl[ii].Gavg0 = 0.0f;
				gMaxTbl[ii].Gavg1 = 0.0f;
				gMaxTbl[ii].Gavg2 = 0.0f;
				
				gMaxTbl[ii].gMx1Begin = 0UL;
				gMaxTbl[ii].gmn2Begin = 0UL;
				gMaxTbl[ii].gMx2End   = 0UL;
				gMaxTbl[ii].gmn1End   = 0UL;
				
				gMaxTbl[ii].NtMaxTime = 0UL;
				gMaxTbl[ii].NtminTime = 0UL;

			}
			else
			{
				avg_t1 += dif_t1;
				avg_t2 += dif_t2;
				avg_t3 += dif_t3;
				//fprintf(stderr, " %2d -> %10lld  %10lld  \n", ii, avg_t1, avg_t2 );
			}

		}
		fprintf(stderr,"----------------------------------------------------------------------------------\n" );

		if(iSScount-ignoredRecord)
		{
			avg_t1 /= (iSScount-ignoredRecord);
			avg_t2 /= (iSScount-ignoredRecord);
			avg_t3 /= (iSScount-ignoredRecord);
		}
		
		fprintf(stderr,">>3rd Average Time Measurement --- SS(%d), SB(%d), SP(%d), SF(%d) Records.. %s \n", 
			iSScount-ignoredRecord, iSBcount-ignoredRecord, iSPcount-ignoredRecord, iSFcount-ignoredRecord, arrPATs_ModeID[aiPATs05].ModeID );
		fprintf(stderr,"  Shift Average Time(t1:SS~SB) : %9ld msec <- 3rd average time  ign(%d==%d) \n", avg_t1, ignoredRecord, ignoredCount );
		fprintf(stderr,"  Shift Average Time(t2:SB~SP) : %9ld msec <- 3rd average time  \n", avg_t2 );
		fprintf(stderr,"  Shift Average Time(t3:SP~SF) : %9ld msec <- 3rd average time  \n", avg_t3 );
		fprintf(stderr,"----------------------------------------------------------------------------------\n" );

	

	}

	SPoint->SStot = iSScount-ignoredRecord;
	SPoint->SBtot = iSBcount-ignoredRecord;
	SPoint->SPtot = iSPcount-ignoredRecord;
	SPoint->SFtot = iSFcount-ignoredRecord;

	SPoint->SSnum = iSScount-ignoredRecord;
	SPoint->SBnum = iSBcount-ignoredRecord;
	SPoint->SPnum = iSPcount-ignoredRecord;
	SPoint->SFnum = iSFcount-ignoredRecord;

	return 0;
}




int FindGminMaxShiftData(short aiPATs05, tSQData_PairCheck_type SPoint, short ignoredCnt)
{
	FILE *fp2out = NULL;

	unsigned int kkloop=0, KkLast;

	#define RCSZ 		2
	sqd2nd_type sq2[RCSZ];  /* 0:SS, 1:SB, 2:MaxNt, 3:MaxNe, 4:g_Max, 5:g_min, 6:SP */
	
	char shi_inp[MAX_CHARS*LENGTH_OF_FILENAME+1];
	char shi_out[MAX_CHARS*LENGTH_OF_FILENAME+1];
	char shigMax[MAX_CHARS*LENGTH_OF_FILENAME+1];
	char shigmin[MAX_CHARS*LENGTH_OF_FILENAME+1];

	unsigned int ii=0U;
	int  ierr = -1;
	short iItemCurOK = 0; /* 1: OK, 0: NG */
	short iSave = 0;
	
	//double gMaxSaveTime[MAX_TABLE_SIZ];

	/* line inputted from file */
	char QualData[QUAL_DATA_MAX_SIZE]; 
	int result;
	unsigned long long RecordCnt=0ULL;

	unsigned long long iNGcount = 0ULL;
	unsigned long long iOKcount = 0ULL;

	int isNaming = 0;

	double g_Max = 0.0f;
	double g_min = GMAX_RVALUE;

	unsigned int iSScount = 0U;
	unsigned int iSBcount = 0U;
	unsigned int iSPcount = 0U;
	unsigned int iSFcount = 0U;
	
	short isSSarea = 0;
	short isSBarea = 0;
	short isSParea = 0;
	short isSFarea = 0;
	short is2File = 0;

	short GminChecked = 1;
	short GMaxChecked = 1;

	int iloop = 0;
	unsigned int iCurrTime = 0;

	tminMax_type gMaxVals[MAX_TABLE_SIZ];
	tminMax_type gminVals[MAX_TABLE_SIZ];

	tminMax_type gMaxFinVals[MAX_TABLE_SIZ];
	tminMax_type gminFinVals[MAX_TABLE_SIZ];
	int iGMax, iGmin;
	char szTimeG[10];
	short isSFpoint = 0;
	
	/* ===================================================================================== */
	/* ===================================================================================== */

	//AllFilesClosed();
	if(inpfile) { fclose(inpfile);	inpfile=NULL;  }

	/* ===================================================================================== */
	memset(shi_inp, 0x00, sizeof(shi_inp)); 
	memset(shi_out, 0x00, sizeof(shi_out)); 
	memset(shigMax, 0x00, sizeof(shigMax)); 
	memset(shigmin, 0x00, sizeof(shigmin)); 

	memset(gMaxVals, 0x00, MAX_TABLE_SIZ*sizeof(tminMax_type));
	memset(gminVals, 0x00, MAX_TABLE_SIZ*sizeof(tminMax_type));

	memset(gMaxFinVals, 0x00, MAX_TABLE_SIZ*sizeof(tminMax_type));
	memset(gMaxFinVals, 0x00, MAX_TABLE_SIZ*sizeof(tminMax_type));

	memset(szTimeG, 0x00, 10*sizeof(char));

	strcpy(shi_inp, shift_file);
	strcpy(shi_out, shift_file);
	strcpy(shigMax, shi_inp);
	strcpy(shigmin, shi_inp);


	if( (aiPATs05>=0 && aiPATs05<MODE_ID_NUMS-1) && (strlen(shi_inp)>0) )
	{
		isNaming = 0;
		for(ii=strlen(shi_inp)-1; ii>0; ii--)
		{
			if( shi_inp[ii]=='.' ) 
			{
				shi_inp[ii+1] = '\0';
				shi_out[ii+1] = '\0';
				shigMax[ii+1] = '\0';
				shigmin[ii+1] = '\0';

				if(ignoredCnt)
				{
					strcat(shi_inp, "i");
					strcat(shi_inp, arrPATs_ModeID[aiPATs05].ModeNm);
				}
				else
				{
					strcat(shi_inp, "b");
					strcat(shi_inp, arrPATs_ModeID[aiPATs05].ModeNm);
				}

				strcat(shi_out, "g");
				strcat(shi_out, arrPATs_ModeID[aiPATs05].ModeNm);

				strcat(shigMax, FILE_EXT_gMAX );
				strcat(shigmin, FILE_EXT_gMin );

				isNaming = 1;
				break;
			}
		}

		if( 0==isNaming )
		{
			if(ignoredCnt)
			{
				strcat(shi_inp, ".i");
				strcat(shi_inp, arrPATs_ModeID[aiPATs05].ModeNm);
			}
			else
			{
				strcat(shi_inp, ".b");
				strcat(shi_inp, arrPATs_ModeID[aiPATs05].ModeNm);
			}

			strcat(shi_out, ".g");
			strcat(shi_out, arrPATs_ModeID[aiPATs05].ModeNm);

			strcat(shigMax, "."); 
			strcat(shigMax, FILE_EXT_gMAX );
			strcat(shigmin, "."); 
			strcat(shigmin, FILE_EXT_gMin );
		}
		

		// read file OK
		if( NULL == (inpfile = fopen( shi_inp, "rb")) ) 
		{
			// FAIL
			fprintf(stderr,"\r\n++ERROR++[%s]:Can not read file (%s) \n\n", __FUNCTION__, shi_inp );
			AllFilesClosed();
			exit(0);
		}

		// write file OK
		if( NULL == (fp2out = fopen( shi_out, "wb")) )	
		{
			// FAIL
			fprintf(stderr,"\r\nCan not create file (%s) for Gmin/GMax \n\n", shi_out );
			AllFilesClosed();
			if(inpfile) fclose(inpfile); 
			exit(0);
		}
		// ==================================================================

	}
	else
	{
		// FAIL
		fprintf(stderr,"\r\n++ERROR++: There is no action. Check plz... (%d) [%s] \n\n", aiPATs05, shi_inp );
		AllFilesClosed();
		//if(fp2gmin) fclose(fp2gmin); 
		//if(fp2gMax) fclose(fp2gMax);
		exit(0);
	}
	/* ===================================================================================== */


	/* ===================================================================================== */
	/* ===================================================================================== */
	/* ------------ min / Max Table Creation ----------------------------------------------- */
	/* ===================================================================================== */
	/* ===================================================================================== */

	memset(QualData, 0x00, QUAL_DATA_MAX_SIZE*sizeof(char) );
	memset(sq2, 0x00, sizeof(sq2) );


	RecordCnt  = 0ULL;

	iNGcount   = 0ULL;
	iOKcount   = 0ULL;



	// ======================================================================
	// Searching g Max
	// ======================================================================

	if( (SPoint.SSnum==SPoint.SBnum && SPoint.SBnum==SPoint.SPnum && SPoint.SSnum==SPoint.SPnum) )
	{
		KkLast = SPoint.SBnum;
	}
	else //if( (SPoint.SStot==SPoint.SBtot && SPoint.SBtot==SPoint.SPtot && SPoint.SStot==SPoint.SPtot) )
	{
		KkLast = SPoint.SBtot; // SBcnt;
	}

	// ======================================================================

	//if( (TYPE_MAX_G == minMaxType) || (TYPE_MAXLAST_G == minMaxType) )
	{
		fprintf(stderr,">>Searching G-Max and G-min %s... SS(%d), SB(%d), SP(%d) (%d)   \r", arrPATs_ModeID[aiPATs05].ModeID, SPoint.SSnum, SPoint.SBnum, SPoint.SPnum, SPoint.SBtot );

		//for(kkloop=1; kkloop<=KkLast; kkloop++)
		{
			//fprintf(stderr,">>Searching g_Max %s -- SS(%d)...  \r", arrPATs_ModeID[aiPATs05].ModeID, kkloop );

			iSScount = 0;
			iSBcount = 0;
			iSPcount = 0;

			isSSarea = 0;
			isSBarea = 0;
			isSParea = 0;
			isSFarea = 0;

			iNGcount   = 0ULL;
			iOKcount   = 0ULL;

			do
			{
				unsigned int i=0;

				/* Read a line from input file. */
				memset( QualData, 0x00, sizeof(QualData) );

				if( NULL == fgets( QualData, QUAL_DATA_MAX_SIZE, inpfile ) )
				{
					ierr = ferror(inpfile);
					//fclose(inpfile);
					//fprintf(stderr,">>Searched g_Max %s -- [%s] \r\n", arrPATs_ModeID[aiPATs05].ModeID, shi_inp ); 			
					break;
				}

				RecordCnt++;


				/* Remove carriage return/line feed at the end of line. */
				i = strlen(QualData);
				
				if(i >= QUAL_DATA_MAX_SIZE)
				{
					fprintf(stderr,"ERROR:%6lld: Not enough Buffer length gMax (%d) \r\n", RecordCnt, i );
				}

				if (--i > 0)
				{
					if (QualData[i] == '\n') QualData[i] = '\0';
					if (QualData[i] == '\r') QualData[i] = '\0'; 

					result = sscanf(QualData, RD_23YNW,
							&sq2[0].Time01,  &sq2[0].iPATs05,	 &sq2[0].iPATs05S,	 &sq2[0].VSP03,      &sq2[0].tqi07,	     &sq2[0].curGear08, 
							&sq2[0].APS09,	 &sq2[0].No10,		 &sq2[0].tgtGear11,  &sq2[0].ShiNew12,   &sq2[0].ShiTy12,    &sq2[0].arrGear, 
							&sq2[0].TqFr13,  &sq2[0].ShiPh14,	 &sq2[0].Ne15,		 &sq2[0].Nt16,	     &sq2[0].LAcc17,     &sq2[0].LPFAcc,
							&sq2[0].sTimePos, 
							&sq2[0].sPosNum, &sq2[0].Gsum,       &sq2[0].sTimeNtPos, &sq2[0].gearRat,    &sq2[0].gearShiOne, &sq2[0].gearShiTwo, 
							&sq2[0].fJerk0,  &sq2[0].MaxAcc,     &sq2[0].minAcc );
					

					/* === 1 STEP : record (17 items check) =============== */
					iSave = 0;
					iItemCurOK = 0; /* NG - FAIL */
					if(QUAL_2ND_NEWDT_ITEM_NUM==result)
					{
						iItemCurOK = 1; /* OK */
						iOKcount ++;
					}
					else if( (QUAL_2ND_NEWDT_ITEM_NUM!=result) && (result!=-1 && i>0) ) 
					{
						iNGcount ++;
						continue; /* reading Next item because of FAIL item */
					}
					/* === 1 STEP : record (17 items check) =============== */


					if( (sq2[0].iPATs05 == aiPATs05) ) // && (sq2[0].curGear08 != sq2[0].tgtGear11) )
					{

						if( 0==strcmp( sq2[0].sTimePos, TXT_SSTIME) ) 
						{
							isSSarea = 1;

							isSBarea = 0;
							isSParea = 0;

							iSScount++;
							iloop = 0;
						}

						if( 0==strncmp( sq2[0].sTimePos, TXT_SBTIME, 4) ) 
						{
							isSBarea = 1;

							isSSarea = 0;
							isSParea = 0;

							/* SB point just one point saved!! */
							gminVals[iloop].iIndex = iloop;
							gminVals[iloop].msTime = (sq2[0].Time01);
							gminVals[iloop].mValue = (sq2[0].minAcc);

							iSBcount++;
						}

						if( 0==strncmp( sq2[0].sTimePos, TXT_SPTIME, 4) ) 
						{
							isSParea = 1;

							isSSarea = 0;
							isSBarea = 0;

							qsort(gminVals, iloop, sizeof(tminMax_type), doubleCompUp); 
							qsort(gMaxVals, sizeof(gMaxVals)/sizeof(tminMax_type), sizeof(tminMax_type), doubleCompDown); 
							//qsort(gminVals, sizeof(gminVals)/sizeof(double), sizeof(double), doubleCompUp); 


						#if 0 // MAX_DATA_FILE_SAVED
							if(fp2gMax)
							{
								fprintf(fp2gMax, gMAX_FORMAT,
										iSScount, sq2[0].Time01, gMaxVals[0] );
						
								fprintf(fp2gMax, "\n");
							}

						#else
							gMaxFinVals[iSPcount].iIndex = iSPcount;
							gMaxFinVals[iSPcount].msTime = gMaxVals[0].msTime;
							gMaxFinVals[iSPcount].mValue = gMaxVals[0].mValue;

							gminFinVals[iSPcount].iIndex = iSPcount;
							gminFinVals[iSPcount].msTime = gminVals[0].msTime;
							gminFinVals[iSPcount].mValue = gminVals[0].mValue;
						#endif

							//fprintf(stderr," iSPcount = %3d -> gMax: %.4lf  gmin: %.4lf \n", iSPcount, gMaxFinVals[iSPcount], gminFinVals[iSPcount] );
							memset(gMaxVals, 0x00, MAX_TABLE_SIZ*sizeof(tminMax_type));
							memset(gminVals, 0x00, MAX_TABLE_SIZ*sizeof(tminMax_type));
							iloop = 0;

							iSPcount++;
						}


						iCurrTime = (unsigned int)( (sq2[0].Time01)*1000*JERK_TIME_SCALE );

						if( isSSarea ) // && (gMaxTbl[iSScount].gMx1Begin <= iCurrTime) )  /* SS~SB */
						{
							//if( (gMaxTbl[iSScount].gMx1Begin <= iCurrTime) )
							gMaxVals[iloop].iIndex = iloop;
							gMaxVals[iloop].msTime = (sq2[0].Time01);
							gMaxVals[iloop].mValue = (sq2[0].MaxAcc);

							gminVals[iloop].iIndex = iloop;
							gminVals[iloop].msTime = (sq2[0].Time01);
							gminVals[iloop].mValue = (sq2[0].minAcc);

							//fprintf(stderr, "==== %3d -> gMax: %.4lf  gmin: %.4lf \n", iloop, gMaxVals[iloop], gminVals[iloop] );
							iloop++;
						}



					}

					if( isSBarea )
					{
						gMaxFinVals[iSBcount].SBnumber = sq2[0].sPosNum;
					}
					
					if( isSParea )
					{
						gMaxFinVals[iSPcount].SPnumber = sq2[0].sPosNum;
					}


				}

			}
			while (!feof (inpfile));

		}

		fprintf(stderr,">>Searched G-min/G-Max %s --- SS(%d), SB(%d), SP(%d), SF(%d) points \r\n", 
							arrPATs_ModeID[aiPATs05].ModeID, SPoint.SStot, SPoint.SBtot, SPoint.SPtot, SPoint.SFtot );

		//if(fp2gMax) { fclose(fp2gMax); fp2gMax=NULL; }
	}

	//if(inpfile) fclose(inpfile);

	//if(fp2gMax) { fclose(fp2gMax); fp2gMax=NULL; }
	//if(fp2gmin) { fclose(fp2gmin); fp2gmin=NULL; }

	//fprintf(stderr,"----------------------------------------------------------------------------------\n" );

#if 0
	for(ii=0; ii<iSPcount; ii++)
	{
		fprintf(stderr, "index: %3d -> GMax: %10.4lf / %10.4lf   Gmin: %10.4lf / %10.4lf  \n", ii, 
			gMaxFinVals[ii].msTime, gMaxFinVals[ii].mValue, gminFinVals[ii].msTime, gminFinVals[ii].mValue );
	}
#endif


#if SAVEMODE
	if(fp2out)
		fprintf(fp2out, TITLE_SHIGG "\n");
#endif


	rewind(inpfile);

	iSScount = 0;
	iSBcount = 0;
	iSPcount = 0;

	isSSarea = 0;
	isSBarea = 0;
	isSParea = 0;
	isSFarea = 0;

	RecordCnt  = 0ULL;

	iNGcount   = 0ULL;
	iOKcount   = 0ULL;

	
	iNGcount   = 0ULL;
	iOKcount   = 0ULL;




	do
	{
		unsigned int i=0;

		/* Read a line from input file. */
		memset( QualData, 0x00, sizeof(QualData) );

		if( NULL == fgets( QualData, QUAL_DATA_MAX_SIZE, inpfile ) )
		{
			ierr = ferror(inpfile);
			//fclose(inpfile);
			break;
		}

		RecordCnt++;


		/* Remove carriage return/line feed at the end of line. */
		i = strlen(QualData);
		
		if(i >= QUAL_DATA_MAX_SIZE)
		{
			fprintf(stderr,"ERROR:%6lld: Not enough Buffer length (%d) \r\n", RecordCnt, i );
		}

		if (--i > 0)
		{

			if (QualData[i] == '\n') QualData[i] = '\0';
			if (QualData[i] == '\r') QualData[i] = '\0'; 

			result = sscanf(QualData, RD_23YNW,
					&sq2[0].Time01,  &sq2[0].iPATs05,	 &sq2[0].iPATs05S,	 &sq2[0].VSP03, 	 &sq2[0].tqi07, 	 &sq2[0].curGear08, 
					&sq2[0].APS09,	 &sq2[0].No10,		 &sq2[0].tgtGear11,  &sq2[0].ShiNew12,	 &sq2[0].ShiTy12,	 &sq2[0].arrGear, 
					&sq2[0].TqFr13,  &sq2[0].ShiPh14,	 &sq2[0].Ne15,		 &sq2[0].Nt16,		 &sq2[0].LAcc17,	 &sq2[0].LPFAcc,
					&sq2[0].sTimePos, 
					&sq2[0].sPosNum, &sq2[0].Gsum,       &sq2[0].sTimeNtPos, &sq2[0].gearRat,	 &sq2[0].gearShiOne, &sq2[0].gearShiTwo, 
					&sq2[0].fJerk0,  &sq2[0].MaxAcc,     &sq2[0].minAcc );

			/* === 1 STEP : record (26 items check) =============== */
			iSave = 0;
			iItemCurOK = 0; /* NG - FAIL */
			if(QUAL_2ND_NEWDT_ITEM_NUM==result)
			{
				iItemCurOK = 1; /* OK */
				iOKcount ++;
			}
			else if( (QUAL_2ND_NEWDT_ITEM_NUM!=result) && (result!=-1 && i>0) ) 
			{
				iNGcount ++;
				continue; /* reading Next item because of FAIL item */
			}
			/* === 1 STEP : record (26 items check) =============== */



			/* ------------------------------------------------------------------- */
			/* -- NOT saving ----------------------------------------------------- */
			/* ------------------------------------------------------------------- */
			is2File = 0;
			
			/* ------------------------------------------------------------------- */
			/* ------------------------------------------------------------------- */
			if( (sq2[0].iPATs05 == aiPATs05) && (sq2[0].curGear08 == sq2[0].tgtGear11) )
			{
				if( 0==strcmp( sq2[0].sTimePos, TXT_SFTIME) ) /* SF point */ // (curSmPreGear+1 == sq2[0].curGear08) 
				{
					isSFarea = 1;
					isSParea = 0;
					isSSarea = 0;
					isSBarea = 0;
				
					is2File = 2;
				
					iSFcount ++;
				}

				//curSmPreGear = sq2[0].curGear08;
				//tgtSmPreGear = sq2[0].tgtGear11;

			}
			/* ------------------------------------------------------------------- */
			/* ------------------------------------------------------------------- */
			else if( (sq2[0].iPATs05 == aiPATs05) && (sq2[0].curGear08 != sq2[0].tgtGear11) )
			{
				is2File = 1;
				if( 0==strcmp( sq2[0].sTimePos, TXT_SSTIME) ) /* SS point */
				{
					isSSarea = 1;
					isSBarea = 0;
					isSParea = 0;

					is2File = 1;
					iSScount ++;
					GminChecked = 1;
					GMaxChecked = 1;
				}
				else if( 0==strncmp( sq2[0].sTimePos, TXT_SBTIME, 4) ) /* SB point */
				{
					isSBarea = 1;

					is2File = 1;
					iSBcount ++;
				}
				else if( 0==strcmp( sq2[0].sTimePos, TXT_SPTIME) ) /* SP point */
				{
					isSParea = 1;
					isSSarea = 0;
					isSBarea = 0;

					is2File = 1;

					iSPcount ++;
				}


				/* -------------------------------------------------- */
				/* GMax/Gmin string default ------------------------- */
				/* -------------------------------------------------- */
				strcpy( szTimeG, TXT_UNKNOWN);

				if( isSSarea || isSBarea )
				{
					// just scale up because of compare GMAX

				#ifndef LOW_PASS_FILTER_GACC /* 2023-02-09, Low Pass Filter for LAcc */
					iGMax = (int)( (gMaxFinVals[iSPcount].mValue)*10000 );
					if( iGMax==(int)((sq2[0].LAcc17)*10000) && iGMax==(int)((sq2[0].MaxAcc)*10000) ) 
					{
						if( GMaxChecked )
						{
							strcpy( szTimeG, TXT_gMAX);
							GMaxChecked = 0;
						}
					}

					// just scale up because of compare Gmin
					iGmin = (int)( (gminFinVals[iSPcount].mValue)*10000 );
					if( iGmin==(int)((sq2[0].LAcc17)*10000) && iGmin==(int)((sq2[0].minAcc)*10000) ) 
					{
						if( GminChecked )
						{
							strcpy( szTimeG, TXT_gmin );
							GminChecked = 0;
						}
					}
 
 				#else /* Low Pass Filtered Data */
					// just scale up because of compare GMAX
					iGMax = (int)( (gMaxFinVals[iSPcount].mValue)*10000 );
					if( iGMax==(int)((sq2[0].LPFAcc)*10000) && iGMax==(int)((sq2[0].MaxAcc)*10000) ) 
					{
						if( GMaxChecked )
						{
							strcpy( szTimeG, TXT_gMAX);
							GMaxChecked = 0;
						}
					}
					
					// just scale up because of compare Gmin
					iGmin = (int)( (gminFinVals[iSPcount].mValue)*10000 );
					if( iGmin==(int)((sq2[0].LPFAcc)*10000) && iGmin==(int)((sq2[0].minAcc)*10000) ) 
					{
						if( GminChecked )
						{
							strcpy( szTimeG, TXT_gmin );
							GminChecked = 0;
						}
					}
				#endif
				

				}


			}

			


		#if SAVEMODE
			if(fp2out && is2File)
			{
				fprintf(fp2out, SAVE_23YNW,
						sq2[0].Time01,     sq2[0].iPATs05,	sq2[0].iPATs05S,   sq2[0].VSP03,	  sq2[0].tqi07,       sq2[0].curGear08, 
						sq2[0].APS09,      sq2[0].No10,     sq2[0].tgtGear11,  sq2[0].ShiNew12,   sq2[0].ShiTy12,     sq2[0].arrGear, 
						sq2[0].TqFr13,	   sq2[0].ShiPh14,	sq2[0].Ne15,	   sq2[0].Nt16, 	  sq2[0].LAcc17,      sq2[0].LPFAcc,
						sq2[0].sTimePos, 
						sq2[0].sPosNum,    sq2[0].Gsum,     sq2[0].sTimeNtPos, szTimeG,           sq2[0].gearRat,	  sq2[0].gearShiOne,  
						sq2[0].gearShiTwo, sq2[0].fJerk0,   sq2[0].MaxAcc,     sq2[0].minAcc );
		
				fprintf(fp2out, "\n");
			}
		#endif

		}

	}
	while (!feof (inpfile));



	if(inpfile) { fclose(inpfile); inpfile=NULL; }
	if(fp2out) { fclose(fp2out); fp2out=NULL; }


	return 1;
}



unsigned int ShiftData_Filtering(short aiPATs05, int avgTime, short iSBchoicePnt)
{
	FILE *fp2in = NULL;
	FILE *fp2out = NULL;

	sqd3rd_type sq3[2];  /* 0:SS, 1:SB, 2:MaxNt, 3:MaxNe, 4:g_Max, 5:g_min, 6:SP */
	int  ierr = -1;
	char shi_inp[MAX_CHARS*LENGTH_OF_FILENAME+1];
	char shi_out[MAX_CHARS*LENGTH_OF_FILENAME+1];
	int ii = 0;
	double fJerk1 = 0.0f;
	
	/* line inputted from file */
	char QualData[QUAL_DATA_MAX_SIZE]; 
	int result;
	unsigned long long RecordCnt=0ULL;

	unsigned long long iNGcount = 0ULL;
	unsigned long long iOKcount = 0ULL;

	unsigned int iSScount = 0;
	unsigned int iSBcount = 0;
	unsigned int iSPcount = 0;
	unsigned int iSFcount = 0;
	unsigned int iignoredCnt = 0;
	
	double gTimeDiff = 0.0f;
	double gSStime = 0.0f;
	double gSBtime = 0.0f;
	double gSPtime = 0.0f;
	double gSFtime = 0.0f;

	double gMaxTime = 0.0f;
	double gminTime = 0.0f;

	double gMaxVal = 0.0f;
	double gminVal = 0.0f;

	double NtMaxTime = 0.0f;
	double NtminTime = 0.0f;
	double NtMaxVal = 0.0f;
	double NtminVal = 0.0f;

	short is2File = 0;
	short isSSpoint = 0;
	short isSBpoint = 0;
	short isSPpoint = 0;
	short isSFpoint = 0;
	
	short ig_Max = 0, ig_min = 0;
	short isNtmin = 0, isNtMax = 0;
	short isGMax = 0, isGmin = 0; /* To be or NOT be */
	short isNtmin_OnSP = 0;
	
	double 	DeltaTime = 0.0f;
	double	DeltaValu = 0.0f;
	int isNaming = 0;
	short iGminMaxDuplicate = 0;
	unsigned int ipreGsumCnt = 0;
	double ipreGsum = 0.0f;
	double Gavg = 0.0f;

	int ipreCG = 0;
	int ipreTG = 0;
	int ipreShi = 0; 
	char ipreArrGear[10];
	double ipreAPS = 0;

	

	AllFilesClosed();


	strcpy(shi_inp, shift_file);
	strcpy(shi_out, shift_file);

	if( (aiPATs05>=0 && aiPATs05<MODE_ID_NUMS-1) && (strlen(shi_out)>0) )
	{
		isNaming = 0;
		for(ii=strlen(shi_out)-1; ii>0; ii--)
		{
			if( shi_out[ii]=='.' ) 
			{
				isNaming = 1;
				shi_inp[ii+1] = '\0';
				shi_out[ii+1] = '\0';

				strcat(shi_inp, "g");
				strcat(shi_inp, arrPATs_ModeID[aiPATs05].ModeNm);
				
				strcat(shi_out, FILTER_EXT_DTA );
				break;
			}
		}

		if( 0==isNaming )
		{
			strcat(shi_inp, ".g");
			strcat(shi_inp, arrPATs_ModeID[aiPATs05].ModeNm);

			strcat(shi_out, ".");
			strcat(shi_out, FILTER_EXT_DTA);
		}
	}


	/* ===================================================================================== */


	/* ===================================================================================== */
	// read file OK
	if( NULL == (fp2in = fopen( shi_inp, "rb")) ) 
	{
		// FAIL
		fprintf(stderr,"\r\n++ERROR++[%s]:Can not read file (%s) \n\n", __FUNCTION__, shi_inp );
		AllFilesClosed();
		exit(0);
	}
	

	// write file OK
	if( NULL == (fp2out = fopen( shi_out, "wb")) ) 
	{
		// FAIL
		fprintf(stderr,"\r\n++ERROR++[%s]:Can not read file (%s) \n\n", __FUNCTION__, shi_out );
		AllFilesClosed();
		exit(0);
	}
	/* ===================================================================================== */



	/* ===================================================================================== */
	/* ===================================================================================== */

	memset(QualData, 0x00, QUAL_DATA_MAX_SIZE*sizeof(char) );
	memset(sq3, 0x00, sizeof(sq3) );

	memset( gMaxTbl, 0x00, MAX_TABLE_SIZ*sizeof(tSBtimePos_type) );

	RecordCnt  = 0ULL;
	iNGcount   = 0ULL;
	iOKcount   = 0ULL;

	iSScount   = 0U;
	iSBcount   = 0U;
	iSPcount   = 0U;
	iSFcount   = 0U;
	iignoredCnt = 0U;

#if SAVEMODE
	if(fp2out)
		fprintf(fp2out, TITLE_TXT "\n", iSBdecision, iSBchoicePnt?"L":"F", avgTime, iJerkTimeLen);
#endif


	do
	{
		unsigned int i=0;

		/* Read a line from input file. */
		memset( QualData, 0x00, sizeof(QualData) );

		if( NULL == fgets( QualData, QUAL_DATA_MAX_SIZE, fp2in ) )
		{
			ierr = ferror(fp2in);
			fclose(fp2in);
			fprintf(stderr,"----------------------------------------------------------------------------------\n" );
			//fprintf(stderr,">>Shift Quality Data Filtering completed!!! %s [%s] -> [%s] \r\n", arrPATs_ModeID[aiPATs05].ModeID, shi_inp, shi_out ); 			
			fprintf(stderr,">>Shift Quality Data Filtering completed!!! %s      \r\n", arrPATs_ModeID[aiPATs05].ModeID ); 			
			break;
		}

		RecordCnt++;


		/* Remove carriage return/line feed at the end of line. */
		i = strlen(QualData);
		
		if(i >= QUAL_DATA_MAX_SIZE)
		{
			fprintf(stderr,"ERROR:%6llu: Not enough Buffer length---(%u) \r\n", RecordCnt, i );
		}

		if (--i > 0)
		{
			if (QualData[i] == '\n') QualData[i] = '\0';
			if (QualData[i] == '\r') QualData[i] = '\0'; 

			result = sscanf(QualData, RD_NewFM,
						&sq3[0].Time01,     &sq3[0].iPATs05,    &sq3[0].iPATs05S,   &sq3[0].VSP03,     &sq3[0].tqi07,      &sq3[0].curGear08, 
						&sq3[0].APS09,      &sq3[0].No10,       &sq3[0].tgtGear11,  &sq3[0].ShiNew12,  &sq3[0].ShiTy12,    &sq3[0].arrGear, 
						&sq3[0].TqFr13,     &sq3[0].ShiPh14,    &sq3[0].Ne15,       &sq3[0].Nt16,      &sq3[0].LAcc17,     &sq3[0].LPFAcc,
						&sq3[0].sTimePos,
						&sq3[0].sPosNum,    &sq3[0].Gsum,       &sq3[0].sTimeNtPos, &sq3[0].sTimeGpos,  &sq3[0].gearRat,   &sq3[0].gearShiOne, 
						&sq3[0].gearShiTwo, &sq3[0].fJerk0,     &sq3[0].MaxAcc,     &sq3[0].minAcc );


			/* === 1 STEP : record (27 items check) =============== */
			if(QUAL_3RD_DATA_ITEM_NUM==result)
			{
				iOKcount ++;
			}
			else if( (QUAL_3RD_DATA_ITEM_NUM!=result) && (result!=-1 && i>0) ) 
			{
				iNGcount ++;
				continue; /* reading Next item because of FAIL item */
			}
			/* === 1 STEP : record (17 items check) =============== */


			is2File = 0;
			gTimeDiff = 0.0f;

			fJerk1 = 0.0f;
			DeltaTime = 0.0f;
			Gavg = 0.0f;


			/* ------------------------------------------------ */
			/* SS Point  ------------ */
			/* ------------------------------------------------ */
			if( 0==strcmp(sq3[0].sTimePos, TXT_SSTIME) || 
				0==strcmp(sq3[0].sTimePos, TXT_SBTIME) || 
				0==strcmp(sq3[0].sTimePos, TXT_SPTIME) ||
				0==strcmp(sq3[0].sTimePos, TXT_SFTIME) )
			{
				is2File = 1;

				if( 0==strcmp( sq3[0].sTimePos, TXT_SSTIME) ) /* SS point */
				{
					isSSpoint = 0;
					isSBpoint = 0;
					isSPpoint = 0;
					isSFpoint = 0;

					ig_Max = 0;
					ig_min = 0;

					isGMax = 0;
					isGmin = 0;

					isNtmin = 0;
					isNtMax = 0;
					isNtmin_OnSP = 0;

					is2File = 1;
					isSSpoint = 1;
					gSStime = sq3[0].Time01;

					gMaxTbl[iSScount].ignored = 0; /* zero-Clear */

					gMaxTbl[iSScount].SSTime = (unsigned int)( (sq3[0].Time01)*1000*JERK_TIME_SCALE );

					iSScount ++;
					gTimeDiff = gSStime;


				}
				/* ------------------------------------------------ */
				/* SB Point  ------------ */
				/* ------------------------------------------------ */
				else if( 0==strcmp( sq3[0].sTimePos, TXT_SBTIME) )  /* SB point */
				{
					is2File = 1;
					isSBpoint = 1;
					gSBtime = sq3[0].Time01;

					iSBcount++;
					gTimeDiff = (gSBtime - gSStime);

					gMaxTbl[iSScount-1].SBTime = (unsigned int)( (sq3[0].Time01)*1000*JERK_TIME_SCALE );

					/* G average for SS~SB */
					if(ipreGsumCnt)
					{
						Gavg = (double)(ipreGsum/ipreGsumCnt);
					}
					else
					{
						fprintf(stderr,"++ERROR+1+ Gaverage checking.. (ipreGsum(%lf)/ipreGsumCnt(%d)  \n", ipreGsum, ipreGsumCnt );
					}


				
				}
				/* ------------------------------------------------ */
				/* SP Point  ------------ */
				/* ------------------------------------------------ */
				else if( 0==strcmp( sq3[0].sTimePos, TXT_SPTIME ) ) /* SP point */
				{
					is2File = 1;

					isSPpoint = 1;
					gSPtime = sq3[0].Time01;

					iSPcount ++;
					gTimeDiff = (gSPtime - gSBtime);

					gMaxTbl[iSScount-1].SPTime = (unsigned int)( (sq3[0].Time01)*1000*JERK_TIME_SCALE );

					/* G average for SB~SP */
					if(ipreGsumCnt)
					{
						Gavg = (double)(ipreGsum/ipreGsumCnt);
					}
					else
					{
						fprintf(stderr,"++ERROR+2+ Gaverage checking.. (ipreGsum(%lf)/ipreGsumCnt(%d)  \n", ipreGsum, ipreGsumCnt );
					}

					gMaxTime = -1.0f;
					gminTime = -1.0f;


					if(0==isGMax)
					{

						gMaxTbl[iSScount-1].ignored = IGN_BECAUSE_GMAX_NONE;
						gMaxTbl[iSScount-1].SPTime  = 0UL;

				#if SAVEMODE
						/* SS, SB, SP saved!! */
						if(fp2out && is2File)
						{
							fprintf(fp2out, SAVE_FltFMT,
									0,	  sq3[0].iPATs05,  sq3[0].iPATs05S,   ipreCG,  ipreTG,	0, 
									0,	  ipreAPS,	  0,	  sq3[0].ShiNew12,	 ipreShi,	ipreArrGear, 
									0,	  0,	0,	  0,   0,	0, 0 /*LPFAcc*/,
									TXT_gMAX_NONE,	0,	 0,   sq3[0].sTimeNtPos, TXT_gMAX_NONE, 0, 
									0,	  0, 0 );
					
							fprintf(fp2out, "\n");
						}
				#endif
					}


					if(0==isGmin)
					{

						gMaxTbl[iSScount-1].ignored = IGN_BECAUSE_GMIN_NONE;
						gMaxTbl[iSScount-1].SPTime  = 0UL;

				#if SAVEMODE
						/* SS, SB, SP saved!! */
						if(fp2out && is2File)
						{
							fprintf(fp2out, SAVE_FltFMT,
									0,	  sq3[0].iPATs05,  sq3[0].iPATs05S,   ipreCG,  ipreTG,	0, 
									0,	  ipreAPS,	  0,	  sq3[0].ShiNew12,	 ipreShi,	ipreArrGear, 
									0,	  0,	0,	  0,   0,	0, 0 /*LPFAcc*/,
									TXT_gmin_NONE,	0,	 0,   sq3[0].sTimeNtPos, TXT_gmin_NONE, 0, 
									0,	  0, 0 );
					
							fprintf(fp2out, "\n");
						}
				#endif
					}


				#if 0
					/* Same Line SP and Nt-min*/
					if( 0==strcmp(sq3[0].sTimeNtPos, TXT_NtminTIME) )
					{
						is2File = 1;

						isNtmin = 1;

						NtminTime = sq3[0].Time01;
						NtminVal  = sq3[0].Nt16;

						//gTimeDiff = (NtminTime - gSStime);

						fprintf(stderr," %5d (SS time: %12.4lf) : SP overlaps %s + %s \n", iSScount-1, gMaxTbl[iSScount-1].SSTime, sq3[0].sTimePos, TXT_NtminTIME );
						strcat(sq3[0].sTimePos, TXT_NtminTIME);
						isNtmin_OnSP = 1;
					}
				#endif

				}
				/* ------------------------------------------------ */
				/* SF Point  ------------ */
				/* ------------------------------------------------ */
				else if( 0==strcmp( sq3[0].sTimePos, TXT_SFTIME ) ) /* SF point */
				{
					is2File = 2;

					isSFpoint = 1;
					gSFtime = sq3[0].Time01;

					iSFcount ++;
					gTimeDiff = (gSFtime - gSPtime);

					gMaxTbl[iSScount-1].SFTime = (unsigned int)( (sq3[0].Time01)*1000*JERK_TIME_SCALE );

					/* G average for SP~SF */
					if(ipreGsumCnt)
					{
						Gavg = (double)(ipreGsum/ipreGsumCnt);
					}
					else
					{
						fprintf(stderr,"++ERROR+3+ Gaverage checking.. (ipreGsum(%lf)/ipreGsumCnt(%d)  \n", ipreGsum, ipreGsumCnt );
					}

					gMaxTime = -1.0f;
					gminTime = -1.0f;


					if(0==isNtMax)
					{

						gMaxTbl[iSScount-1].ignored = IGN_BECAUSE_NTMAX_NONE;
						gMaxTbl[iSScount-1].SFTime  = 0UL;

					#if SAVEMODE
						/* SS, SB, SP saved!! */
						if(fp2out && is2File)
						{
							fprintf(fp2out, SAVE_FltFMT,
									0,	  sq3[0].iPATs05,  sq3[0].iPATs05S,   ipreCG,  ipreTG,	0, 
									0,	  ipreAPS,	0,	  sq3[0].ShiNew12,	 ipreShi,	ipreArrGear, 
									0,	  0,	0,	  0,   0,	0, 0 /*LPFAcc*/,
									TXT_NtMaxTIMENONE,  0,   0,   TXT_NtMaxTIMENONE, sq3[0].sTimeGpos,	0, 
									0,	  0, 0 );
					
							fprintf(fp2out, "\n");
						}
					#endif

					}

					if(0==isNtmin)
					{

						gMaxTbl[iSScount-1].ignored = IGN_BECAUSE_NTMIN_NONE;
						gMaxTbl[iSScount-1].SFTime  = 0UL;

					#if SAVEMODE
						/* SS, SB, SP saved!! */
						if(fp2out && is2File)
						{
							fprintf(fp2out, SAVE_FltFMT,
									0,	  sq3[0].iPATs05,  sq3[0].iPATs05S,   ipreCG,  ipreTG,	0, 
									0,	  ipreAPS,    0, 	  sq3[0].ShiNew12,	 ipreShi,	ipreArrGear, 
									0,	  0,    0, 	  0,   0, 	0,  0 /*LPFAcc*/,
									TXT_NtminTIMENONE,  0,   0,   TXT_NtminTIMENONE, sq3[0].sTimeGpos,	0, 
									0,	  0, 0 );
					
							fprintf(fp2out, "\n");
						}
					#endif
					}


				}


			#if SAVEMODE			
				/* SS, SB, SP saved!! */
				if(fp2out && is2File)
				{
					fprintf(fp2out, SAVE_FltFMT,
							sq3[0].Time01,	  sq3[0].iPATs05,  sq3[0].iPATs05S,   sq3[0].curGear08,  sq3[0].tgtGear11,	sq3[0].VSP03, 
							sq3[0].tqi07,	  sq3[0].APS09,    sq3[0].No10, 	  sq3[0].ShiNew12,	 sq3[0].ShiTy12,	sq3[0].arrGear, 
							sq3[0].TqFr13,	  sq3[0].ShiPh14,  sq3[0].Ne15, 	  sq3[0].Nt16,		 sq3[0].LAcc17, 	sq3[0].LPFAcc,
							gTimeDiff, 
							sq3[0].sTimePos,  sq3[0].sPosNum,  Gavg,              sq3[0].sTimeNtPos, sq3[0].sTimeGpos,  sq3[0].gearRat,	
							sq3[0].fJerk0,    fJerk1,          (int)(DeltaTime/JERK_TIME_SCALE/JERK_TIME_SCALE) );
			
					if(1==is2File) fprintf(fp2out, "\n");
					if(2==is2File) fprintf(fp2out, "\n\n");
				}
			#endif


				ipreCG      = sq3[0].curGear08;
				ipreTG      = sq3[0].tgtGear11;
				ipreShi     = sq3[0].ShiTy12;
				strcpy(ipreArrGear, sq3[0].arrGear);
				ipreAPS     = sq3[0].APS09;
				
				ipreGsumCnt = sq3[0].sPosNum;
				ipreGsum    = sq3[0].Gsum;
			}


			iGminMaxDuplicate = 0;
			if( 0==strcmp(sq3[0].sTimeNtPos, TXT_NtMaxTIME) || 0==strcmp(sq3[0].sTimeNtPos, TXT_NtminTIME) )
			{
				is2File = 1;

				if( 0==strcmp( sq3[0].sTimeNtPos, TXT_NtMaxTIME ) ) /* NtMx */
				{
					isNtMax = 1;
					
					/* CASE : NtMax Position = gmin or gMax position */
					if( 0==strcmp(sq3[0].sTimeGpos, TXT_gMAX) || 0==strcmp(sq3[0].sTimeGpos, TXT_gmin) )
					{
						iGminMaxDuplicate = 1;
						is2File = 0; /* IMPORTANT!! NOT saved!! */
						if( 0==strcmp( sq3[0].sTimeGpos, TXT_gMAX ) ) /* gMAX */
						{
							isGMax = 1; /* To be or NOT be */

							ig_Max = 1;
							gMaxTime = sq3[0].Time01;

						#ifndef LOW_PASS_FILTER_GACC /* 2023-02-09, Low Pass Filter for LAcc */
							gMaxVal  = sq3[0].LAcc17;
						#else /* Low Pass Filtered Data */
							gMaxVal  = sq3[0].LPFAcc;
						#endif
						
							//~ gTimeDiff = (gMaxTime - gSStime);
						}
						else if( 0==strcmp( sq3[0].sTimeGpos, TXT_gmin ) ) /* gmin */
						{
							isGmin = 1; /* To be or NOT be */

							ig_min = 1;
							gminTime = sq3[0].Time01;

						#ifndef LOW_PASS_FILTER_GACC /* 2023-02-09, Low Pass Filter for LAcc */
							gminVal  = sq3[0].LAcc17;
						#else /* Low Pass Filtered Data */
							gminVal  = sq3[0].LPFAcc;
						#endif
						
							//~ gTimeDiff = (gminTime - gSStime);
						}
					}

					NtMaxTime = sq3[0].Time01;
					NtMaxVal  = sq3[0].Nt16;

					gTimeDiff = (NtMaxTime - gSStime);

					if( 0==strcmp(sq3[0].sTimePos, TXT_UNKNOWN) || 
						0==strcmp(sq3[0].sTimePos, TXT_UPCASE) || 
						0==strcmp(sq3[0].sTimePos, TXT_DNCASE) )
					{
						strcpy(sq3[0].sTimePos, TXT_NtMaxTIME);
					}
					else
					{
						fprintf(stderr," %5d (SS time: %12.4lf) : Nt-Max overlaps %s + %s \n", 
							iSScount-1, gMaxTbl[iSScount-1].SSTime/1000.0/JERK_TIME_SCALE, sq3[0].sTimePos, TXT_NtMaxTIME );
						strcat(sq3[0].sTimePos, TXT_NtMaxTIME);
					}

				}
				else if( 0==strcmp( sq3[0].sTimeNtPos, TXT_NtminTIME ) ) /* Ntmn */
				{
					isNtmin = 1;

					/* CASE : NtMax Position = gmin or gMax position */
					if( 0==strcmp(sq3[0].sTimeGpos, TXT_gMAX) || 0==strcmp(sq3[0].sTimeGpos, TXT_gmin) )
					{
						iGminMaxDuplicate = 1;
						is2File = 0; /* IMPORTANT!! NOT saved!! */
						if( 0==strcmp( sq3[0].sTimeGpos, TXT_gMAX ) ) /* gMAX */
						{
							isGMax = 1; /* To be or NOT be */

							ig_Max = 1;
							gMaxTime = sq3[0].Time01;

						#ifndef LOW_PASS_FILTER_GACC /* 2023-02-09, Low Pass Filter for LAcc */
							gMaxVal  = sq3[0].LAcc17;
						#else /* Low Pass Filtered Data */
							gMaxVal  = sq3[0].LPFAcc;
						#endif
						
							//~ gTimeDiff = (gMaxTime - gSStime);
						}
						else if( 0==strcmp( sq3[0].sTimeGpos, TXT_gmin ) ) /* gmin */
						{
							isGmin = 1; /* To be or NOT be */

							ig_min = 1;
							gminTime = sq3[0].Time01;

						#ifndef LOW_PASS_FILTER_GACC /* 2023-02-09, Low Pass Filter for LAcc */
							gminVal  = sq3[0].LAcc17;
						#else /* Low Pass Filtered Data */
							gminVal  = sq3[0].LPFAcc;
						#endif
						
							// ~ gTimeDiff = (gminTime - gSStime);
						}
					}

					NtminTime = sq3[0].Time01;
					NtminVal  = sq3[0].Nt16;

					gTimeDiff = (NtminTime - gSStime);

					if( 0==strcmp(sq3[0].sTimePos, TXT_UNKNOWN) || 
						0==strcmp(sq3[0].sTimePos, TXT_UPCASE) || 
						0==strcmp(sq3[0].sTimePos, TXT_DNCASE) )
					{
						strcpy(sq3[0].sTimePos, TXT_NtminTIME);
					}
					else
					{
						fprintf(stderr," %5d (SS time: %12.4lf) : Nt-min overlaps %s + %s \n", 
							iSScount-1, gMaxTbl[iSScount-1].SSTime/1000.0/JERK_TIME_SCALE, sq3[0].sTimePos, TXT_NtminTIME );
						strcat(sq3[0].sTimePos, TXT_NtminTIME);
					}

					//if( isNtmin_OnSP ) is2File = 0;
				}

			#if SAVEMODE
				/* NtMax, Ntmin saved!! */
				if(fp2out && is2File)
				{
					fprintf(fp2out, SAVE_FltFMT,
							sq3[0].Time01,	  sq3[0].iPATs05,  sq3[0].iPATs05S,   sq3[0].curGear08,  sq3[0].tgtGear11,	sq3[0].VSP03, 
							sq3[0].tqi07,	  sq3[0].APS09,    sq3[0].No10, 	  sq3[0].ShiNew12,	 sq3[0].ShiTy12,	sq3[0].arrGear, 
							sq3[0].TqFr13,	  sq3[0].ShiPh14,  sq3[0].Ne15, 	  sq3[0].Nt16,		 sq3[0].LAcc17, 	sq3[0].LPFAcc,
							gTimeDiff, 
							sq3[0].sTimePos,  sq3[0].sPosNum,  Gavg,			  sq3[0].sTimeNtPos, sq3[0].sTimeGpos,	sq3[0].gearRat, 
							sq3[0].fJerk0,    fJerk1,          (int)(DeltaTime/JERK_TIME_SCALE/JERK_TIME_SCALE) );
			
					if(1==is2File) fprintf(fp2out, "\n");
					if(2==is2File) fprintf(fp2out, "\n\n");
				}
			#endif


			}

			if( 0==strcmp(sq3[0].sTimeGpos, TXT_gMAX) || 0==strcmp(sq3[0].sTimeGpos, TXT_gmin) )
			{
				is2File = 1;

				if( 0==strcmp( sq3[0].sTimeGpos, TXT_gMAX ) ) /* gMAX */
				{
					isGMax = 1; /* To be or NOT be */

					ig_Max = 1;
					gMaxTime = sq3[0].Time01;

				#ifndef LOW_PASS_FILTER_GACC /* 2023-02-09, Low Pass Filter for LAcc */
					gMaxVal  = sq3[0].LAcc17;
				#else /* Low Pass Filtered Data */
					gMaxVal  = sq3[0].LPFAcc;
				#endif

					gTimeDiff = (gMaxTime - gSStime);


					if( 0==strcmp(sq3[0].sTimePos, TXT_UNKNOWN) || 
						0==strcmp(sq3[0].sTimePos, TXT_UPCASE) || 
						0==strcmp(sq3[0].sTimePos, TXT_DNCASE) )
					{
						strcpy(sq3[0].sTimePos, TXT_gMAX);
					}
					else
					{
						fprintf(stderr," %5d (SS time: %12.4lf) : G-Max overlaps %s + %s \n", 
							iSScount-1, gMaxTbl[iSScount-1].SSTime/1000.0/JERK_TIME_SCALE, sq3[0].sTimePos, TXT_gMAX );
						strcat(sq3[0].sTimePos, TXT_gMAX);
					}

				}
				else if( 0==strcmp( sq3[0].sTimeGpos, TXT_gmin ) ) /* gmin */
				{
					isGmin = 1; /* To be or NOT be */

					ig_min = 1;
					gminTime = sq3[0].Time01;


				#ifndef LOW_PASS_FILTER_GACC /* 2023-02-09, Low Pass Filter for LAcc */
					gminVal  = sq3[0].LAcc17;
				#else /* Low Pass Filtered Data */
					gminVal  = sq3[0].LPFAcc;
				#endif

					gTimeDiff = (gminTime - gSStime);

					is2File = 1;

					if( 0==strcmp(sq3[0].sTimePos, TXT_UNKNOWN) || 
						0==strcmp(sq3[0].sTimePos, TXT_UPCASE) || 
						0==strcmp(sq3[0].sTimePos, TXT_DNCASE) )
					{
						strcpy(sq3[0].sTimePos, TXT_gmin);
					}
					else
					{
						fprintf(stderr," %5d (SS time: %12.4lf) : G-min overlaps %s + %s \n", 
							iSScount-1, gMaxTbl[iSScount-1].SSTime/1000.0/JERK_TIME_SCALE, sq3[0].sTimePos, TXT_gmin );
						strcat(sq3[0].sTimePos, TXT_gmin);
					}

				}


				/* ----------------------------------------------------- */
				/* Calc Jerk1											 */
				/* ----------------------------------------------------- */
				fJerk1 = 0.0f;
				DeltaTime = 0;
				if( ig_Max && ig_min )
				{
					ig_Max = 0;
					ig_min = 0;

					DeltaTime = round(gminTime*1000.0*JERK_TIME_SCALE*JERK_TIME_SCALE - gMaxTime*1000.0*JERK_TIME_SCALE*JERK_TIME_SCALE);
					DeltaValu = (gminVal*1000.0f - gMaxVal*1000.0f);

					if( DeltaTime > 0.0f || DeltaTime < 0.0f )
					{
						fJerk1 = DeltaValu*JERK_TIME_SCALE*JERK_TIME_SCALE/DeltaTime;
					}
					else if( 0 == DeltaTime ) 
					{
						fprintf(stderr,"++ERROR++: check DeltaTime... \r\n");
					}
				}
				/* ----------------------------------------------------- */
				/* Calc Jerk1											 */
				/* ----------------------------------------------------- */

			#if SAVEMODE
				if(fp2out && is2File)
				{
					fprintf(fp2out, SAVE_FltFMT,
							sq3[0].Time01,    sq3[0].iPATs05,  sq3[0].iPATs05S,	  sq3[0].curGear08,  sq3[0].tgtGear11,  sq3[0].VSP03, 
							sq3[0].tqi07,     sq3[0].APS09,    sq3[0].No10,       sq3[0].ShiNew12,	 sq3[0].ShiTy12,    sq3[0].arrGear, 
							sq3[0].TqFr13,    sq3[0].ShiPh14,  sq3[0].Ne15,       sq3[0].Nt16,       sq3[0].LAcc17,     sq3[0].LPFAcc,
							gTimeDiff, 
							sq3[0].sTimePos,  sq3[0].sPosNum,  Gavg,			  sq3[0].sTimeNtPos, sq3[0].sTimeGpos,	sq3[0].gearRat, 
							sq3[0].fJerk0,    fJerk1,          (int)(DeltaTime/JERK_TIME_SCALE/JERK_TIME_SCALE) );

					if(1==is2File) fprintf(fp2out, "\n");
					if(2==is2File) fprintf(fp2out, "\n\n");
				}
			#endif

			}

		}

	}
	while (!feof (fp2in));


	if(fp2in)  { fclose(fp2in);   fp2in=NULL; }
	if(fp2out) { fclose(fp2out);  fp2out=NULL; }
	//if(outfile){ fclose(outfile); outfile=NULL; }

	// *.dta file saved!!
	//fprintf(stderr,">>Sorted result file: %s \n", shi_out );
	fprintf(stderr,"----------------------------------------------------------------------------------\n" );


	return iignoredCnt;
}




unsigned int APSData_Filtering(short aiPATs05, int avgTime, short iSBchoicePnt, tSQData_PairCheck_type *SPoint)
{
	FILE *fp2in = NULL;
	FILE *fp2out = NULL;

	sqdflt_type sq9[2]; /* 0:SS, 1:SB, 2:MaxNt, 3:MaxNe, 4:g_Max, 5:g_min, 6:SP */
	char shi_inp[MAX_CHARS*LENGTH_OF_FILENAME+1];
	char shi_out[MAX_CHARS*LENGTH_OF_FILENAME+1];
	short isNaming = 0;
	int ii=0;
	int  ierr = -1;

	/* line inputted from file */
	char QualData[QUAL_DATA_MAX_SIZE]; 
	int result;
	unsigned long long RecordCnt=0ULL;

	unsigned long long iNGcount = 0ULL;
	unsigned long long iOKcount = 0ULL;

	unsigned int iSScount = 0;
	unsigned int iSBcount = 0;
	unsigned int iSPcount = 0;
	unsigned int iSFcount = 0;
	unsigned int iignoredCnt = 0;
		
	short is2File = 0;

	unsigned int ll=0;
	short APSpwrIndex = -1;
	short APSignored = 0;
	double apsMax = 0.0f;
	double apsmin = 0.0f;
	unsigned int ignNtMax = 0;
	unsigned int ignNtmin = 0;
	unsigned int ignGMax = 0;
	unsigned int ignGmin = 0;
	unsigned int ignOthers = 0;

	
	AllFilesClosed();
	if(inpfile) { fclose(inpfile);	inpfile=NULL;  }

	strcpy(shi_inp, shift_file);
	strcpy(shi_out, shift_file);
	if( (aiPATs05>=0 && aiPATs05<MODE_ID_NUMS-1) && (strlen(shi_out)>0) )
	{
		isNaming = 0;
		for(ii=strlen(shi_out)-1; ii>0; ii--)
		{
			if( shi_out[ii]=='.' ) 
			{
				isNaming = 1;
				shi_inp[ii+1] = '\0';
				shi_out[ii+1] = '\0';

				strcat(shi_inp, FILTER_EXT_DTA );
				break;
			}
		}

		if( 0==isNaming )
		{
			strcat(shi_inp, ".");
			strcat(shi_inp, FILTER_EXT_DTA);
		}
	}


	/* ===================================================================================== */
	// read file OK
	if( NULL == (fp2in = fopen( shi_inp, "rb")) ) 
	{
		// FAIL
		fprintf(stderr,"\r\n++ERROR++[%s]:Can not read file (%s) \n\n", __FUNCTION__, shi_inp );
		AllFilesClosed();
		exit(0);
	}

	// write file OK
	if( NULL == (fp2out = fopen( shift_file, "wb")) ) 
	{
		// FAIL
		fprintf(stderr,"\r\n++ERROR++[%s]:Can not read file (%s) \n\n", __FUNCTION__, shift_file );
		AllFilesClosed();
		exit(0);
	}
	/* ===================================================================================== */



	/* ===================================================================================== */
	/* ===================================================================================== */

	memset(QualData, 0x00, QUAL_DATA_MAX_SIZE*sizeof(char) );
	memset(sq9, 0x00, sizeof(sq9) );

	RecordCnt  = 0ULL;
	iNGcount   = 0ULL;
	iOKcount   = 0ULL;

	iSScount   = 0U;
	iSBcount   = 0U;
	iSPcount   = 0U;
	iSFcount   = 0U;
	iignoredCnt = 0U;
	APSignored = 0;

	ignNtMax = 0;
	ignNtmin = 0;
	ignGMax = 0;
	ignGmin = 0;
	ignOthers = 0;

#if SAVEMODE
	if(fp2out)
		fprintf(fp2out, TITLE_TXT "\n", iSBdecision, iSBchoicePnt?"L":"F", avgTime, iJerkTimeLen);
#endif


	fprintf(stderr,">>Checking if APS value is not in APS Table, and then ignored... \n");

	do
	{
		unsigned int i=0;

		/* Read a line from input file. */
		memset( QualData, 0x00, sizeof(QualData) );

		if( NULL == fgets( QualData, QUAL_DATA_MAX_SIZE, fp2in ) )
		{
			ierr = ferror(fp2in);
			fclose(fp2in);
			fprintf(stderr,"----------------------------------------------------------------------------------\n" );
			//fprintf(stderr,">>APS Filtering completed!!! %s [%s] -> [%s] \r\n", arrPATs_ModeID[aiPATs05].ModeID, shi_inp, shift_file ); 			
			//fprintf(stderr,">>APS Filtering completed!!! %s -> %d ignored!!       \r\n", arrPATs_ModeID[aiPATs05].ModeID, iignoredCnt ); 			
			break;
		}

		RecordCnt++;


		/* Remove carriage return/line feed at the end of line. */
		i = strlen(QualData);
		
		if(i >= QUAL_DATA_MAX_SIZE)
		{
			fprintf(stderr,"ERROR:%6llu: Not enough Buffer length---(%u) \r\n", RecordCnt, i );
		}

		if (--i > 0)
		{
			if (QualData[i] == '\n') QualData[i] = '\0';
			if (QualData[i] == '\r') QualData[i] = '\0'; 

			result = sscanf(QualData, RD_TxtFMT,
							&sq9[0].Time01,	  &sq9[0].iPATs05,  &sq9[0].iPATs05S,    &sq9[0].curGear08,  &sq9[0].tgtGear11,  &sq9[0].VSP03, 
							&sq9[0].tqi07,	  &sq9[0].APS09,    &sq9[0].No10,        &sq9[0].ShiNew12,   &sq9[0].ShiTy12,    &sq9[0].arrGear, 
							&sq9[0].TqFr13,	  &sq9[0].ShiPh14,  &sq9[0].Ne15,        &sq9[0].Nt16,       &sq9[0].LAcc17,     &sq9[0].LPFAcc,
							&sq9[0].DiffTime, 
							&sq9[0].sTimePos, &sq9[0].sPosNum,  &sq9[0].Gavg,        &sq9[0].sTimeNtPos, &sq9[0].sTimeGpos,  &sq9[0].gearRat,    
							&sq9[0].fJerk0,	  &sq9[0].fJerk1,	&sq9[0].deltams );


			/* === 1 STEP : record (27 items check) =============== */
			if(QUAL_FLT_DATA_ITEM_NUM==result)
			{
				iOKcount ++;
			}
			else if( (QUAL_FLT_DATA_ITEM_NUM!=result) && (result!=-1 && i>0) ) 
			{
				iNGcount ++;
				continue; /* reading Next item because of FAIL item */
			}
			/* === 1 STEP : record (27 items check) =============== */


			is2File = 1;

			if( 0==strcmp( sq9[0].sTimePos, TXT_SSTIME) ) /* SS point */
			{
				APSpwrIndex = -1; /* initialized */				
				APSignored = 1; /* initialized */ 
				for(ll=0; ll<APS_TABLE_NUM && ApsTble[ll] ; ll++)
				{
					apsMax = ApsTble[ll] + fAPStol;
					apsmin = ApsTble[ll] - fAPStol;
					if( (apsMax >= sq9[0].APS09) && (apsmin <= sq9[0].APS09) )
					{
						// OK
						APSpwrIndex = ll;
						APSignored = 0;
						//fprintf(stderr," %3d APSidx: %2d  %7.2lf fAPStol[ %7.2lf , %7.2lf ] (%.1lf) \n", iSScount, ll, sq9[0].APS09, apsMax, apsmin, fAPStol );
						break;
					}
				}

				iSScount ++;
				is2File = 1;

				if( APSignored ) 
				{
					iignoredCnt ++;
					//iSScount --;
					fprintf(stderr," %5d (SS time: %12.4lf) : APS value %6.2lf <- invalid APS  \n", 
								iSScount-1, gMaxTbl[iSScount-1].SSTime/1000.0/JERK_TIME_SCALE, sq9[0].APS09 );

				}
				
			}
			else if( 0==strncmp( sq9[0].sTimePos, TXT_SBTIME, 4) ) /* SP point */
			{			
				is2File = 1;
				iSBcount ++;

				//if( APSignored ) iSBcount --;

			}
			else if( 0==strncmp( sq9[0].sTimePos, TXT_SPTIME, 4) ) /* SP point */
			{			
				is2File = 1;
				iSPcount ++;

				//if( APSignored ) iSPcount --;

				//fprintf(stderr, "%s - %lf \n", TXT_SPTIME, gTimeDiff  );
			}
			else if( 0==strncmp( sq9[0].sTimePos, TXT_SFTIME, 4) ) /* SF point */
			{			
				is2File = 2;
				iSFcount ++;

				//if( APSignored ) iSFcount --;
			}
			else if( 0==strcmp( sq9[0].sTimePos, TXT_NtminTIME ) ||  /* Ntmin point <- sTimePos  */
					 0==strcmp( sq9[0].sTimeNtPos, TXT_NtminTIME ) ) /* Ntmin point <- sTimeNtPos */
			{			
				is2File = 1; /* 2 New lines */

				//fprintf(stderr, "%s - %lf \n", TXT_SPTIME, gTimeDiff  );
			}
			else if( 0==strcmp( sq9[0].sTimePos, TXT_NtMaxTIME ) ||
					 0==strcmp( sq9[0].sTimeNtPos, TXT_NtMaxTIME ) )
			{
				is2File = 1; /* 1 New lines */
			}
			else if( 0==strcmp( sq9[0].sTimePos, TXT_gMAX ) ||
					 0==strcmp( sq9[0].sTimeGpos, TXT_gMAX ) )
			{
				is2File = 1; /* 1 New lines */
			}
			else if( 0==strcmp( sq9[0].sTimePos, TXT_gmin ) ||
					 0==strcmp( sq9[0].sTimeGpos, TXT_gmin ) )
			{
				is2File = 1; /* 1 New lines */
			}


			if( 1==APSignored )
			{
				is2File = 0; /* NOT saved!!! */
				//continue;
			}
			
			if( gMaxTbl[iSScount-1].ignored )
			{
				/* Just print */
				switch( gMaxTbl[iSScount-1].ignored )
				{					
					case IGN_BECAUSE_NTMAX_NONE:
						if( 0==strcmp( sq9[0].sTimePos, TXT_SSTIME) ) ignNtMax ++;
						break;
					case IGN_BECAUSE_NTMIN_NONE:
						if( 0==strcmp( sq9[0].sTimePos, TXT_SSTIME) ) ignNtmin ++;
						break;
					case IGN_BECAUSE_GMAX_NONE:
						if( 0==strcmp( sq9[0].sTimePos, TXT_SSTIME) ) ignGMax ++;
						break;
					case IGN_BECAUSE_GMIN_NONE:
						if( 0==strcmp( sq9[0].sTimePos, TXT_SSTIME) ) ignGmin ++;
						break;

					case IGN_BECAUSE_GENENAL:
						if( 0==strcmp( sq9[0].sTimePos, TXT_SSTIME) ) ignOthers ++;
						break;
				}

				is2File = 0; /* NOT saved!!! */
				//continue;
			}


		#if SAVEMODE
			if( fp2out && is2File )
			{
				fprintf(fp2out, SAVE_APSFMT, /*SAVE_FltFMT */
						sq9[0].Time01,    sq9[0].iPATs05,  sq9[0].iPATs05S,	  sq9[0].curGear08,  sq9[0].tgtGear11,  sq9[0].VSP03, 
						sq9[0].tqi07,     sq9[0].APS09,    sq9[0].No10,       sq9[0].ShiNew12,	 sq9[0].ShiTy12,    sq9[0].arrGear, 
						sq9[0].TqFr13,    sq9[0].ShiPh14,  sq9[0].Ne15,       sq9[0].Nt16,       sq9[0].LAcc17,     sq9[0].LPFAcc,
						sq9[0].DiffTime, 
						sq9[0].sTimePos,  sq9[0].sPosNum,  sq9[0].Gavg,       sq9[0].sTimeNtPos, sq9[0].sTimeGpos,  sq9[0].gearRat,    
						sq9[0].fJerk0,    sq9[0].fJerk1,   sq9[0].deltams,    APSpwrIndex  );

				if(1==is2File) fprintf(fp2out, "\n");
				if(2==is2File) fprintf(fp2out, "\n\n");
			}
		#endif

		}

	}
	while (!feof (fp2in));


	if(fp2in)  { fclose(fp2in);   fp2in=NULL; }
	if(fp2out) { fclose(fp2out);  fp2out=NULL; }
	if(outfile){ fclose(outfile); outfile=NULL; }


#if 0
	*SScnt = iSScount;
	*SBcnt = iSBcount;
	*SPcnt = iSPcount;
#endif
	
	//fprintf(stderr,">>APS Filtering Sorted result file as below: %s %s \n", arrPATs_ModeID[aiPATs05].ModeID, shift_file );
	fprintf(stderr,">>NONE Data Filtering Sorted result file as below: %s \n", arrPATs_ModeID[aiPATs05].ModeID );

	for(ii=0; ii<iSScount; ii++)
	{
		switch( gMaxTbl[ii].ignored )
		{					
			case IGN_BECAUSE_NTMAX_NONE:
				fprintf(stderr," %5d (SS time: %12.4lf) : NtMax NONE \n", ii, gMaxTbl[ii].SSTime/1000.0/JERK_TIME_SCALE );
				break;
			case IGN_BECAUSE_NTMIN_NONE:
				fprintf(stderr," %5d (SS time: %12.4lf) : Ntmin NONE \n", ii, gMaxTbl[ii].SSTime/1000.0/JERK_TIME_SCALE );
				break;
			case IGN_BECAUSE_GMAX_NONE:
				fprintf(stderr," %5d (SS time: %12.4lf) : G-Max NONE \n", ii, gMaxTbl[ii].SSTime/1000.0/JERK_TIME_SCALE );
				break;
			case IGN_BECAUSE_GMIN_NONE:
				fprintf(stderr," %5d (SS time: %12.4lf) : G-min NONE \n", ii, gMaxTbl[ii].SSTime/1000.0/JERK_TIME_SCALE );
				break;	
			case IGN_BECAUSE_GENENAL:
				fprintf(stderr," %5d (SS time: %12.4lf) : Other NONE \n", ii, gMaxTbl[ii].SSTime/1000.0/JERK_TIME_SCALE );
				break;
		}
	
	}

	if( ignNtMax )
		fprintf(stderr,"  Nt-Max NONE -----------------: %3u invalid...  \n", ignNtMax );
	if( ignNtmin )
		fprintf(stderr,"  Nt-min NONE -----------------: %3u invalid...  \n", ignNtmin );
	if( ignGMax )
		fprintf(stderr,"  G-Max NONE ------------------: %3u invalid...  \n", ignGMax );
	if( ignGmin )
		fprintf(stderr,"  G-min NONE ------------------: %3u invalid...  \n", ignGmin );
	if( ignOthers )
		fprintf(stderr,"  Others ----------------------: %3u invalid...  \n", ignOthers );
	if( iignoredCnt )
		fprintf(stderr,"  Out of APS Power Level ------: %3u invalid...  \n", iignoredCnt );

//	fprintf(stderr,"----------------------------------------------------------------------------------\n" );

	SPoint->SStot = iSScount-iignoredCnt;
	SPoint->SBtot = iSBcount-iignoredCnt;
	SPoint->SPtot = iSPcount-iignoredCnt;
	SPoint->SFtot = iSFcount-iignoredCnt;

	SPoint->SSnum = iSScount-iignoredCnt;
	SPoint->SBnum = iSBcount-iignoredCnt;
	SPoint->SPnum = iSPcount-iignoredCnt;
	SPoint->SFnum = iSFcount-iignoredCnt;


	if( 0==SPoint->SSnum && 0==SPoint->SBnum && 0==SPoint->SPnum )
	{
		fprintf(stderr,"----------------------------------------------------------------------------------\n" );
		fprintf(stderr,">>Result Quality Data is NONE as below %s \n", arrPATs_ModeID[aiPATs05].ModeID );
		fprintf(stderr,"   SS Point Counts -----------: %3u / %3u points \n", SPoint->SStot, SPoint->SSnum);
		fprintf(stderr,"   SB Point Counts -----------: %3u / %3u points \n", SPoint->SBtot, SPoint->SBnum);
		fprintf(stderr,"   SP Point Counts -----------: %3u / %3u points \n", SPoint->SPtot, SPoint->SPnum);
		fprintf(stderr,"   SF Point Counts -----------: %3u / %3u points \n", SPoint->SFtot, SPoint->SFnum);
		fprintf(stderr,"----------------------------------------------------------------------------------\n" );
		fprintf(stderr,"   STOP it !!!!!\n\n");
	}
	return iignoredCnt;
}




int ShiftData_LastSorting(short aiPATs05, int avgTime, short iSBchoicePnt, unsigned int ignoredCnt, short DelYesNo, unsigned int iSBchk)
{
	//sqdflt_type sq4[2];  /* 0:SS, 1:SB, 2:MaxNt, 3:MaxNe, 4:g_Max, 5:g_min, 6:SP */
	sqdAps_type sq4[2];  /* 0:SS, 1:SB, 2:MaxNt, 3:MaxNe, 4:g_Max, 5:g_min, 6:SP */
	
	int  ierr = -1;
	char shi_inp[MAX_CHARS*LENGTH_OF_FILENAME+1];
	char shi_out[MAX_CHARS*LENGTH_OF_FILENAME+1];

	/* line inputted from file */
	char QualData[QUAL_DATA_MAX_SIZE]; 
	int result;
	unsigned long long RecordCnt = 0ULL;
	unsigned long long iNGcount = 0ULL;
	unsigned long long iOKcount = 0ULL;
	short is2File = 0;
	short icurGear = 99; // -1;
	short icurMaxGear = 0;
	short itgtGear = 99; // -1;
	short itgtMaxGear = 0; // -1;
	short iShiftNu = -1;
	short iAPSminIdx = 99; 
	short iAPSMAXIdx = 0;
	short iAPSidx = 0;
	short iOnce = 1;

	unsigned int iSScount = 0;
	unsigned int iSBcount = 0;
	unsigned int iSPcount = 0;
	unsigned int iSFcount = 0;

	unsigned int iNtMxcount = 0;
	unsigned int iNtmncount = 0;

	unsigned int iGMxcount = 0;
	unsigned int iGmncount = 0;
	
	unsigned int iSScountot = 0;
	unsigned int iSBcountot = 0;
	unsigned int iSPcountot = 0;
	unsigned int iSFcountot = 0;
	unsigned int ireSScnt = 1;
	unsigned int ireSPcnt = 1;
	unsigned int llLoop = 0, llLastNum=0, ii=0;
	long avg_t1=0L, avg_t2 = 0L, avg_t3 = 0L;
	long dif_t1=0L, dif_t2 = 0L, dif_t3 = 0L;
	long dif_nt=0, dif_gx=0;
	long avg_nt=0, avg_gx=0;

	unsigned int iNtMxcountot = 0;
	unsigned int iNtmncountot = 0;
	unsigned int iGMxcountot  = 0;
	unsigned int iGmncountot  = 0;
	short isNaming = 0;
	unsigned int ignoredCount = 0;


	AllFilesClosed();


	strcpy(shi_inp, shift_file);
	strcpy(shi_out, shift_file);
	if( (aiPATs05>=0 && aiPATs05<MODE_ID_NUMS-1) && (strlen(shi_out)>0) )
	{
		isNaming = 0;
		for(ii=strlen(shi_out)-1; ii>0; ii--)
		{
			if( shi_out[ii]=='.' ) 
			{
				isNaming = 1;

				shi_out[ii+1] = '\0';				
				strcat(shi_out, FILTER_EXT_GIL );
				break;
			}
		}

		if( 0==isNaming )
		{
			strcat(shi_out, ".");
			strcat(shi_out, FILTER_EXT_GIL);
		}
	}



	/* ===================================================================================== */
	// read file OK
	if( NULL == (inpfile = fopen( shi_inp, "rb")) ) 
	{
		// FAIL
		fprintf(stderr,"\r\n++ERROR++[%s]:Can not read file (%s) \n\n", __FUNCTION__, shi_inp );
		AllFilesClosed();
		exit(0);
	}
	// Write file OK
	if( NULL == (outfile = fopen( shi_out, "wb")) )	
	{
		// FAIL
		fprintf(stderr,"\r\n++ERROR++[%s]:Can not create file (%s) \n\n", __FUNCTION__, shi_out );
		AllFilesClosed();
		exit(0);
	}	
	/* ===================================================================================== */



	/* ===================================================================================== */
	/* ===================================================================================== */

	memset(QualData, 0x00, QUAL_DATA_MAX_SIZE*sizeof(char) );
	memset(sq4, 0x00, sizeof(sq4) );

	memset( gMaxTbl, 0x00, MAX_TABLE_SIZ*sizeof(tSBtimePos_type) );
	
	RecordCnt  = 0ULL;
	iNGcount   = 0ULL;
	iOKcount   = 0ULL;

	iSScount   = 0; 
	iSBcount   = 0; 
	iSPcount   = 0; 
	iSFcount   = 0; 

	iSScountot = 0;
	iSBcountot = 0;
	iSPcountot = 0;
	iSFcountot = 0;

	iNtMxcountot = 0;
	iNtmncountot = 0;
	iGMxcountot  = 0;
	iGmncountot  = 0;

	ireSScnt   = 1;
	ireSPcnt   = 1;

#if SAVEMODE
	if(outfile)
		fprintf(outfile, TITLE_TXT "\n", iSBdecision, iSBchoicePnt?"L":"F", avgTime, iJerkTimeLen);
#endif

	do
	{
		unsigned int i=0;

		/* Read a line from input file. */
		memset( QualData, 0x00, sizeof(QualData) );

		if( NULL == fgets( QualData, QUAL_DATA_MAX_SIZE, inpfile ) )
		{
			//ierr = ferror(inpfile);
			//fclose(inpfile);

			rewind(inpfile); 
			fprintf(stderr,"----------------------------------------------------------------------------------\n" );
			//fprintf(stderr,">>Shift Quality Init APS condition check!!! %s [%s] -> [%s] \r\n", arrPATs_ModeID[aiPATs05].ModeID, shi_inp, shi_out  );			
			fprintf(stderr,">>Shift Quality Init APS condition check!!! %s -> [%s] \r\n", arrPATs_ModeID[aiPATs05].ModeID, shi_out  );			
			break;
		}

		/* Remove carriage return/line feed at the end of line. */
		i = strlen(QualData);
		if(i >= QUAL_DATA_MAX_SIZE)
		{
			fprintf(stderr,"ERROR:%6llu: Not enough Buffer length-1--(%u) \r\n", RecordCnt, i );
		}

		if (--i > 0)
		{
			if (QualData[i] == '\n') QualData[i] = '\0';
			if (QualData[i] == '\r') QualData[i] = '\0'; 

			result = sscanf(QualData, RD_APS_FMT /* RD_FltFMT */,
						&sq4[0].Time01,	  &sq4[0].iPATs05,	&sq4[0].iPATs05S,    &sq4[0].curGear08,  &sq4[0].tgtGear11,  &sq4[0].VSP03, 
						&sq4[0].tqi07,	  &sq4[0].APS09,	&sq4[0].No10,        &sq4[0].ShiNew12,   &sq4[0].ShiTy12,    &sq4[0].arrGear, 
						&sq4[0].TqFr13,	  &sq4[0].ShiPh14,	&sq4[0].Ne15,        &sq4[0].Nt16,       &sq4[0].LAcc17,     &sq4[0].LPFAcc,
						&sq4[0].DiffTime, 
						&sq4[0].sTimePos, &sq4[0].sPosNum,  &sq4[0].Gavg,        &sq4[0].sTimeNtPos, &sq4[0].sTimeGpos,  &sq4[0].gearRat,   
						&sq4[0].fJerk0,   &sq4[0].fJerk1,	&sq4[0].deltams,     &sq4[0].apsIdx);


			/* === 1 STEP : record (27 items check) =============== */
			if(QUAL_APS_DATA_ITEM_NUM==result)
			{
				iOKcount ++;
			}
			else if( (QUAL_APS_DATA_ITEM_NUM!=result) && (result!=-1 && i>0) ) 
			{
				iNGcount ++;
				continue; /* reading Next item because of FAIL item */
			}
			/* === 1 STEP : record (27 items check) =============== */

		#if 0
			if( 0==strcmp( sq4[0].sTimePos, TXT_UNKNOWN) ||
				0==strcmp( sq4[0].sTimePos, TXT_UPCASE) ||
				0==strcmp( sq4[0].sTimePos, TXT_DNCASE) ) 
			{
				continue;
			}
		#endif
		
			if( 0==strcmp( sq4[0].sTimePos, TXT_SSTIME) ) /* first Start poision : SS point */
			{
				gMaxTbl[iSScountot].SSTime = (unsigned int)( (sq4[0].Time01)*1000*JERK_TIME_SCALE );
				iSScountot ++;
			}			
			else if( 0==strcmp( sq4[0].sTimePos, TXT_SBTIME) ) 
			{
				gMaxTbl[iSScountot-1].SBTime = (unsigned int)( (sq4[0].Time01)*1000*JERK_TIME_SCALE );
				iSBcountot ++;
			}
			else if( 0==strcmp( sq4[0].sTimePos, TXT_SPTIME) ) 
			{
				gMaxTbl[iSScountot-1].SPTime = (unsigned int)( (sq4[0].Time01)*1000*JERK_TIME_SCALE );
				iSPcountot ++;
			}
			else if( 0==strncmp( sq4[0].sTimePos, TXT_SFTIME, 4) ) 
			{
				gMaxTbl[iSScountot-1].SFTime = (unsigned int)( (sq4[0].Time01)*1000*JERK_TIME_SCALE );
				iSFcountot ++;
			}

			if( 0==strcmp( sq4[0].sTimeNtPos, TXT_NtMaxTIME) ) /* Upshift case : Nt Max point */
			{
				gMaxTbl[iSScountot-1].NtMaxTime = (unsigned int)( (sq4[0].Time01)*1000*JERK_TIME_SCALE );
				//fprintf(stderr,"%2d -> NtMax = %10ld ", iNtMxcountot, gMaxTbl[iNtMxcountot].NtMaxTime/JERK_TIME_SCALE );
				iNtMxcountot ++;
			}
			else if( 0==strcmp( sq4[0].sTimeNtPos, TXT_NtminTIME) ) /* Upshift case : Nt min final point */
			{
				gMaxTbl[iSScountot-1].NtminTime = (unsigned int)( (sq4[0].Time01)*1000*JERK_TIME_SCALE );
				//fprintf(stderr,"%2d -> Ntmin = %10ld  %10d \n", iNtmncountot, gMaxTbl[iNtmncountot].NtminTime/JERK_TIME_SCALE, (gMaxTbl[iNtmncountot].NtMaxTime-gMaxTbl[iNtmncountot].NtminTime)/JERK_TIME_SCALE );
				iNtmncountot ++;
			}

			if( 0==strcmp( sq4[0].sTimeGpos, TXT_gMAX) ) /* Upshift case : G Max point */
			{
				gMaxTbl[iSScountot-1].gMx1Begin = (unsigned int)( (sq4[0].Time01)*1000*JERK_TIME_SCALE );
				//fprintf(stderr,"%2d -> gXMax = %10ld ", iGMxcountot, gMaxTbl[iGMxcountot].gMx1Begin/JERK_TIME_SCALE );
				iGMxcountot ++;
			}
			else if( 0==strcmp( sq4[0].sTimeGpos, TXT_gmin) ) /* Upshift case : G min final point */
			{
				gMaxTbl[iSScountot-1].gmn1End = (unsigned int)( (sq4[0].Time01)*1000*JERK_TIME_SCALE );
				//fprintf(stderr,"%2d -> gxmin = %10ld ", iGmncountot, gMaxTbl[iGmncountot].gmn1End/JERK_TIME_SCALE );
				iGmncountot ++;
			}


			/* min value saved !! -> Start point */
			if( icurGear > sq4[0].curGear08 ) icurGear = sq4[0].curGear08;
			if( icurMaxGear < sq4[0].curGear08 ) icurMaxGear = sq4[0].curGear08;

			if( itgtGear > sq4[0].tgtGear11 ) itgtGear = sq4[0].tgtGear11;
			if( itgtMaxGear < sq4[0].tgtGear11 ) itgtMaxGear = sq4[0].tgtGear11;

			if( iShiftNu > sq4[0].ShiNew12 ) iShiftNu = sq4[0].ShiNew12;

			if( iAPSminIdx > sq4[0].apsIdx ) iAPSminIdx = sq4[0].apsIdx;
			if( iAPSMAXIdx < sq4[0].apsIdx ) iAPSMAXIdx = sq4[0].apsIdx;

		}
	}
	while (!feof (inpfile));

	//fprintf(stderr,"  icurGear(%d), itgtGear(%d), iShiftNu(%d) iSPcount(%d) \r\n", icurGear, itgtGear, iShiftNu, iSPcount  );
	
	// ==================================================

	ireSScnt   = 1;
	ireSPcnt   = 1;
	iOnce = 1;


	rewind(inpfile); 

	// APS start index
	iAPSidx = iAPSminIdx;


	if( icurGear+1 == itgtGear )
		fprintf(stderr, "  %s UP shift Start ... curGear(%d), tgtGear(%d) \n", arrPATs_ModeID[aiPATs05].ModeID, icurGear, itgtGear );
	else if( icurGear == itgtGear+1 )
		fprintf(stderr, "  %s DOWN shift Start ... curGear(%d), tgtGear(%d) \n", arrPATs_ModeID[aiPATs05].ModeID, icurGear, itgtGear );
	else if( icurGear > itgtGear+1 )
		fprintf(stderr, "  %s SKIPDOWN shift Start ... curGear(%d), tgtGear(%d) \n", arrPATs_ModeID[aiPATs05].ModeID, icurGear, itgtGear );
	else
		fprintf(stderr, "  %s STATIC shift Start ... curGear(%d), tgtGear(%d) \n", arrPATs_ModeID[aiPATs05].ModeID, icurGear, itgtGear );
		

	// ==================================================

	//llLastNum = SPcnt; // - ignoredCnt;
	//fprintf(stderr," llLastNum (%d) = SPcnt (%d) - ignoredCnt (%d) \n", llLastNum, SPcnt, ignoredCnt );


	//fprintf(stderr, " iAPSmin/Max(%d/%d) = cur(%d:<%d>:%d) tgt(%d:%d) iOKcount(%d)/%d \n", iAPSminIdx, iAPSMAXIdx, icurGear, icurMaxGear, sq4[0].curGear08, itgtGear, sq4[0].tgtGear11, iOKcount, SPcnt  );

	//if( TYPE_MAX_NT == minMaxType )
	{
		/* ======================================================= */
		//for(llLoop=1; llLoop<=llLastNum; llLoop++)
		for(;;)
		{
			RecordCnt  = 0ULL;
			iNGcount   = 0ULL;
			iOKcount   = 0ULL;
			
			iSScount   = 0U; 
			iSBcount   = 0U; 
			iSPcount   = 0U;
			iSFcount   = 0U;

			iNtMxcount = 0;
			iNtmncount = 0;
			
			iGMxcount = 0;
			iGmncount = 0;


			//fprintf(stderr," cg =%d (%d), tg: %d (%d),  iAPSidx %d > iAPSMAXIdx %d \n", icurGear, icurMaxGear, itgtGear, itgtMaxGear, iAPSidx , iAPSMAXIdx );

			if( icurGear >= icurMaxGear ) break;

			do
			{
				unsigned int i=0;

				/* Read a line from input file. */
				memset( QualData, 0x00, sizeof(QualData) );

				if( NULL == fgets( QualData, QUAL_DATA_MAX_SIZE, inpfile ) )
				{
					ierr = ferror(inpfile);
					//fclose(inpfile);
					//fprintf(stderr,"----------------------------------------------------------------------------------\n" );

					if( iAPSidx > iAPSMAXIdx )
					{
						// Next Gear
						icurGear ++;
						itgtGear ++;
						// APS Table minimum Index 
						iAPSidx = iAPSminIdx;
					}
					else
					{
						iAPSidx ++;
					}

					rewind(inpfile);

					//fprintf(stderr,">>Shift Sorting ~~~~ curGear(%d), tgtGear(%d), APS[%2d:%6.1lf] \r\n", icurGear, itgtGear, iAPSidx, ApsTble[iAPSidx] );
					break;
				}

				RecordCnt++;


				/* Remove carriage return/line feed at the end of line. */
				i = strlen(QualData);
				
				if(i >= QUAL_DATA_MAX_SIZE)
				{
					fprintf(stderr,"ERROR:%6llu: Not enough Buffer length---(%u) \r\n", RecordCnt, i );
				}

				if (--i > 0)
				{
					if (QualData[i] == '\n') QualData[i] = '\0';
					if (QualData[i] == '\r') QualData[i] = '\0'; 

					result = sscanf(QualData, RD_APS_FMT /* RD_FltFMT */,
								&sq4[0].Time01,   &sq4[0].iPATs05,	&sq4[0].iPATs05S,	 &sq4[0].curGear08,  &sq4[0].tgtGear11, &sq4[0].VSP03, 
								&sq4[0].tqi07,	  &sq4[0].APS09,	&sq4[0].No10,		 &sq4[0].ShiNew12,	 &sq4[0].ShiTy12,	&sq4[0].arrGear, 
								&sq4[0].TqFr13,   &sq4[0].ShiPh14,	&sq4[0].Ne15,		 &sq4[0].Nt16,		 &sq4[0].LAcc17,	&sq4[0].LPFAcc,
								&sq4[0].DiffTime, 
								&sq4[0].sTimePos, &sq4[0].sPosNum,	&sq4[0].Gavg,        &sq4[0].sTimeNtPos, &sq4[0].sTimeGpos, &sq4[0].gearRat,	
								&sq4[0].fJerk0,   &sq4[0].fJerk1,   &sq4[0].deltams,     &sq4[0].apsIdx);


					/* === 1 STEP : record (27 items check) =============== */
					if(QUAL_APS_DATA_ITEM_NUM==result)
					{
						iOKcount ++;
					}
					else if( (QUAL_APS_DATA_ITEM_NUM!=result) && (result!=-1 && i>0) ) 
					{
						iNGcount ++;
						continue; /* reading Next item because of FAIL item */
					}
					/* === 1 STEP : record (27 items check) =============== */


			#if 0
					if( 0==strcmp( sq4[0].sTimePos, TXT_UNKNOWN) ||
						0==strcmp( sq4[0].sTimePos, TXT_UPCASE) ||
						0==strcmp( sq4[0].sTimePos, TXT_DNCASE) ) 
					{
						continue;
					}
			#endif

					is2File = 0;
					if( 0==strcmp( sq4[0].sTimePos, TXT_SSTIME) ) /* SS point */
					{
						iSScount ++;
						is2File = 1;
					}
					else if( 0==strncmp( sq4[0].sTimePos, TXT_SBTIME, 4) ) /* SP point */
					{
						iSBcount ++;
						is2File = 1;
					}
					else if( 0==strncmp( sq4[0].sTimePos, TXT_SPTIME, 4) ) /* SP point */
					{
						iSPcount ++;
						is2File = 1;
					}
					else if( 0==strncmp( sq4[0].sTimePos, TXT_SFTIME, 4) ) /* SF point */
					{
						iSFcount ++;
						is2File = 2;
					}
					else if( 0==strcmp( sq4[0].sTimeNtPos, TXT_NtMaxTIME) ) /* Upshift case : Nt Max point */
					{
						iNtMxcount ++;
						is2File = 1;
					}
					else if( 0==strcmp( sq4[0].sTimeNtPos, TXT_NtminTIME) ) /* Upshift case : Nt min final point */
					{
						iNtmncount ++;
						is2File = 1;
					}
					else if( 0==strcmp( sq4[0].sTimeGpos, TXT_gMAX) ) /* Upshift case : G Max point */
					{
						iGMxcount ++;
						is2File = 1;
					}
					else if( 0==strcmp( sq4[0].sTimeGpos, TXT_gmin) ) /* Upshift case : G min final point */
					{
						iGmncount ++;
						is2File = 1;
					}



					if( ( (iAPSidx == sq4[0].apsIdx) && (icurGear==sq4[0].curGear08) && (itgtGear==sq4[0].tgtGear11) ) ||
						( (iAPSidx == sq4[0].apsIdx) && (icurGear+1==sq4[0].curGear08) && (itgtGear==sq4[0].tgtGear11)  ) )
					{
						//fprintf(stderr, " iAPSindex = cur(%d:%d) tgt(%d:%d) apsIdx %d \n", icurGear, sq4[0].curGear08, itgtGear, sq4[0].tgtGear11, sq4[0].apsIdx );
					#if SAVEMODE
						if(outfile && is2File)
						{
							fprintf(outfile, SAVE_APSFMT /* SAVE_FltFMT */,
									sq4[0].Time01,	 sq4[0].iPATs05,	sq4[0].iPATs05S,    sq4[0].curGear08,   sq4[0].tgtGear11,  sq4[0].VSP03, 
									sq4[0].tqi07,	 sq4[0].APS09,		sq4[0].No10,	    sq4[0].ShiNew12,    sq4[0].ShiTy12,    sq4[0].arrGear, 
									sq4[0].TqFr13,	 sq4[0].ShiPh14,	sq4[0].Ne15,	    sq4[0].Nt16,        sq4[0].LAcc17,	   sq4[0].LPFAcc,
									sq4[0].DiffTime, 
									sq4[0].sTimePos, sq4[0].sPosNum,    sq4[0].Gavg,        sq4[0].sTimeNtPos,  sq4[0].sTimeGpos,  sq4[0].gearRat,	   
									sq4[0].fJerk0,   sq4[0].fJerk1,     sq4[0].deltams,     sq4[0].apsIdx);
					
							if(1==is2File) fprintf(outfile, "\n");
							if(2==is2File) fprintf(outfile, "\n\n");
						}
					#endif

					}

				}

			}
			while (!feof (inpfile));

		}
	}

	if(inpfile) fclose(inpfile);
	if(outfile) fclose(outfile);


	icurGear--;
	itgtGear--;


	if( icurGear+1 == itgtGear )
		fprintf(stderr, "  %s UP shift Ended ... curGear(%d), tgtGear(%d) \n", arrPATs_ModeID[aiPATs05].ModeID, icurGear, itgtGear  );
	else if( icurGear == itgtGear+1 )
		fprintf(stderr, "  %s DOWN shift Ended ... curGear(%d), tgtGear(%d) \n", arrPATs_ModeID[aiPATs05].ModeID, icurGear, itgtGear  );
	else if( icurGear > itgtGear+1 )
		fprintf(stderr, "  %s SKIPDOWN shift Ended ... curGear(%d), tgtGear(%d) \n", arrPATs_ModeID[aiPATs05].ModeID, icurGear, itgtGear  );
	else
		fprintf(stderr, "  %s STATIC shift Ended ... curGear(%d), tgtGear(%d) \n", arrPATs_ModeID[aiPATs05].ModeID, icurGear, itgtGear  );

	fprintf(stderr,"----------------------------------------------------------------------------------\n" );
	

	avg_t1 = 0L;
	avg_t2 = 0L;
	avg_t3 = 0L;
	avg_nt = 0L;
	avg_gx = 0L;
	ignoredCount = 0;

	//fprintf(stderr, "%d %d %d \n", iSScount, iSBcount, iSPcount );
	for(ii=0; ii<iSScountot; ii++)
	{
		dif_t1 = (gMaxTbl[ii].SBTime - gMaxTbl[ii].SSTime)/JERK_TIME_SCALE ;
		dif_t2 = (gMaxTbl[ii].SPTime - gMaxTbl[ii].SBTime)/JERK_TIME_SCALE ;
		
		dif_t3 = (gMaxTbl[ii].SFTime - gMaxTbl[ii].SPTime)/JERK_TIME_SCALE ;

	#if 0
		dif_nt = AVERAGE(gMaxTbl[ii].NtMaxTime/JERK_TIME_SCALE, gMaxTbl[ii].NtminTime/JERK_TIME_SCALE);
		dif_gx = AVERAGE(gMaxTbl[ii].gMx1Begin/JERK_TIME_SCALE, gMaxTbl[ii].gmn1End/JERK_TIME_SCALE);
	#else
		dif_nt = (gMaxTbl[ii].NtMaxTime - gMaxTbl[ii].NtminTime)/JERK_TIME_SCALE ;
		dif_gx = (gMaxTbl[ii].gMx1Begin - gMaxTbl[ii].gmn1End)/JERK_TIME_SCALE ;
	#endif


		
		/* Ignored Case checking -- SS~SP durtion */
		if( (dif_t1 + dif_t2) > SS2SP_OVER_TIME )
		{
			ignoredCount ++;
			//gMaxTbl[ii].ignored = 1;
		
			/* zero Clear because of over time */
			gMaxTbl[ii].SSTime = 0UL;
			gMaxTbl[ii].SBTime = 0UL;
			gMaxTbl[ii].SPTime = 0UL;
			gMaxTbl[ii].SFTime = 0UL;
		
			gMaxTbl[ii].Gavg0 = 0.0f;
			gMaxTbl[ii].Gavg1 = 0.0f;
			gMaxTbl[ii].Gavg2 = 0.0f;
		
			gMaxTbl[ii].gMx1Begin = 0UL;
			gMaxTbl[ii].gmn2Begin = 0UL;
			gMaxTbl[ii].gMx2End   = 0UL;
			gMaxTbl[ii].gmn1End   = 0UL;
		
			gMaxTbl[ii].NtMaxTime = 0UL;
			gMaxTbl[ii].NtminTime = 0UL;
		
		}
		
		/* Ignored Case checking -- SP~SF durtion */
		else if(  dif_t3 > SP2SF_OVER_TIME )
		{
			ignoredCount ++;
			//gMaxTbl[ii].ignored = 1;
		
			/* zero Clear because of over time */
			gMaxTbl[ii].SSTime = 0UL;
			gMaxTbl[ii].SBTime = 0UL;
			gMaxTbl[ii].SPTime = 0UL;
			gMaxTbl[ii].SFTime = 0UL;
			gMaxTbl[ii].SFTime = 0UL;
		
			gMaxTbl[ii].Gavg0 = 0.0f;
			gMaxTbl[ii].Gavg1 = 0.0f;
			gMaxTbl[ii].Gavg2 = 0.0f;
			
			gMaxTbl[ii].gMx1Begin = 0UL;
			gMaxTbl[ii].gmn2Begin = 0UL;
			gMaxTbl[ii].gMx2End   = 0UL;
			gMaxTbl[ii].gmn1End   = 0UL;
			
			gMaxTbl[ii].NtMaxTime = 0UL;
			gMaxTbl[ii].NtminTime = 0UL;
		
		}
		else
		{
			avg_t1 += dif_t1;
			avg_t2 += dif_t2;
			avg_t3 += dif_t3;

			avg_nt += abs(dif_nt);
			avg_gx += abs(dif_gx);
			//fprintf(stderr, " %2d -> %10lld  %10lld  \n", ii, avg_t1, avg_t2 );
		}


	}

	if(iSScountot)
	{
		avg_t1 /= (iSScountot);
		avg_t2 /= (iSScountot);
		avg_t3 /= (iSScountot);
		avg_nt /= (iSScountot);
		avg_gx /= (iSScountot);
	}
	
	fprintf(stderr,">>Average Time after Last Sorting... SS(%d), SB(%d), SP(%d), SF(%d) Records. %s \n", 
						iSScountot, iSBcountot, iSPcountot, iSFcountot, arrPATs_ModeID[aiPATs05].ModeID );
	fprintf(stderr,"  Shift Average Time(t1:SS~SB) : %9ld msec <- 4th average time  \n", avg_t1 );
	fprintf(stderr,"  Shift Average Time(t2:SB~SP) : %9ld msec <- 4th average time  \n", avg_t2 );
	fprintf(stderr,"  Shift Average Time(t3:SP~SF) : %9ld msec <- 4th average time  \n", avg_t3 );
	fprintf(stderr,"  Shift Average Time(NtMx~min) : %9ld msec <- 4th average time  \n", avg_nt );
	fprintf(stderr,"  Shift Average Time(GMax~min) : %9ld msec <- 4th average time  \n", avg_gx );
	//fprintf(stderr,"----------------------------------------------------------------------------------\n" );


	fprintf(stderr,"----------------------------------------------------------------------------------\n" );
	fprintf(stderr,">>Shift Quality Data Sorting finished!!! %s \r\n", arrPATs_ModeID[aiPATs05].ModeID );
	//fprintf(stderr,">>Shift Quality Data Sorting finished!!! %s [%s] -> [%s] \r\n", arrPATs_ModeID[aiPATs05].ModeID, shi_inp, output  );

	fprintf(stderr,">>Final Sorted result text file: %s \n", shi_out );
	fprintf(stderr,"----------------------------------------------------------------------------------\n" );


	if( 0==iSScountot || 0==iSBcountot || 0==iSPcountot )
	{
		fprintf(stderr,"\n\n>>Quality Shift Data NOT exist!!!  \n\n\n");
		AllFilesClosed();
		tempFileDeleteIt(DelYesNo, aiPATs05, iSBchk);

		exit(0);
		return 0;
	}

	return 1;
}



int t1t2GraphAferQsort(graphData_type tblV1[][GRAPH_Y_BUF_SIZ], graphData_type tblV2[][GRAPH_Y_BUF_SIZ], int xxMax, int yyMax, short gType)
{
	int ii=0;
	int xx=0, yy=0, zlp=0;
	int xxMore = xxMax-6;
	short pCnt = 0;
	double gYMaxt1=0.0f, gYmint1 = 9999999.9f, gYavg1=0.0f;
	double gYMaxt2=0.0f, gYmint2 = 9999999.9f, gYavg2=0.0f;
	double gYstep1=0.0f, gYstep2 =0.0f;


	/* ---------------------------------------------------------- */
	/* --- t1, t2 Qsorting	------------------------------------- */
	/* ---------------------------------------------------------- */


	/* --------------------------------- */
	/* gType==0 Case : symbol print ---- */
	/* --------------------------------- */
	if(0==gType)
	{
		// ---------------------------
		// 1st line ------------------
		// ---------------------------
		yy = 1;
		if(tblV1[0][yy].value) 
			fprintf(outfile," %6s:%s", tblV1[0][yy].symbol, tblV1[0][yy].gearTxt);

		pCnt = 0;
		for(yy=2; yy<=yyMax; yy++)
		{
			for(xx=0; xx<xxMax; xx++)
			{
				if(tblV1[xx][yy].value) 
				{
					fprintf(outfile,",%3s:%s", tblV1[xx][yy].symbol, tblV1[xx][yy].gearTxt);
					break;
				}
			}
			
			if(pCnt++>=2) break;
		}

		for(ii=0; ii<(SPACE_ONE_UNIT+1)*(6-1-pCnt); ii++) fprintf(outfile," ");
		if(xxMore) for(ii=0; ii<SPACE_ONE_UNIT*xxMore; ii++) fprintf(outfile," ");

		yy = 1;
		if(tblV2[0][yy].value) 
			fprintf(outfile," %5s%6s:%s", "", tblV2[0][yy].symbol, tblV2[0][yy].gearTxt);

		pCnt = 0;
		for(yy=2; yy<=yyMax; yy++)
		{

			for(xx=0; xx<xxMax; xx++)
			{
				if(tblV2[xx][yy].value) 
				{
					fprintf(outfile,",%3s:%s", tblV2[xx][yy].symbol, tblV2[xx][yy].gearTxt);
					break;
				}
			}
			if(pCnt++>=2) break;
		}
		fprintf(outfile,"\n");

		// ---------------------------
		// 2nd line ------------------
		// ---------------------------

		if(tblV1[0][yy+1].value) 
			fprintf(outfile," %6s:%s", tblV1[0][yy+1].symbol, tblV1[0][yy+1].gearTxt);

		pCnt = 0;
		for(ii=yy+2; ii<=yyMax; ii++)
		{
			for(xx=0; xx<xxMax; xx++)
			{
				if(tblV1[xx][ii].value) 
				{
					fprintf(outfile,",%3s:%s", tblV1[xx][ii].symbol, tblV1[xx][ii].gearTxt);
					break;
				}
			}
			if(pCnt++>=2) break;
		}

		for(ii=0; ii<(SPACE_ONE_UNIT+1)*(6-1-pCnt); ii++) fprintf(outfile," ");
		if(xxMore) for(ii=0; ii<SPACE_ONE_UNIT*xxMore; ii++) fprintf(outfile," ");

		if(yyMax>6) 
			for(ii=0; ii<2; ii++) fprintf(outfile," ");

		if(tblV2[0][yy+1].value) 
			fprintf(outfile,"  %6s:%s", tblV2[0][yy+1].symbol, tblV2[0][yy+1].gearTxt);
		
		pCnt = 0;
		for(ii=yy+2; ii<=yyMax; ii++)
		{
			for(xx=0; xx<xxMax; xx++)
			{
				if(tblV2[xx][ii].value) 
				{
					fprintf(outfile,",%3s:%s", tblV2[xx][ii].symbol, tblV2[xx][ii].gearTxt);
					break;
				}
			}
			if(pCnt++>=2) break;
		}
	}

	fprintf(outfile,"\n [sec]");
	
	for(ii=0; ii<SPACE_ONE; ii++) fprintf(outfile," ");
	
	if(xxMore) for(ii=0; ii<SPACE_ONE_UNIT*xxMore; ii++) fprintf(outfile," ");
	fprintf(outfile," [sec]\n");

	/* 1st column */
	fprintf(outfile,"     |");
	for(ii=0; ii<SPACE_ONE; ii++) fprintf(outfile," ");

	if(xxMore) for(ii=0; ii<SPACE_ONE_UNIT*xxMore; ii++) fprintf(outfile," ");
	/* 2nd column */
	fprintf(outfile,"     |");
	fprintf(outfile,"\n");


	for(xx=0; xx<xxMax ; xx++)
	{
		qsort(tblV1[xx], sizeof(tblV1[xx])/sizeof(graphData_type), sizeof(graphData_type), QsortCompare); 

		qsort(tblV2[xx], sizeof(tblV2[xx])/sizeof(graphData_type), sizeof(graphData_type), QsortCompare); 
	}


	/* min and Max value */
	for(xx=0; xx<xxMax ; xx++)
	{
		for(yy=0; yy<yyMax; yy++)
		{
			if( 0==tblV1[xx][yy].value || 0==tblV2[xx][yy].value ) continue;

			tblV1[xx][0].YvalNum++;
			tblV2[xx][0].YvalNum++;

			if( gYmint1 > tblV1[xx][yy].value ) gYmint1 = tblV1[xx][yy].value ;
			if( gYmint2 > tblV2[xx][yy].value ) gYmint2 = tblV2[xx][yy].value ;

			if( gYMaxt1 <= tblV1[xx][yy].value ) gYMaxt1 = tblV1[xx][yy].value ;
			if( gYMaxt2 <= tblV2[xx][yy].value ) gYMaxt2 = tblV2[xx][yy].value ;
		}
		//fprintf(stderr, "xx:%d : ValidNum1 = %d, ValidNum2 = %d, yyMax = %d \n", xx, tblV1[xx][0].YvalNum, tblV2[xx][0].YvalNum, yyMax);
	}

	/* --- Average --- */
	gYavg1 = (gYMaxt1 + gYmint1)/2;
	for(xx=xxMax-1; xx>=0; xx--)
	{
		if( tblV1[xx][0].YvalNum>0 ) 
			gYstep1 = (gYMaxt1 - gYmint1)/tblV1[xx][0].YvalNum;

		for(yy=yyMax; yy>0; yy--)
		{
			if( tblV1[xx][0].YvalNum <= (yyMax/2) )
			{
				for(zlp=0; zlp<(yyMax/2); zlp++)
				{
					if( (int)gYavg1 > (tblV1[xx][yy+zlp].value+gYstep1) && (tblV1[xx][yy+zlp].value>0) )
					{
						memcpy( (void*)&tblV1[xx][yy+zlp+1], (void*)&tblV1[xx][yy+zlp], sizeof(graphData_type) );
						memset( (void*)&tblV1[xx][yy+zlp], 0x00, sizeof(graphData_type) );
					}
				}
			}
			else if( (int)gYavg1 > tblV1[xx][yy].value && (tblV1[xx][yy].value>0) )
			{
				memcpy( (void*)&tblV1[xx][yy+1], (void*)&tblV1[xx][yy], sizeof(graphData_type) );
				memset( (void*)&tblV1[xx][yy], 0x00, sizeof(graphData_type) );
			}
		}
	}

	gYavg2 = (gYMaxt2 + gYmint2)/2;
	for(xx=xxMax-1; xx>=0; xx--)
	{
		if( tblV2[xx][0].YvalNum>0 ) 
			gYstep2 = (gYMaxt2 - gYmint2)/tblV2[xx][0].YvalNum;

		for(yy=yyMax; yy>=0; yy--)
		{
			if( tblV2[xx][0].YvalNum <= (yyMax/2) )
			{
				for(zlp=0; zlp<(yyMax/2); zlp++)
				{
					if( (int)gYavg2 > (tblV2[xx][yy+zlp].value+gYstep2) && (tblV2[xx][yy+zlp].value>0) )
					{
						memcpy( (void*)&tblV2[xx][yy+zlp+1], (void*)&tblV2[xx][yy+zlp], sizeof(graphData_type) );
						memset( (void*)&tblV2[xx][yy+zlp], 0x00, sizeof(graphData_type) );
					}
				}
			}
			else if( (int)gYavg2 > tblV2[xx][yy].value && (tblV2[xx][yy].value>0) )
			{
				memcpy( (void*)&tblV2[xx][yy+1], (void*)&tblV2[xx][yy], sizeof(graphData_type) );
				memset( (void*)&tblV2[xx][yy], 0x00, sizeof(graphData_type) );
			}

		}
	}


	gYMaxt1 /= 1000.0f;
	gYMaxt2 /= 1000.0f;
	
	gYmint1 /= 1000.0f;
	gYmint2 /= 1000.0f;


	/* ----------- t1 & t2 Graph ------------------ */
	for(yy=0; yy<=yyMax; yy++)
	{
		if(yy>0)
		{
			fprintf(outfile,"\n");
			fprintf(outfile,"     |");
			for(ii=0; ii<SPACE_ONE; ii++) fprintf(outfile," ");

			if(xxMore) for(ii=0; ii<SPACE_ONE_UNIT*xxMore; ii++) fprintf(outfile," ");
			fprintf(outfile,"     |");
			fprintf(outfile,"\n");
		}

		/* ---------------------------------------------- */
		/* 1st Graph */
		/* ---------------------------------------------- */
		if(yy==0)
		{
			fprintf(outfile," %4.2lf|", (float)(gYMaxt1-yy)/10.0f  );
		}
		else if(yy==yyMax)
		{
			fprintf(outfile," %4.2lf|", (gYmint1)/10.0f  );
		}
		else
			fprintf(outfile," %4s|", "");
			
		for(xx=0; xx<xxMax; xx++)
		{
			if( 0 == tblV1[ xx ][ yy ].value ) 
			{
				fprintf(outfile," %5s%4s","","" ); // ".."
			}
			else
			{
				switch(gType)
				{
				case 1:
					fprintf(outfile," %5s%4s", tblV1[ xx ][ yy ].gearTxt, "" );					
					break;
				case 2:
					fprintf(outfile," %5s%-4.1lf", tblV1[ xx ][ yy ].gearTxt, (tblV1[ xx ][ yy ].value /1000.0f/10.0f) );
					break;

				case 0:
				default:
					fprintf(outfile," %5s%4s", tblV1[ xx ][ yy ].symbol, "" );					
					break;
				}
			}
		}

		/* ---------------------------------------------- */
		/* 2nd Graph */
		/* ---------------------------------------------- */
		if(yy==0)
		{
			fprintf(outfile," %4.2lf|", (float)(gYMaxt2-yy)/10.0f  );
		}
		else if(yy==yyMax)
		{
			fprintf(outfile," %4.2lf|", (gYmint2)/10.0f  );
		}
		else
			fprintf(outfile," %4s|", "");

		for(xx=0; xx<xxMax; xx++)
		{
			if( 0 == tblV2[ xx ][ yy ].value ) 
			{
				fprintf(outfile," %5s%4s","","" ); // ".."
			}
			else
			{
				switch(gType)
				{
				case 1: /* shift, for example <1-2> */
					fprintf(outfile," %5s%4s", tblV2[ xx ][ yy ].gearTxt, "" ); // 1
					break;
				case 2: /* shift and value, for example <1-2>1.2 */
					fprintf(outfile," %5s%-4.1lf", tblV2[ xx ][ yy ].gearTxt, (tblV2[ xx ][ yy ].value /1000.0f/10.0f) ); // 2
					break;
				
				case 0: /* symbol */
				default:
					fprintf(outfile," %5s%4s", tblV2[ xx ][ yy ].symbol, "" );					
					break;
				}

			}
		}
#if 0
		fprintf(outfile,"\n");
		fprintf(outfile,"	  |");
		for(ii=0; ii<70; ii++) fprintf(outfile," ");
		fprintf(outfile,"	  | \n");
#endif
	}


	fprintf(outfile,"\n");
	fprintf(outfile,"     |");
	for(ii=0; ii<SPACE_ONE; ii++) fprintf(outfile," ");

	if(xxMore) for(ii=0; ii<SPACE_ONE_UNIT*xxMore; ii++) fprintf(outfile," ");
	fprintf(outfile,"     |");
	fprintf(outfile,"\n");

	fprintf(outfile,"     |");
	for(ii=0; ii<SPACE_ONE; ii++) fprintf(outfile," ");

	if(xxMore) for(ii=0; ii<SPACE_ONE_UNIT*xxMore; ii++) fprintf(outfile," ");
	fprintf(outfile,"     |");
	fprintf(outfile,"\n");

	fprintf(outfile,"     +");
	for(ii=0; ii<SPACE_ONE; ii++) fprintf(outfile,"-");
	if(xxMore) for(ii=0; ii<(SPACE_ONE_UNIT)*xxMore; ii++) fprintf(outfile,"-");

	fprintf(outfile,"     +");
	for(ii=0; ii<SPACE_ONE; ii++) fprintf(outfile,"-");
	if(xxMore) for(ii=0; ii<(SPACE_ONE_UNIT)*xxMore; ii++) fprintf(outfile,"-");
	
	fprintf(outfile,"\n   ");
	for(xx=0; xx<APS_TABLE_NUM && (int)ApsTble[xx]; xx++)
	{
		fprintf(outfile," %6d(%%)", (int)ApsTble[ xx ] );
	}

	for(ii=0; ii<7; ii++) fprintf(outfile," ");

	for(xx=0; xx<APS_TABLE_NUM && (int)ApsTble[xx]; xx++)
	{
		fprintf(outfile," %6d(%%)", (int)ApsTble[ xx ] );
	}

	fprintf(outfile,"\n");

	return 1;
}



int JerkGraphAferQsort(graphData_type tblV1[][GRAPH_Y_BUF_SIZ], graphData_type tblV2[][GRAPH_Y_BUF_SIZ], int xxMax, int yyMax, short gType)
{
	int ii=0;
	int xx=0, yy=0, zlp=0;
	int xxMore = xxMax-6;
	short pCnt=0;

	double gYMaxt1=0.0f, gYmint1 = 9999999.9f, gYavg1=0.0f;
	double gYMaxt2=0.0f, gYmint2 = 9999999.9f, gYavg2=0.0f;
	double gYstep1=0.0f, gYstep2=0.0f;

	/* ---------------------------------------------------------- */
	/* --- Jerk1, Ne MAX Qsorting	----------------------------- */
	/* ---------------------------------------------------------- */

	/* --------------------------------- */
	/* gType==0 Case : symbol print ---- */
	/* --------------------------------- */
	if(0==gType)
	{
		// ---------------------------
		// 1st line ------------------
		// ---------------------------
		yy = 1;
		if(tblV1[0][yy].value) 
			fprintf(outfile," %6s:%s", tblV1[0][yy].symbol, tblV1[0][yy].gearTxt);

		pCnt = 0;
		for(yy=2; yy<=yyMax; yy++)
		{
			for(xx=0; xx<xxMax; xx++)
			{
				if(tblV1[xx][yy].value)
				{
					fprintf(outfile,",%3s:%s", tblV1[xx][yy].symbol, tblV1[xx][yy].gearTxt);
					break;
				}
			}
			if(pCnt++>=2) break;
		}

		for(ii=0; ii<(SPACE_ONE_UNIT+1)*(6-1-pCnt); ii++) fprintf(outfile," ");
		if(xxMore) for(ii=0; ii<SPACE_ONE_UNIT*xxMore; ii++) fprintf(outfile," ");

		yy = 1;
		if(tblV2[0][yy].value) 
			fprintf(outfile," %5s%6s:%s", "",tblV2[0][yy].symbol, tblV2[0][yy].gearTxt);

		pCnt = 0;
		for(yy=2; yy<=yyMax; yy++)
		{
			for(xx=0; xx<xxMax; xx++)
			{
				if(tblV2[xx][yy].value) 
				{
					fprintf(outfile,",%3s:%s", tblV2[xx][yy].symbol, tblV2[xx][yy].gearTxt);
					break;
				}
			}
			if(pCnt++>=2) break;
		}
		fprintf(outfile,"\n");

		// ---------------------------
		// 2nd line ------------------
		// ---------------------------
		if(tblV1[0][yy+1].value) 
			fprintf(outfile," %6s:%s", tblV1[0][yy+1].symbol, tblV1[0][yy+1].gearTxt);

		pCnt = 0;
		for(ii=yy+2; ii<=yyMax; ii++)
		{
			for(xx=0; xx<xxMax; xx++)
			{
				if(tblV1[xx][ii].value) 
				{
					fprintf(outfile,",%3s:%s", tblV1[xx][ii].symbol, tblV1[xx][ii].gearTxt);
					break;
				}
			}
			if(pCnt++>=2) break;
		}

		for(ii=0; ii<(SPACE_ONE_UNIT+1)*(6-1-pCnt); ii++) fprintf(outfile," ");
		if(xxMore) for(ii=0; ii<SPACE_ONE_UNIT*xxMore; ii++) fprintf(outfile," ");

		if(yyMax>6) 
			for(ii=0; ii<2; ii++) fprintf(outfile," ");


		if(tblV2[0][yy+1].value) 
			fprintf(outfile,"  %6s:%s", tblV2[0][yy+1].symbol, tblV2[0][yy+1].gearTxt);

		pCnt = 0;
		for(ii=yy+2; ii<=yyMax; ii++)
		{
			for(xx=0; xx<xxMax; xx++)
			{
				if(tblV2[xx][ii].value) 
				{
					fprintf(outfile,",%3s:%s", tblV2[xx][ii].symbol, tblV2[xx][ii].gearTxt);
					break;
				}
			}
			if(pCnt++>=2) break;
		}
	}

	fprintf(outfile,"\n[G/sec]");
	for(ii=0; ii<SPACE_ONE-1; ii++) fprintf(outfile," ");
	if(xxMore) for(ii=0; ii<SPACE_ONE_UNIT*xxMore; ii++) fprintf(outfile," ");
	fprintf(outfile," [rpm]\n");

	fprintf(outfile,"     |");
	for(ii=0; ii<SPACE_ONE; ii++) fprintf(outfile," ");
	if(xxMore) for(ii=0; ii<SPACE_ONE_UNIT*xxMore; ii++) fprintf(outfile," ");
	fprintf(outfile,"     |");
	fprintf(outfile,"\n");


	for(xx=0; xx<xxMax ; xx++)
	{
		qsort(tblV1[xx], sizeof(tblV1[xx])/sizeof(graphData_type), sizeof(graphData_type), QsortCompare); 

		qsort(tblV2[xx], sizeof(tblV2[xx])/sizeof(graphData_type), sizeof(graphData_type), QsortCompare); 
	}


	/* min and Max Value */
	for(xx=0; xx<xxMax ; xx++)
	{
		for(yy=0; yy<yyMax; yy++)
		{
			if( 0==tblV1[xx][yy].value || 0==tblV2[xx][yy].value ) continue;

			tblV1[xx][0].YvalNum++;
			tblV2[xx][0].YvalNum++;

			if( gYmint1 > tblV1[xx][yy].value ) gYmint1 = tblV1[xx][yy].value ;
			if( gYmint2 > tblV2[xx][yy].value ) gYmint2 = tblV2[xx][yy].value ;

			if( gYMaxt1 <= tblV1[xx][yy].value ) gYMaxt1 = tblV1[xx][yy].value ;
			if( gYMaxt2 <= tblV2[xx][yy].value ) gYMaxt2 = tblV2[xx][yy].value ;

		}
	}

	/* -- Average --- */
	gYavg1 = (gYMaxt1 + gYmint1)/2;
	for(xx=xxMax-1; xx>=0; xx--)
	{
		if( tblV1[xx][0].YvalNum>0 ) 
			gYstep1 = (gYMaxt1 - gYmint1)/tblV1[xx][0].YvalNum;
			
		for(yy=yyMax; yy>=0; yy--)
		{
			if( tblV1[xx][0].YvalNum <= (yyMax/2) )
			{
				for(zlp=0; zlp<=(yyMax/2); zlp++)
				{
					if( (int)gYavg1 > (tblV1[xx][yy+zlp].value+(int)gYstep1) && (tblV1[xx][yy+zlp].value>0) )
					{
						memcpy( (void*)&tblV1[xx][yy+zlp+1], (void*)&tblV1[xx][yy+zlp], sizeof(graphData_type) );
						memset( (void*)&tblV1[xx][yy+zlp], 0x00, sizeof(graphData_type) );
					}
					else if( (int)gYavg1 > (tblV1[xx][yy+zlp].value+(int)(gYstep1/2) ) && (tblV1[xx][yy+zlp].value>0) )
					{
						memcpy( (void*)&tblV1[xx][yy+zlp+1], (void*)&tblV1[xx][yy+zlp], sizeof(graphData_type) );
						memset( (void*)&tblV1[xx][yy+zlp], 0x00, sizeof(graphData_type) );
					}
				}
			}
			else if( (int)gYavg1 > tblV1[xx][yy].value && (tblV1[xx][yy].value>0) )
			{
				memcpy( (void*)&tblV1[xx][yy+1], (void*)&tblV1[xx][yy], sizeof(graphData_type) );
				memset( (void*)&tblV1[xx][yy], 0x00, sizeof(graphData_type) );
			}
		}
	}
	

	gYavg2 = (gYMaxt2 + gYmint2)/2;
	for(xx=xxMax-1; xx>=0; xx--)
	{
		if( tblV2[xx][0].YvalNum>0 ) 
			gYstep2 = (gYMaxt2 - gYmint2)/tblV2[xx][0].YvalNum;

		for(yy=yyMax; yy>=0; yy--)
		{
			if( tblV2[xx][0].YvalNum <= (yyMax/2) )
			{
				for(zlp=0; zlp<(yyMax/2); zlp++)
				{
					if( (int)gYavg2 > (tblV2[xx][yy+zlp].value+(int)(gYstep2/2)) && (tblV2[xx][yy+zlp].value>0) )
					{
						memcpy( (void*)&tblV2[xx][yy+zlp+1], (void*)&tblV2[xx][yy+zlp], sizeof(graphData_type) );
						memset( (void*)&tblV2[xx][yy+zlp], 0x00, sizeof(graphData_type) );
					}
					else if( (int)gYavg2 > (tblV2[xx][yy+zlp].value) && (tblV2[xx][yy+zlp].value>0) )
					{
						memcpy( (void*)&tblV2[xx][yy+zlp+1], (void*)&tblV2[xx][yy+zlp], sizeof(graphData_type) );
						memset( (void*)&tblV2[xx][yy+zlp], 0x00, sizeof(graphData_type) );
					}
				}
			}
			else if( (int)gYavg2 > tblV2[xx][yy].value && (tblV2[xx][yy].value>0) )
			{
				memcpy( (void*)&tblV2[xx][yy+1], (void*)&tblV2[xx][yy], sizeof(graphData_type) );
				memset( (void*)&tblV2[xx][yy], 0x00, sizeof(graphData_type) );
			}

		}
	}

#if 0
	fprintf(stderr,"\n----------------- %lf  %lf  %lf \n", gYavg2, gYMaxt2 , gYmint2 );
	for(xx=0; xx<=xxMax ; xx+=5)
	{
		for(yy=0; yy<=yyMax; yy++)
		{
			fprintf(stderr," %d , %d -> %6d  = [ %d , %d -> %6d ] = %d , %d -> %6d = [ %d , %d -> %6d ] = %d , %d -> %6d = [ %d , %d -> %6d ] \n", 
				xx, yy, tblV2[xx][yy].value, xx+1, yy, tblV2[xx+1][yy].value, xx+2, yy, tblV2[xx+2][yy].value, 
				xx+3, yy, tblV2[xx+3][yy].value, xx+4, yy, tblV2[xx+4][yy].value, xx+5, yy, tblV2[xx+5][yy].value );

		}
	}
#endif


	gYMaxt1 /= 100.0f;
	gYMaxt2 /= 100.0f;

	gYmint1 /= 100.0f;
	gYmint2 /= 100.0f;


	/* ----------- Jerk1 & Ne MAX Graph ------------------ */
	for(yy=0; yy<=yyMax; yy++)
	{
		if(yy>0)
		{
			fprintf(outfile,"\n");
			fprintf(outfile,"     |");

			// ----------------------------------
			// Jerk1, Left Table Value
			// ----------------------------------
			if(2==gType) /* shift and value, for example <1-2>1.2 */
			{
				for(xx=0; xx<xxMax; xx++)
				{
					if( 0 == tblV1[ xx ][ yy-1 ].value ) 
					{
						if( (0==yy || yy==yyMax-1) && xx==xxMax-1 )
							fprintf(outfile," %5s%4s"," ","");
						else
							fprintf(outfile," %5s%4s"," ","");
					}
					else
					{
						fprintf(outfile," %-9.2lf", (tblV1[ xx ][ yy-1 ].value /100.0f) );
					}
				}
				fprintf(outfile,"     |");
			}
			else
			{
				for(ii=0; ii<SPACE_ONE; ii++) fprintf(outfile," ");
				if(xxMore) for(ii=0; ii<SPACE_ONE_UNIT*xxMore; ii++) fprintf(outfile," ");
				fprintf(outfile,"     |");
			}

			// ----------------------------------
			// Ne MAX, Right Table Value
			// ----------------------------------
			if(2==gType) /* shift and value, for example <1-2>1.2 */
			{
				for(xx=0; xx<xxMax; xx++)
				{
					if( 0 == tblV2[ xx ][ yy-1 ].value ) 
					{
						if( (0==yy || yy==yyMax-1) && xx==xxMax-1 )
							fprintf(outfile," %5s%4s"," ","");
						else
							fprintf(outfile," %5s%4s"," ","");
					}
					else
					{
						fprintf(outfile," %-9.2lf", (tblV2[ xx ][ yy-1 ].value /100.0f) );
					}
				}
			}
			fprintf(outfile,"\n");
		}

		if(yy==0)
		{
			fprintf(outfile," %4.1lf|", (float)(gYMaxt1-yy)  );
		}
		else if(yy==yyMax)
		{
			fprintf(outfile," %4.1lf|", (gYmint1)  );
		}
		else
			fprintf(outfile," %4s|", "");
			
		for(xx=0; xx<xxMax; xx++)
		{
			if( 0 == tblV1[ xx ][ yy ].value ) 
			{
				if( (0==yy || yy==yyMax) && xx==xxMax-1 )
				{
					fprintf(outfile," %5s%2s","","");  /* */
					//fprintf(outfile," %5s%2s","..","");  /* */
				}
				else
				{
					fprintf(outfile," %5s%4s","","");
					//fprintf(outfile," %5s%4s","..","");
				}
			}
			else
			{
				if( (0==yy || yy==yyMax) && xx==xxMax-1 )
				{
					if(gType) fprintf(outfile," %5s%2s", tblV1[ xx ][ yy ].gearTxt, "" ); /* shift, for example <1-2> */
					if(0==gType) fprintf(outfile," %4s%3s", tblV1[ xx ][ yy ].symbol, "" ); /* 0 means symbol only */
				}
				else
				{
					if(gType) fprintf(outfile," %5s%4s", tblV1[ xx ][ yy ].gearTxt, "" ); /* shift, for example <1-2> */
					if(0==gType) fprintf(outfile," %4s%5s", tblV1[ xx ][ yy ].symbol, "" ); /* 0 means symbol only */
				}
			}
		}


		if(yy==0)
		{
			fprintf(outfile," %4.1lf|", (float)(gYMaxt2-yy)  );
		}
		else if(yy==yyMax)
		{
			fprintf(outfile," %4.1lf|", (gYmint2)  );
		}
		else
		{
			fprintf(outfile," %4s|", "");
		}

		for(xx=0; xx<xxMax; xx++)
		{
			if( 0 == tblV2[ xx ][ yy ].value ) 
			{
				if( (0==yy || yy==yyMax) && xx==xxMax-1 )
				{
					fprintf(outfile," %5s%2s"," ","");
					//fprintf(outfile," %5s%2s","..","");
				}
				else
				{
					fprintf(outfile," %5s%4s"," ","");
					//fprintf(outfile," %5s%4s","..","");
				}
			}
			else
			{
				if( (0==yy || yy==yyMax) && xx==xxMax-1 )
				{
					if(gType) fprintf(outfile," %5s%2s", tblV2[ xx ][ yy ].gearTxt, "" );
					if(0==gType) fprintf(outfile," %4s%3s", tblV2[ xx ][ yy ].symbol, "" );
				}
				else
				{
					if(gType) fprintf(outfile," %5s%4s", tblV2[ xx ][ yy ].gearTxt, "" );
					if(0==gType) fprintf(outfile," %4s%5s", tblV2[ xx ][ yy ].symbol, "" );
				}
				
			}
		}
	}


	if( 0==gType )
	{
		fprintf(outfile,"\n");
		fprintf(outfile,"     |");
		for(ii=0; ii<SPACE_ONE; ii++) fprintf(outfile," ");
		if(xxMore) for(ii=0; ii<SPACE_ONE_UNIT*xxMore; ii++) fprintf(outfile," ");
		fprintf(outfile,"     |");
		fprintf(outfile,"\n");
	}
	else
	{
		fprintf(outfile,"\n");
		fprintf(outfile,"     |");

		// ----------------------------------
		// Left Table Value
		// ----------------------------------
		if(2==gType)
		{
			for(xx=0; xx<xxMax; xx++)
			{
				if( 0 == tblV1[ xx ][ yy-1 ].value ) 
				{
					if( (0==yy || yy==yyMax-1) && xx==xxMax-1 )
						fprintf(outfile," %5s%4s","  ","");
					else
						fprintf(outfile," %5s%4s","  ","");
				}
				else
				{
					fprintf(outfile," %-9.2lf", (tblV1[ xx ][ yy-1 ].value /100.0f) );
				}
			}
			fprintf(outfile,"     |");
		}
		else
		{
			for(ii=0; ii<SPACE_ONE; ii++) fprintf(outfile," ");
			if(xxMore) for(ii=0; ii<SPACE_ONE_UNIT*xxMore; ii++) fprintf(outfile," ");
			fprintf(outfile,"     |");
		}

		// ----------------------------------
		// Right Table Value
		// ----------------------------------
		if(2==gType)
		{
			for(xx=0; xx<xxMax; xx++)
			{
				if( 0 == tblV2[ xx ][ yy-1 ].value ) 
				{
					if( (0==yy || yy==yyMax-1) && xx==xxMax-1 )
						fprintf(outfile," %5s%4s","  ","");
					else
						fprintf(outfile," %5s%4s","  ","");
				}
				else
				{
					fprintf(outfile," %-9.2lf", (tblV2[ xx ][ yy-1 ].value /100.0f) );
				}
			}
		}
		fprintf(outfile,"\n");
	}


	fprintf(outfile,"     |");
	for(ii=0; ii<SPACE_ONE; ii++) fprintf(outfile," ");
	if(xxMore) for(ii=0; ii<SPACE_ONE_UNIT*xxMore; ii++) fprintf(outfile," ");
	fprintf(outfile,"     |");
	fprintf(outfile,"\n");

	fprintf(outfile,"     +");
	for(ii=0; ii<SPACE_ONE; ii++) fprintf(outfile,"-");
	if(xxMore) for(ii=0; ii<SPACE_ONE_UNIT*xxMore; ii++) fprintf(outfile,"-");

	fprintf(outfile,"     +");
	for(ii=0; ii<SPACE_ONE; ii++) fprintf(outfile,"-");
	if(xxMore) for(ii=0; ii<SPACE_ONE_UNIT*xxMore; ii++) fprintf(outfile,"-");
	
	fprintf(outfile,"\n   ");
	for(xx=0; xx<APS_TABLE_NUM && (int)ApsTble[xx]; xx++)
	{
		fprintf(outfile," %6d(%%)", (int)ApsTble[ xx ] );
	}

	for(ii=0; ii<7; ii++) fprintf(outfile," ");

	for(xx=0; xx<APS_TABLE_NUM && (int)ApsTble[xx]; xx++)
	{
		fprintf(outfile," %6d(%%)", (int)ApsTble[ xx ] );
	}

	fprintf(outfile,"\n");



#if 0
{
	wchar_t  sTime[10][4];
	wcscpy(sym[0],L"\u2764");
	wcscpy(sym[1],L"\u2764");
	wcscpy(sym[2],L"\u2764");

	sprintf(sym[0], L"■"); 
	fprintf(stderr,"■★●◆▲♠♥♣ ◎※℃∇ αβγδ ◈○◇□△♡♤\n");
	fprintf(outfile,"■★●◆▲♠♥♣ ◎※℃∇ αβγδ ◈○◇□△♡♤\n");

	fprintf(outfile,"%ls \n",sym[0] );
	fprintf(stderr,"%ls \n",sym[0] );
	fprintf(stderr,"[%S [-]\n",sym[0] );


	int nLen = MultiByteToWideChar(CP_ACP, 0, sTime, lstrlen(sTime), NULL, NULL);
	// 얻어낸 길이만큼 메모리를 할당한다.
	bstr = SysAllocStringLen(NULL, nLen);
	// 이제 변환을 수행한다.
	MultiByteToWideChar(CP_ACP, 0, sTime, lstrlen(sTime), bstr, nLen);
	// 필요없어지면 제거한다.
	SysFreeString(bstr);

}
#endif

	return 1;
}



int ShiftData_Report(short aiPATs05, int avgTime, short iSBchoicePnt, short gValueDisplay)
{
	sqdAps_type sq4[MAX_RPT_BUF_SIZ];  /* 0:SS, 1:SB, 2:MaxNt, 3:MaxNe, 4:g_Max, 5:g_min, 6:SP */
	
	int  ierr = -1;

	/* line inputted from file */
	char QualData[QUAL_DATA_MAX_SIZE]; 
	int result;
	unsigned long long RecordCnt = 0ULL;
	unsigned long long iNGcount = 0ULL;
	unsigned long long iOKcount = 0ULL;
	short is2File = 0;
	unsigned int iSScountot = 0U;
	unsigned int iSBcountot = 0U;
	unsigned int iSPcountot = 0U;
	unsigned int iSFcountot = 0U;

	unsigned short idxSS = 0;
	unsigned short idxSB = 0;
	unsigned short idxSP = 0;
	unsigned short idxSF = 0;
	
	unsigned short index = 0;
	unsigned short idxJerk = 0;
	unsigned short idxgm = 0;
	unsigned short idxgX = 0;
	unsigned short idxNtMx = 0;
	unsigned short idxNtmn = 0;

	int diffTimegX = 0;
	int diffTimeNtMx = 0;
	int diffTimeNtmn = 0;

	int iSBTime = 0;
	int iSPTime = 0;
	int iSFTime = 0;

				
	short icurGear = 0;
	short itgtGear = 0;
	short ii = 0;
	char sToday[50];
	char currPath[PATH_MAX];
		
	graphData_type tblT1[APS_TABLE_NUM][GRAPH_Y_BUF_SIZ]; /* t1 time table : SS~SB time in msec*10 unit */
	graphData_type tblT2[APS_TABLE_NUM][GRAPH_Y_BUF_SIZ]; /* t2 time table : SB~SP time in msec*10 unit */
	graphData_type tblJerk1[APS_TABLE_NUM][GRAPH_Y_BUF_SIZ]; /* Jerk1 */
	graphData_type tblNeMX[APS_TABLE_NUM][GRAPH_Y_BUF_SIZ]; /* Ne Max */
	graphData_type tblNtMX[APS_TABLE_NUM][GRAPH_Y_BUF_SIZ]; /* Nt Max */

	int xx=0, xxMax, yy=0, yyMax;
	int xxMore=0;
	double gYMaxt1=0.0f, gYmint1 = 9999999.9f;
	double gYMaxt2=0.0f, gYmint2 = 9999999.9f;
	short isNaming = 0;
	char shi_inp[MAX_CHARS*LENGTH_OF_FILENAME+1];
	char shi_out[MAX_CHARS*LENGTH_OF_FILENAME+1];
	double G_dif = 0.0f, Nt_dif = 0.0f;

	AllFilesClosed();
	mb2wc();


	strcpy(shi_inp, shift_file);
	strcpy(shi_out, shift_file);
	if( (aiPATs05>=0 && aiPATs05<MODE_ID_NUMS-1) && (strlen(shi_out)>0) )
	{
		isNaming = 0;
		for(ii=strlen(shi_out)-1; ii>0; ii--)
		{
			if( shi_out[ii]=='.' ) 
			{
				isNaming = 1;

				shi_inp[ii+1] = '\0';
				strcat(shi_inp, FILTER_EXT_GIL );

				shi_out[ii+1] = '\0';
				strcat(shi_out, FILTER_EXT_RPT );
				break;
			}
		}

		if( 0==isNaming )
		{
			strcat(shi_inp, ".");
			strcat(shi_inp, FILTER_EXT_GIL);

			strcat(shi_out, ".");
			strcat(shi_out, FILTER_EXT_RPT);
		}
	}



	/* ===================================================================================== */
	// read file OK
	if( NULL == (inpfile = fopen( shi_inp, "rb")) ) 
	{
		// FAIL
		fprintf(stderr,"\r\n++ERROR++[%s]:Can not read file (%s) \n\n", __FUNCTION__, shi_inp );
		AllFilesClosed();
		exit(0);
	}
	// Write file OK
#if WIDE_CHAR_SYMBOL
	if( NULL == (outfile = fopen( shi_out, "wb,ccs=UTF-8")) )	
#else
	if( NULL == (outfile = fopen( shi_out, "wb")) )	
#endif
	{
		// FAIL
		fprintf(stderr,"\r\n++ERROR++[%s]:Can not create file (%s) \n\n", __FUNCTION__, shi_out );
		AllFilesClosed();
		exit(0);
	}	
	/* ===================================================================================== */

	//_setmode(_fileno(outfile), _O_U8TEXT); /* UTF-8 인코딩으로 설정*/
	//_setmode(_fileno(outfile), _O_BINARY); /*  */


	/* ===================================================================================== */
	/* ===================================================================================== */

	memset(QualData, 0x00, QUAL_DATA_MAX_SIZE*sizeof(char) );
	memset(sq4, 0x00, MAX_RPT_BUF_SIZ*sizeof(sqdAps_type) );

	memset(tblT1, 0x00, APS_TABLE_NUM*GRAPH_Y_BUF_SIZ*sizeof(graphData_type) );
	memset(tblT2, 0x00, APS_TABLE_NUM*GRAPH_Y_BUF_SIZ*sizeof(graphData_type) );

	memset(tblJerk1, 0x00, APS_TABLE_NUM*GRAPH_Y_BUF_SIZ*sizeof(graphData_type) );
	memset(tblNeMX, 0x00, APS_TABLE_NUM*GRAPH_Y_BUF_SIZ*sizeof(graphData_type) );
	memset(tblNtMX, 0x00, APS_TABLE_NUM*GRAPH_Y_BUF_SIZ*sizeof(graphData_type) );
	memset(currPath, 0x00, PATH_MAX*sizeof(char) );
	

	RecordCnt  = 0ULL;
	iNGcount   = 0ULL;
	iOKcount   = 0ULL;


	/* ----------------------------------------------------------------------------- */
	/* DATE & TIME at operating time ----------------------------------------------- */
	/* ----------------------------------------------------------------------------- */
	{
	time_t	currTime;
	struct tm *locTime;


	time( &currTime );
	locTime = localtime( &currTime ); /* time_t	형식으로 변환합니다. */
	/* --------------------------
	locTime->tm_year+1900;
	locTime->tm_mon+1;
	locTime->tm_mday;
	locTime->tm_hour;
	locTime->tm_min;
	locTime->tm_sec;
	locTime->tm_wday;
	------------------------------ */

	memset(sToday, 0x00, sizeof(sToday) );

	sprintf(sToday, "%4d/%02d/%02d(%s), %02d:%02d:%02d", 
			locTime->tm_year+1900, locTime->tm_mon+1, locTime->tm_mday, WeekTXT[ locTime->tm_wday ], 
			locTime->tm_hour+9, locTime->tm_min, locTime->tm_sec );
	}
	/* ----------------------------------------------------------------------------- */
	/* ----------------------------------------------------------------------------- */



#if SAVEMODE
	if(outfile)
	{
		if ( getcwd(currPath, PATH_MAX) == NULL ) 
		{
			fprintf(stderr, "[++ERROR++] Can not check current directory!! \n\n") ;
		} 

		fprintf(outfile," Shift Quality Report %s - %s by GIL&S \n", arrPATs_ModeID[aiPATs05].ModeID, sToday );			
		fprintf(outfile," FileName: %s\\%s \n", currPath, shift_in );			

		for(ii=0; ii<SEPER_DASH; ii++) fprintf(outfile, "-");
		fprintf(outfile, "\n");

	#ifndef LOW_PASS_FILTER_GACC
		fprintf(outfile,"    Shift     APS_Power(%%)   ShiftTime(sec)              Accel(G)           Jerk(G/sec)%d(%s)_%dmsec              rpm(SB-/SP+%dmsec)  \n", iSBdecision, iSBchoicePnt?"L":"F", iJerkTimeLen, iNtTimeLen);
	#else
		fprintf(outfile,"    Shift     APS_Power(%%)   ShiftTime(sec)            LPF-Accel(G)         Jerk(G/sec)%d(%s)_%dmsec              rpm(SB-/SP+%dmsec)  \n", iSBdecision, iSBchoicePnt?"L":"F", iJerkTimeLen, iNtTimeLen);
	#endif
	
		fprintf(outfile, REPORT_TXT "\n");
	}
#endif



	index = 0;

	do
	{
		unsigned int i=0;

		/* Read a line from input file. */
		memset( QualData, 0x00, sizeof(QualData) );

		if( NULL == fgets( QualData, QUAL_DATA_MAX_SIZE, inpfile ) )
		{
			//ierr = ferror(inpfile);
			//fclose(inpfile);

			rewind(inpfile); 
			
			fprintf(stderr,"----------------------------------------------------------------------------------\n" );
			//fprintf(stderr,">>Shift Quality Data final Report!!! %s [%s] -> [%s] \r\n", arrPATs_ModeID[aiPATs05].ModeID, shi_inp, shi_out  );			
			fprintf(stderr,">>Shift Quality Data final Report!!! %s -> [%s] \r\n", arrPATs_ModeID[aiPATs05].ModeID, shi_out  );			
			break;
		}

		/* Remove carriage return/line feed at the end of line. */
		i = strlen(QualData);
		if(i >= QUAL_DATA_MAX_SIZE)
		{
			fprintf(stderr,"ERROR:%6llu: Not enough Buffer length-11--(%u) \r\n", RecordCnt, i );
		}

		if (--i > 0)
		{
			if (QualData[i] == '\n') QualData[i] = '\0';
			if (QualData[i] == '\r') QualData[i] = '\0'; 

			result = sscanf(QualData, RD_APS_FMT /* RD_FltFMT */,
						&sq4[index].Time01,	  &sq4[index].iPATs05,	&sq4[index].iPATs05S,    &sq4[index].curGear08,  &sq4[index].tgtGear11, &sq4[index].VSP03, 
						&sq4[index].tqi07,	  &sq4[index].APS09,	&sq4[index].No10,        &sq4[index].ShiNew12,   &sq4[index].ShiTy12,   &sq4[index].arrGear, 
						&sq4[index].TqFr13,	  &sq4[index].ShiPh14,	&sq4[index].Ne15,        &sq4[index].Nt16,       &sq4[index].LAcc17,    &sq4[index].LPFAcc,
						&sq4[index].DiffTime, 
						&sq4[index].sTimePos, &sq4[index].sPosNum,  &sq4[index].Gavg,        &sq4[index].sTimeNtPos, &sq4[index].sTimeGpos, &sq4[index].gearRat,   
						&sq4[index].fJerk0,   &sq4[index].fJerk1,   &sq4[index].deltams,     &sq4[index].apsIdx);


			/* === 1 STEP : record (28 items check) =============== */
			if(QUAL_APS_DATA_ITEM_NUM==result)
			{
				iOKcount ++;
			}
			else if( (QUAL_APS_DATA_ITEM_NUM!=result) && (result!=-1 && i>0) ) 
			{
				iNGcount ++;
				continue; /* reading Next item because of FAIL item */
			}
			/* === 1 STEP : record (28 items check) =============== */


		#if 0
			if( 0==strcmp( sq4[index].sTimePos, TXT_UNKNOWN) ||
				0==strcmp( sq4[index].sTimePos, TXT_UPCASE) ||
				0==strcmp( sq4[index].sTimePos, TXT_DNCASE) ) 
			{
				continue;
			}
		#endif


			if( itgtGear != sq4[index].tgtGear11 )
			{
				icurGear = sq4[index].curGear08;
				itgtGear = sq4[index].tgtGear11;

				if(outfile)
				{
					for(ii=0; ii<SEPER_DASH; ii++) fprintf(outfile, "-");
					fprintf(outfile, "\n");
				}

			}



			if( 0==strcmp( sq4[index].sTimePos, TXT_SSTIME) ) 
			{
				iSScountot ++;
				idxSS = index;
			}

			if( 0==strcmp( sq4[index].sTimeGpos, TXT_gmin) )
			{
				idxJerk = index;
				idxgm   = index;
			}

			if( 0==strcmp( sq4[index].sTimeGpos, TXT_gMAX) ) 
			{
				if( 0==sq4[idxJerk].deltams )
					idxJerk = index;

				idxgX = index;
				diffTimegX = round( (sq4[index].Time01)*1000*JERK_TIME_SCALE );
			}

			if( 0==strcmp( sq4[index].sTimeNtPos, TXT_NtMaxTIME) ) 
			{
				idxNtMx = index;
				diffTimeNtMx = round( (sq4[index].Time01)*1000*JERK_TIME_SCALE );
			}

			if( 0==strcmp(sq4[index].sTimePos, TXT_SBTIME) ) 
			{
				iSBcountot ++;
				idxSB = index;
				iSBTime = round( (sq4[index].Time01)*1000*JERK_TIME_SCALE );
			}

			if( 0==strcmp(sq4[index].sTimeNtPos, TXT_NtminTIME) ) 
			{
				idxNtmn = index;
				diffTimeNtmn = round( (sq4[index].Time01)*1000*JERK_TIME_SCALE );
			}

			if( 0==strcmp(sq4[index].sTimePos, TXT_SPTIME) ) 
			{
				iSPcountot ++;
				idxSP = index;
				iSPTime = round( (sq4[index].Time01)*1000*JERK_TIME_SCALE );
			}

			if( 0==strncmp( sq4[index].sTimePos, TXT_SFTIME, 4) ) 
			{
				iSFcountot ++;
				idxSF = index;

				is2File = 1;



			#if 1
				/* Invalid condition -> NOT saved!! */
				if( (sq4[idxJerk].fJerk1 <= JERK1_INVALID_VALUE1) || 
					(sq4[idxJerk].fJerk1 > JERK1_INVALID_VALUE2) || 
					(abs(sq4[idxJerk].deltams) <= JERK1_T_INVALID) )
				{
					memset( (void*)&sq4[idxJerk], 0x00, sizeof(sqdAps_type) );
					is2File = 0;
				}
			#endif



			#if SAVEMODE
				if(outfile && is2File)
				{
					fprintf(outfile, SAVE_PRT_01,
							sq4[idxSS].curGear08, sq4[idxSS].tgtGear11,	sq4[idxSS].arrGear, (int)ApsTble[ sq4[idxSS].apsIdx ], sq4[idxSS].APS09 );

					G_dif = (sq4[idxSF].Gavg - sq4[idxSP].Gavg);

				#ifndef LOW_PASS_FILTER_GACC /* 2023-02-09, Low Pass Filter for LAcc */
					fprintf(outfile, SAVE_PRT_02,
							sq4[idxSB].DiffTime,  sq4[idxSP].DiffTime, sq4[idxSF].DiffTime, sq4[idxSS].LAcc17, sq4[idxSB].LAcc17, sq4[idxSP].LAcc17, 
							G_dif );
				#else /* Low Pass Filter */
					fprintf(outfile, SAVE_PRT_02,
							sq4[idxSB].DiffTime,  sq4[idxSP].DiffTime, sq4[idxSF].DiffTime, sq4[idxSS].LPFAcc, sq4[idxSB].LPFAcc, sq4[idxSP].LPFAcc, 
							G_dif );
				#endif
				
					Nt_dif = (sq4[idxNtMx].Nt16 - sq4[idxNtmn].Nt16);
					fprintf(outfile, SAVE_PRT_03,
							sq4[idxJerk].fJerk1,  sq4[idxJerk].deltams, sq4[idxNtmn].Nt16, sq4[idxNtMx].Nt16, Nt_dif  );

				#if 0
					fprintf(outfile, SAVE_PRT_04,
							sq4[idxSB].Nt16, sq4[idxNt].Nt16, (idxNt>idxSB)?"(+)":((idxNt<idxSB)?"(-)":"(0)") ,
							sq4[idxSB].Ne15, sq4[idxNe].Ne15, (idxNe>idxSB)?"(+)":((idxNe<idxSB)?"(-)":"(0)") );
				#else
					fprintf(outfile, SAVE_PRT_05,
							sq4[idxSB].Nt16, sq4[idxSB].Ne15, sq4[idxNtMx].Ne15 );
				#endif

					if(1==is2File) fprintf(outfile, "\n");
					if(2==is2File) fprintf(outfile, "\n\n");		

				}
			#endif



				if( is2File )
				{
					/* ----------------------------------------------------------------------- */
					/* ----------------------------------------------------------------------- */
					/* -- 변속 준비 시간 (S.S~S.B Time) -- */
					/* ----------------------------------------------------------------------- */
					if( 0==tblT1[ sq4[idxSS].apsIdx ][ sq4[idxSS].curGear08 ].value )
					{
						tblT1[ sq4[idxSS].apsIdx ][ sq4[idxSS].curGear08 ].gearIdx = sq4[idxSS].curGear08;
						sprintf(tblT1[ sq4[idxSS].apsIdx ][ sq4[idxSS].curGear08 ].gearTxt,"(%d%d)", sq4[idxSS].curGear08, sq4[idxSS].curGear08+1);
						tblT1[ sq4[idxSS].apsIdx ][ sq4[idxSS].curGear08 ].value   = (int)( (sq4[idxSB].DiffTime)*1000*10 );
					#if WIDE_CHAR_SYMBOL
						wcscpy( tblT1[ sq4[idxSS].apsIdx ][ sq4[idxSS].curGear08 ].symbol, gSymbol[sq4[idxSS].curGear08] );
						//fwprintf(stderr,L"[%4ls],  [%5ls] \n",  tblT1[ sq4[idxSS].apsIdx ][ sq4[idxSS].curGear08 ].symbol, gSymbol[sq4[idxSS].curGear08] );
					#else
						strcpy( tblT1[ sq4[idxSS].apsIdx ][ sq4[idxSS].curGear08 ].symbol, gSymbol[sq4[idxSS].curGear08] );
					#endif
					}
					else 
					{
						/* -- average -- */
						tblT1[ sq4[idxSS].apsIdx ][ sq4[idxSS].curGear08 ].gearIdx = sq4[idxSS].curGear08;
						sprintf(tblT1[ sq4[idxSS].apsIdx ][ sq4[idxSS].curGear08 ].gearTxt,"(%d%d)", sq4[idxSS].curGear08, sq4[idxSS].curGear08+1);
						tblT1[ sq4[idxSS].apsIdx ][ sq4[idxSS].curGear08 ].value += (int)( (sq4[idxSB].DiffTime)*1000*10 );
						tblT1[ sq4[idxSS].apsIdx ][ sq4[idxSS].curGear08 ].value /= 2.0;
					#if WIDE_CHAR_SYMBOL
						wcscpy( tblT1[ sq4[idxSS].apsIdx ][ sq4[idxSS].curGear08 ].symbol, gSymbol[sq4[idxSS].curGear08] );
					#else
						strcpy( tblT1[ sq4[idxSS].apsIdx ][ sq4[idxSS].curGear08 ].symbol, gSymbol[sq4[idxSS].curGear08] );
					#endif
					}
					
					/* ----------------------------------------------------------------------- */
					/* -- 실 변속 시간 (S.B~S.P Time) -- */
					if( 0==tblT2[ sq4[idxSS].apsIdx ][ sq4[idxSS].curGear08 ].value )
					{
						tblT2[ sq4[idxSS].apsIdx ][ sq4[idxSS].curGear08 ].gearIdx = sq4[idxSS].curGear08;
						sprintf(tblT2[ sq4[idxSS].apsIdx ][ sq4[idxSS].curGear08 ].gearTxt,"(%d%d)", sq4[idxSS].curGear08, sq4[idxSS].curGear08+1);
						tblT2[ sq4[idxSS].apsIdx ][ sq4[idxSS].curGear08 ].value   = (int)( (sq4[idxSP].DiffTime)*1000*10 );
					#if WIDE_CHAR_SYMBOL
						wcscpy( tblT2[ sq4[idxSS].apsIdx ][ sq4[idxSS].curGear08 ].symbol, gSymbol[sq4[idxSS].curGear08] );
					#else
						strcpy( tblT2[ sq4[idxSS].apsIdx ][ sq4[idxSS].curGear08 ].symbol, gSymbol[sq4[idxSS].curGear08] );
					#endif
					}
					else
					{
						/* -- average -- */
						tblT2[ sq4[idxSS].apsIdx ][ sq4[idxSS].curGear08 ].gearIdx = sq4[idxSS].curGear08;
						sprintf(tblT2[ sq4[idxSS].apsIdx ][ sq4[idxSS].curGear08 ].gearTxt,"(%d%d)", sq4[idxSS].curGear08, sq4[idxSS].curGear08+1);
						tblT2[ sq4[idxSS].apsIdx ][ sq4[idxSS].curGear08 ].value += (int)( (sq4[idxSP].DiffTime)*1000*10 );
						tblT2[ sq4[idxSS].apsIdx ][ sq4[idxSS].curGear08 ].value /= 2.0;
					#if WIDE_CHAR_SYMBOL
						wcscpy( tblT2[ sq4[idxSS].apsIdx ][ sq4[idxSS].curGear08 ].symbol, gSymbol[sq4[idxSS].curGear08] );					
					#else
						strcpy( tblT2[ sq4[idxSS].apsIdx ][ sq4[idxSS].curGear08 ].symbol, gSymbol[sq4[idxSS].curGear08] ); 				
					#endif
					}

					
					/* ----------------------------------------------------------------------- */
					/* -- Jerk (S,B 부근) -- */
					if( 0==tblJerk1[ sq4[idxSS].apsIdx ][ sq4[idxSS].curGear08 ].value )
					{
						tblJerk1[ sq4[idxSS].apsIdx ][ sq4[idxSS].curGear08 ].gearIdx = sq4[idxSS].curGear08;
						sprintf(tblJerk1[ sq4[idxSS].apsIdx ][ sq4[idxSS].curGear08 ].gearTxt,"(%d%d)", sq4[idxSS].curGear08, sq4[idxSS].curGear08+1);
						tblJerk1[ sq4[idxSS].apsIdx ][ sq4[idxSS].curGear08 ].value = (int)( (sq4[idxJerk].fJerk1)*100 );
					#if WIDE_CHAR_SYMBOL
						wcscpy( tblJerk1[ sq4[idxSS].apsIdx ][ sq4[idxSS].curGear08 ].symbol, gSymbol[sq4[idxSS].curGear08] );
					#else
						strcpy( tblJerk1[ sq4[idxSS].apsIdx ][ sq4[idxSS].curGear08 ].symbol, gSymbol[sq4[idxSS].curGear08] );
					#endif
					}
					else
					{
						/* -- average -- */
						tblJerk1[ sq4[idxSS].apsIdx ][ sq4[idxSS].curGear08 ].gearIdx = sq4[idxSS].curGear08;
						sprintf(tblJerk1[ sq4[idxSS].apsIdx ][ sq4[idxSS].curGear08 ].gearTxt,"(%d%d)", sq4[idxSS].curGear08, sq4[idxSS].curGear08+1);
						tblJerk1[ sq4[idxSS].apsIdx ][ sq4[idxSS].curGear08 ].value += (int)( (sq4[idxJerk].fJerk1)*100 );
						tblJerk1[ sq4[idxSS].apsIdx ][ sq4[idxSS].curGear08 ].value /= 2.0;
					#if WIDE_CHAR_SYMBOL
						wcscpy( tblJerk1[ sq4[idxSS].apsIdx ][ sq4[idxSS].curGear08 ].symbol, gSymbol[sq4[idxSS].curGear08] );
					#else
						strcpy( tblJerk1[ sq4[idxSS].apsIdx ][ sq4[idxSS].curGear08 ].symbol, gSymbol[sq4[idxSS].curGear08] );
					#endif
					}

					
					/* ----------------------------------------------------------------------- */
					/* -- Max Ne (S.B 부근) -- */
					if( 0==tblNeMX[ sq4[idxSS].apsIdx ][ sq4[idxSS].curGear08 ].value )
					{
						tblNeMX[ sq4[idxSS].apsIdx ][ sq4[idxSS].curGear08 ].gearIdx = sq4[idxSS].curGear08;
						sprintf(tblNeMX[ sq4[idxSS].apsIdx ][ sq4[idxSS].curGear08 ].gearTxt,"(%d%d)", sq4[idxSS].curGear08, sq4[idxSS].curGear08+1);
						tblNeMX[ sq4[idxSS].apsIdx ][ sq4[idxSS].curGear08 ].value = (int)( (sq4[idxNtMx].Ne15)*100 );
					#if WIDE_CHAR_SYMBOL
						wcscpy( tblNeMX[ sq4[idxSS].apsIdx ][ sq4[idxSS].curGear08 ].symbol, gSymbol[sq4[idxSS].curGear08] );
					#else
						strcpy( tblNeMX[ sq4[idxSS].apsIdx ][ sq4[idxSS].curGear08 ].symbol, gSymbol[sq4[idxSS].curGear08] );
					#endif
					}
					else
					{
						/* -- average -- */
						tblNeMX[ sq4[idxSS].apsIdx ][ sq4[idxSS].curGear08 ].gearIdx = sq4[idxSS].curGear08;
						sprintf(tblNeMX[ sq4[idxSS].apsIdx ][ sq4[idxSS].curGear08 ].gearTxt,"(%d%d)", sq4[idxSS].curGear08, sq4[idxSS].curGear08+1);
						tblNeMX[ sq4[idxSS].apsIdx ][ sq4[idxSS].curGear08 ].value += (int)( (sq4[idxNtMx].Ne15)*100 );
						tblNeMX[ sq4[idxSS].apsIdx ][ sq4[idxSS].curGear08 ].value /= 2.0;
					#if WIDE_CHAR_SYMBOL
						wcscpy( tblNeMX[ sq4[idxSS].apsIdx ][ sq4[idxSS].curGear08 ].symbol, gSymbol[sq4[idxSS].curGear08] );
					#else
						strcpy( tblNeMX[ sq4[idxSS].apsIdx ][ sq4[idxSS].curGear08 ].symbol, gSymbol[sq4[idxSS].curGear08] );
					#endif
					}

					/* ----------------------------------------------------------------------- */
					/* -- Max Nt (S.B 부근) -- */
					if( 0==tblNtMX[ sq4[idxSS].apsIdx ][ sq4[idxSS].curGear08 ].value )
					{
						tblNtMX[ sq4[idxSS].apsIdx ][ sq4[idxSS].curGear08 ].gearIdx = sq4[idxSS].curGear08;
						sprintf(tblNtMX[ sq4[idxSS].apsIdx ][ sq4[idxSS].curGear08 ].gearTxt,"(%d%d)", sq4[idxSS].curGear08, sq4[idxSS].curGear08+1);
						tblNtMX[ sq4[idxSS].apsIdx ][ sq4[idxSS].curGear08 ].value = (int)( (sq4[idxNtMx].Nt16)*100 );
					#if WIDE_CHAR_SYMBOL
						wcscpy( tblNtMX[ sq4[idxSS].apsIdx ][ sq4[idxSS].curGear08 ].symbol, gSymbol[sq4[idxSS].curGear08] );
					#else
						strcpy( tblNtMX[ sq4[idxSS].apsIdx ][ sq4[idxSS].curGear08 ].symbol, gSymbol[sq4[idxSS].curGear08] );
					#endif
					}
					else
					{
						/* -- average -- */
						tblNtMX[ sq4[idxSS].apsIdx ][ sq4[idxSS].curGear08 ].gearIdx = sq4[idxSS].curGear08;
						sprintf(tblNtMX[ sq4[idxSS].apsIdx ][ sq4[idxSS].curGear08 ].gearTxt,"(%d%d)", sq4[idxSS].curGear08, sq4[idxSS].curGear08+1);
						tblNtMX[ sq4[idxSS].apsIdx ][ sq4[idxSS].curGear08 ].value += (int)( (sq4[idxNtMx].Nt16)*100 );
						tblNtMX[ sq4[idxSS].apsIdx ][ sq4[idxSS].curGear08 ].value /= 2.0;
					#if WIDE_CHAR_SYMBOL
						wcscpy( tblNtMX[ sq4[idxSS].apsIdx ][ sq4[idxSS].curGear08 ].symbol, gSymbol[sq4[idxSS].curGear08] );
					#else
						strcpy( tblNtMX[ sq4[idxSS].apsIdx ][ sq4[idxSS].curGear08 ].symbol, gSymbol[sq4[idxSS].curGear08] );
					#endif
					}
				}


				index = 0;

				idxJerk = 0;

				idxgm   = 0;
				idxgX   = 0;
				idxNtMx = 0;
				idxNtmn = 0;

				idxSS = 0;
				idxSB = 0;
				idxSP = 0;
				idxSF = 0;
				memset(sq4, 0x00, MAX_RPT_BUF_SIZ*sizeof(sqdAps_type) );

			}



			/* ------------------------------------------------ */
			/* SS~ NtMax~ GMax~ Gmin~ SB ~ SP~ Ntmin~ SF ------ */
			/* ------------------------------------------------ */
			index ++;
			if( index >= MAX_RPT_BUF_SIZ ) index = 0;

		}
	}
	while (!feof (inpfile));



	/* -- 1 ------------------------------ */
	/* -- 변속 준비 시간 (S.S~S.B Time) -- */
	/* -- 1 ------------------------------ */
	for(xx=0; xx<APS_TABLE_NUM && (int)ApsTble[xx]; xx++)
	{
		; /* Just, find xxMax ~~ */
	}
	yyMax  = icurGear;
	xxMax  = xx;
	xxMore = xxMax-6;

	
	fprintf(outfile,"\r\n\r\n\n");

	fprintf(outfile," %8s === Up shift Average t1 time (SS~SB) === ", "");
	for(ii=0; ii<FUNC_SPC+14; ii++) fprintf(outfile," ");
	if(xxMore) for(ii=0; ii<(SPACE_ONE_UNIT-2)*xxMore; ii++) fprintf(outfile," ");

	fprintf(outfile," %2s === Up shift Average t2 time (SB~SP) === ", "");
	fprintf(outfile,"\r\n");


	//fprintf(outfile,"t1(SS~SB) ");
	fprintf(outfile,"<t1(SS~SB)>");
	for(xx=0; xx<APS_TABLE_NUM && (int)ApsTble[xx]; xx++)
	{
		fprintf(outfile," %4d(%%)", (int)ApsTble[xx] );
	}

	for(ii=0; ii<FUNC_SPC; ii++) fprintf(outfile," ");

	//fprintf(outfile,"t2(SB~SP) ");
	fprintf(outfile,"<t2(SB~SP)>");
	for(xx=0; xx<APS_TABLE_NUM && (int)ApsTble[xx]; xx++)
	{
		fprintf(outfile," %4d(%%)", (int)ApsTble[xx] );
	}
	fprintf(outfile,"\n");

	
	for(ii=0; ii<60; ii++) fprintf(outfile,"-");
	if(xxMore) for(ii=0; ii<(SPACE_ONE_UNIT-2)*xxMore; ii++) fprintf(outfile,"-");

	for(ii=0; ii<FUNC_SPC-2; ii++) fprintf(outfile," ");

	for(ii=0; ii<60; ii++) fprintf(outfile,"-");
	if(xxMore) for(ii=0; ii<(SPACE_ONE_UNIT-2)*xxMore; ii++) fprintf(outfile,"-");

	fprintf(outfile,"\n");
	

	for(yy=1; yy<=yyMax; yy++)
	{
		//fprintf(outfile,"  <%d-%d> : ", yy,yy+1 ); /* <curGear-tgtGear>*/
		fprintf(outfile,"   (%d%d) : ", yy,yy+1 ); /* <curGear-tgtGear>*/

		for(xx=0; xx<xxMax; xx++)
		{
			if( 0 == tblT1[ xx ][ yy ].value ) 
				fprintf(outfile," %7s","x");
			else
			{
				fprintf(outfile," %7.3lf", (tblT1[ xx ][ yy ].value /1000.0f/10.0f) );
				//tblT1[ 0 ][ yy ].YvalNum ++;
			}
		}

		for(ii=0; ii<FUNC_SPC; ii++) fprintf(outfile," ");
		//fprintf(outfile,"  <%d-%d> : ", yy,yy+1 ); /* <curGear-tgtGear>*/
		fprintf(outfile,"    (%d%d) : ", yy,yy+1 ); /* <curGear-tgtGear>*/

		for(xx=0; xx<xxMax; xx++)
		{
			if( 0 == tblT2[ xx ][ yy ].value ) 
				fprintf(outfile," %7s","x");
			else
			{
				fprintf(outfile," %7.3lf", (tblT2[ xx ][ yy ].value /1000.0f/10.0f) );
				//tblT2[ 0 ][ yy ].YvalNum ++;
			}
		}
		fprintf(outfile,"\n");
	}
	fprintf(outfile,"\r\n");

#if 1

	/* ---------------------------------------------------------- */
	/* --- t1, t2 Qsorting  ------------------------------------- */
	/* ---------------------------------------------------------- */

	t1t2GraphAferQsort(tblT1, tblT2, xxMax, yyMax, gValueDisplay);


#else
	/* ---------------------------------------------------------- */
	/* --- t1, t2 Qsorting  ------------------------------------- */
	/* ---------------------------------------------------------- */
	fprintf(outfile,"\n [sec]");
	for(ii=0; ii<SPACE_ONE; ii++) fprintf(outfile," ");
	fprintf(outfile," [sec]\n");
	
	for(xx=0; xx<xxMax ; xx++)
	{
		qsort(tblT1[xx], sizeof(tblT1[xx])/sizeof(graphData_type), sizeof(graphData_type), QsortCompare); 

		qsort(tblT2[xx], sizeof(tblT2[xx])/sizeof(graphData_type), sizeof(graphData_type), QsortCompare); 
	}


	/* Max Value */
	gYMaxt1 = tblT1[0][0].value/1000.0f;
	gYMaxt2 = tblT2[0][0].value/1000.0f;

	/* min value */
	for(xx=0; xx<xxMax ; xx++)
	{
		for(yy=0; yy<yyMax; yy++)
		{
			if( 0==tblT1[xx][yy].value || 0==tblT2[xx][yy].value ) continue;
			if( gYmint1 > tblT1[xx][yy].value ) gYmint1 = tblT1[xx][yy].value ;
			if( gYmint2 > tblT2[xx][yy].value ) gYmint2 = tblT2[xx][yy].value ;
		}
	}
	gYmint1 /= 1000.0f;
	gYmint2 /= 1000.0f;


	/* ----------- t1 & t2 Graph ------------------ */
	for(yy=0; yy<yyMax; yy++)
	{
		if(yy>0)
		{
			fprintf(outfile,"\n");
			fprintf(outfile,"     |");
			for(ii=0; ii<SPACE_ONE; ii++) fprintf(outfile," ");
			fprintf(outfile,"     |");
			fprintf(outfile,"\n");
		}

		if(yy==0)
		{
			fprintf(outfile," %4.1lf|", (float)(gYMaxt1-yy)/10.0f  );
		}
		else if(yy==yyMax-1)
		{
			fprintf(outfile," %4.1lf|", (gYmint1)/10.0f  );
		}
		else
			fprintf(outfile," %4s|", "");
			
		for(xx=0; xx<xxMax; xx++)
		{
			if( 0 == tblT1[ xx ][ yy ].value ) 
				fprintf(outfile," %9s","x ");
			else
				fprintf(outfile," %5s%-4.1lf", tblT1[ xx ][ yy ].gearTxt, (tblT1[ xx ][ yy ].value /1000.0f/10.0f) );
		}

		for(ii=0; ii<FUNC_SPC-16; ii++) fprintf(outfile," ");

		if(yy==0)
		{
			fprintf(outfile," %4.1lf|", (float)(gYMaxt2-yy)/10.0f  );
		}
		else if(yy==yyMax-1)
		{
			fprintf(outfile," %4.1lf|", (gYmint2)/10.0f  );
		}
		else
			fprintf(outfile," %4s|", "");

		for(xx=0; xx<xxMax; xx++)
		{
			if( 0 == tblT2[ xx ][ yy ].value ) 
				fprintf(outfile," %9s","x ");
			else
				fprintf(outfile," %5s%-4.1lf", tblT2[ xx ][ yy ].gearTxt, (tblT2[ xx ][ yy ].value /1000.0f/10.0f) );
		}
	#if 0
		fprintf(outfile,"\n");
		fprintf(outfile,"     |");
		for(ii=0; ii<70; ii++) fprintf(outfile," ");
		fprintf(outfile,"     | \n");
	#endif
	}


	fprintf(outfile,"\n");
	fprintf(outfile,"     |");
	for(ii=0; ii<SPACE_ONE; ii++) fprintf(outfile," ");
	fprintf(outfile,"     |");
	fprintf(outfile,"\n");

	fprintf(outfile,"     |");
	for(ii=0; ii<SPACE_ONE; ii++) fprintf(outfile," ");
	fprintf(outfile,"     |");
	fprintf(outfile,"\n");

	fprintf(outfile,"     +");
	for(ii=0; ii<SPACE_ONE; ii++) fprintf(outfile,"-");

	fprintf(outfile,"     +");
	for(ii=0; ii<SPACE_ONE; ii++) fprintf(outfile,"-");
	
	fprintf(outfile,"\n   ");
	for(xx=0; xx<APS_TABLE_NUM && (int)ApsTble[xx]; xx++)
	{
		fprintf(outfile," %6d(%%)", (int)ApsTble[ xx ] );
	}

	for(ii=0; ii<7; ii++) fprintf(outfile," ");

	for(xx=0; xx<APS_TABLE_NUM && (int)ApsTble[xx]; xx++)
	{
		fprintf(outfile," %6d(%%)", (int)ApsTble[ xx ] );
	}

	fprintf(outfile,"\n");

#endif


	fprintf(outfile,"\r\n\r\n");


#if 0
	/* -- 2 ------------------------------ */
	/* -- 실 변속 시간 (S.B~S.P Time) ---- */
	/* -- 2 ------------------------------ */
	yyMax = icurGear;
	fprintf(outfile,"t2(SB~SP)-");
	for(xx=0; xx<APS_TABLE_NUM && (int)ApsTble[xx]; xx++)
	{
		fprintf(outfile," %4d(%%)", (int)ApsTble[ xx ] );
	}
	xxMax = xx;
	fprintf(outfile,"\n");
	
	for(yy=1; yy<=yyMax; yy++)
	{
		fprintf(outfile,"  <%d-%d> : ", yy,yy+1 );
		for(xx=0; xx<xxMax; xx++)
		{
			if( 0 == tblT2[ xx ][ yy ] ) 
				fprintf(outfile," %7s","x");
			else
				fprintf(outfile," %7.3lf", (tblT2[ xx ][ yy ]/1000.0f/10.0f) );
		}
		fprintf(outfile,"\n");
	}
	fprintf(outfile,"\r\n\r\n");
#endif


	/* -- 3 ------------------------------ */
	/* -- Jerk (S,B 부근) ---------------- */
	/* -- and              --------------- */
	/* -- Max Ne (S.B 부근) -------------- */
	/* -- 3 ------------------------------ */

	fprintf(outfile," %8s === Up shift Average Jerk1 by LPF === ","");
	for(ii=0; ii<FUNC_SPC+17; ii++) fprintf(outfile," ");
	if(xxMore) for(ii=0; ii<(SPACE_ONE_UNIT-2)*xxMore; ii++) fprintf(outfile," ");

	fprintf(outfile," %2s === Up shift Average Ne Max === ", "");
	fprintf(outfile,"\r\n");
	
	//fprintf(outfile,"<<Jerk1>> ");
	fprintf(outfile," <Jerk1>  ");
	for(xx=0; xx<APS_TABLE_NUM && (int)ApsTble[xx]; xx++)
	{
		fprintf(outfile," %4d(%%)", (int)ApsTble[ xx ] );
	}

	for(ii=0; ii<FUNC_SPC; ii++) fprintf(outfile," ");

	//fprintf(outfile,"<<Ne Max>>");
	fprintf(outfile," <Ne Max> ");
	for(xx=0; xx<APS_TABLE_NUM && (int)ApsTble[xx]; xx++)
	{
		fprintf(outfile," %4d(%%)", (int)ApsTble[ xx ] );
	}

	yyMax = icurGear;
	xxMax = xx;
	xxMore = xxMax-6;


	fprintf(outfile,"\n");
	for(ii=0; ii<60; ii++) fprintf(outfile,"-");
	if(xxMore) for(ii=0; ii<(SPACE_ONE_UNIT-2)*xxMore; ii++) fprintf(outfile,"-");

	for(ii=0; ii<FUNC_SPC-2; ii++) fprintf(outfile," ");

	for(ii=0; ii<60; ii++) fprintf(outfile,"-");
	if(xxMore) for(ii=0; ii<(SPACE_ONE_UNIT-2)*xxMore; ii++) fprintf(outfile,"-");
	fprintf(outfile,"\n");




	for(yy=1; yy<=yyMax; yy++)
	{
		//fprintf(outfile,"  <%d-%d> : ", yy,yy+1 ); /* <curGear-tgtGear>*/
		fprintf(outfile,"   (%d%d) : ", yy,yy+1 ); /* <curGear-tgtGear>*/
		for(xx=0; xx<xxMax; xx++)
		{
			if( 0 == tblJerk1[ xx ][ yy ].value ) 
				fprintf(outfile," %7s","x");
			else
			{
				fprintf(outfile," %7.2lf", (tblJerk1[ xx ][ yy ].value)/100.0f );
				//tblJerk1[ 0 ][ yy ].YvalNum ++;
			}
		}

		for(ii=0; ii<FUNC_SPC; ii++) fprintf(outfile," ");

		//fprintf(outfile,"  <%d-%d> : ", yy,yy+1 ); /* <curGear-tgtGear>*/
		fprintf(outfile,"    (%d%d) : ", yy,yy+1 ); /* <curGear-tgtGear>*/
		for(xx=0; xx<xxMax; xx++)
		{
			if( 0 == tblNeMX[ xx ][ yy ].value ) 
				fprintf(outfile," %7s","x");
			else
			{
				fprintf(outfile," %7.2lf", (tblNeMX[ xx ][ yy ].value)/100.0f );
				//tblNeMX[ 0 ][ yy ].YvalNum ++;
			}
		}
	
		fprintf(outfile,"\n");
	}
	fprintf(outfile,"\r\n");


	/* ---------------------------------------------------------- */
	/* --- Jerk1, Ne_Max Qsorting	----------------------------- */
	/* ---------------------------------------------------------- */
	for(yy=0; yy<GRAPH_Y_BUF_SIZ; yy++)
	{
		for(xx=0; xx<APS_TABLE_NUM; xx++)
		{
			tblJerk1[xx][yy].value = abs( tblJerk1[xx][yy].value );
		}
	}
		
	JerkGraphAferQsort(tblJerk1, tblNeMX, xxMax, yyMax, gValueDisplay );


#if 0
	/* -- 4 ------------------------------ */
	/* -- Max Ne (S.B 부근) -------------- */
	/* -- 4 ------------------------------ */
	yyMax = icurGear;
	fprintf(outfile,"--Ne Max--");
	for(xx=0; xx<APS_TABLE_NUM && (int)ApsTble[xx]; xx++)
	{
		fprintf(outfile," %4d(%%)", (int)ApsTble[ xx ] );
	}
	xxMax = xx;
	fprintf(outfile,"\n");

	for(yy=1; yy<=yyMax; yy++)
	{
		fprintf(outfile,"  <%d-%d> : ", yy,yy+1 ); /* <curGear-tgtGear>*/
		for(xx=0; xx<xxMax; xx++)
		{
			if( 0 == tblNeMX[ xx ][ yy ] ) 
				fprintf(outfile," %7s","x");
			else
				fprintf(outfile," %7.2lf", (tblNeMX[ xx ][ yy ]/100.0f) );
		}
		fprintf(outfile,"\n");
	}
#endif


#if 0
	fprintf(outfile,"\r\n\r\n");

	/* -- 5 ------------------------------ */
	/* -- Max Nt (S.B 부근) -------------- */
	/* -- 5 ------------------------------ */
	yyMax = icurGear;
	fprintf(outfile,"--Nt Max--");
	for(xx=0; xx<APS_TABLE_NUM && (int)ApsTble[xx]; xx++)
	{
		fprintf(outfile," %4d(%%)", (int)ApsTble[ xx ] );
	}
	xxMax = xx;
	fprintf(outfile,"\n");

	for(yy=1; yy<=yyMax; yy++)
	{
		fprintf(outfile,"  <%d-%d> : ", yy,yy+1 ); /* <curGear-tgtGear>*/
		for(xx=0; xx<xxMax; xx++)
		{
			if( 0 == tblNtMX[ xx ][ yy ].value ) 
				fprintf(outfile," %7s","x");
			else
				fprintf(outfile," %7.2lf", (tblNtMX[ xx ][ yy ].value/100.0f) );
		}
		fprintf(outfile,"\n");
	}
	//fprintf(outfile,"\r\n\r\n");
#endif


#if 0
	fprintf(outfile, "\n\n");
	fprintf(outfile, SAVE_PRT_10);
	fprintf(outfile, SAVE_PRT_11);
	fprintf(outfile, SAVE_PRT_12);
	fprintf(outfile, SAVE_PRT_13);
	fprintf(outfile, SAVE_PRT_14);
	fprintf(outfile, SAVE_PRT_15);
	fprintf(outfile, SAVE_PRT_16);
#endif


	if(inpfile) fclose(inpfile);
	if(outfile) fclose(outfile);

//	fprintf(stderr,"----------------------------------------------------------------------------------\n" );
//	fprintf(stderr,">>Shift Quality Data Sorting finished!!! %s [%s] -> [%s] \r\n", arrPATs_ModeID[aiPATs05].ModeID, shi_inp, shi_out  ); 		

	fprintf(stderr,">>Final Sorted result report file: %s \n", shi_out );
	fprintf(stderr,"----------------------------------------------------------------------------------\n" );

	return 1;
}



int tempFileDeleteIt(short YesNo, short iModeID, unsigned int iSBchk)
{
	short isNaming = 0;
	short ii=0;

	char shi_ori[MAX_CHARS*LENGTH_OF_FILENAME+1];
	char shi_out[MAX_CHARS*LENGTH_OF_FILENAME+1];
	
	
	memset(shi_ori, 0x00, (MAX_CHARS*LENGTH_OF_FILENAME+1)*sizeof(char) );
	memset(shi_out, 0x00, (MAX_CHARS*LENGTH_OF_FILENAME+1)*sizeof(char) );
	
	strcpy(shi_ori, shift_file);
	isNaming = 0;
	for(ii=strlen(shi_ori)-1; ii>0; ii--)
	{
		if( shi_ori[ii]=='.' ) 
		{
			shi_ori[ii+1] = '\0';
			isNaming = 1;
			break;
		}
	}
	if( 0==isNaming ) strcat(shi_ori, ".");
	

	if(YesNo)
	{
		// -------------------------------------------------------------
		strcpy(shi_out, shi_ori);
		strcat(shi_out, "b");
		strcat(shi_out, arrPATs_ModeID[iModeID].ModeNm);
		if( isFileExist(shi_out, 0) )
		{
			ii = remove( shi_out );
			if( 0 != ii )
				fprintf(stderr,"  Deleted temp file [%s] -> Failed(%d) \n", shi_out, ii );
		}
		
		// -------------------------------------------------------------
		if( iSBchk )
		{
			strcpy(shi_out, shi_ori);
			strcat(shi_out, "i");
			strcat(shi_out, arrPATs_ModeID[iModeID].ModeNm);
			if( isFileExist(shi_out, 0) )
			{
				ii = remove( shi_out );
				if( 0 != ii )
					fprintf(stderr,"  Deleted temp file [%s] -> Failed(%d) \n", shi_out, ii );
			}
		}
		// -------------------------------------------------------------
		strcpy(shi_out, shi_ori);
		strcat(shi_out, "g");
		strcat(shi_out, arrPATs_ModeID[iModeID].ModeNm);
		
		if( isFileExist(shi_out, 0) )
		{
			ii = remove( shi_out );
			if( 0 != ii )
				fprintf(stderr,"  Deleted temp file [%s] -> Failed(%d) \n", shi_out, ii );
		}
		// -------------------------------------------------------------
		strcpy(shi_out, shi_ori);
		strcat(shi_out, FILTER_EXT_DTA);
		if( isFileExist(shi_out, 0) )
		{
			ii = remove( shi_out );
			if( 0 != ii )
				fprintf(stderr,"  Deleted temp file [%s] -> Failed(%d) \n", shi_out, ii );
		}
		// -------------------------------------------------------------

		strcpy(shi_out, shi_ori);
		strcat(shi_out, arrPATs_ModeID[iModeID].ModeNm);
		if( isFileExist(shi_out, 0) )
		{
			ii = remove( shi_out );
			if( 0 != ii )
				fprintf(stderr,"  Deleted temp file [%s] -> Failed(%d) \n", shi_out, ii );
		}
		//fprintf(stderr,"----------------------------------------------------------------------------------\n" );
	}

	return 1;
}
#endif /* SHIFT_QUALITY_DATA_SORTING */


void FileSearch(char file_path[], char *findPath)
{
	struct _finddata_t fd;
	intptr_t handle;
	int check = 0;
	char file_path2[_MAX_PATH];

	memset(file_path2, 0x00, sizeof(file_path2));
	memset( findPath, 0x00, sizeof(findPath) );

	strcat(file_path, "\\");
	strcpy(file_path2, file_path);
	strcat(file_path, "*.*");
 
	if ((handle = _findfirst(file_path, &fd)) == -1)
	{
		return;
	}
 
	while (_findnext(handle, &fd) == 0)
	{
		char file_pt[_MAX_PATH];
		char find[_MAX_PATH] = "\\shiftLicense.gil";
		int find_sol = 1;

		memset(file_pt, 0x00, sizeof(file_pt) );
		strcpy(file_pt, file_path2);
		strcat(file_pt, fd.name);

 
		//printf("findfolder : %s [%s] \n", fd.name, file_pt );
        //check = isFileOrDir();    

		if (fd.attrib & _A_SUBDIR) 
		{
			if ( fd.name[0] == '.' && fd.name[1] == '.' )
			{
				continue;
			}
			else // if ( fd.name[0] != '.' && fd.name[1] != '.' )
			{
				//findfolder : All Users [C:\Users\All Users]
				//findfolder : Default [C:\Users\Default]
				//findfolder : Default User [C:\Users\Default User]
				//findfolder : Public [C:\Users\Public]
				//printf("findfolder : %s [%s] \n", fd.name, file_pt );

				strcat(file_pt, find);

				find_sol = isFileExist(file_pt, 0);
				if( 1==find_sol /*&& (0==check)*/ )
				{
					//printf("FINDIT : [ %s ]  \n", file_pt );
					strcpy(findPath, file_pt);
					break;
				}
				FileSearch(file_pt, findPath);
			}
		}      
    }
    _findclose(handle);

}


int CheckLicense(void)
{
#define MAC_BUF_SIZ 		300
#define MAC_ADDR_NUM 		6
#define MAC_ADDR_LINE_SIZ 	10
//#define HOSTPC_MAC_FILE 	"C:/Temp/ahLic.mac"
#define HOSTPC_MAC_TEMP 	"C:/Temp/tmp111.tmp"
#define HOSTPC_SEND2ME 		"./send2me.gil"
//#define HOSTPC_LICENSE 		"./shift.lic"
#define SHA3_512_LICENSE_LEN 		128

	char tmpData[MAC_BUF_SIZ];
	FILE *fi = NULL;
	FILE *fr = NULL;
	//FILE *fo = NULL;
	
	char LicFile[MAC_ADDR_LINE_SIZ][MAC_BUF_SIZ] = {0,};
	int result;
	int re = 0, fres, ii=0, licOk=0;
	int iOKc=0, iNGc=0, lloop=0, iFinal=0;
	int iret = 0;
	char userPath[_MAX_PATH] = "C:\\Users";
	char findPath[_MAX_PATH]; 
	unsigned char sha3License[MAC_ADDR_LINE_SIZ][50];
	unsigned char macAdd[MAC_ADDR_LINE_SIZ][7];


	///////////////////////////////////////////////////////////
	memset( LicFile, 0x00, sizeof(LicFile) );
	memset( findPath, 0x00, sizeof(findPath) );
	
	memset( sha3License, 0xFF, sizeof(sha3License) );
 
    FileSearch(userPath, findPath);
	if( NULL == (fr = fopen( findPath /*HOSTPC_LICENSE*/, "rb")) ) 
	{
		fprintf(stderr,"\r\nCan not read License file. [%s] / shiftLicense.gil \n\n", findPath /*HOSTPC_LICENSE */ );
		re = system("getmac > send2me.gil");
		if( 0!=re )
		{
			fprintf(stderr,"\r\nLicense temp check needed.. (%d) \n\n", re);
			exit(0);
		}

		fclose(fr);
		exit(0);
	}
	else
	{
		if( 0 != remove( HOSTPC_SEND2ME ) )
		{
			//fprintf(stderr,"  Deleted temp file -> Failed \n");
		}
	
		ii=0;
		memset( LicFile, 0x00, sizeof(LicFile) );	
		do {
			fscanf(fr, "%s", LicFile[ii] );
			if(LicFile[ii][0] == ' ' || LicFile[ii][0] == '\0' || LicFile[ii][0] == '\r' || LicFile[ii][0] == '\n' ) 
			{
				LicFile[ii][0] = '\0';
				break;
			}
			//fprintf(stderr, "%d -> %s\n", ii, LicFile[ii]);
			if(ii++ > MAC_ADDR_LINE_SIZ) break;
		} while (!feof (fr));
		fclose(fr);
	}
	///////////////////////////////////////////////////////////

	//fres = isFileExist(HOSTPC_MAC_FILE, 0 );
	fres = isFileExist(HOSTPC_MAC_TEMP, 0 );
	//fprintf(stderr, "fres = %d \n", fres );
	if(0==fres)
	{	
		re = system("getmac > C:/Temp/tmp111.tmp");
		if( 0!=re )
		{
			fprintf(stderr,"\r\nLicense check needed.. [%d] \n\n", re);
			exit(0);
		}
	}


	if( NULL == (fi = fopen( HOSTPC_MAC_TEMP, "rb")) ) 
	{
		fprintf(stderr,"\nCan not read license related file... \n\n");
		exit(0);
	}
	else
	{
		#if 0
		if( NULL == (fo = fopen( HOSTPC_MAC_FILE, "wb")) ) 
		{
			fprintf(stderr,"\nCan not write alLic.mac temp file.. \n\n");
			if(fi) fclose(fi);
			if(fo) fclose(fo);
			exit(0);
		}
		#endif
	}
	memset(macAdd, 0x00, sizeof(macAdd) );
	memset(sha3License, 0xFF, sizeof(sha3License) );
	lloop = 0;
	do
	{
		unsigned int i=0;

		/* Read a line from input file. */
		memset( tmpData, 0x00, sizeof(tmpData) );

		if( NULL == fgets( tmpData, MAC_BUF_SIZ, fi ) )
		{
			//fprintf(stderr,"temp Lic ended.. \n" );
			break;
		}

		/* Remove carriage return/line feed at the end of line. */
		i = strlen(tmpData);
		
		if(i >= MAC_BUF_SIZ)
		{
			fprintf(stderr,"\r\nERROR:Not enough Buffer length---(%u) \n\n", i );
		}

		if (--i > 0)
		{
			if (tmpData[i] == '\n') tmpData[i] = '\0';
			if (tmpData[i] == '\r') tmpData[i] = '\0'; 
			if (tmpData[i-1] == '\n') tmpData[i-1] = '\0';
			if (tmpData[i-1] == '\r') tmpData[i-1] = '\0'; 

			result = sscanf(tmpData, "%x-%x-%x-%x-%x-%x",
						&macAdd[lloop][0], &macAdd[lloop][1], &macAdd[lloop][2], &macAdd[lloop][3], &macAdd[lloop][4], &macAdd[lloop][5] );

			
			/* === 1 STEP : record (17 items check) =============== */
			if(MAC_ADDR_NUM==result)
			{
				//fprintf(stderr, "%d [%02x-%02x-%02x-%02x-%02x-%02x] \n", lloop, macAdd[lloop][0], macAdd[lloop][1], macAdd[lloop][2], macAdd[lloop][3], macAdd[lloop][4], macAdd[lloop][5] );
				

				// 3 uppercase + 3 lowercase
		#if 0
				if(fo) fprintf(fo, "%02X-%02X-%02X-%02x-%02x-%02x\n", 
							macAdd[lloop][0], macAdd[lloop][1], macAdd[lloop][2], macAdd[lloop][3], macAdd[lloop][4], macAdd[lloop][5] );
		#else
				sprintf(sha3License[lloop], "<shift><tp.joo@daum.net>%02X-%02X-%02X-%02x-%02x-%02x\0", 
							macAdd[lloop][0], macAdd[lloop][1], macAdd[lloop][2], macAdd[lloop][3], macAdd[lloop][4], macAdd[lloop][5] );
		#endif
		
				iOKc ++;
				lloop ++;
			}
			else if( (MAC_ADDR_NUM!=result) && (result!=-1 && i>0) ) 
			{
				iNGc ++;
				continue; /* reading Next item because of FAIL item */
			}

			if( lloop > MAC_ADDR_LINE_SIZ ) break;
			/* === 1 STEP : record (17 items check) =============== */
		}
	}
	while (!feof (fi));

	iFinal = lloop;
	
	if(fr) { fclose(fr); fr=NULL; }
	if(fi) { fclose(fi); fi=NULL; }
	//if(fo) { fclose(fo); fo=NULL; }

	//if( 0 != remove( HOSTPC_MAC_TEMP ) )
	//{
	//	//fprintf(stderr,"  Deleted temp file -> Failed \n");
	//}

	#if 1
	{
		//unsigned char sha3License[20] = {0,};
		char	sha3digestTxt[SHA3_OUTPUT_SIZ] = {0,};
		unsigned char sha3out[SHA3_OUTPUT_SIZ] = { 0, };
		
		unsigned __int64 	kll=0;
		size_t		ll=0;
		int 		ret, ii;

		memset( sha3out, 0x00, sizeof(sha3out) );
		memset( sha3digestTxt, 0x00, sizeof(sha3digestTxt) );

	#if 0
		if( NULL == (fi = fopen( HOSTPC_MAC_FILE, "rb")) ) 
		{
			fprintf(stderr,"\r\n++ERROR++:Can not read temp Lic file \n\n");
			exit(0);
		}
	#endif

		memset(sha3digestTxt, 0x00, sizeof(sha3digestTxt) ); // initialized
		iret = 0; 
		kll = 0UL;
		licOk = 0;
		lloop = 0;
		//while((ll = fread(sha3License, 15, sizeof(sha3License), fi)) > 0) 
		do {
			// Initialize the SHA3-512 context
			sha3_init(SHA3_512_HASH_BIT, SHA3_SHAKE_NONE);	

			//fscanf(fi, "%s", sha3License );
			ll = strlen( (char *)sha3License[lloop] );
			{
				//printf("[ll=%d[%s]]\n", lloop, sha3License[lloop]	);
				kll += ll;
				sha3_update(sha3License[lloop], ll);
			}
			
			ret = sha3_final(sha3out, SHA3_OUT_512);
			
			for (ii = 0; ii < SHA3_OUT_512; ii++)
			{
				sprintf(&sha3digestTxt[ii*2], "%02x", sha3out[ii]);
			}

			//if(outfile) fprintf(outfile,"%s", sha3digestTxt);
			//printf("[%s] \r\n", sha3digestTxt  );

			for(ii=0; ii<MAC_ADDR_LINE_SIZ; ii++)
			{
				if( ll <= 0 ) break;
				if( LicFile[ii][0] == '\0' || LicFile[ii][0]=='\n' || LicFile[ii][0]=='\r' ) 
				{
					LicFile[ii][0] = '\0';
					break;
				}

				if( 0==strncmp(sha3digestTxt, &LicFile[ii][4], SHA3_512_LICENSE_LEN ) ) 
				{
					licOk ++;
					iret = 0x8000; 
					//fprintf(stderr," %d [%s] \n",ii, sha3digestTxt );
				}
			}
			memset(sha3digestTxt, 0x00, sizeof(sha3digestTxt) ); // initialized
			lloop ++;

			if( lloop > MAC_ADDR_LINE_SIZ || lloop > iFinal) break;
		} while (1); // (!feof (fi));
		iret += licOk;

		if(fr) fclose(fr);
	}
	#endif

	#if 0
	if( iret ) 
	{
		fprintf(stderr," ------->> Licensed OK on the PC. (%#x)\n", iret);
		fprintf(stderr,"---------------------------------------------------------------\n" );
	}
	#endif
	
	return iret; /* License OK */

}




/* ---------------------------------------------------------------------------------------
 * main() function
 *
 * ---------------------------------------------------------------------------------------
 */

int main(int argc, char *argv[])
{ 
	struct stat file_statbuf; 
	int elf_size = 0;

	struct	_finddatai64_t iFiles; // c_file;
	struct	_finddatai64_t fexist; // 2020.06.18

	unsigned __int64     iTotSize = 0ULL;
	__int64     iFileNum = 0ULL;
	int     iFirst=-1, iLast=-1;
	int     isAsteMode = ASTERISK_UNKNOWN; /* initial value must be -1 */
	int  	isAsteminus = 0;
	long	retFiles;
	int 	multifileindex = 0, idx=0;
	int 	olen=0;

	unsigned int titlen=0;

	// index file merged
	int istartIdx=-1, iEndIdx=0, iTotcnt=0;
	int iret = 0;
	
	//char mergefile[50][128];
#define MERGE_TOTAL_CNT 			16
#define MERGE_INDEX_SIZ 			6 // 16
#define MERGE_DATE_SIZ 				26 // 32
#define MERGE_FILE_SIZ 				16
#define MERGE_FILENAME_LENGTH 		64
#define MERGE_SHA256_SIZ 			64
#define MERGE_MAX_FILES 			101

	typedef struct {
		int  mergeIndex;
		char mergeDate[MERGE_DATE_SIZ];
		char mergeFileName[MERGE_FILENAME_LENGTH];
		unsigned int  mergeSize;
		char mergeSHA256[MERGE_SHA256_SIZ];
	} _mergeFiles ;

	char mergeTotIndex[MERGE_TOTAL_CNT];
	char mergeTxtIndex[MERGE_INDEX_SIZ];
	char mergeTxtSize[MERGE_FILE_SIZ];
	char extractFile[MAX_FILENAME_LEN];
	
	_mergeFiles mFile[MERGE_MAX_FILES];
	struct _finddata_t mergeInfo;

// ======================================================================
#pragma pack(push, 1)                /* 구조체를 1바이트 크기로 정렬 */

	typedef struct {
		char mergeTxtIndex[MERGE_INDEX_SIZ];
		char mergeDate[MERGE_DATE_SIZ];
		char mergeFileName[MERGE_FILENAME_LENGTH];
		char mergeTxtSize[MERGE_FILE_SIZ];
		char mergeSHA256[MERGE_SHA256_SIZ+1];
	} _extractFiles ;

	_extractFiles  exFileInfo[MERGE_MAX_FILES];

#pragma pack(pop)

	char strExtract[EXTRACT_FILE_SIZE+1];
// ======================================================================


	
	unsigned int crc32_ctab[CRC32_TAB_SIZE] = {0,};

	int ret_scan = 0;
	int opt;
	unsigned char opt_ok = 0x0;
	int ii;
	int isJoin=0; // --join option

	int isFileMerge=0; // 2017.12.11

	int ismultiFilesMerge=0; // 2022.09.02
	int ismultiFilesExtract=0; // 2022.09.02
	
	int is2ndFileSize=0;
	int is2ndEndian=0; // 1:little, 2:big endian

	//int isAttach = 0;
	int isAttachVer = 0; /* ATT_VERSION or ATT_MCU_32BYTE */
	int isAttachBrd = 0; /* ATT_BOARD */
	int isAttachMdl = 0; /* ATT_MODEL */
	int isAttachCRC = 0; /* ATT_DATEorCRC */
	
	
	int isDelHdr = 0, isFillSize = 0; /* 2009.10.09, CRC Generator */
	int isCRC = 0, isCRCtype = 0;
	int insertCRC=0; // 2017.04.04
	int iUpper = -1;
	int isAppend = 0; /// over-write
	int isNKinform = 0, isCreateNB0=0;
	int isMot2bin = 0;
	int isIntel2bin = 0;
	int isElf2Bin = 0;
	int isRandomNum = 0;
	int iRanMaxi=0;
	unsigned int total_bin_size = 0;
	int c;
	int count=0;
	///int verbose=FALSE;
	size_t 	fr_size;
	int isFloat2Hex = 0;
	int isIgnoreBothFile = 0, isIgnoreInFile=0, isIgnoreOuFile=0;
	int isConvertBMP2C = 0, iRGB = 565;
	int isHash=0;
	int isMD2 = 0; 
	int isMD4 = 0;
	int isMD5 = 0;
	int isSHA1 = 0;
	int isSHA256 = 0; /// SHA2, 2014.06.30
	int isSHA384 = 0; /// SHA2, 2014.06.30
	int isSHA512 = 0; /// SHA2, 2014.06.30
	int isSHA224 = 0; /// 2014.07.31
	int isMD6    = 0; /// 2014.07.31
//#if SHA3_KECCAK_224_256_384_512
	int isSHA3_KECCAK_224 =0, isSHA3_KECCAK_256=0, isSHA3_KECCAK_384=0, isSHA3_KECCAK_512=0;
	int isShake128=0, isShake256=0;
	int isBlake224=0, isBlake256=0, isBlake384=0, isBlake512=0;
	int isRipeMD128 = 0, isRipeMD160 = 0;
//#endif
	int isIDEA = 0;
	int isMJD = 0; /// 2014.07.04
	int isBMPreverse = 0;
	
	unsigned int i_readCount = 0;
	int len_board_name = 0;
	int len_module_name = 0;
	int len_version_name = 0;
	int len_build_date = 0;
	int len_attach_hdr = 0; // 2020.07.07
	int len_checksum = 0;
	int iloop=0;

#if MODIFIED_JULIAN_DATE 
	double	MJDvalue;
	__double_hex_union	cHex;
	mjd_timestamp ymd_hms; /// To Modified Julian Date
	int iTMP=0;
#endif
	char str_cmBuf[COMMA_BUF_SIZE] = {0,};

#define NB0_EXT 			"nb0"

	char extfile_name[MAX_CHARS*LENGTH_OF_FILENAME+1];
	//char mulfile_name[MULTI_IN_FILES_CNT][MAX_CHARS*LENGTH_OF_FILENAME+1];
	//__int64 mulfile_size[MULTI_IN_FILES_CNT]; /// 2014.08.06
	char ttlfilename[MAX_FILENAME_LEN];
	char infile_name[MAX_CHARS*LENGTH_OF_FILENAME+1];
	char sefile_name[MAX_CHARS*LENGTH_OF_FILENAME+1]; // -i option 2nd file name
	__int64  infile_size; /// 2014.08.06
	__int64  sefile_size; /// 2017.04.05
	char outfile_name[MAX_CHARS*LENGTH_OF_FILENAME+1];

	char str_MCUversion[MAX_32CHARS+1]; // 2022-10-18

	char str_boardName[MAX_CHARS+1]; /* PI2000 */
	char str_moduleName[MAX_CHARS+1];
	char str_versionName[MAX_VERSION_LEN+1]; // 2017.12.12
	char str_buildDate[MAX_CHARS*2];
	char str_abuild_date[MAX_CHARS+1];
	char str_fillSiz[MAX_CHARS+1];
	char str_inform[MAX_CHARS+1];
	char str_nb0[MAX_CHARS+1];
	char str_help[MAX_CHARS+1];
	char str_BMPType[MAX_CHARS+1];
	char str_hash[MAX_CHARS+1];
	char str_float[MAX_CHARS*LENGTH_OF_FILENAME+1];
#if MODIFIED_JULIAN_DATE 
	char str_mjd[MAX_CHARS+1];
	char str_cur_date[MAX_CHARS+1];
#endif /// MODIFIED_JULIAN_DATE 
	
#ifdef _MOT2BIN_
	/* line inputted from file */
	char 	Mot2bin_Line[MOT2BIN_MAX_LINE_SIZE];
	unsigned char 	Mot2Bin_Data_Str[MOT2BIN_MAX_LINE_SIZE];
#endif
	char strZeroForced[MAX_CHARS+1];
	char str_dummy[MAX_CHARS+1];
	double	 f2h_float;

	char str_Verbos[MAX_CHARS+1];
	unsigned int iVerbosType = 0;

	char str_DelHdr[MAX_CHARS+1];
	char str_crcAdd[MAX_CHARS+1]; // 2017.04.04
	char str_FileMax[MAX_CHARS+1]; // 2017.04.04
	char str_SecFileSiz[MAX_CHARS+1]; // 2017.12.11
	char strHexType[MAX_CHARS+1]; 

	char str_ignore[MAX_CHARS+1]; 
	char strHex2BinLen[MAX_CHARS+1]; // 2020.07.07
	char strPadByte[MAX_CHARS+1];
//	char strPadArea[MAX_CHARS+1];

	unsigned int iFileLimit=0;
	unsigned int iHdr_len=0;
	/* -------------------------------------------------------------------- */
	/* -- getopt_long stores the option index here. -- */
	int option_index = 0;	
	static int verbose_flag;

#if SHIFT_QUALITY_DATA_SORTING /* 2022-11-13 */
	char str_ShiftOp[10][32+1]; 
	int isShift = 0;
	int isUpShift=0, isDownShift=0;	

	int iTCnt=0;
	short iModeID = -1; /* 8:md_NOR */
	short iPwrOnOff = -1; /* POWER ON, POWER OFF*/
	short itmpFileDeleted = 1;
	short gValueDisplay = 0;
	short iSBchoicePnt = 0;
	char currPath[PATH_MAX];
#endif //SHIFT_QUALITY_DATA_SORTING /* 2022-11-13 */


	static char strOpt[] = "g:ALz:h:b:m:v:i:o:a:c:d:F:I:N:S:f:B:M:J:l:ej:nE:P:kZR:y:x:p:D:N:"; // 2022.11.13

	static struct option long_options[] =
	{
		/* These options set a flag. */
		{"brief",		no_argument,	   &verbose_flag, 0},
		/* These options don't set a flag.	We distinguish them by their indices. */
		{"append",		required_argument, 0, 'a'}, /// Output (append) filename
		{"board",		required_argument, 0, 'b'}, /// Attach Header : Board Name(16byte)
		{"cinfo",		required_argument, 0, 'c'}, /// create current system date or insert checksum
		{"detach",		required_argument, 0, 'd'}, /// detach Header (16byte * 4 lines)
		{"elf",			no_argument,       0, 'e'}, /// elf
		{"float",		required_argument, 0, 'f'}, /// float number to hex-decimal
		{"ignore",		required_argument, 0, 'g'}, /// ignore input & output-file
		{"help",		required_argument, 0, 'h'}, /// help
		{"input",		required_argument, 0, 'i'}, /// input file name
		{"join",		required_argument, 0, 'j'}, // file merged!!  2017.04.05
		{"allpadarea", 	no_argument,       0, 'k'}, /// checksum type for hex2bin
		{"length",		required_argument, 0, 'l'}, /// Mot2bin, hex2bin : MaxSize
		{"model",		required_argument, 0, 'm'}, /// Attach Header : Module Name (16byte)
		{"alignword",	no_argument,       0, 'n'}, // intel family address alignment word ENABLE
		{"output",		required_argument, 0, 'o'}, /// output filename
		{"mcu",			required_argument, 0, 'p'}, // MCU version 32byte added
	//	{"file",		required_argument, 0, 'q'},
	//	{"reverse",		required_argument, 0, 'r'}, 
	//	{"size",		required_argument, 0, 's'}, // 2nd file size
	//	{"title",		required_argument, 0, 't'},
	//	{"up",			required_argument, 0, 'u'}, 
 		{"version", 	required_argument, 0, 'v'}, /// Attach Header : Version Name (16byte)
	//	{"file",		required_argument, 0, 'w'},
		{"extract",		required_argument, 0, 'x'},
		{"merge",		required_argument, 0, 'y'}, /// file merged
		{"verbose",		required_argument, 0, 'z'}, /// verbos --
	
		/// ------------------------------------------------------------------------------
		{"motorola",	no_argument,	   0, 'A'}, /// convert Motorola to binary
		{"bmp", 		required_argument, 0, 'B'}, /// bmp file
	//	{"file",		required_argument, 0, 'C'},
		{"downshift",	required_argument, 0, 'D'}, /* 2022/11/13 - Shift Quality Data Sorting - down */
		{"endian",		required_argument, 0, 'E'}, /* 0:little-endian,  1:big-endian */
		{"fillsize",	required_argument, 0, 'F'}, /// Fill Data (0xFF)
	//	{"CRC", 		required_argument, 0, 'G'}, 
	//	{"file",		required_argument, 0, 'H'},
		{"fileinform",	required_argument, 0, 'I'}, /// File Information in PC
		{"mjd",	        required_argument, 0, 'J'}, /// Modified Julian Date---
	//	{"file",		required_argument, 0, 'K'},
		{"intel",		no_argument,	   0, 'L'}, /// convert Intel Format to binary
		{"checksum",	required_argument, 0, 'M'}, /// checksum MD5, SHA1, SHA256, SHA384, SHA512--
		{"nk",			required_argument, 0, 'N'}, /// WinCE OS Kernel, nk.bin information
	//	{"file",		required_argument, 0, 'O'}, 
		{"padbyte",		required_argument, 0, 'P'}, // Padbyte 0xff
	//	{"file",		required_argument, 0, 'Q'},
		{"random",		required_argument, 0, 'R'},
		{"startaddr",	required_argument, 0, 'S'}, /// start address Hex2bin--
	//	{"file",		required_argument, 0, 'T'},
		{"upshift",		required_argument, 0, 'U'}, /* 2022/11/13 - Shift Quality Data Sorting - up */
	//	{"file",		required_argument, 0, 'V'},
	//	{"file",		required_argument, 0, 'W'},
	//	{"file",		required_argument, 0, 'X'},
	//	{"file",		required_argument, 0, 'Y'},
		{"zeroforced",	no_argument,       0, 'Z'},
	
		{0, 0, 0, 0}
	};
	/* -------------------------------------------------------------------- */



	/// -------------------------------------------
	/// --- buffer initialized -------------------------
	/// -------------------------------------------
	memset(extfile_name, 0x00, (MAX_CHARS*LENGTH_OF_FILENAME)+1 );
	//memset(mulfile_name, 0x00, (MULTI_IN_FILES_CNT*(MAX_CHARS*LENGTH_OF_FILENAME))+1 );
	//memset(mulfile_size, 0x00, sizeof(mulfile_size) );

	memset(ttlfilename, 0x00, MAX_FILENAME_LEN*sizeof(char));
	
	memset(infile_name,  0x00, (MAX_CHARS*LENGTH_OF_FILENAME)+1 );
	infile_size = 0ULL;
	memset(sefile_name,  0x00, (MAX_CHARS*LENGTH_OF_FILENAME)+1 );
	sefile_size = 0ULL;
	isJoin = 0;
	memset(outfile_name, 0x00, (MAX_CHARS*LENGTH_OF_FILENAME)+1 );

#if SHIFT_QUALITY_DATA_SORTING /* 2022-11-13 */
	memset( shift_file, 0x00, (MAX_CHARS*LENGTH_OF_FILENAME)+1 ); // 2022.11.19
	memset( shift_in, 0x00, (MAX_CHARS*LENGTH_OF_FILENAME)+1 ); // 2023.02.12
#endif


	memset(str_boardName,   0x00, (MAX_CHARS+1) );
	memset(str_moduleName,  0x00, (MAX_CHARS+1) );
	memset(str_versionName, 0x00, sizeof(str_versionName) );
	memset(str_buildDate,   0x00, (MAX_CHARS*2) );
	memset(str_abuild_date, 0x00, (MAX_CHARS+1) );
	memset(str_fillSiz,  0x00, (MAX_CHARS+1) ); /// 2014.07.04
	memset(str_inform,   0x00, (MAX_CHARS+1) );
	memset(str_nb0,      0x00, (MAX_CHARS+1) );
	memset(str_address,  0x00, (MAX_CHARS+1) );
	memset(str_help,     0x00, (MAX_CHARS+1) ); /// 2014.07.04

	memset(str_Verbos,   0x00, (MAX_CHARS+1) );
	memset(str_float,    0x00, (MAX_CHARS*LENGTH_OF_FILENAME)+1 );
	memset(str_BMPType,  0x00, (MAX_CHARS+1) ); /// 2013.07.16
	memset(str_hash,     0x00, (MAX_CHARS+1) ); /// 2014.06.26
#if MODIFIED_JULIAN_DATE 
	memset(str_mjd,      0x00, (MAX_CHARS+1) ); /// 2014.07.04
	memset(str_cur_date, 0x00, (MAX_CHARS+1) ); /// 2014.07.04
#endif /// MODIFIED_JULIAN_DATE 
	

#ifdef _MOT2BIN_
	memset(Mot2bin_Line,     0x00, sizeof(Mot2bin_Line) ); /// 2014.06.26
	memset(Mot2Bin_Data_Str, 0x00, sizeof(Mot2Bin_Data_Str) ); /// 2014.06.26
#endif

	memset(str_MCUversion, 0x00, sizeof(str_MCUversion) );

	memset( str_cmBuf, 0x00, sizeof(str_cmBuf) );
	memset( strHex2BinLen, 0x00, sizeof(strHex2BinLen) );


	memset( &cHex, 0x00, sizeof(__double_hex_union) );
	memset( &ymd_hms, 0x00, sizeof(mjd_timestamp) );

	memset( crc32_ctab, 0x00, sizeof(crc32_ctab) );
	memset( str_DelHdr, 0x00, sizeof(str_DelHdr) );

	memset( str_crcAdd, 0x00, sizeof(str_crcAdd) );
	memset( str_FileMax, 0x00, sizeof(str_FileMax) );

	memset( str_SecFileSiz, 0x00, sizeof(str_SecFileSiz) ); // 2017.12.11
	is2ndFileSize = 0;
	is2ndEndian = 0;

	memset( strHexType, 0x00, sizeof(strHexType) ); // 2020.06.20

	memset( strZeroForced, 0x00, sizeof(strZeroForced) ); // 2020.06.20

	memset( str_dummy, 0x00, sizeof(str_dummy) );
	memset( str_ignore, 0x00, sizeof(str_ignore) );

#if SHIFT_QUALITY_DATA_SORTING /* 2022-11-13 */
	memset( str_ShiftOp, 0x00, sizeof(str_ShiftOp) );
	memset( currPath, 0x00, PATH_MAX*sizeof(char) );
#endif

	/// -----------------------------------------------

	is2ndFileSize = 0;
	is2ndEndian = 0;

	/// -----------------------------------------------
	/// --- parameters initialized -------------------------
	/// -----------------------------------------------
	opt_ok          = 0; /* clear */

	//isAttach        = 0; 
	isAttachVer     = 0; /* ATT_VERSION or ATT_MCU_32BYTE */
	isAttachBrd     = 0; /* ATT_BOARD */
	isAttachMdl     = 0; /* ATT_MODEL */
	isAttachCRC     = 0; /* ATT_DATEorCRC */
	
	isDelHdr        = 0; /* default : Adding Header */
	isIgnoreBothFile= 0; /// ignore input & output
	isIgnoreInFile  = isIgnoreOuFile = 0; // 2020.06.12

	isCRC           = 0; /* CRC Generator */
	isCRCtype       = HDR_CRC_UNKNOWN;
	insertCRC       = 0;
	iUpper          = 0;

	isFillSize      = 0;
	isNKinform      = 0; /* NK information */
	isCreateNB0     = 0;
	isMot2bin       = 0;
	isIntel2bin     = 0;
	isElf2Bin       = 0;
	isAppend        = 0; /// output file : over-write
	isFloat2Hex     = 0;
	isConvertBMP2C  = 0;

	isHash          = 0;
	isMD2           = 0; /// MD2 off
	isMD4           = 0; /// MD4 off
	isMD5           = 0;
	isSHA1          = 0;
	isSHA256        = 0; /// SHA2, 2014.06.30
	isSHA384        = 0; /// SHA2, 2014.06.30
	isSHA512        = 0; /// SHA2, 2014.06.30
	isMD6           = 0; /// 2014.07.31
    isSHA224        = 0; /// 2014.07.31
	isIDEA          = 0; /// IDEA, 2014.07.25

#if SHA3_KECCAK_224_256_384_512
	isSHA3_KECCAK_224 = isSHA3_KECCAK_256 = isSHA3_KECCAK_384 = isSHA3_KECCAK_512 = 0;
	isShake128 = isShake256 = 0;
#endif

	isRipeMD128 = isRipeMD160 = 0;

	isBlake224 = isBlake256 = isBlake384 = isBlake512 = 0;
	isRandomNum = 0;

	isMJD           = 0; /// 2014.07.04
	olen            = 0; /// 2014.07.25
	isBMPreverse    = 0; /// 2015.02.12

	verbose                 = FALSE;
	Starting_Address_Setted = FALSE;
	multifileindex          = 0;

	Minimum_Block_Size_Setted = 0;
	Starting_Address_Setted   = 0;
	Floor_Address_Setted      = 0;
	Ceiling_Address_Setted    = 0;
	Max_Length_Setted         = 0;
	Swap_Wordwise             = 0;
	Address_Alignment_Word    = 0;
	Batch_Mode                = 0;
	Enable_Checksum_Error     = 1; // Enable checksum always ON!!

	Cks_range_set             = 0;
	Cks_Addr_set              = 0;
	Force_Value               = 0;


	iFirst = iLast = -1; /* initial value must be -1 */
	isAsteMode     = ASTERISK_UNKNOWN; /* initial value must be -1 */
	isAsteminus    = 0;
	isPadByte = isPadByteAllArea = 0;
	Enable_HexaAddr_Zero_Forced  = HEX2BIN_REAL_ADDR; // CASE of default & no option

	/// -----------------------------------------------


	help_brief();


	#if 1 /* 2022012-07 */	
	if ( getcwd(currPath, PATH_MAX) == NULL ) 
	{
		fprintf(stderr, "\r\n++[ERROR}++ can not check current directory!! \n\n") ;
		exit(EXIT_FAILURE); 
	} 
	//fprintf(stderr, "Current Directory: %s\n", currPath) ;
		

	// ================================================================
	iret = CheckLicense();
	if( iret )
	{
		// License OK~~
		//fprintf(stderr,"\n");		
		fprintf(stderr," --->> Acquired the License on the PC... (%#x) \n", iret);
		fprintf(stderr,"---------------------------------------------------------------\n" );
	}
	else
	{
		beep(700,100);
		AllFilesClosed();
		fprintf(stderr,"\r\n [LICENSED] License is required... \n\n");
		exit(EXIT_FAILURE); /* 0: EXIT_SUCCESS, 1: EXIT_FAILURE */
		return EXIT_FAILURE;
	}
	// ================================================================
	#endif


	/* ---------------------------------------------------------------------
	A (MOTOROLA) : convert hex to Motorola bin
	L (INTEL)    : convert hex to Intel bin
	------------------------------------------------------------------------*/


	while( EOF != (opt = getopt_long(argc, argv, strOpt, long_options, &option_index)) ) 
	{

		switch(opt) 
		{ 

		#if MODIFIED_JULIAN_DATE 
			case 'J' : /// 2014.06.27,  Modified Julian Date--

				if(optarg) 
				{
					memcpy(str_mjd, optarg, MAX_CHARS);
					olen = strlen(str_mjd);

					if( 0==strcasecmp(str_mjd, "test" )  )
					{
						printf("\n---------------- MJD (Modified Julian Date) ----------------- \n");
						

						/// --- Test ------
						/// --- test #1
						cHex.dbl_val = MJDvalue = 50000.000000;
						ymd_hms = Convert2Timestamp( MJDvalue );

						printf("MJD:[ %s ] [", commify(MJDvalue, str_cmBuf, 6)  );
					#if CONVERT_HEX==CONVERT_HEX_MSB
						for(iTMP=0; iTMP<sizeof(double); iTMP++)
						{
							printf("%02X", cHex.dbl_bin[iTMP]);
							if(iTMP<sizeof(double)-1) printf("-");
						}
					#elif CONVERT_HEX==CONVERT_HEX_LSB
						for(iTMP=sizeof(double)-1; iTMP>=0; iTMP--)
						{
							printf("%02X", cHex.dbl_bin[iTMP]);
							if(iTMP>0) printf("-");
						}
					#endif /// CONVERT_HEX_MSB

						printf("] -> DATE:[ %d/%02d/%02d, %02d:%02d:%02d:%03d ] \n", 
									ymd_hms.m_year, ymd_hms.m_month, ymd_hms.m_day, 
									ymd_hms.m_hour, ymd_hms.m_mins,  ymd_hms.m_secs, ymd_hms.m_millis );

						/// --- test #2
						cHex.dbl_val = MJDvalue = 56551.338982;
						ymd_hms = Convert2Timestamp( MJDvalue );

						printf("MJD:[ %s ] [", commify(MJDvalue, str_cmBuf, 6)  );
					#if CONVERT_HEX==CONVERT_HEX_MSB
						for(iTMP=0; iTMP<sizeof(double); iTMP++)
						{
							printf("%02X", cHex.dbl_bin[iTMP]);
							if(iTMP<sizeof(double)-1) printf("-");
						}
					#elif CONVERT_HEX==CONVERT_HEX_LSB
						for(iTMP=sizeof(double)-1; iTMP>=0; iTMP--)
						{
							printf("%02X", cHex.dbl_bin[iTMP]);
							if(iTMP>0) printf("-");
						}
					#endif /// CONVERT_HEX_MSB

						printf("] -> DATE:[ %d/%02d/%02d, %02d:%02d:%02d:%03d ] \n", 
									ymd_hms.m_year, ymd_hms.m_month, ymd_hms.m_day, 
									ymd_hms.m_hour, ymd_hms.m_mins,  ymd_hms.m_secs, ymd_hms.m_millis );

						/// --- test #3
						cHex.dbl_val = MJDvalue = 15020.000000;
						ymd_hms = Convert2Timestamp( MJDvalue );

						printf("MJD:[ %s ] [", commify(MJDvalue, str_cmBuf, 6)  );

					#if CONVERT_HEX==CONVERT_HEX_MSB
						for(iTMP=0; iTMP<sizeof(double); iTMP++)
						{
							printf("%02X", cHex.dbl_bin[iTMP]);
							if(iTMP<sizeof(double)-1) printf("-");
						}
					#elif CONVERT_HEX==CONVERT_HEX_LSB
						for(iTMP=sizeof(double)-1; iTMP>=0; iTMP--)
						{
							printf("%02X", cHex.dbl_bin[iTMP]);
							if(iTMP>0) printf("-");
						}
					#endif /// CONVERT_HEX_MSB

						printf("] -> DATE:[ %d/%02d/%02d, %02d:%02d:%02d:%03d ] \n", 
									ymd_hms.m_year, ymd_hms.m_month, ymd_hms.m_day, 
									ymd_hms.m_hour, ymd_hms.m_mins,  ymd_hms.m_secs, ymd_hms.m_millis );


						if( (outfile = fopen( "number.mjd", "a+")) == NULL )  /// test sample
						{
							beep(700,100);
							myerror("\n\n[++ERROR++] Can not create output file (number.mjd). \n");

							AllFilesClosed();
							exit(EXIT_FAILURE); /* 0: EXIT_SUCCESS, 1: EXIT_FAILURE */
							return 0;
						}

						if( outfile )
						{
							srand( (unsigned)time(NULL)+(unsigned)getpid() );
							for ( idx = 0; idx < MJD_SAMPLE_NUMBER; idx++)
							{
								if(outfile) fprintf(outfile, "%lf \n", (float)( rand()+ (float)(rand()/MATH_PI)) );
							}
							printf("\nThe sample file (number.mjd) is created.. \n" );
						}

						beep(700,100);
						AllFilesClosed();
						
						exit(EXIT_SUCCESS); /* 0: EXIT_SUCCESS, 1: EXIT_FAILURE */
						return 0;

					}
					else if( 0==strcasecmp(str_mjd, "current" ) )  
					{
						time_t	MJD_t;
						struct tm *mjdt;

						printf("\n---------------- MJD (Modified Julian Date) ----------------- \n");
			

						time( &MJD_t );
						mjdt = localtime( &MJD_t ); // time_t	형식으로 변환합니다.
						
						ymd_hms.m_year   = mjdt->tm_year+1900;
						ymd_hms.m_month  = mjdt->tm_mon+1;
						ymd_hms.m_day    = mjdt->tm_mday;
						ymd_hms.m_hour   = mjdt->tm_hour;
						ymd_hms.m_mins   = mjdt->tm_min;
						ymd_hms.m_secs   = mjdt->tm_sec;
						ymd_hms.m_millis = 0;
						
						cHex.dbl_val = MJDvalue = Convert2MJD( ymd_hms );

						printf("DATE:[ %d/%02d/%02d, %02d:%02d:%02d:%03d ] -> MJD:[ %s ] [", 
								ymd_hms.m_year, ymd_hms.m_month, ymd_hms.m_day, 
								ymd_hms.m_hour, ymd_hms.m_mins,  ymd_hms.m_secs, ymd_hms.m_millis, commify(MJDvalue, str_cmBuf, 6)  );

					#if CONVERT_HEX==CONVERT_HEX_MSB
						for(iTMP=0; iTMP<sizeof(double); iTMP++)
						{
							printf("%02x", cHex.dbl_bin[iTMP]);
							if(iTMP<sizeof(double)-1) printf("-");
						}
					#elif CONVERT_HEX==CONVERT_HEX_LSB
						for(iTMP=sizeof(double)-1; iTMP>=0; iTMP--)
						{
							printf("%02x", cHex.dbl_bin[iTMP]);
							if(iTMP>0) printf("-");
						}
					#endif /// CONVERT_HEX_MSB	
						printf("]\n");
					
						if( (outfile = fopen( "date.mjd", "a+")) == NULL )  /// test sample
						{
							beep(700,100);
							myerror("\n\n[++ERROR++] Can not create output file (date.mjd). \n");
							AllFilesClosed();

							exit(0); /// help();
							return 0;
						}
					

						if( outfile )
						{
							int iYear;
							srand( (unsigned)time(NULL)+(unsigned)getpid() );
							for ( idx = 0; idx < MJD_SAMPLE_NUMBER;  )
							{
								/// *** -> 00000.000000  ---> [ 1858, 11, 17, 0, 0, 0 0 ] 
								iYear = rand()%10000;
								if(iYear >= 1858 )
								{
									if(outfile) fprintf(outfile, "%12d %02d %02d %02d %02d %02d \n", 
												iYear, rand()%13, rand()%32, rand()%24, rand()%60, rand()%60  );
									idx++;
								}
								else
								{
									/// idx --;
								}
							}
							printf("\nThe sample file (date.mjd) is created.. (refer:1858/00/00)\n" );
						}

						beep(700,100);
						AllFilesClosed();

						exit(0);
						return 0;
										
					}
					else if( 0==strcasecmp(str_mjd, "date" ) )  
					{
						isMJD = 1; /// to date
					}
					else if( 0==strcasecmp(str_mjd, "mjd" ) )  
					{
						isMJD = 2; /// to MJD
					}
					else
					{
						myerror("\nMJD>> WARNING:Wrong option. --mjd [date|mjd]. Check option.\n");

						beep(700,100);
						AllFilesClosed();

						exit(0);
						return 0;
					}
				}
				else
				{
					myerror("\nMJD>> WARNING:option error. check option --mjd [date|mjd]. \r\n");

					beep(700,100);
					AllFilesClosed();

					exit(0);
					return 0;
				}
				break;
		#endif /// MODIFIED_JULIAN_DATE -------------

		#if CONVERT_BMP2C
			case 'B' : /// Convert BMP file to C text file
				if(optarg) 
				{
					olen = 0;
					isConvertBMP2C = 1;
					memcpy(str_BMPType, optarg, MAX_CHARS);
					olen = strlen(str_BMPType);
					
					if( 0 == strncmp(str_BMPType, "888", olen ) )
						iRGB = 888;
					else if( 0 == strncmp(str_BMPType, "444", olen ) )
						iRGB = 444;
					else if( 0 == strncmp(str_BMPType, "555", olen ) )
						iRGB = 555;
					else if( 0 == strncmp(str_BMPType, "565", olen ) )
						iRGB = 565;
					else
					{
						myerror("RGB Type : wrong option [%s] \n", str_BMPType);

						beep(700,100);
						AllFilesClosed(); // 2020.07.10
						exit(0); /// 2017.11.21

						return 0;
					}
						
				}
				else
				{
					printf("RGB Type ??? [%s] \n", str_BMPType);

					beep(700,100);
					AllFilesClosed();

					exit(0);
					return 0;
				}
				break;
		#endif /// CONVERT_BMP2C


		#if MD5_CHECKSUM_ENCIPHER 
			case 'M' : /// 2014.06.27, MD5/SHA1/SHA256/SHA384/SHA512 Checksum
				if(optarg) 
				{
					isHash    = 1;

					// initialized
					isMD2	  = 0; /// MD2 off
					isMD4	  = 0; /// MD4 off
					isMD5	  = 0; ///  MD5 on
					isMD6	  = 0;				
					isSHA1	  = 0; /// SHA1 off
					isSHA256  = 0; /// SHA2, 256 off
					isSHA384  = 0; /// SHA2, 384 off 
					isSHA512  = 0; /// SHA2, 512 off 
					isSHA224  = 0; /// 2014.07.31
					isIDEA	  = 0; /// 2014.07.26
					isCRC	  = 0;
					isCRCtype = 0;
					
					isSHA3_KECCAK_224 = 0;
					isSHA3_KECCAK_256 = 0;
					isSHA3_KECCAK_384 = 0;
					isSHA3_KECCAK_512 = 0; // 2020.06.11
					isShake128 = 0;
					isShake256 = 0;
					isRipeMD128 = 0;
					isRipeMD160 = 0;

					isBlake224 = 0; // 2020.0721
					isBlake256 = 0;
					isBlake384 = 0;
					isBlake512 = 0;

					memcpy(str_hash, optarg, MAX_CHARS);
					olen = strlen(str_hash);

					if( 0==strcasecmp(str_hash, "MD5") )
					{
						isMD5    = 1; ///  MD5 on
						iUpper   = isupper(str_hash[0]); // 2017.04.27
					}
					else if( 0==strcasecmp(str_hash, "SHA1") )
					{
						isSHA1   = 1; /// SHA1 on
						iUpper   = isupper(str_hash[0]); // 2017.04.27
					}
					else if( 0==strcasecmp(str_hash, "SHA224") )
					{
						isSHA224 = 1; /// 2014.07.31
						iUpper   = isupper(str_hash[0]); // 2017.04.27
					}
					else if( 0==strcasecmp(str_hash, "SHA256") )
					{
						isSHA256 = 1; /// SHA2, 256 on
						iUpper   = isupper(str_hash[0]); // 2017.04.27
					}
					else if( 0==strcasecmp(str_hash, "SHA384") )
					{
						isSHA384 = 1; /// SHA2, 384 on
						iUpper   = isupper(str_hash[0]); // 2017.04.27
					}
					else if( 0==strcasecmp(str_hash, "SHA512") )
					{
						isSHA512 = 1; /// SHA2, 512 on
						iUpper   = isupper(str_hash[0]); // 2017.04.27
					}
					else if( 0==strcasecmp(str_hash, "MD4") )
					{
						isMD4    = 1; /// MD4 on
						iUpper   = isupper(str_hash[0]); // 2017.04.27
					}
					else if( 0==strcasecmp(str_hash, "MD2") )
					{
						isMD2    = 1; /// MD2 on
						iUpper   = isupper(str_hash[0]); // 2017.04.27
					}
					else if( 0==strcasecmp(str_hash, "MD6") )
					{
						isMD6    = 1; /// MD6 on
						iUpper   = isupper(str_hash[0]); // 2017.04.27
					}
					else if( 0==strcasecmp(str_hash, "SHA3-224") )
					{
						isSHA3_KECCAK_224 = 1;
						iUpper   = isupper(str_hash[0]); // 2017.04.27
					}
					else if( 0==strcasecmp(str_hash, "SHA3-256") )
					{
						isSHA3_KECCAK_256 = 1;
						iUpper   = isupper(str_hash[0]); // 2017.04.27
					}
					else if( 0==strcasecmp(str_hash, "SHA3-384") )
					{
						isSHA3_KECCAK_384 = 1;
						iUpper   = isupper(str_hash[0]); // 2017.04.27
					}
					else if( 0==strcasecmp(str_hash, "SHA3-512") )
					{
						isSHA3_KECCAK_512 = 1;
						iUpper   = isupper(str_hash[0]); // 2017.04.27
					}
					else if( 0==strcasecmp(str_hash, "SHAKE128") )
					{
						isShake128 = 1;
						iUpper   = isupper(str_hash[0]); // 2017.04.27
					}
					else if( 0==strcasecmp(str_hash, "SHAKE256") )
					{
						isShake256 = 1;
						iUpper   = isupper(str_hash[0]); // 2017.04.27
					}
				#if defined(RIPEMD160) || defined(RIPEMD128)	
					else if( 0==strcasecmp(str_hash, "RipeMD128") || (0==strcasecmp(str_hash, "RMD128")) )
					{
						isRipeMD128 = 1;
						iUpper   = isupper(str_hash[0]);
					}
					else if( 0==strcasecmp(str_hash, "RipeMD160") || (0==strcasecmp(str_hash, "RMD160")) )
					{
						isRipeMD160 = 1;
						iUpper   = isupper(str_hash[0]);
					}
				#endif
					else if( 0==strcasecmp(str_hash, "crc16") )
					{
						isCRC	  = 1;
						isCRCtype = HDR_CRC16;
						iUpper    = isupper(str_hash[0]); // 2017.04.27
					}
					else if( 0==strcasecmp(str_hash, "crc16ksc") )
					{
						isCRC	  = 1;
						isCRCtype = HDR_KSC_CRC16;
						iUpper    = isupper(str_hash[0]); // 2017.04.27
					}
					else if( 0==strcasecmp(str_hash, "crc16c") )
					{
						isCRC	  = 1;
						isCRCtype = HDR_CRC16CCITT;
						iUpper    = isupper(str_hash[0]); // 2017.04.27
					}
					else if( 0==strcasecmp(str_hash, "crc32") )
					{
						isCRC	  = 1;
						isCRCtype = HDR_CRC32;
						iUpper    = isupper(str_hash[0]); // 2017.04.27
					}
					else if( 0==strcasecmp(str_hash, "crc64") )
					{
						isCRC	  = 1;
						isCRCtype = HDR_CRC64;
						iUpper    = isupper(str_hash[0]); // 2017.04.27
					}
					else if( 0==strcasecmp(str_hash, "crc64isc") )
					{
						isCRC	  = 1;
						isCRCtype = HDR_CRC64_ISC;
						iUpper	  = isupper(str_hash[0]); // 2020.07.16
					}
					else if( 0==strcasecmp(str_hash, "adler32") ) 
					{
						isCRC	  = 1;
						isCRCtype = HDR_ADLER32;
						iUpper    = isupper(str_hash[0]); // 2017.04.27
					}
					else if( 0==strcasecmp(str_hash, "joaat") ) 
					{
						isCRC	  = 1;
						isCRCtype = HDR_JOAAT;
						iUpper    = isupper(str_hash[0]);
					}

				#if BLAKE_224_256_384_512_HASH // 2020.07.23
					else if( 0==strcasecmp(str_hash, "blake224") )
					{
						isBlake224 = 1;
						iUpper   = isupper(str_hash[0]);
					}
					else if( 0==strcasecmp(str_hash, "blake256") )
					{
						isBlake256 = 1;
						iUpper   = isupper(str_hash[0]);
					}
					else if( 0==strcasecmp(str_hash, "blake384") )
					{
						isBlake384 = 1;
						iUpper   = isupper(str_hash[0]);
					}
					else if( 0==strcasecmp(str_hash, "blake512") )
					{
						isBlake512 = 1;
						iUpper   = isupper(str_hash[0]); 
					}
				#endif

					else
					{
						multifileindex = 0; /// only one input file---
						isMD2    = 0; /// MD2 off
						isMD4	 = 0; /// MD4 off
						isMD5	 = 0; /// MD5 off
						isMD6    = 0;
						isSHA1	 = 0; /// SHA1 off
						isSHA224 = 0; /// 2014.07.31
						isSHA256 = 0; /// SHA2, 256 off
						isSHA384 = 0; /// SHA2, 384 off 
						isSHA512 = 0; /// SHA2, 512 off 
						isIDEA	 = 0; /// 2014.07.26

						isCRC	  = 0;
						isCRCtype = HDR_CRC_UNKNOWN;
						iUpper    = 0; // 2017.04.27
						isSHA3_KECCAK_224 = isSHA3_KECCAK_256 = isSHA3_KECCAK_384 = isSHA3_KECCAK_512 = 0; // 2020.06.11
						isShake128 = isShake256 = 0;
						isBlake224 = isBlake256 = isBlake384 = isBlake512 = 0;
						isRipeMD128 = 0;
						isRipeMD160 = 0;

						printf("\n\nWarning: -M or --checksum MD2|MD4|MD5|MD6 \n");
						printf("                          SHA1|SHA224|SHA256|SHA384|SHA512 \n");
						printf("                          SHA3-224|SHA3-256|SHA3-384|SHA3-512|SHAKE128|SHAKE256 \n");
						printf("                          RipeMD128|RipeMD160 \n");
						printf("                          crc16|crc16c|crc32|crc64|adler32 \n");
						printf("                          CRC16|CRC16C|CRC32|CRC64|ADLER32 \n");

						beep(700,100);

						AllFilesClosed();
						exit(0);

						return 0;
					
					}

				}
				else
				{
					multifileindex = 0; /// only one input file---
					isMD2 = isMD4 = isMD5 = isMD6 = 0;
					isSHA1   = 0; /// SHA1 off
					isSHA224 = isSHA256 = isSHA384 = isSHA512 = 0; /// SHA2 OFF
					isIDEA	 = 0; /// 2014.07.26
					isSHA3_KECCAK_224 = isSHA3_KECCAK_256 = isSHA3_KECCAK_384 = isSHA3_KECCAK_512 = 0; // 2020.06.11
					isShake128 = isShake256 = 0;
					isBlake224 = isBlake256 = isBlake384 = isBlake512 = 0;
					isRipeMD128 = 0;
					isRipeMD160 = 0;

					isCRC     = 0;
					isCRCtype = HDR_CRC_UNKNOWN;
					iUpper	  = 0; // 2017.04.27

					printf("\n\nWarning: -M or --checksum MD2|MD4|MD5|MD6 \n");
					printf("                          SHA1|SHA224|SHA256|SHA384|SHA512 \n");
					printf("                          SHA3-224|SHA3-256|SHA3-384|SHA3-512|SHAKE128|SHAKE256 \n");
					printf("                          RipeMD128|RipeMD160 \n");
					printf("                          crc16|crc16c|crc32|crc64|adler32 \n");
					printf("                          CRC16|CRC16C|CRC32|CRC64|ADLER32 \n");

					beep(700,100);

					AllFilesClosed();

					exit(0);
					return 0;

				}
				break;
		#endif /// MD5_CHECKSUM_ENCIPHER -------------


			case 'g' : /* ignore inputfile and output file */

				if(optarg) 
				{
					memcpy(str_ignore, optarg, sizeof(str_ignore) );

					if( 0 == strcasecmp (str_ignore, "both" ) )
						isIgnoreBothFile = 1;
					else if( 0 == strcasecmp (str_ignore, "input" ) )
						isIgnoreInFile = 1;
					else if( 0 == strcasecmp (str_ignore, "output" ) )
						isIgnoreOuFile = 1;
				}

				break;


			case 'f' : /* convert float number to Hex-decial for using DR_GPS Trimble packet */

				olen = 0;
				isFloat2Hex = 1;
				//printf("str_float=[%s] argc=%d, argv[3]=%s, argv[4]=%s, argv[5]=%s argv[6]=%s \n", 
				//	str_float, argc, argv[3], argv[4], argv[5], argv[6] );

				if(optarg) 
				{
					memcpy(str_float, optarg, MAX_CHARS*LENGTH_OF_FILENAME );
					f2h_float = atof(str_float);
				}
				else
				{
					myerror("\r\n WARNING:wrong option --float [float number]. \n\n");

					beep(700,100);
					AllFilesClosed();
					exit(0);
			
					return 0;
				}

				#if 0
				switch(argc)
				{
					case 5:
						if(optarg) 
						{
							memcpy(str_float, optarg, MAX_CHARS*LENGTH_OF_FILENAME );
							f2h_float = atof(str_float);
						}
						else
						{
							printf("\n\n WARNING:wrong option --float [float number]. \r\n");

							beep(700,100);
							AllFilesClosed(); // 2020.07.10
							exit(0); /// 2017.11.21

							return 0;
						}
						break;

					case 6:
						if(argv[3]) 
						{
							memcpy(str_float, argv[3], MAX_CHARS*LENGTH_OF_FILENAME );
							olen = strlen(str_float);
						}

						if( 0 == strcasecmp (str_float, "sin" ) )
						{
							//printf("  sin(%s) ", argv[4] );
							f2h_float = atof( argv[4] );
							f2h_float = (f2h_float*MATH_PI)/180.0;
							f2h_float = sin(f2h_float);
						}
						else if( 0 == strcasecmp (str_float, "cos") )
						{
							//printf("  cos(%s) ", argv[4] );
							f2h_float = atof( argv[4] );
							f2h_float = (f2h_float*MATH_PI)/180.0;
							f2h_float = cos(f2h_float);
						}
						else if( 0 == strcasecmp (str_float, "tan") )
						{
							//printf("  tan(%s) ", argv[4] );
							f2h_float = atof( argv[4] );
							f2h_float = (f2h_float*MATH_PI)/180.0;
							f2h_float = tan(f2h_float);
						}
						else
						{
							printf("\n WARNING:float2hex: invalid format! [%s] \n", str_float );

							beep(700,100);
							AllFilesClosed(); // 2020.07.10
							exit(0); /// 2017.11.21

							return 0;
						}
						break;

					default:
						break;
				}
				#endif
				break;

		    case 'h' : /* help */
				if(optarg)
				{
					memcpy(str_help, optarg, MAX_CHARS);
					olen = strlen(str_help);
					
					if( 0 == strcasecmp(str_help, "intel" ) )
						help_int2bin_form();
					else if( 0 == strcasecmp(str_help, "motorola" ) )
						help_mot2bin_form();
					else
					{
						isIgnoreBothFile = 1; // 2020.06.08
						help();
					}
				}
				else
				{
					help();
				}


				AllFilesClosed(); // 2020.07.10
				exit(0);
				return 0;

			case 'S': /* --startaddr : Starting Address hex2bin */

				printf("\n");
				if( (isMot2bin!=1) && (isIntel2bin!=1) )
				{
					myerror("[++ERROR Starting Address++] Need option --intel or --motorola \n");

					beep(700,100);
					AllFilesClosed(); // 2020.07.10

					exit(0); /// 2017.11.21

					return 0;					
					break;
				}

				if(optarg) 
				{
					memcpy(str_address, optarg, MAX_CHARS);
					olen = strlen(str_address);

					if( olen > MAX_CHARS )
					{
						printf("\n\n[++ERROR++] Start Address(Hex) is too long (%d).. Max:16 \n\n", olen );
					}
					
					ret_scan = sscanf(str_address,"%x",&Starting_Address);
					if( 1!=ret_scan )
					{
						printf("\n\n[++ERROR++] str_address is wrong. It must be Hex Value \n\n" );
					}

					Starting_Address_Setted = TRUE;

			        printf("\nHEX2BIN>> Start address : %s (0x%X) \n", str_address, Starting_Address);
				}
				else
				{
					printf("\nHEX2BIN>> WARNING:wrong option --start [hexa value]. check option\r\n");

					beep(700,100);
					AllFilesClosed(); // 2020.07.10
					exit(0); /// 2017.11.21

					return 0;
				}
				break;

			case 'e': /// elf 2 bin
				isElf2Bin = 1;
				break;

		    case 'A': /* convert mot2bin -> Motorola FORMAT family --- */
				fprintf(stderr,"\n");
				fprintf(stderr,">>Hex family type : MOTOROLA family");

				if( 1==isIntel2bin ) // error
				{
					fprintf(stderr,"\n");
					fprintf(stderr,"[++ERROR Motorola++] Choice one --motorola or --intel option \n");

					beep(700,100);
					AllFilesClosed(); // 2020.07.10
					exit(0); /// 2017.11.21

					return 0;
					break;
				}

				isMot2bin=1;

				break;
				
		    case 'L': /* convert intel2bin -> Intel FORMAT family --- */
				fprintf(stderr,"\n");
				fprintf(stderr,">>Hex family type : Intel family");

				if( 1==isMot2bin ) // Error
				{
					fprintf(stderr,"\n");
					fprintf(stderr,"[++ERROR Intel++] Choice one --intel or --motorola option \n");
				
					beep(700,100);
					AllFilesClosed(); // 2020.07.10
					exit(0); /// 2017.11.21

					return 0;
					break;
				}

				isIntel2bin=1;
				break;

		    case 'n': /* --alignword : Address Alignment Word -> Intel family only --- */
				if( 1==isMot2bin ) 
				{
					fprintf(stderr,"\n");
					fprintf(stderr,"[++ERROR ALIGNWORD++] Need option --intel option only \n");
				
					beep(700,100);
					AllFilesClosed(); // 2020.07.10
					exit(0); /// 2017.11.21

					return 0;
					break;
				}

				if( 1==isIntel2bin )
				{
					fprintf(stderr,">>Addr Alignment   : Enable address alignment word in Intel family");
					Address_Alignment_Word = 1; // true;
				}
				break;
				
			case 'P': // --padbyte

				fprintf(stderr,"\n");
				if( (isMot2bin!=1) && (isIntel2bin!=1) && (1!=isFileMerge) && (1!=isFillSize) )
				{
					myerror("[++ERROR PADBYTE++] Need option --intel or --motorola or --fillsize [size] \n");

					beep(700,100);
					AllFilesClosed(); // 2020.07.10
					exit(0); /// 2017.11.21
					return 0;					
					break;
				}

				Pad_Byte = 0xff;
				if(optarg) 
				{
					memset( strPadByte, 0x00, sizeof(strPadByte) );
					memcpy( strPadByte, optarg, MAX_CHARS);
					olen = strlen(strPadByte);

					isPadByte = 1;
					Pad_Byte = GetHex( strPadByte );

					fprintf(stderr,">>Padding Byte     : 0x%x (default:ff) ", Pad_Byte);
				}
				else
				{
					myerror("\n\n WARNING:wrong option --padbyte [hexa value]. check option [%s] \r\n", strPadByte);

					beep(700,100);
					AllFilesClosed(); // 2020.07.10
					exit(0); /// 2017.11.21
					return 0;
				}
				break;

			case 'E': // --endian

				if( (isMot2bin!=1) && (isIntel2bin!=1) && (isFileMerge!=1) )
				{
					fprintf(stderr,"\n");
					fprintf(stderr,"[++ERROR Endian++] Need option --intel or --motorola or --join \n");
					beep(700,100);
					AllFilesClosed(); // 2020.07.10
					exit(0); /// 2017.11.21

					return 0;
					break;
				}

				if(optarg) 
				{
					Endian = 0; // default little for INTEL or MOTOROLA family

					is2ndFileSize = 0;
					is2ndEndian = 0;

					memset(str_SecFileSiz, 0x00, sizeof(str_SecFileSiz) );
					memcpy(str_SecFileSiz, optarg, MAX_CHARS);

					fprintf(stderr,"\n");
					if( 0==strcasecmp( str_SecFileSiz, "little" ) ) 
					{
						if(isFileMerge)
						{
							is2ndFileSize = 1;
							is2ndEndian = ENDIAN_LITTLE; // little
							fprintf(stderr,">>Endian           : Little endian");
						}
						else // For INTEL Hex2bin or MOTOROLA hex2bni
						{
							fprintf(stderr,">>Endian for CRC   : little endian");
							Endian = 0; // little
						}
					}
					else if( 0==strcasecmp( str_SecFileSiz, "big" ) ) 
					{
						if(isFileMerge)
						{
							is2ndFileSize = 1;
							is2ndEndian = ENDIAN_BIG; // big
							fprintf(stderr,">>Endian           : BIG endian");
						}
						else // For INTEL Hex2bin or MOTOROLA hex2bni
						{
							fprintf(stderr,">>Endian for CRC   : Big endian");
							Endian = 1; // big
						}

					}
					else
					{
						fprintf(stderr,"[++ERROR Endian++] Need option --intel or --motorola or --join \n");
					
						beep(700,100);
						AllFilesClosed(); // 2020.07.10
						exit(0); /// 2017.11.21

						return 0;
						break;
					}
				}
				else
				{
					fprintf(stderr,"\n\n WARNING:wrong option --endian [little|big]. check option\r\n");
			
					beep(700,100);
					AllFilesClosed(); // 2020.07.10
					exit(0); /// 2017.11.21

					return 0;
				}
				break;

		    case 'k':  /* --allpadarea : fill Padbye in all empty area */

				fprintf(stderr,"\n");
				if( (isMot2bin!=1) && (isIntel2bin!=1) )
				{
					fprintf(stderr,"\n[++ERROR PADAREA++] Need option --intel or --motorola \n");

					beep(700,100);
					AllFilesClosed(); // 2020.07.10
					exit(0); /// 2017.11.21

					return 0;					
					break;
				}

				fprintf(stderr,">>Fill Pad Byte    : Pad byte(0x%X) in empty ALL area of binary \n", Pad_Byte);

				isPadByteAllArea = 1;  // The specified char (Pad_Byte) is filled in all area.
				break;

			case 'Z': // --zeroforced

				fprintf(stderr,"\n");
				if( (isMot2bin!=1) && (isIntel2bin!=1) )
				{
					fprintf(stderr,"\n");
					fprintf(stderr,"[++ERROR ZERO_FORCED++] Need option --intel or --motorola \n");

					beep(700,100);
					AllFilesClosed(); // 2020.07.10
					exit(0); /// 2017.11.21

					return 0;
					break;
				}

				Enable_HexaAddr_Zero_Forced = HEX2BIN_ZERO_FORCED;
				fprintf(stderr,">>Address Forced   : Using ZERO addressing forced. \n");

				break;

		    case 'l': /* Hex2bin Max size - length */
			
				if(optarg) 
				{
					olen = 0;
			        memcpy(strHex2BinLen, optarg, MAX_CHARS);
					olen = strlen(strHex2BinLen);

					if( strcasecmp( &strHex2BinLen[olen-2], "kB" ) == 0 )
					{
						Max_Length_Setted = 1;
						strHex2BinLen[olen-2] = 0x00;
						strHex2BinLen[olen-1] = 0x00;
						strHex2BinLen[olen-0] = 0x00;
						Max_Length = str2int(strHex2BinLen);
						Max_Length *= 1024; // Because of KBytes
					}
					else if( strcasecmp( &strHex2BinLen[olen-2], "MB" ) == 0 )
					{
						Max_Length_Setted = 1;
						strHex2BinLen[olen-2] = 0x00;
						strHex2BinLen[olen-1] = 0x00;
						strHex2BinLen[olen-0] = 0x00;
						Max_Length = str2int(strHex2BinLen);
						Max_Length *= (1024*1024); // Because of MBytes
					}
					else
					{
						Max_Length_Setted = 1;
						Max_Length = GetHex( strHex2BinLen );

						//isLength = 1;
						titlen = GetDec( strHex2BinLen ); // 2021.12.27
					}

				#if 0
					if (Max_Length > 0x800000)
					{
						printf("Hex2bin Max Length Error = %u (Max Length = 0x800000) \n", Max_Length);
						Max_Length_Setted = 0;
						exit(1);
					}
				#endif
					
				}
				else
				{
					fprintf(stderr,"\n WARNING:wrong --length [hexa value] in converting --intel or --motorola. check option [%s] \n", strHex2BinLen);

					beep(700,100);
					AllFilesClosed(); // 2020.07.10
					exit(0); /// 2017.11.21

					return 0;
				}
				break;

			case 'R':

				if(optarg) 
				{
			        memcpy(str_dummy, optarg, MAX_CHARS);
					olen = strlen(str_dummy);

					isRandomNum = 1;

					iRanMaxi = str2int(str_dummy);
				}
				break;
				
		    case 'N': /* infotmation */
				if(optarg) 
				{
			    	isNKinform  = 1;
			        memcpy(str_nb0, optarg, MAX_CHARS);
					olen = strlen(str_nb0);

					if( 0==strncmp(NB0_EXT, str_nb0, olen) )
						isCreateNB0 = 1; /// Create *.nb0 file.
					else
						isCreateNB0 = 0;
					
				}
				else
				{
					fprintf(stderr,"\n\n WARNING:wrong option --nk [nb0 | none]. check option\r\n");

					beep(700,100);
					AllFilesClosed(); // 2020.07.10
					exit(0); /// 2017.11.21

					return 0;
				}
				break;
			
		    case 'I': /* infotmation */
				if(optarg) 
				{
			        memcpy(str_inform, optarg, MAX_CHARS);
			        FileCounter(str_inform);
				}

				AllFilesClosed(); // 2020.07.16
		        exit(0);
		    	break;

		    case 'F':

				if(optarg) 
				{
			    	isFillSize = 1; /* Fill 0xFF */

					memcpy(str_fillSiz, optarg, MAX_CHARS);
					olen = strlen(str_fillSiz);

			        fprintf(stderr,"\n>>Total file size  : %s ", str_fillSiz );
				
					if( 0 == strcasecmp( &str_fillSiz[olen-2], "kB" ) )
					{
						str_fillSiz[olen-2] = 0x00;
						str_fillSiz[olen-1] = 0x00;
						str_fillSiz[olen-0] = 0x00;
						total_bin_size = str2int(str_fillSiz);
						total_bin_size *= 1024; // Because of KBytes
						fprintf(stderr," (%#x) ", total_bin_size );
					}
					else if( 0 == strcasecmp( &str_fillSiz[olen-2], "MB" ) )
					{
						str_fillSiz[olen-2] = 0x00;
						str_fillSiz[olen-1] = 0x00;
						str_fillSiz[olen-0] = 0x00;
						total_bin_size = str2int(str_fillSiz);
						total_bin_size *= (1024*1024); // Because of MBytes
						fprintf(stderr," (%#x) ", total_bin_size );
					}
					else
					{
						total_bin_size = GetHex( str_fillSiz );
						fprintf(stderr," (0x%x) %dBytes ", total_bin_size, total_bin_size );
					}

				}
				else
				{
					fprintf(stderr,"\r\n WARNING:wrong option --fillsize [value]. check option \n\n");

					beep(700,100);
					AllFilesClosed(); // 2020.07.10
					exit(0); /// 2017.11.21

					return 0;
				}

		    	break;

			case 'y':
				{
				int ii=0, reidx=0;
				
					// --------------------------------------------------------------------
					iTotcnt = argc;
					do {
						if( strcmp( argv[ii], "--merge" ) == 0 )
						{
							//++printf("\n>> ii    : (%s) [%d] iTotcnt=%d \n", argv[ii], ii, iTotcnt );
							istartIdx = ii;
							break;
						}	
					} while( ii++ < iTotcnt);

					if( istartIdx == -1 ) 
					{
						fprintf(stderr,">> check please --merge option!!\n" );
						break;
					}

					ii++;
					do {
						if( (strncmp( argv[ii], "--", 2 ) == 0) || (strncmp( argv[ii], "-", 1 ) == 0) )
						{
							//++printf(">> iEndIdx    : (%s) [%d] \n", argv[ii], ii );
							iEndIdx = ii;
							break;
						}						
					} while( ii++ < iTotcnt);

					
					memset( mFile, 0x00, sizeof(mFile) );
					//memset(mergefile, 0x00, sizeof(mergefile) );

					fprintf(stderr,"\n>>Merging input file lists : \n");
					for(reidx=1, ii=istartIdx+1; ii<iEndIdx; ii++, reidx++)
					{
						memset( mFile[reidx].mergeFileName, 0x00, sizeof(mFile[reidx].mergeFileName) );
						strcpy( mFile[reidx].mergeFileName, argv[ii] );
						fprintf(stderr,"%2d -> filename: [%s] \n", reidx, mFile[reidx].mergeFileName );
					}

					ismultiFilesMerge = 1;
					// --------------------------------------------------------------------

				}
				break;

			// extract =============================
			case 'x':
				{

					if(optarg) 
					{
	
						memcpy(extractFile, optarg, MAX_FILENAME_LEN);
						olen = strlen(extractFile);
	
						fprintf(stderr,"\n>>Merged filename  : %s ", extractFile );

						if( NULL == (inpfile = fopen( extractFile, "rb")) ) 
						{
							fprintf(stderr,"\n--extract option error, files not found!! [%s] \n", extractFile );
							AllFilesClosed();
							exit(0); /// help();
							return 0;
						}
						fclose(inpfile);
	
						ismultiFilesExtract = 1;
					}
				
				}
				break;
					
			case 'j': /* 2017.04.05, File merge */

				if(optarg) 
				{
				
				#if 0
					isFileMerge = 1;
					memcpy(str_FileMax, optarg, MAX_CHARS);
					result = sscanf(str_FileMax,"%x",&iFileLimit);
					if( result==1 ) 
					{
						// OK
						printf("\n>>2nd file pos    : %#x (%u)", iFileLimit, iFileLimit );
					}
					else
					{
						// Hex read error
						printf("\n>>2nd file pos    : ++ERROR++ (%s)" , str_FileMax );
					}

				#else
				
					olen = 0;
					memcpy(str_FileMax, optarg, MAX_CHARS);
					olen = strlen(str_FileMax);
					isFileMerge = 1;
					
					if( strcasecmp( &str_FileMax[olen-2], "kB" ) == 0 )
					{
						str_FileMax[olen-2] = 0x00;
						str_FileMax[olen-1] = 0x00;
						str_FileMax[olen-0] = 0x00;
						iFileLimit = str2int(str_FileMax);
						iFileLimit *= 1024; // Because of KBytes

						fprintf(stderr,"\n>>2nd file pos     : %#x (%skB)", iFileLimit, str_FileMax );
					}
					else if( strcasecmp( &str_FileMax[olen-2], "MB" ) == 0 )
					{
						str_FileMax[olen-2] = 0x00;
						str_FileMax[olen-1] = 0x00;
						str_FileMax[olen-0] = 0x00;
						iFileLimit = str2int(str_FileMax);
						iFileLimit *= (1024*1024); // Because of MBytes

						fprintf(stderr,"\n>>2nd file pos     : %#x (%sMB)", iFileLimit, str_FileMax );
					}
					else
					{
						iFileLimit = GetHex( str_FileMax );
						fprintf(stderr,"\n>>2nd file pos     : %#x (%u Bytes)", iFileLimit, iFileLimit );
					}

					//printf("\n>>2nd file pos    : %#x (%u)", iFileLimit, iFileLimit );
				#endif

				}
				else
				{
					fprintf(stderr,"\r\n WARNING:wrong option --join [in hexa]. check option\n\n");

					beep(700,100);
					AllFilesClosed(); // 2020.07.10
					exit(0); /// 2017.11.21

					return 0;
				}

				break;

			#if 0
			case 's':
				if(isFileMerge)
				{
					is2ndFileSize = 1;
					is2ndEndian   = 0; // initial value none
					memcpy(str_SecFileSiz, optarg, MAX_CHARS);

					if( 0==strcasecmp( str_SecFileSiz, "little" ) ) 
					{
						is2ndEndian = ENDIAN_LITTLE; // little
					}
					else if( 0==strcasecmp( str_SecFileSiz, "big" ) ) 
					{
						is2ndEndian = ENDIAN_BIG; // big
					}

						
					//result = sscanf(str_SecFileSiz,"%x",&i2ndFileSize);
					//printf("\n>>Join Address : 0x%X \r\n", iFileLimit);				
				}
				else
				{
					is2ndFileSize = 0;
					is2ndEndian = 0;
					memset(str_SecFileSiz, 0x00, MAX_CHARS);

					printf("\n>>Do use --join option!! \n");
				}

				break;
			#endif


		    case 'd': /* detach Header */
				if(optarg) 
				{
			    	isDelHdr = 1; /* detach Header */
				    memcpy(str_DelHdr, optarg, MAX_CHARS);
					olen = strlen(str_DelHdr);

			    	fprintf(stderr,"\n>>Delete HDR size  : ");

					if( 0 == strcasecmp( &str_DelHdr[olen-2], "kB" ) )
					{
						str_DelHdr[olen-2] = 0x00;
						str_DelHdr[olen-1] = 0x00;
						str_DelHdr[olen-0] = 0x00;

						iHdr_len = str2int(str_DelHdr);
						fprintf(stderr," (%dkB) 0x%x \n", iHdr_len, iHdr_len );
					
						iHdr_len *= 1024; // Because of KBytes
					}
					else if( 0 == strcasecmp( &str_DelHdr[olen-2], "MB" ) )
					{
						str_DelHdr[olen-2] = 0x00;
						str_DelHdr[olen-1] = 0x00;
						str_DelHdr[olen-0] = 0x00;
						iHdr_len = str2int(str_DelHdr);
						fprintf(stderr," (%dMB) 0x%x \n", iHdr_len, iHdr_len );
					
						iHdr_len *= (1024*1024); // Because of MBytes
					}
					else if( 0 == strcasecmp( str_DelHdr, "default" ) )
					{
						iHdr_len=16*4; // default size
						fprintf(stderr,"default (%dBytes) \n", iHdr_len );
					}
					else
					{
						iHdr_len = str2int(str_DelHdr);
						if( iHdr_len == 0 ) 
						{
							iHdr_len=16*4; // default size
							fprintf(stderr,"default (%dBytes) 0x%x \n", iHdr_len, iHdr_len );
						}
						else
							fprintf(stderr," (%dBytes) 0x%x \n", iHdr_len, iHdr_len );
					}

				}
				else
				{
					fprintf(stderr,"\n\n WARNING:wrong option --detach [decimal value]. check option\r\n");

					beep(700,100);
					AllFilesClosed(); // 2020.07.10
					exit(0); /// 2017.11.21

					return 0;
				}				

		        break;

	
		    case 'b': /* Attach Header : 16 characters */
				if(optarg) 
				{
					olen = 0;
					//isAttach |= ATT_BOARD;
					isAttachBrd = ATT_BOARD; /* ATT_BOARD */

			        memcpy(str_boardName, optarg, MAX_CHARS);
					olen = strlen(str_boardName);

			        fprintf(stderr,"\n>>Board Name       : %s", str_boardName);

					if( olen > MAX_CHARS )
					{
						fprintf(stderr,"\r\n[++ERROR++] Board Name length is too long.. Max:%d Bytes\n\n", MAX_CHARS );
					}
				}
				else
				{
					fprintf(stderr,"\r\n WARNING:wrong option --board [string]. check option\n\n");

					beep(700,100);
					AllFilesClosed(); // 2020.07.10
					exit(0); /// 2017.11.21

					return 0;
				}
		        break;

				
		    case 'm': /* Attach Header : 16 characters */
				if(optarg) 
				{
					olen = 0;

					//isAttach |= ATT_MODEL;
					isAttachMdl = ATT_MODEL; /* ATT_MODEL */

					if(titlen)
					{
						fprintf(stderr,"\n\n[length] %d  \r\n", titlen );
					}
					else
					{
				        memcpy(str_moduleName, optarg, MAX_CHARS);
						olen = strlen(str_moduleName);

				        fprintf(stderr,"\n>>Model Name       : %s", str_moduleName);

						if( olen > MAX_CHARS )
						{
							fprintf(stderr,"\n\n[++ERROR++] Module Name length is too long.. Max:%d Bytes\n\n", MAX_CHARS );
						}
					}

				}
				else
				{
					fprintf(stderr,"\n\n WARNING:wrong option --model [string]. check option\r\n");

					beep(700,100);
					AllFilesClosed(); // 2020.07.10
					exit(0); /// 2017.11.21

					return 0;
				}
		        break;
		
		    case 'c': /* Attach Header : 16 characters ; date / crc16/crc32/crc64/adler32 */

				isMD2 = isMD4 = isMD5 = isMD6 = isSHA1 = isSHA224 = isSHA256 = isSHA384 = isSHA512 = isIDEA = 0;
				isSHA3_KECCAK_224 = isSHA3_KECCAK_256 = isSHA3_KECCAK_384 = isSHA3_KECCAK_512 = 0; // 2020.06.11
				isShake128 = isShake256 = 0;
				isBlake224 = isBlake256 = isBlake384 = isBlake512 = 0;
				isCRC = 0;
				isRipeMD128 = 0;
				isRipeMD160 = 0;
				
				if(optarg) 
				{
					memcpy(str_crcAdd, optarg, MAX_CHARS);
					isCRC	  = 0;
					insertCRC = 0;
					iUpper    = 0;
					olen = strlen(str_crcAdd);

					fprintf(stderr,"\n>>Security cinfo   : %s", str_crcAdd);

					strcpy(str_hash, str_crcAdd);


					isAttachCRC = ATT_DATEorCRC; /* ATT_DATEorCRC */

					if( 0==strcasecmp( str_crcAdd, "date" ) ) // inserted date
				    {
				    	int i;
				    	time_t 	sys_t;

						//isAttach |= ATT_DATEorCRC;

				    	time( &sys_t );
				    	strcpy( str_buildDate, ctime( &sys_t ) );
				        /*Mon Dec  8 16:41:19 2008*/
						for(i=0; i<MAX_CHARS*2; i++)
						{ if( '\n' == str_buildDate[i] ) str_buildDate[i] = '\0'; }


						/* uppercase */
						for(i=1; i<3; i++) { str_buildDate[i] = str_buildDate[i]^0x20; }
				        
				        /* -> 2008L08MON164119*/
				        /* step1. year */
						memcpy(&str_abuild_date[0], &str_buildDate[20], 4);

				        /* step2. month */
						for( iloop=0; iloop<MONTH_LEN; iloop++)
						{
							if( 0==strncmp(&str_buildDate[4], month_table[iloop].mon, 3 ) )
								break;
						}
						memcpy(&str_abuild_date[4], &month_table[iloop].amon, 1);

				        /* step3. date */
						memcpy(&str_abuild_date[5], &str_buildDate[8], 2);

				        /* step4. week */
						memcpy(&str_abuild_date[7], &str_buildDate[0], 3);

				        /* step5. hour */
						memcpy(&str_abuild_date[10], &str_buildDate[11], 2);

				        /* step5. minute */
						memcpy(&str_abuild_date[12], &str_buildDate[14], 2);

				        /* step6. second */
						memcpy(&str_abuild_date[14], &str_buildDate[17], 2);

						#ifdef DEBUG
						fprintf(stderr,"\nABuild Date>>%s<<", str_abuild_date);
						#endif

						fprintf(stderr,"\n>>Make curr today  : %s", str_buildDate);
				        break;
						
				    }   
					else if( 0==strcasecmp(str_crcAdd, "crc16") )
					{
						isCRC	  = 0;
						insertCRC = 1;
						isCRCtype = HDR_CRC16;
						//isAttach |= ATT_DATEorCRC;
						iUpper = isupper(str_crcAdd[0]); // 2017.04.27
						fprintf(stderr," is inserted as signed image.");
					}
					else if( 0==strcasecmp(str_crcAdd, "crc16ksc") )
					{
						isCRC	  = 0;
						insertCRC = 1;
						isCRCtype = HDR_KSC_CRC16;
						//isAttach |= ATT_DATEorCRC;
						iUpper = isupper(str_crcAdd[0]); // 2017.04.27
						fprintf(stderr," is inserted as signed image.");
					}
					else if( 0==strcasecmp(str_crcAdd, "crc16c") )
					{
						isCRC	  = 0;
						insertCRC = 1;
						isCRCtype = HDR_CRC16CCITT;
						//isAttach |= ATT_DATEorCRC;
						iUpper = isupper(str_crcAdd[0]); // 2017.04.27
						fprintf(stderr," is inserted as signed image.");
					}
					else if( 0==strcasecmp(str_crcAdd, "crc32") )
					{
						isCRC	  = 0;
						insertCRC = 1;
						isCRCtype = HDR_CRC32;
						//isAttach |= ATT_DATEorCRC;
						iUpper = isupper(str_crcAdd[0]); // 2017.04.27
						fprintf(stderr," is inserted as signed image.");
					}
					else if( 0==strcasecmp(str_crcAdd, "crc64") )
					{
						isCRC	  = 0;
						insertCRC = 1;
						isCRCtype = HDR_CRC64;
						//isAttach |= ATT_DATEorCRC;
						iUpper = isupper(str_crcAdd[0]); // 2017.04.27
						fprintf(stderr," is inserted as signed image.");
					}
					else if( 0==strcasecmp(str_crcAdd, "crc64isc") )
					{
						isCRC	  = 0;
						insertCRC = 1;
						isCRCtype = HDR_CRC64_ISC;
						//isAttach |= ATT_DATEorCRC;
						iUpper = isupper(str_crcAdd[0]); // 2017.04.27
						fprintf(stderr," is inserted as signed image.");
					}
					else if( 0==strcasecmp(str_crcAdd, "adler32") ) 
					{
						isCRC	  = 0;
						insertCRC = 1; 
						isCRCtype = HDR_ADLER32;
						//isAttach |= ATT_DATEorCRC;
						iUpper = isupper(str_crcAdd[0]); // 2017.04.27
						fprintf(stderr," is inserted as signed image.");
					}	
					else if( 0==strcasecmp(str_crcAdd, "joaat") ) 
					{
						isCRC	  = 0;
						insertCRC = 1; 
						isCRCtype = HDR_JOAAT;
						//isAttach |= ATT_DATEorCRC;
						iUpper = isupper(str_crcAdd[0]);
						fprintf(stderr," is inserted as signed image.");
					}
					else if( 0==strcasecmp(str_crcAdd, "sha1") )	// SHA1
					{
						isCRC	  = 0;
						insertCRC = 1; 
						isCRCtype = HDR_SHA1;
						//isAttach |= ATT_DATEorCRC;
						iUpper = isupper(str_crcAdd[0]);  
						fprintf(stderr," is inserted as signed image.");
					}	
					else if( 0==strcasecmp(str_crcAdd, "sha224") )	// SHA2-224
					{
						isCRC	  = 0;
						insertCRC = 1; 
						isCRCtype = HDR_SHA224;
						//isAttach |= ATT_DATEorCRC;
						iUpper = isupper(str_crcAdd[0]); // 2017.11.21
						fprintf(stderr," is inserted as signed image.");
					}	
					else if( 0==strcasecmp(str_crcAdd, "sha256") )  // SHA2-256
					{
						isCRC	  = 0;
						insertCRC = 1; 
						isCRCtype = HDR_SHA256;
						//isAttach |= ATT_DATEorCRC;
						iUpper = isupper(str_crcAdd[0]); // 2017.11.21
						fprintf(stderr," is inserted as signed image.");
					}	
					else if( 0==strcasecmp(str_crcAdd, "sha384") ) // SHA2-384
					{
						isCRC	  = 0;
						insertCRC = 1; 
						isCRCtype = HDR_SHA384;
						//isAttach |= ATT_DATEorCRC;
						iUpper = isupper(str_crcAdd[0]); // 2017.11.21
						fprintf(stderr," is inserted as signed image.");
					}	
					else if( 0==strcasecmp(str_crcAdd, "sha512") ) // SHA2-512
					{
						isCRC	  = 0;
						insertCRC = 1; 
						isCRCtype = HDR_SHA512;
						//isAttach |= ATT_DATEorCRC;
						iUpper = isupper(str_crcAdd[0]); // 2017.11.21
						fprintf(stderr," is inserted as signed image.");
					}	
					else if( 0==strcasecmp(str_crcAdd, "sha3-224") ) 
					{
						isCRC	  = 0;
						insertCRC = 1; 
						isCRCtype = HDR_SHA3_224;
						//isAttach |= ATT_DATEorCRC;
						iUpper = isupper(str_crcAdd[0]);
						fprintf(stderr," is inserted as signed image.");
					}	
					else if( 0==strcasecmp(str_crcAdd, "sha3-256") ) 
					{
						isCRC	  = 0;
						insertCRC = 1; 
						isCRCtype = HDR_SHA3_256;
						//isAttach |= ATT_DATEorCRC;
						iUpper = isupper(str_crcAdd[0]);
						fprintf(stderr," is inserted as signed image.");
					}	
					else if( 0==strcasecmp(str_crcAdd, "sha3-384") ) 
					{
						isCRC	  = 0;
						insertCRC = 1; 
						isCRCtype = HDR_SHA3_384;
						//isAttach |= ATT_DATEorCRC;
						iUpper = isupper(str_crcAdd[0]);
						fprintf(stderr," is inserted as signed image.");
					}	
					else if( 0==strcasecmp(str_crcAdd, "sha3-512") ) 
					{
						isCRC	  = 0;
						insertCRC = 1; 
						isCRCtype = HDR_SHA3_512;
						//isAttach |= ATT_DATEorCRC;
						iUpper = isupper(str_crcAdd[0]);
						fprintf(stderr," is inserted as signed image.");
					}	
					else if( 0==strcasecmp(str_crcAdd, "shake128") ) 
					{
						isCRC	  = 0;
						insertCRC = 1; 
						isCRCtype = HDR_SHAKE128;
						//isAttach |= ATT_DATEorCRC;
						iUpper = isupper(str_crcAdd[0]);
						fprintf(stderr," is inserted as signed image.");
					}	
					else if( 0==strcasecmp(str_crcAdd, "shake256") ) 
					{
						isCRC	  = 0;
						insertCRC = 1; 
						isCRCtype = HDR_SHAKE256;
						//isAttach |= ATT_DATEorCRC;
						iUpper = isupper(str_crcAdd[0]);
						fprintf(stderr," is inserted as signed image.");
					}	
					else if( 0==strcasecmp(str_crcAdd, "md5") ) // MD5
					{
						isCRC	  = 0;
						insertCRC = 1; 
						isCRCtype = HDR_MD5;
						//isAttach |= ATT_DATEorCRC;
						iUpper = isupper(str_crcAdd[0]); // 2017.11.21
						fprintf(stderr," is inserted as signed image.");
					}	
					else if( 0==strcasecmp(str_crcAdd, "md6") ) // MD6
					{
						isCRC	  = 0;
						insertCRC = 1; 
						isCRCtype = HDR_MD6;
						//isAttach |= ATT_DATEorCRC;
						iUpper = isupper(str_crcAdd[0]); // 2017.11.21
						fprintf(stderr," is inserted as signed image.");
					}	
					else if( 0==strcasecmp(str_crcAdd, "md2") ) // MD2
					{
						isCRC	  = 0;
						insertCRC = 1; 
						isCRCtype = HDR_MD2; // 2020.07.15
						//isAttach |= ATT_DATEorCRC;
						iUpper = isupper(str_crcAdd[0]); // 2017.11.21
						fprintf(stderr," is inserted as signed image.");
					}	
					else if( 0==strcasecmp(str_crcAdd, "md4") ) // MD4
					{
						isCRC	  = 0;
						insertCRC = 1; 
						isCRCtype = HDR_MD4; // 2020.07.15
						//isAttach |= ATT_DATEorCRC;
						iUpper = isupper(str_crcAdd[0]); // 2017.11.21
						fprintf(stderr," is inserted as signed image.");
					}	
				#if BLAKE_224_256_384_512_HASH
					else if( 0==strcasecmp(str_crcAdd, "blake224") ) 
					{
						isCRC	  = 0;
						insertCRC = 1; 
						isCRCtype = HDR_BLAKE224;
						//isAttach |= ATT_DATEorCRC;
						iUpper = isupper(str_crcAdd[0]);
						fprintf(stderr," is inserted as signed image.");
					}	
					else if( 0==strcasecmp(str_crcAdd, "blake256") ) 
					{
						isCRC	  = 0;
						insertCRC = 1; 
						isCRCtype = HDR_BLAKE256;
						//isAttach |= ATT_DATEorCRC;
						iUpper = isupper(str_crcAdd[0]);
						fprintf(stderr," is inserted as signed image.");
					}	
					else if( 0==strcasecmp(str_crcAdd, "blake384") ) 
					{
						isCRC	  = 0;
						insertCRC = 1; 
						isCRCtype = HDR_BLAKE384;
						//isAttach |= ATT_DATEorCRC;
						iUpper = isupper(str_crcAdd[0]);
						fprintf(stderr," is inserted as signed image.");
					}	
					else if( 0==strcasecmp(str_crcAdd, "blake512") ) 
					{
						isCRC	  = 0;
						insertCRC = 1; 
						isCRCtype = HDR_BLAKE512;
						//isAttach |= ATT_DATEorCRC;
						iUpper = isupper(str_crcAdd[0]);
						fprintf(stderr," is inserted as signed image.");
					}	
				#endif

				#if defined(RIPEMD160) || defined(RIPEMD128)	
					else if( 0==strcasecmp(str_crcAdd, "RipeMD128") || (0==strcasecmp(str_crcAdd, "RMD128")) )
					{
						isCRC	  = 0;
						insertCRC = 1; 
						isCRCtype = HDR_RMD128;
						//isAttach |= ATT_DATEorCRC;
						iUpper = isupper(str_crcAdd[0]);
						fprintf(stderr," is inserted as signed image.");
					}	
					else if( 0==strcasecmp(str_crcAdd, "RipeMD160") || (0==strcasecmp(str_crcAdd, "RMD160")) )
					{
						isCRC	  = 0;
						insertCRC = 1; 
						isCRCtype = HDR_RMD160;
						//isAttach |= ATT_DATEorCRC;
						iUpper = isupper(str_crcAdd[0]);
						fprintf(stderr," is inserted as signed image.");
					}	
				#endif		
					else
					{
						fprintf(stderr,"\n\n WARNING:wrong option --cinfo [string]. Check option (%s) \r\n", str_crcAdd);

						beep(700,100);
						AllFilesClosed();

						exit(0);
						return 0;
					}
				}
				else
				{
					fprintf(stderr,"\n\n WARNING:wrong option --cinfo [string]. Check options\r\n");
					beep(700,100);
					AllFilesClosed();
					exit(0);
					return 0;
				}
		    	break;


		    case 'v': /* Attach Header : 16 characters */

				if(optarg) 
				{
					olen = 0;

					//isAttach |= ATT_VERSION;
					isAttachVer = ATT_VERSION; /* ATT_VERSION or ATT_MCU_32BYTE */

					memcpy(str_versionName, optarg, MAX_VERSION_LEN);
					olen = strlen(str_versionName);
					
					fprintf(stderr,"\n>>Version Name     : %s", str_versionName);

					if( olen > MAX_VERSION_LEN )
					{
						fprintf(stderr,"\n\n[++ERROR++] Version Name length is too long.. Max:%d Bytes\n\n", MAX_VERSION_LEN );
					}
				}
				else
				{
					fprintf(stderr,"\n\n WARNING:wrong option --version [string]. check option\r\n");

					beep(700,100);
					AllFilesClosed(); // 2020.07.10
					exit(0); /// 2017.11.21

					return 0;
				}

				break;

			/* 2022-10-18 MCU 32Bytes packet added */
			case 'p':

				if(optarg) 
				{
					olen = 0;

					//isAttach = ATT_MCU_32BYTE;
					isAttachVer = ATT_MCU_32BYTE; /* ATT_VERSION or ATT_MCU_32BYTE */

					memcpy(str_MCUversion, optarg, MAX_32CHARS);
					olen = strlen(str_MCUversion);
					
					fprintf(stderr,"\n>>MCU Version      : %s", str_MCUversion);

					if( olen > MAX_32CHARS )
					{
						fprintf(stderr,"\n\n[++ERROR++] MCU Version Name length is too long (%d Chars).. Max:%d Bytes\n\n", olen, MAX_32CHARS );
					}
				}
				else
				{
					fprintf(stderr,"\n\n WARNING:wrong option --mcu [string]. check option\r\n");

					beep(700,100);
					AllFilesClosed(); // 2020.07.10
					exit(0); /// 2017.11.21

					return 0;
				}

				break;
				
		    case 'i': /* input file name : 32 characters */
				olen = 0;
				multifileindex = 0;
				iTotSize = 0ULL;

			#if 1
				iFileNum = 0;
				for(ii=1; ii<argc ; ii++)
				{
					if( (retFiles = _findfirsti64( argv[ii], &fexist )) == -1L )
					{
						if( ii>3 && (argv[ii][0] == '-') ) 
						{
							if( (iFileNum>2) && (-1!=iFirst) && (-1==iLast) ) 
							{
								if( (iFileNum>=2) && (ASTERISK_STEP1==isAsteMode) ) 
								{
									//printf("---++--(%d) (%d) (%d) \n", iFileNum, ii, argc); 
									isAsteMode = ASTERISK_FOUND;
								}
								iLast = ii+1;
							}
							else break;
						}
						//printf(" %d : [%s] ++++++ -> %d %d ", ii, argv[ii], iFirst, iLast );
					}
					else
					{
						if( (-1==iFirst) && (-1==iLast) )  
						{
							isAsteMode = ASTERISK_STEP1;
							iFirst = ii;
						}
						iFileNum ++;
						//printf(" %d : [%s] in current directory!  -> %d %d [%d] ", ii, argv[ii], iFirst, iLast, iFileNum );
					}
					_findclose( retFiles );

					//printf(" -> AsteMode %d \n", isAsteMode);
					if( ASTERISK_FOUND==isAsteMode ) break;
				}
				if( retFiles ) _findclose( retFiles );

			#endif		


			#if 1 // 2017.04.05
				isJoin=0;
				for(ii=0; ii<argc; ii++)
				{
					if( 0==strcmp(argv[ii], "-j") || 0==strcmp(argv[ii], "--join") )
					{
						isJoin = 1;
						break;
					}
					else isJoin=0;
				}

				if(isJoin)
				{
					for(ii=0; ii<argc; ii++)
					{
						if( isJoin && (0==strcmp(argv[ii], "-i") || 0==strcmp(argv[ii], "--input")) )
						{
							memset(sefile_name, 0x00, MAX_CHARS*LENGTH_OF_FILENAME);
							strcpy(sefile_name, argv[ii+2] );
							//printf("2nd input=[[ %s ]] \n", sefile_name);	
							isJoin = 2; // OK
							break;
						}
					}
				}

				if( 2==isJoin && (sefile_name[0] != 0x00) && (sefile_name[0] != '-') ) isJoin=3;

			#endif


				if(optarg) 
				{
					memcpy(infile_name, optarg, MAX_CHARS*LENGTH_OF_FILENAME);
					olen = strlen(infile_name);
						
			#if SHIFT_QUALITY_DATA_SORTING /* 2023-02-12 */
					memcpy(shift_in, optarg, MAX_CHARS*LENGTH_OF_FILENAME );
			#endif

				}
				else
				{
					fprintf(stderr,"\n\n[++ERROR++] Input file is NULL.  check option --input [filename]. \n\n" );
					
					beep(700,100);

					AllFilesClosed();

					exit(0);
					return 0;
				}

				
			#if MD5_MULTI_INPUT_FILES
				if( NULL != strstr(infile_name, "@." )  /// NOT olen 
					||  NULL != strstr(infile_name, "$." ) 
					)
				{
					int iIdx=0;
					int iLenFile = 0;
					iLenFile = strlen(infile_name);
					
					for(iIdx=0; iIdx<iLenFile; iIdx++)
					{
						     if( 0==strncmp( &infile_name[iIdx], "@.", 2) ) infile_name[iIdx] = '*';
						else if( 0==strncmp( &infile_name[iIdx], ".@", 2) ) infile_name[iIdx+1] = '*';
						else if( 0==strncmp( &infile_name[iIdx], "$.", 2) ) infile_name[iIdx] = '*';
						else if( 0==strncmp( &infile_name[iIdx], ".$", 2) ) infile_name[iIdx+1] = '*';
						else if( 0==strncmp( &infile_name[iIdx], "$", 1) ) infile_name[iIdx] = '*';
						else if( 0==strncmp( &infile_name[iIdx], "@", 1) ) infile_name[iIdx] = '*';
						else if( 0==strncmp( &infile_name[iIdx], "*.", 2) ) infile_name[iIdx] = '*';
						else if( 0==strncmp( &infile_name[iIdx], ".*", 2) ) infile_name[iIdx+1] = '*';
					}

					multifileindex = 1;
					memcpy( &extfile_name[0], infile_name, iLenFile );

					fprintf(stderr,"\n>>Input files      : @.@ or $.$ ");

					/// opt_ok += CHECK_BIT_IN;
				}
				else if( ASTERISK_FOUND==isAsteMode )
				{
					// no action!!
					// case ah.exe --input *.* ( case of 2 more files )
					//fprintf(stderr," -i or --input *.* isAsteMode(%d) \n", isAsteMode);
					fprintf(stderr,"\n>>Input files      : *.* ");
				}
				else
			#endif /// MD5_MULTI_INPUT_FILES
				{
					if( (retFiles = _findfirsti64( infile_name, &iFiles )) == -1L )
					{
						fprintf(stderr,"\n\nNo input file [%s] \n", infile_name );
						_findclose( retFiles );

						beep(700,100);
						AllFilesClosed();

						exit(0);
						return 0;
					}
					_findclose( retFiles );

					infile_size = iFiles.size; /// file size

				#if 0
					if( iFiles.size>>20 )
						printf("\n>>Input file   : %s (%.3f MB)", infile_name, (iFiles.size/1024.0)/1024.0 );
					else if( iFiles.size>>10 )
						printf("\n>>Input file   : %s (%.3f kB)", infile_name, (iFiles.size/1024.0) );
					else 
				#endif

						if( (iFiles.size/1024.0)/1024.0 > 1.0f )
						{
						fprintf(stderr,"\n>>Input file       : %s (%llu Bytes, %.2lfMB)", infile_name, iFiles.size, (iFiles.size/1024.0)/1024.0 );
						}
						else
						{
						fprintf(stderr,"\n>>Input file       : %s (%llu Bytes, %.2lfkB)", infile_name, iFiles.size, (iFiles.size/1024.0) );
						}

				#if 1 // 2017.04.05, 2nd file information
					if( 3==isJoin ) // --join option
					{
						if( (retFiles = _findfirsti64( sefile_name, &iFiles )) == -1L )
						{
							fprintf(stderr,"\r\nNo 2nd file [%s]. Check it! \n", sefile_name ); // 2nd input file check
							_findclose( retFiles );
						
							beep(700,100);
							AllFilesClosed();

							exit(0);

							return 0;						
						}
						_findclose( retFiles );
						
						sefile_size = iFiles.size; /// file size

						fprintf(stderr,"\n>>2nd input file   : %s (%llu Bytes)", sefile_name, sefile_size );

					}
				#endif

					opt_ok += CHECK_BIT_IN;

					if( strlen(optarg) >= (MAX_CHARS*LENGTH_OF_FILENAME) )
					{
						fprintf(stderr,"\n\n[++ERROR++] Input file name length is too long (%lld Chars).. Max:%d Bytes\n\n", strlen(optarg), (MAX_CHARS*LENGTH_OF_FILENAME) );

						beep(700,100);
						AllFilesClosed();

						exit(0);

						return 0;
					}
				}

				break;
		

			case 'a': /* output file name : 32 characters -- append mode */
				isAppend = 1; /// File Append--

		    case 'o': /* 32 characters  -- write mode */
				olen = 0;

				if(optarg) 
				{
					memcpy(outfile_name, optarg, MAX_CHARS*LENGTH_OF_FILENAME);
			#if SHIFT_QUALITY_DATA_SORTING /* 2022-11-13 */
					memcpy(shift_file, optarg, MAX_CHARS*LENGTH_OF_FILENAME );
			#endif
					olen = strlen(outfile_name);

					fprintf(stderr,"\n>>Output file      : %s (%s)", outfile_name, (isAppend==1)? "append":"new create");
					opt_ok += CHECK_BIT_OUT;

					if( strlen(optarg) >= (MAX_CHARS*LENGTH_OF_FILENAME) )
					{
						fprintf(stderr,"\n\n[++ERROR++] Output file name length is too long (%lld Chars).. Max:%d Bytes \n\n", strlen(optarg), (MAX_CHARS*LENGTH_OF_FILENAME) );

						beep(700,100);
						AllFilesClosed();
						
						exit(0);
						return 0;
					}
				}
				else
				{
					fprintf(stderr,"\n\n[++ERROR++] Output file is NULL.  check option --output|--append [filename]. \n\n" );

					beep(700,100);
					AllFilesClosed();

					exit(0);
					return 0;
				}				
				break;
		

		    case 'z': /// printf, verbos ---

				fprintf(stderr,"\n");
				memcpy(str_Verbos, optarg, MAX_CHARS);
				olen = strlen(str_Verbos);

				if( 0==strcasecmp( str_Verbos, "date" ) ) // inserted date
				{
					iVerbosType = 1;
			    	verbose=TRUE;
				}
				else if( 0==strcasecmp( str_Verbos, "size" ) ) // inserted date
				{
					iVerbosType = 2;
			    	verbose=TRUE;
				}
				else if( 0==strcasecmp( str_Verbos, "datesize" ) || 0==strcasecmp( str_Verbos, "sizedate" )) // inserted date
				{
					iVerbosType = 3;
			    	verbose=TRUE;
				}
				else
				{
					ret_scan = sscanf(str_Verbos,"%u",&iVerbosType);
					if( 1==ret_scan )
					{
			    		verbose=TRUE; // OK
					}
					else
					{
						fprintf(stderr,"\n\n[++ERROR++] str_Verbos parser is wrong [%s]-(%d) \n\n", str_Verbos, iVerbosType );
						fprintf(stderr,"  --verbose date or size or datesize or 1|2|3|4 \n");

						beep(700,100);
						AllFilesClosed(); // 2020.07.10
						exit(0); /// 2017.11.21

						return 0;
					}

				}
		    	break;

		#if SHIFT_QUALITY_DATA_SORTING /* 2022-11-13 */
			case 'U': /* UP - Shift Quality Data Sorting */
				if(optarg) 
				{
				int kk=0;
				int ll=0;
				double aps, aps1, aps2, apstep;
				
					//printf("\n");
					//printf("--1-- optind=%d, argv=%d\n", optind, argc);

					isShift 	 = 1;
					isUpShift	 = 1;
					isDownShift  = 0;
					iModeID      = -1;

					iPwrOnOff    = SHI_PWR_ON;

					memset(str_ShiftOp, 0x00, sizeof(str_ShiftOp) );

					// ------------------------------------------------
					// -- Option Saved --------------------------------
					iTCnt = 0;
					for(kk=optind-1; kk<argc; kk++, iTCnt++)
					{
						//memcpy(str_ShiftOp[0], optarg, MAX_CHARS);
						memcpy(str_ShiftOp[iTCnt], argv[kk], MAX_CHARS);
					}
					// ------------------------------------------------
					// ------------------------------------------------

					itmpFileDeleted = 1;
					if( strstr(str_ShiftOp[0], ".used") )
					{
					int i, len;

						itmpFileDeleted = 0; /* temp file Live ~~~ */
						len = strlen( str_ShiftOp[0] );
						for(i=len; i>0; i--)
						{
							if( 0==strncmp( (char*)&str_ShiftOp[0][i], (char*)".used", 5 ) ) 
							{
								strcpy( (char*)&str_ShiftOp[0][i], (char*)&str_ShiftOp[0][i+5] );
								break;
							}

						}
					}


					iSBchoicePnt = 0; /* 0 means SB first position */
					if( strstr(str_ShiftOp[0], ".last") )
					{
					int i, len;

						iSBchoicePnt = 1; /* 1 means SB Last position */
						len = strlen( str_ShiftOp[0] );
						for(i=len; i>0; i--)
						{
							if( 0==strncmp( (char*)&str_ShiftOp[0][i], (char*)".last", 5 ) ) 
							{
								strcpy( (char*)&str_ShiftOp[0][i], (char*)&str_ShiftOp[0][i+5] );
								break;
							}
						}
					}


					gValueDisplay = 1;
					if( strstr(str_ShiftOp[0], ".g1") || strstr(str_ShiftOp[0], ".g2") )
					{
					int i, len;

						len = strlen( str_ShiftOp[0] );
						for(i=len; i>0; i--)
						{
							if( 0==strncmp( (char*)&str_ShiftOp[0][i], (char*)".g1", 3 ) ) 
							{
								gValueDisplay = 0; /* Graph and value display ~~~ */
								strcpy( (char*)&str_ShiftOp[0][i], (char*)&str_ShiftOp[0][i+3] );
								break;
							}
							else if( 0==strncmp( (char*)&str_ShiftOp[0][i], (char*)".g2", 3 ) ) 
							{
								gValueDisplay = 2; /* Graph and value display ~~~ */
								strcpy( (char*)&str_ShiftOp[0][i], (char*)&str_ShiftOp[0][i+3] );
								break;
							}
						}
					}

					iModeID = -1;
					for(kk=0; kk<MODE_ID_NUMS-1; kk++)
					{
						if( 0==strcasecmp(str_ShiftOp[0], arrPATs_ModeID[kk].ModeNm) )
						{
							iModeID = kk;
							fprintf(stderr,"\n");
							fprintf(stderr,">>PATs-ModeID      : <<%d>> %d, %s", 0, kk, arrPATs_ModeID[kk].ModeID ); 
							break;
						}
					}
					if(iModeID == -1)
					{
						fprintf(stderr,"\n");
						fprintf(stderr,">>PATs-ModeID	   : <<%d>> %d, Unknown ModeID, Check ModeID(ECO, NOR, SPT, ...) [%s] \n\n", 0, iModeID, str_ShiftOp[0] ); 
						AllFilesClosed();
						exit(EXIT_FAILURE);
						break;
					}

					if(itmpFileDeleted) fprintf(stderr," - temp will be deleted."); 


					for(kk=1; kk<iTCnt; kk++)
					{
						if( (0==strncmp(str_ShiftOp[kk], "--", 2)) || (0==strncmp(str_ShiftOp[kk], "-", 1)) ) 
						{
							break;
						}
						
						switch(kk)
						{
						case 1:
							// 3>> SB repoint : SB decision times
							iSBdecision = atoi( str_ShiftOp[kk] ); 
							if( (iSBdecision < SB_DECISION_MAX_TIMES) && (iSBdecision > 0) )
							{
								if(iSBchoicePnt) 
								{
									iSBchoicePnt = iSBdecision-1; /* Last position */
									fprintf(stderr,"\n");
									fprintf(stderr,">>SB decision Num  : <<%d>> %d times (default:3 times) - last position", kk, iSBdecision ); 
								}
								else
								{
									iSBchoicePnt = 0; /* first position */
									fprintf(stderr,"\n");
									fprintf(stderr,">>SB decision Num  : <<%d>> %d times (default:3 times) - first position", kk, iSBdecision ); 
								}
							}
							else
							{
								fprintf(stderr,"\n");
								fprintf(stderr,">>SB decision Num  : <<%d>> %d - Warning times... available range:1~%d ", kk, iSBdecision, SB_DECISION_MAX_TIMES-1 ); 
								fprintf(stderr,"\r\n\n");
								AllFilesClosed();
								exit(EXIT_FAILURE);
							}
							break;

						case 2:
							// 4>> Jerk#1
							iJerkTimeLen = atoi( str_ShiftOp[kk] ); 
							if( (iJerkTimeLen <= JERK_MAX_TIME_mSec) && (iJerkTimeLen >= JERK_min_TIME_mSec) )
							{
								short iMod = 0;

								iMod = iJerkTimeLen%5;
								fprintf(stderr,"\n");
								fprintf(stderr,">>Jerk Time Length : <<%d>> %d msec (unit: msec)", kk, iJerkTimeLen ); 
								if( 0 != iMod )
								{
									fprintf(stderr,"\n  Warning: Jerk Time Length is wrong %d msec, check in 5msec unit...", iJerkTimeLen);
								}
							}
							else
							{
								fprintf(stderr,"\n");
								fprintf(stderr,">>Jerk Time Length : <<%d>> %d msec - Error previous time... available arrange:%d~%d msec", kk, iJerkTimeLen, JERK_min_TIME_mSec, JERK_MAX_TIME_mSec ); 
								fprintf(stderr,"\r\n\n");
								AllFilesClosed();
								exit(EXIT_FAILURE);
							}
							break;

						case 3:
							// 5>> Nt min/Max arrange #1
							iNtTimeLen = atoi( str_ShiftOp[kk] ); 
							if( (iNtTimeLen <= Nt_MAX_TIME_mSec) && (iNtTimeLen >= Nt_min_TIME_mSec) )
							{
								short iMod = 0;
						
								iMod = iNtTimeLen%5;
								fprintf(stderr,"\n");
								fprintf(stderr,">>Nt-min/-Max Time : <<%d>> %d msec (unit: msec)", kk, iNtTimeLen ); 
								if( 0 != iMod )
								{
									fprintf(stderr,"\n	Warning: Nt-min/Nt-Max Time Length is wrong %d msec, check in 5msec unit...", iNtTimeLen);
								}
							}
							else
							{
								fprintf(stderr,"\n");
								fprintf(stderr,">>Nt-min/-Max Time : <<%d>> %d msec - Error previous time... available arrange:%d~%d msec", kk, iNtTimeLen, Nt_min_TIME_mSec, Nt_MAX_TIME_mSec ); 
								fprintf(stderr,"\r\n\n");
								AllFilesClosed();
								exit(EXIT_FAILURE);
							}
							break;

						case 4: /* Power On/Off  APS Level */
							// 1>> APS POWER ON/OFF Level
							olen = strlen(str_ShiftOp[kk]);
							fAPSpwrLvl = atof( str_ShiftOp[kk] ); 
							if(fAPSpwrLvl >= 3.0f) iPwrOnOff = SHI_PWR_ON;
							else iPwrOnOff = SHI_PWR_OFF;
							fprintf(stderr,"\n");
							fprintf(stderr,">>APS Percent lvl  : <<%d>> %.1lf%% -- default(3%%~5%%) \n", kk, fAPSpwrLvl ); 
							fprintf(stderr,">>POWER ON/OFF     :       %s", (iPwrOnOff==SHI_PWR_ON?"PWR On":(iPwrOnOff==SHI_PWR_OFF?"PWR Off":(iPwrOnOff==SHI_STATIC?"Static":(iPwrOnOff==SHI_N_STOP_DN?"Stop Dn":"Unknown")))) ); 
							break;
							
						case 5:
							// 2>> APS tolerance
							fAPStol = atof( str_ShiftOp[kk] ); 
							fprintf(stderr,"\n");
							fprintf(stderr,">>APS Tolerance    : <<%d>> %.1lf%% -- default(-/+%.1f%%)", kk, fAPStol, APS_TOLENANCE ); 
							break;
						case 6:							
							aps1 = atoi( str_ShiftOp[kk] ); 
							fprintf(stderr,"\n");
							fprintf(stderr,">>APS Table Init   : <<%d>> %.1lf ", kk, aps1 ); 
							break;
						case 7:
							aps2 = atof( str_ShiftOp[kk] ); 
							fprintf(stderr,"\n");
							fprintf(stderr,">>APS Table Last   : <<%d>> %.1lf ", kk, aps2 ); 
							break;
						case 8:
							apstep = atof( str_ShiftOp[kk] ); 
							fprintf(stderr,"\n");
							fprintf(stderr,">>APS Table Step   : <<%d>> %.1lf <-- APSt Table is updated as below.", kk, apstep ); 

							for(ll=0, aps=aps1; ((aps<=aps2) && (ll<APS_TABLE_NUM)); aps+=apstep, ll++)
							{
								ApsTble[ll] = aps;
							}
							break;

						case 9:

						default:
							fprintf(stderr,"\n");
							fprintf(stderr,">>upshift options  : <<%d>> (%s) ", kk, str_ShiftOp[kk] ); 
							break;
						}

					}
					

				}
				else
				{
					fprintf(stderr,"\n\n[++ERROR++] up shift option error. %s \r\n", optarg );

					beep(700,100);
					AllFilesClosed();

					exit(0);
					return 0;
				}

				break;

			case 'D': /* DOWN - Shift Quality Data Sorting */
				if(optarg) 
				{
				int kk=0;
				int ll=0;
				double aps, aps1, aps2, apstep;
				
					//printf("\n");
					//printf("--1-- optind=%d, argv=%d\n", optind, argc);

					isShift 	 = 1;
					isUpShift	 = 0;
					isDownShift  = 1;
					iModeID      = -1;

					iPwrOnOff    = SHI_PWR_ON;

					memset(str_ShiftOp, 0x00, sizeof(str_ShiftOp) );

					// ------------------------------------------------
					// -- Option Saved --------------------------------
					iTCnt = 0;
					for(kk=optind-1; kk<argc; kk++, iTCnt++)
					{
						//memcpy(str_ShiftOp[0], optarg, MAX_CHARS);
						memcpy(str_ShiftOp[iTCnt], argv[kk], MAX_CHARS);
					}
					// ------------------------------------------------
					// ------------------------------------------------
					itmpFileDeleted = 1;
					if( strstr(str_ShiftOp[0], ".used") || strstr(str_ShiftOp[0], "_used") )
					{
					int i, len;

						itmpFileDeleted = 0; /* temp file Live ~~~ */
						len = strlen( str_ShiftOp[0] );
						for(i=len; i>0; i--)
						{
							if( '.' == str_ShiftOp[0][i] || '_' == str_ShiftOp[0][i] ) 
							{
								str_ShiftOp[0][i]='\0';
								break;
							}
						}
					}


					iModeID = -1;
					for(kk=0; kk<MODE_ID_NUMS-1; kk++)
					{
						if( 0==strcasecmp(str_ShiftOp[0], arrPATs_ModeID[kk].ModeNm) )
						{
							iModeID = kk;
							fprintf(stderr,"\n");
							fprintf(stderr,">>PATs-ModeID      : <<%d>> %d, %s", 0, kk, arrPATs_ModeID[kk].ModeID ); 
							break;
						}
					}
					if(iModeID == -1)
					{
						fprintf(stderr,"\n");
						fprintf(stderr,">>PATs-ModeID	   : <<%d>> %d, Unknown ModeID +++", 0, iModeID ); 
					}

					if(itmpFileDeleted) 
					{
						//fprintf(stderr,"\n");
						fprintf(stderr," -- temp files to be deleted..."); 
					}
					
					for(kk=1; kk<iTCnt; kk++)
					{
						if( (0==strncmp(str_ShiftOp[kk], "--", 2)) || (0==strncmp(str_ShiftOp[kk], "-", 1)) ) 
						{
							break;
						}
						
						switch(kk)
						{
						case 1: /* Power On/Off  APS Level */
							// 1>> APS POWER ON/OFF Level
							olen = strlen(str_ShiftOp[kk]);
							fAPSpwrLvl = atof( str_ShiftOp[kk] ); 
							if(fAPSpwrLvl >= 3.0f) iPwrOnOff = SHI_PWR_ON;
							else iPwrOnOff = SHI_PWR_OFF;
							fprintf(stderr,"\n");
							fprintf(stderr,">>VS Percent lvl   : <<%d>> %.2lf%% -- default(3%%~5%%) \n", kk, fAPSpwrLvl ); 
							fprintf(stderr,">>POWER ON/OFF     :      %s", (iPwrOnOff==SHI_PWR_ON?"PWR On":(iPwrOnOff==SHI_PWR_OFF?"PWR Off":(iPwrOnOff==SHI_STATIC?"Static":(iPwrOnOff==SHI_N_STOP_DN?"Stop Dn":"Unknown")))) ); 
							break;
							
						case 2:
							// 2>> APS tolerance
							fAPStol = atof( str_ShiftOp[kk] ); 
							fprintf(stderr,"\n");
							fprintf(stderr,">>VS Tolerance     : <<%d>> %.1lf -- default(-/+1.0)", kk, fAPStol ); 
							break;
							
						case 3:
							aps1 = atof( str_ShiftOp[kk] ); 
							fprintf(stderr,"\n");
							fprintf(stderr,">>VS Table Init    : <<%d>> %.1lf ", kk, aps1 ); 
							break;
						case 4:
							aps2 = atof( str_ShiftOp[kk] ); 
							fprintf(stderr,"\n");
							fprintf(stderr,">>VS Table Last    : <<%d>> %.1lf ", kk, aps2 ); 
							break;
						case 5:
							apstep = atof( str_ShiftOp[kk] ); 
							fprintf(stderr,"\n");
							fprintf(stderr,">>VS Table Step    : <<%d>> %.1lf -- VS Table is updated...", kk, apstep ); 

							for(ll=0, aps=aps1; ((aps<=aps2) && (ll<APS_TABLE_NUM)); aps+=apstep, ll++)
							{
								ApsTble[ll] = aps;
							}
							break;

						case 6:
						case 7:


						default:
							fprintf(stderr,"\n");
							fprintf(stderr,">>upshift options  : <<%d>> (%s) ", kk, str_ShiftOp[kk] ); 
							break;
						}

					}
					

				}
				else
				{
					fprintf(stderr,"\n\n[++ERROR++] up shift option error. %s \r\n", optarg );

					beep(700,100);
					AllFilesClosed();

					exit(0);
					return 0;
				}

				break;

				break;

		#endif /* SHIFT_QUALITY_DATA_SORTING */


			default:
				//printf("\n\nWARNING : Unknwon option [%c]. Check options.... \r\n", opt);
				fprintf(stderr,"\n\n");
				beep(700,100);

				AllFilesClosed();

				//exit(0);
				//return 0;
				break;
		}

	}



	g_iUpper = iUpper; // uppercase for crc16/crc32/crc64/crc16c or CRC16/CRC32/CRC64/CRC16C


	if( argc < 2 )
	{
		help();

		beep(700,100);
		AllFilesClosed();

		exit(0);
		return 0;
	}


	
	/* =========================================== */
	/* ============== INPUT FILE ================= */
	/* =========================================== */

#if MD5_MULTI_INPUT_FILES
	if(1==isIgnoreBothFile)
	{
		// input file and output files are ignored
		if(verbose==TRUE) LOG_ERR("\n\n[INPUT] Input & output files are ignored.");
	}
	else if(1==isIgnoreInFile)
	{
		// input file is ignored
		if(verbose==TRUE) LOG_ERR("\n\n[INPUT] Input file is ignored.");
	}	
	else if( multifileindex > 0 || (ASTERISK_FOUND==isAsteMode) || (1==isIgnoreBothFile && 1==isFloat2Hex) || (1==ismultiFilesMerge) || (1==ismultiFilesExtract) ) /// input files ignored---
	{
		/// MD5 / SHA1 Checksum 에서만 사용한다...
		/// printf("There are input file...\n");
		/// if(verbose==TRUE) LOG_ERR("\n\n[INPUT] Input files are multi.. %d/ %d/ %d/ %d/ %d", multifileindex, isAsteMode, isIgnoreBothFile, isIgnoreInFile, isIgnoreOuFile );
	}
	else
#endif /// MD5_MULTI_INPUT_FILES
	{

	#if 0
		if( (0x00==infile_name[0]) )  
		{ 
			isIgnoreBothFile =1; 
			isIgnoreInFile = 1; 
		}
		
		if( (0x00==outfile_name[0]) ) 
		{ 
			//printf("\n\n[++ERROR++] output file file [%s]. \n", outfile_name);
			isIgnoreBothFile =1; 
			isIgnoreOuFile = 1; 
		}
	#endif

		/* ========== INPUT FILE ================= */
		if( NULL == (inpfile = fopen( infile_name, "rb")) ) 
		{
			beep(700,100);
			printf("\n\n[++ERROR++] Can not open input file[%s]. Use --input option! \n",infile_name);

			AllFilesClosed();

			exit(0); /// help();
			return 0;
		}
		else
		{
			/// OK ---
			if (fstat(fileno(inpfile), &file_statbuf) < 0) 
			{ 
				printf("\n\n[++ERROR++]Cannot stat [%s]\n", infile_name ); 

				AllFilesClosed(); // 2020.07.10
				exit(0);
				return 2; 
			} 	 
		}
		/* ========== INPUT FILE ================= */
	}



	/* ========== OUTPUT FILE ================= */
	if( (1==isIgnoreBothFile && 1==isFloat2Hex) )
	{
		// 2020.06.23, ignore input file and output file
		if(verbose==TRUE) LOG_ERR("\n\n[OUTPUT] Output files are multi.. %d/ %d/ %d/ %d", multifileindex, isIgnoreBothFile, isIgnoreInFile, isIgnoreOuFile );
	}
	else if(1==isIgnoreBothFile)
	{
		if(verbose==TRUE) LOG_ERR("\n\n[OUTPUT] isIgnoreBothFile is TRUE. ");
		// input and output BOTH files are ignored
	}
	else if(1==isIgnoreOuFile)
	{
		if(verbose==TRUE) LOG_ERR("\n\n[OUTPUT] isIgnoreOuFile is TRUE. ");
		// output file is ignored
	}
	else if(1==ismultiFilesExtract)
	{
		if(verbose==TRUE) LOG_ERR("\n\n[OUTPUT] ismultiFilesExtract is TRUE. so this do extract files --merge option file.. \n");
	}
	else if(1==isAppend)
	{
		/// CHECKING input = output
		if( 0==strcasecmp(infile_name, outfile_name) )
		{
			printf("\n\n[++ERROR++] DO check input[%s] and output[%s]. filenames are same!!! \n",infile_name, outfile_name );

			beep(700,100);
			AllFilesClosed();

			exit(0); /// help();
			return 0;
		} 
		else if( (outfile = fopen( outfile_name, "ab")) == NULL )  /// append
		{
			int i=0, iLen, imkFolder=0;
			int nResult;
			char szDir[255] = {0,};

			if(outfile) { fclose(outfile); outfile=NULL; }

			// ~~if(inpfile) fclose(inpfile);
			// ~~if(data_buf) free(data_buf);

			
			iLen = strlen(outfile_name);
			memset( szDir, 0x00, sizeof(szDir) );
			for(i=0; i<iLen; i++)
			{
				szDir[i] = outfile_name[i];
				if( '\\' == outfile_name[i]  )
				{
					szDir[i+1] = 0x00;
			
					nResult = mkdir( szDir );
					if( nResult == 0 )
					{
						imkFolder = 1;
						// OK
					}
					else if( nResult == -1 )
					{
						imkFolder = 0;
						// mkdir ERROR
						if(outfile) { fclose(outfile);	outfile=NULL; }
					
						printf("\n\nCan not create folder [%s] - (append mode) \n", szDir);
					}
				}
			}

			if( 1==imkFolder )
			{
				// mkdir OK
				if( NULL == (outfile = fopen( outfile_name, "wb"))	)	
				{
					// FAIL
					printf("\n\nCan not create folder & output file. [%s / %s] - (append mode) \n", szDir, outfile_name);
					if( NULL == outfile_name || outfile_name[0] == 0x00 )
						printf("Need an output file option (--output or --append).\n" );
			
					isIgnoreBothFile = 1; 
					isIgnoreOuFile = 1; 

				}
			}
			else
			{
				printf("\n\nCan not create output file. [%s] - (append mode) \n", outfile_name);
				if( NULL == outfile_name || outfile_name[0] == 0x00 )
					printf("Need an output file option (--output or --append) \n" );

				isIgnoreBothFile = 1; 
				isIgnoreOuFile = 1; 

			}
			
			//~~~~ exit(0); /// help();
			//~~~~ return 0;

		}	
		else
		{
			/// OK---
			/// if(verbose==TRUE) LOG_ERR("\n\n[OUTPUT-APPEND] ELSE...  " );
		}
		
	}
	else
	{
		/// --- CHECKING input = output ----
		if( 1==isIgnoreBothFile || 1==isIgnoreOuFile )
		{
			//
			if(verbose==TRUE) LOG_ERR("\n\n[OUTPUT] isIgnoreBothFile(%d) or isIgnoreOuFile(%d) is TRUE ", isIgnoreBothFile, isIgnoreOuFile);
		}
		else if( 0==strcasecmp(infile_name, outfile_name) )
		{
			beep(700,100);
			printf("\n\n[++ERROR++] DO check input[%s] and output[%s]. Filenames are same!! \n",infile_name, outfile_name );

			AllFilesClosed();

			exit(0); /// help();
			return 0;
		} 
		else if( NULL == (outfile = fopen( outfile_name, "wb"))  ) /// write
		{
			int i=0, iLen, imkFolder=0;
			int nResult;
			char szDir[255] = {0,};
						

			if(outfile) { fclose(outfile);  outfile=NULL; }

			//~~ if(inpfile) fclose(inpfile);
			//~~ if(data_buf) free(data_buf);

			iLen = strlen(outfile_name);
			memset( szDir, 0x00, sizeof(szDir) );
			for(i=0; i<iLen; i++)
			{
				szDir[i] = outfile_name[i];
				if( '\\' == outfile_name[i]  )
				{
					szDir[i+1] = 0x00;

					nResult = mkdir( szDir );
					if( nResult == 0 )
					{
						imkFolder = 1;
						// OK
					}
					else if( nResult == -1 )
					{
						imkFolder = 0;
						// mkdir ERROR
						if(outfile) { fclose(outfile);	outfile=NULL; }
					
						printf("\n\nCan not create folder [%s] \n", szDir);
					}
				}
			}

			if( 1==imkFolder )
			{
				// mkdir OK
				if( NULL == (outfile = fopen( outfile_name, "wb"))  )	
				{
					// FAIL
					printf("\n\n[++ERROR++]Can NOT write folder & output file. [%s/ %s] \n", szDir, outfile_name);
					if( NULL == outfile_name || outfile_name[0] == 0x00 )
						printf("Need an output file option (--output or --append)\n" );

				}
			}
			else
			{
				printf("\n\n[++ERROR++]Can NOT write output file. [%s] \n", outfile_name);
				if( NULL == outfile_name || outfile_name[0] == 0x00 )
					printf("Need an output file option (--output or --append) \n" );
			}
			//isIgnoreBothFile = 1; 
			//isIgnoreOuFile = 1; 

			//~~~~ exit(0); /// help();
			//~~~~ return 0;
		}
		else
		{
			//if(verbose==TRUE) LOG_ERR("\n\n[OUTPUT-WRITE] ELSE...  " );
			/// OK---
		}
	}
	/* ========== OUTPUT FILE ================= */


	

	if( (0==isHash) && (1==isIgnoreOuFile) )
	{

		beep(700,100);
		AllFilesClosed();

		printf("\n\n[++ERROR++]Can not create output file[%s]!!! please check!!! \n\n", outfile_name);

		exit(0); /// help();
		return 0;	
	}


	// --------------------------------------------------------------------------------
	// --------------------------------------------------------------------------------
	// --------------------------------------------------------------------------------
	// --------------------------------------------------------------------------------
	// --------------------------------------------------------------------------------
	// --------------------------------------------------------------------------------



	/* 1. Header (Version) 붙이기 */
	if(    (0 == isDelHdr) 
		/* && ( ((ATT_VERSION|ATT_DATEorCRC|ATT_MODEL|ATT_BOARD)==isAttach) || ATT_MCU_32BYTE==isAttach) */ /* 2022-10-18 */
		&& ( (ATT_VERSION==isAttachVer||ATT_MCU_32BYTE==isAttachVer)||(ATT_DATEorCRC==isAttachCRC)||(ATT_MODEL==isAttachMdl)||(ATT_BOARD==isAttachBrd) )
		&& (0 == isIgnoreBothFile) /// 2014.07.14
		&& (0 == isCRC) 
		&& (0 == isFillSize) 
		&& (0 == isNKinform) 
		&& (0 == isMot2bin) 
		&& (0 == isIntel2bin) 
		&& (0 == isElf2Bin)
		&& (0 == isFloat2Hex) 
		&& (0 == isBMPreverse) /// 2015.02.12
		&& (0 == isFileMerge) // 2017.04.05
#if CONVERT_BMP2C
		&& (0 == isConvertBMP2C) /// 2013.07.09
#endif /// CONVERT_BMP2C

#if MD2_CHECKSUM_ENCIPHER 
		&& (0 == isMD2) /// 2014.07.29
#endif /// MD5_CHECKSUM_ENCIPHER

#if MD4_CHECKSUM_ENCIPHER 
		&& (0 == isMD4) /// 2014.07.26
#endif /// MD5_CHECKSUM_ENCIPHER

#if MD5_CHECKSUM_ENCIPHER 
		&& (0 == isMD5) /// 2014.06.26
#endif /// MD5_CHECKSUM_ENCIPHER

#if SHA1_HASH_ENCIPHER
		&& (0 == isSHA1) /// 2014.06.29
#endif /// SHA1_HASH_ENCIPHER

#if SHA2_256_384_512
		&& (0 == isSHA256) /// 2014.06.30
		&& (0 == isSHA384) /// 2014.06.30
		&& (0 == isSHA512) /// 2014.06.30
#endif /// SHA2_256_384_512

#if MD6_CHECKSUM_ENCIPHER
		&& (0 == isMD6 ) /// 2014.07.31
#endif

#if SHA2_224_CHECKSUM
		&& (0 == isSHA224 ) /// 2014.07.31
#endif

#if SHA3_KECCAK_224_256_384_512 // 2020.06.11
		&& (0 == isSHA3_KECCAK_224)
		&& (0 == isSHA3_KECCAK_256)
		&& (0 == isSHA3_KECCAK_384)
		&& (0 == isSHA3_KECCAK_512) 
		&& (0 == isShake128)
		&& (0 == isShake256)
#endif

#if defined(RIPEMD160) || defined(RIPEMD128)	
		&& (0==isRipeMD128)
		&& (0==isRipeMD160)
#endif

#if BLAKE_224_256_384_512_HASH
		&& (0 == isBlake224)
		&& (0 == isBlake256)
		&& (0 == isBlake384)
		&& (0 == isBlake512)
#endif // BLAKE_224_256_384_512_HASH

#if IDEA_ENCIPHER_ALGORITHM 
		&& (0 == isIDEA) /// 2014.07.26
#endif

		&& (0 == isRandomNum) // 2020.0731

#if MODIFIED_JULIAN_DATE
		&& (0 == isMJD) /// 2014.07.04
#endif /// MODIFIED_JULIAN_DATE
	   
#if SHIFT_QUALITY_DATA_SORTING /* 2022-11-13 */
		&& (0 == isShift) && (0 == isUpShift) && (0 == isDownShift)
#endif

	   )
	{

	
	/* =================================== */
	len_attach_hdr = 0; // 2020.07.07, Total Header Length


	/* ++++++++ BOARD NAME +++++++++++++++++++++++++++ */
	/* step1 : board name (16 characters) : ex) PI2000 */
	/* +++++++++++++++++++++++++++++++++++++++++++++++ */

	if( ATT_BOARD==isAttachBrd )
	{

		if( 0x00 == str_boardName[0] )
	        printf("\n>>Board Name (default)  : %s", DUMMY_FILL);

		len_board_name = strlen(str_boardName);
		if( len_board_name < MAX_CHARS )
		{
			if( len_board_name == 0 ) 
			{ 
				memcpy(str_boardName, DUMMY_FILL, DUMMY_FILL_LEN); 
				len_board_name=DUMMY_FILL_LEN; 
			}
			if(outfile) fprintf(outfile,"%s", str_boardName);
			while( len_board_name < MAX_CHARS )
			{
				if(outfile) fprintf(outfile,"%c",SPACE_FILL1);
				len_board_name++;
			}
		}
		else
		{
			count=0;
			while( count < MAX_CHARS )
			{
				if(outfile) fprintf(outfile,"%c",str_boardName[count] );
				count++;
			}
		}
		len_attach_hdr += MAX_CHARS; // for Board Name

	}



	/* ++++++++ MODEL NAME +++++++++++++++++++++++++++ */
	/* step2 : Model name (16 characters) */
	/* +++++++++++++++++++++++++++++++++++++++++++++++ */

	if( ATT_MODEL==isAttachMdl )
	{

		if( 0x00 == str_moduleName[0] )
	        printf("\n>>Model Name (default)  : %s", DUMMY_FILL);
		
		len_module_name = strlen(str_moduleName);
		if( len_module_name < MAX_CHARS )
		{
			if( len_module_name == 0 ) 
			{ 
				memcpy(str_moduleName, DUMMY_FILL, DUMMY_FILL_LEN); 
				len_module_name=DUMMY_FILL_LEN; 
			}
			if(outfile) fprintf(outfile,"%s", str_moduleName);
			while( len_module_name < MAX_CHARS )
			{
				if(outfile) fprintf(outfile,"%c",SPACE_FILL2);
				len_module_name++;
			}
		}
		else
		{
			count=0;
			while( count < MAX_CHARS )
			{
				if(outfile) fprintf(outfile,"%c",str_moduleName[count] );
				count++;
			}
		}
		len_attach_hdr += MAX_CHARS; // for Module Name

	}



	// --------------------------------------------------
	/* ++++++++ VERSION NAME +++++++++++++++++++++++++++ */
	/* step4 : Version name (16 characters) -> 
	   step3 : Version name 32 Bytes : 2017.11.21 */
	/* +++++++++++++++++++++++++++++++++++++++++++++++ */

	if( (ATT_VERSION==isAttachVer) || (ATT_MCU_32BYTE==isAttachVer) )
	{

		if( ATT_VERSION==isAttachVer ) /* 16Bytes */
		{
			len_version_name = strlen(str_versionName);
			if( len_version_name < MAX_VERSION_LEN )
			{
				if( len_version_name == 0 ) 
				{ 
					memcpy(str_versionName, DUMMY_FILL, DUMMY_FILL_LEN); 
					len_version_name = DUMMY_FILL_LEN; 
				}
				if(outfile) fprintf(outfile,"%s", str_versionName);
				while( len_version_name < MAX_VERSION_LEN )
				{
					if(outfile) fprintf(outfile,"%c",SPACE_FILL4);
					len_version_name++;
				}
			}
			else
			{
				count=0;
				while( count < MAX_VERSION_LEN )
				{
					if(outfile) fprintf(outfile,"%c",str_versionName[count] );
					count++;
				}
			}

			len_attach_hdr += MAX_VERSION_LEN; // for Version Name

		}
		else if(ATT_MCU_32BYTE == isAttachVer) /* 32Bytes 2022-10-18 */
		{
		
			len_version_name = strlen(str_MCUversion);
			if( len_version_name < MAX_32CHARS )
			{
				if( len_version_name == 0 ) 
				{ 
					memcpy(str_MCUversion, DUMMY_FILL, DUMMY_FILL_LEN); 
					len_version_name = DUMMY_FILL_LEN; 
				}
				if(outfile) fprintf(outfile,"%s", str_MCUversion);
				while( len_version_name < MAX_32CHARS )
				{
					if(outfile) fprintf(outfile,"%c",SPACE_FILL4);
					len_version_name++;
				}
			}
			else
			{
				count=0;
				while( count < MAX_32CHARS )
				{
					if(outfile) fprintf(outfile,"%c",str_MCUversion[count] );
					count++;
				}
			}

			len_attach_hdr += MAX_32CHARS; // for MCU Version Name
			
		}

	}
	

	/* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
	/* ============================================================ */
	/* CRC/SHA1/SHA256/MD5 etc HEADER INSERTION     ++++++++++++++ */
	/* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */

	if( ATT_DATEorCRC==isAttachCRC )
	{

		if( (1 == insertCRC) )
		{
			unsigned __int64  kll = 0UL;
		
			LOG_V("\n");
			if( HDR_CRC16 == isCRCtype )
			{
				kll = RunCRC16(infile_name, NULL, 0, 0, insertCRC, iVerbosType, str_hash);

				if( iUpper ) // 2017.04.27
				{
					if(outfile) fprintf(outfile,"%04X", g_calcCrc16);
				}
				else
				{
					if(outfile) fprintf(outfile,"%04x", g_calcCrc16);
				}
				
				len_build_date = 4;
				while( len_build_date < MAX_CRC_LEN_CODE )
				{
					if(outfile) fprintf(outfile,"%c",SPACE_FILL3);
					len_build_date++;
				}

				len_attach_hdr += MAX_CRC_LEN_CODE; // 2020.07.07, for CRC16 Name
				len_checksum = MAX_CRC_LEN_CODE; /* 2022-10-22 */

			} 	
			else if( HDR_KSC_CRC16 == isCRCtype )
			{
				kll = RunKSC_CRC16(infile_name, NULL, 0, 0, insertCRC, iVerbosType, str_hash);

				if( iUpper ) // 2017.04.27
				{
					if(outfile) fprintf(outfile,"%04X", g_calcCrc16);
				}
				else
				{
					if(outfile) fprintf(outfile,"%04x", g_calcCrc16);
				}

				len_build_date = 4;
				while( len_build_date < MAX_CRC_LEN_CODE )
				{
					if(outfile) fprintf(outfile,"%c",SPACE_FILL3);
					len_build_date++;
				}

				len_attach_hdr += MAX_CRC_LEN_CODE; // 2020.07.07, for CRC16 Name
				len_checksum = MAX_CRC_LEN_CODE; /* 2022-10-22 */

			} 	
			else if( HDR_CRC16CCITT == isCRCtype )
			{
 				kll = RunCRC16CCITT(infile_name, NULL, 0, 0, insertCRC, iVerbosType, str_hash);

				if( iUpper ) // 2017.04.27
				{
					if(outfile) fprintf(outfile,"%04X", g_calcCrc16);
				}
				else
				{
					if(outfile) fprintf(outfile,"%04x", g_calcCrc16);
				}

				len_build_date = 4;
				while( len_build_date < MAX_CRC_LEN_CODE )
				{
					if(outfile) fprintf(outfile,"%c",SPACE_FILL3);
					len_build_date++;
				}

				len_attach_hdr += MAX_CRC_LEN_CODE; // 2020.07.07, for CRC16 Name
				len_checksum = MAX_CRC_LEN_CODE; /* 2022-10-22 */

			}
			else if( HDR_CRC32 == isCRCtype  )
			{
 				kll = RunCRC32(infile_name, NULL, 0, 0, insertCRC, iVerbosType, str_hash);

				if( iUpper ) // 2017.04.27
				{
					if(outfile) fprintf(outfile,"%08X", g_calcCrc32);
				}
				else
				{
					if(outfile) fprintf(outfile,"%08x", g_calcCrc32);
				}
				
				len_build_date = 8;
				while( len_build_date < MAX_CRC_LEN_CODE )
				{
					if(outfile) fprintf(outfile,"%c",SPACE_FILL3);
					len_build_date++;
				}

				len_attach_hdr += MAX_CRC_LEN_CODE; // 2020.07.07, for CRC32 Name
				len_checksum = MAX_CRC_LEN_CODE; /* 2022-10-22 */

			}
 			else if( HDR_CRC64 == isCRCtype  )
			{
				kll = RunCRC64(infile_name, NULL, 0, 0, insertCRC, iVerbosType, str_hash);

				if( iUpper ) // 2017.04.27
				{
					if(outfile) fprintf(outfile,"%016llX", g_calcCrc64);
				}
				else
				{
					if(outfile) fprintf(outfile,"%016llx", g_calcCrc64);
				}
				
				len_build_date = 16;
				while( len_build_date < MAX_CRC_LEN_CODE )
				{
					if(outfile) fprintf(outfile,"%c",SPACE_FILL3);
					len_build_date++;
				}

				len_attach_hdr += MAX_CRC_LEN_CODE; // 2020.07.07, for CRC64 Name
				len_checksum = MAX_CRC_LEN_CODE; /* 2022-10-22 */

			}
			else if( HDR_CRC64_ISC==isCRCtype )
			{
				kll = RunCRC64_isc(infile_name, NULL, 0, 0, insertCRC, iVerbosType, str_hash);

				if( iUpper ) // 2020.07.16
				{
					if(outfile) fprintf(outfile,"%016llX", g_calcCrc64);
				}
				else
				{
					if(outfile) fprintf(outfile,"%016llx", g_calcCrc64);
				}
				
				len_build_date = 16;
				while( len_build_date < MAX_CRC_LEN_CODE )
				{
					if(outfile) fprintf(outfile,"%c",SPACE_FILL3);
					len_build_date++;
				}

				len_attach_hdr += MAX_CRC_LEN_CODE; // 2020.07.07, for CRC64 Name
				len_checksum = MAX_CRC_LEN_CODE; /* 2022-10-22 */

			}
			else if( HDR_ADLER32 == isCRCtype  ) 
			{
 				kll = RunAdler32(infile_name, NULL, 0, 0, insertCRC, iVerbosType, str_hash);

				if( iUpper ) // 2017.04.27
				{
					if(outfile) fprintf(outfile,"%08X", g_calcAdler32);
				}
				else
				{
					if(outfile) fprintf(outfile,"%08x", g_calcAdler32);
				}
				
				len_build_date = 8;
				while( len_build_date < MAX_CRC_LEN_CODE )
				{
					if(outfile) fprintf(outfile,"%c",SPACE_FILL3);
					len_build_date++;
				}

				len_attach_hdr += MAX_CRC_LEN_CODE; // 2020.07.07, for CRC64 Name
				len_checksum = MAX_CRC_LEN_CODE; /* 2022-10-22 */

			}
			else if( HDR_JOAAT == isCRCtype  ) 
			{
 				kll = RunJoaat(infile_name, NULL, 0, 0, insertCRC, iVerbosType, str_hash);

				if( iUpper )
				{
					if(outfile) fprintf(outfile,"%08X", g_calcJoaat);
				}
				else
				{
					if(outfile) fprintf(outfile,"%08x", g_calcJoaat);
				}
				
				len_build_date = 8;
				while( len_build_date < MAX_CRC_LEN_CODE )
				{
					fprintf(outfile,"%c", SPACE_FILL3);
					len_build_date++;
				}

				len_attach_hdr += MAX_CRC_LEN_CODE;
				len_checksum = MAX_CRC_LEN_CODE; /* 2022-10-22 */
			}

		#if 1 // 2017.11.21
			else if(HDR_SHA1 == isCRCtype )
			{
				ShaBuffer SHA1output;
				size_t nBytes;
				sha1_context ctx;
				unsigned char sha1_buf[SHA_READ_BUFSIZ]; /// NERVER modified!!!!
				unsigned __int64 	kll=0UL; //, ll=0UL;

				memset( &ctx, 0x00, sizeof(sha1_context) );
				memset( sha1_buf, 0x00, SHA_READ_BUFSIZ );
	
				/// SHA1 initial ----
				sha1_starts(&ctx);
	
				kll = 0UL;
				while((nBytes = fread(sha1_buf, 1, sizeof(sha1_buf), inpfile)) > 0)
				{
					kll += nBytes;
					sha1_update(&ctx, sha1_buf, (int)nBytes);
				}
				
				sha1_finish(&ctx, SHA1output);
	
				if(outfile) fprintf(outfile, "%s", SHA1output);
				printf("%s  *%s*%s__(%llu) \r\n", SHA1output, str_hash, infile_name, infile_size );

				len_attach_hdr += SHA_HASH_LENGTH; // 2020.07.07, for SHA1 Hash 
				len_checksum = SHA_HASH_LENGTH; /* 2022-10-22 */

				//free(sha1_buf);
				
				//printf("\nSHA1>> Calculated SHA1 Hash Value - OK");
			}
			else if(HDR_SHA224 == isCRCtype )
			{
				unsigned __int64 	kll=0UL, ll=0UL;
				sha224_ctx		ctx224; 	
				unsigned char	sha224_buf[SHA_READ_BUFSIZ];
				int i;
			
			
				// printf("SHA2>> SHA-224 hashing... \n");
		
				/// initial ----
				memset( &ctx224, 0x00, sizeof(sha224_ctx) );
				memset( sha224_buf, 0x00, sizeof(sha224_buf) );
	
	
				/// SHA2 Calculate ----
				sha224_init(&ctx224);
						
				kll = 0UL;
				while((ll = fread(sha224_buf, 1, sizeof(sha224_buf), inpfile)) > 0) 
				{
					kll += ll;
					sha224_update(&ctx224, sha224_buf, ll);
				}
				
				sha224_final(&ctx224, sha224_buf);
	
				//sha224Print (sha224_buf);

				if(iUpper) // upper-case
				{
					for (i = 0; i < SHA224_DIGEST_SIZE; i++)
					{
						if(outfile) fprintf(outfile, "%02X", sha224_buf[i]);
						printf("%02X", sha224_buf[i]);
					}
				}
				else
				{
					for (i = 0; i < SHA224_DIGEST_SIZE; i++)
					{
						if(outfile) fprintf(outfile, "%02x", sha224_buf[i]);
						printf("%02x", sha224_buf[i]);
					}
				}
				
				printf("  *%s*%s__(%llu) \r\n", str_hash, infile_name, infile_size );
				//printf("  *SHA224*%s__(%llu) \r\n", infile_name, infile_size );

				//fprintf(outfile,"  *SHA224*%s__(%llu) \r\n", infile_name, infile_size );
				//fprintf(outfile,"%s", sha256_buf);

				len_attach_hdr += (SHA224_DIGEST_SIZE*2); // 2020.07.07, for SHA1 Hash 
				len_checksum = (SHA224_DIGEST_SIZE*2); /* 2022-10-22 */

			}
			else if(HDR_SHA256 == isCRCtype )
			{
				unsigned __int64 	kll=0, ll=0;
				SHA256_CTX 		ctx256;
				char	sha256_buf[SHA2_BUFLEN];

		
				//printf("SHA2>> SHA-256 hashing... \n");

				/// initial
				memset( &ctx256, 0x00, sizeof(SHA256_CTX) );
				memset( sha256_buf, 0x00, sizeof(sha256_buf) );

				/// SHA2-256 Calculate ----
				SHA256_Init(&ctx256);

				kll = 0UL;
				while((ll = fread(sha256_buf, 1, SHA2_BUFLEN, inpfile)) > 0) 
				{
					kll += ll;
					SHA256_Update(&ctx256, (unsigned char*)sha256_buf, ll);

					//printf("\bSHA-256 Hashing(%s) -> read : %.1f MB \r", infile_name, (float)kll/(1024.0*1024.0) );
				}

				SHA256_End(&ctx256, sha256_buf);

				if(outfile) fprintf(outfile,"%s", sha256_buf);

				printf("%s	*%s*%s__(%llu) \r\n", sha256_buf, str_hash, infile_name, infile_size );

				len_attach_hdr += (SHA256_DIGEST_SIZE*2); // 2020.07.07, for SHA1 Hash 
				len_checksum = (SHA256_DIGEST_SIZE*2); /* 2022-10-22 */

			}				
			else if(HDR_SHA384 == isCRCtype )
			{

				unsigned __int64 	kll=0, ll=0;
				SHA384_CTX 		ctx384;
				char	sha384_buf[SHA2_BUFLEN];

				//printf("SHA2>> SHA-384 hashing... \n");

				/// initial ---
				memset( &ctx384, 0x00, sizeof(SHA384_CTX) );
				memset( sha384_buf, 0x00, sizeof(sha384_buf) );

				/// SHA2-384 Calculate ----
				SHA384_Init(&ctx384);

				kll = 0UL;
				while((ll = fread(sha384_buf, 1, SHA2_BUFLEN, inpfile)) > 0) 
				{
					kll += ll;
					SHA384_Update(&ctx384, (unsigned char*)sha384_buf, ll);
				}

				SHA384_End(&ctx384, sha384_buf);

				if(outfile) fprintf(outfile,"%s", sha384_buf);

				printf("%s  *%s*%s__(%llu) \r\n", sha384_buf, str_hash, infile_name, infile_size );
				//fprintf(outfile,"%s  *SHA384*%s__(%llu) \r\n", sha384_buf, infile_name, infile_size );

				len_attach_hdr += (SHA384_DIGEST_SIZE*2); // 2020.07.07, for SHA1 Hash 
				len_checksum = (SHA384_DIGEST_SIZE*2); /* 2022-10-22 */

			}
			else if(HDR_SHA512 == isCRCtype )
			{
				unsigned __int64 	kll=0, ll=0;
				SHA512_CTX 		ctx512;
				unsigned char	sha512_buf[SHA2_BUFLEN];

				//printf("SHA2>> SHA2-512 hashing... \n");


				/// initial
				memset( &ctx512, 0x00, sizeof(SHA512_CTX) );
				memset( sha512_buf, 0x00, sizeof(sha512_buf) );


				/// SHA2-512 Calculate ----
				SHA512_Init(&ctx512);

				kll = 0UL;
				while((ll = fread(sha512_buf, 1, SHA2_BUFLEN, inpfile)) > 0) 
				{
					kll += ll;
					SHA512_Update(&ctx512, (unsigned char*)sha512_buf, ll);
				}

				SHA512_End(&ctx512, sha512_buf);

				if(outfile) fprintf(outfile,"%s", sha512_buf);

				printf("%s  *%s*%s__(%llu) \r\n", sha512_buf, str_hash, infile_name, infile_size );
				//fprintf(outfile,"%s  *SHA512*%s__(%llu) \r\n", sha512_buf, infile_name, infile_size );

				len_attach_hdr += (SHA512_DIGEST_SIZE*2); // 2020.07.07, for SHA1 Hash 
				len_checksum = (SHA512_DIGEST_SIZE*2); /* 2022-10-22 */

			}
			else if(HDR_SHA3_224 == isCRCtype )
			{
				unsigned char sha3Buf[SHA3_BUFLEN] = {0,};
				char	sha3digestTxt[SHA3_OUTPUT_SIZ] = {0,};
				unsigned char sha3out[SHA3_OUTPUT_SIZ] = { 0, };
				
				unsigned __int64 	kll=0;
				size_t		ll=0;
				int 		ret;

				//printf("SHA3-KECCAK>> SHA3-224 hashing... \n");

				memset( sha3Buf, 0x00, sizeof(sha3Buf) );
				memset( sha3out, 0x00, sizeof(sha3out) );
				memset( sha3digestTxt, 0x00, sizeof(sha3digestTxt) );

				// Initialize the SHA3-256 context
				sha3_init(SHA3_224_HASH_BIT, SHA3_SHAKE_NONE);	

				kll = 0UL;
				while((ll = fread(sha3Buf, 1, sizeof(sha3Buf), inpfile)) > 0) 
				{
					kll += ll;
					sha3_update(sha3Buf, ll);
				}
				
				ret = sha3_final(sha3out, SHA3_OUT_224);
				
				for (ii = 0; ii < SHA3_OUT_224; ii++)
				{
					if( iUpper )
						sprintf(&sha3digestTxt[ii*2], "%02X", sha3out[ii]);
					else
						sprintf(&sha3digestTxt[ii*2], "%02x", sha3out[ii]);
				}

				if(outfile) fprintf(outfile,"%s", sha3digestTxt);
				printf("%s  *%s*%s__(%llu) \r\n", sha3digestTxt, str_hash, infile_name, infile_size   );

				len_attach_hdr += (SHA3_OUT_224*2); // 2020.07.07, for SHA1 Hash 
				len_checksum = (SHA3_OUT_224*2); /* 2022-10-22 */
	
			}
			else if(HDR_SHA3_256 == isCRCtype )
			{
				unsigned char sha3Buf[SHA3_BUFLEN] = {0,};
				char	sha3digestTxt[SHA3_OUTPUT_SIZ] = {0,};
				unsigned char sha3out[SHA3_OUTPUT_SIZ] = { 0, };
				
				unsigned __int64 	kll=0;
				size_t		ll=0;
				int 		ret;

				memset( sha3Buf, 0x00, sizeof(sha3Buf) );
				memset( sha3out, 0x00, sizeof(sha3out) );
				memset( sha3digestTxt, 0x00, sizeof(sha3digestTxt) );
				
				// Initialize the SHA3-256 context
				sha3_init(SHA3_256_HASH_BIT, SHA3_SHAKE_NONE);	

				kll = 0UL;
				while((ll = fread(sha3Buf, 1, sizeof(sha3Buf), inpfile)) > 0) 
				{
					kll += ll;
					sha3_update(sha3Buf, ll);
				}
				
				ret = sha3_final(sha3out, SHA3_OUT_256);
				
				for (ii = 0; ii < SHA3_OUT_256; ii++)
				{
					if( iUpper )
						sprintf(&sha3digestTxt[ii*2], "%02X", sha3out[ii]);
					else
						sprintf(&sha3digestTxt[ii*2], "%02x", sha3out[ii]);
				}

				if(outfile) fprintf(outfile,"%s", sha3digestTxt);
				printf("%s  *%s*%s__(%llu) \r\n", sha3digestTxt, str_hash, infile_name, infile_size   );

				len_attach_hdr += (SHA3_OUT_256*2); // 2020.07.07, for SHA1 Hash 
				len_checksum = (SHA3_OUT_256*2); /* 2022-10-22 */

			}
			else if(HDR_SHA3_384 == isCRCtype )
			{
				unsigned char sha3Buf[SHA3_BUFLEN] = {0,};
				char	sha3digestTxt[SHA3_OUTPUT_SIZ] = {0,};
				unsigned char sha3out[SHA3_OUTPUT_SIZ] = { 0, };
				
				unsigned __int64 	kll=0;
				size_t		ll=0;
				int 		ret;

				memset( sha3Buf, 0x00, sizeof(sha3Buf) );
				memset( sha3out, 0x00, sizeof(sha3out) );
				memset( sha3digestTxt, 0x00, sizeof(sha3digestTxt) );
				
				// Initialize the SHA3-384 context
				sha3_init(SHA3_384_HASH_BIT, SHA3_SHAKE_NONE);	

				kll = 0UL;
				while((ll = fread(sha3Buf, 1, sizeof(sha3Buf), inpfile)) > 0) 
				{
					kll += ll;
					sha3_update(sha3Buf, ll);
				}
				
				ret = sha3_final(sha3out, SHA3_OUT_384);
				
				for (ii = 0; ii < SHA3_OUT_384; ii++)
				{
					if( iUpper )
						sprintf(&sha3digestTxt[ii*2], "%02X", sha3out[ii]);
					else
						sprintf(&sha3digestTxt[ii*2], "%02x", sha3out[ii]);
				}
				
				if(outfile) fprintf(outfile,"%s", sha3digestTxt);
				printf("%s  *%s*%s__(%llu) \r\n", sha3digestTxt, str_hash, infile_name, infile_size   );

				len_attach_hdr += (SHA3_OUT_384*2); // 2020.07.07, for SHA1 Hash 
				len_checksum = (SHA3_OUT_384*2); /* 2022-10-22 */

			}
			else if(HDR_SHA3_512 == isCRCtype )
			{
				unsigned char sha3Buf[SHA3_BUFLEN] = {0,};
				char	sha3digestTxt[SHA3_OUTPUT_SIZ] = {0,};
				unsigned char sha3out[SHA3_OUTPUT_SIZ] = { 0, };
				
				unsigned __int64 	kll=0;
				size_t		ll=0;
				int 		ret;

				memset( sha3Buf, 0x00, sizeof(sha3Buf) );
				memset( sha3out, 0x00, sizeof(sha3out) );
				memset( sha3digestTxt, 0x00, sizeof(sha3digestTxt) );
				
				// Initialize the SHA3-512 context
				sha3_init(SHA3_512_HASH_BIT, SHA3_SHAKE_NONE);	

				kll = 0UL;
				while((ll = fread(sha3Buf, 1, sizeof(sha3Buf), inpfile)) > 0) 
				{
					kll += ll;
					sha3_update(sha3Buf, ll);
				}
				
				ret = sha3_final(sha3out, SHA3_OUT_512);
				
				for (ii = 0; ii < SHA3_OUT_512; ii++)
				{
					if( iUpper )
						sprintf(&sha3digestTxt[ii*2], "%02X", sha3out[ii]);
					else
						sprintf(&sha3digestTxt[ii*2], "%02x", sha3out[ii]);
				}
				

				if(outfile) fprintf(outfile,"%s", sha3digestTxt);
				printf("%s  *%s*%s__(%llu) \r\n", sha3digestTxt, str_hash, infile_name, infile_size   );

				len_attach_hdr += (SHA3_OUT_512*2); // 2020.07.07, for SHA1 Hash 
				len_checksum = (SHA3_OUT_512*2); /* 2022-10-22 */

			}
			else if( HDR_SHAKE128 == isCRCtype )
			{
				unsigned char sha3Buf[SHA3_BUFLEN] = {0,};
				char	sha3digestTxt[SHA3_OUTPUT_SIZ] = {0,};
				unsigned char sha3out[SHA3_OUTPUT_SIZ] = { 0, };
				
				unsigned __int64 	kll=0;
				size_t		ll=0;
				int 		ret;

				memset( sha3Buf, 0x00, sizeof(sha3Buf) );
				memset( sha3out, 0x00, sizeof(sha3out) );
				memset( sha3digestTxt, 0x00, sizeof(sha3digestTxt) );
				
				// Initialize the SHAKE128 context
				sha3_init(SHAKE_128_HASH_BIT, SHA3_SHAKE_USE);	

				kll = 0UL;
				while((ll = fread(sha3Buf, 1, sizeof(sha3Buf), inpfile)) > 0) 
				{
					kll += ll;
					sha3_update(sha3Buf, ll);
				}
				
				ret = sha3_final(sha3out, SHAKE_OUT_128);
				
				for (ii = 0; ii < SHAKE_OUT_128; ii++)
				{
					if( iUpper )
						sprintf(&sha3digestTxt[ii*2], "%02X", sha3out[ii]);
					else
						sprintf(&sha3digestTxt[ii*2], "%02x", sha3out[ii]);
				}
					

				if(outfile) fprintf(outfile,"%s", sha3digestTxt);
				printf("%s  *%s*%s__(%llu) \r\n", sha3digestTxt, str_hash, infile_name, infile_size   );

				len_attach_hdr += (SHAKE_OUT_128*2); // 2020.07.07, for SHA1 Hash 
				len_checksum = (SHAKE_OUT_128*2); /* 2022-10-22 */

			}
			else if( HDR_SHAKE256 == isCRCtype )
			{
				unsigned char sha3Buf[SHA3_BUFLEN] = {0,};
				char	sha3digestTxt[SHA3_OUTPUT_SIZ] = {0,};
				unsigned char sha3out[SHA3_OUTPUT_SIZ] = { 0, };
				
				unsigned __int64 	kll=0;
				size_t		ll=0;
				int 		ret;
				
				memset( sha3Buf, 0x00, sizeof(sha3Buf) );
				memset( sha3out, 0x00, sizeof(sha3out) );
				memset( sha3digestTxt, 0x00, sizeof(sha3digestTxt) );
				
				// Initialize the SHAKE128 context
				sha3_init(SHAKE_256_HASH_BIT, SHA3_SHAKE_USE);	

				kll = 0UL;
				while((ll = fread(sha3Buf, 1, sizeof(sha3Buf), inpfile)) > 0) 
				{
					kll += ll;
					sha3_update(sha3Buf, ll);
				}
				
				ret = sha3_final(sha3out, SHAKE_OUT_256);
				
				for (ii = 0; ii < SHAKE_OUT_256; ii++)
				{
					if( iUpper )
						sprintf(&sha3digestTxt[ii*2], "%02X", sha3out[ii]);
					else
						sprintf(&sha3digestTxt[ii*2], "%02x", sha3out[ii]);
				}
					

				if(outfile) fprintf(outfile,"%s", sha3digestTxt);
				printf("%s  *%s*%s__(%llu) \r\n", sha3digestTxt, str_hash, infile_name, infile_size   );

				len_attach_hdr += (SHAKE_OUT_256*2); // 2020.07.07, for SHA1 Hash 
				len_checksum = (SHAKE_OUT_256*2); /* 2022-10-22 */

			}
			else if(HDR_MD5 == isCRCtype )
			{
				/* Computes the message digest for a specified file.
				Prints out message digest, a space, the file name, and a carriage return.
				*/
				MD5_CTX mdContext;
				unsigned char md5_data[MD_HASH_BUFSIZ];
				unsigned __int64  nBytes = 0UL, kll = 0UL;

				//printf("MD5>> MD5 hashing... \n");
	
				/// initial
				memset( &mdContext, 0x00, sizeof(MD5_CTX) );
				memset( &md5_data, 0x00, sizeof(md5_data) );
	
	
				MD5Init (&mdContext);
	
				kll = 0UL;
				while ((nBytes = fread (md5_data, 1, sizeof(md5_data), inpfile)) != 0)
				{
					kll += nBytes;				
					MD5Update (&mdContext, md5_data, nBytes);
				}

				MD5Final(&mdContext);
				MDPrint(&mdContext);
	
				printf ("  *MD5*%s__(%llu) \r\n", infile_name, infile_size);
				//fprintf(outfile,"  *MD5*%s__(%llu) \r\n", infile_name, infile_size );

			#if 0
				len_build_date = 32;
				while( len_build_date < MAX_HASH_CODE )
				{
					fprintf(outfile,"%c",SPACE_FILL3);
					len_build_date++;
				}
			#endif

				len_attach_hdr += (16*2); // 2020.07.07, for SHA1 Hash 
				len_checksum = (16*2); /* 2022-10-22 */

			}
			else if(HDR_MD6 == isCRCtype )
			{

				unsigned long long nBytes = 0UL, kll = 0UL;
				unsigned char md6_data[MD_HASH_BUFSIZ];
				//double elapsed_time = end_time - start_time;
				//unsigned long long elapsed_ticks = end_ticks - start_ticks;


				/* -------------------------------------------------- */
				/* ------ MD6 : set default md6 parameter settings ------ */
				md6_dgtLen = 256;           /* digest length */
				md6_keylen = 0;             /* key length in bytes (at most 64) */
				md6_modPar = 64;            /* mode parameter */
				md6_roundN = md6_default_r(md6_dgtLen,md6_keylen);  /* number of rounds */
				md6_use_default_r = 1;     /* 1 if r should be set to the default, 0 if r is explicitly provided */
				/* -------------------------------------------------- */

		
				//printf("MD6>> MD6 hashing... \n");

				/// initial
				nBytes = 0UL;
				memset( md6_data, 0x00, sizeof(md6_data) );

				/// MD6 initial ----
				hash_init();

				kll = 0UL;
				while((nBytes = fread(md6_data, 1, sizeof(md6_data), inpfile)) > 0) 
				{
					kll += nBytes;
					hash_update(md6_data,nBytes*8);
				}

				hash_final();

				if(outfile) fprintf(outfile,"%s", md6_st.hexhashval);
				
				printf("%s  *MD6*%s__(%llu)  \n", md6_st.hexhashval, infile_name, infile_size  );
				//fprintf(outfile,"%s  *MD6*%s__(%llu) \r\n", md6_st.hexhashval, infile_name, infile_size );


			#if 0
				len_build_date = 64;
				while( len_build_date < MAX_HASH_CODE )
				{
					fprintf(outfile,"%c",SPACE_FILL3);
					len_build_date++;
				}
			#endif

				len_attach_hdr += (32*2); // 2020.07.07, for SHA1 Hash 
				len_checksum = (32*2); /* 2022-10-22 */

			}	
			else if(HDR_MD2 == isCRCtype )
			{
				MD2_CTX mdContext;
				size_t nBytes;
				unsigned char md2_data[MD_HASH_BUFSIZ];
				unsigned char md2_output[MD2_DIGEST_LENGTH] = {0,};
				
				//printf("MD2>> MD2 hashing... \n");
	
				/// initial
				memset( &mdContext, 0x00, sizeof(MD2_CTX) );
				memset( &md2_data, 0x00, sizeof(md2_data) );
				memset( &md2_output, 0x00, sizeof(md2_output) );
	
				MD2_Init(&mdContext);
	
				kll = 0UL;				
				while ((nBytes = fread (md2_data, 1, sizeof(md2_data), inpfile)) != 0)
				{
					kll += nBytes;
					MD2_Update(&mdContext, md2_data, nBytes);
	
					//printf("\bMD2 Hashing(%s) -> read : %.1f MB \r", infile_name, (float)(nBytes*(isize++))/(1024.0*1024.0) );
				}

				MD2_Final( md2_output, &mdContext );
				MD2Print( md2_output );
	
				printf ("  *MD2*%s__(%llu) \n", infile_name, infile_size );
	
				len_attach_hdr += (MD2_DIGEST_LENGTH*2); // 2020.07.15, for MD2 Hash 
				len_checksum = (MD2_DIGEST_LENGTH*2); /* 2022-10-22 */
	
			}
			else if(HDR_MD4 == isCRCtype )
			{
				MD4_CTX mdContext;
				int nBytes;
				unsigned char md4_data[MD_HASH_BUFSIZ];
				unsigned char md4_output[MD4_DIGEST_LENGTH] = {0,};
				
				//printf("MD4>> MD4 hashing... \n");
	
				/// initial
				memset( &mdContext, 0x00, sizeof(MD4_CTX) );
				memset( &md4_data, 0x00, sizeof(md4_data) );
				memset( &md4_output, 0x00, sizeof(md4_output) );
	
	
				MD4Init (&mdContext);
	
				kll = 0UL;
				while ((nBytes = fread (md4_data, 1, sizeof(md4_data) /*1024*/, inpfile)) != 0)
				{
					kll += nBytes;
					MD4Update (&mdContext, md4_data, nBytes);

					//printf("\bMD4 Hashing(%s) -> read : %.1f MB \r", infile_name, (float)(nBytes*(isize++))/(1024.0*1024.0) );
				}

				MD4Final(md4_output, &mdContext);
				MD4Print(md4_output);
	
				printf ("  *MD4*%s__(%llu) \r\n", infile_name, infile_size);

				len_attach_hdr += (MD4_DIGEST_LENGTH*2); // 2020.07.15, for MD4 Hash 
				len_checksum = (MD4_DIGEST_LENGTH*2); /* 2022-10-22 */

			}

		#if BLAKE_224_256_384_512_HASH
			else if( HDR_BLAKE224 == isCRCtype )
			{
				unsigned char inbuf[BLOCK224], out[BLAKE224_LEN]={0,}; 
				char outTxt[BLAKE224_LEN*2]={0,}; 
				state224 blakeS; 
				unsigned __int64 kll = 0UL;
				size_t		ll=0;
				
				memset( inbuf, 0x00, sizeof(inbuf) );
				memset( out, 0x00, sizeof(out) );
				
				//Initialize the BLAKE-224 context
				blake224_init( &blakeS ); 
				
				kll = 0ULL;
				while((ll = fread(inbuf, 1, sizeof(inbuf), inpfile)) > 0) 
				{
					kll += ll;
					//Absorb input data
					blake224_update( &blakeS, inbuf, ll ); 				
				}
				
				//Finish absorbing phase
				blake224_final( &blakeS, out ); 
				
				
				memset( outTxt, 0x00, sizeof(outTxt) );
				for (ii = 0; ii < BLAKE224_LEN; ii++) 
				{
					if( iUpper )
						sprintf(&outTxt[ii*2], (char*)"%02X", out[ii]);
					else
						sprintf(&outTxt[ii*2], (char*)"%02x", out[ii]);
				}
				
				if(outfile) fprintf(outfile,"%s", outTxt);
				printf("%s  *%s*%s__(%llu) \r\n", outTxt, str_hash, infile_name, infile_size   );
			
				len_attach_hdr += (BLAKE224_LEN*2);
				len_checksum = (BLAKE224_LEN*2); /* 2022-10-22 */

			}
			else if( HDR_BLAKE256 == isCRCtype )
			{
				unsigned char inbuf[BLOCK256], out[BLAKE256_LEN]={0,}; 
				char outTxt[BLAKE256_LEN*2]={0,}; 
				state256 blakeS;	
				unsigned __int64 kll = 0UL;
				size_t		ll=0;
				
				memset( inbuf, 0x00, sizeof(inbuf) );
				memset( out, 0x00, sizeof(out) );

				//Initialize the BLAKE-256 context
				blake256_init( &blakeS ); 
				
				kll = 0ULL;
				while((ll = fread(inbuf, 1, sizeof(inbuf), inpfile)) > 0) 
				{
					kll += ll;
					//Absorb input data
					blake256_update( &blakeS, inbuf, ll ); 
				}
			
				//Finish absorbing phase
				blake256_final( &blakeS, out ); 
				
				memset( outTxt, 0x00, sizeof(outTxt) );
				for (ii = 0; ii < BLAKE256_LEN; ii++)
				{
					if( iUpper )
						sprintf(&outTxt[ii*2], (char*)"%02X", out[ii]);
					else
						sprintf(&outTxt[ii*2], (char*)"%02x", out[ii]);
				}

				if(outfile) fprintf(outfile,"%s", outTxt);
				printf("%s  *%s*%s__(%llu) \r\n", outTxt, str_hash, infile_name, infile_size   );
			
				len_attach_hdr += (BLAKE256_LEN*2);
				len_checksum = (BLAKE256_LEN*2); /* 2022-10-22 */

			}
			else if( HDR_BLAKE384 == isCRCtype )
			{
				unsigned char inbuf[BLOCK384], out[BLAKE384_LEN] ={0,}; 
				char outTxt[BLAKE384_LEN*2]={0,}; 
				state384 blakeS;
				unsigned __int64 kll = 0UL;
				size_t		ll=0;
				
				memset( inbuf, 0x00, sizeof(inbuf) );
				memset( out, 0x00, sizeof(out) );
				
				//Initialize the BLAKE-384 context
				blake384_init( &blakeS ); 
				
				kll = 0ULL;
				while((ll = fread(inbuf, 1, sizeof(inbuf), inpfile)) > 0) 
				{
					kll += ll;				
					//Absorb input data
					blake384_update( &blakeS, inbuf, ll ); 
				}
				
				//Finish absorbing phase
				blake384_final( &blakeS, out ); 

				memset( outTxt, 0x00, sizeof(outTxt) );
				for (ii = 0; ii < BLAKE384_LEN; ii++)
				{
					if( iUpper )
						sprintf(&outTxt[ii*2], (char*)"%02X", out[ii]);
					else
						sprintf(&outTxt[ii*2], (char*)"%02x", out[ii]);
				}

				if(outfile) fprintf(outfile,"%s", outTxt);
				printf("%s  *%s*%s__(%llu) \r\n", outTxt, str_hash, infile_name, infile_size   );
			
				len_attach_hdr += (BLAKE384_LEN*2);
				len_checksum = (BLAKE384_LEN*2); /* 2022-10-22 */

			}
			else if( HDR_BLAKE512 == isCRCtype )
			{
				unsigned char inbuf[BLOCK512], out[BLAKE512_LEN]={0,}; 
				char outTxt[BLAKE512_LEN*2]={0,}; 
				state512 blakeS; 
				unsigned __int64 kll = 0UL;
				size_t		ll=0;
				
				memset( inbuf, 0x00, sizeof(inbuf) );
				memset( out, 0x00, sizeof(out) );
				
				//Initialize the BLAKE512 context
				blake512_init( &blakeS ); 
				
				kll = 0ULL;
				while((ll = fread(inbuf, 1, sizeof(inbuf), inpfile)) > 0) 
				{
					kll += ll;				
					//Absorb input data
					blake512_update( &blakeS, inbuf, ll ); 
				}

				//Finish absorbing phase
				blake512_final( &blakeS, out ); 

				memset( outTxt, 0x00, sizeof(outTxt) );
				for (ii = 0; ii < BLAKE512_LEN; ii++)
				{
					if( iUpper )
						sprintf(&outTxt[ii*2], (char*)"%02X", out[ii]);
					else
						sprintf(&outTxt[ii*2], (char*)"%02x", out[ii]);
				}

				if(outfile) fprintf(outfile,"%s", outTxt);
				printf("%s  *%s*%s__(%llu) \r\n", outTxt, str_hash, infile_name, infile_size   );
			
				len_attach_hdr += (BLAKE512_LEN*2);
				len_checksum = (BLAKE512_LEN*2); /* 2022-10-22 */

			}
		#endif

		#if defined(RIPEMD160) || defined(RIPEMD128)	
			else if(HDR_RMD128 == isCRCtype )
			{
#define RMD128_INBUF_SIZ 		(1024)  // NOT NOT NEVER modify~~~
#define RMD128_BUF_LEN 			5 // 160/32
	
				unsigned char inbuf[RMD128_INBUF_SIZ];
				char outTxt[RMD128_INBUF_SIZ*2]={0,}; 
				
				unsigned long MDbuf[RMD128_BUF_LEN];   /* contains (A, B, C, D(, E))   */
				unsigned char hashcode[RMD128_DIGEST_SIZE]; /* for final hash-value		   */


				unsigned __int64	kll;
				size_t		nbytes; // ll;
				unsigned long X[16];		  /* current 16-word chunk	   */
				unsigned int  i, j; 		  /* counters					   */
				unsigned long length[2];	  /* length in bytes of message   */
				unsigned long offset;		  /* # of unprocessed bytes at	  */
	
	
				memset( inbuf, 0x00, sizeof(inbuf) ); // 2018.06.15
					
				//Initialize the RIPEMD-160 context
				MD128init(MDbuf);
				length[0] = 0;
				length[1] = 0;
	
				kll = 0UL;
				while((nbytes = fread(inbuf, 1, sizeof(inbuf), inpfile)) > 0) 
				{
					kll += nbytes;
			
					/* process all complete blocks */
					for (i=0; i<(nbytes>>6); i++) {
					   for (j=0; j<16; j++)
						  X[j] = BYTES_TO_DWORD(inbuf+64*i+4*j);
					   MD128compress(MDbuf, X);
					}
					/* update length[] */
					if (length[0] + nbytes < length[0])
					   length[1]++; 				 /* overflow to msb of length */
					length[0] += nbytes;
	
				}

				//Finish absorbing phase		
				/* finish: */
				offset = length[0] & 0x3C0;   /* extract bytes 6 to 10 inclusive */
				MD128finish(MDbuf, inbuf+offset, length[0], length[1]);
	
				for (ii=0; ii<RMD128_DIGEST_SIZE; ii+=4) 
				{
					hashcode[ii]   = (unsigned char)(MDbuf[ii>>2]);
					hashcode[ii+1] = (unsigned char)(MDbuf[ii>>2] >>  8);
					hashcode[ii+2] = (unsigned char)(MDbuf[ii>>2] >> 16);
					hashcode[ii+3] = (unsigned char)(MDbuf[ii>>2] >> 24);
				}
			
				memset( outTxt, 0x00, sizeof(outTxt) );
				for (ii = 0; ii < RMD128_DIGEST_SIZE; ii++)
				{
					sprintf(&outTxt[ii*2], (char*)"%02x", hashcode[ii]);
				}
				

				printf("%s  *RMD128*%s__(%llu) \r\n", outTxt, infile_name, infile_size );
				if(outfile) fprintf(outfile, "%s", outTxt);

				len_attach_hdr += (RMD128_DIGEST_SIZE*2);
				len_checksum = (RMD128_DIGEST_SIZE*2); /* 2022-10-22 */

			}
			else if(HDR_RMD160 == isCRCtype )
			{
						
#define RMD160_INBUF_SIZ 		(1024) // NOT NOT NEVER modify~~~
#define RMD160_BUF_LEN 			5 // 160/32


				unsigned char inbuf[RMD160_INBUF_SIZ];
				char outTxt[RMD160_INBUF_SIZ*2]={0,}; 
				
				unsigned long MDbuf[RMD160_BUF_LEN];   /* contains (A, B, C, D(, E))   */
				unsigned char hashcode[RMD160_DIGEST_SIZE]; /* for final hash-value		   */
				
				unsigned __int64	kll;
				size_t		nbytes; // ll;
				unsigned long X[16];		  /* current 16-word chunk	   */
				unsigned int  i, j; 		  /* counters					   */
				unsigned long length[2];	  /* length in bytes of message   */
				unsigned long offset;		  /* # of unprocessed bytes at	  */


				memset( inbuf, 0x00, sizeof(inbuf) ); // 2018.06.15
				memset( outTxt, 0x00, sizeof(outTxt) ); // 2018.06.15
					
				//Initialize the RIPEMD-160 context
				MD160init(MDbuf);
				length[0] = 0;
				length[1] = 0;

				kll = 0UL;
				while((nbytes = fread(inbuf, 1, sizeof(inbuf), inpfile)) > 0) 
				{
					kll += nbytes;
			
					/* process all complete blocks */
					for (i=0; i<(nbytes>>6); i++) {
					   for (j=0; j<16; j++)
						  X[j] = BYTES_TO_DWORD(inbuf+64*i+4*j);
					   MD160compress(MDbuf, X);
					}
					/* update length[] */
					if (length[0] + nbytes < length[0])
					   length[1]++; 				 /* overflow to msb of length */
					length[0] += nbytes;

				}

				//Finish absorbing phase		
				/* finish: */
				offset = length[0] & 0x3C0;   /* extract bytes 6 to 10 inclusive */
				MD160finish(MDbuf, inbuf+offset, length[0], length[1]);

				for (ii=0; ii<RMD160_DIGEST_SIZE; ii+=4) 
				{
					hashcode[ii]   = (unsigned char)(MDbuf[ii>>2]);
					hashcode[ii+1] = (unsigned char)(MDbuf[ii>>2] >>  8);
					hashcode[ii+2] = (unsigned char)(MDbuf[ii>>2] >> 16);
					hashcode[ii+3] = (unsigned char)(MDbuf[ii>>2] >> 24);
				}
			
				memset( outTxt, 0x00, sizeof(outTxt) );
				for (ii = 0; ii <RMD160_DIGEST_SIZE; ii++)
				{
					sprintf(&outTxt[ii*2], (char*)"%02x", hashcode[ii]);
				}
				
				printf("%s  *RMD160*%s__(%llu) \r\n", outTxt, infile_name, infile_size );
				if(outfile) fprintf(outfile, "%s", outTxt);
				
				len_attach_hdr += (RMD160_DIGEST_SIZE*2);
				len_checksum = (RMD160_DIGEST_SIZE*2); /* 2022-10-22 */

			}	
		#endif		

			else
			{
				printf("\n[++CRC ADD_HEADER ERROR++] Unknown isCRCtype = %d \n\n", isCRCtype);
			}
		#endif // 2017.11.21


		/////////////////////////////////////////////////////////////////////////
		/////////////////////////////////////////////////////////////////////////////////


			if(inpfile) { fclose(inpfile); inpfile=NULL; } // file close for checksum


			// ==========================================
			// ==========================================
			/* ========== INPUT FILE ================= */
			if( NULL == (inpfile = fopen( infile_name, "rb")) ) 
			{
				beep(700,100);
				printf("\n\n[++ERROR++] Can not open input file[%s]. Use --input option!! \n",infile_name);
				if( NULL == infile_name || infile_name[0] == 0x00)
					printf("[++ERROR++] Must be input file with -i or --input option. \n" );
			
				AllFilesClosed();

				exit(0); /// help();
				return 0;
			}
			else
			{
				/// OK ---
				if (fstat(fileno(inpfile), &file_statbuf) < 0) 
				{ 
					printf("\n\n[++ERROR++]Cannot stat [%s]\n", infile_name ); 

					AllFilesClosed(); // 2020.07.10
					exit(0);
					return 2; 
				}	 
			}
			/* ========== INPUT FILE ================= */
			// ==========================================
			// ==========================================

		}
		else if( 0==strcasecmp( str_crcAdd, "date" ) ) // inserted date
		{
			// --- Insert date ---
			/* step3 : Compile Date (32 characters) */
			len_build_date = strlen(str_abuild_date);
			if( len_build_date < MAX_CHARS )
			{
				if( len_build_date == 0 ) 
				{ 
					len_build_date = sizeof("2017D04TUE123456");
					memcpy(str_abuild_date, "2017D04TUE123456", len_build_date);
				}

				if(outfile) fprintf(outfile,"%s", str_abuild_date);

				while( len_build_date < MAX_CHARS )
				{
					if(outfile) fprintf(outfile,"%c",SPACE_FILL3);
					len_build_date++;
				}
			}
			else
			{
				count=0;
				while( count < MAX_CHARS )
				{
					if(outfile) fprintf(outfile,"%c", str_abuild_date[count] );
					count++;
				}
			}

			len_attach_hdr += MAX_CHARS; // 2020.07.07, for date 
			len_checksum = MAX_CHARS; /* 2022-10-22 */

		}
		/* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
		/* HEADER INSERTION ENDED */
		/* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
	}
	

		
		/* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
		/* ++ Encapsulation Header ++ */
		/* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */

		/// Attached Header --- 2014.07.15
		if(inpfile && outfile)
		{
			unsigned char read_buf[RD_BUF_SIZ] = {0,};

			//printf("\nHDR>> Attaching Header... ");
			memset( read_buf, 0x00, sizeof(read_buf) ); /// initial
			
			while( fr_size = fread(read_buf, sizeof(unsigned char), (RD_BUF_SIZ), inpfile ) )
			{
				fwrite(read_buf, sizeof(unsigned char), fr_size, outfile); 
			}

			printf("\nHDR>> Attached Header!! (%dBytes) - OK", len_attach_hdr);

			if( len_checksum && (ATT_DATEorCRC==isAttachCRC) && (1==insertCRC) )
			{
				printf("\n      Encapsulated checksum (%s) length : %d ", str_crcAdd, len_checksum);
			}
		}
		else
		{
			printf("\nHDR>> Can NOT open/write input file[%s] or output file[%s] \n", infile_name, outfile_name);
		}

		AllFilesClosed();

		//// -------------------------------------------------------------



	} /* 0 == isDelHdr && 0==isCRC */
	/* 2. Header 삭제하기 */
	else if( 1 == isDelHdr )
	{
		/* 16*4 byte 이동 */
		///printf("\n>>Moved Position 16*4 ");

		if( 0==iHdr_len ) 
			iHdr_len=16*4; // default

		if(inpfile) fseek(inpfile, iHdr_len, SEEK_SET);  /* -- (16 Bytes * 4 Lines) : SEEK_CUR, SEEK_END */


		/// 2014.07.15
		if(inpfile && outfile)
		{
			unsigned char read_buf[BUFSIZE];

			printf("\nHDR>> Deleting Header size: %d Bytes", iHdr_len);
			memset( read_buf, 0x00, sizeof(read_buf) ); /// initial
			while( fr_size = fread(read_buf, sizeof(unsigned char), BUFSIZE, inpfile ) )
			{
				fwrite(read_buf, sizeof(unsigned char), fr_size, outfile); 
			}

			printf("\nHDR>> Deleted Header!! - OK");
		}
		else
		{
			printf("\nHDR>> Can NOT write/open output file[%s] or input file[%s] \n", outfile_name, infile_name );
		}

		AllFilesClosed();

		//// -------------------------------------------------------------

	}
	else if( 1 == isBMPreverse )
	{

#ifdef  __ING___

	#define WIDTH 		800
	#define HIEGHT 		480
	#define BMP_RESOL 	(WIDTH*HIEGHT)*4 /// 32bit
	
		unsigned char read_buf[BMP_RESOL+800*480];
		unsigned int idx=1;
		unsigned int ibmpIndex = 0;

		printf("ibmpIndex = %d \n", ibmpIndex);

		fr_size = fread(read_buf, sizeof(unsigned char), 54, inpfile );
		fwrite(read_buf, sizeof(unsigned char), fr_size, outfile); 

		if(inpfile && outfile)
		{
			memset( read_buf, 0xff, sizeof(read_buf) );
			for( idx=1; idx<HIEGHT; idx+=1 ) 
			{
				ibmpIndex=WIDTH*(HIEGHT-idx)*4;
				printf("ibmpIndex = %d, idx=%d \n", ibmpIndex, idx);
				fseek(inpfile, ibmpIndex, SEEK_SET);
				fr_size = fread(read_buf, sizeof(unsigned char), BMP_RESOL, inpfile );
				fwrite(read_buf, sizeof(unsigned char), fr_size, outfile); 
			}
			printf("\nBMP Reverse>> Convert BMP!!! - OK");

		}
		else
		{
			printf("\nBMP Reverse>> Can not convert bmp \n");
		}

		AllFilesClosed();

		//// -------------------------------------------------------------
#endif
	

	}
	
	/* 3. CRC 생성하기 */
	else if( 1 == isCRC )
	{
		unsigned int iLenSub=0;
		char cdate[30];
		time_t	sys_t;
		int ilen;
		struct	tm *pTime;

		pTime = current_time();

		time( &sys_t );
		
		memset( cdate, 0x00, sizeof(cdate) );
		strcpy( cdate, ctime( &sys_t ) );
		ilen = strlen(cdate);
		cdate[ilen-0] = '\0';
		cdate[ilen-1] = '\0';
		/// cdate = Sun Jun 29 12:09:12 2014		

		
		LOG_V("\n");

	
	#if 1 /// multi-input files
		if( HDR_CRC16==isCRCtype )
		{
			printf("CRC16>> ");

			if( (1==iVerbosType || 3==iVerbosType) && (TRUE == verbose) && outfile ) /// 2014.08.05
			{
				if(outfile) 
				{
					fprintf(outfile, "----------------------------------------------------------------------------------\r\n" );
					fprintf(outfile, "CRC16 checksum is created at %s by %s \n", cdate, EmailText );
					fprintf(outfile, "----------------------------------------------------------------------------------\r\n" );
				}
			}
		}
		else if( HDR_KSC_CRC16==isCRCtype )
		{
			printf("KSC-CRC16>> ");

			if( (1==iVerbosType || 3==iVerbosType) && (TRUE == verbose) && outfile ) /// 2014.08.05
			{
				if(outfile) 
				{
					fprintf(outfile, "----------------------------------------------------------------------------------\r\n" );
					fprintf(outfile, "KSC-CRC16 checksum is created at %s by %s \n", cdate, EmailText );
					fprintf(outfile, "----------------------------------------------------------------------------------\r\n" );
				}
			}
		}
		else if( HDR_CRC16CCITT==isCRCtype )
		{
			printf("CRC16CCITT>> ");
			if( (1==iVerbosType || 3==iVerbosType) && (TRUE == verbose) && outfile ) /// 2014.08.05
			{
				if(outfile) 
				{
					fprintf(outfile, "----------------------------------------------------------------------------------\r\n" );
					fprintf(outfile, "CRC16CCITT checksum is created at %s by %s \n", cdate, EmailText );
					fprintf(outfile, "----------------------------------------------------------------------------------\r\n" );
				}
			}
		}
		else if( HDR_CRC32==isCRCtype )
		{
			printf("CRC32>> ");
			if( (1==iVerbosType || 3==iVerbosType) && (TRUE == verbose) && outfile ) /// 2014.08.05
			{
				if(outfile) 
				{
					fprintf(outfile, "----------------------------------------------------------------------------------\r\n" );
					fprintf(outfile, "CRC32 checksum is created at %04d/%02d/%02d/%s %02d:%02d:%02d by %s \n", pTime->tm_year, pTime->tm_mon, pTime->tm_mday,  
														WeekTXT[pTime->tm_wday], pTime->tm_hour, pTime->tm_min, pTime->tm_sec, EmailText );
					fprintf(outfile, "----------------------------------------------------------------------------------\r\n" );
				}
			}
		}
		else if( HDR_CRC64==isCRCtype )
		{
			printf("CRC64>> ");
			if( (1==iVerbosType || 3==iVerbosType) && (TRUE == verbose) && outfile ) /// 2014.08.05
			{
				if(outfile) 
				{
					fprintf(outfile, "----------------------------------------------------------------------------------\r\n" );
					fprintf(outfile, "CRC64 checksum is created at %04d/%02d/%02d/%s %02d:%02d:%02d by %s \n", pTime->tm_year, pTime->tm_mon, pTime->tm_mday,  
														WeekTXT[pTime->tm_wday], pTime->tm_hour, pTime->tm_min, pTime->tm_sec, EmailText );
					fprintf(outfile, "----------------------------------------------------------------------------------\r\n" );
				}
			}
		}
		else if( HDR_CRC64_ISC==isCRCtype )
		{
			printf("CRC64ISC>> ");
			if( (1==iVerbosType || 3==iVerbosType) && (TRUE == verbose) && outfile ) /// 2014.08.05
			{
				if(outfile) 
				{
					fprintf(outfile, "----------------------------------------------------------------------------------\r\n" );
					fprintf(outfile, "CRC64ISC checksum is created at %04d/%02d/%02d/%s %02d:%02d:%02d by %s \n", pTime->tm_year, pTime->tm_mon, pTime->tm_mday,  
														WeekTXT[pTime->tm_wday], pTime->tm_hour, pTime->tm_min, pTime->tm_sec, EmailText );
					fprintf(outfile, "----------------------------------------------------------------------------------\r\n" );
				}
			}
		}
		else if( HDR_ADLER32==isCRCtype )
		{
			printf("ADLER32>> ");
			if( (1==iVerbosType || 3==iVerbosType) && (TRUE == verbose) && outfile ) /// 2014.08.05
			{
				if(outfile) 
				{
					fprintf(outfile, "----------------------------------------------------------------------------------\r\n" );
					fprintf(outfile, "ADLER32 checksum is created at %04d/%02d/%02d/%s %02d:%02d:%02d by %s \n", pTime->tm_year, pTime->tm_mon, pTime->tm_mday,  
														WeekTXT[pTime->tm_wday], pTime->tm_hour, pTime->tm_min, pTime->tm_sec, EmailText );
					fprintf(outfile, "----------------------------------------------------------------------------------\r\n" );
				}
			}
		}
		else if( HDR_JOAAT==isCRCtype )
		{
			printf("JOAAT>> ");
			if( (1==iVerbosType || 3==iVerbosType) && (TRUE == verbose) && outfile )  
			{
				if(outfile) 
				{
					fprintf(outfile, "----------------------------------------------------------------------------------\r\n" );
					fprintf(outfile, "JOAAT checksum is created at %04d/%02d/%02d/%s %02d:%02d:%02d by %s \n", pTime->tm_year, pTime->tm_mon, pTime->tm_mday,  
														WeekTXT[pTime->tm_wday], pTime->tm_hour, pTime->tm_min, pTime->tm_sec, EmailText );
					fprintf(outfile, "----------------------------------------------------------------------------------\r\n" );
				}
			}
		}
		else					
		{
			printf("CRC**>> ");
		}


		if( ASTERISK_FOUND == isAsteMode ) /* For ah.exe --input *.* */
		{
			unsigned __int64 	kll = 0UL;
			unsigned __int64 	totSize = 0UL;

			if(inpfile) { fclose(inpfile); inpfile=NULL; }
			if(data_buf){ free(data_buf); data_buf=NULL; }

			printf("CRC>> Calculating for input files* \n");
			printf("------------------------------------------------------\r\n" );

			iTotSize = 0LL;
			totSize = 0LL;

			for(ii=iFirst; ii<=(argc-isAsteminus); ii++)
			{
				if( NULL != (inpfile = fopen( argv[ii], "rb")) ) 
				{
			
					if( HDR_CRC16==isCRCtype )
					{
						kll = RunCRC16(argv[ii], outfile_name, totSize, multifileindex, 0, iVerbosType, str_hash);
					}
					else if( HDR_KSC_CRC16==isCRCtype )
					{
						kll = RunKSC_CRC16(argv[ii], outfile_name, totSize, multifileindex, 0, iVerbosType, str_hash);
					}
					else if( HDR_CRC16CCITT==isCRCtype )
					{ 
						kll = RunCRC16CCITT(argv[ii], outfile_name, totSize, multifileindex, 0, iVerbosType, str_hash);	
					}
					else if( HDR_CRC32==isCRCtype )
					{
						kll = RunCRC32(argv[ii], outfile_name, totSize, multifileindex, 0, iVerbosType, str_hash);
					}
					else if( HDR_CRC64==isCRCtype )
					{
						kll = RunCRC64(argv[ii], outfile_name, totSize, multifileindex, 0, iVerbosType, str_hash);
					}
					else if( HDR_CRC64_ISC==isCRCtype )
					{
						kll = RunCRC64_isc(argv[ii], outfile_name, totSize, multifileindex, 0, iVerbosType, str_hash);
					}
					else if( HDR_ADLER32==isCRCtype )
					{
						kll = RunAdler32(argv[ii], outfile_name, totSize, multifileindex, 0, iVerbosType, str_hash);
					}
					else if( HDR_JOAAT==isCRCtype )
					{
						kll = RunJoaat(argv[ii], outfile_name, totSize, multifileindex, 0, iVerbosType, str_hash);
					}
					else
					{
						printf("CRC>> Check CRC unknown type (%d) .... \r\n", isCRCtype );
					}
	
					if(inpfile) { fclose(inpfile); inpfile=NULL; }
					if(data_buf){ free(data_buf); data_buf=NULL; }

					iTotSize += kll;
					multifileindex ++;
				}
			}
			printf("\nCRC>> Number of Input files : %d files (Total size: %llu Byte) ---", multifileindex, iTotSize );

			AllFilesClosed();
	
		}
		else if( multifileindex > 0 )
		{
			unsigned __int64  kll = 0UL;

			if(inpfile) { fclose(inpfile); inpfile=NULL; }
			if(data_buf){ free(data_buf); data_buf=NULL; }


			/* Find first .c file in current directory */
			if( (retFiles = _findfirsti64( extfile_name, &iFiles )) == -1L )
			{
				printf("CRC>>No file [%s] in current directory!!! [%s] \n", extfile_name, infile_name );
			}
			else
			{

				printf("CRC calculating for input files... \n");
				printf("------------------------------------------------------\r\n" );

				do {

					iLenSub = strlen(iFiles.name);

					if( iFiles.attrib & _A_SUBDIR )
					{

					}
					else
					{
						if( NULL == (inpfile = fopen( iFiles.name, "rb")) ) 
						{
							beep(700,100);
							printf("\n[CRC++ERROR++] Can not open multi-input file (%s). \n", iFiles.name );

							AllFilesClosed();

							exit(0);
							return 0;
						}
				
						if( HDR_CRC16==isCRCtype )
						{
							kll = RunCRC16(iFiles.name, outfile_name, iFiles.size, multifileindex, 0, iVerbosType, str_hash);
						}
						else if( HDR_KSC_CRC16==isCRCtype )
						{
							kll = RunKSC_CRC16(iFiles.name, outfile_name, iFiles.size, multifileindex, 0, iVerbosType, str_hash);
						}
						else if( HDR_CRC16CCITT==isCRCtype )
						{ 
							kll = RunCRC16CCITT(iFiles.name, outfile_name, iFiles.size, multifileindex, 0, iVerbosType, str_hash);	
						}
						else if( HDR_CRC32==isCRCtype )
						{
							kll = RunCRC32(iFiles.name, outfile_name, iFiles.size, multifileindex, 0, iVerbosType, str_hash);
						}
						else if( HDR_CRC64==isCRCtype )
						{
							kll = RunCRC64(iFiles.name, outfile_name, iFiles.size, multifileindex, 0, iVerbosType, str_hash);
						}
						else if( HDR_CRC64_ISC==isCRCtype )
						{
							kll = RunCRC64_isc(iFiles.name, outfile_name, iFiles.size, multifileindex, 0, iVerbosType, str_hash);
						}
						else if( HDR_ADLER32==isCRCtype )
						{
							kll = RunAdler32(iFiles.name, outfile_name, iFiles.size, multifileindex, 0, iVerbosType, str_hash);
						}
						else if( HDR_JOAAT==isCRCtype )
						{
							kll = RunJoaat(iFiles.name, outfile_name, iFiles.size, multifileindex, 0, iVerbosType, str_hash);
						}
						else
						{
							printf("CRC>> Check CRC unknown type .... \r\n" );
						}
		
						if(inpfile) { fclose(inpfile); inpfile=NULL; }
						if(data_buf){ free(data_buf); data_buf=NULL; }

						/* file length checking */
						if( iFiles.size != kll ) printf("CRC>> file length is wrong!! (%lld) (%lld)... \r\n", kll, iFiles.size );

						iTotSize += kll;
						multifileindex ++;
					}

				} while( 0 == _findnexti64( retFiles, &iFiles ) );
				_findclose( retFiles );

				multifileindex--;

				printf("\nCRC>> Number of Input files : %d files (Total size: %llu Byte) ---", multifileindex, iTotSize );

			}

		}
		else
	#endif /// MULTI_INPUT_FILES
		{
			unsigned __int64 kll=0UL;

			if(data_buf){ free(data_buf); data_buf=NULL; }

			if( HDR_CRC16==isCRCtype )
			{ 
				kll = RunCRC16(infile_name, outfile_name, infile_size, 0, 0, iVerbosType, str_hash);
			}
			else if( HDR_KSC_CRC16==isCRCtype )
			{ 
				kll = RunKSC_CRC16(infile_name, outfile_name, infile_size, 0, 0, iVerbosType, str_hash);
			}
			else if( HDR_CRC16CCITT==isCRCtype )
			{ 
				kll = RunCRC16CCITT(infile_name, outfile_name, infile_size, 0, 0, iVerbosType, str_hash);
			}
			else if( HDR_CRC32==isCRCtype )
			{
				kll = RunCRC32(infile_name, outfile_name, infile_size, 0, 0, iVerbosType, str_hash);
			}
			else if( HDR_CRC64==isCRCtype )
			{
				kll = RunCRC64(infile_name, outfile_name, infile_size, 0, 0, iVerbosType, str_hash);
			}
			else if( HDR_CRC64_ISC==isCRCtype )
			{
				kll = RunCRC64_isc(infile_name, outfile_name, infile_size, 0, 0, iVerbosType, str_hash);
			}
			else if( HDR_ADLER32==isCRCtype )
			{
				kll = RunAdler32(infile_name, outfile_name, infile_size, 0, 0, iVerbosType, str_hash);
			}
			else if( HDR_JOAAT==isCRCtype )
			{
				kll = RunJoaat(infile_name, outfile_name, infile_size, 0, 0, iVerbosType, str_hash);
			}
			else if( HDR_CRC_UNKNOWN==isCRCtype )
			{
				printf("CRC>> Check CRC type : [%d]. \n", isCRCtype );
			}
			else
			{
				printf("CRC>> Check CRC unknown type : [%d]... \n", isCRCtype );
			}
		}

		AllFilesClosed();

		opt_ok = 0; /* 2014.07.16 : NOTE !!! clear!!!  */

	}
	else if( 1 == isFillSize )
	{

		unsigned int index = 0;
		int ii;

		printf("\n");
		printf("FILL>> Filling pad byte(0x%02x) at end of input file. \n", Pad_Byte);
	
		data_buf = (unsigned char*)malloc( MAX_BUF_SIZ*sizeof(unsigned char) );

		while( EOF != (c=fgetc(inpfile)) )
		{
			if(outfile) fprintf(outfile,"%c", c);
			//data_buf[index] = c;
	
			index++;
	
			if( MAX_BUF_SIZ < index ) {
				beep(700,100);

				AllFilesClosed();

				printf("\n\nFILL>> [++ERROR++] MAX:%u MB. Too bigger file... \n",MAX_BUF_SIZ/(ONE_MEGA_BYTE) );
				exit(0);
				return 0;
			}
		}
	
		if( total_bin_size < index )
		{
			printf("FILL>> Checking file size... %s (= %dKB). Check please... \r\n\n", str_fillSiz, index/1024);
		}
		else
		{
			for(ii=0; ii<(total_bin_size-index); ii++)
			{
				if(outfile) fprintf(outfile,"%c", Pad_Byte); // 2020.07.03 0xFF);
			}
		
			printf("FILL>> Total file size : %d Bytes. (%#x)\n", total_bin_size, total_bin_size );
			printf("FILL>> Fill pad byte (%#x) -  OK \n", Pad_Byte);
			
		}

		AllFilesClosed();

	}
	else if( 1 == isNKinform )
	{
		int idx, ii=0;
		unsigned int load_addr;
		unsigned int image_size;
		unsigned int record_addr, record_size, record_crc, tot_record_size=0;
		unsigned int rom_header_ind = 0, rom_header_addr = 0;
		unsigned int nk_calc_crc=0, rcl=0;
		char rom_header_txt[5] = {0,};
		static int 	prev_record_addr = -1, prev_record_size = -1;
		int	is_BIN_OK = 1;
		FILE *BIN_nb0 = NULL;
		ROMHDR   rom_hdr_s;


/* -- nk.bin structure ---- */
#define NK_BIN_ID 				7	/* 7 bytes */
#define NK_BIN_LAUNCH_ADDR 		4	/* 4bytes */
#define NK_BIN_IMAGE_SIZE 		4	/* 4bytes */

#define NK_BIN_RECORD_ADDR		4
#define NK_BIN_RECORD_SIZE		4
#define NK_BIN_RECORD_CRC		4

#define pTOC_OFFSET_ADDR 		0x44
   
   
#define BIN_SYNC 				'B'  
#define ERR_OPENFILE  			1
#define ERR_READFILE  			2
#define ERR_FILETYPE  			3
#define ERR_ALLOCMEM  			4
#define ERR_NOERR 				5

#define IMAGE_FILE_MACHINE_SH3          0x01a2
#define IMAGE_FILE_MACHINE_SH3E         0x01a4
#define IMAGE_FILE_MACHINE_SH3DSP       0x01a3
#define IMAGE_FILE_MACHINE_SH3E         0x01a4
#define IMAGE_FILE_MACHINE_SH4          0x01a6
#define IMAGE_FILE_MACHINE_I386         0x014c
#define IMAGE_FILE_MACHINE_THUMB        0x01c2
#define IMAGE_FILE_MACHINE_POWERPC      0x01F0
#define IMAGE_FILE_MACHINE_ARM          0x01c0
#define IMAGE_FILE_MACHINE_THUMB        0x01c2
#define IMAGE_FILE_MACHINE_MIPS16       0x0266
#define IMAGE_FILE_MACHINE_ALPHA64      0x0284
#define IMAGE_FILE_MACHINE_MIPSFPU16    0x0466
#define IMAGE_FILE_MACHINE_R4000        0x0166
#define IMAGE_FILE_MACHINE_R10000       0x0168
#define IMAGE_FILE_MACHINE_WCEMIPSV2    0x0169
#define IMAGE_FILE_MACHINE_MIPSFPU      0x0366


		if( 1==isCreateNB0 )
		{
			/* ========== OUTPUT FILE ================= */
			if( (BIN_nb0 = fopen( "CE_nk.nb0", "wb")) == NULL ) 
			{
				printf("\n\n[++ERROR++] Can not create output file (NK.NB0). \n");

				beep(700,100);
				AllFilesClosed();

				exit(0); /// help();
				return 0;
			}
		}

		printf("WinCE BIN Information...... FileName : %s \r\n", infile_name);
		printf("------------------------------------------------------------------------- \r\n");

		if(outfile) 
		{
			fprintf(outfile, "WinCE BIN Information...... FileName : %s \r\n", infile_name);
			fprintf(outfile,"------------------------------------------------------------------------- \r\n");
		}

		data_buf = (unsigned char*)malloc( MAX_BUF_SIZ*sizeof(unsigned char) );

		memset(data_buf, 0x00, MAX_BUF_SIZ*sizeof(unsigned char));
		fr_size = fread(data_buf, sizeof(unsigned char), NK_BIN_ID, inpfile );
		if( (0==strncmp((char*)data_buf, (char*)"B000FF", NK_BIN_ID-1)) && (0x0A==data_buf[NK_BIN_ID-1]) )
		{
			data_buf[NK_BIN_ID-1] = '\0';
			printf("%s Inform> Standard BIN Format              : %s+0x0A \r\n", infile_name, data_buf);
			if(outfile) fprintf(outfile,"%s Inform> Standard BIN Format              : %s+0x0A \r\n", infile_name, data_buf);
			is_BIN_OK = 1;	
		}
		else if( (0==strncmp( (char*)data_buf, (char*)"X000FF", NK_BIN_ID-1)) && (0x0A==data_buf[NK_BIN_ID-1]) )
		{
			data_buf[NK_BIN_ID-1] = '\0';
			printf("%s Inform> Extended BIN Format              : %s+0x0A \r\n", infile_name, data_buf);
			if(outfile) fprintf(outfile,"%s Inform> Extended BIN Format              : %s+0x0A \r\n", infile_name, data_buf);
			is_BIN_OK = 1;	
		}
		else
		{
			printf("%s Inform> Unknown BIN Format               :( ", infile_name);
			if(outfile) fprintf(outfile,"%s Inform> Unknown BIN Format               :( ", infile_name);
			for(idx=0; idx<NK_BIN_ID; idx++)
			{
				printf("%c", data_buf[idx] );
				if(outfile) fprintf(outfile,"%c", data_buf[idx] );
			}
			printf(") \n");
			if(outfile) fprintf(outfile,") \r\n");
			is_BIN_OK = 0;	
		}


		if( is_BIN_OK )
		{

			/* ------ Load Address to SDRAM ---------------- */
			memset(data_buf, 0x00, MAX_BUF_SIZ*sizeof(unsigned char));
			fr_size = fread(data_buf, sizeof(unsigned char), NK_BIN_LAUNCH_ADDR, inpfile );
			load_addr = data_buf[3]<<24 | data_buf[2]<<16 | data_buf[1]<<8 | data_buf[0];
			printf("%s Inform> Load Address (RAM start address) : 0x%X \r\n", infile_name, load_addr);
			if(outfile) fprintf(outfile,"%s Inform> Load Address (RAM start address) : 0x%X \r\n", infile_name, load_addr);
			
			printf("%s Inform> pTOC                             : 0x%X \r\n", infile_name, (load_addr+pTOC_OFFSET_ADDR) );
			if(outfile) fprintf(outfile,"%s Inform> pTOC                             : 0x%X \r\n", infile_name, (load_addr+pTOC_OFFSET_ADDR) );


			/* ------ bin image Size ---------------- */
			memset(data_buf, 0x00, MAX_BUF_SIZ*sizeof(unsigned char));
			fr_size = fread(data_buf, sizeof(unsigned char), NK_BIN_IMAGE_SIZE, inpfile );
			image_size = data_buf[3]<<24 | data_buf[2]<<16 | data_buf[1]<<8 | data_buf[0];
			printf("%s Inform> BIN Image size                   : 0x%08X (%.3fKB) \r\n", infile_name, image_size, (image_size/1024.0));
			if(outfile) fprintf(outfile,"%s Inform> BIN Image size                   : 0x%08X (%.3fKB) \r\n", infile_name, image_size, (image_size/1024.0));
	
			/* Record Start */
			prev_record_addr = -1;
			prev_record_size = -1;

			printf("------------------------------------------------------------------------- \r\n");
			if(outfile) fprintf(outfile,"------------------------------------------------------------------------- \r\n");
			for(idx=1; is_BIN_OK ; idx++)
			{
				/* ------ Record 1 ~ N ---------------- */
				memset(data_buf, 0x00, MAX_BUF_SIZ*sizeof(unsigned char));
				fr_size = fread(data_buf, sizeof(unsigned char), 
								(NK_BIN_RECORD_ADDR+NK_BIN_RECORD_SIZE+NK_BIN_RECORD_CRC), 
								inpfile );

				record_addr = data_buf[3]<<24 | data_buf[2]<<16 | data_buf[1]<<8 | data_buf[0];
				record_size = data_buf[3+4]<<24 | data_buf[2+4]<<16 | data_buf[1+4]<<8 | data_buf[0+4];
				record_crc  = data_buf[3+4+4]<<24 | data_buf[2+4+4]<<16 | data_buf[1+4+4]<<8 | data_buf[0+4+4];
				if( (0==record_addr) && (0==record_crc) ) { /*printf("***"); */break; }
				tot_record_size += record_size;
				
				printf("    Record_%04d> Launch Address       : 0x%X ", idx, record_addr);
				if(outfile) fprintf(outfile,"    Record_%04d> Launch Address       : 0x%X ", idx, record_addr);

				if( rom_header_addr==record_addr )
				{
					printf(" <-- *ROM Header Address* \r\n");
					if(outfile) fprintf(outfile," <-- *ROM Header Address* \r\n");
				}
				else
				{
					printf("\r\n");
					if(outfile) fprintf(outfile,"\r\n");
				}

				printf("    Record_%04d> Payload Size         : (0x%-6X) %d Bytes (%.1f KB) \r\n", idx, record_size, record_size, record_size/1024.0 );
				if(outfile) fprintf(outfile,"    Record_%04d> Payload Size         : (0x%-6X) %d Bytes (%.1f KB) \r\n", idx, record_size, record_size, record_size/1024.0 );

				printf("    Record_%04d> CheckSum             : 0x%08X ", idx, record_crc);
				if(outfile) fprintf(outfile,"    Record_%04d> CheckSum             : 0x%08X ", idx, record_crc);


			#if 1
				if( (1==isCreateNB0) && (-1!=prev_record_addr) && (-1!=prev_record_size) )
				{
					if( (prev_record_addr + prev_record_size) < record_addr )
					{
						for(ii=0; ii < (record_addr - (prev_record_addr + prev_record_size)) ; ii++)
						{
							if(BIN_nb0) fprintf(BIN_nb0,"%c", 0x00);
						}
					}
				}
			#endif


				/* --------- payload -------- */
				memset(data_buf, 0x00, MAX_BUF_SIZ*sizeof(unsigned char));
				fr_size = fread(data_buf, sizeof(unsigned char), record_size, inpfile ); /* size 만큰 payload reading.. */
				/* --------- payload -------- */
				if( fr_size <= 0) break;

				if( 1==isCreateNB0 )
				{
					fwrite(data_buf, sizeof(unsigned char), record_size, BIN_nb0 );
				}
				/// -----Calc CRC for all Record-----------
				nk_calc_crc = 0UL;
				for(rcl=0; rcl<record_size; rcl++)
					nk_calc_crc += data_buf[rcl];
				printf(" (Calc_CRC: 0x%08X", nk_calc_crc);
				if(outfile) fprintf(outfile," (Calc_CRC: 0x%08X", nk_calc_crc);

				if(nk_calc_crc==record_crc) 
				{
					printf(" - CRC OK) \n");
					if(outfile) fprintf(outfile," - CRC OK) \r\n");
				}
				else
				{
					printf(" - CRC Failed) \n");
					if(outfile) fprintf(outfile," - CRC Failed) \r\n");
				}
				/// -----------------------------------------

			#if 1 /// 2014.07.19, new--
			
				if( rom_header_addr==record_addr )
				{
					memset( &rom_hdr_s, 0x00, sizeof(ROMHDR) );
					memcpy( (void*)&rom_hdr_s, data_buf, sizeof(ROMHDR) );

					printf("     ------------ ROMHDR ---------------\n");
					printf("      DLL First           : 0x%08lX \n", rom_hdr_s.dllfirst);
					printf("      DLL Last            : 0x%08lX \n", rom_hdr_s.dlllast);
					printf("      Physical First      : 0x%08lX \n", rom_hdr_s.physfirst);
					printf("      Physical Last       : 0x%08lX \n", rom_hdr_s.physlast);
					printf("      Num Modules         : %10ld \n", rom_hdr_s.nummods);
					printf("      RAM Start           : 0x%08lX \n", rom_hdr_s.ulRamStart);
					printf("      RAM Free            : 0x%08lX \n", rom_hdr_s.ulRAMFree);
					printf("      RAM End             : 0x%08lX \n", rom_hdr_s.ulRAMEnd);
					printf("      Num Copy Entries    : %10ld \n", rom_hdr_s.ulCopyEntries);
					printf("      Copy Entries Offset : 0x%08lX \n", rom_hdr_s.ulCopyOffset);
					printf("      Length of profile   : %10lu \n", rom_hdr_s.ulProfileLen);
					printf("      Prof Symbol Offset  : 0x%08lX \n", rom_hdr_s.ulProfileOffset);
					printf("      Num Files           : %10ld \n", rom_hdr_s.numfiles);
					printf("      Kernel flags        : 0x%08lX \n", rom_hdr_s.ulKernelFlags);
					printf("      MiscFlags           : 0x%08X \n", rom_hdr_s.usMiscFlags);
					printf("      CPU                 : 0x%04x ", rom_hdr_s.usCPUType);

					if(outfile) 
					{
						fprintf(outfile,"     ------------ ROMHDR --------------- \r\n");
						fprintf(outfile,"      DLL First           : 0x%08lX \r\n", rom_hdr_s.dllfirst);
						fprintf(outfile,"      DLL Last            : 0x%08lX \r\n", rom_hdr_s.dlllast);
						fprintf(outfile,"      Physical First      : 0x%08lX \r\n", rom_hdr_s.physfirst);
						fprintf(outfile,"      Physical Last       : 0x%08lX \r\n", rom_hdr_s.physlast);
						fprintf(outfile,"      Num Modules         : %10ld \r\n", rom_hdr_s.nummods);
						fprintf(outfile,"      RAM Start           : 0x%08lX \r\n", rom_hdr_s.ulRamStart);
						fprintf(outfile,"      RAM Free            : 0x%08lX \r\n", rom_hdr_s.ulRAMFree);
						fprintf(outfile,"      RAM End             : 0x%08lX \r\n", rom_hdr_s.ulRAMEnd);
						fprintf(outfile,"      Num Copy Entries    : %10ld \r\n", rom_hdr_s.ulCopyEntries);
						fprintf(outfile,"      Copy Entries Offset : 0x%08lX \r\n", rom_hdr_s.ulCopyOffset);
						fprintf(outfile,"      Length of profile   : %10lu \r\n", rom_hdr_s.ulProfileLen);
						fprintf(outfile,"      Prof Symbol Offset  : 0x%08lX \r\n", rom_hdr_s.ulProfileOffset);
						fprintf(outfile,"      Num Files           : %10ld \r\n", rom_hdr_s.numfiles);
						fprintf(outfile,"      Kernel flags        : 0x%08lX \r\n", rom_hdr_s.ulKernelFlags);
						fprintf(outfile,"      MiscFlags           : 0x%08X \r\n", rom_hdr_s.usMiscFlags);
						fprintf(outfile,"      CPU                 : 0x%04x ", rom_hdr_s.usCPUType);
					}
					switch(rom_hdr_s.usCPUType) 
					{
					case IMAGE_FILE_MACHINE_SH3:
					  printf("(SH3)\n");
					  if(outfile) fprintf(outfile,"(SH3) \r\n");
					  break;
					case IMAGE_FILE_MACHINE_SH3E:
					  printf("(SH3e)\n");
					  if(outfile) fprintf(outfile,"(SH3e)\r\n");
					  break;
					case IMAGE_FILE_MACHINE_SH3DSP:
					  printf("(SH3-DSP)\n");
					  if(outfile) fprintf(outfile,"(SH3-DSP)\r\n");
					  break;
					case IMAGE_FILE_MACHINE_SH4:
					  printf("(SH4)\n");
					  if(outfile) fprintf(outfile,"(SH4)\r\n");
					  break;
					case IMAGE_FILE_MACHINE_I386:
					  printf("(x86)\n");
					  if(outfile) fprintf(outfile,"(x86)\r\n");
					  break;
					case IMAGE_FILE_MACHINE_THUMB:
					  printf("(Thumb)\n");
					  if(outfile) fprintf(outfile,"(Thumb)\r\n");
					  break;
					case IMAGE_FILE_MACHINE_ARM:
					  printf("(ARM)\n");
					  if(outfile) fprintf(outfile,"(ARM)\r\n");
					  break;
					case IMAGE_FILE_MACHINE_POWERPC:
					  printf("(PPC)\n");
					  if(outfile) fprintf(outfile,"(PPC)\r\n");
					  break;
					case IMAGE_FILE_MACHINE_R4000:
					  printf("(R4000)\n");
					  if(outfile) fprintf(outfile,"(R4000)\r\n");
					  break;
					case IMAGE_FILE_MACHINE_MIPS16:
					  printf("(MIPS16)\n");
					  if(outfile) fprintf(outfile,"(MIPS16)\r\n");
					  break;
					case IMAGE_FILE_MACHINE_MIPSFPU:
					  printf("(MIPSFPU)\n");
					  if(outfile) fprintf(outfile,"(MIPSFPU)\r\n");
					  break;
					case IMAGE_FILE_MACHINE_MIPSFPU16:
					  printf("(MIPSFPU16)\n");
					  if(outfile) fprintf(outfile,"(MIPSFPU16)\r\n");
					  break; 
					default:
					  printf("(Unknown)\r\n");
					  if(outfile) fprintf(outfile,"(Unknown)\r\n");
					  break;
					}


					printf("      RAM -- FSRAMPERCENT : 0x%08lX \n", rom_hdr_s.ulFSRamPercent);
					printf("      Device Start addr   : 0x%08lX \n", rom_hdr_s.ulDrivglobStart);
					printf("      Device length       : 0x%08lX \n", rom_hdr_s.ulDrivglobLen);
					printf("      ROM Header ext.     : 0x%08X \n", rom_hdr_s.pExtensions);
					printf("      Tracking MEM start  : 0x%08lX \n", rom_hdr_s.ulTrackingStart); /// Start Address
					printf("      Tracking MEM end    : 0x%08lX \n",rom_hdr_s.ulTrackingLen);
					printf("	  Extensions		  : 0x%08lX \n", rom_hdr_s.pExtensions );

					if(outfile) 
					{
						fprintf(outfile,"      RAM -- FSRAMPERCENT : 0x%08lX \r\n", rom_hdr_s.ulFSRamPercent);
						fprintf(outfile,"      Device Start addr   : 0x%08lX \r\n", rom_hdr_s.ulDrivglobStart);
						fprintf(outfile,"      Device length       : 0x%08lX \r\n", rom_hdr_s.ulDrivglobLen);

						fprintf(outfile,"      ROM Header ext.     : 0x%08lX \r\n", rom_hdr_s.pExtensions);
						fprintf(outfile,"      Tracking MEM start  : 0x%08lX \r\n", rom_hdr_s.ulTrackingStart); /// Start Address
						fprintf(outfile,"      Tracking MEM end    : 0x%08lX \r\n",rom_hdr_s.ulTrackingLen);
					
						fprintf(outfile,"      Extensions          : 0x%08lX \r\n", rom_hdr_s.pExtensions );
					}
			
					if (rom_hdr_s.pExtensions) 
					{
				#if 0
						printf("     -------- ROMHDR Extensions ---------\n");


						printf("     ---------- COPY SECTIONS -----------\n");


						printf("     ------------- MODULES --------------\n");
				#endif
					}



					printf("     ----------------------------------- \r\n");
					if(outfile) fprintf(outfile,"     ----------------------------------- \r\n");

				}

				if(2==idx) /// ROM Header Indicator , idx=2
				{
					sprintf(rom_header_txt,"%c%c%c%c", data_buf[0], data_buf[1], data_buf[2], data_buf[3]);
					rom_header_ind  = (data_buf[0]<<24 | data_buf[1]<<16 | data_buf[2]<<8 | data_buf[3]);
					rom_header_addr = (data_buf[7]<<24 | data_buf[6]<<16 | data_buf[5]<<8 | data_buf[4]);
					printf("    Record_%04d> ROM Header Indicator : %s \r\n", idx, rom_header_txt );
					printf("    Record_%04d> ROM Header Address   : 0x%08X \r\n", idx, rom_header_addr);

					if(outfile) fprintf(outfile,"    Record_%04d> ROM Header Indicator : %s \r\n", idx, rom_header_txt);
					if(outfile) fprintf(outfile,"    Record_%04d> ROM Header Address   : 0x%08X \r\n", idx, rom_header_addr);
				}

				printf(" \r\n");
				if(outfile) fprintf(outfile," \r\n");
			#endif

				prev_record_addr = record_addr;
				prev_record_size = record_size;
				
				
			}

			memset(data_buf, 0x00, MAX_BUF_SIZ*sizeof(unsigned char));

			printf("WinCE BIN (%s) payload size = %.3f MB (0x%X)\r\n", infile_name, (tot_record_size/1024.0)/1024.0,  tot_record_size);
			if(outfile) fprintf(outfile,"WinCE BIN (%s) payload size = %.3f MB (0x%X) \r\n", infile_name, (tot_record_size/1024.0)/1024.0,  tot_record_size);


			if( 1==isCreateNB0 )
			{
				if(BIN_nb0) fclose(BIN_nb0);
				printf("\nWinCE OS Kernel(NK.BIN) Information!!! Created CE_nk.nb0 - OK");
				if(outfile) fprintf(outfile,"\r\nWinCE OS Kernel(NK.BIN) Information!!! Created CE_nk.nb0 - OK");
			}
			else
			{
				printf("\nWinCE OS Kernel(NK.BIN) Information!!! - OK");
			}

		}
		else
		{
			printf("\nThis file[%s] is not WinCE OS Kernel(NK.BIN)... Check file.. \n\n", infile_name );
		}

		AllFilesClosed();

		opt_ok = 0; /* NOTE !!! clear!!!  */

	}

#if ELF2BIN	
	else if( 1 == isElf2Bin )
	{

		if (fstat(fileno(inpfile), &file_statbuf) < 0) 
		{ 
			printf("\n\n[++ERROR++]Cannot stat [%s]\n", infile_name ); 

			AllFilesClosed(); // 2020.07.10
			exit(0);
			return 2; 
		}	 
		
		
		elf_size = file_statbuf.st_size; 
		printf("ELF2BIN>> ELF size      : %u Bytes\n", elf_size ); 

		data_buf = (unsigned char*)malloc( elf_size );
		memset( data_buf, 0x00, elf_size * sizeof(unsigned char) );
		
		if( NULL==data_buf ) 
		{ 
			printf("ELF2BIN>> Cannot allocate space for %s!\n", infile_name ); 
		} 
		else
		{
			if (fread(data_buf, sizeof (char), elf_size, inpfile) != elf_size) 
			{ 
				printf("ELF2BIN>> Cannot read %s!\n", infile_name ); 
			} 
			
			elf_dumpimage(data_buf, elf_size); 
		}

		AllFilesClosed();

	}
#endif

	else if( 1 == isMot2bin )
	{
		unsigned int iErrCount = 0;
		
		/* line inputted from file */
		char HexaLine[HEX_MAX_LINE_SIZE];
		
		/* flag that a file was read */
		bool Fileread;
	
		/* cmd-line parameter # */
		char *pBin = NULL;
	
		int result;
	
		/* Application specific */
		unsigned int First_Word, Address;
		unsigned int Type;
		unsigned int Exec_Address;
		unsigned int temp;
		unsigned int Record_Count, Record_Checksum;
		unsigned int hexFamily=2; // Intel Family:1, Motorola Family : 2
	
		unsigned char Data_Str[HEX_MAX_LINE_SIZE];


		Phys_AddrTemp = 0; // added at 2016.03.10
		iErrCount = 0; // clear
		checksum_err_cnt = 0;	// 2022.08.03

		memset( HexaLine, 0x00, HEX_MAX_LINE_SIZE*sizeof(char) );
		memset( Data_Str, 0x00, HEX_MAX_LINE_SIZE*sizeof(unsigned char) );


		Fileread = 1;
	
		/* When the hex file is opened, the program will read it in 2 passes.
		The first pass gets the highest and lowest addresses so that we can allocate
		the right size.
		The second pass processes the hex data. */
	
		/* To begin, assume the lowest address is at the end of the memory.
		 While reading each records, subsequent addresses will lower this number.
		 At the end of the input file, this value will be the lowest address.
	
		 A similar assumption is made for highest address. It starts at the
		 beginning of memory. While reading each records, subsequent addresses will raise this number.
		 At the end of the input file, this value will be the highest address. 
		----------------------------------------------------------------------- */

		Lowest_Address = (unsigned int)-1;
		Highest_Address = 0;
		Records_Start = 0;
		Record_Nb = 0;
		First_Word = 0;
	
		/* get highest and lowest addresses so that we can allocate the right size */
		do
		{
			unsigned int i;
	
			/* Read a line from input file. */
			memset( HexaLine, 0x00, HEX_MAX_LINE_SIZE*sizeof(char) );
	
			if( NULL == fgets( HexaLine, HEX_MAX_LINE_SIZE, inpfile ) )
			{
				break;
			}
			
			Record_Nb++;
	
			/* Remove carriage return/line feed at the end of line. */
			i = strlen(HexaLine);

		#if 0 // 2016.03.09_initial
			if (--i != 0)
		#else
			if (--i > 0)
		#endif
			{
				if (HexaLine[i] == '\n') HexaLine[i] = '\0';
				if (HexaLine[i] == '\r') HexaLine[i] = '\0';
	
				pBin = (char *) Data_Str;
	
				switch(HexaLine[1])
				{
				case '0':
					Nb_Bytes = 1; /* This is to fix the Highest_Address set to -1 when Nb_Bytes = 0 */
					break;
	
				/* 16 bits address */
				// S1 Record. The type of record field is 'S1' (0x5331).
				// The address field is intrepreted as a 2-byte address. 
				// The data field is composed of memory loadable data.
				case '1':
					result = sscanf (HexaLine,"S%1x%2x%4x",&Type,&Nb_Bytes,&First_Word);
		            if (result != 3 && (i>0) ) 
		            {
						// fprintf(stderr,"S1:Error in line %d of hex file.\n", Record_Nb);
						printf("\nLine%6d :S1-Error in hex file. ", Record_Nb );
						if( result<=0 ) printf(" Check INTEL family hexa type!!!");


						iErrCount ++; // 2016.03.05
						// ---------------------------------
						if( iErrCount > MAX_ERR_COUNT*2 ) // 2016.03.05
						{
							printf("\n\nCheck Motorola hexa family type!! Maybe INTEL family hex type!\n");
						
							iErrCount = 0; // clear
							AllFilesClosed();
						
							exit(0);
							return 1;
						}
						// ---------------------------------

		            }
				
					/* Adjust Nb_Bytes for the number of data bytes */
					Nb_Bytes = Nb_Bytes - 3;
					break;
	
				/* 24 bits address */
				// S2 Record. The type of record field is 'S2' (0x5332). 
				// The address field is intrepreted as a 3-byte address. 
				// The data field is composed of memory loadable data.
				case '2':
					result = sscanf (HexaLine,"S%1x%2x%6x",&Type,&Nb_Bytes,&First_Word);
		            if (result != 3 && (i>0) ) 
		            {
						//fprintf(stderr,"S2:Error in line %d of hex file.\n", Record_Nb);
						printf("Line%6d :S2-Error in hex file. (%d) \n", Record_Nb, result );
						iErrCount ++; // 2016.03.05

						// ---------------------------------
						if( iErrCount > MAX_ERR_COUNT*2 ) // 2022.08.05
						{
							printf("\n\nCheck Motorola hexa family type!! Maybe INTEL family hexa type!!\n");
						
							iErrCount = 0; // clear
							AllFilesClosed();
						
							exit(0);
							return 1;
						}
						// ---------------------------------

		            }
				
					/* Adjust Nb_Bytes for the number of data bytes */
					Nb_Bytes = Nb_Bytes - 4;
					break;
	
				/* 32 bits address */
				// S3 Record. The type of record field is 'S3' (0x5333). 
				// The address field is intrepreted as a 4-byte address. 
				// The data field is composed of memory loadable data.
				case '3':
					result = sscanf (HexaLine,"S%1x%2x%8x",&Type,&Nb_Bytes,&First_Word);
		            if (result != 3 && (i>0) ) 
	            	{
						//fprintf(stderr,"S3:Error in line %d of hex file.\n", Record_Nb);
						printf("Line%6d :S3-Error in hex file. (%d) \n", Record_Nb, result );
						iErrCount ++; // 2016.03.05

						// ---------------------------------
						if( iErrCount > MAX_ERR_COUNT*2 ) // 2022.08.05
						{
							printf("\n\nCheck Motorola hexa family type!! Maybe INTEL family hexa type!!!\n");
						
							iErrCount = 0; // clear
							AllFilesClosed();
						
							exit(0);
							return 1;
						}
						// ---------------------------------

		            }
				
					/* Adjust Nb_Bytes for the number of data bytes */
					Nb_Bytes = Nb_Bytes - 5;
					break;
				}
	
				Phys_Addr = First_Word;

			#if HEX2BIN_MOTOROLA_ZERO_FORCED // 2020.06.24 Zero-Forced!!
				// ----------------------------------------
				// Hex Address Zero based converted 
				// ----------------------------------------
				if( HEX2BIN_ZERO_FORCED==Enable_HexaAddr_Zero_Forced )
				{
					// S 3 0D 00000000 FFFFFFFF8007E80B7C
					// S 3 15 08000000 000C0020F99C0208799102087B910208ED
					// S 3 15 9FC00000 00B4083C2D20093C40002935201009AD7D
			
					if( iOnceCheck && (HexaLine[1]=='1' || HexaLine[1]=='2' || HexaLine[1]=='3') )
					{
						iOnceCheck = 0;
						if( Phys_Addr>0x00000000 )
						{
							Phys_AddrTemp = Phys_Addr;

							printf("\n");
							printf("--------------------------------------------------------\n");
							printf("Line%6d :S%c-Record: Phys_Addr = 0x%08X. \n", Record_Nb, HexaLine[1], Phys_Addr );
						}
					}
					
					if( Phys_AddrTemp && (0==iOnceCheck) )
					{
						Phys_Addr -= Phys_AddrTemp;
						//if(verbose) printf("S%c-rrecord: Phys_Addr = 0x%08X \n", HexaLine[1], Phys_Addr );
					}
				}
			#endif

	
				/* Set the lowest address as base pointer. */
				if (Phys_Addr < Lowest_Address)
					Lowest_Address = Phys_Addr;
	
				/* Same for the top address. */
				temp = Phys_Addr + Nb_Bytes -1;
	
				if (temp > Highest_Address)
				{
					Highest_Address = temp;

					Real_Length = Highest_Address; // real size 2016.03.05
					if (verbose) 
					{
						if(iVerbosType<2) printf("Line%6d :Highest Address : %#08x  Read Byte:%#02x \r", Record_Nb, Highest_Address, Nb_Bytes );
						else printf("Line%6d :Highest Address : %#08x  Read Byte:%#02x \n", Record_Nb, Highest_Address, Nb_Bytes );
					}

				}

			}
		}
		while (!feof (inpfile));


		// ---------------------------------
		if( iErrCount > MAX_ERR_COUNT ) // 2016.03.05
		{
			printf("\n\nCheck Motorola hexa type!!! Maybe INTEL family hexa type!! \n");

			iErrCount = 0; // clear

			AllFilesClosed();

			exit(0);
			return 1;
		}
		// ---------------------------------


		
		Allocate_Memory_And_Rewind();
	
		Record_Nb = 0;
	
		/* Read the file & process the lines. */
		do /* repeat until EOF(Filin) */
		{
			int i;
	
			Checksum = 0;
	
			/* Read a line from input file. */
			memset( HexaLine, 0x00, sizeof(HexaLine) );
			if( NULL == fgets( HexaLine, HEX_MAX_LINE_SIZE, inpfile ) )
			{
				break;
			}
			
			Record_Nb++;
	
			/* Remove carriage return/line feed at the end of line. */
			i = strlen(HexaLine);

		#if 0 // 2016.03.09_initial
			if (--i != 0)
		#else
			if (--i > 0)
		#endif
			{
				if (HexaLine[i] == '\n') HexaLine[i] = '\0';
				if (HexaLine[i] == '\r') HexaLine[i] = '\0';
	
				/* Scan starting address and nb of bytes. */
				/* Look at the record type after the 'S' */
				Type = 0;
	
				switch(HexaLine[1])
				{
				case '0':
					result = sscanf (HexaLine,"S0%2x0000484452%2x",&Nb_Bytes,&Record_Checksum);

					//if (result != 2) fprintf(stderr,"S0:Error in line %d of hex file\n", Record_Nb);
		            if (result != 2 && (i>0) ) 
		            {
						// ------------------------------------------------------------
						// -- [S0 17 52656C65617365204275696C645C726F6D702E6F7574 77] 
						// ------------------------------------------------------------
						char TempC[3];
						int  iidx=2, wasChar=0;
						unsigned char TempVal=0, TempCRCSumVal=0, TempLenVal=0, TempCRC=0, iCount=0;
						char szS0tmp[5]={0,}, szS0rectxt[1024]={0,};


						memset( szS0rectxt, 0x00, sizeof(szS0rectxt) );

						strcpy( szS0rectxt, "S0-record         : [" );
						for(iidx=2; HexaLine[iidx] != 0x00 && iidx<(Nb_Bytes+1)*2; iidx+=2 )
						{
							memset( TempC, 0x00, sizeof(TempC) );
							strncpy( (char*)TempC, (char *)&HexaLine[iidx], 2);
							TempVal = str2hex(TempC);
							iCount ++;
							if( 2==iidx ) 
							{
								TempLenVal = TempVal; // length once saved!
							}

							/* ========== checksum loginc ========= */
							{
								TempCRCSumVal += TempVal;
								TempCRCSumVal &= 0xFF;

								//printf("idx(%d) -> %02x -> %02x \n", iidx, TempVal, TempCRCSumVal );

								if( iCount <= TempLenVal )
								{
									memset( szS0tmp, 0x00, sizeof(szS0tmp) );
									if( TempVal >= 0x20 && TempVal <= 0x7F )
									{
										sprintf(szS0tmp, "%c", TempVal);
										wasChar=1;
									}
									else
									{
										if(wasChar) 
										{ 
											wasChar=0;  
											sprintf(szS0tmp, " %02x ", TempVal);
										}
										else 
										{
											sprintf(szS0tmp, "%02x ", TempVal);
										}
									}

									strcat( szS0rectxt, szS0tmp );
								}
							}
							
						}
						strcat( szS0rectxt, "]");
						printf("%s -> ", szS0rectxt );


						strncpy( (char*)TempC, (char *)&HexaLine[iidx], 2);
						TempVal  = str2hex(TempC);
						TempCRC  = TempVal;


						// ========= checksum confirm!!
						if( ((255-TempCRCSumVal)&0xFF) == (TempCRC&0xFF) )
						{
							// OK !!!!
							//Checksum = Nb_Bytes + 0x52 + 0x65 + 0x6C + 0x65 + 0x61 + 0x73 + 0x65 + 0x20 + 0x42 + 0x75 + 0x69 + 0x6C + 0x64 + 0x5C + 0x72 + 0x6F + 0x6D + 0x70 + 0x2E + 0x6F + 0x75 + 0x74;
							//printf("S0 Checksum = %#02x -- OK \n", (255-Checksum) );
							printf("Line #%d :S0 Checksum is OK \n", Record_Nb );
						}
						else
						{
							//fprintf(stderr,"Error in line %d of hex file\n", Record_Nb);
							printf("Line #%d :S0-Error in selected hex file. This checksum should be 0x%02x \n", Record_Nb, (255-TempCRCSumVal)&0xFF );
							//printf("[%s]\n", HexaLine );
						}

						// read checksum ---
						Checksum = TempCRCSumVal;
						Record_Checksum = TempCRC;
		            }
					else // 0000484452
					{
		                Checksum = Nb_Bytes + 0x48 + 0x44 + 0x52;
						printf("Line%6d :S0-record(default): [%c%c%c] \n", Record_Nb, 0x48, 0x44, 0x52 );
					}

	
					/* Adjust Nb_Bytes for the number of data bytes */
					Nb_Bytes = 0;
					break;
	
				/* 16 bits address */
				case '1':
					result = sscanf (HexaLine,"S%1x%2x%4x%s",&Type,&Nb_Bytes,&Address,Data_Str);
					if (result != 4) fprintf(stderr,"Line%6d :S1:Error in hex file\n", Record_Nb);
					Checksum = Nb_Bytes + (Address >> 8) + (Address & 0xFF);
	
					/* Adjust Nb_Bytes for the number of data bytes */
					Nb_Bytes = Nb_Bytes - 3;
					break;
	
				/* 24 bits address */
				case '2':
					result = sscanf (HexaLine,"S%1x%2x%6x%s",&Type,&Nb_Bytes,&Address,Data_Str);
					if (result != 4) fprintf(stderr,"Line%6d :S2:Error in hex file\n", Record_Nb);
					Checksum = Nb_Bytes + (Address >> 16) + (Address >> 8) + (Address & 0xFF);
	
					/* Adjust Nb_Bytes for the number of data bytes */
					Nb_Bytes = Nb_Bytes - 4;
					break;
	
				/* 32 bits address */
				case '3':
					result = sscanf (HexaLine,"S%1x%2x%8x%s",&Type,&Nb_Bytes,&Address,Data_Str);
					if (result != 4) fprintf(stderr,"Line%6d :S3:Error in hex file\n", Record_Nb);
					Checksum = Nb_Bytes + (Address >> 24) + (Address >> 16) + (Address >> 8) + (Address & 0xFF);
	
					/* Adjust Nb_Bytes for the number of data bytes */
					Nb_Bytes = Nb_Bytes - 5;
					break;
	
				case '5':
					result = sscanf (HexaLine,"S%1x%2x%4x%2x",&Type,&Nb_Bytes,&Record_Count,&Record_Checksum);
					if (result != 4) fprintf(stderr,"Line%6d :S5:Error in hex file\n", Record_Nb);
					Checksum = Nb_Bytes + (Record_Count >> 8) + (Record_Count & 0xFF);
	
					/* Adjust Nb_Bytes for the number of data bytes */
					Nb_Bytes = 0;
					break;
	
				case '7':
					result = sscanf (HexaLine,"S%1x%2x%8x%2x",&Type,&Nb_Bytes,&Exec_Address,&Record_Checksum);
					if (result != 4) fprintf(stderr,"Line%6d :S7:Error in hex file\n", Record_Nb);
					Checksum = Nb_Bytes + (Exec_Address >> 24) + (Exec_Address >> 16) + (Exec_Address >> 8) + (Exec_Address & 0xFF);
					Nb_Bytes = 0;
					break;
	
				case '8':
					result = sscanf (HexaLine,"S%1x%2x%6x%2x",&Type,&Nb_Bytes,&Exec_Address,&Record_Checksum);
					if (result != 4) fprintf(stderr,"Line%6d :S8:Error in hex file\n", Record_Nb);
					Checksum = Nb_Bytes + (Exec_Address >> 16) + (Exec_Address >> 8) + (Exec_Address & 0xFF);
					Nb_Bytes = 0;
					break;
				case '9':
					result = sscanf (HexaLine,"S%1x%2x%4x%2x",&Type,&Nb_Bytes,&Exec_Address,&Record_Checksum);
					if (result != 4) fprintf(stderr,"Line%6d :S9:Error in hex file\n", Record_Nb);
					Checksum = Nb_Bytes + (Exec_Address >> 8) + (Exec_Address & 0xFF);
					Nb_Bytes = 0;
					break;
				}
	
				pBin = (char *) Data_Str;
	
				/* If we're reading the last record, ignore it. */
				switch (Type)
				{
				/* Data record */
				case 1:
				case 2:
				case 3:
					if (Nb_Bytes == 0)
					{
						fprintf(stderr,"zero byte length Data record is ignored.\n");
						break;
					}
	
					Phys_Addr = Address;

				#if HEX2BIN_MOTOROLA_ZERO_FORCED // 2020.06.24 Zero-Forced!!
					// ----------------------------------------
					// Hex Address Zero based converted 
					// ----------------------------------------
					if( HEX2BIN_ZERO_FORCED==Enable_HexaAddr_Zero_Forced )
					{
						// S 3 0D 00000000 FFFFFFFF8007E80B7C
						// S 3 15 08000000 000C0020F99C0208799102087B910208ED
						// S 3 15 9FC00000 00B4083C2D20093C40002935201009AD7D
				
						if( Phys_AddrTemp )
						{
							if( Phys_Addr >= Phys_AddrTemp )
							{
								Phys_Addr -= Phys_AddrTemp;
								//EB_Printf(TEXT("[dnw] S%d-Record: Phys_Addr = 0x%08X \r\n"), Type, Phys_Addr );
							}
							else
							{
								printf("S%d-Record: Zero-forced Error!!! Phys_Addr: %#08X, Phys_Addr_Base: %#08X \n", Type, Phys_Addr, Phys_AddrTemp );
							}
						}
					}
				#endif


					pBin = ReadDataBytes(pBin);
	
					/* Read the Checksum value. */
					result = sscanf (pBin, "%2x",&Record_Checksum);
					if (result != 1) fprintf(stderr,"Line%6d ::Error in hex file (result%d) \n", Record_Nb, result);
					break;
	
				case 5:
					if( verbose ) fprintf(stderr,"Line%6d ::S5:Record total: %d\n",Record_Nb, Record_Count);
					break;
	
				case 7:
					if( verbose ) fprintf(stderr,"Line%6d ::S7:Execution Address (unused): %#08x\n",Record_Nb, Exec_Address);
					break;
	
				case 8:
					if( verbose ) fprintf(stderr,"Line%6d ::S8:Execution Address (unused): %#06x\n",Record_Nb, Exec_Address);
					break;
	
				case 9:
					if( verbose ) fprintf(stderr,"Line%6d ::S9:Execution Address (unused): %#04x\n",Record_Nb, Exec_Address);
					break;
	
				/* Ignore all other records */
				default: // S4, S6
					if( verbose )
					{
						printf("Line%6d ::S%d-Record: Phys_Addr: %#08X \n", Record_Nb, Type, Phys_Addr );
						//printf("[%s] \r\n", HexaLine );
					}
					// ignore -- break;

				}
	
				Record_Checksum &= 0xFF;
	
				/* Verify Checksum value. */
			#if 1
				VerifyChecksumValue(Record_Checksum, hexFamily); /* hexFamily:2 MOTOROLA family*/
			#else
				if (((Record_Checksum + Checksum) != 0xFF) && Enable_Checksum_Error)
				{
					printf("Line%6d ::Checksum error: should be 0x%02x, not be 0x%02x. \n",Record_Nb, 255-Checksum, Record_Checksum); ///, HexaLine);
					Status_Checksum_Error = true;
				}
			#endif
			}
		}
		while (!feof (inpfile));
		/*-----------------------------------------------------------------------------*/
		
		printf("Binary file start : %#010x \n", Lowest_Address);
		printf("Records start     : %#010x \n", Records_Start);
		printf("Highest address   : %#010x \n", Highest_Address /*, (float)(Highest_Address/1024.0) */ );
		printf("Pad byte(def:FF)  : 0x%02X \n", Pad_Byte);


	#if HEX2BIN_MOTOROLA_ZERO_FORCED // 2016.03.10
		// ----------------------------------------
		// Hex Address Zero based converted 
		// ----------------------------------------
		if( HEX2BIN_ZERO_FORCED==Enable_HexaAddr_Zero_Forced )
		{
			if( Phys_AddrTemp )
			{
				printf("------ [ Zero-Address Forced Logic ] -------------------\n");
				printf("Physical Base address    : 0x%08X \n", Phys_AddrTemp );
				printf("Hex file real start addr : 0x%08X \n", Lowest_Address + Phys_AddrTemp );
				printf("Hex real Highest address : 0x%08X <- %.2fkB\n", Highest_Address + Phys_AddrTemp, (float)((Highest_Address + Phys_AddrTemp)/1024.0) );
				printf("--------------------------------------------------------\r\n");
				printf("Motorola S-Record address Zero-forced!!! \n");
			}

		}
	#endif


		WriteMemory();

		if (Status_Checksum_Error && Enable_Checksum_Error)
		{
			printf("\n");
			printf("MOTOROLA family hexa: Checksum error (%d) is detected !!!\n", checksum_err_cnt);

			AllFilesClosed();

			exit(0);
			return 1;
		}
		else
		{
			printf("--------------------------------------------------------\n");
			printf("Parserd line number of hex record : %d lines \n", Record_Nb);
			printf("Converted the MOTOROLA hexa to binary. -- OK \n");

			AllFilesClosed();

			exit(0);
			return 1;
		}

	}
	else if( 1 == isIntel2bin )
	{
#define INTEL_BASE_ADDR_CMP 		0x0010 

		unsigned int iErrCount = 0;
		
		/* line inputted from file */
		char HexaLine[HEX_MAX_LINE_SIZE];
	
		/* flag that a file was read */
		bool Fileread;
	
		/* cmd-line parameter # */
		char *pBin = NULL;
	
		int result;
	
		/* Application specific */
		unsigned int First_Word, Address, Segment, Upper_Address;
		unsigned int Type;
		unsigned int Offset = 0x00;
		unsigned int temp;
		const unsigned int hexFamily=1; // Intel Family:1, Motorola Family : 2
		
		/* We will assume that when one type of addressing is selected, it will be valid for all the
		 current file. Records for the other type will be ignored. */
		unsigned int Seg_Lin_Select = NO_ADDRESS_TYPE_SELECTED;
	
		unsigned int temp2;
	
		unsigned char Data_Str[HEX_MAX_LINE_SIZE];
	
		checksum_err_cnt = 0;	// 2022.08.03

		Phys_AddrTemp = 0; // added at 2016.03.10

		iErrCount = 0; // added at 2016.03.07, Error count clear
	
		memset(HexaLine, 0x00, sizeof(HexaLine) ); /// 2014.06.26
		memset(Data_Str, 0x00, sizeof(Data_Str) ); /// 2014.06.26
	

		/* allocate a buffer */
		Memory_Block = (unsigned char *)NoFailMalloc(Max_Length); 
		
		/* For EPROM or FLASH memory types, fill unused bytes with FF or the value specified by the p option */
		//memset (Memory_Block, 0xff, Max_Length);
	

		Fileread = 1;
	
		/* When the hex file is opened, the program will read it in 2 passes.
		The first pass gets the highest and lowest addresses so that we can allocate
		the right size.
		The second pass processes the hex data. */
	
		/* To begin, assume the lowest address is at the end of the memory.
		 While reading each records, subsequent addresses will lower this number.
		 At the end of the input file, this value will be the lowest address.
	
		 A similar assumption is made for highest address. It starts at the
		 beginning of memory. While reading each records, subsequent addresses will raise this number.
		 At the end of the input file, this value will be the highest address. */

		Lowest_Address = (unsigned int)-1;
		Highest_Address = 0;
		Records_Start = 0;
		Segment = 0;
		Upper_Address = 0;
		Record_Nb = 0;	  // Used for reporting errors
		First_Word = 0;

		printf("\n");
		
		/* Check if are set Floor and Ceiling Address and range is coherent*/
		VerifyRangeFloorCeil();
	
		/* get highest and lowest addresses so that we can allocate the rintervallo incoerenteight size */
		do
		{
			unsigned int i;
	

			/* Read a line from input file. */
			memset( HexaLine, 0x00, sizeof(HexaLine) );
	
			if( NULL == fgets( HexaLine, HEX_MAX_LINE_SIZE, inpfile ) )
			{
				break;
			}
			
			Record_Nb++;

	
			/* Remove carriage return/line feed at the end of line. */
			i = strlen(HexaLine);

		#if 0 // 2016.03.09_initial
			if (--i != 0)
		#else
			if (--i > 0)
		#endif
			{
				if (HexaLine[i] == '\n') HexaLine[i] = '\0';
				if (HexaLine[i] == '\r') HexaLine[i] = '\0'; // added by 2016.03.10
	
				/* Scan the first two bytes and nb of bytes.
				   The two bytes are read in First_Word since its use depend on the
				   record type: if it's an extended address record or a data record.
				   */
				result = sscanf (HexaLine, ":%2x%4x%2x%s",&Nb_Bytes,&First_Word,&Type,Data_Str);
				if (result != 4 && (result!=-1 && i>0) ) 
				{
					// fprintf(stderr,"Error in line %d of hex file\n", Record_Nb);
					printf("Line%6d :0-Error in hex file. result(%d) Length:%d \n", Record_Nb, result, i );

					iErrCount ++; // 2016.03.05


					// ---------------------------------
					if( iErrCount > MAX_ERR_COUNT ) // 2020.06.30
					{
						printf("\n\nCheck Intel hexa family type!!!  Maybe MOTOROLA family in this hex file!! \n");
					
						iErrCount = 0; // clear

						AllFilesClosed();

						exit(0);
						return 1;
					}
					// ---------------------------------

				}
			
				pBin = (char *) Data_Str;
	
				/* If we're reading the last record, ignore it. */
				switch (Type)
				{
				/* Data record */
				case 0: /* 16/20/32bit address offset */
					if (Nb_Bytes == 0) // Length 
						break;
	
					Address = First_Word;
	
					if (Seg_Lin_Select == SEGMENTED_ADDRESS)
					{
						Phys_Addr = (Segment << 4) + Address;
					}
					else
					{

				#if HEX2BIN_INTEL_ZERO_FORCED // 2020.06.29
						// ----------------------------------------
						// Hex Address Zero based converted 
						// ----------------------------------------
						if( HEX2BIN_ZERO_FORCED==Enable_HexaAddr_Zero_Forced )
						{
							if( Upper_Address>INTEL_BASE_ADDR_CMP )
							{
								Phys_AddrTemp = Upper_Address;
								Upper_Address = 0x0000;
								
								if(verbose)
								printf("Line%6d :%d: Upper_Addr: 0x%X to zero-based Addr: 0x%X \n", Record_Nb, Type, Phys_AddrTemp, Upper_Address );
							}
						}
				#endif

						/* LINEAR_ADDRESS or NO_ADDRESS_TYPE_SELECTED
						   Upper_Address = 0 as specified in the Intel spec. until an extended address
						   record is read. */
						Phys_Addr = ((Upper_Address << 16) + Address);
					}
	
					if (verbose) fprintf(stderr,"Line%6d :0:Physical Address: %#08x  ",Record_Nb, Phys_Addr);
	
					/* Floor address */
					if (Floor_Address_Setted)
					{
						/* Discard if lower than Floor_Address */
					  if (Phys_Addr < (Floor_Address - Starting_Address)) {
						if (verbose) fprintf(stderr,"Line%6d :0:Discard physical address less than %#08x \n",Record_Nb, Floor_Address - Starting_Address);
						break; 
					  }
					}
					/* Set the lowest address as base pointer. */
					if (Phys_Addr < Lowest_Address)
						Lowest_Address = Phys_Addr;
	
					/* Same for the top address. */
					temp = Phys_Addr + Nb_Bytes -1;
	
					/* Ceiling address */
					if (Ceiling_Address_Setted)
					{
						/* Discard if higher than Ceiling_Address */
						if (temp  > (Ceiling_Address +	Starting_Address)) {
						  if (verbose) fprintf(stderr,"Line%6d :0:Discard physical address more than %#08x \n", Record_Nb, Ceiling_Address + Starting_Address);
						  break;
						}
					}
					
					if (temp > Highest_Address)
					{
						Highest_Address = temp;
						
						Real_Length = Highest_Address; // added at 2016.03.05, for real file size
					}

					if (verbose) 
					{
						if(iVerbosType<2) fprintf(stderr,"Line%6d :0:Highest_Address: %#08x  \n", Record_Nb, Highest_Address);
						else fprintf(stderr,"Line%6d :0:Highest_Address: %#08x  \n", Record_Nb, Highest_Address);
					}
					break;
		
				case 1:
					if (verbose) 
					{
						ClearScreen();
						fprintf(stderr,"Line%6d :1:End of File record\n", Record_Nb);
					}
					break;
	
				case 2: // 20bit address
					/* First_Word contains the offset. It's supposed to be 0000 so
					   we ignore it. */
	
					/* First extended segment address record ? */
					if (Seg_Lin_Select == NO_ADDRESS_TYPE_SELECTED)
						Seg_Lin_Select = SEGMENTED_ADDRESS;
	
					/* Then ignore subsequent extended linear address records */
					if (Seg_Lin_Select == SEGMENTED_ADDRESS)
					{
						result = sscanf (pBin, "%4x%2x",&Segment,&temp2);
						if (result != 2) 
						{
							fprintf(stderr,"Line%6d :2:Error in hex file\n", Record_Nb);
							iErrCount ++; // 2016.03.05
						}
						
						if (verbose) 
						{
							ClearScreen();
							fprintf(stderr,"Line%6d :2:Extended Segment Address record: %#04x \n",Record_Nb, Segment);
						}
						
						/* Update the current address. */
						Phys_Addr = (Segment << 4);
					}
					else
					{
						fprintf(stderr,"Line%6d :2:Ignored extended linear address record\n", Record_Nb);
					}
					break;
	
				case 3: // 20bit address
					if (verbose) 
					{
						ClearScreen();
						fprintf(stderr,"Line%6d :3:Start Segment Address record: ignored\n", Record_Nb);
					}
					break;
	
				case 4: // 32bit address
					/* First_Word contains the offset. It's supposed to be 0000 so
					   we ignore it. */
	
					/* First extended linear address record ? */
					if (Seg_Lin_Select == NO_ADDRESS_TYPE_SELECTED)
						Seg_Lin_Select = LINEAR_ADDRESS;
	
					/* Then ignore subsequent extended segment address records */
					if (Seg_Lin_Select == LINEAR_ADDRESS)
					{
						result = sscanf (pBin, "%4x%2x",&Upper_Address,&temp2);
						if (result != 2) 
						{
							fprintf(stderr,"Line%6d :4:Error in hex file. \n", Record_Nb);
							iErrCount ++; // 2016.03.05
						}
						
						if (verbose) 
						{
							ClearScreen();
							printf("Line%6d :4:Extended Linear Address record: %#04x \n",Record_Nb, Upper_Address);
						}

						/* Update the current address. */
						Phys_Addr = (Upper_Address << 16);

					#if HEX2BIN_INTEL_ZERO_FORCED // 2020.06.29
						// ----------------------------------------
						// Hex Address Zero based converted 
						// ----------------------------------------
						if( HEX2BIN_ZERO_FORCED==Enable_HexaAddr_Zero_Forced )
						{
							
							//[2020/06/29;08:49:52.916] [dnw] :4: Extended Linear Address record: Upper_Addr:0x0800, Segment:0x0000 
							//[2020/06/29;08:49:52.932] [dnw] :4: Physical Address: 0x08000000 
				
							// :02 0000 04 0030 CA -> Extended Linear Address Record (32bit address format)
							// :02 0000 05	   -> Extended Linear Address Record (32bit address format)
							//
							// :02 0000 02	   -> Extended Linear Address Record (20bit address format)
							// :02 0000 03	   -> Extended Linear Address Record (20bit address format)
							//
							// :0E 0000 00 FF730332FFA480FF0F800FA00F409C
							// :00 0000 01 FF -> end-of-file recoed
				
							if( Upper_Address>INTEL_BASE_ADDR_CMP )
							{
								if( iOnceCheck )
								{
									iOnceCheck = 0;
									Phys_AddrTemp = Phys_Addr;
									Upper_Address = 0x0000;
									
									printf("Line%6d :%06d:Upper_Address: 0x%X to zero-based addr: 0x%X \n", Record_Nb, Type, Phys_AddrTemp, Upper_Address );
									printf("                   Parsig on the rebased address: 0x%X from 0x%X.\n", Upper_Address, Phys_AddrTemp);
								}
							}

							if( Phys_AddrTemp && (0==iOnceCheck) )
							{
								Phys_Addr -= Phys_AddrTemp;
							}

						}
					#endif

						if (verbose) 
						{
							ClearScreen();
							fprintf(stderr,"Line%6d :4:Physical Address              : %#08x \n", Record_Nb, Phys_Addr);
						}
					}
					else
					{
						ClearScreen();
						fprintf(stderr,"Line%6d :4:Ignored extended segment address record. \n", Record_Nb);
					}
					break;
	
				case 5: // 32bit address
					if (verbose) 
					{
						ClearScreen();
						fprintf(stderr,"Line%6d :5:Start Linear Address record   : ignored \n", Record_Nb );
					}
					break;
	
				default:
					if (verbose) 
					{
						ClearScreen();
						fprintf(stderr,"Line%6d :%d:Unknown Intel record type.  Maybe MOTOROLA family type!! \n", Record_Nb, Type );
					}
					break;
				}
			}
		}
		while (!feof (inpfile));

		
		// ---------------------------------
		if( iErrCount > MAX_ERR_COUNT ) // 2016.03.05
		{
			printf("\n\nCheck Intel hexa family type!!! Maybe MOTOROLA family hex type!!\n");

			iErrCount = 0; // clear

			AllFilesClosed();

			exit(0);
			return 1;
		}
		// ---------------------------------

	
		if (Address_Alignment_Word)
			Highest_Address += (Highest_Address - Lowest_Address) + 1;
	
		Allocate_Memory_And_Rewind();
	
		Segment = 0;
		Upper_Address = 0;
		Record_Nb = 0;
	
		/* Read the file & process the lines. */
		do /* repeat until EOF(Filin) */
		{
			unsigned int i;
	

			memset( HexaLine, 0x00, sizeof(HexaLine) );
	
			/* Read a line from input file. */
			if( NULL == fgets( HexaLine, HEX_MAX_LINE_SIZE, inpfile ) )
			{
				break;
			}

			Record_Nb++;
	
			/* Remove carriage return/line feed at the end of line. */
			i = strlen(HexaLine);


			//fprintf(stderr,"Record: %d; length: %d\n", Record_Nb, i);
		#if 0
			if (--i != 0)
		#else
			if (--i > 0)
		#endif
			{
				if (HexaLine[i] == '\n') { HexaLine[i] = '\0'; }
				if (HexaLine[i] == '\r') { HexaLine[i] = '\0'; }
	
				/* Scan the first two bytes and nb of bytes.
				   The two bytes are read in First_Word since its use depend on the
				   record type: if it's an extended address record or a data record.
				*/
				memset( Data_Str, 0x00, sizeof(Data_Str) ); // added at 2016.03.09

				result = sscanf (HexaLine, ":%2x%4x%2x%s",&Nb_Bytes,&First_Word,&Type,Data_Str);
				if (result != 4) 
				{ 
					printf("Intel family : %4d line error. (%d) \n", Record_Nb, result );
				}
	
				Checksum = Nb_Bytes + (First_Word >> 8) + (First_Word & 0xFF) + Type;
	
				pBin = (char *) Data_Str;
	
				/* If we're reading the last record, ignore it. */
				switch (Type)
				{
				/* Data record */
				case 0:
					if (Nb_Bytes == 0)
					{
						//printf("0 byte length Data record ignored\n");
						break;
					}
	
					Address = First_Word;
	
					if (Seg_Lin_Select == SEGMENTED_ADDRESS)
					{
						Phys_Addr = (Segment << 4) + Address;
					}
					else
					{

				#if HEX2BIN_INTEL_ZERO_FORCED // 2020.06.29
						// ----------------------------------------
						// Hex Address Zero based converted 
						// ----------------------------------------
						if( HEX2BIN_ZERO_FORCED==Enable_HexaAddr_Zero_Forced )
						{
							if( Upper_Address>INTEL_BASE_ADDR_CMP )
							{
								Phys_AddrTemp = Upper_Address;
								Upper_Address = 0x0000;
								
								if(verbose)
								printf("Line%6d :%d: Upper_Addr:: 0x%X to zero-based Addr:: 0x%X \n", Record_Nb, Type, Phys_AddrTemp, Upper_Address );
							}
						}
				#endif
            	
						/* LINEAR_ADDRESS or NO_ADDRESS_TYPE_SELECTED
						   Upper_Address = 0 as specified in the Intel spec. until an extended address
						   record is read. */
						if (Address_Alignment_Word)
							Phys_Addr = ((Upper_Address << 16) + (Address << 1)) + Offset;
						else
							Phys_Addr = ((Upper_Address << 16) + Address);
					}

					/* Check that the physical address stays in the buffer's range. */
					if ((Phys_Addr >= Lowest_Address) && (Phys_Addr <= Highest_Address))
					{
						/* The memory block begins at Lowest_Address */
						Phys_Addr -= Lowest_Address;
	
						pBin = ReadDataBytes(pBin);
	
						/* Read the Checksum value. */
						result = sscanf (pBin, "%2x",&temp2);
						if (result != 1) fprintf(stderr,":0:Error in line %d of hex file. \n", Record_Nb);
	
						/* Verify Checksum value. */
						Checksum = (Checksum + temp2) & 0xFF;

						VerifyChecksumValue(temp2, hexFamily); /* hexFamily:1 INTEL family*/
					}
					else
					{
						if (Seg_Lin_Select == SEGMENTED_ADDRESS)
							fprintf(stderr,":0:Data record skipped at %#6x:%4x\n",Segment,Address);
						else
							fprintf(stderr,":0:Data record skipped at %#10x\n",Phys_Addr);
					}
	
					break;
	
				/* End of file record */
				case 1:
					/* Simply ignore checksum errors in this line. */
					break;
	
				/* Extended segment address record */
				case 2:
					/* First_Word contains the offset. It's supposed to be 0000 so
					   we ignore it. */
	
					/* First extended segment address record ? */
					if (Seg_Lin_Select == NO_ADDRESS_TYPE_SELECTED)
						Seg_Lin_Select = SEGMENTED_ADDRESS;
	
					/* Then ignore subsequent extended linear address records */
					if (Seg_Lin_Select == SEGMENTED_ADDRESS)
					{
						result = sscanf (pBin, "%4x%2x",&Segment,&temp2);
						if (result != 2) fprintf(stderr,":2:Error in line %d of hex file (result:%d) \n", Record_Nb, result);
	
						/* Update the current address. */
						Phys_Addr = (Segment << 4);
	
						/* Verify Checksum value. */
						Checksum = (Checksum + (Segment >> 8) + (Segment & 0xFF) + temp2) & 0xFF;

						VerifyChecksumValue((Segment >> 8) + (Segment & 0xFF) + temp2, hexFamily);  /* hexFamily:1 INTEL family*/

					}
					break;
	
				/* Start segment address record */
				case 3:
					/* Nothing to be done since it's for specifying the starting address for
					   execution of the binary code */
					break;
	
				/* Extended linear address record */
				case 4:
					/* First_Word contains the offset. It's supposed to be 0000 so
					   we ignore it. */
	
					if (Address_Alignment_Word)
					{
						sscanf (pBin, "%4x",&Offset);
						Offset = Offset << 16;
						Offset -= Lowest_Address;
	
					}
					/* First extended linear address record ? */
					if (Seg_Lin_Select == NO_ADDRESS_TYPE_SELECTED)
						Seg_Lin_Select = LINEAR_ADDRESS;
	
					/* Then ignore subsequent extended segment address records */
					if (Seg_Lin_Select == LINEAR_ADDRESS)
					{
						result = sscanf (pBin, "%4x%2x",&Upper_Address,&temp2);
						if (result != 2) printf(":4:Error in line %d of hex file (result:%d) \n", Record_Nb, result);
	

					#if HEX2BIN_INTEL_ZERO_FORCED  // 2020.06.29
						// ----------------------------------------
						// Hex Address Zero based converted 
						// ----------------------------------------
						if( HEX2BIN_ZERO_FORCED==Enable_HexaAddr_Zero_Forced )
						{
							if( Upper_Address>INTEL_BASE_ADDR_CMP )
							{
								Phys_AddrTemp = Upper_Address;
								Upper_Address = 0x0000;
								
								if(verbose)
								printf("Line%6d :%d: Upper_Addr:*: 0x%X to zero-based Addr:*: 0x%X \n", Record_Nb, Type, Phys_AddrTemp, Upper_Address );
							}
						}
					#endif

						/* Update the current address. */
						Phys_Addr = (Upper_Address << 16);
	
						/* Verify Checksum value. */
						Checksum = (Checksum + (Upper_Address >> 8) + (Upper_Address & 0xFF) + temp2) & 0xFF;

						//VerifyChecksumValue( temp2, hexFamily); /* hexFamily:1 INTEL family*/

					#if HEX2BIN_INTEL_ZERO_FORCED // 2020.06.29
						// ----------------------------------------
						// Hex Address Zero based converted 
						// ----------------------------------------
						if( HEX2BIN_ZERO_FORCED==Enable_HexaAddr_Zero_Forced )
						{
							Checksum = (Checksum + (Phys_AddrTemp >> 8) + (Phys_AddrTemp & 0xFF) ) & 0xFF;							

							//fprintf(stderr,"[+FORCED+1+]");
							//VerifyChecksumValue( (Phys_AddrTemp >> 8) + (Phys_AddrTemp & 0xFF), hexFamily); /* hexFamily:1 INTEL family*/
							VerifyChecksumValue( Checksum, hexFamily); /* hexFamily:1 INTEL family*/
						}
						else
						{
							VerifyChecksumValue( temp2, hexFamily); /* hexFamily:1 INTEL family*/
						}
					#else

						VerifyChecksumValue( temp2, hexFamily); /* hexFamily:1 INTEL family*/

					#endif



					}
					break;
	
				/* Start linear address record */
				case 5:
					/* Nothing to be done since it's for specifying the starting address for
					   execution of the binary code */
					break;
				default:
					if (verbose) 
					{
						fprintf(stderr,"Intel Unknown record type (%d) \n", Type);
					}
					break;
				}
			}
		}
		while (!feof (inpfile));
		/*-----------------------------------------------------------------------------*/
	
		printf("Binary file start : 0x%08x \n", Lowest_Address);
		printf("Records start     : 0x%08x \n", Records_Start);
		printf("Highest address   : 0x%08x \n", Highest_Address);
		printf("Pad byte(def:0xff): 0x%02X \n", Pad_Byte);
	

	#if HEX2BIN_INTEL_ZERO_FORCED // 2016.03.10
		// ----------------------------------------
		// Hex Address Zero based converted 
		// ----------------------------------------
		if( HEX2BIN_ZERO_FORCED==Enable_HexaAddr_Zero_Forced )
		{
			if( Phys_AddrTemp )
			{
				printf("-------------------------------------------------------- \n");
				printf("Hex file real start addr : 0x%08X \n", Lowest_Address + Phys_AddrTemp );
				printf("Hex real Highest address : 0x%08X \n", Highest_Address + Phys_AddrTemp );
				printf("Hex real starting addr.  : 0x%08X \n", Starting_Address + Phys_AddrTemp );
				printf("-------------------------------------------------------- \n");
			}
		}
	#endif


		WriteMemory();
	
		if (Status_Checksum_Error && Enable_Checksum_Error)
		{
			printf("\n");
			printf("INTEL family haxa: Checksum error (%d) detected !!!\n", checksum_err_cnt);

			AllFilesClosed();

			exit(0);
			return 1;
		}
		else
		{
			printf("--------------------------------------------------------\n");
			printf("Parserd line number of hex record : %d lines \n", Record_Nb);
			printf("Converted the Intel hexa to binary. -- OK \n");

			AllFilesClosed();

			exit(0);
			return 1;
		}

	}

#if SHIFT_QUALITY_DATA_SORTING /* 2022-11-13 */
	else if( (1==isUpShift) && (1==isShift) )
	{
		tSQData_PairCheck_type SPoint; 
		unsigned int iSBchk=0;
		unsigned int ignoredCnt = 0;
		int ii=0, iavgtm=0;
		tSQData_PairCheck_type SRPoint; 

		fprintf(stderr,"\r\n");
		fprintf(stderr,"----------------------------------------------------------------------------------\n" );
		fprintf(stderr,"Shift Quality Data Sorting --> UP Shift...   \n");
		fprintf(stderr,"----------------------------------------------------------------------------------\n" );


		// --------------------------------------------------------
		// 1st STEP -----------------------------------------------
		// input  : key-in~~~~
		// output : ~~.ECO
		iavgtm = ShiftQualData(iModeID, iPwrOnOff, SHIFT_UP, iSBchoicePnt, &SPoint, &iSBchk);

		// --------------------------------------------------------
		// 2nd STEP -----------------------------------------------
		// input  : *.ECO
		// output : *.bECO if return value ichk==1
		ignoredCnt = SSnNtPointFix(iModeID, iSBchoicePnt, SPoint, &SRPoint, &iSBchk);



		// 3rd STEP -----------------------------------------------
		// input  : *.bECO 
		// output : *.iECO -> GMax and Gmin Table 
		if(ignoredCnt)
		{
			ignored_QSData(iModeID, iSBchoicePnt, &SRPoint);
		}



		// 3rd STEP -----------------------------------------------
		// input  : *.bECO or *.iECO
		// output : *.gECO -> GMax and Gmin Table 
		FindGminMaxShiftData(iModeID, SRPoint, ignoredCnt);



		// --------------------------------------------------------
		// 4th STEP -----------------------------------------------
		// input  : *.gECO 
		// output : *.dta // *.txt
		ignoredCnt = ShiftData_Filtering(iModeID, iavgtm, iSBchoicePnt);



		// 5th STEP -----------------------------------------------
		// input  : *.dat 
		// output : *.txt
		ignoredCnt = APSData_Filtering(iModeID, iavgtm, iSBchoicePnt, &SPoint);


		if( SPoint.SStot>0 && SPoint.SBtot>0 && SPoint.SPtot>0 )
		{
			// --------------------------------------------------------
			// 6th STEP -----------------------------------------------
			// input  : *.txt 
			// output : *.gil
			ShiftData_LastSorting(iModeID, iavgtm, iSBchoicePnt, ignoredCnt, itmpFileDeleted, iSBchk);
			// --------------------------------------------------------


			// --------------------------------------------------------
			// 7th STEP -----------------------------------------------
			// input  : *.gil 
			// output : *.rpt
			ShiftData_Report(iModeID, iavgtm, iSBchoicePnt, gValueDisplay);
		}
		// --------------------------------------------------------
		// 8th temp file deleted!! --------------------------------
		tempFileDeleteIt(itmpFileDeleted, iModeID, iSBchk);


	}
	else if( (1==isDownShift) && (1==isShift) )
	{
		printf("\r\n");
		fprintf(stderr,"----------------------------------------------------------------------------------\n" );
		fprintf(stderr,"Shift Quality Data Sorting --> DOWN Shift...   \n");
		fprintf(stderr,"----------------------------------------------------------------------------------\n" );










	}
#endif
	
	else if(1 == isFloat2Hex)
	{
		size_t i;	 
		__float_hex_union fh_v;


		if( inpfile && outfile )
		{
			fprintf(outfile,"-------float_number-----------LSB_format---------MSB_format----- \r\n");

			i_readCount = 0;
			while( EOF != (c=fscanf(inpfile,"%s", str_float)) )
			{
				i_readCount ++;

				f2h_float = atof( str_float );

				if( (TRUE==verbose) && 1==iVerbosType )
					printf("reading:[%-20s]   ->  using data:[%15.6f] \n", str_float , f2h_float );
		
				fprintf(outfile,"%15.6f  ->         ", f2h_float);
				fh_v.flt_val = (float)f2h_float;
				
				/* LSB */
				fprintf(outfile, " %02X-%02X-%02X-%02X        ", fh_v.flt_bin[3], fh_v.flt_bin[2], fh_v.flt_bin[1], fh_v.flt_bin[0] ); 
				/* MSB */
				fprintf(outfile, " %02X-%02X-%02X-%02X \r\n", fh_v.flt_bin[0], fh_v.flt_bin[1], fh_v.flt_bin[2], fh_v.flt_bin[3] ); 

			}

			AllFilesClosed();

		}	
		else
		{

			printf("-------float_number-----------LSB_format---------MSB_format----- \n");
			fh_v.flt_val = (float)f2h_float;


			printf("  %16.6f       -> ", f2h_float);

			/* LSB */
			printf(" %02X-%02X-%02X-%02X        ", fh_v.flt_bin[3], fh_v.flt_bin[2], fh_v.flt_bin[1], fh_v.flt_bin[0] ); 
			/* MSB */
			for ( i = 0; i < sizeof(fh_v.flt_bin); ++i )
			{		
				printf("%02X%c", fh_v.flt_bin[i],  i<(sizeof(fh_v.flt_bin)-1) ? '-' : '\n'); 
			}

		#if __NOT_USED___
			{
				double	pit_tmp, rol_tmp1, rol_tmp2;
				
				pit_tmp = asin( fh_v.flt_val ) * 180.0f/MATH_PI;
				//rol_tmp1 = asin( Cal_Yup.f_val / cos(ang_pitch) );
				//rol_tmp2 = acos( Cal_Zup.f_val / cos(ang_pitch) );
			
				printf("re-degree	 %.2f	  \n",  pit_tmp );
			}	
		#endif


		}
		

		opt_ok = 0; /* 2014.07.16 : NOTE !!! clear!!!  */

		if(i_readCount)
			printf("\nFLT>> Converted float number to hex-decimal!! Count:%d - OK", i_readCount);
		else			
			printf("\nFLT>> Converted float number to hex-decimal!! - OK");


	}

#if CONVERT_BMP2C
	else if(1 == isConvertBMP2C)
	{
    BITMAPFILEHEADER bf;
    BITMAPINFO bmi;

	PixelData *databuffer = NULL;
	DWORD bytes_read = 0, bmpsize, fillindex, nextlinei, linei, i, offset;
	int swaprgb = 0;
	int binout = 0;


		bmpsize = bmi.bmiHeader.biWidth * bmi.bmiHeader.biHeight;
		databuffer = (PixelData *) malloc(sizeof(PixelData) * bmpsize);
		if(NULL == databuffer)
		{
			printf("Error allocating temporary data buffer, is image too big?\n");
			fclose(infile);
			exit(-1);
		}



		switch(iRGB)
		{
			case 888:
				break;

			case 444:
				break;

			case 555:
				break;

			case 565:
				break;


			default:
				break;
		}

		if(databuffer) free(databuffer);

		opt_ok = 0; /* 2014.07.16 : NOTE !!! clear!!!  */

		printf("\nBMP>> Converted BMP file to C text file - OK" );	


	}

	
	{  

		BITMAPFILEHEADER fileHeader;	// 비트맵 파일 헤더 구조체 변수
		BITMAPINFOHEADER infoHeader;	// 비트맵 정보 헤더 구조체 변수
	
		unsigned char *image;	 // ?? ??? ???
		int size;				 // ?? ??? ??
		int width, height;		 // ??? ???? ??, ?? ??
		int padding;			 // ?? ???? ?? ??? 4? ??? ?? ? ?? ??? ??
	
		// ? ??? ??? ASCII ??. ???? ?? ?? ???? ?? ??
		char ascii[] = { '#', '#', '@', '%', '=', '+', '*', ':', '-', '.', ' ' };	// 11?
	
		fpBmp = fopen("Peppers80x80.bmp", "rb");	// ??? ??? ???? ??? ??
		if (fpBmp == NULL)	  // ?? ??? ????
			return 0;		  // ???? ??
	
		// ??? ?? ?? ??. ??? ???? ?? ???? ?? ???? ??
		if (fread(&fileHeader, sizeof(BITMAPFILEHEADER), 1, inpfile) < 1)
		{
			fclose(inpfile);
			return 0;
		}
	
		// ?? ??? MB? ??? ??(2??? ??? BM? ?? ????? ????? MB? ?)
		// ?? ??? ?? ??? ???? ??
		if (fileHeader.bfType != 'MB')
		{
			fclose(inpfile);
			return 0;
		}
	
		// ??? ?? ?? ??. ??? ???? ?? ???? ?? ???? ??
		if (fread(&infoHeader, sizeof(BITMAPINFOHEADER), 1, inpfile) < 1)
		{
			fclose(inpfile);
			return 0;
		}
	
		// 24?? ???? ??? ???? ??
		if (infoHeader.biBitCount != 24)
		{
			fclose(inpfile);
			return 0;
		}
	
		size = infoHeader.biSizeImage;	  // ?? ??? ??
		width = infoHeader.biWidth; 	  // ??? ???? ?? ??
		height = infoHeader.biHeight;	  // ??? ???? ?? ??
	
		// ???? ?? ??? ?? ??? ??? ?? ? ?? ??? ??? 4? ???? ??
		// ??? 4?? ???? ??? ?? ??? ?? ? ??.
		// ?? ?? ??? 0??? ?? ??? 4? ??? ??? ?? 4? ???? ??
		padding = (PIXEL_ALIGN - ((width * PIXEL_SIZE) % PIXEL_ALIGN)) % PIXEL_ALIGN;
	
		if (size == 0)	  // ?? ??? ??? 0???
		{
			// ???? ?? ?? * ?? ??? ?? ??? ???? ??? ?? ? ? ??? ??
			// ??? ???? ?? ??? ???? ?? ???? ??? ?? ? ??
			size = (width * PIXEL_SIZE + padding) * height;
		}
	
		image = malloc(size);	 // ?? ???? ???? ?? ??? ??
	
		// ?? ???? ?? ???? ?? ??? ??
		fseek(inpfile, fileHeader.bfOffBits, SEEK_SET);
	
		// ???? ?? ??? ???? ??. ??? ???? ?? ???? ?? ???? ??
		if (fread(image, size, 1, inpfile) < 1)
		{
			fclose(inpfile);
			return 0;
		}
	
		fclose(inpfile);	  // ??? ?? ??
	
		outfile = fopen("ascii.txt", "w");	// ?? ??? ??? ?? ??
		if (outfile == NULL)	  // ?? ??? ????
		{
			free(image);	  // ?? ???? ??? ?? ??? ??
			return 0;		  // ???? ??
		}
	
		// ?? ???? ?? ?? ???? ????? ????? ??
		// ?? ???? ??
		for (int y = height - 1; y >= 0; y--)
		{
			// ?? ???? ??
			for (int x = 0; x < width; x++)
			{
				// ??? ? ??? ???? ?? ???? ??
				// (x * ?? ??)? ??? ?? ??
				// (y * (?? ?? * ?? ??))? ??? ? ?? ??? ??
				// ?? ?? * y? ??? ??? ?? ??
				int index = (x * PIXEL_SIZE) + (y * (width * PIXEL_SIZE)) + (padding * y);
	
				// ?? ??? ??? RGBTRIPLE ???? ???? RGBTRIPLE ???? ??
				RGBTRIPLE *pixel = (RGBTRIPLE *)&image[index];
	
				// RGBTRIPLE ???? ??, ??, ???? ???
				unsigned char blue = pixel->rgbtBlue;
				unsigned char green = pixel->rgbtGreen;
				unsigned char red = pixel->rgbtRed;
	
				// ??, ??, ???? ??? ??? ?? ???? ?? ? ??
				unsigned char gray = (red + green + blue) / PIXEL_SIZE;
	
				// ???? ASCII ??? ??? ?? ? 256?? ??? ???? ?? 
				// ASCII ??? ???? ?? ? ??
				char c = ascii[gray * sizeof(ascii) / 256];
	
				// ??? ?????? ??? ??, ?? ??? ????
				// ?? ASCII ??? ??? ??? ????? ???? ??? ???? ???? ??
				// ?? ??? ? ? ????
				fprintf(outfile, "%c%c", c, c);	 // ??? ??? ?? ??
			}
	
			fprintf(outfile, "\n");	 // ?? ?? ??? ???? ???? ??
		}
	
		fclose(outfile);	  // ??? ?? ??
	
		free(image);	  // ?? ???? ??? ?? ??? ??
	
		return 0;
	}

#endif /// CONVERT_BMP2C


#if MD5_CHECKSUM_ENCIPHER /// 2014.06.27, MD5 Checksum
	else if(1 == isMD5)
	{
		/* Computes the message digest for a specified file.
		Prints out message digest, a space, the file name, and a carriage return.
		*/
		MD5_CTX mdContext;
		int nBytes;
		unsigned char md5_data[MD_HASH_BUFSIZ]; // 1024*10];
		struct	tm *pTime;

		pTime = current_time();

		if( (1==iVerbosType || 3==iVerbosType) && (TRUE == verbose) && outfile) /// 2014.08.01
		{
			fprintf(outfile, "----------------------------------------------------------------------------------\r\n" );
			fprintf(outfile, "MD5 Hash is created at %04d/%02d/%02d/%s %02d:%02d:%02d by %s \n", pTime->tm_year, pTime->tm_mon, pTime->tm_mday,  
													WeekTXT[pTime->tm_wday], pTime->tm_hour, pTime->tm_min, pTime->tm_sec, EmailText );
			fprintf(outfile, "----------------------------------------------------------------------------------\r\n" );
		}

		LOG_V("\n");

	#if MD5_MULTI_INPUT_FILES
		if( ASTERISK_FOUND == isAsteMode ) /* For ah.exe --input *.* */
		{
			unsigned __int64 	kll = 0UL;


			printf("MD5>> MD5 hashing for input files* \n");
			for(ii=0; ii<16*2; ii++) printf("-");  
			printf(" %d --\r\n", 16*2 );

			if(inpfile) { fclose(inpfile); inpfile=NULL; }
			if(data_buf){ free(data_buf); data_buf=NULL; }


		#if 1 // 
			for(ii=iFirst; ii<=(argc-isAsteminus); ii++)
			{
				if( NULL != (inpfile = fopen( argv[ii], "rb")) ) 
				{
					/// initial ----
					memset( &mdContext, 0x00, sizeof(MD5_CTX) );
					memset( &md5_data, 0x00, sizeof(md5_data) );

					/// MD5 Calculate ----
					MD5Init (&mdContext);

					kll = 0UL;
					while ((nBytes = fread (md5_data, 1, sizeof(md5_data), inpfile)) > 0)
					{
						kll += nBytes;
						MD5Update (&mdContext, md5_data, nBytes);

						// LOG_V("\bMD5 Hashing for (%s) -> read : %lld Bytes \r", argv[ii], kll );
					}
					ClearScreen(); 
					
					MD5Final(&mdContext);
					MDPrint(&mdContext);

					if( (2==iVerbosType || 3==iVerbosType) && (TRUE == verbose) )
					{
						//printf ("  *%s*%s__(%llu) \r\n", str_hash, argv[ii], kll );
						printf ("  *%s*%s__(%s) \n", str_hash, argv[ii], commify(kll, str_cmBuf, 0) );
						if(outfile) fprintf(outfile,"  *%s*%s__(%llu) \n", str_hash, argv[ii], kll );
					}
					else
					{
						printf ("  *%s*%s \n", str_hash, argv[ii] );
						if(outfile) fprintf(outfile,"  *%s*%s \n", str_hash, argv[ii] );
					}

					iTotSize += kll;
					multifileindex ++;
				}
				
				if(inpfile) { fclose(inpfile); inpfile=NULL; }
				if(data_buf){ free(data_buf); data_buf=NULL; }

			}

			printf("\nMD5>> Number of Input files : %d files (Total size: %llu Byte) ---", multifileindex, iTotSize );

			AllFilesClosed();

		#endif
	
		}
		else if( multifileindex > 0 )
		{

			int iLenFile = 0;
			unsigned int iLenSub=0;
			iLenFile = strlen(infile_name);
			unsigned __int64 	kll = 0UL;


			if(inpfile) { fclose(inpfile); inpfile=NULL; }
			if(data_buf){ free(data_buf); data_buf=NULL; }


			/* Find first .c file in current directory */
			if( (retFiles = _findfirsti64( extfile_name, &iFiles )) == -1L )
			{
				printf("MD5>>No file [%s] in current directory!!! [%s] \n", extfile_name, infile_name );
			}
			else
			{
				printf("MD5>> MD5 hashing for input files... \n");
				for(ii=0; ii<16*2; ii++) printf("-");  
				printf(" %d --\r\n", 16*2 );

				do {

					iLenSub = strlen(iFiles.name);

					memset( ttlfilename, 0x00, MAX_FILENAME_LEN );
					memcpy( ttlfilename, iFiles.name, iLenSub );
					

					if( iFiles.attrib & _A_SUBDIR ) /* Subdirectory.  Value: 0x10. */
					{

				#if 0
						printf("subdir -- [%s] ---- \r\n", iFiles.name);
						if( 0!=strcmp(iFiles.name,"..") || 0!=strcmp(iFiles.name,".") )
						{
							std::string s2 = dn;
							s2 += "\\";
							s2 += iFiles.name;
							r += insertDirectory(s2.c_str(),list);
						}
				#endif

					}
				#if 0
					else if( iFiles.attrib & _A_SYSTEM ) /* System file.  Not normally seen with the DIR command, unless the /A or /A:S option is used.  Value: 0x04. */
					{
						printf("SYSTEM ---- [%s] \r\n", iFiles.name);
					}
					else if( iFiles.attrib & _A_RDONLY ) /* Read-only.  File cannot be opened for writing, and a file with the same name cannot be created. Value: 0x01. */
					{
						printf("READONLY -- [%s] \r\n", iFiles.name);
					}
					else if( iFiles.attrib & _A_NORMAL ) /* Normal.  File can be read or written to without restriction. Value: 0x00. */
					{
						printf("NORMAL ---- [%s] \r\n", iFiles.name);
					}
					else if( iFiles.attrib & _A_HIDDEN ) /* Hidden file.  Not normally seen with the DIR command, unless the /AH option is used.  Returns information about normal files as well as files with this attribute.  Value: 0x02. */
					{
						printf("HIDDEN ---- [%s] \r\n", iFiles.name);
					}
					else if( iFiles.attrib & _A_ARCH )  /* Archive.  Set whenever the file is changed, and cleared by the BACKUP command. Value: 0x20. */
				#endif
					else
					{
						/// FileName
						// memcpy( mulfile_name[multifileindex], iFiles.name, iLenSub );
						/// File Size
						/// mulfile_size[multifileindex] = iFiles.size;

					#if 0 // 2020.06.08
						if( iFiles.size>>20 )
							printf("\n>>Input Files  : %s (%.3f MB)", mulfile_name[multifileindex], (iFiles.size/1024.0)/1024.0 );
						else if( iFiles.size>>10 )
							printf("\n>>Input Files  : %s (%.3f kB)", mulfile_name[multifileindex], (iFiles.size/1024.0) );
						else  
							printf("\n>>Input Files  : %s (%lu Byte)", mulfile_name[multifileindex], iFiles.size );
					#endif


						if( NULL == (inpfile = fopen( ttlfilename, "rb")) ) 
						{
							beep(700,100);
							printf("\n\n[++ERROR++] Can not open multi-input file (%s) \n", ttlfilename );

							AllFilesClosed();

							exit(0);
							return 0;
						}

						/// initial ----
						memset( &mdContext, 0x00, sizeof(MD5_CTX) );
						memset( &md5_data, 0x00, sizeof(md5_data) );

						/// MD5 Calculate ----
						MD5Init (&mdContext);

						kll = 0UL;
						while ((nBytes = fread (md5_data, 1, sizeof(md5_data), inpfile)) != 0)
						{
							kll += nBytes;
						
							MD5Update (&mdContext, md5_data, nBytes);

							// LOG_V("\bMD5 Hashing for (%s) -> read : %lld Bytes \r", ttlfilename, kll );
						}
						ClearScreen(); 
						
						MD5Final(&mdContext);
						MDPrint(&mdContext);


						if( (2==iVerbosType || 3==iVerbosType) && (TRUE == verbose) )
						{
							printf ("  *%s*%s__(%llu) \r\n", str_hash, iFiles.name, iFiles.size );
							if(outfile) fprintf(outfile,"  *%s*%s__(%llu) \r\n", str_hash, iFiles.name, iFiles.size );
						}
						else
						{
							printf ("  *%s*%s \r\n", str_hash, iFiles.name );
							if(outfile) fprintf(outfile,"  *%s*%s \r\n", str_hash, iFiles.name );
						}

						if(inpfile) { fclose(inpfile); inpfile=NULL; }
						if(data_buf){ free(data_buf); data_buf=NULL; }

						iTotSize += iFiles.size;

						multifileindex ++;
					}


					//if( multifileindex >= MULTI_IN_FILES_CNT )
					//{
					//	printf("\n\n>>Too many of Input files(%d).	Max number is %u. ", multifileindex, MULTI_IN_FILES_CNT );
					//	break;
					//}
				} while( 0 == _findnexti64( retFiles, &iFiles ) );
				_findclose( retFiles );

				multifileindex--;

				if( iTotSize>>20 )
					printf("\nMD5>> Number of Input files : %d files (Total size: %.3f MB) ---", multifileindex, (iTotSize/1024.0)/1024.0 );
				else if( iTotSize>>10 )
					printf("\nMD5>> Number of Input files : %d files (Total size: %.3f kB) ---", multifileindex, (iTotSize/1024.0) );
				else
					printf("\nMD5>> Number of Input files : %d files (Total size: %llu Byte) ---", multifileindex, iTotSize );

			}

	
		}
		else
	#endif /// MD5_MULTI_INPUT_FILES
		{

			unsigned __int64 	kll = 0UL;

			printf("MD5>> MD5 hashing... \n");

			/// initial
			memset( &mdContext, 0x00, sizeof(MD5_CTX) );
			memset( &md5_data, 0x00, sizeof(md5_data) );


			MD5Init (&mdContext);

			kll = 0UL;
			while ((nBytes = fread (md5_data, 1, sizeof(md5_data), inpfile)) != 0)
			{
				kll += nBytes;
			
				MD5Update (&mdContext, md5_data, nBytes);

				// LOG_V("\bMD5 Hashing for (%s) -> read : %lld Bytes \r", infile_name, kll );
			}
			ClearScreen(); 
			
			MD5Final(&mdContext);
			MDPrint(&mdContext);

			if( (2==iVerbosType || 3==iVerbosType) && (TRUE == verbose) )
			{
				printf ("  *%s*%s__(%llu) \r\n", str_hash, infile_name, infile_size);
				if(outfile) fprintf(outfile,"  *%s*%s__(%llu) \r\n", str_hash, infile_name, infile_size );
			}
			else
			{
				printf ("  *%s*%s \r\n", str_hash, infile_name );
				if(outfile) fprintf(outfile,"  *%s*%s \r\n", str_hash, infile_name );
			}

			if(inpfile) { fclose(inpfile); inpfile=NULL; }
			if(data_buf){ free(data_buf); data_buf=NULL; }
			printf("\nMD5>> Calculated MD5 Hash Value - OK" );

		}

		opt_ok = 0; /* 2014.07.16 : NOTE !!! clear!!!  */

	}
#endif /// MD5_CHECKSUM_ENCIPHER


#if MD4_CHECKSUM_ENCIPHER /// 2014.07.26, MD4 Checksum
	else if(1 == isMD4)
	{
		/* Computes the message digest for a specified file.
		Prints out message digest, a space, the file name, and a carriage return.
		*/
		MD4_CTX mdContext;
		int nBytes;
		unsigned char md4_data[MD_HASH_BUFSIZ]; // 1024*16];
		unsigned char md4_output[MD4_DIGEST_LENGTH] = {0,};
		struct	tm *pTime; // 2020.07.15
	
		pTime = current_time();

		if( (1==iVerbosType || 3==iVerbosType) && (TRUE == verbose) && outfile ) /// 2014.08.01
		{
			fprintf(outfile, "----------------------------------------------------------------------------------\r\n" );
			fprintf(outfile, "MD4 Hash is created at %04d/%02d/%02d/%s %02d:%02d:%02d by %s \n", pTime->tm_year, pTime->tm_mon, pTime->tm_mday,  
													WeekTXT[pTime->tm_wday], pTime->tm_hour, pTime->tm_min, pTime->tm_sec, EmailText );
			fprintf(outfile, "----------------------------------------------------------------------------------\r\n" );
		}


		LOG_V("\n");

		if( ASTERISK_FOUND == isAsteMode ) /* For ah.exe --input *.* */
		{
			unsigned __int64 	kll = 0UL;


			printf("MD4>> MD4 hashing for input files* \n");
			for(ii=0; ii<MD4_DIGEST_LENGTH*2; ii++) printf("-");  
			printf(" %d --\r\n", MD4_DIGEST_LENGTH*2 );

			if(inpfile) { fclose(inpfile); inpfile=NULL; }
			if(data_buf){ free(data_buf); data_buf=NULL; }

			for(ii=iFirst; ii<=(argc-isAsteminus); ii++)
			{
				if( NULL != (inpfile = fopen( argv[ii], "rb")) ) 
				{
					/// initial ----
					memset( &mdContext, 0x00, sizeof(MD4_CTX) );
					memset( &md4_data, 0x00, sizeof(md4_data) );
					memset( &md4_output, 0x00, sizeof(md4_output) );

					/// MD4 Calculate ----
					MD4Init (&mdContext);
					
					kll = 0UL;
					while ((nBytes = fread (md4_data, 1, sizeof(md4_data) /*1024*/, inpfile))> 0 )
					{
						kll += nBytes;
						MD4Update (&mdContext, md4_data, nBytes);

						// LOG_V("\bMD4 Hashing for (%s) -> read : %lld Bytes \r", argv[ii], kll );
					}
					ClearScreen(); 
					
					MD4Final(md4_output, &mdContext);
					MD4Print(md4_output);

					if( (2==iVerbosType || 3==iVerbosType) && (TRUE == verbose) )
					{
						printf ("  *%s*%s__(%llu) \r\n", str_hash, argv[ii], kll	);
						if(outfile) fprintf(outfile,"  *%s*%s__(%llu) \r\n", str_hash, argv[ii], kll	);
					}
					else
					{
						printf ("  *%s*%s \r\n", str_hash, argv[ii] );
						if(outfile) fprintf(outfile,"  *%s*%s \r\n", str_hash, argv[ii] );
					}
					
					iTotSize += kll;
				
					multifileindex ++;
				}
				
				if(inpfile) { fclose(inpfile); inpfile=NULL; }
				if(data_buf){ free(data_buf); data_buf=NULL; }

			}

			printf("\nMD4>> Number of Input files : %d files (Total size: %llu Byte) ---", multifileindex, iTotSize );

			AllFilesClosed();

		}
		else if( multifileindex > 0 )
		{
			int iLenFile = 0;
			unsigned int iLenSub=0;
			iLenFile = strlen(infile_name);
			unsigned __int64 	kll = 0UL;


			if(inpfile) { fclose(inpfile); inpfile=NULL; }
			if(data_buf){ free(data_buf); data_buf=NULL; }

			/* Find first .c file in current directory */
			if( (retFiles = _findfirsti64( extfile_name, &iFiles )) == -1L )
			{
				printf("MD4>>No file [%s] in current directory!!! [%s] \n", extfile_name, infile_name );
			}
			else
			{
			
				printf("MD4>> MD4 hashing for input files... \n");
				for(ii=0; ii<MD4_DIGEST_LENGTH*2; ii++) printf("-");  
				printf(" %d --\r\n", MD4_DIGEST_LENGTH*2 );

				do {

					iLenSub = strlen(iFiles.name);
			
					if( iFiles.attrib & _A_SUBDIR )
					{

					}
					else
					{

						if( NULL == (inpfile = fopen( iFiles.name, "rb")) ) 
						{
							beep(700,100);
							printf("\n\nMD4>> [++ERROR++] Can not open multi-input file (%s) \n", iFiles.name );

							AllFilesClosed();

							exit(0);
							return 0;
						}

						/// initial ----
						memset( &mdContext, 0x00, sizeof(MD4_CTX) );
						memset( &md4_data, 0x00, sizeof(md4_data) );
						memset( &md4_output, 0x00, sizeof(md4_output) );

						/// MD4 Calculate ----
						MD4Init (&mdContext);

						kll = 0UL;
						while ((nBytes = fread (md4_data, 1, sizeof(md4_data) /*1024*/, inpfile)) != 0)
						{
							kll += nBytes;
						
							MD4Update (&mdContext, md4_data, nBytes);

							// LOG_V("\bMD4 Hashing for (%s) -> read : %lld Bytes \r", iFiles.name, kll );
						}
						ClearScreen(); 

						MD4Final(md4_output, &mdContext);
						MD4Print(md4_output);

						if( (2==iVerbosType || 3==iVerbosType) && (TRUE == verbose) )
						{
							printf ("  *%s*%s__(%llu) \r\n", str_hash, iFiles.name, iFiles.size	);
							if(outfile) fprintf(outfile,"  *%s*%s__(%llu) \r\n", str_hash, iFiles.name, iFiles.size	);
						}
						else
						{
							printf ("  *%s*%s \r\n", str_hash, iFiles.name );
							if(outfile) fprintf(outfile,"  *%s*%s \r\n", str_hash, iFiles.name );
						}
						if(inpfile) { fclose(inpfile); inpfile=NULL; }
						if(data_buf){ free(data_buf); data_buf=NULL; }

						iTotSize += iFiles.size;

						multifileindex ++;
					}

				} while( 0 == _findnexti64( retFiles, &iFiles ) );
				_findclose( retFiles );

				multifileindex--;

				printf("\nMD4>> Number of Input files : %d files (Total size: %llu Byte) ---", multifileindex, iTotSize );
			
			}

		}
		else
		{
		
			unsigned __int64	kll = 0UL;

			printf("MD4>> MD4 hashing... \n");

			/// initial
			memset( &mdContext, 0x00, sizeof(MD4_CTX) );
			memset( &md4_data, 0x00, sizeof(md4_data) );
			memset( &md4_output, 0x00, sizeof(md4_output) );


			MD4Init (&mdContext);

			kll = 0UL;
			while ((nBytes = fread (md4_data, 1, sizeof(md4_data) /*1024*/, inpfile)) != 0)
			{
				kll += nBytes;

				MD4Update (&mdContext, md4_data, nBytes);

				// LOG_V("\bMD4 Hashing for (%s) -> read : %lld Bytes \r", infile_name, kll );
			}
			ClearScreen(); 

			MD4Final(md4_output, &mdContext);
			MD4Print(md4_output);

			if( (2==iVerbosType || 3==iVerbosType) && (TRUE == verbose) )
			{
				printf ("  *%s*%s__(%llu) \r\n", str_hash, infile_name, infile_size);
				if(outfile) fprintf(outfile,"  *%s*%s__(%llu) \r\n", str_hash, infile_name, infile_size);
			}
			else
			{
				printf ("  *%s*%s \r\n", str_hash, infile_name );
				if(outfile) fprintf(outfile,"  *%s*%s \r\n", str_hash, infile_name );
			}
			
			if(inpfile) { fclose(inpfile);	inpfile=NULL; }
			if(data_buf){ free(data_buf); data_buf=NULL; }
			printf("\nMD4>> Calculated MD4 Hash Value - OK" );

		}

		opt_ok = 0; /* 2014.07.16 : NOTE !!! clear!!!  */

	}
#endif /// MD4_CHECKSUM_ENCIPHER


#if MD2_CHECKSUM_ENCIPHER /// 2014.07.29
	else if(1 == isMD2)
	{
		/* Computes the message digest for a specified file.
		Prints out message digest, a space, the file name, and a carriage return.
		*/
		MD2_CTX mdContext;
		size_t nBytes;
		unsigned char md2_data[MD_HASH_BUFSIZ]; // 1024*16];
		unsigned char md2_output[MD2_DIGEST_LENGTH] = {0,};
		unsigned int iLenSub=0;
		struct	tm *pTime;  // 2020.07.15
	
		pTime = current_time();	
		
		if( (1==iVerbosType || 3==iVerbosType) && (TRUE == verbose) && outfile ) /// 2014.08.01
		{
			fprintf(outfile, "----------------------------------------------------------------------------------\r\n" );
			fprintf(outfile, "MD2 Hash is created at %04d/%02d/%02d/%s %02d:%02d:%02d by %s \n", pTime->tm_year, pTime->tm_mon, pTime->tm_mday,  
													WeekTXT[pTime->tm_wday], pTime->tm_hour, pTime->tm_min, pTime->tm_sec, EmailText );
			fprintf(outfile, "----------------------------------------------------------------------------------\r\n" );
		}


		LOG_V("\n");
		if( ASTERISK_FOUND == isAsteMode ) /* For ah.exe --input *.* */
		{
			unsigned __int64 	kll = 0UL;

			printf("MD2>> MD2 hashing for input files* \n");
			for(ii=0; ii<MD2_DIGEST_LENGTH*2; ii++) printf("-");  
			printf(" %d --\r\n", MD2_DIGEST_LENGTH*2 );

			if(inpfile) { fclose(inpfile); inpfile=NULL; }
			if(data_buf){ free(data_buf); data_buf=NULL; }

			for(ii=iFirst; ii<=(argc-isAsteminus); ii++)
			{
				if( NULL != (inpfile = fopen( argv[ii], "rb")) ) 
				{
					/// initial ----
					memset( &mdContext, 0x00, sizeof(MD2_CTX) );
					memset( &md2_data, 0x00, sizeof(md2_data) );
					memset( &md2_output, 0x00, sizeof(md2_output) );

					/// MD2 Calculate ----
					MD2_Init(&mdContext);
					
					kll = 0UL;
					while ((nBytes = fread (md2_data, 1, sizeof(md2_data), inpfile)) > 0)
					{
						kll += nBytes;
						MD2_Update(&mdContext, md2_data, nBytes);

						// LOG_V("\bMD2 Hashing for (%s) -> read : %lld Bytes \r", argv[ii], kll );
					}
					ClearScreen(); 
					
					MD2_Final( md2_output, &mdContext );
					MD2Print( md2_output );

					if( (2==iVerbosType || 3==iVerbosType) && (TRUE == verbose) )
					{
						printf ("  *%s*%s__(%llu) \n", str_hash, argv[ii], kll );
						if(outfile) fprintf(outfile,"  *%s*%s__(%llu) \n", str_hash, argv[ii], kll );
					}
					else
					{
						printf ("  *%s*%s \n", str_hash, argv[ii] );
						if(outfile) fprintf(outfile,"  *%s*%s \n", str_hash, argv[ii] );
					}
					
					if(inpfile) { fclose(inpfile); inpfile=NULL; }
					if(data_buf){ free(data_buf); data_buf=NULL; }

					iTotSize += kll;
					multifileindex ++;
				}
			}

			printf("\nMD2>> Number of Input files : %d files (Total size: %llu Byte) ---", multifileindex, iTotSize );

			AllFilesClosed();

		}
		else if( multifileindex > 0 )
		{
			unsigned __int64 	kll = 0UL;


			if(inpfile) { fclose(inpfile); inpfile=NULL; }
			if(data_buf){ free(data_buf); data_buf=NULL; }

			/* Find first .c file in current directory */
			if( (retFiles = _findfirsti64( extfile_name, &iFiles )) == -1L )
			{
				printf("MD2>>No file [%s] in current directory!!! [%s] \n", extfile_name, infile_name );
			}
			else
			{

				printf("MD2>> MD2 hashing for input files... \n");
				//printf("--------------------------------------------------------\r\n" );
				for(ii=0; ii<MD2_DIGEST_LENGTH*2; ii++) printf("-");  
				printf(" %d --\r\n", MD2_DIGEST_LENGTH*2 );

				do {

					iLenSub = strlen(iFiles.name);

					if( iFiles.attrib & _A_SUBDIR )
					{

					}
					else
					{

						if( NULL == (inpfile = fopen( iFiles.name, "rb")) ) 
						{
							beep(700,100);
							printf("\n\nMD2>>[++ERROR++] Can not open multi-input file (%s) \n", iFiles.name );

							AllFilesClosed();

							exit(0);
							return 0;
						}
		
						/// initial ----
						memset( &mdContext, 0x00, sizeof(MD2_CTX) );
						memset( &md2_data, 0x00, sizeof(md2_data) );
						memset( &md2_output, 0x00, sizeof(md2_output) );
		
						/// MD2 Calculate ----
						MD2_Init(&mdContext);
						
						kll = 0UL;
						while ((nBytes = fread (md2_data, 1, sizeof(md2_data), inpfile)) != 0)
						{
							kll += nBytes;
							MD2_Update(&mdContext, md2_data, nBytes);

							// LOG_V("\bMD2 Hashing for (%s) -> read : %lld \r", iFiles.name, kll );
						}
						ClearScreen(); 

						MD2_Final( md2_output, &mdContext );
						MD2Print( md2_output );
		
						if( (2==iVerbosType || 3==iVerbosType) && (TRUE == verbose) )
						{
							printf ("  *%s*%s__(%llu) \n", str_hash, iFiles.name, iFiles.size );
							if(outfile) fprintf(outfile,"  *%s*%s__(%llu) \n", str_hash, iFiles.name, iFiles.size );
						}
						else
						{
							printf ("  *%s*%s \n", str_hash, iFiles.name );
							if(outfile) fprintf(outfile,"  *%s*%s \n", str_hash, iFiles.name );
						}
						
						if(inpfile) { fclose(inpfile); inpfile=NULL; }
						if(data_buf){ free(data_buf); data_buf=NULL; }

						iTotSize += iFiles.size;
						multifileindex ++;
					}

				} while( 0 == _findnexti64( retFiles, &iFiles ) );
				_findclose( retFiles );

				multifileindex--;

				printf("\nMD2>> Number of Input files : %d files (Total size: %llu Byte) ---", multifileindex, iTotSize );

			}

		}
		else
		{

			unsigned __int64 	kll = 0UL;

			printf("MD2>> MD2 hashing... \n");

			/// initial
			memset( &mdContext, 0x00, sizeof(MD2_CTX) );
			memset( &md2_data, 0x00, sizeof(md2_data) );
			memset( &md2_output, 0x00, sizeof(md2_output) );

			MD2_Init(&mdContext);


			kll = 0UL;
			while ((nBytes = fread (md2_data, 1, sizeof(md2_data), inpfile)) != 0)
			{
				kll += nBytes;

				MD2_Update(&mdContext, md2_data, nBytes);

				// LOG_V("\bMD2 Hashing for (%s) -> read : %lld Bytes \r", infile_name, kll );
			}
			ClearScreen(); 

			MD2_Final( md2_output, &mdContext );
			MD2Print( md2_output );

			if( (2==iVerbosType || 3==iVerbosType) && (TRUE == verbose) )
			{
				printf ("  *%s*%s__(%llu) \r\n", str_hash, infile_name, infile_size );
				if(outfile) fprintf(outfile,"  *%s*%s__(%llu) \r\n", str_hash, infile_name, infile_size );
			}
			else
			{
				printf ("  *%s*%s  \r\n", str_hash, infile_name );
				if(outfile) fprintf(outfile,"  *%s*%s \r\n", str_hash, infile_name );
			}

			if(inpfile) { fclose(inpfile);	inpfile=NULL; }
			if(data_buf){ free(data_buf); data_buf=NULL; }
			printf("\nMD2>> Calculated MD2 Hash Value - OK" );

		}

		opt_ok = 0; /* 2014.07.16 : NOTE !!! clear!!!  */

	}
#endif /// MD2_CHECKSUM_ENCIPHER

#if SHA1_HASH_ENCIPHER
	else if(1 == isSHA1)
	{
		ShaBuffer SHA1output;
	    size_t nBytes;
	    sha1_context ctx;
		unsigned char sha1_buf[SHA_READ_BUFSIZ]; /// NERVER modified!!!!
		unsigned int iLenSub=0;
		struct	tm *pTime;  // 2020.07.15

		//unsigned char *sha1_buf;
		//sha1_buf = (unsigned char*)malloc( SHA_READ_BUFSIZ*sizeof(unsigned char) );
			
		pTime = current_time();

		if( (1==iVerbosType || 3==iVerbosType) && (TRUE == verbose) && outfile ) /// 2014.08.01
		{
			fprintf(outfile, "----------------------------------------------------------------------------------\r\n" );
			fprintf(outfile, "SHA1 Hash is created at %04d/%02d/%02d/%s %02d:%02d:%02d by %s \n", pTime->tm_year, pTime->tm_mon, pTime->tm_mday,  
													WeekTXT[pTime->tm_wday], pTime->tm_hour, pTime->tm_min, pTime->tm_sec, EmailText );
			fprintf(outfile, "----------------------------------------------------------------------------------\r\n" );
		}

		LOG_V("\n");
		
	#if SHA1_MULTI_INPUT_FILES
		if( ASTERISK_FOUND == isAsteMode ) /* For ah.exe --input *.* */
		{
			unsigned __int64 	kll = 0UL;


			printf("SHA1>> SHA1 hashing for input files* (%d) \n", argc);
			for(ii=0; ii<20*2; ii++) printf("-");  
			printf(" %d --\r\n", 20*2 );

			if(inpfile) { fclose(inpfile); inpfile=NULL; }
			if(data_buf){ free(data_buf); data_buf=NULL; }

			for(ii=iFirst; ii<=(argc-isAsteminus); ii++)
			{
				if( NULL != (inpfile = fopen( argv[ii], "rb")) ) 
				{
					/// initial ----
					memset( &ctx, 0x00, sizeof(sha1_context) );
					memset( sha1_buf, 0x00, sizeof(sha1_buf) );

					/// SHA1 Calculate ----
					sha1_starts(&ctx);
	
					kll = 0UL;
					while((nBytes = fread(sha1_buf, 1, sizeof(sha1_buf), inpfile)) > 0)
					{
						kll += nBytes;
						sha1_update(&ctx, sha1_buf, (int)nBytes);

						// LOG_V("\bSHA1 Hashing for (%s) -> read : %lld Bytes \r", argv[ii], kll );
					}
					ClearScreen(); 

					sha1_finish(&ctx, SHA1output);
	
					memset(&ctx, 0, sizeof(sha1_context));

					if( (2==iVerbosType || 3==iVerbosType) && (TRUE == verbose) )
					{
						printf("%s  *%s*%s__(%llu) \r\n", SHA1output, str_hash, argv[ii], kll );
						if(outfile) fprintf(outfile,"%s  *%s*%s__(%llu) \r\n", SHA1output, str_hash, argv[ii], kll );
					}
					else
					{
						printf("%s  *%s*%s \r\n", SHA1output, str_hash, argv[ii] );
						if(outfile) fprintf(outfile,"%s  *%s*%s \r\n", SHA1output, str_hash, argv[ii] );
					}

					if(inpfile) { fclose(inpfile); inpfile=NULL; }
					if(data_buf){ free(data_buf); data_buf=NULL; }

					//if(sha1_buf) free(sha1_buf);

					iTotSize += kll;
					multifileindex ++;
				}
			}

			printf("\nSHA1>> Number of Input files : %d files (Total size: %llu Byte) ---", multifileindex, iTotSize );

			AllFilesClosed();

		}
		else if( multifileindex > 0 )
		{
			unsigned __int64 	kll = 0UL;
	
	
			if(inpfile) { fclose(inpfile); inpfile=NULL; }
			if(data_buf){ free(data_buf); data_buf=NULL; }

		#if 1 // 
			/* Find first .c file in current directory */
			if( (retFiles = _findfirsti64( extfile_name, &iFiles )) == -1L )
			{
				printf("SHA1>>No file [%s] in current directory!!! [%s] \n", extfile_name, infile_name );
			}
			else
			{

				printf("SHA1>> SHA1 hashing for input files... \n");
				//printf("----------------------------------------------------------\r\n" );
				for(ii=0; ii<20*2; ii++) printf("-");  
				printf(" %d --\r\n", 20*2 );


				do {

					iLenSub = strlen(iFiles.name);

					if( iFiles.attrib & _A_SUBDIR )
					{

					}
					else
					{

						if( NULL == (inpfile = fopen( iFiles.name, "rb")) ) 
						{
							beep(700,100);
							printf("\n\nSHA1>>[++ERROR++] Can not open multi-input file (%s) \n", iFiles.name );

							AllFilesClosed();
							//if(sha1_buf) free(sha1_buf);

							exit(0);
							return 0;
						}
			
						/// initial ----
						memset( &ctx, 0x00, sizeof(sha1_context) );
						memset( sha1_buf, 0x00, sizeof(sha1_buf) );
		
		
						/// SHA1 Calculate ----
						sha1_starts(&ctx);
		
						while((nBytes = fread(sha1_buf, 1, sizeof(sha1_buf), inpfile)) > 0)
						{
							kll += nBytes;

							sha1_update(&ctx, sha1_buf, (int)nBytes);

							// LOG_V("\bSHA1 Hashing for (%s) -> read : %lld Bytes \r", iFiles.name, kll );
						}
						ClearScreen(); 

						sha1_finish(&ctx, SHA1output);
		
						memset(&ctx, 0, sizeof(sha1_context));

						if( (2==iVerbosType || 3==iVerbosType) && (TRUE == verbose) )
						{
							printf("%s  *%s*%s__(%llu) \r\n", SHA1output, str_hash, iFiles.name, iFiles.size  );
							if(outfile) fprintf(outfile,"%s  *%s*%s__(%llu) \r\n", SHA1output, str_hash, iFiles.name, iFiles.size  );
						}
						else
						{
							printf("%s  *%s*%s \r\n", SHA1output, str_hash, iFiles.name );
							if(outfile) fprintf(outfile,"%s  *%s*%s \r\n", SHA1output, str_hash, iFiles.name );
						}

						if(inpfile) { fclose(inpfile); inpfile=NULL; }
						if(data_buf){ free(data_buf); data_buf=NULL; }
						//if(sha1_buf) free(sha1_buf);

						iTotSize += iFiles.size;
						multifileindex ++;
					}

				} while( 0 == _findnexti64( retFiles, &iFiles ) );
				_findclose( retFiles );
				// if(sha1_buf) free(sha1_buf);

				multifileindex--;

				printf("\nSHA1>> Number of Input files : %d files (Total size: %llu Byte) ---", multifileindex, iTotSize );

			}
		#endif

		}
		else
	#endif /// MD5_MULTI_INPUT_FILES
		{
			unsigned __int64	kll = 0UL;

			printf("SHA1>> SHA1 hashing... \n");
			
			/// initial
			memset( &ctx, 0x00, sizeof(sha1_context) );
			memset( sha1_buf, 0x00, sizeof(sha1_buf) );

			/// SHA1 Calculate ----
		    sha1_starts(&ctx);

			kll = 0UL;
		    while((nBytes = fread(sha1_buf, 1, sizeof(sha1_buf), inpfile)) > 0)
		    {
				kll += nBytes;

		        sha1_update(&ctx, sha1_buf, (int)nBytes);

				// LOG_V("\bSHA1 Hashing for (%s) -> read : %lld Bytes \r", infile_name, kll );
		    }
			ClearScreen(); 

		    sha1_finish(&ctx, SHA1output);

		    memset(&ctx, 0, sizeof(sha1_context));

			if( (2==iVerbosType || 3==iVerbosType) && (TRUE == verbose) )
			{
				printf("%s  *%s*%s__(%llu) \r\n", SHA1output, str_hash, infile_name, infile_size );
				if(outfile) fprintf(outfile,"%s  *%s*%s__(%llu) \r\n", SHA1output, str_hash, infile_name, infile_size );
			}
			else
			{
				printf("%s  *%s*%s \r\n", SHA1output, str_hash, infile_name );
				if(outfile) fprintf(outfile,"%s  *%s*%s \r\n", SHA1output, str_hash, infile_name );
			}

			if(inpfile) { fclose(inpfile);	inpfile=NULL; }
			if(data_buf){ free(data_buf); data_buf=NULL; }
			//if(sha1_buf) free(sha1_buf);

			printf("\nSHA1>> Calculated SHA1 Hash Value - OK");

		}

		opt_ok = 0; /* 2014.07.16 : NOTE !!! clear!!!  */


	}
#endif /// SHA1_HASH_ENCIPHER


#if SHA2_256_384_512
	else if(1 == isSHA256)
	{
		unsigned __int64 	kll=0UL, ll=0UL;
		SHA256_CTX 		ctx256;
		char	sha256_buf[SHA2_BUFLEN];
		unsigned int iLenSub=0;
		struct	tm *pTime;  // 2020.07.15
	
		pTime = current_time();

		if( (1==iVerbosType || 3==iVerbosType) && (TRUE == verbose) && outfile ) /// 2014.08.01
		{
	#if (BYTE_ORDER==LITTLE_ENDIAN)
			fprintf(outfile, "----------------------------------------------------------------------------------\r\n" );
			fprintf(outfile, "SHA-256 Hash is created at %04d/%02d/%02d/%s %02d:%02d:%02d by %s \n", pTime->tm_year, pTime->tm_mon, pTime->tm_mday,  
													WeekTXT[pTime->tm_wday], pTime->tm_hour, pTime->tm_min, pTime->tm_sec, EmailText );
			fprintf(outfile, "----------------------------------------------------------------------------------\r\n" );
	#else
			fprintf(outfile, "----------------------------------------------------------------------------------\r\n" );
			fprintf(outfile, "SHA-256 Hash(BIG endian) is created at %04d/%02d/%02d/%s %02d:%02d:%02d by %s \n", pTime->tm_year, pTime->tm_mon, pTime->tm_mday,  
													WeekTXT[pTime->tm_wday], pTime->tm_hour, pTime->tm_min, pTime->tm_sec, EmailText );
			fprintf(outfile, "----------------------------------------------------------------------------------\r\n" );
	#endif
		}


		LOG_V("\n");

	#if SHA2_MULTI_INPUT_FILES
		if( ASTERISK_FOUND == isAsteMode )  /* For ah.exe --input *.* */
		{
			unsigned __int64 	kll = 0UL;

			printf("SHA2>> SHA256 hashing for input files* \n");
			for(ii=0; ii<SHA256_DIGEST_LENGTH*2; ii++) printf("-");  
			printf(" %d --\r\n", SHA256_DIGEST_LENGTH*2 );

			if(inpfile) { fclose(inpfile); inpfile=NULL; }
			if(data_buf){ free(data_buf); data_buf=NULL; }

			for(ii=iFirst; ii<=(argc-isAsteminus); ii++)
			{
				if( NULL != (inpfile = fopen( argv[ii], "rb")) ) 
				{
					/// initial ----
					memset( &ctx256, 0x00, sizeof(SHA256_CTX) );
					memset( sha256_buf, 0x00, sizeof(sha256_buf) );
					
					/// SHA2 Calculate ----
					SHA256_Init(&ctx256);
					
					kll = 0UL;
					while((ll = fread(sha256_buf, 1, sizeof(sha256_buf) /*SHA2_BUFLEN*/, inpfile)) > 0) 
					{
						kll += ll;
						SHA256_Update(&ctx256, (unsigned char*)sha256_buf, ll);

						// LOG_V("\bSHA-256 Hashing for (%s) -> read : %lld \r", argv[ii], kll );
					}
					ClearScreen(); 
					
					SHA256_End(&ctx256, sha256_buf);

					if( (2==iVerbosType || 3==iVerbosType) && (TRUE == verbose) )
					{
						printf("%s  *%s*%s__(%llu) \r\n", sha256_buf, str_hash, argv[ii], kll );
						if(outfile) fprintf(outfile,"%s  *%s*%s__(%llu) \r\n", sha256_buf, str_hash, argv[ii], kll );	
					}
					else
					{
						printf("%s  *%s*%s \r\n", sha256_buf, str_hash, argv[ii] );
						if(outfile) fprintf(outfile,"%s  *%s*%s \r\n", sha256_buf, str_hash, argv[ii] );	
					}
					
					if(inpfile) { fclose(inpfile); inpfile=NULL; }
					if(data_buf){ free(data_buf); data_buf=NULL; }
	
					iTotSize += kll;
					multifileindex ++;
				}
			}
			printf("\nSHA2>> SHA-256 Number of Input files : %d files (Total size: %llu Byte) ---", multifileindex, iTotSize );

			AllFilesClosed();

		}
		else if( multifileindex > 0 )
		{

			if(inpfile) { fclose(inpfile); inpfile=NULL; }
			if(data_buf){ free(data_buf); data_buf=NULL; }

	#if 1 // 
			/* Find first .c file in current directory */
			if( (retFiles = _findfirsti64( extfile_name, &iFiles )) == -1L )
			{
				printf("SHA2>>No file [%s] in current directory!!! [%s] \n", extfile_name, infile_name );
			}
			else
			{

				printf("SHA2>> SHA-256 hashing for input files... \n");
				for(ii=0; ii<SHA256_DIGEST_LENGTH*2; ii++) printf("-");  
				printf(" %d --\r\n", SHA256_DIGEST_LENGTH*2 );

				do {

					iLenSub = strlen(iFiles.name);

					if( iFiles.attrib & _A_SUBDIR )
					{

					}
					else
					{

						if( NULL == (inpfile = fopen( iFiles.name, "rb")) ) 
						{
							beep(700,100);
							printf("\n\n[++ERROR++] Can not open multi-input file (%s) in SHA2-256. \n", iFiles.name );

							AllFilesClosed();
	
							exit(0);
							return 0;
						}
			
						/// initial ----
						memset( &ctx256, 0x00, sizeof(SHA256_CTX) );
						memset( sha256_buf, 0x00, sizeof(sha256_buf) );
						
						/// SHA2 Calculate ----
						SHA256_Init(&ctx256);
						
						kll = 0UL;
						while((ll = fread(sha256_buf, 1, SHA2_BUFLEN, inpfile)) > 0) 
						{
							kll += ll;
							SHA256_Update(&ctx256, (unsigned char*)sha256_buf, ll);

							// LOG_V("\bSHA-256 Hashing for (%s) -> read : %lld \r", iFiles.name, kll );
						}
						ClearScreen(); 

						SHA256_End(&ctx256, sha256_buf);

						if( (2==iVerbosType || 3==iVerbosType) && (TRUE == verbose) )
						{
							printf("%s  *SHA256*%s__(%llu) \r\n", sha256_buf, iFiles.name, iFiles.size );
							if(outfile) fprintf(outfile,"%s  *SHA256*%s__(%llu) \r\n", sha256_buf, iFiles.name, iFiles.size );	
						}
						else
						{
							printf("%s  *SHA256*%s \r\n", sha256_buf, iFiles.name );
							if(outfile) fprintf(outfile,"%s  *SHA256*%s \r\n", sha256_buf, iFiles.name );	
						}
						
						if(inpfile) { fclose(inpfile); inpfile=NULL; }
						if(data_buf){ free(data_buf); data_buf=NULL; }

						iTotSize += iFiles.size;
						multifileindex ++;
					}

				} while( 0 == _findnexti64( retFiles, &iFiles ) );
				_findclose( retFiles );

				multifileindex--;

				if( iTotSize>>20 )
					printf("\nSHA2>> Number of Input files : %d files (Total size: %.3f MB) ---", multifileindex, (iTotSize/1024.0)/1024.0 );
				else if( iTotSize>>10 )
					printf("\nSHA2>> Number of Input files : %d files (Total size: %.3f kB) ---", multifileindex, (iTotSize/1024.0) );
				else
					printf("\nSHA2>> Number of Input files : %d files (Total size: %llu Byte) ---", multifileindex, iTotSize );

			}
	#endif

		}
		else
	#endif /// SHA2_MULTI_INPUT_FILES
		{
			printf("SHA2>> SHA-256 hashing... \n");


			/// initial
			memset( &ctx256, 0x00, sizeof(SHA256_CTX) );
			memset( sha256_buf, 0x00, sizeof(sha256_buf) );

			/// SHA2 Calculate ----
			SHA256_Init(&ctx256);

			kll = 0UL;
			while((ll = fread(sha256_buf, 1, SHA2_BUFLEN, inpfile)) > 0) 
			{
				kll += ll;
				SHA256_Update(&ctx256, (unsigned char*)sha256_buf, ll);

				// LOG_V("\bSHA-256 Hashing for (%s) -> read : %lld  \r", infile_name, kll );
			}
			ClearScreen(); 

			SHA256_End(&ctx256, sha256_buf);


			if( (2==iVerbosType || 3==iVerbosType) && (TRUE == verbose) )
			{
				printf("%s  *SHA256*%s__(%llu) \r\n", sha256_buf, infile_name, infile_size );
				if(outfile) fprintf(outfile,"%s  *SHA256*%s__(%llu) \r\n", sha256_buf, infile_name, infile_size );
			}
			else
			{
				printf("%s  *SHA256*%s \r\n", sha256_buf, infile_name );
				if(outfile) fprintf(outfile,"%s  *SHA256*%s \r\n", sha256_buf, infile_name );
			}
			
			if(inpfile) { fclose(inpfile); inpfile=NULL; }
			if(data_buf){ free(data_buf); data_buf=NULL; }
			printf("\nSHA2>> Calculated SHA-256 Hash Value - OK" );

		}	

		opt_ok = 0; /* 2014.07.16 : NOTE !!! clear!!!  */

	}
	else if(1 == isSHA384)
	{
		unsigned __int64 	kll=0, ll=0;
		SHA384_CTX 		ctx384;
		char	sha384_buf[SHA2_BUFLEN];
		unsigned int iLenSub=0;
		struct	tm *pTime;
	
		pTime = current_time();

		if( (1==iVerbosType || 3==iVerbosType) && (TRUE == verbose) && outfile) /// 2014.08.01
		{
	#if (BYTE_ORDER==LITTLE_ENDIAN)
			fprintf(outfile, "----------------------------------------------------------------------------------\r\n" );
			fprintf(outfile, "SHA-384 Hash is created at %04d/%02d/%02d/%s %02d:%02d:%02d by %s \n", pTime->tm_year, pTime->tm_mon, pTime->tm_mday,  
													WeekTXT[pTime->tm_wday], pTime->tm_hour, pTime->tm_min, pTime->tm_sec, EmailText );
			fprintf(outfile, "----------------------------------------------------------------------------------\r\n" );
	#else
			fprintf(outfile, "----------------------------------------------------------------------------------\r\n" );
			fprintf(outfile, "SHA-384 Hash(BIG endian) is created at %04d/%02d/%02d/%s %02d:%02d:%02d by %s \n", pTime->tm_year, pTime->tm_mon, pTime->tm_mday,  
													WeekTXT[pTime->tm_wday], pTime->tm_hour, pTime->tm_min, pTime->tm_sec, EmailText );
			fprintf(outfile, "----------------------------------------------------------------------------------\r\n" );
	#endif
		}	


		LOG_V("\n");

	#if SHA2_MULTI_INPUT_FILES
		if( ASTERISK_FOUND == isAsteMode ) /* For ah.exe --input *.* */
		{
			unsigned __int64 	kll = 0UL;


			printf("SHA2>> SHA-384 hashing for input files* \n");
			for(ii=0; ii<SHA384_DIGEST_LENGTH*2; ii++) printf("-");  
			printf(" %d --\r\n", SHA384_DIGEST_LENGTH*2 );

			if(inpfile) { fclose(inpfile); inpfile=NULL; }
			if(data_buf){ free(data_buf); data_buf=NULL; }

			for(ii=iFirst; ii<=(argc-isAsteminus); ii++)
			{
				if( NULL != (inpfile = fopen( argv[ii], "rb")) ) 
				{
					/// initial ----
					memset( &ctx384, 0x00, sizeof(SHA384_CTX) );
					memset( sha384_buf, 0x00, sizeof(sha384_buf) );
					
					/// SHA2 Calculate ----
					SHA384_Init(&ctx384);
					
					kll = 0UL;
					while((ll = fread(sha384_buf, 1, sizeof(sha384_buf) /*SHA2_BUFLEN*/, inpfile)) > 0) 
					{
						kll += ll;
						SHA384_Update(&ctx384, (unsigned char*)sha384_buf, ll);

						// LOG_V("\bSHA-384 Hashing for (%s) -> read : %lld Bytes \r", argv[ii], kll );
					}
					ClearScreen(); 

					SHA384_End(&ctx384, sha384_buf);

					if( (2==iVerbosType || 3==iVerbosType) && (TRUE == verbose) )
					{
						printf("%s  *SHA384*%s__(%llu) \r\n", sha384_buf, argv[ii], kll );
						if(outfile) fprintf(outfile,"%s  *SHA384*%s__(%llu) \r\n", sha384_buf, argv[ii], kll );
					}
					else
					{
						printf("%s  *SHA384*%s \r\n", sha384_buf, argv[ii] );
						if(outfile) fprintf(outfile,"%s  *SHA384*%s \r\n", sha384_buf, argv[ii] );
					}

					if(inpfile) { fclose(inpfile); inpfile=NULL; }
					if(data_buf){ free(data_buf); data_buf=NULL; }
		
					iTotSize += kll;
					multifileindex ++;
				}
			}

			printf("\nSHA2>> SHA-384 Number of Input files : %d files (Total size: %llu Byte) ---", multifileindex, iTotSize );

			AllFilesClosed();
	
		}
		else if( multifileindex > 0 )
		{

			if(inpfile) { fclose(inpfile); inpfile=NULL; }
			if(data_buf){ free(data_buf); data_buf=NULL; }


			/* Find first .c file in current directory */
			if( (retFiles = _findfirsti64( extfile_name, &iFiles )) == -1L )
			{
				printf("SHA2>>No file [%s] in current directory!!! [%s] in SHA-384\n", extfile_name, infile_name );
			}
			else
			{

				printf("SHA2>> SHA-384 hashing for input files... \n");
				//printf("-------------------------------------------------------------\r\n" );
				for(ii=0; ii<SHA384_DIGEST_LENGTH*2; ii++) printf("-");  
				printf(" %d --\r\n", SHA384_DIGEST_LENGTH*2 );

				do {

					iLenSub = strlen(iFiles.name);

					if( iFiles.attrib & _A_SUBDIR )
					{

					}
					else
					{

						if( NULL == (inpfile = fopen( iFiles.name, "rb")) ) 
						{
							beep(700,100);
							printf("\n\n[++ERROR++] Can not open multi-input file (%s) in SHA2-384. \n", iFiles.name );

							AllFilesClosed();
	
							exit(0);
							return 0;
						}
			
						/// initial ----
						memset( &ctx384, 0x00, sizeof(SHA384_CTX) );
						memset( sha384_buf, 0x00, sizeof(sha384_buf) );
						
						/// SHA2 Calculate ----
						SHA384_Init(&ctx384);
						
						kll = 0;
						while((ll = fread(sha384_buf, 1, SHA2_BUFLEN, inpfile)) > 0) 
						{
							kll += ll;
							SHA384_Update(&ctx384, (unsigned char*)sha384_buf, ll);

							// LOG_V("\bSHA-384 Hashing for (%s) -> read : %lld \r", iFiles.name, kll );
						}
						ClearScreen(); 

						SHA384_End(&ctx384, sha384_buf);

						if( (2==iVerbosType || 3==iVerbosType) && (TRUE == verbose) )
						{
							printf("%s  *SHA384*%s__(%llu) \r\n", sha384_buf, iFiles.name, iFiles.size );
							if(outfile) fprintf(outfile,"%s  *SHA384*%s__(%llu) \r\n", sha384_buf, iFiles.name, iFiles.size );
						}
						else
						{
							printf("%s  *SHA384*%s \r\n", sha384_buf, iFiles.name );
							if(outfile) fprintf(outfile,"%s  *SHA384*%s \r\n", sha384_buf, iFiles.name );
						}
						if(inpfile) { fclose(inpfile); inpfile=NULL; }
						if(data_buf){ free(data_buf); data_buf=NULL; }
			
						iTotSize += iFiles.size;
						multifileindex ++;
					}

				} while( 0 == _findnexti64( retFiles, &iFiles ) );
				_findclose( retFiles );

				multifileindex--;

				if( iTotSize>>20 )
					printf("\nSHA2>> Number of Input files : %d files (Total size: %.3f MB) ---", multifileindex, (iTotSize/1024.0)/1024.0 );
				else if( iTotSize>>10 )
					printf("\nSHA2>> Number of Input files : %d files (Total size: %.3f kB) ---", multifileindex, (iTotSize/1024.0) );
				else
					printf("\nSHA2>> Number of Input files : %d files (Total size: %llu Byte) ---", multifileindex, iTotSize );

			}

		}
		else
	#endif /// SHA2_MULTI_INPUT_FILES
		{
			printf("SHA2>> SHA-384 hashing... \n");


			/// initial ---
			memset( &ctx384, 0x00, sizeof(SHA384_CTX) );
			memset( sha384_buf, 0x00, sizeof(sha384_buf) );

			/// SHA2 Calculate ----
			SHA384_Init(&ctx384);

			kll = 0;
			while((ll = fread(sha384_buf, 1, SHA2_BUFLEN, inpfile)) > 0) 
			{
				kll += ll;
				SHA384_Update(&ctx384, (unsigned char*)sha384_buf, ll);

				// LOG_V("\bSHA-384 Hashing for (%s) -> read : %lld \r", infile_name, kll );
			}
			ClearScreen(); 

			SHA384_End(&ctx384, sha384_buf);

			if( (2==iVerbosType || 3==iVerbosType) && (TRUE == verbose) )
			{
				printf("%s  *SHA384*%s__(%llu) \r\n", sha384_buf, infile_name, infile_size );
				if(outfile) fprintf(outfile,"%s  *SHA384*%s__(%llu) \r\n", sha384_buf, infile_name, infile_size );
			}
			else
			{
				printf("%s  *SHA384*%s \r\n", sha384_buf, infile_name );
				if(outfile) fprintf(outfile,"%s  *SHA384*%s \r\n", sha384_buf, infile_name );
			}

			if(inpfile) { fclose(inpfile);	inpfile=NULL; }
			if(data_buf){ free(data_buf); data_buf=NULL; }
			printf("\nSHA2>> Calculated SHA-384 Hash Value - OK" );

		}	

		opt_ok = 0; /* 2014.07.16 : NOTE !!! clear!!!  */

	}
	else if(1 == isSHA512)
	{
		unsigned __int64 	kll=0, ll=0;
		SHA512_CTX 		ctx512;
		unsigned char	sha512_buf[SHA2_BUFLEN];
		unsigned int iLenSub=0;
		struct	tm *pTime;  // 2020.07.15
	
		pTime = current_time();

		if( (1==iVerbosType || 3==iVerbosType) && (TRUE == verbose) && outfile) /// 2014.08.01
		{
	#if (BYTE_ORDER==LITTLE_ENDIAN)
			fprintf(outfile, "----------------------------------------------------------------------------------\r\n" );
			fprintf(outfile, "SHA-512 Hash is created at %04d/%02d/%02d/%s %02d:%02d:%02d by %s \n", pTime->tm_year, pTime->tm_mon, pTime->tm_mday,  
													WeekTXT[pTime->tm_wday], pTime->tm_hour, pTime->tm_min, pTime->tm_sec, EmailText );
			fprintf(outfile, "----------------------------------------------------------------------------------\r\n" );
	#else
			fprintf(outfile, "----------------------------------------------------------------------------------\r\n" );
			fprintf(outfile, "SHA-512 Hash(BIG endian) is created at %04d/%02d/%02d/%s %02d:%02d:%02d by %s \n", pTime->tm_year, pTime->tm_mon, pTime->tm_mday,  
													WeekTXT[pTime->tm_wday], pTime->tm_hour, pTime->tm_min, pTime->tm_sec, EmailText );
			fprintf(outfile, "----------------------------------------------------------------------------------\r\n" );
	#endif
		}


		LOG_V("\n");

	#if SHA2_MULTI_INPUT_FILES
		if( ASTERISK_FOUND == isAsteMode )  /* For ah.exe --input *.* */
		{
			unsigned __int64 	kll = 0UL;


			printf("SHA2>> SHA-512 hashing for input files* \n");
			for(ii=0; ii<SHA512_DIGEST_LENGTH*2; ii++) printf("-");  
			printf(" %d --\r\n", SHA512_DIGEST_LENGTH*2 );

			if(inpfile) { fclose(inpfile); inpfile=NULL; }
			if(data_buf){ free(data_buf); data_buf=NULL; }

			for(ii=iFirst; ii<=(argc-isAsteminus); ii++)
			{
				if( NULL != (inpfile = fopen( argv[ii], "rb")) ) 
				{
					/// initial ----
					memset( &ctx512, 0x00, sizeof(SHA512_CTX) );
					memset( sha512_buf, 0x00, sizeof(sha512_buf) );
					
					/// SHA2 Calculate ----
					SHA512_Init(&ctx512);
					
					kll = 0;
					while((ll = fread(sha512_buf, 1, sizeof(sha512_buf) /*SHA2_BUFLEN*/, inpfile)) > 0) 
					{
						kll += ll;
						SHA512_Update(&ctx512, (unsigned char*)sha512_buf, ll);

						// LOG_V("\bSHA-512 Hashing for (%s) -> read : %lld Bytes \r", argv[ii], kll );
					}
					ClearScreen(); 

					SHA512_End(&ctx512, sha512_buf);

					if( (2==iVerbosType || 3==iVerbosType) && (TRUE == verbose) )
					{
						printf("%s  *SHA512*%s__(%llu) \r\n", sha512_buf, argv[ii], kll );
						if(outfile) fprintf(outfile,"%s  *SHA512*%s__(%llu) \r\n", sha512_buf, argv[ii], kll );
					}
					else
					{
						printf("%s  *SHA512*%s \r\n", sha512_buf, argv[ii] );
						if(outfile) fprintf(outfile,"%s  *SHA512*%s \r\n", sha512_buf, argv[ii] );
					}
					
					if(inpfile) { fclose(inpfile); inpfile=NULL; }
					if(data_buf){ free(data_buf); data_buf=NULL; }

					iTotSize += kll;
					multifileindex ++;
				}
			}

			printf("\nSHA2>> SHA-512 Number of Input files : %d files (Total size: %llu Byte) ---", multifileindex, iTotSize );

			AllFilesClosed();

		}
		else if( multifileindex > 0 )
		{
	
			if(inpfile) { fclose(inpfile); inpfile=NULL; }
			if(data_buf){ free(data_buf); data_buf=NULL; }

			/* Find first .c file in current directory */
			if( (retFiles = _findfirsti64( extfile_name, &iFiles )) == -1L )
			{
				printf("SHA2>>No file [%s] in current directory!!! [%s] in SHA2-512 \n", extfile_name, infile_name );
			}
			else
			{

				printf("SHA2>> SHA-512 hashing for input files... \n");
				//printf("-------------------------------------------------------------\r\n" );
				for(ii=0; ii<SHA512_DIGEST_LENGTH*2; ii++) printf("-");  
				printf(" %d --\r\n", SHA512_DIGEST_LENGTH*2 );
				
				do {

					iLenSub = strlen(iFiles.name);

					if( iFiles.attrib & _A_SUBDIR )
					{

					}
					else
					{

						if( NULL == (inpfile = fopen( iFiles.name, "rb")) ) 
						{
							beep(700,100);
							printf("\n\nSHA2>>[++ERROR++] Can not open multi-input file (%s) in SHA2-512. \n", iFiles.name );

							AllFilesClosed();
	
							exit(0);
							return 0;
						}
			
						/// initial ----
						memset( &ctx512, 0x00, sizeof(SHA512_CTX) );
						memset( sha512_buf, 0x00, sizeof(sha512_buf) );
						
						/// SHA2 Calculate ----
						SHA512_Init(&ctx512);
						
						kll = 0;
						while((ll = fread(sha512_buf, 1, SHA2_BUFLEN, inpfile)) > 0) 
						{
							kll += ll;
							SHA512_Update(&ctx512, (unsigned char*)sha512_buf, ll);

							// LOG_V("\bSHA-512 Hashing for (%s) -> read : %lld \r", iFiles.name, kll );
						}
						ClearScreen(); 

						SHA512_End(&ctx512, sha512_buf);

						if( (2==iVerbosType || 3==iVerbosType) && (TRUE == verbose) )
						{
							printf("%s  *SHA512*%s__(%llu) \r\n", sha512_buf, iFiles.name, iFiles.size );
							if(outfile) fprintf(outfile,"%s  *SHA512*%s__(%llu) \r\n", sha512_buf, iFiles.name, iFiles.size );
						}
						else
						{
							printf("%s  *SHA512*%s \r\n", sha512_buf, iFiles.name );
							if(outfile) fprintf(outfile,"%s  *SHA512*%s \r\n", sha512_buf, iFiles.name );
						}
						
						if(inpfile) { fclose(inpfile); inpfile=NULL; }
						if(data_buf){ free(data_buf); data_buf=NULL; }


						iTotSize += iFiles.size;
						multifileindex ++;
					}

				} while( 0 == _findnexti64( retFiles, &iFiles ) );
				_findclose( retFiles );

				multifileindex--;

				if( iTotSize>>20 )
					printf("\nSHA2>> Number of Input files : %d files (Total size: %.3f MB) ---", multifileindex, (iTotSize/1024.0)/1024.0 );
				else if( iTotSize>>10 )
					printf("\nSHA2>> Number of Input files : %d files (Total size: %.3f kB) ---", multifileindex, (iTotSize/1024.0) );
				else
					printf("\nSHA2>> Number of Input files : %d files (Total size: %llu Byte) ---", multifileindex, iTotSize );

			}

		}
		else
	#endif /// SHA2_MULTI_INPUT_FILES
		{
			printf("SHA2>> SHA-512 hashing... \n");


			/// initial
			memset( &ctx512, 0x00, sizeof(SHA512_CTX) );
			memset( sha512_buf, 0x00, sizeof(sha512_buf) );


			/// SHA2 Calculate ----
			SHA512_Init(&ctx512);

			kll = 0;
			while((ll = fread(sha512_buf, 1, SHA2_BUFLEN, inpfile)) > 0) 
			{
				kll += ll;
				SHA512_Update(&ctx512, (unsigned char*)sha512_buf, ll);

				// LOG_V("\bSHA-512 Hashing for (%s) -> read : %lld \r", infile_name,  kll  );
			}
			ClearScreen(); 

			SHA512_End(&ctx512, sha512_buf);

			if( (2==iVerbosType || 3==iVerbosType) && (TRUE == verbose) )
			{
				printf("%s  *SHA512*%s__(%llu) \r\n", sha512_buf, infile_name, infile_size );
				if(outfile) fprintf(outfile,"%s  *SHA512*%s__(%llu) \r\n", sha512_buf, infile_name, infile_size );
			}
			else
			{
				printf("%s  *SHA512*%s \r\n", sha512_buf, infile_name  );
				if(outfile) fprintf(outfile,"%s  *SHA512*%s \r\n", sha512_buf, infile_name );
			}

			if(inpfile) { fclose(inpfile);	inpfile=NULL; } //			
			if(data_buf){ free(data_buf); data_buf=NULL; }
			printf("\nSHA2>> Calculated SHA-512 Hash Value - OK" );

		}	

		opt_ok = 0; /* 2014.07.16 : NOTE !!! clear!!!  */

	}
#endif /// SHA2_256_384_512

#if SHA2_224_CHECKSUM
	else if(1 == isSHA224)
	{
		unsigned __int64 	kll=0, ll=0;
		sha224_ctx      ctx224;		
		unsigned char	sha224_buf[SHA2_BUFLEN]; // 1024*10];
		unsigned int iLenSub=0;
		struct	tm *pTime;
	
		pTime = current_time();

		if( (1==iVerbosType || 3==iVerbosType) && (TRUE == verbose) && outfile ) /// 2014.08.01
		{
			fprintf(outfile, "----------------------------------------------------------------------------------\r\n" );
			fprintf(outfile, "SHA-224 Hash is created at %04d/%02d/%02d/%s %02d:%02d:%02d by %s \n", pTime->tm_year, pTime->tm_mon, pTime->tm_mday,  
													WeekTXT[pTime->tm_wday], pTime->tm_hour, pTime->tm_min, pTime->tm_sec, EmailText );
			fprintf(outfile, "----------------------------------------------------------------------------------\r\n" );
		}


		LOG_V("\n");

	#if SHA2_MULTI_INPUT_FILES
		if( ASTERISK_FOUND == isAsteMode )  /* For ah.exe --input *.* */
		{
			unsigned __int64 	kll = 0UL;


			printf("SHA2>> SHA-224 hashing for input files* \n");
			for(ii=0; ii<SHA224_DIGEST_SIZE*2; ii++) printf("-");  
			printf(" %d --\r\n", SHA224_DIGEST_SIZE*2 );

			if(inpfile) { fclose(inpfile); inpfile=NULL; }
			if(data_buf){ free(data_buf); data_buf=NULL; }

			for(ii=iFirst; ii<=(argc-isAsteminus); ii++)
			{
				if( NULL != (inpfile = fopen( argv[ii], "rb")) ) 
				{
					/// initial ----
					memset( &ctx224, 0x00, sizeof(sha224_ctx) );
					memset( sha224_buf, 0x00, sizeof(sha224_buf) );
					
					/// SHA2 Calculate ----
					sha224_init(&ctx224);
							
					kll = 0UL;
					while((ll = fread(sha224_buf, 1, sizeof(sha224_buf), inpfile)) > 0) 
					{
						kll += ll;
						sha224_update(&ctx224, sha224_buf, ll);

						// LOG_V("\bSHA-224 Hashing for (%s) -> read : %lld Bytes \r", argv[ii], kll );
					}
					ClearScreen(); 

					sha224_final(&ctx224, sha224_buf);
					sha224Print (sha224_buf);

					if( (2==iVerbosType || 3==iVerbosType) && (TRUE == verbose) )
					{
						printf("  *SHA224*%s__(%llu) \r\n", argv[ii], kll );
						if(outfile) fprintf(outfile,"  *SHA224*%s__(%llu) \r\n", argv[ii], kll );
					}
					else
					{
						printf("  *SHA224*%s \r\n", argv[ii] );
						if(outfile) fprintf(outfile,"  *SHA224*%s \r\n", argv[ii] );
					}

					if(inpfile) { fclose(inpfile); inpfile=NULL; }
					if(data_buf){ free(data_buf); data_buf=NULL; }
	
					iTotSize += kll;
					multifileindex ++;
				}
			}
			printf("\nSHA2>> SHA-224 Number of Input files : %d files (Total size: %llu Byte) ---", multifileindex, iTotSize );

			AllFilesClosed();

		}
		else if( multifileindex > 0 )
		{
	
			if(inpfile) { fclose(inpfile); inpfile=NULL; }
			if(data_buf){ free(data_buf); data_buf=NULL; }

		#if 1 // 
			/* Find first .c file in current directory */
			if( (retFiles = _findfirsti64( extfile_name, &iFiles )) == -1L )
			{
				printf("SHA2>>No file [%s] in current directory!!! [%s] in SHA-224 \n", extfile_name, infile_name );
			}
			else
			{
				printf("SHA2>> SHA-224 hashing for input files... \n");
				//printf("-------------------------------------------------------------\r\n" );
				for(ii=0; ii<SHA224_DIGEST_SIZE*2; ii++) printf("-");  
				printf(" %d --\r\n", SHA224_DIGEST_SIZE*2 );
				
				do {
					iLenSub = strlen(iFiles.name);

					if( iFiles.attrib & _A_SUBDIR )
					{

					}
					else
					{
						if( NULL == (inpfile = fopen( iFiles.name, "rb")) ) 
						{
							beep(700,100);
							printf("\n\nSHA2>> [++ERROR++] Can not open multi-input file (%s) in SHA2-224. \n", iFiles.name );

							AllFilesClosed();
	
							exit(0);
							return 0;
						}
			
						/// initial ----
						memset( &ctx224, 0x00, sizeof(sha224_ctx) );
						memset( sha224_buf, 0x00, sizeof(sha224_buf) );
						
						/// SHA2 Calculate ----
						sha224_init(&ctx224);
								
						kll = 0;
						while((ll = fread(sha224_buf, 1, sizeof(sha224_buf), inpfile)) > 0) 
						{
							kll += ll;
							sha224_update(&ctx224, sha224_buf, ll);

							// LOG_V("\bSHA-224 Hashing for (%s) -> read : %lld \r", iFiles.name, kll  );
						}
						ClearScreen(); 

						sha224_final(&ctx224, sha224_buf);
						sha224Print (sha224_buf);


						if( (2==iVerbosType || 3==iVerbosType) && (TRUE == verbose) )
						{
							printf("  *SHA224*%s__(%llu) \r\n", iFiles.name, iFiles.size );
							if(outfile) fprintf(outfile,"  *SHA224*%s__(%llu) \r\n", iFiles.name, iFiles.size );
						}
						else
						{
							printf("  *SHA224*%s \r\n", iFiles.name );
							if(outfile) fprintf(outfile,"  *SHA224*%s \r\n", iFiles.name );
						}

						if(inpfile) { fclose(inpfile); inpfile=NULL; }
						if(data_buf){ free(data_buf); data_buf=NULL; }
		
						iTotSize += iFiles.size;
						multifileindex ++;
					}

				} while( 0 == _findnexti64( retFiles, &iFiles ) );
				_findclose( retFiles );

				multifileindex--;

				if( iTotSize>>20 )
					printf("\nSHA2>> Number of Input files : %d files (Total size: %.3f MB) ---", multifileindex, (iTotSize/1024.0)/1024.0 );
				else if( iTotSize>>10 )
					printf("\nSHA2>> Number of Input files : %d files (Total size: %.3f kB) ---", multifileindex, (iTotSize/1024.0) );
				else
					printf("\nSHA2>> Number of Input files : %d files (Total size: %llu Byte) ---", multifileindex, iTotSize );

			}
		#endif	

		}
		else
	#endif /// SHA2_MULTI_INPUT_FILES
		{
			printf("SHA2>> SHA-224 hashing... \n");


			/// initial ----
			memset( &ctx224, 0x00, sizeof(sha224_ctx) );
			memset( sha224_buf, 0x00, sizeof(sha224_buf) );


			/// SHA2 Calculate ----
			sha224_init(&ctx224);
					
			kll = 0;
			while((ll = fread(sha224_buf, 1, sizeof(sha224_buf), inpfile)) > 0) 
			{
				kll += ll;
				sha224_update(&ctx224, sha224_buf, ll);

				// LOG_V("\bSHA-224 Hashing for (%s) -> read : %lld \r", infile_name, kll );
			}
			ClearScreen(); 

			sha224_final(&ctx224, sha224_buf);
			sha224Print (sha224_buf);


			if( (2==iVerbosType || 3==iVerbosType) && (TRUE == verbose) )
			{
				printf("  *SHA224*%s__(%llu) \r\n", infile_name, infile_size );
				if(outfile) fprintf(outfile,"  *SHA224*%s__(%llu) \r\n", infile_name, infile_size );
			}			
			else
			{
				printf("  *SHA224*%s \r\n", infile_name );
				if(outfile) fprintf(outfile,"  *SHA224*%s \r\n", infile_name );
			}
			
			if(inpfile) { fclose(inpfile); inpfile=NULL; }
			if(data_buf){ free(data_buf); data_buf=NULL; }
			printf("\nSHA2>> Calculated SHA-224 Hash Value - OK" );

		}	
		
		opt_ok = 0; /* 2014.07.16 : NOTE !!! clear!!!  */

	}		
#endif


#if MD6_CHECKSUM_ENCIPHER
	else if(1 == isMD6)
	{
		unsigned long long nBytes = 0UL, kll = 0UL;
		char md6_data[1024*10]; // MD_HASH_BUFSIZ
		//double elapsed_time = end_time - start_time;
		//unsigned long long elapsed_ticks = end_ticks - start_ticks;
		unsigned int iLenSub=0;
		struct	tm *pTime;

		pTime = current_time();
		
		if( (1==iVerbosType || 3==iVerbosType) && (TRUE == verbose) && outfile ) /// 2014.08.01
		{
			fprintf(outfile, "----------------------------------------------------------------------------------\r\n" );
			fprintf(outfile, "MD6 Hash is created at %04d/%02d/%02d/%s %02d:%02d:%02d by %s \n", pTime->tm_year, pTime->tm_mon, pTime->tm_mday,  
													WeekTXT[pTime->tm_wday], pTime->tm_hour, pTime->tm_min, pTime->tm_sec, EmailText );
			fprintf(outfile, "----------------------------------------------------------------------------------\r\n" );
		}


		/* -------------------------------------------------- */
		/* ------ MD6 : set default md6 parameter settings ------ */
		md6_dgtLen = 256;           /* digest length */
		md6_keylen = 0;             /* key length in bytes (at most 64) */
		md6_modPar = 64;            /* mode parameter */
		md6_roundN = md6_default_r(md6_dgtLen,md6_keylen);  /* number of rounds */
		md6_use_default_r = 1;     /* 1 if r should be set to the default, 0 if r is explicitly provided */
		/* -------------------------------------------------- */

		LOG_V("\n");

		if( ASTERISK_FOUND == isAsteMode ) /* For ah.exe --input *.* */
		{
			unsigned __int64 	kll = 0UL;

			printf("MD6>> MD6 hashing for input files* \n");
			for(ii=0; ii<32*2; ii++) printf("-");  
			printf(" %d --\r\n", 32*2 );

			if(inpfile) { fclose(inpfile); inpfile=NULL; }
			if(data_buf){ free(data_buf); data_buf=NULL; }

			for(ii=iFirst; ii<=(argc-isAsteminus); ii++)
			{
				if( NULL != (inpfile = fopen( argv[ii], "rb")) ) 
				{
					/// initial ----
					nBytes = 0UL;
					memset( md6_data, 0x00, sizeof(md6_data) );
	
					/// MD6 Calculate ----
					hash_init();
					
					kll = 0UL;
					while((nBytes = fread(md6_data, 1, sizeof(md6_data), inpfile)) > 0) 
					{
						kll += nBytes;
						hash_update(md6_data,nBytes*8);

						// LOG_V("\bMD6 Hashing for (%s) -> read : %lld Bytes \r", argv[ii], kll );
					}
					ClearScreen(); 

					hash_final();

					if( (2==iVerbosType || 3==iVerbosType) && (TRUE == verbose) )
					{
						printf("%s  *MD6*%s__(%llu) \r\n", md6_st.hexhashval, argv[ii], kll );
						if(outfile) fprintf(outfile,"%s  *MD6*%s__(%llu) \r\n", md6_st.hexhashval, argv[ii], kll );
					}		
					else
					{
						printf("%s  *MD6*%s \r\n", md6_st.hexhashval, argv[ii] );
						if(outfile) fprintf(outfile,"%s  *MD6*%s \r\n", md6_st.hexhashval, argv[ii] );
					}
					
					if(inpfile) { fclose(inpfile); inpfile=NULL; }
					if(data_buf){ free(data_buf); data_buf=NULL; }

					iTotSize += kll;
					multifileindex ++;
				}
			}
			printf("\nMD6>> Number of Input files : %d files (Total size: %llu Byte) ---", multifileindex, iTotSize );

			AllFilesClosed();

		}
		else if( multifileindex > 0 )
		{


			if(inpfile) { fclose(inpfile); inpfile=NULL; }
			if(data_buf){ free(data_buf); data_buf=NULL; }

		#if 1 // 
			/* Find first .c file in current directory */
			if( (retFiles = _findfirsti64( extfile_name, &iFiles )) == -1L )
			{
				printf("MD6>>No file [%s] in current directory!!! [%s] \n", extfile_name, infile_name );
			}
			else
			{

				printf("MD6>> MD6 hashing for input files... \n");
				//printf("--------------------------------------------------------\r\n" );
				for(ii=0; ii<32*2; ii++) printf("-");  
				printf(" %d --\r\n", 32*2 );

				do {

					iLenSub = strlen(iFiles.name);

					if( iFiles.attrib & _A_SUBDIR )
					{
					}
					else
					{

 						if( NULL == (inpfile = fopen( iFiles.name, "rb")) ) 
						{
							beep(700,100);
							printf("\n\n[++ERROR++] Can not open multi-input file (%s) in SHA2-512. \n", iFiles.name );

							AllFilesClosed();

							exit(0);
							return 0;
						}
		
						/// initial ----
						nBytes = 0UL;
						memset( md6_data, 0x00, sizeof(md6_data) );
		
		
						/// MD6 Calculate ----
						hash_init();
						
						kll = 0UL;
						while((nBytes = fread(md6_data, 1, sizeof(md6_data), inpfile)) > 0) 
						{
							kll += nBytes;
							hash_update(md6_data,nBytes*8);

							//LOG_V("\bMD6 Hashing for (%s) -> read : %lld Bytes \r", iFiles.name, kll );
						}
						//ClearScreen(); 

						hash_final();
		
				#if 0
						elapsed_time = end_time - start_time;
						printf("-- Elapsed time = %.3f seconds\n",elapsed_time);
						elapsed_ticks = end_ticks - start_ticks;
						printf("-- Total clock ticks = %lld\n",(long long int)elapsed_ticks);
				#endif
					
						/// encode(encfilename,argv[i]);
						/// print_hash(infile_name);


						if( (2==iVerbosType || 3==iVerbosType) && (TRUE == verbose) )
						{
							printf("%s  *MD6*%s__(%llu) \r\n", md6_st.hexhashval, iFiles.name, iFiles.size );
							if(outfile) fprintf(outfile,"%s  *MD6*%s__(%llu) \r\n", md6_st.hexhashval, iFiles.name, iFiles.size );
						}		
						else
						{
							printf("%s  *MD6*%s \r\n", md6_st.hexhashval, iFiles.name );
							if(outfile) fprintf(outfile,"%s  *MD6*%s \r\n", md6_st.hexhashval, iFiles.name );
						}
						
						if(inpfile) { fclose(inpfile); inpfile=NULL; }
						if(data_buf){ free(data_buf); data_buf=NULL; }
	
						iTotSize += iFiles.size;
						multifileindex ++;
					}

				} while( 0 == _findnexti64( retFiles, &iFiles ) );
				_findclose( retFiles );

				multifileindex--;

				if( iTotSize>>20 )
					printf("\nMD6>> Number of Input files : %d files (Total size: %.3f MB) ---", multifileindex, (iTotSize/1024.0)/1024.0 );
				else if( iTotSize>>10 )
					printf("\nMD6>> Number of Input files : %d files (Total size: %.3f kB) ---", multifileindex, (iTotSize/1024.0) );
				else
					printf("\nMD6>> Number of Input files : %d files (Total size: %llu Byte) ---", multifileindex, iTotSize );

			}
		#endif

		}
		else
		{
			printf("MD6>> MD6 hashing... \n");

			/// initial
			nBytes = 0UL;
			memset( md6_data, 0x00, sizeof(md6_data) );


			/// MD6 Calculate ----
			hash_init();

			kll = 0UL;
			while((nBytes = fread(md6_data, 1, sizeof(md6_data), inpfile)) > 0) 
			{
				kll += nBytes;
				hash_update(md6_data,nBytes*8);

				// LOG_V("\bMD6 Hashing for (%s) -> read : %lld Bytes \r", infile_name, kll );
			}
			ClearScreen(); 

			hash_final();


			if( (2==iVerbosType || 3==iVerbosType) && (TRUE == verbose) )
			{
				printf("%s	*MD6*%s__(%llu)  \r\n", md6_st.hexhashval, infile_name, infile_size  );
				if(outfile) fprintf(outfile,"%s  *MD6*%s__(%llu) \r\n", md6_st.hexhashval, infile_name, infile_size );
			}
			else
			{
				printf("%s  *MD6*%s \r\n", md6_st.hexhashval, infile_name  );
				if(outfile) fprintf(outfile,"%s  *MD6*%s \r\n", md6_st.hexhashval, infile_name );
			}

			if(inpfile) { fclose(inpfile); inpfile=NULL; }
			if(data_buf){ free(data_buf); data_buf=NULL; }

			printf("\nMD6>> Calculated MD6 Hash Value - OK" );

		}	

		opt_ok = 0; /* 2014.07.16 : NOTE !!! clear!!!  */

	}
#endif

#if SHA3_KECCAK_224_256_384_512 // SHA3, 2017.08.22
	else if(1 == isSHA3_KECCAK_224)
	{
		unsigned char sha3Buf[SHA3_BUFLEN] = {0,};
		char	sha3digestTxt[SHA3_OUTPUT_SIZ] = {0,};
		unsigned char sha3out[SHA3_OUTPUT_SIZ] = { 0, };
		
		unsigned __int64 	kll=0;
		size_t		ll=0;
		int 		ret;
		unsigned int iLenSub=0;
	#if 1
		struct	tm *pTime;

		pTime = current_time();

		if( (1==iVerbosType || 3==iVerbosType) && (TRUE == verbose) && outfile) /// 2014.08.01
		{
			fprintf(outfile, "----------------------------------------------------------------------------------\r\n" );
			fprintf(outfile, "SHA3-224 Hash is created at %04d/%02d/%02d/%s %02d:%02d:%02d by %s \n", pTime->tm_year, pTime->tm_mon, pTime->tm_mday,  
													WeekTXT[pTime->tm_wday], pTime->tm_hour, pTime->tm_min, pTime->tm_sec, EmailText );
			fprintf(outfile, "----------------------------------------------------------------------------------\r\n" );
		}
	#endif

		LOG_V("\n");

		if( ASTERISK_FOUND == isAsteMode ) /* For ah.exe --input *.* */
		{
			int iIdx=0;
			unsigned __int64 	kll = 0UL;

			printf("HA3-KECCAK>> SHA3-224 hashing for input files* \n");
			for(ii=0; ii<SHA3_OUT_224*2; ii++) printf("-");  
			printf(" %d --\r\n", SHA3_OUT_224*2 );

			if(inpfile) { fclose(inpfile); inpfile=NULL; }
			if(data_buf){ free(data_buf); data_buf=NULL; }

			for(ii=iFirst; ii<=(argc-isAsteminus); ii++)
			{
				if( NULL != (inpfile = fopen( argv[ii], "rb")) ) 
				{
					memset( sha3Buf, 0x00, sizeof(sha3Buf) );
					memset( sha3out, 0x00, sizeof(sha3out) );
					memset( sha3digestTxt, 0x00, sizeof(sha3digestTxt) );
					
					// Initialize the SHA3-224 context
					sha3_init(SHA3_224_HASH_BIT, SHA3_SHAKE_NONE);				
	
					kll = 0UL;
					while((ll = fread(sha3Buf, 1, sizeof(sha3Buf), inpfile)) > 0) 
					{
						kll += ll;
						sha3_update(sha3Buf, ll);
						
						//Digest the message
						// LOG_V("\bSHA3-224 Hashing for (%s) -> read : %lld Bytes \r", argv[ii], kll );
					}
					ClearScreen(); 

					ret = sha3_final(sha3out, SHA3_OUT_224);
	
					for (iIdx = 0; iIdx < SHA3_OUT_224; iIdx++)
					{
						sprintf(&sha3digestTxt[iIdx*2], "%02x", sha3out[iIdx]);
					}

					if( (2==iVerbosType || 3==iVerbosType) && (TRUE == verbose) )
					{
						printf("%s  *SHA3-224*%s__(%llu) \r\n", sha3digestTxt, argv[ii], kll );
						if(outfile) fprintf(outfile, "%s  *SHA3-224*%s__(%llu) \r\n", sha3digestTxt, argv[ii], kll );
					}
					else
					{
						printf("%s  *SHA3-224*%s \r\n", sha3digestTxt, argv[ii] );
						if(outfile) fprintf(outfile, "%s  *SHA3-224*%s \r\n", sha3digestTxt, argv[ii] );
					}
					
					if(inpfile) { fclose(inpfile); inpfile=NULL; }
					if(data_buf){ free(data_buf); data_buf=NULL; }

					iTotSize += kll;
					multifileindex ++;
				}
			}
			
			printf("\nHA3-KECCAK>> SHA3-224 Number of Input files : %d files (Total size: %llu Byte) ---", multifileindex, iTotSize );

			AllFilesClosed();

		}
		else if( multifileindex > 0 )
		{
		

			if(inpfile) { fclose(inpfile); inpfile=NULL; }
			if(data_buf){ free(data_buf); data_buf=NULL; }

		#if 1 // 
			/* Find first .c file in current directory */
			if( (retFiles = _findfirsti64( extfile_name, &iFiles )) == -1L )
			{
				printf("SHA3-KECCAK>>No file [%s] in current directory!!! [%s] in SHA3-224 \n", extfile_name, infile_name );
			}
			else
			{

				printf("SHA3-KECCAK>> SHA3-224 hashing for input files... \n");
				//printf("-------------------------------------------------------------\r\n" );
				for(ii=0; ii<SHA3_OUT_224*2; ii++) printf("-");  
				printf(" %d --\r\n", SHA3_OUT_224*2 );

				do {
					iLenSub = strlen(iFiles.name);

					if( iFiles.attrib & _A_SUBDIR )
					{

					}
					else
					{
						if( NULL == (inpfile = fopen( iFiles.name, "rb")) ) 
						{
							beep(700,100);
							printf("\n\n[++ERROR++] Can not open multi-input file (%s) in SHA3-224. \n", iFiles.name );

							AllFilesClosed();
	
							exit(0);
							return 0;
						}

						memset( sha3Buf, 0x00, sizeof(sha3Buf) );
						memset( sha3out, 0x00, sizeof(sha3out) );
						memset( sha3digestTxt, 0x00, sizeof(sha3digestTxt) );
						
						// Initialize the SHA3-224 context
						sha3_init(SHA3_224_HASH_BIT, SHA3_SHAKE_NONE);				
		
						kll = 0;
						while((ll = fread(sha3Buf, 1, sizeof(sha3Buf), inpfile)) > 0) 
						{
							kll += ll;
		
							sha3_update(sha3Buf, ll);
							
							//Digest the message
							// LOG_V("\bSHA3-224 Hashing for (%s) -> read : %lld \r", iFiles.name,  kll );
						}
						ClearScreen(); 

						ret = sha3_final(sha3out, SHA3_OUT_224);
		
						for (ii = 0; ii < SHA3_OUT_224; ii++)
						{
							//printf("%02x", sha3out[ii]);
							sprintf(&sha3digestTxt[ii*2], "%02x", sha3out[ii]);
						}


						if( (2==iVerbosType || 3==iVerbosType) && (TRUE == verbose) )
						{
							printf("%s  *SHA3-224*%s__(%llu) \r\n", sha3digestTxt, iFiles.name, iFiles.size );
							if(outfile) fprintf(outfile, "%s  *SHA3-224*%s__(%llu) \r\n", sha3digestTxt, iFiles.name, iFiles.size );
						}
						else
						{
							printf("%s  *SHA3-224*%s \r\n", sha3digestTxt, iFiles.name  );
							if(outfile) fprintf(outfile, "%s  *SHA3-224*%s \r\n", sha3digestTxt, iFiles.name  );
						}
						
						if(inpfile) { fclose(inpfile); inpfile=NULL; }
						if(data_buf){ free(data_buf); data_buf=NULL; }

						iTotSize += iFiles.size;
						multifileindex ++;
					}

				} while( 0 == _findnexti64( retFiles, &iFiles ) );
				_findclose( retFiles );

				multifileindex--;

				if( iTotSize>>20 )
					printf("\nSHA3-KECCAK>> Number of Input files : %d files (Total size: %.3f MB) ---", multifileindex, (iTotSize/1024.0)/1024.0 );
				else if( iTotSize>>10 )
					printf("\nSHA3-KECCAK>> Number of Input files : %d files (Total size: %.3f kB) ---", multifileindex, (iTotSize/1024.0) );
				else
					printf("\nSHA3-KECCAK>> Number of Input files : %d files (Total size: %llu Byte) ---", multifileindex, iTotSize );

			}
		#endif
		
		}
		else
		{
			printf("SHA3-KECCAK>> SHA3-224 hashing... \n");
		
		
			memset( sha3Buf, 0x00, sizeof(sha3Buf) );
			memset( sha3out, 0x00, sizeof(sha3out) );
			memset( sha3digestTxt, 0x00, sizeof(sha3digestTxt) );
			
			// Initialize the SHA3-256 context
			sha3_init(SHA3_224_HASH_BIT, SHA3_SHAKE_NONE);	

			kll = 0;
			while((ll = fread(sha3Buf, 1, sizeof(sha3Buf), inpfile)) > 0) 
			{
				kll += ll;
			
				sha3_update(sha3Buf, ll);
				
				//Digest the message
				// LOG_V("\bSHA3-224 Hashing for (%s) -> read : %lld \r", infile_name, kll );
			}
			ClearScreen(); 

			ret = sha3_final(sha3out, SHA3_OUT_224);
			
			for (ii = 0; ii < SHA3_OUT_224; ii++)
			{
				sprintf(&sha3digestTxt[ii*2], "%02x", sha3out[ii]);
			}
			

			if( (2==iVerbosType || 3==iVerbosType) && (TRUE == verbose) )
			{
				printf("%s  *SHA3-224*%s__(%llu) \r\n", sha3digestTxt, infile_name, infile_size   );
				if(outfile) fprintf(outfile, "%s  *SHA3-224*%s__(%llu) \r\n", sha3digestTxt, infile_name, infile_size );
			}			
			else
			{
				printf("%s  *SHA3-224*%s \r\n", sha3digestTxt, infile_name );
				if(outfile) fprintf(outfile, "%s  *SHA3-224*%s \r\n", sha3digestTxt, infile_name );
			}

			if(inpfile) { fclose(inpfile); inpfile=NULL; }
			if(data_buf){ free(data_buf); data_buf=NULL; }
			printf("\nSHA3-KECCAK>> Calculated SHA3-224 Hash Value - OK" );
				
		}	
		
		opt_ok = 0; /* 2014.07.16 : NOTE !!! clear!!!  */

	}
	else if(1 == isSHA3_KECCAK_256)
	{
		unsigned char sha3Buf[SHA3_BUFLEN] = {0,};
		char	sha3digestTxt[SHA3_OUTPUT_SIZ] = {0,};
		unsigned char sha3out[SHA3_OUTPUT_SIZ] = { 0, };
		
		unsigned __int64 	kll=0;
		size_t		ll=0;
		int 		ret;
		unsigned int iLenSub=0;
	#if 1
		struct	tm *pTime;

		pTime = current_time();

		if( (1==iVerbosType || 3==iVerbosType) && (TRUE == verbose) && outfile) /// 2014.08.01
		{
			fprintf(outfile, "----------------------------------------------------------------------------------\r\n" );
			fprintf(outfile, "SHA3-256 Hash is created at %04d/%02d/%02d/%s %02d:%02d:%02d by %s \n", pTime->tm_year, pTime->tm_mon, pTime->tm_mday,  
													WeekTXT[pTime->tm_wday], pTime->tm_hour, pTime->tm_min, pTime->tm_sec, EmailText );
			fprintf(outfile, "----------------------------------------------------------------------------------\r\n" );
		}		
	#endif

		LOG_V("\n");
	
		if( ASTERISK_FOUND == isAsteMode )  /* For ah.exe --input *.* */
		{
			int iIdx=0;
			unsigned __int64 	kll = 0UL;


			printf("SHA3-KECCAK>> SHA3-256 hashing for input files* \n");
			for(ii=0; ii<SHA3_OUT_256*2; ii++) printf("-");  
			printf(" %d --\r\n", SHA3_OUT_256*2 );

			if(inpfile) { fclose(inpfile); inpfile=NULL; }
			if(data_buf){ free(data_buf); data_buf=NULL; }

			for(ii=iFirst; ii<=(argc-isAsteminus); ii++)
			{
				if( NULL != (inpfile = fopen( argv[ii], "rb")) ) 
				{
					memset( sha3Buf, 0x00, sizeof(sha3Buf) );
					memset( sha3out, 0x00, sizeof(sha3out) );
					memset( sha3digestTxt, 0x00, sizeof(sha3digestTxt) );
					
					// Initialize the SHA3-256 context
					sha3_init(SHA3_256_HASH_BIT, SHA3_SHAKE_NONE);				
	
					kll = 0UL;
					while((ll = fread(sha3Buf, 1, sizeof(sha3Buf), inpfile)) > 0) 
					{
						kll += ll;
						sha3_update(sha3Buf, ll);
						
						//Digest the message
						// LOG_V("\bSHA3-256 Hashing for (%s) -> read : %lld Bytes \r", argv[ii], kll );
					}
					ClearScreen(); 

					ret = sha3_final(sha3out, SHA3_OUT_256);
	
					for (iIdx = 0; iIdx < SHA3_OUT_256; iIdx++)
					{
						//printf("%02x", sha3out[ii]);
						sprintf(&sha3digestTxt[iIdx*2], "%02x", sha3out[iIdx]);
					}
	
					if( (2==iVerbosType || 3==iVerbosType) && (TRUE == verbose) )
					{
						printf("%s  *SHA3-256*%s__(%llu) \r\n", sha3digestTxt, argv[ii], kll );
						if(outfile) fprintf(outfile, "%s  *SHA3-256*%s__(%llu) \r\n", sha3digestTxt, argv[ii], kll );
					}
					else
					{
						printf("%s  *SHA3-256*%s \r\n", sha3digestTxt, argv[ii]);
						if(outfile) fprintf(outfile, "%s  *SHA3-256*%s \r\n", sha3digestTxt, argv[ii] );
					}

					if(inpfile) { fclose(inpfile); inpfile=NULL; }
					if(data_buf){ free(data_buf); data_buf=NULL; }
				
					iTotSize += kll;
					multifileindex ++;
				}
			}
			printf("\nSHA3-KECCAK>> SHA3-256 Number of Input files : %d files (Total size: %llu Byte) ---", multifileindex, iTotSize );

			AllFilesClosed();
	
		}
		else if( multifileindex > 0 )
		{	
			if(inpfile) { fclose(inpfile); inpfile=NULL; }
			if(data_buf){ free(data_buf); data_buf=NULL; }

		#if 1 // 
			/* Find first .c file in current directory */
			if( (retFiles = _findfirsti64( extfile_name, &iFiles )) == -1L )
			{
				printf("SHA3-KECCAK>>No file [%s] in current directory!!! [%s] in SHA3-256 \n", extfile_name, infile_name );
			}
			else
			{

				printf("SHA3-KECCAK>> SHA3-256 hashing for input files... \n");
				//printf("-------------------------------------------------------------\r\n" );
				for(ii=0; ii<SHA3_OUT_256*2; ii++) printf("-");  
				printf(" %d --\r\n", SHA3_OUT_256*2 );
				
				do {

					iLenSub = strlen(iFiles.name);

					if( iFiles.attrib & _A_SUBDIR )
					{

					}
					else
					{
						if( NULL == (inpfile = fopen( iFiles.name, "rb")) ) 
						{
							beep(700,100);
							printf("\n\n[++ERROR++] Can not open multi-input file (%s) in SHA3-256. \n", iFiles.name );

							AllFilesClosed();
	
							exit(0);
							return 0;
						}

						memset( sha3Buf, 0x00, sizeof(sha3Buf) );
						memset( sha3out, 0x00, sizeof(sha3out) );
						memset( sha3digestTxt, 0x00, sizeof(sha3digestTxt) );
						
						// Initialize the SHA3-256 context
						sha3_init(SHA3_256_HASH_BIT, SHA3_SHAKE_NONE);				
		
						kll = 0;
						while((ll = fread(sha3Buf, 1, sizeof(sha3Buf), inpfile)) > 0) 
						{
							kll += ll;
		
							sha3_update(sha3Buf, ll);
							
							//Digest the message
							// LOG_V("\bSHA3-256 Hashing for (%s) -> read : %lld  \r", iFiles.name, kll );
						}
						ClearScreen(); 

						ret = sha3_final(sha3out, SHA3_OUT_256);
		
						for (ii = 0; ii < SHA3_OUT_256; ii++)
						{
							//printf("%02x", sha3out[ii]);
							sprintf(&sha3digestTxt[ii*2], "%02x", sha3out[ii]);
						}
		
						if( (2==iVerbosType || 3==iVerbosType) && (TRUE == verbose) )
						{
							printf("%s  *SHA3-256*%s__(%llu) \r\n", sha3digestTxt, iFiles.name, iFiles.size );
							if(outfile) fprintf(outfile, "%s  *SHA3-256*%s__(%llu) \r\n", sha3digestTxt, iFiles.name, iFiles.size );
						}
						else
						{
							printf("%s  *SHA3-256*%s \r\n", sha3digestTxt, iFiles.name );
							if(outfile) fprintf(outfile, "%s  *SHA3-256*%s \r\n", sha3digestTxt, iFiles.name  );
						}

						if(inpfile) { fclose(inpfile); inpfile=NULL; }
						if(data_buf){ free(data_buf); data_buf=NULL; }
					
						iTotSize += iFiles.size;
						multifileindex ++;
					}

				} while( 0 == _findnexti64( retFiles, &iFiles ) );
				_findclose( retFiles );

				multifileindex--;

				if( iTotSize>>20 )
					printf("\nSHA3-KECCAK>> Number of Input files : %d files (Total size: %.3f MB) ---", multifileindex, (iTotSize/1024.0)/1024.0 );
				else if( iTotSize>>10 )
					printf("\nSHA3-KECCAK>> Number of Input files : %d files (Total size: %.3f kB) ---", multifileindex, (iTotSize/1024.0) );
				else
					printf("\nSHA3-KECCAK>> Number of Input files : %d files (Total size: %llu Byte) ---", multifileindex, iTotSize );

			}
		#endif
		
		}
		else
		{
			printf("SHA3-KECCAK>> SHA3-256 hashing... \n");
		
		
			memset( sha3Buf, 0x00, sizeof(sha3Buf) );
			memset( sha3out, 0x00, sizeof(sha3out) );
			memset( sha3digestTxt, 0x00, sizeof(sha3digestTxt) );
			
			// Initialize the SHA3-256 context
			sha3_init(SHA3_256_HASH_BIT, SHA3_SHAKE_NONE);	

			kll = 0;
			while((ll = fread(sha3Buf, 1, sizeof(sha3Buf), inpfile)) > 0) 
			{
				kll += ll;
			
				sha3_update(sha3Buf, ll);
				
				//Digest the message
				// LOG_V("\bSHA3-256 Hashing for (%s) -> read : %lld  \r", infile_name, kll );
			}
			ClearScreen(); 

			ret = sha3_final(sha3out, SHA3_OUT_256);
			
			for (ii = 0; ii < SHA3_OUT_256; ii++)
			{
				sprintf(&sha3digestTxt[ii*2], "%02x", sha3out[ii]);
			}


			if( (2==iVerbosType || 3==iVerbosType) && (TRUE == verbose) )
			{
				printf("%s  *SHA3-256*%s__(%llu) \r\n", sha3digestTxt, infile_name, infile_size   );
				if(outfile) fprintf(outfile, "%s  *SHA3-256*%s__(%llu) \r\n", sha3digestTxt, infile_name, infile_size );
			}
			else
			{
				printf("%s  *SHA3-256*%s \r\n", sha3digestTxt, infile_name );
				if(outfile) fprintf(outfile, "%s  *SHA3-256*%s \r\n", sha3digestTxt, infile_name );
			}
			
			if(inpfile) { fclose(inpfile); inpfile=NULL; }
			if(data_buf){ free(data_buf); data_buf=NULL; }
			printf("\nSHA3-KECCAK>> Calculated SHA3-256 Hash Value - OK" );
				
		}	
		
		opt_ok = 0; /* 2014.07.16 : NOTE !!! clear!!!  */

	}
	else if(1 == isSHA3_KECCAK_384)
	{
		unsigned char sha3Buf[SHA3_BUFLEN] = {0,};
		char	sha3digestTxt[SHA3_OUTPUT_SIZ] = {0,};
		unsigned char sha3out[SHA3_OUTPUT_SIZ] = { 0, };
		
		unsigned __int64 	kll=0;
		size_t		ll=0;
		int 		ret;
		unsigned int iLenSub=0;
		struct	tm *pTime;
	
		pTime = current_time();

		if( (1==iVerbosType || 3==iVerbosType) && (TRUE == verbose) && outfile) /// 2014.08.01
		{
			fprintf(outfile, "----------------------------------------------------------------------------------\r\n" );
			fprintf(outfile, "SHA3-384 Hash is created at %04d/%02d/%02d/%s %02d:%02d:%02d by %s \n", pTime->tm_year, pTime->tm_mon, pTime->tm_mday,  
													WeekTXT[pTime->tm_wday], pTime->tm_hour, pTime->tm_min, pTime->tm_sec, EmailText );
			fprintf(outfile, "----------------------------------------------------------------------------------\r\n" );
		}		

		LOG_V("\n");

		if( ASTERISK_FOUND == isAsteMode )  /* For ah.exe --input *.* */
		{
			int iIdx=0;
			unsigned __int64 	kll = 0UL;

			printf("SHA3-KECCAK>> SHA3-384  hashing for input files* \n");
			for(ii=0; ii<SHA3_OUT_384*2; ii++) printf("-");  
			printf(" %d --\r\n", SHA3_OUT_384*2 );

			if(inpfile) { fclose(inpfile); inpfile=NULL; }
			if(data_buf){ free(data_buf); data_buf=NULL; }

			for(ii=iFirst; ii<=(argc-isAsteminus); ii++)
			{
				if( NULL != (inpfile = fopen( argv[ii], "rb")) ) 
				{
					memset( sha3Buf, 0x00, sizeof(sha3Buf) );
					memset( sha3out, 0x00, sizeof(sha3out) );
					memset( sha3digestTxt, 0x00, sizeof(sha3digestTxt) );
					
					// Initialize the SHA3-256 context
					sha3_init(SHA3_384_HASH_BIT, SHA3_SHAKE_NONE);				
	
					kll = 0UL;
					while((ll = fread(sha3Buf, 1, sizeof(sha3Buf), inpfile)) > 0) 
					{
						kll += ll;	
						sha3_update(sha3Buf, ll);

						//Digest the message
						// LOG_V("\bSHA3-384 Hashing for (%s) -> read : %lld Bytes \r", argv[ii], kll );
					}
					ClearScreen(); 

					ret = sha3_final(sha3out, SHA3_OUT_384);
	
					for (iIdx = 0; iIdx < SHA3_OUT_384; iIdx++)
					{
						sprintf(&sha3digestTxt[iIdx*2], "%02x", sha3out[iIdx]);
					}

					if( (2==iVerbosType || 3==iVerbosType) && (TRUE == verbose) )
					{
						printf("%s  *SHA3-384*%s__(%llu) \r\n", sha3digestTxt, argv[ii], kll );
						if(outfile) fprintf(outfile, "%s  *SHA3-384*%s__(%llu) \r\n", sha3digestTxt, argv[ii], kll );
					}
					else
					{
						printf("%s  *SHA3-384*%s \r\n", sha3digestTxt, argv[ii] );
						if(outfile) fprintf(outfile, "%s  *SHA3-384*%s \r\n", sha3digestTxt, argv[ii] );
					}

					if(inpfile) { fclose(inpfile); inpfile=NULL; }
					if(data_buf){ free(data_buf); data_buf=NULL; }

					iTotSize += kll;
					multifileindex ++;
				}
			}
			
			printf("\nSHA3-KECCAK>> SHA3-384 Number of Input files : %d files (Total size: %llu Byte) ---", multifileindex, iTotSize );

			AllFilesClosed();
	
		}
		else if( multifileindex > 0 )
		{
		
			if(inpfile) { fclose(inpfile); inpfile=NULL; }
			if(data_buf){ free(data_buf); data_buf=NULL; }

#if 1 // 
			/* Find first .c file in current directory */
			if( (retFiles = _findfirsti64( extfile_name, &iFiles )) == -1L )
			{
				printf("SHA3-KECCAK>>No file [%s] in current directory!!! [%s] in SHA3-384 \n", extfile_name, infile_name );
			}
			else
			{

				printf("SHA3-KECCAK>> SHA3-384 hashing for input files... \n");
				//printf("-------------------------------------------------------------\r\n" );
				for(ii=0; ii<SHA3_OUT_384*2; ii++) printf("-");  
				printf(" %d --\r\n", SHA3_OUT_384*2 );
				
				do {

					iLenSub = strlen(iFiles.name);

					if( iFiles.attrib & _A_SUBDIR )
					{

					}
					else
					{
						if( NULL == (inpfile = fopen( iFiles.name, "rb")) ) 
						{
							beep(700,100);
							printf("\n\n[++ERROR++] Can not open multi-input file (%s) in SHA3-384. \n", iFiles.name );

							AllFilesClosed();

							exit(0);
							return 0;
						}

						memset( sha3Buf, 0x00, sizeof(sha3Buf) );
						memset( sha3out, 0x00, sizeof(sha3out) );
						memset( sha3digestTxt, 0x00, sizeof(sha3digestTxt) );
						
						// Initialize the SHA3-256 context
						sha3_init(SHA3_384_HASH_BIT, SHA3_SHAKE_NONE);				
		
						kll = 0;
						while((ll = fread(sha3Buf, 1, sizeof(sha3Buf), inpfile)) > 0) 
						{
							kll += ll;
		
							sha3_update(sha3Buf, ll);
							
							//Digest the message
							// LOG_V("\bSHA3-384 Hashing for (%s) -> read : %lld \r", iFiles.name, kll  );
						}
						ClearScreen(); 

						ret = sha3_final(sha3out, SHA3_OUT_384);
		
						for (ii = 0; ii < SHA3_OUT_384; ii++)
						{
							//printf("%02x", sha3out[ii]);
							sprintf(&sha3digestTxt[ii*2], "%02x", sha3out[ii]);
						}

						if( (2==iVerbosType || 3==iVerbosType) && (TRUE == verbose) )
						{
							printf("%s  *SHA3-384*%s__(%llu) \r\n", sha3digestTxt, iFiles.name, iFiles.size   );
							if(outfile) fprintf(outfile, "%s  *SHA3-384*%s__(%llu) \r\n", sha3digestTxt, iFiles.name, iFiles.size	);
						}
						else
						{
							printf("%s  *SHA3-384*%s \r\n", sha3digestTxt, iFiles.name );
							if(outfile) fprintf(outfile, "%s  *SHA3-384*%s \r\n", sha3digestTxt, iFiles.name );
						}

						if(inpfile) { fclose(inpfile); inpfile=NULL; }
						if(data_buf){ free(data_buf); data_buf=NULL; }

						iTotSize += iFiles.size;
						multifileindex ++;
					}

				} while( 0 == _findnexti64( retFiles, &iFiles ) );
				_findclose( retFiles );

				multifileindex--;

				if( iTotSize>>20 )
					printf("\nSHA3-KECCAK>> Number of Input files : %d files (Total size: %.3f MB) ---", multifileindex, (iTotSize/1024.0)/1024.0 );
				else if( iTotSize>>10 )
					printf("\nSHA3-KECCAK>> Number of Input files : %d files (Total size: %.3f kB) ---", multifileindex, (iTotSize/1024.0) );
				else
					printf("\nSHA3-KECCAK>> Number of Input files : %d files (Total size: %llu Byte) ---", multifileindex, iTotSize );

			}
#endif
		
		}
		else
		{
			printf("SHA3-KECCAK>> SHA3-384 hashing... \n");
		
		
			memset( sha3Buf, 0x00, sizeof(sha3Buf) );
			memset( sha3out, 0x00, sizeof(sha3out) );
			memset( sha3digestTxt, 0x00, sizeof(sha3digestTxt) );
			
			// Initialize the SHA3-384 context
			sha3_init(SHA3_384_HASH_BIT, SHA3_SHAKE_NONE);	

			kll = 0;
			while((ll = fread(sha3Buf, 1, sizeof(sha3Buf), inpfile)) > 0) 
			{
				kll += ll;
			
				sha3_update(sha3Buf, ll);
				
				//Digest the message
				// LOG_V("\bSHA3-384 Hashing for (%s) -> read : %lld \r", infile_name, kll );
			}
			ClearScreen(); 

			ret = sha3_final(sha3out, SHA3_OUT_384);
			
			for (ii = 0; ii < SHA3_OUT_384; ii++)
			{
				sprintf(&sha3digestTxt[ii*2], "%02x", sha3out[ii]);
			}
			

			if( (2==iVerbosType || 3==iVerbosType) && (TRUE == verbose) )
			{
				printf("%s  *SHA3-384*%s__(%llu) \r\n", sha3digestTxt, infile_name, infile_size   );
				if(outfile) fprintf(outfile, "%s  *SHA3-384*%s__(%llu) \r\n", sha3digestTxt, infile_name, infile_size );
			}			
			else
			{
				printf("%s  *SHA3-384*%s \r\n", sha3digestTxt, infile_name );
				if(outfile) fprintf(outfile, "%s  *SHA3-384*%s \r\n", sha3digestTxt, infile_name );
			}
			
			if(inpfile) { fclose(inpfile); inpfile=NULL; }
			if(data_buf){ free(data_buf); data_buf=NULL; }
			printf("\nSHA3-KECCAK>> Calculated SHA3-384 Hash Value - OK" );
				
		}	
		
		opt_ok = 0; /* 2014.07.16 : NOTE !!! clear!!!  */

	}
	else if(1 == isSHA3_KECCAK_512)
	{
		unsigned char sha3Buf[SHA3_BUFLEN] = {0,};
		char	sha3digestTxt[SHA3_OUTPUT_SIZ] = {0,};
		unsigned char sha3out[SHA3_OUTPUT_SIZ] = { 0, };
		
		unsigned __int64 	kll=0;
		size_t		ll=0;
		int 		ret;
		unsigned int iLenSub=0;
		struct	tm *pTime;
	
		pTime = current_time();

		if( (1==iVerbosType || 3==iVerbosType) && (TRUE == verbose) && outfile) /// 2014.08.01
		{
			fprintf(outfile, "----------------------------------------------------------------------------------\r\n" );
			fprintf(outfile, "SHA3-512 Hash is created at %04d/%02d/%02d/%s %02d:%02d:%02d by %s \n", pTime->tm_year, pTime->tm_mon, pTime->tm_mday,  
													WeekTXT[pTime->tm_wday], pTime->tm_hour, pTime->tm_min, pTime->tm_sec, EmailText );
			fprintf(outfile, "----------------------------------------------------------------------------------\r\n" );
		}		

		LOG_V("\n");

		if( ASTERISK_FOUND == isAsteMode )  /* For ah.exe --input *.* */
		{
			int iIdx=0;
			unsigned __int64 	kll = 0UL;


			printf("SHA3-KECCAK>> SHA3-512 hashing for input files* \n");
			for(ii=0; ii<SHA3_OUT_512*2; ii++) printf("-");  
			printf(" %d --\r\n", SHA3_OUT_512*2 );

			if(inpfile) { fclose(inpfile); inpfile=NULL; }
			if(data_buf){ free(data_buf); data_buf=NULL; }

			for(ii=iFirst; ii<=(argc-isAsteminus); ii++)
			{
				if( NULL != (inpfile = fopen( argv[ii], "rb")) ) 
				{
					memset( sha3Buf, 0x00, sizeof(sha3Buf) );
					memset( sha3out, 0x00, sizeof(sha3out) );
					memset( sha3digestTxt, 0x00, sizeof(sha3digestTxt) );
					
					// Initialize the SHA3-512 context
					sha3_init(SHA3_512_HASH_BIT, SHA3_SHAKE_NONE);				
	
					kll = 0UL;
					while((ll = fread(sha3Buf, 1, sizeof(sha3Buf), inpfile)) > 0) 
					{
						kll += ll;
	
						sha3_update(sha3Buf, ll);
						
						//Digest the message
						// LOG_V("\bSHA3-512 Hashing for (%s) -> read : %lld Bytes \r", argv[ii], kll );
					}
					ClearScreen(); 

					ret = sha3_final(sha3out, SHA3_OUT_512);
	
					for (iIdx = 0; iIdx < SHA3_OUT_512; iIdx++)
					{
						sprintf(&sha3digestTxt[iIdx*2], "%02x", sha3out[iIdx]);
					}

					if( (2==iVerbosType || 3==iVerbosType) && (TRUE == verbose) )
					{
						printf("%s  *SHA3-512*%s__(%llu) \r\n", sha3digestTxt, argv[ii], kll );
						if(outfile) fprintf(outfile, "%s  *SHA3-512*%s__(%llu) \r\n", sha3digestTxt, argv[ii], kll);
					}
					else
					{
						printf("%s  *SHA3-512*%s \r\n", sha3digestTxt, argv[ii] );
						if(outfile) fprintf(outfile, "%s  *SHA3-512*%s \r\n", sha3digestTxt, argv[ii]);
					}

					if(inpfile) { fclose(inpfile); inpfile=NULL; }
					if(data_buf){ free(data_buf); data_buf=NULL; }

					iTotSize += kll;
					multifileindex ++;
				}
			}
			printf("\nSHA3-KECCAK>> SHA3-512 Number of Input files : %d files (Total size: %llu Byte) ---", multifileindex, iTotSize );

			AllFilesClosed();

		}
		else if( multifileindex > 0 )
		{
		
			if(inpfile) { fclose(inpfile); inpfile=NULL; }
			if(data_buf){ free(data_buf); data_buf=NULL; }

		#if 1 // 
			/* Find first .c file in current directory */
			if( (retFiles = _findfirsti64( extfile_name, &iFiles )) == -1L )
			{
				printf("SHA3-KECCAK>>No file [%s] in current directory!!! [%s] in SHA3-512 \n", extfile_name, infile_name );
			}
			else
			{

				printf("SHA3-KECCAK>> SHA3-512 hashing for input files... \n");
				//printf("-------------------------------------------------------------\r\n" );
				for(ii=0; ii<SHA3_OUT_512*2; ii++) printf("-");  
				printf(" %d --\r\n", SHA3_OUT_512*2 );
				
				do {

					iLenSub = strlen(iFiles.name);

					if( iFiles.attrib & _A_SUBDIR )
					{

					}
					else
					{
						if( NULL == (inpfile = fopen( iFiles.name, "rb")) ) 
						{
							beep(700,100);
							printf("\n\n[++ERROR++] Can not open multi-input file (%s) in SHA3-512. \n", iFiles.name );

							AllFilesClosed();
	
							exit(0);
							return 0;
						}

						memset( sha3Buf, 0x00, sizeof(sha3Buf) );
						memset( sha3out, 0x00, sizeof(sha3out) );
						memset( sha3digestTxt, 0x00, sizeof(sha3digestTxt) );
						
						// Initialize the SHA3-512 context
						sha3_init(SHA3_512_HASH_BIT, SHA3_SHAKE_NONE);				
		
						kll = 0;
						while((ll = fread(sha3Buf, 1, sizeof(sha3Buf), inpfile)) > 0) 
						{
							kll += ll;
		
							sha3_update(sha3Buf, ll);
							
							//Digest the message
							// LOG_V("\bSHA3-512 Hashing for (%s) -> read : %lld \r", iFiles.name, kll );
						}
						ClearScreen(); 

						ret = sha3_final(sha3out, SHA3_OUT_512);
		
						for (ii = 0; ii < SHA3_OUT_512; ii++)
						{
							sprintf(&sha3digestTxt[ii*2], "%02x", sha3out[ii]);
						}

						
						if( (2==iVerbosType || 3==iVerbosType) && (TRUE == verbose) )
						{
							printf("%s  *SHA3-512*%s__(%llu) \r\n", sha3digestTxt, iFiles.name, iFiles.size  );
							if(outfile) fprintf(outfile, "%s  *SHA3-512*%s__(%llu) \r\n", sha3digestTxt, iFiles.name, iFiles.size );
						}
						else
						{
							printf("%s  *SHA3-512*%s \r\n", sha3digestTxt, iFiles.name );
							if(outfile) fprintf(outfile, "%s  *SHA3-512*%s \r\n", sha3digestTxt, iFiles.name );
						}

						if(inpfile) { fclose(inpfile); inpfile=NULL; }
						if(data_buf){ free(data_buf); data_buf=NULL; }

						iTotSize += iFiles.size;
						multifileindex ++;
					}

				} while( 0 == _findnexti64( retFiles, &iFiles ) );
				_findclose( retFiles );

				multifileindex--;

				if( iTotSize>>20 )
					printf("\nSHA3-KECCAK>> Number of Input files : %d files (Total size: %.3f MB) ---", multifileindex, (iTotSize/1024.0)/1024.0 );
				else if( iTotSize>>10 )
					printf("\nSHA3-KECCAK>> Number of Input files : %d files (Total size: %.3f kB) ---", multifileindex, (iTotSize/1024.0) );
				else
					printf("\nSHA3-KECCAK>> Number of Input files : %d files (Total size: %llu Byte) ---", multifileindex, iTotSize );

			}
		#endif
		
		}
		else
		{
			printf("SHA3-KECCAK>> SHA3-512 hashing... \n");
		
		
			memset( sha3Buf, 0x00, sizeof(sha3Buf) );
			memset( sha3out, 0x00, sizeof(sha3out) );
			memset( sha3digestTxt, 0x00, sizeof(sha3digestTxt) );
			
			// Initialize the SHA3-512 context
			sha3_init(SHA3_512_HASH_BIT, SHA3_SHAKE_NONE);	

			kll = 0;
			while((ll = fread(sha3Buf, 1, sizeof(sha3Buf), inpfile)) > 0) 
			{
				kll += ll;
			
				sha3_update(sha3Buf, ll);
				
				//Digest the message
				// LOG_V("\bSHA3-512 Hashing for (%s) -> read : %lld \r", infile_name, kll );
			}
			ClearScreen(); 

			ret = sha3_final(sha3out, SHA3_OUT_512);
			
			for (ii = 0; ii < SHA3_OUT_512; ii++)
			{
				sprintf(&sha3digestTxt[ii*2], "%02x", sha3out[ii]);
			}
			

			if( (2==iVerbosType || 3==iVerbosType) && (TRUE == verbose) )
			{
				printf("%s  *SHA3-512*%s__(%llu) \r\n", sha3digestTxt, infile_name, infile_size   );
				if(outfile) fprintf(outfile, "%s  *SHA3-512*%s__(%llu) \r\n", sha3digestTxt, infile_name, infile_size );
			}			
			else
			{
				printf("%s  *SHA3-512*%s \r\n", sha3digestTxt, infile_name );
				if(outfile) fprintf(outfile, "%s  *SHA3-512*%s \r\n", sha3digestTxt, infile_name );
			}

			if(inpfile) { fclose(inpfile); inpfile=NULL; }
			if(data_buf){ free(data_buf); data_buf=NULL; }
			printf("\nSHA3-KECCAK>> Calculated SHA3-512 Hash Value - OK" );
				
		}	
		
		opt_ok = 0; /* 2014.07.16 : NOTE !!! clear!!!  */

	}
	else if(1 == isShake128)
	{
		unsigned char sha3Buf[SHA3_BUFLEN] = {0,};
		char	sha3digestTxt[SHA3_OUTPUT_SIZ] = {0,};
		unsigned char sha3out[SHA3_OUTPUT_SIZ] = { 0, };
		
		unsigned __int64 	kll=0;
		size_t		ll=0;
		int 		ret;
		unsigned int iLenSub=0;
	#if 1
		struct	tm *pTime;
	
		pTime = current_time();
		
		if( (1==iVerbosType || 3==iVerbosType) && (TRUE == verbose) && outfile) /// 2014.08.01
		{
			fprintf(outfile, "----------------------------------------------------------------------------------\r\n" );
			fprintf(outfile, "SHAKE128 Hash is created at %04d/%02d/%02d/%s %02d:%02d:%02d by %s \n", pTime->tm_year, pTime->tm_mon, pTime->tm_mday,  
													WeekTXT[pTime->tm_wday], pTime->tm_hour, pTime->tm_min, pTime->tm_sec, EmailText );
			fprintf(outfile, "----------------------------------------------------------------------------------\r\n" );
		}		
	#endif

		LOG_V("\n");

		if( ASTERISK_FOUND == isAsteMode )  /* For ah.exe --input *.* */
		{
			int iIdx=0;
			unsigned __int64 	kll = 0UL;


			printf("SHAKE128>> SHAKE128 hashing for input files* \n");
			for(ii=0; ii<SHAKE_OUT_128*2; ii++) printf("-");  
			printf(" %d --\r\n", SHAKE_OUT_128*2 );

			if(inpfile) { fclose(inpfile); inpfile=NULL; }
			if(data_buf){ free(data_buf); data_buf=NULL; }

			for(ii=iFirst; ii<=(argc-isAsteminus); ii++)
			{
				if( NULL != (inpfile = fopen( argv[ii], "rb")) ) 
				{
					memset( sha3Buf, 0x00, sizeof(sha3Buf) );
					memset( sha3out, 0x00, sizeof(sha3out) );
					memset( sha3digestTxt, 0x00, sizeof(sha3digestTxt) );
					
					// Initialize the Shake128 context
					sha3_init(SHAKE_128_HASH_BIT, SHA3_SHAKE_USE);	
	
					kll = 0UL;
					while((ll = fread(sha3Buf, 1, sizeof(sha3Buf), inpfile)) > 0) 
					{
						kll += ll;
						sha3_update(sha3Buf, ll);
						
						//Digest the message
						// LOG_V("\bSHAKE128 Hashing for (%s) -> read : %lld Bytes \r", argv[ii], kll );
					}
					ClearScreen(); 

					ret = sha3_final(sha3out, SHAKE_OUT_128);
					
					for (iIdx = 0; iIdx < SHAKE_OUT_128; iIdx++)
					{
						sprintf(&sha3digestTxt[iIdx*2], "%02x", sha3out[iIdx]);
					}

					if( (2==iVerbosType || 3==iVerbosType) && (TRUE == verbose) )
					{
						printf("%s  *SHAKE128*%s__(%llu) \r\n", sha3digestTxt, argv[ii], kll );
						if(outfile) fprintf(outfile, "%s  *SHAKE128*%s__(%llu) \r\n", sha3digestTxt, argv[ii], kll );
					}
					else
					{
						printf("%s  *SHAKE128*%s \r\n", sha3digestTxt, argv[ii] );
						if(outfile) fprintf(outfile, "%s  *SHAKE128*%s \r\n", sha3digestTxt, argv[ii] );
					}

					if(inpfile) { fclose(inpfile); inpfile=NULL; }
					if(data_buf){ free(data_buf); data_buf=NULL; }

					iTotSize += kll;
					multifileindex ++;
				}
			}
			printf("\nSHAKE128>> SHAKE128 Number of Input files : %d files (Total size: %llu Byte) ---", multifileindex, iTotSize );

			AllFilesClosed();
	
		}
		else if( multifileindex > 0 )
		{
		
			if(inpfile) { fclose(inpfile); inpfile=NULL; }
			if(data_buf){ free(data_buf); data_buf=NULL; }

	#if 1 // 
			/* Find first .c file in current directory */
			if( (retFiles = _findfirsti64( extfile_name, &iFiles )) == -1L )
			{
				printf("SHAKE128>>No file [%s] in current directory!!! [%s] \n", extfile_name, infile_name );
			}
			else
			{

				printf("SHAKE128>> SHAKE128 hashing for input files... \n");
				//printf("-------------------------------------------------------------\r\n" );
				for(ii=0; ii<SHAKE_OUT_128*2; ii++) printf("-");  
				printf(" %d --\r\n", SHAKE_OUT_128*2 );
				
				do {
					iLenSub = strlen(iFiles.name);

					if( iFiles.attrib & _A_SUBDIR )
					{

					}
					else
					{
						if( NULL == (inpfile = fopen( iFiles.name, "rb")) ) 
						{
							beep(700,100);
							printf("\n\n[++ERROR++] Can not open multi-input file (%s) in SHAKE128. \n", iFiles.name );

							AllFilesClosed();
	
							exit(0);
							return 0;
						}
		
						memset( sha3Buf, 0x00, sizeof(sha3Buf) );
						memset( sha3out, 0x00, sizeof(sha3out) );
						memset( sha3digestTxt, 0x00, sizeof(sha3digestTxt) );
						
						// Initialize the Shake128 context
						sha3_init(SHAKE_128_HASH_BIT, SHA3_SHAKE_USE);	
		
						kll = 0;
						while((ll = fread(sha3Buf, 1, sizeof(sha3Buf), inpfile)) > 0) 
						{
							kll += ll;
		
							sha3_update(sha3Buf, ll);
							
							//Digest the message
							// LOG_V("\bSHAKE128 Hashing for (%s) -> read : %lld \r", iFiles.name, kll );
						}
						ClearScreen(); 

						ret = sha3_final(sha3out, SHAKE_OUT_128);
						
						for (ii = 0; ii < SHAKE_OUT_128; ii++)
						{
							sprintf(&sha3digestTxt[ii*2], "%02x", sha3out[ii]);
						}

						
						if( (2==iVerbosType || 3==iVerbosType) && (TRUE == verbose) )
						{
							printf("%s  *SHAKE128*%s__(%llu) \r\n", sha3digestTxt, iFiles.name, iFiles.size );
							if(outfile) fprintf(outfile, "%s  *SHAKE128*%s__(%llu) \r\n", sha3digestTxt, iFiles.name, iFiles.size );
						}
						else
						{
							printf("%s  *SHAKE128*%s \r\n", sha3digestTxt, iFiles.name );
							if(outfile) fprintf(outfile, "%s  *SHAKE128*%s \r\n", sha3digestTxt, iFiles.name );
						}

						if(inpfile) { fclose(inpfile); inpfile=NULL; }
						if(data_buf){ free(data_buf); data_buf=NULL; }

						iTotSize += iFiles.size;
						multifileindex ++;
					}

				} while( 0 == _findnexti64( retFiles, &iFiles ) );
				_findclose( retFiles );

				multifileindex--;

				if( iTotSize>>20 )
					printf("\nSHAKE128>> Number of Input files : %d files (Total size: %.3f MB) ---", multifileindex, (iTotSize/1024.0)/1024.0 );
				else if( iTotSize>>10 )
					printf("\nSHAKE128>> Number of Input files : %d files (Total size: %.3f kB) ---", multifileindex, (iTotSize/1024.0) );
				else
					printf("\nSHAKE128>> Number of Input files : %d files (Total size: %llu Byte) ---", multifileindex, iTotSize );

			}
	#endif
		
		}
		else
		{
			printf("SHAKE128>> SHAKE128 hashing... \n");
		
		
			memset( sha3Buf, 0x00, sizeof(sha3Buf) );
			memset( sha3out, 0x00, sizeof(sha3out) );
			memset( sha3digestTxt, 0x00, sizeof(sha3digestTxt) );
			
			// Initialize the SHAKE128 context
			sha3_init(SHAKE_128_HASH_BIT, SHA3_SHAKE_USE);	

			kll = 0;
			while((ll = fread(sha3Buf, 1, sizeof(sha3Buf), inpfile)) > 0) 
			{
				kll += ll;
			
				sha3_update(sha3Buf, ll);
				
				//Digest the message
				// LOG_V("\bSHAKE128 Hashing for (%s) -> read : %lld \r", infile_name, kll );
			}
			ClearScreen(); 

			ret = sha3_final(sha3out, SHAKE_OUT_128);
			
			for (ii = 0; ii < SHAKE_OUT_128; ii++)
			{
				sprintf(&sha3digestTxt[ii*2], "%02x", sha3out[ii]);
			}
				

			if( (2==iVerbosType || 3==iVerbosType) && (TRUE == verbose) )
			{
				printf("%s  *SHAKE128*%s__(%llu) \r\n", sha3digestTxt, infile_name, infile_size   );
				if(outfile) fprintf(outfile, "%s  *SHAKE128*%s__(%llu) \r\n", sha3digestTxt, infile_name, infile_size );
			}
			else 
			{
				printf("%s  *SHAKE128*%s \r\n", sha3digestTxt, infile_name );
				if(outfile) fprintf(outfile, "%s  *SHAKE128*%s \r\n", sha3digestTxt, infile_name );
			}

			if(inpfile) { fclose(inpfile); inpfile=NULL; }
			if(data_buf){ free(data_buf); data_buf=NULL; }
			printf("\nSHAKE128>> Calculated SHAKE128 Hash Value - OK" );
			
		}
	}
	else if(1 == isShake256)
	{
		unsigned char sha3Buf[SHA3_BUFLEN] = {0,};
		char	sha3digestTxt[SHA3_OUTPUT_SIZ] = {0,};
		unsigned char sha3out[SHA3_OUTPUT_SIZ] = { 0, };
		
		unsigned __int64 	kll=0;
		size_t		ll=0;
		int 		ret;
		unsigned int iLenSub=0;
	#if 1
		struct	tm *pTime;
	
		pTime = current_time();

		if( (1==iVerbosType || 3==iVerbosType) && (TRUE == verbose) && outfile) /// 2014.08.01
		{
			fprintf(outfile, "----------------------------------------------------------------------------------\r\n" );
			fprintf(outfile, "SHAKE256 Hash is created at %04d/%02d/%02d/%s %02d:%02d:%02d by %s \n", pTime->tm_year, pTime->tm_mon, pTime->tm_mday,  
													WeekTXT[pTime->tm_wday], pTime->tm_hour, pTime->tm_min, pTime->tm_sec, EmailText );
			fprintf(outfile, "----------------------------------------------------------------------------------\r\n" );
		}		
	#endif

		LOG_V("\n");

		if( ASTERISK_FOUND == isAsteMode )  /* For ah.exe --input *.* */
		{
			int iIdx=0;
			unsigned __int64 	kll = 0UL;


			printf("SHAKE256>> SHAKE256 hashing for input files* \n");
			for(ii=0; ii<SHAKE_OUT_256*2; ii++) printf("-");  
			printf(" %d --\r\n", SHAKE_OUT_256*2 );

			if(inpfile) { fclose(inpfile); inpfile=NULL; }
			if(data_buf){ free(data_buf); data_buf=NULL; }

			for(ii=iFirst; ii<=(argc-isAsteminus); ii++)
			{
				if( NULL != (inpfile = fopen( argv[ii], "rb")) ) 
				{
					memset( sha3Buf, 0x00, sizeof(sha3Buf) );
					memset( sha3out, 0x00, sizeof(sha3out) );
					memset( sha3digestTxt, 0x00, sizeof(sha3digestTxt) );
					
					// Initialize the Shake128 context
					sha3_init(SHAKE_256_HASH_BIT, SHA3_SHAKE_USE);	
	
					kll = 0UL;
					while((ll = fread(sha3Buf, 1, sizeof(sha3Buf), inpfile)) > 0) 
					{
						kll += ll;
						sha3_update(sha3Buf, ll);
						
						//Digest the message
						// LOG_V("\bSHAKE256 Hashing for (%s) -> read : %lld Bytes \r", argv[ii], kll );
					}
					ClearScreen(); 

					ret = sha3_final(sha3out, SHAKE_OUT_256);
					
					for (iIdx = 0; iIdx < SHAKE_OUT_256; iIdx++)
					{
						sprintf(&sha3digestTxt[iIdx*2], "%02x", sha3out[iIdx]);
					}

					if( (2==iVerbosType || 3==iVerbosType) && (TRUE == verbose) )
					{
						printf("%s  *SHAKE256*%s__(%llu) \r\n", sha3digestTxt, argv[ii], kll );
						if(outfile) fprintf(outfile, "%s  *SHAKE128*%s__(%llu) \r\n", sha3digestTxt, argv[ii], kll );
					}
					else
					{
						printf("%s  *SHAKE256*%s \r\n", sha3digestTxt, argv[ii] );
						if(outfile) fprintf(outfile, "%s  *SHAKE128*%s \r\n", sha3digestTxt, argv[ii] );
					}

					if(inpfile) { fclose(inpfile); inpfile=NULL; }
					if(data_buf){ free(data_buf); data_buf=NULL; }

					iTotSize += kll;
					multifileindex ++;
				}
			}
			printf("\nSHAKE256>> SHAKE256 Number of Input files : %d files (Total size: %llu Byte) ---", multifileindex, iTotSize );

			AllFilesClosed();
	
		}
		else if( multifileindex > 0 )
		{
		
			if(inpfile) { fclose(inpfile); inpfile=NULL; }
			if(data_buf){ free(data_buf); data_buf=NULL; }

		#if 1 // 
			/* Find first .c file in current directory */
			if( (retFiles = _findfirsti64( extfile_name, &iFiles )) == -1L )
			{
				printf("SHAKE256>>No file [%s] in current directory!!! [%s] \n", extfile_name, infile_name );
			}
			else
			{

				printf("SHAKE256>> SHAKE256 hashing for input files... \n");
				//printf("-------------------------------------------------------------\r\n" );
				for(ii=0; ii<SHAKE_OUT_256*2; ii++) printf("-");  
				printf(" %d --\r\n", SHAKE_OUT_256*2 );
				
				do {

					iLenSub = strlen(iFiles.name);

					if( iFiles.attrib & _A_SUBDIR )
					{

					}
					else
					{
						if( NULL == (inpfile = fopen( iFiles.name, "rb")) ) 
						{
							beep(700,100);
							printf("\n\n[++ERROR++] Can not open multi-input file (%s) in SHAKE256. \n", iFiles.name );

							AllFilesClosed();
	
							exit(0);
							return 0;
						}

						memset( sha3Buf, 0x00, sizeof(sha3Buf) );
						memset( sha3out, 0x00, sizeof(sha3out) );
						memset( sha3digestTxt, 0x00, sizeof(sha3digestTxt) );
						
						// Initialize the Shake128 context
						sha3_init(SHAKE_256_HASH_BIT, SHA3_SHAKE_USE);	
		
						kll = 0UL;
						while((ll = fread(sha3Buf, 1, sizeof(sha3Buf), inpfile)) > 0) 
						{
							kll += ll;
							sha3_update(sha3Buf, ll);
							
							//Digest the message
							// LOG_V("\bSHAKE256 Hashing for (%s) -> read : %lld Bytes \r", iFiles.name, kll );
						}
						ClearScreen(); 

						ret = sha3_final(sha3out, SHAKE_OUT_256);
						
						for (ii = 0; ii < SHAKE_OUT_256; ii++)
						{
							sprintf(&sha3digestTxt[ii*2], "%02x", sha3out[ii]);
						}

						if( (2==iVerbosType || 3==iVerbosType) && (TRUE == verbose) )
						{
							printf("%s  *SHAKE256*%s__(%llu) \r\n", sha3digestTxt, iFiles.name, iFiles.size  );
							if(outfile) fprintf(outfile, "%s  *SHAKE128*%s__(%llu) \r\n", sha3digestTxt, iFiles.name, iFiles.size	);
						}
						else
						{
							printf("%s  *SHAKE256*%s \r\n", sha3digestTxt, iFiles.name );
							if(outfile) fprintf(outfile, "%s  *SHAKE128*%s \r\n", sha3digestTxt, iFiles.name );
						}

						if(inpfile) { fclose(inpfile); inpfile=NULL; }
						if(data_buf){ free(data_buf); data_buf=NULL; }

						iTotSize += iFiles.size;
						multifileindex ++;
					}

				} while( 0 == _findnexti64( retFiles, &iFiles ) );
				_findclose( retFiles );

				multifileindex--;

				if( iTotSize>>20 )
					printf("\nSHAKE256>> Number of Input files : %d files (Total size: %.3f MB) ---", multifileindex, (iTotSize/1024.0)/1024.0 );
				else if( iTotSize>>10 )
					printf("\nSHAKE256>> Number of Input files : %d files (Total size: %.3f kB) ---", multifileindex, (iTotSize/1024.0) );
				else
					printf("\nSHAKE256>> Number of Input files : %d files (Total size: %llu Byte) ---", multifileindex, iTotSize );

			}
		#endif
		
		}
		else
		{
			printf("SHAKE256>> SHAKE256 hashing... \n");

			memset( sha3Buf, 0x00, sizeof(sha3Buf) );
			memset( sha3out, 0x00, sizeof(sha3out) );
			memset( sha3digestTxt, 0x00, sizeof(sha3digestTxt) );
			
			// Initialize the SHAKE128 context
			sha3_init(SHAKE_256_HASH_BIT, SHA3_SHAKE_USE);	

			kll = 0;
			while((ll = fread(sha3Buf, 1, sizeof(sha3Buf), inpfile)) > 0) 
			{
				kll += ll;
			
				sha3_update(sha3Buf, ll);
				
				//Digest the message
				// LOG_V("\bSHAKE256 Hashing for (%s) -> read : %lld Bytes \r", infile_name, kll );
			}
			ClearScreen(); 

			ret = sha3_final(sha3out, SHAKE_OUT_256);
			
			for (ii = 0; ii < SHAKE_OUT_256; ii++)
			{
				sprintf(&sha3digestTxt[ii*2], "%02x", sha3out[ii]);
			}

			if( (2==iVerbosType || 3==iVerbosType) && (TRUE == verbose) )
			{
				printf("%s  *SHAKE256*%s__(%llu) \r\n", sha3digestTxt, infile_name, infile_size   );
				if(outfile) fprintf(outfile, "%s  *SHAKE256*%s__(%llu) \r\n", sha3digestTxt, infile_name, infile_size );
			}
			else
			{
				printf("%s  *SHAKE256*%s \r\n", sha3digestTxt, infile_name );
				if(outfile) fprintf(outfile, "%s  *SHAKE256*%s \r\n", sha3digestTxt, infile_name );
			}

			if(inpfile) { fclose(inpfile); inpfile=NULL; }
			if(data_buf){ free(data_buf); data_buf=NULL; }
			printf("\nSHAKE256>> Calculated SHAKE256 Hash Value - OK" );
			
		}
	}
#endif // SHA3_KECCAK_224_256_384_512

	else if(1 == isRipeMD128)
	{
		
#define RMD128_INBUF_SIZ 		(1024)  // NOT NOT NEVER modify~~~
#define RMD128_BUF_LEN 			5 // 160/32
#define RMD128_LENGTH 			16 // 160/8

		unsigned char inbuf[RMD128_INBUF_SIZ];
		char outTxt[RMD128_INBUF_SIZ*2]={0,}; 
		
		unsigned long MDbuf[RMD128_BUF_LEN];   /* contains (A, B, C, D(, E))   */
		unsigned char hashcode[RMD128_LENGTH]; /* for final hash-value		   */
		
		unsigned __int64	kll;
		size_t		nbytes; // ll;
		unsigned long X[16];		  /* current 16-word chunk	   */
		unsigned int  i, j; 		  /* counters					   */
		unsigned long length[2];	  /* length in bytes of message   */
		unsigned long offset;		  /* # of unprocessed bytes at	  */
		unsigned int iLenSub=0;
		
	#if 1
		struct	tm *pTime;
	
		pTime = current_time();

		if( (1==iVerbosType || 3==iVerbosType) && (TRUE == verbose) && outfile) /// 2014.08.01
		{
			fprintf(outfile, "----------------------------------------------------------------------------------\r\n" );
			fprintf(outfile, "RipeMD128 Hash is created at %04d/%02d/%02d/%s %02d:%02d:%02d by %s \n", pTime->tm_year, pTime->tm_mon, pTime->tm_mday,  
													WeekTXT[pTime->tm_wday], pTime->tm_hour, pTime->tm_min, pTime->tm_sec, EmailText );
			fprintf(outfile, "----------------------------------------------------------------------------------\r\n" );
		}		
	#endif

		LOG_V("\n");

		if( ASTERISK_FOUND == isAsteMode )  /* For ah.exe --input *.* */
		{
			unsigned __int64 	kll = 0UL;


			printf("RMD128>> RMD128 hashing for input files* \n");
			for(ii=0; ii<RMD128_LENGTH*2; ii++) printf("-");  
			printf(" %d --\r\n", RMD128_LENGTH*2 );

			if(inpfile) { fclose(inpfile); inpfile=NULL; }
			if(data_buf){ free(data_buf); data_buf=NULL; }

			for(ii=iFirst; ii<=(argc-isAsteminus); ii++)
			{
				if( NULL != (inpfile = fopen( argv[ii], "rb")) ) 
				{
					
					memset( inbuf, 0x00, sizeof(inbuf) ); // 2018.06.15
						
					//Initialize the RIPEMD-160 context
					MD128init(MDbuf);
					length[0] = 0;
					length[1] = 0;
					
					kll = 0UL;
					while((nbytes = fread(inbuf, 1, sizeof(inbuf), inpfile)) > 0) 
					{
						kll += nbytes;
					
						/* process all complete blocks */
						for (i=0; i<(nbytes>>6); i++) {
						   for (j=0; j<16; j++)
							  X[j] = BYTES_TO_DWORD(inbuf+64*i+4*j);
						   MD128compress(MDbuf, X);
						}
						/* update length[] */
						if (length[0] + nbytes < length[0])
						   length[1]++; 				 /* overflow to msb of length */
						length[0] += nbytes;
					
						//Digest the message
						// LOG_V("\bRMD128 Hashing for (%s) -> read : %lld Bytes \r", argv[ii], kll );
					}
					ClearScreen(); 
					
					//Finish absorbing phase		
					/* finish: */
					offset = length[0] & 0x3C0;   /* extract bytes 6 to 10 inclusive */
					MD128finish(MDbuf, inbuf+offset, length[0], length[1]);
					
					for (ii=0; ii<RMD128_LENGTH; ii+=4) 
					{
						hashcode[ii]   = (unsigned char)(MDbuf[ii>>2]);
						hashcode[ii+1] = (unsigned char)(MDbuf[ii>>2] >>  8);
						hashcode[ii+2] = (unsigned char)(MDbuf[ii>>2] >> 16);
						hashcode[ii+3] = (unsigned char)(MDbuf[ii>>2] >> 24);
					}
					
					memset( outTxt, 0x00, sizeof(outTxt) );
					for (ii = 0; ii < RMD128_LENGTH; ii++)
					{
						sprintf(&outTxt[ii*2], (char*)"%02x", hashcode[ii]);
					}
	
					if( (2==iVerbosType || 3==iVerbosType) && (TRUE == verbose) )
					{
						printf("%s  *RMD128*%s__(%llu) \r\n", outTxt, argv[ii], kll );
						if(outfile) fprintf(outfile, "%s  *RMD128*%s__(%llu) \r\n", outTxt, argv[ii], kll );
					}
					else
					{
						printf("%s  *RMD128*%s \r\n", outTxt, argv[ii] );
						if(outfile) fprintf(outfile, "%s  *RMD128*%s \r\n", outTxt, argv[ii] );
					}

					if(inpfile) { fclose(inpfile); inpfile=NULL; }
					if(data_buf){ free(data_buf); data_buf=NULL; }

					iTotSize += kll;
					multifileindex ++;
				}
			}
			printf("\nRMD128>> RMD128 Number of Input files : %d files (Total size: %llu Byte) ---", multifileindex, iTotSize );

			AllFilesClosed();
	
		}
		else if( multifileindex > 0 )
		{
		
			if(inpfile) { fclose(inpfile); inpfile=NULL; }
			if(data_buf){ free(data_buf); data_buf=NULL; }

			/* Find first .c file in current directory */
			if( (retFiles = _findfirsti64( extfile_name, &iFiles )) == -1L )
			{
				printf("RMD128>>No file [%s] in current directory!!! [%s] \n", extfile_name, infile_name );
			}
			else
			{

				printf("RMD128>> RMD128 hashing for input files... \n");
				//printf("-------------------------------------------------------------\r\n" );
				for(ii=0; ii<RMD128_LENGTH*2; ii++) printf("-");  
				printf(" %d --\r\n", RMD128_LENGTH*2 );
				
				do {

					iLenSub = strlen(iFiles.name);

					if( iFiles.attrib & _A_SUBDIR )
					{

					}
					else
					{
						if( NULL == (inpfile = fopen( iFiles.name, "rb")) ) 
						{
							beep(700,100);
							printf("\n\n[++ERROR++] Can not open multi-input file (%s) in RMD128. \n", iFiles.name );

							AllFilesClosed();
	
							exit(0);
							return 0;
						}

						memset( inbuf, 0x00, sizeof(inbuf) ); // 2018.06.15
							
						//Initialize the RIPEMD-160 context
						MD128init(MDbuf);
						length[0] = 0;
						length[1] = 0;
						
						kll = 0UL;
						while((nbytes = fread(inbuf, 1, sizeof(inbuf), inpfile)) > 0) 
						{
							kll += nbytes;
						
							/* process all complete blocks */
							for (i=0; i<(nbytes>>6); i++) {
							   for (j=0; j<16; j++)
								  X[j] = BYTES_TO_DWORD(inbuf+64*i+4*j);
							   MD128compress(MDbuf, X);
							}
							/* update length[] */
							if (length[0] + nbytes < length[0])
							   length[1]++; 				 /* overflow to msb of length */
							length[0] += nbytes;
						
							//Digest the message
							// LOG_V("\bRMD128 Hashing for (%s) -> read : %lld Bytes \r", iFiles.name, kll );
						}
						ClearScreen(); 
						
						//Finish absorbing phase		
						/* finish: */
						offset = length[0] & 0x3C0;   /* extract bytes 6 to 10 inclusive */
						MD128finish(MDbuf, inbuf+offset, length[0], length[1]);
						
						for (ii=0; ii<RMD128_LENGTH; ii+=4) 
						{
							hashcode[ii]   = (unsigned char)(MDbuf[ii>>2]);
							hashcode[ii+1] = (unsigned char)(MDbuf[ii>>2] >>  8);
							hashcode[ii+2] = (unsigned char)(MDbuf[ii>>2] >> 16);
							hashcode[ii+3] = (unsigned char)(MDbuf[ii>>2] >> 24);
						}
						
						memset( outTxt, 0x00, sizeof(outTxt) );
						for (ii = 0; ii < RMD128_LENGTH; ii++)
						{
							sprintf(&outTxt[ii*2], (char*)"%02x", hashcode[ii]);
						}

						if( (2==iVerbosType || 3==iVerbosType) && (TRUE == verbose) )
						{
							printf("%s  *RMD128*%s__(%llu) \r\n", outTxt, iFiles.name, iFiles.size  );
							if(outfile) fprintf(outfile, "%s  *RMD128*%s__(%llu) \r\n", outTxt, iFiles.name, iFiles.size	);
						}
						else
						{
							printf("%s  *RMD128*%s \r\n", outTxt, iFiles.name );
							if(outfile) fprintf(outfile, "%s  *RMD128*%s \r\n", outTxt, iFiles.name );
						}

						if(inpfile) { fclose(inpfile); inpfile=NULL; }
						if(data_buf){ free(data_buf); data_buf=NULL; }

						iTotSize += iFiles.size;
						multifileindex ++;
					}

				} while( 0 == _findnexti64( retFiles, &iFiles ) );
				_findclose( retFiles );

				multifileindex--;

				printf("\nRMD128>> Number of Input files : %d files (Total size: %llu Byte) ---", multifileindex, iTotSize );

			}

		}
		else
		{

			printf("RMD128>> RMD128 hashing... \n");

			memset( inbuf, 0x00, sizeof(inbuf) ); // 2018.06.15
				
			//Initialize the RIPEMD-160 context
			MD128init(MDbuf);
			length[0] = 0;
			length[1] = 0;

			kll = 0UL;
			while((nbytes = fread(inbuf, 1, sizeof(inbuf), inpfile)) > 0) 
			{
				kll += nbytes;
		
				/* process all complete blocks */
				for (i=0; i<(nbytes>>6); i++) {
				   for (j=0; j<16; j++)
					  X[j] = BYTES_TO_DWORD(inbuf+64*i+4*j);
				   MD128compress(MDbuf, X);
				}
				/* update length[] */
				if (length[0] + nbytes < length[0])
				   length[1]++; 				 /* overflow to msb of length */
				length[0] += nbytes;

				//Digest the message
				// LOG_V("\bRMD128 Hashing for (%s) -> read : %lld Bytes \r", infile_name, kll );
			}
			ClearScreen(); 

			//Finish absorbing phase		
			/* finish: */
			offset = length[0] & 0x3C0;   /* extract bytes 6 to 10 inclusive */
			MD128finish(MDbuf, inbuf+offset, length[0], length[1]);

			for (ii=0; ii<RMD128_LENGTH; ii+=4) 
			{
				hashcode[ii]   = (unsigned char)(MDbuf[ii>>2]);
				hashcode[ii+1] = (unsigned char)(MDbuf[ii>>2] >>  8);
				hashcode[ii+2] = (unsigned char)(MDbuf[ii>>2] >> 16);
				hashcode[ii+3] = (unsigned char)(MDbuf[ii>>2] >> 24);
			}
		
			memset( outTxt, 0x00, sizeof(outTxt) );
			for (ii = 0; ii < RMD128_LENGTH; ii++)
			{
				sprintf(&outTxt[ii*2], (char*)"%02x", hashcode[ii]);
			}
			
			if( (2==iVerbosType || 3==iVerbosType) && (TRUE == verbose) )
			{
				printf("%s  *RMD128*%s__(%llu) \r\n", outTxt, infile_name, infile_size   );
				if(outfile) fprintf(outfile, "%s  *RMD128*%s__(%llu) \r\n", outTxt, infile_name, infile_size );
			}
			else
			{
				printf("%s  *RMD128*%s \r\n", outTxt, infile_name );
				if(outfile) fprintf(outfile, "%s  *RMD128*%s \r\n", outTxt, infile_name );
			}
			
			if(inpfile) { fclose(inpfile); inpfile=NULL; }
			if(data_buf){ free(data_buf); data_buf=NULL; }
			printf("\nRMD128>> Calculated RMD128 Hash Value - OK" );

		}
	}
	else if(1 == isRipeMD160)
	{
		
#define RMD160_INBUF_SIZ 		(1024) // NOT NOT NEVER modify~~~
#define RMD160_BUF_LEN 			5 // 160/32
#define RMD160_LENGTH 			20 // 160/8

		unsigned char inbuf[RMD160_INBUF_SIZ];
		char outTxt[RMD160_INBUF_SIZ*2]={0,}; 
		
		unsigned long MDbuf[RMD160_BUF_LEN];   /* contains (A, B, C, D(, E))   */
		unsigned char hashcode[RMD160_LENGTH]; /* for final hash-value		   */
		
		unsigned __int64	kll;
		size_t		nbytes; // ll;
		unsigned long		  X[16];		  /* current 16-word chunk	   */
		unsigned int  i, j; 		  /* counters					   */
		unsigned long length[2];	  /* length in bytes of message   */
		unsigned long offset;		  /* # of unprocessed bytes at	  */
		unsigned int iLenSub=0;
		
	#if 1
		struct	tm *pTime;
	
		pTime = current_time();

		if( (1==iVerbosType || 3==iVerbosType) && (TRUE == verbose) && outfile) /// 2014.08.01
		{
			fprintf(outfile, "----------------------------------------------------------------------------------\r\n" );
			fprintf(outfile, "RipeMD160 Hash is created at %04d/%02d/%02d/%s %02d:%02d:%02d by %s \n", pTime->tm_year, pTime->tm_mon, pTime->tm_mday,  
													WeekTXT[pTime->tm_wday], pTime->tm_hour, pTime->tm_min, pTime->tm_sec, EmailText );
			fprintf(outfile, "----------------------------------------------------------------------------------\r\n" );
		}		
	#endif

		LOG_V("\n");

		if( ASTERISK_FOUND == isAsteMode )  /* For ah.exe --input *.* */
		{
			unsigned __int64 	kll = 0UL;


			printf("RipeMD160>> RipeMD160 hashing for input files* \n");
			for(ii=0; ii<RMD160_LENGTH*2; ii++) 
				printf("-");  
			printf(" %d --\r\n", RMD160_LENGTH*2 );

			if(inpfile) { fclose(inpfile); inpfile=NULL; }
			if(data_buf){ free(data_buf); data_buf=NULL; }

			for(ii=iFirst; ii<=(argc-isAsteminus); ii++)
			{
				if( NULL != (inpfile = fopen( argv[ii], "rb")) ) 
				{
					
					memset( inbuf, 0x00, sizeof(inbuf) ); // 2018.06.15
						
					//Initialize the RIPEMD-160 context
					MD160init(MDbuf);
					length[0] = 0;
					length[1] = 0;
					
					kll = 0UL;
					while((nbytes = fread(inbuf, 1, sizeof(inbuf), inpfile)) > 0) 
					{
						kll += nbytes;
					
						/* process all complete blocks */
						for (i=0; i<(nbytes>>6); i++) {
						   for (j=0; j<16; j++)
							  X[j] = BYTES_TO_DWORD(inbuf+64*i+4*j);
						   MD160compress(MDbuf, X);
						}
						/* update length[] */
						if (length[0] + nbytes < length[0])
						   length[1]++; 				 /* overflow to msb of length */
						length[0] += nbytes;
					
						//Digest the message
						// LOG_V("\bRipeMD160 Hashing for (%s) -> read : %lld Bytes \r", argv[ii], kll );
					}
					ClearScreen(); 
					
					//Finish absorbing phase		
					/* finish: */
					offset = length[0] & 0x3C0;   /* extract bytes 6 to 10 inclusive */
					MD160finish(MDbuf, inbuf+offset, length[0], length[1]);
					
					for (ii=0; ii<RMD160_LENGTH; ii+=4) 
					{
						hashcode[ii]   = (unsigned char)(MDbuf[ii>>2]);
						hashcode[ii+1] = (unsigned char)(MDbuf[ii>>2] >>  8);
						hashcode[ii+2] = (unsigned char)(MDbuf[ii>>2] >> 16);
						hashcode[ii+3] = (unsigned char)(MDbuf[ii>>2] >> 24);
					}
					
					memset( outTxt, 0x00, sizeof(outTxt) );
					for (ii = 0; ii < RMD160_LENGTH; ii++)
					{
						sprintf(&outTxt[ii*2], (char*)"%02x", hashcode[ii]);
					}
	
					if( (2==iVerbosType || 3==iVerbosType) && (TRUE == verbose) )
					{
						printf("%s  *RMD160*%s__(%llu) \r\n", outTxt, argv[ii], kll );
						if(outfile) fprintf(outfile, "%s  *RMD160*%s__(%llu) \r\n", outTxt, argv[ii], kll );
					}
					else
					{
						printf("%s  *RMD160*%s \r\n", outTxt, argv[ii] );
						if(outfile) fprintf(outfile, "%s  *RMD160*%s \r\n", outTxt, argv[ii] );
					}

					if(inpfile) { fclose(inpfile); inpfile=NULL; }
					if(data_buf){ free(data_buf); data_buf=NULL; }

					iTotSize += kll;
					multifileindex ++;
				}
			}
			printf("\nRipeMD160>> RipeMD160 Number of Input files : %d files (Total size: %llu Byte) ---", multifileindex, iTotSize );

			AllFilesClosed();
	
		}
		else if( multifileindex > 0 )
		{
		
			if(inpfile) { fclose(inpfile); inpfile=NULL; }
			if(data_buf){ free(data_buf); data_buf=NULL; }

			/* Find first .c file in current directory */
			if( (retFiles = _findfirsti64( extfile_name, &iFiles )) == -1L )
			{
				printf("RipeMD160>>No file [%s] in current directory!!! [%s] \n", extfile_name, infile_name );
			}
			else
			{

				printf("RipeMD160>> RipeMD160 hashing for input files... \n");
				//printf("-------------------------------------------------------------\r\n" );
				for(ii=0; ii<RMD160_LENGTH*2; ii++) printf("-");  
				printf(" %d --\r\n", RMD160_LENGTH*2 );
				
				do {

					iLenSub = strlen(iFiles.name);

					if( iFiles.attrib & _A_SUBDIR )
					{

					}
					else
					{
						if( NULL == (inpfile = fopen( iFiles.name, "rb")) ) 
						{
							beep(700,100);
							printf("\n\n[++ERROR++] Can not open multi-input file (%s) in SHAKE256. \n", iFiles.name );

							AllFilesClosed();
	
							exit(0);
							return 0;
						}

						memset( inbuf, 0x00, sizeof(inbuf) ); // 2018.06.15
							
						//Initialize the RIPEMD-160 context
						MD160init(MDbuf);
						length[0] = 0;
						length[1] = 0;
						
						kll = 0UL;
						while((nbytes = fread(inbuf, 1, sizeof(inbuf), inpfile)) > 0) 
						{
							kll += nbytes;
						
							/* process all complete blocks */
							for (i=0; i<(nbytes>>6); i++) {
							   for (j=0; j<16; j++)
								  X[j] = BYTES_TO_DWORD(inbuf+64*i+4*j);
							   MD160compress(MDbuf, X);
							}
							/* update length[] */
							if (length[0] + nbytes < length[0])
							   length[1]++; 				 /* overflow to msb of length */
							length[0] += nbytes;
						
							//Digest the message
							// LOG_V("\bRipeMD160 Hashing for (%s) -> read : %lld Bytes \r", iFiles.name, kll );
						}
						ClearScreen(); 
						
						//Finish absorbing phase		
						/* finish: */
						offset = length[0] & 0x3C0;   /* extract bytes 6 to 10 inclusive */
						MD160finish(MDbuf, inbuf+offset, length[0], length[1]);
						
						for (ii=0; ii<RMD160_LENGTH; ii+=4) 
						{
							hashcode[ii]   = (unsigned char)(MDbuf[ii>>2]);
							hashcode[ii+1] = (unsigned char)(MDbuf[ii>>2] >>  8);
							hashcode[ii+2] = (unsigned char)(MDbuf[ii>>2] >> 16);
							hashcode[ii+3] = (unsigned char)(MDbuf[ii>>2] >> 24);
						}
						
						memset( outTxt, 0x00, sizeof(outTxt) );
						for (ii = 0; ii < RMD160_LENGTH; ii++)
						{
							sprintf(&outTxt[ii*2], (char*)"%02x", hashcode[ii]);
						}

						if( (2==iVerbosType || 3==iVerbosType) && (TRUE == verbose) )
						{
							printf("%s  *RMD160*%s__(%llu) \r\n", outTxt, iFiles.name, iFiles.size  );
							if(outfile) fprintf(outfile, "%s  *RMD160*%s__(%llu) \r\n", outTxt, iFiles.name, iFiles.size	);
						}
						else
						{
							printf("%s  *RMD160*%s \r\n", outTxt, iFiles.name );
							if(outfile) fprintf(outfile, "%s  *RMD160*%s \r\n", outTxt, iFiles.name );
						}

						if(inpfile) { fclose(inpfile); inpfile=NULL; }
						if(data_buf){ free(data_buf); data_buf=NULL; }

						iTotSize += iFiles.size;
						multifileindex ++;
					}

				} while( 0 == _findnexti64( retFiles, &iFiles ) );
				_findclose( retFiles );

				multifileindex--;

				printf("\nRipeMD160>> Number of Input files : %d files (Total size: %llu Byte) ---", multifileindex, iTotSize );

			}

		}
		else
		{

			printf("RipeMD160>> RipeMD160 hashing... \n");

			memset( inbuf, 0x00, sizeof(inbuf) ); // 2018.06.15
				
			//Initialize the RIPEMD-160 context
			MD160init(MDbuf);
			length[0] = 0;
			length[1] = 0;

			kll = 0UL;
			while((nbytes = fread(inbuf, 1, sizeof(inbuf), inpfile)) > 0) 
			{
				kll += nbytes;
		
				/* process all complete blocks */
				for (i=0; i<(nbytes>>6); i++) {
				   for (j=0; j<16; j++)
					  X[j] = BYTES_TO_DWORD(inbuf+64*i+4*j);
				   MD160compress(MDbuf, X);
				}
				/* update length[] */
				if (length[0] + nbytes < length[0])
				   length[1]++; 				 /* overflow to msb of length */
				length[0] += nbytes;

				//Digest the message
				// LOG_V("\bRipeMD160 Hashing for (%s) -> read : %lld Bytes \r", infile_name, kll );
			}
			ClearScreen(); 

			//Finish absorbing phase		
			/* finish: */
			offset = length[0] & 0x3C0;   /* extract bytes 6 to 10 inclusive */
			MD160finish(MDbuf, inbuf+offset, length[0], length[1]);

			for (ii=0; ii<RMD160_LENGTH; ii+=4) 
			{
				hashcode[ii]   = (unsigned char)(MDbuf[ii>>2]);
				hashcode[ii+1] = (unsigned char)(MDbuf[ii>>2] >>  8);
				hashcode[ii+2] = (unsigned char)(MDbuf[ii>>2] >> 16);
				hashcode[ii+3] = (unsigned char)(MDbuf[ii>>2] >> 24);
			}
		
			memset( outTxt, 0x00, sizeof(outTxt) );
			for (ii = 0; ii < RMD160_LENGTH; ii++)
			{
				sprintf(&outTxt[ii*2], (char*)"%02x", hashcode[ii]);
			}
			
			if( (2==iVerbosType || 3==iVerbosType) && (TRUE == verbose) )
			{
				printf("%s  *RMD160*%s__(%llu) \r\n", outTxt, infile_name, infile_size   );
				if(outfile) fprintf(outfile, "%s  *RMD160*%s__(%llu) \r\n", outTxt, infile_name, infile_size );
			}
			else
			{
				printf("%s  *RMD160*%s \r\n", outTxt, infile_name );
				if(outfile) fprintf(outfile, "%s  *RMD160*%s \r\n", outTxt, infile_name );
			}
			
			if(inpfile) { fclose(inpfile); inpfile=NULL; }
			if(data_buf){ free(data_buf); data_buf=NULL; }
			printf("\nRipeMD160>> Calculated RipeMD160 Hash Value - OK" );

		}
	}

#if BLAKE_224_256_384_512_HASH
	else if(1 == isBlake224)
	{
		unsigned char inbuf[BLOCK224], out[BLAKE224_LEN]={0,}; 
		char outTxt[BLAKE224_LEN*2]={0,}; 
		state224 blakeS; 
		unsigned __int64 kll;
		size_t		ll=0;
		unsigned int iLenSub=0;
		struct	tm *pTime = NULL;
	
		pTime = current_time();

		if( (1==iVerbosType || 3==iVerbosType) && (TRUE == verbose) && outfile) /// 2014.08.01
		{
			fprintf(outfile, "----------------------------------------------------------------------------------\r\n" );
			fprintf(outfile, "BLAKE224 Hash is created at %04d/%02d/%02d/%s %02d:%02d:%02d by %s \n", pTime->tm_year, pTime->tm_mon, pTime->tm_mday,  
													WeekTXT[pTime->tm_wday], pTime->tm_hour, pTime->tm_min, pTime->tm_sec, EmailText );
			fprintf(outfile, "----------------------------------------------------------------------------------\r\n" );
		}		

		LOG_V("\n");

		if( ASTERISK_FOUND == isAsteMode )	/* For ah.exe --input *.* */
		{
			//unsigned __int64 	kll = 0UL;

			printf("BLAKE224>> BLAKE224 hashing for input files* (%d) \n", argc );

		#if 0
			//for(ii=0; ii<=(argc-isAsteminus); ii++) printf("%3d : [%s] (%d:%d)\n", ii, argv[ii], isAsteminus, iFirst );

					
			for(ii=0; ii<BLAKE224_LEN*2; ii++) printf("-");  printf(" %d --\r\n", BLAKE224_LEN*2 );

			if(inpfile) { fclose(inpfile); inpfile=NULL; }
			if(data_buf){ free(data_buf); data_buf=NULL; }

			for(ii=iFirst; ii<=(argc-isAsteminus) ; ii++)
			{
				if( NULL != (inpfile = fopen( argv[ii], "rb")) ) 
				{
					memset( inbuf, 0x00, sizeof(inbuf) );
					memset( out, 0x00, sizeof(out) );
					
					//Initialize the BLAKE-224 context
					blake224_init( &blakeS ); 

					kll = 0ULL;
					while((ll = fread(inbuf, 1, sizeof(inbuf), inpfile)) > 0) 
					{
						kll += ll;
					
						//Absorb input data
						blake224_update( &blakeS, inbuf, ll ); 
					
						//Digest the message
						printf("\bBLAKE224 Hashing for (%s) -> read : %lld Bytes \r", argv[ii], kll );
					}
					ClearScreen(); 
					
					//Finish absorbing phase
					blake224_final( &blakeS, out ); 

					memset( outTxt, 0x00, sizeof(outTxt) );
					
					for (ii = 0; ii < BLAKE224_LEN; ii++) 
					{
						if( iUpper )
							sprintf(&outTxt[ii*2], (char*)"%02X", out[ii]);
						else
							sprintf(&outTxt[ii*2], (char*)"%02x", out[ii]);
					}

					if( (2==iVerbosType || 3==iVerbosType) && (TRUE == verbose) )
					{
						printf("%s  *BLAKE224*%s__(%llu) \r\n", outTxt, argv[ii], kll );
						if(outfile) fprintf(outfile, "%s  *BLAKE224*%s__(%llu) \r\n", outTxt, argv[ii], kll );
					}
					else
					{
						printf("%s  *BLAKE224*%s \r\n", outTxt, argv[ii] );
						if(outfile) fprintf(outfile, "%s  *BLAKE224*%s \r\n", outTxt, argv[ii] );
					}
					
					if(inpfile) { fclose(inpfile); inpfile=NULL; }
					if(data_buf){ free(data_buf); data_buf=NULL; }

					iTotSize += kll;
					multifileindex ++;
				}
				else
				{
					printf("==NOT-OPEN== [%s] === \n", argv[ii] );
				}
			}
			printf("\nBLAKE224>> BLAKE224 Number of Input files : %d files (Total size: %llu Byte) ---", multifileindex, iTotSize );

		#endif
		
			AllFilesClosed();
	
		}
		else if( multifileindex > 0 )
		{

			if(inpfile) { fclose(inpfile); inpfile=NULL; }
			if(data_buf){ free(data_buf); data_buf=NULL; }

			/* Find first .c file in current directory */
			if( (retFiles = _findfirsti64( extfile_name, &iFiles )) == -1L )
			{
				printf("BLAKE224>>No file [%s] in current directory!!! [%s] \n", extfile_name, infile_name );
			}
			else
			{

				printf("BLAKE224>> BLAKE224 hashing for input files... \n");
				//printf("-------------------------------------------------------------\r\n" );
				for(ii=0; ii<BLAKE224_LEN*2; ii++) printf("-");  
				printf(" %d --\r\n", BLAKE224_LEN*2 );

				do {

					iLenSub = strlen(iFiles.name);

					if( iFiles.attrib & _A_SUBDIR )
					{

					}
					else
					{
						if( NULL == (inpfile = fopen( iFiles.name, "rb")) ) 
						{
							beep(700,100);
							printf("\n\n[++ERROR++] Can not open multi-input file (%s) in BLAKE224. \n", iFiles.name );

							AllFilesClosed();
	
							exit(0);
							return 0;
						}

						memset( inbuf, 0x00, sizeof(inbuf) );
						memset( out, 0x00, sizeof(out) );
						
						//Initialize the BLAKE-224 context
						blake224_init( &blakeS ); 
						
						kll = 0ULL;
						while((ll = fread(inbuf, 1, sizeof(inbuf), inpfile)) > 0) 
						{
							kll += ll;
						
							//Absorb input data
							blake224_update( &blakeS, inbuf, ll ); 
						
							//Digest the message
							// LOG_V("\bBLAKE224 Hashing for (%s) -> read : %lld Bytes \r", iFiles.name, kll );
						}
						ClearScreen(); 
						
						//Finish absorbing phase
						blake224_final( &blakeS, out ); 
						
						memset( outTxt, 0x00, sizeof(outTxt) );
						
						for (ii = 0; ii < BLAKE224_LEN; ii++) 
						{
							if( iUpper )
								sprintf(&outTxt[ii*2], (char*)"%02X", out[ii]);
							else
								sprintf(&outTxt[ii*2], (char*)"%02x", out[ii]);
						}

						
						if( (2==iVerbosType || 3==iVerbosType) && (TRUE == verbose) )
						{
							printf("%s  *%s*%s__(%llu) \r\n", outTxt, str_hash, iFiles.name, iFiles.size   );
							if(outfile) fprintf(outfile, "%s  *%s*%s__(%llu) \r\n", outTxt, str_hash, iFiles.name, iFiles.size );
						}
						else
						{
							printf("%s  *%s*%s \r\n", outTxt, str_hash, iFiles.name );
							if(outfile) fprintf(outfile, "%s  *%s*%s \r\n", outTxt, str_hash, iFiles.name );
						}
						
						if(inpfile) { fclose(inpfile); inpfile=NULL; }
						if(data_buf){ free(data_buf); data_buf=NULL; }


						iTotSize += iFiles.size;
						multifileindex ++;
					}

				} while( 0 == _findnexti64( retFiles, &iFiles ) );
				_findclose( retFiles );

				multifileindex--;

				printf("\nBLAKE224>> Number of Input files : %d files (Total size: %llu Byte) ---", multifileindex, iTotSize );

			}
		
		}
		else
		{
			//unsigned char inbuf[BLOCK224], out[BLAKE224_LEN]={0,}; 
			//char outTxt[BLAKE224_LEN*2]={0,}; 
			//state224 S; 
			//unsigned __int64 kll;
			//size_t 		ll;


			printf("BLAKE224>> BLAKE224 hashing... \n");

			memset( inbuf, 0x00, sizeof(inbuf) );
			memset( out, 0x00, sizeof(out) );

			//Initialize the BLAKE-224 context
			blake224_init( &blakeS ); 

			kll = 0ULL;
			while((ll = fread(inbuf, 1, sizeof(inbuf), inpfile)) > 0) 
			{
				kll += ll;

				//Absorb input data
				blake224_update( &blakeS, inbuf, ll ); 

				//Digest the message
				// LOG_V("\bBLAKE224 Hashing for (%s) -> read : %lld Bytes \r", infile_name, kll );
			}
			ClearScreen(); 

			//Finish absorbing phase
			blake224_final( &blakeS, out ); 


			memset( outTxt, 0x00, sizeof(outTxt) );
			for (ii = 0; ii < BLAKE224_LEN; ii++) 
			{
				if( iUpper )
					sprintf(&outTxt[ii*2], (char*)"%02X", out[ii]);
				else
					sprintf(&outTxt[ii*2], (char*)"%02x", out[ii]);
			}
			
			if( (2==iVerbosType || 3==iVerbosType) && (TRUE == verbose) )
			{
				printf("%s  *%s*%s__(%llu) \r\n", outTxt, str_hash, infile_name, infile_size   );
				if(outfile) fprintf(outfile, "%s  *%s*%s__(%llu) \r\n", outTxt, str_hash, infile_name, infile_size );
			}
			else
			{
				printf("%s  *%s*%s \r\n", outTxt, str_hash, infile_name );
				if(outfile) fprintf(outfile, "%s  *%s*%s \r\n", outTxt, str_hash, infile_name );
			}
			
			if(inpfile) { fclose(inpfile); inpfile=NULL; }
			if(data_buf){ free(data_buf); data_buf=NULL; }
			printf("\nBLAKE224>> Calculated BLAKE224 Hash Value - OK" );

		}
	}
	else if(1 == isBlake256)
	{
		unsigned char inbuf[BLOCK256], out[BLAKE256_LEN]={0,}; 
		char outTxt[BLAKE256_LEN*2]={0,}; 
		state256 blakeS; 	
		unsigned __int64  kll;
		size_t		ll;

		unsigned int iLenSub=0;
		struct	tm *pTime = NULL;
	
		pTime = current_time();

		if( (1==iVerbosType || 3==iVerbosType) && (TRUE == verbose) && outfile) /// 2014.08.01
		{
			fprintf(outfile, "----------------------------------------------------------------------------------\r\n" );
			fprintf(outfile, "BLAKE256 Hash is created at %04d/%02d/%02d/%s %02d:%02d:%02d by %s \n", pTime->tm_year, pTime->tm_mon, pTime->tm_mday,  
													WeekTXT[pTime->tm_wday], pTime->tm_hour, pTime->tm_min, pTime->tm_sec, EmailText );
			fprintf(outfile, "----------------------------------------------------------------------------------\r\n" );
		}		

		LOG_V("\n");

		if( ASTERISK_FOUND == isAsteMode )	/* For ah.exe --input *.* */
		{
			//unsigned __int64 	kll = 0UL;

			printf("BLAKE256>> BLAKE256 hashing for input files* (%d) \n", argc );


		#if 0
			//for(ii=0; ii<=(argc-isAsteminus); ii++) printf("%3d : [%s] (%d:%d)\n", ii, argv[ii], isAsteminus, iFirst );

					
			for(ii=0; ii<BLAKE256_LEN*2; ii++) printf("-");  printf(" %d --\r\n", BLAKE256_LEN*2 );

			if(inpfile) { fclose(inpfile); inpfile=NULL; }
			if(data_buf){ free(data_buf); data_buf=NULL; }

			for(ii=iFirst; ii<=(argc-isAsteminus) ; ii++)
			{
				//if( 0==isFileExist( (const char *)argv[ii], 0 ) ) continue;
				if( NULL != (inpfile = fopen( argv[ii], "rb")) ) 
				{

					memset( inbuf, 0x00, sizeof(inbuf) );
					memset( out, 0x00, sizeof(out) );

					//Initialize the BLAKE-256 context
					blake256_init( &blakeS ); 
					
					kll = 0ULL;
					while((ll = fread(inbuf, 1, sizeof(inbuf), inpfile)) > 0) 
					{
						kll += ll;
					
						//Absorb input data
						blake256_update( &blakeS, inbuf, ll ); 
					
						//Digest the message
						printf("\bBLAKE256 Hashing for (%s) -> read : %lld Bytes \r", argv[ii], kll );
					}
					ClearScreen(); 
					
					//Finish absorbing phase
					blake256_final( &blakeS, out ); 

					memset( outTxt, 0x00, sizeof(outTxt) );
					for (ii = 0; ii < BLAKE256_LEN; ii++)
					{
						if( iUpper )
							sprintf(&outTxt[ii*2], (char*)"%02X", out[ii]);
						else
							sprintf(&outTxt[ii*2], (char*)"%02x", out[ii]);
					}

					if( (2==iVerbosType || 3==iVerbosType) && (TRUE == verbose) )
					{
						printf("%s  *BLAKE256*%s__(%llu) \r\n", outTxt, argv[ii], kll );
						if(outfile) fprintf(outfile, "%s  *BLAKE256*%s__(%llu) \r\n", outTxt, argv[ii], kll );
					}
					else
					{
						printf("%s  *BLAKE256*%s \r\n", outTxt, argv[ii] );
						if(outfile) fprintf(outfile, "%s  *BLAKE256*%s \r\n", outTxt, argv[ii] );
					}
					
					if(inpfile) { fclose(inpfile); inpfile=NULL; }
					if(data_buf){ free(data_buf); data_buf=NULL; }
					
					iTotSize += iFiles.size;
					multifileindex ++;

				}
				else
				{
					printf("==NOT-OPEN== [%s] === \n", argv[ii] );
				}
			}
			printf("\nBLAKE224>> BLAKE256 Number of Input files : %d files (Total size: %llu Byte) ---", multifileindex, iTotSize );
		#endif
			AllFilesClosed();
	
		}
		else if( multifileindex > 0 )
		{

			if(inpfile) { fclose(inpfile); inpfile=NULL; }
			if(data_buf){ free(data_buf); data_buf=NULL; }

			/* Find first .c file in current directory */
			if( (retFiles = _findfirsti64( extfile_name, &iFiles )) == -1L )
			{
				printf("BLAKE224>>No file [%s] in current directory!!! [%s] \n", extfile_name, infile_name );
			}
			else
			{

				printf("BLAKE256>> BLAKE256 hashing for input files... \n");
				//printf("-------------------------------------------------------------\r\n" );
				for(ii=0; ii<BLAKE256_LEN*2; ii++) printf("-");  
				printf(" %d --\r\n", BLAKE256_LEN*2 );
				
				do {

					iLenSub = strlen(iFiles.name);

					if( iFiles.attrib & _A_SUBDIR )
					{

					}
					else
					{
						if( NULL == (inpfile = fopen( iFiles.name, "rb")) ) 
						{
							beep(700,100);
							printf("\n\n[++ERROR++] Can not open multi-input file (%s) in BLAKE256. \n", iFiles.name );

							AllFilesClosed();
	
							exit(0);
							return 0;
						}

						memset( inbuf, 0x00, sizeof(inbuf) );
						memset( out, 0x00, sizeof(out) );


						//Initialize the BLAKE-256 context
					    blake256_init( &blakeS ); 
						
						kll = 0ULL;
						while((ll = fread(inbuf, 1, sizeof(inbuf), inpfile)) > 0) 
						{
							kll += ll;

							//Absorb input data
							blake256_update( &blakeS, inbuf, ll ); 

							//Digest the message
							// LOG_V("\bBLAKE256 Hashing for (%s) -> read : %lld Bytes \r", iFiles.name, kll );
						}
						ClearScreen(); 

						//Finish absorbing phase
					    blake256_final( &blakeS, out ); 
						

						memset( outTxt, 0x00, sizeof(outTxt) );
						for (ii = 0; ii < BLAKE256_LEN; ii++)
						{
							if( iUpper )
								sprintf(&outTxt[ii*2], (char*)"%02X", out[ii]);
							else
								sprintf(&outTxt[ii*2], (char*)"%02x", out[ii]);
						}
	
						
						if( (2==iVerbosType || 3==iVerbosType) && (TRUE == verbose) )
						{
							printf("%s  *%s*%s__(%llu) \r\n", outTxt, str_hash, iFiles.name, iFiles.size   );
							if(outfile) fprintf(outfile, "%s  *%s*%s__(%llu) \r\n", outTxt, str_hash, iFiles.name, iFiles.size );
						}
						else
						{
							printf("%s  *%s*%s \r\n", outTxt, str_hash, iFiles.name );
							if(outfile) fprintf(outfile, "%s  *%s*%s \r\n", outTxt, str_hash, iFiles.name );
						}
						
						if(inpfile) { fclose(inpfile); inpfile=NULL; }
						if(data_buf){ free(data_buf); data_buf=NULL; }


						iTotSize += iFiles.size;
						multifileindex ++;
					}

				} while( 0 == _findnexti64( retFiles, &iFiles ) );
				_findclose( retFiles );

				multifileindex--;

				printf("\nBLAKE256>> Number of Input files : %d files (Total size: %llu Byte) ---", multifileindex, iTotSize );

			}
		
		}
		else
		{

			printf("BLAKE256>> BLAKE256 hashing... \n");

			memset( inbuf, 0x00, sizeof(inbuf) );
			memset( out, 0x00, sizeof(out) );
			
			
			//Initialize the BLAKE-256 context
			blake256_init( &blakeS ); 
			
			kll = 0ULL;
			while((ll = fread(inbuf, 1, sizeof(inbuf), inpfile)) > 0) 
			{
				kll += ll;
			
				//Absorb input data
				blake256_update( &blakeS, inbuf, ll ); 
			
				//Digest the message
				// LOG_V("\bBLAKE256 Hashing for (%s) -> read : %lld Bytes \r", iFiles.name, kll );
			}
			ClearScreen(); 

			//Finish absorbing phase
			blake256_final( &blakeS, out ); 

			memset( outTxt, 0x00, sizeof(outTxt) );
			for (ii = 0; ii < BLAKE256_LEN; ii++)
			{
				if( iUpper )
					sprintf(&outTxt[ii*2], (char*)"%02X", out[ii]);
				else
					sprintf(&outTxt[ii*2], (char*)"%02x", out[ii]);
			}

			if( (2==iVerbosType || 3==iVerbosType) && (TRUE == verbose) )
			{
				printf("%s  *%s*%s__(%llu) \r\n", outTxt, str_hash, infile_name, infile_size );
				if(outfile) fprintf(outfile, "%s  *%s*%s__(%llu) \r\n", outTxt, str_hash, infile_name, infile_size );
			}
			else
			{
				printf("%s  *%s*%s \r\n", outTxt, str_hash, infile_name );
				if(outfile) fprintf(outfile, "%s  *%s*%s \r\n", outTxt, str_hash, infile_name );
			}

			if(inpfile) { fclose(inpfile); inpfile=NULL; }
			if(data_buf){ free(data_buf); data_buf=NULL; }
			printf("\nBLAKE256>> Calculated BLAKE256 Hash Value - OK" );

		}
	}
	else if(1 == isBlake384)
	{
		unsigned char inbuf[BLOCK384], out[BLAKE384_LEN] ={0,}; 
		char outTxt[BLAKE384_LEN*2]={0,}; 
		state384 blakeS; 		
		unsigned __int64  kll;
		size_t		ll;
		struct	tm *pTime = NULL;
		unsigned int iLenSub = 0;


		pTime = current_time();
	
		if( (1==iVerbosType || 3==iVerbosType) && (TRUE == verbose) && outfile) /// 2014.08.01
		{
			fprintf(outfile, "----------------------------------------------------------------------------------\r\n" );
			fprintf(outfile, "BLAKE384 Hash is created at %04d/%02d/%02d/%s %02d:%02d:%02d by %s \n", pTime->tm_year, pTime->tm_mon, pTime->tm_mday,  
													WeekTXT[pTime->tm_wday], pTime->tm_hour, pTime->tm_min, pTime->tm_sec, EmailText );
			fprintf(outfile, "----------------------------------------------------------------------------------\r\n" );
		}		
	
		LOG_V("\n");
	
		if( ASTERISK_FOUND == isAsteMode )	/* For ah.exe --input *.* */
		{
			//unsigned __int64	kll = 0UL;
	
			printf("BLAKE384>> BLAKE384 hashing for input files* (%d) \n", argc );

		#if 0
			//for(ii=0; ii<=(argc-isAsteminus); ii++) printf("%3d : [%s] (%d:%d)\n", ii, argv[ii], isAsteminus, iFirst );
	
					
			for(ii=0; ii<BLAKE384_LEN*2; ii++) printf("-");  printf(" %d --\r\n", BLAKE384_LEN*2 );
	
			if(inpfile) { fclose(inpfile); inpfile=NULL; }
			if(data_buf){ free(data_buf); data_buf=NULL; }
	
			for(ii=iFirst; ii<=(argc-isAsteminus) ; ii++)
			{

				printf("FILE - [%s] : ", argv[ii] );
				if( isFileExist( argv[ii] ) ) printf(" -- OK \n");
				else printf(" --- NONE ----------------- \n");


				if( (inpfile = fopen( argv[ii], "rb")) != NULL ) 
				{
					
					memset( inbuf, 0x00, sizeof(inbuf) );
					memset( out, 0x00, sizeof(out) );
					
					//Initialize the BLAKE-384 context
					blake384_init( &blakeS ); 
					
					kll = 0ULL;
					while((ll = fread(inbuf, 1, sizeof(inbuf), inpfile)) > 0) 
					{
						kll += ll;
					
						//Absorb input data
						blake384_update( &blakeS, inbuf, ll ); 
					
						//Digest the message
						printf("\bBLAKE384 Hashing for (%s) -> read : %lld Bytes \r", argv[ii], kll );
					}
					ClearScreen(); 
					
					//Finish absorbing phase
					blake384_final( &blakeS, out ); 
					
					
					memset( outTxt, 0x00, sizeof(outTxt) );
					for (ii = 0; ii < BLAKE384_LEN; ii++)
					{
						if( iUpper )
							sprintf(&outTxt[ii*2], (char*)"%02X", out[ii]);
						else
							sprintf(&outTxt[ii*2], (char*)"%02x", out[ii]);
					}
					
					
					if( (2==iVerbosType || 3==iVerbosType) && (TRUE == verbose) )
					{
						printf("%s  *BLAKE384*%s__(%llu) \r\n", outTxt, argv[ii], kll );
						if(outfile) fprintf(outfile, "%s  *BLAKE384*%s__(%llu) \r\n", outTxt, argv[ii], kll );
					}
					else
					{
						printf("%s  *BLAKE384*%s \r\n", outTxt, argv[ii] );
						if(outfile) fprintf(outfile, "%s  *BLAKE384*%s \r\n", outTxt, argv[ii] );
					}
					
					if(inpfile) { fclose(inpfile); inpfile=NULL; }
					if(data_buf){ free(data_buf); data_buf=NULL; }
					
					iTotSize += iFiles.size;
					multifileindex ++;
	
				}

				
			}
			printf("\nBLAKE384>> BLAKE384 Number of Input files : %d files (Total size: %llu Byte) ---", multifileindex, iTotSize );
		#endif
			AllFilesClosed();
	
		}
		else if( multifileindex > 0 )
		{

			if(inpfile) { fclose(inpfile); inpfile=NULL; }
			if(data_buf){ free(data_buf); data_buf=NULL; }
	
			/* Find first .c file in current directory */
			if( (retFiles = _findfirsti64( extfile_name, &iFiles )) == -1L )
			{
				printf("BLAKE384>>No file [%s] in current directory!!! [%s] \n", extfile_name, infile_name );
			}
			else
			{
	
				printf("BLAKE384>> BLAKE384 hashing for input files... \n");
				//printf("-------------------------------------------------------------\r\n" );
				for(ii=0; ii<BLAKE384_LEN*2; ii++) printf("-");  
				printf(" %d --\r\n", BLAKE384_LEN*2 );
				
				do {
	
					iLenSub = strlen(iFiles.name);
	
					if( iFiles.attrib & _A_SUBDIR )
					{
	
					}
					else
					{
						if( NULL == (inpfile = fopen( iFiles.name, "rb")) ) 
						{
							beep(700,100);
							printf("\n\n[++ERROR++] Can not open multi-input file (%s) in BLAKE384. \n", iFiles.name );
	
							AllFilesClosed();
	
							exit(0);
							return 0;
						}

						memset( inbuf, 0x00, sizeof(inbuf) );
						memset( out, 0x00, sizeof(out) );
						
						//Initialize the BLAKE-384 context
						blake384_init( &blakeS ); 
						
						kll = 0ULL;
						while((ll = fread(inbuf, 1, sizeof(inbuf), inpfile)) > 0) 
						{
							kll += ll;
						
							//Absorb input data
							blake384_update( &blakeS, inbuf, ll ); 
						
							//Digest the message
							// LOG_V("\bBLAKE384 Hashing for (%s) -> read : %lld Bytes \r", iFiles.name, kll );
						}
						ClearScreen(); 
						
						//Finish absorbing phase
						blake384_final( &blakeS, out ); 

						memset( outTxt, 0x00, sizeof(outTxt) );
						for (ii = 0; ii < BLAKE384_LEN; ii++)
						{
							if( iUpper )
								sprintf(&outTxt[ii*2], (char*)"%02X", out[ii]);
							else
								sprintf(&outTxt[ii*2], (char*)"%02x", out[ii]);
						}
						
						
						if( (2==iVerbosType || 3==iVerbosType) && (TRUE == verbose) )
						{
							printf("%s  *%s*%s__(%llu) \r\n", outTxt, str_hash, iFiles.name, iFiles.size   );
							if(outfile) fprintf(outfile, "%s  *%s*%s__(%llu) \r\n", outTxt, str_hash, iFiles.name, iFiles.size );
						}
						else
						{
							printf("%s  *%s*%s \r\n", outTxt, str_hash, iFiles.name );
							if(outfile) fprintf(outfile, "%s  *%s*%s \r\n", outTxt, str_hash, iFiles.name );
						}

						if(inpfile) { fclose(inpfile); inpfile=NULL; }
						if(data_buf){ free(data_buf); data_buf=NULL; }
	
	
						iTotSize += iFiles.size;
						multifileindex ++;
					}
	
				} while( 0 == _findnexti64( retFiles, &iFiles ) );
				_findclose( retFiles );
	
				multifileindex--;
	
				printf("\nBLAKE384>> Number of Input files : %d files (Total size: %llu Byte) ---", multifileindex, iTotSize );
	
			}
		
		}
		else
		{

			printf("BLAKE384>> BLAKE384 hashing... \n");

			memset( inbuf, 0x00, sizeof(inbuf) );
			memset( out, 0x00, sizeof(out) );

			//Initialize the BLAKE-384 context
		    blake384_init( &blakeS ); 

			kll = 0ULL;
			while((ll = fread(inbuf, 1, sizeof(inbuf), inpfile)) > 0) 
			{
				kll += ll;

				//Absorb input data
				blake384_update( &blakeS, inbuf, ll ); 

				//Digest the message
				// LOG_V("\bBLAKE384 Hashing for (%s) -> read : %lld Bytes \r", iFiles.name, kll );
			}
			ClearScreen(); 

			//Finish absorbing phase
		    blake384_final( &blakeS, out ); 


			memset( outTxt, 0x00, sizeof(outTxt) );
			for (ii = 0; ii < BLAKE384_LEN; ii++)
			{
				if( iUpper )
					sprintf(&outTxt[ii*2], (char*)"%02X", out[ii]);
				else
					sprintf(&outTxt[ii*2], (char*)"%02x", out[ii]);
			}

			if( (2==iVerbosType || 3==iVerbosType) && (TRUE == verbose) )
			{
				printf("%s  *%s*%s__(%llu) \r\n", outTxt, str_hash, infile_name, infile_size  );
				if(outfile) fprintf(outfile, "%s  *%s*%s__(%llu) \r\n", outTxt, str_hash, infile_name, infile_size  );
			}
			else
			{
				printf("%s  *%s*%s \r\n", outTxt, str_hash, infile_name );
				if(outfile) fprintf(outfile, "%s  *%s*%s \r\n", outTxt, str_hash, infile_name );
			}
	
			if(inpfile) { fclose(inpfile); inpfile=NULL; }
			if(data_buf){ free(data_buf); data_buf=NULL; }
			printf("\nBLAKE384>> Calculated BLAKE384 Hash Value - OK" );
	
		}
	}
	else if(1 == isBlake512)
	{
		unsigned char inbuf[BLOCK512], out[BLAKE512_LEN]={0,}; 
		char outTxt[BLAKE512_LEN*2]={0,}; 
		state512 blakeS; 
		unsigned __int64 kll;
		size_t		ll;
		struct	tm *pTime = NULL;
		unsigned int iLenSub = 0;

		pTime = current_time();
	
		if( (1==iVerbosType || 3==iVerbosType) && (TRUE == verbose) && outfile) /// 2014.08.01
		{
			fprintf(outfile, "----------------------------------------------------------------------------------\r\n" );
			fprintf(outfile, "BLAKE512 Hash is created at %04d/%02d/%02d/%s %02d:%02d:%02d by %s \n", pTime->tm_year, pTime->tm_mon, pTime->tm_mday,  
													WeekTXT[pTime->tm_wday], pTime->tm_hour, pTime->tm_min, pTime->tm_sec, EmailText );
			fprintf(outfile, "----------------------------------------------------------------------------------\r\n" );
		}		
	
		LOG_V("\n");
	
		if( ASTERISK_FOUND == isAsteMode )	/* For ah.exe --input *.* */
		{
			//unsigned __int64	kll = 0UL;
	
			printf("BLAKE512>> BLAKE512 hashing for input files* (%d) \n", argc );

#if 0
			//for(ii=0; ii<=(argc-isAsteminus); ii++) printf("%3d : [%s] (%d:%d)\n", ii, argv[ii], isAsteminus, iFirst );
	
					
			for(ii=0; ii<BLAKE512_LEN*2; ii++) printf("-");  printf(" %d --\r\n", BLAKE512_LEN*2 );
	
			if(inpfile) { fclose(inpfile); inpfile=NULL; }
			if(data_buf){ free(data_buf); data_buf=NULL; }
	
			for(ii=iFirst; ii<=(argc-isAsteminus) ; ii++)
			{

				printf("FILE - [%s] : ", argv[ii] );
				if( isFileExist( argv[ii] ) ) printf(" -- OK \n");
				else printf(" --- NONE ----------------- \n");


				if( (inpfile = fopen( argv[ii], "rb")) != NULL ) 
				{
					
					memset( inbuf, 0x00, sizeof(inbuf) );
					memset( out, 0x00, sizeof(out) );
					
					//Initialize the BLAKE-384 context
					blake384_init( &blakeS ); 
					
					kll = 0ULL;
					while((ll = fread(inbuf, 1, sizeof(inbuf), inpfile)) > 0) 
					{
						kll += ll;
					
						//Absorb input data
						blake384_update( &blakeS, inbuf, ll ); 
					
						//Digest the message
						printf("\bBLAKE384 Hashing for (%s) -> read : %lld Bytes \r", argv[ii], kll );
					}
					ClearScreen(); 
					
					//Finish absorbing phase
					blake384_final( &blakeS, out ); 
					
					
					memset( outTxt, 0x00, sizeof(outTxt) );
					for (ii = 0; ii < BLAKE384_LEN; ii++)
					{
						if( iUpper )
							sprintf(&outTxt[ii*2], (char*)"%02X", out[ii]);
						else
							sprintf(&outTxt[ii*2], (char*)"%02x", out[ii]);
					}
					
					
					if( (2==iVerbosType || 3==iVerbosType) && (TRUE == verbose) )
					{
						printf("%s	*BLAKE384*%s__(%llu) \r\n", outTxt, argv[ii], kll );
						if(outfile) fprintf(outfile, "%s  *BLAKE384*%s__(%llu) \r\n", outTxt, argv[ii], kll );
					}
					else
					{
						printf("%s	*BLAKE384*%s \r\n", outTxt, argv[ii] );
						if(outfile) fprintf(outfile, "%s  *BLAKE384*%s \r\n", outTxt, argv[ii] );
					}
					
					if(inpfile) { fclose(inpfile); inpfile=NULL; }
					if(data_buf){ free(data_buf); data_buf=NULL; }
					
					iTotSize += iFiles.size;
					multifileindex ++;
	
				}

				
			}
			printf("\nBLAKE384>> BLAKE384 Number of Input files : %d files (Total size: %llu Byte) ---", multifileindex, iTotSize );
#endif
			AllFilesClosed();
	
		}
		else if( multifileindex > 0 )
		{
		
			if(inpfile) { fclose(inpfile); inpfile=NULL; }
			if(data_buf){ free(data_buf); data_buf=NULL; }
	
			/* Find first .c file in current directory */
			if( (retFiles = _findfirsti64( extfile_name, &iFiles )) == -1L )
			{
				printf("BLAKE512>>No file [%s] in current directory!!! [%s] \n", extfile_name, infile_name );
			}
			else
			{
	
				printf("BLAKE512>> BLAKE512 hashing for input files... \n");
				//printf("-------------------------------------------------------------\r\n" );
				for(ii=0; ii<BLAKE512_LEN*2; ii++) printf("-");  
				printf(" %d --\r\n", BLAKE512_LEN*2 );

				do {
	
					iLenSub = strlen(iFiles.name);
	
					if( iFiles.attrib & _A_SUBDIR )
					{
	
					}
					else
					{
						if( NULL == (inpfile = fopen( iFiles.name, "rb")) ) 
						{
							beep(700,100);
							printf("\n\n[++ERROR++] Can not open multi-input file (%s) in BLAKE512 \n", iFiles.name );
	
							AllFilesClosed();
	
							exit(0);
							return 0;
						}

						memset( inbuf, 0x00, sizeof(inbuf) );
						memset( out, 0x00, sizeof(out) );
						
						//Initialize the BLAKE512 context
						blake512_init( &blakeS ); 
						
						kll = 0ULL;
						while((ll = fread(inbuf, 1, sizeof(inbuf), inpfile)) > 0) 
						{
							kll += ll;
						
							//Absorb input data
							blake512_update( &blakeS, inbuf, ll ); 
						
							//Digest the message
							// LOG_V("\bBLAKE512 Hashing for (%s) -> read : %lld Bytes \r", iFiles.name, kll );
						}
						ClearScreen(); 
						
						//Finish absorbing phase
						blake512_final( &blakeS, out ); 
						
						
						memset( outTxt, 0x00, sizeof(outTxt) );
						for (ii = 0; ii < BLAKE512_LEN; ii++)
						{
							if( iUpper )
								sprintf(&outTxt[ii*2], (char*)"%02X", out[ii]);
							else
								sprintf(&outTxt[ii*2], (char*)"%02x", out[ii]);
						}
						
						if( (2==iVerbosType || 3==iVerbosType) && (TRUE == verbose) )
						{
							printf("%s  *%s*%s__(%llu) \r\n", outTxt, str_hash, iFiles.name, iFiles.size );
							if(outfile) fprintf(outfile, "%s  *%s*%s__(%llu) \r\n", outTxt, str_hash, iFiles.name, iFiles.size );
						}
						else
						{
							printf("%s  *%s*%s \r\n", outTxt, str_hash, iFiles.name  );
							if(outfile) fprintf(outfile, "%s  *%s*%s \r\n", outTxt, str_hash, iFiles.name );
						}
						
						if(inpfile) { fclose(inpfile); inpfile=NULL; }
						if(data_buf){ free(data_buf); data_buf=NULL; }
	
	
						iTotSize += iFiles.size;
						multifileindex ++;
					}
	
				} while( 0 == _findnexti64( retFiles, &iFiles ) );
				_findclose( retFiles );
	
				multifileindex--;
	
				printf("\nBLAKE512>> Number of Input files : %d files (Total size: %llu Byte) ---", multifileindex, iTotSize );
	
			}
		
		}
		else
		{

			printf("BLAKE512>> BLAKE512 hashing... \n");

			memset( inbuf, 0x00, sizeof(inbuf) );
			memset( out, 0x00, sizeof(out) );

			//Initialize the BLAKE512 context
			blake512_init( &blakeS ); 

			kll = 0ULL;
			while((ll = fread(inbuf, 1, sizeof(inbuf), inpfile)) > 0) 
			{
				kll += ll;

				//Absorb input data
				blake512_update( &blakeS, inbuf, ll ); 

				//Digest the message
				// LOG_V("\bBLAKE512 Hashing for (%s) -> read : %lld Bytes \r", iFiles.name, kll );
			}
			ClearScreen(); 

			//Finish absorbing phase
			blake512_final( &blakeS, out ); 
			
			memset( outTxt, 0x00, sizeof(outTxt) );
			for (ii = 0; ii < BLAKE512_LEN; ii++)
			{
				if( iUpper )
					sprintf(&outTxt[ii*2], (char*)"%02X", out[ii]);
				else
					sprintf(&outTxt[ii*2], (char*)"%02x", out[ii]);
			}

			if( (2==iVerbosType || 3==iVerbosType) && (TRUE == verbose) )
			{
				printf("%s  *%s*%s__(%llu) \r\n", outTxt, str_hash, infile_name, infile_size  );
				if(outfile) fprintf(outfile, "%s  *%s*%s__(%llu) \r\n", outTxt, str_hash, infile_name, infile_size	);
			}
			else
			{
				printf("%s  *%s*%s \r\n", outTxt, str_hash, infile_name );
				if(outfile) fprintf(outfile, "%s  *%s*%s \r\n", outTxt, str_hash, infile_name );
			}
	
			if(inpfile) { fclose(inpfile); inpfile=NULL; }
			if(data_buf){ free(data_buf); data_buf=NULL; }
			printf("\nBLAKE512>> Calculated BLAKE512 Hash Value - OK" );
	
		}
	}
#endif // BLAKE_224_256_384_512_HASH



#if IDEA_ENCIPHER_ALGORITHM 
	else if( 1==isIDEA ) /// 2014.07.26
	{
		long **K, **L;
		ushort i, j, key[8] = {1, 2, 3, 4, 5, 6, 7, 8};
		ushort X[4] = {0, 1, 2, 3}, Y[4];

		K = calloc(9, sizeof(long *));
		L = calloc(9, sizeof(long *));

		for (i = 0; i < 9; i++) 
		{
			K[i] = calloc(6, sizeof(long));
			L[i] = calloc(6, sizeof(long));

			for (j = 0; j < 6; j++) K[i][j] = L[i][j] = 0;
		}

		IDEA_encryption_key_schedule(key, K);
		IDEA_encryption(X, Y, K);
		IDEA_decryption_key_schedule(K, L);
		IDEA_encryption(Y, X, L);

		for (i = 0; i < 9; i++) 
		{
			free(K[i]);
			free(L[i]);
		}
		free(K);
		free(L);
	}
#endif /// IDEA_ENCIPHER_ALGORITHM 



#if MODIFIED_JULIAN_DATE
	else if(1 == isMJD) /// to date
	{
		printf("\n---------------- MJD (Modified Julian Date) ----------------- \n");
		
		printf("MJD>> Processing Modified Julian Date to DATE.. \n");	
		while( EOF != (c=fscanf(inpfile,"%lf", &MJDvalue)) )
		{
			ymd_hms = Convert2Timestamp( MJDvalue );
			cHex.dbl_val = MJDvalue;

			printf("MJD:[ %17s ] [",  commify(MJDvalue, str_cmBuf, 6) );
		#if CONVERT_HEX==CONVERT_HEX_MSB
			for(iTMP=0; iTMP<sizeof(double); iTMP++)
			{
				printf("%02X", cHex.dbl_bin[iTMP]);
				if(iTMP<sizeof(double)-1) printf("-");
			}
		#elif CONVERT_HEX==CONVERT_HEX_LSB
			for(iTMP=sizeof(double)-1; iTMP>=0; iTMP--)
			{
				printf("%02X", cHex.dbl_bin[iTMP]);
				if(iTMP>0) printf("-");
			}
		#endif /// CONVERT_HEX_MSB	

			printf("] -> DATE:[ %d/%02d/%02d, %02d:%02d:%02d:%03d ] \n", 
						ymd_hms.m_year, ymd_hms.m_month, ymd_hms.m_day, 
						ymd_hms.m_hour, ymd_hms.m_mins,  ymd_hms.m_secs, ymd_hms.m_millis );

			if(outfile) 
				fprintf(outfile, "MJD:[ %17s ] -> DATE:[ %d/%02d/%02d, %02d:%02d:%02d:%03d ] \r\n", 
						commify(MJDvalue, str_cmBuf, 6), ymd_hms.m_year, ymd_hms.m_month, ymd_hms.m_day, 
						ymd_hms.m_hour, ymd_hms.m_mins,  ymd_hms.m_secs, ymd_hms.m_millis );
		}


		opt_ok = 0; /* 2014.07.16 : NOTE !!! clear!!!  */

		AllFilesClosed();

		printf("\nMJD>> Converted MJD(Modified Julian Date) to DATE - OK" );


	}
	else if(2 == isMJD) /// to MJD
	{

		printf("\n---------------- MJD (Modified Julian Date) ----------------- \n");
		
		printf("MJD>> Processing Modified Julian Date to MJD number.. \n");	

		ymd_hms.m_millis = 0;
		while( EOF != (c=fscanf(inpfile,"%d %d %d %d %d %d", 
				&ymd_hms.m_year, &ymd_hms.m_month, &ymd_hms.m_day, 
				&ymd_hms.m_hour, &ymd_hms.m_mins, &ymd_hms.m_secs )) )
		{
			MJDvalue = Convert2MJD( ymd_hms );
			cHex.dbl_val = MJDvalue;

			printf("DATE:[ %d/%02d/%02d, %02d:%02d:%02d:%03d ] -> MJD:[ %17s ] [", 
						ymd_hms.m_year, ymd_hms.m_month, ymd_hms.m_day, 
						ymd_hms.m_hour, ymd_hms.m_mins,  ymd_hms.m_secs, ymd_hms.m_millis, commify(MJDvalue, str_cmBuf, 6) );

		#if CONVERT_HEX==CONVERT_HEX_MSB
			for(iTMP=0; iTMP<sizeof(double); iTMP++)
			{
				printf("%02X", cHex.dbl_bin[iTMP]);
				if(iTMP<sizeof(double)-1) printf("-");
			}
		#elif CONVERT_HEX==CONVERT_HEX_LSB
			for(iTMP=sizeof(double)-1; iTMP>=0; iTMP--)
			{
				printf("%02X", cHex.dbl_bin[iTMP]);
				if(iTMP>0) printf("-");
			}
		#endif /// CONVERT_HEX_MSB	
			printf("]\n");

			fprintf(outfile, "DATE:[ %d/%02d/%02d, %02d:%02d:%02d:%03d ] -> MJD:[ %17s ] \r\n", 
						ymd_hms.m_year, ymd_hms.m_month, ymd_hms.m_day, 
						ymd_hms.m_hour, ymd_hms.m_mins,  ymd_hms.m_secs, ymd_hms.m_millis, commify(MJDvalue, str_cmBuf, 6) );
		}


		opt_ok = 0; /* 2014.07.16 : NOTE !!! clear!!!  */

		AllFilesClosed();

		printf("\nMJD>> Converted DATE to MJD(Modified Julian Date) - OK" );


	}
#endif /// MODIFIED_JULIAN_DATE


	else if( (1==ismultiFilesExtract) )
	{
		unsigned __int64	kll=0UL, ll=0UL;
		int  ii=0, iiTot=0, hashOK=0;
		unsigned __int64	fSize=0UL, fUsize=0UL, fRead=0UL;

		SHA256_CTX		ctx256;
		char	sha256_buf[SHA2_BUFLEN];
	

		if( NULL != (inpfile = fopen( extractFile, "rb")) ) 
		{
			printf("\n");
			memset(mergeTotIndex, 0x00, sizeof(mergeTotIndex) );
			ll = fread(mergeTotIndex, 1, MERGE_TOTAL_CNT, inpfile); 
			kll += ll;
			iiTot = atoi(mergeTotIndex);
			printf(">>Merged total cnt: %s (%d)\n", mergeTotIndex, iiTot);

			memset(str_moduleName, 0x00, sizeof(str_moduleName) );
			ll = fread(str_moduleName, 1, MAX_VERSION_LEN, inpfile); 
			kll += ll;
			printf(">>Model Name      : %s\n", str_moduleName);


			memset(str_versionName, 0x00, sizeof(str_versionName) );
			ll = fread(str_versionName, 1, MAX_VERSION_LEN, inpfile); 
			kll += ll;
			printf(">>Version Name    : %s\n", str_versionName);

			memset(exFileInfo, 0x00, sizeof(exFileInfo) );
			for(ii=0; ii<iiTot; ii++)
			{
				ll = fread((char *)&exFileInfo[ii], 1, sizeof(exFileInfo[0]), inpfile);
				kll += ll;
				printf("Index ---------: [%s] len:%lld \n", exFileInfo[ii].mergeTxtIndex, strlen(exFileInfo[ii].mergeTxtIndex) );
				printf("  Date --------: [%s] len:%lld \n", exFileInfo[ii].mergeDate, strlen(exFileInfo[ii].mergeDate) );
				printf("  FileName ----: [%s] len:%lld \n", exFileInfo[ii].mergeFileName, strlen(exFileInfo[ii].mergeFileName) );
				printf("  FileSize ----: [%s] len:%lld \n", exFileInfo[ii].mergeTxtSize, strlen(exFileInfo[ii].mergeTxtSize)  );
				exFileInfo[ii].mergeSHA256[MERGE_SHA256_SIZ] = 0x00; 
				printf("  File SHA256 -: [%s] len:%lld \n", exFileInfo[ii].mergeSHA256, strlen(exFileInfo[ii].mergeSHA256)  );

				if(inpfile) fseek(inpfile, -1, SEEK_CUR);  /* because of SHA256 - 64bytes, SEEK_CUR, SEEK_END */

			}


			// === create output file			
			printf("\n");
			for(ii=0; ii<iiTot; ii++)
			{
				if( NULL == (outfile = fopen( exFileInfo[ii].mergeFileName, "wb"))	)	
				{
					printf("[++ERROR++]Can NOT create extract file (%s)... \n", exFileInfo[ii].mergeFileName );
				}
				else
				{
				
					fSize = atoi(exFileInfo[ii].mergeTxtSize);
					fRead = EXTRACT_FILE_SIZE;
					fUsize = 0UL;
					memset(strExtract, 0x00, sizeof(strExtract) );

					printf(">>%2d -> Extracting --: %s / %lld \n", ii+1, exFileInfo[ii].mergeFileName, fSize );
					while((ll = fread(strExtract, 1, fRead, inpfile)) > 0) 
					{
						kll += ll;

						fwrite(strExtract, sizeof(unsigned char), ll, outfile); 
						fUsize += ll;

						if( (fSize-fUsize) < 1024 ) fRead=(fSize-fUsize);
					}					
					if(outfile) fclose(outfile);

				}
			}

		}
		if(inpfile) fclose(inpfile);


		// ==================================================================
		// === SHA256 Hashing
		printf("\n");
		hashOK = 0; // 0 for OK
		for(ii=0; ii<iiTot; ii++)
		{
		
			if( NULL == (inpfile = fopen( exFileInfo[ii].mergeFileName, "rb")) ) 
			{
				printf("Files not found for SHA256!! [%s] \n", exFileInfo[ii].mergeFileName );
			}
			else
			{
				/// initial
				memset( &ctx256, 0x00, sizeof(SHA256_CTX) );
				memset( sha256_buf, 0x00, sizeof(sha256_buf) );
				
				/// SHA2 Calculate ----
				SHA256_Init(&ctx256);

				kll = 0UL;
				printf(">>%2d -> Read *SHA256*%s   %s \n", ii+1, exFileInfo[ii].mergeSHA256, exFileInfo[ii].mergeFileName );
				while((ll = fread(sha256_buf, 1, SHA2_BUFLEN, inpfile)) > 0) 
				{
					kll += ll;
					SHA256_Update(&ctx256, (unsigned char*)sha256_buf, ll);
				}
				SHA256_End(&ctx256, sha256_buf);
				
				if( 0 == strncmp(exFileInfo[ii].mergeSHA256, sha256_buf, MERGE_SHA256_SIZ ) )
				{
					printf("     SHA256 Hash OK... \n" );
				}
				else
				{
					printf("  SHA256 Hash NG ---:%s -> Not same file. Check plz.. \n", sha256_buf );
					hashOK ++;
				}
			}
			if(inpfile) fclose(inpfile);				
		}
		// ==================================================================
		if( 0 == hashOK )
			printf("\nAll extracted files are trusted files... \n");
		else
			printf("\n[++ERROR++]An untrusted file has been created... check plz...\n");

			
	}
	else if( (1==ismultiFilesMerge) )
	{
	int fileNum = iEndIdx-istartIdx;
	int ii=1, i=0, length=0;
	long 	attFile;
	int iErr = 0;

	unsigned __int64	kll=0UL, ll=0UL;
	SHA256_CTX		ctx256;
	char	sha256_buf[SHA2_BUFLEN];

		//==1== Total Counter
		printf("\n>>Total merged file count : %d \n",  fileNum-1) ;
		memset(mergeTotIndex, 0x00, sizeof(mergeTotIndex) );
		sprintf(mergeTotIndex,"%d",fileNum-1 );
		
		for(i=0; i<MERGE_TOTAL_CNT; i++)
		{ if( ('\n' == mergeTotIndex[i]) || ('\r' == mergeTotIndex[i]) || (0x20 == mergeTotIndex[i]) ) mergeTotIndex[i] = 0x00; }

		// ----------------------------------------------------
		// sub store-0 : Total File Counter		
		if(outfile) 
		{
			fprintf(outfile,"%s", mergeTotIndex );
		}
		length = strlen(mergeTotIndex);
		while( length < MERGE_TOTAL_CNT )
		{
			if(outfile) fprintf(outfile,"%c",SPACE_FILL1);
			length++;
		}		
		// ----------------------------------------------------


		// ================= MODEL NAME ====================================
		if( ATT_MODEL == isAttachMdl )
		{
			/* ++++++++ MODEL NAME +++++++++++++++++++++++++++ */
			/* step2 : Model name (16 characters) */
			/* +++++++++++++++++++++++++++++++++++++++++++++++ */
			len_module_name = strlen(str_moduleName);
			if( len_module_name < MAX_CHARS )
			{
				if( len_module_name == 0 ) 
				{ 
					memcpy(str_moduleName, DUMMY_FILL, DUMMY_FILL_LEN); 
					len_module_name=DUMMY_FILL_LEN; 
				}
				if(outfile) fprintf(outfile,"%s", str_moduleName);
				while( len_module_name < MAX_CHARS )
				{
					if(outfile) fprintf(outfile,"%c",SPACE_FILL2);
					len_module_name++;
				}
			}
			else
			{
				count=0;
				while( count < MAX_CHARS )
				{
					if(outfile) fprintf(outfile,"%c",str_moduleName[count] );
					count++;
				}
			}
		}


		// ================= VERSION NAME ====================================
		if( (ATT_VERSION == isAttachVer) || (ATT_MCU_32BYTE == isAttachVer) )
		{

			if( ATT_VERSION == isAttachVer )
			{
				// --------------------------------------------------
				/* ++++++++ VERSION NAME +++++++++++++++++++++++++++ */
				/* step4 : Version name (16 characters) -> 
				   step3 : Version name 32 Bytes : 2017.11.21 */
				/* +++++++++++++++++++++++++++++++++++++++++++++++ */
				len_version_name = strlen(str_versionName);
				if( len_version_name < MAX_VERSION_LEN )
				{
					if( len_version_name == 0 ) 
					{ 
						memcpy(str_versionName, DUMMY_FILL, DUMMY_FILL_LEN); 
						len_version_name = DUMMY_FILL_LEN; 
					}
					if(outfile) fprintf(outfile,"%s", str_versionName);
					while( len_version_name < MAX_VERSION_LEN )
					{
						if(outfile) fprintf(outfile,"%c",SPACE_FILL4);
						len_version_name++;
					}
				}
				else
				{
					count=0;
					while( count < MAX_VERSION_LEN )
					{
						if(outfile) fprintf(outfile,"%c",str_versionName[count] );
						count++;
					}
				}
			}
			else if( ATT_MCU_32BYTE == isAttachVer )
			{
				len_version_name = strlen(str_MCUversion);
				if( len_version_name < MAX_32CHARS )
				{
					if( len_version_name == 0 ) 
					{ 
						memcpy(str_MCUversion, DUMMY_FILL, DUMMY_FILL_LEN); 
						len_version_name = DUMMY_FILL_LEN; 
					}
					if(outfile) fprintf(outfile,"%s", str_MCUversion);
					while( len_version_name < MAX_32CHARS )
					{
						if(outfile) fprintf(outfile,"%c",SPACE_FILL4);
						len_version_name++;
					}
				}
				else
				{
					count=0;
					while( count < MAX_32CHARS )
					{
						if(outfile) fprintf(outfile,"%c",str_MCUversion[count] );
						count++;
					}
				}
			
			}

		}


		

		//==2== File List-up (Date/Size) and Index
		iErr = 0;
		for(ii=1; ii<fileNum; ii++)
		{

			if( (attFile = _findfirst( mFile[ii].mergeFileName, &mergeInfo )) == -1L )
			{
				printf( "No files in current directory!! (%d)-[%s] \n", ii, mFile[ii].mergeFileName );
				iErr ++;
			}
			else
			{
				if( !(mergeInfo.attrib & _A_SUBDIR) )
				{
					mFile[ii].mergeIndex = ii;
					mFile[ii].mergeSize = mergeInfo.size;
					memset(mFile[ii].mergeDate, 0x00, sizeof(mFile[ii].mergeDate) );
					strncpy(mFile[ii].mergeDate, ctime( &( mergeInfo.time_write )), 24 );

					//printf( "[%d] => [%s] [%s] [%s], Size: %ld bytes \n", ii, mFile[ii].mergeFileName, mergeInfo.name, mFile[ii].mergeDate, mergeInfo.size );
				}
			}

			_findclose( attFile );
		}


		//==3== SHA2 ----
		printf("SHA2>> SHA-256 hashing for encapsulating files... \n");
		for(ii=1; ii<fileNum; ii++)
		{

			if( NULL == (inpfile = fopen( mFile[ii].mergeFileName, "rb")) ) 
			{
				printf("--merge option error, files not found!! [%s] \n", mFile[ii].mergeFileName );
				iErr ++;

				//AllFilesClosed();
				//exit(0); /// help();
				//return 0;
			}

			/// initial
			memset( &ctx256, 0x00, sizeof(SHA256_CTX) );
			memset( sha256_buf, 0x00, sizeof(sha256_buf) );

			/// SHA2 Calculate ----
			SHA256_Init(&ctx256);

			kll = 0UL;
			printf("Waiting for SHA256**[%s]... \r", mFile[ii].mergeFileName);
			while((ll = fread(sha256_buf, 1, SHA2_BUFLEN, inpfile)) > 0) 
			{
				kll += ll;
				SHA256_Update(&ctx256, (unsigned char*)sha256_buf, ll);
			}
			SHA256_End(&ctx256, sha256_buf);

			memset(mFile[ii].mergeSHA256, 0x00, MERGE_SHA256_SIZ*sizeof(char) );
			strcpy(mFile[ii].mergeSHA256, sha256_buf );

			printf("\r");
			printf("%2d -> %s  *SHA256*%s [%s] / %u Bytes \n", ii, mFile[ii].mergeSHA256, mFile[ii].mergeFileName, mFile[ii].mergeDate, mFile[ii].mergeSize );


			if(outfile) 
			{

				// --------------------------------------------------------
				// sub store-1: Index Number
				memset(mergeTxtIndex, 0x00, MERGE_INDEX_SIZ*sizeof(char) );
				sprintf(mergeTxtIndex,"%d",ii );
				fprintf(outfile,"%s", mergeTxtIndex);
				
				length = strlen(mergeTxtIndex);
				while( length < MERGE_INDEX_SIZ )
				{
					if(outfile) fprintf(outfile,"%c",SPACE_FILL1);
					length++;
				}
				// --------------------------------------------------------

				// --------------------------------------------------------
				// sub store-2: File Date
				fprintf(outfile,"%s", mFile[ii].mergeDate );
				length = strlen(mFile[ii].mergeDate);
				while( length < MERGE_DATE_SIZ )
				{
					if(outfile) fprintf(outfile,"%c",SPACE_FILL1);
					length++;
				}


				// --------------------------------------------------------
				// sub store-3: File Name
				fprintf(outfile,"%s", mFile[ii].mergeFileName );
				length = strlen(mFile[ii].mergeFileName);
				while( length < MERGE_FILENAME_LENGTH )
				{
					if(outfile) fprintf(outfile,"%c",SPACE_FILL1);
					length++;
				}
				// --------------------------------------------------------


				// --------------------------------------------------------
				// sub store-4: File Size
				memset(mergeTxtSize, 0x00, sizeof(mergeTxtSize) );
				itoa(mFile[ii].mergeSize, mergeTxtSize, 10 );
				fprintf(outfile,"%s", mergeTxtSize );
				length = strlen(mergeTxtSize);
				while( length < MERGE_FILE_SIZ )
				{
					if(outfile) fprintf(outfile,"%c",SPACE_FILL1);
					length++;
				}
				// --------------------------------------------------------

				// sub store-5: Hash Code SHA256 (64bytes) for specific file
				fprintf(outfile,"%s", mFile[ii].mergeSHA256 );
				length = strlen(mFile[ii].mergeSHA256);
				while( length < MERGE_SHA256_SIZ )
				{
					if(outfile) fprintf(outfile,"%c",SPACE_FILL1);
					length++;
				}
				// --------------------------------------------------------


			}

			fclose(inpfile);


		}


		//==4 step== FILE Merging ----
		if(outfile) 
		{
			size_t	fr_size;
			size_t	tot_size = 0L;
			unsigned char read_buf[RD_BUF_SIZ] = {0,};
			

			printf("Encapulating...\n");
			for(ii=1; ii<fileNum; ii++)
			{
			
				printf("%2d -> [%s / %u Bytes] to output [%s / ", ii, mFile[ii].mergeFileName, mFile[ii].mergeSize, outfile_name );
				if( NULL == (inpfile = fopen( mFile[ii].mergeFileName, "rb")) ) 
				{
					printf("--merge option error2, files not found!! [%s] \n", mFile[ii].mergeFileName );
					iErr ++;

					//AllFilesClosed();
					//exit(0); /// help();
					//return 0;
				}

				memset( read_buf, 0x00, sizeof(read_buf) ); /// initial
				while( fr_size = fread(read_buf, sizeof(unsigned char), BUFSIZE, inpfile ) )
				{
					fwrite(read_buf, sizeof(unsigned char), fr_size, outfile); 
					tot_size += fr_size;
				}

				printf("%lld Bytes] ... \n", tot_size );

				if(inpfile) { fclose(inpfile); inpfile=NULL; }

			}

			if(outfile) fclose(outfile);

			if( 0 == iErr )
			{
				printf("\n>>Output file 	: %s (%s)", outfile_name, (isAppend==1)? "append":"new create");
				printf("\n>>Merge Completed...\n");
			}
			else
			{
				printf("\n>>Check input files...\n");
			}
		}
		else
		{
			printf("\n\n[++ERROR++] Can NOT Encapulating... check plz.....\n\n");
		}

		if(outfile) fclose(outfile);

		
	}

	else if( (1==isFileMerge) && (3==isJoin) ) // 2017.04.05
	{

		if( 0==strcasecmp(sefile_name, outfile_name) )
		{
			beep(700,100);
			printf("\n\n[++ERROR++] DO check 2nd input[%s] and output[%s]. Same!!! \n",sefile_name, outfile_name  );

			AllFilesClosed();
		
			exit(0); /// help();
			return 0;
		} 


		/// Attached Header --- 2014.07.15
		if(inpfile && outfile)
		{
			unsigned char Siz2ndTmp[4+1] = {0,};
			unsigned char Siz2ndBuf[4+1] = {0,};
			unsigned char read_buf[BUFSIZE] = {0,};
			unsigned __int64       first_file_tot_size = 0UL;
			unsigned int  sec_file_tot_size = 0;
			unsigned int  ipad_byte_cnt = 0;
			int aa=0, isum=0, totsum=0, iago=0, iidx=0;


			memset( Siz2ndTmp, 0x00, sizeof(Siz2ndTmp) );  
			memset( Siz2ndBuf, 0x00, sizeof(Siz2ndBuf) );  
			memset( read_buf, 0x00, sizeof(read_buf) ); /// initial

			// --- 1st file created
			first_file_tot_size = 0LL;
			while( fr_size = fread(read_buf, sizeof(unsigned char), BUFSIZE, inpfile ) )
			{
				fwrite(read_buf, sizeof(unsigned char), fr_size, outfile); 
				first_file_tot_size += fr_size;
			}

			if(inpfile) { fclose(inpfile); inpfile=NULL; }
			if(data_buf){ free(data_buf); data_buf=NULL; }


			// ----------------------------------------------------------
			// 2nd file의 size을 구하여 1st file의 마지막에 붙여라...
			// ----------------------------------------------------------
			if(is2ndFileSize)
			{

				// --------------------------------------------------
				// --------------------------------------------------
				// --- 2nd file is read because of 2nd file size -----------
				/* ========== INPUT FILE ================= */
				if( NULL == (inpfile = fopen( sefile_name, "rb")) ) 
				{
					beep(700,100);
					printf("\n\n[++ERROR++] Can not open 2nd input file[%s]. \n",sefile_name);

					AllFilesClosed();

					exit(0); /// help();
					return 0;
				}
				else
				{
					/// OK ---
					if (fstat(fileno(inpfile), &file_statbuf) < 0) 
					{ 
						printf("\n\n[++ERROR++]Can not stat 2nd file:[%s]\n", sefile_name ); 

						AllFilesClosed(); // 2020.07.10

						exit(0); /// 2017.11.21
						return 2; 
					} 	 
				}
				/* ========== INPUT FILE ================= */


				// -----------------------------------------------
				// 2nd file size ? 
				// -----------------------------------------------
				sec_file_tot_size = 0;
				while( fr_size = fread(read_buf, sizeof(unsigned char), BUFSIZE, inpfile ) )
				{
					sec_file_tot_size += fr_size;
				}

				//printf("\n>>2nd file size : 0x%08x (%d Byte)", sec_file_tot_size, sec_file_tot_size );
				if(inpfile) { fclose(inpfile); inpfile=NULL; }
				if(data_buf){ free(data_buf); data_buf=NULL; }
				// --------------------------------------------------
				// --------------------------------------------------



				// ----------------------------------------
				// 1) 0xFF padding : 4Byte 는 2nd file size를 위해 pad에서 제외...
				// ----------------------------------------
				ipad_byte_cnt = 0; // initial
				while( first_file_tot_size < iFileLimit - 4 ) // j option limit size (hexa)
				{
					fprintf(outfile,"%c", Pad_Byte); // 2020.06.23 <- 0xFF);
					first_file_tot_size++;
					ipad_byte_cnt ++;
				}

				if( ipad_byte_cnt > 0 )
				{
					if( ipad_byte_cnt>>20 )
						printf("\n>>Fill Pad byte   : (0x%02X) Size: %.3f MB (%#x)", Pad_Byte, (ipad_byte_cnt/1024.0)/1024.0, ipad_byte_cnt );
					else if( ipad_byte_cnt>>10 )
						printf("\n>>Fill Pad byte   : (0x%02X) Size: %.3f kB (%#x)", Pad_Byte, (ipad_byte_cnt/1024.0), ipad_byte_cnt );
					else 
						printf("\n>>Fill Pad byte   : (0x%02X) Size: %u Byte (%#x)", Pad_Byte, ipad_byte_cnt, ipad_byte_cnt );
				}
				

				// ----------------------------------------
				// 2) size : bottom 4 byte, 마지막 4byte는 2nd file의 file size
				// itoa(int val, char * buf, int radix);
				// ----------------------------------------
				//itoa(sec_file_tot_size, Siz2ndBuf, 16);
				//fprintf(outfile,"%s",Siz2ndBuf );
				
				sprintf( (char *)Siz2ndBuf,"%08x",sec_file_tot_size );
				printf("\n>>2nd file size   : %#04x (%d Byte)", sec_file_tot_size, sec_file_tot_size); // , Siz2ndBuf );

		#if 1
				iago = 0;
				totsum = 0;

				for( aa=0; aa<8;aa++)
				{
					if(Siz2ndBuf[aa]>='0' && Siz2ndBuf[aa]<='9')
					{
						isum |= Siz2ndBuf[aa]-0x30;
						iago ++;
					}
					else if(Siz2ndBuf[aa]>='A' && Siz2ndBuf[aa]<='F') 
					{
						//fprintf(outfile,"%c",Siz2ndBuf[aa]-0x41+10 );
						isum |= Siz2ndBuf[aa]-0x41+10;
						iago ++;
					}
					else if(Siz2ndBuf[aa]>='a' && Siz2ndBuf[aa]<='f') 
					{
						//fprintf(outfile,"%c",Siz2ndBuf[aa]-0x61+10 );
						isum |= Siz2ndBuf[aa]-0x61+10;
						iago ++;
					}
					else 
					{
						printf("\n>>2nd file size : Error check for out of range!!\n");
						fprintf(outfile,"%c",'Z' );
					}

					if( iago == 2 )
					{
						if( ENDIAN_BIG == is2ndEndian ) // Big Endian
							fprintf(outfile,"%c",isum );
						else if( ENDIAN_LITTLE == is2ndEndian ) // Little Endian
							sprintf( (char *)&Siz2ndTmp[iidx],"%c",isum );


						totsum += isum;

						//printf("\n>>[%c] [%x] %d totsum:[0x%x]\n", isum, isum, isum, totsum);
						iidx ++;

						isum = 0;
						iago = 0;
					}
					else
					{
						isum <<= 4;
					}

				}


				//if( iidx > 4 )
				//{
				//	printf("\n>>++ERROR++ 2nd file size field (4Byte) is over!!! [0x%x] \n", totsum);
				//}

				if( ENDIAN_LITTLE == is2ndEndian ) // Little Endian
				{
					for( aa=3; aa>=0;aa--)
					{
						fprintf(outfile,"%c",Siz2ndTmp[aa] );
					}
				}
		#else
				fprintf(outfile,"%s",Siz2ndBuf );

		#endif


			}
			else
			{
				ipad_byte_cnt = 0; // initial
				while( first_file_tot_size < iFileLimit ) // j option limit size (hexa)
				{
					fprintf(outfile,"%c", Pad_Byte); // <-- 2020.06.23 0xFF);
					first_file_tot_size++;
					ipad_byte_cnt ++;
				}


				if( ipad_byte_cnt > 0 )
				{
					if( ipad_byte_cnt>>20 )
						printf("\n>>Fill Pad byte   : (0x%02X) size: %.3f MB (%#x)", Pad_Byte, (ipad_byte_cnt/1024.0)/1024.0, ipad_byte_cnt );
					else if( ipad_byte_cnt>>10 )
						printf("\n>>Fill Pad byte   : (0x%02X) size: %.3f kB (%#x)", Pad_Byte, (ipad_byte_cnt/1024.0), ipad_byte_cnt );
					else 
						printf("\n>>Fill Pad byte   : (0x%02X) size: %u Byte (%#x)", Pad_Byte, ipad_byte_cnt, ipad_byte_cnt );
				
				}
			}


			// --- 2nd file is added -----------
			/* ========== INPUT FILE ================= */
			if( NULL == (inpfile = fopen( sefile_name, "rb")) ) 
			{
				beep(700,100);
				printf("\n\n[++ERROR++] Can not open input file [%s]. Use --input option!!!\n", sefile_name);

				AllFilesClosed();

				exit(EXIT_FAILURE); /* 0: EXIT_SUCCESS, 1: EXIT_FAILURE */
				return 0;
			}
			else
			{
				/// OK ---
				if (fstat(fileno(inpfile), &file_statbuf) < 0) 
				{ 
					printf("\n\n[++ERROR++]Cannot stat 2nd file:[%s]\n", sefile_name ); 

					AllFilesClosed(); // 2020.07.10
					exit(0); /// 2017.11.21
					return 2; 
				} 	 
			}
			/* ========== INPUT FILE ================= */


			// -----------------------------------------------
			// append 1st file to 2nd-file
			// -----------------------------------------------
			first_file_tot_size = 0UL;
			while( fr_size = fread(read_buf, sizeof(unsigned char), BUFSIZE, inpfile ) )
			{
				fwrite(read_buf, sizeof(unsigned char), fr_size, outfile); 
				first_file_tot_size += fr_size;
			}

			printf("\n>>Merged Total file size : %lld (0x%llx) Bytes", first_file_tot_size, first_file_tot_size );


		}
		else
		{
			printf("\nHDR>> Can not open input file[%s] or output file[%s] \n", infile_name, outfile_name);
		}

	}
	else if( (1==isFileMerge) && (3!=isJoin) ) // 2017.11.28
	{
		printf("\n\n[++ERROR++] DO check 2nd input[%s] for merging file!! \n",sefile_name );
	}
	else if(1==isRandomNum)
	{
		int ran;
		int caa=0, cbb=0, cdd=0, ccc=0, cee=0, cff=0;
		int c00=0, c11=0, c22=0, c77=0, c88=0, c99=0;

		srand( (unsigned int)time(NULL)+(unsigned int)getpid() );

		fprintf(stderr,"\n\n== Random Generator == (0x00~0xff: Count:%d) SEED:%lld %d\n", iRanMaxi, time(NULL), getpid());
		if(outfile) fprintf(outfile,"\n\n== Random Generator == (0x00~0xff: Count:%d) SEED:%lld %d\n", iRanMaxi, time(NULL), getpid());

		for ( idx = 1; idx <= iRanMaxi; idx++ )
		{
			ran = rand()%0x100;

			fprintf(stderr, "%02x ",  ran );
			if(outfile) fprintf(outfile,"%02x ", ran);

			if( 0xff == ran ) cff++;
			if( 0xee == ran ) cee++;
			if( 0xdd == ran ) cdd++;
			if( 0xcc == ran ) ccc++;
			if( 0xbb == ran ) cbb++;
			if( 0xaa == ran ) caa++;

			if( 0x10 == ran ) c00++;
			if( 0x11 == ran ) c11++;
			if( 0x22 == ran ) c22++;
			if( 0x77 == ran ) c77++;
			if( 0x88 == ran ) c88++;
			if( 0x99 == ran ) c99++;

			if( (0==idx%20) ) 
			{
				fprintf(stderr,"\n");
				if(outfile) fprintf(outfile,"\n");
			}
		}

		AllFilesClosed();
		
		fprintf(stderr,"\n\n");
		fprintf(stderr,"10:%d, 11:%d, 22:%d, 77:%d, 88:%d, 99:%d \n", c00, c11, c22, c77, c88, c99);
		fprintf(stderr,"aa:%d, bb:%d, cc:%d, dd:%d, ee:%d, ff:%d \n", caa, cbb, ccc, cdd, cee, cff);
	}
	else 
	{
		//printf("\n\n[++ERROR++] DO check -option");
	}
	printf("\r\n");


	/* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
	AllFilesClosed();
	exit(EXIT_SUCCESS); /* 0: EXIT_SUCCESS, 1: EXIT_FAILURE */
	
	/* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
	return(0);
}



