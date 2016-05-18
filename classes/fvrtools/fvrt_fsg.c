
/* Erstellen und ausführen! */

#include "dlp_cscope.h" /* Indicate C scope */
#include "dlp_fvrtools.h"

/*
 * Documentation in fvrtools.def
 */
INT16 CGEN_PUBLIC CFvrtools_FsgNormalize(CFvrtools* _this, CFst* itFsgSrc, CFst* itFsgDst)
{
	return NOT_EXEC;
}

/*
 * Documentation in fvrtools.def
 */
BOOL CGEN_PUBLIC CFvrtools_FsgCheck(CFvrtools* _this, CFst* itFsg)
{
  BOOL          bRet        = FALSE;                                            /* Return value                      */
  FST_ITYPE     nMyIniState = 0;                                                /* State iterator of Fsg             */
  CData*        idVal       = NULL;                                             /* Count braces for state            */
  ICREATEEX(CData,idVal,"CFvrtools_Synthesize~idVal",NULL);                     /*                                   */
  CData_AddComp(idVal,"nBr",T_INT);                                             /* Comp for count of braces          */
  CData_AddComp(idVal,"Vis",T_BOOL);                                            /* Comp for flag state already visit */
  CData_Allocate(idVal,UD_XS(itFsg,0));                                         /* Allocate memory                   */

  /* Simple tests */                                                            /* --------------------------------- */
  if (itFsg==NULL)                                                              /* FST is NULL                       */
    return OK(IERROR(_this,ERR_NULLINST,"itFsg",0,0));                          /*   but must not be                 */
  bRet = CFvrtools_ParseFsgCheck(_this, itFsg, nMyIniState, idVal);
  IDESTROY(idVal);                                                              /*                                   */
  return bRet;                                                                  /* Return result                     */
}

BOOL CGEN_PROTECTED CFvrtools_ParseFsgCheck(CFvrtools* _this, CFst* itFsg, FST_ITYPE nMyIniState, CData* idVal)
{
  BOOL          bRet      = TRUE;                                               /* Return value                      */
  FST_STYPE     nTos      = -1;                                                 /* Input symbol index in itSeq       */
  FST_TID_TYPE* lpTI      = NULL;                                               /* Transition iterator               */
  BYTE*         lpT       = NULL;                                               /* Transition pointer                */
  FST_STYPE     nOsBo     = -1;                                                 /* Symbol index of opening brace     */
  FST_STYPE     nOsBc     = -1;                                                 /* Symbol index of closing brace     */
  FST_ITYPE     nBrace    = (FST_ITYPE)CData_Dfetch(idVal,nMyIniState,0);       /* Counter for braces                */
  FST_ITYPE     nState    = -1;                                                 /* Next state                        */
  CData*        idState   = NULL;                                               /* Data for state Excerpt error      */
  ICREATEEX(CData,idState,"CFvrtools_Synthesize~idState",NULL);                 /*                                   */
  CData_AddComp(idState,"Sta",T_INT);                                           /*                                   */
  CData_Allocate(idState,1);                                                    /* Allocate memory for               */

  nOsBo = CFvrtools_FindOs("[",FALSE,itFsg);                                    /* Find opening brace symbol         */
  nOsBc = CFvrtools_FindOs("]",FALSE,itFsg);                                    /* Find closing brace symbol         */
  lpTI=CFst_STI_Init(itFsg,0,FSTI_SORTINI);                                     /* Initialize transition iterator    */
  while ((lpT=CFst_STI_TfromS(lpTI,nMyIniState,lpT))!=NULL){                    /* Iterate over states               */
    nTos = *CFst_STI_TTos(lpTI,lpT);                                            /* Get output symbol of act. transi. */
    if (nTos == nOsBo) nBrace++;                                                /* Found opening brace               */
    if (nTos == nOsBc) nBrace--;                                                /* Found closing brace               */
    if (nBrace < 0 ){                                                           /* More closing brace than open      */
      printf("\n ERROR: To many closing braces after State: %d", nMyIniState);  /* Error Message                     */
      goto FALSE_RENDER_AND_RETURN;                                             /* Clean exit with false return      */
    }
    nState = *CFst_STI_TTer(lpTI,lpT);                                          /* Get next terminal state           */
    if (CData_Dfetch(idVal,nState,1) == TRUE){                                  /* Check is already visited          */
      if ((FST_ITYPE)CData_Dfetch(idVal,nState,0) != nBrace){                   /*   ...Check is count of brace same */
        printf("\n ERROR: Wrong count of brace on State: %d", nMyIniState);     /* Error Message                     */
        goto FALSE_RENDER_AND_RETURN;                                           /* Clean exit with false return      */
      }
      if (CFst_STI_TfromS(lpTI,nMyIniState,lpT)==NULL){                         /*   ...and no more transitions left */
        goto TRUE_RETURN;                                                       /* Clean exit with true return       */
      }                                                                         /*                                   */
    }                                                                           /*                                   */
    else{                                                                       /* State wasn't visited before       */
      CData_Dstore(idVal,nBrace,nState,0);                                      /* Store count of braces             */
      CData_Dstore(idVal,TRUE,nState,1);                                        /* Set flag that state is visited    */
    }                                                                           /*                                   */

    if ((CFst_STI_TfromS(lpTI,nMyIniState,lpT))!=NULL){                         /* Check is more than one transition */
      nBrace = (FST_ITYPE)CData_Dfetch(idVal,nMyIniState,0);                    /* Reset count for next transition   */
      if (nMyIniState != nState){                                               /* Check selfloop                    */
        bRet = CFvrtools_ParseFsgCheck(_this, itFsg, nState, idVal);            /* Is no selfloop start recursion    */
      }
      if (!bRet){                                                               /* Check wrong return                */
        printf("\n |-> ERROR: Backtracking closing state %d", nMyIniState);     /* Error Message                     */
        goto FALSE_RETURN;                                                      /* Clean exit with false return      */
      }                                                                         /*                                   */
    }                                                                           /*                                   */
    else{                                                                       /* otherwise...                      */
      nMyIniState = nState;                                                     /* ... set next state to actually    */
      lpT = NULL;
    }                                                                           /*                                   */
  }                                                                             /*                                   */

TRUE_RETURN:                                                                    /* : Clean exit label                */
  CFst_STI_Done(lpTI);                                                          /*                                   */
  IDESTROY(idState);                                                            /*                                   */
  return TRUE;                                                                  /*                                   */
FALSE_RENDER_AND_RETURN:                                                        /* : Clean exit label                */
  CData_Dstore(idState, nMyIniState,0,0);                                       /* Store state in Data to use excerpt*/
  ISETOPTION(itFsg,"/backward")                                                 /* Set option for CFst_Excerpt...    */
  CFst_Excerpt(itFsg, itFsg, 0, idState, 0, NULL, -1);                          /* Reduce FST to state with error    */
  IRESETOPTIONS(itFsg);                                                         /* ...Reset option                   */
FALSE_RETURN:                                                                   /*                                   */
  CFst_STI_Done(lpTI);                                                          /* : Clean exit label                */
  IDESTROY(idState);                                                            /*                                   */
  return FALSE;                                                                 /*                                   */
}
