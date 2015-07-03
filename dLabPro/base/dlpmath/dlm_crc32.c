/* dLabPro mathematics library
 * - CRC32 hash function
 *
 * AUTHOR : Matthias Wolff,
 * PACKAGE: dLabPro/base
 * 
 * Copyright 2013 dLabPro contributors and others (see COPYRIGHT file) 
 * - Chair of System Theory and Speech Technology, TU Dresden
 * - Chair of Communications Engineering, BTU Cottbus
 * 
 * This file is part of dLabPro.
 * 
 * dLabPro is free software: you can redistribute it and/or modify it under the
 * terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option)
 * any later version.
 * 
 * dLabPro is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
 * details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with dLabPro. If not, see <http://www.gnu.org/licenses/>.
 */

/* NOTE:
 * 
 * This is the prototypical implementation of a dLabPro hash generator. If there
 * will be more, make them so (and put them into an own library).
 * 
 * These should be implemented:
 * - MD5 (it's so common)
 * - whirlpool (it's so free :) 
 */

#include "dlp_kernel.h"
#include "dlp_base.h" 
#include "dlp_math.h" 

/* -- Private -- */

#define CRC32POLY  0x04C11DB7                                                   /* CRC-32 polynomial                 */
#define CRC32INIT  0xFFFFFFFF                                                   /* Initial value of shift register   */
#define CRC32FINAL 0xFFFFFFFF                                                   /* Final XOR value                   */

typedef struct tag_CRC32_TYPE
{
  UINT32 nCrc32;                                                             /* Shift register                   */
} CRC32_TYPE;

UINT32 CGEN_IGNORE dlm_bitreversal_32(UINT32 n)
{
  INT32       i = 0;
  UINT32 m = 0;
  for (i=0; i<32; i++)
  {
    m <<= 1;
    m |= (n&(1<<i))>>i;
  }
  return m;
}

/* -- Public -- */

/**
 * Creates a new CRC-32 hash generator.
 * 
 * <h4>Example</h4>
 * <pre class="code">
 *
 *  UINT64 nCrc32 = 0;
 *  void* lpH = {@link dlm_crc32_init}();
 *  {@link dlm_crc32_add}(lpH,(const BYTE*)"Hello world",11);
 *  {@link dlm_crc32_finalize}(lpH,(BYTE*)&nCrc32,sizeof(nCrc32));
 *  printf("CRC-32=0x%08X",(unsigned int)nCrc32);
 * </pre>
 * 
 * @return Pointer to the generator instance
 * @see dlm_crc32_add
 * @see dlm_crc32_finalize
 * @see dlm_crc32
 */
void* dlm_crc32_init()
{
  CRC32_TYPE* H = (CRC32_TYPE*)dlp_calloc(1,sizeof(CRC32_TYPE));                /* Allocate hash generator data      */
  H->nCrc32 = CRC32INIT;                                                        /* Initialize hash generator         */
  return H;                                                                     /* Return pointer to generator data  */
}

/**
 * Feeds a plain text block into a CRC-32 hash generator.
 * 
 * @param lpH
 *          Pointer to a hash generator instance (returned by
 *          {@link dlm_crc32_init})
 * @param lpBuffer
 *          Pointer to plain text buffer (casting from <code>const char*</code>
 *          is ok)
 * @param nLen
 *          Number of bytes stored in plain text buffer 
 * @see dlm_crc32_init
 * @see dlm_crc32_finalize
 * @see dlm_crc32
 */
