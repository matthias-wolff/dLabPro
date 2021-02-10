/* dLabPro class CDlpObject (object)
 * - XML serialization / deserialization
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

#ifndef __NOXMLSTREAM

/* Macro - Read read field from XML stream */
#define __XML_READFIELD(A,B,C,D) \
  { \
    INT32 i=0; char* tx=NULL; \
    if (C->ex.fld.nType==T_PTR) \
      *(B**)C->lpData = (B*)dlp_realloc(*(B**)C->lpData,D->nArrlen,sizeof(B)); \
    for \
    ( \
      i=0,tx=strtok((char*)D->lpValue,","); \
      (i<D->nArrlen) && (i<C->ex.fld.nArrlen) && tx; \
      i++, tx=strtok(NULL,",") \
    ) \
    { \
      dlp_sscanx(tx,A,&((B*)C->lpData)[i]); \
      IFCHECK printf(i==0?" = ":", "); \
      IFCHECK dlp_printx(stdout,&((B*)C->lpData)[i],A,i,FALSE,FALSE); \
    } \
  }

char* xml_get_fqname(CDlpObject* _this, char* lpsFqName, const char* lpsRootName)
{
  INT32 nRootInameLen = 0;
  INT32 nInameLen     = 0;

  /* Get fully qualified name; trim root instance name */
  CDlpObject_GetFQName(_this,lpsFqName,TRUE);

  /* Cut root instance name from instance name */
  nRootInameLen=dlp_strlen(lpsRootName);
  nInameLen=dlp_strlen(lpsFqName);
  if (dlp_strncmp(lpsFqName,lpsRootName,nRootInameLen)) return NULL;
  dlp_memmove(lpsFqName,lpsFqName+nRootInameLen,nInameLen-nRootInameLen+1);
  return lpsFqName;
}

void xml_split_name(const char* lpsName, char* lpsPar, char* lpsObj)
{
  const char* x = NULL;
  INT32         n = dlp_strlen(lpsName);
  if (lpsPar) lpsPar[0]='\0';
  if (lpsObj) lpsObj[0]='\0';
  if (!n) return;
  for (x=&lpsName[n-1]; x>=lpsName; x--)
    if (*x=='.')
      break;
  dlp_memmove(lpsPar,lpsName,x-lpsName); lpsPar[x-lpsName]='\0';
  dlp_strcpy(lpsObj,x+1);
}

/**
 * Generic serialization to XML stream.
 *
 * @param _this  This instance
 * @param lpiDst Destination stream
 * @return O_K if successful, an error code otherwise
 */
INT16 CDlpObject_SerializeXml(CDlpObject* _this, CXmlStream* lpiDst)
{
  SWord*   lpWord;
  hnode_t* hn;
  hscan_t  hs;

  DEBUGMSG(-1,"CDlpObject_SerializeXml for '%s'",_this->m_lpInstanceName,0,0);
#ifdef __NORTTI
  return IERROR(_this,ERR_NOTSUPPORTED,"Serialization in __NORTTI mode",0,0);
#endif

  /* Generic instance serialization procedure: serialize all fields */
  hash_scan_begin(&hs,_this->m_lpDictionary);
  while ((hn = hash_scan_next(&hs))!=NULL)
  {
    DLPASSERT((lpWord = (SWord*)hnode_get(hn))!=NULL); /* NULL entry in dictionary */

    /* Serialize fields */
    if (lpWord->nWordType == WL_TYPE_FIELD)
      if (!(lpWord->nFlags & FF_NOSAVE))
        if (NOK(CDlpObject_SerializeFieldXml(_this,lpiDst,lpWord)))
          /* Error message!*/ DLPASSERT(FMSG("XML serialization failed"));

    /* Serialize nested instances */
    if (lpWord->nWordType == WL_TYPE_INSTANCE)
    {
      CDlpObject* lpInst            = NULL;
      char        lpFqNameI[L_SSTR];
      INT16       nRet              = O_K;

      lpInst = (CDlpObject*)lpWord->lpData;
      sprintf(lpFqNameI,"%s",CXmlStream_Encode(lpiDst,lpWord->lpName));

      if (lpInst)
      {
        lpiDst->m_nDepth--;
        CXmlStream_BeginInstance(lpiDst,lpFqNameI,lpInst->m_lpClassName);
#ifdef __cplusplus
        nRet = lpInst->SerializeXml(lpiDst);
#else
        nRet = lpInst->SerializeXml(lpInst,lpiDst);
#endif
        CXmlStream_EndInstance(lpiDst);
        lpiDst->m_nDepth++;
      }
      else nRet=NOT_EXEC;
      IF_NOK(nRet) IERROR(_this,ERR_SERIALIZE,lpWord->lpName,0,0);
    }
  }

  return O_K;
}

