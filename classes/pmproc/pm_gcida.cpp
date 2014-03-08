// dLabPro class CPMproc (PMproc)
// - Class CPMproc - GCIDA code
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


#include "dlp_pmproc.h"

INT16 CGEN_PUBLIC CPMproc::Gcida(data* dSignal, data* dPM) {
  if (!dPM || !dPM->IsEmpty()) return NOT_EXEC;

  INT32 nSamples = dSignal->GetNRecs();

  FLOAT64* samples = (FLOAT64*)dlp_calloc(nSamples, sizeof(FLOAT64));
  for(INT32 iSamples=0; iSamples<nSamples; iSamples++) samples[iSamples] = dSignal->Dfetch(iSamples,0);

  PERIOD* periods;
  INT32 nPeriods = 0;

  DEBUGMSG(-1,"\nCalculate PM from GCIDA ...",0,0,0);
  dlm_gcida(samples, nSamples, &periods, &nPeriods, m_nSrate, m_nMin, m_nMean, m_nMax);

  dPM->AddNcomps(T_SHORT, 2);
  dPM->SetCname(0, "pm");
  dPM->SetCname(1, "v/uv");
  ISETFIELD_RVALUE(dPM,"fsr", 1000.0/m_nSrate);

  dPM->AddRecs(nPeriods, 1);
  for(INT32 iPeriods = 0; iPeriods < nPeriods; iPeriods++) {
    dPM->Dstore((FLOAT64)(periods+iPeriods)->nPer, iPeriods, 0);
    dPM->Dstore((FLOAT64)(periods+iPeriods)->stimulation, iPeriods, 1);
  }

/*
  INT16 nAuxCopy = 0;

  for (INT32 i = 0; i < nPeriods; i++)
  {
    nAuxCopy = (INT16)CData_Dfetch(dPM,i,0);
    fprintf(stdout,"PM from GCIDA  %i = %i\n",i,nAuxCopy);
  }
*/

  dlp_free(samples);
  dlp_free(periods);
  return O_K;
}
