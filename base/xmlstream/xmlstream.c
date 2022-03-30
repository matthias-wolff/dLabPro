/* dLabPro XML stream library
 * - Implementation file
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

/* COPYRIGHT AND PERMISSION OF EXPAT LIBRARY:
 *
 * Copyright (c) 1998, 1999, 2000 Thai Open Source Software Center Ltd
 *                                and Clark Cooper
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 */

#include "dlp_cscope.h"
#include "dlp_kernel.h"
#include "dlp_xmlstream.h"

/* Error messages */
static char CXmlStream_Errors[][100] =
{
  "No more memory available.",
  "Cannot read from file '%s'.",
  "In file %s(%d) at position %d:",
  "Parse error - %s.",
  "Unexpected tag '%s'.",
  "Misplaced %s.",
  "Missing attribute '%s' in tag '%s'.",
  "Internal error. Reason: %s.",
  "Type %s invalid or not expected."
};

/* Private macros */
#define ERRORMSG(A,B,C,D)    { char lpFile[L_PATH]; dlp_splitpath(__FILE__,NULL,lpFile); \
                               printf("\n%s(%d): error CXmlStream%ld: ",lpFile,__LINE__,-(long)(A)); \
                               printf(CXmlStream_Errors[-(A)-11000],B,C,D); }
#define IFCHECK              if (FALSE)
#define CONVERT_FMODE(nMode) (((nMode)&XMLS_READ) ? (((nMode)&XMLS_ZIPPED)?"zrb":"rb") : (((nMode)&XMLS_ZIPPED)?"zwb":"w"))

/**
 *
 */
BOOL CXmlStream_CheckIsXml(const char* lpsFilename, const int nMode)
{
  DLP_FILE* lpFile;                                                             /* Temporary file descriptor         */
  char buf[5];                                                                  /* Read input buffer                 */
  if(!(nMode&XMLS_READ))                                                        /* If not in read-mode               */
  {                                                                             /* >>                                */
    /* This is not sensitive to /noerror!
    char lpError[256];snprintf(lpError,255,"%s (not in read-mode)",lpsFilename);/ *   Generate Error message          * /
    ERRORMSG(XMLSERR_READ,lpError,0,0);                                         / *   Error                           */
    return FALSE;                                                               /*   no check possible               */
  }                                                                             /* <<                                */
  lpFile = dlp_fopen(lpsFilename,CONVERT_FMODE(nMode));                         /* Open file with converted mode     */
  if(!lpFile)                                                                   /* File not opened?                  */
  {                                                                             /* >>                                */
    /* This is not sensitive to /noerror!
    ERRORMSG(XMLSERR_READ,lpsFilename,0,0);                                     / *   Error                           */
    return FALSE;                                                               /*   no check possible               */
  }                                                                             /* <<                                */
  dlp_fread(buf,sizeof(char),5,lpFile);                                         /* Get first 5 bytes from file       */
  dlp_fclose(lpFile);                                                           /* Close file                        */
  return !strncmp(buf,"<?xml",5);                                               /* Check if buffer match xml-head    */
}

/**
 *
 */
CXmlStream* CXmlStream_CreateInstance(const char* lpsFilename, const int nMode)
{
  CXmlStream* lpXmlStream = (CXmlStream*)__dlp_calloc(1,sizeof(CXmlStream),__FILE__,__LINE__,"CXmlStream",NULL);

  if (!lpXmlStream) return NULL;

  IF_OK(CXmlStream_Constructor(lpXmlStream,lpsFilename,nMode)) return lpXmlStream;

  dlp_free(lpXmlStream);
  return NULL;
}

short CXmlStream_SetBuffer(CXmlStream* _this,void *buf,size_t si){
  if (!(_this->m_nMode & XMLS_READ)) return NOT_EXEC;

  /* Create parser */
  _this->m_lpiParser = XML_ParserCreate(NULL);
  if (!_this->m_lpiParser){ CXmlStream_Destructor(_this); return XMLSERR_NOMEM; }

  /* Parametrize parser */
  XML_SetUserData(_this->m_lpiParser,_this);
  XML_SetElementHandler(_this->m_lpiParser,CXmlStream_OnOpenTag,CXmlStream_OnCloseTag);
  XML_SetCharacterDataHandler(_this->m_lpiParser,CXmlStream_OnText);

  /* Create DOM */
  _this->m_lpDom = hash_create(HASHCOUNT_T_MAX,0,0,NULL);
  if (!_this->m_lpDom){ CXmlStream_Destructor(_this); return XMLSERR_NOMEM; }

  while(si){
    size_t li=MIN(si,1<<20);
    BOOL bDone=li==si;
    /* Parse Document */
    if (!XML_Parse(_this->m_lpiParser,buf,li,(int)bDone)) {
      ERRORMSG(XMLSERR_INFILE,_this->m_lpsFileName,XML_GetCurrentLineNumber(_this->m_lpiParser),XML_GetCurrentColumnNumber(_this->m_lpiParser));
      ERRORMSG(XMLSERR_PARSE,XML_ErrorString(XML_GetErrorCode(_this->m_lpiParser)),0,0);

      CXmlStream_Destructor(_this);
      return XMLSERR_PARSE;
    }
    si-=li;
    buf+=li;
  }

  return O_K;
}

