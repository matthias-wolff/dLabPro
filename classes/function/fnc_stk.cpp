// dLabPro class CFunction (function)
// - Argument stack implementation
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
#include "dlp_dgen.h"

// -- Public stack API --

/**
 * Pushes a boolean value onto the stack.
 *
 * @param bVal
 *          The value to be pushed.
 */
void CGEN_PUBLIC CFunction::PushLogic(BOOL bVal)
{
  Push();
  m_aStack[0].nType = T_BOOL;
  m_aStack[0].val.b = bVal;
}

/**
 * Pushes a number onto the stack.
 *
 * @param nVal
 *          The value to be pushed.
 */
void CGEN_PUBLIC CFunction::PushNumber(COMPLEX64 nVal)
{
  Push();
  m_aStack[0].nType = T_COMPLEX;
  m_aStack[0].val.n = nVal;
}

/**
 * Pushes a string onto the stack.
 *
 * @param lpsVal
 *          The string to be pushed.
 */
void CGEN_PUBLIC CFunction::PushString(const char* lpsVal)
{
  Push();
  m_aStack[0].nType = T_STRING;
  m_aStack[0].val.s = (char*)dlp_malloc((dlp_strlen(lpsVal)+1)*sizeof(char));
  dlp_strcpy(m_aStack[0].val.s,lpsVal);
}

/**
 * Pushes an instance onto the stack.
 *
 * @param iVal
 *          Pointer to the instance to be pushed.
 */
void CGEN_PUBLIC CFunction::PushInstance(CDlpObject* iVal)
{
  if (!iVal || (iVal->m_nClStyle & CS_SECONDARY)==0)
  {
    Push();
    m_aStack[0].nType = T_INSTANCE;
    m_aStack[0].val.i = iVal;
  }
  if (!iVal || (iVal->m_nClStyle & CS_SECONDARY)==0)
  {
    m_bAiUsed = FALSE;
    m_iAi     = iVal;
  }
  else
  {
    m_bAiUsed = TRUE;
    m_iAi2    = iVal;
  }
}

/**
 * EXPERIMENTAL - Pushes a stack brake.
 */
void CGEN_PUBLIC CFunction::PushBrake()
{
  Push();
  m_aStack[0].nType = T_STKBRAKE;
}

/**
 * Returns the boolean value at position <code>nPos</code> of the stack. If the
 * type of the stack item is not <code>T_BOOL</code>, the method will perform
 * the following typecasts:
 * <table>
 *   <tr><th>Type                   </th><th>Cast                      </th></tr>
 *   <tr><td><code>T_DOUBLE</code>  </td><td>Value != 0.               </td></tr>
 *   <tr><td><code>T_INSTANCE</code></td><td>Value != <code>NULL</code></td></tr>
 *   <tr><td><code>T_STRING</code>  </td><td>Length of string &gt; 0   </td></tr>
 * </table>
 *
 * @param nPos
 *           The item to be retrieved (0 for stack top).
 * @param nArg
 *           The index of the method/function argument being retrieved (if
 *           applicable). The value is (only) used for displaying error
 *           messages.
 * @return The value of the stack item as boolean. If no such stack item exists,
 *         the method will return <code>FALSE</code>.
 * @see PopLogic
 * @see StackNumber
 * @see StackInstance
 * @see StackString
 */
BOOL CGEN_PUBLIC CFunction::StackLogic(INT16 nPos, INT16 nArg DEFAULT(0))
{
  StkItm* lpSi = StackGet(nPos);
  if (!lpSi) return FALSE;
  switch (lpSi->nType)
  {
    case T_BOOL    : { return lpSi->val.b;                            }
    case T_COMPLEX : { return (!CMPLX_EQUAL(lpSi->val.n,CMPLX(0.)));  }
    case T_INSTANCE: { return (lpSi->val.i!=NULL);                    }
    case T_STRING  : { return (dlp_strlen(lpSi->val.s)>0);            }
    case T_STKBRAKE: { return FALSE;                                  }
    default        : { DLPASSERT(FMSG("Unknown type of stack item")); }
  }
  return FALSE;
}

