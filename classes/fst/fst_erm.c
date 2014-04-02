/* dLabPro class CFst (fst)
 * - Epsilon removal
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

#define TD_W(FST,T,OFS) *(FST_WTYPE*)(CData_XAddr(AS(CData,FST->td),T,0)+OFS)

/**
 * Internal use. Called by {@link Epsdist CFst_Epsdist}.
 */
void CGEN_PRIVATE CFst_Edist_Fwd
(
  CFst*         _this,                                                         /* Automaton instance                 */
  FST_TID_TYPE* lpTI,                                                          /* Automaton iterator data struct     */
  FST_ITYPE     nS,                                                            /* Curr. term. state of epsilon path  */
  FST_WTYPE     nDist,                                                         /* Weight of curr.eps.path (=distance)*/
  FST_WTYPE*    lpW,                                                           /* Weight distance array              */
  FST_ITYPE*    lpTos                                                          /* Output symbol distance array       */
)
{
  BYTE*     lpT  = NULL;                                                       /* Current transition                 */
  FST_ITYPE nTer = 0;                                                          /* Terminal state of curr. transition */
  FST_WTYPE nW   = 0.;                                                         /* Weight of current transition       */

  if (SD_FLG(_this,nS+lpTI->nFS)&SD_FLG_USER3) return;                         /* Circle in eps-path                 */
  SD_FLG(_this,nS+lpTI->nFS) |= SD_FLG_USER3;                                  /* We have been here                  */

  lpT = NULL;                                                                  /* Initialize transition enumeration  */
  while ((lpT=CFst_STI_TfromS(lpTI,nS,lpT))!=NULL)                             /* For all transitions leaving nS     */
  {
    if (*CFst_STI_TTis(lpTI,lpT)!=-1) continue;                                /* Skip non-epsilon transitions       */
    nTer = *CFst_STI_TTer(lpTI,lpT);                                           /* Get terminal state of transition   */

    if (lpTI->nOfTTos && *CFst_STI_TTos(lpTI,lpT)>=0)                          /* If output symb. and curr. not eps  */
      if (lpTos[nTer]<0)                                                       /*   If no outher output symb. stored */
        lpTos[nTer]=*CFst_STI_TTos(lpTI,lpT);                                  /*     Store output symbol            */
      /* else ERROR! */
    
    if (lpTI->nOfTW>0)                                                         /* Have weights?                      */
    {                                                                          /* YES:                               */
      nW        = CFst_Wsr_Op(_this,nDist,*CFst_STI_TW(lpTI,lpT),OP_MULT);     /*   Get weight of current eps.path   */
      lpW[nTer] = CFst_Wsr_Op(_this,lpW[nTer],nW,OP_ADD);                      /*   Store weight of current eps.path */
      CFst_Edist_Fwd(_this,lpTI,nTer,nW,lpW,lpTos);                            /*   Traverse automaton               */
    }
    else
    {                                                                          /* NO:                                */
      lpW[nTer] = 1.;                                                          /*   nTer can be reached via eps.path */
      CFst_Edist_Fwd(_this,lpTI,nTer,0.,lpW,lpTos);                            /*   Traverse automaton               */
    }
  }
}

/*
 * Manual page at fst_man.def
 */
