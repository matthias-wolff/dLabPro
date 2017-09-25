/* dLabPro class CFvrtools (fvrtools)
 * - Class CFvrtools worker functions
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

/**
 * Finds a string in an FST's input symbol table.
 *
 * @param lpsStr
 *          The string to search.
 * @param bAdd
 *          Add the string if not found.
 * @param itFst
 *          The FST to search the string in.
 * @return The zero-based index of the string in the input symbol table of <code>itFst</code>, or -1 if the string
 *         was neither found nor added.
 */
FST_STYPE CGEN_SPROTECTED CFvrtools_FindIs(const char* lpsStr, BOOL bAdd, CFst* itFst)
{
  CData*    idIs = NULL;                                                        /* The input symbol table of itFst   */
  FST_STYPE nIs  = -1;                                                          /* The zero-based symbol index       */

  if (itFst==NULL) return -1;                                                   /* No FST, no service!               */
  idIs = AS(CData,itFst->is);                                                   /* Get input symbol table            */
  if (CData_GetNComps(idIs)==0)                                                 /* Table has no components           */
  {                                                                             /* >>                                */
    if (!bAdd) return -1;                                                       /*   If adding disabled -> forget it */
    CData_AddComp(idIs,"TIS",255);                                              /*   Add symbol component            */
  }                                                                             /* <<                                */
  nIs = CData_Find(idIs,0,CData_GetNRecs(idIs),1,0,lpsStr);                     /* Find the string                   */
  if (nIs>=0 || !bAdd) return nIs;                                              /* Found or adding disabled -> return*/

  nIs = CData_AddRecs(idIs,1,25);                                               /* Add record to symbol table        */
  CData_Sstore(idIs,lpsStr,nIs,0);                                              /* Store string                      */
  return nIs;                                                                   /* Return symbol index               */
}

/**
 * Finds a string in an FST's output symbol table.
 *
 * @param lpsStr
 *          The string to search.
 * @param bAdd
 *          Add the string if not found.
 * @param itFst
 *          The FST to search the string in.
 * @return The zero-based index of the string in the input symbol table of <code>itFst</code>, or -1 if the string
 *         was neither found nor added.
 */
FST_STYPE CGEN_SPROTECTED CFvrtools_FindOs(const char* lpsStr, BOOL bAdd, CFst* itFst)
{
  CData*    idOs = NULL;                                                        /* The input symbol table of itFst   */
  FST_STYPE nIs  = -1;                                                          /* The zero-based symbol index       */

  if (itFst==NULL) return -1;                                                   /* No FST, no service!               */
  idOs = AS(CData,itFst->os);                                                   /* Get input symbol table            */
  if (CData_GetNComps(idOs)==0)                                                 /* Table has no components           */
  {                                                                             /* >>                                */
    if (!bAdd) return -1;                                                       /*   If adding disabled -> forget it */
    CData_AddComp(idOs,"TOS",255);                                              /*   Add symbol component            */
  }                                                                             /* <<                                */
  nIs = CData_Find(idOs,0,CData_GetNRecs(idOs),1,0,lpsStr);                     /* Find the string                   */
  if (nIs>=0 || !bAdd) return nIs;                                              /* Found or adding disabled -> return*/

  nIs = CData_AddRecs(idOs,1,25);                                               /* Add record to symbol table        */
  CData_Sstore(idOs,lpsStr,nIs,0);                                              /* Store string                      */
  return nIs;                                                                   /* Return symbol index               */
}

/**
 * Adds a token to a sequence FST. Adds one state and one transition from the previous last state to the new state,
 * labels that transition with the name component of the token and weights it with the weight component. This member
 * function is invoked by {@link CFvrtools_StrToSeq}. There are no checks performed!
 *
 * @param _this
 *          This instance of CFvrtools.
 * @param lpsTok
 *          The token to add ("&lt;name&gt;" or "&lt;name&gt;:&lt;weight&gt;").
 * @param nU
 *          The unit number in <code>itSeq</code> to add the token to.
 * @param itSeq
 *          The sequence FST to add the token to.
 */
