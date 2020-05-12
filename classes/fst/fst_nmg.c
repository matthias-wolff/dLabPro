/* dLabPro class CFst (fst)
 * - n-multigrams
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
#include "dlp_math.h"

/**
 * <p>Stores a sequence into an n-multigram.</p>
 * <h3>Note</h3>
 * <p>The method will only store the first {@link max_len m_nMaxLen} symbols of
 * the sequence <code>lpSseq</code>.</p>
 *
 * @param _this   n-muligram instance (<code>CFst</code> instance in acceptor
 *                mode) to append sequence to
 * @param nUnit   Unit to append sequence to
 * @param nSeqId  Terminal state index (=unique identifier) of sequence in the
 *                n-multigram to append <code>lpSseq</code> to (0 for root)
 * @param lpSseq  A pointer to a <code>FST_SEQ_TYPE</code> structure defining
 *                the symbols of the sequence to store. The items of the
 *                sequence must be of type <code>FST_STYPE</code>.
 * @param lpTseq  A pointer to a <code>FST_SEQ_TYPE</code> structure defining
 *                the terminal states of the sequence to store, can be
 *                <code>NULL</code>. The items of the sequence must be of type
 *                <code>FST_ITYPE</code>.
 * @param lpCseq  A pointer to a <code>FST_SEQ_TYPE</code> structure defining
 *                increment values for transition reference counters in the
 *                n-multigram (may be <code>NULL</code>). The items of the
 *                sequence must be of type <code>FST_ITYPE</code>. If this
 *                parameter is <code>NULL</code> the default increment is 1.
 * @param bSubSeq Also store all subsequences starting at the 2nd, 3rd, 4th,
 *                etc. position of the sequence (multigram-mode). Make all
 *                states final.
 * @return        Terminal state index (=unique identifier) of the stored
 *                sequence if successful, a (negative) error code otherwise
 */
INT32 CGEN_PROTECTED CFst_Nmg_StoreSeq
(
  CFst*         _this,
  INT32          nUnit,
  FST_ITYPE     nSeqId,
  FST_SEQ_TYPE* lpSseq,
  FST_SEQ_TYPE* lpTseq,
  FST_SEQ_TYPE* lpCseq,
  BOOL          bSubSeq
)
{
  FST_TID_TYPE* lpTI         = NULL;                                            /* Transducer iterator data structure*/
  BYTE*         lpT          = NULL;                                            /* Pointer to current transition     */
  BYTE*         lpSymb       = NULL;                                            /* Pointer to current symbol in seq. */
  FST_ITYPE     nTer         = 0;                                               /* Current terminal state in seq.    */
  FST_ITYPE     nS           = 0;                                               /* Current state                     */
  FST_ITYPE     nT           = 0;                                               /* Current transition                */
  INT32         nFirst       = 0;                                               /* First symbol                      */
  INT32         nSymb        = 0;                                               /* Current symbol                    */
  BOOL          bBranch      = FALSE;                                           /* Branch flag                       */
  BOOL          bUnitChanged = FALSE;                                           /* Unit changed flag                 */
  BOOL          bFinal       = FALSE;                                           /* Make final state flag             */

  /* Validate */
  CHECK_THIS_RV(-1);
  if (!lpSseq) return -1;
  DLPASSERT(nUnit>=0 && nUnit<UD_XXU(_this)); /* Check before calling! */
  IFCHECK printf("\n   Adding sequence to unit %ld ",(long)nUnit);

  if (nSeqId<0 || nSeqId>=UD_XS(_this,nUnit)) nSeqId=0;

  /* Initialization */
  if (UD_XS(_this,nUnit)==0) CFst_Addstates(_this,nUnit,1,FALSE);               /* Add start state if none present   */
  if (UD_MXT(_this)-UD_XXT(_this) < _this->m_nGrany)                            /* Get space for adding transitions  */
    CData_Realloc(AS(CData,_this->td),UD_XXT(_this)+_this->m_nGrany);           /* | (saves 1x sorting transitions!) */

  /* NO RETURNS BEYOND THIS POINT! */
  lpTI = CFst_STI_Init(_this,nUnit,FSTI_SORTINI|FSTI_SLOPPY);
  DLPASSERT(lpTI->nOfTTis>0);                                                   /* Check before calling! */
  IFCHECK
  {
    printf(" nFT=%ld",(long)((lpTI->lpFT-CData_XAddr(AS(CData,_this->td),0,0))/lpTI->nRlt));
    printf(" nFTu=%ld",(long)((lpTI->lpFTunsrtd-CData_XAddr(AS(CData,_this->td),0,0))/lpTI->nRlt));
  }

  /* Loop over all sub-sequences */
  for (nFirst=0; nFirst<lpSseq->nCnt; nFirst++)
  {
    IFCHECK printf("\n   Store sub-seq. %ld at state %ld: ",(long)nFirst,(long)nSeqId);
    nS = nSeqId;

    /* Loop over symbol sequence starting at symbol nFirst */
    for
    (
      nSymb=nFirst, lpSymb=lpSseq->lpItm+(lpSseq->nOfs*nFirst), bBranch=FALSE;
      nSymb<lpSseq->nCnt && (nSymb-nFirst)<_this->m_nMaxLen;
      nSymb++, lpSymb+=lpSseq->nOfs
    )
    {
      IFCHECK
      {
        if (nSymb>nFirst) printf(" => ");
        printf("%ld (%ld->",(long)(*(FST_STYPE*)lpSymb),(long)nS);
      }
      bFinal = (bSubSeq || nSymb==lpSseq->nCnt-1);                              /* End of sequence!                  */

      /* Seek transition from state nS with input symbol lpSeq[i] */
      lpT = NULL;
      nTer = -1;
      if (!lpTseq)                                                              /*     No terminal state sequence    */
      {                                                                         /*     >>                            */
        if (!bBranch)                                                           /*       Not yet branched            */
        {                                                                       /*       >>                          */
          while ((lpT=CFst_STI_TfromS(lpTI,nS,lpT))!=NULL)                      /*         Seek matching trans.      */
            if (*(FST_STYPE*)lpSymb == *CFst_STI_TTis(lpTI,lpT)) break;         /*         ...                       */
        }                                                                       /*       <<                          */
      }                                                                         /*     <<                            */
      else                                                                      /*     Have terminal state sequence  */
      {                                                                         /*     >>                            */
        nTer = *(FST_ITYPE*)(lpTseq->lpItm+(lpTseq->nOfs*(nFirst+nSymb)));      /*       Get terminal state index    */
        while ((lpT=CFst_STI_TfromS(lpTI,nS,lpT))!=NULL)                        /*       Seek matching trans.        */
          if (*(FST_STYPE*)lpSymb == *CFst_STI_TTis(lpTI,lpT) &&                /*       ...                         */
              nTer == *CFst_STI_TTer(lpTI,lpT)) break;                          /*       ...                         */
      }                                                                         /*     <<                            */

      /* No matching transition found or already branched */
      if (!lpT)                                                                 /*     Found usable n-gram?          */
      {                                                                         /*     >> NO:                        */
        /* Add one new state and one new transition and store input symbol */   /*       - - - - - - - - - - - - - - */
        bBranch      = TRUE;                                                    /*       Remember new branch started */
        bUnitChanged = TRUE;                                                    /*       Remember unit changed       */
        /* WARNING: THIS INVALIDATES ALL POINTERS IN lpTI! --> */               /*                                   */
        if (nTer<0 || nTer>=UD_XS(_this,nUnit))                                 /*       Have no valid term. state   */
        {                                                                       /*       >>                          */
          IFCHECK printf("*");                                                  /*         Protocol (verbose level 1)*/
          nTer = CFst_Addstates(_this,nUnit,1,bFinal)-UD_FS(_this,nUnit);       /*         Create new terminal state */
          /* TODO: Replace all occurrences of current state index in lpTSeq by nTer! */
        }                                                                       /*       <<                          */
        nT = CFst_Addtrans(_this,nUnit,nS,nTer);                                /*       Add transition              */
        /* <-- */                                                               /*                                   */
        lpT = CData_XAddr(AS(CData,_this->td),nT,0);                            /*       Calculate correct trans.ptr.*/
        *CFst_STI_TTis(lpTI,lpT) = *(FST_STYPE*)lpSymb;                         /*       Store input symbol          */
        if(lpTI->nOfTRc) *CFst_STI_TRc(lpTI,lpT) = 0;                           /*       Initialize ref. counter     */
      }                                                                         /*     <<                            */
      else                                                                      /*     YES:                          */
      {                                                                         /*     >>                            */
        bUnitChanged = FALSE;                                                   /*       Remember unit not changed   */
        if (bFinal)                                                             /*       Shall be a final state?     */
          SD_FLG(_this,lpTI->nFS+*CFst_STI_TTer(lpTI,lpT))|=SD_FLG_FINAL;       /*         Make it so                */
      }                                                                         /*     <<                            */
      IFCHECK printf("%d)[%ld]",(int)*CFst_STI_TTer(lpTI,lpT),                  /*     Protocol (verbose level 1)    */
        (long)((lpT-CData_XAddr(AS(CData,_this->td),0,0))/CData_GetRecLen(AS(CData,_this->td))));

      /* Increment transition reference counter */
      if (lpTI->nOfTRc)
      {
        if (lpCseq && nSymb<lpCseq->nCnt)
          (*CFst_STI_TRc(lpTI,lpT)) +=
            *(FST_ITYPE*)(lpCseq->lpItm+(nSymb*lpCseq->nOfs));
        else
          (*CFst_STI_TRc(lpTI,lpT))++;
      }

      /* Next state */
      nS = *CFst_STI_TTer(lpTI,lpT);

      /* Re-initialize iterator */
      if (bUnitChanged) CFst_STI_UnitChanged(lpTI,FSTI_CADD);
    }

    /* Stop if no sub-sequencing */
    if (!bSubSeq) break;
  }

   CFst_STI_Done(lpTI);
  return nS;
}