INT16 CGEN_PUBLIC CFst_Epsdist(CFst* _this, CFst* itSrc, INT32 nUnit)
{
  FST_TID_TYPE* lpTI    = NULL;                                                /* Automaton iterator data struct     */
  BYTE*         lpT     = NULL;                                                /* Current transition                 */
  FST_WTYPE*    lpW     = NULL;                                                /* Distance array                     */
  FST_STYPE*    lpTos   = NULL;                                                /* Output symbol distance array       */
  FST_ITYPE     nS      = 0;                                                   /* Current state                      */
  FST_ITYPE     nIni    = 0;
  FST_WTYPE     nNeMult = 0.;                                                  /* Neutral element of multiplication  */

  /* Validate */
  CHECK_THIS_RV(NOT_EXEC);
  CFst_Check(_this);
  if (nUnit<0 || nUnit>=UD_XXU(itSrc))                                         /* TODO: add multi-unit support!      */
    return IERROR(_this,FST_BADID,"unit",nUnit,0);
  if (CData_FindComp(AS(CData,itSrc->td),NC_TD_TIS)<0)
    return
      IERROR(_this,FST_MISS,"input symbol component",NC_TD_TIS,
        "transition table");

  /* Initialize - NO RETURNS BEYOND THIS POINT */                              /* ---------------------------------- */
  CREATEVIRTUAL(CFst,itSrc,_this);                                             /* Overlapping arguments support      */
  lpTI            = CFst_STI_Init(itSrc,nUnit,FSTI_SORTINI);                   /* Create automaton iterator          */
  itSrc->m_nWsr   = CFst_Wsr_GetType(itSrc,&itSrc->m_nIcW);                    /* Determine weight semiring          */
  itSrc->m_nIcTos = CData_FindComp(AS(CData,itSrc->td),NC_TD_TOS);             /* Find output symbol component       */
  lpW             = (FST_WTYPE*)dlp_calloc(lpTI->nXS,sizeof(FST_WTYPE));       /* Allocate weight distance array     */
  lpTos           = (FST_STYPE*)dlp_calloc(lpTI->nXS,sizeof(FST_STYPE));       /* Allocate output symbol dist. array */
  nNeMult         = CFst_Wsr_NeAdd(itSrc->m_nWsr);                             /* Get neutral element of multipl.    */

  /* Initialize destination */                                                 /* ---------------------------------- */
  CFst_Reset(BASEINST(_this),TRUE);                                            /* Reset destination automaton        */
  CFst_Addunit(_this,UD_NAME(itSrc,nUnit));                                    /* Add one unit                       */
  CFst_Addstates(_this,0,UD_XS(itSrc,nUnit),FALSE);                            /* Add number of states of source     */

  CData_AddComp(AS(CData,_this->td),NC_TD_TIS,DLP_TYPE(FST_STYPE));            /* Add input symbol comp. to trans.   */
  if (itSrc->m_nIcTos>0)                                                       /* If source has output symbols ...   */
    CData_AddComp(AS(CData,_this->td),NC_TD_TOS,DLP_TYPE(FST_STYPE));          /*   Add output symbol comp. to trans.*/
  if (itSrc->m_nIcW>0)                                                         /* If source has weights ...          */
    CData_AddComp(AS(CData,_this->td),CData_GetCname(AS(CData,itSrc->td),      /* |                                  */
      itSrc->m_nIcW),DLP_TYPE(FST_WTYPE));                                     /*   Add weight component to trans.   */

  /* Copy input and output symbol table */                                     /* ---------------------------------- */
  CData_Copy(_this->is,itSrc->is);                                             /* Copy input symbol table            */
  CData_Copy(_this->os,itSrc->os);                                             /* Copy output symbol table           */

  /* Mark states to calculate epsilon distances for */                         /* ---------------------------------- */
  for (nS=0; nS<lpTI->nXS; nS++)
  {
    if (SD_FLG(itSrc,nS+lpTI->nFS)&SD_FLG_FINAL)                               /*   Final states in source ...       */
      SD_FLG(_this,nS)|=SD_FLG_FINAL;                                          /*   ... are also final in dest.      */

    SD_FLG(itSrc,nS+lpTI->nFS) &= ~SD_FLG_USER1;                               /*   Clear start eps. path flag       */
    SD_FLG(itSrc,nS+lpTI->nFS) &= ~SD_FLG_USER2;                               /*   Clear end eps. path flag         */

    if (nS==0) SD_FLG(itSrc,nS+lpTI->nFS) |= SD_FLG_USER1;                     /*   Start state may start eps.path   */

    if
    (
      (SD_FLG(itSrc,nS+lpTI->nFS)&SD_FLG_FINAL) ||                             /*   Explicit ...                     */
      CFst_STI_TfromS(lpTI,nS,lpT)==NULL                                       /*   or implicit final states ...     */
    )
    {                                                                          /*   >>                               */
      SD_FLG(itSrc,nS+lpTI->nFS) |= SD_FLG_USER2;                              /*     are potential eps. path ends   */
    }                                                                          /*   <<                               */

    lpT = NULL;
    while ((lpT=CFst_STI_TtoS(lpTI,nS,lpT))!=NULL)                             /*   All trans. terminating at nS     */
      if (*CFst_STI_TTis(lpTI,lpT)>=0)                                         /*     non-eps. trans. to here ...    */
      {                                                                        /*     >>                             */
        SD_FLG(itSrc,nS+lpTI->nFS) |= SD_FLG_USER1;                            /*       may be start of eps. path    */
        break;                                                                 /*       sure!                        */
      }                                                                        /*     <<                             */

    lpT = NULL;
    while ((lpT=CFst_STI_TfromS(lpTI,nS,lpT))!=NULL)                           /*   All trans. starting at nS        */
      if (*CFst_STI_TTis(lpTI,lpT)>=0)                                         /*     non-eps. trans. from here ...  */
      {                                                                        /*     >>                             */
        SD_FLG(itSrc,nS+lpTI->nFS) |= SD_FLG_USER2;                            /*       may be end of eps. path      */
        break;                                                                 /*       sure!                        */
      }                                                                        /*     <<                             */
  }  

  /* Loop over states */                                                       /* ---------------------------------- */
  for (nIni=0; nIni<lpTI->nXS; nIni++)
  {
    if ((SD_FLG(itSrc,nIni+lpTI->nFS)&SD_FLG_USER1)==0) continue;              /* Skip states not starting eps.paths */

    for (nS=0; nS<lpTI->nXS; nS++)                                             /* At all states ...                  */
    {                                                                          /*                                    */
      SD_FLG(itSrc,nS+lpTI->nFS) &= ~SD_FLG_USER3;                             /*   Clear SD_FLG_USER2               */
      lpW  [nS] = nNeMult;                                                     /*   Clear state distance array       */
      lpTos[nS] = -1;                                                          /*   Clear output symbol dist. array  */
    }                                                                          /*                                    */

    CFst_Edist_Fwd(itSrc,lpTI,nIni,CFst_Wsr_NeMult(itSrc->m_nWsr),lpW,lpTos);  /* Calculate epsilon distances nIni-> */

    for (nS=0; nS<lpTI->nXS; nS++)                                             /* For all states                     */
      if (lpW[nS]!=nNeMult)                                                    /*   that we hit with an eps. path    */
        if (SD_FLG(itSrc,nS+lpTI->nFS)&SD_FLG_USER2)                           /*     and that may end an eps. path  */
          if (SD_FLG(itSrc,nS+lpTI->nFS)&SD_FLG_USER3)                         /*       and that were actually hit   */
            CFst_AddtransEx(_this,0,nIni,nS,-1,lpTos[nS],lpW[nS]);             /*         add transition to _this    */
  }  

  /* Clean up */                                                               /* ---------------------------------- */
  dlp_free(lpW);                                                               /* Destroy distance array             */
  dlp_free(lpTos);                                                             /* Destroy output symbol dist. array  */
  CFst_STI_Done(lpTI);                                                         /* Destroy automaton iterator         */
  DESTROYVIRTUAL(itSrc,_this);                                                 /* Overlapping arguments support      */
  CFst_Check(_this);                                                           /* TODO: remove after debugging       */
  return O_K;                                                                  /* That was gooood ...                */
}

