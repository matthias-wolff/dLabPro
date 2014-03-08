/* dLabPro mathematics library
 * - FIR/IIR Filter implementation
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

INT16 dlm_filter_freqt_fir(FLOAT64* input, INT32 n_in, FLOAT64* output, INT32 n_out, FLOAT64 lambda) {
  INT32 k = 0;
  INT32 l = 0;
  FLOAT64* z = (FLOAT64*) dlp_calloc(n_in+1, sizeof(FLOAT64));
  FLOAT64* tmp;

  if (input == output) {
    tmp = (FLOAT64*) dlp_calloc(n_out, sizeof(FLOAT64));
  } else {
    tmp = output;
  }
  *z = 1.0;

  for (k = 0; k < n_out; k++) {
    for (l = 1; l < n_in; l++) {
      z[l] = z[l] + lambda * (z[l - 1] - z[l + 1]);
    }
    for (l = 0, tmp[k] = 0; l < n_in; l++) {
      tmp[k] += input[l] * z[l];
    }
    dlp_memmove(z + 1, z, n_in * sizeof(FLOAT64));
    *z = 0.0;
  }

  if (input == output) {
    dlp_memmove(output, tmp, n_out * sizeof(FLOAT64));
    dlp_free(tmp);
  }
  dlp_free(z);
  return O_K;
}

INT16 dlm_freqt(FLOAT64* c1, INT32 m1, FLOAT64* c2, INT32 m2, FLOAT64 lambda) {
  extern void freqt(FLOAT64*,const INT32,FLOAT64*,const INT32,const FLOAT64);

  freqt(c1, m1, c2, m2, lambda);
  return O_K;
}

INT16 dlm_freqtC(COMPLEX64* c1, INT32 m1, COMPLEX64* c2, INT32 m2, FLOAT64 lambda) {
  register INT32 i;
  register INT32 j;
  COMPLEX64* d;
  COMPLEX64* g;
  FLOAT64 lambda2 = 1.0 - lambda * lambda;

  d = (COMPLEX64*) dlp_calloc(m2+m2, sizeof(COMPLEX64));
  if (!d) return ERR_MEM;
  g = d + m2;

  for (i = -m1 + 1; i <= 0; i++) {
    if (0 < m2) {
      d[0] = g[0];
      g[0] = CMPLX_PLUS(c1[-i], CMPLX_MULT_R(g[0], lambda));
    }
    if (1 < m2) {
      d[1] = g[1];
      g[1] = CMPLX_PLUS(CMPLX_MULT_R(d[0],lambda2), CMPLX_MULT_R(g[1], lambda));
    }
    for (j = 2; j < m2; j++) {
      d[j] = g[j];
      g[j] = CMPLX_PLUS(d[j-1], CMPLX_MULT_R(CMPLX_MINUS(g[j], g[j-1]), lambda));
    }
  }

  dlp_memmove(c2, g, m2 * sizeof(COMPLEX64));

  dlp_free(d);
  return O_K;
}

/**
 * <p>Warped-filter implementation: Y(z) = X(z)B(z)/A(z). The filter coefficients
 * are intended to be warped. The filter coefficients were transformed by <code>dlm_mcep2b</code></p>
 * <img src="../resources/base/dlp_math/filter_m.gif" align="absmiddle">
 * @param b
 *          Pointer to filter coefficients <CODE>b</CODE>(k) (k=0..n_b-1).
 * @param n_b
 *          Number of filter coefficients <CODE>b</CODE>.
 * @param a
 *          Pointer to filter coefficients <CODE>a</CODE>(k) (k=0..n_a-1).
 * @param n_a
 *          Number of filter coefficients <CODE>a</CODE>.
 * @param input
 *          Pointer to input signal x.
 * @param output
 *          Pointer to output signal y.
 * @param n
 *          Length of input signal = length of output signal.
 * @param memory
 *          Pointer to filter states (can be <CODE>NULL</CODE>).
 * @param n_m
 *          Number of filter states (mostly: <CODE>n_m=n_b-1</CODE>).
 * @param lambda
 *          Warping factor.
 * @return <code>O_K</code> if successfull, a (negative) error code otherwise
 * @see    dlm_mcep2b
 */