short CXmlStream_GetBuffer(CXmlStream* _this,void **buf,size_t *si){
  if(buf) *buf=_this->m_lpBuf;
  if(si)  *si=_this->m_nBufPos;
  return O_K;
}

#ifndef DLP_PRINTF_BUFSIZE
#  define DLP_PRINTF_BUFSIZE 4096
#endif

INT32 CXmlStream_Printf(CXmlStream* _this,const char* format, ...){
   va_list va;
   size_t len;
   char buf[DLP_PRINTF_BUFSIZE];
   va_start(va,format);
#ifdef HAS_vsnprintf
   (void)vsnprintf(buf, sizeof(buf), format, va);
#else
   (void)vsprintf(buf, format, va);
#endif
   va_end(va);
   len=strlen(buf);
   if(len<=0) return 0;

   if(_this->m_lpFile) return dlp_fwrite(buf,len,1,_this->m_lpFile);

   while(_this->m_nBufSi-_this->m_nBufPos<len)
     if(!(_this->m_lpBuf=realloc(_this->m_lpBuf,_this->m_nBufSi+=4096))) return 0;

   memcpy(_this->m_lpBuf+_this->m_nBufPos,buf,len);
   _this->m_nBufPos+=len;
   
   return len;
}

/**
 *
 */
short CXmlStream_Constructor(CXmlStream* _this, const char* lpsFilename, const int nMode)
{
  char bBuffer             = lpsFilename==NULL;
  _this->m_lpFile          = lpsFilename ? dlp_fopen(lpsFilename,CONVERT_FMODE(nMode)) : NULL;
  _this->m_nDepth          = 0;
  _this->m_nMode           = nMode;
  _this->m_lpCurObject     = NULL;
  _this->m_lpsBuffer       = NULL;
  _this->m_lpsXmlText      = NULL;
  _this->m_nFieldsNotFound = 0;
  _this->m_lpsRootInameFq[0] = '\0';

  dlp_memset(_this->m_lpsInameFq,0,255);
  dlp_strcpy(_this->m_lpsFileName,lpsFilename ? lpsFilename : "Buffer");

  /* Verify file */
  if (!bBuffer && !_this->m_lpFile)
  {
    CXmlStream_Destructor(_this);
    return -1;
  }

  _this->m_lpBuf=NULL;
  _this->m_nBufSi=_this->m_nBufPos=0;

  /* Initialize stream for reading */
  if (!bBuffer && (_this->m_nMode & XMLS_READ))
  {
    /* Create parser */
    _this->m_lpiParser = XML_ParserCreate(NULL);
    if (!_this->m_lpiParser)
    {
      CXmlStream_Destructor(_this);
      return XMLSERR_NOMEM;
    }

    /* Parametrize parser */
    XML_SetUserData(_this->m_lpiParser,_this);
    XML_SetElementHandler(_this->m_lpiParser,CXmlStream_OnOpenTag,CXmlStream_OnCloseTag);
    XML_SetCharacterDataHandler(_this->m_lpiParser,CXmlStream_OnText);

    /* Create DOM */
    _this->m_lpDom = hash_create(HASHCOUNT_T_MAX,0,0,NULL);
    if (!_this->m_lpDom)
    {
      CXmlStream_Destructor(_this);
      return XMLSERR_NOMEM;
    }

    /* Parse Document */
    _this->m_lpsBuffer = (char*)__dlp_calloc(XMLS_BUFFERLEN,sizeof(char),__FILE__,__LINE__,"CXmlStream",NULL);
    for (;;)
    {
      BOOL bDone;
      long nLen;

      nLen = dlp_fread(_this->m_lpsBuffer,1,XMLS_BUFFERLEN,_this->m_lpFile);
      if (dlp_ferror(_this->m_lpFile))
      {
        CXmlStream_Destructor(_this);
        return XMLSERR_READ;
      }
      bDone = dlp_feof(_this->m_lpFile);

      if (!XML_Parse(_this->m_lpiParser,_this->m_lpsBuffer,nLen,(int)bDone))
      {
        ERRORMSG(XMLSERR_INFILE,_this->m_lpsFileName,XML_GetCurrentLineNumber(_this->m_lpiParser),XML_GetCurrentColumnNumber(_this->m_lpiParser));
        ERRORMSG(XMLSERR_PARSE,XML_ErrorString(XML_GetErrorCode(_this->m_lpiParser)),0,0);

        CXmlStream_Destructor(_this);
        return XMLSERR_PARSE;
      }

      if (bDone) break;
    }

    /* Print DOM * /
    printf("\n"); dlp_fprint_x_line(stdout,'-',80);
    hnode_t* hn;
    hscan_t  hs;
    SDomObject* iDo;
    hash_scan_begin(&hs,_this->m_lpDom);
    while ((hn = hash_scan_next(&hs))!=NULL)
    {
      iDo = (SDomObject*)hnode_get(hn);
      if (!iDo) continue;
      printf("\n%d %s %s",iDo->nType,iDo->lpsType,iDo->lpsName);
    }
    printf("\n"); dlp_fprint_x_line(stdout,'-',80); printf("\n");*/
  }

  /* Initialize stream for writing */
  else if (_this->m_nMode & XMLS_WRITE)
  {
    CXmlStream_Printf(_this,"<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n");
  }

  return O_K;
}

