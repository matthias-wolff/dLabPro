/* dLabPro class CGmm (gmm)
 * - Auxilary and information methods
 *
 * AUTHOR : Matthias Wolff
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
#include "dlp_gmm.h"                                                            /* Include class header file         */

/**
 * Returns the code of the currently used floating point data type. The type is
 * defined by the first numeric component in the mean vector table
 * <code>idMean</code>.
 * 
 * @param _this
 *          Pointer to CGmm instance
 * @param idMean
 *          Mean vector table, the first numeric component determines the type.
 *          May be <code>NULL</code>; in this case the method will use the
 *          field {@link mean}.
 * @return dLabPro type code of floating point type used by this instance,
 *         <code>T_FLOAT</code> or <code>T_DOUBLE</code>
 */
INT16 CGEN_PUBLIC CGmm_GetTypeEx(CGmm* _this, CData* idMean)
{
  INT32 i = 0;
  CHECK_THIS_RV(T_DOUBLE);

  if (!idMean && _this->m_idMean) idMean = AS(CData,_this->m_idMean);
  for (i=0; i<CData_GetNComps(idMean); i++)
  {
    INT16 nType = CData_GetCompType(idMean,i);
    if (nType==T_DOUBLE) return T_DOUBLE;
    if (nType==T_FLOAT ) return T_FLOAT;
  }
  return T_DOUBLE;
}

/**
 * <p>Returns the code of the currently used floating point data type. The type
 * is defined by the first numeric component in the mean vector table (see field
 * {@link mean}).</p>
 * 
 * @param _this
 *          Pointer to CGmm instance
 * @return dLabPro type code of floating point type used by this instance,
 *         <code>T_FLOAT</code> or <code>T_DOUBLE</code>
 */
INT16 CGEN_PUBLIC CGmm_GetType(CGmm* _this)
{
  return CGmm_GetTypeEx(_this,NULL);
}

/**
 * Returns the feature space dimensionality N. The dimensionality is defined by
 * the number of components in the mean vector table {@link mean m_idMean}.
 * 
 * @param _this
 *          Pointer to CGmm instance
 * @return The feature space dimensionality or 0 in case of errors
 */
INT32 CGEN_PUBLIC CGmm_GetDim(CGmm* _this)
{
  CHECK_THIS_RV(0);
  return CData_GetNComps(AS(CData,_this->m_idMean));
}

/**
 * Returns the number of Gaussian mixture models M.
 * 
 * @param _this
 *          Pointer to CGmm instance
 * @return The number of mixture models or 0 in case of errors
 */
INT32 CGEN_PUBLIC CGmm_GetNMix(CGmm* _this)
{
  CHECK_THIS_RV(0);
  if (_this->m_iMmap) return CVmap_GetOutDim(AS(CVmap,_this->m_iMmap));
  return CGmm_GetNGauss(_this);
}

/**
 * <p>Returns the number of single Gaussians K. This number is defined by the
 * number of records in the mean vector table {@link mean m_idMean}.</p>
 * <h4>Remarks</h4>
 * <ul>
 *   <li>One or several of the single Gaussians make up a Gaussian mixture. To
 *   determine the number of Gaussian mixtures use CGmm_GetNModels.</li>
 *   <li>Several Gaussians may share one covariance matrix. To determine the
 *   number of different covariance matrices use {@link GetNIcov CGmm_GetNIcov}
 *   </li>
 * </ul>
 * @see GetNValidGauss CGmm_GetNValidGauss
 * 
 * @param _this
 *          Pointer to CGmm instance
 * @return The number of single Gaussians or 0 in case of errors
 */
INT32 CGEN_PUBLIC CGmm_GetNGauss(CGmm* _this)
{
  CHECK_THIS_RV(0);
  return CData_GetNRecs(AS(CData,_this->m_idMean));
}

/*
 * Manual page at gmm.def
 */
INT32 CGEN_PUBLIC CGmm_GetNValidGauss(CGmm* _this)
{
  INT32 k      = 0;
  INT32 K      = 0;
  INT32 nValid = 0;
  
  CHECK_THIS_RV(0);
  K = CGmm_GetNGauss(_this);
  if (K==0) return 0;

  for (k=0,nValid=0; k<K; k++)
    IF_OK(CGmm_IsValidGauss(_this,k)) 
      nValid++;

  return nValid;
}

