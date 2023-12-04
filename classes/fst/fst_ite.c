/* dLabPro class CFst (fst)
 * - Iterators
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
 * <p>Initializes a new graph iterator.</p>
 * <h3 style="color:red">Important Note</h3>
 * <p>Adding or removing states or transitions to/from the unit being iterated
 * invalidates the iterator. In these cases you <em>must</em> call 
 * {@link STI_UnitChanged CFst_STI_UnitChanged} to update the iterator.</p>
 *
 * @param iFst  Graph to search in
 * @param nUnit Unit to search in
 * @param nMode Operation mode, a combination of the following constants:
 *              <table cellpadding="3">
 *                <tr><th>C Symbol</th><th>Numeric value</th><th>Description</th></tr>
 *                <tr><td><code>FSTI_SORTINI</code></td><td><code>0x0001</code></td>
 *                    <td>Sort transitions by initial state (cannot be used together with <code>FSTI_SORTTER</code>)</td></tr>
 *                <tr><td><code>FSTI_SORTTER</code></td><td><code>0x0002</code></td>
 *                    <td>Sort transitions by terminal state (cannot be used together with <code>FSTI_SORTINI</code>)</td></tr>
 *                <tr><td><code>FSTI_SLOPPY</code></td><td><code>0x0004</code></td>
 *                    <td>Experimental: partially sorted transition list (only valid in combination with
 *                        <code>FSTI_SORTINI</code> or <code>FSTI_SORTTER</code>).</td></tr>
 *                <tr><td><code>FSTI_PTR</code></td><td><code>0x0004</code></td>
 *                    <td>Experimental: Forward and backward pointers for each state are stored.
 *                        Transition sorting is avoided. Forward and backward iteration is very fast.
 *                        Every change enforces a recreation of the pointer lists.</td></tr>
 *              </table>
 * @return      A pointer to a graph iteration data structure
 *              or <code>NULL</code> in case of errors.
 * @see STI_Done CFst_STI_Done
 * @see STI_UnitChanged CFst_STI_UnitChanged
 */
FST_TID_TYPE* CGEN_EXPORT CFst_STI_Init(CFst* iFst, INT32 nUnit, INT32 nMode)
{
  FST_TID_TYPE* lpTI = NULL;
  INT32          nC   = -1;
  
  /* Validation */
  if (iFst==NULL                    ) return NULL;
  if (nUnit<0 || nUnit>=UD_XXU(iFst)) return NULL;

  CFst_Check(iFst);

  /* One-time initialization */
  lpTI           = (FST_TID_TYPE*)__dlp_calloc(1,sizeof(FST_TID_TYPE),__FILE__,__LINE__,"CFst_STI_Init","");
  lpTI->iFst     = iFst;
  lpTI->nUnit    = nUnit;
  lpTI->nMode    = nMode;
  lpTI->nOfTIni  = CDlpTable_GetCompOffset(AS(CData,iFst->td)->m_lpTable,IC_TD_INI );
  lpTI->nOfTTer  = CDlpTable_GetCompOffset(AS(CData,iFst->td)->m_lpTable,IC_TD_TER );
  lpTI->nOfTTis  = CDlpTable_GetCompOffset(AS(CData,iFst->td)->m_lpTable,CData_FindComp(AS(CData,iFst->td),NC_TD_TIS));
  lpTI->nOfTTos  = CDlpTable_GetCompOffset(AS(CData,iFst->td)->m_lpTable,CData_FindComp(AS(CData,iFst->td),NC_TD_TOS));
  lpTI->nOfTRc   = CDlpTable_GetCompOffset(AS(CData,iFst->td)->m_lpTable,CData_FindComp(AS(CData,iFst->td),NC_TD_RC ));
  lpTI->nOfTData = CDlpTable_GetCompOffset(AS(CData,iFst->td)->m_lpTable,IC_TD_DATA);
  lpTI->nRls     = CData_GetRecLen(AS(CData,iFst->sd));
  lpTI->nRlt     = CData_GetRecLen(AS(CData,iFst->td));

  if ((nMode&FSTI_SLOPPY)==0)
  {
    dlp_free(iFst->m_lpFts);
    iFst->m_lpFts=NULL;
    iFst->m_nXFts=0;
  }
  CFst_STI_UnitChanged(lpTI,FSTI_CANY);

  /* One-time initialization */
  CFst_Wsr_GetType(iFst,&nC);
  if (nC>=0) lpTI->nOfTW = CDlpTable_GetCompOffset(AS(CData,iFst->td)->m_lpTable,nC);

  return lpTI;
}

/**
 * <p>Updates the iterator after changes of the state or transition table.
 * Updating an existing iterator is faster then destroying it and instanciating
 * a new one.</p>
 * <h3 style="color:red">Important Note</h3>
 * <p>This function <b>must</b> be called after adding or removing states or
 * transitions to/from the unit being iterated!</p>
 *
 * @param lpTI
 *          Iterator data structure (returned by {@link STI_Init CFst_STI_Init})
 * @param nMode
 *          Information about the change. An improper value may cause undefined
 *          behaviour. Only the value <code>FSTI_CANY</code> is always safe. Use
 *          exactly one of the following constants:
 *          <table cellpadding="3">
 *            <tr><th>C Symbol</th><th>Numeric value</th><th>Description</th></tr>
 *            <tr><td><code>FSTI_CADD</code></td><td><code>0x0001</code></td>
 *                <td>Added states and/or transitions</td></tr>
 *            <tr><td><code>FSTI_CANY</code></td><td><code>0x0000</code></td>
 *                <td>Any other changes</td></tr>
 *            </table>
 * @see STI_Init CFst_STI_Init
 */
