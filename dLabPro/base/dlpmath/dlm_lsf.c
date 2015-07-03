/* dLabPro mathematics library
 * - Line Spectral Frequencies
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

int CGEN_IGNORE qsort_compare_lsf(const void* lsf1, const void* lsf2) {
  return((*((FLOAT64*)lsf1) < *((FLOAT64*)lsf2)) ? -1 : 1);
}

/**
 * <p>Conversion of prediction polynomial to line spectral frequencies (LSF).</p>
 *
 * @param poly
 *          Pointer to polynomial coefficients to convert.
 * @param lsf
 *          Pointer to resulting LSF coefficients <CODE>a</CODE>(k) (k = 0 ... order - 1).
 * @param order
 *          Order of polynomial, equals to number of LSF coefficients.
 * @return <CODE>-1</CODE>, if the polynomial is unstable and no stabilising is possible, otherwise: number of roots outside unit circle, which were mirrored into the unit circle.
 */
INT16 dlm_poly2lsf(FLOAT64* poly, FLOAT64* lsf, INT16 order) {
  COMPLEX64* roots1 = NULL;
  COMPLEX64* roots2 = NULL;
  FLOAT64* a1 = NULL;
  FLOAT64* a2 = NULL;
  FLOAT64* q1 = NULL;
  FLOAT64* p1 = NULL;
  FLOAT64* q = NULL;
  FLOAT64* p = NULL;
  INT16  i_order;
  INT16  n_roots_stabilised = 0;

  i_order = 0;

  if((!poly) || (!lsf)) return NOT_EXEC;

  roots1 = (COMPLEX64*)dlp_calloc(2*order, sizeof(COMPLEX64));
  roots2 = roots1 + order;
  a1          = (FLOAT64*)dlp_calloc(6*(order+1),sizeof(FLOAT64));
  a2          = a1          + order + 1;
  q1          = a2          + order + 1;
  p1          = q1          + order + 1;
  p           = p1          + order + 1;
  q           = p           + order + 1;

  while((i_order < order) && (*(poly + i_order) == 0.0)) {
    i_order++;
    order--;
    poly++;
  }

  if(order < 1) {
    dlp_free(roots1);
    return NOT_EXEC;
  }

  dlm_roots(poly, roots1, order);
  for(i_order = 0; i_order < order; i_order++) {
    if(roots1[i_order].x * roots1[i_order].x + roots1[i_order].y * roots1[i_order].y >= 1.0) {
      dlp_free(roots1);
      dlp_free(a1);
      return -1;
    }
  }

  lsf++;

  memcpy(a1, poly, order * sizeof(FLOAT64));
  for(i_order = 0; i_order <= order; i_order++) {
    a2[i_order] = a1[order - i_order];
    p1[i_order] = a1[i_order] - a2[i_order];
    q1[i_order] = a1[i_order] + a2[i_order];
  }
  if((order % 2) == 0) {
    p[0] = p1[0];
    p[1] = p1[1];
    for(i_order = 2; i_order < order; i_order++) {
      p[i_order] = p[i_order - 2] + p1[i_order];
    }
    memcpy(q, q1, (order + 1) * sizeof(FLOAT64));
    IF_NOK(dlm_roots(q, roots2, order + 1)) {
      dlp_free(roots1);
      dlp_free(a1);
      return NOT_EXEC;
    }
    for(i_order = 0; i_order < order; i_order += 2) {
      lsf[i_order] = ABS(atan2(roots2[i_order].y, roots2[i_order].x));
    }
    order -= 2;
    IF_NOK(dlm_roots(p, roots1, order + 1)) {
      dlp_free(roots1);
      dlp_free(a1);
      return NOT_EXEC;
    }
    for(i_order = 0; i_order < order; i_order += 2) {
      lsf[i_order + 1] = ABS(atan2(roots1[i_order].y, roots1[i_order].x));
    }
    order++;
  } else {
    p[0] = p1[0];
    q[0] = q1[0];
    for(i_order = 1; i_order < order; i_order++) {
      p[i_order] =  p[i_order - 1] + p1[i_order];
      q[i_order] = -q[i_order - 1] + q1[i_order];
    }
    order--;
    IF_NOK(dlm_roots(p, roots1, order + 1)) {
      dlp_free(roots1);
      dlp_free(a1);
      return NOT_EXEC;
    }
    IF_NOK(dlm_roots(q, roots2, order + 1)) {
      dlp_free(roots1);
      dlp_free(a1);
      return NOT_EXEC;
    }
    for(i_order = 0; i_order < order; i_order += 2) {
      lsf[i_order] = ABS(atan2(roots1[i_order].y, roots1[i_order].x));
      lsf[i_order + 1] = ABS(atan2(roots2[i_order].y, roots2[i_order].x));
    }
  }

  qsort(lsf, order, sizeof(FLOAT64), qsort_compare_lsf);

  lsf--;

  dlp_free(roots1);
  dlp_free(a1);

  return n_roots_stabilised;
}

