/* dLabPro mathematics library
 * - Cepstrum analysis methods
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

#define MAX_PADE_ORDER 7

#ifdef EPS
  #undef EPS
#endif
#define EPS 1.0e-20

/**
 * <p>Conversion of LPC to mel-Cepstrum.</p>
 *
 * @param a
 *          Pointer to LPC coefficients <CODE>a[i]</CODE> (i = 0...<CODE>a_len</CODE> - 1).
 * @param a_len
 *          Order of LPC analysis equals to the dimension of <CODE>a</CODE>.
 * @param c
 *          Pointer to resulting cepstral coefficients <CODE>c[j]</CODE> (j = 0...<CODE>c_len</CODE> - 1).
 * @param c_len
 *          Order of cepstrum equals to the dimension of <CODE>c</CODE>.
 * @param lambda
 *          Warping factor.
 * @return <code>O_K</code> if successfull, a (negative) error code otherwise
 */
INT16 dlm_lpc2mcep(FLOAT64* a, INT16 a_len, FLOAT64* c, INT16 c_len, FLOAT64 lambda) {
  INT16    m;
  INT16    i;
  INT16    am_len = c_len;
  FLOAT64  k;
  FLOAT64  km;
  FLOAT64  sum;
  FLOAT64* am = NULL;

  am = (FLOAT64*)dlp_calloc(am_len, sizeof(FLOAT64));

  k = a[0];

  a[0] = 1.0;

  dlm_lpc2mlpc(a, a_len, am, am_len, lambda);

  km = k / am[0];

  for(m = am_len - 1; m >= 0; m--) am[m] /= am[0];

  c[0] = log(km);

  for(m = 1; m < c_len; m++) {
    sum = 0.0;
    if(m < a_len) {
      for(i = 1; i < m; i++) {
        sum += (FLOAT64)(m - i) * c[m - i] * am[i];
      }
      c[m] = -am[m] - sum / (FLOAT64)m;
    } else {
      for(i = 1; i < a_len; i++) {
        sum += (FLOAT64)(m - i) * c[m - i] * am[i];
      }
      c[m] = -sum / (FLOAT64)m;
    }
  }

  dlp_free(am);
  return O_K;
}

/**
 * <p>Cepstral analysis of (windowed) signal frame.</p>
 *
 * @param samples
 *          Pointer to the (windowed) signal frame
 * @param n_samples
 *          Number of samples given in <CODE>samples</CODE>.
 * @param c
 *          Pointer to resulting cepstral coefficients <CODE>c[j]</CODE> (j = 0...<CODE>c_len</CODE> -1).
 * @param c_len
 *          Order of cepstrum equals to the dimension of <CODE>c</CODE>.
 * @param lambda
 *          Warping factor.
 * @param scale
 *          Signal scaling factor.
 * @param method
 *          Method used for cepstrum calculation.<br>
 *             Possible values:<br>
 *               - <CODE>DLM_CALCCEP_METHOD_S_LPC_BURG_CEP</CODE><br>
 *               - <CODE>DLM_CALCCEP_METHOD_S_LPC_BURG_MCEP</CODE><br>
 *               - <CODE>DLM_CALCCEP_METHOD_S_LPC_BURG_CEP_MCEP</CODE><br>
 *               - <CODE>DLM_CALCCEP_METHOD_S_LPC_BURG_MLPC_MCEP</CODE><br>
 *               - <CODE>DLM_CALCCEP_METHOD_S_MLPC_BURG_MCEP</CODE><br>
 *               - <CODE>DLM_CALCCEP_METHOD_S_LPC_LEVI_CEP</CODE><br>
 *               - <CODE>DLM_CALCCEP_METHOD_S_LPC_LEVI_MCEP</CODE><br>
 *               - <CODE>DLM_CALCCEP_METHOD_S_LPC_LEVI_CEP_MCEP</CODE><br>
 *               - <CODE>DLM_CALCCEP_METHOD_S_LPC_LEVI_MLPC_MCEP</CODE><br>
 *               - <CODE>DLM_CALCCEP_METHOD_S_MLPC_LEVI_MCEP</CODE><br>
 *               - <CODE>DLM_CALCCEP_METHOD_S_CEP_UELS</CODE><br>
 *               - <CODE>DLM_CALCCEP_METHOD_S_MCEP_UELS</CODE><br>
 *               - <CODE>DLM_CALCCEP_METHOD_S_FFT_CEP</CODE><br>
 *               - <CODE>DLM_CALCCEP_METHOD_S_FFT_CEP_MCEP</CODE><br>
 *               - <CODE>DLM_CALCCEP_METHOD_S_FFT_MFFT_MCEP</CODE><br>
 * @return <code>O_K</code> if successfull, a (negative) error code otherwise
 */