void CGEN_PROTECTED CFvrtools_AddToSeq(CFvrtools* _this, const char* lpsTok, INT32 nU, CFst* itSeq)
{
  char*   lpsT = NULL;                                                          /* Name component of token           */
  char*   lpsW = NULL;                                                          /* Weight component of token         */
  INT32   nIni = -1;                                                            /* Initial state of new transition   */
  INT32   nTer = -1;                                                            /* Terminal state of new transition  */
  INT32   nTis = -1;                                                            /* Input symbol index of new trans.  */
  FLOAT64 nW   = NAN;                                                           /* Weight of new transition          */
  INT32   nT   = -1;                                                            /* Absolute index of new transition  */
  CData*  idTd = NULL;                                                          /* Transition table of itSeq         */

  /* Prepare token */                                                           /* --------------------------------- */
  lpsT = (char*)dlp_calloc(dlp_strlen(lpsTok)+1,sizeof(char));                  /* Allocate string buffer            */
  dlp_strcpy(lpsT,lpsTok);                                                      /* Copy token                        */
  for (lpsW=lpsT; *lpsW!='\0'; lpsW++)                                          /* Loop over token characters        */
    if (*lpsW==':')                                                             /*   Name-weight delimiter found     */
      { *lpsW++='\0'; break; }                                                  /*     Split token                   */
  if (dlp_strlen(lpsW)>0)                                                       /* Have weight component             */
    dlp_sscanx(lpsW,T_DOUBLE,&nW);                                              /*   Get weight                      */

  /* Prepare target */                                                          /* --------------------------------- */
  if (UD_XS(itSeq,0)==0)                                                        /* No root state                     */
    CFst_Addstates(itSeq,nU,1,0);                                               /*   Create it                       */
  idTd = AS(CData,itSeq->td);                                                   /* Get transition table              */
  nTis = CFvrtools_FindIs(lpsT,TRUE,itSeq);                                     /* Find/add token in/to symbol tab.  */

  /* Add token to sequence */                                                   /* --------------------------------- */
  nIni = UD_XS(itSeq,0)-1;                                                      /* Get initial state ID              */
  nTer = CFst_Addstates(itSeq,nU,1,0)-UD_FS(itSeq,0);                           /* Add new state to sequence         */
  nT = CFst_Addtrans(itSeq,nU,nIni,nTer);                                       /* Add new transition                */
  CData_Dstore(idTd,nTis,nT,CData_FindComp(idTd,NC_TD_TIS));                    /* Store trans. input symbol index   */
  CData_Dstore(idTd,nW  ,nT,CData_FindComp(idTd,NC_TD_LSR));                    /* Store transition weight           */

  /* Clean-up */                                                                /* --------------------------------- */
  dlp_free(lpsT);                                                               /* Free string buffer                */
}

/**
 * Check a sequence FST symbol table.
 * This function fails if there are input symbols containing a square bracket [ or ] and other characters.
 * Furthermore this function removes all strings in parentheses (XX) from the input symbols, concatenates
 * them and stores the concatenation in <code>itSeq.ud.rtext</code>.
 * All transitions with empty string as input symbol afterwards are converted to epsilon transitions.
 * If <code>itSeq</code> is <code>NULL</code>, no changes are mode to <code>idS</code>.
 *
 * @param _this
 *          This instance of CFvrtools.
 * @param itSeq
 *          The sequence FST in unit 0 (maybe NULL for no changes).
 * @param idS
 *          The symbol table to check (maybe itSeq->is or itSeq->os).
 * @param nBO
 *          Return pointer for opening bracket index
 * @param nBC
 *          Return pointer for closing bracket index
 * @return <code>O_K</code> if successful, a (negative) error code otherwise.
 */
