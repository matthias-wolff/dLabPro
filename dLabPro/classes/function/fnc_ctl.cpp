// dLabPro class CFunction (function)
// - Program control
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

/*
 * Manual page at function.def
 */
INT16 CGEN_PUBLIC CFunction::Exec()
{
  DLPASSERT((m_nXm & XM_ARCHIVE) || !(m_nXm & XM_EXEC));                        // Is already being executed!

  // Start execution                                                            // ------------------------------------
  dlp_register_signals();                                                       // Register dlp_interrupt to CTRL-C
  IFCHECKEX(1)                                                                  // On verbose level 1
  {                                                                             // >>
    char lpsFqName[255];                                                        //   Do some screen protocol
    printf("\n Executing function %s",GetFQName(lpsFqName));                    //   ...
    printf("\n   PP   #T #S *%-13s: Message","ACTIVE");                         //   ...
  }                                                                             // <<
  StartExec();                                                                  // Initialize executing function

  // Token loop                                                                 // ------------------------------------
  FNC_MSG(1,"Starting token loop",0,0,0,0,0);                                   // Protocol
  while (ExecuteToken()!=FNC_NOMORETOKENS && !(m_nXm&XM_QUIT)) {};              // Token executing loop

  // Break in step mode after last token                                        // ------------------------------------
  if (StepBreak())                                                              // Step break
    while (ExecuteToken()!=FNC_NOMORETOKENS && !(m_nXm&XM_QUIT)) {};            //   Token executing loop

  // End execution                                                              // ------------------------------------
  FNC_MSG(1,"End of function",0,0,0,0,0);                                       // Protocol
  StopExec();                                                                   // Cleanup function after execution

  return O_K;                                                                   // Ok
}

/**
 * Initializes the function for execution.
 *
 * @return <code>O_K</code> if successfull, a (negative) error code otherwise.
 */
INT16 CGEN_PRIVATE CFunction::StartExec()
{
  INT32       nTok    = 0;                                                      // Current token
  INT32       nXTok   = 0;                                                      // Number of tokens
  INT32       nSrcid  = -1;                                                     // Source file @ caller's program ptr.
  INT32       nLine   = -1;                                                     // Source line @ caller's program ptr.
  CFunction* iCaller = NULL;                                                    // Caller

  FNC_MSG(1,"Initializing",0,0,0,0,0);                                          // Protocol
  DLPASSERT(m_idArg)                                                            // Must initialize arg. list before!

  // Initialize token sequence                                                  // ------------------------------------
  iCaller = GetCaller();                                                        // Get caller
  if ((m_nXm & XM_INLINE) && iCaller)                                           // Inline function (and have caller)
  {                                                                             // >>
    nSrcid = (INT32)CData_Dfetch(iCaller->m_idTsq,iCaller->m_nPp,OF_SRCID);     //   Get calling source file
    nLine  = (INT32)CData_Dfetch(iCaller->m_idTsq,iCaller->m_nPp,OF_LINE );     //   Get calling source line
    nXTok  = m_idTsq->GetNRecs();                                               //   Get number of tokens
    CData_Sstore(m_idSfl,CData_Sfetch(iCaller->m_idSfl,nSrcid,0),0,0);          //   Fake source file name
    for (nTok=0; nTok<nXTok; nTok++)                                            //   Loop over tokens
    {                                                                           //   >>
      *(INT32*)m_idTsq->XAddr(nTok,OF_SRCID) = 0;                               //     Fake source file index
      *(INT32*)m_idTsq->XAddr(nTok,OF_LINE ) = nLine;                           //     Fake source line
    }                                                                           //   <<
  }                                                                             // <<

  // Initialize function                                                        // ------------------------------------
  IFIELD_RESET(CData,"teq"); CDgen::TsqInit(m_idTeq);                           // Initialize token execution queue
  StackInit();                                                                  // Initialize stack
  m_lpMic = NULL;                                                               // Clear MIC (caller in m_iCaller!)
  if (!(m_nXm&XM_ARCHIVE)) m_nPp = 0;                                           // Initialize program pointer
  m_nXm &= ~XM_ARCHIVE;                                                         // Clear archive flag
  m_nXm |= XM_EXEC;                                                             // Set executing state
  m_bDisarm = FALSE;                                                            // Reset disarm option
  if (m_idTsq->GetNRecs()==0 && GetRoot()==this) m_nXm |= XM_BREAK;             // Empty root function -> break mode
  StepBreak();                                                                  // Break here in step mode
  return O_K;                                                                   // Ok
}