/*
 * Manual page in fst_man.def
 */
INT16 CGEN_PUBLIC CFst_Addseq
(
  CFst*  _this,
  CData* idSrc,
  INT32   nIcTis,
  INT32   nIcTer,
  INT32   nIcRci,
  INT32   nUnit
)
{
  FST_SEQ_TYPE Sseq;                                                            /* Symbol sequence                   */
  FST_SEQ_TYPE Tseq;                                                            /* Terminal state sequence           */
  FST_SEQ_TYPE Cseq;                                                            /* Reference counter increment seq.  */
  INT32        i    = 0;                                                        /* Loop counter                      */
  INT32        nS   = -1;                                                       /* Final state of update sequence    */

  /* Validate */
  CHECK_THIS_RV(NOT_EXEC);

  if (!idSrc || CData_IsEmpty(idSrc)) return IERROR(_this,ERR_NULLINST,"idSrc",0,0);
  if (nUnit<0 || nUnit>UD_XXU(_this)) return IERROR(_this,FST_BADID,"unit",nUnit,0);

  if (nIcTis<0 || nIcTis>=CData_GetNComps(idSrc))
    return IERROR(_this,FST_BADID,"source component",nIcTis,0);

  /* Initialize empty n-gram */
  if (CData_IsEmpty(AS(CData,_this->ud)))
  {
    CData_AddComp(AS(CData,_this->td),NC_TD_TIS,DLP_TYPE(FST_STYPE));
    CData_AddComp(AS(CData,_this->td),NC_TD_RC ,T_LONG             );
    CData_AddComp(AS(CData,_this->td),NC_TD_PSR,DLP_TYPE(FST_WTYPE));
    CFst_Addunit(_this,"Nmg");
  }

  if (CData_FindComp(AS(CData,_this->td),NC_TD_TIS)<0)
    return IERROR(_this,FST_MISS,"component",NC_TD_TIS,"transition table");

  /* Fetch symbol sequence */
  if (CData_GetCompType(idSrc,nIcTis)==DLP_TYPE(FST_STYPE))
  {
    Sseq.lpItm = CData_XAddr(idSrc,0,nIcTis);
    Sseq.nOfs  = CData_GetRecLen(idSrc);
    Sseq.nCnt  = CData_GetNRecs(idSrc);
  }
  else if (dlp_is_numeric_type_code(CData_GetCompType(idSrc,nIcTis)))
  {
    Sseq.lpItm = (BYTE*)dlp_calloc(CData_GetNRecs(idSrc),sizeof(FST_STYPE));
    Sseq.nOfs  = sizeof(FST_STYPE);
    Sseq.nCnt  = CData_GetNRecs(idSrc);
    for (i=0; i<Sseq.nCnt; i++)
      *(FST_STYPE*)(Sseq.lpItm+i*sizeof(FST_STYPE)) =
        (FST_STYPE)CData_Dfetch(idSrc,i,nIcTis);
  }
  else return IERROR(_this,FST_BADID,"source component",nIcTis,0);

  /* Fetch terminal state sequence */
  if (CData_GetCompType(idSrc,nIcTer)==DLP_TYPE(FST_ITYPE))
  {
    Tseq.lpItm = CData_XAddr(idSrc,0,nIcTer);
    Tseq.nOfs  = CData_GetRecLen(idSrc);
    Tseq.nCnt  = CData_GetNRecs(idSrc);
  }
  else if (dlp_is_numeric_type_code(CData_GetCompType(idSrc,nIcTer)))
  {
    Tseq.lpItm = (BYTE*)dlp_calloc(CData_GetNRecs(idSrc),sizeof(FST_ITYPE));
    Tseq.nOfs  = sizeof(FST_ITYPE);
    Tseq.nCnt  = CData_GetNRecs(idSrc);
    for (i=0; i<Tseq.nCnt; i++)
      *(FST_ITYPE*)(Tseq.lpItm+i*sizeof(FST_ITYPE)) =
        (FST_ITYPE)CData_Dfetch(idSrc,i,nIcTer);
  }
  else Tseq.lpItm=NULL;

  /* Fetch reference counter increment sequence */
  if (CData_GetCompType(idSrc,nIcRci)==DLP_TYPE(FST_ITYPE))
  {
    Cseq.lpItm = CData_XAddr(idSrc,0,nIcRci);
    Cseq.nOfs  = CData_GetRecLen(idSrc);
    Cseq.nCnt  = CData_GetNRecs(idSrc);
  }
  else if (dlp_is_numeric_type_code(CData_GetCompType(idSrc,nIcRci)))
  {
    Cseq.lpItm = (BYTE*)dlp_calloc(CData_GetNRecs(idSrc),sizeof(FST_ITYPE));
    Cseq.nOfs  = sizeof(FST_ITYPE);
    Cseq.nCnt  = CData_GetNRecs(idSrc);
    for (i=0; i<Cseq.nCnt; i++)
      *(FST_ITYPE*)(Cseq.lpItm+i*sizeof(FST_ITYPE)) =
        (FST_ITYPE)CData_Dfetch(idSrc,i,nIcRci);
  }
  else Cseq.lpItm=NULL;

  /* Store sequence */
  nS = CFst_Nmg_StoreSeq(_this,nUnit,0,&Sseq,Tseq.lpItm?&Tseq:NULL,
         Cseq.lpItm?&Cseq:NULL,_this->m_bMultigram);

  /* Clean up */
  if (Sseq.lpItm && Sseq.lpItm!=CData_XAddr(idSrc,0,nIcTis)) dlp_free(Sseq.lpItm);
  if (Tseq.lpItm && Tseq.lpItm!=CData_XAddr(idSrc,0,nIcTer)) dlp_free(Tseq.lpItm);
  if (Cseq.lpItm && Cseq.lpItm!=CData_XAddr(idSrc,0,nIcRci)) dlp_free(Cseq.lpItm);

  return nS;
}

/**
 * <p>Retrieves the unique transition sequence (starting from the root state) for a given transducer input symbol
 * sequence from a forward tree.</p>
 *
 * <h3>Notes:</h3>
 * <ul>
 *   <li>There are <i>no</i> checks performed, whatsoever.</li>
 * </ul>
 *
 * @param _this  Acceptor instance
 * @param lpTI   Iterator of <code>_this</code> instance
 * @param Sseq   Sequence of symbols
 * @param lpTseq Buffer to be filled with (global) transition index sequence
 * @return       Length of fetched sequence (may be smaller than <code>Sseq.nCnt</code>)
 */
INT32 CGEN_PRIVATE CFst_Nmg_FetchSeq(CFst* _this, FST_TID_TYPE* lpTI, FST_SEQ_TYPE Sseq, FST_ITYPE* lpTseq)
{
  BYTE*     lpT  = NULL;                                                       /* Pointer to current transition      */
  BYTE*     lpS  = NULL;                                                       /* Current symbol                     */
  INT32      nS   = 0;                                                          /* Current symbol index               */
  FST_ITYPE nIni = 0;                                                          /* Current initial state              */

  /* Validate */
  DLPASSERT(lpTseq && dlp_size(lpTseq)>=Sseq.nCnt*sizeof(FST_ITYPE));
  DLPASSERT(lpTI->iFst==_this);                                                /* Check before calling!              */
  DLPASSERT(lpTI->nOfTTis>0);                                                  /* Check before calling!              */

  /* Debug message */
  IFCHECKEX(3)
  {
    printf("\n       Fetch seq.:");
    for (nS=0; nS<Sseq.nCnt; nS++)
      printf("%s%ld",(nS>0) ? "," : "",(long)*(FST_STYPE*)(Sseq.lpItm+Sseq.nOfs*nS));
    printf(" (%ld symbols)\n                   0",(long)Sseq.nCnt);
  }


  /* Loop over sequence */
  for (nS=0,nIni=0,lpS=Sseq.lpItm; nS<Sseq.nCnt; nS++,lpS+=Sseq.nOfs)
  {
    lpT = NULL;
    while ((lpT=CFst_STI_TfromS(lpTI,nIni,lpT))!=NULL)
      if (*(FST_STYPE*)lpS == *CFst_STI_TTis(lpTI,lpT))
        break;

    if (!lpT) break;

    lpTseq[nS] = CFst_STI_GetTransId(lpTI,lpT);
    nIni       = *CFst_STI_TTer(lpTI,lpT);

    IFCHECKEX(3) printf("->%ld",(long)nIni);
  }
  IFCHECKEX(3) printf(" (%ld transitions)",(long)nS);

  return nS;
}

/**
 * Copies a multigram weighting vector to a buffer and scales its values to sum up to 1.
 *
 * <h3>Notes:</h3>
 * <ul>
 *   <li>There are <i>no</i> checks performed, whatsoever.</li>
 * </ul>
 *
 * @param _this     <i>n</i>-multigram acceptor instance
 * @param idWeights Table containing exactly one record and at least <code>nLen</code> components <i>or</i> exactly
 *                  one component and at least <code>nLen</code> records. All components must be numeric.
 * @param lpW       Buffer to be filled with interpolation weights
 * @param nLen      Number of interpolation weights
 */
