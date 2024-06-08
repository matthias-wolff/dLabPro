/* dLabPro DNorm3 stream library
 * - Implementation file
 *
 * AUTHOR : Matthias Eichner
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

#include "dlp_cscope.h"
#include "dlp_kernel.h"
#include "dlp_object.h"
#ifndef __NODN3STREAM
#include "dlp_dn3stream.h"


/**
 * Create CDN3Stream instance.
 *
 * @param lpsFilename  Name of file to read or write
 * @param nMode        Mode of stream (read/write)
 * @param lpsFileclass Class of file to process
 * @return Pointer to new CDN3Stream
 */
CDN3Stream* CDN3Stream_CreateInstance(const char* lpsFilename, const INT16 nMode, const char* lpsFileclass)
{
  CDN3Stream* lpiNewCDN3Stream = NULL;

  lpiNewCDN3Stream = (CDN3Stream*)dlp_calloc(1,sizeof(CDN3Stream));

  if(CDN3Stream_Constructor(lpiNewCDN3Stream,lpsFilename,nMode,lpsFileclass) != O_K)
  {
    CDN3Stream_Destructor(lpiNewCDN3Stream);
    dlp_free(lpiNewCDN3Stream);
    return NULL;
  }

  return lpiNewCDN3Stream;
}

/**
 * Destroy CDN3Stream instance.
 *
 * @param _this This instance
 * @return O_K if successful, an error code otherwise
 */
INT16 CDN3Stream_DestroyInstance(CDN3Stream* _this)
{
  if(!_this) return NOT_EXEC;

  CDN3Stream_Destructor(_this);
  dlp_free(_this);
  _this = NULL;

  return O_K;
}

/**
 * Implementation of CDN3Stream constructor.
 *
 * @param _this        This instance
 * @param lpsFilename  Pointer to file name
 * @param nMode        File mode, DN3_READ or DN3_WRITE
 * @param lpsFileclass Identifier of file class (usually a dLabPro class identifier)
 * @return O_K if successful, NOT_EXEC otherwise
 */
INT16 CDN3Stream_Constructor(CDN3Stream* _this, const char* lpsFilename, const INT16 nMode, const char* lpsFileclass)
{
  if(!_this) return NOT_EXEC;

  /* init fields */
  dlp_strncpy(_this->m_lpsFilename ,lpsFilename ,CDN3_FILENAME);
  dlp_strncpy(_this->m_lpsFileclass,lpsFileclass,L_NAMES      );

  _this->m_nMode              =  nMode;
  _this->m_nTypesafe          =  1;
  _this->m_nKnr               =  1;
  _this->m_nRnr               =  1;
  _this->m_lpsGlobal          =  (char*)dlp_calloc(1,CDN3_GLOBLENGTH);
  _this->m_nLevel             =  0;
  _this->m_nArrayIdx          = -1;
  _this->m_nContainer         = -1;
  _this->m_lpParms            = (DPARA*)dlp_calloc(1,sizeof(DPARA));
  _this->m_lpDnfile           = NULL;
  _this->m_lpiTIT             = CDlpTable_CreateInstance();
  _this->m_lpiDoubleCollector = CDlpTable_CreateInstance();
  _this->m_lpiLongCollector   = CDlpTable_CreateInstance();
  _this->m_lpiCharCollector   = CDlpTable_CreateInstance();

  if
  (
    !_this->m_lpiTIT             ||
    !_this->m_lpiDoubleCollector ||
    !_this->m_lpiLongCollector   ||
    !_this->m_lpiCharCollector
  )
  {
    return NOT_EXEC;
  }

  /* write table name into realisation string */
  _this->m_lpiTIT->m_rtext = dlp_calloc(1,L_NAMES);
  if(_this->m_lpiTIT->m_rtext) dlp_strcpy(_this->m_lpiTIT->m_rtext,"TIT");
  _this->m_lpiDoubleCollector->m_rtext = dlp_calloc(1,L_NAMES);
  if(_this->m_lpiDoubleCollector->m_rtext) dlp_strcpy(_this->m_lpiDoubleCollector->m_rtext,"DoubleCollector");
  _this->m_lpiLongCollector->m_rtext = dlp_calloc(1,L_NAMES);
  if(_this->m_lpiLongCollector->m_rtext) dlp_strcpy(_this->m_lpiLongCollector->m_rtext,"LongCollector");
  _this->m_lpiCharCollector->m_rtext = dlp_calloc(1,L_NAMES);
  if(_this->m_lpiCharCollector->m_rtext) dlp_strcpy(_this->m_lpiCharCollector->m_rtext,"CharCollector");

  /* Build type information table */
  CDlpTable_AddComp(_this->m_lpiTIT, "name", L_NAMES);
  CDlpTable_AddComp(_this->m_lpiTIT, "type", L_NAMES);
  CDlpTable_AddComp(_this->m_lpiTIT, "ntyp", T_SHORT);
  CDlpTable_AddComp(_this->m_lpiTIT, "cont", T_INT  );
  CDlpTable_AddComp(_this->m_lpiTIT, "arrl", T_INT  );
  CDlpTable_AddComp(_this->m_lpiTIT, "rnr" , T_SHORT);
  CDlpTable_AddComp(_this->m_lpiTIT, "knr" , T_SHORT);
  CDlpTable_AddComp(_this->m_lpiTIT, "coli", T_INT  );
  CDlpTable_AddComp(_this->m_lpiTIT, "arri", T_INT );
  CDlpTable_Alloc(_this->m_lpiTIT, 1);

  /* Build collectors */
  CDlpTable_AddComp(_this->m_lpiDoubleCollector,"value",T_DOUBLE);
  CDlpTable_Alloc  (_this->m_lpiDoubleCollector,CDN3_GRANULARITY);
  CDlpTable_AddComp(_this->m_lpiLongCollector  ,"value",T_LONG  );
  CDlpTable_Alloc  (_this->m_lpiLongCollector  ,CDN3_GRANULARITY);
  CDlpTable_AddComp(_this->m_lpiCharCollector  ,"value",T_CHAR  );
  CDlpTable_Alloc  (_this->m_lpiCharCollector  ,CDN3_GRANULARITY);

  switch(nMode)
  {
    case CDN3_WRITE:
    {
      FILE* lpFile = NULL;

      _this->m_nTypesafe = 1;
      lpFile = fopen(_this->m_lpsFilename,"w");
      if (!lpFile) return CDN3_FILENOTCREATE;
      else fclose(lpFile);
      remove(_this->m_lpsFilename);

      /* manage writing of collectors and type information table */
      if(!dlp_strcmp(lpsFileclass,"data")) _this->m_bWriteCol =  FALSE;
      else                                 _this->m_bWriteCol =  TRUE;

      /* Write file class information to global information string */
      dlp_memset(_this->m_lpsGlobal,0,CDN3_GLOBLENGTH);
      sprintf(_this->m_lpsGlobal,"CLASS=%s ",_this->m_lpsFileclass);
      break;
    }
    case CDN3_READ:
    {
      INT16 nRet   = 0;
      FILE* lpFile = NULL;

      lpFile=fopen(_this->m_lpsFilename,"rb");
      if (!lpFile) return CDN3_FILEOPEN;
      else fclose(lpFile);
      if(!dlp_strlen(lpsFileclass)) return O_K;
      if(!dlp_strcmp(lpsFileclass,"data")) return O_K;
      else
      {
        nRet = CDN3Stream_VerifyFileClass(_this);
        if(nRet != O_K) return nRet;
        return CDN3Stream_ReadCollectors(_this);
      }
    }
    default: return NOT_EXEC;
  }

  return O_K;
}