/**
 * Deinitializes the function after execution.
 *
 * @return <code>O_K</code> if successfull, a (negative) error code otherwise.
 */
INT16 CGEN_PRIVATE CFunction::StopExec()
{
  IFCHECKEX(1) printf("\n Cleaning up");                                        // Protocol (Verbose level 1)
  m_nXm &= ~XM_EXEC;                                                            // Reset executing state

  StackDestroy();                                                               // Destroy stack
  ArgClear();                                                                   // Clear actual arguments
  m_iAi  = NULL;                                                                // Clear primary active instance
  m_iAi2 = NULL;                                                                // Clear secondary active instance

  SWord*   lpWord = NULL;                                                       // Pointer to currently examined word
  hscan_t  hs;                                                                  // Hash scan data struct
  hnode_t* hn     = NULL;                                                       // Pointer to current hash node
  hash_scan_begin(&hs,m_lpDictionary);                                          // Initialize scanning dictionary
  while ((hn = hash_scan_next(&hs))!=NULL)                                      // Scan dictionary
  {                                                                             // >>
    DLPASSERT((lpWord = (SWord*)hnode_get(hn))!=NULL);                          //   NULL entry in dictionary
    if (lpWord->nWordType == WL_TYPE_INSTANCE)                                  //   Is current word is an instance?
    {                                                                           //   >>
      IFCHECKEX(2) printf("\n - Destroy instance %s",lpWord->lpName);           //     Protocol (verbose level 2)
      Deinstanciate((CDlpObject*)lpWord->lpData,0);                             //     Destroy it
    }                                                                           //   <<
  }                                                                             // <<
  IDESTROY(m_idTeq);                                                            // Destroy token queue (saves memory)

  if ((m_nXm & XM_QUIT) && GetCaller()) GetCaller()->m_nXm |= XM_QUIT;          // Propagate quit state to calling fnc.
  return O_K;                                                                   // Ok
}

/**
 * Jumps to a label.
 *
 * @param lpsLabel
 *          The label identifier
 */
INT16 CGEN_PROTECTED CFunction::JumpLabel(const char* lpsLabel)
{
  INT32 nTok = 0;                                                                // Current token

  // Validation                                                                 // ------------------------------------
  DLPASSERT(m_nXm & XM_EXEC);                                                   // Must only be called when running
  if (!lpsLabel) return NOT_EXEC;                                               // No label, no service!

  // Seek label                                                                 // ------------------------------------
  FNC_MSG(1,"JMP(\"%s\")",lpsLabel,0,0,0,0);                                    // Protocol
  for (nTok=m_nPp-1; nTok>=0; nTok--)                                           // Seek backward
    if (__TTYP_IS(nTok,TT_LAB))                                                 //   Token is a label definition...
      if (__TOK_IS(nTok,lpsLabel))                                              //     The right one...
        break;                                                                  //       Got'cha!
  if (nTok<0)                                                                   // Didn't find label
    for (nTok=m_nPp+1; nTok<m_idTsq->GetNRecs(); nTok++)                        //   Seek forward
      if (__TTYP_IS(nTok,TT_LAB))                                               //     Token is a label definition...
        if (__TOK_IS(nTok,lpsLabel))                                            //       The right one...
          break;                                                                //         Got'cha!
  if (nTok>=m_idTsq->GetNRecs())                                                // Or didn't I?
    return IERROR(this,FNC_LABEL,lpsLabel,0,0);                                 //   oops...
  m_nPp=nTok+1;                                                                 // Continue at token following label

  IFCHECKEX(1) printf(", PP=%ld ",(long)m_nPp);                                 // Protocol
  return O_K;                                                                   // Ok
}

/**
 * Conditional branch. Jumps to
 * <ul>
 *   <li><code><b>bCondition=TRUE </code></b> the following token,</li>
 *   <li><code><b>bCondition=FALSE</code></b> the matching <code>else</code>
 *     or, if no such token exists, to the matching <code>endif</code>
 *     token</li>
 * </ul>
 *
 * @param bCondition
 *          Jump condition: <code>TRUE</code> for if-path, <code>FALSE</code>
 *          for else-path.
 * @return <code>O_K</code> if successfull, a (negative) error code otherwise.
 */
