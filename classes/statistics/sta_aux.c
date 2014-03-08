/* dLabPro class CStatistics (statistics)
 * - Auxilary methods
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
 * Returns the statistics' dimensionality N. The dimensionality is defined by
 * the number of components in the statistics data table {@link dat m_idDat}.
 *
 * @param _this
 *          Pointer to CStatistics instance
 * @return The dimensionality or 0 in case of errors
 */
INT32 CGEN_PUBLIC CStatistics_GetDim(CStatistics* _this)
{
  CHECK_THIS_RV(0);
  return CData_GetNComps(_this->m_idDat);
}

/**
 * Returns the number of statistics classes (labels) C. The number of classes is
 * defined by the number of blocks in the statistics data table
 * {@link dat m_idDat}.
 *
 * @param _this
 *          Pointer to CStatistics instance
 * @return The number of classes or 0 in case of errors
 */
INT32 CGEN_PUBLIC CStatistics_GetNClasses(CStatistics* _this)
{
  CHECK_THIS_RV(0);
  if (CData_IsEmpty(_this->m_idDat)) return 0;
  return CData_GetNBlocks(_this->m_idDat);
}

/**
 * Returns the order of the statistics. The statistic's order <i>K</i> is the
 * greatest exponent <i>k</i> for which the statistics records sums up
 * <i>x<aup>k</sup></i>. The minimal order is two. The maximal order may be
 * specified using the {@link Setup CStatistics_Setup} method.
 *
 * @param _this
 *          Pointer to CStatistics instance
 * @return The order of the statistics or 0 in case of errors
 */
INT32 CGEN_PUBLIC CStatistics_GetOrder(CStatistics* _this)
{
  INT32 N    = 0;
  INT32 nRpb = 0;
  CHECK_THIS_RV(0);
  N    = CStatistics_GetDim(_this);
  nRpb = CData_GetNRecsPerBlock(_this->m_idDat);
  if (N<=0) return 0;
  return nRpb-N-2;
}

/**
 * Returns the total sample size over all classes.
 *
 * @param _this
 *          Pointer to CStatistics instance
 * @return The total sample size
 */
INT32 CGEN_PUBLIC CStatistics_GetNSamples(CStatistics* _this)
{
  INT32    c     = 0;                                                            /* Statistics class loop counter     */
  INT32    nSsz  = 0;                                                            /* Total sample size                 */
  FLOAT64* lpSsz = NULL;                                                         /* Ptr. to class' k sample size      */
  CHECK_THIS_RV(0);                                                             /* Check this pointer                */
  for (c=0; c<CStatistics_GetNClasses(_this); c++)                              /* Loop over statistics classes      */
  {                                                                             /* >>                                */
    lpSsz = CStatistics_GetPtr(_this,c,STA_DAI_SSIZE);                          /*   Get ptr. to class' k sample size*/
    if (lpSsz) nSsz += (INT32)*lpSsz;                                            /*   Sum up                          */
  }                                                                             /* <<                                */
  return nSsz;
}

/**
 * Returns a pointer to one of the data arrays of a statistics' class
 *
 * @param _this
 *           Pointer to CStatistics instance
 * @param c
 *           Statistics class index
 * @param nData
 *           Data array identifier, one of the <code>STA_DAI_XXX</code>
 *           constants
 * @return The pointer to the first double value of the desired array and class
 *         or <code>NULL</code> in case of errors.
 */
FLOAT64* CGEN_PRIVATE CStatistics_GetPtr(CStatistics* _this, INT32 c, INT16 nData)
{
  INT32 N    = 0;                                                               /* Statistics dimensionality         */
  INT32 nRpb = 0;                                                               /* Records per block                 */
  CHECK_THIS_RV(NULL);                                                          /* Check this pointer                */
  N    = CStatistics_GetDim(_this);                                             /* Get statistics dimensionality     */
  nRpb = CData_GetNRecsPerBlock(_this->m_idDat);                                /* Get records per block             */
  if (nData<STA_DAI_KSUM)                                                       /* Before k-th order sums            */
    return (FLOAT64*)CData_XAddr(_this->m_idDat,c*nRpb + nData,0);              /*   Return pointer to data          */
  else if (nData==STA_DAI_KSUM)                                                 /* k-th order sums                   */
    return (FLOAT64*)CData_XAddr(_this->m_idDat,c*nRpb + N+4,0);                /*   Return pointer to data          */
  else return NULL;                                                             /* Cannot happen                     */
}

