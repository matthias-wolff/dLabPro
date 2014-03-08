/* dLabPro mathematics library
 * - Root functions
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
#include "f2c.h"

#define SIGN(a,b) ((b) >= 0.0 ? fabs(a) : -fabs(a))

INT16 CGEN_IGNORE dlm_pascal(FLOAT64* p, INT16 n_order) {
  INT16 i_order1;
  INT16 i_order2;
  INT16 i_order3;
  FLOAT64* pp = NULL;

  if ((!p) || (n_order < 1)) return NOT_EXEC;

  pp = (FLOAT64*) dlp_calloc(n_order*n_order, sizeof(FLOAT64));

  for (i_order1 = 0; i_order1 < n_order; i_order1++) {
    pp[i_order1 * n_order] = 1.0;
    pp[i_order1 * n_order + i_order1] = (i_order1 % 2) ? -1 : 1;
  }

  for (i_order1 = 1; i_order1 < n_order - 1; i_order1++) {
    for (i_order2 = i_order1 + 1; i_order2 < n_order; i_order2++) {
      pp[i_order2 * n_order + i_order1] =
          pp[(i_order2 - 1) * n_order + i_order1] - pp[(i_order2 - 1) * n_order + i_order1 - 1];
    }
  }
  for (i_order1 = 0; i_order1 < n_order; i_order1++) {
    for (i_order2 = i_order1; i_order2 < n_order; i_order2++) {
      p[i_order1 * n_order + i_order2] = 0.0;
      for (i_order3 = 0; i_order3 < n_order; i_order3++) {
        p[i_order1 * n_order + i_order2] += pp[i_order1 * n_order + i_order3] * pp[i_order2 * n_order + i_order3];
      }
    }
  }
  for (i_order1 = 0; i_order1 < n_order; i_order1++) {
    for (i_order2 = 0; i_order2 < i_order1; i_order2++) {
      p[i_order1 * n_order + i_order2] = p[i_order2 * n_order + i_order1];
    }
  }
  dlp_free(pp);
  return O_K;
}

/**
 * <p>Converts a coefficient vector of a polynomial in z: a(1)z^0+a(2)z^1+...
 * to a coefficient vector of a polynomial in s: b(1)s^0+b(2)s^1+...
 * by a bilinear map z:= (s+1)/(s-1)
 * which converts zeros of z inside the unit circle
 * into zeros of s in the left halfplane,
 * i.e. prepares a polynomial for the Routh/Hurwitz-test.</p>
 *
 * @param poly
 *          Pointer to polynomial coefficients.
 * @param n_order
 *          Number of polynomial coefficients stored in  <CODE>poly</CODE>.
 * @return <code>O_K</code> if successful, a (negative) error code otherwise
 */
INT16 dlm_z2s(FLOAT64* poly, INT16 n_order) {
  INT16 i_order1;
  INT16 i_order2;
  INT32 n_order2 = n_order * n_order;
  FLOAT64* pp = NULL;
  FLOAT64* pm = NULL;
  FLOAT64* ppd = NULL;
  FLOAT64* pmd = NULL;
  FLOAT64* poly_z = NULL;
  FLOAT64* poly_s = NULL;

  pp = (FLOAT64*) dlp_calloc(2*n_order2+4*n_order, sizeof(FLOAT64));
  pm = pp + n_order2;
  ppd = pm + n_order2;
  pmd = ppd + n_order;
  poly_z = pmd + n_order;
  poly_s = poly_z + n_order;

  if (dlm_pascal(pp, n_order) != O_K) return NOT_EXEC;

  dlp_memmove(pm, pp, n_order2 * sizeof(FLOAT64));

  for (i_order1 = 1; i_order1 < n_order; i_order1 += 2) {
    for (i_order2 = 0; i_order2 < n_order; i_order2++) {
      pm[i_order2 * n_order + i_order1] = -pp[i_order2 * n_order + i_order1];
    }
  }

  for (i_order1 = 0; i_order1 < n_order; i_order1++) {
    for (i_order2 = 0; i_order2 < (n_order - i_order1); i_order2++) {
      pmd[i_order2] = pm[i_order2 * n_order + n_order - i_order1 - 1 - i_order2];
    }
    for (i_order2 = n_order - i_order1; i_order2 < n_order; i_order2++) {
      pmd[i_order2] = 0.0;
    }
    for (i_order2 = 0; i_order2 <= i_order1; i_order2++) {
      ppd[i_order2] = pp[i_order2 * n_order + i_order1 - i_order2];
    }
    for (i_order2 = i_order1 + 1; i_order2 < n_order; i_order2++) {
      ppd[i_order2] = 0.0;
    }
    dlm_filter_fir(ppd, n_order, pmd, poly_z, n_order, NULL, 0);
    for (i_order2 = 0; i_order2 < n_order; i_order2++) {
      poly_s[i_order2] += poly_z[i_order2] * poly[n_order - 1 - i_order1];
    }
  }
  dlp_memmove(poly, poly_s, n_order * sizeof(FLOAT64));
  dlp_free(pp);
  return O_K;
}

