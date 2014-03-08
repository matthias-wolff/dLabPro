/* dLabPro class CFst (fst)
 * - Unit wiring
 *
 * AUTHOR : Matthias Eichner
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

#define FST_WIRE_MODE_1 1
#define FST_WIRE_MODE_2 2
#define FST_WIRE_MODE_3 3
#define FST_WIRE_MODE_4 4
#define FST_WIRE_MODE_5 5

/* sort comparision function defined in data_wrk.c */
extern int cf_long_up(const void* a, const void* b);
extern int cf_long_down(const void* a, const void* b);

INT16 CGEN_PROTECTED CFst_Wire(CFst* _this, CFst* lpsSrc, CData* lpdBigram)
{
  INT16  bMode   = 0;
  INT32   i       = 0;
  INT32   nUnit   = 0;
  INT32   nUidT   = 0;
  INT32   nRecsB  = 0;
  INT32   nCompsB = 0;
  INT32   nTrans  = 0;
  INT32   nXTrans = 0;
  INT32   nXTrNew = 0;
  INT32   nXTrSiz = 0;
  INT32   nXS     = 0;
  INT32   nRLT    = 0;
  INT32   nSN     = 0;
  INT32   nEN     = 0;
  INT32   nTrcId  =-1;
  INT32   nRmT    = 0;
  INT32   nXSN    = 0;
  INT32   nXEN    = 0;
  BYTE*  bpData  = NULL;
  BYTE*  bpUid   = NULL;
  BYTE*  bpTP    = NULL;
  BYTE*  bpTIS   = NULL;
  BYTE*  bpTOS   = NULL;
  BYTE*  bpTo    = NULL;
  BYTE*  bpFrom  = NULL;
  INT32*  lpSN    = NULL;
  INT32*  lpEN    = NULL;
  INT32*  lpRmT   = NULL;
  FLOAT64 nProb   = 0.0;
  CData* lpAux   = NULL;
  BOOL   bImplct = FALSE;
  INT16  nWsrt   = FST_WSR_NONE;
  INT32   nIcW    = -1;
  char lpsInit[L_INPUTLINE];

  CFst_Check(lpsSrc);
  CFst_Check(_this);

  if(!CData_ReadInitializer(AS(CData,_this->td),lpsInit,L_INPUTLINE,FALSE)) lpsInit[0]='\0';
  bImplct=_this->m_bImplicit;
  CREATEVIRTUAL(CFst,lpsSrc,_this);
  ICREATEEX(CData,lpAux,"~aux",NULL);
  CFst_Copy(BASEINST(_this),BASEINST(lpsSrc));
  _this->m_bImplicit=bImplct;
  nWsrt = CFst_Wsr_GetType(_this,&nIcW);                                        /* Get current weight semiring type  */

  /* determine operation mode from bigram dimension and verify lpdBigram */
  if(lpdBigram)
  {
    nRecsB  = CData_GetNRecs(lpdBigram);
    nCompsB = CData_GetNComps(lpdBigram);
    nTrcId  = CData_FindComp(AS(CData,_this->td),NC_TD_RC);

    if(nRecsB!=nCompsB) return IERROR(_this,FST_BIGRAM,BASEINST(lpdBigram)->m_lpInstanceName,"",0);
    if(nRecsB==UD_XXU(_this)+1) 
    {
      if (nWsrt==FST_WSR_NONE) bMode  = FST_WIRE_MODE_2;
      else                     bMode  = FST_WIRE_MODE_3;
      IFCHECK printf("\n Assuming '%s' is a unit %s (mode=%d).",BASEINST(lpdBigram)->m_lpInstanceName,bMode==FST_WIRE_MODE_2?"wire table":"bigram",bMode==FST_WIRE_MODE_2?2:3);
    }
    else
    {
      if (nWsrt==FST_WSR_NONE) bMode  = FST_WIRE_MODE_4;
      else                     bMode  = FST_WIRE_MODE_5;
      IFCHECK printf("\n Assuming '%s' is a node %s (mode=%d).",BASEINST(lpdBigram)->m_lpInstanceName,bMode==FST_WIRE_MODE_4?"wire table":"bigram",bMode==FST_WIRE_MODE_4?4:5);
    }

    for(i=0;i<nCompsB;i++)
    {
      if(!dlp_is_numeric_type_code(CData_GetCompType(lpdBigram,i)))
      {
        return IERROR(_this,FST_BIGRAM,BASEINST(lpdBigram)->m_lpInstanceName,"",0);
      }
    }

    lpRmT = (INT32*)dlp_calloc(nRecsB*nCompsB,sizeof(INT32));
  }
  else 
  {
    bMode = FST_WIRE_MODE_1;
    IFCHECK printf("\n No bigram is used (mode=1).");
  }

  /* allocate fields for indizes */
  nXSN = nXEN = CData_GetNRecs(AS(CData,_this->sd)) + (_this->m_bImplicit ? 1 : 2);
  lpSN = (INT32*)dlp_malloc(nXSN*sizeof(INT32));
  lpEN = (INT32*)dlp_malloc(nXEN*sizeof(INT32));
  lpSN = (INT32*)dlp_memset(lpSN,-1,nXSN*sizeof(INT32));
  lpEN = (INT32*)dlp_memset(lpEN,-1,nXEN*sizeof(INT32));
  if(!lpSN || !lpEN) return IERROR(_this,ERR_NOMEM,0,0,0);
  lpEN[0] = 0;
  lpSN[_this->m_bImplicit ? 0 : nXSN-1] = 0;

  /* If option /add_uid is set then append uid components to td and sd) */
  if(_this->m_bIndex) 
  {
    CData_AddComp(AS(CData,_this->td),NC_TD_TOS,DLP_TYPE(FST_STYPE));
    nUidT = CData_GetNComps(AS(CData,_this->td))-1;
    CData_Mark(AS(CData,_this->td),nUidT,1);
    ISETOPTION(AS(CData,_this->td),"/mark");
    CData_Fill(AS(CData,_this->td),CMPLX(-1),CMPLX(0));
    IRESETOPTIONS(AS(CData,_this->td));
    CData_Unmark(AS(CData,_this->td));
  }

  nRLT    = CData_GetRecLen(AS(CData,_this->td));
  nXTrans = UD_XXT(_this);

  /* add new start and terminal node */
  IFCHECK printf("\n Wire: add new start and terminal node");
  CData_AddRecs(AS(CData,_this->sd),_this->m_bImplicit?1:2,_this->m_nGrany);
  ISETOPTION(AS(CData,_this->sd),"/rec");
  CData_Shift(AS(CData,_this->sd),AS(CData,_this->sd),1);
  IRESETOPTIONS(AS(CData,_this->sd));
  SD_FLG(_this,_this->m_bImplicit ? 0 : CData_GetNRecs(AS(CData,_this->sd))-1)|=0x01;

  /* collect all units, build Start-End-Node table */
  for(nUnit=0; nUnit<UD_XXU(_this); nUnit++)
  {
    nXS += UD_XS(_this,nUnit);

    for(nTrans=UD_FT(_this,nUnit); nTrans<UD_FT(_this,nUnit)+UD_XT(_this,nUnit); nTrans++)
    {
      /* collect all transistions into one unit */
      TD_INI(_this,nTrans) += UD_FS(_this,nUnit)+1;
      TD_TER(_this,nTrans) += UD_FS(_this,nUnit)+1;

      /* Reset transistion reference counters in mode 3 and 5 */
      if((bMode==FST_WIRE_MODE_3||bMode==FST_WIRE_MODE_3)&&nTrcId>=0)
          *(INT32*)CData_XAddr(AS(CData,_this->td),nTrans,nTrcId) = 0;
      
      /* build Start-End-Node index */
      if(TD_INI(_this,nTrans)==UD_FS(_this,nUnit)+1 && lpSN[TD_INI(_this,nTrans)]<0)
      {
        lpSN[TD_INI(_this,nTrans)]=nUnit+1;
        nSN++;
      }
      if(SD_FLG(_this,TD_TER(_this,nTrans))&0x01 && lpEN[TD_TER(_this,nTrans)]<0)
      {
        SD_FLG(_this,TD_TER(_this,nTrans))=0x00;
        lpEN[TD_TER(_this,nTrans)]=nUnit+1;
        nEN++;
      }

      /* mode 2-5: */
      /* remove transitions from and to zero node which have zero bigram probability */
      /* probability from zero node to unit/node i is taken fom last record of lpdBigram */
      /* probability from unit/node i to zero node is taken fom last component of lpdBigram */
      if(bMode!=FST_WIRE_MODE_1 && (TD_INI(_this,nTrans)==0 || SD_FLG(_this,TD_TER(_this,nTrans))&0x01))
      {
        nProb = CFst_Wsr_NeAdd(nWsrt);

        /* Old version
        if((*(INT32*)bpFrom == 0  && 
            (((bMode==FST_WIRE_MODE_2||bMode==FST_WIRE_MODE_3) && CData_Dfetch(lpdBigram,0,nUnit+1)<=0.0)          ||
             ((bMode==FST_WIRE_MODE_4||bMode==FST_WIRE_MODE_5) && CData_Dfetch(lpdBigram,0,*(INT32*)bpTo)<=0.0))) ||
           (*(INT32*)bpTo == 0    && 
            (((bMode==FST_WIRE_MODE_2||bMode==FST_WIRE_MODE_3) && CData_Dfetch(lpdBigram,nUnit+1,0)<=0.0)         ||
             ((bMode==FST_WIRE_MODE_4||bMode==FST_WIRE_MODE_5) && CData_Dfetch(lpdBigram,*(INT32*)bpFrom,0)<=0.0))))
        {
          IFCHECK printf("\n Queue transition (%ld): %ld -> %ld (%ld) for removal.",(long)nTrans,(long)*(INT32*)bpFrom,(long)*(INT32*)bpTo,nUnit);
          lpRmT[nRmT++] = nTrans;
        }
        */

        if(TD_INI(_this,nTrans)==0)
        {
          if(bMode==FST_WIRE_MODE_2||bMode==FST_WIRE_MODE_3)
            nProb = CData_Dfetch(lpdBigram,0,nUnit+1);
          else if(bMode==FST_WIRE_MODE_4||bMode==FST_WIRE_MODE_5)
            nProb = CData_Dfetch(lpdBigram,0,TD_TER(_this,nTrans));
        }
        else if(SD_FLG(_this,TD_TER(_this,nTrans))&0x01)
        {
          if(bMode==FST_WIRE_MODE_2||bMode==FST_WIRE_MODE_3)
            nProb = CData_Dfetch(lpdBigram,nUnit+1,0);
          else if(bMode==FST_WIRE_MODE_4||bMode==FST_WIRE_MODE_5)
            nProb = CData_Dfetch(lpdBigram,TD_INI(_this,nTrans),0);
        }
        
        if (nProb==CFst_Wsr_NeAdd(nWsrt))
        {
          /* Queue transition for removal */
          IFCHECK printf("\n Queue transition (%ld): %ld -> %ld (%ld) for removal.",
            (long)nTrans,(long)TD_INI(_this,nTrans),(long)TD_TER(_this,nTrans),(long)nUnit);
          lpRmT[nRmT++] = nTrans;
        }
        else
        {
          /* Write transition probability from bigram to transition */
          if(nIcW>0) CData_Dstore(AS(CData,_this->td),nProb,nTrans,nIcW);
        }
      }
    }
  }

  /* adjust unit descriptor */
  CData_Realloc(AS(CData,_this->ud),1);
  UD_FS(_this,0) = 0;
  UD_FT(_this,0) = 0;
  UD_XS(_this,0) = nXS+(_this->m_bImplicit?1:2);
  UD_XT(_this,0) = UD_XXT(_this);

  /* -- Remove transitions -- */
  /* TODO: RemoveTrans is very expensive because of memmove of big memory blocks. Instead condense 
           transition table lpT by moving the remaining memory blocks one by one */

  qsort(lpRmT,nRmT,sizeof(INT32),cf_long_down);

  for(i=0;i<nRmT;i++)
  {
    if(i>0) DLPASSERT(lpRmT[i]!=lpRmT[i-1]); /* Test for multiple occurences of the same transition ID */
    IFCHECK printf("\n Remove trans %ld",(long)lpRmT[i]);
    DLPASSERT(CFst_Deltrans(_this,0,lpRmT[i]));
  }
  nXTrans = CData_GetNRecs(AS(CData,_this->td));

  /* Verify operation mode */
  if((bMode == FST_WIRE_MODE_4||bMode == FST_WIRE_MODE_5) && nRecsB!=nEN+1)
  {
    dlp_free(lpSN);
    dlp_free(lpEN);
    CData_Reset(BASEINST(_this),TRUE);
    DESTROYVIRTUAL(lpsSrc,_this);
    return IERROR(_this,FST_BIGRAM,BASEINST(lpdBigram)->m_lpInstanceName,"node",0);
  }

  /***********************************************************************************/
  /* connect former units with new start and terminal node and units with each other */
  /***********************************************************************************/

  /* init new transition table size and number of new transistions */
  nXTrNew = nXTrSiz = 0;

  /* add new transitions => loop over all transitions from old end to old start state */
  for(nEN=0;nEN<nXEN;nEN++) if(lpEN[nEN]>=0) for(nSN=0;nSN<nXSN;nSN++) if(lpSN[nSN]>=0)
  {
    if(bMode==FST_WIRE_MODE_1 && nEN!=0 && nSN!=(_this->m_bImplicit?0:nXSN-1)) continue;
    /* skip transitions having zero bigram probability */
    if((bMode==FST_WIRE_MODE_2||bMode==FST_WIRE_MODE_3) && (nProb=CData_Dfetch(lpdBigram,lpEN[nEN],lpSN[nSN]))==CFst_Wsr_NeAdd(nWsrt)) continue;
    if((bMode==FST_WIRE_MODE_4||bMode==FST_WIRE_MODE_5) && (nProb=CData_Dfetch(lpdBigram,nEN,nSN))==CFst_Wsr_NeAdd(nWsrt))             continue;

    /* resize transition table */
    if(nXTrNew>=nXTrSiz){
      nXTrSiz+=_this->m_nGrany*100;
      DLPASSERT(CData_Realloc(AS(CData,_this->td),nXTrans+nXTrSiz)==O_K);

      /* re-init pointer to first new transition */
      bpTo   = CData_XAddr(AS(CData,_this->td),nXTrans,IC_TD_TER);
      bpFrom = CData_XAddr(AS(CData,_this->td),nXTrans,IC_TD_INI);
      bpData = CData_XAddr(AS(CData,_this->td),nXTrans,IC_TD_DATA);
      bpUid  = CData_XAddr(AS(CData,_this->td),nXTrans,nUidT);
      bpTP   = CData_XAddr(AS(CData,_this->td),nXTrans,nIcW);
      bpTIS  = CData_XAddr(AS(CData,_this->td),nXTrans,CData_FindComp(AS(CData,_this->td),NC_TD_TIS));
      bpTOS  = CData_XAddr(AS(CData,_this->td),nXTrans,CData_FindComp(AS(CData,_this->td),NC_TD_TOS));
      DLPASSERT(bpTo!=NULL);
      DLPASSERT(bpFrom!=NULL);
    }
    nXTrNew++;

    *(FST_ITYPE*)bpFrom = nEN;
    *(FST_ITYPE*)bpTo   = nSN;
    if(bpTIS!=NULL) *(FST_STYPE*)bpTIS  = -1;
    if(bpTOS!=NULL) *(FST_STYPE*)bpTOS  = -1;
    
    /* write uid for all (new) exit transitions */
    if(_this->m_bIndex) *(INT32*)bpUid = lpEN[nEN];

    if(bpTP!=NULL)
    {
      /* write bigram probability in mode 3 and 5*/
      if((bMode==FST_WIRE_MODE_3||bMode==FST_WIRE_MODE_5)) *(FLOAT64*)bpTP = nProb;
      /* else initialize with neutral element */
      else *(FLOAT64*)bpTP = CFst_Wsr_NeMult(nWsrt);
    }

    CData_IncNRecs(AS(CData,_this->td),1);
    IFCHECK 
    {
      if (bMode!=FST_WIRE_MODE_3&&bMode!=FST_WIRE_MODE_5) printf("\n Wire: Added transition %ld->%ld",(long)*(INT32*)bpFrom,(long)*(INT32*)bpTo);
      else                                                printf("\n Wire: Added transition %ld->%ld with probability=%g",(long)*(INT32*)bpFrom,(long)*(INT32*)bpTo,(double)nProb);
    }
    
    bpFrom += nRLT; 
    bpTo   += nRLT; 
    bpData += nRLT; 
    bpUid  += nRLT;
    if (bpTIS) bpTIS += nRLT;
    if (bpTOS) bpTOS += nRLT;
    if (bpTP ) bpTP  += nRLT;
  }

  /* adjust unit descriptor */
  CData_Dstore(AS(CData,_this->ud),CData_GetNRecs(AS(CData,_this->td)),0,IC_UD_XT);

  /* Recalc transition probabilities */
  /* TODO: Implement for FST depending on type of weigths */
  /* 2004-09-03 MWX: --> *//* if (CData_FindComp(_this->td,NC_TD_PSR)>=0) *//* <--?? *//* CStr_CalcTp(_this,0); */

  /* Initialize transition data */
  if(lpsInit[0]){
    INT32 nCtr    = 0;
    INT32 nFT     = 0;
    INT32 nXT     = 0;
    INT32 nTrans  = nXTrans;
    INT32 nCount  = UD_XT(_this,0)-nXTrans;

    nFT  = UD_FT(_this,0);
    nXT  = UD_XT(_this,0);
    for (nCtr=0; nTrans+nCtr<nFT+nXT && nCtr<nCount; nCtr++)
      IF_NOK(CData_InitializeRecordEx(AS(CData,_this->td),lpsInit,nTrans+nCtr,IC_TD_DATA))
        return IERROR(_this,FST_INTERNAL,__FILE__,__LINE__,"");
  }

  /* cleanup */
  dlp_free(lpSN);
  dlp_free(lpEN);
  dlp_free(lpRmT);

  DESTROYVIRTUAL(lpsSrc,_this);
  IDESTROY(lpAux);

  CFst_Check(_this); /* Delete after debugging */

  return O_K;
}


