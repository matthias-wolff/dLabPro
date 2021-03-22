/* dLabPro class CDlpObject (object)
 * - Static public functions
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
#include "dlp_kernel.h"
#include "dlp_object.h"
#include "dlp_config.h"

/* Disable some MSVC warnings */
#ifdef __MSVC
  #pragma warning( disable : 4100 ) /* Unreferenzierter formaler Parameter */
#endif

/* Global static variables */
static char        __lpInFile[L_PATH]   = "";   /* Source file name of current error           */
static INT32        __nInLine            = -1;   /* Source file line of current error           */
static CDlpObject* __iLastErrorInst     = NULL; /* Instance in which most recent error occ.    */
static INT16       __nLastError         = 0;    /* ID of most recent error                     */
static INT16       __nElevel            = 255;  /* Global error tolerance                      */
static INT32        __nErrors            = 0;    /* Error counter                               */
static INT32        __nWarnings          = 0;    /* Warning counter                             */
static char        __lpsTraceError[255] = "";   /* Assert on this error                        */
static SWord*      __lpClassRegistry    = NULL; /* The class registry                          */
static INT32        __nXClassRegistry    = 0;    /* The class registry                          */
static LPF_FORMEX  __FormExFunc         = NULL; /* Pointer to std.formula interpretation func. */
static CDlpObject* __iFormExInst        = NULL; /* Pointer to std.formula interpreter instance */
static UINT64      __nSerialNum         = 1;    /* Next free serial number for objects         */

/**
 * Returns a pointer to the specified static field of CDlpObject.
 *
 * @param lpsName Identifier of field
 *                - "in_file"        : __lpInFile
 *                - "in_line"        : __nInLine
 *                - "last_error_inst": __lpLastErrorInst
 *                - "last_error"     : __nLastError
 *                - "elevel"         : __nElevel
 *                - "errors"         : __nErrors
 *                - "warnings"       : __nWarnings
 *                - "trace_error"    : __lpsTraceError
 * @return The pointer if successfull, NULL otherwise.
 */
void* CDlpObject_GetStaticFieldPtr(const char* lpFieldIdentifier)
{
  if      (dlp_strcmp(lpFieldIdentifier,"in_file"        )==0) return &__lpInFile;
  else if (dlp_strcmp(lpFieldIdentifier,"in_line"        )==0) return &__nInLine;
  else if (dlp_strcmp(lpFieldIdentifier,"last_error_inst")==0) return &__iLastErrorInst;
  else if (dlp_strcmp(lpFieldIdentifier,"last_error"     )==0) return &__nLastError;
  else if (dlp_strcmp(lpFieldIdentifier,"elevel"         )==0) return &__nElevel;
  else if (dlp_strcmp(lpFieldIdentifier,"errors"         )==0) return &__nErrors;
  else if (dlp_strcmp(lpFieldIdentifier,"warnings"       )==0) return &__nWarnings;
  else if (dlp_strcmp(lpFieldIdentifier,"trace_error"    )==0) return &__lpsTraceError;
  else return NULL;
}

/**
 * Sets a new formula execution function and its argument pointer.
 */
void CDlpObject_SetFormexFunc
(
  LPF_FORMEX    FormExFunc,
  CDlpObject* iFormExInst
)
{
  __FormExFunc  = FormExFunc;
  __iFormExInst = iFormExInst;
}

/**
 * Retrieves the current formula execution function and its argument poointer.
 */
void CDlpObject_GetFormexFunc
(
  LPF_FORMEX*    lpFormExFunc,
  CDlpObject** lpiFormExInst
)
{
  if (lpFormExFunc ) *lpFormExFunc  = __FormExFunc;
  if (lpiFormExInst) *lpiFormExInst = __iFormExInst;
}

/**
 * Registers an instanciation function and assosicates it with a dLabPro class
 * name.
 *
 * @param lpClassWord
 *         <code>SWord</code> struct containing dLabPro class info
 * @see CDlpObject_CreateInstanceOf
 * @see CDlpObject_UnregisterAllClasses
 */
void CDlpObject_RegisterClass(const SWord* lpClassWord)
{
  INT32 nXC = 0;

  if (!lpClassWord                           ) return;
  if (lpClassWord->nWordType!=WL_TYPE_FACTORY) return;

  nXC               = __nXClassRegistry++;
  __lpClassRegistry = (SWord*)dlp_realloc(__lpClassRegistry,__nXClassRegistry,sizeof(SWord));
  dlp_memmove(&__lpClassRegistry[nXC],lpClassWord,sizeof(SWord));
}