/**
 *
 */
short CXmlStream_DestroyInstance(CXmlStream* _this)
{
  if(!_this) return NOT_EXEC;

  CXmlStream_Destructor(_this);
  dlp_free(_this);

  return O_K;
}

/**
 *
 */
short CXmlStream_Destructor(CXmlStream* _this)
{
  if (_this->m_lpFile   ) dlp_fclose(_this->m_lpFile);
  if (_this->m_lpsBuffer ) dlp_free(_this->m_lpsBuffer);
  if (_this->m_lpsXmlText) dlp_free(_this->m_lpsXmlText);
  if (_this->m_lpiParser  ) XML_ParserFree(_this->m_lpiParser);

  if (_this->m_lpDom)
  {
    hscan_t     hs;
    hnode_t*    hn;
    SDomObject* lpDo;

    while (!hash_isempty(_this->m_lpDom))
    {
      hash_scan_begin(&hs,_this->m_lpDom);
      if ((hn = hash_scan_next(&hs))!=NULL)
      {
        lpDo = (SDomObject*)hnode_get(hn);

        if (lpDo->lpValue)
          switch (lpDo->nType)
          {
          case XMLS_DT_FIELD:
            dlp_free(lpDo->lpValue);
            lpDo->lpValue=NULL;
            break;
          case XMLS_DT_TABLE:
            CDlpTable_DestroyInstance((CDlpTable*)lpDo->lpValue);
            lpDo->lpValue=NULL;
            break;
          }

        DLPASSERT(lpDo->lpValue==NULL); /* Value not freed!! */

        hash_scan_delfree(_this->m_lpDom,hn);
        dlp_free(lpDo);
      }
    }
    hash_destroy(_this->m_lpDom);
  }

  return O_K;
}

/**
 * Expat event handler for opening XML tags.
 */