/**
 * Returns the numeric value at position <code>nPos</code> of the stack. If the
 * type of the stack item is not <code>T_DOUBLE</code>, the method will perform
 * the following typecasts:
 * <table>
 *   <tr><th>Type                   </th><th>Cast                        </th></tr>
 *   <tr><td><code>T_BOOL</code>    </td><td>0. or 1.                    </td></tr>
 *   <tr><td><code>T_INSTANCE</code></td><td><b>Type cast error</b>      </td></tr>
 *   <tr><td><code>T_STRING</code>  </td><td>String interpreted as number</td></tr>
 * </table>
 *
 * @param nPos
 *           The item to be retrieved (0 for stack top).
 * @param nArg
 *           The index of the method/function argument being retrieved (if
 *           applicable). The value is (only) used for displaying error
 *           messages.
 * @return The value of the stack item as double. If no such stack item exists
 *         or if it cannot be converted to a numeric value, the method will
 *         return <code>0.</code>.
 * @see PopNumber
 * @see StackLogic
 * @see StackInstance
 * @see StackString
 */
COMPLEX64 CGEN_PUBLIC CFunction::StackNumber(INT16 nPos, INT16 nArg DEFAULT(0))
{
  COMPLEX64 z     = CMPLX(0);
  StkItm*   lpSi  = StackGet(nPos);

  if (!lpSi) return CMPLX(0.);
  switch (lpSi->nType)
  {
    case T_BOOL    : { return CMPLX(lpSi->val.b?1.:0.); }
    case T_COMPLEX : { return lpSi->val.n; }
    case T_INSTANCE: {
      IERROR(this,FNC_TYPECAST,nArg,"instance","number");
      return CMPLX(0.); }
    case T_STRING  : {
      if (dlp_sscanc(lpSi->val.s, &z) == O_K) return z;
      IERROR(this,FNC_TYPECAST,nArg,"string","number");
      return CMPLX(0.); }
    case T_STKBRAKE: { return CMPLX(0.); }
    default        : DLPASSERT(FMSG("Unknown type of stack item"));
  }
  return CMPLX(0.);
}

/**
 * Returns the instance at position <code>nPos</code> of the stack. If the
 * type of the stack item is not <code>T_INSTANCE</code>, the method will perform
 * the following typecasts:
 * <table>
 *   <tr><th>Type                   </th><th>Cast                                     </th></tr>
 *   <tr><td><code>T_BOOL</code>    </td><td><b>Type cast error</b>                   </td></tr>
 *   <tr><td><code>T_DOUBLE</code>  </td><td><b>Type cast error</b>                   </td></tr>
 *   <tr><td><code>T_STRING</code>  </td><td>String interpreted as instance identifier</td></tr>
 * </table>
 *
 * @param nPos
 *           The item to be retrieved (0 for stack top).
 * @param nArg
 *           The index of the method/function argument being retrieved (if
 *           applicable). The value is (only) used for displaying error
 *           messages.
 * @return The value of the stack item as double. If no such stack item exists
 *         or if it cannot be converted to a numeric value, the method will
 *         return <code>0.</code>.
 * @see PopInstance
 * @see StackLogic
 * @see StackNumber
 * @see StackString
 */