/**
 * Destroys the class registry and frees all associated memory.
 *
 * @see CDlpObject_RegisterClass
 * @see CDlpObject_CreateInstanceOf
 */
void CDlpObject_UnregisterAllClasses()
{
  dlp_free(__lpClassRegistry);
  __lpClassRegistry=NULL;
}

/**
 * Lists all registered classes at stdout.
 */
void CDlpObject_PrintClassRegistry()
{
  INT32 nC  = 0;

  printf("\n   Table of classes");
  for (nC=0; nC<__nXClassRegistry; nC++)
    printf("\n   %2d: %-16s %s",(int)nC,__lpClassRegistry[nC].lpName,
      __lpClassRegistry[nC].ex.fct.version.no);
}

/**
 * Copies the global class registry into the instances dictionary.
 *
 * @param _this
 *          This instance
 * @param bAutoInst
 *          If <code>TRUE</code> create auto-instances
 * @return <code>O_K</code> if successful, a (negative) error code otherwise
 **/
INT16 CDlpObject_LoadClassRegistry(CDlpObject* _this, BOOL bAutoInst)
{
  INT32          nC            = 0;
  const SWord* lpFactoryWord = NULL;
  CDlpObject*  lpInst        = NULL;

  for (nC=0; nC<__nXClassRegistry; nC++)
  {
    lpFactoryWord = &__lpClassRegistry[nC];
    DLPASSERT(lpFactoryWord && lpFactoryWord->nWordType==WL_TYPE_FACTORY);

    /* Call installation function */
    if (NOK((lpFactoryWord->ex.fct.lpfInstall)(_this))) continue;

    /* Register singleton or auto instance in dictionary */
    if ((lpFactoryWord->nFlags & (CS_SINGLETON|CS_AUTOINSTANCE)) && bAutoInst)
    {
      SWord newword;
      const char* lpsInstanceName = lpFactoryWord->ex.fct.lpAutoname;
      if (lpFactoryWord->nFlags & CS_SINGLETON)
        lpsInstanceName = lpFactoryWord->lpName;

      if (dlp_strlen(lpFactoryWord->lpName) ==0) continue;
      lpInst = (lpFactoryWord->ex.fct.lpfFactory)(lpsInstanceName);
      if (!lpInst) continue;

      memset(&newword,0,sizeof(SWord));
      newword.nWordType = WL_TYPE_INSTANCE;
      newword.lpData    = lpInst;
      strcpy(newword.lpName,lpsInstanceName);
      strcpy(newword.lpObsname,lpFactoryWord->lpObsname);
      CDlpObject_RegisterWord(_this,&newword);
      lpInst->m_nInStyle |= IS_GLOBAL;
    }

    /* Register class factory in dictionary */
    if (!(lpFactoryWord->nFlags & CS_SINGLETON))
    {
      CDlpObject_RegisterWord(_this,lpFactoryWord);
    }
  }

  return O_K;
}

/**
 * Creates a new instance of class <code>lpsClassName</code>.
 *
 * @param lpsClassName    dLabPro class identifier
 * @param lpsInstanceName Name of new instance
 * @return A pointer to a new instance of class <code>lpsClassName</code> or
 *         <code>NULL</code> in case of errors.
 * @see CDlpObject_RegisterClass
 * @see CDlpObject_UnregisterAllClasses
 */
CDlpObject* CDlpObject_CreateInstanceOf
(
  const char* lpsClassName,
  const char* lpsInstanceName
)
{
  INT32 nC  = 0;                                                                  /* Current class index               */

  for (nC=0; nC<__nXClassRegistry; nC++)                                        /* Loop over classes                 */
    if                                                                          /*   If dLabPro or C/C++ id matches  */
    (                                                                           /*   |                               */
      dlp_strcmp(lpsClassName,__lpClassRegistry[nC].lpName        )==0 ||       /*   |                               */
      dlp_strcmp(lpsClassName,__lpClassRegistry[nC].ex.fct.lpCname)==0          /*   |                               */
    )                                                                           /*   |                               */
      if (__lpClassRegistry[nC].ex.fct.lpfFactory)                              /*     If there is a class factory   */
        return (__lpClassRegistry[nC].ex.fct.lpfFactory)(lpsInstanceName);      /*       Instantiate                 */

  return NULL;                                                                  /* Sorry, it didn't work out         */
}

