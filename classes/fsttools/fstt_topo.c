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

/*
 * Manual page at fst.def
 */
INT16 CGEN_PUBLIC CFsttools_RemoveExLoops(CFsttools* _this, INT32 nUnit,CFst * itFst)
{
  INT32 nU;                                                                      /* current unit index                */
  INT32 nCTIS=CData_FindComp(itFst->td,NC_TD_TIS);                               /* component index for TIS           */

  /* Validation */                                                              /* --------------------------------- */
  CHECK_THIS_RV(NOT_EXEC);                                                      /* check this pointer                */
  CFst_Check(itFst);                                                            /* check this (destination) instance */
  if (nUnit>=UD_XXU(itFst)) return IERROR(itFst,FST_BADID,"unit",nUnit,0);      /* check unit index                  */

  for(nU=nUnit>=0?nUnit:0;(nUnit>=0&&nU==nUnit)||nU<UD_XXU(itFst);nU++)         /* for the one or all source units   */
  {                                                                             /* >>                                */

    INT32 nT;                                                                    /*   transition index                */
    FST_ITYPE nFT=UD_FT(itFst,nU);                                              /*   first trans. of unit            */
    FST_ITYPE nLT=nFT+UD_XT(itFst,nU);                                          /*   last trans. of unit + 1         */
    INT32 nTremove=nFT-1;                                                        /*                                   */
    IFCHECKEX(1) printf("\n   Unit: %i/%i",nU,UD_XXU(itFst));                   /*   protocol                        */

    /* Sort transitions according to INI */                                     /*   ------------------------------- */
    FST_TID_TYPE *lpTIX=CFst_STI_Init(itFst,nU,FSTI_SORTINI);                   /*   sort transitions                */
    CFst_STI_Done(lpTIX);                                                       /*   STI end                         */

    /* Check all transitions */                                                 /*   ------------------------------- */
    for(nT=nFT;nT<nLT;)                                                         /*   loop over all trans. in unit    */
    {                                                                           /*   >>                              */

      INT32 nTnxt;                                                               /*     index of next trans.          */
      INT32 nTIS=(INT32)CData_Dfetch(itFst->td,nT,nCTIS);                         /*     TIS of nT                     */
      INT32 nTnxtStart=-1;                                                       /*     start-trans. for next-search  */
      FST_ITYPE nTER=TD_TER(itFst,nT);                                          /*     Ter-State of nT               */

      /* Search for trans. follwing nT */                                       /*     ----------------------------- */
      for(nTnxt=nTremove+1;nTnxt<nLT;nTnxt++)                                   /*     loop over trans.              */
        if(nT!=nTnxt && nTER==TD_INI(itFst,nTnxt)){                             /*       if nTnxt following nT       */
          if(nTnxtStart<0) nTnxtStart=nT;                                       /*         set search-start          */
          if(nTIS==(INT32)CData_Dfetch(itFst->td,nTnxt,nCTIS)) break;            /*         if TIS equal remove trans */
        }else if(nTER<TD_INI(itFst,nTnxt)){ nTnxt=nLT; break; }                 /*       no more trans. due to sort  */
      if(nTnxt>=nLT){ nTremove=nFT-1; nT++; continue; }                         /*     if nothing found continue nT+1*/

      /* Remove found transition */                                             /*     ----------------------------- */
      nTremove=nTnxt;                                                           /*     remove nTnxt                  */
      for(nTnxt=nTnxtStart<0?nFT:nTnxtStart;nTnxt<nLT;nTnxt++)                  /*     loop over pos. follow. trans. */
        if(nTnxt!=nTremove && nTnxt!=nT && nTER==TD_INI(itFst,nTnxt))           /*       if follow. trans. found     */
          TD_INI(itFst,nTnxt)=TD_TER(itFst,nTremove);                           /*         set start to end of nTrem.*/
        else if(nTER<TD_INI(itFst,nTnxt)) break;                                /*       no more trans. due to sort  */
      TD_TER(itFst,nT)=TD_TER(itFst,nTremove);                                  /*     skip nTremove                 */

    }                                                                           /*   <<                              */
  }                                                                             /* <<                                */

  CFst_Trim(itFst,nUnit,0);                                                     /* Trim orphaned states              */
  return O_K;                                                                   /* All done                          */
}

