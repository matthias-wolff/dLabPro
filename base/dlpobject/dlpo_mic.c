/* dLabPro class CDlpObject (object)
 * - Method invocation context functions
 *
 * AUTHOR : Matthias Wolff
 * PACKAGE: dLabPro/base
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
#include "dlp_object.h"

/**
 * <p>Sets a new method invocation context. The method invocation context must
 * be set prior to invoking primary method invocation callback (PMIC) functions,
 * option changed callback (OCC) functions and field changed callback (FCC)
 * functions. The invocation context allows these callback functions to access
 * prefix and postfix arguments. For convenience, the function propagates the
 * method invocation context to all instance fields of this instance thus
 * allowing them to access the context as well.</p>
 *
 * <p>The method invocation context struct <code>SMic</code> stores a pointer to
 * the calling instance along with pointers to functions which obtain arguments
 * for method calls.</p>
 *
 * @param _this
 *          Pointer to this instance
 * @param lpMic
 *          Pointer to a method invocation context struct
 * @return The previous method invocation context
 * @see CDlpObject_MicGet
 * @see CDlpObject_MicGetB
 * @see CDlpObject_MicGetN
 * @see CDlpObject_MicGetI
 * @see CDlpObject_MicGetS
 * @see CDlpObject_MicPutB
 * @see CDlpObject_MicPutN
 * @see CDlpObject_MicPutI
 * @see CDlpObject_MicPutS
 * @see CDlpObject_MicNextToken
 * @see CDlpObject_MicRefuseToken
 */
const SMic* CDlpObject_MicSet(CDlpObject* _this, const SMic* lpMic)
{
  const SMic* lpRetval;

  CHECK_THIS_RV(NULL);
  lpRetval = _this->m_lpMic;
  _this->m_lpMic = lpMic;
  if (lpMic)
  {
    DLPASSERT(lpMic->iCaller    );
    DLPASSERT(lpMic->GetB       );
    DLPASSERT(lpMic->GetN       );
    DLPASSERT(lpMic->GetI       );
    DLPASSERT(lpMic->GetS       );
    DLPASSERT(lpMic->PutB       );
    DLPASSERT(lpMic->PutN       );
    DLPASSERT(lpMic->PutI       );
    DLPASSERT(lpMic->PutS       );
    DLPASSERT(lpMic->NextToken  );
    DLPASSERT(lpMic->RefuseToken);
  }
  return lpRetval;
}

/**
 * <p>Gets the current method invocation context. The method invocation context must
 * be set prior to invoking primary method invocation callback (PMIC) functions,
 * option changed callback (OCC) functions and field changed callback (FCC)
 * functions. The invocation context allows these callback functions to access
 * prefix and postfix arguments. For convenience, the function propagates the
 * method invocation context to all instance fields of this instance thus
 * allowing them to access the context as well.</p>
 *
 * <p>The method invocation context struct <code>SMic</code> stores a pointer to
 * the calling instance along with pointers to functions which obtain arguments
 * for method calls.</p>
 *
 * @param _this
 *          Pointer to this instance
 * @return The method invocation context. If this instance has no own invocation
 *         context the parent instances' invocation context will be returned
 * @see CDlpObject_MicSet
 * @see CDlpObject_MicGetB
 * @see CDlpObject_MicGetN
 * @see CDlpObject_MicGetI
 * @see CDlpObject_MicGetS
 * @see CDlpObject_MicPutB
 * @see CDlpObject_MicPutN
 * @see CDlpObject_MicPutI
 * @see CDlpObject_MicPutS
 * @see CDlpObject_MicNextToken
 * @see CDlpObject_MicRefuseToken
 */
const SMic* CDlpObject_MicGet(CDlpObject* _this)
{
  CHECK_THIS_RV(NULL);
  if (_this->m_lpMic) return _this->m_lpMic;
  return CDlpObject_MicGet(CDlpObject_GetParent(_this));
}

