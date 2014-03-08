// dLabPro class CFunction (function)
// - Token interpretation methods
//
// AUTHOR : Matthias Wolff
// PACKAGE: dLabPro/classes
// 
// Copyright 2013 dLabPro contributors and others (see COPYRIGHT file) 
// - Chair of System Theory and Speech Technology, TU Dresden
// - Chair of Communications Engineering, BTU Cottbus
// 
// This file is part of dLabPro.
// 
// dLabPro is free software: you can redistribute it and/or modify it under the
// terms of the GNU Lesser General Public License as published by the Free
// Software Foundation, either version 3 of the License, or (at your option)
// any later version.
// 
// dLabPro is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
// details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with dLabPro. If not, see <http://www.gnu.org/licenses/>.

#include "dlp_function.h"
#include "dlp_var.h"
#include "dlp_math.h"

/**
 * Interprets a formula token.
 *
 * @param lpsToken
 *          The token to be interpreted
 */
INT16 CGEN_PROTECTED CFunction::ItpAsFormula(const char* lpsToken)
{
  if (!m_lpsLastFml) m_lpsLastFml = (char*)dlp_calloc(L_INPUTLINE, 1);          // Allocate last formula buffer
  INT32 nLine = (INT32)CData_Dfetch(m_idTsq,m_nPp,OF_LINE);                     // Get current line in script (or 0)
  FNC_MSG(2,"  - Formula \"%s\"",lpsToken,0,0,0,0);                             // Protocol
  IF_NOK(Formula2RPN(lpsToken,m_lpsLastFml,L_INPUTLINE-1)) return NOT_EXEC;     // Translate to UPN
  DLPASSERT(m_nTeqOffset==0);                                                   // Just the current formula!
  TokenProcessed();                                                             // Remove formula token
  PostCommand(m_lpsLastFml,NULL,nLine,TRUE);                                    // Post UPN code to head of exec. queue
  m_nTeqOffset = -1;                                                            // No futher tokens to be deleted
  return O_K;                                                                   // That's it
}

/**
 * Interprets a token as a directive. Directives have the highest execution
 * priority.
 *
 * @param lpsToken
 *          The token to be interpreted
 * @return <code>O_K</code> if the token was successfully interpreted as
 *         directive, a (negative) error code otherwise.
 */
INT16 CGEN_PRIVATE CFunction::ItpAsDirective(const char* lpsToken)
{
  if (dlp_strcmp(lpsToken,"?error")==0)                                         // ?error directive
  {                                                                             // >>
    BOOL bErr = (*(INT16*)GetStaticFieldPtr("last_error"))<0;                   //   Get error state
    FNC_MSG(2,"  - ?error: %s",bErr?"TRUE":"FALSE",0,0,0,0);                    //   Protocol
    PushLogic(bErr);                                                            //   Push error
    return O_K;                                                                 //   Yes, it was a directive
  }                                                                             // <<
  SetLastError(0,NULL);                                                         // Clear the error flag
  return NOT_EXEC;                                                              // Sorry, that was not a directive
}

/**
 * Tries interpreting a token as a word identifier.
 *
 * @param lpsToken
 *          Pointer to token string
 * @return <code>O_K</code> if the word was found an successfully executed, a
 *         dLabPro error code if the word was found and there were errors when
 *         executing it or <code>FNC_UNKNOWN</code> if no such word was found.
 */
