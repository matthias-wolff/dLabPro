/* dLabPro mathematics library
 * - Linear Predictive Coding
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
 * <p>Bilinear transformation of LPC coefficients.</p>
 *
 * @param a
 *          Pointer to lpc coefficients to be transformed.
 * @param a_len
 *          Amount of input LPC coefficients stored in <CODE>a</CODE>.
 * @param am
 *          Pointer to transformed LPC coefficients.
 * @param am_len
 *          Amount of output Mel-LPC coefficients stored in <CODE>am</CODE>.
 * @param lambda
 *          Bilinear transformation factor (warping factor).
 * @return <code>O_K</code> if successfull, a (negative) error code otherwise
 */
INT16 dlm_lpc2mlpc(FLOAT64* a, INT16 a_len, FLOAT64* am, INT16 am_len, FLOAT64 lambda) {
  INT16 i;
  INT16 m;
  INT16 b = 0;
  FLOAT64 lambda2 = 1 - lambda * lambda;
  FLOAT64* am0 = NULL;
  FLOAT64* aa = NULL;

  if ((!a) || (!am)) return ERR_MEM;

  if (lambda == 0.0) {
    for (i = 0; i < MIN(a_len, am_len); i++)
      am[i] = a[i];
    for (; i < am_len; i++)
      am[i] = 0.0;
    return O_K;
  }

  if (a == am) {
    aa = (FLOAT64*) dlp_malloc(a_len*sizeof(FLOAT64));
    memcpy(aa, a, a_len * sizeof(FLOAT64));
    b = 1;
  } else {
    aa = a;
  }

  am0 = (FLOAT64*) dlp_calloc(am_len, sizeof(FLOAT64));
  dlp_memset(am, 0L, am_len * sizeof(FLOAT64));

  for (i = -(a_len - 1); i <= 0; i++) {
    am[0] = aa[-i] + lambda * am0[0];
    am[1] = lambda2 * am0[0] + lambda * am0[1];
    for (m = 2; m < am_len; m++) {
      am[m] = am0[m - 1] + lambda * (am0[m] - am[m - 1]);
    }
    dlp_memmove(am0, am, am_len * sizeof(FLOAT64));
  }
  dlp_free(am0);
  if (b == 1) {
    dlp_free(aa);
  }
  return O_K;
}

/**
 * <p>Bilinear transformation of LPC coefficients.</p>
 *
 * @param am
 *          Pointer to Mel-LPC coefficients to be transformed.
 * @param am_len
 *          Amount of input Mel-LPC coefficients stored in <CODE>am</CODE>.
 * @param a
 *          Pointer to LPC coefficients.
 * @param a_len
 *          Amount of output LPC coefficients stored in <CODE>a</CODE>.
 * @param lambda
 *          Bilinear transformation factor (warping factor).
 * @return <code>O_K</code> if successfull, a (negative) error code otherwise
 */
INT16 dlm_mlpc2lpc(FLOAT64* am, INT16 am_len, FLOAT64* a, INT16 a_len, FLOAT64 lambda) {
  return dlm_lpc2mlpc(am, am_len, a, a_len, -lambda);
}

FLOAT64 CGEN_IGNORE sum2(FLOAT64* vek1, FLOAT64* vek2, INT32 len) {
  INT32 i;
  FLOAT64 sum = 0;

  for (i = 0; i < len; i++) {
    sum += *(vek1 + i) * *(vek2 + i);
  }

  return sum;
}

/**
 * <p>LPC parameter estimation via Burg method.</p>
 *
 * @param samples
 *          Pointer to signal samples to be analysed.
 * @param nSamples
 *          Amount of signal samples stored in <CODE>samples</CODE>.
 * @param a
 *          Pointer to resulting LPC coefficients <CODE>a</CODE>(k) (k = 0 ... p - 1).
 * @param p
 *          Order of LPC analysis.
 * @param scale
 *          Signal scaling factor.
 * @return <code>O_K</code> if successfull, a (negative) error code otherwise
 */