/**
 * Checks if an instance is of a given type or can be casted to this type.
 *
 * @param lpsClassName dLabPro class identifier to check for
 * @param iInst        Instance to check
 * @return The instance or NULL if no type cast is possible
 * @see CDlpObject_OfKindStr
 */
CDlpObject* CDlpObject_OfKind(const char* lpsClassName, CDlpObject* iInst)
{
  if (iInst == NULL) return NULL;
  if (CDlpObject_IsKindOf(iInst,lpsClassName)) return iInst;
  return NULL;
}

/**
 * Checks if a dLabPro class identifier denotes a given type or a type which
 * can be casted to the given type.
 *
 * <h3>Note</h3>
 * <p>This procedure is quite large-scaled. Use OfKind whenever possible!</p>
 *
 * @param lpsClassName     dLabPro class identifier to check for
 * @param lpsBaseClassName Base class to check for
 * @return TRUE if a type case is possible, FALSE otherwise
 * @see CDlpObject_OfKind
 */
BOOL CDlpObject_OfKindStr(const char* lpsClassName, const char* lpsBaseClassName)
{
  INT32        nC     = 0;
  const char* lpsBuf = NULL;

  while (lpsClassName)
  {
    if (dlp_strcmp(lpsClassName,lpsBaseClassName)==0) return TRUE;

    lpsBuf = NULL;
    for (nC=0; nC<__nXClassRegistry; nC++)
      if (dlp_strcmp(__lpClassRegistry[nC].lpName,lpsClassName)==0)
        lpsBuf = __lpClassRegistry[nC].ex.fct.lpBaseClass;
    lpsClassName = lpsBuf;
  }

  return FALSE;
}

/* -- Instance tree methods -- */                                               /* ================================= */

/**
 * <p>Validates a pointer to an instance. Using this method an application may
 * determine if an instance has been destroyed an its pointer is no longer
 * valid.</p>
 *
 * <h4>Remarks</h4>
 * <ul>
 *   <li>All instances register themselves with the XAlloc heap management
 *     system executing the base classes' constructor
 *     {@link CDlpObject_Constructor} and unrigister themselves executing the
 *     base classes' destructor {@link CDlpObject_Destructor}.</li>
 * </ul>
 *
 * @param _this
 *          Pointer to be validated
 * @param nSerialNum
 *          Supposed serial number of that object (zero -> no check of serial num.)
 * @return <code>_this</code> id the pointer is valid, <code>NULL</code>
 *         otherwise
 */
CDlpObject* CDlpObject_CheckInstancePtr(CDlpObject* iInst, UINT64 nSerialNum)
{
  alloclist_t *li;
  if(!iInst) return NULL;                                                       /* Pointer is NULL -> invalid        */
  if(!(li = dlp_xalloc_find_object_ex(iInst)) || li->nType!='I') return NULL;   /* Find pointer in XAlloc list       */
  if(nSerialNum && nSerialNum!=iInst->m_nSerialNum) return NULL;                /* Check serial number        */
  return iInst;
}

/**
 * Returns the parent of this instance in the instance tree.
 *
 * @param _this
 *          Pointer to this instance
 * @return Pointer to the parent instance or <code>NULL</code> if the passed
 *         pointer is invalid or this instance is the root of the tree.
 */
CDlpObject* CDlpObject_GetParent(CDlpObject* _this)
{
  CHECK_THIS_RV(NULL);                                                          /* Check this pointer                */
  if (!_this->m_lpContainer) return NULL;                                       /* No container word -> no parent    */
  return _this->m_lpContainer->lpContainer;                                     /* Return owner of container word    */
}

/**
 * Returns the parent of this instance in the instance tree.
 *
 * @param _this
 *          Pointer to this instance
 * @return The pointer to the root instance.
 */
CDlpObject* CDlpObject_GetRoot(CDlpObject* _this)
{
  CDlpObject* iParent = NULL;                                                   /* Pointer to parent instance        */
  CHECK_THIS_RV(NULL);                                                          /* Check this pointer                */
  iParent = CDlpObject_GetParent(_this);                                        /* Get parent                        */
  if (!iParent) return _this;                                                   /* No parent -> this is root         */
  return CDlpObject_GetRoot(iParent);                                           /* Go ask parent                     */
}

/**
 * Gets the fully qualified name of this instance.
 *
 * @param _this  This instance
 * @param lpName A pointer to a buffer to store the result in
 * @return The buffer
 */

