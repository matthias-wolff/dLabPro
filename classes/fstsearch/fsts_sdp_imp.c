/* dLabPro class CFstsearch (fstsearch)
 * - Synchroneous dynamic programming
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

#include "fsts_sdp_imp.h"
#define GEN_EXPORT
#define GEN_PUBLIC
#define GEN_PRIVATE

#define IFDOPRINT(A,B) \
  if (BASEINST(_this)->m_nCheck>=A && ((_this->m_nPrintstop>=0 && _this->m_nPrintstop<=B) || (INT16)_this->m_nPrintstop==-2))

#define FLG_EPS    0x1
#define FLG_FWD    0x2

/**
 * Initialize backtracking tree.
 *
 * @param iFst Pointer to automaton iterator data struct
 * @return     A pointer to a backtracking tree data struct or <code>NULL</code> in case of an error
 */
FST_BT_TYPE* GEN_EXPORT CFst_Sdp_BtInit(FST_TID_TYPE* lpTI, INT32 nGrany)
{
  FST_BT_TYPE* lpBT = NULL;                                                     /* Backtracking data struct (return) */

  if (!lpTI || !lpTI->iFst) return NULL;                                        /* Return if FST instance invalid    */

  lpBT =                                                                        /* Allocate data struct              */
    (FST_BT_TYPE*)__dlp_calloc(1,sizeof(FST_BT_TYPE),__FILE__,__LINE__,         /* |                                 */
      BASEINST(lpTI->iFst)->m_lpClassName,                                      /* |                                 */
      BASEINST(lpTI->iFst)->m_lpInstanceName);                                  /* |                                 */
  if (!lpBT) return NULL;                                                       /* Out of memory                     */
  lpBT->lpTI   = lpTI;                                                          /* Store FST instance                */
  lpBT->nGrany = nGrany;                                                        /* Store allocation granularity      */
  lpBT->nLen   = 0;                                                             /* Initial length is 0               */
  lpBT->lpVec  = NULL;                                                          /* Zero-initialize vector buffer     */
  return lpBT;                                                                  /* Return pointer to data struct     */
}

/**
 * Destroy backtracking tree.
 *
 * @param lpBT Pointer to backtracking tree data struct. The pointer will become invalid!
 */
void GEN_EXPORT CFst_Sdp_BtDone(FST_BT_TYPE* lpBT)
{
  INT32 nV = 0;                                                                 /* Vector index                       */

  if (!lpBT) return;                                                           /* Return if lpBT==NULL               */
  for (nV=0; nV<(INT32)(dlp_size(lpBT->lpVec)/sizeof(BYTE**)); nV++)            /* For all elements of vector buffer  */
    dlp_free(lpBT->lpVec[nV]);                                                 /*   Free transition vector           */
  dlp_free(lpBT->lpVec);                                                       /* Free vector buffer                 */
  dlp_free(lpBT);                                                              /* Free data struct                   */
}

/**
 * Store a transition in the backtracking tree
 *
 * @param lpBT Pointer to backtracking tree data struct
 * @param lpT  The transition to store
 * @param bEps If <code>TRUE</code> only store epsilon transitions, if <code>FALSE</code> only
 *                store non-epsilon transitions
 */
void GEN_EXPORT CFst_Sdp_BtStore(FST_BT_TYPE* lpBT, BYTE* lpT, BYTE* lpTprev, INT16 bEps)
{
  INT32 nV = 0;                                                                 /* Vector index                       */
  INT32 nE = 0;                                                                 /* Element index in vector            */

  if (!lpBT) return;                                                           /* Return if lpBT==NULL               */
  /* This line will obmit transitions if a worse path uses the same
   * epsilon transition as a better path one epsilon layer later.
   * In result the gw flag is set correct but backtracking gives
   * a wrong path.
  if (bEps && lpTprev==lpT) return; */                                         /* Do not store duplicate eps. trans. */
  if (lpT!=(BYTE*)-1 && lpT!=(BYTE*)-2)                                        /* Normal transition?                 */
  {                                                                            /* >>                                 */
    if ( bEps && *CFst_STI_TTis(lpBT->lpTI,lpT)!=-1) return;                   /*   Filter non-epsilon transintions  */
    if (!bEps && *CFst_STI_TTis(lpBT->lpTI,lpT)==-1) return;                   /*   Filter epsilon transintions      */
  }                                                                            /* <<                                 */

  nV = lpBT->nLen/lpBT->nGrany;                                                /* Calculate write address (vector)   */
  nE = lpBT->nLen%lpBT->nGrany;                                                /* Calculate write address (element)  */

  DLPASSERT(nV<=(INT32)(dlp_size(lpBT->lpVec)/sizeof(BYTE**)));                 /* Address error!                     */
  if (nV==(INT32)(dlp_size(lpBT->lpVec)/sizeof(BYTE**)))                        /* Need new transition vector ?       */
  {                                                                            /* >>                                 */
    lpBT->lpVec =                                                              /*   Reallocate vector buffer         */
      (BYTE***)__dlp_realloc(lpBT->lpVec,nV+1,sizeof(BYTE**),__FILE__,__LINE__,/*   |                                */
        BASEINST(lpBT->lpTI->iFst)->m_lpClassName,                             /*   |                                */
        BASEINST(lpBT->lpTI->iFst)->m_lpInstanceName);                         /*   |                                */
    lpBT->lpVec[nV] =                                                          /*   Allocate a new transition vector */
      (BYTE**)__dlp_calloc(lpBT->nGrany,sizeof(BYTE*),__FILE__,__LINE__,       /*   |                                */
        BASEINST(lpBT->lpTI->iFst)->m_lpClassName,                             /*   |                                */
        BASEINST(lpBT->lpTI->iFst)->m_lpInstanceName);                         /*   |                                */
    DLPASSERT(lpBT->lpVec[nV]);                                                /*   TODO: Check out-of-memory!       */
  }                                                                            /* <<                                 */
  lpBT->lpVec[nV][nE]=lpT;                                                     /* Store transition                   */
  lpBT->nLen++;                                                                /* Increment tree length              */
}

/**
 * Fetch a transition from the backtracking tree
 *
 * @param lpBT Pointer to backtracking tree data struct
 * @param nT   The index of the transition to fetch
 */
BYTE* GEN_EXPORT CFst_Sdp_BtFetch(FST_BT_TYPE* lpBT, INT32 nT)
{
  INT32 nV = 0;                                                                 /* Vector index                       */
  INT32 nE = 0;                                                                 /* Element index in vector            */

  if (!lpBT) return NULL;                                                      /* Return NULL if lpBT==NULL          */
  nV = nT/lpBT->nGrany;                                                        /* Calculate write address (vector)   */
  nE = nT%lpBT->nGrany;                                                        /* Calculate write address (element)  */

  DLPASSERT(nV>=0 && nE>=0);                                                   /* Address error!                     */
  DLPASSERT(nV<(INT32)(dlp_size(lpBT->lpVec)/sizeof(BYTE**))&&nE<lpBT->nGrany); /* Address error!                     */
  if (nV<0 || nE<0) return NULL;                                               /* On address error return NULL       */
  if (nV>=(INT32)(dlp_size(lpBT->lpVec)/sizeof(BYTE**)) || nE>=lpBT->nGrany)    /* On address error ...               */
    return NULL;                                                               /*   Return NULL                      */

  return lpBT->lpVec[nV][nE];                                                  /* Return transition                  */
}

/**
 * Prints the backtracking tree.
 *
 * @param lpBT Pointer to backtracking tree data struct
 */
