#ifndef __COMMON_H__
#define __COMMON_H__

/***********************************************************************************
 * @file    : common.h
 * @brief   : keyword define
 *     - 2008/12/05 : Attached Header, common.h is created.
 *           
 *           
 * @version : 2.0
 * @date    : 2014/06/30
 * @author  : Yoon-Hwan Joo (tp.joo@daum.net or toppy_joo@naver.com) 
 ************************************************************************************/


#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>


//typedef int boolean;
//typedef unsigned char byte;
//typedef unsigned short word;

//typedef unsigned long  dword;

//typedef unsigned char uint8_t;	/* 1-byte  (8-bits)  */
//typedef unsigned int uint32_t;
//typedef unsigned long long uint64_t;	/* 8-bytes (64-bits) */
//typedef unsigned short int uint16_t;

//typedef unsigned int uint16;	/* at LEAST 16 bits, maybe more */
//typedef unsigned int word16;

//typedef unsigned char uint8;
//typedef unsigned int  uint32;
//typedef unsigned long long uint64;

//typedef unsigned int uint_t;
//typedef unsigned char char_t;

//typedef unsigned int error_t;


/* typedef a 32 bit type */
typedef unsigned long int UINT4;

typedef unsigned long ULONG;
typedef unsigned int UINT;
typedef signed short SHORT;
typedef unsigned short USHORT;
typedef void* 		PVOID;

//typedef unsigned long word32;


typedef unsigned short WORD;
typedef unsigned long  DWORD;
//
// 
// --------------------------------------------
// stdint.h
// 7.18.1.1 Exact-width integer types
typedef __int8            int8_t;
typedef __int16           int16_t;
typedef __int32           int32_t;
typedef __int64           int64_t;



// 7.18.1.4 Integer types capable of holding object pointers
#ifdef _WIN64 // [
   typedef __int64           intptr_t;
   typedef unsigned __int64  uintptr_t;
#else // _WIN64 ][
   typedef int               intptr_t;
   typedef unsigned int      uintptr_t;
#endif // _WIN64 ]

// 7.18.1.5 Greatest-width integer types
typedef int64_t   		intmax_t;
typedef unsigned long long  		uintmax_t;
// --------------------------------------------


#ifndef min /* if macro not already defined */ 
#define min(a,b) 		((a)<(b) ? (a) : (b)) 
#endif /* if min macro not already defined */ 

#ifndef max
#define max(a,b) 		((a)>(b) ? (a) : (b)) 
#endif

#define swap(a) 		(((a)<<8)|((a)>>8))     

#define MATH_PI 		3.1415926535897932384626433832795F




#ifndef TRUE
#define TRUE 		1
#define FALSE 		0
#endif

typedef enum
{
	false = 0,
	true = 1,
} bool;




// macros
#if 0
#define DEBUGMSG(msg...) errormsg("DEBUG", msg);
#else
#define DEBUGMSG(msg...)
#endif

void errormsg(const char *func, int line, const char *str, ...);


#define myerror(msg...) errormsg(__func__, __LINE__, msg);

#endif //__COMMON_H__