INT16 dlm_lpc_burg(FLOAT64* samples, INT32 nSamples, FLOAT64* a, INT16 p, FLOAT64 scale) {
  register INT32 i = 0;
  INT16 m = 0;
  FLOAT64 den = 0.0;
  FLOAT64 num = 0.0;
  FLOAT64 rc = 0.0;
  FLOAT64 alpha = 0.0;
  FLOAT64* aa = NULL;
  FLOAT64* aaa = NULL;
  FLOAT64* eb = NULL;
  FLOAT64* ef = NULL;
  FLOAT64* efp = NULL;
  FLOAT64* ebp = NULL;

  if ((!samples) || (!a)) return NOT_EXEC;

  if (((p - 1) > nSamples) || ((p - 1) < 1)) {
    return ERR_MDIM;
  }

  aa = (FLOAT64*) dlp_calloc(p+p+nSamples+nSamples+nSamples+nSamples, sizeof(FLOAT64));
  aaa = aa + p;
  eb = aaa + p;
  ef = eb + nSamples;
  efp = ef + nSamples;
  ebp = efp + nSamples;

  p--;
  for (i = 0; i < nSamples; i++) {
    *(eb + i) = (FLOAT64) (*(samples + i));
    *(ef + i) = (FLOAT64) (*(samples + i));
  }

  *(aaa + 0) = 1.0;

  alpha = sum2(ef, ef, nSamples);

  for (m = 1; m <= p; m++) {
    memcpy(efp, ef + m, (nSamples - m) * sizeof(FLOAT64));
    memcpy(ebp, eb + m - 1, (nSamples - m) * sizeof(FLOAT64));

    num = -2.0 * sum2(ebp, efp, nSamples - m);
    den = sum2(efp, efp, nSamples - m) + sum2(ebp, ebp, nSamples - m);

    rc = num / (den + 1.0e-20);
    *(aaa + m) = -rc;

    memcpy(efp, ef, nSamples * sizeof(FLOAT64));
    memcpy(ebp, eb, nSamples * sizeof(FLOAT64));

    for (i = 1; i < nSamples; i++) {
      *(ef + i) = *(ef + i) + rc * *(ebp + i - 1);
      *(eb + i) = *(ebp + i - 1) + rc * *(efp + i);
    }

    alpha *= 1.0 - rc * rc;

    for (i = 1; i < m; i++) {
      *(aaa + i) = *(aa + i) + rc * *(aa + m - i);
    }
    memcpy(aa, aaa, (p + 1) * sizeof(FLOAT64));
  }

  *(aaa + 0) = sqrt(alpha) * scale;

  memcpy(a, aaa, (p + 1) * sizeof(FLOAT64));

  dlp_free(aa);

  return O_K;
}

/**
 * <p>LPC parameter estimation via Burg method.</p>
 *
 * @param samples
 *          Pointer to signal samples to be analysed.
 * @param nSamples
 *          Amount of signal samples stored in <CODE>samples</CODE>.
 * @param a
 *          Pointer to resulting LPC coefficients <CODE>a</CODE>(k) (k = 0 ... p - 1).
 * @param p
 *          Order of LPC analysis.
 * @param lambda
 *          Warping factor.
 * @param scale
 *          Signal scaling factor.
 * @return <code>O_K</code> if successfull, a (negative) error code otherwise
 */