INT16 dlm_calcmcep(FLOAT64* samples, INT32 n_samples, FLOAT64* c, INT16 c_len, FLOAT64 lambda, FLOAT64 scale, INT16 method) {
  INT32 nFft = 0;
  INT16 i;
  FLOAT64* real = NULL;
  FLOAT64* imag = NULL;

  switch(method) {
  case DLM_CALCCEP_METHOD_S_LPC_BURG_CEP:
    if(dlm_lpc_burg(samples, n_samples, c, c_len, exp(scale)) != O_K) return NOT_EXEC;
    if(dlm_gmult(c, c, c_len, -1.0) != O_K) return NOT_EXEC;
    if(dlm_lpc2mcep(c, c_len, c, c_len, 0.0) != O_K) return NOT_EXEC;
    break;
  case DLM_CALCCEP_METHOD_S_LPC_BURG_MCEP:
    if(dlm_lpc_burg(samples, n_samples, c, c_len, exp(scale)) != O_K) return NOT_EXEC;
    if(dlm_gmult(c, c, c_len, -1.0) != O_K) return NOT_EXEC;
    if(dlm_lpc2mcep(c, c_len, c, c_len, lambda) != O_K) return NOT_EXEC;
    break;
  case DLM_CALCCEP_METHOD_S_LPC_BURG_CEP_MCEP:
    if(dlm_lpc_burg(samples, n_samples, c, c_len, exp(scale)) != O_K) return NOT_EXEC;
    if(dlm_gmult(c, c, c_len, -1.0) != O_K) return NOT_EXEC;
    if(dlm_lpc2mcep(c, c_len, c, c_len, 0.0) != O_K) return NOT_EXEC;
    if(dlm_cep2mcep(c, c_len, c, c_len, lambda, NULL) != O_K) return NOT_EXEC;
    break;
  case DLM_CALCCEP_METHOD_S_LPC_BURG_MLPC_MCEP:
    if(dlm_lpc_burg(samples, n_samples, c, c_len, exp(scale)) != O_K) return NOT_EXEC;
    if(dlm_lpc2mlpc(c, c_len, c, c_len, lambda) != O_K) return NOT_EXEC;
    if(dlm_gmult(c, c, c_len, -1.0) != O_K) return NOT_EXEC;
    if(dlm_lpc2mcep(c, c_len, c, c_len, 0.0) != O_K) return NOT_EXEC;
    break;
  case DLM_CALCCEP_METHOD_S_MLPC_BURG_MCEP:
    if(dlm_lpc_mburg(samples, n_samples, c, c_len, lambda, exp(scale)) != O_K) return NOT_EXEC;
    if(dlm_gmult(c, c, c_len, -1.0) != O_K) return NOT_EXEC;
    if(dlm_lpc2mcep(c, c_len, c, c_len, 0.0) != O_K) return NOT_EXEC;
    break;
  case DLM_CALCCEP_METHOD_S_LPC_LEVI_CEP:
    if(dlm_lpc_mlev(samples, n_samples, c, c_len, 0.0, exp(scale)) != O_K) return NOT_EXEC;
    if(dlm_gmult(c, c, c_len, -1.0) != O_K) return NOT_EXEC;
    if(dlm_lpc2mcep(c, c_len, c, c_len, 0.0) != O_K) return NOT_EXEC;
    break;
  case DLM_CALCCEP_METHOD_S_LPC_LEVI_MCEP:
    if(dlm_lpc_mlev(samples, n_samples, c, c_len, 0.0, exp(scale)) != O_K) return NOT_EXEC;
    if(dlm_gmult(c, c, c_len, -1.0) != O_K) return NOT_EXEC;
    if(dlm_lpc2mcep(c, c_len, c, c_len, lambda) != O_K) return NOT_EXEC;
    break;
  case DLM_CALCCEP_METHOD_S_LPC_LEVI_CEP_MCEP:
    if(dlm_lpc_mlev(samples, n_samples, c, c_len, 0.0, exp(scale)) != O_K) return NOT_EXEC;
    if(dlm_gmult(c, c, c_len, -1.0) != O_K) return NOT_EXEC;
    if(dlm_lpc2mcep(c, c_len, c, c_len, 0.0) != O_K) return NOT_EXEC;
    if(dlm_cep2mcep(c, c_len, c, c_len, lambda, NULL) != O_K) return NOT_EXEC;
    break;
  case DLM_CALCCEP_METHOD_S_LPC_LEVI_MLPC_MCEP:
    if(dlm_lpc_mlev(samples, n_samples, c, c_len, 0.0, exp(scale)) != O_K) return NOT_EXEC;
    if(dlm_lpc2mlpc(c, c_len, c, c_len, lambda) != O_K) return NOT_EXEC;
    if(dlm_gmult(c, c, c_len, -1.0) != O_K) return NOT_EXEC;
    if(dlm_lpc2mcep(c, c_len, c, c_len, 0.0) != O_K) return NOT_EXEC;
    break;
  case DLM_CALCCEP_METHOD_S_MLPC_LEVI_MCEP:
    if(dlm_lpc_mlev(samples, n_samples, samples, n_samples, lambda, exp(scale)) != O_K) return NOT_EXEC;
    if(dlm_gmult(c, c, c_len, -1.0) != O_K) return NOT_EXEC;
    if(dlm_lpc2mcep(samples, n_samples, samples, c_len, 0.0) != O_K) return NOT_EXEC;
    break;
  case DLM_CALCCEP_METHOD_S_CEP_UELS:
  case DLM_CALCCEP_METHOD_S_MCEP_UELS:
    if(dlm_mcep_uels_sptk(samples, n_samples, c, c_len, lambda, scale) != O_K) return NOT_EXEC;
    break;
  case DLM_CALCCEP_METHOD_S_FFT_CEP:
  case DLM_CALCCEP_METHOD_S_FFT_CEP_MCEP:
  case DLM_CALCCEP_METHOD_S_FFT_MFFT_MCEP:
    nFft = 1 << (((INT16)dlm_log2_i(n_samples))+1);
    real = (FLOAT64*)dlp_malloc(nFft*2);
    imag = real + nFft;
    dlp_memmove(real, samples, nFft*sizeof(FLOAT64));
    dlm_fft(real,imag,nFft,FALSE);
    dlm_fft_mag(real, imag, nFft);
    dlm_fft_ln(real, NULL, nFft, 0.0);
    break;
  default:
    return NOT_EXEC;
  }
  for(i = 0; i < c_len; i++) {
    if(!dlp_finite(c[i])) {
      return NOT_EXEC;
    }
  }

  return O_K;
}

