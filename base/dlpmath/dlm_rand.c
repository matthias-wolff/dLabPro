/* dLabPro mathematics library
 * - Random functions
 *
 * AUTHOR : Guntram Strecha,
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
 * <p>Normally (Gaussian) distributed random values, using the polar method of George Marsaglia.</p>
 *
 * @return Gaussian distributed random value
 */
FLOAT64 dlm_rand_gauss_polar() {
  static  BOOL    toggle = TRUE;
  static  FLOAT64 x1     = 0.0;
  static  FLOAT64 x2     = 0.0;
  static  FLOAT64 x      = 0.0;

  if(toggle == TRUE) {
    toggle = FALSE;
    do {
      x1 = 2.0*((FLOAT64)dlp_rand()/(FLOAT64)RAND_MAX) - 1.0;
      x2 = 2.0*((FLOAT64)dlp_rand()/(FLOAT64)RAND_MAX) - 1.0;
      x = x1*x1 + x2*x2;
    } while((x >= 1.0) || (x == 0.0));
    return x1 * sqrt(-2.0 * log(x) / x);
  }
  toggle = TRUE;
  return x2 * x;
}

/**
 * <p>A normally distributed random number generator. It seems to be known as
 * the Box-Muller-Method. We avoid the uniform rv's
 * being 0.0 since this will result in infinite values, and double count the
 * 0 == 2pi.</p>
 * <p><b>Author:</b> Jeremy Lea (<a
 * href="http://home.online.no/~pjacklam/notes/invnorm/impl/lea/lea.c">
 * http://home.online.no/~pjacklam/notes/invnorm/impl/lea/lea.c</a>)</p>
 */
FLOAT64 dlm_rand_gauss_bm() {
  static INT32    i    = 1;
  static FLOAT64 u[2] = {0.0, 0.0};
  FLOAT64        r[2];

  if (i == 1) {
    r[0] = sqrt(-2*log((FLOAT64)(dlp_rand()+1)/((FLOAT64)RAND_MAX+1.)));
    r[1] = 2*F_PI*(FLOAT64)(dlp_rand()+1)/((FLOAT64)RAND_MAX+1.);
    u[0] = r[0]*sin(r[1]);
    u[1] = r[0]*cos(r[1]);
    i = 0;
  }
  else i = 1;
  return u[i];
}