INT16 CGEN_PROTECTED CFvrtools_CheckSeq(CFvrtools* _this, CFst* itSeq, CData* idS, INT32 *pBO, INT32 *pBC)
{
  INT16 nRet = O_K;                                                             /* The return value                  */
  INT32 nI,nC;                                                                  /* Symbol, character index           */
  INT32 nXI;                                                                    /* Number of input symbols           */
  INT32 nOI;                                                                    /* Input symbol record length        */
  INT32 nXC;                                                                    /* Number of characters per symbol   */
  INT32 nBO = -1;                                                               /* Opening bracket index             */
  INT32 nBC = -1;                                                               /* Closing bracket index             */
  char *lpI;                                                                    /* Input symbol pointer              */
  FST_TID_TYPE* lpTI;                                                           /* Transition iterator               */
  BYTE* lpT = NULL;                                                             /* Transition pointer                */

  /* Initialization */                                                          /* --------------------------------- */
  CHECK_THIS_RV(NOT_EXEC);                                                      /* Check this instance               */
  if (idS==NULL || CData_IsEmpty(idS))                                          /* Symbol table is empty             */
    FVRT_EXCEPTION(ERR_NULLINST,"idS is empty",0,0);                            /*   Error message and exit          */

  nXI=CData_GetNRecs(idS);                                                      /* Get number of symbols             */
  nOI=CData_GetRecLen(idS);                                                     /* Get input symbol record length    */
  nXC=CData_GetCompType(idS,0);                                                 /* Get number of characters per symb.*/

  /* Check path integrity and get comments */                                   /* --------------------------------- */
  if (itSeq)                                                                    /* Have a sequence                   */
  {                                                                             /* >>                                */
    FST_ITYPE nS;                                                               /*   Current state                   */
    INT32 nLen = 0;                                                             /*   Comment buffer size             */
    char* lpO;                                                                  /*   Comment buffer                  */
    INT32 nOw;                                                                  /*   Comment buffer write index      */
    lpI = (char*)CData_XAddr(idS,0,0);                                          /*   Get symbol pointer              */
    lpTI = CFst_STI_Init(itSeq,0,0);                                            /*   Setup iterator without sorting  */

    /* Pass I: Check path integrity and get required comment buffer size */     /*   - - - - - - - - - - - - - - - - */
    for (nS=0; nS<lpTI->nXS; nS++)                                              /*   Loop over states                */
      SD_FLG(itSeq,nS+lpTI->nFS) &= ~SD_FLG_USER1;                              /*     Clear user flag #1            */
    nS = 0;                                                                     /*   Current state is the start state*/
    while ((lpT=CFst_STI_TfromS(lpTI,nS,NULL))!=NULL)                           /*   Traverse sequence               */
    {                                                                           /*   >>                              */
      if (CFst_STI_TfromS(lpTI,nS,lpT)!=NULL)                                   /*     Not a single path             */
        FVRT_EXCEPTION(FVRT_SEQSYNTAX,"path not unique",0,0);                   /*       Invalid argument!           */
      if (SD_FLG(itSeq,nS+lpTI->nFS) & SD_FLG_USER1)                            /*     We have been here before      */
        FVRT_EXCEPTION(FVRT_SEQSYNTAX,"input contains cycles",0,0);             /*       Invalid argument!           */
      nLen += dlp_strlen(&lpI[*CFst_STI_TTis(lpTI,lpT)*nOI]);                   /*     Accumulate req. buffer size   */
      SD_FLG(itSeq,nS+lpTI->nFS) |= SD_FLG_USER1;                               /*     Mark current state visited    */
      nS = *CFst_STI_TTer(lpTI,lpT);                                            /*     Get next state                */
    }                                                                           /*   <<                              */
    for (nS=0; nS<lpTI->nXS; nS++)                                              /*   Loop over states                */
      SD_FLG(itSeq,nS+lpTI->nFS) &= ~SD_FLG_USER1;                              /*     Clear user flag #1            */

    /* Pass II: Get comment */                                                  /*   - - - - - - - - - - - - - - - - */
    dlp_free(AS(CData,itSeq->ud)->m_lpTable->m_rtext);                          /*   Free itSeq.ud.rtext             */
    lpO = (char*)dlp_calloc(nLen+1,sizeof(char));                               /*   Allocate comment buffer         */
    AS(CData,itSeq->ud)->m_lpTable->m_rtext = lpO;                              /*   Make it the new itSeq.ud.rtext  */
    nOw = 0;                                                                    /*   Initialize write index          */
    nS = 0;                                                                     /*   Current state is the start state*/
    while ((lpT=CFst_STI_TfromS(lpTI,nS,NULL))!=NULL)                           /*   Traverse sequence               */
    {                                                                           /*   >>                              */
      INT32 nBrk = 0;                                                           /*     Current brace level is 0      */
      char* lpR = &lpI[*CFst_STI_TTis(lpTI,lpT)*nOI];                           /*     Pointer to input symbol       */
      for(nC=0; nC<nXC && lpR[nC]; nC++)                                        /*     Loop over characters          */
        if (lpR[nC]=='(')                                                       /*       Opening parenthesis         */
          nBrk++;                                                               /*         Count                     */
        else if (lpR[nC]==')')                                                  /*       Closing parenthesis         */
        {                                                                       /*       >>                          */
          nBrk--; if (nBrk<0) nBrk=0;                                           /*         Count, ignore parity      */
          lpO[nOw++] = ' ';                                                     /*         Write comment delimiter   */
        }                                                                       /*       <<                          */
        else if (nBrk>0)                                                        /*       Character in parentheses    */
          lpO[nOw++] = lpR[nC];                                                 /*         Write to comment          */
      nS = *CFst_STI_TTer(lpTI,lpT);                                            /*     Get next state                */
    }                                                                           /*   <<                              */
    dlp_strtrimright(lpO);                                                      /*   Trim tailing space from comment */

    CFst_STI_Done(lpTI);                                                        /*   Dispose of iterator             */
  }                                                                             /* <<                                */

  /* Remove (XX) in output symbols & check for singular [ or ] */               /* --------------------------------- */
  lpI=(char*)CData_XAddr(idS,0,0);                                              /* Get symbol pointer                */
  for(nI=0;nI<nXI;nI++,lpI+=nOI){                                               /* Loop over input symbols >>        */
    INT32 nBrk=0;                                                               /*   Bracket depth                   */
    INT32 nCd=0;                                                                /*   Character writing index         */
    for(nC=0;nC<nXC && lpI[nC];nC++){                                           /*   Loop over characters >>         */
      if((lpI[nC]=='[' || lpI[nC]==']') && (nC || lpI[nC+1]))                   /*     Check for singular [ or ]     */
        FVRT_EXCEPTION(FVRT_SEQSYNTAX,"Symbol [ or ] occurs with other characters",0,0); /* Check failed             */
      if(lpI[nC]=='[') nBO=nI;                                                  /*     Return opening bracket index  */
      if(lpI[nC]==']') nBC=nI;                                                  /*     Return closing bracket index  */
      if(!itSeq) continue;                                                      /*     No changes                    */
      if(lpI[nC]=='(') nBrk++;                                                  /*     Increase bracket depth on open*/
      if(!nBrk) lpI[nCd++]=lpI[nC];                                             /*     Copy char. if outside bracket */
      if(lpI[nC]==')') nBrk--;                                                  /*     Decrease brck. depth on close */
    }                                                                           /*   <<                              */
    if(itSeq) lpI[nCd]='\0';                                                    /*   End of string                   */
  }                                                                             /* <<                                */
  if(nBO<0 || nBC<0) FVRT_EXCEPTION(FVRT_SEQSYNTAX,"Symbol [ or ] missing",0,0);/* Check existance of bracket symbols*/
  if(pBO) *pBO=nBO;                                                             /* Export opening bracket index      */
  if(pBC) *pBC=nBC;                                                             /* Export closing bracket index      */
  if(!itSeq) return nRet;                                                       /* No changes -> return              */

  /* Remove empty output symbols */
  lpI=(char*)CData_XAddr(idS,0,0);                                              /* Get symbol pointer                */
  lpTI=CFst_STI_Init(itSeq,0,0);                                                /* Setup iterator without sorting    */
  for(lpT=lpTI->lpFT ; lpT<lpTI->lpFT+lpTI->nRlt*lpTI->nXT ; lpT+=lpTI->nRlt){  /* Loop over all transitions >>      */
    FST_STYPE *nTis=CFst_STI_TTis(lpTI,lpT);                                    /*   Get input symbol pointer        */
    if(*nTis>=0 && *nTis<nXI && !lpI[*nTis*nOI]) *nTis=-1;                      /*   Clear input sym. on empty string*/
  }                                                                             /* <<                                */
  CFst_STI_Done(lpTI);                                                          /* Finalize iterator                 */

  /* Clean-up */                                                                /* --------------------------------- */
L_EXCEPTION:                                                                    /* : Clean exit label                */
  return nRet;                                                                  /* Return                            */
}