/**
 * Implementation of CDN3Stream destructor.
 *
 * @param _this This instance
 * @return O_K if successful, an error code otherwise
 */
INT16 CDN3Stream_Destructor(CDN3Stream* _this)
{
  INT16 nRetVal    = O_K;

  if(!_this) return NOT_EXEC;

  if(_this->m_nMode == CDN3_WRITE)
  {
    /* Write collectors */
    if(_this->m_bWriteCol)
    {
      if(O_K != CDN3Stream_SaveCollectors(_this)) nRetVal = NOT_EXEC;
    }
    if(O_K != CDN3Stream_WriteGlobalInf(_this)) nRetVal = NOT_EXEC;
  }

  dlp_free(_this->m_lpsGlobal);

  CDlpTable_DestroyInstance(_this->m_lpiTIT);
  CDlpTable_DestroyInstance(_this->m_lpiDoubleCollector);
  CDlpTable_DestroyInstance(_this->m_lpiLongCollector);
  CDlpTable_DestroyInstance(_this->m_lpiCharCollector);
  dlp_free(_this->m_lpParms);

  return nRetVal;
}

/**
 * Write field into collector.
 *
 * @param _this       This instance
 * @param nType       Type of data to write into collector
 * @param lpData      Pointer to field data
 * @param nArrayLen   Number of elements in array
 * @return O_K if successful, an error code otherwise
 */
INT16 CDN3Stream_WriteCollector(CDN3Stream* _this, INT16 nType, void* lpData, INT32 nArrayLen)
{
  INT32       i           = 0;
  INT32       nRecs       = 0;
  CDlpTable* lpCollector = NULL;

  if(!_this)       return NOT_EXEC;

  /* select collector depending on type of data */
  switch (nType)
  {
  case T_STRING  :
  case T_CSTRING :
  case T_TEXT    :
  case T_UCHAR   :
  case T_CHAR    : lpCollector = _this->m_lpiCharCollector;   break;
  case T_USHORT  :
  case T_SHORT   :
  case T_ULONG   :
  case T_LONG    : lpCollector = _this->m_lpiLongCollector;   break;
  case T_FLOAT   :
  case T_DOUBLE  : lpCollector = _this->m_lpiDoubleCollector; break;
  default         : DLPASSERT(FALSE);
  }

  /* enshure collector capacity */
  if(CDlpTable_GetNRecs(lpCollector) + nArrayLen > CDlpTable_GetMaxRecs(lpCollector))
    if(CDlpTable_Realloc(lpCollector,CDlpTable_GetMaxRecs(lpCollector) + nArrayLen + CDN3_GRANULARITY) != O_K)
      return FALSE;

  /* write start index in type information table (record already allocated by 'AddType') */
  CDlpTable_Dstore(_this->m_lpiTIT,CDlpTable_GetNRecs(lpCollector),CDlpTable_GetNRecs(_this->m_lpiTIT)-1, OF_COLLIDX);

  /* write data into collector */
  for(i=0; i<nArrayLen; i++)
  {
    nRecs = CDlpTable_GetNRecs(lpCollector);

    switch (nType)
    {
      case T_STRING :
      case T_CSTRING:
      case T_TEXT   :
      case T_UCHAR  :
      case T_CHAR   :
        CDlpTable_Dstore(lpCollector,((char*)lpData)[i],nRecs,0);
        break;
      case T_USHORT :
      case T_SHORT  :
      {
        INT64 nBuf = 0;
        nBuf = (INT64)((INT16*)lpData)[i];
        CDlpTable_Dstore(lpCollector,nBuf,nRecs,0);
        break;
      }
      case T_ULONG:
      case T_LONG :
        CDlpTable_Dstore(lpCollector,((INT64*)lpData)[i],nRecs,0);
        break;
      case T_FLOAT:
      {
        FLOAT64 nBuf = 0;
        nBuf = (FLOAT64)((FLOAT32*)lpData)[i];
        CDlpTable_Dstore(lpCollector,nBuf,nRecs,0);
        break;
      }
      case T_DOUBLE:
        CDlpTable_Dstore(lpCollector,((FLOAT64*)lpData)[i],nRecs,0);
        break;
      default: DLPASSERT(FALSE);
    }

    CDlpTable_SetNRecs(lpCollector, nRecs + 1);
  }

  return O_K;
}

/**
 * Add field to type information table.
 *
 * @param _this       This instance
 * @param lpsName     Name of field
 * @param lpsTypeName Name of type
 * @param nType       Type of field
 * @param nArrayLen   Number of elements in char array
 * @return O_K if successful, an error code otherwise
 */