/**
 * Generic deserialization from XML stream.
 *
 * @param _this  This instance
 * @param lpiSrc Source stream
 * @return O_K if successful, an error code otherwise
 */
INT16 CDlpObject_DeserializeXml(CDlpObject* _this, CXmlStream* lpiSrc)
{
  SWord*      lpWord;
  SDomObject* lpiDo;
  CDlpObject* lpObj;
  hnode_t*    hn;
  hscan_t     hs;
  char        lpsFqName[255];
  char        lpsParName[255];
  char        lpsObjName[255];
  INT16       nRet = O_K;

  DEBUGMSG(-1,"CDlpObject_DeserializeXml for '%s'",_this->m_lpInstanceName,0,0);
#ifdef __NORTTI
  return IERROR(_this,ERR_NOTSUPPORTED,"Deserialization in __NORTTI mode",0,0);
#endif

  /* Set fully qualified name of root instance if blank */
  if(lpiSrc->m_lpsRootInameFq[0]=='\0') CDlpObject_GetFQName(BASEINST(_this),lpiSrc->m_lpsRootInameFq,TRUE);

  hash_scan_begin(&hs,_this->m_lpDictionary);
  while ((hn = hash_scan_next(&hs))!=NULL)
  {
    DLPASSERT((lpWord = (SWord*)hnode_get(hn))!=NULL); /* NULL entry in dictionary */

    /* Deserialize fields */
    if (lpWord->nWordType == WL_TYPE_FIELD)
      if (!(lpWord->nFlags & FF_NOSAVE))
        if (NOK(CDlpObject_DeserializeFieldXml(_this,lpiSrc,lpWord)))
          return NOT_EXEC;

    /* Deserialize present nested instances */
    if (lpWord->nWordType == WL_TYPE_INSTANCE)
    {
      lpObj = (CDlpObject*)lpWord->lpData;
#ifdef __cplusplus
      nRet = lpObj->DeserializeXml(lpiSrc);
#else
      nRet = lpObj->DeserializeXml(lpObj,lpiSrc);
#endif
      IF_NOK(nRet) IDESTROY(lpObj);
    }
  }

  /* Deserialize absent nested instances */                                     /* --------------------------------- */
  if (!xml_get_fqname(_this,lpsFqName,lpiSrc->m_lpsRootInameFq))                /* Get fully qualified name of _this */
  {                                                                             /* >> (failed)                       */
    IFCHECK printf(" *** Root instance name doesn't match *** ");               /*   Hard-wired error message        */
    IERROR(_this,ERR_DESERIALIZE,lpsFqName,0,0);                                /*   Error message                   */
    return O_K;                                                                 /*   Return O_K anyway               */
  }                                                                             /* <<                                */
  hash_scan_begin(&hs,lpiSrc->m_lpDom);                                         /* Initialize iteration              */
  while ((hn = hash_scan_next(&hs))!=NULL)                                      /* Loop over DOM objects             */
  {                                                                             /* >>                                */
    if (!(lpiDo = (SDomObject*)hnode_get(hn))) continue;                        /*   Object NULL -> should not happen*/
    if (lpiDo->nType != XMLS_DT_INSTANCE) continue;                             /*   Object not an instance          */
    if (strstr(lpiDo->lpsName,"[")) continue;                                   /*   HACK: instance in a field       */
    xml_split_name(lpiDo->lpsName,lpsParName,lpsObjName);                       /*   Split object and parent names   */
    if (dlp_strcmp(lpsParName,lpsFqName)!=0) continue;                          /*   Object not node of this instance*/
    if (!dlp_strlen(lpsObjName)) continue;                                      /*   Root node                       */
    if (CDlpObject_FindWord(_this,lpsObjName,WL_TYPE_DONTCARE)) continue;       /*   Object is not absent            */
    lpObj = CDlpObject_Instantiate(_this,lpiDo->lpsType,lpsObjName,             /*   Instantiate new object          */
      _this!=CDlpObject_GetRoot(_this));                                        /*   |                               */
    if (!(lpWord = CDlpObject_FindWord(_this,lpsObjName,WL_TYPE_DONTCARE)))     /*   Find created word, if not >>    */
    {                                                                           /*   >>                              */
      IERROR(_this,ERR_DESERIALIZE,lpsObjName,0,0);                             /*     Error message                 */
      continue;                                                                 /*     Forget it ...                 */
    }                                                                           /*   <<                              */
#ifdef __cplusplus
    nRet = lpObj->DeserializeXml(lpiSrc);                                       /*   Deserialize nested instance     */
#else
    nRet = lpObj->DeserializeXml(lpObj,lpiSrc);                                 /*   Deserialize nested instance     */
#endif
    IF_NOK(nRet) IDESTROY(lpObj);                                               /*   Destroy instance on error       */
  }                                                                             /* <<                                */

  return O_K;
}

