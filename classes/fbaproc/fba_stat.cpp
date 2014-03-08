// dLabPro class CFBAproc (FBAproc)
// - Status information
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

#include "dlp_fbaproc.h"

/**
 * Print status information
 *
 */
void CGEN_PUBLIC CFBAproc::Status() {
  SWord*   lpField;

  printf("\nStatus of Instance FBAproc %s:\n", this->m_lpInstanceName);
  if((lpField = CDlpObject_FindWordInternal(this, "pfa_lambda",      WL_TYPE_FIELD)) != NULL) printf("\n%-32s: %33.16f %-100s",                 lpField->lpName, (double)m_nPfaLambda, lpField->lpComment);
  if((lpField = CDlpObject_FindWordInternal(this, "syn_lambda",      WL_TYPE_FIELD)) != NULL) printf("\n%-32s: %33.16f %-100s",                 lpField->lpName, (double)m_nSynLambda, lpField->lpComment);
  if((lpField = CDlpObject_FindWordInternal(this, "crate",           WL_TYPE_FIELD)) != NULL) printf("\n%-32s: %16ld                  %-100s",  lpField->lpName, (  long)m_nCrate, lpField->lpComment);
  if((lpField = CDlpObject_FindWordInternal(this, "DC",              WL_TYPE_FIELD)) != NULL) printf("\n%-32s: %33.16f %-100s",                 lpField->lpName, (double)m_nDC, lpField->lpComment);
  if((lpField = CDlpObject_FindWordInternal(this, "len",             WL_TYPE_FIELD)) != NULL) printf("\n%-32s: %16ld                  %-100s",  lpField->lpName, (  long)m_nLen, lpField->lpComment);
  if((lpField = CDlpObject_FindWordInternal(this, "matrix_analysis", WL_TYPE_FIELD)) != NULL) printf("\n%-32s: %16ld                  %-100s",  lpField->lpName, (  long)m_nMatrixAnalysis, lpField->lpComment);
  if((lpField = CDlpObject_FindWordInternal(this, "min_log",         WL_TYPE_FIELD)) != NULL) printf("\n%-32s: %33.16f %-100s",                 lpField->lpName, (double)m_nMinLog, lpField->lpComment);
  if((lpField = CDlpObject_FindWordInternal(this, "n_periods",       WL_TYPE_FIELD)) != NULL) printf("\n%-32s: %16ld                  %-100s",  lpField->lpName, (  long)m_nNPeriods, lpField->lpComment);
  if((lpField = CDlpObject_FindWordInternal(this, "out_dim",         WL_TYPE_FIELD)) != NULL) printf("\n%-32s: %16ld                  %-100s",  lpField->lpName, (  long)m_nOutDim, lpField->lpComment);
  if((lpField = CDlpObject_FindWordInternal(this, "preem",           WL_TYPE_FIELD)) != NULL) printf("\n%-32s: %33.16f %-100s",                 lpField->lpName, (double)m_nPreem, lpField->lpComment);
  if((lpField = CDlpObject_FindWordInternal(this, "quantization",    WL_TYPE_FIELD)) != NULL) printf("\n%-32s: %16ld                  %-100s",  lpField->lpName, (  long)m_nQuantization, lpField->lpComment);
  if((lpField = CDlpObject_FindWordInternal(this, "srate",           WL_TYPE_FIELD)) != NULL) printf("\n%-32s: %33.16f %-100s",                 lpField->lpName, (double)m_nSrate, lpField->lpComment);
  if((lpField = CDlpObject_FindWordInternal(this, "sync",            WL_TYPE_FIELD)) != NULL) printf("\n%-32s: %16ld                  %-100s",  lpField->lpName, (  long)m_nSync, lpField->lpComment);
  if((lpField = CDlpObject_FindWordInternal(this, "wlen",            WL_TYPE_FIELD)) != NULL) printf("\n%-32s: %16ld                  %-100s",  lpField->lpName, (  long)m_nWlen, lpField->lpComment);
  if((lpField = CDlpObject_FindWordInternal(this, "wnorm",           WL_TYPE_FIELD)) != NULL) printf("\n%-32s: %16ld                  %-100s",  lpField->lpName, (  long)m_nWnorm, lpField->lpComment);
  if((lpField = CDlpObject_FindWordInternal(this, "wtype",           WL_TYPE_FIELD)) != NULL) printf("\n%-32s: %16s                  %-100s",   lpField->lpName,         m_lpsWtype, lpField->lpComment);
  if((lpField = CDlpObject_FindWordInternal(this, "type",            WL_TYPE_FIELD)) != NULL) printf("\n%-32s: %16s                  %-100s",   lpField->lpName,         m_lpsType, lpField->lpComment);
  if((lpField = CDlpObject_FindWordInternal(this, "base_f0",         WL_TYPE_FIELD)) != NULL) printf("\n%-32s: %33.16f %-100s",                 lpField->lpName, (double)m_nBaseF0, lpField->lpComment);
  if((lpField = CDlpObject_FindWordInternal(this, "base_i0",         WL_TYPE_FIELD)) != NULL) printf("\n%-32s: %33.16f %-100s",                 lpField->lpName, (double)m_nBaseI0, lpField->lpComment);
  printf("\n");
  if((lpField = CDlpObject_FindWordInternal(this, "/delta_mf",            WL_TYPE_OPTION)) != NULL) printf("\n%-32s: %16s                  %-100s",   lpField->lpName, (m_bDeltaMf==TRUE)           ? "TRUE":"FALSE", lpField->lpComment);
  if((lpField = CDlpObject_FindWordInternal(this, "/energy",              WL_TYPE_OPTION)) != NULL) printf("\n%-32s: %16s                  %-100s",   lpField->lpName, (m_bEnergy==TRUE)            ? "TRUE":"FALSE", lpField->lpComment);
  if((lpField = CDlpObject_FindWordInternal(this, "/log_energy",          WL_TYPE_OPTION)) != NULL) printf("\n%-32s: %16s                  %-100s",   lpField->lpName, (m_bLogEnergy==TRUE)         ? "TRUE":"FALSE", lpField->lpComment);
  if((lpField = CDlpObject_FindWordInternal(this, "/rmdc",                WL_TYPE_OPTION)) != NULL) printf("\n%-32s: %16s                  %-100s",   lpField->lpName, (m_bRmdc==TRUE)              ? "TRUE":"FALSE", lpField->lpComment);
  if((lpField = CDlpObject_FindWordInternal(this, "/time_domain_warping", WL_TYPE_OPTION)) != NULL) printf("\n%-32s: %16s                  %-100s",   lpField->lpName, (m_bTimeDomainWarping==TRUE) ? "TRUE":"FALSE", lpField->lpComment);
  if((lpField = CDlpObject_FindWordInternal(this, "/ana_smooth_fea",      WL_TYPE_OPTION)) != NULL) printf("\n%-32s: %16s                  %-100s",   lpField->lpName, (m_bAnaSmoothFea==TRUE)      ? "TRUE":"FALSE", lpField->lpComment);
  if((lpField = CDlpObject_FindWordInternal(this, "/syn_smooth_fea",      WL_TYPE_OPTION)) != NULL) printf("\n%-32s: %16s                  %-100s",   lpField->lpName, (m_bSynSmoothFea==TRUE)      ? "TRUE":"FALSE", lpField->lpComment);
  if((lpField = CDlpObject_FindWordInternal(this, "/syn_enhancement",     WL_TYPE_OPTION)) != NULL) printf("\n%-32s: %16s                  %-100s",   lpField->lpName, (m_bSynEnhancement==TRUE)    ? "TRUE":"FALSE", lpField->lpComment);
  printf("\n");
  printf("\n");
}

// EOF