void GEN_EXPORT CFst_Sdp_BtPrint(FST_BT_TYPE* lpBT)
{
  INT32  nT         = 0;                                                        /* Current transition index in tree   */
  BYTE* lpT        = NULL;                                                     /* Current transition pointer         */
  INT32  nXL        = 0;                                                        /* Layer counter                      */
  INT32  nXT        = 0;                                                        /* Sync. time counter                 */
  INT16 nPrintstop = 0;                                                        /* Printstop flag                     */

  for (nT=0,nXL=-1,nXT=-1; nT<lpBT->nLen; nT++)                                /* For all transitions in tree        */
  {                                                                            /* >>                                 */
    lpT = CFst_Sdp_BtFetch(lpBT,nT);                                           /*   Get transition pointer           */
    if (lpT==(BYTE*)-1)                                                        /*   Layer boundary marker?           */
    {                                                                          /*   >>                               */
      nXT++; nXL++;                                                            /*     Count sync. time and layer     */
      printf("\n   %6ld: === LAYER BOUNDARY === Time %ld, Layer %ld ===",      /*     Protocol                       */
      (long)nT,(long)nXT,(long)nXL);                                           /*     |                              */
      if (nPrintstop!=-2)                                                      /*     Not in non-stop mode?          */
      {                                                                        /*     >>                             */
        printf("\n    continue <cr>, no print -1, nonstop -2: ");              /*       Prompt                       */
        dlp_getx(T_SHORT,&nPrintstop);                                         /*       Read user input              */
        if (nPrintstop==-1) return;                                            /*       User requested stop ->return */
      }                                                                        /*     <<                             */
    }                                                                          /*   <<                               */
    else if (lpT==(BYTE*)-2)                                                   /*   Epsilon layer marker?            */
    {                                                                          /*   >>                               */
      nXL++;                                                                   /*     Count layer                    */
      printf("\n   %6ld: --- EPSILON LAYER --- Time %ld, Layer %ld ---",       /*     Protocol                       */
      (long)nT,(long)nXT,(long)nXL);                                           /*     |                              */
    }                                                                          /*   <<                               */
    else if (lpT!=NULL)                                                        /*   Valid transition pointer?        */
      printf                                                                   /*     Protocol                       */
      (                                                                        /*     |                              */
        "\n   %6ld: %3ld --(%3ld | %5.5g)--> %3ld",(long)nT,                   /*     |                              */
        (long)*CFst_STI_TIni(lpBT->lpTI,lpT),                                  /*     |                              */
        (long)*CFst_STI_TTis(lpBT->lpTI,lpT),                                  /*     |                              */
        (double)*CFst_STI_TW  (lpBT->lpTI,lpT),                                /*     |                              */
        (long)*CFst_STI_TTer(lpBT->lpTI,lpT)                                   /*     |                              */
      );                                                                       /*     |                              */
    else DLPASSERT(FALSE);                                                     /*   Invalid entry in tree            */
  }                                                                            /* <<                                 */
}

/**
 * Load synchroneous transition weights for time t.
 *
 * @param _this Pointer to automaton instance to get weights for
 * @param lpSWa Pointer to matrix holding synchroneous weights
 * @param t     Time (column index in <code>lpSWa</code>)
 * @param nXW   Number of weights (number of rows in <code>lpSWa</code>)
 * @param lpSW  Pointer to array to be filled with synchroneous transition
 *              weights (values of line <code>t</code> in matrix
 *              <code>lpSWa</code>).
 */
void GEN_PRIVATE CFst_Sdp_GetSWeights
(
  CFst*         _this,
  FST_WTYPE*    lpSWa,
  INT32          t,
  INT32          nXW,
  FST_WTYPE**   lpSW
)
{
  IFCHECKEX(2) printf("\n     Load sync. weights for time %ld",(long)t);        /* Protocol                          */

  if (lpSWa) *lpSW=lpSWa+t*nXW;                                                 /* If weight matrix present          */
  else *lpSW=NULL;                                                              /* No weights found                  */
}

/**
 * Clears a DP layer buffer.
 *
 * @param _this Pointer to automaton instance
 * @param lpLB  Pointer to layer buffer to clear
 * @param nXS   Length of layer buffer
 */
void GEN_PRIVATE CFst_Sdp_ClearLB(CFst* _this, FST_LB_TYPE* lpLB, INT32 nXS)   /* CFst_Sdp_ClearLB ================ */
{
  INT32      nS    = 0;                                                         /* State counter                     */
  FST_WTYPE nZero = CFst_Wsr_NeAdd(_this->m_nWsr);                              /* Get semiring zero                 */
  for (nS=0; nS<nXS; nS++)                                                      /* For all states ...                */
  {                                                                             /* >>                                */
    lpLB[nS].lpT = NULL;                                                        /*   No transition lead here         */
    lpLB[nS].nD  = nZero;                                                       /*   Infinite distance from start    */
  }                                                                             /* <<                                */
}

/**
 * Expands one state during layer traversal.
 *
 * <h4>Remarks</h4>
 * <ol>
 *   <li>2007-07-20: optimized for speed (removed all sub-function calls)</li>
 * </ol>
 *
 * @param _this  Pointer to automaton instance
 * @param lpTI   Pointer to automaton iterator data struct
 * @param lpSW   Pointer to array of synchroneous transition weights for current time
 * @param nS     Index of state to be expanded
 * @param t      Current time
 * @param nD     Distance of state <code>nS</code> from start state
 * @param lpLBwr Pointer to write layer buffer
 * @param bFlags FLG_EPS: process epsilon-transitions (input symbol) only, FLG_FWD: do forward algorithm
 * @return       The number of newly found shortes distances to successing states
 */
INT32 GEN_PRIVATE CFst_Sdp_ExpandState
(
  CFst*         _this,
  FST_TID_TYPE* lpTI,
  FST_WTYPE*    lpSW,
  INT32          t,
  FST_ITYPE     nS,
  FST_WTYPE     nD,
  FST_LB_TYPE*  lpLBwr,
  INT16         bFlags
)
{
  register INT32      nCtr = 0;                                                  /* Counter (return value)             */
  register BYTE*     lpT  = NULL;                                               /* Current transition with ...        */
  register FST_STYPE nTis = -1;                                                 /* - input symbol                     */
  register FST_ITYPE nTer = -1;                                                 /* - terminal state                   */
  register FST_WTYPE nAw  = 0.;                                                 /* - asynchroneous weight             */
  register FST_WTYPE nSw  = 0.;                                                 /* - synchroneous weight              */
  register FST_WTYPE nW   = 0.;                                                 /* Distance of term. state from start */
  register BOOL      bOpt = FALSE;                                              /* Optimal path flag                  */

  IFDOPRINT(2,t) printf("\n       Expanding state %3ld [%lg]",nS,nD);           /* Protocol                           */
  /* NOTE: assuming strictly sorted transition list; will fail otherwise! */    /*                                    */
  for                                                                           /* For all transitions leaving nS ... */
  (                                                                             /* |                                  */
    lpT = lpTI->lpFT + lpTI->nRlt*(lpTI->iFst->m_lpFts[nS]-lpTI->nFT);          /* | Ptr. to first transition at nS   */
    ;                                                                           /* | (in body)                        */
    lpT += lpTI->nRlt                                                           /* | Ptr. to next transition          */
  )                                                                             /* |                                  */
  {                                                                             /* >>                                 */
    if((lpT - AS(CData,lpTI->iFst->td)->m_lpTable->m_theDataPointer)            /* TODO: HACK - avoid invalid read    */
        >= (AS(CData,lpTI->iFst->td)->m_lpTable->m_nrec                         /*              outside boundaries    */
            *AS(CData,lpTI->iFst->td)->m_lpTable->m_reclen)) break;             /*       HACK - end (Guntram)         */
    if (!lpT || (*(FST_ITYPE*)(lpT+lpTI->nOfTIni)!=nS)) break;                  /*   Not a transition (at nS)         */
    nTis = *(FST_STYPE*)(lpT+lpTI->nOfTTis);                                    /*   Get input symbol                 */
    if ( (bFlags&FLG_EPS) && (nTis!=-1)) continue;                              /*   Epsilon mode: skip non-epsilon   */
    if (!(bFlags&FLG_EPS) && (nTis==-1)) continue;                              /*   Non-epsilon mode: skip epsilon   */
    nTer = *(FST_ITYPE*)(lpT+lpTI->nOfTTer);                                    /*   Get terminal state               */
    nAw  = *(FST_WTYPE*)(lpT+lpTI->nOfTW  );                                    /*   Get asynchroneous weight         */
    nSw  = (!lpSW || nTis<0)?CFst_Wsr_NeMult(_this->m_nWsr):lpSW[nTis];         /*   Get synchroneous weight          */
    if (_this->m_nWsr==FST_WSR_PROB)                                            /*   Probability weight semiring      */
    {                                                                           /*   >>                               */
      nW   = nD*nAw*nSw;                                                        /*     Dist. of term. state from start*/
      if(bFlags&FLG_FWD){ nW += lpLBwr[nTer].nD; bOpt=TRUE; } else              /*     Sum paths                      */
        bOpt = (nW > lpLBwr[nTer].nD);                                          /*     Most probable path?            */
    }                                                                           /*   <<                               */
    else                                                                        /*   Log. or tropical weight semiring */
    {                                                                           /*   >>                               */
      nW   = nD+nAw+nSw;                                                        /*     Dist. of term. state from start*/
      if(bFlags&FLG_FWD){                                                       /*     If we do forward calculation >>*/
        nW = CFst_Wsr_Op(_this,nW,lpLBwr[nTer].nD,OP_ADD);                      /*     Sum up weigth (min for tsr)    */
        bOpt=TRUE;                                                              /*     Allways probable path          */
      }else bOpt = (nW < lpLBwr[nTer].nD);                                      /*     Shortest path?                 */
    }                                                                           /*   <<                               */
    IFDOPRINT(3,t)                                                              /*   Protocol                         */
      printf("\n       --(%3ld | %5.5lg o %5.5lg)--> %3ld [%5.5lg]",            /*   |                                */
        nTis,nAw,nSw,nTer,nW);                                                  /*   |                                */
    if (bOpt)                                                                   /*   Optimal path                     */
    {                                                                           /*   >>                               */
      IFDOPRINT(3,t)                                                            /*     On verbose lvl.3 when printing */
      {                                                                         /*     >>                             */
        printf(" > %5.5lg *",lpLBwr[nTer].nD);                                  /*       Protocol beam update         */
        if (SD_FLG(lpTI->iFst,nTer+lpTI->nFS)&SD_FLG_FINAL) printf(" PATH END");/*       Protocol path ends           */
      }                                                                         /*     <<                             */
      lpLBwr[nTer].lpT = lpT;                                                   /*     Remember current transition... */
      lpLBwr[nTer].nD  = nW;                                                    /*     ... and distance               */
      nCtr++;                                                                   /*     Count                          */
    }                                                                           /*   <<                               */
  }                                                                             /* <<                                 */
  IFDOPRINT(3,t) printf("\n       %ld beam%s updated",(long)nCtr,nCtr!=1?"s":"");/*     Protocol                       */
  return nCtr;                                                                  /* Return counter                     */
}                                                                               /*                                    */

