// dLabPro class CFunction (function)
// - formula translator
//
// AUTHOR : Robert Schubert
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
#include "dlp_dgen.h"
#include "dlp_math.h"

/**
 * Translates a mathematical expression in prefix/infix notation (formula)
 * into its equivalent reverse polish notation (RPN) code in dlabpro. The method
 * uses this class' internal DGen instance {@link par},
 * {@link FormulaTagSyntax}, and {@link FormulaTranslate}).
 *
 * @param lpsIn
 *          Pointer to the input string (formula)
 * @param lpsOut
 *          Pointer to memory area allocated for the output string (code)
 * @param nMaxLength
 *          Size of the allocated memory in bytes
 * @return success indicator
 * @see <a class="code" href="cgen.html">CDgen</a>
 * @see FormulaTagSyntax
 * @see FormulaTranslate
 */
INT16 CGEN_PROTECTED CFunction::Formula2RPN(const char *lpsIn, char* lpsOut, INT16 nMaxLength)
{
  if (!lpsOut) return NOT_EXEC;                                                 // No output buffer, no service!
  lpsOut[0]='\0';                                                               // Clear output buffer
  if (!dlp_strlen(lpsIn)) return NOT_EXEC;                                      // No output buffer, nearly no service!
  if (nMaxLength<=0) return NOT_EXEC;                                           // Are you gonna fool me?

  //
  // add white space character to end of input string (insignificant delimiter)
  //
  char* lpsInCopy = (char*) dlp_malloc((3 + dlp_strlen(lpsIn)) * sizeof(char));
  dlp_strcpy(lpsInCopy, lpsIn);
  dlp_strcat(lpsInCopy, "\n");

  CDgen* iPar = GetDlpParser();
  DLPASSERT(iPar);                                                              // need internal parser
  DLPASSERT(iPar->m_idTsq);                                                     // need internal parser's data structures

  FNC_MSG(1, "  - beginning translation of '%s' to RPN code", lpsIn, 0,0,0,0);

  //
  // tokenise the mathematical expression using function's internal DGen instance
  //
  iPar->Setup("fml");
  iPar->TsqInit(iPar->m_idTsq);          // reset token sequence structure
  IF_NOK (iPar->TokenizeString(lpsInCopy)) // invoke first tokenisation
  {
    FNC_MSG(1, "  - translation failed on first level tokenisation:", 0, 0, 0, 0, 0);
    if (m_nCheck >= 1) iPar->m_idTsq->PrintList();
    DLPTHROW(NOT_EXEC);
  }
  FNC_MSG(2, "  - result of first level tokenisation:", 0, 0, 0, 0, 0);
  if (m_nCheck >= 2) iPar->m_idTsq->PrintList();
  IF_NOK (iPar->Tokenize2("fml"))         // invoke second, infix-specific tokenisation
              // (bracket parity check, two-character operators)
  {
    FNC_MSG(1, "   - translation failed on second level tokenisation:", 0, 0, 0, 0, 0);
    if (m_nCheck >= 1) iPar->m_idTsq->PrintList();
    DLPTHROW(NOT_EXEC);
  }
  FNC_MSG(2, "  - result of second level tokenisation:", 0, 0, 0, 0, 0);
  if (m_nCheck >= 2) iPar->m_idTsq->PrintList();

  //
  // run syntax tagger
  //
  IF_NOK (FormulaTagSyntax(iPar->m_idTsq))
  {
    FNC_MSG(1, "  - translation failed on syntax tagging:", 0, 0, 0, 0, 0);
    if (m_nCheck >= 1) iPar->m_idTsq->PrintList();
    DLPTHROW(NOT_EXEC);
  }
  FNC_MSG(2, "  - result of syntax tagging:", 0, 0, 0, 0, 0);
  if (m_nCheck >= 2) iPar->m_idTsq->PrintList();

  //
  // run translator algorithm
  //
  IF_NOK (FormulaTranslate(iPar->m_idTsq, lpsOut, nMaxLength))
  {
    FNC_MSG(1, "  - translation failed on postfix algorithm:", 0, 0, 0, 0, 0);
    if (m_nCheck >= 1) iPar->m_idTsq->PrintList();
    DLPTHROW(NOT_EXEC);
  }
  FNC_MSG(1, "  - result of postfix algorithm: '%s'", lpsOut, 0, 0, 0, 0);

  dlp_free(lpsInCopy);                                                          // Free copy of input buffer
  iPar->m_idTsq->Allocate(0);                                                   // Clear token sequence data
  iPar->TsqInit(iPar->m_idTsq);                                                 // Reset token sequence structure
  iPar->Setup("dlp");                                                           // Restore default tokenizer state
  return O_K;

DLPCATCH(NOT_EXEC)
  lpsOut[0] = '\0';                                                             // Clear output buffer (whatever)
  dlp_free(lpsInCopy);                                                          // Free copy of input buffer
  iPar->m_idTsq->Allocate(0);                                                   // Clear tokenizer data
  iPar->TsqInit(iPar->m_idTsq);                                                 // Reset token sequence structure
  iPar->Setup("dlp");                                                           // Restore default state
  return NOT_EXEC;
}

