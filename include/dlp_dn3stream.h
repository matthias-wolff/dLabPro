/* dLabPro DNorm3 stream library
 * - Header file
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

#ifndef __DN3STREAM_H
#define __DN3STREAM_H

#include "dlp_base.h"
#include "dlp_table.h"
#include "dlp_dnorm.h"

/* Defines */
#define CDN3_READ  0
#define CDN3_WRITE 1

#define COMP_DESCR_LEN          8
#define CDN3_XFERBUFLEN         20000
#define CDN3_MAXLEVEL           20
#define CDN3_GLOBLENGTH         200
#define CDN3_FILENAME           L_PATH
#define OF_NAME                 0       /* Identifier      */
#define OF_TYPENAME             1       /* RTTI            */
#define OF_NTYPE                2       /* Type code       */
#define OF_CONTAINER            3       /* Container index */
#define OF_ARRLEN               4       /* Array length    */
#define OF_RNR                  5       /* Realization no. */
#define OF_KNR                  6       /* Class no.       */
#define OF_COLLIDX              7       /* Collector index */
#define OF_ARRIDX               8       /* Array index     */
#define CDN3_GRANULARITY        20

/* Defines - Error codes */
#undef CDN3_CANNOTREADGLOB    
#undef CDN3_NOMEM             
#undef CDN3_MISSCLASS         
#undef CDN3_WRONGCLASS        
#undef CDN3_TYPEUNSAFE        
#undef CDN3_CANNOTREADTIT     
#undef CDN3_CANNOTREADCOLL    
#undef CDN3_FILENOTCREATE     
#undef CDN3_FILEOPEN
#undef CDN3_INTERNAL
#undef CDN3_MAXLEVELREACHED
#define CDN3_CANNOTREADGLOB    -1001
#define CDN3_NOMEM             -1002
#define CDN3_MISSCLASS         -1003
#define CDN3_WRONGCLASS        -1004
#define CDN3_TYPEUNSAFE        -1005
#define CDN3_CANNOTREADTIT     -1006
#define CDN3_CANNOTREADCOLL    -1007
#define CDN3_FILENOTCREATE     -1008
#define CDN3_INTERNAL          -1009
#define CDN3_MAXLEVELREACHED   -1010
#define CDN3_FILEOPEN          -1011

#ifdef __cplusplus
extern "C" {
#endif

/* Class CDN3Stream */
typedef struct CDN3Stream
{
  /* Fields */
  BOOL       m_bOldStyle;                      /* Compatibility mode                           */
  BOOL       m_bWriteCol;                      /* Enable/disable writing of collectors         */
  INT16      m_nMode;                          /* Mode (read/write) as used while constructing */
  INT16      m_nTypesafe;                      /* Typesafe?                                    */
  INT16      m_nKnr;                           /* Class number                                 */
  INT16      m_nRnr;                           /* Realization number                           */
  char       m_lpsFilename[CDN3_FILENAME];     /* Filename as used while constructing          */
  char*      m_lpsGlobal;                      /* Global information string                    */
  char       m_lpsFileclass[L_NAMES];          /* Class of file                                */
  INT64      m_lpContainerList[CDN3_MAXLEVEL]; /* */
  INT64      m_lpArrayIdxList[CDN3_MAXLEVEL];  /* */
  INT32      m_nLevel;                         /* */
  INT32      m_nArrayIdx;                      /* */
  INT32      m_nContainer;                     /* */
  DPARA*     m_lpParms;                        /* */
  DNORM_DCB* m_lpDnfile;                       /* */
  CDlpTable* m_lpiTIT;                         /* Type information table                       */
  CDlpTable* m_lpiDoubleCollector;             /* Double Collector                             */
  CDlpTable* m_lpiLongCollector;               /* Long Collector                               */
  CDlpTable* m_lpiCharCollector;               /* Char Collector                               */
} CDN3Stream;

/* Methods */
CDN3Stream* CDN3Stream_CreateInstance(const char* lpsFilename, const INT16 nMode, const char* lpsFileclass);
INT16       CDN3Stream_DestroyInstance(CDN3Stream*);
INT16       CDN3Stream_Constructor(CDN3Stream*, const char* lpsFilename, const INT16 nMode, const char* lpsFileclass);
INT16       CDN3Stream_Destructor(CDN3Stream*);
INT16       CDN3Stream_WriteCollector(CDN3Stream* _this, INT16 nType, void* lpData, INT32 nArrayLen);
INT16       CDN3Stream_AddType(CDN3Stream* _this, const char* lpsName, const char* lpsTypeName, INT16 nType, INT32 nArrayLen);
INT32       CDN3Stream_LookupNameOnLevel(CDN3Stream*, const char* lpsName);
INT32       CDN3Stream_SaveCollectors(CDN3Stream*);
INT16       CDN3Stream_ReadCollectors(CDN3Stream*);
INT16       CDN3Stream_WriteGlobalInf(CDN3Stream*);
INT16       CDN3Stream_ParseGlobal(CDN3Stream*, const char* lpsKey, char* lpsBuffer, INT16 nBufferLen);
INT16       CDN3Stream_NextObject(CDN3Stream*);
INT16       CDN3Stream_SetParms(CDN3Stream*, CDlpTable* lpiSrc);
INT16       CDN3Stream_GetParms(CDN3Stream*, CDlpTable* lpiDst);
INT16       CDN3Stream_SerializeTable(CDN3Stream*, CDlpTable* lpiSrc);
INT16       CDN3Stream_DeserializeTable(CDN3Stream*, CDlpTable* lpiDst);
INT16       CDN3Stream_EnterLevel(CDN3Stream*, char lpsName[L_NAMES]);
INT16       CDN3Stream_LeaveLevel(CDN3Stream*);
INT16       CDN3Stream_GetFileClass(CDN3Stream*,char *lpsClassName);
INT16       CDN3Stream_VerifyFileClass(CDN3Stream*);
INT16      _CDN3Stream_XferWrbuf(CDN3Stream* _this, CDlpTable* lpiTable, BYTE* lpDest, BYTE* lpSrc, INT32 nRecs);
INT16      _CDN3Stream_XferRdbuf(CDN3Stream* _this, CDlpTable* lpiTable, BYTE* lpDest, BYTE* lpSrc, INT32 nRecs);
INT32       CDN3Stream_GetDNRecLen(CDN3Stream* _this, CDlpTable* lpiTable);

#ifdef __cplusplus
}
#endif

#endif  /* __DN3STREAM_H */

/* EOF */