void CGEN_PRIVATE CFst_Nmg_GetMgiWeights
(
  CFst*      _this,
  CData*     idWeights,
  FST_WTYPE* lpW,
  INT16      nLen
)
{
  INT16     i    = 0;
  FST_WTYPE nSum = 0.;

  /* Validate */
  if (!idWeights && !lpW) return;
  DLPASSERT(lpW && dlp_size(lpW)>=nLen*sizeof(FST_WTYPE));

  /* Copy interpolation vector */
  if (CData_GetNComps(idWeights)==1)                                           /* Vector stored as one component ... */
    for (i=0; i<nLen; i++)
      lpW[i] = (FST_WTYPE)CData_Dfetch(idWeights,i,0);
  else if (CData_GetNRecs(idWeights)==1)                                       /* Vector stored as one record ...    */
    for (i=0; i<nLen; i++)
      lpW[i] = (FST_WTYPE)CData_Dfetch(idWeights,0,i);
  else DLPASSERT(FALSE);                                                       /* Inconclusive ...                   */

  /* Rescale */
  for (i=0,nSum=0.; i<nLen; i++) nSum += lpW[i];
  for (i=0; i<nLen; i++) lpW[i] = (nSum==0.) ? 1./(FST_WTYPE)nLen : lpW[i]/nSum;

  /* Protocol */
  IFCHECKEX(2)
    for (i=0; i<nLen; i++)
      printf("%s%G%s",i==0?"\n       IWVec = (":"",lpW[i],i<nLen-1?",":")");

}

/**
 * <p>Calculates the conditional probability of the last symbol in a sequence.</p>
 *
 * <p>The method has two operation modes:</p>
 * <ul>
 *   <li><b><i>n</i>-gram mode (<code>lpW == NULL</code>):</b><br>
 *   The method returns the conditional <i>n</i>-gram probability of the last symbol in the sequence <code>Sseq</code>,
 *   where <i>n</i>=<code>Sseq.nCnt</code>, and sets <code>*lpActualOrder</code> to <i>n</i>. If no such <i>n</i>-gram
 *   exists, the method returns <code>exp(-{@link wceil m_nWceil})</code> and sets <code>*lpActualOrder</code> to
 *   <code>-1</code>.<br> &nbsp;</li>
 *   <li><b>Interpolated <i>n</i>-multigram mode (<code>lpW != NULL</code>):</b><br>
 *   The method returns the condiational interpolated <i>n</i>-multigram probability of the last symbol in the sequence
 *   <code>Sseq</code>, where <i>n</i>=<code>Sseq.nCnt</code>, and sets <code>*lpActualOrder</code> to the order of the
 *   longest <i>n</i>-gram whose conditional probability was actually used by the interpolation. If none of the
 *   <i>n</i>-grams to be interpolated exists, the method returns <code>exp(-{@link wceil m_nWceil})</code> and sets
 *   <code>*lpActualOrder</code> to <code>-1</code>.</li>
 * </ul>
 *
 * <h3>Notes:</h3>
 * <ul>
 *   <li>There are <i>no</i> checks performed, whatsoever.</li>
 *   <li>Method assumes that field {@link ic_w m_nIcW} is properly set.</li>
 *   <li>Method assumes that field {@link nmg_t m_lpNmgT} points to a valid buffer with at least
 *   <code>Sseq.nCnt</code> elements.</li>
 *   <li>In interpolated <i>n</i>-multigram mode the method will disregard the zerogram probability as long as the
 *   field {@link symbols m_nSymbols} is less or equal 0 even if there is a non-zero interpolation weight specified
 *   for the zerogram. In the latter case the interpolation vector will <i>not</i> be rescaled to sum up to 1!</li>
 * </ul>
 *
 * @param _this         <i>n</i>-multigram acceptor instance
 * @param lpTI          Iterator of <code>_this</code> instance
 * @param Sseq          Sequence of symbols
 * @param lpW           Buffer containing n-multigram weights. May be <code>NULL</code>, otherwise the buffer is
 *                      expected to contain exactly <code>Sseq.nCnt+1</code> multigram weights which sum up to 1.
 * @param lpActualOrder Buffer to be filled with order of longest actually used <i>n</i>-gram, may be
 *                      <code>NULL</code>
 * @return              The conditional (interpolated) probability of the last symbol in <code>Sseq</code> under the
 *                      condition of all preceeding symbols
 */
FST_WTYPE CGEN_PRIVATE CFst_Nmg_CalcCondProb
(
  CFst*            _this,
  FST_TID_TYPE*    lpTI,
  FST_SEQ_TYPE     Sseq,
  const FST_WTYPE* lpW,
  INT16*           lpActualOrder
)
{
  FST_SEQ_TYPE Sssq;
  INT32         nIcPfl  = -1;                                                   /* Floor probability component in sd  */
  FST_ITYPE    nS      = -1;                                                   /* Terminal state of (n-1)-gram       */
  FST_WTYPE    nResult = 0.;                                                   /* Result                             */
  FST_WTYPE    nP      = 0.;                                                   /* Conditional n-gram probability     */
  INT16        nAO     = -1;                                                   /* Currently used n-gram order        */
  INT16        nMaxAO  = -1;                                                   /* Maximal actually used n-gram order */
  INT16        nXO     = (INT16)Sseq.nCnt;                                     /* Maximal n-gram order               */

  if (lpW==NULL)
  {
    /* n-gram mode */
    nResult = 0.;
    nAO     = (INT16)CFst_Nmg_FetchSeq(_this,lpTI,Sseq,_this->m_lpNmgT);
    if (nAO==nXO)
    {
      /* n-gram found: fetch conditional probability from last transition */
      nResult = *(FST_WTYPE*)CData_XAddr(AS(CData,_this->td),_this->m_lpNmgT[nXO-1],_this->m_nIcW);
      nMaxAO  = nXO;
    }
    else if (_this->m_nSymbols>0 && _this->m_nRcfloor>0.)
    {
      /* n-gram not found (Jeffrey smoothing mode): get floor probability */
      nIcPfl = CData_FindComp(AS(CData,_this->sd),"~PFL");
      DLPASSERT(nIcPfl>0)
      if (nIcPfl>0)
      {
        if (nAO==nXO-1)
        {
          /* (n-1)-gram found: fetch conditional floor prob. from last state */
          nS      = (nXO>1) ? TD_TER(_this,_this->m_lpNmgT[nXO-2]) : 0;
          nResult = *(FST_WTYPE*)CData_XAddr(AS(CData,_this->sd),nS,nIcPfl);
        }
        else
          /* (n-1)-gram not found: equal distribution of symbols */
          nResult = 1./(FST_WTYPE)_this->m_nSymbols;
      }
      nMaxAO = nXO;                                                               /* n-gram "found" (by smoothing)   */
    }
    else nMaxAO = nAO;
    DLPASSERT(!(_this->m_nSymbols>0 && _this->m_nRcfloor>0.) || (nMaxAO==nXO));   /* Smoothed n-gram always "found"! */

    /* Limit conditional probability to exp(-wceil) */
    if (nResult<exp(-_this->m_nWceil)) nResult = exp(-_this->m_nWceil);

    /* Protocol */
    IFCHECKEX(2)
    {
      INT32 i = 0;
      printf("\n       H(%ld%s",(long)*(FST_STYPE*)&Sseq.lpItm[(Sseq.nCnt-1)*Sseq.nOfs],Sseq.nCnt==1?")":"|");
      for (i=0; i<Sseq.nCnt-1; i++)
        printf("%ld%s",(long)*(FST_STYPE*)(Sseq.lpItm+i*Sseq.nOfs),(i<Sseq.nCnt-2)?",":")");
      printf(" = %g (nXO=%hd, nAO=%hd",(double)nResult,(short)nXO,(short)nMaxAO);
      if (nAO<nMaxAO) printf(" Unsmoothed nAO=%ld",(long)nAO);
      printf(")");
    }
  }
  else
  {
    /* Interpolated n-multigram mode */
    if (_this->m_nSymbols>0)
    {
      nResult = lpW[0]*1./(FLOAT64)_this->m_nSymbols;                           /* Zerogram                           */
      nMaxAO  = 0;
    }
    else
      nResult = 0.;

    memmove(&Sssq,&Sseq,sizeof(FST_SEQ_TYPE));                                 /* Copy Sseq for sub-sequences        */
    for ( ; Sssq.nCnt>0; Sssq.nCnt--,Sssq.lpItm+=Sssq.nOfs)                    /* Multigram interpolation ...        */
    {
      nP = lpW[Sssq.nCnt]*CFst_Nmg_CalcCondProb(_this,lpTI,Sssq,NULL,&nAO);    /* Get conditional n-gram probability */
      nResult += nP;                                                           /* Weighted sum                       */
      IFCHECKEX(2) printf(" *%G = %G",lpW[Sssq.nCnt],nP);
      if (nAO>nMaxAO) nMaxAO = nAO;
    }

    if (nMaxAO<0) nResult = exp(-_this->m_nWceil);

    /* Protocol */
    IFCHECKEX(2)
    {
      INT32 i = 0;
      printf("\n       P(%ld%s",(long)*(FST_STYPE*)&Sseq.lpItm[(Sseq.nCnt-1)*Sseq.nOfs],Sseq.nCnt==1?")":"|");
      for (i=0; i<Sseq.nCnt-1; i++)
        printf("%ld%s",(long)*(FST_STYPE*)(Sseq.lpItm+i*Sseq.nOfs),(i<Sseq.nCnt-2)?",":")");
      printf(" = %g (nXO=%hd, nAO=%hd)",(double)nResult,(short)nXO,(short)nMaxAO);
    }
  }

  if (lpActualOrder!=NULL) *lpActualOrder=nMaxAO;
  return nResult;
}