/**
 * <p>Returns the number of inverse covariance matrices C. This number is
 * defined by the number of blocks in the inverse convariance table
 * {@link mean m_idIcov}.</p>
 * <h4>Remarks</h4>
 * <p>Several Gaussians may share one covariance matrix. To determine the
 * number of Gaussians use {@link GetNGauss CGmm_GetNGauss}.</p>
 * 
 * @param _this
 *          Pointer to CGmm instance
 * @return The number of single Gaussians or 0 in case of errors
 */
INT32 CGEN_PUBLIC CGmm_GetNIcov(CGmm* _this)
{
  CHECK_THIS_RV(0);
  return _this->m_idIcov ? CData_GetNRecs(AS(CData,_this->m_idIcov)) : 0;
}

/**
 * Returns the maximal Mahalanobis distance or the minimal (logarithmic)
 * probability.
 * 
 * @param _this
 *          Pointer to CGmm instance
 * @param nMode
 *          Operation mode, one of the GMMG_XXX constants
 */
FLOAT64 CGEN_PUBLIC CGmm_GetLimit(CGmm* _this, INT16 nMode)
{
  switch (nMode)                                                                /* Branch for operation mode         */
  {                                                                             /* >>                                */
    case GMMG_MDIST : return _this->m_nDceil;                                   /*   Maximal distance                */
    case GMMG_LDENS : return -0.5*_this->m_nDceil;                              /*   Minimal log. probability density*/
    case GMMG_NLDENS: return 0.5*_this->m_nDceil;                               /*   Maximal neg. log. prob. density */
    case GMMG_DENS  : return 0.;                                                /*   Minimal probability density     */
    default         : DLPASSERT(FMSG("Invalid operation mode"));                /*   nMode defective                 */
  }                                                                             /* <<                                */
  return 0.;
}

/**
 * <p>Determines if a single Gaussian is valid. A Gaussian is valid if it
 * has a mean vector, an inverse variance vector and either no inverse
 * covariance matrix or a covariance matrix of full rank.</p>
 * @see GetNValidGauss CGmm_GetNValidGauss
 * 
 * @param _this
 *          Pointer to CGmm instance
 * @param k
 *          Gaussian index
 * @return <code>O_K</code> if the Gaussian is valid, a negative error code
 *         otherwise: <code>NOT_EXEC</code> if <code>k</code> is not a valid
 *         Gaussian index, <code>GMM_RANK</code> if the covariance matrix has
 *         no full rank.
 */
INT16 CGEN_PUBLIC CGmm_IsValidGauss(CGmm* _this, INT32 k)
{
  CHECK_THIS_RV(NOT_EXEC);                                                      /* Check this pointer                */
  if (k<0 || k>=CGmm_GetNGauss(_this)                ) return NOT_EXEC;         /* Check Gaussian index              */
  if (k>=CData_GetNRecs(AS(CData,_this->m_idMean))   ) return NOT_EXEC;         /* Has no mean vector -> invalid     */
  if (k>=CData_GetNRecs(AS(CData,_this->m_idIvar))   ) return NOT_EXEC;         /* Has no inv.var. vector -> invalid */
  if (CData_IsEmpty(AS(CData,_this->m_idIcov))       ) return O_K;              /* Has no covariance matrix -> ok    */
  if (CData_Dfetch(AS(CData,_this->m_idCdet),k,0)==0.) return GMM_RANK;         /* Cov.matr. no full rank -> invalid */
  return O_K;
}

/**
 * Checks the Gaussian mean vectors (field m_idMean) for presence and data type.
 * 
 * @param _this
 *           Pointer to CGmm instance
 * @return <code>O_K</code> if check successfull, a (negative) error code
 *         otherwise
 * @see mean m_idMean
 */
INT16 CGEN_PROTECTED CGmm_CheckMean(CGmm* _this)
{
  INT16 nType = 0;
  
  CHECK_THIS_RV(NOT_EXEC);
  IFCHECK printf("\n   Checking Gaussian mean vectors ...");
  IFCHECK printf("\n   - mean (mean vector sequence) : ");
  if (!_this->m_idMean)
  {
    IFCHECK printf("not present -> INVALID");
    return NOT_EXEC;
  }
  IFCHECK printf("present");
  nType = CData_IsHomogen(AS(CData,_this->m_idMean));
  IFCHECK  printf("\n     - Data type                 : %ld ",(long)nType);
  if (nType!=T_FLOAT && nType!=T_DOUBLE)
  {
    IFCHECK
    {
      if (nType==0) printf("NOT HOMOGENOUS -> INVALID");
      else          printf("BAD TYPE, should be a floating type");
    }
    return NOT_EXEC;
  }
   IFCHECK printf("ok");
  IFCHECK printf("\n   - Master floating point type  : %ld ",(long)_this->m_nType);
  if (_this->m_nType!=CGmm_GetType(_this))
  {
    IFCHECK printf("WRONG TYPE, should be %ld",(long)CGmm_GetType(_this));
    return NOT_EXEC;
  }
   IFCHECK printf("ok");
  
  return O_K;
}