INT16 dlm_filter_m(FLOAT64* b, INT32 n_b, FLOAT64* a, INT32 n_a, FLOAT64* input, FLOAT64* output, INT32 n, FLOAT64* memory, INT32 n_m, FLOAT64 lambda) {
  register INT32 k;
  register INT32 i;
  FLOAT64 b0 = 1;
  FLOAT64 a0 = 1;
  FLOAT64 m0 = 0;
  FLOAT64 oi = 0;
  FLOAT64* pb = NULL;
  FLOAT64* pa = NULL;
  INT16 is_m_free_needed = FALSE;
  INT16 is_b_free_needed = FALSE;
  INT16 is_a_free_needed = FALSE;
  FLOAT64 lambda2 = 1.0 - lambda * lambda;

  if (b == NULL) {
    n_b = 1;
    pb = &b0;
  } else {
    pb = (FLOAT64*) dlp_malloc(n_b*sizeof(FLOAT64));
    if (!pb) return ERR_MEM;
    dlp_memmove(pb, b, n_b * sizeof(FLOAT64));
    is_b_free_needed = TRUE;
  }
  if (a == NULL) {
    n_a = 1;
    pa = &a0;
  } else {
    pa = (FLOAT64*) dlp_malloc(n_a*sizeof(FLOAT64));
    if (!pa) {
      if (is_b_free_needed) dlp_free(pb);
      return ERR_MEM;
    }
    dlp_memmove(pa, a, n_a * sizeof(FLOAT64));
    is_a_free_needed = TRUE;
  }

  if (memory == NULL) {
    n_m = MAX(n_a,n_b) - 1;
    memory = (FLOAT64*) dlp_calloc(n_m, sizeof(FLOAT64));
    is_m_free_needed = TRUE;
  } else {
    if (n_m < (MAX(n_a,n_b) - 1)) return NOT_EXEC;
  }

  if (pb) dlm_mcep2b(pb, pb, n_b, lambda);
  if (pa) dlm_mcep2b(pa, pa, n_a, lambda);

  for (i = 0; i < n; i++) {
    if (lambda != 0.0) {
      memory[0] += lambda * memory[1];
      for (k = 1; k < (n_m - 1); k++)
        memory[k] += lambda * (memory[k + 1] - memory[k - 1]);
    }
    for (k = 1, m0 = 0; k < n_a; k++)
      m0 += pa[k] * memory[k - 1];
    for (k = 1, oi = 0; k < n_b; k++)
      oi += pb[k] * memory[k - 1];
    m0 = (input[i] + m0 * lambda2) / pa[0];
    output[i] = (oi * lambda2 + pb[0] * m0);
    if (n_m > 1) dlp_memmove(memory + 1, memory, (n_m - 1) * sizeof(FLOAT64));
    memory[0] = m0;
  }

  if (is_m_free_needed) dlp_free(memory);
  if (is_b_free_needed) dlp_free(pb);
  if (is_a_free_needed) dlp_free(pa);

  return O_K;
}

/**
 * <p>Warped-FIR-filter implementation: Y(z) = X(z)B(z). The filter coefficients
 * are intended to be warped. The filter coefficients were transformed by <code>dlm_mcep2b</code></p>.
 * <img src="../resources/base/dlp_math/filter_mfir.gif" align="absmiddle">
 * @param b
 *          Pointer to filter coefficients <CODE>b</CODE>(k) (k=0..n_b-1).
 * @param n_b
 *          Number of filter coefficients <CODE>b</CODE>.
 * @param input
 *          Pointer to input signal x.
 * @param output
 *          Pointer to output signal y.
 * @param n
 *          Length of input signal = length of output signal.
 * @param memory
 *          Pointer to filter states (can be <CODE>NULL</CODE>).
 * @param n_m
 *          Number of filter states (mostly: <CODE>n_m=n_b-1</CODE>).
 * @param lambda
 *          Warping factor.
 * @return <code>O_K</code> if successfull, a (negative) error code otherwise
 */
INT16 dlm_filter_mfir(FLOAT64* b, INT32 n_b, FLOAT64* input, FLOAT64* output, INT32 n, FLOAT64* memory, INT32 n_m, FLOAT64 lambda) {
  return dlm_filter_m(b, n_b, NULL, 0, input, output, n, memory, n_m, lambda);
}

/**
 * <p>Warped-IIR filter implementation: Y(z) = X(z)/A(z). The filter coefficients
 * are intended to be warped. The filter coefficients were transformed by <code>dlm_mcep2b</code></p>.
 * <img src="../resources/base/dlp_math/filter_miir.gif" align="absmiddle">
 * @param a
 *          Pointer to filter coefficients <CODE>a</CODE>(k) (k=0..n_b-1).
 * @param n_a
 *          Number of filter coefficients <CODE>a</CODE>.
 * @param input
 *          Pointer to input signal x.
 * @param output
 *          Pointer to output signal y.
 * @param n
 *          Length of input signal = length of output signal.
 * @param memory
 *          Pointer to filter states (can be <CODE>NULL</CODE>).
 * @param n_m
 *          Number of filter states (mostly: <CODE>n_m=MAX(n_a,n_b)-1</CODE>).
 * @return <code>O_K</code> if successfull, a (negative) error code otherwise
 */
INT16 dlm_filter_miir(FLOAT64* a, INT32 n_a, FLOAT64* input, FLOAT64* output, INT32 n, FLOAT64* memory, INT32 n_m, FLOAT64 lambda) {
  return dlm_filter_m(NULL, 0, a, n_a, input, output, n, memory, n_m, lambda);
}