CDlpObject* CGEN_PUBLIC CFunction::StackInstance(INT16 nPos, INT16 nArg DEFAULT(0))
{
  StkItm* lpSi = StackGet(nPos);
  if (!lpSi) return NULL;
  switch (lpSi->nType)
  {
    case T_BOOL:
      IERROR(this,FNC_TYPECAST,nArg,"logic","instance");
      return NULL;
    case T_DOUBLE:
    case T_COMPLEX:
      IERROR(this,FNC_TYPECAST,nArg,"number","instance");
      return NULL;
    case T_INSTANCE:
    {
      CHECK_IPTR(lpSi->val.i,0);                                                //     If invalid, make it NULL
      if (lpSi->val.i && !lpSi->val.i->m_lpContainer)                           //     Temporary instance on stack
      {                                                                         //     >>
        DLPASSERT(m_nStackInstPos>=0 && m_nStackInstPos<L_STACK);               //       Check tmp. instance cache ptr.
        if (m_lpasStackInst[m_nStackInstPos])                                   //       There's someone at my place...
        {                                                                       //       >>
          FNC_MSG(2,"  - Destroy temp. instance %s",                            //         You get a nice funeral speech
            m_lpasStackInst[m_nStackInstPos]->m_lpInstanceName,0,0,0,0);        //         | (on verbose level 2)
          IDESTROY(m_lpasStackInst[m_nStackInstPos]);                           //         And you're dead man!
        }                                                                       //       <<
        FNC_MSG(2,"  - Cache temp. instance %s",lpSi->val.i->m_lpInstanceName,  //       Protocol (verbose level 2)
          0,0,0,0);                                                             //       |
        m_lpasStackInst[m_nStackInstPos] = lpSi->val.i;                         //       'Caus this is my place
        m_nStackInstPos++;                                                      //       And the next one gets onther
        if (m_nStackInstPos>=L_STACK) m_nStackInstPos = 0;                      //       No more places -> start over
      }                                                                         //     <<
      return lpSi->val.i;
    }
    case T_STRING  :
    {
      CDlpObject* iVal = FindInstanceWord(lpSi->val.s,NULL);
      if (!iVal) IERROR(this,FNC_TYPECAST,nArg,"string","instance");
      return iVal;
    }
    case T_STKBRAKE: return NULL;
    default        : DLPASSERT(FMSG("Unknown type of stack item"));
  }
  return NULL;
}

/**
 * Returns the string value at position <code>nPos</code> of the stack. If the
 * type of the stack item is not <code>T_STRING</code>, the method will perform
 * the following typecasts:
 * <table>
 *   <tr><th>Type                   </th><th>Cast                       </th></tr>
 *   <tr><td><code>T_BOOL</code>    </td><td>"TRUE" or "FALSE"          </td></tr>
 *   <tr><td><code>T_COMPLEX</code> </td><td>Value represented as string</td></tr>
 *   <tr><td><code>T_INSTANCE</code></td><td>Instance identifier        </td></tr>
 * </table>
 * <h3>Note</h3>
 * <p>The return value is a pointer to the field {@link stack_str}. It contains
 * a <em>copy</em> of the string stored on the stack (i.e. it does not point to
 * the actual stack value). The buffer contents remains valid until the next
 * call to <code>StackString</code> or {@link PopString}. You need not care
 * about any memory management of this buffer.</p>
 *
 * @param nPos
 *           The item to be retrieved (0 for stack top).
 * @param nArg
 *           The index of the method/function argument being retrieved (if
 *           applicable). The value is (only) used for displaying error
 *           messages.
 * @return The value of the stack item as string. If no such stack item exists,
 *         the method will return an empty string.
 * @see PopString
 * @see StackLogic
 * @see StackNumber
 * @see StackInstance
 */