/**
 * Obtains an argument for invocating a method from the instances method
 * invocation context.
 *
 * @param _this
 *          Pointer to this instance
 * @param nArg
 *           The index of the method/function argument being retrieved . The
 *           value is (only) used for displaying error messages.
 * @param lpSi
 *           Pointer to a stack item struct to be filled with the argument.
 * @return <code>lpSi</code> or <code>NULL</code> in case of errors.
 *
 * @see CDlpObject_MicSet
 * @see CDlpObject_MicGetB
 * @see CDlpObject_MicGetN
 * @see CDlpObject_MicGetI
 * @see CDlpObject_MicGetS
 */
StkItm* CDlpObject_MicGetX(CDlpObject* _this, INT16 nArg, StkItm* lpSi)
{
  if (!CDlpObject_MicGet(_this)      ) return NULL;
  if (!CDlpObject_MicGet(_this)->GetX) return NULL;
  return CDlpObject_MicGet(_this)->GetX(CDlpObject_MicGet(_this)->iCaller,nArg,lpSi);
}

/**
 * Obtains a boolean argument for invocating a method from the instances
 * method invocation context.
 *
 * @param _this
 *          Pointer to this instance
 * @param nArg
 *           The index of the method/function argument being retrieved . The
 *           value is (only) used for displaying error messages.
 * @param nPos
 *          The index of the argument by type (for compatibility with dLabPro
 *          2.4)
 * @return The value for the argument. If no method invocation context is set or
 *         if there is no such argument, the function will return
 *         <code>FALSE</code>.
 *
 * @see CDlpObject_MicSet
 * @see CDlpObject_MicGetX
 * @see CDlpObject_MicGetN
 * @see CDlpObject_MicGetI
 * @see CDlpObject_MicGetS
 */
BOOL CDlpObject_MicGetB(CDlpObject* _this, INT16 nArg, INT16 nPos)
{
  if (!CDlpObject_MicGet(_this)      ) return FALSE;
  if (!CDlpObject_MicGet(_this)->GetB) return FALSE;
  return CDlpObject_MicGet(_this)->GetB(CDlpObject_MicGet(_this)->iCaller,nArg,nPos);
}

/**
 * Obtains a numeric argument for invocating a method from the instances
 * method invocation context.
 *
 * @param _this
 *          Pointer to this instance
 * @param nArg
 *           The index of the method/function argument being retrieved . The
 *           value is (only) used for displaying error messages.
 * @param nPos
 *          The index of the argument by type (for compatibility with dLabPro
 *          2.4)
 * @return The value for the argument. If no method invocation context is set or
 *         if there is no such argument, the function will return 0.
 *
 * @see CDlpObject_MicSet
 * @see CDlpObject_MicGetX
 * @see CDlpObject_MicGetB
 * @see CDlpObject_MicGetI
 * @see CDlpObject_MicGetS
 */
COMPLEX64 CDlpObject_MicGetN(CDlpObject* _this, INT16 nArg, INT16 nPos)
{
  if (!CDlpObject_MicGet(_this)      ) return CMPLX(0);
  if (!CDlpObject_MicGet(_this)->GetN) return CMPLX(0);
  return CDlpObject_MicGet(_this)->GetN(CDlpObject_MicGet(_this)->iCaller,nArg,nPos);
}

/**
 * Obtains an instance argument for invocating a method from the instances
 * method invocation context.
 *
 * @param _this
 *          Pointer to this instance
 * @param nArg
 *           The index of the method/function argument being retrieved . The
 *           value is (only) used for displaying error messages.
 * @param nPos
 *          The index of the argument by type (for compatibility with dLabPro
 *          2.4)
 * @return The value for the argument. If no method invocation context is set or
 *         if there is no such argument, the function will return
 *         <code>NULL</code>.
 *
 * @see CDlpObject_MicSet
 * @see CDlpObject_MicGetX
 * @see CDlpObject_MicGetB
 * @see CDlpObject_MicGetN
 * @see CDlpObject_MicGetS
 */
