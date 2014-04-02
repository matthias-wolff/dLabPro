/* dLabPro class CFst (fst)
 * - MW's PLAYGROUND - DO NOT TOUCH, DO NOT USE! - or you'll be doomed...
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

#define IFDOPRINT(A,B) \
  if (BASEINST(_this)->m_nCheck>=A && ((_this->m_nPrintstop>=0 && _this->m_nPrintstop<=B) || _this->m_nPrintstop==-2))

/*
 * EXPERIMENTAL!
 */
INT16 CGEN_PUBLIC CFst_X1(CFst* _this, CFst* itSrc, INT32 nUnit)
{
#if 0
  FST_TID_TYPE* lpTIsrc = NULL;
  BYTE*         lpTsrc  = NULL;                                                /* Pointer to current source trans.   */
  FST_WTYPE*    lpWBrd  = NULL;                                                /* Weight read layer buffer           */
  FST_WTYPE*    lpWBwr  = NULL;                                                /* Weight write layer buffer          */
  FST_WTYPE*    lpWBsp  = NULL;                                                /* Weight layer buffer swap pointer   */
  BYTE*         lpLBrd  = NULL;                                                /* State presence read layer buffer   */
  BYTE*         lpLBwr  = NULL;                                                /* State presence write layer buffer  */
  BYTE*         lpLBsp  = NULL;                                                /* State presence layer buf. swap ptr.*/
  FST_ITYPE*    lpDS    = NULL;                                                /* Destination state map              */
  FST_WTYPE     nNeAdd  = 0.;                                                  /* Neutral element of addition        */
  FST_WTYPE     nNeMult = 0.;                                                  /* Neutral element of multiplication  */
  FST_ITYPE     nIni0   = 0;                                                   /* Root state of layer traversal      */
  FST_ITYPE     nTer    = 0;                                                   /* Terminal state of cur. src. trans. */
  FST_STYPE     nTis    = 0;                                                   /* Input symbol of current src. trans.*/
  FST_STYPE     nTos    = 0;                                                   /* Output symbol of cur. src. trans.  */
  BOOL          bFin    = FALSE;                                               /* Terminal state of cur.trans. final */
  FST_ITYPE     nS      = 0;                                                   /* Current source state               */
  FST_WTYPE     nW      = 0.;                                                  /* Weight of current transition       */
  FST_WTYPE     nWa     = 0.;                                                  /* Current aggregated weight          */
  INT32          nL      = 0;                                                   /* Current layer                      */
  INT32          nB      = 0;                                                   /* Current beam width                 */
  INT32          nGrany  = 0;                                                   /* Copy of dest. allocation grany     */
  INT32          nMaxLen = 0;                                                   /* Copy of dest. max. path length     */
  FST_WTYPE     nWceil  = 0.;                                                  /* Copy of dest. ceiling weight       */
  INT16         nCheck  = FALSE;                                               /* Copy of destination check level    */
  INT32          nCtr[]  = {0,0,0,0,0,0};                                       /* Event counters                     */

  /* Validate */
  CHECK_THIS_RV(NOT_EXEC);
  nGrany  = _this->m_nGrany;
  nMaxLen = _this->m_nMaxLen;
  nWceil  = _this->m_nWceil;
  nCheck  = BASEINST(_this)->m_nCheck;
  /*CREATEVIRTUAL(CFst,itSrc,_this);                          / * == NO RETURNS BEYOND THIS POINT == */
  if (_this == itSrc) 
  { 
    ICREATEEX(CFst,_this,"#",NULL); 
    if (!_this) return IERROR(_this,ERR_NOMEM,0,0,0); 
  } 
  else if(BASEINST(_this)->m_lpInstanceName[0]=='#') 
  { 
    DLPASSERT(dlp_strlen(BASEINST(_this)->m_lpInstanceName)<L_NAMES-2); 
    dlp_strcat(BASEINST(_this)->m_lpInstanceName,"."); 
  }

  /* Protocol */
  IFCHECK
  {
    printf("\n"); dlp_fprint_x_line(stdout,'-',dlp_maxprintcols());
    printf("\n CFst_X1(%s,%s,%ld)\n",BASEINST(_this)->m_lpInstanceName,BASEINST(itSrc)->m_lpInstanceName,(long)nUnit);
  }

  /* Initialize destination */
  CFst_CopyUi(_this,itSrc,NULL,nUnit);                                         /* Copy source unit                   */
  CData_Clear(AS(CData,_this->sd)); CData_SetNRecs(AS(CData,_this->sd),0);     /* Clear destination state table      */
  CData_Clear(AS(CData,_this->td)); CData_SetNRecs(AS(CData,_this->td),0);     /* Clear destination transition table */
  UD_XS(_this,0)   = 0;                                                        /* Adjust destination state count     */
  UD_XT(_this,0)   = 0;                                                        /* Adjust destination trans. count    */
  BASEINST(_this)->m_nCheck  = nCheck;
  _this->m_nMaxLen = nMaxLen;
  _this->m_nWceil  = nWceil;
  _this->m_nGrany  = nGrany>UD_XT(itSrc,nUnit)?nGrany:UD_XT(itSrc,nUnit);
  _this->m_nIcTis  = CData_FindComp(AS(CData,_this->td),NC_TD_TIS);
  _this->m_nIcTos  = CData_FindComp(AS(CData,_this->td),NC_TD_TOS);
  _this->m_nWsr    = CFst_Wsr_GetType(_this,&_this->m_nIcW);

  IFCHECK
  {
    printf("\n - Weight distance limit     : %8lg",_this->m_nWceil );
    printf("\n - Topological distance limit: %8ld",_this->m_nMaxLen);
    printf("\n - Allocation granularity    : %8ld",_this->m_nGrany );
    printf("\n"); dlp_fprint_x_line(stdout,'-',dlp_maxprintcols());
    printf("\n");
  }

  /* Initialize */
  nNeAdd  = CFst_Wsr_NeAdd (_this->m_nWsr);
  nNeMult = CFst_Wsr_NeMult(_this->m_nWsr);
  lpTIsrc = CFst_STI_Init(itSrc,nUnit,FSTI_SORTINI);
  lpWBwr  = (FST_WTYPE*)dlp_calloc(lpTIsrc->nXS,sizeof(FST_WTYPE));
  lpWBrd  = (FST_WTYPE*)dlp_calloc(lpTIsrc->nXS,sizeof(FST_WTYPE));
  lpLBwr  = (BYTE*     )dlp_calloc(lpTIsrc->nXS,sizeof(BYTE     ));
  lpLBrd  = (BYTE*     )dlp_calloc(lpTIsrc->nXS,sizeof(BYTE     ));
  lpDS    = (FST_ITYPE*)dlp_calloc(lpTIsrc->nXS,sizeof(FST_ITYPE));
  for (nS=0; nS<lpTIsrc->nXS; nS++) lpDS[nS] = -1;
  lpDS[0] = 0;
  CFst_Cps_NewState(_this,SD_FLG(itSrc,lpTIsrc->nFS)&SD_FLG_FINAL);

  /* Loop over all states */
  for (nIni0=0; nIni0<lpTIsrc->nXS; nIni0++)
  {
    /* Only process destination states */
    if (lpDS[nIni0]<0) continue;

    /* Initialize layer traversal of itSrc starting at nIni0 */
    for (nS=0; nS<lpTIsrc->nXS; nS++) { lpLBrd[nS]=0; lpWBrd[nS]=nNeAdd; }     /* Clear read layer buffers           */
    lpLBrd[nIni0] = 1;                                                         /* Start traversal at state nIni0     */
    lpWBrd[nIni0] = nNeMult;                                                   /* Start aggregated weight is "0"     */
    nB            = 1;                                                         /* Start beam width is 1              */
    IFCHECKEX(1) {                                                             /* Protocol                           */
      printf("\n   Traversing from state %ld:",(long)nIni0);
      nCtr[0]++;
    }

    /* Do Layer traversal of itSrc starting at nIni0 */
    for (nL=0; nL<_this->m_nMaxLen && nB>0; nL++)                              /* >> Layer loop >>                   */
    {
      IFCHECKEX(1) nCtr[1]++;                                                  /* Count layers                       */
      IFCHECKEX(2) printf("\n     nL=%ld:",(long)nL);                          /* Protocol                           */
      for (nS=0; nS<lpTIsrc->nXS; nS++) { lpLBwr[nS]=0; lpWBwr[nS]=nNeAdd; }   /* Clear write layer buffers          */

      /* Walk one step */
      for (nS=0,nB=0; nS<lpTIsrc->nXS; nS++)                                   /* For all source states ...          */
        if (lpLBrd[nS])                                                        /*  ... living on current layer       */
        {
          IFCHECKEX(1   ) nCtr[2]++;                                           /* Count enumerated states            */
          IFDOPRINT(3,nL) printf("\n       nS=%ld",(long)nS);                  /* Protocol                           */

          for                                                                  /* For all outgoing transitions       */
          (
            lpTsrc = CFst_STI_TfromS(lpTIsrc,nS,NULL);                         /* First transition leaving nS        */
            lpTsrc;                                                            /* ==NULL --> no more transitions     */
            lpTsrc = CFst_STI_TfromS(lpTIsrc,nS,lpTsrc)                        /* Next transition leaving nS         */
          )
          {
            nTer = *CFst_STI_TTer(lpTIsrc,lpTsrc);                             /* Get terminal state of transition   */
            nTis = *CFst_STI_TTis(lpTIsrc,lpTsrc);                             /* Get input symbol of transition     */
            nTos = *CFst_STI_TTos(lpTIsrc,lpTsrc);                             /* Get output symbol of transition    */
            nW   = *CFst_STI_TW  (lpTIsrc,lpTsrc);                             /* Get transition weight              */
            nWa  = CFst_Wsr_Op(_this,nW,lpWBrd[nS],OP_MULT);                   /* Get aggregated transition weight   */
            bFin = SD_FLG(itSrc,lpTIsrc->nFS+nTer)&SD_FLG_FINAL;               /* Is terminal state final?           */

            IFCHECKEX(1) nCtr[3]++;                                            /* Count enumerated transitions       */
            IFDOPRINT(3,nL)                                                    /* Protocol                           */
              printf("\n         %ld (w=%5G) --(%ld:%ld/%G[%G])--> %ld (w=%G)",
              (long)nS,(double)lpWBrd[nS],(long)nTis,(long)nTos,(double)nW,(double)nWa,(long)nTer,(double)lpWBwr[nS]);

            if (CFst_Wsr_Op(_this,nWa,_this->m_nWceil,OP_LESS)>0.)             /* Check path weight                  */
            {
              IFCHECKEX(1) nCtr[4]++;                                          /* Count ceiling break-throughs       */
              IFDOPRINT(3,nL) printf(" beyond ceiling --> TERMINATE PATH");
              continue;
            }

            if ((/*nTis<0 &&*/ nTos<0) && !bFin)                               /* Epsilon and not final transition?  */
            {                                                                  /*  ... YES: go on traversing         */
              nWa = CFst_Wsr_Op(_this,lpWBwr[nTer],nWa,OP_ADD);                /* Aggregate terminal state weight    */
              if (lpLBwr[nTer]==0) nB++;                                       /* Track beam width                   */
              lpLBwr[nTer] = 1;                                                /* Term. state lives on next layer    */
              lpWBwr[nTer] = nWa;                                              /* Store aggregated state weight      */
              IFDOPRINT(3,nL) printf(" newweight=%G",(double)lpWBwr[nTer]);    /* Protocol                           */
            }
            else                                                               /*  ... NO:                           */
            {
              if (lpDS[nTer]<0)
              {
                lpDS[nTer]=CData_GetNRecs(AS(CData,_this->sd));                /* Indicate destination state         */
                CFst_Cps_NewState(_this,bFin);                                 /* Add destination state              */
              }

              CFst_Cps_NewTrans(_this,lpDS[nIni0],lpDS[nTer],nTis,nTos,nWa);   /* Create destination transition      */
              IFDOPRINT(3,nL)                                                  /* Protocol                           */
                printf(" --> Add trans. %ld --(%ld:%ld/%5g)--> %ld",
                (long)lpDS[nIni0],(long)nTis,(long)nTis,(double)nWa,(long)lpDS[nTer]);
            }
          }
        }      

      /* Finish layer */
      lpLBsp = lpLBrd; lpLBrd = lpLBwr; lpLBwr = lpLBsp;                       /* Swap layer buffers                 */
      lpWBsp = lpWBrd; lpWBrd = lpWBwr; lpWBwr = lpWBsp;                       /* Swap layer buffers                 */
      IFDOPRINT(3,nL) printf("\n      ");                                      /* Protocol                           */
      IFCHECKEX(2   ) printf(" beamwidth=%ld",(long)nB);
      IFDOPRINT(3,nL)                                                          /* Break protocol?                    */
      {
        printf("\n    continue <cr>, no print -1, nonstop -2: ");
        dlp_getx(T_SHORT,&_this->m_nPrintstop);
      }
    }                                                                          /* << Layer loop <<                   */

    IFCHECKEX(1) nCtr[5]+=nB;                                                  /* Count paths with overlength        */
    IFCHECKEX(2) printf("\n    ") ;                                            /* Protocol                           */
    IFCHECKEX(1)                                                               /* Protocol                           */
      printf(" nXL=%ld, nXT=%ld/%ld",
      (long)nL,(long)CData_GetNRecs(AS(CData,_this->td)),(long)CData_GetMaxRecs(AS(CData,_this->td)));
  }

  /* Finish destination */
  UD_XS(_this,0) = CData_GetNRecs(AS(CData,_this->sd));                        /* Adjust state count                 */
  UD_XT(_this,0) = CData_GetNRecs(AS(CData,_this->td));                        /* Adjust transition count            */

  /* Clean up */
  dlp_free(lpDS  );
  dlp_free(lpLBrd);
  dlp_free(lpLBwr);
  dlp_free(lpWBrd);
  dlp_free(lpWBwr);
  CFst_STI_Done(lpTIsrc);
  DESTROYVIRTUAL(itSrc,_this);
  _this->m_nGrany = nGrany;

  /* Protocol */
  IFCHECKEX(1)
  {
    printf("\n\n   Source FST instance     : %s" ,BASEINST(itSrc)->m_lpInstanceName);
    printf("\n   Destination FST instance: %s" ,BASEINST(_this)->m_lpInstanceName);
    printf("\n   Traversal root states   : %ld",(long)nCtr[0]);
    printf("\n   Search layers           : %ld",(long)nCtr[1]);
    printf("\n   Node enumerations       : %ld",(long)nCtr[2]);
    printf("\n   Trans enumerations      : %ld",(long)nCtr[3]);
    printf("\n   Terminated paths");
    printf("\n   - Ceiling break-throughs: %ld",(long)nCtr[4]);
    printf("\n   - Overlength            : %ld",(long)nCtr[5]);
    printf("\n\n CFst_X1 done.\n");
    dlp_fprint_x_line(stdout,'-',dlp_maxprintcols());
    printf("\n");
  }
#endif
  return O_K;
}

