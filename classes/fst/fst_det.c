/* dLabPro class CFst (fst)
 * - Transducer determinization and minimization
 *
 * AUTHOR : Maximiliano Cuevas, Matthias Wolff
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
 * Adds one record to the determinization auxilary table. Each record describes
 * one transition originating in a particular state of the destination
 * transducer. The method adds the output symbol and the weight of the
 * transition to the residuals associated with the state. This method must not
 * be called except as part of {@link Det_LoadAuxTable CFst_Det_LoadAuxTable}.
 *
 * @param _this   Pointer to automaton instance
 * @param iAuxTab Pointer to a determinization auxilary table instance
 * @param nResStr Index of residual string to concatenate transition output
 *                symbols to
 * @param nResW   Residual weight to add transition weights to
 * @param nTer    Terminal state (in source transducer) of transition to add
 * @param nTis    Input symbol of transition to add
 * @param nTos    Output symbol of transition to add
 * @param nW      Weight of transition to add
 * @param bFinal  <code>TRUE</code>for an epsilon transition to be generated
 *                for a non-zero resiadual of a final state, <code>FALSE</code>
 *                for an ordinary transition
 * @see Det_LoadAuxTable CFst_Det_LoadAuxTable
 */
void CGEN_PRIVATE CFst_Det_LoadAuxTable_AddRec
(
  CFst*     _this,
  CData*    idAuxTab,
  FST_ITYPE nResStr,
  FST_WTYPE nResW,
  FST_ITYPE nTer,
  FST_STYPE nTis,
  FST_STYPE nTos,
  FST_WTYPE nW,
  BOOL      bFinal
)
{
  FST_STYPE lpBuf[2] = {-1,-1};                                                /* String buffer                     */
  FST_ITYPE nStr     = -1;                                                     /* String index                      */
  INT32      nR       = 0;                                                      /* Index of record to write          */

  IFCHECK
    printf("\n       - %c (src. ter. state=%ld, ",bFinal?'F':'T',(long)nTer);

  nR = CData_AddRecs(idAuxTab,1,_this->m_nGrany);                              /* Add a new row                     */
  *(FST_ITYPE*)(CData_XAddr(idAuxTab,nR,0)) = nTer;                            /* Store terminal state              */
  *(FST_STYPE*)(CData_XAddr(idAuxTab,nR,1)) = nTis;                            /* Store input symbol                */
  *(BYTE*     )(CData_XAddr(idAuxTab,nR,4)) = bFinal;                          /* Store transition type             */

  lpBuf[0] = nTos; lpBuf[1]=-1;                                                /* Store accumulated output string   */
  nStr     = CFst_Ssr_Store(_this->m_lpDetSt,lpBuf);
  *(FST_ITYPE*)(CData_XAddr(idAuxTab,nR,2)) =
    CFst_Ssr_Mult(_this->m_lpDetSt,nResStr,nStr);

  IFCHECK
  {
    printf("accstr=");
    CFst_Ssr_Print(_this->m_lpDetSt,*(FST_ITYPE*)(CData_XAddr(idAuxTab,nR,2)));
    printf(", ");
  }

  *(FST_WTYPE*)(CData_XAddr(idAuxTab,nR,3)) =                                  /* Store accumulated weight          */
    CFst_Wsr_Op(_this,nResW,nW,OP_MULT);

  IFCHECK printf("accweight=%g",(double)*(FST_WTYPE*)(CData_XAddr(idAuxTab,nR,3)));

  IFCHECK printf(")");
}

/**
 * Traverses an automaton until the first non-epsilon transition or a final
 * state whichever occours first. This method is called recursively from
 * {@link Det_LoadAuxTable}.
 */
void CGEN_PRIVATE CFst_Det_LoadAuxTable_Walk
(
  CFst*         _this,
  FST_TID_TYPE* lpTI,
  FST_ITYPE     nIniSrc,
  FST_ITYPE     nResStr,
  FST_WTYPE     nResW,
  CData*        idAuxTab
)
{
  BYTE*     lpT      = NULL;                                                   /* Pointer to current transition     */
  FST_ITYPE nTerSrc  = -1;                                                     /* Terminal state of current trans.  */
  FST_STYPE nTisSrc  = -1;                                                     /* Input symbol of current trans.    */
  FST_STYPE nTosSrc  = -1;                                                     /* Output symbol of current trans.   */
  FST_WTYPE nWSrc    = 0.;                                                     /* Weight symbol of current trans.   */
  FST_STYPE lpBuf[2] = {-1,-1};                                                /* String buffer                     */
  FST_ITYPE nStr     = -1;                                                     /* String index                      */
  FST_ITYPE nAccStr  = -1;                                                     /* Accumulated residual string       */
  FST_WTYPE nAccW    = 0.;                                                     /* Accumulated residual weight       */

  while ((lpT=CFst_STI_TfromS(lpTI,nIniSrc,lpT))!=NULL)                        /* Enum. trans. starting in nIniSrc  */
  {
    nTerSrc = *CFst_STI_TTer(lpTI,lpT);                                        /* Get terminal state                */
    nTisSrc = *CFst_STI_TTis(lpTI,lpT);                                        /* Get input symbol                  */
    nTosSrc = lpTI->nOfTTos>0 ? *CFst_STI_TTos(lpTI,lpT) : -1;                 /* Get output symbol                 */
    nWSrc   = lpTI->nOfTW  >0 ? *CFst_STI_TW  (lpTI,lpT)                       /* Get weight                        */
                              : CFst_Wsr_NeMult(_this->m_nWsr);

    IFCHECKEX(2)
      printf("\n       Walk: %ld -(%ld:%ld/%g) -> %ld",
        (long)nIniSrc,(long)nTisSrc,(long)nTosSrc,(double)nWSrc,(long)nTerSrc);

    lpBuf[0] = nTosSrc; lpBuf[1]=-1;                                           /* Store output symbol as string     */
    nStr     = CFst_Ssr_Store(_this->m_lpDetSt,lpBuf);
    nAccStr  = CFst_Ssr_Mult(_this->m_lpDetSt,nResStr,nStr);                   /* Accumulate residual string        */
    nAccW    = CFst_Wsr_Op(_this,nResW,nWSrc,OP_MULT);                         /* Accumulate residual weight        */

    if                                                                         /* If                                */
    (
      nTisSrc>=0 ||                                                            /* - not an epsilon transition or    */
      (SD_FLG(lpTI->iFst,lpTI->nFS+nTerSrc)&0x01)==0x01                        /* - terminal state final            */
    )
    {
      CFst_Det_LoadAuxTable_AddRec                                             /* Add record to auxilary table      */
      (
        _this,idAuxTab,nAccStr,nAccW,nTerSrc,nTisSrc,
        -1,CFst_Wsr_NeMult(_this->m_nWsr),FALSE
      );
    }
    else
    {
      CFst_Det_LoadAuxTable_Walk(_this,lpTI,nTerSrc,nAccStr,nAccW,idAuxTab);   /* Go on walking...                  */
    }
  }
}

