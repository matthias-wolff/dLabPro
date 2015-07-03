// dLabPro class CPMproc (PMproc)
// - Class CPMproc - EpochDetect code
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

#define N_HIST 100
#define ROUND32(X) (((X) >= 0) ? (INT32)((X)+0.5) : (INT32)((X)-0.5))
#define ROUND16(X) (((X) >= 0) ? (INT16)((X)+0.5) : (INT16)((X)-0.5))

INT16 CGEN_PRIVATE CPMproc::Epochdetect(data* dSignal, data* dPM) {

  FLOAT64*  samples          = NULL;
  FLOAT64*  samples_filt_r   = NULL;
  FLOAT64*  samples_filt_i   = NULL;
  FLOAT64*  samples_filt_p   = NULL;
  FLOAT64*  samples_filt_e   = NULL;
  FLOAT64*  freq             = NULL;
  FLOAT64   filt_a[1]        = { 1.0 };
  FLOAT64   filt_high_a[3][3]   = { { +1.0000000000000, +1.9861162115409, -0.9862119291608 }, { +1.0000000000000, +1.9722337291953, -0.9726139693131 }, { +1.0000000000000, +1.9444776577671, -0.9459779362323 } };
  FLOAT64   filt_high_b[3][3]   = { { +0.9930820351754, -1.9861640703508, +0.9930820351754 }, { +0.9862119246271, -1.9724238492542, +0.9862119246271 }, { +0.9726138984998, -1.9452277969997, +0.9726138984998 } };
  FLOAT64   energy_voiceless = 0.0004;
  INT16*   samples_filt_v   = NULL;
  INT16    stimulation      = 1;
  INT16    t0_cur           = m_nSrate / m_nMean;
  INT16    t0_old           = m_nSrate / m_nMean;
  INT16    t0_next          = m_nSrate / m_nMean;
  INT16    t0_min           = m_nSrate / m_nMax;
  INT16    t0_max           = m_nSrate / m_nMin;
  INT16    t0_mean          = m_nSrate / m_nMean;
  INT32     nSamples         = 0;
  INT32     iSamples1        = 0;
  INT32     iSamples2        = 0;
  INT32     iSamples3        = 0;
  INT32     iSamples4        = 0;
  INT32     iSamples5        = 0;
  INT32     nPeriods         = 0;
  INT32     iPeriods1        = 0;
  INT32     iPeriods2        = 0;
  INT32     n_filt_a         = 1;
  INT32     n_iter           = 2;
  INT32     iter             = 0;
  PERIOD*   periods          = NULL;

  if (!dPM || !dPM->IsEmpty()) return NOT_EXEC;

  dPM->AddNcomps(T_SHORT, 2);
  dPM->SetCname(0, "pm");
  dPM->SetCname(1, "v/uv");
  ISETFIELD_RVALUE(dPM,"fsr", 1000.0/m_nSrate);

  if(!m_lpFiltBR || !m_lpFiltBI) {
    ChfaAllocTables();
  }

  DEBUGMSG(-1,"\nCalculate PM from Epoch_Detect ...",0,0,0);

  nSamples       = dSignal->GetNRecs();

  samples = (FLOAT64*)dlp_calloc(nSamples, sizeof(FLOAT64));
  memcpy(samples, dSignal->XAddr(0,0), nSamples * sizeof(FLOAT64));

  samples_filt_r = (FLOAT64*)dlp_calloc(2 * nSamples, sizeof(FLOAT64));
  samples_filt_i = samples_filt_r + nSamples;
  samples_filt_p = (FLOAT64*)dlp_calloc(nSamples, sizeof(FLOAT64));
  samples_filt_e = (FLOAT64*)dlp_calloc(nSamples, sizeof(FLOAT64));
  freq           = (FLOAT64*)dlp_calloc(nSamples, sizeof(FLOAT64));
  samples_filt_v = (INT16*)dlp_calloc(nSamples, sizeof(INT16));

  switch(m_nSrate/8000) {
    case 4:
      ChfaFilter(filt_high_b[0], 3, filt_high_a[0], 3, samples, nSamples, samples_filt_r, 0);
    break;
    case 3:
    case 2:
      ChfaFilter(filt_high_b[1], 3, filt_high_a[1], 3, samples, nSamples, samples_filt_r, 0);
    break;
    case 1:
      ChfaFilter(filt_high_b[2], 3, filt_high_a[2], 3, samples, nSamples, samples_filt_r, 0);
    break;
    default:
      dlp_memmove(samples_filt_r, samples, nSamples * sizeof(FLOAT64));
  }

  dlp_memmove(samples, samples_filt_r, nSamples * sizeof(FLOAT64));
  dlp_memset(samples_filt_r, 0L, nSamples * sizeof(FLOAT64));

  for(iSamples1 = 0; iSamples1 < nSamples; iSamples1++) {
    *(freq + iSamples1) = t0_mean;
  }

  do {
    dlp_memset(samples_filt_r, 0L, 2 * nSamples * sizeof(FLOAT64));

    for(iSamples1 = 0; iSamples1 < nSamples; iSamples1++) {
      t0_cur = MIN(t0_max - 1, MAX(t0_min, ROUND32(*(freq + iSamples1))));
      ChfaFilter(*(m_lpFiltBR + t0_cur - t0_min), t0_cur, filt_a, n_filt_a, samples + iSamples1, 1, samples_filt_r + iSamples1, iSamples1);
      ChfaFilter(*(m_lpFiltBI + t0_cur - t0_min), t0_cur, filt_a, n_filt_a, samples + iSamples1, 1, samples_filt_i + iSamples1, iSamples1);
      *(samples_filt_r + iSamples1) = *(samples_filt_r + iSamples1) / (FLOAT64)(MIN(iSamples1 + 1, t0_cur));
      *(samples_filt_i + iSamples1) = *(samples_filt_i + iSamples1) / (FLOAT64)(MIN(iSamples1 + 1, t0_cur));
      *(samples_filt_p + iSamples1) = atan2(*(samples_filt_i + iSamples1), *(samples_filt_r + iSamples1));
      *(samples_filt_e + iSamples1) = sqrt(*(samples_filt_r + iSamples1) * *(samples_filt_r + iSamples1) + *(samples_filt_i + iSamples1) * *(samples_filt_i + iSamples1));
    }

    energy_voiceless = ChfaGetEnergyVoiceless(samples_filt_e, nSamples);

    dlp_memset(freq, 0L, nSamples * sizeof(FLOAT64));

    iSamples1 = 0;
    iSamples2 = 1;
    while(iSamples2 < nSamples) {
      if(((iSamples1 == 0) || ((iSamples2 - iSamples1) >= t0_min)) && (fabs(*(samples_filt_p + iSamples2) - *(samples_filt_p + iSamples2 - 1)) > (1.8 * F_PI))) {
        if((iSamples2 - iSamples1) > t0_max) {
          *(freq + iSamples1) = t0_max;
          iSamples1 += t0_max;
        } else {
          *(freq + iSamples1) = iSamples2 - iSamples1;
          iSamples1 = iSamples2;
        }
      }
      iSamples2++;
    }
    if(iSamples1 < iSamples2) {
      *(freq + iSamples1) = nSamples - iSamples1;
    }

    iSamples1 = 0;
    while((iSamples1 < nSamples)) {
      iSamples2 = iSamples1 + ROUND32(*(freq + iSamples1));
      iSamples4 = MIN(nSamples-1, iSamples2);
      iSamples5 = iSamples4 + ROUND32(*(freq + iSamples4));
      for(iSamples3 = iSamples1 + 1; iSamples3 < iSamples2; iSamples3++) {
        while((iSamples4 < (iSamples5 - 1)) && ((fabs(*(samples_filt_p + iSamples4) - *(samples_filt_p + iSamples3)) > fabs(*(samples_filt_p + iSamples4 + 1) - *(samples_filt_p + iSamples3))) || (*(samples_filt_p + iSamples4) < *(samples_filt_p + iSamples4 + 1)))) {
          iSamples4++;
        }
        *(freq + iSamples3) = MIN(t0_max, MAX(t0_min, iSamples4 - iSamples3));
      }

      iSamples1 = iSamples2;
    }

    dlp_memset(samples_filt_v, 0L, nSamples * sizeof(INT16));
    for(iSamples3 = 0; iSamples3 < nSamples; iSamples3++) {
      if(*(samples_filt_e + iSamples3) > energy_voiceless) {
        *(samples_filt_v + iSamples3) = 1;
      }
    }
    iSamples3 = 0;
    do {
      iSamples5 = iSamples3;
      do {
        iSamples4 = iSamples5;
        while((iSamples4 < nSamples) && (*(samples_filt_v + iSamples4) == *(samples_filt_v + iSamples3))) iSamples4++;
        iSamples5 = iSamples4;
        while((iSamples5 < nSamples) && (*(samples_filt_v + iSamples5) != *(samples_filt_v + iSamples3))) iSamples5++;
      } while((iSamples5 < nSamples) && ((iSamples5 - iSamples4) < t0_max));


      if(*(samples_filt_v + iSamples3) == 0) {
        while((iSamples4 < nSamples) && (fabs(*(samples_filt_p + iSamples4) - *(samples_filt_p + iSamples4 - 1)) < (1.8 * F_PI))) {
          *(samples_filt_v + iSamples4) = 0;
          iSamples4++;
        }
        if(iSamples4 == nSamples) {
          t0_next = t0_mean;
        } else {
          t0_next = ROUND16(*(freq + iSamples4));
        }
        for(iSamples5 = iSamples3; iSamples5 < iSamples4; iSamples5++) {
          *(samples_filt_v + iSamples5) = 0;
          *(freq + iSamples5) = (FLOAT64)(t0_old) + (FLOAT64)(t0_next - t0_old) * (FLOAT64)(iSamples5 - iSamples3) / (FLOAT64)(iSamples4 - iSamples3);
        }
      } else {
        while((iSamples4 > 0) && (fabs(*(samples_filt_p + iSamples4) - *(samples_filt_p + iSamples4 - 1)) < (1.8 * F_PI))) {
          *(samples_filt_v + iSamples4) = 0;
          iSamples4--;
        }
        *(samples_filt_v + iSamples4) = 0;
        for(iSamples5 = iSamples3; iSamples5 < iSamples4; iSamples5++) {
          *(samples_filt_v + iSamples5) = 1;
        }
        if(iSamples4 == 0) {
          t0_old = t0_mean;
        } else {
          t0_old = ROUND16(*(freq + iSamples4 - 1));
        }
      }
      iSamples3 = iSamples4;
    } while(iSamples3 < nSamples);

    iter++;
  } while(iter < n_iter);

  nPeriods = 0;
  iSamples1 = 0;
  while(iSamples1 < nSamples) {
    if(*(samples_filt_v + iSamples1) == 0) {
      stimulation = 0;
      iSamples2 = iSamples1 + 1;
      while((iSamples2 < nSamples) && (*(samples_filt_v + iSamples2) == 0)) iSamples2++;
      iSamples3 = 0;
      iPeriods2 = 0;
      while(iSamples3 < (iSamples2 - iSamples1)) {
        t0_cur = (INT16)(*(freq + iSamples1) + (*(freq + iSamples2 - 1) - *(freq + iSamples1)) * (FLOAT64)(iSamples3) / (FLOAT64)(iSamples2 - iSamples1));
        iSamples3 += t0_cur;
        periods = (PERIOD*)realloc(periods, (nPeriods + iPeriods2 + 1) * sizeof(PERIOD));
        (periods + nPeriods + iPeriods2)->nPer = t0_cur;
        (periods + nPeriods + iPeriods2)->stimulation = stimulation;
        iPeriods2++;
      }
      for(iPeriods1 = 0; iPeriods1 < iPeriods2; iPeriods1++) {
        iSamples4 = ROUND32(((FLOAT64)(iSamples2 - iSamples1 - iSamples3) * ((FLOAT64)(iPeriods1) - 0.5) / (FLOAT64)(iPeriods2)));
        iSamples5 = ROUND32(((FLOAT64)(iSamples2 - iSamples1 - iSamples3) * ((FLOAT64)(iPeriods1) + 0.5) / (FLOAT64)(iPeriods2)));
        (periods + nPeriods + iPeriods1)->nPer += iSamples5 - iSamples4;
      }
      nPeriods += iPeriods2;
      iSamples1 = iSamples2;
    } else {
      stimulation = 1;
      do {
        t0_cur = ROUND16(*(freq + iSamples1));
        periods = (PERIOD*)realloc(periods, (nPeriods + 1) * sizeof(PERIOD));
        (periods + nPeriods)->nPer = t0_cur;
        (periods + nPeriods)->stimulation = stimulation;
        nPeriods++;
        iSamples1 += t0_cur;
      } while(*(samples_filt_v + iSamples1) == 1);
    }
  }

  dPM->AddRecs(nPeriods, 1);
  for(iPeriods1 = 0; iPeriods1 < nPeriods; iPeriods1++) {
    dPM->Dstore((FLOAT64)(periods+iPeriods1)->nPer, iPeriods1, 0);
    dPM->Dstore((FLOAT64)(periods+iPeriods1)->stimulation, iPeriods1, 1);
  }

  free(periods);
  dlp_free(samples);
  dlp_free(samples_filt_r);
  dlp_free(samples_filt_p);
  dlp_free(samples_filt_e);
  dlp_free(freq);
  dlp_free(samples_filt_v);

  return O_K;
}

