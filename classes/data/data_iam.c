/* dLabPro class CData (data)
 * - Class CData interactive methods (user defined SMI functions)
 *
 * AUTHOR : Matthias Eichner
 * PACKAGE: dLabPro/classes
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
#include "dlp_data.h"

INT16 CGEN_PUBLIC CData_AddComp(CData* _this, const char* lpsName, INT16 nType)
{
  CHECK_THIS_RV(NOT_EXEC);

  if(O_K != CDlpTable_AddComp(_this->m_lpTable,lpsName,nType)) return NOT_EXEC;

  return O_K;
}

INT16 CGEN_PUBLIC CData_AddNcomps(CData* _this, INT16 nCType, INT32 nCount)
{
  CHECK_THIS_RV(NOT_EXEC);

  if(O_K != CDlpTable_AddNcomps(_this->m_lpTable, nCType, nCount)) return NOT_EXEC;

  return O_K;
}

INT16 CGEN_PUBLIC CData_InsertComp(CData* _this, const char* sCName, INT16 nCType, INT32 nInsertAt)
{
  CHECK_THIS_RV(NOT_EXEC);
  return CDlpTable_InsertComp(_this->m_lpTable,sCName,nCType,nInsertAt);
}

INT16 CGEN_PUBLIC CData_InsertNcomps(CData* _this, INT16 nCType, INT32 nInsertAt, INT32 nCount)
{
  CHECK_THIS_RV(NOT_EXEC);
  return CDlpTable_InsertNcomps(_this->m_lpTable,"",nCType,nInsertAt,nCount);
}

INT16 CGEN_PUBLIC CData_GetCompType(CData* _this, INT32 nComp)
{
  CHECK_THIS_RV(0);
  return CDlpTable_GetCompType(_this->m_lpTable,nComp);
}

INT16 CGEN_PUBLIC CData_Scopy(CData* _this, data* lpSrc)
{
  CHECK_THIS_RV(NOT_EXEC);

  if (OK(CDlpTable_Scopy(_this->m_lpTable, lpSrc->m_lpTable)))
    return CData_CopyDescr(_this,lpSrc);
  else
    return NOT_EXEC;
}

INT16 CGEN_PUBLIC CData_CopyCnames(CData* _this, data* iSrc, INT32 jx, INT32 jy, INT32 n)
{
  INT32  i      = 0;
  char* lpName = NULL;

  CHECK_THIS_RV(NOT_EXEC);
  if (!iSrc) return NOT_EXEC;
  if (_this==iSrc) return O_K;

  CHECK_DATA(_this);
  CHECK_DATA(iSrc);

  for(i=0;i<n;i++)
  {
    lpName = CDlpTable_GetCompName(iSrc->m_lpTable,jx+i);
    if (NULL == lpName) continue;
    IF_NOK(CData_SetCname(_this,jy+i,lpName))
      IERROR(_this,DATA_INTERNAL,"Cannot set component name",0,0);
  }

  return O_K;
}

INT16 CGEN_PUBLIC CData_SetCnames(CData* _this, data* x, INT32 jx)
{
  INT32 i      = 0;
  INT32 nComps = 0;
  INT32 nRecs  = 0;
  char sBuf[L_SSTR];

  CHECK_THIS_RV(NOT_EXEC);

  if (CData_IsEmpty(x))
    return IERROR(_this,DATA_EMPTY,BASEINST(x)->m_lpInstanceName,0,0);

  nComps = CData_GetNComps(_this);
  nRecs  = CData_GetNRecs(x);

  if (nComps <= 0)
    return IERROR(_this,DATA_BADCOMP,(int)jx,BASEINST(x)->m_lpInstanceName,0);
  /*if (dlp_is_symbolic_type_code(CData_GetCompType(x,jx)) != TRUE)
    return IERROR(_this,DATA_INTERNAL,"Not a symbolic component.",0,0);*/

  for(i=0; i<nComps && i<nRecs; i++)
  {
    dlp_sprintx(sBuf,(char*)CData_XAddr(x,i,jx),CData_GetCompType(x,jx),_this->m_bExact);
    CData_SetCname(_this,i,dlp_strtrimleft(dlp_strtrimright(sBuf)));
  }

  return O_K;
}

INT16 CGEN_PUBLIC CData_GetCnames(CData* _this, CData* idSrc)
{
  INT32 i = 0;

  CHECK_THIS_RV(NOT_EXEC);
  if (idSrc==_this)
  {
    ICREATEEX(CData,idSrc,"~temp",NULL);
    CData_Copy(BASEINST(idSrc),BASEINST(_this));
  }

  CData_Reset(BASEINST(_this),TRUE);
  CData_AddComp(_this,"cname",L_NAMES);
  CData_Allocate(_this,CData_GetNComps(idSrc));
  for (i=0; i<CData_GetNComps(idSrc); i++)
    CData_Sstore(_this,CData_GetCname(idSrc,i),i,0);

  if (strncmp(BASEINST(idSrc)->m_lpInstanceName,"~temp",L_NAMES)==0)
  {
    IDESTROY(idSrc);
  }
  return O_K;
}

INT16 CGEN_PUBLIC CData_SetCname(CData* _this, INT32 nIComp, const char* sCName)
{
  CHECK_THIS_RV(NOT_EXEC);

  return CDlpTable_SetCompName(_this->m_lpTable, nIComp, sCName);
}

const char* CGEN_PUBLIC CData_GetCname(CData* _this, INT32 nIComp)
{
  CHECK_THIS_RV(NULL);
  return CDlpTable_GetCompName(_this->m_lpTable, nIComp);
}

INT32 CGEN_PUBLIC CData_FindComp(CData* _this, const char* lpName)
{
  CHECK_THIS_RV(-1);

  return CDlpTable_CompNameToIdx(_this->m_lpTable, lpName);
}

INT16 CGEN_PUBLIC CData_Allocate(CData* _this, INT32 n)
{
  if(_this->m_bFast == TRUE) {
    return CData_AllocateUninitialized(_this, n);
  }
  CHECK_THIS_RV(NOT_EXEC);
  return CDlpTable_Allocate(_this->m_lpTable, n);
}

INT16 CGEN_PUBLIC CData_Array(CData* _this, INT16 nCType, INT32 nComps, INT32 nRecs)
{
  INT16       nErr     = O_K;
  const char* lpsToken = NULL;
  CHECK_THIS_RV(NOT_EXEC);

  if (nComps<=0 || nRecs<=0) return NOT_EXEC;

  CData_Reset(BASEINST(_this),TRUE);
  IF_NOK((nErr = CData_AddNcomps(_this,nCType,nComps))) return nErr;
  IF_NOK((nErr = CData_Allocate(_this,nRecs))         ) return nErr;

  lpsToken = MIC_NEXTTOKEN_FORCE;
  if (!lpsToken) return O_K;
  if (dlp_strcmp(lpsToken,"{")!=0) { MIC_REFUSETOKEN; return O_K; }
  CData_InitializeEx(_this,0,0,-1);
  return O_K;
}

BOOL CGEN_PUBLIC CData_IsEmpty(CData* _this)
{
  CHECK_THIS_RV(TRUE);
  return CDlpTable_IsEmpty(_this->m_lpTable);
}

COMPLEX64 CGEN_PUBLIC CData_Cfetch(CData* _this, INT32 nIRec, INT32 nIComp)
{
  CHECK_THIS_RV(CMPLX(0));

  return CDlpTable_Cfetch(_this->m_lpTable, nIRec, nIComp);
}

FLOAT64 CGEN_PUBLIC CData_Dfetch(CData* _this, INT32 nIRec, INT32 nIComp)
{
  CHECK_THIS_RV(0.);

  return CDlpTable_Dfetch(_this->m_lpTable, nIRec, nIComp);
}

void* CGEN_PUBLIC CData_Pfetch(CData* _this, INT32 nIRec, INT32 nIComp)
{
  CHECK_THIS_RV(NULL);

  return CDlpTable_Pfetch(_this->m_lpTable, nIRec, nIComp);
}

const char* CGEN_PUBLIC CData_Sfetch(CData* _this, INT32 nIRec, INT32 nIComp)
{
  CHECK_THIS_RV(NULL);

  return CDlpTable_Sfetch(_this->m_lpTable, nIRec, nIComp);
}

INT16 CGEN_PUBLIC CData_Cstore(CData* _this, COMPLEX64 dVal, INT32 nIRec, INT32 nIComp)
{
  CHECK_THIS_RV(NOT_EXEC);

  if(O_K != CDlpTable_Cstore(_this->m_lpTable, dVal, nIRec, nIComp)) return NOT_EXEC;

  return O_K;
}

INT16 CGEN_PUBLIC CData_Dstore(CData* _this, FLOAT64 dVal, INT32 nIRec, INT32 nIComp)
{
  CHECK_THIS_RV(NOT_EXEC);

  if(O_K != CDlpTable_Dstore(_this->m_lpTable, dVal, nIRec, nIComp)) return NOT_EXEC;

  return O_K;
}

INT16 CGEN_PUBLIC CData_Pstore(CData* _this, void* lpVal, INT32 nIRec, INT32 nIComp)
{
  CHECK_THIS_RV(NOT_EXEC);

  if(O_K != CDlpTable_Pstore(_this->m_lpTable, lpVal, nIRec, nIComp)) return NOT_EXEC;

  return O_K;
}

INT16 CGEN_PUBLIC CData_Sstore(CData* _this, const char* sVal, INT32 nIRec, INT32 nIComp)
{
  CHECK_THIS_RV(NOT_EXEC);

  if(O_K != CDlpTable_Sstore(_this->m_lpTable, sVal, nIRec, nIComp)) return NOT_EXEC;

  return O_K;
}

INT16 CGEN_PUBLIC CData_Clear(CData* _this)
{
  CHECK_THIS_RV(NOT_EXEC);

  if(O_K != CDlpTable_Clear(_this->m_lpTable)) return NOT_EXEC;

  return O_K;
}