void CXmlStream_OnOpenTag(void* __this, const char* lpsElement, const char** lpAttr)
{
  long        nAttr     = 0;
  const char* lpsName   = NULL;
  const char* lpsType   = NULL;
  const char* lpsClass  = NULL;
  const char* lpsArrlen = NULL;
  char*       tx        = NULL;

  CXmlStream* _this = (CXmlStream*)__this;
  DLPASSERT(_this);

  /* Scan for universal attributes */
  for (nAttr=0; lpAttr[nAttr]; nAttr+=2)
  {
    if      (dlp_strcmp(lpAttr[nAttr],"name"  )==0) lpsName   = lpAttr[nAttr+1];
    else if (dlp_strcmp(lpAttr[nAttr],"type"  )==0) lpsType   = lpAttr[nAttr+1];
    else if (dlp_strcmp(lpAttr[nAttr],"class" )==0) lpsClass  = lpAttr[nAttr+1];
    else if (dlp_strcmp(lpAttr[nAttr],"arrlen")==0) lpsArrlen = lpAttr[nAttr+1];
  }

  /* Check if XML text buffer is clear */
  if (dlp_strlen(_this->m_lpsXmlText)>0)
  {
    tx = &_this->m_lpsXmlText[dlp_strlen(_this->m_lpsXmlText)-1];
    while (tx>=_this->m_lpsXmlText && (iswspace(*tx))) *tx--='\0';
    if (dlp_strlen(_this->m_lpsXmlText)>0)
    {
      /* Oops ... */
      ERRORMSG(XMLSERR_INFILE   ,_this->m_lpsFileName,XML_GetCurrentLineNumber(_this->m_lpiParser),XML_GetCurrentColumnNumber(_this->m_lpiParser));
      ERRORMSG(XMLSERR_MISPLACED,"text",0,0);

      _this->m_lpsXmlText[0] = '\0';
    }
  }

  /* Handle elements */
  if (dlp_strcmp(lpsElement,"INSTANCE")==0)
  {
    /* Check for (root or nested) instance */
    BOOL bOk = ((_this->m_lpCurObject==NULL) && dlp_strlen(lpsClass)>0);

    /* Check for instance field */
    if (!bOk)
      if (_this->m_lpCurObject && _this->m_lpCurObject->nType==XMLS_DT_FIELD)
        if (dlp_strcmp(lpsClass,_this->m_lpCurObject->lpsType)==0 || dlp_strlen(lpsClass)==0)
          bOk = TRUE;

    /* Deserialize instance or error */
    if (bOk) CXmlStream_BeginInstance(_this,lpsName,lpsClass);
    else
    {
      ERRORMSG(XMLSERR_INFILE   ,_this->m_lpsFileName,XML_GetCurrentLineNumber(_this->m_lpiParser),XML_GetCurrentColumnNumber(_this->m_lpiParser));
      ERRORMSG(XMLSERR_MISPLACED,"tag \"INSTANCE\"",0,0);
    }
  }
  else if (dlp_strcmp(lpsElement,"FIELD")==0)
  {
    SDomObject* lpField = NULL;

    /* Check for required attributes */
    if (dlp_strlen(lpsName)<=0)
    {
      ERRORMSG(XMLSERR_INFILE   ,_this->m_lpsFileName,XML_GetCurrentLineNumber(_this->m_lpiParser),XML_GetCurrentColumnNumber(_this->m_lpiParser));
      ERRORMSG(XMLSERR_MISSATTR,"name","FIELD",0);
      return;
    }

    if (dlp_strlen(lpsType)<=0)
    {
      ERRORMSG(XMLSERR_INFILE   ,_this->m_lpsFileName,XML_GetCurrentLineNumber(_this->m_lpiParser),XML_GetCurrentColumnNumber(_this->m_lpiParser));
      ERRORMSG(XMLSERR_MISSATTR,"type","FIELD",0);
      return;
    }

    /* Check if current object is clear */
    if (_this->m_lpCurObject)
    {
      ERRORMSG(XMLSERR_INFILE  ,_this->m_lpsFileName,XML_GetCurrentLineNumber(_this->m_lpiParser),XML_GetCurrentColumnNumber(_this->m_lpiParser));
      ERRORMSG(XMLSERR_INTERNAL,"previous object unfinished",0,0);
      _this->m_lpCurObject = NULL;
    }

    lpField = (SDomObject*)dlp_calloc(1,sizeof(SDomObject));
    lpField->nType = XMLS_DT_FIELD;
    sprintf(lpField->lpsName,"%s.%s",_this->m_lpsInameFq,lpsName);
    strcpy(lpField->lpsType,lpsType);

    if (!dlp_strlen(lpsArrlen) || sscanf(lpsArrlen,"%ld",&lpField->nArrlen)!=1)
      lpField->nArrlen = 1;

    _this->m_lpCurObject = lpField;
    hash_alloc_insert(_this->m_lpDom,lpField->lpsName,lpField);
  }
  else if (dlp_strcmp(lpsElement,"TABLE")==0)
  {
    SDomObject* lpTable = NULL;

    /* Check if current object is clear */
    if (_this->m_lpCurObject)
    {
      ERRORMSG(XMLSERR_INFILE  ,_this->m_lpsFileName,XML_GetCurrentLineNumber(_this->m_lpiParser),XML_GetCurrentColumnNumber(_this->m_lpiParser));
      ERRORMSG(XMLSERR_INTERNAL,"previous object unfinished",0,0);
      _this->m_lpCurObject = NULL;
    }

    lpTable = (SDomObject*)dlp_calloc(1,sizeof(SDomObject));
    lpTable->nType = XMLS_DT_TABLE;
    sprintf(lpTable->lpsName,"%s.~table",_this->m_lpsInameFq);

    /* Is there already a table? */
    if (hash_lookup(_this->m_lpDom,lpTable->lpsName))
    {
      ERRORMSG(XMLSERR_INFILE   ,_this->m_lpsFileName,XML_GetCurrentLineNumber(_this->m_lpiParser),XML_GetCurrentColumnNumber(_this->m_lpiParser));
      ERRORMSG(XMLSERR_MISPLACED,"tag \"TABLE\"",0,0);
      dlp_free(lpTable);
      return;
    }

    /* Instantiate table */
    lpTable->lpValue = CDlpTable_CreateInstance();
    if (!lpTable->lpValue)
    {
      ERRORMSG(XMLSERR_NOMEM,0,0,0);
      dlp_free(lpTable);
      return;
    }

    _this->m_lpCurObject = lpTable;
    hash_alloc_insert(_this->m_lpDom,lpTable->lpsName,lpTable);
  }
  else if (dlp_strcmp(lpsElement,"COMP")==0)
  {
    CDlpTable* lpTable = NULL;
    short     nType   = -1;

    /* Get CDlpTable instance */
    if (!_this->m_lpCurObject || _this->m_lpCurObject->nType!=XMLS_DT_TABLE) return;
    lpTable = (CDlpTable*)_this->m_lpCurObject->lpValue;
    if (!lpTable) return;

    /* Check for required attributes */
    if (dlp_strlen(lpsType)<=0)
    {
      ERRORMSG(XMLSERR_INFILE   ,_this->m_lpsFileName,XML_GetCurrentLineNumber(_this->m_lpiParser),XML_GetCurrentColumnNumber(_this->m_lpiParser));
      ERRORMSG(XMLSERR_MISSATTR,"type","COMP",0);
      return;
    }

    /* Check type name */
    if ((nType=dlp_get_type_code(lpsType))<0)
    {
      ERRORMSG(XMLSERR_INFILE ,_this->m_lpsFileName,XML_GetCurrentLineNumber(_this->m_lpiParser),XML_GetCurrentColumnNumber(_this->m_lpiParser));
      ERRORMSG(XMLSERR_BADTYPE,lpsType,0,0);
      return;
    }

    /* Not possible if there are already records stored in table */
    if (CDlpTable_GetNRecs(lpTable)>0)
    {
      ERRORMSG(XMLSERR_INFILE   ,_this->m_lpsFileName,XML_GetCurrentLineNumber(_this->m_lpiParser),XML_GetCurrentColumnNumber(_this->m_lpiParser));
      ERRORMSG(XMLSERR_MISPLACED,"tag \"COMP\"",0,0);
      return;
    }

    /* Insert a component */
    CDlpTable_AddComp(lpTable,lpsName,nType);
  }
  else if (dlp_strcmp(lpsElement,"REC")==0)
  {
    CDlpTable* lpTable = NULL;

    if (!_this->m_lpCurObject || _this->m_lpCurObject->nType!=XMLS_DT_TABLE) return;
    lpTable = (CDlpTable*)_this->m_lpCurObject->lpValue;
    if (!lpTable) return;

    /* Are there components? */
    if (!CDlpTable_GetNComps(lpTable)) return;

    /* Insert a record */
    if (CDlpTable_GetMaxRecs(lpTable)<=CDlpTable_GetNRecs(lpTable))
      IF_NOK(CDlpTable_Realloc(lpTable,max((long)(1.1*CDlpTable_GetMaxRecs(lpTable)),100+CDlpTable_GetMaxRecs(lpTable))))
      {
        ERRORMSG(XMLSERR_NOMEM,0,0,0);
        return;
      }

    /* Reset cell counter */
    _this->m_nCompCtr=0;
    _this->m_bCell   =FALSE;
  }
  else if (dlp_strcmp(lpsElement,"CELL")==0)
  {
    _this->m_bCell=TRUE;
  }
  else
  {
    ERRORMSG(XMLSERR_INFILE   ,_this->m_lpsFileName,XML_GetCurrentLineNumber(_this->m_lpiParser),XML_GetCurrentColumnNumber(_this->m_lpiParser));
    ERRORMSG(XMLSERR_TAGUNKNOWN,lpsElement,0,0);
  }
}

