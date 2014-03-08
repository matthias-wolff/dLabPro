/* dLabPro class CFst (fst)
 * - Synchroneous token passing search
 *
 * AUTHOR : Thomas Hutschenreuther
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

#define IFDOPRINT(A,B) \
  if (BASEINST(_this)->m_nCheck>=A && ((_this->m_nPrintstop>=0 && _this->m_nPrintstop<=B) || (INT16)_this->m_nPrintstop==-2))
#undef printf




INT16 CGEN_PUBLIC CFst_Stp_checkArgs
(
  CFst*  _this,
  CData* idWeights
)
{

  if (idWeights == NULL) return IERROR(_this, FST_EMPTY, "Synchroneous weights array", NULL, NULL);

  return O_K;
}

void CGEN_PRIVATE CFst_Stp_printWLR(FST_WLR_TYPE* lpWLR)
{
  printf("\n -------------------WLR------------------------");
  printf("\n id:              %lx", (unsigned long)lpWLR);
  printf("\n weight:          %lf", (double)lpWLR->nWeight);
  printf("\n terminal symbol: %ld", (long)lpWLR->sTerminal);
  printf("\n associated time: %ld", (long)lpWLR->t);
  printf("\n nPointers:       %ld", (long)lpWLR->nPointers);
  printf("\n ----------------------------------------------");
}

/**
 * Destroy one WLR
 * @param lpWLR   WLR to destroy
 *
 */

void CGEN_PUBLIC CFst_Stp_destroyWLR(CFst* _this, FST_WLR_TYPE** lpWLR);
void CGEN_PUBLIC CFst_Stp_destroyWLR(CFst* _this, FST_WLR_TYPE** lpWLR)
{
  if ((*lpWLR)==NULL) return;                      /* if nothing to destroy do nothing*/
  if ((*lpWLR)->nPointers<2){                      /* if nobody else points to WLR*/
    CFst_Stp_destroyWLR(_this, &((*lpWLR)->lpNext));        /* >> destroy it recursively*/
/*    printf("\n -----------------DESTROYING-------------------"); */
/*    CFst_Stp_printWLR(*lpWLR); */
    dlp_free((*lpWLR));
    _this->m_nWLRs--;
    *lpWLR = NULL;                          /* */
  }else{                                /* else */
    (*lpWLR)->nPointers--;                      /* remove one pointer */
  }
  return;
}



/**
 * Keep track of n-best alignments using backtracking table
 * Takes a WLR and determines via its associated weight, wether it is to be put into the backtracking table
 *
 * @param lpBestTable  Pointer to table keeping track of the n best hypotheses for word boundaries for each time
 * @param lpWLR      Pointer to WLR to be put into Table
 * @param t         Time
 * @param nPaths    Number of paths to be found
 *
 * @return Return value is 1 if WLR has been put on top of the table entry for time t and 0 otherwise
 *
 */
INT16 CGEN_PUBLIC CFst_Stp_updateTable
(
CFst*       _this,
FST_WLR_TYPE*** lpBestTable,
FST_WLR_TYPE**   lpWLR,
INT32       t,
INT32       nPaths
)
{
  INT32       i         = 0;
  FST_WLR_TYPE*  lpTmp      = NULL;
  BOOL       bFromTable     = FALSE;
  BOOL       bBest       = FALSE;

  while(i<nPaths){
    if((*lpBestTable)[t*nPaths+i]==NULL){
      (*lpBestTable)[t*nPaths+i] = (*lpWLR);
      if (i==0) bBest = TRUE;
      if (bFromTable==0) (*lpWLR)->nPointers++;
      break;
    }

    if((*lpBestTable)[t*nPaths+i]->nWeight>=(*lpWLR)->nWeight){
      if (i==0) bBest = TRUE;
      lpTmp = NULL;
      if ((*lpBestTable)[t*nPaths+i]->nIState!=(*lpWLR)->nIState){
        lpTmp = (*lpBestTable)[t*nPaths+i];                      /* remove element from bestTable*//*TODO----Fehlerteufel */
        (*lpBestTable)[t*nPaths+i] = *lpWLR;                    /* insert element into bestTable */
        if (bFromTable == 0) {(*lpWLR)->nPointers++;}
        *lpWLR = lpTmp;
        bFromTable = 1;
      }else{
        CFst_Stp_destroyWLR(_this, &((*lpBestTable)[t*nPaths+i]));
        (*lpBestTable)[t*nPaths+i] = *lpWLR;
        if (bFromTable == 0) {(*lpWLR)->nPointers++;}
        break;
      }
    }else{
      if ((*lpBestTable)[t*nPaths+i]->nIState==(*lpWLR)->nIState){
        CFst_Stp_destroyWLR(_this, lpWLR);
        break;
      }
    }
    if((i==nPaths-1)&&((*lpBestTable)[t*nPaths+i])){
      if ((*lpBestTable)[t*nPaths+i]->nWeight<(*lpWLR)->nWeight){
        CFst_Stp_destroyWLR(_this, lpWLR);
        break;
      }else{
        dlp_assert("\n Warning in table updater", __FILE__, __LINE__);
      }
    }

    i++;
  }
  return bBest;
}

/**
 * Clear backtracking table
 * @param lpBestTable  Pointer to table keeping track of the n best hypotheses for word boundaries for each time
 * @param nLength      Length of backtracking table
 * @param nPaths     Number of entries per timestep
 *
 */

