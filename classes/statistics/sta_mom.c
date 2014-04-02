/* dLabPro class CStatistics (statistics)
 * - Moment estimation methods
 *
 * AUTHOR : Christian-M. Westendorf, Matthias Wolff
 * PACKAGE: dLabPro/classes
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

#include "dlp_cscope.h"                                                         /* Indicate C scope                  */
#include "dlp_statistics.h"                                                     /* Include class header file         */
#include "dlp_math.h"

/**
 * Compute extents of class c. There are no checks performed!
 *
 * @param _this
 *          Pointer to this CStatistics instance
 * @param lpY
 *          Pointer to output buffer, must be capable of holding N doubles
 * @param c
 *          The statistics' class index (0 <= c < C)
 * @param N
 *          The statistics' dimensionality
 * @param nMode
 *          Extent mode, one of the <code>STA_KMA_XXX</code> constants
 */
void CGEN_PRIVATE CStatistics_Moment0
(
  CStatistics* _this,
  FLOAT64*     lpY,
  INT32        c,
  INT32        N,
  INT16        nMode
)
{
  INT32    n     = 0;                                                            /* Dimension loop counter            */
  FLOAT64* lpSsz = NULL;                                                         /* Ptr. to class' c sample size      */
  FLOAT64* lpMin = NULL;                                                         /* Ptr. to class' c minimum vector   */
  FLOAT64* lpMax = NULL;                                                         /* Ptr. to class' c maximum vector   */

  DLPASSERT((lpSsz = CStatistics_GetPtr(_this,c,STA_DAI_SSIZE)));               /* Get ptr. to class' c sample size  */
  DLPASSERT((lpMin = CStatistics_GetPtr(_this,c,STA_DAI_MIN  )));               /* Get ptr. to class' c min. vector  */
  DLPASSERT((lpMax = CStatistics_GetPtr(_this,c,STA_DAI_MAX  )));               /* Get ptr. to class' c max. vector  */
  for (n=0; n<N; n++)                                                           /* Loop over dimensions              */
    if (lpSsz[0]>0.)                                                            /*   Have samples for class c        */
      switch (nMode)                                                            /*     Branch for mode               */
      {                                                                         /*     >>                            */
        case STA_KMA_MIN : lpY[n] = lpMin[n];          break;                   /*       Minimum                     */
        case STA_KMA_MAX : lpY[n] = lpMax[n];          break;                   /*       Maximum                     */
        case STA_KMA_SPAN: lpY[n] = lpMax[n]-lpMin[n]; break;                   /*       Span                        */
        default          : lpY[n] = 1.;                break;                   /*       0-th moment := 1 by def.    */
      }                                                                         /*     <<                            */
    else lpY[n] = 0.;                                                           /*   No samples, no computations     */
}

/**
 * Estimates 1st order moment of class c. There are no checks performed!
 *
 * @param _this
 *          Pointer to this CStatistics instance
 * @param lpY
 *          Pointer to output buffer, must be capable of holding N doubles
 * @param c
 *          The statistics' class index (0 <= c < C)
 * @param N
 *          The statistics' dimensionality
 * @param nMode
 *          Extent mode, one of the <code>STA_KMA_XXX</code> constants
 */
void CGEN_PRIVATE CStatistics_Moment1
(
  CStatistics* _this,
  FLOAT64*      lpY,
  INT32         c,
  INT32         N,
  INT16        nMode
)
{
  INT32    n     = 0;                                                            /* Dimension loop counter            */
  FLOAT64* lpSsz = NULL;                                                         /* Ptr. to class' c sample size      */
  FLOAT64* lpSum = NULL;                                                         /* Ptr. to class' c sum vector       */

  DLPASSERT((lpSsz = CStatistics_GetPtr(_this,c,STA_DAI_SSIZE)));               /* Get ptr. to class' c sample size  */
  DLPASSERT((lpSum = CStatistics_GetPtr(_this,c,STA_DAI_SUM  )));               /* Get ptr. to class' c sum vector   */
  for (n=0; n<N; n++)                                                           /* Loop over dimensions              */
    if (lpSsz[0]>0. && (nMode&STA_KMA_CENT)==0)                                 /*   Have samples and not central    */
      switch (nMode)                                                            /*     Branch for mode               */
      {                                                                         /*     >>                            */
        case 0 : lpY[n] = lpSum[n]/lpSsz[0]; break;                             /*       Mean                        */
        default: DLPASSERT(FMSG("Wrong mode for 1st moment"));                  /*       Cannot happen               */
      }                                                                         /*     <<                            */
    else lpY[n] = 0.;                                                           /*   No samples, no computations     */
}

