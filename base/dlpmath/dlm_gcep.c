/* dLabPro mathematics library
 * - Generalized cepstrum analysis methods
 *
 * AUTHOR : Guntram Strecha
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
 * <p id=dlm_gmult>Gamma multiply.</p>
 *
 * <p>This function multiplies the Generalized Cepstrum coefficients by gamma, except the zero<sup>th</sup>
 * coefficient.</p>
 *
 * @param gc_in  Pointer to input buffer.
 * @param gc_out Pointer to output buffer.
 * @param n      Number of coefficients, i.e. length of in/out buffer.
 * @param gamma  Generalized cepstrum factor.
 * @return <code>O_K</code>
 */
INT16 dlm_gmult(FLOAT64* gc_in, FLOAT64* gc_out, INT32 n, FLOAT64 gamma) {
  INT32 i = 0;
  for (i = n-1; i > 0; i--)
    gc_out[i] = gc_in[i] * gamma;
  gc_out[0] = gc_in[0];
  return O_K;
}

/**
 * <p id=dlm_gmult>Gamma multiply.</p>
 *
 * <p>This function divides the Generalized Cepstrum coefficients by gamma, except the zero<sup>th</sup>
 * coefficient.</p>
 *
 * @param gc_in  Pointer to input buffer.
 * @param gc_out Pointer to output buffer.
 * @param n      Number of coefficients, i.e. length of in/out buffer.
 * @param gamma  Generalized cepstrum factor.
 * @return <code>O_K</code>
 */
INT16 dlm_igmult(FLOAT64* gc_in, FLOAT64* gc_out, INT32 n, FLOAT64 gamma) {
  return dlm_gmult(gc_in, gc_out, n, 1.0/gamma);
}

/**
 * <p id=dlm_gnorm>Gain normalization.</p>
 *
 * <p>This function normalizes Generalized cepstrum coefficients using the gain given in the zero<sup>th</sup>
 * coefficient.</p>
 *
 * @param gc_in  Pointer to input buffer.
 * @param gc_out Pointer to output buffer.
 * @param n      Number of coefficients, i.e. length of in/out buffer.
 * @param gamma  Generalized cepstrum factor.
 * @return <code>O_K</code>
 */
INT16 dlm_gnorm(FLOAT64* gc_in, FLOAT64* gc_out, INT32 n, FLOAT64 gamma) {
  extern void gnorm(FLOAT64*,FLOAT64*,INT32,const FLOAT64);

  gnorm(gc_in, gc_out, n, gamma);
  return O_K;
}

/**
 * <p id=dlm_ignorm>Inverse gain normalization.</p>
 *
 * <p>This function denormalizes the Generalized Cepstrum coefficients using the gain given in the zero<sup>th</sup>
 * coefficient. This function is the inverse of {@link dlm_gnorm}.</p>
 *
 * @param gc_in  Pointer to input buffer.
 * @param gc_out Pointer to output buffer.
 * @param n      Number of coefficients, i.e. length of in/out buffer.
 * @param gamma  Generalized cepstrum factor.
 * @return <code>O_K</code>
 */
INT16 dlm_ignorm(FLOAT64* gc_in, FLOAT64* gc_out, INT32 n, FLOAT64 gamma) {
  extern void ignorm(FLOAT64*,FLOAT64*,INT32,const FLOAT64);

  ignorm(gc_in, gc_out, n, gamma);
  return O_K;
}

INT16 dlm_gc2gc(FLOAT64* gc_in, INT32 n_in, FLOAT64 gamma_in, FLOAT64* gc_out, INT32 n_out, FLOAT64 gamma_out) {
  extern void gc2gc(FLOAT64*,const INT32,const FLOAT64,FLOAT64*,const INT32,const FLOAT64);

  gc2gc(gc_in, n_in, gamma_in, gc_out, n_out, gamma_out);
  return O_K;
}

/**
 * <p id=dlm_mgcep2mgcep>Mel-Generalized Cepstral Transform.</p>
 *
 * <p>This function transforms the Mel-Generalized Cepstrum with warping factor &lambda;<sub>1</sub> and &gamma;<sub>1</sub>
 * to Mel-Generalized Cepstrum with warping factor &lambda;<sub>2</sub> and &gamma;<sub>2</sub>.</p>
 * <p>The Mel-Generalized Cepstrum is assumed to be in normalized form.
 *
 * @param mgc_in     Pointer to input buffer.
 * @param n_in       Number of coefficients, i.e. length of mgc_in buffer.*
 * @param gamma_in   Generalized cepstrum factor of mgc_in.
 * @param lambda_in  Warping factor of mgc_in.
 * @param mgc_out    Pointer to output buffer.
 * @param n_out      Number of coefficients, i.e. length of mgc_out buffer.
 * @param gamma_out  Generalized cepstrum factor of mgc_out.
 * @param lambda_out Warping factor of mgc_out.
 * @return <code>O_K</code>
 */
