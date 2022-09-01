/* dLabPro mathematics library
 * - MEL-Filter analysis
 *
 * AUTHOR : Frank Duckhorn
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
#include <stdlib.h>
#include <math.h>
#include "dlp_kernel.h"
#include "dlp_base.h"
#include "dlp_math.h"

#define MLP_ABS(A) ((A)>0?(A):-(A))

/*
 --- convcore: convolution parameter display ---------------------------

 k      mid f_mid(Hz)    Bark   b[Hz]     width   f_width(Hz)  norm

 0        6   187.50     1.84   102.53       24   750.00      5.70661
 1        9   281.25     2.74   105.64       24   750.00      5.70661
 2       12   375.00     3.62   109.90       24   750.00      5.70661
 3       15   468.75     4.46   115.25       24   750.00      5.70661
 4       18   562.50     5.27   121.59       24   750.00      5.70661
 5       21   656.25     6.04   128.86       24   750.00      5.70661
 6       24   750.00     6.77   136.97       24   750.00      5.70661
 7       27   843.75     7.46   145.86       24   750.00      5.70661
 8       30   937.50     8.10   155.45       24   750.00      5.70661
 9       33  1031.25     8.71   165.70       24   750.00      5.70661
 10       37  1156.25     9.46   180.30       32  1000.00      7.60474
 11       41  1281.25    10.14   195.87       32  1000.00      7.60474
 12       45  1406.25    10.77   212.34       32  1000.00      7.60474
 13       49  1531.25    11.34   229.62       32  1000.00      7.60474
 14       53  1656.25    11.86   247.65       32  1000.00      7.60474
 15       57  1781.25    12.35   266.40       32  1000.00      7.60474
 16       61  1906.25    12.79   285.80       32  1000.00      7.60474
 17       65  2031.25    13.20   305.83       32  1000.00      7.60474
 18       69  2156.25    13.59   326.45       32  1000.00      7.60474
 19       73  2281.25    13.94   347.63       32  1000.00      7.60474
 20       77  2406.25    14.27   369.35       32  1000.00      7.60474
 21       83  2593.75    14.73   402.88       48  1500.00     11.40275
 22       91  2843.75    15.29   449.25       64  2000.00     15.20164
 23      101  3156.25    15.90   509.73       80  2500.00     19.00087
 24      113  3531.25    16.55   585.69       96  3000.00     22.80029
 25      127  3968.75    17.21   678.61      112  3500.00     26.59978
 26      143  4468.75    17.89   790.03      128  4000.00     30.39935
 27      161  5031.25    18.58   921.48      144  4500.00     34.19896
 28      181  5656.25    19.26  1074.53      160  5000.00     37.99860
 29      203  6343.75    19.94  1250.74      176  5500.00     41.79828

 ------------------------------------------------------------------------
 */