INT16 CGEN_PROTECTED CFunction::JumpConditional(BOOL bCondition)
{
  // Validation                                                                 // ------------------------------------
  DLPASSERT(m_nXm & XM_EXEC);                                                   // Must only be called when running

  INT32 nIfLv = (INT32)m_idTsq->Dfetch(m_nPp-1,OF_BLV1);                        // Get current if nesting level
  FNC_MSG(1,"JMPC(%c%ld)",bCondition?'+':'-',(long)nIfLv,0,0,0);                // Protocol

  if (bCondition)                                                               // TRUE path:
  {                                                                             // >>
    IFCHECKEX(1) printf(", PP=%ld ",(long)m_nPp);                               //   Protocol
    return O_K;                                                                 //   Nothing to be done for this....
  }                                                                             // <<
                                                                                // FALSE path:
  for (; m_nPp<m_idTsq->GetNRecs(); m_nPp++)                                    // Loop over remaining tokens
    if ((INT32)m_idTsq->Dfetch(m_nPp,OF_BLV1)==nIfLv)                           //   On same if nesting level
    {                                                                           //   >>
      if (__TOK_IS(m_nPp,"else" )) break;                                       //     "else"  --> found false path
      if (__TOK_IS(m_nPp,"end"  )) break;                                       //     "end" --> found end of branch
    }                                                                           //   <<
  if (m_nPp>=m_idTsq->GetNRecs())                                               // Or did't I?
    return IERROR(this,FNC_UNEXOEF,"\'else\' or \'end\'",0,0);                  //   Error
  m_nPp++;                                                                      // Continue at next token

  IFCHECKEX(1) printf(", PP=%ld ",(long)m_nPp);                                 // Protocol
  return O_K;                                                                   // Ok
}

/**
 * End of conditional branch. Jumps to the matching <code>endif</code> token.
 *
 * @return <code>O_K</code> if successfull, a (negative) error code otherwise.
 */
INT16 CGEN_PROTECTED CFunction::JumpEnd()
{
  // Validation                                                                 // ------------------------------------
  DLPASSERT(m_nXm & XM_EXEC);                                                   // Must only be called when running

  INT32 nIfLv = (INT32)m_idTsq->Dfetch(m_nPp-1,OF_BLV1);                        // Get current if nesting level
  FNC_MSG(1,"JMP(END,%ld)",(long)nIfLv,0,0,0,0);                                // Protocol

  for (; m_nPp<m_idTsq->GetNRecs(); m_nPp++)                                    // Loop over remaining tokens
    if ((INT32)m_idTsq->Dfetch(m_nPp,OF_BLV1)==nIfLv)                           //   On same if nesting level
      if (__TOK_IS(m_nPp,"end")) break;                                         //     "end" --> that's my guy
  if (m_nPp>=m_idTsq->GetNRecs())                                               // Or isn't it?
    return IERROR(this,FNC_UNEXOEF,"'endif'",0,0);                              //   Error
  m_nPp++;                                                                      // Continue at token following "end"

  IFCHECKEX(1) printf(", PP=%ld ",(long)m_nPp);                                 // Protocol
  return O_K;                                                                   // Ok
}

/**
 * Jump back to while token
 */
INT16 CGEN_PROTECTED CFunction::JumpWhile()
{
  // Validation                                                                 // ------------------------------------
  DLPASSERT(m_nXm & XM_EXEC);                                                   // Must only be called when running

  INT32 nIfLv = (INT32)m_idTsq->Dfetch(m_nPp-1,OF_BLV1);                        // Get current if nesting level
  FNC_MSG(1,"JMP(WHILE)",0,0,0,0,0);                                            // Protocol

  for (; m_nPp>=0; m_nPp--)                                                     // Loop over remaining tokens
    if ((INT32)m_idTsq->Dfetch(m_nPp,OF_BLV1)==nIfLv)                           //   On same if nesting level
    {
      if (__TOK_IS(m_nPp,"if"   )) nIfLv--;
      if (__TOK_IS(m_nPp,"while")) break;                                       //     "while" --> that's my guy
    }
  if (m_nPp<0)                                                                  // Or isn't it?
    return IERROR(this,FNC_UNEXOEF,"'while'",0,0);                              //   Error

  IFCHECKEX(1) printf(", PP=%ld ",(long)m_nPp);                                 // Protocol
  return O_K;                                                                   // Ok
}

