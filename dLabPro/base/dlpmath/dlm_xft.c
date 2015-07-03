/* dLabPro mathematics library
 * - Discrete and fast Fourier transform
 *
 * AUTHOR : Matthias Wolff
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

INT16 dlm_fft_log10(FLOAT64* real, FLOAT64* imag, INT32 len, FLOAT64 min_log) {
  INT32 k;
  FLOAT64 tmp;

  real[0] = 0.5 * log10(real[0]*real[0] + imag[0]*imag[0]) - min_log*0.4342944819032518167;
  imag[0] = 0.0;

  for (k = 1; k <= len/2;k++) {
    tmp         = atan2(imag[k], real[k])*0.4342944819032518167;
    real[k]     = 0.5 * log10(real[k]*real[k] + imag[k]*imag[k]) - min_log*0.4342944819032518167;
    real[len-k] = real[k];
    imag[k]     = tmp;
    imag[len-k] = -tmp;
  }

  if((len % 2) == 0) {
    imag[k-1] = 0.0;
  }
  return O_K;
}

INT16 dlm_fft_ln(FLOAT64* real, FLOAT64* imag, INT32 len, FLOAT64 min_log) {
  INT32 k;
  FLOAT64 tmp;

  real[0] = 0.5 * log(real[0]*real[0] + imag[0]*imag[0]) - min_log;
  imag[0] = 0.0;

  for (k = 1; k <= len/2;k++) {
    tmp         = atan2(imag[k], real[k]);
    real[k]     = 0.5 * log(real[k]*real[k] + imag[k]*imag[k]) - min_log;
    real[len-k] = real[k];
    imag[k]     = tmp;
    imag[len-k] = -tmp;
  }

  if((len % 2) == 0) {
    imag[k-1] = 0.0;
  }
  return O_K;
}

INT16 dlm_fft_mag(FLOAT64* real, FLOAT64* imag, INT32 len) {
  INT32 k;
  real[0] = sqrt(real[0]*real[0] + imag[0]*imag[0]);
  imag[0] = 0.0;

  for (k = 1; k <= len/2; k++)  {
    real[k] = real[len-k] = sqrt(real[k]*real[k] + imag[k]*imag[k]);
    imag[k] = imag[len-k] = 0.0;
  }
  return O_K;
}

/**
 * Calculate warped magnitude spectrum from magnitude.
 *
 * @return O_K if successfull, NOT_EXEC otherwise
 */
INT16 dlm_fft_warp(FLOAT64* data_in, FLOAT64* data_out, INT32 len, FLOAT64 lambda) {
  FLOAT64 tmp1;
  FLOAT64 tmp2;
  FLOAT64 tmp4;
  INT32   tmp3;
  INT32   i;
  INT32   len2 =  len / 2;

  data_out[0] = data_in[0];
  for (i = 1; i <= len2; i += 1) {
    tmp1 = i * F_PI / (len2 - 1);
    tmp2 = (tmp1 - 2.0 * atan2(lambda * sin(tmp1), (1.0 + lambda * cos(tmp1)))) * (len2 - 1) / F_PI;
    tmp3 = (INT32) tmp2;
    tmp4 = tmp2 - tmp3;
    data_out[i] = data_in[tmp3] * (1.0 - tmp4) + data_in[tmp3+1] * tmp4;
    data_out[len - i] = data_out[i];
  }
  return O_K;
}

/**
 * Calculate warped complex spectrum from complex spectrum.
 *
 * @return O_K if successfull, NOT_EXEC otherwise
 */
INT16 dlm_fft_warpC(COMPLEX64* data_in, COMPLEX64* data_out, INT32 len, FLOAT64 lambda) {
  FLOAT64 tmp1;
  FLOAT64 tmp2;
  FLOAT64 tmp4;
  INT32   tmp3;
  INT32   i;
  INT32   len2 =  len / 2;

  data_out[0] = data_in[0];
  for (i = 1; i <= len2; i += 1) {
    tmp1 = i * F_PI / (len2 - 1);
    tmp2 = (tmp1 - 2.0 * atan2(lambda * sin(tmp1), (1.0 + lambda * cos(tmp1)))) * (len2 - 1) / F_PI;
    tmp3 = (INT32) tmp2;
    tmp4 = tmp2 - tmp3;
    data_out[i] = CMPLX_PLUS(CMPLX_MULT_R(data_in[tmp3],(1.0-tmp4)), CMPLX_MULT_R(data_in[tmp3+1],tmp4));
    data_out[len - i] = data_out[i];
  }
  return O_K;
}

/*
 * Deallocates sine and cosing tables
 */
void dlm_fft_cleanup()
{
  extern FLOAT64* _sintbl;
  if(_sintbl != NULL) {
    free(_sintbl);
  }
}