/**
 * Checks the Gaussian inverse variance vectors (field m_idIvar) for presence,
 * data type and consistency.
 * 
 * @param _this
 *           Pointer to CGmm instance
 * @return <code>O_K</code> if check successfull, a (negative) error code
 *         otherwise
 * @see ivar m_idIvar
 */
INT16 CGEN_PROTECTED CGmm_CheckIvar(CGmm* _this)
{
  INT32  nCols = 0;
  INT32  nRows = 0;
  INT16 nType = 0;

  CHECK_THIS_RV(NOT_EXEC);
  IFCHECK printf("\n   Checking Gaussian inverse variance vectors ...");
  IFCHECK printf("\n   - Master floating point type  : %ld ",(long)_this->m_nType);
  if (_this->m_nType!=CGmm_GetType(_this))
  {
    IFCHECK printf("WRONG TYPE, should be %ld",(long)CGmm_GetType(_this));
    return NOT_EXEC;
  }
   IFCHECK printf("ok");
  IFCHECK printf("\n   - ivar (inv. variance vects.) : ");
  if (!_this->m_idIvar)
  {
    IFCHECK printf("not present -> INVALID");
    return NOT_EXEC;
  }

  IFCHECK printf("present");
  nCols = CData_GetNComps(AS(CData,_this->m_idIvar));
  nRows = CData_GetNRecs (AS(CData,_this->m_idIvar));
  IFCHECK  printf("\n     - Number of components      : %ld ",(long)nCols);
  if (nCols!=CGmm_GetDim(_this))
  {
    IFCHECK printf("NOT OK, should be %ld",(long)CGmm_GetDim(_this));
    return NOT_EXEC;
  }
   IFCHECK printf("ok");
  IFCHECK  printf("\n     - Number of vectors         : %ld ",(long)nRows);
  if (nRows!=CGmm_GetNGauss(_this))
  {
    IFCHECK printf("NOT OK, should be %ld",(long)CGmm_GetNGauss(_this));
    return NOT_EXEC;
  }
   IFCHECK printf("ok");
  nType = CData_IsHomogen(AS(CData,_this->m_idIvar));
  IFCHECK  printf("\n     - Data type                 : %ld ",(long)nType);
  if (nType!=_this->m_nType)
  {
    IFCHECK
    {
      if (nType==0) printf("NOT HOMOGENOUS -> INVALID");
      else          printf("BAD TYPE, should be %ld",(long)_this->m_nType);
    }
    return NOT_EXEC;
  }
   IFCHECK printf("ok");
  
  return O_K;
}

/**
 * <p>Checks the determinants (field m_idCdet) of the (co-)variance matrices for
 * presence, data type and consistency.</p>
 * <h4>Remarks</h4>
 * <p>The method does <em>not</em> check if the determinants are
 * <em>correct</em>!</p>
 * 
 * @param _this
 *           Pointer to CGmm instance
 * @return <code>O_K</code> if check successfull, a (negative) error code
 *         otherwise
 * @see ivar m_idCdet
 */