INT16 CGEN_IGNORE dlm_mf_analyze(MLP_CNVC_TYPE* lpCnvc, FLOAT64* frame, FLOAT64* result, INT32 nLen, INT32 nCoeff, FLOAT64 nMinLog) {
  INT32    i      = 0;
  INT16    ret    = O_K;
  FLOAT64* real_f = NULL;
  FLOAT64* imag_f = NULL;

  real_f = (FLOAT64*)dlp_calloc(MAX(lpCnvc->n_out,nLen), sizeof(FLOAT64));
  imag_f = (FLOAT64*)dlp_calloc(MAX(lpCnvc->n_out,nLen), sizeof(FLOAT64));

  for (i = 0; i < nLen; i++) {
    real_f[i] = frame[i];
    imag_f[i] = 0.0;
  }

  /* Compute log magnitude spectrum */
  if((ret = dlm_fft(real_f, imag_f, nLen, FALSE)) != O_K) {
    dlp_free(real_f);
    dlp_free(imag_f);
    return ret;
  }

  if(dlp_charin('S', lpCnvc->type)) {
    real_f[0] = 0.5f*log(real_f[0]*real_f[0] + imag_f[0]*imag_f[0]);
    real_f[0] = MAX(nMinLog, real_f[0]);
    for (i = 1; i <= nLen/2; i++) {
      real_f[i] = 0.5f*log(real_f[i]*real_f[i] + imag_f[i]*imag_f[i]);
      real_f[i] = MAX(nMinLog, real_f[i]);
      real_f[nLen-i] = real_f[i];
    }
  } else {
    real_f[0] = sqrt(real_f[0]*real_f[0] + imag_f[0]*imag_f[0]);
    for (i = 1; i <= nLen/2; i++) {
      real_f[i] = sqrt(real_f[i]*real_f[i] + imag_f[i]*imag_f[i]);
      real_f[nLen-i] = real_f[i];
    }
  }
  if(dlp_charin('N',lpCnvc->type)) dlm_noiserdc(real_f,nLen,FALSE);
  dlm_mf_convolve(lpCnvc, real_f, imag_f);

  if(dlp_charin('T', lpCnvc->type)) {
    for(i = 0; i < nCoeff; i++) {
      result[i] = MAX(nMinLog,log(imag_f[i])) - lpCnvc->quant_energ;
    }
  } else {
    for(i = 0; i < nCoeff; i++) {
      result[i] = imag_f[i] - lpCnvc->quant_energ;
    }
  }
  
  dlp_free(real_f);
  dlp_free(imag_f);

  return O_K;
}

#define DLM_MF_WTYPE_MEL         1
#define DLM_MF_WTYPE_BILINEAR    2
#define DLM_MF_FTYPE_SINC        1
#define DLM_MF_FTYPE_TRIANGULAR  2

/**
 * Initialize convolution core data struct.
 * 
 * @param lpCnvc
 *          Pointer to convolution core data struct
 * @param nIn
 *          Number of input channels                        
 * @param nOut
 *          Number of output channels 
 * @param quant_energ
 *          FFT quantization error
 * @return <code>O_K</code> if successfull, a (negative) error code otherwise
 */