/**
 * Internal use. Called by {@link Epsremove CFst_Epsremove}.
 * Returns the source state corresponding to a given destination state and
 * maintains a bidirectional map between source and destination states. If
 * no destination state exists for the given source state, the method will
 * add a new destination state. The qualification of the new state will be
 * copied form <code>nTerSrc</code>.
 *
 * @param _this   Pointer to destination automaton instance
 * @param lpTIsrc Pointer to source automaton iterator data struct
 * @param nTerSrc Unit relative index of source terminal state
 * @return The destination state corresponding to the source state
 */
FST_ITYPE CGEN_PRIVATE CFst_Erm_GetDestTerState
(
  CFst*          _this,
  FST_TID_TYPE* lpTIsrc,
  FST_ITYPE     nTerSrc
)
{
  FST_ITYPE nTerDst = -1;                                                      /* Absolute index of dst. term. state */

  nTerDst = (FST_ITYPE)CData_Dfetch(AS(CData,lpTIsrc->iFst->sd),               /* Lookup dest. terminal state        */
    lpTIsrc->nFS+nTerSrc,lpTIsrc->iFst->m_nIcSdAux);                           /* |                                  */
  if (nTerDst<0)                                                               /* No corresponding term. state       */
  {                                                                            /* >>                                 */
    nTerDst = CFst_AddstateCopy(_this,0,lpTIsrc->iFst,lpTIsrc->nFS+nTerSrc);   /*   Copy state to destination        */
    CData_Dstore(AS(CData,lpTIsrc->iFst->sd),(FLOAT64)nTerDst,                  /*   Set corresponding dst. state     */
      lpTIsrc->nFS+nTerSrc,lpTIsrc->iFst->m_nIcSdAux);                         /*   |                                */
    CData_Dstore(AS(CData,_this->sd),(FLOAT64)nTerSrc,                          /*   Set corresponding src. state     */
      nTerDst,_this->m_nIcSdAux);                                              /*   |                                */
  }                                                                            /* <<                                 */
  return nTerDst;                                                              /* Return result                      */
}