/**
 * Estimates 2nd order (central) moment of class c. There are no checks
 * performed!
 *
 * @param _this
 *          Pointer to this CStatistics instance
 * @param lpY
 *          Pointer to output buffer, must be capable of holding N doubles
 * @param c
 *          The statistics' class index (0 <= c < C)
 * @param N
 *          The statistics' dimensionality
 * @param nMode
 *          Extent mode, a co of the <code>STA_KMA_XXX</code> constants
 */
void CGEN_PRIVATE CStatistics_Moment2
(
  CStatistics* _this,
  FLOAT64*      lpY,
  INT32         c,
  INT32         N,
  INT16        nMode
)
{
  INT32    n     = 0;                                                            /* Dimension loop counter            */
  FLOAT64* lpSsz = NULL;                                                         /* Ptr. to class' c sample size      */
  FLOAT64* lpSum = NULL;                                                         /* Ptr. to class' c sum vector       */
  FLOAT64* lpMsm = NULL;                                                         /* Ptr. to class' c mixed sum vector */

  DLPASSERT((lpSsz = CStatistics_GetPtr(_this,c,STA_DAI_SSIZE)));               /* Get ptr. to class' c sample size  */
  DLPASSERT((lpSum = CStatistics_GetPtr(_this,c,STA_DAI_SUM  )));               /* Get ptr. to class' c sum vector   */
  DLPASSERT((lpMsm = CStatistics_GetPtr(_this,c,STA_DAI_MSUM )));               /* Get ptr. to class' c mix. sum v.  */
  for (n=0; n<N; n++)                                                           /* Loop over dimensions              */
  {                                                                             /* >>                                */
    if (lpSsz[0]<=0.) { lpY[n] = 0.; continue; }                                /*   No samples, no 2nd moment       */
    if (nMode&STA_KMA_CENT && nMode&STA_KMA_N1)                                 /*   Variance (\sigma^2_{n-1})       */
      lpY[n] = CStatistics_Cov(_this,lpMsm,lpSum,N,lpSsz[0],n,n);               /*   ...                             */
    else if (nMode&STA_KMA_CENT)                                                /*   2nd ctrl. moment (\sigma^2_n)   */
      lpY[n] = (lpMsm[n*N+n]-(lpSum[n]*lpSum[n]/lpSsz[0]))/lpSsz[0];            /*   ...                             */
    else                                                                        /*   2nd moment (\sigma^2_n)         */
      lpY[n] = lpMsm[n*N+n]/lpSsz[0];                                           /*   ...                             */
    if (nMode&STA_KMA_SDEV) lpY[n] = sqrt(lpY[n]);                              /*   "Standard deviation"            */
    if (nMode&STA_KMA_COEF) lpY[n] = sqrt(lpY[n])/lpSum[n]*lpSsz[0];            /*   "Variation coefficent"          */
  }                                                                             /* <<                                */
}

/**
 * Estimates k-th order (central) moment of class c. There are no checks
 * performed! Computing the k-th order contral moment is done using Steiner's
 * shift theorem (see Lexikon der Stochastik)

 * @author Christian-M. Westendorf
 * @param _this
 *          Pointer to this CStatistics instance
 * @param lpY
 *          Pointer to output buffer, must be capable of holding N doubles
 * @param c
 *          The statistics' class index (0 <= c < C)
 * @param k
 *          The moment order
 * @param N
 *          The statistics' dimensionality
 * @param K
 *          The statistics' maximal order
 * @param nMode
 *          Extent mode, one of the <code>STA_KMA_XXX</code> constants
 */