/**
 * Expat event handler for closing XML tags.
 */
void CXmlStream_OnCloseTag(void* __this, const char* lpsElement)
{
  CXmlStream* _this = (CXmlStream*)__this;
  DLPASSERT(_this);

  /* Handle elements */
  if (dlp_strcmp(lpsElement,"INSTANCE")==0)
  {
    CXmlStream_EndInstance(_this);
  }
  else if (dlp_strcmp(lpsElement,"FIELD")==0)
  {
    if (!_this->m_lpCurObject) return;
    if (_this->m_lpCurObject->nType!=XMLS_DT_FIELD)
    {
      _this->m_lpCurObject=NULL;
      return;
    }

    if (_this->m_lpsXmlText)
    {
      /* Hand over pointer, will be freed by destructor */
      _this->m_lpCurObject->lpValue = _this->m_lpsXmlText;
      _this->m_lpsXmlText = NULL;
    }

    IFCHECK
    {
      printf("\n   F %-3ld %-12s %-20s = ",_this->m_lpCurObject->nArrlen,_this->m_lpCurObject->lpsType,_this->m_lpCurObject->lpsName);
      if (_this->m_lpCurObject->lpValue) printf("\"%s\"",_this->m_lpCurObject->lpValue);
      else printf("NULL");
    }

    /* Notify field complete */
    _this->m_lpCurObject=NULL;
  }
  else if (dlp_strcmp(lpsElement,"TABLE")==0)
  {
    CDlpTable* lpTable = NULL;

    if (!_this->m_lpCurObject || _this->m_lpCurObject->nType!=XMLS_DT_TABLE) return;
    lpTable = (CDlpTable*)_this->m_lpCurObject->lpValue;
    if (!lpTable) return;

    IFCHECK
    {
      if (CDlpTable_GetNComps(lpTable)>0)
      {
        char lpsBuf[48];
        sprintf(lpsBuf,"(%ldx%ld[/%ld])",(long)CDlpTable_GetNComps(lpTable),(long)CDlpTable_GetNRecs(lpTable),(long)CDlpTable_GetMaxRecs(lpTable));
        printf("\n   T %-16s %-20s",lpsBuf,_this->m_lpCurObject->lpsName);
        CDlpTable_Print(lpTable);
      }
      else printf("\n   T                  %-20s : (empty)",_this->m_lpCurObject->lpsName);
    }

    /* Notify table complete */
    _this->m_lpCurObject=NULL;
  }
  else if (dlp_strcmp(lpsElement,"COMP")==0)
  {
    /* Nothing to be done for this */
  }
  else if (dlp_strcmp(lpsElement,"REC")==0)
  {
    CDlpTable* lpiTable = NULL;

    if (!_this->m_lpCurObject || _this->m_lpCurObject->nType!=XMLS_DT_TABLE) return;
    lpiTable = (CDlpTable*)_this->m_lpCurObject->lpValue;
    if (!lpiTable) return;

    /* Adjust record counter */
    CDlpTable_IncNRecs(lpiTable,1);

    /* Reset cell recording mechanism */
    _this->m_bCell   =FALSE;
    _this->m_nCompCtr=-1;
  }
  else if (dlp_strcmp(lpsElement,"CELL")==0)
  {
    CDlpTable* lpiTable = NULL;

    if (!_this->m_lpCurObject || _this->m_lpCurObject->nType!=XMLS_DT_TABLE) return;
    lpiTable = (CDlpTable*)_this->m_lpCurObject->lpValue;
    if (!lpiTable) return;

    if (_this->m_bCell && _this->m_nCompCtr>=0 && _this->m_nCompCtr<CDlpTable_GetNComps(lpiTable))
    {
      short nType = CDlpTable_GetCompType(lpiTable,_this->m_nCompCtr);
      if (dlp_is_symbolic_type_code(nType))
        dlp_memmove
        (
          CDlpTable_XAddr(lpiTable,CDlpTable_GetNRecs(lpiTable),_this->m_nCompCtr),
          _this->m_lpsXmlText,MIN(nType,(short)dlp_strlen(_this->m_lpsXmlText)+1)
        );
      else if (nType == T_PTR)
      	/* Do nothing */;
      else
        dlp_sscanx
        (
          _this->m_lpsXmlText,nType,
          CDlpTable_XAddr(lpiTable,CDlpTable_GetNRecs(lpiTable),_this->m_nCompCtr)
        );
    }

    if (_this->m_lpsXmlText) _this->m_lpsXmlText[0]='\0';
    _this->m_bCell=FALSE;
    _this->m_nCompCtr++;
  }
  else
  {
    ERRORMSG(XMLSERR_INFILE   ,_this->m_lpsFileName,XML_GetCurrentLineNumber(_this->m_lpiParser),XML_GetCurrentColumnNumber(_this->m_lpiParser));
    ERRORMSG(XMLSERR_TAGUNKNOWN,lpsElement,0,0);
  }
}

