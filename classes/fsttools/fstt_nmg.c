/* dLabPro class CFsttools (fsttools)
 * - optimization methods
 *
 * AUTHOR : Frank Duckhorn
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
#include "dlp_fsttools.h"

#define S_BACKOFF   2

INT32 find_state(FST_TID_TYPE* lpTI,INT32 *lpHist,INT32 nNHist)
{
  INT32 nS = 0;
  while(nNHist>0){
    BYTE* lpT = NULL;
    while((lpT=CFst_STI_TfromS(lpTI,nS,lpT))!=NULL){
      if(*CFst_STI_TTis(lpTI,lpT)==lpHist[0]) break;
    }
    if(!lpT) return -1;
    nS = *CFst_STI_TTer(lpTI,lpT);
    nNHist--;
    lpHist++;
  }
  return nS;
}

/*
 * Manual page at fst.def
 */
INT16 CGEN_PUBLIC CFsttools_Nmg2lm(CFsttools* _this,CFst* itSrc,INT32 nUnit,INT32 nNGram,CFst* itDst)
{
  struct state {
    struct state *lpNext;
    INT32         nS,nN;
  }            *lpStates, **lpStatesEnd;
  FST_TID_TYPE* lpTI;
  INT32*        lpHist;

  /* Validation */
  CHECK_THIS_RV(NOT_EXEC);
  CFst_Check(itSrc);
  if (nNGram<=0 || nUnit<0 || nUnit>=UD_XXU(itSrc)) return NOT_EXEC;

  /* Prepare itDst */
  CFst_CopyUi(itDst,itSrc,NULL,nUnit);

  /* Init iteration */
  lpTI = CFst_STI_Init(itDst,nUnit,FSTI_SORTINI);
  lpStates = (struct state *)dlp_malloc(sizeof(struct state));
  lpStates->lpNext = NULL;
  lpStates->nS = lpStates->nN = 0;
  lpStatesEnd = &lpStates->lpNext;
  lpHist = (INT32*)dlp_calloc(UD_XS(itDst,0),(nNGram+1)*sizeof(INT32));

  /* Loop until we have some states to do */
  while(lpStates){
    BYTE*         lpT = NULL;
    struct state* lpS;
    INT32*        lpLHist;

    /* Remove frist state */
    lpS=lpStates;
    lpStates=lpStates->lpNext;
    if(!lpStates) lpStatesEnd=&lpStates;
    lpLHist=lpHist+lpS->nS*(nNGram+1);

    /* Loop over leaving transitions */
    while((lpT=CFst_STI_TfromS(lpTI,lpS->nS,lpT))!=NULL){
      INT32         nTis = *CFst_STI_TTis(lpTI,lpT);
      struct state* lpS2 = (struct state *)dlp_malloc(sizeof(struct state));
      INT32*        lpLHist2;
      lpS2->nS = *CFst_STI_TTer(lpTI,lpT);
      lpS2->nN = lpS->nN+1;
      lpLHist2 = lpHist+lpS2->nS*(nNGram+1);
      memcpy(lpLHist2,lpLHist,(lpLHist[0]+1)*sizeof(INT32));
      lpLHist2[++lpLHist2[0]]=nTis;

      /* Change transitions terminal state or add terminal state to list */
      if(nTis==S_BACKOFF || lpLHist[0]>=nNGram-1){
        *CFst_STI_TTer(lpTI,lpT) = find_state(lpTI,lpLHist2+2,lpLHist2[0]- (nTis==S_BACKOFF ? 2 : 1) );
        dlp_free(lpS2);
      }else{
        *lpStatesEnd=lpS2;
        lpStatesEnd=&lpS2->lpNext;
      }
    }

    dlp_free(lpS);
  }

  CFst_TrimStates(itDst,0);

  /* CleanUp */
  CFst_STI_Done(lpTI);
  dlp_free(lpHist);

  return O_K;
}

/*
 * Manual page at fst.def
 */