char* CGEN_PUBLIC CFunction::StackString(INT16 nPos, INT16 nArg DEFAULT(0))
{
  DLPASSERT(m_nStackStrPos>=0 && m_nStackStrPos<L_STACK);
  StkItm* lpSi = StackGet(nPos);
  if (!lpSi) return NULL;
  m_lpasStackStr[m_nStackStrPos] =
    (char*)dlp_realloc(m_lpasStackStr[m_nStackStrPos],L_NAMES+1,sizeof(char));
  dlp_strcpy(m_lpasStackStr[m_nStackStrPos],"");
  switch (lpSi->nType)
  {
    case T_BOOL:
      dlp_strcpy(m_lpasStackStr[m_nStackStrPos],lpSi->val.b?"TRUE":"FALSE");
      break;
    case T_COMPLEX:
      dlp_sprintc(m_lpasStackStr[m_nStackStrPos],lpSi->val.n,TRUE);
      dlp_strtrimleft(m_lpasStackStr[m_nStackStrPos]);
      if (dlp_strcmp(m_lpasStackStr[m_nStackStrPos],"-0")==0)                   // MinGW @ WinXP has a "-0" :((
        dlp_strcpy(m_lpasStackStr[m_nStackStrPos],"0");
      break;
    case T_INSTANCE:
      dlp_strcpy(m_lpasStackStr[m_nStackStrPos],
        lpSi->val.i?lpSi->val.i->m_lpInstanceName:"");
      break;
    case T_STRING:
      if ((dlp_strlen(lpSi->val.s)+1)*sizeof(char)>dlp_size(m_lpasStackStr[m_nStackStrPos]))
      {
        m_lpasStackStr[m_nStackStrPos] =
          (char*)dlp_realloc(m_lpasStackStr[m_nStackStrPos],
          dlp_strlen(lpSi->val.s)+1,sizeof(char));
      }
      dlp_strcpy(m_lpasStackStr[m_nStackStrPos],lpSi->val.s);
      break;
    case T_STKBRAKE: return NULL;
    default:
      DLPASSERT(FMSG("Unknown type of stack item"));
  }
  char* lpsRetval = m_lpasStackStr[m_nStackStrPos];
  m_nStackStrPos++;
  if (m_nStackStrPos>=L_STACK) m_nStackStrPos = 0;
  return lpsRetval;
}


/**
 * Pops the top stack item and returns it.
 *
 * @param nArg
 *           The index of the method/function argument being retrieved (if
 *           applicable). The value is (only) used for displaying error
 *           messages.
 * @return The value of the topmost stack item. If no such stack item
 *         exists, the method will return <code>NULL</code>.
 */
StkItm* CGEN_PUBLIC CFunction::PopAny(INT16 nArg, StkItm* lpSi) {
  StkItm* lpSiFnc = StackGet(0);
  if (!lpSi || !lpSiFnc) return NULL;
  dlp_memmove(lpSi,lpSiFnc,sizeof(StkItm));
  switch(lpSi->nType) {
  case T_BOOL    : lpSi->val.b = PopLogic   (nArg);break;
  case T_COMPLEX : lpSi->val.n = PopNumber  (nArg);break;
  case T_STRING  : lpSi->val.s = PopString  (nArg);break;
  case T_INSTANCE: lpSi->val.i = PopInstance(nArg);break;
  }
  return lpSi;
}

/**
 * Pops the top stack item and returns its logic value. For type conversion
 * rules see {@link StackLogic}.
 *
 * @param nArg
 *           The index of the method/function argument being retrieved (if
 *           applicable). The value is (only) used for displaying error
 *           messages.
 * @return The value of the topmost stack item as boolean. If no such stack item
 *         exists, the method will return <code>FALSE</code>.
 * @see StackLogic
 * @see PopNumber
 * @see PopInstance
 * @see PopString
 */
BOOL CGEN_PUBLIC CFunction::PopLogic(INT16 nArg DEFAULT(0))
{
  BOOL bRes = StackLogic(0,nArg);
  Pop();
  return bRes;
}

/**
 * Pops the top stack item and returns its numeric value. For type conversion
 * rules see {@link StackNumber}.
 *
 * @param nArg
 *           The index of the method/function argument being retrieved (if
 *           applicable). The value is (only) used for displaying error
 *           messages.
 * @return The value of the topmost stack item as number. If no such stack item
 *         exists, the method will return <code>0.</code>.
 * @see StackNumber
 * @see PopLogic
 * @see PopInstance
 * @see PopString
 */
COMPLEX64 CGEN_PUBLIC CFunction::PopNumber(INT16 nArg DEFAULT(0))
{
  COMPLEX64 nRes = StackNumber(0,nArg);
  Pop();
  return nRes;
}

