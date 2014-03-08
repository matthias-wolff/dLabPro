/* dLabPro class CDlpObject (object)
 * - Overridable functions
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
#include "dlp_config.h"

/* Disable some MSVC warnings */
#ifdef __MSVC
  #pragma warning( disable : 4100 ) /* Unreferenzierter formaler Parameter */
#endif

/**
 * Base class implementation of constructor.
 *
 * @param __this          This instance
 * @param lpsInstanceName Pointer to character buffer containing the instance name
 * @param bCallVirtual    For derived classes: if TRUE call base class implementation
 */
void CDlpObject_Constructor(CDlpObject* _this, const char* lpsInstanceName, BOOL bCallVirtual)
{
  DEBUGMSG(-1,"CDlpObject_Constructor for '%s' (bCallVirtual=%d)",lpsInstanceName,bCallVirtual,0);

  /* Register instance */
  dlp_xalloc_register_object('I',_this,1,sizeof(CDlpObject),__FILE__,__LINE__,
    "object",lpsInstanceName);

  /* Zero-initialize instance */
  _this->m_lpClassName[0]    = 0;
  _this->m_lpObsoleteName[0] = 0;
  _this->m_lpProjectName[0]  = 0;
  _this->m_lpInstanceName[0] = 0;
  _this->m_nClStyle          = 0;
  _this->m_nInStyle          = 0;
  _this->m_version.no[0]     = 0;
  _this->m_version.date[0]   = 0;
  _this->m_nCheck            = ML_SILENT;
  _this->m_nRC               = 0;                                               /* REMOVE TOGETHER WITH CItp         */
  _this->m_nSerialNum        = CDlpObject_GetNextSerialNum();
  _this->m_lpContainer       = NULL;
  _this->m_iAliasInst        = NULL;
  _this->m_lpMic             = NULL;
  dlp_strcpy(_this->m_lpInstanceName,lpsInstanceName);
  dlp_strcpy(_this->m_lpClassName,"object");

  /* Create dictionaries */
  _this->m_lpDictionary = hash_create(HASHCOUNT_T_MAX,0,0,NULL);
  _this->m_lpObsIds     = hash_create(HASHCOUNT_T_MAX,0,0,NULL);
  _this->m_lpOpDict     = hash_create(HASHCOUNT_T_MAX,0,0,NULL);

#ifndef __cplusplus

  /* Initialize pointers to virtual member functions              */
  /* NOTE: All derived classes initialize these fields with their */
  /*       own implementations of the virtual functions           */
  _this->AutoRegisterWords = CDlpObject_AutoRegisterWords;
  _this->Reset             = CDlpObject_Reset;
  _this->Init              = CDlpObject_Init;
  _this->Serialize         = CDlpObject_Serialize;
  _this->SerializeXml      = CDlpObject_SerializeXml;
  _this->Deserialize       = CDlpObject_Deserialize;
  _this->DeserializeXml    = CDlpObject_DeserializeXml;
  _this->Copy              = CDlpObject_Copy;
  _this->ClassProc         = CDlpObject_ClassProc;
  _this->GetInstanceInfo   = CDlpObject_GetInstanceInfo;
  _this->IsKindOf          = CDlpObject_IsKindOf;
  _this->Destructor        = CDlpObject_Destructor;
  _this->ResetAllOptions   = CDlpObject_ResetAllOptions;

  /* Initialize pointers to base/derived instance                   */
  /* NOTE: Derived classes declare and define an own pointer to the */
  /*       base instance. However, they override the definition of  */
  /*       the pointer to the derived instance.                     */
  _this->m_lpBaseInstance    = _this;
  _this->m_lpDerivedInstance = NULL;

#endif /* #ifndef __cplusplus */

  if (bCallVirtual)
  {
    DLPASSERT(OK(CDlpObject_AutoRegisterWords(_this)));
    CDlpObject_Init(_this,TRUE);
  }
}

/**
 * Base class implementation of destructor.
 *
 * @param _this This instance
 */
