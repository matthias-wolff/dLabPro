/* dLabPro class CFst (fst)
 * - HMM topology methods
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

#include "dlp_cscope.h" /* Indicate C scope */
#include "dlp_fst.h"

/**
 * Finds or adds one destination state (source state x input symbol) to the
 * destination automaton during HMM-conversion.
 * 
 * @param _this   Pointer to this (destination) automaton instance
 * @param lpTIsrc Source automaton iterator data struct
 * @param nSsrc   Unit relative index of source state
 * @param nTisSrc Source input symbol
 * @return The global zero-based index of the destination state 
 */
FST_ITYPE CGEN_PRIVATE CFst_Hmm_FindAddState
(
  CFst*         _this,
  FST_TID_TYPE* lpTIsrc,
  FST_ITYPE     nSsrc,
  FST_STYPE     nTisSrc
)
{
  INT32      nIc   = -1;                                                         /* Comp. index of aux. state quali.  */
  FST_ITYPE nSdst = -1;                                                         /* Dest. state index (return value)  */

  nIc = _this->m_nIcSdAux;                                                      /* Get comp.index of aux.state quali.*/
  for (nSdst=0; nSdst<UD_XS(_this,0); nSdst++)                                  /* Loop over destination states      */
    if                                                                          /*   Destination state ...           */
    (                                                                           /*   |                               */ 
      (FST_ITYPE)CData_Dfetch(AS(CData,_this->sd),nSdst,nIc  )==nSsrc   &&      /*   | ... belongs to src. state and */
      (FST_STYPE)CData_Dfetch(AS(CData,_this->sd),nSdst,nIc+1)==nTisSrc         /*   | ... belongs to input symbol   */
    )                                                                           /*   |                               */
    {                                                                           /*   >>                              */
      break;                                                                    /*     Gotcha!                       */
    }                                                                           /*   <<                              */
  if (nSdst<UD_XS(_this,0)) return nSdst;                                       /* State found -> that's it          */

  nSdst = CFst_AddstateCopy(_this,0,lpTIsrc->iFst,lpTIsrc->nFS+nSsrc);          /* Add new destination state         */
  CData_Dstore(AS(CData,_this->sd),nSsrc  ,nSdst,nIc  );                        /* Qualify with source state         */
  CData_Dstore(AS(CData,_this->sd),nTisSrc,nSdst,nIc+1);                        /* Qualify with input symbol         */
  return nSdst;                                                                 /* Return state index                */
}

/**
 * Converts one unit to an HMM style automaton. There are no checks performed.
 *
 * @param _this Pointer to this (destination) automaton instance
 * @param itSrc Pointer to source automaton instance
 * @param nUnit Index of unit to convert
 * @return O_K if successfull, a (negative) error code otherwise
 * @see Hmm CFst_Hmm
 */
