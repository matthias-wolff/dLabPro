/* dLabPro mathematics library
 * - Eigenvector and eigenvalue computation
 *
 * AUTHOR : Matthias Wolff,
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
 * <p id="dlm_eigen_jac">Eigenvalue and eigenvector computation of symmetric matrices.<p>
 * 
 * @param A
 *          Pointer to input matrix. Must be square (<code>n</code> x
 *          <code>n</code>) and symmetric. Replaced in computation by diagonal
 *          matrix of eigenvectors (columns).
 * @param B
 *          Pointer to <code>n</code> x <code>n</code> matrix to be filled with
 *          the resultant eigenvalues
 * @param nXD
 *          Order of matrix (number of rows and columns)
 * @param bNorm
 *          If non-zero, normalize eigenvectors squareroot of their eigenvalues
 * @return <code>O_K</code> if successfull, a (negative) error code otherwise:
 *         </table>
 * @cgen:TODO: Function internally computes the transposed eigenvector matrix at
 *             the first place and transposes it back later. Optimize it!
 */
INT16 dlm_eigen_jac(FLOAT64* A, FLOAT64* B, INT32 nXD, INT16 bNorm) {
  integer i = 0;
  integer j = 0;
  integer n = (integer) nXD;
  integer n__1 = n + 1;
  integer info = 0;
  integer c__1 = 1;
  integer c_n1 = -1;
  void* work = NULL;
  void* b = NULL;
  char uplo[1] = { 'U' };
  char jobz[1] = { 'V' };
  extern integer ilaenv_(integer*,char*,char*,integer*,integer*,integer*,integer*,ftnlen,ftnlen);

#ifdef __MAX_TYPE_32BIT
  extern int ssyev_(char*,char*,integer*,real*,integer*,real*,real*,integer*,integer*);
  extern int f2c_scopy(integer *, real *, integer *, real *, integer *);
  char opts[8] = {'S', 'S', 'Y', 'T', 'R', 'D'};
  integer lwork = (ilaenv_(&c__1, opts, uplo, &n, &c_n1, &c_n1, &c_n1, (ftnlen) 6, (ftnlen) 1) + 2) * n;
  work = dlp_calloc(lwork, sizeof(real));
  b = dlp_calloc(n, sizeof(real));
  if (!work || !b) return ERR_MEM;
  ssyev_(jobz, uplo, &n, A, &n, b, work, &lwork, &info);
  for (i = 0; i < n; i++) dlp_memmove(B + (n - i - 1) * n, A + i * n, n * sizeof(FLOAT64));
  dlp_memset(A, 0, n * n * sizeof(FLOAT64));
  f2c_scopy(&n, b, &c_n1, (real*)A, &n__1);
#else
  extern int dsyev_(char*,char*,integer*,doublereal*,integer*,doublereal*,doublereal*,integer*,integer*);
  extern int f2c_dcopy(integer *, doublereal *, integer *, doublereal *, integer *);
  char opts[8] = { 'D', 'S', 'Y', 'T', 'R', 'D' };
  integer lwork = (ilaenv_(&c__1, opts, uplo, &n, &c_n1, &c_n1, &c_n1, (ftnlen) 6, (ftnlen) 1) + 2) * n;
  work = dlp_calloc(lwork, sizeof(doublereal));
  b = dlp_calloc(n, sizeof(doublereal));
  if (!work || !b) return ERR_MEM;
  dsyev_(jobz, uplo, &n, A, &n, b, work, &lwork, &info);
  for (i = 0; i < n; i++) dlp_memmove(B + (n - i - 1) * n, A + i * n, n * sizeof(FLOAT64));
  dlp_memset(A, 0, n * n * sizeof(FLOAT64));
  f2c_dcopy(&n, b, &c_n1, (doublereal*)A, &n__1);
#endif

  /* Normalize eigenvector matrix by squareroots of eigenvalues */
  if (bNorm) {
    for (i = 0; i < nXD; i++) {
      FLOAT64 a = sqrt(B[i * nXD + i]);
      for (j = i; j < nXD; j++) {
        if (a != 0.0) A[j * nXD + i] /= a;
        else A[j * nXD + i] = 0;
      }
    }
  }

  dlp_free(work);
  dlp_free(b);
  if (info != 0) return NOT_EXEC;
  return O_K;
}

/** 
 * Restore original matrix from eigenvalue and eigenvector matrices.
 * 
 * @param Z
 *          Pointer to <code>n</code> x <code>n</code> result matrix
 * @param EVL
 *          Pointer to <code>n</code> x <code>n</code> eigenvalue matrix. Will
 *          be overwritten in computation.
 * @param EVC
 *          Pointer to <code>n</code> x <code>n</code> eigenvector matrix
 * @param nXD
 *          Order of matrix (number of rows and columns)
 * @return <code>O_K</code> if successfull, a (negative) error code otherwise
 * 
 * @see dlm_eigen_jac
 */
INT16 dlm_ieigen(FLOAT64* Z, FLOAT64* EVL, FLOAT64* EVC, INT32 nXD, INT16 bNorm) {
  INT32 i, j;
  FLOAT64 v;

  /* Normalize eigenvector matrix by squareroots of eigenvalues */
  if (bNorm) for (i = 0; i < nXD; i++) {
    FLOAT64 a = sqrt(EVL[i * nXD + i]);
    for (j = i; j < nXD; j++) {
      if (a != 0.0) EVC[j * nXD + i] *= a;
      else EVC[j * nXD + i] = 0;
    }
  }

  for (i = 0; i < nXD; i++) {
    v = EVL[i * nXD + i];
    for (j = 0; j < nXD; j++)
      EVL[i * nXD + j] = v * EVC[i * nXD + j];
  }

  return dlm_matrop(Z, NULL, NULL, EVL, nXD, nXD, EVC, nXD, nXD, OP_MULT);
}

/* EOF */
