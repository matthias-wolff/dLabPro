/* dLabPro mathematics library
 * - LU Decomposition
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

/**
 * <p id="dlm_solve_lud">Solves the set of linear equations A X = B.</p>
 * Here A[0..n_a-1][0..n_a-1] is input but will be overwritten with permutated
 * LU decomposed matrix A, B[0..n_b-1][0..n_a-1] is input and output.
 *
 * @param A   Matrix A.
 * @param n_a Number of rows of A and B and columns of A.
 * @param B   Matrix B.
 * @param n_b Number of columns of B.
 */
INT16 dlm_solve_lud(FLOAT64* A, INT32 n_a, FLOAT64* B, INT32 n_b) {
  integer n = (integer)n_a;
  integer nrhs = (integer)n_b;
  integer info = 0;
  integer* ipiv = dlp_calloc(n, sizeof(integer));
  char trans[1] = { 'N' };

#ifdef __MAX_TYPE_32BIT
  extern int sgetrf_(integer*,integer*,real*,integer*,integer*,integer*);
  extern int sgetrs_(char*,integer*,integer*,real*,integer*,integer*,real*,integer*,integer*);
  sgetrf_(&n, &n, (real*)A, &n, ipiv, &info);
  sgetrs_(trans, &n, &nrhs, (real*)A, &n, ipiv, (real*)B, &n, &info);
#else
  extern int dgetrf_(integer*,integer*,doublereal*,integer*,integer*,integer*);
  extern int dgetrs_(char*,integer*,integer*,doublereal*,integer*,integer*,doublereal*,integer*,integer*);
  dgetrf_(&n, &n, A, &n, ipiv, &info);
  dgetrs_(trans, &n, &nrhs, (doublereal*)A, &n, ipiv, (doublereal*)B, &n, &info);
#endif

  dlp_free(ipiv);
  return O_K;
}

/*
 * Complex variant of {@link dlm_solve_lud}.
 */
INT16 dlm_solve_ludC(COMPLEX64* A, INT32 n_a, COMPLEX64* B, INT32 n_b) {
  integer n = (integer)n_a;
  integer nrhs = (integer)n_b;
  integer info = 0;
  integer* ipiv = dlp_calloc(n, sizeof(integer));
  char trans[1] = { 'N' };

#ifdef __MAX_TYPE_32BIT
  extern int cgetrf_(integer*,integer*,complex*,integer*,integer*,integer*);
  extern int cgetrs_(char*,integer*,integer*,complex*,integer*,integer*,complex*,integer*,integer*);
  cgetrf_(&n, &n, (complex*)A, &n, ipiv, &info);
  cgetrs_(trans, &n, &nrhs, (complex*)A, &n, ipiv, (complex*)B, &n, &info);
#else
  extern int zgetrf_(integer*,integer*,doublecomplex*,integer*,integer*,integer*);
  extern int zgetrs_(char*,integer*,integer*,doublecomplex*,integer*,integer*,doublecomplex*,integer*,integer*);
  zgetrf_(&n, &n, (doublecomplex*)A, &n, ipiv, &info);
  zgetrs_(trans, &n, &nrhs, (doublecomplex*)A, &n, ipiv, (doublecomplex*)B, &n, &info);
#endif

  dlp_free(ipiv);
  return O_K;
}

/**
 * <p id="dlm_det_lud">Calculates determinant using LU decomposition.</p>
 * The input square matrix <code>A</code> of size <code>nXA&times;nXa</code> will
 * be overwritten with permutated LU decomposition of <code>A</code>.
 *
 * @param A Input matrix A
 * @param nXA Dimension of A
 * @param d Pointer to determinant to calculate.
 */
INT16 dlm_det_lud(FLOAT64* A, INT32 nXA, FLOAT64* d) {
  integer n = (integer)nXA;
  integer info = 0;
  integer* ipiv = dlp_calloc(n, sizeof(integer));
#ifdef __MAX_TYPE_32BIT
  extern int sgetrf_(integer*,integer*,real*,integer*,integer*,integer*);
#else
  extern int dgetrf_(integer*,integer*,doublereal*,integer*,integer*,integer*);
#endif

  if(!ipiv) return ERR_MEM;
#ifdef __MAX_TYPE_32BIT
  sgetrf_(&n,&n,(real*)A,&n,ipiv,&info);
#else
  dgetrf_(&n,&n,(doublereal*)A,&n,ipiv,&info);
#endif
  if(d != NULL) *d = (info > 0) ? 0.0 : dlm_get_det_trf(A, nXA, ipiv);
  dlp_free(ipiv);
  return (info == 0) ? O_K : NOT_EXEC;
}

/*
 * Complex variant of {@link dlm_det_lud}.
 */
INT16 dlm_det_ludC(COMPLEX64* A, INT32 nXA, COMPLEX64* d) {
  integer n = (integer)nXA;
  integer info = 0;
  integer* ipiv = dlp_calloc(n, sizeof(integer));
#ifdef __MAX_TYPE_32BIT
  extern int cgetrf_(integer*,integer*,complex*,integer*,integer*,integer*);
#else
  extern int zgetrf_(integer*,integer*,doublecomplex*,integer*,integer*,integer*);
#endif

  if(!ipiv) return ERR_MEM;
#ifdef __MAX_TYPE_32BIT
  cgetrf_(&n,&n,(complex*)A,&n,ipiv,&info);
#else
  zgetrf_(&n,&n,(doublecomplex*)A,&n,ipiv,&info);
#endif
  if(d != NULL) *d = (info > 0) ? CMPLX(0.0) : dlm_get_det_trfC(A, nXA, ipiv);
  dlp_free(ipiv);
  return (info == 0) ? O_K : NOT_EXEC;
}
