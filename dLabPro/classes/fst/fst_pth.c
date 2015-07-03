/* dLabPro class CFst (fst)
 * - Paths
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
#include "dlp_math.h"

#define IFDOPRINT(A,B) \
  if (BASEINST(_this)->m_nCheck>=A && ((_this->m_nPrintstop>=0 && _this->m_nPrintstop<=B) || _this->m_nPrintstop==-2))

/*
 * Manual page at fst_man.def
 */
INT16 CGEN_PUBLIC CFst_Fslist(CFst* _this, INT32 nUnit, CData* idDst)
{
  INT32 nR  = 0;                                                                /* Current record in dest. instance  */
  INT32 nU  = 0;                                                                /* Current unit                      */
  INT32 nS  = 0;                                                                /* Current state                     */
  INT32 nFS = 0;                                                                /* First state of current unit       */
  INT32 nXS = 0;                                                                /* Number of states of current unit  */

  /* Initialize destination instance */
  CData_Reset(BASEINST(idDst),TRUE);
  CData_AddComp(idDst,"fsid",T_LONG);

  /* Loop over units */
  for (nU=nUnit<0?0:nUnit; nU<UD_XXU(_this); nU++)
  {
    nFS = UD_FS(_this,nU);
    nXS = UD_XS(_this,nU);

    /* Loop over states */
    for (nS=nFS; nS<nFS+nXS; nS++)
      if ((SD_FLG(_this,nS)&0x01)==0x01)
      {
        nR = CData_AddRecs(idDst,1,100);
        CData_Dstore(idDst,nS-(nUnit>=0?nFS:0),nR,0);
      }

    /* Stop in single unit mode */
    if (nUnit>=0) break;
  }

  return O_K;
}

/*
 * Manual page at fst_man.def
 */
INT16 CGEN_PUBLIC CFst_Fsunify(CFst* _this, INT32 nUnit)
{
  INT32         nU         = 0;
  FST_ITYPE     nS         = 0;
  BYTE*         lpT        = NULL;
  FST_TID_TYPE* lpIterator = NULL;

  for (nU=nUnit<0?0:nUnit; nU<UD_XXU(_this); nU++)
  {
    /* Add a new final state */
    CFst_Addstates(_this,nU,1,TRUE);
    IFCHECKEX(1) printf("\n Created new final state: %ld (absolute: %ld)",(long)UD_XS(_this,nU)-1,(long)(UD_XS(_this,nU)-1+UD_FS(_this,nU)));

    /* Initialize graph iterator */
    lpIterator = CFst_STI_Init(_this,nU,FSTI_PTR);

    /* Find all final states (but ignore the new final state: UD_XS(_this,nU)-1 ) */
    for (nS=UD_FS(_this,nU); nS<UD_FS(_this,nU)+UD_XS(_this,nU)-1; nS++)
    {
      if ((SD_FLG(_this,nS)&SD_FLG_FINAL)==SD_FLG_FINAL)
      {
        IFCHECKEX(1) printf("\n Found final state: %ld (absolute: %ld)",(long)(nS-UD_FS(_this,nU)),(long)nS);

        /* Find out if there are transitions originating in current final state */
        if (CFst_STI_TfromS(lpIterator,nS-UD_FS(_this,nU),NULL)!=NULL)
        {
          IFCHECKEX(1) printf("\n Create transition: %ld --> %ld",(long)(nS-UD_FS(_this,nU)),(long)UD_XS(_this,nU)-1);

          /* Insert epsilon transition leading to the new final state */
          CFst_Addtrans(_this,nU,nS-UD_FS(_this,nU),UD_XS(_this,nU)-1);
          CFst_STI_UnitChanged(lpIterator,FSTI_CADD);

          /* Make current state a non-final state */
          SD_FLG(_this,nS)&=~0x01;   /* x = x & 0xFE */
        }
        else
        {
          /* Find out if there are transitions leading to current final state,
             if yes, detour them */
          IFCHECKEX(1) printf("\n Outer final state: %ld",(long)(nS-UD_FS(_this,nU)));
          lpT=NULL;
          while ((lpT=CFst_STI_TtoS(lpIterator,nS-UD_FS(_this,nU),lpT))!=NULL)
          {
            IFCHECKEX(1)
              printf("\n Detour transition: %ld --> (%ld ~> %ld)",(long)*CFst_STI_TIni(lpIterator,lpT),
                  (long)*CFst_STI_TTer(lpIterator,lpT),(long)UD_XS(_this,nU)-1);

            /* Set terminal state of transition lpT to the new final state */
            *CFst_STI_TTer(lpIterator,lpT) = UD_XS(_this,nU)-1;
          }
        }
      }
    }

    /* Destroy graph iterator */
    CFst_STI_Done(lpIterator);

    /* Stop in single unit mode */
    if (nUnit>=0) break;
  }

  /* Trim result */
  CFst_Trim(_this,nUnit,0.);
  return O_K;
}

/**
 * Tree expansion recusion.
 *
 * @param _this  Pointer to this (destination) automaton instance
 * @param lpTI   Pointer to automaton iterator data structure describing the source automaton
 * @param nSsrc  The current unit relative state index in the source automaton
 * @param nSdst  The current unit relative state index in the destination automaton
 * @param nDepth The current recursion depth
 * @return       0 if ok, -1 if expansion of path was aborted, -2 if
 *               recursion shall be aborted.
 */
