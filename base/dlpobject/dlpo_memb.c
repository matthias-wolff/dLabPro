/* dLabPro class CDlpObject (object)
 * - Handling of class members (fields, options)
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

/* Disable some MSVC warnings */
#ifdef __MSVC
  #pragma warning( disable : 4100 ) /* Unreferenzierter formaler Parameter */
#endif

/**
 * Sets the value of the field associated with lpWord to the given value.
 * The value is committed in the argument list and has to be of the
 * field's type. No type validation, except instance type validation, is
 * performed.
 *
 * Setting a field takes place in three stages. First, the current value
 * of the field is saved and the new value is copied to the field. Second,
 * the field changed callback function (FCCF) is called. Third, if the
 * FCCF returns O_K the saved copy of the field's value is destroyed,
 * else the field is reset to the saved value and the new value is
 * destroyed (NOT for instance types!).
 *
 * @param _this  This instance
 * @param lpWord Dictionary entry of field to set
 * @param ...    New value (1 argument)
 * @return O_K if successful, a negative error code otherwise
 */
INT16 CDlpObject_SetField(CDlpObject* _this, SWord* lpWord,void* lpWhat)
{
  char*       txghost;
  void*       vdghost;
  char        sghost[255];
  CDlpObject* iNew;
  CDlpObject* iGhost;

  CHECK_THIS_RV(NOT_EXEC);

  if (!lpWord) return NOT_EXEC;
  if (lpWord->nWordType != WL_TYPE_FIELD)      return NOT_EXEC;
  if (lpWord->nFlags & (FF_HIDDEN | FF_NOSET)) return NOT_EXEC;
  if (!lpWord->lpData)                         return NOT_EXEC;
  DLPASSERT(lpWord->lpContainer == _this); /* Word does not belong to this instance */

  /* Do nothing if FF_NONAUTOMATIC */
  if (lpWord->nFlags & FF_NONAUTOMATIC)
  {
    /* Call field update handler */
    if (lpWord->ex.fld.lpfCallback)
      return INVOKE_CALLBACK(_this,lpWord->ex.fld.lpfCallback);
    return NOT_EXEC;
  }

  switch (lpWord->ex.fld.nType)
  {

  /*
     If the type in question is one that would normally be promoted, the pro-
       moted type should be used as the argument to va_arg().  The following de-
       scribes which types should be promoted (and to what):
       -   short is promoted to int
       -   float is promoted to double
       -   char is promoted to int */
  case T_BOOL   : DOFIELDUPDATE(     BOOL,lpWord,lpWhat); break;
  case T_UCHAR  : DOFIELDUPDATE(    UINT8,lpWord,lpWhat); break;
  case T_CHAR   : DOFIELDUPDATE(     INT8,lpWord,lpWhat); break;
  case T_USHORT : DOFIELDUPDATE(   UINT16,lpWord,lpWhat); break;
  case T_SHORT  : DOFIELDUPDATE(    INT16,lpWord,lpWhat); break;
  case T_UINT   : DOFIELDUPDATE(   UINT32,lpWord,lpWhat); break;
  case T_INT    : DOFIELDUPDATE(    INT32,lpWord,lpWhat); break;
  case T_ULONG  : DOFIELDUPDATE(   UINT64,lpWord,lpWhat); break;
  case T_LONG   : DOFIELDUPDATE(    INT64,lpWord,lpWhat); break;
  case T_FLOAT  : DOFIELDUPDATE(  FLOAT32,lpWord,lpWhat); break;
  case T_DOUBLE : DOFIELDUPDATE(  FLOAT64,lpWord,lpWhat); break;
  case T_COMPLEX: DOFIELDUPDATE(COMPLEX64,lpWord,lpWhat); break;
  case T_TEXT   :
  case T_STRING : {
    txghost = *(char**)lpWord->lpData;
    *(char**)lpWord->lpData = (char*)dlp_malloc(dlp_strlen(*(const char**)lpWhat)+1);
    dlp_strcpy(*(char**)lpWord->lpData,*(const char**)lpWhat);

    if (lpWord->ex.fld.lpfCallback != NULL &&
        NOK(INVOKE_CALLBACK(_this,lpWord->ex.fld.lpfCallback)))
    {
      dlp_free(*(char**)lpWord->lpData);
      *(char**)lpWord->lpData = txghost;
    }
    else dlp_free(txghost);
    break;}
  case T_PTR: {
    vdghost = *(void**)lpWord->lpData;
    *(void**)lpWord->lpData = lpWhat;

    if (lpWord->ex.fld.lpfCallback != NULL &&
        NOK(INVOKE_CALLBACK(_this,lpWord->ex.fld.lpfCallback)))
    {
      dlp_free(*(void**)lpWord->lpData);
      *(void**)lpWord->lpData = vdghost;
    }
    else dlp_free(vdghost);
    break;}
  case T_INSTANCE:
    iNew = *(CDlpObject**)lpWhat;
    CHECK_IPTR(iNew,0);
    /* Run-time type check */
    if
    (
      dlp_strlen(lpWord->ex.fld.lpType) >0                  &&
      iNew != NULL                                          &&
      CDlpObject_OfKind(lpWord->ex.fld.lpType,iNew) == NULL
    )
    {
      break;
    }

    /* Remove current attachment a create a ghost */
    CHECK_IPTR(*(CDlpObject**)lpWord->lpData,lpWord->ex.fld.nISerialNum);       /*   Check current instance pointer  */
    iGhost = *(CDlpObject**)lpWord->lpData;                                     /*   Save currently attached inst.   */
    *(CDlpObject**)lpWord->lpData = iNew;                                       /*   Attach new instance             */
    lpWord->ex.fld.nISerialNum = iNew->m_nSerialNum;                            /*   Copy serial number              */
    if (lpWord->ex.fld.lpfCallback != NULL &&                                   /*   Invoke field set callback fctn. */
        NOK(INVOKE_CALLBACK(_this,lpWord->ex.fld.lpfCallback)))                 /*   | ... refused new attachment    */
    {                                                                           /*   >>                              */
      *(CDlpObject**)lpWord->lpData = iGhost;                                   /*     Restore old attachment        */
      /* WARNING: A refused instance is not destroyed here!                                                          */
    }                                                                           /*   <<                              */
    else                                                                        /*   New attachment accepted         */
    {                                                                           /*   >>                              */
      if (iNew  ) iNew  ->m_nRC++;                                              /*     REMOVE TOGETHER WITH CItp     */
      if (iGhost) iGhost->m_nRC--;                                              /*     REMOVE TOGETHER WITH CItp     */
      if (CDlpObject_GetParent(iGhost)==_this)                                  /*     We are owner of old attachment*/
        IDESTROY(iGhost);                                                       /*       We (and only we) destroy it */
    }                                                                           /*   <<                              */

    break;
  }

  if (lpWord->ex.fld.nType >0 && lpWord->ex.fld.nType <=256)
  {
    dlp_strncpy(sghost,(char*)lpWord->lpData,lpWord->ex.fld.nType);
    dlp_strncpy((char*)lpWord->lpData,*(char**)lpWhat,lpWord->ex.fld.nType);

    if (lpWord->ex.fld.lpfCallback != NULL &&
        NOK(INVOKE_CALLBACK(_this,lpWord->ex.fld.lpfCallback)))
    {
      dlp_strncpy((char*)lpWord->lpData,sghost,lpWord->ex.fld.nType);
    }
  }

  return O_K;
}

