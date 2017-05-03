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
  BOOL          nRet        = TRUE;                                             /* Return value                      */
  FST_TID_TYPE* iSeTr       = NULL;                                             /* Find trans. of itFvr              */
  BYTE*         lpTr        = NULL;                                             /* Transition of itFvr               */
  FST_ITYPE     nState      = 0;                                                /* State of itFvr                    */
  FST_ITYPE     nIsFvr      = NULL;                                             /* Input symbol of itFvr             */
  FST_ITYPE     nCTIS       = NULL;                                             /* Find comp ~Tis                    */
  CData*        idIs        = NULL;                                             /* InputSymbol table of itFvr        */
  CData*        idTd        = NULL;                                             /* Sequence table of itFvr           */

  /* Test input */                                                              /* --------------------------------- */
  CHECK_THIS_RV(NOT_EXEC);                                                      /* Check this instance               */
  if (!CFvrtools_IsFvr(_this,nU,itFvr))                                         /* Check Input is an FVR?            */
    FVRT_EXCEPTION(FALSE,BASEINST(itFvr)->m_lpInstanceName,0,0);                /*   Error message and exit          */

  /* Initialization */                                                          /* --------------------------------- */
  idIs = AS(CData,itFvr->is);                                                   /* InputSymbol table of itFvrTwo     */
  idTd = AS(CData,itFvr->td);                                                   /* Sequence table of itFvrTwo        */
  nCTIS = CData_FindComp(AS(CData,itFvr->td),NC_TD_TIS);                        /* Get comp. index of input symbol   */
  iSeTr = CFst_STI_Init(itFvr,0,FSTI_SORTTER);                                  /* find trans. of itFvr              */

  /* Start iteration over all transition */                                     /* --------------------------------- */
  while(1){                                                                     /* First loop to iterate back        */
    while((lpTr=CFst_STI_TfromS(iSeTr, nState, lpTr))!=NULL && nRet){           /* Iterate over all transitions fwd  */
      nIsFvr = *CFst_STI_TTis(iSeTr, lpTr);                                     /*   Get input symbol of lpTr        */
      nState = *CFst_STI_TTer(iSeTr, lpTr);                                     /*   Take Ter node of lpTr           */
      lpTr = NULL;                                                              /*   Reset iterator before start     */
      if (strcmp(CData_Sfetch(idIs,nIsFvr,0), "INT") == 0){                     /*   Symbol is "INT"                 */
        lpTr=CFst_STI_TfromS(iSeTr, nState, NULL);                              /*     Take next transition          */
        if (lpTr == NULL ||                                                     /*     No value exist                */
            CFst_STI_TfromS(iSeTr, nState, lpTr) != NULL ||                     /*     Or more than one value exist  */
            CData_Dfetch(idTd,CFst_STI_GetTransId(iSeTr, lpTr),nCTIS) == -1 ||  /*     Or symbol is empty            */
            CFst_STI_TfromS(iSeTr,*CFst_STI_TTer(iSeTr, lpTr), NULL) != NULL){  /*     Or to much input after value  */
          nRet = FALSE; break;                                                  /*       Set return FALSE            */
        }                                                                       /*                                   */
        lpTr=CFst_STI_TtoS(iSeTr, nState, NULL);                                /*   Take prev. transition "INT"     */
        nState = *CFst_STI_TIni(iSeTr, lpTr);                                   /*   Take Ini node of lpTr           */
      }                                                                         /*                                   */
    }                                                                           /*                                   */
    lpTr = CFst_STI_TtoS(iSeTr, nState, NULL);                                  /* Take prev. trans to go back       */
    if (lpTr == NULL)                                                           /* If no trans exist? root reached   */
      break;                                                                    /*   leave function                  */
    else                                                                        /* Or else                           */
      nState = *CFst_STI_TIni(iSeTr, lpTr);                                     /*   get prev. state                 */
  }

  /* Clean-up */                                                                /* --------------------------------- */
