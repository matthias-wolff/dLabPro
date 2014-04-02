/* dLabPro class CDlpObject (object)
 * - Runtime class information functions
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
 * Adds a word to the dictionary.
 *
 * @param _this  This instance
 * @param lpWord The word to add
 * @param lpObs  The obsolete identifier of the word to add (or NULL)
 * @param ...    Only for field words: (constant) initialization value
 * @return The word added to the dictionary or NULL if an error occured
 */
SWord* CDlpObject_RegisterWord(CDlpObject* _this, const SWord* lpWord,...)
{
  char    lpBuf[2*L_NAMES];
  va_list ap;
  SWord*  lpNewWord;

  CHECK_THIS_RV(NULL);
  DEBUGMSG(DM_KERNEL,"Register Word (instance %s; id='%s'; Hashcode='%ld')",_this->m_lpInstanceName,lpWord->lpName,(long)lpWord->lpName);
#ifdef __NORTTI
  IERROR(_this,ERR_NOTSUPPORTED,"Register word in __NORTTI mode",0,0);
  DLPASSERT(FALSE);
  return NULL;
#endif

  /* Validation */
  if (!lpWord || !dlp_strlen(lpWord->lpName))
  {
    DLPASSERT(FALSE);  /* Word must not be NULL and an identifier must be given */
    IERROR(_this,ERR_REGWORD,0,0,0); return NULL;
  }

  /* Check if name exists in dictionary */
  if ((lpNewWord = CDlpObject_FindWord(_this,lpWord->lpName,WL_TYPE_DONTCARE))!=NULL)
  {
    /* If name already exists in dictionary and it's a method than unregister  */
    /* previously registered word. This allows overiding of methods in derived */
    /* classes but introduces the risk of multiple method names in classes.    */
    /* This should not happen since the register code for a class is generated */
    /* automaticallly.                                                         */
    if (lpNewWord->nWordType == WL_TYPE_METHOD)
    {
      CDlpObject_UnregisterWord(_this,lpNewWord,FALSE);
    }
    else
    {
      IERROR(_this,ERR_DOUBLEDEF,lpWord->lpName,0,0);
      return NULL;
    }
    lpNewWord = NULL;
  }

  /* Prepare new dictionary entry */
  strcpy(lpBuf,_this->m_lpInstanceName);strcat(lpBuf,".");strcat(lpBuf,lpWord->lpName);
  lpNewWord = (SWord*)__dlp_malloc(sizeof(SWord),__FILE__,__LINE__,_this->m_lpClassName,lpBuf);
  memmove(lpNewWord,lpWord,sizeof(SWord));
  lpNewWord->lpContainer = _this;

  /* WL_TYPE_FIELD */
  if (lpWord->nWordType == WL_TYPE_FIELD)
  {
    /* Done if PF_NONAUTOMATIC */
    /* if (nFlags & PF_NONAUTOMATIC) return TRUE; */

    /* The parameter was succesfully created so store pointer */
    /* to init-value and zero init                            */

    va_start(ap,lpWord);
    switch (lpWord->ex.fld.nType)
    {
    case T_BOOL    : {
                       BOOL c = (BOOL)va_arg(ap,int);
                       lpNewWord->ex.fld.lpInit.n = c;
                       if (lpNewWord->lpData) *(BOOL*)lpNewWord->lpData = 0;
                       break;}
    case T_UCHAR   : {
                       UINT8 c = (UINT8)va_arg(ap,int);
                       lpNewWord->ex.fld.lpInit.n = c;
                       if (lpNewWord->lpData) *(UINT8*)lpNewWord->lpData = 0U;
                       break;}
    case T_CHAR    : {
                       INT8 c = (INT8)va_arg(ap,int);
                       lpNewWord->ex.fld.lpInit.n = c;
                       if (lpNewWord->lpData) *(INT8*)lpNewWord->lpData = 0;
                       break;}
    case T_USHORT  : {
                       UINT16 i = (UINT16)va_arg(ap,int);
                       lpNewWord->ex.fld.lpInit.n = i;
                       if (lpNewWord->lpData) *(UINT16*)lpNewWord->lpData = 0U;
                       break;}
    case T_SHORT   : {
                       INT16 i = (INT16)va_arg(ap,int);
                       lpNewWord->ex.fld.lpInit.n = i;
                       if (lpNewWord->lpData) *(INT16*)lpNewWord->lpData = 0;
                       break;}
    case T_UINT    : {
                       UINT32 i = (UINT32)va_arg(ap,int);
                       lpNewWord->ex.fld.lpInit.n = i;
                       if (lpNewWord->lpData) *(UINT32*)lpNewWord->lpData = 0U;
                       break;}
    case T_INT     : {
                       INT32 i = (INT32)va_arg(ap,int);
                       lpNewWord->ex.fld.lpInit.n = i;
                       if (lpNewWord->lpData) *(INT32*)lpNewWord->lpData = 0;
                       break;}
    case T_ULONG   : {
                       UINT64 l = (UINT64)va_arg(ap,long);
                       lpNewWord->ex.fld.lpInit.n = l;
                       if (lpNewWord->lpData) *(UINT64*)lpNewWord->lpData = 0UL;
                       break;}
    case T_LONG    : {
                       INT64 l = (INT64)va_arg(ap,long);
                       lpNewWord->ex.fld.lpInit.n = l;
                       if (lpNewWord->lpData) *(INT64*)lpNewWord->lpData = 0L;
                       break;}
    case T_FLOAT   : {
                       FLOAT32 f = (FLOAT32)va_arg(ap,double);
                       lpNewWord->ex.fld.lpInit.c = CMPLX(f);
                       if (lpNewWord->lpData) *(FLOAT32*)lpNewWord->lpData = 0.F;
                       break;}
    case T_DOUBLE  : {
                       FLOAT64 d = (FLOAT64)va_arg(ap,double);
                       lpNewWord->ex.fld.lpInit.c = CMPLX(d);
                       if (lpNewWord->lpData) *(FLOAT64*)lpNewWord->lpData = 0.;
                       break;}
    case T_COMPLEX : {
                       COMPLEX64 z = (COMPLEX64)va_arg(ap,COMPLEX64);
                       lpNewWord->ex.fld.lpInit.c = z;
                       if (lpNewWord->lpData) *(COMPLEX64*)lpNewWord->lpData = CMPLX(0.);
                       break;}
    case T_PTR     : {
                       lpNewWord->ex.fld.lpInit.p = va_arg(ap,void*);
                       if (lpNewWord->lpData) *(void**)lpNewWord->lpData = NULL;
                       break;}
    case T_INSTANCE: {
                       lpNewWord->ex.fld.lpInit.p = va_arg(ap,void*);
                       if (lpNewWord->lpData) *(void**)lpNewWord->lpData = NULL;
                       break;}
    case T_STRING  :
    case T_TEXT    : {
                       lpNewWord->ex.fld.lpInit.s = va_arg(ap,char*);
                       if (lpNewWord->lpData) *(char**)lpNewWord->lpData = NULL;
                       break;}
    case T_IGNORE  : break; /* Basst scho... */
    default:
      if (lpWord->ex.fld.nType >0 && lpWord->ex.fld.nType <=256)
      {
        lpNewWord->ex.fld.lpInit.s = va_arg(ap,char*);
        dlp_memset((char*)lpNewWord->lpData,0,lpWord->ex.fld.nType);
      }
      else DLPASSERT(FALSE); /* Invalid type code */
    }
    va_end(ap);
  }

  /* Add word to dictionary */
  hash_alloc_insert(_this->m_lpDictionary,lpNewWord->lpName,lpNewWord);

  /* WL_TYPE_INSTANCE */
  if (lpWord->nWordType == WL_TYPE_INSTANCE)
  {
    if (lpWord->lpData) ((CDlpObject*)lpWord->lpData)->m_lpContainer = lpNewWord;
  }

  /* Register obsolete identifier */
  if (dlp_strlen(lpNewWord->lpObsname))
    hash_alloc_insert(_this->m_lpObsIds,lpNewWord->lpObsname,lpNewWord->lpName);

  return lpNewWord;
}