INT16 CGEN_PRIVATE CPMproc::ChfaFilter(FLOAT64* b, INT32 nb, FLOAT64* a, INT32 na, FLOAT64* input, INT32 nInput, FLOAT64* output, INT32 have_mem) {
  INT32  iInput    = 0;
  INT32  ia        = 0;
  INT32  ib        = 0;
  INT32  na1       = 0;
  INT32  nb1       = 0;
  FLOAT64*  p1_input  = NULL;
  FLOAT64*  p2_input  = NULL;
  FLOAT64*  p1_output = NULL;
  FLOAT64*  p2_output = NULL;
  FLOAT64*  p_a       = NULL;
  FLOAT64*  p_b       = NULL;

  a++;
  na--;

  p_b = b;
  p1_input = input;
  p1_output = output;
  for(iInput = 0; iInput < nInput; iInput++, p1_output++, p1_input++, p_b = b) {
    *p1_output = 0;
    p2_input   = p1_input;
    nb1 = MIN(nb, iInput + have_mem + 1);
    for(ib = 0; ib < nb1; ib++, p2_input--, p_b++) {
      *p1_output += *p2_input * *p_b;
    }
  }

  p_a = a;
  p1_output = output;
  for(iInput = 0; iInput < nInput; iInput++, p1_output++, p_a = a) {
    p2_output = p1_output - 1;
    na1 = MIN(na, iInput + have_mem);
    for(ia = 0; ia < na1; ia++, p2_output--, p_a++) {
      *p1_output += *p2_output * *p_a;
    }
  }
  return O_K;
}