/*
 * Manual page at fst.def
 */
INT16 CGEN_PUBLIC CFsttools_OneTisPerPath(CFsttools* _this, INT32 nUnit,CFst *itFst)
{
  INT32 nNT;                                                                     /* Number of transitions             */
  INT32 nNGm=0;                                                                  /* Total number of input symbols     */
  INT32 nU;                                                                      /* Current unit index                */
  INT32 nT;                                                                      /* Current transition index          */
  INT32 nGm;                                                                     /* Current input symbol index        */
  INT32 nCTIS=CData_FindComp(itFst->td,NC_TD_TIS);                               /* Component index for input symbols */
  INT32 *lpStateForTIS=NULL;                                                     /* Map from input symbols to states  */
  INT32 nSetIni=-1;                                                              /* Adjust init of next trans.        */

  /* Validation */                                                              /* --------------------------------- */
  CHECK_THIS_RV(NOT_EXEC);                                                      /* check this pointer                */
  CFst_Check(itFst);                                                            /* check this (destination) instance */

  nNT=UD_XXT(itFst);                                                            /* Get number of transitions         */
  for(nT=0;nT<nNT;nT++)                                                         /* Loop over all transitions         */
  {                                                                             /* >>                                */
    nGm=(INT32)CData_Dfetch(itFst->td,nT,nCTIS);                                 /*   Read input symbol               */
    if(nGm>=nNGm) nNGm=nGm+1;                                                   /*   Adjust number of input symbols  */
  }                                                                             /* <<                                */
  lpStateForTIS=(INT32*)dlp_calloc(nNGm,sizeof(INT32));                           /* Alloc memory for buffer           */
  for(nU=nUnit>=0?nUnit:0;(nUnit>=0&&nU==nUnit)||nU<UD_XXU(itFst);nU++)         /* Loop over selected units          */
  {                                                                             /* >>                                */
    IFCHECKEX(1) printf("\n .. %i/%i",nU,UD_XXU(itFst));                        /*   Protocol                        */
    FST_ITYPE nFT=UD_FT(itFst,nU);                                              /*   Get first trans. index          */
    FST_ITYPE nLT=nFT+UD_XT(itFst,nU);                                          /*   Get last trans. index + 1       */
    /* Check all transition in unit */                                          /*   ------------------------------- */
    for(nT=nFT;nT<nLT;nT++)                                                     /*   Loop over transitions in unit   */
    {                                                                           /*   >>                              */
      /* If trans. is begin of a path, reset lpStateForTIS */                   /*     ----------------------------- */
      if(TD_INI(itFst,nT)==0 || nT==nFT)                                        /*     If trans. is begin of new path*/
      {                                                                         /*     >>                            */
        nSetIni=-1;                                                             /*       Reset nSetIni               */
        for(nGm=0;nGm<nNGm;nGm++) lpStateForTIS[nGm]=-1;                        /*       Reset input symbol map      */
      }                                                                         /*     <<                            */
      /* Adjust init of transition */                                           /*     ----------------------------- */
      if(nSetIni>=0)                                                            /*     If init of trans. to adjust   */
      {                                                                         /*     >>                            */
        TD_INI(itFst,nT)=nSetIni;                                               /*       Adjust init of trans.       */
        nSetIni=-1;                                                             /*       Reset nSetIni               */
      }                                                                         /*     <<                            */
      /* Update input symbol map or set trans. ter. and nSetIni */              /*     ----------------------------- */
      nGm=(INT32)CData_Dfetch(itFst->td,nT,nCTIS);                               /*     Get input symbol of trans.    */
      if(nGm>=0)                                                                /*     If input symbol exists        */
      {                                                                         /*     >>                            */
        if(lpStateForTIS[nGm]<0) lpStateForTIS[nGm]=TD_TER(itFst,nT);           /*       If IS not in path update map*/
        else nSetIni=TD_TER(itFst,nT)=lpStateForTIS[nGm];                       /*       Else set nSetIni and TER    */
      }                                                                         /*     <<                            */
    }                                                                           /*   <<                              */
  }                                                                             /* <<                                */
  dlp_free(lpStateForTIS);                                                      /* Free used memory                  */
  CFst_Trim(itFst,nUnit,0);                                                     /* trim orphaned states              */
  return O_K;                                                                   /* all done                          */
}