/**
 * Adds a operator to the dictionary.
 *
 * @param _this  This instance
 * @param lpWord The operator to add
 * @return The operator added to the dictionary or NULL if an error occured
 */
SWord* CDlpObject_RegisterOperator(CDlpObject* _this, const SWord* lpWord)
{
  char    lpBuf[2*L_NAMES];
  SWord*  lpNewWord;

  CHECK_THIS_RV(NULL);
  DEBUGMSG(DM_KERNEL,"Register operator (instance %s; id='%s'; Hashcode='%ld')",_this->m_lpInstanceName,lpWord->lpName,(long)lpWord->lpName);
#ifdef __NORTTI
  IERROR(_this,ERR_NOTSUPPORTED,"Register operator in __NORTTI mode",0,0);
  DLPASSERT(FALSE);
  return NULL;
#endif

  /* Validation */
  if (!lpWord || !dlp_strlen(lpWord->lpName))
  {
    DLPASSERT(FALSE);  /* Word must not be NULL and an identifier must be given */
    IERROR(_this,ERR_REGWORD,0,0,0); return NULL;
  }

  /* Check if name exists in dictionary */
  if ((lpNewWord = CDlpObject_FindOperator(_this,lpWord->lpName))!=NULL)
  {
    /* If name already exists in dictionary then unregister previously         */
    /* registered operators. This allows overriding of operations.             */
    CDlpObject_UnregisterOperator(_this,lpNewWord);
    lpNewWord = NULL;
  }

  /* Prepare new dictionary entry */
  strcpy(lpBuf,_this->m_lpInstanceName);strcat(lpBuf,".");strcat(lpBuf,lpWord->lpName);
  lpNewWord = (SWord*)__dlp_malloc(sizeof(SWord),__FILE__,__LINE__,_this->m_lpClassName,lpBuf);
  memmove(lpNewWord,lpWord,sizeof(SWord));
  lpNewWord->lpContainer = _this;

  /* Add operator to dictionary */
  hash_alloc_insert(_this->m_lpOpDict,lpNewWord->lpName,lpNewWord);

  return lpNewWord;
}

