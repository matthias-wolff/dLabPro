/* dLabPro class CStatistics (statistics)
 * - Setup and update methods
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

/*
 * Manual page at statistics.def
 */
INT16 CGEN_PUBLIC CStatistics_Setup
(
  CStatistics*    _this,
  INT32         nOrder,
  INT32         nDim,
  INT32         nCls,
  CData*          idLtb,
  INT32         nIcLtb
)
{
  INT32    c     = 0;                                                         /* Statistics class loop counter     */
  INT32    n     = 0;                                                         /* Dimension loop couner             */
  FLOAT64* lpMin = NULL;                                                      /* Ptr. to class' k minimum vector   */
  FLOAT64* lpMax = NULL;                                                      /* Ptr. to class' k maximum vector   */

  /* Validate */                                                                /* --------------------------------- */
  CHECK_THIS_RV(NOT_EXEC);                                                      /* Check this pointer                */
  if (nOrder<2) nOrder = 2;                                                     /* Default order is 2                */
  if (nDim  <1) nDim   = 1;                                                     /* Default dimensionality is 1       */
  if (nCls  <1) nCls   = 1;                                                     /* Default number of classes is 1    */

  /* Initialize statistics */                                                   /* --------------------------------- */
  CStatistics_Reset(_this,TRUE);                                                /* Start over                        */
  IFIELD_RESET(CData,"dat");                                                    /* Create/reset statistic data       */
  CData_Array(_this->m_idDat,T_DOUBLE,nDim,nCls*(nOrder+nDim+2));               /* Allocate statistic data           */
  CData_SetNBlocks(_this->m_idDat,nCls);                                        /* Set number of blocks              */
  if (CData_IsEmpty(_this->m_idDat)) return IERROR(_this,ERR_NOMEM,0,0,0);      /* Check if it worked out...         */
  for (c=0; c<nCls; c++)                                                        /* Loop over classes                 */
  {                                                                             /* >>                                */
    lpMin = CStatistics_GetPtr(_this,c,STA_DAI_MIN);                            /*   Get ptr. to class' k min. vec.  */
    lpMax = CStatistics_GetPtr(_this,c,STA_DAI_MAX);                            /*   Get ptr. to class' k max. vec.  */
    for (n=0; n<nDim; n++)                                                      /*   Loop over dimensions            */
    {                                                                           /*   >>                              */
      lpMin[n] = T_DOUBLE_MAX;                                                  /*     Initialize minimum vector     */
      lpMax[n] = T_DOUBLE_MIN;                                                  /*     Initialize maximum vector     */
    }                                                                           /*   <<                              */
  }                                                                             /* <<                                */

  /* Initialize label table */                                                  /* --------------------------------- */
  if (CData_IsEmpty(idLtb)) return O_K;                                         /* No label table -> that's it       */
  if (CData_GetNRecs(idLtb)!=nCls)                                              /* Bad number of labels              */
    return IERROR(_this,STA_NOTSETUP," (wrong no. of labels in idLtb)",0,0);    /*   -> Error                        */
  if (nIcLtb<0)                                                                 /* Label component not specified     */
    for (nIcLtb=0; nIcLtb<CData_GetNComps(idLtb); nIcLtb++)                     /*   Seek first symbolic component   */
      if (dlp_is_symbolic_type_code(CData_GetCompType(idLtb,nIcLtb)))           /*   ...                             */
        break;                                                                  /*   ...                             */
  if (!dlp_is_symbolic_type_code(CData_GetCompType(idLtb,nIcLtb)))              /* Label component not symbolic      */
    return IERROR(_this,STA_NOTSETUP," (label comp. not found in idLtb)",0,0);  /*   -> Error                        */
  IFIELD_RESET(CData,"ltb");                                                    /* Create/reset label table          */
  CData_SelectComps(_this->m_idLtb,idLtb,nIcLtb,1);                             /* Copy label table                  */

  return O_K;                                                                   /* Done                              */
}

/**
 * Updates the statistics with one vector. There are no checks performed!
 *
 * @param _this
 *          Pointer to this CStatistics instance
 * @param lpX
 *          Pointer to a buffer containing the vector to update the statistics
 *          with (is expected to contain <i>N</i> = <code>{@link dat}.dim</code>
 *          double values)
 * @param c
 *          Index of class this vector belongs to (0 &le; <code>k</code> &lt;
 *          <i>C</i> = <code>{@link dat}.nblock</code>)
 */