/**
 * <p>IIR filter implementation: Y(z) = B(z)X(z)/A(z).</p>
 * <img src="../resources/base/dlp_math/filter.gif" align="absmiddle">
 *
 * @param b
 *          Pointer to filter coefficients <CODE>b</CODE>(k) (k=0..n_b-1).
 * @param n_b
 *          Number of filter coefficients <CODE>b</CODE>.
 * @param a
 *          Pointer to filter coefficients <CODE>a</CODE>(k) (k=0..n_b-1).
 * @param n_a
 *          Number of filter coefficients <CODE>a</CODE>.
 * @param input
 *          Pointer to input signal x.
 * @param output
 *          Pointer to output signal y.
 * @param n
 *          Length of input signal = length of output signal.
 * @param memory
 *          Pointer to filter states (can be <CODE>NULL</CODE>).
 * @param n_m
 *          Number of filter states (mostly: <CODE>n_m=MAX(n_a,n_b)-1</CODE>).
 * @return <code>O_K</code> if successfull, a (negative) error code otherwise
 */
INT16 dlm_filter(FLOAT64* b, INT32 n_b, FLOAT64* a, INT32 n_a, FLOAT64* input, FLOAT64* output, INT32 n, FLOAT64* memory, INT32 n_m) {
  register INT32 k;
  register INT32 i;
  FLOAT64 b0 = 1;
  FLOAT64 a0 = 1;
  FLOAT64 m0 = 0;
  FLOAT64 oi = 0;
  FLOAT64* pb = NULL;
  FLOAT64* pa = NULL;
  FLOAT64* pm = NULL;
  INT16 is_memfree_needed = FALSE;

  if (b == NULL) {
    n_b = 1;
    pb = &b0;
  } else {
    pb = b;
  }
  if (a == NULL) {
    n_a = 1;
    pa = &a0;
  } else {
    pa = a;
  }

  if (memory == NULL) {
    n_m = MAX(n_a,n_b) - 1;
    memory = (FLOAT64*) dlp_calloc(n_m, sizeof(FLOAT64));
    is_memfree_needed = TRUE;
  } else {
    if (n_m < (MAX(n_a,n_b) - 1)) return NOT_EXEC;
  }
  pm = memory - 1;

  for (i = 0; i < n; i++) {
    for (k = 1, m0 = 0; k < n_a; k++)
      m0 += pa[k] * pm[k];
    for (k = 1, oi = 0; k < n_b; k++)
      oi += pb[k] * pm[k];
    m0 = (m0 + input[i]) / pa[0];
    output[i] = (oi + pb[0] * m0);
    if (n_m > 1) dlp_memmove(memory + 1, memory, (n_m - 1) * sizeof(FLOAT64));
    memory[0] = m0;
  }

  if (is_memfree_needed) dlp_free(memory);

  return O_K;
}

/**
 * <p>FIR filter implementation: Y(z) = B(z)X(z).</p>
 * <img src="../resources/base/dlp_math/filter_fir.gif" align="absmiddle">
 *
 * @param b
 *          Pointer to filter coefficients <CODE>b</CODE>(k) (k=0..n_b-1).
 * @param n_b
 *          Number of filter coefficients <CODE>b</CODE>.
 * @param input
 *          Pointer to input signal x.
 * @param output
 *          Pointer to output signal y.
 * @param n
 *          Length of input signal = length of output signal.
 * @param memory
 *          Pointer to filter states (can be <CODE>NULL</CODE>).
 * @param n_m
 *          Number of filter states (mostly: <CODE>n_m=n_b-1</CODE>).
 * @return <code>O_K</code> if successfull, a (negative) error code otherwise
 */
INT16 dlm_filter_fir(FLOAT64* b, INT32 n_b, FLOAT64* input, FLOAT64* output, INT32 n, FLOAT64* memory, INT32 n_m) {
  return dlm_filter(b, n_b, NULL, 0, input, output, n, memory, n_m);
}

/**
 * <p>IIR filter implementation: Y(z) = X(z)/A(z).</p>
 * <img src="../resources/base/dlp_math/filter_iir.gif" align="absmiddle">
 *
 * @param a
 *          Pointer to filter coefficients <CODE>a</CODE>(k) (k=0..n_a-1).
 * @param n_a
 *          Number of filter coefficients <CODE>a</CODE>.
 * @param input
 *          Pointer to input signal x.
 * @param output
 *          Pointer to output signal y.
 * @param n
 *          Length of input signal = length of output signal.
 * @param memory
 *          Pointer to filter states (can be <CODE>NULL</CODE>).
 * @param n_m
 *          Number of filter states (mostly: <CODE>n_m=n_a-1</CODE>).
 * @return <code>O_K</code> if successfull, a (negative) error code otherwise
 */
INT16 dlm_filter_iir(FLOAT64* a, INT32 n_a, FLOAT64* input, FLOAT64* output, INT32 n, FLOAT64* memory, INT32 n_m) {
  return dlm_filter(NULL, 0, a, n_a, input, output, n, memory, n_m);
}