/**
 * Creates or updates a table holding information on transitions originating in
 * a particular state. This table is needed during transducer determinization.
 *
 * @param _this   Pointer to automaton instance
 * @param nUnit   Unit in this instance to create state table for
 *                (value not checked!)
 * @param nIniSrc Initial state of transitions to add to table
 *                (value not checked!)
 * @param nResStr Index of residual string to concatenate transition output
 *                symbols to
 * @param nResW   Residual weight to add transition weights to
 * @param iAuxTab A pointer to a table instance to be initialized
 * @param lpTI    Transducer iterator data structure
 * @return <code>TRUE</code> if the specified residual data denote a final
 *         state (i.e. if the respective state of the source transducer
 *         is final <i>and</i> there are no residual strings and weights),
 *         <code>FALSE</code> otherwise.
 */
BOOL CGEN_PRIVATE CFst_Det_LoadAuxTable
(
  CFst*         _this,
  CFst*         itSrc,
  INT32          nUnit,
  FST_ITYPE     nIniSrc,
  FST_ITYPE     nResStr,
  FST_WTYPE     nResW,
  CData*        idAuxTab,
  FST_TID_TYPE* lpTI
)
{
  BOOL          bFinal     = TRUE;
  FST_STYPE     nPseudoTis = -1;                                                /* Pseudo input symbol encoding output string */

  /* Validate */
  CHECK_THIS_RV(FALSE);
  DLPASSERT(_this->m_lpDetSt);
  DLPASSERT(_this->m_idDetRt);
  DLPASSERT(idAuxTab);

  /* Initialize table */
  if (CData_IsEmpty(idAuxTab))
  {
    CData_Reset(BASEINST(idAuxTab),TRUE);
    CData_AddComp(idAuxTab,"nTerSrc",DLP_TYPE(FST_ITYPE));                     /* Terminal state in src. transducer */
    CData_AddComp(idAuxTab,"nTisSrc",DLP_TYPE(FST_STYPE));                     /* Input symbol in src. transducer   */
    CData_AddComp(idAuxTab,"nAccStr",DLP_TYPE(FST_ITYPE));                     /* Accumulated output string         */
    CData_AddComp(idAuxTab,"nAccW"  ,DLP_TYPE(FST_WTYPE));                     /* Accumulated weight                */
    CData_AddComp(idAuxTab,"bFinal" ,T_BYTE             );                     /* Residual type                     */
  }

  /* NO RETURNS BEYOND THIS POINT! */

  /* Check if residual is final */
  if ((SD_FLG(itSrc,UD_FS(itSrc,nUnit)+nIniSrc)&0x01)==0x01)
  {
    if
    (
      CFst_Ssr_Len(_this->m_lpDetSt,nResStr)>0 ||
      !CFst_Wsr_Op(_this,nResW,CFst_Wsr_NeMult(_this->m_nWsr),OP_EQUAL)
    )
    {
      /* Store final state residual; this will generate a (chain of) epsilon
         transition(s) for each nResStr carrying nResStr and nResW and
         terminating at a final state */
      nPseudoTis = -nResStr-2;
      CFst_Det_LoadAuxTable_AddRec
      (
        _this,idAuxTab,nResStr,nResW,nIniSrc,nPseudoTis,-1,
        CFst_Wsr_NeMult(_this->m_nWsr),TRUE
      );
      bFinal=FALSE;
    }
  }
  else bFinal=FALSE;

  /* Store transitions leaving nIniSrc */
  /* MWX 2004-18-11: Naive implementation; may leave unnecessary epsilon transitions --> * /
  {
    BYTE* lpT = NULL;
    while ((lpT=CFst_STI_TfromS(lpTI,nIniSrc,lpT))!=NULL)
    {
      CFst_Det_LoadAuxTable_AddRec
      (
        _this,idAuxTab,nResStr,nResW,
        *CFst_STI_TTer(lpTI,lpT),
        *CFst_STI_TTis(lpTI,lpT),
        lpTI->nOfTTos>0 ? *CFst_STI_TTos(lpTI,lpT) : -1,
        lpTI->nOfTW  >0 ? *CFst_STI_TW  (lpTI,lpT)
                        : CFst_Wsr_NeMult(_this->m_nWsr),
        FALSE
      );
    }
  }*/
  /* <-- Skip over epsilon input symbols --> */
  CFst_Det_LoadAuxTable_Walk(_this,lpTI,nIniSrc,nResStr,nResW,idAuxTab);
  /* <-- */

  /* Clean up */
  return bFinal;
}

/**
 * Debugging: print determinization auxilary table.
 *
 * @param _this   Pointer to automaton instance
 * @param iAuxTab Pointer to a determinization auxilary table instance
 */
void CGEN_PRIVATE CFst_Det_PrintAuxTable(CFst* _this, CData* idAuxTab)
{
  INT32 i;

  printf("\n       Determinization auxilary table:");
  printf("\n       no.: Typ nTerSrc nTisSrc   nAccW nAccStr");
  for (i=0; i<CData_GetNRecs(idAuxTab); i++)
  {
    printf("\n       %3ld: ",i);
    printf("%c   ",*(BYTE*     )CData_XAddr(idAuxTab,i,4)?'F':'T');
    printf("%7ld ",(long)*(FST_ITYPE*)CData_XAddr(idAuxTab,i,0));
    printf("%7ld ",(long)*(FST_STYPE*)CData_XAddr(idAuxTab,i,1));
    printf("%7g  ",(double)*(FST_WTYPE*)CData_XAddr(idAuxTab,i,3));
    CFst_Ssr_Print(_this->m_lpDetSt,*(FST_ITYPE*)CData_XAddr(idAuxTab,i,2));
  }
}