INT16 CDN3Stream_AddType
(
  CDN3Stream* _this,
  const char* lpsName,
  const char* lpsTypeName,
  INT16       nType,
  INT32        nArrayLen
)
{
  INT32 nRecs = 0;

  if(!_this)
  {
    printf("\nThis instance is NULL.",lpsName);
    return NOT_EXEC;
  }
  if(CDN3Stream_LookupNameOnLevel(_this,lpsName)!=NOT_EXEC)
  {
    printf("\nDuplicate name \"%s\".",lpsName);
    return NOT_EXEC;
  }

  nRecs = CDlpTable_GetNRecs(_this->m_lpiTIT);

  CDlpTable_Realloc (_this->m_lpiTIT,nRecs+1);
  CDlpTable_SetNRecs(_this->m_lpiTIT,nRecs+1);

  CDlpTable_Sstore(_this->m_lpiTIT,lpsName            ,nRecs,OF_NAME     );
  CDlpTable_Dstore(_this->m_lpiTIT,_this->m_nContainer,nRecs,OF_CONTAINER);
  CDlpTable_Dstore(_this->m_lpiTIT,_this->m_nArrayIdx ,nRecs,OF_ARRIDX   );
  CDlpTable_Sstore(_this->m_lpiTIT,lpsTypeName        ,nRecs,OF_TYPENAME );
  CDlpTable_Dstore(_this->m_lpiTIT,nType              ,nRecs,OF_NTYPE    );
  CDlpTable_Dstore(_this->m_lpiTIT,nArrayLen          ,nRecs,OF_ARRLEN   );

  if(nType == T_INSTANCE)
  {
    CDlpTable_Dstore(_this->m_lpiTIT,_this->m_nRnr,nRecs,OF_RNR);
    CDlpTable_Dstore(_this->m_lpiTIT,_this->m_nKnr,nRecs,OF_KNR);
  }
  else
  {
    CDlpTable_Dstore(_this->m_lpiTIT,0,nRecs,OF_RNR);
    CDlpTable_Dstore(_this->m_lpiTIT,0,nRecs,OF_KNR);
  }

  return O_K;
}

/**
 * Lookup name on level.
 *
 * @param _this   This instance
 * @param lpsName Name of field
 * @return index if found, NOT_EXEC otherwise
 */
INT32 CDN3Stream_LookupNameOnLevel(CDN3Stream* _this, const char* lpsName)
{
  INT32       i     = 0;
  CDlpTable* lpTIT = NULL;

  if(!_this)        return NOT_EXEC;
  if(!_this->m_lpiTIT) return NOT_EXEC;

  lpTIT = _this->m_lpiTIT;

  for (i=0; i<CDlpTable_GetNRecs(lpTIT); i++)
  {
    INT32 nContainer = (INT32)CDlpTable_Dfetch(lpTIT, i, OF_CONTAINER);
    /*INT32 nArridx    = (INT32)CDlpTable_Dfetch(lpTIT, i, OF_ARRIDX   );*/

    if (nContainer != _this->m_nContainer) continue;

    /* BUG:  Test of m_ArrayIdx makes the serialization depend        */
    /*       on the order of instances.                               */
    /* HACK: Don't test for arrayindex, BUT deserialization of more   */
    /*       than one instance of the same type will fail!            */
    /* TODO: Use full qualified instance name instead of array index. */
    /*                                                                */
    /* OLD: -->                                                       */
    /*if (nArridx    != lpTIT->m_ArrayIdx ) continue;                 */
    /* <--                                                            */

    if (dlp_strcmp((char*)CDlpTable_XAddr(lpTIT,i,OF_NAME),lpsName) == 0) return i;
  }
  return NOT_EXEC;
}

/**
 * Read global information string
 *
 * @param _this          This instance
 * @param lpsClassName   Name of root class
 * @return O_K if successful, an error code otherwise
 */
INT16 CDN3Stream_GetFileClass(CDN3Stream* _this,char *lpsClassName)
{
  /* read global information string */
  if ((_this->m_lpDnfile=dopen(_this->m_lpsFilename, "r", _this->m_lpParms))==NULL)
    return CDN3_CANNOTREADGLOB;
  dlp_strcpy(_this->m_lpsGlobal,_this->m_lpParms->xt);
  dclose(_this->m_lpDnfile, _this->m_lpParms);

  /* printf("\n GlobalInf: '%s'",_this->m_lpsGlobal); */

  /* parse file class information in global information string */
  if (CDN3Stream_ParseGlobal(_this,"CLASS=",lpsClassName,L_NAMES) != O_K)
    return CDN3_MISSCLASS;

  return O_K;
}

/**
 * Read global information string and verify file class.
 *
 * @param _this      This instance
 * @return O_K if successful, an error code otherwise
 */
INT16 CDN3Stream_VerifyFileClass(CDN3Stream* _this)
{
  char* lpBuffer = (char*)dlp_calloc(1, CDN3_GLOBLENGTH);
  INT16 nErr;

  /* read file class name */
  nErr = CDN3Stream_GetFileClass(_this,lpBuffer);
  IF_NOK(nErr)
  {
    dlp_free(lpBuffer);
    return nErr;
  }

  /* verify file class */
  if (dlp_strlen(_this->m_lpsFileclass))
  {
    if (!dlp_strlen(lpBuffer) || dlp_strncmp(_this->m_lpsFileclass,lpBuffer,L_NAMES) != 0)
    {
      dlp_free(lpBuffer);
      return CDN3_WRONGCLASS;
    }
  }

  dlp_free(lpBuffer);
  return O_K;
}

/**
 * Read collectors and type information table.
 *
 * @param _this      This instance
 * @return O_K if successful, an error code otherwise
 */