/**
 * <p>Bilinear transformation of cepstral coefficients.</p>
 *
 * @param c1
 *          Pointer to cepstral coefficients <CODE>c1[i]</CODE> (i = 0...<CODE>m1</CODE> - 1).
 * @param m1
 *          Order of cepstrum equals to the dimension of <CODE>c1</CODE>.
 * @param c2
 *          Pointer to transformaed cepstral coefficients <CODE>c2[i]</CODE> (i = 0...<CODE>m2</CODE> - 1).
 * @param m2
 *          Order of cepstrum equals to the dimension of <CODE>c2</CODE>.
 * @param lambda
 *          Warping factor.
 * @param d
 *          Temporary array of length 2*m2. Can be NULL.
 * @return <code>O_K</code> if successfull, a (negative) error code otherwise
 * @see     dlm_mcep2cep
 */
INT16 dlm_cep2mcep(FLOAT64* c1, INT32 m1, FLOAT64* c2, INT32 m2, FLOAT64 lambda, FLOAT64* d) {
  INT32    i = 0;
  INT32    j = 0;
  FLOAT64  b = 1 - lambda * lambda;
  FLOAT64* g = NULL;
  BOOL     need_free = FALSE;

  if(d == NULL) {
    d = (FLOAT64*)dlp_calloc(m2+m2, sizeof(FLOAT64));
    need_free = TRUE;
  }

  g = d + m2;

  if(!g || !d) return ERR_MEM;

  for(i = m1 - 1; i >= 0; i--) {

    *(d + 0) = *(g + 0);
    *(g + 0) = *(c1 + i) + lambda * *(g + 0);

    if(m2 > 0) {
      *(d + 1) = *(g + 1);
      *(g + 1) = b * *(d + 0) + lambda * *(g + 1);
    }

    for(j = 2; j < m2; j++) {
      *(d + j) = *(g + j);
      *(g + j) = *(d + j - 1) + lambda * (*(g + j) - *(g + j - 1));
    }
  }

  for( i = 0; i < m2; ++i ) c2[i] = g[i];

  if(need_free) dlp_free(d);

  return O_K;
}

/**
 * <p>Bilinear back-transformation of cepstral coefficients.<br>This is equal to calling dlm_cep2mcep
 * with lambda = -lambda</p>
 *
 * @param c1
 *          Pointer to cepstral coefficients <CODE>c1[i]</CODE> (i = 0...<CODE>m1</CODE> - 1).
 * @param m1
 *          Order of cepstrum equals to the dimension of <CODE>c1</CODE>.
 * @param c2
 *          Pointer to transformaed cepstral coefficients <CODE>c2[i]</CODE> (i = 0...<CODE>m2</CODE> - 1).
 * @param m2
 *          Order of cepstrum equals to the dimension of <CODE>c2</CODE>.
 * @param lambda
 *          Warping factor.
 * @param d
 *          Temporary array of length 2*m2. Can be NULL.
 * @return <code>O_K</code> if successfull, a (negative) error code otherwise
 * @see     dlm_cep2mcep
 */
INT16 dlm_mcep2cep(FLOAT64* c1, INT32 m1, FLOAT64* c2, INT32 m2, FLOAT64 lambda, FLOAT64* d) {
  return dlm_cep2mcep(c1, m1, c2, m2, -lambda, d);
}

/**
 * <p>Bilinear transformation of mel-cepstral coefficients to mel-cepstral coefficients.</p>
 *
 * @param c1
 *          Pointer to cepstral coefficients <CODE>c1[i]</CODE> (i = 0...<CODE>m1</CODE> - 1).
 * @param m1
 *          Order of cepstrum equals to the dimension of <CODE>c1</CODE>.
 * @param c2
 *          Pointer to transformaed cepstral coefficients <CODE>c2[i]</CODE> (i = 0...<CODE>m2</CODE> - 1).
 * @param m2
 *          Order of cepstrum equals to the dimension of <CODE>c2</CODE>.
 * @param lambda1
 *          Warping factor entangled in <code>c1</c1>.
 * @param lambda2
 *          Target warping factor.
 * @param d
 *          Temporary array of length 2*m2. Can be NULL.
 * @return <code>O_K</code> if successfull, a (negative) error code otherwise
 * @see     dlm_mcep2cep
 * @see     dlm_cep2mcep
 */
