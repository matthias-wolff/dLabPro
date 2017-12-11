/* dLabPro class CFvrtools (fvrtools)
 * - Class CFvrtools interactive methods (user defined SMI functions)
 *
 * AUTHOR : Matthias Wolff, Werner Meyer
 * PACKAGE: dLabPro/classes
 *
 * Copyright 2013-2015 dLabPro contributors and others (see COPYRIGHT file)
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
#include "dlp_fvrtools.h"

/*
 * Documentation in fvrtools.def
 */
BOOL CGEN_PUBLIC CFvrtools_IsFvr(CFvrtools* _this, INT32 nU, CFst* itFvr)
{
  FST_TID_TYPE* lpTI   = NULL;                                                  /* Transition iterator               */
  BYTE*         lpT    = NULL;                                                  /* Transition pointer                */
  BOOL          bRet   = FALSE;                                                 /* Return value                      */
  FST_STYPE     nIsFvr = -1;                                                    /* Index of input symbol "FVR"       */

  /* Simple tests */                                                            /* --------------------------------- */
  if (itFvr==NULL)                                                              /* FST is NULL                       */
    return OK(IERROR(_this,ERR_NULLINST,"itFvr",0,0));                          /*   but must not be                 */
  if (nU<0 || nU>=UD_XXU(itFvr))                                                /* Unit index is invalid             */
    return OK(IERROR(itFvr,FST_BADID,"unit",nU,0));                             /*   but must not be                 */
  if ((nIsFvr = CFvrtools_FindIs("FVR",FALSE,itFvr))<0)                         /* Find input symbol "FVR"           */
    return FALSE;                                                               /*   Not found -> not an FVR         */

  /* Structure test */                                                          /* --------------------------------- */
  if (CFst_Analyze(itFvr,nU,FST_FWDTREE)==0) return FALSE;                      /* Not a forward tree -> not an FVR  */

  /* - Iterate transitions starting at root; NO RETURNS BEYOND HERE! */         /*                                   */
  lpTI=CFst_STI_Init(itFvr,nU,0);                                               /* Initialize transition iterator    */
  if (lpTI->nOfTTis<=0) goto L_EXCEPTION;                                       /* No input symbols -> not an FVR    */
  lpT=CFst_STI_TfromS(lpTI,0,lpT);                                              /* Get 1st transition from root      */
  if (lpT==NULL) goto L_EXCEPTION;                                              /* There is none -> not an FVR       */
  if (nIsFvr==*CFst_STI_TTis(lpTI,lpT))                                         /* Input symbol is "FVR"             */
    bRet = TRUE;                                                                /*   Looks very good...              */
  if (CFst_STI_TfromS(lpTI,0,lpT)!=NULL)                                        /* There are more trans. from root   */
    bRet = FALSE;                                                               /*   Not so good, after all :(       */

L_EXCEPTION:                                                                    /* : Clean exit label                */
  CFst_STI_Done(lpTI);                                                          /* Release iterator                  */
  return bRet;                                                                  /* Return result                     */
}

/*
 * Documentation in fvrtools.def
 */
BOOL CGEN_PUBLIC CFvrtools_IsComplete(CFvrtools* _this, INT32 nU, CFst* itFvr){
  BOOL          bLeaf       = TRUE;                                             /* bool for found leaf               */
  FST_ITYPE     nState      = 0;                                                /* State of itFvr                    */
  FST_ITYPE     nIt         = 0;                                                /* Iterator for td                   */
  FST_ITYPE     nCompIni;
  CData*        idInpS      = NULL;                                             /* Data of final state               */
  CData*        idInpTd     = NULL;                                             /* Data of final state               */

  /* Test input */                                                              /* --------------------------------- */
  CHECK_THIS_RV(NOT_EXEC);                                                      /* Check this instance               */
  if (itFvr==NULL)                                                              /* FST is NULL                       */
    return OK(IERROR(_this,ERR_NULLINST,"itFvr",0,0));                          /*   but must not be                 */
  if (!CFvrtools_IsFvr(_this,nU,itFvr))                                         /* Check Input is an FVR?            */
    return OK(IERROR(_this,FVRT_NOTFVR,BASEINST(itFvr)->m_lpInstanceName,0,0)); /*   Error message and exit          */

  /* Initialization */                                                          /* --------------------------------- */
  idInpTd = AS(CData, itFvr->td);                                               /* Sequence table of itDst           */
  idInpS  = AS(CData, itFvr->sd);                                               /* Sequence table of itDst           */
  nCompIni = CData_FindComp(AS(CData,itFvr->td),"~INI");                        /* Get comp. index of weight         */

  while(nState <= UD_XT(itFvr,0)){                                              /* Iterate over all states >>        */
    nIt = 0; bLeaf = TRUE;                                                      /*   Reset iterater and bool         */
    while(nIt < CData_GetNRecs(idInpTd)){                                       /*   Iterate over trans. table >>    */
      if(nState == CData_Dfetch(idInpTd,nIt,nCompIni)){                         /*     Is state a IniState >>        */
        bLeaf = FALSE; break;                                                   /*       couldn't be a value => break*/
      }                                                                         /*     <<                            */
      nIt++;                                                                    /*     next transition               */
    }                                                                           /*   <<                              */
    if(bLeaf==TRUE && CData_Dfetch(idInpS,nState,0)!=1) return FALSE;           /*   Leaf is not final state         */
    nState++;                                                                   /*   Next state for next iteration   */
  }                                                                             /* <<                                */

  /* Clean-up */                                                                /* --------------------------------- */
  return TRUE;                                                                  /* Return value                      */
}

/*
 * Documentation in fvrtools.def
 */
INT16 CGEN_PUBLIC CFvrtools_FromString(CFvrtools* _this, const char* lpsSrc, CFst* itFvr)
{
  INT16 nRet = O_K;                                                             /* The return value                  */

  /* Initialization */                                                          /* --------------------------------- */
  CHECK_THIS_RV(NOT_EXEC);                                                      /* Check this instance               */
  if (dlp_strlen(lpsSrc)==0)                                                    /* Input empty                       */
    FVRT_EXCEPTION(FVRT_SEQSYNTAX,"must not be empty",0,0);                     /*   Error message and exit          */

  /* Create wFVR from string representation */                                  /* --------------------------------- */
  IF_NOK(nRet=CFvrtools_StrToSeq(_this,lpsSrc,itFvr)) goto L_EXCEPTION;         /* Create normalized sequence FST    */
  IF_NOK(nRet=CFvrtools_SeqToFvr(_this,itFvr,itFvr) ) goto L_EXCEPTION;         /* Create FVR                        */

  /* Clean-up */                                                                /* --------------------------------- */
L_EXCEPTION:                                                                    /* : Clean exit label                */
  return nRet;                                                                  /* Return                            */
}

/*
 * Documentation in fvrtools.def
 */
INT16 CGEN_PUBLIC CFvrtools_FromFst(CFvrtools* _this, CFst* itSeq, CFst* itFvr)
{
  INT16         nRet = O_K;                                                     /* The return value                  */

  /* Initialization */                                                          /* --------------------------------- */
  CHECK_THIS_RV(NOT_EXEC);                                                      /* Check this instance               */
  if (itSeq==NULL)                                                              /* Output not correct instance       */
    FVRT_EXCEPTION(ERR_NULLINST,"itSrc",0,0);                                   /*   Error message and exit          */
  if (itFvr==NULL)                                                              /* Output not correct instance       */
    FVRT_EXCEPTION(ERR_NULLINST,"itFvr",0,0);                                   /*   Error message and exit          */
  if (!CFvrtools_IsFvr(_this,0,itSeq))                                          /* Check Input is an FVR?            */
    return OK(IERROR(_this,FVRT_NOTFVR,BASEINST(itSeq)->m_lpInstanceName,0,0)); /* */

  /* Create wFVR from sequence FST  */                                          /* --------------------------------- */
  IF_NOK(nRet=CFvrtools_CheckSeq(_this,itSeq,AS(CData,itSeq->is),NULL,NULL) )   /* Check sequence symbol table       */
    goto L_EXCEPTION;                                                           /* |                                 */
  CFst_CopyUi(itFvr,itSeq,NULL,0);                                              /* Get the first unit                */
  IF_NOK(nRet=CFvrtools_SeqToFvr(_this,itFvr,itFvr) ) goto L_EXCEPTION;         /* Create FVR                        */

  /* Clean-up */                                                                /* --------------------------------- */
L_EXCEPTION:                                                                    /* : Clean exit label                */
  return nRet;                                                                  /* Return                            */
}

/*
 * Documentation in fvrtools.def
 */
