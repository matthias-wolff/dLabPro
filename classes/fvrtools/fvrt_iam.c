/* dLabPro class CFvrtools (fvrtools)
 * - Class CFvrtools interactive methods (user defined SMI functions)
 *
 * AUTHOR : Matthias Wolff, Werner Mexer
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
INT16 CGEN_PRIVATE CFvrtools_CheckSeq(CFvrtools* _this, CFst* itSeq)
{
  INT16 nRet = O_K;                                                             /* The return value                  */
  INT32 nI,nC;
  INT32 nXI=CData_GetNRecs(AS(CData,itSeq->is));
  INT32 nOI=CData_GetRecLen(AS(CData,itSeq->is));
  INT32 nXC=CData_GetCompType(AS(CData,itSeq->is),0);
  char *lpI;
  FST_TID_TYPE *lpTI;
  BYTE* lpT;

  /* Initialization */                                                          /* --------------------------------- */
  CHECK_THIS_RV(NOT_EXEC);                                                      /* Check this instance               */
  if (itSeq==NULL)                                                              /* Output not correct instance       */
    FVRT_EXCEPTION(ERR_NULLINST,"itDst is NULL",0,0);                           /*   Error message and exit          */

  /* Remove (XX) in output symbols & check for singular [ or ] */               /* --------------------------------- */
  lpI=(char*)CData_XAddr(AS(CData,itSeq->is),0,0);
  for(nI=0;nI<nXI;nI++,lpI+=nOI){
    INT32 nBrk=0;
    INT32 nCd=0;
    for(nC=0;nC<nXC && lpI[nC];nC++){
      if((lpI[nC]=='[' || lpI[nC]==']') && (nC || lpI[nC+1]))
        FVRT_EXCEPTION(FVRT_SEQSYNTAX,"symbol [ or ] occurs with other characters",0,0);
      if(lpI[nC]=='(') nBrk++;
      if(!nBrk) lpI[nCd++]=lpI[nC];
      if(lpI[nC]==')') nBrk--;
    }
    lpI[nCd]='\0';
  }

  /* Remove empty output symbols */
  lpI=(char*)CData_XAddr(AS(CData,itSeq->is),0,0);
  lpTI=CFst_STI_Init(itSeq,0,0);
  for(lpT=lpTI->lpFT ; lpT<lpTI->lpFT+lpTI->nRlt*lpTI->nXT ; lpT+=lpTI->nRlt){
    FST_STYPE *nTis=CFst_STI_TTis(lpTI,lpT);
    if(*nTis>=0 && *nTis<nXI && !lpI[*nTis*nOI]) *nTis=-1;
  }
  CFst_STI_Done(lpTI);

  /* Clean-up */                                                                /* --------------------------------- */
L_EXCEPTION:                                                                    /* : Clean exit label                */
  return nRet;                                                                  /* Return                            */
}

/*
 * Documentation in fvrtools.def
 */