INT16 dlm_lpc_mburg(FLOAT64* samples, INT32 nSamples, FLOAT64* a, INT16 p, FLOAT64 lambda, FLOAT64 scale) {
  register INT32 i = 0;
  INT16 m = 0;
  FLOAT64 den = 0.0;
  FLOAT64 num = 0.0;
  FLOAT64 rc = 0.0;
  FLOAT64 alpha = 0.0;
  FLOAT64* aa = NULL;
  FLOAT64* aaa = NULL;
  FLOAT64* eb = NULL;
  FLOAT64* ef = NULL;
  FLOAT64* efp = NULL;
  FLOAT64* ebpw = NULL;

  if (lambda == 0.0) return dlm_lpc_burg(samples, nSamples, a, p, scale);
  if ((!samples) || (!a)) return NOT_EXEC;

  if (((p - 1) > nSamples) || ((p - 1) < 1)) {
    return ERR_MDIM;
  }

  aa = (FLOAT64*) dlp_calloc(p+p+nSamples+nSamples+nSamples+nSamples, sizeof(FLOAT64));
  aaa = aa + p;
  eb = aaa + p;
  ef = eb + nSamples;
  efp = ef + nSamples;
  ebpw = efp + nSamples;

  p--;
  for (i = 0; i < nSamples; i++) {
    *(eb + i) = (FLOAT64) (*(samples + i));
    *(ef + i) = (FLOAT64) (*(samples + i));
  }

  *(aaa + 0) = 1.0;

  alpha = sum2(ef, ef, nSamples);

  for (m = 1; m <= p; m++) {
    memcpy(efp, ef + m, (nSamples - m) * sizeof(FLOAT64));
    memset(ebpw, 0, nSamples * sizeof(FLOAT64));
    for (i = m; i < nSamples; i++) {
      ebpw[i] = eb[i - 1] + lambda * (ebpw[i - 1] - eb[i]);
    }
    memmove(ebpw, ebpw + m, (nSamples - m) * sizeof(FLOAT64));

    num = -2.0 * sum2(ebpw, efp, nSamples - m);
    den = sum2(efp, efp, nSamples - m) + sum2(ebpw, ebpw, nSamples - m);

    rc = num / (den + 1.0e-20);
    *(aaa + m) = -rc;

    memcpy(efp, ef, nSamples * sizeof(FLOAT64));

    for (i = m; i < nSamples; i++) {
      *(ef + i) = *(ef + i) + rc * *(ebpw + i - m);
      *(eb + i) = *(ebpw + i - m) + rc * *(efp + i);
    }

    alpha *= 1.0 - rc * rc;

    for (i = 1; i < m; i++) {
      *(aaa + i) = *(aa + i) + rc * *(aa + m - i);
    }
    memcpy(aa, aaa, (p + 1) * sizeof(FLOAT64));
  }

  *(aaa + 0) = sqrt(alpha) * scale;

  memcpy(a, aaa, (p + 1) * sizeof(FLOAT64));

  dlp_free(aa);

  return O_K;
}

void CGEN_IGNORE macorr(FLOAT64* x, INT32 l, FLOAT64* r, INT32 np, FLOAT64 lambda) {
  FLOAT64 sum;
  FLOAT64 y[l];
  register INT32 k, i;

  memcpy(y, x, l * sizeof(FLOAT64));

  for (k = 0; k < np; ++k) {
    for (sum = i = 0; i < l - k; ++i) {
      sum += x[i + k] * y[i];
    }
    r[k] = sum;
    y[0] += -lambda * y[1];
    for (i = 1; i < l - 1; i++)
      y[i] += lambda * (y[i - 1] - y[i + 1]);
    y[i] += lambda * y[i - 1];
  }
}

/**
 * <p>Mel-LPC parameter estimation via Levinson-Durbin recursion.</p>
 *
 * @param x
 *          Pointer to signal samples to be analysed.
 * @param flng
 *          Amount of signal samples stored in <CODE>samples</CODE>.
 * @param a
 *          Pointer to resulting LPC coefficients <CODE>a</CODE>(k) (k = 0 ... p - 1).
 * @param m
 *          Order of LPC analysis.
 * @param lambda
 *          Warping factor. Zero means no warping.
 * @param scale
 *          Signal scaling factor.
 * @return <code>O_K</code> if successfull, a (negative) error code otherwise
 */