void dlm_crc32_add(void* lpH, const BYTE* lpBuffer, INT32 nLen)
{
  CRC32_TYPE* H     = NULL;                                                     /* Pointer to generator data struct  */
  BYTE        nTbit = 0;                                                        /* Current plain text bit            */
  BYTE        nSbit = 0;                                                        /* Most sign. shift register bit     */
  INT32     i     = 0;                                                        /* Byte counter                      */
  INT32     j     = 0;                                                        /* Bit counter                       */
   
  if (!lpH || !lpBuffer || nLen<=0) return;                                     /* One word: NO!                     */
  H = ((CRC32_TYPE*)lpH);                                                       /* Pointer cast                      */
  for (i=0; i<nLen; i++)                                                        /* Loop over plain text bytes        */
    for (j=0; j<8; j++)                                                         /*   Loop over bits of current byte  */
    {                                                                           /*   >>                              */
      nTbit = 0x01&(lpBuffer[i]>>j);                                            /*     Get current plain text bit    */
      nSbit = (H->nCrc32&0x80000000)?1:0;                                       /*     Get most sign. shift reg. bit */
      if (nTbit!=nSbit)                                                         /*     Data bit != shift reg. bit    */
        H->nCrc32 = (H->nCrc32<<1)^CRC32POLY;                                   /*       Shift + XOR with polynomial */
      else                                                                      /*     Data bit == shift reg. bit    */
        H->nCrc32 = H->nCrc32<<1;                                               /*       Just shift                  */
    }                                                                           /*   <<                              */
}

/**
 * Computes the CRC-32 hash value from the internal data of the hash generator
 * an destroys the generator.
 * 
 * @param lpH
 *          Pointer to a hash generator instance (returned by
 *          {@link dlm_crc32_init})
 * @param lpHash
 *          Pointer to a buffer to be filled with the CRC-32 hash value (make it
 *          a 32 bit unsigned integer :)
 * @param nLen
 *          Number of bytes which can be stored in the buffer (need 4)
 * @return <code>O_K</code> if successfull, a (negative) error code otherwise 
 * @see dlm_crc32_init
 * @see dlm_crc32_add
 * @see dlm_crc32
 */
INT16 dlm_crc32_finalize(void* lpH, BYTE* lpHash, INT32 nLen)
{
  CRC32_TYPE* H    = NULL;                                                      /* Pointer to generator data struct  */
  INT16       nErr = O_K;                                                       /* Return value                      */

  if (!lpH) return NOT_EXEC;                                                    /* No hash generator, no service     */
  H = ((CRC32_TYPE*)lpH);                                                       /* Pointer cast                      */
  if (lpHash && (size_t)nLen>=sizeof(H->nCrc32))                                        /* Output buffer ok                  */
  {                                                                             /* >>                                */
    H->nCrc32 = dlm_bitreversal_32(H->nCrc32);                                  /*   Final bit reversal              */
    H->nCrc32 = H->nCrc32 ^ CRC32FINAL;                                         /*   Final XOR                       */ 
    dlp_memmove(lpHash,&H->nCrc32,sizeof(H->nCrc32));                           /*   Copy hash                       */
  }                                                                             /* <<                                */
  else if (lpHash)                                                              /* Output buffer too small           */
    nErr = NOT_EXEC;                                                            /*   Error                           */
  dlp_free(lpH);                                                                /* Destroy hash generator data       */
  return nErr;                                                                  /* Return error code                 */
}

/**
 * Computes the CRC32 hash of a plain text. This all-in-one function is provided
 * for convenience.
 * 
 * @param lpsText
 *          Pointer to the plain text
 * @return The CRC32 hash for <code>lpsText</code>
 * @see dlm_crc32_init
 * @see dlm_crc32_add
 * @see dlm_crc32_finalize
 */
UINT64 dlm_crc32(const char* lpsText)
{
  UINT32 nCrc32 = 0;                                                         /* The return value                  */ 
  CRC32_TYPE* lpH = dlm_crc32_init();                                           /* Initialize hash generator         */
  dlm_crc32_add(lpH,(const BYTE*)lpsText,dlp_strlen(lpsText));                  /* Feed text through hash generator  */
  dlm_crc32_finalize(lpH,(BYTE*)&nCrc32,sizeof(UINT32));                     /* Get hash and destroy generator    */
  return (UINT64)nCrc32;                                                 /* Return hash                       */
}

/* EOF */