INT16 CGEN_PUBLIC CData_Fill(CData* _this, COMPLEX64 dStart, COMPLEX64 dDelta)
{
  INT32     i      = 0;
  INT32     j      = 0;
  INT32     nXXN   = 0;
  INT32     nXC    = 0;
  COMPLEX64 dValue = dStart;
  COMPLEX64 nTmp   = CMPLX(0.0);
  BOOL      bStor  = FALSE;

  CHECK_THIS_RV(NOT_EXEC);

  if (CData_IsEmpty(_this)) return NOT_EXEC;
  if (_this->m_bMark && _this->m_markMode==CDATA_MARK_BLOCKS)
    return
      IERROR(_this,DATA_INTERNAL,"block marking not supported for -fill",0,0);

  if((dStart.y!=0.0) || (dDelta.y!=0.0)) CData_Tconvert(_this,_this,T_COMPLEX);

  nXXN = CData_GetMaxRecs(_this);
  nXC  = CData_GetNComps(_this);

  for (i=0; i<nXXN; i++)
  {
  	bStor = FALSE;
    if (!_this->m_bMark || _this->m_markMode!=CDATA_MARK_RECS || CData_RecIsMarked(_this,i))
    {
      for (j=0; j<nXC; j++)
      {
        if
        (
          (!_this->m_bMark || _this->m_markMode!=CDATA_MARK_COMPS || CData_CompIsMarked(_this,j)) &&
          dlp_is_numeric_type_code(CData_GetCompType(_this,j))
        )
        {
          if (!_this->m_bMark || _this->m_markMode!=CDATA_MARK_CELLS || CData_CellIsMarked(_this,nXC*i+j))
          {
            if (_this->m_bNoise) {
              nTmp = CMPLX_DIV_R(CMPLXY(dlp_rand(),dlp_rand()),RAND_MAX);
              CData_Cstore(_this,CMPLX_PLUS(dValue,nTmp),i,j);
            } else {
              CData_Cstore(_this,dValue,i,j);
            }
            bStor = TRUE;
          }
        }
      }
    }
    if (bStor)
    {
      dValue = CMPLX_PLUS(dValue,dDelta);
    }
  }

  return O_K;
}

INT16 CGEN_PUBLIC CData_Lookup(CData* _this, data* dSel, INT32 nSelComp, data *dTab, INT32 nTabComp, INT32 nCount)
{
  INT32 i                    = 0;
  INT32 nTo                  = 0;
  INT32 nIdx                 = 0;
  INT32 nRecs                = 0;
  INT32 nRecln               = 0;
  INT32 nTabLen              = 0;
  char lpsInit[L_INPUTLINE];

  CHECK_THIS_RV(NOT_EXEC);

  if (CData_IsEmpty(dSel))
    return
      IERROR(_this,DATA_EMPTY,dSel?BASEINST(dSel)->m_lpInstanceName:"NULL",0,0);
  if (CData_GetNComps(dSel)<=nSelComp)
    return
      IERROR(_this,DATA_BADCOMP,(int)nSelComp,BASEINST(dSel)->m_lpInstanceName,0);
  if(dTab && CData_GetNComps(dTab) < nTabComp+nCount)
    return
      IERROR(_this,DATA_BADCOMP,(int)(nTabComp+nCount),
        BASEINST(dTab)->m_lpInstanceName,0);

  if (dSel==_this)
  {
    ICREATEEX(CData,dSel,"~temp",NULL);
    CData_Copy(BASEINST(dSel),BASEINST(_this));
  } else if (dTab==_this)
  {
    ICREATEEX(CData,dTab,"~temp",NULL);
    CData_Copy(BASEINST(dTab),BASEINST(_this));
  }

  nRecs = CData_GetNRecs(dSel);
  CData_Reset(BASEINST(_this),TRUE);

  for (i=nTabComp; i<nTabComp+nCount; i++)
    if (dTab)
      CData_AddComp(_this,CData_GetCname(dTab,i),CData_GetCompType(dTab,i));
    else
      CData_AddComp(_this,"",255);

  nRecln = CData_GetRecLen(_this);
  CData_Allocate(_this,nRecs);
  nTabLen = CData_GetNRecs(dTab);

  CData_ReadInitializer(_this,lpsInit,L_INPUTLINE,FALSE);

  for (i=0,nTo=0;i<nRecs;i++,nTo++)
  {
    nIdx=(INT32)CData_Dfetch(dSel,i,nSelComp);
    if (nIdx > -1 && nIdx < nTabLen)
    {
      dlp_memmove(CData_XAddr(_this,nTo,0),CData_XAddr(dTab,nIdx,nTabComp),nRecln);
    }
    else if (dlp_strlen(lpsInit)>0) CData_InitializeRecordEx(_this,lpsInit,i,0);
  }

  CData_SetNRecs(_this, nTo);

  if (strncmp(BASEINST(dSel)->m_lpInstanceName,"~temp",L_NAMES)==0) {
    CData_CopyDescr(_this,dSel);
    IDESTROY(dSel);
  } else if(dTab && strncmp(BASEINST(dTab)->m_lpInstanceName,"~temp",L_NAMES)==0) {
    CData_CopyDescr(_this,dSel);
    IDESTROY(dTab);
  }

  return O_K;
}

INT16 CGEN_PUBLIC CData_Lookup2(CData* _this, data* dSel1, INT32 nSel1Comp, data* dSel2, INT32 nSel2Comp, data *dTab)
{
  INT32  i                   = 0;
  INT32  j                   = 0;
  INT32  nIdx1               = 0;
  INT32  nIdx2               = 0;
  INT32  nRecs               = 0;
  INT32  nComps              = 0;
  INT32  nTabLen             = 0;
  char lpsInit[L_INPUTLINE];
  INT16 bMatrix             = _this->m_bMatrix;

  CHECK_THIS_RV(NOT_EXEC);
  CData_ReadInitializer(_this,lpsInit,L_INPUTLINE,FALSE);

  if(CData_IsEmpty(dSel1) == TRUE)        return IERROR(_this,DATA_EMPTY,BASEINST(dSel1)->m_lpInstanceName,0,0);
  if(CData_GetNComps(dSel1) <= nSel1Comp) return IERROR(_this,DATA_BADCOMP,(int)nSel1Comp,BASEINST(dSel1)->m_lpInstanceName,0);
  if(CData_IsEmpty(dSel2) == TRUE)        return IERROR(_this,DATA_EMPTY,BASEINST(dSel2)->m_lpInstanceName,0,0);
  if(CData_GetNComps(dSel2) <= nSel2Comp) return IERROR(_this,DATA_BADCOMP,(int)nSel2Comp,BASEINST(dSel2)->m_lpInstanceName,0);
  if(CData_IsEmpty(dTab) == TRUE)         return IERROR(_this,DATA_EMPTY,BASEINST(dTab)->m_lpInstanceName,0,0);

  nRecs = CData_GetNRecs(dSel1);
  /*if (CData_GetNRecs(dSel2) < nRecs) nRecs = CData_GetNRecs(dSel2);*/

  nTabLen = CData_GetNRecs(dTab);
  nComps  = CData_GetNComps(dTab);

  CData_Reset(BASEINST(_this),TRUE);

  for (i=0; i<CData_GetNRecs(dSel2); i++)
  {
    nIdx2 = (INT32)CData_Dfetch(dSel2,i,nSel2Comp);
    if(nIdx2 > -1 && nIdx2 < nComps)
    {
      CData_AddComp(_this,CData_GetCname(dTab,nIdx2),CData_GetCompType(dTab,nIdx2));
      if(bMatrix == FALSE) break;
    }
  }

  if(CData_GetNComps(_this) == 0)
    return
      IERROR(_this,DATA_BADCOMP,(int)nSel2Comp,BASEINST(dSel2)->m_lpInstanceName,0);

  CData_AllocateUninitialized(_this,nRecs);

  for (i=0;i<nRecs;i++)
  {
    nIdx1=(INT32)CData_Dfetch(dSel1,i,nSel1Comp);

    if(bMatrix == TRUE)
    {
      for(j=0; j<CData_GetNRecs(dSel2); j++)
      {
        nIdx2=(INT32)CData_Dfetch(dSel2,j,nSel2Comp);
        if(nIdx2 > -1 && nIdx2 < nComps)
        {
          dlp_memmove(CData_XAddr(_this,i,j),CData_XAddr(dTab,nIdx1,nIdx2),CData_GetCompSize(_this,j));
        }
      }
    }
    else
    {
      nIdx2=(INT32)CData_Dfetch(dSel2,i,nSel2Comp);
      if (nIdx1>=0 && nIdx1<nTabLen && nIdx2>=0 && nIdx2<nComps)
        dlp_memmove(CData_XAddr(_this,i,0),CData_XAddr(dTab,nIdx1,nIdx2),CData_GetCompSize(_this,0));
      else if (dlp_strlen(lpsInit)>0) CData_InitializeRecordEx(_this,lpsInit,i,0);
    }
  }

  CData_CopyDescr(_this,dSel1);

  return O_K;
}

INT16 CGEN_PUBLIC CData_Aggregate
(
  CData* _this, CData* iSrc, CData* iMask, COMPLEX64 dParam, const char* lpOpname
)
{
  INT16 nOpcode = -1;

  CHECK_THIS_RV(NOT_EXEC);

  if ((nOpcode = dlp_aggrop_code(lpOpname))<0)
    return IERROR(_this,DATA_OPCODE,lpOpname,"aggregation",0);
  return CData_Aggregate_Int(_this,iSrc,iMask,dParam,nOpcode);
}

/*
 * Manual page in data.def
 */