void CGEN_EXPORT CFst_STI_UnitChanged(FST_TID_TYPE* lpTI, INT16 nMode)
{
  BOOL bRealloc  = TRUE;
  INT32 nXSold    = 0;
  INT32 nFTunsrtd = 0;
  INT32 nXTunsrtd = 0;
  
  /* Update unit metrics */
  nXSold     = lpTI->nXS;
  bRealloc   = (lpTI->lpFT!=CData_XAddr(AS(CData,lpTI->iFst->td),lpTI->nFT,0)); /* Detect transition list realloc.   */
  nFTunsrtd  = (INT32)((lpTI->lpFTunsrtd-lpTI->lpFT)/lpTI->nRlt + lpTI->nFT +1); /* Index of first unsorted transition*/
  nXTunsrtd  = UD_XT(lpTI->iFst,lpTI->nUnit) - nFTunsrtd;                       /* How may unsorted transitions?     */
  lpTI->nFS  = UD_FS(lpTI->iFst,lpTI->nUnit);
  lpTI->nXS  = UD_XS(lpTI->iFst,lpTI->nUnit);
  lpTI->nFT  = UD_FT(lpTI->iFst,lpTI->nUnit);
  lpTI->nXT  = UD_XT(lpTI->iFst,lpTI->nUnit);
  lpTI->lpFT = CData_XAddr(AS(CData,lpTI->iFst->td),lpTI->nFT,0);
  bRealloc  |= (lpTI->iFst->m_nXFts<lpTI->nXS);                                 /* Sort index to small --> resort!   */

  /* (Re-)sort sorted iterators */
  if (lpTI->nMode&(FSTI_SORTINI|FSTI_SORTTER))                                  /* For sorted iterators...           */
  {                                                                             /* >>                                */
    if (nMode==FSTI_CADD && (lpTI->nMode&FSTI_SLOPPY)!=0 && !bRealloc)          /*   Just added in SLOPPY(!) mode    */
    {                                                                           /*   >>                              */
      DLPASSERT(nXSold<=lpTI->nXS);                                             /*     Not just added something!     */
      if (nXTunsrtd>lpTI->iFst->m_nGrany) CFst_STI_Sort(lpTI);                  /*     Too many unsrtd tr. -> resort */
      else                                                                      /*     Acceptable no. unsrtd. trans. */
        for (; nXSold<lpTI->nXS; nXSold++)                                      /*       Loop over added states      */
          lpTI->iFst->m_lpFts[nXSold] = nFTunsrtd;                              /*         All trans. unsorted       */
    }                                                                           /*   <<                              */
    else                                                                        /*   Strict sorting or any change    */
      CFst_STI_Sort(lpTI);                                                      /*     Resort iterator               */
  }                                                                             /* <<                                */
  else                                                                          /* For unsorted iterators...         */
  {                                                                             /* >>                                */
    dlp_free(lpTI->iFst->m_lpFts);                                              /*   Clear trans. sorting index      */
    lpTI->iFst->m_lpFts = NULL;                                                 /*   ...                             */
    lpTI->iFst->m_nXFts = 0;                                                    /*   ...                             */
    lpTI->lpFTunsrtd = NULL;                                                    /*   All transitions unsorted        */
  }                                                                             /* <<                                */

  if(lpTI->lpPTFwd) dlp_free(lpTI->lpPTFwd);
  if(lpTI->lpPTBwd) dlp_free(lpTI->lpPTBwd);
  if(lpTI->lpPTMem) dlp_free(lpTI->lpPTMem);

  if(lpTI->nMode==FSTI_PTR){
    CFst *_this=lpTI->iFst;
    INT32 nT;
    lpTI->lpPTFwd=(FST_TID_PT_TYPE **)dlp_calloc(lpTI->nXS,sizeof(FST_TID_PT_TYPE *));
    lpTI->lpPTBwd=(FST_TID_PT_TYPE **)dlp_calloc(lpTI->nXS,sizeof(FST_TID_PT_TYPE *));
    lpTI->lpPTMem=(FST_TID_PT_TYPE  *)dlp_malloc(lpTI->nXT*sizeof(FST_TID_PT_TYPE));
    for(nT=0;nT<lpTI->nXT;nT++){
      BYTE *lpT=CFst_STI_GetTransPtr(lpTI,nT+lpTI->nFT);
      INT32 nS;
      lpTI->lpPTMem[nT].lpT=lpT;
      nS=*CFst_STI_TIni(lpTI,lpT);
      lpTI->lpPTMem[nT].lpFN=lpTI->lpPTFwd[nS];
      lpTI->lpPTFwd[nS]=lpTI->lpPTMem+nT;
      nS=*CFst_STI_TTer(lpTI,lpT);
      lpTI->lpPTMem[nT].lpBN=lpTI->lpPTBwd[nS];
      lpTI->lpPTBwd[nS]=lpTI->lpPTMem+nT;
    }
  }
}