/**
 * Adds one state to the destination transducer during determinization. When
 * called for the first time, the method will add two components to the
 * {@link sd state table}. These components hold the index of the first record
 * and the number of records in the residual table asociated with the state.
 * The method will also initialize the residual table.
 *
 * @param  _this  Pointer to automaton instance
 * @param  bFinal If <code>TRUE</code> the new state is a final state
 * @return The non-negative index of the new state in the state table if
 *         successfull, a (negative) error code otherwise
 */
FST_ITYPE CGEN_PRIVATE CFst_Det_AddState(CFst* _this, BOOL bFinal)
{
  FST_ITYPE nS = 0;

  /* Validate */
  CHECK_THIS_RV(-1);

  /* Initialize residual table and residual index in sd */
  if (UD_XS(_this,0)==0)
  {
    DLPASSERT(_this->m_nIcSdAux<0);               /* Residual table already initialized? */
    DLPASSERT(_this->m_idDetRt   );               /* Residual not created?               */

    CData_Reset(_this->m_idDetRt,TRUE);
    CData_AddComp(AS(CData,_this->m_idDetRt),"nIniSrc",DLP_TYPE(FST_ITYPE));
    CData_AddComp(AS(CData,_this->m_idDetRt),"nResStr",DLP_TYPE(FST_ITYPE));
    CData_AddComp(AS(CData,_this->m_idDetRt),"nResW"  ,DLP_TYPE(FST_WTYPE));

    _this->m_nIcSdAux = CData_GetNComps(AS(CData,_this->sd));
    CData_AddComp(AS(CData,_this->sd),"~RSF",DLP_TYPE(FST_ITYPE));
    CData_AddComp(AS(CData,_this->sd),"~RSX",DLP_TYPE(FST_ITYPE));
  }

  /* Add new state */
  nS = CFst_Addstates(_this,0,1,bFinal);

  /* Initialize residual index */
  *(FST_ITYPE*)(CData_XAddr(AS(CData,_this->sd),nS,_this->m_nIcSdAux  )) = (FST_ITYPE)CData_GetNRecs(AS(CData,_this->m_idDetRt));
  *(FST_ITYPE*)(CData_XAddr(AS(CData,_this->sd),nS,_this->m_nIcSdAux+1)) = 0;

  return nS;
}

/**
 * Adds a path (a chain of transitions and states) to the destination
 * transducer during determinization. The method will add as many states
 * and transitions as there are symbols in <code>nTosStr</code>, but at
 * least one to hold the input symbol and weight.
 *
 * @param _this   Pointer to automaton instance
 * @param nIni    Initial state of path
 * @param nTis    Input symbol of (first) transition
 * @param nTosStr Output symbol string
 * @param nW      Weight of (first) transition
 * @return The index of the last transition of the generated path
 */
FST_ITYPE CGEN_PRIVATE CFst_Det_AddPath
(
  CFst*      _this,
  FST_ITYPE  nIni,
  FST_STYPE  nTis,
  FST_ITYPE  nTosStr,
  FST_WTYPE  nW
)
{
  INT32      nLen = 0;
  FST_ITYPE i    = 0;
  FST_ITYPE nTer = 0;
  FST_ITYPE nT   = 0;

  nLen = CFst_Ssr_Len(_this->m_lpDetSt,nTosStr);
  if (nLen<=0) nLen=1;

  /* For each output symbol ... */
  for (i=0; i<nLen; nIni=nTer,i++)
  {
    /* Add a state and a transition */
    nTer = CFst_Det_AddState(_this,FALSE);
    nT   = CFst_Addtrans(_this,0,nIni,nTer);

    if (i==0)
    {
      /* Store input symbo and weight */
      IFCHECKEX(1) printf("\n       Add dst. path: %ld",(long)nIni);
      if (_this->m_nIcTis>=IC_TD_DATA) *(FST_ITYPE*)CData_XAddr(AS(CData,_this->td),nT,_this->m_nIcTis)=nTis;
      if (_this->m_nIcW  >=IC_TD_DATA) *(FST_WTYPE*)CData_XAddr(AS(CData,_this->td),nT,_this->m_nIcW  )=nW;
    }

    /* Store output symbol */
    if (_this->m_nIcTos>=IC_TD_DATA)
      *(FST_ITYPE*)CData_XAddr(AS(CData,_this->td),nT,_this->m_nIcTos) =
        CFst_Ssr_GetAt(_this->m_lpDetSt,nTosStr,i);

    /* Protocol message */
    IFCHECKEX(1)
      printf
      (
        " -(%ld:%ld/%g)-> %ld",
        (long)(i==0?nTis:-1),
        (long)(_this->m_nIcTos>=IC_TD_DATA?*(FST_ITYPE*)CData_XAddr(AS(CData,_this->td),nT,_this->m_nIcTos):-1),
        (double)(i==0?nW:CFst_Wsr_NeMult(CFst_Wsr_GetType(_this,NULL))),
        (long)nTer
      );
  }

  return nT;
}

/**
 * Adds one entry to the residual table during determinization.
 *
 * @param _this   Pointer to automaton instance containing residual table to
 *                update
 * @param nSdst   State index in destination (this) transducer to associate
 *                residual with
 * @param nSsrc   Residual information: state index in source transducer
 * @param nResStr Residual information: index of residual string
 * @param nResW   Residual information: residual weight
 */
