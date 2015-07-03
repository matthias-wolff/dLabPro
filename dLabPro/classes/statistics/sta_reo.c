/* dLabPro class CStatistics (statistics)
 * - Statistics reorganization
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
#include "dlp_statistics.h"                                                     /* Include class header file         */

/**
 * <p>INTERNAL USE ONLY. This method is called by {@link -pool} to pool the
 * statistics blocks contained in <code>idSrc</code> and store the result in
 * <code>idPool</code>. The operation is controlled through the pooling mode
 * flag <code>nMode</code> as follows:</p>
 *
 * <ul>
 *   <li>nMode=0: Pool sum data, <code>idPool</code> will be overwritten</li>
 *   <li>nMode=1: Pool min data</li>
 *   <li>nMode=2: Pool max data</li>
 * </ul>
 *
 * <h3>Remarks</h3>
 * <ul>
 *   <li>In order to pool raw statistics data, there are three calls
 *     (<code>nMode</code>=0,1 and 2) necessary. The first call of these
 *     <em>must</em> be the one with <code>nMode</code>=0!</li>
 *   <li>There are NO checks performed</li>
 * </ul>
 *
 * @param idPool
 *          Pooled raw statistics data block
 * @param idSrc
 *          Raw statistics data blocks to be pooled
 * @param nMode
 *          Pooling mode, see above
 */
void CGEN_SPRIVATE CStatistics_PoolInt(CData* idPool, CData* idSrc, INT32 nMode)
{
  CData* idAux      = NULL;                                                     /* Auxilary data instance #1         */

  if (nMode==0)                                                                 /* Sum aggregation mode              */
  {                                                                             /* >>                                */
    ISETOPTION(idPool,"/block");                                                /*   Switch target to block mode     */
    CData_Aggregate(idPool,idSrc,NULL,CMPLX(0),"sum");                          /*   Aggregate (sum up)              */
    IRESETOPTIONS(idPool);                                                      /*   Switch target to normal mode    */
  }                                                                             /* <<                                */
  else if (nMode==1 || nMode==2)                                                /* Extrama aggregation modes         */
  {                                                                             /* >>                                */
    ICREATEEX(CData,idAux,"CStatistics_Pool_int.~idAux",NULL);                  /*   Create auxilary data instance #1*/
    ISETOPTION(idAux,"/block");                                                 /*   Switch target to block mode     */
    CData_Aggregate(idAux,idSrc,NULL,CMPLX(0),nMode==1?"min":"max");            /*   Aggregate (minimum or maximum)  */
    dlp_memmove(CData_XAddr(idPool,nMode,0),CData_XAddr(idAux,nMode,0),         /*   Copy aggregated min. or max. ...*/
      CData_GetRecLen(idAux));                                                  /*   | ... to target                 */
    IRESETOPTIONS(idAux);                                                       /*   Switch target to normal mode    */
    IDESTROY(idAux);                                                            /*   Destroy auxilary data inst. #1  */
  }                                                                             /* <<                                */
  else                                                                          /* Unknown mode                      */
    DLPASSERT(FMSG("Invalid internal pooling mode"));                           /*   Not so good ...                 */
}

/*
 * Manual page at statistics.def
 */
