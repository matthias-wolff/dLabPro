// dLabPro class CHistogram (histogram)
// - Histogram organization
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
 * Returns bytes per histogram block.
 */
INT32 CGEN_PROTECTED CHistogram::BytesPerBlock()
{
  return (m_bins * data_reclen(m_hist));
}

/**
 * Quantization of vector components type 1 (constant quant. interval).
 *
 * Uses the following fields of class CVh:
 * m_bins  - quantization steps
 * m_hdim  - components of u,v
 * wflag - window flag
 *    0  - quant. all data
 *    1  - quant. only in window min,max
 *
 * @param u    input vector
 * @param v    quant. vector
 * @param minm minmax array
 * @param il   index
 */
void CGEN_PROTECTED CHistogram::QuantVec
(
  FLOAT64* u,
  INT32*   v,
  FLOAT64* minm,
  INT32    il
)
{
  INT32 i = 0;
  INT32 q = 0;
  INT32 omax = 0;
  INT32 omin = 0;
  FLOAT64 min = 0.;
  FLOAT64 c1 = 0.;

  min = minm[0];
  c1 = m_bins / (minm[1] - min);

  if (m_hmode == 3)
  {
    omin = 2 * il * m_hdim;
    omax = 2 * il * m_hdim + m_hdim;
  }

  for (i = 0; i < m_hdim; i++)
  {
    if (m_hmode == 2)
      c1 = m_bins / (minm[i + m_hdim] - minm[i]);
    if (m_hmode == 3)
      c1 = m_bins / (minm[i + omax] - minm[i + omin]);
    if (c1 == 1.0)
      q = (INT32) (u[i] - min);
    else
      q = (INT32) (c1 * (u[i] - min));
    if (m_wflag == 0)
    {
      if (q > m_bins - 1)
        q = m_bins - 1;
      if (q < 0)
        q = 0;
    } else
    {
      if (q > m_bins - 1)
        q = -1;
      if (q < 0)
        q = -1;
    }
    v[i] = q;
  }
}

/**
 * Select histogram block i, store to ihist.
 *
 * @param ihist histogram i
 * @param  i    histogram block index
 * @param  j    histogram component index (select only a component);
 *              -1 to select complete block
 * @return O_K if ok, NOT_EXEC if not executed
 */
INT16 CGEN_PUBLIC CHistogram::SelectBlock(CData* ihist, INT32 i, INT32 j)
{
  INT32 l, rl;

  if (CheckHisto() != O_K)
    return (NOT_EXEC);
  if (ihist == NULL)
    return (NOT_EXEC);
  if (i < 0 || i >= m_nhist)
    return (NOT_EXEC);

  rl = BytesPerBlock();

  data_reset (ihist);
  if (j < 0)
  {
    data_scopy (m_hist, ihist);
    data_arr_alloc (ihist,m_bins);
    copy_data_descr (m_hist, ihist);
    set_data_nblock (ihist, 1);
    dl_memmove ( (char*)data_ptr(ihist),
        (char*)xaddr(m_hist,i*m_bins,0), rl);
    return (O_K);
  }

  if (j > -1 && j < data_dim(m_hist))
  {
    comp_mdef (ihist,1,T_DOUBLE);
    data_arr_alloc (ihist,m_bins);
    copy_comp_text (m_hist, j, ihist, 0, 1);
    copy_data_descr(m_hist, ihist);
    set_data_nblock (ihist, 1);
    for (l = 0; l < m_bins; l++)
      dstore (dfetch(m_hist,i*m_bins+l,j), ihist,l,0);
    return (O_K);
  }

  return (NOT_EXEC);
}

/**
 * Copy histogram block i to histogram block j.
 *
 * Remark: If j < 0, block i is copied to all histogram blocks.
 *
 * @return O_K if ok, NOT_EXEC if not executed
 */
INT16 CGEN_PUBLIC CHistogram::CopyBlock(INT32 i, INT32 j)
{
  INT32 l, rl;

  if (CheckHisto() != O_K)
    return (NOT_EXEC);
  if (i < 0 || i >= m_nhist)
    return (NOT_EXEC);

  rl = BytesPerBlock();

  if (j < 0)
  {
    for (l = 0; l < m_nhist; l++)
      dl_memmove ( (char*)xaddr(m_hist,l*m_bins,0),
          (char*)xaddr(m_hist,i*m_bins,0), rl);
    return (O_K);
  }

  if (j > -1 && j < m_hdim)
  {
    dl_memmove ( (char*)xaddr(m_hist,j*m_bins,0),
        (char*)xaddr(m_hist,i*m_bins,0), rl);
    return (O_K);
  }

  return (NOT_EXEC);
}