INT16 CGEN_PRIVATE CFunction::ItpAsWord(const char* lpsToken)
{
  // Local variables                                                            // ------------------------------------
  SWord*      lpWord       = NULL;                                              // Word
  CDlpObject* iCont        = NULL;                                              // Container of word
  INT16       nErr         = O_K;                                               // Error code
  INT16       nElvl        = -1;                                                // Error level
  char        lpsFqid[255];                                                     // Fully qualified instance name

  // Validation                                                                 // ------------------------------------
  if (!dlp_strlen(lpsToken)) return NOT_EXEC;                                   // Neither good nor bad

  // Find word by identifier                                                    // ------------------------------------
  if (!(lpWord = FindWordAi(lpsToken))) return FNC_UNDEF;                       // Find word
  iCont = (CDlpObject*)lpWord->lpContainer;                                     // Get word container
  IFCHECKEX(2) CDlpObject_GetFQName(iCont,lpsFqid,FALSE);                       // Get fully qualified container id

  // Action depends on word type                                                // ------------------------------------
  switch (lpWord->nWordType)                                                    // Branch by word type
  {                                                                             //
  case WL_TYPE_METHOD:                                                          // - Method:
    FNC_MSG(2,"  - Method %s %s",lpsFqid,lpWord->lpName,0,0,0);                 //   Protocol
    FNC_MSG(2,"  - Invoke PMCF 0x%08X ...",&lpWord->ex.mth.lpfCallback,0,0,0,0);//   Protocol
    nElvl = m_bNoerror ? CDlpObject_SetErrorLevel(0) : -1;                      //   Switch errors off
    if (lpWord->ex.mth.lpfCallback)                                             //   Have invocation callback function?
    {                                                                           //   >>
      if (iCont==m_iAi && !m_bAiUsed)                                           //     Method of active primary inst.?
      {                                                                         //     >>
        if (StackInstance(0)==m_iAi)                                            //       Active instance on stack top?
          Pop();                                                                //         This is expected -> pop it
        else                                                                    //       Active inst. not on stack top?
        {                                                                       //       >>
          IERROR(this,FNC_SYNTAX,0,0,0);                                        //         Error
          nErr  = FNC_SYNTAX;                                                   //         Cannot continue
          m_iAi = NULL;                                                         //         Clear active instance
        }                                                                       //       <<
      }                                                                         //     <<
      IF_OK(nErr)                                                               //     Everything ok?
      {                                                                         //     >>
        DLPASSERT(CDlpObject_MicSet(iCont,&m_mic)==NULL);                       //       Set MIC, assert clean!
        BOOL bFnc = m_iAi && CDlpObject_IsKindOf(m_iAi,"function");             //       Active instance is a function
        nErr =(iCont->*(lpWord->ex.mth.lpfCallback))();                         //       Invoke method on container
        if (!bFnc || dlp_strcmp(lpWord->lpName,"-destroy")!=0)                  //       NOT function -destroy
        {                                                                       //       >>
        	iCont->ResetAllOptions();                                             //         Reset options of container
        	CDlpObject_MicSet(iCont,NULL);                                        //         Clear invocation context
        	if (iCont==m_iAi) m_bAiUsed = TRUE;                                   //         Active instance was used
        	if (iCont->m_nClStyle & CS_SECONDARY) m_iAi2 = NULL;                  //         Deactivate secondary instances
        }                                                                       //       <<
        IFCHECKEX(2) printf(" ok (%d)",nErr);                                   //       Protocol
      }                                                                         //     <<
      else                                                                      //     Were there errors?
        IFCHECKEX(2) printf("failed");                                          //       Protocol
    }                                                                           //   <<
    else                                                                        //   No callback function
    {                                                                           //   >>
      IFCHECKEX(2) printf("failed");                                            //     Protocol
      nErr = NOT_EXEC;                                                          //     This still means "word processed"!
    }                                                                           //   <<
    if (nElvl>=0) CDlpObject_SetErrorLevel(nElvl);                              //   /noerror: switch errors back on
    m_bNoerror = FALSE;                                                         //   Reset /noerror option
    return nErr;                                                                //   Return error code

  case WL_TYPE_OPTION :                                                         // - Option:
    FNC_MSG(2,"  - Set option %s %s",lpsFqid,lpWord->lpName,0,0,0);             //   Protocol
    FNC_MSG(2,"  - Invoke OUCF 0x%08X",&lpWord->ex.opt.lpfCallback,0,0,0,0);    //   Protocol
    *(BOOL*)lpWord->lpData = TRUE;                                              //   Set option
    if (lpWord->ex.opt.lpfCallback)                                             //   Have invocation callback function?
    {                                                                           //   >>
// MWX 2006-06-31: Change MIC tracking -->                                      //     ./.
//    lpMicSave = CDlpObject_MicSet(iCont,&m_mic);                              //     ./.
      DLPASSERT(CDlpObject_MicSet(iCont,&m_mic)==NULL);                         //     Set MIC, assert clean!
// <--                                                                          //     ./.
      nErr = (iCont->*(lpWord->ex.opt.lpfCallback))();                          //     Invoke option changed callb. f.
      IF_NOK(nErr) *(BOOL*)lpWord->lpData = FALSE;                              //     Set refused
// MWX 2006-06-31: Change MIC tracking -->                                      //     ./.
//    CDlpObject_MicSet(iCont,lpMicSave);                                       //     ./.
      CDlpObject_MicSet(iCont,NULL);                                            //     Clear invocation context
// <--                                                                          //     ./.
      IFCHECKEX(2) printf(" ok (%d)",nErr);                                     //     Protocol
      return nErr;                                                              //     Done
    }                                                                           //   <<
    IFCHECKEX(2) printf(" ok (no callback function)");                          //   Protocol
    return O_K;                                                                 //   Done

  case WL_TYPE_FIELD:                                                           // - Field:
    FNC_MSG(2,"  - Fetch field %s.%s",lpsFqid,lpWord->lpName,0,0,0);            //   Protocol
    if (lpWord->lpData) switch (lpWord->ex.fld.nType)                           //   Branch by field type
    {                                                                           //
    case T_BOOL    : { PushNumber(CMPLX(*(     BOOL*)lpWord->lpData)); break; } //   - Fetch boolean
    case T_UCHAR   : { PushNumber(CMPLX(*(    UINT8*)lpWord->lpData)); break; } //   - Fetch unsigned char
    case T_CHAR    : { PushNumber(CMPLX(*(     INT8*)lpWord->lpData)); break; } //   - Fetch signed char
    case T_USHORT  : { PushNumber(CMPLX(*(   UINT16*)lpWord->lpData)); break; } //   - Fetch unsigned short
    case T_SHORT   : { PushNumber(CMPLX(*(    INT16*)lpWord->lpData)); break; } //   - Fetch signed short
    case T_UINT    : { PushNumber(CMPLX(*(   UINT32*)lpWord->lpData)); break; } //   - Fetch unsigned int
    case T_INT     : { PushNumber(CMPLX(*(    INT32*)lpWord->lpData)); break; } //   - Fetch signed int
    case T_ULONG   : { PushNumber(CMPLX(*(   UINT64*)lpWord->lpData)); break; } //   - Fetch unsigned long
    case T_LONG    : { PushNumber(CMPLX(*(    INT64*)lpWord->lpData)); break; } //   - Fetch signed long
    case T_FLOAT   : { PushNumber(CMPLX(*(  FLOAT32*)lpWord->lpData)); break; } //   - Fetch float
    case T_DOUBLE  : { PushNumber(CMPLX(*(  FLOAT64*)lpWord->lpData)); break; } //   - Fetch double
    case T_COMPLEX : { PushNumber(*(COMPLEX64*)lpWord->lpData); break;        } //   - Fetch complex double
    case T_STRING  : /* Fall through */                                         //   |
    case T_CSTRING : /* Fall through */                                         //   |
    case T_TEXT    : { PushString(*(char**)lpWord->lpData);  break;           } //   - Fetch char*
    case T_INSTANCE: {                                                          //   - Fetch instance
      CDlpObject* lpInst = (CDlpObject*)*(void**)lpWord->lpData;                //       Get pointer to instance
      ItpInstance(lpInst);                                                      //       Interprete instance
      break; }                                                                  //       Done
    default        : {                                                          //   - All other types
      if (lpWord->ex.fld.nType>0 && lpWord->ex.fld.nType<=256)                  //     Is character array?
        PushString((char*)lpWord->lpData);                                      //       Fetch and push to stack
      else                                                                      //     No charcter array
        IERROR(this,FNC_CANTEXEC,"field",iCont->m_lpClassName,lpWord->lpName); }//       Error
    }                                                                           //     <<
    return O_K;                                                                 //   Done (Field)
  case WL_TYPE_INSTANCE:                                                        // - Instance:
    if (!lpWord->lpData)                                                        //   Instance associated with word?
    {                                                                           //   >>
      IERROR(this,FNC_CANTEXEC,"instance",lpWord->lpName,"");                   //     NO --> Error!
      return O_K;
    }                                                                           //   <<
    FNC_MSG(2,"  - Instance %s",CDlpObject_GetFQName(                           //   Protocol
      (CDlpObject*)lpWord->lpData,dlp_get_a_buffer(),FALSE),0,0,0,0);           //   |
    ItpInstance((CDlpObject*)lpWord->lpData);                                   //   Interprete instance
    return O_K;                                                                 //   Done

  case WL_TYPE_FACTORY:
    FNC_MSG(2,"  - Class identifier %s",lpsToken,0,0,0,0);
    const char* lpsName = GetNextToken();
    if (!dlp_strlen(lpsName))
    {
      IERROR(this,FNC_EXPECT,"identifier",0,0);
      return NOT_EXEC;
    }
    CDlpObject* iInst = Instantiate(lpWord->lpName,lpsName);
    if (!iInst) return NOT_EXEC;
    CVar* iVar = (CVar*)CDlpObject_OfKind("var",iInst);
    if (!iVar || m_nStackLen==0) return O_K;
    FNC_MSG(2,"  - Setting up variable",0,0,0,0,0);
    switch (m_aStack[0].nType)
    {
      case T_BOOL    : iVar->Bset(PopLogic   ()); return O_K;
      case T_COMPLEX : iVar->Vset(PopNumber  ()); return O_K;
      case T_STRING  : iVar->Sset(PopString  ()); return O_K;
      case T_INSTANCE: iVar->Iset(PopInstance()); return O_K;
    }
    return O_K;
  }

  return NOT_EXEC;
}