/**
 * Serialize a field in XML stream.
 *
 * @param _this   This instance
 * @param lpiDst   Destination stream
 * @param lpWord  field to serialize
 * @return O_K if successful, an error code otherwise
 */
INT16 CDlpObject_SerializeFieldXml(CDlpObject* _this, CXmlStream* lpiDst, SWord* lpWord)
{
  INT32  i               = 0;
  INT16 nType           = 0;
  INT32  nArrlen         = 0;
  char  lpBuf[L_NAMES];
  char  lpType[L_NAMES];
  char  lpOutBuf[256];

  /* Validation */
  if (!lpWord) return NOT_EXEC;

  /* Resolve pointer types */
  nType   = lpWord->ex.fld.nType;
  nArrlen = lpWord->ex.fld.nArrlen;
  dlp_strcpy(lpType,lpWord->ex.fld.lpType);
  if (dlp_strlen(lpType)==0) dlp_strcpy(lpType,dlp_get_type_name(lpWord->ex.fld.nType));

  if(nType == T_PTR)
  {
    /* Get pointer type name (truncate '*' from type) */
    dlp_strncpy(lpBuf,lpWord->ex.fld.lpType,strlen(lpWord->ex.fld.lpType)-1);
    nType = dlp_get_type_code(lpBuf);

    /* Get size of field */
    switch(nType)
    {
    case T_UCHAR   :
    case T_CHAR    :
    case T_USHORT  :
    case T_SHORT   :
    case T_LONG    :
    case T_UINT    :
    case T_INT     :
    case T_ULONG   :
    case T_FLOAT   :
    case T_DOUBLE  :
    case T_COMPLEX :
      nArrlen = dlp_size(lpWord->lpData) / dlp_get_type_size(nType);
      break;
    case T_INSTANCE:
      nArrlen = dlp_size(lpWord->lpData) / sizeof(CDlpObject*);
      break;
    default        :
      IERROR(_this,ERR_DN3,0,0,0);
      return NOT_EXEC;
    }
  }

  /* Open field tag */
  XML_INDENT_LINE(lpiDst,lpiDst->m_nDepth);
  CXmlStream_Printf(lpiDst,"<FIELD name=\"%s\" type=\"%s\"",CXmlStream_Encode(lpiDst,lpWord->lpName),lpType);
  if (lpWord->ex.fld.nArrlen>1) CXmlStream_Printf(lpiDst," arrlen=\"%ld\"",(long)nArrlen);
  CXmlStream_Printf(lpiDst,">");

  /* Write field value(s) */
  switch (nType)
  {
  case T_BOOL    :
  case T_UCHAR   :
  case T_CHAR    :
  case T_USHORT  :
  case T_SHORT   :
  case T_UINT    :
  case T_INT     :
  case T_ULONG   :
  case T_LONG    :
  case T_FLOAT   :
  case T_DOUBLE  :
  case T_COMPLEX :
    for (i=0; i<nArrlen; i++)
    {
      if (i>0) CXmlStream_Printf(lpiDst,",");
      dlp_printx_ext(lpOutBuf,lpWord->lpData,nType,i,FALSE,TRUE,TRUE);
      CXmlStream_Printf(lpiDst,lpOutBuf);
    }
    break;
  case T_PTR     : DLPASSERT(FMSG("Unexpected pointer type")); break;
  case T_STRING  :
  case T_CSTRING :
  case T_TEXT    :
    dlp_printx_ext(lpOutBuf,(void*)CXmlStream_Encode(lpiDst,*(char**)lpWord->lpData),T_STRING,0,FALSE,TRUE,TRUE);
    CXmlStream_Printf(lpiDst,lpOutBuf);
    break;

  case T_INSTANCE:
    for (i=0; i<nArrlen; i++)
    {
      INT16 nRet = O_K;
      char  lpFqNameI[255];

      CDlpObject* lpInst = ((CDlpObject**)lpWord->lpData)[i];
      sprintf(lpFqNameI,"%s[%ld]",CXmlStream_Encode(lpiDst,lpWord->lpName),(long)i);

      if (lpInst)
      {
        CXmlStream_Printf(lpiDst,"\n");
        CXmlStream_BeginInstance(lpiDst,lpFqNameI,lpInst->m_lpClassName);
#ifdef __cplusplus
        nRet = lpInst->SerializeXml(lpiDst);
#else
        nRet = lpInst->SerializeXml(lpInst,lpiDst);
#endif
        CXmlStream_EndInstance(lpiDst);
        XML_INDENT_LINE(lpiDst,lpiDst->m_nDepth);
      }
      else
      {
        CXmlStream_Printf(lpiDst,"\n");
        XML_INDENT_LINE(lpiDst,lpiDst->m_nDepth+1);
        CXmlStream_Printf(lpiDst,"<INSTANCE name=\"%s\"/>\n",lpFqNameI);
        XML_INDENT_LINE(lpiDst,lpiDst->m_nDepth);
      }

      IF_NOK(nRet) IERROR(_this,ERR_SERIALIZE,lpWord->lpName,0,0);
    }
    break;

  case T_IGNORE:
    break;

  default:
    if (nType>=1 && nType<=L_SSTR)
    {
      char lpBuf[256];
      dlp_strncpy(lpBuf,(char*)lpWord->lpData,nType);
      dlp_printx_ext(lpOutBuf,(void*)CXmlStream_Encode(lpiDst,lpBuf),T_STRING,0,FALSE,TRUE,TRUE);
      CXmlStream_Printf(lpiDst,lpOutBuf);
    }
    else DLPASSERT(FMSG("Unexpected field type"));
  }

  /* Close field tag */
  CXmlStream_Printf(lpiDst,"</FIELD>\n");

  /* That's it */
  return O_K;
}