INT16 CDN3Stream_ReadCollectors(CDN3Stream* _this)
{
  INT32  nRetVal  = 0;
  char* lpBuffer = (char*)dlp_calloc(1, CDN3_GLOBLENGTH);
  short nTmpShort1 = 0;
  short nTmpShort2 = 0;


  /* Load type information table */
  if (O_K != CDN3Stream_ParseGlobal(_this,"TIT=",lpBuffer,L_NAMES)) return CDN3_TYPEUNSAFE;
  else
  {
    DLPASSERT(_this->m_lpiTIT)
    nRetVal = sscanf(lpBuffer,"%hd,%hd", &nTmpShort1, &nTmpShort2);
    _this->m_nKnr = (INT16)nTmpShort1;
    _this->m_nRnr = (INT16)nTmpShort2;
    if (nRetVal != 2 || O_K != CDN3Stream_DeserializeTable(_this,_this->m_lpiTIT))
    {
      dlp_free(lpBuffer);
      return CDN3_CANNOTREADTIT;
    }
  }
  /*_this->m_lpiTIT->Print(_this->m_lpiTIT);*/

  /* Load double collector */
  if (O_K == CDN3Stream_ParseGlobal(_this,"DOUBLE=",lpBuffer,L_NAMES))
  {
    DLPASSERT(_this->m_lpiDoubleCollector)
    nRetVal = sscanf(lpBuffer,"%hd,%hd", &nTmpShort1, &nTmpShort2);
    _this->m_nKnr = (INT16)nTmpShort1;
    _this->m_nRnr = (INT16)nTmpShort2;
    if (nRetVal != 2 || O_K != CDN3Stream_DeserializeTable(_this,_this->m_lpiDoubleCollector))
    {
      dlp_free(lpBuffer);
      return CDN3_CANNOTREADCOLL;
    }
  }

  /* Load long collector */
  if (O_K == CDN3Stream_ParseGlobal(_this,"LONG=",lpBuffer,L_NAMES))
  {
    DLPASSERT(_this->m_lpiLongCollector)
    nRetVal = sscanf(lpBuffer,"%hd,%hd", &nTmpShort1, &nTmpShort2);
    _this->m_nKnr = (INT16)nTmpShort1;
    _this->m_nRnr = (INT16)nTmpShort2;
    if (nRetVal != 2 || O_K != CDN3Stream_DeserializeTable(_this,_this->m_lpiLongCollector))
    {
      dlp_free(lpBuffer);
      return CDN3_CANNOTREADCOLL;
    }
  }
  /*dlpTable_Print(_this->m_lpiLongCollector);*/

  /* Load char collector */
  if (O_K == CDN3Stream_ParseGlobal(_this,"CHAR=",lpBuffer,L_NAMES))
  {
    DLPASSERT(_this->m_lpiCharCollector)
    nRetVal = sscanf(lpBuffer,"%hd,%hd", &nTmpShort1, &nTmpShort2);
    _this->m_nKnr = (INT16)nTmpShort1;
    _this->m_nRnr = (INT16)nTmpShort2;
    if (nRetVal != 2 || O_K != CDN3Stream_DeserializeTable(_this,_this->m_lpiCharCollector))
    {
      dlp_free(lpBuffer);
      return CDN3_CANNOTREADCOLL;
    }
  }

  dlp_free(lpBuffer);
  return O_K;
}

/**
 * Save collectors and type information table.
 *
 * @param _this      This instance
 * @return O_K if successful, an error code otherwise
 */
INT32 CDN3Stream_SaveCollectors(CDN3Stream* _this)
{
  char* buffer = _this->m_lpsGlobal;
  buffer      += strlen(buffer);

  /* Shrink collectors */
  CDlpTable_Realloc(_this->m_lpiCharCollector,  CDlpTable_GetNRecs(_this->m_lpiCharCollector));
  CDlpTable_Realloc(_this->m_lpiLongCollector,  CDlpTable_GetNRecs(_this->m_lpiLongCollector));
  CDlpTable_Realloc(_this->m_lpiDoubleCollector,CDlpTable_GetNRecs(_this->m_lpiDoubleCollector));

  /* Write type information table */
  if (_this->m_lpiTIT)
  {
    sprintf(buffer, "TIT=%hd,%hd ", (short)_this->m_nKnr, (short)_this->m_nRnr);
    buffer += strlen(buffer);
    if(CDN3Stream_SerializeTable(_this, _this->m_lpiTIT) != O_K) return NOT_EXEC;
  }
  /* _this->m_lpiTIT->Print(_this->m_lpiTIT);*/

  /* Write double collector */
  if (CDlpTable_GetNRecs(_this->m_lpiDoubleCollector) > 0)
  {
    sprintf(buffer, "DOUBLE=%hd,%hd ", (short)_this->m_nKnr, (short)_this->m_nRnr);
    buffer += strlen(buffer);
    if(CDN3Stream_SerializeTable(_this, _this->m_lpiDoubleCollector) != O_K) return NOT_EXEC;
  }

  /* Write long collector */
  if (CDlpTable_GetNRecs(_this->m_lpiLongCollector) > 0)
  {
    sprintf(buffer, "LONG=%hd,%hd ", (short)_this->m_nKnr, (short)_this->m_nRnr);
    buffer += strlen(buffer);
    if(CDN3Stream_SerializeTable(_this, _this->m_lpiLongCollector) != O_K) return NOT_EXEC;
  }

  /* Write char collector */
  if (CDlpTable_GetNRecs(_this->m_lpiCharCollector) > 0)
  {
    sprintf(buffer, "CHAR=%hd,%hd ", (short)_this->m_nKnr, (short)_this->m_nRnr);
    buffer += strlen(buffer);
    if(CDN3Stream_SerializeTable(_this, _this->m_lpiCharCollector) != O_K) return NOT_EXEC;
  }

  return O_K;
}

/**
 * Write global information string.
 *
 * @param _this      This instance
 * @return O_K if successful, an error code otherwise
 */
INT16 CDN3Stream_WriteGlobalInf(CDN3Stream* _this)
{
  if(_this->m_lpsGlobal)
  {
    dlp_memset(_this->m_lpParms, 0, sizeof (DPARA));
    dparmset(_this->m_lpParms, PA_XT, _this->m_lpsGlobal, strlen(_this->m_lpsGlobal)+1);

    if ((_this->m_lpDnfile=dopen(_this->m_lpsFilename, "a", _this->m_lpParms))==NULL)
      return FALSE;

    dclose(_this->m_lpDnfile, _this->m_lpParms);
    return TRUE;
  }
  else return NOT_EXEC;
}