INT16 CGEN_PRIVATE CFst_TreeUnit_Walk
(
  CFst*         _this,
  FST_TID_TYPE* lpTI,
  FST_ITYPE     nSsrc,
  FST_ITYPE     nSdst,
  INT32          nDepth
)
{
  INT16     nRet     = 0;                                                      /* Return value                      */
  BYTE*     lpT      = NULL;                                                   /* Pointer to current source trans.  */
  INT32*     lpClimit = NULL;                                                   /* Ptr. to curr. state's cycle limit */
  FST_ITYPE nTerSrc  = 0;                                                      /* Term. state of curr. src. trans.  */
  FST_ITYPE nTerDst  = 0;                                                      /* Term. state of curr. dst. trans.  */

  if (nDepth>=_this->m_nMaxLen)
  {
    IFCHECK printf("\n   PATH TOO LONG expanding state %ld[%ld]:",(long)nSsrc,(long)nDepth);
    return -1;
  }

  IFCHECK printf("\n   Expanding state %ld[%ld]:",(long)nSsrc,(long)nDepth);

  /* Enumerate transitions starting in nSsrc */
  while ((lpT=CFst_STI_TfromS(lpTI,nSsrc,lpT))!=NULL)
  {
    nTerSrc  = *CFst_STI_TTer(lpTI,lpT);
    lpClimit = NULL;
    IFCHECK printf("\n     -> %ld",(long)nTerSrc);
    if (_this->m_bClimit)
    {
      lpClimit = (INT32*)CData_XAddr(AS(CData,lpTI->iFst->sd),nTerSrc+lpTI->nFS,lpTI->iFst->m_nIcSdAux);
      IFCHECK printf("(%ld)%s",(long)*lpClimit,*lpClimit<0?" STOP":"");

      /* Do not expand when cycle limit is reached */
      if (*lpClimit<0) continue;
    }

    /* Add destination state and copy qualification */
    if (_this->m_bNoloops && nSsrc==nTerSrc)
      nTerDst = nSdst;
    else
      nTerDst = CFst_AddstateCopy(_this,0,lpTI->iFst,nTerSrc+lpTI->nFS);

    /* Add destination transition and copy qualification */
    CFst_AddtransCopy(_this,0,nSdst,nTerDst,lpTI->iFst,CFst_STI_GetTransId(lpTI,lpT));

    /* Do not expand self loops in /noloops mode */
    if (_this->m_bNoloops && nSsrc==nTerSrc) continue;

    /* Descent */
    IFCHECK printf("\n   Descent to depth [%ld]",(long)nDepth+1);
    if (_this->m_bClimit) (*lpClimit)--;
    nRet = CFst_TreeUnit_Walk(_this,lpTI,nTerSrc,nTerDst,nDepth+1);
    if (_this->m_bClimit) (*lpClimit)++;
    IFCHECK printf("\n   Ascent to depth [%ld]",(long)nDepth);

    if (nRet==-2)
    {
      IFCHECK printf(" ABORT");
      return -2;
    }
  }

  return nRet;
}

/**
 * Expands one unit into a tree. There are no checks performed.
 *
 * @param _this Pointer to this (destination) automaton instance
 * @param itSrc Pointer to source automaton instance
 * @param nUnit Index of unit to process
 * @return O_K if successfull, a (negative) error code otherwise
 * @see Tree CFst_Tree
 */
INT16 CGEN_PROTECTED CFst_TreeUnit(CFst* _this, CFst* itSrc, INT32 nUnit)
{
  FST_TID_TYPE* lpTI = NULL;                                                   /* Automaton iterator data structure */

  /* Verify parameters */
  DLPASSERT(_this!=itSrc);

  /* Initialize destination */
  CData_Scopy(AS(CData,_this->sd),AS(CData,itSrc->sd));
  CData_Scopy(AS(CData,_this->td),AS(CData,itSrc->td));
  CData_SelectRecs(AS(CData,_this->ud),AS(CData,itSrc->ud),nUnit,1);
  UD_FS(_this,0)=0;
  UD_FT(_this,0)=0;
  UD_XS(_this,0)=0;
  UD_XT(_this,0)=0;

  /* Add destination zero state */
  CFst_Addstates(_this,0,1,(SD_FLG(itSrc,UD_FS(itSrc,nUnit))&0x01)==0x01);

  /* Expand */
  lpTI = CFst_STI_Init(itSrc,nUnit,FSTI_SORTINI);
  switch (CFst_TreeUnit_Walk(_this,lpTI,0,0,0))
  {
  case -1: IERROR(_this,FST_PATHTOOLONG,"expanding",nUnit,0); break;
  case -2: IERROR(_this,FST_INTERNAL,__FILE__,__LINE__,0   ); break;
  }
  CFst_STI_Done(lpTI);

  return O_K;
}

/*
 * Manual page at fst_man.def
 */
INT16 CGEN_PUBLIC CFst_Tree(CFst* _this, CFst* itSrc, INT32 nUnit)
{
  CFst*     itUnit = NULL;                                                     /* Current unit                      */
  INT32      nU     = 0;                                                        /* Current unit index                */
  FST_ITYPE nS     = 0;                                                        /* Current state                     */
  INT16     nErr   = O_K;                                                      /* Current error status              */
  INT16     nRet   = O_K;                                                      /* Return value                      */

  CHECK_THIS_RV(NOT_EXEC);
  CFst_Check(_this);
  CFst_Check(itSrc);

  /* Protocol */
  IFCHECK
  {
    printf("\n"); dlp_fprint_x_line(stdout,'-',dlp_maxprintcols());
    printf("\n CFst_Tree(%s,%s,%ld)",BASEINST(_this)->m_lpInstanceName,itSrc?BASEINST(itSrc)->m_lpInstanceName:"NULL",(long)nUnit);
    printf("\n");
    if (_this->m_bNoloops) printf(" /noloops");
    if (_this->m_bClimit ) printf(" /climit" );
    if (_this->m_bIndex  ) printf(" /index"  );
    printf("\n"); dlp_fprint_x_line(stdout,'-',dlp_maxprintcols());
    printf("\n");
  }

  /* Validation */
  if (_this->m_bClimit && _this->m_bIndex)
  {
    if
    (
      _this->m_nClimit< IC_SD_DATA ||
      _this->m_nClimit>=CData_GetNComps(AS(CData,itSrc->sd))
    )
    {
      return IERROR(_this,FST_BADID,"state table component",_this->m_nClimit,0);
    }

    if (!dlp_is_numeric_type_code(CData_GetCompType(AS(CData,itSrc->sd),_this->m_nClimit)))
      return IERROR(_this,FST_BADCTYPE,"state",_this->m_nClimit,0);
  }

  /* NO RETURNS BEYOND THIS POINT! */
  CREATEVIRTUAL(CFst,itSrc,_this);
  CData_Scopy(AS(CData,_this->ud),AS(CData,itSrc->ud));
  CData_Scopy(AS(CData,_this->sd),AS(CData,itSrc->sd));
  CData_Scopy(AS(CData,_this->td),AS(CData,itSrc->td));
  ICREATEEX(CFst,itUnit,"CFst_Tree~itUnit",NULL);

  /* Add and initialize cycle limits component to source */
  if (_this->m_bClimit)
  {
    itSrc->m_nIcSdAux = CData_GetNComps(AS(CData,itSrc->sd));
    CData_AddComp(AS(CData,itSrc->sd),"CLMT",T_LONG);

    for (nS=0; nS<UD_XXS(itSrc); nS++)
      CData_Dstore
      (
        AS(CData,itSrc->sd),
        _this->m_bIndex ? CData_Dfetch(AS(CData,itSrc->sd),nS,_this->m_nClimit) : _this->m_nClimit,
        nS,itSrc->m_nIcSdAux
      );
  }

  /* Loop over units */
  for (nU=0; nU<UD_XXU(itSrc); nU++)
  {
    CFst_Reset(BASEINST(itUnit),TRUE);
    BASEINST(itUnit)->m_nCheck    = BASEINST(_this)->m_nCheck;
    itUnit->m_nMaxLen   = _this->m_nMaxLen;
    itUnit->m_nClimit   = _this->m_nClimit;
    itUnit->m_bNoloops  = _this->m_bNoloops;
    itUnit->m_bClimit   = _this->m_bClimit;
    itUnit->m_bIndex    = _this->m_bIndex;

    if (nUnit<0 || nUnit==nU)
    {
      IFCHECK printf("\n Expanding unit %ld\n -------------------",(long)nU);
      nErr=CFst_TreeUnit(itUnit,itSrc,nU);
      IFCHECK printf("\n -------------------");
    }
    else
    {
      IFCHECK printf("\n Copying unit %ld",(long)nU);
      nErr=CFst_CopyUi(itUnit,itSrc,NULL,nU);
    }
    IF_NOK(nErr) nRet=nErr;

    nErr=CFst_Cat(_this,itUnit);
    IF_NOK(nErr) nRet=nErr;
  }

  /* Copy input and output symbol table */                                      /* --------------------------------- */
  CData_Copy(_this->is,itSrc->is);                                              /* Copy input symbol table           */
  CData_Copy(_this->os,itSrc->os);                                              /* Copy output symbol table          */

  /* Add cycle limits component to source */
  if (_this->m_bClimit)
  {
    DLPASSERT(dlp_strcmp(CData_GetCname(AS(CData,itSrc->sd),itSrc->m_nIcSdAux),"CLMT")==0);
    CData_DeleteComps(AS(CData,itSrc->sd),itSrc->m_nIcSdAux,1);
  }

  /* Clean up */
  IDESTROY(itUnit);
  DESTROYVIRTUAL(itSrc,_this);
  CFst_Check(_this);                                                           /* TODO: Remove after debugging      */

  /* Protocol */
  IFCHECK
  {
    printf("\n\n CFst_Tree done.\n");
    dlp_fprint_x_line(stdout,'-',dlp_maxprintcols());
    printf("\n");
  }
  return nRet;
}