INT16 CDlpObject_SetFieldNoWord(CDlpObject* _this,void *lpDst,INT32 nFlags,INT16 nType,const char *lpsTName,const char *lpsIName,...)
{
  va_list ap;
  if(nFlags & FF_NONAUTOMATIC) return O_K;
  if(!lpDst) return O_K;
  va_start(ap,lpsIName);
  switch(nType){
    case T_BOOL:    *(     BOOL*)lpDst = va_arg(ap,          int); break;
    case T_UCHAR:   *(    UINT8*)lpDst = va_arg(ap,unsigned  int); break;
    case T_CHAR:    *(     char*)lpDst = va_arg(ap,          int); break;
    case T_USHORT:  *(   UINT16*)lpDst = va_arg(ap,unsigned  int); break;
    case T_SHORT:   *(    INT16*)lpDst = va_arg(ap,          int); break;
    case T_UINT:    *(   UINT32*)lpDst = va_arg(ap,unsigned long); break;
    case T_INT:     *(    INT32*)lpDst = va_arg(ap,         long); break;
    case T_ULONG:   *(   UINT64*)lpDst = va_arg(ap,unsigned long); break;
    case T_LONG:    *(    INT64*)lpDst = va_arg(ap,         long); break;
    case T_FLOAT:   *(  FLOAT32*)lpDst = va_arg(ap,       double); break;
    case T_DOUBLE:  *(  FLOAT64*)lpDst = va_arg(ap,       double); break;
    case T_COMPLEX: *(COMPLEX64*)lpDst = va_arg(ap,    COMPLEX64); break;
    case T_PTR:     *(    void**)lpDst = va_arg(ap,        void*); break;
    case T_INSTANCE:
      if(nFlags & (FF_NOSET | FF_HIDDEN)) *(void **)lpDst=NULL;
      else *(CDlpObject**)lpDst = *(CDlpObject**)CDlpObject_CreateInstanceOf(lpsTName,lpsIName);
    break;
    case T_STRING:
    case T_TEXT:    *(char**)lpDst = va_arg(ap,char*); break;
    default:
      if(nType>0 && nType<=256) dlp_memmove(lpDst,va_arg(ap,char*),nType);
  }
  va_end(ap);
  return O_K;
}