/**
 * Parse global information string and fill lpsBuffer with the string associated
 * with lpsKey.
 *
 * @param _this      This instance
 * @param lpsKey     The key
 * @param lpsBuffer  The character buffer to fill
 * @param nBufferLen The maximal number of characters to be written (including the
 *                   terminal null)
 * @return O_K if successful, an error code otherwise
 */
INT16 CDN3Stream_ParseGlobal(CDN3Stream* _this, const char* lpsKey, char* lpsBuffer, INT16 nBufferLen)
{
  char* b  = NULL;
  char* tx = NULL;
  char* ty = NULL;

  /* find key */
  b = strstr(_this->m_lpsGlobal,lpsKey);
  if (!b) return NOT_EXEC;

  /* copy value */
  b += strlen(lpsKey);
  dlp_memset(lpsBuffer,0,nBufferLen);
  tx = b;
  ty = lpsBuffer;
  while (*tx!=' ') *ty++ = *tx++;

  return O_K;
}

/**
 * Process next object.
 *
 * @param _this This instance
 * @return O_K if successful, an error code otherwise
 */
INT16 CDN3Stream_NextObject(CDN3Stream* _this)
{
  if (!++(_this->m_nRnr))
  {
    _this->m_nRnr++;
    if (!++(_this->m_nKnr)) return FALSE;
  }
  return TRUE;
}

/**
 * Serialize dlpTable instance.
 *
 * @param _this  This instance
 * @param lpiSrc Pointer to table to serialize
 * @return O_K if successful, an error code otherwise
 */
INT16 CDN3Stream_SerializeTable(CDN3Stream* _this, CDlpTable* lpiSrc)
{
  if(!_this                                ) return NOT_EXEC;
  if(!lpiSrc                               ) return NOT_EXEC;
  if(_this->m_nKnr < 0 || _this->m_nRnr < 0) return NOT_EXEC;

  /* clear dnorm param struct */
  if(!_this->m_lpParms) return NOT_EXEC;
  dlp_memset(_this->m_lpParms,0,sizeof(DPARA));

  /* open dnorm file */
  _this->m_lpDnfile = dopen(_this->m_lpsFilename, "a", _this->m_lpParms);
  if(!_this->m_lpDnfile) return NOT_EXEC;

  if(CDN3Stream_SetParms(_this,lpiSrc) != O_K)
  {
    dclose(_this->m_lpDnfile,_this->m_lpParms);
    return NOT_EXEC;
  }

  if(dset(_this->m_lpDnfile,_this->m_nKnr,_this->m_nRnr,_this->m_lpParms) == EOF)
  {
    dclose(_this->m_lpDnfile,_this->m_lpParms);
    return NOT_EXEC;
  }

  /* write data in a temporary buffer and transfer to destination */
  /* via xfer-functions (because of DNorm-Bug)                    */
  if(CDlpTable_GetRecLen(lpiSrc) > 0 && lpiSrc->m_nrec)
  {
    INT32  i      = 0;
    INT32  nRLn   = 0;
    INT32  nRLnDn = 0;
    INT32  nRec   = lpiSrc->m_nrec;
    INT32  nBRec  = 0;
    INT32  nBlk   = 0;
    INT32  nMod   = 0;
    INT64  nWrite = 0;
    BYTE* lpBuf = NULL;

    nRLn   = CDlpTable_GetRecLen(lpiSrc);
    nRLnDn = CDN3Stream_GetDNRecLen(_this,lpiSrc);
    nBRec  = CDN3_XFERBUFLEN/nRLn;          /* number of records fitting in the buffer */
    nBlk   = (INT32)(nRec/nBRec);            /* number of blocks to write */
    nMod   = nRec%nBRec;                    /* last block has nMod records */

    lpBuf = (BYTE*)dlp_calloc(nRLn*nBRec,sizeof(BYTE));
    if (!lpBuf) return NOT_EXEC;

    /* write nBlk blocks each containing nBRec records */
    for(i=0; i<nBlk; i++)
    {
      if(NOK(_CDN3Stream_XferWrbuf(_this,lpiSrc,lpBuf,CDlpTable_XAddr(lpiSrc,i*nBRec,0),nBRec))) return NOT_EXEC;
      nWrite += dwrite(lpBuf,nRLnDn,nBRec,_this->m_lpDnfile);
    }

    /* write remaining nMod records */
    if(nMod)
    {
      if(NOK(_CDN3Stream_XferWrbuf(_this,lpiSrc,lpBuf,CDlpTable_XAddr(lpiSrc,nBlk*nBRec,0),nMod))) return NOT_EXEC;
      nWrite += dwrite(lpBuf,nRLnDn,nMod,_this->m_lpDnfile);
    }

    dlp_free(lpBuf);

    if(nWrite != nRec)
    {
      dclose(_this->m_lpDnfile, _this->m_lpParms);
      _this->m_lpDnfile = NULL;
      return NOT_EXEC;
    }
  }

  /* close file and free param struct */
  if(dclose(_this->m_lpDnfile, _this->m_lpParms))
  {
    _this->m_lpDnfile = NULL;
    return NOT_EXEC;
  }

  _this->m_lpDnfile = NULL;

  /* increment rnr in stream */
  CDN3Stream_NextObject(_this);

  return O_K;
}

/**
 * Deserializes a table (instance of class CDlpTable) from the Dnorm 3 stream.
 *
 * <h3>Notes</h3>
 * <ul>
 *   <li>The fields <code>m_nKnr</code> and <code>m_nRnr</code> must be set to
 *       select class and realization numbers prior to calling this method.</li>
 * </ul>
 *
 * @param _this  This instance
 * @param lpiDst Pointer to table to deserialize
 * @return <p><code>O_K</code> if successful, an error code otherwise:</p>
 * <table>
 *   <tr><th>Value</th><th>Error code</th><th>Description</th></tr>
 *   <tr><td>-2</td><td>-</td><td>Out of memory</td></tr>
 *   <tr><td>-1</td><td><code>NOT_EXEC</code></td><td>Other errors</td></tr>
 * </table>
 */