void CDlpObject_Destructor(CDlpObject* _this)
{
  SWord*   lpWord;
  hnode_t* hn;
  hscan_t  hs;

  DEBUGMSG(-1,"CDlpObject_Destructor for '%s'",_this->m_lpInstanceName,0,0);

  /* Detach external instances */
  hash_scan_begin(&hs,_this->m_lpDictionary);
  while ((hn = hash_scan_next(&hs))!=NULL)
  {
    DLPASSERT((lpWord = (SWord*)hnode_get(hn))!=NULL); /* NULL entry in dictionary */

    if
    (
      lpWord->nWordType == WL_TYPE_FIELD &&
      lpWord->ex.fld.nType == T_INSTANCE
    )
    {
      CHECK_IPTR(*(CDlpObject**)lpWord->lpData,lpWord->ex.fld.nISerialNum);
      if (CDlpObject_GetParent(*(CDlpObject**)lpWord->lpData)!=_this)
        *(CDlpObject**)lpWord->lpData = NULL;
    }
  }

#ifndef __NORTTI
  /* Clear words */
  if(NOK(CDlpObject_UnregisterAllWords(_this)) )
    IERROR(_this,ERR_DESTROY," Error while UnregisterAllWords of instance %s !",
      _this->m_lpInstanceName,0);
  /* Clear operators */
  if(NOK(CDlpObject_UnregisterAllOperators(_this)) )
    IERROR(_this,ERR_DESTROY," Error while UnregisterAllOperators of instance %s !",
      _this->m_lpInstanceName,0);
#endif

  /* Destroy dictionaries */
  if(!hash_isempty(_this->m_lpObsIds))
    IERROR(_this,ERR_DESTROY,
      " Destructor of instance '%s': hash 'ObsIds' not empty !",
      _this->m_lpInstanceName,0);
  if(!hash_isempty(_this->m_lpDictionary))
    IERROR(_this,ERR_DESTROY,
      " Destructor of instance '%s': hash 'Dictionary' not empty !",
      _this->m_lpInstanceName,0);
  if(!hash_isempty(_this->m_lpOpDict))
    IERROR(_this,ERR_DESTROY,
      " Destructor of instance '%s': hash 'OpDict' not empty !",
      _this->m_lpInstanceName,0);

  hash_destroy(_this->m_lpObsIds    );
  hash_destroy(_this->m_lpDictionary);
  hash_destroy(_this->m_lpOpDict);

#ifdef __cplusplus
  /* Unregister */
  dlp_xalloc_unregister_object(_this);
#endif
}

/**
 * Base class implementation of one-time dictionary initialization
 *
 * @param _this This instance
 * @return O_K if successful, an error code otherwise
 */
INT16 CDlpObject_AutoRegisterWords(CDlpObject* _this)
{
  INT16 nErr = O_K;
  DEBUGMSG(-1,"CDlpObject_AutoRegisterWords for '%s'",_this->m_lpInstanceName,0,0);
  nErr = CDlpObject_UnregisterAllWords(_this);
  if (!OK(nErr)) return nErr;

  REGISTER_FIELD("check","",LPMV(m_nCheck),NULL,"Verbose level",0,T_SHORT,1,"",0)
  return O_K;
}

/**
 * Base class implementation of one-time instance initialization. Init
 * calls the Reset function.
 *
 * @param _this        This instance
 * @param bCallVirtual For derived classes: if TRUE call base class implementation
 * @return O_K if successful, an error code otherwise
 */
INT16 CDlpObject_Init(CDlpObject* _this, BOOL bCallVirtual)
{
  INT16 nErr = O_K;

  DEBUGMSG(-1,"CDlpObject_Init for '%s'",_this->m_lpInstanceName,0,0);

#ifndef __NORTTI /* init will be done through AutoRegisterWords */
  nErr = INVOKE_VIRTUAL_1(ResetAllOptions,TRUE); IF_NOK(nErr) return nErr;
  nErr = CDlpObject_ResetAllFields(_this,TRUE);  IF_NOK(nErr) return nErr;
#endif

  return nErr;
}

/**
 * Base class implementation of instance reset.
 *
 * @param _this         This instance
 * @param bResetMembers If TRUE reset all options and fields
 * @return O_K if successful, an error code otherwise
 */
INT16 CDlpObject_Reset(CDlpObject* _this, BOOL bResetMembers)
{
  INT16 nErr = O_K;

  DEBUGMSG(-1,"CDlpObject_Reset for '%s'; (bResetMembers=%d)",_this->m_lpInstanceName,bResetMembers,0);

#ifdef __NORTTI
  IERROR(_this,ERR_DANGEROUS,"CDlpObject_Reset","nothing will be reseted in __NORTTI mode",0);
  DLPASSERT(FALSE);
#else
  if (bResetMembers)
  {
    nErr = INVOKE_VIRTUAL_1(ResetAllOptions,FALSE); IF_NOK(nErr) return nErr;
    nErr = CDlpObject_ResetAllFields(_this,FALSE);  IF_NOK(nErr) return nErr;
  }
#endif

  return nErr;
}

/**
 * Generic instance copy.
 *
 * @param _this This instance
 * @param iSrc  Source instance to copy
 * @return O_K if successful, an error code otherwise
 */
INT16 CDlpObject_Copy(CDlpObject* _this, CDlpObject* iSrc)
{
  return CDlpObject_CopySelective(_this, iSrc, WL_TYPE_FIELD | WL_TYPE_INSTANCE);
}

