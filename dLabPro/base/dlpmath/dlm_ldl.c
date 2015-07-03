/* dLabPro mathematics library
 * - LDL factorization
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

#include "dlp_kernel.h"
#include "dlp_base.h" 
#include "dlp_math.h"

/**
 * <p id="dlm_factldl">Factorisize matrix to lower triangular matrix L and diagonal matrix D (A=LDL<sup>T</sup>)<p>
 *
 * @param l Double array for source matrix and lower triangular matrix
 * @param d Double array for diagonal matrix
 * @param N Dimension of the matrix
 * @return <code>O_K</code> if successfull
 */
#define MAT(a,i,j)  ((a)[(j)*N+(i)])                                            /* Makro for indexing mat.-elements  */
INT16 dlm_factldl(FLOAT64 *l,FLOAT64 *d,INT32 N)
{
  INT32 i,j,k;                                                                   /* Loop indizies                     */

  /* Calculate LDL */                                                           /*   ------------------------------- */
  for(j=0;j<N;j++){                                                             /*   Loop over matrix dimension >>   */
    for(i=0;i<j;i++) MAT(d,i,0)=MAT(l,j,i)*MAT(l,i,i);                          /*     Calc. V from upper triag. mat.*/
    MAT(d,j,0)=MAT(l,j,j);                                                      /*     Get diagonal element          */
    for(i=0;i<j;i++) MAT(d,j,0)-=MAT(l,j,i)*MAT(d,i,0);                         /*     Sub. prod. of V and row of A  */
    MAT(l,j,j)=MAT(d,j,0);                                                      /*     Save new diagonal element     */
    for(k=j+1;k<N;k++){                                                         /*     For low. triag. row len. >>   */
      for(i=0;i<j;i++) MAT(l,k,j)-=MAT(l,k,i)*MAT(d,i,0);                       /*       Calculate new row elem. of A*/
      MAT(l,k,j)/=MAT(d,j,0);                                                   /*       Normalize row element       */
    }                                                                           /*     <<                            */
  }                                                                             /*   <<                              */
  /* Store block */                                                             /*   ------------------------------- */
  for(j=0;j<N;j++){                                                             /*   Loop over matrix dimension >>   */
    MAT(d,j,j)=MAT(l,j,j);                                                      /*     Save new diagonal elem. of A  */
    MAT(l,j,j)=1;                                                               /*     Insert ones in diagonal       */
    for(i=j+1;i<N;i++) MAT(l,j,i)=0;                                            /*     Clear upper triangular matrix */
  }                                                                             /*   <<                              */
  for(j=0;j<N;j++) for(i=0;i<N;i++) if(i!=j) MAT(d,j,i)=0;                      /*     Clear triangular matrizies    */

  return O_K;                                                                   /* All done                          */
}

/*
 * Complex version of {@link dlm_factldl}
 */
INT16 dlm_factldlC(COMPLEX64 *l,COMPLEX64 *d,INT32 N)
{
  INT32 i,j,k;                                                                  /* Loop indizies                     */

  /* Calculate LDL */                                                           /*   ------------------------------- */
  for(j=0;j<N;j++){                                                             /*   Loop over matrix dimension >>   */
    for(i=0;i<j;i++) MAT(d,i,0)=dlp_scalopC(MAT(l,j,i),MAT(l,i,i),OP_MULT);     /*     Calc. V from upper triag. mat.*/
    MAT(d,j,0)=MAT(l,j,j);                                                      /*     Get diagonal element          */
    for(i=0;i<j;i++) MAT(d,j,0)=CMPLX_MINUS(MAT(d,j,0),dlp_scalopC(MAT(l,j,i),MAT(d,i,0),OP_MULT));    /*     Sub. prod. of V and row of A  */
    MAT(l,j,j)=MAT(d,j,0);                                                      /*     Save new diagonal element     */
    for(k=j+1;k<N;k++){                                                         /*     For low. triag. row len. >>   */
      for(i=0;i<j;i++) MAT(l,k,j)=CMPLX_MINUS(MAT(l,k,j),dlp_scalopC(MAT(l,k,i),MAT(d,i,0),OP_MULT)); /*       Calculate new row elem. of A*/
      MAT(l,k,j)=dlp_scalopC(MAT(l,k,j),MAT(d,j,0),OP_DIV);                     /*       Normalize row element       */
    }                                                                           /*     <<                            */
  }                                                                             /*   <<                              */
  /* Store block */                                                             /*   ------------------------------- */
  for(j=0;j<N;j++){                                                             /*   Loop over matrix dimension >>   */
    MAT(d,j,j)=MAT(l,j,j);                                                      /*     Save new diagonal elem. of A  */
    MAT(l,j,j)=CMPLX(1.);                                                       /*     Insert ones in diagonal       */
    for(i=j+1;i<N;i++) MAT(l,j,i)=CMPLX(0.);                                    /*     Clear upper triangular matrix */
  }                                                                             /*   <<                              */
  for(j=0;j<N;j++) for(i=0;i<N;i++) if(i!=j) MAT(d,j,i)=CMPLX(0.);              /*     Clear triangular matrizies    */

  return O_K;                                                                   /* All done                          */
}
#undef MAT                                                                      /* Undef makro MAT                   */
