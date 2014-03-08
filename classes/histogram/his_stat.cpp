// dLabPro class CHistogram (histogram)
// - Statistics
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
 * Compute sum of histogram ic in block hdat.
 *
 * @param hdat  pointer to histogram block
 * @param dim   dimensionality
 * @param steps histogram resolution
 * @param ic    component index
 */
static FLOAT64 CHistogram_sum
(
  FLOAT64* hdat,
  INT32    dim,
  INT32    steps,
  INT32    ic
)
{
  INT32 i;
  FLOAT64 s;

  s = 0.;
  for (i = 0; i < steps; i++)
    s += hdat[i * dim + ic];
  return (s);
}

/**
 * Return bin width of histogram k.
 *
 * @param hdat  pointer to min-max-data
 * @param dim   dimensionality
 * @param steps histogram resolution
 * @param mode  histogram-mode
 * @param ic    component index
 */
static FLOAT64 CHistogram_binwidth
(
  FLOAT64* minm,
  INT32    dim,
  INT16   mode,
  INT32    steps,
  INT32    ic
)
{
  FLOAT64 dx = 0.;

  if      (mode == 1) dx = (minm[1]-minm[0])/(FLOAT64)steps;
  else if (mode == 2) dx = (minm[dim+ic]-minm[ic])/(FLOAT64)steps;
  else if (mode == 3) dx = (minm[dim+ic]-minm[ic])/(FLOAT64)steps;

  return dx;
}

/**
 * Return upper border of histogram k.
 *
 * @param hdat  pointer to min-max-data
 * @param dim   dimensionality
 * @param steps histogram resolution
 * @param mode  histogram-mode
 * @param ic    component index
 */
static FLOAT64 CHistogram_uborder
(
  FLOAT64* minm,
  INT32    dim,
  INT16    mode,
  INT32    ic
)
{
  if (mode == 1)
    return (minm[0]);
  if (mode == 2)
    return (minm[ic]);
  if (mode == 3)
    return (minm[ic]);
  return (0);
}

/**
 * Compute lower border for quantil p.
 *
 * @param hdat  pointer to histogram
 * @param minm  pointer to minimum-maximum-record
 * @param p     quantil
 * @param dim   dimensionality
 * @param steps histogram resolution
 * @param ic    component index
 * @param n     order of moment
 * @param mode  histogram mode
 */
static FLOAT64 CHistogram_p_quantil
(
  FLOAT64* hdat,
  FLOAT64* minm,
  FLOAT64  p,
  FLOAT64* p0,
  INT32    dim,
  INT32    steps,
  INT16   ic,
  INT16   mode
)
{
  INT32 i;
  FLOAT64 g, h, s, q;
  FLOAT64 gr = 0.;

  s = CHistogram_sum(hdat, dim, steps, ic);
  h = p * s;

  g = 0.;
  for (i = 0; i < steps; i++)
  {
    g += hdat[i * dim + ic];
    if (g > h)
      break;
    gr = g;
  }

  p0[0] = gr / s;
  q = i * CHistogram_binwidth(minm, dim, mode, steps, ic);
  return q;
}

/* MW 2004-08-10: Not used!  -->
 * Compute geometric mean of a histogram
 *
 * @param hdat  Pointer to histogram
 * @param minm  Pointer to minimum-maximum-record
 * @param dim   Dimensionality
 * @param steps Histogram resolution
 * @param ic    Component index
 * @param mode  Histogram mode
 * @return geometric mean
static FLOAT64 comp_gmean
(
  FLOAT64* hdat,
  FLOAT64* minm,
  INT32    dim,
  INT32    steps,
  INT32    ic,
  INT16   mode
)
{
  INT32   i;
  FLOAT64 g,h,s;

  s = HIS_sum (hdat, dim, steps, ic);

  g = 0.;
  for (i=0; i< steps; i++)
  {
    h= hdat[i*dim+ic];
    if (h <= 0) h = 10e-300;
    g += log(h/s);
    //printf ("\n gcomp: h=%g s =%g g=%g %g", h,s,g,log(h/s));
  }

  g = g/log(10.);
  g = g/steps;

  return (pow(10., g));
}
<-- */

/**
 * Comp. n-th moment of component ic
 *
 * @param hdat  pointer to histogram
 * @param minm  pointer to minimum-maximum-record
 * @param dim   dimensionality
 * @param steps histogram resolution
 * @param ic    component index
 * @param n     order of moment
 * @param mode  histogram mode
 */
