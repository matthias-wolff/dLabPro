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

#ifdef NUMINT_INTERVALS
#undef NUMINT_INTERVALS
#endif
#define NUMINT_INTERVALS 1000000

/**
 * Student's t-density with k degrees of freedom.
 *
 * @param x
 *          The x value
 * @param k
 *          The degrees of freedom
 * @return The value of the Student's t-density of x.
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

/**
 * The Gamma function.
 *
 * @param x
 *          The x value.
 * @return The value of the Gamma function of x.
 */
FLOAT64 dlm_gamma(FLOAT64 x)
{
  /*TODO: Implement complex gamma function */
  #ifdef __TMS
  return 0.;
  #else
  return tgamma(x);
  #endif
}

/**
 * Natural logarithm of the Gamma function.
 *
 * @param x
 *          The x value, must be positive.
 * @return The value of the log-Gamma function of x.
 */
FLOAT64 dlm_lgamma(FLOAT64 x)
{
  #ifdef __TMS
  return 0.;
  #else
  return lgamma(x);
  #endif
}

/**
 * Euler's Beta function.
 *
 * @param alpha
 *          The alpha parameter.
 * @param beta
 *          The beta parameter.
 * @return The value of the Beta function of alpha and beta.
 */
FLOAT64 dlm_beta(FLOAT64 alpha, FLOAT64 beta)
{
  #ifdef __TMS
  return 0.;
  #else
  return exp(lgamma(alpha)+lgamma(beta)-lgamma(alpha+beta));
  #endif
}

/**
 * Beta density with the form parameters alpha and beta.
 *
 * @param x
 *          The x value, 0&le;P&le;1.
 * @param alpha
 *          The alpha parameter, must be non-negative.
 * @param beta
 *          The beta parameter, must be non-negative.
 * @return The value of the Beta density of x.
 */
FLOAT64 dlm_betadens(FLOAT64 x, FLOAT64 alpha, FLOAT64 beta)
{
  #ifdef __TMS
  return 0.;
  #else
  FLOAT64 v;
  if (x<0 || x>1) return 0;
  v = exp(lgamma(alpha+beta) - lgamma(alpha) - lgamma(beta) +
      (alpha-1)*log(x) + (beta-1)*log(1-x));
  /*printf("\n*** dlp_betadens(%g,%g,%g)=%g",x,alpha,beta,v);*/
  if (dlp_isnan(v)) return 0;
  return v;
  #endif
}

/**
 * P-quantile of the Beta cumulative distribution function with the form
 * parameters alpha and beta. The quantile is the inverse of the Beta CDF.
 *
 * @param P
 *          The quantile probability, 0&le;P&le;1.
 * @param alpha
 *          The alpha parameter, must be non-negative.
 * @param beta
 *          The beta parameter, must be non-negative.
 * @return The value of the Beta CDF of x.
 */
FLOAT64 dlm_betaquant(FLOAT64 P, FLOAT64 alpha, FLOAT64 beta)
{
  #ifdef __TMS
  return 0.;
  #else

  FLOAT64 p;                                                                   /* Beta density value                 */
  FLOAT64 s;                                                                   /* Sum of beta densities              */
  FLOAT64 s0;                                                                  /* Previous sum                       */
  FLOAT64 x;                                                                   /* x-value                            */
  FLOAT64 x0;                                                                  /* Previous x-value                   */
  FLOAT64 c;                                                                   /* Normalizing constant (1/Beta)      */
  FLOAT64 i;                                                                   /* Interval width                     */
  FLOAT64 N;                                                                   /* Interval counter                   */

  if (P<=0) return 0;                                                          /* P out of bounds                    */
  if (P>=1) return 1;                                                          /* P out of bounds                    */

  c = lgamma(alpha) + lgamma(beta) - lgamma(alpha+beta);                       /* Initialize normalizing constant    */
  i = 1/((FLOAT64)NUMINT_INTERVALS);                                           /* Initialize interval width          */
  for (s=0, s0=0, x=i/2, x0=i/2, N=0; x<=1; x+=i)                              /* Loop over intervals                */
  {                                                                            /* >>                                 */
    p = exp((alpha-1)*log(x)+(beta-1)*log(1-x)-c);                             /*   Compute Beta density value       */
    if (!dlp_isnan(p))                                                         /*   Density is a number              */
    {                                                                          /*   >>                               */
      s += p*i;                                                                /*     Sum up density intervals       */
      N ++;                                                                    /*     Count intervals                */
    }                                                                          /*   <<                               */
    /*printf("\n*** x=%g, p(x)=%g, F(x)=%g, x0=%g, F(x0)=%g",x,p,s,x0,s0);*/
    if (s>P) break;                                                            /*   Quantile prob. reached -> break  */
    s0 = s; x0 = x;                                                            /*   Remember previous values         */
  }                                                                            /* <<                                 */
  if ((s-s0)==0) return x0;                                                    /* Prevent division by zero           */
  return (P-s0)*(x-x0)/(s-s0)+x0;                                              /* Return linear interpolation        */

  #endif
}

/* EOF */