INT16 CGEN_PROTECTED CStatistics_UpdateVector
(
  CStatistics* _this,
  FLOAT64*      lpX,
  INT32         c,
  FLOAT64       w
)
{
  INT32    i     = 0;                                                            /* Mixed sum index                   */
  INT32    k     = 0;                                                            /* Order loop counter (for k>2)      */
  INT32    n     = 0;                                                            /* 1st dimension loop couner         */
  INT32    m     = 0;                                                            /* 2nd dimension loop couner         */
  INT32    K     = 0;                                                            /* Statistics order                  */
  INT32    N     = 0;                                                            /* Statistics dimensionality         */
  FLOAT64* lpSsz = NULL;                                                         /* Ptr. to class' c sample size      */
  FLOAT64* lpMin = NULL;                                                         /* Ptr. to class' c minimum vector   */
  FLOAT64* lpMax = NULL;                                                         /* Ptr. to class' c maximum vector   */
  FLOAT64* lpSum = NULL;                                                         /* Ptr. to class' c sum vector       */
  FLOAT64* lpMsm = NULL;                                                         /* Ptr. to class' c mixed sum matrix */
  FLOAT64* lpKsm = NULL;                                                         /* Ptr. to class' c k-th ord.sum vec.*/

  /* Validate */                                                                /* --- DEBUG ONLY ------------------ */
  DLPASSERT(_this);                                                             /* Check this pointer                */
  DLPASSERT(_this->m_idDat);                                                    /* Check statistics data table       */
  DLPASSERT((INT32)(dlp_size(lpX)/sizeof(FLOAT64)) == CStatistics_GetDim(_this)); /* Check update vector buffer        */
  DLPASSERT(c>=0 && c<CStatistics_GetNClasses(_this));                          /* Check class index                 */

  /* Initialize */                                                              /* --------------------------------- */
  K = CStatistics_GetOrder(_this);                                              /* Get statistics order              */
  N = CStatistics_GetDim(_this);                                                /* Get statistics dimensionality     */
  DLPASSERT((lpSsz = CStatistics_GetPtr(_this,c,STA_DAI_SSIZE)));               /* Get ptr. to class' c sample size  */
  DLPASSERT((lpMin = CStatistics_GetPtr(_this,c,STA_DAI_MIN  )));               /* Get ptr. to class' c min. vector  */
  DLPASSERT((lpMax = CStatistics_GetPtr(_this,c,STA_DAI_MAX  )));               /* Get ptr. to class' c max. vector  */
  DLPASSERT((lpSum = CStatistics_GetPtr(_this,c,STA_DAI_SUM  )));               /* Get ptr. to class' c sum vector   */
  DLPASSERT((lpMsm = CStatistics_GetPtr(_this,c,STA_DAI_MSUM )));               /* Get ptr. to class' c mix.sum.matr.*/
  DLPASSERT((lpKsm = CStatistics_GetPtr(_this,c,STA_DAI_KSUM )) || K<=2);       /* Get ptr. to class' c k-th ord.sum.*/

  /* Update */                                                                  /* --------------------------------- */
  (*lpSsz)+=w;                                                                  /* Increment sample size             */
  for (n=0,i=0; n<N; n++)                                                       /* Loop over dimensions              */
  {                                                                             /* >>                                */
    if (lpMin[n] > lpX[n]) lpMin[n] = lpX[n];                                   /*   Track minimum                   */
    if (lpMax[n] < lpX[n]) lpMax[n] = lpX[n];                                   /*   Track maximum                   */
    lpSum[n] += lpX[n]*w;                                                       /*   Update sum                      */
    for (m=0; m<N; m++,i++) lpMsm[i] += lpX[n]*lpX[m]*w;                        /*   Update mixed sum                */
    for (k=3; k<=K; k++) lpKsm[(k-3)*N+n] += dlm_pow(lpX[n],k)*w;               /*   Update k-th order sums          */
  }                                                                             /* <<                                */

  return O_K;                                                                   /* Done                              */
}

/*
 * Manual page at statistics.def
 */