/**
 * Interprets a token as a number.
 *
 * @param lpsToken
 *          Pointer to the token to be interpreted
 */
INT16 CGEN_PROTECTED CFunction::ItpAsNumber(const char* lpsToken)
{
  COMPLEX64 n            = CMPLX(0.);                                           // Number parse buffer
  char   sBuf[L_SSTR];
  if (!dlp_strlen(lpsToken)                ) return NOT_EXEC;                   // Empty token cannot be a number
  if (NOK(dlp_sscanx(lpsToken,T_COMPLEX,&n))) return NOT_EXEC;                  // Scan token, failure -> not a number
  dlp_sprintx(sBuf,(char*)&n,T_COMPLEX,TRUE);                                   // HACK: 64-bit compatible print
  FNC_MSG(2,"  - Constant number %s",dlp_strtrimleft(sBuf),0,0,0,0);            // Protocol
  PushNumber(n);                                                                // Push the number
  return O_K;                                                                   // Yes, it was a number
}

/**
 * Interprets a token (sequence) as a constant list. A list starts with the
 * token "{". If a token sequence has been interpreted as a list, a temporary
 * data instance will be pushed to the stack.
 *
 * @param lpsToken
 *          Pointer to the token to be interpreted
 */
INT16 CGEN_PROTECTED CFunction::ItpAsList(const char* lpsToken)
{
  CData*    idTmp             = NULL;
  char      sVal[L_INPUTLINE];
  INT32     nR                = 0;
  INT32     nC                = 0;
  INT32     nBlv              = 0;

  if (dlp_strcmp(lpsToken,"{")!=0) return NOT_EXEC;                             // Constant lists start with "{"
  if                                                                            // Check if it is "data { ..."
  (                                                                             // |
    m_nStackLen>0                               &&                              // | Stack must not be empty
    m_aStack[0].nType==T_INSTANCE               &&                              // | Stack top must be an instance ...
    (                                                                           // |
      /*m_aStack[0].val.i->IsKindOf("data"    ) ||*/                                // | ... of class data ...
      m_aStack[0].val.i->IsKindOf("function")                                   // | ... or of class function ...
    )                                           &&                              // |
    m_aStack[0].val.i == m_iAi                  &&                              // | Stack top must be active ...
    !m_bAiUsed                                                                  // | ... and virginal
  )                                                                             // |
  {                                                                             // >>
    FNC_MSG(2,"  - Constant list, delegated to class %s",                       //   Protocol
      m_aStack[0].val.i->m_lpClassName,0,0,0,0);                                //   |
    return NOT_EXEC;                                                            //   Have it done by function
  }                                                                             // <<

  // Read constant list                                                         // ------------------------------------
  FNC_MSG(2,"  - Constant list",0,0,0,0,0);                                     // Protocol
  ICREATEEX(CData,idTmp,"~tmpList",NULL);                                       // Create list data instance
  for (nBlv=0,nR=0,nC=0;;)                                                      // Do ad infinitum
  {                                                                             // >>
    lpsToken = GetNextToken();                                                  //   Get next token
    if (!lpsToken) { IERROR(_this,FNC_UNEXOEF,"}",0,0); return O_K; }           //   No more tokens -> error!
    if (!dlp_strlen(lpsToken)) continue;                                        //   Skip empty tokens

    // Handle curly braces                                                      //   - - - - - - - - - - - - - - - - -
    if (dlp_strcmp(lpsToken,"}")==0)                                            //   Closing curly brace
    {                                                                           //   >>
      if (nBlv==1)                                                              //    Close record
      {                                                                         //    >>
        if (nC>CData_GetNComps(idTmp)) IERROR(this,FNC_LISTINI,"many",nR,0);    //       Too many initializers
        if (nC<CData_GetNComps(idTmp)) IERROR(this,FNC_LISTINI,"few" ,nR,0);    //       Too few initializers
        nR++;                                                                   //       Next record
        nC=-1;                                                                  //       Off components
      }                                                                         //     <<
      nBlv--;                                                                   //     Decrement brace level
      if (nBlv<0) break;                                                        //     End of list -> stop
      continue;                                                                 //     Continue with next token
    }                                                                           //   <<
    else if (dlp_strcmp(lpsToken,"{")==0)                                       //   Opening curly brace
    {                                                                           //   >>
      if (nC==-1) nC=0;                                                         //     In components
      nBlv++;                                                                   //     Increment brace level
      if (nBlv >1) IERROR(this,FNC_IGNORE," extra {",0,0);                      //     Too many opening braces
      continue;                                                                 //     Continue with next token
    }                                                                           //   <<
    if (nC<0) IERROR(this,FNC_IGNORE," extra initializers between records",0,0);//   Must not be off components

    // Push value of current initializer on the stack                           //   - - - - - - - - - - - - - - - - -
    dlp_strcpy(sVal,lpsToken);                                                  //   Copy token
    if(sVal[0]=='\"' || sVal[0]=='\'')                                          //   If the token is doublequoted >>
      PushString(dlp_strconvert(SC_UNESCAPE,sVal,                               //     Push the unquoted string
            dlp_strunquotate(sVal,sVal[0],sVal[0])));                           //     on the stack
    else if (ItpAsWord(lpsToken)==FNC_UNDEF)                                    //   << Try to parse as word
      if (ItpAsNumber(lpsToken)!=O_K)                                           //     Try to parse as number
        if (ItpAsOperator(lpsToken)!=O_K){                                      //     Try to parse as operator
          IERROR(this,FNC_UNDEF,lpsToken,0,0);                                  //     Error message
          PushNumber(CMPLX(0));                                                 //     Default is the number zero
        }                                                                       //   >>

    // Build data structure                                                     //   - - - - - - - - - - - - - - - - -
    INT16 nType=T_DOUBLE;                                                       //   Type of the current component
    if (nR==0) {                                                                //   If in first record add component >>
      switch(m_aStack[0].nType){                                                //     Check type of stack top element >>
      case T_BOOL:     nType=T_BOOL;   break;                                   //       Boolean type
      case T_COMPLEX:  nType=T_DOUBLE; break;                                   //       Any number type
      case T_STRING:   nType=255;      break;                                   //       String type
      case T_INSTANCE: IERROR(this,FNC_INVALID,lpsToken,0,0); break;            //       An instance cannot be used
      }                                                                         //     <<
      CData_AddComp(idTmp,"",nType);                                            //   Add a new component
    }else nType=CData_GetCompType(idTmp,nC);                                    //   Read component type
    if (CData_GetNRecs(idTmp)<=nR) CData_Reallocate(idTmp,nR+1);                //   Allocate next record

    // Fill in data                                                             //   - - - - - - - - - - - - - - - - -
    switch(nType){                                                              //   Switch component type
    case T_BOOL: CData_Dstore(idTmp,PopLogic(),nR,nC);  break;                  //     Boolean: read logic + store number
    case 255:    CData_Sstore(idTmp,PopString(),nR,nC); break;                  //     String: read + store string
    default:                                                                    //     Any numberic type: >>
      COMPLEX64 val=PopNumber();                                                //       Read number from stack
      if(val.y && nType!=T_COMPLEX){                                            //       Complex number and double type >>
        CData_Mark(idTmp,nC,1);                                                 //         Convert the component to complex
        idTmp->m_bMark=TRUE;                                                    //         |
        CData_Tconvert(idTmp,idTmp,T_COMPLEX);                                  //         |
        CData_Unmark(idTmp);                                                    //         |
      }                                                                         //       <<
      CData_Cstore(idTmp,val,nR,nC);                                            //       Store the number
    break;                                                                      //     <<
    }                                                                           //   <<
    nC++;                                                                       //   Next component
  }                                                                             // <<

  // Optimize string components                                                 // ------------------------------------


  // Aftermath                                                                  // ------------------------------------
  PushInstance(idTmp);                                                          // Push temporary instance
  return O_K;                                                                   // NO further processing!
}