/**
 * Expat event handler for XML text.
 */
void CXmlStream_OnText(void* __this, const XML_Char* lpsText, int nLen)
{
  /*const XML_Char* tx        = NULL;*/
  long            nFirstNew = 0;

  CXmlStream* _this = (CXmlStream*)__this;
  DLPASSERT(_this);
  DLPASSERT(sizeof(XML_Char)==sizeof(char));

  /* Do not process strings consisting of white chars only */
  if (!lpsText) return;
  /* THIS IS DOUBTFUL -->
  for (tx=&lpsText[nLen-1]; tx>=lpsText; tx--)
    if (!iswspace(*tx))
      break;
  if (tx<lpsText) return;
  <-- */

  /* Just record; all contiguous text belongs to exactly one DOM object or table cell */
  nFirstNew = dlp_strlen(_this->m_lpsXmlText);
  if (dlp_size(_this->m_lpsXmlText) < dlp_strlen(_this->m_lpsXmlText)+nLen+1)
  {
    long nLenNew = (long)(1.1*(nFirstNew+nLen))+1;

    _this->m_lpsXmlText = (char*)__dlp_realloc(_this->m_lpsXmlText,nLenNew,sizeof(char),__FILE__,__LINE__,"CXmlStream",NULL);
    dlp_memset(&_this->m_lpsXmlText[nFirstNew],0,nLenNew-nFirstNew);
  }

  if (!_this) { ERRORMSG(XMLSERR_NOMEM,0,0,0); return; }

  dlp_memmove(&_this->m_lpsXmlText[dlp_strlen(_this->m_lpsXmlText)],lpsText,nLen);
  _this->m_lpsXmlText[nFirstNew+nLen] = '\0';
}