/**
 * Parses a sequence FST and recursively builds an FVR tree. This member function is invoked by
 * {@link CFvrtools_SeqToFvr}.
 *
 * @param _this
 *          This instance of CFvrtools.
 * @param itSeq
 *          The (normalized) sequence FST as obtained through {@link CFvrtools_StrToSeq} in unit 0.
 * @param nIni
 *          Zero-based index of the first state in <code>itSeq</code> to parse.
 * @param nPar
 *          Zero-based index of the parent state in <code>itFvr</code> to append parser result to.
 * @param itFvr
 *          The feature-value relation to append the parse result to.
 * @return <code>O_K</code> if successful, a (negative) error code otherwise.
 * @see CFvrtools_StrToSeq
 * @see CFvrtools_SeqToFvr
 */
INT16 CGEN_PROTECTED CFvrtools_ParseSeq
(
  CFvrtools* _this,
  CFst*      itSeq,
  FST_ITYPE  nIni,
  FST_ITYPE  nPar,
  CFst*      itFvr
)
{
  CData*        idFvrTd   = NULL;                                               /* Transition table of itFvr         */
  FST_TID_TYPE* lpSeqTI   = NULL;                                               /* Transition iterator in itSeq      */
  FST_TID_TYPE* lpFvrTI   = NULL;                                               /* Transition iterator in itFvr      */
  BYTE*         lpSeqT    = NULL;                                               /* Transition pointer in itSeq       */
  BYTE*         lpFvrT    = NULL;                                               /* Transition pointer in itFvr       */
  FST_STYPE     nIsBo     = -1;                                                 /* Symbol index of opening brace     */
  FST_STYPE     nIsBc     = -1;                                                 /* Symbol index of closing brace     */
  FST_STYPE     nIsFs     = -1;                                                 /* Symbol index of final state       */
  INT32         nXIS      = NULL;                                               /* Number input symbols              */
  INT32         nBCnt     = 0;                                                  /* Brace counter                     */
  FST_STYPE     nTis      = -1;                                                 /* Input symbol index in itSeq       */
  FST_ITYPE     nTer      = -1;                                                 /* Terminal state in itSeq           */
  FST_ITYPE     nSsq      = -1;                                                 /* 1st state of sub-sequence in itSeq*/
  FST_WTYPE     nW        = NAN;                                                /* Weight in itSeq                   */
  FST_WTYPE     nWL       = 0;                                                  /* Preserved weight of eps. trans.   */
  FST_ITYPE     nChd      = -1;                                                 /* Child state index in itFvr        */
  FST_ITYPE     nFvrT     = -1;                                                 /* Transition index in itFvr         */
  BOOL          bTis      = FALSE;                                              /* Input symbol found                */
  BOOL          bFs       = FALSE;                                              /* Final state found                 */
  FST_ITYPE*    lpnTisPar = NULL;                                               /* Next parent state for input symbs.*/
  INT32         i         = 0;                                                  /* Just a loop counter               */
  INT16         nRet      = O_K;                                                /* Return value                      */

  /* Initialize */                                                              /* --------------------------------- */
  nIsBo = CFvrtools_FindIs("[",FALSE,itSeq);                                    /* Find opening brace symbol         */
  nIsBc = CFvrtools_FindIs("]",FALSE,itSeq);                                    /* Find closing brace symbol         */
  nIsFs = CFvrtools_FindIs("$",FALSE,itSeq);                                    /* Find closing brace symbol         */
  idFvrTd = AS(CData,itFvr->td);                                                /* Get transition table of itFvr     */
  nXIS = CData_GetNRecs(AS(CData,itFvr->is));                                   /* Get number of input symbols       */

  /* Add a state and transition to target */                                    /* --------------------------------- */
  nChd = CFst_Addstates(itFvr,0,1,FALSE);                                       /* Add child state                   */
  nFvrT = CFst_Addtrans(itFvr,0,nPar,nChd);                                     /* Connect from parent state         */
  CData_Dstore(idFvrTd,nW,nFvrT,CData_FindComp(idFvrTd,NC_TD_LSR));             /* Store default weight              */

  /* Iterate sequence FST */                                                    /* --------------------------------- */
  /* NO RETURNS BEYOND HERE! */                                                 /*                                   */
  lpSeqTI = CFst_STI_Init(itSeq,0,0);                                           /* Initialize transition iterator    */
  while ((lpSeqT = CFst_STI_TfromS(lpSeqTI,nIni,lpSeqT))!=NULL)                 /* Iterate transitions in itSeq      */
  {                                                                             /* >>                                */
    nTer = *CFst_STI_TTer(lpSeqTI,lpSeqT);                                      /*   Get trans. terminal state index */
    nTis = *CFst_STI_TTis(lpSeqTI,lpSeqT);                                      /*   Get trans. input symbol index   */
    nW   = *CFst_STI_TW  (lpSeqTI,lpSeqT) + nWL;                                /*   Get transition weight           */
    nWL  = 0;                                                                   /*   Reset preserved weight          */
    /*printf("%ld: '%s'\n",nChd,FVRT_STIS(itFvr,nTis));*/

    if (nTis==nIsBo)                                                            /*   Input symbol is '['             */
    {                                                                           /*   >>                              */
      if (nBCnt==0)                                                             /*     Top level brace               */
        nSsq = nTer;                                                            /*       Start state of sub-sequence */
      nBCnt++;                                                                  /*     Count braces                  */
    }                                                                           /*   <<                              */
    else if (nTis==nIsBc)                                                       /*   Input symbol is ']'             */
    {                                                                           /*   >>                              */
      if (nBCnt==1)                                                             /*     Top level brace               */
      {                                                                         /*     >>                            */
        nRet = CFvrtools_ParseSeq(_this,itSeq,nSsq,nChd,itFvr);                 /*       Recursively parse sub-seq.  */
        if (NOK(nRet))                                                          /*       Failed                      */
          goto L_EXCEPTION;                                                     /*         Give up                   */
      }                                                                         /*     <<                            */
      else if (nBCnt==0)                                                        /*     End of sub-sequence           */
        break;                                                                  /*       Parsing complete            */
      nBCnt--;                                                                  /*     Count braces                  */
    }                                                                           /*   <<                              */
    else if (nTis==nIsFs)                                                       /*   Top-level finite state symbol($)*/
    {                                                                           /*   >>                              */
      bFs=TRUE;                                                                 /*     Finite state found true       */
    }                                                                           /*   <<                              */
    else if (nBCnt==0 && nTis<0)                                                /*   Top-level epsilon symbol        */
    {                                                                           /*   >>                              */
      nWL=nW;                                                                   /*     Preserve weight for next iter.*/
    }                                                                           /*   <<                              */
    else if (nBCnt==0)                                                          /*   Other top-level input symbol    */
    {                                                                           /*   >>                              */
      if (bTis)                                                                 /*     Already have an input symbol  */
        IERROR(_this,FVRT_EXTRATIS,FVRT_STIS(itFvr,nTis),nFvrT,0);              /*       This one is ignored         */
      else                                                                      /*     Don't have  input symbol yet  */
      {                                                                         /*     >>                            */
        CData_Dstore(idFvrTd,nTis,nFvrT,CData_FindComp(idFvrTd,NC_TD_TIS));     /*       Store input symbol          */
        CData_Dstore(idFvrTd,nW  ,nFvrT,CData_FindComp(idFvrTd,NC_TD_LSR));     /*       Store weight                */
        if(bFs)                                                                 /*       Actual state is finite state*/
        {                                                                       /*       >>                          */
          CData_Dstore(AS(CData,itFvr->sd),1,nChd,0);                           /*         Mark finite state         */
          bFs=FALSE;                                                            /*         Reset bool                */
        }                                                                       /*       <<                          */
      }                                                                         /*     <<                            */
      bTis=TRUE;                                                                /*     Remember had input symbol     */
    }                                                                           /*   <<                              */

    nIni = nTer;                                                                /*   Next state in sequence          */
  }                                                                             /* <<                                */
  if (!bTis)                                                                    /* Didn't get an input symbol        */
    IERROR(_this,FVRT_NOTIS,nFvrT,0,0);                                         /*   Warning                         */

  /* Weird wire: cascade equally labeled transitions */                         /* --------------------------------- */
  lpnTisPar = (FST_ITYPE*)dlp_calloc(nXIS,sizeof(FST_ITYPE));                   /* Parent state for inp.symb. list   */
  for (i=0; i<nXIS; i++) lpnTisPar[i]=-1;                                       /* Initialize with -1 (= no parent)  */
  lpFvrTI = CFst_STI_Init(itFvr,0,0);                                           /* Initialize transition iterator    */
  /*printf("starting at state %ld\n",nChd);*/
  while ((lpFvrT=CFst_STI_TfromS(lpFvrTI,nChd,lpFvrT))!=NULL)                   /* Loop over transitions from nChd   */
  {                                                                             /* >>                                */
    if ((nTis = *CFst_STI_TTis(lpFvrTI,lpFvrT))<0) continue;                    /*   Get (non-neg.!) trans.imp.symb. */
    /*printf("- '%s'\n",FVRT_STIS(itFvr,nTis));*/
    if (lpnTisPar[nTis]>=0)                                                     /*   Have new parent state for nTis  */
    {                                                                           /*   >>                              */
      *CFst_STI_TIni(lpFvrTI,lpFvrT)=lpnTisPar[nTis];                           /*     Modify initial state of trans.*/
      CFst_STI_UnitChanged(lpFvrTI,FSTI_CANY);                                  /*     Update trans. iterator (!!)   */
    }                                                                           /*   <<                              */
    lpnTisPar[nTis] = *CFst_STI_TTer(lpFvrTI,lpFvrT);                           /*   Store new parent state for nTis */
  }                                                                             /* <<                                */

  /* Clean-up */                                                                /* --------------------------------- */
L_EXCEPTION:                                                                    /* : Clean exit label                */
  dlp_free(lpnTisPar);
  CFst_STI_Done(lpFvrTI);
  CFst_STI_Done(lpSeqTI);                                                       /* Release iterator                  */
  return nRet;                                                                  /* Return                            */
}

