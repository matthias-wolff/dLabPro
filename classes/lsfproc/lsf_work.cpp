// dLabPro class CLSFproc (LSFproc)
// - Class CLSFproc - analysis
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


#include "dlp_lsfproc.h"
#include "dlp_math.h"


/**
 * Analyse
 *
 * Derived instances of FBAproc should override method
 * Analyse() to add the desired functionality
 *
 * @return O_K if successfull, NOT_EXEC otherwise
 */
INT16 CGEN_PUBLIC CLSFproc::AnalyzeFrame()
{
  FLOAT64 alpha  = 0.0;

  CLPCproc::AnalyzeFrame();

  FLOAT64* lpc = (FLOAT64*)m_idRealFrame->XAddr(0,0);
  FLOAT64* lsf = (FLOAT64*)dlp_calloc(m_nCoeff, sizeof(FLOAT64));

  if(!lsf) return IERROR(this, ERR_MEM, "lsf", 0,0);

  alpha = *lpc;
  *lpc = 1.0;

  if(dlm_gmult(lpc, lpc, m_nCoeff, -1.0) != O_K) return NOT_EXEC;

  INT16 n_roots_stabilised = dlm_poly2lsf(lpc, lsf, m_nCoeff);
  switch(n_roots_stabilised) {
    case -1:
      dlp_free(lsf);
      return NOT_EXEC;
    case 0:
      break;
    default:
      IERROR(this, ERR_N_ROOTS_UNSTABLE, n_roots_stabilised, 0, 0);
  }

  *lsf = alpha;

  memcpy((FLOAT64*)m_idRealFrame->XAddr(0,0), lsf, m_nCoeff * sizeof(FLOAT64));

  dlp_free(lsf);

  return O_K;
}


void CGEN_PUBLIC CLSFproc::PrepareOutput(CData* dResult)
{
  //m_nOutDim = m_nLen/2;
  m_nOutDim = m_nCoeff;
  CFBAproc::PrepareOutput(dResult);
}