/* ----------------------------------------------------------------------------------------------------------------- */

/*
 * NOTE:    Method looses qualification of left-unconnected start states.
 * Options: /index /push
 */
INT16 CGEN_PUBLIC CFst_Union(CFst* _this, CFst* itSrc)
{
  INT32          nU     = 0;                                                   /* Current unit                       */
  FST_ITYPE     nT     = 0;                                                    /* Current transition                 */
  FST_ITYPE     nS0    = 0;                                                    /* State offset                       */
  FST_STYPE     nTos   = -1;                                                   /* Current output symbol              */
  INT32          nIcTos = -1;                                                  /* Component index of output symbols  */
  BOOL          bDelS0 = FALSE;                                                /* Start state of unit was deleted    */
  BOOL          bIndex = FALSE;                                                /* Copy of _this->m_bIndex option     */
  BOOL          bPush  = FALSE;                                                /* Copy of _this->m_bPush option      */
  FST_TID_TYPE* lpTI   = NULL;                                                 /* Iterator of source automaton       */

  /* Validation */
  CHECK_THIS_RV(NOT_EXEC);
  CFst_Check(_this);
  CFst_Check(itSrc);
  bIndex = _this->m_bIndex;
  bPush  = _this->m_bPush;

  /* Initialize - NO RETURNS BEYOND THIS POINT! */
  CREATEVIRTUAL(CFst,itSrc,_this);
  CFst_Copy(BASEINST(_this),BASEINST(itSrc));

  /* Copy input and output symbol table */                                      /* --------------------------------- */
  CData_Copy(_this->is,itSrc->is);                                              /* Copy input symbol table           */
  CData_Copy(_this->os,itSrc->os);                                              /* Copy output symbol table          */

  /* Add unit index as output symbol */
  if (bIndex)
  {
    nIcTos = CData_FindComp(AS(CData,_this->td),NC_TD_TOS);
    if (nIcTos<0)
    {
      CData_AddComp(AS(CData,_this->td),NC_TD_TOS,DLP_TYPE(FST_STYPE));
      nIcTos = CData_FindComp(AS(CData,_this->td),NC_TD_TOS);
    }
    DLPASSERT(nIcTos>0);

    for (nU=0; nU<UD_XXU(_this); nU++)
      for (nT=UD_FT(_this,nU); nT<UD_FT(_this,nU)+UD_XT(_this,nU); nT++)
      {
        nTos = -1;
        if ( bPush && SD_FLG(_this,TD_TER(_this,nT)+UD_FS(_this,nU))&SD_FLG_FINAL) nTos = (FST_STYPE)nU;
        if (!bPush && TD_INI(_this,nT)==0) nTos = (FST_STYPE)nU;
        CData_Dstore(AS(CData,_this->td),(FLOAT64)nTos,nT,nIcTos);
      }
  }

  /* Add a new start state */
  CData_InsertRecs(AS(CData,_this->sd),0,1,_this->m_nGrany);
  nS0 = 1;

  /* Collect units */
  CData_Realloc(AS(CData,_this->ud),1);
  UD_XS(_this,0)=UD_XXS(_this);
  UD_XT(_this,0)=UD_XXT(_this);

  /* Loop over former units */
  for (nU=0; nU<UD_XXU(itSrc); nU++)
  {
    lpTI = CFst_STI_Init(itSrc,nU,0);

    /* Delete the start state if no transitions end there */
    bDelS0 = FALSE;
    if (CFst_STI_TtoS(lpTI,0,NULL)==NULL)
    {
      CData_DeleteRecs(AS(CData,_this->sd),lpTI->nFS+nS0,1);
      nS0--;
      bDelS0 = TRUE;
    }

    /* Detour transitions */
    for (nT=lpTI->nFT; nT<lpTI->nFT+lpTI->nXT; nT++)
    {
      if (TD_INI(_this,nT)!=0 || !bDelS0) TD_INI(_this,nT)+=(lpTI->nFS+nS0);
      if (TD_TER(_this,nT)!=0 || !bDelS0) TD_TER(_this,nT)+=(lpTI->nFS+nS0);
    }

    /* Connect new start state */
    if (!bDelS0) CFst_Addtrans(_this,0,0,lpTI->nFS+nS0);

    CFst_STI_Done(lpTI);
  }

  /* Finish and check result */
  UD_XS(_this,0)=UD_XXS(_this);
  DESTROYVIRTUAL(itSrc,_this);
  CFst_Check(_this);
  return O_K;
}