/**
 * Interprets the assignment (generic instance copy) operator "=".
 *
 * @param lpsToken
 *          Pointer to the token to be interpreted
 */
INT16 CGEN_PROTECTED CFunction::ItpAsOpAssign(const char* lpsToken)
{
  if                                                                            // It's instance copying if
  (                                                                             // |
    dlp_strcmp(lpsToken,"=")==0                                    &&           // | - the operation name is "=" AND
    m_nStackLen>=2                                                 &&           // | - there are two args. on the stack
    m_aStack[0].nType==T_INSTANCE && m_aStack[1].nType==T_INSTANCE              // | - ... which are both instances
  )                                                                             // |
  {                                                                             // >>
    if (CDlpObject_OfKind("var",m_aStack[0].val.i)) return NOT_EXEC;            //   Variables have their own "="
    CDlpObject* iArg2 = PopInstance(2);                                         //   Get argument 2
    CDlpObject* iArg1 = PopInstance(1);                                         //   Get argument 1
    FNC_MSG(2,"  - Assignment operator (%s=%s)",                                //   Protocol
      iArg2?iArg2->m_lpInstanceName:"NULL",                                     //   |
      iArg1?iArg1->m_lpInstanceName:"NULL",0,0,0);                              //   |
    if (iArg2)                                                                  //   Destination instance not NULL
      iArg2->Copy(iArg1);                                                       //     Copy arg. 1 to arg. 2
    else                                                                        //   Destination instance NULL
      IERROR(this,FNC_ASGNONNULL,0,0,0);                                        //     Warning
    return O_K;                                                                 //   Done.
  }                                                                             // <<
  return NOT_EXEC;                                                              // N��
}

/**
 * Executes the equality operators == and !=.
 *
 * @param nOpc
 *          Scalar operation code
 * @param nOps
 *          Number of operands
 * @return <code>O_K</code> if the operation has been executed, a (negative)
 *         error code otherwise
 */