/**
 * Creates a graph with the best <code>nPaths</code> paths (minimum cost) per
 * state taken from <code>itSrc</code>. There are no checks performed.
 *
 * @param _this  Pointer to this (destination) automaton instance
 * @param itSrc  Pointer to source automaton instance
 * @param nUnit  Index of unit to process 
 * @param nPaths Number of paths to be extracted 
 * @return O_K if successfull, a (negative) error code otherwise
 * @see BestN CFst_BestN
 */
INT16 CGEN_PROTECTED CFst_BestNUnitLocal
(
  CFst* _this,
  CFst* itSrc,
  INT32  nUnit,
  INT32  nPaths
)
{
  FST_TID_TYPE* lpTI   = NULL;
  FST_ITYPE     nT     = 0;
  FST_WTYPE     nW     = 0.;                                                   /* Weight of current transition       */
  FST_WTYPE     nPot   = 0.;                                                   /* Current potential                  */
  INT32          nIcPot = -1;                                                   /* Comp. index od state potential     */
  INT32          nR     = 0;                                                    /* Curr. record in compr. trans. tab. */
  INT32          nFTs   = 0;                                                    /* First transition at current state  */
  INT32          nXTs   = 0;                                                    /* No. transitions at current state   */
  CData*        idTidx = NULL;                                                 /* Compressed transition table        */
  CData*        idTatS = NULL;                                                 /* List of trans. leaving cur. state  */
  CData*        idTd   = NULL;                                                 /* Destination transition table       */

  /* Validate */
  DLPASSERT(_this!=itSrc);

  /* Prepare destination */
  CFst_CopyUi(_this,itSrc,NULL,nUnit);
  CData_AddComp(AS(CData,_this->td),"TPOT",DLP_TYPE(FST_WTYPE));
  _this->m_nIcTdAux = CData_GetNComps(AS(CData,_this->td))-1;

  /* Initialize */
  nIcPot = CData_FindComp(AS(CData,_this->sd),NC_SD_POT);
  _this->m_nWsr = CFst_Wsr_GetType(_this,&_this->m_nIcW);
  DLPASSERT(nIcPot       >=0           );                                      /* Should have been checked by caller */
  DLPASSERT(_this->m_nWsr!=FST_WSR_NONE);                                      /* Should have been checked by caller */
  DLPASSERT(_this->m_nIcW>=0           );                                      /* Should have been checked by caller */
  ICREATEEX(CData,idTidx,"~CFst_BestNUnitLocal.idTidx",NULL);
  ICREATEEX(CData,idTatS,"~CFst_BestNUnitLocal.idTatS",NULL);
  ICREATEEX(CData,idTd  ,"~CFst_BestNUnitLocal.idTd"  ,NULL);

  /* Calculate transition potentials */
  lpTI = CFst_STI_Init(_this,0,FSTI_SORTINI);
  for (nT=0; nT<lpTI->nXT; nT++)
  {
    nPot = *(FST_WTYPE*)CData_XAddr(AS(CData,_this->sd),TD_TER(_this,nT),nIcPot);
    nW   = *(FST_WTYPE*)CData_XAddr(AS(CData,_this->td),nT,_this->m_nIcW);
    nPot = CFst_Wsr_Op(_this,nPot,nW,OP_MULT);
    *(FST_WTYPE*)CData_XAddr(AS(CData,_this->td),nT,_this->m_nIcTdAux) = nPot;
  }

  /* Select n-best outgoing transitions per state */
  CData_Compress(idTidx,AS(CData,_this->td),IC_TD_INI);
  for (nR=0; nR<CData_GetNRecs(idTidx); nR++)
  {
    nFTs = (INT32)CData_Dfetch(idTidx,nR,1);
    nXTs = (INT32)CData_Dfetch(idTidx,nR,2);
    CData_SelectRecs(idTatS,AS(CData,_this->td),nFTs,nXTs);
    switch (_this->m_nWsr)
    {
      case FST_WSR_LOG:
      case FST_WSR_TROP:
        CData_Sortup(idTatS,idTatS,_this->m_nIcTdAux);
        break;
      case FST_WSR_PROB:
        CData_Sortdown(idTatS,idTatS,_this->m_nIcTdAux);
        break;
      default:
        DLPASSERT(FMSG("Invalid weight semiring type"));
    }
    CData_SelectRecs(idTatS,idTatS,0,nPaths);
    CData_Cat(idTd,idTatS);
  }

  /* Finish destination */
  CData_Copy(_this->td,BASEINST(idTd));
  UD_XT(_this,0) = CData_GetNRecs(AS(CData,_this->td));

  /* Clean up */
  IDESTROY(idTatS);
  IDESTROY(idTidx);
  IDESTROY(idTd  );
  CFst_STI_Done(lpTI);
  CData_DeleteComps(AS(CData,_this->td),_this->m_nIcTdAux,1);
  _this->m_nIcTdAux = -1;
  return O_K;
}