/**
 * Expand one layer during layer traversal.
 *
 * @param _this  Pointer to automaton instance
 * @param lpTI   Pointer to automaton iterator data struct
 * @param lpSW   Pointer to array of synchroneous transition weights for current time
 * @param t      Current time
 * @param lpLBrd Pointer to read layer buffer pointer
 * @param lpLBwr Pointer to write layer buffer pointer
 * @param lpBT   Pointer to backtracking tree
 * @param bFlags FLG_EPS: process epsilon-transitions (input symbol) only, FLG_FWD: do forward algorithm
 * @return       The new number of active states (beam width)
 */
INT32 GEN_PRIVATE CFst_Sdp_ExpandLayer
(
  CFst*         _this,
  FST_TID_TYPE* lpTI,
  FST_WTYPE*    lpSW,
  INT32          t,
  INT32          nTmax,
  FST_LB_TYPE** lpLBrd,
  FST_LB_TYPE** lpLBwr,
  FST_BT_TYPE*  lpBT,
  INT16         bFlags
)
{
  /* Local variables */                                                         /* --------------------------------- */
  FST_ITYPE    nS    = 0;                                                       /* Current state                     */
  FLOAT64       nAS   = 0.;                                                       /* Number of active states           */
  INT32         nCtr  = 0;                                                       /* No. of succ. states (beamwidth)   */
  INT32         nPrn  = 0;                                                       /* Pruned beams counter              */
  FST_WTYPE    nPrnT = 0.;                                                      /* Pruning threshold                 */
  FST_LB_TYPE* lpAux = NULL;                                                    /* Swap buffer                       */

  /* Protocol */                                                                /* --------------------------------- */
  IFCHECKEX(1) printf("\n     Expanding %slayer: ",bFlags&FLG_EPS?"epsilon ":"");/* Protocol                         */

  /* Initialize epsilon layer */                                                /* --------------------------------- */
  if (bFlags&FLG_EPS)                                                           /* If epsilon layer                  */
    for (nS=0; nS<lpTI->nXS; nS++)                                              /*   For all states                  */
      dlp_memmove(&(*lpLBwr)[nS],&(*lpLBrd)[nS],sizeof(FST_LB_TYPE));           /*     Copy read layer buffer        */

  /* Initialize pruning */                                                      /* --------------------------------- */
  if (_this->m_bPrune)                                                          /* Option /cprune set                */
  {                                                                             /* >>                                */
    FLOAT64 nMin = T_DOUBLE_MAX;                                                 /*   Maximum global weight           */
    FLOAT64 nMax = T_DOUBLE_MIN;                                                 /*   Minimum global weight           */
    for (nS=0; nS<lpTI->nXS; nS++)                                              /*   For all states ...              */
      if ((*lpLBrd)[nS].lpT!=NULL)                                              /*     Being active ...              */
      {                                                                         /*     >>                            */
        nAS++;                                                                  /*       Count active states         */
        if ((*lpLBrd)[nS].nD>nMax) nMax = (*lpLBrd)[nS].nD;                     /*       Comp. maximum global weight */
        if ((*lpLBrd)[nS].nD<nMin) nMin = (*lpLBrd)[nS].nD;                     /*       Comp. minimum global weight */
      }                                                                         /*     <<                            */
    if (nAS<FST_SDP_KEEPA) nAS=-1.; else nAS/=lpTI->nXS;                        /*   Enforce KEEPA, comp. % active   */
    nPrnT = nMax-_this->m_nPrnConst*(nMax-nMin);                                /*   Compute pruning threshold       */
    IFCHECKEX(1) printf("Prune @: %lg (min=%lg, max=%lg)",nPrnT,nMin,nMax);     /*   Protocol (pruning threshold)    */
  }                                                                             /* <<                                */

  /* Expand states */                                                           /* --------------------------------- */
  for (nS=0; nS<lpTI->nXS; nS++)                                                /* For all states ...                */
    if ((*lpLBrd)[nS].lpT!=NULL)                                                /*   Being active ...                */
    {                                                                           /*   >>                              */
      if                                                                        /*     Pruning state if ...          */
      (                                                                         /*     |                             */
        _this->m_bPrune                                                      && /*     | Pruning is active           */
        t>FST_SDP_DMZ && t<nTmax-FST_SDP_DMZ                                 && /*     | Outside demilitarized zone  */
        nAS>FST_SDP_KEEPR                                                    && /*     | Enforce KEEPR               */
        CFst_Wsr_Op(_this,(*lpLBrd)[nS].nD,nPrnT,OP_LESS)                       /*     | Above pruning threshold     */
      )                                                                         /*     |                             */
      {                                                                         /*     >>                            */
        IFDOPRINT(2,t)                                                          /*       Protocol                    */
          printf("\n       PRUNING state %3ld [%lg]",nS,(*lpLBrd)[nS].nD);      /*       |                           */
        (*lpLBrd)[nS].lpT = NULL;                                               /*       Clear transition pointer    */
        nPrn++;                                                                 /*       Count pruned beams          */
      }                                                                         /*     <<                            */
      else                                                                      /*     Keeping state...              */
        nCtr+=                                                                  /*       expand                      */
          CFst_Sdp_ExpandState(_this,lpTI,lpSW,t,nS,(*lpLBrd)[nS].nD,*lpLBwr,   /*       |                           */
            bFlags);                                                            /*       |                           */
    }                                                                           /*   <<                              */

  /* Prune write layer buffer */                                                /* --------------------------------- */
  /* TODO: cbw pruning! */

  if(!(bFlags&FLG_FWD))                                                         /* Backtracking needed ?             */
  {                                                                             /* >>                                */
    /* Lay traces */                                                            /* --------------------------------- */
    for (nS=0; nS<lpTI->nXS; nS++)                                              /* For all states...                 */
      if ((*lpLBwr)[nS].lpT!=NULL)                                              /*   Being active...                 */
        CFst_Sdp_BtStore(lpBT,(*lpLBwr)[nS].lpT,(*lpLBrd)[nS].lpT,bFlags&FLG_EPS);/*   Store trace                   */
    CFst_Sdp_BtStore(lpBT,bFlags&FLG_EPS?(BYTE*)-2:(BYTE*)-1,NULL,bFlags&FLG_EPS);/* Store layer boundary            */
  }                                                                             /* <<                                */

/*  printf("\n Expand layer %3i (%s): ",t,bFlags&FLG_EPS?"eps":"---");
  printf("\n   "); for(nS=0;nS<lpTI->nXS;nS++) printf(" %9.4g",(*lpLBrd)[nS].nD);
  printf("\n   "); for(nS=0;nS<lpTI->nXS;nS++) printf(" %9.4g",(*lpLBwr)[nS].nD);*/

  /* Switch layer */                                                           /* ---------------------------------- */
  lpAux   = *lpLBrd;                                                           /* Remember read layer buffer pointer */
  *lpLBrd = *lpLBwr;                                                           /* Set read to write                  */
  *lpLBwr = lpAux;                                                             /* Set write to read                  */
  CFst_Sdp_ClearLB(_this,*lpLBwr,lpTI->nXS);                                   /* Clear write layer buffer           */

  /* Protocol */                                                               /* ---------------------------------- */
  IFCHECKEX(1)                                                                 /* On verbose level 1                 */
  {                                                                            /* >>                                 */
    INT32 nCtr2;                                                               /*   Auxilary counter                 */
    for (nS=0,nCtr2=0; nS<lpTI->nXS; nS++) if ((*lpLBrd)[nS].lpT) nCtr2++;     /*   Count total beamwidth            */
    IFCHECKEX(2) printf("\n    ");                                             /*   Print protocol                   */
    printf(" Beamwidth: %ld",(long)nCtr2);                                     /*   Print protocol                   */
    if (nPrn) printf(" (pruned: %ld)",(long)nPrn);                             /*   Print protocol                   */
  }                                                                            /* <<                                 */
  IFDOPRINT(3,t)                                                               /* On verbose level 3 when printing   */
    if ((INT16)_this->m_nPrintstop!=-2)                                        /*   If not in nonstop mode           */
    {                                                                          /*   >>                               */
      printf("\n    continue <cr>, no print -1, nonstop -2: ");                /*     Prompt                         */
      dlp_getx(T_SHORT,&_this->m_nPrintstop);                                  /*     Read user input                */
    }                                                                          /*   <<                               */

  return (bFlags&FLG_EPS)&&(bFlags&FLG_FWD)?0:nCtr;  /* TODO: break criterion */         /* Return beamwidth                   */
}