/**
 * Frees memory associated with a graph iterator.
 *
 * @param lpTI Iterator data structure (returned by {@link STI_Init CFst_STI_Init})
 * @see STI_Init CFst_STI_Init
 */
void CGEN_EXPORT CFst_STI_Done(FST_TID_TYPE* lpTI)
{
  if ((lpTI->nMode&FSTI_SLOPPY)==0)
  {
    dlp_free(lpTI->iFst->m_lpFts);
    lpTI->iFst->m_lpFts = NULL;
    lpTI->iFst->m_nXFts = 0;   
  }
  if(lpTI->lpPTFwd) dlp_free(lpTI->lpPTFwd);
  if(lpTI->lpPTBwd) dlp_free(lpTI->lpPTBwd);
  if(lpTI->lpPTMem) dlp_free(lpTI->lpPTMem);
  dlp_free(lpTI);
}

/**
 * Determines if the transition list of the <code>CFst</code> instance
 * associated with the iterator is sorted and if the transition sorting index
 * is valid.
 * 
 * @param lpTI
 *          Iterator data structure (returned by {@link STI_Init CFst_STI_Init})
 * @param nMode
 *          FSTI_SORTINI or FSTI_SORTTER
 * @param lpFSunsrtd
 *          Pointer to buffer to be filled with unit relative index of the first
 *          unsorted state in case of a partially sorted transition list; may be
 *          <code>NULL</code>
 * @param lpFTunsrtd
 *          Pointer to buffer to be filled with unit relative index of the first
 *          unsorted transition in case of a partially sorted transition list;
 *          may be <code>NULL</code>
 * @return <code>TRUE</code> if the transition list is completely(!) sorted,
 *         <code>FALSE</code> otherwise.
 * @see STI_Sort CFst_STI_Sort
 */
BOOL CGEN_EXPORT CFst_STI_IsSorted
(
  FST_TID_TYPE* lpTI,
  INT32          nMode,
  FST_ITYPE*    lpFSunsrtd,
  FST_ITYPE*    lpFTunsrtd
)
{
  FST_ITYPE  nT      = 0;                                                       /* Current transition                */
  BYTE*      lpAdj   = NULL;                                                    /* Pointer to current adjacent state */
  FST_ITYPE  nAdjSav = 0;                                                       /* Previous adjacent state           */

  DLPASSERT(nMode&(FSTI_SORTINI|FSTI_SORTTER));                                 /* Which do you want to have checked?*/
  if (lpFSunsrtd) *lpFSunsrtd = 0;                                              /* Unsorted from start               */
  if (lpFTunsrtd) *lpFTunsrtd = 0;                                              /* Unsorted from start               */
  if ((nMode&(FSTI_SORTINI|FSTI_SORTTER))==0) return FALSE;                     /* No sorting criterion specified    */
  if (!lpTI->iFst->m_lpFts) return FALSE;                                       /* No sorting index -> not sorted    */

  for                                                                           /* Loop over transition list         */
  (                                                                             /* |                                 */
    nT=lpTI->nFT, nAdjSav=-1,                                                   /* |                                 */
      lpAdj=lpTI->lpFT+(nMode&FSTI_SORTINI?lpTI->nOfTIni:lpTI->nOfTTer);        /* |                                 */
    nT<lpTI->nFT+lpTI->nXT;                                                     /* |                                 */
    nT++, lpAdj+=lpTI->nRlt                                                     /* |                                 */
  )                                                                             /* |                                 */
  {                                                                             /* >>                                */
    if (*(FST_ITYPE*)lpAdj<nAdjSav) return FALSE;                               /*   Not sorted by ini./ter. state   */
    if (*(FST_ITYPE*)lpAdj>nAdjSav)                                             /*   Next adjecent state group       */
      if (lpTI->iFst->m_lpFts[*(FST_ITYPE*)lpAdj]!=nT)                          /*     Sort index does not point here*/
        return FALSE;                                                           /*       Sort index corrupt!         */
    nAdjSav = *(FST_ITYPE*)lpAdj;                                               /*   Remember adjacent state         */
    if (lpFSunsrtd) *lpFSunsrtd = *(FST_ITYPE*)lpAdj+1;                         /*   Sorted until adjacent state     */
    if (lpFTunsrtd) *lpFTunsrtd = nT-lpTI->nFT+1;                               /*   Sorted until current transition */
  }                                                                             /* <<                                */
  return TRUE;                                                                  /* Yeah, that was really ... sorted  */
}

int CFst_STI_Sort_Icmp(const void* lpT1, const void* lpT2)
{
  if (((FST_ITYPE*)lpT1)[IC_TD_INI] > ((FST_ITYPE*)lpT2)[IC_TD_INI]) return  1;
  if (((FST_ITYPE*)lpT1)[IC_TD_INI] < ((FST_ITYPE*)lpT2)[IC_TD_INI]) return -1;
  return 0;
}

