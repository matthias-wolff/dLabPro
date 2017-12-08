/* dLabPro data table library
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

#ifndef DLPTABLE_H
#define DLPTABLE_H

#include "dlp_base.h"

/* Defines */
#define COMP_DESCR_LEN    255
#define CDN3_XFERBUFLEN 20000
#define COMP_ALLOC         20

/* Force C linkage in C++ units */
#ifdef __cplusplus
extern "C" {
#endif /* #ifdef __cplusplus */

/* Struct SDlpTableComp - Describes one table component */
typedef struct tag_SDlpTableComp
{
  char  lpName[COMP_DESCR_LEN];
  INT16 ctype;
  INT64  size;
  INT64  offset;
} SDlpTableComp;

/* Class CDlpTable - Fields */
typedef struct tag_CDlpTable
{
  INT32           m_dim;
  INT32           m_maxdim;
  INT32           m_nrec;
  INT32           m_maxrec;
  INT32           m_reclen;
  FLOAT64         m_descr0;
  FLOAT64         m_descr1;
  FLOAT64         m_descr2;
  FLOAT64         m_descr3;
  FLOAT64         m_descr4;
  FLOAT64         m_fsr;
  FLOAT64         m_zf;
  FLOAT64         m_ofs;
  char*          m_rtext;
  char*          m_vrtext;
  BYTE*          m_theDataPointer;
  SDlpTableComp* m_compDescrList;
} CDlpTable;

/* Class CDlpTable - Methods */
CDlpTable*     CDlpTable_CreateInstance(void);
INT16          CDlpTable_DestroyInstance(CDlpTable*);
INT16          CDlpTable_Constructor(CDlpTable*);
INT16          CDlpTable_Destructor(CDlpTable*);
INT16          CDlpTable_Init(CDlpTable*);
INT16          CDlpTable_SReset(CDlpTable*);
INT16          CDlpTable_Reset(CDlpTable*);
INT16          CDlpTable_Copy(CDlpTable*, CDlpTable* lpiSrc, INT32 nFirstRec, INT32 nRecs);
INT16          CDlpTable_Scopy(CDlpTable*, CDlpTable* lpiDst);
INT16          CDlpTable_Dcopy(CDlpTable*, CDlpTable* lpiSrc);

INT16          CDlpTable_InsertNcomps(CDlpTable*, const char* lpsName, INT16 nType, INT32 nInsertAt, INT32 nCount);
INT16          CDlpTable_InsertComp(CDlpTable*, const char* lpsName, INT16 nType, INT32 nInsertAt);
INT16          CDlpTable_AddNcomps(CDlpTable*, INT16 nType, INT32 nCount);
INT16          CDlpTable_AddComp(CDlpTable*, const char* lpsName, INT16 nType);
INT16          CDlpTable_Allocate(CDlpTable*, INT32 nRecs);
INT16          CDlpTable_AllocateUninitialized(CDlpTable*, INT32 nRecs);
INT16          CDlpTable_Alloc(CDlpTable*, INT32 nRecs);
INT16          CDlpTable_AllocUninitialized(CDlpTable*, INT32 nRecs);
INT16          CDlpTable_Realloc(CDlpTable*, INT32 nRecs);
INT32          CDlpTable_AddRecs(CDlpTable*, INT32 nRecs, INT32 nRealloc);
INT32          CDlpTable_InsertRecs(CDlpTable*, INT32 nInsertAt, INT32 nRecs, INT32 nRealloc);
INT16          CDlpTable_Clear(CDlpTable*);
INT16          CDlpTable_IsEmpty(CDlpTable*);

INT32          CDlpTable_GetNRecs(CDlpTable*);
INT32          CDlpTable_IncNRecs(CDlpTable*, INT32 nRecs);
INT32          CDlpTable_SetNRecs(CDlpTable*, INT32 nRecs);
INT32          CDlpTable_GetMaxRecs(CDlpTable*);
INT32          CDlpTable_GetRecLen(CDlpTable*);
INT32          CDlpTable_GetNComps(CDlpTable*);
INT32          CDlpTable_GetCompSize(CDlpTable*, INT32 nComp);
INT64          CDlpTable_GetCompOffset(CDlpTable*, INT32 nComp);
INT16          CDlpTable_GetCompType(CDlpTable*, INT32 nComp);
char*          CDlpTable_GetCompName(CDlpTable*, INT32 nComp);
INT16          CDlpTable_SetCompName(CDlpTable*, INT32 nComp, const char* lpsName);
FLOAT64        CDlpTable_GetFsr(CDlpTable*);

SDlpTableComp* CDlpTable_FindCompByName(CDlpTable*, const char* lpsName);
SDlpTableComp* CDlpTable_FindCompByIdx(CDlpTable*, INT32 nComp);
INT32          CDlpTable_CompNameToIdx(CDlpTable*, const char* lpsName);

BYTE*          CDlpTable_XAddr(CDlpTable*, INT32 nRec, INT32 nComp);
COMPLEX64      CDlpTable_Cfetch(CDlpTable*, INT32 nRec, INT32 nComp);
FLOAT64        CDlpTable_Dfetch(CDlpTable*, INT32 nRec, INT32 nComp);
INT16          CDlpTable_Cstore(CDlpTable*, COMPLEX64 nVal, INT32 nRec, INT32 nComp);
INT16          CDlpTable_Dstore(CDlpTable*, FLOAT64 nVal, INT32 nRec, INT32 nComp);
void*          CDlpTable_Pfetch(CDlpTable*, INT32 nRec, INT32 nComp);
INT16          CDlpTable_Pstore(CDlpTable*, void* lpsVal, INT32 nRec, INT32 nComp);
char*          CDlpTable_Sfetch(CDlpTable*, INT32 nRec, INT32 nComp);
INT16          CDlpTable_Sstore(CDlpTable*, const char* lpsVal, INT32 nRec, INT32 nComp);

INT16          CDlpTable_DeleteRecs(CDlpTable*, CDlpTable* lpiSrc, INT32 nFirstRec, INT32 nRecs);
INT16          CDlpTable_DeleteComps(CDlpTable*, CDlpTable* lpiSrc, INT32 nFirstComp, INT32 nComps);
INT16          CDlpTable_Join(CDlpTable*, CDlpTable* lpiSrc);
INT16          CDlpTable_Cat(CDlpTable*, CDlpTable* lpiSrc);
INT16          CDlpTable_CatEx(CDlpTable*, CDlpTable* lpiSrc, INT32 nFirstRec, INT32 nCount);

INT16          CDlpTable_Descr(CDlpTable*);
INT16          CDlpTable_Print(CDlpTable*);

#ifdef __cplusplus
}
#endif /* #ifdef __cplusplus */

#endif /* #ifndef DLPTABLE_H */

/* EOF */
