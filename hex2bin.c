
/************************************************************************************
 * @file    : hex2bin.c 
 * @brief   : hex2bin
 *           
 *           
 * @version : 2.0
 * @date    : 2014/06/30
 * @author  : Public author 
 *            tp.joo@daum.net
 ************************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <math.h>  
#include <assert.h>	/* assert() */
#include <unistd.h>
#include <memory.h> 
#include <ctype.h> 

#include "common.h"
#include "feature.h"
#include "defs.h"



//#define DEBUG
#ifdef _MOT2BIN_
/* Size in bytes.  Max length must be in powers of 2: 1,2,4,8,16,32, etc. */
#define MOT2BIN_MEMORY_SIZE 		(4*1024*1024)  /* 4MBytes */
#define MOT2BIN_ADDRESS_MASK 		(MOT2BIN_MEMORY_SIZE-1)

#define MOT2BIN_MAX_LINE_SIZE 		256
#endif

#ifdef _INTEL2BIN_

#define MAX_BIN_LENGTH_SIZE 		0x800000 // Max_Length 8MByte Limit

#define HEX_MAX_LINE_SIZE 			1024
#define MAX_ERR_COUNT 				10 // error count


/* Size in bytes.  Max length must be in powers of 2: 1,2,4,8,16,32, etc. */
#define INT2BIN_MEMORY_SIZE 		(4*1024*1024) /* 4MBytes */

/* This mask is for mapping the target binary inside our
binary buffer. If for example, we are generating a binary
file with records starting at FFF00000, the bytes will be
stored at the beginning of the memory buffer. */
#define INT2BIN_ADDRESS_MASK 		(INT2BIN_MEMORY_SIZE-1)

/* The data records can contain 255 bytes: this means 512 characters. */
#define INT2BIN_MAX_LINE_SIZE 		1024

#define NO_ADDRESS_TYPE_SELECTED 	0
#define LINEAR_ADDRESS 				1
#define SEGMENTED_ADDRESS 			2
#endif



#if defined(_MOT2BIN_) || defined(_INTEL2BIN_)

int Pad_Byte = 0xFF;  // --join --intel --motorola
int isPadByte = 0, isPadByteAllArea=0;

int Enable_Checksum_Error = 1; // check checksum
int Status_Checksum_Error = false;
unsigned int checksum_err_cnt = 0;
unsigned char Checksum;
unsigned int Record_Nb;
	
/* This will hold binary codes translated from hex file. */
unsigned char *Memory_Block = NULL;
unsigned int Starting_Address, Phys_Addr;
unsigned int Records_Start; // Lowest address of the records
unsigned int Max_Length = 0; // MOT2BIN_MEMORY_SIZE;
unsigned int Real_Length = 0; // 2020.06.24, real file size

unsigned int Minimum_Block_Size = 0x1000; // 4096 byte
unsigned int Floor_Address = 0x0;  
unsigned int Ceiling_Address = 0xFFFFFFFF; 
int Module;
int Minimum_Block_Size_Setted = 0;
int Starting_Address_Setted = 0;
int Floor_Address_Setted = 0;
int Ceiling_Address_Setted = 0;
int Max_Length_Setted = 0;
int Swap_Wordwise = 0;
int Address_Alignment_Word = 0;
int Batch_Mode = 0;
//int Verbose_Flag = 0;

int Endian = 0;  /* 0:little-endian,  1:big-endian */



// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#define HEX2BIN_MOTOROLA_ZERO_FORCED 	1 // 2020.06.26
#define HEX2BIN_INTEL_ZERO_FORCED 		1 // 2020.06.30

#if 1 // HEX2BIN_MOTOROLA_ZERO_FORCED // 2020.06.24

enum {
	HEX2BIN_REAL_ADDR = 0,  // Never modified!!!
	HEX2BIN_ZERO_FORCED,    // read address is too big, so read address match to zero address!!!
	HEX2BIN_HIGH_ADDR,

	HEX2BIN_MAX = 0xff
};

int Enable_HexaAddr_Zero_Forced = HEX2BIN_REAL_ADDR; // 2016.03.10, hex file address zero-forced!!
unsigned int Phys_AddrTemp = 0x00000000; // added at 2016.03.10
unsigned int iOnceCheck = 1; // added at 2016.03.10

#endif
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


#define LAST_CHECK_METHOD 5

typedef enum Crc
{
    CHK8_SUM =0,
    CHK16,
    CRC8,
    CRC16,
    CRC32,
    CHK16_8
} t_CRC;

t_CRC Cks_Type = CHK8_SUM;
unsigned int Cks_Start = 0, Cks_End = 0, Cks_Addr = 0, Cks_Value = 0;
int Cks_range_set = 0;
int Cks_Addr_set = 0;
int Force_Value = 0;

//char strHex2BinLen[MAX_CHARS+1];
//char strPadByte[MAX_CHARS+1];
//char strPadArea[MAX_CHARS+1];

unsigned int Crc_Poly = 0x07, Crc_Init = 0, Crc_XorOut = 0;
int Crc_RefIn = false;
int Crc_RefOut = false;


/* flag that a file was read */
int Fileread = 0;



/* This will hold binary codes translated from hex file. */
unsigned char	Hex2Bin_Checksum = 0;

/* Application specific */
unsigned int 	Nb_Bytes;
unsigned int 	First_Word, Segment, Upper_Address;
unsigned int    Address, Lowest_Address, Highest_Address, Starting_Address;
unsigned int    Real_Highest_Address=0;
unsigned int 	Records_Start; // Lowest address of the records


/* This mask is for mapping the target binary inside the binary buffer. 
If for example, we are generating a binary file with records starting at FFF00000, 
the bytes will be stored at the beginning of the memory buffer.  */
unsigned int    Address_Mask = (MOT2BIN_MEMORY_SIZE-1);

unsigned int    Record_Count, Record_Checksum;

char str_address[MAX_CHARS+1];




// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// ++ functions 
// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

unsigned char u16_hi(unsigned short value)
{
	return (unsigned char)((value & 0xFF00) >> 8);
}

unsigned char u16_lo(unsigned short value)
{
	return (unsigned char)(value & 0x00FF);
}

unsigned char u32_b3(unsigned int value)
{
	return (unsigned char)((value & 0xFF000000) >> 24);
}

unsigned char u32_b2(unsigned int value)
{
	return (unsigned char)((value & 0x00FF0000) >> 16);
}

unsigned char u32_b1(unsigned int value)
{
	return (unsigned char)((value & 0x0000FF00) >> 8);
}

unsigned char u32_b0(unsigned int value)
{
	return (unsigned char)(value & 0x000000FF);
}

unsigned char u64_b7(unsigned long long value)
{
	return (unsigned char)((value & 0xFF00000000000000ULL) >> 56);
}