typedef BYTE* (FST_TXSFUNC_TYPE)(FST_TID_TYPE*,INT32,BYTE*);

/**
 * Recursive transition walking function for <code>CFst_Excerpt</code>.
 *
 * @param lpTI
 *          Transition iterator data stuct (must be non-<code>NULL</code> and
 *          properly initialized)
 * @param lpED
 *          Excerpt data struct (must be non-<code>NULL</code> and properly
 *          initialized).
 * @param nS
 *          The state to expand.
 * @param nIis
 *          The zero-based index of the input symbol in the string
 *          <code>lpED->lpIs</code> to consume next.
 * @param nDepth
 *          The number of states expanded until here.
 */
BOOL CGEN_SPRIVATE CFst_ExcerptWalk
(
  FST_TID_TYPE*  lpTI,
  FST_EXCR_TYPE* lpED,
  FST_ITYPE      nS,
  FST_ITYPE      nIis,
  FST_ITYPE      nDepth
)
{
  FST_ITYPE*        lpSn   = NULL;                                              /* Next state to expand              */
  FST_ITYPE         nT     = 0;                                                 /* (Global) transition index         */
  BYTE*             lpT    = NULL;                                              /* Currently expanded transition     */
  FST_TXSFUNC_TYPE* lpfTxS = NULL;                                              /* Forward or backward trans. getter */
  FST_ITYPE         nIni   = 0;                                                 /* Initial state of current trans.   */
  FST_ITYPE         nTer   = 0;                                                 /* Terminal state of current trans.  */
  FST_STYPE         nTis   = -1;                                                /* Transducer input symbol           */
  FST_STYPE         nSis   = -1;                                                /* String input symbol               */
  INT32             nIisn  = nIis;                                              /* Index of inp.symb.to consume next */
  BOOL              bRet   = lpED->lpIs==NULL;                                  /* Return value                      */

  /* Protocol */                                                                /* --------------------------------- */
  if (BASEINST(lpTI->iFst)->m_nCheck>=2)                                        /* On verbose level 2                */
    printf("\n   - [D:%ld] Expanding state %ld %sward",                         /*   Protocol                        */
      (long)nDepth,(long)nS,lpED->bBwd?"back":"for");                           /*   |                               */

  /* Limit recursion depth (This is defined behavior!) */                       /* --------------------------------- */
  if (lpED->nMaxLen>=0 && nDepth>=lpED->nMaxLen)                                /* Maximal path length reached       */
  {                                                                             /* >>                                */
    if (BASEINST(lpTI->iFst)->m_nCheck>=2)                                      /*   On verbose level 2              */
      printf("\n     - Path length limit exceeded -> terminate");               /*     Protocol                      */
    lpTI->bPathsClipped = TRUE;                                                 /*   Remember a path was clipped     */
    return bRet;                                                                /*   Let it be, let it behee...      */
  }                                                                             /* <<                                */

  /* Entire input string matched */                                             /* --------------------------------- */
  if (lpED->lpIs && (nIisn<0 || nIisn>=lpED->nXIs))                             /* Entire input string matched       */
  {                                                                             /* >>                                */
    if (BASEINST(lpTI->iFst)->m_nCheck>=2)                                      /*   On verbose level 2              */
      printf("\n     - Input string matched");                                  /*     Protocol                      */
    return TRUE;                                                                /*   Return "success"                */
  }                                                                             /* <<                                */

  /* Prevent from running in cycles */                                          /* --------------------------------- */
  if (lpED->lpIs==NULL)                                                         /* Only if no input string           */
  {                                                                             /* >>                                */
    if (SD_FLG(lpTI->iFst,lpTI->nFS+nS)&SD_FLG_USER1)                           /*   Have already been here          */
    {                                                                           /*   >>                              */
      if (BASEINST(lpTI->iFst)->m_nCheck>=2)                                    /*     On verbose level 2            */
        printf("\n     - Terminating cycle");                                   /*       Protocol                    */
      return bRet;                                                              /*     Do not expand again           */
    }                                                                           /*   <<                              */
    SD_FLG(lpTI->iFst,lpTI->nFS+nS)|=SD_FLG_USER1;                              /*   Remember we have been here      */
  }                                                                             /* <<                                */

  /* Iterate transitions from/to nS */                                          /* --------------------------------- */
  if (lpED->bBwd) lpfTxS = CFst_STI_TtoS;                                       /* Backward walking transition getter*/
  else            lpfTxS = CFst_STI_TfromS;                                     /* Forward walking transition getter */
  while ((lpT=lpfTxS(lpTI,nS,lpT))!=NULL)                                       /* Enumerate transitions from/to nS  */
  {                                                                             /* >>                                */
    nT   = CFst_STI_GetTransId(lpTI,lpT);                                       /*   Get global transition index     */
    nIni = *CFst_STI_TIni(lpTI,lpT);                                            /*   Get initial state index         */
    nTer = *CFst_STI_TTer(lpTI,lpT);                                            /*   Get terminal state index        */
    if (BASEINST(lpTI->iFst)->m_nCheck>=2)                                      /*     On verbose level 2            */
      printf("\n     - Walking transition (%ld)--[%ld]->(%ld)",                 /*       Protocol                    */
        (long)nIni,(long)nT,(long)nTer);                                        /*       |                           */

    /* Match on input string (if any) */                                        /*   - - - - - - - - - - - - - - - - */
    if (lpED->lpIs)                                                             /*   If there is an input string     */
    {                                                                           /*   >>                              */
      nTis = *CFst_STI_TTis(lpTI,lpT);                                          /*     Get transducer input symbol   */
      if (nTis>=0)                                                              /*     Non-epsilon input symbol      */
      {                                                                         /*     >>                            */
        nSis = lpED->lpIs[nIis];                                                /*       Get string input symbol     */
        if (nSis!=nTis) continue;                                               /*       Path not good!              */
        if (lpED->bBwd) nIisn = nIis-1; else nIisn = nIis+1;                    /*       One string symbol consumed  */
      }                                                                         /*     <<                            */
    }                                                                           /*   <<                              */

    /* Forward or backward walking recursion */                                 /*   - - - - - - - - - - - - - - - - */
    if (lpED->bBwd) lpSn = CFst_STI_TIni(lpTI,lpT);                             /*   Get next state to expand (bwd.) */
    else            lpSn = CFst_STI_TTer(lpTI,lpT);                             /*   Get next state to expand (fwd.) */
    if (CFst_ExcerptWalk(lpTI,lpED,*lpSn,nIisn,nDepth+1))                       /*   Expand next state               */
    {                                                                           /*   >> (Path was good)              */
      if (BASEINST(lpTI->iFst)->m_nCheck>=2)                                    /*     On verbose level 2            */
        printf("\n   - [D:%ld] Keeping transition (%ld)--[%ld]-->(%ld)",        /*       Protocol                    */
          (long)nDepth,(long)nIni,(long)nT,(long)nTer);                         /*       |                           */
      CData_Mark(AS(CData,lpTI->iFst->sd),lpTI->nFS+nIni,1);                    /*     Mark initial state            */
      CData_Mark(AS(CData,lpTI->iFst->sd),lpTI->nFS+nTer,1);                    /*     Mark terminal state           */
      CData_Mark(AS(CData,lpTI->iFst->td),nT,1);                                /*     Mark transition               */
      bRet = TRUE;                                                              /*     There was a successful expns. */
    }                                                                           /*   <<                              */
  }                                                                             /* <<                                */

  return bRet;                                                                  /* Return successful expansion       */
}