static FLOAT64 CHistogram_comp_nmoment
(
  FLOAT64* hdat,
  FLOAT64* minm,
  INT32    dim,
  INT32    steps,
  INT32    ic,
  INT32    n,
  INT16    mode
)
{
  INT32 i;
  FLOAT64 g, h, k, s;

  s = 0.;
  k = (FLOAT64) n + 1.;
  h = CHistogram_binwidth(minm, dim, mode, steps, ic);
  g = CHistogram_uborder(minm, dim, mode, ic);

  for (i = 0; i < steps; i++)
  {
    s += hdat[i * dim + ic] * (pow(g + h, k) - pow(g, k));
    g += h;
  }
  s /= (n + 1);
  return (s);
}

void CGEN_PUBLIC CHistogram::MeanVarComp ( FLOAT64 *hdat, FLOAT64 *minm,
                                FLOAT64 *mean, FLOAT64 *vari)
/* ---------------------------------------------------------
   comp. mean /variance vector of histogram block

   hdat  - pointer to histogram
   minm  - pointer to minimum-maximum-record
   mean  - mean vector (dim components)
   vari  - variance vector (dim components)
   dim   - dimensionality

------------------------------------------------------------ */
    {
    FLOAT64   d, s, v, h;
    INT32     ic;

  if (hdat == NULL) return;
  if (minm == NULL) return;

    dl_memset (mean, 0, m_hdim*sizeof(FLOAT64));
    dl_memset (vari, 0, m_hdim*sizeof(FLOAT64));

    for (ic=0; ic<m_hdim; ic++)
  {  d=CHistogram_comp_nmoment ( hdat, minm, m_hdim, m_bins, ic, 1, m_hmode);
        v=CHistogram_comp_nmoment ( hdat, minm, m_hdim, m_bins, ic, 2, m_hmode);
        s=CHistogram_sum   ( hdat, m_hdim, m_bins, ic );
        h = CHistogram_binwidth (minm, m_hdim, m_hmode, m_bins, ic);
        if (s > 0) mean[ic] = d/(s*h);
        if (s > 0) vari[ic] = v/(s*h)-pow( mean[ic],2.);
       }
    }

/**
 * Compute lower an higher border of p-quantil for one histogram block
 *
 * @param hdat  pointer to histogram
 * @param minm  pointer to minimum-maximum-record
 * @param p     order of quantil
 * @param qlo   upper border for quantil
 * @param pq    real quantil value
 * @param dim   dimensionality
 * @param steps histogram resolution
 * @param mode  histogram mode
 */
static void CHistogram_hist_quantil
(
  FLOAT64* hdat,
  FLOAT64* minm,
  FLOAT64  p,
  FLOAT64* qlo,
  FLOAT64* qhi,
  FLOAT64* pq,
  INT32    dim,
  INT32    steps,
  INT16    mode
)
{
  FLOAT64 h;
  short ic;

  dl_memset(qlo, 0, dim*sizeof(FLOAT64));
  dl_memset(qhi, 0, dim*sizeof(FLOAT64));
  dl_memset(pq, 0, dim*sizeof(FLOAT64));

  for (ic = 0; ic < dim; ic++)
  {
    h = CHistogram_binwidth(minm, dim, mode, steps, ic);
    qlo[ic] = CHistogram_p_quantil(hdat, minm, p, &pq[ic], dim, steps, ic, mode);
    qhi[ic] = qlo[ic] + h;
  }
}

/* MW 2004-08-10: Not used -->
   comp. geometric mean vector
         for one histogram block

   hdat  - pointer to histogram
   minm  - pointer to minimum-maximum-record
   mean  - mean vector (dim components)
   vari  - variance vector (dim components)
   dim   - dimensionality
   steps - histogram resolution
   mode  - histogram mode
static void hist_gmean
(
  FLOAT64* hdat,
  FLOAT64* minm,
  FLOAT64* gmean,
  INT32    dim,
  INT32    steps,
  INT16   mode
)
{
  INT32 ic;

  dl_memset(gmean,0,dim*sizeof(FLOAT64));

  for (ic=0; ic<dim; ic++) gmean[ic] = comp_gmean(hdat,minm,dim,steps,ic,mode);
}
<-- */

