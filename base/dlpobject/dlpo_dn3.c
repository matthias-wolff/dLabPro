/* dLabPro class CDlpObject (object)
 * - DN3 serialization / deserialization
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
#include "dlp_table.h"
#include "dlp_object.h"
#include "dlp_config.h"

#ifndef __NODN3STREAM

/**
 * Generic serialization to DNorm3 stream.
 *
 * @param _this  This instance
 * @param lpiDst Destination stream
 * @return O_K if successful, an error code otherwise
 */
INT16 CDlpObject_Serialize(CDlpObject* _this, CDN3Stream* lpiDst)
{
  SWord*   lpWord;
  hnode_t* hn;
  hscan_t  hs;
  INT16    nRet;

  DEBUGMSG(-1,"CDlpObject_Serialize for '%s'",_this->m_lpInstanceName,0,0);
#ifdef __NORTTI
  return IERROR(_this,ERR_NOTSUPPORTED,"Serialization in __NORTTI mode",0,0);
#endif

  hash_scan_begin(&hs,_this->m_lpDictionary);
  while ((hn = hash_scan_next(&hs))!=NULL)
  {
    DLPASSERT((lpWord = (SWord*)hnode_get(hn))!=NULL); /* NULL entry in dictionary */

    /* Serialize fields */
    if (lpWord->nWordType == WL_TYPE_FIELD)
      if (!(lpWord->nFlags & FF_NOSAVE))
        IF_NOK(CDlpObject_SerializeField(_this,lpiDst,lpWord))
          return NOT_EXEC;

    /* Serialize nested instances */
    if (lpWord->nWordType == WL_TYPE_INSTANCE && lpWord->lpData)
    {
      IF_OK(CDN3Stream_AddType(lpiDst,lpWord->lpName,((CDlpObject*)lpWord->lpData)->m_lpClassName,T_INSTANCE,1))
      {
        CDN3Stream_EnterLevel(lpiDst,lpWord->lpName);
#ifdef __cplusplus
        nRet = ((CDlpObject*)lpWord->lpData)->Serialize(lpiDst);
#else
        nRet = ((CDlpObject*)lpWord->lpData)->Serialize((CDlpObject*)lpWord->lpData,lpiDst);
#endif
        IF_NOK(nRet) IERROR(_this,ERR_SERIALIZE,lpWord->lpName,0,0);
        CDN3Stream_LeaveLevel(lpiDst);
      }
      else IERROR(_this,ERR_SERIALIZE,lpWord->lpName,0,0);
    }
  }
  return O_K;
}

/**
 * Used by CDlpObject_Deserialze ONLY!
 */
INT16 CDlpObject_DeserializeInstanceWord(CDlpObject* _this, CDN3Stream* lpiSrc, INT32 nTIT, SWord* lpWord)
{
  INT16 nRet = O_K;
  lpiSrc->m_nRnr = (INT16)CDlpTable_Dfetch(lpiSrc->m_lpiTIT,nTIT,OF_RNR);
  lpiSrc->m_nKnr = (INT16)CDlpTable_Dfetch(lpiSrc->m_lpiTIT,nTIT,OF_KNR);
  CDN3Stream_EnterLevel(lpiSrc,lpWord->lpName);
#ifdef __cplusplus
  nRet = ((CDlpObject*)lpWord->lpData)->Deserialize(lpiSrc);
#else
  nRet = ((CDlpObject*)lpWord->lpData)->Deserialize((CDlpObject*)lpWord->lpData,lpiSrc);
#endif
  IF_NOK(nRet) IERROR(_this,ERR_DESERIALIZE,lpWord->lpName,0,0);
  CDN3Stream_LeaveLevel(lpiSrc);
  return nRet;
}

/**
 * Generic deserialization from DNorm3 stream.
 *
 * @param _this  This instance
 * @param lpiSrc Source stream
 * @return O_K if successful, an error code otherwise
 */