/* MWX 2002-08-28: FUNCTION BEHAVIOR CHANGED!!!
   Reason: FQ name must be the same whether or not an interpreter is running !!!
  (XML deserialization in stand-alone applciations) */

char* CDlpObject_GetFQName(CDlpObject* _this, char* lpName, BOOL bForceArray)
{
  char        lpBuf1[255];
  char        lpBuf2[255];
  SWord*      lpWord;
  CDlpObject* lpInst = _this;

  if (!_this)
  {
    dlp_strcpy(lpName,"(null)");
    return lpName;
  }

  lpBuf2[0]='\0';

  if (!_this->m_lpContainer) dlp_strcpy(lpName,_this->m_lpInstanceName);
  else
  {
    lpWord = _this->m_lpContainer;
    while (TRUE)
    {
      /* Do no include interpreter instance name */
      if (dlp_strcmp(lpWord->lpContainer->m_lpClassName,"itp")==0) break;

      if (lpWord->nWordType==WL_TYPE_FIELD &&
          lpWord->ex.fld.nType==T_INSTANCE &&
          (lpWord->ex.fld.nArrlen!=1 || bForceArray))
      {
        /* Seek instance in array */
        INT32 i = 0;

        for (i=0; i<lpWord->ex.fld.nArrlen; i++)
          if (((CDlpObject**)lpWord->lpData)[i] == lpInst)
            break;

        if (i<lpWord->ex.fld.nArrlen)
          sprintf(lpBuf1,".%s[%ld]%s",lpWord->lpName,(long)i,lpBuf2);
        else
          /* MWX 2002-01-16:
             This is an error actually, but what should be done here ???? --> */
          sprintf(lpBuf1,".%s%s",lpWord->lpName,lpBuf2);
          /* <-- */
      }
      else sprintf(lpBuf1,".%s%s",lpWord->lpName,lpBuf2);
      strcpy(lpBuf2,lpBuf1);

      lpInst = (CDlpObject*)lpWord->lpContainer;
      DLPASSERT(lpInst); /* Word must belong to an instance! */
      if (!lpInst->m_lpContainer) break; /* No parent --> stop */
      lpWord = lpInst->m_lpContainer;
    }
    if (lpBuf2[0] == '.') dlp_memmove(lpBuf2,&lpBuf2[1],dlp_strlen(lpBuf2));
    if (strlen(lpBuf2)>0)  sprintf(lpName,"%s.%s",lpInst->m_lpInstanceName,lpBuf2);
    else                  strcpy(lpName,lpInst->m_lpInstanceName);
  }

  return lpName;
}

/**
 * Finds an instance by name.
 *
 * @param lpsName The instance name to search
 * @return The instance or NULL if no instance was found
 */
CDlpObject* CDlpObject_FindInstance(CDlpObject* _this, const char* lpsName)
{
  SWord* lpWord = CDlpObject_MicFindWord(_this,lpsName);
  if (!lpWord) return NULL;
  if (lpWord->nWordType==WL_TYPE_INSTANCE) return (CDlpObject*)lpWord->lpData;
  if (lpWord->nWordType==WL_TYPE_FIELD && lpWord->ex.fld.nType==T_INSTANCE)
    return *(CDlpObject**)lpWord->lpData;
  return NULL;
}

/* == Error methods == */                                                       /* ================================= */

/**
 * Sets the error tolerance of the dLabPro error management system.
 *
 * @param nErrorLevel New error tolerance level
 * @return The previous error tolerance level
 */
INT16 CDlpObject_SetErrorLevel(INT16 nErrorLevel)
{
  INT16 nOldEl = __nElevel;
  __nElevel = nErrorLevel;
  return nOldEl;
}

/**
 * Gets the current tolerance of the dLabPro error management system.
 *
 * @return The current error tolerance level
 */
INT16 CDlpObject_GetErrorLevel()
{
  return __nElevel;
}

/**
 * Sets the error identifier for the error tracing mechanism. If the
 * spefied error occurs during a dLabPro session, the error management
 * system will interrupt the session through a debug assertion. This
 * feature is not available in release mode.
 *
 * @param lpsTraceError Identifier of error to be traced
 * @see CDlpObject_GetTraceError
 */
void CDlpObject_SetTraceError(const char* lpsTraceError)
{
  dlp_strncpy(__lpsTraceError,lpsTraceError,255);
}