unsigned char u64_b6(unsigned long long value)
{
	return (unsigned char)((value & 0x00FF000000000000ULL) >> 48);
}

unsigned char u64_b5(unsigned long long value)
{
	return (unsigned char)((value & 0x0000FF0000000000ULL) >> 40);
}

unsigned char u64_b4(unsigned long long value)
{
	return (unsigned char)((value & 0x000000FF00000000ULL) >> 32);
}

unsigned char u64_b3(unsigned long long value)
{
	return (unsigned char)((value & 0x00000000FF000000ULL) >> 24);
}

unsigned char u64_b2(unsigned long long value)
{
	return (unsigned char)((value & 0x0000000000FF0000ULL) >> 16);
}

unsigned char u64_b1(unsigned long long value)
{
	return (unsigned char)((value & 0x000000000000FF00ULL) >> 8);
}

unsigned char u64_b0(unsigned long long value)
{
	return (unsigned char)(value & 0x00000000000000FFULL);
}

/* Checksum/CRC conversion to ASCII */
unsigned char nibble2ascii(unsigned char value)
{
  unsigned char result = value & 0x0f;

  if (result > 9) return result + 0x41-0x0A;
  else return result + 0x30;
}

bool cs_isdecdigit(char c)
{
    return (c >= 0x30) && (c < 0x3A);
}

unsigned char tohex(unsigned char c)
{
  if ((c >= '0') && (c < '9'+1))
    return (c - '0');
  if ((c >= 'A') && (c < 'F'+1))
    return (c - 'A' + 0x0A);
  if ((c >= 'a') && (c < 'f'+1))
    return (c - 'a' + 0x0A);

  return 0;
}

unsigned char todecimal(unsigned char c)
{
  if ((c >= '0') && (c < '9'+1))
    return (c - '0');

  return 0;
}


const unsigned char Reflect8[256] = {
  0x00,0x80,0x40,0xC0,0x20,0xA0,0x60,0xE0,0x10,0x90,0x50,0xD0,0x30,0xB0,0x70,0xF0,
  0x08,0x88,0x48,0xC8,0x28,0xA8,0x68,0xE8,0x18,0x98,0x58,0xD8,0x38,0xB8,0x78,0xF8,
  0x04,0x84,0x44,0xC4,0x24,0xA4,0x64,0xE4,0x14,0x94,0x54,0xD4,0x34,0xB4,0x74,0xF4,
  0x0C,0x8C,0x4C,0xCC,0x2C,0xAC,0x6C,0xEC,0x1C,0x9C,0x5C,0xDC,0x3C,0xBC,0x7C,0xFC,
  0x02,0x82,0x42,0xC2,0x22,0xA2,0x62,0xE2,0x12,0x92,0x52,0xD2,0x32,0xB2,0x72,0xF2,
  0x0A,0x8A,0x4A,0xCA,0x2A,0xAA,0x6A,0xEA,0x1A,0x9A,0x5A,0xDA,0x3A,0xBA,0x7A,0xFA,
  0x06,0x86,0x46,0xC6,0x26,0xA6,0x66,0xE6,0x16,0x96,0x56,0xD6,0x36,0xB6,0x76,0xF6,
  0x0E,0x8E,0x4E,0xCE,0x2E,0xAE,0x6E,0xEE,0x1E,0x9E,0x5E,0xDE,0x3E,0xBE,0x7E,0xFE,
  0x01,0x81,0x41,0xC1,0x21,0xA1,0x61,0xE1,0x11,0x91,0x51,0xD1,0x31,0xB1,0x71,0xF1,
  0x09,0x89,0x49,0xC9,0x29,0xA9,0x69,0xE9,0x19,0x99,0x59,0xD9,0x39,0xB9,0x79,0xF9,
  0x05,0x85,0x45,0xC5,0x25,0xA5,0x65,0xE5,0x15,0x95,0x55,0xD5,0x35,0xB5,0x75,0xF5,
  0x0D,0x8D,0x4D,0xCD,0x2D,0xAD,0x6D,0xED,0x1D,0x9D,0x5D,0xDD,0x3D,0xBD,0x7D,0xFD,
  0x03,0x83,0x43,0xC3,0x23,0xA3,0x63,0xE3,0x13,0x93,0x53,0xD3,0x33,0xB3,0x73,0xF3,
  0x0B,0x8B,0x4B,0xCB,0x2B,0xAB,0x6B,0xEB,0x1B,0x9B,0x5B,0xDB,0x3B,0xBB,0x7B,0xFB,
  0x07,0x87,0x47,0xC7,0x27,0xA7,0x67,0xE7,0x17,0x97,0x57,0xD7,0x37,0xB7,0x77,0xF7,
  0x0F,0x8F,0x4F,0xCF,0x2F,0xAF,0x6F,0xEF,0x1F,0x9F,0x5F,0xDF,0x3F,0xBF,0x7F,0xFF,
};

unsigned short Reflect16(unsigned short Value16)
{
  return (((unsigned short) Reflect8[u16_lo(Value16)]) << 8) | ((unsigned short) Reflect8[u16_hi(Value16)]);
}

unsigned int Reflect24(unsigned int Value24)
{
  return (
		  (((unsigned int) Reflect8[u32_b0(Value24)]) << 16) |
		  (((unsigned int) Reflect8[u32_b1(Value24)]) << 8)  |
		   ((unsigned int) Reflect8[u32_b2(Value24)])
		  );
}

unsigned int Reflect32(unsigned int Value32)
{
  return (
		  (((unsigned int) Reflect8[u32_b0(Value32)]) << 24) |
		  (((unsigned int) Reflect8[u32_b1(Value32)]) << 16) |
		  (((unsigned int) Reflect8[u32_b2(Value32)]) << 8)  |
		   ((unsigned int) Reflect8[u32_b3(Value32)])
		  );
}

unsigned long long Reflect40(unsigned long long Value40)
{
  return (
		  (((unsigned long long) Reflect8[u64_b0(Value40)]) << 32) |
		  (((unsigned long long) Reflect8[u64_b1(Value40)]) << 24) |
		  (((unsigned long long) Reflect8[u64_b2(Value40)]) << 16) |
		  (((unsigned long long) Reflect8[u64_b3(Value40)]) << 8)  |
		   ((unsigned long long) Reflect8[u64_b4(Value40)])
		  );
}

unsigned long long Reflect64(unsigned long long Value64)
{
  return (
		  (((unsigned long long) Reflect8[u64_b0(Value64)]) << 56) |
		  (((unsigned long long) Reflect8[u64_b1(Value64)]) << 48) |
		  (((unsigned long long) Reflect8[u64_b2(Value64)]) << 40) |
		  (((unsigned long long) Reflect8[u64_b3(Value64)]) << 32) |
		  (((unsigned long long) Reflect8[u64_b4(Value64)]) << 24) |
		  (((unsigned long long) Reflect8[u64_b5(Value64)]) << 16) |
		  (((unsigned long long) Reflect8[u64_b6(Value64)]) << 8)  |
		   ((unsigned long long) Reflect8[u64_b7(Value64)])
		  );
}