/**
 *
 */
short CXmlStream_BeginInstance(CXmlStream* _this, const char* lpInstanceName, const char* lpClassName)
{
  /* Trace containment */
  _this->m_nDepth++;
  if (_this->m_nDepth>1)
  {
    dlp_strcat(_this->m_lpsInameFq,".");
    dlp_strcat(_this->m_lpsInameFq,lpInstanceName);
  }

  /* Read/write dependent actions */
  if (_this->m_nMode&XMLS_WRITE)
  {
    XML_INDENT_LINE(_this,_this->m_nDepth);
    CXmlStream_Printf(_this,"<INSTANCE name=\"%s\" class=\"%s\">\n",lpInstanceName,lpClassName);
    _this->m_nDepth++;

    return O_K;
  }
  else if (_this->m_nMode&XMLS_READ)
  {
    SDomObject* lpInst = (SDomObject*)dlp_calloc(1,sizeof(SDomObject));
    lpInst->nType = dlp_strlen(lpClassName) ? XMLS_DT_INSTANCE : XMLS_DT_NULLINST;
    dlp_strcpy(lpInst->lpsName,_this->m_lpsInameFq);
    dlp_strcpy(lpInst->lpsType,lpClassName);
    hash_alloc_insert(_this->m_lpDom,lpInst->lpsName,lpInst);

    IFCHECK
    {
      if (_this->m_lpCurObject)
        printf("\n   F %-3ld %-12s %-20s = ...",_this->m_lpCurObject->nArrlen,_this->m_lpCurObject->lpsType,_this->m_lpCurObject->lpsName);
      else if (_this->m_nDepth==1)
        printf("\n   R     %-12s %-20s",lpClassName,lpInstanceName);

      printf("\n   %c >>> %-12s %-20s",lpInst->nType==XMLS_DT_NULLINST?'N':'I',lpInst->lpsType,lpInst->lpsName);
    }

    _this->m_lpCurObject=NULL;
    return O_K;
  }
  else
  {
    DLPASSERT(FMSG("CXmlStream_BeginInstance: invalid mode"));
    return NOT_EXEC;
  }
}

/**
 *
 */
short CXmlStream_EndInstance(CXmlStream* _this)
{
  if (_this->m_nMode&(XMLS_WRITE|XMLS_READ))
  {
    char* tx = NULL;

    _this->m_nDepth--;


    if (_this->m_nMode&XMLS_READ)
    {
      IFCHECK  printf("\n   I <<<              %-20s",_this->m_lpsInameFq);
    }
    else if (_this->m_nMode&XMLS_WRITE)
    {
      XML_INDENT_LINE(_this,_this->m_nDepth);
      CXmlStream_Printf(_this,"</INSTANCE>\n");
      _this->m_nDepth--;
    }

    for (tx=&_this->m_lpsInameFq[dlp_strlen(_this->m_lpsInameFq)-1]; *tx; tx--)
      if (*tx=='.')
      {
        *tx=0;
        break;
      }

    return O_K;
  }
  else
  {
    DLPASSERT(FMSG("CXmlStream_EndInstance: invalid mode"));
    return NOT_EXEC;
  }
}

/**
 *
 */
