// dLabPro class CMELproc (MELproc)
// - Class CMELproc - mel-filter
//
// AUTHOR : Soeren Wittenberg, TU Dresden
// PACKAGE: dLabPro/classes
// 
// Copyright 2013 dLabPro contributors and others (see COPYRIGHT file) 
// - Chair of System Theory and Speech Technology, TU Dresden
// - Chair of Communications Engineering, BTU Cottbus
// 
// This file is part of dLabPro.
// 
// dLabPro is free software: you can redistribute it and/or modify it under the
// terms of the GNU Lesser General Public License as published by the Free
// Software Foundation, either version 3 of the License, or (at your option)
// any later version.
// 
// dLabPro is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
// details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with dLabPro. If not, see <http://www.gnu.org/licenses/>.
#include "dlp_melproc.h"

/**
 * Analyse
 *
 * Derived instances of FBAproc should override method
 * Analyse() to add the desired functionality
 *
 * @return O_K if successfull, NOT_EXEC otherwise
 */
INT16 CGEN_PUBLIC CMELproc::AnalyzeFrame()
{
  return dlm_mf_analyze(m_lpCnvc, (FLOAT64*)m_idRealFrame->XAddr(0,0), (FLOAT64*)m_idRealFrame->XAddr(0,0), m_nLen, m_nCoeff, m_nMinLog);
}

void CGEN_VPUBLIC CMELproc::PrepareOutput(CData* dResult)
{
  m_nOutDim = m_nCoeff;
  CFBAproc::PrepareOutput(dResult);
}

INT16 CGEN_VPROTECTED CMELproc::OnPfaLambdaChangedImpl() {
  return InitMF();
}

INT16 CGEN_PRIVATE CMELproc::InitMF() {
  if(m_lpCnvc == NULL) {
    m_lpCnvc = (MLP_CNVC_TYPE*)dlp_calloc(1,sizeof(MLP_CNVC_TYPE));             // Alloc. convolution core data struct
  }
  if (!m_lpCnvc) return NOT_EXEC;                                               // Check memory objects
  dlp_strcpy(m_lpCnvc->type,m_lpsMfType);
  m_lpCnvc->lambda = m_nPfaLambda;
  if (NOK(dlm_mf_init(m_lpCnvc,m_nLen,m_nCoeff,m_nMinLog))) {                   // Initialize convolution core
    dlm_mf_done(m_lpCnvc);                                                      //   Destroy convolution core
    return IERROR(this, FBA_BADARG, m_lpsMfType, "mf_type", "[MB][ST]");        //   Das war nuescht!
  }                                                                             // <<

  return O_K;
}

INT16 CGEN_PUBLIC CMELproc::Mf2mcep(CData* idMel, CData* idCep, INT16 nCoeff) {
  INT16    ret         = O_K;
  INT32    i           = 0;
  INT32    j           = 0;
  FLOAT64* real_d      = NULL;
  FLOAT64* imag_d      = NULL;
  CData*   idAux       = NULL;

  if (idMel  == NULL)                    return IERROR(this,  ERR_NULLINST, 0, 0, 0);
  if (idCep == NULL)                     return IERROR(this,  ERR_NULLINST, 0, 0, 0);
  if (idMel->IsEmpty())                  return IERROR(idMel, DATA_EMPTY, idMel->m_lpInstanceName, 0, 0);
  if (idMel->GetCompType(0) != T_DOUBLE) return IERROR(idMel, DATA_BADCOMPTYPE, 0, idMel->m_lpInstanceName, "double");

  if(idMel == idCep) {
    CREATEVIRTUAL(CData, idMel, idCep);
  }

  idCep->Reset(TRUE);
  idCep->CopyDescr(idMel);
  idCep->AddNcomps(T_DOUBLE, nCoeff);
  idCep->Allocate(idMel->GetNRecs());

  if (dlp_charin('M', m_lpsMfType)) {
    ret = NOT_EXEC;
  } else {
    real_d = (FLOAT64*)dlp_calloc(m_nCoeff, sizeof(FLOAT64));
    imag_d = (FLOAT64*)dlp_calloc(m_nCoeff, sizeof(FLOAT64));

    for (i = 0; i < idMel->GetNRecs(); i++) {
      for (j = 0; j < m_nCoeff; j++) {
        real_d[j] = (FLOAT64)idMel->Dfetch(i,j);
      }
      dlm_idct3(real_d, m_nCoeff, imag_d);
      imag_d[0] = 0.5 * imag_d[0];
      dlp_memmove(idCep->XAddr(i, 0), imag_d, nCoeff*sizeof(FLOAT64));
    }
    dlp_free(real_d);
    dlp_free(imag_d);
  }

  for(j = 0; j < idMel->GetNComps(); j++) if(dlp_is_symbolic_type_code(idMel->GetCompType(j))) break;

  ICREATEEX(CData, idAux, "~idAux", NULL);
  CData_SelectComps(idAux, idMel, j, idMel->GetNComps()-j);
  CData_Join(idCep, idAux);
  IDESTROY(idAux);

  DESTROYVIRTUAL(idMel, idCep);

  return ret;
}