/**
 * Deserialize a field from XML stream.
 *
 * @cgen:TODO: Review for dLabPro 2.5 (fully qualified name for DOM lookup will not work!)
 * @param _this  This instance
 * @param lpiSrc   Source stream
 * @param lpWord field to deserialize
 * @return O_K if successful, an error code otherwise
 */
INT16 CDlpObject_DeserializeFieldXml(CDlpObject* _this, CXmlStream* lpiSrc, SWord* lpWord)
{
  INT16       nType           = -1;
  INT16       nDomType        = -1;
  INT32       nArrayLen       = 0;
  INT32       i               = 0;
  INT16       nRet            = O_K;
  void*       lpData          = NULL;
  SDomObject* lpDo            = NULL;
  char        lpFqName[255];
  char        lpDomName[255];
  char        lpBuf[L_NAMES];
  char        lpType[L_NAMES];

  /* Get fully qualified name; trim root instance name */
  if (!xml_get_fqname(_this,lpFqName,lpiSrc->m_lpsRootInameFq))
  {
    IFCHECK printf(" *** Root instance name doesn't match *** ");
    IERROR(_this,ERR_DESERIALIZE,lpFqName,0,0);
    return O_K;
  }

  /* Retrieve type information from word */
  lpData    = lpWord->lpData;
  nType     = lpWord->ex.fld.nType;
  nArrayLen = lpWord->ex.fld.nArrlen;
  dlp_strcpy(lpType,lpWord->ex.fld.lpType);
  if (dlp_strlen(lpType)==0) dlp_strcpy(lpType,dlp_get_type_name(lpWord->ex.fld.nType));

  if(nType == T_PTR)
  {
    /* Get pointer type name (truncate '*' from type) */
    dlp_strncpy(lpBuf,lpWord->ex.fld.lpType,strlen(lpWord->ex.fld.lpType)-1);
    nType = dlp_get_type_code(lpBuf);
  }
  if (nType<0)
  {
    IFCHECK printf(" *** INVALID FIELD TYPE NAME ***");
    return O_K;
  }

  /* Find word in DOM */
  sprintf(lpDomName,"%s.%s",lpFqName,lpWord->lpName);
  IFCHECK printf("\n   DFX %-20s",lpDomName);
  if ((lpDo = CXmlStream_FindObject(lpiSrc,lpDomName))==NULL)
  {
    IFCHECK printf(" *** NOT FOUND ***");
    CXmlStream_ObjectNotFound(lpiSrc);
    IERROR(_this,ERR_STREAMOBJ,lpDomName,0,0);
    return O_K;
  }

  nDomType = dlp_get_type_code(lpDo->lpsType);
  if (nDomType<0)
  {
    /* Define as instance type */
    nDomType = T_INSTANCE;
  }

  /* Match types */
  if (nDomType!=nType)
  {
    IFCHECK printf(" *** FIELD TYPE MISMATCH ***");
    return O_K;
  }

  /* Read data */
  switch (nType)
  {
    case T_BOOL    : __XML_READFIELD(nType,     BOOL,lpWord,lpDo); break;
    case T_UCHAR   : __XML_READFIELD(nType,    UINT8,lpWord,lpDo); break;
    case T_CHAR    : __XML_READFIELD(nType,     INT8,lpWord,lpDo); break;
    case T_USHORT  : __XML_READFIELD(nType,   UINT16,lpWord,lpDo); break;
    case T_SHORT   : __XML_READFIELD(nType,    INT16,lpWord,lpDo); break;
    case T_UINT    : __XML_READFIELD(nType,   UINT32,lpWord,lpDo); break;
    case T_INT     : __XML_READFIELD(nType,    INT32,lpWord,lpDo); break;
    case T_ULONG   : __XML_READFIELD(nType,   UINT64,lpWord,lpDo); break;
    case T_LONG    : __XML_READFIELD(nType,    INT64,lpWord,lpDo); break;
    case T_FLOAT   : __XML_READFIELD(nType,  FLOAT32,lpWord,lpDo); break;
    case T_DOUBLE  : __XML_READFIELD(nType,  FLOAT64,lpWord,lpDo); break;
    case T_COMPLEX : __XML_READFIELD(nType,COMPLEX64,lpWord,lpDo); break;
    case T_PTR     : DLPASSERT(FMSG("Unexpected pointer type")); break;
    case T_STRING  :
    case T_CSTRING :
    case T_TEXT    :
      if (dlp_strlen((char*)lpDo->lpValue)!=dlp_strlen(*(char**)lpWord->lpData))
        *(char**)lpWord->lpData = (char*)dlp_realloc(*(char**)lpWord->lpData,1,dlp_strlen((char*)lpDo->lpValue)+1);
      dlp_strcpy(*(char**)lpWord->lpData,(char*)lpDo->lpValue);
      IFCHECK printf(" = (0x%0p) %s",SCSTR(*(char**)lpWord->lpData),*(char**)lpWord->lpData);
      break;

    case T_INSTANCE:
    {
      for (i=0; i<nArrayLen; i++)
      {
        CDlpObject* lpInst = ((CDlpObject**)lpWord->lpData)[i];

        if (lpInst)
        {
  #ifdef __cplusplus
          nRet = lpInst->DeserializeXml(lpiSrc);
  #else
          nRet = lpInst->DeserializeXml(lpInst,lpiSrc);
  #endif
          IF_NOK(nRet) IDESTROY(lpInst);
        }
        else
        {
          char lpFqNameI[255];
          SDomObject* lpDo = NULL;

          sprintf(lpFqNameI,"%s.%s[%ld]",lpFqName,lpWord->lpName,(long)i);
          lpDo = CXmlStream_FindObject(lpiSrc,lpFqNameI);
          if(!lpDo && !i) {
            /* Bugfix: deserialize inner objects (arraylen=0) with existing container */ 
            lpDo = CXmlStream_FindObject(lpiSrc,lpDomName);
            lpWord->ex.fld.nArrlen=nArrayLen=0;
          }

          if (!lpDo) nRet = NOT_EXEC;
          else if (lpDo->nType!=XMLS_DT_NULLINST)
          {
            lpInst = CDlpObject_CreateInstanceOf(lpWord->ex.fld.lpType,lpWord->lpName);
            lpInst = BASEINST_WORDTYPE(lpInst);
            if (!lpInst)
            {
              IERROR(_this,ERR_CREATEINSTANCE,lpWord->lpName,0,0);
              lpInst = NULL;
            }
            else
            {
              ((CDlpObject**)lpWord->lpData)[i] = lpInst;
              ((CDlpObject**)lpWord->lpData)[i]->m_lpContainer = lpWord;
              lpWord->ex.fld.nISerialNum = lpInst->m_nSerialNum;
            }
            IFCHECK printf("INSTANCE(%s)=0x%0p",lpWord->ex.fld.lpType,lpData);
#ifdef __cplusplus
            nRet = lpInst->DeserializeXml(lpiSrc);
#else
            nRet = lpInst->DeserializeXml(lpInst,lpiSrc);
#endif
            IF_NOK(nRet) IDESTROY(lpInst);
          }
        }

        IF_NOK(nRet) IERROR(_this,ERR_DESERIALIZE,lpWord->lpName,0,0);
      }

      break;

    default:
      if (nType>=1 && nType<=L_SSTR)
      {
        dlp_memset((char*)lpWord->lpData,'\0',nType);
        dlp_strncpy((char*)lpWord->lpData,(char*)lpDo->lpValue,nType);
        IFCHECK printf(" = %s",(char*)lpWord->lpData);
      }
      else DLPASSERT(FMSG("Unexpected field type"));
    }
  }

  return O_K; /* Always hang on ... :)) */
}

