// dLabPro class CCPproc (CPproc)
// - Class CCPproc - analysis
//
// AUTHOR : Matthias Eichner, Dresden
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


#include "dlp_cpproc.h"
#include "dlp_math.h"

/**
 * Analyse
 *
 * Derived instances of FBAproc should override method
 * Analyse() to add the desired functionality
 *
 * @return O_K if successfull, NOT_EXEC otherwise
 */
INT16 CGEN_PUBLIC CCPproc::AnalyzeFrame() {
  INT16 ret = O_K;

  if(!strcmp(m_lpsType, "MelFilter")) {
    CData* idCep = NULL;
    CData* idMel = NULL;
    ICREATEEX(CData, idCep, "~idCep", NULL);
    ICREATEEX(CData, idMel, "~idMel", NULL);
    if((ret = CMELproc::AnalyzeFrame()) != O_K) return ret;
    idMel->Select(m_idRealFrame, 0, m_nCoeff);
    if((ret = Mf2mcep(idMel, idCep, m_nCoeff)) != O_K) return ret;
    dlp_memmove(m_idRealFrame->XAddr(0, 0), idCep->XAddr(0, 0), m_nCoeff * sizeof(FLOAT64));
    IDESTROY(idMel);
    IDESTROY(idCep);
  } else {
    if(!strcmp(m_lpsWarptype, "time")) {
      if(!strcmp(m_lpsType, "BurgLPC")) {
        dlm_calcmcep((FLOAT64*)m_idRealFrame->XAddr(0, 0), m_nWlen, (FLOAT64*)m_idRealFrame->XAddr(0, 0), m_nCoeff, 0.0, -m_nMinLog, DLM_CALCCEP_METHOD_S_MLPC_BURG_MCEP);
      } else if(!strcmp(m_lpsType, "LevinsonLPC")) {
        dlm_calcmcep((FLOAT64*)m_idRealFrame->XAddr(0, 0), m_nWlen, (FLOAT64*)m_idRealFrame->XAddr(0, 0), m_nCoeff, 0.0, -m_nMinLog, DLM_CALCCEP_METHOD_S_MLPC_LEVI_MCEP);
      } else if(!strcmp(m_lpsType, "Uels")) {
        dlm_calcmcep((FLOAT64*)m_idRealFrame->XAddr(0, 0), m_nLen, (FLOAT64*)m_idRealFrame->XAddr(0, 0), m_nCoeff, 0.0, -m_nMinLog, DLM_CALCCEP_METHOD_S_MCEP_UELS);
      } else if(!strcmp(m_lpsType, "LogFFT")) {
        dlp_memset(m_idImagFrame->XAddr(0, 0), 0L, m_nLen * sizeof(FLOAT64));
        if((ret = dlm_fft((FLOAT64*)m_idRealFrame->XAddr(0, 0),(FLOAT64*)m_idImagFrame->XAddr(0, 0),m_nLen,FALSE)) != O_K) {
          if(ret == ERR_MDIM) IERROR(this, FBA_BADFRAMELEN, m_nLen, "FFT", 0);
          return NOT_EXEC;
        }
        LN();
        if((ret = dlm_fft((FLOAT64*)m_idRealFrame->XAddr(0, 0), (FLOAT64*)m_idImagFrame->XAddr(0, 0), m_nLen, TRUE)) != O_K) {
          if(ret == ERR_MDIM) IERROR(this, FBA_BADFRAMELEN, m_nLen, "FFT", 0);
          return NOT_EXEC;
        }
        for(INT32 i = 1; i < m_nCoeff; i++) {
          ((FLOAT64*)m_idRealFrame->XAddr(0, 0))[i] *= 2.0;
        }
      } else {
        IERROR(this,ERR_NULLINST,0,0,0);
        return NOT_EXEC;
      }
    } else {
      if(!strcmp(m_lpsType, "BurgLPC")) {
        if(!strcmp(m_lpsWarptype, "lpc")) {
          dlm_calcmcep((FLOAT64*)m_idRealFrame->XAddr(0, 0), m_nWlen, (FLOAT64*)m_idRealFrame->XAddr(0, 0),
              m_nCoeff, m_nPfaLambda, -m_nMinLog, DLM_CALCCEP_METHOD_S_LPC_BURG_MLPC_MCEP);
        } else if(!strcmp(m_lpsWarptype, "cepstrum")) {
          dlm_calcmcep((FLOAT64*)m_idRealFrame->XAddr(0, 0), m_nWlen, (FLOAT64*)m_idRealFrame->XAddr(0, 0),
              m_nCoeff, m_nPfaLambda, -m_nMinLog, DLM_CALCCEP_METHOD_S_LPC_BURG_CEP_MCEP);
        } else if(!strcmp(m_lpsWarptype, "none")) {
          dlm_calcmcep((FLOAT64*)m_idRealFrame->XAddr(0, 0), m_nWlen, (FLOAT64*)m_idRealFrame->XAddr(0, 0),
              m_nCoeff, 0.0, -m_nMinLog, DLM_CALCCEP_METHOD_S_LPC_BURG_CEP);
        } else {
          IERROR(this, CP_WARPTYPE, m_lpsWarptype, 0, 0);
          return NOT_EXEC;
        }
      } else if(!strcmp(m_lpsType, "LevinsonLPC")) {
        if(!strcmp(m_lpsWarptype, "lpc")) {
          dlm_calcmcep((FLOAT64*)m_idRealFrame->XAddr(0, 0), m_nWlen, (FLOAT64*)m_idRealFrame->XAddr(0, 0),
              m_nCoeff, m_nPfaLambda, -m_nMinLog, DLM_CALCCEP_METHOD_S_LPC_LEVI_MLPC_MCEP);
        } else if(!strcmp(m_lpsWarptype, "cepstrum")) {
          dlm_calcmcep((FLOAT64*)m_idRealFrame->XAddr(0, 0), m_nWlen, (FLOAT64*)m_idRealFrame->XAddr(0, 0), m_nWlen
              / 2, m_nPfaLambda, -m_nMinLog, DLM_CALCCEP_METHOD_S_LPC_LEVI_CEP_MCEP);
        } else if(!strcmp(m_lpsWarptype, "none")) {
          dlm_calcmcep((FLOAT64*)m_idRealFrame->XAddr(0, 0), m_nWlen, (FLOAT64*)m_idRealFrame->XAddr(0, 0),
              m_nCoeff, 0.0, -m_nMinLog, DLM_CALCCEP_METHOD_S_LPC_LEVI_CEP);
        } else {
          IERROR(this, CP_WARPTYPE, m_lpsWarptype, 0, 0);
          return NOT_EXEC;
        }
      } else if(!strcmp(m_lpsType, "Uels")) {
        if((ret = dlm_calcmcep((FLOAT64*)m_idRealFrame->XAddr(0, 0), m_nLen, (FLOAT64*)m_idRealFrame->XAddr(0, 0), m_nCoeff,
            m_nPfaLambda, exp(-m_nMinLog), DLM_CALCCEP_METHOD_S_MCEP_UELS)) != O_K) {
          return NOT_EXEC;
        }
      } else if(!strcmp(m_lpsType, "LogFFT")) {
        dlp_memset(m_idImagFrame->XAddr(0, 0), 0L, m_nLen * sizeof(FLOAT64));
        if((ret = dlm_fft((FLOAT64*)m_idRealFrame->XAddr(0, 0), (FLOAT64*)m_idImagFrame->XAddr(0, 0), m_nLen, FALSE)) != O_K) {
          if(ret == ERR_MDIM) IERROR(this, FBA_BADFRAMELEN, m_nLen, "FFT", 0);
          return NOT_EXEC;
        }
        LN();
        if(!strcmp(m_lpsWarptype, "lpc")) {
          IERROR(this, CP_WARPTYPE, m_lpsWarptype, 0, 0);
          return NOT_EXEC;
        } else if(!strcmp(m_lpsWarptype, "cepstrum")) {
          dlp_memset(m_idImagFrame->XAddr(0, 0), 0L, m_nLen * sizeof(FLOAT64));
          if((ret = dlm_fft((FLOAT64*)m_idRealFrame->XAddr(0, 0), (FLOAT64*)m_idImagFrame->XAddr(0, 0), m_nLen, TRUE)) != O_K) {
            if(ret == ERR_MDIM) IERROR(this, FBA_BADFRAMELEN, m_nLen, "FFT", 0);
            return NOT_EXEC;
          }
          dlm_cep2mcep((FLOAT64*)m_idRealFrame->XAddr(0, 0), m_nLen, (FLOAT64*)m_idRealFrame->XAddr(0, 0),
              m_nCoeff, m_nPfaLambda, NULL);
        } else if(!strcmp(m_lpsWarptype, "spectrum")) {
          WARP();
          dlp_memset(m_idImagFrame->XAddr(0, 0), 0L, m_nLen * sizeof(FLOAT64));
          if((ret = dlm_fft((FLOAT64*)m_idRealFrame->XAddr(0, 0), (FLOAT64*)m_idImagFrame->XAddr(0, 0), m_nLen, TRUE)) != O_K) {
            if(ret == ERR_MDIM) IERROR(this, FBA_BADFRAMELEN, m_nLen, "FFT", 0);
            return NOT_EXEC;
          }
        } else if(!strcmp(m_lpsWarptype, "none")) {
          dlp_memset(m_idImagFrame->XAddr(0, 0), 0L, m_nLen * sizeof(FLOAT64));
          if((ret = dlm_fft((FLOAT64*)m_idRealFrame->XAddr(0, 0), (FLOAT64*)m_idImagFrame->XAddr(0, 0), m_nLen, TRUE)) != O_K) {
            if(ret == ERR_MDIM) IERROR(this, FBA_BADFRAMELEN, m_nLen, "FFT", 0);
            return NOT_EXEC;
          }
        } else {
          IERROR(this,ERR_NULLINST,0,0,0);
        }
        for(INT32 i = 1; i < m_nCoeff; i++) {
          ((FLOAT64*)m_idRealFrame->XAddr(0, 0))[i] *= 2.0;
        }
      } else if(strcmp(m_lpsWarptype, "none")) {
        IERROR(this,ERR_NULLINST,0,0,0);
      }
    }
  }
  return O_K;
}