INT16 CGEN_IGNORE dlm_mf_init(MLP_CNVC_TYPE* lpCnvc, INT16 nIn, INT16 nOut, FLOAT64 quant_energ) {
  INT16 j;
  INT16 k;
  INT16 l;
  INT16 nErr= O_K;
  INT16 wtype = DLM_MF_WTYPE_MEL;
  INT16 ftype = DLM_MF_FTYPE_SINC;
  
  DLPASSERT(lpCnvc);
  if (!lpCnvc)
    return NOT_EXEC;

  /* Free present data */
  dlm_mf_done(lpCnvc);


  /* Basic initialization and memory allocation */
  lpCnvc->n_in        = dlm_log2_i(nIn)>=0 ? nIn : 1 << ((INT16)dlm_log_bC(CMPLX(2),CMPLX(nIn)).x + 1);
  lpCnvc->n_out       = nOut;
  lpCnvc->quant_energ = quant_energ;
  lpCnvc->mid         = (FLOAT64*) dlp_calloc(nOut,sizeof(FLOAT64));
  lpCnvc->width[0]    = (FLOAT64*) dlp_calloc(nOut,sizeof(FLOAT64));
  lpCnvc->width[1]    = (FLOAT64*) dlp_calloc(nOut,sizeof(FLOAT64));
  lpCnvc->norm        = (FLOAT64*) dlp_calloc(nOut,sizeof(FLOAT64));
  lpCnvc->z           = (FLOAT64*) dlp_calloc(nOut,sizeof(FLOAT64));
  lpCnvc->a           = (FLOAT64**)dlp_calloc(nOut,sizeof(FLOAT64*));

  for (k=0; k<nOut; k++) {
    lpCnvc->a[k] = (FLOAT64*)dlp_calloc(2*nIn,sizeof(FLOAT64));
    if (!lpCnvc->a[k]) return ERR_MEM;
  }

  /* Check memory */
  if (!lpCnvc->mid || !lpCnvc->width[0] || !lpCnvc->width[1] || !lpCnvc->norm|| !lpCnvc->a||!lpCnvc->z
      || nErr==ERR_MEM) {
    dlm_mf_done(lpCnvc);
    DLPASSERT(0);
  }

  if(dlp_charin('M', lpCnvc->type)) {
    wtype = DLM_MF_WTYPE_MEL;
  } else if(dlp_charin('B', lpCnvc->type)) {
    wtype = DLM_MF_WTYPE_BILINEAR;
  } else { 
    dlm_mf_done(lpCnvc);
    return NOT_EXEC;
  }
  if(dlp_charin('S', lpCnvc->type)) {
    ftype = DLM_MF_FTYPE_SINC;
  } else if(dlp_charin('T', lpCnvc->type)) {
    ftype = DLM_MF_FTYPE_TRIANGULAR;
  } else { 
    dlm_mf_done(lpCnvc);
    return NOT_EXEC;
  }
  
  /* Generate convolution core */
  switch(wtype) {
  case DLM_MF_WTYPE_MEL:
    lpCnvc->mid[0] = 6;
    for (k=1; k<lpCnvc->n_out; k++) {
      if (k<=9) {
        lpCnvc->mid[k] = lpCnvc->mid[k-1] + 3;
      }
      if ((k>9) && (k<=20)) {
        lpCnvc->mid[k] = lpCnvc->mid[k-1] + 4;
      }
      if (k>20) {
        lpCnvc->mid[k] = lpCnvc->mid[k-1] + (lpCnvc->mid[k-1] - lpCnvc->mid[k-2] + 2);
      }
    }
    
    break;
  case DLM_MF_WTYPE_BILINEAR:
    for (k = 0; k < lpCnvc->n_out; k++) {
      FLOAT64 tmp = (FLOAT64)((k+1)*lpCnvc->n_in/2)/(FLOAT64)(lpCnvc->n_out+1);
      lpCnvc->mid[k] = tmp + (FLOAT64)(lpCnvc->n_in)*atan2(-lpCnvc->lambda*sin(2.0*F_PI*tmp/(FLOAT64)lpCnvc->n_in), 1+lpCnvc->lambda*cos(2.0*F_PI*tmp/(FLOAT64)lpCnvc->n_in))/F_PI;
    }
    break;
  default:
    dlm_mf_done(lpCnvc);
    return NOT_EXEC;
  }

  lpCnvc->width[0][0] = lpCnvc->mid[1] - lpCnvc->mid[0];
  lpCnvc->width[1][0] = lpCnvc->mid[1] - lpCnvc->mid[0];
  for (k=1; k<lpCnvc->n_out-1; k++) {
    lpCnvc->width[0][k] = lpCnvc->mid[k]-lpCnvc->mid[k-1];
    lpCnvc->width[1][k] = lpCnvc->mid[k+1]-lpCnvc->mid[k];
  }
  lpCnvc->width[0][k] = lpCnvc->mid[k]-lpCnvc->mid[k-1];
  lpCnvc->width[1][k] = lpCnvc->mid[k]-lpCnvc->mid[k-1];
  
  switch(ftype) {
  case DLM_MF_FTYPE_SINC:
    for (k=0; k<lpCnvc->n_out; k++) {
      FLOAT64 width = lpCnvc->width[0][k] + lpCnvc->width[1][k];
      lpCnvc->norm[k] = 0.0;
      for (j=-lpCnvc->n_in/2; j < lpCnvc->n_in/2; j++) {
        l = (j + lpCnvc->n_in) % lpCnvc->n_in;
        if(fabs((FLOAT64)j-lpCnvc->mid[k]) < F_TINY) {
          lpCnvc->a[k][l] = 1.0;
        } else {
          lpCnvc->a[k][l] = width * (FLOAT64)(sin((FLOAT64)(((FLOAT64)j-lpCnvc->mid[k])*F_PI)/width) / (((FLOAT64)j-lpCnvc->mid[k])*F_PI));
        }
        lpCnvc->norm[k] += lpCnvc->a[k][l];
      }
    }
    break;
  case DLM_MF_FTYPE_TRIANGULAR:
    for (k = 0; k < lpCnvc->n_out; k++) {
      FLOAT64 width_l = 2.0 * lpCnvc->width[0][k];
      FLOAT64 width_r = 2.0 * lpCnvc->width[1][k];
      INT32 l1 = (INT32)ceil(lpCnvc->mid[k]-width_l);
      INT32 l2 = (INT32)ceil(lpCnvc->mid[k]+width_r);
      if((l2 - l1) < 1) {
        l = (INT32)(lpCnvc->mid[k] + 0.5);
        lpCnvc->norm[k] = 1.0;
        lpCnvc->a[k][l] = 1.0;
      } else {
        lpCnvc->norm[k] = 0.0;
        for (j = l1; j < l2; j++) {
          l = (j+lpCnvc->n_in) % (lpCnvc->n_in);
          if(j < lpCnvc->mid[k]) {
            lpCnvc->a[k][l] = ((FLOAT64)j - (lpCnvc->mid[k]-width_l)) / width_l;
          } else {
            lpCnvc->a[k][l] = ((lpCnvc->mid[k]+width_r) - (FLOAT64)j) / width_r;
          }
          lpCnvc->norm[k] += lpCnvc->a[k][l];
        }
      }
    }
    break;
  default:
    dlm_mf_done(lpCnvc);
    return NOT_EXEC;
  }
  
  lpCnvc->n_in        = nIn;
  return O_K;
}