INT16 dlm_mcep2mcep(FLOAT64* c1, INT32 m1, FLOAT64* c2, INT32 m2, FLOAT64 lambda1, FLOAT64 lambda2, FLOAT64* d) {
  return dlm_cep2mcep(c1, m1, c2, m2, (lambda2-lambda1)/(1-lambda2*lambda1), d);
}

/**
 * <p id="dlm_mcep2b">Transform of (warped) coefficients to filter coefficients</p>
 *
 * @param c
 *          Pointer to filter coefficients <CODE>c</CODE>(k) (k=0..m-1).
 * @param b
 *          Pointer to transformed filter coefficients <CODE>b</CODE>(k) (k=0..m-1).
 * @param m
 *          Number of filter coefficients <CODE>c</CODE> and <code>b</code>.
 * @param lambda
 *          Warping factor.
 * @return <code>O_K</code> if successfull, a (negative) error code otherwise
 *
 * @see   dlm_filter_m
 */
INT16 dlm_mcep2b(FLOAT64* mc, FLOAT64* b, INT32 m, FLOAT64 lambda) {
  extern void mc2b(FLOAT64*, FLOAT64*, INT32 , const FLOAT64);

  mc2b(mc, b, m-1, lambda);
  return O_K;
}

INT16 CGEN_IGNORE dlm_b2mcep(FLOAT64* b, FLOAT64* mc, INT32 m, FLOAT64 lambda) {
  extern void b2mc(FLOAT64*, FLOAT64*, INT32, const FLOAT64);

  b2mc(b, mc, m-1, lambda);
  return O_K;
}

/**
 * <p>Cepstral analysis of (windowed) signal frame using UELS method according to SPTK.</p>
 *
 * @param samples
 *          Pointer to the (windowed) signal frame
 * @param n_samples
 *          Number of samples given in <CODE>samples</CODE>.
 * @param mc
 *          Pointer to resulting (mel-)cepstral coefficients <CODE>c[j]</CODE> (j = 0...<CODE>c_len</CODE> -1).
 * @param n_order
 *          Order of cepstrum equals to the dimension of <CODE>c</CODE>.
 * @param lambda
 *          Warping factor.
 * @param scale
 *          Signal scaling factor.
 * @return <code>O_K</code> if successfull, a (negative) error code otherwise
 */
INT16 dlm_mcep_uels_sptk(FLOAT64* samples, INT32 n_samples, FLOAT64* mc, INT32 n_order, FLOAT64 lambda, FLOAT64 scale) {
  INT32      i       = 0;
  BOOL       ret     = 0;
  FLOAT64*   xw      = (FLOAT64*)dlp_calloc(n_samples, sizeof(COMPLEX64));
  extern int mcep(FLOAT64*,const INT32,FLOAT64*,const INT32,const FLOAT64,const INT32,const INT32,const FLOAT64,const FLOAT64,const FLOAT64,const INT32);

  for(i = n_samples-1; i >= 0; i--) xw[i] = samples[i] * scale;

  ret = mcep(xw, n_samples, mc, n_order-1, lambda, 2, 60, 0.001, 0.000001, 0.0, 0);

  dlp_free(xw);

  if (ret == 0) return O_K;
  else return NOT_EXEC;

}

/**
 * <p>Cepstral analysis of (windowed) signal frame using UELS method without performance optimizations.</p>
 *
 * @param samples
 *          Pointer to the (windowed) signal frame
 * @param n_samples
 *          Number of samples given in <CODE>samples</CODE>.
 * @param mc
 *          Pointer to resulting (mel-)cepstral coefficients <CODE>c[j]</CODE> (j = 0...<CODE>c_len</CODE> -1).
 * @param n_order
 *          Order of cepstrum equals to the dimension of <CODE>c</CODE>.
 * @param lambda
 *          Warping factor.
 * @param scale
 *          Signal scaling factor.
 * @return <code>O_K</code> if successfull, a (negative) error code otherwise
 */