INT16 dlm_lpc_mlev(FLOAT64* x, INT32 flng, FLOAT64* a, INT32 m, FLOAT64 lambda, FLOAT64 scale) {
  FLOAT64 *r = NULL;
  extern int levdur(FLOAT64*,FLOAT64*,const INT32,FLOAT64);

  if (lambda == 0) return dlm_lpc_lev(x, flng, a, m, scale);
  if ((!x) || (!a)) return NOT_EXEC;
  if ((m <= 0) || (flng < 1)) return NOT_EXEC;

  r = (FLOAT64*) dlp_calloc(m, sizeof(FLOAT64));

  if (!r) return ERR_MEM;

  macorr(x, flng, r, m, lambda);
  levdur(r, a, m - 1, -1);

  *a = sqrt(*a) * scale;

  dlp_free(r);
  return O_K;
}

/**
 * <p>LPC parameter estimation via Levinson-Durbin recursion.</p>
 *
 * @param x
 *          Pointer to signal samples to be analysed.
 * @param flng
 *          Amount of signal samples stored in <CODE>samples</CODE>.
 * @param a
 *          Pointer to resulting LPC coefficients <CODE>a<sub>k</sub>(k = 0 ... p - 1)</CODE>.
 * @param m
 *          Order of LPC analysis.
 * @param scale
 *          Signal scaling factor.
 * @return <code>O_K</code> if successfull, a (negative) error code otherwise
 */
INT16 dlm_lpc_lev(FLOAT64* x, INT32 flng, FLOAT64* a, INT32 m, FLOAT64 scale) {
  INT16 flag = 0;
  extern int lpc(FLOAT64*,const INT32,FLOAT64*,const INT32,const FLOAT64);

  if ((!x) || (!a)) return NOT_EXEC;
  if ((m <= 0) || (flng < 1)) return NOT_EXEC;

  flag = lpc(x, flng, a, m - 1, 0.000001);

  *a = sqrt(*a) * scale;

  return (flag == 0) ? O_K : NOT_EXEC;
}

/**
 * <p>Convert the PARCOR coefficients to LPC coefficients.</p>
 *
 * @param k
 *          Pointer to PARCOR coefficients.
 * @param a
 *          Pointer to resulting LPC coefficients.
 * @param p
 *          Order of LPC and PARCOR.
 * @return <code>O_K</code> if successfull, a (negative) error code otherwise
 */
INT16 dlm_parcor2lpc(FLOAT64 *k, FLOAT64 *a, INT16 p) {
  extern void par2lpc(FLOAT64*,FLOAT64*,const int);

  par2lpc(k, a, p - 1);
  return O_K;
}

/**
 * <p>Convert the mel-cepstrum coefficients to LPC coefficients.</p>
 *
 * @param mc
 *          Pointer to mel-cepstrum coefficients.
 * @param mc_len
 *          Order of mel-cepstrum.
 * @param a
 *          Pointer to resulting LPC coefficients.
 * @param a_len
 *          Order of LPC.
 * @param lambda
 *          Warping factor.
 * @return <code>O_K</code> if successfull, a (negative) error code otherwise
 */
INT16 dlm_mcep2lpc(FLOAT64* mc, INT16 mc_len, FLOAT64* a, INT16 a_len, FLOAT64 lambda) {
  INT16 m = 0;
  INT16 i = 0;
  INT16 c_len = mc_len;
  FLOAT64 sum = 0.0;
  FLOAT64* c = NULL;

  c = (FLOAT64*) dlp_calloc(c_len, sizeof(FLOAT64));

  dlm_mcep2cep(mc, mc_len, c, c_len, lambda, NULL);

  a[0] = exp(c[0]);

  for (m = 1; m < a_len; m++) {
    sum = 0.0;
    if (m < c_len) {
      for (i = 1; i < m; i++) {
        sum += (FLOAT64) (i) * a[m - i] * c[i];
      }
      a[m] = -c[m] - sum / (FLOAT64) m;
    } else {
      for (i = 1; i < c_len; i++) {
        sum += (FLOAT64) (i) * a[m - i] * c[i];
      }
      a[m] = -sum / (FLOAT64) m;
    }
  }

  dlp_free(c);
  return O_K;
}