void CGEN_PRIVATE CFst_Stp_freeBackTrackTable(CFst* _this, FST_WLR_TYPE*** lpBestTable,INT32 nLength, INT32 nPaths){
  INT32 i = 0;
  INT32 j = 0;
  for (i = 0;i<nLength;i++){
    for (j = 0; j < nPaths; j++){
      if (((*lpBestTable)[i*nPaths+j])!=NULL) {
        dlp_free((*lpBestTable)[i*nPaths+j]);
        _this->m_nWLRs--;
      }
    }
  }
  dlp_free(*lpBestTable);
}

/**
 * Build up FST containing the n-best wordalignment hypotheses
 *
 * This is done by passing through the backtrackingtable from the end to the start.
 *
 * Initially n Paths are started according to the n entries in the end of the table.
 * Then at every timestep, if a path closes (the WLR defining this path points to a WLR at this timestep),
 * a new path defined by the best WLR in the backtracktable for this timestep is started.
 *
 * Path costs are determined by the difference between the weight of the WLR defining the path and the weight of the one it points to.
 * The terminal symbol of a path is the terminal symbol stored in the corresponding WLR
 *
 * @param lpBestTable  Pointer to table keeping track of the n best hypotheses for word boundaries for each time
 * @param nLength      Length of backtracking table
 * @param nPaths     Number of paths to be found
 *
 * @return Errorcode if errors occur
 */


INT16 CGEN_PRIVATE CFst_stp_backtrackTable(CFst* _this, FST_WLR_TYPE*** lpBestTable, INT32 nLength, INT32 nPaths){
  INT32*      lpStateList   = (INT32*)dlp_calloc(2*nLength,sizeof(INT32));    /* Array: t(=index) | index of state at t | number of paths ending at t*/
  INT32        i        = 0;                        /* General purpose loop index counter*/
  INT32      j        = 0;                        /* General purpose loop index counter*/
  INT32       nTmp       = 0;
  INT32      nOpen      = 0;                        /* Number of currently open paths*/
  INT16      nErr      = 0;
  FST_WTYPE    nWeight      = 0.0;                        /* Weight of current transition*/
  FST_WLR_TYPE*  lpWLR      = NULL;                        /* Pointer to current WLR*/

  /* initialize stateArray*/
  for (i = 0; i < nLength; i++){
    lpStateList[i] = -1;                          /* no states*/
    lpStateList[i+nLength] = 0;                        /* no transitions*/
  }


  /*initialize fst containing the n-best paths*/
  CFst_Addunit(_this, "nBest");
  CFst_Addstates(_this, 0, 1, FALSE);                      /* Add initial state*/
  lpStateList[nLength-1] = CFst_Addstates(_this, 0, 1, TRUE);          /* Add final state*/

  /*backtrack decisionlist*/
  for (i = nLength-1; i >= 0; i--){

    nOpen = nOpen - lpStateList[i+nLength];                            /* refresh count of open paths (previously open paths - currently closing paths)*/
    nTmp = nPaths - nOpen;
    for (j = nTmp; j > 0; j--){                              /*  --> less than n paths open?*/
      lpWLR = (FST_WLR_TYPE*) (*lpBestTable)[i*nPaths+(nTmp-j)];              /*    query besttable for WLR*/
      if (lpWLR == NULL){
        continue;
      }
      if (lpWLR->lpNext==NULL) {                                /* transition from initial state (no further states to expect)*/
        nErr = CFst_AddtransEx(_this, 0, 0, lpStateList[lpWLR->t], -1, lpWLR->sTerminal, lpWLR->nWeight); /* add transition */
        if (nErr < 0)
        {
          dlp_free(lpStateList);
          return nErr;
        }
        nOpen++;
        continue;
      }
      if (lpStateList[lpWLR->lpNext->t] == -1){                        /*    --> not any end of path yet at lpWLR->t?*/
        lpStateList[lpWLR->lpNext->t] = CFst_Addstates(_this, 0, 1, FALSE);          /*      add new state do destination acceptor (and register it to stateList)*/
        lpStateList[lpWLR->lpNext->t + nLength]++;                      /*      increase paths ending at t*/
        nWeight = CFst_Wsr_Op(_this,lpWLR->nWeight,lpWLR->lpNext->nWeight,OP_DIV);      /*       calculate transition weight*/
        nErr = CFst_AddtransEx(_this, 0, lpStateList[lpWLR->lpNext->t], lpStateList[lpWLR->t], -1, lpWLR->sTerminal, nWeight);
        if (nErr < 0)
        {
          dlp_free(lpStateList);
          return nErr;
        }
        nOpen++;
      }else{                                          /*    <-> */
        lpStateList[lpWLR->lpNext->t + nLength]++;                      /*      increase paths ending at t*/
        nWeight = CFst_Wsr_Op(_this,lpWLR->nWeight,lpWLR->lpNext->nWeight,OP_DIV);      /*       calculate transition weight*/
        nErr = CFst_AddtransEx(_this, 0, lpStateList[lpWLR->lpNext->t], lpStateList[lpWLR->t], -1, lpWLR->sTerminal, nWeight);
        if (nErr < 0)
        {
          dlp_free(lpStateList);
          return nErr;
        }
        nOpen++;
      }                                            /*    <-- */
    }                                              /*  <--*/
  }
  dlp_free(lpStateList);
  if (nErr>=0){
    return O_K;
  }else{
    return nErr;
  }
} /*tick*/