/**
 * Removes a word from the dictionary.
 *
 * @param _this            This instance
 * @param lpWord           The word to remove
 * @param bDeleteInstrance If TRUE, delete instance attached to word
 */
INT16 CDlpObject_UnregisterWord(CDlpObject* _this, SWord* lpWord, INT16 bDeleteInstance)
{
  CDlpObject* lpInst = NULL;
  hnode_t* hn;

  /* Validation */
  CHECK_THIS_RV(NOT_EXEC);
  if (!lpWord) return O_K;
#ifdef __NORTTI
  IERROR(_this,ERR_DANGEROUS,"CDlpObject_UnregisterWord","Nothing will be unregistered in __NORTTI mode.",0);
  DLPASSERT(FALSE);
  return ERR_DANGEROUS;
#endif
  if (!dlp_strlen(lpWord->lpName))
  {
    DLPASSERT(FALSE);  /* Word without identifier */
    return NOT_EXEC;
  }

  /* Look up and delete obsolete name */
  if (dlp_strlen(lpWord->lpObsname) &&
      (hn = hash_lookup(_this->m_lpObsIds,lpWord->lpObsname))!=NULL)
  {
    /* NOTE: Key and value are both pointers into the word structure, no need to free 'em */
    hash_scan_delfree(_this->m_lpObsIds,hn);
  }

  /* Look up and delete word */
  hn = hash_lookup(_this->m_lpDictionary,lpWord->lpName);
  if (!hn) return NOT_EXEC;

  DLPASSERT(lpWord == hnode_get(hn)); /* Identifier not unique */
  if (lpWord->nWordType == WL_TYPE_FIELD) CDlpObject_ResetField(_this,lpWord,TRUE);

  if (lpWord->nWordType == WL_TYPE_INSTANCE && bDeleteInstance)
  {
    lpInst = (CDlpObject*)lpWord->lpData;

#ifdef __cplusplus
    delete(lpInst);
    DLP_CHECK_MEMINTEGRITY
#else
    {
      /* save pointer to derived instance, since base instance will be destroyed */
      void* lpGhost=lpInst->m_lpDerivedInstance;
      /* destroy base instance */
      lpInst->Destructor(lpInst);
      /* destroy derived instance */
      dlp_free(lpGhost);
    }
#endif
    lpWord->lpData = NULL;
  }

  /* Remove node from hash and free node */
  /* NOTE: Key is pointer into word structure, no need to free it */
  hash_scan_delfree(_this->m_lpDictionary,hn);

  /* Free word */
  dlp_free(lpWord);

  return O_K;
}