int CFst_STI_Sort_Tcmp(const void* lpT1, const void* lpT2)
{
  if (((FST_ITYPE*)lpT1)[IC_TD_TER] > ((FST_ITYPE*)lpT2)[IC_TD_TER]) return  1;
  if (((FST_ITYPE*)lpT1)[IC_TD_TER] < ((FST_ITYPE*)lpT2)[IC_TD_TER]) return -1;
  return 0;
}

/**
 * <p>Sorts the transition list of the associated <code>CFst</code> instance
 * and creates a transition sorting index. Only the transitions of unit 
 * <code>lpTI-&gt;nUnit</code> will be sorted. The transition sorting index will be
 * stored in field <code>m_lpFts</code> of the <code>CFst</code> instance
 * associated with the iterator (<code>lpTI-&gt;iFst</code>).
 * <code>lpTI-&gt;iFst-&gt;m_lpFts[nS]</code> will be filled with the index of the
 * first transition in the transition table {@link td} of
 * <code>lpTI-&gt;iFst</code> whose initial state is <code>nS</code>.</p>
 * <h3 style="color:red">Important Note</h3>
 * <p>This function modifies the transition list of the <code>CFst</code>
 * instance associated with the iterator!</p>
 *
 * @param lpTI Iterator data structure (returned by {@link STI_Init CFst_STI_Init})
 * @see STI_IsSorted CFst_STI_IsSorted
 */
void CGEN_EXPORT CFst_STI_Sort(FST_TID_TYPE* lpTI)
{
  /* Local variables */
  FST_ITYPE  nAatc = 0;                                                         /* Accumulated adjacent trans. count */
  FST_ITYPE  nS    = 0;                                                         /* Current state                     */
  FST_ITYPE  nT    = 0;                                                         /* Current transition                */
  FST_ITYPE  nFSu  = -1;                                                        /* First unsorted state              */
  FST_ITYPE  nFTu  = -1;                                                        /* First unsorted transition         */
  BYTE*      lpAdj = NULL;                                                      /* Ptr. to current adjacent state    */
  BOOL       bMustSort = TRUE;

  /* Validate */                                                                /* --------------------------------- */
  DLPASSERT(lpTI->nMode&(FSTI_SORTINI|FSTI_SORTTER));                           /* Don't fool around with me buddy!  */
  if (CFst_STI_IsSorted(lpTI,lpTI->nMode,&nFSu,&nFTu)) return;                  /* Check for (partial) sorting       */
  bMustSort =  (lpTI->nMode&FSTI_SLOPPY)==0                                     /* Must sort if not in sloppy mode & */
            || lpTI->nXT-nFTu > lpTI->iFst->m_nGrany;                           /* | if too many unsorted transitions*/

  if (BASEINST(lpTI->iFst)->m_nCheck>=2)                                        /* Protocol (verbose level 2)        */
  {                                                                             /* |                                 */
    printf("\n    Sorting unit %ld",(long)lpTI->nUnit);                         /* |                                 */
    if (lpTI->nMode&FSTI_SLOPPY)                                                /* |                                 */
      printf(" (sloppy mode: nFSu=%ld, nFTu=%ld)",(long)nFSu,(long)nFTu);       /* |                                 */
  }                                                                             /* |                                 */

  /* Allocate memory for transition sorting index */                            /* --------------------------------- */
  if (lpTI->iFst->m_nXFts<lpTI->nXS)                                            /* Is the current one adequate?      */
  {                                                                             /* >> No.                            */
    dlp_free(lpTI->iFst->m_lpFts);                                              /*   Free it                         */
    lpTI->iFst->m_lpFts=NULL;                                                   /*   Clear pointer                   */
    lpTI->iFst->m_nXFts=0;                                                      /*   Reset pointer size              */
  }                                                                             /* <<                                */
  if (!lpTI->iFst->m_lpFts)                                                     /* If there's no search index        */
  {                                                                             /* >>                                */
    lpTI->iFst->m_nXFts = lpTI->nXS+lpTI->iFst->m_nGrany;                       /*   Set pointer size                */
    lpTI->iFst->m_lpFts =                                                       /*   Allocate a new one              */
      (FST_ITYPE*)__dlp_calloc(lpTI->iFst->m_nXFts,sizeof(FST_ITYPE),           /*   |                               */
      __FILE__,__LINE__,"CFst",BASEINST(lpTI->iFst)->m_lpInstanceName);         /*   |                               */
    bMustSort = TRUE;                                                           /*   Must sort                       */
  }                                                                             /* <<                                */

  /* Actually have to do all this */                                            /* --------------------------------- */
  if (!bMustSort)                                                               /* Not necessary to sort             */
  {                                                                             /* >>                                */
    lpTI->lpFTunsrtd = lpTI->lpFT+nFTu*lpTI->nRlt;                              /*   Remember first unsorted trans.  */
    for (; nFSu<lpTI->nXS; nFSu++) lpTI->iFst->m_lpFts[nFSu]=nFTu;              /*   Initialize "unsorted" sort index*/
    if (BASEINST(lpTI->iFst)->m_nCheck>=2) printf("--> SKIP SORTING");          /*   Protocol (verbose level 2)      */
    return;                                                                     /*   Return                          */
  }                                                                             /* <<                                */

  /* Sort transitions */                                                        /* --------------------------------- */
  DLPASSERT(IC_TD_INI<2 && IC_TD_TER<2);                                        /* Comparison function will fail!    */
  dlpsort(lpTI->lpFT,lpTI->nXT,lpTI->nRlt,                                      /* Qsort                             */
    lpTI->nMode&FSTI_SORTINI?CFst_STI_Sort_Icmp:CFst_STI_Sort_Tcmp);            /* |                                 */

  /* Build transition search index */                                           /* --------------------------------- */
  for (nS=0; nS<lpTI->nXS; nS++) lpTI->iFst->m_lpFts[nS]=0;                     /* Clear sort index                  */
  lpAdj = (BYTE*)(lpTI->nMode&FSTI_SORTINI ?                                    /* Get initial adjecent state ptr.   */
    CFst_STI_TIni(lpTI,lpTI->lpFT) : CFst_STI_TTer(lpTI,lpTI->lpFT));           /* |                                 */
  for (nT=lpTI->nFT; nT<lpTI->nFT+lpTI->nXT; nT++,lpAdj+=lpTI->nRlt)            /* Loop over transitions             */
    lpTI->iFst->m_lpFts[*(FST_ITYPE*)lpAdj]++;                                  /*   Count adjacent trans. per state */
  for (nAatc=0,nS=0; nS<lpTI->nXS; nS++)                                        /* Loop over states                  */
  {                                                                             /* >>                                */
    INT32 nBuf = nAatc;                                                         /*   Remember accum. trans. index    */
    nAatc += lpTI->iFst->m_lpFts[nS];                                           /*   Accumulate transition index     */
    lpTI->iFst->m_lpFts[nS] = nBuf+lpTI->nFT;                                   /*   Store current accum. trans. ind.*/
  }                                                                             /* <<                                */

  /* Clean up */                                                                /* --------------------------------- */
  lpTI->lpFTunsrtd =                                                            /* Remember first unsorted trans.    */
    CData_XAddr(AS(CData,lpTI->iFst->td),lpTI->nFT+lpTI->nXT,0);                /* |                                 */
  CFst_Check(lpTI->iFst);                                                       /* TODO: Remove after debugging!     */
  DLPASSERT(CFst_STI_IsSorted(lpTI,lpTI->nMode,NULL,NULL));                     /* TODO: Remove after debugging!     */
}