/*
 * Re-entry of tokens into automaton
 * @param t    Current step
 * @param nUnit    Number of unit in source fst to find recognition network
 * @param nPaths  Number of paths to be found
 * @param lpTI    Iterator instance for source fst
 * @param lpSW    Synchroneous weights array for current timestep
 * @param lpNewTokens  Pointer to array containing the tokens of the current timestep
 * @param lpBestTable  Pointer to table to be filled with the n-best hypotheses
 * @param itSrc    Source fst
 */

INT16 CGEN_PRIVATE CFst_Stp_propagateEntryTokens
(
  CFst*      _this,
  INT32       t,
  INT32      nUnit,
  INT32      nPaths,
  FST_TID_TYPE*   lpTI,
  FST_WTYPE*      lpSW,
  FST_TOK_TYPE***  lpNewTokens,
  FST_WLR_TYPE*** lpBestTable,
  CFst*      itSrc
)
{
  BYTE* lpTX = NULL;
  FST_WLR_TYPE* lpWLR = NULL;
  FST_ITYPE nTerS = -1;
  FST_STYPE nTos  = -1;
  FST_STYPE nTis  = -1;
  FST_WTYPE nWX  = 0.0;
  FST_WTYPE nWNew = 0.0;
  FST_WTYPE nSw  = 0.0;


  if (t == 0) {                                            /* initial input */
    lpWLR             = (FST_WLR_TYPE*)dlp_calloc(1,sizeof(FST_WLR_TYPE));          /* Create WLR*/
    lpWLR->lpNext        = NULL;                                        /*   attach nothing*/
    lpWLR->nWeight         = 0;                                /* currently accumulated weight --> weight of new WLR */
    lpWLR->sTerminal      = -1;
    lpWLR->t          = t;
    lpWLR->nPointers      = 0;
    lpWLR->nTState        = 0;
  }else{
    if (!(*lpBestTable)[(t-1)*nPaths]) return O_K;                          /*if no entry tokens to propagate return*/
    lpWLR = (*lpBestTable)[(t-1)*nPaths];
  }
  lpWLR->nPointers++;


  while((lpTX = CFst_STI_TfromS(lpTI, lpWLR->nTState+UD_FS(itSrc,nUnit), lpTX))!=NULL)                /* for all transitions leaving terminal state with best weight*/
  {
    nTerS = *CFst_STI_TTer(lpTI, lpTX);                                      /* Get terminal state (j) of current transition*/
    nTis  = *CFst_STI_TTis(lpTI, lpTX);                                      /* Get input symbol*/
    nTos  = *CFst_STI_TTos(lpTI, lpTX);                                      /* Get output symbol*/
    nWX   = *CFst_STI_TW(lpTI, lpTX);                                      /* Get weight of current transition (w_ij)*/
    nSw  = (!lpSW || nTis<0)?CFst_Wsr_NeMult(_this->m_nWsr):lpSW[nTis];                         /*   Get synchroneous weight          */
    nWNew = CFst_Wsr_Op(itSrc, nWX, nSw, OP_MULT);          /* s_j  = w_ij + d_j(t) */
    nWNew = CFst_Wsr_Op(itSrc, lpWLR->nWeight, nWNew, OP_MULT);

    if (nWNew<(*lpNewTokens)[nTerS]->nWeight){                            /* --> check wether weight changes*/
      (*lpNewTokens)[nTerS]->nWeight=nWNew;
      (*lpNewTokens)[nTerS]->bChanged = TRUE;


      /*propagate WLR*/
      if (!(*lpNewTokens)[nTerS]->lpWLR||(*lpNewTokens)[nTerS]->lpWLR!=lpWLR){      /* no need to propagate WLR twice via different transitions*/
        CFst_Stp_destroyWLR(_this, &((*lpNewTokens)[nTerS]->lpWLR));                  /*      remove pointer from WLR*/
        (*lpNewTokens)[nTerS]->lpWLR = lpWLR;                /*      token of terminal state points to WLR of initial state*/
        (*lpNewTokens)[nTerS]->lpWLR->nPointers++;                            /*      one more pointer points to WLR */                    /*        one more pointer points to WLR */
      }
      if (t==0){
        CFst_Stp_destroyWLR(_this, &((*lpNewTokens)[nTerS]->lpWLR));
        (*lpNewTokens)[nTerS]->lpWLR = NULL;

      }
      /*collect or propagate terminal symbol*/
      if (nTos>=0){
        if ((*lpNewTokens)[nTerS]->sTerminal>=0){
          CFst_Stp_destroyWLR(_this, &lpWLR);
          return IERROR (_this, FST_SYMBOLCRASH, NULL, NULL, NULL);

        }
        (*lpNewTokens)[nTerS]->sTerminal = nTos;
      }else{
        (*lpNewTokens)[nTerS]->sTerminal = -1;
      }
    }



  }

  CFst_Stp_destroyWLR(_this, &lpWLR);
  return O_K;
}

/**
 * Update weights of one token
 * @param lpNewTokens  Pointer to array holding pointers to tokens corresponding to this timestep
 * @param lpOldTokens  Pointer to array holding pointers to tokens corresponding to the last timestep
 * @param lpBestTable  Pointer to table keeping track of the n best hypotheses for word boundaries for each time
 * @param lpTI    Pointer to automaton iterator data struct
 * @param lpTX    Pointer to current transition
 * @param lpSW    Synchroneous weights for this timestep
 * @param t       Time (column index in <code>lpSWa</code>)
 * @param nPaths   Number of paths to be found
 * @param nPassings  Number of Tokens passed at current step
 * @param bEps    Process epsilon transitions?
 */