INT16 CGEN_PROTECTED CFunction::OpEqual(SWord* lpWord)
{
  INT16 nOpc = (lpWord == NULL) ? -1 : lpWord->ex.op.nOpc;
  INT16 nOps = (lpWord == NULL) ? -1 : lpWord->ex.op.nOps;
  if (nOpc!=OP_EQUAL && nOpc!=OP_NEQUAL) return NOT_EXEC;                       // Not an equality operation
  DLPASSERT(nOps==2);                                                           // Cannot happen
  BOOL bRes;                                                                    // Result value
  if(dlp_is_numeric_type_code(StackGet(0)->nType) &&                            // Both arguments numeric ? >>
      dlp_is_numeric_type_code(StackGet(1)->nType)){                            // |
    COMPLEX64 nArg2 = PopNumber(2);                                             //   Get argument 1 as number
    COMPLEX64 nArg1 = PopNumber(1);                                             //   Get argument 2 as numer
    bRes = CMPLX_EQUAL(nArg1,nArg2);                                            //   Test equal
  }else{                                                                        // << non numeric arguments >>
    char* lpsArg2 = PopString(2);                                               //   Get argument 2 as string
    char* lpsArg1 = PopString(1);                                               //   Get argument 1 as string
    bRes=dlp_strcmp(lpsArg1,lpsArg2)==0;                                        //   Test equal
  }                                                                             // <<
  if(nOpc!=OP_EQUAL) bRes=!bRes;                                                // Negate for not equality
  PushNumber(CMPLX(bRes));                                                      // Give back result
  return O_K;                                                                   // Done
}

/**
 * Executes logical operators &&, ||, etc.
 *
 * @param nOpc
 *          Scalar operation code
 * @param nOps
 *          Number of operands
 * @return <code>O_K</code> if the operation has been executed, a (negative)
 *         error code otherwise
 */
INT16 CGEN_PROTECTED CFunction::OpLogic(SWord* lpWord)
{
  INT16 nOpc = (lpWord == NULL) ? -1 : lpWord->ex.op.nOpc;
  INT16 nOps = (lpWord == NULL) ? -1 : lpWord->ex.op.nOps;
  COMPLEX64 nArg1 = CMPLX(0.);                                                  // Number argument #1
  COMPLEX64 nArg2 = CMPLX(0.);                                                  // Number argument #2
  BOOL      bArg1 = FALSE;                                                      // Logik argument #1
  BOOL      bArg2 = FALSE;                                                      // Logik argument #2

  if (!dlp_is_logic_op_code(nOpc)) return NOT_EXEC;                             // This is no logic operation
  DLPASSERT(nOps>=1 && nOps<=2);                                                // Cannot happen

  if (nOpc==OP_LESS || nOpc==OP_GREATER || nOpc==OP_LEQ || nOpc==OP_GEQ)        // Comparing values (numeric args)
  {                                                                             // >>
    nArg2 = PopNumber(2);                                                       //   Get mandatory second argument
    nArg1 = PopNumber(1);                                                       //   Get mandatory first argument
    PushLogic((BOOL)dlp_scalopC(nArg1,nArg2,nOpc).x);                           // Calculate result and push to stack
  }                                                                             // <<
  else                                                                          // Other logic ops. (boolean args)
  {                                                                             // >>
    if (nOps>1) bArg2 = PopLogic(2);                                            //   Get optional second argument
    bArg1 = PopLogic(1);                                                        //   Get mandatory first argument
    PushLogic((BOOL)dlp_scalop(bArg1,bArg2,nOpc));                              // Calculate result and push to stack
  }                                                                             // <<
  return O_K;                                                                   // Done
}

/**
 * Executes scalar string operators + and *.
 *
 * @param nOpc
 *          Scalar operation code
 * @param nOps
 *          Number of operands
 * @return <code>O_K</code> if the operation has been executed, a (negative)
 *         error code otherwise
 */
INT16 CGEN_PROTECTED CFunction::OpStrsc(SWord* lpWord)
{
  INT16 nOpc = (lpWord == NULL) ? -1 : lpWord->ex.op.nOpc;
  if (nOpc!=OP_ADD && nOpc!=OP_MULT) return NOT_EXEC;                           // Not a string + or *
  if (m_aStack[0].nType!=T_STRING&&m_aStack[1].nType!=T_STRING) return NOT_EXEC;// Both args are no strings
  DLPASSERT(m_nStackLen>=2)                                                     // To be checked before!

  char* lpsArg2 = PopString(2);                                                 // Get argument 2 as string
  char* lpsArg1 = PopString(1);                                                 // Get argument 1 as string
  char* lpsBuf  =                                                               // Allocate a buffer
    (char*)dlp_malloc((dlp_strlen(lpsArg1)+dlp_strlen(lpsArg2)+1)*sizeof(char));  // |
  dlp_strcpy(lpsBuf,lpsArg1);                                                   // Copy argument 1 into buffer
  dlp_strcat(lpsBuf,lpsArg2);                                                   // Append argument 2 to buffer
  PushString(lpsBuf);                                                           // Push result
  dlp_free(lpsBuf);                                                             // Free the buffer
  return O_K;                                                                   // Done
}

/**
 * Executes matrix operators.
 *
 * @param lpWord
 *          Operator word
 * @return <code>O_K</code> if the operation has been executed, a (negative)
 *         error code otherwise
 */