/**
 <p>
 Tag the tokenised formula: Classify each token in the data table
 into operand/operation types and replace its <code>ttyp</code> value with
 that opcode.
 </p>

 target type topology:
 <table>
  <tr><th>type</th><th>description</th></tr>
  <tr><td><code>TT_NUM</code></td><td>integer/floating-point number literal</td></tr>
  <tr><td><code>TT_STR</code></td><td>string literal</td></tr>
  <tr><td><code>TT_DATA</code></td><td>data instance as literal, as monadic functor (component fetch, component store), as binary functor (cell fetch, cell store)</td></tr>
  <tr><td><code>TT_VAR</code></td><td>variable literal</td></tr>
  <tr><td><code>TT_WORD</code></td><td>other instance or field of numerical/boolean/string type</td></tr>
  <tr><td><code>TT_FUNC</code></td><td>prefix scalar/aggregation/string functor</td></tr>
  <tr><td><code>TT_OBR</code></td><td>opening round/square/curly bracket</td></tr>
  <tr><td><code>TT_CBR</code></td><td>closing round/square/curly bracket</td></tr>
  <tr><td><code>TT_ASEP</code></td><td>function argument separator (comma)</td></tr>
  <tr><td><code>TT_OPR</code></td><td>prefix/infix operator</td></tr>
 </table>

 special considerations:
 <br>
 <ul>
     <li>   <code>TT_NUM/TT_STR</code> have read-only semantics,
            but <code>TT_VAR/TT_DATA/TT_WORD</code> have both read and write semantics,
            depending on context. Thus whenever processing one of these tokens in
            {@link FormulaTranslate}, their right-hand side (forward context) will have to
      be checked for occurrance of the set operator <code>=</code> ("look-ahead").
      In that case, output code must be modified to account for leftvalue specialties
      (<code>=, -set, -copy, -store, -xstore</code>).
</li><li>   variables pointing to some data instance are classified <code>TT_DATA</code>
</li><li>   Only those prefix functors (<code>TT_FUNC</code>) listed in <code>-list aggrops -list strops -list scalops</code> are recognised.
</li>
 </ul>
 @param idTsq data table with untagged token sequence queue
 @return success indicator
 @see Formula2RPN
 */