/**
 * <p>Conversion of line spectral frequencies (LSF) to prediction polynomial.</p>
 *
 * @param lsf
 *          Pointer to resulting LSF coefficients <CODE>a</CODE>(k) (k = 0 ... order - 1).
 * @param poly
 *          Pointer to polynomial coefficients to convert.
 * @param order
 *          Order of polynomial, equals to number of LSF coefficients.
 * @return <code>O_K</code> if successfull, a (negative) error code otherwise
 */
INT16 dlm_lsf2poly(FLOAT64* lsf, FLOAT64* poly, INT16 order) {
  INT16    orderQ;
  INT16    orderP;
  INT16    i_order1;
  INT16    i_order2;
  FLOAT64* rQ_r;
  FLOAT64* rQ_i;
  FLOAT64* rP_r;
  FLOAT64* rP_i;
  FLOAT64* z_r;
  FLOAT64* z_i;
  FLOAT64* Q_r;
  FLOAT64* Q_i;
  FLOAT64* P_r;
  FLOAT64* P_i;
  FLOAT64* P1;
  FLOAT64* Q1;

  FLOAT64 a0;

  if((!poly) || (!lsf)) return NOT_EXEC;
  if(order < 1) return NOT_EXEC;

  a0 = *lsf;

  lsf++;
  order--;

  orderQ = (order + 1) / 2 * 2;
  orderP = order / 2 * 2;

  rQ_r = (FLOAT64*)dlp_calloc(2*orderQ + 2*orderP + 4*(orderQ+1) + 2*(orderP+1) + 2*order, sizeof(FLOAT64));
  rQ_i = rQ_r + orderQ;
  rP_r = rQ_i + orderQ;
  rP_i = rP_r + orderP;
  Q_r  = rP_i + orderP;
  Q_i  = Q_r  + orderQ + 1;
  P_r  = Q_i  + orderQ + 1;
  P_i  = P_r  + orderP + 1;
  z_r  = P_i  + orderP + 1;
  z_i  = z_r  + order;
  P1   = z_i  + order;
  Q1   = P1   + orderQ + 1;

  for(i_order1 = 0; i_order1 < order; i_order1++) {
    *(z_r + i_order1) = cos((FLOAT64)(*(lsf + i_order1)));
    *(z_i + i_order1) = sin((FLOAT64)(*(lsf + i_order1)));
  }
  for(i_order1 = 0; i_order1 < (orderQ / 2); i_order1++) {
    *(rQ_r + i_order1) = *(rQ_r + orderQ / 2 + i_order1) = *(z_r + 2 * i_order1);
    *(rQ_i + i_order1) = *(z_i + 2 * i_order1);
    *(rQ_i + orderQ / 2 + i_order1) = -*(z_i + 2 * i_order1);
  }
  for(i_order1 = 0; i_order1 < (orderP / 2); i_order1++) {
    *(rP_r + i_order1) = *(rP_r + orderP / 2 + i_order1) = *(z_r + 2 * i_order1 + 1);
    *(rP_i + i_order1) = *(z_i + 2 * i_order1 + 1);
    *(rP_i + orderP / 2 + i_order1) = -*(z_i + 2 * i_order1 + 1);
  }

  dlp_memset(Q_r + 0, 0L, (orderQ + 1) * sizeof(FLOAT64));
  dlp_memset(Q_i + 0, 0L, (orderQ + 1) * sizeof(FLOAT64));
  *(Q_r + 0) = 1.0;

  for(i_order1 = 0; i_order1 < orderQ; i_order1++) {
    for(i_order2 = i_order1; i_order2 >= 0; i_order2--) {
      *(Q_r + i_order2 + 1) -= *(rQ_r + i_order1) * *(Q_r + i_order2) - *(rQ_i + i_order1) * *(Q_i + i_order2);
      *(Q_i + i_order2 + 1) -= *(rQ_r + i_order1) * *(Q_i + i_order2) + *(rQ_i + i_order1) * *(Q_r + i_order2);
    }
  }
  dlp_memset(P_r + 1, 0L, (orderP + 1) * sizeof(FLOAT64));
  dlp_memset(P_i + 0, 0L, (orderP + 1) * sizeof(FLOAT64));
  *(P_r + 0) = 1.0;

  for(i_order1 = 0; i_order1 < orderP; i_order1++) {
    for(i_order2 = i_order1; i_order2 >= 0; i_order2--) {
      *(P_r + i_order2 + 1) -= *(rP_r + i_order1) * *(P_r + i_order2) - *(rP_i + i_order1) * *(P_i + i_order2);
      *(P_i + i_order2 + 1) -= *(rP_r + i_order1) * *(P_i + i_order2) + *(rP_i + i_order1) * *(P_r + i_order2);
    }
  }

  if(order % 2 == 1) {
    *(P1 + 0) = *(P_r + 0);
    *(P1 + 1) = *(P_r + 1);
    for(i_order1 = 2; i_order1 < orderQ + 1; i_order1++) {
      *(P1 + i_order1) = *(P_r + i_order1) - *(P_r + i_order1 - 2);
    }
    memcpy(Q1, Q_r, (orderQ + 1) * sizeof(FLOAT64));
  } else {
    *(P1 + 0) = *(P_r + 0);
    *(Q1 + 0) = *(Q_r + 0);
    for(i_order1 = 1; i_order1 < orderQ + 1; i_order1++) {
      *(P1 + i_order1) = *(P_r + i_order1) - *(P_r + i_order1 - 1);
      *(Q1 + i_order1) = *(Q_r + i_order1) + *(Q_r + i_order1 - 1);
    }
  }

  for(i_order1 = 1; i_order1 < order + 1; i_order1++) {
    *(poly + i_order1) = (*(P1 + i_order1) + *(Q1 + i_order1)) / 2.0;
  }

  *poly = a0;

  dlp_free(rQ_r);
  return O_K;
}