/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////// */
/* DEBUGGING METHODS                                                                                                 */

FST_WTYPE __Ndt_Descend(FST_TID_TYPE* lpTI, FST_SEQ_TYPE* lpSeq, INT32 nOrder, INT32 nDepth)
{
  FST_STYPE nTis = 0;
  FST_WTYPE nSum = 0.;

  DLPASSERT(nDepth<nOrder);

  for (nTis=0; nTis<lpTI->iFst->m_nSymbols; nTis++)
  {
    ((FST_ITYPE*)lpSeq->lpItm)[nDepth] = nTis;
    if (nDepth<nOrder-1)
      nSum += __Ndt_Descend(lpTI,lpSeq,nOrder,nDepth+1);
    else
      nSum += exp(-CFst_Nmg_CalcSeqProb(lpTI->iFst,lpTI,*lpSeq,nOrder,NULL,NULL,NULL));
    if (nDepth==0) printf(".");
  }

  return nSum;
}

INT16 CFst_Nmg_DeepTest(CFst* _this, INT32 nOrder)
{
  FST_SEQ_TYPE  seq;
  FST_TID_TYPE* lpTI = NULL;

  CHECK_THIS_RV(NOT_EXEC);
  if (CFst_Wsr_GetType(_this,&_this->m_nIcW)!=FST_WSR_PROB)
    return IERROR(_this,FST_MISS,"transition probability component",NC_TD_PSR,"transition table");
  if (nOrder>_this->m_nMaxLen) nOrder = _this->m_nMaxLen;

  printf("\n   Deep test of %ld-grams in n-multigram %s",(long)nOrder,BASEINST(_this)->m_lpInstanceName);
  printf("\n   - Symbols : %ld",(long)_this->m_nSymbols);
  printf("\n   - RC floor: %G",(double)_this->m_nRcfloor);

  lpTI            = CFst_STI_Init(_this,0,FSTI_SORTINI);
  seq.lpItm       = (BYTE*)dlp_calloc(nOrder,sizeof(FST_STYPE));
  seq.nCnt        = nOrder;
  seq.nOfs        = sizeof(FST_STYPE);
  _this->m_lpNmgT = (FST_ITYPE*)dlp_calloc(nOrder+1,sizeof(FST_ITYPE));

  printf("\n   - Testing : ");
  printf("\n   - Prob.sum: %G",(double)__Ndt_Descend(lpTI,&seq,nOrder,0));
  printf("\n");

  dlp_free(_this->m_lpNmgT);
  dlp_free(seq.lpItm);
  CFst_STI_Done(lpTI);
  _this->m_lpNmgT = NULL;
  return O_K;
}