INT16 dlm_mcep_uels(FLOAT64* samples, INT32 n_samples, FLOAT64* mc, INT32 n_order, FLOAT64 lambda, FLOAT64 scale) {
  INT16      itr1     = 2;
  INT16      itr2     = 30;
  INT16      m        = n_order - 1;
  INT32      i        = 0;
  INT32      j        = 0;
  INT32      k        = 0;
  BOOL       flag     = FALSE;
  FLOAT64    dd       = 0.000001;
  FLOAT64    ep       = 0.0;
  FLOAT64    det      = 0.0;
  FLOAT64    tmp      = 0.0;;
  COMPLEX64* lpS_C    = (COMPLEX64*)dlp_calloc(3*n_samples, sizeof(COMPLEX64));
  COMPLEX64* lpG_C    = lpS_C + n_samples;
  COMPLEX64* lpPsiE_C = lpG_C + n_samples;
  FLOAT64*   lpD      = (FLOAT64*)dlp_calloc(m, sizeof(FLOAT64));
  FLOAT64*   lpH      = (FLOAT64*)dlp_calloc(m*m, sizeof(FLOAT64));
  FLOAT64*   lpA      = (FLOAT64*)dlp_calloc(m*m, sizeof(FLOAT64));
  FLOAT64*   lpT      = (FLOAT64*)dlp_calloc(m*m, sizeof(FLOAT64));
  FLOAT64*   lpPsiE_R = (FLOAT64*)dlp_calloc(3*n_samples, sizeof(FLOAT64));
  FLOAT64*   lpG_R    = lpPsiE_R + n_samples;
  FLOAT64*   lpDummy  = lpG_R + n_samples;

  *lpDummy = 1.0;

  for(i = n_samples-1; i >= 0; i--) lpS_C[i].x = samples[i];

  dlm_fftC(lpS_C, n_samples, FALSE);
  for (i = 0; i < n_samples; i ++) lpS_C[i] = CMPLX(dlm_pow(CMPLX_ABS(lpS_C[i]),2.0));

  dlm_lpc_mburg(samples, n_samples, mc, n_order, lambda, scale);
  dlm_gc2gc(mc, n_order, -1, mc, n_order, 0.0);

  for(j = 0; (j < itr2) && !flag; j++) {
    ep = mc[0];
    mc[0] = 0.0;

    dlm_filter_freqt_fir(mc, n_order, lpG_R, n_samples, -lambda);

    for(i = n_samples-1; i >= 0; i--) lpG_C[i] = CMPLX(lpG_R[i]);
    dlm_fftC(lpG_C, n_samples, FALSE);

    for(i = n_samples-1; i >= 0; i--) {
      tmp = exp(lpG_C[i].x*2.0);
      lpPsiE_C[i] = CMPLX_DIV_R(lpS_C[i], tmp);
    }

    dlm_fftC(lpPsiE_C, n_samples, TRUE);

    if(lambda != 0.0) {
      for(i = n_samples/2; i > 0; i--) lpPsiE_R[i] = 2.0 * lpPsiE_C[i].x;
      lpPsiE_R[0] = lpPsiE_C[0].x;

      dlm_filter_freqt_fir(lpPsiE_R, n_samples/2, lpPsiE_R, n_samples, lambda);

      for(i = n_samples-1; i > 0; i--) lpPsiE_R[i] = 0.5 * lpPsiE_R[i];
    } else {
      for(i = n_samples-1; i >= 0; i--) {
        lpPsiE_R[i] = lpPsiE_C[i].x;
      }
    }

    for(i = 0; i < m; i++) {
      for(k = 0; k < m; k++) {
        lpH[i+k*m] = 2.0 * (lpPsiE_R[ABS((k-i))] + lpPsiE_R[k+i+2]);
      }
    }

    dlm_solve_lud(lpH,m,lpPsiE_R+1,1);
    dlm_transpose(lpT,lpH,m,m);
    dlm_mult(lpA,lpT,m,m,lpH,m,m);
    dlm_invert_gel(lpA,m,&det);
    dlm_mult(lpH,lpA,m,m,lpT,m,m);
    dlm_mult(lpD,lpH,m,m,lpPsiE_R+1,m,1);

    for(i = 0; i < m; i++) mc[i+1] = mc[i+1] + 2.0 * lpD[i];

    mc[0] = log(sqrt(lpPsiE_R[0])*scale);

    if(j > itr1) {
      if((ep-mc[0])/mc[0] < dd) {
        flag = TRUE;
      }
    }
  }

  dlp_free(lpS_C);
  dlp_free(lpD);
  dlp_free(lpA);
  dlp_free(lpH);
  dlp_free(lpT);
  dlp_free(lpPsiE_R);

  return (flag == TRUE) ? O_K : NOT_EXEC;
}

/************************************************************************
*
*   convolution: convolves 1 row of cepstrum coefs with the filter
*                memory s
*
*   FLOAT64*  s.        synthesis-filter memory si
*   FLOAT64*  cepCoef   vector of Cepstral coeficients
*
*   parameters:  m_nCoeff   number of cepstral coef from C1 to Cn
*/
FLOAT64 CGEN_IGNORE dlm_convolution(FLOAT64* s, FLOAT64* cepCoef, INT32 nCoeff) {
  INT32   i;
  FLOAT64 summ=0;

  for (i = nCoeff-1; i > 0; i--) {
      summ += cepCoef[i] * *s;
      s++;
    }

  return (summ);
}

/**
 * <p>Cepstral synthesis filter according to Robert Vích, Institute of Photonics and Electronics.</p>
 *
 * @param cep
 *          Pointer to the cepstrum
 * @param n_order
 *          Order of cepstrum equals to the length of <CODE>c</CODE>.
 * @param exc
 *          Input excitation signal
 * @param exc_len
 *          Length of <CODE>exc</CODE>, i.e. number of samples to synthesize.
 * @param pade_order
 *          Pade order of exponential approximation
 * @param syn
 *          Output synthesis samples
 * @param s
 *          Filter states (if *s is NULL, it will be allocated)
 * @return <code>O_K</code> if successfull, a (negative) error code otherwise
 */