INT16 CGEN_PROTECTED CFunction::OpMatrx(SWord* lpWord)
{
  INT16 nOpc = (lpWord == NULL) ? -1 : lpWord->ex.op.nOpc;
  INT16 nOps = (lpWord == NULL) ? -1 : lpWord->ex.op.nOps;
  CData*    idArg1 = NULL;                                                      // Data argument 1
  CData*    idArg2 = NULL;                                                      // Data argument 2
  COMPLEX64 nArg1  = CMPLX(0.);                                                 // Scalar argument 1
  COMPLEX64 nArg2  = CMPLX(0.);                                                 // Scalar argument 2
  CData*    idTmp  = NULL;                                                      // Temporary data instance
  BOOL      bDmop  = (nOpc>=OP_MATROP_MIN && nOpc<=OP_MATROP_MAX);              // Dedicated matrix operation

  if (nOps==0 && bDmop)                                                         // Constant dedicated matrix operation
  {                                                                             // >>
    FNC_MSG(2,"  - Constant matrix operation %ld",(long)nOpc,0,0,0,0);          //   Protocol (verbose level 2)
    ICREATEEX(CData,idTmp,"#TMP#m_op",NULL);                                    //   Create temp. result instance
    ((LP_MOP_FUNC)lpWord->ex.op.lpfCallback)                                    //   Execute operation
      (idTmp,NULL,T_IGNORE,NULL,T_IGNORE,nOpc);                                 //   |
    PushInstance(idTmp);                                                        //   Push result
    return O_K;                                                                 //   Operation executed
  }                                                                             // <<
  else if (nOps==1)                                                             // Unary operator
  {                                                                             // >>
    DLPASSERT(m_nStackLen>=1);                                                  //   Must have been checked before
    if (m_aStack[0].nType==T_INSTANCE)                                          //   Argument 1 is an instance
    {                                                                           //   >>
      if (!CDlpObject_OfKind("data",m_aStack[0].val.i)) return FNC_TYPECAST;    //     Cannot type cast
      idArg1 = (CData*)PopInstance(1);                                          //     Pop data argument
      FNC_MSG(2,"  - Unary matrix operation %ld %s",(long)nOpc,                 //     Protocol (verbose level 2)
        idArg1?idArg1->m_lpInstanceName:"NULL",0,0,0);                          //     |
      ICREATEEX(CData,idTmp,"#TMP#m_D_op",NULL);                                //     Create temp. result instance
      ((LP_MOP_FUNC)lpWord->ex.op.lpfCallback)                                  //     Execute operation
        (idTmp,idArg1,T_INSTANCE,NULL,T_IGNORE,nOpc);                           //     |
      PushInstance(idTmp);                                                      //     Push result
      return O_K;                                                               //     Operation executed
    }                                                                           //   <<
    else if (bDmop)                                                             //   Dedicated matrix op (arg1 scalar)
    {                                                                           //   >>
      nArg1 = PopNumber(1);                                                     //     Pop scalar argument
      FNC_MSG(2,"  - Unary dedicated matrix operation %ld %g+%gi",              //     Protocol (verbose level 2)
        (long)nOpc,(double)nArg1.x,(double)nArg2.y,0,0);                        //     |
      ICREATEEX(CData,idTmp,"#TMP#m_N_op",NULL);                                //     Create temp. result instance
      ((LP_MOP_FUNC)lpWord->ex.op.lpfCallback)                                  //     Execute operation
        (idTmp,&nArg1,(nArg1.y==0)?T_DOUBLE:T_COMPLEX,NULL,T_IGNORE,nOpc);      //     |
      PushInstance(idTmp);                                                      //     Push result
      return O_K;                                                               //     Operation executed
    }                                                                           //   <<
  }                                                                             // <<
  else if (nOps==2)                                                             // Binary operator
  {                                                                             // >>
    DLPASSERT(m_nStackLen>=2);                                                  //   Must have been checked before
    if (m_aStack[0].nType==T_INSTANCE && m_aStack[1].nType==T_INSTANCE)         //   Both arguments are instances
    {                                                                           //   >>
      if (!CDlpObject_OfKind("data",m_aStack[0].val.i)) return FNC_TYPECAST;    //     Cannot type cast
      if (!CDlpObject_OfKind("data",m_aStack[1].val.i)) return FNC_TYPECAST;    //     Cannot type cast
      idArg2 = (CData*)PopInstance(2);                                          //     Pop data argument 2
      idArg1 = (CData*)PopInstance(1);                                          //     Pop data argument 1
      FNC_MSG(2,"  - Binary matrix operation %ld %s, %s",(long)nOpc,            //     Protocol (verbose level 2)
        idArg1?idArg1->m_lpInstanceName:"NULL",                                 //     |
        idArg2?idArg2->m_lpInstanceName:"NULL",0,0);                            //     |
      ICREATEEX(CData,idTmp,"#TMP#m_DD_op",NULL);                               //     Create temp. result instance
      ((LP_MOP_FUNC)lpWord->ex.op.lpfCallback)                                  //     Execute operation
        (idTmp,idArg1,T_INSTANCE,idArg2,T_INSTANCE,nOpc);                       //     |
      PushInstance(idTmp);                                                      //     Push result
      return O_K;                                                               //     Operation executed
    }                                                                           //   <<
    else if (m_aStack[1].nType==T_INSTANCE)                                     //   Only argument 1 is an instance
    {                                                                           //   >>
      if (!CDlpObject_OfKind("data",m_aStack[1].val.i)) return FNC_TYPECAST;    //     Cannot type cast
      nArg2  = PopNumber(2);                                                    //     Pop scalar argument 2
      idArg1 = (CData*)PopInstance(1);                                          //     Pop data argument 1
      FNC_MSG(2,"  - Binary matrix operation",0,0,0,0,0);                       //     Protocol (verbose level 2)
      IFCHECKEX(2) printf(" %ld %s, %g+%gi",(long)nOpc,                         //     Protocol (verbose level 2)
        idArg1?idArg1->m_lpInstanceName:"NULL",(double)nArg2.x,(double)nArg2.y, //     |
        0);                                                                     //     |
      ICREATEEX(CData,idTmp,"#TMP#m_DN_op",NULL);                               //     Create temp. result instance
      ((LP_MOP_FUNC)lpWord->ex.op.lpfCallback)                                  //     Execute operation
        (idTmp,idArg1,T_INSTANCE,&nArg2,(nArg2.y==0)?T_DOUBLE:T_COMPLEX,nOpc);  //     |
      PushInstance(idTmp);                                                      //     Push result
      return O_K;                                                               //     Operation executed
    }                                                                           //   <<
    else if (m_aStack[0].nType==T_INSTANCE)                                     //   Only argument 2 is an instance
    {                                                                           //   >>
      if (!CDlpObject_OfKind("data",m_aStack[0].val.i)) return FNC_TYPECAST;    //     Cannot type cast
      idArg2 = (CData*)PopInstance(2);                                          //     Pop data argument 2
      nArg1  = PopNumber(1);                                                    //     Pop scalar argument 1
      FNC_MSG(2,"  - Binary matrix operation",0,0,0,0,0);                       //     Protocol (verbose level 2)
      IFCHECKEX(2) printf(" %ld %g+%gi, %s",(long)nOpc,                         //     Protocol (verbose level 2)
        (double)nArg1.x,(double)nArg1.y,idArg2?idArg2->m_lpInstanceName:"NULL", //     |
        0);                                                                     //     |
      ICREATEEX(CData,idTmp,"#TMP#m_ND_op",NULL);                               //     Create temp. result instance
      ((LP_MOP_FUNC)lpWord->ex.op.lpfCallback)                                  //     Execute operation
        (idTmp,&nArg1,(nArg1.y==0)?T_DOUBLE:T_COMPLEX,idArg2,T_INSTANCE,nOpc);  //     |
      PushInstance(idTmp);                                                      //     Push result
      return O_K;                                                               //     Operation executed
    }                                                                           //   <<
    else if (bDmop)                                                             //   Dedicated matrix operation
    {                                                                           //   >>
      nArg2 = PopNumber(2);                                                     //     Pop scalar argument 2
      nArg1 = PopNumber(1);                                                     //     Pop scalar argument 1
      FNC_MSG(2,"  - Binary matrix operation %ld %g+%gi, %g+%gi",(long)nOpc,    //     Protocol (verbose level 2)
        (double)nArg1.x,(double)nArg1.y,(double)nArg2.x,(double)nArg2.y);       //     |
      ICREATEEX(CData,idTmp,"#TMP#m_NN_op",NULL);                               //     Create temp. result instance
      ((LP_MOP_FUNC)lpWord->ex.op.lpfCallback)                                  //     Execute operation
        (idTmp,&nArg1,(nArg1.y==0)?T_DOUBLE:T_COMPLEX,&nArg2,                   //     |
        (nArg2.y==0)?T_DOUBLE:T_COMPLEX,nOpc);                                  //     |
      PushInstance(idTmp);                                                      //     Push result
      return O_K;                                                               //     Operation executed
    }                                                                           //   <<
  }                                                                             // <<
  return NOT_EXEC;                                                              // Ach n���!
}