INT16 CGEN_PROTECTED CGmm_CheckCdet(CGmm* _this)
{
  INT32  nCols = 0;
  INT32  nRows = 0;
  INT16 nType = 0;

  CHECK_THIS_RV(NOT_EXEC);
  IFCHECK printf("\n   Checking determinants of covariance matrices ...");
  IFCHECK printf("\n   - Master floating point type  : %ld ",(long)_this->m_nType);
  if (_this->m_nType!=CGmm_GetType(_this))
  {
    IFCHECK printf("WRONG TYPE, should be %ld",(long)CGmm_GetType(_this));
    return NOT_EXEC;
  }
   IFCHECK printf("ok");
  IFCHECK printf("\n   - cdet (cov.mat. determinants): ");
  if (!_this->m_idCdet)
  {
    IFCHECK printf("not present -> INVALID");
    return NOT_EXEC;
  }

  IFCHECK printf("present");
  nCols = CData_GetNComps(AS(CData,_this->m_idCdet));
  nRows = CData_GetNRecs (AS(CData,_this->m_idCdet));
  IFCHECK  printf("\n     - Number of components      : %ld ",(long)nCols);
  if (nCols!=1)
  {
    IFCHECK printf("NOT OK, should be 1");
    return NOT_EXEC;
  }
   IFCHECK printf("ok");
  IFCHECK  printf("\n     - Number of vectors         : %ld ",(long)nRows);
  if (nRows!=CGmm_GetNGauss(_this))
  {
    IFCHECK printf("NOT OK, should be %ld",(long)CGmm_GetNGauss(_this));
    return NOT_EXEC;
  }
   IFCHECK printf("ok");
  nType = CData_IsHomogen(AS(CData,_this->m_idCdet));
  IFCHECK  printf("\n     - Data type                 : %ld ",(long)nType);
  if (nType!=_this->m_nType)
  {
    IFCHECK
    {
      if (nType==0) printf("NOT HOMOGENOUS -> INVALID");
      else          printf("BAD TYPE, should be %ld",(long)_this->m_nType);
    }
    return NOT_EXEC;
  }
   IFCHECK printf("ok");
  
  return O_K;
}

/**
 * Checks the inverse covariance matrices (field m_idIcov) and the
 * (co)variance tying map (field m_idCmap) for data type, consistency and
 * plausibility. The method calls {@link CGmm_CheckCmap}.
 * 
 * @param _this
 *           Pointer to CGmm instance
 * @return <code>O_K</code> if check successfull, a (negative) error code
 *         otherwise
 * @see CGmm_CheckCmap
 * @see icov m_idIcov
 * @see cmap m_idCmap
 */
INT16 CGEN_PROTECTED CGmm_CheckIcov(CGmm* _this)
{
  INT32  N     = 0;
  INT32  nCols = 0;
  INT16 nType = 0;
  
  CHECK_THIS_RV(NOT_EXEC);
  N = CGmm_GetDim(_this);
  IFCHECK printf("\n   Checking inverse covariance matrices ...");
  if (!_this->m_idIcov)
  {
    IFCHECK printf("\n   - icov (inverse cov. matrices): not present");
    if (_this->m_idCmap && !CData_IsEmpty(AS(CData,_this->m_idCmap)))
    {
      IFCHECK
      {
        printf("\n   - cmap (covariance tying map)   : present -> INCONSISTENT!");
        printf("\n   - FAILED");
        return NOT_EXEC;
      }
    }
    IFCHECK printf("\n   - OK");
    return O_K;
  }
  
  IFCHECK printf("\n   - Master floating point type  : %ld ",(long)_this->m_nType);
  if (_this->m_nType!=CGmm_GetType(_this))
  {
    IFCHECK printf("WRONG TYPE, should be %ld",(long)CGmm_GetType(_this));
    return NOT_EXEC;
  }
   IFCHECK printf("ok");
  IFCHECK printf("\n   - icov (inverse cov. matrices): present");
  nCols = CData_GetNComps(AS(CData,_this->m_idIcov));
  IFCHECK  printf("\n     - Number of elements        : %ld ",(long)nCols);
  if (nCols!=N*(N-1)/2)
  {
    IFCHECK printf("NOT OK, should be %ld x (%ld-1) / 2",(long)N,(long)N);
    return NOT_EXEC;
  }
  IFCHECK printf("ok");
  IFCHECK  printf("\n     - Number of matrices        : %ld ",(long)CGmm_GetNIcov(_this));
  nType = CData_IsHomogen(AS(CData,_this->m_idIcov));
  IFCHECK  printf("\n     - Data type                 : %ld ",(long)nType);
  if (nType!=_this->m_nType)
  {
    IFCHECK
    {
      if (nType==0) printf("NOT HOMOGENOUS -> INVALID");
      else          printf("BAD TYPE, should be %ld",(long)_this->m_nType);
    }
    return NOT_EXEC;
  }
   IFCHECK printf("ok");

  return CGmm_CheckCmap(_this);
}

/**
 * Checks the mixture map (field m_iMmap) for consistency.
 * 
 * @param _this
 *           Pointer to CGmm instance
 * @return <code>O_K</code> if check successfull, a (negative) error code
 *         otherwise
 * @see mmap m_iMmap
 */