/*
 * Manual page at fst.def
 */
INT16 CGEN_PUBLIC CFsttools_RestoreLoops(CFsttools* _this, CFst *itSrc, INT32 nUnit, CFst *itDst)
{
  INT32 nU;                                                                      /* current unit index                */
  INT32 nCTIS=CData_FindComp(itDst->td,NC_TD_TIS);                               /* component index for TIS           */
  INT32 nGMs=0;                                                                  /* number of input symbols           */
  INT32 *lpLoopID;                                                               /* loop trans. id's in itSrc for TIS */
  INT32 nT;                                                                      /* transition index                  */

  /* Validation */                                                              /* --------------------------------- */
  CHECK_THIS_RV(NOT_EXEC);                                                      /* check this pointer                */
  CFst_Check(itDst);                                                            /* check this (destination) instance */
  CFst_Check(itSrc);                                                            /* check itSrc instance              */
  if (nUnit>=UD_XXU(itDst)) return IERROR(itDst,FST_BADID,"unit",nUnit,0);      /* check unit index                  */

  /* Get number of input symbols */                                             /* --------------------------------- */
  for(nT=0;nT<UD_XXT(itSrc);nT++)                                               /* loop over all trans. of itSrc     */
    if(TD_INI(itSrc,nT)==TD_TER(itSrc,nT)){                                     /*   if transition is a loop         */
      INT32 nTIS=(INT32)CData_Dfetch(itSrc->td,nT,nCTIS);                         /*     get input symbol              */
      if(nTIS>=nGMs) nGMs=nTIS+1;                                               /*     update number of input symbols*/
    }                                                                           /*   <<                              */

  /* Initialize lpLoopID */                                                     /* --------------------------------- */
  DLPASSERT((lpLoopID=(INT32 *)dlp_calloc(sizeof(INT32),nGMs)));                  /* allocate memory for lpLoopID      */
  for(nT=0;nT<nGMs;nT++) lpLoopID[nT]=-1;                                       /* initialize lpLoopID to -1         */

  /* Find loop transition id's in itSrc */                                      /* --------------------------------- */
  for(nT=0;nT<UD_XXT(itSrc);nT++)                                               /* loop over all trans. of itSrc     */
    if(TD_INI(itSrc,nT)==TD_TER(itSrc,nT)){                                     /*   if transition is a loop         */
      INT32 nTIS=(INT32)CData_Dfetch(itSrc->td,nT,nCTIS);                         /*     get input symbol              */
      if(nTIS>=0 && nTIS<nGMs) lpLoopID[nTIS]=nT;                               /*     if TIS is valid, set trans. id*/
    }                                                                           /*   <<                              */

  nCTIS=CData_FindComp(itDst->td,NC_TD_TIS);                                    /* get comp. index for TIS in itDst  */
  CFst_Hmm(itDst,itDst,nUnit);                                                  /* convert itDst to hmm              */

  /* Copy loops from itSrc to itDst according to lpLoopID */                    /* --------------------------------- */
  for(nU=nUnit>=0?nUnit:0;(nUnit>=0&&nU==nUnit)||nU<UD_XXU(itDst);nU++)         /* for the one or all source units   */
  {                                                                             /* >>                                */
    INT32 *lpStateTIS;                                                           /*   TIS assign to state             */
    INT32 nS;                                                                    /*   state index                     */
    /* Initialize lpStateTIS */                                                 /*   ------------------------------- */
    DLPASSERT((lpStateTIS=(INT32 *)dlp_calloc(sizeof(INT32),UD_XS(itDst,nU))));   /*   alloc memory for lpStateTIS     */
    for(nS=0;nS<UD_XS(itDst,nU);nS++) lpStateTIS[nS]=-1;                        /*   initialize lpStateTIS[] = -1    */
    /* Get TIS of states (TIS of state = TIS of trans. ending in state) */      /*   ------------------------------- */
    for(nT=UD_FT(itDst,nU);nT<UD_FT(itDst,nU)+UD_XT(itDst,nU);nT++){            /*   for all trans. in unit          */
      INT32 nTIS=(INT32)CData_Dfetch(itDst->td,nT,nCTIS);                         /*     get input symbol              */
      if(nTIS>=0 && nTIS<nGMs) lpStateTIS[TD_TER(itDst,nT)]=nTIS;               /*     set terminal-state TIS        */
    }                                                                           /*   <<                              */
    /* Copy loops from itSrc to itDst */                                        /*   ------------------------------- */
    for(nS=0;nS<UD_XS(itDst,nU);nS++)                                           /*   for all states in unit          */
      if(lpStateTIS[nS]>=0 && lpLoopID[lpStateTIS[nS]]>=0)                      /*     if state has TIS & loop found */
        CFst_AddtransCopy(itDst,nU,(FST_ITYPE)nS,(FST_ITYPE)nS,itSrc,(FST_ITYPE)lpLoopID[lpStateTIS[nS]]);       /*       copy loop from itSrc        */
    dlp_free(lpStateTIS);                                                       /*   free lpStateTIS                 */
  }                                                                             /* <<                                */
  free(lpLoopID);                                                               /* free lpLoopID                     */
  return O_K;                                                                   /* all done                          */
}