INT16 CGEN_PRIVATE CFst_Stp_updateWeights
(
  CFst*       _this,
  CFst*      itSrc,
  FST_TOK_TYPE*** lpNewTokens,
  FST_TOK_TYPE*** lpOldTokens,
  FST_WLR_TYPE*** lpBestTable,
  FST_TID_TYPE*  lpTI,
  BYTE*      lpTX,
  FST_WTYPE*    lpSW,
  INT32      t,
  INT32       nPaths,
  INT32*      nPassings,
  char       bEps
)
{
  FST_WLR_TYPE*    lpWLR = NULL;
  FST_WTYPE    nWTmp = 0.0;
  FST_WTYPE        nWNew = 0.0;
  FST_WTYPE    nWX  = 0.0;                            /* Current transition weight*/
  FST_WTYPE    nSw  = 0.0;
  FST_STYPE    nTis  = -1;
  FST_STYPE    nTos  = -1;
  FST_ITYPE    nIniS  = 0;
  FST_ITYPE    nTerS  = 0;


  nTis  = *CFst_STI_TTis(lpTI, lpTX);                                      /* Get input symbol*/
    if ( bEps && (nTis!=-1)) return O_K;                                                          /*   Epsilon mode: skip non-epsilon   */
  if (!bEps && (nTis==-1)) return O_K;                                                         /*   Non-epsilon mode: skip epsilon   */
  nIniS = *CFst_STI_TIni(lpTI, lpTX);                                      /* Get initial state (i) of current transition*/
  if (SD_FLG(itSrc,nIniS)) return O_K;                                  /* re-entry into automaton is handled elsewhere*/
  nTerS = *CFst_STI_TTer(lpTI, lpTX);                                      /* Get terminal state (j) of current transition*/
    nTos  = *CFst_STI_TTos(lpTI, lpTX);                                      /* Get output symbol*/
  nWX   = *CFst_STI_TW(lpTI, lpTX);                                      /* Get weight of current transition (w_ij)*/
  nSw  = (!lpSW || nTis<0)?CFst_Wsr_NeMult(_this->m_nWsr):lpSW[nTis];                         /*   Get synchroneous weight          */
  nWNew = CFst_Wsr_Op(itSrc, nWX, nSw, OP_MULT);                                /* s_j  = w_ij + d_j(t) */
  nWTmp = CFst_Wsr_Op(itSrc, (*lpOldTokens)[nIniS]->nWeight, nWNew, OP_MULT);                  /* D_j' = D_i + s_j*/

  if ((nWTmp >= (*lpNewTokens)[nTerS]->nWeight) && !(*lpOldTokens)[nIniS]->bChanged) return O_K;    /* Discard suboptimal hypotheses that were already considered*/


  /*Create WLR and update backtracking table*/
  if (SD_FLG(itSrc,nTerS)&& (!CFst_Wsr_Op(_this,nWTmp,CFst_Wsr_NeAdd(_this->m_nWsr),OP_EQUAL)))        /* if final state reached*/
  {
    if ((*lpOldTokens)[nIniS]->sTerminal>=0||nTos>=0)                            /* and symbol seen*/
    {
      lpWLR             = (FST_WLR_TYPE*)dlp_calloc(1,sizeof(FST_WLR_TYPE));          /* Create WLR*/
      _this->m_nWLRs++;

      if ((*lpOldTokens)[nIniS]->lpWLR!=NULL)                                /* if already one WLR in history of token*/
      {
        lpWLR->lpNext         = (*lpOldTokens)[nIniS]->lpWLR;                    /*   attach old WLR to new WLR*/
        lpWLR->lpNext->nPointers  += 1;                                /*   one more instance points to old WLR*/
      }
      else
      {                                                /* else*/
        lpWLR->lpNext=NULL;                                        /*   attach nothing*/
      }
      lpWLR->nWeight         = nWTmp;                                /* currently accumulated weight --> weight of new WLR */
      if ((*lpOldTokens)[nIniS]->sTerminal>=0)                              /* output symbol was seen on some former transition*/
        lpWLR->sTerminal      = (*lpOldTokens)[nIniS]->sTerminal;                  /* remember terminal symbol*/
      if (nTos >= 0)                                            /* output symbol was seen on the current transition*/
        lpWLR->sTerminal      = nTos;                                /* remember terminal symbol*/
      lpWLR->t          = t;                                  /* time of creation*/
      lpWLR->nPointers      = 0;                                  /* new WLR --> no Pointers*/
      lpWLR->nTState        = nTerS;                                /* Terminal state of creating transition*/
      lpWLR->nIState        = nIniS;                                /* Initial state of creating transition*/
      CFst_Stp_updateTable(_this, lpBestTable, &lpWLR, t, nPaths);                    /* Put WLR into n backtrackingtable at t*/
    }else{
      return (INT32) IERROR(_this, FST_NOSYMBOL, NULL, NULL, NULL);                    /* Empty word NOT accepted*/
    }
  }

  /*update weight of token of terminal state*/
  if ((*lpNewTokens)[nTerS]&&(*lpOldTokens)[nIniS])
  {
    if (nWTmp<(*lpNewTokens)[nTerS]->nWeight)                            /* --> check wether weight changes*/
    {
      (*nPassings)++;
      (*lpNewTokens)[nTerS]->nWeight=nWTmp;                            /* update weight of token*/
      (*lpNewTokens)[nTerS]->bChanged = TRUE;                            /* token was updated*/

      if (!SD_FLG(itSrc,nTerS))                                  /* if terminal state of current transition not final*/
      {
        /*propagate WLR*/
        CFst_Stp_destroyWLR(_this, &((*lpNewTokens)[nTerS]->lpWLR));                /*      remove pointer from WLR*/
        (*lpNewTokens)[nTerS]->lpWLR = NULL;
        if ((*lpOldTokens)[nIniS]->lpWLR != NULL)                          /*      --> if initial state points to WLR*/
        {
          (*lpNewTokens)[nTerS]->lpWLR = (*lpOldTokens)[nIniS]->lpWLR;              /*        token of terminal state points to WLR of initial state*/
          (*lpNewTokens)[nTerS]->lpWLR->nPointers++;                        /*        one more pointer points to WLR */
        }                                              /*      <-- */
          /*collect or propagate terminal symbol*/
        if (nTos>=0)                                        /* if terminal symbol on this transition*/
        {
          if ((*lpNewTokens)[nTerS]->sTerminal>=0)                        /*   if already seen output symbol on this path*/
          {
            return IERROR (_this, FST_SYMBOLCRASH, NULL, NULL, NULL);              /*     return error*/
          }
          (*lpNewTokens)[nTerS]->sTerminal = nTos;                        /*   collect output symbol*/
        }
        else                                            /* else*/
        {
          (*lpNewTokens)[nTerS]->sTerminal = (*lpOldTokens)[nIniS]->sTerminal;          /*  propagate output symbol */
        }
      }
    }

  }
  return O_K;
}
/* Perform one token passing step
 *
 * @param t    Current step
 * @param nUnit    Number of unit in source fst to find recognition network
 * @param nPaths  Number of paths to be found
 * @param bEps    Process epsilon transitions?
 * @param lpTI    Iterator instance for source fst
 * @param lpSW    Synchroneous weights array for current timestep
 * @param lpOldTokens  Pointer to array containing the tokens of the last timestep
 * @param lpNewTokens  Pointer to array containing the tokens of the current timestep
 * @param lpBestTable  Pointer to table to be filled with the n-best hypotheses
 * @param itSrc    Source fst
 */