INT16 CGEN_PUBLIC CFvrtools_Synthesize(CFvrtools* _this, CFst* itDst, CFst* itFvr)
{
  INT16         nRet        = NULL;                                             /* The return value                  */
  FST_ITYPE     nCTIS;                                                          /* Find comp ~Tis                    */
  FST_ITYPE     nCTOS;                                                          /* Find comp ~Tos                    */
  FST_ITYPE     nTis        = -1;                                               /* Terminal input symbol             */
  FST_ITYPE     rank        = NULL;                                             /* Rank of actually iteration(tran.) */
  FST_ITYPE     nMyIniState = 0;                                                /* State iterator of original Fvr    */
  FST_ITYPE     nRecIdSym   = 0;                                                /* variable for NRec of idSym        */
  FST_ITYPE     nComp       = 0;                                                /* Numbers of possible constellation */
  CData*        idRank      = NULL;                                             /* rank of source Fvr                */
  CData*        idSym       = NULL;                                             /* Symbol const. of act. permutation */
  CData*        idSymRef    = NULL;                                             /* Reference(rank) value of idSym    */
  CData*        idSymList   = NULL;                                             /* All variation of symbol const.    */
  CData*        idStArray   = NULL;                                             /* Store number of State as ref.     */
  CData*        idStList    = NULL;                                             /* Store number of State as ref. list*/
  CData*        idTWeight   = NULL;                                             /* Store weight of Transition        */
  CData*        idDstTd     = NULL;                                             /* Transition table of itDst         */
  FST_TID_TYPE* iMySearch   = CFst_STI_Init(itFvr,0,FSTI_SORTTER);              /* find trans. of act. node          */
  FST_TID_TYPE* iMySearch2  = CFst_STI_Init(itFvr,0,FSTI_SORTINI);              /* find trans. of act. child(2.level)*/
  BYTE*         lpTrans     = NULL;                                             /* transition of 1. level iteration  */
  BYTE*         lpTrans2    = NULL;                                             /* transition of 2. level iteration  */
  FST_ITYPE     nAux;                                                           /* Auxiliary value                   */
  FST_ITYPE     nAux2;                                                          /* Auxiliary value2                  */
  FST_ITYPE     nAux3       = 0;                                                /* Auxiliary value3                  */
  FST_ITYPE     nIsym       = NULL;                                             /* Input Symbol for store new values */
  FST_ITYPE     nRec;                                                           /* Number of transitions             */
  FST_ITYPE     nSym;                                                           /* Number of symbols                 */
  FST_ITYPE     nTransItDst;                                                    /* Transition iterator for new Fst   */
  FST_ITYPE     nTer;                                                           /* Auxiliary value for permutation   */
  FST_ITYPE     nU;                                                             /* Unit index in target              */
  BOOL          permutCheck = TRUE;                                             /* Checking the combination of symb. */
  BOOL          isNotLeaf   = FALSE;                                            /* Different mode to store           */
  FST_STYPE     nIsBo       = -1;                                               /* Symbol index of opening brace     */
  FST_STYPE     nIsBc       = -1;                                               /* Symbol index of closing brace     */
  FST_ITYPE*    p = NULL; p = (FST_ITYPE*) dlp_calloc(1,sizeof(FST_ITYPE));     /* Dyn. array for permutation        */

  /* Initialization */                                                          /* --------------------------------- */
  if (p == NULL){                                                               /* Check dynamic Array               */
   printf("Kein virtueller RAM mehr vorhanden ... !"); return ERR_MEM;}         /*                                   */
  CHECK_THIS_RV(NOT_EXEC);                                                      /* Check this instance               */
  if (itDst==NULL)                                                              /* Output not correct instance       */
    FVRT_EXCEPTION(ERR_NULLINST,"itDst is NULL",0,0);                           /*   Error message and exit          */
  if (itFvr==NULL)                                                              /* Input empty                       */
    return OK(IERROR(_this,ERR_NULLINST,"itFvr",0,0));                          /*   Error message and exit          */
  if (!CFvrtools_IsFvr(_this,0,itFvr))                                          /* Check Input is an FVR?            */
    FVRT_EXCEPTION(FVRT_NOTFVR,BASEINST(itFvr)->m_lpInstanceName,0,0);          /*   Error message and exit          */
  /* TODO: Use CREATEVIRTUAL!*/
  if (itDst==itFvr)                                                             /* Input and Output are not same     */
    FVRT_EXCEPTION(ERR_GENERIC,"Source and target must not be identical",0,0);  /*   Error message and exit          */

  ICREATEEX(CData,idRank,"CFvrtools_Synthesize~idRank",NULL);                   /*                                   */
  ICREATEEX(CData,idSym,"CFvrtools_Synthesize~idSym",NULL);                     /*                                   */
  ICREATEEX(CData,idSymRef,"CFvrtools_Synthesize~idSymRef",NULL);               /*                                   */
  ICREATEEX(CData,idSymList,"CFvrtools_Synthesize~idSymList",NULL);             /*                                   */
  ICREATEEX(CData,idStArray,"CFvrtools_Synthesize~idStArray",NULL);             /*                                   */
  ICREATEEX(CData,idStList,"CFvrtools_Synthesize~idStList",NULL);               /*                                   */
  ICREATEEX(CData,idTWeight,"CFvrtools_Synthesize~idTWeight",NULL);             /*                                   */
  CFst_Reset(BASEINST(itDst),TRUE);                                             /* Reset target                      */
  CFst_Rank(itFvr, 0, idRank);                                                  /* Get rank to discern by equal symb.*/
  CData_Copy(itDst->is,itFvr->is);                                              /* Copy input symbol table           */
  nIsBo = CFvrtools_FindIs("[",TRUE,itDst);                                     /* Find opening brace symbol         */
  nIsBc = CFvrtools_FindIs("]",TRUE,itDst);                                     /* Find closing brace symbol         */
  ISETOPTION(itDst,"/lsr"); ISETOPTION(itDst,"/fst");                           /* Set some options                  */
  nU = CFst_Addunit(itDst,"FVR");                                               /* Add a unit to the FVR sequence    */
  IRESETOPTIONS(itDst);                                                         /* Clear options                     */
  idDstTd = AS(CData,itDst->td);                                                /* Get transition table of itDst     */
  nCTIS = CData_FindComp(idDstTd,NC_TD_TIS);                                    /* Get comp. index of input symbol   */
  nCTOS = CData_FindComp(idDstTd,NC_TD_TOS);                                    /* Get comp. index of output symbol  */

  /* Add first 2 states and 1 transition as base */                             /* --------------------------------- */
  if ((nU=CFst_Addstates(itDst,0,2,0))<0)                                       /* Add first 2 states in target      */
    return IERROR(itDst,FST_INTERNAL,__FILE__,__LINE__,"");                     /* Check added state                 */
  CFst_AddtransIam(itDst, 0, nU, nU+1);                                         /* Add transition between both states*/
  CData_Dstore(idDstTd, 0, 0, nCTIS);                                           /* Store 0 for first input symbol    */
  CData_Dstore(idDstTd, CData_Dfetch(idRank,1,0), 0, nCTOS);                    /* save rank of trans. from source   */
  CData_AddComp(idSym,"Sym",T_LONG);    CData_Allocate(idSym,1);                /* Allocate memory for symbol(s)     */
  CData_AddComp(idSymRef,"Ref",T_LONG); CData_Allocate(idSymRef,1);             /* Allocate memory for Ref to symbol */
  CData_AddComp(idStArray,"Ref",T_LONG); CData_Allocate(idStArray,1);           /* Allocate memory for Ref to symbol */
  CData_AddComp(idTWeight,"~LSR",T_FLOAT); CData_Allocate(idTWeight,UD_XS(itFvr,0));/* Allocate memory for weight    */

  /* Start iteration over all states and collect needed information */          /* --------------------------------- */
  while (nMyIniState<=UD_XT(itFvr,0))                                           /* loop over all States of FVR       */
  {                                                                             /*                                   */
	CFst_STI_Done(iMySearch);
    iMySearch = CFst_STI_Init(itFvr,0,FSTI_SORTTER);                            /*                                   */
    if( (lpTrans=CFst_STI_TtoS(iMySearch, nMyIniState, NULL)) != NULL ){        /* Get input transition              */
      nTis = *CFst_STI_TTis(iMySearch, lpTrans);                                /* Get Tis to compare with child     */
      CData_Dstore(idTWeight,*CFst_STI_TW(iMySearch, lpTrans),nMyIniState,0);   /* Store weight of transition        */
    }                                                                           /*                                   */
    CData_Reallocate(idSym,0);                                                  /* Reset value for next permutation  */
    CData_Reallocate(idSymRef,0);                                               /* Reset value for next permutation  */
    CData_Reallocate(idStArray,0);                                              /* Reset value for next permutation  */

    CFst_STI_Done(iMySearch);
    nAux = 0; lpTrans = NULL; iMySearch = CFst_STI_Init(itFvr,0,FSTI_SORTINI);  /* --------------------------------- */
    while ((lpTrans=CFst_STI_TfromS(iMySearch,nMyIniState,lpTrans))!=NULL)      /* loop over trans. from act. node   */
    {                                                                           /*  to take in sep. list for permut  */
      FST_ITYPE nTerS = *CFst_STI_TTer(iMySearch,lpTrans);                      /* Get terminal state                */
      rank = CData_Dfetch(idRank,nTerS,0);                                      /*  ... to get rank of state         */
      FST_ITYPE nISym = *CFst_STI_TTis(iMySearch,lpTrans);                      /* Get input symbol                  */

      if ( nTis != nISym ){                                                     /* Is child and parent different     */
                                                                                /*  Take ...                         */
        CData_Reallocate(idSym,nAux+1);                                         /* reserve new memory for next value */
        CData_Dstore(idSym,nISym,nAux,0);                                       /*    symbol for Permutation         */
        CData_Reallocate(idSymRef,nAux+1);                                      /* reserve new memory for next Refv. */
        CData_Dstore(idSymRef,rank,nAux,0);                                     /*    ref. value to check sequence   */
        CData_Reallocate(idStArray,nAux+1);                                     /* reserve new memory for next Refv. */
        CData_Dstore(idStArray,nTerS,nAux++,0);                                 /*    ref. value to check save point */
        lpTrans2 = NULL; nAux2 = nTerS;                                         /* --------------------------------- */
        while((lpTrans2=CFst_STI_TfromS(iMySearch2,nAux2,lpTrans2))!=NULL)      /* Check child with same input sym.  */
        {                                                                       /* Dont if act. par. & child same... */
                                                                                /* ... notice before                 */
          FST_ITYPE nTerS2 = *CFst_STI_TTer(iMySearch2,lpTrans2);               /* Get terminal state of act. trans. */
          rank = CData_Dfetch(idRank,nTerS2,0);                                 /*  ... to get rank of state         */
          FST_ITYPE nISym2 = *CFst_STI_TTis(iMySearch2,lpTrans2);               /* Get input symbol of act. trans.   */
          if ( nISym2 == nISym ){                                               /* Take double input for permutation */
                                                                                /* Take ...                          */
            CData_Reallocate(idSym,nAux+1);                                     /* reserve new memory for next value */
            CData_Dstore(idSym,nISym,nAux,0);                                   /*    symbol for Permutation         */
            CData_Reallocate(idSymRef,nAux+1);                                  /* reserve new memory for next Refv. */
            CData_Dstore(idSymRef,rank,nAux,0);                                 /*    ref. value to check sequence   */
            CData_Reallocate(idStArray,nAux+1);                                 /* reserve new memory for next Refv. */
            CData_Dstore(idStArray,nTerS2,nAux++,0);                            /*    ref. value to check save point */
            lpTrans2 = NULL;                                                    /* reset value for next child        */
            nAux2 = nTerS2;                                                     /* set child as next node            */
          }
        }
      }/*if ( nTis != nISym )*/
    }/*while ((lpTrans=CFst_STI_TfromS(iMySearch,nMyIniState,lpTrans))!=NULL)*/

    /* Start permutation only when is more than one element */                  /* --------------------------------- */
    if (nAux > 0){                                                              /* start only if more than one symb. */
      dlp_free(p);
      p = (FST_ITYPE*) dlp_realloc(p, nAux, sizeof(FST_ITYPE));                 /* Auxiliary value for permutation   */
      nRecIdSym = CData_GetNRecs(idSym);                                        /* Get NRec of idSym                 */
      for(nAux = 0; nAux < nRecIdSym; nAux++) p[nAux]=nAux;                     /* fill aux. with series of numbers  */
      CData_Join(idSymList,idSym);                                              /* Save first sym. constellation     */
      CData_Join(idStList,idStArray);
      nAux = 1;                                                                 /* Iterator over symbols             */
      while (nAux < nRecIdSym){                                                 /* Start permutation algorithm       */
        p[nAux]--; nAux2 = (nAux % 2)*p[nAux];                                  /* Auxiliary values for permutation  */
        nAux3 = CData_Dfetch(idSym,nAux,0);                                     /* Swap values... Tis,               */
        CData_Dstore(idSym,CData_Dfetch(idSym,nAux2,0),nAux,0);                 /*                                   */
        CData_Dstore(idSym,nAux3,nAux2,0);                                      /*                                   */
        nAux3 = CData_Dfetch(idSymRef,nAux,0);                                  /*    ...Rank(reference),            */
        CData_Dstore(idSymRef,CData_Dfetch(idSymRef,nAux2,0),nAux,0);           /*                                   */
        CData_Dstore(idSymRef,nAux3,nAux2,0);                                   /*                                   */
        nAux3 = CData_Dfetch(idStArray,nAux,0);                                 /*    ...Ter(reference)              */
        CData_Dstore(idStArray,CData_Dfetch(idStArray,nAux2,0),nAux,0);         /*                                   */
        CData_Dstore(idStArray,nAux3,nAux2,0);                                  /*                                   */
        permutCheck = TRUE;                                                     /* Reset bool for permutation check  */
        for (nAux = 0; nAux < nRecIdSym; nAux++){                               /* Check actually permutation is ok  */
          for(nAux2 = 0; nAux2+nAux+1 < nRecIdSym; nAux2++){                    /*                                   */
            if(CData_Dfetch(idSym,nAux,0)==CData_Dfetch(idSym,nAux2+nAux+1,0)   /* Symbol1 == Symbol2 &&...          */
              && CData_Dfetch(idSymRef,nAux,0)>CData_Dfetch(idSymRef,nAux+nAux2+1,0))/*       ...Rank1 > Rank2       */
              permutCheck = FALSE;                                              /*                                   */
          }                                                                     /*                                   */
        }                                                                       /*                                   */
        for(nAux = 1; p[nAux] == 0; nAux++) p[nAux] = nAux;                     /* Reset aux. value for next permut. */
        if (permutCheck){                                                       /* Check is permutation ok?...       */
          CData_Join(idSymList,idSym);                                          /* ... than store the combination    */
          CData_Join(idStList,idStArray);                                       /* ... and combination of ref. value */
        }
      } /* End while (nAux < nRecIdSym) */

      /* Start to save in new FST */                                            /* --------------------------------- */
      nRec = CData_GetNRecs(idDstTd);                                           /* Get number of already exists trans*/
      for(nTransItDst = 0; nTransItDst < nRec; nTransItDst++){                  /* Iterate over this transistions    */
        isNotLeaf = FALSE;                                                      /* Reset bool                        */
        if((INT32)CData_Dfetch(idDstTd,nTransItDst,nCTIS) == nTis               /* Check input symbol is same        */
          && (FST_ITYPE)CData_Dfetch(idDstTd,nTransItDst,nCTOS) == (nMyIniState)){/* Check rank is same  */
          nTer = (INT32)CData_Dfetch(idDstTd,nTransItDst,0);                    /* Get terminal state of transition  */
          nComp = CData_GetNComps(idSymList);                                   /* Get number of possible permut.    */
          for(nAux2 = 0; nAux2 < nComp; nAux2++){                               /* ...loop over this to store permut.*/
            if(nAux2==0){                                                       /* First iteration, check is it leaf */
              for(nAux3=0; nAux3<nRec; nAux3++){                                /*    ...loop over tran.table        */
                if(nTer == (INT32)CData_Dfetch(idDstTd,nAux3,1)){               /* is nTer IniState -> its not a leaf*/
                  nTer = (INT32)CData_Dfetch(idDstTd,nAux3,0);                  /*    Get next terminal State        */
                  nIsym = (INT32)CData_Dfetch(idDstTd,nAux3,nCTIS);             /*    Get input symbol               */
                  nAux  = (INT32)CData_Dfetch(idDstTd,nAux3,nCTOS);             /*    Get output symbol              */
                  isNotLeaf = TRUE;                                             /*   Set bool to store values correct*/
                  break;                                                        /* there are no more terminal States */
                }
              }
            }
            nSym = CData_GetNRecs(idSymList)*3;                                 /* Number of symbols                 */
            if ((nU=CFst_Addstates(itDst,0,nSym,0))<0)                          /* Add states and get first IniTer   */
              return IERROR(itDst,FST_INTERNAL,__FILE__,__LINE__,"");           /* ...Error if not correct added     */
            if(nAux2 == 0 && isNotLeaf)                                         /* is not leaf than change first tran*/
              CData_Dstore(idDstTd,nU+nSym-1, nAux3,1);                         /* ...open chain and add already stat*/
            for(nAux3 = 0; nAux3 < nSym; nAux3++){                              /* Add new transitions               */
              if (nAux3 == 0)                                                   /* first tran to already exist state */
                CFst_AddtransIam(itDst, 0, (INT32)CData_Dfetch(idDstTd,nTransItDst,0), nU);/*Add trans.  */
              else{                                                             /* last transition, nTer is not leaf */
                CFst_AddtransIam(itDst, 0, nU, nU+1); nU++;                     /* Add transition                    */
              }                                                                 /* Store Value...                    */
              if ( nAux3 % 3 == 0){                                             /* Opening brace                     */
                  CData_Dstore(idDstTd,nIsBo,CData_GetNRecs(idDstTd)-1,nCTIS);
          	  	  CData_Dstore(idDstTd,0,CData_GetNRecs(idDstTd)-1,nCTOS);
              }
              else if ( nAux3 % 3 == 1){                                        /* Symbol and value                  */
                  CData_Dstore(idDstTd,CData_Dfetch(idSymList,(nAux3-1)/3,nAux2),CData_GetNRecs(idDstTd)-1,nCTIS);
                  CData_Dstore(idDstTd,CData_Dfetch(idStList,(nAux3-1)/3,nAux2),CData_GetNRecs(idDstTd)-1,nCTOS);
              }
              else if ( nAux3 % 3 == 2){                                        /* Closing brace                     */
                  CData_Dstore(idDstTd,nIsBc,CData_GetNRecs(idDstTd)-1,nCTIS);
              	  CData_Dstore(idDstTd,0,CData_GetNRecs(idDstTd)-1,nCTOS);
              }
              else
            	  printf("\n ERROR");
            }
            if(isNotLeaf && nAux2 > 0){
              CFst_AddtransIam(itDst, 0, nU, nTer);
              CData_Dstore(idDstTd,nIsym,CData_GetNRecs(idDstTd)-1,nCTIS);
              CData_Dstore(idDstTd,nAux, CData_GetNRecs(idDstTd)-1,nCTOS);
            }
          } /* for(nAux2 = 0; nAux2 < nComp; nAux2++){ */                       /* End of possible permutations      */
        } /* if((INT32)CData_Dfetch(idDstTd... */                               /* End of check input symbol and rank*/
      } /* for(nTransItDst = 0; nTransItDst < nRec; nTransItDst++){ */          /* End of loop over transitions      */
    } /* End if(nAux > 0){ */                                                   /* End of start if more than one symb*/

    CData_Reallocate(idSymList,0);                                              /* Reset list of input symbols       */
    CData_Reallocate(idStList,0);                                               /* Reset list of original state num. */
    dlp_free(p);
    p = (FST_ITYPE*) dlp_realloc(p, 1, sizeof(FST_ITYPE));                      /* Reset Auxiliary value for permut. */
    nMyIniState ++;                                                             /* Increment IniState for next state */
    CFst_Tree(itDst, itDst, 0);                                                 /* Make tree of added states         */
  }/* End while(nMyIniState<=UD_XT(itFvr,0)) */                                 /* End of loop over states of IniFst */

  /* Add weight and finite state */
  nAux = UD_XS(itDst,0);                                                        /* Auxiliary values to set final st. */
  CFst_STI_Done(iMySearch);
  iMySearch   = CFst_STI_Init(itDst,0,FSTI_SORTINI);                            /* Set search to find finite State   */
  for(nMyIniState=0; nMyIniState < nAux; nMyIniState++){                        /* Iterate over all new states       */
    if((lpTrans=CFst_STI_TfromS(iMySearch, nMyIniState, NULL)) == NULL)         /* Check state has not transition... */
      CData_Dstore(AS(CData,itDst->sd),1,nMyIniState,0); }                      /* ... yes? Than take it to finite   */
                                                                                /* --------------------------------- */
  CFst_STI_Done(iMySearch2);
  iMySearch2  = CFst_STI_Init(itDst,0,FSTI_SORTTER);                            /* Set search to find weight of trans*/
  for(nMyIniState=0; nMyIniState < nAux; nMyIniState++){                        /* Iterate over all new states       */
    if( (lpTrans2=CFst_STI_TtoS(iMySearch2, nMyIniState, NULL)) != NULL )       /* Check state has transition...     */
      CData_Dstore(idDstTd,CData_Dfetch(idTWeight,*CFst_STI_TTos(iMySearch2, lpTrans2),0),nMyIniState-1,4);
  }                                                                             /* Store weight                      */

  for(nAux=0; nAux < CData_GetNRecs(idDstTd); nAux++)
    CData_Dstore(idDstTd,-1,nAux,nCTOS);
  if(CFvrtools_IsFvr(_this, 0, itDst))
    nRet = O_K;

  ISETOPTION(itDst,"/lsr"); ISETOPTION(itDst,"/fsa");                           /* Set some options                  */
  IRESETOPTIONS(itDst);                                                         /* Clear options                     */

  /* Clean-up */                                                                /* --------------------------------- */
L_EXCEPTION:                                                                    /*                                   */
  IDESTROY(idRank);                                                             /*                                   */
  IDESTROY(idSym);                                                              /*                                   */
  IDESTROY(idSymRef);                                                           /*                                   */
  IDESTROY(idSymList);                                                          /*                                   */
  IDESTROY(idStArray);                                                          /*                                   */
  IDESTROY(idStList);                                                           /*                                   */
  IDESTROY(idTWeight);                                                          /*                                   */
  CFst_STI_Done(iMySearch);                                                     /*                                   */
  CFst_STI_Done(iMySearch2);                                                    /*                                   */
  dlp_free(p);																    /*                                   */
  return nRet;                                                                  /*                                   */
}