/**
 * <p>Calculates the logarithmic probability of a symbol sequence. Given a symbol sequence <i>S</i> =
 * (<i>s</i><sub>1</sub>, ..., <i>s</i><sub><i>N</i></sub>) the method calculates <i>w</i> = &sum;<sub><i>i</i></sub>
 * log <i>P</i>(<i>s</i><sub><i>i</i></sub>|<i>s</i><sub><i>i-O</i></sub>, ..., <i>s</i><sub><i>i</i>-1</sub>),
 * where <i>O</i> stands for the maximal <i>n</i>-gram order <code>nOrder</code>. The method calculates the conditional
 * probabilities by calling {@link Nmg_CalcCondProb CFst_Nmg_CalcCondProb} for each sub-sequence.</p>
 *
 * <p>The method has two operation modes depending on <code>idWeights</code> (see
 * {@link Nmg_CalcCondProb CFst_Nmg_CalcCondProb} for details):</p>
 * <ul>
 *   <li><i>n</i>-gram mode (<code>idWeights == NULL</code>)</li>
 *   <li>Interpolated <i>n</i>-multigram mode (<code>idWeights != NULL</code>)</li>
 * </ul>
 *
 * <h3>Notes:</h3>
 * <ul>
 *   <li>There are <i>no</i> checks performed, whatsoever.</li>
 *   <li>Method assumes that field {@link ic_w m_nIcW} is properly set.</li>
 *   <li>Method assumes that field {@link nmg_t m_lpNmgT} points to a valid buffer with at least
 *   <code>Sseq.nCnt</code> elements.</li>
 *   <li>Method assumes that field {@link nmg_w m_lpNmgW} points to a valid buffer with at least
 *   <code>nOrder</code>+1 elements.</li>
 * </ul>
 *
 * @see Nmg_CalcCondProb CFst_Nmg_CalcCondProb
 *
 * @param _this         <i>n</i>-multigram acceptor instance
 * @param lpTI          Iterator of <code>_this</code> instance
 * @param Sseq          Sequence of symbols
 * @param nOrder        <i>n</i>-gram order or maximal <i>n</i>-multigram order
 * @param idWeights     <i>n</i>-gram interpolation vector (may be <code>NULL</code>)
 * @param lpSid         Pointer to buffer to be filled with segment index (= global transition index in multigram
 *                      acceptor) for sequence (may be <code>NULL</code>)
 * @param lpActualOrder Buffer to be filled with order of longest actually used <i>n</i>-gram, may be
 *                      <code>NULL</code>
 * @return              The negative logarithmic probability of the segment
 */
FST_WTYPE CGEN_PRIVATE CFst_Nmg_CalcSeqProb
(
  CFst*         _this,
  FST_TID_TYPE* lpTI,
  FST_SEQ_TYPE  Sseq,
  INT32          nOrder,
  CData*        idWeights,
  FST_ITYPE*    lpSid,
  INT16*        lpActualOrder
)
{
  FST_SEQ_TYPE Sssq;                                                           /* Sub-sequence data struct           */
  INT32         nS     = 0;                                                     /* Current symbol index               */
  FLOAT64       nLp    = 0.;                                                    /* Log. probability sum               */
  INT16        nAO    = -1;                                                    /* Actually used n-gram order         */
  INT16        nMaxAO = -1;                                                    /* Maximal actually used n-gram order */

  /* Get multigram segment index */
  if (lpSid!=NULL)
  {
    if (Sseq.nCnt>CFst_Nmg_FetchSeq(_this,lpTI,Sseq,_this->m_lpNmgT)) *lpSid=-1;
    else *lpSid = _this->m_lpNmgT[Sseq.nCnt-1];
  }

  /* Get sequence probability */
  if (nOrder>Sseq.nCnt) nOrder = Sseq.nCnt;
  for (nS=0; nS<Sseq.nCnt; nS++)
  {
    Sssq.lpItm = Sseq.lpItm+ ((nS-nOrder+1<0) ? 0 : nS-nOrder+1)*Sseq.nOfs;     /* Fill symbol sub-sequence struct ...*/
    Sssq.nOfs  = Sseq.nOfs;
    Sssq.nCnt  = (nOrder>nS+1) ? nS+1 : nOrder;

    CFst_Nmg_GetMgiWeights(_this,idWeights,_this->m_lpNmgW,(INT16)Sssq.nCnt+1); /* Copy and scale interpol. weights   */
    nLp += log(CFst_Nmg_CalcCondProb(_this,lpTI,Sssq,_this->m_lpNmgW,&nAO));    /* Aggregate log. cond. probability   */

    if (nAO>nMaxAO) nMaxAO=nAO;
  }
  if (lpActualOrder!=NULL) *lpActualOrder = nMaxAO;

  return -nLp;
}

/*
 * Manual page in fst_man.def
 */
