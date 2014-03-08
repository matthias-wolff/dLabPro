/* dLabPro mathematics library
 * - Signal processing algorithms
 *
 * AUTHOR : Rico Petrick
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

/**
 * Time domain convolution of two signals.
 *
 * @param Z
 *          Pointer to output signal, buffer must be able to hold
 *          <code>nLa</code>+<code>nLb</code>-1 double values
 * @param A
 *          Pointer to buffer containing left operand, must not be qual
 *          <code>Z</code>
 * @param nLa
 *          Length of left operand
 * @param B
 *          Pointer to buffer containing right operand, must not be qual
 *          <code>Z</code>
 * @param nLb
 *          Length of right operand
 * @return <code>O_K</code> if successful, a (negative) error code otherwise
 */
INT16 dlm_convol_t
(
  FLOAT64*       Z,
  const FLOAT64* A,
  INT32          nLa,
  const FLOAT64* B,
  INT32          nLb
)
{
  INT32   i,k,I_idx;
  INT32   nZ,nI;
  FLOAT64 *pI;
  FLOAT64 sum;

  DLPASSERT(Z!=A && Z!=B);                                                      /* Assert operands are not the result*/
  DLPASSERT(dlp_size(Z) >= (nLa+nLb-1)*sizeof(FLOAT64));                         /* Check size of result buffer       */
  DLPASSERT(dlp_size(A) >= nLa*sizeof(FLOAT64));                                 /* Check size of left operand buffer */
  DLPASSERT(dlp_size(B) >= nLb*sizeof(FLOAT64));                                 /* Check size of right operand buffer*/

  nZ = nLa +    nLb -1 ;                                                        /* length of output array            */
  nI = nLa + 2*(nLb -1);                                                        /* length of intermediate array      */
  pI = (FLOAT64*)dlp_calloc((nI),sizeof(FLOAT64));                                /* allocate intermediate array       */

  for (i = 0; i < nLa; i++)                                                     /* move A into intermediate array    */
  {                                                                             /* with offset nLb-1                 */
    pI[i + nLb -1] = A[i];                                                      /* note: I needs to be zeroed before */
  }                                                                             /* this was done by calloc           */
  I_idx = nLb -1;                                                               /* set startindex of intermed. array */

  /* convolution loop       */
  for (i=0; i < nZ; i++, I_idx++)
  {
    sum = 0;
    for (k = 0;k < nLb; k++)                                                    /* build output for one i            */
    {
      sum += B[k] * pI[I_idx - k];
    }
    Z[i] = sum;                                                                 /* write to output                   */
  }

  dlp_free(pI);

  return  O_K;                                                                  /* All done successfully             */
}

/**
 * Cross Correlation Function (standardized to max. value; not unbiased)
 *
 * @param Z
 *          Pointer to target buffer, buffer must be able to hold
 *          <code>nLb</code>-1 double values
 * @param A
 *          Pointer to buffer containing first signal, must not be qual
 *          <code>Z</code>.
 * @param nLa
 *          Length of first signal.
 * @param B
 *          Pointer to buffer containing second signal, must not be qual
 *          <code>Z</code>.
 * @param nLb
 *          Length of second signal.
 * @return <code>O_K</code> if successful, a (negative) error code otherwise
 */
INT16 dlm_ccf
(
  FLOAT64*       Z,
  const FLOAT64* A,
  INT32          nLa,
  const FLOAT64* B,
  INT32          nLb
)
{
  INT32   i,k,nZ;
  FLOAT64 sum;

  nZ = nLb ;                                                                    /* length of output array            */
  DLPASSERT(Z!=A && Z!=B);                                                      /* Assert operands are not the result*/
  DLPASSERT(dlp_size(Z) >= nZ*sizeof(FLOAT64));                                 /* Check size of result buffer       */
  DLPASSERT(dlp_size(A) >= nLa*sizeof(FLOAT64));                                /* Check size of first signal buffer */
  DLPASSERT(dlp_size(B) >= nLb*sizeof(FLOAT64));                                /* Check size of second signal buffer*/

  for (i=0; i<nZ; i++)
  {
    sum=0.;
    for (k=0; k<(nLb-i); k++) sum += A[k]*B[k+i];
    Z[i] = (sum/(FLOAT64)nLb);
  }

  return  O_K;
}

INT16 dlm_distribution(FLOAT64* lpR, FLOAT64* lpS, FLOAT64* lpP, INT32 nRS, INT32 nCS, INT32 nCP) {
  INT32 nCounter;
  INT32 iR = 0;
  INT32 iC = 0;
  INT32 iCP = 0;
  FLOAT64 val = 0.0;
  FLOAT64 lim = 0.0;

  for (iR = 0; iR < nRS; iR++) {
    for(iCP = 0; iCP < nCP; iCP++) {
      nCounter = 0;
      lim = lpP[iCP];
      for(iC = 0; iC < nCS; iC++) {
        val = lpS[iC];
        if(val <= lim) {
          nCounter++;
        }
      }
      lpR[iCP] = nCounter;
    }
    lpR += nCP;
    lpS += nCS;
  }
  return O_K;
}

INT16 dlm_distributionC(COMPLEX64* lpR, COMPLEX64* lpS, COMPLEX64* lpP, INT32 nRS, INT32 nCS, INT32 nCP) {
  INT32 nCounter_r;
  INT32 nCounter_i;
  INT32 iR = 0;
  INT32 iC = 0;
  INT32 iCP = 0;
  COMPLEX64 val = CMPLX(0.0);
  COMPLEX64 lim = CMPLX(0.0);

  for (iR = 0; iR < nRS; iR++) {
    for(iCP = 0; iCP < nCP; iCP++) {
      nCounter_r = 0;
      nCounter_i = 0;
      lim = lpP[iCP];
      for(iC = 0; iC < nCS; iC++) {
        val = lpS[iC];
        if(val.x <= lim.x) {
          nCounter_r++;
        }
        if(val.y <= lim.y) {
          nCounter_i++;
        }
      }
      lpR[iCP].x = nCounter_r;
      lpR[iCP].y = nCounter_i;
    }
    lpR += nCP;
    lpS += nCS;
  }
  return O_K;
}

/* EOF */
