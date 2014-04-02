// dLabPro class CHistogram (histogram)
// - Data check and ERROR display
//
// AUTHOR : Christian-M. Westendorf, Matthias Wolff
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

#include "dlp_histogram.h"

/**
 * Check minmax data, return mode.
 */
INT16 CGEN_PROTECTED CHistogram::CheckMinmax()
{
  if (data_empty (m_minmax) == TRUE)
    return IERROR(this,HIS_EMPTY,0,0,0);

  if (data_chtype (m_minmax, T_DOUBLE) != TRUE)
    return IERROR(this,HIS_TYPE,0,0,0);

  if (data_nrec(m_minmax) == 2 && data_dim(m_minmax) == 1)
    return (1);
  if (data_nrec(m_minmax) == 2)
    return (2);
  if (data_nrec(m_minmax) > 2)
    return (3);

  return (-1);
}

/**
 * Check minmax data, setup hmode.
 */
INT16 CGEN_PROTECTED CHistogram::SetupHmode()
{
  m_hmode = CheckMinmax();
  return (m_hmode);
}

/**
 * Check histogram data.
 */
INT16 CGEN_PROTECTED CHistogram::CheckHisto()
{
  INT16 mode;

  if (data_empty (m_hist) == TRUE)
    return IERROR(this,HIS_EMPTY,0,0,0);

  if (data_chtype (m_hist, T_DOUBLE) != TRUE)
    return IERROR(this,HIS_TYPE,0,0,0);

  m_bins = (INT32) data_descr(m_hist,DESCR2);

  if (m_bins < 0 || m_bins > data_nrec(m_hist))
    return IERROR(this,HIS_HISTO,0,0,0);

  m_hdim = data_ndim(m_hist);
  m_nhist = data_nblock(m_hist);
  if (m_nhist == 0)
    m_nhist = data_nrec(m_hist) / m_bins;

  set_data_nblock(m_hist, m_nhist);

  if (m_nhist == 0)
    return IERROR(this,HIS_HISTO,0,0,0);

  mode = (INT16) data_descr(m_hist,DESCR1);
  if (mode >= 1 && mode <= 3)
    m_hmode = mode;
  else
    set_data_descr (m_hist,mode,DESCR1);

  return (O_K);
}

/**
 * Check consistency between histogram and minmax data.
 */
INT16 CGEN_PROTECTED CHistogram::CheckConsist()
{
  INT16 ierr, mode;
  INT32 nhist;

  mode = HIS_MODE;
  nhist = data_nblock(m_hist);

  mode = CheckMinmax();
  if (mode == -1)
    return (NOT_EXEC);

  if ((ierr = CheckHisto()) != O_K)
    return (ierr);

  switch (mode)
  {
  case 1:
    if (data_nrec(m_minmax) != 2 || data_dim (m_minmax) != 1)
      return IERROR(this,HIS_INCONS,0,0,0);
    break;

  case 2:
    if (data_nrec(m_minmax) != 2)
      return IERROR(this,HIS_INCONS,0,0,0);
    if (data_dim(m_minmax) != data_dim(m_hist))
      return IERROR(this,HIS_INCONS,0,0,0);
    break;

  case 3:
    if (data_nrec(m_minmax) < 2 * nhist)
      return IERROR(this,HIS_INCONS,0,0,0);
    if (data_dim(m_minmax) != data_dim(m_hist))
      return IERROR(this,HIS_INCONS,0,0,0);
    break;

  default:
    return IERROR(this,HIS_INCONS,0,0,0);
  }

  return (O_K);
}

// EOF