#else /* #ifndef __NOXMLSTREAM */
  #ifdef __MSVC
    #pragma warning( disable : 4100 )
  #endif /* #ifdef __MSVC */

  INT16 CDlpObject_SerializeXml       (CDlpObject* _this, struct CXmlStream* lpiDst               ) { return NOT_EXEC; }
  INT16 CDlpObject_DeserializeXml     (CDlpObject* _this, struct CXmlStream* lpiSrc               ) { return NOT_EXEC; }
  INT16 CDlpObject_SerializeFieldXml  (CDlpObject* _this, struct CXmlStream* lpiDst, SWord* lpWord) { return NOT_EXEC; }
  INT16 CDlpObject_DeserializeFieldXml(CDlpObject* _this, struct CXmlStream* lpiSrc, SWord* lpWord) { return NOT_EXEC; }

#endif /* #ifndef __NOXMLSTREAM */

#ifdef __cplusplus
  INT16 CDlpObject::SerializeXml  (CXmlStream* lpiDst) { return CDlpObject_SerializeXml  (this,lpiDst); }
  INT16 CDlpObject::DeserializeXml(CXmlStream* lpiSrc) { return CDlpObject_DeserializeXml(this,lpiSrc); }

#endif /* #ifdef __cplusplus */

/* EOF */
