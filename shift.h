
#ifndef __SHIFT_QUALITY_DATA_H__
#define __SHIFT_QUALITY_DATA_H__

/************************************************************************************
 * @file  : shift.h
 * @brief : 
 *  - 2008/12/05 : Attached Header Created.
 *
 *
 * How to build :
 *     1) Install cygwin
 *     2) command as below:
 *        gcc -o AttachHdr -mno-cygwin AttachHdr.c  or
 *        gcc-3  -o AttachHdr -mno-cygwin AttachHdr.c 
 *
 * How to profiling
 *     1) gcc -pg -g -o AttachHdr -mno-cygwin AttachHdr.c 
 *     2) AttachHdr  -> running and then create file, gmon.out.
 *     3) gprof -p -b AttachHdr.exe
 *     4) result is as below:
 *         Flat profile:
 *
 * @version : 2.0
 * @date	: 2021/10/07
 * @author	: Yoon-Hwan Joo (tp.joo@daum.net or toppy_joo@naver.com) 
 *
 ************************************************************************************/


#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "hash.h"
#include "defs.h"


#if 0
#define SNDERR(...) snd_lib_error(__FILE__, __LINE__, __FUNCTION__, 0, __VA_ARGS__) /**< Shows a sound error message. */
#define SNDERR(args...) snd_lib_error(__FILE__, __LINE__, __FUNCTION__, 0, ##args) /**< Shows a sound error message. */
#endif



const char AttVersion[] = "1.0.16"; /* ver 1.0.12 : 2022.12.11 : Shift Quality Data Sorting */
const char EmailText[]  = "tp.joo@daum.net";


enum {
	MOT_HEX_S0 = 0x0001,
	MOT_HEX_S1 = 0x0002,
	MOT_HEX_S2 = 0x0004,
	MOT_HEX_S3 = 0x0008,
	MOT_HEX_S4 = 0x0010,
	MOT_HEX_S5 = 0x0020,
	MOT_HEX_S6 = 0x0040,
	MOT_HEX_S7 = 0x0080,
	MOT_HEX_S8 = 0x0100,
	MOT_HEX_S9 = 0x0200
};


#define HDR_CRC16 				0x0015
#define HDR_KSC_CRC16 			0x0016
#define HDR_CRC16CCITT 			0x0017
#define HDR_ADLER32 			0x0018 /// zlib
#define HDR_CRC32 				0x0019
#define HDR_CRC64 				0x001a
#define HDR_CRC64_ISC 			0x001b
#define HDR_JOAAT 				0x001c // 2020.07.22

#define HDR_SHA1 				0x0020 // 2020.06.10
#define HDR_SHA224 				0x0021
#define HDR_SHA256 				0x0022
#define HDR_SHA384 				0x0023
#define HDR_SHA512 				0x0024
#define HDR_SHA3_224 			0x0025
#define HDR_SHA3_256 			0x0026
#define HDR_SHA3_384 			0x0027
#define HDR_SHA3_512 			0x0028
#define HDR_SHAKE128 			0x0029
#define HDR_SHAKE256 			0x002a

#define HDR_MD2 				0x0030
#define HDR_MD4 				0x0031
#define HDR_MD5 				0x0032
#define HDR_MD6 				0x0033

#define HDR_BLAKE224 			0x0035
#define HDR_BLAKE256 			0x0036
#define HDR_BLAKE384 			0x0037
#define HDR_BLAKE512 			0x0038


#define HDR_RMD128 				0x0040
#define HDR_RMD160 				0x0041
#define HDR_RMD320 				0x0042

#define HDR_CRC_UNKNOWN 			0xffff



#define LOG_ERR(fmt, ...) 		fprintf(stderr, fmt "\n", ## __VA_ARGS__)
#define LOG_INF(fmt, ...)  		fprintf(stdout, fmt "\n", ## __VA_ARGS__)
#define LOG_V(fmt, ...) 		fprintf(stderr, fmt, ## __VA_ARGS__)


#define COMMA_BUF_SIZE  			256 /// 2014.07.23


#define DEBUG_LEVEL_	3
#ifdef  DEBUG_LEVEL_
#define dprta(n, fmt, args...)		if (DEBUG_LEVEL_ <= n) fprintf(stderr, "%s:%d,"fmt, __FILE__, __LINE__, ## args)
#define dprt0(n, fmt)				if (DEBUG_LEVEL_ <= n) fprintf(stderr, "%s:%d,"fmt, __FILE__, __LINE__)
#define dprtn(n, fmt, args...)		if (DEBUG_LEVEL_ <= n) fprintf(stderr, " "fmt, ## args)
#else 
#define dprta(n, fmt, args...)
#define dprt0(n, fmt)
#define dprtn(n, fmt, args...)
#endif	/* DEBUG_LEVEL_ */