/**
 * Pops the top stack item and returns a pointer to the denoted instance. For
 * type conversion rules see {@link StackInstance}.
 *
 * @param nArg
 *           The index of the method/function argument being retrieved (if
 *           applicable). The value is (only) used for displaying error
 *           messages.
 * @return The value of the topmost stack item as instance. If no such stack
 *         item exists, the method will return <code>NULL</code>.
 * @see StackInstance
 * @see PopLogic
 * @see PopNumber
 * @see PopString
 */
CDlpObject* CGEN_PUBLIC CFunction::PopInstance(INT16 nArg DEFAULT(0))
{
  CDlpObject* iRes = StackInstance(0,nArg);
  Pop();
  return iRes;
}

/**
 * <p>Pops the top stack item and returns its string representation. For type
 * conversion rules see {@link StackString}.</p>
 * <p>The return value is a pointer to the field {@link stack_str}. It contains
 * a <em>copy</em> of the string stored on the stack (i.e. it does not point to
 * the actual stack value). The buffer contents remains valid until the next
 * call to <code>StackString</code> or {@link PopString}. You need not care
 * about any memory management of this buffer.</p>
 *
 * @param nArg
 *           The index of the method/function argument being retrieved (if
 *           applicable). The value is (only) used for displaying error
 *           messages.
 * @return The value of the topmost stack item as string. If no such stack item
 *         exists, the method will return an empty string.
 * @see StackString
 * @see PopLogic
 * @see PopNumber
 * @see PopInstance
 */
char* CGEN_PUBLIC CFunction::PopString(INT16 nArg DEFAULT(0))
{
  char* lpsRes = StackString(0,nArg);
  Pop();
  return lpsRes;
}

/**
 * Prints the stack contents at stdout.
 */
void CGEN_PUBLIC CFunction::StackPrint()
{
  char sBuf[L_SSTR];

  if (m_nStackLen<=0)
  {
    printf("\n   %20s --: EMPTY","STACK DUMP");
    return;
  }

  for (INT32 i=0; i<m_nStackLen; i++)
  {
    printf("\n   %20s %02ld: ",i?"":"STACK DUMP",(long)i);
    switch (m_aStack[i].nType)
    {
      case T_BOOL:
        printf("B %s",m_aStack[i].val.b?"TRUE":"FALSE");
        break;
      case T_DOUBLE:
      case T_COMPLEX:
        dlp_sprintx(sBuf,(char*)&m_aStack[i].val.n,T_COMPLEX,TRUE);                   //     HACK: 64-bit compatible print
        printf("N %s",dlp_strtrimleft(sBuf));
        break;
      case T_STRING:
        printf("S \"%s\"",m_aStack[i].val.s ?
          dlp_strconvert(SC_ESCAPE,dlp_get_a_buffer(),m_aStack[i].val.s) :
          "(null)");
        break;
      case T_INSTANCE:
        printf("I %s",m_aStack[i].val.i
          ? (m_aStack[i].val.i->m_lpInstanceName
            ? m_aStack[i].val.i->m_lpInstanceName
            : "???")
          : "NULL");
        break;
      case T_STKBRAKE:
        printf("- -----");
        break;
      default:
        printf("?");
    }
  }
}

// -- Secret invocation context reflection API --

StkItm* CFunction_PopAny(CDlpObject* __this, INT16 nArg, StkItm* lpSi)
{
  DLPASSERT(__this);
  return (lpSi = ((CFunction*)__this)->PopAny(nArg, lpSi));
}

BOOL CFunction_PopLogic(CDlpObject* __this, INT16 nArg, INT16 nPos)
{
  DLPASSERT(__this);
  return ((CFunction*)__this)->PopLogic(nArg);
}

COMPLEX64 CFunction_PopNumber(CDlpObject* __this, INT16 nArg, INT16 nPos)
{
  DLPASSERT(__this);
  return ((CFunction*)__this)->PopNumber(nArg);
}

