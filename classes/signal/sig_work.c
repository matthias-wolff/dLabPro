/* dLabPro class CSignal (signal)
 * - Worker functions
 *
 * AUTHOR : strecha
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

#include "dlp_cscope.h"                                                         /* Indicate C scope                  */
#include "dlp_signal.h"
#include "dlp_base.h"
#include "dlp_matrix.h"

/* ==================== */

COMPLEX64 CGEN_PRIVATE CSignal_GetMinQuant(INT16 nType, COMPLEX64 nScale) {
  if(dlp_is_integer_type_code(nType)) return dlp_scalopC(nScale,CMPLX(0),OP_INVT);
  if(dlp_is_float_type_code(nType)) {
    switch(nType) {
      case T_FLOAT: { return dlp_scalopC(CMPLX(__FLT_MIN__),nScale,OP_DIV);break; }
      case T_DOUBLE:
      case T_COMPLEX: { return dlp_scalopC(CMPLX(__DBL_MIN__),nScale,OP_DIV);break; }
    }
  }
  return dlp_scalopC(nScale,CMPLX(0),OP_INVT);
}

INT16 CGEN_PRIVATE CSignal_GetVar(CData* idSrc, const char* sName, COMPLEX64* nValue) {
  CDlpObject* iO = CDlpObject_FindInstance(BASEINST(idSrc), sName);
  if (iO && CDlpObject_IsKindOf(iO, "var")) {
    *nValue = ((CVar*)iO)->m_nNVal;
    return O_K;
  }
  return NOT_EXEC;
}

INT16 CGEN_PRIVATE CSignal_SetVar(CData* idDst, const char* sName, COMPLEX64 nValue) {
  CDlpObject* iO = CDlpObject_FindInstance(BASEINST(idDst), sName);
  if (iO && CDlpObject_IsKindOf(iO, "var")) {
    CVar_Vset(AS(CVar,iO),nValue);
  } else {
    iO = CDlpObject_Instantiate(BASEINST(idDst), "var", sName, FALSE);
    if (iO != NULL) CVar_Vset(AS(CVar,iO),nValue);
  }
  return O_K;
}

INT16 CGEN_PRIVATE CSignal_GetData(CData* idSrc, const char* sName, CData** iData) {
  CDlpObject* iO = CDlpObject_FindInstance(BASEINST(idSrc), sName);
  if (iO && CDlpObject_IsKindOf(iO, "data")) {
    *iData = (CData*)iO;
    return O_K;
  }
  return NOT_EXEC;
}

INT16 CGEN_PRIVATE CSignal_SetData(CData* idDst, const char* sName, CData* iData) {
  CDlpObject* iO = CDlpObject_FindInstance(BASEINST(idDst), sName);
  if (iO && CDlpObject_IsKindOf(iO, "data")) {
    CData_Copy(BASEINST(AS(CData,iO)),BASEINST(iData));
  } else {
    iO = CDlpObject_Instantiate(BASEINST(idDst), "data", sName, FALSE);
    if (iO != NULL) CData_Copy(BASEINST(AS(CData,iO)),BASEINST(iData));
  }
  return O_K;
}

INT16 CGEN_PRIVATE CSignal_ScaleImpl(CData* idY, CData* idX, COMPLEX64 nScale) {
  INT32 nC = 0;
  COMPLEX64 nT = CMPLX(0);
  COMPLEX64 nS = CMPLX(0);
  COMPLEX64 nQuant = CMPLX(0);

  CData_Copy(BASEINST(idY),BASEINST(idX));
  if (CData_GetNNumericComps(idY) > 0) {
    while ((nC < CData_GetNComps(idY)) && dlp_is_symbolic_type_code(CData_GetCompType(idY, nC))) nC++;
    nQuant = CMPLX((FLOAT64)CData_GetCompType(idY,nC));
    if (!CData_IsHomogen(idY) || !dlp_is_float_type_code(CData_GetCompType(idY, nC))) {
      CData_Tconvert(idY, idX, T_DOUBLE);
    }
    if(!CMPLX_EQUAL(nScale,CMPLX(1.0))) {
      CMatrix_Op(idY, idY, T_INSTANCE, &nScale, (nScale.y == 0) ? T_DOUBLE : T_COMPLEX, OP_DIV_EL);
    }
    if(NOK(CSignal_GetVar(idY,"nType",&nT)) || CMPLX_EQUAL(nT,CMPLX(0))) CSignal_SetVar(idY, "nType", nQuant);
    if(NOK(CSignal_GetVar(idY,"nScale",&nS)) || CMPLX_EQUAL(nS,CMPLX(0))) {
      CSignal_SetVar(idY, "nScale", nScale);
    } else {
      CSignal_SetVar(idY, "nScale", dlp_scalopC(nS, nScale, OP_MULT));
    }
  }
  return O_K;
}

/**
 * Set the signal scale factor
 *
 * @param idDst  Destination data instance.
 * @param nScale Scale factor
 * @return       <code>O_K</code>
 */
INT16 CGEN_PUBLIC CSignal_SetScale(CData *idDst, COMPLEX64 nScale) {
  return CSignal_SetVar(idDst,"nScale",nScale);
}

INT16 CGEN_PRIVATE CSignal_FftImpl(CData* idY, CData* idX, BOOL bInv) {
  INT32 i = 0;
  INT32 nR = 0;
  INT32 nC = 0;
  INT16 nErr = O_K;
  COMPLEX64 nScale = CMPLX(1);
  CData* idR = NULL;
  CData* idS = NULL;
  CData* idL = NULL;

  CSignal_GetVar(idX, "nScale", &nScale);
  FOP_PRECALC(idX,idY,idS,idR,idL);

  CData_Copy(BASEINST(idR), BASEINST(idS));
  if (bInv) CSignal_ScaleImpl(idR, idR, nScale);
  if(!CData_IsHomogen(idR) || !dlp_is_complex_type_code(CData_GetCompType(idR,0)))
    CData_Tconvert(idR, idR, T_COMPLEX);

  nR = CData_GetNRecs(idR);
  nC = CData_GetNComps(idR);

  for (i = 0; i < nR; i++) {
    nErr = dlm_fftC((COMPLEX64*)CData_XAddr(idR, i, 0), nC, bInv);
    if (nErr != O_K) break;
  }

  if (!bInv) CSignal_ScaleImpl(idR, idR, dlp_scalopC(nScale, CMPLX(0), OP_INVT));
  FOP_POSTCALC(idX,idY,idS,idR,idL);

  idY->m_nCinc = 1./idX->m_nCinc/nC;                                             /* Set phys. unit of comp. axis      */
  if      (dlp_strcmp(idX->m_lpCunit,"s"  )==0) dlp_strcpy(idY->m_lpCunit,"Hz" );/* Rename abscissa "s"   -> "Hz"     */
  else if (dlp_strcmp(idX->m_lpCunit,"ms" )==0) dlp_strcpy(idY->m_lpCunit,"kHz");/* Rename abscissa "ms"  -> "kHz"    */
  else if (dlp_strcmp(idX->m_lpCunit,"Hz" )==0) dlp_strcpy(idY->m_lpCunit,"s"  );/* Rename abscissa "Hz"  -> "s"      */
  else if (dlp_strcmp(idX->m_lpCunit,"kHz")==0) dlp_strcpy(idY->m_lpCunit,"ms" );/* Rename abscissa "kHz" -> "ms"     */
  else                                          dlp_strcpy(idY->m_lpCunit,""   );/* None of the above units -> clear  */
  return O_K;
}