INT16 CGEN_PUBLIC CStatistics_Pool
(
  CStatistics* _this,
  CStatistics* iSrc,
  CData*       idMap
)
{
  INT32   i          = 0;                                                        /* Current component index           */
  INT32   nC         = 0;                                                        /* Current pooled statistics class   */
  INT32   nCs        = 0;                                                        /* Current source class index        */
  INT32   nXC        = 0;                                                        /* Number of pooled classes          */
  INT32   nRpb       = 0;                                                        /* Statistics raw data block size    */
  INT16  nCheckSave = 0;                                                        /* Saved check level                 */
  CData* idAux      = NULL;                                                     /* Auxilary data instance #1         */
  CData* idPmp      = NULL;                                                     /* Pooling map                       */
  CData* idPcd      = NULL;                                                     /* Pooled class raw data buffer      */

  /* Initialize */                                                              /* --------------------------------- */
  CHECK_THIS_RV(0);                                                             /* Check this instance               */
  IF_NOK(CStatistics_Check(iSrc))                                               /* Check source statistics           */
    return IERROR(_this,ERR_INVALARG,"iSrc",0,0);                               /* ...                               */
  nCheckSave = _this->m_nCheck;                                                 /* Save check level                  */
  CStatistics_Reset(BASEINST(_this),TRUE);                                      /* Reset destination                 */
  _this->m_nCheck = nCheckSave;                                                 /* Restore check level               */
  IFIELD_RESET(CData,"dat");                                                    /* Create pool raw stats. data inst. */

  /* Protocol */                                                                /* --------------------------------- */
  IFCHECK                                                                       /* On verbose level 1                */
  {                                                                             /* >>                                */
    printf("\n"); dlp_fprint_x_line(stdout,'-',dlp_maxprintcols());             /*   Print protocol header           */
    printf("\n   statistics -pool");                                            /*   ...                             */
    printf("\n"); dlp_fprint_x_line(stdout,'-',dlp_maxprintcols());printf("\n");/*   ...                             */
  }                                                                             /* <<                                */

  /* No map --> pool all classes */                                             /* --------------------------------- */
  if (CData_IsEmpty(idMap))                                                     /* NULL or empty map instance        */
  {                                                                             /* >>                                */
    IFCHECK printf("\n   Empty pooling map --> pool all classes");              /*   Protocol (verbose level 1)      */
    CStatistics_PoolInt(_this->m_idDat,iSrc->m_idDat,0);                        /*   Pool sum data                   */
    CStatistics_PoolInt(_this->m_idDat,iSrc->m_idDat,1);                        /*   Pool min data                   */
    CStatistics_PoolInt(_this->m_idDat,iSrc->m_idDat,2);                        /*   Pool max data                   */
    STA_PROTOCOL_FOOTER(1,"done");                                              /*   Print protocol footer           */
    return O_K;                                                                 /*   That's it                       */
  }

  IFCHECK printf("\n   Pooling by map");                                        /* Protocol (verbose level 1)        */
  ICREATEEX(CData,idAux,"CStatistics_Pool.~idAux",NULL);                        /* Create auxilary data instance #1  */
  ICREATEEX(CData,idPmp,"CStatistics_Pool.~idPmp",NULL);                        /* Create pooling map                */
  ICREATEEX(CData,idPcd,"CStatistics_Pool.~idPcs",NULL);                        /* Create pooled raw stats.data inst.*/

  /* Find and copy map component (pooled class) */                              /* --------------------------------- */
  for (i=0; i<CData_GetNComps(idMap); i++)                                      /* Loop over components of idMap     */
    if (dlp_is_numeric_type_code(CData_GetCompType(idMap,i)))                   /*   Is current component numeric?   */
    {                                                                           /*   >> (Yes)                        */
      CData_SelectComps(idPmp,idMap,i,1);                                       /*     Copy component                */
      break;                                                                    /*     Have ready :)                 */
    }                                                                           /*   <<                              */
  if (CData_IsEmpty(idPmp))                                                     /* Have not got map component        */
  {                                                                             /* >>                                */
    IERROR(_this,STA_BADCOMP,"map",BASEINST(idMap)->m_lpInstanceName,"numeric");/*   Error message                   */
    DLPTHROW(STA_BADCOMP);                                                      /*   Throw exception                 */
  }                                                                             /* <<                                */

  /* Create source class component */                                           /* --------------------------------- */
  CData_AddComp(idPmp,"srcc",T_LONG);                                           /* Add source class index component  */
  for (i=0; i<CData_GetNRecs(idPmp); i++) CData_Dstore(idPmp,i,i,1);            /* Fill it                           */

  /* Finish pooling map and initialize pooling */                               /* --------------------------------- */
  CData_Sortup(idPmp,idPmp,0);                                                  /* Sort map by pooled class index    */
  nXC = (INT32)CData_Dfetch(idPmp,CData_GetNRecs(idPmp)-1,0)+1;                 /* Get greatest pooled class index   */
  IFCHECK printf("\n   Pooling %ld statistics classes",(long)nXC);              /* Protocol (verbose level 1)        */
  IFCHECKEX(3) CData_Print(idPmp);                                              /* Print pooling map (verbose lvl.3) */
  nRpb = CData_GetNRecsPerBlock(iSrc->m_idDat);                                 /* Get block size                    */

  /* Prepare pooled statistics */                                               /* --------------------------------- */
  CData_Scopy(_this->m_idDat,iSrc->m_idDat);                                    /* Create target raw data components */
  CData_Allocate(_this->m_idDat,nRpb*nXC);                                      /* Allocate target raw data          */
  CData_SetNBlocks(_this->m_idDat,nXC);                                         /* Set target statistics block number*/

  /* Pooling loop */                                                            /* --------------------------------- */
  for (i=0; i<CData_GetNRecs(idPmp); )                                          /* Loop over pooling map             */
  {                                                                             /* >>                                */
    /* - Copy raw statistics data of one pooled class */                        /*   - - - - - - - - - - - - - - - - */
    nC = (INT32)CData_Dfetch(idPmp,0,0);                                         /*   Get pooled class index          */
    IFCHECK printf("\n     Pooled class %3ld");                                 /*   Protocol (verbose level 1)      */
    for (i=0; i<CData_GetNRecs(idPmp); i++)                                     /*   Loop over partition of pool.map */
    {                                                                           /*   >>                              */
      if (nC != (INT32)CData_Dfetch(idPmp,0,0)) break;                           /*     Not the current class anymore */
      nCs = (INT32)CData_Dfetch(idPmp,i,1);                                      /*     Get source class index        */
      IFCHECK printf("\n     - Source class %3ld",nCs);                         /*     Protocol (verbose level 1)    */
      CData_SelectBlocks(idAux,iSrc->m_idDat,nCs,1);                            /*     Copy raw stats. data block    */
      CData_Cat(idPcd,idAux);                                                   /*     Append to buffer              */
    }                                                                           /*   <<                              */

    /* - Pool data */                                                           /*   - - - - - - - - - - - - - - - - */
    CData_SetNBlocks(idPcd,CData_GetNRecs(idPcd)/nRpb);                         /*   Set block count of aggr. buffer */
    IFCHECK                                                                     /*   Protocol (verbose level 1)      */
      printf("\n     - Aggregating %ld statistics classes",                     /*   |                               */
      (long)CData_GetNBlocks(idPcd));                                           /*   |                               */
    CStatistics_PoolInt(idAux,idPcd,0);                                         /*   Pool sum data                   */
    CStatistics_PoolInt(idAux,idPcd,1);                                         /*   Pool min data                   */
    CStatistics_PoolInt(idAux,idPcd,2);                                         /*   Pool max data                   */

    /* - Store pooled raw statistics data block */                              /*   - - - - - - - - - - - - - - - - */
    dlp_memmove                                                                 /*   Copy pooled raw stats. data     */
    (                                                                           /*   |                               */
      CData_XAddr(_this->m_idDat,nC*nRpb,0),                                    /*   | To target statistics block    */
      CData_XAddr(idAux,0,0),                                                   /*   | From aggregation buffer       */
      CData_GetNRecs(idAux)*CData_GetRecLen(idAux)                              /*   | Length of aggregation buffer  */
    );                                                                          /*   |                               */

    /* - Clean up auxilary instances */                                         /*   - - - - - - - - - - - - - - - - */
    CData_Reset(idPcd,TRUE);                                                    /*   Clear aggregation buffer        */
  }

  /* Clean up */                                                                /* --------------------------------- */
  IDESTROY(idAux);                                                              /* Destroy auxilary data instance #1 */
  IDESTROY(idPmp);                                                              /* Destroy pooling map               */
  IDESTROY(idPcd);                                                              /* Destroy pooled cls. raw data inst.*/
  STA_PROTOCOL_FOOTER(1,"done");                                                /* Print protocol footer             */
  return O_K;                                                                   /* Ok                                */

DLPCATCH(STA_BADCOMP)                                                           /* == Catch STA_BADCOMP exception    */
  IDESTROY(idAux);                                                              /* Destroy auxilary data instance #1 */
  IDESTROY(idPmp);                                                              /* Destroy pooling map               */
  IDESTROY(idPcd);                                                              /* Destroy pooled cls. raw data inst.*/
  STA_PROTOCOL_FOOTER(1,"FAILED");                                              /* Print protocol footer             */
  return NOT_EXEC;                                                              /* Not ok                            */
}

