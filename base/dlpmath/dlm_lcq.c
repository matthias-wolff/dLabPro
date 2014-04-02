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

FLOAT64 CGEN_IGNORE dlm_lcq_synthesize_step(FLOAT64* lcp, INT16 n_lcp, FLOAT64* mem) {
  INT16    i_lcp   = 0;
  FLOAT64  tmp     = 0.0;
  FLOAT64  out     = 0.0;
  FLOAT64  out1[2] = { 0.0, 0.0 };
  FLOAT64  out2[2] = { 0.0, 0.0 };
  FLOAT64* z[3]    = { mem, mem+n_lcp, mem+2*n_lcp };

  out1[0] = z[2][0];
  out2[0] = 0.0;
  out1[1] = out1[0];
  out2[1] = 0.0;

  for(i_lcp=0; i_lcp<n_lcp; i_lcp++) {
    tmp = out1[i_lcp&1] + lcp[i_lcp]*z[0][i_lcp]+z[1][i_lcp];
    out2[i_lcp&1] = out2[i_lcp&1] + lcp[i_lcp]*out1[i_lcp&1] + z[0][i_lcp];
    z[1][i_lcp] = z[0][i_lcp];
    z[0][i_lcp] = out1[i_lcp&1];
    out1[i_lcp&1] = tmp;
  }

  if(n_lcp&1) {
    out = (out2[0]+out2[1]-z[2][1])/2.0;
    z[2][1] = out1[1];
  } else {
    out = (out1[0]-out1[1]+out2[0]+out2[1])/2.0;
  }

  return out;
}

/**
 * <p>Synthesize LCQ coefficients using nested filter</p>
 *
 * @param lcq
 *          Pointer to LCQ coefficients <CODE>lcq</CODE>(k) (k=0..n_lcq-1).
 * @param n_lcq
 *          Number of LCQ coefficients
 * @param exc
 *          Pointer to excitation signal of length <code>n_exc</code>.
 * @param n_exc
 *          Length of excitation signal.
 * @param n_pade_order
 *          Order of Pade approximation.
 * @param syn
 *          Pointer to synthesized signal of length <code>n_exc</code>.
 * @param mem
 *          Pointer to filter states. Pointer must be either of size
 *          <code>n_pade_order*sizeof(FLOAT64*)</code> or can be <code>NULL</code>.
 * @return <code>O_K</code> if successfull, a (negative) error code otherwise
 *
 */