/**
 * <p>Kleene closure of one unit. There are no checks performed.</p>
 * <h3>Note</h3>
 * <p>Closed automaton may stop at start state after closure!</p>
 *
 * @param _this Pointer to this (destination) automaton instance
 * @param itSrc Pointer to source automaton instance
 * @param nUnit Index of unit to determinize
 * @return O_K if successfull, a (negative) error code otherwise
 * @see Close CFst_Close
 */
INT16 CGEN_PROTECTED CFst_CloseUnit(CFst* _this, CFst* itSrc, INT32 nUnit)
{
  FST_ITYPE nFin   = -1;                                                       /* The unified final state           */
  FST_ITYPE nT     = 0;                                                        /* Current transition                */
  BOOL      bLocal;

  DLPASSERT(_this!=itSrc);

  /* Initialize destination */
  bLocal=_this->m_bLocal;
  CFst_CopyUi(_this,itSrc,NULL,nUnit);
  _this->m_bLocal=bLocal;
  CFst_Fsunify(_this,0);
  nFin = UD_XS(_this,0)-1;

  /* Detour transitions to unified final state to start state */
  for (nT=0; nT<UD_XT(_this,0); nT++)
    if (TD_TER(_this,nT)==nFin)
      TD_TER(_this,nT)=0;

  /* Finish */
  CFst_Delstate(_this,0,nFin);
  SD_FLG(_this,0)|=SD_FLG_FINAL;

  return O_K;
}