CDlpObject* CFunction_PopInstance(CDlpObject* __this, INT16 nArg, INT16 nPos)
{
  DLPASSERT(__this);
  return ((CFunction*)__this)->PopInstance(nArg);
}

char* CFunction_PopString(CDlpObject* __this, INT16 nArg, INT16 nPos)
{
  DLPASSERT(__this);
  return ((CFunction*)__this)->PopString(nArg);
}

void CFunction_PushLogic(CDlpObject* __this, BOOL bVal)
{
  DLPASSERT(__this);
  ((CFunction*)__this)->PushLogic(bVal);
}

void CFunction_PushNumber(CDlpObject* __this, COMPLEX64 nVal)
{
  DLPASSERT(__this);
  ((CFunction*)__this)->PushNumber(nVal);
}

void CFunction_PushInstance(CDlpObject* __this, CDlpObject* iVal)
{
  DLPASSERT(__this);
  ((CFunction*)__this)->PushInstance(iVal);
}

void CFunction_PushString(CDlpObject* __this, const char* lpsVal)
{
  DLPASSERT(__this);
  ((CFunction*)__this)->PushString(lpsVal);
}

const char* CFunction_GetNextToken(CDlpObject* __this, BOOL bSameLine)
{
  DLPASSERT(__this);
  return ((CFunction*)__this)->GetNextToken(bSameLine);
}

void CFunction_RefuseToken(CDlpObject* __this)
{
  DLPASSERT(__this);
  ((CFunction*)__this)->RefuseToken();
}

const char* CFunction_GetNextTokenDel(CDlpObject* __this)
{
  DLPASSERT(__this);
  return ((CFunction*)__this)->GetNextTokenDel();
}

// -- Protected stack API --

/**
 * Initializes the stack.
 */
void CGEN_PROTECTED CFunction::StackInit()
{
  StackDestroy();
  m_aStack           = (StkItm*     )dlp_calloc(L_STACK,sizeof(StkItm)     );
  m_lpasStackStr     = (char**      )dlp_calloc(L_STACK,sizeof(char*)      );
  m_lpasStackInst    = (CDlpObject**)dlp_calloc(L_STACK,sizeof(CDlpObject*));
  m_nStackLen        = 0;
  m_mic.iCaller      = this;
  m_mic.GetX         = &CFunction_PopAny;
  m_mic.GetB         = &CFunction_PopLogic;
  m_mic.GetN         = &CFunction_PopNumber;
  m_mic.GetI         = &CFunction_PopInstance;
  m_mic.GetS         = &CFunction_PopString;
  m_mic.PutB         = &CFunction_PushLogic;
  m_mic.PutN         = &CFunction_PushNumber;
  m_mic.PutI         = &CFunction_PushInstance;
  m_mic.PutS         = &CFunction_PushString;
  m_mic.NextToken    = &CFunction_GetNextToken;
  m_mic.RefuseToken  = &CFunction_RefuseToken;
  m_mic.NextTokenDel = &CFunction_GetNextTokenDel;
}

/**
 * Empties the temporary instance cache and destroying all temporary instances.
 */
void CGEN_PROTECTED CFunction::StackClearInst()
{
  INT32 i = 0;                                                                  // Loop counter
  if (!m_lpasStackInst) return;                                                 // No cache, no service
  for (i=0; i<L_STACK; i++)                                                     // Loop over cache entries
    if (m_lpasStackInst[i])                                                     //   Something in here?
    {                                                                           //   >>
      DLPASSERT(!m_lpasStackInst[i]->m_lpContainer);                            //     No registered instances here!
      FNC_MSG(2,"  - Destroy temp. instance %s",                                //     Protocol (verbose level 2)
        m_lpasStackInst[i]->m_lpInstanceName,0,0,0,0);                          //     |
      IDESTROY(m_lpasStackInst[i]);                                             //     Destroy temporary instance
    }                                                                           //   <<
}