INT16 dlm_mgcep2mgcep(FLOAT64* mgc_in, INT32 n_in, FLOAT64 gamma_in, FLOAT64 lambda_in, FLOAT64* mgc_out, INT32 n_out, FLOAT64 gamma_out, FLOAT64 lambda_out) {
  FLOAT64 lambda = (lambda_out - lambda_in) / (1 - lambda_in * lambda_out);

  dlm_gnorm(mgc_in, mgc_in, n_in, gamma_in);
  if (lambda == 0) {
     dlm_gc2gc(mgc_in, n_in, gamma_in, mgc_out, n_out, gamma_out);
  } else {
     dlm_freqt(mgc_in, n_in, mgc_out, n_out, lambda);
     dlm_gc2gc(mgc_out, n_out, gamma_in, mgc_out, n_out, gamma_out);
  }
  dlm_ignorm(mgc_out, mgc_out, n_out, gamma_out);
  return O_K;
}

/**
 * <p id=dlm_gcep2gcep>Generalized Cepstral Transform.</p>
 *
 * <p>This function transforms the Generalized Cepstrum with &gamma;<sub>1</sub>
 * to Generalized Cepstrum with &gamma;<sub>2</sub>.</p>
 * <p>The Generalized Cepstrum is assumed to be in normalized form.
 *
 * @param mgc_in     Pointer to input buffer.
 * @param n_in       Number of coefficients, i.e. length of mgc_in buffer.*
 * @param gamma_in   Generalized cepstrum factor of mgc_in.
 * @param mgc_out    Pointer to output buffer.
 * @param n_out      Number of coefficients, i.e. length of mgc_out buffer.
 * @param gamma_out  Generalized cepstrum factor of mgc_out.
 * @return <code>ERR_MEM</code> on memory allocation failure, <code>O_K</code> otherwise.
 */
INT16 dlm_gcep2gcep(FLOAT64* gc_in, INT32 n_in, FLOAT64 gamma_in, FLOAT64* gc_out, INT32 n_out, FLOAT64 gamma_out) {
  return dlm_mgcep2mgcep(gc_in, n_in, gamma_in, 0.0, gc_out, n_out, gamma_out, 0.0);
}

/**
 * <p id=dlm_gcep>Generalized Cepstral analysis of (windowed) signal frame.</p>
 *
 * <p>This function calculates <code>order</code> Generalized Cepstrum coefficients from signal frame of length
 * <code>n</code> given in <code>input</code> using given Generalized Cepstrum parameter <code>-1&le;gamma<0</code>.
 *
 *@param input Pointer to signal frame.
 *@param n     Input frame length.
 *@param output Pointer to output.
 *@param order  Number of Generalized Cepstrum coefficients to calculate.
 *@param gamma  Generalized Cepstrum parameter.
 *@param lambda Warping factor.
 *@param scale  Scale factor of input data.
 *@return <code>NOT_EXEC</code> if iteration exceeds limit, <code>O_K</code> otherwise.
 */