#ifndef G_GUINT64_CONSTANT
#define G_GUINT64_CONSTANT(val) (val##UL)
#endif

void *crc_table = NULL;

/* private */

void init_crc8_normal_tab(unsigned char polynom)
{
  int i, j;
  unsigned char crc;
  unsigned char *p;

  p = (unsigned char *) crc_table;

  for (i=0; i<256; i++)
    {
	  crc = (unsigned char) i;

	  for (j=0; j<8; j++)
        {
          if (crc & 0x80) crc = (crc << 1) ^ polynom;
          else            crc <<= 1;
        }
	  *p++ = crc;
    }
}

void init_crc8_reflected_tab(unsigned char polynom)
{
  int i, j;
  unsigned char crc;
  unsigned char *p;

  p = (unsigned char *) crc_table;

  for (i=0; i<256; i++)
    {
	  crc = (unsigned char) i;

	  for (j=0; j<8; j++)
        {
          if (crc & 0x01) crc = (crc >> 1) ^ polynom;
          else            crc >>= 1;
        }
	  *p++ = crc;
    }
}

/* Common routines for calculations */
void init_crc16_normal_tab(unsigned short polynom)
{
  int i, j;
  unsigned short crc;
  unsigned short *p;

  p = (unsigned short *) crc_table;

  for (i=0; i<256; i++)
    {
	  crc = ((unsigned short) i) << 8;

	  for (j=0; j<8; j++)
        {
		  if ( crc & 0x8000 ) crc = ( crc << 1 ) ^ polynom;
		  else                crc <<= 1;
        }
	  *p++ = crc;
    }
}

void init_crc16_reflected_tab(unsigned short polynom)
{
  int i, j;
  unsigned short crc;
  unsigned short *p;

  p = (unsigned short *) crc_table;

  for (i=0; i<256; i++)
    {
	  crc   = (unsigned short) i;

	  for (j=0; j<8; j++)
        {
		  if ( crc & 0x0001 ) crc = ( crc >> 1 ) ^ polynom;
		  else                crc >>= 1;
        }
	  *p++ = crc;
    }
}

void init_crc32_normal_tab(unsigned int polynom)
{
  int i, j;
  unsigned int crc;
  unsigned int *p;

  p = (unsigned int *) crc_table;

  for (i=0; i<256; i++)
    {
	  crc = ((unsigned int) i) << 24;

	  for (j=0; j<8; j++)
        {
		  if ( crc & 0x80000000L ) crc = ( crc << 1 ) ^ polynom;
		  else                     crc <<= 1;
        }
	  *p++ = crc;
    }
}

void init_crc32_reflected_tab(unsigned int polynom)
{
  int i, j;
  unsigned int crc;
  unsigned int *p;

  p = (unsigned int *) crc_table;

  for (i=0; i<256; i++)
    {
	  crc = (unsigned int) i;

	  for (j=0; j<8; j++)
        {
		  if ( crc & 0x00000001L ) crc = ( crc >> 1 ) ^ polynom;
		  else                     crc >>= 1;
        }
	  *p++ = crc;
    }
}

/* Common routines for calculations */

unsigned char update_crc8(unsigned char crc, unsigned char c)
{
  return (((unsigned char *) crc_table)[crc ^ c]);
}

unsigned short update_crc16_normal(unsigned short crc, char c )
{
  unsigned short short_c;

  short_c  = 0x00ff & (unsigned short) c;

  /* Normal form */
  return (crc << 8) ^ ((unsigned short *) crc_table)[(crc >> 8) ^ short_c];
}

unsigned short update_crc16_reflected(unsigned short crc, char c )
{
  unsigned short short_c;

  short_c  = 0x00ff & (unsigned short) c;

  /* Reflected form */
  return (crc >> 8) ^ ((unsigned short *) crc_table)[(crc ^ short_c) & 0xff];
}

unsigned int update_crc32_normal(unsigned int crc, char c )
{
  unsigned int long_c;

  long_c = 0x000000ffL & (unsigned int) c;

  return (crc << 8) ^ ((unsigned int *) crc_table)[((crc >> 24) ^ long_c) & 0xff];
}

unsigned int update_crc32_reflected(unsigned int crc, char c )
{
  unsigned int long_c;

  long_c = 0x000000ffL & (unsigned int) c;

  return (crc >> 8) ^ ((unsigned int *) crc_table)[(crc ^ long_c) & 0xff];
}





void *NoFailMalloc (size_t size)
{
    void *result = NULL;

    if ((result = malloc (size)) == NULL)
    {
        fprintf (stderr,"Can't allocate memory. size(%lld) \n", size);
        exit(1);
    }
    return (result);
}


// 0 or 1
int GetBin(const char *str)
{
    int result;
    unsigned int value;

    result = sscanf(str,"%u",&value);

    if (result == 1) return value & 1;
    else
    {
        fprintf(stderr,"GetBin: some error occurred.\n");
        exit (1);
    }
}

int GetDec(const char *str)
{
    int result;
    unsigned int value;

    result = sscanf(str,"%u",&value);

    if (result == 1) return value;
    else
    {
        fprintf(stderr,"GetDec: some error occurred. \n");
        exit (1);
    }
}

int GetHex(const char *str)
{
    int result;
    unsigned int value;

    result = sscanf(str,"%x",&value);

    if (result == 1) return value;
    else
    {
        fprintf(stderr,"GetHex: some error occurred. \n");
        exit (1);
    }
}


// Char t/T: true f/F: false
bool GetBoolean(const char *str)
{
    int result;
    unsigned char value, temp;

    result = sscanf(str,"%c",&value);
    temp = tolower(value);

    if ((result == 1) && ((temp == 't') || (temp == 'f')))
    {
        return (temp == 't');
    }
    else
    {
        fprintf(stderr,"GetBoolean: some error occurred when parsing options.\n");
        exit (1);
    }
}


void VerifyChecksumValue(unsigned int checksum, unsigned int hexFam)
{
	if( hexFam==1 ) /* INTEL Family */
	{
	    if ((Checksum != 0) && Enable_Checksum_Error)
		{
			fprintf(stderr,"[++ERROR++]Checksum error in record %d: less then 0x%02x, should be 0x%02x. \n",
				Record_Nb, (256-Checksum)&0xFF, (checksum + ((256-Checksum) & 0xFF))&0xFF );

			checksum_err_cnt ++;	

			Status_Checksum_Error = true;
		}
	}
	else // if( hexFam==2 ) /* MOTOROLA Family */
	{
		if (((checksum + Checksum) != 0xFF) && Enable_Checksum_Error)
		{
			fprintf(stderr,"[++ERROR++]Checksum error in Line%6d: should be 0x%02x, not be 0x%02x. \n",
				Record_Nb, 255-Checksum, checksum ); 

			checksum_err_cnt ++;	

			Status_Checksum_Error = true;
		}

	}
}