/**
 *
 */
INT16 GEN_PRIVATE CFst_Sdp_Backtracking
(
  CFst*        _this,
  FST_BT_TYPE* lpBT,
  INT32         nET,
  INT32         nEL,
  FST_ITYPE    nES,
  FST_WTYPE    nED,
  CData*       idWeights
)
{
  INT32      t      = 0;                                                        /* Current time                       */
  INT32      l      = 0;                                                        /* Current layer                      */
  INT32      nState = 0;                                                        /* Current newly added state          */
  INT32      nTrans = 0;                                                        /* Current newly added transition     */
  INT32      nRls   = 0;                                                        /* Record length of state table       */
  INT32      nRlt   = 0;                                                        /* Record length of trans. table      */
  INT32      nT     = 0;                                                        /* Current trace with                 */
  BYTE*     lpT    = NULL;                                                     /* - transition pointer               */
  BYTE*     lpS    = NULL;                                                     /* Pointer to current state           */
  INT16     bNel   = FALSE;                                                    /* Non-epsilon layer flag             */
  INT32      nFC    = 0;                                                        /* First numeric comp. in idWeights   */
  FST_WTYPE nSW    = 0.;                                                       /* Synchroneous weight of curr. trans.*/
  FST_WTYPE nW     = 0.;                                                       /* Asynch. weight if curr. trans.     */
  FST_STYPE nTis   = 0;                                                        /* Input symbols of curr. trans.      */

  /* Backtracking */                                                           /* ---------------------------------- */
  IFCHECKEX(1)                                                                 /* On verbose level 1                 */
  {                                                                            /* >>                                 */
    printf("\n Backtracking");                                                 /*   Protocol                         */
    printf("\n   Tree size: %ldk transitions",(long)lpBT->nLen/1000);          /*   Protocol                         */
    printf("\n   End time : %ld",(long)nET);                                   /*   Protocol                         */
    printf("\n   End layer: %ld",(long)nEL);                                   /*   Protocol                         */
    printf("\n   End state: %ld [%5.5g]",(long)nES,(double)nED);               /*   Protocol                         */
  }                                                                            /* <<                                 */
  IFCHECKEX(3) CFst_Sdp_BtPrint(lpBT);                                         /* On verbose level 3: print BT tree  */

  /* Initialize */                                                             /* ---------------------------------- */
  nRls = CData_GetRecLen(AS(CData,_this->sd));                                 /* Get record length of state table   */
  nRlt = CData_GetRecLen(AS(CData,_this->td));                                 /* Get record length of trans. table  */
  for (nFC=0; nFC<CData_GetNComps(idWeights); nFC++)                           /* Seek first num. sync. wieght comp. */
    if (dlp_is_numeric_type_code(CData_GetCompType(idWeights,nFC)))            /* ...                                */
      break;                                                                   /* ...                                */

  /* Seek end of best path layer */                                            /* ---------------------------------- */
  for (nT=0,t=-1,l=-1; nT<lpBT->nLen; nT++)                                    /* For all traces                     */
  {                                                                            /* >>                                 */
    lpT = CFst_Sdp_BtFetch(lpBT,nT);                                           /*   Get transition                   */
    if      (lpT==(BYTE*)-1) { t++; l++; }                                     /*   Count layer breaks and sync.time */
    else if (lpT==(BYTE*)-2) { l++; }                                          /*   Count layer breaks               */
    else if (t==nET && l==nEL) break;                                          /*   Stop on end of best path's layer */
  }                                                                            /* <<                                 */
  if (nT==lpBT->nLen) nT--;                                                    /* Reset to last valid trace is necc. */
  IFCHECKEX(1) printf("\n   Starting at trace %ld",(long)nT);                  /* Protocol                           */

  /* Follow trace */                                                           /* ---------------------------------- */
  for (t=nET; nT>=0; nT--)                                                     /* Backward for all remaining traces  */
  {                                                                            /* >>                                 */
    bNel=FALSE;                                                                /*   Reset non-epsilon layer flag     */

    /* Seek next trace */                                                      /*  - - - - - - - - - - - - - - - - - */
    for (; nT>=0; nT--)                                                        /*   Bkw. for all remaining traces    */
    {                                                                          /*   >>                               */
      lpT = CFst_Sdp_BtFetch(lpBT,nT);                                         /*     Get transition                 */
      if (lpT==(BYTE*)-1) { bNel=TRUE; continue; }                             /*     Detect non-epsilon layer state */
      if ((lpT==(BYTE*)-2 || lpT==(BYTE*)-1) && bNel)                          /*     Unexpected layer boundary      */
      {                                                                        /*     >>                             */
        DLPASSERT(FMSG("Broken path!"));                                       /*       TODO: error msg. and return! */
      }                                                                        /*     <<                             */
      if (lpT!=(BYTE*)-2 && lpT!=(BYTE*)-1)                                    /*     Normal transition              */
        if (*CFst_STI_TTer(lpBT->lpTI,lpT)==nES)                               /*       Continuing trace             */
        {                                                                      /*       >>                           */
          lpS =                                                                /*         Get pointer to src. state  */
            CData_XAddr(AS(CData,lpBT->lpTI->iFst->sd),                        /*         |                          */
              *CFst_STI_TIni(lpBT->lpTI,lpT)+lpBT->lpTI->nFS,0);               /*         |                          */
          nState = CData_AddRecs(AS(CData,_this->sd),1,_this->m_nGrany);       /*         Add a new dest. state      */
          nTrans = CData_AddRecs(AS(CData,_this->td),1,_this->m_nGrany);       /*         Add a new dest. transition */
          dlp_memmove(CData_XAddr(AS(CData,_this->sd),nState,0),lpS,nRls);     /*         Copy data from src. state  */
          dlp_memmove(CData_XAddr(AS(CData,_this->td),nTrans,0),lpT,nRlt);     /*         Copy data from src. trans. */
          nW = *CFst_STI_TW(lpBT->lpTI,lpT);

          if (idWeights && !CData_IsEmpty(idWeights))
            if ((nTis=*CFst_STI_TTis(lpBT->lpTI,lpT))>=0)
            {
              nSW = (FST_WTYPE)CData_Dfetch(idWeights,t,nFC+nTis);
              nW  = CFst_Wsr_Op(_this,nW,nSW,OP_MULT);
              *(FST_WTYPE*)CData_XAddr(AS(CData,_this->td),nTrans,_this->m_nIcW)=nW;
              t--;
            }

          TD_INI(_this,nTrans)=nState-1;                                       /*         Set initial state          */
          TD_TER(_this,nTrans)=nState;                                         /*         Set terminal state         */
          UD_XS(_this,0)++;                                                    /*         Inc. state count of unit 0 */
          UD_XT(_this,0)++;                                                    /*         Inc. trans. count of unit 0*/
          nES = *CFst_STI_TIni(lpBT->lpTI,lpT);                                /*         Next ter. state to search  */
          IFCHECKEX(1)                                                         /*         On verbose level 1         */
            printf("\n   Trace %6ld: %ld -(%ld|%g)-> %ld",(long)nT,            /*           Protocol                 */
              (long)*CFst_STI_TIni(lpBT->lpTI,lpT),                            /*           |                        */
              (long)*CFst_STI_TTis(lpBT->lpTI,lpT),                            /*           |                        */
              (double)nW,                                                      /*           |                        */
              (long)*CFst_STI_TTer(lpBT->lpTI,lpT));                           /*           |                        */
          nT--;                                                                /*         Goto previous trace ...    */
          break;                                                               /*         ... and break loop         */
        }                                                                      /*       <<                           */
    }                                                                          /*   <<                               */

    /* Seek next layer boundary */                                             /*  - - - - - - - - - - - - - - - - - */
    for (; nT>=0; nT--)                                                        /*   Bkw. for all remaining traces    */
    {                                                                          /*   >>                               */
      lpT = CFst_Sdp_BtFetch(lpBT,nT);                                         /*     Get transition                 */
      if (lpT==(BYTE*)-2 || lpT==(BYTE*)-1)                                    /*     If found layer break           */
      {                                                                        /*     >>                             */
        IFCHECKEX(2) printf("\n   Layer break %ld",(long)nT);                  /*       Protocol on verbose level 2  */
        break;                                                                 /*       Break loop                   */
      }                                                                        /*     <<                             */
    }                                                                          /*   <<                               */
  }                                                                            /* <<                                 */

  return O_K;                                                                  /* That's it                          */
}