INT16 CGEN_PUBLIC CData_GenIndex(data *_this, data *iSrc, data *iTab, INT32 nSrcIdx, INT32 nTabIdx)
{
  INT32      i          = 0;
  INT32      j          = 0;
  INT32      nRecsTab   = 0;
  INT32      nRecsSrc   = 0;
  INT32      nSrcComp   = 0;
  INT16      nCompType1 = 0;
  INT16      nCompType2 = 0;
  INT16      nWarn      = 0;
  char*      lpLab      = NULL;
  COMPLEX64* dBuf       = NULL;
  COMPLEX64  dValue     = CMPLX(0);
  char       lpsInit[L_INPUTLINE];
  char       bLabel;

  CHECK_THIS_RV(NOT_EXEC);

  if (CData_IsEmpty(iSrc)) return IERROR(_this,DATA_EMPTY,BASEINST(iSrc)->m_lpInstanceName,0,0);
  if (CData_IsEmpty(iTab)) return IERROR(_this,DATA_EMPTY,BASEINST(iTab)->m_lpInstanceName,0,0);
  if (nSrcIdx > -1 && nSrcIdx >= CData_GetNComps(iSrc)) return NOT_EXEC;
  if (nTabIdx <  0 || nTabIdx >= CData_GetNComps(iTab)) return NOT_EXEC;

  CData_ReadInitializer(_this,lpsInit,L_INPUTLINE,FALSE);

  nRecsTab   = CData_GetNRecs(iTab);
  nRecsSrc   = CData_GetNRecs(iSrc);
  nSrcComp   = nSrcIdx;
  nCompType1 = CData_GetCompType(iTab,nTabIdx);

  if (nSrcIdx < 0)
  {
    for (i=0; i<CData_GetNComps(iSrc); i++)
      if (dlp_is_numeric_type_code(CData_GetCompType(iSrc,i)) == dlp_is_numeric_type_code(nCompType1)) break;
    if (i == CData_GetNComps(iSrc)) return NOT_EXEC;
    else nSrcComp = i;
  }

  if ( dlp_is_numeric_type_code(CData_GetCompType(iSrc,nSrcComp)) &&
      !dlp_is_numeric_type_code(CData_GetCompType(iTab,nTabIdx))   )
  {
    return NOT_EXEC;
  }
  if ( dlp_is_symbolic_type_code(CData_GetCompType(iSrc,nSrcComp)) &&
      !dlp_is_symbolic_type_code(CData_GetCompType(iTab,nTabIdx))   )
  {
    return NOT_EXEC;
  }

  bLabel=_this->m_bLabel;

  CREATEVIRTUAL(CData,iSrc,_this);
  /*CREATEVIRTUAL(CData,iTab,_this);*/ /*TODO: Why is this commented out? */

  CData_Reset(BASEINST(_this),TRUE);
  CData_AddComp(_this,CData_GetCname(iTab,nTabIdx),T_LONG);
  CData_AllocateUninitialized(_this,CData_GetNRecs(iSrc));
  CData_CopyDescr(_this,iSrc);
  CData_Fill(_this,CMPLX(-1),CMPLX(0));

  if (dlp_is_numeric_type_code(nCompType2=CData_GetCompType(iSrc,nSrcComp)) == TRUE)
  {
    dBuf = (COMPLEX64*)dlp_malloc(nRecsTab*sizeof(COMPLEX64));
    CData_CcompFetch(iTab,dBuf,nTabIdx,nRecsTab);
    for (i=0; i<nRecsSrc; i++)
    {
      dValue = CData_Cfetch(iSrc,i,nSrcComp);
      for (j=0; j<nRecsTab; j++) if (CMPLX_EQUAL(dValue,dBuf[j])) break;
      if (j < nRecsTab) {
        CData_Dstore (_this,j,i,0);
      } else {
        nWarn++;
      }
    }
    dlp_free (dBuf);
  }
  else
  {
    nCompType1=CData_GetCompType(iTab,nTabIdx);
    if (nCompType2 < nCompType1) nCompType1 = nCompType2;
    /*CData_CorrectPhdLabel(iSrc,nSrcComp);*/
    for (i=0; i<nRecsSrc; i++)
    {
      char*   lpC   = NULL;
      INT32 nLIdx = 0;
      lpLab = (char*)CData_XAddr(iSrc,i,nSrcComp);
      if(bLabel){
        nLIdx=strlen(lpLab)-1;
        if(nLIdx && lpLab[nLIdx]==']') nLIdx--; else nLIdx=0;
        if(nLIdx && lpLab[nLIdx]>='0' && lpLab[nLIdx]<='9') nLIdx--; else nLIdx=0;
        while(nLIdx && lpLab[nLIdx]>='0' && lpLab[nLIdx]<='9') nLIdx--;
        if(nLIdx && lpLab[nLIdx]=='[') lpLab[nLIdx]='\0'; else nLIdx=0;
      }

      for (j=0; j<nRecsTab; j++)
      {
        lpC = (char*)CData_XAddr(iTab,j,nTabIdx);
        if (!dlp_strncmp(lpLab,lpC,nCompType1)) break;
      }

      if(nLIdx) lpLab[nLIdx]='[';

      if (j<nRecsTab)
        CData_Dstore (_this,j,i,0);
      else
      {
        nWarn++;
        IERROR(_this,DATA_NOTFOUND,"Label ",lpLab,0);
        if (dlp_strlen(lpsInit)>0) CData_InitializeRecordEx(_this,lpsInit,i,0);
      }
    }
  }

  DESTROYVIRTUAL(iSrc,_this);
  /*DESTROYVIRTUAL(iTab,_this);*/
  return (nWarn == nRecsSrc) ? IERROR(_this,DATA_NOTFOUND_ERR,"At least one label were","",0) : O_K;
}

/*
 * Manual page in data.def
 */
INT16 CGEN_PUBLIC CData_Tconvert(CData* _this, data* idSrc, INT16 nCType)
{
  BOOL      bMark        = FALSE;                                               /* Copy of /mark option              */
  BOOL      bForce       = FALSE;                                               /* Copy of /force option             */
  INT32     nC           = 0;                                                   /* Current component index           */
  INT32     nR           = 0;                                                   /* Current record index              */
  INT32     nXC          = 0;                                                   /* Number of components              */
  INT32     nXR          = 0;                                                   /* Number of records                 */
  INT16     nCtypOld     = T_IGNORE;                                            /* Present component type            */
  INT16     nCtypNew     = T_IGNORE;                                            /* Target component type             */
  COMPLEX64 nBuf         = CMPLX(0);                                            /* Complex buffer                    */
  char      lpsBuf[256];                                                        /* Copy buffer                       */

  /* Validation */                                                              /* --------------------------------- */
  CHECK_THIS_RV(NOT_EXEC);                                                      /* Check this pointer                */
  CHECK_DATA(_this);                                                            /* Check this (dest.) data instance  */
  if (!dlp_is_numeric_type_code(nCType) && !dlp_is_symbolic_type_code(nCType))  /* Bad target component type         */
    return IERROR(_this,DATA_NOSUPPORT,"Target component type","",0);           /*   Error                           */
  if (CData_IsEmpty(idSrc))                                                     /* Source NULL or empty              */
  {                                                                             /* >>                                */
    CData_Reset(BASEINST(_this),TRUE);                                          /*   Clear destination               */
    return O_K;                                                                 /*   Ok                              */
  }                                                                             /* <<                                */
  CHECK_DATA(idSrc)                                                             /* Check source data instance        */

  if(CData_IsHomogen(idSrc)==nCType)                                            /* Dest. consistently of desired type*/
  {                                                                             /* >>                                */
    if (_this!=idSrc) CData_Copy(BASEINST(_this),BASEINST(idSrc));              /*   Just copy                       */
    return O_K;                                                                 /*   Ok                              */
  }                                                                             /* <<                                */

  /* Create and initialize temporary destination */                             /* --------------------------------- */
  bMark  = _this->m_bMark;                                                      /* Copy /mark option                 */
  bForce = _this->m_bForce;                                                     /* Copy /force option                */
  CREATEVIRTUAL(CData,idSrc,_this);                                             /* Identical arguments support       */
  if (!_this) return IERROR(_this,ERR_NOMEM,0,0,0);                             /* Not successfull --> out of memory */
  CData_Reset(BASEINST(_this),TRUE);                                            /* Clear this instance               */
  CData_CopyDescr(_this,idSrc);                                                 /* Copy descriptions                 */
  nXC = CData_GetNComps(idSrc);                                                 /* Get number of components          */
  nXR = CData_GetNRecs(idSrc);                                                  /* Get number of records             */

  /* Create component structure and allocate records */                         /* --------------------------------- */
  for (nC=0; nC<nXC; nC++)                                                      /* Loop over components              */
  {                                                                             /* >>                                */
    nCtypOld = CData_GetCompType(idSrc,nC);                                     /*   Determine current comp. type    */
    DLPASSERT(nCtypOld>0);                                                      /*   Component does not exist        */
    if                                                                          /*   Convert if                      */
    (                                                                           /*   |                               */
      (!bMark || CData_CompIsMarked(idSrc,nC))       &&                         /*   | (unmarked mode or marked) AND */
      (bForce || dlp_is_numeric_type_code(nCtypOld))                            /*   | (forced mode or numeric)      */
    )                                                                           /*   |                               */
      CData_AddComp(_this,CData_GetCname(idSrc,nC),nCType);                     /*     Add component of new type     */
    else                                                                        /*   otherwise do not convert        */
      CData_AddComp(_this,CData_GetCname(idSrc,nC),nCtypOld);                   /*     Add component of old type     */
  }                                                                             /* <<                                */
  CData_AllocateUninitialized(_this,CData_GetNRecs(idSrc));                     /* Allocate records                  */

  /* Copy data */                                                               /* --------------------------------- */
  for (nC=0; nC<nXC; nC++)                                                      /* Loop over components              */
  {                                                                             /* >>                                */
    nCtypOld = CData_GetCompType(idSrc,nC);                                     /*   Determine current comp. type    */
    nCtypNew = CData_GetCompType(_this,nC);                                     /*   Determine new comp. type        */
    if (nCtypOld==nCtypNew)                                                     /*   Old and new type identical      */
      for (nR=0; nR<nXR; nR++)                                                  /*     Loop over records             */
        dlp_memmove(CData_XAddr(_this,nR,nC),CData_XAddr(idSrc,nR,nC),          /*       Move data                   */
          CData_GetCompSize(idSrc,nC));                                         /*       |                           */
    else if (dlp_is_numeric_type_code(nCtypOld))                                /*   Old component numeric           */
    {                                                                           /*   >>                              */
      if (dlp_is_numeric_type_code(nCtypNew))                                   /*     New component also numeric    */
        for (nR=0; nR<nXR; nR++)                                                /*       Loop over records           */
          CData_Dstore(_this,CData_Dfetch(idSrc,nR,nC),nR,nC);                  /*         Type save copying         */
      else if (dlp_is_symbolic_type_code(nCtypNew))                             /*     New component symbolic        */
        for (nR=0; nR<nXR; nR++)                                                /*       Loop over records           */
        {                                                                       /*       >>                          */
          dlp_sprintx(lpsBuf,(char*)CData_XAddr(idSrc,nR,nC),                   /*         Convert number to string  */
            nCtypOld,_this->m_bExact);                                          /*           |                       */
          CData_Sstore(_this,dlp_strtrimleft(lpsBuf),nR,nC);                    /*         Store                     */
        }                                                                       /*       <<                          */
      else DLPASSERT(FMSG("Unexpected target component type"));                 /*     Should not happen :)          */
    }                                                                           /*   <<                              */
    else if (dlp_is_symbolic_type_code(nCtypOld))                               /*   Old component symbolic          */
    {                                                                           /*   >>                              */
      if (dlp_is_numeric_type_code(nCtypNew))                                   /*     New component numeric         */
        for (nR=0; nR<nXR; nR++)                                                /*       Loop over records           */
        {                                                                       /*       >>                          */
          dlp_memset(lpsBuf,'\0',256);                                          /*         Clear copy buffer         */
          dlp_strcpy(lpsBuf,CData_Sfetch(idSrc,nR,nC));                         /*         Fetch string              */
          if (dlp_sscanc(lpsBuf,&nBuf)!=O_K) {                                  /*         Convert string to number  */
            nBuf = CMPLX(0);                                                    /*           Default value           */
          }                                                                     /*         <<                        */
          CData_Cstore(_this,nBuf,nR,nC);                                       /*         Store                     */
        }                                                                       /*       <<                          */
      else if (dlp_is_symbolic_type_code(nCtypNew))                             /*     New component also symbolic   */
        for (nR=0; nR<nXR; nR++)                                                /*       Loop over records           */
        {                                                                       /*       >>                          */
          dlp_memset(lpsBuf,'\0',256);                                          /*         Clear copy buffer         */
          dlp_strcpy(lpsBuf,CData_Sfetch(idSrc,nR,nC));                         /*         Fetch string              */
          CData_Sstore(_this,lpsBuf,nR,nC);                                     /*         Store string              */
        }                                                                       /*       <<                          */
      else DLPASSERT(FMSG("Unexpected target component type"));                 /*     Should not happen :)          */
    }                                                                           /*   <<                              */
    else DLPASSERT(FMSG("Unexpected source component type"));                   /*   Should also not happen!         */
  }                                                                             /* <<                                */

  /* Finish destination and clean up */                                         /* --------------------------------- */
  _this->m_nblock = idSrc->m_nblock;                                            /* Copy number of blocks             */
  DESTROYVIRTUAL(idSrc,_this);                                                  /* Identical argument support        */
  return O_K;                                                                   /* Auwieh:                           */
}