/**
 * Copies selected word types.
 *
 * @param _this This instance
 * @param iSrc  Source instance to copy
 * @param nWhat Word type to copy (for more than one use "|")
 */
INT16 CDlpObject_CopySelective(CDlpObject* _this, CDlpObject* iSrc, INT16 nWhat)
{
  hscan_t     hs;
  hnode_t*    hn;
  SWord*      lpWord = NULL;
  CDlpObject* iSrcInt;
  CDlpObject* iDstInt;

  CHECK_THIS_RV(NOT_EXEC);
  DEBUGMSG(-1,"CDlpObject_Copy for '%s'",_this->m_lpInstanceName,0,0);
#ifdef __NORTTI
  if(strcmp(_this->m_lpClassName,"data")){
    IERROR(_this,ERR_DANGEROUS,"CDlpObject_Copy","No fields can be copied in __NORTTI mode.",0);
    DLPASSERT(FALSE);
  }
#endif

  if(_this == iSrc) return O_K;
  if(!iSrc)         return NOT_EXEC;

  /* TODO: Check this! --> */
  /*if(!CDlpObject_IsKindOf(iSrc,_this->m_lpClassName))*/

#ifdef __cplusplus
  if(!iSrc->IsKindOf(_this->m_lpClassName))
#else
  if(!BASEINST(iSrc)->IsKindOf(BASEINST(iSrc),_this->m_lpClassName))
#endif
    return
      IERROR(_this,ERR_NOTOFKIND,iSrc->m_lpInstanceName,_this->m_lpClassName,0);
  /* <-- */

  /* Loop over own dictionary */
  hash_scan_begin(&hs,_this->m_lpDictionary);
  while ((hn = hash_scan_next(&hs))!=NULL)
  {
    DLPASSERT((lpWord = (SWord*)hnode_get(hn))!=NULL); /* NULL entry in dictionary */
    if (!lpWord) continue;
    switch (lpWord->nWordType & nWhat)
    {
      case WL_TYPE_FIELD:
        CDlpObject_CopyField(_this,lpWord,iSrc);
        break;
      case WL_TYPE_INSTANCE:
        iDstInt = (CDlpObject*)lpWord->lpData;
        iSrcInt = CDlpObject_FindInstanceWord(iSrc,lpWord->lpName,NULL);
        if (iSrcInt && iDstInt)
#ifdef __cplusplus
          iDstInt->Copy(iSrcInt);
#else
        BASEINST(iDstInt)->Copy(BASEINST(iDstInt),BASEINST(iSrcInt));
#endif
        break;
      case WL_TYPE_DONTCARE: /* Not supported */
      case WL_TYPE_ERROR:    /* Not supported */
      case WL_TYPE_FACTORY:  /* Not supported */
      case WL_TYPE_METHOD:   /* Not supported */
      case WL_TYPE_OPERATOR: /* Not supported */
      case WL_TYPE_OPTION:   /* Not supported */
      default: break;
    }
  }

  /* Loop over source's dictionary */
  hash_scan_begin(&hs,iSrc->m_lpDictionary);
  while ((hn = hash_scan_next(&hs))!=NULL)
  {
    DLPASSERT((lpWord = (SWord*)hnode_get(hn))!=NULL); /* NULL entry in dictionary */
    if (!lpWord) continue;
    switch (lpWord->nWordType & nWhat)
    {
      case WL_TYPE_INSTANCE:
        iSrcInt = (CDlpObject*)lpWord->lpData;
        iDstInt = CDlpObject_FindInstanceWord(_this,lpWord->lpName,NULL);
        if (iDstInt) continue; /* Has already been copied */
        iDstInt = CDlpObject_Instantiate(_this,iSrcInt->m_lpClassName,iSrcInt->m_lpInstanceName,FALSE);
        if (iSrcInt && iDstInt)
#ifdef __cplusplus
          iDstInt->Copy(iSrcInt);
#else
        BASEINST(iDstInt)->Copy(BASEINST(iDstInt),BASEINST(iSrcInt));
#endif
        break;
      case WL_TYPE_DONTCARE: /* Don't know */
      case WL_TYPE_ERROR:    /* Don't know */
      case WL_TYPE_FACTORY:  /* Don't know */
      case WL_TYPE_METHOD:   /* Don't know */
      case WL_TYPE_OPERATOR: /* Don't know */
      case WL_TYPE_OPTION:   /* Don't know */
      default: break;
    }
  }

  _this->m_nCheck = iSrc->m_nCheck;

  return O_K;
}