/*
 *
 */
INT16 CGEN_PUBLIC CFst_Close(CFst* _this, CFst* itSrc, INT32 nUnit)
{
  CFst* itUnit = NULL;                                                         /* Current unit                      */
  INT32 nU     = 0;                                                            /* Current unit index                */
  BOOL  bLocal;

  /* Validation */
  CHECK_THIS_RV(NOT_EXEC);
  CFst_Check(_this);
  CFst_Check(itSrc);
  if (nUnit>=UD_XXU(itSrc)) return IERROR(_this,FST_BADID,"unit",nUnit,0);

  /* NO RETURNS BEYOND THIS POINT! */
  bLocal=_this->m_bLocal;
  CREATEVIRTUAL(CFst,itSrc,_this);
  CFst_Reset(BASEINST(_this),TRUE);
  if (UD_XXU(itSrc)>1) { ICREATEEX(CFst,itUnit,"CFst_Closure~itUnit",NULL); }
  else                 itUnit = _this;

  if (BASEINST(_this)->m_nCheck>BASEINST(itUnit)->m_nCheck) BASEINST(itUnit)->m_nCheck=BASEINST(_this)->m_nCheck;
  if (BASEINST(itSrc)->m_nCheck>BASEINST(itUnit)->m_nCheck) BASEINST(itUnit)->m_nCheck=BASEINST(itSrc)->m_nCheck;
  itUnit->m_bLocal = bLocal;

  /* Copy input and output symbol table */                                      /* --------------------------------- */
  CData_Copy(_this->is,itSrc->is);                                              /* Copy input symbol table           */
  CData_Copy(_this->os,itSrc->os);                                              /* Copy output symbol table          */
  
  /* Kleene closure of units */
  for (nU=0; nU<UD_XXU(itSrc); nU++)
  {
    if (nU==nUnit || nUnit<0)
      CFst_CloseUnit(itUnit,itSrc,nU);
    else
      CFst_CopyUi(itUnit,itSrc,NULL,nU);

    if (UD_XXU(itSrc)>1) CFst_Cat(_this,itUnit);
  }

  /* Clean up */
  if (UD_XXU(itSrc)>1) IDESTROY(itUnit);
  DESTROYVIRTUAL(itSrc,_this);
  CFst_Check(_this);                                                           /* TODO: Remove after debugging      */
  return O_K;
}

/* EOF */