/**
 * Executes signal operators.
 *
 * @param iCont
 *          signal instance
 * @param lpWord
 *          Operator word
 * @return <code>O_K</code> if the operation has been executed, a (negative)
 *         error code otherwise
 */
INT16 CGEN_PROTECTED CFunction::OpSignal(SWord* lpWord) {
  INT16   i      = 0;
  INT16   nRes   = (lpWord == NULL) ? -1 : lpWord->ex.op.nRes;
  INT16   nOps   = (lpWord == NULL) ? -1 : lpWord->ex.op.nOps;
  INT16   nOpc   = (lpWord == NULL) ? -1 : lpWord->ex.op.nOpc;
  CData*  idTmp  = NULL;
  StkItm* lpRes  = NULL;
  StkItm* lpArgs = NULL;
  char    lpsTmpName[64] = "";

  if(NOK(OpVerifySignature(lpWord->ex.op.lpsSig,nOps))) return NOT_EXEC;

  if(lpWord && lpWord->ex.op.lpfCallback) {
    lpRes = (StkItm*)dlp_calloc(nRes,sizeof(StkItm));
    for(i = 0; i < nRes; i++) {
      sprintf(lpsTmpName,"#TMP#%s_%d",dlp_sigop_sym(nOpc),i);
      ICREATEEX(CData,idTmp,lpsTmpName,NULL);                               //     Create temp. result instance
      lpRes[i].nType = T_INSTANCE;
      lpRes[i].val.i = idTmp;
    }
    lpArgs = (StkItm*)dlp_calloc(nOps,sizeof(StkItm));
    for(i = 0; i < nOps; i++) PopAny(0,&lpArgs[i]);
    ((LP_FOP_FUNC)lpWord->ex.op.lpfCallback)(nOpc,lpRes,lpArgs);
    for(i = 0; i < nRes; i++) PushInstance(lpRes[i].val.i);
    dlp_free(lpRes);
    dlp_free(lpArgs);
  }
  return O_K;
}

/**
 * Checks if signature of operation is consistent with stack
 *
 * @param lpsSig
 *          The signature
 * @param nOps
 *          Number of operands
 * @return <code>O_K</code> if consistent or <code>FNC_TYPECAST</code> if not.
 */
INT16 CGEN_PROTECTED CFunction::OpVerifySignature(const char* lpsSig, INT16 nOps) {
  INT16 i;
  INT16 nSigType;

  DLPASSERT(m_nStackLen>=nOps);

  for(i = 0; i < nOps; i++) {
    nSigType = dlp_op_opstype(lpsSig,nOps-i);
    if(nSigType & T_OP_INSTANCE) {
      if(m_aStack[i].nType != T_INSTANCE)
        return IERROR(this,FNC_TYPECAST,i,dlp_get_type_name(m_aStack[i].nType),dlp_get_type_name(T_INSTANCE));
      if(nSigType & T_OP_DATA) {
        if(m_aStack[i].val.i && !CDlpObject_OfKind("data",m_aStack[i].val.i))
          return IERROR(this,FNC_TYPECAST,i,m_aStack[i].val.i->m_lpClassName,"data");
      } else if(nSigType & T_OP_VAR) {
        if(m_aStack[i].val.i && !CDlpObject_OfKind("var",m_aStack[i].val.i))
          return IERROR(this,FNC_TYPECAST,i,m_aStack[i].val.i->m_lpClassName,"var");
      }
    } else if(nSigType & T_OP_REAL) {
      if(!dlp_is_float_type_code  (m_aStack[i].nType) &&
         !dlp_is_complex_type_code(m_aStack[i].nType))
        return IERROR(this,FNC_TYPECAST,i,dlp_get_type_name(m_aStack[i].nType),"real number");
    } else if(nSigType & T_OP_COMPLEX) {
      if(!dlp_is_complex_type_code(m_aStack[i].nType))
        return IERROR(this,FNC_TYPECAST,i,dlp_get_type_name(m_aStack[i].nType),dlp_get_type_name(T_COMPLEX));
    } else if(nSigType & T_OP_INTEGER) {
      if(!dlp_is_float_type_code  (m_aStack[i].nType) &&
         !dlp_is_complex_type_code(m_aStack[i].nType) &&
         !dlp_is_integer_type_code(m_aStack[i].nType))
        return IERROR(this,FNC_TYPECAST,i,dlp_get_type_name(m_aStack[i].nType),"integer");
    } else if(nSigType & T_OP_BOOL) {
      if(m_aStack[i].nType != T_BOOL)
        return IERROR(this,FNC_TYPECAST,i,dlp_get_type_name(m_aStack[i].nType),dlp_get_type_name(T_BOOL));
    } else if(nSigType & T_OP_STRING) {
      if(!dlp_is_symbolic_type_code(m_aStack[i].nType))
        return IERROR(this,FNC_TYPECAST,i,dlp_get_type_name(m_aStack[i].nType),dlp_get_type_name(T_STRING));
    }
  }
  return O_K;
}
/**
 * Interprets a token as a constant or a (scalar numeric or data) operator.
 *
 * @param lpsToken
 *          Pointer to the token to be interpreted
 * @return <code>O_K>/code> if the token was an operatiom code and the
 *         operation has been executed, a (negative) error code oterhwise
 */