/**
 * Copies the one field from iSrc to this instance
 *
 * @param _this  This (destination) instance
 * @param lpWord Pointer to a SWord structure identifying the field to copy
 * @param iSrc   The source instance to copy the field from
 * @return O_K if successful, an error code otherwise
 *
 * HACK: This implementation copies simple types and strings only!!!
 * TODO: Implement instance and pointer copying
 */
INT16 CDlpObject_CopyField(CDlpObject* _this, SWord* lpWord, CDlpObject* iSrc)
{
  SWord* lpWordSrc = NULL;

  /* Validate input */
  CHECK_THIS_RV(NOT_EXEC);
  if (!lpWord) return NOT_EXEC;
  if (!iSrc  ) return NOT_EXEC;
  if (lpWord->nFlags & (FF_NONAUTOMATIC|FF_NOSAVE)) return NOT_EXEC;

  /* Get source word */
  lpWordSrc = CDlpObject_FindWord(iSrc,lpWord->lpName,WL_TYPE_FIELD);
  DLPASSERT(lpWordSrc);                                     /* Instance type?     */
  DLPASSERT(lpWord->nWordType   ==lpWordSrc->nWordType   ); /* Field overwritten? */
  DLPASSERT(lpWord->ex.fld.nType==lpWordSrc->ex.fld.nType); /* Field overwritten? */

  /*printf("\n Copy field %s.%s --> %s.%s",BASEINST(iSrc)->m_lpInstanceName,lpWord->lpName,BASEINST(_this)->m_lpInstanceName,lpWord->lpName);*/

  /* Copy data */
  switch (lpWord->ex.fld.nType)
  {
  case T_BOOL   : dlp_memmove(lpWord->lpData,lpWordSrc->lpData,sizeof(     BOOL)); return O_K;
  case T_UCHAR  : dlp_memmove(lpWord->lpData,lpWordSrc->lpData,sizeof(    UINT8)); return O_K;
  case T_CHAR   : dlp_memmove(lpWord->lpData,lpWordSrc->lpData,sizeof(     INT8)); return O_K;
  case T_USHORT : dlp_memmove(lpWord->lpData,lpWordSrc->lpData,sizeof(   UINT16)); return O_K;
  case T_SHORT  : dlp_memmove(lpWord->lpData,lpWordSrc->lpData,sizeof(    INT16)); return O_K;
  case T_UINT   : dlp_memmove(lpWord->lpData,lpWordSrc->lpData,sizeof(   UINT32)); return O_K;
  case T_INT    : dlp_memmove(lpWord->lpData,lpWordSrc->lpData,sizeof(    INT32)); return O_K;
  case T_ULONG  : dlp_memmove(lpWord->lpData,lpWordSrc->lpData,sizeof(   UINT64)); return O_K;
  case T_LONG   : dlp_memmove(lpWord->lpData,lpWordSrc->lpData,sizeof(    INT64)); return O_K;
  case T_FLOAT  : dlp_memmove(lpWord->lpData,lpWordSrc->lpData,sizeof(  FLOAT32)); return O_K;
  case T_DOUBLE : dlp_memmove(lpWord->lpData,lpWordSrc->lpData,sizeof(  FLOAT64)); return O_K;
  case T_COMPLEX: dlp_memmove(lpWord->lpData,lpWordSrc->lpData,sizeof(COMPLEX64)); return O_K;
  case T_STRING :
  case T_CSTRING:
  case T_TEXT   :
    dlp_free(*(char**)lpWord->lpData);
    *(char**)lpWord->lpData = NULL;
    if (*(char**)lpWordSrc->lpData)
    {
      *(char**)lpWord->lpData = (char*)dlp_malloc(dlp_size(*(char**)lpWordSrc->lpData));
      dlp_strcpy(*(char**)lpWord->lpData,*(char**)lpWordSrc->lpData);
    }
    return O_K;
  case T_INSTANCE:
    if (*(CDlpObject**)lpWordSrc->lpData)
    {
      if (*(CDlpObject**)lpWord->lpData==NULL)
      {
      #ifdef __cplusplus
        *(CDlpObject**)lpWord->lpData = CDlpObject_CreateInstanceOf(lpWordSrc->ex.fld.lpType,lpWordSrc->lpName);
      #else
        *(CDlpObject**)lpWord->lpData = *(CDlpObject**)CDlpObject_CreateInstanceOf(lpWordSrc->ex.fld.lpType,lpWordSrc->lpName);
      #endif
        if (!*(CDlpObject**)lpWord->lpData)
          IERROR(_this,ERR_CREATEINSTANCE,lpWord->lpName,0,0);
        else
          (*(CDlpObject**)lpWord->lpData)->m_lpContainer=lpWord;
      }
    #ifdef __cplusplus
      return (*(CDlpObject**)lpWord->lpData)->Copy(*(CDlpObject**)lpWordSrc->lpData);
    #else
      return (*(CDlpObject**)lpWord->lpData)->Copy(*(CDlpObject**)lpWord->lpData,*(CDlpObject**)lpWordSrc->lpData);
    #endif
    }
    else if (*(CDlpObject**)lpWord->lpData)
    {
      IDESTROY((*(CDlpObject**)lpWord->lpData));
    }
  default:
    if (lpWord->ex.fld.nType>0 && lpWord->ex.fld.nType<=255)
    {
      dlp_memmove(lpWord->lpData,lpWordSrc->lpData,lpWord->ex.fld.nType);
      return O_K;
    }
    return NOT_EXEC;
  }
}