INT16 dlm_routh(FLOAT64* poly, INT16 order) {
  INT16 i;
  INT16 j;
  INT16 n = (order + 1) / 2 * 2 + 2;
  FLOAT64* b1 = NULL;
  FLOAT64* b2 = NULL;
  FLOAT64* a = NULL;

  a = (FLOAT64*) dlp_calloc(order+n+n, sizeof(FLOAT64));
  b1 = a + order;
  b2 = b1 + n;

  for (i = order - 1; i >= 0; i--)
    a[i] = poly[i] / poly[0];

  dlm_z2s(a, order);

  for (i = order - 1; i >= 0; i--) {
    b1[n - order + i] = a[order - i - 1];
    b2[i] = 0.0;
  }
  for (; n - order + i >= 0; i--) {
    b1[n - order + i] = 0.0;
    b2[i] = 0.0;
  }

  for (j = n - 3; j > 2; j -= 2) {
    for (i = 0; i < n - 3; i += 2)
      b2[n - 1 - i] = b1[n - 3 - i] - b1[n - 1] * b1[n - 4 - i] / b1[n - 2];
    for (i = 1; i < n - 3; i += 2)
      b2[n - 1 - i] = b1[n - 3 - i] - b1[n - 2] * b2[n - 2 - i] / b2[n - 1];
    if ((b2[n - 1] < 0.0) || (b2[n - 2] < 0.0)) {
      dlp_free(a);
      return DLM_ROOTS_UNSTABLE;
    }
    memcpy(b1, b2, n * sizeof(FLOAT64));
  }

  dlp_free(a);

  return DLM_ROOTS_STABLE;
}

/**
 * <p>Calculates roots of a polynomial.</p>
 *
 * @param a
 *          Pointer to polynomial coefficients.
 * @param m
 *          Number of polynomial coefficients stored in  <CODE>a</CODE>.
 * @param rtr
 *          Resulting real roots.
 * @param rti
 *          Resulting imaginary roots.
 * @return <code>O_K</code> if successful, a (negative) error code otherwise
 */
INT16 dlm_roots(FLOAT64* A, COMPLEX64* Z, INT16 nA) {
  INT16 j, k;
  integer m = nA;
  integer ilo;
  integer ihi;
  integer info;
  integer c__1 = 1;
  integer lwork = 0;
  char job1[1] = { 'S' };
  char job2[1] = { 'E' };
  char compz[1] = { 'N' };

#ifdef __MAX_TYPE_32BIT
  int sgebal_(char*,integer*,real*,integer*,integer*,integer*,real*,integer*);
  int shseqr_(char*,char*,integer*,integer*,integer*,real*,integer*,real*,real*,real*,integer*,real*,integer*,integer*);
#else
  int dgebal_(char*,integer*,doublereal*,integer*,integer*,integer*,doublereal*,integer*);
  int dhseqr_(char*,char*,integer*,integer*,integer*,doublereal*,integer*,doublereal*,doublereal*,doublereal*,integer*,doublereal*,integer*,integer*);
#endif

#define _H(i,j) *(hess+(j)+(i)*m)

  if ((!A) || (!Z)) return NOT_EXEC;

  k = 0;
  while ((k < m) && (*(A + k) == 0.0)) {
    k++;
    A++;
  }
  m -= k;
  m--;
  if (m <= 0) return NOT_EXEC;

#ifdef __MAX_TYPE_32BIT
  real* hess = (real*) dlp_malloc((4*m+m*m)*sizeof(real));
  if (!hess) return ERR_MEM;
  real* scale = hess + m * m;
  real* wr = scale + m;
  real* wi = wr + m;
  real* work = wi + m;
#else
  doublereal* hess=(doublereal*)dlp_malloc((4*m+m*m)*sizeof(doublereal));
  if(!hess) return ERR_MEM;
  doublereal* scale = hess + m*m;
  doublereal* wr = scale + m;
  doublereal* wi = wr + m;
  doublereal* work = wi + m;
#endif
  lwork = m;

  for (k = 0; k < m; k++) {
    _H(m-1,k) = -A[k + 1] / A[0];
    for (j = 0; j < m - 1; j++)
      _H(j,k) = ((j == (k - 1)) && (k > 0)) ? 1.0 : 0.0;
  }
#ifdef __MAX_TYPE_32BIT
  sgebal_(job1, &m, hess, &m, &ilo, &ihi, scale, &info);
  shseqr_(job2, compz, &m, &ilo, &ihi, hess, &m, wr, wi, NULL, &c__1, work, &lwork, &info);
#else
  dgebal_(job1, &m, hess, &m, &ilo, &ihi, scale, &info);
  dhseqr_(job2, compz, &m, &ilo, &ihi, hess, &m, wr, wi, NULL, &c__1, work, &lwork, &info);
#endif
  dlp_free(hess);
  for (j = 0; j < m; j++) {
    Z[j].x = wr[j];
    Z[j].y = wi[j];
  }
  return (info == 0) ? O_K : NOT_EXEC;
#undef _H
}