/**
 * Determines the epsilon distance between the start start and a given state.
 * 
 * @param lpTIedt
 *          Pointer to transition iterator data struct of epsilon distance
 *          tansducer
 * @param nSsrc
 *          Unit relative index of state to seek an epsilon path
 *          <code>0==&gt;nSsrc</code> for
 * @param lpEpsWFZ
 *          Pointer to a buffer to be filled with weight of epsilon path
 *          <code>0==&gt;nSsrc</code>
 * @return <code>TRUE</code> if the requsted epsilon path exists,
 *         <code>FALSE</code> otherwise
 */
BOOL CGEN_PRIVATE CFst_Erm_EpsDistFromStart
(
  FST_TID_TYPE* lpTIedt,
  FST_ITYPE     nSsrc,
  FST_WTYPE*    lpEpsWFZ
)
{
  BYTE* lpTedt = NULL;                                                          /* Pointer to current transition     */
  DLPASSERT(lpTIedt);                                                           /* Check pointer to iterator         */
  DLPASSERT(lpEpsWFZ);                                                          /* Check pointer to return buffer    */
  while ((lpTedt=CFst_STI_TfromS(lpTIedt,0,lpTedt)))                            /* Follow transitions from src. st.  */
    if (*CFst_STI_TTer(lpTIedt,lpTedt)==nSsrc)                                  /*   This is the eps. path 0==>nSsrc */
    {                                                                           /*   >>                              */
      *lpEpsWFZ = *CFst_STI_TW(lpTIedt,lpTedt);                                 /*     Get weight                    */
      return TRUE;                                                              /*     That's it, found an eps. path */
    }                                                                           /*   <<                              */
  return FALSE;                                                                 /* Did not find an epsilon path      */
}

/*
 * Manual page at fst_man.def
 */