/*
 * Manual page at data.def
 */
INT16 CGEN_PUBLIC CData_CopyDescr(CData* _this, CData* iSrc)
{
  CHECK_THIS_RV(NOT_EXEC);
  if (!iSrc) return NOT_EXEC;
  if (_this==iSrc) return O_K;

  /* Copy mapped numeric fields */
  _this->m_lpTable->m_descr0 = iSrc->m_lpTable->m_descr0;
  _this->m_lpTable->m_descr1 = iSrc->m_lpTable->m_descr1;
  _this->m_lpTable->m_descr2 = iSrc->m_lpTable->m_descr2;
  _this->m_lpTable->m_descr3 = iSrc->m_lpTable->m_descr3;
  _this->m_lpTable->m_descr4 = iSrc->m_lpTable->m_descr4;
  _this->m_lpTable->m_fsr    = iSrc->m_lpTable->m_fsr;
  _this->m_lpTable->m_zf     = iSrc->m_lpTable->m_zf;
  _this->m_lpTable->m_ofs    = iSrc->m_lpTable->m_ofs;

  /* Copy fields */
  _this->m_nCinc             = iSrc->m_nCinc;
  _this->m_nCofs             = iSrc->m_nCofs;

  /* Copy base instance fields */
  BASEINST(_this)->m_nCheck = BASEINST(iSrc)->m_nCheck;

  /* Copy strings */
  dlp_strncpy(_this->m_lpCunit,iSrc->m_lpCunit,sizeof(_this->m_lpCunit));
  dlp_strncpy(_this->m_lpRunit,iSrc->m_lpRunit,sizeof(_this->m_lpRunit));
  dlp_free(_this->m_lpTable->m_rtext);
#ifndef __NOXALLOC
  if (dlp_size(iSrc->m_lpTable->m_rtext)>0)
  {
    _this->m_lpTable->m_rtext = (char*)dlp_malloc(dlp_size(iSrc->m_lpTable->m_rtext)*sizeof(char));
    dlp_strcpy(_this->m_lpTable->m_rtext,iSrc->m_lpTable->m_rtext);
  }
#endif
  dlp_free(_this->m_lpTable->m_vrtext);
#ifndef __NOXALLOC
  if (dlp_size(iSrc->m_lpTable->m_vrtext)>0)
  {
    _this->m_lpTable->m_vrtext = (char*)dlp_malloc(dlp_size(iSrc->m_lpTable->m_vrtext)*sizeof(char));
    dlp_strcpy(_this->m_lpTable->m_vrtext,iSrc->m_lpTable->m_vrtext);
  }
#endif
  dlp_free(_this->m_ftext);
#ifndef __NOXALLOC
  if (dlp_size(iSrc->m_ftext)>0)
  {
    _this->m_ftext = (char*)dlp_malloc(dlp_size(iSrc->m_ftext)*sizeof(char));
    dlp_strcpy(_this->m_ftext,iSrc->m_ftext);
  }
#endif

  return O_K;
}

INT16 CGEN_PUBLIC CData_CopyMark(CData* _this, CData* iSrc)
{
  CHECK_THIS_RV(NOT_EXEC);
  if (!iSrc) return NOT_EXEC;
  if (_this==iSrc) return O_K;

  CData_Unmark(_this);
  if(iSrc->m_markMap)
  {
    if
    (
      (iSrc->m_markMode==CDATA_MARK_COMPS && CData_GetNComps(iSrc)==CData_GetNComps(_this)) ||
      (iSrc->m_markMode==CDATA_MARK_RECS && CData_GetNRecs(iSrc)==CData_GetNRecs(_this)) ||
      (iSrc->m_markMode==CDATA_MARK_CELLS && CData_GetNComps(iSrc)==CData_GetNComps(_this) &&
       CData_GetNRecs(iSrc)==CData_GetNRecs(_this))
    )
    {
      _this->m_markMap=(char*)dlp_calloc(dlp_size(iSrc->m_markMap),sizeof(char));
      dlp_memmove(_this->m_markMap,iSrc->m_markMap,dlp_size(iSrc->m_markMap));
      _this->m_markMode   =iSrc->m_markMode;
      _this->m_markMapSize=iSrc->m_markMapSize;
      return O_K;
    }
  }

  return NOT_EXEC;
}

INT16 CGEN_PUBLIC CData_CopyLabels(CData* _this, CData* iSrc)
{
  INT32 i      = 0;
  INT32 j      = 0;
  INT32 k      = -1;
  INT32 l      = 0;
  INT32 m      = 0;
  INT32 M1     = 0;
  INT32 M2     = 0;
  INT32 nRecsT = 0;
  INT32 nRecsS = 0;

  CHECK_THIS_RV(NOT_EXEC);
  if (!iSrc) return IERROR(_this,ERR_NULLARG,"iSrc",0,0);
  if (_this==iSrc) return O_K;

  nRecsS = CData_GetNRecs(iSrc);
  nRecsT = CData_GetNRecs(_this);

  for (j=0; j<CData_GetNComps(iSrc); j++)
  {
    if (dlp_is_symbolic_type_code(CData_GetCompType(iSrc,j)))
    {
      for (l=k+1; l<CData_GetNComps(_this); l++)
        if (dlp_is_symbolic_type_code(CData_GetCompType(_this,l))) break;

      k=l;
      if (k >= CData_GetNComps(_this)) break;
      if(_this->m_lpTable->m_fsr == iSrc->m_lpTable->m_fsr) {
        if(nRecsS != nRecsT) {
          char* lpBuf = dlp_get_a_buffer();
          sprintf(lpBuf,"%ld!=%ld",(long)CData_GetNRecs(iSrc),(long)CData_GetNRecs(_this));
          IERROR(_this,DATA_DIMMISMATCH,BASEINST(iSrc)->m_lpInstanceName,BASEINST(_this)->m_lpInstanceName,lpBuf);
          nRecsS = MIN(nRecsS, nRecsT);
        }
        for (i=0; i<nRecsS; i++) CData_Sstore(_this,(char*)CData_XAddr(iSrc,i,j),i,k);
      } else if((_this->m_lpTable->m_fsr > 0.0) && (iSrc->m_lpTable->m_fsr > 0.0)) {
        for (i=0; i<nRecsS; i++) {
          M2 = (INT32)(iSrc->m_lpTable->m_fsr * (FLOAT64)(i+1) / _this->m_lpTable->m_fsr + 0.5);
          M2 = MIN(M2, nRecsT);
          for(m = M1; m < M2; m++) CData_Sstore(_this,(char*)CData_XAddr(iSrc,i,j),m,k);
          M1 = M2;
        }
        for(m = M1; m < nRecsT; m++) CData_Sstore(_this,(char*)CData_XAddr(iSrc,i-1,j),m,k);
      } else {
        char* lpBuf = dlp_get_a_buffer();
        if(_this->m_lpTable->m_fsr <= 0.0) {
          sprintf(lpBuf,"m_fsr=%f",(double)_this->m_lpTable->m_fsr);
          IERROR(_this,ERR_ILLEGALMEMBERVAL,"m_fsr",BASEINST(_this)->m_lpInstanceName,lpBuf);
        } else {
          sprintf(lpBuf,"m_fsr=%f",(double)iSrc->m_lpTable->m_fsr);
          IERROR(_this,ERR_ILLEGALMEMBERVAL,"m_fsr",BASEINST(iSrc)->m_lpInstanceName,lpBuf);
        }
        for (i=0; i<nRecsS; i++) {
          M2 = (INT32)((FLOAT64)(i+1) * (FLOAT64)nRecsT / (FLOAT64)nRecsS + 0.5);
          for(m = M1; m < M2; m++) CData_Sstore(_this,(char*)CData_XAddr(iSrc,i,j),m,k);
          M1 = M2;
        }
      }
    }
  }

  return O_K;
}