INT16 CGEN_PUBLIC CFvrtools_FromFst(CFvrtools* _this, CFst* itSrc, CFst* itFvr)
{
  INT16 nRet = O_K;                                                             /* The return value                  */

  /* Initialization */                                                          /* --------------------------------- */
  CHECK_THIS_RV(NOT_EXEC);                                                      /* Check this instance               */
  if (itSrc==NULL)                                                              /* Output not correct instance       */
    FVRT_EXCEPTION(ERR_NULLINST,"itDst is NULL",0,0);                           /*   Error message and exit          */
  /* Create wFVR from sequence FST  */                                          /* --------------------------------- */
  CFst_CopyUi(itSrc,itFvr,NULL,0);                                              /* Get the first unit                */
  IF_NOK(nRet=CFvrtools_CheckSeq(_this,itFvr) ) goto L_EXCEPTION;               /* Check sequence symbol table       */
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
  INT16         nRet        = NULL;                                              /* The return value                  */
  FST_ITYPE     nTis        = -1;
  FST_ITYPE     nTisRank    = -1;
  FST_ITYPE     rank        = NULL;
  FST_ITYPE     nAuxState   = 0;
  FST_ITYPE     nMyUnit     = 0;
  FST_ITYPE     nMyIniState = 0;
  FST_ITYPE     nCTIS       = 0;
  FST_ITYPE     nRecIdSym   = 0;                                                /* variable for NRec of idSym        */
  CData*        idRank      = NULL;                                             /* rank of source Fvr                */
  CData*        idSym       = NULL;                                             /* Symbol constellation of actually permutation */
  CData*        idSymRef    = NULL;                                             /* Reference(rank) value of idSym */
  CData*        idSymList   = NULL;                                             /* All variation of symbol constellation*/
  CData*        idRefList   = NULL;                                             /* All reference values of symbol constellation */
  CData*        idStList    = NULL;                                             /* Store number of or. State as ref. */
  FST_TID_TYPE* iMySearch   = CFst_STI_Init(itFvr,nMyUnit,FSTI_SORTINI);        /* find trans. of act. node          */
  FST_TID_TYPE* iMySearch2  = CFst_STI_Init(itFvr,nMyUnit,FSTI_SORTINI);        /* find trans. of act. child(2.level)*/
  BYTE*         lpTrans     = NULL;
  BYTE*         lpTrans2    = NULL;
  BYTE*         lpT         = NULL;
  int           nAux;                                                           /* Auxiliary value                   */
  int           nAux2;                                                          /* Auxiliary value                   */
  int           nAux3;
  int           nAux4;
  int           nAux5;
  int           nAux6=0;
  int           nRec;
  FST_ITYPE nTransItDst;
  FST_ITYPE nTer;
  int           j;                                                              /* Auxiliary value for permutation   */
  INT32         nU          = -1;                                               /* Unit index in target              */
  FST_ITYPE     swapAuxTis;
  FST_ITYPE     swapAuxNode;
  BOOL          permutCheck = TRUE;
  BOOL          isNotLeaf = FALSE;
  FST_ITYPE*    p  = NULL; p  = (FST_ITYPE*) dlp_calloc(1,sizeof(FST_ITYPE));   /* Dyn. array for permutation        */

  /* Initialization */                                                          /* --------------------------------- */
  if (p == NULL){                                                               /* Check dynamic Array               */
   printf("Kein virtueller RAM mehr vorhanden ... !");
   return ERR_MEM;}
  CHECK_THIS_RV(NOT_EXEC);                                                      /* Check this instance               */
  if (itDst==NULL)                                                              /* Output not correct instance       */
    FVRT_EXCEPTION(ERR_NULLINST,"itDst is NULL",0,0);                           /*   Error message and exit          */
  if (itFvr==NULL)                                                              /* Input empty                       */
    return OK(IERROR(_this,ERR_NULLINST,"itFvr",0,0));                          /*   Error message and exit          */
  if (!CFvrtools_IsFvr(_this,0,itFvr))                                          /* Check Input is an FVR?            */
    FVRT_EXCEPTION(FVRT_NOTFVR,BASEINST(itFvr)->m_lpInstanceName,0,0);          /*   Error message and exit          */
  if (itDst==itFvr)                                                             /* Input and Output are not same     */
    FVRT_EXCEPTION(ERR_GENERIC,"Source and target must not be identical",0,0);  /*   Error message and exit          */
  ICREATEEX(CData,idRank,"CFvrtools_Synthesize~idRank",NULL);                   /* */
  ICREATEEX(CData,idSym,"CFvrtools_Synthesize~idSym",NULL);                     /* */
  ICREATEEX(CData,idSymRef,"CFvrtools_Synthesize~idSymRef",NULL);               /* */
  ICREATEEX(CData,idSymList,"CFvrtools_Synthesize~idSymList",NULL);             /* */
  ICREATEEX(CData,idRefList,"CFvrtools_Synthesize~idRefList",NULL);             /* */
  ICREATEEX(CData,idStList,"CFvrtools_Synthesize~idStList",NULL);               /* TODO: noch offen! */
  CFst_Check(itFvr);      //needed?
  CFst_Reset(BASEINST(itDst),TRUE);                                             /* Reset target                      */
  CFst_Rank(itFvr, 0, idRank);                                                  /* Get rank to discern by equal symb.*/

  ISETOPTION(itDst,"/lsr"); ISETOPTION(itDst,"/fst");                           /* Set some options                  */
  nU = CFst_Addunit(itDst,"");                                                  /* Add a unit to the FVR sequence    */
  IRESETOPTIONS(itDst);                                                         /* Clear options                     */

  nCTIS = CData_FindComp(AS(CData,itDst->td),NC_TD_TIS);

  /* Add first 2 states and 1 transition as base */                             /* --------------------------------- */
  if ((nU=CFst_Addstates(itDst,0,2,0))<0)                                       /* Add first 2 states in target      */
    return IERROR(itDst,FST_INTERNAL,__FILE__,__LINE__,"");                     /* Check added state                 */
  CFst_AddtransIam(itDst, 0, 0, 1);                                             /* Add transition between both states*/
  CData_Dstore(AS(CData,itDst->td), 0, 0, nCTIS);                               /* Store 0 for first input symbol    */
  CData_Dstore(AS(CData,itDst->td), CData_Dfetch(idRank,1,0), 0, 3);            /* save rank of trans. from source   */
  CData_AddComp(idSym,"Sym",T_LONG);    CData_Allocate(idSym,1);                /* Allocate memory for symbol(s)     */
  CData_AddComp(idSymRef,"Ref",T_LONG); CData_Allocate(idSymRef,1);             /* Allocate memory for Ref to symbol */

  /* Start iteration over all states and collect needed information */          /* --------------------------------- */
  while (nMyIniState<=UD_XT(itFvr,0))                                           /* loop over all States of FVR       */
  {                                                                             /*                                   */
    if( (lpT=CFst_STI_TtoS(iMySearch, nMyIniState, NULL)) != NULL )
      nTis = *CFst_STI_TTis(iMySearch, lpT);                                    /* get Tis to compare with child     */
    nTisRank = (FST_ITYPE)CData_Dfetch(idRank,nMyIniState,0);                   /* get Rank of actually State        */
    CData_Reallocate(idSym,0);
    CData_Reallocate(idSymRef,0);

    nAux = 0; lpTrans = NULL;                                                   /* --------------------------------- */
    while ((lpTrans=CFst_STI_TfromS(iMySearch,nMyIniState,lpTrans))!=NULL)      /* loop over trans. from act. node   */
    {                                                                           /*  to take in sep. list for permut  */
      FST_ITYPE nTerS = *CFst_STI_TTer(iMySearch,lpTrans);                      /* Get terminal state                */
      rank = CData_Dfetch(idRank,nTerS,0);                                      /*  ... to get rank of state         */
      FST_ITYPE nISym = *CFst_STI_TTis(iMySearch,lpTrans);                      /* Get input symbol                  */

      if ( nTis != nISym ){                                                     /* Is child and parent different     */
                                                                                /*  Take ...                         */
        CData_Reallocate(idSym, nAux+1);                                        /* reserve new memory for next value */
        CData_Dstore(idSym,nISym,nAux,0);                                       /*    symbol for Permutation         */
        CData_Reallocate(idSymRef, nAux+1);                                     /* reserve new memory for next Refv. */
        CData_Dstore(idSymRef,rank,nAux++,0);                                   /*    ref. value to check sequence   */

        lpTrans2 = NULL; nAuxState = nTerS;                                     /* --------------------------------- */
        while((lpTrans2=CFst_STI_TfromS(iMySearch2,nAuxState,lpTrans2))!=NULL)  /* Check child with same input sym.  */
        {                                                                       /* Dont if act. par. & child same... */
                                                                                /* ... notice before                 */
          FST_ITYPE nTerS2 = *CFst_STI_TTer(iMySearch2,lpTrans2);               /* Get terminal state of act. trans. */
          rank = CData_Dfetch(idRank,nTerS2,0);                                 /*  ... to get rank of state         */
          FST_ITYPE nISym2 = *CFst_STI_TTis(iMySearch2,lpTrans2);               /* Get input symbol of act. trans.   */
          if ( nISym2 == nISym ){                                               /* Take double input for permutation */
                                                                                /* Take ...                          */
            CData_Reallocate(idSym, nAux+1);                                    /* reserve new memory for next value */
            CData_Dstore(idSym,nISym,nAux,0);                                   /*    symbol for Permutation         */
            CData_Reallocate(idSymRef, nAux+1);                                 /* reserve new memory for next Refv. */
            CData_Dstore(idSymRef,rank,nAux++,0);                               /*    ref. value to check sequence   */
            lpTrans2 = NULL;                                                    /* reset value for next child        */
            nAuxState = nTerS2;                                                 /* set child as next node            */
          }//if ( nISym2 == nISym )
        }//while((lpTrans2=CFst_STI_TfromS(iMySearch2,nAuxState,lpTrans2))!=NULL)
      }//if ( nTis != nISym )
    }//while ((lpTrans=CFst_STI_TfromS(iMySearch,nMyIniState,lpTrans))!=NULL)

    /* Start permutation only when is more than one element */
    if (nAux > 0){                                                              /* --------------------------------- */
      p = (FST_ITYPE*) dlp_realloc(p, nAux, sizeof(FST_ITYPE));                 /* Auxiliary value for permutation   */
      nRecIdSym = CData_GetNRecs(idSym);                                        /* Get NRec of idSym                 */
      for(nAux = 0; nAux < nRecIdSym; nAux++){                                  /*   fill auxiliary value...         */
        p[nAux]=nAux;                                                           /*   ...with series of numbers       */
      }
      CData_Join(idSymList,idSym);                                              /* Save first sym. constellation     */
      CData_Join(idRefList,idSymRef);                                           /* Save first sym. constellation     */
      nAux = 1;
      while (nAux < nRecIdSym){   //while (SymArray[nAux] != -1){
        p[nAux]--; j = (nAux % 2)*p[nAux];
        //Swap Tis and Ter(reference)
          swapAuxTis = CData_Dfetch(idSym,nAux,0);
          CData_Dstore(idSym,CData_Dfetch(idSym,j,0),nAux,0);
          CData_Dstore(idSym,swapAuxTis,j,0);
          swapAuxNode = CData_Dfetch(idSymRef,nAux,0);
          CData_Dstore(idSymRef,CData_Dfetch(idSymRef,j,0),nAux,0);
          CData_Dstore(idSymRef,swapAuxNode,j,0);

        // Check actually permutation with using reference Data (idSymRef), is it allowed?
        permutCheck = TRUE;
        for (nAux = 0; nAux < nRecIdSym; nAux++){
          for(nAux2 = 0; nAux2+nAux+1 < nRecIdSym; nAux2++){
            if(CData_Dfetch(idSym,nAux,0) == CData_Dfetch(idSym,nAux2+nAux+1,0) && CData_Dfetch(idSymRef,nAux,0) > CData_Dfetch(idSymRef,nAux+nAux2+1,0) )
              permutCheck = FALSE;
          }
        }
        // Reset auxilary value for next iteration of permutation
        nAux = 1;
        while(p[nAux] == 0){
          p[nAux] = nAux;
          nAux++;
        }
        // write in list when combination is accept
        if (permutCheck){
          CData_Join(idSymList,idSym);
          CData_Join(idRefList,idSymRef);
        } // if (permutCheck)
      } // End while (nAux < CData_GetNRecs(idSym))

      /* ========================= */
      /* START TO SAVE IN NEW FST  */
      /* ========================= */

    // neue Variabelnzuordnung nAux == nTransItDst
    nTransItDst = nAux;
    nTer = nAux6; // Terminal State of parent Symbol

      nRec = CData_GetNRecs(AS(CData,itDst->td));         // in Tis (2) speichern!          // Verwendete Funktion: Unterschied zu CData_GetNRecs?
      for (nTransItDst = 0; nTransItDst < nRec; nTransItDst++){                 // Durchlauf der bisherigen Transitionen vom neuen FST (itDst)
        isNotLeaf = FALSE;
        //printf("\n Vergleich: %d == %d && %d == %d",(INT32)CData_Dfetch(AS(CData,itDst->td),nTransItDst,2), nTis, (FST_ITYPE)CData_Dfetch(AS(CData,itDst->td),nTransItDst,RNK),nTisRank);
        if((INT32)CData_Dfetch(AS(CData,itDst->td),nTransItDst,2) == nTis && (FST_ITYPE)CData_Dfetch(AS(CData,itDst->td),nTransItDst,3) == (nTisRank)){       // Vergleich von Input Symbol und Rankfolge um korrekte stelle zum anhängen der ermittelten Permutationsmöglichkeiten zu finden
          nTer = (INT32)CData_Dfetch(AS(CData,itDst->td),nTransItDst,0);        // td - Data: 0 (last value) -> Terminal State
          for(nAux3 = 0; nAux3 < CData_GetNComps(idSymList); nAux3++){    // Durchlauf aller gesammelter Permutationsmöglichkeiten
            if(nAux3==0){
              for(nAux5=0; nAux5<nRec; nAux5++){
                if ( nTer == (INT32)CData_Dfetch(AS(CData,itDst->td),nAux5,1) ){         // Wenn vorhanden: ist anzuknüpfender aktueller State kein Blatt --> bereits vorhandene Transition muss aufgebrochen und neu zugeordnet werden
                  nTer = (INT32)CData_Dfetch(AS(CData,itDst->td),nAux5,0);
                  nAux6 = (INT32)CData_Dfetch(AS(CData,itDst->td),nAux5,2);   // save inputsymbol
                  nAux  = (INT32)CData_Dfetch(AS(CData,itDst->td),nAux5,3);
                  isNotLeaf = TRUE;
                  break;
                }
              }
            }

            if ((nU=CFst_Addstates(itDst,0,CData_GetNRecs(idSymList),0))<0)   // Konnten States (in Abhängigkeit der Anzahl der Symbole) hinzugefügt werden?
              return IERROR(itDst,FST_INTERNAL,__FILE__,__LINE__,"");

            // Umschreiben der alten (bisherigen) Transition: Suche nachfolgende Transition. Ist sie vorhanden? So schreibe INI um, vom letzten hinzugefügten neuen State!
            // Suchen nachfolgende Transition    // aktueller terminal State ist Input State in nachfolgender Schleife
            if(nAux3 == 0 && isNotLeaf){
              CData_Dstore(AS(CData,itDst->td),nU+CData_GetNRecs(idSymList)-1, nAux5,1);      // öffne Kette indem bereits vorhandene Transition an letzten neuen State (als IniState) angefügt wird
            }

            // Füge nachfolgende neuen Transitionen hinzu
            for(nAux4 = 0; nAux4 < CData_GetNRecs(idSymList); nAux4++){
              if (nAux4 == 0){   // Knüpfe erste Transition an bereits vorhandenem State an
                CFst_AddtransIam(itDst, 0, (INT32)CData_Dfetch(AS(CData,itDst->td),nTransItDst,0), nU);
              }
              else if(nAux3>0 && nAux4 == CData_GetNRecs(idSymList)-1 && isNotLeaf){
                CFst_AddtransIam(itDst, 0, nU, nU+1);    //nU-(nAux3*(CData_GetNRecs(idSymList)-1)));
                nU++;
              }
              else{
                CFst_AddtransIam(itDst, 0, nU, nU+1);
                nU++;
              }
              CData_Dstore(AS(CData,itDst->td),       // hinzufügen des Symbols
                CData_Dfetch(idSymList,nAux4,nAux3),
                CData_GetNRecs(AS(CData,itDst->td))-1,
                2);
              CData_Dstore(AS(CData,itDst->td), CData_Dfetch(idRefList,nAux4,nAux3), CData_GetNRecs(AS(CData,itDst->td))-1, 3);
            }

            if(isNotLeaf && nAux3 > 0){
              CFst_AddtransIam(itDst, 0, nU, nTer);
              CData_Dstore(AS(CData,itDst->td),       // hinzufügen des Symbols
                nAux6,
                CData_GetNRecs(AS(CData,itDst->td))-1, 2);
              CData_Dstore(AS(CData,itDst->td), nAux, CData_GetNRecs(AS(CData,itDst->td))-1, 3);
            }
          } // for(nAux3 = 0; nAux3 < CData_GetNComps(idSymList); nAux3++){   // Durchlauf aller gesammelter Permutationsmöglichkeiten
        // reset new variable to control
        } // if((INT32)CData_Dfetch(AS(CData,itDst->td),nTransItDst,2) == nTis && (FST_ITYPE)CData_Dfetch(idRankItDst,nTransItDst,0) == (nTisRank)){       // Vergleich von Input Symbol und Rankfolge um korrekte stelle zum anhängen der ermittelten Permutationsmöglichkeiten zu finden
      } //       for (nTransItDst = 0; nTransItDst < nRec; nTransItDst++){                 // Durchlauf der bisherigen Transitionen vom neuen FST
    } // End if (nAux > 0){        // there is one edge? -> use permutation
    //Reset all variables
    CData_Reallocate(idSymList,0);
    CData_Reallocate(idRefList,0);
    p = (FST_ITYPE*) dlp_realloc(p, 1, sizeof(FST_ITYPE));
    //CData_Print(AS(CData,itDst->td));
    //printf("\n----------END-OF-NODE-%d---------\n", nMyIniState);
    nMyIniState ++;
    CFst_Tree(itDst, itDst, 0);
  }// End while (nMyIniState<=UD_XT(itFvr,0))
  CFst_Determinize(itDst,itDst,0);                    // Determinize zu umfangreiceh Berechnung --> Ersetzen durch einfachen durchlauf Funktion zur Markierung der Finite States!
  itDst->is = itFvr->is;
  if(CFvrtools_IsFvr(_this, 0, itDst))
    nRet = O_K;

L_EXCEPTION:
  CFst_STI_Done(iMySearch);
  CFst_STI_Done(iMySearch2);
  IDESTROY(idRank);
  IDESTROY(idSym);
  IDESTROY(idSymRef);
  IDESTROY(idSymList);
  IDESTROY(idRefList);
  dlp_free(p);                    // dlp_free(SymArray) Fehlermeldung! "xalloc: ERROR Pointer .... not found in xalloc ..."
  return nRet;
}

/* EOF */
