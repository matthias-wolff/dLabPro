// dLabPro class CMELproc (MELproc)
// - Status information
//
// AUTHOR : C.-M. Westendorf, S. Wittenberg, Guntram Strecha
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

#include "dlp_melproc.h"

/**
 * Print status information
 *
 */
void CGEN_PUBLIC CMELproc::Status() {
  INT16 k;
  FLOAT32 f, mel,b,hl,hr;
  SWord*   lpField;

  CFBAproc::Status();

  printf("\nStatus of Instance MELproc %s:\n", this->m_lpInstanceName);
  if((lpField = CDlpObject_FindWordInternal(this, "coeff",           WL_TYPE_FIELD)) != NULL) printf("\n%-32s: %16ld                  %-100s",  lpField->lpName, m_nCoeff, lpField->lpComment);
  if((lpField = CDlpObject_FindWordInternal(this, "mf_type",         WL_TYPE_FIELD)) != NULL) printf("\n%-32s: %16s                  %-100s",   lpField->lpName, m_lpsMfType, lpField->lpComment);
  printf("\n");

  if((lpField = CDlpObject_FindWordInternal(this, "/syn_mcep",       WL_TYPE_OPTION)) != NULL) printf("\n%-32s: %16s                  %-100s",   lpField->lpName, (m_bSynMcep==TRUE)        ? "TRUE":"FALSE", lpField->lpComment);
  if((lpField = CDlpObject_FindWordInternal(this, "/syn_cep",        WL_TYPE_OPTION)) != NULL) printf("\n%-32s: %16s                  %-100s",   lpField->lpName, (m_bSynCep==TRUE)         ? "TRUE":"FALSE", lpField->lpComment);
  if((lpField = CDlpObject_FindWordInternal(this, "/syn_mlsadf",     WL_TYPE_OPTION)) != NULL) printf("\n%-32s: %16s                  %-100s",   lpField->lpName, (m_bSynMlsadf==TRUE)      ? "TRUE":"FALSE", lpField->lpComment);
  printf("\n");
  printf("\n");

  if (!m_lpCnvc) return;

  printf("\n --- convcore: convolution parameter display --------------------------- ");
  printf("\n                                                     f_width(Hz)         ");
  printf("\n    k      mid f_mid(Hz)    Bark   b[Hz]     width   left  right  norm   ");
  printf("\n");
  for (k=0; k<m_lpCnvc->n_out; k++)
  {
    f   = (FLOAT32)(m_lpCnvc->mid[k]*m_nSrate)/m_nLen;
    hl   = (FLOAT32)(m_lpCnvc->width[0][k]*m_nSrate)/m_nLen;
    hr   = (FLOAT32)(m_lpCnvc->width[1][k]*m_nSrate)/m_nLen;
    mel = 13.0f * (FLOAT32)atan (0.76 * f / 1000.0) +
          3.5f * (FLOAT32)atan(dlm_pow((FLOAT64)(f/7500.0),2.0));
    b   = 25.0f + 75.0f * (FLOAT32)dlm_pow((FLOAT64)( 1.0 + 1.4 *
          (FLOAT32)dlm_pow((FLOAT64) f / 1000.0 , 2.0)), 0.69);
    printf("\n %4d %8.2f %8.2f %8.2f %8.2f %8.2f",k,m_lpCnvc->mid[k],f,mel,b,m_lpCnvc->width[0][k]+m_lpCnvc->width[1][k]);
    printf(" %8.2f %8.2f  %10.5f ",hl, hr, m_lpCnvc->norm[k]);
  }
  printf("\n");
  printf("\n ------------------------------------------------------------------------ ");
  printf("\n");
}

// EOF