INT16 CGEN_PUBLIC CFst_Multigram
(
  CFst*  _this,
  CData* idSeq,
  INT32   nComp,
  CData* idWeights,
  CFst*  itDst,
  INT32   nUnit
)
{
  FST_TID_TYPE* lpTI     = NULL;                            /* Transducer iterator data struct                       */
  FST_STYPE*    lpS      = NULL;                            /* Input symbol buffer                                   */
  FST_ITYPE     nS       = 0;                               /* Current symbol (= current initial state)              */
  FST_ITYPE     nXS      = 0;                               /* Number of symbols in input sequence                   */
  FST_ITYPE     nT       = 0;                               /* Current transition                                    */
  FST_ITYPE     nIni     = 0;                               /* Current initial state                                 */
  FST_ITYPE     nTer     = 0;                               /* Current terminal state                                */
  FST_WTYPE     nW       = 0.;                              /* Current cond. symbol prob. / sequence weigth          */
  FST_ITYPE     nSid     = -1;                              /* Multigram segment index (=global transition index)    */
  INT32          nIcTis   = -1;                              /* Component index of transducer input symbols           */
  INT32          nIcW     = -1;                              /* Component index of transition probabilities           */
  INT32          nIcRcDst = -1;                              /* Component index of transition ref. counters (itDst)   */
  INT32          nIcRcNmg = -1;                              /* Component index of transition ref. counters (_this)   */
  INT32          nIcAor   = -1;                              /* Component index of maximal actually used n-gram order */
  INT32          nIcLen   = -1;                              /* Component index of segment length                     */
  INT32          nOrder   = 0;                               /* Maximal n-gram order to be used                       */
  INT16         nAO      = 0;                               /* Maximal actually used n-gram order                    */
  INT32          i        = 0;                               /* Auxilary counter                                      */
  FST_SEQ_TYPE  Sseq;                                       /* Symbol sequence data struct                           */

  /* Protocol */
  IFCHECK
  {
    printf("\n "); dlp_fprint_x_line(stdout,'-',99);
    printf("\n CFst_Multigram(");
    printf("%s,",_this?BASEINST(_this)->m_lpInstanceName:"NULL");
    printf("%s,",idSeq?BASEINST(idSeq)->m_lpInstanceName:"NULL");
    printf("%ld,",(long)nComp);
    printf("%s,",idWeights?BASEINST(idWeights)->m_lpInstanceName:"NULL");
    printf("%s,",itDst?BASEINST(itDst)->m_lpInstanceName:"NULL");
    printf("%ld)\n",(long)nUnit);
    if (_this->m_bSegment) printf(" /segment");
    printf("\n "); dlp_fprint_x_line(stdout,'-',99); printf("\n ");
  }

  /* Validate */
  CHECK_THIS_RV(NOT_EXEC);
  if (itDst==NULL ) return IERROR(_this,ERR_NULLARG,"itDst",0,0);
  if (itDst==_this) return IERROR(_this,ERR_GENERIC,"Source and destination must not be equal.",0,0);

  if (!CFst_GetType(_this,FST_FWDTREE))
    return IERROR(_this,ERR_GENERIC,"This instance must be a forward tree.",0,0);
  if (CData_FindComp(AS(CData,_this->td),NC_TD_TIS)<0)
    return IERROR(_this,FST_MISS,"input symbol component",NC_TD_TIS,"transition table");
  if (CData_FindComp(AS(CData,_this->td),NC_TD_RC)<0)
    return IERROR(_this,FST_MISS,"transition reference counter component",NC_TD_RC,"transition table");
  if (CFst_Wsr_GetType(_this,&_this->m_nIcW)!=FST_WSR_PROB)
    return IERROR(_this,FST_MISS,"transition probability component",NC_TD_PSR,"transition table");

  /* Initialize destination */
  CFst_Reset(BASEINST(itDst),TRUE);
  CFst_Addunit(itDst,BASEINST(idSeq)->m_lpInstanceName);
  CData_AddComp(AS(CData,itDst->td),NC_TD_TIS,DLP_TYPE(FST_ITYPE));
  CData_AddComp(AS(CData,itDst->td),_this->m_bSegment?NC_TD_LSR:NC_TD_PSR,DLP_TYPE(FST_WTYPE));
  CData_AddComp(AS(CData,itDst->td),"hlen",T_SHORT);
  nIcTis = CData_FindComp(AS(CData,itDst->td),NC_TD_TIS);
  nIcW   = CData_FindComp(AS(CData,itDst->td),(char*)(_this->m_bSegment?NC_TD_LSR:NC_TD_PSR));
  nIcAor = CData_FindComp(AS(CData,itDst->td),"hlen");
  if (_this->m_bSegment)
  {
    CData_AddComp(AS(CData,itDst->td),NC_TD_RC,T_LONG);
    nIcRcDst = CData_FindComp(AS(CData,itDst->td),NC_TD_RC);
    nIcRcNmg = CData_FindComp(AS(CData,_this->td),NC_TD_RC);
    CData_AddComp(AS(CData,itDst->td),"len",T_SHORT);
    nIcLen = CData_FindComp(AS(CData,itDst->td),"len");
  }
  if (idSeq==NULL || CData_IsEmpty(idSeq)) return O_K;

  /* Validate */
  if (nComp<0 || nComp>CData_GetNComps(idSeq))
    return IERROR(_this,FST_BADID,"component",nComp,0);
  if (!dlp_is_numeric_type_code(CData_GetCompType(idSeq,nComp)))
    return IERROR(_this,FST_BADCTYPE,"sequence",nComp,0);

  /* Initialize hidden fields and local variables */
  _this->m_lpNmgT = (FST_ITYPE*)dlp_calloc(CData_GetNRecs(idSeq),sizeof(FST_ITYPE));
  nXS    = CData_GetNRecs(idSeq);
  nOrder = _this->m_nMaxLen;
  if (nOrder<=0) nOrder=nXS;
  DLPASSERT(nIcW>=IC_TD_DATA);
  DLPASSERT(nIcAor>=IC_TD_DATA);

  if (idWeights!=NULL)                                                         /* Allocate interpolation weight buf. */
    _this->m_lpNmgW = (FST_WTYPE*)dlp_calloc(nOrder+1,sizeof(FST_WTYPE));

  lpS = (FST_STYPE*)dlp_calloc(nXS,sizeof(FST_STYPE));                         /* Copy symbol sequence to a buffer...*/
  for (nS=0; nS<nXS; nS++) lpS[nS]=(FST_ITYPE)CData_Dfetch(idSeq,nS,nComp);

  lpTI = CFst_STI_Init(_this,nUnit,FSTI_SORTINI);                              /* Initialize graph iterator          */

  /* Create states */
  CFst_Addstates(itDst,0,nXS+1,FALSE);
  SD_FLG(itDst,nXS)|=0x01;                                                     /* Make last state final              */

  /* Create transitions */
  if (_this->m_bSegment)                                                       /* Segment graph mode ...             */
  {
    IF_NOK(CData_AllocUninitialized(AS(CData,itDst->td),nXS*_this->m_nMaxLen))              /* Allocate all transitions at once   */
      return IERROR(_this,ERR_NOMEM,0,0,0);                                     /* Error will leak memory!            */

    for (nT=0, nIni=0; nIni<nXS; nIni++)
      for (nTer=nIni+1; (nTer<nIni+nOrder+1) && (nTer<=nXS); nTer++,nT++)
      {
        DLPASSERT(nT<nXS*_this->m_nMaxLen);

        Sseq.lpItm = (BYTE*)&lpS[nIni];                                        /* Fill symbol sequence struct ...    */
        Sseq.nOfs  = sizeof(FST_STYPE);
        Sseq.nCnt  = nTer-nIni;

        nW = CFst_Nmg_CalcSeqProb(_this,lpTI,Sseq,nOrder,idWeights,&nSid,&nAO);/* Calculate segment probability      */

        TD_INI(itDst,nT)=nIni;                                                 /* Add transition to dest. FST        */
        TD_TER(itDst,nT)=nTer;
        *(FST_WTYPE*)CData_XAddr(AS(CData,itDst->td),nT,nIcW  ) = nW;          /* Store sequence probability         */
        *(INT16*    )CData_XAddr(AS(CData,itDst->td),nT,nIcAor) = nAO;         /* Store max. act. used n-gram order  */
        *(INT16*    )CData_XAddr(AS(CData,itDst->td),nT,nIcLen) = (INT16)(nTer-nIni);/* Store segment length         */
        *(FST_STYPE*)CData_XAddr(AS(CData,itDst->td),nT,nIcTis) = (FST_STYPE)nSid;/* Store segment idx. as input sym.*/
        if (nSid>=0)                                                           /* Store segment reference counter    */
          *(INT32*)CData_XAddr(AS(CData,itDst->td),nT,nIcRcDst)
            = (INT32)CData_Dfetch(AS(CData,_this->td),nSid,nIcRcNmg);

        IFCHECKEX(1)
        {
          printf("\n   %-3ld -> %-3ld: -log(P(",nIni,nTer);
          for (i=0; i<Sseq.nCnt; i++)
            printf("%s%ld",i>0?",":"",(long)*(FST_STYPE*)(Sseq.lpItm+i*Sseq.nOfs));
          printf(")) = %g",(double)nW);
        }
      }

    CData_SetNRecs(AS(CData,itDst->td),nT);                                     /* Finish destination unit ...       */
    UD_XT(itDst,0)=nT;
  }
  else                                                                          /* Sequence mode ...                 */
  {
    IF_NOK(CData_AllocUninitialized(AS(CData,itDst->td),nXS))                                /* Allocate all transitions at once  */
      return IERROR(_this,ERR_NOMEM,0,0,0);                                     /* Error will leak memory!           */

    for (nS=0; nS<nXS; nS++)
    {
      Sseq.lpItm = (BYTE*)&lpS[ (nS-nOrder+1<0) ? 0 : nS-nOrder+1 ];            /* Fill symbol sequence struct ...   */
      Sseq.nOfs  = sizeof(FST_STYPE);
      Sseq.nCnt  = (nOrder>nS+1) ? nS+1 : nOrder;

      CFst_Nmg_GetMgiWeights(_this,idWeights,_this->m_lpNmgW,(INT16)Sseq.nCnt+1);/*Copy and scale interpol. weights  */
      nW = CFst_Nmg_CalcCondProb(_this,lpTI,Sseq,_this->m_lpNmgW,&nAO);         /* Calculate conditional probability */

      TD_INI(itDst,nS)=nS;                                                      /* Add transition to dest. FST       */
      TD_TER(itDst,nS)=nS+1;
      *(FST_STYPE*)CData_XAddr(AS(CData,itDst->td),nS,nIcTis) = lpS[nS];        /* Store input symbol                */
      *(FST_WTYPE*)CData_XAddr(AS(CData,itDst->td),nS,nIcW  ) = nW;             /* Store symbol probability          */
      *(INT16*    )CData_XAddr(AS(CData,itDst->td),nS,nIcAor) = nAO;            /* Store actually used n-gram length */

      IFCHECKEX(1)
      {
        printf("\n   %-3ld: s=%-3ld ",nS,lpS[nS]);
        printf("P(%ld",(long)*(FST_STYPE*)(Sseq.lpItm+(Sseq.nCnt-1)*Sseq.nOfs));
        if (Sseq.nCnt>1) printf("|");
        for (i=0; i<Sseq.nCnt-1; i++)
          printf("%s%ld",i>0?",":"",(long)*(FST_STYPE*)(Sseq.lpItm+i*Sseq.nOfs));
        printf(") = %g",(double)nW);
      }
    }

    CData_SetNRecs(AS(CData,itDst->td),nS);                                     /* Finish destination unit ...       */
    UD_XT(itDst,0)=nS;
  }

  /* Clean up */
  CFst_STI_Done(lpTI);
  dlp_free(lpS);
  dlp_free(_this->m_lpNmgW);
  dlp_free(_this->m_lpNmgT);
  _this->m_lpNmgW = NULL;
  _this->m_lpNmgT = NULL;

  IFCHECK
  {
    printf("\n\n CFst_Multigram done.");
    printf("\n "); dlp_fprint_x_line(stdout,'-',99); printf("\n ");
  }
  return O_K;
}

/*
 * Manual page in fst_man.def
 */