/* Check if are set Floor and Ceiling Address and range is coherent*/
void VerifyRangeFloorCeil(void)
{
    if (Floor_Address_Setted && Ceiling_Address_Setted && (Floor_Address >= Ceiling_Address))
    {
        printf("Floor address %#08x higher than Ceiling address %#08x \n",Floor_Address,Ceiling_Address);
        exit(1);
    }
}

void CrcParamsCheck(void)
{
    switch (Cks_Type)
    {
    case CRC8:
        Crc_Poly &= 0xFF;
        Crc_Init &= 0xFF;
        Crc_XorOut &= 0xFF;
        break;
    case CRC16:
        Crc_Poly &= 0xFFFF;
        Crc_Init &= 0xFFFF;
        Crc_XorOut &= 0xFFFF;
        break;
    case CRC32:
        break;
    default:
        fprintf (stderr,"See file CRC list.txt for parameters\n");
        exit(1);
    }
}

void WriteMemBlock16(unsigned short Value)
{
    if (Endian == 1) /* 1: Big Endian */
    {
        Memory_Block[Cks_Addr - Lowest_Address]    = u16_hi(Value);
        Memory_Block[Cks_Addr - Lowest_Address +1] = u16_lo(Value);
    }
    else
    {
        Memory_Block[Cks_Addr - Lowest_Address +1] = u16_hi(Value);
        Memory_Block[Cks_Addr - Lowest_Address]    = u16_lo(Value);
    }
}

void WriteMemBlock32(unsigned int Value)
{
    if (Endian == 1) /* 1: Big Endian */
    {
        Memory_Block[Cks_Addr - Lowest_Address]    = u32_b3(Value);
        Memory_Block[Cks_Addr - Lowest_Address +1] = u32_b2(Value);
        Memory_Block[Cks_Addr - Lowest_Address +2] = u32_b1(Value);
        Memory_Block[Cks_Addr - Lowest_Address +3] = u32_b0(Value);
    }
    else
    {
        Memory_Block[Cks_Addr - Lowest_Address +3] = u32_b3(Value);
        Memory_Block[Cks_Addr - Lowest_Address +2] = u32_b2(Value);
        Memory_Block[Cks_Addr - Lowest_Address +1] = u32_b1(Value);
        Memory_Block[Cks_Addr - Lowest_Address]    = u32_b0(Value);
    }
}

void WriteMemory(void)
{
	unsigned int i;

    if ((Cks_Addr >= Lowest_Address) && (Cks_Addr < Highest_Address))
    {
        if(Force_Value)
        {
            switch (Cks_Type)
            {
                case 0:
                    Memory_Block[Cks_Addr - Lowest_Address] = Cks_Value;
                    fprintf(stdout,"Addr %#08x set to %#02x \n",Cks_Addr, Cks_Value);
                    break;
                case 1:
                    WriteMemBlock16(Cks_Value);
                    fprintf(stdout,"Addr %#08x set to %#04x \n",Cks_Addr, Cks_Value);
                    break;
                case 2:
                    WriteMemBlock32(Cks_Value);
                    fprintf(stdout,"Addr %#08x set to %#08x \n",Cks_Addr, Cks_Value);
                    break;
                default:;
            }
        }
        else if (Cks_Addr_set)
        {
            /* Add a checksum to the binary file */
            if (!Cks_range_set)
            {
                Cks_Start = Lowest_Address;
                Cks_End = Highest_Address;
            }
            /* checksum range MUST BE in the array bounds */

            if (Cks_Start < Lowest_Address)
            {
                fprintf(stdout,"Modifying range start from %X to %X\n",Cks_Start,Lowest_Address);
                Cks_Start = Lowest_Address;
            }
            if (Cks_End > Highest_Address)
            {
                fprintf(stdout,"Modifying range end from %X to %X\n",Cks_End,Highest_Address);
                Cks_End = Highest_Address;
            }

            crc_table = NULL;

            switch (Cks_Type)
            {
            case CHK8_SUM:
            {
                unsigned char wCKS = 0;

                for (i=Cks_Start; i<=Cks_End; i++)
                {
                    wCKS += Memory_Block[i - Lowest_Address];
                }

                fprintf(stdout,"8-bit Checksum = %#02x\n",wCKS & 0xff);
                Memory_Block[Cks_Addr - Lowest_Address] = wCKS;
                fprintf(stdout,"Addr %#08x set to %#02x\n",Cks_Addr, wCKS);
            }
            break;

            case CHK16:
            {
                unsigned short wCKS, w;

                wCKS = 0;

                if (Endian == 1) /* 1: Big Endian */
                {
                    for (i=Cks_Start; i<=Cks_End; i+=2)
                    {
                        w =  Memory_Block[i - Lowest_Address +1] | ((unsigned short)Memory_Block[i - Lowest_Address] << 8);
                        wCKS += w;
                    }
                }
                else
                {
                    for (i=Cks_Start; i<=Cks_End; i+=2)
                    {
                        w =  Memory_Block[i - Lowest_Address] | ((unsigned short)Memory_Block[i - Lowest_Address +1] << 8);
                        wCKS += w;
                    }
                }
                fprintf(stdout,"16-bit Checksum = %#04x\n",wCKS);
                WriteMemBlock16(wCKS);
                fprintf(stdout,"Addr %08X set to %04X\n",Cks_Addr, wCKS);
            }
            break;

            case CHK16_8:
            {
                unsigned short wCKS;

                wCKS = 0;

                for (i=Cks_Start; i<=Cks_End; i++)
                {
                    wCKS += Memory_Block[i - Lowest_Address];
                }

                fprintf(stdout,"16-bit Checksum = %04X\n",wCKS);
                WriteMemBlock16(wCKS);
                fprintf(stdout,"Addr %08X set to %04X\n",Cks_Addr, wCKS);
            }
            break;

            case CRC8:
            {
                unsigned char CRC8;
                crc_table = NoFailMalloc(256);

                if (Crc_RefIn)
                {
                    init_crc8_reflected_tab(Reflect8[Crc_Poly]);
                    CRC8 = Reflect8[Crc_Init];
                }
                else
                {
                    init_crc8_normal_tab(Crc_Poly);
                    CRC8 = Crc_Init;
                }

                for (i=Cks_Start; i<=Cks_End; i++)
                {
                    CRC8 = update_crc8(CRC8,Memory_Block[i - Lowest_Address]);
                }

                CRC8 = (CRC8 ^ Crc_XorOut) & 0xff;
                Memory_Block[Cks_Addr - Lowest_Address] = CRC8;
                fprintf(stdout,"Addr %08X set to %02X\n",Cks_Addr, CRC8);
            }
            break;

            case CRC16:
            {
                unsigned short CRC16;
                crc_table = NoFailMalloc(256 * 2);

                if (Crc_RefIn)
                {
                    init_crc16_reflected_tab(Reflect16(Crc_Poly));
                    CRC16 = Reflect16(Crc_Init);

                    for (i=Cks_Start; i<=Cks_End; i++)
                    {
                        CRC16 = update_crc16_reflected(CRC16,Memory_Block[i - Lowest_Address]);
                    }
                }
                else
                {
                    init_crc16_normal_tab(Crc_Poly);
                    CRC16 = Crc_Init;


                    for (i=Cks_Start; i<=Cks_End; i++)
                    {
                        CRC16 = update_crc16_normal(CRC16,Memory_Block[i - Lowest_Address]);
                    }
                }

                CRC16 = (CRC16 ^ Crc_XorOut) & 0xffff;
                WriteMemBlock16(CRC16);
                fprintf(stdout,"Addr %08X set to %04X\n",Cks_Addr, CRC16);
            }
            break;

            case CRC32:
            {
                unsigned int CRC32;

                crc_table = NoFailMalloc(256 * 4);
                if (Crc_RefIn)
                {
                    init_crc32_reflected_tab(Reflect32(Crc_Poly));
                    CRC32 = Reflect32(Crc_Init);

                    for (i=Cks_Start; i<=Cks_End; i++)
                    {
                        CRC32 = update_crc32_reflected(CRC32,Memory_Block[i - Lowest_Address]);
                    }
                }
                else
                {
                    init_crc32_normal_tab(Crc_Poly);
                    CRC32 = Crc_Init;

                    for (i=Cks_Start; i<=Cks_End; i++)
                    {
                        CRC32 = update_crc32_normal(CRC32,Memory_Block[i - Lowest_Address]);
                    }
                }

                CRC32 ^= Crc_XorOut;
                WriteMemBlock32(CRC32);
                fprintf(stdout,"Addr %#08x set to %#08x\n",Cks_Addr, CRC32);
            }
            break;

            default:
                ;
            }

            if (crc_table != NULL) free(crc_table);
        }
    }
    else
    {
        if(Force_Value || Cks_Addr_set)
        {
            fprintf (stderr,"Force/Check address outside of memory range\n");
        }
    }

    /* write binary file */
	if(outfile)
	{
	    fwrite (Memory_Block,
	            Max_Length,
	            1,
	            outfile);
	}

    free (Memory_Block);


    // Minimum_Block_Size is set; the memory buffer is multiple of this?
    if ( Minimum_Block_Size_Setted==1 )
    {
        Module = Max_Length % Minimum_Block_Size;
        if (Module)
        {
            Module = Minimum_Block_Size - Module;
            Memory_Block = (unsigned char *) NoFailMalloc(Module);
	#if 0 // 2020.06.24
            memset (Memory_Block,Pad_Byte,Module);
	#else
			if( 1==isPadByteAllArea ) // original version 2.5
				memset (Memory_Block,Pad_Byte,Module);
			else
				memset (Memory_Block,0xff /*Pad_Byte*/,Module);
	#endif
	
			if(outfile)
			{
	            fwrite (Memory_Block,
	                    Module,
	                    1,
	                    outfile);
			}

            if(Memory_Block) free (Memory_Block);

            if (Max_Length_Setted==1)
                fprintf(stdout,"Attention Max Length changed by Minimum Block Size\n");

            // extended 
            Max_Length += Module;
            Highest_Address += Module;
            fprintf(stdout,"Extended\nHighest address:  %#08x\n",Highest_Address);
            fprintf(stdout,"Max Length:       %u\n\n",Max_Length);

        }
    }
}