INT16 dlm_lcq_synthesize(FLOAT64* lcq, INT16 n_lcq, FLOAT64* exc, INT32 n_exc, INT16 n_pade_order, FLOAT64* syn, FLOAT64** mem) {
  INT16        i_lcp     = 0;
  INT16        i_exc     = 0;
  INT16        n_lcp     = n_lcq - 1;
  INT16        i_pade_order = 0;
  FLOAT64      pos[MAX_PADE_ORDER+1];                  /* Pointer to outer filter states   */
  FLOAT64*     lcq_old   = NULL;
  FLOAT64*     lcq_cur   = NULL;
  FLOAT64*     lcp       = NULL;
  FLOAT64*     z[MAX_PADE_ORDER][4];
  static const FLOAT64 pade[][MAX_PADE_ORDER] = {
      { 0.5, 0.0,      0.0,      0.0,      0.0,      0.0,      0.0      },
      { 0.5, 1.0/ 6.0, 0.0,      0.0,      0.0,      0.0,      0.0      },
      { 0.5, 1.0/ 5.0, 1.0/12.0, 0.0,      0.0,      0.0,      0.0      },
      { 0.5, 3.0/14.0, 1.0/ 9.0, 1.0/20.0, 0.0,      0.0,      0.0      },
      { 0.5, 2.0/ 9.0, 1.0/ 8.0, 1.0/14.0, 1.0/30.0, 0.0,      0.0      },
      { 0.5, 5.0/22.0, 2.0/15.0, 1.0/12.0, 1.0/20.0, 1.0/42.0, 0.0      },
      { 0.5, 3.0/13.0, 5.0/36.0, 1.0/11.0, 3.0/50.0, 1.0/27.0, 1.0/56.0 } };

  if(n_pade_order > MAX_PADE_ORDER) {
    return ERR_MDIM;
  }

  if(!mem || !syn || !exc || !lcq) return ERR_MDIM;

  if(*mem == NULL) {
    *mem = (FLOAT64*)dlp_calloc(n_lcq+n_pade_order*(2*n_lcp+2),sizeof(FLOAT64));
    memmove(*mem,lcq,n_lcq*sizeof(FLOAT64));
  }

  memset(pos, 0L, (MAX_PADE_ORDER+1)*sizeof(FLOAT64));

  lcq_cur = (FLOAT64*)dlp_malloc((n_lcq)*sizeof(FLOAT64));
  lcp  = (FLOAT64*)dlp_calloc(n_lcp,sizeof(FLOAT64));


  lcq_old = *mem;
  for(i_pade_order = 0; i_pade_order<n_pade_order; i_pade_order++) {
    z[i_pade_order][0] = lcq_old+n_lcq + i_pade_order*(2*n_lcp+2);
    z[i_pade_order][1] = z[i_pade_order][0] + n_lcp;
    z[i_pade_order][2] = z[i_pade_order][1] + n_lcp;
    z[i_pade_order][3] = z[i_pade_order][2] + n_lcp;
  }

  for(i_exc=0; i_exc<n_exc; i_exc++) {

    dlm_lsf_interpolate(lcq_old, lcq_cur, lcq, n_lcq, i_exc, n_exc);
    dlm_lsf2lsp(lcq_cur+1, lcp, n_lcq-1);
    for(i_lcp=0; i_lcp<n_lcp; i_lcp++) lcp[i_lcp] *= -2.0;

    for(i_pade_order = 0; i_pade_order < n_pade_order; i_pade_order++) {
      pos[i_pade_order+1] = 2.0*lcq_cur[0]*dlm_lsf_synthesize_step(lcp, n_lcp, z[i_pade_order], z[i_pade_order][2][0]);
      pos[i_pade_order+1] *= pade[n_pade_order-1][i_pade_order];
    }

    pos[0] = exp(lcq_cur[0])*exc[i_exc];
    for(i_pade_order=1; i_pade_order <= n_pade_order; i_pade_order+=2) pos[0] += pos[i_pade_order];
    for(i_pade_order=2; i_pade_order <= n_pade_order; i_pade_order+=2) pos[0] -= pos[i_pade_order];
    for(i_pade_order=0; i_pade_order <= n_pade_order; i_pade_order++)  syn[i_exc] += pos[i_pade_order];
    for(i_pade_order=0; i_pade_order <  n_pade_order; i_pade_order++)  z[i_pade_order][2][0] = pos[i_pade_order];
  }

  memmove(lcq_old, lcq, n_lcq*sizeof(FLOAT64));
  dlp_free(lcp);
  dlp_free(lcq_cur);

  return O_K;
}