/**
 * Resets the value of a field to its initial value.
 *
 * @param _this       This instance
 * @param lpWord      Dictionary entry of field to reset
 * @param bDestroying TRUE if the reset operation is part of the
 *                    destruction of this instance.
 * @return O_K if successful, a negative error code otherwise
 */
INT16 CDlpObject_ResetField(CDlpObject* _this, SWord* lpWord, BOOL bDestroying)
{
  CDlpObject* iInst   = NULL;                                                   /* Instance field data               */
  char*       lpsInit = NULL;                                                   /* String field init value           */

  /* Validate */                                                                /* --------------------------------- */
  CHECK_THIS_RV(NOT_EXEC);                                                      /* Check this pointer                */
  if (!lpWord                           ) return NOT_EXEC;                      /* No word, no service               */
  if (lpWord->nWordType != WL_TYPE_FIELD) return NOT_EXEC;                      /* No field word, no service         */
  if (!lpWord->lpData                   ) return NOT_EXEC;                      /* No data associated, no service    */
  DLPASSERT(lpWord->lpContainer==_this);                                        /* Not owner of word                 */
  DEBUGMSG(DM_KERNEL,"Reset %s.%s",                                             /* Kernel debugging message          */
    ((CDlpObject*)(lpWord->lpContainer))->m_lpInstanceName,lpWord->lpName,0);   /* |                                 */
  if (lpWord->nFlags & FF_NONAUTOMATIC) return O_K;                             /* Non-automatic field -> you care!  */

  /* Free memory associated with instance, string and pointer fields */         /* --------------------------------- */
  switch (lpWord->ex.fld.nType)                                                 /* Branch for field type             */
  {                                                                             /* >>                                */
    case T_INSTANCE:                                                            /*   Instance field                  */
      if (lpWord->nFlags&FF_NONAUTOMATIC) break;                                /*   Do nothing                      */
      CHECK_IPTR(*(CDlpObject**)lpWord->lpData,lpWord->ex.fld.nISerialNum);     /*     Check associated instance ptr.*/
      iInst = *(CDlpObject**)lpWord->lpData;                                    /*     Get associated instance       */
      if (iInst == NULL) break;                                                 /*     Not present, fine...          */
      if (CDlpObject_GetParent(iInst)!=_this)                                   /*     We do not own associated inst.*/
      {                                                                         /*     >>                            */
        if (iInst->m_nRC>0) iInst->m_nRC--;                                     /*       REMOVE TOGETHER WITH CItp   */
        *(CDlpObject**)lpWord->lpData = NULL;                                   /*       Just forget it!             */
      }                                                                         /*     <<                            */
      else                                                                      /*     We own associated instance    */
      {                                                                         /*     >> (We're responsible for it) */
        CDlpObject_Reset(iInst,1);                                              /*       Reset it                    */
        if (bDestroying)                                                        /*       On self destruction         */
        {                                                                       /*       >>                          */
#ifdef __cplusplus                                                                      /*         Destroy it                */
          delete *(CDlpObject**)lpWord->lpData; *(CDlpObject**)lpWord->lpData=NULL;
#else
          void *idInst = *(CDlpObject**)lpWord->lpData;
          if(strncmp(lpWord->ex.fld.lpType,"DlpObject",10)) idInst=((CDlpObject *)idInst)->m_lpDerivedInstance;
          (*(CDlpObject**)lpWord->lpData)->Destructor(*(CDlpObject**)lpWord->lpData);
          dlp_free(idInst);
          *(CDlpObject**)lpWord->lpData = NULL;                               /*         Forget it                 */
#endif
          lpWord->ex.fld.nISerialNum = 0;                                       /*         Reset serial number       */
        }                                                                       /*       <<                          */
      }                                                                         /*     <<                            */
      break;                                                                    /*     - - -                         */
    case T_CSTRING:                                                             /*   Constant string field           */
    case T_STRING:                                                              /*   String field                    */
    case T_TEXT  :                                                              /*   Also string field               */
    case T_PTR:                                                                 /*   Pointer field                   */
      dlp_free(*(void**)lpWord->lpData);                                        /*     Free buffer                   */
      break;                                                                    /*     - - -                         */
  }                                                                             /* <<                                */

  /* Init field with default value */                                           /* --------------------------------- */
  if (bDestroying) return O_K;                                                  /* Not on self destruction           */
  switch (lpWord->ex.fld.nType)                                                 /* Branch for field type             */
  {                                                                             /* >> (overlength for legibility)    */
    case T_BOOL   : *(     BOOL*)lpWord->lpData=lpWord->ex.fld.lpInit.n;  break;/* Boolean                           */
    case T_UCHAR  : *(    UINT8*)lpWord->lpData=lpWord->ex.fld.lpInit.n;  break;/* Unsigned char                     */
    case T_CHAR   : *(     INT8*)lpWord->lpData=lpWord->ex.fld.lpInit.n;  break;/* Signed char                       */
    case T_USHORT : *(   UINT16*)lpWord->lpData=lpWord->ex.fld.lpInit.n;  break;/* Unsigned short                    */
    case T_SHORT  : *(    INT16*)lpWord->lpData=lpWord->ex.fld.lpInit.n;  break;/* Signed short                      */
    case T_UINT   : *(   UINT32*)lpWord->lpData=lpWord->ex.fld.lpInit.n;  break;/* Unsigned int                      */
    case T_INT    : *(    INT32*)lpWord->lpData=lpWord->ex.fld.lpInit.n;  break;/* Signed int                        */
    case T_ULONG  : *(   UINT64*)lpWord->lpData=lpWord->ex.fld.lpInit.n;  break;/* Unsigned long                     */
    case T_LONG   : *(    INT64*)lpWord->lpData=lpWord->ex.fld.lpInit.n;  break;/* Signed long                       */
    case T_FLOAT  : *(  FLOAT32*)lpWord->lpData=lpWord->ex.fld.lpInit.c.x;break;/* Float                             */
    case T_DOUBLE : *(  FLOAT64*)lpWord->lpData=lpWord->ex.fld.lpInit.c.x;break;/* Double                            */
    case T_COMPLEX: *(COMPLEX64*)lpWord->lpData=lpWord->ex.fld.lpInit.c;  break;/* Complex                           */
    case T_PTR    : *(    void**)lpWord->lpData=lpWord->ex.fld.lpInit.p;  break;/* Pointer                           */
    case T_CSTRING :                                                            /* Constant string                   */
    case T_STRING  :                                                            /* String                            */
    case T_TEXT    :                                                            /* Also string                       */
      lpsInit = lpWord->ex.fld.lpInit.s;                                        /*   Get pointer to init value       */
      if (dlp_strlen(lpsInit)>0)                                                /*   Init value != ""                */
      {                                                                         /*   >>                              */
        *(char**)lpWord->lpData = (char*)dlp_calloc((dlp_strlen(lpsInit)+1),sizeof(char));/*     Allocate field string         */
        dlp_strcpy(*(char**)lpWord->lpData,lpsInit);                            /*     Copy init value to field      */
      }                                                                         /*   <<                              */
      break;                                                                    /*   - - -                           */
  }                                                                             /* <<                                */
  if (lpWord->ex.fld.nType>0 && lpWord->ex.fld.nType<=L_SSTR)                   /* Static string                     */
  {                                                                             /* >>                                */
    lpsInit = lpWord->ex.fld.lpInit.s;                                          /*   Get pointer init value          */
    dlp_strcpy((char*)lpWord->lpData,lpsInit);                                  /*   Copy to field                   */
  }                                                                             /* <<                                */

  return O_K;                                                                   /* All done                          */
}