/**
 * Returns the pointer for a given global transiton index.
 *
 * @param lpTI   Iterator data structure (returned by {@link STI_Init CFst_STI_Init})
 * @param nTrans Transition index.
 * @return       The pointer to the transition or <code>NULL</code> if not found.
 */
BYTE* CGEN_EXPORT CFst_STI_GetTransPtr(FST_TID_TYPE* lpTI, INT32 nTrans)
{
  if (nTrans< lpTI->nFT          ) return NULL;
  if (nTrans>=lpTI->nFT+lpTI->nXT) return NULL;
  return CDATA_XADDR(AS(CData,lpTI->iFst->td),nTrans,0);
}

/**
 * Returns the global transition index for a given transition pointer.
 *
 * @param lpTI    Iterator data structure (returned by {@link STI_Init CFst_STI_Init})
 * @param lpTrans Transition pointer.
 * @return        The global transition index or -1 if not found.
 */
FST_ITYPE CGEN_EXPORT CFst_STI_GetTransId(FST_TID_TYPE* lpTI, BYTE* lpTrans)
{
  DLPASSERT(lpTI && lpTI->iFst);
  DLPASSERT(CData_XAddr(AS(CData,lpTI->iFst->td),UD_FT(lpTI->iFst,lpTI->nUnit),0) == lpTI->lpFT);
  DLPASSERT(UD_XT(lpTI->iFst,lpTI->nUnit)  == lpTI->nXT);
  
  if (lpTrans<lpTI->lpFT || lpTrans>lpTI->lpFT+(size_t)lpTI->nRlt*(size_t)(lpTI->nXT-1))
    return -1;
  return lpTI->nFT + (FST_ITYPE)((lpTrans-lpTI->lpFT)/(size_t)lpTI->nRlt);
/*
   if (lpTrans<CData_XAddr(AS(CData,lpTI->iFst->td),0,0)) return -1;
  if (lpTrans>CData_XAddr(AS(CData,lpTI->iFst->td),CData_GetNRecs(AS(CData,lpTI->iFst->td))-1,0)) return -1;
  return (FST_ITYPE)(lpTrans-CData_XAddr(AS(CData,lpTI->iFst->td),0,0))/lpTI->nRlt;
*/
}

/**
 * <p>Returns the unit relative initial state index of transition lpTrans.</p>
 * <p><b>NO VALIDATION OF <code>lpTrans</code> IS PERFORMED!</b></p>
 *
 * @param lpTI    Iterator data structure (returned by {@link STI_Init CFst_STI_Init})
 * @param lpTrans Pointer to transition
 * @return        Pointer to the unit relative initial state index
 */
FST_ITYPE* CGEN_EXPORT CFst_STI_TIni(FST_TID_TYPE* lpTI, BYTE* lpTrans)
{
  return (FST_ITYPE*)(lpTrans+lpTI->nOfTIni);
}