void CGEN_PRIVATE CFst_Det_AddResidual
(
  CFst*     _this,
  FST_ITYPE nSdst,
  FST_ITYPE nSsrc,
  FST_ITYPE nResStr,
  FST_WTYPE nResW
)
{
  INT32 nR  = 0;
  /*INT32 nFR = 0;*/
  /*INT32 nXR = 0;*/

  /* Check for duplicates */
  /* HACK: This seems only to be necessary for certain final state residuals (input symbol <0) */
  /* This hack produces a wrong determinization result if there are two identical paths
   * (identical in symbols and weight) to the same final state.
   * example:
   *   fst f;
   *   "" f /lsr /fst -addunit;
   *   0 1 f -addstates;
   *   0 1 f /final -addstates;
   *   0 0 1 f -addtrans { 0 0 0.69 };
   *   0 0 1 f -addtrans { 0 0 0.69 };
   *   f 0 f -determinize;
   */
/*  nFR = *(FST_ITYPE*)(CData_XAddr(AS(CData,_this->sd),nSdst,_this->m_nIcSdAux  ));
  nXR = *(FST_ITYPE*)(CData_XAddr(AS(CData,_this->sd),nSdst,_this->m_nIcSdAux+1));
  for (nR=nFR; nR<nFR+nXR; nR++)
    if
    (
      *(FST_ITYPE*)(CData_XAddr(AS(CData,_this->m_idDetRt),nR,0)) == nSsrc   &&
      *(FST_ITYPE*)(CData_XAddr(AS(CData,_this->m_idDetRt),nR,1)) == nResStr &&
      CFst_Wsr_Op(_this,*(FST_WTYPE*)(CData_XAddr(AS(CData,_this->m_idDetRt),nR,2)),nResW,OP_EQUAL)
    )
    {
      IFCHECKEX(1) CFst_Det_PrtResidual(_this,nR);
      return;
    }*/

  /* Update residual table */
  nR = CData_AddRecs(AS(CData,_this->m_idDetRt),1,_this->m_nGrany);
  *(FST_ITYPE*)(CData_XAddr(AS(CData,_this->m_idDetRt),nR,0)) = nSsrc;
  *(FST_ITYPE*)(CData_XAddr(AS(CData,_this->m_idDetRt),nR,1)) = nResStr;
  *(FST_WTYPE*)(CData_XAddr(AS(CData,_this->m_idDetRt),nR,2)) = nResW;

  /* Update residual index */
  (*(FST_ITYPE*)(CData_XAddr(AS(CData,_this->sd),nSdst,_this->m_nIcSdAux+1)))++;

  IFCHECKEX(1) CFst_Det_PrtResidual(_this,nR);
}

/**
 * Debugging: prints one line of the residual table.
 *
 * @param _this Pointer to automaton instance containing residual table
 * @param nR    Index of residual to print (table row)
 */
void CGEN_PRIVATE CFst_Det_PrtResidual(CFst* _this, FST_ITYPE nR)
{
  FST_ITYPE nIniSrc = 0;
  FST_ITYPE nResStr = 0;
  FST_WTYPE nResW   = 0.;

  CHECK_THIS();
  DLPASSERT(_this->m_idDetRt                   );
  DLPASSERT(nR>=0                              );
  DLPASSERT(nR<CData_GetNRecs(AS(CData,_this->m_idDetRt)));

  nIniSrc = *(FST_ITYPE*)CData_XAddr(AS(CData,_this->m_idDetRt),nR,0);
  nResStr = *(FST_ITYPE*)CData_XAddr(AS(CData,_this->m_idDetRt),nR,1);
  nResW   = *(FST_WTYPE*)CData_XAddr(AS(CData,_this->m_idDetRt),nR,2);

  printf("\n       - R (src. ini. state=%ld, string=",(long)nIniSrc);
  CFst_Ssr_Print(_this->m_lpDetSt,nResStr);
  printf(", weight=%g)",(double)nResW);
}

/**
 * Compares the residual tables of two states.
 *
 * @param _this  Pointer to automaton instance containing residual table
 * @param nS1    First state index
 * @param nS2    Second state index
 * @param lpMap  Static map (will be reallocated)
 * @param nMapSi Current size of map (initial: 0)
 * @return <code>TRUE</code> if the residual tables are equal,
 * <code>FALSE</code> otherwise
 */
BOOL CGEN_PRIVATE CFst_Det_CmpResiduals(CFst* _this, FST_ITYPE nS1, FST_ITYPE nS2, FST_ITYPE** lpMap, INT32 *nMapSi)
{
  FST_ITYPE  nR1   = 0;
  FST_ITYPE  nR2   = 0;
  FST_ITYPE  nFR1  = 0;
  FST_ITYPE  nFR2  = 0;
  FST_ITYPE  nXR1  = 0;
  FST_ITYPE  nXR2  = 0;
  INT32      i     = 0;
  BYTE*      lpDR;
  INT32      nDRRecLen;
  INT32      nDRWOff;
  BYTE*      lpSD;
  INT32      nSDRecLen;
  INT32      nSDOff0;
  INT32      nSDOff1;


  IFCHECKEX(3) printf("\n       - States %ld and %ld",(long)nS1,(long)nS2);

  /* Get data pointer for m_idDetRT + sd */
  lpDR      = AS(CData,_this->m_idDetRt)->m_lpTable->m_theDataPointer;
  nDRRecLen = AS(CData,_this->m_idDetRt)->m_lpTable->m_reclen;
  nDRWOff   = AS(CData,_this->m_idDetRt)->m_lpTable->m_compDescrList[2].offset;
  lpSD      = AS(CData,_this->sd)->m_lpTable->m_theDataPointer;
  nSDRecLen = AS(CData,_this->sd)->m_lpTable->m_reclen;
  nSDOff0   = AS(CData,_this->sd)->m_lpTable->m_compDescrList[_this->m_nIcSdAux+0].offset;
  nSDOff1   = AS(CData,_this->sd)->m_lpTable->m_compDescrList[_this->m_nIcSdAux+1].offset;

  /* Compare number of residuals */
  nXR1 = *(FST_ITYPE*)(lpSD + nS1*nSDRecLen + nSDOff1);
  nXR2 = *(FST_ITYPE*)(lpSD + nS2*nSDRecLen + nSDOff1);

  if (nXR1!=nXR2)
  {
    IFCHECKEX(3) printf(" -> Not equal (different number of residuals)");
    return FALSE;
  }

  /* Compare residual tables */
  nFR1 = *(FST_ITYPE*)(lpSD + nS1*nSDRecLen + nSDOff0);
  nFR2 = *(FST_ITYPE*)(lpSD + nS2*nSDRecLen + nSDOff0);

  IFCHECKEX(4)
  {
    printf("\n       State %ld:",(long)nS1); for (nR1=nFR1; nR1<nFR1+nXR1; nR1++) CFst_Det_PrtResidual(_this,nR1);
    printf("\n       State %ld:",(long)nS2); for (nR2=nFR2; nR2<nFR2+nXR2; nR2++) CFst_Det_PrtResidual(_this,nR2);
  }

  /* MWX 2004-10-27 TODO: Veeeery naive implementation;
                          compare sorted hash lists instead!  --> */
  /* MWX 2004-12-02 Problem: how to compare floats with hash lists?? */
  if(nXR1>*nMapSi) *lpMap=(FST_ITYPE*)dlp_realloc(*lpMap,*nMapSi=nXR1,sizeof(FST_ITYPE));
  for (i=0; i<nXR1; i++) lpMap[0][i]=-1;

  for (nR1=nFR1; nR1<nFR1+nXR1; nR1++)
  {
    for (nR2=nFR2; nR2<nFR2+nXR2; nR2++)
    {
      if (((FST_ITYPE*)(lpDR+nR1*nDRRecLen))[0]!=((FST_ITYPE*)(lpDR+nR2*nDRRecLen))[0]) continue;
      if (((FST_ITYPE*)(lpDR+nR1*nDRRecLen))[1]!=((FST_ITYPE*)(lpDR+nR2*nDRRecLen))[1]) continue;
      if
      (
        !CFst_Wsr_Op
        (
          _this,
          *(FST_WTYPE*)(lpDR+nR1*nDRRecLen+nDRWOff),
          *(FST_WTYPE*)(lpDR+nR2*nDRRecLen+nDRWOff),
          OP_EQUAL
        )
      )
      {
        continue;
      }

      /* Lines equal -> write map */
      if (lpMap[0][nR1-nFR1]==-1)
      {
        lpMap[0][nR1-nFR1]=nR2;
        break;
      }
    }
  }

  /* Analyze map */
  for (i=0; i<nXR1; i++)
    if (lpMap[0][i]<0)
      break;

  /* <-- */

  IFCHECKEX(3) printf(" -> %s",i==nXR1?"Equal":"Not equal (different data)");
  return (i==nXR1);
}