FLOAT64 CGEN_IGNORE dlm_mlcq_synthesize_step(FLOAT64* lcp, INT16 n_lcp, FLOAT64 lambda, FLOAT64* mem) {
  INT16      i_lcp   = 0;
  FLOAT64  tmp1    = 0.0;
  FLOAT64  tmp2    = 0.0;
  FLOAT64  tmp3    = 0.0;
  FLOAT64  out     = 0.0;
  FLOAT64  lambda2 = 1.0-lambda*lambda;
  FLOAT64  out1[2] = { 0.0, 0.0 };
  FLOAT64  out2[2] = { 0.0, 0.0 };
  FLOAT64* r[2]    = { lcp+1, lcp+1+n_lcp };
  FLOAT64* z[4]    = { mem, mem+n_lcp, mem+2*n_lcp, mem+3*n_lcp };

  z[3][1] = z[3][0]+lambda*z[3][1];

  out1[0] = z[3][1];
  out2[0] = 0.0;
  out1[1] = out1[0];
  out2[1] = 0.0;

  for(i_lcp=0; i_lcp<n_lcp; i_lcp++) {
    tmp1 = z[0][i_lcp]+lambda*z[1][i_lcp];
    tmp2 = z[1][i_lcp]+lambda*z[2][i_lcp];
    tmp3 = out1[i_lcp&1] + lambda2*(r[0][i_lcp]*tmp1+lambda2*r[1][i_lcp]*tmp2);
    out2[i_lcp&1] = out2[i_lcp&1] + r[0][i_lcp]*out1[i_lcp&1] + lambda2*r[1][i_lcp]*tmp1;
    z[2][i_lcp] = tmp2;
    z[1][i_lcp] = tmp1;
    z[0][i_lcp] = out1[i_lcp&1];
    out1[i_lcp&1] = tmp3;
  }

  if(n_lcp&1) {
    z[3][3] = z[3][2]+lambda*z[3][3];
    z[3][2] = out1[1];
    out1[1] = 2.0*lambda*out1[1] - lambda2*z[3][3];
    out = (out1[1]+lambda2*(out2[0]+out2[1]))/2.0;
  } else {
    out = (out1[0]*(1.0+lambda)-out1[1]*(1.0-lambda)+lambda2*(out2[0]+out2[1]))/2.0;
  }

  return out;
}

/**
 * <p>Synthesize LCQ coefficients using nested filter</p>
 *
 * @param mlcq
 *          Pointer to LCQ coefficients <CODE>lcq</CODE>(k) (k=0..n_lcq-1).
 * @param n_mlcq
 *          Number of LCQ coefficients
 * @param exc
 *          Pointer to excitation signal of length <code>n_exc</code>.
 * @param n_exc
 *          Length of excitation signal.
 * @param n_pade_order
 *          Order of Pade approximation.
 * @param lambda
 *          Warping factor.
 * @param syn
 *          Pointer to synthesized signal of length <code>n_exc</code>.
 * @param mem
 *          Pointer to filter states. Pointer must be either of size
 *          <code>n_pade_order*sizeof(FLOAT64*)</code> or can be <code>NULL</code>.
 * @return <code>O_K</code> if successful, a (negative) error code otherwise
 *
 */