void CGEN_PRIVATE CStatistics_MomentK
(
  CStatistics* _this,
  FLOAT64*      lpY,
  INT32         c,
  INT32         k,
  INT32         N,
  INT32         K,
  INT16        nMode
)
{
  INT32    j     = 0;                                                            /* Order loop counter                */
  INT32    n     = 0;                                                            /* Dimension loop counter            */
  FLOAT64* lpSsz = NULL;                                                         /* Ptr. to class' c sample size      */

  if (k==0) {CStatistics_Moment0(_this,lpY,c,N,nMode); return;}                 /* Order 0 has a dedicated routine   */
  if (k==1) {CStatistics_Moment1(_this,lpY,c,N,nMode); return;}                 /* Order 1 has a dedicated routine   */
  if (k==2) {CStatistics_Moment2(_this,lpY,c,N,nMode); return;}                 /* Order 2 has a dedicated routine   */

  for (n=0; n<N; n++) lpY[n] = 0.;                                              /* Clear destination buffer          */
  if (k>K) return;                                                              /* Enforce it                        */
  DLPASSERT((lpSsz = CStatistics_GetPtr(_this,c,STA_DAI_SSIZE)));               /* Get ptr. to class' c sample size  */

  if (nMode&STA_KMA_CENT)                                                       /* k-th central moment               */
  {                                                                             /* >> (via Steiner's shift theorem)  */
    FLOAT64** lppKsm = NULL;                                                     /*   Pointers to k-th order sums     */
    FLOAT64*  lp0sm  = NULL;                                                     /*   Pointer to 0th order sums buff. */
    FLOAT64*  lp2sm  = NULL;                                                     /*   Pointer to 2nd order sums buff. */
    FLOAT64*  lpMsm  = NULL;                                                     /*   Ptr. to class' c mixed sum vec. */
    lppKsm = (FLOAT64**)dlp_calloc(K+1,sizeof(FLOAT64*));                         /*   Allocate pointer array          */
    lp0sm  = (FLOAT64* )dlp_calloc(N  ,sizeof(FLOAT64 ));                         /*   Allocate 0th order sums buffer  */
    lp2sm  = (FLOAT64* )dlp_calloc(N  ,sizeof(FLOAT64 ));                         /*   Allocate 2nd order sums buffer  */
    lpMsm  = CStatistics_GetPtr(_this,c,STA_DAI_MSUM);                          /*   Get ptr. to class' c mix. sum v.*/
    for (n=0; n<N; n++) { lp0sm[n] = 1.; lp2sm[n] = lpMsm[n*N+n]; }             /*   Init 0th and copy 2nd order sums*/
    lppKsm[0] = lp0sm;                                                          /*   Ptr. to 2nd order sum vector    */
    lppKsm[1] = CStatistics_GetPtr(_this,c,STA_DAI_SUM);                        /*   Ptr. to 1st order sum vector    */
    lppKsm[2] = lp2sm;                                                          /*   Ptr. to 2nd order sum vector    */
    for (j=3; j<=K; j++)                                                        /*   Ptrs. to k-th order sum vectors */
      lppKsm[j] = CStatistics_GetPtr(_this,c,STA_DAI_KSUM) + N*(j-3);           /*   ...                             */
    for (n=0; n<N; n++)                                                         /*   Loop over dimensions            */
      for (j=0; j<=k; j++)                                                      /*     Compute E(X-EX)^K             */
        lpY[n] += dlm_pow(-1.,k-j) * dlp_scalop(k,j,OP_NOVERK)                      /*     |                             */
                * lppKsm[j][n]/lpSsz[0] * dlm_pow(lppKsm[1][n]/lpSsz[0],k-j);       /*     |                             */
    dlp_free(lppKsm);                                                           /*   Free sum vectors pointer array  */
    dlp_free(lp0sm);                                                            /*   Free 0th order sums buffer      */
    dlp_free(lp2sm);                                                            /*   Free 2nd order sums buffer      */
  }                                                                             /* <<                                */
  else                                                                          /* k-th ordinary moment              */
  {                                                                             /* >>                                */
    FLOAT64* lpKsm = CStatistics_GetPtr(_this,c,STA_DAI_KSUM) + N*(k-3);         /*   Get ptr. to k-th order sum vec. */
    for (n=0; n<N; n++) lpY[n] = lpKsm[n]/lpSsz[0];                             /*   Estimate k-th moment            */
  }                                                                             /* <<                                */
}