L_EXCEPTION:                                                                    /* : Clean exit label                */
  CFst_STI_Done(iSeTr);                                                         /* Done iterator of itFvr            */
  return nRet;                                                                  /* Returng value                     */
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
  INT16 nRet = O_K;                                                             /* The return value                  */

  /* Initialization */                                                          /* --------------------------------- */
  CHECK_THIS_RV(NOT_EXEC);                                                      /* Check this instance               */
  if (itSeq==NULL)                                                              /* Output not correct instance       */
    FVRT_EXCEPTION(ERR_NULLINST,"itSrc is NULL",0,0);                           /*   Error message and exit          */
  /* Create wFVR from sequence FST  */                                          /* --------------------------------- */
  CFst_CopyUi(itFvr,itSeq,NULL,0);                                              /* Get the first unit                */
  IF_NOK(nRet=CFvrtools_CheckSeq(_this,itFvr,AS(CData,itFvr->is),NULL,NULL) )   /* Check sequence symbol table       */
    goto L_EXCEPTION;                                                           /* |                                 */
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
  nIsBo = CFvrtools_FindIs("[",FALSE,itFvr);                                    /* Find opening brace symbol         */
  nIsBc = CFvrtools_FindIs("]",FALSE,itFvr);                                    /* Find closing brace symbol         */
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
        if((INT32)CData_Dfetch(idDstTd,nTransItDst,2) == nTis                   /* Check input symbol is same        */
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

  CData_Copy(itDst->is,itFvr->is);                                              /* Copy input symbol table           */
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
  BOOL          SearchCompl = NULL;                                             /* State of search                   */
  CData*        idFvrIs     = NULL;                                             /* InputSymbol table of itFvr        */
  CData*        idDstIs     = NULL;                                             /* InputSymbol table of itDst        */
  CData*        idTFvr      = NULL;                                             /* Transition table of Fvr           */
  CData*        idTDst      = NULL;                                             /* Transition table of Dst           */

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
  nCTIS = CData_FindComp(AS(CData,itDst->td),NC_TD_TIS);                        /* Get comp. index of input symbol   */
  nCLsrFvr = CData_FindComp(AS(CData,itFvr->td),NC_TD_LSR);                     /* Get comp. index of weight         */
  nCLsrDst = CData_FindComp(AS(CData,itDst->td),NC_TD_LSR);                     /* Get comp. index of weight         */

  /* Start iteration over all transition of Fvr */
  while (SearchCompl == FALSE){                                                 /* only on root is TtoS empty->TRUE  */
    while ((lpTrFvr=CFst_STI_TfromS(iSeTrFvr, nIniStFvr, lpTrFvr)) != NULL){    /* For all transition of Fvr-node    */
      nIsFvr = *CFst_STI_TTis(iSeTrFvr, lpTrFvr);                               /* Get input symbol of Fvr transition*/
      lpTrDst = NULL;
      while ((lpTrDst=CFst_STI_TfromS(iSeTrDst, nIniStDst, lpTrDst)) != NULL){  /* For all transition of Dst-node    */
        nIsDst = *CFst_STI_TTis(iSeTrDst, lpTrDst);                             /* Get input symbol of Dst transition*/
        if (strcmp(CData_Sfetch(idDstIs,nIsDst,0), CData_Sfetch(idFvrIs,nIsFvr,0)) == 0){  /* compare both symbols   */
          nTerStDst = *CFst_STI_TTer(iSeTrDst, lpTrDst);                        /* take next node of itDst           */
          nWFvr = CData_Dfetch(idTFvr, CFst_STI_GetTransId(iSeTrFvr, lpTrFvr), nCLsrFvr);  /* Get weight of itFvrtr  */
          nAux = CFst_STI_GetTransId(iSeTrDst, lpTrDst);                        /* Transition index of itDst         */
          nWDst = CData_Dfetch(idTDst, nAux, nCLsrDst);                         /* Get weight of itDst-transition    */
          CData_Dstore(idTDst,nWFvr + nWDst - nWFvr*nWDst,nAux,nCLsrDst);       /* Store new weight                  */
          break;                                                                /* cancel-> only one symbol possible */
        }                                                                       /*                                   */
      }                                                                         /*                                   */
      nTerStFvr = *CFst_STI_TTer(iSeTrFvr, lpTrFvr);                            /* take next node of itFvr           */
      if (lpTrDst == NULL){                                                     /* if input symbol not exist         */
        nTerStDst = CFst_Addstates(itDst,0,1,0);                                /* ... add new node                  */
        nAux = CFst_Addtrans(itDst, 0 , nIniStDst, nTerStDst);                  /* ... add new transition to new node*/
        *(FST_ITYPE*)CData_XAddr(idTDst, nAux, nCTIS) = CFvrtools_FindIs(CData_Sfetch(idFvrIs,nIsFvr,0),TRUE,itDst);
        *(FST_WTYPE*)CData_XAddr(idTDst, nAux, nCLsrDst) = CData_Dfetch(idTFvr, CFst_STI_GetTransId(iSeTrFvr, lpTrFvr), nCLsrFvr);
        CFst_STI_UnitChanged(iSeTrDst,FSTI_CANY);                               /* ^-- store new Is and weight       */
                                                                                /* update transducer iterator        */
      }                                                                         /*                                   */
      if (CFst_STI_TfromS(iSeTrFvr,nTerStFvr, NULL)==NULL){                     /* Check is next node a leaf         */
        continue;                                                               /*   ...take next iteration step     */
      }else{                                                                    /* New node is not a leaf...         */
        nIniStFvr = nTerStFvr;                                                  /*  Next node is new ini node of Fvr */
        nIniStDst = nTerStDst;                                                  /*  Next node is new ini node of Dst */
        lpTrFvr = NULL;                                                         /*  Reset lpTrFvr for next iteration */
        lpTrDst = NULL;                                                         /*  Reset lpTrDst for next iteration */
      }                                                                         /*                                   */
    }/* while((lpTrFvr=CFst_STI_TfromS(iSeTrFvr, nIniStFvr, lpTrFvr))!=NULL){*/ /* End of lpTrFvr for all Fvr-node   */
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
BOOL CGEN_PUBLIC CFvrtools_Adjust(CFvrtools* _this, CFst* itWom, CFst* itInp){
  BOOL          nRet         = TRUE;                                            /* The return value                  */
  BOOL          bSearchCompl = FALSE;                                           /* State of search                   */
  BOOL  		bSearchFwd   = TRUE;                                            /* Direction of search               */
  BOOL  		bDelete      = FALSE;                                           /* Delete Path to last Square        */
  BOOL  		bAddInp      = FALSE;                                           /* Add part of path to itInp         */
  BOOL  		bAddWom      = FALSE;                                           /* Add part of path (value) to itWom */
  BOOL  		bTrFound     = FALSE;                                           /* Found transition of itInp         */
  FST_ITYPE     nIniStWom    = 0;                                               /* Unit to search in itWom           */
  FST_ITYPE     nIniStInp    = 0;                                               /* Unit to search in itInp           */
  FST_ITYPE     nTerStWom    = NULL;                                            /* State iterator of original Wom    */
  FST_ITYPE     nTerStInp    = NULL;                                            /* State iterator of original Inp    */
  FST_ITYPE     nIsWom       = 0;                                               /* Input symbol of itWom             */
  FST_ITYPE     nIsInp       = 0;                                               /* Input symbol of itInp             */
  FST_TID_TYPE* iSeTrWom     = NULL;                                            /* find trans. of first itWom        */
  FST_TID_TYPE* iSeTrInp     = NULL;                                            /* find trans. of second itInp       */
  BYTE*         lpTrWom      = NULL;                                            /* Transition of itWom               */
  BYTE*         lpTrInp      = NULL;                                            /* Transition of itInp               */
  BYTE*         lpTrAux      = NULL;                                            /* Altern. trans. of itInp and itWom */
  CData*        idWomIs      = NULL;                                            /* InputSymbol table of itWom        */
  CData*        idInpIs      = NULL;                                            /* InputSymbol table of itInp        */
  CData*        idTWom       = NULL;                                            /* Transition table of Wom           */
  CData*        idTInp       = NULL;                                            /* Transition table of Inp           */
  FST_ITYPE     nAux;                                                           /* Auxilary value                    */
  FST_ITYPE     nCTISWom;                                                       /* Index line ~TIS of itWom          */
  FST_ITYPE     nCTISInp;                                                       /* Index line ~TIS of itInp          */
  FST_ITYPE     nCLsrWom;                                                       /* Index line ~LSR of itWom          */
  FST_ITYPE     nCLsrInp;                                                       /* Index line ~LSR of itInp          */

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
  idTInp  = AS(CData,itInp->td);                                                /* Sequence table of itInp           */
  idTWom  = AS(CData,itWom->td);                                                /* Sequence table of itWom           */

  nCTISWom = CData_FindComp(AS(CData,itWom->td),NC_TD_TIS);                     /* Get comp. index of input symbol   */
  nCTISInp = CData_FindComp(AS(CData,itInp->td),NC_TD_TIS);                     /* Get comp. index of input symbol   */
  nCLsrWom = CData_FindComp(AS(CData,itWom->td),NC_TD_LSR);                     /* Get comp. index of weight         */
  nCLsrInp = CData_FindComp(AS(CData,itInp->td),NC_TD_LSR);                     /* Get comp. index of weight         */

  iSeTrWom = CFst_STI_Init(itWom,0,FSTI_SORTTER);                               /* find trans. of first itWom        */
  iSeTrInp = CFst_STI_Init(itInp,0,FSTI_SORTTER);                               /* find trans. of second itInp       */

  /* Start function */                                                          /* --------------------------------- */
  while(!bSearchCompl){                                                         /* Complete if itWom searched        */
    /* Start iteration over all transition of itWom compare with itInp and take right adjustment                     */
    while(bSearchFwd && !bAddWom && !bAddInp){                                  /* Start search in itWom             */
      if (bTrFound){                                                            /* If before trans in itInp found    */
        bTrFound = FALSE;                                                       /* ... reset to FALSE for next edge  */
        lpTrWom = NULL;                                                         /* ... reset lptrans for next node   */
      }                                                                         /*                                   */
      lpTrWom = CFst_STI_TfromS(iSeTrWom, nIniStWom, lpTrWom);                  /* Get next transition of itWom      */
      if (lpTrWom){                                                             /* Trans of itWom exist              */
        nTerStWom = *CFst_STI_TTer(iSeTrWom, lpTrWom);                          /*   take next node of itInp         */
        nIsWom = *CFst_STI_TTis(iSeTrWom, lpTrWom);                             /*   Get TIS of Wom transition       */
      }                                                                         /*                                   */
      lpTrInp = NULL;                                                           /* Reset lpTrans of itInp            */
      while ((lpTrInp=CFst_STI_TfromS(iSeTrInp, nIniStInp, lpTrInp)) != NULL    /* For all transition of itInp-node  */
          && lpTrWom != NULL){                                                  /*  and lpTrWom exist                */
        nIsInp = *CFst_STI_TTis(iSeTrInp, lpTrInp);                             /* Get input symbol of itInp         */
        if (strcmp(CData_Sfetch(idInpIs,nIsInp,0), CData_Sfetch(idWomIs,nIsWom,0)) == 0){ /* Compare both symbols    */
          nAux = CFst_STI_GetTransId(iSeTrInp, lpTrInp);                        /*   Get transition index of itInp   */
          CData_Dstore(idTWom, CData_Dfetch(idTInp, nAux, nCLsrInp), CFst_STI_GetTransId(iSeTrWom, lpTrWom),nCLsrWom);
                                                                                /*   ^--Store new weight in itWom    */
          nTerStInp = *CFst_STI_TTer(iSeTrInp, lpTrInp);                        /*   Take next node of itInp         */
          nIniStInp = nTerStInp;                                                /*   Set new initial state           */
          nIniStWom = nTerStWom;                                                /*   ... for next iteration step     */
          bTrFound = TRUE;                                                      /*   Trans. found -> reset loop      */
          break;                                                                /*   Break, only one symbol possible */
        }                                                                       /*                                   */
      }                                                                         /*                                   */
      if (lpTrWom == NULL && lpTrInp == NULL) bSearchFwd = FALSE;               /* No trans. on both FVR? -->Go Back */
      if (lpTrWom == NULL && lpTrInp != NULL){                                  /* No trans on itWom but on itInp    */
        bAddWom = TRUE;                                                         /*   Add value (transition) to itWom */
        lpTrAux = NULL;                                                         /*   Reset auxiliary variable        */
        nIsInp = *CFst_STI_TTis(iSeTrInp, lpTrInp);                             /*   Get input symbol of itInp       */
        while ((lpTrAux=CFst_STI_TfromS(iSeTrWom, nIniStWom, lpTrAux)) != NULL){/*   Check maybe symbol already exist*/
          nIsWom = *CFst_STI_TTis(iSeTrWom, lpTrAux);                           /*     Get input symbol of itWom     */
          if (strcmp(CData_Sfetch(idInpIs,nIsInp,0), CData_Sfetch(idWomIs,nIsWom,0)) == 0){/* Compare both symbols   */
            bAddWom = FALSE;                                                    /*        If exist reset bool        */
          }                                                                     /*                                   */
        }                                                                       /*                                   */
      }                                                                         /*                                   */
      else if (lpTrWom != NULL && lpTrInp == NULL){                             /* No trans on itInp but on itWom    */
        nIsWom = *CFst_STI_TTis(iSeTrWom, lpTrWom);                             /*   Get input symbol of itWom       */
        nRet = FALSE;                                                           /*   FVR changed so not identic      */
        if (strcmp("INT", CData_Sfetch(idWomIs,nIsWom,0)) != 0){                /*   Must not be "INT"               */
          bAddInp = TRUE;                                                       /*   Add transition to itInp         */
          lpTrAux = NULL;                                                       /*   Reset auxiliary variable        */
          while ((lpTrAux=CFst_STI_TfromS(iSeTrInp, nIniStInp, lpTrAux)) != NULL){/* Check maybe symbol already exist*/
            nIsInp = *CFst_STI_TTis(iSeTrInp, lpTrAux);                         /*     Get input symbol of itInp     */
            if (strcmp(CData_Sfetch(idInpIs,nIsInp,0), CData_Sfetch(idWomIs,nIsWom,0)) == 0){/* Compare both symbols */
              bAddInp = FALSE;                                                  /*        If exist reset bool        */
              nRet = TRUE;                                                      /*        ... and return value       */
            }                                                                   /*                                   */
          }                                                                     /*                                   */
        }                                                                       /*                                   */
      }                                                                         /*                                   */
    }/* End of while(bSearchFwd && !bAddWom && !bAddInp){ */                    /*                                   */
    /* Execute add transition and state to itInp */
    if (bAddInp == TRUE){                                                       /* While trans on itWom exist        */
      nTerStInp = CFst_Addstates(itInp,0,1,0);                                  /*   Add one node to itInp           */
      nAux = CFst_Addtrans(itInp, 0, nIniStInp, nTerStInp);                     /*   Add transition to this node     */
      nIsWom = *CFst_STI_TTis(iSeTrWom, lpTrWom);                               /*   Get input symbol of itWom       */
      *(FST_ITYPE*)CData_XAddr(idTInp, nAux, nCTISInp) = CFvrtools_FindIs(CData_Sfetch(idWomIs,nIsWom,0),TRUE,itInp);
      *(FST_WTYPE*)CData_XAddr(idTInp, nAux, nCLsrInp) = 0;                     /* ^-- Add symbol and set weight 0   */
      CFst_STI_UnitChanged(iSeTrInp,FSTI_CANY);                                 /*   Update iterator                 */
      bAddInp = FALSE; bTrFound = TRUE;                                         /*   Reset bool                      */
      nIniStInp = nTerStInp;                                                    /*   Set next initial state in itInp */
      nIniStWom = nTerStWom;                                                    /*   Set next initial state in itWom */
    }                                                                           /*                                   */
    /* Execute add transition and state to itWom (value) */
    if (bAddWom == TRUE){                                                       /* While trans on itInp exist        */
      nTerStWom = CFst_Addstates(itWom,0,1,0);                                  /*   Add one node to itWom           */
      nAux = CFst_Addtrans(itWom, 0, nIniStWom, nTerStWom);                     /*   Add transition to this node     */
      nIsInp = *CFst_STI_TTis(iSeTrInp, lpTrInp);                               /*   Get input symbol of itInp       */
      *(FST_ITYPE*)CData_XAddr(idTWom, nAux, nCTISWom) = CFvrtools_FindIs(CData_Sfetch(idInpIs,nIsInp,0),TRUE,itWom);
      *(FST_WTYPE*)CData_XAddr(idTWom, nAux, nCLsrWom) = CData_Dfetch(idTInp, CFst_STI_GetTransId(iSeTrInp, lpTrInp), nCLsrInp);
      CFst_STI_UnitChanged(iSeTrWom,FSTI_CANY);                                 /*   Update iterator                 */
      bAddWom = FALSE; bDelete = TRUE; bTrFound = TRUE;                         /*   Reset bool                      */
      nIniStInp = *CFst_STI_TTer(iSeTrInp, lpTrInp);                            /*   Set next initial state in itInp */
      nIniStWom = nTerStWom;                                                    /*   Set next initial state in itWom */
    }                                                                           /*                                   */
    /* Go back an search for next brunch or ending in root */
    while(!bSearchFwd && !bSearchCompl){                                        /* While trans exist on both FVR     */
      nTerStInp = nIniStInp;                                                    /*   Set last IniSt to TerSt of itInp*/
      nTerStWom = nIniStWom;                                                    /*   Set last IniSt to TerSt of itWom*/
      lpTrWom = CFst_STI_TtoS(iSeTrWom, nTerStWom, NULL);                       /*   Get last transition of itWom    */
      lpTrInp = CFst_STI_TtoS(iSeTrInp, nTerStInp, NULL);                       /*   Get last transition of itInp    */
      if (lpTrWom == NULL){bSearchCompl = TRUE; break;}                         /*   Root reached?-> Search complete */
      nIniStWom = *CFst_STI_TIni(iSeTrWom, lpTrWom);                            /*   Otherwise take previous IniSt   */
      nIniStInp = *CFst_STI_TIni(iSeTrInp, lpTrInp);                            /*   ... of itWom and itInp          */
      if (bDelete){                                                             /*   While itInp path alr. complete  */
        CFst_Deltrans(itInp,0,CFst_STI_GetTransId(iSeTrInp, lpTrInp));          /*     Delete last transition        */
        CFst_Delstate(itInp, 0, nTerStInp);                                     /*     Delete last node              */
        CFst_STI_UnitChanged(iSeTrInp,FSTI_CANY);                               /*     Update iterator               */
        lpTrInp = NULL;                                                         /*     lpTrInp not more exist        */
      }                                                                         /*                                   */
      if (CFst_STI_TfromS(iSeTrInp,nIniStInp,NULL)!=lpTrInp) bDelete = FALSE;   /*   Stop delete, other branch exist */
      if (CFst_STI_TfromS(iSeTrInp,nIniStInp,lpTrInp) != NULL){                 /*   Other branch not visited before */
        bSearchFwd = TRUE; bDelete = FALSE;                                     /*   Stop delete and search forward  */
      }                                                                         /*                                   */
    }                                                                           /*                                   */
  } /* END OF while(!bSearchCompl) */                                           /*                                   */

  /* Clean-up */                                                                /* --------------------------------- */
L_EXCEPTION:                                                                    /* : Clean exit label                */
  CFst_STI_Done(iSeTrInp);                                                      /* Done iterator of itInp            */
  CFst_STI_Done(iSeTrWom);                                                      /* Done iterator of itWom            */
  return nRet;                                                                  /* Return                            */
}                                                                               /*                                   */

/*
 * Documentation in fvrtools.def
 */
BOOL CGEN_PUBLIC CFvrtools_Compare(CFvrtools* _this, CFst* itFvrOne, CFst* itFvrTwo, const char* sOpname){
  BOOL          nRet        = TRUE;                                             /* The return value                  */
  BOOL          bSearchCompl = FALSE;                                           /* State of search                   */
  FST_ITYPE     nIniStOne   = 0;                                                /* Ini state of itFvrOne             */
  FST_ITYPE     nIniStTwo   = 0;                                                /* Ini state of itFvrTwo             */
  FST_ITYPE     nTerStOne   = NULL;                                             /* State iterator of original Wom    */
  FST_ITYPE     nTerStTwo   = NULL;                                             /* State iterator of original Inp    */
  FST_ITYPE     nIsOne      = 0;                                                /* Input symbol of itFvrOne          */
  FST_ITYPE     nIsTwo      = 0;                                                /* Input symbol of itFvrTwo          */
  FST_TID_TYPE* iSeTrOne    = NULL;                                             /* find trans. of first itFvrOne     */
  FST_TID_TYPE* iSeTrTwo    = NULL;                                             /* find trans. of second itFvrTwo    */
  BYTE*         lpTrOne     = NULL;                                             /* Transition of itFvrOne            */
  BYTE*         lpTrTwo     = NULL;                                             /* Transition of itFvrTwo            */
  CData*        idIsOne     = NULL;                                             /* InputSymbol table of itFvrOne     */
  CData*        idIsTwo     = NULL;                                             /* InputSymbol table of itFvrTwo     */
  CData*        idTOne      = NULL;                                             /* Transition table of Wom           */
  CData*        idTTwo      = NULL;                                             /* Transition table of Inp           */
  FST_ITYPE     nCLsrOne;                                                       /* Index line ~LSR of itFvr          */
  FST_ITYPE     nCLsrTwo;                                                       /* Index line ~LSR of itDst          */

  /* Simple tests */                                                            /* --------------------------------- */
  CHECK_THIS_RV(NOT_EXEC);                                                      /* Check this instance               */
  if (sOpname == NULL || sOpname == 0 || (strcmp(sOpname,"symbol")!=0 && strcmp(sOpname,"all")!=0)) /* Check option  */
    return IERROR(_this,DATA_OPCODE,sOpname,"string",0);                        /*   String not correct              */
  if (itFvrOne==NULL)                                                           /* FST is NULL                       */
    return OK(IERROR(_this,ERR_NULLINST,"itFvrOne",0,0));                       /*   but must not be                 */
  if (itFvrTwo==NULL)                                                           /* FST is NULL                       */
    return OK(IERROR(_this,ERR_NULLINST,"itFvrTwo",0,0));                       /*   but must not be                 */
  if (!CFvrtools_IsFvr(_this,0,itFvrOne))                                       /* Check Input is an FVR?            */
    FVRT_EXCEPTION(FALSE,BASEINST(itFvrOne)->m_lpInstanceName,0,0);             /*   Error message and exit          */
  if (!CFvrtools_IsFvr(_this,0,itFvrTwo))                                       /* Check Input is an FVR?            */
    FVRT_EXCEPTION(FALSE,BASEINST(itFvrTwo)->m_lpInstanceName,0,0);             /*   Error message and exit          */

  idIsOne = AS(CData,itFvrOne->is);                                             /* InputSymbol table of itFvrOne     */
  idIsTwo = AS(CData,itFvrTwo->is);                                             /* InputSymbol table of itFvrTwo     */
  idTOne  = AS(CData,itFvrTwo->td);                                             /* Sequence table of itFvrTwo        */
  idTTwo  = AS(CData,itFvrOne->td);                                             /* Sequence table of itFvrOne        */
  nCLsrOne = CData_FindComp(AS(CData,itFvrOne->td),NC_TD_LSR);                  /* Get comp. index of weight         */
  nCLsrTwo = CData_FindComp(AS(CData,itFvrTwo->td),NC_TD_LSR);                  /* Get comp. index of weight         */

  /* Short check */
  if (CData_GetNRecs(AS(CData,itFvrOne->sd))!=CData_GetNRecs(AS(CData,itFvrTwo->sd))) /* Different count of both FVRs*/
    return FALSE;                                                               /*          must not be              */
  if (CData_GetNRecs(idTOne)!=CData_GetNRecs(idTTwo))                           /* Different transition of both FVRs */
    return FALSE;                                                               /*   must not be                     */

  iSeTrOne = CFst_STI_Init(itFvrOne,0,FSTI_SORTTER);                            /* find trans. of first itFvrOne     */
  iSeTrTwo = CFst_STI_Init(itFvrTwo,0,FSTI_SORTTER);                            /* find trans. of second itFvrTwo    */

  /* Start function */                                                          /* --------------------------------- */
  while (!bSearchCompl){
    while ((lpTrOne=CFst_STI_TfromS(iSeTrOne, nIniStOne, lpTrOne)) != NULL && !bSearchCompl){ /* For all transition of itFvrOne-node*/
      nIsOne = *CFst_STI_TTis(iSeTrOne, lpTrOne);                               /* Get input symbol of itFvrTwo      */
      nTerStOne = *CFst_STI_TTer(iSeTrOne, lpTrOne);                            /*   take next node of itInp         */
      lpTrTwo = NULL;                                                           /* Reset iterator before start       */
      while ((lpTrTwo=CFst_STI_TfromS(iSeTrTwo, nIniStTwo, lpTrTwo)) != NULL){  /* For all transition of itFvrTwo-nod*/
        nIsTwo = *CFst_STI_TTis(iSeTrTwo, lpTrTwo);                             /* Get input symbol of itFvrTwo      */
        if (strcmp(CData_Sfetch(idIsTwo,nIsTwo,0), CData_Sfetch(idIsOne,nIsOne,0)) == 0){ /* Compare both symbols    */
          if (strcmp(sOpname,"all")==0 &&                                       /* Compare weight too */
              CData_Dfetch(idTOne, CFst_STI_GetTransId(iSeTrOne, lpTrOne), nCLsrOne) != CData_Dfetch(idTTwo, CFst_STI_GetTransId(iSeTrTwo, lpTrTwo), nCLsrTwo)){
              nRet = FALSE;                                                     /* Weight different Return False     */
              bSearchCompl = TRUE;                                              /* Search is now complete            */
          }                                                                     /*                                   */
          nTerStTwo = *CFst_STI_TTer(iSeTrTwo, lpTrTwo);                        /*   Take next node of itFvrTwo      */
          nIniStOne = nTerStOne;                                                /*   Set new initial state           */
          nIniStTwo = nTerStTwo;                                                /*   ... for next iteration step     */
          lpTrOne = NULL;                                                       /*   Reset iterator                  */
          break;                                                                /*   Break, only one symbol possible */
        }                                                                       /*                                   */
      }                                                                         /*                                   */
      if (lpTrOne != NULL && lpTrTwo == NULL){                                  /* Didnt found second symbol         */
        nRet = FALSE;                                                           /*   Return False                    */
        bSearchCompl = TRUE;                                                    /*   Search is now complete          */
      }                                                                         /*                                   */
    }                                                                           /*                                   */
    lpTrOne = CFst_STI_TtoS(iSeTrOne, nIniStOne, NULL);                         /* Go back take prev. transition     */
    lpTrTwo = CFst_STI_TtoS(iSeTrTwo, nIniStTwo, NULL);                         /* Go back take prev. transition     */
    if (lpTrOne == NULL && lpTrTwo == NULL)                                     /* Both prev. transition didnt exist */
        bSearchCompl = TRUE;                                                    /*   Search is complete              */
    else{                                                                       /* otherwise                         */
      nIniStOne = *CFst_STI_TIni(iSeTrOne, lpTrOne);                            /*   take prev node of itOne         */
      nIniStTwo = *CFst_STI_TIni(iSeTrTwo, lpTrTwo);                            /*   take prev node of itTwo         */
    }
  }

  /* Clean-up */                                                                /* --------------------------------- */
L_EXCEPTION:                                                                    /* : Clean exit label                */
  CFst_STI_Done(iSeTrOne);                                                      /* Done iterator of itFvrTwo         */
  CFst_STI_Done(iSeTrTwo);                                                      /* Done iterator of itFvrOne         */
  return nRet;                                                                  /* Return                            */
}

/* EOF */