INT16 CGEN_PUBLIC CFst_AnalyzeMultigram(CFst* _this, CData* idCtrs, INT32 nUnit)
{
  FST_TID_TYPE* lpTI    = NULL;                                                /* Transition iterator data struct    */
  BYTE*         lpT     = NULL;                                                /* Pointer to current transition      */
  INT32          nL      = 0;                                                   /* Current layer (tree depth)         */
  FST_ITYPE     nS      = 0;                                                   /* Current state index                */
  FST_ITYPE     nFS     = -1;                                                  /* First state of nUnit               */
  FST_ITYPE     nXS     = -1;                                                  /* Number of states in nUnit          */
  FST_ITYPE     nT      = 0;                                                   /* Current transition index           */
  FST_ITYPE     nXTs    = 0;                                                   /* Transition counter at current state*/
  INT32          nRc     = 0;                                                   /* Current trans. reference counter   */
  INT32          nIcRc   = -1;                                                  /* Reference counter component        */
  INT32          nIcW    = -1;                                                  /* Weight component                   */
  INT32          nBw     = 0;                                                   /* Current beamwidth                  */
  INT32          nNgmCtr = 0;                                                   /* n-gram counter                     */
  INT32          nObsCtr = 0;                                                   /* Observation counter                */
  FST_ITYPE     nErrItm = -1;                                                  /* State or trans. index causing error*/
  INT16         nErr    = O_K;                                                 /* Last error code                    */
  INT32          nType   = 0;                                                   /* Automaton properties               */
  FST_ITYPE*    lpLBrd  = NULL;                                                /* State presence read layer buffer   */
  FST_ITYPE*    lpLBwr  = NULL;                                                /* State presence write layer buffer  */
  FST_ITYPE*    lpGhost = NULL;                                                /* Auxilary: layer buffer swapping    */

  /* Check n-multigram tree */
  CHECK_THIS_RV(NOT_EXEC);
  CFst_Check(_this);
  if (nUnit<0 || nUnit>=UD_XXU(_this)) return IERROR(_this,FST_BADID,"unit",nUnit,0);
  nType = CFst_Analyze(_this,nUnit,0xFFFF);
  if (!(nType & FST_ACCEPTOR)) return IERROR(_this,FST_BADTYPE,"an acceptor"       ,0,0);
  if (!(nType & FST_FWDCONN )) return IERROR(_this,FST_BADTYPE,"forward connected" ,0,0);
  if (!(nType & FST_BKWCONN )) return IERROR(_this,FST_BADTYPE,"backward connected",0,0);
  if (!(nType & FST_FWDTREE )) return IERROR(_this,FST_BADTYPE,"a forward tree"    ,0,0);
  if ( (nType & FST_LOOPS   )) return IERROR(_this,FST_BADTYPE,"acyclic"           ,0,0);

  /* Check reference counters and weigths */
  nIcRc = CData_FindComp(AS(CData,_this->td),NC_TD_RC);
  if (nIcRc<0) return IERROR(_this,FST_MISS,"reference counter component",NC_TD_RC,"transition table");

  nIcW = -1;
  if (CFst_Wsr_GetType(_this,&nIcW)!=FST_WSR_PROB)
  {
    if (nIcW>=IC_TD_DATA) CData_DeleteComps(AS(CData,_this->td),nIcW,1);
    CFst_Probs(_this,nUnit);
  }

  /* Initialize destination */
  if (idCtrs!=NULL)
  {
    CData_Reset(BASEINST(idCtrs),TRUE);
    CData_AddComp(idCtrs,"ngm" ,T_LONG  );
    CData_AddComp(idCtrs,"obs" ,T_LONG  );
    CData_AddComp(idCtrs,"Xngm",T_DOUBLE);
    CData_AllocateUninitialized(idCtrs,_this->m_nMaxLen+1);
  }

  /* Initialize search -- NO RETURNS BEYOND THIS POINT -- */
  nFS    = UD_FS(_this,nUnit);
  nXS    = UD_XS(_this,nUnit);
  lpTI   = CFst_STI_Init(_this,nUnit,FSTI_SORTINI);
  lpLBrd = (FST_ITYPE*)dlp_calloc(nXS,sizeof(FST_ITYPE));
  lpLBwr = (FST_ITYPE*)dlp_calloc(nXS,sizeof(FST_ITYPE));
  if (!lpLBrd) return IERROR(_this,ERR_NOMEM,0,0,0);
  if (!lpLBwr) return IERROR(_this,ERR_NOMEM,0,0,0);

  /* Traverse tree */
  lpLBrd[0] = 1;
  for (nL=1,nBw=1; nL<=_this->m_nMaxLen+1 && nBw>0 && nErr==O_K; nL++)         /* Descent m_nMaxLen times into tree  */
  {
    for (nS=0,nBw=0,nNgmCtr=0,nObsCtr=0; nS<nXS && nErr==O_K; nS++)            /* For all states...                  */
      if (lpLBrd[nS])                                                          /* ...that are nL steps away from root*/
      {
        lpT  = NULL;
        nXTs = 0;
        while ((lpT=CFst_STI_TfromS(lpTI,nS,lpT))!=NULL && nErr==O_K)          /* Enumerate transitions leaving nS   */
        {
          /* Check multigram tree integrety */
          if (lpLBwr[*CFst_STI_TTer(lpTI,lpT)]!=0)                             /* ERROR: Not a forward tree          */
          {
            nErrItm = nS;
            nErr    = FST_PATHNOTUNIQUE;
            IFCHECKEX(1) printf("\n     Not a forward tree at state %ld",(long)nS);
          }

          /* Keep traversal goin' */
          lpLBwr[*CFst_STI_TTer(lpTI,lpT)]++;                                  /* State occurs on next descent level */
          nBw++;                                                               /* Count transitions on descent level */
          nXTs++;                                                              /* Count transitions on this state    */

          /* Count n-grams */
          nT  = CFst_STI_GetTransId(lpTI,lpT);                                 /* Get global transition index of lpT */
          nRc = (INT32)CData_Dfetch(AS(CData,_this->td),nT,nIcRc);              /* Get transition reference counter   */
          if (nRc>0)
          {
            nNgmCtr++;                                                         /* Count different nL-grams           */
            nObsCtr+=nRc;                                                      /* Count number of observations       */
          }
        }

        /* Check multigram tree integrety */
        if (nXTs==0 && (SD_FLG(_this,nS+nFS)&0x01)!=0x01)                      /* ERROR: Broken path                 */
        {
          nErrItm = nS;
          nErr    = FST_PATHBROKEN;
          IFCHECKEX(1) printf("\n     Broken path at state %ld",(long)nS);
        }
      }

    /* Auto-detect number of elementary smybols */
    if (nL==1 && _this->m_nSymbols<=0) _this->m_nSymbols = nNgmCtr;

    /* Store counters */
    if (idCtrs!=NULL && nL<=_this->m_nMaxLen)
    {
      CData_Dstore(idCtrs,nNgmCtr                                  ,nL,0);     /* Store number of different nL-grams */
      CData_Dstore(idCtrs,nObsCtr                                  ,nL,1);     /* Store number of oberservations     */
      CData_Dstore(idCtrs,CData_Dfetch(idCtrs,0,0)+nNgmCtr         , 0,0);     /* Store total number of diff. n.grams*/
      CData_Dstore(idCtrs,CData_Dfetch(idCtrs,0,1)+nObsCtr         , 0,1);     /* Store total number of observations */
    }

    /* Swap layer buffers */
    lpGhost = lpLBrd; lpLBrd = lpLBwr; lpLBwr = lpGhost;
    dlp_memset(lpLBwr,0,nXS*sizeof(FST_ITYPE));
  }

  /* Store max. number of diff. nL-grams*/
  if (idCtrs!=NULL)
    for (nL=1; nL<=_this->m_nMaxLen; nL++)
      CData_Dstore(idCtrs,dlm_pow((FLOAT64)_this->m_nSymbols,(FLOAT64)nL),nL,2);

  /* Check multigram tree integrety */
  if (nBw>0)                                                                   /* ERROR: Paths INT32er than m_nMaxLen */
  {
    nErrItm = nUnit;
    nErr    = FST_PATHTOOLONG;
    IFCHECKEX(1) printf("\n     Tree too deep");
  }

  /* Clean up and auwieh: */
  CFst_STI_Done(lpTI);
  dlp_free(lpLBwr);
  dlp_free(lpLBrd);

  switch (nErr)
  {
  case FST_PATHNOTUNIQUE: return IERROR(_this,FST_PATHNOTUNIQUE,nUnit,nErrItm,"backward");
  case FST_PATHBROKEN   : return IERROR(_this,FST_PATHBROKEN   ,nUnit,nErrItm,"forward" );
  case FST_PATHTOOLONG  : return IERROR(_this,FST_PATHTOOLONG  ,"traversing",nUnit,0    );
  case O_K              : return O_K;
  default               : DLPASSERT(FMSG("Unknow error"));
  }
  return NOT_EXEC;
}

/**
 * Discounts transitions by nDiscount and removes open paths.
 * Uses flag <code>SD_FLG_USER1</code> to mark leaves.
 * @param _this     This automaton instance
 * @param nDiscount Value for discounting
 * @param nUnit     Unit to be processed (-1 for all)
 * @return O_K if successfull, a (negative) error code otherwise
 */