INT16 CGEN_PUBLIC dlm_cep_synthesize(FLOAT64* cep, INT16 order, FLOAT64* exc, INT32 exc_len, INT16 pade_order, FLOAT64* syn, FLOAT64** s) {
  INT32 n;
  INT16 i;
  INT16 k;
  FLOAT64        gain;                                   /* Gain factor                      */
  FLOAT64*       ps = syn;                               /* Pointer to synthesis signal      */
  FLOAT64*       pe = exc;                               /* Pointer to excitation signal     */
  FLOAT64*       pis;                                    /* Pointer to inner filter states   */
  FLOAT64        pos[MAX_PADE_ORDER+1];                  /* Pointer to outer filter states   */
  FLOAT64*       pc;                                     /* Pointer to cesptral coefficients */
  FLOAT64        tmp;

#ifndef __TMS
  static FLOAT64 pade_order_old = 7;                     /* Pade order                       */
  static FLOAT64 order_old = 30;                         /* Order of cepstrum                */
#endif

#ifdef __TMS
    near
#endif
    static const FLOAT64 pade[][MAX_PADE_ORDER] = {
      { 0.5f, 0.0f,       0.0f,       0.0f,       0.0f,       0.0f,       0.0f       },
      { 0.5f, 1.0f/ 6.0f, 0.0f,       0.0f,       0.0f,       0.0f,       0.0f       },
      { 0.5f, 1.0f/ 5.0f, 1.0f/12.0f, 0.0f,       0.0f,       0.0f,       0.0f       },
      { 0.5f, 3.0f/14.0f, 1.0f/ 9.0f, 1.0f/20.0f, 0.0f,       0.0f,       0.0f       },
      { 0.5f, 2.0f/ 9.0f, 1.0f/ 8.0f, 1.0f/14.0f, 1.0f/30.0f, 0.0f,       0.0f       },
      { 0.5f, 5.0f/22.0f, 2.0f/15.0f, 1.0f/12.0f, 1.0f/20.0f, 1.0f/42.0f, 0.0f       },
      { 0.5f, 3.0f/13.0f, 5.0f/36.0f, 1.0f/11.0f, 3.0f/50.0f, 1.0f/27.0f, 1.0f/56.0f } };

#ifndef __TMS
  if(pade_order > MAX_PADE_ORDER) {
    return ERR_MDIM;
  }

  if(*s == NULL) {
    *s = (FLOAT64*)dlp_calloc(pade_order*(order-1), sizeof(FLOAT64));
    pade_order_old = pade_order;
    order_old = order;
  } else if((pade_order_old != pade_order) || (order_old != order)) {
    dlp_free(*s);
    *s = (FLOAT64*)dlp_calloc(pade_order*(order-1), sizeof(FLOAT64));
    pade_order_old = pade_order;
    order_old = order;
  }
#endif

  pc = cep;

  gain = exp(*pc);
  *pc = 0.0f;

  for (n = 0; n < exc_len; n++) {
    pis = *s;

    for(i = 1; i <= pade_order; i++) {
      tmp = *pc * *pis;
    for(k=order-2; k>0; k--) {
        tmp += pc[k] * (pis[k] = pis[k-1]);
      }
      pos[i] = tmp * pade[pade_order-1][i-1];
      pis += order-1;
    }

    *ps = 0.0;
    pos[0] = gain * *pe;

    for( i = 1; i <= pade_order; ++i){
        if( i & 1 ){ pos[0] += pos[i];
        }else{ pos[0] -= pos[i];
        }
        *ps += pos[i];
        if( i < pade_order ) *(*s+i*(order-1)) = pos[i];
    }
    *ps += pos[0];
    *(*s) = pos[0];

    pe++;
    ps++;
  }

  pc = NULL;

  return O_K;
}

/**
 * <p>Mel-Cepstral synthesis filter (extension to cepstral filter according to Robert Vích, Institute of Photonics and Electronics).</p>
 *
 * @param cep
 *          Pointer to the mel-cepstrum
 * @param n_order
 *          Order of cepstrum equals to the length of <CODE>c</CODE>.
 * @param exc
 *          Input excitation signal
 * @param exc_len
 *          Length of <CODE>exc</CODE>, i.e. number of samples to synthesize.
 * @param lambda
 *          Warping factor
 * @param pade_order
 *          Pade order of exponential approximation
 * @param syn
 *          Output synthesis samples
 * @param s
 *          Filter states (if *s is NULL, it will be allocated)
 * @return <code>O_K</code> if successfull, a (negative) error code otherwise
 */