/*
 * Documentation in fvrtools.def
 */
INT16 CGEN_PUBLIC CFvrtools_Union(CFvrtools* _this, CFst* itDst, CFst* itFvr)
{
  INT16         nRet        = O_K;                                              /* The return value                  */
  FST_ITYPE     nIniStFvr   = 0;                                                /* Ini state of itFvr                */
  FST_ITYPE     nIniStDst   = 0;                                                /* Ini state of itDst                */
  FST_ITYPE     nTerStFvr   = NULL;                                             /* State iterator of original Fvr    */
  FST_ITYPE     nTerStDst   = NULL;                                             /* State iterator of original Dst    */
  FST_ITYPE     nIsFvr;                                                         /* Input symbol of Fvr node          */
  FST_ITYPE     nIsDst;                                                         /* Input symbol of Dst node          */
  FST_ITYPE     nAux;                                                           /* Auxilary value                    */
  FST_ITYPE     nCTIS;                                                          /* Index line ~TIS                   */
  FST_ITYPE     nCLsrFvr;                                                       /* Index line ~LSR of itFvr          */
  FST_ITYPE     nCLsrDst;                                                       /* Index line ~LSR of itDst          */
  FST_WTYPE     nWFvr;                                                          /* Weight of first Fvr               */
  FST_WTYPE     nWDst;                                                          /* Weight of second Fvr (Dst)        */
  FST_TID_TYPE* iSeTrFvr    = CFst_STI_Init(itFvr,0,FSTI_SORTTER);              /* find trans. of first itFvr        */
  FST_TID_TYPE* iSeTrDst    = CFst_STI_Init(itDst,0,FSTI_SORTTER);              /* find trans. of second itDst       */
  BYTE*         lpTrFvr     = NULL;                                             /* Transition of itFvr               */
  BYTE*         lpTrDst     = NULL;                                             /* Transition of itDst               */
  BOOL          SearchCompl = FALSE;                                            /* State of search                   */
  CData*        idFvrIs     = NULL;                                             /* InputSymbol table of itFvr        */
  CData*        idDstIs     = NULL;                                             /* InputSymbol table of itDst        */
  CData*        idTFvr      = NULL;                                             /* Transition table of Fvr           */
  CData*        idTDst      = NULL;                                             /* Transition table of Dst           */
  CData*        idSFvr      = NULL;                                             /* State table of Fvr                */
  CData*        idSDst      = NULL;                                             /* State table of Dst                */

  /* Initialization */                                                          /* --------------------------------- */
  CHECK_THIS_RV(NOT_EXEC);                                                      /* Check this instance               */
  if (itDst==NULL)                                                              /* Output not correct instance       */
    FVRT_EXCEPTION(ERR_NULLINST,"itDst is NULL",0,0);                           /*   Error message and exit          */
  if (itFvr==NULL)                                                              /* Output not correct instance       */
    FVRT_EXCEPTION(ERR_NULLINST,"itFvr is NULL",0,0);                           /*   Error message and exit          */
  if (!CFvrtools_IsFvr(_this,0,itDst))                                          /* Check Input is an FVR?            */
    FVRT_EXCEPTION(FVRT_NOTFVR,BASEINST(itDst)->m_lpInstanceName,0,0);          /*   Error message and exit          */
  if (!CFvrtools_IsFvr(_this,0,itFvr))                                          /* Check Input is an FVR?            */
    FVRT_EXCEPTION(FVRT_NOTFVR,BASEINST(itFvr)->m_lpInstanceName,0,0);          /*   Error message and exit          */
  idFvrIs = AS(CData,itFvr->is);                                                /* InputSymbol table of itFvr        */
  idDstIs = AS(CData,itDst->is);                                                /* InputSymbol table of itDst        */
  idTFvr = AS(CData, itFvr->td);                                                /* Sequence table of itFvr           */
  idTDst = AS(CData, itDst->td);                                                /* Sequence table of itDst           */
  idSFvr = AS(CData, itFvr->sd);                                                /* Sequence table of itFvr           */
  idSDst = AS(CData, itDst->sd);                                                /* Sequence table of itDst           */
  nCTIS = CData_FindComp(AS(CData,itDst->td),NC_TD_TIS);                        /* Get comp. index of input symbol   */
  nCLsrFvr = CData_FindComp(AS(CData,itFvr->td),NC_TD_LSR);                     /* Get comp. index of weight         */
  nCLsrDst = CData_FindComp(AS(CData,itDst->td),NC_TD_LSR);                     /* Get comp. index of weight         */

  /* Start iteration over all transition of Fvr */
  while (SearchCompl == FALSE){                                                 /* only on root is TtoS empty->TRUE  */
    while ((lpTrFvr=CFst_STI_TfromS(iSeTrFvr, nIniStFvr, lpTrFvr)) != NULL){    /* For all transition of Fvr-node    */
      nIsFvr = *CFst_STI_TTis(iSeTrFvr, lpTrFvr);                               /* Get input symbol of Fvr transition*/
      nTerStFvr = *CFst_STI_TTer(iSeTrFvr, lpTrFvr);                            /* Take next node of itFvr           */
      lpTrDst = NULL;
      while ((lpTrDst=CFst_STI_TfromS(iSeTrDst, nIniStDst, lpTrDst)) != NULL){  /* For all transition of Dst-node    */
        nIsDst = *CFst_STI_TTis(iSeTrDst, lpTrDst);                             /* Get input symbol of Dst transition*/
        nTerStDst = *CFst_STI_TTer(iSeTrDst, lpTrDst);                          /* Take next node of itDst           */
        if (strcmp(CData_Sfetch(idDstIs,nIsDst,0), CData_Sfetch(idFvrIs,nIsFvr,0)) == 0){  /* compare both symbols   */
          nIniStFvr = nTerStFvr;                                                /*  Next node is new ini node of Fvr */
          nIniStDst = nTerStDst;                                                /*  Next node is new ini node of Dst */
          nWFvr = CData_Dfetch(idTFvr, CFst_STI_GetTransId(iSeTrFvr, lpTrFvr), nCLsrFvr);  /* Get weight of itFvrtr  */
          nAux = CFst_STI_GetTransId(iSeTrDst, lpTrDst);                        /* Transition index of itDst         */
          nWDst = CData_Dfetch(idTDst, nAux, nCLsrDst);                         /* Get weight of itDst-transition    */
          CData_Dstore(idTDst,nWFvr + nWDst - nWFvr*nWDst,nAux,nCLsrDst);       /* Store new weight                  */
          break;                                                                /* Cancel-> only one symbol possible */
        }                                                                       /*                                   */
        else if(CData_Dfetch(idSFvr,nTerStFvr,0)==1 && CData_Dfetch(idSDst,nTerStDst,0)==1){/* Value on finale state */
          CFst_Deltrans(itDst, 0, CFst_STI_GetTransId(iSeTrDst, lpTrDst));      /*     Delete tr. after final state  */
          CFst_Delstate(itDst, 0, nTerStDst);                                   /*     Delete last node              */
          CFst_STI_UnitChanged(iSeTrDst,FSTI_CANY);                             /*     Update iterator               */
          if ((lpTrDst=CFst_STI_TfromS(iSeTrDst, nIniStDst, NULL)) == NULL)     /*     No more transitions?          */
            lpTrDst=CFst_STI_TtoS(iSeTrDst, nIniStDst, NULL);                   /*        than take prev. trans      */
          else nTerStDst = *CFst_STI_TTer(iSeTrDst, lpTrDst);                   /*     otherwise take terminal state */
          break;
        }
      }                                                                         /*                                   */
      if (lpTrDst == NULL){                                                     /* If input symbol not exist         */
        nTerStDst = CFst_Addstates(itDst,0,1,0);                                /* ... add new node                  */
        if(CData_Dfetch(idSFvr,nTerStFvr,0)==1){
          CData_Dstore(idSDst, 1, nTerStDst, 0);
        }
        nAux = CFst_Addtrans(itDst, 0 , nIniStDst, nTerStDst);                  /* ... add new transition to new node*/
        *(FST_ITYPE*)CData_XAddr(idTDst, nAux, nCTIS) = CFvrtools_FindIs(CData_Sfetch(idFvrIs,nIsFvr,0),TRUE,itDst);
        *(FST_WTYPE*)CData_XAddr(idTDst, nAux, nCLsrDst) = CData_Dfetch(idTFvr, CFst_STI_GetTransId(iSeTrFvr, lpTrFvr), nCLsrFvr);
                                                                                /* ^-- store new Is and weight       */
        CFst_STI_UnitChanged(iSeTrDst,FSTI_CANY);                               /* update transducer iterator        */
        nIniStFvr = *CFst_STI_TTer(iSeTrFvr, lpTrFvr);                          /* Take next node of itFvr           */
        nIniStDst = nTerStDst;                                                  /*  Next node is new ini node of Dst */
        lpTrFvr = NULL;                                                         /*  Reset lpTrFvr for next iteration */
      }                                                                         /*                                   */
      if (CFst_STI_TfromS(iSeTrFvr,nTerStFvr, NULL)==NULL){                     /* Check is next node a leaf         */
        continue;                                                               /*   ...take next iteration step     */
      }else{                                                                    /* New node is not a leaf...         */
        lpTrFvr = NULL;                                                         /*  Reset lpTrFvr for next iteration */
      }                                                                         /*                                   */
    }/* while((lpTrFvr=CFst_STI_TfromS(iSeTrFvr, nIniStFvr, lpTrFvr))!=NULL){*/ /* End of lpTrFvr for all Fvr-node  */
    lpTrFvr = CFst_STI_TtoS(iSeTrFvr, nIniStFvr, NULL);                         /* Take prev edge to ini node of Fvr */
    lpTrDst = CFst_STI_TtoS(iSeTrDst, nIniStDst, NULL);                         /* Take prev edge to ini node of Dst */
    if (lpTrFvr == NULL){                                                       /* is prev edge empty? (root)        */
      SearchCompl = TRUE;                                                       /*   ...set search complete          */
    }
    else{                                                                       /* otherwise                         */
      nIniStFvr = *CFst_STI_TIni(iSeTrFvr, lpTrFvr);                            /*   ...Take prev node to go back    */
      nIniStDst = *CFst_STI_TIni(iSeTrDst, lpTrDst);                            /*   ...Take prev node to go back    */
    }                                                                           /*                                   */
  } /* while (SearchCompl == NULL){ */                                          /* End of search                     */

  /* Clean-up */                                                                /* --------------------------------- */
L_EXCEPTION:                                                                    /* : Clean exit label                */
  CFst_STI_Done(iSeTrFvr);                                                      /*                                   */
  CFst_STI_Done(iSeTrDst);                                                      /*                                   */
  return nRet;                                                                  /* Return                            */
}