CDlpObject* CDlpObject_MicGetI(CDlpObject* _this, INT16 nArg, INT16 nPos)
{
  if (!CDlpObject_MicGet(_this)      ) return NULL;
  if (!CDlpObject_MicGet(_this)->GetI) return NULL;
  return CDlpObject_MicGet(_this)->GetI(CDlpObject_MicGet(_this)->iCaller,nArg,nPos);
}

/**
 * Obtains a string argument for invocating a method from the instances
 * method invocation context.
 *
 * @param _this
 *          Pointer to this instance
 * @param nArg
 *           The index of the method/function argument being retrieved . The
 *           value is (only) used for displaying error messages.
 * @param nPos
 *          The index of the argument by type (for compatibility with dLabPro
 *          2.4)
 * @return The value for the argument. If no method invocation context is set or
 *         if there is no such argument, the function will return
 *         <code>NULL</code>.
 *
 * @see CDlpObject_MicSet
 * @see CDlpObject_MicGetX
 * @see CDlpObject_MicGetB
 * @see CDlpObject_MicGetN
 * @see CDlpObject_MicGetI
 */
char* CDlpObject_MicGetS(CDlpObject* _this, INT16 nArg, INT16 nPos)
{
  if (!CDlpObject_MicGet(_this)      ) return NULL;
  if (!CDlpObject_MicGet(_this)->GetS) return NULL;
  return CDlpObject_MicGet(_this)->GetS(CDlpObject_MicGet(_this)->iCaller,nArg,nPos);
}

/**
 * Passes a (return) value to the invocation context. The function does
 * nothing if there was no method invocation context set through
 * {@link CDlpObject_MicSet}.
 *
 * @param _this
 *          Pointer to this instance
 * @param lpSi
 *          Pointer to a stack item struct containing the value
 * @see CDlpObject_MicSet
 * @see CDlpObject_MicPutB
 * @see CDlpObject_MicPutN
 * @see CDlpObject_MicPutS
 * @see CDlpObject_MicPutI
 */
void CDlpObject_MicPutX(CDlpObject* _this, StkItm* lpSi)
{
  if (!CDlpObject_MicGet(_this)      ) return;
  if (!CDlpObject_MicGet(_this)->GetS) return;
  if (!lpSi                ) return;
  switch (lpSi->nType)
  {
    case T_BOOL    : CDlpObject_MicPutB(_this,lpSi->val.b);   return;
    case T_DOUBLE  : CDlpObject_MicPutN(_this,lpSi->val.n);   return;
    case T_COMPLEX : CDlpObject_MicPutN(_this,lpSi->val.n);   return;
    case T_STRING  : CDlpObject_MicPutS(_this,lpSi->val.s);   return;
    case T_INSTANCE: CDlpObject_MicPutI(_this,lpSi->val.i);   return;
  }
}

/**
 * Passes a boolean (return) value to the invocation context. The function does
 * nothing if there was no method invocation context set through
 * {@link CDlpObject_MicSet}.
 *
 * @param _this
 *          Pointer to this instance
 * @param bVal
 *          Value to pass to invocation context
 * @see CDlpObject_MicSet
 * @see CDlpObject_MicPutX
 * @see CDlpObject_MicPutN
 * @see CDlpObject_MicPutS
 * @see CDlpObject_MicPutI
 */
void CDlpObject_MicPutB(CDlpObject* _this, BOOL bVal)
{
  if (!CDlpObject_MicGet(_this)      ) return;
  if (!CDlpObject_MicGet(_this)->PutB) return;
  CDlpObject_MicGet(_this)->PutB(CDlpObject_MicGet(_this)->iCaller,bVal);
}

/**
 * Passes a numeric (return) value to the invocation context. The function does
 * nothing if there was no method invocation context set through
 * {@link CDlpObject_MicSet}.
 *
 * @param _this
 *          Pointer to this instance
 * @param nVal
 *          Value to pass to invocation context
 * @see CDlpObject_MicSet
 * @see CDlpObject_MicPutX
 * @see CDlpObject_MicPutB
 * @see CDlpObject_MicPutS
 * @see CDlpObject_MicPutI
 */