/**
 * Resets all fields.
 *
 * @param _this This instance
 * @param bInit TRUE if function is called during the instanciation.
 * @return O_K if successful, a negative error code otherwise
 */
INT16 CDlpObject_ResetAllFields(CDlpObject* _this, BOOL bInit)
{
  hscan_t  hs;
  hnode_t* hn;
  SWord*   lpWord;

  CHECK_THIS_RV(NOT_EXEC);
#ifdef __NORTTI
  IERROR(_this,ERR_DANGEROUS,"CDlpObject_ResetAllFields","No fields will be reseted in __NORTTI mode.",0);
  DLPASSERT(FALSE);
  return NOT_EXEC;
#endif

  hash_scan_begin(&hs,_this->m_lpDictionary);
  while ((hn = hash_scan_next(&hs))!=NULL)
  {
    DLPASSERT((lpWord = (SWord*)hnode_get(hn))!=NULL); /* NULL entry in dictionary */
    if (lpWord->nWordType == WL_TYPE_FIELD) CDlpObject_ResetField(_this,lpWord,0);
    if (lpWord->nWordType == WL_TYPE_INSTANCE)
      if (lpWord->lpData)
        CDlpObject_Reset((CDlpObject*)lpWord->lpData,1);
  }

  return O_K;
}