/**
 * Empties the dictionary.
 *
 * @param _this This instance
 * @return O_K if successful, NOT_EXEC otherwise
 */
INT16 CDlpObject_UnregisterAllWords(CDlpObject* _this)
{
  hscan_t  hs;
  hnode_t* hn;
  INT16    nErr = O_K;
  INT16    nRet = O_K;
  SWord*   lpWord;

  CHECK_THIS_RV(NOT_EXEC);
  DEBUGMSG(DM_KERNEL," Unregister all words of instance '%s' !",_this->m_lpInstanceName,0,0);
#ifdef __NORTTI
  if(hash_isempty(_this->m_lpDictionary)) return O_K;
  IERROR(_this,ERR_DANGEROUS,"CDlpObject_UnregisterAllWords","Nothing will be unregistered in __NORTTI mode.",0);
  DLPASSERT(FALSE);
  return ERR_DANGEROUS;
#endif

  while (!hash_isempty(_this->m_lpDictionary))
  {
    hash_scan_begin(&hs,_this->m_lpDictionary);
    if ((hn = hash_scan_next(&hs))!=NULL)
    {
      lpWord = (SWord*)hnode_get(hn);
      nErr = CDlpObject_UnregisterWord(_this,lpWord,TRUE);
      if (NOK(nErr))
      {
        IERROR(_this,ERR_DESTROY,
          " Error unregistering word '%s' of instance '%s' !",
          lpWord->lpName,_this->m_lpInstanceName);
        nRet = NOT_EXEC;
      }
    }
  }

  return nRet;
}

/**
 * Removes a operator from the dictionary.
 *
 * @param _this            This instance
 * @param lpWord           The operator to remove
 */
INT16 CDlpObject_UnregisterOperator(CDlpObject* _this, SWord* lpWord)
{
  hnode_t* hn;

  /* Validation */
  CHECK_THIS_RV(NOT_EXEC);
  if (!lpWord) return O_K;
#ifdef __NORTTI
  IERROR(_this,ERR_DANGEROUS,"CDlpObject_UnregisterOperator","Nothing will be unregistered in __NORTTI mode.",0);
  DLPASSERT(FALSE);
  return ERR_DANGEROUS;
#endif
  if (!dlp_strlen(lpWord->lpName))
  {
    DLPASSERT(FALSE);  /* Operator without identifier */
    return NOT_EXEC;
  }

  /* Look up and delete operator */
  hn = hash_lookup(_this->m_lpOpDict,lpWord->lpName);
  if (!hn) return NOT_EXEC;

  DLPASSERT(lpWord == hnode_get(hn)); /* Identifier not unique */

  hash_scan_delfree(_this->m_lpOpDict,hn);
  dlp_free(lpWord);

  return O_K;
}

/**
 * Empties the operations dictionary.
 *
 * @param _this This instance
 * @return O_K if successful, NOT_EXEC otherwise
 */