/**
 * Fills a data table with the sample size (frequency) or the estimated a-priori
 * probability of the classes.
 *
 * @param _this
 *          Pointer to this CStatistics instance
 * @param idDst
 *          Pointer to the destination data instance
 * @param bProb
 *          If <code>TRUE</code> the method estimates class a-priori
 *          probabilities otherwise it stores the classes' sample sizes
 * @return <code>O_K</code> if successsfull, a (negative) error code otherwise
 */
INT16 CGEN_PUBLIC CStatistics_FreqEx
(
  CStatistics* _this,
  CData*       idDst,
  BOOL         bProb
)
{
  INT32    c     = 0;                                                            /* Class loop counter                */
  INT32    C     = 0;                                                            /* Number of statistics classes      */
  INT32    nTsz  = 0;                                                            /* Total sample size                 */
  FLOAT64* lpSsz = NULL;                                                         /* Pointer to class' c sample size   */

  /* Validate */                                                                /* --------------------------------- */
  if (!idDst) return IERROR(_this,ERR_NULLARG,"idDst",0,0);                     /* Check output data instance        */
  CData_Reset(idDst,TRUE);                                                      /* Clear destination instance        */
  CHECK_THIS_RV(NOT_EXEC);                                                      /* Check this pointer                */
  IF_NOK(CStatistics_Check(_this))                                              /* Check instance data               */
    return IERROR(_this,STA_NOTSETUP," ( use -status for details)",0,0);        /* ...                               */

  /* Initialize */                                                              /* --------------------------------- */
  C = CStatistics_GetNClasses(_this);                                           /* Get number of statistics classes  */
  CData_Array(idDst,T_DOUBLE,1,C);                                              /* Allocate output data instance     */
  CData_SetNBlocks(idDst,C);                                                    /* Set block number                  */
  if (!CData_XAddr(idDst,0,0)) return IERROR(_this,ERR_NOMEM,0,0,0);            /* Should have been successfull ...  */

  /* Store sample sizes / estimated a-rpior probabilities */                    /* --------------------------------- */
  nTsz = bProb ? CStatistics_GetNSamples(_this) : 1;                            /* Get frequency divident            */
  for (c=0; c<C; c++)                                                           /* Loop over classes                 */
  {                                                                             /* >>                                */
    DLPASSERT((lpSsz = CStatistics_GetPtr(_this,c,STA_DAI_SSIZE)));             /*   Get ptr. to class' c sample size*/
    CData_Dstore(idDst,lpSsz[0]/(FLOAT64)nTsz,c,0);                              /*   Store sample size of class c    */
  }                                                                             /* <<                                */

  return O_K;
}

/**
 * Checks the statistics' data (field m_idDat) for presence and consistency.
 *
 * @param _this
 *           Pointer to CStatistics instance
 * @return <code>O_K</code> if check successfull, a (negative) error code
 *         otherwise
 * @see dat m_idDat
 */
INT16 CGEN_PROTECTED CStatistics_CheckDat(CStatistics* _this)
{
  INT16 nType = 0;
  INT32  N     = 0;
  INT32  C     = 0;
  INT32  nXR   = 0;

  CHECK_THIS_RV(NOT_EXEC);
  IFCHECK printf("\n   Checking statistics data ...");
  IFCHECK printf("\n   - dat (statistics data): ");
  if (!_this->m_idDat)
  {
    IFCHECK printf("not present -> INVALID");
    return NOT_EXEC;
  }
  IFCHECK printf("present");
  nType = CData_IsHomogen(_this->m_idDat);
  IFCHECK  printf("\n     - Data type          : %ld ",(long)nType);
  if (nType!=T_DOUBLE)
  {
    IFCHECK
    {
      if (nType==0) printf("NOT HOMOGENOUS -> INVALID");
      else          printf("BAD TYPE, should be double");
    }
    return NOT_EXEC;
  }
   IFCHECK printf("ok");
   nXR = CData_GetNRecs(_this->m_idDat);
  N   = CStatistics_GetDim(_this);
  C   = CStatistics_GetNClasses(_this);
  IFCHECK  printf("\n     - Number of records  : %ld ",(long)nXR);
  if (nXR<C*(N+4))
  {
    IFCHECK printf("NOT OK, should be >= %ld*(%ld+4)",(long)C,(long)N);
    return NOT_EXEC;
  }
  IFCHECK printf("ok");

  return O_K;
}

