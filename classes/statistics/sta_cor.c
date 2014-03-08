/* dLabPro class CStatistics (statistics)
 * - Correlation estimation methods
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

/**
 * Estimates one covariance. There are no checks performed!
 *
 * @param _this
 *          Pointer to this CStatistics instance
 * @param lpMsum
 *          Pointer to N x N array containing mixed sums x[n]*x[m]
 * @param lpSum
 *          Pointer to 1 x N array containing component sums x[n]
 * @param N
 *          The statistics' dimensionality
 * @param I
 *          The sample size
 * @param n
 *          First covariance dimension
 * @param m
 *          Second covariance dimension
 * @return  Cov(nm)
 */
FLOAT64 CGEN_PRIVATE CStatistics_Cov
(
  CStatistics* _this,
  FLOAT64*      lpMsum,
  FLOAT64*      lpSum,
  INT32         N,
  FLOAT64       I,
  INT32         n,
  INT32         m
)
{
  FLOAT64 nCov = 0.;                                                            /* Return value                      */
  if (!(_this->m_bWeighted || _this->m_bSn) && I<=1.) return _this->m_nMinVar;  /* Sample size < 2 -> min. variance  */
  if ( (_this->m_bWeighted || _this->m_bSn) && I<=0.) return _this->m_nMinVar;  /* Sample size < 2 -> min. variance  */
  nCov  = lpMsum[m*N+n]-(lpSum[n]*lpSum[m]/I);                                  /* Estimate (co)variance             */
  nCov /= (_this->m_bWeighted || _this->m_bSn) ? I : (I-1.);                    /* ... (\sigma_n or \sigma_{n-1})    */
  return fabs(nCov)<_this->m_nMinVar ? _this->m_nMinVar : nCov;                 /* Limit to min. variance and return */
}

/**
 * Estimates correlation based matrices (scatter, covariance, correlation/determination coefficient matrices) for all
 * classes of the statistics.
 *
 * @param _this
 *          Pointer to this CStatistics instance
 * @param idDst
 *          Pointer to a data instance to be filled with the correlation matrices
 * @param nMode
 *          One of the <code>STA_CMA_XXX</CODE> constants
 * @return <code>O_K</code> if successfull, a negative error code otherwise
 */
INT16 CGEN_PUBLIC CStatistics_CorrelationEx
(
  CStatistics* _this,
  CData*       idDst,
  INT16        nMode
)
{
  INT32    i     = 0;                                                            /* Output array index                */
  INT32    c     = 0;                                                            /* Class loop counter                */
  INT32    n     = 0;                                                            /* Dimension loop counter #1         */
  INT32    m     = 0;                                                            /* Dimension loop counter #2         */
  INT32    C     = 0;                                                            /* Number of statistics classes      */
  INT32    N     = 0;                                                            /* Statistics dimensionality         */
  FLOAT64  nVarN = 0.;                                                           /* Variance of dimension n           */
  FLOAT64  nVarM = 0.;                                                           /* Variance of dimension m           */
  FLOAT64* lpY   = NULL;                                                         /* Pointer into output data instance */
  FLOAT64* lpSsz = NULL;                                                         /* Ptr. to class' c sample size      */
  FLOAT64* lpSum = NULL;                                                         /* Ptr. to class' c sum vector       */
  FLOAT64* lpMsm = NULL;                                                         /* Ptr. to class' c mixed sum vector */

  /* Validate */                                                                /* --------------------------------- */
  if (!idDst) return IERROR(_this,ERR_NULLARG,"idDst",0,0);                     /* Check output data instance        */
  CData_Reset(idDst,TRUE);                                                      /* Clear destination instance        */
  CHECK_THIS_RV(NOT_EXEC);                                                      /* Check this pointer                */
  IF_NOK(CStatistics_Check(_this))                                              /* Check instance data               */
    return IERROR(_this,STA_NOTSETUP," ( use -status for details)",0,0);        /* ...                               */

  /* Initialize */                                                              /* --------------------------------- */
  C = CStatistics_GetNClasses(_this);                                           /* Get number of statistics classes  */
  N = CStatistics_GetDim(_this);                                                /* Get statistics dimensionality     */
  CData_Array(idDst,T_DOUBLE,N,C*N);                                            /* Allocate output data instance     */
  CData_SetNBlocks(idDst,C);                                                    /* Set block number                  */
  if (!CData_XAddr(idDst,0,0)) return IERROR(_this,ERR_NOMEM,0,0,0);            /* Should have been successfull ...  */

  /* Loop over classes */                                                       /* --------------------------------- */
  for (c=0; c<C; c++)                                                           /* Loop over classes                 */
  {                                                                             /* >>                                */
    DLPASSERT((lpSsz = CStatistics_GetPtr(_this,c,STA_DAI_SSIZE)));             /*   Get ptr. to class' c sample size*/
    DLPASSERT((lpSum = CStatistics_GetPtr(_this,c,STA_DAI_SUM  )));             /*   Get ptr. to class' c sum vector */
    DLPASSERT((lpMsm = CStatistics_GetPtr(_this,c,STA_DAI_MSUM )));             /*   Get ptr. to class' c mix. sum v.*/
    lpY = (FLOAT64*)CData_XAddr(idDst,c*N,0);                                    /*   Get pointer to output data vect.*/
    for (n=0,i=0; n<N; n++)                                                     /*   Loop over dimensions n          */
      for (m=0; m<N; m++,i++)                                                   /*     Loop over dimensions m        */
        if (nMode==STA_CMA_SCAT) lpY[i] = lpSsz[0]>0. ? lpMsm[i]/lpSsz[0] : 0.; /*       Scatter                     */
        else                                                                    /*       All other parameters        */
        {                                                                       /*       >>                          */
          lpY[i] = CStatistics_Cov(_this,lpMsm,lpSum,N,lpSsz[0],n,m);           /*         Estimate covariance       */
          if (nMode==STA_CMA_CORC || nMode==STA_CMA_DETC)                       /*         "Coefficients"            */
          {                                                                     /*         >>                        */
            nVarN = CStatistics_Cov(_this,lpMsm,lpSum,N,lpSsz[0],n,n);          /*           Est. variance of dim. n */
            nVarM = CStatistics_Cov(_this,lpMsm,lpSum,N,lpSsz[0],m,m);          /*           Est. variance of dim. m */
            lpY[i] /= sqrt(nVarN)*sqrt(nVarM);                                  /*           Correlation coefficent  */
            if (nMode==STA_CMA_DETC) lpY[i] *= lpY[i];                          /*           Coeff. of determination */
          }                                                                     /*         <<                        */
        }                                                                       /*       <<                          */
  }                                                                             /* <<                                */

  return O_K;
}
/* EOF */