INT16 CGEN_PROTECTED CFst_HmmUnit(CFst* _this, CFst* itSrc, INT32 nUnit)
{
  FST_TID_TYPE* lpTIsrc = NULL;                                                 /* Source transition iterator        */
  BYTE*         lpTsrc  = NULL;                                                 /* Pointer to source transition      */
  FST_STYPE     nTis    = -1;                                                   /* Input symbol                      */
  FST_ITYPE     nIniSrc = 0;                                                    /* Source initial state              */
  FST_ITYPE     nTerSrc = 0;                                                    /* Source terminal state             */
  FST_ITYPE     nIniDst = 0;                                                    /* Destination initial state         */
  FST_ITYPE     nTerDst = 0;                                                    /* Destination terminal state        */

  /* Initialize */                                                              /* --------------------------------- */
  CFst_Reset(BASEINST(_this),TRUE);                                             /* Reset destination instance        */
  CData_Scopy(AS(CData,_this->sd),AS(CData,itSrc->sd));                         /* Copy structure of state table     */
  CData_Scopy(AS(CData,_this->td),AS(CData,itSrc->td));                         /* Copy structure of trans. table    */
  CData_SelectRecs(AS(CData,_this->ud),AS(CData,itSrc->ud),nUnit,1);            /* Copy unit description of nUnit    */
  UD_FS(_this,0)=0;                                                             /* Reset first state index           */
  UD_FT(_this,0)=0;                                                             /* Reset first transition index      */
  UD_XS(_this,0)=0;                                                             /* Reset number of states            */
  UD_XT(_this,0)=0;                                                             /* Reset number of transitions       */
  _this->m_nGrany = itSrc->m_nGrany;                                            /* Copy reallocation granularity     */ 
  _this->m_nWsr   = CFst_Wsr_GetType(itSrc,&_this->m_nIcW);                     /* Copy weight semiring type /comp.  */
  _this->m_nIcTis = CData_FindComp(AS(CData,itSrc->td),NC_TD_TIS);              /* Set input symbol component in td  */
  _this->m_nIcTos = CData_FindComp(AS(CData,itSrc->td),NC_TD_TOS);              /* Set output symbol component in td */

  /* Do conversion */                                                           /* --------------------------------- */
  lpTIsrc           = CFst_STI_Init(itSrc,nUnit,FSTI_SORTINI);                  /* Create source transition iterator */
  _this->m_nIcSdAux = CData_GetNComps(AS(CData,_this->sd));                     /* Get comp.index of aux.state quali.*/
  CData_AddComp(AS(CData,_this->sd),"Ssrc"   ,T_LONG             );             /* Add source state aux. state quali.*/ 
  CData_AddComp(AS(CData,_this->sd),NC_TD_TIS,DLP_TYPE(FST_STYPE));             /* Add input symbol aux. state quali.*/
  CFst_Hmm_FindAddState(_this,lpTIsrc,0,-1);                                    /* Create start state in destination */
  for (nIniDst=0; nIniDst<UD_XS(_this,0); nIniDst++)                            /* Loop over destination states      */
  {                                                                             /* >>                                */
    nIniSrc =                                                                   /*   Get source state of dest. state */
      (FST_ITYPE)CData_Dfetch(AS(CData,_this->sd),nIniDst,_this->m_nIcSdAux);   /*   |                               */
    while ((lpTsrc=CFst_STI_TfromS(lpTIsrc,nIniSrc,lpTsrc))!=NULL)              /*   For all trans. from src. state  */
    {                                                                           /*   >>                              */
      nTerSrc = *CFst_STI_TTer(lpTIsrc,lpTsrc);                                 /*     Get source terminal state     */
      nTis    = *CFst_STI_TTis(lpTIsrc,lpTsrc);                                 /*     Get input symbol              */
      nTerDst = CFst_Hmm_FindAddState(_this,lpTIsrc,nTerSrc,nTis);              /*     Find/add dest. terminal state */
      if (nTerDst>=0 && nTerDst<UD_XS(_this,0))                                 /*     If successfull                */
        CFst_AddtransCopy(_this,0,nIniDst,nTerDst,itSrc,                        /*       Add dest. trans. and ...    */
          CFst_STI_GetTransId(lpTIsrc,lpTsrc));                                 /*       | ... copy quali. from src. */
    }                                                                           /*   <<                              */
  }                                                                             /* <<                                */
  CFst_STI_Done(lpTIsrc);                                                       /* Destroy src. transition iterator  */

  /* Clean up */                                                                /* --------------------------------- */
  CData_DeleteComps(AS(CData,_this->sd),_this->m_nIcSdAux,2);                   /* Delete auxilary state quali.      */
  _this->m_nIcSdAux = -1;                                                       /* Reset field ic_sd_aux             */
  return O_K;                                                                   /* All done                          */
}

/*
 * Manual page at fst.def
 */
INT16 CGEN_PUBLIC CFst_Hmm(CFst* _this, CFst* itSrc, INT32 nUnit)
{
  CFst* itUnit = NULL;                                                          /* Current unit                      */
  INT32  nU     = 0;                                                             /* Current unit index                */

  /* Validation */                                                              /* --------------------------------- */
  CHECK_THIS_RV(NOT_EXEC);                                                      /* Check this pointer                */
  CFst_Check(_this);                                                            /* Check this (destination) instance */
  CFst_Check(itSrc);                                                            /* Check source instance             */
  if (nUnit>=UD_XXU(itSrc)) return IERROR(_this,FST_BADID,"unit",nUnit,0);      /* Check unit index                  */

  /* NO RETURNS BEYOND THIS POINT! */                                           /* !!!                               */
  CREATEVIRTUAL(CFst,itSrc,_this);                                              /* Overlapping arguments support     */
  CFst_Reset(BASEINST(_this),TRUE);                                             /* Reset destination instance        */
  if (nUnit<0) { ICREATEEX(CFst,itUnit,"CFst_Hmm~itUnit",NULL); }               /* All units -> create aux. FST inst.*/
  else         itUnit = _this;                                                  /* One unit  -> use this instance    */

  /* Copy input and output symbol table */                                      /* --------------------------------- */
  CData_Copy(_this->is,itSrc->is);                                              /* Copy input symbol table           */
  CData_Copy(_this->os,itSrc->os);                                              /* Copy output symbol table          */

  /* Process units */                                                           /* --------------------------------- */
  for (nU=nUnit<0?0:nUnit; nU<UD_XXU(itSrc); nU++)                              /* For the one or all source units   */
  {                                                                             /* >>                                */
    CFst_HmmUnit(itUnit,itSrc,nU);                                              /*   Process a single unit           */
    if (nUnit>=0) break;                                                        /*   One unit  -> that's it          */
    CFst_Cat(_this,itUnit);                                                     /*   All units -> append to dest.    */
  }                                                                             /* <<                                */

  /* Clean up */                                                                /* --------------------------------- */
  if (nUnit<0) IDESTROY(itUnit);                                                /* All units -> destroy aux.FST inst.*/
  DESTROYVIRTUAL(itSrc,_this);                                                  /* Overlapping arguments support     */
  CFst_Check(_this);                                                            /* TODO: Remove after debugging      */
  return O_K;                                                                   /* All done                          */
}

/* EOF */