INT16 CGEN_PROTECTED CFunction::FormulaTagSyntax(CData* idTsq)
{
  INT16       nTok;
  const char* lpsTokType = NULL;
  const char* lpsToken   = NULL;
  COMPLEX64   n;
  SWord*      lpWord     = NULL;

  // use the numeric "line" column for opcodes
  idTsq->SetCname(OF_TTYP, "opc");

  for (nTok = 0; nTok < idTsq->GetNRecs(); nTok++)
  {
    lpsTokType = idTsq->Sfetch(nTok, OF_TTYP);
    lpsToken = idTsq->Sfetch(nTok, OF_TOK);

    if (0 == dlp_strcmp(lpsTokType, TT_DEL))
    {
      // token is an operator, comma, or bracket
      if (0 == dlp_strcmp(lpsToken, ","))
        idTsq->Sstore(TT_ASEP, nTok, OF_TTYP);
      else if (0 == dlp_strcmp(lpsToken, "(") || 0 == dlp_strcmp(lpsToken, "[") || 0 == dlp_strcmp(lpsToken, "{"))
        idTsq->Sstore(TT_OBR, nTok, OF_TTYP);
      else if (0 == dlp_strcmp(lpsToken, ")") || 0 == dlp_strcmp(lpsToken, "]") || 0 == dlp_strcmp(lpsToken, "}"))
        idTsq->Sstore(TT_CBR, nTok, OF_TTYP);
      else
        idTsq->Sstore(TT_OPR, nTok, OF_TTYP);
    }
    else if (0 == dlp_strcmp(lpsTokType, TT_UNK))
    {
      // Seek word for token
      lpWord = FindWordAi(lpsToken);                                            // Find word for token
      if (lpWord && lpWord->lpData)
      {
        switch (lpWord->nWordType)                                              // Branch by word type
        {
        case WL_TYPE_METHOD:  /* FALL THROUGH */                                // - Method:
        case WL_TYPE_OPTION:  /* FALL THROUGH */                                // - Option:
        case WL_TYPE_FACTORY:
          return IERROR(this,FNC_INVALID,"word type of token",lpsToken,0);
        case WL_TYPE_FIELD:                                                     // - Field:
          if (T_INSTANCE != lpWord->ex.fld.nType)
            idTsq->Sstore(TT_WORD, nTok, OF_TTYP);
          else
          {
            if (0 == dlp_strcmp(lpWord->ex.fld.lpType, "data"))
              idTsq->Sstore(TT_DATA, nTok, OF_TTYP);
            else if (0 == dlp_strcmp(lpWord->ex.fld.lpType, "var"))
              idTsq->Sstore(TT_VAR, nTok, OF_TTYP);
            else
              return
                IERROR(this,FNC_INVALID,"type of instance field",lpsToken,0);
          }
          break;
        case WL_TYPE_INSTANCE:                                                  // - Instance:
          if (CDlpObject_OfKind("data",(CDlpObject*)lpWord->lpData))
            idTsq->Sstore(TT_DATA, nTok, OF_TTYP);
          else if (CDlpObject_OfKind("var",(CDlpObject*)lpWord->lpData))
            idTsq->Sstore(TT_VAR, nTok, OF_TTYP);
          else
            return IERROR(this,FNC_INVALID,"instance type",lpsToken,0);
          break;
        }
      }
      else if                                                                   // Functor ?
      (
        dlp_scalop_code(lpsToken) != -1 ||
        dlm_matrop_code(lpsToken,NULL) != -1 ||
        dlp_aggrop_code(lpsToken) != -1 ||
        dlp_strop_code(lpsToken) != -1 ||
        dlp_sigop_code(lpsToken) != -1
      )
      {
        idTsq->Sstore(TT_FUNC, nTok, OF_TTYP);
      }
      else if (-1 != dlp_constant_code(lpsToken))                               // Constant symbol ?
        idTsq->Sstore(TT_NUM, nTok, OF_TTYP);
      else if (dlp_sscanc(lpsToken,&n) == O_K)                                  // Literal number ?
        idTsq->Sstore(TT_NUM, nTok, OF_TTYP);
      else
        return IERROR(this,FNC_UNDEF,lpsToken,0,0);
    }
    else if (0 != dlp_strcmp(lpsTokType, TT_STR))
    {
      // no clue on meaning of character, white space, end of line, form, label, if, else, endif here...
      return IERROR(this,FNC_INVALID,"formula token",lpsToken,0);
    }

    // cope with alias instances ("v[2,1]=7" and the like)
    if (0 == dlp_strcmp(TT_VAR, idTsq->Sfetch(nTok, OF_TTYP)) \
    && ((CDlpObject*)lpWord->lpData)->m_iAliasInst \
    && 0 == dlp_strcmp(((CDlpObject*) lpWord->lpData)->m_iAliasInst->m_lpClassName, "data"))
      idTsq->Sstore(TT_DATA, nTok, OF_TTYP);
  }

  return O_K;
}