INT16 dlm_mlcq_synthesize(FLOAT64* mlcq, INT16 n_mlcq, FLOAT64* exc, INT32 n_exc, INT16 n_pade_order, FLOAT64 lambda, FLOAT64* syn, FLOAT64** mem) {
  INT16    i_mlcp       = 0;
  INT16    i_exc        = 0;
  INT16    n_mlcp       = n_mlcq - 1;
  FLOAT64  lambda2      = 1.0-lambda*lambda;
  INT16    i_pade_order = 0;
  FLOAT64  tmp          = 0.0;
  FLOAT64  pos[MAX_PADE_ORDER+1];                  /* Pointer to outer filter states   */
  FLOAT64* mlcp[2]      = { NULL, NULL };
  FLOAT64* z[MAX_PADE_ORDER][4];
  FLOAT64  e[2]         = { 1.0, 1.0 };
  FLOAT64* mlcq_old     = NULL;
  FLOAT64* mlcq_cur     = NULL;
  static const FLOAT64 pade[][MAX_PADE_ORDER] = {
      { 0.5, 0.0,      0.0,      0.0,      0.0,      0.0,      0.0      },
      { 0.5, 1.0/ 6.0, 0.0,      0.0,      0.0,      0.0,      0.0      },
      { 0.5, 1.0/ 5.0, 1.0/12.0, 0.0,      0.0,      0.0,      0.0      },
      { 0.5, 3.0/14.0, 1.0/ 9.0, 1.0/20.0, 0.0,      0.0,      0.0      },
      { 0.5, 2.0/ 9.0, 1.0/ 8.0, 1.0/14.0, 1.0/30.0, 0.0,      0.0      },
      { 0.5, 5.0/22.0, 2.0/15.0, 1.0/12.0, 1.0/20.0, 1.0/42.0, 0.0      },
      { 0.5, 3.0/13.0, 5.0/36.0, 1.0/11.0, 3.0/50.0, 1.0/27.0, 1.0/56.0 } };

  if(n_pade_order > MAX_PADE_ORDER) {
    return ERR_MDIM;
  }

  if(!mem || !syn || !exc || !mlcq) return ERR_MDIM;

  if(*mem == NULL) {
    *mem = (FLOAT64*)dlp_calloc(n_mlcq+n_pade_order*(3*n_mlcp+4),sizeof(FLOAT64));
    memmove(*mem,mlcq,n_mlcq*sizeof(FLOAT64));
  }

  mlcq_cur = (FLOAT64*)dlp_malloc((n_mlcq)*sizeof(FLOAT64));
  mlcp[0]  = (FLOAT64*)dlp_calloc(2*n_mlcp,sizeof(FLOAT64));
  mlcp[1]  = mlcp[0]+n_mlcp;

  mlcq_old = *mem;
  for(i_pade_order = 0; i_pade_order<n_pade_order; i_pade_order++) {
    z[i_pade_order][0] = mlcq_old+n_mlcq + i_pade_order*(3*n_mlcp+4);
    z[i_pade_order][1] = z[i_pade_order][0] + n_mlcp;
    z[i_pade_order][2] = z[i_pade_order][1] + n_mlcp;
    z[i_pade_order][3] = z[i_pade_order][2] + n_mlcp;
  }

  for(i_exc=0; i_exc<n_exc; i_exc++) {

    dlm_lsf_interpolate(mlcq_old, mlcq_cur, mlcq, n_mlcq, i_exc, n_exc);
    dlm_lsf2lsp(mlcq_cur+1, mlcp[0], n_mlcq-1);

    e[0] = e[1] = 1.0;
    for(i_mlcp=0; i_mlcp<n_mlcp; i_mlcp++) {
      tmp = 1.0+2.0*mlcp[0][i_mlcp]*lambda+lambda*lambda;
      e[i_mlcp&1] *= tmp;
      mlcp[0][i_mlcp] = (-2.0*mlcp[0][i_mlcp]-2.0*lambda)/tmp;
      mlcp[1][i_mlcp] = 1.0/tmp;
    }


    for(i_pade_order = 0; i_pade_order < n_pade_order; i_pade_order++) {
      z[i_pade_order][3][1] = z[i_pade_order][3][0]+lambda*z[i_pade_order][3][1];
      if(n_mlcq&1) {
        pos[i_pade_order+1] = 2.0*mlcq_cur[0]*dlm_mlsf_synthesize_step(mlcp,n_mlcp,e,z[i_pade_order],lambda,z[i_pade_order][3][1])/(e[0]+e[1]*lambda2);
      } else {
        pos[i_pade_order+1] = 2.0*mlcq_cur[0]*dlm_mlsf_synthesize_step(mlcp,n_mlcp,e,z[i_pade_order],lambda,z[i_pade_order][3][1])/(e[0]*(1-lambda)+e[1]*(1+lambda));
      }
      pos[i_pade_order+1] *= pade[n_pade_order-1][i_pade_order];
    }

    if(n_mlcp&1) {
      pos[0] = exp(2.0*mlcq_cur[0]/(e[0]+e[1]*lambda2)) * exc[i_exc];
    } else {
      pos[0] = exp(2.0*mlcq_cur[0]/(e[0]*(1-lambda)+e[1]*(1+lambda))) * exc[i_exc];
    }

    for(i_pade_order=1; i_pade_order <= n_pade_order; i_pade_order+=2) pos[0] += pos[i_pade_order];
    for(i_pade_order=2; i_pade_order <= n_pade_order; i_pade_order+=2) pos[0] -= pos[i_pade_order];
    for(i_pade_order=0; i_pade_order <= n_pade_order; i_pade_order++)  syn[i_exc] += pos[i_pade_order];
    for(i_pade_order=0; i_pade_order <  n_pade_order; i_pade_order++)  z[i_pade_order][3][0] = pos[i_pade_order];
  }

  memmove(*mem, mlcq, n_mlcq*sizeof(FLOAT64));
  dlp_free(mlcp[0]);
  dlp_free(mlcq_cur);

  return O_K;
}