INT16 CGEN_VPROTECTED CCPproc::RootsTrack(CData* idCep, CData* idRoots, CData* idVUV) {
  return CFBAproc::RootsTrackImpl(idCep, idRoots, idVUV);
}

void CGEN_PUBLIC CCPproc::PrepareOutput(CData* dResult) {
  m_nOutDim = m_nCoeff;
  CFBAproc::PrepareOutput(dResult);
}

INT16 CGEN_VPROTECTED CCPproc::OnPfaLambdaChangedImpl() {
  if(!dlp_strcmp(m_lpsType, "MelFilter")) {
    return CMELproc::OnPfaLambdaChangedImpl();
  }
  return O_K;
}

INT16 CGEN_PUBLIC CCPproc::QuantizeImpl(CData* idCep, INT32 nCS, INT32 nCC, INT32 nQ, CData* idRes) {
  INT32 iRec;
  INT32 nRec;
  INT32 nComp;
  INT32 iComp;
  INT16 ret = O_K;

  if(idCep == NULL || CData_GetNComps(idCep) <= 0) return NOT_EXEC;
  if((nQ != 8) && (nQ != 16) && (nQ != 32) && (nQ != 64)) return NOT_EXEC;

  nRec = idCep->GetNRecs();
  nComp = idCep->GetNComps();

  if(nCS > nComp) return NOT_EXEC;
  if(nCS < 0) nCS = 0;
  if((nCC < 0) || ((nCS+nCC) > nComp)) nCC = nComp - nCS;

  for(iComp = nCS; iComp < nCS+nCC; iComp++) {
    if(!dlp_is_float_type_code(idCep->GetCompType(iComp))) return NOT_EXEC;
  }

  CData_Copy(idRes, idCep);

  for(iRec = 0; iRec < nRec; iRec++) {
    ret = dlm_cep_quantize((FLOAT64*)idRes->XAddr(iRec,nCS),nCC,nQ);
    if(NOK(ret)) return ret;
  }
  CData_Mark(idRes, nCS, nCC);idRes->m_bMark = TRUE;
  CData_Tconvert(idRes, idRes, (nQ == 8) ? T_CHAR : (nQ == 16) ? T_SHORT : (nQ == 32) ? T_INT : T_LONG);
  idRes->m_bMark = FALSE;CData_Unmark(idRes);

  return O_K;
}