/**
 * Converts the value of the field identified by lpWord to a string in the form
 * "fieldname=value".
 *
 * @param _this      This instance
 * @param lpsBuffer  String buffer to be filled
 * @param nBufferLen Maximum number of characters to be written (inclunding the terminal null)
 * @param lpWord     SWord structure identifying the field to print
 * @return O_K if successful, a negative error code otherwise
 */
INT16 CDlpObject_FieldToString(CDlpObject* _this, char* lpsBuffer, size_t nBufferLen, SWord* lpWord)
{
  /* TODO: What about using dlp_printx?? */
  if (!lpWord || lpWord->nWordType!=WL_TYPE_FIELD) return NOT_EXEC;
  if (!lpWord->lpData)                             return NOT_EXEC;
  if (!lpsBuffer)                                  return NOT_EXEC;
  lpsBuffer[0]=0;

  /* Check buffer length */
  if (dlp_is_numeric_type_code(lpWord->ex.fld.nType) && nBufferLen<dlp_strlen(lpWord->lpName)+16) return NOT_EXEC;

  /* Print value */
  switch (lpWord->ex.fld.nType)
  {
  case T_BOOL    : sprintf(lpsBuffer,"%s=%s",    lpWord->lpName,                (*(     BOOL*)lpWord->lpData==0)?"FALSE":"TRUE"); break;
  case T_UCHAR   : sprintf(lpsBuffer,"%s=%hu",   lpWord->lpName,(unsigned short) *(    UINT8*)lpWord->lpData); break;
  case T_CHAR    : sprintf(lpsBuffer,"%s=%hd",   lpWord->lpName,(short)          *(     INT8*)lpWord->lpData); break;
  case T_USHORT  : sprintf(lpsBuffer,"%s=%hu",   lpWord->lpName,(unsigned short) *(   UINT16*)lpWord->lpData); break;
  case T_SHORT   : sprintf(lpsBuffer,"%s=%hd",   lpWord->lpName,(short)          *(    INT16*)lpWord->lpData); break;
  case T_UINT    : sprintf(lpsBuffer,"%s=%u" ,   lpWord->lpName,(unsigned int)   *(   UINT32*)lpWord->lpData); break;
  case T_INT     : sprintf(lpsBuffer,"%s=%d" ,   lpWord->lpName,(int)            *(    INT32*)lpWord->lpData); break;
  case T_ULONG   : sprintf(lpsBuffer,"%s=%lu",   lpWord->lpName,(unsigned long)  *(   UINT64*)lpWord->lpData); break;
  case T_LONG    : sprintf(lpsBuffer,"%s=%ld",   lpWord->lpName,(long)           *(    INT64*)lpWord->lpData); break;
  case T_FLOAT   : sprintf(lpsBuffer,"%s=%g",    lpWord->lpName,(double)         *(  FLOAT32*)lpWord->lpData); break;
  case T_DOUBLE  : sprintf(lpsBuffer,"%s=%g",    lpWord->lpName,(double)         *(  FLOAT64*)lpWord->lpData); break;
  case T_COMPLEX : sprintf(lpsBuffer,"%s=%g+i%g",lpWord->lpName,(double)         ((COMPLEX64*)lpWord->lpData)->x, (double)((COMPLEX64*)lpWord->lpData)->y); break;
  default:
    if (lpWord->ex.fld.nType == T_STRING || lpWord->ex.fld.nType == T_TEXT ||
      (lpWord->ex.fld.nType > 0 && lpWord->ex.fld.nType < 256))
    {
      char* lpValue = (lpWord->ex.fld.nType<256) ? (char*)lpWord->lpData : *(char**)lpWord->lpData;

      if (!lpValue) sprintf(lpsBuffer,"%s=0" ,lpWord->lpName);
      else if (nBufferLen < dlp_strlen(lpWord->lpName)+3+dlp_strlen(lpValue))
      {
        /* Truncate string to fit in buffer */
        char* lpBuf = (char*)dlp_calloc(nBufferLen,sizeof(char));
        dlp_strncpy(lpBuf,lpValue,nBufferLen-4-dlp_strlen(lpWord->lpName));
        sprintf(lpsBuffer,"%s=\"%s\"",lpWord->lpName,lpBuf);
        dlp_free(lpBuf);
      }
      else sprintf(lpsBuffer,"%s=\"%s\"",lpWord->lpName,lpValue);
      return O_K;
    }
    else
    {
      DLPASSERT(FALSE);  /*Can only process primitive types and strings*/
      return NOT_EXEC;
    }
  }

  return O_K;
}