/*
 * Manual page at fst_man.def
 */
INT16 CGEN_PUBLIC CFst_Excerpt
(
  CFst*  _this,
  CFst*  itSrc,
  INT32  nUnit,
  CData* idStates,
  INT32  nIcStates,
  CData* idIs,
  INT32  nParam
)
{
  INT16          nCheck    = ML_SILENT;                                         /* Check level of _this              */
  INT16          nCheckSrc = ML_SILENT;                                         /* Check level of itSrc              */
  BOOL           bForward  = FALSE;                                             /* Forward traversal                 */
  BOOL           bBackward = FALSE;                                             /* Backward traversal                */
  INT32          nR        = 0;                                                 /* Record counter                    */
  FST_ITYPE      nS        = 0;                                                 /* State index                       */
  FST_ITYPE      nSe       = 0;                                                 /* State index in excerpt unit       */
  FST_ITYPE      nFS       = 0;                                                 /* First state in source unit        */
  FST_ITYPE      nXS       = 0;                                                 /* Number of states in source unit   */
  FST_ITYPE      nTe       = 0;                                                 /* Transition index in excerpt unit  */
  FST_ITYPE      nXTe      = 0;                                                 /* Number of trans. in excerpt unit  */
  FST_ITYPE*     lpSM      = NULL;                                              /* State map                         */
  FST_TID_TYPE*  lpTI      = NULL;                                              /* Automaton iterator data struct.   */
  FST_EXCR_TYPE* lpED      = NULL;                                              /* Excerpt data struct.              */
  CFst*          itExc     = NULL;                                              /* Excerpt transducer                */

  CHECK_THIS_RV(NOT_EXEC);
  CFst_Check(_this);
  CFst_Check(itSrc);

  /* Protocol */
  IFCHECK
  {
    printf("\n"); dlp_fprint_x_line(stdout,'-',dlp_maxprintcols());
    printf("\n CFst_Excerpt(%s,%s,%ld,%s,%ld,%s,%ld)\n",
      BASEINST(_this)->m_lpInstanceName,
      itSrc?BASEINST(itSrc)->m_lpInstanceName:"NULL",
      (long)nUnit,
      idStates?BASEINST(idStates)->m_lpInstanceName:"NULL",
      (long)nIcStates,
      idIs?BASEINST(idIs)->m_lpInstanceName:"NULL",
      (long)nParam);
    if (_this->m_bForward ) printf(" /forward" );
    if (_this->m_bBackward) printf(" /backward");
    printf("\n"); dlp_fprint_x_line(stdout,'-',dlp_maxprintcols());
    printf("\n");
  }

  /* Validate */
  if (nUnit<0 || nUnit>=UD_XXU(itSrc))
    return IERROR(itSrc,FST_BADID,"unit",nUnit,0);
  if (nIcStates<0 || nIcStates>=CData_GetNComps(idStates))
    return IERROR(_this,FST_BADID,"component",nIcStates,0);
  if (!dlp_is_numeric_type_code(CData_GetCompType(idStates,nIcStates)))
    return IERROR(_this,FST_BADCTYPE,"state",nIcStates,0);
  if (idIs)
  {
    if (nParam<0 || nParam>=CData_GetNComps(idIs))
      return IERROR(_this,FST_BADID,"component",nParam,0);
    if (!dlp_is_numeric_type_code(CData_GetCompType(idIs,nParam)))
      return IERROR(_this,FST_BADCTYPE,"input symbol",nParam,0);
  }

  nCheck    = BASEINST(_this)->m_nCheck;
  nCheckSrc = BASEINST(itSrc)->m_nCheck;
  bForward  = _this->m_bForward;
  bBackward = _this->m_bBackward;
  if (!bForward && !bBackward) { bForward=TRUE; bBackward=TRUE; }

  /* NO RETURNS BEYOND THIS POINT! */
  CREATEVIRTUAL(CFst,itSrc,_this);
  ICREATEEX(CFst,itExc,"CFst_Excerpt~itExc",NULL);
  CFst_Reset(BASEINST(_this),TRUE);
  BASEINST(_this)->m_nCheck = nCheck;
  BASEINST(itSrc)->m_nCheck = nCheck;

  /* Copy input and output symbol table */
  CData_Copy(_this->is,itSrc->is);
  CData_Copy(_this->os,itSrc->os);

  /* Prepare state map and excerpt data struct */
  lpSM = (FST_ITYPE*)dlp_calloc(UD_XS(itSrc,nUnit),sizeof(FST_ITYPE));
  lpED = (FST_EXCR_TYPE*)dlp_calloc(1,sizeof(FST_EXCR_TYPE));
  lpED->nXIs = 0;
  if (idIs)
  {
    lpED->lpIs = (FST_STYPE*)dlp_calloc(CData_GetNRecs(idIs),sizeof(FST_ITYPE));
    for (nR=0; nR<CData_GetNRecs(idIs); nR++)
    {
      FST_STYPE nSis = (FST_STYPE)CData_Dfetch(idIs,nR,nParam);
      if (nSis>=0) lpED->lpIs[lpED->nXIs++]=nSis;
    }
    lpED->nMaxLen = itSrc->m_nMaxLen;
    if (lpED->nXIs>lpED->nMaxLen) lpED->nMaxLen = lpED->nXIs;
  }
  else
    lpED->nMaxLen = nParam;

  /* Loop over states to excerpt from */
  for (nR=0; nR<CData_GetNRecs(idStates); nR++)
  {
    /* Prepare excerpt transducer */
    CFst_Reset(BASEINST(itExc),TRUE);
    CData_Copy(itExc->is,itSrc->is);
    CData_Copy(itExc->os,itSrc->os);
    CData_SelectRecs(AS(CData,itExc->ud),AS(CData,itSrc->ud),nUnit,1);
    UD_FS(itExc,0)=0; UD_XS(itExc,0)=0;
    UD_FT(itExc,0)=0; UD_XT(itExc,0)=0;

    /* Get and check state index to excerpt */
    nS = (FST_ITYPE)CData_Dfetch(idStates,nR,nIcStates);
    if (nS<0 || nS>UD_XS(itSrc,nUnit))
    {
      IERROR(itSrc,FST_BADID,"state",nUnit,0);
      CFst_Cat(_this,itExc);
      BASEINST(_this)->m_nCheck = nCheck;
      continue;
    }

    /* Initialize marking in itSrc's state and transition tables */
    CData_Unmark(AS(CData,itSrc->sd)); (AS(CData,itSrc->sd))->m_bRec = TRUE;
    CData_Unmark(AS(CData,itSrc->td)); (AS(CData,itSrc->td))->m_bRec = TRUE;
    CData_Mark(AS(CData,itSrc->sd),UD_FS(itSrc,nUnit)+nS,1);

    /* Determine states and transitions to excerpt */
    IFCHECK printf("\n Excerpt from state %ld",nS);
    if (bForward)
    {
      IFCHECK printf("\n - forward...",nS);
      CFst_ResetStateFlag(itSrc,nUnit,SD_FLG_USER1);
      lpED->bBwd = FALSE;
      lpTI = CFst_STI_Init(itSrc,nUnit,0/*HACK: Sorting not compatible with marking (and SLOWER)!*/);
      CFst_ExcerptWalk(lpTI,lpED,nS,0,0);
      CFst_STI_Done(lpTI);
      IFCHECKEX(2) printf("\n   "); IFCHECK printf("done.");
    }
    if (bBackward)
    {
      IFCHECK printf("\n - backward...",nS);
      CFst_ResetStateFlag(itSrc,nUnit,SD_FLG_USER1);
      lpED->bBwd = TRUE;
      lpTI = CFst_STI_Init(itSrc,nUnit,0/*HACK: Sorting not compatible with marking (and SLOWER)!*/);
      CFst_ExcerptWalk(lpTI,lpED,nS,lpED->nXIs-1,0);
      CFst_STI_Done(lpTI);
      IFCHECKEX(2) printf("\n   "); IFCHECK printf("done.");
    }
    CFst_ResetStateFlag(itSrc,nUnit,SD_FLG_USER1);
    IFCHECKEX(3)
    {
      CData_Print(AS(CData,itSrc->sd));
      CData_Print(AS(CData,itSrc->td));
    }

    /* Copy states */
    IFCHECK printf("\n - Copying states...");
    AS(CData,itExc->sd)->m_bMark = TRUE;
    CData_CopyMarked(AS(CData,itExc->sd),AS(CData,itSrc->sd),TRUE);
    AS(CData,itExc->sd)->m_bMark = FALSE;
    UD_XS(itExc,0)=CData_GetNRecs(AS(CData,itExc->sd));
    IFCHECK printf("done (%ld states).",(long)UD_XS(itExc,0));

    /* Copy transitions */
    IFCHECK printf("\n - Copying transitions...");
    AS(CData,itExc->td)->m_bMark = TRUE;
    CData_CopyMarked(AS(CData,itExc->td),AS(CData,itSrc->td),TRUE);
    AS(CData,itExc->td)->m_bMark = FALSE;
    UD_XT(itExc,0)=CData_GetNRecs(AS(CData,itExc->td));
    IFCHECK printf("done (%ld transitions).",(long)UD_XT(itExc,0));

    /* Adjust state indices */
    IFCHECK printf("\n - Adjusting state indices...");
    nFS = UD_FS(itSrc,nUnit);
    nXS = UD_XS(itSrc,nUnit);
    for (nS=0,nSe=0; nS<nXS; nS++)
    {
      lpSM[nS] = CData_IsMarked(AS(CData,itSrc->sd),nFS+nS) ? nSe++ : -1;
      IFCHECKEX(2) printf("\n   (%ld) -> (%ld)",(long)nS,(long)lpSM[nS]);
    }
    for (nTe=0,nXTe=UD_XT(itExc,0); nTe<nXTe; nTe++)
    {
      TD_INI(itExc,nTe) = lpSM[TD_INI(itExc,nTe)];
      TD_TER(itExc,nTe) = lpSM[TD_TER(itExc,nTe)];
    }
    IFCHECKEX(2) printf("\n   "); IFCHECK printf("done.");

    /* Cat result to this instance */
    CFst_Cat(_this,itExc);
    BASEINST(_this)->m_nCheck = nCheck;
  }

  /* Clear marking in itSrc's state and transition tables */
  CData_Unmark(AS(CData,itSrc->sd)); (AS(CData,itSrc->sd))->m_bRec = FALSE;
  CData_Unmark(AS(CData,itSrc->td)); (AS(CData,itSrc->td))->m_bRec = FALSE;

  /* Clean up */
  dlp_free(lpSM);
  dlp_free(lpED->lpIs);
  dlp_free(lpED);
  IDESTROY(itExc);
  DESTROYVIRTUAL(itSrc,_this);
  BASEINST(itSrc)->m_nCheck = nCheckSrc;

  /* Protocol */
  IFCHECK
  {
    printf("\n\n CFst_Excerpt done.\n");
    dlp_fprint_x_line(stdout,'-',dlp_maxprintcols());
    printf("\n");
  }
  return O_K;
}