void CDlpObject_MicPutN(CDlpObject* _this, COMPLEX64 nVal)
{
  if (!CDlpObject_MicGet(_this)      ) return;
  if (!CDlpObject_MicGet(_this)->PutN) return;
  CDlpObject_MicGet(_this)->PutN(CDlpObject_MicGet(_this)->iCaller,nVal);
}

/**
 * Passes an instance pointer (as retrun value) to the invocation context. The
 * function does nothing if there was no method invocation context set through
 * {@link CDlpObject_MicSet}.
 *
 * @param _this
 *          Pointer to this instance
 * @param iVal
 *          Value to pass to invocation context
 * @see CDlpObject_MicSet
 * @see CDlpObject_MicPutX
 * @see CDlpObject_MicPutB
 * @see CDlpObject_MicPutN
 * @see CDlpObject_MicPutS
 */
void CDlpObject_MicPutI(CDlpObject* _this, CDlpObject* iVal)
{
  if (!CDlpObject_MicGet(_this)      ) return;
  if (!CDlpObject_MicGet(_this)->PutI) return;
  CDlpObject_MicGet(_this)->PutI(CDlpObject_MicGet(_this)->iCaller,iVal);
}

/**
 * Passes a string (return) value to the invocation context. The function does
 * nothing if there was no method invocation context set through
 * {@link CDlpObject_MicSet}.
 *
 * @param _this
 *          Pointer to this instance
 * @param lpsVal
 *          Value to pass to invocation context
 * @see CDlpObject_MicSet
 * @see CDlpObject_MicPutX
 * @see CDlpObject_MicPutB
 * @see CDlpObject_MicPutN
 * @see CDlpObject_MicPutI
 */
void CDlpObject_MicPutS(CDlpObject* _this, const char* lpsVal)
{
  if (!CDlpObject_MicGet(_this)      ) return;
  if (!CDlpObject_MicGet(_this)->PutS) return;
  CDlpObject_MicGet(_this)->PutS(CDlpObject_MicGet(_this)->iCaller,lpsVal);
}

/**
 * Obtains the next postfix argument. This function may be called at any time
 * provided a method invocation context was set through
 * {@link CDlpObject_MicSet}.
 *
 * @param _this
 *          Pointer to this instance
 * @param bSameLine
 *          If <code>TRUE</code> only get tokens located in the current
 *          instruction
 * @return The value for the argument. If no method invocation context is set or
 *         if there are no more postfix arguments, the function will return
 *         <code>NULL</code>.
 * @see CDlpObject_MicSet
 * @see CDlpObject_MicRefuseToken
 * @see CDlpObject_MicCmdLine
 */
const char* CDlpObject_MicNextToken(CDlpObject* _this, BOOL bSameLine)
{
  if (!CDlpObject_MicGet(_this)           ) return NULL;
  if (!CDlpObject_MicGet(_this)->NextToken) return NULL;
  return CDlpObject_MicGet(_this)->NextToken(CDlpObject_MicGet(_this)->iCaller,bSameLine);
}

/**
 * Obtains the string of insignificant delimiter characters following the token
 * most recently retrieved by CDlpObject_GetNextToken. This function may be
 * called after any call to {@link CDlpObject_MicNextToken}.
 *
 * @param _this
 *          Pointer to this instance
 * @return Pointer to the delimiter string or <code>NULL</code> if no such
 *         string exists.
 * @see CDlpObject_MicSet
 * @see CDlpObject_MicRefuseToken
 * @see CDlpObject_MicCmdLine
 */
const char* CDlpObject_MicNextTokenDel(CDlpObject* _this)
{
  if (!CDlpObject_MicGet(_this)              ) return NULL;
  if (!CDlpObject_MicGet(_this)->NextTokenDel) return NULL;
  return CDlpObject_MicGet(_this)->NextTokenDel(CDlpObject_MicGet(_this)->iCaller);
}