/*
 * Manual page at function.def
 */
INT16 CGEN_PROTECTED CFunction::Swap()
{
  // Delegate to running function                                               // ------------------------------------
  FNC_DELEGATE Swap();                                                          // Use a weird macro (see function.def)

  StackSwap();                                                                  // Do the work
  return O_K;
}

/*
 * Manual page at function.def
 */
INT16 CGEN_PROTECTED CFunction::Dup()
{
  // Delegate to running function                                               // ------------------------------------
  FNC_DELEGATE Swap();                                                          // Use a weird macro (see function.def)

  StackDup();                                                                  // Do the work
  return O_K;
}

/*
 * Manual page at function.def
 */
INT16 CGEN_PROTECTED CFunction::Rot()
{
  // Delegate to running function                                               // ------------------------------------
  FNC_DELEGATE Swap();                                                          // Use a weird macro (see function.def)

  StackRot();                                                                   // Do the work
  return O_K;
}

/*
 * Manual page at function.def
 */
INT16 CGEN_PROTECTED CFunction::If(BOOL bCondition)
{
  // Delegate to running function                                               // ------------------------------------
  FNC_DELEGATE If(bCondition);                                                  // Use a weird macro (see function.def)

  // Jump to end of token sequence                                              // ------------------------------------
  if (m_nXm & XM_BREAK)                                                         // Interactive mode?
    return IERROR(this,FNC_NOTALLOWED,"if","in interactive mode",0);            //   That's not allowed!
  return JumpConditional(bCondition);                                           // Conditional branch
}

/*
 * Manual page at function.def
 */
INT16 CGEN_PROTECTED CFunction::Else()
{
  // Delegate to running function                                               // ------------------------------------
  FNC_DELEGATE Else();                                                          // Use a weird macro (see function.def)

  // Jump to end of token sequence                                              // ------------------------------------
  if (m_nXm & XM_BREAK)                                                         // Interactive mode?
    return IERROR(this,FNC_NOTALLOWED,"else","in interactive mode",0);          //   That's not allowed!
  return JumpEnd();                                                             // Unconditional jump to "end"
}

/*
 * Manual page at function.def
 */
INT16 CGEN_PROTECTED CFunction::While(BOOL bCondition)
{
  // Delegate to running function                                               // ------------------------------------
  FNC_DELEGATE While(bCondition);                                               // Use a weird macro (see function.def)

  // If condition negative, jump to end of block                                // ------------------------------------
  if (m_nXm & XM_BREAK)                                                         // Interactive mode?
    return IERROR(this,FNC_NOTALLOWED,"while","in interactive mode",0);         //   That's not allowed!
  if (!bCondition) JumpEnd();

  return O_K;
}

/*
 * Manual page at function.def
 */
INT16 CGEN_PROTECTED CFunction::Break()
{
  // Delegate to running function                                               // ------------------------------------
  FNC_DELEGATE Break();                                                         // Use a weird macro (see function.def)

  // Backward compatibility                                                     // ------------------------------------
  if (StackGetLength()>0)
  {
    IERROR(this,FNC_COMPAT," did you mean \"brk\"?",0,0);
    return Brk(PopString());
  }

  // Validate
  if (m_nXm & XM_BREAK)                                                         // Interactive mode?
    return IERROR(this,FNC_NOTALLOWED,"while","in interactive mode",0);         //   That's not allowed!

  // Jump to end of while(!) block                                              // ------------------------------------
  JumpWhile(); m_nPp++;
  JumpEnd();
  return O_K;
}

/*
 * Manual page at function.def
 */
INT16 CGEN_PROTECTED CFunction::Continue()
{
  // Delegate to running function                                               // ------------------------------------
  FNC_DELEGATE Continue();                                                      // Use a weird macro (see function.def)
  if (m_nXm & XM_BREAK)                                                         // Interactive mode?
    return IERROR(this,FNC_NOTALLOWED,"while","in interactive mode",0);         //   That's not allowed!

  // Jump while label                                                           // ------------------------------------
  JumpWhile();
  for (; m_nPp>=0; m_nPp--)
    if (__TTYP_IS(m_nPp,TT_LAB))
      break;

  return O_K;
}

/*
 * Manual page at function.def
 */