/**
 * Base class implementation of the class procedure. The class procedure
 * is called when the instance identifier is interpreted.
 *
 * @param _this This instance
 * @return O_K if successful, an error code otherwise
 */
INT16 CDlpObject_ClassProc(CDlpObject* _this)
{
  DEBUGMSG(-1,"CDlpObject_ClassProc for '%s'",_this->m_lpInstanceName,0,0);

  return O_K;
}

/**
 * Base class implementation of the class installation precedure.
 * The base class implementation does nothing.
 *
 * @param iItp Pointer to dLabPro interpreter, NULL if no interpreter
 *             is running
 * @return O_K
 */
INT16 CDlpObject_InstallProc(void* iItp)
{
  return O_K;
}

/**
 * Base class implementation of the class factory.
 * The base class implementation does nothing because the base class
 * itself cannot be instanciated.
 *
 * @param lpsName Identifier for the instance to be created
 * @return NULL
 */
CDlpObject* CDlpObject_CreateInstance(const char* lpsName)
{
  CDlpObject* iNewInstance;
  ICREATEEX(CDlpObject,iNewInstance,lpsName,NULL);
  return iNewInstance;
}

/**
 * Base class implementation of the class information function.
 * The base class implementation does nothing.
 *
 * @param lpWord Pointer to a SWord structure which will receive the
 *               information
 * @return O_K if successful, an error code otherwise
 */
INT16 CDlpObject_GetClassInfo(SWord* lpClassWord)
{
  if (!lpClassWord) return NOT_EXEC;
  dlp_memset(lpClassWord,0,sizeof(SWord));

  lpClassWord->nWordType          = WL_TYPE_FACTORY;
  lpClassWord->nFlags             = CS_AUTOACTIVATE;

#ifdef __cplusplus

  lpClassWord->ex.fct.lpfFactory  = (LP_FACTORY_PROC)CDlpObject::CreateInstance;
  lpClassWord->ex.fct.lpfInstall  = CDlpObject::InstallProc;

#else /* #ifdef __cplusplus */

  lpClassWord->ex.fct.lpfFactory  = (LP_FACTORY_PROC)CDlpObject_CreateInstance;
  lpClassWord->ex.fct.lpfInstall  = CDlpObject_InstallProc;

#endif /* #ifdef __cplusplus */

  lpClassWord->ex.fct.lpProject   = "dlpobject";
  lpClassWord->ex.fct.lpBaseClass = "-";
  lpClassWord->lpComment          = "Generic object";
  lpClassWord->ex.fct.lpAutoname  = "";
  lpClassWord->ex.fct.lpCname     = "CDlpObject";
  lpClassWord->ex.fct.lpAuthor    = "Matthias Wolff";

  dlp_strcpy(lpClassWord->lpName             ,"object");
  dlp_strcpy(lpClassWord->lpObsname          ,"");
  dlp_strcpy(lpClassWord->ex.fct.version.no  ,"2.5.1");

  return O_K;
}

/**
 * Base class implementation of the instance information function.
 * The base class implementation does nothing.
 *
 * @param lpWord Pointer to a SWord structure which will receive the
 *               information
 * @return O_K if successful, an error code otherwise
 */
INT16 CDlpObject_GetInstanceInfo(CDlpObject* _this, SWord* lpClassWord)
{
  return CDlpObject_GetClassInfo(lpClassWord);
}

/**
 * Checks if this instance is (1) of the given class type or (2)
 * is of a class type derived from the given type. This base class
 * implementation tests only for case (1). It will be overwritten
 * by all derived classes.
 *
 * @param _this        This instance
 * @param lpsClassName Type name to check for
 * @return TRUE if this instance is of the given type or cna be
 *         casted to this type, FALSE otherwise
 */
BOOL CDlpObject_IsKindOf(CDlpObject* _this, const char* lpsClassName)
{
  return (dlp_strncmp(_this->m_lpClassName,lpsClassName,L_NAMES) ==0);
}