/**
 * Compute 2nd-mixed moment of component ic,jc.
 *
 * @param hdat  pointer to histogram
 * @param minm  pointer to minimum-maximum-record
 * @param dim   dimensionality
 * @param steps histogram resolution
 * @param ic    component index 1
 * @param jc    component index 2
 */
static FLOAT64 CHistogram_comp_mixed
(
  FLOAT64* hdat,
  FLOAT64* minm,
  INT32    dim,
  INT32    steps,
  INT32    ic,
  INT32    jc,
  INT16 mode
)
{
    INT32        i;
    FLOAT64  gi,hi,s;
    FLOAT64  gj,hj;

    s = 0.;
    hi = CHistogram_binwidth (minm, dim, mode, steps, ic);
    hj = CHistogram_binwidth (minm, dim, mode, steps, jc);
    gi = CHistogram_uborder  (minm, dim, mode, ic);
    gj = CHistogram_uborder  (minm, dim, mode, jc);

    for (i=0; i< steps; i++)
   s += hdat[i*dim+ic] * hdat[i*dim+jc] * (gi + i * hi + hi/2.)
                                              * (gj + i * hj + hj/2.);

    return (s);
    }

/**
 * Compute moment vector.
 *
 * @param hdat  pointer to histogram
 * @param mom   pointer to moment vector
 * @param dim   dimensionality
 * @param steps histogram resolution
 * @param n     order of moment
 */
static void CHistogram_nmoment_2
(
  FLOAT64* hdat,
  FLOAT64* mom,
  FLOAT64* minp,
  INT32    dim,
  INT32    steps,
  INT32    n,
  INT16    mode
)
{
  INT32 k;

  for (k = 0; k < dim; k++)
    mom[k] = CHistogram_comp_nmoment(hdat, minp, dim, steps, k, n, mode);
}

/**
 * Compute mixed moment matrix.
 *
 * @param hdat  pointer to histogram
 * @param mom   pointer to moment matrix
 * @param dim   dimensionality
 * @param steps histogram resolution
 */
static void CHistogram_mmoment_2
(
  FLOAT64* hdat,
  FLOAT64* mom,
  FLOAT64* minp,
  INT32    dim,
  INT32    steps,
  INT16    mode
)
{
  INT32 i, k;

  for (k = 0; k < dim; k++)
    for (i = 0; i < dim; i++)
      mom[k * dim + i] = CHistogram_comp_mixed(hdat, minp, dim, steps, k, i,
          mode);
}

/**
 * Calculate statistic from vector histograms
 *
 * @param vhisto absolute histogram data
 * @param stat   statistic data, generated
 * @param minm   minmax vector sequence
 * @param smode  statistic  mode
 */
INT16 CGEN_PUBLIC CHistogram::GenStatistic(data* stat, INT16 smode)
{
  INT32   k,l,offs,n;
  FLOAT64 *histp, *sp, *p_mm;
  FLOAT64 *sptr = NULL;
  INT32   sdim;

  IDENTIFY  ("CVh::GenStatistic");

  if (CheckConsist () != O_K)    return (NOT_EXEC);

  sdim  = data_dim(m_hist);
  histp = (FLOAT64*)data_ptr(m_hist);
  n     = data_nrec(m_hist)/m_bins;
  p_mm  = (FLOAT64*)data_ptr(m_minmax);

  data_reset    (stat);
  data_scopy    (m_hist, stat);

  if (smode == 0) data_arr_alloc(stat,n*(4+sdim));
  if (smode  > 0) data_arr_alloc(stat,n*(3+sdim));
  sp = (FLOAT64*)data_ptr(stat);

  copy_data_descr (m_hist, stat);

  switch(m_hmode)
  {

  case 1:
  case 2:
    for (l=0; l<n; l++)
    {
      offs = l * m_bins * sdim;
      if (smode > 0)
      {
        sptr = &sp[sdim*(3+smode)*l];
        for (k=1; k<=smode; k++)
          CHistogram_nmoment_2  ( &histp[offs], &sptr[(2+k)*sdim], p_mm, sdim, m_bins, k, m_hmode);
      }
      if (smode == 0)
      {
        sptr = &sp[sdim*(4+sdim)*l];
        CHistogram_nmoment_2  (&histp[offs], &sptr[3*sdim], p_mm, sdim, m_bins, 1, 2);
        CHistogram_mmoment_2  (&histp[offs], &sptr[4*sdim],p_mm, sdim, m_bins, m_hmode);
      }
      dl_memmove ( &sptr[sdim], p_mm, 2 * sdim * sizeof(FLOAT64));
      sptr[0] = CHistogram_sum (&histp[offs], sdim, m_bins, 0);
    }
    break;

  case 3:
    for (l=0; l<n; l++)
    {
      offs = l * m_bins * sdim;
      if (smode > 0)
      {
        sptr = &sp[sdim*(3+smode)*l];
        for (k=1; k<=smode; k++)
        CHistogram_nmoment_2(&histp[offs],&sptr[(2+k)*sdim],&p_mm[2*sdim*l],sdim,m_bins,k,m_hmode);
      }
      if (smode == 0)
      {
        sptr = &sp[sdim*(4+sdim)*l];
        CHistogram_nmoment_2(&histp[offs],&sptr[3*sdim],&p_mm[2*sdim*l],sdim,m_bins,1,m_hmode);
        CHistogram_mmoment_2(&histp[offs],&sptr[4*sdim],&p_mm[2*sdim*l],sdim,m_bins,m_hmode);
      }
      dl_memmove ( &sptr[sdim], &p_mm[2*sdim*l], 2 * sdim * sizeof(FLOAT64));
      sptr[0] = CHistogram_sum (&histp[offs], sdim, m_bins, 0);
    }
  break;
  }

  set_data_descr(stat, DESCR1, (FLOAT64)smode);
  return O_K;
}

