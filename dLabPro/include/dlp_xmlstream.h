/* dLabPro XML stream library
 * - Header file
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

#ifndef __XMLSTREAM_H
#define __XMLSTREAM_H

#include "dlp_base.h"
#include "dlp_table.h"
#include "kzl_hash.h"
#include "xpat.h"

/* Defines - CXmlStream modes */
#define XMLS_READ          0x0001
#define XMLS_WRITE         0x0002
#define XMLS_ZIPPED        0x0004

/* Defines - DOM object types */
#define XMLS_DT_INSTANCE   0x0001
#define XMLS_DT_NULLINST   0x0002
#define XMLS_DT_FIELD      0x0003
#define XMLS_DT_TABLE      0x0004

/* Constants */
#define XMLS_BUFFERLEN     8192
#define L_DOMOBJECTNAME    512

/* TODO: Move to dlp_base.h? */
#ifndef max
  #define max(a,b)    (((a) > (b)) ? (a) : (b))
#endif
#ifndef min
  #define min(a,b)    (((a) < (b)) ? (a) : (b))
#endif

/* Defines - Error codes */
#define XMLSERR_NOMEM      -11000
#define XMLSERR_READ       -11001
#define XMLSERR_INFILE     -11002
#define XMLSERR_PARSE      -11003
#define XMLSERR_TAGUNKNOWN -11004
#define XMLSERR_MISPLACED  -11005
#define XMLSERR_MISSATTR   -11006
#define XMLSERR_INTERNAL   -11007
#define XMLSERR_BADTYPE    -11008

/* Macros */
#define XML_INDENT_LINE(A,B) { short i=0; for (i=0; i<B; i++) dlp_fprintf(A,"\t"); }

/* Class CXmlStream */
#ifdef __cplusplus
extern "C" {
#endif

typedef struct SDomObject
{
  short nType;                        /* Object type (field or table)          */
  char  lpsName[L_DOMOBJECTNAME];     /* Fully qualified name of field         */
  char  lpsType[L_NAMES];             /* Variable type of value                */
  long  nArrlen;                      /* Array length of value                 */
  void* lpValue;                      /* Object value, char* or dlpTable*      */
  BOOL  bUsed;                        /* Value was used during deserialization */
} SDomObject;

typedef struct CXmlStream
{
  /* -- Fields -- */
  XML_Parser* m_lpiParser;            /* The XML parser instance                         */
  hash_t*     m_lpDom;                /* Document object model                           */
  DLP_FILE*   m_lpFile;               /* Associated file                                 */
  char        m_lpsFileName[L_PATH];  /* Name of associated file                         */
  short       m_nDepth;               /* Tag nesting level                               */
  char        m_lpsInameFq[255];      /* Fully qualified name of current instance        */
  char        m_lpsRootInameFq[255];  /* Fully qualified name of root instance for deser.*/
                                      /* (initialized if first instance is deserial.'ed  */
  SDomObject* m_lpCurObject;          /* Parser: currently parsed field or table         */
  int         m_nMode;                /* XMLS_WRITE or XMLS_READ                         */
  char*       m_lpsBuffer;            /* Multi-function text buffer, freed by destructor */
  char*       m_lpsXmlText;           /* Parser: XML text buffer,  freed by destructor   */
  BOOL        m_bCell;                /* TRUE between <CELL> and </CELL> tags            */
  long        m_nCompCtr;             /* Counts cells between <REC> and </REC> tags      */
  long        m_nFieldsNotFound;      /* Deserialize: no. of fields not found in stream  */

} CXmlStream;

/* Methods - Constructors and destructors */
BOOL        CXmlStream_CheckIsXml(const char* lpsFilename, const int nMode);
CXmlStream* CXmlStream_CreateInstance(const char* lpsFilename, const int nMode);
short       CXmlStream_Constructor(CXmlStream*, const char* lpsFilename, const int nMode);
short       CXmlStream_DestroyInstance(CXmlStream*);
short       CXmlStream_Destructor(CXmlStream*);

/* Methods - Expat event handlers */
void        CXmlStream_OnOpenTag(void* __this, const char* lpsElement, const char** lpAttr);
void        CXmlStream_OnCloseTag(void* __this, const char* lpsElement);
void        CXmlStream_OnText(void* __this, const XML_Char* lpsText, int nLen);

/* Methods - Handle instances and tables */
short       CXmlStream_BeginInstance(CXmlStream*, const char* lpsInstanceName, const char* lpsClassName);
short       CXmlStream_EndInstance(CXmlStream*);
short       CXmlStream_SerializeTable(CXmlStream*, CDlpTable* lpiTable, const char* lpsFqName);

/* Methods - DOM */
SDomObject* CXmlStream_FindObject(CXmlStream*, const char* lpsFqName);
void        CXmlStream_ObjectNotFound(CXmlStream*);

/* Methods - Character encoding */
const char* CXmlStream_Encode(CXmlStream*, const char* lpsText);

#ifdef __cplusplus
}
#endif

#endif /* __XMLSTREAM_H */

/* EOF */