FLOAT64 CGEN_PRIVATE CPMproc::ChfaGetEnergyVoiceless(FLOAT64* samples_filt_e, INT32 nSamples) {
  INT32    iSamples1 = 0;
  INT32    min_index = 0;
  FLOAT64  hist_e[N_HIST];
  FLOAT64  max       = 0.0;
  FLOAT64  min       = 0.0;
  FLOAT64  min_e     = 2.0;
  FLOAT64  max_e     = 0.0;

  for(iSamples1 = 0; iSamples1 < nSamples; iSamples1++) {
    if(*(samples_filt_e + iSamples1) < min_e) {
      min_e = *(samples_filt_e + iSamples1);
    }
    if(*(samples_filt_e + iSamples1) > max_e) {
      max_e = *(samples_filt_e + iSamples1);
    }
  }
  memset(hist_e, 0L, N_HIST * sizeof(FLOAT64));
  for(iSamples1 = 0; iSamples1 < nSamples; iSamples1++) {
    (*(hist_e + (INT32)((*(samples_filt_e + iSamples1) - min_e) / max_e * (FLOAT64)(N_HIST))))++;
  }
  max = *(hist_e + 0);
  for(iSamples1 = 1; iSamples1 < N_HIST; iSamples1++) {
    max = MAX(max, *(hist_e + iSamples1));
  }
  min = 1.0;
  for(iSamples1 = 0; iSamples1 < N_HIST; iSamples1++) {
    *(hist_e + iSamples1) = m_nLevel * *(hist_e + iSamples1) / max;
    *(hist_e + iSamples1) = sqrt(*(hist_e + iSamples1) * *(hist_e + iSamples1) + (FLOAT64)(iSamples1) / (FLOAT64)(N_HIST) * (FLOAT64)(iSamples1) / (FLOAT64)(N_HIST));
    if(min > *(hist_e + iSamples1)) {
      min = *(hist_e + iSamples1);
      min_index = iSamples1;
    }
  }
  return((FLOAT64)(min_index) / (FLOAT64)(N_HIST) * max_e + min_e);
}

