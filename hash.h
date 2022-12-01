
#ifndef __HASH_H__
#define __HASH_H__


/***********************************************************************************
 * @file    : hash.h
 * @brief   : keyword define
 *           
 *           
 * @version : 2.0
 * @date    : 2022/11/20
 * @author  : Yoon-Hwan Joo (tp.joo@daum.net or toppy_joo@naver.com) 
 ************************************************************************************/



#define ONE_KILO_BYTE 			1024.0
#define ONE_MEGA_BYTE 			(1024*1024)
#define HALF_MEGA_BYTE 			(512*1024)
#define MAX_BUF_SIZ 			(50*(ONE_MEGA_BYTE)-HALF_MEGA_BYTE) /* NK.NB0 , EBOOT.NB0 size 변경시 수정후 building */



typedef unsigned char      u_int8_t;	/* 1-byte  (8-bits)  */
typedef unsigned int       u_int32_t;	/* 4-bytes (32-bits) */
typedef unsigned long long u_int64_t;	/* 8-bytes (64-bits) */
typedef unsigned short     u_int16_t;



#endif /* __HASH_H__ */