/*
 * Manual page at statistics.def
 */
INT16 CGEN_PUBLIC CStatistics_Merge(CStatistics* _this, CStatistics* iSrc)
{
	INT32   nB    = 0;                                                            /* Current block                     */
	INT32   nC    = 0;                                                            /* Current component                 */
	INT32   nR    = 0;                                                            /* Current record                    */
	INT32   nXB   = 0;                                                            /* Number of statistics blocks       */
	INT32   nXC   = 0;                                                            /* Number of components              */
	INT32   nXR   = 0;                                                            /* Number of records per block       */
	FLOAT64 nVal  = 0.;                                                           /* Merged statistics value           */
	FLOAT64 nVal1 = 0.;                                                           /* Statistics value of this instance */
	FLOAT64 nVal2 = 0.;                                                           /* Statistics value of source inst.  */

  /* Initialize */                                                              /* --------------------------------- */
  CHECK_THIS_RV(NOT_EXEC);                                                      /* Check this instance               */
  if (_this==iSrc) return O_K;                                                  /* Nothing to be done                */
  IF_NOK(CStatistics_Check(iSrc))                                               /* Check source statistics           */
    return IERROR(_this,ERR_INVALARG,"iSrc",0,0);                               /* ...                               */

  /* Check-up */                                                                /* --------------------------------- */
  nXB = CData_GetNBlocks      (AS(CData,_this->m_idDat));                       /* Get number of statistics blocks   */
  nXC = CData_GetNComps       (AS(CData,_this->m_idDat));                       /* Get number of components          */
  nXR = CData_GetNRecsPerBlock(AS(CData,_this->m_idDat));                       /* Get number of records per block   */
  if                                                                            /* Instances incompatible if ...     */
  (                                                                             /* |                                 */
    nXB != CData_GetNBlocks      (AS(CData,iSrc->m_idDat)) ||                   /* | ... no. of blocks mismatch   OR */
    nXC != CData_GetNComps       (AS(CData,iSrc->m_idDat)) ||                   /* | ... no. of comps. mismatch   OR */
    nXR != CData_GetNRecsPerBlock(AS(CData,iSrc->m_idDat))                      /* | ... no. of records mismatch     */
  )                                                                             /* |                                 */
  {                                                                             /* >>                                */
  	return IERROR(_this,STA_INCOMPAT,0,0,0);                                    /*   So sorry ...                    */
  }                                                                             /* <<                                */

  /* Do merge */                                                                /* --------------------------------- */
  for (nB=0; nB<nXB; nB++)                                                      /* Loop over blocks                  */
  	for (nR=0; nR<nXR; nR++)                                                    /*   Loop over records of block      */
  		for (nC=0; nC<nXC; nC++)                                                  /*     Loop over components          */
			{                                                                         /*     >>                            */
				nVal1 = CData_Dfetch(AS(CData,_this->m_idDat),nB*nXR+nR,nC);            /*       Get value from this         */
				nVal2 = CData_Dfetch(AS(CData,iSrc ->m_idDat),nB*nXR+nR,nC);            /*       Get value from source       */
				switch (nR)                                                             /*       Depending on value type     */
				{                                                                       /*       >>                          */
				case STA_DAI_MIN: nVal = MIN(nVal1,nVal2); break;                       /*         Aggregate minimums        */
				case STA_DAI_MAX: nVal = MAX(nVal1,nVal2); break;                       /*         Aggregate maximums        */
				default         : nVal = nVal1+nVal2;      break;                       /*         Aggregate all other       */
				}                                                                       /*       <<                          */
				CData_Dstore(AS(CData,_this->m_idDat),nVal,nB*nXR+nR,nC);               /*       Store merged value          */
			}                                                                         /*     <<                            */

  /* Aftermath */                                                               /* --------------------------------- */
  return O_K;                                                                   /* Everything's ok                   */
}
/* EOF */