/// --float / double to hex --------------------------
typedef union {
	float		   flt_val;		
	unsigned char  flt_bin[sizeof(float)]; 
} __float_hex_union; /// = { 0.866f };	 

typedef union { 	  
	double	       dbl_val;	   
	unsigned char  dbl_bin[sizeof(double)];    
} __double_hex_union;
/// --float / double to hex --------------------------





typedef struct _BIN_IMAGE_HEADER{	
	unsigned char sync[7];	 
	unsigned long addr;	  
	unsigned long length;	
}BIN_IMAGE_HEADER;	 
   
typedef struct _BIN_SECTION{    
	unsigned long addr;      /// 4byte
	unsigned long length;    /// 4byte
	unsigned long checksum;  /// 4byte
} BIN_SECTION;  


/// Said first TOC information the relative address 0x40 at two four-byte data, 
/// the first parity information (ECEC), the second refers clear address of the ROMHDR structure. 
/// This ROMHDR structure contains a lot of useful information, such as image file module file, 
/// there are some segments (Section) information, such as eboot is to relocate the global variable. 
		
typedef struct ROM_HEADER { 
	ULONG	   dllfirst;			/// First DLL address 
	ULONG	   dlllast; 			/// Last DLL address 
	ULONG	   physfirst;			/// First physical address 
	ULONG	   physlast;			/// Highest physical address 
	ULONG	   nummods; 			/// Number of TOCentry's 
	ULONG	   ulRamStart;			/// Start of RAM 
	ULONG	   ulRAMFree;			/// Start of RAM free space 
	ULONG	   ulRAMEnd;			/// End of RAM 
	ULONG	   ulCopyEntries;		/// Number of copy section entries 
	ULONG	   ulCopyOffset;		/// Offset to copy section 
	ULONG	   ulProfileLen;		/// Length of PROFentries RAM 
	ULONG	   ulProfileOffset; 	/// Offset to PROFentries 
	ULONG	   numfiles;			/// Number of FILES 
	ULONG	   ulKernelFlags;		/// Optional kernel flags from ROMFLAGS. Bib config option 
	ULONG	   ulFSRamPercent;		/// Percentage of RAM used for filesystem 
									// from FSRAMPERCENT .bib config option
									// byte 0 = #4K chunks/Mbyte of RAM for filesystem 0-2Mbytes 0-255
									// byte 1 = #4K chunks/Mbyte of RAM for filesystem 2-4Mbytes 0-255
									// byte 2 = #4K chunks/Mbyte of RAM for filesystem 4-6Mbytes 0-255
									// byte 3 = #4K chunks/Mbyte of RAM for filesystem > 6Mbytes 0-255

	ULONG	   ulDrivglobStart; 	/// Device driver global starting address 
	ULONG	   ulDrivglobLen;		/// Device driver global length 
	USHORT     usCPUType;			/// CPU (machine) Type 
	USHORT     usMiscFlags; 		/// Miscellaneous flags 
	PVOID	   pExtensions; 		/// Pointer to ROM Header extensions 
	ULONG	   ulTrackingStart; 	/// Tracking memory starting address 
	ULONG	   ulTrackingLen;		/// Tracking memory ending address 
} ROMHDR; 


typedef struct ROMChain_t {
    struct ROMChain_t *pNext;
    ROMHDR *pTOC;
} ROMChain_t;


//
// ROM Header extension: PID
//
#define PID_LENGTH 				10

typedef struct ROMPID {
  union{
    DWORD dwPID[PID_LENGTH];        // PID
    struct{
      char  name[(PID_LENGTH - 4) * sizeof(DWORD)];
      DWORD type;
      PVOID pdata;
      DWORD length;
      DWORD reserved;
    };
  };
  PVOID pNextExt;                 // pointer to next extension if any
} ROMPID, EXTENSION;

typedef struct _RECORD_DATA {
    DWORD dwStartAddress;
    DWORD dwLength;
    DWORD dwChecksum;
    DWORD dwFilePointer;
} RECORD_DATA, *PRECORD_DATA;

#define MAX_RECORDS 2048
RECORD_DATA g_Records[MAX_RECORDS];