/*
 * Manual page at fst.def
 */
#define TD_START(fst,t)    (nDir?TD_TER(fst,t):TD_INI(fst,t))                    /* Start of trans. t according nDir  */
#define TD_END(fst,t)      (nDir?TD_INI(fst,t):TD_TER(fst,t))                    /* End of trans. t accoding nDir     */
INT16 CGEN_PUBLIC CFsttools_ComputePath(CFsttools* _this, CData *idTrans, INT32 nIt, CData *idPath,CFst *itFst)
{
  INT32 nCNld=CData_FindComp(idTrans,"~Nldavg");                                 /* component index of Nldavg         */
  INT32 nCRC=CData_FindComp(idTrans,"~RC");                                      /* component index of RC             */
  INT32 nU;                                                                      /* current unit index                */
  INT32 nS;                                                                      /* current state index               */
  INT32 nT;                                                                      /* current transition index          */
  INT32 NT=CData_GetNRecs(idTrans);                                              /* number of transitions             */
  INT32 nTStart;                                                                 /* start transition of path          */
  INT32 nDir;                                                                    /* direction of path                 */
  INT32 nItC;                                                                    /* iteration index                   */
  INT32 nLT;                                                                     /* last trans. of unit + 1           */
  INT32 nFT;                                                                     /* first trans. of unit              */
  FLOAT64 *lpPath=NULL;                                                          /* buffer for path-weights           */
  FLOAT64 *lpStates;                                                             /* first buffer for state-weights    */
  FLOAT64 *lpStates2;                                                            /* second buffer for state-weights   */
  FLOAT64 nMin;                                                                  /* minimum weight                    */
  FLOAT64 nNldnew;                                                               /* new Weight                        */

  /* Validation */                                                              /* --------------------------------- */
  CHECK_THIS_RV(NOT_EXEC);                                                      /* check this pointer                */
  CFst_Check(itFst);                                                            /* check this (destination) instance */

  /* Alloc and Initialize lpPath */                                             /* --------------------------------- */
  lpPath=(FLOAT64*)dlp_calloc(NT*(nIt+1),sizeof(FLOAT64));                        /* alloc memory for lpPath           */
  for(nT=0;nT<NT;nT++){                                                         /* loop over all transitions         */
    INT32 nRC=(INT32)CData_Dfetch(idTrans,nT,nCRC);                               /*   get RC of transition nT         */
    lpPath[nT*(nIt+1)]=nRC==0?1000:CData_Dfetch(idTrans,nT,nCNld);              /*   copy path weight from idTrans   */
    for(nItC=0;nItC<nIt;nItC++) lpPath[nT*(nIt+1)+nItC+1]=lpPath[nT*(nIt+1)];   /*   init all other path weights     */
  }                                                                             /* <<                                */

  /* Compute paths for all units */                                             /* --------------------------------- */
  for(nU=0;nU<UD_XXU(itFst);nU++)                                               /* loop over all units               */
  {                                                                             /* >>                                */
    IFCHECKEX(1) printf("\n   Unit: %i/%i",nU,UD_XXU(itFst));                   /*   protocol                        */
    lpStates=(FLOAT64*)dlp_calloc(UD_XS(itFst,nU),sizeof(FLOAT64));               /*   alloc memory for lpStates       */
    lpStates2=(FLOAT64*)dlp_calloc(UD_XS(itFst,nU),sizeof(FLOAT64));              /*   alloc memory for lpStates2      */
    nFT=UD_FT(itFst,nU);                                                        /*   get first trans. of unit        */
    nLT=UD_FT(itFst,nU)+UD_XT(itFst,nU);                                        /*   get last trans. of unit + 1     */
    for(nTStart=nFT;nTStart<nLT;nTStart++)                                      /*   loop over all trans. in unit    */
    {                                                                           /*   >>                              */
      IFCHECKEX(2) printf("\n   Unit: %i/%i Trans: %i/%i",                      /*     protocol                      */
        nU,UD_XXU(itFst),nTStart-UD_FT(itFst,nU),UD_XT(itFst,nU));              /*     |                             */
      for(nDir=0;nDir<2;nDir++)                                                 /*     calc forward and backw. path  */
      {                                                                         /*     >>                            */
        for(nS=0;nS<UD_XS(itFst,nU);nS++) lpStates[nS]=lpStates2[nS]=1000000;   /*       init lpStates* by max weight*/
        lpStates[TD_END(itFst,nTStart)]=0;                                      /*       path-begin-weight = 0       */
        for(nItC=0;nItC<nIt;nItC++)                                             /*       loop over itera. / path len */
        {                                                                       /*       >>                          */
          for(nT=nFT;nT<nLT;nT++)                                               /*         loop over all trans in u. */
          {                                                                     /*         >>                        */
            IFCHECKEX(3) printf("\n   Unit: %i/%i T1: %i/%i I: %i/%i T2: %i/%i",/*           protocol                */
              nU,UD_XXU(itFst), nTStart-UD_FT(itFst,nU),UD_XT(itFst,nU),        /*           |                       */
              nItC,nIt+1, nT-UD_FT(itFst,nU),UD_XT(itFst,nU));                  /*           |                       */
            if(lpStates[TD_START(itFst,nT)]<1000000)                            /*           path goes to trans-start*/
            {                                                                   /*           >>                      */
              nNldnew=lpStates[TD_START(itFst,nT)]+lpPath[nT*(nIt+1)];          /*             calc new state weight */
              if(nNldnew<lpStates2[TD_END(itFst,nT)])                           /*             new state w. better ? */
                lpStates2[TD_END(itFst,nT)]=nNldnew;                            /*               save new state w.   */
            }                                                                   /*           <<                      */
          }                                                                     /*         <<                        */
          /* Swap lpStates + Add path weight to lpPath */                       /*         ------------------------- */
          nMin=lpStates2[0];                                                    /*         set fist min              */
          for(nS=0;nS<UD_XS(itFst,nU);nS++)                                     /*         loop over all states      */
          {                                                                     /*         >>                        */
            if(lpStates2[nS]<nMin) nMin=lpStates2[nS];                          /*           update min of path-w.'s */
            lpStates[nS]=lpStates2[nS];                                         /*           swap lpStates           */
            lpStates2[nS]=1000000;                                              /*           init lpStates2          */
          }                                                                     /*         <<                        */
          lpPath[nTStart*(nIt+1)+nItC+1]+=nMin;                                 /*         add path-w. to lpPath     */
        }                                                                       /*       <<                          */
      }                                                                         /*     <<                            */
    }                                                                           /*   <<                              */
    dlp_free(lpStates);                                                         /*   free lpStates                   */
    dlp_free(lpStates2);                                                        /*   free lpStates2                  */
  }                                                                             /* <<                                */

  /* Replace 100000 by 0 in lpPath (not existing paths) */                      /* --------------------------------- */
  for(nT=0;nT<NT*(nIt+1);nT++) if(lpPath[nT]>=1000000) lpPath[nT]=0;            /* weight for not exist. path = 0    */

  /* Store lpPath to idPath */                                                  /* --------------------------------- */
  CData_Array(idPath,CData_GetCompType(idTrans,nCNld),nIt+1,NT);                /* create idPath as array (nIt+1)xNT */
  CData_DblockStore(idPath,lpPath,0,nIt+1,NT,-1);                               /* copy lpPath to idPath             */
  dlp_free(lpPath);                                                             /* free lpPath                       */

  /* Add ~Id to idPath */                                                       /* --------------------------------- */
  CData_AddComp(idPath,"~Id",T_LONG);                                           /* add ~Id-Component                 */
  CData_Mark(idPath,nIt+1,1);                                                   /* mark new component                */
  idPath->m_bMark=TRUE; CData_Fill(idPath,CMPLX(0),CMPLX(1));                   /* fill Trans-Id's in ~Id            */
  CData_Unmark(idPath);                                                         /* unmark idPath                     */
  return O_K;                                                                   /* all done                          */
}
#undef TD_START                                                                 /* remove makro                      */
#undef TD_END                                                                   /* remove makro                      */