/**
 Translate the tagged tokenised formula into one string in
 reverse polish notation (RPN, also "postfix notation";
 no brackets, stack-type operations).
 Use a version of Dijkstra's stack based "shunting yard" algorithm,
 modified to allow for non-scalar operands, assignment and prefix operators,
 in order to achieve this.

 @param idTsq data table with tagged token sequence queue
 @param lpsOut pointer to memory area allocated for the output string
 @param nMaxLength size of the allocated memory in bytes
 @return success indicator
 @see Formula2RPN
*/
INT16 CGEN_PROTECTED CFunction::FormulaTranslate(CData* idTsq, char* lpsOut, INT16 nMaxLength)
{
  INT16 nTok;                                           // position of input token in queue
  INT16 nStackTok;                                      // position of stacked token in queue
  INT16 nErr = 0;                                       // procudural error status
  char* lpsTok = NULL;                                  // current input token
  char* lpsTokTag = NULL;                               // current input token's type tag
  char* lpsNextTok = NULL;                              // next input token (for look-ahead)
  char* lpsNextTokTag = NULL;                           // next input token's type tag (for look-ahead)
  // token stack: encapsulate stack functions
  class CStackUnderflow {};                             // stack exceptions...
  class CStackOverflow {};
  static const INT16 nStackMax = 1024;                  // token stack capacity
  /* stack data structure: encapsulate
     * operations (push, pop, peek)
     * checks (empty, full)
     * exceptions (underflow, overflow)
     uses:
     - TokStack (token stack for shunting yard algorithm)
     - ArgNumStack (track number of arguments for current parenthesis; distinguish store/fetch from xstore/xfetch)
     - ArgTypeStack (track type of arguments for current parenthesis; distinguish data from scalar functions) [currently not used]
  */
  class CStack                                          // token stack (type definition)
  {
    private:
      INT16 nPtr;
      INT16 lpanArray[nStackMax];

    public:
      CStack() : nPtr(-1) { }
      // store token position
      void Push(INT16 nItem)
      {
  if (nPtr > nStackMax - 2) throw CStackOverflow();
  lpanArray[++nPtr] = nItem;
      };
      // retrieve token position
      INT16 Pop()
      {
  if (nPtr < 0) throw CStackUnderflow();
  return lpanArray[nPtr--];
      };
      // read token position (do not change pointer)
      INT16 Peek()
      {
  if (nPtr < 0) throw CStackUnderflow();
  return lpanArray[nPtr];
      };
      // ...
      bool IsEmpty() { return (nPtr < 0); }
      bool IsFull() { return (nPtr >= nStackMax); }
  } lpTokStack, lpArgNumStack, lpArgTypeStack;          // token stack, and auxiliary stacks (instantiation)
  class COperatorPrecedence                             // operator precedence table (type definition)
  {
      public:
  static INT16 LeftRank(INT16 nOpcode)
  {
    switch(nOpcode)
    {
      // prefix ops
      case OP_NEG:  return 1;
      case OP_NOT:  return 1;
      // infix ops
      case OP_ADD:
      case OP_DIFF: return 5;
      case OP_MULT:
      case OP_MULT_EL:
      case OP_MULT_KRON:
      case OP_MOD:
      case OP_LDIV:
      case OP_DIV:  return 3;
      case OP_NEQUAL:
      case OP_EQUAL:
      case OP_GREATER:
      case OP_LESS:
      case OP_GEQ:
      case OP_LEQ:  return 7;
      case OP_AND:
      case OP_OR:   return 8;
      default:      return 1;
    }
  }
  static INT16 RightRank(INT16 nOpcode)
  {
    switch(nOpcode)
    {
      // prefix ops
      case OP_NEG:  return 0;
      case OP_NOT:  return 2;
      // infix ops
      case OP_ADD:
      case OP_DIFF: return 6;
      case OP_MULT:
      case OP_MULT_EL:
      case OP_MULT_KRON:
      case OP_MOD:
      case OP_LDIV:
      case OP_DIV:  return 5;
      case OP_NEQUAL:
      case OP_EQUAL:
      case OP_GREATER:
      case OP_LESS:
      case OP_GEQ:
      case OP_LEQ:  return 7;
      case OP_AND:
      case OP_OR:   return 8;
      default:      return 2;
    }
  }
  } lpOperatorPrecedence;                                // operator precedence table (instantiation)
  class CBracket                                         // bracket matching (type definition)
  {
    public:
      static bool Match(char* lpsOpen, char* lpsClose)
      {
  if (0 == dlp_strcmp(lpsOpen, "(") && 0 == dlp_strcmp(lpsClose, ")"))
    return true;
  else if (0 == dlp_strcmp(lpsOpen, "[") && 0 == dlp_strcmp(lpsClose, "]"))
    return true;
  else if (0 == dlp_strcmp(lpsOpen, "{") && 0 == dlp_strcmp(lpsClose, "}"))
    return true;
  else
    return false;
      }
  } lpBracket;                                           // bracket matching (instantiation)
  /* output (string) buffer:
     encapsulate these operations:
     * memory management (allocation, size checks)
     * left value caching
     * appendix of new characters
  */
  class COutOfMemory {};                                 // output buffer exceptions...
  class COutputBuffer                                    // output buffer (type definition)
  {
    private:
      char* lpsBuffer;                                   // output string
      UINT16 nMaxLength;                         // maximum length of the output string
      char  lpsLVBuffer[L_INPUTLINE];                    // left value buffer (temporary)

    public:
      COutputBuffer(UINT16 nLength)
      {
  lpsBuffer = (char*) malloc(nLength*sizeof(char));
  lpsBuffer[0]='\0';
  lpsLVBuffer[0]='\0';
  nMaxLength = nLength;
      };

      ~COutputBuffer()
      {
  free(lpsBuffer);
      };

      // append strings to current buffer
      void Append(const char* lpsTail1, const char* lpsTail2, const char* lpsTail3)
      {
  if (! (dlp_strlen(lpsBuffer) + dlp_strlen(lpsTail1) + dlp_strlen(lpsTail2) + dlp_strlen(lpsTail3) < nMaxLength))
    throw COutOfMemory();

  dlp_strcat(lpsBuffer, lpsTail1);
  dlp_strcat(lpsBuffer, lpsTail2);
  dlp_strcat(lpsBuffer, lpsTail3);
      };

      // move output gathered so far to separate temporary buffer
      void StoreLeftValue()
      {
  dlp_strcpy(lpsLVBuffer, lpsBuffer);
  dlp_strcpy(lpsBuffer, "");
      };

      // append temporarily saved code to final output
      void RestoreLeftValue()
      {
  if (! (dlp_strlen(lpsBuffer) + dlp_strlen(lpsLVBuffer) < nMaxLength))
    throw COutOfMemory();

  dlp_strcat(lpsBuffer, lpsLVBuffer);
      };

      // save final output to external memory
      void Finalise(char* lpsOut, UINT16 nLength)
      {
//  "if" not allowed interactively
//  Append(" ?error if \"\nSee field last_fml for RPN code causing the error.\" -echo endif", "", "");

  if (dlp_strlen(lpsBuffer) >= nLength)
    throw COutOfMemory();

  dlp_strcpy(lpsOut, lpsBuffer);
      };

      const char* GetCurrentLeftValue()
      {
  return lpsLVBuffer;
      };
      const char* GetCurrentRightValue()
      {
  return lpsBuffer;
      };
  } lpOutputBuffer(nMaxLength);                             // output buffer (instantiation)

  ///////////////////////////////////////////////////////////////
  //    shunting yard algorithm starts here
  ///////////////////////////////////////////////////////////////

  try
  {

    for (nTok = 0; nTok < idTsq->GetNRecs() && nErr == 0; nTok++)
    {
  lpsTok        = (char*) idTsq->XAddr(nTok, OF_TOK);
  lpsTokTag     = (char*) idTsq->XAddr(nTok, OF_TTYP);
  lpsNextTok    = (char*) idTsq->XAddr(nTok+1, OF_TOK);
  lpsNextTokTag = (char*) idTsq->XAddr(nTok+1, OF_TTYP);

  //
  // read-only literals: add to output
  //
  if      (0 == dlp_strcmp(lpsTokTag, TT_NUM))
    lpOutputBuffer.Append(" ", lpsTok, "");
  else if (0 == dlp_strcmp(lpsTokTag, TT_STR))
    lpOutputBuffer.Append(" \"", lpsTok, "\"");

  //
  // read-write literals: look-ahead, then add to output ('=': add setcode, '[': divert to stack)
  //
  else if (0 == dlp_strcmp(lpsTokTag, TT_DATA))
  {
    if (0 == dlp_strcmp(lpsNextTokTag,TT_OBR) \
      && 0 == dlp_strcmp(lpsNextTok,"["))
      lpTokStack.Push(nTok);
    else
    {
      lpOutputBuffer.Append(" ", lpsTok, "");

      if (0 == dlp_strcmp(lpsNextTokTag,TT_OPR) \
        && 0 == dlp_strcmp(lpsNextTok,"="))
        lpOutputBuffer.Append(" -copy", "", "");

      if (!lpArgTypeStack.IsEmpty()) // are we within brackets?
      {
        lpArgTypeStack.Pop();
        lpArgTypeStack.Push(1); // data argument, not only scalar
      }
    }
  }
  else if (0 == dlp_strcmp(lpsTokTag, TT_VAR))
  {
    lpOutputBuffer.Append(" ", lpsTok, "");

    if (0 == dlp_strcmp(lpsNextTokTag,TT_OPR) \
      && 0 == dlp_strcmp(lpsNextTok,"="))
      lpOutputBuffer.Append(" =", "", "");
  }
  else if (0 == dlp_strcmp(lpsTokTag, TT_WORD))
  {
    if (0 == dlp_strcmp(lpsNextTokTag,TT_OPR) \
      && 0 == dlp_strcmp(lpsNextTok,"="))
    {
      char lpsTok2[255];

      dlp_strncpy(lpsTok2, lpsTok, 255);
      dlp_strcpy(strrchr(lpsTok2, '.'), " -set ");         // get end of container, shove in method call
      dlp_strcat(lpsTok2, 1 + strrchr(lpsTok, '.'));       // get begin of last field, reappend postfix argument

      lpOutputBuffer.Append(" ", lpsTok2, "");
    }
    else
      lpOutputBuffer.Append(" ", lpsTok, "");
  }

  //
  // function tokens: push to stack
  //
  else if (0 == dlp_strcmp(lpsTokTag, TT_FUNC))
    lpTokStack.Push(nTok);

  //
  // opening bracket: push to stack
  //
  else if (0 == dlp_strcmp(lpsTokTag, TT_OBR))
  {
    lpTokStack.Push(nTok);

    if (0 == strcmp("[", lpsTok) && 0 != strcmp(TT_DATA, (char*) idTsq->XAddr(nTok-1, OF_TTYP)))
      nErr = IERROR(this,FNC_INTERNAL, "square brackets must follow data instance", "formula-RPN translator ", nTok);

    lpArgNumStack.Push(1);  // first argument up to next separator
    lpArgTypeStack.Push(0); // default: scalar
  }

  //
  // argument separator: pop off stack until last opening bracket
  //
  else if (0 == dlp_strcmp(lpsTokTag, TT_ASEP))
  {
    while (0 != dlp_strcmp(TT_OBR, (char*) idTsq->XAddr(nStackTok = lpTokStack.Peek(), OF_TTYP)) && nErr == 0)
    {
      lpTokStack.Pop();
      if (0 != dlp_strcmp(TT_OPR, (char*) idTsq->XAddr(nStackTok, OF_TTYP)))
        nErr = IERROR(this,FNC_INTERNAL, "functor not followed by brackets", "formula-RPN translator ", nStackTok);

      lpOutputBuffer.Append(" ", (char*) idTsq->XAddr(nStackTok, OF_TOK), "");
    }

    lpArgNumStack.Push(1+lpArgNumStack.Pop()); // another argument
  }

  //
  // closing bracket: pop off stack until last opening bracket, then pop off stack if functor token
  //
  else if (0 == dlp_strcmp(lpsTokTag, TT_CBR))
  {
    char* lpsStackTok = NULL;                // token stacked within current brackets
    char* lpsStackTokTag = NULL;             // that token's tag
      INT16 nArgNum = lpArgNumStack.Pop();     // number of arguments between current brackets
//    INT16 nArgType = lpArgTypeStack.Pop();   // type of arguments between current brackets

    do
    {
      nStackTok = lpTokStack.Pop();

      lpsStackTok = (char*) idTsq->XAddr(nStackTok, OF_TOK);
      lpsStackTokTag = (char*) idTsq->XAddr(nStackTok, OF_TTYP);

      if (0 == dlp_strcmp(TT_OBR, lpsStackTokTag))
      {
        if (! lpBracket.Match(lpsStackTok, lpsTok))
    nErr = IERROR(this,FNC_INTERNAL, "bracket mismatch", "formula-RPN translator ", nTok);
        break;
      }
      else
      {
        if (0 != dlp_strcmp(TT_OPR, lpsStackTokTag))
    nErr = IERROR(this,FNC_INTERNAL, "functor not followed by brackets", "formula-RPN translator ", nStackTok);

        lpOutputBuffer.Append(" ", lpsStackTok, "");
      }
    }
    while (nErr == 0);

    if (lpTokStack.IsEmpty()) nStackTok = -1;
    else nStackTok = lpTokStack.Peek();
    lpsStackTok = (char*) idTsq->XAddr(nStackTok, OF_TOK);
    if (0 == dlp_strcmp(TT_FUNC, (char*) idTsq->XAddr(nStackTok, OF_TTYP))) // scalop/strop/aggrop functor
    {
      INT16 nOpcode;

      lpTokStack.Pop();

      if ((nOpcode = dlp_scalop_code(lpsStackTok)) == -1)
        if ((nOpcode = dlm_matrop_code(lpsStackTok,NULL)) == -1)
          if ((nOpcode = dlp_strop_code(lpsStackTok)) == -1)
            if ((nOpcode = dlp_aggrop_code(lpsStackTok)) == -1)
              if ((nOpcode = dlp_sigop_code(lpsStackTok)) == -1)
                nErr = IERROR(this,FNC_INTERNAL, "use of unknown prefix function", "formula-RPN translator ", nStackTok);
              else
                lpOutputBuffer.Append(" ", lpsStackTok, ""); // have dlabpro interpreter calculate the signal operation
            else
              lpOutputBuffer.Append(" ", lpsStackTok, ""); // have dlabpro interpreter calculate the scalar aggrop
          else
            lpOutputBuffer.Append(" ", lpsStackTok, ""); // have dlabpro interpreter calculate the scalar strop
        else
          lpOutputBuffer.Append(" ", lpsStackTok, ""); // have dlabpro interpreter calculate the matrix operation
      else
        lpOutputBuffer.Append(" ", lpsStackTok, ""); // have dlabpro interpreter calculate the scalar scalop

    } // endif *op functor on top of stack

        if (0 == dlp_strcmp(TT_DATA, (char*) idTsq->XAddr(nStackTok, OF_TTYP))) // data fetch/store functor
    {
      lpTokStack.Pop();

      if (0 != dlp_strcmp(lpsTok, "]"))
        nErr = IERROR(this,FNC_INTERNAL, "invalid use of brackets after data instance", "formula-RPN translator ", nStackTok);
      if (0 == dlp_strcmp(lpsNextTokTag,TT_OPR) \
        && 0 == dlp_strcmp(lpsNextTok,"="))
        if (2 == nArgNum)      // "data store"
    lpOutputBuffer.Append(" ", lpsStackTok, " -store");
        else if (1 == nArgNum) // "data xstore"
    // insert 0 1 before component (last numeric result), then target instance, then operation
    lpOutputBuffer.Append(" 0 -swap 1 -swap ", lpsStackTok, " -xstore");
        else nErr = IERROR(this,FNC_INTERNAL, "improper use of square brackets after data instance", "formula-RPN translator ", nStackTok);
      else
        if (2 == nArgNum)      // "data fetch"
    lpOutputBuffer.Append(" ", lpsStackTok, " -fetch");
        else if (1 == nArgNum) // "data xfetch"
        {
    // append 1, then lpsStackTok (source instance), then operation (leaves temporary instance on stack)
    lpOutputBuffer.Append(" 1 ", lpsStackTok, " /comp -xfetch");

    if (!lpArgTypeStack.IsEmpty()) // are there any surrounding brackets?
    {
      lpArgTypeStack.Pop();
      lpArgTypeStack.Push(1); // result of xfetch is data argument, not only scalar
    }

        }
        else nErr = IERROR(this,FNC_INTERNAL, "improper use of square brackets after data instance", "formula-RPN translator ", nStackTok);
    } // endif data functor on top of stack

  } // endif closing bracket

  //
  // operators: pop off stack as long as stacked token has higher rank, then push to stack
  //
  else if (0 == dlp_strcmp(lpsTokTag, TT_OPR))
  {
    if (0 == dlp_strcmp(lpsTok, "="))
      // no additional steps needed here -- everything already solved via look-ahead
      lpOutputBuffer.StoreLeftValue();
    else
    {
        char* lpsLastTokTag = (char*) idTsq->XAddr(nTok - 1, OF_TTYP);

        // first, make sure operators are recognised correctly by dlabpro interpreter
        if (0 == dlp_strcmp(lpsTok, "-"))
    // look back: discern negation (monadic prefix op) from substraction (binary infix op)
    // '-' is negation iff it is the first token, or its preceding token
    //     (1) starts a term (opening bracket, argument separator, or '='), or
    //     (2) is another operator.
    if (0 == nTok \
     || 0 == dlp_strcmp(TT_OBR, lpsLastTokTag)  \
     || 0 == dlp_strcmp(TT_ASEP, lpsLastTokTag) \
     || 0 == dlp_strcmp(TT_OPR, lpsLastTokTag))
      idTsq->Sstore("neg", nTok, OF_TOK);
        if (0 == dlp_strcmp(lpsTok, "!"))
    idTsq->Sstore("not", nTok, OF_TOK);

        // second, compare precedence with stacked operators, if any
        while (! lpTokStack.IsEmpty() && 0 == dlp_strcmp(TT_OPR, (char*) idTsq->XAddr(nStackTok = lpTokStack.Peek(), OF_TTYP)) && nErr == 0)
        {
    char* lpsStackTok = (char*) idTsq->XAddr(nStackTok, OF_TOK);
    INT16 nOpcStackTok = dlp_scalop_code(lpsStackTok);
    INT16 nOpcTok      = dlp_scalop_code(lpsTok     );
    if (nOpcStackTok<0) nOpcStackTok = dlm_matrop_code(lpsStackTok,NULL);
    if (nOpcTok     <0) nOpcTok      = dlm_matrop_code(lpsTok     ,NULL);

    if (lpOperatorPrecedence.LeftRank(nOpcStackTok) < lpOperatorPrecedence.RightRank(nOpcTok))
    {
      lpTokStack.Pop();
      lpOutputBuffer.Append(" ", lpsStackTok, "");
    }
    else break;
        }

        // third, push new operator to stack
        lpTokStack.Push(nTok);

    } // endif operators
  }
  else DLPASSERT(0);                            // no previous syntax tagging

  FNC_MSG(3, "    processed token %d (%s): %s", nTok, lpsTok, lpOutputBuffer.GetCurrentRightValue(), 0, 0);
    } // endfor

    // pop remaining operators from stack
    while (!lpTokStack.IsEmpty() && nErr == 0)
    {
      nStackTok = lpTokStack.Pop();

      if (0 != dlp_strcmp(TT_OPR, (char*) idTsq->XAddr(nStackTok, OF_TTYP)))
  nErr = IERROR(this,FNC_INTERNAL, "functor not followed by brackets", "formula-RPN translator ", nStackTok);

      lpOutputBuffer.Append(" ", (char*) idTsq->XAddr(nStackTok, OF_TOK), "");
    }

    FNC_MSG(3, "    cleared remaining items on stack: %s", lpOutputBuffer.GetCurrentRightValue(), 0, 0, 0, 0);

    lpOutputBuffer.RestoreLeftValue();
    lpOutputBuffer.Finalise(lpsOut, nMaxLength);

  } // endtry
  catch(CStackOverflow& ex)
  {
    nErr = IERROR(this,FNC_INTERNAL, "stack overflow", "formula-RPN translator ", nTok);
  }
  catch(CStackUnderflow& ex)
  {
    nErr = IERROR(this,FNC_INTERNAL, "stack underflow", "formula-RPN translator ", nTok);
  }
  catch(COutOfMemory& ex)
  {
    nErr = IERROR(this,FNC_INTERNAL, "output buffer exceeded", "formula-RPN translator ", nTok);
  }

  if (nErr != 0)
  {
    FNC_MSG(2, "  - interim results:\n", 0, 0, 0, 0, 0);
    FNC_MSG(2, "  - > left value output: \"%s\"\n", lpOutputBuffer.GetCurrentLeftValue(), 0, 0, 0, 0);
    FNC_MSG(2, "  - > right value/default output: \"%s\"\n", lpOutputBuffer.GetCurrentRightValue(), 0, 0, 0, 0);

    dlp_strcpy(lpsOut, "");
    return NOT_EXEC;
  }
  else
    return O_K;
}