/**
 * <p>Calculates roots by tracking from known roots using homotopy method by
 * Alexander, Stonick, 1993, ICASSP - Fast Adaptive Polynomial Root Tracking
 * Using A Homotopy Continuation Method.</p>
 *
 * @param poly1
 *          Polynomial of predecessor
 * @param rtr1
 *          Real part of roots of predecessor.
 * @param rti1
 *          Imaginary part of roots of predecessor.
 * @param poly2
 *          Polynomial of successor where the roots are calculated from.
 * @param rtr2
 *          Real part of roots of successor to be calculated.
 * @param rti2
 *          Imaginary part of roots of successor to be calculated.
 * @param m
 *          Number of roots, i.e. size of <CODE>rtr[12]</CODE> and <CODE>rti[12]</CODE>.
 * @param n_step
 *          Number of interim points on the path from old to new roots
 * @param n_iter
 *          Maximum number of iterations of newtons method per interim point
 * @param eps
 *          Maximum error used for breaking condition at newtons method
 * @return <code>O_K</code> if successful, a (negative) error code otherwise
 */
INT16 dlm_roots_track_homotopy(FLOAT64 poly1[], FLOAT64 rtr1[], FLOAT64 rti1[], FLOAT64 poly2[], FLOAT64 rtr2[], FLOAT64 rti2[], INT16 m, INT16 n_step, INT16 n_iter, FLOAT64 eps) {
  INT16 i_step = 0;
  INT16 i_iter = 0;
  INT16 i = 0;
  INT16 j = 0;
  FLOAT64 pos = 0.0;
  FLOAT64 F1[2] = { 0.0, 0.0 };
  FLOAT64 F2[2] = { 0.0, 0.0 };
  FLOAT64 H[2] = { 0.0, 0.0 };
  FLOAT64 dH[2] = { 0.0, 0.0 };
  FLOAT64 delta[2] = { 0.0, 0.0 };
  FLOAT64* dP1 = NULL;
  FLOAT64* dP2 = NULL;
  FLOAT64 tmp = 0.0;
  FLOAT64* trackLen = NULL;
  FLOAT64 alpha = 0.3;

  if (!poly1 || !poly2 || !rtr1 || !rti1 || !rtr2 || !rti2) return NOT_EXEC;

  dP1 = (FLOAT64*) dlp_calloc(3*m, sizeof(FLOAT64));
  dP2 = dP1 + m;
  trackLen = dP2 + m;

  for (i = 0; i < m; i++) {
    rtr2[i] = sqrt(rtr1[i] * rtr1[i] + rti1[i] * rti1[i]);
    rti2[i] = atan2(rti1[i], rtr1[i]);
    dP1[i] = poly1[i] * (FLOAT64) (m - i);
    dP2[i] = poly2[i] * (FLOAT64) (m - i);
  }

  for (i_step = 1; i_step <= n_step; i_step++) {
    pos = (FLOAT64) i_step / (FLOAT64) n_step;
    for (i = 0; i < m; i++) {
      for (j = m, F1[0] = 0.0, F1[1] = 0.0, F2[0] = 0.0, F2[1] = 0.0; j >= 0; j--) {
        tmp = dlm_pow(rtr2[i], j);
        F1[0] += poly1[m - j] * tmp * cos(rti2[i] * (FLOAT64) j);
        F1[1] += poly1[m - j] * tmp * sin(rti2[i] * (FLOAT64) j);
        F2[0] += poly2[m - j] * tmp * cos(rti2[i] * (FLOAT64) j);
        F2[1] += poly2[m - j] * tmp * sin(rti2[i] * (FLOAT64) j);
      }
      H[0] = (1.0 - pos) * F1[0] + pos * F2[0];
      H[1] = (1.0 - pos) * F1[1] + pos * F2[1];
      tmp = H[0];
      H[0] = sqrt(H[0] * H[0] + H[1] * H[1]);
      H[1] = atan2(H[1], tmp);
      i_iter = 0;
      while ((H[0] > eps) && (i_iter < n_iter)) {
        for (j = m, F1[0] = 0.0, F1[1] = 0.0, F2[0] = 0.0, F2[1] = 0.0; j > 0; j--) {
          tmp = dlm_pow(rtr2[i], j - 1);
          F1[0] += dP1[m - j] * tmp * cos(rti2[i] * (FLOAT64) (j - 1));
          F1[1] += dP1[m - j] * tmp * sin(rti2[i] * (FLOAT64) (j - 1));
          F2[0] += dP2[m - j] * tmp * cos(rti2[i] * (FLOAT64) (j - 1));
          F2[1] += dP2[m - j] * tmp * sin(rti2[i] * (FLOAT64) (j - 1));
        }
        dH[0] = (1.0 - pos) * F1[0] + pos * F2[0];
        dH[1] = (1.0 - pos) * F1[1] + pos * F2[1];
        tmp = dH[0];
        dH[0] = sqrt(dH[0] * dH[0] + dH[1] * dH[1]);
        dH[1] = atan2(dH[1], tmp);

        delta[0] = H[0] / dH[0];
        delta[1] = H[1] - dH[1];

        trackLen[i] += delta[0];

        tmp = rtr2[i];
        rtr2[i] = rtr2[i] * cos(rti2[i]);
        rti2[i] = tmp * sin(rti2[i]);

        rtr2[i] -= alpha * delta[0] * cos(delta[1]);
        rti2[i] -= alpha * delta[0] * sin(delta[1]);
        tmp = rtr2[i];
        rtr2[i] = sqrt(rtr2[i] * rtr2[i] + rti2[i] * rti2[i]);
        rti2[i] = atan2(rti2[i], tmp);

        for (j = m, F1[0] = 0.0, F1[1] = 0.0, F2[0] = 0.0, F2[1] = 0.0; j >= 0; j--) {
          tmp = dlm_pow(rtr2[i], j);
          F1[0] += poly1[m - j] * tmp * cos(rti2[i] * (FLOAT64) j);
          F1[1] += poly1[m - j] * tmp * sin(rti2[i] * (FLOAT64) j);
          F2[0] += poly2[m - j] * tmp * cos(rti2[i] * (FLOAT64) j);
          F2[1] += poly2[m - j] * tmp * sin(rti2[i] * (FLOAT64) j);
        }
        H[0] = (1.0 - pos) * F1[0] + pos * F2[0];
        H[1] = (1.0 - pos) * F1[1] + pos * F2[1];
        tmp = H[0];
        H[0] = sqrt(H[0] * H[0] + H[1] * H[1]);
        H[1] = atan2(H[1], tmp);

        i_iter++;
      }
      if (i_iter == n_iter) {
        dlp_free(dP1);
        return NOT_EXEC;
      }
    }
  }

  for (i = 0; i < m; i++) {
    tmp = rtr2[i];
    rtr2[i] = rtr2[i] * cos(rti2[i]);
    rti2[i] = tmp * sin(rti2[i]);
  }

  for (tmp = trackLen[0], i = 1; i < m; i++)
    tmp = MAX(tmp,trackLen[i]);

  dlp_free(dP1);

  if ((tmp * alpha) > 1.0) {
    return NOT_EXEC;
  }

  return O_K;
}

