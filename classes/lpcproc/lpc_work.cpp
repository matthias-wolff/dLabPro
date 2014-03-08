// dLabPro class CLPCproc (LPCproc)
// - Class CLPCproc - analysis
//
// AUTHOR : Guntram Strecha, Dresden
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


#include "dlp_lpcproc.h"
#include "dlp_math.h"


/**
 * Analyse
 *
 * Derived instances of FBAproc should override method
 * Analyse() to add the desired functionality
 *
 * @return O_K if successfull, NOT_EXEC otherwise
 */
INT16 CGEN_PUBLIC CLPCproc::AnalyzeFrame()
{
  FLOAT64* lpc   = (FLOAT64*)dlp_calloc(m_nCoeff, sizeof(FLOAT64));

  if(!lpc) {
    return ERR_MEM;
  }
  if(!strcmp(m_lpsType, "BurgLPC")) {
    if(m_nPfaLambda != 0.0) {
      if(dlm_lpc_mburg((FLOAT64*)m_idRealFrame->XAddr(0,0), m_nWlen, lpc, m_nCoeff, m_nPfaLambda, exp(-m_nMinLog)) != O_K) return NOT_EXEC;
    } else {
      if(dlm_lpc_burg((FLOAT64*)m_idRealFrame->XAddr(0,0), m_nWlen, lpc, m_nCoeff, exp(-m_nMinLog)) != O_K) return NOT_EXEC;
    }
  } else if(!strcmp(m_lpsType, "LevinsonLPC")) {
    if(dlm_lpc_mlev((FLOAT64*)m_idRealFrame->XAddr(0,0), m_nWlen, lpc, m_nCoeff, m_nPfaLambda, exp(-m_nMinLog)) != O_K) return NOT_EXEC;
  } else {
    return IERROR(this, ERR_GENERIC, "type of LPC analysis must be \"BurgLPC\" or \"LevinsonLPC\"", 0, 0);
  }

  memcpy((FLOAT64*)m_idRealFrame->XAddr(0,0), lpc, m_nCoeff * sizeof(FLOAT64));

  dlp_free(lpc);

  return O_K;

}

void CGEN_PUBLIC CLPCproc::PrepareOutput(CData* dResult)
{
  //m_nOutDim = m_nLen/2;
  m_nOutDim = m_nCoeff;
  CFBAproc::PrepareOutput(dResult);
}