INT16 CGEN_PROTECTED CFst_Discount(CFst* _this, INT32 nDiscount, INT32 nUnit)
{
  INT32      i     = 0;
  INT32      nIte  = 0;
  INT32      nU    = 0;
  INT32      nIcRc = 0;
  INT32      nRc   = 0;
  INT32      nRm   = 0;
  INT32      nRecln = 0;
  CData *lpAux2   = NULL;
  FST_ITYPE nS    = 0;
  FST_ITYPE nT    = 0;
  CData *lpAux    = NULL;
  BYTE *lpRec     = NULL;
  FST_TID_TYPE* lpTI = NULL;

  /* Protocol */
  IFCHECK
  {
    printf("\n"); dlp_fprint_x_line(stdout,'-',dlp_maxprintcols());
    printf("\n CFst_Discount(%s,%ld,%ld)",BASEINST(_this)->m_lpInstanceName,(long)nDiscount,(long)nUnit);
    printf("\n"); dlp_fprint_x_line(stdout,'-',dlp_maxprintcols()); printf("\n");
  }

  /* Validation*/
  CHECK_THIS_RV(NOT_EXEC);
  CFst_Check(_this);
  if (nDiscount<0)
    return IERROR(_this,ERR_GENERIC,"A negative value for parameter nDiscount is bogus.",0,0);
  if (nUnit>(UD_XXU(_this)-1))
    return IERROR(_this,ERR_GENERIC,"Unit doesn't exist.",0,0);
  if(CFst_Wsr_GetType(_this,NULL)!=0 && CFst_Wsr_GetType(_this,NULL)!=FST_WSR_PROB)
    return IERROR(_this,ERR_GENERIC,"Discount works only for automatons having a probability semiring or no weights at all.",0,0);
  if (CData_FindComp(AS(CData,_this->td),NC_TD_RC)<0)
    return IERROR(_this,FST_MISS,"transition reference counter component",NC_TD_RC,"transition table");

  /* Initialization */
  ICREATEEX(CData,lpAux,"~aux",NULL);
  ICREATEEX(CData,lpAux2,"~aux2",NULL);
  nIcRc = CData_FindComp(AS(CData,_this->td),NC_TD_RC);
  CFst_ResetStateFlag(_this,nUnit,SD_FLG_USER1);

  /* Loop over units */
  for (nU=nUnit<0?0:nUnit; nU<UD_XXU(_this); nU++)
  {
    /* Iterate until all transitions having zero references counters and leading to leaves are removed */
    for (nIte=0;UD_XT(_this,nU)>0;nIte++)
    {
      lpTI = CFst_STI_Init(_this,nU,FSTI_SORTINI);       /* Initialize graph iterator          */

      /* Mark leaves */
      for (nS=lpTI->nFS; nS<lpTI->nFS+lpTI->nXS; nS++)
        if (SD_FLG(_this,nS)&SD_FLG_FINAL)
          if (CFst_STI_TfromS(lpTI,nS-lpTI->nFS,NULL) == NULL)
            SD_FLG(_this,nS)|=SD_FLG_USER1;

      nRm = 0;  /* Reset counter of transitions to be deleted */

      IFCHECK printf("\n\n Iteration %ld in unit %ld\n",(long)nIte,(long)nU);
      IFCHECK dlp_fprint_x_line(stdout,'-',dlp_maxprintcols());
      IFCHECK printf("\n");

      /* Discount transitions (subtract nDiscount from transition counter of every transition)          */
      /* and mark transitions to leaves that have a reference counter less or equal zero after discount */
      for (nT=lpTI->nFT; nT<lpTI->nFT+UD_XT(_this,nU); nT++)
      {
        nRc = (INT32)CData_Dfetch(AS(CData,_this->td),nT,nIcRc);
        if (nIte == 0) nRc -= nDiscount;

        if ((nRc <= 0) && (SD_FLG(_this,TD_TER(_this,nT)) & SD_FLG_USER1))
        {
          CData_Dstore(AS(CData,_this->td),-1,nT,nIcRc);
          nRm++;
        }
        else if (nIte == 0) CData_Dstore(AS(CData,_this->td),nRc<0?0:nRc,nT,nIcRc);
      }

      /* No more transitions to delete? */
      if (nRm == 0) break;

      IFCHECK printf("\n Unit %ld: %ld out of %ld transitions to be removed.",(long)nU,(long)nRm,(long)UD_XT(_this,nU));

      /* Remove marked transitions */
      CData_Scopy(lpAux,AS(CData,_this->td));
      CData_AllocateUninitialized(lpAux,UD_XT(_this,nU)-nRm);
      lpRec = CData_XAddr(lpAux,0,0);
      nRecln = CData_GetRecLen(AS(CData,_this->td));

      for(nT=lpTI->nFT, i=0; nT<lpTI->nFT+UD_XT(_this,nU); nT++)
      {
        if((INT32)CData_Dfetch(AS(CData,_this->td),nT,nIcRc)>=0)
        {
          /* Copy transition */
          dlp_memmove(lpRec,CData_XAddr(AS(CData,_this->td),nT,0),nRecln);
          lpRec+=nRecln;
        }
      }

      /* Update transition table */
      CData_SelectRecs(lpAux2,AS(CData,_this->td),0,lpTI->nFT);
      CData_Cat(lpAux2,lpAux);
      CData_SelectRecs(lpAux,AS(CData,_this->td),lpTI->nFT+UD_XT(_this,nU),UD_XXT(_this)-(lpTI->nFT+UD_XT(_this,nU)));
      CData_Cat(lpAux2,lpAux);
      CData_Copy(BASEINST(_this->td),BASEINST(lpAux2));

      /* Adjust unit descriptions */
      UD_XT(_this,nU)-=nRm;
      for (i=nU+1; i<UD_XXU(_this); i++) UD_FT(_this,i)-=nRm;

      /* Reset user state flag */
      CFst_ResetStateFlag(_this,nUnit,SD_FLG_USER1);

      /* Remove open states */
      IFCHECK printf("\n Unit %ld: Remove open states",(long)nU);
      CFst_TrimStates(_this,nU);
    }

    /* Check unit */
    if (UD_XT(_this,nU)==0) IERROR(_this,FST_UNITEMPTY,nU,0,0);

    /* Stop in single unit mode */
    if (nUnit>=0) break;
  }

  /* Recalc transition probabilities if exist, this behavior is defined and must not be changed! */
  if (CFst_Wsr_GetType(_this,NULL)==FST_WSR_PROB) CFst_Probs(_this,-1);

  /* Cleanup */
  if(lpAux!=NULL)  IDESTROY(lpAux);
  if(lpAux2!=NULL) IDESTROY(lpAux2);

  CFst_Check(_this); /* TODO: Remove after debugging */

  /* Protocol */
  IFCHECK
  {
    printf("\n\n CFst_Discount done.\n");
    dlp_fprint_x_line(stdout,'-',dlp_maxprintcols());
    printf("\n");
  }

  return O_K;
}

/**
 * Internal use. Called by {@link MarkConnected CFst_MarkConnected}.
 */
void CGEN_PRIVATE CFst_Mc_Fwd(CFst* _this, FST_TID_TYPE* lpTI, FST_ITYPE nS)
{
  BYTE* lpT = NULL;

  if (SD_FLG(_this,nS+lpTI->nFS)&SD_FLG_USER3) return;
  SD_FLG(_this,nS+lpTI->nFS)|=SD_FLG_USER3;

  while ((lpT=CFst_STI_TfromS(lpTI,nS,lpT)))
    CFst_Mc_Fwd(_this,lpTI,*CFst_STI_TTer(lpTI,lpT));
}

/**
 * Internal use. Called by {@link MarkConnected CFst_MarkConnected}.
 */
void CGEN_PRIVATE CFst_Mc_Bkw(CFst* _this, FST_TID_TYPE* lpTI, FST_ITYPE nS)
{
  BYTE* lpT = NULL;

  if (SD_FLG(_this,nS+lpTI->nFS)&SD_FLG_USER4) return;
  SD_FLG(_this,nS+lpTI->nFS)|=SD_FLG_USER4;

  while ((lpT=CFst_STI_TtoS(lpTI,nS,lpT)))
    CFst_Mc_Bkw(_this,lpTI,*CFst_STI_TIni(lpTI,lpT));
}

/**
 * Marks connection of states. Sets <code>SD_FLG_USER3</code> for all states
 * which can be reached traversing the automaton forward from the start state
 * and <code>SD_FLG_USER4</code> for all states which can be reached traversing
 * the automaton backward from any of the final states. There are no checks
 * performed.
 * @param _this This automaton instance
 * @param nUnit Unit to be processed (-1 for all)
 * @return O_K if successfull, a (negative) error code otherwise
 */
INT16 CGEN_PROTECTED CFst_MarkConnected(CFst* _this, INT32 nUnit)
{
  INT32          nU   = 0;
  FST_ITYPE     nS   = 0;
  FST_TID_TYPE* lpTI = NULL;

  /* Clear user flags at all states */
  for (nS=0; nS<UD_XXS(_this); nS++)
    SD_FLG(_this,nS)&=~(SD_FLG_USER3|SD_FLG_USER4);

  /* Loop over units */
  for (nU=(nUnit<0)?0:nUnit; nU<UD_XXU(_this); nU++)
  {
    /* Forward pass */
    lpTI = CFst_STI_Init(_this,nU,FSTI_PTR);
    #if 0
    CFst_Mc_Fwd(_this,lpTI,0);

    /* Backward pass */
    for (nS=0; nS<UD_XS(_this,nU); nS++)
      if (SD_FLG(_this,nS+UD_FS(_this,nU))&SD_FLG_FINAL)
        CFst_Mc_Bkw(_this,lpTI,nS);
    #else
    FST_ITYPE *queue=(FST_ITYPE*)dlp_malloc((size_t)UD_XS(_this,nUnit)*sizeof(FST_ITYPE));
    FST_ITYPE qused=0;
    BYTE* lpT = NULL;

    /* Forward pass */
    SD_FLG(_this,0+lpTI->nFS)|=SD_FLG_USER3;
    queue[qused++]=0;
    while(qused){
      nS=queue[--qused];
      while((lpT=CFst_STI_TfromS(lpTI,nS,lpT))){
        FST_ITYPE nX=*CFst_STI_TTer(lpTI,lpT);
        if(SD_FLG(_this,nX+lpTI->nFS)&SD_FLG_USER3) continue;
        SD_FLG(_this,nX+lpTI->nFS)|=SD_FLG_USER3;
        queue[qused++]=nX;
      }
    }

    /* Backward pass */
    for(nS=0;nS<UD_XS(_this,nUnit);nS++) if(SD_FLG(_this,nS+lpTI->nFS)&SD_FLG_FINAL){
      SD_FLG(_this,nS+lpTI->nFS)|=SD_FLG_USER4;
      queue[qused++]=nS;
    }
    while(qused){
      nS=queue[--qused];
      while((lpT=CFst_STI_TtoS(lpTI,nS,lpT))){
        FST_ITYPE nX=*CFst_STI_TIni(lpTI,lpT);
        if(SD_FLG(_this,nX+lpTI->nFS)&SD_FLG_USER4) continue;
        SD_FLG(_this,nX+lpTI->nFS)|=SD_FLG_USER4;
        queue[qused++]=nX;
      }
    }

    dlp_free(queue);
    #endif
    CFst_STI_Done(lpTI);

    /* Break in single unit mode */
    if (nUnit>=0) break;
  }

  return O_K;
}