/**
 * Retrieves the current error tracing identifier.
 *
 * @param lpsBuffer  A pointer to the character buffer to fill
 * @param nBufferLen The size of the committed buffer (in bytes)
 * @see CDlpObject_SetTraceError
 */
void CDlpObject_GetTraceError(char* lpsBuffer, INT16 nBufferLen)
{
  dlp_strncpy(lpsBuffer,__lpsTraceError,nBufferLen);
}

/**
 * Gets the number of errors occured since the session start.
 *
 * @return Number of errors occured
 */
INT32 CDlpObject_GetErrorCount()
{
  return __nErrors;
}

/**
 * Stores information on the mst recent dLabPro error.
 *
 * @param nError The error code
 * @param lpErrorInst The instance causing the error
 */
void CDlpObject_SetLastError(INT16 nError, CDlpObject* iErrorInst)
{
  __nLastError     = nError;
  __iLastErrorInst = iErrorInst;
}

/**
 * Stores source file name and line of last interpreter error.
 *
 * @param lpInFile Pointer to buffer containing the file name
 * @param nInLine  The line number
 */
void CDlpObject_SetErrorPos(const char* lpInFile, INT32 nInLine)
{
  dlp_strncpy(__lpInFile,lpInFile,L_PATH);
  __nInLine = nInLine;
}

/**
 * Retrieves source file name and line of last interpreter error.
 *
 * @param lpInFile
 *          Pointer to buffer to be filled with the file name. The buffer is
 *          expected to be at least <code>_L_PATH</code> bytes large.
 * @param lpInLine
 *          Pointer to a long to be filled with the line number.
 */
void CDlpObject_GetErrorPos(char* lpInFile, INT32* lpInLine)
{
  dlp_strncpy(lpInFile,__lpInFile,L_PATH);
  if (lpInLine) *lpInLine = __nInLine;
}

#define RETURN_ERR(A) {char __buf[255]; sprintf(__buf,"%s%d",lpsInstName,(int)(-A));  \
                       if (strcmp(__buf,__lpsTraceError)==0) DLPASSERT(FALSE); \
                       return A;}

/**
 * THIS FUNCTION SHOULD NOT BE CALLED DIRECTLY,use the macros ERRORMSG and
 * ERRORRET instead
 *
 * Prints an error or warning message.
 *
 * @param iInst       Instance in which the error occured (may be NULL)
 * @param lpsFilename Location of error (source file)
 * @param nLine       Location of error (line number in source file)
 * @param nErrorID    Numeric error identifier
 * @param ...         Up to three additional value depending on error message
 * @return The numeric error identifier
 */
