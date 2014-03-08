// dLabPro class CFTTproc (FTTproc)
// - Status information
//
// AUTHOR : Steffen Kuerbis
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

#include "dlp_fttproc.h"

/**
 * Print status information
 *
 */
void CGEN_PUBLIC CFTTproc::Status() {
  INT16 k;
  //SWord*   lpField;

  CFBAproc::Status();

  printf("\nStatus of Instance FTTproc %s:\n", this->m_lpInstanceName);
  printf("\n");
  printf("\n");

  if (!m_lpFtt) return;
  printf("\n ---_ FTT-parameters  --------------------------- ");
  printf("\n");
  printf("\n start frequency[Hz]:        %8.2f", m_lpFtt->start_freq);
  printf("\n bandwidth [Bark]:           %8.2f", m_lpFtt->bw_coeff);
  printf("\n frequency increment [Bark]: %8.2f", m_lpFtt->finc_coeff);
  printf("\n smooth_coeff:               %8.2f", m_lpFtt->sm_coeff);
  printf("\n norm_coeff:                 %8.2f", m_lpFtt->norm_coeff);
  printf("\n ftt_type:                          %s ", m_lpFtt->type);
  if (dlp_charin('C', m_lpFtt->type))
  {  printf("\n log_range:                  %8.2f", m_lpFtt->log);
     printf("\n Max_value:                  %8.2f", m_lpFtt->max_value);
  }
  printf("\n");
  printf("\n");

  printf("\n --- frequency/bandwidth pairs  --------------------------- ");
  printf("\n");
  printf("\n    k      mid(Hz)      b[Hz]    ");
  printf("\n");
  for (k=0; k<m_nCoeff; k++)
  {
    printf("\n %4d    %8.2f    %8.2f",k,(m_lpFtt->midfreq[k]),m_lpFtt->bandwidth[k]);
  }
  printf("\n");
/*  printf("\n /noreset:        %s", (m_bNoreset==TRUE) ? "TRUE":"FALSE");*/
  printf("\n ------------------------------------------------------------------------ ");
  printf("\n");
}

// EOF
