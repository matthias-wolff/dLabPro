/* dLabPro mathematics library
 * - Combinatorics
 *
 * AUTHOR : Matthias Wolff,
 * PACKAGE: dLabPro/base
 * 
 * Copyright 2013 dLabPro contributors and others (see COPYRIGHT file) 
 * - Chair of System Theory and Speech Technology, TU Dresden
 * - Chair of Communications Engineering, BTU Cottbus
 * 
 * This file is part of dLabPro.
 * 
 * dLabPro is free software: you can redistribute it and/or modify it under the
 * terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option)
 * any later version.
 * 
 * dLabPro is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
 * details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with dLabPro. If not, see <http://www.gnu.org/licenses/>.
 */

#include "dlp_kernel.h"
#include "dlp_base.h"
#include "dlp_math.h"

/**
 * Computes n over k.
 *
 * @param n
 *         <i>n</i>
 * @param k
 *         <i>k</i>
 */
 /* Author: Christian-M. Westendorf */
INT64 dlm_n_over_k(INT32 n, INT32 k)
{
  FLOAT64 nn = (FLOAT64)n;
  FLOAT64 kk = 1.;
  FLOAT64 r  = 1.;

  if (k==0) return 1;
  if (k==1) return n;
  if (k==n) return 1;

  do { r = r*nn/kk; nn --; kk ++; } while ((nn>=n-(k-1))&&(kk<=k));
  if (kk<k      ) do { r = r/kk; kk++; } while (kk<=k      );
  if (nn>n-(k-1)) do { r = r*nn; nn--; } while (nn>=n-(k-1));

  return (INT64)r;
}

/* EOF */