INT16 CDlpObject_UnregisterAllOperators(CDlpObject* _this)
{
  hscan_t  hs;
  hnode_t* hn;
  INT16    nRet = O_K;
  SWord*   lpWord;

  CHECK_THIS_RV(NOT_EXEC);
  DEBUGMSG(DM_KERNEL," Unregister all operators of instance '%s' !",_this->m_lpInstanceName,0,0);
#ifdef __NORTTI
  if(hash_isempty(_this->m_lpDictionary)) return O_K;
  IERROR(_this,ERR_DANGEROUS,"CDlpObject_UnregisterAllOperators","Nothing will be unregistered in __NORTTI mode.",0);
  DLPASSERT(FALSE);
  return ERR_DANGEROUS;
#endif

  while (!hash_isempty(_this->m_lpOpDict))
  {
    hash_scan_begin(&hs,_this->m_lpOpDict);
    if ((hn = hash_scan_next(&hs))!=NULL)
    {
      lpWord = (SWord*)hnode_get(hn);
      hash_scan_delfree(_this->m_lpOpDict,hn);
      dlp_free(lpWord);
    }
  }

  return nRet;
}

/**
 * INTERNAL USE ONLY, use CDlpObject_FindWord instead.
 * Finds a word in the dictionary by its indentifier. Does NOT handle qualified
 * identifiers.
 *
 * @param _this        This instance
 * @param lpIdentifier Identifier of word to search
 * @param nMask        Word types to search. Any combination of WL_TYPE_XXX
 *                     codes. Default is WL_TYPE_DONTCARE.
 * @return A pointer to the word or NULL if identifier was not found.
 * @see CDlpObject_FindWord
 */
SWord* CDlpObject_FindWordInternal(CDlpObject* _this, const char* lpIdentifier, INT16 nMask)
{
  hnode_t* hn;
  char*    lpId2;

  CHECK_THIS_RV(NULL);
#ifdef __NORTTI
  IERROR(_this,ERR_NOTSUPPORTED,"Finding a word in the dictionary in __NORTTI mode",0,0);
  DLPASSERT(FALSE);
  return NULL;
#endif

  if ((hn = hash_lookup(_this->m_lpDictionary,lpIdentifier))!=NULL)
  {
    DLPASSERT(hnode_get(hn));
    return (nMask&((SWord*)hnode_get(hn))->nWordType) ? (SWord*)hnode_get(hn) : NULL;
  }
  else
  {
    /* Not found, check obsolete names */
    if ((hn = hash_lookup(_this->m_lpObsIds,lpIdentifier))!=NULL)
    {
      DLPASSERT(hnode_get(hn));
      lpId2 = (char*)hnode_get(hn);
      hn = hash_lookup(_this->m_lpDictionary,lpId2);
      DLPASSERT(hn); /* Found obsolete identifier, but not the new one */
      return (nMask&((SWord*)hnode_get(hn))->nWordType) ? (SWord*)hnode_get(hn) : NULL;
    }
  }

  return NULL;
}

/**
 * Finds a word in the dictionary by its (qualified) indentifier.
 *
 * @param _this        This instance
 * @param lpIdentifier Identifier of word to search
 * @param nMask        Word types to search. Any combination of WL_TYPE_XXX
 *                     codes. Default is WL_TYPE_DONTCARE.
 * @return A pointer to the word or NULL if identifier was not found.
 */