/**
 * Retrieves the remaining source file line as postfix argument.
 * <h3>Note</h3>
 * <p>This function returns a pointer to a static buffer and is <em>not thread
 * safe</em>!</p>
 *
 * @param _this
 *          Pointer to this instance
 * @return Pointer to buffer containing the remaining line of the source file
 * @see CDlpObject_MicSet
 * @see CDlpObject_MicNextToken
 * @see CDlpObject_MicRefuseToken
 */
const char* CDlpObject_MicCmdLine(CDlpObject* _this)
{
  /* TODO: Implementation does not work properly with the CFunction method
   *       invocation context. Use implementation of CCgen::GetRestOfDefLine
   *       here after class CItp has finally been removed!*/
  static char __lpsCmdLine[L_INPUTLINE+1];
  const char* lpsToken = NULL;

  __lpsCmdLine[0]='\0';
  while ((lpsToken=CDlpObject_MicNextToken(_this,TRUE))!=NULL)
  {
    if (strlen(__lpsCmdLine)) strcat(__lpsCmdLine," ");
    strcat(__lpsCmdLine,lpsToken);
  }
  return __lpsCmdLine;
}

/**
 * Discards a postfix argument. This is undoing a call to
 * {@link CDlpObject_MicNextToken}. This function may be called at any time
 * provided a method invocation context was set through
 * {@link CDlpObject_MicSet}.
 *
 * @param _this
 *          Pointer to this instance
 * @see CDlpObject_MicSet
 * @see CDlpObject_MicNextToken
 * @see CDlpObject_MicCmdLine
 */
void CDlpObject_MicRefuseToken(CDlpObject* _this)
{
  if (!CDlpObject_MicGet(_this)             ) return;
  if (!CDlpObject_MicGet(_this)->RefuseToken) return;
  CDlpObject_MicGet(_this)->RefuseToken(CDlpObject_MicGet(_this)->iCaller);
}

/**
 * Finds a word in the instance, the instances method invocation context or in
 * the container instance. If (and only if) the <code>lpsId</code> is preceeded
 * by "root." the word is searched in the root of the instance tree.
 *
 * @param _this
 *          Pointer to this instance
 * @param lpsId
 *          Identifier of word to find
 * @return The pointer to the word or <code>NULL</code> if nuch such word was
 *         found.
 */
SWord* CDlpObject_MicFindWord(CDlpObject* _this, const char* lpsId)
{
  SWord*      lpWord = NULL;
  CDlpObject* iInst  = NULL;

  /* If lpsId is preceeded by "root." seek in root instance (only!) */
  if (dlp_strncmp(lpsId,"root.",5)==0)
    return
      CDlpObject_FindWord(CDlpObject_GetRoot(_this),&lpsId[5],WL_TYPE_DONTCARE);

  /* Find in this instance */
  lpWord = CDlpObject_FindWord(_this,lpsId,WL_TYPE_DONTCARE);
  if (lpWord) return lpWord;

  /* Find in caller */
  if (CDlpObject_MicGet(_this) && CDlpObject_MicGet(_this)->iCaller)
  {
    iInst =  CDlpObject_MicGet(_this)->iCaller;
    while (iInst && !lpWord)
    {
      lpWord = CDlpObject_FindWord(iInst,lpsId,WL_TYPE_DONTCARE);
      if (iInst->m_lpContainer) iInst = iInst->m_lpContainer->lpContainer;
      else iInst = NULL;
    }
    if (lpWord) return lpWord;
  }

  /* Find in container */
  if (_this->m_lpContainer)
  {
    iInst = _this->m_lpContainer->lpContainer;
    while (iInst && !lpWord)
    {
      lpWord = CDlpObject_FindWord(iInst,lpsId,WL_TYPE_DONTCARE);
      if (iInst->m_lpContainer) iInst = iInst->m_lpContainer->lpContainer;
      else iInst = NULL;
    }
    return lpWord;
  }

  return lpWord;
}

/* EOF */