INT16 CGEN_PUBLIC CData_Join(CData* _this, data* x)
{
  INT16 nRetVal = 0;
  CData *lpAux  = NULL;

  CHECK_THIS_RV(NOT_EXEC);
  if (!x) return O_K;

  if(_this==x) {ICREATEEX(CData,lpAux,"~lpAux",NULL);CData_Copy(BASEINST(lpAux),BASEINST(x));x=lpAux;}

  nRetVal = CDlpTable_Join(_this->m_lpTable, x->m_lpTable);
  if(nRetVal==ERR_TRUNCATE)
  {
    IERROR(_this,DATA_TRUNCATE,"records",BASEINST(x)->m_lpInstanceName,
      BASEINST(_this)->m_lpInstanceName);
    nRetVal = O_K;
  }

  if(lpAux!=NULL) IDESTROY(lpAux);

  return nRetVal;
}

INT16 CGEN_PUBLIC CData_NJoin(CData* _this, data* x, INT32 jx, INT32 n)
{
  INT16 nRet    = 0;
  CData* lpVirt = NULL;

  CHECK_THIS_RV(NOT_EXEC);

  ICREATEEX(CData,lpVirt,"lpVirt",NULL);

  nRet = CData_Select(lpVirt,x,jx,n);
  if(nRet != O_K)
  {
    IERROR(_this,DATA_INTERNAL,"CData_Select failed during CData_NJoin",0,0);
    IDESTROY(lpVirt);
    return NOT_EXEC;
  }

  nRet = CData_Join(_this,lpVirt);
  if(nRet != O_K)
  {
    IERROR(_this,DATA_INTERNAL,"CData_Join failed during CData_NJoin",0,0);
    IDESTROY(lpVirt);
    return NOT_EXEC;
  }

  IDESTROY(lpVirt);

  return O_K;
}

INT16 CGEN_PUBLIC CData_Cat(CData* _this, data* x)
{
  INT16 nRetVal = 0;

  CHECK_THIS_RV(NOT_EXEC);
  if (!x) return O_K;

  nRetVal = CDlpTable_Cat(_this->m_lpTable, x->m_lpTable);
  if(nRetVal==ERR_TRUNCATE)
  {
    IERROR(_this,DATA_TRUNCATE,"components",BASEINST(x)->m_lpInstanceName,
      BASEINST(_this)->m_lpInstanceName);
    nRetVal = O_K;
  }

  return nRetVal;
}

/*
 * Manual page at data_man.def
 */
INT16 CGEN_PUBLIC CData_Select(CData* _this, CData* idSrc, INT32 nFirst, INT32 nCount)
{
  CHECK_THIS_RV(NOT_EXEC);

  if      ( _this->m_bRec  ) return CData_SelectRecs  (_this,idSrc,nFirst,nCount);
  else if ( _this->m_bBlock) return CData_SelectBlocks(_this,idSrc,nFirst,nCount);
  else if (!_this->m_bMark ) return CData_SelectComps (_this,idSrc,nFirst,nCount);
  else                       return CData_CopyMarked  (_this,idSrc,TRUE);
}

/*
 * Manual page at data_man.def
 */
INT16 CGEN_PUBLIC CData_Delete
(
  CData* _this,
  CData* idSrc,
  INT32 nFirst,
  INT32 nCount
)
{
  BOOL bRec   = FALSE;                                                          /* Copy of /rec-option               */
  BOOL bBlock = FALSE;                                                          /* Copy of /block-option             */
  BOOL bMark  = FALSE;                                                          /* Copy of /mark-option              */

  /* Validation */                                                              /* --------------------------------- */
  CHECK_THIS_RV(NOT_EXEC);                                                      /* Check this pointer                */
  bRec   = _this->m_bRec;                                                       /* Copy /rec-option                  */
  bBlock = _this->m_bBlock;                                                     /* Copy /block-option                */
  bMark  = _this->m_bMark;                                                      /* Copy /mark-option                 */
  if (!idSrc             ) { CData_Reset(BASEINST(_this),TRUE); return O_K; }   /* No source - nothing to be done    */
  if (idSrc!=_this       ) CData_Copy(BASEINST(_this),BASEINST(idSrc));         /* Copy source                       */
  if (nCount==0 && !bMark) return O_K;                                          /* No items to be deleted            */
  CHECK_DATA(idSrc)                                                             /* Check source instance             */
  if (CData_GetNComps(idSrc)==0) return O_K;                                    /* Source empty - nothing to be done */

  /* Work */                                                                    /* --------------------------------- */
  if      ( bRec  ) return CData_DeleteRecs  (_this,nFirst,nCount);             /* Delete records                    */
  else if ( bBlock) return CData_DeleteBlocks(_this,nFirst,nCount);             /* Delete blocks                     */
  else if (!bMark ) return CData_DeleteComps (_this,nFirst,nCount);             /* Delete components (default)       */
  else              return CData_CopyMarked  (_this,idSrc,FALSE);               /* Delete marked items               */
}

INT16 CGEN_PUBLIC CData_Mark(CData* _this, INT32 jx, INT32 n)
{
  /* Local variables */
  INT32 i = 0;

  /* Verify */
  CHECK_THIS_RV(NOT_EXEC);
  if(0>jx || 0>n) return NOT_EXEC;

  /* Set mark mode */
  if     (_this->m_bRec  ) _this->m_markMode = CDATA_MARK_RECS;
  else if(_this->m_bBlock) _this->m_markMode = CDATA_MARK_BLOCKS;
  else if(_this->m_bCell ) _this->m_markMode = CDATA_MARK_CELLS;
  else                     _this->m_markMode = CDATA_MARK_COMPS;

  /* Verify */
  if(O_K != CData_VerifyMarkMap(_this))
    IERROR(_this,DATA_INTERNAL,"CData_VerifyMarkMap failed during CData_Mark",0,
      0);
  if(_this->m_markMapSize*8 < jx+n) return NOT_EXEC;

  /* Mark elements */
  for (i=0; i<n; i++) CData_MarkElem(_this,jx+i);

  /* That's it */
  return O_K;
}

INT16 CGEN_PUBLIC CData_Dmark(CData* _this, CData* idMark)
{
  INT32 nC  = 0;
  INT32 nR  = 0;
  INT32 nXC = 0;
  INT32 nXR = 0;
  INT32 nMode = 0;                         /*1: component, 2: record 3: matrix*/

  CHECK_THIS_RV(NOT_EXEC);
  if (!idMark || CData_IsEmpty(idMark)) return O_K;

  CData_Unmark(_this);

  if (CData_GetNRecs(idMark)==1  && CData_GetNComps(idMark)>1) nMode=1;
  if (CData_GetNComps(idMark)==1 && CData_GetNRecs(idMark)>1)  nMode=2;
  if (CData_GetNComps(idMark)>1  && CData_GetNRecs(idMark)>1)  nMode=3;
  if (_this->m_bRec)  nMode=1;
  if (_this->m_bComp) nMode=2;
  if(nMode==0){
     IERROR(_this,DATA_AMBIGUOUS,"idMark only one element: define with /rec or /comp",0,0);
     nMode=1;
  }

  IRESETOPTIONS(_this);
  switch(nMode){
  case 1 :
    nXC = CData_GetNComps(idMark)<CData_GetNComps(_this)?CData_GetNComps(idMark):CData_GetNComps(_this);
    for (nC=0; nC<nXC; nC++)
      if ((INT32)CData_Dfetch(idMark,0,nC))
        CData_Mark(_this,nC,1);
    break;
  case 2 :
    nXR = CData_GetNRecs(idMark)<CData_GetNRecs(_this)?CData_GetNRecs(idMark):CData_GetNRecs(_this);
    _this->m_bRec = TRUE;
    for (nR=0; nR<nXR; nR++)
      if ((INT32)CData_Dfetch(idMark,nR,0))
        CData_Mark(_this,nR,1);
    break;
  case 3 :
    nXC = CData_GetNComps(idMark)<CData_GetNComps(_this)?CData_GetNComps(idMark):CData_GetNComps(_this);
    nXR = CData_GetNRecs (idMark)<CData_GetNRecs (_this)?CData_GetNRecs (idMark):CData_GetNRecs (_this);
    _this->m_bCell = TRUE;
    for (nC=0; nC<nXC; nC++)
      for (nR=0; nR<nXR; nR++)
        if ((INT32)CData_Dfetch(idMark,nR,nC))
          CData_Mark(_this,nR*nXC+nC,1);
    break;
  }

  IRESETOPTIONS(_this);
  return O_K;
}

INT16 CGEN_PUBLIC CData_Unmark(CData* _this)
{
  if(_this->m_markMap) dlp_free(_this->m_markMap);
  _this->m_markMap     = NULL;
  _this->m_markMapSize = 0;
  return O_K;
}

/*
 * Manual page at data_man.def
 */
INT16 CGEN_PUBLIC CData_Xfetch(CData* _this, INT32 nFirst, INT32 nCount)
{
  CData* idTmp = NULL;
  INT16  nRet;

  if (!CDlpObject_MicGet(BASEINST(_this))) return NOT_EXEC;
  if (!CDlpObject_OfKind("function",CDlpObject_MicGet(BASEINST(_this))->iCaller)) /* Would cause a memory leak! */
    return NOT_EXEC;                                                            /* --> don't do it! */

  ICREATEEX(CData,idTmp,"#TMP#-xfetch",NULL);
  if (_this->m_bRec)
    nRet = CData_SelectRecs(idTmp,_this,nFirst,nCount);
  else if (_this->m_bBlock)
    nRet = CData_SelectBlocks(idTmp,_this,nFirst,nCount);
  else
    nRet = CData_SelectComps(idTmp,_this,nFirst,nCount);
  MIC_PUT_I(BASEINST(idTmp));
  return nRet;
}
/*
 * Manual page at data_man.def
 */