void Allocate_Memory_And_Rewind(void)
{

    Records_Start = Lowest_Address;

    if( (Starting_Address_Setted ==1) )
    {
        Lowest_Address = Starting_Address;
    }
    else
    {
        Starting_Address = Lowest_Address;
    }

    if (Max_Length_Setted == 0)
    {
        Max_Length = Highest_Address - Lowest_Address + 1;
    }
    else
    {
        Highest_Address = Lowest_Address + Max_Length - 1;

	#if 1 // HEX2BIN_MOTOROLA_ZERO_FORCED // 2020.06.24
		if( (Real_Length+1 > Max_Length) || (Real_Length > Highest_Address) )
		{
			// Real_Length(0xD34B) Max_Length(0xC800) Highest_Address (0xC7FF)
			printf("\n\nMax. length (%#x) is too small (%#x != %#x). \n", Max_Length, Real_Length, Highest_Address );

			if( HEX2BIN_HIGH_ADDR != Enable_HexaAddr_Zero_Forced ) // 2018.07.05
			{
				Highest_Address = Real_Length;
				Max_Length = Real_Length+1;
			}
		}
	#endif

    }
	printf("\n");
	printf("--------------------------------------------------------\n");
	printf("Converting hexa to binary \n");
	printf("--------------------------------------------------------\n");
	printf("Lowest address    : %#010x \n",Lowest_Address);
	printf("Highest address   : %#010x \n",Highest_Address);
	printf("Starting address  : %#010x \n",Starting_Address);
	printf("Bin file length   : %#010x (%u) -> %.2fkB \n",(Real_Length+1), (Real_Length+1), (float)(Real_Length+1)/1024.0 ); // bescause of 0,1..
	printf("Max Length        : %#010x (%u) -> %.2fkB \n",Max_Length, Max_Length, (float)(Max_Length/1024.0) );



#if defined(HEX2BIN_MOTOROLA_ZERO_FORCED) || defined(HEX2BIN_INTEL_ZERO_FORCED)// 2016.03.10
	// ----------------------------------------
	// Hex Address Zero based converted 
	// ----------------------------------------
	if( HEX2BIN_ZERO_FORCED == Enable_HexaAddr_Zero_Forced )
	{
		if( Phys_AddrTemp )
		{
			printf("------ [ Zero-Address Forced Logic ] -------------------\n");
			printf("Hex file real start addr : 0x%08X \n", Lowest_Address + Phys_AddrTemp );
			printf("Hex real Highest address : 0x%08X <- %.2fkB \n", Highest_Address + Phys_AddrTemp, (float)(Highest_Address + Phys_AddrTemp)/1024.0 );
			printf("Hex real starting addr.  : 0x%08X \n", Starting_Address + Phys_AddrTemp );
			printf("--------------------------------------------------------\n");
		}
	}
#endif



	if( Max_Length > MAX_BIN_LENGTH_SIZE ) // 2016.03.07
	{
		printf("\n");
		printf("The parsed binary length is %.2fMB. Maximum binary limit is %.2fMBytes!! \n", (float)Max_Length/(1024.0*1024.0), MAX_BIN_LENGTH_SIZE/1024.0/1024.0 );
		printf("Use the Zero-Address Forced option!!! ( --zeroforced ) \n\n"); 

		//AllFilesClosed();		

		if(inpfile) { fclose(inpfile);	inpfile=NULL; }
		if(outfile) { fclose(outfile);	outfile=NULL; }
		if(data_buf){ free(data_buf);	data_buf=NULL; }

		exit(0);
		return;
	}
	else
	{

    /* Now that we know the buffer size, we can allocate it. */
    /* allocate a buffer */
    Memory_Block = (unsigned char *) NoFailMalloc(Max_Length);

	}
    /* For EPROM or FLASH memory types, fill unused bytes with FF or the value specified by the p option */

#if 0 // 2020.06.24
    memset (Memory_Block,Pad_Byte,Max_Length);
#else

	if( 1==isPadByteAllArea ) // original version 2.5
	{
		memset (Memory_Block, Pad_Byte, Max_Length);
	}
	else
	{
		memset (Memory_Block,0xff /*Pad_Byte*/,Max_Length);
		if( Max_Length-1 > Real_Length )
		memset (&Memory_Block[Real_Length+1],Pad_Byte, (Max_Length-Real_Length-1) );
	}
#endif

    rewind(inpfile);
}