INT16 CGEN_PUBLIC CFsttools_LmAddInputTrans(CFsttools* _this,CFst* itLM,INT32 nUnit)
{
  INT32         nS,nT;
  INT32         nIcTis;
  INT32         nIcTos;
  FST_TID_TYPE* lpTI;
  FLOAT64       nW;
  INT32*        lpAdd;
  INT32         nAdd;

  /* Validation */
  CHECK_THIS_RV(NOT_EXEC);
  CFst_Check(itLM);
  if (nUnit<0 || nUnit>=UD_XXU(itLM)) return NOT_EXEC;

  /* Init */
  nIcTis = CData_FindComp(AS(CData,itLM->td),NC_TD_TIS);
  nIcTos = CData_FindComp(AS(CData,itLM->td),NC_TD_TOS);
  if(nIcTis<0) return NOT_EXEC;
  if(nIcTos<0){
    CData_SetCname(AS(CData,itLM->td),nIcTis,NC_TD_TOS);
    CData_InsertComp(AS(CData,itLM->td),NC_TD_TIS,T_LONG,nIcTis);
  }
  for(nT=(nIcTos<0?0:UD_FT(itLM,nUnit)) ; nT<(nIcTos<0?UD_XXT(itLM):UD_FT(itLM,nUnit)+UD_XT(itLM,nUnit)) ; nT++)
      CData_Dstore(AS(CData,itLM->td),-1.,nT,nIcTis);
  lpTI = CFst_STI_Init(itLM,nUnit,FSTI_SORTTER);
  lpAdd=(INT32*)dlp_malloc(UD_XS(itLM,nUnit)*2*sizeof(INT32));
  nAdd=0;

  /* Loop over states */
  for(nS=0;nS<UD_XS(itLM,nUnit);nS++){
    BYTE* lpT = NULL;
    INT32 nTos = -1;
    while((lpT=CFst_STI_TtoS(lpTI,nS,lpT))!=NULL){
      INT32 nTos1 = *CFst_STI_TTos(lpTI,lpT);
      if(nTos1==S_BACKOFF) continue;
      if(nTos<0) nTos=nTos1;
      else if(nTos!=nTos1) return NOT_EXEC;
      *CFst_STI_TTer(lpTI,lpT)=UD_XS(itLM,nUnit)+nAdd;
    }
    if(nTos>=0){
      lpAdd[nAdd*2]=nS;
      lpAdd[nAdd*2+1]=nTos;
      nAdd++;
    }
  }

  /* CleanUp */
  CFst_STI_Done(lpTI);

  /* Add new states and transitions */
  CFst_Addstates(itLM,nUnit,nAdd,0);
  nW=CFst_Wsr_NeMult(CFst_Wsr_GetType(itLM,NULL));
  for(nAdd--,nS=UD_XS(itLM,nUnit);nAdd>=0;nAdd--)
    CFst_AddtransEx(itLM,nUnit,--nS,lpAdd[nAdd*2],lpAdd[nAdd*2+1],-1,nW);

  /* CleanUp */
  dlp_free(lpAdd);

  return O_K;
}

INT16 CGEN_PUBLIC CFsttools_RemoveMl2(CFsttools* _this,CFst* itR)
{
  char *lpSLoop;
  char *lpDelT;
  FST_TID_TYPE* lpTI;
  BYTE* lpT;
  INT32 nCTMS1;
  INT32 nOffTMS1;
  INT32 nOffTMS2;
  INT32 nDel=0;
  INT32 nT;

  nCTMS1 = CData_FindComp(AS(CData,itR->td),"~TMS");
  nOffTMS1 = CData_GetCompOffset(AS(CData,itR->td),nCTMS1);
  nOffTMS2 = CData_GetCompOffset(AS(CData,itR->td),nCTMS1+1);

  lpSLoop=(char *)dlp_calloc(CData_GetNRecs(AS(CData,itR->sd)),sizeof(char));
  lpTI = CFst_STI_Init(itR,0,FSTI_SORTINI);
  for(lpT=NULL;(lpT=CFst_STI_TfromS(lpTI,-1,lpT))!=NULL;){
    INT32 nIni =*CFst_STI_TIni(lpTI,lpT);
    INT32 nTer =*CFst_STI_TTer(lpTI,lpT);
    INT32 nTMS1=*((INT32*)(lpT+nOffTMS1));
    INT32 nTMS2=*((INT32*)(lpT+nOffTMS2));
    if(nIni!=nTer) continue;
    if(nTMS1==99997 && nTMS2==99998) lpSLoop[nIni]=1;
    if(nTMS1==99998 && nTMS2==99997) lpSLoop[nIni]=1;
  }

  lpDelT=(char *)dlp_calloc(CData_GetNRecs(AS(CData,itR->td)),sizeof(char));
  for(lpT=NULL;(lpT=CFst_STI_TfromS(lpTI,-1,lpT))!=NULL;){
    INT32 nIni =*CFst_STI_TIni(lpTI,lpT);
    INT32 nTer =*CFst_STI_TTer(lpTI,lpT);
    INT32 nTMS1=*((INT32*)(lpT+nOffTMS1));
    INT32 nTMS2=*((INT32*)(lpT+nOffTMS2));
    BYTE *lpT2;
    if(nTMS1!=99998 || nTMS2!=99998) continue;
    if(lpSLoop[nIni] && lpSLoop[nTer]) continue;
    nDel++;
    lpDelT[CFst_STI_GetTransId(lpTI,lpT)]=1;
    for(lpT2=NULL;(lpT2=CFst_STI_TfromS(lpTI,nTer,lpT2))!=NULL;){
      *CFst_STI_TIni(lpTI,lpT2)=nIni;
      if(*CFst_STI_TTer(lpTI,lpT2)==nTer) *CFst_STI_TTer(lpTI,lpT2)=nIni;
    }
  }
//  printf("\nDel: %i\n",nDel);

  CFst_STI_Done(lpTI);
  dlp_free(lpSLoop);

  for(nT=CData_GetNRecs(AS(CData,itR->td))-1;nT>=0;nT--)
    if(lpDelT[nT]) CFst_Deltrans(itR,0,nT);

  dlp_free(lpDelT);

  CFst_TrimStates(itR,0);

  return O_K;
}