BOOL CGEN_PUBLIC CData_Xstore(CData* _this,CData* idSrc, INT32 nFirst, INT32 nCount, INT32 nPos)
{
  BOOL       bCheck = TRUE;
  COMPLEX64* buffer = NULL;
  INT32      i      = 0;

  if ((_this->m_bRec==FALSE)&&(_this->m_bBlock==FALSE)) _this->m_bComp=TRUE;

  /* Check if the arguments are valid */
  if (_this->m_bRec)
  {
    if ((CData_GetNRecs(idSrc)<nFirst+nCount))
    {
      printf("Not enough records in the scource instance. ABORTING !!\n");
      bCheck=FALSE;
    }
    if ((CData_GetNRecs(_this)<nPos+nCount))
    {
      printf("Not enough records in the dest. instance. ABORTING !! \n");
      bCheck=FALSE;
    }
    if (CData_GetNComps(_this)!=CData_GetNComps(idSrc))
    {
      printf("The source instance must have the same number of components as the dest. instance. ABORTING !!\n");
      bCheck=FALSE;
    }
  }

  if (_this->m_bBlock)
  {
    if ((CData_GetNBlocks(idSrc)<nFirst+nCount))
    {
      printf("Not enough blocks in the scource instance. ABORTING !!\n");
      bCheck=FALSE;
    }
    if ((CData_GetNBlocks(_this)<nPos+nCount))
    {
      printf("Not enough blocks in the dest. instance. ABORTING !! \n");
      bCheck=FALSE;
    }
    if ((CData_GetNRecsPerBlock(idSrc)!=CData_GetNRecsPerBlock(_this)) || (CData_GetNComps(idSrc)!=CData_GetNComps(_this)))
    {
      printf("The source and destination must have the same block format. ABORTING!! \n");
      bCheck=FALSE;
    }

    if(_this->m_bComp)
    {
      if((CData_GetNComps(idSrc)<nFirst+nCount))
      {
        printf("Not enough components in the scource instance. ABORTING !!\n");
        bCheck=FALSE;
      }
      if((CData_GetNComps(_this)<nPos+nCount))
      {
        printf("Not enough components in the dest. instance. ABORTING !! \n");
        bCheck=FALSE;
      }
      if(CData_GetNRecs(_this)!=CData_GetNRecs(idSrc))
      {
        printf("The source instance must have the same number of records as the dest. instance. ABORTING!! \n");
        bCheck=FALSE;
      }
    }
  }

  /* Proceed of checks successfull */
  if(bCheck)
  {
    if(_this->m_bRec)
    {
      dlp_memmove(CData_XAddr(_this,nPos,0),CData_XAddr(idSrc,nFirst,0),
        nCount*CData_GetRecLen(idSrc));
      /* MWX 2006-02-15: Would not copy strings 'n stuff!
      buffer=(FLOAT64*)dlp_calloc(nCount*CData_GetNComps(idSrc),sizeof(FLOAT64));
      for(i=0;i<nCount;i++)
      {
        CData_DrecFetch(idSrc,buffer,nFirst+i,CData_GetNComps(idSrc),-1);
        CData_DrecStore(_this,buffer,nPos+i,CData_GetNComps(_this),-1);
      }
      dlp_free(buffer);
      */
    }

    if(_this->m_bBlock)
    {
      buffer=(COMPLEX64*)dlp_malloc(nCount*CData_GetNRecsPerBlock(idSrc)*CData_GetNComps(idSrc)*sizeof(COMPLEX64)); /* Dynamic memory is allocated */
      for(i=0;i<nCount;i++)
      {
        CData_CblockFetch(idSrc,buffer,nFirst+i,CData_GetNComps(idSrc),CData_GetNRecsPerBlock(idSrc),-1);    /* Block nFirst + i is copied to buffer */
        CData_CblockStore(_this,buffer,nPos+i,CData_GetNComps(_this),CData_GetNRecsPerBlock(_this),-1);   /* Buffer is stored in Block nPos+i */
      }
      dlp_free(buffer);
    }

    if(_this->m_bComp)
    {
      buffer=(COMPLEX64*)dlp_malloc(nCount*CData_GetNRecs(idSrc)*sizeof(COMPLEX64)); /* Dynamic memory is allocated */
      for(i=0;i<nCount;i++)
      {
        CData_CcompFetch(idSrc,buffer,nFirst+i,CData_GetNRecs(idSrc));          /* Comp. nFirst + i is copied to buffer */
        CData_CcompStore(_this,buffer,nPos+i,CData_GetNRecs(_this));          /* Buffer is stored in comp. nPos+i */
      }
      dlp_free(buffer);
    }
  }

  return bCheck;
}

BOOL CGEN_PUBLIC CData_Reshape(CData* _this, CData* iSrc, INT32 nRec, INT32 nComp)
{
  INT32  i         = 0;
  INT32  nRecs     = 0;
  INT32  nComps    = 0;
  INT32  nBufSize  = nRec*nComp;
  INT16  nTypeCode = 0;
  INT16  nAux      = 0;

  nRecs=CData_GetNRecs(iSrc);
  nComps=CData_GetNComps(iSrc);

  if((nRec*nComp)!=(nRecs*nComps))
  {
    sprintf(dlp_get_a_buffer(),"%ldx%ld!=%ldx%ld",(long)nRec,(long)nComp,(long)nRecs,(long)nComps);
    IERROR(_this,DATA_DIMMISMATCH,"source","target",dlp_get_a_buffer());
    return FALSE;
  }

  /* Check if data is homogeneous */
  nTypeCode=CData_IsHomogen(iSrc);

  if(nTypeCode<0)
  {
    nAux=CData_GetCompType(iSrc,0);
    nTypeCode=nAux;

    for(i=1;i<nComps;i++)
    {
      nAux=CData_GetCompType(iSrc,i);
      if(dlp_is_numeric_type_code(nTypeCode)!=dlp_is_numeric_type_code(nAux))
      {
        IERROR(_this,DATA_HOMOGEN,BASEINST(iSrc)->m_lpInstanceName,0,0);
        return FALSE;
      }
      if(nAux>nTypeCode) nTypeCode=nAux;
    }
  }

  CREATEVIRTUAL(data,iSrc,_this);
  CData_Reset(BASEINST(_this),TRUE);

  DLPASSERT(nRecs*nComps==nBufSize);

  if(dlp_is_numeric_type_code(nTypeCode))
  {
    COMPLEX64 *buffer;

    CData_AddNcomps(_this,T_DOUBLE,nComp);
    CData_AllocateUninitialized(_this,nRec);

    buffer=(COMPLEX64*)dlp_malloc(nBufSize*sizeof(COMPLEX64));
    if (buffer==NULL) { IERROR(_this,ERR_NOMEM,0,0,0); return FALSE; }

    for(i=0;i<nComps;i++)
      CData_CcompFetch(iSrc,&buffer[i*nRecs],i,nRecs);
    for(i=0;i<nComp;i++)
      CData_CcompStore(_this,&buffer[i*nRec],i,nRec);

    if(buffer!=NULL) dlp_free(buffer);
  }
  else
  {
    INT32  j      = 0;
    INT32  nSum   = 0;
    char *lpDest = NULL;

    CData_AddNcomps(_this,nTypeCode,nComp);
    CData_AllocateUninitialized(_this,nRec);

    lpDest = (char*)CData_XAddr(_this,0,0);
    DLPASSERT(lpDest);

    for(j=0;j<nComps&&(nSum<nRec*nComp);j++)
    {
      for(i=0;i<nRecs&&(nSum<nRec*nComp);i++)
      {
        dlp_strncpy(lpDest+(nSum*nTypeCode*sizeof(char)),(char*)CData_XAddr(iSrc,i,j),nTypeCode);
        nSum++;
      }
    }
  }

  DESTROYVIRTUAL(iSrc,_this);

  return TRUE;
}

BOOL CGEN_PUBLIC CData_Repmat(CData* _this, CData* iSrc, INT32 nRec, INT32 nComp)
{
  INT32  i      = 0;
  BOOL  bCheck = TRUE;
  data* dBuffer;

  CREATEVIRTUAL(CData,iSrc,_this);
  ICREATEEX(CData,dBuffer,"Buffer",NULL);
  /* Check if arguments are valid */
  if((nRec<1)||(nComp<1))
  {
    printf("nRec and nComp must be greater than 1 - ABORTING(repmat)\n");
    bCheck=FALSE;
  }

  if(bCheck)
  {
    CData_Reset(BASEINST(_this),TRUE);            /* dest. instance is reseted */
    for(i=0;i<nRec; i++) CData_Cat(dBuffer,iSrc);
    for(i=0;i<nComp;i++) CData_Join(_this,dBuffer);
  }

  DESTROYVIRTUAL(iSrc,_this);
  IDESTROY(dBuffer);
  return bCheck;
}

/**
 * Shift comps/records/blocks in data instance.
 *
 * @param n     shift lenght
 * @return OK if successful, NOT_EXEC otherwise
 *
 * @cgen:TODO: Implement comp and block mode.
 */
INT16 CGEN_PUBLIC CData_Shift(CData* _this, CData* iSrc, INT32 nCount)
{
  INT32   nSize  = 0;
  BYTE*  lpSrc  = NULL;
  BYTE*  lpDest = NULL;

  CHECK_THIS_RV(NOT_EXEC);
  if (!iSrc)
  {
  	CData_Reset(BASEINST(_this),TRUE);
  	return NOT_EXEC;
  }
  CHECK_DATA(iSrc)
  if (nCount==0 || CData_IsEmpty(iSrc))
  {
  	CData_Copy(BASEINST(_this),BASEINST(iSrc));
  	return O_K;
  }
  if(iSrc!=_this) CData_Copy(BASEINST(_this),BASEINST(iSrc));

  /* shift mode */
  if(_this->m_bRec)
  {
    if(nCount>0)
    {
      nSize  = CData_GetRecLen(_this)*(CData_GetNRecs(_this)-nCount);
      lpSrc  = CData_XAddr(_this,0,0);
      lpDest = CData_XAddr(_this,nCount,0);
      dlp_memmove(lpDest,lpSrc,nSize);
      memset(lpSrc,0,CData_GetRecLen(_this)*nCount);
    }
    else
    {
      nCount = -nCount;
      nSize  = CData_GetRecLen(_this)*(CData_GetNRecs(_this)-nCount);
      lpSrc  = CData_XAddr(_this,nCount,0);
      lpDest = CData_XAddr(_this,0,0);
      dlp_memmove(lpDest,lpSrc,nSize);
      memset(CData_XAddr(_this,CData_GetNRecs(_this)-nCount,0),0,CData_GetRecLen(_this)*nCount);
    }

    DLP_CHECK_MEMINTEGRITY;
  }
  else
    return IERROR(_this,DATA_INTERNAL,"Only record mode (/rec) supported.",0,0);

  return O_K;
}

