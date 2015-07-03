// dLabPro class CFBAproc (FBAproc)
// - Class CFBAproc - FFT wrapper code
//
// AUTHOR : Matthias Eichner, Dresden
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
#include "dlp_math.h"

/**
 * Calculate magnitude of spectrum.
 *
 * @return O_K if successfull, NOT_EXEC otherwise
 */
INT16 CGEN_PROTECTED CFBAproc::MAG()
{
  FLOAT64* real = NULL;
  FLOAT64* imag = NULL;

  real = (FLOAT64*)m_idRealFrame->XAddr(0,0);
  imag = (FLOAT64*)m_idImagFrame->XAddr(0,0);

  DLPASSERT(real);
  DLPASSERT(imag);

  return dlm_fft_mag(real, imag, m_nLen);
}


/**
 * Calculate log10 of magnitude spectrum.
 *
 * @return O_K if successfull, NOT_EXEC otherwise
 */
INT16 CGEN_PROTECTED CFBAproc::LOG10() {
  FLOAT64*  real = (FLOAT64*)m_idRealFrame->XAddr(0,0);
  FLOAT64*  imag = (FLOAT64*)m_idImagFrame->XAddr(0,0);
  DLPASSERT(real);
  DLPASSERT(imag);
  return dlm_fft_log10(real, imag, m_nLen, m_nMinLog);
}

INT16 CGEN_PROTECTED CFBAproc::LN() {
  register FLOAT64*  real = NULL;
  register FLOAT64*  imag = NULL;

  real = (FLOAT64*)m_idRealFrame->XAddr(0,0);
  imag = (FLOAT64*)m_idImagFrame->XAddr(0,0);

  DLPASSERT(real);
  DLPASSERT(imag);

  return dlm_fft_ln(real, imag, m_nLen, m_nMinLog);
}

/* EOF */