/**
 * Deallocate core data struct.
 * 
 * @param lpCnvc
 *          Pointer to convolution core data struct
 */
INT16 CGEN_IGNORE dlm_mf_done(MLP_CNVC_TYPE* lpCnvc) {
  INT16 k;

  DLPASSERT(lpCnvc);
  if (!lpCnvc)
    return NOT_EXEC;

  if (lpCnvc->mid)      dlp_free(lpCnvc->mid );
  if (lpCnvc->width[0]) dlp_free(lpCnvc->width[0]);
  if (lpCnvc->width[1]) dlp_free(lpCnvc->width[1]);
  if (lpCnvc->norm)     dlp_free(lpCnvc->norm );
  if (lpCnvc->z)        dlp_free(lpCnvc->z );
  if (lpCnvc->a) {
    for (k=0; k<lpCnvc->n_out; k++)dlp_free(lpCnvc->a[k]);
    dlp_free(lpCnvc->a);
  }
  return O_K;
}

/**
 * Do convolution
 * (see Release Note 32/64-Bit  difference in feature extraction)
 * 
 * @param lpCnvc
 *          Pointer to convolution core data struct
 * @param lpIn
 *          Pointer to input data buffer
 * @param lpOut
 *          Pointer to input data buffer
 */
INT16 CGEN_IGNORE dlm_mf_convolve(MLP_CNVC_TYPE* lpCnvc, FLOAT64* lpIn, FLOAT64* lpOut) {
  register FLOAT64* lpA;
  register FLOAT64* lpI;
  register FLOAT64  z;
  register INT16    j;
  register INT16    k;

  DLPASSERT(lpCnvc);
  if (!lpCnvc) {
    for (k=0; k<lpCnvc->n_out; k++)
      lpOut[k]=0.;
    return NOT_EXEC;
  }

  for (k=0; k < lpCnvc->n_out; k++) {
    z = 0.0;
    lpA = lpCnvc->a[k];
    lpI = lpIn;
    j = lpCnvc->n_in;
    while(j--) {
      z += *lpA++ * *lpI++;
    }
    lpOut[k] = z / lpCnvc->norm[k];
  }
  
  return O_K;
}