/**
 * Reorganize histogram.
 *
 * <p>Remarks</p>
 * <ul>
 *   <li>If (exist == NULL), the number of histogram blocks is set to nind,
 *       by adding empty blocks or removing blocks from the end.</li>
 *   <li>If (exist != NULL), histogram blocks may selected as follows:
 *       exist(i,0) == 0: block i is deleted;
 *       exist(i,0) >  0: block i is preserved</li>
 *   <li> Only preserved blocks will be available after reorganization! The
 *        index correspondence may be changed by this operation!nind is not used
 *        in this case.</li>
 * </ul>
 *
 * @param exist  -  index existing flag sequence (may be NULL)
 * @param nind   -  index number
 * @return O_K if ok, NOT_EXEC if not executed
 */
INT16 CGEN_PUBLIC CHistogram::ReorgEx(CData* exist, INT32 nind)
  {
  INT32 n, ifree, i, imax, rl;
  CData* h;

  if (CheckHisto() != O_K)
    return (NOT_EXEC);

  /* --- change only histogram number */
  if (exist == NULL)
  {
    if (nind > 0 && nind != data_nblock(m_hist))
    {
      data_realloc (m_hist, nind * m_bins);
      set_data_nblock(m_hist, nind);
      set_data_nrec (m_hist, nind * m_bins);
      return (O_K);
    } else
      return (NOT_EXEC);
  }

  /* --- reorganize according to exist */
  if (exist != NULL)
  {
    imax = data_nrec(exist);

    n = 0;
    for (i = 0; i < imax; i++)
    {
      if (dfetch (exist,i,0) > 0.0)
        n++;
    }

    if (n == imax)
      return (O_K);

    h = data_create("h");
    data_copy (m_hist,h);

    rl = BytesPerBlock();
    data_realloc (m_hist, n * m_bins);

    ifree = 0;

    for (i = 0; i < imax; i++)
    {
      if (dfetch (exist,i,0) > 0.0)
      {
        dl_memmove ( xaddr(m_hist,ifree,0),xaddr(h,i*m_bins,0), rl);
        ifree += m_bins;
      }
    }

    if (m_hmode == 3)
    {
      data_copy (m_hist,h);
      rl = 2 * data_reclen(m_minmax);
      data_realloc (m_minmax, n * 2);

      ifree = 0;

      for (i = 0; i < imax; i++)
      {
        if (dfetch (exist,i,0) > 0.0)
        {
          dl_memmove ( xaddr(m_minmax,ifree,0),xaddr(h,2*i,0), rl);
          ifree += 2;
        }
      }
    }

    data_destroy(h);

    set_data_nblock(m_hist, n);
    return (O_K);
  }

  return (O_K);
}

/**
 * Compute minmax data of pooled histogram. The blocks of hist are added.
 *
 * @param minmax minmax data
 * @param mm     pooled minmax data
 * @return O_K if ok, NOT_EXEC if not executed
 */
static INT16 CHistogram_minmax_poole(CData* minmax, CData* mm)
{
  FLOAT64 d, dmi, dmx;
  INT16 i, j;

  if (mm == NULL)
    return (NOT_EXEC);
  if (data_empty(minmax) == TRUE)
    return (NOT_EXEC);

  if (data_nrec(minmax) == 2)
  {
    data_copy (minmax,mm);
    return (O_K);
  }

  data_reset (mm);
  data_scopy (minmax, mm);
  data_arr_alloc (mm, 2);

  for (j = 0; j < data_dim(minmax); j++)
  {
    dmi = dfetch(minmax,0,j);
    dmx = dfetch(minmax,1,j);

    for (i = 1; i < data_nrec(minmax) / 2; i++)
    {
      d = dfetch(minmax,2*i,j);
      if (d < dmi)
        dmi = d;
      d = dfetch(minmax,2*i+1,j);
      if (d > dmx)
        dmx = d;
    }
    dstore (dmi, mm, 0, j);
    dstore (dmx, mm, 1, j);
  }

  return (O_K);
}

/**
 * Compute pooled histogram h from histogram.
 *
 * <p>Remark: In the moment, only for m_hmode = 1, 2 (histogram resampling needed !)</p>
 */
INT16 CGEN_PUBLIC CHistogram::Poole(CHistogram* h)
{
  if (h == NULL)
    return (NOT_EXEC);
  if (CheckHisto() != O_K)
    return (NOT_EXEC);

  if (m_hmode > 2)
    return IERROR(this,HIS_POOLE, 0,0,0);

  CHistogram_minmax_poole(m_minmax, h->m_minmax);

  /* MWX 2004-03-10: Replaced by CData_Aggregate -->
   data_aggrop (m_hist, h->m_hist,  NULL, 0, OP_SUM, 3, T_DOUBLE, 0);
   */
  CData_Aggregate_Int(h->m_hist, m_hist, NULL, CMPLX(0), OP_SUM);
  /* <-- */
  h->m_nhist = 1;
  h->m_bins = m_bins;
  h->m_hmode = m_hmode;
  h->m_hdim = m_hdim;
  h->m_calls = m_calls;
  h->m_count = 0;
  h->m_ssize = m_ssize;
  if (m_hmode == 1)
  {
    m_min = dfetch(m_minmax,0,0);
    m_max = dfetch(m_minmax,1,0);
  }

  return (O_K);
}

// EOF