/**
 * Orders the resulting path topologically.
 */
void GEN_PRIVATE CFst_Sdp_Order(CFst* _this)
{
/*MWX 2006-09-02: Optimized --> */
  /*CFst_Order(_this,_this,NULL,0,0); */                                         /* Order result                       */
  /*CFst_STI_Done(CFst_STI_Init(_this,0,FSTI_SORTINI)); */                       /* Sort transition list               */
/* <-- */

  FST_TID_TYPE* lpTI   =NULL;                                                   /* Source FST iterator               */
  INT32*         lpOrd = NULL;                                                   /* State ordering index array        */
  BYTE*         lpT   = NULL;                                                   /* Pointer to current transition     */
  BYTE*         lpSd  = NULL;                                                   /* State quali. copy buffer          */
  FST_ITYPE     nT    = 0;                                                      /* Current transition                */
  FST_ITYPE     nS    = 0;                                                      /* Current state                     */
  FST_ITYPE     nSn   = 0;                                                      /* Current new state index (ordering)*/
  INT32          nRln  = 0;                                                      /* Record length od sd (in bytes)    */

  nRln  = CData_GetRecLen(AS(CData,_this->sd));                                 /* Get record length                 */
  lpTI  = CFst_STI_Init(_this,0,FSTI_SORTINI);                                  /* Create a sorted iterator          */
  lpOrd = (INT32*)dlp_calloc(UD_XS(_this,0),sizeof(INT32));                       /* Create state ordering index array */
  lpT   = NULL;                                                                 /* Initialize current transition ptr.*/
  while ((lpT=CFst_STI_TfromS(lpTI,nS,NULL))!=NULL)                             /* Enumerate the (unique!) path      */
  {                                                                             /* >>                                */
    nS        = *CFst_STI_TTer(lpTI,lpT);                                       /*   Get next state of path          */
    lpOrd[nS] = ++nSn;                                                          /*   Remember new (ordered) state id.*/
  }                                                                             /* <<                                */
  lpSd = (BYTE*)dlp_calloc(UD_XS(_this,0)*nRln,1);                              /* Allocate state quali. copy buffer */
  dlp_memmove(lpSd,CData_XAddr(AS(CData,_this->sd),0,0),UD_XS(_this,0)*nRln);   /* Copy state quali.                 */
  for (nS=UD_FS(_this,0); nS<UD_FS(_this,0)+UD_XS(_this,0); nS++)               /* Loop over all states              */
    dlp_memmove(CData_XAddr(AS(CData,_this->sd),lpOrd[nS],0),&lpSd[nS*nRln],    /*   Copy new state quali.           */
      nRln);                                                                    /*   |                               */
  dlp_free(lpSd);                                                               /* Free state quali. buffer          */
  for (nT=UD_FT(_this,0); nT<UD_FT(_this,0)+UD_XT(_this,0); nT++)               /* Loop over all transitions         */
  {                                                                             /* >>                                */
    TD_INI(_this,nT) = lpOrd[TD_INI(_this,nT)];                                 /*   Map initial states              */
    TD_TER(_this,nT) = lpOrd[TD_TER(_this,nT)];                                 /*   Map terminal states             */
  }                                                                             /* <<                                */
  dlp_free(lpOrd);                                                              /* Free ordering index array         */
  CFst_STI_Done(lpTI);                                                          /* Destroy iterator                  */
  CFst_STI_Done(CFst_STI_Init(_this,0,FSTI_SORTINI));                           /* Sort transition list              */
}

/**
 * Removes epsilon/epsilon transitions fromt the resulting chain. The chain must
 * be topologically ordered!
 */