struct fstt_usedpath {                                                          /* Struture for paths in history >>  */
  FLOAT64 nWeight;                                                               /*   Path weight                     */
  unsigned char *lpTransRC;                                                     /*   Trans. ref. counters for path   */
};                                                                              /* <<                                */

struct fstt_usedpathunit {                                                      /* Path collector per unit >>        */
  struct fstt_usedpath *lpUP;                                                   /*   Paths saved for this unit       */
  INT32 nUnitRC;                                                                 /*   Number of paths in lpUP         */
};                                                                              /* <<                                */

int fstt_cmp_usedpath(const void *sp1,const void *sp2){                         /* Path comperator function >>       */
  FLOAT64 w1=((struct fstt_usedpath *)sp1)->nWeight;                             /*   Get path weight of path 1       */
  FLOAT64 w2=((struct fstt_usedpath *)sp2)->nWeight;                             /*   Get path weight of path 2       */
  if(w1<w2) return -1;                                                          /*   Path 2 better ?                 */
  if(w2<w1) return 1;                                                           /*   Path 1 better ?                 */
  return 0;                                                                     /*   Paths equal ?                   */
}                                                                               /* <<                                */

/*
 * Manual page at fst.def
 */
INT16 CGEN_PUBLIC CFsttools_RcByUsedPath(CFsttools* _this, CData *idTransRC, FLOAT64 nPathWeight, INT32 nUnit, BOOL bReset, CData *idUnitTransCount,CFst *itFst)
{
  static struct fstt_usedpathunit *lpUPU=NULL;                                  /* Hist. for save trans. ref. cnt.'s */
  INT32 nNU;                                                                     /* Number of units                   */

  /* Validation */                                                              /* --------------------------------- */
  CHECK_THIS_RV(NOT_EXEC);                                                      /* Check this pointer                */
  CFst_Check(itFst);                                                            /* Check this (destination) instance */
  if (nUnit>=UD_XXU(itFst) || nUnit<0)                                          /* Check unit index                  */
    return IERROR(itFst,FST_BADID,"unit",nUnit,0);                              /* |                                 */

  nNU=UD_XXU(itFst);                                                            /* Get number of units               */

  /* Reset history if demanded or necessary */                                  /* --------------------------------- */
  if(!lpUPU || bReset)                                                          /* History should be reseted?        */
    {                                                                           /* >>                                */
    if(lpUPU)                                                                   /*   History exists -> free memory   */
      {                                                                         /*   >>                              */
      INT32 nU,nP;                                                               /*     Unit index and path index     */
      for(nU=0;nU<nNU;nU++) if(lpUPU[nU].lpUP)                                  /*     Loop over all units used      */
      {                                                                         /*     >>                            */
        for(nP=0;nP<lpUPU[nU].nUnitRC;nP++) if(lpUPU[nU].lpUP[nP].lpTransRC)    /*       Loop over all paths and     */
          free(lpUPU[nU].lpUP[nP].lpTransRC);                                   /*       | free path memory          */
        free(lpUPU[nU].lpUP);                                                   /*     Free path collector memory    */
      }                                                                         /*     <<                            */
      free(lpUPU);                                                              /*     Free unit memory              */
    }                                                                           /*   <<                              */
    DLPASSERT((lpUPU=(struct fstt_usedpathunit *)                               /*   Create new unit memory          */
      calloc(nNU,sizeof(struct fstt_usedpathunit))));                           /*   |                               */
  }                                                                             /* <<                                */

  /* Update history if new path given */                                        /* --------------------------------- */
  if(idTransRC)                                                                 /* If new path given                 */
  {                                                                             /* >>                                */
    INT32 nT;                                                                    /*   Transition index                */
    if(UD_XXT(itFst)!=CData_GetNRecs(idTransRC))                                /*   If number of trans. doesnt match*/
      IERROR(itFst,FST_INVALID,"number of trans. doesnt match idTransRC",0,0);  /*   | return error                  */
    lpUPU[nUnit].lpUP=(struct fstt_usedpath *)realloc(lpUPU[nUnit].lpUP,        /*   Get memory for new path in      */
      (lpUPU[nUnit].nUnitRC+1)*sizeof(struct fstt_usedpath));                   /*   | collector for unit            */
    lpUPU[nUnit].lpUP[lpUPU[nUnit].nUnitRC].nWeight=nPathWeight;                /*   Commit path weight              */
    DLPASSERT((lpUPU[nUnit].lpUP[lpUPU[nUnit].nUnitRC].lpTransRC=               /*   Get memory for new path         */
      (unsigned char *)calloc(UD_XT(itFst,nUnit),sizeof(unsigned char))));      /*   |                               */
    for(nT=0;nT<UD_XT(itFst,nUnit);nT++)                                        /*   Loop over all trans. of unit    */
      lpUPU[nUnit].lpUP[lpUPU[nUnit].nUnitRC].lpTransRC[nT]=                    /*   | Copy trans. ref. cnt.         */
        (unsigned char)CData_Dfetch(idTransRC,UD_FT(itFst,nUnit)+nT,0);         /*   | |                             */
    lpUPU[nUnit].nUnitRC++;                                                     /*   Increment unit path counter     */
  }                                                                             /* <<                                */

  /* Create new transition reference counters */                                /* --------------------------------- */
  if(idUnitTransCount)                                                          /* If new trans. ref. count. to crt. */
  {                                                                             /* >>                                */
    INT32 nU;                                                                    /*   Current unit index              */
    INT32 nT;                                                                    /*   Current trans. index            */
    INT32 nP;                                                                    /*   Current path index              */
    INT32 nNT;                                                                   /*   Number of transitions           */
    INT32 nNU;                                                                   /*   Number of units                 */
    INT32 nNPu;                                                                  /*   Number of paths per unit        */
    INT32 nNTnew;                                                                /*   Destinated number of trans.     */
    INT32 nNTnow;                                                                /*   Real new numb. of trans. per u. */
    INT32 nNTnowall=0;                                                           /*   Total new numb. of trans.       */
    FLOAT64 *lpRC;                                                               /*   New reference counters          */
    nNT=UD_XXT(itFst);                                                          /*   Get number of trans.            */
    nNU=UD_XXU(itFst);                                                          /*   Get number of units             */
    DLPASSERT((lpRC=(FLOAT64 *)calloc(nNT,sizeof(FLOAT64))));                     /*   Get memory for new ref. cnt.'s  */
    if(nNU!=CData_GetNRecs(idUnitTransCount))                                   /*   If number of units doesnt match */
      IERROR(itFst,FST_INVALID,"num. of units doesnt match idUnitTransCnt",0,0);/*   | return error                  */
    for(nU=0;nU<nNU;nU++)                                                       /*   Loop over all units             */
    {                                                                           /*   >>                              */
      nNPu=lpUPU[nU].nUnitRC;                                                   /*     Get number of paths per unit  */
      nNTnew=(INT32)CData_Dfetch(idUnitTransCount,nU,0);                         /*     Get dest. number of trans.    */
      printf("\n Try to reduce unit %i to %i/%i transitions.",                  /*     Protocol                      */
        nU,nNTnew,UD_XT(itFst,nU));                                             /*     |                             */
      /* Sort paths in current unit according there weight */                   /*     ----------------------------- */
      qsort(lpUPU[nU].lpUP,nNPu,sizeof(struct fstt_usedpath),fstt_cmp_usedpath);/*     Sort paths in unit            */
      /* Finding of new reference counters */                                   /*     ----------------------------- */
      nNTnow=0;                                                                 /*     Init number of new trans.     */
      for(nP=0;nP<nNPu && nNTnow<nNTnew;nP++)                                   /*     L. ov. sort. path until nNTnow*/
      {                                                                         /*     >>                            */
        nNTnow=0;                                                               /*       Init number of new trans.   */
        for(nT=0;nT<UD_XT(itFst,nU);nT++)                                       /*       Loop over all trans. in unit*/
        {                                                                       /*       >>                          */
          lpRC[UD_FT(itFst,nU)+nT]+=(INT32)lpUPU[nU].lpUP[nP].lpTransRC[nT];     /*         Update ref. cnt. by path  */
          if(lpRC[UD_FT(itFst,nU)+nT]) nNTnow++;                                /*         RC!=0 -> new trans.       */
        }                                                                       /*       <<                          */
      }                                                                         /*     <<                            */
      nNTnowall+=nNTnow;                                                        /*     Inc. total num. of new trans. */
      printf("\n Reduce unit %i to %i/%i transitions (%i/%i paths).",           /*     Protocol                      */
        nU,nNTnow,UD_XT(itFst,nU),nP,nNPu);                                     /*     |                             */
    }                                                                           /*   <<                              */
    CData_DcompStore(itFst->td,lpRC,CData_FindComp(itFst->td,NC_SD_RC),nNT);    /*   Store new ref. counters         */
    printf("\n All in all %i/%i transitions left.",nNTnowall,nNT);              /*   Protocol                        */
    free(lpRC);                                                                 /*   Free new ref. cnt.'s memory     */
  }                                                                             /* <<                                */

  return O_K;                                                                   /* All done                          */
}

/* EOF */