/**
 * Base class implementation of instance reset all options.
 *
 * @param _this Pointer to this instance
 * @return O_K
 */
INT16 CDlpObject_ResetAllOptions(CDlpObject* _this, BOOL bInit)
{
  return O_K;
}

/**
 * Base class implementation of the instance self check function.
 *
 * @param nMode Check mode, a combination of the CFM_XXX constants
 * @return <code>O_K</code> if successfull, a (negative) error code otherwise
 */
INT16 CDlpObject_Check(CDlpObject* _this, INT32 nMode)
{
  hscan_t  hs;
  hnode_t* hn;
  INT16    nRet         = O_K;
  SWord*   lpWord       = NULL;
  char     sFqn[L_SSTR];

  CHECK_THIS_RV(NOT_EXEC);
  DEBUGMSG(DM_KERNEL," Checking all fields of instance '%s' !",
    _this->m_lpInstanceName,0,0);

  hash_scan_begin(&hs,_this->m_lpDictionary);
  while ((hn = hash_scan_next(&hs))!=NULL)
  {
    lpWord = (SWord*)hnode_get(hn);
    if (!lpWord) continue;
    switch (lpWord->nWordType)
    {
      case WL_TYPE_FIELD:
        switch (lpWord->ex.fld.nType)
        {
          case T_INSTANCE:
            if (!*(CDlpObject**)lpWord->lpData) continue;
            CHECK_IPTR(*(CDlpObject**)lpWord->lpData,lpWord->ex.fld.nISerialNum);
            if (!*(CDlpObject**)lpWord->lpData)
            {
              sprintf(sFqn,"%s.%s",_this->m_lpInstanceName,lpWord->lpName);
              IERROR(_this,ERR_BADPTR,*(CDlpObject**)lpWord->lpData,"field",
                sFqn);
            }
            else if (nMode & CFM_RECURSIVE)
            {
              if (CDlpObject_GetParent(*(CDlpObject**)lpWord->lpData)==_this)
                CDlpObject_Check(*(CDlpObject**)lpWord->lpData,nMode);
            }
            break;

          case T_STRING:
          case T_CSTRING:
          case T_TEXT:
          case T_PTR:
            if (!*(void**)lpWord->lpData) continue;
            if (!dlp_in_xalloc(*(void**)lpWord->lpData))
            {
                sprintf(sFqn,"%s.%s",_this->m_lpInstanceName,lpWord->lpName);
                IERROR(_this,ERR_BADPTR,*(void**)lpWord->lpData,"field",sFqn);
              *(void**)lpWord->lpData = NULL;
            }
            break;
        }
        break;

      case WL_TYPE_INSTANCE:
        if (!(CDlpObject*)lpWord->lpData) continue;
        if (!CDlpObject_CheckInstancePtr((CDlpObject*)lpWord->lpData,0))
        {
          lpWord->lpData = NULL;
          sprintf(sFqn,"%s.%s",_this->m_lpInstanceName,lpWord->lpName);
          IERROR(_this,ERR_BADPTR,*(CDlpObject**)lpWord->lpData,"instance",
            sFqn);
        }
        else if (nMode & CFM_RECURSIVE)
        {
          if (CDlpObject_GetParent((CDlpObject*)lpWord->lpData)==_this)
            CDlpObject_Check((CDlpObject*)lpWord->lpData,nMode);
        }
        break;
    }
  }

  return nRet;
}

/* CDlpObject - C++ Member functions */
#ifdef __cplusplus

CDlpObject::CDlpObject(const char* lpsInstanceName, BOOL bCallVirtual)
{
  CDlpObject_Constructor(this,lpsInstanceName,bCallVirtual);
}

CDlpObject::~CDlpObject()
{
  CDlpObject_Destructor(this);
}

INT16 CDlpObject::AutoRegisterWords()
{
  return CDlpObject_AutoRegisterWords(this);
}

INT16 CDlpObject::Reset(BOOL bResetMembers)
{
  return CDlpObject_Reset(this,bResetMembers);
}

INT16 CDlpObject::Init(BOOL bCallVirtual)
{
  return CDlpObject_Init(this,bCallVirtual);
}

INT16 CDlpObject::Copy(CDlpObject* iSrc)
{
  return CDlpObject_Copy(this,iSrc);
}

INT16 CDlpObject::ClassProc()
{
  return CDlpObject_ClassProc(this);
}

INT16 CDlpObject::InstallProc(void* iItp)
{
  return CDlpObject_InstallProc(iItp);
}

CDlpObject* CDlpObject::CreateInstance(const char* lpsName)
{
  return CDlpObject_CreateInstance(lpsName);
}