void GEN_PRIVATE CFst_Sdp_Epsremove(CFst* _this)
{
  INT32      nT     = 0;
  INT32      nIcTis = -1;
  INT32      nIcTos = -1;
  INT32      nIcW   = -1;
  INT16     nWsrt  = FST_WSR_NONE;
  FST_STYPE nTis   = 0;
  FST_STYPE nTos   = 0;
  FST_WTYPE nW     = 0.;
  FST_WTYPE nWe    = 0.;

  if (!_this->m_bEpsremove) return;                                             /* Not selected -> forget it!        */
  nIcTis = CData_FindComp(AS(CData,_this->td),NC_TD_TIS);                       /* Get input symbol component        */
  nIcTos = CData_FindComp(AS(CData,_this->td),NC_TD_TOS);                       /* Get output symbol component       */
  nWsrt  = CFst_Wsr_GetType(_this,&nIcW);                                       /* Get weight type and component     */
  DLPASSERT(nIcTis>=0 && nIcW>=0);                                              /* Need input symbols and weights    */

  for (nT=0; nT<UD_XT(_this,0); nT++)                                           /* Loop over transitions             */
  {                                                                             /* >>                                */
    DLPASSERT(nT==UD_XT(_this,0)-1 || TD_TER(_this,nT)==TD_INI(_this,nT+1));    /*   Not topologically ordered!      */

    nTis = (INT32)CData_Dfetch(AS(CData,_this->td),nT,nIcTis);                   /*   Get transition input symbol     */
    nTos = nIcTos>=0 ? (INT32)CData_Dfetch(AS(CData,_this->td),nT,nIcTos) : -1;  /*   Get transition output symbol    */
    nW   = CData_Dfetch(AS(CData,_this->td),nT,nIcW);                           /*   Get transition weight           */
    if (nTis>=0 || nTos>=0)                                                     /*   Not an eps./eps. transiton      */
    {                                                                           /*   >>                              */
      nW  = CFst_Wsr_Op(_this,nW,nWe,OP_MULT);                                  /*     Add eps. weight to curr.trans.*/
      nWe = CFst_Wsr_NeMult(nWsrt);                                             /*     Clear epsilon weight          */
      CData_Dstore(AS(CData,_this->td),nW,nT,nIcW);                             /*     Store transition weight       */
      continue;                                                                 /*     Next transition please        */
    }                                                                           /*   << (eps./eps. transition)       */
    nWe = CFst_Wsr_Op(_this,nW,nWe,OP_MULT);                                    /*   Aggregate epsilon weight        */
    if (nT==UD_XT(_this,0)-1)                                                   /*   Final transition                */
    {                                                                           /*   >>                              */
      SD_FLG(_this,TD_INI(_this,nT)+0)|=SD_FLG_FINAL;                           /*     Make initial state final      */
      SD_FLG(_this,TD_TER(_this,nT)+0)&=~SD_FLG_FINAL;                          /*     Make term. state unconnected  */
    }                                                                           /*   <<                              */
    else                                                                        /*   Not the final transition        */
      TD_INI(_this,nT+1)=TD_INI(_this,nT);                                      /*     Detour next transition        */
  }                                                                             /* <<                                */
  CFst_TrimStates(_this,0);                                                     /* Trim                              */
  if (nWe!=CFst_Wsr_NeMult(nWsrt) && UD_XT(_this,0)>0)                          /* Epsilon weight left >>            */
  {                                                                             /* >> (put to final transition)      */
    nT = UD_XT(_this,0)-1;                                                      /*   Get final transition index      */
    nW = CData_Dfetch(AS(CData,_this->td),nT,nIcW);                             /*   Get transition weight           */
    nW = CFst_Wsr_Op(_this,nW,nWe,OP_MULT);                                     /*   Add eps. weight to last trans.  */
    CData_Dstore(AS(CData,_this->td),nW,nT,nIcW);                               /*   Store transition weight         */
  }                                                                             /* <<                                */
}

/**
 * Synchroneous dynamic programming of one unit.
 *
 * @param _this     Pointer to destination automaton instance
 * @param itSrc     Pointer to source automaton instance
 * @param nUnit     The unit to process
 * @param idWeights The synchroneous transition weights. The instance must have
 *                  as many components as there are transitions in unit
 *                  <code>nUnit</code> of <code>itSrc</code>. The number of
 *                  records determines the number of search layers.
 */