/**
 * Converts the value of the field identified by lpWord to a double value.
 *
 * @param _this    This instance
 * @param lpWord   SWord structure identifying the field to print
 * @param lpBuffer Pointer to a double to fill with result
 * @return O_K if successful, a negative error code otherwise
 */
INT16 CDlpObject_FieldToDouble(CDlpObject* _this, SWord* lpWord, FLOAT64* lpBuffer)
{
  CHECK_THIS_RV(NOT_EXEC);

  if (!lpWord)                            return NOT_EXEC;
  if (!lpBuffer)                          return NOT_EXEC;
  if (lpWord->nWordType != WL_TYPE_FIELD) return NOT_EXEC;
  if (!lpWord->lpData)                    return NOT_EXEC;
  DLPASSERT(lpWord->lpContainer == _this); /* Word does not belong to this instance */

  switch (lpWord->ex.fld.nType)
  {
  case T_BOOL    : *lpBuffer = (FLOAT64)*(   BOOL*)lpWord->lpData; return O_K;
  case T_UCHAR   : *lpBuffer = (FLOAT64)*(  UINT8*)lpWord->lpData; return O_K;
  case T_CHAR    : *lpBuffer = (FLOAT64)*(   INT8*)lpWord->lpData; return O_K;
  case T_USHORT  : *lpBuffer = (FLOAT64)*( UINT16*)lpWord->lpData; return O_K;
  case T_SHORT   : *lpBuffer = (FLOAT64)*(  INT16*)lpWord->lpData; return O_K;
  case T_UINT    : *lpBuffer = (FLOAT64)*( UINT32*)lpWord->lpData; return O_K;
  case T_INT     : *lpBuffer = (FLOAT64)*(  INT32*)lpWord->lpData; return O_K;
  case T_ULONG   : *lpBuffer = (FLOAT64)*( UINT64*)lpWord->lpData; return O_K;
  case T_LONG    : *lpBuffer = (FLOAT64)*(  INT64*)lpWord->lpData; return O_K;
  case T_FLOAT   : *lpBuffer = (FLOAT64)*(FLOAT32*)lpWord->lpData; return O_K;
  case T_DOUBLE  : *lpBuffer = (FLOAT64)*(FLOAT64*)lpWord->lpData; return O_K;
  case T_TEXT    :
  case T_CSTRING :
  case T_STRING  : return NOT_EXEC; /* TODO: Try to convert? */
  default        : return NOT_EXEC;
  }
}

/**
 * Initializes the value of the field identified by lpWord from a key/value pair
 * in the form "fieldname=value" in the source string lpsBuffer. The source string
 * may be composite, i.e. the field/value pairs may be enclosed in field delimiter
 * marks (C_FIELDDEL). If such field delimiter marks are found, all text outside
 * these marks will be ignored by CDlpObject_FieldFromString.
 *
 * @param _this     This instance
 * @param lpWord    SWord structure identifying the field to print
 * @param lpsBuffer Pointer to a string buffer
 * @return O_K if successful, a negative error code otherwise
 */