INT16 CGEN_PROTECTED CGmm_CheckMmap(CGmm* _this)
{
  CHECK_THIS_RV(NOT_EXEC);
  IFCHECK printf("\n   Checking mixture map ...");
  IFCHECK printf("\n   - mmap (mixture map)          : ");
  if (!_this->m_iMmap)
  {
    IFCHECK printf("not present, 1-by-1 mapping, ok");
    return O_K;
  }
  IFCHECK printf("present ");
  IFCHECK  printf("\n     - Map size                  : %ld ",
  (long)CVmap_GetInDim(AS(CVmap,_this->m_iMmap)));
  if (CVmap_GetInDim(AS(CVmap,_this->m_iMmap))!=CGmm_GetNGauss(_this))
  {
    IFCHECK printf("NOT OK, should be %ld",(long)CGmm_GetNGauss(_this));
    return NOT_EXEC;
  }
  IFCHECK printf(", ok");
  return O_K;
}

/**
 * Checks the covariance tying map (field m_idCmap) for data type, consistency
 * and plausibility.
 * 
 * @param _this
 *           Pointer to CGmm instance
 * @return <code>O_K</code> if check successfull, a (negative) error code
 *         otherwise
 * @see cmap m_idCmap
 */
INT16 CGEN_PROTECTED CGmm_CheckCmap(CGmm* _this)
{
  INT32 i     = 0;
  INT32 nRows = 0;
  
  CHECK_THIS_RV(NOT_EXEC);
  IFCHECK printf("\n   Checking covariance tying map ...");
  IFCHECK printf("\n   - cmap (covariance tying map) : ");
  if (!_this->m_idCmap)
  {
    IFCHECK printf("not present, ");
    if (CGmm_GetNIcov(_this)!=CGmm_GetNGauss(_this))
    {
      IFCHECK printf("WRONG NUMBER OF INV. COV. MATRICES, should be %ld",
      (long)CGmm_GetNGauss(_this));
      return NOT_EXEC;
    }
    IFCHECK printf("1-by-1 mapping, ok");
  }
  else
  {
    IFCHECK printf("present ");
    IFCHECK  printf("\n     - Index component type long : ");
    if (CData_GetCompType(AS(CData,_this->m_idCmap),0)!=T_LONG)
    {
      IFCHECK printf("NO --> INVALID");
      return NOT_EXEC;
    }
    IFCHECK printf("yes, ok");
    nRows = CData_GetNRecs(AS(CData,_this->m_idCmap));
    IFCHECK  printf("\n     - Map size                  : %ld ",(long)nRows);
    if (nRows!=CGmm_GetNGauss(_this))
    {
      IFCHECK printf("NOT OK, should be %ld",(long)CGmm_GetNGauss(_this));
      return NOT_EXEC;
    }
    IFCHECK printf(", ok");
    IFCHECK  printf("\n     - Checking map items        : ");
    for (i=0; i<nRows; i++)
    {
      INT32 nIdx = (INT32)CData_Dfetch(AS(CData,_this->m_idCmap),i,0);
      if (nIdx<0 || nIdx>=CGmm_GetNIcov(_this))
      {
        IFCHECK printf("bad item #%ld (=%ld, should be >=0 and <%ld",(long)i,(long)nIdx,(long)CGmm_GetNIcov(_this));
        return NOT_EXEC;
      }
      IFCHECK printf(" ok");
    }
  }

  return O_K;
}

/**
 * Checks the precalculated data for Gaussian distance/density computation.
 * 
 * @param _this
 *           Pointer to CGmm instance
 * @return <code>O_K</code> if check successfull, a (negative) error code
 *         otherwise
 * @see alpha m_lpAlpha
 * @see beta  m_lpBeta
 * @see gamma m_lpGamma
 * @see delta m_lpDelta
 * @see I     m_lpI
 * @see V     m_lpV
 */