INT16 GEN_PUBLIC CFst_SdpUnit
(
  CFst*  _this,
  CFst*  itSrc,
  INT32   nUnit,
  CData* idWeights
)
{
  INT16         nWtype = T_IGNORE;                                              /* Default weight variable type code */
  INT32          t      = 0;                                                     /* Current time                      */
  INT32          i      = 0;                                                     /* Auxilary loop counter             */
  INT32          nTmax  = 0;                                                     /* Maximal time                      */
  INT32          nXL    = 0;                                                     /* No. actually expanded layers      */
  INT32          nXW    = 0;                                                     /* No. of synch. weights (at time t) */
  FST_TID_TYPE* lpTI   = NULL;                                                  /* Source FST iterator               */
  FST_LB_TYPE*  lpLBrd = NULL;                                                  /* Read layer buffer                 */
  FST_LB_TYPE*  lpLBwr = NULL;                                                  /* Write layer buffer                */
  FST_BT_TYPE*  lpBT   = NULL;                                                  /* Backtracking tree                 */
  BOOL          bSWd   = FALSE;                                                 /* Aync. weights directly from input */
  FST_WTYPE*    lpSWa  = NULL;                                                  /* Synch. weights array (all times)  */
  FST_WTYPE*    lpSW   = NULL;                                                  /* Synch. weights array (at time t)  */
  FST_STYPE     nTis   = -1;                                                    /* Current input symbol              */
  INT32          nC     = 0;                                                     /* Component counter                 */
  FST_ITYPE     nS     = 0;                                                     /* Current state                     */
  FST_ITYPE     nES    = -1;                                                    /* End state (backtracking)          */
  FST_WTYPE     nED    = 0.;                                                    /* End distance (backtracking)       */
  INT32          nEL    = -1;                                                    /* End layer                         */
  INT32          nET    = -1;                                                    /* End time                          */
  INT16         bFlags = 0;                                                     /* Flags for expanding layers        */
  FLOAT64       *lpSum  = NULL;                                                  /* Pointer to matrix for prob sums   */

  if(!_this->m_bFwd) DLPASSERT(_this!=itSrc);                                   /* Source and dest. must be different*/

  /* Initialization */                                                          /* --------------------------------- */
  _this->m_nWsr = CFst_Wsr_GetType(itSrc,&_this->m_nIcW);                       /* Get weight semiring type and comp.*/
  nTmax         = idWeights ? CData_GetNRecs(idWeights) : _this->m_nMaxLen;     /* Determine max. path length        */
  nED           = CFst_Wsr_NeAdd(_this->m_nWsr);                                /* Initialize end weight             */
  lpTI          = CFst_STI_Init(itSrc,nUnit,FSTI_SORTINI);                      /* Create source iterator            */
  lpBT          = CFst_Sdp_BtInit(lpTI,_this->m_nGrany);                        /* Create backtracking tree          */
  lpLBrd        = (FST_LB_TYPE*)dlp_calloc(lpTI->nXS,sizeof(FST_LB_TYPE));      /* Allocate read layer buffer        */
  lpLBwr        = (FST_LB_TYPE*)dlp_calloc(lpTI->nXS,sizeof(FST_LB_TYPE));      /* Allocate write layer buffer       */
  IFCHECKEX(1) printf("\n Max. sync. time: %ld",(long)nTmax);                   /* Protocol                          */
  IFCHECKEX(1) printf("\n States         : %ld",(long)lpTI->nXS);               /* Protocol                          */
  if(_this->m_bFwd)                                                             /* If we should do forward algo.     */
  {                                                                             /* >>                                */
    lpSum = (FLOAT64*)dlp_calloc(lpTI->nXS*(nTmax+1),sizeof(FLOAT64));          /*   Initialize sum matrix           */
    bFlags |= FLG_FWD;                                                          /*   Set flag                        */
  }                                                                             /* <<                                */

  CFst_Sdp_ClearLB(_this,lpLBrd,lpTI->nXS);                                     /* Clear read layer buffer           */
  CFst_Sdp_ClearLB(_this,lpLBwr,lpTI->nXS);                                     /* Clear write layer buffer          */

  /* Count input symbols */                                                     /* --------------------------------- */
  _this->m_nSymbols = -1;                                                       /* No input symbols found so far     */
  for (i=lpTI->nFT; i<lpTI->nFT+lpTI->nXT; i++)                                 /* For all transitions of nUnit      */
  {                                                                             /* >>                                */
    nTis = *CFst_STI_TTis(lpTI,CFst_STI_GetTransPtr(lpTI,i));                   /*   Get input symbol                */
    if (nTis>_this->m_nSymbols) _this->m_nSymbols=nTis;                         /*   Remember greatest symbol index  */
  }                                                                             /* <<                                */
  _this->m_nSymbols++;                                                          /* Count = last index +1             */
  IFCHECKEX(1) printf("\n Input symbols  : %ld",(long)_this->m_nSymbols);       /* Protocol                          */

  /* Analyze (and convert) synchroneous weights array */                        /* --------------------------------- */
  nWtype = DLP_TYPE(FST_WTYPE);                                                 /* Get default weight variable type  */
  for (nC=0,bSWd=TRUE; nC<CData_GetNComps(idWeights); nC++)                     /* Loop over idWeights' components   */
    if (CData_GetCompType(idWeights,nC)!=nWtype) bSWd=FALSE;                    /*   and see if they're all FST_WTYPE*/
  IFCHECKEX(1) printf("\n Convert weights: %s",bSWd?"NO":"YES");                /* Protocol                          */
  if (bSWd)                                                                     /* Use sync. idWeights directly      */
  {                                                                             /* >>                                */
    lpSWa = (FST_WTYPE*)CData_XAddr(idWeights,0,0);                             /*   Get 'em pointer                 */
    nXW   = CData_GetNComps(idWeights);                                         /*   Get no. of weights per record   */
  }                                                                             /* <<                                */
  else                                                                          /* Need to convert sync. weights     */
  {                                                                             /* >>                                */
    INT32 nCa;                                                                  /*   Numeric component counter       */
    INT32 nR;                                                                   /*   Record counter                  */
    nXW   = _this->m_nSymbols;                                                  /*   One weight for each symbol, pls.*/
    lpSWa = (FST_WTYPE*)dlp_calloc(nXW*nTmax,sizeof(FST_WTYPE));                /*   Allocate converted weights array*/
    for (nR=0; nR<nXW*nTmax; nR++) lpSWa[nR]=CFst_Wsr_NeMult(_this->m_nWsr);    /*   Clear                           */
    for (nC=0,nCa=0; nC<CData_GetNComps(idWeights) && nCa<nXW; nC++)            /*   Loop over idWeigths' components */
      if (dlp_is_numeric_type_code(CData_GetCompType(idWeights,nC)))            /*     The numeric ones ...          */
      {                                                                         /*     >>                            */
        for (nR=0; nR<MIN(CData_GetNRecs(idWeights),nTmax); nR++)               /*       Loop over idWeigths' rcorz  */
          lpSWa[nR*nXW + nCa] = (FST_WTYPE)CData_Dfetch(idWeights,nR,nC);       /*         Convert weight            */
        nCa++;                                                                  /*       Count numeric components    */
      }                                                                         /*     <<                            */
  }                                                                             /* <<                                */

  /* Plant beam seed and expand over epsilon transitions */                     /* - - - - - - - - - - - - - - - - - */
  IFCHECKEX(1) printf("\n Initializing layer traversal");                       /* Protocol                          */
  IFCHECKEX(1) printf("\n   Time -1, Layer %ld:",(long)nXL);                    /* Protocol                          */
  lpLBrd[0].lpT = (BYTE*)(-1);                                                  /* Search beam seed                  */
  lpLBrd[0].nD  = CFst_Wsr_NeMult(_this->m_nWsr);                               /* Search beam seed                  */
  for (i=0; i<nTmax; i++)                                                       /* Expand at most nTmax eps. layers  */
  {                                                                             /* >>                                */
    nXL++;                                                                      /*   Count layers                    */
    if (CFst_Sdp_ExpandLayer(_this,lpTI,lpSW,t,nTmax,&lpLBrd,&lpLBwr,lpBT,      /*   If no new beams expanding layer */
      FLG_EPS|bFlags)<=0)                                                       /*   |                               */
        break;                                                                  /*     Stop expanding                */
  }                                                                             /* <<                                */
  if(_this->m_bFwd) for(i=0;i<lpTI->nXS;i++) lpSum[i]=lpLBrd[i].nD;             /* Copy sums                         */

  /* Layer traversal of itSrc */                                                /* --------------------------------- */
  IFCHECKEX(1) printf("\n Layer traversal");                                    /* Protocol                          */
  for (t=0; t<nTmax; t++)                                                       /* For all times t ...               */
  {                                                                             /* >>                                */
    IFCHECKEX(1) printf("\n   Time %ld, Layer %ld:",(long)t,(long)nXL);         /*   Protocol                        */
    CFst_Sdp_GetSWeights(_this,lpSWa,t,nXW,&lpSW);                              /*   Load weights for time t         */

    /* Expand states over all transitions */                                    /*   - - - - - - - - - - - - - - - - */
    CFst_Sdp_ExpandLayer(_this,lpTI,lpSW,t,nTmax,&lpLBrd,&lpLBwr,lpBT,bFlags);  /*   Do one search step              */
    nXL++;                                                                      /*   Count layers                    */

    /* Expand states over epsilon transitions */                                /*   - - - - - - - - - - - - - - - - */
    for (i=0; i<nTmax; i++)                                                     /*   Expand at most nTmax eps. layers*/
    {                                                                           /*   >>                              */
      nXL++;                                                                    /*     Count layers                  */
      if (CFst_Sdp_ExpandLayer(_this,lpTI,lpSW,t,nTmax,&lpLBrd,&lpLBwr,lpBT,    /*     If no new beams expandg.layer */
        FLG_EPS|bFlags)<=0)                                                     /*     |                             */
          break;                                                                /*       Stop expanding              */
    }                                                                           /*   <<                              */
    if(_this->m_bFwd) for(i=0;i<lpTI->nXS;i++)                                  /* Copy sums                         */
      lpSum[ (t+1)*lpTI->nXS + i ] = lpLBrd[i].nD;                              /* |                                 */

    /* Check for path ends */                                                   /* - - - - - - - - - - - - - - - - - */
    if (!idWeights || t==nTmax-1)                                               /* Async. search or last layer?      */
      for (nS=0; nS<lpTI->nXS; nS++)                                            /*   For all states in read LB       */
        if (lpLBrd[nS].lpT && (SD_FLG(lpTI->iFst,nS+lpTI->nFS)&SD_FLG_FINAL))   /*     State active and final?       */
        {                                                                       /*     >>                            */
          IFCHECKEX(1)                                                          /*       Protocol                    */
            printf("\n   PATH END at state %ld [%5.5g]",                        /*       |                           */
            (long)nS,(double)lpLBrd[nS].nD);                                    /*       |                           */
          if (CFst_Wsr_Op(_this,lpLBrd[nS].nD,nED,OP_GREATER))                  /*       This end state better?      */
          {                                                                     /*       >>                          */
            nES = nS;                                                           /*         Remember state            */
            nED = lpLBrd[nS].nD;                                                /*         Remember distance         */
            IFCHECKEX(1) printf(" *");                                          /*         Protocol                  */
          }                                                                     /*       <<                          */
        }                                                                       /*     <<                            */
  }                                                                             /* <<                                */
  IFCHECKEX(1) printf("\n End of layer traversal");                             /* Protocol                          */

  /* Clean up */                                                                /* --------------------------------- */
  dlp_free(lpLBrd);                                                             /* Free read layer buffer            */
  dlp_free(lpLBwr);                                                             /* Free write layer buffer           */
  if (!bSWd) dlp_free(lpSWa);                                                   /* Free converted sync.weights array */

  if(_this->m_bFwd)                                                             /* If we should do forward algo.     */
  {                                                                             /* >>                                */
    CData_Array(idWeights,DLP_TYPE(FST_WTYPE),lpTI->nXS,nTmax+1);               /*   Set size of weight array        */
    CData_DblockStore(idWeights,lpSum,0,lpTI->nXS,nTmax+1,-1);                  /*   Copy sums to weight array       */
    dlp_free(lpSum);                                                            /*   Destroy sums                    */
    /* Clean up */                                                              /*   ------------------------------- */
    CFst_Sdp_BtDone(lpBT);                                                      /*   Free backtracking tree          */
    CFst_STI_Done(lpTI);                                                        /*   Free automaton iterator         */
    return O_K;                                                                 /*   All done...                     */
  }                                                                             /* <<                                */

  /* Prepare destination instance */                                            /* --------------------------------- */
  CData_Scopy(AS(CData,_this->ud),AS(CData,itSrc->ud));                         /* Copy structure of unit table      */
  CData_Scopy(AS(CData,_this->sd),AS(CData,itSrc->sd));                         /* Copy structure of state table     */
  CData_Scopy(AS(CData,_this->td),AS(CData,itSrc->td));                         /* Copy structure of transition table*/
  CData_AllocateUninitialized(AS(CData,_this->ud),1);                           /* Allocate one unit                 */
  dlp_memmove(                                                                  /* Copy description of unit nUnit    */
    CData_XAddr(AS(CData,_this->ud),0,0),                                       /* |                                 */
    CData_XAddr(AS(CData,itSrc->ud),nUnit,0),                                   /* |                                 */
    CData_GetRecLen(AS(CData,itSrc->ud)));                                      /* |                                 */
  CData_AllocateUninitialized(AS(CData,_this->sd),1);                           /* Allocate one state                */
  dlp_memmove(                                                                  /* Copy description of final state   */
    CData_XAddr(AS(CData,_this->sd),0,0),                                       /* |                                 */
    CData_XAddr(AS(CData,itSrc->sd),UD_FS(itSrc,nUnit)+nES,0),                  /* |                                 */
    CData_GetRecLen(AS(CData,itSrc->sd)));                                      /* |                                 */
  UD_FS(_this,0)=0;                                                             /* Set state offset                  */
  UD_XS(_this,0)=1;                                                             /* Set state count                   */
  UD_FT(_this,0)=0;                                                             /* Set transition offset             */
  UD_XT(_this,0)=0;                                                             /* Set transition count              */

  /* Backtracking */                                                            /* --------------------------------- */
  nEL=nXL-1; nET=t-1;                                                           /* Layer and time of best path's end */
  CFst_Sdp_Backtracking(_this,lpBT,nET,nEL,nES,nED,idWeights);                  /* Do backtracking                   */
  CData_SetDescr(AS(CData,_this->ud),DESCR4,lpBT->nLen);

  /* Clean up */                                                                /* --------------------------------- */
  CFst_Sdp_BtDone(lpBT);                                                        /* Free backtracking tree            */
  CFst_STI_Done(lpTI);                                                          /* Free automaton iterator           */

  /* Finish destination instance */                                             /* --------------------------------- */
  for (nS=0; nS<UD_XS(_this,0)-1; nS++) SD_FLG(_this,nS)&=~SD_FLG_FINAL;        /* Clear all final state flags       */
  SD_FLG(_this,UD_XS(_this,0)-1)|=SD_FLG_FINAL;                                 /* Make last state final             */
  CFst_Reverse(_this,0);                                                        /* Reverse result                    */
  CFst_Sdp_Order(_this);                                                        /* Order states topologically        */
  CFst_Sdp_Epsremove(_this);                                                    /* Remove epsilon transitions        */
  _this->m_nGw = (FLOAT64)nED;                                                   /* Store path weight                 */

  return O_K;                                                                   /* All done...                       */
}