INT16 dlm_mgcep(FLOAT64* input, INT32 n, FLOAT64* output, INT16 order, FLOAT64 gamma, FLOAT64 lambda, FLOAT64 scale) {
  INT16      itr1 = 2;
  INT16      itr2 = 30;
  INT16      m = order - 1;
  INT32      i;
  INT32      j;
  INT32      k;
  BOOL       flag = FALSE;
  FLOAT64    dd = 0.000001;
  FLOAT64    ep = 0.0;
  FLOAT64    tmp1;
  FLOAT64    tmp2;
  FLOAT64    det;
  COMPLEX64* lpS_C    = (COMPLEX64*)dlp_calloc(5*n, sizeof(COMPLEX64));
  COMPLEX64* lpG_C    = lpS_C + n;
  COMPLEX64* lpPsiR_C = lpG_C + n;
  COMPLEX64* lpPsiP_C = lpPsiR_C + n;
  COMPLEX64* lpPsiQ_C = lpPsiP_C + n;
  FLOAT64*   lpD      = (FLOAT64*)dlp_calloc(m, sizeof(FLOAT64));
  FLOAT64*   lpH      = (FLOAT64*)dlp_calloc(m*m, sizeof(FLOAT64));
  FLOAT64*   lpA      = (FLOAT64*)dlp_calloc(m*m, sizeof(FLOAT64));
  FLOAT64*   lpT      = (FLOAT64*)dlp_calloc(m*m, sizeof(FLOAT64));
  FLOAT64*   lpPsiR_R = (FLOAT64*)dlp_calloc(5*n, sizeof(FLOAT64));
  FLOAT64*   lpPsiP_R = lpPsiR_R + n;
  FLOAT64*   lpPsiQ_R = lpPsiP_R + n;
  FLOAT64*   lpG_R    = lpPsiQ_R + n;
  FLOAT64*   lpDummy  = lpG_R + n;

  *lpDummy = 1.0;

  for(i = n-1; i >= 0; i--) lpS_C[i].x = input[i];

  dlm_fftC(lpS_C, n, FALSE);
  for (i = 0; i < n; i ++) lpS_C[i] = CMPLX(dlm_pow(CMPLX_ABS(lpS_C[i]),2.0));

  dlm_lpc_mburg(input, n, output, order, lambda, scale);
  dlm_gc2gc(output, m, -1, output, m, gamma);

  for(j = 0; (j < itr2) && !flag; j++) {
    ep = output[0];
    for(i = order-1; i > 0; i--) output[i] *= gamma;
    output[0] = 1.0;

    dlm_filter_freqt_fir(output, order, lpG_R, n, -lambda);

    for(i = n-1; i >= 0; i--) lpG_C[i] = CMPLX(lpG_R[i]);
    dlm_fftC(lpG_C, n, FALSE);

    for(i = n-1; i >= 0; i--) {
      tmp1 = dlp_scalop(CMPLX_ABS(dlp_scalopC(lpG_C[i], CMPLX(1.0+1.0/gamma), OP_POW)), 2.0, OP_POW);
      tmp2 = dlp_scalop(CMPLX_ABS(dlp_scalopC(lpG_C[i], CMPLX(2.0+1.0/gamma), OP_POW)), 2.0, OP_POW);
      lpPsiR_C[i] = CMPLX_DIV_R(CMPLX_MULT_R(lpG_C[i], lpS_C[i].x), tmp1);
      lpPsiP_C[i] = CMPLX(lpS_C[i].x / tmp1);
      lpPsiQ_C[i] = CMPLX_SQR(lpG_C[i]);lpPsiQ_C[i] = CMPLX_DIV_R(CMPLX_MULT_R(lpPsiQ_C[i], lpS_C[i].x), tmp2);
    }

    dlm_fftC(lpPsiR_C, n, TRUE);
    dlm_fftC(lpPsiP_C, n, TRUE);
    dlm_fftC(lpPsiQ_C, n, TRUE);

    if(lambda != 0.0) {
      dlp_memset(lpPsiP_R, 0, 3*n*sizeof(FLOAT64));
      for(i = n/2; i > 0; i--) {
        lpPsiR_R[i] = 2.0 * lpPsiR_C[i].x;
        lpPsiP_R[i] = 2.0 * lpPsiP_C[i].x;
        lpPsiQ_R[i] = 2.0 * lpPsiQ_C[i].x;
      }
      lpPsiR_R[0] = lpPsiR_C[0].x;
      lpPsiP_R[0] = lpPsiP_C[0].x;
      lpPsiQ_R[0] = lpPsiQ_C[0].x;

      dlm_filter_freqt_fir(lpPsiR_R, n/2, lpPsiR_R, n, lambda);
      dlm_filter_freqt_fir(lpPsiP_R, n/2, lpPsiP_R, n, lambda);
      dlm_filter_freqt_fir(lpPsiQ_R, n/2, lpPsiQ_R, n, lambda);

      for(i = n-1; i > 0; i--) {
        lpPsiR_R[i] = 0.5 * lpPsiR_R[i];
        lpPsiP_R[i] = 0.5 * lpPsiP_R[i];
        lpPsiQ_R[i] = 0.5 * (1.0+gamma) * lpPsiQ_R[i];
      }
    } else {
      for(i = n-1; i >= 0; i--) {
        lpPsiR_R[i] = lpPsiR_C[i].x;
        lpPsiP_R[i] = lpPsiP_C[i].x;
        lpPsiQ_R[i] = (1.0+gamma) * lpPsiQ_C[i].x;
      }
    }

    for(i = 0; i < m; i++) {
      for(k = 0; k < m; k++) {
        lpH[i+k*m] = 2.0 * (lpPsiP_R[ABS(k-i)] + lpPsiQ_R[k+i+2]);
      }
    }

    dlm_transpose(lpT,lpH,m,m);
    dlm_mult(lpA,lpT,m,m,lpH,m,m);
    dlm_invert_gel(lpA,m,&det);
    dlm_mult(lpH,lpA,m,m,lpT,m,m);
    dlm_mult(lpD,lpH,m,m,lpPsiR_R+1,m,1);

    for(i = 0; i <  m; i++) output[i+1] = output[i+1]/gamma + 2.0 * lpD[i];

    output[0] = lpPsiR_R[0];
    for(i = 1; i <= m; i++) output[0] += gamma * output[i] * lpPsiR_R[i];
    output[0] = sqrt(output[0])*scale;

    if(j > itr1) {
      if((ep-output[0])/output[0] < dd) {
        flag = TRUE;
      }
    }
  }

  dlm_ignorm(output, output, m, gamma);

  dlp_free(lpS_C);
  dlp_free(lpD);
  dlp_free(lpA);
  dlp_free(lpH);
  dlp_free(lpT);
  dlp_free(lpPsiR_R);

  return (flag == TRUE) ? O_K : NOT_EXEC;
}