SWord* CDlpObject_FindWord(CDlpObject* _this, const char* lpIdentifier, INT16 nMask)
{
  char        lpPart1[L_INPUTLINE];
  const char* lpPart2;
  SWord*      lpWord;

  /* Validation */
  CHECK_THIS_RV(NULL);
  if (!dlp_strlen(lpIdentifier)) return NULL;
#ifdef __NORTTI
  IERROR(_this,ERR_NOTSUPPORTED,"Finding a word in the dictionary in __NORTTI mode",0,0);
  DLPASSERT(FALSE);
  return NULL;
#endif


  /* Delegate to root? */
  if (dlp_strncmp(lpIdentifier,"root.",5)==0) lpIdentifier = &lpIdentifier[4];
  if (lpIdentifier[0]=='.' && CDlpObject_GetRoot(_this))
    return CDlpObject_FindWord(CDlpObject_GetRoot(_this),&lpIdentifier[1],nMask);

  /* Extract first part of qualified name */
  memset(lpPart1,0,L_NAMES);
  lpPart2 = lpIdentifier;
  while (*lpPart2 == '.') lpPart2++;
  while (*lpPart2 && *lpPart2 != '.') lpPart2++;
  dlp_strncpy(lpPart1,lpIdentifier,lpPart2-lpIdentifier);

  /* Look up first part of qualified name */
  lpWord = CDlpObject_FindWordInternal(_this,lpPart1,nMask);
  if (!lpWord && _this->m_iAliasInst)
    lpWord = CDlpObject_FindWordInternal(_this->m_iAliasInst,lpPart1,nMask);
  if (!lpWord) return NULL;

  /* Further qualifications? */
  if (!*lpPart2++) return lpWord;

  /* Is qualification legal? */
  if (lpWord->nWordType == WL_TYPE_INSTANCE)
  {
    if (!lpWord->lpData)
    {
      IERROR(_this,ERR_NULLINST,lpPart1,0,0);
      return NULL;
    }
    return CDlpObject_FindWord((CDlpObject*)lpWord->lpData,lpPart2,WL_TYPE_DONTCARE);
  }
  if (lpWord->nWordType == WL_TYPE_FIELD && lpWord->ex.fld.nType == T_INSTANCE)
  {
    if (!lpWord->lpData || !*(void**)lpWord->lpData)
    {
       IERROR(_this,ERR_NULLINST,lpPart1,0,0);
       return NULL;
    }
    return CDlpObject_FindWord((CDlpObject*)*(void**)lpWord->lpData,lpPart2,WL_TYPE_DONTCARE);
  }

  /* No, qualification is illegal for this type of word */
  IERROR(_this,ERR_ILLEGALQUALI,lpPart2,lpPart1,0);
  return NULL;
}

/**
 * Finds an instance by identifier.
 *
 * @param _this        This instance
 * @param lpIdentifier Identifier of word to search
 * @param lpClassName  Only return instances of this class (NULL for any class)
 * @return A pointer to the instance or NULL if not found
 */
CDlpObject* CDlpObject_FindInstanceWord(CDlpObject* _this, const char* lpIdentifier, const char* lpClassName)
{
  SWord*      lpWord = NULL;
  CDlpObject* iInst  = NULL;

  CHECK_THIS_RV(NULL);

  lpWord = CDlpObject_FindWord(_this,lpIdentifier,WL_TYPE_DONTCARE);
  if (!lpWord) return NULL;

  if (lpWord->nWordType == WL_TYPE_INSTANCE) iInst=(CDlpObject*)lpWord->lpData;
  if (lpWord->nWordType == WL_TYPE_FIELD && lpWord->ex.fld.nType == T_INSTANCE) iInst=*(CDlpObject**)lpWord->lpData;
  if (!iInst) return NULL;
  if (!dlp_strlen(lpClassName)) return iInst;
  return CDlpObject_OfKind(lpClassName,iInst);
}

/**
 * Finds a class factory by identifier.
 *
 * @param _this        This instance
 * @param lpIdentifier Identifier of class factory to search
 * @return A pointer to the class factory or NULL if not found
 */
SWord* CDlpObject_FindFactoryWord(CDlpObject* _this, const char* lpIdentifier)
{
  SWord* lpWord;

  CHECK_THIS_RV(NULL);

  lpWord = CDlpObject_FindWord(_this,lpIdentifier,WL_TYPE_DONTCARE);
  if (!lpWord) return NULL;

  if (lpWord->nWordType == WL_TYPE_FACTORY) return lpWord;

  return NULL;
}

/**
 * Finds a field word through its data pointer.
 *
 * <h3>Note</h3>
 * <p>This function is very complex and should be used rarely.</p>
 *
 * @param _this  This instance
 * @param lpData Pointer to search
 * @param bRecursive Descend instance fields
 * @return The field word or NULL if not found
 */