/**
 * Rotate comps/records/blocks in data instance.
 *
 * @param n     shift length
 * @return OK if successful, NOT_EXEC otherwise
 *
 * @cgen:TODO: Implement comp and block mode.
 */
INT16 CGEN_PUBLIC CData_Rotate(CData* _this, CData* iSrc, INT32 nCount)
{
  INT32   nSize  = 0;
  BYTE*  lpSrc  = NULL;
  BYTE*  lpDest = NULL;
  void*  lpTmp  = NULL;

  CHECK_THIS_RV(NOT_EXEC);
  if(!iSrc || nCount==0) return NOT_EXEC;
  CHECK_DATA(iSrc)
  if(CData_IsEmpty(iSrc)) return NOT_EXEC;
  if(iSrc!=_this) CData_Copy(BASEINST(_this),BASEINST(iSrc));

  /* shift mode */
  if(_this->m_bRec)
  {
    if(nCount>0)
    {
      lpTmp  = dlp_malloc(CData_GetRecLen(_this)*nCount);
      if (!lpTmp) return IERROR(_this,ERR_NOMEM,0,0,0);
      lpSrc  = CData_XAddr(_this, CData_GetNRecs(_this)-nCount, 0);
      dlp_memmove(lpTmp, lpSrc, CData_GetRecLen(_this)*nCount);
      nSize  = CData_GetRecLen(_this)*(CData_GetNRecs(_this)-nCount);
      lpSrc  = CData_XAddr(_this, 0, 0);
      lpDest = CData_XAddr(_this, nCount, 0);
      dlp_memmove(lpDest, lpSrc, nSize);
      dlp_memmove(lpSrc, lpTmp, CData_GetRecLen(_this)*nCount);
      dlp_free(lpTmp);
    }
    else
    {
      nCount = -nCount;
      lpTmp  = dlp_malloc(CData_GetRecLen(_this)*nCount);
      if (!lpTmp) return IERROR(_this,ERR_NOMEM,0,0,0);
      lpDest = CData_XAddr(_this, 0, 0);
      dlp_memmove(lpTmp, lpDest, CData_GetRecLen(_this)*nCount);
      nSize  = CData_GetRecLen(_this)*(CData_GetNRecs(_this)-nCount);
      lpSrc  = CData_XAddr(_this, nCount, 0);
      dlp_memmove(lpDest, lpSrc, nSize);
      lpDest = CData_XAddr(_this, CData_GetNRecs(_this)-nCount, 0);
      dlp_memmove(lpDest, lpTmp, CData_GetRecLen(_this)*nCount);
      dlp_free(lpTmp);
    }
    DLP_CHECK_MEMINTEGRITY;
  }
  else
    return IERROR(_this,DATA_INTERNAL,"Only record mode (/rec) supported.",0,0);

  return O_K;
}

/**
 * Checks if all components of the data table are of the same data type.
 *
 * @param _this
 *           Pointer to data instance
 * @return The common type code of all components or 0 if the table is not
 *         homogeneous or in case of errors.
 */
INT16 CGEN_PUBLIC CData_IsHomogen(CData* _this)
{
  INT16 nTypeCode = -1;
  INT32  i         = 0;
  INT32  nComps    = 0;

  CHECK_THIS_RV(0);

  nComps = CData_GetNComps(_this);
  if(nComps<1) return 0;
  nTypeCode = CData_GetCompType(_this,0);

  for(i=1;i<nComps;i++)
  {
    if(nTypeCode != CData_GetCompType(_this,i)) return 0;
  }

  return nTypeCode;
}

/*
 * Manual page at data_man.def
 */
INT16 CGEN_PUBLIC CData_Print(CData* _this)
{
  INT32 nXR      = 0;                                                            /* Number of records                 */
  INT32 nXC      = 0;                                                            /* Number of components              */
  INT32 nRpb     = 0;                                                            /* Number of records per block       */
  BOOL bCompact = TRUE;                                                         /* Compact printing mode             */

  /* Validation */                                                              /* --------------------------------- */
  CHECK_THIS_RV(NOT_EXEC);                                                      /* Check this pointer                */
  CHECK_DATA(_this);                                                            /* Check this instance               */

  /* Initialize */                                                              /* --------------------------------- */
  nXR      = CData_GetNRecs(_this);                                             /* Get number of records             */
  nXC      = CData_GetNComps(_this);                                            /* Get number of components          */
  nRpb     = CData_GetNRecsPerBlock(_this);                                     /* Get number of records per block   */
  bCompact = !_this->m_bList;                                                   /* Compact in all modes but /list    */

  /* Print header */                                                            /* --------------------------------- */
  dlp_init_printstop();                                                         /* Init. automatic list breaking     */
  dlp_inc_printlines(1);                                                        /* The users return key              */
  if (bCompact)                                                                 /* Compact mode                      */
  {                                                                             /* >>                                */
    printf("\n data %s",BASEINST(_this)->m_lpInstanceName);                     /*   Protocol                        */
    printf("\n"); dlp_fprint_x_line(stdout,'-',dlp_maxprintcols());             /*   Protocol (delimiter)            */
    dlp_inc_printlines(2);                                                      /*   Adjust number of printed lines  */
  }                                                                             /* <<                                */
  else                                                                          /* Not compact mode                  */
  {                                                                             /* >>                                */
    printf("\n"); dlp_fprint_x_line(stdout,'-',dlp_maxprintcols());             /*   Protocol (delimiter)            */
    printf("\n   Data content of instance");                                    /*   Protocol                        */
    printf("\n   data %s",BASEINST(_this)->m_lpInstanceName);                   /*   Protocol                        */
    printf("\n"); dlp_fprint_x_line(stdout,'-',dlp_maxprintcols());             /*   Protocol (delimiter)            */
    printf("\n   Components  : %6ld (Capacity         : %ld)",(long)nXC,        /*   Protocol                        */
      (long)_this->m_lpTable->m_maxdim);                                        /*   |                               */
    printf("\n   Records     : %6ld (Capacity         : %ld)",(long)nXR,        /*   Protocol                        */
      (long)CData_GetMaxRecs(_this));                                           /*   |                               */
    printf("\n   Blocks      : %6ld (Records per block: %ld)",                  /*   Protocol                        */
      (long)CData_GetNBlocks(_this),(long)nRpb);                                /*   |                               */
    printf("\n"); dlp_fprint_x_line(stdout,'-',dlp_maxprintcols());             /*   Protocol (delimiter)            */
    printf("\n   Data contents");                                               /*   Protocol                        */
    dlp_inc_printlines(9);                                                      /*   Adjust number of printed lines  */
  }                                                                             /* <<                                */

  /* Print data content */                                                      /* --------------------------------- */
  if (CData_IsEmpty(_this))                                                     /* Instance empty                    */
  {                                                                             /* >>                                */
    printf("\n   [no data]");                                                   /*   Protcol                         */
    dlp_inc_printlines(1);                                                      /*   Adjust number of printed lines  */
  }                                                                             /* <<                                */
  else if (nXC==1 && CData_GetCompType(_this,0)==1)                             /* Single character component        */
    CData_PrintText(_this);                                                     /*   -> Text mode                    */
  else if (_this->m_bList)                                                      /* /list option set                  */
    CData_PrintList(_this);                                                     /*   -> List mode                    */
  else                                                                          /* Otherwise                         */
    CData_PrintVectors(_this);                                                  /*   -> Vector mode                  */

  /* Print footer */                                                            /* --------------------------------- */
  printf("\n"); dlp_fprint_x_line(stdout,'-',dlp_maxprintcols());               /* Print footer                      */
  printf("\n");                                                                 /* Print footer                      */
  if (!bCompact) printf("\nUse <-status> for further information.\n");          /* Print addt'l footer               */
  return O_K;                                                                   /* That's it                         */
}

/*
 * Manual page at data_man.def
 */
