// dLabPro class CLCQproc (LCQproc)
// - Class CLCQproc - analysis
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


#include "dlp_lcqproc.h"
#include "dlp_math.h"


/**
 * Analyse
 *
 * Derived instances of FBAproc should override method
 * Analyse() to add the desired functionality
 *
 * @return O_K if successfull, NOT_EXEC otherwise
 */
INT16 CGEN_PUBLIC CLCQproc::AnalyzeFrame()
{
  INT32   i      = 0;
  FLOAT64 alpha  = 0.0;

  if(CCPproc::AnalyzeFrame() != O_K) return NOT_EXEC;

  FLOAT64* cep = (FLOAT64*)m_idRealFrame->XAddr(0,0);
  FLOAT64* lcq = (FLOAT64*)dlp_calloc(m_nCoeff, sizeof(FLOAT64));

  if(!lcq) return IERROR(this, ERR_MEM, "lcq", 0,0);

  alpha = *cep;
  for(i = 1; i < m_nCoeff; i++) cep[i] /= 2.0;

  if(dlm_poly2lsf(cep, lcq, m_nCoeff) < 0) {
    IERROR(this, ERR_POLY_UNSTABLE, 0, 0, 0);
    dlp_free(lcq);
    return NOT_EXEC;
  }

  *lcq = alpha;
  for(i = 1; i < m_nCoeff; i++) {
    if((lcq[i] < 0.0) || (lcq[i] > F_PI)) {
      IERROR(this, ERR_POLY_UNSTABLE, 0, 0, 0);
      dlp_free(lcq);
      return NOT_EXEC;
    }
  }

  Normalize(lcq, m_nCoeff);

  dlp_memmove((FLOAT64*)m_idRealFrame->XAddr(0,0), lcq, m_nCoeff*sizeof(FLOAT64));

  dlp_free(lcq);

  return O_K;
}

INT16 CGEN_PRIVATE CLCQproc::Normalize(FLOAT64* lcq, INT16 n_lcq) {
  for(INT16 i = 1; i < n_lcq; i++) {
    lcq[i] = (lcq[i] - F_PI * (FLOAT64)i / (FLOAT64)m_nCoeff) * (FLOAT64)(2*m_nCoeff) / F_PI * 12.0;
  }
  return O_K;
}

void CGEN_PUBLIC CLCQproc::PrepareOutput(CData* dResult)
{
  //m_nOutDim = m_nLen/2;
  m_nOutDim = m_nCoeff;
  CFBAproc::PrepareOutput(dResult);
}