void CGEN_IGNORE dlm_lsf2lsp(FLOAT64* lsf, FLOAT64* lsp, INT16 n) {
  INT16 i;

  for(i = 0; i < n; i++) {
    if(i & 2) {
      if(i & 1) {
        lsp[i] = cos(lsf[n-i/2-(n&1)]);
      } else {
        lsp[i] = cos(lsf[n-i/2-1+(n&1)]);
      }
    } else {
      if(i & 1) {
        lsp[i] = cos(lsf[i/2+1]);
      } else {
        lsp[i] = cos(lsf[i/2]);
      }
    }
  }
}

void CGEN_IGNORE dlm_lsf_interpolate(FLOAT64* lsf1, FLOAT64* lsf2, FLOAT64 *lsf3, INT16 n_lsf, INT32 num, INT32 den) {
  INT16 i_lsf;
  for(i_lsf=0; i_lsf<n_lsf; i_lsf++) {
    lsf2[i_lsf] = ((FLOAT64)num*lsf3[i_lsf] + (FLOAT64)(den-num)*lsf1[i_lsf])/(FLOAT64)den;
  }
}

/**
 * <p>Conversion of mel-line spectral frequencies (MLSF) to prediction polynomial
 * using Mel-FIR filter structure.</p>
 *
 * @param mlsf
 *          Pointer to MLSF coefficients <CODE>mlsf</CODE>(k) (k = 0 ... n_mlsf - 1).
 * @param n_mlsf
 *          Order of MLSF coefficients.
 * @param poly
 *          Pointer to resulting polynomial coefficients.
 * @param n_poly
 *          Order of resulting polynomial. Can be different than n_mlsf.
 * @param lambda
 *          Warping factor.
 * @param mem
 *          Pointer to working memory. If <code>*mem</code> is <code>NULL</code>
 *          memory will be allocated but not freed afterwards.
 * @return <code>O_K</code> if successful, a (negative) error code otherwise
 */
