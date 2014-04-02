/* dLabPro mathematics library
 * - Fast Wavelet Transform - unoptimized
 *
 * AUTHOR : Soeren Wittenberg
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

#include "dlp_kernel.h"
#include "dlp_base.h" 
#include "dlp_math.h"

/* define */
#define SQRT2 1.414213562  /* square of 2 */
#define SQRT3 1.73205081   /* square of 3 */

/* constants */
const FLOAT64 h0 = (1. + SQRT3)/(4. * SQRT2);
const FLOAT64 h1 = (3. + SQRT3)/(4. * SQRT2); 
const FLOAT64 h2 = (3. - SQRT3)/(4. * SQRT2); 
const FLOAT64 h3 = (1. - SQRT3)/(4. * SQRT2);

 /*   Daubechies D4 transform
 *
 *    FLOAT64* sig         signal array
 *    FLOAT64* trans       transformed signal
 *    INT32    size        signal size
 *    INT16   level       detail level (max. level = -1)  
  */
INT16 CGEN_IGNORE dlm_fwt_d4(FLOAT64* sig, FLOAT64* trans, INT32 size, INT16 level)
{
  register INT32 i;
  register INT32 n;
  register INT32 size2;
  FLOAT64* sigTemp;
  

  if(size < 4)
  {
    return NOT_EXEC;
  } 
  
  sigTemp = (FLOAT64*)dlp_calloc(size, sizeof(FLOAT64));
  dlp_memmove(sigTemp, sig, size * sizeof(FLOAT64));
  
  for (n = size; n >= 4; n >>= 1)
  {
    size2 = n >> 1;
    for (i = 0; i <= size2-2; i++)
    {
      trans[i]       = sigTemp[i*2]*h0 + sigTemp[i*2+1]*h1 + sigTemp[i*2+2]*h2 + sigTemp[i*2+3]*h3; /* scaling function coefficients */
      trans[i+size2] = sigTemp[i*2]*h3 - sigTemp[i*2+1]*h2 + sigTemp[i*2+2]*h1 - sigTemp[i*2+3]*h0; /* wavelet function coefficents */
    }

    trans[i]       = sigTemp[n-2]*h0 + sigTemp[n-1]*h1 + sigTemp[0]*h2 + sigTemp[1]*h3;
    trans[i+size2] = sigTemp[n-2]*h3 - sigTemp[n-1]*h2 + sigTemp[0]*h1 - sigTemp[1]*h0;

    dlp_memmove(sigTemp, trans, n * sizeof(FLOAT64));
    level--;
    if(level == 0) break; /* reduce detail level and exit if reaching 0; if detail level is max. (-1) nothing happens */
  }
  
  dlp_free(sigTemp);
  
  return O_K;
}

 /*   Inverse Daubechies D4 transform
 *
 *    FLOAT64* trans       transformed signal
 *    FLOAT64* sig         signal array 
 *    INT32    size        transformed signal array size
 *    INT16   level       detail level (max. level = -1) that was used to transform signal  
 */
INT16 CGEN_IGNORE dlm_fwt_d4_inv(FLOAT64* trans, FLOAT64* sig, INT32 size, INT16 level)
{
  register INT32 i;
  register INT32 n;
  register INT32 size2;
  FLOAT64*  transTemp;

  if(size < 4)
  {
    return NOT_EXEC;
  } 

  transTemp = (FLOAT64*)dlp_calloc(size, sizeof(FLOAT64));
  dlp_memmove(transTemp, trans, size * sizeof(FLOAT64));

  if(level > 0)
  {
    n = size >> (level-1);
    if(n<4) n=4;
  }
  else
    n=4;

  for (; n <= size; n <<= 1)
  {
    size2 = n >> 1;
    sig[0] = transTemp[size2-1]*h2 + transTemp[size2+size2-1]*h1 + transTemp[0]*h0 + transTemp[size2]*h3;
    sig[1] = transTemp[size2-1]*h3 - transTemp[size2+size2-1]*h0 + transTemp[0]*h1 - transTemp[size2]*h2;
        
    for (i = 0; i < size2-1; i++)
    {
      sig[2*i+2] = transTemp[i]*h2 + transTemp[i+size2]*h1 + transTemp[i+1]*h0 + transTemp[i+size2+1]*h3;
      sig[2*i+3] = transTemp[i]*h3 - transTemp[i+size2]*h0 + transTemp[i+1]*h1 - transTemp[i+size2+1]*h2;
    }
    
    dlp_memmove(transTemp, sig, n * sizeof(FLOAT64));
    level--;
    if(level == 0) break;
  }
 
  dlp_free(transTemp);
  
  return O_K;
}

 /*   Haar transform (from "Ripples in Mathematics")
 *
 *    FLOAT64* sig         signal array
 *    FLOAT64* trans       transformed signal
 *    INT32    size        signal size
 *    INT16   level       detail level (max. level = -1)  
  */
INT16 CGEN_IGNORE dlm_fwt_haar(FLOAT64* sig, FLOAT64* trans, INT32 size, INT16 level)
{
  register INT32 i;
  register INT32 n;
  register INT32 size2;
  FLOAT64* sigTemp;
  

  if(size < 2)
  {
    return NOT_EXEC;
  } 
  
  sigTemp = (FLOAT64*)dlp_calloc(size, sizeof(FLOAT64));
  dlp_memmove(sigTemp, sig, size * sizeof(FLOAT64));
  
  for (n = size; n >= 2; n >>= 1)
  {
    size2 = n >> 1;
    for (i = 0; i < size2; i++)
    {
      trans[i]       = 0.5*(sigTemp[2*i] + sigTemp[i*2+1]);
      trans[i+size2] = sigTemp[i*2] - trans[i];
    }
    dlp_memmove(sigTemp, trans, n * sizeof(FLOAT64));
    level--;
    if(level == 0) break;
  }
  
  dlp_free(sigTemp);
  
  return O_K;
}

 /*   Inverse Haar transform (from "Ripples in Mathematics")
 *
 *    FLOAT64* trans       transformed signal
 *    FLOAT64* sig         signal array
 *    INT32    size        transformed signal array size 
 *    INT16   level       detail level (max. level = -1) that was used to transform signal 
 */
INT16 CGEN_IGNORE dlm_fwt_haar_inv(FLOAT64* trans, FLOAT64* sig, INT32 size, INT16 level)
{
  register INT32 i;
  register INT32 n;
  register INT32 size2;
  FLOAT64*  transTemp;

  if(size < 2)
  {
    return NOT_EXEC;
  } 

  transTemp = (FLOAT64*)dlp_calloc(size, sizeof(FLOAT64));
  dlp_memmove(transTemp, trans, size * sizeof(FLOAT64));

  if(level > 0)
  {
    n = size >> (level-1);
    if(n<2) n=2;
  }
  else
    n=2;
  
  for (; n <= size; n <<= 1)
  {
    size2 = n >> 1;
        
    for (i = 0; i < size2; i++)
    {
      sig[2*i]   = transTemp[i] + transTemp[i+size2];
      sig[2*i+1] = 2*transTemp[i] - sig[2*i];
    }
    dlp_memmove(transTemp, sig, n * sizeof(FLOAT64));
  }
 
  dlp_free(transTemp);
  
  return O_K;
}