INT16 CGEN_PROTECTED CGmm_CheckPrecalc(CGmm* _this)
{
  INT16 s     = 0;
  INT32 K     = 0;
  INT32 N     = 0;
  INT32 nSize = 0;
  
  CHECK_THIS_RV(NOT_EXEC);
  s = dlp_get_type_size(_this->m_nType);
  K = CGmm_GetNGauss(_this);
  N = CGmm_GetDim(_this);

  IFCHECK printf("\n   Checking precalculated data ...");
  IFCHECK printf("\n   - Feature space dimensionality: %ld ",(long)_this->m_nN);
  if (_this->m_nN!=N)
  {
    IFCHECK printf("WRONG, should be %ld",(long)N);
    return NOT_EXEC;
  }
  IFCHECK printf("ok");
  IFCHECK printf("\n   - Number of single Gaussians  : %ld ",(long)_this->m_nK);
  if (_this->m_nK!=K)
  {
    IFCHECK printf("WRONG, should be %ld",(long)K);
    return NOT_EXEC;
  }
  IFCHECK printf("ok");
  IFCHECK printf("\n   - Floating point type         : %ld ",(long)_this->m_nType);
  if (_this->m_nType!=CGmm_GetType(_this))
  {
    IFCHECK printf("WRONG TYPE, should be %ld",(long)CGmm_GetType(_this));
    return NOT_EXEC;
  }
  IFCHECK printf("ok");
  IFCHECK printf("\n   - Floating point type size    : %hd bytes",(short)s);
  if(_this->m_nLDL)
  {
    nSize = dlp_size(_this->m_lpLdlL);
    IFCHECK printf("\n   - Length of LDL L matrix      : %ld bytes, ",(long)nSize);
    if (nSize!=K*N*(N-1)/2*s)
    {
      IFCHECK printf("WRONG SIZE, should be %ld x %ld x %hd",(long)(N*(N-1)/2),(long)K,(short)s);
      return NOT_EXEC;
    }
    IFCHECK printf("ok");
    nSize = dlp_size(_this->m_lpLdlD);
    IFCHECK printf("\n   - Length of LDL D vector      : %ld bytes, ",(long)nSize);
    if (nSize!=K*N*s)
    {
      IFCHECK printf("WRONG SIZE, should be %ld x %ld x %hd",(long)N,(long)K,(short)s);
      return NOT_EXEC;
    }
    IFCHECK printf("ok");
  }
  else
  {
    nSize = dlp_size(_this->m_lpAlpha);
    IFCHECK printf("\n   - Length of alpha vector      : %ld bytes, ",(long)nSize);
    if (nSize!=K*s)
    {
      IFCHECK printf("WRONG SIZE, should be 1 x %ld x %hd",(long)K,(short)s);
      return NOT_EXEC;
    }
    IFCHECK printf("ok");
  }
  nSize = dlp_size(_this->m_lpBeta);
  IFCHECK printf("\n   - Length of beta vectors      : %ld bytes, ",(long)nSize);
  if (nSize!=K*N*s)
  {
    IFCHECK printf("WRONG SIZE, should be %ld x %ld x %hd",(long)K,(long)N,(short)s);
    return NOT_EXEC;
  }
  IFCHECK printf("ok");
  IFCHECK printf("\n   - Length of gamma vector      : ");
  if (_this->m_lpGamma)
  {
    nSize = dlp_size(_this->m_lpGamma);
    IFCHECK printf("%ld bytes, ",(long)nSize);
    if (nSize!=K*s)
    {
      IFCHECK printf("WRONG SIZE, should be 1 x %ld x %hd",(long)K,(short)s);
      return NOT_EXEC;
    }
  }
  else IFCHECK printf("not present, ");
  IFCHECK printf("ok");
  nSize = dlp_size(_this->m_lpDelta);
  IFCHECK printf("\n   - Length of delta vector      : %ld bytes, ",(long)nSize);
  if (nSize!=K*s)
  {
    IFCHECK printf("WRONG SIZE, should be 1 x %ld x %hd",(long)K,(short)s);
    return NOT_EXEC;
  }
  IFCHECK printf("ok");
  nSize = dlp_size(_this->m_lpI);
  IFCHECK printf("\n   - Length of I vector          : %ld bytes, ",(long)nSize);
  if (_this->m_idIcov)
  {
    if (nSize!=K*(INT32)sizeof(void*))
    {
      IFCHECK printf("WRONG SIZE, should be 1 x %ld x %hd",(long)K,(short)sizeof(void*));
      return NOT_EXEC;
    }
    else
    {
      BYTE* lp1  = NULL;
      INT32  nLen = 0;
      INT32  k    = 0;

      IFCHECK printf("ok");
      IFCHECK printf("\n   - Checking ptrs. of I vector  : ");
      lp1  = (BYTE*)CData_XAddr(AS(CData,_this->m_idIcov),0,0);
      nLen = dlp_size(lp1);
      if (lp1==NULL || nLen==0)
      {
        IFCHECK printf("inv. cov. (field icov) table invalid -> FAIULURE");
        return NOT_EXEC;
      }
      for (k=0; k<K; k++)
        if (((BYTE**)_this->m_lpI)[k]<lp1 ||((BYTE**)_this->m_lpI)[k]>=lp1+nLen)
        {
          IFCHECK printf("pointer #%ld invalid -> FAILURE",(long)k);
          return NOT_EXEC;
        }
      IFCHECK printf("ok");
    }
  }
  else
  {
    if (_this->m_lpI)
    {
      IFCHECK printf(" no inv. cov. matrices (field icov) -> INCONSISTENT");
      return NOT_EXEC;
    }
    else IFCHECK printf("ok (no covariance matrices present)");
  }
  nSize = dlp_size(_this->m_lpV);
  IFCHECK printf("\n   - Length of V vector          : %ld bytes, ",(long)nSize);
  if (nSize!=K*(INT32)sizeof(void*))
  {
    IFCHECK printf("WRONG SIZE, should be 1 x %ld x %hd",(long)K,(short)sizeof(void*));
    return NOT_EXEC;
  }
  else
  {
    BYTE* lp1  = NULL;
    INT32  nLen = 0;
    INT32  k    = 0;

    IFCHECK printf("ok");
    IFCHECK printf("\n   - Checking ptrs. of V vector  : ");
    lp1  = (BYTE*)CData_XAddr(AS(CData,_this->m_idIvar),0,0);
    nLen = dlp_size(lp1);
    if (lp1==NULL || nLen==0)
    {
      IFCHECK printf("inv. cov. (field ivar) table invalid -> FAIULURE");
      return NOT_EXEC;
    }
    for (k=0; k<K; k++)
      if (((BYTE**)_this->m_lpV)[k]<lp1 || ((BYTE**)_this->m_lpV)[k]>=lp1+nLen)
      {
        IFCHECK printf("pointer #%ld invalid -> FAILURE",(long)k);
        return NOT_EXEC;
      }
  }
  IFCHECK printf("ok");

#ifdef GMM_SSE2
{
  INT16 nType = 0;
  IFCHECK printf("\n   Checking SSE2 precalculated data ...");
  IFCHECK printf("\n   - Inverse covariance matrices : ");
  if (CData_IsEmpty(_this->m_idSse2Icov))
  {
    IFCHECK printf(" not present, FAILURE");
    return NOT_EXEC;
  }
  nSize = CData_GetNRecs(_this->m_idSse2Icov) * 
    CData_GetNComps(_this->m_idSse2Icov);
  IFCHECK  printf("\n     - Size                      : %ld bytes ",(long)nSize);
  if (nSize!=K*N*N)
  {
    IFCHECK printf("NOT OK, should be %ld x %ld x %ld",(long)K,(long)N,(long)N);
    return NOT_EXEC;
  }
  IFCHECK printf("ok");
  nType = CData_IsHomogen(_this->m_idSse2Icov);
  IFCHECK  printf("\n     - Data type                 : %ld ",(long)nType);
  if (nType!=_this->m_nType)
  {
    IFCHECK
    {
      if (nType==0) printf("NOT HOMOGENOUS -> INVALID");
      else          printf("BAD TYPE, should be %ld",(long)_this->m_nType);
    }
    return NOT_EXEC;
  }
   IFCHECK printf("ok");
  nSize = dlp_size(_this->m_lpSse2Buf);
  IFCHECK printf("\n   - Auxilary buffer size        : %ld bytes ",(long)nSize);
  if (nSize!=2*N*s)
  {
    IFCHECK printf("NOT OK, should be 2 x %ld x %ld",(long)N,(long)N);
    return NOT_EXEC;
  }
  IFCHECK printf("ok");
}
#endif                                                                          /* #ifdef GMM_SSE2                   */
  
  return O_K;
}

