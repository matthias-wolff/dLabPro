// dLabPro class CCPproc (CPproc)
// - Class CCPproc - status
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


#include "dlp_cpproc.h"

void CGEN_PUBLIC CCPproc::Status() {
  SWord* lpField;

  CMELproc::Status();

  printf("\nStatus of Instance CPproc %s:\n", this->m_lpInstanceName);
  if((lpField = CDlpObject_FindWordInternal(this, "coeff", WL_TYPE_FIELD)) != NULL) printf(
      "\n%-32s: %16ld                  %-100s", lpField->lpName, (long)m_nCoeff, lpField->lpComment);
  printf("\n");
  printf("\n");
}

// EOF