/**
 * Creates a sequence FST from a wFVR string. The format of wFVR strings is described with {@link -from_string} and in
 * <a href="#WW14">[WW14]</a>. The resulting FST is normalized in the sense that every instance of '[' and ']' will
 * become a separate transition and all comments "(...)" will be stripped away. The concatenation of all comments will
 * be stored as the unit name in <code>itSeq</code>.
 *
 * @param _this
 *          This instance of CFvrtools.
 * @param lpsSrc
 *          The source string.
 * @param itSeq
 *          Filled with the resulting sequence FST, must not be <code>NULL</code>.
 * @return <code>O_K</code> if successful, a (negative) error code otherwise.
 * @see CFvrtools_SeqToFvr
 */
INT16 CGEN_PROTECTED CFvrtools_StrToSeq(CFvrtools* _this, const char* lpsSrc, CFst* itSeq)
{
  char*       lpsTok = NULL;                                                    /* Token buffer                      */
  char*       lpsCmt = NULL;                                                    /* Comment buffer                    */
  const char* x      = NULL;                                                    /* Read pointer in source            */
  char*       t      = NULL;                                                    /* Write pointer in token buffer     */
  char*       c      = NULL;                                                    /* Write pointer in comment buffer   */
  INT16       nPct   = 0;                                                       /* Parentheses counter '(' ')'       */
  INT16       nBct   = 0;                                                       /* Brackets counter '[' ']'          */
  INT32       nU     = -1;                                                      /* Unit index in target              */
  INT16       nRet   = O_K;                                                     /* The return value                  */

  /* Initialization */                                                          /* --------------------------------- */
  CHECK_THIS_RV(NOT_EXEC);                                                      /* Check this instance               */
  if (itSeq==NULL) return NOT_EXEC;                                             /* Need target                       */

  /* Create a sequence (linear FST) from the input string */                    /* --------------------------------- */
  lpsTok = (char*)dlp_calloc(dlp_strlen(lpsSrc)+1,sizeof(char)); t=lpsTok;      /* Allocate token buffer             */
  lpsCmt = (char*)dlp_calloc(dlp_strlen(lpsSrc)+1,sizeof(char)); c=lpsCmt;      /* Allocate comment buffer           */
  CFst_Reset(BASEINST(itSeq),TRUE);                                             /* Reset target                      */
  /* equivalent to dLabPro instruction: "" itSeq /lsr /fsa -addunit; -> */
  ISETOPTION(itSeq,"/lsr"); ISETOPTION(itSeq,"/fsa");                           /* Set some options                  */
  nU = CFst_Addunit(itSeq,"");                                                  /* Add a unit to the FVR sequence    */
  IRESETOPTIONS(itSeq);                                                         /* Clear options                     */
  /* <- */

  for (x=lpsSrc; ; x++)                                                         /* Loop over input characters        */
  {                                                                             /* >>                                */
    switch (*x)                                                                 /*   Branch for characters           */
    {                                                                           /*   >>                              */
    case '(':                                                                   /*   Begin of comment                */
      nPct++;                                                                   /*     Count parentheses             */
      break;                                                                    /*     ==                            */
    case ')':                                                                   /*   End of comment                  */
      *c++=' '; *c='\0';                                                        /*     Write space to comment        */
      nPct--;                                                                   /*     Count parentheses             */
      if (nPct<0)                                                               /*     Too many closing              */
        FVRT_EXCEPTION(FVRT_SEQSYNTAX,"too many \")\"",0,0);                    /*       Syntax error (irrecoverable)*/
      break;                                                                    /*     ==                            */
    case '[': /* Fall through */                                                /*   Begin of node                   */
    case ']': /* Fall through */                                                /*   End of node                     */
    case '$': /* Fall through */                                                /*   End of node                     */
    case '\0':                                                                  /*   End of input                    */
      if (dlp_strlen(lpsTok)>0)                                                 /*     Non-empty token               */
      {                                                                         /*     >>                            */
        CFvrtools_AddToSeq(_this,lpsTok,nU,itSeq);                              /*       Add token to sequence       */
        t=lpsTok; *t='\0';                                                      /*       Scrap token buffer          */
      }                                                                         /*     <<                            */
      if (*x=='[') nBct++; else if (*x==']') nBct--;                            /*     Count brackets                */
      if (*x=='[') CFvrtools_AddToSeq(_this,"[",nU,itSeq);                      /*     Add begin node to sequence    */
      if (*x=='$') CFvrtools_AddToSeq(_this,"$",nU,itSeq);                      /*     Add final node to sequence    */
      if (*x==']') CFvrtools_AddToSeq(_this,"]",nU,itSeq);                      /*     Add end node to sequence      */
      if (nBct<0)                                                               /*     Too many closing              */
        FVRT_EXCEPTION(FVRT_SEQSYNTAX,"too many \"]\"",0,0);                    /*       Syntax error (irrecoverable)*/
      break;                                                                    /*     ==                            */
    default:                                                                    /*   All other characters            */
      if (nPct>0) { *c++=*x; *c='\0'; } else { *t++=*x; *t='\0'; }              /*     Gather comment or token       */
      break;                                                                    /*     ==                            */
    }                                                                           /*   <<                              */
    if (*x=='\0') break;                                                        /*   End of input -> end of loop     */
  }                                                                             /* <<                                */
  if (nPct>0)                                                                   /* Unclosed parentheses              */
    FVRT_EXCEPTION(FVRT_SEQSYNTAX,"too many \"(\"",0,0);                        /*   Syntax error (irrecoverable)    */
  if (nBct>0)                                                                   /* Unclosed brackets                 */
    FVRT_EXCEPTION(FVRT_SEQSYNTAX,"too many \"[\"",0,0);                        /*   Syntax error (irrecoverable)    */

  /* Finish target */                                                           /* --------------------------------- */
  if (UD_XS(itSeq,nU)>0)                                                        /* If target has states              */
    SD_FLG(itSeq,UD_FS(itSeq,nU)+UD_XS(itSeq,nU)-1)|=SD_FLG_FINAL;              /*   Make the last one final         */
  ISETFIELD_SVALUE(AS(CData,itSeq->ud),"rtext",dlp_strtrimright(lpsCmt));       /* Store in itSeq.ud.rtext           */

  /* Clean-up */                                                                /* --------------------------------- */
L_EXCEPTION:                                                                    /* : Clean exit label                */
  IF_NOK(nRet)                                                                  /* Something went wrong              */
  {                                                                             /* >>                                */
    CFst_Reset(BASEINST(itSeq),TRUE);                                           /*   Reset target                    */
  }                                                                             /* <<                                */
  dlp_free(lpsTok);                                                             /* Free token buffer                 */
  dlp_free(lpsCmt);                                                             /* Free comment buffer               */
  return nRet;                                                                  /* Return                            */
}