/**
 * Checks the statistics' label table (field m_idLtb) for consistency.
 *
 * @param _this
 *           Pointer to CStatistics instance
 * @return <code>O_K</code> if check successfull, a (negative) error code
 *         otherwise
 * @see ltb m_idLtb
 */
INT16 CGEN_PROTECTED CStatistics_CheckLtb(CStatistics* _this)
{
  INT32 C     = 0;
  INT32 nType = -1;
  INT32 nXC   = 0;
  INT32 nXR   = 0;

  CHECK_THIS_RV(NOT_EXEC);
  IFCHECK printf("\n   Checking label table ...");
  IFCHECK printf("\n   - ltb (label table)    : ");
  if (!_this->m_idLtb)
  {
    IFCHECK printf("not present -> ok");
    return O_K;
  }
  IFCHECK printf("present");
   nXC = CData_GetNComps(_this->m_idLtb);
   nXR = CData_GetNRecs(_this->m_idLtb);
  C   = CStatistics_GetNClasses(_this);
  IFCHECK  printf("\n     - Size               : %ld x %ld ",(long)nXR,(long)nXC);
  if (nXR!=C || nXC!=1)
  {
    IFCHECK printf("NOT OK, should be %ld x 1",(long)C);
    return NOT_EXEC;
  }
   IFCHECK printf("ok");
  nType = CData_GetCompType(_this->m_idLtb,0);
  IFCHECK  printf("\n     - Data type          : %ld ",(long)nType);
  if (nType<=0 || nType>L_SSTR)
  {
    IFCHECK printf("NOT OK, should be a symbolic type");
    return NOT_EXEC;
  }
   IFCHECK printf("ok");
  return O_K;
}

/**
 * Performs a checkup of the CStatistics instances internal data. The method
 * calls {@link CheckDat CStatistics_CheckDat} and
 * {@link CheckLtb CStatistics_CheckLtb}.
 *
 * @param _this
 *           Pointer to CStatistics instance
 * @return <code>O_K</code> if check successfull, a (negative) error code
 *         otherwise
 */
INT16 CGEN_PUBLIC CStatistics_Check(CStatistics* _this)
{
  CHECK_THIS_RV(NOT_EXEC);
  IF_NOK(CStatistics_CheckDat(_this)) return NOT_EXEC;
  IF_NOK(CStatistics_CheckLtb(_this)) return NOT_EXEC;
  return O_K;
}

/*
 * Manual page at statistics.def
 */
INT16 CGEN_PUBLIC CStatistics_Status(CStatistics* _this)
{
  INT16 nS1 = O_K;
  INT16 nS2 = O_K;
  BOOL  bOk = FALSE;

  CHECK_THIS_RV(NOT_EXEC);                                                      /* Check this pointer                */
  printf("\n"); dlp_fprint_x_line(stdout,'-',dlp_maxprintcols());               /* Print protocol header             */
  printf("\n   Status of instance\n   statistics %s",_this->m_lpInstanceName);  /* ...                               */
  printf("\n"); dlp_fprint_x_line(stdout,'-',dlp_maxprintcols());               /* ...                               */
  printf("\n   Order                  : %ld",(long)CStatistics_GetOrder(_this));/* Report order                      */
  printf("\n   Dimensionality         : %ld",(long)CStatistics_GetDim(_this));  /* Report dimensionality             */
  printf("\n   Number of classes      : %ld",(long)CStatistics_GetNClasses(_this));/* Report number of classes          */
  printf("\n   Total sample size      : %ld",(long)CStatistics_GetNSamples(_this));/* Report total sample size          */
  printf("\n");
  nS1 = CStatistics_CheckDat(_this);
  nS2 = CStatistics_CheckLtb(_this);
  if (_this->m_nCheck==0)
  {
    printf("\n   Statistics data        : %s",OK(nS1)?"ok":"DEFECTIVE");
    printf("\n   Label table            : %s",OK(nS2)?"ok":"DEFECTIVE");
  }
  bOk = OK(nS1)&&OK(nS2);
  printf("\n\n   Statistics is %s.",bOk?"correctly setup":"CORRUPT");
  printf("\n"); dlp_fprint_x_line(stdout,'-',dlp_maxprintcols());               /* Print protocol footer             */
  if (_this->m_nCheck==0 && !bOk)
    printf("\n   Set field check to 1 for more details.");
  printf("\n");                                                                 /* ...                               */
  return O_K;                                                                   /* Done.                             */
}

/* EOF */