int cf_comp1_itype_up(const void* a, const void* b)
{
  const FST_ITYPE *ai=(FST_ITYPE*)a;
  const FST_ITYPE *bi=(FST_ITYPE*)b;
  if(ai[1] > bi[1]) return  1;
  if(ai[1] < bi[1]) return -1;

  return 0;
}

/**
 * Determinizes one unit. There are no checks performed.
 *
 * @param _this Pointer to this (destination) automaton instance
 * @param itSrc Pointer to source automaton instance
 * @param nUnit Index of unit to determinize
 * @return O_K if successfull, a (negative) error code otherwise
 * @see Determinize CFst_Determinize
 */
INT16 CGEN_PROTECTED CFst_DeterminizeUnit(CFst* _this, CFst* itSrc, INT32 nUnit)
{
  CData*     idAux   = NULL;           /* Auxilary table for determinization                    */
  FST_ITYPE  nIniDst = 0;              /* Currently processed initial destination state         */
  FST_ITYPE  nTDst   = 0;              /* Currently added transition in destination transducer  */
  FST_ITYPE  nS      = 0;              /* Current state                                         */
  BOOL       bFinal  = FALSE;          /* Current destination state is final                    */
  INT32       nR      = 0;              /* Current record                                        */
  INT32       nR2     = 0;              /* Current record                                        */
  INT32       nFR     = 0;              /* First record in residual table for current state      */
  INT32       nXR     = 0;              /* Number of records in residual table for current state */
  FST_STYPE  nTis    = -1;             /* Transducer input symbol                               */
  FST_ITYPE  nSumStr = 0;              /* String sum                                            */
  FST_WTYPE  nSumW   = 0;              /* Weight sum                                            */
  BOOL       bFsr    = FALSE;          /* Current state in aux. table is a final state residual */
  INT16      nCheck  = 0;
  FST_TID_TYPE* lpTI = NULL;
  FST_ITYPE* lpMap   = NULL;
  INT32      nMapSi  = 0;

  IFCHECK
  {
    printf("\n"); dlp_fprint_x_line(stdout,'-',dlp_maxprintcols());
    printf("\n   Unit %ld",(long)nUnit);
  }

  DLPASSERT(_this!=itSrc);

  /* Initialize destination */
  nCheck=BASEINST(_this)->m_nCheck;
  CFst_Reset(BASEINST(_this),TRUE);
  BASEINST(_this)->m_nCheck=nCheck;
  CData_Scopy(AS(CData,_this->sd),AS(CData,itSrc->sd));
  CData_Scopy(AS(CData,_this->td),AS(CData,itSrc->td));
  CData_SelectRecs(AS(CData,_this->ud),AS(CData,itSrc->ud),nUnit,1);
  UD_FS(_this,0)=0;
  UD_FT(_this,0)=0;
  UD_XS(_this,0)=0;
  UD_XT(_this,0)=0;
  _this->m_nGrany = itSrc->m_nGrany;
  _this->m_nWsr   = CFst_Wsr_GetType(itSrc,&_this->m_nIcW);
  _this->m_nIcTis = CData_FindComp(AS(CData,itSrc->td),NC_TD_TIS);
  _this->m_nIcTos = CData_FindComp(AS(CData,itSrc->td),NC_TD_TOS);
  BASEINST(_this)->m_nCheck = BASEINST(_this)->m_nCheck>BASEINST(itSrc)->m_nCheck?BASEINST(_this)->m_nCheck:BASEINST(itSrc)->m_nCheck;

  /* NO RETURNS BEYOND THIS POINT! */
  ICREATEEX(CData,idAux,"id_det_rt",
    CDlpObject_FindWord(BASEINST(_this),"det_rt",WL_TYPE_FIELD));
  _this->m_idDetRt = BASEINST(idAux);
  ICREATEEX(CData,idAux,"CFst_DeterminizeUnit~idAux",NULL );
  _this->m_lpDetSt = CFst_Ssr_Init(_this->m_nGrany);
  lpTI = CFst_STI_Init(itSrc,nUnit,FSTI_SORTINI);                              /* Sorted transition iterator        */
  DLPASSERT(lpTI->nOfTTis>0);

  /* Create start state */
  IFCHECK printf("\n     Creating dst. zero state",nIniDst);
  CFst_Det_AddState(_this,(SD_FLG(itSrc,UD_FS(itSrc,nUnit))&0x01)==0x01);

  /* Create start residual(s) */
  CFst_Det_AddResidual(_this,0,0,-1,CFst_Wsr_NeMult(_this->m_nWsr));           /* Residual of start state           */

  /* Build destination transducer */
  for (nIniDst=0; nIniDst<UD_XS(_this,0); nIniDst++)
  {
    IFCHECK printf("\n     Continuing from dst. state %ld",(long)nIniDst);

    /* Clear auxilary table */
    CData_SetNRecs(idAux,0);

    /* Find residuals of current state in residual table */
    nFR = *(FST_ITYPE*)(CData_XAddr(AS(CData,_this->sd),nIniDst,_this->m_nIcSdAux  )); /* First residual of state */
    nXR = *(FST_ITYPE*)(CData_XAddr(AS(CData,_this->sd),nIniDst,_this->m_nIcSdAux+1)); /* Number of residuals     */

    /* If a state has no residuals at all it was generated by CFst_Det_AddPath
       as an intermediate state of a multi-character output string. Such states
       do not need to be processed further.*/
    if (nXR==0)
    {
      IFCHECK printf("\n       Intermediate state --> skipping");
      continue;
    }

    /* Loop over residuals of current state and gather auxilary table */
    IFCHECK printf("\n       Loading residuals");
    for (nR=nFR, bFinal=FALSE; nR<nFR+nXR; nR++)
    {
      IFCHECKEX(1) CFst_Det_PrtResidual(_this,nR);

      /* Load source transitions leaving source state into idAux */
      bFinal |=
        CFst_Det_LoadAuxTable
        (
          _this,itSrc,nUnit,
          *(FST_ITYPE*)CData_XAddr(AS(CData,_this->m_idDetRt),nR,0),                     /* Initial state in src. transducer */
          *(FST_ITYPE*)CData_XAddr(AS(CData,_this->m_idDetRt),nR,1),                     /* Residual string                  */
          *(FST_WTYPE*)CData_XAddr(AS(CData,_this->m_idDetRt),nR,2),                     /* Residual weight                  */
          idAux,
          lpTI
        );
    }
    IFCHECK if(bFinal) printf("\n       Final state.");

    if (CData_GetNRecs(idAux)>0)
    {
      /* Sort auxilary table by input symbol */
      qsort(CData_XAddr(idAux,0,0),CData_GetNRecs(idAux),CData_GetRecLen(idAux),cf_comp1_itype_up);

      IFCHECKEX(2) CFst_Det_PrintAuxTable(_this,idAux);

      /* Traverse auxilary table by groups of equal input symbols */
      nXR = CData_GetNRecs(idAux);
      for
      (
        nR=0,nFR=0,nSumStr=CFst_Ssr_NeAdd(),nSumW=CFst_Wsr_NeAdd(_this->m_nWsr);
        nR<nXR;
        nR++
      )
      {
        /* Get input symbol and sum up output strings and weights */
        nTis    = *(FST_STYPE*)CData_XAddr(idAux,nR,1);
        nSumStr = CFst_Ssr_Add(_this->m_lpDetSt,nSumStr,*(FST_ITYPE*)CData_XAddr(idAux,nR,2));
        nSumW   = CFst_Wsr_Op(_this,nSumW,*(FST_WTYPE*)CData_XAddr(idAux,nR,3),OP_ADD);
        bFsr    = (BOOL)*(BYTE*)CData_XAddr(idAux,nR,4);

        /* End of group */
        if
        (
          nR==nXR-1                                    ||                      /* End of table                      */
          nTis!=*(FST_STYPE*)CData_XAddr(idAux,nR+1,1)                         /* Next input symbol different       */
        )
        {
          /* Create new state(s) and transition(s) in destination transducer */
          nTDst = CFst_Det_AddPath(_this,nIniDst,nTis<-1?-1:nTis,nSumStr,nSumW);

          /* Store new residuals */
          IFCHECK printf("\n       Storing residuals");
          for (nR2=nFR; nR2<=nR; nR2++)
            CFst_Det_AddResidual
            (
              _this,UD_XS(_this,0)-1,*(FST_ITYPE*)CData_XAddr(idAux,nR2,0),
              CFst_Ssr_Dif(_this->m_lpDetSt,*(FST_ITYPE*)CData_XAddr(idAux,nR2,2),nSumStr),
              bFsr
              ? CFst_Wsr_NeMult(_this->m_nWsr)
              : CFst_Wsr_Op(_this,*(FST_WTYPE*)CData_XAddr(idAux,nR2,3),nSumW,OP_DIV)
            );

          /* Check last new destination state
             (try to find previously generated identical state) */
          IFCHECK printf("\n       Comparing state residual tables (states *<-->%ld)",(long)UD_XS(_this,0)-1);
          for (nS=0; nS<UD_XS(_this,0)-1; nS++)
            if (CFst_Det_CmpResiduals(_this,nS,UD_XS(_this,0)-1,&lpMap,&nMapSi))
            {
              IFCHECK printf("\n       State %ld identified as state %ld --> removing copy",(long)UD_XS(_this,0)-1,(long)nS);

              /* Modify last new transition */
              TD_TER(_this,nTDst)=nS;

              /* Delete last new state and its residual table */
              UD_XS(_this,0)--;
              CData_IncNRecs(AS(CData,_this->sd),-1);
              CData_IncNRecs(AS(CData,_this->m_idDetRt),nR-nFR+1);
              break;
            }

          /* Reset string and weight sums */
          nSumStr = CFst_Ssr_NeAdd();
          nSumW   = CFst_Wsr_NeAdd(_this->m_nWsr);
          nFR     = nR+1;
        }

      }
    }
    else if (!bFinal)
    {
      /* Broken path! */
      IFCHECK printf("\n       BROKEN PATH.");
      bFinal=TRUE;
    }

    /* Make state final */
    if (bFinal)
      SD_FLG(_this,nIniDst)|=0x01;

    IFCHECKEX(2)
    {
      dlp_inc_printlines(dlp_maxprintlines());
      dlp_if_printstop();
    }
  }

  /* Clean up */
  dlp_free(lpMap);
  CFst_STI_Done(lpTI);
  IDESTROY(idAux);
  idAux=AS(CData,_this->m_idDetRt);
  IDESTROY(idAux);
  CFst_Ssr_Done(_this->m_lpDetSt);
  _this->m_lpDetSt = NULL;
  _this->m_idDetRt = NULL;
  CData_DeleteComps(AS(CData,_this->sd),_this->m_nIcSdAux,2);
  return O_K;
}