INT16 CDlpObject_Error(CDlpObject* iInst, const char* lpsFilename, INT32 nLine, INT16 nErrorID, ...)
{
#ifdef _NO_DLP_ERR_
  if(!strncmp(iInst->m_lpClassName,"gmm",4) && nErrorID==-1003) return nErrorID; /* Ignore GMM_RANK */
  printf("[%s:%i:] dlabpro-error: %i\n",lpsFilename,nLine,nErrorID);
  return nErrorID;
#else

  SWord*  lpError              = NULL;
  char    lpsPre[L_NAMES]      = "error";
  char    lpsInstName[L_NAMES] = "root";
  char    lpsBuffer[255];
  void*   lpArg1               = NULL;
  void*   lpArg2               = NULL;
  void*   lpArg3               = NULL;
  INT32    nErrLine             = 0;
  char    lpsErrFile[L_PATH];
  va_list ap;

  if (nErrorID >= O_K) return O_K;               /* i.e. no error */

#ifndef __NORTTI
  /* Find Error */
  if (iInst && nErrorID>-10000)
  {
    dlp_errorcode2id(lpsBuffer,nErrorID);
    lpError = CDlpObject_FindWord(iInst,lpsBuffer,WL_TYPE_ERROR);

    /* HACK: Try old error identifiers - DELETE THIS --> */
    if (!lpError)
    {
      sprintf(lpsBuffer,"error%d",(int)(-nErrorID));
      lpError = CDlpObject_FindWord(iInst,lpsBuffer,WL_TYPE_ERROR);
    }
    /* <-- */
  }
#endif

  /* Set last-error value (only on real errors) */
  if (!lpError || lpError->nFlags <= EL_ERROR)
  {
    __iLastErrorInst = iInst;
    __nLastError     = nErrorID;
  }

  /* Check error level */
  if (lpError && lpError->nFlags > __nElevel) return nErrorID;

  /* Get items of the parameter list */
  va_start(ap,nErrorID);
  lpArg1 = va_arg(ap,void*);
  lpArg2 = va_arg(ap,void*);
  lpArg3 = va_arg(ap,void*);
  va_end(ap);

  /* Get in-script position */
  if (__nInLine>=0 && dlp_strlen(__lpInFile))
  {
    nErrLine = __nInLine;
    dlp_strcpy(lpsErrFile,__lpInFile);
  }
  else
  {
    nErrLine = nLine;
    dlp_strcpy(lpsErrFile,lpsFilename);
  }

  /* Print common, unspecific errors */
  switch (nErrorID)
  {
  case NOT_EXEC:
    if (__nElevel>0)
      dlp_error_message(lpsErrFile,nErrLine,"%s root%hd: Function failed.",lpsPre,(short)(-nErrorID));
    __nErrors++;
    RETURN_ERR(nErrorID);
  case ERR_GENERIC:
    if (__nElevel>0)
      dlp_error_message(lpsErrFile,nErrLine,"%s root%hd: %s",lpsPre,(short)(-nErrorID),lpArg1);
    __nErrors++;
    RETURN_ERR(nErrorID);
  case ERR_DEBUG:
    if (__nElevel>0)
      dlp_error_message(lpsErrFile,nErrLine,"%s-message: {%s} {%s}.",lpsPre,lpArg1,lpArg2);
    __nErrors++;
    RETURN_ERR(nErrorID);
  case ERR_REGWORD:
    if (__nElevel>0)
      dlp_error_message(lpsErrFile,nErrLine,"%s root%hd: Failed to create a word.",lpsPre,(short)(-nErrorID));
    __nErrors++;
    RETURN_ERR(nErrorID);
  case ERR_DOUBLEDEF:
    if (__nElevel>0)
      dlp_error_message(lpsErrFile,nErrLine,"%s root%hd: Identifier %s already defined.",lpsPre,(short)(-nErrorID),lpArg1);
    __nErrors++;
    RETURN_ERR(nErrorID);
  case ERR_NOTFIELD:
    if (__nElevel>0)
      dlp_error_message(lpsErrFile,nErrLine,"%s root%hd: '%s' is not a valid field.",lpsPre,(short)(-nErrorID),lpArg1);
    __nErrors++;
    RETURN_ERR(nErrorID);
  case ERR_BADARG:
    dlp_error_message(lpsErrFile,nErrLine,"%s root%hd: Argument %ld is not of type '%s'.",lpsPre,(short)(-nErrorID),lpArg1,lpArg2);
    __nErrors++;
    RETURN_ERR(nErrorID);
  case ERR_OBSOLETEID:
    if (__nElevel >= 2) /* This is just a warning */
    {
      dlp_error_message(lpsErrFile,nErrLine,"warning root%hd: '%s' is deprecated. Use '%s' instead.",(short)(-nErrorID),lpArg1,lpArg2);
      __nWarnings++;
    }
    __nLastError     = 0;
    __iLastErrorInst = NULL;
    RETURN_ERR(nErrorID);
  case ERR_ILLEGALQUALI:
    if (__nElevel>0)
      dlp_error_message(lpsErrFile,nErrLine,"%s root%hd: Illegal qualification '.%s' of instance '%s'.",lpsPre,(short)(-nErrorID),lpArg1,lpArg2);
    __nErrors++;
    RETURN_ERR(nErrorID);
  case ERR_NOMEM:
    if (__nElevel>0)
      dlp_error_message(lpsErrFile,nErrLine,"%s root%hd: No more memory available.",lpsPre,(short)(-nErrorID));
    __nErrors++;
    RETURN_ERR(nErrorID);
  case ERR_NULLINST:
    if (__nElevel >= 1) /* This is just a warning */
    {
      dlp_error_message(lpsErrFile,nErrLine,"%s root%hd: Instance '%s' is NULL, empty or invalid.",lpsPre,(short)(-nErrorID),lpArg1);
      __nWarnings++;
    }
    __nLastError     = 0;
    __iLastErrorInst = NULL;
    RETURN_ERR(nErrorID);
  case ERR_DN3:
    if (__nElevel>0)
      dlp_error_message(lpsErrFile,nErrLine,"%s root%hd: DNorm3/XML stream error.",lpsPre,(short)(-nErrorID),lpArg1);
    __nErrors++;
    RETURN_ERR(nErrorID);
  case ERR_NOTOFKIND:
    if (__nElevel>0)
      dlp_error_message(lpsErrFile,nErrLine,"%s root%hd: Instance '%s' is not of type '%s'.",lpsPre,(short)(-nErrorID),lpArg1,lpArg2);
    __nErrors++;
    RETURN_ERR(nErrorID);
  case ERR_NULLARG:
    if (__nElevel>0)
      dlp_error_message(lpsErrFile,nErrLine,"%s root%hd: Argument '%s' must not be NULL.",lpsPre,(short)(-nErrorID),lpArg1);
    __nErrors++;
    RETURN_ERR(nErrorID);
  case ERR_INVALARG:
    if (__nElevel>0)
      dlp_error_message(lpsErrFile,nErrLine,"%s root%hd: Argument '%s' is invalid.",lpsPre,(short)(-nErrorID),lpArg1);
    __nErrors++;
    RETURN_ERR(nErrorID);
  case ERR_FILEOPEN:
    if (__nElevel>0)
      dlp_error_message(lpsErrFile,nErrLine,"%s root%hd: Cannot find file '%s' or access it for %s.(%s:%ld)",lpsPre,(short)(-nErrorID),lpArg1,lpArg2,lpsFilename,(long)nLine);
    __nErrors++;
    RETURN_ERR(nErrorID);
  case ERR_CHDIR:
    if (__nElevel>0)
      dlp_error_message(lpsErrFile,nErrLine,"%s root%hd: Cannot change to or create folder '%s'.",lpsPre,(short)(-nErrorID),lpArg1);
    __nErrors++;
    RETURN_ERR(nErrorID);
  case ERR_GETCWD:
    if (__nElevel>0)
      dlp_error_message(lpsErrFile,nErrLine,"%s root%hd: Cannot get current folder.(%s:%ld)",lpsPre,(short)(-nErrorID),lpsFilename,(long)nLine);
    __nErrors++;
    RETURN_ERR(nErrorID);
  case ERR_FILECLOSE:
    if (__nElevel>0)
      dlp_error_message(lpsErrFile,nErrLine,"%s root%hd: Cannot close file '%s'.(%s:%ld)",lpsPre,(short)(-nErrorID),lpArg1,lpsFilename,(long)nLine);
    __nErrors++;
    RETURN_ERR(nErrorID);
  case ERR_NOTSUPPORTED:
    if (__nElevel>0)
      dlp_error_message(lpsErrFile,nErrLine,"%s root%hd: '%s' is not supported by your platform or executable.(%s:%ld)",lpsPre,(short)(-nErrorID),lpArg1,lpsFilename,(long)nLine);
    __nErrors++;
    RETURN_ERR(nErrorID);
  case ERR_SERIALIZE:
    if (__nElevel >= 2) /* This is just a warning */
    {
      dlp_error_message(lpsErrFile,nErrLine,"warning root%hd: Serialization of field '%s' failed.",(short)(-nErrorID),lpArg1);
      __nWarnings++;
    }
    __nLastError     = 0;
    __iLastErrorInst = NULL;
    RETURN_ERR(nErrorID);
  case ERR_DESERIALIZE:
    if (__nElevel >= 2) /* This is just a warning */
    {
      dlp_error_message(lpsErrFile,nErrLine,"warning root%hd: Deserialization of field '%s' failed.",(short)(-nErrorID),lpArg1);
      __nWarnings++;
    }
    __nLastError     = 0;
    __iLastErrorInst = NULL;
    RETURN_ERR(nErrorID);
  case ERR_STREAMOBJ:
    if (__nElevel >= 2) /* This is just a warning */
    {
      dlp_error_message(lpsErrFile,nErrLine,"warning root%hd: Object '%s' not found in stream.",(short)(-nErrorID),lpArg1);
      __nWarnings++;
    }
    __nLastError     = 0;
    __iLastErrorInst = NULL;
    RETURN_ERR(nErrorID);
  case ERR_CREATEINSTANCE:
    if (__nElevel>0)
      dlp_error_message(lpsErrFile,nErrLine,"%s root%hd: Cannot create instance of class %s.",lpsPre,(short)(-nErrorID),lpArg1);
    __nErrors++;
    RETURN_ERR(nErrorID);
  case ERR_BADPTR:
    if (__nElevel>0)
      dlp_error_message(lpsErrFile,nErrLine,"%s root%hd: Bad pointer 0x%08X %s %s.",lpsPre,(short)(-nErrorID),lpArg1,lpArg2,lpArg3);
    __nErrors++;
    RETURN_ERR(nErrorID);
  case ERR_INTERNAL:
    if (__nElevel>0)
      dlp_error_message(lpsErrFile,nErrLine,"%s root%hd: Internal error%s at %s(%ld).",lpsPre,(short)(-nErrorID),lpArg1,lpArg2,lpArg3);
    __nErrors++;
    RETURN_ERR(nErrorID);
  case ERR_EXCEPTION:
    if (__nElevel>0)
#ifdef __TMS
      dlp_error_message(lpsErrFile,nErrLine,"%s root%hd: Exception %d at %s(%d).",lpsPre,(short)(-nErrorID),lpArg1,lpArg2,lpArg3);
#else
      dlp_error_message(lpsErrFile,nErrLine,"%s root%hd: Exception %ld at %s(%ld).",lpsPre,(short)(-nErrorID),lpArg1,lpArg2,lpArg3);
#endif
    __nErrors++;
    RETURN_ERR(nErrorID);
  case ERR_ILLEGALMEMBERVAL:
    if (__nElevel >= 2) /* This is just a warning */
    {
      dlp_error_message(lpsErrFile,nErrLine,"warning root%hd: Member '%s' of instance '%s' has illegal value: %s",(short)(-nErrorID),lpArg1,lpArg2,lpArg3);
      __nWarnings++;
    }
    __nLastError     = 0;
    __iLastErrorInst = NULL;
    RETURN_ERR(nErrorID);
  case ERR_DANGEROUS:
    if (__nElevel >= 2) /* This is just a warning */
    {
      dlp_error_message(lpsErrFile,nErrLine,"warning root%hd: %s is dangerous. %s",(short)(-nErrorID),lpArg1,lpArg2);
      __nWarnings++;
    }
    __nLastError     = 0;
    __iLastErrorInst = NULL;
    RETURN_ERR(nErrorID);
  }

  strcpy(lpsInstName,"???");
  if (iInst) dlp_strcpy(lpsInstName,iInst->m_lpClassName);
  if (iInst && lpError)
  {
    if (lpError->nFlags == EL_FATAL   ) strcpy(lpsPre,"FATAL error");
    if (lpError->nFlags == EL_WARNING ) strcpy(lpsPre,"warning");
    if (lpError->nFlags == EL_WARNING2) strcpy(lpsPre,"warning");
    if (lpError->nFlags <= __nElevel)
    {
      if (lpError->lpComment) sprintf(lpsBuffer,lpError->lpComment,lpArg1,lpArg2,lpArg3);
      else                    sprintf(lpsBuffer,"An unknown error occured.");
      dlp_error_message(lpsErrFile,nErrLine,"%s %s%hd: %s",lpsPre,iInst->m_lpClassName,(short)(-nErrorID),lpsBuffer);
      if (lpError->nFlags <= EL_ERROR) __nErrors++;
      else                             __nWarnings++;
    }
    RETURN_ERR(nErrorID);
  }
  else
  {
#ifdef __NORTTI
    if (iInst) dlp_error_message(lpsErrFile,nErrLine," error %s%hd occured.",iInst->m_lpClassName,(short)(-nErrorID));
#else
    if (iInst) dlp_error_message(lpsErrFile,nErrLine," error %s%hd: An unknown error occured.",iInst->m_lpClassName,(short)(-nErrorID));
#endif
    else       dlp_error_message(lpsErrFile,nErrLine," error ???%hd: An unknown error occured.",(short)(-nErrorID));
    __nErrors++;
  }

  RETURN_ERR(nErrorID);
#endif
}

/**
 * Prints an error protocol of the running session and resets the error
 * and warning counters.
 */
void CDlpObject_ErrorLog()
{
  printf("\n%s pass - ",dlp_get_binary_name());

  if (__nErrors   != 1) printf("%ld errors, " ,(long)__nErrors  );
  else                  printf("1 error, ");
  if (__nWarnings != 1) printf("%ld warnings.",(long)__nWarnings);
  else                  printf("1 warning.");
  __nErrors   = 0;
  __nWarnings = 0;

  printf("\n");
}

/**
 * Returns next serial number
 */
UINT64 CDlpObject_GetNextSerialNum()
{
  if(!__nSerialNum) __nSerialNum++;
  return __nSerialNum++;
}

/* EOF */