INT16 dlm_mlsf2poly_filt(FLOAT64* mlsf, INT16 n_mlsf, FLOAT64* poly, INT16 n_poly, FLOAT64 lambda, FLOAT64** mem) {
  INT16      i_mlsf  = 0;
  INT16      i_poly  = 0;
  INT16      n_mlsp = n_mlsf+(n_mlsf&1);
  FLOAT64    lambda2 = 1.0-lambda*lambda;
  FLOAT64    tmp    = 0.0;
  FLOAT64    out[2]  = { 0.0, 0.0 };
  FLOAT64*   mlsp[2]    = { NULL, NULL };
  FLOAT64    e[2]    = { 1.0, 1.0 };
  FLOAT64*   z[3]    = { NULL, NULL, NULL };

  if(!mem || !poly || !mlsf) return ERR_MDIM;

  if(*mem == NULL) {
    *mem = (FLOAT64*)dlp_calloc(3*n_mlsp,sizeof(FLOAT64));
    if(!*mem) return ERR_MEM;
  }

  mlsp[0]  = (FLOAT64*)dlp_calloc(2*n_mlsp,sizeof(FLOAT64));
  mlsp[1] = mlsp[0] + n_mlsp;

  z[0] = *mem;
  z[1] = z[0] + n_mlsp;
  z[2] = z[1] + n_mlsp;

  dlm_lsf2lsp(mlsf+1,mlsp[0],n_mlsf-1);

  for(i_mlsf=0; i_mlsf<n_mlsf-1; i_mlsf++) {
    tmp = 1.0+2.0*mlsp[0][i_mlsf]*lambda+lambda*lambda;
    e[i_mlsf&1] *= tmp;
    mlsp[0][i_mlsf] = -lambda2*(2.0*mlsp[0][i_mlsf]+lambda)/tmp;
    mlsp[1][i_mlsf] = lambda2/tmp;
  }
  if(n_mlsf&1) {
    tmp = 1.0-lambda;
    e[0] *= tmp;
    mlsp[0][n_mlsf-1] = lambda2/tmp;
    mlsp[1][n_mlsf-1] = 0.0;
    tmp = 1.0+lambda;
    e[1] *= tmp;
    mlsp[0][n_mlsf] = -lambda2/tmp;
    mlsp[1][n_mlsf] = 0.0;
  } else {
    tmp = lambda2;
    e[1] *= tmp;
    mlsp[0][n_mlsf-1] = lambda;
    mlsp[1][n_mlsf-1] = -1.0;
  }

  out[0] = 1.0;
  out[1] = 1.0;

  for(i_poly=0; i_poly<n_poly; i_poly++) {
    for(i_mlsf=0; i_mlsf<n_mlsp; i_mlsf++) {
      z[0][i_mlsf] = z[0][i_mlsf] + lambda*z[1][i_mlsf];
      z[1][i_mlsf] = z[1][i_mlsf] + lambda*(z[2][i_mlsf]-z[0][i_mlsf]);
      tmp = out[i_mlsf&1] + (mlsp[0][i_mlsf]*z[0][i_mlsf] + mlsp[1][i_mlsf]*z[1][i_mlsf]);
      z[2][i_mlsf] = z[1][i_mlsf];
      z[1][i_mlsf] = z[0][i_mlsf];
      z[0][i_mlsf] = out[i_mlsf&1];
      out[i_mlsf&1] = tmp;
    }
    poly[i_poly] = (e[0]*out[0]+e[1]*out[1]) / (e[0]+e[1]);
    out[0] = 0.0;
    out[1] = 0.0;
  }

  poly[0] = mlsf[0];

  dlp_free(mlsp[0]);

  return O_K;
}