/*
 * Manual at fst_man.def
 */
INT16 CGEN_PUBLIC CFst_Potential(CFst* _this, INT32 nUnit)
{
  FST_TID_TYPE* lpTI    = NULL;                                                /* Graph iterator data struct         */
  BYTE*         lpT     = NULL;                                                /* Current transition                 */
  INT32          nIcP    = -1;                                                  /* Component index of potential       */
  FST_WTYPE     nNeAdd  = 0.;                                                  /* Neutral element of addition        */
  FST_WTYPE     nNeMult = 0.;                                                  /* Neutral element of multiplication  */
  INT32          nU      = 0;                                                   /* Current unit                       */
  FST_ITYPE     nS      = 0;                                                   /* Current state                      */
  FST_ITYPE     nFS     = 0;                                                   /* First state of current unit        */
  FST_ITYPE     nXS     = 0;                                                   /* Number of states in current unit   */
  FST_ITYPE     nIni    = 0;                                                   /* Initial state of current transition*/
  INT32          nL      = 0;                                                   /* Current Viterbi search layer       */
  INT32          nXL     = 0;                                                   /* Maximal number of search layers    */
  INT32          nB      = 0;                                                   /* Current beam width                 */
  FST_WTYPE     nW      = 0.;                                                  /* Current accumulated weight         */
  FST_WTYPE*    lpPIni  = NULL;                                                /* Ptr.to potential of curr.ini.state */
  FST_WTYPE*    lpPTer  = NULL;                                                /* Ptr.to potential of curr.ter.state */
  BYTE*         lpLBrd  = NULL;                                                /* Write state presence layer buffer  */
  BYTE*         lpLBwr  = NULL;                                                /* Read state presence layer buffer   */
  BYTE*         lpSwap  = NULL;                                                /* Buffer swapping pointer            */
  INT32          nCtr[]  = {0,0,0};                                             /* Event counters                     */

  /* Validate */
  CHECK_THIS_RV(0);
  CFst_Check(_this);
  if (nUnit>=UD_XXU(_this)) return IERROR(_this,FST_BADID,"unit",nUnit,0);

  /* Protocol */
  IFCHECK
  {
    printf("\n"); dlp_fprint_x_line(stdout,'-',dlp_maxprintcols());
    printf("\n CFst_Potential(%s,%ld)",BASEINST(_this)->m_lpInstanceName,(long)nUnit);
    printf("\n"); dlp_fprint_x_line(stdout,'-',dlp_maxprintcols());
    printf("\n");
  }

  /* Initialize */
  _this->m_nWsr = CFst_Wsr_GetType(_this,&_this->m_nIcW);
  nNeAdd        = CFst_Wsr_NeAdd (_this->m_nWsr);
  nNeMult       = CFst_Wsr_NeMult(_this->m_nWsr);

  /* Create potential component in state table */
  nIcP = CData_FindComp(AS(CData,_this->sd),NC_SD_POT);
  if (nIcP<0)
  {
    CData_AddComp(AS(CData,_this->sd),NC_SD_POT,DLP_TYPE(FST_WTYPE));
    nIcP = CData_GetNComps(AS(CData,_this->sd))-1;
  }

  /* Loop over units */
  for (nU=nUnit<0?0:nUnit; nU<UD_XXU(_this); nU++)
  {
    /* Initialize search */
    nFS    = UD_FS(_this,nU);
    nXS    = UD_XS(_this,nU);
    nXL    = nXS>_this->m_nMaxLen ? nXS : _this->m_nMaxLen;
    lpTI   = CFst_STI_Init(_this,nU,FSTI_SORTTER);
    lpLBrd = (BYTE*)dlp_calloc(UD_XS(_this,nU),sizeof(BYTE));
    lpLBwr = (BYTE*)dlp_calloc(UD_XS(_this,nU),sizeof(BYTE));

    for (nS=0,nB=0; nS<nXS; nS++)
    {
      *(FST_WTYPE*)CData_XAddr(AS(CData,_this->sd),nS+nFS,nIcP) = nNeAdd;
      lpLBrd[nS] = 0;
      if (SD_FLG(_this,nS+nFS)&0x01)
      {
        *(FST_WTYPE*)CData_XAddr(AS(CData,_this->sd),nS+nFS,nIcP) = nNeMult;
        lpLBrd[nS] = 1;
        nB++;
      }
      lpLBwr[nS] = 0;
    }

    /* Viterbi traversal */
    for (nL=0; nB>0 && nL<=nXL; nL++)
    {
      IFCHECKEX(1) { printf("\n   nL=%ld:",(long)nL); nCtr[0]++; }

      /* Walk one step */
      for (nS=0,nB=0; nS<nXS; nS++)
        if (lpLBrd[nS])
        {
          IFCHECKEX(1   ) nCtr[1]++;
          IFDOPRINT(2,nL) printf("\n     nS=%ld",(long)nS);

          /* For all transitions arriving at state nS... */
          lpT = NULL;
          while ((lpT=CFst_STI_TtoS(lpTI,nS,lpT))!=NULL)
          {
            nIni   = *CFst_STI_TIni(lpTI,lpT);
            lpPIni = (FST_WTYPE*)CData_XAddr(AS(CData,_this->sd),nIni+nFS,nIcP);
            lpPTer = (FST_WTYPE*)CData_XAddr(AS(CData,_this->sd),nS  +nFS,nIcP);
            nW     = CFst_Wsr_Op(_this,*CFst_STI_TW(lpTI,lpT),*lpPTer,OP_MULT);

            IFCHECKEX(1) nCtr[2]++;
            IFDOPRINT(2,nL)
              printf("\n         %ld (w=%5g) --(w=%5g)--> %ld (w=%5g); newweight=%5g",
              (long)nIni,(double)*lpPIni,(double)*CFst_STI_TW(lpTI,lpT),(long)nS,(double)*lpPTer,(double)nW);

            if (CFst_Wsr_Op(_this,*lpPIni,nW,OP_LESS))
            {
              if (lpLBwr[nIni]==0) nB++;
              lpLBwr[nIni] = 1;
              *lpPIni      = nW;
            }
          }
        }

      /* Finish layer */
      lpSwap = lpLBrd; lpLBrd = lpLBwr; lpLBwr = lpSwap;
      for (nS=0; nS<nXS; nS++) lpLBwr[nS]=0;

      /* Protocol */
      IFDOPRINT(2,nL) printf("\n    ");
      IFCHECKEX(1   ) printf(" beamwidth=%ld",(long)nB);
      IFDOPRINT(2,nL)
      {
        printf("\n    continue <cr>, no print -1, nonstop -2: ");
        dlp_getx(T_SHORT,&_this->m_nPrintstop);
      }
    }

    /* Clean up */
    dlp_free(lpLBrd);
    dlp_free(lpLBwr);
    CFst_STI_Done(lpTI);

    /* Verification */
    if (nL==_this->m_nMaxLen && nB>0)
      IERROR(_this,FST_PATHTOOLONG,"calculating potential of",nU,0);

    /* Stop in single unit mode */
    if (nUnit>=0) break;
  }

  /* Protocol */
  IFCHECKEX(1)
  {
    printf("\n\n   FST instance      : %s" ,BASEINST(_this)->m_lpInstanceName);
    printf("\n   Search layers     : %ld",(long)nCtr[0]);
    printf("\n   Node enumerations : %ld",(long)nCtr[1]);
    printf("\n   Trans enumerations: %ld",(long)nCtr[2]);
    printf("\n\n CFst_Paths done.\n");
    dlp_fprint_x_line(stdout,'-',dlp_maxprintcols());
    printf("\n");
  }

  return O_K;
}