INT32 CGEN_PRIVATE CFst_Stp_passTokens
(
  CFst*      _this,
  INT32       t,
  INT32      nUnit,
  INT32      nPaths,
  char      bEps,
  FST_TID_TYPE*   lpTI,
  FST_WTYPE*      lpSW,
  FST_TOK_TYPE***  lpOldTokens,
  FST_TOK_TYPE***  lpNewTokens,
  FST_WLR_TYPE*** lpBestTable,
  CFst*      itSrc


)
{

  INT32       i        = 0;                            /* loop counter*/
  INT32      nIs      = 0;                            /* current state (not unit-relative)*/
  INT32       nPassings    = 0;
  INT32       nErr       = 0;
  BYTE*         lpTX       = NULL;                            /* Ptr. to current source transition  */


  /*token-passing */
  for(i = 0;i < UD_XS(itSrc, nUnit);i++)                          /* for each state*/
  {
    nIs = i + UD_FS(itSrc, nUnit);                            /* bias initial state to unit*/
    while((lpTX = CFst_STI_TfromS(lpTI, nIs, lpTX))!=NULL)                /* for each transition */
    {
        nErr = CFst_Stp_updateWeights(_this, itSrc, lpNewTokens, lpOldTokens, lpBestTable, lpTI, lpTX, lpSW, t, nPaths, &nPassings, bEps); /*update token*/
        if (nErr < 0)
          return nErr;
    }
  }
   /*finalize changes*/
   for (i = 0; i < UD_XS(itSrc,nUnit); i++)                                                  /* for each state*/
   {

     (*lpOldTokens)[i]->bChanged = FALSE;                                                  /* initialize change status*/
    if ((*lpNewTokens)[i]->bChanged)                                                /* if token was updated*/
    {
      CFst_Stp_destroyWLR(_this, &((*lpOldTokens)[i]->lpWLR));                                      /* remove pointer from token to be updated*/
      (*lpOldTokens)[i]->lpWLR = (*lpNewTokens)[i]->lpWLR;                                        /* finalize WLR*/
      if (bEps&&(*lpOldTokens)[i]->lpWLR) (*lpOldTokens)[i]->lpWLR->nPointers++;
      (*lpOldTokens)[i]->nWeight = (*lpNewTokens)[i]->nWeight;                                      /* finalize weight*/
      (*lpOldTokens)[i]->sTerminal = (*lpNewTokens)[i]->sTerminal;                                    /* finalize terminal symbol*/
      (*lpOldTokens)[i]->bChanged = TRUE;                                                  /* was finalized*/
    }
    if(!bEps&&(*lpOldTokens)[i]->bChanged==FALSE)
    {
      (*lpOldTokens)[i]->lpWLR = NULL;
      (*lpOldTokens)[i]->nWeight = CFst_Wsr_NeAdd(_this->m_nWsr);          /*weight of tokens = 0_semiring*/
      (*lpOldTokens)[i]->sTerminal = -1;
    }
    if (SD_FLG(itSrc,i)) (*lpOldTokens)[i]->sTerminal = -1;
    (*lpNewTokens)[i]->bChanged = FALSE;
   }
   return nPassings;


}