short CXmlStream_SerializeTable(CXmlStream* _this, CDlpTable* lpiTable, const char* lpsFqName)
{
  long nComp = 0;
  long nRec  = 0;
  char lpOutBuf[256];

  if (!lpiTable) return NOT_EXEC;

  if(!_this)
  {
    XML_INDENT_LINE(_this,_this->m_nDepth);
    CXmlStream_Printf(_this,"<TABLE/>\n");
    return NOT_EXEC;
  }

  /* Open table tag */
  XML_INDENT_LINE(_this,_this->m_nDepth);
  CXmlStream_Printf(_this,"<TABLE name=\"%s\">\n",CXmlStream_Encode(_this,lpsFqName));

  /* Serialize component structure */
  for (nComp=0; nComp<lpiTable->m_dim; nComp++)
  {
    XML_INDENT_LINE(_this,_this->m_nDepth+1);
    CXmlStream_Printf
    (
      _this,"<COMP name=\"%s\" type=\"%s\"/>\n",
      CXmlStream_Encode(_this,lpiTable->m_compDescrList[nComp].lpName),
      dlp_get_type_name(lpiTable->m_compDescrList[nComp].ctype)
    );
  }

  /* Serialize cells */
  for (nRec=0; nRec<lpiTable->m_nrec; nRec++)
  {
    XML_INDENT_LINE(_this,_this->m_nDepth+1);
    CXmlStream_Printf(_this,"<REC>\n");

    for (nComp=0; nComp<lpiTable->m_dim; nComp++)
    {
      XML_INDENT_LINE(_this,_this->m_nDepth+2);
      CXmlStream_Printf(_this,"<CELL>");
      if (CDlpTable_GetCompType(lpiTable,nComp)>1 && CDlpTable_GetCompType(lpiTable,nComp)<=255)
        CXmlStream_Printf(_this,"%s",CXmlStream_Encode(_this,(char*)CDlpTable_XAddr(lpiTable,nRec,nComp)));
      else{
        dlp_printx_ext(lpOutBuf,CDlpTable_XAddr(lpiTable,nRec,nComp),CDlpTable_GetCompType(lpiTable,nComp),0,FALSE,TRUE,TRUE);
        CXmlStream_Printf(_this,lpOutBuf);
      }
      CXmlStream_Printf(_this,"</CELL>\n");
    }

    XML_INDENT_LINE(_this,_this->m_nDepth+1);
    CXmlStream_Printf(_this,"</REC>\n");
  }

  /* Close table tag */
  XML_INDENT_LINE(_this,_this->m_nDepth);
  CXmlStream_Printf(_this,"</TABLE>\n");

  return O_K;
}

/**
 *
 */
SDomObject* CXmlStream_FindObject(CXmlStream* _this, const char* lpFqName)
{
  hnode_t* hn;

  hn = hash_lookup(_this->m_lpDom,lpFqName);
  if (!hn) return NULL;

  return (SDomObject*)hnode_get(hn);
}

/**
 *
 */
void CXmlStream_ObjectNotFound(CXmlStream* _this)
{
  _this->m_nFieldsNotFound++;
}

/**
 *
 */
const char* CXmlStream_Encode(CXmlStream* _this, const char* lpText)
{
  const unsigned char* tx = NULL;
  char*                ty = NULL;

  /* Prepare string conversion buffer */
  if (_this->m_lpsBuffer==NULL || dlp_size(_this->m_lpsBuffer)<dlp_strlen(lpText)+1)
    _this->m_lpsBuffer = (char*)__dlp_realloc(_this->m_lpsBuffer,(int)(1.1*dlp_strlen(lpText))+1,sizeof(char),__FILE__,__LINE__,"CXmlStream",NULL);

  dlp_memset(_this->m_lpsBuffer,0,dlp_size(_this->m_lpsBuffer));

  if (dlp_strlen(lpText)==0) return _this->m_lpsBuffer;

  /* Copy input to output string */
  for (tx=(unsigned char*)lpText,ty=_this->m_lpsBuffer; *tx; tx++,ty++)
  {
    if      ('>' ==*tx) { dlp_strcpy(ty,"&gt;"  ); ty+=3; }
    else if ('<' ==*tx) { dlp_strcpy(ty,"&lt;"  ); ty+=3; }
    else if ('&' ==*tx) { dlp_strcpy(ty,"&amp;" ); ty+=4; }
    else if ('\"'==*tx) { dlp_strcpy(ty,"&quot;"); ty+=5; }
    else if ('\''==*tx) { dlp_strcpy(ty,"&apos;"); ty+=5; }
/*    else if (*tx>0x7F )
    {
      char lpBuf[16];
      sprintf(lpBuf,"&#%hhu;",*tx);
      dlp_strcpy(ty,lpBuf);
      ty+=dlp_strlen(lpBuf)-1;
    }   */
    else if (*tx<0x20 && *tx!='\t' && *tx!='\n' && *tx!='\r')
    {
      /* 2002-08-20 HACK: These characters are forbidden in XML; replace by underscore */
      *ty='_';
    }
    else *ty=*tx;

    /* Enlarge buffer if necessary */
    if ((long)dlp_strlen(_this->m_lpsBuffer) > (long)(dlp_size(_this->m_lpsBuffer)-10))
    {
      _this->m_lpsBuffer = (char*)__dlp_realloc(_this->m_lpsBuffer,ty-_this->m_lpsBuffer+512,sizeof(char),__FILE__,__LINE__,"CXmlStream",NULL);
      ty = &_this->m_lpsBuffer[dlp_strlen(_this->m_lpsBuffer)-1];
    }
  }

  *ty=0; /* Just in case ... */

  return _this->m_lpsBuffer;
}

/* EOF */
