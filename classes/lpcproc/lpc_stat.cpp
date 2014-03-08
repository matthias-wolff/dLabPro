// dLabPro class CLPCproc (LPCproc)
// - class LPCproc - status
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

#include "dlp_lpcproc.h"

void CGEN_PUBLIC CLPCproc::Status() {
  SWord*   lpField;

  CFBAproc::Status();

  printf("\nStatus of Instance LPCproc %s:\n", this->m_lpInstanceName);
  if((lpField = CDlpObject_FindWordInternal(this, "coeff",           WL_TYPE_FIELD)) != NULL) printf("\n%-32s: %16ld                  %-100s",  lpField->lpName, m_nCoeff, lpField->lpComment);
  printf("\n");
  printf("no options\n");
  printf("\n");

}

// EOF