/**
 * <p>Returns the unit relative terminal state index of transition lpTrans.</p>
 * <p><b>NO VALIDATION OF <code>lpTrans</code> IS PERFORMED!</b></p>
 *
 * @param lpTI    Iterator data structure (returned by {@link STI_Init CFst_STI_Init})
 * @param lpTrans Pointer to transition
 * @return        Pointer to the (unit relative) terminial state index
 */
FST_ITYPE* CGEN_EXPORT CFst_STI_TTer(FST_TID_TYPE* lpTI, BYTE* lpTrans)
{
  return (FST_ITYPE*)(lpTrans+lpTI->nOfTTer);
}

/**
 * <p>Returns the transducer input symbol of transition lpTrans.</p>
 * <p><b>NO VALIDATION OF <code>lpTrans</code> IS PERFORMED!</b></p>
 *
 * @param lpTI    Iterator data structure (returned by {@link STI_Init CFst_STI_Init})
 * @param lpTrans Pointer to transition
 * @return        Pointer to the transducer input symbol
 */
FST_STYPE* CGEN_EXPORT CFst_STI_TTis(FST_TID_TYPE* lpTI, BYTE* lpTrans)
{
  DLPASSERT(lpTI->nOfTTis>0);
  return (FST_STYPE*)(lpTrans+lpTI->nOfTTis);
}

/**
 * <p>Returns the transducer output symbol of transition lpTrans.</p>
 * <p><b>NO VALIDATION OF <code>lpTrans</code> IS PERFORMED!</b></p>
 *
 * @param lpTI    Iterator data structure (returned by {@link STI_Init CFst_STI_Init})
 * @param lpTrans Pointer to transition
 * @return        Pointer to the transducer output symbol
 */
FST_STYPE* CGEN_EXPORT CFst_STI_TTos(FST_TID_TYPE* lpTI, BYTE* lpTrans)
{
  DLPASSERT(lpTI->nOfTTos>0);
  return (FST_STYPE*)(lpTrans+lpTI->nOfTTos);
}

/**
 * <p>Returns the reference counter of transition lpTrans.</p>
 * <p><b>NO VALIDATION OF <code>lpTrans</code> IS PERFORMED!</b></p>
 *
 * @param lpTI    Iterator data structure (returned by {@link STI_Init CFst_STI_Init})
 * @param lpTrans Pointer to transition
 * @return        Pointer to the transition reference counter
 */
FST_ITYPE* CGEN_EXPORT CFst_STI_TRc(FST_TID_TYPE* lpTI, BYTE* lpTrans)
{
  DLPASSERT(lpTI->nOfTRc>0);
  return (FST_ITYPE*)(lpTrans+lpTI->nOfTRc);
}

/**
 * <p>Returns the weight of transition lpTrans.</p>
 * <p><b>NO VALIDATION OF <code>lpTrans</code> IS PERFORMED!</b></p>
 *
 * @param lpTI    Iterator data structure (returned by {@link STI_Init CFst_STI_Init})
 * @param lpTrans Pointer to transition
 * @return        Pointer to the transition weight
 */
FST_WTYPE* CGEN_EXPORT CFst_STI_TW(FST_TID_TYPE* lpTI, BYTE* lpTrans)
{
  DLPASSERT(lpTI->nOfTW>0);
  return (FST_WTYPE*)(lpTrans+lpTI->nOfTW);
}

/**
 * <p>Returns a pointer to the next transition with the initial state
 * <code>nState</code>. The search starts from transition <code>lpTrans</code>.
 * If <code>lpTrans</code> is <code>NULL</code> the transition table is
 * searched from the start.</p>
 * <p>This function may be used to enumerate all transitions originating from a
 * given node:</p>
 * <pre class="code">
 *
 *   FST_ITYPE     nMyUnit     = 0;
 *   FST_ITYPE     nMyIniState = 1;
 *   FST_TID_TYPE* iMySearch   = {@link STI_Init CFst_STI_Init}(iMyFst,nMyUnit,FSTI_SORTINI);
 *   BYTE*         lpTrans     = NULL;
 *
 *   <span class="c-key">while</span> ((lpTrans=<span class="c-key">CFst_STI_TfromS</span>(iMySearch,nMyIniState,lpTrans))!=NULL)
 *   {
 *     <span class="c-cmt">/&#42; e.g. Get terminal state &#42;/</span>
 *     FST_ITYPE nTerS = *{@link STI_TTer CFst_STI_TTer}(iMySearch,lpTrans);
 *     ...
 *   }
 *   {@link STI_Done CFst_STI_Done}(iMySearch);
 *
 * </pre>
 * <p>If <code>nState</code> is negative, the next entry of the transition
 * table is returned.</p>
 *
 * <h3>Runtime Considerations</h3>
 * <ul>
 *   <li>This iteration function is faster for sorted graphs.</li>
 *   <li>You should rather use a forward iteration using
 *   <code>CStr_SSI_TfromN</code> than a backward iteration using
 *   {@link STI_TtoS CFst_STI_TtoS}.
 * </ul>
 * 
 * @param lpTI    Iterator data structure (returned by {@link STI_Init CFst_STI_Init})
 * @param nState  Unit relative initial state index of transitions to search
 *                (or -1 for any transition)
 * @param lpTrans Start transition (may be <code>NULL</code>)
 * @return        A pointer to the next transition or <code>NULL</code> if no
 *                such transition was found
 */