/**
 * <p>Calculates roots by tracking from known roots using coefficient matching
 * method by Starer, Nehorai, 1991, Transactions - Adaptive Polynomial Factorization
 * By Coefficient Matching.</p>
 *
 * @param poly1
 *          Polynomial of predecessor
 * @param rtr1
 *          Real part of roots of predecessor.
 * @param rti1
 *          Imaginary part of roots of predecessor.
 * @param poly2
 *          Polynomial of successor where the roots are calculated from.
 * @param rtr2
 *          Real part of roots of successor to be calculated.
 * @param rti2
 *          Imaginary part of roots of successor to be calculated.
 * @param m
 *          Number of roots, i.e. size of <CODE>rtr[12]</CODE> and <CODE>rti[12]</CODE>.
 * @param n_iter
 *          Maximum number of iterations of newtons method per interim point
 * @param eps
 *          Maximum error used for breaking condition at newtons method
 * @return <code>O_K</code> if successful, a (negative) error code otherwise
 */
FLOAT64 dlm_roots_track_match_poly(COMPLEX64 poly1[], COMPLEX64 rt1[], COMPLEX64 poly2[], COMPLEX64 rt2[], INT16 m, INT16 n_iter, FLOAT64 eps) {
  INT16 i = 0;
  INT16 j = 0;
  INT16 i_iter = 0;
  FLOAT64 alpha = 1.0;
  FLOAT64 e0 = 0.0;
  FLOAT64 e1 = 0.0;
  FLOAT64 e2 = 0.0;
  COMPLEX64* poly_c = NULL;
  COMPLEX64* T = NULL;
  COMPLEX64* L = NULL;
  COMPLEX64* E = NULL;
  COMPLEX64* D = NULL;
  COMPLEX64* P1 = NULL;
  COMPLEX64* P2 = NULL;

  if (!poly1 || !poly2 || !rt1 || !rt2) return NOT_EXEC;

  poly_c = (COMPLEX64*) dlp_calloc(m+1, sizeof(COMPLEX64));
  P1 = (COMPLEX64*) dlp_calloc(m, sizeof(COMPLEX64));
  P2 = (COMPLEX64*) dlp_calloc(m, sizeof(COMPLEX64));
  E = (COMPLEX64*) dlp_calloc(m, sizeof(COMPLEX64));
  D = (COMPLEX64*) dlp_calloc(m, sizeof(COMPLEX64));
  T = (COMPLEX64*) dlp_calloc(m*m, sizeof(COMPLEX64));
  L = (COMPLEX64*) dlp_calloc(m*m, sizeof(COMPLEX64));

  dlp_memmove(rt2, rt1, m * sizeof(COMPLEX64));
  for (i = 0; i <= m; i++) {
    poly_c[i] = dlp_scalopC(poly1[i], poly1[0], OP_DIV);
  }

  for (e1 = 0.0, i = 0; i < m; i++) {
    P1[i] = rt2[i];
    E[i] =
        dlp_scalopC(dlp_scalopC(poly2[i + 1], poly2[0], OP_DIV), dlp_scalopC(poly1[i + 1], poly1[0], OP_DIV), OP_DIFF);
    e0 += E[i].x * E[i].x;
  }

  e1 = e0;

  while ((i_iter < n_iter) && (sqrt(e1 / (FLOAT64) m) > eps)) {

    for (i = 0; i < m; i++) {
      for (j = 0; j <= i; j++) {
        (*(T + (i) * m + j)).x = poly_c[i - j].x;
        (*(T + (i) * m + j)).y = poly_c[i - j].y;
      }
    }
    for (j = 0; j < m; j++) {
      (*(L + (0) * m + j)).x = 1.0;
      (*(L + (0) * m + j)).y = 0.0;
      (*(L + (1) * m + j)).x = P1[j].x;
      (*(L + (1) * m + j)).y = P1[j].y;
    }
    for (i = 2; i < m; i++) {
      for (j = 0; j < m; j++) {
        (*(L + (i) * m + j)).x = (*(L + (i - 1) * m + j)).x * P1[j].x - (*(L + (i - 1) * m + j)).y * P1[j].y;
        (*(L + (i) * m + j)).y = (*(L + (i - 1) * m + j)).x * P1[j].y + (*(L + (i - 1) * m + j)).y * P1[j].x;
      }
    }

    dlm_solve_ludC(T, m, E, 1);
    dlm_solve_ludC(L, m, E, 1);

    dlp_memmove(D, E, m * sizeof(COMPLEX64));

    alpha = 1.0;

    do {
      for (i = 0; i < m; i++) {
        P2[i].x = P1[i].x - alpha * D[i].x;
        P2[i].y = P1[i].y - alpha * D[i].y;
      }

      dlm_polyC(P2, m, poly_c);

      for (e2 = 0.0, i = 0; i < m; i++) {
        E[i].x = dlp_scalopC(poly2[i + 1], poly2[0], OP_DIV).x - poly_c[i + 1].x;
        E[i].y = -poly_c[i + 1].y;
        e2 += E[i].x * E[i].x + E[i].y * E[i].y;
      }
      alpha /= 2.0;
    } while ((alpha > 1.0e-10) && (e2 / e1 > 1.0));
    dlp_memmove(P1, P2, m * sizeof(COMPLEX64));
    e1 = e2;
    i_iter++;
  }

  for (i = 0; i < m; i++) {
    rt2[i] = P1[i];
  }

  dlp_free(D);
  dlp_free(E);
  dlp_free(P1);
  dlp_free(P2);
  dlp_free(T);
  dlp_free(L);
  dlp_free(poly_c);

  return e2 / e0;
}