INT16 CGEN_PUBLIC CMELproc::QuantizeImpl(CData* idMel, INT32 nCS, INT32 nCC, INT32 nQ, CData* idRes) {
  INT32 iRec;
  INT32 nRec;
  INT32 nComp;
  INT32 iComp;
  FLOAT64 nScale = dlm_pow2(nQ-1) / 20.79441541681;

  if(idMel == NULL || CData_GetNComps(idMel) <= 0) return NOT_EXEC;
  if((nQ != 8) && (nQ != 16) && (nQ != 32) && (nQ != 64)) return NOT_EXEC;

  nRec = idMel->GetNRecs();
  nComp = idMel->GetNComps();

  if(nCS > nComp) return NOT_EXEC;
  if(nCS < 0) nCS = 0;
  if((nCC < 0) || ((nCS+nCC) > nComp)) nCC = nComp - nCS;

  for(iComp = nCS; iComp < nCS+nCC; iComp++) {
    if(!dlp_is_float_type_code(idMel->GetCompType(iComp))) return NOT_EXEC;
  }

  CData_Copy(idRes, idMel);

  for(iRec = 0; iRec < nRec; iRec++) {
    for(iComp = nCS; iComp < nCS+nCC; iComp++) {
      CData_Dstore(idRes,round(nScale*CData_Dfetch(idRes,iRec,iComp)),iRec,iComp);
    }
  }

  CData_Mark(idRes, nCS, nCC);idRes->m_bMark = TRUE;
  CData_Tconvert(idRes, idRes, (nQ == 8) ? T_CHAR : (nQ == 16) ? T_SHORT : (nQ == 32) ? T_INT : T_LONG);
  idRes->m_bMark = FALSE;CData_Unmark(idRes);

  return O_K;
}

INT16 CGEN_PUBLIC CMELproc::DequantizeImpl(CData* idMel, INT32 nCS, INT32 nCC, INT32 nQ, CData* idRes) {
  INT32 iRec;
  INT32 nRec;
  INT32 nComp;
  INT32 iComp;
  FLOAT64 nScale = dlm_pow2(nQ-1) / 20.79441541681;

  if(idMel == NULL || CData_GetNComps(idMel) <= 0) return NOT_EXEC;
  if((nQ != 8) && (nQ != 16) && (nQ != 32) && (nQ != 64)) return NOT_EXEC;

  nRec = idMel->GetNRecs();
  nComp = idMel->GetNComps();

  if(nCS > nComp) return NOT_EXEC;
  if(nCS < 0) nCS = 0;
  if((nCC < 0) || ((nCS+nCC) > nComp)) nCC = nComp - nCS;

  for(iComp = nCS; iComp < nCS+nCC; iComp++) {
    if(!dlp_is_integer_type_code(idMel->GetCompType(iComp))) return NOT_EXEC;
  }

  CData_Copy(idRes, idMel);

  CData_Mark(idRes, nCS, nCC);idRes->m_bMark = TRUE;
  CData_Tconvert(idRes, idRes, T_DOUBLE);
  idRes->m_bMark = FALSE;CData_Unmark(idRes);

  for(iRec = 0; iRec < nRec; iRec++) {
    for(iComp = nCS; iComp < nCS+nCC; iComp++) {
      CData_Dstore(idRes,CData_Dfetch(idRes,iRec,iComp)/nScale,iRec,iComp);
    }
  }

 return O_K;
}
/* EOF */
