
/************************************************************************************
 * @file    : hash.c 
 * @brief   : Message Digest, Hash from public domain (
 *     - 2008/12/05 : MD2/MD4/MD5/MD6, SHA1,SHA2 224/256/384/512,
 *     -              SHA3 KECCAK 224/256/384/512 
 *     - 2008/12/05 : Blake224/256/384/512, 
 *     - 2008/12/05 : RIPEMD128/RIPEMD160, etc...
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

#include "common.h"
#include "feature.h"
#include "hash.h"



int verbose = 0;
int g_iUpper = -1;




#if BLAKE_224_256_384_512_HASH

#define BLOCK224 			(32*1024) // 64
#define BLAKE224_LEN 		28

#define BLOCK256 			(32*1024) // 64
#define BLAKE256_LEN 		32

#define BLOCK384 			(32*1024) // 64
#define BLAKE384_LEN 		48


#define BLOCK512 			(32*1024) // 64
#define BLAKE512_LEN 		64


#define U8TO32_BIG(p)					      \
  (((unsigned int)((p)[0]) << 24) | ((unsigned int)((p)[1]) << 16) |  \
   ((unsigned int)((p)[2]) <<  8) | ((unsigned int)((p)[3])      )) 


#define U32TO8_BIG(p, v)				        \
  (p)[0] = (unsigned char)((v) >> 24); (p)[1] = (unsigned char)((v) >> 16); \
  (p)[2] = (unsigned char)((v) >>  8); (p)[3] = (unsigned char)((v)      ); 


#define U8TO64_BIG(p) \
  (((unsigned long long)U8TO32_BIG(p) << 32) | (unsigned long long)U8TO32_BIG((p) + 4)) 


#define U64TO8_BIG(p, v)		      \
  U32TO8_BIG((p),     (unsigned int)((v) >> 32)); \
  U32TO8_BIG((p) + 4, (unsigned int)((v)      ));


typedef struct 
{ 
  unsigned int h[8], s[4], t[2]; 
  unsigned int buflen, nullt; 
  unsigned char  buf[64]; 
} state256; 

typedef state256 state224; 


typedef struct 
{ 
  unsigned long long h[8], s[4], t[2]; 
  unsigned int buflen, nullt; 
  unsigned char buf[128]; 
} state512; 

typedef state512 state384; 


const unsigned char sigma[][16] = 
//static unsigned char sigma[][16] = 
{ 
  { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 }, 
  {14, 10, 4, 8, 9, 15, 13, 6, 1, 12, 0, 2, 11, 7, 5, 3 }, 
  {11, 8, 12, 0, 5, 2, 15, 13, 10, 14, 3, 6, 7, 1, 9, 4 }, 
  { 7, 9, 3, 1, 13, 12, 11, 14, 2, 6, 5, 10, 4, 0, 15, 8 }, 
  { 9, 0, 5, 7, 2, 4, 10, 15, 14, 1, 11, 12, 6, 8, 3, 13 }, 
  { 2, 12, 6, 10, 0, 11, 8, 3, 4, 13, 7, 5, 15, 14, 1, 9 }, 
  {12, 5, 1, 15, 14, 13, 4, 10, 0, 7, 6, 3, 9, 2, 8, 11 }, 
  {13, 11, 7, 14, 12, 1, 3, 9, 5, 0, 15, 4, 8, 6, 2, 10 }, 
  { 6, 15, 14, 9, 11, 3, 0, 8, 12, 2, 13, 7, 1, 4, 10, 5 }, 
  {10, 2, 8, 4, 7, 6, 1, 5, 15, 11, 9, 14, 3, 12, 13 , 0 }, 
  { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 }, 
  {14, 10, 4, 8, 9, 15, 13, 6, 1, 12, 0, 2, 11, 7, 5, 3 }, 
  {11, 8, 12, 0, 5, 2, 15, 13, 10, 14, 3, 6, 7, 1, 9, 4 }, 
  { 7, 9, 3, 1, 13, 12, 11, 14, 2, 6, 5, 10, 4, 0, 15, 8 }, 
  { 9, 0, 5, 7, 2, 4, 10, 15, 14, 1, 11, 12, 6, 8, 3, 13 }, 
  { 2, 12, 6, 10, 0, 11, 8, 3, 4, 13, 7, 5, 15, 14, 1, 9 } 
}; 


const unsigned int u256[16] = 
//static unsigned int u256[16] = 
{ 
  0x243f6a88, 0x85a308d3, 0x13198a2e, 0x03707344, 
  0xa4093822, 0x299f31d0, 0x082efa98, 0xec4e6c89, 
  0x452821e6, 0x38d01377, 0xbe5466cf, 0x34e90c6c, 
  0xc0ac29b7, 0xc97c50dd, 0x3f84d5b5, 0xb5470917 
}; 


const unsigned long long u512[16] = 
//static uint64_t u512[16] = 
{ 
  0x243f6a8885a308d3ULL, 0x13198a2e03707344ULL,  
  0xa4093822299f31d0ULL, 0x082efa98ec4e6c89ULL, 
  0x452821e638d01377ULL, 0xbe5466cf34e90c6cULL,  
  0xc0ac29b7c97c50ddULL, 0x3f84d5b5b5470917ULL, 
  0x9216d5d98979fb1bULL, 0xd1310ba698dfb5acULL,  
  0x2ffd72dbd01adfb7ULL, 0xb8e1afed6a267e96ULL, 
  0xba7c9045f12c7f99ULL, 0x24a19947b3916cf7ULL,  
  0x0801f2e2858efc16ULL, 0x636920d871574e69ULL 
}; 




static /*const*/ unsigned char padding[129] = 
{ 
  0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 
}; 



void blake224_compress( state224 *S, const unsigned char *block ) 
{ 
  unsigned int v[16], m[16], i; 
#define ROT224(x,n) (((x)<<(32-n))|( (x)>>(n))) 
#define G224(a,b,c,d,e)          \
  v[a] += (m[sigma[i][e]] ^ u256[sigma[i][e+1]]) + v[b]; \
  v[d] = ROT224( v[d] ^ v[a],16);        \
  v[c] += v[d];           \
  v[b] = ROT224( v[b] ^ v[c],12);        \
  v[a] += (m[sigma[i][e+1]] ^ u256[sigma[i][e]])+v[b]; \
  v[d] = ROT224( v[d] ^ v[a], 8);        \
  v[c] += v[d];           \
  v[b] = ROT224( v[b] ^ v[c], 7); 


  for( i = 0; i < 16; ++i )  m[i] = U8TO32_BIG( block + i * 4 ); 


  for( i = 0; i < 8; ++i )  v[i] = S->h[i]; 


  v[ 8] = S->s[0] ^ u256[0]; 
  v[ 9] = S->s[1] ^ u256[1]; 
  v[10] = S->s[2] ^ u256[2]; 
  v[11] = S->s[3] ^ u256[3]; 
  v[12] = u256[4]; 
  v[13] = u256[5]; 
  v[14] = u256[6]; 
  v[15] = u256[7]; 


  /* don't xor t when the block is only padding */ 
  if ( !S->nullt ) 
  { 
    v[12] ^= S->t[0]; 
    v[13] ^= S->t[0]; 
    v[14] ^= S->t[1]; 
    v[15] ^= S->t[1]; 
  } 


  for( i = 0; i < 14; ++i ) 
  { 
    /* column step */ 
    G224( 0,  4,  8, 12,  0 ); 
    G224( 1,  5,  9, 13,  2 ); 
    G224( 2,  6, 10, 14,  4 ); 
    G224( 3,  7, 11, 15,  6 ); 
    /* diagonal step */ 
    G224( 0,  5, 10, 15,  8 ); 
    G224( 1,  6, 11, 12, 10 ); 
    G224( 2,  7,  8, 13, 12 ); 
    G224( 3,  4,  9, 14, 14 ); 
  } 


  for( i = 0; i < 16; ++i )  S->h[i % 8] ^= v[i]; 


  for( i = 0; i < 8 ; ++i )  S->h[i] ^= S->s[i % 4]; 
} 




void blake224_init( state224 *S ) 
{ 
  S->h[0] = 0xc1059ed8; 
  S->h[1] = 0x367cd507; 
  S->h[2] = 0x3070dd17; 
  S->h[3] = 0xf70e5939; 
  S->h[4] = 0xffc00b31; 
  S->h[5] = 0x68581511; 
  S->h[6] = 0x64f98fa7; 
  S->h[7] = 0xbefa4fa4; 
  S->t[0] = S->t[1] = S->buflen = S->nullt = 0; 
  S->s[0] = S->s[1] = S->s[2] = S->s[3] = 0; 
} 




void blake224_update( state224 *S, const unsigned char *in, unsigned long long inlen ) 
{ 
 int left = S->buflen; 
 int fill = 64 - left; 


 /* data left and data received fill a block  */ 
 if( left && ( inlen >= fill ) ) 
 { 
   memcpy( ( void * ) ( S->buf + left ), ( void * ) in, fill ); 
   S->t[0] += 512; 


   if ( S->t[0] == 0 ) S->t[1]++; 


    blake224_compress( S, S->buf ); 
    in += fill; 
    inlen  -= fill; 
    left = 0; 
  } 


  /* compress blocks of data received */ 
  while( inlen >= 64 ) 
  { 
    S->t[0] += 512; 


    if ( S->t[0] == 0 ) S->t[1]++; 


    blake224_compress( S, in ); 
    in += 64; 
    inlen -= 64; 
  } 


  /* store any data left */ 
  if( inlen > 0 ) 
  { 
    memcpy( ( void * ) ( S->buf + left ), \
            ( void * ) in, ( size_t ) inlen ); 
    S->buflen = left + ( int )inlen; 
  } 
  else S->buflen = 0; 
} 




void blake224_final( state224 *S, unsigned char *out ) 
{ 
  unsigned char msglen[8], zz = 0x00, oz = 0x80; 
  unsigned int lo = S->t[0] + ( S->buflen << 3 ), hi = S->t[1]; 


  /* support for hashing more than 2^32 bits */ 
  if ( lo < ( S->buflen << 3 ) ) hi++; 


  U32TO8_BIG(  msglen + 0, hi ); 
  U32TO8_BIG(  msglen + 4, lo ); 


  if ( S->buflen == 55 )   /* one padding byte */ 
  { 
    S->t[0] -= 8; 
    blake224_update( S, &oz, 1 ); 
  } 
  else 
  { 
    if ( S->buflen < 55 )   /* enough space to fill the block  */ 
    { 
      if ( !S->buflen ) S->nullt = 1; 


      S->t[0] -= 440 - ( S->buflen << 3 ); 
      blake224_update( S, padding, 55 - S->buflen ); 
    } 
    else   /* need 2 compressions */ 
    { 
      S->t[0] -= 512 - ( S->buflen << 3 ); 
      blake224_update( S, padding, 64 - S->buflen ); 
      S->t[0] -= 440; 
      blake224_update( S, padding + 1, 55 ); 
      S->nullt = 1; 
    } 


    blake224_update( S, &zz, 1 ); 
    S->t[0] -= 8; 
  } 


  S->t[0] -= 64; 
  blake224_update( S, msglen, 8 ); 
  U32TO8_BIG( out + 0, S->h[0] ); 
  U32TO8_BIG( out + 4, S->h[1] ); 
  U32TO8_BIG( out + 8, S->h[2] ); 
  U32TO8_BIG( out + 12, S->h[3] ); 
  U32TO8_BIG( out + 16, S->h[4] ); 
  U32TO8_BIG( out + 20, S->h[5] ); 
  U32TO8_BIG( out + 24, S->h[6] ); 
} 




void blake224_hash( unsigned char *out, const unsigned char *in, unsigned long long inlen ) 
{ 
  state224 S; 
  blake224_init( &S ); 
  blake224_update( &S, in, inlen ); 
  blake224_final( &S, out ); 
} 



void blake256_compress( state256 *S, const unsigned char *block ) 
{ 
  unsigned int v[16], m[16], i; 
#define ROT256(x,n) (((x)<<(32-n))|( (x)>>(n))) 
#define G256(a,b,c,d,e)          \
  v[a] += (m[sigma[i][e]] ^ u256[sigma[i][e+1]]) + v[b]; \
  v[d] = ROT256( v[d] ^ v[a],16);        \
  v[c] += v[d];           \
  v[b] = ROT256( v[b] ^ v[c],12);        \
  v[a] += (m[sigma[i][e+1]] ^ u256[sigma[i][e]])+v[b]; \
  v[d] = ROT256( v[d] ^ v[a], 8);        \
  v[c] += v[d];           \
  v[b] = ROT256( v[b] ^ v[c], 7); 


  for( i = 0; i < 16; ++i )  m[i] = U8TO32_BIG( block + i * 4 ); 


  for( i = 0; i < 8; ++i )  v[i] = S->h[i]; 


  v[ 8] = S->s[0] ^ u256[0]; 
  v[ 9] = S->s[1] ^ u256[1]; 
  v[10] = S->s[2] ^ u256[2]; 
  v[11] = S->s[3] ^ u256[3]; 
  v[12] = u256[4]; 
  v[13] = u256[5]; 
  v[14] = u256[6]; 
  v[15] = u256[7]; 


  /* don't xor t when the block is only padding */ 
  if ( !S->nullt ) 
  { 
    v[12] ^= S->t[0]; 
    v[13] ^= S->t[0]; 
    v[14] ^= S->t[1]; 
    v[15] ^= S->t[1]; 
  } 


  for( i = 0; i < 14; ++i ) 
  { 
    /* column step */ 
    G256( 0,  4,  8, 12,  0 ); 
    G256( 1,  5,  9, 13,  2 ); 
    G256( 2,  6, 10, 14,  4 ); 
    G256( 3,  7, 11, 15,  6 ); 
    /* diagonal step */ 
    G256( 0,  5, 10, 15,  8 ); 
    G256( 1,  6, 11, 12, 10 ); 
    G256( 2,  7,  8, 13, 12 ); 
    G256( 3,  4,  9, 14, 14 ); 
  } 


  for( i = 0; i < 16; ++i )  S->h[i % 8] ^= v[i]; 


  for( i = 0; i < 8 ; ++i )  S->h[i] ^= S->s[i % 4]; 
} 




void blake256_init( state256 *S ) 
{ 
  S->h[0] = 0x6a09e667; 
  S->h[1] = 0xbb67ae85; 
  S->h[2] = 0x3c6ef372; 
  S->h[3] = 0xa54ff53a; 
  S->h[4] = 0x510e527f; 
  S->h[5] = 0x9b05688c; 
  S->h[6] = 0x1f83d9ab; 
  S->h[7] = 0x5be0cd19; 
  S->t[0] = S->t[1] = S->buflen = S->nullt = 0; 
  S->s[0] = S->s[1] = S->s[2] = S->s[3] = 0; 
} 




void blake256_update( state256 *S, const unsigned char *in, unsigned long long inlen ) 
{ 
 int left = S->buflen; 
 int fill = 64 - left; 


 /* data left and data received fill a block  */ 
 if( left && ( inlen >= fill ) ) 
 { 
   memcpy( ( void * ) ( S->buf + left ), ( void * ) in, fill ); 
   S->t[0] += 512; 


   if ( S->t[0] == 0 ) S->t[1]++; 


    blake256_compress( S, S->buf ); 
    in += fill; 
    inlen  -= fill; 
    left = 0; 
  } 


  /* compress blocks of data received */ 
  while( inlen >= 64 ) 
  { 
    S->t[0] += 512; 


    if ( S->t[0] == 0 ) S->t[1]++; 


    blake256_compress( S, in ); 
    in += 64; 
    inlen -= 64; 
  } 


  /* store any data left */ 
  if( inlen > 0 ) 
  { 
    memcpy( ( void * ) ( S->buf + left ),   \
            ( void * ) in, ( size_t ) inlen ); 
    S->buflen = left + ( int )inlen; 
  } 
  else S->buflen = 0; 
} 




void blake256_final( state256 *S, unsigned char *out ) 
{ 
  unsigned char msglen[8], zo = 0x01, oo = 0x81; 
  unsigned int lo = S->t[0] + ( S->buflen << 3 ), hi = S->t[1]; 


  /* support for hashing more than 2^32 bits */ 
  if ( lo < ( S->buflen << 3 ) ) hi++; 


  U32TO8_BIG(  msglen + 0, hi ); 
  U32TO8_BIG(  msglen + 4, lo ); 


  if ( S->buflen == 55 )   /* one padding byte */ 
  { 
    S->t[0] -= 8; 
    blake256_update( S, &oo, 1 ); 
  } 
  else 
  { 
    if ( S->buflen < 55 )   /* enough space to fill the block  */ 
    { 
      if ( !S->buflen ) S->nullt = 1; 


      S->t[0] -= 440 - ( S->buflen << 3 ); 
      blake256_update( S, padding, 55 - S->buflen ); 
    } 
    else   /* need 2 compressions */ 
    { 
      S->t[0] -= 512 - ( S->buflen << 3 ); 
      blake256_update( S, padding, 64 - S->buflen ); 
      S->t[0] -= 440; 
      blake256_update( S, padding + 1, 55 ); 
      S->nullt = 1; 
    } 


    blake256_update( S, &zo, 1 ); 
    S->t[0] -= 8; 
  } 


  S->t[0] -= 64; 
  blake256_update( S, msglen, 8 ); 
  U32TO8_BIG( out + 0, S->h[0] ); 
  U32TO8_BIG( out + 4, S->h[1] ); 
  U32TO8_BIG( out + 8, S->h[2] ); 
  U32TO8_BIG( out + 12, S->h[3] ); 
  U32TO8_BIG( out + 16, S->h[4] ); 
  U32TO8_BIG( out + 20, S->h[5] ); 
  U32TO8_BIG( out + 24, S->h[6] ); 
  U32TO8_BIG( out + 28, S->h[7] ); 
} 



void blake384_compress( state384 *S, const unsigned char *block ) 
{ 
  unsigned long long v[16], m[16], i; 
#define ROT384(x,n) (((x)<<(64-n))|( (x)>>(n))) 
#define G384(a,b,c,d,e)          \
  v[a] += (m[sigma[i][e]] ^ u512[sigma[i][e+1]]) + v[b];\
  v[d] = ROT384( v[d] ^ v[a],32);        \
  v[c] += v[d];           \
  v[b] = ROT384( v[b] ^ v[c],25);        \
  v[a] += (m[sigma[i][e+1]] ^ u512[sigma[i][e]])+v[b];  \
  v[d] = ROT384( v[d] ^ v[a],16);        \
  v[c] += v[d];           \
  v[b] = ROT384( v[b] ^ v[c],11); 


  for( i = 0; i < 16; ++i )  m[i] = U8TO64_BIG( block + i * 8 ); 


  for( i = 0; i < 8; ++i )  v[i] = S->h[i]; 


  v[ 8] = S->s[0] ^ u512[0]; 
  v[ 9] = S->s[1] ^ u512[1]; 
  v[10] = S->s[2] ^ u512[2]; 
  v[11] = S->s[3] ^ u512[3]; 
  v[12] =  u512[4]; 
  v[13] =  u512[5]; 
  v[14] =  u512[6]; 
  v[15] =  u512[7]; 


  /* don't xor t when the block is only padding */ 
  if ( !S->nullt ) 
  { 
    v[12] ^= S->t[0]; 
    v[13] ^= S->t[0]; 
    v[14] ^= S->t[1]; 
    v[15] ^= S->t[1]; 
  } 


  for( i = 0; i < 16; ++i ) 
  { 
    /* column step */ 
    G384( 0, 4, 8, 12, 0 ); 
    G384( 1, 5, 9, 13, 2 ); 
    G384( 2, 6, 10, 14, 4 ); 
    G384( 3, 7, 11, 15, 6 ); 
    /* diagonal step */ 
    G384( 0, 5, 10, 15, 8 ); 
    G384( 1, 6, 11, 12, 10 ); 
    G384( 2, 7, 8, 13, 12 ); 
    G384( 3, 4, 9, 14, 14 ); 
  } 


  for( i = 0; i < 16; ++i )  S->h[i % 8] ^= v[i]; 


  for( i = 0; i < 8 ; ++i )  S->h[i] ^= S->s[i % 4]; 
} 




void blake384_init( state384 *S ) 
{ 
  S->h[0] = 0xcbbb9d5dc1059ed8ULL; 
  S->h[1] = 0x629a292a367cd507ULL; 
  S->h[2] = 0x9159015a3070dd17ULL; 
  S->h[3] = 0x152fecd8f70e5939ULL; 
  S->h[4] = 0x67332667ffc00b31ULL; 
  S->h[5] = 0x8eb44a8768581511ULL; 
  S->h[6] = 0xdb0c2e0d64f98fa7ULL; 
  S->h[7] = 0x47b5481dbefa4fa4ULL; 
  S->t[0] = S->t[1] = S->buflen = S->nullt = 0; 
  S->s[0] = S->s[1] = S->s[2] = S->s[3] = 0; 
} 




void blake384_update( state384 *S, const unsigned char *in, unsigned long long inlen ) 
{ 
 int left = S->buflen; 
 int fill = 128 - left; 


 /* data left and data received fill a block  */ 
 if( left && ( inlen >= fill ) ) 
 { 
   memcpy( ( void * ) ( S->buf + left ), ( void * ) in, fill ); 
   S->t[0] += 1024; 


   if ( S->t[0] == 0 ) S->t[1]++; 


   blake384_compress( S, S->buf ); 
    in += fill; 
    inlen  -= fill; 
    left = 0; 
  } 


  /* compress blocks of data received */ 
  while( inlen >= 128 ) 
  { 
    S->t[0] += 1024; 


    if ( S->t[0] == 0 ) S->t[1]++; 


    blake384_compress( S, in ); 
    in += 128; 
    inlen -= 128; 
  } 


  /* store any data left */ 
  if( inlen > 0 ) 
  { 
    memcpy( ( void * ) ( S->buf + left ), \
            ( void * ) in, ( size_t ) inlen ); 
    S->buflen = left + ( int )inlen; 
  } 
  else S->buflen = 0; 
} 




void blake384_final( state384 *S, unsigned char *out ) 
{ 
  unsigned char msglen[16], zz = 0x00, oz = 0x80; 
  unsigned long long lo = S->t[0] + ( S->buflen << 3 ), hi = S->t[1]; 


  /* support for hashing more than 2^32 bits */ 
  if ( lo < ( S->buflen << 3 ) ) hi++; 


  U64TO8_BIG(  msglen + 0, hi ); 
  U64TO8_BIG(  msglen + 8, lo ); 


  if ( S->buflen == 111 )   /* one padding byte */ 
  { 
    S->t[0] -= 8; 
    blake384_update( S, &oz, 1 ); 
  } 
  else 
  { 
    if ( S->buflen < 111 )  /* enough space to fill the block */ 
    { 
      if ( !S->buflen ) S->nullt = 1; 


      S->t[0] -= 888 - ( S->buflen << 3 ); 
      blake384_update( S, padding, 111 - S->buflen ); 
    } 
    else   /* need 2 compressions */ 
    { 
      S->t[0] -= 1024 - ( S->buflen << 3 ); 
      blake384_update( S, padding, 128 - S->buflen ); 
      S->t[0] -= 888; 
      blake384_update( S, padding + 1, 111 ); 
      S->nullt = 1; 
    } 


    blake384_update( S, &zz, 1 ); 
    S->t[0] -= 8; 
  } 


  S->t[0] -= 128; 
  blake384_update( S, msglen, 16 ); 
  U64TO8_BIG( out + 0, S->h[0] ); 
  U64TO8_BIG( out + 8, S->h[1] ); 
  U64TO8_BIG( out + 16, S->h[2] ); 
  U64TO8_BIG( out + 24, S->h[3] ); 
  U64TO8_BIG( out + 32, S->h[4] ); 
  U64TO8_BIG( out + 40, S->h[5] ); 
} 




void blake384_hash( unsigned char *out, const unsigned char *in, unsigned long long inlen ) 
{ 
  state384 S; 
  blake384_init( &S ); 
  blake384_update( &S, in, inlen ); 
  blake384_final( &S, out ); 
} 



void blake512_compress( state512 *S, const unsigned char *block ) 
{ 
  unsigned long long v[16], m[16], i; 
#define ROT512(x,n) (((x)<<(64-n))|( (x)>>(n))) 
#define G512(a,b,c,d,e)          \
  v[a] += (m[sigma[i][e]] ^ u512[sigma[i][e+1]]) + v[b];\
  v[d] = ROT512( v[d] ^ v[a],32);        \
  v[c] += v[d];           \
  v[b] = ROT512( v[b] ^ v[c],25);        \
  v[a] += (m[sigma[i][e+1]] ^ u512[sigma[i][e]])+v[b];  \
  v[d] = ROT512( v[d] ^ v[a],16);        \
  v[c] += v[d];           \
  v[b] = ROT512( v[b] ^ v[c],11); 


  for( i = 0; i < 16; ++i )  m[i] = U8TO64_BIG( block + i * 8 ); 


  for( i = 0; i < 8; ++i )  v[i] = S->h[i]; 


  v[ 8] = S->s[0] ^ u512[0]; 
  v[ 9] = S->s[1] ^ u512[1]; 
  v[10] = S->s[2] ^ u512[2]; 
  v[11] = S->s[3] ^ u512[3]; 
  v[12] =  u512[4]; 
  v[13] =  u512[5]; 
  v[14] =  u512[6]; 
  v[15] =  u512[7]; 


  /* don't xor t when the block is only padding */ 
  if ( !S->nullt ) 
  { 
    v[12] ^= S->t[0]; 
    v[13] ^= S->t[0]; 
    v[14] ^= S->t[1]; 
    v[15] ^= S->t[1]; 
  } 


  for( i = 0; i < 16; ++i ) 
  { 
    /* column step */ 
    G512( 0, 4, 8, 12, 0 ); 
    G512( 1, 5, 9, 13, 2 ); 
    G512( 2, 6, 10, 14, 4 ); 
    G512( 3, 7, 11, 15, 6 ); 
    /* diagonal step */ 
    G512( 0, 5, 10, 15, 8 ); 
    G512( 1, 6, 11, 12, 10 ); 
    G512( 2, 7, 8, 13, 12 ); 
    G512( 3, 4, 9, 14, 14 ); 
  } 


  for( i = 0; i < 16; ++i )  S->h[i % 8] ^= v[i]; 


  for( i = 0; i < 8 ; ++i )  S->h[i] ^= S->s[i % 4]; 
} 




void blake512_init( state512 *S ) 
{ 
  S->h[0] = 0x6a09e667f3bcc908ULL; 
  S->h[1] = 0xbb67ae8584caa73bULL; 
  S->h[2] = 0x3c6ef372fe94f82bULL; 
  S->h[3] = 0xa54ff53a5f1d36f1ULL; 
  S->h[4] = 0x510e527fade682d1ULL; 
  S->h[5] = 0x9b05688c2b3e6c1fULL; 
  S->h[6] = 0x1f83d9abfb41bd6bULL; 
  S->h[7] = 0x5be0cd19137e2179ULL; 
  S->t[0] = S->t[1] = S->buflen = S->nullt = 0; 
  S->s[0] = S->s[1] = S->s[2] = S->s[3] = 0; 
} 




void blake512_update( state512 *S, const unsigned char *in, unsigned long long inlen ) 
{ 
 int left = S->buflen; 
 int fill = 128 - left; 


 /* data left and data received fill a block  */ 
 if( left && ( inlen >= fill ) ) 
 { 
   memcpy( ( void * ) ( S->buf + left ), ( void * ) in, fill ); 
   S->t[0] += 1024; 


   if ( S->t[0] == 0 ) S->t[1]++; 


   blake512_compress( S, S->buf ); 
    in += fill; 
    inlen  -= fill; 
    left = 0; 
  } 


  /* compress blocks of data received */ 
  while( inlen >= 128 ) 
  { 
    S->t[0] += 1024; 


    if ( S->t[0] == 0 ) S->t[1]++; 


    blake512_compress( S, in ); 
    in += 128; 
    inlen -= 128; 
  } 


  /* store any data left */ 
  if( inlen > 0 ) 
  { 
    memcpy( ( void * ) ( S->buf + left ),   \
            ( void * ) in, ( size_t ) inlen ); 
    S->buflen = left + ( int )inlen; 
  } 
  else S->buflen = 0; 
} 




void blake512_final( state512 *S, unsigned char *out ) 
{ 
  unsigned char msglen[16], zo = 0x01, oo = 0x81; 
  unsigned long long lo = S->t[0] + ( S->buflen << 3 ), hi = S->t[1]; 


  /* support for hashing more than 2^32 bits */ 
  if ( lo < ( S->buflen << 3 ) ) hi++; 


  U64TO8_BIG(  msglen + 0, hi ); 
  U64TO8_BIG(  msglen + 8, lo ); 


  if ( S->buflen == 111 )   /* one padding byte */ 
  { 
    S->t[0] -= 8; 
    blake512_update( S, &oo, 1 ); 
  } 
  else 
  { 
    if ( S->buflen < 111 )  /* enough space to fill the block */ 
    { 
      if ( !S->buflen ) S->nullt = 1; 


      S->t[0] -= 888 - ( S->buflen << 3 ); 
      blake512_update( S, padding, 111 - S->buflen ); 
    } 
    else   /* need 2 compressions */ 
    { 
      S->t[0] -= 1024 - ( S->buflen << 3 ); 
      blake512_update( S, padding, 128 - S->buflen ); 
      S->t[0] -= 888; 
      blake512_update( S, padding + 1, 111 ); 
      S->nullt = 1; 
    } 


    blake512_update( S, &zo, 1 ); 
    S->t[0] -= 8; 
  } 


  S->t[0] -= 128; 
  blake512_update( S, msglen, 16 ); 
  U64TO8_BIG( out + 0, S->h[0] ); 
  U64TO8_BIG( out + 8, S->h[1] ); 
  U64TO8_BIG( out + 16, S->h[2] ); 
  U64TO8_BIG( out + 24, S->h[3] ); 
  U64TO8_BIG( out + 32, S->h[4] ); 
  U64TO8_BIG( out + 40, S->h[5] ); 
  U64TO8_BIG( out + 48, S->h[6] ); 
  U64TO8_BIG( out + 56, S->h[7] ); 
} 




void blake512_hash( unsigned char *out, const unsigned char *in, unsigned long long inlen ) 
{ 
  state512 S; 
  blake512_init( &S ); 
  blake512_update( &S, in, inlen ); 
  blake512_final( &S, out ); 
} 

#endif // BLAKE_224_256_384_512_HASH



#if defined(RIPEMD128) || defined(RIPEMD160) || defined(RIPEMD320)
#define RMD128_DIGEST_SIZE      16 // 128/8
#define RMD128_BLOCK_SIZE       64

#define RMD160_DIGEST_SIZE      20 // 160/8
#define RMD160_BLOCK_SIZE       64

#define RMD256_DIGEST_SIZE      32 // 256/8
#define RMD256_BLOCK_SIZE       64

#define RMD320_DIGEST_SIZE      40 //320/8
#define RMD320_BLOCK_SIZE       64


/* initial values  */
#define RMD_H0  0x67452301UL
#define RMD_H1  0xefcdab89UL
#define RMD_H2  0x98badcfeUL
#define RMD_H3  0x10325476UL
#define RMD_H4  0xc3d2e1f0UL
#define RMD_H5  0x76543210UL
#define RMD_H6  0xfedcba98UL
#define RMD_H7  0x89abcdefUL
#define RMD_H8  0x01234567UL
#define RMD_H9  0x3c2d1e0fUL
/* constants */
#define RMD_K1  0x00000000UL
#define RMD_K2  0x5a827999UL
#define RMD_K3  0x6ed9eba1UL
#define RMD_K4  0x8f1bbcdcUL
#define RMD_K5  0xa953fd4eUL
#define RMD_K6  0x50a28be6UL
#define RMD_K7  0x5c4dd124UL
#define RMD_K8  0x6d703ef3UL
#define RMD_K9  0x7a6d76e9UL
#endif




#if defined(RIPEMD128) || defined(RIPEMD160)

/* if this line causes a compiler error, 
   adapt the defintion of dword above */
typedef int the_correct_size_was_chosen [sizeof (unsigned long) == 4? 1: -1];

/********************************************************************/

/* macro definitions */

/* collect four bytes into one word: */
#define BYTES_TO_DWORD(strptr)                    \
            (((unsigned long) *((strptr)+3) << 24) | \
             ((unsigned long) *((strptr)+2) << 16) | \
             ((unsigned long) *((strptr)+1) <<  8) | \
             ((unsigned long) *(strptr)))

/* ROL(x, n) cyclically rotates x over n bits to the left */
/* x must be of an unsigned 32 bits type and 0 <= n < 32. */
#define RMD_ROL(x, n)        (((x) << (n)) | ((x) >> (32-(n))))

/* the five basic functions F(), G() and H() */
#define RMD_F(x, y, z)        ((x) ^ (y) ^ (z)) 
#define RMD_G(x, y, z)        (((x) & (y)) | (~(x) & (z))) 
#define RMD_H(x, y, z)        (((x) | ~(y)) ^ (z))
#define RMD_I(x, y, z)        (((x) & (z)) | ((y) & ~(z))) 
#define RMD_J(x, y, z)        ((x) ^ ((y) | ~(z)))

#endif



#if RIPEMD128


/* the eight basic operations FF() through III() */
#define RMD128_FF(a, b, c, d, x, s)        {\
      (a) += RMD_F((b), (c), (d)) + (x);\
      (a) = RMD_ROL((a), (s));\
   }
#define RMD128_GG(a, b, c, d, x, s)        {\
      (a) += RMD_G((b), (c), (d)) + (x) + 0x5a827999UL;\
      (a) = RMD_ROL((a), (s));\
   }
#define RMD128_HH(a, b, c, d, x, s)        {\
      (a) += RMD_H((b), (c), (d)) + (x) + 0x6ed9eba1UL;\
      (a) = RMD_ROL((a), (s));\
   }
#define RMD128_II(a, b, c, d, x, s)        {\
      (a) += RMD_I((b), (c), (d)) + (x) + 0x8f1bbcdcUL;\
      (a) = RMD_ROL((a), (s));\
   }
#define RMD128_FFF(a, b, c, d, x, s)        {\
      (a) += RMD_F((b), (c), (d)) + (x);\
      (a) = RMD_ROL((a), (s));\
   }
#define RMD128_GGG(a, b, c, d, x, s)        {\
      (a) += RMD_G((b), (c), (d)) + (x) + 0x6d703ef3UL;\
      (a) = RMD_ROL((a), (s));\
   }
#define RMD128_HHH(a, b, c, d, x, s)        {\
      (a) += RMD_H((b), (c), (d)) + (x) + 0x5c4dd124UL;\
      (a) = RMD_ROL((a), (s));\
   }
#define RMD128_III(a, b, c, d, x, s)        {\
      (a) += RMD_I((b), (c), (d)) + (x) + 0x50a28be6UL;\
      (a) = RMD_ROL((a), (s));\
   }



/********************************************************************/

void MD128init(unsigned long *MDbuf)
{
   MDbuf[0] = RMD_H0; // 0x67452301UL;
   MDbuf[1] = RMD_H1; // 0xefcdab89UL;
   MDbuf[2] = RMD_H2; // 0x98badcfeUL;
   MDbuf[3] = RMD_H3; // 0x10325476UL;

   return;
}


/********************************************************************/

void MD128compress(unsigned long *MDbuf, unsigned long *X)
{
   unsigned long aa = MDbuf[0],  bb = MDbuf[1],  cc = MDbuf[2],  dd = MDbuf[3];
   unsigned long aaa = MDbuf[0], bbb = MDbuf[1], ccc = MDbuf[2], ddd = MDbuf[3];

   /* round 1 */
   RMD128_FF(aa, bb, cc, dd, X[ 0], 11);
   RMD128_FF(dd, aa, bb, cc, X[ 1], 14);
   RMD128_FF(cc, dd, aa, bb, X[ 2], 15);
   RMD128_FF(bb, cc, dd, aa, X[ 3], 12);
   RMD128_FF(aa, bb, cc, dd, X[ 4],  5);
   RMD128_FF(dd, aa, bb, cc, X[ 5],  8);
   RMD128_FF(cc, dd, aa, bb, X[ 6],  7);
   RMD128_FF(bb, cc, dd, aa, X[ 7],  9);
   RMD128_FF(aa, bb, cc, dd, X[ 8], 11);
   RMD128_FF(dd, aa, bb, cc, X[ 9], 13);
   RMD128_FF(cc, dd, aa, bb, X[10], 14);
   RMD128_FF(bb, cc, dd, aa, X[11], 15);
   RMD128_FF(aa, bb, cc, dd, X[12],  6);
   RMD128_FF(dd, aa, bb, cc, X[13],  7);
   RMD128_FF(cc, dd, aa, bb, X[14],  9);
   RMD128_FF(bb, cc, dd, aa, X[15],  8);
                             
   /* round 2 */
   RMD128_GG(aa, bb, cc, dd, X[ 7],  7);
   RMD128_GG(dd, aa, bb, cc, X[ 4],  6);
   RMD128_GG(cc, dd, aa, bb, X[13],  8);
   RMD128_GG(bb, cc, dd, aa, X[ 1], 13);
   RMD128_GG(aa, bb, cc, dd, X[10], 11);
   RMD128_GG(dd, aa, bb, cc, X[ 6],  9);
   RMD128_GG(cc, dd, aa, bb, X[15],  7);
   RMD128_GG(bb, cc, dd, aa, X[ 3], 15);
   RMD128_GG(aa, bb, cc, dd, X[12],  7);
   RMD128_GG(dd, aa, bb, cc, X[ 0], 12);
   RMD128_GG(cc, dd, aa, bb, X[ 9], 15);
   RMD128_GG(bb, cc, dd, aa, X[ 5],  9);
   RMD128_GG(aa, bb, cc, dd, X[ 2], 11);
   RMD128_GG(dd, aa, bb, cc, X[14],  7);
   RMD128_GG(cc, dd, aa, bb, X[11], 13);
   RMD128_GG(bb, cc, dd, aa, X[ 8], 12);

   /* round 3 */
   RMD128_HH(aa, bb, cc, dd, X[ 3], 11);
   RMD128_HH(dd, aa, bb, cc, X[10], 13);
   RMD128_HH(cc, dd, aa, bb, X[14],  6);
   RMD128_HH(bb, cc, dd, aa, X[ 4],  7);
   RMD128_HH(aa, bb, cc, dd, X[ 9], 14);
   RMD128_HH(dd, aa, bb, cc, X[15],  9);
   RMD128_HH(cc, dd, aa, bb, X[ 8], 13);
   RMD128_HH(bb, cc, dd, aa, X[ 1], 15);
   RMD128_HH(aa, bb, cc, dd, X[ 2], 14);
   RMD128_HH(dd, aa, bb, cc, X[ 7],  8);
   RMD128_HH(cc, dd, aa, bb, X[ 0], 13);
   RMD128_HH(bb, cc, dd, aa, X[ 6],  6);
   RMD128_HH(aa, bb, cc, dd, X[13],  5);
   RMD128_HH(dd, aa, bb, cc, X[11], 12);
   RMD128_HH(cc, dd, aa, bb, X[ 5],  7);
   RMD128_HH(bb, cc, dd, aa, X[12],  5);

   /* round 4 */
   RMD128_II(aa, bb, cc, dd, X[ 1], 11);
   RMD128_II(dd, aa, bb, cc, X[ 9], 12);
   RMD128_II(cc, dd, aa, bb, X[11], 14);
   RMD128_II(bb, cc, dd, aa, X[10], 15);
   RMD128_II(aa, bb, cc, dd, X[ 0], 14);
   RMD128_II(dd, aa, bb, cc, X[ 8], 15);
   RMD128_II(cc, dd, aa, bb, X[12],  9);
   RMD128_II(bb, cc, dd, aa, X[ 4],  8);
   RMD128_II(aa, bb, cc, dd, X[13],  9);
   RMD128_II(dd, aa, bb, cc, X[ 3], 14);
   RMD128_II(cc, dd, aa, bb, X[ 7],  5);
   RMD128_II(bb, cc, dd, aa, X[15],  6);
   RMD128_II(aa, bb, cc, dd, X[14],  8);
   RMD128_II(dd, aa, bb, cc, X[ 5],  6);
   RMD128_II(cc, dd, aa, bb, X[ 6],  5);
   RMD128_II(bb, cc, dd, aa, X[ 2], 12);

   /* parallel round 1 */
   RMD128_III(aaa, bbb, ccc, ddd, X[ 5],  8); 
   RMD128_III(ddd, aaa, bbb, ccc, X[14],  9);
   RMD128_III(ccc, ddd, aaa, bbb, X[ 7],  9);
   RMD128_III(bbb, ccc, ddd, aaa, X[ 0], 11);
   RMD128_III(aaa, bbb, ccc, ddd, X[ 9], 13);
   RMD128_III(ddd, aaa, bbb, ccc, X[ 2], 15);
   RMD128_III(ccc, ddd, aaa, bbb, X[11], 15);
   RMD128_III(bbb, ccc, ddd, aaa, X[ 4],  5);
   RMD128_III(aaa, bbb, ccc, ddd, X[13],  7);
   RMD128_III(ddd, aaa, bbb, ccc, X[ 6],  7);
   RMD128_III(ccc, ddd, aaa, bbb, X[15],  8);
   RMD128_III(bbb, ccc, ddd, aaa, X[ 8], 11);
   RMD128_III(aaa, bbb, ccc, ddd, X[ 1], 14);
   RMD128_III(ddd, aaa, bbb, ccc, X[10], 14);
   RMD128_III(ccc, ddd, aaa, bbb, X[ 3], 12);
   RMD128_III(bbb, ccc, ddd, aaa, X[12],  6);

   /* parallel round 2 */
   RMD128_HHH(aaa, bbb, ccc, ddd, X[ 6],  9);
   RMD128_HHH(ddd, aaa, bbb, ccc, X[11], 13);
   RMD128_HHH(ccc, ddd, aaa, bbb, X[ 3], 15);
   RMD128_HHH(bbb, ccc, ddd, aaa, X[ 7],  7);
   RMD128_HHH(aaa, bbb, ccc, ddd, X[ 0], 12);
   RMD128_HHH(ddd, aaa, bbb, ccc, X[13],  8);
   RMD128_HHH(ccc, ddd, aaa, bbb, X[ 5],  9);
   RMD128_HHH(bbb, ccc, ddd, aaa, X[10], 11);
   RMD128_HHH(aaa, bbb, ccc, ddd, X[14],  7);
   RMD128_HHH(ddd, aaa, bbb, ccc, X[15],  7);
   RMD128_HHH(ccc, ddd, aaa, bbb, X[ 8], 12);
   RMD128_HHH(bbb, ccc, ddd, aaa, X[12],  7);
   RMD128_HHH(aaa, bbb, ccc, ddd, X[ 4],  6);
   RMD128_HHH(ddd, aaa, bbb, ccc, X[ 9], 15);
   RMD128_HHH(ccc, ddd, aaa, bbb, X[ 1], 13);
   RMD128_HHH(bbb, ccc, ddd, aaa, X[ 2], 11);

   /* parallel round 3 */   
   RMD128_GGG(aaa, bbb, ccc, ddd, X[15],  9);
   RMD128_GGG(ddd, aaa, bbb, ccc, X[ 5],  7);
   RMD128_GGG(ccc, ddd, aaa, bbb, X[ 1], 15);
   RMD128_GGG(bbb, ccc, ddd, aaa, X[ 3], 11);
   RMD128_GGG(aaa, bbb, ccc, ddd, X[ 7],  8);
   RMD128_GGG(ddd, aaa, bbb, ccc, X[14],  6);
   RMD128_GGG(ccc, ddd, aaa, bbb, X[ 6],  6);
   RMD128_GGG(bbb, ccc, ddd, aaa, X[ 9], 14);
   RMD128_GGG(aaa, bbb, ccc, ddd, X[11], 12);
   RMD128_GGG(ddd, aaa, bbb, ccc, X[ 8], 13);
   RMD128_GGG(ccc, ddd, aaa, bbb, X[12],  5);
   RMD128_GGG(bbb, ccc, ddd, aaa, X[ 2], 14);
   RMD128_GGG(aaa, bbb, ccc, ddd, X[10], 13);
   RMD128_GGG(ddd, aaa, bbb, ccc, X[ 0], 13);
   RMD128_GGG(ccc, ddd, aaa, bbb, X[ 4],  7);
   RMD128_GGG(bbb, ccc, ddd, aaa, X[13],  5);

   /* parallel round 4 */
   RMD128_FFF(aaa, bbb, ccc, ddd, X[ 8], 15);
   RMD128_FFF(ddd, aaa, bbb, ccc, X[ 6],  5);
   RMD128_FFF(ccc, ddd, aaa, bbb, X[ 4],  8);
   RMD128_FFF(bbb, ccc, ddd, aaa, X[ 1], 11);
   RMD128_FFF(aaa, bbb, ccc, ddd, X[ 3], 14);
   RMD128_FFF(ddd, aaa, bbb, ccc, X[11], 14);
   RMD128_FFF(ccc, ddd, aaa, bbb, X[15],  6);
   RMD128_FFF(bbb, ccc, ddd, aaa, X[ 0], 14);
   RMD128_FFF(aaa, bbb, ccc, ddd, X[ 5],  6);
   RMD128_FFF(ddd, aaa, bbb, ccc, X[12],  9);
   RMD128_FFF(ccc, ddd, aaa, bbb, X[ 2], 12);
   RMD128_FFF(bbb, ccc, ddd, aaa, X[13],  9);
   RMD128_FFF(aaa, bbb, ccc, ddd, X[ 9], 12);
   RMD128_FFF(ddd, aaa, bbb, ccc, X[ 7],  5);
   RMD128_FFF(ccc, ddd, aaa, bbb, X[10], 15);
   RMD128_FFF(bbb, ccc, ddd, aaa, X[14],  8);

   /* combine results */
   ddd += cc + MDbuf[1];               /* final result for MDbuf[0] */
   MDbuf[1] = MDbuf[2] + dd + aaa;
   MDbuf[2] = MDbuf[3] + aa + bbb;
   MDbuf[3] = MDbuf[0] + bb + ccc;
   MDbuf[0] = ddd;

   return;
}

/********************************************************************/

void MD128finish(unsigned long *MDbuf, unsigned char *strptr, unsigned long lswlen, unsigned long mswlen)
{
   unsigned int i;                                 /* counter       */
   unsigned long        X[16];                             /* message words */

   memset(X, 0, 16*sizeof(unsigned long));

   /* put bytes from strptr into X */
   for (i=0; i<(lswlen&63); i++) {
      /* byte i goes into word X[i div 4] at pos.  8*(i mod 4)  */
      X[i>>2] ^= (unsigned long) *strptr++ << (8 * (i&3));
   }

   /* append the bit m_n == 1 */
   X[(lswlen>>2)&15] ^= (unsigned long)1 << (8*(lswlen&3) + 7);

   if ((lswlen & 63) > 55) {
      /* length goes to next block */
      MD128compress(MDbuf, X);
      memset(X, 0, 16*sizeof(unsigned long));
   }

   /* append length in bits*/
   X[14] = lswlen << 3;
   X[15] = (lswlen >> 29) | (mswlen << 3);
   MD128compress(MDbuf, X);

   return;
}

#endif // RIPEMD128


#if RIPEMD160

/* the ten basic operations FF() through III() */
#define RMD160_FF(a, b, c, d, e, x, s)        {\
      (a) += RMD_F((b), (c), (d)) + (x);\
      (a) = RMD_ROL((a), (s)) + (e);\
      (c) = RMD_ROL((c), 10);\
   }
#define RMD160_GG(a, b, c, d, e, x, s)        {\
      (a) += RMD_G((b), (c), (d)) + (x) + 0x5a827999UL;\
      (a) = RMD_ROL((a), (s)) + (e);\
      (c) = RMD_ROL((c), 10);\
   }
#define RMD160_HH(a, b, c, d, e, x, s)        {\
      (a) += RMD_H((b), (c), (d)) + (x) + 0x6ed9eba1UL;\
      (a) = RMD_ROL((a), (s)) + (e);\
      (c) = RMD_ROL((c), 10);\
   }
#define RMD160_II(a, b, c, d, e, x, s)        {\
      (a) += RMD_I((b), (c), (d)) + (x) + 0x8f1bbcdcUL;\
      (a) = RMD_ROL((a), (s)) + (e);\
      (c) = RMD_ROL((c), 10);\
   }
#define RMD160_JJ(a, b, c, d, e, x, s)        {\
      (a) += RMD_J((b), (c), (d)) + (x) + 0xa953fd4eUL;\
      (a) = RMD_ROL((a), (s)) + (e);\
      (c) = RMD_ROL((c), 10);\
   }
#define RMD160_FFF(a, b, c, d, e, x, s)        {\
      (a) += RMD_F((b), (c), (d)) + (x);\
      (a) = RMD_ROL((a), (s)) + (e);\
      (c) = RMD_ROL((c), 10);\
   }
#define RMD160_GGG(a, b, c, d, e, x, s)        {\
      (a) += RMD_G((b), (c), (d)) + (x) + 0x7a6d76e9UL;\
      (a) = RMD_ROL((a), (s)) + (e);\
      (c) = RMD_ROL((c), 10);\
   }
#define RMD160_HHH(a, b, c, d, e, x, s)        {\
      (a) += RMD_H((b), (c), (d)) + (x) + 0x6d703ef3UL;\
      (a) = RMD_ROL((a), (s)) + (e);\
      (c) = RMD_ROL((c), 10);\
   }
#define RMD160_III(a, b, c, d, e, x, s)        {\
      (a) += RMD_I((b), (c), (d)) + (x) + 0x5c4dd124UL;\
      (a) = RMD_ROL((a), (s)) + (e);\
      (c) = RMD_ROL((c), 10);\
   }
#define RMD160_JJJ(a, b, c, d, e, x, s)        {\
      (a) += RMD_J((b), (c), (d)) + (x) + 0x50a28be6UL;\
      (a) = RMD_ROL((a), (s)) + (e);\
      (c) = RMD_ROL((c), 10);\
   }


/********************************************************************/

void MD160init(unsigned long *MDbuf)
{
   MDbuf[0] = RMD_H0; // 0x67452301UL;
   MDbuf[1] = RMD_H1; // 0xefcdab89UL;
   MDbuf[2] = RMD_H2; // 0x98badcfeUL;
   MDbuf[3] = RMD_H3; // 0x10325476UL;
   MDbuf[4] = RMD_H4; // 0xc3d2e1f0UL;

   return;
}

/********************************************************************/

void MD160compress(unsigned long *MDbuf, unsigned long *X)
{
   unsigned long aa = MDbuf[0],  bb = MDbuf[1],  cc = MDbuf[2],
         dd = MDbuf[3],  ee = MDbuf[4];
   unsigned long aaa = MDbuf[0], bbb = MDbuf[1], ccc = MDbuf[2],
         ddd = MDbuf[3], eee = MDbuf[4];

   /* round 1 */
   RMD160_FF(aa, bb, cc, dd, ee, X[ 0], 11);
   RMD160_FF(ee, aa, bb, cc, dd, X[ 1], 14);
   RMD160_FF(dd, ee, aa, bb, cc, X[ 2], 15);
   RMD160_FF(cc, dd, ee, aa, bb, X[ 3], 12);
   RMD160_FF(bb, cc, dd, ee, aa, X[ 4],  5);
   RMD160_FF(aa, bb, cc, dd, ee, X[ 5],  8);
   RMD160_FF(ee, aa, bb, cc, dd, X[ 6],  7);
   RMD160_FF(dd, ee, aa, bb, cc, X[ 7],  9);
   RMD160_FF(cc, dd, ee, aa, bb, X[ 8], 11);
   RMD160_FF(bb, cc, dd, ee, aa, X[ 9], 13);
   RMD160_FF(aa, bb, cc, dd, ee, X[10], 14);
   RMD160_FF(ee, aa, bb, cc, dd, X[11], 15);
   RMD160_FF(dd, ee, aa, bb, cc, X[12],  6);
   RMD160_FF(cc, dd, ee, aa, bb, X[13],  7);
   RMD160_FF(bb, cc, dd, ee, aa, X[14],  9);
   RMD160_FF(aa, bb, cc, dd, ee, X[15],  8);
                             
   /* round 2 */
   RMD160_GG(ee, aa, bb, cc, dd, X[ 7],  7);
   RMD160_GG(dd, ee, aa, bb, cc, X[ 4],  6);
   RMD160_GG(cc, dd, ee, aa, bb, X[13],  8);
   RMD160_GG(bb, cc, dd, ee, aa, X[ 1], 13);
   RMD160_GG(aa, bb, cc, dd, ee, X[10], 11);
   RMD160_GG(ee, aa, bb, cc, dd, X[ 6],  9);
   RMD160_GG(dd, ee, aa, bb, cc, X[15],  7);
   RMD160_GG(cc, dd, ee, aa, bb, X[ 3], 15);
   RMD160_GG(bb, cc, dd, ee, aa, X[12],  7);
   RMD160_GG(aa, bb, cc, dd, ee, X[ 0], 12);
   RMD160_GG(ee, aa, bb, cc, dd, X[ 9], 15);
   RMD160_GG(dd, ee, aa, bb, cc, X[ 5],  9);
   RMD160_GG(cc, dd, ee, aa, bb, X[ 2], 11);
   RMD160_GG(bb, cc, dd, ee, aa, X[14],  7);
   RMD160_GG(aa, bb, cc, dd, ee, X[11], 13);
   RMD160_GG(ee, aa, bb, cc, dd, X[ 8], 12);

   /* round 3 */
   RMD160_HH(dd, ee, aa, bb, cc, X[ 3], 11);
   RMD160_HH(cc, dd, ee, aa, bb, X[10], 13);
   RMD160_HH(bb, cc, dd, ee, aa, X[14],  6);
   RMD160_HH(aa, bb, cc, dd, ee, X[ 4],  7);
   RMD160_HH(ee, aa, bb, cc, dd, X[ 9], 14);
   RMD160_HH(dd, ee, aa, bb, cc, X[15],  9);
   RMD160_HH(cc, dd, ee, aa, bb, X[ 8], 13);
   RMD160_HH(bb, cc, dd, ee, aa, X[ 1], 15);
   RMD160_HH(aa, bb, cc, dd, ee, X[ 2], 14);
   RMD160_HH(ee, aa, bb, cc, dd, X[ 7],  8);
   RMD160_HH(dd, ee, aa, bb, cc, X[ 0], 13);
   RMD160_HH(cc, dd, ee, aa, bb, X[ 6],  6);
   RMD160_HH(bb, cc, dd, ee, aa, X[13],  5);
   RMD160_HH(aa, bb, cc, dd, ee, X[11], 12);
   RMD160_HH(ee, aa, bb, cc, dd, X[ 5],  7);
   RMD160_HH(dd, ee, aa, bb, cc, X[12],  5);

   /* round 4 */
   RMD160_II(cc, dd, ee, aa, bb, X[ 1], 11);
   RMD160_II(bb, cc, dd, ee, aa, X[ 9], 12);
   RMD160_II(aa, bb, cc, dd, ee, X[11], 14);
   RMD160_II(ee, aa, bb, cc, dd, X[10], 15);
   RMD160_II(dd, ee, aa, bb, cc, X[ 0], 14);
   RMD160_II(cc, dd, ee, aa, bb, X[ 8], 15);
   RMD160_II(bb, cc, dd, ee, aa, X[12],  9);
   RMD160_II(aa, bb, cc, dd, ee, X[ 4],  8);
   RMD160_II(ee, aa, bb, cc, dd, X[13],  9);
   RMD160_II(dd, ee, aa, bb, cc, X[ 3], 14);
   RMD160_II(cc, dd, ee, aa, bb, X[ 7],  5);
   RMD160_II(bb, cc, dd, ee, aa, X[15],  6);
   RMD160_II(aa, bb, cc, dd, ee, X[14],  8);
   RMD160_II(ee, aa, bb, cc, dd, X[ 5],  6);
   RMD160_II(dd, ee, aa, bb, cc, X[ 6],  5);
   RMD160_II(cc, dd, ee, aa, bb, X[ 2], 12);

   /* round 5 */
   RMD160_JJ(bb, cc, dd, ee, aa, X[ 4],  9);
   RMD160_JJ(aa, bb, cc, dd, ee, X[ 0], 15);
   RMD160_JJ(ee, aa, bb, cc, dd, X[ 5],  5);
   RMD160_JJ(dd, ee, aa, bb, cc, X[ 9], 11);
   RMD160_JJ(cc, dd, ee, aa, bb, X[ 7],  6);
   RMD160_JJ(bb, cc, dd, ee, aa, X[12],  8);
   RMD160_JJ(aa, bb, cc, dd, ee, X[ 2], 13);
   RMD160_JJ(ee, aa, bb, cc, dd, X[10], 12);
   RMD160_JJ(dd, ee, aa, bb, cc, X[14],  5);
   RMD160_JJ(cc, dd, ee, aa, bb, X[ 1], 12);
   RMD160_JJ(bb, cc, dd, ee, aa, X[ 3], 13);
   RMD160_JJ(aa, bb, cc, dd, ee, X[ 8], 14);
   RMD160_JJ(ee, aa, bb, cc, dd, X[11], 11);
   RMD160_JJ(dd, ee, aa, bb, cc, X[ 6],  8);
   RMD160_JJ(cc, dd, ee, aa, bb, X[15],  5);
   RMD160_JJ(bb, cc, dd, ee, aa, X[13],  6);

   /* parallel round 1 */
   RMD160_JJJ(aaa, bbb, ccc, ddd, eee, X[ 5],  8);
   RMD160_JJJ(eee, aaa, bbb, ccc, ddd, X[14],  9);
   RMD160_JJJ(ddd, eee, aaa, bbb, ccc, X[ 7],  9);
   RMD160_JJJ(ccc, ddd, eee, aaa, bbb, X[ 0], 11);
   RMD160_JJJ(bbb, ccc, ddd, eee, aaa, X[ 9], 13);
   RMD160_JJJ(aaa, bbb, ccc, ddd, eee, X[ 2], 15);
   RMD160_JJJ(eee, aaa, bbb, ccc, ddd, X[11], 15);
   RMD160_JJJ(ddd, eee, aaa, bbb, ccc, X[ 4],  5);
   RMD160_JJJ(ccc, ddd, eee, aaa, bbb, X[13],  7);
   RMD160_JJJ(bbb, ccc, ddd, eee, aaa, X[ 6],  7);
   RMD160_JJJ(aaa, bbb, ccc, ddd, eee, X[15],  8);
   RMD160_JJJ(eee, aaa, bbb, ccc, ddd, X[ 8], 11);
   RMD160_JJJ(ddd, eee, aaa, bbb, ccc, X[ 1], 14);
   RMD160_JJJ(ccc, ddd, eee, aaa, bbb, X[10], 14);
   RMD160_JJJ(bbb, ccc, ddd, eee, aaa, X[ 3], 12);
   RMD160_JJJ(aaa, bbb, ccc, ddd, eee, X[12],  6);

   /* parallel round 2 */
   RMD160_III(eee, aaa, bbb, ccc, ddd, X[ 6],  9); 
   RMD160_III(ddd, eee, aaa, bbb, ccc, X[11], 13);
   RMD160_III(ccc, ddd, eee, aaa, bbb, X[ 3], 15);
   RMD160_III(bbb, ccc, ddd, eee, aaa, X[ 7],  7);
   RMD160_III(aaa, bbb, ccc, ddd, eee, X[ 0], 12);
   RMD160_III(eee, aaa, bbb, ccc, ddd, X[13],  8);
   RMD160_III(ddd, eee, aaa, bbb, ccc, X[ 5],  9);
   RMD160_III(ccc, ddd, eee, aaa, bbb, X[10], 11);
   RMD160_III(bbb, ccc, ddd, eee, aaa, X[14],  7);
   RMD160_III(aaa, bbb, ccc, ddd, eee, X[15],  7);
   RMD160_III(eee, aaa, bbb, ccc, ddd, X[ 8], 12);
   RMD160_III(ddd, eee, aaa, bbb, ccc, X[12],  7);
   RMD160_III(ccc, ddd, eee, aaa, bbb, X[ 4],  6);
   RMD160_III(bbb, ccc, ddd, eee, aaa, X[ 9], 15);
   RMD160_III(aaa, bbb, ccc, ddd, eee, X[ 1], 13);
   RMD160_III(eee, aaa, bbb, ccc, ddd, X[ 2], 11);

   /* parallel round 3 */
   RMD160_HHH(ddd, eee, aaa, bbb, ccc, X[15],  9);
   RMD160_HHH(ccc, ddd, eee, aaa, bbb, X[ 5],  7);
   RMD160_HHH(bbb, ccc, ddd, eee, aaa, X[ 1], 15);
   RMD160_HHH(aaa, bbb, ccc, ddd, eee, X[ 3], 11);
   RMD160_HHH(eee, aaa, bbb, ccc, ddd, X[ 7],  8);
   RMD160_HHH(ddd, eee, aaa, bbb, ccc, X[14],  6);
   RMD160_HHH(ccc, ddd, eee, aaa, bbb, X[ 6],  6);
   RMD160_HHH(bbb, ccc, ddd, eee, aaa, X[ 9], 14);
   RMD160_HHH(aaa, bbb, ccc, ddd, eee, X[11], 12);
   RMD160_HHH(eee, aaa, bbb, ccc, ddd, X[ 8], 13);
   RMD160_HHH(ddd, eee, aaa, bbb, ccc, X[12],  5);
   RMD160_HHH(ccc, ddd, eee, aaa, bbb, X[ 2], 14);
   RMD160_HHH(bbb, ccc, ddd, eee, aaa, X[10], 13);
   RMD160_HHH(aaa, bbb, ccc, ddd, eee, X[ 0], 13);
   RMD160_HHH(eee, aaa, bbb, ccc, ddd, X[ 4],  7);
   RMD160_HHH(ddd, eee, aaa, bbb, ccc, X[13],  5);

   /* parallel round 4 */   
   RMD160_GGG(ccc, ddd, eee, aaa, bbb, X[ 8], 15);
   RMD160_GGG(bbb, ccc, ddd, eee, aaa, X[ 6],  5);
   RMD160_GGG(aaa, bbb, ccc, ddd, eee, X[ 4],  8);
   RMD160_GGG(eee, aaa, bbb, ccc, ddd, X[ 1], 11);
   RMD160_GGG(ddd, eee, aaa, bbb, ccc, X[ 3], 14);
   RMD160_GGG(ccc, ddd, eee, aaa, bbb, X[11], 14);
   RMD160_GGG(bbb, ccc, ddd, eee, aaa, X[15],  6);
   RMD160_GGG(aaa, bbb, ccc, ddd, eee, X[ 0], 14);
   RMD160_GGG(eee, aaa, bbb, ccc, ddd, X[ 5],  6);
   RMD160_GGG(ddd, eee, aaa, bbb, ccc, X[12],  9);
   RMD160_GGG(ccc, ddd, eee, aaa, bbb, X[ 2], 12);
   RMD160_GGG(bbb, ccc, ddd, eee, aaa, X[13],  9);
   RMD160_GGG(aaa, bbb, ccc, ddd, eee, X[ 9], 12);
   RMD160_GGG(eee, aaa, bbb, ccc, ddd, X[ 7],  5);
   RMD160_GGG(ddd, eee, aaa, bbb, ccc, X[10], 15);
   RMD160_GGG(ccc, ddd, eee, aaa, bbb, X[14],  8);

   /* parallel round 5 */
   RMD160_FFF(bbb, ccc, ddd, eee, aaa, X[12] ,  8);
   RMD160_FFF(aaa, bbb, ccc, ddd, eee, X[15] ,  5);
   RMD160_FFF(eee, aaa, bbb, ccc, ddd, X[10] , 12);
   RMD160_FFF(ddd, eee, aaa, bbb, ccc, X[ 4] ,  9);
   RMD160_FFF(ccc, ddd, eee, aaa, bbb, X[ 1] , 12);
   RMD160_FFF(bbb, ccc, ddd, eee, aaa, X[ 5] ,  5);
   RMD160_FFF(aaa, bbb, ccc, ddd, eee, X[ 8] , 14);
   RMD160_FFF(eee, aaa, bbb, ccc, ddd, X[ 7] ,  6);
   RMD160_FFF(ddd, eee, aaa, bbb, ccc, X[ 6] ,  8);
   RMD160_FFF(ccc, ddd, eee, aaa, bbb, X[ 2] , 13);
   RMD160_FFF(bbb, ccc, ddd, eee, aaa, X[13] ,  6);
   RMD160_FFF(aaa, bbb, ccc, ddd, eee, X[14] ,  5);
   RMD160_FFF(eee, aaa, bbb, ccc, ddd, X[ 0] , 15);
   RMD160_FFF(ddd, eee, aaa, bbb, ccc, X[ 3] , 13);
   RMD160_FFF(ccc, ddd, eee, aaa, bbb, X[ 9] , 11);
   RMD160_FFF(bbb, ccc, ddd, eee, aaa, X[11] , 11);

   /* combine results */
   ddd += cc + MDbuf[1];               /* final result for MDbuf[0] */
   MDbuf[1] = MDbuf[2] + dd + eee;
   MDbuf[2] = MDbuf[3] + ee + aaa;
   MDbuf[3] = MDbuf[4] + aa + bbb;
   MDbuf[4] = MDbuf[0] + bb + ccc;
   MDbuf[0] = ddd;

   return;
}

/********************************************************************/

void MD160finish(unsigned long *MDbuf, unsigned char *strptr, unsigned long lswlen, unsigned long mswlen)
{
   unsigned int i;                                 /* counter       */
   unsigned long        X[16];                             /* message words */

   memset(X, 0, 16*sizeof(unsigned long));

   /* put bytes from strptr into X */
   for (i=0; i<(lswlen&63); i++) {
      /* byte i goes into word X[i div 4] at pos.  8*(i mod 4)  */
      X[i>>2] ^= (unsigned long) *strptr++ << (8 * (i&3));
   }

   /* append the bit m_n == 1 */
   X[(lswlen>>2)&15] ^= (unsigned long)1 << (8*(lswlen&3) + 7);

   if ((lswlen & 63) > 55) {
      /* length goes to next block */
      MD160compress(MDbuf, X);
      memset(X, 0, 16*sizeof(unsigned long));
   }

   /* append length in bits*/
   X[14] = lswlen << 3;
   X[15] = (lswlen >> 29) | (mswlen << 3);
   MD160compress(MDbuf, X);

   return;
}

#endif // RIPEMD160



#if RIPEMD320

struct rmd320_ctx {
	unsigned long long byte_count;
	unsigned int state[10];
	__le32 buffer[16];
};



#define K1  RMD_K1
#define K2  RMD_K2
#define K3  RMD_K3
#define K4  RMD_K4
#define K5  RMD_K5
#define KK1 RMD_K6
#define KK2 RMD_K7
#define KK3 RMD_K8
#define KK4 RMD_K9
#define KK5 RMD_K1
#define F1(x, y, z) (x ^ y ^ z)		/* XOR */
#define F2(x, y, z) (z ^ (x & (y ^ z)))	/* x ? y : z */
#define F3(x, y, z) ((x | ~y) ^ z)
#define F4(x, y, z) (y ^ (z & (x ^ y)))	/* z ? x : y */
#define F5(x, y, z) (x ^ (y | ~z))
#define ROUND(a, b, c, d, e, f, k, x, s)  { \
	(a) += f((b), (c), (d)) + le32_to_cpup(&(x)) + (k); \
	(a) = rol32((a), (s)) + (e); \
	(c) = rol32((c), 10); \
}
static void rmd320_transform(unsigned int *state, const __le32 *in)
{
	unsigned int aa, bb, cc, dd, ee, aaa, bbb, ccc, ddd, eee;
	/* Initialize left lane */
	aa = state[0];
	bb = state[1];
	cc = state[2];
	dd = state[3];
	ee = state[4];
	/* Initialize right lane */
	aaa = state[5];
	bbb = state[6];
	ccc = state[7];
	ddd = state[8];
	eee = state[9];
	/* round 1: left lane */
	ROUND(aa, bb, cc, dd, ee, F1, K1, in[0],  11);
	ROUND(ee, aa, bb, cc, dd, F1, K1, in[1],  14);
	ROUND(dd, ee, aa, bb, cc, F1, K1, in[2],  15);
	ROUND(cc, dd, ee, aa, bb, F1, K1, in[3],  12);
	ROUND(bb, cc, dd, ee, aa, F1, K1, in[4],   5);
	ROUND(aa, bb, cc, dd, ee, F1, K1, in[5],   8);
	ROUND(ee, aa, bb, cc, dd, F1, K1, in[6],   7);
	ROUND(dd, ee, aa, bb, cc, F1, K1, in[7],   9);
	ROUND(cc, dd, ee, aa, bb, F1, K1, in[8],  11);
	ROUND(bb, cc, dd, ee, aa, F1, K1, in[9],  13);
	ROUND(aa, bb, cc, dd, ee, F1, K1, in[10], 14);
	ROUND(ee, aa, bb, cc, dd, F1, K1, in[11], 15);
	ROUND(dd, ee, aa, bb, cc, F1, K1, in[12],  6);
	ROUND(cc, dd, ee, aa, bb, F1, K1, in[13],  7);
	ROUND(bb, cc, dd, ee, aa, F1, K1, in[14],  9);
	ROUND(aa, bb, cc, dd, ee, F1, K1, in[15],  8);
	/* round 1: right lane */
	ROUND(aaa, bbb, ccc, ddd, eee, F5, KK1, in[5],   8);
	ROUND(eee, aaa, bbb, ccc, ddd, F5, KK1, in[14],  9);
	ROUND(ddd, eee, aaa, bbb, ccc, F5, KK1, in[7],   9);
	ROUND(ccc, ddd, eee, aaa, bbb, F5, KK1, in[0],  11);
	ROUND(bbb, ccc, ddd, eee, aaa, F5, KK1, in[9],  13);
	ROUND(aaa, bbb, ccc, ddd, eee, F5, KK1, in[2],  15);
	ROUND(eee, aaa, bbb, ccc, ddd, F5, KK1, in[11], 15);
	ROUND(ddd, eee, aaa, bbb, ccc, F5, KK1, in[4],   5);
	ROUND(ccc, ddd, eee, aaa, bbb, F5, KK1, in[13],  7);
	ROUND(bbb, ccc, ddd, eee, aaa, F5, KK1, in[6],   7);
	ROUND(aaa, bbb, ccc, ddd, eee, F5, KK1, in[15],  8);
	ROUND(eee, aaa, bbb, ccc, ddd, F5, KK1, in[8],  11);
	ROUND(ddd, eee, aaa, bbb, ccc, F5, KK1, in[1],  14);
	ROUND(ccc, ddd, eee, aaa, bbb, F5, KK1, in[10], 14);
	ROUND(bbb, ccc, ddd, eee, aaa, F5, KK1, in[3],  12);
	ROUND(aaa, bbb, ccc, ddd, eee, F5, KK1, in[12],  6);
	/* Swap contents of "a" registers */
	swap(aa, aaa);
	/* round 2: left lane" */
	ROUND(ee, aa, bb, cc, dd, F2, K2, in[7],   7);
	ROUND(dd, ee, aa, bb, cc, F2, K2, in[4],   6);
	ROUND(cc, dd, ee, aa, bb, F2, K2, in[13],  8);
	ROUND(bb, cc, dd, ee, aa, F2, K2, in[1],  13);
	ROUND(aa, bb, cc, dd, ee, F2, K2, in[10], 11);
	ROUND(ee, aa, bb, cc, dd, F2, K2, in[6],   9);
	ROUND(dd, ee, aa, bb, cc, F2, K2, in[15],  7);
	ROUND(cc, dd, ee, aa, bb, F2, K2, in[3],  15);
	ROUND(bb, cc, dd, ee, aa, F2, K2, in[12],  7);
	ROUND(aa, bb, cc, dd, ee, F2, K2, in[0],  12);
	ROUND(ee, aa, bb, cc, dd, F2, K2, in[9],  15);
	ROUND(dd, ee, aa, bb, cc, F2, K2, in[5],   9);
	ROUND(cc, dd, ee, aa, bb, F2, K2, in[2],  11);
	ROUND(bb, cc, dd, ee, aa, F2, K2, in[14],  7);
	ROUND(aa, bb, cc, dd, ee, F2, K2, in[11], 13);
	ROUND(ee, aa, bb, cc, dd, F2, K2, in[8],  12);
	/* round 2: right lane */
	ROUND(eee, aaa, bbb, ccc, ddd, F4, KK2, in[6],   9);
	ROUND(ddd, eee, aaa, bbb, ccc, F4, KK2, in[11], 13);
	ROUND(ccc, ddd, eee, aaa, bbb, F4, KK2, in[3],  15);
	ROUND(bbb, ccc, ddd, eee, aaa, F4, KK2, in[7],   7);
	ROUND(aaa, bbb, ccc, ddd, eee, F4, KK2, in[0],  12);
	ROUND(eee, aaa, bbb, ccc, ddd, F4, KK2, in[13],  8);
	ROUND(ddd, eee, aaa, bbb, ccc, F4, KK2, in[5],   9);
	ROUND(ccc, ddd, eee, aaa, bbb, F4, KK2, in[10], 11);
	ROUND(bbb, ccc, ddd, eee, aaa, F4, KK2, in[14],  7);
	ROUND(aaa, bbb, ccc, ddd, eee, F4, KK2, in[15],  7);
	ROUND(eee, aaa, bbb, ccc, ddd, F4, KK2, in[8],  12);
	ROUND(ddd, eee, aaa, bbb, ccc, F4, KK2, in[12],  7);
	ROUND(ccc, ddd, eee, aaa, bbb, F4, KK2, in[4],   6);
	ROUND(bbb, ccc, ddd, eee, aaa, F4, KK2, in[9],  15);
	ROUND(aaa, bbb, ccc, ddd, eee, F4, KK2, in[1],  13);
	ROUND(eee, aaa, bbb, ccc, ddd, F4, KK2, in[2],  11);
	/* Swap contents of "b" registers */
	swap(bb, bbb);
	/* round 3: left lane" */
	ROUND(dd, ee, aa, bb, cc, F3, K3, in[3],  11);
	ROUND(cc, dd, ee, aa, bb, F3, K3, in[10], 13);
	ROUND(bb, cc, dd, ee, aa, F3, K3, in[14],  6);
	ROUND(aa, bb, cc, dd, ee, F3, K3, in[4],   7);
	ROUND(ee, aa, bb, cc, dd, F3, K3, in[9],  14);
	ROUND(dd, ee, aa, bb, cc, F3, K3, in[15],  9);
	ROUND(cc, dd, ee, aa, bb, F3, K3, in[8],  13);
	ROUND(bb, cc, dd, ee, aa, F3, K3, in[1],  15);
	ROUND(aa, bb, cc, dd, ee, F3, K3, in[2],  14);
	ROUND(ee, aa, bb, cc, dd, F3, K3, in[7],   8);
	ROUND(dd, ee, aa, bb, cc, F3, K3, in[0],  13);
	ROUND(cc, dd, ee, aa, bb, F3, K3, in[6],   6);
	ROUND(bb, cc, dd, ee, aa, F3, K3, in[13],  5);
	ROUND(aa, bb, cc, dd, ee, F3, K3, in[11], 12);
	ROUND(ee, aa, bb, cc, dd, F3, K3, in[5],   7);
	ROUND(dd, ee, aa, bb, cc, F3, K3, in[12],  5);
	/* round 3: right lane */
	ROUND(ddd, eee, aaa, bbb, ccc, F3, KK3, in[15],  9);
	ROUND(ccc, ddd, eee, aaa, bbb, F3, KK3, in[5],   7);
	ROUND(bbb, ccc, ddd, eee, aaa, F3, KK3, in[1],  15);
	ROUND(aaa, bbb, ccc, ddd, eee, F3, KK3, in[3],  11);
	ROUND(eee, aaa, bbb, ccc, ddd, F3, KK3, in[7],   8);
	ROUND(ddd, eee, aaa, bbb, ccc, F3, KK3, in[14],  6);
	ROUND(ccc, ddd, eee, aaa, bbb, F3, KK3, in[6],   6);
	ROUND(bbb, ccc, ddd, eee, aaa, F3, KK3, in[9],  14);
	ROUND(aaa, bbb, ccc, ddd, eee, F3, KK3, in[11], 12);
	ROUND(eee, aaa, bbb, ccc, ddd, F3, KK3, in[8],  13);
	ROUND(ddd, eee, aaa, bbb, ccc, F3, KK3, in[12],  5);
	ROUND(ccc, ddd, eee, aaa, bbb, F3, KK3, in[2],  14);
	ROUND(bbb, ccc, ddd, eee, aaa, F3, KK3, in[10], 13);
	ROUND(aaa, bbb, ccc, ddd, eee, F3, KK3, in[0],  13);
	ROUND(eee, aaa, bbb, ccc, ddd, F3, KK3, in[4],   7);
	ROUND(ddd, eee, aaa, bbb, ccc, F3, KK3, in[13],  5);
	/* Swap contents of "c" registers */
	swap(cc, ccc);
	/* round 4: left lane" */
	ROUND(cc, dd, ee, aa, bb, F4, K4, in[1],  11);
	ROUND(bb, cc, dd, ee, aa, F4, K4, in[9],  12);
	ROUND(aa, bb, cc, dd, ee, F4, K4, in[11], 14);
	ROUND(ee, aa, bb, cc, dd, F4, K4, in[10], 15);
	ROUND(dd, ee, aa, bb, cc, F4, K4, in[0],  14);
	ROUND(cc, dd, ee, aa, bb, F4, K4, in[8],  15);
	ROUND(bb, cc, dd, ee, aa, F4, K4, in[12],  9);
	ROUND(aa, bb, cc, dd, ee, F4, K4, in[4],   8);
	ROUND(ee, aa, bb, cc, dd, F4, K4, in[13],  9);
	ROUND(dd, ee, aa, bb, cc, F4, K4, in[3],  14);
	ROUND(cc, dd, ee, aa, bb, F4, K4, in[7],   5);
	ROUND(bb, cc, dd, ee, aa, F4, K4, in[15],  6);
	ROUND(aa, bb, cc, dd, ee, F4, K4, in[14],  8);
	ROUND(ee, aa, bb, cc, dd, F4, K4, in[5],   6);
	ROUND(dd, ee, aa, bb, cc, F4, K4, in[6],   5);
	ROUND(cc, dd, ee, aa, bb, F4, K4, in[2],  12);
	/* round 4: right lane */
	ROUND(ccc, ddd, eee, aaa, bbb, F2, KK4, in[8],  15);
	ROUND(bbb, ccc, ddd, eee, aaa, F2, KK4, in[6],   5);
	ROUND(aaa, bbb, ccc, ddd, eee, F2, KK4, in[4],   8);
	ROUND(eee, aaa, bbb, ccc, ddd, F2, KK4, in[1],  11);
	ROUND(ddd, eee, aaa, bbb, ccc, F2, KK4, in[3],  14);
	ROUND(ccc, ddd, eee, aaa, bbb, F2, KK4, in[11], 14);
	ROUND(bbb, ccc, ddd, eee, aaa, F2, KK4, in[15],  6);
	ROUND(aaa, bbb, ccc, ddd, eee, F2, KK4, in[0],  14);
	ROUND(eee, aaa, bbb, ccc, ddd, F2, KK4, in[5],   6);
	ROUND(ddd, eee, aaa, bbb, ccc, F2, KK4, in[12],  9);
	ROUND(ccc, ddd, eee, aaa, bbb, F2, KK4, in[2],  12);
	ROUND(bbb, ccc, ddd, eee, aaa, F2, KK4, in[13],  9);
	ROUND(aaa, bbb, ccc, ddd, eee, F2, KK4, in[9],  12);
	ROUND(eee, aaa, bbb, ccc, ddd, F2, KK4, in[7],   5);
	ROUND(ddd, eee, aaa, bbb, ccc, F2, KK4, in[10], 15);
	ROUND(ccc, ddd, eee, aaa, bbb, F2, KK4, in[14],  8);
	/* Swap contents of "d" registers */
	swap(dd, ddd);
	/* round 5: left lane" */
	ROUND(bb, cc, dd, ee, aa, F5, K5, in[4],   9);
	ROUND(aa, bb, cc, dd, ee, F5, K5, in[0],  15);
	ROUND(ee, aa, bb, cc, dd, F5, K5, in[5],   5);
	ROUND(dd, ee, aa, bb, cc, F5, K5, in[9],  11);
	ROUND(cc, dd, ee, aa, bb, F5, K5, in[7],   6);
	ROUND(bb, cc, dd, ee, aa, F5, K5, in[12],  8);
	ROUND(aa, bb, cc, dd, ee, F5, K5, in[2],  13);
	ROUND(ee, aa, bb, cc, dd, F5, K5, in[10], 12);
	ROUND(dd, ee, aa, bb, cc, F5, K5, in[14],  5);
	ROUND(cc, dd, ee, aa, bb, F5, K5, in[1],  12);
	ROUND(bb, cc, dd, ee, aa, F5, K5, in[3],  13);
	ROUND(aa, bb, cc, dd, ee, F5, K5, in[8],  14);
	ROUND(ee, aa, bb, cc, dd, F5, K5, in[11], 11);
	ROUND(dd, ee, aa, bb, cc, F5, K5, in[6],   8);
	ROUND(cc, dd, ee, aa, bb, F5, K5, in[15],  5);
	ROUND(bb, cc, dd, ee, aa, F5, K5, in[13],  6);
	/* round 5: right lane */
	ROUND(bbb, ccc, ddd, eee, aaa, F1, KK5, in[12],  8);
	ROUND(aaa, bbb, ccc, ddd, eee, F1, KK5, in[15],  5);
	ROUND(eee, aaa, bbb, ccc, ddd, F1, KK5, in[10], 12);
	ROUND(ddd, eee, aaa, bbb, ccc, F1, KK5, in[4],   9);
	ROUND(ccc, ddd, eee, aaa, bbb, F1, KK5, in[1],  12);
	ROUND(bbb, ccc, ddd, eee, aaa, F1, KK5, in[5],   5);
	ROUND(aaa, bbb, ccc, ddd, eee, F1, KK5, in[8],  14);
	ROUND(eee, aaa, bbb, ccc, ddd, F1, KK5, in[7],   6);
	ROUND(ddd, eee, aaa, bbb, ccc, F1, KK5, in[6],   8);
	ROUND(ccc, ddd, eee, aaa, bbb, F1, KK5, in[2],  13);
	ROUND(bbb, ccc, ddd, eee, aaa, F1, KK5, in[13],  6);
	ROUND(aaa, bbb, ccc, ddd, eee, F1, KK5, in[14],  5);
	ROUND(eee, aaa, bbb, ccc, ddd, F1, KK5, in[0],  15);
	ROUND(ddd, eee, aaa, bbb, ccc, F1, KK5, in[3],  13);
	ROUND(ccc, ddd, eee, aaa, bbb, F1, KK5, in[9],  11);
	ROUND(bbb, ccc, ddd, eee, aaa, F1, KK5, in[11], 11);
	/* Swap contents of "e" registers */
	swap(ee, eee);
	/* combine results */
	state[0] += aa;
	state[1] += bb;
	state[2] += cc;
	state[3] += dd;
	state[4] += ee;
	state[5] += aaa;
	state[6] += bbb;
	state[7] += ccc;
	state[8] += ddd;
	state[9] += eee;
}
static int rmd320_init(struct shash_desc *desc)
{
	struct rmd320_ctx *rctx = shash_desc_ctx(desc);
	rctx->byte_count = 0;
	rctx->state[0] = RMD_H0;
	rctx->state[1] = RMD_H1;
	rctx->state[2] = RMD_H2;
	rctx->state[3] = RMD_H3;
	rctx->state[4] = RMD_H4;
	rctx->state[5] = RMD_H5;
	rctx->state[6] = RMD_H6;
	rctx->state[7] = RMD_H7;
	rctx->state[8] = RMD_H8;
	rctx->state[9] = RMD_H9;
	memset(rctx->buffer, 0, sizeof(rctx->buffer));
	return 0;
}
static int rmd320_update(struct shash_desc *desc, const u8 *data,
			 unsigned int len)
{
	struct rmd320_ctx *rctx = shash_desc_ctx(desc);
	const u32 avail = sizeof(rctx->buffer) - (rctx->byte_count & 0x3f);
	rctx->byte_count += len;
	/* Enough space in buffer? If so copy and we're done */
	if (avail > len) {
		memcpy((char *)rctx->buffer + (sizeof(rctx->buffer) - avail),
		       data, len);
		goto out;
	}
	memcpy((char *)rctx->buffer + (sizeof(rctx->buffer) - avail),
	       data, avail);
	rmd320_transform(rctx->state, rctx->buffer);
	data += avail;
	len -= avail;
	while (len >= sizeof(rctx->buffer)) {
		memcpy(rctx->buffer, data, sizeof(rctx->buffer));
		rmd320_transform(rctx->state, rctx->buffer);
		data += sizeof(rctx->buffer);
		len -= sizeof(rctx->buffer);
	}
	memcpy(rctx->buffer, data, len);
out:
	return 0;
}
/* Add padding and return the message digest. */
static int rmd320_final(struct shash_desc *desc, u8 *out)
{
	struct rmd320_ctx *rctx = shash_desc_ctx(desc);
	u32 i, index, padlen;
	__le64 bits;
	__le32 *dst = (__le32 *)out;
	static const u8 padding[64] = { 0x80, };
	bits = cpu_to_le64(rctx->byte_count << 3);
	/* Pad out to 56 mod 64 */
	index = rctx->byte_count & 0x3f;
	padlen = (index < 56) ? (56 - index) : ((64+56) - index);
	rmd320_update(desc, padding, padlen);
	/* Append length */
	rmd320_update(desc, (const u8 *)&bits, sizeof(bits));
	/* Store state in digest */
	for (i = 0; i < 10; i++)
		dst[i] = cpu_to_le32p(&rctx->state[i]);
	/* Wipe context */
	memset(rctx, 0, sizeof(*rctx));
	return 0;
}

#endif






#if MD2_CHECKSUM_ENCIPHER

#define MD2_DIGEST_LENGTH 16

typedef struct md2 {
    size_t len;
    unsigned char data[16]; /* stored unalligned data between Update's */
    unsigned char checksum[16];
    unsigned char state[16]; /* lower 16 bytes of X */
}MD2_CTX;

/// typedef struct md2 MD2_CTX;



/*
 * Copyright (c) 2006 Kungliga Tekniska Hogskolan
 * (Royal Institute of Technology, Stockholm, Sweden).
 * All rights reserved.
 *
 */

const unsigned char subst[256] = {
  41, 46, 67, 201, 162, 216, 124, 1, 61, 54, 84, 161, 236, 240, 6,
  19, 98, 167, 5, 243, 192, 199, 115, 140, 152, 147, 43, 217, 188,
  76, 130, 202, 30, 155, 87, 60, 253, 212, 224, 22, 103, 66, 111, 24,
  138, 23, 229, 18, 190, 78, 196, 214, 218, 158, 222, 73, 160, 251,
  245, 142, 187, 47, 238, 122, 169, 104, 121, 145, 21, 178, 7, 63,
  148, 194, 16, 137, 11, 34, 95, 33, 128, 127, 93, 154, 90, 144, 50,
  39, 53, 62, 204, 231, 191, 247, 151, 3, 255, 25, 48, 179, 72, 165,
  181, 209, 215, 94, 146, 42, 172, 86, 170, 198, 79, 184, 56, 210,
  150, 164, 125, 182, 118, 252, 107, 226, 156, 116, 4, 241, 69, 157,
  112, 89, 100, 113, 135, 32, 134, 91, 207, 101, 230, 45, 168, 2, 27,
  96, 37, 173, 174, 176, 185, 246, 28, 70, 97, 105, 52, 64, 126, 15,
  85, 71, 163, 35, 221, 81, 175, 58, 195, 92, 249, 206, 186, 197,
  234, 38, 44, 83, 13, 110, 133, 40, 132, 9, 211, 223, 205, 244, 65,
  129, 77, 82, 106, 220, 55, 200, 108, 193, 171, 250, 36, 225, 123,
  8, 12, 189, 177, 74, 120, 136, 149, 139, 227, 99, 232, 109, 233,
  203, 213, 254, 59, 0, 29, 57, 242, 239, 183, 14, 102, 88, 208, 228,
  166, 119, 114, 248, 235, 117, 75, 10, 49, 68, 80, 180, 143, 237,
  31, 26, 219, 153, 141, 51, 159, 17, 131, 20
};

void MD2_Init (MD2_CTX *m)
{
	memset(m, 0, sizeof(*m));
}


static void MD2_calc(MD2_CTX *m, const void *v)
{
    unsigned char x[48], L;
    const unsigned char *p = v;
    int i, j, t;

    L = m->checksum[15];
    for (i = 0; i < 16; i++)
		L = m->checksum[i] ^= subst[p[i] ^ L];

    for (i = 0; i < 16; i++) {
		x[i]      = m->state[i];
		x[i + 16] = p[i];
		x[i + 32] = x[i] ^ p[i];
    }

    t = 0;

    for (i = 0; i < 18; i++) {
		for (j = 0; j < 48; j++)
		    t = x[j] ^= subst[t];
		t = (t + i) & 0xff;
    }

    memcpy(m->state, x, 16);
    memset(x, 0, sizeof(x));
}


void MD2_Update (MD2_CTX *m, const void *v, size_t len)
{
    size_t idx = m->len & 0xf;
    const unsigned char *p = v;

    m->len += len;
    if (len + idx >= 16) {
		if (idx) {
		    memcpy(m->data + idx, p, 16 - idx);
		    MD2_calc(m, m->data);
		    p += 16;
		    len -= 16 - idx;
		}
		while (len >= 16) {
		    MD2_calc(m, p);
		    p += 16;
		    len -= 16;
		}
		idx = 0;
    }

    memcpy(m->data + idx, p, len);
}

void MD2_Final (void *res, MD2_CTX *m)
{
    unsigned char pad[16];
    size_t padlen;

    padlen = 16 - (m->len % 16);
    memset(pad, padlen, padlen);

    MD2_Update(m, pad, padlen);
    memcpy(pad, m->checksum, 16);
    MD2_Update(m, pad, 16);

    memcpy(res, m->state, MD2_DIGEST_LENGTH);
    memset(m, 0, sizeof(m));
}

static void MD2Print (unsigned char *MD2_digest)
{
  int i;

  for (i = 0; i < MD2_DIGEST_LENGTH; i++)
  {
    printf ("%02x", MD2_digest[i]);
	if(outfile) fprintf(outfile, "%02x", MD2_digest[i]);
  }
}
#endif /// MD2_CHECKSUM_ENCIPHER


#if MD4_CHECKSUM_ENCIPHER

#define	MD4_BLOCK_LENGTH 				64
#define	MD4_DIGEST_LENGTH 				16
#define	MD4_DIGEST_STRING_LENGTH 		(MD4_DIGEST_LENGTH * 2 + 1)

typedef struct MD4Context {
	unsigned int state[4];			/* state */
	unsigned int count[2];			/* number of bits, mod 2^64 */
	unsigned char buffer[MD4_BLOCK_LENGTH];	/* input buffer */
} MD4_CTX;


void MD4Transform(unsigned int buf[4], const unsigned char *inc);


/*	The below was retrieved from
 *	http://www.openbsd.org/cgi-bin/cvsweb/~checkout~/src/lib/libc/hash/md4.c?rev=1.2
 *	with the following changes:
 *	CVS-$OpenBSD stuff deleted
 *	#includes commented out.
 *	Support context->count as unsigned int[2] instead of uint64_t
 *	Add htole32 define from http://www.squid-cache.org/mail-archive/squid-dev/200307/0130.html
 *		(The bswap32 definition in the patch.)
 *		This is only used on BIG_ENDIAN systems, so we can always swap the bits.
 *	change BYTE_ORDER == LITTLE_ENDIAN (OpenBSD-defined) to WORDS_BIGENDIAN (autoconf-defined)
 */



/*#if BYTE_ORDER == LITTLE_ENDIAN*/
#ifndef WORDS_BIGENDIAN

#define htole32_4(buf)		/* Nothing */
#define htole32_14(buf)		/* Nothing */
#define htole32_16(buf)		/* Nothing */

#else

#define htole32(x) \
 (((((unsigned int)x) & 0xff000000) >> 24) | \
 ((((unsigned int)x) & 0x00ff0000) >> 8) | \
 ((((unsigned int)x) & 0x0000ff00) << 8) | \
 ((((unsigned int)x) & 0x000000ff) << 24)) 

#define htole32_4(buf) do {				\
	(buf)[ 0] = htole32((buf)[ 0]);		\
	(buf)[ 1] = htole32((buf)[ 1]);		\
	(buf)[ 2] = htole32((buf)[ 2]);		\
	(buf)[ 3] = htole32((buf)[ 3]);		\
} while (0)

#define htole32_14(buf) do {			\
	(buf)[ 0] = htole32((buf)[ 0]);		\
	(buf)[ 1] = htole32((buf)[ 1]);		\
	(buf)[ 2] = htole32((buf)[ 2]);		\
	(buf)[ 3] = htole32((buf)[ 3]);		\
	(buf)[ 4] = htole32((buf)[ 4]);		\
	(buf)[ 5] = htole32((buf)[ 5]);		\
	(buf)[ 6] = htole32((buf)[ 6]);		\
	(buf)[ 7] = htole32((buf)[ 7]);		\
	(buf)[ 8] = htole32((buf)[ 8]);		\
	(buf)[ 9] = htole32((buf)[ 9]);		\
	(buf)[10] = htole32((buf)[10]);		\
	(buf)[11] = htole32((buf)[11]);		\
	(buf)[12] = htole32((buf)[12]);		\
	(buf)[13] = htole32((buf)[13]);		\
} while (0)

#define htole32_16(buf) do {			\
	(buf)[ 0] = htole32((buf)[ 0]);		\
	(buf)[ 1] = htole32((buf)[ 1]);		\
	(buf)[ 2] = htole32((buf)[ 2]);		\
	(buf)[ 3] = htole32((buf)[ 3]);		\
	(buf)[ 4] = htole32((buf)[ 4]);		\
	(buf)[ 5] = htole32((buf)[ 5]);		\
	(buf)[ 6] = htole32((buf)[ 6]);		\
	(buf)[ 7] = htole32((buf)[ 7]);		\
	(buf)[ 8] = htole32((buf)[ 8]);		\
	(buf)[ 9] = htole32((buf)[ 9]);		\
	(buf)[10] = htole32((buf)[10]);		\
	(buf)[11] = htole32((buf)[11]);		\
	(buf)[12] = htole32((buf)[12]);		\
	(buf)[13] = htole32((buf)[13]);		\
	(buf)[14] = htole32((buf)[14]);		\
	(buf)[15] = htole32((buf)[15]);		\
} while (0)

#endif

/*
 * Start MD4 accumulation.
 * Set bit count to 0 and buffer to mysterious initialization constants.
 */
void MD4Init(MD4_CTX *ctx)
{
	ctx->count[0] = 0;
	ctx->count[1] = 0;
	ctx->state[0] = 0x67452301;
	ctx->state[1] = 0xefcdab89;
	ctx->state[2] = 0x98badcfe;
	ctx->state[3] = 0x10325476;
}


/*
 * Update context to reflect the concatenation of another buffer full of bytes.
 */
void MD4Update(MD4_CTX *ctx, const unsigned char *buf, size_t len)
{
	unsigned int count;

	/* Bytes already stored in ctx->buffer */
	count = (unsigned int)((ctx->count[0] >> 3) & 0x3f);

	/* Update bitcount */
/*	ctx->count += (unsigned long long)len << 3;*/
	if ((ctx->count[0] += ((unsigned int)len << 3)) < (unsigned int)len) 
	{
		/* Overflowed ctx->count[0] */
		ctx->count[1]++;
	}
	ctx->count[1] += ((unsigned int)len >> 29);

	/* Handle any leading odd-sized chunks */
	if (count) 
	{
		unsigned char *p = (unsigned char *)ctx->buffer + count;

		count = MD4_BLOCK_LENGTH - count;
		if (len < count) {
			memcpy(p, buf, len);
			return;
		}
		memcpy(p, buf, count);
		htole32_16((unsigned int *)ctx->buffer);
		MD4Transform(ctx->state, ctx->buffer);
		buf += count;
		len -= count;
	}

	/* Process data in MD4_BLOCK_LENGTH-byte chunks */
	while (len >= MD4_BLOCK_LENGTH) 
	{
		memcpy(ctx->buffer, buf, MD4_BLOCK_LENGTH);
		htole32_16((unsigned int *)ctx->buffer);
		MD4Transform(ctx->state, ctx->buffer);
		buf += MD4_BLOCK_LENGTH;
		len -= MD4_BLOCK_LENGTH;
	}

	/* Handle any remaining bytes of data. */
	memcpy(ctx->buffer, buf, len);
}


/*
 * Final wrapup - pad to 64-byte boundary with the bit pattern 
 * 1 0* (64-bit count of bits processed, MSB-first)
 */
void MD4Final(unsigned char digest[MD4_DIGEST_LENGTH], MD4_CTX *ctx)
{
	unsigned int count;
	unsigned char *p;

	/* number of bytes mod 64 */
	count = (unsigned int)(ctx->count[0] >> 3) & 0x3f;

	/*
	 * Set the first char of padding to 0x80.
	 * This is safe since there is always at least one byte free.
	 */
	p = ctx->buffer + count;
	*p++ = 0x80;

	/* Bytes of padding needed to make 64 bytes */
	count = 64 - 1 - count;

	/* Pad out to 56 mod 64 */
	if (count < 8) {
		/* Two lots of padding:  Pad the first block to 64 bytes */
		memset(p, 0, count);
		htole32_16((unsigned int *)ctx->buffer);
		MD4Transform(ctx->state, ctx->buffer);

		/* Now fill the next block with 56 bytes */
		memset(ctx->buffer, 0, 56);
	} 
	else {
		/* Pad block to 56 bytes */
		memset(p, 0, count - 8);
	}
	htole32_14((unsigned int *)ctx->buffer);

	/* Append bit count and transform */
	((unsigned int *)ctx->buffer)[14] = ctx->count[0];
	((unsigned int *)ctx->buffer)[15] = ctx->count[1];

	MD4Transform(ctx->state, ctx->buffer);
	htole32_4(ctx->state);
	memcpy(digest, ctx->state, MD4_DIGEST_LENGTH);
	memset(ctx, 0, sizeof(*ctx));	/* in case it's sensitive */
}



/* The three core functions - F1 is optimized somewhat */

/* #define F1(x, y, z) (x & y | ~x & z) */
#define MD4_F1(x, y, z) (z ^ (x & (y ^ z)))
#define MD4_F2(x, y, z) ((x & y) | (x & z) | (y & z))
#define MD4_F3(x, y, z) (x ^ y ^ z)

/* This is the central step in the MD4 algorithm. */
#define MD4STEP(f, w, x, y, z, data, s) \
	( w += f(x, y, z) + data,  w = w<<s | w>>(32-s) )

/*
 * The core of the MD4 algorithm, this alters an existing MD4 hash to
 * reflect the addition of 16 longwords of new data.  MD4Update blocks
 * the data and converts bytes into longwords for this routine.
 */
void MD4Transform(unsigned int buf[4], const unsigned char inc[MD4_BLOCK_LENGTH])
{
	unsigned int a, b, c, d;
	const unsigned int *in = (const unsigned int *)inc;

	a = buf[0];
	b = buf[1];
	c = buf[2];
	d = buf[3];

	MD4STEP(MD4_F1, a, b, c, d, in[ 0],  3);
	MD4STEP(MD4_F1, d, a, b, c, in[ 1],  7);
	MD4STEP(MD4_F1, c, d, a, b, in[ 2], 11);
	MD4STEP(MD4_F1, b, c, d, a, in[ 3], 19);
	MD4STEP(MD4_F1, a, b, c, d, in[ 4],  3);
	MD4STEP(MD4_F1, d, a, b, c, in[ 5],  7);
	MD4STEP(MD4_F1, c, d, a, b, in[ 6], 11);
	MD4STEP(MD4_F1, b, c, d, a, in[ 7], 19);
	MD4STEP(MD4_F1, a, b, c, d, in[ 8],  3);
	MD4STEP(MD4_F1, d, a, b, c, in[ 9],  7);
	MD4STEP(MD4_F1, c, d, a, b, in[10], 11);
	MD4STEP(MD4_F1, b, c, d, a, in[11], 19);
	MD4STEP(MD4_F1, a, b, c, d, in[12],  3);
	MD4STEP(MD4_F1, d, a, b, c, in[13],  7);
	MD4STEP(MD4_F1, c, d, a, b, in[14], 11);
	MD4STEP(MD4_F1, b, c, d, a, in[15], 19);

	MD4STEP(MD4_F2, a, b, c, d, in[ 0] + 0x5a827999,  3);
	MD4STEP(MD4_F2, d, a, b, c, in[ 4] + 0x5a827999,  5);
	MD4STEP(MD4_F2, c, d, a, b, in[ 8] + 0x5a827999,  9);
	MD4STEP(MD4_F2, b, c, d, a, in[12] + 0x5a827999, 13);
	MD4STEP(MD4_F2, a, b, c, d, in[ 1] + 0x5a827999,  3);
	MD4STEP(MD4_F2, d, a, b, c, in[ 5] + 0x5a827999,  5);
	MD4STEP(MD4_F2, c, d, a, b, in[ 9] + 0x5a827999,  9);
	MD4STEP(MD4_F2, b, c, d, a, in[13] + 0x5a827999, 13);
	MD4STEP(MD4_F2, a, b, c, d, in[ 2] + 0x5a827999,  3);
	MD4STEP(MD4_F2, d, a, b, c, in[ 6] + 0x5a827999,  5);
	MD4STEP(MD4_F2, c, d, a, b, in[10] + 0x5a827999,  9);
	MD4STEP(MD4_F2, b, c, d, a, in[14] + 0x5a827999, 13);
	MD4STEP(MD4_F2, a, b, c, d, in[ 3] + 0x5a827999,  3);
	MD4STEP(MD4_F2, d, a, b, c, in[ 7] + 0x5a827999,  5);
	MD4STEP(MD4_F2, c, d, a, b, in[11] + 0x5a827999,  9);
	MD4STEP(MD4_F2, b, c, d, a, in[15] + 0x5a827999, 13);

	MD4STEP(MD4_F3, a, b, c, d, in[ 0] + 0x6ed9eba1,  3);
	MD4STEP(MD4_F3, d, a, b, c, in[ 8] + 0x6ed9eba1,  9);
	MD4STEP(MD4_F3, c, d, a, b, in[ 4] + 0x6ed9eba1, 11);
	MD4STEP(MD4_F3, b, c, d, a, in[12] + 0x6ed9eba1, 15);
	MD4STEP(MD4_F3, a, b, c, d, in[ 2] + 0x6ed9eba1,  3);
	MD4STEP(MD4_F3, d, a, b, c, in[10] + 0x6ed9eba1,  9);
	MD4STEP(MD4_F3, c, d, a, b, in[ 6] + 0x6ed9eba1, 11);
	MD4STEP(MD4_F3, b, c, d, a, in[14] + 0x6ed9eba1, 15);
	MD4STEP(MD4_F3, a, b, c, d, in[ 1] + 0x6ed9eba1,  3);
	MD4STEP(MD4_F3, d, a, b, c, in[ 9] + 0x6ed9eba1,  9);
	MD4STEP(MD4_F3, c, d, a, b, in[ 5] + 0x6ed9eba1, 11);
	MD4STEP(MD4_F3, b, c, d, a, in[13] + 0x6ed9eba1, 15);
	MD4STEP(MD4_F3, a, b, c, d, in[ 3] + 0x6ed9eba1,  3);
	MD4STEP(MD4_F3, d, a, b, c, in[11] + 0x6ed9eba1,  9);
	MD4STEP(MD4_F3, c, d, a, b, in[ 7] + 0x6ed9eba1, 11);
	MD4STEP(MD4_F3, b, c, d, a, in[15] + 0x6ed9eba1, 15);

	buf[0] += a;
	buf[1] += b;
	buf[2] += c;
	buf[3] += d;
}


static void MD4Print (unsigned char *MD4_digest)
{
  int i;

  for (i = 0; i < MD4_DIGEST_LENGTH; i++)
  {
    printf ("%02x", MD4_digest[i]);
	if(outfile) fprintf(outfile, "%02x", MD4_digest[i]);
  }
}

#ifdef EXAMPLE___
void md4_calc(unsigned char *output, const unsigned char *input, unsigned int inlen)
{
	MD4_CTX	context;

	MD4Init(&context);
	MD4Update(&context, input, inlen);
	MD4Final(output, &context);
}
#endif
#endif /// MD4_CHECKSUM_ENCIPHER


#ifdef MD5_CHECKSUM_ENCIPHER
/*
 **********************************************************************
 ** md5.h -- Header file for implementation of MD5                   **
 ** RSA Data Security, Inc. MD5 Message Digest Algorithm             **
 ** Created: 2/17/90 RLR                                             **
 ** Revised: 12/27/90 SRD,AJ,BSK,JT Reference C version              **
 ** Revised (for MD5): RLR 4/27/91                                   **
 **   -- G modified to have y&~z instead of y&z                      **
 **   -- FF, GG, HH modified to add in last register done            **
 **   -- Access pattern: round 2 works mod 5, round 3 works mod 3    **
 **   -- distinct additive constant for each step                    **
 **   -- round 4 added, working mod 7                                **
 **********************************************************************
 */

/*
 **********************************************************************
 ** Copyright (C) 1990, RSA Data Security, Inc. All rights reserved. **
 **                                                                  **
 ** License to copy and use this software is granted provided that   **
 ** it is identified as the "RSA Data Security, Inc. MD5 Message     **
 ** Digest Algorithm" in all material mentioning or referencing this **
 ** software or this function.                                       **
 **                                                                  **
 ** License is also granted to make and use derivative works         **
 ** provided that such works are identified as "derived from the RSA **
 ** Data Security, Inc. MD5 Message Digest Algorithm" in all         **
 ** material mentioning or referencing the derived work.             **
 **                                                                  **
 ** RSA Data Security, Inc. makes no representations concerning      **
 ** either the merchantability of this software or the suitability   **
 ** of this software for any particular purpose.  It is provided "as **
 ** is" without express or implied warranty of any kind.             **
 **                                                                  **
 ** These notices must be retained in any copies of any part of this **
 ** documentation and/or software.                                   **
 **********************************************************************
 */


/* Data structure for MD5 (Message Digest) computation */
typedef struct {
  UINT4 i[2];                   /* number of _bits_ handled mod 2^64 */
  UINT4 buf[4];                                    /* scratch buffer */
  unsigned char in[64];                              /* input buffer */
  unsigned char digest[16];     /* actual digest after MD5Final call */
} MD5_CTX;



/*
 **********************************************************************
 ** Copyright (C) 1990, RSA Data Security, Inc. All rights reserved. **
 **                                                                  **
 ** License to copy and use this software is granted provided that   **
 ** it is identified as the "RSA Data Security, Inc. MD5 Message     **
 ** Digest Algorithm" in all material mentioning or referencing this **
 ** software or this function.                                       **
 **                                                                  **
 ** License is also granted to make and use derivative works         **
 ** provided that such works are identified as "derived from the RSA **
 ** Data Security, Inc. MD5 Message Digest Algorithm" in all         **
 ** material mentioning or referencing the derived work.             **
 **                                                                  **
 ** RSA Data Security, Inc. makes no representations concerning      **
 ** either the merchantability of this software or the suitability   **
 ** of this software for any particular purpose.  It is provided "as **
 ** is" without express or implied warranty of any kind.             **
 **                                                                  **
 ** These notices must be retained in any copies of any part of this **
 ** documentation and/or software.                                   **
 **********************************************************************
 */

void MD5Init ();
void MD5Update ();
void MD5Final ();

/* forward declaration */
static void MD5_Transform (UINT4 *buf, UINT4 *in);

static unsigned char PADDING[64] = {
  0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

/* F, G and H are basic MD5 functions: selection, majority, parity */
#define MD5_F(x, y, z) (((x) & (y)) | ((~x) & (z)))
#define MD5_G(x, y, z) (((x) & (z)) | ((y) & (~z)))
#define MD5_H(x, y, z) ((x) ^ (y) ^ (z))
#define MD5_I(x, y, z) ((y) ^ ((x) | (~z))) 


/* ROTATE_LEFT rotates x left n bits */
#define ROTATE_LEFT(x, n) (((x) << (n)) | ((x) >> (32-(n))))

/* FF, GG, HH, and II transformations for rounds 1, 2, 3, and 4 */
/* Rotation is separate from addition to prevent recomputation */
#define FF(a, b, c, d, x, s, ac) \
  {(a) += MD5_F ((b), (c), (d)) + (x) + (UINT4)(ac); \
   (a) = ROTATE_LEFT ((a), (s)); \
   (a) += (b); \
  }
#define GG(a, b, c, d, x, s, ac) \
  {(a) += MD5_G ((b), (c), (d)) + (x) + (UINT4)(ac); \
   (a) = ROTATE_LEFT ((a), (s)); \
   (a) += (b); \
  }
#define HH(a, b, c, d, x, s, ac) \
  {(a) += MD5_H ((b), (c), (d)) + (x) + (UINT4)(ac); \
   (a) = ROTATE_LEFT ((a), (s)); \
   (a) += (b); \
  }
#define II(a, b, c, d, x, s, ac) \
  {(a) += MD5_I ((b), (c), (d)) + (x) + (UINT4)(ac); \
   (a) = ROTATE_LEFT ((a), (s)); \
   (a) += (b); \
  }

void MD5Init (MD5_CTX *mdContext)
{
  mdContext->i[0] = mdContext->i[1] = (UINT4)0;

  /* Load magic initialization constants.
   */
  mdContext->buf[0] = (UINT4)0x67452301UL;
  mdContext->buf[1] = (UINT4)0xefcdab89UL;
  mdContext->buf[2] = (UINT4)0x98badcfeUL;
  mdContext->buf[3] = (UINT4)0x10325476UL;
}

void MD5Update (MD5_CTX *mdContext, unsigned char *inBuf, unsigned int inLen)
{
  UINT4 in[16];
  int mdi;
  unsigned int i, ii;

  /* compute number of bytes mod 64 */
  mdi = (int)((mdContext->i[0] >> 3) & 0x3F);

  /* update number of bits */
  if ((mdContext->i[0] + ((UINT4)inLen << 3)) < mdContext->i[0])
    mdContext->i[1]++;
  mdContext->i[0] += ((UINT4)inLen << 3);
  mdContext->i[1] += ((UINT4)inLen >> 29);

  while (inLen--) {
    /* add new character to buffer, increment mdi */
    mdContext->in[mdi++] = *inBuf++;

    /* transform if necessary */
    if (mdi == 0x40) {
      for (i = 0, ii = 0; i < 16; i++, ii += 4)
        in[i] = (((UINT4)mdContext->in[ii+3]) << 24) |
                (((UINT4)mdContext->in[ii+2]) << 16) |
                (((UINT4)mdContext->in[ii+1]) << 8) |
                ((UINT4)mdContext->in[ii]);
      MD5_Transform (mdContext->buf, in);
      mdi = 0;
    }
  }
}

void MD5Final (MD5_CTX *mdContext)
{
  UINT4 in[16];
  int mdi;
  unsigned int i, ii;
  unsigned int padLen;

  /* save number of bits */
  in[14] = mdContext->i[0];
  in[15] = mdContext->i[1];

  /* compute number of bytes mod 64 */
  mdi = (int)((mdContext->i[0] >> 3) & 0x3F);

  /* pad out to 56 mod 64 */
  padLen = (mdi < 56) ? (56 - mdi) : (120 - mdi);
  MD5Update (mdContext, PADDING, padLen);

  /* append length in bits and transform */
  for (i = 0, ii = 0; i < 14; i++, ii += 4)
    in[i] = (((UINT4)mdContext->in[ii+3]) << 24) |
            (((UINT4)mdContext->in[ii+2]) << 16) |
            (((UINT4)mdContext->in[ii+1]) << 8) |
            ((UINT4)mdContext->in[ii]);
  MD5_Transform (mdContext->buf, in);

  /* store buffer in digest */
  for (i = 0, ii = 0; i < 4; i++, ii += 4) {
    mdContext->digest[ii] = (unsigned char)(mdContext->buf[i] & 0xFF);
    mdContext->digest[ii+1] =
      (unsigned char)((mdContext->buf[i] >> 8) & 0xFF);
    mdContext->digest[ii+2] =
      (unsigned char)((mdContext->buf[i] >> 16) & 0xFF);
    mdContext->digest[ii+3] =
      (unsigned char)((mdContext->buf[i] >> 24) & 0xFF);
  }
}


/* Basic MD5 step. MD5_Transform buf based on in.
 */
static void MD5_Transform (UINT4 *buf, UINT4 *in)
{
  UINT4 a = buf[0], b = buf[1], c = buf[2], d = buf[3];

  /* Round 1 */
#define S11 7
#define S12 12
#define S13 17
#define S14 22
  FF ( a, b, c, d, in[ 0], S11, 3614090360UL); /* 1 */
  FF ( d, a, b, c, in[ 1], S12, 3905402710UL); /* 2 */
  FF ( c, d, a, b, in[ 2], S13,  606105819UL); /* 3 */
  FF ( b, c, d, a, in[ 3], S14, 3250441966UL); /* 4 */
  FF ( a, b, c, d, in[ 4], S11, 4118548399UL); /* 5 */
  FF ( d, a, b, c, in[ 5], S12, 1200080426UL); /* 6 */
  FF ( c, d, a, b, in[ 6], S13, 2821735955UL); /* 7 */
  FF ( b, c, d, a, in[ 7], S14, 4249261313UL); /* 8 */
  FF ( a, b, c, d, in[ 8], S11, 1770035416UL); /* 9 */
  FF ( d, a, b, c, in[ 9], S12, 2336552879UL); /* 10 */
  FF ( c, d, a, b, in[10], S13, 4294925233UL); /* 11 */
  FF ( b, c, d, a, in[11], S14, 2304563134UL); /* 12 */
  FF ( a, b, c, d, in[12], S11, 1804603682UL); /* 13 */
  FF ( d, a, b, c, in[13], S12, 4254626195UL); /* 14 */
  FF ( c, d, a, b, in[14], S13, 2792965006UL); /* 15 */
  FF ( b, c, d, a, in[15], S14, 1236535329UL); /* 16 */

  /* Round 2 */
#define S21 5
#define S22 9
#define S23 14
#define S24 20
  GG ( a, b, c, d, in[ 1], S21, 4129170786UL); /* 17 */
  GG ( d, a, b, c, in[ 6], S22, 3225465664UL); /* 18 */
  GG ( c, d, a, b, in[11], S23,  643717713UL); /* 19 */
  GG ( b, c, d, a, in[ 0], S24, 3921069994UL); /* 20 */
  GG ( a, b, c, d, in[ 5], S21, 3593408605UL); /* 21 */
  GG ( d, a, b, c, in[10], S22,   38016083UL); /* 22 */
  GG ( c, d, a, b, in[15], S23, 3634488961UL); /* 23 */
  GG ( b, c, d, a, in[ 4], S24, 3889429448UL); /* 24 */
  GG ( a, b, c, d, in[ 9], S21,  568446438UL); /* 25 */
  GG ( d, a, b, c, in[14], S22, 3275163606UL); /* 26 */
  GG ( c, d, a, b, in[ 3], S23, 4107603335UL); /* 27 */
  GG ( b, c, d, a, in[ 8], S24, 1163531501UL); /* 28 */
  GG ( a, b, c, d, in[13], S21, 2850285829UL); /* 29 */
  GG ( d, a, b, c, in[ 2], S22, 4243563512UL); /* 30 */
  GG ( c, d, a, b, in[ 7], S23, 1735328473UL); /* 31 */
  GG ( b, c, d, a, in[12], S24, 2368359562UL); /* 32 */

  /* Round 3 */
#define S31 4
#define S32 11
#define S33 16
#define S34 23
  HH ( a, b, c, d, in[ 5], S31, 4294588738UL); /* 33 */
  HH ( d, a, b, c, in[ 8], S32, 2272392833UL); /* 34 */
  HH ( c, d, a, b, in[11], S33, 1839030562UL); /* 35 */
  HH ( b, c, d, a, in[14], S34, 4259657740UL); /* 36 */
  HH ( a, b, c, d, in[ 1], S31, 2763975236UL); /* 37 */
  HH ( d, a, b, c, in[ 4], S32, 1272893353UL); /* 38 */
  HH ( c, d, a, b, in[ 7], S33, 4139469664UL); /* 39 */
  HH ( b, c, d, a, in[10], S34, 3200236656UL); /* 40 */
  HH ( a, b, c, d, in[13], S31,  681279174UL); /* 41 */
  HH ( d, a, b, c, in[ 0], S32, 3936430074UL); /* 42 */
  HH ( c, d, a, b, in[ 3], S33, 3572445317UL); /* 43 */
  HH ( b, c, d, a, in[ 6], S34,   76029189UL); /* 44 */
  HH ( a, b, c, d, in[ 9], S31, 3654602809UL); /* 45 */
  HH ( d, a, b, c, in[12], S32, 3873151461UL); /* 46 */
  HH ( c, d, a, b, in[15], S33,  530742520UL); /* 47 */
  HH ( b, c, d, a, in[ 2], S34, 3299628645UL); /* 48 */

  /* Round 4 */
#define S41 6
#define S42 10
#define S43 15
#define S44 21
  II ( a, b, c, d, in[ 0], S41, 4096336452UL); /* 49 */
  II ( d, a, b, c, in[ 7], S42, 1126891415UL); /* 50 */
  II ( c, d, a, b, in[14], S43, 2878612391UL); /* 51 */
  II ( b, c, d, a, in[ 5], S44, 4237533241UL); /* 52 */
  II ( a, b, c, d, in[12], S41, 1700485571UL); /* 53 */
  II ( d, a, b, c, in[ 3], S42, 2399980690UL); /* 54 */
  II ( c, d, a, b, in[10], S43, 4293915773UL); /* 55 */
  II ( b, c, d, a, in[ 1], S44, 2240044497UL); /* 56 */
  II ( a, b, c, d, in[ 8], S41, 1873313359UL); /* 57 */
  II ( d, a, b, c, in[15], S42, 4264355552UL); /* 58 */
  II ( c, d, a, b, in[ 6], S43, 2734768916UL); /* 59 */
  II ( b, c, d, a, in[13], S44, 1309151649UL); /* 60 */
  II ( a, b, c, d, in[ 4], S41, 4149444226UL); /* 61 */
  II ( d, a, b, c, in[11], S42, 3174756917UL); /* 62 */
  II ( c, d, a, b, in[ 2], S43,  718787259UL); /* 63 */
  II ( b, c, d, a, in[ 9], S44, 3951481745UL); /* 64 */

  buf[0] += a;
  buf[1] += b;
  buf[2] += c;
  buf[3] += d;
}


/* Prints message digest buffer in mdContext as 32 hexadecimal digits.
   Order is from low-order byte to high-order byte of digest.
   Each byte is printed with high-order hexadecimal digit first.
 */
static void MDPrint (MD5_CTX *mdContext)
{
  int i;

  for (i = 0; i < 16; i++)
  {
    printf ("%02x", mdContext->digest[i]);
	if(outfile) fprintf(outfile, "%02x", mdContext->digest[i]);
  }
}



/*
 **********************************************************************
 ** End of md5.c                                                     **
 ******************************* (cut) ********************************
 */

#endif /// MD5_CHECKSUM_ENCIPHER






#if MD6_CHECKSUM_ENCIPHER

/* GCC */
/// #include <stdint.h>
inline unsigned long long ticks() 
{
	/* read timestamp counter */
	unsigned int lo, hi;
	asm volatile (
		"xorl %%eax,%%eax \n        cpuid"
		::: "%rax", "%rbx", "%rcx", "%rdx");
	asm volatile ("rdtsc" : "=a" (lo), "=d" (hi));
	return (unsigned long long)hi << 32 | lo;
} 


#define   md6_w    64

/* Define "md6_word" appropriately for given value of md6_w.
** Also define PR_MD6_WORD to be the appropriate hex format string,
** using the format strings from inttypes.h .
** The term `word' in comments means an `md6_word'.
*/

#if (md6_w==64)                    /* standard md6 */
typedef unsigned long long md6_word;
#define PR_MD6_WORD "%.16" PRIx64
#endif


/* MD6 compression function constants  */
#define md6_n      89    /* size of compression input block, in words  */
#define md6_c      16    /* size of compression output, in words       */
                         /* a c-word block is also called a "chunk"    */
#define md6_max_r 255    /* max allowable value for number r of rounds */

/* Compression function routines                                
** These are ``internal'' routines that need not be called for  
** ordinary md6 usage.
*/


typedef unsigned long long md6_control_word;                      /* (r,L,z,p,d) */
typedef unsigned long long md6_nodeID; /* (ell,i) */


/* MD6 mode of operation.
** MD6 mode of operation is defined in file md6_mode.c 
*/

/* MD6 constants related to standard mode of operation                 */
/* These five values give lengths of the components of compression     */
/* input block; they should sum to md6_n.                              */

#define md6_q 15         /* # Q words in compression block (>=0)       */
#define md6_k  8         /* # key words per compression block (>=0)    */
#define md6_u (64/md6_w) /* # words for unique node ID (0 or 64/w)     */
#define md6_v (64/md6_w) /* # words for control word (0 or 64/w)       */
#define md6_b 64         /* # data words per compression block (>0)    */

#define md6_default_L 64    /* large so that MD6 is fully hierarchical */

#define md6_max_stack_height 29
    /* max_stack_height determines the maximum number of bits that
    ** can be processed by this implementation (with default L) to be:
    **    (b*w) * ((b/c) ** (max_stack_height-3)
    **    = 2 ** 64  for b = 64, w = 64, c = 16, and  max_stack_height = 29
    ** (We lose three off the height since level 0 is unused,
    ** level 1 contains the input data, and C has 0-origin indexing.)
    ** The smallest workable value for md6_max_stack_height is 3.
    ** (To avoid stack overflow for non-default L values, 
    ** we should have max_stack_height >= L + 2.)
    ** (One level of storage could be saved by letting st->N[] use
    ** 1-origin indexing, since st->N[0] is now unused.)
    */

/* MD6 state.
** 
** md6_state is the main data structure for the MD6 hash function.
*/

typedef struct {

  int d;           /* desired hash bit length. 1 <= d <= 512.      */
  int hashbitlen;  /* hashbitlen is the same as d; for NIST API    */

  unsigned char hashval[ md6_c*(md6_w/8) ];
      /* e.g. unsigned char hashval[128]                           */
      /* contains hashval after call to md6_final                  */
      /* hashval appears in first floor(d/8) bytes, with           */
      /* remaining (d mod 8) bits (if any) appearing in            */
      /* high-order bit positions of hashval[1+floor(d/8)].        */

  unsigned char hexhashval[(md6_c*(md6_w/8))+1];
      /* e.g. unsigned char hexhashval[129];                       */
      /* zero-terminated string representing hex value of hashval  */

  int initialized;         /* zero, then one after md6_init called */
  unsigned long long bits_processed;                /* bits processed so far */
  unsigned long long compression_calls;    /* compression function calls made*/
  int finalized;          /* zero, then one after md6_final called */

  md6_word K[ md6_k ];  
      /* k-word (8 word) key (aka "salt") for this instance of md6 */
  int keylen;
      /* number of bytes in key K. 0<=keylen<=k*(w/8)              */

  int L;
      /* md6 mode specification parameter. 0 <= L <= 255           */
      /* L == 0 means purely sequential (Merkle-Damgaard)          */
      /* L >= 29 means purely tree-based                           */
      /* Default is md6_default_L = 64 (hierarchical)              */

  int r;
      /* Number of rounds. 0 <= r <= 255                           */

  int top;
      /* index of block corresponding to top of stack              */

  md6_word B[ md6_max_stack_height ][ md6_b ];
      /* md6_word B[29][64]                                        */
      /* stack of 29 64-word partial blocks waiting to be          */
      /* completed and compressed.                                 */
      /* B[1] is for compressing text data (input);                */
      /* B[ell] corresponds to node at level ell in the tree.      */

  unsigned int bits[ md6_max_stack_height ];    
      /* bits[ell] =                                               */
      /*    number of bits already placed in B[ell]                */
      /*    for 1 <= ell < max_stack_height                        */
      /* 0 <= bits[ell] <= b*w                                     */

  unsigned long long i_for_level[ md6_max_stack_height ];
      /* i_for_level[ell] =                                        */
      /*    index of the node B[ ell ] on this level (0,1,...)     */
      /* when it is output   */

} md6_state;



/* MD6 main interface routines
** These routines are defined in md6_mode.c
*/



/* MD6 return codes.
**
** The interface routines defined in md6_mode.c always return a
** "return code": an integer giving the status of the call.
** The codes
*/

/* SUCCESS:  */
#define MD6_SUCCESS 		0

/* ERROR CODES: */
#define MD6_FAIL 1           /* some other problem                     */
#define MD6_BADHASHLEN 2     /* hashbitlen<1 or >512 bits              */
#define MD6_NULLSTATE 3      /* null state passed to MD6               */
#define MD6_BADKEYLEN 4      /* key length is <0 or >512 bits          */
#define MD6_STATENOTINIT 5   /* state was never initialized            */
#define MD6_STACKUNDERFLOW 6 /* MD6 stack underflows (shouldn't happen)*/
#define MD6_STACKOVERFLOW 7  /* MD6 stack overflow (message too long)  */
#define MD6_NULLDATA 8       /* null data pointer                      */
#define MD6_NULL_N 9         /* compress: N is null                    */
#define MD6_NULL_B 10        /* standard compress: null B pointer      */
#define MD6_BAD_ELL 11       /* standard compress: ell not in {0,255}  */
#define MD6_BAD_p 12         /* standard compress: p<0 or p>b*w        */
#define MD6_NULL_K 13        /* standard compress: K is null           */
#define MD6_NULL_Q 14        /* standard compress: Q is null           */
#define MD6_NULL_C 15        /* standard compress: C is null           */
#define MD6_BAD_L 16         /* standard compress: L <0 or > 255       */ 
                             /* md6_init: L<0 or L>255                 */
#define MD6_BAD_r 17         /* compress: r<0 or r>255                 */
                             /* md6_init: r<0 or r>255                 */
#define MD6_OUT_OF_MEMORY 18 /* compress: storage allocation failed    */


/* compression hook, if defined, points to a function that is 
** called after each compression operation.                             
**
** compression hook must be set *after* md6_init or md6_full_init 
** is called.
*/

void (* compression_hook)(md6_word *C,
			  const md6_word *Q,
			  md6_word *K,
			  int ell,
			  int i,
			  int r,
			  int L,
			  int z,
			  int p,
			  int keylen,
			  int d,
			  md6_word *N
			  );


/// md6_nist.h
typedef unsigned char BitSequence;
typedef unsigned long long DataLength;
typedef enum { SUCCESS = 0, FAIL = 1, BAD_HASHLEN = 2 } HashReturn;
typedef md6_state hashState;
HashReturn Init( hashState *state, int hashbitlen);
HashReturn Update( hashState *state, const BitSequence *data, DataLength databitlen);
HashReturn Final( hashState *state, BitSequence *hashval );
HashReturn Hash( int hashbitlen, const BitSequence *data, DataLength databitlen, BitSequence *hashval );




/* Variables defining lengths of various values */
#define   wwww   md6_w  /* # bits in a word (64) */
#define   nnnn   md6_n  /* # words in compression input (89) */
#define   cccc   md6_c  /* # words in compression output (16) */
#define   bbbb   md6_b  /* # message words per compression input block (64) */
#define   vvvv   md6_v  /* # words in control word (1) */
#define   uuuu   md6_u  /* # words in unique nodeID (1) */
#define   kkkk   md6_k  /* # key words per compression input block (8) */
#define   qqqq   md6_q  /* # Q words per compression input block (15) */


/* "Tap positions" for feedback shift-register */

#if (nnnn==89)
#define  t0   17     /* index for linear feedback */
#define  t1   18     /* index for first input to first and */
#define  t2   21     /* index for second input to first and */
#define  t3   31     /* index for first input to second and */
#define  t4   67     /* index for second input to second and */
#define  t5   89     /* last tap */
#endif

/* Loop-unrolling setup.
**
** Define macros for loop-unrolling within compression function 
** These expand to:     loop_body(right-shift,left-shift,step)      
** These are given for word sizes 64, 32, 16, and 8, although   
** only w=64 is needed for the standard MD6 definition.         
**                                                              
** Also given for each word size are the constants S0 and Smask 
** needed in the recurrence for round constants.                
*/

#if (wwww==64)                        /* for standard word size */
#define RL00 loop_body(10,11, 0)
#define RL01 loop_body( 5,24, 1)
#define RL02 loop_body(13, 9, 2)
#define RL03 loop_body(10,16, 3)
#define RL04 loop_body(11,15, 4)
#define RL05 loop_body(12, 9, 5)
#define RL06 loop_body( 2,27, 6)
#define RL07 loop_body( 7,15, 7)
#define RL08 loop_body(14, 6, 8)
#define RL09 loop_body(15, 2, 9)
#define RL10 loop_body( 7,29,10)
#define RL11 loop_body(13, 8,11)
#define RL12 loop_body(11,15,12)
#define RL13 loop_body( 7, 5,13)
#define RL14 loop_body( 6,31,14)
#define RL15 loop_body(12, 9,15)

const md6_word S0 = (md6_word)0x0123456789abcdefULL;
const md6_word Smask = (md6_word)0x7311c2812425cfa0ULL;
#endif



/* Main compression loop. */
/*
** Perform the md6 "main compression loop" on the array A.
** This is where most of the computation occurs; it is the "heart"
** of the md6 compression algorithm.
** Input:
**     A                  input array of length t+n already set up
**                        with input in the first n words.
**     r                  number of rounds to run (178); each is c steps
** Modifies:
**     A                  A[n..r*c+n-1] filled in.
*/
void md6_main_compression_loop( md6_word* A , int r )
{ md6_word x, S;
  int i,j;

  /*
  ** main computation loop for md6 compression
  */
  S = S0;
  for (j = 0, i = nnnn; j<r*cccc; j+=cccc)
  {

/* ***************************************************************** */
#define loop_body(rs,ls,step)                                       \
      x = S;                                /* feedback constant     */ \
      x ^= A[i+step-t5];                    /* end-around feedback   */ \
      x ^= A[i+step-t0];                    /* linear feedback       */ \
      x ^= ( A[i+step-t1] & A[i+step-t2] ); /* first quadratic term  */ \
      x ^= ( A[i+step-t3] & A[i+step-t4] ); /* second quadratic term */ \
      x ^= (x >> rs);                       /* right-shift           */ \
      A[i+step] = x ^ (x << ls);            /* left-shift            */   
/* ***************************************************************** */

      /*
      ** Unroll loop c=16 times. (One "round" of computation.)
      ** Shift amounts are embedded in macros RLnn.
      */
      RL00 RL01 RL02 RL03 RL04 RL05 RL06 RL07
      RL08 RL09 RL10 RL11 RL12 RL13 RL14 RL15

      /* Advance round constant S to the next round constant. */
      S = (S << 1) ^ (S >> (wwww-1)) ^ (S & Smask);
      i += 16;
  }
}


/* ``Bare'' compression routine.
**
** Compresses n-word input to c-word output.
*/
/* Assumes n-word input array N has been fully set up.
** Input:
**	 N				 input array of n w-bit words (n=89)
**	 A				 working array of a = rc+n w-bit words
**					 A is OPTIONAL, may be given as NULL 
**					 (then md6_compress allocates and uses its own A).
**	 r				 number of rounds			 
** Modifies:
**	 C				 output array of c w-bit words (c=16)
** Returns one of the following:
**	 MD6_SUCCESS (0)	
**	 MD6_NULL_N 		
**	 MD6_NULL_C 		
**	 MD6_BAD_r			
**	 MD6_OUT_OF_MEMORY	
*/

int md6_compress( md6_word *C,
		  md6_word *N,
		  int r,
		  md6_word *A
		 )
{ md6_word* A_as_given = A;

  /* check that input is sensible */
  if ( N == NULL) return MD6_NULL_N;
  if ( C == NULL) return MD6_NULL_C;
  if ( r<0 || r > md6_max_r) return MD6_BAD_r;

  if ( A == NULL) A = calloc(r*cccc+nnnn,sizeof(md6_word));
  if ( A == NULL) return MD6_OUT_OF_MEMORY;

  memcpy( A, N, nnnn*sizeof(md6_word) );    /* copy N to front of A */

  md6_main_compression_loop( A, r );          /* do all the work */

  memcpy( C, A+(r-1)*cccc+nnnn, cccc*sizeof(md6_word) ); /* output into C */

  if ( A_as_given == NULL )           /* zero and free A if nec. */
    { memset(A,0,(r*cccc+nnnn)*sizeof(md6_word)); /* contains key info */
      free(A);           
    }

  return MD6_SUCCESS;
}


/* Control words.*/
/* Construct control word V for given inputs.
** Input:
**   r = number of rounds
**   L = mode parameter (maximum tree height)
**   z = 1 iff this is final compression operation
**   p = number of pad bits in a block to be compressed
**   keylen = number of bytes in key
**   d = desired hash output length
**   Does not check inputs for validity.
** Returns:
**   V = constructed control word
*/
md6_control_word md6_make_control_word(	int r, 
					int L, 
					int z, 
					int p, 
					int keylen, 
					int d 
					)
{ md6_control_word V;
  V = ( (((md6_control_word) 0) << 60) | /* reserved, width  4 bits */
	(((md6_control_word) r) << 48) |           /* width 12 bits */
	(((md6_control_word) L) << 40) |           /* width  8 bits */
	(((md6_control_word) z) << 36) |           /* width  4 bits */
	(((md6_control_word) p) << 20) |           /* width 16 bits */
	(((md6_control_word) keylen) << 12 ) |     /* width  8 bits */
        (((md6_control_word) d)) );                /* width 12 bits */
  return V;
}


/* Node ID's. */
/* Make "unique nodeID" U based on level ell and position i 
** within level; place it at specified destination.
** Inputs:
**    dest = address of where nodeID U should be placed
**    ell = integer level number, 1 <= ell <= ...
**    i = index within level, i = 0, 1, 2,...
** Returns
**    U = constructed nodeID
*/
md6_nodeID md6_make_nodeID( int ell,                     /* level number */
			      int i    /* index (0,1,2,...) within level */
			    )
{ md6_nodeID U;
  U = ( (((md6_nodeID) ell) << 56) | 
	((md6_nodeID) i) );
  return U;
}


/* Assembling components of compression input. */
/* Pack data before compression into n-word array N. */
void md6_pack( md6_word*N,
	       const md6_word* Q,
	       md6_word* K,
	       int ell, int i,
	       int r, int L, int z, int p, int keylen, int d,
	       md6_word* B )
{ int j;
  int ni;
  md6_nodeID U;
  md6_control_word V;    

  ni = 0;

  for (j=0;j<qqqq;j++) N[ni++] = Q[j];       /* Q: Q in words     0--14 */

  for (j=0;j<kkkk;j++) N[ni++] = K[j];       /* K: key in words  15--22 */

  U = md6_make_nodeID(ell,i);             /* U: unique node ID in 23 */
  /* The following also works for variants 
  ** in which u=0.
  */
  memcpy((unsigned char *)&N[ni],
	 &U,
	 min(uuuu*(wwww/8),sizeof(md6_nodeID)));
  ni += uuuu;

  V = md6_make_control_word(
			r,L,z,p,keylen,d);/* V: control word in   24 */
  /* The following also works for variants
  ** in which v=0.
  */
  memcpy((unsigned char *)&N[ni],
	 &V,
	 min(vvvv*(wwww/8),sizeof(md6_control_word)));
  ni += vvvv;

  memcpy(N+ni,B,bbbb*sizeof(md6_word));      /* B: data words    25--88 */
}
	       

/* Standard compress: assemble components and then compress*/
/* Perform md6 block compression using all the "standard" inputs.
** Input:
**     Q              q-word (q=15) approximation to (sqrt(6)-2)
**     K              k-word key input (k=8)
**     ell            level number
**     i              index within level
**     r              number of rounds in this compression operation
**     L              mode parameter (max tree height)
**     z              1 iff this is the very last compression
**     p              number of padding bits of input in payload B
**     keylen         number of bytes in key
**     d              desired output hash bit length
**     B              b-word (64-word) data input block (with zero padding)
** Modifies:
**     C              c-word output array (c=16)
** Returns one of the following:
**   MD6_SUCCESS (0)   MD6_BAD_p
**   MD6_NULL_B        MD6_BAD_HASHLEN
**   MD6_NULL_C        MD6_NULL_K
**   MD6_BAD_r         MD6_NULL_Q
**   MD6_BAD_ELL       MD6_OUT_OF_MEMORY
*/
int md6_standard_compress( md6_word* C,
			   const md6_word* Q,
			   md6_word* K,
			   int ell, int i,
			   int r, int L, int z, int p, int keylen, int d,
			   md6_word* B 
			   )
{ md6_word N[md6_n];
  md6_word A[5000];       /* MS VS can't handle variable size here */

  /* check that input values are sensible */
  if ( (C == NULL) ) return MD6_NULL_C;
  if ( (B == NULL) ) return MD6_NULL_B;
  if ( (r<0) | (r>md6_max_r) ) return MD6_BAD_r;
  if ( (L<0) | (L>255) ) return MD6_BAD_L;
  if ( (ell < 0) || (ell > 255) ) return MD6_BAD_ELL;
  if ( (p < 0) || (p > bbbb*wwww ) ) return MD6_BAD_p;
  if ( (d <= 0) || (d > cccc*wwww/2) ) return MD6_BADHASHLEN;
  if ( (K == NULL) ) return MD6_NULL_K;
  if ( (Q == NULL) ) return MD6_NULL_Q;

  /* pack components into N for compression */
  md6_pack(N,Q,K,ell,i,r,L,z,p,keylen,d,B);

  /* call compression hook if it is defined. */
  /* -- for testing and debugging.           */
  if (compression_hook != NULL)
    compression_hook(C,Q,K,ell,i,r,L,z,p,keylen,d,B);

  return md6_compress(C,N,r,A);
}
//// ------------------------------------------------





/* Default number of rounds                                    */
/* (as a function of digest size d and keylen                  */
int md6_default_r( int d , int keylen )
{ 
  int r;
  /* Default number of rounds is forty plus floor(d/4) */
  r = 40 + (d/4);
  /* unless keylen > 0, in which case it must be >= 80 as well */
  if (keylen>0)
    r = max(80,r);
  return r;
}


/* MD6 Constant Vector Q
** Q = initial 960 bits of fractional part of sqrt(6)
**
** Given here for w = 64, 32, 16, and 8, although only
** w = 64 is needed for the standard version of MD6.
*/

#if (wwww==64) /* for standard version */
/* 15 64-bit words */
static const md6_word Q[15] =
{
    0x7311c2812425cfa0ULL,
    0x6432286434aac8e7ULL, 
    0xb60450e9ef68b7c1ULL, 
    0xe8fb23908d9f06f1ULL, 
    0xdd2e76cba691e5bfULL, 
    0x0cd0d63b2c30bc41ULL, 
    0x1f8ccf6823058f8aULL, 
    0x54e5ed5b88e3775dULL, 
    0x4ad12aae0a6d6031ULL, 
    0x3e7f16bb88222e0dULL, 
    0x8af8671d3fb50c2cULL, 
    0x995ad1178bd25c31ULL, 
    0xc878c1dd04c4b633ULL, 
    0x3b72066c7a1552acULL, 
    0x0d6f3522631effcbULL, 
};
#endif



/* Endianness. */

/* routines for dealing with byte ordering */

int md6_byte_order = 0;    
/* md6_byte_order describes the endianness of the 
** underlying machine:
** 0 = unknown
** 1 = little-endian
** 2 = big-endian
*/

/* Macros to detect machine byte order; these
** presume that md6_byte_order has been setup by
** md6_detect_byte_order()
*/
#define MD6_LITTLE_ENDIAN (md6_byte_order == 1)
#define MD6_BIG_ENDIAN    (md6_byte_order == 2)
 

/* determine if underlying machine is little-endian or big-endian
** set global variable md6_byte_order to reflect result
** Written to work for any w.
*/
void md6_detect_byte_order( void )
{ md6_word x = 1 | (((md6_word)2)<<(wwww-8));
  unsigned char *cp = (unsigned char *)&x;
  if ( *cp == 1 )        md6_byte_order = 1;      /* little-endian */
  else if ( *cp == 2 )   md6_byte_order = 2;      /* big-endian    */
  else                   md6_byte_order = 0;      /* unknown       */
}



/* return byte-reversal of md6_word x.
** Written to work for any w, w=8,16,32,64.
*/
md6_word md6_byte_reverse( md6_word x )
{ 
#define mask8  ((md6_word)0x00ff00ff00ff00ffULL)
#define mask16 ((md6_word)0x0000ffff0000ffffULL)

#if (wwww==64)
  x = (x << 32) | (x >> 32);
#endif

#if (wwww >= 32)
  x = ((x & mask16) << 16) | ((x & ~mask16) >> 16);
#endif

#if (wwww >= 16)
  x = ((x & mask8) << 8) | ((x & ~mask8) >> 8);
#endif

  return x;
}



/* Byte-reverse words x[0...count-1] if machine is little_endian */
void md6_reverse_little_endian( md6_word *x, int count )
{
  int i;
  if (MD6_LITTLE_ENDIAN)
    for (i=0;i<count;i++)
      x[i] = md6_byte_reverse(x[i]);
}


/* Appending one bit string onto another.
*/
/* Append bit string src to the end of bit string dest
** Input:
**	   dest 		a bit string of destlen bits, starting in dest[0]
**					if destlen is not a multiple of 8, the high-order
**					bits are used first
**	   src			a bit string of srclen bits, starting in src[0]
**					if srclen is not a multiple of 8, the high-order
**					bits are used first
** Modifies:
**	   dest 		when append_bits returns, dest will be modified to
**					be a bit-string of length (destlen+srclen).
**					zeros will fill any unused bit positions in the 
**					last byte.
*/
void append_bits( unsigned char *dest, unsigned int destlen, unsigned char *src,  unsigned int srclen )
{ int i, di, accumlen;
  unsigned short accum;
  int srcbytes;

  if (srclen == 0) return;

  /* Initialize accum, accumlen, and di */
  accum = 0;    /* accumulates bits waiting to be moved, right-justified */
  accumlen = 0; /* number of bits in accumulator */
  if (destlen%8 != 0)
    { accumlen = destlen%8;
      accum = dest[destlen/8];        /* grab partial byte from dest     */
      accum = accum >> (8-accumlen);  /* right-justify it in accumulator */
    }
  di = destlen/8;        /* index of where next byte will go within dest */
  
  /* Now process each byte of src */
  srcbytes = (srclen+7)/8;   /* number of bytes (full or partial) in src */
  for (i=0;i<srcbytes;i++)
    { /* shift good bits from src[i] into accum */
      if (i != srcbytes-1) /* not last byte */
	{ accum = (accum << 8) ^ src[i];  
	  accumlen += 8;
	}
      else /* last byte */
	{ int newbits = ((srclen%8 == 0) ? 8 : (srclen%8));
	  accum = (accum << newbits) | (src[i] >> (8-newbits));
	  accumlen += newbits;
	}
      /* do as many high-order bits of accum as you can (or need to) */
      while ( ( (i != srcbytes-1) & (accumlen >= 8) ) ||
	      ( (i == srcbytes-1) & (accumlen > 0) ) )
	{ int numbits = min(8,accumlen);
	  unsigned char bits;
	  bits = accum >> (accumlen - numbits);    /* right justified */
	  bits = bits << (8-numbits);              /* left justified  */
	  bits &= (0xff00 >> numbits);             /* mask            */
	  dest[di++] = bits;                       /* save            */
	  accumlen -= numbits; 
	}
    }
}


/* State initialization. (md6_full_init, with all parameters specified)
*/
/* Initialize md6_state
** Input:
**	   st		  md6_state to be initialized
**	   d		  desired hash bit length 1 <= d <= w*(c/2)    (<=512 bits)
**	   key		  key (aka salt) for this hash computation	   (byte array)
**				  defaults to all-zero key if key==NULL or keylen==0
**	   keylen	  length of key in bytes; 0 <= keylen <= (k*8) (<=64 bytes)
**	   L		  md6 mode parameter; 0 <= L <= 255
**				  md6.h defines md6_default_L for when you want default
**	   r		  number of rounds; 0 <= r <= 255
** Output:
**	   updates components of state
**	   returns one of the following:
**		 MD6_SUCCESS
**		 MD6_NULLSTATE
**		 MD6_BADKEYLEN
**		 MD6_BADHASHLEN
*/
int md6_full_init( md6_state *st,       /* uninitialized state to use */
		   int d,                          /* hash bit length */
		   unsigned char *key,        /* key; OK to give NULL */
		   int keylen,     /* keylength (bytes); OK to give 0 */
		   int L,           /* mode; OK to give md6_default_L */
		   int r                          /* number of rounds */
		   )
{ /* check that md6_full_init input parameters make some sense */
  if (st == NULL) return MD6_NULLSTATE;
  if ( (key != NULL) && ((keylen < 0) || (keylen > kkkk*(wwww/8))) )
    return MD6_BADKEYLEN;
  if ( d < 1 || d > 512 || d > wwww*cccc/2 ) return MD6_BADHASHLEN;

  md6_detect_byte_order();
  memset(st,0,sizeof(md6_state));  /* clear state to zero */
  st->d = d;                       /* save hashbitlen */
  if (key != NULL && keylen > 0)   /* if no key given, use memset zeros*/
    { memcpy(st->K,key,keylen);    /* else save key (with zeros added) */
      st->keylen = keylen;
      /* handle endian-ness */       /* first byte went into high end */
      md6_reverse_little_endian(st->K,kkkk);
    }
  else
    st->keylen = 0;
  if ( (L<0) | (L>255) ) return MD6_BAD_L;
  st->L = L;
  if ( (r<0) | (r>255) ) return MD6_BAD_r;
  st->r = r;
  st->initialized = 1;  
  st->top = 1;
  /* if SEQ mode for level 1; use IV=0  */
  /* zero bits already there by memset; */
  /* we just need to set st->bits[1]    */
  if (L==0) st->bits[1] = cccc*wwww;     
  compression_hook = NULL;     /* just to be sure default is "not set" */
  return MD6_SUCCESS;
}


/* State initialization. (md6_init, which defaults most parameters.)
*/
/* Same as md6_full_init, but with default key, L, and r */
int md6_init( md6_state *st, int d)
{ return md6_full_init(st,
		       d,
		       NULL,
		       0,
		       md6_default_L,
		       md6_default_r(d,0)
		       );
}


/* Data structure notes.
*/

/*
Here are some notes on the data structures used (inside state).

* The variable B[] is a stack of length-b (b-64) word records,
  each corresponding to a node in the tree.  B[ell] corresponds
  to a node at level ell.  Specifically, it represents the record which,
  when compressed, will yield the value at that level. (It only
  contains the data payload, not the auxiliary information.)
  Note that B[i] is used to store the *inputs* to the computation at
  level i, not the output for the node at that level.  
  Thus, for example, the message input is stored in B[1], not B[0].

* Level 0 is not used.  The message bytes are placed into B[1].

* top is the largest ell for which B[ell] has received data,
  or is equal to 1 in case no data has been received yet at all.

* top is never greater than L+1.  If B[L+1] is
  compressed, the result is put back into B[L+1]  (this is SEQ).

* bits[ell] says how many bits have been placed into
  B[ell].  An invariant maintained is that of the bits in B[ell], 
  only the first bits[ell] may be nonzero; the following bits must be zero.

* The B nodes may have somewhat different formats, depending on the level:
  -- Level 1 node contains a variable-length bit-string, and so
     0 <= bits[1] <= b*w     is all we can say.
  -- Levels 2...top always receive data in c-word chunks (from
     children), so for them bits[ell] is between 0 and b*w,
     inclusive, but is also a multiple of cw.  We can think of these
     nodes as have (b/c) (i.e. 4) "slots" for chunks.
  -- Level L+1 is special, in that the first c words of B are dedicated
     to the "chaining variable" (or IV, for the first node on the level).

* When the hashing is over, B[top] will contain the 
  final hash value, in the first or second (if top = L+1) slot.

*/

/* Compress one block -- compress data at a node (md6_compress_block).
*/
/* compress block at level ell, and put c-word result into C.
** Input:
**	   st		  current md6 computation state
**	   ell		  0 <= ell < max_stack_height-1
**	   z		  z = 1 if this is very last compression; else 0
** Output:
**	   C		  c-word array to put result in
** Modifies:
**	   st->bits[ell]  (zeroed)
**	   st->i_for_level[ell] (incremented)  
**	   st->B[ell] (zeroed)
**	   st->compression_calls (incremented)
** Returns one of the following:
**	   MD6_SUCCESS
**	   MD6_NULLSTATE
**	   MD6_STATENOTINIT
**	   MD6_STACKUNDERFLOW
**	   MD6_STACKOVERFLOW
*/

int md6_compress_block( md6_word *C, md6_state *st, int ell, int z )
{ int p, err;

  /* check that input values are sensible */
  if ( st == NULL) return MD6_NULLSTATE;
  if ( st->initialized == 0 ) return MD6_STATENOTINIT;
  if ( ell < 0 ) return MD6_STACKUNDERFLOW;
  if ( ell >= md6_max_stack_height-1 ) return MD6_STACKOVERFLOW;

  st->compression_calls++;

  if (ell==1) /* leaf; hashing data; reverse bytes if nec. */
    { if (ell<(st->L + 1)) /* PAR (tree) node */
	md6_reverse_little_endian(&(st->B[ell][0]),bbbb);
      else /* SEQ (sequential) node; don't reverse chaining vars */
	md6_reverse_little_endian(&(st->B[ell][cccc]),bbbb-cccc);
    }

  p = bbbb*wwww - st->bits[ell];          /* number of pad bits */

  err = 
    md6_standard_compress( 
      C,                                      /* C    */
      Q,                                      /* Q    */
      st->K,                                  /* K    */
      ell, st->i_for_level[ell],              /* -> U */
      st->r, st->L, z, p, st->keylen, st->d,  /* -> V */
      st->B[ell]                              /* B    */
			   );                         
  if (err) return err; 

  st->bits[ell] = 0; /* clear bits used count this level */
  st->i_for_level[ell]++;

  memset(&(st->B[ell][0]),0,bbbb*sizeof(md6_word));     /* clear B[ell] */
  return MD6_SUCCESS;
}


/* Process (compress) a node and its compressible ancestors.
*/
int md6_process( md6_state *st, int ell, int final )
/*
** Do processing of level ell (and higher, if necessary) blocks.
** 
** Input:
**     st         md6 state that has been accumulating message bits
**                and/or intermediate results
**     ell        level number of block to process
**     final      true if this routine called from md6_final 
**                     (no more input will come)
**                false if more input will be coming
**                (This is not same notion as "final bit" (i.e. z)
**                 indicating the last compression operation.)
** Output (by side effect on state):
**     Sets st->hashval to final chaining value on final compression.
** Returns one of the following:
**     MD6_SUCCESS
**     MD6_NULLSTATE
**     MD6_STATENOTINIT
*/
{ int err, z, next_level;
  md6_word C[cccc];

  /* check that input values are sensible */
  if ( st == NULL) return MD6_NULLSTATE;
  if ( st->initialized == 0 ) return MD6_STATENOTINIT;

  if (!final) /* not final -- more input will be coming */
    { /* if not final and block on this level not full, nothing to do */
      if ( st->bits[ell] < bbbb*wwww ) 
	return MD6_SUCCESS;
      /* else fall through to compress this full block, 
      **       since more input will be coming 
      */
    }
  else /* final -- no more input will be coming */
    { if ( ell == st->top )
	{ if (ell == (st->L + 1)) /* SEQ node */
	    { if ( st->bits[ell]==cccc*wwww && st->i_for_level[ell]>0 )
		return MD6_SUCCESS;
	      /* else (bits>cw or i==0, so fall thru to compress */
	    }
           else /* st->top == ell <= st->L so we are at top tree node */
	     { if ( ell>1 && st->bits[ell]==cccc*wwww)
		 return MD6_SUCCESS;
	       /* else (ell==1 or bits>cw, so fall thru to compress */
	     }
	}
      /* else (here ell < st->top so fall through to compress */
    }

  /* compress block at this level; result goes into C */
  /* first set z to 1 iff this is the very last compression */
  z = 0; if (final && (ell == st->top)) z = 1; 
  if ((err = md6_compress_block(C,st,ell,z))) 
      return err;
  if (z==1) /* save final chaining value in st->hashval */
    { memcpy( st->hashval, C, md6_c*(wwww/8) );
      return MD6_SUCCESS;
    }
  
  /* where should result go? To "next level" */
  next_level = min(ell+1,st->L+1);
  /* Start sequential mode with IV=0 at that level if necessary 
  ** (All that is needed is to set bits[next_level] to c*w, 
  ** since the bits themselves are already zeroed, either
  ** initially, or at the end of md6_compress_block.)
  */
  if (next_level == st->L + 1 
      && st->i_for_level[next_level]==0
      && st->bits[next_level]==0 )
    st->bits[next_level] = cccc*wwww;   
  /* now copy C onto next level */
  memcpy((char *)st->B[next_level] + st->bits[next_level]/8,
	 C,
	 cccc*(wwww/8));
  st->bits[next_level] += cccc*wwww;   
  if (next_level > st->top) st->top = next_level;

  return md6_process(st,next_level,final);
}


/* Update -- incorporate data string into hash computation.
*/
/* Process input byte string data, updating state to reflect result
** Input:
**	   st				already initialized state to be updated
**	   data 			byte string of length databitlen bits 
**						to be processed (aka "M")
**	   databitlen		number of bits in string data (aka "m")
** Modifies:
**	   st				updated to reflect input of data
*/
int md6_update( md6_state *st, unsigned char *data, unsigned long long databitlen )
{ 
  unsigned int j, portion_size;
  int err;

  /* check that input values are sensible */
  if ( st == NULL ) return MD6_NULLSTATE;
  if ( st->initialized == 0 ) return MD6_STATENOTINIT;
  if ( data == NULL ) return MD6_NULLDATA;
  
  j = 0; /* j = number of bits processed so far with this update */
  while (j<databitlen)
    { /* handle input string in portions (portion_size in bits)
      ** portion_size may be zero (level 1 data block might be full, 
      ** having size b*w bits) */
      portion_size = min(databitlen-j,
			 (unsigned int)(bbbb*wwww-(st->bits[1]))); 

      if ((portion_size % 8 == 0) && 
	  (st->bits[1] % 8 == 0) &&
	  (j % 8 == 0))
	{ /* use mempy to handle easy, but most common, case */
	  memcpy((char *)st->B[1] + st->bits[1]/8,
		 &(data[j/8]),                                 
		 portion_size/8);
	}
      else /* handle messy case where shifting is needed */
	{ append_bits((unsigned char *)st->B[1], /* dest */
		      st->bits[1],   /* dest current bit size */
		      &(data[j/8]),  /* src */
		      portion_size); /* src size in bits  */
	}
      j += portion_size;
      st->bits[1] += portion_size;
      st->bits_processed += portion_size;

      /* compress level-1 block if it is now full 
	 but we're not done yet */
      if (st->bits[1] == bbbb*wwww && j<databitlen)
	{ if ((err=md6_process(st,
			       1,    /* ell */
			       0     /* final */
			       ))) 
	    return err; 
	}
    } /* end of loop body handling input portion */
  return MD6_SUCCESS;
}


/* Convert hash value to hexadecimal, and store it in state.
*/
/*
** Convert hashval in st->hashval into hexadecimal, and
** save result in st->hexhashval
** This will be a zero-terminated string of length ceil(d/4).
** Assumes that hashval has already been "trimmed" to correct 
** length.
** 
** Returns one of the following:
**	  MD6_SUCCESS
**	  MD6_NULLSTATE 					 (if input state pointer was NULL)
*/
int md6_compute_hex_hashval( md6_state *st )
{ int i;
  static unsigned char hex_digits[] = "0123456789abcdef";

  /* check that input is sensible */
  if ( st == NULL ) return MD6_NULLSTATE;
  
  for (i=0;i<((st->d+7)/8);i++)
    { st->hexhashval[2*i]   
	= hex_digits[ ((st->hashval[i])>>4) & 0xf ];
      st->hexhashval[2*i+1] 
	= hex_digits[ (st->hashval[i]) & 0xf ];
    }
  
  /* insert zero string termination byte at position ceil(d/4) */
  st->hexhashval[(st->d+3)/4] = 0;
  return MD6_SUCCESS;
}


/* Extract last d bits of chaining variable as hash value.*/

void trim_hashval(md6_state *st)
{ /* trim hashval to desired length d bits by taking only last d bits */
  /* note that high-order bit of a byte is considered its *first* bit */
  int full_or_partial_bytes = (st->d+7)/8;
  int bits = st->d % 8;                 /* bits in partial byte */
  int i;

  /* move relevant bytes to the front */
  for ( i=0; i<full_or_partial_bytes; i++ )
    st->hashval[i] = st->hashval[cccc*(wwww/8)-full_or_partial_bytes+i];

  /* zero out following bytes */
  for ( i=full_or_partial_bytes; i<cccc*(wwww/8); i++ )
    st->hashval[i] = 0;

  /* shift result left by (8-bits) bit positions, per byte, if needed */
  if (bits>0)
    { for ( i=0; i<full_or_partial_bytes; i++ )
	{ st->hashval[i] = (st->hashval[i] << (8-bits));
	  if ( (i+1) < cccc*(wwww/8) )
	    st->hashval[i] |= (st->hashval[i+1] >> bits);
	}
    }
}

/* Final -- no more data; finish up and produce hash value.*/
/* Do final processing to produce md6 hash value
** Input:
**     st              md6 state that has been accumulating message bits
**                     and/or intermediate results
** Output (by side effect on state):
**     hashval         If this is non-NULL, final hash value copied here.
**                     (NULL means don't copy.)  In any case, the hash
**                     value remains in st->hashval.
**     st->hashval     this is a 64-byte array; the first st->d
**                     bits of which will be the desired hash value
**                     (with high-order bits of a byte used first), and
**                     remaining bits set to zero (same as hashval)
**     st->hexhashval  this is a 129-byte array which contains the
**                     zero-terminated hexadecimal version of the hash
** Returns one of the following:
**     MD6_SUCCESS
**     MD6_NULLSTATE
**     MD6_STATENOTINIT
*/
int md6_final( md6_state *st , unsigned char *hashval)
{ int ell, err;

  /* check that input values are sensible */
  if ( st == NULL) return MD6_NULLSTATE;
  if ( st->initialized == 0 ) return MD6_STATENOTINIT;

  /* md6_final was previously called */
  if ( st->finalized == 1 ) return MD6_SUCCESS;

  /* force any processing that needs doing */
  if (st->top == 1) ell = 1;
  else for (ell=1; ell<=st->top; ell++)
	 if (st->bits[ell]>0) break;
  /* process starting at level ell, up to root */
  err = md6_process(st,ell,1);
  if (err) return err;

  /* md6_process has saved final chaining value in st->hashval */

  md6_reverse_little_endian( (md6_word*)st->hashval, cccc );

  /* 4/15/09: Following two lines were previously out of order, which 
  **          caused errors depending on whether caller took hash output 
  **          from  st->hashval (which was correct) or from 
  **                hashval parameter (which was incorrect, since it 
  **                                   missed getting "trimmed".)
  */
  trim_hashval( st );
  if (hashval != NULL) memcpy( hashval, st->hashval, (st->d+7)/8 );

  md6_compute_hex_hashval( st );

  st->finalized = 1;
  return MD6_SUCCESS;
}

/* Routines for hashing message given "all at once".*/
int md6_full_hash( int d,                    /* hash bit length */
		   unsigned char *data,/* complete data to hash */
		   unsigned long long databitlen,   /* its length in bits */
		   unsigned char *key,       /* OK to give NULL */
		   int keylen,       /* (in bytes) OK to give 0 */
		   int L,     /* mode; OK to give md6_default_L */
		   int r,                   /* number of rounds */
		   unsigned char *hashval             /* output */
		   )
{ md6_state st;
  int err;

  err = md6_full_init(&st,d,key,keylen,L,r);
  if (err) return err;
  err = md6_update(&st,data,databitlen);
  if (err) return err;
  md6_final(&st,hashval);
  if (err) return err;
  return MD6_SUCCESS;
}


/// int d: hash bit length
/// byte *data : complete data to hash
/// unsigned long long databitlen : its length in bits
/// unsigned char *hashval : output
int md6_hash( int d, unsigned char *data, unsigned long long databitlen, unsigned char *hashval )
{ 
  int err;

  err = md6_full_hash(d,data,databitlen,
		      NULL,0,md6_default_L,md6_default_r(d,0),hashval);
  if (err) return err;
  return MD6_SUCCESS;
}

//// end of md6_mode.c -------------------------------



HashReturn Init( hashState *state, int hashbitlen)
{ int err;
  if ((err = md6_init( (md6_state *) state, 
		       hashbitlen
		       )))
    return err;
  state->hashbitlen = hashbitlen;
  return SUCCESS;
}

HashReturn Update( hashState *state, const BitSequence *data, DataLength databitlen )
{ 
	return md6_update( (md6_state *) state, 
		     (unsigned char *)data, 
		     (unsigned long long) databitlen );
}

HashReturn Final( hashState *state, BitSequence *hashval )
{ 
	return md6_final( (md6_state *) state, (unsigned char *) hashval );
}

HashReturn Hash( int hashbitlen, const BitSequence *data, DataLength databitlen, BitSequence *hashval )
{ int err;
  md6_state state;
  if ((err = Init( &state, hashbitlen ))) 
    return err;
  if ((err = Update( &state, data, databitlen ))) 
    return err;
  return Final( &state, hashval );
}


/* ------------ MD6 parameters --------------------- */
int md6_dgtLen = 256;         /* digest length */
int md6_modPar = 64;          /* mode parameter */
int md6_roundN;               /* number of rounds */
int md6_use_default_r = 1;    /* 1 if r should be set to the default, 0 if r is explicitly provided */
unsigned char md6_K[100];     /* key */
int md6_keylen = 0;           /* key length in bytes (at most 64) */
md6_state md6_st;             /* md6 computation state */

char md6_msg[5000];           /* message to be hashed (if given with -M) */
int msglenbytes;              /* message length in bytes */


/* return integer starting at s (input presumed to end with '\n')
** (It may be expressed in exponential format e.g. 1e9.)
*/
unsigned long long get_int(char *s)
{ 
	long double g;
	sscanf(s,"%Lg",&g);
	return (unsigned long long)g;
}

/* routines to escape/unescape filenames, in case they
** contain backslash or \n 's.
*/
/* input t, output s -- recode t so it all newlines and 
** backslashes are escaped as \n and \\ respectively.
** Also, a leading '-' is escaped to \-.
*/
void encode(char *s, char *t)
{ 
	if (*t && *t=='-')
	{ *s++ = '\\'; *s++ = '-'; t++; }
	while (*t)
    { if (*t=='\\')      { *s++ = '\\'; *s++ = '\\'; }     
      else if (*t=='\n') { *s++ = '\\'; *s++ = 'n';  }
      else               *s++ = *t;
      t++;
    }
	*s = 0;
	return;
}


/* inverse of encode -- s is unescaped version of t. */
void decode(char *s, char *t)
{ while (*t)
    { if (*t == '\\')
	{ if (*(t+1)=='\\')     { *s++ = '\\'; t+=1; }
	  else if (*(t+1)=='n') { *s++ = '\n'; t+=1; } 
	  else if (*(t+1)=='-') { *s++ = '-'; t+=1; }
	  else if (*(t+1)==0)   { *s++ = '\\'; }
	  else                  { *s++ = *t; }
	}
      else *s++ = *t;
      t++;
    }
  *s = 0;
  return;
}


/* timing variables and routines */
double start_time;
double end_time;
unsigned long long start_ticks;
unsigned long long end_ticks;

void start_timer()
{
#if 0
  start_time = ((double)clock())/CLOCKS_PER_SEC;
  start_ticks = ticks();
#endif
}

void end_timer()
{
#if 0
  end_time = ((double)clock())/CLOCKS_PER_SEC;
  end_ticks = ticks();
#endif
}

int print_times = 0;

void print_time()
{ 
#ifdef DEBUG_PRINT

  double elapsed_time = end_time - start_time;
  unsigned long long elapsed_ticks = end_ticks - start_ticks;
  unsigned long long bytes = md6_st.bits_processed/8;
  int bits = md6_st.bits_processed % 8;
  if (!print_times) return;
  printf("-- Length = ");
  if (md6_st.bits_processed==0) printf("0");
  if (bytes>0) printf("%g byte",(double)bytes);
  if (bytes>1) printf("s");
  if (bytes>0 && bits>0) printf(" + ");
  if (bits>0) printf("%d bit",bits);
  if (bits>1) printf("s");
  printf("\n");
  printf("-- Compression calls made = %g\n",(double)md6_st.compression_calls);
  if (elapsed_time == 0.0)
    printf("-- Elapsed time too short to measure...\n");
  else
    { printf("-- Elapsed time = %.3f seconds.\n", elapsed_time);
      printf("-- Megabytes per second = %g.\n",
	     (bytes/elapsed_time)/1000000.0);
      printf("-- Microseconds per compression function = %g.\n",
	     (elapsed_time*1.0e6 / md6_st.compression_calls ));
    }
  printf("-- Total clock ticks = %lld\n",
	 (long long int)elapsed_ticks);
  if (bytes>0)
    printf("-- Clock ticks / byte = %lld\n",
	   (long long int)(elapsed_ticks/bytes));
  printf("-- Clock ticks / compression function call = %lld\n",
	 (long long int)(elapsed_ticks/md6_st.compression_calls));

#endif
}



/* testing and debugging */

/* Global variables used by compression_hook_1 */
/// FILE *outFile = NULL;
int  print_input_output = 0;
//int  print_intermediate = 0;

void compression_hook_1(md6_word *C,
			const md6_word *Q,
			md6_word *K,
			int ell,
			int ii,
			int r,
			int L,
			int z,
			int p,
			int keylen,
			int d,
			md6_word *B
)
{ 
	md6_word A[5000];
#ifdef DEBUG_PRINT
	int i;
	time_t now;
#endif

	md6_pack(A,Q,K,ell,ii,r,L,z,p,keylen,d,B);

	md6_main_compression_loop( A, r);





#ifdef DEBUG_PRINT

	if (ell==1 && ii==0)
	{ 
		time(&now);
		printf("-- d = %6d (digest length in bits)\n",d);
		printf("-- L = %6d (number of parallel passes)\n",L);
		printf("-- r = %6d (number of rounds)\n",r);

		/* print key out as chars, since for md6sum it is essentially
		** impossible to enter non-char keys...
		*/
		printf("-- K = '");
		for (i=0;i<keylen;i++) 
			printf("%c",(int)(K[i/(wwww/8)]>>8*(7-(i%(wwww/8))))&0xff);
		printf("' (key)\n");
		printf("-- k = %6d (key length in bytes)\n",keylen);
		printf("\n");
	}

  printf("MD6 compression function computation ");
  printf("(level %d, index %d):\n",ell,ii);
  printf("Input (%d words):\n",nnnn);


  for (i=0;i<r*cccc+nnnn;i++)
  {
	if ((i<qqqq))
	{ 
	  printf("A[%4d] = " PR_MD6_WORD,i,A[i]);
	  printf(" Q[%d]\n",i);
	}
	else if ((i>=qqqq)&&(i<qqqq+kkkk))
	{ 
	  printf("A[%4d] = " PR_MD6_WORD,i,A[i]);
	  printf(" key K[%d]\n",i-qqqq);
	}
	else if ((uuuu>0)&&(i==qqqq+kkkk+uuuu-1))
	{ 
	  printf("A[%4d] = " PR_MD6_WORD,i,A[i]);
	  printf(" nodeID U = (ell,i) = (%d,%d)\n",ell,ii);
	}
	else if ((vvvv>0)&&(i==qqqq+kkkk+uuuu+vvvv-1))
	{ 
	  printf("A[%4d] = " PR_MD6_WORD,i,A[i]);
	  printf(" control word V = "
				  "(r,L,z,p,keylen,d) = "
		  "(%d,%d,%d,%d,%d,%d)\n",r,L,z,p,keylen,d);
	}
	else if ((i>=qqqq+kkkk+uuuu+vvvv)&&(i<nnnn))
	{ 
	  printf("A[%4d] = " PR_MD6_WORD,i,A[i]);
	  printf(" data B[%2d] ",i-qqqq-kkkk-uuuu-vvvv);
	  if (ell < L+1) /* PAR node */
		{ if (ell == 1)
		{ if ( (i+(p/wwww))<nnnn )
			printf("input message word %4d",
				ii*bbbb+(i-(qqqq+kkkk+uuuu+vvvv)));
		  else
			printf("padding");
		}
		  else
		if ( (i+(p/wwww))< nnnn )
		  printf(
			  "chaining from (%d,%d)",
			  ell-1,
			  4*ii+(i-(qqqq+kkkk+uuuu+vvvv))/cccc);
		else 
		  printf("padding");
		}
	  else /* SEQ node: ell == L+1 */
		{ if (i-(qqqq+kkkk+uuuu+vvvv)<cccc) /* initial portion: IV or chaining */
		{ if (ii == 0)
			printf("IV");
		  else
			printf("chaining from (%d,%d)",ell,ii-1);
		}
		  else /* data, chaining from below, or padding */
		{ if (i+(p/wwww)>=nnnn)
			printf("padding");
		  else if (ell == 1)
			printf("input message word %4d",
				ii*(bbbb-cccc)+(i-(qqqq+kkkk+uuuu+vvvv+cccc)));
		  else 
			printf(
				"chaining from (%d,%d)",
				ell-1,
				3*ii+(i-(qqqq+kkkk+uuuu+vvvv+cccc))/cccc);
		}
		}
	  printf("\n");
	}
	else if ((i>=r*cccc+nnnn-cccc))
	{ if ((i==r*cccc+nnnn-cccc))
		printf("Output (%d words of chaining values):\n",cccc);
	  printf("A[%4d] = " PR_MD6_WORD,i,A[i]);
	  printf(" output chaining value C[%d]\n",i-(r*cccc+nnnn-cccc));
	}
	  else 
	{ if (i==nnnn)
		{ if (print_intermediate)
		printf("Intermediate values:\n");
		  else
		printf(
			"Intermediate values A[%d..%d] omitted... "
			"\n",nnnn,r*cccc+nnnn-cccc-1);
		}
	  if (print_intermediate)
		printf("A[%4d] = " PR_MD6_WORD "\n",i,A[i]);
	}
  }
  printf("\n");

#endif

}


/* interface to hash routines
*/

void hash_init()
{ int err;
  start_timer();
  if ((err=md6_full_init(&md6_st, md6_dgtLen, md6_K,md6_keylen, md6_modPar, md6_roundN)))
    { printf("Bad MD6 parameters; can't initialize md6. "
	     "errcode = %d\n",err);
      return;
    }
  if (print_input_output)
    compression_hook = compression_hook_1;
}

void hash_update(char* data, 
		 unsigned long long databitlen)
{ int err;
  if ((err=md6_update(&md6_st, 
		      (unsigned char *)data, 
		      databitlen)))
    { printf("MD6 update error. error code: %d\n",err);
      return;
    }
}

void hash_final()
{ int err;
  if ((err=md6_final(&md6_st,NULL)))
    { printf("MD6 finalization error. error code: %d\n",err);
      return;
    }
  end_timer();
}

void hash_filep(FILE *inFile)
{ unsigned long long bytes;
  char data[1024];
  if (inFile==NULL)
    { printf("hash_filep has NULL input file pointer.\n");
      return;
    }
  hash_init();
  while ((bytes = fread (data, 1, 1024, inFile)) != 0)
    hash_update(data,bytes*8);
  hash_final();
}

void hash_stdin()
{ hash_filep(stdin);
}

void hash_file( char *filename )
{ FILE *inFile = fopen (filename, "rb");
  if ( inFile == NULL ) 
    { printf("%s can't be opened.\n", filename);
      return;
    }
  hash_filep(inFile);
  fclose(inFile);
}

void hash_b(unsigned long long bitlen)
/* Hash dummy input file of length bitlen bits.
** File (hex) repeats with period 7:
**   11 22 33 44 55 66 77 11 22 33 44 55 66 77 11 22 33 ...
*/
{ int i;
  char data[700];  /* nice if length is multiple of 7 for periodicity */
  for (i=0;i<700;i++)
    data[i] =  0x11 + (char)((i % 7)*(0x11));
  hash_init();
  while (bitlen>0)
    { unsigned long long part_len = min(700*8,bitlen);
      hash_update(data,part_len);
      bitlen = bitlen - part_len;
    }
  hash_final();
}

void print_hash(char *filename)
{ 
#ifdef DEBUG_PRINT
  print_tod();
  if (print_input_output == 0)
	printf("%s %s\n", md6_st.hexhashval,filename);
  else
	printf("Final hash value = %s\n", md6_st.hexhashval);
  print_time(); /* running time */
#endif
}

#endif /// MD6_CHECKSUM_ENCIPHER







#if SHA1_HASH_ENCIPHER

/**
 *
 *  Copyright (C) 2006-2010, Brainspark B.V.
 *
 *  This file is part of PolarSSL (http://www.polarssl.org)
 *  Lead Maintainer: Paul Bakker <polarssl_maintainer at polarssl.org>
 *
 *  All rights reserved.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */


#define MAX_BUFFER_SIZE				4096

#define OBJECT_FILE_LIST 			0xFF000001
#define OBJECT_COMMIT_FILE 			0xFF000002




/*20 bytes for sha so 40 bytes to put it in hex and one byte for NULL
 * A SHA is valid only if it ends with a NULL char*/
#define SHA_HASH_LENGTH				40


typedef unsigned char ShaBuffer[SHA_HASH_LENGTH+1];

///bool sha_buffer(const unsigned char *input, int ilen, ShaBuffer sha);
#ifdef __NOT_USED__
bool sha_file(const char *path, ShaBuffer sha);
#endif
///bool sha_compare(const ShaBuffer s1, const ShaBuffer s2);
/// inline void sha_reset(ShaBuffer sha);



/*
 * 32-bit integer manipulation macros (big endian)
 */

#ifndef GET_ULONG_BE
#define GET_ULONG_BE(n,b,i)                             \
{                                                       \
    (n) = ( (unsigned long) (b)[(i)    ] << 24 )        \
        | ( (unsigned long) (b)[(i) + 1] << 16 )        \
        | ( (unsigned long) (b)[(i) + 2] <<  8 )        \
        | ( (unsigned long) (b)[(i) + 3]       );       \
}
#endif

#ifndef PUT_ULONG_BE
#define PUT_ULONG_BE(n,b,i)                             \
{                                                       \
    (b)[(i)    ] = (unsigned char) ( (n) >> 24 );       \
    (b)[(i) + 1] = (unsigned char) ( (n) >> 16 );       \
    (b)[(i) + 2] = (unsigned char) ( (n) >>  8 );       \
    (b)[(i) + 3] = (unsigned char) ( (n)       );       \
}
#endif

typedef struct
{
    unsigned long total[2];     /*!< number of bytes processed  */
    unsigned long state[5];     /*!< intermediate digest state  */
    unsigned char buffer[64];   /*!< data block being processed */
    unsigned char ipad[64];     /*!< HMAC: inner padding        */
    unsigned char opad[64];     /*!< HMAC: outer padding        */
} sha1_context;



static void sha1_starts(sha1_context *ctx);
static void sha1_update(sha1_context *ctx, const unsigned char *input, int ilen);
static void sha1_finish(sha1_context *ctx, ShaBuffer sha);

/*
 * SHA-1 context setup
 */
void sha1_starts(sha1_context *ctx)
{
    ctx->total[0] = 0;
    ctx->total[1] = 0;

    ctx->state[0] = 0x67452301UL;
    ctx->state[1] = 0xEFCDAB89UL;
    ctx->state[2] = 0x98BADCFEUL;
    ctx->state[3] = 0x10325476UL;
    ctx->state[4] = 0xC3D2E1F0UL;
}

static void sha1_process( sha1_context *ctx, const unsigned char data[64] )
{
    unsigned long temp, W[16], A, B, C, D, E;

    GET_ULONG_BE( W[ 0], data,  0 );
    GET_ULONG_BE( W[ 1], data,  4 );
    GET_ULONG_BE( W[ 2], data,  8 );
    GET_ULONG_BE( W[ 3], data, 12 );
    GET_ULONG_BE( W[ 4], data, 16 );
    GET_ULONG_BE( W[ 5], data, 20 );
    GET_ULONG_BE( W[ 6], data, 24 );
    GET_ULONG_BE( W[ 7], data, 28 );
    GET_ULONG_BE( W[ 8], data, 32 );
    GET_ULONG_BE( W[ 9], data, 36 );
    GET_ULONG_BE( W[10], data, 40 );
    GET_ULONG_BE( W[11], data, 44 );
    GET_ULONG_BE( W[12], data, 48 );
    GET_ULONG_BE( W[13], data, 52 );
    GET_ULONG_BE( W[14], data, 56 );
    GET_ULONG_BE( W[15], data, 60 );

#define SHA1_S(x,n) ((x << n) | ((x & 0xFFFFFFFF) >> (32 - n)))

#define SHA1_R(t)                                            \
(                                                       \
    temp = W[(t -  3) & 0x0F] ^ W[(t - 8) & 0x0F] ^     \
           W[(t - 14) & 0x0F] ^ W[ t      & 0x0F],      \
    ( W[t & 0x0F] = SHA1_S(temp,1) )                    \
)

#define SHA1_P(a,b,c,d,e,x)                                 \
{                                                           \
    e += SHA1_S(a,5) + F(b,c,d) + K + x; b = SHA1_S(b,30);  \
}

    A = ctx->state[0];
    B = ctx->state[1];
    C = ctx->state[2];
    D = ctx->state[3];
    E = ctx->state[4];

#define F(x,y,z) (z ^ (x & (y ^ z)))

#define K 0x5A827999

    SHA1_P( A, B, C, D, E, W[0]  );
    SHA1_P( E, A, B, C, D, W[1]  );
    SHA1_P( D, E, A, B, C, W[2]  );
    SHA1_P( C, D, E, A, B, W[3]  );
    SHA1_P( B, C, D, E, A, W[4]  );
    SHA1_P( A, B, C, D, E, W[5]  );
    SHA1_P( E, A, B, C, D, W[6]  );
    SHA1_P( D, E, A, B, C, W[7]  );
    SHA1_P( C, D, E, A, B, W[8]  );
    SHA1_P( B, C, D, E, A, W[9]  );
    SHA1_P( A, B, C, D, E, W[10] );
    SHA1_P( E, A, B, C, D, W[11] );
    SHA1_P( D, E, A, B, C, W[12] );
    SHA1_P( C, D, E, A, B, W[13] );
    SHA1_P( B, C, D, E, A, W[14] );
    SHA1_P( A, B, C, D, E, W[15] );
    SHA1_P( E, A, B, C, D, SHA1_R(16) );
    SHA1_P( D, E, A, B, C, SHA1_R(17) );
    SHA1_P( C, D, E, A, B, SHA1_R(18) );
    SHA1_P( B, C, D, E, A, SHA1_R(19) );

#undef K
#undef F

#define F(x,y,z) (x ^ y ^ z)
#define K 0x6ED9EBA1

    SHA1_P( A, B, C, D, E, SHA1_R(20) );
    SHA1_P( E, A, B, C, D, SHA1_R(21) );
    SHA1_P( D, E, A, B, C, SHA1_R(22) );
    SHA1_P( C, D, E, A, B, SHA1_R(23) );
    SHA1_P( B, C, D, E, A, SHA1_R(24) );
    SHA1_P( A, B, C, D, E, SHA1_R(25) );
    SHA1_P( E, A, B, C, D, SHA1_R(26) );
    SHA1_P( D, E, A, B, C, SHA1_R(27) );
    SHA1_P( C, D, E, A, B, SHA1_R(28) );
    SHA1_P( B, C, D, E, A, SHA1_R(29) );
    SHA1_P( A, B, C, D, E, SHA1_R(30) );
    SHA1_P( E, A, B, C, D, SHA1_R(31) );
    SHA1_P( D, E, A, B, C, SHA1_R(32) );
    SHA1_P( C, D, E, A, B, SHA1_R(33) );
    SHA1_P( B, C, D, E, A, SHA1_R(34) );
    SHA1_P( A, B, C, D, E, SHA1_R(35) );
    SHA1_P( E, A, B, C, D, SHA1_R(36) );
    SHA1_P( D, E, A, B, C, SHA1_R(37) );
    SHA1_P( C, D, E, A, B, SHA1_R(38) );
    SHA1_P( B, C, D, E, A, SHA1_R(39) );

#undef K
#undef F

#define F(x,y,z) ((x & y) | (z & (x | y)))
#define K 0x8F1BBCDC

    SHA1_P( A, B, C, D, E, SHA1_R(40) );
    SHA1_P( E, A, B, C, D, SHA1_R(41) );
    SHA1_P( D, E, A, B, C, SHA1_R(42) );
    SHA1_P( C, D, E, A, B, SHA1_R(43) );
    SHA1_P( B, C, D, E, A, SHA1_R(44) );
    SHA1_P( A, B, C, D, E, SHA1_R(45) );
    SHA1_P( E, A, B, C, D, SHA1_R(46) );
    SHA1_P( D, E, A, B, C, SHA1_R(47) );
    SHA1_P( C, D, E, A, B, SHA1_R(48) );
    SHA1_P( B, C, D, E, A, SHA1_R(49) );
    SHA1_P( A, B, C, D, E, SHA1_R(50) );
    SHA1_P( E, A, B, C, D, SHA1_R(51) );
    SHA1_P( D, E, A, B, C, SHA1_R(52) );
    SHA1_P( C, D, E, A, B, SHA1_R(53) );
    SHA1_P( B, C, D, E, A, SHA1_R(54) );
    SHA1_P( A, B, C, D, E, SHA1_R(55) );
    SHA1_P( E, A, B, C, D, SHA1_R(56) );
    SHA1_P( D, E, A, B, C, SHA1_R(57) );
    SHA1_P( C, D, E, A, B, SHA1_R(58) );
    SHA1_P( B, C, D, E, A, SHA1_R(59) );

#undef K
#undef F

#define F(x,y,z) (x ^ y ^ z)
#define K 0xCA62C1D6

    SHA1_P( A, B, C, D, E, SHA1_R(60) );
    SHA1_P( E, A, B, C, D, SHA1_R(61) );
    SHA1_P( D, E, A, B, C, SHA1_R(62) );
    SHA1_P( C, D, E, A, B, SHA1_R(63) );
    SHA1_P( B, C, D, E, A, SHA1_R(64) );
    SHA1_P( A, B, C, D, E, SHA1_R(65) );
    SHA1_P( E, A, B, C, D, SHA1_R(66) );
    SHA1_P( D, E, A, B, C, SHA1_R(67) );
    SHA1_P( C, D, E, A, B, SHA1_R(68) );
    SHA1_P( B, C, D, E, A, SHA1_R(69) );
    SHA1_P( A, B, C, D, E, SHA1_R(70) );
    SHA1_P( E, A, B, C, D, SHA1_R(71) );
    SHA1_P( D, E, A, B, C, SHA1_R(72) );
    SHA1_P( C, D, E, A, B, SHA1_R(73) );
    SHA1_P( B, C, D, E, A, SHA1_R(74) );
    SHA1_P( A, B, C, D, E, SHA1_R(75) );
    SHA1_P( E, A, B, C, D, SHA1_R(76) );
    SHA1_P( D, E, A, B, C, SHA1_R(77) );
    SHA1_P( C, D, E, A, B, SHA1_R(78) );
    SHA1_P( B, C, D, E, A, SHA1_R(79) );

#undef K
#undef F

    ctx->state[0] += A;
    ctx->state[1] += B;
    ctx->state[2] += C;
    ctx->state[3] += D;
    ctx->state[4] += E;
}

/*
 * SHA-1 process buffer
 */
void sha1_update( sha1_context *ctx, const unsigned char *input, int ilen )
{
    int fill;
    unsigned long left;

    if( ilen <= 0 )
        return;

    left = ctx->total[0] & 0x3F;
    fill = 64 - left;

    ctx->total[0] += ilen;
    ctx->total[0] &= 0xFFFFFFFF;

    if( ctx->total[0] < (unsigned long) ilen )
        ctx->total[1]++;

    if( left && ilen >= fill )
    {
        memcpy( (void *) (ctx->buffer + left),
                (void *) input, fill );
        sha1_process( ctx, ctx->buffer );
        input += fill;
        ilen  -= fill;
        left = 0;
    }

    while( ilen >= 64 )
    {
        sha1_process( ctx, input );
        input += 64;
        ilen  -= 64;
    }

    if( ilen > 0 )
    {
        memcpy( (void *) (ctx->buffer + left),
                (void *) input, ilen );
    }
}

static const unsigned char sha1_padding[64] =
{
 0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

/*
 * SHA-1 final digest
 */
void sha1_finish( sha1_context *ctx, ShaBuffer output)
{
	unsigned int i;
	unsigned char buffer[20];
	unsigned long last, padn;
	unsigned long high, low;
	unsigned char msglen[8];

    high = ( ctx->total[0] >> 29 )
         | ( ctx->total[1] <<  3 );
    low  = ( ctx->total[0] <<  3 );

    PUT_ULONG_BE( high, msglen, 0 );
    PUT_ULONG_BE( low,  msglen, 4 );

    last = ctx->total[0] & 0x3F;
    padn = ( last < 56 ) ? ( 56 - last ) : ( 120 - last );

    sha1_update( ctx, (unsigned char *) sha1_padding, padn );
    sha1_update( ctx, msglen, 8 );

    PUT_ULONG_BE( ctx->state[0], buffer,  0 );
    PUT_ULONG_BE( ctx->state[1], buffer,  4 );
    PUT_ULONG_BE( ctx->state[2], buffer,  8 );
    PUT_ULONG_BE( ctx->state[3], buffer, 12 );
    PUT_ULONG_BE( ctx->state[4], buffer, 16 );

	output[0] = '\0';
	for(i = 0; i < 20; i++)
	{
		char str[3];
		snprintf(str, 3, "%2.2x", (unsigned char)buffer[i]);
		strcat((char*)output, str);
	}
	output[SHA_HASH_LENGTH] = '\0';
}

/*
 * output = SHA-1( input buffer )
 */
bool sha_buffer( const unsigned char *input, int ilen, ShaBuffer output)
{
    sha1_context ctx;
    sha1_starts( &ctx );
    sha1_update( &ctx, input, ilen );
    sha1_finish( &ctx, output );
    memset( &ctx, 0, sizeof( sha1_context ));
	return true;
}


#ifdef __NOT_USED__
/*
 * output = SHA-1( file contents )
 */
bool sha_file( const char *path, ShaBuffer output)
{
    FILE *f;
    size_t n;
    sha1_context ctx;
    unsigned char buf[1024];


    if((f = fopen( path, "rb")) == NULL)
        return false;

    sha1_starts(&ctx);

    while((n = fread(buf, 1, sizeof(buf), f)) > 0)
        sha1_update(&ctx, buf, (int)n);

    sha1_finish(&ctx, output);

    memset(&ctx, 0, sizeof(sha1_context));

    if(ferror(f) != 0)
    {
        fclose(f);
        return(false);
    }

    fclose(f);
    return true;
}
#endif /// __NOT_USED__

bool sha_compare(const ShaBuffer s1, const ShaBuffer s2)
{
	int i;
	for(i = 0; i < SHA_HASH_LENGTH; i++)
	{
		if(s1[i] != s2[i])
			return false;
	}
	return true;
}

void sha_reset(ShaBuffer sha)
{
	memset(sha, 0, SHA_HASH_LENGTH+1);
	return;
}
#endif /// SHA1_HASH_ENCIPHER




#if SHA2_224_CHECKSUM

#define SHFR(x, n)    (x >> n)
#define ROTR(x, n)   ((x >> n) | (x << ((sizeof(x) << 3) - n)))
#define ROTL(x, n)   ((x << n) | (x >> ((sizeof(x) << 3) - n)))
#define CH(x, y, z)  ((x & y) ^ (~x & z))
#define MAJ(x, y, z) ((x & y) ^ (x & z) ^ (y & z))

#define SHA256_F1(x) (ROTR(x,  2) ^ ROTR(x, 13) ^ ROTR(x, 22))
#define SHA256_F2(x) (ROTR(x,  6) ^ ROTR(x, 11) ^ ROTR(x, 25))
#define SHA256_F3(x) (ROTR(x,  7) ^ ROTR(x, 18) ^ SHFR(x,  3))
#define SHA256_F4(x) (ROTR(x, 17) ^ ROTR(x, 19) ^ SHFR(x, 10))


#define SHA256_SCR(i)                         \
{                                             \
    w[i] =  SHA256_F4(w[i -  2]) + w[i -  7]  \
          + SHA256_F3(w[i - 15]) + w[i - 16]; \
}


#define SHA256_EXP(a, b, c, d, e, f, g, h, j)               \
{                                                           \
    t1 = wv[h] + SHA256_F2(wv[e]) + CH(wv[e], wv[f], wv[g]) \
         + sha256_k[j] + w[j];                              \
    t2 = SHA256_F1(wv[a]) + MAJ(wv[a], wv[b], wv[c]);       \
    wv[d] += t1;                                            \
    wv[h] = t1 + t2;                                        \
}


#define UNPACK32(x, str)                      \
{                                             \
    *((str) + 3) = (unsigned char) ((x)      );       \
    *((str) + 2) = (unsigned char) ((x) >>  8);       \
    *((str) + 1) = (unsigned char) ((x) >> 16);       \
    *((str) + 0) = (unsigned char) ((x) >> 24);       \
}

#define PACK32(str, x)                        \
{                                             \
    *(x) =   ((unsigned int) *((str) + 3)      )    \
           | ((unsigned int) *((str) + 2) <<  8)    \
           | ((unsigned int) *((str) + 1) << 16)    \
           | ((unsigned int) *((str) + 0) << 24);   \
}

const unsigned int sha224_h0[8] =
            {0xc1059ed8, 0x367cd507, 0x3070dd17, 0xf70e5939,
             0xffc00b31, 0x68581511, 0x64f98fa7, 0xbefa4fa4};

const unsigned int sha256_k[64] =
            {0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5,
             0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
             0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
             0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
             0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc,
             0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
             0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7,
             0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
             0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
             0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
             0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3,
             0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
             0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5,
             0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
             0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
             0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2};


#define SHA224_DIGEST_SIZE ( 224 / 8)
#define SHA256_DIGEST_SIZE ( 256 / 8)
#define SHA384_DIGEST_SIZE ( 384 / 8)
#define SHA512_DIGEST_SIZE ( 512 / 8)

#define SHA256_BLOCK_SIZE  ( 512 / 8)
#define SHA512_BLOCK_SIZE  (1024 / 8)
#define SHA384_BLOCK_SIZE  SHA512_BLOCK_SIZE
#define SHA224_BLOCK_SIZE  SHA256_BLOCK_SIZE


typedef struct {
    unsigned int tot_len;
    unsigned int len;
    unsigned char block[2 * SHA256_BLOCK_SIZE];
    unsigned int h[8];
} sha256_ctx;


typedef sha256_ctx sha224_ctx;



void sha256_transf(sha256_ctx *ctx, const unsigned char *message, unsigned int block_nb)
{
    unsigned int w[64];
    unsigned int wv[8];
    unsigned int sha224_t1, sha224_t2;
    const unsigned char *sub_block;
    int i;

#ifndef UNROLL_LOOPS
    int j;
#endif

    for (i = 0; i < (int) block_nb; i++) {
        sub_block = message + (i << 6);

#ifndef UNROLL_LOOPS
        for (j = 0; j < 16; j++) {
            PACK32(&sub_block[j << 2], &w[j]);
        }

        for (j = 16; j < 64; j++) {
            SHA256_SCR(j);
        }

        for (j = 0; j < 8; j++) {
            wv[j] = ctx->h[j];
        }

        for (j = 0; j < 64; j++) {
            sha224_t1 = wv[7] + SHA256_F2(wv[4]) + CH(wv[4], wv[5], wv[6])
                + sha256_k[j] + w[j];
            sha224_t2 = SHA256_F1(wv[0]) + MAJ(wv[0], wv[1], wv[2]);
            wv[7] = wv[6];
            wv[6] = wv[5];
            wv[5] = wv[4];
            wv[4] = wv[3] + sha224_t1;
            wv[3] = wv[2];
            wv[2] = wv[1];
            wv[1] = wv[0];
            wv[0] = sha224_t1 + sha224_t2;
        }

        for (j = 0; j < 8; j++) {
            ctx->h[j] += wv[j];
        }
#else
        PACK32(&sub_block[ 0], &w[ 0]); PACK32(&sub_block[ 4], &w[ 1]);
        PACK32(&sub_block[ 8], &w[ 2]); PACK32(&sub_block[12], &w[ 3]);
        PACK32(&sub_block[16], &w[ 4]); PACK32(&sub_block[20], &w[ 5]);
        PACK32(&sub_block[24], &w[ 6]); PACK32(&sub_block[28], &w[ 7]);
        PACK32(&sub_block[32], &w[ 8]); PACK32(&sub_block[36], &w[ 9]);
        PACK32(&sub_block[40], &w[10]); PACK32(&sub_block[44], &w[11]);
        PACK32(&sub_block[48], &w[12]); PACK32(&sub_block[52], &w[13]);
        PACK32(&sub_block[56], &w[14]); PACK32(&sub_block[60], &w[15]);

        SHA256_SCR(16); SHA256_SCR(17); SHA256_SCR(18); SHA256_SCR(19);
        SHA256_SCR(20); SHA256_SCR(21); SHA256_SCR(22); SHA256_SCR(23);
        SHA256_SCR(24); SHA256_SCR(25); SHA256_SCR(26); SHA256_SCR(27);
        SHA256_SCR(28); SHA256_SCR(29); SHA256_SCR(30); SHA256_SCR(31);
        SHA256_SCR(32); SHA256_SCR(33); SHA256_SCR(34); SHA256_SCR(35);
        SHA256_SCR(36); SHA256_SCR(37); SHA256_SCR(38); SHA256_SCR(39);
        SHA256_SCR(40); SHA256_SCR(41); SHA256_SCR(42); SHA256_SCR(43);
        SHA256_SCR(44); SHA256_SCR(45); SHA256_SCR(46); SHA256_SCR(47);
        SHA256_SCR(48); SHA256_SCR(49); SHA256_SCR(50); SHA256_SCR(51);
        SHA256_SCR(52); SHA256_SCR(53); SHA256_SCR(54); SHA256_SCR(55);
        SHA256_SCR(56); SHA256_SCR(57); SHA256_SCR(58); SHA256_SCR(59);
        SHA256_SCR(60); SHA256_SCR(61); SHA256_SCR(62); SHA256_SCR(63);

        wv[0] = ctx->h[0]; wv[1] = ctx->h[1];
        wv[2] = ctx->h[2]; wv[3] = ctx->h[3];
        wv[4] = ctx->h[4]; wv[5] = ctx->h[5];
        wv[6] = ctx->h[6]; wv[7] = ctx->h[7];

        SHA256_EXP(0,1,2,3,4,5,6,7, 0); SHA256_EXP(7,0,1,2,3,4,5,6, 1);
        SHA256_EXP(6,7,0,1,2,3,4,5, 2); SHA256_EXP(5,6,7,0,1,2,3,4, 3);
        SHA256_EXP(4,5,6,7,0,1,2,3, 4); SHA256_EXP(3,4,5,6,7,0,1,2, 5);
        SHA256_EXP(2,3,4,5,6,7,0,1, 6); SHA256_EXP(1,2,3,4,5,6,7,0, 7);
        SHA256_EXP(0,1,2,3,4,5,6,7, 8); SHA256_EXP(7,0,1,2,3,4,5,6, 9);
        SHA256_EXP(6,7,0,1,2,3,4,5,10); SHA256_EXP(5,6,7,0,1,2,3,4,11);
        SHA256_EXP(4,5,6,7,0,1,2,3,12); SHA256_EXP(3,4,5,6,7,0,1,2,13);
        SHA256_EXP(2,3,4,5,6,7,0,1,14); SHA256_EXP(1,2,3,4,5,6,7,0,15);
        SHA256_EXP(0,1,2,3,4,5,6,7,16); SHA256_EXP(7,0,1,2,3,4,5,6,17);
        SHA256_EXP(6,7,0,1,2,3,4,5,18); SHA256_EXP(5,6,7,0,1,2,3,4,19);
        SHA256_EXP(4,5,6,7,0,1,2,3,20); SHA256_EXP(3,4,5,6,7,0,1,2,21);
        SHA256_EXP(2,3,4,5,6,7,0,1,22); SHA256_EXP(1,2,3,4,5,6,7,0,23);
        SHA256_EXP(0,1,2,3,4,5,6,7,24); SHA256_EXP(7,0,1,2,3,4,5,6,25);
        SHA256_EXP(6,7,0,1,2,3,4,5,26); SHA256_EXP(5,6,7,0,1,2,3,4,27);
        SHA256_EXP(4,5,6,7,0,1,2,3,28); SHA256_EXP(3,4,5,6,7,0,1,2,29);
        SHA256_EXP(2,3,4,5,6,7,0,1,30); SHA256_EXP(1,2,3,4,5,6,7,0,31);
        SHA256_EXP(0,1,2,3,4,5,6,7,32); SHA256_EXP(7,0,1,2,3,4,5,6,33);
        SHA256_EXP(6,7,0,1,2,3,4,5,34); SHA256_EXP(5,6,7,0,1,2,3,4,35);
        SHA256_EXP(4,5,6,7,0,1,2,3,36); SHA256_EXP(3,4,5,6,7,0,1,2,37);
        SHA256_EXP(2,3,4,5,6,7,0,1,38); SHA256_EXP(1,2,3,4,5,6,7,0,39);
        SHA256_EXP(0,1,2,3,4,5,6,7,40); SHA256_EXP(7,0,1,2,3,4,5,6,41);
        SHA256_EXP(6,7,0,1,2,3,4,5,42); SHA256_EXP(5,6,7,0,1,2,3,4,43);
        SHA256_EXP(4,5,6,7,0,1,2,3,44); SHA256_EXP(3,4,5,6,7,0,1,2,45);
        SHA256_EXP(2,3,4,5,6,7,0,1,46); SHA256_EXP(1,2,3,4,5,6,7,0,47);
        SHA256_EXP(0,1,2,3,4,5,6,7,48); SHA256_EXP(7,0,1,2,3,4,5,6,49);
        SHA256_EXP(6,7,0,1,2,3,4,5,50); SHA256_EXP(5,6,7,0,1,2,3,4,51);
        SHA256_EXP(4,5,6,7,0,1,2,3,52); SHA256_EXP(3,4,5,6,7,0,1,2,53);
        SHA256_EXP(2,3,4,5,6,7,0,1,54); SHA256_EXP(1,2,3,4,5,6,7,0,55);
        SHA256_EXP(0,1,2,3,4,5,6,7,56); SHA256_EXP(7,0,1,2,3,4,5,6,57);
        SHA256_EXP(6,7,0,1,2,3,4,5,58); SHA256_EXP(5,6,7,0,1,2,3,4,59);
        SHA256_EXP(4,5,6,7,0,1,2,3,60); SHA256_EXP(3,4,5,6,7,0,1,2,61);
        SHA256_EXP(2,3,4,5,6,7,0,1,62); SHA256_EXP(1,2,3,4,5,6,7,0,63);

        ctx->h[0] += wv[0]; ctx->h[1] += wv[1];
        ctx->h[2] += wv[2]; ctx->h[3] += wv[3];
        ctx->h[4] += wv[4]; ctx->h[5] += wv[5];
        ctx->h[6] += wv[6]; ctx->h[7] += wv[7];
#endif /* !UNROLL_LOOPS */
    }
}


void sha224_init(sha224_ctx *ctx)
{
#ifndef UNROLL_LOOPS
    int i;
    for (i = 0; i < 8; i++) {
        ctx->h[i] = sha224_h0[i];
    }
#else
    ctx->h[0] = sha224_h0[0]; 
	ctx->h[1] = sha224_h0[1];
    ctx->h[2] = sha224_h0[2]; 
	ctx->h[3] = sha224_h0[3];
    ctx->h[4] = sha224_h0[4]; 
	ctx->h[5] = sha224_h0[5];
    ctx->h[6] = sha224_h0[6]; 
	ctx->h[7] = sha224_h0[7];
#endif /* !UNROLL_LOOPS */

    ctx->len = 0;
    ctx->tot_len = 0;
}

void sha224_update(sha224_ctx *ctx, const unsigned char *message, unsigned int len)
{
    unsigned int block_nb;
    unsigned int new_len, rem_len, tmp_len;
    const unsigned char *shifted_message;

    tmp_len = SHA224_BLOCK_SIZE - ctx->len;
    rem_len = len < tmp_len ? len : tmp_len;

    memcpy(&ctx->block[ctx->len], message, rem_len);

    if (ctx->len + len < SHA224_BLOCK_SIZE) {
        ctx->len += len;
        return;
    }

    new_len = len - rem_len;
    block_nb = new_len / SHA224_BLOCK_SIZE;

    shifted_message = message + rem_len;

    sha256_transf(ctx, ctx->block, 1);
    sha256_transf(ctx, shifted_message, block_nb);

    rem_len = new_len % SHA224_BLOCK_SIZE;

    memcpy(ctx->block, &shifted_message[block_nb << 6],
           rem_len);

    ctx->len = rem_len;
    ctx->tot_len += (block_nb + 1) << 6;
}

void sha224_final(sha224_ctx *ctx, unsigned char *digest)
{
    unsigned int block_nb;
    unsigned int pm_len;
    unsigned int len_b;

#ifndef UNROLL_LOOPS
    int i;
#endif

    block_nb = (1 + ((SHA224_BLOCK_SIZE - 9)
                     < (ctx->len % SHA224_BLOCK_SIZE)));

    len_b = (ctx->tot_len + ctx->len) << 3;
    pm_len = block_nb << 6;

    memset(ctx->block + ctx->len, 0, pm_len - ctx->len);
    ctx->block[ctx->len] = 0x80;
    UNPACK32(len_b, ctx->block + pm_len - 4);

    sha256_transf(ctx, ctx->block, block_nb);

#ifndef UNROLL_LOOPS
    for (i = 0 ; i < 7; i++) {
        UNPACK32(ctx->h[i], &digest[i << 2]);
    }
#else
   UNPACK32(ctx->h[0], &digest[ 0]);
   UNPACK32(ctx->h[1], &digest[ 4]);
   UNPACK32(ctx->h[2], &digest[ 8]);
   UNPACK32(ctx->h[3], &digest[12]);
   UNPACK32(ctx->h[4], &digest[16]);
   UNPACK32(ctx->h[5], &digest[20]);
   UNPACK32(ctx->h[6], &digest[24]);
#endif /* !UNROLL_LOOPS */
}


static void sha224Print (unsigned char *sha224_digest)
{
  int i;

  for (i = 0; i < SHA224_DIGEST_SIZE; i++)
  {
    printf ("%02x", sha224_digest[i]);
	if(outfile) fprintf(outfile, "%02x", sha224_digest[i]);
  }
}


/* SHA-224 functions */
void sha224(const unsigned char *message, unsigned int len, unsigned char *digest)
{
    sha224_ctx ctx;

    sha224_init(&ctx);
    sha224_update(&ctx, message, len);
    sha224_final(&ctx, digest);
}
#endif /// SHA2_224_CHECKSUM -------------




#if SHA2_256_384_512 /// 2014.06.30
/*
 * FILE:	sha2.c
 * AUTHOR:	Aaron D. Gifford - http://www.aarongifford.com/
 * 
 * Copyright (c) 2000-2001, Aaron D. Gifford
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 */


#define SHA2_BUFLEN 			(100*1024) // 16384 // 16*1024



/*** SHA-256/384/512 Various Length Definitions ***********************/
#define SHA256_BLOCK_LENGTH			64
#define SHA256_DIGEST_LENGTH 		32
#define SHA256_DIGEST_STRING_LENGTH	(SHA256_DIGEST_LENGTH * 2 + 1)
#define SHA384_BLOCK_LENGTH			128
#define SHA384_DIGEST_LENGTH 		48
#define SHA384_DIGEST_STRING_LENGTH	(SHA384_DIGEST_LENGTH * 2 + 1)
#define SHA512_BLOCK_LENGTH			128
#define SHA512_DIGEST_LENGTH		64
#define SHA512_DIGEST_STRING_LENGTH	(SHA512_DIGEST_LENGTH * 2 + 1)

#if 0
/*** SHA-256/384/512 Context Structures *******************************/
/* NOTE: If your architecture does not define either u_intXX_t types or
 * uintXX_t (from inttypes.h), you may need to define things by hand
 * for your system:
 */

typedef unsigned char      u_int8_t;	/* 1-byte  (8-bits)  */
typedef unsigned int       u_int32_t;	/* 4-bytes (32-bits) */
typedef unsigned long long u_int64_t;	/* 8-bytes (64-bits) */
typedef unsigned short     u_int16_t;
#endif

/*
 * Most BSD systems already define u_intXX_t types, as does Linux.
 * Some systems, however, like Compaq's Tru64 Unix instead can use
 * uintXX_t types defined by very recent ANSI C standards and included
 * in the file:
 *
 *   #include <inttypes.h>
 *
 * If you choose to use <inttypes.h> then please define: 
 *
 *   #define SHA2_USE_INTTYPES_H
 *
 * Or on the command line during compile:
 *
 *   cc -DSHA2_USE_INTTYPES_H ...
 */
#ifdef SHA2_USE_INTTYPES_H
typedef struct _SHA256_CTX {
	unsigned int	state[8];
	unsigned long long	bitcount;
	unsigned char	buffer[SHA256_BLOCK_LENGTH];
} SHA256_CTX;
typedef struct _SHA512_CTX {
	unsigned long long	state[8];
	unsigned long long	bitcount[2];
	unsigned char	buffer[SHA512_BLOCK_LENGTH];
} SHA512_CTX;
#else /* SHA2_USE_INTTYPES_H */

typedef struct _SHA256_CTX {
	u_int32_t	state[8];
	u_int64_t	bitcount;
	u_int8_t	buffer[SHA256_BLOCK_LENGTH];
} SHA256_CTX;
typedef struct _SHA512_CTX {
	u_int64_t	state[8];
	u_int64_t	bitcount[2];
	u_int8_t	buffer[SHA512_BLOCK_LENGTH];
} SHA512_CTX;

#endif /* SHA2_USE_INTTYPES_H */

typedef SHA512_CTX SHA384_CTX;


/*
 * UNROLLED TRANSFORM LOOP NOTE:
 * You can define SHA2_UNROLL_TRANSFORM to use the unrolled transform
 * loop version for the hash transform rounds (defined using macros
 * later in this file).  Either define on the command line, for example:
 *
 *   cc -DSHA2_UNROLL_TRANSFORM -o sha2 sha2.c sha2prog.c
 *
 * or define below:
 *
 *   #define SHA2_UNROLL_TRANSFORM
 *
 */


#define SHA2_UNROLL_TRANSFORM 				1 /// 2014.06.30

#define LITTLE_ENDIAN 			1234
#define BIG_ENDIAN 				4321

#define BYTE_ORDER 			LITTLE_ENDIAN   /// 2014.06.30 -- OK
///#define BYTE_ORDER 			BIG_ENDIAN



/*** SHA-256/384/512 Machine Architecture Definitions *****************/
/*
 * BYTE_ORDER NOTE:
 *
 * Please make sure that your system defines BYTE_ORDER.  If your
 * architecture is little-endian, make sure it also defines
 * LITTLE_ENDIAN and that the two (BYTE_ORDER and LITTLE_ENDIAN) are
 * equivilent.
 *
 * If your system does not define the above, then you can do so by
 * hand like this:
 *
 *   #define LITTLE_ENDIAN 1234
 *   #define BIG_ENDIAN    4321
 *
 * And for little-endian machines, add:
 *
 *   #define BYTE_ORDER LITTLE_ENDIAN 
 *
 * Or for big-endian machines:
 *
 *   #define BYTE_ORDER BIG_ENDIAN
 *
 * The FreeBSD machine this was written on defines BYTE_ORDER
 * appropriately by including <sys/types.h> (which in turn includes
 * <machine/endian.h> where the appropriate definitions are actually
 * made).
 */
#if !defined(BYTE_ORDER) || (BYTE_ORDER != LITTLE_ENDIAN && BYTE_ORDER != BIG_ENDIAN)
#error Define BYTE_ORDER to be equal to either LITTLE_ENDIAN or BIG_ENDIAN
#endif

/*
 * Define the followingsha2_* types to types of the correct length on
 * the native archtecture.   Most BSD systems and Linux define u_intXX_t
 * types.  Machines with very recent ANSI C headers, can use the
 * uintXX_t definintions from inttypes.h by defining SHA2_USE_INTTYPES_H
 * during compile or in the sha.h header file.
 *
 * Machines that support neither u_intXX_t nor inttypes.h's uintXX_t
 * will need to define these three typedefs below (and the appropriate
 * ones in sha.h too) by hand according to their system architecture.
 *
 * Thank you, Jun-ichiro itojun Hagino, for suggesting using u_intXX_t
 * types and pointing out recent ANSI C support for uintXX_t in inttypes.h.
 */
#ifdef SHA2_USE_INTTYPES_H

typedef unsigned char  sha2_byte;	/* Exactly 1 byte */
typedef unsigned int sha2_word32;	/* Exactly 4 bytes */
typedef unsigned long long sha2_word64;	/* Exactly 8 bytes */

#else /* SHA2_USE_INTTYPES_H */

typedef u_int8_t  sha2_byte;	/* Exactly 1 byte */
typedef u_int32_t sha2_word32;	/* Exactly 4 bytes */
typedef u_int64_t sha2_word64;	/* Exactly 8 bytes */

#endif /* SHA2_USE_INTTYPES_H */


/*** SHA-256/384/512 Various Length Definitions ***********************/
/* NOTE: Most of these are in sha2.h */
#define SHA256_SHORT_BLOCK_LENGTH 			(SHA256_BLOCK_LENGTH - 8)
#define SHA384_SHORT_BLOCK_LENGTH 			(SHA384_BLOCK_LENGTH - 16)
#define SHA512_SHORT_BLOCK_LENGTH 			(SHA512_BLOCK_LENGTH - 16)


/*** ENDIAN REVERSAL MACROS *******************************************/
#if BYTE_ORDER == LITTLE_ENDIAN
#define REVERSE32(w,x)	{ \
	sha2_word32 tmp = (w); \
	tmp = (tmp >> 16) | (tmp << 16); \
	(x) = ((tmp & 0xff00ff00UL) >> 8) | ((tmp & 0x00ff00ffUL) << 8); \
}
#define REVERSE64(w,x)	{ \
	sha2_word64 tmp = (w); \
	tmp = (tmp >> 32) | (tmp << 32); \
	tmp = ((tmp & 0xff00ff00ff00ff00ULL) >> 8) | \
	      ((tmp & 0x00ff00ff00ff00ffULL) << 8); \
	(x) = ((tmp & 0xffff0000ffff0000ULL) >> 16) | \
	      ((tmp & 0x0000ffff0000ffffULL) << 16); \
}
#endif /* BYTE_ORDER == LITTLE_ENDIAN */

/*
 * Macro for incrementally adding the unsigned 64-bit integer n to the
 * unsigned 128-bit integer (represented using a two-element array of
 * 64-bit words):
 */
#define ADDINC128(w,n)	{ \
	(w)[0] += (sha2_word64)(n); \
	if ((w)[0] < (n)) { \
		(w)[1]++; \
	} \
}

/*
 * Macros for copying blocks of memory and for zeroing out ranges
 * of memory.  Using these macros makes it easy to switch from
 * using memset()/memcpy() and using bzero()/bcopy().
 *
 * Please define either SHA2_USE_MEMSET_MEMCPY or define
 * SHA2_USE_BZERO_BCOPY depending on which function set you
 * choose to use:
 */
#if !defined(SHA2_USE_MEMSET_MEMCPY) && !defined(SHA2_USE_BZERO_BCOPY)
/* Default to memset()/memcpy() if no option is specified */
#define	SHA2_USE_MEMSET_MEMCPY	1
#endif
#if defined(SHA2_USE_MEMSET_MEMCPY) && defined(SHA2_USE_BZERO_BCOPY)
/* Abort with an error if BOTH options are defined */
#error Define either SHA2_USE_MEMSET_MEMCPY or SHA2_USE_BZERO_BCOPY, not both!
#endif

#ifdef SHA2_USE_MEMSET_MEMCPY
#define MEMSET_BZERO(p,l)	memset((p), 0, (l))
#define MEMCPY_BCOPY(d,s,l)	memcpy((d), (s), (l))
#endif
#ifdef SHA2_USE_BZERO_BCOPY
#define MEMSET_BZERO(p,l)	bzero((p), (l))
#define MEMCPY_BCOPY(d,s,l)	bcopy((s), (d), (l))
#endif


/*** THE SIX LOGICAL FUNCTIONS ****************************************/
/*
 * Bit shifting and rotation (used by the six SHA-XYZ logical functions:
 *
 *   NOTE:  The naming of R and S appears backwards here (R is a SHIFT and
 *   S is a ROTATION) because the SHA-256/384/512 description document
 *   (see http://csrc.nist.gov/cryptval/shs/sha256-384-512.pdf) uses this
 *   same "backwards" definition.
 */
/* Shift-right (used in SHA-256, SHA-384, and SHA-512): */
#define SHA2_R(b,x) 		((x) >> (b))
/* 32-bit Rotate-right (used in SHA-256): */
#define SHA2_S32(b,x)	(((x) >> (b)) | ((x) << (32 - (b))))
/* 64-bit Rotate-right (used in SHA-384 and SHA-512): */
#define SHA2_S64(b,x)	(((x) >> (b)) | ((x) << (64 - (b))))

/* Two of six logical functions used in SHA-256, SHA-384, and SHA-512: */
#define Ch(x,y,z)	(((x) & (y)) ^ ((~(x)) & (z)))
#define Maj(x,y,z)	(((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))

/* Four of six logical functions used in SHA-256: */
#define Sigma0_256(x)	(SHA2_S32(2,  (x)) ^ SHA2_S32(13, (x)) ^ SHA2_S32(22, (x)))
#define Sigma1_256(x)	(SHA2_S32(6,  (x)) ^ SHA2_S32(11, (x)) ^ SHA2_S32(25, (x)))
#define sigma0_256(x)	(SHA2_S32(7,  (x)) ^ SHA2_S32(18, (x)) ^ SHA2_R(3 ,   (x)))
#define sigma1_256(x)	(SHA2_S32(17, (x)) ^ SHA2_S32(19, (x)) ^ SHA2_R(10,   (x)))

/* Four of six logical functions used in SHA-384 and SHA-512: */
#define Sigma0_512(x)	(SHA2_S64(28, (x)) ^ SHA2_S64(34, (x)) ^ SHA2_S64(39, (x)))
#define Sigma1_512(x)	(SHA2_S64(14, (x)) ^ SHA2_S64(18, (x)) ^ SHA2_S64(41, (x)))
#define sigma0_512(x)	(SHA2_S64( 1, (x)) ^ SHA2_S64( 8, (x)) ^ SHA2_R(7,   (x)))
#define sigma1_512(x)	(SHA2_S64(19, (x)) ^ SHA2_S64(61, (x)) ^ SHA2_R(6,   (x)))

/*** INTERNAL FUNCTION PROTOTYPES *************************************/
/* NOTE: These should not be accessed directly from outside this
 * library -- they are intended for private internal visibility/use
 * only.
 */
void SHA512_Last(SHA512_CTX*);
void SHA256_Transform(SHA256_CTX*, const sha2_word32*);
void SHA512_Transform(SHA512_CTX*, const sha2_word64*);


/*** SHA-XYZ INITIAL HASH VALUES AND CONSTANTS ************************/
/* Hash constant words K for SHA-256: */
const static sha2_word32 K256[64] = {
	0x428a2f98UL, 0x71374491UL, 0xb5c0fbcfUL, 0xe9b5dba5UL,
	0x3956c25bUL, 0x59f111f1UL, 0x923f82a4UL, 0xab1c5ed5UL,
	0xd807aa98UL, 0x12835b01UL, 0x243185beUL, 0x550c7dc3UL,
	0x72be5d74UL, 0x80deb1feUL, 0x9bdc06a7UL, 0xc19bf174UL,
	0xe49b69c1UL, 0xefbe4786UL, 0x0fc19dc6UL, 0x240ca1ccUL,
	0x2de92c6fUL, 0x4a7484aaUL, 0x5cb0a9dcUL, 0x76f988daUL,
	0x983e5152UL, 0xa831c66dUL, 0xb00327c8UL, 0xbf597fc7UL,
	0xc6e00bf3UL, 0xd5a79147UL, 0x06ca6351UL, 0x14292967UL,
	0x27b70a85UL, 0x2e1b2138UL, 0x4d2c6dfcUL, 0x53380d13UL,
	0x650a7354UL, 0x766a0abbUL, 0x81c2c92eUL, 0x92722c85UL,
	0xa2bfe8a1UL, 0xa81a664bUL, 0xc24b8b70UL, 0xc76c51a3UL,
	0xd192e819UL, 0xd6990624UL, 0xf40e3585UL, 0x106aa070UL,
	0x19a4c116UL, 0x1e376c08UL, 0x2748774cUL, 0x34b0bcb5UL,
	0x391c0cb3UL, 0x4ed8aa4aUL, 0x5b9cca4fUL, 0x682e6ff3UL,
	0x748f82eeUL, 0x78a5636fUL, 0x84c87814UL, 0x8cc70208UL,
	0x90befffaUL, 0xa4506cebUL, 0xbef9a3f7UL, 0xc67178f2UL
};

/* Initial hash value H for SHA-256: */
const static sha2_word32 sha256_initial_hash_value[8] = {
	0x6a09e667UL,
	0xbb67ae85UL,
	0x3c6ef372UL,
	0xa54ff53aUL,
	0x510e527fUL,
	0x9b05688cUL,
	0x1f83d9abUL,
	0x5be0cd19UL
};

/* Hash constant words K for SHA-384 and SHA-512: */
const static sha2_word64 K512[80] = {
	0x428a2f98d728ae22ULL, 0x7137449123ef65cdULL,
	0xb5c0fbcfec4d3b2fULL, 0xe9b5dba58189dbbcULL,
	0x3956c25bf348b538ULL, 0x59f111f1b605d019ULL,
	0x923f82a4af194f9bULL, 0xab1c5ed5da6d8118ULL,
	0xd807aa98a3030242ULL, 0x12835b0145706fbeULL,
	0x243185be4ee4b28cULL, 0x550c7dc3d5ffb4e2ULL,
	0x72be5d74f27b896fULL, 0x80deb1fe3b1696b1ULL,
	0x9bdc06a725c71235ULL, 0xc19bf174cf692694ULL,
	0xe49b69c19ef14ad2ULL, 0xefbe4786384f25e3ULL,
	0x0fc19dc68b8cd5b5ULL, 0x240ca1cc77ac9c65ULL,
	0x2de92c6f592b0275ULL, 0x4a7484aa6ea6e483ULL,
	0x5cb0a9dcbd41fbd4ULL, 0x76f988da831153b5ULL,
	0x983e5152ee66dfabULL, 0xa831c66d2db43210ULL,
	0xb00327c898fb213fULL, 0xbf597fc7beef0ee4ULL,
	0xc6e00bf33da88fc2ULL, 0xd5a79147930aa725ULL,
	0x06ca6351e003826fULL, 0x142929670a0e6e70ULL,
	0x27b70a8546d22ffcULL, 0x2e1b21385c26c926ULL,
	0x4d2c6dfc5ac42aedULL, 0x53380d139d95b3dfULL,
	0x650a73548baf63deULL, 0x766a0abb3c77b2a8ULL,
	0x81c2c92e47edaee6ULL, 0x92722c851482353bULL,
	0xa2bfe8a14cf10364ULL, 0xa81a664bbc423001ULL,
	0xc24b8b70d0f89791ULL, 0xc76c51a30654be30ULL,
	0xd192e819d6ef5218ULL, 0xd69906245565a910ULL,
	0xf40e35855771202aULL, 0x106aa07032bbd1b8ULL,
	0x19a4c116b8d2d0c8ULL, 0x1e376c085141ab53ULL,
	0x2748774cdf8eeb99ULL, 0x34b0bcb5e19b48a8ULL,
	0x391c0cb3c5c95a63ULL, 0x4ed8aa4ae3418acbULL,
	0x5b9cca4f7763e373ULL, 0x682e6ff3d6b2b8a3ULL,
	0x748f82ee5defb2fcULL, 0x78a5636f43172f60ULL,
	0x84c87814a1f0ab72ULL, 0x8cc702081a6439ecULL,
	0x90befffa23631e28ULL, 0xa4506cebde82bde9ULL,
	0xbef9a3f7b2c67915ULL, 0xc67178f2e372532bULL,
	0xca273eceea26619cULL, 0xd186b8c721c0c207ULL,
	0xeada7dd6cde0eb1eULL, 0xf57d4f7fee6ed178ULL,
	0x06f067aa72176fbaULL, 0x0a637dc5a2c898a6ULL,
	0x113f9804bef90daeULL, 0x1b710b35131c471bULL,
	0x28db77f523047d84ULL, 0x32caab7b40c72493ULL,
	0x3c9ebe0a15c9bebcULL, 0x431d67c49c100d4cULL,
	0x4cc5d4becb3e42b6ULL, 0x597f299cfc657e2aULL,
	0x5fcb6fab3ad6faecULL, 0x6c44198c4a475817ULL
};

/* Initial hash value H for SHA-384 */
const static sha2_word64 sha384_initial_hash_value[8] = {
	0xcbbb9d5dc1059ed8ULL,
	0x629a292a367cd507ULL,
	0x9159015a3070dd17ULL,
	0x152fecd8f70e5939ULL,
	0x67332667ffc00b31ULL,
	0x8eb44a8768581511ULL,
	0xdb0c2e0d64f98fa7ULL,
	0x47b5481dbefa4fa4ULL
};

/* Initial hash value H for SHA-512 */
const static sha2_word64 sha512_initial_hash_value[8] = {
	0x6a09e667f3bcc908ULL,
	0xbb67ae8584caa73bULL,
	0x3c6ef372fe94f82bULL,
	0xa54ff53a5f1d36f1ULL,
	0x510e527fade682d1ULL,
	0x9b05688c2b3e6c1fULL,
	0x1f83d9abfb41bd6bULL,
	0x5be0cd19137e2179ULL
};

/*
 * Constant used by SHA256/384/512_End() functions for converting the
 * digest to a readable hexadecimal character string:
 */
static const char *sha2_hex_digits = "0123456789abcdef";


/*** SHA-256: *********************************************************/
void SHA256_Init(SHA256_CTX* context) 
{
	if (context == (SHA256_CTX*)0) {
		return;
	}
	MEMCPY_BCOPY(context->state, sha256_initial_hash_value, SHA256_DIGEST_LENGTH);
	MEMSET_BZERO(context->buffer, SHA256_BLOCK_LENGTH);
	context->bitcount = 0;
}

#ifdef SHA2_UNROLL_TRANSFORM

/* Unrolled SHA-256 round macros: */

#if BYTE_ORDER == LITTLE_ENDIAN

#define ROUND256_0_TO_15(a,b,c,d,e,f,g,h)	\
	REVERSE32(*data++, W256[j]); \
	T1 = (h) + Sigma1_256(e) + Ch((e), (f), (g)) + \
             K256[j] + W256[j]; \
	(d) += T1; \
	(h) = T1 + Sigma0_256(a) + Maj((a), (b), (c)); \
	j++


#else /* BYTE_ORDER == LITTLE_ENDIAN */

#define ROUND256_0_TO_15(a,b,c,d,e,f,g,h)	\
	T1 = (h) + Sigma1_256(e) + Ch((e), (f), (g)) + \
	     K256[j] + (W256[j] = *data++); \
	(d) += T1; \
	(h) = T1 + Sigma0_256(a) + Maj((a), (b), (c)); \
	j++

#endif /* BYTE_ORDER == LITTLE_ENDIAN */

#define ROUND256(a,b,c,d,e,f,g,h)	\
	s0 = W256[(j+1)&0x0f]; \
	s0 = sigma0_256(s0); \
	s1 = W256[(j+14)&0x0f]; \
	s1 = sigma1_256(s1); \
	T1 = (h) + Sigma1_256(e) + Ch((e), (f), (g)) + K256[j] + \
	     (W256[j&0x0f] += s1 + W256[(j+9)&0x0f] + s0); \
	(d) += T1; \
	(h) = T1 + Sigma0_256(a) + Maj((a), (b), (c)); \
	j++


void SHA256_Transform(SHA256_CTX* context, const sha2_word32* data) 
{
	sha2_word32	a, b, c, d, e, f, g, h, s0, s1;
	sha2_word32	T1, *W256;
	int		j;

	W256 = (sha2_word32*)context->buffer;

	/* Initialize registers with the prev. intermediate value */
	a = context->state[0];
	b = context->state[1];
	c = context->state[2];
	d = context->state[3];
	e = context->state[4];
	f = context->state[5];
	g = context->state[6];
	h = context->state[7];

	j = 0;
	do {
		/* Rounds 0 to 15 (unrolled): */
		ROUND256_0_TO_15(a,b,c,d,e,f,g,h);
		ROUND256_0_TO_15(h,a,b,c,d,e,f,g);
		ROUND256_0_TO_15(g,h,a,b,c,d,e,f);
		ROUND256_0_TO_15(f,g,h,a,b,c,d,e);
		ROUND256_0_TO_15(e,f,g,h,a,b,c,d);
		ROUND256_0_TO_15(d,e,f,g,h,a,b,c);
		ROUND256_0_TO_15(c,d,e,f,g,h,a,b);
		ROUND256_0_TO_15(b,c,d,e,f,g,h,a);
	} while (j < 16);

	/* Now for the remaining rounds to 64: */
	do {
		ROUND256(a,b,c,d,e,f,g,h);
		ROUND256(h,a,b,c,d,e,f,g);
		ROUND256(g,h,a,b,c,d,e,f);
		ROUND256(f,g,h,a,b,c,d,e);
		ROUND256(e,f,g,h,a,b,c,d);
		ROUND256(d,e,f,g,h,a,b,c);
		ROUND256(c,d,e,f,g,h,a,b);
		ROUND256(b,c,d,e,f,g,h,a);
	} while (j < 64);

	/* Compute the current intermediate hash value */
	context->state[0] += a;
	context->state[1] += b;
	context->state[2] += c;
	context->state[3] += d;
	context->state[4] += e;
	context->state[5] += f;
	context->state[6] += g;
	context->state[7] += h;

	/* Clean up */
	a = b = c = d = e = f = g = h = T1 = 0;
}

#else /* SHA2_UNROLL_TRANSFORM */

void SHA256_Transform(SHA256_CTX* context, const sha2_word32* data) 
{
	sha2_word32	a, b, c, d, e, f, g, h, s0, s1;
	sha2_word32	T1, T2, *W256;
	int		j;

	W256 = (sha2_word32*)context->buffer;

	/* Initialize registers with the prev. intermediate value */
	a = context->state[0];
	b = context->state[1];
	c = context->state[2];
	d = context->state[3];
	e = context->state[4];
	f = context->state[5];
	g = context->state[6];
	h = context->state[7];

	j = 0;
	do {
#if BYTE_ORDER == LITTLE_ENDIAN
		/* Copy data while converting to host byte order */
		REVERSE32(*data++,W256[j]);
		/* Apply the SHA-256 compression function to update a..h */
		T1 = h + Sigma1_256(e) + Ch(e, f, g) + K256[j] + W256[j];
#else /* BYTE_ORDER == LITTLE_ENDIAN */
		/* Apply the SHA-256 compression function to update a..h with copy */
		T1 = h + Sigma1_256(e) + Ch(e, f, g) + K256[j] + (W256[j] = *data++);
#endif /* BYTE_ORDER == LITTLE_ENDIAN */
		T2 = Sigma0_256(a) + Maj(a, b, c);
		h = g;
		g = f;
		f = e;
		e = d + T1;
		d = c;
		c = b;
		b = a;
		a = T1 + T2;

		j++;
	} while (j < 16);

	do {
		/* Part of the message block expansion: */
		s0 = W256[(j+1)&0x0f];
		s0 = sigma0_256(s0);
		s1 = W256[(j+14)&0x0f];	
		s1 = sigma1_256(s1);

		/* Apply the SHA-256 compression function to update a..h */
		T1 = h + Sigma1_256(e) + Ch(e, f, g) + K256[j] + 
		     (W256[j&0x0f] += s1 + W256[(j+9)&0x0f] + s0);
		T2 = Sigma0_256(a) + Maj(a, b, c);
		h = g;
		g = f;
		f = e;
		e = d + T1;
		d = c;
		c = b;
		b = a;
		a = T1 + T2;

		j++;
	} while (j < 64);

	/* Compute the current intermediate hash value */
	context->state[0] += a;
	context->state[1] += b;
	context->state[2] += c;
	context->state[3] += d;
	context->state[4] += e;
	context->state[5] += f;
	context->state[6] += g;
	context->state[7] += h;

	/* Clean up */
	a = b = c = d = e = f = g = h = T1 = T2 = 0;
}

#endif /* SHA2_UNROLL_TRANSFORM */

void SHA256_Update(SHA256_CTX* context, const sha2_byte *data, size_t len) 
{
	unsigned int	freespace, usedspace;

	if (len == 0) {
		/* Calling with no data is valid - we do nothing */
		return;
	}

	/* Sanity check: */
	assert(context != (SHA256_CTX*)0 && data != (sha2_byte*)0);

	usedspace = (context->bitcount >> 3) % SHA256_BLOCK_LENGTH;
	if (usedspace > 0) {
		/* Calculate how much free space is available in the buffer */
		freespace = SHA256_BLOCK_LENGTH - usedspace;

		if (len >= freespace) {
			/* Fill the buffer completely and process it */
			MEMCPY_BCOPY(&context->buffer[usedspace], data, freespace);
			context->bitcount += freespace << 3;
			len -= freespace;
			data += freespace;
			SHA256_Transform(context, (sha2_word32*)context->buffer);
		} else {
			/* The buffer is not yet full */
			MEMCPY_BCOPY(&context->buffer[usedspace], data, len);
			context->bitcount += len << 3;
			/* Clean up: */
			usedspace = freespace = 0;
			return;
		}
	}
	while (len >= SHA256_BLOCK_LENGTH) {
		/* Process as many complete blocks as we can */
		SHA256_Transform(context, (sha2_word32*)data);
		context->bitcount += SHA256_BLOCK_LENGTH << 3;
		len -= SHA256_BLOCK_LENGTH;
		data += SHA256_BLOCK_LENGTH;
	}
	if (len > 0) {
		/* There's left-overs, so save 'em */
		MEMCPY_BCOPY(context->buffer, data, len);
		context->bitcount += len << 3;
	}
	/* Clean up: */
	usedspace = freespace = 0;
}

void SHA256_Final(sha2_byte digest[], SHA256_CTX* context) 
{
	sha2_word32	*d = (sha2_word32*)digest;
	unsigned int	usedspace;

	/* Sanity check: */
	assert(context != (SHA256_CTX*)0);

	/* If no digest buffer is passed, we don't bother doing this: */
	if (digest != (sha2_byte*)0) {
		usedspace = (context->bitcount >> 3) % SHA256_BLOCK_LENGTH;
#if BYTE_ORDER == LITTLE_ENDIAN
		/* Convert FROM host byte order */
		REVERSE64(context->bitcount,context->bitcount);
#endif
		if (usedspace > 0) {
			/* Begin padding with a 1 bit: */
			context->buffer[usedspace++] = 0x80;

			if (usedspace <= SHA256_SHORT_BLOCK_LENGTH) {
				/* Set-up for the last transform: */
				MEMSET_BZERO(&context->buffer[usedspace], SHA256_SHORT_BLOCK_LENGTH - usedspace);
			} else {
				if (usedspace < SHA256_BLOCK_LENGTH) {
					MEMSET_BZERO(&context->buffer[usedspace], SHA256_BLOCK_LENGTH - usedspace);
				}
				/* Do second-to-last transform: */
				SHA256_Transform(context, (sha2_word32*)context->buffer);

				/* And set-up for the last transform: */
				MEMSET_BZERO(context->buffer, SHA256_SHORT_BLOCK_LENGTH);
			}
		} else {
			/* Set-up for the last transform: */
			MEMSET_BZERO(context->buffer, SHA256_SHORT_BLOCK_LENGTH);

			/* Begin padding with a 1 bit: */
			*context->buffer = 0x80;
		}
		/* Set the bit count: */
		*(sha2_word64*)&context->buffer[SHA256_SHORT_BLOCK_LENGTH] = context->bitcount;

		/* Final transform: */
		SHA256_Transform(context, (sha2_word32*)context->buffer);

#if BYTE_ORDER == LITTLE_ENDIAN
		{
			/* Convert TO host byte order */
			int	j;
			for (j = 0; j < 8; j++) {
				REVERSE32(context->state[j],context->state[j]);
				*d++ = context->state[j];
			}
		}
#else
		MEMCPY_BCOPY(d, context->state, SHA256_DIGEST_LENGTH);
#endif
	}

	/* Clean up state data: */
	MEMSET_BZERO(context, sizeof(SHA256_CTX));
	usedspace = 0;
}

char *SHA256_End(SHA256_CTX* context, char buffer[]) 
{
	sha2_byte	digest[SHA256_DIGEST_LENGTH], *d = digest;
	int		i;

	/* Sanity check: */
	assert(context != (SHA256_CTX*)0);

	if (buffer != (char*)0) {
		SHA256_Final(digest, context);

		for (i = 0; i < SHA256_DIGEST_LENGTH; i++) {
			*buffer++ = sha2_hex_digits[(*d & 0xf0) >> 4];
			*buffer++ = sha2_hex_digits[*d & 0x0f];
			d++;
		}
		*buffer = (char)0;
	} else {
		MEMSET_BZERO(context, sizeof(SHA256_CTX));
	}
	MEMSET_BZERO(digest, SHA256_DIGEST_LENGTH);
	return buffer;
}


char* SHA256_Data(const sha2_byte* data, size_t len, char digest[SHA256_DIGEST_STRING_LENGTH]) 
{
	SHA256_CTX	context;

	SHA256_Init(&context);
	SHA256_Update(&context, data, len);
	return SHA256_End(&context, digest);
}


/*** SHA-512: *********************************************************/
void SHA512_Init(SHA512_CTX* context) {
	if (context == (SHA512_CTX*)0) {
		return;
	}
	MEMCPY_BCOPY(context->state, sha512_initial_hash_value, SHA512_DIGEST_LENGTH);
	MEMSET_BZERO(context->buffer, SHA512_BLOCK_LENGTH);
	context->bitcount[0] = context->bitcount[1] =  0;
}

#ifdef SHA2_UNROLL_TRANSFORM

/* Unrolled SHA-512 round macros: */
#if BYTE_ORDER == LITTLE_ENDIAN

#define ROUND512_0_TO_15(a,b,c,d,e,f,g,h)	\
	REVERSE64(*data++, W512[j]); \
	T1 = (h) + Sigma1_512(e) + Ch((e), (f), (g)) + \
             K512[j] + W512[j]; \
	(d) += T1, \
	(h) = T1 + Sigma0_512(a) + Maj((a), (b), (c)), \
	j++


#else /* BYTE_ORDER == LITTLE_ENDIAN */

#define ROUND512_0_TO_15(a,b,c,d,e,f,g,h)	\
	T1 = (h) + Sigma1_512(e) + Ch((e), (f), (g)) + \
             K512[j] + (W512[j] = *data++); \
	(d) += T1; \
	(h) = T1 + Sigma0_512(a) + Maj((a), (b), (c)); \
	j++

#endif /* BYTE_ORDER == LITTLE_ENDIAN */

#define ROUND512(a,b,c,d,e,f,g,h)	\
	s0 = W512[(j+1)&0x0f]; \
	s0 = sigma0_512(s0); \
	s1 = W512[(j+14)&0x0f]; \
	s1 = sigma1_512(s1); \
	T1 = (h) + Sigma1_512(e) + Ch((e), (f), (g)) + K512[j] + \
             (W512[j&0x0f] += s1 + W512[(j+9)&0x0f] + s0); \
	(d) += T1; \
	(h) = T1 + Sigma0_512(a) + Maj((a), (b), (c)); \
	j++



void SHA512_Transform(SHA512_CTX* context, const sha2_word64* data) 
{
	sha2_word64	a, b, c, d, e, f, g, h, s0, s1;
	sha2_word64	T1, *W512 = (sha2_word64*)context->buffer;
	int		j;

	/* Initialize registers with the prev. intermediate value */
	a = context->state[0];
	b = context->state[1];
	c = context->state[2];
	d = context->state[3];
	e = context->state[4];
	f = context->state[5];
	g = context->state[6];
	h = context->state[7];

	j = 0;
	do {
		ROUND512_0_TO_15(a,b,c,d,e,f,g,h);
		ROUND512_0_TO_15(h,a,b,c,d,e,f,g);
		ROUND512_0_TO_15(g,h,a,b,c,d,e,f);
		ROUND512_0_TO_15(f,g,h,a,b,c,d,e);
		ROUND512_0_TO_15(e,f,g,h,a,b,c,d);
		ROUND512_0_TO_15(d,e,f,g,h,a,b,c);
		ROUND512_0_TO_15(c,d,e,f,g,h,a,b);
		ROUND512_0_TO_15(b,c,d,e,f,g,h,a);
	} while (j < 16);

	/* Now for the remaining rounds up to 79: */
	do {
		ROUND512(a,b,c,d,e,f,g,h);
		ROUND512(h,a,b,c,d,e,f,g);
		ROUND512(g,h,a,b,c,d,e,f);
		ROUND512(f,g,h,a,b,c,d,e);
		ROUND512(e,f,g,h,a,b,c,d);
		ROUND512(d,e,f,g,h,a,b,c);
		ROUND512(c,d,e,f,g,h,a,b);
		ROUND512(b,c,d,e,f,g,h,a);
	} while (j < 80);

	/* Compute the current intermediate hash value */
	context->state[0] += a;
	context->state[1] += b;
	context->state[2] += c;
	context->state[3] += d;
	context->state[4] += e;
	context->state[5] += f;
	context->state[6] += g;
	context->state[7] += h;

	/* Clean up */
	a = b = c = d = e = f = g = h = T1 = 0;
}

#else /* SHA2_UNROLL_TRANSFORM */

void SHA512_Transform(SHA512_CTX* context, const sha2_word64* data) 
{
	sha2_word64	a, b, c, d, e, f, g, h, s0, s1;
	sha2_word64	T1, T2, *W512 = (sha2_word64*)context->buffer;
	int		j;

	/* Initialize registers with the prev. intermediate value */
	a = context->state[0];
	b = context->state[1];
	c = context->state[2];
	d = context->state[3];
	e = context->state[4];
	f = context->state[5];
	g = context->state[6];
	h = context->state[7];

	j = 0;
	do {
#if BYTE_ORDER == LITTLE_ENDIAN
		/* Convert TO host byte order */
		REVERSE64(*data++, W512[j]);
		/* Apply the SHA-512 compression function to update a..h */
		T1 = h + Sigma1_512(e) + Ch(e, f, g) + K512[j] + W512[j];
#else /* BYTE_ORDER == LITTLE_ENDIAN */
		/* Apply the SHA-512 compression function to update a..h with copy */
		T1 = h + Sigma1_512(e) + Ch(e, f, g) + K512[j] + (W512[j] = *data++);
#endif /* BYTE_ORDER == LITTLE_ENDIAN */
		T2 = Sigma0_512(a) + Maj(a, b, c);
		h = g;
		g = f;
		f = e;
		e = d + T1;
		d = c;
		c = b;
		b = a;
		a = T1 + T2;

		j++;
	} while (j < 16);

	do {
		/* Part of the message block expansion: */
		s0 = W512[(j+1)&0x0f];
		s0 = sigma0_512(s0);
		s1 = W512[(j+14)&0x0f];
		s1 =  sigma1_512(s1);

		/* Apply the SHA-512 compression function to update a..h */
		T1 = h + Sigma1_512(e) + Ch(e, f, g) + K512[j] +
		     (W512[j&0x0f] += s1 + W512[(j+9)&0x0f] + s0);
		T2 = Sigma0_512(a) + Maj(a, b, c);
		h = g;
		g = f;
		f = e;
		e = d + T1;
		d = c;
		c = b;
		b = a;
		a = T1 + T2;

		j++;
	} while (j < 80);

	/* Compute the current intermediate hash value */
	context->state[0] += a;
	context->state[1] += b;
	context->state[2] += c;
	context->state[3] += d;
	context->state[4] += e;
	context->state[5] += f;
	context->state[6] += g;
	context->state[7] += h;

	/* Clean up */
	a = b = c = d = e = f = g = h = T1 = T2 = 0;
}

#endif /* SHA2_UNROLL_TRANSFORM */

void SHA512_Update(SHA512_CTX* context, const sha2_byte *data, size_t len) 
{
	unsigned int	freespace, usedspace;

	if (len == 0) {
		/* Calling with no data is valid - we do nothing */
		return;
	}

	/* Sanity check: */
	assert(context != (SHA512_CTX*)0 && data != (sha2_byte*)0);

	usedspace = (context->bitcount[0] >> 3) % SHA512_BLOCK_LENGTH;
	if (usedspace > 0) {
		/* Calculate how much free space is available in the buffer */
		freespace = SHA512_BLOCK_LENGTH - usedspace;

		if (len >= freespace) {
			/* Fill the buffer completely and process it */
			MEMCPY_BCOPY(&context->buffer[usedspace], data, freespace);
			ADDINC128(context->bitcount, freespace << 3);
			len -= freespace;
			data += freespace;
			SHA512_Transform(context, (sha2_word64*)context->buffer);
		} else {
			/* The buffer is not yet full */
			MEMCPY_BCOPY(&context->buffer[usedspace], data, len);
			ADDINC128(context->bitcount, len << 3);
			/* Clean up: */
			usedspace = freespace = 0;
			return;
		}
	}
	while (len >= SHA512_BLOCK_LENGTH) {
		/* Process as many complete blocks as we can */
		SHA512_Transform(context, (sha2_word64*)data);
		ADDINC128(context->bitcount, SHA512_BLOCK_LENGTH << 3);
		len -= SHA512_BLOCK_LENGTH;
		data += SHA512_BLOCK_LENGTH;
	}
	if (len > 0) {
		/* There's left-overs, so save 'em */
		MEMCPY_BCOPY(context->buffer, data, len);
		ADDINC128(context->bitcount, len << 3);
	}
	/* Clean up: */
	usedspace = freespace = 0;
}

void SHA512_Last(SHA512_CTX* context) 
{
	unsigned int	usedspace;

	usedspace = (context->bitcount[0] >> 3) % SHA512_BLOCK_LENGTH;
#if BYTE_ORDER == LITTLE_ENDIAN
	/* Convert FROM host byte order */
	REVERSE64(context->bitcount[0],context->bitcount[0]);
	REVERSE64(context->bitcount[1],context->bitcount[1]);
#endif
	if (usedspace > 0) {
		/* Begin padding with a 1 bit: */
		context->buffer[usedspace++] = 0x80;

		if (usedspace <= SHA512_SHORT_BLOCK_LENGTH) {
			/* Set-up for the last transform: */
			MEMSET_BZERO(&context->buffer[usedspace], SHA512_SHORT_BLOCK_LENGTH - usedspace);
		} else {
			if (usedspace < SHA512_BLOCK_LENGTH) {
				MEMSET_BZERO(&context->buffer[usedspace], SHA512_BLOCK_LENGTH - usedspace);
			}
			/* Do second-to-last transform: */
			SHA512_Transform(context, (sha2_word64*)context->buffer);

			/* And set-up for the last transform: */
			MEMSET_BZERO(context->buffer, SHA512_BLOCK_LENGTH - 2);
		}
	} else {
		/* Prepare for final transform: */
		MEMSET_BZERO(context->buffer, SHA512_SHORT_BLOCK_LENGTH);

		/* Begin padding with a 1 bit: */
		*context->buffer = 0x80;
	}
	/* Store the length of input data (in bits): */
	*(sha2_word64*)&context->buffer[SHA512_SHORT_BLOCK_LENGTH] = context->bitcount[1];
	*(sha2_word64*)&context->buffer[SHA512_SHORT_BLOCK_LENGTH+8] = context->bitcount[0];

	/* Final transform: */
	SHA512_Transform(context, (sha2_word64*)context->buffer);
}

void SHA512_Final(sha2_byte digest[], SHA512_CTX* context) 
{
	sha2_word64	*d = (sha2_word64*)digest;

	/* Sanity check: */
	assert(context != (SHA512_CTX*)0);

	/* If no digest buffer is passed, we don't bother doing this: */
	if (digest != (sha2_byte*)0) {
		SHA512_Last(context);

		/* Save the hash data for output: */
#if BYTE_ORDER == LITTLE_ENDIAN
		{
			/* Convert TO host byte order */
			int	j;
			for (j = 0; j < 8; j++) {
				REVERSE64(context->state[j],context->state[j]);
				*d++ = context->state[j];
			}
		}
#else
		MEMCPY_BCOPY(d, context->state, SHA512_DIGEST_LENGTH);
#endif
	}

	/* Zero out state data */
	MEMSET_BZERO(context, sizeof(SHA512_CTX));
}

char *SHA512_End(SHA512_CTX* context, char buffer[]) 
{
	sha2_byte	digest[SHA512_DIGEST_LENGTH], *d = digest;
	int		i;

	/* Sanity check: */
	assert(context != (SHA512_CTX*)0);

	if (buffer != (char*)0) {
		SHA512_Final(digest, context);

		for (i = 0; i < SHA512_DIGEST_LENGTH; i++) {
			*buffer++ = sha2_hex_digits[(*d & 0xf0) >> 4];
			*buffer++ = sha2_hex_digits[*d & 0x0f];
			d++;
		}
		*buffer = (char)0;
	} else {
		MEMSET_BZERO(context, sizeof(SHA512_CTX));
	}
	MEMSET_BZERO(digest, SHA512_DIGEST_LENGTH);
	return buffer;
}

char* SHA512_Data(const sha2_byte* data, size_t len, char digest[SHA512_DIGEST_STRING_LENGTH]) 
{
	SHA512_CTX	context;

	SHA512_Init(&context);
	SHA512_Update(&context, data, len);
	return SHA512_End(&context, digest);
}


/*** SHA-384: *********************************************************/
void SHA384_Init(SHA384_CTX* context) 
{
	if (context == (SHA384_CTX*)0) {
		return;
	}
	MEMCPY_BCOPY(context->state, sha384_initial_hash_value, SHA512_DIGEST_LENGTH);
	MEMSET_BZERO(context->buffer, SHA384_BLOCK_LENGTH);
	context->bitcount[0] = context->bitcount[1] = 0;
}

void SHA384_Update(SHA384_CTX* context, const sha2_byte* data, size_t len) 
{
	SHA512_Update((SHA512_CTX*)context, data, len);
}

void SHA384_Final(sha2_byte digest[], SHA384_CTX* context) 
{
	sha2_word64	*d = (sha2_word64*)digest;

	/* Sanity check: */
	assert(context != (SHA384_CTX*)0);

	/* If no digest buffer is passed, we don't bother doing this: */
	if (digest != (sha2_byte*)0) {
		SHA512_Last((SHA512_CTX*)context);

		/* Save the hash data for output: */
#if BYTE_ORDER == LITTLE_ENDIAN
		{
			/* Convert TO host byte order */
			int	j;
			for (j = 0; j < 6; j++) {
				REVERSE64(context->state[j],context->state[j]);
				*d++ = context->state[j];
			}
		}
#else
		MEMCPY_BCOPY(d, context->state, SHA384_DIGEST_LENGTH);
#endif
	}

	/* Zero out state data */
	MEMSET_BZERO(context, sizeof(SHA384_CTX));
}

char *SHA384_End(SHA384_CTX* context, char buffer[]) 
{
	sha2_byte	digest[SHA384_DIGEST_LENGTH], *d = digest;
	int		i;

	/* Sanity check: */
	assert(context != (SHA384_CTX*)0);

	if (buffer != (char*)0) {
		SHA384_Final(digest, context);

		for (i = 0; i < SHA384_DIGEST_LENGTH; i++) {
			*buffer++ = sha2_hex_digits[(*d & 0xf0) >> 4];
			*buffer++ = sha2_hex_digits[*d & 0x0f];
			d++;
		}
		*buffer = (char)0;
	} else {
		MEMSET_BZERO(context, sizeof(SHA384_CTX));
	}
	MEMSET_BZERO(digest, SHA384_DIGEST_LENGTH);
	return buffer;
}

char* SHA384_Data(const sha2_byte* data, size_t len, char digest[SHA384_DIGEST_STRING_LENGTH]) 
{
	SHA384_CTX	context;

	SHA384_Init(&context);
	SHA384_Update(&context, data, len);
	return SHA384_End(&context, digest);
}
#endif /// SHA2_256_384_512



#if SHA3_KECCAK_224_256_384_512

#define KECCAK_SPONGE_BIT		1600
#define KECCAK_ROUND			24
#define KECCAK_STATE_SIZE		200

#define KECCAK_SHA3_224			224
#define KECCAK_SHA3_256			256
#define KECCAK_SHA3_384			384
#define KECCAK_SHA3_512			512

#define KECCAK_SHAKE128			128
#define KECCAK_SHAKE256			256

#define KECCAK_SHA3_SUFFIX		0x06
#define KECCAK_SHAKE_SUFFIX		0x1F


#define SHA3_224_HASH_BIT 		224
#define SHA3_256_HASH_BIT 		256
#define SHA3_384_HASH_BIT 		384
#define SHA3_512_HASH_BIT 		512

#define SHA3_OUT_LENGTH_DIV 	8

#define SHA3_OUT_224 			(SHA3_224_HASH_BIT / SHA3_OUT_LENGTH_DIV)
#define SHA3_OUT_256 			(SHA3_256_HASH_BIT / SHA3_OUT_LENGTH_DIV)
#define SHA3_OUT_384 			(SHA3_384_HASH_BIT / SHA3_OUT_LENGTH_DIV)
#define SHA3_OUT_512 			(SHA3_512_HASH_BIT / SHA3_OUT_LENGTH_DIV)

#define SHAKE_OUT_128 			(256/8)
#define SHAKE_OUT_256 			(512/8)
#define SHAKE_128_HASH_BIT 		128
#define SHAKE_256_HASH_BIT 		256

#define SHA3_OUTPUT_SIZ 		512

#define SHA3_BUFLEN 			(100*1024) // (1024*16)


typedef enum
{
	SHA3_OK = 0,
	SHA3_PARAMETER_ERROR = 1,
} SHA3_RETRUN;


typedef enum
{
	SHA3_SHAKE_NONE = 0,
	SHA3_SHAKE_USE = 1,
} SHA3_USE_SHAKE;



static unsigned int keccakRate = 0;
static unsigned int keccakCapacity = 0;
static unsigned int keccakSuffix = 0;

static unsigned char keccak_state[KECCAK_STATE_SIZE] = { 0x00, };
static int end_offset;

static const unsigned int keccakf_rndc[KECCAK_ROUND][2] =
{
	{0x00000001, 0x00000000}, {0x00008082, 0x00000000},
	{0x0000808a, 0x80000000}, {0x80008000, 0x80000000},
	{0x0000808b, 0x00000000}, {0x80000001, 0x00000000},
	{0x80008081, 0x80000000}, {0x00008009, 0x80000000},
	{0x0000008a, 0x00000000}, {0x00000088, 0x00000000},
	{0x80008009, 0x00000000}, {0x8000000a, 0x00000000},

	{0x8000808b, 0x00000000}, {0x0000008b, 0x80000000},
	{0x00008089, 0x80000000}, {0x00008003, 0x80000000},
	{0x00008002, 0x80000000}, {0x00000080, 0x80000000},
	{0x0000800a, 0x00000000}, {0x8000000a, 0x80000000},
	{0x80008081, 0x80000000}, {0x00008080, 0x80000000},
	{0x80000001, 0x00000000}, {0x80008008, 0x80000000}
};

static const unsigned keccakf_rotc[KECCAK_ROUND] =
{
	 1,  3,  6, 10, 15, 21, 28, 36, 45, 55,  2, 14,
	27, 41, 56,  8, 25, 43, 62, 18, 39, 61, 20, 44
};

static const unsigned keccakf_piln[KECCAK_ROUND] =
{
	10,  7, 11, 17, 18,  3,  5, 16,  8, 21, 24,  4,
	15, 23, 19, 13, 12,  2, 20, 14, 22,  9,  6,  1
};


void ROL64(unsigned int* in, unsigned int* out, int offset)
{
	int shift = 0;

	if (offset == 0)
	{
		out[1] = in[1];
		out[0] = in[0];
	}
	else if (offset < 32)
	{
		shift = offset;

		out[1] = (unsigned int)((in[1] << shift) ^ (in[0] >> (32 - shift)));
		out[0] = (unsigned int)((in[0] << shift) ^ (in[1] >> (32 - shift)));
	}
	else if (offset < 64)
	{
		shift = offset - 32;

		out[1] = (unsigned int)((in[0] << shift) ^ (in[1] >> (32 - shift)));
		out[0] = (unsigned int)((in[1] << shift) ^ (in[0] >> (32 - shift)));
	}
	else
	{
		out[1] = in[1];
		out[0] = in[0];
	}
}


void keccakf(unsigned char* state)
{
	unsigned int t[2], bc[5][2], s[25][2] = { 0x00, };
	int i, j, round;

	for (i = 0; i < 25; i++)
	{
		s[i][0] = (unsigned int)(state[i * 8 + 0]) |
			(unsigned int)(state[i * 8 + 1] << 8) |
			(unsigned int)(state[i * 8 + 2] << 16) |
			(unsigned int)(state[i * 8 + 3] << 24);
		s[i][1] = (unsigned int)(state[i * 8 + 4]) |
			(unsigned int)(state[i * 8 + 5] << 8) |
			(unsigned int)(state[i * 8 + 6] << 16) |
			(unsigned int)(state[i * 8 + 7] << 24);
	}

	for (round = 0; round < KECCAK_ROUND; round++)
	{
		/* Theta */
		for (i = 0; i < 5; i++)
		{
			bc[i][0] = s[i][0] ^ s[i + 5][0] ^ s[i + 10][0] ^ s[i + 15][0] ^ s[i + 20][0];
			bc[i][1] = s[i][1] ^ s[i + 5][1] ^ s[i + 10][1] ^ s[i + 15][1] ^ s[i + 20][1];
		}

		for (i = 0; i < 5; i++)
		{
			ROL64(bc[(i + 1) % 5], t, 1);

			t[0] ^= bc[(i + 4) % 5][0];
			t[1] ^= bc[(i + 4) % 5][1];

			for (j = 0; j < 25; j += 5)
			{
				s[j + i][0] ^= t[0];
				s[j + i][1] ^= t[1];
			}
		}

		/* Rho & Pi */
		t[0] = s[1][0];
		t[1] = s[1][1];

		for (i = 0; i < KECCAK_ROUND; i++)
		{
			j = keccakf_piln[i];

			bc[0][0] = s[j][0];
			bc[0][1] = s[j][1];

			ROL64(t, s[j], keccakf_rotc[i]);

			t[0] = bc[0][0];
			t[1] = bc[0][1];
		}

		/* Chi */
		for (j = 0; j < 25; j += 5)
		{
			for (i = 0; i < 5; i++)
			{
				bc[i][0] = s[j + i][0];
				bc[i][1] = s[j + i][1];
			}

			for (i = 0; i < 5; i++)
			{
				s[j + i][0] ^= (~bc[(i + 1) % 5][0]) & bc[(i + 2) % 5][0];
				s[j + i][1] ^= (~bc[(i + 1) % 5][1]) & bc[(i + 2) % 5][1];
			}
		}

		/* Iota */
		s[0][0] ^= keccakf_rndc[round][0];
		s[0][1] ^= keccakf_rndc[round][1];
	}

	for (i = 0; i < 25; i++)
	{
		state[i * 8 + 0] = (unsigned char)(s[i][0]);
		state[i * 8 + 1] = (unsigned char)(s[i][0] >> 8);
		state[i * 8 + 2] = (unsigned char)(s[i][0] >> 16);
		state[i * 8 + 3] = (unsigned char)(s[i][0] >> 24);
		state[i * 8 + 4] = (unsigned char)(s[i][1]);
		state[i * 8 + 5] = (unsigned char)(s[i][1] >> 8);
		state[i * 8 + 6] = (unsigned char)(s[i][1] >> 16);
		state[i * 8 + 7] = (unsigned char)(s[i][1] >> 24);
	}
}


int keccak_absorb(unsigned char* input, int inLen, int rate, int capacity)
{
	unsigned char* buf = input;
	int iLen = inLen;
	int rateInBytes = rate / 8;
	int blockSize = 0;
	int i = 0;

	if ((rate + capacity) != KECCAK_SPONGE_BIT)
		return SHA3_PARAMETER_ERROR;

	if (((rate % 8) != 0) || (rate < 1))
		return SHA3_PARAMETER_ERROR;

	while (iLen > 0)
	{
		if ((end_offset != 0) && (end_offset < rateInBytes))
		{
			blockSize = (((iLen + end_offset) < rateInBytes) ? (iLen + end_offset) : rateInBytes);

			for (i = end_offset; i < blockSize; i++)
				keccak_state[i] ^= buf[i - end_offset];

			buf += blockSize - end_offset;
			iLen -= blockSize - end_offset;
		}
		else
		{
			blockSize = ((iLen < rateInBytes) ? iLen : rateInBytes);

			for (i = 0; i < blockSize; i++)
				keccak_state[i] ^= buf[i];

			buf += blockSize;
			iLen -= blockSize;
		}

		if (blockSize == rateInBytes)
		{
			keccakf(keccak_state);
			blockSize = 0;
		}

		end_offset = blockSize;
	}

	return SHA3_OK;
}


int keccak_squeeze(unsigned char* output, int outLen, int rate, int suffix)
{
	unsigned char* buf = output;
	int oLen = outLen;
	int rateInBytes = rate / 8;
	int blockSize = end_offset;
	int i = 0;

	keccak_state[blockSize] ^= suffix;

	if (((suffix & 0x80) != 0) && (blockSize == (rateInBytes - 1)))
		keccakf(keccak_state);

	keccak_state[rateInBytes - 1] ^= 0x80;

	keccakf(keccak_state);

	while (oLen > 0)
	{
		blockSize = ((oLen < rateInBytes) ? oLen : rateInBytes);
		for (i = 0; i < blockSize; i++)
			buf[i] = keccak_state[i];
		buf += blockSize;
		oLen -= blockSize;

		if (oLen > 0)
			keccakf(keccak_state);
	}

	return SHA3_OK;
}


void sha3_init(int bitSize, int useSHAKE)
{
	keccakCapacity = bitSize * 2;
	keccakRate = KECCAK_SPONGE_BIT - keccakCapacity;

	if (useSHAKE)
		keccakSuffix = KECCAK_SHAKE_SUFFIX;
	else
		keccakSuffix = KECCAK_SHA3_SUFFIX;

	memset(keccak_state, 0x00, KECCAK_STATE_SIZE);

	end_offset = 0;
}


int sha3_update(unsigned char* input, int inLen)
{
	return keccak_absorb(input, inLen, keccakRate, keccakCapacity);
}


int sha3_final(unsigned char* output, int outLen)
{
	int ret = 0;

	ret = keccak_squeeze(output, outLen, keccakRate, keccakSuffix);

	keccakRate = 0;
	keccakCapacity = 0;
	keccakSuffix = 0;

	memset(keccak_state, 0x00, KECCAK_STATE_SIZE);

	return ret;
}


int sha3_hash(unsigned char* output, int outLen, unsigned char* input, int inLen, int bitSize, int useSHAKE)
{
	int ret = 0;

	if (useSHAKE == SHA3_SHAKE_USE)
	{
		if ((bitSize != KECCAK_SHAKE128) && (bitSize != KECCAK_SHAKE256))
			return SHA3_PARAMETER_ERROR;

		sha3_init(bitSize, SHA3_SHAKE_USE);
	}
	else
	{
		if ((bitSize != KECCAK_SHA3_224) && (bitSize != KECCAK_SHA3_256) &&
			(bitSize != KECCAK_SHA3_384) && (bitSize != KECCAK_SHA3_512))
			return SHA3_PARAMETER_ERROR;

		if ((bitSize / 8) != outLen)
			return SHA3_PARAMETER_ERROR;

		sha3_init(bitSize, SHA3_SHAKE_NONE);
	}

	sha3_update(input, inLen);

	ret = sha3_final(output, outLen);

	return ret;
}
#endif // SHA3_KECCAK_224_256_384_512





#if CRC_CHECKSUM

/* Redis uses the CRC64 variant with "Jones" coefficients and init value of 0.
 *
 * Specification of this CRC64 variant follows:
 * Name: crc-64-jones
 * Width: 64 bites
 * Poly: 0xad93d23594c935a9
 * Reflected In: True
 * Xor_In: 0xffffffffffffffff
 * Reflected_Out: True
 * Xor_Out: 0x0
 * Check("123456789"): 0xe9c6d914c4b8d9ca
 *
 * Copyright (c) 2012, Salvatore Sanfilippo <antirez at gmail dot com>
 * All rights reserved.
 *
 */


static const unsigned long long crc64_tab[256] = {
    0x0000000000000000ULL, 0x7ad870c830358979ULL,
    0xf5b0e190606b12f2ULL, 0x8f689158505e9b8bULL,
    0xc038e5739841b68fULL, 0xbae095bba8743ff6ULL,
    0x358804e3f82aa47dULL, 0x4f50742bc81f2d04ULL,
    0xab28ecb46814fe75ULL, 0xd1f09c7c5821770cULL,
    0x5e980d24087fec87ULL, 0x24407dec384a65feULL,
    0x6b1009c7f05548faULL, 0x11c8790fc060c183ULL,
    0x9ea0e857903e5a08ULL, 0xe478989fa00bd371ULL,
    0x7d08ff3b88be6f81ULL, 0x07d08ff3b88be6f8ULL,
    0x88b81eabe8d57d73ULL, 0xf2606e63d8e0f40aULL,
    0xbd301a4810ffd90eULL, 0xc7e86a8020ca5077ULL,
    0x4880fbd87094cbfcULL, 0x32588b1040a14285ULL,
    0xd620138fe0aa91f4ULL, 0xacf86347d09f188dULL,
    0x2390f21f80c18306ULL, 0x594882d7b0f40a7fULL,
    0x1618f6fc78eb277bULL, 0x6cc0863448deae02ULL,
    0xe3a8176c18803589ULL, 0x997067a428b5bcf0ULL,
    0xfa11fe77117cdf02ULL, 0x80c98ebf2149567bULL,
    0x0fa11fe77117cdf0ULL, 0x75796f2f41224489ULL,
    0x3a291b04893d698dULL, 0x40f16bccb908e0f4ULL,
    0xcf99fa94e9567b7fULL, 0xb5418a5cd963f206ULL,
    0x513912c379682177ULL, 0x2be1620b495da80eULL,
    0xa489f35319033385ULL, 0xde51839b2936bafcULL,
    0x9101f7b0e12997f8ULL, 0xebd98778d11c1e81ULL,
    0x64b116208142850aULL, 0x1e6966e8b1770c73ULL,
    0x8719014c99c2b083ULL, 0xfdc17184a9f739faULL,
    0x72a9e0dcf9a9a271ULL, 0x08719014c99c2b08ULL,
    0x4721e43f0183060cULL, 0x3df994f731b68f75ULL,
    0xb29105af61e814feULL, 0xc849756751dd9d87ULL,
    0x2c31edf8f1d64ef6ULL, 0x56e99d30c1e3c78fULL,
    0xd9810c6891bd5c04ULL, 0xa3597ca0a188d57dULL,
    0xec09088b6997f879ULL, 0x96d1784359a27100ULL,
    0x19b9e91b09fcea8bULL, 0x636199d339c963f2ULL,
    0xdf7adabd7a6e2d6fULL, 0xa5a2aa754a5ba416ULL,
    0x2aca3b2d1a053f9dULL, 0x50124be52a30b6e4ULL,
    0x1f423fcee22f9be0ULL, 0x659a4f06d21a1299ULL,
    0xeaf2de5e82448912ULL, 0x902aae96b271006bULL,
    0x74523609127ad31aULL, 0x0e8a46c1224f5a63ULL,
    0x81e2d7997211c1e8ULL, 0xfb3aa75142244891ULL,
    0xb46ad37a8a3b6595ULL, 0xceb2a3b2ba0eececULL,
    0x41da32eaea507767ULL, 0x3b024222da65fe1eULL,
    0xa2722586f2d042eeULL, 0xd8aa554ec2e5cb97ULL,
    0x57c2c41692bb501cULL, 0x2d1ab4dea28ed965ULL,
    0x624ac0f56a91f461ULL, 0x1892b03d5aa47d18ULL,
    0x97fa21650afae693ULL, 0xed2251ad3acf6feaULL,
    0x095ac9329ac4bc9bULL, 0x7382b9faaaf135e2ULL,
    0xfcea28a2faafae69ULL, 0x8632586aca9a2710ULL,
    0xc9622c4102850a14ULL, 0xb3ba5c8932b0836dULL,
    0x3cd2cdd162ee18e6ULL, 0x460abd1952db919fULL,
    0x256b24ca6b12f26dULL, 0x5fb354025b277b14ULL,
    0xd0dbc55a0b79e09fULL, 0xaa03b5923b4c69e6ULL,
    0xe553c1b9f35344e2ULL, 0x9f8bb171c366cd9bULL,
    0x10e3202993385610ULL, 0x6a3b50e1a30ddf69ULL,
    0x8e43c87e03060c18ULL, 0xf49bb8b633338561ULL,
    0x7bf329ee636d1eeaULL, 0x012b592653589793ULL,
    0x4e7b2d0d9b47ba97ULL, 0x34a35dc5ab7233eeULL,
    0xbbcbcc9dfb2ca865ULL, 0xc113bc55cb19211cULL,
    0x5863dbf1e3ac9decULL, 0x22bbab39d3991495ULL,
    0xadd33a6183c78f1eULL, 0xd70b4aa9b3f20667ULL,
    0x985b3e827bed2b63ULL, 0xe2834e4a4bd8a21aULL,
    0x6debdf121b863991ULL, 0x1733afda2bb3b0e8ULL,
    0xf34b37458bb86399ULL, 0x8993478dbb8deae0ULL,
    0x06fbd6d5ebd3716bULL, 0x7c23a61ddbe6f812ULL,
    0x3373d23613f9d516ULL, 0x49aba2fe23cc5c6fULL,
    0xc6c333a67392c7e4ULL, 0xbc1b436e43a74e9dULL,
    0x95ac9329ac4bc9b5ULL, 0xef74e3e19c7e40ccULL,
    0x601c72b9cc20db47ULL, 0x1ac40271fc15523eULL,
    0x5594765a340a7f3aULL, 0x2f4c0692043ff643ULL,
    0xa02497ca54616dc8ULL, 0xdafce7026454e4b1ULL,
    0x3e847f9dc45f37c0ULL, 0x445c0f55f46abeb9ULL,
    0xcb349e0da4342532ULL, 0xb1eceec59401ac4bULL,
    0xfebc9aee5c1e814fULL, 0x8464ea266c2b0836ULL,
    0x0b0c7b7e3c7593bdULL, 0x71d40bb60c401ac4ULL,
    0xe8a46c1224f5a634ULL, 0x927c1cda14c02f4dULL,
    0x1d148d82449eb4c6ULL, 0x67ccfd4a74ab3dbfULL,
    0x289c8961bcb410bbULL, 0x5244f9a98c8199c2ULL,
    0xdd2c68f1dcdf0249ULL, 0xa7f41839ecea8b30ULL,
    0x438c80a64ce15841ULL, 0x3954f06e7cd4d138ULL,
    0xb63c61362c8a4ab3ULL, 0xcce411fe1cbfc3caULL,
    0x83b465d5d4a0eeceULL, 0xf96c151de49567b7ULL,
    0x76048445b4cbfc3cULL, 0x0cdcf48d84fe7545ULL,
    0x6fbd6d5ebd3716b7ULL, 0x15651d968d029fceULL,
    0x9a0d8ccedd5c0445ULL, 0xe0d5fc06ed698d3cULL,
    0xaf85882d2576a038ULL, 0xd55df8e515432941ULL,
    0x5a3569bd451db2caULL, 0x20ed197575283bb3ULL,
    0xc49581ead523e8c2ULL, 0xbe4df122e51661bbULL,
    0x3125607ab548fa30ULL, 0x4bfd10b2857d7349ULL,
    0x04ad64994d625e4dULL, 0x7e7514517d57d734ULL,
    0xf11d85092d094cbfULL, 0x8bc5f5c11d3cc5c6ULL,
    0x12b5926535897936ULL, 0x686de2ad05bcf04fULL,
    0xe70573f555e26bc4ULL, 0x9ddd033d65d7e2bdULL,
    0xd28d7716adc8cfb9ULL, 0xa85507de9dfd46c0ULL,
    0x273d9686cda3dd4bULL, 0x5de5e64efd965432ULL,
    0xb99d7ed15d9d8743ULL, 0xc3450e196da80e3aULL,
    0x4c2d9f413df695b1ULL, 0x36f5ef890dc31cc8ULL,
    0x79a59ba2c5dc31ccULL, 0x037deb6af5e9b8b5ULL,
    0x8c157a32a5b7233eULL, 0xf6cd0afa9582aa47ULL,
    0x4ad64994d625e4daULL, 0x300e395ce6106da3ULL,
    0xbf66a804b64ef628ULL, 0xc5bed8cc867b7f51ULL,
    0x8aeeace74e645255ULL, 0xf036dc2f7e51db2cULL,
    0x7f5e4d772e0f40a7ULL, 0x05863dbf1e3ac9deULL,
    0xe1fea520be311aafULL, 0x9b26d5e88e0493d6ULL,
    0x144e44b0de5a085dULL, 0x6e963478ee6f8124ULL,
    0x21c640532670ac20ULL, 0x5b1e309b16452559ULL,
    0xd476a1c3461bbed2ULL, 0xaeaed10b762e37abULL,
    0x37deb6af5e9b8b5bULL, 0x4d06c6676eae0222ULL,
    0xc26e573f3ef099a9ULL, 0xb8b627f70ec510d0ULL,
    0xf7e653dcc6da3dd4ULL, 0x8d3e2314f6efb4adULL,
    0x0256b24ca6b12f26ULL, 0x788ec2849684a65fULL,
    0x9cf65a1b368f752eULL, 0xe62e2ad306bafc57ULL,
    0x6946bb8b56e467dcULL, 0x139ecb4366d1eea5ULL,
    0x5ccebf68aecec3a1ULL, 0x2616cfa09efb4ad8ULL,
    0xa97e5ef8cea5d153ULL, 0xd3a62e30fe90582aULL,
    0xb0c7b7e3c7593bd8ULL, 0xca1fc72bf76cb2a1ULL,
    0x45775673a732292aULL, 0x3faf26bb9707a053ULL,
    0x70ff52905f188d57ULL, 0x0a2722586f2d042eULL,
    0x854fb3003f739fa5ULL, 0xff97c3c80f4616dcULL,
    0x1bef5b57af4dc5adULL, 0x61372b9f9f784cd4ULL,
    0xee5fbac7cf26d75fULL, 0x9487ca0fff135e26ULL,
    0xdbd7be24370c7322ULL, 0xa10fceec0739fa5bULL,
    0x2e675fb4576761d0ULL, 0x54bf2f7c6752e8a9ULL,
    0xcdcf48d84fe75459ULL, 0xb71738107fd2dd20ULL,
    0x387fa9482f8c46abULL, 0x42a7d9801fb9cfd2ULL,
    0x0df7adabd7a6e2d6ULL, 0x772fdd63e7936bafULL,
    0xf8474c3bb7cdf024ULL, 0x829f3cf387f8795dULL,
    0x66e7a46c27f3aa2cULL, 0x1c3fd4a417c62355ULL,
    0x935745fc4798b8deULL, 0xe98f353477ad31a7ULL,
    0xa6df411fbfb21ca3ULL, 0xdc0731d78f8795daULL,
    0x536fa08fdfd90e51ULL, 0x29b7d047efec8728ULL,
};


unsigned long long make_crc64(unsigned long long crc, const unsigned char *s, unsigned long long l) 
{
	unsigned long long j;

	for (j = 0; j < l; j++) 
	{
		unsigned char byte = s[j];
		crc = crc64_tab[(unsigned char)crc ^ byte] ^ (crc >> 8);
	}
	return crc;
}


/* Compute CRC-64 in the manner of xz, using the ECMA-182 polynomial,
   bit-reversed, with one's complement pre and post processing.  Provide a
   means to combine separately computed CRC-64's. 

   0x42F0E1EBA9EA3693 or 0xC96C5795D7870F42 (0xA17870F5D4F51B49)
   -------------------------------------------------------------------------- */

#define CRC64_POLY_ECMA_182 			(0xc96c5795d7870f42ULL)



#define CRC30_CDMA 						(0x38E74301)

#if 1
/*
 * Copyright (C) 2013  Internet Systems Consortium, Inc. ("ISC")
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND ISC DISCLAIMS ALL WARRANTIES WITH
 * REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS.  IN NO EVENT SHALL ISC BE LIABLE FOR ANY SPECIAL, DIRECT,
 * INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE
 * OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */


/*%<
 * ECMA-182 CRC64 polynomial.
 */
static const unsigned long long crc64_tab_isc[256] = {
	0x0000000000000000ULL, 0x42F0E1EBA9EA3693ULL, 0x85E1C3D753D46D26ULL,
	0xC711223CFA3E5BB5ULL, 0x493366450E42ECDFULL, 0x0BC387AEA7A8DA4CULL,
	0xCCD2A5925D9681F9ULL, 0x8E224479F47CB76AULL, 0x9266CC8A1C85D9BEULL,
	0xD0962D61B56FEF2DULL, 0x17870F5D4F51B498ULL, 0x5577EEB6E6BB820BULL,
	0xDB55AACF12C73561ULL, 0x99A54B24BB2D03F2ULL, 0x5EB4691841135847ULL,
	0x1C4488F3E8F96ED4ULL, 0x663D78FF90E185EFULL, 0x24CD9914390BB37CULL,
	0xE3DCBB28C335E8C9ULL, 0xA12C5AC36ADFDE5AULL, 0x2F0E1EBA9EA36930ULL,
	0x6DFEFF5137495FA3ULL, 0xAAEFDD6DCD770416ULL, 0xE81F3C86649D3285ULL,
	0xF45BB4758C645C51ULL, 0xB6AB559E258E6AC2ULL, 0x71BA77A2DFB03177ULL,
	0x334A9649765A07E4ULL, 0xBD68D2308226B08EULL, 0xFF9833DB2BCC861DULL,
	0x388911E7D1F2DDA8ULL, 0x7A79F00C7818EB3BULL, 0xCC7AF1FF21C30BDEULL,
	0x8E8A101488293D4DULL, 0x499B3228721766F8ULL, 0x0B6BD3C3DBFD506BULL,
	0x854997BA2F81E701ULL, 0xC7B97651866BD192ULL, 0x00A8546D7C558A27ULL,
	0x4258B586D5BFBCB4ULL, 0x5E1C3D753D46D260ULL, 0x1CECDC9E94ACE4F3ULL,
	0xDBFDFEA26E92BF46ULL, 0x990D1F49C77889D5ULL, 0x172F5B3033043EBFULL,
	0x55DFBADB9AEE082CULL, 0x92CE98E760D05399ULL, 0xD03E790CC93A650AULL,
	0xAA478900B1228E31ULL, 0xE8B768EB18C8B8A2ULL, 0x2FA64AD7E2F6E317ULL,
	0x6D56AB3C4B1CD584ULL, 0xE374EF45BF6062EEULL, 0xA1840EAE168A547DULL,
	0x66952C92ECB40FC8ULL, 0x2465CD79455E395BULL, 0x3821458AADA7578FULL,
	0x7AD1A461044D611CULL, 0xBDC0865DFE733AA9ULL, 0xFF3067B657990C3AULL,
	0x711223CFA3E5BB50ULL, 0x33E2C2240A0F8DC3ULL, 0xF4F3E018F031D676ULL,
	0xB60301F359DBE0E5ULL, 0xDA050215EA6C212FULL, 0x98F5E3FE438617BCULL,
	0x5FE4C1C2B9B84C09ULL, 0x1D14202910527A9AULL, 0x93366450E42ECDF0ULL,
	0xD1C685BB4DC4FB63ULL, 0x16D7A787B7FAA0D6ULL, 0x5427466C1E109645ULL,
	0x4863CE9FF6E9F891ULL, 0x0A932F745F03CE02ULL, 0xCD820D48A53D95B7ULL,
	0x8F72ECA30CD7A324ULL, 0x0150A8DAF8AB144EULL, 0x43A04931514122DDULL,
	0x84B16B0DAB7F7968ULL, 0xC6418AE602954FFBULL, 0xBC387AEA7A8DA4C0ULL,
	0xFEC89B01D3679253ULL, 0x39D9B93D2959C9E6ULL, 0x7B2958D680B3FF75ULL,
	0xF50B1CAF74CF481FULL, 0xB7FBFD44DD257E8CULL, 0x70EADF78271B2539ULL,
	0x321A3E938EF113AAULL, 0x2E5EB66066087D7EULL, 0x6CAE578BCFE24BEDULL,
	0xABBF75B735DC1058ULL, 0xE94F945C9C3626CBULL, 0x676DD025684A91A1ULL,
	0x259D31CEC1A0A732ULL, 0xE28C13F23B9EFC87ULL, 0xA07CF2199274CA14ULL,
	0x167FF3EACBAF2AF1ULL, 0x548F120162451C62ULL, 0x939E303D987B47D7ULL,
	0xD16ED1D631917144ULL, 0x5F4C95AFC5EDC62EULL, 0x1DBC74446C07F0BDULL,
	0xDAAD56789639AB08ULL, 0x985DB7933FD39D9BULL, 0x84193F60D72AF34FULL,
	0xC6E9DE8B7EC0C5DCULL, 0x01F8FCB784FE9E69ULL, 0x43081D5C2D14A8FAULL,
	0xCD2A5925D9681F90ULL, 0x8FDAB8CE70822903ULL, 0x48CB9AF28ABC72B6ULL,
	0x0A3B7B1923564425ULL, 0x70428B155B4EAF1EULL, 0x32B26AFEF2A4998DULL,
	0xF5A348C2089AC238ULL, 0xB753A929A170F4ABULL, 0x3971ED50550C43C1ULL,
	0x7B810CBBFCE67552ULL, 0xBC902E8706D82EE7ULL, 0xFE60CF6CAF321874ULL,
	0xE224479F47CB76A0ULL, 0xA0D4A674EE214033ULL, 0x67C58448141F1B86ULL,
	0x253565A3BDF52D15ULL, 0xAB1721DA49899A7FULL, 0xE9E7C031E063ACECULL,
	0x2EF6E20D1A5DF759ULL, 0x6C0603E6B3B7C1CAULL, 0xF6FAE5C07D3274CDULL,
	0xB40A042BD4D8425EULL, 0x731B26172EE619EBULL, 0x31EBC7FC870C2F78ULL,
	0xBFC9838573709812ULL, 0xFD39626EDA9AAE81ULL, 0x3A28405220A4F534ULL,
	0x78D8A1B9894EC3A7ULL, 0x649C294A61B7AD73ULL, 0x266CC8A1C85D9BE0ULL,
	0xE17DEA9D3263C055ULL, 0xA38D0B769B89F6C6ULL, 0x2DAF4F0F6FF541ACULL,
	0x6F5FAEE4C61F773FULL, 0xA84E8CD83C212C8AULL, 0xEABE6D3395CB1A19ULL,
	0x90C79D3FEDD3F122ULL, 0xD2377CD44439C7B1ULL, 0x15265EE8BE079C04ULL,
	0x57D6BF0317EDAA97ULL, 0xD9F4FB7AE3911DFDULL, 0x9B041A914A7B2B6EULL,
	0x5C1538ADB04570DBULL, 0x1EE5D94619AF4648ULL, 0x02A151B5F156289CULL,
	0x4051B05E58BC1E0FULL, 0x87409262A28245BAULL, 0xC5B073890B687329ULL,
	0x4B9237F0FF14C443ULL, 0x0962D61B56FEF2D0ULL, 0xCE73F427ACC0A965ULL,
	0x8C8315CC052A9FF6ULL, 0x3A80143F5CF17F13ULL, 0x7870F5D4F51B4980ULL,
	0xBF61D7E80F251235ULL, 0xFD913603A6CF24A6ULL, 0x73B3727A52B393CCULL,
	0x31439391FB59A55FULL, 0xF652B1AD0167FEEAULL, 0xB4A25046A88DC879ULL,
	0xA8E6D8B54074A6ADULL, 0xEA16395EE99E903EULL, 0x2D071B6213A0CB8BULL,
	0x6FF7FA89BA4AFD18ULL, 0xE1D5BEF04E364A72ULL, 0xA3255F1BE7DC7CE1ULL,
	0x64347D271DE22754ULL, 0x26C49CCCB40811C7ULL, 0x5CBD6CC0CC10FAFCULL,
	0x1E4D8D2B65FACC6FULL, 0xD95CAF179FC497DAULL, 0x9BAC4EFC362EA149ULL,
	0x158E0A85C2521623ULL, 0x577EEB6E6BB820B0ULL, 0x906FC95291867B05ULL,
	0xD29F28B9386C4D96ULL, 0xCEDBA04AD0952342ULL, 0x8C2B41A1797F15D1ULL,
	0x4B3A639D83414E64ULL, 0x09CA82762AAB78F7ULL, 0x87E8C60FDED7CF9DULL,
	0xC51827E4773DF90EULL, 0x020905D88D03A2BBULL, 0x40F9E43324E99428ULL,
	0x2CFFE7D5975E55E2ULL, 0x6E0F063E3EB46371ULL, 0xA91E2402C48A38C4ULL,
	0xEBEEC5E96D600E57ULL, 0x65CC8190991CB93DULL, 0x273C607B30F68FAEULL,
	0xE02D4247CAC8D41BULL, 0xA2DDA3AC6322E288ULL, 0xBE992B5F8BDB8C5CULL,
	0xFC69CAB42231BACFULL, 0x3B78E888D80FE17AULL, 0x7988096371E5D7E9ULL,
	0xF7AA4D1A85996083ULL, 0xB55AACF12C735610ULL, 0x724B8ECDD64D0DA5ULL,
	0x30BB6F267FA73B36ULL, 0x4AC29F2A07BFD00DULL, 0x08327EC1AE55E69EULL,
	0xCF235CFD546BBD2BULL, 0x8DD3BD16FD818BB8ULL, 0x03F1F96F09FD3CD2ULL,
	0x41011884A0170A41ULL, 0x86103AB85A2951F4ULL, 0xC4E0DB53F3C36767ULL,
	0xD8A453A01B3A09B3ULL, 0x9A54B24BB2D03F20ULL, 0x5D45907748EE6495ULL,
	0x1FB5719CE1045206ULL, 0x919735E51578E56CULL, 0xD367D40EBC92D3FFULL,
	0x1476F63246AC884AULL, 0x568617D9EF46BED9ULL, 0xE085162AB69D5E3CULL,
	0xA275F7C11F7768AFULL, 0x6564D5FDE549331AULL, 0x279434164CA30589ULL,
	0xA9B6706FB8DFB2E3ULL, 0xEB46918411358470ULL, 0x2C57B3B8EB0BDFC5ULL,
	0x6EA7525342E1E956ULL, 0x72E3DAA0AA188782ULL, 0x30133B4B03F2B111ULL,
	0xF7021977F9CCEAA4ULL, 0xB5F2F89C5026DC37ULL, 0x3BD0BCE5A45A6B5DULL,
	0x79205D0E0DB05DCEULL, 0xBE317F32F78E067BULL, 0xFCC19ED95E6430E8ULL,
	0x86B86ED5267CDBD3ULL, 0xC4488F3E8F96ED40ULL, 0x0359AD0275A8B6F5ULL,
	0x41A94CE9DC428066ULL, 0xCF8B0890283E370CULL, 0x8D7BE97B81D4019FULL,
	0x4A6ACB477BEA5A2AULL, 0x089A2AACD2006CB9ULL, 0x14DEA25F3AF9026DULL,
	0x562E43B4931334FEULL, 0x913F6188692D6F4BULL, 0xD3CF8063C0C759D8ULL,
	0x5DEDC41A34BBEEB2ULL, 0x1F1D25F19D51D821ULL,
	0xD80C07CD676F8394ULL, 0x9AFCE626CE85B507ULL
};

unsigned long long isc_crc64_init() 
{
	unsigned long long crc;

	crc = 0xffffffffffffffffULL;
	return crc;
}


unsigned long long isc_crc64_update(unsigned long long crc, const void *data, size_t len) 
{
	const unsigned char *p = data;
	int i;

	if( (NULL==data) )
	{
		printf("\n ERROR isc_crc64_update is erro ++ \n\n");
		return 0L;
	}

	while (len-- > 0U) 
	{
		i = ((int) (crc >> 56) ^ *p++) & 0xff;
		crc = crc64_tab_isc[i] ^ (crc << 8);
	}
	return crc;
}


unsigned long long isc_crc64_final(unsigned long long crc) 
{
	unsigned long long calcCRC;

	calcCRC = crc ^ 0xffffffffffffffffULL;

	return calcCRC;
}
#endif



#define check_init(a, b)


#define POLY_UINT64_C 					(0xad93d23594c935a9ULL)
/******************** BEGIN GENERATED PYCRC FUNCTIONS ********************/
/**
 * Generated on Sun Dec 21 14:14:07 2014,
 * by pycrc v0.8.2, https://www.tty1.net/pycrc/
 *
 * LICENSE ON GENERATED CODE:
 * ==========================
 * As of version 0.6, pycrc is released under the terms of the MIT licence.
 * The code generated by pycrc is not considered a substantial portion of the
 * software, therefore the author of pycrc will not claim any copyright on
 * the generated code.
 * ==========================
 *
 * CRC configuration:
 *    Width        = 64
 *    Poly         = 0xad93d23594c935a9
 *    XorIn        = 0xffffffffffffffff
 *    ReflectIn    = True
 *    XorOut       = 0x0000000000000000
 *    ReflectOut   = True
 *    Algorithm    = bit-by-bit-fast
 *
 * Modifications after generation (by matt):
 *   - included finalize step in-line with update for single-pass generation
 *   - adjusted function parameters to match expected prototypes.
 *****************************************************************************/

/**
 * Reflect all bits of a \a data word of \a data_len bytes.
 *
 * \param data         The data word to be reflected.
 * \param data_len     The width of \a data expressed in number of bits.
 * \return             The reflected data.
 *****************************************************************************/
static inline unsigned long long crc_reflect(unsigned long long data, size_t data_len) {
    unsigned int i;
    unsigned long long ret;

    ret = data & 0x01;
    for (i = 1; i < data_len; i++) {
        data >>= 1;
        ret = (ret << 1) | (data & 0x01);
    }
    return ret;
}

/**
 *  Update the crc value with new data.
 *
 * \param crc      The current crc value.
 * \param data     Pointer to a buffer of \a data_len bytes.
 * \param data_len Number of bytes in the \a data buffer.
 * \return         The updated crc value.
 ******************************************************************************/
unsigned long long crc64(unsigned long long crc, void *in_data, unsigned long long data_len) 
{
    unsigned char *data = in_data;
    unsigned int i;
    bool bit;
    unsigned char c;

    while (data_len--) {
        c = *data++;
        for (i = 0x01; i & 0xff; i <<= 1) {
            bit = crc & 0x8000000000000000ULL;
            if (c & i) {
                bit = !bit;
            }
            crc <<= 1;
            if (bit) {
                crc ^= POLY_UINT64_C;
            }
        }
        crc &= 0xffffffffffffffffULL;
    }
    crc = crc & 0xffffffffffffffffULL;
    return crc_reflect(crc, 64) ^ 0x0000000000000000ULL;
}
/******************** END GENERATED PYCRC FUNCTIONS ********************/



/*-
 *
 *  The polynomial itself and its table of feedback terms.  The
 *  polynomial is
 *  X^32+X^26+X^23+X^22+X^16+X^12+X^11+X^10+X^8+X^7+X^5+X^4+X^2+X^1+X^0
 *
 *  Note that we take it "backwards" and put the highest-order term in
 *  the lowest-order bit.  The X^32 term is "implied"; the LSB is the
 *  X^31 term, etc.  The X^0 term (usually shown as "+1") results in
 *  the MSB being 1
 *
 * CRC32 code derived from work by Gary S. Brown.
 *
 * HDLC, ANSI X3.66, ITU-T V.42, Ethernet, Serial ATA, MPEG-2, PKZIP, Gzip, Bzip2, PNG,[14] many others
 *
 */

#define CRC32_POLYNOMIAL 			(0xEDB88320)
#define CRC32_TAB_SIZE 				256

#if 1
void makeCRCtable(unsigned long *table, unsigned long id) 
{
	unsigned long i, j, k;

	for(i = 0; i < CRC32_TAB_SIZE; ++i) 
	{
		k = i;
		for(j = 0; j < 8; ++j) 
		{
			if (k & 1) k = (k >> 1) ^ id;
			else k >>= 1;
		}
		table[i] = k;
	}
}


unsigned long calcCRC32(const unsigned char *mem, signed long size, unsigned long CRC) 
{
	unsigned long table[CRC32_TAB_SIZE];

	CRC = ~CRC;
	makeCRCtable(table, CRC32_POLYNOMIAL);

	while(size--)
		CRC = table[(CRC ^ *(mem++)) & 0xFF] ^ (CRC >> 8);

	return ~CRC;
}
#endif


const unsigned int crc32_tab[] = {
	0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419, 0x706af48f,
	0xe963a535, 0x9e6495a3,	0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988,
	0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91, 0x1db71064, 0x6ab020f2,
	0xf3b97148, 0x84be41de,	0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
	0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec,	0x14015c4f, 0x63066cd9,
	0xfa0f3d63, 0x8d080df5,	0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172,
	0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b,	0x35b5a8fa, 0x42b2986c,
	0xdbbbc9d6, 0xacbcf940,	0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
	0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423,
	0xcfba9599, 0xb8bda50f, 0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,
	0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d,	0x76dc4190, 0x01db7106,
	0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
	0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818, 0x7f6a0dbb, 0x086d3d2d,
	0x91646c97, 0xe6635c01, 0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e,
	0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457, 0x65b0d9c6, 0x12b7e950,
	0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
	0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2, 0x4adfa541, 0x3dd895d7,
	0xa4d1c46d, 0xd3d6f4fb, 0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0,
	0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9, 0x5005713c, 0x270241aa,
	0xbe0b1010, 0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
	0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17, 0x2eb40d81,
	0xb7bd5c3b, 0xc0ba6cad, 0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a,
	0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683, 0xe3630b12, 0x94643b84,
	0x0d6d6a3e, 0x7a6a5aa8, 0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
	0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb,
	0x196c3671, 0x6e6b06e7, 0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc,
	0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5, 0xd6d6a3e8, 0xa1d1937e,
	0x38d8c2c4, 0x4fdff252, 0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
	0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55,
	0x316e8eef, 0x4669be79, 0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
	0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f, 0xc5ba3bbe, 0xb2bd0b28,
	0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
	0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a, 0x9c0906a9, 0xeb0e363f,
	0x72076785, 0x05005713, 0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38,
	0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21, 0x86d3d2d4, 0xf1d4e242,
	0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
	0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c, 0x8f659eff, 0xf862ae69,
	0x616bffd3, 0x166ccf45, 0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2,
	0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db, 0xaed16a4a, 0xd9d65adc,
	0x40df0b66, 0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
	0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605, 0xcdd70693,
	0x54de5729, 0x23d967bf, 0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94,
	0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d
};

void makeCRC32table(unsigned int *table, unsigned int id) 
{
	unsigned int i, j, k;

	for(i = 0; i < CRC32_TAB_SIZE; ++i) 
	{
		k = i;
		for(j = 0; j < 8; ++j) 
		{
			if (k & 1) k = (k >> 1) ^ id;
			else k >>= 1;
		}
		table[i] = k;
	}
}
//// -----------------------------------------------------
//// -----------------------------------------------------
//// -----------------------------------------------------

unsigned int make_crc32(unsigned int crc, const void *buf, size_t size)
{
	const unsigned char *p;

	p = buf;
	crc = crc ^ ~0U;

	while (size--)
		crc = crc32_tab[(crc ^ *p++) & 0xFF] ^ (crc >> 8);

	return crc ^ ~0U;
}





/*===========================================================================
FUNCTION 
 make_crc16()
DESCRIPTION
       - Buf  : CRC를 생성하고자하는 Data + 2byte(crc)
       - size : Buf Size - 2 
       - CRC Generation (polynomial : x^16 + x^12 + x^5 + 1)
       - KS X ISO 18234-2 Appendix C
DEPENDENCIES
  
RETURN VALUE

SIDE EFFECTS
  None
===========================================================================*/
#if 1
//---------------------------------------------------------------------------
// 표준 CRC 테이블
unsigned int KSCcrc16Tbl[256] = {
	0x0000, 0xC0C1, 0xC181, 0x0140, 0xC301, 0x03C0, 0x0280, 0xC241, 
	0xC601, 0x06C0, 0x0780,	0xC741, 0x0500, 0xC5C1, 0xC481, 0x0440, 
	0xCC01, 0x0CC0, 0x0D80, 0xCD41, 0x0F00, 0xCFC1,	0xCE81, 0x0E40, 
	0x0A00, 0xCAC1, 0xCB81, 0x0B40, 0xC901, 0x09C0, 0x0880, 0xC841, 
	0xD801,	0x18C0, 0x1980, 0xD941, 0x1B00, 0xDBC1, 0xDA81, 0x1A40, 
	0x1E00, 0xDEC1, 0xDF81, 0x1F40,	0xDD01, 0x1DC0, 0x1C80, 0xDC41, 
	0x1400, 0xD4C1, 0xD581, 0x1540, 0xD701, 0x17C0, 0x1680,	0xD641, 
	0xD201, 0x12C0, 0x1380, 0xD341, 0x1100, 0xD1C1, 0xD081, 0x1040, 
	0xF001, 0x30C0,	0x3180, 0xF141, 0x3300, 0xF3C1, 0xF281, 0x3240, 
	0x3600, 0xF6C1, 0xF781, 0x3740, 0xF501,	0x35C0, 0x3480, 0xF441, 
	0x3C00, 0xFCC1, 0xFD81, 0x3D40, 0xFF01, 0x3FC0, 0x3E80, 0xFE41,
	0xFA01, 0x3AC0, 0x3B80, 0xFB41, 0x3900, 0xF9C1, 0xF881, 0x3840, 
	0x2800, 0xE8C1, 0xE981,	0x2940, 0xEB01, 0x2BC0, 0x2A80, 0xEA41, 
	0xEE01, 0x2EC0, 0x2F80, 0xEF41, 0x2D00, 0xEDC1,	0xEC81, 0x2C40, 
	0xE401, 0x24C0, 0x2580, 0xE541, 0x2700, 0xE7C1, 0xE681, 0x2640, 
	0x2200,	0xE2C1, 0xE381, 0x2340, 0xE101, 0x21C0, 0x2080, 0xE041, 
	0xA001, 0x60C0, 0x6180, 0xA141,	0x6300, 0xA3C1, 0xA281, 0x6240, 
	0x6600, 0xA6C1, 0xA781, 0x6740, 0xA501, 0x65C0, 0x6480,	0xA441, 
	0x6C00, 0xACC1, 0xAD81, 0x6D40, 0xAF01, 0x6FC0, 0x6E80, 0xAE41, 
	0xAA01, 0x6AC0,	0x6B80, 0xAB41, 0x6900, 0xA9C1, 0xA881, 0x6840, 
	0x7800, 0xB8C1, 0xB981, 0x7940, 0xBB01,	0x7BC0, 0x7A80, 0xBA41, 
	0xBE01, 0x7EC0, 0x7F80, 0xBF41, 0x7D00, 0xBDC1, 0xBC81, 0x7C40,
	0xB401, 0x74C0, 0x7580, 0xB541, 0x7700, 0xB7C1, 0xB681, 0x7640, 
	0x7200, 0xB2C1, 0xB381,	0x7340, 0xB101, 0x71C0, 0x7080, 0xB041, 
	0x5000, 0x90C1, 0x9181, 0x5140, 0x9301, 0x53C0,	0x5280, 0x9241, 
	0x9601, 0x56C0, 0x5780, 0x9741, 0x5500, 0x95C1, 0x9481, 0x5440, 
	0x9C01,	0x5CC0, 0x5D80, 0x9D41, 0x5F00, 0x9FC1, 0x9E81, 0x5E40, 
	0x5A00, 0x9AC1, 0x9B81, 0x5B40,	0x9901, 0x59C0, 0x5880, 0x9841, 
	0x8801, 0x48C0, 0x4980, 0x8941, 0x4B00, 0x8BC1, 0x8A81,	0x4A40, 
	0x4E00, 0x8EC1, 0x8F81, 0x4F40, 0x8D01, 0x4DC0, 0x4C80, 0x8C41, 
	0x4400, 0x84C1,	0x8581, 0x4540, 0x8701, 0x47C0, 0x4680, 0x8641, 
	0x8201, 0x42C0, 0x4380, 0x8341, 0x4100,	0x81C1, 0x8081, 0x4040
};

//===========================================================================
// CRC 체크 표준
//===========================================================================
// KS초안_KSCIEC62056-61 에 정의된 표준 CRC 16 계산 테이블 입니다.
// 유용하게 사용하세요.

//---------------------------------------------------------------------------
// 계산식
//#define SCrcCal(data) SCRC = ( (SCRC>>8) ^ KSCcrc16Tbl[ (SCRC^data) & 0xFF ] )
//U16 SCRC = 0xFFFF;
unsigned short make_ksc_crc16(unsigned short crc_seed, unsigned char *c_ptr, unsigned int len) 
{
	unsigned short crc = crc_seed;
	unsigned int index = 0;

	index = 0;
	while (len--)
	{

		crc = ( (crc>>8) ^ KSCcrc16Tbl[ (crc^c_ptr[index]) & 0xFF ] );
		index++;
	}
	return (crc);
}
#endif


unsigned int crctable[256] =
{
	0x0000, 0x1189, 0x2312, 0x329B, 0x4624, 0x57AD, 0x6536, 0x74BF,
	0x8C48, 0x9DC1, 0xAF5A, 0xBED3, 0xCA6C, 0xDBE5, 0xE97E, 0xF8F7,
	0x0919, 0x1890, 0x2A0B, 0x3B82, 0x4F3D, 0x5EB4, 0x6C2F, 0x7DA6,
	0x8551, 0x94D8, 0xA643, 0xB7CA, 0xC375, 0xD2FC, 0xE067, 0xF1EE,
	0x1232, 0x03BB, 0x3120, 0x20A9, 0x5416, 0x459F, 0x7704, 0x668D,
	0x9E7A, 0x8FF3, 0xBD68, 0xACE1, 0xD85E, 0xC9D7, 0xFB4C, 0xEAC5,
	0x1B2B, 0x0AA2, 0x3839, 0x29B0, 0x5D0F, 0x4C86, 0x7E1D, 0x6F94,
	0x9763, 0x86EA, 0xB471, 0xA5F8, 0xD147, 0xC0CE, 0xF255, 0xE3DC,
	0x2464, 0x35ED, 0x0776, 0x16FF, 0x6240, 0x73C9, 0x4152, 0x50DB,
	0xA82C, 0xB9A5, 0x8B3E, 0x9AB7, 0xEE08, 0xFF81, 0xCD1A, 0xDC93,
	0x2D7D, 0x3CF4, 0x0E6F, 0x1FE6, 0x6B59, 0x7AD0, 0x484B, 0x59C2,
	0xA135, 0xB0BC, 0x8227, 0x93AE, 0xE711, 0xF698, 0xC403, 0xD58A,
	0x3656, 0x27DF, 0x1544, 0x04CD, 0x7072, 0x61FB, 0x5360, 0x42E9,
	0xBA1E, 0xAB97, 0x990C, 0x8885, 0xFC3A, 0xEDB3, 0xDF28, 0xCEA1,
	0x3F4F, 0x2EC6, 0x1C5D, 0x0DD4, 0x796B, 0x68E2, 0x5A79, 0x4BF0,
	0xB307, 0xA28E, 0x9015, 0x819C, 0xF523, 0xE4AA, 0xD631, 0xC7B8,
	0x48C8, 0x5941, 0x6BDA, 0x7A53, 0x0EEC, 0x1F65, 0x2DFE, 0x3C77,
	0xC480, 0xD509, 0xE792, 0xF61B, 0x82A4, 0x932D, 0xA1B6, 0xB03F,
	0x41D1, 0x5058, 0x62C3, 0x734A, 0x07F5, 0x167C, 0x24E7, 0x356E,
	0xCD99, 0xDC10, 0xEE8B, 0xFF02, 0x8BBD, 0x9A34, 0xA8AF, 0xB926,
	0x5AFA, 0x4B73, 0x79E8, 0x6861, 0x1CDE, 0x0D57, 0x3FCC, 0x2E45,
	0xD6B2, 0xC73B, 0xF5A0, 0xE429, 0x9096, 0x811F, 0xB384, 0xA20D,
	0x53E3, 0x426A, 0x70F1, 0x6178, 0x15C7, 0x044E, 0x36D5, 0x275C,
	0xDFAB, 0xCE22, 0xFCB9, 0xED30, 0x998F, 0x8806, 0xBA9D, 0xAB14,
	0x6CAC, 0x7D25, 0x4FBE, 0x5E37, 0x2A88, 0x3B01, 0x099A, 0x1813,
	0xE0E4, 0xF16D, 0xC3F6, 0xD27F, 0xA6C0, 0xB749, 0x85D2, 0x945B,
	0x65B5, 0x743C, 0x46A7, 0x572E, 0x2391, 0x3218, 0x0083, 0x110A,
	0xE9FD, 0xF874, 0xCAEF, 0xDB66, 0xAFD9, 0xBE50, 0x8CCB, 0x9D42,
	0x7E9E, 0x6F17, 0x5D8C, 0x4C05, 0x38BA, 0x2933, 0x1BA8, 0x0A21,
	0xF2D6, 0xE35F, 0xD1C4, 0xC04D, 0xB4F2, 0xA57B, 0x97E0, 0x8669,
	0x7787, 0x660E, 0x5495, 0x451C, 0x31A3, 0x202A, 0x12B1, 0x0338,
	0xFBCF, 0xEA46, 0xD8DD, 0xC954, 0xBDEB, 0xAC62, 0x9EF9, 0x8F70
};

unsigned short make_crc16(unsigned short crc_seed, unsigned char *c_ptr, unsigned int len) 
{
	unsigned short crc = crc_seed;
	unsigned int index = 0;

	index = 0;
	while (len--)
	{
		//crc = (crc << 8) ^ crctable[((crc >> 8) ^ *cdata++)];
		crc = (crc << 8) ^ crctable[( (crc >> 8) ^ c_ptr[index] )];
		index++;
		//printf("%d", crc);
	}
	return (crc);
}



/*	
 * Copyright 2001-2010 Georges Menie (www.menie.org)
 * All rights reserved.
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 */

/* CRC16 implementation acording to CCITT standards */

const unsigned short crc16_tab_ccitt[256]= {
	0x0000,0x1021,0x2042,0x3063,0x4084,0x50a5,0x60c6,0x70e7,
	0x8108,0x9129,0xa14a,0xb16b,0xc18c,0xd1ad,0xe1ce,0xf1ef,
	0x1231,0x0210,0x3273,0x2252,0x52b5,0x4294,0x72f7,0x62d6,
	0x9339,0x8318,0xb37b,0xa35a,0xd3bd,0xc39c,0xf3ff,0xe3de,
	0x2462,0x3443,0x0420,0x1401,0x64e6,0x74c7,0x44a4,0x5485,
	0xa56a,0xb54b,0x8528,0x9509,0xe5ee,0xf5cf,0xc5ac,0xd58d,
	0x3653,0x2672,0x1611,0x0630,0x76d7,0x66f6,0x5695,0x46b4,
	0xb75b,0xa77a,0x9719,0x8738,0xf7df,0xe7fe,0xd79d,0xc7bc,
	0x48c4,0x58e5,0x6886,0x78a7,0x0840,0x1861,0x2802,0x3823,
	0xc9cc,0xd9ed,0xe98e,0xf9af,0x8948,0x9969,0xa90a,0xb92b,
	0x5af5,0x4ad4,0x7ab7,0x6a96,0x1a71,0x0a50,0x3a33,0x2a12,
	0xdbfd,0xcbdc,0xfbbf,0xeb9e,0x9b79,0x8b58,0xbb3b,0xab1a,
	0x6ca6,0x7c87,0x4ce4,0x5cc5,0x2c22,0x3c03,0x0c60,0x1c41,
	0xedae,0xfd8f,0xcdec,0xddcd,0xad2a,0xbd0b,0x8d68,0x9d49,
	0x7e97,0x6eb6,0x5ed5,0x4ef4,0x3e13,0x2e32,0x1e51,0x0e70,
	0xff9f,0xefbe,0xdfdd,0xcffc,0xbf1b,0xaf3a,0x9f59,0x8f78,
	0x9188,0x81a9,0xb1ca,0xa1eb,0xd10c,0xc12d,0xf14e,0xe16f,
	0x1080,0x00a1,0x30c2,0x20e3,0x5004,0x4025,0x7046,0x6067,
	0x83b9,0x9398,0xa3fb,0xb3da,0xc33d,0xd31c,0xe37f,0xf35e,
	0x02b1,0x1290,0x22f3,0x32d2,0x4235,0x5214,0x6277,0x7256,
	0xb5ea,0xa5cb,0x95a8,0x8589,0xf56e,0xe54f,0xd52c,0xc50d,
	0x34e2,0x24c3,0x14a0,0x0481,0x7466,0x6447,0x5424,0x4405,
	0xa7db,0xb7fa,0x8799,0x97b8,0xe75f,0xf77e,0xc71d,0xd73c,
	0x26d3,0x36f2,0x0691,0x16b0,0x6657,0x7676,0x4615,0x5634,
	0xd94c,0xc96d,0xf90e,0xe92f,0x99c8,0x89e9,0xb98a,0xa9ab,
	0x5844,0x4865,0x7806,0x6827,0x18c0,0x08e1,0x3882,0x28a3,
	0xcb7d,0xdb5c,0xeb3f,0xfb1e,0x8bf9,0x9bd8,0xabbb,0xbb9a,
	0x4a75,0x5a54,0x6a37,0x7a16,0x0af1,0x1ad0,0x2ab3,0x3a92,
	0xfd2e,0xed0f,0xdd6c,0xcd4d,0xbdaa,0xad8b,0x9de8,0x8dc9,
	0x7c26,0x6c07,0x5c64,0x4c45,0x3ca2,0x2c83,0x1ce0,0x0cc1,
	0xef1f,0xff3e,0xcf5d,0xdf7c,0xaf9b,0xbfba,0x8fd9,0x9ff8,
	0x6e17,0x7e36,0x4e55,0x5e74,0x2e93,0x3eb2,0x0ed1,0x1ef0
};
  
unsigned short make_crc16_ccitt(unsigned short crc, const void *buf, int len)
{
	int counter;
	///unsigned short crc = 0;
	for( counter = 0; counter < len; counter++)
		crc = (crc<<8) ^ crc16_tab_ccitt[((crc>>8) ^ *(char *)buf++)&0x00FF];

	return crc;
}





/* adler32.c -- compute the Adler-32 checksum of a data stream
 * Copyright (C) 1995-1998 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h 
 */

#define BASE 			65521L /* largest prime smaller than 65536 */
#define NMAX 			5552
/* NMAX is the largest n such that 255n(n+1)/2 + (n+1)(BASE-1) <= 2^32-1 */

#define DO1(buf,i)  {s1 += buf[i]; s2 += s1;}
#define DO2(buf,i)  {DO1(buf,i); DO1(buf,i+1);}
#define DO4(buf,i)  {DO2(buf,i); DO2(buf,i+2);}
#define DO8(buf,i)  {DO4(buf,i); DO4(buf,i+4);}
#define DO16(buf)   {DO8(buf,0); DO8(buf,8);}



unsigned int make_adler32(unsigned int adler, const unsigned char *buf, unsigned int len)
{
	unsigned int s1 = adler & 0xffff;
	unsigned int s2 = (adler >> 16) & 0xffff;
	int k;

	if( NULL==buf ) return 1L;

    while (len > 0) 
	{
		k = len < NMAX ? len : NMAX;
		len -= k;
		while (k >= 16) 
		{
			DO16(buf);
			buf += 16;
			k -= 16;
        }
        if (k != 0) do {
			s1 += *buf++;
			s2 += s1;
		} while (--k);
		s1 %= BASE;
		s2 %= BASE;
	}
	return (s2 << 16) | s1;
}


unsigned int joaat_hash(unsigned int joaat_hash, unsigned char *key, size_t len)
{
    unsigned int hash = joaat_hash; // initial value must be 0x00
    size_t i;

    for (i = 0; i < len; i++)
    {
        hash += key[i];
        hash += (hash << 10);
        hash ^= (hash >> 6);
    }
    hash += (hash << 3);
    hash ^= (hash >> 11);
    hash += (hash << 15);
    return hash;
}



/*===========================================================================
FUNCTION 
 checkCrc()

DESCRIPTION

DEPENDENCIES
  
RETURN VALUE

SIDE EFFECTS

===========================================================================*/
unsigned char checkCrc(unsigned char* pData, int Len)
{
	unsigned int value =0;
	int i, j, crc = 0xffff;
	unsigned short crcCalc=0, crcOrg = 0;
	//unsigned char bCrcOk = 0; 
	unsigned char* DataPtr = NULL;
	
	DataPtr = pData;
	crcOrg =  *(DataPtr+Len-2) << 8 |*(DataPtr+Len-1);
	
	for(i = 0; i < Len-2; i++) 
	{  
		value = DataPtr[i];
		value <<= 8; 
		
		for (j = 0; j < 8; j++) {
			value <<= 1;
			crc <<= 1;  
			if ((crc ^ value) & 0x10000){ crc ^= 0x1021; }
		}  
	}
	
	crcCalc = ~crc & 0xffff;
	
	if(crcOrg == crcCalc) { return 1; }  
	else { return 0; }
}


#ifdef CHECKSUM_PROGRESS_DISPLAY
void ChecksumPrint(char *text, char *filename, __int64 Totfilesize, __int64 readsize, int insertCRC )
{

	if( 1==insertCRC ) return; // 2017.04.04

	printf("\b%s>> (%s) -> reading : %lld Bytes \r", text, filename, Totfilesize );
}

void ClearScreen()
{
	#if 0
	//          1234567890123456789012345678901234567890123456789012345678901234567890
	LOG_V("                                                                                                   \r");
	#endif
}
#endif	

/* -------------------------------------------------------------------------
CRC32        : 32-bit checksum using Cyclic Redundancy Check (CRC32). crc 
CRC64-ISO    : 64-bit checksum using the Cyclic Redundancy Check defined by ISO (CRC-64-ISO) spcrc 
CRC64-ECMA-182 : 64-bit checksum using the alternative Cyclic Redundancy Check defined in ECMA-182 (CRC-64-ECMA-182) altcrc 
CRC32-CRC64 : Combination of CRC32 and CRC64 sequence checksums with the sequence length cdigest 
MD5          : 128-bit Hash using Message Digest 5 (MD5) md 
SHA1         : 160-bit Hash using Secure Hash Algorithm 1 (SHA-1) shaa 
SHA2-224     : 224-bit Hash using Secure Hash Algorithm 2 (SHA-2) shab 
SHA2-256     : 256-bit Hash using Secure Hash Algorithm 2 (SHA-2) shac 
SHA2-384     : 384-bit Hash using Secure Hash Algorithm 2 (SHA-2) shad 
SHA2-512     : 512-bit Hash using Secure Hash Algorithm 2 (SHA-2) shae 
 ------------------------------------------------------------------------- */

unsigned short g_calcCrc16;
unsigned int g_calcCrc32;
unsigned int g_calcAdler32;
unsigned long long g_calcCrc64;
unsigned int g_calcJoaat;



unsigned __int64 RunCRC16(char *infile_name, char *outfile_name, __int64 Fsize, int mIndex, int insertCRC, int iVerbosType, char *str_hash)
{ 
	unsigned short calcCrc16 = 0xFFFF; 
	unsigned __int64  TotDLen=0LL;
	size_t 	 LenRead=0;
	
	if( 0==mIndex )
		printf("CRC16>> Creating CRC16 [ KS X ISO 18234-2 ]... \n");

	if(data_buf) { free(data_buf); data_buf=NULL; }
	data_buf = (unsigned char*)malloc( MAX_BUF_SIZ*sizeof(unsigned char) );

	memset( data_buf, 0x00, MAX_BUF_SIZ*sizeof(unsigned char) );
	
	TotDLen = 0LL;
	calcCrc16 = 0xFFFF; // initial value
	while ( LenRead = fread(data_buf, sizeof(unsigned char), CRC_BUFSIZE, inpfile) )
	{
		TotDLen += LenRead;
		g_calcCrc16 = calcCrc16 = make_crc16(calcCrc16, data_buf, LenRead);
			
	#ifdef CHECKSUM_PROGRESS_DISPLAY
		ChecksumPrint("CRC16 Hashing", infile_name, TotDLen, LenRead, insertCRC );
	#endif	
	}

	ClearScreen();

	if(1 == insertCRC) 
	{
		if( g_iUpper ) // 2017.04.27
			printf("%04X   *%s*%s__(%llu) \r\n", g_calcCrc16, str_hash, infile_name, TotDLen );
		else
			printf("%04x   *%s*%s__(%llu) \r\n", g_calcCrc16, str_hash, infile_name, TotDLen );

		return TotDLen;
	}
	// ---------------------------------------	


	if( inpfile ) // 2020.07.17 && outfile ) /// && TotDLen )	
	{

		if(verbose && (2==iVerbosType || 3==iVerbosType) )
		{
			if( g_iUpper )
			{
				if(outfile) fprintf(outfile,"%04X  *%s*%s__(%llu) \r\n", calcCrc16, str_hash,infile_name, TotDLen );
			}
			else
			{
				if(outfile) fprintf(outfile,"%04x  *%s*%s__(%llu) \r\n", calcCrc16, str_hash,infile_name, TotDLen );
			}
		}
		else
		{
			if( g_iUpper )
			{
				if(outfile) fprintf(outfile,"%04X  *%s*%s \r\n", calcCrc16, str_hash,infile_name );
			}
			else
			{
				if(outfile) fprintf(outfile,"%04x  *%s*%s \r\n", calcCrc16, str_hash,infile_name );
			}
		}

		if( 0==mIndex ) /// 1 file
		{
			if(verbose && (2==iVerbosType || 3==iVerbosType) )
				printf("%04x   *%s*%s__(%llu) \r\n", calcCrc16, str_hash, infile_name, TotDLen );
			else
				printf("%04x   *%s*%s \r\n", calcCrc16, str_hash, infile_name );
		}
		else /// multi-input
		{
			if(verbose && (2==iVerbosType || 3==iVerbosType) )
				printf("%04x   *%s*%s__(%llu) \r\n", calcCrc16, str_hash, infile_name, TotDLen );
			else
				printf("%04x   *%s*%s \r\n", calcCrc16, str_hash, infile_name );
		}


	}
	else
	{
		printf("CRC16>> Can not create CRC16 for file [%s] or wrong length(%llu) \r\n", infile_name, TotDLen );
	}
	
	if(data_buf) { free(data_buf); data_buf=NULL; }

	//Fsize = TotDLen; /* 인자로 값을 넘길경우,  맨마지막에 하라~~ else fclosed !!! */

	return TotDLen;
}



unsigned __int64 RunKSC_CRC16(char *infile_name, char *outfile_name, __int64 Fsize, int mIndex, int insertCRC, int iVerbosType, char *str_hash)
{ 
	unsigned short calcCrc16 = 0xFFFF; 
	unsigned __int64  TotDLen=0LL;
	size_t	 LenRead=0;
	
	if( 0==mIndex )
		printf("KSC-CRC16>> Creating KSC-CRC16 [ KSCIEC62056-61 ]... \n");

	if(data_buf) { free(data_buf); data_buf=NULL; }
	data_buf = (unsigned char*)malloc( MAX_BUF_SIZ*sizeof(unsigned char) );

	memset( data_buf, 0x00, MAX_BUF_SIZ*sizeof(unsigned char) );
	
	TotDLen = 0LL;
	calcCrc16 = 0xFFFF; // initial value
	while ( LenRead = fread(data_buf, sizeof(unsigned char), CRC_BUFSIZE, inpfile) )
	{
		TotDLen += LenRead;
		g_calcCrc16 = calcCrc16 = make_ksc_crc16(calcCrc16, data_buf, LenRead);
			
	#ifdef CHECKSUM_PROGRESS_DISPLAY
		ChecksumPrint("KSC-CRC16 Hashing", infile_name, TotDLen, LenRead, insertCRC );
	#endif	
	}

	ClearScreen();

	if(1 == insertCRC) 
	{
		if( g_iUpper ) // 2017.04.27
			printf("%04X   *%s*%s__(%llu) \r\n", g_calcCrc16, str_hash, infile_name, TotDLen );
		else
			printf("%04x   *%s*%s__(%llu) \r\n", g_calcCrc16, str_hash, infile_name, TotDLen );

		return TotDLen;
	}
	// ---------------------------------------	


	if( inpfile ) // 2020.07.17 && outfile ) /// && TotDLen )	
	{

		if(verbose && (2==iVerbosType || 3==iVerbosType) )
		{
			if( g_iUpper )
			{
				if(outfile) fprintf(outfile,"%04X  *%s*%s__(%llu) \r\n", calcCrc16, str_hash,infile_name, TotDLen );
			}
			else
			{
				if(outfile) fprintf(outfile,"%04x  *%s*%s__(%llu) \r\n", calcCrc16, str_hash,infile_name, TotDLen );
			}
		}
		else
		{
			if( g_iUpper )
			{
				if(outfile) fprintf(outfile,"%04X  *%s*%s \r\n", calcCrc16, str_hash,infile_name );
			}
			else
			{
				if(outfile) fprintf(outfile,"%04x  *%s*%s \r\n", calcCrc16, str_hash,infile_name );
			}
		}

		if( 0==mIndex ) /// 1 file
		{
			if(verbose && (2==iVerbosType || 3==iVerbosType) )
				printf("%04x   *%s*%s__(%llu) \r\n", calcCrc16, str_hash, infile_name, TotDLen );
			else
				printf("%04x   *%s*%s \r\n", calcCrc16, str_hash, infile_name );
		}
		else /// multi-input
		{
			if(verbose && (2==iVerbosType || 3==iVerbosType) )
				printf("%04x   *%s*%s__(%llu) \r\n", calcCrc16, str_hash, infile_name, TotDLen );
			else
				printf("%04x   *%s*%s \r\n", calcCrc16, str_hash, infile_name );
		}

	}
	else
	{
		printf("KSC-CRC16>> Can not create KSC-CRC16 for file [%s] or wrong length(%llu) \r\n", infile_name, TotDLen );
	}
	
	if(data_buf) { free(data_buf); data_buf=NULL; }

	//Fsize = TotDLen; /* 인자로 값을 넘길경우,  맨마지막에 하라~~ else fclosed !!! */

	return TotDLen;
}


unsigned __int64 RunCRC16CCITT(char *infile_name, char *outfile_name, __int64 Fsize, int mIndex, int insertCRC, int iVerbosType, char *str_hash)
{ 
	unsigned short calcCrc16 = 0; 
	unsigned __int64  TotDLen=0LL;
	size_t 	 LenRead=0;

	if(0==mIndex)
		printf("CRC16CCITT>> Creating CRC16 [ CCITT standards ]..... \n");

	data_buf = (unsigned char*)malloc( MAX_BUF_SIZ*sizeof(unsigned char) );

	TotDLen = 0LL;
	calcCrc16 = 0;
	memset( data_buf, 0x00, MAX_BUF_SIZ*sizeof(unsigned char) );
	while ( LenRead = fread(data_buf, sizeof(unsigned char), CRC_BUFSIZE, inpfile) )
	{
		TotDLen += LenRead;
		g_calcCrc16 = calcCrc16 = make_crc16_ccitt( calcCrc16, data_buf, LenRead);
		
	#ifdef CHECKSUM_PROGRESS_DISPLAY
		ChecksumPrint("CRC16CCITT Hashing", infile_name, TotDLen, LenRead, insertCRC  );
	#endif		

	}
	ClearScreen();
	
	if(data_buf) { free(data_buf); data_buf=NULL; }

	if(1 == insertCRC) 
	{
	
		if( g_iUpper ) // 2017.04.27
			printf("%04X   *%s*%s__(%llu) \r\n", g_calcCrc16, str_hash, infile_name, TotDLen );
		else
			printf("%04x   *%s*%s__(%llu) \r\n", g_calcCrc16, str_hash, infile_name, TotDLen );

		return TotDLen;
	}
	// ---------------------------------------	


	if( inpfile ) // 2020.07.17 && outfile )	
	{
		//fwrite((unsigned char*)&crc_res, sizeof(unsigned char), 2 , outfile);

		if(verbose && (2==iVerbosType || 3==iVerbosType) )
		{
			if(outfile) fprintf(outfile,"%04x  *%s*%s__(%llu)  \r\n", calcCrc16, str_hash, infile_name, TotDLen);
		}
		else
		{
			if(outfile) fprintf(outfile,"%04x  *%s*%s  \r\n", calcCrc16, str_hash, infile_name);
		}

		if( 0==mIndex ) /// 1 file
		{
			if(verbose && (2==iVerbosType || 3==iVerbosType) )
			{
				printf("%04x  *%s*%s__(%llu) \r\n", calcCrc16, str_hash, infile_name, TotDLen );
			}
			else
			{
				printf("%04x  *%s*%s \r\n", calcCrc16, str_hash, infile_name );
			}
		}
		else /// multi-input files
		{
			if(verbose && (2==iVerbosType || 3==iVerbosType) )
				printf("%04x  *%s*%s__(%llu) \r\n", calcCrc16, str_hash, infile_name, TotDLen );
			else
				printf("%04x  *%s*%s \r\n", calcCrc16, str_hash, infile_name );
		}

	}
	else
	{
		printf("CRC16CCITT> Can not create CRC16CCITT for file [%s] or wrong length(%llu) \r\n", infile_name, TotDLen );
	}

	//*Fsize = TotDLen; /* 인자로 값을 넘길경우,  맨마지막에 하라~~ else fclosed !!! */

	return TotDLen;
}



unsigned __int64 RunCRC32(char *infile_name, char *outfile_name, __int64 Fsize, int mIndex, int insertCRC, int iVerbosType, char *str_hash)
{
	unsigned int calcCrc32=0;
	unsigned __int64  TotDLen=0;
	size_t 	 LenRead=0;

	if(0==mIndex)
		printf("CRC32>> Creating CRC32 [ Polynomial : 0xEDB88320 ]... \n");

	data_buf = (unsigned char*)malloc( MAX_BUF_SIZ*sizeof(unsigned char) );
	memset( data_buf, 0x00, MAX_BUF_SIZ*sizeof(unsigned char) );

	TotDLen = 0LL;
	while ( LenRead = fread(data_buf, sizeof(unsigned char), CRC_BUFSIZE, inpfile) )
	{
		TotDLen += LenRead;

	#if 0 // !Choice One!!!
		g_calcCrc32 = calcCrc32 = make_crc32(calcCrc32, (void*)data_buf, LenRead);
	#else
		g_calcCrc32 = calcCrc32 = calcCRC32(data_buf, (unsigned long)LenRead, calcCrc32);
	#endif

	
	#ifdef CHECKSUM_PROGRESS_DISPLAY
		ChecksumPrint("CRC32 Hashing", infile_name, TotDLen, LenRead, insertCRC  );
	#endif

	}
	ClearScreen();
	
	if(data_buf) { free(data_buf); data_buf=NULL; }

	if(1 == insertCRC) 
	{
		if( g_iUpper ) // 2017.04.27
			printf("%08X   *%s*%s__(%llu) \r\n", g_calcCrc32, str_hash, infile_name, TotDLen );
		else	
			printf("%08x   *%s*%s__(%llu) \r\n", g_calcCrc32, str_hash, infile_name, TotDLen );

		return TotDLen;
	}
	// ---------------------------------------	

	if( inpfile ) //  2020.07.17 && outfile )
	{

		if(verbose && (2==iVerbosType || 3==iVerbosType) )
		{
			if(outfile) fprintf(outfile,"%08x  *%s*%s__(%llu) \r\n", calcCrc32, str_hash, infile_name, TotDLen );
		}
		else
		{
			if(outfile) fprintf(outfile,"%08x  *%s*%s \r\n", calcCrc32, str_hash, infile_name );
		}

		if(0==mIndex) /// 1 file
		{
			if(verbose && (2==iVerbosType || 3==iVerbosType) )
			{
				printf("%08x  *%s*%s__(%llu) \r\n", calcCrc32, str_hash, infile_name, TotDLen );
			}
			else
			{
				printf("%08x  *%s*%s \r\n", calcCrc32, str_hash, infile_name );
			}
		}
		else /// multi-input files
		{
			if(verbose && (2==iVerbosType || 3==iVerbosType) )
				printf("%08x  *%s*%s__(%llu) \r\n", calcCrc32, str_hash, infile_name, TotDLen  );
			else
				printf("%08x  *%s*%s \r\n", calcCrc32, str_hash, infile_name );
		}


	}
	else
	{
		printf("CRC32>> Can not create CRC32 for file [%s]. \r\n", infile_name );
	}


	//*Fsize = TotDLen; /* 인자로 값을 넘길경우,  맨마지막에 하라~~ else fclosed !!! */

	return TotDLen;
}


unsigned __int64 RunCRC64(char *infile_name, char *outfile_name, __int64 Fsize, int mIndex, int insertCRC, int iVerbosType, char *str_hash)
{
	unsigned long long calcCrc64 = 0ULL;
	unsigned __int64  TotDLen=0LL;
	size_t 	 LenRead=0;

	if(0==mIndex)
		printf("CRC64>> Creating CRC64 [ Polynomial : 0xad93d23594c935a9 ]... \n");

	data_buf = (unsigned char*)malloc( MAX_BUF_SIZ*sizeof(unsigned char) );
	memset( data_buf, 0x00, MAX_BUF_SIZ*sizeof(unsigned char) );

	TotDLen = 0LL;
	while ( LenRead = fread(data_buf, sizeof(unsigned char), CRC_BUFSIZE, inpfile) )
	{
		TotDLen += LenRead;

	#if 1 
		g_calcCrc64 = calcCrc64 = make_crc64(calcCrc64, (void*)data_buf, LenRead);
	#else
		///g_calcCrc64 = calcCrc64 = crc64(calcCrc64, (void*)data_buf, LenRead);
	#endif
	
	#ifdef CHECKSUM_PROGRESS_DISPLAY
		ChecksumPrint("CRC64 Hashing", infile_name, TotDLen, LenRead, insertCRC  );
	#endif

	}
	ClearScreen();

	if(data_buf) { free(data_buf); data_buf=NULL; }

	if(1 == insertCRC) // in header
	{
		if( g_iUpper ) // 2017.04.27
			printf("%016llX    *%s*%s__(%llu) \r\n", g_calcCrc64, str_hash, infile_name, TotDLen );
		else	
			printf("%016llx    *%s*%s__(%llu) \r\n", g_calcCrc64, str_hash, infile_name, TotDLen );

		return TotDLen;
	}
	// --------------------------------------------------------------------	

	if( inpfile ) //  2020.07.17 && outfile )
	{
		if(verbose && (2==iVerbosType || 3==iVerbosType) )
		{
			if(outfile) fprintf(outfile,"%016llx  *%s*%s__(%llu) \r\n", calcCrc64, str_hash, infile_name, TotDLen );
		}
		else
		{
			if(outfile) fprintf(outfile,"%016llx  *%s*%s \r\n", calcCrc64, str_hash, infile_name );
		}

		if( 0==mIndex ) /// 1 files ---
		{
			if(verbose && (2==iVerbosType || 3==iVerbosType) )
			{
				printf("%016llx  *%s*%s__(%llu) \r\n", calcCrc64, str_hash, infile_name, TotDLen );
			}
			else
			{
				printf("%016llx  *%s*%s \r\n", calcCrc64, str_hash, infile_name  );
			}
		}
		else /// multi-input files
		{
			if(verbose && (2==iVerbosType || 3==iVerbosType) )
				printf("%016llx  *%s*%s__(%llu) \r\n", calcCrc64, str_hash, infile_name, TotDLen );
			else
				printf("%016llx  *%s*%s \r\n", calcCrc64, str_hash, infile_name );
		}

	}
	else
	{
		printf("CRC64>> Can not create CRC64 for file [%s]. \r\n", infile_name );
	}

	//*Fsize = TotDLen; /* 인자로 값을 넘길경우,  맨마지막에 하라~~ else fclosed !!! */

	return TotDLen;
}



unsigned __int64 RunCRC64_isc(char *infile_name, char *outfile_name, __int64 Fsize, int mIndex, int insertCRC, int iVerbosType, char *str_hash)
{
	unsigned long long calcCrc64 = 0ULL;
	unsigned __int64  TotDLen=0LL;
	size_t 	 LenRead=0;

	if(0==mIndex)
		printf("CRC64>> Creating CRC64_ISC(Internet Systems Consortium)... \n");

	data_buf = (unsigned char*)malloc( MAX_BUF_SIZ*sizeof(unsigned char) );
	memset( data_buf, 0x00, MAX_BUF_SIZ*sizeof(unsigned char) );

	TotDLen = 0LL;

	isc_crc64_init(); // Internet Consorisum C
	while ( LenRead = fread(data_buf, sizeof(unsigned char), CRC_BUFSIZE, inpfile) )
	{
		TotDLen += LenRead;

	#if 0 // NEVER!!! 
		g_calcCrc64 = calcCrc64 = make_crc64(calcCrc64, (void*)data_buf, LenRead);
	#else
		g_calcCrc64 = calcCrc64 = isc_crc64_update(calcCrc64, (void*)data_buf, LenRead);
	#endif
	
	#ifdef CHECKSUM_PROGRESS_DISPLAY
		ChecksumPrint("CRC64_ISC Hashing", infile_name, TotDLen, LenRead, insertCRC  );
	#endif
	}

	g_calcCrc64 = calcCrc64 = isc_crc64_final(calcCrc64);

	
	ClearScreen();

	if(data_buf) { free(data_buf); data_buf=NULL; }

	if(1 == insertCRC) // in header
	{
		if( g_iUpper ) // 2017.04.27
			printf("%016llX    *%s*%s__(%llu) \r\n", g_calcCrc64, str_hash, infile_name, TotDLen );
		else	
			printf("%016llx    *%s*%s__(%llu) \r\n", g_calcCrc64, str_hash, infile_name, TotDLen );

		return TotDLen;
	}
	// --------------------------------------------------------------------	

	if( inpfile ) //  2020.07.17 && outfile )
	{
		if(verbose && (2==iVerbosType || 3==iVerbosType) )
		{
			if(outfile) fprintf(outfile,"%016llx  *%s*%s__(%llu) \r\n", calcCrc64, str_hash, infile_name, TotDLen );
		}
		else
		{
			if(outfile) fprintf(outfile,"%016llx  *%s*%s \r\n", calcCrc64, str_hash, infile_name );
		}

		if( 0==mIndex ) /// 1 files ---
		{
			if(verbose && (2==iVerbosType || 3==iVerbosType) )
			{
				printf("%016llx  *%s*%s__(%llu) \r\n", calcCrc64, str_hash, infile_name, TotDLen );
			}
			else
			{
				printf("%016llx  *%s*%s \r\n", calcCrc64, str_hash, infile_name  );
			}
		}
		else /// multi-input files
		{
			if(verbose && (2==iVerbosType || 3==iVerbosType) )
				printf("%016llx  *%s*%s__(%llu) \r\n", calcCrc64, str_hash, infile_name, TotDLen );
			else
				printf("%016llx  *%s*%s \r\n", calcCrc64, str_hash, infile_name );
		}

	}
	else
	{
		printf("CRC64_ISC>> Can not create CRC64ISC for file [%s]. \r\n", infile_name );
	}

	//*Fsize = TotDLen; /* 인자로 값을 넘길경우,  맨마지막에 하라~~ else fclosed !!! */

	return TotDLen;
}


unsigned __int64 RunAdler32(char *infile_name, char *outfile_name, __int64 Fsize, int mIndex, int insertCRC, int iVerbosType, char *str_hash)
{
	unsigned int calcAdler32 = 1;
	unsigned __int64  TotDLen=0;
	size_t	 LenRead=0;
	
	if(0==mIndex)
		printf("ADLER32>> Creating Adler32 [ zlib ]... \n");

	data_buf = (unsigned char*)malloc( MAX_BUF_SIZ*sizeof(unsigned char) );
	memset( data_buf, 0x00, MAX_BUF_SIZ*sizeof(unsigned char) );

	TotDLen = 0LL;
	while ( LenRead = fread(data_buf, sizeof(unsigned char), CRC_BUFSIZE, inpfile) )
	{
		TotDLen += LenRead;
		g_calcAdler32 = calcAdler32 = make_adler32(calcAdler32, data_buf, LenRead);

	#ifdef CHECKSUM_PROGRESS_DISPLAY
		ChecksumPrint("ADLER32 Hashing", infile_name, TotDLen, LenRead, insertCRC  );
	#endif

	}
	ClearScreen();

	if(data_buf) { free(data_buf); data_buf=NULL; }

	if(1 == insertCRC) 
	{
		if( g_iUpper ) // 2017.04.27
			printf("%08X     *%s*%s__(%llu) \r\n", g_calcAdler32, str_hash, infile_name, TotDLen );
		else
			printf("%08x     *%s*%s__(%llu) \r\n", g_calcAdler32, str_hash, infile_name, TotDLen );

		return TotDLen;
	}
	// ---------------------------------------	
	
	if( inpfile ) // 2020.07.17  && outfile )
	{
		if(verbose && (2==iVerbosType || 3==iVerbosType) )
		{
			if(outfile) fprintf(outfile,"%08x  *%s*%s__(%llu) \r\n", calcAdler32, str_hash, infile_name, TotDLen );
		}
		else
		{
			if(outfile) fprintf(outfile,"%08x  *%s*%s \r\n", calcAdler32, str_hash, infile_name );
		}

		if( 0==mIndex ) /// 1 files ---
		{
			if(verbose && (2==iVerbosType || 3==iVerbosType) )
			{
				printf("%08x  *%s*%s__(%llu) \r\n", calcAdler32, str_hash, infile_name, TotDLen );
			}
			else
			{
				printf("%08x  *%s*%s  \r\n", calcAdler32, str_hash, infile_name );
			}
		}
		else /// multi-input files
		{
			if(verbose && (2==iVerbosType || 3==iVerbosType) )
				printf("%08x  *%s*%s__(%llu) \r\n", calcAdler32, str_hash, infile_name, TotDLen );
			else
				printf("%08x  *%s*%s \r\n", calcAdler32, str_hash, infile_name );
		}


	}
	else
	{
		printf("ADLER32>> Can not create Adler32 for file [%s]. \r\n", infile_name );
	}


	//*Fsize = TotDLen; /* 인자로 값을 넘길경우,  맨마지막에 하라~~ else fclosed !!! */

	return TotDLen;
}



unsigned __int64 RunJoaat(char *infile_name, char *outfile_name, __int64 Fsize, int mIndex, int insertCRC, int iVerbosType, char *str_hash)
{
	unsigned int calcJoaat = 0;
	unsigned __int64  TotDLen=0;
	size_t	 LenRead=0;
	
	if(0==mIndex)
		printf("JOAAT>> Creating JOAAT Hash... \n");

	data_buf = (unsigned char*)malloc( MAX_BUF_SIZ*sizeof(unsigned char) );
	memset( data_buf, 0x00, MAX_BUF_SIZ*sizeof(unsigned char) );

	TotDLen = 0LL;
	calcJoaat = 0;
	while ( LenRead = fread(data_buf, sizeof(unsigned char), BUFSIZE, inpfile) )
	{
		TotDLen += LenRead;
		g_calcJoaat = calcJoaat = joaat_hash(calcJoaat, data_buf, LenRead);
		
	#ifdef CHECKSUM_PROGRESS_DISPLAY
		ChecksumPrint("JOAAT Hashing", infile_name, TotDLen, LenRead, insertCRC  );
	#endif

	}
	ClearScreen();

	if(data_buf) { free(data_buf); data_buf=NULL; }

	if(1 == insertCRC) 
	{
		if( g_iUpper ) // 2017.04.27
			printf("%08X   *%s*%s__(%llu) \r\n", g_calcJoaat, str_hash, infile_name, TotDLen );
		else
			printf("%08x   *%s*%s__(%llu) \r\n", g_calcJoaat, str_hash, infile_name, TotDLen );

		return TotDLen;
	}
	// ---------------------------------------	
	
	if( inpfile ) // 2020.07.17  && outfile )
	{
		if(verbose && (2==iVerbosType || 3==iVerbosType) )
		{
			if(outfile) fprintf(outfile,"%08x  *%s*%s__(%llu) \r\n", calcJoaat, str_hash, infile_name, TotDLen );
		}
		else
		{
			if(outfile) fprintf(outfile,"%08x  *%s*%s \r\n", calcJoaat, str_hash, infile_name );
		}

		if( 0==mIndex ) /// 1 files ---
		{
			if(verbose && (2==iVerbosType || 3==iVerbosType) )
			{
				printf("%08x  *%s*%s__(%llu) \r\n", calcJoaat, str_hash, infile_name, TotDLen );
			}
			else
			{
				printf("%08x  *%s*%s  \r\n", calcJoaat, str_hash, infile_name );
			}
		}
		else /// multi-input files
		{
			if(verbose && (2==iVerbosType || 3==iVerbosType) )
				printf("%08x  *%s*%s__(%llu) \r\n", calcJoaat, str_hash, infile_name, TotDLen );
			else
				printf("%08x  *%s*%s \r\n", calcJoaat, str_hash, infile_name );
		}
	}
	else
	{
		printf("JOAAT>> Can not create JOAAT for file [%s]. \r\n", infile_name );
	}

	//*Fsize = TotDLen; /* 인자로 값을 넘길경우,  맨마지막에 하라~~ else fclosed !!! */
	return TotDLen;
}


#endif // CRC_CHECKSUM