SWord* CDlpObject_FindFieldPtr(CDlpObject* _this, const void* lpData, BOOL bRecursive)
{
  hscan_t  hs;
  hnode_t* hn;
  SWord*   lpWord;

  CHECK_THIS_RV(NULL);

  if (!lpData) return NULL;
#ifdef __NORTTI
  IERROR(_this,ERR_NOTSUPPORTED,"Finding a field word in the dictionary in __NORTTI mode",0,0);
  DLPASSERT(FALSE);
  return NULL;
#endif

  hash_scan_begin(&hs,_this->m_lpDictionary);
  while ((hn = hash_scan_next(&hs))!=NULL)
  {
    lpWord = (SWord*)hnode_get(hn);
    DLPASSERT(lpWord); /* NULL pointer as value in dictionary */

    if (lpWord->nWordType == WL_TYPE_FIELD)
    {
      if (lpData == lpWord->lpData) return lpWord;
      if (lpWord->ex.fld.nType == T_INSTANCE && bRecursive)
      {
        /* Recursively look into instance fields */
        lpWord = CDlpObject_FindFieldPtr((CDlpObject*)*(void**)lpWord->lpData,lpData,TRUE);
        if (lpWord) return lpWord;
      }
    }
  }

  return NULL;
}

/**
 * Finds an operator in the dictionary by its (qualified) indentifier.
 *
 * @param _this        This instance
 * @param lpIdentifier Identifier of word to search
 * @return A pointer to the word or NULL if identifier was not found.
 */
SWord* CDlpObject_FindOperator(CDlpObject* _this, const char* lpIdentifier)
{
  hnode_t* hn;

  CHECK_THIS_RV(NULL);
#ifdef __NORTTI
  IERROR(_this,ERR_NOTSUPPORTED,"Finding a operator in the dictionary in __NORTTI mode",0,0);
  DLPASSERT(FALSE);
  return NULL;
#endif

  if ((hn = hash_lookup(_this->m_lpOpDict,lpIdentifier))!=NULL)
  {
    return (SWord*)hnode_get(hn);
  }

  return NULL;
}

/**
 * Creates and instance of an arbitrary class and registers it with this instance
 *
 * @param _this           This instance
 * @param lpsClassName    The identifier of the new instances class
 * @param lpsInstanceName The new instances identifier
 * @param bGlobal         If TRUE, the instance will be visible from functions nested within this instance
 */
CDlpObject* CDlpObject_Instantiate(CDlpObject* _this, const char* lpsClassName, const char* lpsInstanceName, BOOL bGlobal)
{
  CDlpObject* iInst = NULL;                                                     /* The newly create instane          */
  SWord iwrd;                                                                   /* An SWord struct                   */

  if (!(iInst = CDlpObject_CreateInstanceOf(lpsClassName,lpsInstanceName)))     /* Create instance, if failed        */
    return NULL;                                                                /* Bad enough                        */
#ifndef __cplusplus
  if(strncmp(lpsClassName,"DlpObject",10)) iInst=((CDlpObject**)iInst)[0];      /* Switch to base inst               */
#endif
  memset(&iwrd,0,sizeof(SWord));                                                /* Zero-init SWord struct            */
  iwrd.nWordType = WL_TYPE_INSTANCE;                                            /* It an ... uuuhm ... instance!     */
  iwrd.lpData    = iInst;                                                       /* Store instance pointer            */
  strcpy(iwrd.lpName,iInst->m_lpInstanceName);                                  /* Name word                         */
  CDlpObject_RegisterWord(_this,&iwrd);                                         /* Register with this instance       */
  if (bGlobal) iInst->m_nInStyle |= IS_GLOBAL;                                  /* Control visibility                */
  return iInst;
}

/*
 * Compares two dictionary words by name.
 * This function is intended to use with qsort.
 */
int CGEN_PUBLIC CDlpObject_CompareWordsByName(const void* lpWord1, const void* lpWord2) {
  return (int)dlp_strcmp((*(SWord**)lpWord1)->lpName,(*(SWord**)lpWord2)->lpName);
}

/* EOF */