char *ReadDataBytes(char *p)
{
	unsigned int i,temp2;
	int result;

	/* Read the Data bytes. */
	/* Bytes are written in the Memory block even if checksum is wrong. */
	i = Nb_Bytes;

	do
	{
		result = sscanf (p, "%2x",&temp2);
		if (result != 1) fprintf(stderr,"Error in line %d of hex file. \n", Record_Nb);
		p += 2;

		/* Check that the physical address stays in the buffer's range. */

		if (Phys_Addr < Max_Length)
		{
			/* Overlapping record will erase the pad bytes */
			if (Swap_Wordwise)
			{
				if (Memory_Block[Phys_Addr ^ 1] != Pad_Byte) fprintf(stderr,"\bOverlapped record detected (%d) Pad(%#08x -> %#x) \r", Swap_Wordwise, Phys_Addr, Pad_Byte);
				Memory_Block[Phys_Addr++ ^ 1] = temp2;
			}
			else
			{
				// ~if (Memory_Block[Phys_Addr] != Pad_Byte) fprintf(stderr,"\bOverlapped record detected Pad(%#08x -> %#x) \r", Phys_Addr, Pad_Byte);
				Memory_Block[Phys_Addr++] = temp2;
			}

			Checksum = (Checksum + temp2) & 0xFF;
		}
	}
	while (--i != 0);

	return p;
}



UINT str2hex(char *str)
{
	int i;
	UINT number=0; 
	int order=1;
	char ch;

	for(i=strlen(str)-1; i>=0; i--)
	{
		ch=str[i];

		if(ch=='x' || ch=='X') break;

		if(ch=='.' || ch=='-') return 0; 

		if(ch>='0' && ch<='9')
		{
		    number+=order*(ch-'0');
		    order*=16;
		}
	
		if(ch>='A' && ch<='F')
		{
		    number+=order*(ch-'A'+10);
		    order*=16;
		}
		if(ch>='a' && ch<='f')
		{
		    number+=order*(ch-'a'+10);
		    order*=16;
		}

	}
	return number;
}


UINT str2int(char *str)
{
	int i;
	UINT number=0; 
	int order=1;
	char ch;

	for(i=strlen(str)-1;i>=0;i--)
	{
		ch=str[i];
#if 0
		if(ch=='x' || ch=='X')break;
#else
		if(ch=='.' || ch=='-') return 0; 
#endif

		if(ch>='0' && ch<='9')
		{
		    number+=order*(ch-'0');
		    order*=10;
		}

	}
	return number;
}


#endif /// _MOT2BIN_ || _INTEL2BIN_




void help_int2bin_form(void)
{

 printf("Intel Hex formats \n"
        "================= \n"
        "  \n"
        "Hexadecimal values are always in uppercase. Each line is a record. \n"
        "The sum of all the bytes in each record should be 00 (modulo 256). \n"
        "  \n"
        "Record types:  \n"
        "  \n"
        "00: data records  \n"
        "01: end-of-file record  \n"
        "02: extended address record \n"
        " \n"
        "Data record \n"
        "-----------  \n"
        "	:0D011C0000000000C3E0FF0000000000C30F   \n"
        "  \n"
        ": 0D 011C 00 00000000C3E0FF0000000000C3 0F  \n"
        "|  |   |   | -------------+------------  |  \n"
        "|  |   |   |              |              +--- Checksum    \n"
        "|  |   |   |              +------------------ Data bytes  \n"
        "|  |   |   +--------------------------------- Record type  \n"
        "|  |   +------------------------------------- Address  \n"
        "|  +----------------------------------------- Number of data bytes  \n"
        "+-------------------------------------------- Start of record  \n"
        "  \n"
        "End of file record  \n"
        "------------------  \n"
        "	:00000001FE  \n"
        "  \n"
        ": 00 0000 01 FE  \n"
        "|  |   |   |  |  \n"
        "|  |   |   |  +--- Checksum  \n"
        "|  |   |   +------ Record type  \n"
        "|  |   +---------- Address  \n"
        "|  +-------------- Number of data bytes  \n"
        "+----------------- Start of record  \n"
        "  \n"
        "Extended address record  \n"
        "-----------------------  \n"
        "	:02010002E0001B  \n"
        "  \n"
        ": 02 0100 02 E000 1B  \n"
        "|  |   |   |  |    |  \n"
        "|  |   |   |  |    +--- Checksum  \n"
        "|  |   |   |  +-------- Segment address  \n"
        "|  |   |   +----------- Record type  \n"
        "|  |   +--------------- Address  \n"
        "|  +------------------- Number of data bytes  \n"
        "+---------------------- Start of record  \n"
        "  \n"
        "Following data records will start at E000:0100 or E0100  \n");

	exit(0);

}