INT16 CDN3Stream_DeserializeTable(CDN3Stream* _this, CDlpTable* lpiDst)
{
  INT32 nRLnDn = 0;
  INT32 nRec   = 0;

  if(!_this                                ) return NOT_EXEC;
  if(!lpiDst                               ) return NOT_EXEC;
  if(_this->m_nKnr < 0 || _this->m_nRnr < 0) return NOT_EXEC;

  CDlpTable_Reset(lpiDst);

  /* clear dnorm param struct */
  dlp_memset(_this->m_lpParms,0,sizeof(DPARA));

  /* open dnorm file */
  _this->m_lpDnfile = dopen(_this->m_lpsFilename, "r", _this->m_lpParms);
  if(!_this->m_lpDnfile) return NOT_EXEC;

  /* lookup of rnr and knr */
  if(dget(_this->m_lpDnfile, _this->m_nKnr, _this->m_nRnr, _this->m_lpParms) == EOF)
  {
    dclose(_this->m_lpDnfile, _this->m_lpParms);
    return NOT_EXEC;
  }

  if(CDN3Stream_GetParms(_this, lpiDst) != O_K)
  {
    dclose(_this->m_lpDnfile, _this->m_lpParms);
    return NOT_EXEC;
  }

  /* allocate memory */
  nRec   = CDlpTable_GetNRecs(lpiDst);
  nRLnDn = CDN3Stream_GetDNRecLen(_this,lpiDst);
  CDlpTable_Allocate(lpiDst, nRec);
  if (CDlpTable_XAddr(lpiDst,0,0)==NULL)
  {
    dclose(_this->m_lpDnfile, _this->m_lpParms);
    return -2;
  }

  {
    /* read data in a temporary buffer and transfer to destination */
    /* via xfer-functions (because of DNorm-Bug) */
    INT32  i      = 0;
    INT32  nBRec  = CDN3_XFERBUFLEN/nRLnDn;       /* number of records fitting in the buffer */
    INT32  nBlk   = (INT32)(nRec/nBRec);          /* number of blocks to read */
    INT32  nMod   = nRec%nBRec;                   /* last block has nMod records */
    INT64  nRead  = 0;
    BYTE* lpBuf = NULL;

    lpBuf = (BYTE*)dlp_calloc(nRLnDn*nBRec,sizeof(BYTE));
    if (lpBuf == NULL)
    {
      dclose(_this->m_lpDnfile, _this->m_lpParms);
      return -2;
    }

    /* read nBlk blocks each containing nBRec records */
    for(i=0; i<nBlk; i++)
    {
      nRead += dread(lpBuf,nRLnDn,nBRec,_this->m_lpDnfile);
      _CDN3Stream_XferRdbuf(_this,lpiDst,CDlpTable_XAddr(lpiDst,i*nBRec,0),lpBuf,nBRec);
    }

    /* read remaining nMod records */
    if(nMod)
    {
      nRead += dread(lpBuf,nRLnDn,nMod,_this->m_lpDnfile);
      _CDN3Stream_XferRdbuf(_this,lpiDst,CDlpTable_XAddr(lpiDst,nBlk*nBRec,0),lpBuf,nMod);
    }

    dlp_free(lpBuf);

    if(nRead <= 0)
    {
      dclose(_this->m_lpDnfile, _this->m_lpParms);
      _this->m_lpDnfile = NULL;
      return NOT_EXEC;
    }
  }

  /* close file and free param struct */
  if(dclose(_this->m_lpDnfile, _this->m_lpParms))
  {
    _this->m_lpDnfile = NULL;
    return NOT_EXEC;
  }

  _this->m_lpDnfile = NULL;

  return O_K;
}

/**
 * Read DNorm parameter structure.
 *
 * @param _this      This instance
 * @param lpiDst     pointer to table
 * @return O_K if successful, an error code otherwise
 */
INT16 CDN3Stream_GetParms(CDN3Stream* _this, CDlpTable* lpiDst)
{
  INT32 i=0;

  _this->m_bOldStyle = FALSE;

  for(i=0; i<_this->m_lpParms->vdim; i++)
  {
    INT16 w       = _this->m_lpParms->rb[i].size;
    char  a       = _this->m_lpParms->rb[i].format;
    INT16 nType   = -1;
    char  name[5];
    

    switch(a) {
      case 'a': nType = w;       DLPASSERT(_this->m_bOldStyle==FALSE);break;
      case 'b': nType = T_UCHAR; DLPASSERT(_this->m_bOldStyle==FALSE);break;
      case 'c': nType = T_CHAR;  DLPASSERT(_this->m_bOldStyle==FALSE);break;
      case 'd': nType = T_USHORT;DLPASSERT(_this->m_bOldStyle==FALSE);break;
      case 'e': nType = T_SHORT; DLPASSERT(_this->m_bOldStyle==FALSE);break;
      case 'f': nType = T_UINT;  DLPASSERT(_this->m_bOldStyle==FALSE);break;
      case 'g': nType = T_INT;   DLPASSERT(_this->m_bOldStyle==FALSE);break;
      case 'h': nType = T_ULONG; DLPASSERT(_this->m_bOldStyle==FALSE);break;
      case 'i': nType = T_LONG;  DLPASSERT(_this->m_bOldStyle==FALSE);break;
      case 'j': nType = T_FLOAT; DLPASSERT(_this->m_bOldStyle==FALSE);break;
      case 'k': nType = T_DOUBLE;DLPASSERT(_this->m_bOldStyle==FALSE);break;
      case 'A': nType = w;
                _this->m_bOldStyle=TRUE;
                break;
      case 'U': switch(w) {
                  case  1: nType = T_UCHAR; break;
                  case  2: nType = T_USHORT;break;
                  case  4: nType = T_LONG;  break;
                  default: return NOT_EXEC;
                }
                _this->m_bOldStyle=TRUE;
                break;
      case 'I': switch(w) {
                  case  1: nType = T_CHAR; break;
                  case  2: nType = T_SHORT;break;
                  case  4: nType = T_LONG; break;
                  default: return NOT_EXEC;
                }
                _this->m_bOldStyle=TRUE;
                break;
      case 'R':
      case 'D': switch(w) {
                  case  4: nType = T_FLOAT; break;
                  case  8: nType = T_DOUBLE;break;
                  default: return NOT_EXEC;
                }
                _this->m_bOldStyle=TRUE;
                break;
      default: return NOT_EXEC;
    }

    name[0] = _this->m_lpParms->rb[i].name;
    name[1] = _this->m_lpParms->rb[i].name2;
    name[2] = _this->m_lpParms->rb[i].name3;
    name[3] = _this->m_lpParms->rb[i].name4;
    name[4] = 0;

    CDlpTable_AddComp(lpiDst,name,nType);
  }

  lpiDst->m_maxrec = _this->m_lpParms->vanz;
  lpiDst->m_nrec   = _this->m_lpParms->vanz;
  lpiDst->m_zf     = _this->m_lpParms->zf;
  lpiDst->m_fsr    = _this->m_lpParms->fsr;
  lpiDst->m_ofs    = _this->m_lpParms->ofs;
  lpiDst->m_descr0 = _this->m_lpParms->rres1;
  lpiDst->m_descr1 = _this->m_lpParms->rres2;
  lpiDst->m_descr2 = _this->m_lpParms->rres3;
  lpiDst->m_descr3 = _this->m_lpParms->rres4;
  lpiDst->m_descr4 = _this->m_lpParms->rres5;

  /* get realization description */
  if(0 != _this->m_lpParms->rt)
  {
    if(lpiDst->m_rtext)
    {
      dlp_free(lpiDst->m_rtext);
      lpiDst->m_rtext=NULL;
    }
    lpiDst->m_rtext = (char*)dlp_calloc(strlen(_this->m_lpParms->rt)+1,1);
    if(NULL == lpiDst->m_rtext) return NOT_EXEC;
    dlp_strcpy(lpiDst->m_rtext, _this->m_lpParms->rt);
  }

  /* get realization attribute description */
  if(0 != _this->m_lpParms->vrt)
  {
    if(lpiDst->m_vrtext)
    {
      dlp_free(lpiDst->m_vrtext);
      lpiDst->m_vrtext=NULL;
    }
    lpiDst->m_vrtext = (char*)dlp_calloc(strlen(_this->m_lpParms->vrt)+1,1);
    if(NULL == lpiDst->m_vrtext) return NOT_EXEC;
    dlp_strcpy(lpiDst->m_vrtext, _this->m_lpParms->vrt);
  }

  return O_K;
}