/**
 * Estimates the skewness of class c. There are no checks performed!
 *
 * @param _this
 *          Pointer to this CStatistics instance
 * @param lpY
 *          Pointer to output buffer, must be capable of holding N doubles
 * @param c
 *          The statistics' class index (0 <= c < C)
 * @param N
 *          The statistics' dimensionality
 * @param K
 *          The statistics' maximal order
 * @param nMode
 *          Extent mode, one of the <code>STA_KMA_XXX</code> constants
 */
void CGEN_PRIVATE CStatistics_Skewness
(
  CStatistics* _this,
  FLOAT64*      lpY,
  INT32         c,
  INT32         N,
  INT32         K
)
{
  INT32    n     = 0;                                                            /* Dimension loop counter            */
  FLOAT64* lpVar = NULL;                                                         /* Buffer for variance vector        */
  FLOAT64* lp3cm = NULL;                                                         /* Buffer for 3rd central moment vec.*/
  for (n=0; n<N; n++) lpY[n] = 0.;                                              /* Clear output array                */
  if (K<3) return;                                                              /* Need at least 3rd order stats.    */
  lpVar = (FLOAT64*)dlp_calloc(N,sizeof(FLOAT64));                                /* Allocate variance vector          */
  lp3cm = (FLOAT64*)dlp_calloc(N,sizeof(FLOAT64));                                /* Allocate 3rd central moment vector*/
  CStatistics_Moment2(_this,lpVar,c,  N,  STA_KMA_CENT|STA_KMA_N1);             /* Estimate variance vectors         */
  CStatistics_MomentK(_this,lp3cm,c,3,N,K,STA_KMA_CENT           );             /* Estimate 3rd order central moment */
  for (n=0; n<N; n++) lpY[n] = lp3cm[n]/dlm_pow(lpVar[n],1.5);                      /* Compute skewness                  */
  dlp_free(lpVar);                                                              /* Free variance vector              */
  dlp_free(lp3cm);                                                              /* Free 3rd central moment vector    */
}

/**
 * Estimates the excess of class c. There are no checks performed!
 *
 * @param _this
 *          Pointer to this CStatistics instance
 * @param lpY
 *          Pointer to output buffer, must be capable of holding N doubles
 * @param c
 *          The statistics' class index (0 <= c < C)
 * @param N
 *          The statistics' dimensionality
 * @param K
 *          The statistics' maximal order
 * @param nMode
 *          Extent mode, one of the <code>STA_KMA_XXX</code> constants
 */
void CGEN_PRIVATE CStatistics_Excess
(
  CStatistics* _this,
  FLOAT64*      lpY,
  INT32         c,
  INT32         N,
  INT32         K
)
{
  INT32    n     = 0;                                                            /* Dimension loop counter            */
  FLOAT64* lpVar = NULL;                                                         /* Buffer for variance vector        */
  FLOAT64* lp4cm = NULL;                                                         /* Buffer for 4th central moment vec.*/
  for (n=0; n<N; n++) lpY[n] = 0.;                                              /* Clear output array                */
  if (K<4) return;                                                              /* Need at least 3rd order stats.    */
  lpVar = (FLOAT64*)dlp_calloc(N,sizeof(FLOAT64));                                /* Allocate variance vector          */
  lp4cm = (FLOAT64*)dlp_calloc(N,sizeof(FLOAT64));                                /* Allocate 3rd central moment vector*/
  CStatistics_Moment2(_this,lpVar,c,  N,  STA_KMA_CENT|STA_KMA_N1);             /* Estimate variance vectors         */
  CStatistics_MomentK(_this,lp4cm,c,4,N,K,STA_KMA_CENT           );             /* Estimate 4th order central moment */
  for (n=0; n<N; n++) lpY[n] = lp4cm[n]/lpVar[n]/lpVar[n]-3;                    /* Compute excess                    */
  dlp_free(lpVar);                                                              /* Free variance vector              */
  dlp_free(lp4cm);                                                              /* Free 3rd central moment vector    */
}