INT16 CGEN_PROTECTED CFunction::Goto()
{
  // Delegate to running function                                               // ------------------------------------
  FNC_DELEGATE Goto();                                                          // Use a weird macro (see function.def)

  // Jump to end of token sequence                                              // ------------------------------------
  const char* lpsLabelname = GetNextToken();                                    // Get the next token
  if (!dlp_strlen(lpsLabelname))                                                // If not exists...
    return IERROR(this,FNC_EXPECT,"label name",0,0);                            //   Synatx error
  return JumpLabel(lpsLabelname);                                               // Unconditional jump to label
}

/*
 * Manual page at function.def
 */
INT16 CGEN_PROTECTED CFunction::Leave()
{
  // Delegate to running function                                               // ------------------------------------
  FNC_DELEGATE Leave();                                                         // Use a weird macro (see function.def)

  // Leave from root function                                                   // ------------------------------------
  if (!GetCaller())                                                             // Root function (main program)
  {                                                                             // >>
    FNC_MSG(2,"  - Return/leave in root function",0,0,0,0,0);                   //   Protocol
    Quit();                                                                     //   Leaving root function is quitting
    return O_K;                                                                 //   Still good this far ...
  }                                                                             // <<

  // Jump to end of token sequence                                              // ------------------------------------
  FNC_MSG(2,"  - JMP(EOF)",0,0,0,0,0);                                          // Protocol
  m_nPp = m_idTsq->GetNRecs();                                                  // Move program ptr. to EOF
  IFCHECKEX(2) printf(", PP=%ld ",(long)m_nPp);                                 // Protocol
  return O_K;                                                                   // Ok
}

/*
 * Manual page at function.def
 */
INT16 CGEN_PROTECTED CFunction::Return()
{
  // Delegate to running function                                               // ------------------------------------
  FNC_DELEGATE Return();                                                        // Use a weird macro (see function.def)

  // Do return                                                                  // ------------------------------------
  ArgReturnVal();                                                               // Return top of stack
  return Leave();                                                               // Jump to EOF
}

/*
 * Manual page at function.def
 */
INT16 CGEN_PUBLIC CFunction::Brk(const char* sId)
{
  // Delegate to running function                                               // ------------------------------------
  FNC_DELEGATE Brk(sId);                                                        // Use a weird macro (see function.def)

  // Enter break state                                                          // ------------------------------------
  if (dlp_get_nonstop_mode()) return O_K;                                       // Do not break in nonstop mode
  printf("\nBREAK POINT '%s'",sId);                                             // Print break point message
  PrintStackTrace();                                                            // Print stack trace
  m_nXm |= XM_BREAK;                                                            // Set break mode
  return O_K;                                                                   // Jo
}

/*
 * Manual page at function.def
 */
INT16 CGEN_PUBLIC CFunction::Cont()
{
  // Delegate to running function                                               // ------------------------------------
  FNC_DELEGATE Cont();                                                          // Use a weird macro (see function.def)

  // Leave break state                                                          // ------------------------------------
  if ((m_nPp<0 || m_nPp>=m_idTsq->GetNRecs()) && GetRoot()==this) return O_K;   // Do not continue at end of root fctn.
  m_nXm &= ~XM_BREAK;                                                           // Reset break mode
  return O_K;                                                                   // Jo
}

/*
 * Manual page at function.def
 */
INT16 CGEN_PUBLIC CFunction::Step()
{
  // Delegate to running function                                               // ------------------------------------
  FNC_DELEGATE Step();                                                          // Use a weird macro (see function.def)

  // Set step mode                                                              // ------------------------------------
  if (!(m_nXm & XM_BREAK)) return NOT_EXEC;                                     // Only in break mode
  m_nXm &= ~XM_BREAK;                                                           // Clear break mode
  if (GetRootFnc()) GetRootFnc()->m_nXm |= XM_STEP;                             // Set step mode
  return O_K;                                                                   // OK
}

/*
 * Manual page at function.def
 */
INT16 CGEN_VPROTECTED CFunction::Quit()
{
  // Delegate to running function                                               // ------------------------------------
  FNC_DELEGATE Quit();                                                          // Use a weird macro (see function.def)

  // Do quit                                                                    // ------------------------------------
  FNC_MSG(1,"QUIT(%ld)",(long)dlp_get_retval(),0,0,0,0);                        // Protocol
  m_nXm |= XM_QUIT;                                                             // Set quit state
  return O_K;                                                                   // Ok
}

// EOF