INT16 CGEN_PUBLIC CData_Status(CData* _this)
{
  INT32       nC         = 0;
  INT32       nXR        = 0;
  INT32       nXC        = 0;
  INT32       nRpb       = 0;
  INT64       nOffset    = 0;
  INT64       nSize      = 0;
  INT16       nTypeCode  = 0;
  const char* lpTypeName = NULL;
  char        lpBuf[512];

  CHECK_THIS_RV(NOT_EXEC);
  CHECK_DATA(_this)

  nXR  = CData_GetNRecs(_this);
  nXC  = CData_GetNComps(_this);
  nRpb = CData_GetNRecsPerBlock(_this);

  dlp_init_printstop();
  printf("\n"); dlp_fprint_x_line(stdout,'-',dlp_maxprintcols());
  printf("\n   Status of instance");
  printf("\n   data %s",BASEINST(_this)->m_lpInstanceName);
  printf("\n"); dlp_fprint_x_line(stdout,'-',dlp_maxprintcols());
  printf("\n   Components     : %6ld (Capacity         : %ld)",(long)nXC,(long)_this->m_lpTable->m_maxdim);
  printf("\n   Records        : %6ld (Capacity         : %ld)",(long)nXR,(long)CData_GetMaxRecs(_this));
  printf("\n   Blocks         : %6ld (Records per block: %ld)",(long)CData_GetNBlocks(_this),(long)nRpb);
  dlp_inc_printlines(8);
  if (dlp_strlen(_this->m_lpCunit))
  {
    printf("\n   Components");
    printf("\n   - Increment    : %6.5g %s",(double)_this->m_nCinc,_this->m_lpCunit);
    printf("\n   - Offset of 1st: %6.5g %s",(double)_this->m_nCofs,_this->m_lpCunit);
    dlp_inc_printlines(3);
  }
  printf("\n   Records");
  printf("\n   - Length       : %6ld bytes",(long)CData_GetRecLen(_this));
  dlp_inc_printlines(2);
  if (dlp_strlen(_this->m_lpRunit))
  {
    printf("\n   - Width        : %6.5g %s",(double)_this->m_lpTable->m_zf,_this->m_lpRunit);
    printf("\n   - Increment    : %6.5g %s",(double)_this->m_lpTable->m_fsr,_this->m_lpRunit);
    printf("\n   - Offset of 1st: %6.5g %s",(double)_this->m_lpTable->m_ofs,_this->m_lpRunit);
    dlp_inc_printlines(3);
  }
  printf("\n   User descriptions");
  printf("\n   - 0            : %6.5g",(double)_this->m_lpTable->m_descr0);
  printf("\n   - 1            : %6.5g",(double)_this->m_lpTable->m_descr1);
  printf("\n   - 2            : %6.5g",(double)_this->m_lpTable->m_descr2);
  printf("\n   - 3            : %6.5g",(double)_this->m_lpTable->m_descr3);
  printf("\n   - 4            : %6.5g",(double)_this->m_lpTable->m_descr4);
  printf("\n   - File text    : %s",dlp_strlen(_this->m_ftext)?_this->m_ftext:"-none-");
  printf("\n   - Realization  : %s",dlp_strlen(_this->m_lpTable->m_rtext)?_this->m_lpTable->m_rtext:"-none-");
  printf("\n   - Description  : %s",dlp_strlen(_this->m_lpTable->m_vrtext)?_this->m_lpTable->m_vrtext:"-none-");
  dlp_inc_printlines(9);

  printf("\n"); dlp_fprint_x_line(stdout,'-',dlp_maxprintcols());
  printf("\n   Component structure");
  if (CData_GetNComps(_this)==0)
    printf("\n   [no components]");
  else
  {
    printf("\n                        Name  Offset   Bytes   Type");
    dlp_inc_printlines(3);
    for(nC=0; nC<CData_GetNComps(_this); nC++)
    {
      nSize      = CDlpTable_GetCompSize(_this->m_lpTable,nC);
      nTypeCode  = CDlpTable_GetCompType(_this->m_lpTable,nC);
      nOffset    = CDlpTable_GetCompOffset(_this->m_lpTable,nC);
      lpTypeName = dlp_get_type_name(nTypeCode);
      dlp_strcpy(lpBuf,CData_GetCname(_this,nC));
      printf("\n%18ld: %8s %7ld %7ld %6ld %s",(long)nC,lpBuf,(long)nOffset,(long)nSize,(long)nTypeCode,lpTypeName?lpTypeName:"???");
      dlp_inc_printlines(1);
      if (dlp_if_printstop()) break;
    }
  }
  printf("\n"); dlp_fprint_x_line(stdout,'-',dlp_maxprintcols());
  printf("\nUse <-print> to view the data content.\n");
  return O_K;
}

/*
 * Manual page at data.def
 */
INT16 CGEN_PUBLIC CData_ArrOp(CData* _this)
{
  const char*  lpsToken = NULL;
  INT32        nCnt     = 0;
  int          nScan    = 0;
  INT32        nC       = -1;
  INT32        nR       = -1;

  /* Initialize */                                                              /* --------------------------------- */
  if (!CDlpObject_MicGet(BASEINST(_this))) return NOT_EXEC;                     /* Need invocation context           */

  /* Read tokens */
  nCnt = 0;
  while ((lpsToken = MIC_NEXTTOKEN_FORCE))
  {
    if (dlp_strcmp(lpsToken,"]")==0) break;
    if (sscanf(lpsToken,"%d",&nScan)!=1)
    {
      IERROR(_this,DATA_CNVT,lpsToken,"a number",0);
      nScan = -1;
    }
    if      (nCnt==0) { nC = nScan; }
    else if (nCnt==1) { nR = nC; nC = nScan; }
    else return IERROR(_this,DATA_HERESCAN,"]",0,0);
    nCnt++;
  }
  if (!lpsToken) return IERROR(_this,DATA_HERESCAN,"]",0,0);

  if (nR < 0)
  {
    CData* idTmp = NULL;
    ICREATE(CData,idTmp,NULL);
    CData_SelectComps(idTmp,_this,nC,1);
    MIC_PUT_I(BASEINST(idTmp));
  }
  else if (dlp_is_symbolic_type_code(CData_GetCompType(_this,nC)))
    MIC_PUT_S((char*)CData_XAddr(_this,nR,nC));
  else
    MIC_PUT_C(CData_Cfetch(_this,nR,nC));

  return O_K;
}

/*
 * Manual page at data.def
 */
INT16 CGEN_PUBLIC CData_Quantize(CData* _this, CData* dIn)
{
  CData  *lpAux1 = NULL;
  CData  *lpAux2 = NULL;

  /* Validation */
  if(dIn==NULL)          return IERROR(_this,ERR_NULLARG,0,0,0);
  if(CData_IsEmpty(dIn)) return IERROR(_this,DATA_EMPTY,BASEINST(dIn)->m_lpInstanceName,0,0);

  /* Initialization */
  CREATEVIRTUAL(CData,dIn,_this);
  ICREATEEX(CData,lpAux1,"~aux1",NULL);
  ICREATEEX(CData,lpAux2,"~aux2",NULL);
  CData_Reset(BASEINST(_this),TRUE);

  /* Calculate quantization vector */
  ISETOPTION(lpAux1,"/rec");
  ISETOPTION(lpAux2,"/rec");
  CData_Aggregate(lpAux1,dIn,NULL,CMPLX(0),"max");
  CData_Aggregate(lpAux2,dIn,NULL,CMPLX(0),"min");
  CData_ScalopD(lpAux1,lpAux1,lpAux2,"add");
  CData_Scalop(lpAux1,lpAux1,CMPLX(0.5),"mult");
  CData_Tconvert(lpAux1,lpAux1,T_SHORT);
  CData_Tconvert(lpAux1,lpAux1,T_DOUBLE);
  CData_ScalopD(_this,dIn,lpAux1,"diff");
  CData_Reset(BASEINST(lpAux2), TRUE);
  CData_Scalop(lpAux2,_this,CMPLX(0),"abs");
  ISETOPTION(lpAux2,"/rec");
  CData_Aggregate(lpAux2,lpAux2,NULL,CMPLX(0),"max");
  IRESETOPTIONS(lpAux2);
  CData_Scalop(lpAux2,lpAux2,CMPLX(-1),"pow");
  CData_Scalop(lpAux2,lpAux2,CMPLX(T_SHORT_MAX),"mult");
  CData_Tconvert(lpAux2,lpAux2,T_SHORT);
  CData_Tconvert(lpAux2,lpAux2,T_DOUBLE);
  CData_CopyDescr(lpAux1,dIn);
  CData_CopyDescr(lpAux2,dIn);

  /* Perform quantization and append quantization vector */
  CData_ScalopD(_this,_this,lpAux2,"mult");
  CData_Cat(lpAux1,lpAux2);
  CData_Cat(lpAux1,_this);
  CData_Tconvert(_this,lpAux1,T_SHORT);

  DESTROYVIRTUAL(dIn,_this);
  IDESTROY(lpAux1);
  IDESTROY(lpAux2);

  ISETFIELD_SVALUE(_this,"rtext", "compressed");
  return O_K;
}

/*
 * Manual page at data.def
 */
INT16 CGEN_PUBLIC CData_Dequantize(CData* _this, CData* dIn)
{
  CData  *lpAux1 = NULL;
  CData  *lpAux2 = NULL;

  /* Validation */
  if(dIn==NULL)          return IERROR(_this,ERR_NULLARG,0,0,0);
  if(CData_IsEmpty(dIn)) return IERROR(dIn ,DATA_EMPTY,BASEINST(dIn)->m_lpInstanceName,0,0);

  /* Initialization */
  CREATEVIRTUAL(CData,dIn,_this);
  ICREATEEX(CData,lpAux1,"~aux1",NULL);
  ICREATEEX(CData,lpAux2,"~aux2",NULL);
  CData_Reset(BASEINST(_this),TRUE);

  /* Get quantization vector from input table */
  ISETOPTION(lpAux1,"/rec");
  ISETOPTION(lpAux2,"/rec");
  CData_Select(lpAux2,dIn,0,1);
  CData_Select(lpAux1,dIn,1,1);
  IRESETOPTIONS(lpAux1);
  IRESETOPTIONS(lpAux2);
  CData_Tconvert(lpAux1,lpAux1, T_DOUBLE);
  CData_Tconvert(lpAux2,lpAux2, T_DOUBLE);
  ISETOPTION(_this,"/rec");
  CData_Delete(_this,dIn,0,2);
  IRESETOPTIONS(_this);

  /* Perform dequantization */
  CData_Tconvert(_this,_this,T_DOUBLE);
  CData_ScalopD(_this,_this,lpAux1,"div");
  CData_ScalopD(_this,_this,lpAux2,"add");

  DESTROYVIRTUAL(dIn,_this);
  IDESTROY(lpAux1);
  IDESTROY(lpAux2);

  ISETFIELD_SVALUE(_this,"rtext","uncompressed");
  return O_K;
}

/* -- Static helper functions ----------------------------------------------- */

/*
 * The aux. routine corrects the PHONDAT label by removing
 * of spaces at the end of the Label;
 *
INT16 CGEN_PUBLIC CData_CorrectPhdLabel(data *x, INT32 ix)
{
  INT32  j,k,nc,T;
  char *p;

  if (CData_IsEmpty(x) == TRUE) return NOT_EXEC;

  T  = CData_GetNRecs(x);
  nc = CData_GetCompType(x,ix);

  if (nc < 256)
  for (j=0; j<T; j++)
  {  p=(char*)CData_XAddr(x,j,ix);
    k=strlen(p);
    k=MIN(k-1,nc-1);
    while (k>=0 && p[k]==' ')
    {  p[k]=0;
      k--;
    }
  }

    return O_K;
}
*/
/* EOF */