/*
 * Documentation in fvrtools.def
 */
BOOL CGEN_PUBLIC CFvrtools_Adjust(CFvrtools* _this, CFst* itWom, CFst* itInp, CFst* itQry){
  BOOL          nRet         = TRUE;                                            /* The return value                  */
  BOOL  		bTrFound     = FALSE;                                           /* Found transition of itInp         */
  FST_ITYPE     nIniStWom    = 0;                                               /* Unit to search in itWom           */
  FST_ITYPE     nIniStInp    = 0;                                               /* Unit to search in itInp           */
  FST_ITYPE     nIniStQry    = 0;                                               /* Unit to search in itInp           */
  FST_ITYPE     nTerStInp    = NULL;                                            /* State iterator of original Inp    */
  FST_ITYPE     nTerStQry    = NULL;                                            /* State iterator of original Qry    */
  FST_ITYPE     nIt          = 0;                                               /* Iterator                          */
  FST_ITYPE     nIsWom       = 0;                                               /* Input symbol of itWom             */
  FST_ITYPE     nIsInp       = 0;                                               /* Input symbol of itInp             */
  FST_ITYPE     nIsQry       = 0;                                               /* Input symbol of itQry             */
  FST_ITYPE     nTrId        = 0;                                               /* Transition ID            */
  FST_TID_TYPE* iSeTrWom     = NULL;                                            /* find trans. of itWom              */
  FST_TID_TYPE* iSeTrInp     = NULL;                                            /* find trans. of itInp              */
  FST_TID_TYPE* iSeTrQry     = NULL;                                            /* find trans. of itQry              */
  BYTE*         lpTrWom      = NULL;                                            /* Transition of itWom               */
  BYTE*         lpTrInp      = NULL;                                            /* Transition of itInp               */
  BYTE*         lpTrQry      = NULL;                                            /* Transition of itQry               */
  CData*        idWomIs      = NULL;                                            /* InputSymbol table of itWom        */
  CData*        idInpIs      = NULL;                                            /* InputSymbol table of itInp        */
  CData*        idQryIs      = NULL;                                            /* InputSymbol table of itInp        */
  CData*        idTInp       = NULL;                                            /* Transition table of Inp           */
  CData*        idTQry       = NULL;                                            /* Transition table of Qry           */
  CData*        idWomS       = NULL;                                            /* Transition table of Inp           */
  CData*        idSymPath    = NULL;                                            /* Data for path of symbol           */
  FST_ITYPE     nCTISInp;                                                       /* Index line ~TIS of itInp          */
  FST_ITYPE     nCTISQry;                                                       /* Index line ~TIS of itInp          */
  FST_ITYPE     nCLSRInp;                                                       /* Index line ~TIS of itInp          */
  FST_ITYPE     nCLSRQry;                                                       /* Index line ~TIS of itInp          */

  CHECK_THIS_RV(NOT_EXEC);                                                      /* Check this instance               */
  /* Simple tests */                                                            /* --------------------------------- */
  if (CData_IsEmpty(itWom->ud))                                                 /* FST is NULL                       */
    return OK(IERROR(_this,ERR_NULLINST,"itWom",0,0));                          /*   but must not be                 */
  if (CData_IsEmpty(itInp->ud))                                                 /* FST is NULL                       */
    return OK(IERROR(_this,ERR_NULLINST,"itInp",0,0));                          /*   but must not be                 */
  if (!CFvrtools_IsFvr(_this,0,itWom))                                          /* Check Input is an FVR?            */
    return OK(IERROR(_this,FVRT_NOTFVR,BASEINST(itWom)->m_lpInstanceName,0,0)); /*   but must not be                 */
  if (!CFvrtools_IsFvr(_this,0,itInp))                                          /* Check Input is an FVR?            */
    return OK(IERROR(_this,FVRT_NOTFVR,BASEINST(itInp)->m_lpInstanceName,0,0)); /*   but must not be                 */
  if (itWom==itInp)                                                             /* Input and Model are not same      */
    return OK(IERROR(_this,ERR_GENERIC,"Model and input must not be identical",0,0));/*   but must not be            */
  if (itWom==itQry)                                                             /* Input and Output are not same     */
    return OK(IERROR(_this,ERR_GENERIC,"Model and query must not be identical",0,0));/*   but must not be            */
  if (itInp==itQry)                                                             /* Input and Output are not same     */
    return OK(IERROR(_this,ERR_GENERIC,"Input and query must not be identical",0,0));/*   but must not be            */
  if (CData_GetNRecs(itWom->sd) < CData_GetNRecs(itInp->sd))                    /* Check size of itWom and itInp     */
    return OK(IERROR(_this,FALSE,"itInp bigger than itWom",0,0));               /*   itWom must be bigger            */

  ICREATEEX(CData,idSymPath,"CFvrtools_Adjust~idSymPath",NULL);                 /*                                   */
  CData_AddComp(idSymPath,"~TIS",T_LONG);                                       /* Allocate memory for symbol(s)     */
  CData_AddComp(idSymPath,"~LSR",T_FLOAT);                                      /* Allocate memory for symbol(s)     */

  idWomIs = AS(CData,itWom->is);                                                /* InputSymbol table of itWom        */
  idInpIs = AS(CData,itInp->is);                                                /* InputSymbol table of itInp        */
  idTInp  = AS(CData,itInp->td);                                                /* Sequence table of itInp           */
  idWomS  = AS(CData,itWom->sd);                                                /* Sequence table of itFvr           */
  idQryIs = AS(CData,itQry->is);                                                /* InputSymbol table of itQry        */
  idTQry  = AS(CData,itQry->td);                                                /* Transition table of itQry         */

  if (CData_FindComp(idTQry,NC_TD_TIS)==-1){                                    /* Check comp ~TIS exist >>          */
    CData_AddComp(idTQry, NC_TD_TIS, T_INT);                                    /*   Add comp ~TIS                   */
  }                                                                             /* <<                                */
  if (CData_FindComp(idTQry,NC_TD_LSR)==-1){                                    /* Check comp ~LSR exist >>          */
    CData_AddComp(idTQry, NC_TD_LSR, T_FLOAT);                                  /*   Add comp ~LSR                   */
  }                                                                             /* <<                                */
  nCTISQry = CData_FindComp(AS(CData,itQry->td),NC_TD_TIS);                     /* ...get comp index                 */
  nCLSRQry = CData_FindComp(AS(CData,itQry->td),NC_TD_LSR);                     /* ...get comp index                 */
  nCTISInp = CData_FindComp(AS(CData,itInp->td),NC_TD_TIS);                     /* Get comp. index of input symbol   */
  nCLSRInp = CData_FindComp(AS(CData,itInp->td),NC_TD_LSR);                     /* Get comp. index of input symbol   */

  iSeTrWom = CFst_STI_Init(itWom,0,FSTI_SORTTER);                               /* Find trans. of first itWom        */
  iSeTrInp = CFst_STI_Init(itInp,0,FSTI_SORTTER);                               /* Find trans. of second itInp       */

  /* Start function */                                                          /* --------------------------------- */
  while (TRUE){                                                                 /* Start Function break only on Root */
    while ((lpTrWom=CFst_STI_TfromS(iSeTrWom, nIniStWom, lpTrWom))!=NULL){      /* Iterate over itWom                */
      nIsWom = *CFst_STI_TTis(iSeTrWom, lpTrWom);                               /*   Get TIS of itWom transition     */
      bTrFound = FALSE;                                                         /*   Found symbol of itWom           */
      if (nIsWom==-1){                                                          /*   Should be value in itInp        */
        if((lpTrInp=CFst_STI_TfromS(iSeTrInp, nIniStInp, NULL))!=NULL){         /*   Is there a value?               */
          if(CData_Dfetch(idWomS,*CFst_STI_TTer(iSeTrInp, lpTrInp),0)==1) bTrFound = TRUE; /* Should be final state  */
        }                                                                       /*     Value found, all right        */
      }                                                                         /*                                   */
      else{                                                                     /*   OTHERWISE                       */
        CData_Reallocate(idSymPath,CData_GetNRecs(idSymPath)+1);                /*     store sym to remember on path */
        CData_Dstore(idSymPath, nIsWom, CData_GetNRecs(idSymPath)-1, 0);        /*     ...to built up itQry          */
        while ((lpTrInp=CFst_STI_TfromS(iSeTrInp, nIniStInp, lpTrInp))!=NULL){  /*     find correct trans of itInp   */
          nIsInp = *CFst_STI_TTis(iSeTrInp, lpTrInp);                           /*     Get TIS of itInp transition   */
          if (dlp_strcmp(CData_Sfetch(idInpIs,nIsInp,0), CData_Sfetch(idWomIs,nIsWom,0)) == 0){/*Compare both symbols*/
            CData_Dstore(idSymPath, CData_Dfetch(idTInp,CFst_STI_GetTransId(iSeTrInp,lpTrInp),nCLSRInp),CData_GetNRecs(idSymPath)-1,1);
            nIniStWom = *CFst_STI_TTer(iSeTrWom, lpTrWom);                      /*       Skip identic transition     */
            nIniStInp = *CFst_STI_TTer(iSeTrInp, lpTrInp);                      /*       ... take next states        */
            bTrFound = TRUE;                                                    /*       bool nothing to add in itQry*/
            break;                                                              /*       no more transitions possible*/
          }                                                                     /*     <<                            */
        }                                                                       /*   <<                              */
        if (lpTrInp == NULL){                                                   /*   when no correct transition found*/
          nTerStInp = CFst_Addstates(itInp,0,1,0);                              /*     Add new state                 */
          nTrId = CFst_Addtrans(itInp,0,nIniStInp,nTerStInp);                   /*     Add missing transition        */
          CFst_STI_UnitChanged(iSeTrInp, FSTI_CADD);                            /*     Uptade iterator               */
          nIniStInp = nTerStInp;                                                /*     New state is next state       */
          nIsInp = CFvrtools_FindIs(CData_Sfetch(idWomIs,nIsWom,0),TRUE,itInp); /*     Look for input symbol         */
          CData_Dstore(idTInp, nIsInp, nTrId, nCTISInp);                        /*     Store symbol ID in tr-table   */
          nIniStWom = *CFst_STI_TTer(iSeTrWom, lpTrWom);                        /*     Take next state for itWom     */
          nRet = FALSE;                                                         /*     It was not complete           */
        }                                                                       /*   <<                              */
      }                                                                         /* <<                                */

      if(!bTrFound){                                                            /*   Add features to itQry           */
        nRet = FALSE;                                                           /*     itInp and itWom not identic   */
        if(CData_IsEmpty(itQry->ud)){                                           /*     Is itQry empty? >>            */
          ISETOPTION(itQry,"/fsa");                                             /*       Set Option for fsa          */
          ISETOPTION(itQry,"/lsr");                                             /*       Set Option for fsa          */
          CFst_AddunitIam(itQry,"FVR");                                         /*       Add first unit              */
          CFst_Addstates(itQry,0,1,0);                                          /*       Add first state             */
        }                                                                       /*     <<                            */
        nIt = 0; nIniStQry = 0;                                                 /*     Reset iterator, start on root */
        while(nIt<CData_GetNRecs(idSymPath)){                                   /*     Iterator over found symbols   */
          lpTrQry = NULL;                                                       /*     Reset transition iterator     */
          while(iSeTrQry!=NULL && (lpTrQry=CFst_STI_TfromS(iSeTrQry,nIniStQry,lpTrQry))!=NULL){/* Trans. in itQry >> */
            nIsQry = *CFst_STI_TTis(iSeTrQry, lpTrQry);                         /*       Take symbol and compare >>  */
            if (dlp_strcmp(CData_Sfetch(idQryIs,nIsQry,0), CData_Sfetch(idWomIs,CData_Dfetch(idSymPath,nIt,0),0)) == 0){
              nIniStQry = *CFst_STI_TTer(iSeTrQry, lpTrQry);                    /*         Identic symbol found take */
              break;                                                            /*         ... next state and break  */
            }                                                                   /*       <<                          */
          }                                                                     /*     <<                            */
          if(lpTrQry==NULL){                                                    /*     No transition in itQry >>     */
            nTerStQry = CFst_Addstates(itQry,0,1,0);                            /*       Add new state               */
            nTrId = CFst_Addtrans(itQry,0,nIniStQry,nTerStQry);                 /*       Add transition              */
            nIniStQry = nTerStQry;                                              /*       New state is next state     */
            nIsQry = CFvrtools_FindIs(CData_Sfetch(idWomIs,CData_Dfetch(idSymPath,nIt,0),0),TRUE,itQry);/* Add symbol*/
            CData_Dstore(idTQry, nIsQry, nTrId, nCTISQry);                      /*       Store symbol in trans. table*/
            CData_Dstore(idTQry,CData_Dfetch(idSymPath,nIt,1),nTrId,nCLSRQry);  /*       Store weight in trans. table*/
            if (!iSeTrQry)                                                      /*       If iterator isnt active     */
              iSeTrQry = CFst_STI_Init(itQry,0,FSTI_SORTTER);                   /*         create iterator for itQry */
            else                                                                /*       OTHERWISE                   */
              CFst_STI_UnitChanged(iSeTrQry, FSTI_CADD);                        /*         Update iterator           */
          }                                                                     /*     <<                            */
          nIt++;                                                                /*     next symbol for next loop     */
        }                                                                       /*     <<                            */
      }                                                                         /*   <<                              */
    }                                                                           /* << End of "Iterate over itWom"    */

    if (lpTrWom==NULL){                                                         /* No trans in itWom go back >>      */
      lpTrWom = CFst_STI_TtoS(iSeTrWom, nIniStWom, NULL);                       /*   Get previous transition of itWom*/
      lpTrInp = CFst_STI_TtoS(iSeTrInp, nIniStInp, NULL);                       /*   Get previous transition of itInp*/
      if (lpTrWom==NULL){ break; }                                              /*   Root reached -> leaf main func. */
      nIniStWom = *CFst_STI_TIni(iSeTrWom, lpTrWom);                            /*   Get previous Ini state of itWom */
      nIniStInp = *CFst_STI_TIni(iSeTrInp, lpTrInp);                            /*   Get previous Ini state of itInp */
      CData_Reallocate(idSymPath,CData_GetNRecs(idSymPath)-1);                  /*   Delete last symbol              */
    }                                                                           /* <<                                */
  }                                                                             /* END OF while (TRUE)               */

  /* Clean-up */                                                                /* --------------------------------- */
  IDESTROY(idSymPath);                                                          /* Done data for symbol path         */
  CFst_STI_Done(iSeTrInp);                                                      /* Done iterator of itInp            */
  CFst_STI_Done(iSeTrWom);                                                      /* Done iterator of itWom            */
  if (iSeTrQry != NULL) CFst_STI_Done(iSeTrQry);                                /* Done iterator of itInp            */
  /*L_EXCEPTION:*/                                                              /* : Clean exit label                */
  return nRet;                                                                  /* Return                            */
}                                                                               /*                                   */