INT16 CDlpObject_FieldFromString(CDlpObject* _this, SWord* lpWord, const char* lpsBuffer)
{
  UINT32 i = 0;
  const char* tx  = NULL;
  const char* ty  = NULL;
  char* lpBuf     = NULL;
  char* lpBuf2    = NULL;
  char* lpEnd     = NULL;
  char* lpValue   = NULL;
  char  lpSeek[L_NAMES+10];
  BOOL  bInstring, bQuoted;

  /* Copy (part of) source to working buffer */
  if ((tx = strstr(lpsBuffer,C_FIELDDEL))!=NULL)
  {
    /* Composite string; has start and end marks of for section of serialized fields */
    /* Found start mark, now find end mark                                           */
    if ((ty = strstr(&tx[dlp_strlen(C_FIELDDEL)],C_FIELDDEL))==NULL) return NOT_EXEC;
    lpBuf = (char*)dlp_calloc(ty-tx-dlp_strlen(C_FIELDDEL)+1,sizeof(char));
    dlp_strncpy(lpBuf,&tx[dlp_strlen(C_FIELDDEL)],ty-tx-dlp_strlen(C_FIELDDEL));
    lpEnd = &lpBuf[ty-tx-dlp_strlen(C_FIELDDEL)];
  }
  else
  {
    lpBuf = (char*)dlp_malloc((dlp_strlen(lpsBuffer)+1)*sizeof(char));
    dlp_strcpy(lpBuf,lpsBuffer);
    lpEnd = &lpBuf[dlp_strlen(lpsBuffer)];
  }

  /* Seek key (i.e. field name plus '=') */
  sprintf(lpSeek,"%s=",lpWord->lpName);
  bInstring = FALSE;
  bQuoted   = FALSE;
  lpValue   = NULL;
  lpBuf2    = (char*)dlp_calloc(dlp_strlen(lpsBuffer)+1,sizeof(char));

  for (tx = lpBuf; tx<lpEnd && *tx!=0; tx++)
  {
    /* look for next delimiter outside a string */
    for(ty=tx; ty<lpEnd && *ty!=0; ty++)
    {
      if(*ty=='\\' && ty<lpEnd && *(ty+1)=='\\') {ty++; continue;} /* skip quoted backslashes  */
      if(*ty=='\\')                     bQuoted   = TRUE;        /* found unquoted backslash */
      if(*ty=='\"' && !bQuoted)         bInstring = !bInstring;  /* found unquoted '"'       */
      if(*ty==C_FIELDSEP && !bInstring) break;                   /* found delimiter          */
    }

    /* if delimiter was found, look for key */
    if(ty)
    {
      while(iswspace(*tx) && *tx) tx++;                          /* Skip white spaces before key */
      for(i=0; tx[i]==lpSeek[i] && i<dlp_strlen(lpSeek); i++) ;  /* compare string and key       */

      if(i == dlp_strlen(lpSeek))                                /* found key, set value         */
      {
        if((size_t)(ty-&tx[i]) > dlp_strlen(lpsBuffer)-1) break;
        dlp_strncpy(lpBuf2,&tx[i],ty-&tx[i]);
        lpValue = lpBuf2;
        break;
      }
      else tx = ty;                                              /* key not found -> continue search */

    }
    else break;            /* no more delimiters found */

    bQuoted = FALSE;
  }

  if(!lpValue) {dlp_free(lpBuf); dlp_free(lpBuf2); return NOT_EXEC;} /* Did not find key */

  /* Read value and set field */
  dlp_strtrimleft(lpValue);
  dlp_strtrimright(lpValue);
  dlp_strunquotate(lpValue,'\"','\"');

  switch (lpWord->ex.fld.nType)
  {
  case T_UCHAR   : { unsigned short v; sscanf(lpValue,"%hu",&v);    UINT8 nTmp=(  UINT8)v;CDlpObject_SetField(_this,lpWord,&nTmp); break; }
  case T_CHAR    : { short          v; sscanf(lpValue,"%hd",&v);     INT8 nTmp=(   INT8)v;CDlpObject_SetField(_this,lpWord,&nTmp); break; }
  case T_USHORT  : { unsigned short v; sscanf(lpValue,"%hu",&v);   UINT16 nTmp=( UINT16)v;CDlpObject_SetField(_this,lpWord,&nTmp); break; }
  case T_SHORT   : { short          v; sscanf(lpValue,"%hd",&v);    INT16 nTmp=(  INT16)v;CDlpObject_SetField(_this,lpWord,&nTmp); break; }
  case T_UINT    : { unsigned int   v; sscanf(lpValue,"%u" ,&v);   UINT32 nTmp=( UINT32)v;CDlpObject_SetField(_this,lpWord,&nTmp); break; }
  case T_INT     : { int            v; sscanf(lpValue,"%d" ,&v);    INT32 nTmp=(  INT32)v;CDlpObject_SetField(_this,lpWord,&nTmp); break; }
  case T_ULONG   : { unsigned long  v; sscanf(lpValue,"%lu",&v);   UINT64 nTmp=( UINT64)v;CDlpObject_SetField(_this,lpWord,&nTmp); break; }
  case T_LONG    : { long           v; sscanf(lpValue,"%ld",&v);    INT64 nTmp=(  INT64)v;CDlpObject_SetField(_this,lpWord,&nTmp); break; }
  case T_FLOAT   : { float          v; sscanf(lpValue,"%f" ,&v);  FLOAT32 nTmp=(FLOAT32)v;CDlpObject_SetField(_this,lpWord,&nTmp); break; }
  case T_DOUBLE  : { double         v; sscanf(lpValue,"%lf",&v);  FLOAT64 nTmp=(FLOAT64)v;CDlpObject_SetField(_this,lpWord,&nTmp); break; }
  case T_COMPLEX : { COMPLEX64      v; dlp_sscanc(lpValue  ,&v);                          CDlpObject_SetField(_this,lpWord,&v);    break; }

  default:
    if (lpWord->ex.fld.nType == T_STRING || lpWord->ex.fld.nType == T_TEXT ||
      (lpWord->ex.fld.nType > 0 && lpWord->ex.fld.nType < 256))
    {
      /* TODO: unquote string! */
      CDlpObject_SetField(_this,lpWord,&lpValue);
    }
    else
    {
      DLPASSERT(FALSE); /* Can only process primitive types and strings */
      dlp_free(lpBuf);
      return NOT_EXEC;
    }
  }

  /* Clean up */
  dlp_free(lpBuf);
  dlp_free(lpBuf2);

  return O_K;
}