/* Initialize array containing tokens
 * @param itSrc   Source fst
 * @param nUnit   Unit in source fst to find recognition network
 * @param lpTokens   Pointer to array to be initialized
 */
void CGEN_PRIVATE CFst_Stp_initializeTokens
(
  CFst* _this,
  CFst* itSrc,
  INT32 nUnit,
  FST_TOK_TYPE*** lpTokens
)
{
  INT32 i = 0;
  *lpTokens = (FST_TOK_TYPE**)dlp_calloc(UD_XS(itSrc,nUnit),sizeof(FST_TOK_TYPE*));    /* Allocate array of pointers to tokens*/
  for (i = 0; i < UD_XS(itSrc,nUnit); i++){                        /* all other tokens*/
    (*lpTokens)[i] = (FST_TOK_TYPE*)dlp_calloc(1,sizeof(FST_TOK_TYPE));          /* allocate memory for token*/
    (*lpTokens)[i]->lpWLR = NULL;                            /* pointer to WLR*/
    (*lpTokens)[i]->nWeight = CFst_Wsr_NeAdd(_this->m_nWsr);              /* weight of tokens = 0_semiring*/
    (*lpTokens)[i]->sTerminal = -1;
    (*lpTokens)[i]->bChanged = FALSE;
  }
}
/* Tidy up after finishing stp or if error occurs
 *
 * @param itSrc    Source fst
 * @param lpOldTokens  Pointer to array containing the tokens of the last timestep
 * @param lpNewTokens  Pointer to array containing the tokens of the current timestep
 * @param lpBtTable  Pointer to backtracking table
 * @param lpTI    Iterator instance for source fst
 * @param nPaths  Number of paths to be found
 * @param nLength  Length of backtracking table
 * @param nUnit    Number of unit in source fst to find recognition network
 */

void CGEN_PRIVATE CFst_Stp_Done
(
  CFst* _this,
  CFst* itSrc,
  FST_TOK_TYPE*** lpOldTokens,
  FST_TOK_TYPE*** lpNewTokens,
  FST_WLR_TYPE*** lpBtTable,
  FST_TID_TYPE** lpTI,
  INT32 nPaths,
  INT32 nLength,
  INT32 nUnit
)
{
  INT32 i = 0;
  for(i = 0;i < UD_XS(itSrc, nUnit);i++)                      /* loop over all states (remove tokens)*/
  {
    if ((*lpOldTokens)[i]!=NULL)
    {
      if (((*lpOldTokens)[i]->lpWLR)!=NULL)
        CFst_Stp_destroyWLR(_this, &((*lpOldTokens)[i]->lpWLR));

      dlp_free((*lpOldTokens)[i]);
    }
    if ((*lpNewTokens)[i]!=NULL) dlp_free((*lpNewTokens)[i]);

  }
  dlp_free((*lpNewTokens));
  dlp_free(*lpOldTokens);
  CFst_Stp_freeBackTrackTable(_this, lpBtTable, nLength, nPaths);
  CFst_STI_Done(*lpTI);

}