/**
 * <p>Calculates polynomial from roots.</p>
 *
 * @param rtr
 *          Real roots.
 * @param rti
 *          Imaginary roots.
 * @param m
 *          Number of roots stored in  <CODE>rtr</CODE> and <CODE>rti</CODE>.
 * @param ar
 *          Pointer to resulting real polynomial coefficients.
 * @param ai
 *          Pointer to resulting imaginary polynomial coefficients (or <code>NULL</code>).
 * @return <code>O_K</code> if successful, a (negative) error code otherwise
 */
INT16 dlm_poly(FLOAT64 rtr[], FLOAT64 rti[], INT16 m, FLOAT64 ar[], FLOAT64 ai[]) {
  INT16 i = 0;
  INT16 j = 0;
  FLOAT64* arp = NULL;
  FLOAT64* aip = NULL;

  if ((!ar) || (!rtr) || (!rti)) return NOT_EXEC;

  if (ai == NULL) {
    aip = (FLOAT64*) dlp_calloc(m+1, sizeof(FLOAT64));
    if (!aip) return ERR_MEM;
    arp = ar;
  } else {
    aip = ai;
    arp = ar;
  }

  dlp_memset(arp, 0L, (m + 1) * sizeof(FLOAT64));
  dlp_memset(aip, 0L, (m + 1) * sizeof(FLOAT64));
  arp[0] = 1.0;

  for (i = 0; i < m; i++) {
    for (j = i; j >= 0; j--) {
      arp[j + 1] = arp[j + 1] - rtr[i] * arp[j] + rti[i] * aip[j];
      aip[j + 1] = aip[j + 1] - rti[i] * arp[j] - rtr[i] * aip[j];
    }
  }

  if (ai == NULL) dlp_free(aip);

  return O_K;
}