INT16 CGEN_PUBLIC dlm_mcep_synthesize(FLOAT64* mcep, INT16 order, FLOAT64* exc, INT32 exc_len, FLOAT64 lambda, INT16 pade_order, FLOAT64* syn, FLOAT64** s) {
  INT32 n;
  INT16 i;
  INT16 k;
  FLOAT64  gain; /* Gain factor */
  FLOAT64  lambda2 = 1.0f - lambda*lambda;
  FLOAT64* ps = syn;                               /* Pointer to synthesis signal      */
  FLOAT64* pe = exc;                               /* Pointer to excitation signal     */
  FLOAT64* pis;                                    /* Pointer to inner filter states   */
  FLOAT64  pos[MAX_PADE_ORDER+1];                  /* Pointer to outer filter states   */
  FLOAT64* pc;                                     /* Pointer to cesptral coefficients */
  FLOAT64  tmp;

#ifndef __TMS
  static FLOAT64 pade_order_old = 7;                     /* Pade order                       */
  static FLOAT64 order_old = 30;                          /* Order of cepstrum                */
#endif

#  ifdef __TMS
    near
#  endif
    static const FLOAT64 pade[][MAX_PADE_ORDER] = {
      { 0.5, 0.0,      0.0,      0.0,      0.0,      0.0,      0.0      },
      { 0.5, 1.0/ 6.0, 0.0,      0.0,      0.0,      0.0,      0.0      },
      { 0.5, 1.0/ 5.0, 1.0/12.0, 0.0,      0.0,      0.0,      0.0      },
      { 0.5, 3.0/14.0, 1.0/ 9.0, 1.0/20.0, 0.0,      0.0,      0.0      },
      { 0.5, 2.0/ 9.0, 1.0/ 8.0, 1.0/14.0, 1.0/30.0, 0.0,      0.0      },
      { 0.5, 5.0/22.0, 2.0/15.0, 1.0/12.0, 1.0/20.0, 1.0/42.0, 0.0      },
      { 0.5, 3.0/13.0, 5.0/36.0, 1.0/11.0, 3.0/50.0, 1.0/27.0, 1.0/56.0 } };

#ifndef __TMS
  if(pade_order > MAX_PADE_ORDER) {
    return ERR_MDIM;
  }

  if(*s == NULL) {
    *s = (FLOAT64*)dlp_calloc(pade_order*(order-1), sizeof(FLOAT64));
    pade_order_old = pade_order;
    order_old = order;
  } else if((pade_order_old != pade_order) || (order_old != order)) {
    dlp_free(*s);
    *s = (FLOAT64*)dlp_calloc(pade_order*(order-1), sizeof(FLOAT64));
    pade_order_old = pade_order;
    order_old = order;
  }
#endif

  pc = mcep;

  for (i = order - 2; i >= 0; i--) pc[i] -= lambda * pc[i + 1];

  gain = exp(*pc);
  *pc = 0.0;

  for (n = 0; n < exc_len; n++) {
    pis = *s;

    for(i = 1; i <= pade_order; i++) {
      if(lambda != 0.0) {
        pis[0] += lambda*pis[1];
        for(k=1; k<(order-2); k++) pis[k] += lambda * (pis[k+1]-pis[k-1]);
      }
      tmp = *pc * *pis;
      for(k=order-2; k>0; k--) {
        tmp += pc[k] * (pis[k] = pis[k-1]);
      }
      pos[i] = tmp * lambda2 * pade[pade_order-1][i-1];
      pis += order-1;
    }

    *ps = 0.0;
    pos[0] = gain * *pe;

    for( i = 1; i <= pade_order; ++i){
        if( i & 1 ){ pos[0] += pos[i];
        }else{ pos[0] -= pos[i];
        }
        *ps += pos[i];
        if( i < pade_order ) *(*s+i*(order-1)) = pos[i];
    }
    *ps += pos[0];
    *(*s) = pos[0];

    pe++;
    ps++;
  }

  pc = NULL;

  return O_K;
}

/**
 * <p>Mel-Cepstral synthesis filter (Mel-Log-Spectrum Approximation Filter, MLSADF).</p>
 *
 * @param mcep
 *          Pointer to the mel-cepstrum
 * @param n_order
 *          Order of cepstrum equals to the length of <CODE>c</CODE>.
 * @param exc
 *          Input excitation signal
 * @param exc_len
 *          Length of <CODE>exc</CODE>, i.e. number of samples to synthesize.
 * @param lambda
 *          Warping factor
 * @param pade_order
 *          Pade order of exponential approximation
 * @param syn
 *          Output synthesis samples
 * @param s
 *          Filter states (if *s is NULL, it will be allocated)
 * @return <code>O_K</code> if successfull, a (negative) error code otherwise
 */
