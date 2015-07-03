// dLabPro class CCPproc (CPproc)
// - class CPproc - cepstral analysis and synthesis
//
// AUTHOR : M. Cuevas
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

INT16 CGEN_VPROTECTED CCPproc::SynthesizeFrameImpl(FLOAT64* mcep, INT16 n_mcep, FLOAT64* exc, INT32 n_exc,
    FLOAT64 nPfaLambda, FLOAT64 nSynLambda, FLOAT64* syn) {
  return CMELproc::SynthesizeFrameImpl(mcep, n_mcep, exc, n_exc, nPfaLambda, nSynLambda, syn);
}

BOOL CGEN_VPUBLIC CCPproc::IsFeaVoiceless(FLOAT64* mcep, INT16 n_mcep) {
  FLOAT64* cep = (FLOAT64*)dlp_calloc(2, sizeof(FLOAT64));
  BOOL isVoiceless = FALSE;

  if(!cep) return IERROR(this, ERR_NOMEM, 0,0,0);

  dlm_mcep2cep(mcep, n_mcep, cep, 2, m_nPfaLambda, NULL);

  isVoiceless = cep[1] < 1.0;

  dlp_free(cep);

  return isVoiceless;
}

INT16 CGEN_VPUBLIC CCPproc::FeaEnhancement(FLOAT64* mcep, INT16 n_mcep) {
  FLOAT64 lambda = (m_nPfaLambda-m_nSynLambda)/(1.0-m_nSynLambda*m_nPfaLambda);

  dlm_mcep_enhance(mcep, mcep, n_mcep, lambda);

  return O_K;
}

// EOF
