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
  FST_ITYPE*    lpnTisPar = NULL;                                               /* Next parent state for input symbs.*/
  INT32         i         = 0;                                                  /* Just a loop counter               */
  INT16         nRet      = O_K;                                                /* Return value                      */

  /* Initialize */                                                              /* --------------------------------- */
  nIsBo = CFvrtools_FindIs("[",FALSE,itSeq);                                    /* Find opening brace symbol         */
  nIsBc = CFvrtools_FindIs("]",FALSE,itSeq);                                    /* Find closing brace symbol         */
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
      nPct--;                                                                   /*     Count parentheses             */
      if (nPct<0)                                                               /*     Too many closing              */
        FVRT_EXCEPTION(FVRT_SEQSYNTAX,"too many \")\"",0,0);                    /*       Syntax error (irrecoverable)*/
      break;                                                                    /*     ==                            */
    case '[': /* Fall through */                                                /*   Begin of node                   */
    case ']': /* Fall through */                                                /*   End of node                     */
    case '\0':                                                                  /*   End of input                    */
      if (dlp_strlen(lpsTok)>0)                                                 /*     Non-empty token               */
      {                                                                         /*     >>                            */
        CFvrtools_AddToSeq(_this,lpsTok,nU,itSeq);                              /*       Add token to sequence       */
        t=lpsTok; *t='\0';                                                      /*       Scrap token buffer          */
      }                                                                         /*     <<                            */
      if (*x=='[') nBct++; else if (*x==']') nBct--;                            /*     Count brackets                */
      if (*x=='[') CFvrtools_AddToSeq(_this,"[",nU,itSeq);                      /*     Add begin node to sequence    */
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
