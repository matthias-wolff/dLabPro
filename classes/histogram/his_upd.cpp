// dLabPro class CHistogram (histogram)
// - Histogram update
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
 * Increment vector histogram.
 *
 * @param v  quant vector
 * @param w  increment
 * @param il index
 */
void CGEN_PROTECTED CHistogram::IncrementHisto(INT32* v, FLOAT64 w, INT32 il)
{
  INT32 k, q;
  FLOAT64* p;

  p = (FLOAT64*) xaddr(m_hist,il*m_bins,0);

  for (k = 0; k < m_hdim; k++)
  {
    q = v[k];
    if (q > -1 && q < m_bins)
      p[q * m_hdim + k] += w;
  }
}

/**
 * Search index maximum
 *
 * @param ind index sequence
 * @param i   component containing index
 * @return max. index value or -1, if ind empty
 */
static INT32 CHistogram_max_index(CData* ind, INT32 i)
{
  INT32 j, n;
  FLOAT64 j1, jmax;

  if (data_empty(ind) == TRUE)
    return (-1);

  jmax = (INT32) dfetch(ind,0,i);
  n = data_nrec(ind);
  for (j = 1; j < n; j++)
    if ((j1 = dfetch(ind,j,i)) > jmax)
      jmax = j1;
  return ((INT32) jmax);
}

/**
 * Prepare histogram data.
 *
 * @param x  first data
 * @param wv weight data
 * @return max. index value or -1, if index empty
 */
INT32 CGEN_PROTECTED CHistogram::Prepare(CData* x, CData* wv)
{
  INT32 imax = 1, i, n;

  if (m_hist == NULL)
    m_hist = data_create("hist");

  m_nhist = data_nblock(m_hist);

  if (data_empty(m_indexlist) != TRUE)
  {
    imax = CHistogram_max_index(m_indexlist, 0) + 1;
    if (imax > m_nhist)
      m_nhist = imax;
  }

  if (data_empty(wv) != TRUE)
  {
    imax = data_ndim(wv);
    if (imax > m_nhist)
      m_nhist = imax;
  }

  if (m_nhist == 0)
    m_nhist = 1;

  if (data_empty(m_hist) != TRUE)
  {
    data_realloc (m_hist, m_nhist * m_bins);
    set_data_nrec (m_hist, m_nhist * m_bins);
    m_hdim = data_dim(m_hist);
  } else
  {
    data_reset(m_hist);
    n = data_dim(x);
    m_hdim = 0;
    for (i = 0; i < n; i++)
      if (comp_type(x,i) > 256 && i != m_icomp)
      {
        comp_def(m_hist, comp_text(x,i),T_DOUBLE);
        m_hdim++;
      }
    data_arr_alloc (m_hist, m_bins * m_nhist);
  }

  set_data_descr(m_hist, DESCR1, (FLOAT64)m_hmode);
  set_data_descr(m_hist, DESCR2, (FLOAT64)m_bins);

  set_data_nblock(m_hist, m_nhist);

  return (m_nhist);
}

/**
 * Update histogram data st by data records from x
 *
 * Uses the followong fields of class CVh:
 * hist      - histogram data       (must be double, may be empty)
 * minp      - minima-maxima        (must be double)
 * icomp     - index component      (may be -1, if not index in data x)
 * nind      - index number         (only for initialization required)
 * m_bins    - histogram resolution (only for initialization required)
 * indexlist - index list of vectors
 *
 * if index list cannot be created, index indep. histogram is made:
 *    - no label,  no index comp. or no label file
 *    - label, but no ltab
 * mode  -  1  constant data interval           (minm->dim  == 1,
 *                                               minm->nvec == 1)
 *          2  component spec. quantization     (minm->nvec == 2)
 *          3  index - comp. spec. quantization (minm->nvec > 2)
 *
 * @param x  Feature vector sequence
 * @param wv Weighting vector sequence (may be NULL)
 * @return O_K if ok or NOT_EXEC if not executed
 */
INT16 CGEN_PUBLIC CHistogram::UpdateHist(CData* x, CData* wv)
{
  INT32 t, il, wdim, T;
  FLOAT64 cw;
  FLOAT64 *xp, *wvp, *p_mm;
  INT16 lflag, flag_w, mflag;
  INT32 *qx;
  INT16 ierr;

  IDENTIFY ("CVh::UpdateHist");

  /* Check input data */
  if (data_empty(x) == TRUE)
    return IERROR(this,HIS_NOINP,0,0,0);
  if (data_ndim(x) == 0)
    return IERROR(this,HIS_NODAT,0,0,0);
  T = data_nrec(x);
  m_count = 0;

  /* Check index list data */

  lflag = 0;
  if (data_empty(m_indexlist) != TRUE)
    lflag = 1;

  /* Use minp data */
  SetupHmode();
  if (m_hmode <= 0)
    return NOT_EXEC;
  p_mm = (FLOAT64*) data_ptr(m_minmax);

  /* Prepare histogram array */
  Prepare(x, wv);
  if ((ierr = CheckConsist()) != O_K)
    return NOT_EXEC;

  /* Check weight data */
  flag_w = 0;
  wvp = NULL;
  wdim = data_ndim(wv);
  if (data_empty(wv) != TRUE && lflag == 0)
  {
    if (wdim < m_nhist)
      return (NOT_EXEC);
    flag_w = 1;
    wvp = (FLOAT64*) dl_calloc (wdim,sizeof(FLOAT64),"UpdateHist");
  }

  mflag = 0;
  if (data_empty(wv) != TRUE && lflag == 1)
  {
    if (wdim == 1)
      mflag = 1;
  }

  if (flag_w == 1 || mflag == 1)
    if (data_nrec(wv) < T)
    {
      T = data_nrec(wv);
      printf("histogram update: less weights than data ... only %d records", T);
    }

  /* Aux. arrays */
  qx = (INT32*) dl_calloc(m_hdim+2, sizeof(INT32), "UpdateHist");
  xp = (FLOAT64*) dl_calloc(m_hdim+2, sizeof(FLOAT64),"UpdateHist");

  /* Hard update, index driven */
  cw = 1.;
  if (flag_w == 0)
  {
    il = 0;
    for (t = 0; t < T; t++)
    {
      dvec_fetch(xp, x, t, m_hdim, m_icomp);
      if (lflag != 0)
        il = (INT32) dfetch(m_indexlist,t,0);
      if (il > -1 && il < m_nhist)
      {
        m_count++;
        if (mflag == 1)
          cw = dfetch(wv,t,0);
        QuantVec(xp, qx, p_mm, il);
        IncrementHisto(qx, cw, il);
      }
    }
  }

  /* Soft updating weighted by w-array */
  if (flag_w == 1)
  {
    for (t = 0; t < T; t++)
    {
      dvec_fetch(wvp,wv,t,wdim,-1);
      dvec_fetch(xp,x,t,m_hdim,m_icomp);
      for (il = 0; il < m_nhist; il++)
      {
        m_count++;
        QuantVec(xp, qx, p_mm, il);
        IncrementHisto(qx, wvp[il], il);
      }
    }
  }

  /* Ending activities */dl_free(qx);
  dl_free(xp);
  if (flag_w == 1)
    dl_free (wvp);

  return O_K;
}

// EOF
