/* dLabPro mathematics library
 * - Cholesky factorization of matrices
 *
 * AUTHOR :  
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
#include "f2c.h"

/**
 * Cholesky factorization of a square, symmetric matrix. The function computes
 * an upper triangular matrix <b>Z</b> such that <b>Z</b>'&middot;<b>Z</b> =
 * <b>A</b>.
 * 
 * @param Z
 *          Pointer to output matrix, buffer must be able to hold
 *          <code>nXD</code><sup>2</sup> double values
 * @param A
 *          Pointer to square, symmetric input matrix, must not be equal
 *          <code>Z</code>.
 * @param nXD
 *          Number of rows and columns of <code>A</code> and <code>Z</code>
 * @return <code>O_K</code> if successful, a (negative) error code otherwise
 */
INT16 dlm_cholf(FLOAT64* Z, const FLOAT64* A, INT32 nXD) {
  integer info = 0;
  integer n = (integer) nXD;
  char uplo[1] = { 'U' };
#ifdef __MAX_TYPE_32BIT
  extern int slacpy_(char*,integer*,integer*,real*,integer*,real*,integer *ldb);
  extern int spotrf_(char*,integer*,real*,integer*,integer*);
#else
  extern int dlacpy_(char*,integer*,integer*,doublereal*,integer*,doublereal*,integer *ldb);
  extern int dpotrf_(char*,integer*,doublereal*,integer*,integer*);
#endif

  /* Declare variables */
  DLPASSERT(Z != A);                                                            /* Assert input is not output        */
  DLPASSERT(dlp_size(Z) >= nXD * nXD * sizeof(FLOAT64));                        /* Check size of output buffer       */
  DLPASSERT(dlp_size(A) >= nXD * nXD * sizeof(FLOAT64));                        /* Check size of input buffer        */

  /* ... computation ... *//* --------------------------------- */
#ifdef __MAX_TYPE_32BIT
  spotrf_(uplo, &n, (real*)A, &n, &info);
  slacpy_(uplo, &n, &n, (real*)A, &n, (real*)Z, &n);
#else
  dpotrf_(uplo, &n, (doublereal*)A, &n, &info);
  dlacpy_(uplo, &n, &n, (doublereal*)A, &n, (doublereal*)Z, &n);
#endif
  return (info == 0) ? O_K : NOT_EXEC; /* All done successfully             */
}

INT16 dlm_cholfC(COMPLEX64* Z, const COMPLEX64* A, INT32 nXD) {
  integer info = 0;
  integer n = (integer) nXD;
  char uplo[1] = { 'U' };
#ifdef __MAX_TYPE_32BIT
  extern int clacpy_(char*,integer*,integer*,complex*,integer*,complex*,integer *ldb);
  extern int cpotrf_(char*,integer*,complex*,integer*,integer*);
#else
  extern int zlacpy_(char*,integer*,integer*,doublecomplex*,integer*,doublecomplex*,integer *ldb);
  extern int zpotrf_(char*,integer*,doublecomplex*,integer*,integer*);
#endif

  /* Declare variables */
  DLPASSERT(Z != A);                                                            /* Assert input is not output        */
  DLPASSERT(dlp_size(Z) >= nXD * nXD * sizeof(FLOAT64));                        /* Check size of output buffer       */
  DLPASSERT(dlp_size(A) >= nXD * nXD * sizeof(FLOAT64));                        /* Check size of input buffer        */

  /* ... computation ... *//* --------------------------------- */
#ifdef __MAX_TYPE_32BIT
  cpotrf_(uplo, &n, (complex*)A, &n, &info);
  clacpy_(uplo, &n, &n, (complex*)A, &n, (complex*)Z, &n);
#else
  zpotrf_(uplo, &n, (doublecomplex*)A, &n, &info);
  zlacpy_(uplo, &n, &n, (doublecomplex*)A, &n, (doublecomplex*)Z, &n);
#endif
  return (info == 0) ? O_K : NOT_EXEC; /* All done successfully             */
}
/* EOF */