/*
 * Documentation in fvrtools.def
 */
FLOAT64 CGEN_PUBLIC CFvrtools_CompareWithModel(CFvrtools* _this, CFst* itWom, CFst* itInp){
  FLOAT64       nRet         = TRUE;                                            /* The return value                  */
  BOOL          bTrFound     = FALSE;                                           /* Found transition of itInp         */
  BOOL          bValFound    = FALSE;                                           /* Found value in itWom              */
  FST_ITYPE     nIniStWom    = 0;                                               /* Unit to search in itWom           */
  FST_ITYPE     nIniStInp    = 0;                                               /* Unit to search in itInp           */
  FST_ITYPE     nTerStWom    = NULL;                                            /* State iterator of original Wom    */
  FST_ITYPE     nIsWom       = 0;                                               /* Input symbol of itWom             */
  FST_ITYPE     nIsInp       = 0;                                               /* Input symbol of itInp             */
  FST_TID_TYPE* iSeTrWom     = NULL;                                            /* find trans. of first itWom        */
  FST_TID_TYPE* iSeTrInp     = NULL;                                            /* find trans. of second itInp       */
  BYTE*         lpTrWom      = NULL;                                            /* Transition of itWom               */
  BYTE*         lpTrInp      = NULL;                                            /* Transition of itInp               */
  CData*        idWomIs      = NULL;                                            /* InputSymbol table of itWom        */
  CData*        idInpIs      = NULL;                                            /* InputSymbol table of itInp        */
  CData*        idWomTd      = NULL;                                            /* InputSymbol table of itInp        */
  CData*        idInpTd      = NULL;                                            /* InputSymbol table of itInp        */
  CData*        idWomS       = NULL;                                            /* State table of Fvr                */
  CData*        idInpS       = NULL;                                            /* State table of Dst                */
  CData*        idAux        = NULL;                                            /* Auxillary data                    */
  FST_ITYPE     nCountWom    = 0;                                               /* Counter of itWom                  */
  FST_ITYPE     nCountInp    = 0;                                               /* Counter of itInp                  */
  FST_ITYPE     nCountVal    = 0;                                               /* Counter of abs. congruence value  */
  FST_ITYPE     nAux;                                                           /* Auxiliary value                   */

  ICREATEEX(CData,idAux,"CFvrtools_CompareWithModel~idAux",NULL);               /*                                   */

  CHECK_THIS_RV(NOT_EXEC);                                                      /* Check this instance               */
  /* Simple tests */                                                            /* --------------------------------- */
  if (itWom==NULL)                                                              /* FST is NULL                       */
    return OK(IERROR(_this,ERR_NULLINST,"itWom",0,0));                          /*   but must not be                 */
  if (itInp==NULL)                                                              /* FST is NULL                       */
    return OK(IERROR(_this,ERR_NULLINST,"itInp",0,0));                          /*   but must not be                 */
  if (!CFvrtools_IsFvr(_this,0,itWom))                                          /* Check Input is an FVR?            */
    FVRT_EXCEPTION(FALSE,BASEINST(itWom)->m_lpInstanceName,0,0);                /*   Error message and exit          */
  if (!CFvrtools_IsFvr(_this,0,itInp))                                          /* Check Input is an FVR?            */
    FVRT_EXCEPTION(FALSE,BASEINST(itInp)->m_lpInstanceName,0,0);                /*   Error message and exit          */

  idWomIs = AS(CData,itWom->is);                                                /* InputSymbol table of itWom        */
  idInpIs = AS(CData,itInp->is);                                                /* InputSymbol table of itInp        */
  idWomTd = AS(CData,itWom->td);                                                /* InputSymbol table of itInp        */
  idInpTd = AS(CData,itInp->td);                                                /* InputSymbol table of itInp        */
  idWomS = AS(CData, itWom->sd);                                                /* Sequence table of itFvr           */
  idInpS = AS(CData, itInp->sd);                                                /* Sequence table of itDst           */

  iSeTrWom = CFst_STI_Init(itWom,0,FSTI_SORTTER);                               /* Find trans. of first itWom        */
  iSeTrInp = CFst_STI_Init(itInp,0,FSTI_SORTTER);                               /* Find trans. of second itInp       */

  /* Count number of features, number of input features should not be bigger of number of schema features            */
  nCountWom = CData_GetNRecs(idWomTd);                                          /* Get number of Wom transitions     */
  nAux = 0; while(nAux <= CData_GetNRecs(idWomS)){                              /* Iteration over ini states         */
    if(CData_Dfetch(idWomS,nAux,0)==1)                                          /* Found final state                 */
      nCountWom--;                                                              /*   Count                           */
    nAux++;                                                                     /* next state                        */
  }                                                                             /*                                   */
  nAux = 0; while(nAux <= CData_GetNRecs(idInpS)){                              /* Iteration over states of itInp    */
    if(CData_Dfetch(idInpS,nAux,0)==1)                                          /* Found final state                 */
      nCountInp++;                                                              /*   Count value (final state)       */
    nAux++;                                                                     /* next state                        */
  }                                                                             /*                                   */
  if(nCountWom < CData_GetNRecs(idInpTd)-nCountInp){                            /* Compare time of features          */
    nRet = 0;                                                                   /*   to less features in itInp       */
    goto L_EXCEPTION;                                                           /*   breakup                         */
  }                                                                             /*                                   */

  /* Start function */                                                          /* --------------------------------- */
  nCountInp=0;                                                                  /* Now count conformable features    */
  while(TRUE){                                                                  /* Complete if itWom searched        */
    while ((lpTrWom=CFst_STI_TfromS(iSeTrWom, nIniStWom, lpTrWom))!=NULL){      /* Iterate over itWom >>             */
      nIsWom = *CFst_STI_TTis(iSeTrWom, lpTrWom);                               /*   Get TIS of itWom transition     */
      bValFound = FALSE;                                                        /*   Found symbol of itWom           */
      nTerStWom = *CFst_STI_TTer(iSeTrWom, lpTrWom);                            /*   Take terminal state...          */
      if(nIsWom!=-1 && CData_Dfetch(idWomS,nTerStWom,0)==1) bValFound = TRUE;   /*   to check is final state         */
                                                                                /*                                   */
      lpTrInp = NULL; bTrFound = FALSE;                                         /*   Reset iterator and bool         */
      while ((lpTrInp=CFst_STI_TfromS(iSeTrInp, nIniStInp, lpTrInp))!=NULL){    /*     find correct trans of itInp   */
        nIsInp = *CFst_STI_TTis(iSeTrInp, lpTrInp);                             /*     Get TIS of itInp transition   */
        if (dlp_strcmp(CData_Sfetch(idInpIs,nIsInp,0), CData_Sfetch(idWomIs,nIsWom,0)) == 0){/*Compare both symbols  */
          bTrFound = TRUE;                                                      /*       bool nothing to add in itQry*/
          break;                                                                /*       no more transitions possible*/
        }                                                                       /*     <<                            */
      }                                                                         /*   <<                              */
                                                                                /*                                   */
      if(bTrFound==TRUE){                                                       /*   When trans. found in itInp >>   */
        nIniStInp = *CFst_STI_TTer(iSeTrInp, lpTrInp);                          /*     Take next state of itInp      */
        nIniStWom = *CFst_STI_TTer(iSeTrWom, lpTrWom);                          /*     Take next state of itWom      */
        if(bValFound==FALSE) { nCountInp++; }                                   /*     No value --> count feature    */
        else{                                                                   /*     Otherwise                     */
          if (CData_Dfetch(idInpS,nIniStInp,0)==1) nCountVal++;                 /*       final state? count value    */
          else { nRet = 0; goto L_EXCEPTION; }                                  /*       otherwise error, there has  */
        }                                                                       /*         ... to be a value in itInp*/
        lpTrWom = NULL;                                                         /*     Reset for next iteration step */
      }else{                                                                    /*   << When transition not found >> */
        if(bValFound) { nRet = 0; goto L_EXCEPTION; }                           /*     Missing value in itInp        */
      }                                                                         /*   <<                              */
    }                                                                           /* <<                                */
                                                                                /*                                   */
    if (lpTrWom==NULL){                                                         /* No trans in itWom go back >>      */
      lpTrWom = CFst_STI_TtoS(iSeTrWom, nIniStWom, NULL);                       /*   Get previous transition of itWom*/
      lpTrInp = CFst_STI_TtoS(iSeTrInp, nIniStInp, NULL);                       /*   Get previous transition of itInp*/
      if (lpTrWom==NULL){ break; }                                              /*   Root reached -> leaf main func. */
      nIniStWom = *CFst_STI_TIni(iSeTrWom, lpTrWom);                            /*   Get previous Ini state of itWom */
      nIniStInp = *CFst_STI_TIni(iSeTrInp, lpTrInp);                            /*   Get previous Ini state of itInp */
    }                                                                           /* <<                                */
  }                                                                             /* END of while(TRUE)                */

  /* Relative number of matched feature + absolute congruence value                                                  */
  nRet = nCountVal + (FLOAT64) nCountInp / (FLOAT64) nCountWom;

  /* Clean-up */                                                                /* --------------------------------- */
L_EXCEPTION:                                                                    /* : Clean exit label                */
  IDESTROY(idAux);                                                              /*                                   */
  CFst_STI_Done(iSeTrInp);                                                      /* Done iterator of itInp            */
  CFst_STI_Done(iSeTrWom);                                                      /* Done iterator of itWom            */
  return nRet;                                                                  /* Return                            */
}