/**
 * Returns the number of stack entries.
 */
INT32 CGEN_PUBLIC CFunction::StackGetLength()
{
  INT32 i;

  if (!m_aStack) return 0;
  for (i=0; i<m_nStackLen; i++) if (m_aStack[i].nType==T_STKBRAKE) break;
  return i;
}

/**
 * Clears the stack and frees memory associated with strings stored on stacks.
 *
 * @param bDestroy
 *          If <code>TRUE</code> all items will be removed from the stack. If
 *          <code>FALSE</code> (default value) only items down to the next stack
 *          brake (but not the brake itself, cf. {@link StackClearBrake}) will
 *          be removed.
 * @see StackClearBrake
 */
void CGEN_PROTECTED CFunction::StackClear(BOOL bDestroy DEFAULT(FALSE))
{
  INT32 i = 0;                                                                  // Loop counter

  if (m_aStack)                                                                 // Have a stack
    for (i=0; i<L_STACK; i++)                                                   //   Loop over stack items
    {                                                                           //   >>
      if (!bDestroy && m_aStack[i].nType==T_STKBRAKE) break;                    //     Disregard brakes on destruction
      if (m_aStack[i].nType==T_STRING) dlp_free(m_aStack[i].val.s);             //     Free strings
      if (m_aStack[i].nType==T_INSTANCE && m_aStack[i].val.i)                   //     Non-NULL instances ...
        if (!m_aStack[i].val.i->m_lpContainer)                                  //       ... without container ...
        {                                                                       //       >> ... are temporary
          FNC_MSG(2,"  - Destroy temp. instance %s",                            //         Protocol (verbose level 2)
            m_aStack[i].val.i->m_lpInstanceName,0,0,0,0);                       //         |
          IDESTROY(m_aStack[i].val.i);                                          //         Destroy temporary instance
        }                                                                       //       <<
      dlp_memset(&m_aStack[i],0,sizeof(StkItm));                                //     Clear stack item
      m_nStackLen--;                                                            //     Decrement stack length
    }                                                                           //   <<
    if (m_nStackLen<0) m_nStackLen=0;                                           //   Cannot happen

  StackClearInst();                                                             // Clear instance cache
  if (!bDestroy) return;                                                        // All done if not destroying stack

  if (m_lpasStackStr)                                                           // Have string cache
    for (i=0; i<L_STACK; i++)                                                   //   Loop over strings in cache
      dlp_free(m_lpasStackStr[i]);                                              //     Free memory
}

/**
 * EXPERIMENTAL - Clears the stack down to the next stack brake (inclusive).
 *
 * @see StackClear
 */
void CGEN_PROTECTED CFunction::StackClearBrake()
{
  for (; m_nStackLen; m_nStackLen--)
  {
    if (m_aStack[0].nType==T_STKBRAKE) { Pop(TRUE); return; }
    Pop();
  }
}

/**
 * Destroys the stack and frees all associated memory.
 */
void CGEN_PROTECTED CFunction::StackDestroy()
{
  StackClear(TRUE);
  dlp_free(m_aStack       );
  dlp_free(m_lpasStackStr );
  dlp_free(m_lpasStackInst);
  m_nStackLen = -1;
}

/**
 * Pushes the stack clearing the top item. The method moves all stack items one
 * position down and increases the stack size (field {@link stack_len}) by one.
 *
 * @see PushLogic
 * @see PushNumber
 * @see PushString
 * @see PushInstance
 * @see StackGet
 * @see Pop
 */
void CGEN_PROTECTED CFunction::Push()
{
  dlp_memmove(&m_aStack[1],&m_aStack[0],(L_STACK-1)*sizeof(StkItm));
  dlp_memset(&m_aStack[0],0,sizeof(StkItm));
  if (m_nStackLen>=L_STACK)
    IERROR(this,FNC_STACKOVERFLOW,0,0,0);
  else
    m_nStackLen++;
}