INT16 CGEN_PROTECTED CFunction::ItpAsOperator(const char* lpsToken)
{
  INT16 nOpc  = -1;                                                             // Opcode
  INT16 nOps  = -1;                                                             // Number of operands
  SWord*      lpWord       = NULL;                                              // Word
  CDlpObject* iCont        = NULL;                                              // Container of word

  // Try constants                                                              // ------------------------------------
  nOpc = dlp_constant_code(lpsToken);                                           // Get opcode for token
  if (nOpc>=0)                                                                  // Operation found
  {                                                                             // >>
    COMPLEX64 nConst = dlp_constant(nOpc);                                      //   Retrieve constant
    if (dlp_is_logic_op_code(nOpc))                                             //   Logical constant
      PushLogic((BOOL)nConst.x);                                                //     Push
    else if (dlp_is_pointer_op_code(nOpc))                                      //   Pointer constant
      PushInstance((CDlpObject*)(size_t)nConst.x);                              //     Push
    else                                                                        //   All other constans are numeric
      PushNumber(nConst);                                                       //     Push
    return O_K;                                                                 //   Ok
  }                                                                             // <<

  // Try matrix, scalar and signal operations                                   // ------------------------------------
  if ((iCont=FindInstance("matrix")) && (lpWord=iCont->FindOperator(lpsToken)))// Is lpsToken a matrix op. symbol?
  {
    IF_OK(OpEqual(lpWord)) return O_K;                                          // Equality operators
    IF_OK(OpLogic(lpWord)) return O_K;                                          // Logic operators
    IF_OK(OpMatrx(lpWord)) return O_K;                                          // Matrix operation
  }
  if ((lpWord = this->FindOperator(lpsToken)))                                  // Is lpsToken a scalop symbol?
  {                                                                             // >>
    nOps = lpWord->ex.op.nOps;                                                  //   Get number of operators
    nOpc = lpWord->ex.op.nOpc;                                                  //   Get operator code

    if (nOps>m_nStackLen)                                                       //   There are too few arguments
    {                                                                           //   >>
      IERROR(this,FNC_STACKUNDERFLOW," on operator ",lpsToken,0);               //     Stack underflow error
      StackClear();                                                             //     Clear stack
      return O_K;                                                               //     Ok
    }                                                                           //   <<
    IF_OK(OpEqual(lpWord)) return O_K;                                          //   Equality operators
    IF_OK(OpLogic(lpWord)) return O_K;                                          //   Logic operators
    IF_OK(OpStrsc(lpWord)) return O_K;                                          //   Scalar string operators

    // All other operations purely numeric                                      // ------------------------------------
    COMPLEX64 nArg1 = CMPLX(0.);                                                //   Argument #1
    COMPLEX64 nArg2 = CMPLX(0.);                                                //   Argument #2
    if (nOps>1) nArg2 = PopNumber(2);                                           //   Get optional second argument
    nArg1 = PopNumber(1);                                                       //   Get mandatory first argument
    PushNumber(dlp_scalopC(nArg1,nArg2,nOpc));                                  //   Calculate result and push to stack
    return O_K;                                                                 //   Ok
  }                                                                             // <<
  if ((iCont=FindInstance("signal")) && (lpWord=iCont->FindOperator(lpsToken))) // Is lpsToken a signal op. symbol?
    if(OpSignal(lpWord)==O_K) return O_K;                                       //   Signal operation

  return NOT_EXEC;                                                              // Its not an operation
}

/**
 * Called when instance identifiers is being interpreted.
 *
 * @param iInst
 *          Pointer to instance to be iterpreted
 * @param iCont
 *          Pointer to container instance of <code>iInst</code>
 */
void CGEN_PROTECTED CFunction::ItpInstance(CDlpObject* iInst)
{
  PushInstance(iInst);                                                          // Push inst. to stack (and activate)
/*
  if ((iInst->m_nClStyle & CS_SECONDARY)==0)                                    // If not secondary
  {                                                                             // >>
    PushInstance(iInst);                                                        //   Push instance to stack
    m_bAiUsed = FALSE;                                                          //   Mark virginal
  }                                                                             // <<
  else m_bAiUsed = TRUE;                                                        // Secondary: mark used
  m_iAi = iInst;                                                                // Set active
  IFCHECKEX(2) printf(", activating instance");                                 // Protocol
*/
  if (iInst && GetRootFnc() && !GetRootFnc()->m_bDisarm)                        // If class proc. enabled
  {                                                                             // >>
// MWX 2006-06-31: Change MIC tracking -->                                      //   ./.
//  lpMicSave = CDlpObject_MicSet(iInst,&m_mic);                                //   ./.
    DLPASSERT(CDlpObject_MicSet(iInst,&m_mic)==NULL);                           //   Set MIC, assert clean!
// <--                                                                          //   ./.
    FNC_MSG(2,"  - Invoking class proc.",0,0,0,0,0);                            //   Protocol
    iInst->ClassProc();                                                         //   Invoke class proc.
// MWX 2006-06-31: Change MIC tracking -->                                      //   ./.
//  CDlpObject_MicSet(iInst,lpMicSave);                                         //   ./.
    CDlpObject_MicSet(iInst,NULL);                                              //   Clear invocation context
// <--                                                                          //   ./.
  }                                                                             // <<
  else if (iInst) FNC_MSG(2,"  - DISARMED",0,0,0,0,0);                          // Protocol
  if(GetRootFnc()) GetRootFnc()->m_bDisarm = FALSE;                             // Clear /disarm option
}

// EOF