/**
 * Sets an option.
 *
 * @param _this  This instance
 * @param lpWord Dictionary entry of option to set
 * @return O_K if successful, a negative error code otherwise
 */
INT16 CDlpObject_SetOption(CDlpObject* _this, SWord* lpWord)
{
  CHECK_THIS_RV(NOT_EXEC);

  if (!lpWord || lpWord->nWordType!=WL_TYPE_OPTION) return NOT_EXEC;
  if (!lpWord->lpData)                              return NOT_EXEC;
  DLPASSERT(lpWord->lpContainer == _this); /* Word does not belong to this instance */

  *(BOOL*)lpWord->lpData = TRUE;
    if (lpWord->ex.opt.lpfCallback)
    return INVOKE_CALLBACK(_this,lpWord->ex.opt.lpfCallback);

  return O_K;
}

/**
 * Copies all option values.
 *
 * @param _this This instance
 * @param iSrc  Source instance to copy
 * @return O_K if successful, an error code otherwise
 */
INT16 CDlpObject_CopyAllOptions(CDlpObject* _this, CDlpObject* iSrc)
{
  hscan_t  hs;
  hnode_t* hn;
  SWord*   lpWord = NULL;

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

#ifdef __cplusplus
  if(!iSrc->IsKindOf(_this->m_lpClassName))
#else
  if(!BASEINST(iSrc)->IsKindOf(BASEINST(iSrc),_this->m_lpClassName))
#endif
    return
      IERROR(_this,ERR_NOTOFKIND,iSrc->m_lpInstanceName,_this->m_lpClassName,0);

  /* Loop over own dictionary */
  hash_scan_begin(&hs,_this->m_lpDictionary);
  while ((hn = hash_scan_next(&hs))!=NULL)
  {
    DLPASSERT((lpWord = (SWord*)hnode_get(hn))!=NULL); /* NULL entry in dictionary */
    if (!lpWord) continue;
    if (lpWord->nWordType==WL_TYPE_OPTION)
    {
      SWord* lpwSrc = CDlpObject_FindWord(iSrc,lpWord->lpName,WL_TYPE_OPTION);
      if (lpwSrc)
      	*(BOOL*)lpWord->lpData = *(BOOL*)lpwSrc->lpData;
    }
  }
  _this->m_nCheck = iSrc->m_nCheck;

  return O_K;
}

/**
 * Resets all options.
 *
 * @param _this This instance
 * @param bInit TRUE if function is called during instanciation
 * @return O_K if successful, a negative error code otherwise
 */
/*
INT16 CDlpObject_ResetAllOptions(CDlpObject* _this, BOOL bInit)
{
  hscan_t  hs;
  hnode_t* hn;
  SWord*   lpWord;

  CHECK_THIS_RV(NOT_EXEC);

  hash_scan_begin(&hs,_this->m_lpDictionary);
  while ((hn = hash_scan_next(&hs))!=NULL)
  {
    DLPASSERT((lpWord = (SWord*)hnode_get(hn))!=NULL); / * NULL entry in dictionary * /

    if (lpWord->nWordType == WL_TYPE_OPTION && lpWord->lpData)
      if (!(lpWord->nFlags & OF_NONAUTOMATIC) || bInit)
      {
        DEBUGMSG(DM_KERNEL,"Reset option %s.%s",_this->m_lpInstanceName,lpWord->lpName,0);
          *(BOOL*)lpWord->lpData = FALSE;
      }
  }

  return O_K;
}
*/
/* EOF */