BYTE* CGEN_EXPORT CFst_STI_TfromS(FST_TID_TYPE* lpTI, FST_ITYPE nState, BYTE* lpTrans)
{
  /* Validation */
  if (nState>lpTI->nXS) return NULL;

  /* PT mode */
  if(lpTI->nMode==FSTI_PTR && nState>=0){
    if(!lpTrans) return lpTI->lpPTFwd[nState]?lpTI->lpPTFwd[nState]->lpT:NULL;
    INT32 nT=(lpTrans-lpTI->lpFT)/lpTI->nRlt;
    return lpTI->lpPTMem[nT].lpFN?lpTI->lpPTMem[nT].lpFN->lpT:NULL;
  }

  /* Get pointer to first transition to search */
  if (lpTrans==NULL)
  {
    if (!lpTI->iFst->m_lpFts || (lpTI->nMode&FSTI_SORTINI)==0 || nState<=0)
      lpTrans = lpTI->lpFT;
    else
      lpTrans = lpTI->lpFT + lpTI->nRlt*(lpTI->iFst->m_lpFts[nState] - lpTI->nFT);
  }
  else lpTrans+=lpTI->nRlt;

  /* Simple iteration */
  if (nState<0) return lpTrans<(lpTI->lpFT+lpTI->nRlt*lpTI->nXT) ? lpTrans : NULL;

  /* Traverse transition table */
  for (; lpTrans<(lpTI->lpFT+lpTI->nRlt*lpTI->nXT); lpTrans+=lpTI->nRlt)
  {
    if (nState==*(FST_ITYPE*)(lpTrans+lpTI->nOfTIni)) return lpTrans;
    if (lpTI->iFst->m_lpFts && (lpTI->nMode&FSTI_SORTINI)!=0)
    {
      if ((lpTI->nMode&FSTI_SLOPPY)!=0)
      {
        /* Sloppy sorting: continue with unsorted transitions */
        if (lpTrans<lpTI->lpFTunsrtd) lpTrans = lpTI->lpFTunsrtd-lpTI->nRlt;
      }
      else
        /* Strict sorting: no (further) transition found! */
        return NULL;
    }
  }

  /* Not found! */
  return NULL;
}

/**
 * <p>Returns a pointer to the next transition with the terminal state
 * <code>nState</code>. The search starts from transition <code>lpTrans</code>.
 * If <code>lpTrans</code> is <code>NULL</code> the transition table is
 * searched from the start.</p>
 * <p>This function may be used to enumerate all transitions ending in a given
 * state:</p>
 * <pre class="code">
 *
 *   FST_ITYPE     nMyUnit     = 0;
 *   FST_ITYPE     nMyIniState = 1;
 *   FST_TID_TYPE* iMySearch   = {@link STI_Init CFst_STI_Init}(iMyFst,nMyUnit,FSTI_SORTINI);
 *   BYTE*         lpTrans     = NULL;
 *
 *   <span class="c-key">while</span> ((lpTrans=<span class="c-key">CFst_STI_TtoS</span>(iMySearch,nMyIniState,lpTrans))!=NULL)
 *   {
 *     <span class="c-cmt">/&#42; e.g. Get initial state &#42;/</span>
 *     FST_ITYPE nIniS = *{@link STI_TIni CFst_STI_TIni}(iMySearch,lpTrans);
 *     ...
 *   }
 *   {@link STI_Done CFst_STI_Done}(iMySearch);
 *
 * </pre>
 * <p>If <code>nState</code> is negative, the next entry of the transition
 * table is returned.</p>
 *
 * <h3>Runtime Considerations</h3>
 * <ul>
 *   <li>This iteration function is <i><b>not</b></i> faster for sorted graphs.</li>
 *   <li>You should rather use a forward iteration using
 *   <code>CStr_SSI_TfromN</code> than a backward iteration using
 *   {@link STI_TtoS CFst_STI_TtoS}.
 * </ul>
 *
 * @param lpTI    Iterator data structure (returned by {@link STI_Init CFst_STI_Init})
 * @param nState  Unit relative terminal state index of transitions to search
 *                (or -1 for any transition)
 * @param lpTrans Start transition (may be <code>NULL</code>)
 * @return        A pointer to the next transition or <code>NULL</code> if no
 *                such transition was found
 */