FLOAT64 CGEN_IGNORE dlm_lsf_synthesize_step(FLOAT64* lsp, INT16 n_lsp, FLOAT64** z, FLOAT64 in) {
  INT16    i_lsp   = 0;
  FLOAT64  tmp     = 0.0;
  FLOAT64  out     = 0.0;
  FLOAT64  out1[2] = { in, in };
  FLOAT64  out2[2] = { 0.0, 0.0 };

  for(i_lsp=0; i_lsp<n_lsp; i_lsp++) {
    out2[i_lsp&1] = out2[i_lsp&1] + lsp[i_lsp]*out1[i_lsp&1] + z[0][i_lsp];
    tmp = out1[i_lsp&1] + lsp[i_lsp]*z[0][i_lsp] + z[1][i_lsp];
    z[1][i_lsp] = z[0][i_lsp];
    z[0][i_lsp] = out1[i_lsp&1];
    out1[i_lsp&1] = tmp;
  }
  if(n_lsp&1) {
    out = (out2[0]+out2[1]-z[2][1])/2.0;
    z[2][1] = out1[1];
  } else {
    out = (out1[0]-out1[1]+out2[0]+out2[1])/2.0;
  }

  return out;
}

/**
 * <p>Synthesize LSF coefficients using LSF synthesis filter</p>
 *
 * @param lsf
 *          Pointer to LSF coefficients <CODE>lsf</CODE>(k) (k=0..n_lsf-1).
 * @param n_lsf
 *          Number of LSF coefficients
 * @param exc
 *          Pointer to excitation signal of length <code>n_exc</code>.
 * @param n_exc
 *          Lenght of excitation signal.
 * @param syn
 *          Pointer to synthesized signal of length <code>n_exc</code>.
 * @param mem
 *          Pointer to working memory. If <code>*mem</code> is NULL,
 *          memory will be allocated but not freed afterwards.
 * @return <code>O_K</code> if successful, a (negative) error code otherwise
 *
 * This filter should be produce the same output as <code>dlm_mlsf_synthesize</code>
 * calling with <code>lambda=0.0</code>.
 *
 * @see dlm_mlsf_synthesize
 */
INT16 dlm_lsf_synthesize(FLOAT64* lsf, INT16 n_lsf, FLOAT64* exc, INT32 n_exc, FLOAT64* syn, FLOAT64** mem) {
  INT16 n_lsp = n_lsf - 1;
  INT16 i_lsp = 0;
  INT32 i_exc = 0;
  FLOAT64* lsp = NULL;
  FLOAT64* lsf_old = NULL;
  FLOAT64* lsf_cur = NULL;
  FLOAT64* z[3] = { NULL, NULL, NULL };

  if(!mem || !syn || !exc || !lsf) return ERR_MDIM;

  if(*mem == NULL) {
    *mem = (FLOAT64*)dlp_calloc(n_lsf+n_lsp*2+2, sizeof(FLOAT64));
    if(!*mem) return ERR_MEM;
    memmove(*mem, lsf, n_lsf*sizeof(FLOAT64));
  }

  lsf_old = *mem;
  z[0] = lsf_old + n_lsf;
  z[1] = z[0] + n_lsp;
  z[2] = z[1] + n_lsp;

  lsf_cur = (FLOAT64*)dlp_malloc((n_lsf)*sizeof(FLOAT64));
  lsp = (FLOAT64*)dlp_malloc((n_lsp)*sizeof(FLOAT64));

  if(!lsf_cur || !lsp) return ERR_MEM;

  for(i_exc=0; i_exc<n_exc; i_exc++) {

    dlm_lsf_interpolate(lsf_old, lsf_cur, lsf, n_lsf, i_exc, n_exc);
    dlm_lsf2lsp(lsf_cur+1, lsp, n_lsf-1);
    for(i_lsp=0; i_lsp<n_lsp; i_lsp++) lsp[i_lsp] *= -2.0;

    syn[i_exc] = lsf_cur[0]*exc[i_exc]-dlm_lsf_synthesize_step(lsp,n_lsp,z,z[2][0]);
    z[2][0] = syn[i_exc];
  }

  memmove(lsf_old, lsf, n_lsf*sizeof(FLOAT64));

  dlp_free(lsp);
  dlp_free(lsf_cur);
  return O_K;
}