void help_mot2bin_form(void)
{

 printf("Motorola S-Record Format \n"
        "======================== \n"
        "\n"
        "    A file in Motorola S-record format is an ASCII file. There are three different  \n"
        "    formats:   \n"
        "   \n"
        "        S19     for 16-bit address   \n"
        "        S2      for 24-bit address   \n"
        "        S3      for 32-bit address   \n"
        "   \n"
        "   \n"
        "    The files consist of optional symbol table information, data specifications    \n"
        "    for loading memory, and a terminator record.   \n"
        "   \n"
        "        [ $$ {module_record}   \n"
        "        symbol records   \n"
        "        $$ [ module_record ]   \n"
        "        symbol_records   \n"
        "        $$]   \n"
        "        header_record   \n"
        "        data_records   \n"
        "        record_count_record   \n"
        "        terminator_record   \n"
        "   \n"
        "   \n"
        "Module Record (Optional)   \n"
        "   \n"
        "    Each object file contains one record for each module that is a component of it. This   \n"
        "    record contains the name of the module. There is one module record for each relocatable    \n"
        "    object created by the assembler. The name of the relocatable object module   \n"
        "    contained in the record comes from the IDNT directive. For absolute objects created   \n"
        "    by the linker, there is one module record for each relocatable object file linked,   \n"
        "    plus an additional record whose name comes from the NAME command for the   \n"
        "    linker.   \n"
        "   \n"
        "    Example:   \n"
        "        $$ MODNAME   \n"
        "   \n"
        "Symbol Record (Optional)   \n"
        "   \n"
        "    As many symbol records as needed can be contained in the object module. Up to 4   \n"
        "    symbols per line can be used, but it is not mandatory that each line contain 4    \n"
        "    symbols. A module can contain only symbol records.   \n"
        "   \n"
        "    Example:   \n"
        "        APPLE $00000 LABEL1 $ODOC3   \n"
        "        MEM $OFFFF ZEEK $01947   \n"
        "   \n"
        "    The module name associated with the symbols can be specified in the   \n"
        "    module_record preceding the symbol records.   \n"
        "   \n"
        "    Example:   \n"
        "        $$MAIN   \n"
        "   \n"
        "    Symbols are assumed to be in the module named in the preceding module_record   \n"
        "    until another module is specified with another module_record. Symbols defined by   \n"
        "    the linker's PUBLIC command appear following the first module record, which   \n"
        "    indicates the name of the output object module specified by the linker's NAME   \n"
        "    command.   \n"
        "   \n"
        "*****************************************************************************************   \n"
        "Header Record (SO)   \n"
        "   \n"
        "    Each object module has exactly one header record with the following format:   \n"
        "   \n"
        "        S00600004844521B   \n"
        "   \n"
        "    Description:   \n"
        "        S0         Identifies the record as a header record   \n"
        "        06         The number of bytes following this one   \n"
        "        0000       The address field, which is ignored   \n"
        "        484452     The string HDR in ASCII   \n"
        "        1B         The checksum   \n"
        "   \n"
        "*****************************************************************************************   \n"
        "   \n"
        "Data Record (S1)   \n"
        "   \n"
        "    A data record specifies data bytes that are to be loaded into memory. Figure 1   \n"
        "    shows the format for such a record. The columns shown in the figure represent half   \n"
        "    of a byte (4 bits).   \n"
        "   \n"
        "                ---------------------------------------------   \n"
        "                |  1 2  3 4   5 6 7 8   9 ... 40    41 42   |   \n"
        "                |                                           |   \n"
        "                |  S ID byte   load    data...data checksum |   \n"
        "                |       count address   1      n            |   \n"
        "                ---------------------------------------------   \n"
        "            Figure 1: Data Record Formatter 16-Bit Load Address \n"
        "   \n"
        "        Column      Description   \n"
        "   \n"
        "        1           Contains the ASCII character S, which indicates the start of   \n"
        "                    a record in Motorola S-record format.   \n"
        "        2           Contains the ASCII character identifying the record type.   \n"
        "                    For data records, this character is 1.   \n"
        "        3 to 4      Contain the count of the number of bytes following this one   \n"
        "                    within the record. The count includes the checksum and the   \n"
        "                    load address bytes but not the byte count itself.   \n"
        "        5 to 8      Contain the load address. The first data byte is to be loaded   \n"
        "                    into this address and subsequent bytes into the next sequential   \n"
        "                    address. Columns 5 and 6 contain the high-order address   \n"
        "                    byte, and columns 7 and 8 contain the low-order address byte.   \n"
        "        9 to 40     Contain the specifications for up to 16 bytes of data.   \n"
        "        41 to 42    Contain a checksum for the record. To calculate this, take the   \n"
        "                    sum of the values of all bytes from the byte count up to the   \n"
        "                    last data byte, inclusive, modulo 256. Subtract this result   \n"
        "                    from 255.   \n"
        "   \n"
        "*****************************************************************************************   \n"
        "   \n"
        "Data Record (S2)   \n"
        "   \n"
        "    A data record specifies data bytes that are to be loaded into memory. Figure 2   \n"
        "    shows the format for such a record. The columns shown in the figure represent half   \n"
        "    of a byte (4 bits).   \n"
        "   \n"
        "                ----------------------------------------------------   \n"
        "                |  1 2   3 4   5 6 7 8 9 10   11 ...  42   43 44   |   \n"
        "                |                                                  |   \n"
        "                |  S ID  byte     load        data...data checksum |   \n"
        "                |        count   address       1      n            |   \n"
        "                ----------------------------------------------------   \n"
        "                Figure 2: Data Record Format for 24-Bit Load Address   \n"
        "   \n"
        "        Column      Description   \n"
        "   \n"
        "        1           Contains the ASCII character S, which indicates the start of   \n"
        "                    a record in Motorola S-record format.   \n"
        "        2           Contains the ASCII character identifying the record type.   \n"
        "                    For data records, this character is 2.   \n"
        "        3 to 4      Contain the count of the number of bytes following this one   \n"
        "                    within the record. The count includes the checksum and the   \n"
        "                    load address bytes but not the byte count itself.   \n"
        "        5 to 10     Contain the load address. The first data byte is to be loaded   \n"
        "                    into this address and subsequent bytes into the next sequential    \n"
        "                    address. Columns 5 and 6 contain the high-order address   \n"
        "                    byte, and columns 9 and 10 contain the low-order address byte.   \n"
        "        11 to 42    Contain the specifications for up to 16 bytes of data.   \n"
        "        43 to 44    Contain a checksum for the record. To calculate this, take the   \n"
        "                    sum of the values of all bytes from the byte count up to the   \n"
        "                    last data byte, inclusive, modulo 256. Subtract this result   \n"
        "                    from 255.   \n"
        "   \n"
        "*****************************************************************************************   \n"
        "   \n"
        "Data Record (S3)   \n"
        "   \n"
        "   \n"
        "    A data record specifies data bytes that are to be loaded into memory. Figure 3   \n"
        "    shows the format for such a record. The columns shown in the figure represent half   \n"
        "    of a byte (4 bits).   \n"
        "   \n"
        "                ----------------------------------------------------------   \n"
        "                |  1 2   3 4   5 6 7 8 9 10 11 12   13 ... 44    45 46   |   \n"
        "                |                                                        |   \n"
        "                |  S ID  byte        load          data...data  checksum |   \n"
        "                |        count      address         1      n             |   \n"
        "                ----------------------------------------------------------   \n"
        "                Figure 3: Data Record Format for 32-Bit Load Address   \n"
        "   \n"
        "    Column          Description   \n"
        "   \n"
        "    1               Contains the ASCII character S, which indicates the start of   \n"
        "                    a record in Motorola S-record format.   \n"
        "    2               Contains the ASCII character identifying the record type.   \n"
        "                    For data records, this digit is 3 for 32-bit addresses.   \n"
        "    3 to 4          Contain the count of the number of bytes following this one   \n"
        "                    within the record. The count includes the checksum and the   \n"
        "                    load address bytes but not the byte count itself.   \n"
        "    5 to 12         Contain the load address. The first data byte is to be loaded   \n"
        "                    into this address and subsequent bytes into the next sequential    \n"
        "                    address. Columns 5 and 6 contain the high-order address   \n"
        "                    byte, and columns 11 and 12 contain the low-order address byte.   \n"
        "    13 to 44        Contain the specifications for up to 15 bytes of data.   \n"
        "    45 to 46        Contain a checksum for the record. To calculate this, take the   \n"
        "                    sum of the values of all bytes from the byte count up to the   \n"
        "                    last data byte, inclusive, modulo 256. Subtract this result   \n"
        "                    from 255.   \n"
        "   \n"
        "*****************************************************************************************   \n"
        "   \n"
        "Record Count Record (S5)   \n"
        "   \n"
        "    The record count record verifies the number of data records preceding it. Figure 4   \n"
        "    shows the format for such a record. The columns shown in the figure represent half   \n"
        "    of a byte (4 bits).   \n"
        "   \n"
        "                --------------------------------------   \n"
        "                |  1 2   3 4      5 6 7 8     9 10   |   \n"
        "                |                                    |   \n"
        "                |  S ID  byte    # of data  checksum |   \n"
        "                |        count    records            |   \n"
        "                --------------------------------------   \n"
        "                Figure 4: Record Count Record Format   \n"
        "   \n"
        "    Column          Description   \n"
        "   \n"
        "    1               Contains the ASCII character S, which indicates the start of   \n"
        "                    a record in Motorola S-record format.   \n"
        "    2               Contains the ASCII character 5, which indicates a record   \n"
        "                    count record.   \n"
        "    3 to 4          Contain the byte count, ASCII string 03.   \n"
        "    5 to 8          Contain the number of data records in this file. The high-   \n"
        "                    order byte is in columns 5 and 6.   \n"
        "    9 to 10         Contain the checksum for the record.   \n"
        "   \n"
        "    Example:   \n"
        "        S503010DEE   \n"
        "   \n"
        "    The example above shows a record count record indicating a total of 269 records   \n"
        "    (0x010D) and a checksum of 0xEE.   \n"
        "   \n"
        "*****************************************************************************************   \n"
        "   \n"
        "Terminator Record for 32-bit address (S7)   \n"
        "   \n"
        "    A terminator record specifies the end of the data records. Figure 5 shows the   \n"
        "    format for such a record. The columns shown in the figure represent half of a byte   \n"
        "    (4 bits).   \n"
        "   \n"
        "                -------------------------------------   \n"
        "                |  1 2   3 4      5...12    13 14   |   \n"
        "                |                                   |   \n"
        "                |  S ID  byte     load     checksum |   \n"
        "                |        count   address            |   \n"
        "                -------------------------------------   \n"
        "                Figure5: Terminator Record Format for 32-Bit Load Address   \n"
        "   \n"
        "    Column          Description   \n"
        "   \n"
        "    1               Contains the ASCII character S, which indicates the start of   \n"
        "                    a record in Motorola S-record format.   \n"
        "    2               Contains the ASCII character 7, which indicates a 32-bit   \n"
        "                    load address.   \n"
        "    3 to 4          Contain the byte count, ASCII string 04.   \n"
        "    5 to 12         Contain the load address that is either set to zero or to the   \n"
        "                    starting address specified in the END directive or START   \n"
        "                    command (there are no data bytes).   \n"
        "    13 to 14        Contain the checksum for the record.   \n"
        "   \n"
        "*****************************************************************************************   \n"
        "   \n"
        "Terminator Record for 24-bit address (S8)   \n"
        "   \n"
        "    A terminator record specifies the end of the data records. Figure 6 shows the   \n"
        "    format for such a record. The columns shown in the figure represent half of a byte   \n"
        "    (4 bits).   \n"
        "   \n"
        "                ----------------------------------------   \n"
        "                |  1 2   3 4    5 6 7 8 9 10   11 12   |   \n"
        "                |                                      |   \n"
        "                |  S ID  byte       load      checksum |   \n"
        "                |        count     address             |   \n"
        "                ----------------------------------------   \n"
        "                Figure 6: Terminator Record Format for 24-Bit Load Address   \n"
        "   \n"
        "    Column          Description   \n"
        "   \n"
        "    1               Contains the ASCII character S, which indicates the start of   \n"
        "                    a record in Motorola S-record format.   \n"
        "    2               Contains the ASCII character 8, which indicates a 24-bit   \n"
        "                    load address.   \n"
        "    3 to 4          Contain the byte count, ASCII string 04.   \n"
        "    5 to 10         Contain the load address, which is either set to zero or to the   \n"
        "                    starting address specified in the END directive or START   \n"
        "                    command. There are no data bytes.   \n"
        "    11 to 12        Contain the checksum for the record.   \n"
        "   \n"
        "    Example:   \n"
        "        S804000AF0001   \n"
        "   \n"
        "    The previous example shows a terminator record with a 24-bit load address of   \n"
        "    0x000AF0 and a checksum of 0x01.   \n"
        "   \n"
        "*****************************************************************************************   \n"
        "   \n"
        "Terminator Record for 16-bit address (S9)   \n"
        "   \n"
        "   \n"
        "    A terminator record specifies the end of the data records. Figure 7 shows the   \n"
        "    format for such a record. The columns shown in the figure represent half of a byte   \n"
        "    (4 bits).   \n"
        "   \n"
        "                -------------------------------------   \n"
        "                |  1 2   3 4    5 6 7 8      9 10   |   \n"
        "                |                                   |   \n"
        "                |  S ID  byte    load      checksum |   \n"
        "                |        count  address             |   \n"
        "                -------------------------------------   \n"
        "                Figure 7: Terminator Record Format for 16-Bit Load Address   \n"
        "   \n"
        "    Column         Description   \n"
        "   \n"
        "    1               Contains the ASCII character S, which indicates the start of   \n"
        "                    a record in Motorola S-record format.   \n"
        "    2               Contains the ASCII character 9, which indicates a 16-bit   \n"
        "                    load address.   \n"
        "    3 to 4          Contain the byte count, ASCII string 04.   \n"
        "    5 to 8          Contain the load address, which is either set to zero or to the   \n"
        "                    starting address specified in the END directive or START   \n"
        "                    command (there are no data bytes).   \n"
        "    9 to 10         Contain the checksum for the record.   \n"
        "   \n"
        "*****************************************************************************************    \n" );
	exit(0);
}