/**
 * Set DNorm parameter structure.
 *
 * @param _this      This instance
 * @param lpSrc      pointer to table
 * @return O_K if successful, an error code otherwise
 */
INT16 CDN3Stream_SetParms(CDN3Stream* _this, CDlpTable* lpiSrc)
{
  INT32  i    = 0;
  INT32  j    = 0;
  INT32  l    = 0;
  char  a    = '\0';
  char* name = NULL;

  if (!_this ) return NOT_EXEC;
  if (!lpiSrc) return NOT_EXEC;

  _this->m_lpParms->vdim  = lpiSrc->m_dim;
  _this->m_lpParms->vanz  = lpiSrc->m_maxrec;
  _this->m_lpParms->knr   = _this->m_nKnr;
  _this->m_lpParms->rnr   = _this->m_nRnr;
  _this->m_lpParms->vsize = lpiSrc->m_reclen;
  _this->m_lpParms->zf    = lpiSrc->m_zf;
  _this->m_lpParms->fsr   = lpiSrc->m_fsr;
  _this->m_lpParms->ofs   = lpiSrc->m_ofs;
  _this->m_lpParms->rres1 = lpiSrc->m_descr0;
  _this->m_lpParms->rres2 = lpiSrc->m_descr1;
  _this->m_lpParms->rres3 = lpiSrc->m_descr2;
  _this->m_lpParms->rres4 = lpiSrc->m_descr3;
  _this->m_lpParms->rres5 = lpiSrc->m_descr4;

  l = sizeof(RB) * lpiSrc->m_dim;

  if (l > _this->m_lpParms->lrb)
  {
    _this->m_lpParms->lrb  = (INT16)l;
    if(_this->m_lpParms->rb) free(_this->m_lpParms->rb);
    _this->m_lpParms->rb = (RB*)calloc(lpiSrc->m_dim,sizeof(RB));
    if(!_this->m_lpParms->rb) return NOT_EXEC;
  }

  for (i=0; i<lpiSrc->m_dim; i++)
  {
    j = CDlpTable_GetCompType(lpiSrc,i);
    a = 'a';
    switch (j)
    {
    case T_UCHAR  :  a = 'b' ;  break;
    case T_CHAR   :  a = 'c' ;  break;
    case T_USHORT :  a = 'd' ;  break;
    case T_SHORT  :  a = 'e' ;  break;
    case T_UINT   :  a = 'f' ;  break;
    case T_INT    :  a = 'g' ;  break;
    case T_ULONG  :  a = 'h' ;  break;
    case T_LONG   :  a = 'i' ;  break;
    case T_FLOAT  :  a = 'j' ;  break;
    case T_DOUBLE :  a = 'k' ;  break;
    }

    /* HACK: truncate _this->GetCompSize(i) to uchar (problem ?) */
    _this->m_lpParms->rb[i].size=(unsigned char)CDlpTable_GetCompSize(lpiSrc,i);

    _this->m_lpParms->rb[i].format = a;
    name = CDlpTable_GetCompName(lpiSrc,i);

    if (name != NULL)
    {
      _this->m_lpParms->rb[i].name  = name[0];
      _this->m_lpParms->rb[i].name2 = name[1];
      _this->m_lpParms->rb[i].name3 = name[2];
      _this->m_lpParms->rb[i].name4 = name[3];
    }
  }

  /* Set realization description */
  if (lpiSrc->m_rtext != 0)
    if (strlen(lpiSrc->m_rtext) > 0) dparmset(_this->m_lpParms,PA_RT,lpiSrc->m_rtext,dlp_strlen(lpiSrc->m_rtext));


  /* Set realization attribute description */
  if (lpiSrc->m_vrtext != 0)
    if (strlen(lpiSrc->m_vrtext) > 0) dparmset(_this->m_lpParms,PA_VRT,lpiSrc->m_vrtext,dlp_strlen(lpiSrc->m_vrtext));

  return O_K;
}

/**
 * Enter level.
 *
 * @param _this      This instance
 * @param lpName     name of field
 * @return O_K if successful, an error code otherwise
 */