FLOAT64 CGEN_IGNORE dlm_mlsf_synthesize_step(FLOAT64** mlsp, INT16 n_mlsp, FLOAT64* e, FLOAT64** z, FLOAT64 lambda, FLOAT64 in) {
  INT16    i_mlsp    = 0;
  FLOAT64  lambda2   = 1.0-lambda*lambda;
  FLOAT64  out       = 0.0;
  FLOAT64  tmp1      = 0.0;
  FLOAT64  tmp2      = 0.0;
  FLOAT64  tmp3      = 0.0;
  FLOAT64  out1[2]   = { in, in };
  FLOAT64  out2[2]   = { 0.0, 0.0 };

  for(i_mlsp=0; i_mlsp<n_mlsp; i_mlsp++) {
    tmp1 = z[0][i_mlsp]+lambda*z[1][i_mlsp];
    tmp2 = z[1][i_mlsp]+lambda*z[2][i_mlsp];
    tmp3 = out1[i_mlsp&1] + lambda2*(mlsp[0][i_mlsp]*tmp1+lambda2*mlsp[1][i_mlsp]*tmp2);
    out2[i_mlsp&1] = out2[i_mlsp&1] + mlsp[0][i_mlsp]*out1[i_mlsp&1] + lambda2*mlsp[1][i_mlsp]*tmp1;
    z[2][i_mlsp] = tmp2;
    z[1][i_mlsp] = tmp1;
    z[0][i_mlsp] = out1[i_mlsp&1];
    out1[i_mlsp&1] = tmp3;
  }

  if(n_mlsp&1) {
    z[3][3] = z[3][2]+lambda*z[3][3];
    z[3][2] = out1[1];
    out1[1] = 2.0*lambda*out1[1] - lambda2*z[3][3];
    out = lambda2*(e[1]*(out1[1]+lambda2*out2[1])+e[0]*out2[0]);
  } else {
    out = lambda2*(e[0]*((1.0-lambda)*out2[0]+out1[0])+e[1]*((1.0+lambda)*out2[1]-out1[1]));
  }
  return out;
}

/**
 * <p>Synthesize (Mel-)LSF coefficients using LSF synthesis filter</p>
 *
 * @param lsf
 *          Pointer to (Mel-)LSF coefficients <CODE>mlsf</CODE>(k) (k=0..n_lsf-1).
 * @param n_lsf
 *          Number of (Mel-)LSF coefficients
 * @param exc
 *          Pointer to excitation signal of length <code>n_exc</code>.
 * @param n_exc
 *          Lenght of excitation signal.
 * @param lambda
 *          Warping factor.
 * @param syn
 *          Pointer to synthesized signal of length <code>n_exc</code>.
 * @param mem
 *          Pointer to working memory. If <code>*mem</code> is <code>NULL</code>,
 *          memory will be allocated but not freed afterwards.
 * @return <code>O_K</code> if successfull, a (negative) error code otherwise
 *
 * If <code>lambda=0.0</code> this should be produce the same output as <code>dlm_lsf_synthesize</code>.

 * @see dlm_lsf_synthesize
 */