/**
 * Compute mean/var from histogram.
 *
 * @param mean mean data
 * @param var  variance data
 */
INT16 CGEN_PUBLIC CHistogram::MeanVar(data* mean, data* var)
{
  INT16   ierr;
  FLOAT64* mp = NULL;
  INT32    j;

  IDENTIFY  ("CVh::MeanVar");
  if ((ierr = CheckConsist ()) != O_K ) return(ierr);

  data_reset(mean);
  data_reset(var);
  data_scopy(m_hist,mean);
  data_scopy(m_hist,var);
  ierr=data_arr_alloc(mean,m_nhist);
  ierr=data_arr_alloc(var, m_nhist);

  if (ierr != O_K) return (ierr);

  for (j=0; j<m_nhist;j++)
  {
    if (m_hmode < 3)  mp = (FLOAT64*)xaddr(m_minmax,0,0);
    if (m_hmode == 3) mp = (FLOAT64*)xaddr(m_minmax,2 *j,0);

    MeanVarComp
    (
      (FLOAT64*)xaddr(m_hist,j*m_bins,0),mp,
      (FLOAT64*)xaddr(mean,j,0), (FLOAT64*)xaddr(var,j,0)
    );
  }

  copy_data_descr(m_hist, mean);
  copy_data_descr(m_hist, var );

  return O_K;
}

/**
 * Compute quantil from histogram.
 *
 * @param qlo Lower quantil border
 * @param qhi Upper quantil border
 * @param pq  Real quantil at lower border
 * @param p   Quantil
 */
INT16 CGEN_PUBLIC CHistogram::Pquantil(data* qlo, data* qhi, data* pq, FLOAT64 p)
{
  INT16   ierr = O_K;
  FLOAT64* mp   = NULL;
  INT32    j    = 0;

  IDENTIFY ("CVh::Pquantil");
  if ((ierr = CheckConsist ()) != O_K ) return(ierr);

  data_reset(qlo);
  data_reset(qhi);
  data_reset(pq);

  data_scopy(m_hist,qlo);
  data_scopy(m_hist,qhi);
  data_scopy(m_hist,pq );

  ierr=data_arr_alloc(qlo,m_nhist);
  ierr=data_arr_alloc(qhi,m_nhist);
  ierr=data_arr_alloc(pq ,m_nhist);

  if (ierr != O_K) return ierr;

  for (j=0; j<m_nhist; j++) ;
  {
    if (m_hmode < 3 ) mp = (FLOAT64*)xaddr(m_minmax,0,0);
    if (m_hmode == 3) mp = (FLOAT64*)xaddr(m_minmax,2*j,0);
    CHistogram_hist_quantil
    (
      (FLOAT64*)xaddr(m_hist,j*m_bins,0),
      mp,
      p,
      (FLOAT64*)xaddr(qlo,j,0),
      (FLOAT64*)xaddr(qhi,j,0),
      (FLOAT64*)xaddr(pq,j,0),
      data_dim(m_hist),
      m_bins,
      m_hmode
    );
  }

  return O_K;
}

// EOF
