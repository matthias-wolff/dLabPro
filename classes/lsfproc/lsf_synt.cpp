// dLabPro class CLSFproc (LSFproc)
// - class LSFproc - LSF analysis and synthesis
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
#include "dlp_lsfproc.h"
#include "dlp_math.h"

INT16 CGEN_VPROTECTED CLSFproc::SynthesizeFrameImpl(FLOAT64* lsf, INT16 n_lsf, FLOAT64* exc, INT32 n_exc, FLOAT64 nPfaLambda, FLOAT64 nSynLambda, FLOAT64* syn) {
  INT16   ret   = O_K;
  INT16   n_lpc = n_lsf;
  FLOAT64* lpc   = NULL;

  if(m_bSynLsf) {
    if(nPfaLambda == nSynLambda) {
      return dlm_lsf_synthesize(lsf, n_lsf, exc, n_exc, syn, &m_lpMemLsf);
    } else {
      return dlm_mlsf_synthesize(lsf, n_lsf, exc, n_exc, (nPfaLambda-nSynLambda)/(1.0-nSynLambda*nPfaLambda), syn, &m_lpMemLsf);
    }
  }

  if(nPfaLambda != nSynLambda) n_lpc = 4*n_lsf;

  lpc = (FLOAT64*)dlp_calloc(n_lpc, sizeof(FLOAT64));
  if(!lpc) return IERROR(this, ERR_MEM, "lpc", 0, 0);

  if(nPfaLambda == nSynLambda) {
    if(m_bSynLpcFilt) {
      if(dlm_lsf2poly_filt(lsf, n_lsf, lpc, n_lpc, &m_lpMemLsf) != O_K) { dlp_free(lpc); return NOT_EXEC; }
    } else {
      if(dlm_lsf2poly(lsf, lpc, n_lsf) != O_K) { dlp_free(lpc); return NOT_EXEC; }
    }
  } else {
    if(m_bSynLpcFilt) {
      if(dlm_mlsf2poly_filt(lsf, n_lsf, lpc, n_lpc, (nPfaLambda-nSynLambda)/(1.0-nSynLambda*nPfaLambda), &m_lpMemLsf) != O_K) { dlp_free(lpc); return NOT_EXEC; }
    } else {
      if(dlm_lsf2poly(lsf, lpc, n_lsf) != O_K) { dlp_free(lpc); return NOT_EXEC; }
      if(dlm_mlpc2lpc(lpc,n_lsf,lpc,n_lpc,(nPfaLambda-nSynLambda)/(1.0-nSynLambda*nPfaLambda)) != O_K) { dlp_free(lpc); return NOT_EXEC; }
    }
    nPfaLambda = nSynLambda = 0.0;
  }

  if(dlm_gmult(lpc, lpc, n_lpc, -1.0) != O_K) return NOT_EXEC;

  ret = CLPCproc::SynthesizeFrameImpl(lpc, n_lpc, exc, n_exc, nPfaLambda, nSynLambda, syn);

  dlp_free(lpc);
  return ret;
}