INT16 CDlpObject::GetClassInfo(SWord* lpClassWord)
{
  return CDlpObject_GetClassInfo(lpClassWord);
}

INT16 CDlpObject::GetInstanceInfo(SWord* lpClassWord)
{
  return CDlpObject_GetInstanceInfo(this,lpClassWord);
}

BOOL CDlpObject::IsKindOf(const char* lpsClassName)
{
  return CDlpObject_IsKindOf(this,lpsClassName);
}

INT16 CDlpObject::ResetAllOptions(BOOL bInit)
{
  return CDlpObject_ResetAllOptions(this,bInit);
}

INT16 CDlpObject::Check(INT32 nMode)
{
  return CDlpObject_Check(this,nMode);
}

SWord* CDlpObject::RegisterWord(const SWord* lpWord,...)
{
  /* TODO: Check if this works for all types!!! */
  void*   lpValue;
  va_list ap;
  va_start(ap,lpWord);
  lpValue = va_arg(ap,void*);
  va_end(ap);

  return CDlpObject_RegisterWord(this,lpWord,lpValue);
}

SWord* CDlpObject::RegisterOperator(const SWord* lpWord)
{
  return CDlpObject_RegisterOperator(this, lpWord);
}

INT16 CDlpObject::UnregisterWord(SWord* lpWord, INT16 bDeleteInstance)
{
  return CDlpObject_UnregisterWord(this,lpWord,bDeleteInstance);
}

INT16 CDlpObject::UnregisterAllWords()
{
  return CDlpObject_UnregisterAllWords(this);
}

INT16 CDlpObject::UnregisterAllOperators()
{
  return CDlpObject_UnregisterAllOperators(this);
}

SWord* CDlpObject::FindWordInternal(const char* lpsName, INT16 nMask)
{
  return CDlpObject_FindWordInternal(this,lpsName,nMask);
}

SWord* CDlpObject::FindWord(const char* lpsName, INT16 nMask)
{
  return CDlpObject_FindWord(this,lpsName,nMask);
}

CDlpObject* CDlpObject::FindInstanceWord(const char *lpsInstanceName, const char* lpsClassName)
{
  return CDlpObject_FindInstanceWord(this,lpsInstanceName,lpsClassName);
}

SWord* CDlpObject::FindFactoryWord(const char *lpsName)
{
  return CDlpObject_FindFactoryWord(this,lpsName);
}

CDlpObject* CDlpObject::Instantiate(const char* lpsClassName, const char* lpsInstanceName, BOOL bGlobal)
{
  return CDlpObject_Instantiate(this,lpsClassName,lpsInstanceName,bGlobal);
}

SWord* CDlpObject::FindFieldPtr(const void* lpData, BOOL bRecursive)
{
  return CDlpObject_FindFieldPtr(this,lpData,bRecursive);
}

SWord* CDlpObject::FindOperator(const char* lpsName)
{
  return CDlpObject_FindOperator(this,lpsName);
}

char* CDlpObject::GetFQName(char* lpsName, BOOL bForceArray)
{
  return CDlpObject_GetFQName(this,lpsName,bForceArray);
}

INT16 CDlpObject::SetField(SWord* lpWord,void* lpWhat)
{
  return CDlpObject_SetField(this,lpWord,lpWhat);
}

INT16 CDlpObject::ResetField(SWord* lpWord, BOOL bDestroying)
{
  return CDlpObject_ResetField(this,lpWord,bDestroying);
}

INT16 CDlpObject::ResetAllFields(BOOL bInit)
{
  return CDlpObject_ResetAllFields(this,bInit);
}

INT16 CDlpObject::FieldToString(char* lpsBuffer, size_t nBufferLen, SWord* lpWord)
{
  return CDlpObject_FieldToString(this,lpsBuffer,nBufferLen,lpWord);
}

INT16 CDlpObject::FieldToDouble(SWord* lpWord, FLOAT64* lpBuffer)
{
  return CDlpObject_FieldToDouble(this,lpWord,lpBuffer);
}

INT16 CDlpObject::FieldFromString(SWord* lpWord, const char* lpsBuffer)
{
  return CDlpObject_FieldFromString(this,lpWord,lpsBuffer);
}

INT16 CDlpObject::SetOption(SWord* lpWord)
{
  return CDlpObject_SetOption(this,lpWord);
}

INT16 CDlpObject::CopyAllOptions(CDlpObject* iSrc)
{
	return CDlpObject_CopyAllOptions(this, iSrc);
}

void CDlpObject::PrintMember(SWord* lpWord, INT16 nHow, BOOL bLast)
{
  CDlpObject_PrintMember(this,lpWord,nHow,bLast);
}