/*
 * Manual page at fst_man.def
 */
INT16 CGEN_PUBLIC CFst_Determinize(CFst* _this, CFst* itSrc, INT32 nUnit)
{
  CFst* itUnit = NULL;                                                         /* Current unit                      */
  INT32  nU     = 0;                                                            /* Current unit index                */

  /* Validation */
  CHECK_THIS_RV(NOT_EXEC);
  CFst_Check(_this);
  CFst_Check(itSrc);
  if (nUnit>=UD_XXU(itSrc)) return IERROR(_this,FST_BADID,"unit",nUnit,0);

  /* NO RETURNS BEYOND THIS POINT! */
  CREATEVIRTUAL(CFst,itSrc,_this);
  CFst_Reset(BASEINST(_this),TRUE);
  if (nUnit<0) { ICREATEEX(CFst,itUnit,"CFst_Determinize~itUnit",NULL); }
  else         itUnit = _this;

  if (BASEINST(_this)->m_nCheck>BASEINST(itUnit)->m_nCheck) BASEINST(itUnit)->m_nCheck=BASEINST(_this)->m_nCheck;
  if (BASEINST(itSrc)->m_nCheck>BASEINST(itUnit)->m_nCheck) BASEINST(itUnit)->m_nCheck=BASEINST(itSrc)->m_nCheck;

  /* Determinize units */
  for (nU=nUnit<0?0:nUnit; nU<UD_XXU(itSrc); nU++)
  {
    CFst_DeterminizeUnit(itUnit,itSrc,nU);
    if (nUnit>=0) break;
    CFst_Cat(_this,itUnit);
  }

  /* Copy input and output symbol tables */
  CData_Copy(_this->is,itSrc->is);
  CData_Copy(_this->os,itSrc->os);

  /* Clean up */
  DLPASSERT(itUnit->m_lpDetSt==NULL);                                          /* String table not destroyed        */
  DLPASSERT(itUnit->m_idDetRt==NULL);                                          /* Residual table not destroyed      */
  if (nUnit<0) IDESTROY(itUnit);
  DESTROYVIRTUAL(itSrc,_this);
  CFst_Check(_this);                                                           /* TODO: Remove after debugging      */
  return O_K;
}