INT16 CDlpObject_Deserialize(CDlpObject* _this, CDN3Stream* lpiSrc)
{
  SWord*      lpWord;
  hnode_t*    hn;
  hscan_t     hs;
  INT32        nTIT;
  CDlpTable*  lpTIT      = NULL;
  const char* lpsObjName = NULL;
  const char* lpsObjType = NULL;

  DEBUGMSG(-1,"CDlpObject_Deserialize for '%s'",_this->m_lpInstanceName,0,0);
#ifdef __NORTTI
  return IERROR(_this,ERR_NOTSUPPORTED,"Deserialization in __NORTTI mode not available.",0,0);
#endif

  hash_scan_begin(&hs,_this->m_lpDictionary);
  while ((hn = hash_scan_next(&hs))!=NULL)
  {
    DLPASSERT((lpWord = (SWord*)hnode_get(hn))!=NULL); /* NULL entry in dictionary */

    /* Deserialize fields */
    if (lpWord->nWordType == WL_TYPE_FIELD)
      if (!(lpWord->nFlags & FF_NOSAVE))
        IF_NOK(CDlpObject_DeserializeField(_this,lpiSrc,lpWord))
          return NOT_EXEC;

    /* Deserialize present nested instances */
    if (lpWord->nWordType == WL_TYPE_INSTANCE && lpWord->lpData)
    {
      nTIT = CDN3Stream_LookupNameOnLevel(lpiSrc,lpWord->lpName);
      if (nTIT>=0) CDlpObject_DeserializeInstanceWord(_this,lpiSrc,nTIT,lpWord);
      else         IERROR(_this,ERR_DESERIALIZE,lpWord->lpName,0,0);
    }
  }

  /* Deserialize absent nested instances */                                     /* --------------------------------- */
  lpTIT = lpiSrc->m_lpiTIT;                                                     /* CDN3Stream's type info. table     */
  for (nTIT=0; nTIT<CDlpTable_GetNRecs(lpTIT); nTIT++)                          /* Loop over objects (inefficient!)  */
  {                                                                             /* >>                                */
    if (lpiSrc->m_nContainer!=(INT32)CDlpTable_Dfetch(lpTIT,nTIT,OF_CONTAINER)) /*   Object not in current container */
      continue;                                                                 /*     Forget it ...                 */
    if ((INT32)CDlpTable_Dfetch(lpTIT,nTIT,OF_NTYPE)!=T_INSTANCE)               /*   Object not an instance          */
      continue;                                                                 /*     Forget it ...                 */
    lpsObjName = (char*)CDlpTable_XAddr(lpTIT,nTIT,OF_NAME);                    /*   Get object's name               */
    lpsObjType = (char*)CDlpTable_XAddr(lpTIT,nTIT,OF_TYPENAME);                /*   Get object's type name          */
    if (CDlpObject_FindWord(_this,lpsObjName,WL_TYPE_DONTCARE))                 /*   Object present                  */
       continue;                                                                /*     Forget it ...                 */

    CDlpObject_Instantiate(_this,lpsObjType,lpsObjName,                         /*   Instantiate new object          */
      _this!=CDlpObject_GetRoot(_this));                                        /*   |                               */
    if (!(lpWord = CDlpObject_FindWord(_this,lpsObjName,WL_TYPE_DONTCARE)))     /*   Find created word, if not >>    */
    {                                                                           /*   >>                              */
      IERROR(_this,ERR_DESERIALIZE,lpsObjName,0,0);                             /*     Error message                 */
      continue;                                                                 /*     Forget it ...                 */
    }                                                                           /*   <<                              */
    CDlpObject_DeserializeInstanceWord(_this,lpiSrc,nTIT,lpWord);               /*   Deserialize it                  */
  }

  return O_K;
}

/**
 * Serialize a field in DNorm3 stream.
 *
 * @param _this  This instance
 * @param lpiDst Destination stream
 * @param lpWord Pointer to a SWord structure identifying the field to serialize
 * @return O_K if successful, an error code otherwise
 */