/**
 * Performs a checkup of the CGmm instances internal data. The method calls
 * {@link CheckMean CGmm_CheckMean}, {@link CheckIvar CGmm_CheckIvar}, 
 * {@link CheckIcov CGmm_CheckICov}, {@link CheckCdet CGmm_CheckCdet},
 * {@link CheckMmap CGmm_CheckMmap} and {@link CheckPrecalc CGmm_CheckPrecalc}.
 * 
 * @param _this
 *           Pointer to CGmm instance
 * @return <code>O_K</code> if check successfull, a (negative) error code
 *         otherwise
 */
INT16 CGEN_PUBLIC CGmm_Check(CGmm* _this)
{
  CHECK_THIS_RV(NOT_EXEC);
  IF_NOK(CGmm_CheckMean   (_this)) return NOT_EXEC;
  IF_NOK(CGmm_CheckIvar   (_this)) return NOT_EXEC;
  IF_NOK(CGmm_CheckIcov   (_this)) return NOT_EXEC;
  IF_NOK(CGmm_CheckCdet   (_this)) return NOT_EXEC;
  IF_NOK(CGmm_CheckMmap   (_this)) return NOT_EXEC;
#ifndef __NOXALLOC  
  IF_NOK(CGmm_CheckPrecalc(_this)) return NOT_EXEC;
#endif
  return O_K;
}