INT16 CGEN_PUBLIC CFst_StpUnit
(
  CFst*  _this,
  CFst*  itSrc,
  INT32  nUnit,
  INT32  nPaths,
  CData*  idWeights
)
{
  INT32          t          = 0;                            /* Current time                      */
  INT32      i        = 0;                            /* general purpose loop iteration index */
  INT32          nTmax      = 0;                            /* Maximal time                      */
  INT32          nC         = 0;                                                     /* Component counter                 */
  INT32          nXW        = 0;                                                      /* No. of synch. weights (at time t) */
  INT32       nErr      = 0;
  INT32       nPassings    = 0;
  INT32       nEps      = 0;                            /* No. of epsilon updates*/
  BOOL          bSWd       = FALSE;                                                 /* Sync. weights directly from input */
  FST_TOK_TYPE** lpOldTokens = NULL;                            /* Array holding the valid tokens*/
  FST_TOK_TYPE** lpNewTokens  = NULL;                            /* temporarily used for token propagation*/
  FST_WLR_TYPE** lpBestTable   = NULL;                            /* Table containing the n-best word-link-records for each time t, where a terminal state is reached*/
  FST_TID_TYPE* lpTI       = NULL;                            /* Source FST iterator               */
  FST_STYPE    nTis      = -1;
  FST_WTYPE*    lpSWa      = NULL;                                                   /* Synch. weights array (all times)  */
  FST_WTYPE*    lpSW      = NULL;                            /* Synch. weights array (for time t) */

  nErr = CFst_Stp_checkArgs(_this, idWeights);
  if (nErr < 0) return nErr;

  DLPASSERT(_this!=itSrc);                                                        /* Source and dest. must be different*/

  /* Initialization */                                                            /* --------------------------------- */
  _this->m_nWLRs = 0;
  _this->m_nWsr = CFst_Wsr_GetType(itSrc,&_this->m_nIcW);                         /* Get weight semiring type and comp.*/
  nTmax         = idWeights ? CData_GetNRecs(idWeights) : _this->m_nMaxLen;       /* Determine max. path length        */
  lpTI          = CFst_STI_Init(itSrc,nUnit,FSTI_SORTINI);                        /* Create source iterator            */
  IFCHECKEX(1) printf("\n Max. sync. time: %ld",(long)nTmax);                     /* Protocol                          */
  IFCHECKEX(1) printf("\n States         : %ld",(long)lpTI->nXS);                 /* Protocol                          */

  /* Count input symbols */                                                      /* --------------------------------- */
  _this->m_nSymbols = -1;                                                       /* No input symbols found so far     */
  for (i=lpTI->nFT; i<lpTI->nFT+lpTI->nXT; i++)                              /* For all transitions of nUnit      */
  {                                                                              /* >>                                */
    nTis = *CFst_STI_TTis(lpTI,CFst_STI_GetTransPtr(lpTI,i));                     /*   Get input symbol                */
    if (nTis>_this->m_nSymbols) _this->m_nSymbols=nTis;                           /*   Remember greatest symbol index  */
  }                                                                             /* <<                                */
  _this->m_nSymbols++;                                                          /* Count = last index +1             */
  IFCHECKEX(1) printf("\n Input symbols  : %ld",(long)_this->m_nSymbols);         /* Protocol                          */

  /* Analyze (and convert) synchroneous weights array */                          /* --------------------------------- */
  for (nC=0,bSWd=TRUE; nC<CData_GetNComps(idWeights); nC++)                       /* Loop over idWeights' components   */
    if (CData_GetCompType(idWeights,nC)!=DLP_TYPE(FST_WTYPE)) bSWd=FALSE;       /*   and see if they're all FST_WTYPE*/
  IFCHECKEX(1) printf("\n Convert weights: %s",bSWd?"NO":"YES");                  /* Protocol                          */
  if (bSWd)                                                                       /* Use sync. idWeights directly      */
  {                                                                               /* >>                                */
    lpSWa = (FST_WTYPE*)CData_XAddr(idWeights,0,0);                             /*   Get 'em pointer                 */
    nXW   = CData_GetNComps(idWeights);                                         /*   Get no. of weights per record   */
  }                                                                               /* <<                                */
  else                                                                            /* Need to convert sync. weights     */
  {                                                                               /* >>                                */
    INT32 nCa;                                                                   /*   Numeric component counter       */
    INT32 nR;                                                                    /*   Record counter                  */
    nXW   = _this->m_nSymbols;                                                  /*   One weight for each symbol, pls.*/
    lpSWa = (FST_WTYPE*)dlp_calloc(nXW*nTmax,sizeof(FST_WTYPE));                /*   Allocate converted weights array*/
    for (nR=0; nR<nXW*nTmax; nR++) lpSWa[nR]=CFst_Wsr_NeMult(_this->m_nWsr);    /*   Clear                           */
    for (nC=0,nCa=0; nC<CData_GetNComps(idWeights) && nCa<nXW; nC++)            /*   Loop over idWeigths' components */
      if (dlp_is_numeric_type_code(CData_GetCompType(idWeights,nC)))          /*     The numeric ones ...          */
      {                                                                       /*     >>                            */
        for (nR=0; nR<MIN(CData_GetNRecs(idWeights),nTmax); nR++)           /*       Loop over idWeigths' rcorz  */
          lpSWa[nR*nXW + nCa] = (FST_WTYPE)CData_Dfetch(idWeights,nR,nC); /*         Convert weight            */
        nCa++;                                                              /*       Count numeric components    */
      }                                                                       /*     <<                            */
  }


  /*Initialize tokens*/
  CFst_Stp_initializeTokens(_this, itSrc, nUnit, &lpOldTokens);
  CFst_Stp_initializeTokens(_this, itSrc, nUnit, &lpNewTokens);

  /*Initialize table containing n-best alternatives*/
  lpBestTable = (FST_WLR_TYPE**)dlp_calloc(nTmax*nPaths,sizeof(FST_WLR_TYPE*));
  for(i = 1;i < nTmax;i++){
    INT32 j;
    for(j = 0;j < nPaths;j++){
      lpBestTable[i*nPaths+j] = NULL;
    }
  }
  /*token-passing */
  for(t = 0;t < nTmax;t++)
  {


    CFst_Sdp_GetSWeights(_this, lpSWa, t, nXW, &lpSW);

      /* propagate entry tokens */
      if (t<(nTmax-1)){
      nPassings = CFst_Stp_propagateEntryTokens(_this, t, nUnit, nPaths, lpTI, lpSW, &lpNewTokens, &lpBestTable, itSrc);
        if (nPassings < 0)                                  /*if error occured*/
        {
          CFst_Stp_Done(_this, itSrc,&lpOldTokens,&lpNewTokens,&lpBestTable,&lpTI,nPaths,nTmax,nUnit);  /*free memory*/
          if (!bSWd) { dlp_free(lpSWa); }                          /* if weight array converted --> free*/
          return nPassings;                                /*return error*/
        }
      }
      /* non-epsilon transitions transitions */
      nPassings = CFst_Stp_passTokens(_this, t, nUnit, nPaths, FALSE, lpTI, lpSW, &lpOldTokens, &lpNewTokens, &lpBestTable, itSrc);
      if (nPassings<0)
      {
        CFst_Stp_Done(_this, itSrc,&lpOldTokens,&lpNewTokens,&lpBestTable,&lpTI,nPaths,nTmax,nUnit);
        if (!bSWd) { dlp_free(lpSWa); }                          /* if weight array converted --> free*/
        return nPassings;
      }
      /* epsilon transitions */
      nEps = 0;
      for(i = 0;i < nTmax;i++)
      {
        nPassings = 0;
        nPassings = CFst_Stp_passTokens(_this, t, nUnit, nPaths, TRUE, lpTI, lpSW, &lpOldTokens, &lpNewTokens, &lpBestTable, itSrc);
        if (nPassings==0)
          break;
        if (nPassings < 0){
          CFst_Stp_Done(_this, itSrc,&lpOldTokens,&lpNewTokens,&lpBestTable,&lpTI,nPaths,nTmax,nUnit);
          if (!bSWd) { dlp_free(lpSWa); }                          /* if weight array converted --> free*/
          return nPassings;
        }
        nEps++;
      }

    /* memorize changes */
    for (i = 0; i<UD_XS(itSrc,nUnit); i++){
      lpOldTokens[i]->lpWLR = (lpNewTokens)[i]->lpWLR;                                        /* finalize WLR*/
      lpOldTokens[i]->nWeight = (lpNewTokens)[i]->nWeight;                                      /* finalize weight*/
      lpOldTokens[i]->sTerminal = (lpNewTokens)[i]->sTerminal;                                    /* finalize terminal symbol*/
      lpNewTokens[i]->nWeight = CFst_Wsr_NeAdd(_this->m_nWsr);          /*weight of tokens = 0_semiring*/
      lpNewTokens[i]->lpWLR = NULL;
      lpNewTokens[i]->sTerminal = -1;
    }

  }
    /*Aftermath*/
  CData_Scopy(AS(CData,_this->ud),AS(CData,itSrc->ud));                         /* Copy structure of unit table      */
  CData_Scopy(AS(CData,_this->sd),AS(CData,itSrc->sd));                         /* Copy structure of state table     */
  CData_Scopy(AS(CData,_this->td),AS(CData,itSrc->td));                         /* Copy structure of transition table*/

  nErr = CFst_stp_backtrackTable(_this, &lpBestTable, nTmax, nPaths);          /* build target fst           */
  if (nErr<0) {
    CFst_Stp_Done(_this, itSrc,&lpOldTokens,&lpNewTokens,&lpBestTable,&lpTI,nPaths,nTmax,nUnit);
    if (!bSWd) { dlp_free(lpSWa); }                          /* if weight array converted --> free*/
    return nErr;
  }

  CFst_Stp_Done(_this, itSrc,&lpOldTokens,&lpNewTokens,&lpBestTable,&lpTI,nPaths,nTmax,nUnit);
  if (!bSWd) { dlp_free(lpSWa); }                          /* if weight array converted --> free*/
  return O_K;
}