/**
 * <p id=dlm_unwrap>Unwraps radian phases given in imaginary part of <code>C</code> to their 2&pi; complement if the
 * phase jumps greater than &pi;.</p>
 *
 * @param S
 *          Pointer to an array containing radian complex numbers, overwritten with the result.
 * @param nSL
 *          Length of input <code>S</code>.
 * @return <code>O_K</code> if successfull, a (negative) error code otherwise
 */
INT16 dlm_unwrapC(COMPLEX64* S, INT32 nSL) {
  INT32 iSL = 0;
  INT16* nCorr = (INT16*) dlp_calloc(nSL, sizeof(INT16));
  if (nCorr == NULL) return ERR_MEM;

  for (iSL = 1; iSL < nSL; iSL++) {
    if (((S + iSL)->y - (S + iSL - 1)->y) > F_PI) *(nCorr + iSL) = *(nCorr + iSL - 1) - 1;
    else if (((S + iSL)->y - (S + iSL - 1)->y) < -F_PI) *(nCorr + iSL) = *(nCorr + iSL - 1) + 1;
    else *(nCorr + iSL) = *(nCorr + iSL - 1);
  }
  for (iSL = 1; iSL < nSL; iSL++) {
    (S + iSL)->y += *(nCorr + iSL) * 2 * F_PI;
  }
  dlp_free(nCorr);
  return O_K;
}

/**
 * <p id=dlm_fft_C>Computes the complex (inverse) fast Fourier transform.</p>
 *
 * @param C
 *          Pointer to an array containing the input, will be
 *          overwritten with the result.
 * @param nXL
 *          Length of signals. If length not power of 2 {@link dlm_dftC} will be
 *          executed.
 * @param bInv
 *          If non-zero the function computes the inverse complex Fourier
 *          transform
 * @return <code>O_K</code> if successfull, a (begative) error code otherwise
 *
 * <h4>Remarks</h4>
 * <ul>
 *   <li>The function creates the tables by its own, stores them in static arrays and
 *   reuses them for all subsequent calls with the same value of
 *   <code>nXL</code>.</li>
 * </ul>
 */
INT16 dlm_fftC
(
  COMPLEX64*     C,
  INT32          nXL,
  INT16          bInv
)
{
  register INT32 i;
  INT16 ret = O_K;
  FLOAT64* RE = (FLOAT64*)dlp_calloc(nXL, sizeof(FLOAT64));
  FLOAT64* IM = (FLOAT64*)dlp_calloc(nXL, sizeof(FLOAT64));

  for(i = nXL-1; i >= 0; i--) {
    RE[i] = C[i].x;
    IM[i] = C[i].y;
  }
  if((ret = dlm_fft(RE,IM,nXL,bInv)) == O_K) {
    for(i = nXL-1; i >= 0; i--) {
      C[i].x = RE[i];
      C[i].y = IM[i];
    }
  }

  dlp_free(RE);
  dlp_free(IM);

  return ret;                                                                   /* All done                          */
}

/**
 * Computes the complex (inverse) fast Fourier transform.
 * 
 * @param RE
 *          Pointer to an array containing the real part of the input, will be
 *          overwritten with real part of output.
 * @param IM
 *          Pointer to an array containing the imaginary part of the input, will
 *          be overwritten with imaginary part of output.
 * @param nXL
 *          Length of signals, must be a power of 2 (otherwise the function will
 *          return an <code>ERR_MDIM</code> error)
 * @param bInv
 *          If non-zero the function computes the inverse complex Fourier
 *          transform
 * @return <code>O_K</code> if successfull, a (begative) error code otherwise
 * 
 * <h4>Remarks</h4>
 * <ul>
 *   <li>The function creates the tables by its own, stores them in static arrays and
 *   reuses them for all subsequent calls with the same value of <code>nXL</code>.</li>
 * </ul>
 */
INT16 dlm_fft
(
  FLOAT64*       RE,
  FLOAT64*       IM,
  INT32          nXL,
  INT16          bInv
)
{
  extern int ifft(FLOAT64*,FLOAT64*,const int);
  extern int fft(FLOAT64*,FLOAT64*,const int);
  INT16  order;

  /* Initialize */                                                               /* --------------------------------- */
  order = (INT16)dlm_log2_i(nXL);                                                /* Compute FFT order                 */
  if (order<=0) return ERR_MDIM;                                                 /* nXL is not power of 2 --> ://     */
#ifndef __NOXALLOC
  DLPASSERT(dlp_size(RE)>=nXL*sizeof(FLOAT64));                                  /* Assert RE has the right length    */
  DLPASSERT(dlp_size(IM)>=nXL*sizeof(FLOAT64));                                  /* Assert IM has the right length    */
#endif

  if(bInv) {
    if(ifft(RE,IM,nXL) != 0) return NOT_EXEC;
  } else {
    if(fft(RE,IM,nXL) != 0) return NOT_EXEC;
  }

  return O_K;                                                                    /* All done                          */
}

/* EOF */