/*
 * Manual page at fst_man.def
 */
INT16 CGEN_PUBLIC CFst_Rndwalk(CFst* _this, CFst* itSrc, INT32 nUnit, INT32 nPaths, FLOAT64 nWInfluence)
{
  INT32 nP;
  INT32 nS;
  FST_WTYPE *lpWSum;
  FST_TID_TYPE* lpTI;
  BYTE* lpT = NULL;
  INT16 nWsr;
  FST_ITYPE **lpOS = NULL;
  FST_ITYPE **lpIS = NULL;
  BOOL bDifferentOs;
  BOOL bDifferentIs;
  INT32 nOfStk;
  INT32 nStackSize=0;
  INT32 nStackPos=0;
  INT32 *lpStack=NULL;

  /* Validation */
  CHECK_THIS_RV(NOT_EXEC);
  CFst_Check(_this);
  CFst_Check(itSrc);
  if (nUnit<0 || nUnit>=UD_XXU(itSrc)) return IERROR(_this,FST_BADID,"unit",nUnit,0);

  /* NO RETURNS BEYOND THIS POINT! */
  bDifferentOs=_this->m_bDifferentOs;
  bDifferentIs=_this->m_bDifferentIs;
  CREATEVIRTUAL(CFst,itSrc,_this);
  CFst_Reset(BASEINST(_this),TRUE);
  if (BASEINST(_this)->m_nCheck<BASEINST(itSrc)->m_nCheck) BASEINST(_this)->m_nCheck=BASEINST(itSrc)->m_nCheck;
  CData_Copy(_this->td,itSrc->td);
  CData_Allocate(AS(CData,_this->td),0);
  nWsr=CFst_Wsr_GetType(itSrc,NULL);
  if(bDifferentOs) lpOS=(FST_ITYPE **)dlp_malloc(nPaths*sizeof(FST_ITYPE*));
  if(bDifferentIs) lpIS=(FST_ITYPE **)dlp_malloc(nPaths*sizeof(FST_ITYPE*));

  /* Copy input and output symbol table */                                      /* --------------------------------- */
  CData_Copy(_this->is,itSrc->is);                                              /* Copy input symbol table           */
  CData_Copy(_this->os,itSrc->os);                                              /* Copy output symbol table          */

  /* Initialization */
  nOfStk = CData_FindComp(AS(CData,itSrc->td),"~STK");
  if(nOfStk>=0) nOfStk=CDlpTable_GetCompOffset(AS(CData,itSrc->td)->m_lpTable,nOfStk);
  lpTI = CFst_STI_Init(itSrc,nUnit,FSTI_SORTINI);
  lpWSum = (FST_WTYPE*)dlp_malloc(UD_XS(itSrc,0)*sizeof(FST_WTYPE));
  for(nS=0;nS<UD_XS(itSrc,0);nS++){
    lpWSum[nS]=0.;
    while ((lpT=CFst_STI_TfromS(lpTI,nS,lpT))!=NULL){
      FST_WTYPE nW=*CFst_STI_TW(lpTI,lpT);
      if(nWsr!=FST_WSR_PROB) nW=exp(-nW);
      lpWSum[nS]+=dlm_pow(nW,nWInfluence);
      if(SD_FLG(itSrc,nS)&SD_FLG_FINAL) lpWSum[nS]+=1.;
    }
  }

  for(nP=0;nP<nPaths;nP++){
    char lpsUName[32];
    INT32 nSdst = 0;
    INT32 nSizeOS=16;
    INT32 nSizeIS=16;
    INT32 nTryCount=0;
    INT32 nPthLen=0;
    BOOL bReject=FALSE;
    IFCHECK printf("\npath: %li start",nP);
    snprintf(lpsUName,31,"Path %li",(long)nP);
    CFst_Addunit(_this,lpsUName);
    CFst_Addstates(_this,nP,1,FALSE);
    if(bDifferentOs){
      lpOS[nP]=(FST_ITYPE *)dlp_malloc(nSizeOS*sizeof(FST_ITYPE));
      lpOS[nP][0]=0;
    }
    if(bDifferentIs){
      lpIS[nP]=(FST_ITYPE *)dlp_malloc(nSizeIS*sizeof(FST_ITYPE));
      lpIS[nP][0]=0;
    }
    nS = 0;
    nStackPos = 0;
    while(TRUE){
      BYTE* lpT = NULL;
      FST_WTYPE nRnd = (FST_WTYPE)dlp_rand() / (FST_WTYPE)RAND_MAX;
      FST_WTYPE  nWSum = 0.;
      lpT = NULL;
      while ((lpT=CFst_STI_TfromS(lpTI,nS,lpT))!=NULL){
        FST_WTYPE nW=*CFst_STI_TW(lpTI,lpT);
        if(nWsr!=FST_WSR_PROB) nW=exp(-nW);
        nWSum+=dlm_pow(nW,nWInfluence) / lpWSum[nS];
        if(nWSum>=nRnd) break;
      }
      if(++nTryCount>1000){
        IFCHECK printf("\t=> Abort (to many retries for finding a transition)");
        bReject=TRUE;
        break;
      }
      if(lpT && nOfStk>=0){
        INT32 nStk=*((INT32*)(lpT+nOfStk));
        if(nStk>0){
          if(nStackSize==nStackPos) lpStack=(INT32*)dlp_realloc(lpStack,(nStackSize+=8),sizeof(INT32));
          lpStack[nStackPos++]=nStk;
        }else if(nStk<0){
          if(nStackPos>0 && -nStk==lpStack[nStackPos-1]) nStackPos--;
          else continue;
        }
      }
      if((SD_FLG(itSrc,nS)&SD_FLG_FINAL) && !lpT && nOfStk>=0 && nStackPos) continue;
      nTryCount=0;
      IFCHECKEX(2){
        printf("\npath: %li(%5li) trans: %li->%li wsum-tot: %g wsum-now: %g rnd: %g",nP,nPthLen,nS,lpT?*CFst_STI_TTer(lpTI,lpT):-1,lpWSum[nS],nWSum,nRnd);
        if(nOfStk) printf(" stksi: %li stktop: %li",nStackPos,nStackPos?lpStack[nStackPos-1]:-1);
      }
      if(bDifferentOs && lpT && *CFst_STI_TTos(lpTI,lpT)>=0){
        if(lpOS[nP][0]==nSizeOS-1) lpOS[nP]=(FST_ITYPE *)dlp_realloc(lpOS[nP],(nSizeOS+=16),sizeof(FST_ITYPE));
        lpOS[nP][++lpOS[nP][0]]=*CFst_STI_TTos(lpTI,lpT);
      }
      if(bDifferentIs && lpT && *CFst_STI_TTis(lpTI,lpT)>=0){
        if(lpIS[nP][0]==nSizeIS-1) lpIS[nP]=(FST_ITYPE *)dlp_realloc(lpIS[nP],(nSizeIS+=16),sizeof(FST_ITYPE));
        lpIS[nP][++lpIS[nP][0]]=*CFst_STI_TTis(lpTI,lpT);
      }
      if((SD_FLG(itSrc,nS)&SD_FLG_FINAL) && !lpT){
        SD_FLG(_this,nSdst)|=SD_FLG_FINAL;
        break;
      }
      if(!lpT){ IERROR(_this,FST_PATHBROKEN,nUnit,nS,""); continue; }
      CFst_Addstates(_this,nP,1,FALSE);
      CFst_AddtransCopy(_this,nP,nSdst,nSdst+1,itSrc,CFst_STI_GetTransId(lpTI,lpT));
      if(++nPthLen>10000){
        IFCHECK printf("\t=> Abort (path to long)");
        bReject=TRUE;
        break;
      }
      nS=*CFst_STI_TTer(lpTI,lpT);
      nSdst++;
    }
    if(bDifferentOs && !bReject){
      INT32 nP1;
      INT32 nI;
      for(nP1=0;nP1<nP;nP1++) if(lpOS[nP][0]==lpOS[nP1][0]){
        for(nI=1;nI<=lpOS[nP][0];nI++) if(lpOS[nP][nI]!=lpOS[nP1][nI]) break;
        if(nI>lpOS[nP][0]) break;
      }
      if(nP1<nP){
        IFCHECK printf("\t=> Abort (output symbol sequences don't match)");
        bReject=TRUE;
      }
    }
    if(bDifferentIs && !bReject){
      INT32 nP1;
      INT32 nI;
      for(nP1=0;nP1<nP;nP1++) if(lpIS[nP][0]==lpIS[nP1][0]){
        for(nI=1;nI<=lpIS[nP][0];nI++) if(lpIS[nP][nI]!=lpIS[nP1][nI]) break;
        if(nI>lpIS[nP][0]) break;
      }
      if(nP1<nP){
        IFCHECK printf("\t=> Abort (input symbol sequences don't match)");
        bReject=TRUE;
      }
    }
    if(bReject){
      if(bDifferentOs) dlp_free(lpOS[nP]);
      if(bDifferentIs) dlp_free(lpIS[nP]);
      CFst_Delunit(_this,nP--);
    }
  }

  /* Clean up */
  IFCHECK printf("\n");
  CFst_STI_Done(lpTI);
  dlp_free(lpWSum);
  if(bDifferentOs){
    for(nP=0;nP<nPaths;nP++) dlp_free(lpOS[nP]);
    dlp_free(lpOS);
  }
  if(bDifferentIs){
    for(nP=0;nP<nPaths;nP++) dlp_free(lpIS[nP]);
    dlp_free(lpIS);
  }
  if(lpStack) dlp_free(lpStack);
  DESTROYVIRTUAL(itSrc,_this);

  return O_K;
}