/*
 * Manual page at fst_man.def
 */
INT16 GEN_PUBLIC CFst_Sdp
(
  CFst*  _this,
  CFst*  itSrc,
  INT32   nUnit,
  CData* idWeights
)
{
  INT32   nIcW       = -1;
  INT16  nCheck     = 0;
  BOOL   bEpsremove = FALSE;
  BOOL   bPrune     = FALSE;
  BOOL   bFwd       = FALSE;
  FLOAT64 nPrnConst  = 0.;
  INT32   nGrany     = 0;

  CHECK_THIS_RV(NOT_EXEC);
/*  CFst_Check(_this); The check is not requiered because of CFst_Reset on _this. */
  CFst_Check(itSrc);

  /* Protocol */                                                                /* --------------------------------- */
  IFCHECKEX(1)                                                                  /* On verbose level 1                */
  {                                                                             /* >>                                */
    printf("\n"); dlp_fprint_x_line(stdout,'-',dlp_maxprintcols());             /*   Print separator                 */
    printf("\n CFst_Sdp(");                                                     /*   Print function signature        */
    printf("%s,",BASEINST(_this)->m_lpInstanceName);                            /*   ...                             */
    printf("%s,",BASEINST(itSrc)->m_lpInstanceName);                            /*   ...                             */
    printf("%ld,%s)\n",(long)nUnit,BASEINST(idWeights)->m_lpInstanceName);      /*   ...                             */
    printf("\n Weigt-SR   : %s",CFst_Wsr_GetName(CFst_Wsr_GetType(itSrc,NULL)));/*   Print weight semiring type      */
    printf("\n"); dlp_fprint_x_line(stdout,'-',dlp_maxprintcols());             /*   Print separator                 */
  }                                                                             /* <<                                */

  /* Validate */                                                                /* --------------------------------- */
  if (nUnit<0 || nUnit>=UD_XXU(itSrc))                                          /* Does source unit exist?           */
    return IERROR(_this,FST_BADID,"unit",nUnit,0);                              /*   It must!                        */
  if (UD_XT(itSrc,nUnit)==0) IERROR(_this,FST_UNITEMPTY,nUnit,0,0);             /* Source unit must be non-empty     */
  CFst_Wsr_GetType(itSrc,&nIcW);                                                /* Get weight semiring type          */
  if (nIcW<0)                                                                   /* Have weights?                     */
    return IERROR(_this,FST_MISS,"weight component","","transition table");     /*   Must have!                      */
  if (CData_FindComp(AS(CData,itSrc->td),NC_TD_TIS)<0)                          /* Have input symbols?               */
    return IERROR(_this,FST_MISS,"input symbol component","","transition table");/*   Must have!                     */

  /* Protocol */
  IFCHECKEX(1)                                                                  /* On verbose level 1                */
  {                                                                             /* >>                                */
    printf("\n Pruning        : ");                                             /*   Protocol                        */
    if (_this->m_bPrune)                                                        /*   ...                             */
    {                                                                           /*   ...                             */
      if (_this->m_nPrnConst<=0 || _this->m_nPrnConst>=1)                       /*   ...                             */
        printf("AUTO-OFF");                                                     /*   ...                             */
      else                                                                      /*   ...                             */
      {                                                                         /*   ...                             */
        printf("ON");                                                           /*   ...                             */
        printf("\n - prn_const    : %lg",_this->m_nPrnConst);                   /*   ...                             */
        printf("\n - KEEPA        : %ld",(long)FST_SDP_KEEPA);                  /*   ...                             */
        printf("\n - KEEPR        : %g",(double)FST_SDP_KEEPR);                 /*   ...                             */
        printf("\n - DMZ          : %ld",(long)FST_SDP_DMZ);                    /*   ...                             */
      }                                                                         /*   ...                             */
    }                                                                           /*   ...                             */
    else printf("OFF");                                                         /*   ...                             */
  }                                                                             /* <<                                */
  if (_this->m_nPrnConst<=0 || _this->m_nPrnConst>=1) _this->m_bPrune=FALSE;    /* Pruning auto-off                  */

  if(!_this->m_bFwd){
    /* Reset destination instance */                                            /* --------------------------------- */
    nCheck     = BASEINST(_this)->m_nCheck;
    bEpsremove = _this->m_bEpsremove;
    bPrune     = _this->m_bPrune;
    nPrnConst  = _this->m_nPrnConst;
    nGrany     = _this->m_nGrany;
    bFwd       = _this->m_bFwd;
    CREATEVIRTUAL(CFst,itSrc,_this);
    CFst_Reset(BASEINST(_this),TRUE);
    BASEINST(_this)->m_nCheck = nCheck;
    _this->m_bEpsremove       = bEpsremove;
    _this->m_bPrune           = bPrune;
    _this->m_bFwd             = bFwd;
    _this->m_nPrnConst        = nPrnConst;
    _this->m_nGrany           = nGrany;

    /* Copy input and output symbol table */                                    /* --------------------------------- */
    CData_Copy(_this->is,itSrc->is);                                            /*   Copy input symbol table         */
    CData_Copy(_this->os,itSrc->os);                                            /*   Copy output symbol table        */
  }

  CFst_SdpUnit(_this,itSrc,nUnit,idWeights);

  if(!_this->m_bFwd){
    DESTROYVIRTUAL(itSrc,_this);
    CFst_Check(_this);                                                          /* TODO: Remove after debugging      */
  }

  /* Protocol */                                                                /* --------------------------------- */
  IFCHECKEX(1)                                                                  /* On verbose level 1                */
  {                                                                             /* >>                                */
    printf("\n\n CFst_Sdp done.");                                              /*   Print function identifier       */
    printf("\n"); dlp_fprint_x_line(stdout,'-',dlp_maxprintcols());             /*   Print separator                 */
    printf("\n");                                                               /*   ...                             */
  }                                                                             /* <<                                */
  return O_K;
}

/* EOF */