INT16 CGEN_PUBLIC CCPproc::DequantizeImpl(CData* idCep, INT32 nCS, INT32 nCC, INT32 nQ, CData* idRes) {
  INT32 iRec;
  INT32 nRec;
  INT32 nComp;
  INT32 iComp;

  if(idCep == NULL || CData_GetNComps(idCep) <= 0) return NOT_EXEC;
  if((nQ != 8) && (nQ != 16) && (nQ != 32) && (nQ != 64)) return NOT_EXEC;

  nRec = idCep->GetNRecs();
  nComp = idCep->GetNComps();

  if(nCS > nComp) return NOT_EXEC;
  if(nCS < 0) nCS = 0;
  if((nCC < 0) || ((nCS+nCC) > nComp)) nCC = nComp - nCS;

  for(iComp = nCS; iComp < nCS+nCC; iComp++) {
    if(!dlp_is_integer_type_code(idCep->GetCompType(iComp))) return NOT_EXEC;
  }

  CData_Copy(idRes, idCep);

  CData_Mark(idRes, nCS, nCC);idRes->m_bMark = TRUE;
  CData_Tconvert(idRes, idRes, T_DOUBLE);
  idRes->m_bMark = FALSE;CData_Unmark(idRes);

  for(iRec = 0; iRec < nRec; iRec++) {
    dlm_cep_dequantize((FLOAT64*)idRes->XAddr(iRec,nCS),nCC,nQ);
  }

  return O_K;
}

// EOF ////////////////////////////////////////////////////////////////////////