INT16 CDN3Stream_EnterLevel(CDN3Stream* _this, char lpsName[L_NAMES])
{
  /* Save old container in list */
  _this->m_lpContainerList[_this->m_nLevel]  = _this->m_nContainer;
  _this->m_lpArrayIdxList[_this->m_nLevel++] = _this->m_nArrayIdx;

  /* Get new container index */
  _this->m_nContainer = CDN3Stream_LookupNameOnLevel(_this,lpsName);
  _this->m_nArrayIdx = 0;

  if (_this->m_nContainer == NOT_EXEC)  return CDN3_INTERNAL;
  if (_this->m_nLevel > CDN3_MAXLEVEL ) return CDN3_MAXLEVELREACHED;
  return O_K;
}

/**
 * Leave level.
 *
 * @param _this      This instance
 * @return O_K if successful, an error code otherwise
 */
INT16 CDN3Stream_LeaveLevel(CDN3Stream* _this)
{
  _this->m_nContainer = _this->m_lpContainerList[--_this->m_nLevel];
  _this->m_nArrayIdx  = _this->m_lpArrayIdxList[_this->m_nLevel];
  if (_this->m_nLevel < 0) return CDN3_INTERNAL;
  return O_K;
}



/*************************************************************************************
  the following functions are work arounds for a bug DNorm3.0
**************************************************************************************/

INT16 _CDN3Stream_XferRdbuf(CDN3Stream* _this, CDlpTable* lpiTable, BYTE* lpDst, BYTE* lpSrc, INT32 nRecs)
{
  INT32 nOffsDst = 0;
  INT32 nOffsSrc = 0;
  INT32 iRec     = 0;
  INT32 nComp    = 0;

  for(iRec=0; iRec<nRecs; iRec++)
  {
    for(nComp=0; nComp<CDlpTable_GetNComps(lpiTable); nComp++)
    {
      switch(CDlpTable_GetCompType(lpiTable,nComp))
      {
        case T_LONG:
          switch(_this->m_lpParms->rb[nComp].size) {
            case 4:
              *(INT64*)(lpDst+nOffsDst) = (INT64)*(int32_t*)(lpSrc+nOffsSrc);
              nOffsSrc += sizeof(int32_t);
              break;
            case 8:
              *(INT64*)(lpDst+nOffsDst) = (INT64)*(int64_t*)(lpSrc+nOffsSrc);
              nOffsSrc += sizeof(int64_t);
              break;
            default: return NOT_EXEC;
          }
          nOffsDst += sizeof(INT64);
          break;
        case T_INT:
          switch(_this->m_lpParms->rb[nComp].size) {
            case 4:
              *(INT32*)(lpDst+nOffsDst) = (INT32)*(int32_t*)(lpSrc+nOffsSrc);
              nOffsSrc += sizeof(int32_t);
              break;
            case 8:
              *(INT32*)(lpDst+nOffsDst) = (INT32)*(int64_t*)(lpSrc+nOffsSrc);
              nOffsSrc += sizeof(int64_t);
              break;
            default: return NOT_EXEC;
          }
          nOffsDst += sizeof(INT32);
          break;
        case T_FLOAT:
          switch(_this->m_lpParms->rb[nComp].size) {
            case 4:
              *(FLOAT32*)(lpDst+nOffsDst) = (FLOAT32)*(float*)(lpSrc+nOffsSrc);
              nOffsSrc += sizeof(float);
              break;
            case 8:
              *(FLOAT32*)(lpDst+nOffsDst) = (FLOAT32)*(double*)(lpSrc+nOffsSrc);
              nOffsSrc += sizeof(double);
              break;
            default: return NOT_EXEC;
          }
          nOffsDst += sizeof(FLOAT32);
          break;
        case T_DOUBLE:
          switch(_this->m_lpParms->rb[nComp].size) {
            case 4:
              *(FLOAT64*)(lpDst+nOffsDst) = (FLOAT64)*(float*)(lpSrc+nOffsSrc);
              nOffsSrc += sizeof(float);
              break;
            case 8:
              *(FLOAT64*)(lpDst+nOffsDst) = (FLOAT64)*(double*)(lpSrc+nOffsSrc);
              nOffsSrc += sizeof(double);
              break;
            default: return NOT_EXEC;
          }
          nOffsDst += sizeof(FLOAT64);
          break;
        default:
          memmove(lpDst+nOffsDst,lpSrc+nOffsSrc,CDlpTable_GetCompSize(lpiTable,nComp));
          nOffsSrc += CDlpTable_GetCompSize(lpiTable,nComp);
          nOffsDst += CDlpTable_GetCompSize(lpiTable,nComp);
      }
    }
  }

  return O_K;
}

INT16 _CDN3Stream_XferWrbuf(CDN3Stream* _this, CDlpTable* lpiTable, BYTE* lpDst, BYTE* lpSrc, INT32 nRecs)
{
  INT32 nOffsDst = 0;
  INT32 nOffsSrc = 0;
  INT32 iRec     = 0;
  INT32 nComp    = 0;

  for(iRec=0; iRec<nRecs; iRec++) {
    for(nComp=0; nComp<CDlpTable_GetNComps(lpiTable); nComp++) {
      memmove(lpDst+nOffsDst,lpSrc+nOffsSrc,CDlpTable_GetCompSize(lpiTable,nComp));
      nOffsSrc += CDlpTable_GetCompSize(lpiTable,nComp);
      nOffsDst += CDlpTable_GetCompSize(lpiTable,nComp);
    }
  }

  return O_K;
}

/**
 * Calculates the length in bytes of a DNorm record corresponding to the
 * component structure of lpiTable. The function takes into account that
 * long integers have 32 bits in DNorm regardless of their actual platform
 * dependent length.
 *
 * @param lpiTable A pointer to a CDlpTable instance
 * @return The length in bytes of a DNorm record for lpiTable
 */
INT32 CDN3Stream_GetDNRecLen(CDN3Stream* _this,CDlpTable* lpiTable)
{
  INT32 nRLnDn;

  if (!lpiTable || !_this->m_lpParms) return 0;

  nRLnDn = _this->m_lpParms->vsize;

  return nRLnDn;
}

#endif
/* EOF */