INT16 dlm_mlsf_synthesize(FLOAT64* mlsf, INT16 n_mlsf, FLOAT64* exc, INT32 n_exc, FLOAT64 lambda, FLOAT64* syn, FLOAT64** mem) {
  INT16    i_mlsp    = 0;
  INT32    i_exc     = 0;
  INT16    n_mlsp    = n_mlsf - 1;
  FLOAT64  lambda2   = 1.0-lambda*lambda;
  FLOAT64  tmp      = 0.0;
  FLOAT64* mlsp[2]   = { NULL, NULL };
  FLOAT64* z[4]      = { NULL, NULL, NULL, NULL };
  FLOAT64  e[2]      = { 1.0, 1.0 };
  FLOAT64* mlsf_old  = NULL;
  FLOAT64* mlsf_cur  = NULL;

  if(!mem || !syn || !exc || !mlsf) return ERR_MDIM;

  if(*mem == NULL) {
    *mem = (FLOAT64*)dlp_calloc(n_mlsf+3*n_mlsp+4,sizeof(FLOAT64));
    if(!*mem) return ERR_MEM;
    memmove(*mem,mlsf,n_mlsf*sizeof(FLOAT64));
  }

  mlsf_cur = (FLOAT64*)dlp_malloc((n_mlsf)*sizeof(FLOAT64));
  mlsp[0]  = (FLOAT64*)dlp_calloc(2*n_mlsp,sizeof(FLOAT64));
  mlsp[1]  = mlsp[0] + n_mlsp;

  mlsf_old = *mem;
  z[0] = mlsf_old + n_mlsf;
  z[1] = z[0] + n_mlsp;
  z[2] = z[1] + n_mlsp;
  z[3] = z[2] + n_mlsp;

  for(i_exc=0; i_exc<n_exc; i_exc++) {

    dlm_lsf_interpolate(mlsf_old, mlsf_cur, mlsf, n_mlsf, i_exc, n_exc);
    dlm_lsf2lsp(mlsf_cur+1, mlsp[0], n_mlsf-1);

    e[0] = e[1] = 1.0;
    for(i_mlsp=0; i_mlsp<n_mlsp; i_mlsp++) {
      tmp = 1.0+2.0*mlsp[0][i_mlsp]*lambda+lambda*lambda;
      e[i_mlsp&1] *= tmp;
      mlsp[0][i_mlsp] = (-2.0*mlsp[0][i_mlsp]-2.0*lambda)/tmp;
      mlsp[1][i_mlsp] = 1.0/tmp;
    }

    z[3][1] = z[3][0]+lambda*z[3][1];

    if(n_mlsp&1) {
      syn[i_exc] = (2.0*mlsf_cur[0]*exc[i_exc]-dlm_mlsf_synthesize_step(mlsp, n_mlsp, e, z, lambda, z[3][1]))/(e[0]+e[1]*lambda2);
    } else {
      syn[i_exc] = (2.0*mlsf_cur[0]*exc[i_exc]-dlm_mlsf_synthesize_step(mlsp, n_mlsp, e, z, lambda, z[3][1]))/(e[0]*(1-lambda)+e[1]*(1+lambda));
    }

    z[3][0] = syn[i_exc];
  }

  memmove(*mem, mlsf, n_mlsf*sizeof(FLOAT64));
  dlp_free(mlsp[0]);
  dlp_free(mlsf_cur);

  return O_K;
}

/**
 * <p>Conversion of line spectral frequencies (MLSF) to prediction polynomial
 * using FIR filter structure.</p>
 *
 * @param lsf
 *          Pointer to MLSF coefficients <CODE>lsf</CODE>(k) (k = 0 ... n_lsf - 1).
 * @param n_lsf
 *          Order of MLSF coefficients.
 * @param poly
 *          Pointer to resulting polynomial coefficients.
 * @param n_poly
 *          Order of resulting polynomial. Can be different than n_mlsf.
 * @param mem
 *          Pointer to working memory. If <code>*mem</code> is <code>NULL</code>
 *          memory will be allocated but not freed afterwards.
 * @return <code>O_K</code> if successful, a (negative) error code otherwise
 */
