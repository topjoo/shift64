
#ifndef __FEATURE_H__
#define __FEATURE_H__

/***********************************************************************************
 * @file    : feature.h
 * @brief   : Features define
 *     - 2008/12/05 : feature.h file is created.
 *           
 *           
 * @version : 2.0
 * @date    : 2014/06/30
 * @author  : Yoon-Hwan Joo (tp.joo@daum.net or toppy_joo@naver.com) 
 ************************************************************************************/

#include <stdio.h>



#define BLAKE_224_256_384_512_HASH 	1 // 2020.07.27

#define RIPEMD160 					1 // 2020.07.23
#define RIPEMD128 					1 // 2020.07.31
#define RIPEMD320 					0


#define MD2_CHECKSUM_ENCIPHER 		1 /// 2014.07.29
#define MD4_CHECKSUM_ENCIPHER 		1 /// 2014.07.26
#define MD5_CHECKSUM_ENCIPHER 		1
#define MD5_MULTI_INPUT_FILES 		1
#define MD6_CHECKSUM_ENCIPHER 		1 /// 2014.07.31


#define SHA1_HASH_ENCIPHER 			1 /// 2014.06.29
#define SHA1_MULTI_INPUT_FILES 		1
#define SHA2_256_384_512 			1 /// 2014.06.30
#define SHA2_MULTI_INPUT_FILES 		1
#define SHA2_224_CHECKSUM 			1 /// 2014.07.21
#define SHA3_KECCAK_224_256_384_512 	1 // 2020.06.11


#define CRC_CHECKSUM 				1
#define CHECKSUM_PROGRESS_DISPLAY 	1 //-- 2015.09.01





#define MODIFIED_JULIAN_DATE 		1 /// 2014.07.04, Modified Julian Date ---
#define MJD_SAMPLE_NUMBER 			10	/// sample number ---
#define CONVERT_HEX_MSB 			11
#define CONVERT_HEX_LSB				22
#define CONVERT_HEX					CONVERT_HEX_LSB






#define _MOT2BIN_ 					1
#define MOT2BIN_1_0_10 				1 /// added 2014.07.23
#define _INTEL2BIN_ 				1
#define HEX2BIN_1_0_10 				1 /// added 2014.07.23
#define ERROR_HEX2BIN_EXIT 			1 /// added 2014.07.28, if Size Error, Exit
#define ELF2BIN 					1 /// 2014.08.14

#define CONVERT_BMP2C 				0 /// not supported!!





/*--------------------------------------------------------------------------
                     INCLUDE FILES 
---------------------------------------------------------------------------*/
#define CRC_BUFSIZE 			(32*1024)

#define BUFSIZE 				(1024*2)
#define SHA_READ_BUFSIZ 		(100*1024)
#define MD_HASH_BUFSIZ 			(100*1024)







/* ------------------------------------------------------------------------ */

FILE *inpfile = NULL;
FILE *outfile = NULL;
unsigned char *data_buf = NULL;

/* ------------------------------------------------------------------------ */


#endif //__FEATURE_H__