void CDlpObject::PrintAllMembers(INT16 nWordType, INT16 nHow)
{
  CDlpObject_PrintAllMembers(this,nWordType,nHow);
}

INT16 CDlpObject::PrintField(const char* lpsName, BOOL bInline)
{
  return CDlpObject_PrintField(this,lpsName,bInline);
}

void CDlpObject::PrintVersionInfo()
{
  CDlpObject_PrintVersionInfo(this);
}

void CDlpObject::PrintDictionary(INT16 nHow)
{
  CDlpObject_PrintDictionary(this,nHow);
}

void* CDlpObject::GetStaticFieldPtr(const char* lpsName)
{
  return CDlpObject_GetStaticFieldPtr(lpsName);
}

void CDlpObject::SetFormexFunc(LPF_FORMEX FormExFunc, CDlpObject* iFormExInst)
{
  CDlpObject_SetFormexFunc(FormExFunc,iFormExInst);
}

void CDlpObject::GetFormexFunc(LPF_FORMEX* FormExFunc, CDlpObject** lpiFormExInst)
{
  CDlpObject_GetFormexFunc(FormExFunc,lpiFormExInst);
}

void CDlpObject::RegisterClass(const SWord* lpClassWord)
{
  CDlpObject_RegisterClass(lpClassWord);
}

void CDlpObject::UnregisterAllClasses()
{
  CDlpObject_UnregisterAllClasses();
}

INT16 CDlpObject::LoadClassRegistry(BOOL bAutoInst)
{
  return CDlpObject_LoadClassRegistry(this,bAutoInst);
}
CDlpObject* CDlpObject::CreateInstanceOf(const char* lpsClassName, const char* lpsInstanceName)
{
  return CDlpObject_CreateInstanceOf(lpsClassName,lpsInstanceName);
}

CDlpObject* CDlpObject::OfKind(const char* lpsClassName, CDlpObject* iInst)
{
  return CDlpObject_OfKind(lpsClassName,iInst);
}

BOOL CDlpObject::OfKindStr(const char* lpsClassName, const char* lpsBaseClassName)
{
  return CDlpObject_OfKindStr(lpsClassName,lpsBaseClassName);
}

CDlpObject* CDlpObject::CheckInstancePtr(CDlpObject* iInst, UINT64 nSerialNum)
{
  return CDlpObject_CheckInstancePtr(iInst,nSerialNum);
}

CDlpObject* CDlpObject::GetParent()
{
  return CDlpObject_GetParent(this);
}

CDlpObject* CDlpObject::GetRoot()
{
  return CDlpObject_GetRoot(this);
}

CDlpObject* CDlpObject::FindInstance(const char* lpsName)
{
  return CDlpObject_FindInstance(this,lpsName);
}

INT16 CDlpObject::SetErrorLevel(INT16 nErrorLevel)
{
  return CDlpObject_SetErrorLevel(nErrorLevel);
}

INT16 CDlpObject::GetErrorLevel()
{
  return CDlpObject_GetErrorLevel();
}

void CDlpObject::SetTraceError(const char* lpsTraceError)
{
  CDlpObject_SetTraceError(lpsTraceError);
}

void CDlpObject::GetTraceError(char* lpsBuffer, INT16 nBufferLen)
{
  CDlpObject_GetTraceError(lpsBuffer,nBufferLen);
}

INT32 CDlpObject::GetErrorCount()
{
  return CDlpObject_GetErrorCount();
}

void CDlpObject::SetLastError(INT16 nError, CDlpObject* iErrorInst)
{
  CDlpObject_SetLastError(nError,iErrorInst);
}

void CDlpObject::SetErrorPos(const char* lpInFile, INT32 nInLine)
{
  CDlpObject_SetErrorPos(lpInFile, nInLine);
}

void CDlpObject::GetErrorPos(char* lpInFile, INT32* lpInLine)
{
  CDlpObject_GetErrorPos(lpInFile, lpInLine);
}

INT16 CDlpObject::Error(CDlpObject* iInst, const char* lpsFilename, INT32 nLine, INT16 nErrorID, ...)
{
  void*   lpArg1  = NULL;
  void*   lpArg2  = NULL;
  void*   lpArg3  = NULL;
  va_list ap;

  va_start(ap,nErrorID);
  lpArg1 = va_arg(ap,void*);
  lpArg2 = va_arg(ap,void*);
  lpArg3 = va_arg(ap,void*);
  va_end(ap);

  return CDlpObject_Error(iInst,lpsFilename,nLine,nErrorID,lpArg1,lpArg2,lpArg3);
}

void CDlpObject::ErrorLog()
{
  CDlpObject_ErrorLog();
}

#endif /* #ifdef __cplusplus */


/* EOF */