INT16 dlm_lsf2poly_filt(FLOAT64* lsf, INT16 n_lsf, FLOAT64* poly, INT16 n_poly, FLOAT64** mem) {
  INT16      i_lsp    = 0;
  INT16      i_poly   = 0;
  INT16      n_lsp    = n_lsf+(n_lsf&1);
  FLOAT64    tmp      = 0.0;
  FLOAT64    out[2]   = { 0.0, 0.0 };
  FLOAT64*   lsp[2]   = { NULL, NULL };
  FLOAT64*   z[2]     = { NULL, NULL };

  if(!mem || !lsf || !poly) return ERR_MDIM;

  if(*mem == NULL) {
    *mem = (FLOAT64*)dlp_calloc(2*n_lsp,sizeof(FLOAT64));
    if(!*mem) return ERR_MEM;
  }

  lsp[0]  = (FLOAT64*)dlp_calloc(2*n_lsp,sizeof(FLOAT64));
  lsp[1] = lsp[0] + n_lsp;

  z[0] = *mem;
  z[1] = z[0] + n_lsp;

  dlm_lsf2lsp(lsf+1,lsp[0],n_lsf-1);
  for(i_lsp=0; i_lsp<n_lsf-1; i_lsp++) {
    lsp[0][i_lsp] = -2.0*lsp[0][i_lsp];
    lsp[1][i_lsp] = 1.0;
  }

  if(n_lsf&1) {
    lsp[0][n_lsf-1] = 1.0;
    lsp[1][n_lsf-1] = 0.0;
    lsp[0][n_lsf] = -1.0;
    lsp[1][n_lsf] = 0.0;
  } else {
    lsp[0][n_lsf-1] = 0.0;
    lsp[1][n_lsf-1] = -1.0;
  }

/*  for(i=1; i<order; i++) r[i-1] = -2.0*cos(lsf[i]); */

  out[0] = 1.0;
  out[1] = 1.0;

  for(i_poly=0; i_poly<n_poly; i_poly++) {
    for(i_lsp=0; i_lsp<n_lsp; i_lsp++) {
      tmp = out[i_lsp&1] + lsp[0][i_lsp]*z[0][i_lsp] + lsp[1][i_lsp]*z[1][i_lsp];
      z[1][i_lsp] = z[0][i_lsp];
      z[0][i_lsp] = out[i_lsp&1];
      out[i_lsp&1] = tmp;
    }
    poly[i_poly] = 0.5 * (out[0]+out[1]);
    out[0] = 0.0;
    out[1] = 0.0;
  }

  poly[0] = lsf[0];

  dlp_free(lsp[0]);
  return O_K;
}

/**
 * <p>Conversion of line spectral frequencies (LSF) to mel-line spectral frequencies (MLSF).</p>
 *
 * @param lsf
 *          Pointer to LSF parameters <CODE>lsf</CODE>(k) (k = 0 ... n - 1).
 * @param mlsf
 *          Pointer to MLSF parameters <CODE>mlsf</CODE>(k) (k = 0 ... n - 1).
 * @param n
 *          Number of parameters of <CODE>lsf</CODE> and <CODE>mlsf</CODE>.
 * @param lambda
 *          Warping factor &lambda;.
 * @return <code>O_K</code> if successful, a (negative) error code otherwise
 */
INT16 dlm_lsf2mlsf(FLOAT64* lsf, FLOAT64* mlsf, INT16 n, FLOAT64 lambda) {
  INT32    i    = 0;
  FLOAT64  tmp1 = 0.0;
  FLOAT64  tmp2 = 0.0;
  FLOAT64  tmp4 = 0.0;
  INT32    tmp3 = 0;
  FLOAT64* dlsf = NULL;

  if(!lsf || !mlsf) return ERR_MDIM;

  dlsf = (FLOAT64*)dlp_calloc(n, sizeof(FLOAT64));

  if(!dlsf) return ERR_MEM;

  for(i = 0; i < n; i++) {
    dlsf[i] = lsf[i] - (FLOAT64)(i+1) * F_PI / (FLOAT64)(n);
  }

  for(i = 0; i < n; i++) {
    tmp1 = i * F_PI / (FLOAT64)(n - 1);
    tmp2 = i - 2.0 * (n-1) * atan2(lambda * sin(tmp1), 1.0 + lambda * cos(tmp1)) / F_PI;
    tmp3 = (INT32)tmp2;
    tmp4 = tmp2 - tmp3;
    if(tmp3 < (n-1)) {
      mlsf[i] = dlsf[tmp3] + (dlsf[tmp3+1] - dlsf[tmp3]) * tmp4;
    } else {
      mlsf[i] = dlsf[tmp3];
    }
    if((tmp3 > 0) && (tmp3 < (n-2))) {
      mlsf[i] = mlsf[i] - (tmp4 * (1.0 - tmp4) / 4.0 * ((dlsf[tmp3+2] - dlsf[tmp3+1]) - (dlsf[tmp3] - dlsf[tmp3-1])));
    }
  }

  for(i = 0; i < n; i++) {
    mlsf[i] = mlsf[i] + (FLOAT64)(i+1) * F_PI / (FLOAT64)(n);
  }

  mlsf[0] = MAX(0.0, mlsf[0]);
  mlsf[n-1] = MIN(F_PI, mlsf[n-1]);

  dlp_free(dlsf);

  return O_K;
}
