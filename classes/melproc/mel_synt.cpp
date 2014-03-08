// dLabPro class CMELproc (MELproc)
// - class MELproc - Mel-filter synthesis
//
// AUTHOR : Roland Doerschel
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
#include "dlp_melproc.h"
#include "dlp_math.h"
#include "dlp_kernel.h"

//////////////////////////////////////////////////////////////////////////////////////////
//
//   mel filter synthesis: generates synthetic speech from mel filter coefficients
//   this method overloads CFBAproc::SynthesizeUsingPM
//
//    data* idMel          mel data object                    ( type FBA_FLOAT )
//    data* idPm           pitch data object                  ( 2*T_SHORT      )
//    data* idSyn          synthesized speech data object     ( type FBA_FLOAT )
INT16 CGEN_VPROTECTED CMELproc::SynthesizeUsingPM(CData* idFea, CData *idPm, CData* idSyn) {
  if(!strcmp("MELproc", this->m_lpClassName)) {
    Mf2mcep(idFea, idFea, m_nCoeff);
  }
  return CFBAproc::SynthesizeUsingPM(idFea, idPm, idSyn);
}

/*
 * Synthesis using into control. This method overloads CFBAproc::SynthesizeUsingInto.
 * *
 * @return O_K if successfull, NOT_EXEC otherwise
 */
INT16 CGEN_VPROTECTED CMELproc::SynthesizeUsingInto(data *idFea, data *idInto, data *idSyn) {
  if(!strcmp("MELproc", this->m_lpClassName)) {
    Mf2mcep(idFea, idFea, m_nCoeff);
  }
  return CFBAproc::SynthesizeUsingInto(idFea, idInto, idSyn);
}

INT16 CGEN_PRIVATE CMELproc::SynthesizeCepstralFiltering(CData* idMel, CData* idPm, CData* idSyn) {
  CData  *idExcite    = NULL;
  CData  *idMCep      = NULL;
  FLOAT64 *excitation  = NULL;
  FLOAT64 *synthesis   = NULL;
  INT32    nSamples    = 0;
  INT16   nPer        = 0;
  INT16   nCoeff      = m_nCoeff;

  ICREATEEX(CData, idMCep, "~idMCep", NULL);

  Mf2mcep(idMel, idMCep, nCoeff);

  // Generate modulated excitation
  ICREATEEX(CData,idExcite,"~idExcite",NULL);
  DLPASSERT(O_K==ModEx(idPm,idExcite));
  DLPASSERT(!idExcite->IsEmpty());

  nSamples = idExcite->GetNRecs();

  idSyn->Reset();
  idSyn->AddComp("syn", T_DOUBLE);
  idSyn->Allocate(nSamples);

  excitation = (FLOAT64*)idExcite->XAddr(0, 0);
  synthesis = (FLOAT64*)idSyn->XAddr(0, 0);

  for(INT32 currSample = 0, currPer = 0; (currSample < nSamples) && (currPer < idMCep->GetNRecs()); currSample += nPer, currPer++) {
    nPer = (INT16)idPm->Dfetch(currPer,0);

    SynthesizeFrame((FLOAT64*)idMCep->XAddr(currPer,0), nCoeff, excitation+currSample, nPer, synthesis+currSample);
  }

  IDESTROY(idMCep);
  IDESTROY(idExcite);

  if (m_nMinLog != 0.0) for (INT32 i=0; i<idSyn->GetNRecs(); i++) *((FLOAT64*)idSyn->XAddr(0, 0)+i) *= exp(m_nMinLog);

  return O_K;
}

INT16 CGEN_VPROTECTED CMELproc::SynthesizeFrameImpl(FLOAT64* mcep, INT16 n_mcep, FLOAT64* exc, INT32 n_exc, FLOAT64 nPfaLambda, FLOAT64 nSynLambda, FLOAT64* syn) {
  FLOAT64 lambda = (nPfaLambda-nSynLambda)/(1.0-nSynLambda*nPfaLambda);

  if(m_bSynMlsadf) {
    dlm_mcep_synthesize_mlsadf(mcep, n_mcep, exc, n_exc, lambda, 5, syn, &m_lpMem);
  } else if(m_bSynCep) {
    dlm_mcep2cep(mcep, n_mcep, mcep, n_mcep, lambda, NULL);
    dlm_cep_synthesize(mcep, n_mcep, exc, n_exc, m_nPadeOrder, syn, &m_lpMem);
  } else {
    dlm_mcep_synthesize(mcep, n_mcep, exc, n_exc, lambda, m_nPadeOrder, syn, &m_lpMem);
  }

  return O_K;
}

// EOF
