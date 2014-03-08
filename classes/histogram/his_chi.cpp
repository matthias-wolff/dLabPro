// dLabPro class CHistogram (histogram)
// - chi-qu-statistic, histogram comparison
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
static FLOAT64 CHistogram_chi_sum
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
 * Chi-quadrat-statistic for two histograms.
 *
 * @param hdat1 histogram data1 
 * @param hdat2 histogram data1 
 * @param c1    component in hdat1
 * @param c2    component in hdat2
 * @param dim1  dim. of hdat1
 * @param dim2  dim. of hdat2
 * @param steps histogram steps
 * @param return chiqu
 */
static FLOAT64 CHistogram_chiqu_1
(
  FLOAT64* hdat1,
  FLOAT64* hdat2,
  INT32    ic1,
  INT32    ic2,
  INT32    dim1,
  INT32    dim2,
  INT32    steps
)
{
  INT32   k;
  FLOAT64 sum1, sum2, c1, c2, a, h1, h2;
  FLOAT64 chiqu = 0.;

  sum1 = CHistogram_chi_sum(hdat1, dim1, steps, ic1);
  sum2 = CHistogram_chi_sum(hdat2, dim2, steps, ic2);

  c1 = sqrt (sum1/sum2);
  c2 = sqrt (sum2/sum1);

  for (k=0; k < steps; k++)
  {
    h1 = hdat1[k*dim1+ic1];
    h2 = hdat2[k*dim2+ic2];
    a  = c1 * h1 - c2 * h2;
    chiqu += ( a * a )/(h1 + h2);
    }

  return chiqu;
}

/**
 * Compute chi-quadrat-statistic for two histograms.
 *
 * Remarks: The histograms must have the same resolution and interval borders.
 *
 * @param h1  histogram 1
 * @param h2  histogram 2
 * @param chi chi-quadrat-statistic
 * @return O_K if ok, NOT_EXEC if not executed
 */
INT16 CGEN_PUBLIC CHistogram::Chiqu(CHistogram* h1, CHistogram* h2, CData* chi)
{
  INT16 ierr;
  INT32 dim, dim1, dim2, steps;
  INT32 i, j, n, n1, n2;
  FLOAT64 *hp1, *hp2, *cp;
  data *hist1, *hist2;

  if ((ierr = h1->CheckHisto()) != O_K)
    return (ierr);
  if ((ierr = h2->CheckHisto()) != O_K)
    return (ierr);

  hist1 = h1->m_hist;
  hist2 = h2->m_hist;
  steps = (INT32) data_descr (hist1, DESCR2);
  if (steps != data_descr (hist1, DESCR2))
    return IERROR(this,HIS_INCONS,0,0,0);

  if (data_descr (hist1, DESCR1) != data_descr (hist1, DESCR1))
    return IERROR(this,HIS_INCONS,0,0,0);

  dim1 = data_dim (hist1);
  dim2 = data_dim (hist2);
  dim = dim1;
  if (dim2 < dim)
    dim = dim2;

  n1 = data_nblock(hist1);
  n2 = data_nblock(hist2);
  n = n1;
  if (n2 < n)
    n = n2;

  data_reset (chi);
  comp_mdef (chi, dim, T_DOUBLE);
  data_arr_alloc (chi, n);

  cp = (FLOAT64*) data_ptr (chi);
  hp1 = (FLOAT64*) data_ptr (hist1);
  hp2 = (FLOAT64*) data_ptr (hist2);

  for (i = 0; i < n; i++)
    for (j = 0; j < dim; j++)
      cp[i * dim + j] = CHistogram_chiqu_1(&hp1[i * dim1 * steps],
          &hp2[i * dim2 * steps], j, j, dim1, dim2, steps);

  copy_data_descr (hist1, chi);

  return (ierr);
}

/**
 * Compute chi-quadrat-statistic for one histogram set. Only histograms of the
 * same component ic are compared.
 *
 * Remarks: The histograms must have the same resolution and interval borders.
 *
 * @param hist histogram data
 * @param chi  chi-quadrat-statistic
 * @param ic   component index
 * @return O_K if ok, NOT_EXEC if not executed
 */
INT16 CGEN_PUBLIC CHistogram::ChiquComp(CData* chi, INT32 ic)
{
  INT16 ierr, steps;
  INT32 dim;
  INT32 i, j, k, n;
  FLOAT64 *hp, *cp;

  if ((ierr = CheckHisto()) != O_K)
    return (ierr);

  steps = HIS_STEPS;
  dim = data_dim (m_hist);
  n = data_nblock(m_hist);

  if (ic > dim - 1)
    return (NOT_EXEC);

  data_reset (chi);
  comp_mdef (chi, n, T_DOUBLE);
  data_arr_alloc (chi, n);

  cp = (FLOAT64*) data_ptr (chi);
  hp = (FLOAT64*) data_ptr (m_hist);

  if (ic > -1)
  {
    for (i = 0; i < n; i++)
      for (j = 0; j < n; j++)
        cp[i * n + j] = CHistogram_chiqu_1(&hp[i * dim * steps],
            &hp[j * dim * steps], ic, ic, dim, dim, steps);
  }

  /* Compute mean chi-quadrat */
  if (ic < 0)
  {
    for (i = 0; i < n; i++)
      for (j = 0; j < n; j++)
        cp[i * n + j] = 0;

    for (k = 0; k < dim; k++)
      for (i = 0; i < n; i++)
        for (j = 0; j < n; j++)
          cp[i * n + j] += CHistogram_chiqu_1(&hp[i * dim * steps],
              &hp[j * dim * steps], k, k, dim, dim, steps);

    for (i = 0; i < n; i++)
      for (j = 0; j < n; j++)
        cp[i * n + j] /= dim;

  }

  copy_data_descr (m_hist, chi);

  return (ierr);
}

// EOF