#if CONVERT_BMP2C
typedef union tagPixelData
{
    struct
    {
        WORD red:4;
        WORD green:4;
        WORD blue:4;
        WORD unused:4;
    } color754xx;
    struct
    {
        WORD red:5;
        WORD green:5;
        WORD blue:5;
        WORD unused:1;
    } color555;
    struct
    {
        WORD red:5;
        WORD green:6;
        WORD blue:5;
    } color565;
    struct
    {
        DWORD red:8;
        DWORD green:8;
        DWORD blue:8;
        DWORD alpha:8;
    } color888;
    DWORD pixel32;
    WORD pixel16[2];
} PixelData;


#define TO555(pd, r, g, b) \
		pd.color555.red = (r >> 3); \
		pd.color555.green = (g >> 3); \
		pd.color555.blue = (b >> 3); \
		pd.color555.unused = 0;

#define TO565(pd, r, g, b) \
		pd.color565.red = (r >> 3); \
		pd.color565.green = (g >> 2); \
		pd.color565.blue = (b >> 3);

#define TO888(pd, r, g, b) \
		pd.color888.red = r; \
		pd.color888.green = g; \
		pd.color888.blue = b; \
		pd.color888.alpha = 0;

#define TO_lPC754XX(pd, r, g, b) \
		pd.color754xx.red = (r >> 4); \
		pd.color754xx.green = (g >> 4); \
		pd.color754xx.blue = (b >> 4); \
		pd.color754xx.unused = 0;



#pragma pack(push, 1)                /* 구조체를 1바이트 크기로 정렬 */

typedef struct _BITMAPFILEHEADER	 // BMP 비트맵 파일 헤더 구조체
{
	unsigned short bfType;			 // BMP 파일 매직 넘버
	unsigned int   bfSize;			 // 파일 크기
	unsigned short bfReserved1; 	 // 예약
	unsigned short bfReserved2; 	 // 예약
	unsigned int   bfOffBits;		 // 비트맵 데이터의 시작 위치
} BITMAPFILEHEADER;

typedef struct _BITMAPINFOHEADER	 // BMP 비트맵 정보 헤더 구조체(DIB 헤더)
{
	unsigned int   biSize;			 // 현재 구조체의 크기
	int 		   biWidth; 		 // 비트맵 이미지의 가로 크기
	int 		   biHeight;		 // 비트맵 이미지의 세로 크기
	unsigned short biPlanes;		 // 사용하는 색상판의 수
	unsigned short biBitCount;		 // 픽셀 하나를 표현하는 비트 수
	unsigned int   biCompression;	 // 압축 방식
	unsigned int   biSizeImage; 	 // 비트맵 이미지의 픽셀 데이터 크기
	int 		   biXPelsPerMeter;  // 그림의 가로 해상도(미터당 픽셀)
	int 		   biYPelsPerMeter;  // 그림의 세로 해상도(미터당 픽셀)
	unsigned int   biClrUsed;		 // 색상 테이블에서 실제 사용되는 색상 수
	unsigned int   biClrImportant;	 // 비트맵을 표현하기 위해 필요한 색상 인덱스 수
} BITMAPINFOHEADER;

typedef struct _RGBTRIPLE			 // 24비트 비트맵 이미지의 픽셀 구조체
{
	unsigned char rgbtBlue; 		 // 파랑
	unsigned char rgbtGreen;		 // 초록
	unsigned char rgbtRed;			 // 빨강
} RGBTRIPLE;

#pragma pack(pop)


#define PIXEL_SIZE   3    // 픽셀 한 개의 크기 3바이트(24비트)
#define PIXEL_ALIGN  4    // 픽셀 데이터 가로 한 줄은 4의 배수 크기로 저장됨

#endif /// CONVERT_BMP2C






#define MONTH_LEN 	12
typedef struct {
	char 	mon[3];
	char   amon;	
} _month_table_ ;

_month_table_ month_table[MONTH_LEN] = 
{
	{ "Jan", 'A' },
	{ "Feb", 'B' },
	{ "Mar", 'C' },
	{ "Apr", 'D' },
	{ "May", 'E' },
	{ "Jun", 'F' },
	{ "Jul", 'G' },
	{ "Aug", 'H' },
	{ "Sep", 'I' },
	{ "Oct", 'J' },
	{ "Nov", 'K' },
	{ "Dec", 'L' },
};



#define ENDIAN_LITTLE 		1
#define ENDIAN_BIG 			2

enum {
	ASTERISK_UNKNOWN = -1,
	ASTERISK_INIT = 0,
	ASTERISK_STEP1 = 1,
	ASTERISK_FOUND = 2,

	ASTERISK_MAX = 9
};


#endif /* __SHIFT_QUALITY_DATA_H__ */