/*
 * Manual page at gmm.def
 */
INT16 CGEN_PUBLIC CGmm_Status(CGmm* _this)
{
  INT16 nS1 = O_K;
  INT16 nS2 = O_K;
  INT16 nS3 = O_K;
  INT16 nS4 = O_K;
  INT16 nS5 = O_K;
  INT16 nS6 = O_K;
  BOOL  bOk = FALSE;

  CHECK_THIS_RV(NOT_EXEC);
  printf("\n"); dlp_fprint_x_line(stdout,'-',dlp_maxprintcols());
  printf("\n   Status of instance\n   gmm %s",BASEINST(_this)->m_lpInstanceName);
  printf("\n"); dlp_fprint_x_line(stdout,'-',dlp_maxprintcols());
  printf("\n   Floating point type           : %ld",(long)CGmm_GetType       (_this));
  printf(" (%s)",CGmm_GetType(_this)==T_FLOAT?"float":"double"                      );
  printf("\n   SSE2 available                : %s" ,CGmm_Sse2()?"yes":"no"          );
  printf("\n   Distance / log. density limit : %g",(double)_this->m_nDceil          );
  printf(" / %g"                                  ,(double)(-0.5*_this->m_nDceil)   );
  printf("\n   Feature space dimensionality  : %ld",(long)CGmm_GetDim        (_this));
  printf("\n   Number of mixture models      : %ld",(long)CGmm_GetNMix       (_this));
  printf("\n   Number of single Gaussians    : %ld",(long)CGmm_GetNGauss     (_this));
  printf("\n   - Valid                       : %ld",(long)CGmm_GetNValidGauss(_this));
  printf("\n   Number of covariance matrices : %ld",(long)CGmm_GetNIcov      (_this));
  if (_this->m_idCmap)
  {
    printf(" (tied)\n   Variance vectors tied         : ");
    printf(CData_GetDescr(AS(CData,_this->m_idIvar),0)?"no":"yes");
  }
  printf("\n");
  nS1 = CGmm_CheckMean(_this);
  nS2 = CGmm_CheckIvar(_this);
  nS3 = CGmm_CheckIcov(_this);
  nS4 = CGmm_CheckCdet(_this);
  nS5 = CGmm_CheckMmap(_this);
  nS6 = CGmm_CheckPrecalc(_this);
  if (BASEINST(_this)->m_nCheck==0)
  {
    printf("\n   Mean vectors                  : %s",OK(nS1)?"ok":"DEFECTIVE");
    printf("\n   Inverse variance vectors      : %s",OK(nS2)?"ok":"DEFECTIVE");
    printf("\n   Inverse covariance matrices   : %s",OK(nS3)?"ok":"DEFECTIVE");
    printf("\n   Determinants of cov. matrices : %s",OK(nS4)?"ok":"DEFECTIVE");
    printf("\n   Mixture map                   : %s",OK(nS5)?"ok":"DEFECTIVE");
    printf("\n   Precalculated data            : %s",OK(nS6)?"ok":"DEFECTIVE");
  }
  bOk = OK(nS1)&&OK(nS2)&&OK(nS3)&&OK(nS4)&&OK(nS5)&&OK(nS6);
  printf("\n\n   Gaussians are %s.",bOk?"correctly setup":"CORRUPT");
  if (CGmm_GetNValidGauss(_this)<CGmm_GetNGauss(_this))
    printf("\n   There are %ld INVALID Gaussians.",
    (long)CGmm_GetNGauss(_this)-CGmm_GetNValidGauss(_this));
  printf("\n"); dlp_fprint_x_line(stdout,'-',dlp_maxprintcols());
  if (BASEINST(_this)->m_nCheck==0 && !bOk)
    printf("\n   Type <1 %s -set check; -status;> for more details.",
      BASEINST(_this)->m_lpInstanceName);
  printf("\n");

  return O_K;
}

/* EOF */
