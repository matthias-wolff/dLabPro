// dLabPro class CLPCproc (LPCproc)
// - class LPCproc - LPC analysis and synthesis
//
// AUTHOR : Guntram Strecha
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

#include "dlp_base.h"
#include "dlp_lpcproc.h"
#include "dlp_math.h"

INT16 CGEN_VPROTECTED CLPCproc::SynthesizeFrameImpl(FLOAT64* lpcCoef, INT16 n_lpcCoeff, FLOAT64* exc, INT32 n_exc, FLOAT64 nPfaLambda, FLOAT64 nSynLambda, FLOAT64* syn) {
  INT32    i    = 0;
  FLOAT64  gain = 0.0;
  FLOAT64* lpc  = NULL;

  if(n_lpcCoeff != m_nCoeff) {
    if(m_lpMemLpc != NULL) {
      dlp_free(m_lpMemLpc);
    }
  }
  if(m_lpMemLpc == NULL) {
    m_lpMemLpc = (FLOAT64*)dlp_calloc(n_lpcCoeff - 1, sizeof(FLOAT64));        //  allocate memory
    if(!m_lpMemLpc) return ERR_MEM;
    m_nCoeff = n_lpcCoeff;
  }

  lpc = (FLOAT64*)dlp_calloc(n_lpcCoeff, sizeof(FLOAT64));
  if(!lpc) return IERROR(this, ERR_MEM, "lpc", 0, 0);

  gain = *lpcCoef;
  *lpcCoef = 1.0;
  if(nPfaLambda != nSynLambda) {
    dlm_mlpc2lpc(lpcCoef, n_lpcCoeff, lpc, n_lpcCoeff, (nPfaLambda-nSynLambda)/(1-nPfaLambda*nSynLambda));
  } else {
    dlp_memmove(lpc, lpcCoef, n_lpcCoeff*sizeof(FLOAT64));
  }

  gain = gain/ *lpc;
  *lpc = 1.0;
  dlm_filter_iir(lpc, n_lpcCoeff, exc, syn, n_exc, m_lpMemLpc, n_lpcCoeff - 1);
  for(i = 0; i < n_exc; i++) {
    syn[i] *= gain;
  }

  dlp_free(lpc);
  return O_K;
}

// EOF
