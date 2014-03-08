/* dLabPro mathematics library
 * - Statistics
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
 * Returns the Student's t-density with k degrees of freedom of x
 *
 * @param x the x value
 * @param k the degrees of freedom
 */
FLOAT64 dlm_studt(FLOAT64 x, FLOAT64 k)
{
  #ifdef __TMS
  return 0.;
  #else
  if (k>50) return 1./sqrt(2.*F_PI)*exp(-0.5*x*x);
  return tgamma((k+1.)/2.)/sqrt(k*F_PI)/tgamma(k/2.)*pow(1.+x*x/k,-1.*(k+1.)/2.);
  #endif
}

/* EOF */