INT16 CDlpObject_SerializeField(CDlpObject* _this, CDN3Stream* lpiDst, SWord* lpWord)
{
  INT16 nType               = 0;
  INT16 nPType              = 0;
  INT32  nArrayLen           = 0;
  INT32  i                   = 0;
  char  lpTypeName[L_NAMES];
  char  lpBuf[L_NAMES];
  void* lpData              = NULL;

  /* Validation */
  if (!lpWord->lpData) return NOT_EXEC;

  /* Get field type information */
  nPType    = 0;
  lpData    = lpWord->lpData;
  nArrayLen = lpWord->ex.fld.nArrlen;
  nType     = lpWord->ex.fld.nType;
  dlp_strcpy(lpTypeName, lpWord->ex.fld.lpType);

  /* resolve pointer types */
  if(nType == T_PTR)
  {
    /* get pointer type (truncate '*' from type) */
    dlp_strncpy(lpBuf, lpTypeName, strlen(lpTypeName) - 1);
    nPType = dlp_get_type_code(lpBuf);

    /* Get size of field */
    switch(nPType)
    {
    case T_USHORT  :
    case T_SHORT   :
    case T_LONG    :
    case T_ULONG   :
    case T_FLOAT   :
    case T_DOUBLE  : nArrayLen = dlp_size(lpData) / dlp_get_type_size(nPType);
             nType = nPType;
             break;
    case T_INSTANCE: if(!dlp_strcmp(lpWord->ex.fld.lpType,"data"))
             {
              /*nArrayLen   = dlp_size(lpData) / sizeof();*/
              break;
             }
    case T_IGNORE:
      break;
    default:
      IERROR(_this,ERR_DN3,0,0,0);
      return O_K;
    }
  }

  /* handle string and text type */
  if (dlp_is_symbolic_type_code(nType))
  {
    if ((nType == T_TEXT)||(nType == T_STRING))
    {
      lpData = *((void**)lpWord->lpData);
      if(lpData) nArrayLen = dlp_strlen((char*)lpData)+1;
      else nArrayLen = 0;
    }
    else
    {
      nArrayLen = nType;
      nType = T_CHAR;
    }
  }

  /*DLPASSERT(nArrayLen);*/

  /* Add type of field to type information table */
  if(CDN3Stream_AddType(lpiDst,lpWord->lpName,lpTypeName,nType,nArrayLen)==NOT_EXEC)
  {
    IERROR(_this,ERR_DN3,0,0,0);
    return NOT_EXEC;
  }

  /* add primitive types to collectors */
  switch (nType)
  {

  case T_IGNORE : break;
  case T_CSTRING:
  case T_STRING :
  case T_TEXT   :
  case T_UCHAR  :
  case T_CHAR   :
  case T_USHORT :
  case T_SHORT  :
  case T_UINT   :
  case T_INT    :
  case T_ULONG  :
  case T_LONG   :
  case T_FLOAT  :
  case T_DOUBLE : CDN3Stream_WriteCollector(lpiDst,nType,lpData,nArrayLen); break;

  /* write instances */
  case T_PTR    : DLPASSERT(FALSE); break; /* This cannot happen, see above */
  case T_INSTANCE  :
  {
    INT16 nRet = O_K;
    /* if instance is not of type "data" then enter ... */
    BOOL bIsData = (!dlp_strncmp(lpWord->ex.fld.lpType,"data",dlp_strlen(lpWord->ex.fld.lpType)));
    if (!bIsData) CDN3Stream_EnterLevel(lpiDst, lpWord->lpName);

    for (i=0; i<nArrayLen; i++)
    {
      IFCHECK printf("\n SAVE RQ for INSTANCE(%s)[%ld] %s",lpTypeName,(long)i,lpWord->lpName);

      lpData = ((CDlpObject**)lpWord->lpData)[i];

      if(lpData == NULL)
      {
        /* NULL instances are registered using knr=-1 and rnr=0 */
        INT32 nTITIdx = CDlpTable_GetNRecs(lpiDst->m_lpiTIT)-1;
        CDlpTable_Dstore(lpiDst->m_lpiTIT,-1,nTITIdx,OF_KNR);
        CDlpTable_Dstore(lpiDst->m_lpiTIT,0,nTITIdx,OF_RNR);
      }
      else
      {
        /* Serialize instance */
#ifdef __cplusplus
        nRet = ((CDlpObject*)lpData)->Serialize(lpiDst);
#else
        nRet = ((CDlpObject*)lpData)->Serialize((CDlpObject*)lpData,lpiDst);
#endif
        if(nRet != O_K) IERROR(_this,ERR_SERIALIZE,lpWord->lpName,0,0);
      }
      lpiDst->m_nArrayIdx++;
    }

    /* ... and leave level */
    if (!bIsData) CDN3Stream_LeaveLevel(lpiDst);
    break;
  }

  default:
    IERROR(_this,ERR_SERIALIZE,lpWord->lpName,0,0);
    return O_K;
  }

  return O_K;
}