/**
 * Minimizes one unit. There are no checks performed.
 *
 * @param _this Pointer to this (destination) automaton instance
 * @param itSrc Pointer to source automaton instance
 * @param nUnit Index of unit to minimize
 * @return O_K if successfull, a (negative) error code otherwise
 * @see Minimize CFst_Minimize
 */
INT16 CGEN_PROTECTED CFst_MinimizeUnit(CFst* _this, CFst* itSrc, INT32 nUnit)
{
  CFst* itAux  = NULL;
  INT16 nErr   = O_K;

  DLPASSERT(_this!=itSrc);

  ICREATEEX(CFst,itAux,"CFst_MinimizeUnit~itAux",NULL);

  CFst_CopyUi(_this,itSrc,NULL,nUnit);
  CFst_Reverse(_this,0);
  IF_OK((nErr = CFst_DeterminizeUnit(itAux,_this,0)))
  {
    CFst_Reverse(itAux,0);
    nErr = CFst_DeterminizeUnit(_this,itAux,0);
  }

  IDESTROY(itAux);
  return nErr;
}

/*
 * Manual page at fst_man.def
 */
#define NS    4
#define QNONE ((struct fstl_s*)-1)
struct fstl_s;
struct fstl_t;
struct fstl_t {
  struct fstl_t *nnxt,*nprv,*bnxt,*bprv;
  struct fstl_s *b,*n;
  BYTE *lpT;
  INT32 id;
};
struct fstl_s {
  struct fstl_s *qnxt;
  struct fstl_t *b,*n;
  INT32 xn,xb;
  INT32 id;
  char nodo;
};
INT16 CGEN_PROTECTED CFst_Lazymin(CFst* _this)
{
  struct fstl_t *rT;
  struct fstl_s *rS, *rQ=NULL;
  FST_STYPE lpSDef[NS] = { -1, -1, -1, 0 }; /* Default symbols to use for IS,OS,PHN,STK */
  BYTE *lpT,*lpFT;                          /* Transition pointer */
  FST_TID_TYPE* lpTI;                       /* Transition iterator */
  INT32 lpSOf[NS];                          /* Byte offset of symbols (IS,OS,PHN,STK) */
  INT32 nS,nXS,nFS;                         /* State index + number + start id */
  INT32 nT,nXT;                             /* Transition index + number */

  /* Initialize buffer fields */
  rS=(struct fstl_s*)dlp_calloc(nXS=UD_XS(_this,0),sizeof(struct fstl_s));
  rT=(struct fstl_t*)dlp_calloc(nXT=UD_XT(_this,0),sizeof(struct fstl_t));
  lpTI = CFst_STI_Init(_this,0,FSTI_SORTINI);
  lpFT = CFst_STI_TfromS(lpTI,-1,NULL);

  /* Initialize lpSOf */
  lpSOf[0]=lpTI->nOfTTis ? lpTI->nOfTTis : -1;
  lpSOf[1]=lpTI->nOfTTos ? lpTI->nOfTTos : -1;
  for(nS=2;nS<NS;nS++){
    lpSOf[nS] = CData_FindComp(AS(CData,_this->td),nS==2?"~PHN":"~STK");
    if(lpSOf[nS]>=0) lpSOf[nS] = CData_GetCompOffset(AS(CData,_this->td),lpSOf[nS]);
  }

  /* Initialize rS+rT */
  memset(rS,0,sizeof(struct fstl_s)*nXS);
  for(nT=0,lpT=lpFT;nT<nXT;nT++,lpT+=lpTI->nRlt){
    FST_ITYPE nTer=*CFst_STI_TTer(lpTI,lpT);                   /* Terminal state */
    FST_ITYPE nIni=*CFst_STI_TIni(lpTI,lpT);                   /* Initial state */
    rT[nT].lpT=lpT;
    rT[nT].b=rS+nIni;
    rT[nT].n=rS+nTer;
    rT[nT].id=nT;
    rS[nIni].xn++;
    rS[nTer].xb++;
    if(rS[nIni].n) rS[nIni].n->nprv=rT+nT;
    rT[nT].nnxt=rS[nIni].n; rS[nIni].n=rT+nT;
    rT[nT].nprv=NULL;
    if(rS[nTer].b) rS[nTer].b->bprv=rT+nT;
    rT[nT].bnxt=rS[nTer].b; rS[nTer].b=rT+nT;
    rT[nT].bprv=NULL;
  }
  nFS=UD_FS(_this,0);
  rS[0].nodo=1;
  for(nS=0;nS<nXS;nS++){
    rS[nS].qnxt=QNONE;
    rS[nS].id=nS;
    if(SD_FLG(_this,nFS+nS)&SD_FLG_FINAL) rS[nS].nodo=1;
    else if(nS && (rS[nS].xb==1 || rS[nS].xn==1)){
      rS[nS].qnxt=rQ; rQ=rS+nS;
    }
  }

  /* Do all interessting things */
  while(rQ){
    struct fstl_s *s=rQ;
    rQ=rQ->qnxt;
    s->qnxt=QNONE;

    if(s->xb==1){
      /* Work on states with only one ingoing transition */
      struct fstl_t *t=s->b, *to, *tp;
      struct fstl_s *s2=t->b;
      /* Check symbols movablility */
      for(nS=0;nS<NS;nS++) if(lpSOf[nS]>=0 && *(FST_STYPE*)(t->lpT+lpSOf[nS])!=lpSDef[nS]){
        for(to=s->n;to;to=to->nnxt) if(*(FST_STYPE*)(to->lpT+lpSOf[nS])!=lpSDef[nS]) break;
        if(to) break;
      }
      if(nS<NS) continue;
      if((tp=s->n)){
        /* Move all outgoing transitions from s to s2 + move symbols */
        for(to=s->n;to;to=to->nnxt){
          FST_STYPE sym;
          for(nS=0;nS<NS;nS++) if(lpSOf[nS]>=0 && (sym=*(FST_STYPE*)(t->lpT+lpSOf[nS]))!=lpSDef[nS]) *(FST_STYPE*)(to->lpT+lpSOf[nS])=sym;
          if(lpTI->nOfTW>0) *CFst_STI_TW(lpTI,to->lpT)=CFst_Wsr_Op(_this,*CFst_STI_TW(lpTI,to->lpT),*CFst_STI_TW(lpTI,t->lpT),OP_MULT);
          *CFst_STI_TIni(lpTI,to->lpT)=*CFst_STI_TIni(lpTI,t->lpT);
          s2->xn++;
          to->b=s2;
          tp=to;
        }
        if(s2->n) s2->n->nprv=tp;
        tp->nnxt=s2->n; s2->n=s->n;
      }
      /* Move transition t from s2 to s */
      if(t->nprv) t->nprv->nnxt=t->nnxt;
      else s2->n=t->nnxt;
      t->b=s; s->n=t;
      s2->xn--;
      /* Insert s2 in queue */
      if(s2->qnxt==QNONE && s2->xn==1 && !s2->nodo){ s2->qnxt=rQ; rQ=s2; }
    }else if(s->xn==1){
      /* Work on states with only one outgoing transition */
      struct fstl_t *t=s->n,*to,*tp;
      struct fstl_s *s2=t->n;
      /* Check symbols movablility */
      for(nS=0;nS<NS;nS++) if(lpSOf[nS]>=0 && *(FST_STYPE*)(t->lpT+lpSOf[nS])!=lpSDef[nS]){
        for(to=s->b;to;to=to->bnxt) if(*(FST_STYPE*)(to->lpT+lpSOf[nS])!=lpSDef[nS]) break;
        if(to) break;
      }
      if(nS<NS) continue;
      if((tp=s->b)){
        /* Move all ingoing transitions from s to s2 + move symbols */
        for(to=s->b;to;to=to->bnxt){
          FST_STYPE sym;
          for(nS=0;nS<NS;nS++) if(lpSOf[nS]>=0 && (sym=*(FST_STYPE*)(t->lpT+lpSOf[nS]))!=lpSDef[nS]) *(FST_STYPE*)(to->lpT+lpSOf[nS])=sym;
          if(lpTI->nOfTW>0) *CFst_STI_TW(lpTI,to->lpT)=CFst_Wsr_Op(_this,*CFst_STI_TW(lpTI,to->lpT),*CFst_STI_TW(lpTI,t->lpT),OP_MULT);
          *CFst_STI_TTer(lpTI,to->lpT)=*CFst_STI_TTer(lpTI,t->lpT);
          s2->xb++;
          to->n=s2;
          tp=to;
        }
        if(s2->b) s2->b->bprv=tp;
        tp->bnxt=s2->b; s2->b=s->b;
      }
      /* Move transition t from s2 to s */
      if(t->bprv) t->bprv->bnxt=t->bnxt;
      else s2->b=t->bnxt;
      t->n=s; s->b=t;
      s2->xb--;
      /* Insert s2 in queue */
      if(s2->qnxt==QNONE && s2->xb==1 && !s2->nodo){ s2->qnxt=rQ; rQ=s2; }
    }
  }

  /* Free buffer fields */
  CFst_STI_Done(lpTI);
  dlp_free(rT);
  dlp_free(rS);

  /* Delete unused states and transitions */
  _this->m_bLocal=TRUE;
  CFst_Trim(_this,0,0);

  return O_K;
}