INT16 CGEN_PUBLIC CStatistics_Update
(
  CStatistics* _this,
  CData*       idVec,
  INT32         nIcLab,
  CData*       idW
)
{
  INT32        i           = 0;                                                  /* Update vector loop counter        */
  INT32        I           = 0;                                                  /* Number of update vectors          */
  INT32        c           = 0;                                                  /* Class of current update vector    */
  INT32        C           = 0;                                                  /* Number of classes                 */
  INT32        n           = 0;                                                  /* Dimension loop counter            */
  INT32        N           = 0;                                                  /* Statistics' dimensionality        */
  FLOAT64      w           = 0.;                                                 /* Weight of current update vector   */
  char*          lpsLab      = NULL;                                               /* Symbolic label of curr. upd. vec. */
  FLOAT64*     lpX         = NULL;                                               /* Vector copy buffer                */
  INT32        nVecIgnored = 0;                                                  /* Number of ignored vectors         */

  /* Validate */                                                                /* --------------------------------- */
  CHECK_THIS_RV(NOT_EXEC);                                                      /* Check this pointer                */
  IF_NOK(CStatistics_Check(_this))                                              /* Check instance data               */
    return IERROR(_this,STA_NOTSETUP," ( use -status for details)",0,0);        /* ...                               */
  if (CData_IsEmpty(idVec)) return O_K;                                         /* No input vector(s), no service!   */
  I = CData_GetNRecs(idVec);                                                    /* Get number of update vectors      */
  C = CStatistics_GetNClasses(_this);                                           /* Get number of statitistics classes*/
  N = CStatistics_GetDim(_this);                                                /* Get statistics vector dimension   */
  if (C>1)                                                                      /* Multiclass statistics needs labels*/
  {                                                                             /* >>                                */
    if (_this->m_idLtb)                                                         /*   Need symbolic labels            */
    {                                                                           /*   >>                              */
      if ((nIcLab<0 || nIcLab>=CData_GetNComps(idVec)))                         /*     Symbolic label comp. not spec.*/
        for (nIcLab=0; nIcLab<CData_GetNComps(idVec); nIcLab++)                 /*       Seek label component        */
          if (dlp_is_symbolic_type_code(CData_GetCompType(idVec,nIcLab)))       /*       ...                         */
            break;                                                              /*       ...                         */
      if (!dlp_is_symbolic_type_code(CData_GetCompType(idVec,nIcLab)))          /*     Symbolic label comp. not found*/
        return                                                                  /*       -> Error                    */
          IERROR(_this,STA_BADCOMP,"Label",idVec->m_lpInstanceName,"symbolic"); /*       |                           */
    }                                                                           /*   <<                              */
    else                                                                        /*   Need numeric labels             */
    {                                                                           /*   >>                              */
      if (!dlp_is_numeric_type_code(CData_GetCompType(idVec,nIcLab)) &&         /*     Numeric label comp. not found */
          (nIcLab>=0 || CData_GetNComps(idW)!=C))                               /*     |                             */
        return                                                                  /*       -> Error                    */
          IERROR(_this,STA_BADCOMP,"Label",idVec->m_lpInstanceName,"numeric");  /*       |                           */
    }                                                                           /*   <<                              */
  }                                                                             /* <<                                */
  /*else if (nIcLab>=0) IERROR(_this,STA_IGNORE,"label component",0,0);        / * Only one class  -> ignore labels  */
  if (dlp_is_numeric_type_code(CData_GetCompType(idVec,nIcLab)))                /* Check no. of comps. in idVec ...  */
  {                                                                             /* >>                                */
    if (CData_GetNNumericComps(idVec)!=N+1)                                     /*   Wrong number of numeric comps.  */
      IERROR(_this,STA_DIM,idVec->m_lpInstanceName,"numeric components",N+1);   /*     -> Warning                    */
  }                                                                             /* <<                                */
  else if (CData_GetNNumericComps(idVec)!=N)                                    /* Wrong number of numeric comps.    */
    IERROR(_this,STA_DIM,idVec->m_lpInstanceName,"numeric components",N);       /*   -> Warning                      */

  if (idW)                                                                      /* Weigths passed -> check 'em       */
  {                                                                             /* >>                                */
    if (!dlp_is_numeric_type_code(CData_GetCompType(idW,0)))                    /*   Component 0 not numeric         */
      return                                                                    /*   -> Error                        */
        IERROR(_this,STA_BADCOMP,"Weight",idW->m_lpInstanceName,"numeric");     /*   |                               */
    if (CData_GetNComps(idW)!=1 && CData_GetNComps(idW)!=C)                     /*   More than one component         */
      IERROR(_this,STA_IGNORE,"components in weight sequence",0,0);             /*   -> Warning                      */
    if (CData_GetNRecs(idW)!=I)                                                 /*   Not exactly one weight per vec. */
      IERROR(_this,STA_DIM,idW->m_lpInstanceName,"records",I);                  /*   -> Warning                      */
  }                                                                             /* <<                                */

  /* Initialize - NO RETURNS BEYOND THIS POINT! - */                            /* --------------------------------- */
  lpX = (FLOAT64*)dlp_calloc(N,sizeof(FLOAT64));                                  /* Allocate vector copy buffer       */

  /* Update statistics */                                                       /* --------------------------------- */
  for (i=0; i<I; i++)                                                           /* Loop over update vectors          */
  {                                                                             /* >>                                */
    /* Get vector label */                                                      /*   - - - - - - - - - - - - - - - - */
    if (C>1)                                                                    /*   Multiclass stats. needs labels  */
    {                                                                           /*   >>                              */
      if (_this->m_idLtb)                                                       /*     idVec contains symbolic labs. */
      {                                                                         /*     >>                            */
        INT32 nLIdx = 0;
        DLPASSERT(dlp_is_symbolic_type_code(CData_GetCompType(idVec,nIcLab)));  /*       Must be checked before!     */
        lpsLab = (char*)CData_XAddr(idVec,i,nIcLab);                            /*       Get string ptr. to label    */
        if(_this->m_bLabel){
          nLIdx=strlen(lpsLab)-1;
          if(nLIdx && lpsLab[nLIdx]==']') nLIdx--; else nLIdx=0;
          if(nLIdx && lpsLab[nLIdx]>='0' && lpsLab[nLIdx]<='9') nLIdx--; else nLIdx=0;
          while(nLIdx && lpsLab[nLIdx]>='0' && lpsLab[nLIdx]<='9') nLIdx--;
          if(nLIdx && lpsLab[nLIdx]=='[') lpsLab[nLIdx]='\0'; else nLIdx=0;
        }
        c      = CData_Find(_this->m_idLtb,0,C,1,0,lpsLab);                     /*       Look up label -> class idx. */
        if(nLIdx) lpsLab[nLIdx]='[';
        if (c<0)                                                                /*       Label invalid               */
        {                                                                       /*       >>                          */
          IERROR(_this,STA_SLAB,i,lpsLab?lpsLab:"(null)",0);                    /*         Warning                   */
          continue;                                                             /*         Ignore record             */
        }                                                                       /*       <<                          */
      }                                                                         /*     <<                            */
      else if(nIcLab>=0)                                                        /*     idVec contains numeric labs.  */
      {                                                                         /*     >>                            */
        c = (INT32)CData_Dfetch(idVec,i,nIcLab);                                 /*       Fetch label                 */
        if (c<0 || c>=C)                                                        /*       Label invalid               */
        {                                                                       /*       >>                          */
          IERROR(_this,STA_NLAB,i,c,0);                                         /*         Warning                   */
          continue;                                                             /*         Ignore record             */
        }                                                                       /*       <<                          */
      }                                                                         /*     <<                            */
      else c = 0;                                                               /*     Default class is 0            */
    }                                                                           /*   <<                              */
    else c = 0;                                                                 /*   Default class is 0              */

    do                                                                          /*   Loop over classes               */
    {                                                                           /*   >>                              */

    /* Get (weighted) update vector and update statistics */                    /*   - - - - - - - - - - - - - - - - */
    if (idW)                                                                    /*   Using weights                   */
    {                                                                           /*   >>                              */
      w = CData_Dfetch(idW,i,_this->m_idLtb || nIcLab>=0 ? 0 : c);              /*     Fetch weight for this vector  */
      if(w==0.) continue;                                                       /*     Nothing to do if no weight    */
      _this->m_bWeighted=TRUE;
    } else w=1.;                                                                /*   <<                              */
    CData_DrecFetch(idVec,lpX,i,N,nIcLab);                                      /*   Fetch update vector             */
    fpclassify(0.);
    for (n=0; n<N; n++)                                                         /*   Loop over vector components     */
      if (fabs(lpX[n])>1E100)                                                   /*     Check value                   */
        break;                                                                  /*       There's something wrong ... */
    if (n<N) { nVecIgnored++; continue; }                                       /*   Ignore this vector              */
    CStatistics_UpdateVector(_this,lpX,c,w);                                    /*   Update statistics with vector   */

    }                                                                           /*   <<                              */
    while(!_this->m_idLtb && nIcLab<0 && ++c<C);                                /*   Next class if there is one      */
  }                                                                             /* <<                                */

  /* Clean up */                                                                /* --------------------------------- */
  if (nVecIgnored>0) IERROR(_this,STA_VECIGNORED,nVecIgnored,0,0);              /* Error: some vectors ignored       */
  dlp_free(lpX);                                                                /* Free vector copy buffer           */
  return O_K;                                                                   /* Done                              */
}

/* EOF */