/**
 * Creates an FVR tree from a sequence FST.
 *
 * @param _this
 *          This instance of CFvrtools.
 * @param itSeq
 *          The (normalized) sequence FST as obtained through {@link CFvrtools_StrToSeq}.
 * @param itFvr
 *          Filled with the resulting FVR tree, instance may be identical with <code>itSeq</code>.
 * @return <code>O_K</code> if successful, a (negative) error code otherwise.
 * @see CFvrtools_StrToSeq
 */
INT16 CGEN_PROTECTED CFvrtools_SeqToFvr(CFvrtools* _this, CFst* itSeq, CFst* itFvr)
{
  INT16 nRet = O_K;                                                             /* The return value                  */

  /* Initialization */                                                          /* --------------------------------- */
  CHECK_THIS_RV(NOT_EXEC);                                                      /* Check this instance               */
  if (itSeq==NULL) return NOT_EXEC;                                             /* Need source                       */
  if (itFvr==NULL) return NOT_EXEC;                                             /* Need target                       */
  /* NO RETURNS BEYOND HERE! */                                                 /*                                   */
  CREATEVIRTUAL(CFst,itSeq,itFvr);                                              /* Handle source==target             */

  /* Create FVR tree */                                                         /* --------------------------------- */
  CFst_Reset(BASEINST(itFvr),TRUE);                                             /* Reset target                      */
  CData_CopyDescr(AS(CData,itFvr->ud),AS(CData,itSeq->ud));                     /* Copy unit table descriptors       */
  CData_Copy(itFvr->is,itSeq->is);                                              /* Copy input symbol table           */
  ISETOPTION(itFvr,"/lsr"); ISETOPTION(itFvr,"/fsa");                           /* Set some options                  */
  CFst_Addunit(itFvr,"FVR");                                                    /* Add a unit                        */
  IRESETOPTIONS(itFvr);                                                         /* Clear options                     */
  CFst_Addstates(itFvr,0,1,FALSE);                                              /* Add root state                    */
  IF_NOK(nRet=CFvrtools_ParseSeq(_this,itSeq,0,0,itFvr)) goto L_EXCEPTION;      /* Recursively parse input sequence  */

  /* Clean-up */                                                                /* --------------------------------- */
L_EXCEPTION:                                                                    /* : Clean exit label                */
  IF_NOK(nRet)                                                                  /* Something went wrong              */
  {                                                                             /* >>                                */
    CFst_Reset(BASEINST(itFvr),TRUE);                                           /*   Reset target                    */
  }                                                                             /* <<                                */
  DESTROYVIRTUAL(itSeq,itFvr);                                                  /* Handle source==target             */
  return nRet;                                                                  /* Return                            */
}

/* EOF */