/**
 * Retrieves a stack item.
 *
 * @param nPos
 *           The item to be retrieved (0 for stack top).
 * @return Pointer to the specified stack item or <code>NULL</code> if no such
 *         item exists.
 * @see Push
 * @see Pop
 */
StkItm* CGEN_PUBLIC CFunction::StackGet(INT16 nPos)
{
  if (nPos <0 || nPos >= L_STACK) return NULL;
  if (nPos >= m_nStackLen) return NULL;
  return &m_aStack[nPos];
}

/**
 * Pops the top stack item. The method moves all stack items one position up and
 * decreases the stack size (field {@link stack_len}) by one.
 *
 * @param bClearBrake
 *          If <code>TRUE</code> the method will pop a stack brake from the
 *          stack top. If <code>FALSE</code> (default value) the method will
 *          cause a <code>FNC_STACKUNDERFLOW</code> error if the stack top is
 *          a stack brake.
 *
 * @see Push
 * @see StackGet
 */
void CGEN_PUBLIC CFunction::Pop(BOOL bClearBrake DEFAULT(FALSE))
{
  if (m_nStackLen<=0 || (!bClearBrake && m_aStack[0].nType==T_STKBRAKE))
  {
    IERROR(this,FNC_STACKUNDERFLOW,"","",0);
    return;
  }
  if (m_aStack[0].nType==T_STRING) dlp_free(m_aStack[0].val.s);
  dlp_memmove(&m_aStack[0],&m_aStack[1],(L_STACK-1)*sizeof(StkItm));
  m_nStackLen--;
}

/**
 * Exchanges the top two stack items. Stack size and the items' values/types are not changed.
 */
void CGEN_PROTECTED CFunction::StackSwap()
{
  StkItm lpTmp;
  if (m_nStackLen < 2)
  {
    IERROR(this,FNC_STACKUNDERFLOW,"","",0);
    return;
  }
  dlp_memmove(&lpTmp,&m_aStack[0],sizeof(StkItm));
  dlp_memmove(&m_aStack[0],&m_aStack[1],sizeof(StkItm));
  dlp_memmove(&m_aStack[1],&lpTmp,sizeof(StkItm));
}

/**
 * Duplicates the top stack item. Stack size is increased by 1. The item's value/type is not changed.
 */
void CGEN_PROTECTED CFunction::StackDup()
{
  if (m_nStackLen < 1)
  {
    IERROR(this,FNC_STACKUNDERFLOW,"","",0);
    return;
  }
  if(m_aStack[0].nType == T_INSTANCE) {
    CDlpObject* iInst = CDlpObject_Instantiate(NULL, m_aStack[0].val.i->m_lpClassName, m_aStack[0].val.i->m_lpInstanceName, this!=GetRoot());
    iInst->Copy(m_aStack[0].val.i);
    iInst->CopyAllOptions(m_aStack[0].val.i);
    PushInstance(iInst);
  } else {
    dlp_memmove(&m_aStack[1],&m_aStack[0],(L_STACK-1)*sizeof(StkItm));
    if (m_nStackLen>=L_STACK)
      IERROR(this,FNC_STACKOVERFLOW,0,0,0);
    else
      m_nStackLen++;
  }
}

/**
 * Rotates the stack upwards. Top becomes bottom, the rest is shifted up by 1. Stack size and the items' values/types are not changed.
 */
void CGEN_PROTECTED CFunction::StackRot()
{
  StkItm lpTmp;
  if (m_nStackLen < 1)
  {
    IERROR(this,FNC_STACKUNDERFLOW,"","",0);
    return;
  }
  dlp_memmove(&lpTmp,&m_aStack[0],sizeof(StkItm));
  dlp_memmove(&m_aStack[0],&m_aStack[1],(m_nStackLen-1)*sizeof(StkItm));
  dlp_memmove(&m_aStack[m_nStackLen-1],&lpTmp,sizeof(StkItm));
}
// EOF
