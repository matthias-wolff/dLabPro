/* dLabPro mathematics library
 * - Gaussian elimination
 *
 * AUTHOR : Matthias Wolff
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
 * <p id="dlm_invert_gel">Inverts a matrix and computes its determinant through Gaussian
 * elimination.</p>
 *
 * @param A
 *          Pointer to input matrix, replaced in computation by resultant
 *          inverse
 * @param nXA
 *          Order of matrix (number of rows and columns)
 * @param lpnDet
 *          Pointer to be filled with resultant determinant (may be
 *          <code>NULL</code>)
 * @return <code>O_K</code> if successfull, a (negative) error code otherwise
 */
INT16 dlm_invert_gel(FLOAT64* A, INT32 nXA, FLOAT64* lpnDet) {
  integer n = (integer) nXA;
  integer c__1 = 1;
  integer c_n1 = -1;
  integer info = 0;
  integer* ipiv = dlp_calloc(n, sizeof(integer));
  void* work = NULL;
  char opts[1] = { ' ' };
  extern integer ilaenv_(integer*,char*,char*,integer*,integer*,integer*,integer*,ftnlen,ftnlen);

#ifdef __MAX_TYPE_32BIT
  extern int sgetrf_(integer*,integer*,real*,integer*,integer*,integer*);
  extern int sgetri_(integer*,real*,integer*,integer*,real*,integer*,integer*);
  char name[8] = { 'S', 'G', 'E', 'T', 'R', 'I' };
  integer lwork = n * ilaenv_(&c__1, name, opts, &n, &c_n1, &c_n1, &c_n1, (ftnlen)6, (ftnlen)1);
  work = dlp_calloc(lwork, sizeof(real));
  if(!ipiv || !work) return ERR_MEM;
  sgetrf_(&n,&n,A,&n,ipiv,&info);
  if(lpnDet != NULL) *lpnDet = (info > 0) ? 0.0 : dlm_get_det_trf(A, nXA, ipiv);
  sgetri_(&n,A,&n,ipiv,work,&lwork,&info);
#else
  extern int dgetrf_(integer*,integer*,doublereal*,integer*,integer*,integer*);
  extern int dgetri_(integer*,doublereal*,integer*,integer*,doublereal*,integer*,integer*);
  char name[8] = { 'D', 'G', 'E', 'T', 'R', 'I' };
  integer lwork = n * ilaenv_(&c__1, name, opts, &n, &c_n1, &c_n1, &c_n1, (ftnlen) 6, (ftnlen) 1);
  work = dlp_calloc(lwork, sizeof(doublereal));
  if (!ipiv || !work) return ERR_MEM;
  dgetrf_(&n, &n, A, &n, ipiv, &info);
  if (lpnDet != NULL) *lpnDet = (info > 0) ? 0.0 : dlm_get_det_trf(A, nXA, ipiv);
  dgetri_(&n, A, &n, ipiv, work, &lwork, &info);
#endif

  dlp_free(work);
  dlp_free(ipiv);
  return (info == 0) ? O_K : NOT_EXEC;
}

/**
 * <p>Same as {@link dlm_invert_gel} but for complex input</p>
 *
 */
INT16 dlm_invert_gelC(COMPLEX64* A, INT32 nXA, COMPLEX64* lpnDet) {
  integer n = (integer) nXA;
  integer c__1 = 1;
  integer c_n1 = -1;
  integer info = 0;
  integer* ipiv = dlp_calloc(n, sizeof(integer));
  void* work = NULL;
  char opts[1] = { ' ' };
  extern integer ilaenv_(integer*,char*,char*,integer*,integer*,integer*,integer*,ftnlen,ftnlen);

#ifdef __MAX_TYPE_32BIT
  extern int cgetrf_(integer*,integer*,complex*,integer*,integer*,integer*);
  extern int cgetri_(integer*,complex*,integer*,integer*,complex*,integer*,integer*);
  char name[8] = { 'C', 'G', 'E', 'T', 'R', 'I' };
  integer lwork = n * ilaenv_(&c__1, name, opts, &n, &c_n1, &c_n1, &c_n1, (ftnlen)6, (ftnlen)1);
  work = dlp_calloc(lwork, sizeof(complex));
  if(!ipiv || !work) return ERR_MEM;
  cgetrf_(&n,&n,(complex*)A,&n,ipiv,&info);
  if(lpnDet != NULL) *lpnDet = (info > 0) ? CMPLX(0.0) : dlm_get_det_trfC(A, nXA, ipiv);
  cgetri_(&n,(complex*)A,&n,ipiv,work,&lwork,&info);
#else
  extern int zgetrf_(integer*,integer*,doublecomplex*,integer*,integer*,integer*);
  extern int zgetri_(integer*,doublecomplex*,integer*,integer*,doublecomplex*,integer*,integer*);
  char name[8] = { 'Z', 'G', 'E', 'T', 'R', 'I' };
  integer lwork = n * ilaenv_(&c__1, name, opts, &n, &c_n1, &c_n1, &c_n1, (ftnlen) 6, (ftnlen) 1);
  work = dlp_calloc(lwork, sizeof(doublecomplex));
  if (!ipiv || !work) return ERR_MEM;
  zgetrf_(&n, &n, (doublecomplex*) A, &n, ipiv, &info);
  if (lpnDet != NULL) *lpnDet = (info > 0) ? CMPLX(0.0) : dlm_get_det_trfC(A, nXA, ipiv);
  zgetri_(&n, (doublecomplex*) A, &n, ipiv, work, &lwork, &info);
#endif

  dlp_free(work);
  dlp_free(ipiv);
  return (info == 0) ? O_K : NOT_EXEC;
}

FLOAT64 dlm_get_det_trf(FLOAT64* A, INT32 nXA, void* ipiv) {
  INT32 iXA = 0;
  FLOAT64 det = 1.0;
  BOOL neg = FALSE;
  integer* p_ipiv = (integer*) ipiv;

  for (iXA = 0; iXA < nXA; iXA++) {
    det *= A[iXA + iXA * nXA];
    neg = (p_ipiv[iXA] != (iXA + 1)) ? !neg : neg;
  }
  return neg ? -det : det;
}

COMPLEX64 dlm_get_det_trfC(COMPLEX64* A, INT32 nXA, void* ipiv) {
  INT32 iXA = 0;
  COMPLEX64 det = CMPLX(1.0);
  BOOL neg = FALSE;
  integer* p_ipiv = (integer*) ipiv;

  for (iXA = 0; iXA < nXA; ++iXA) {
    det = CMPLX_MULT(det,A[iXA+iXA*nXA]);
    neg = (p_ipiv[iXA] != (iXA + 1)) ? !neg : neg;
  }
  return neg ? CMPLX_NEG(det) : det;
}
/* EOF */