INT16 CGEN_PUBLIC dlm_mcep_synthesize_mlsadf(FLOAT64* mcep, INT16 order, FLOAT64* exc, INT32 exc_len, FLOAT64 lambda, INT16 pade_order, FLOAT64* syn, FLOAT64** s) {
  INT32           i              = 0;
  INT32           i_mcep         = 0;
  static INT32    pade_order_old = 0;                          /* Pade order                  */
  static INT32    order_old      = 0;                          /* Order of cepstrum           */
  FLOAT64*        b              = NULL;
  FLOAT64*        b_old          = NULL;
  FLOAT64*        b_cur          = NULL;
  extern FLOAT64 mlsadf(FLOAT64,FLOAT64*,const INT32,const FLOAT64,const INT32,FLOAT64*);

  if(pade_order > 5) {
    return ERR_MDIM;
  }

  b     = (FLOAT64*)dlp_malloc(order * sizeof(FLOAT64));
  b_cur = (FLOAT64*)dlp_malloc(order * sizeof(FLOAT64));
  if(!b_cur || !b) return ERR_MEM;

  dlm_mcep2b(mcep, b, order, lambda);

  if(*s == NULL) {
    *s = (FLOAT64*)dlp_calloc(3*(pade_order+1)+pade_order*(order+1)+order, sizeof(FLOAT64));
    pade_order_old = pade_order;
    order_old = order;
    b_old = *s+3*(pade_order+1)+pade_order*(order+1);
    dlp_memmove(b_old, b, order * sizeof(FLOAT64));
  } else if((pade_order_old != pade_order) || (order_old != order)) {
    dlp_free(*s);
    *s = (FLOAT64*)dlp_calloc(3*(pade_order+1)+pade_order*(order+1)+order, sizeof(FLOAT64));
    pade_order_old = pade_order;
    order_old = order;
    b_old = *s+3*(pade_order+1)+pade_order*(order+1);
  } else {
    b_old = *s+3*(pade_order+1)+pade_order*(order+1);
  }

  for(i = 0; i < exc_len; i++) {
    for(i_mcep = 0; i_mcep < order; i_mcep++) {
      b_cur[i_mcep] = b_old[i_mcep] + (b[i_mcep] - b_old[i_mcep])*(FLOAT64)(i)/(FLOAT64)(exc_len);
    }


    *syn = exp(*b_cur) * *exc;
    if(order > 1) *syn = mlsadf(*syn, b_cur, order-1, lambda, pade_order, *s);

    exc++;
    syn++;
  }

  dlp_memmove(b_old, b, order*sizeof(FLOAT64));

  dlp_free(b);
  dlp_free(b_cur);

  return O_K;
}

/**
 * <p>Scaling of cepstrum to prepare for quantization. This function calculate
 * <code>c[i] = c[i]*(i+1)</code> according to Oppenheimer and scales up to
 * <code>32768</code>.</p>
 *
 * @param cep
 *          Pointer to the cepstrum coefficients.
 * @param n
 *          Number of cepstrum coefficients.
 * @param q
 *         bit width of target
 * @return <code>O_K</code> if successfull, <code>ERR_TRUNCATE</code> if any cepstrum
 * coeffizent exceeds ln(32768).
 */
INT16 CGEN_PUBLIC dlm_cep_quantize(FLOAT64* cep, INT16 n, INT16 q) {
  INT16 i;
  FLOAT64 s = 2.0 * (dlm_pow2(q-1) / 20.79441541681); /* avoid overrun! */

  for(i=0; i<n; i++) {
    if(cep[i] > 20.79441541681) return ERR_TRUNCATE;  /* 2*LN(32768) */
    cep[i] = round(s * cep[i] * (FLOAT64)(i+1) / 2.0);
  }
  return O_K;
}

/**
 * <p> This function is the inverse of <code>dlm_cep_quantize</code>.</p>
 */
INT16 CGEN_PUBLIC dlm_cep_dequantize(FLOAT64* cep, INT16 n, INT16 q) {
  INT16 i;
  FLOAT64 s = 2.0 * (dlm_pow2(q-1) / 20.79441541681); /* avoid overrun! */

  for(i=0; i<n; i++) {
    cep[i] = cep[i] * 2.0/(s*(FLOAT64)(i+1));
  }
  return O_K;
}

INT16 CGEN_PUBLIC dlm_mcep_enhance(FLOAT64* mc, FLOAT64* mc_new, INT16 n, FLOAT64 lambda) {
  INT16    i       = 0;
  INT16    a_len   = 96;
  FLOAT64  e1      = 0.0;
  FLOAT64  e2      = 0.0;
  FLOAT64  beta    = 0.25;
  FLOAT64  gain    = mc[0];
  FLOAT64* mc_tmp  = (FLOAT64*)dlp_calloc(n, sizeof(FLOAT64));
  FLOAT64* a       = (FLOAT64*)dlp_calloc(a_len, sizeof(FLOAT64));

  if(!mc_tmp || !a) return ERR_MEM;

  gain = mc[0];

  mc_tmp[0] = 0.0;
  for(i = 1; i < n; i++) {
    mc_tmp[i] = -mc[i];
  }
  dlm_mcep2lpc(mc_tmp, n, a, a_len, lambda);
  for(i = 0; i < a_len; i++) {
    e1 += a[i]*a[i];
  }

  mc_new[0] = 0.0;
  for(i = 1; i < n; i++) {
    mc_new[i] = mc[i] * (1.0 + beta);
  }

  for(i = 0; i < n; i++) {
    mc_tmp[i] = -mc_new[i];
  }
  dlm_mcep2lpc(mc_tmp, n, a, a_len, lambda);
  for(i = 0; i < a_len; i++) {
    e2 += a[i]*a[i];
  }

  mc_new[0] = gain + log(e1/e2) / 2.0;

  dlp_free(mc_tmp);
  dlp_free(a);

  return O_K;
}