/*
 * Manual page at fst_man.def
 */
INT16 CGEN_PUBLIC CFst_PushWeights(CFst* _this, CFst* itSrc, INT32 nUnit)
{
  CFst* itUnit = NULL;                                                         /* Current unit                      */
  INT32  nU     = 0;                                                            /* Current unit index                */
  INT32  nIcP;
  FST_TID_TYPE* lpTI;
  BYTE *lpT;

  /* Validation */
  CHECK_THIS_RV(NOT_EXEC);
  CFst_Check(_this);
  CFst_Check(itSrc);
  if (nUnit>=UD_XXU(itSrc)) return IERROR(_this,FST_BADID,"unit",nUnit,0);

  /* NO RETURNS BEYOND THIS POINT! */
  CREATEVIRTUAL(CFst,itSrc,_this);
  CFst_Reset(BASEINST(_this),TRUE);
  if (nUnit<0) { ICREATEEX(CFst,itUnit,"CFst_PushWeights~itUnit",NULL); }
  else         itUnit = _this;

  if (BASEINST(_this)->m_nCheck>BASEINST(itUnit)->m_nCheck) BASEINST(itUnit)->m_nCheck=BASEINST(_this)->m_nCheck;
  if (BASEINST(itSrc)->m_nCheck>BASEINST(itUnit)->m_nCheck) BASEINST(itUnit)->m_nCheck=BASEINST(itSrc)->m_nCheck;

  /* Copy input and output symbol table */                                      /* --------------------------------- */
  CData_Copy(_this->is,itSrc->is);                                              /* Copy input symbol table           */
  CData_Copy(_this->os,itSrc->os);                                              /* Copy output symbol table          */

  /* Loop over units */
  for (nU=nUnit<0?0:nUnit; nU<UD_XXU(itSrc); nU++)
  {
    CFst_CopyUi(itUnit,itSrc,NULL,nU);
    CFst_Potential(itUnit,0);
    nIcP=CData_FindComp(AS(CData,itUnit->sd),NC_SD_POT);
    lpTI=CFst_STI_Init(itUnit,0,0);
    for(lpT=NULL;(lpT=CFst_STI_TfromS(lpTI,-1,lpT));)
    {
      FST_WTYPE *nW=CFst_STI_TW(lpTI,lpT);
      if(*CFst_STI_TTer(lpTI,lpT)!=0)
        *nW=CFst_Wsr_Op(itUnit,*nW,CData_Dfetch(AS(CData,itUnit->sd),*CFst_STI_TTer(lpTI,lpT),nIcP),OP_MULT);
      if(*CFst_STI_TIni(lpTI,lpT)!=0)
        *nW=CFst_Wsr_Op(itUnit,*nW,CData_Dfetch(AS(CData,itUnit->sd),*CFst_STI_TIni(lpTI,lpT),nIcP),OP_DIV);
    }
    CFst_STI_Done(lpTI);
    CFst_Cat(_this,itUnit);
  }

  /* Clean up */
  if (nUnit<0) IDESTROY(itUnit);
  DESTROYVIRTUAL(itSrc,_this);
  CFst_Check(_this);                                                           /* TODO: Remove after debugging      */
  return O_K;
}

/* EOF */
