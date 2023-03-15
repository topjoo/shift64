
#ifndef __DEFS_H__
#define __DEFS_H__

/***********************************************************************************
 * @file    : defs.h
 * @brief   : keyword define
 *           
 *           
 * @version : 2.0
 * @date    : 2022/11/20
 * @author  : Yoon-Hwan Joo (tp.joo@daum.net or toppy_joo@naver.com) 
 ************************************************************************************/



#define RD_BUF_SIZ 				1024*10

#define MULTI_IN_FILES_CNT 			(4096)
#define LENGTH_OF_FILENAME			16 /* 16*16 -> 256 */
#define MAX_CHARS 					16
#define MAX_VERSION_LEN 			16 // 2017.12.12
#define MAX_CRC_LEN_CODE 			16 // 2017.11.21
#define MAX_HASH_CODE 				128 // 2017.11.21
#define MAX_32CHARS 				32 // 2022-10-18

#define MAX_FILENAME_LEN 			256

#define SPACE_FILL1 		'\0'
#define SPACE_FILL2 		'\0' //0xFF
#define SPACE_FILL3 		'\0'
#define SPACE_FILL4 		'\0'

#define DUMMY_FILL 			"TOPJOOhdr"
#define DUMMY_FILL_LEN 		9

#define TRUE 		1
#define FALSE 		0


#define CHECK_BIT_IN 		0x01
#define CHECK_BIT_OUT 		0x02


#define ATT_VERSION 		0x0001 /* 16Bytes */
#define ATT_DATEorCRC 		0x0002 /* 16Bytes or variable */
#define ATT_MODEL 			0x0004 /* 16Bytes */
#define ATT_BOARD 			0x0008 /* 16Bytes */

#define ATT_MCU_32BYTE 		0x0010 /* 32Bytes */


#define EXTRACT_FILE_SIZE 		1024




#define SHIFT_QUALITY_DATA_SORTING 	1 /* 2022-11-13 - Shift Quality Data Sorting Program */
#define QUAL_DATA_MAX_SIZE 	 		(1024)



#define INPUT_FORMATCS 				1

#if INPUT_FORMATCS
#define QUAL_TSV_DATA_ITEM_NUM 		17 /* 5ms_16select.tsv - Original data nums */
#else
#define QUAL_TSV_DATA_ITEM_NUM 		15 /* 5ms_16select.tsv - Original data nums */
#endif

#define QUAL_TSV_CASE1_17_ITEM_NUM 		17 /* 5ms_16select.tsv - Original data nums */
#define QUAL_TSV_CASE2_15_ITEM_NUM 		15 /* 5ms_16select.tsv - Original data nums */
#define QUAL_TSV_CASE2_NEW_15_ITEM 		15 /*  - Original data nums */

#define QUAL_TSV_CASE3_15_NEW_ITEMS 	15 /* New_FF8_PONUPSX2.tsv - Original data nums */
#define QUAL_TSV_CASE4_15_NEW_ITEMS 	15 /* GN7_3.5GDI_PONDOWN_KD.tsv - Original data nums */




#define QUAL_2ND_DATA_ITEM_NUM 		29 /* 2023-03-04, +LPFLAcc, +EngTemp06, +NetEng_Acor, +MSs_Ntg, */
#define QUAL_2ND_NEWDT_ITEM_NUM 	31 /* 2023-03-04, +Gsum, +LPFLAcc, +EngTemp06, +NetEng_Acor, +MSs_Ntg */

#define QUAL_3RD_DATA_ITEM_NUM 		32 /* 2023-03-04, +Gsum, +LPFLAcc, +EngTemp06, +NetEng_Acor, +MSs_Ntg  */

#define QUAL_4TH_DOWNSHI_ITEM_NUM 	35 /* 2023-03-09, +MSs_Ntg, New +sTimeMSpos, +MaxMSs, +minMSs  */


#define QUAL_FLT_DATA_ITEM_NUM 		31 /* 2023-03-04, +Gavg, +LPFLAcc, +EngTemp06, +NetEng_Acor, +MSs_Ntg  */
#define QUAL_APS_DATA_ITEM_NUM 		30 /* 2023-03-04, +Gavg, +LPFLAcc, -tqi07, -TqFr13, +EngTemp06, +NetEng_Acor, +MSs_Ntg  */


#define QUAL_FLT_Down_ITEM_NUM 		34 /* 2023-03-11,  */
#define QUAL_VS_Down_ITEM_NUM 		34 /* 2023-03-12  */



/* ---- Just Debug Message ---- */
#define DEBUG_MSG_1ST_POINT_TIME 		0
#define DEBUG_MSG_OVER_TIME 			0

#define DEBUG_GMAX_GMIN_LOG 			0
#define DEBUG_GMAX_GMIN_LOG2 			0

#endif /* __DEFS_H__ */