/**
 * <p>Calculates polynomial from roots.</p>
 *
 * @param rt
 *          Pointer to roots.
 * @param m
 *          Number of roots stored in  <CODE>rt</CODE>.
 * @param a
 *          Pointer to resulting complex polynomial coefficients.
 * @return <code>O_K</code> if successful, a (negative) error code otherwise
 */
INT16 dlm_polyC(COMPLEX64* rt, INT16 m, COMPLEX64* a) {
  INT16 i = 0;
  INT16 j = 0;

  if ((!a) || (!rt)) return NOT_EXEC;

  dlp_memset(a, 0L, (m + 1) * sizeof(COMPLEX64));
  a[0].x = 1.0;

  for (i = 0; i < m; i++) {
    for (j = i; j >= 0; j--) {
      a[j + 1].x = a[j + 1].x - rt[i].x * a[j].x + rt[i].y * a[j].y;
      a[j + 1].y = a[j + 1].y - rt[i].y * a[j].x - rt[i].x * a[j].y;
    }
  }
  return O_K;
}

/**
 * <p>Stabilises a polynomial. I.e. mirroring all roots located outside the unit circle into the unit circle.</p>
 *
 * @param poly
 *          Pointer to polynomial coefficients.
 * @param n_poly
 *          Number of polynomial coefficients stored in  <CODE>poly</CODE>.
 * @return  Number of mirrored roots.
 */