/*
 * Manual page at fst_man.def
 */
INT16 CGEN_PUBLIC CFst_Minimize(CFst* _this, CFst* itSrc, INT32 nUnit)
{
  CFst* itUnit = NULL;                                                         /* Current unit                      */
  INT32  nU     = 0;                                                            /* Current unit index                */
  BOOL  bLazy;

  /* Validation */
  CHECK_THIS_RV(NOT_EXEC);
  CFst_Check(_this);
  CFst_Check(itSrc);
  if (nUnit>=UD_XXU(itSrc)) return IERROR(_this,FST_BADID,"unit",nUnit,0);
  bLazy = _this->m_bLazy;

  /* NO RETURNS BEYOND THIS POINT! */
  CREATEVIRTUAL(CFst,itSrc,_this);
  CFst_Reset(BASEINST(_this),TRUE);
  if (nUnit<0) { ICREATEEX(CFst,itUnit,"CFst_Minimize~itUnit",NULL); }
  else         itUnit = _this;

  if (BASEINST(_this)->m_nCheck>BASEINST(itUnit)->m_nCheck) BASEINST(itUnit)->m_nCheck=BASEINST(_this)->m_nCheck;
  if (BASEINST(itSrc)->m_nCheck>BASEINST(itUnit)->m_nCheck) BASEINST(itUnit)->m_nCheck=BASEINST(itSrc)->m_nCheck;

  /* Minimize units */
  for (nU=nUnit<0?0:nUnit; nU<UD_XXU(itSrc); nU++)
  {
    if(bLazy)
    {
      CFst_CopyUi(itUnit,itSrc,NULL,nU);
      CFst_Lazymin(itUnit);
    }
    else CFst_MinimizeUnit(itUnit,itSrc,nU);
    if (nUnit>=0) break;
    CFst_Cat(_this,itUnit);
  }

  /* Copy input and output symbol tables */
  CData_Copy(_this->is,itSrc->is);
  CData_Copy(_this->os,itSrc->os);

  /* Clean up */
  if (nUnit<0) IDESTROY(itUnit);
  DESTROYVIRTUAL(itSrc,_this);
  CFst_Check(_this);                                                           /* TODO: Remove after debugging      */
  return O_K;
}

/* EOF */