INT16 CGEN_PRIVATE CPMproc::ChfaAllocTables() {
  INT32  i;
  INT32  j;
  INT16 freq_min = m_nSrate / m_nMax;
  INT16 freq_max = m_nSrate / m_nMin;

  m_lpFiltBR = (FLOAT64**)dlp_calloc(freq_max - freq_min, sizeof(FLOAT64*));
  m_lpFiltBI = (FLOAT64**)dlp_calloc(freq_max - freq_min, sizeof(FLOAT64*));
  if(!m_lpFiltBR || !m_lpFiltBI) return ERR_MEM;
  for(i = 0; i < (freq_max - freq_min); i++) {
    *(m_lpFiltBR + i) = (FLOAT64*)dlp_calloc(i + freq_min, sizeof(FLOAT64));
    *(m_lpFiltBI + i) = (FLOAT64*)dlp_calloc(i + freq_min, sizeof(FLOAT64));
    if(!*(m_lpFiltBR+i) || !*(m_lpFiltBI+i)) return ERR_MEM;
    for(j = 0; j < (i + freq_min); j++) {
      *(*(m_lpFiltBR + i) + j) = cos(2.0 * F_PI * (FLOAT64)(i + freq_min - 1 - j) / (FLOAT64)(i + freq_min));
      *(*(m_lpFiltBI + i) + j) = sin(2.0 * F_PI * (FLOAT64)(i + freq_min - 1 - j) / (FLOAT64)(i + freq_min));
    }
  }
  return O_K;
}

INT16 CGEN_PRIVATE CPMproc::ChfaFreeTables() {
  UINT32  i;

  if(m_lpFiltBR) {
    for(i = 0; i < dlp_size(m_lpFiltBR)/sizeof(FLOAT64*); i++) {
      dlp_free(*(m_lpFiltBR + i));
    }
    dlp_free(m_lpFiltBR);
    m_lpFiltBR = NULL;
  }
  if(m_lpFiltBI) {
    for(i = 0; i < dlp_size(m_lpFiltBI)/sizeof(FLOAT64*); i++) {
      dlp_free(*(m_lpFiltBI + i));
    }
    dlp_free(m_lpFiltBI);
    m_lpFiltBI = NULL;
  }
  return O_K;
}