INT32 dlm_stabilise(FLOAT64* poly, INT32 n_poly) {
  COMPLEX64* roots = NULL;
  FLOAT64* a1 = NULL;
  FLOAT64* a2 = NULL;
  FLOAT64 a0;
  FLOAT64 r;
  FLOAT64 rr;
  INT16 is_stable;
  INT32 i_poly1;
  INT32 i_poly2;
  INT32 i_poly3;
  INT32 n_roots_stabilised = 0;

  roots = (COMPLEX64*) dlp_calloc(n_poly-1, sizeof(COMPLEX64));
  a1 = (FLOAT64*) dlp_calloc(2*n_poly, sizeof(FLOAT64));
  a2 = a1 + n_poly;

  a0 = poly[0];

  dlm_roots(poly, roots, n_poly);
  is_stable = 1;
  rr = 1.0;
  for (i_poly1 = 0; i_poly1 < (n_poly - 1); i_poly1++) {
    r = roots[i_poly1].x * roots[i_poly1].x + roots[i_poly1].y * roots[i_poly1].y;
    if (r > 1.0) {
      n_roots_stabilised++;
      is_stable = 0;
      roots[i_poly1].x /= r;
      roots[i_poly1].y /= r;
      rr = rr * sqrt(r);
    }
  }
  if (is_stable == 0) {
    a1[0] = 1.0;
    memset(a1 + 1, 0L, (n_poly - 1) * sizeof(FLOAT64));
    memset(a2, 0L, n_poly * sizeof(FLOAT64));
    for (i_poly2 = 0; i_poly2 < (n_poly - 1); i_poly2++) {
      for (i_poly3 = i_poly2 + 1; i_poly3 > 0; i_poly3--) {
        a1[i_poly3] -= roots[i_poly2].x * a1[i_poly3 - 1] - roots[i_poly2].y * a2[i_poly3 - 1];
        a2[i_poly3] -= roots[i_poly2].x * a2[i_poly3 - 1] + roots[i_poly2].y * a1[i_poly3 - 1];
      }
    }
    for (i_poly2 = 1; i_poly2 < n_poly; i_poly2++) {
      poly[i_poly2] = a1[i_poly2];
    }
    *poly = a0 * rr;
  }

  dlp_free(roots);
  dlp_free(a1);

  return n_roots_stabilised;
}

/**
 * <p>Converts roots given in cartesian form (x+iy) to polar form (re<sub>p</sub>).</p>
 *
 * @param x
 *          Pointer to real part of input and modulus of output.
 * @param y
 *          Pointer to imaginary part of input and phase of output.
 * @param n
 *          Number of roots stored in  <CODE>rtr</CODE> and <CODE>rti</CODE>.
 * @return <code>O_K</code> if successful, a (negative) error code otherwise
 */
INT32 dlm_roots_cart2pol(FLOAT64* x, FLOAT64* y, INT32 n) {
  INT32 i = 0;
  FLOAT64 tmp = 0.0;

  if (!x || !y) return NOT_EXEC;

  for (i = 0; i < n; i++) {
    tmp = x[i];
    x[i] = sqrt(tmp * tmp + y[i] * y[i]);
    y[i] = atan2(y[i], tmp);
  }

  return O_K;
}

/**
 * <p>Converts roots given in cartesian form (re<sub>p</sub>) to polar form (x+jy).</p>
 *
 * @param r
 *          Pointer to real part of input and modulus of output.
 * @param p
 *          Pointer to imaginary part of input and phase of output.
 * @param n
 *          Number of roots stored in  <CODE>rtr</CODE> and <CODE>rti</CODE>.
 * @return <code>O_K</code> if successful, a (negative) error code otherwise
 */
INT32 dlm_roots_pol2cart(FLOAT64* r, FLOAT64* p, INT32 n) {
  INT32 i = 0;
  FLOAT64 tmp = 0.0;

  if (!r || !p) return NOT_EXEC;

  for (i = 0; i < n; i++) {
    tmp = r[i];
    r[i] = tmp * cos(p[i]);
    p[i] = tmp * sin(p[i]);
  }

  return O_K;
}