BYTE* CGEN_EXPORT CFst_STI_TtoS(FST_TID_TYPE* lpTI, INT32 nState, BYTE* lpTrans)
{
  /* Validation */
  if (nState>lpTI->nXS) return NULL;

  /* PT mode */
  if(lpTI->nMode==FSTI_PTR && nState>=0){
    if(!lpTrans) return lpTI->lpPTBwd[nState]?lpTI->lpPTBwd[nState]->lpT:NULL;
    INT32 nT=(lpTrans-lpTI->lpFT)/lpTI->nRlt;
    return lpTI->lpPTMem[nT].lpBN?lpTI->lpPTMem[nT].lpBN->lpT:NULL;
  }

  /* Get pointer to first transition to search */
  if (lpTrans==NULL)
  {
    if (!lpTI->iFst->m_lpFts || (lpTI->nMode&FSTI_SORTTER)==0 || nState<=0)
      lpTrans = lpTI->lpFT;
    else
      lpTrans = lpTI->lpFT + lpTI->nRlt*(lpTI->iFst->m_lpFts[nState] - lpTI->nFT);
  }
  else lpTrans+=lpTI->nRlt;

  /* Simple iteration */
  if (nState<0) return lpTrans<(lpTI->lpFT+lpTI->nRlt*lpTI->nXT) ? lpTrans : NULL;

  /* Traverse transition table */
  for (; lpTrans<(lpTI->lpFT+lpTI->nRlt*lpTI->nXT); lpTrans+=lpTI->nRlt)
  {
    if (nState==*CFst_STI_TTer(lpTI,lpTrans)) return lpTrans;
    if (lpTI->iFst->m_lpFts && (lpTI->nMode&FSTI_SORTTER)!=0)
    {
      if ((lpTI->nMode&FSTI_SLOPPY)!=0)
      {
        /* Sloppy sorting: continue with unsorted transitions */
        if (lpTrans<lpTI->lpFTunsrtd) lpTrans = lpTI->lpFTunsrtd-lpTI->nRlt;
      }
      else
        /* Strict sorting: no (further) transition found! */
        return NULL;
    }
  }

  /* Not found! */
  return NULL;
}

/**
 * <p>Walks through an automaton graph and calls a user defined callback
 * function for every transition traversed during the walk. The walk continues
 * until all paths starting in <code>nState</code></p>
 * <ul>
 *   <li>terminated in a final state <i>or</i></li>
 *   <li>reached the maximal path length as specified through field {@link max_len}.
 * </ul>
 * <p>If a call to the user defined walk callback function returns
 * <code>FALSE</code>, the walk will be aborted (in <b>all</b> paths).</p>
 *
 * <h3>Automaton walking callback function</h3>
 * <pre class="code">
 *
 *   typedef BOOL (<b>FST_WALKFUNC_TYPE</b>)(FST_TID_TYPE* lpTI, BYTE* lpTrans, void* lpWalkFuncData);
 *
 *   <b>lpTI          </b>: Pointer to iterator data structure
 *   <b>lpTrans       </b>: Pointer to current transition (for use with CFst_STI_TXxx methods)
 *   <b>lpWalkFuncData</b>: User defined pointer to pass to callback function
 * </pre>
 * <p>Please note that the callback function must return <code>TRUE</code> for
 * the recursion to continue. If the function returns <code>FALSE</code> the
 * recursion will be aborted.</p>
 *
 * <h3>Runtime Considerations</h3>
 * <ul>
 *   <li>This iteration function is faster for sorted graphs.</li>
 *   <li>This iteration function is implemented as a recursion, the maximal
 *       recursion depth may be controlled through field {@link max_len}.</li>
 *   <li>The iteration is safe for cyclic graphs.</li>
 * </ul>
 *
 * @param lpTI           Pointer to iterator data structure (returned by
 *                       {@link STI_Init CFst_STI_Init})
 * @param lpWalkFunc     User defined callback function
 * @param lpWalkFuncData User defined pointer to be passed to the walk callback
 *                       function
 * @param nState         Automaton state to start the walk at (unit relative 
 *                       state index)
 * @param bBwd           If <code>TRUE</code> walk backward, otherwise walk forward.
 */
BOOL CGEN_SPUBLIC CFst_STI_Walk
(
  FST_TID_TYPE*      lpTI,
  FST_WALKFUNC_TYPE* lpWalkFunc,
  void*              lpWalkFuncData,
  FST_ITYPE          nState,
  BOOL               bBwd
)
{
  return CFst_STI_WalkInt(lpTI,lpWalkFunc,lpWalkFuncData,nState,bBwd,0);
}

/**
 * Implementation of the automaton walking recursion. See
 * {@link STI_Walk CFst_STI_Walk} for details.
 */
BOOL CGEN_SPRIVATE CFst_STI_WalkInt
(
  FST_TID_TYPE*      lpTI,
  FST_WALKFUNC_TYPE* lpWalkFunc,
  void*              lpWalkFuncData,
  FST_ITYPE          nState,
  BOOL               bBwd,
  INT32              nDepth
)
{
  BYTE* lpTrans = NULL;

  if (nDepth>=lpTI->iFst->m_nMaxLen) return TRUE;

  if (bBwd)
    while ((lpTrans=CFst_STI_TtoS(lpTI,nState,lpTrans))!=NULL)
    {
      lpTI->nDepth = nDepth;
      if (!lpWalkFunc(lpTI,lpTrans,lpWalkFuncData)) return FALSE;
      CFst_STI_WalkInt(lpTI,lpWalkFunc,lpWalkFuncData,*CFst_STI_TIni(lpTI,lpTrans),TRUE,nDepth+1);
    }
  else
    while ((lpTrans=CFst_STI_TfromS(lpTI,nState,lpTrans))!=NULL)
    {
      lpTI->nDepth = nDepth;
      if (!lpWalkFunc(lpTI,lpTrans,lpWalkFuncData)) return FALSE;
      CFst_STI_WalkInt(lpTI,lpWalkFunc,lpWalkFuncData,*CFst_STI_TTer(lpTI,lpTrans),FALSE,nDepth+1);
    }

  return TRUE;
}

/* EOF */