#ifndef __UNENTANGLE_FST
  #include "dlp_profile.h"
#endif /* #ifndef __UNENTANGLE_FST */

INT16 CFst_TraverseTest(CFst* _this, INT32 nCycles)
{
#ifndef __UNENTANGLE_FST

  FST_TID_TYPE* lpTI = NULL;
  BYTE*         lpT  = NULL;
  INT32          i    = 0;
  INT32          s    = 0;
  INT32          a    = 0;
  INT32          b    = 0;
  INT32          c    = 0;
  INT32          d    = 0;
  CProfile*     P    = NULL;

  printf("\nFST %s - Traversal test:",SCSTR(BASEINST(_this)->m_lpInstanceName));
  printf("\n- %ld cycles through %ld transitions",(long)nCycles,(long)CData_GetNRecs(AS(CData,_this->td)));
  ICREATEEX(CProfile,P,"~CFst_TraverseTest.P",NULL);

  CProfile_BeginTimer(P);
  lpTI = CFst_STI_Init(_this,0,FSTI_SORTINI);
  CProfile_EndTimer(P,"\n- Creating FST iterator: ");

  CProfile_BeginTimer(P);
  for (i=0,c=0; i<nCycles; i++)
    for (s=0,a=0,b=T_INT_MAX; s<lpTI->nXS; s++)
    {
      for (d=0,lpT=CFst_STI_TfromS(lpTI,s,NULL); lpT; lpT=CFst_STI_TfromS(lpTI,s,lpT),d++)
        c++;
      if (d>a) a=d;
      if (d<b) b=d;
    }
  printf("\n- %ld transitions enumerated",c);
  printf("\n- min/max/mean branches: %ld/%ld/%G",(long)b,(long)a,(double)(UD_XT(_this,0)/UD_XS(_this,0)));
  CProfile_EndTimer(P,"\n- Traversal: ");

  CProfile_BeginTimer(P);
  CFst_STI_Done(lpTI);
  CProfile_EndTimer(P,"\n- Clean up: ");

  IDESTROY(P);
  return O_K;

#else /* #ifndef __UNENTANGLE_FST */

  return IERROR(_this,FST_INTERNAL,__FILE__,__LINE__,0);
  /* NOTE: __UNENTANGLE_FST was defined (probably in dlp_config.h or fst.def).
   *       Undefine it to use this feature!
   */

#endif /* #ifndef __UNENTANGLE_FST */
}

INT16 CGEN_PUBLIC CFst_Debug(CFst* _this, FLOAT64 nParam, const char* sOp)
{
  if (strcmp(sOp,"?")==0)
  {
    printf("\n   fst -debug operation codes:");
    printf("\n   - nmg_stochastic: Check if all n-gram probabilities of order nParam sum up to 1");
    printf("\n   - traverse      : Cyclic traversal (for profiling)");
    printf("\n");
    return O_K;
  }

  if (strcmp(sOp,"nmg_stochastic")==0) return CFst_Nmg_DeepTest(_this,(INT32)nParam);
  if (strcmp(sOp,"traverse"      )==0) return CFst_TraverseTest(_this,(INT32)nParam);

  IERROR(_this,ERR_GENERIC,"Unknown debug operation",0,0);
  CFst_Debug(_this,0.,"?");
  return NOT_EXEC;
}

/* EOF */