INT16 CGEN_PUBLIC CFst_Epsremove(CFst* _this, CFst* itSrc, INT32 nUnit)
{
  CFst*         itEdt   = NULL;                                                 /* Epsilon distance transducer       */
  FST_TID_TYPE* lpTIsrc = NULL;                                                 /* Ptr.to src. iterator data struct  */
  FST_TID_TYPE* lpTIedt = NULL;                                                 /* Ptr.to eps.dist.FST iterator d.s. */
  BYTE*         lpTsrc  = NULL;                                                 /* Current source transition         */
  BYTE*         lpTedt  = NULL;                                                 /* Current epsilon distance trans.   */
  FST_ITYPE     nS      = 0;                                                    /* Current destination state         */
  FST_ITYPE     nT      = 0;                                                    /* Current destination transition    */
  FST_WTYPE     nW      = 0.;                                                   /* Current dest. transition weight   */
  FST_ITYPE     nTsrc   = 0;                                                    /* Corresponding transition in src.  */
  FST_ITYPE     nSsrc   = 0;                                                    /* Corresponding state in source     */
  FST_ITYPE     nTerSrc = 0;                                                    /* Current terminal state in source  */
  FST_ITYPE     nTerDst = 0;                                                    /* Current terminal state in dest.   */
  FST_ITYPE     nTerEdt = 0;                                                    /* Curr. ter. state in eps.dist. FST */
  BOOL          bEpsWFZ = FALSE;                                                /* Epsilon-path-from-start-flag      */
  FST_WTYPE     nEpsWFZ = 0.;                                                   /* Weight of epsilon path from start */
  INT32          nIcTis  = -1;                                                   /* Input symbol component            */
  INT16         nCheck  = -1;                                                   /* Verbose level                     */

  /* Validate */
  CHECK_THIS_RV(NOT_EXEC);
  CFst_Check(_this);
  if (nUnit<0 || nUnit>=UD_XXU(itSrc))                                          /* TODO: add multi-unit support!     */
    return IERROR(_this,FST_BADID,"unit",nUnit,0);
  if (CData_FindComp(AS(CData,itSrc->td),NC_TD_TIS)<0)
    return
      IERROR(_this,FST_MISS,"input symbol component",NC_TD_TIS,
        "transition table");

  /* Protocol */                                                                /* --------------------------------- */
  nCheck = BASEINST(itSrc)->m_nCheck;                                           /* Get maximum verbose level ...     */
  if (nCheck<BASEINST(_this)->m_nCheck) nCheck = BASEINST(_this)->m_nCheck;     /* ...                               */
  BASEINST(_this)->m_nCheck = nCheck;                                           /* Set verbose level                 */
  IFCHECKEX(1)                                                                  /* On verbose level 1                */
  {                                                                             /* >>                                */
    printf("\n"); dlp_fprint_x_line(stdout,'-',dlp_maxprintcols());             /*   Print separator                 */
    printf("\n CFst_Epsremove(");                                               /*   Print function signature        */
    printf("%s,",BASEINST(_this)->m_lpInstanceName);                            /*   ...                             */
    printf("%s,",BASEINST(itSrc)->m_lpInstanceName);                            /*   ...                             */
    printf("%ld)",(long)nUnit);                                                 /*   ...                             */
    printf("\n"); dlp_fprint_x_line(stdout,'-',dlp_maxprintcols());             /*   Print separator                 */
  }                                                                             /* <<                                */

  /* Check for epsilon transition (you never know...) */                        /* --------------------------------- */
  nIcTis = CData_FindComp(AS(CData,itSrc->td),NC_TD_TIS);                       /* Get input symbol component        */
  for (nT=UD_FT(itSrc,nUnit); nT<UD_FT(itSrc,nUnit)+UD_XT(itSrc,nUnit); nT++)   /* Loop over trans. of input unit    */
    if ((INT32)CData_Dfetch(AS(CData,itSrc->td),nT,nIcTis)<0)                    /*   Input symbol is epsilon         */
      break;                                                                    /*     Well ...                      */
  if (nT==UD_FT(itSrc,nUnit)+UD_XT(itSrc,nUnit))                                /* Did not find any eps. transitions */
  {                                                                             /* >>                                */
    IFCHECKEX(1) printf("\n No epsilon transitions found in source");           /*   Protocol                        */
    goto L_OK;                                                                  /*   Nothing to be done              */
  }                                                                              /* <<                                */

  /* Initialize - NO RETURNS BEYOND THIS POINT */                               /* --------------------------------- */
  CREATEVIRTUAL(CFst,itSrc,_this);                                              /* Overlapping arguments support     */
  CFst_CopyUi(_this,itSrc,NULL,nUnit);                                          /* Copy unit description             */
  UD_XS(_this,0)=1; CData_SetNRecs(AS(CData,_this->sd),1);                      /* Remove all states but start state */
  UD_XT(_this,0)=0; CData_SetNRecs(AS(CData,_this->td),0);                      /* Remove all transitions            */
  CData_AddComp(AS(CData,_this->sd),"~MAP",DLP_TYPE(FST_ITYPE));                /* Add dst->src state map comp. ...  */
  _this->m_nIcSdAux = CData_GetNComps(AS(CData,_this->sd))-1;                   /* and remember index                */
  CData_AddComp(AS(CData,itSrc->sd),"~MAP",DLP_TYPE(FST_ITYPE));                /* Add src->dst state map comp. ...  */
  itSrc->m_nIcSdAux = CData_GetNComps(AS(CData,itSrc->sd))-1;                   /* and remember index                */
  lpTIsrc = CFst_STI_Init(itSrc,nUnit,FSTI_SORTINI);                            /* Initialize source interator       */
  for (nSsrc=0; nSsrc<lpTIsrc->nXS; nSsrc++)                                    /* Initialize src->dst state map     */
    CData_Dstore(AS(CData,itSrc->sd),(FLOAT64)-1,                             /* |                                 */
      lpTIsrc->nFS+nSsrc,itSrc->m_nIcSdAux);                                    /* |                                 */
  _this->m_nWsr = CFst_Wsr_GetType(_this,&_this->m_nIcW);                       /* Get weight semiring               */
  BASEINST(_this)->m_nCheck = nCheck;                                           /* Set verbose level                 */

  /* Copy input and output symbol table */                                     /* ---------------------------------- */
  CData_Copy(_this->is,itSrc->is);                                             /* Copy input symbol table            */
  CData_Copy(_this->os,itSrc->os);                                             /* Copy output symbol table           */

  /* Calculate epsilon distances */                                             /* --------------------------------- */
  ICREATEEX(CFst,itEdt,"CFst_Epsremove.itEdt",NULL);                            /* Create auxilary CFst instance     */
  CFst_Epsdist(itEdt,itSrc,nUnit);                                              /* Calculate epsilon distances       */
  lpTIedt = CFst_STI_Init(itEdt,0,FSTI_SORTINI);                                /* Init. eps. distance FST interator */

  /* Final checks */                                                            /* --------------------------------- */
  if (_this->m_nWsr!=FST_WSR_NONE)                                              /* If automata weighted              */
  {                                                                             /* >>                                */
    DLPASSERT(lpTIsrc->nOfTW>0);                                                /*   Source not weighted!            */
    DLPASSERT(lpTIedt->nOfTW>0);                                                /*   Eps.dist.FST not weighted!      */
  }                                                                             /* <<                                */

  /* Add pseudo start states to destination */                                  /* --------------------------------- */
  lpTedt = NULL;                                                                /* Init. eps. dist. transition ptr.  */
  while ((lpTedt=CFst_STI_TfromS(lpTIedt,0,lpTedt)))                            /* Enum epsilon paths from start st. */
    CFst_Erm_GetDestTerState(_this,lpTIsrc,*CFst_STI_TTer(lpTIedt,lpTedt));     /*   Add pseudo start state          */

  /* Build destination */                                                       /* --------------------------------- */
  for (nS=0; nS<UD_XS(_this,0); nS++)                                           /* Loop over newly added states      */
  {                                                                             /* >>                                */
    nSsrc = (FST_ITYPE)CData_Dfetch(AS(CData,_this->sd),nS,_this->m_nIcSdAux);  /*   Get corresponding state in src. */
    IFCHECK printf("\n   State %ld[%ld]: ",(long)nS,(long)nSsrc);               /*   Protocol (verbose level 1)      */
    lpTsrc = NULL;                                                              /*   Initilize source trans. pointer */
    while ((lpTsrc=CFst_STI_TfromS(lpTIsrc,nSsrc,lpTsrc)))                      /*   Follow transitions from src. st.*/
    {                                                                           /*   >>                              */
      if (*CFst_STI_TTis(lpTIsrc,lpTsrc)<0) continue;                           /*     Ignore epsilon transitions    */
      nTerSrc = *CFst_STI_TTer(lpTIsrc,lpTsrc);                                 /*     Get source terminal state     */
      bEpsWFZ = CFst_Erm_EpsDistFromStart(lpTIedt,nSsrc,&nEpsWFZ);              /*     Get epsilon weight from start */

      /* Add non-epsilon transition to destination */                           /*     - - - - - - - - - - - - - - - */
      if (SD_FLG(itSrc,lpTIsrc->nFS+nTerSrc)&SD_FLG_USER2)                      /*     Non-eps. trans. from nTerSrc  */
      {                                                                         /*     >>(leftover from CFst_Epsdist)*/
        nTerDst = CFst_Erm_GetDestTerState(_this,lpTIsrc,nTerSrc);              /*       Get (or add) dest.ter. state*/
        nTsrc   = CFst_STI_GetTransId(lpTIsrc,lpTsrc);                          /*       Get index of source trans.  */
        CFst_AddtransCopy(_this,0,nS,nTerDst,itSrc,nTsrc);                      /*       Copy source transition      */
        IFCHECK printf("\n     T %ld[%ld]->%ld[%ld]",                           /*       Protocol (verbose level 1)  */
          (long)nS,(long)nSsrc,(long)nTerDst,(long)nTerSrc);                    /*       |                           */
        if (bEpsWFZ)                                                            /*       Epsilon path 0 ==> nSsrc    */
        {                                                                       /*       >>                          */
          nT = CFst_AddtransCopy(_this,0,0,nTerDst,itSrc,nTsrc);                /*         Add transition from 0     */
          IFCHECK printf("\n     Z 0[0]->%ld[%ld]",(long)nTerDst,(long)nTerSrc);/*         Protocol (verbose lvl 1)  */
          if (_this->m_nWsr!=FST_WSR_NONE)                                      /*         If automata weighted      */
            TD_W(_this,nT,lpTIsrc->nOfTW) =                                     /*           Multiply with start wgt.*/
              CFst_Wsr_Op(_this,*CFst_STI_TW(lpTIsrc,lpTsrc),nEpsWFZ,OP_MULT);  /*           |                       */
        }                                                                       /*       <<                          */
      }                                                                         /*     <<                            */
                                                                                /*                                   */
      /* Add transitions to all states reachable through epsilon paths */       /*     - - - - - - - - - - - - - - - */
      lpTedt = NULL;                                                            /*     Init. eps. dist. trans. ptr.  */
      while ((lpTedt=CFst_STI_TfromS(lpTIedt,nTerSrc,lpTedt)))                  /*     Follow epsilon paths          */
      {                                                                         /*     >>                            */
        nTerEdt = *CFst_STI_TTer(lpTIedt,lpTedt);                               /*       Get source terminal state   */
        nTerDst = CFst_Erm_GetDestTerState(_this,lpTIsrc,nTerEdt);              /*       Get (or add) dest.term.state*/
        nTsrc   = CFst_STI_GetTransId(lpTIsrc,lpTsrc);                          /*       Get index of source trans.  */
        nT      = CFst_AddtransCopy(_this,0,nS,nTerDst,itSrc,nTsrc);            /*       Copy source transition      */
        IFCHECK                                                                 /*       Protocol (verbose level 1)  */
          printf("\n     E %ld[%ld]->[%ld]->%ld[%ld]",                          /*       |                           */
          (long)nS,(long)nSsrc,(long)nTerSrc,                                   /*       |                           */
            nTerDst,nTerEdt);                                                   /*       |                           */
        if (_this->m_nWsr!=FST_WSR_NONE)                                        /*       If automata weighted        */
        {
          nW = CFst_Wsr_Op                                                      /*         Weight of dest. trans. is:*/
          (                                                                     /*         |                         */
            _this,                                                              /*         | (knows the semiring)    */
            *CFst_STI_TW(lpTIsrc,lpTsrc),                                       /*         | (source trans. weight)  */
            *CFst_STI_TW(lpTIedt,lpTedt),                                       /*         | (eps. path weight)      */
            OP_MULT                                                             /*         | (multiplied)            */
          );                                                                    /*         |                         */
          TD_W(_this,nT,lpTIsrc->nOfTW) = nW;                                   /*         Store weight              */
        }                                                                       /*       <<                          */

        /* Additionally: add trans. 0 ==(eps)==(nTsrc)->nTerDst*/               /*       - - - - - - - - - - - - - - */
        if (bEpsWFZ)                                                            /*       Epsilon path 0 ==> nSsrc    */
        {                                                                       /*       >>                          */
          nT = CFst_AddtransCopy(_this,0,0,nTerDst,itSrc,nTsrc);                /*         Add transition from 0     */
          IFCHECK                                                               /*         Protocol (verbose lvl 1)  */
            printf("\n     Z 0[0]->[%ld]->%ld[%ld]",                            /*         |                         */
             (long)nTerSrc,(long)nTerDst,(long)nTerEdt);                        /*         |                         */
          if (_this->m_nWsr!=FST_WSR_NONE)                                      /*         If automata weighted      */
            TD_W(_this,nT,lpTIsrc->nOfTW) =                                     /*           Multiply with start wgt.*/
              CFst_Wsr_Op(_this,nW,nEpsWFZ,OP_MULT);                            /*           |                       */
        }                                                                       /*       <<                          */
      }                                                                         /*     <<                            */
    }                                                                           /*   <<                              */
  }                                                                             /* <<                                */

  /* Clean up */                                                                /* --------------------------------- */
  CFst_STI_Done(lpTIedt);                                                       /* Destroy eps. dist. FST iterator   */
  CFst_STI_Done(lpTIsrc);                                                       /* Destroy source iterator           */
  CData_DeleteComps(AS(CData,_this->sd),_this->m_nIcSdAux,1);                   /* Remove aux. state component       */
  CData_DeleteComps(AS(CData,itSrc->sd),itSrc->m_nIcSdAux,1);                   /* Remove aux. state component       */
  IDESTROY(itEdt);                                                              /* Destroy eps. distance transducer  */
  DESTROYVIRTUAL(itSrc,_this);                                                  /* Overlapping arguments support     */
  CFst_Check(_this);                                                            /* TODO: remove after debugging      */

  /* Protocol */                                                                /* --------------------------------- */
L_OK:                                                                           /* Jump label                        */
  IFCHECKEX(1)                                                                  /* On verbose level 1                */
  {                                                                             /* >>                                */
    printf("\n\n CFst_Epsremove done.");                                        /*   Print function identifier       */
    printf("\n"); dlp_fprint_x_line(stdout,'-',dlp_maxprintcols());             /*   Print separator                 */
    printf("\n");                                                               /*   ...                             */
  }                                                                             /* <<                                */

  return O_K;                                                                   /* jo!                               */
}

/* EOF */