/**
 * Deserialize a field from DNorm3 stream.
 *
 * @param _this  This instance
 * @param lpiSrc Source stream
 * @param lpWord Pointer to a SWord structure identifying the field to deserialize
 * @return O_K if successful, an error code otherwise
 */
INT16 CDlpObject_DeserializeField(CDlpObject* _this, CDN3Stream* lpiSrc, SWord* lpWord)
{
  INT16  nType                = 0;
  INT32   i                    = 0;
  INT32   nTIT                 = 0;
  INT32   nColl                = 0;
  INT32   nArrayLen            = 0;
  INT32   nRecLen              = 0;
  char   lpsName[L_NAMES];
  char   lpsTypeName[L_NAMES];
  char   lpsIndent[L_NAMES];
  void*  lpData               = NULL;
  BYTE*  lpX                  = NULL;

  /* Validation */
  if (!lpWord->lpData) return NOT_EXEC;

  /* Lookup name in type information table */
  nTIT = CDN3Stream_LookupNameOnLevel(lpiSrc,lpWord->lpName);
  if (NOT_EXEC == nTIT)
  {
    nTIT = CDN3Stream_LookupNameOnLevel(lpiSrc,lpWord->lpObsname);
    if (nTIT == NOT_EXEC)
    {
      IERROR(_this,ERR_DESERIALIZE,lpWord->lpName,0,0);
      return O_K;
    }
    /* MWX 2002-12-04 HACK: Want obsolete field ID warning? -->
    ERRORMSG(ERR_DN3, 0, 0, 0);
    <-- */
    dlp_strcpy(lpsName,lpWord->lpObsname);
  }
  else strcpy(lpsName,lpWord->lpName);

  IFCHECK
  {
    lpsIndent[0]='\0';
    for (i=0; i<lpiSrc->m_nLevel; i++) strcat(lpsIndent,"  ");
  }

  /* Retrieve type information */
  lpData    = lpWord->lpData;
  nType     = lpWord->ex.fld.nType;
  nArrayLen = lpWord->ex.fld.nArrlen;

  /* Adjust this values for T_PTR types */
  if (nType == T_PTR)
  {
    /* Retrieve type pointed to */
    dlp_strcpy(lpsTypeName,lpWord->ex.fld.lpType);
    i = dlp_strlen(lpsTypeName)-1;
    if(lpsTypeName[i]=='*') lpsTypeName[i] = '\0';

    /* Try to get nType value for this */
    nType = dlp_get_type_code(lpsTypeName);
    if (nType == NOT_EXEC) IERROR(_this,ERR_DN3,0,0,0);
  }
  if (dlp_is_symbolic_type_code(nType))
  {
    if ((nType != T_TEXT)&&(nType != T_STRING)) nType = T_CHAR;
    nArrayLen = (INT32)CDlpTable_Dfetch(lpiSrc->m_lpiTIT,nTIT,OF_ARRLEN);
  }

  /* Get collector index (only for primitive types) */
  if(nType != T_INSTANCE) nColl = (INT32)CDlpTable_Dfetch(lpiSrc->m_lpiTIT,nTIT,OF_COLLIDX);

  switch (nType)
  {
    case T_CSTRING:
    case T_STRING :
    case T_TEXT   :
    {
      lpX     = CDlpTable_XAddr(lpiSrc->m_lpiCharCollector,nColl,0);
      DLPASSERT(lpX);
      if (*(char**)lpData)
      {
        dlp_free(*(char**)lpData);
        *(char**)lpData=NULL;
      }
      if (nArrayLen)
      {
        *(char**)lpData = (char*)dlp_calloc(nArrayLen+1,sizeof(char));
        dlp_strncpy(*(char**)lpData,(char*)lpX,nArrayLen);
      }
      break;
    }
    case T_UCHAR:
    case T_CHAR :
    {
      lpX     = CDlpTable_XAddr(lpiSrc->m_lpiCharCollector,nColl,0);
      nRecLen = CDlpTable_GetRecLen(lpiSrc->m_lpiCharCollector);
      DLPASSERT(lpX);
      for(i=0; i<nArrayLen; i++,lpX += nRecLen)
        dlp_memmove(&((INT8*)lpData)[i],lpX,sizeof(INT8));
      break;
    }
    case T_USHORT:
    case T_SHORT :
    {
      INT64 nBuf = 0;
      lpX     = CDlpTable_XAddr(lpiSrc->m_lpiLongCollector,nColl,0);
      nRecLen = CDlpTable_GetRecLen(lpiSrc->m_lpiLongCollector);
      DLPASSERT(lpX);
      for(i=0; i<nArrayLen; i++,lpX += nRecLen)
      {
        dlp_memmove(&nBuf,lpX,sizeof(INT64));
        ((INT16*)lpData)[i] = (INT16)nBuf;
      }
      break;
    }
    case T_ULONG:
    case T_LONG :
    {
      lpX     = CDlpTable_XAddr(lpiSrc->m_lpiLongCollector,nColl,0);
      nRecLen = CDlpTable_GetRecLen(lpiSrc->m_lpiLongCollector);
      DLPASSERT(lpX);
      for(i=0; i<nArrayLen; i++,lpX += nRecLen)
        dlp_memmove(&((INT64*) lpData)[i],lpX,sizeof(INT64));
      break;
    }
    case T_FLOAT:
    {
      FLOAT64 nBuf = 0;
      lpX     = CDlpTable_XAddr(lpiSrc->m_lpiDoubleCollector,nColl,0);
      nRecLen = CDlpTable_GetRecLen(lpiSrc->m_lpiDoubleCollector);
      DLPASSERT(lpX);
      for(i=0; i<nArrayLen; i++,lpX += nRecLen)
      {
        dlp_memmove(&nBuf,lpX,sizeof(FLOAT64));
        ((FLOAT32*) lpData)[i] = (FLOAT32)nBuf;
      }
      break;
    }
    case T_DOUBLE:
    {
      lpX     = CDlpTable_XAddr(lpiSrc->m_lpiDoubleCollector,nColl,0);
      nRecLen = CDlpTable_GetRecLen(lpiSrc->m_lpiDoubleCollector);
      DLPASSERT(lpX);
      for(i=0; i<nArrayLen; i++,lpX += nRecLen)
        dlp_memmove(&((FLOAT64*) lpData)[i],lpX,sizeof(FLOAT64));
      break;
    }

    /* Read instances */
    case T_PTR      : DLPASSERT(FALSE); break; /* This cannot happen, see above */
    case T_INSTANCE :
    {
      INT16 nRet = O_K;

      /* if instance is not of type "data" then enter ... */
      BOOL bIsData = (!dlp_strncmp(lpWord->ex.fld.lpType,"data",dlp_strlen(lpWord->ex.fld.lpType)));
      if (!bIsData) CDN3Stream_EnterLevel(lpiSrc, lpWord->lpName);

      /* look up knr and rnr of instance in file */
      lpiSrc->m_nRnr = (INT16)CDlpTable_Dfetch(lpiSrc->m_lpiTIT,nTIT,OF_RNR);
      lpiSrc->m_nKnr = (INT16)CDlpTable_Dfetch(lpiSrc->m_lpiTIT,nTIT,OF_KNR);

      for (i=0; i<nArrayLen; i++)
      {
        IFCHECK printf("\n %sRESTORE RQ for INSTANCE(%s)[%ld] %s",lpsIndent,lpWord->ex.fld.lpType,(long)i,lpWord->lpName);

        lpData = ((CDlpObject**)lpWord->lpData)[i];

        if (lpiSrc->m_nKnr == -1 && lpiSrc->m_nRnr == 0)
        {
          IFCHECK printf("\n %s- DN3 stream : found NULL-INSTANCE",lpsIndent);

          /* found NULL instance -> restore to NULL instance */
          if (lpData)
          {
            IFCHECK printf("\n %s- Destination: INSTANCE(%s)=0x%0p --> destroy",lpsIndent,lpWord->ex.fld.lpType,lpData);
            /* Destroy instance */
          #ifdef __cplusplus
            delete((CDlpObject*)lpData);
          #else
            ((CDlpObject*)lpData)->Destructor((CDlpObject*)lpData);
          #endif
          }
          lpData = NULL;
        }
        else
        {
          IFCHECK printf("\n %s- DN3 stream : found INSTANCE(%s)",lpsIndent,lpWord->ex.fld.lpType);

          /* Found empty data instance -> reset or create one */
          if(lpData)
          {
            IFCHECK printf("\n %s- Destination: INSTANCE(%s)=0x%0p --> reset",lpsIndent,lpWord->ex.fld.lpType,lpData);
            CDlpObject_Reset((CDlpObject*)lpData,TRUE);
          }
          /* Create instance by calling class factory */
          else
          {
            IFCHECK printf("\n %s- Destination: INSTANCE(%s)=NULL --> create: ",lpsIndent,lpWord->ex.fld.lpType,lpData);
            lpData = CDlpObject_CreateInstanceOf(lpWord->ex.fld.lpType,lpWord->lpName);
            lpData = BASEINST_WORDTYPE(lpData);
            if (!lpData)
            {
              IERROR(_this,ERR_CREATEINSTANCE,lpWord->lpName,0,0);
              lpData=NULL;
            }
            else
            {
              ((CDlpObject**)lpWord->lpData)[i] = (CDlpObject*)lpData;
              ((CDlpObject**)lpWord->lpData)[i]->m_lpContainer=lpWord;
              lpWord->ex.fld.nISerialNum = ((CDlpObject *)lpData)->m_nSerialNum;
            }
            IFCHECK printf("INSTANCE(%s)=0x%0p",lpWord->ex.fld.lpType,lpData);
          }


          if (lpiSrc->m_nKnr == 0 && lpiSrc->m_nRnr == 0 && bIsData)
          {
            IFCHECK printf("\n %s- DN3 stream : empty INSTANCE(data) --> leave it",lpsIndent);
          }
          else
          {
            IFCHECK printf("\n %s- DN3 stream : deserialize INSTANCE(%s)",lpsIndent,lpWord->ex.fld.lpType);
          #ifdef __cplusplus
            if (lpData) nRet = ((CDlpObject*)lpData)->Deserialize(lpiSrc);
          #else
            if (lpData) nRet = ((CDlpObject*)lpData)->Deserialize((CDlpObject*)lpData,lpiSrc);
          #endif
            if(nRet != O_K) IERROR(_this,ERR_DESERIALIZE,lpWord->lpName,0,0);
          }
        }

        lpiSrc->m_nArrayIdx++;
      }

      /* ... and leave level */
      if (!bIsData) CDN3Stream_LeaveLevel(lpiSrc);
      break;
    }

    case T_IGNORE:
      break;

    default:
      IERROR(_this,ERR_DESERIALIZE,lpWord->lpName,0,0);
      return O_K;
  }

  return O_K;
}

#else /* #ifndef __NODN3STREAM */
  #ifdef __MSVC
    #pragma warning( disable : 4100 )
  #endif /* #ifdef __MSVC */

INT16 CDlpObject_Serialize       (CDlpObject* _this, CDN3Stream* lpiDst               ) { return NOT_EXEC; }
INT16 CDlpObject_Deserialize     (CDlpObject* _this, CDN3Stream* lpiSrc               ) { return NOT_EXEC; }
INT16 CDlpObject_SerializeField  (CDlpObject* _this, CDN3Stream* lpiDst, SWord* lpWord) { return NOT_EXEC; }
INT16 CDlpObject_DeserializeField(CDlpObject* _this, CDN3Stream* lpiSrc, SWord* lpWord) { return NOT_EXEC; }

#endif /* #ifndef __NODN3STREAM */

#ifdef __cplusplus
INT16 CDlpObject::Serialize  (CDN3Stream* lpiDst) { return CDlpObject_Serialize  (this,lpiDst); }
INT16 CDlpObject::Deserialize(CDN3Stream* lpiSrc) { return CDlpObject_Deserialize(this,lpiSrc); }

#endif /* #ifdef __cplusplus */

/* EOF */