INT16 CGEN_PUBLIC CFst_Stp
(
  CFst*  _this,
  CFst*  itSrc,
  INT32   nUnit,
  INT32   nPaths,
  CData* idWeights
)
{
  INT32  nIcW       = -1;
  INT16 nCheck     = 0;
  BOOL  bEpsremove = FALSE;

  CHECK_THIS_RV(NOT_EXEC);
  CFst_Check(_this);
  CFst_Check(itSrc);

  /* Protocol */                                                                /* --------------------------------- */
  IFCHECKEX(1)                                                                  /* On verbose level 1                */
  {                                                                             /* >>                                */
    printf("\n"); dlp_fprint_x_line(stdout,'-',dlp_maxprintcols());             /*   Print separator                 */
    printf("\n CFst_Sdp(");                                                     /*   Print function signature        */
    printf("%s,",BASEINST(_this)->m_lpInstanceName);                            /*   ...                             */
    printf("%s,",BASEINST(itSrc)->m_lpInstanceName);                            /*   ...                             */
    printf("%ld,",(long)nUnit);                                                 /*   ...                             */
    printf("%s)",BASEINST(idWeights)->m_lpInstanceName);                        /*   ...                             */
    printf("\n\n Weigt-SR: %s",CFst_Wsr_GetName(CFst_Wsr_GetType(itSrc,NULL))); /*   Print weight semiring type      */
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

  nCheck     = BASEINST(_this)->m_nCheck;
  bEpsremove = _this->m_bEpsremove;
  CREATEVIRTUAL(CFst,itSrc,_this);
  CFst_Reset(BASEINST(_this),TRUE);
  BASEINST(_this)->m_nCheck = nCheck;
  _this->m_bEpsremove       = bEpsremove;

  /* Copy input and output symbol table */                                      /* --------------------------------- */
  CData_Copy(_this->is,itSrc->is);                                              /* Copy input symbol table           */
  CData_Copy(_this->os,itSrc->os);                                              /* Copy output symbol table          */

  CFst_StpUnit(_this,itSrc,nUnit,nPaths,idWeights);

  DESTROYVIRTUAL(itSrc,_this);
  CFst_Check(_this);                                                            /* TODO: Remove after debugging      */

  /* Protocol */                                                                /* --------------------------------- */
  IFCHECKEX(1)                                                                  /* On verbose level 1                */
  {                                                                             /* >>                                */
    printf("\n\n CFst_Sdp done.");                                              /*   Print function identifier       */
    printf("\n"); dlp_fprint_x_line(stdout,'-',dlp_maxprintcols());             /*   Print separator                 */
    printf("\n");                                                               /*   ...                             */
  }                                                                             /* <<                                */
  return O_K;
}

/*EOF*/
