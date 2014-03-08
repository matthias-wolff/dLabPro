// dLabPro class CLCQproc (LCQproc)
// - class LCQproc - LCQ analysis and synthesis
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
#include "dlp_lcqproc.h"
#include "dlp_math.h"

INT16 CGEN_VPROTECTED CLCQproc::SynthesizeFrameImpl(FLOAT64* lcq, INT16 n_lcq, FLOAT64* exc, INT32 n_exc, FLOAT64 nPfaLambda, FLOAT64 nSynLambda, FLOAT64* syn) {
  INT16    ret       = NOT_EXEC;
  INT16    n_cep     = n_lcq;
  FLOAT64* lcq_cop   = NULL;
  FLOAT64* cep       = NULL;
  FLOAT64  lambda    = (nPfaLambda-nSynLambda)/(1.0-nSynLambda*nPfaLambda);

  lcq_cop = (FLOAT64*)dlp_calloc(n_lcq, sizeof(FLOAT64));
  if(!lcq_cop) return IERROR(this, ERR_NOMEM, 0, 0, 0);
  dlp_memmove(lcq_cop, lcq, n_lcq*sizeof(FLOAT64));
  DeNormalize(lcq_cop, n_lcq);

  if(m_bSynLcq) {
    if(nPfaLambda == nSynLambda) {
      ret = dlm_lcq_synthesize(lcq_cop, n_lcq, exc, n_exc, m_nPadeOrder, syn, &m_lpMem);
    } else {
      ret = dlm_mlcq_synthesize(lcq_cop, n_lcq, exc, n_exc, m_nPadeOrder, lambda, syn, &m_lpMem);
    }
  } else {

    if(nPfaLambda != nSynLambda) n_cep = 4*n_lcq;

    cep = (FLOAT64*)dlp_calloc(n_cep, sizeof(FLOAT64));
    if(!cep) return IERROR(this, ERR_MEM, "cep", 0, 0);

    if(nPfaLambda == nSynLambda) {
      if(m_bSynCepLcqFilt) {
        dlm_lsf2poly_filt(lcq_cop, n_lcq, cep, n_cep, NULL);
      } else if(m_bSynCepLcq) {
        dlm_lsf2poly(lcq_cop, cep, n_lcq);
      } else {
        dlm_lsf2poly(lcq_cop, cep, n_lcq);
      }
    } else {
      if(m_bSynCepMlcqFilt) {
        dlm_mlsf2poly_filt(lcq_cop, n_lcq, cep, n_cep, lambda, NULL);
        nPfaLambda = nSynLambda = 0.0;
      } else if(m_bSynMcepMlcqFilt) {
        dlm_lsf2poly_filt(lcq_cop, n_lcq, cep, n_cep, NULL);
      } else if(m_bSynCepMlcq){
        dlm_lsf2poly(lcq_cop, cep, n_lcq);
        dlm_mlpc2lpc(cep,n_lcq,cep,n_cep,lambda);
        nPfaLambda = nSynLambda = 0.0;
      } else if(m_bSynMcepMlcq) {
        dlm_lsf2poly(lcq_cop, cep, n_lcq);
      } else {
        dlm_lsf2poly(lcq_cop, cep, n_lcq);
      }
    }

    for(INT16 i = 1; i < n_cep; i++) {
      cep[i] *= 2.0 * cep[0];
    }

    ret = CCPproc::SynthesizeFrameImpl(cep, n_lcq, exc, n_exc, nPfaLambda, nSynLambda, syn);

    dlp_free(cep);
  }

  dlp_free(lcq_cop);

  return ret;
}

INT16 CGEN_PRIVATE CLCQproc::DeNormalize(FLOAT64* lcq, INT16 n_lcq) {
  for(INT16 i = 1; i < n_lcq; i++) {
    lcq[i] = F_PI * (lcq[i] / (FLOAT64)(24*n_lcq) + (FLOAT64)i / (FLOAT64)n_lcq);
  }
  return O_K;
}

INT16 CGEN_PRIVATE CLCQproc::Lcq2Mcep(FLOAT64* lcq, INT16 n_lcq, FLOAT64* mcep) {
  FLOAT64* lcq_cop = (FLOAT64*)dlp_calloc(n_lcq, sizeof(FLOAT64));

  if(!lcq_cop) return IERROR(this, ERR_NOMEM, 0, 0, 0);

  dlp_memmove(lcq_cop, lcq, n_lcq*sizeof(FLOAT64));
  DeNormalize(lcq_cop, n_lcq);
  dlm_lsf2poly(lcq_cop, mcep, n_lcq);
  for(INT16 i = 1; i < n_lcq; i++) {
    mcep[i] *= 2.0 * mcep[0];
  }

  dlp_free(lcq_cop);

  return O_K;
}

BOOL CGEN_VPROTECTED CLCQproc::IsFeaVoiceless(FLOAT64* lcq, INT16 n_lcq) {
  FLOAT64* mcep        = (FLOAT64*)dlp_calloc(n_lcq, sizeof(FLOAT64));
  BOOL    isVoiceless = FALSE;

  if(!mcep) return IERROR(this, ERR_NOMEM, 0, 0, 0);

  Lcq2Mcep(lcq, n_lcq, mcep);
  isVoiceless = CCPproc::IsFeaVoiceless(mcep, n_lcq);

  dlp_free(mcep);

  return isVoiceless;
}

//EOF