/**
 * Estimates the k-th (central) moment vectors for all classes of the
 * statistics.
 *
 * @param _this
 *          Pointer to this CStatistics instance
 * @param idDst
 *          Pointer to a data instance to be filled with the moment vectors
 * @param k
 *          The moment order
 * @param nMode
 *          A combination of the <code>STA_KMA_XXX</CODE> constants, the
 *          interpretation of this parameter depends on the moment order
 *          <code>k</code>
 * @return <code>O_K</code> if successfull, a negative error code otherwise
 */
INT16 CGEN_PUBLIC CStatistics_MomentEx
(
  CStatistics* _this,
  CData*       idDst,
  INT16        k,
  INT16        nMode
)
{
  INT32    c   = 0;                                                              /* Class loop counter                */
  INT32    C   = 0;                                                              /* Number of statistics classes      */
  INT32    K   = 0;                                                              /* Statistics order                  */
  INT32    N   = 0;                                                              /* Statistics dimensionality         */
  FLOAT64* lpY = NULL;                                                           /* Pointer into output data instance */

  /* Validate */                                                                /* --------------------------------- */
  if (!idDst) return IERROR(_this,ERR_NULLARG,"idDst",0,0);                     /* Check output data instance        */
  CData_Reset(idDst,TRUE);                                                      /* Clear destination instance        */
  CHECK_THIS_RV(NOT_EXEC);                                                      /* Check this pointer                */
  IF_NOK(CStatistics_Check(_this))                                              /* Check instance data               */
    return IERROR(_this,STA_NOTSETUP," ( use -status for details)",0,0);        /* ...                               */

  /* Initialize */                                                              /* --------------------------------- */
  C = CStatistics_GetNClasses(_this);                                           /* Get number of statistics classes  */
  K = CStatistics_GetOrder(_this);                                              /* Get statistics order              */
  N = CStatistics_GetDim(_this);                                                /* Get statistics dimensionality     */
  CData_Array(idDst,T_DOUBLE,N,C);                                              /* Allocate output data instance     */
  CData_SetNBlocks(idDst,C);                                                    /* Set block number                  */
  if (!CData_XAddr(idDst,0,0)) return IERROR(_this,ERR_NOMEM,0,0,0);            /* Should have been successfull ...  */

  /* Loop over classes and estimate moments */                                  /* --------------------------------- */
  for (c=0; c<C; c++)                                                           /* Loop over classes                 */
  {                                                                             /* >>                                */
    lpY = (FLOAT64*)CData_XAddr(idDst,c,0);                                      /*   Get pointer to output vector    */
    if      (k==3 && nMode&STA_KMA_SKEW) CStatistics_Skewness(_this,lpY,c,N,K); /*   Skewness                        */
    else if (k==4 && nMode&STA_KMA_EXCS) CStatistics_Excess  (_this,lpY,c,N,K); /*   Excess (Kurtosis)               */
    else switch (k)                                                             /*   Branch for moment order         */
    {                                                                           /*   >>                              */
      case 0 : CStatistics_Moment0(_this,lpY,c,  N,  nMode); break;             /*     k=0                           */
      case 1 : CStatistics_Moment1(_this,lpY,c,  N,  nMode); break;             /*     k=1                           */
      case 2 : CStatistics_Moment2(_this,lpY,c,  N,  nMode); break;             /*     k=2                           */
      default: CStatistics_MomentK(_this,lpY,c,k,N,K,nMode); break;             /*     All other orders              */
    }                                                                           /*   <<                              */
  }                                                                             /* <<                                */

  return O_K;                                                                   /* All done                          */
}

/* EOF */