/*
 * Documentation in fvrtools.def
 */
const char* CGEN_PUBLIC CFvrtools_Hash(CFvrtools* _this, INT32 nU, CFst* itFvr){
  static char   lpName[L_NAMES] = "";                                           /* Hash value                        */
  INT16         nRet        = O_K;                                              /* Bool to check IsFvr               */
  FST_ITYPE     nIniStFvr   = 0;                                                /* Ini state of itFvr                */
  FST_ITYPE     nTerStFvr   = NULL;                                             /* State iterator of original Fvr    */
  CData*        idPaSy      = NULL;                                             /* Data to store features of path    */
  CData*        idPaSyList  = NULL;                                             /* Data to list paths                */
  CData*        idAux       = NULL;                                             /* Auxiliary data                    */
  BYTE*         lpTrFvr     = NULL;                                             /* Transition of itFvr               */
  FST_TID_TYPE* iSeTrFvr    = NULL;                                             /* find trans. of first itFvr        */

  CHECK_THIS_RV(NULL);                                                          /* Check this instance               */
  /* Simple tests */                                                            /* --------------------------------- */
  if (itFvr==NULL)                                                              /* FST is NULL                       */
    return (char*)OK(IERROR(_this,ERR_NULLINST,"itFvr",0,0));                   /*   but must not be                 */
  if (!CFvrtools_IsFvr(_this,nU,itFvr))                                         /* Check Input is an FVR?            */
    FVRT_EXCEPTION(FALSE,BASEINST(itFvr)->m_lpInstanceName,0,0);                /*   Error message and exit          */

  iSeTrFvr = CFst_STI_Init(itFvr,nU,FSTI_SORTINI);                              /* find trans. of first itFvr        */
  ICREATEEX(CData,idPaSy,"CFvrtools_Hash~idPaSy",NULL);                         /*                                   */
  ICREATEEX(CData,idPaSyList,"CFvrtools_Hash~idPaSyList",NULL);                 /*                                   */
  ICREATEEX(CData,idAux,"CFvrtools_Hash~idAux",NULL);                           /*                                   */
  CData_AddComp(idPaSy,"Ref",255); CData_Allocate(idPaSy,0);                    /* Allocate memory for Ref to symbol */

  while(TRUE){                                                                  /* List all paths                    */
    while((lpTrFvr = CFst_STI_TfromS(iSeTrFvr, nIniStFvr, lpTrFvr)) != NULL){   /* Take path of feature from FVR >>  */
      nTerStFvr = *CFst_STI_TTer(iSeTrFvr, lpTrFvr);                            /*   Take next node of itFvr         */
      CData_Reallocate(idPaSy,CData_GetNRecs(idPaSy)+1);                        /*   Get memory for next feature     */
      CData_Sstore(idPaSy, CData_Sfetch(AS(CData,itFvr->is),*CFst_STI_TTis(iSeTrFvr, lpTrFvr),0), CData_GetNRecs(idPaSy)-1, 0);
      if (CFst_STI_TfromS(iSeTrFvr, nTerStFvr, NULL)!=NULL){                    /*   Next state is not a leaf >>     */
        nIniStFvr = nTerStFvr;                                                  /*     Next state is next ini state  */
        lpTrFvr = NULL;                                                         /*     Reset iterator                */
      }else{                                                                    /*   << OTHERWISE >>                 */
        CData_Strop(idAux, idPaSy, NULL, "rcat");                               /*     All strings to one record     */
        CData_Cat(idPaSyList,idAux);                                            /*     Add string to a list          */
        CData_Reallocate(idPaSy,CData_GetNRecs(idPaSy)-1);                      /*     Delete last symbol            */
      }                                                                         /*   <<                              */
    }                                                                           /* <<                                */
    CData_Reallocate(idPaSy,CData_GetNRecs(idPaSy)-1);                          /* Delete last symbol                */
    lpTrFvr = CFst_STI_TtoS(iSeTrFvr, nIniStFvr, NULL);                         /* Take previous transition          */
    if(lpTrFvr == NULL) break;                                                  /* Root reached, all paths taken     */
    nIniStFvr = *CFst_STI_TIni(iSeTrFvr, lpTrFvr);                              /* Take previous state of transition */
  }                                                                             /*                                   */
  CData_Sortup(idPaSyList, idPaSyList,0);                                       /* Sort list                         */
  CData_Strop(idAux, idPaSyList, "CRC-32", "hash");                             /* take hash value of list           */
  dlp_strcpy(lpName,CData_Sfetch(idAux,0,0));                                   /* store hash value in string        */

  /* Clean-up */                                                                /* --------------------------------- */
  CFst_STI_Done(iSeTrFvr);                                                      /* Done iterator of itFvr            */
  IDESTROY(idPaSy);                                                             /* Destroy data of symbol path       */
  IDESTROY(idPaSyList);                                                         /* Destroy data of path list         */
  IDESTROY(idAux);                                                              /* Destroy auxiliary data            */
L_EXCEPTION:                                                                    /* : Clean exit label                */
  if(nRet==FALSE) dlp_strcpy(lpName,"FALSE");                                   /* Bool FALSE give string "FALSE"    */
  return lpName;                                                                /* Return string                     */
}


/* EOF */