/**
 * Removes all unconnected states and transitions. Unconnected states and
 * transitions are not part of a consecutive path from the start state to a
 * final state)
 *
 * @param  _this   This CFst instance
 * @param  nU      Unit to process
 */
INT16 CGEN_PROTECTED CFst_TrimStates(CFst* _this, INT32 nU)
{
  INT32      i           = 0;
  INT32      nRm         = 0;
  INT32      nRecln      = 0;
  INT32      nIte        = 0;
  INT32      nIni        = 0;
  INT32      nTer        = 0;
  FST_ITYPE nS          = 0;
  FST_ITYPE nT          = 0;
  FST_ITYPE nFT         = 0;
  FST_ITYPE nXT         = 0;
  CData *lpSMapNewToOld = NULL;
  CData *lpSMapOldToNew = NULL;
  CData *lpAux          = NULL;
  BYTE *lpRec           = NULL;

  /* Protocol */
  IFCHECK
  {
    printf("\n"); dlp_fprint_x_line(stdout,'-',dlp_maxprintcols());
    printf("\n CFst_TrimStates(%s,%ld)",BASEINST(_this)->m_lpInstanceName,(long)nU);
    printf("\n"); dlp_fprint_x_line(stdout,'-',dlp_maxprintcols()); printf("\n");
  }

  /* Verify */
  DLPASSERT((nU>=0)&&(nU<=UD_XXU(_this)));

  /* Initialize */
  ICREATEEX(CData,lpAux,"~lpAux",NULL);
  ICREATEEX(CData,lpSMapNewToOld,"~lpSMapNewToOld",NULL);
  ICREATEEX(CData,lpSMapOldToNew,"~lpSMapOldToNew",NULL);
  CData_Reset(BASEINST(lpSMapNewToOld),FALSE);
  CData_Reset(BASEINST(lpSMapOldToNew),FALSE);
  CData_AddComp(lpSMapNewToOld,"idx",T_LONG);
  CData_AddComp(lpSMapOldToNew,"idx",T_LONG);
  CData_AllocUninitialized(lpSMapNewToOld,UD_XS(_this,nU));
  CData_AllocateUninitialized(lpSMapOldToNew,UD_XS(_this,nU));
  CData_Fill(lpSMapOldToNew,CMPLX(-1.0),CMPLX(0.0));

  /* Mark connected states */
  CFst_MarkConnected(_this, nU);

  /* Build maps */
  for(nS=0, i=0; nS<UD_XS(_this,nU); nS++)
  {
    if((SD_FLG(_this,nS+UD_FS(_this,nU))&SD_FLG_USER3 &&
       SD_FLG(_this,nS+UD_FS(_this,nU))&SD_FLG_USER4) ||
       nS==0)
    {
      CData_IncNRecs(lpSMapNewToOld,1);
      CData_Dstore(lpSMapNewToOld,nS,i,0);
      CData_Dstore(lpSMapOldToNew,i,nS,0);
      i++;
    }
  }

  /* Count open states to be removed */
  nRm = UD_XS(_this,nU)-CData_GetNRecs(lpSMapNewToOld);

  if(nRm>0)
  {
    IFCHECK printf("\n Unit %ld: %ld out of %ld open states scheduled for removal in iteration %ld.",(long)nU,(long)nRm,(long)UD_XS(_this,nU),(long)++nIte);

    nFT = UD_FT(_this,nU);
    nXT = UD_XT(_this,nU);

    if(nRm>UD_XS(_this,nU)*0.3){
      /* Keep only transitions that are valid */
      FST_ITYPE nTdst = nFT;

      for (nT=nFT; nT<nFT+nXT; nT++)
      {
        /* Map initial and final state */
        nIni = (INT32)CData_Dfetch(lpSMapOldToNew,TD_INI(_this,nT),0);
        nTer = (INT32)CData_Dfetch(lpSMapOldToNew,TD_TER(_this,nT),0);
        /* Copy transition to keeping ones if it is valid */
        if(nIni>=0&&nTer>=0){
          if(nT!=nTdst)
          {
            ISETOPTION(AS(CData,_this->td),"/rec");
            CData_Xstore(AS(CData,_this->td),AS(CData,_this->td),nT,1,nTdst);
            IRESETOPTIONS(AS(CData,_this->td));
          }
          nTdst++;
        }
      }
      /* Reduce transition table length to number of valid transitions */
      CData_DeleteRecs(AS(CData,_this->td),nTdst,nXT-(nTdst-nFT));
      UD_XT(_this,nU)-=nXT-(nTdst-nFT);
      for (i=nU+1; i<UD_XXU(_this); i++) UD_FT(_this,i)-=nXT-(nTdst-nFT);

    }else{
      /* Remove transitions that will become invalid */
      for (nT=nFT; nT<nFT+nXT; )
      {
        /* Map initial and final state */
        nIni = (INT32)CData_Dfetch(lpSMapOldToNew,TD_INI(_this,nT),0);
        nTer = (INT32)CData_Dfetch(lpSMapOldToNew,TD_TER(_this,nT),0);

        /* Remove transitions having invalid initial or final state */
        if(nIni<0||nTer<0)
        {
          IFCHECK printf("\n Unit %ld: Removing invalid transition %ld (%ld->%ld)",(long)nU,(long)nT,(long)nIni,(long)nTer);

          /* Remove from transition table */
          CData_DeleteRecs(AS(CData,_this->td),nT,1);

          /* Adjust unit descriptions */
          UD_XT(_this,nU)--;
          for (i=nU+1; i<UD_XXU(_this); i++) UD_FT(_this,i)--;
          nXT--;
        }
        else nT++;
      }
    }
    nFT = UD_FT(_this,nU);
    nXT = UD_XT(_this,nU);

    /* Remove open states */
    IFCHECK printf("\n Unit %ld: Removing %ld open states",(long)nU,(long)nRm);
    CData_Scopy(lpAux,AS(CData,_this->sd));
    CData_AllocateUninitialized(lpAux,CData_GetNRecs(lpSMapNewToOld));
    lpRec  = CData_XAddr(lpAux,0,0);
    nRecln = CData_GetRecLen(AS(CData,_this->sd));

    for(i=0; i<CData_GetNRecs(lpSMapNewToOld); i++)
    {
      nS = (INT32)CData_Dfetch(lpSMapNewToOld,i,0);

      /* State survived -> copy state description */
      dlp_memmove(lpRec,CData_XAddr(AS(CData,_this->sd),nS+UD_FS(_this,nU),0),nRecln);
      lpRec+=nRecln;
    }


    /* Update state table */
    CData_SelectRecs(lpSMapNewToOld,AS(CData,_this->sd),0,UD_FS(_this,nU));
    CData_Cat(lpSMapNewToOld,lpAux);
    CData_SelectRecs(lpAux,AS(CData,_this->sd),UD_FS(_this,nU)+UD_XS(_this,nU),UD_XXS(_this)-(UD_FS(_this,nU)+UD_XS(_this,nU)));
    CData_Cat(lpSMapNewToOld,lpAux);
    CData_Copy(_this->sd,BASEINST(lpSMapNewToOld));

    /* Adjust transition table */
    for(nT=nFT; nT<nFT+nXT; nT++)
    {
      /* Map initial and final state */
      TD_INI(_this,nT)=(INT32)CData_Dfetch(lpSMapOldToNew,TD_INI(_this,nT),0);
      TD_TER(_this,nT)=(INT32)CData_Dfetch(lpSMapOldToNew,TD_TER(_this,nT),0);
    }

    /* Adjust unit descriptions */
    UD_XS(_this,nU)-=nRm;
    for (i=nU+1; i<UD_XXU(_this); i++) UD_FS(_this,i)-=nRm;
  }
  else IFCHECK printf("\n Nothing to do.");

  /* Clear user flags at all states */
  for (nS=0; nS<UD_XXS(_this); nS++)
    SD_FLG(_this,nS)&=~(SD_FLG_USER3|SD_FLG_USER4);

  /* Cleanup */
  if(lpAux         !=NULL) IDESTROY(lpAux);
  if(lpSMapNewToOld!=NULL) IDESTROY(lpSMapNewToOld);
  if(lpSMapOldToNew!=NULL) IDESTROY(lpSMapOldToNew);

  CFst_Check(_this); /* TODO: Remove after debugging */

  /* Protocol */
  IFCHECK
  {
    printf("\n\n CFst_TrimStates done.\n");
    dlp_fprint_x_line(stdout,'-',dlp_maxprintcols());
    printf("\n");
  }

  return O_K;
}

/* EOF */
