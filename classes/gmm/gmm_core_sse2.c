/* dLabPro class CGmm (gmm)
 * - SSE2 optimzed computation core
 *
 * AUTHOR : Rainer Schaffer, Matthias Wolff
 * PACKAGE: dLabPro/classes
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

#if !(defined __GNUC__ && __i386)                                               /* Test for valid target platform    */
#error GMM_SSE2 not supported for this platform. Disable SSE2 in gmm.def.       /* ASSEMBLER INCOMPATIBLE!           */
#endif                                                                          /*                                   */

#ifdef __GNUC__
  #undef WIN32
#endif
#include "swp-sse2.h"                                                           /* Include SSE2 instruction set      */

#define delta ((float*)_this->m_lpDelta)                                        /* Delta vector 1)                   */
                                                                                /* 1) see release notes for details  */                                                                                
/* NO JAVADOC
 * Calculates the Mahalanobis distance or the (logarithmic) probability density
 * of a feature vector given a single Gaussian distribution. There are NO CHECKS
 * performed.
 * 
 * Complexity: 2N + N*N where N=m_nDim
 * 
 * @author Rainer Schaffer
 * @param _this
 *          Pointer to GMM instance
 * @param x
 *          Feature vector (expected to have N=m_nDim values
 * @param k
 *          Index of single Gaussian
 * @param nMode
 *          Operation mode, one of the GMMG_XXX constants
 * @return  The (logarithmic) probability density or Mahalanobis distance of x
 *          in single Gaussian k.
 */
float CGmm_GaussF_SSE2(CGmm* _this, float* x, INT32 k, INT16 nMode)
{
  INT32           dim;                                                           /* Feature space dimensionality      */
  float*         icov;                                                          /* Ptr. to inverse covariance matrix */
  float*         h;                                                             /* Ptr. to aux. buffer (2*dim floats)*/
  float*         m;                                                             /* Ptr. to mean vector               */
  short           i = 0, j = 0;                                                  /* (Rainer's variables)              */
  volatile float tsum[4];                                                       /* (Rainer's variables)              */
  register float *covd0, *covd1, *covd2, *covd3;                                /* (Rainer's variables)              */

  dim  = _this->m_nN;                                                           /* Initialize Rainer's varibles      */
  m    = &((float*)CData_XAddr(_this->m_idMean,0,0))[k*dim];                    /* Initialize Rainer's varibles      */
  icov = &((float*)CData_XAddr(_this->m_idSse2Icov,0,0))[k*dim*dim];            /* Initialize Rainer's varibles      */
  h    = (float*)_this->m_lpSse2Buf;                                            /* Initialize Rainer's varibles      */
  if (*(float*)CData_XAddr(_this->m_idCdet,k,0)==0.)                            /* Covariance matrix invalid         */
    return (float)CGmm_GetLimit(_this,nMode);                                   /*   Return limit                    */

  /* ---- Rainer's code ----> */
  if( m == NULL ) 
    return(.0);

  /* Bestimmung der ersten Werte der Matrix-Vektor-Multiplikation (i = 0) */
  covd0 = &icov[0];
  covd1 = &icov[dim];
  covd2 = &icov[2*dim];
  covd3 = &icov[3*dim];
  xorps_r2r( xmm3, xmm3 );  /* Ergebnisvektor xmm3 (Vekt-Vekt-Mult) l�schen */
  movaps_r2r( xmm3, xmm4 ); /* Ergebnisvektor xmm6 (Matr-Vekt-Mult) l�schen */
  movaps_r2r( xmm3, xmm5 ); /* Ergebnisvektor xmm6 (Matr-Vekt-Mult) l�schen */
  movaps_r2r( xmm3, xmm6 ); /* Ergebnisvektor xmm6 (Matr-Vekt-Mult) l�schen */
  movaps_r2r( xmm3, xmm7 ); /* Ergebnisvektor xmm7 (Matr-Vekt-Mult) l�schen */
  for( j = 0; j < dim-3; j += 4 ){ 
    /* Differenz zweier Vektoren (m-x) */
    movups_m2r( *(m+j), xmm0 );
    movups_m2r( *(x+j), xmm1 );
    subps_r2r( xmm1, xmm0 );
    movups_r2m( xmm0, *(h+j) );
    /* Bestimmung der Elemente */
    movups_m2r( *(covd0+j), xmm1 );
    mulps_r2r( xmm0, xmm1 );
    addps_r2r( xmm1, xmm4 ); /* xmm4 = a3, a2, a1, a0 */
    movups_m2r( *(covd1+j), xmm2 );
    mulps_r2r( xmm0, xmm2 );
    addps_r2r( xmm2, xmm5 ); /* xmm5 = b3, b2, b1, b0 */
    movups_m2r( *(covd2+j), xmm1 );
    mulps_r2r( xmm0, xmm1 );
    addps_r2r( xmm1, xmm6 ); /* xmm6 = c3, c2, c1, c0 */
    movups_m2r( *(covd3+j), xmm2 );
    mulps_r2r( xmm0, xmm2 );
    addps_r2r( xmm2, xmm7 ); /* xmm7 = d3, d2, d1, d0 */
  }
  movaps_r2r( xmm4, xmm0 );   /* xmm0 = a3 , a2 , a1 , a0  */
  movlhps_r2r( xmm6, xmm4 );  /* xmm4 = c1 , c0 , a1 , a0  */
  movhlps_r2r( xmm0, xmm6 );  /* xmm6 = c3 , c2 , a3 , a2  */
  addps_r2r( xmm4, xmm6 );    /* xmm6 = c1', c0', a1', a0' */
  movaps_r2r( xmm5, xmm0 );   /* xmm0 = b3 , b2 , b1 , b0  */
  movlhps_r2r( xmm7, xmm5 );  /* xmm5 = d1 , d0 , b1 , b0  */
  movhlps_r2r( xmm0, xmm7 );  /* xmm7 = d3 , d2 , b3 , b2  */
  addps_r2r( xmm5, xmm7 );    /* xmm7 = d1', d0', b1', b0' */
  movaps_r2r( xmm6, xmm1 );   /* xmm1 = c1', c0', a1', a0' */
  unpckhps_r2r( xmm7, xmm6 ); /* xmm6 = d1', c1', d0', c0' */
  unpcklps_r2r( xmm7, xmm1 ); /* xmm1 = b1', a1', b0', a0' */
  movaps_r2r( xmm1, xmm0 );   /* xmm0 = b1', a1', b0', a0' */
  movlhps_r2r( xmm6, xmm1 );  /* xmm1 = d0', c0', b0', a0' */
  movhlps_r2r( xmm0, xmm6 );  /* xmm6 = d1', c1', b1', a1' */
  addps_r2r( xmm1, xmm6 );    /* xmm6 = d* , c* , b* , a*  */
  for( ; j < dim; ++j ){ /* Durchf�hrung nichtparalleler Befehle  */
    h[j] = m[j] - x[j]; 
    tsum[0] = covd0[j]*h[j];
    tsum[1] = covd1[j]*h[j];
    tsum[2] = covd2[j]*h[j];
    tsum[3] = covd3[j]*h[j];
    movups_m2r( *tsum, xmm0 );
    addps_r2r( xmm0, xmm6 );
  } 
  /* Vektor-Vektor-Multiplikation (1. Teil) */
  movups_m2r( *(h), xmm0 );
  mulps_r2r( xmm6, xmm0 );
  addps_r2r( xmm0, xmm3 );

  /* Bestimmung der weiteren Werte der Matrix-Vektor-Multiplikation (i > 0) */
  for( i = 4; i < dim-3; i += 4 ){
    covd0 = &icov[i*dim];
    covd1 = &icov[(i+1)*dim];
    covd2 = &icov[(i+2)*dim];
    covd3 = &icov[(i+3)*dim];
    xorps_r2r( xmm4, xmm4 ); /* Ergebnisvektoren xmm4 bis xmm7 l�schen */
    movaps_r2r( xmm4, xmm5 );
    movaps_r2r( xmm4, xmm6 );
    movaps_r2r( xmm4, xmm7 );
    for( j = 0; j < dim-3; j += 4 ){ 
      /* sum += covd[j] * h[j]; */
      /* Lesen der Differenz zweier Vektoren (m-x) */
      movups_m2r( *(h+j), xmm0 );
      /* Bestimmung der Elemente */
      movups_m2r( *(covd0+j), xmm1 );
      mulps_r2r( xmm0, xmm1 );
      addps_r2r( xmm1, xmm4 ); /* xmm4 = a3, a2, a1, a0 */
      movups_m2r( *(covd1+j), xmm2 );
      mulps_r2r( xmm0, xmm2 );
      addps_r2r( xmm2, xmm5 ); /* xmm5 = b3, b2, b1, b0 */
      movups_m2r( *(covd2+j), xmm1 );
      mulps_r2r( xmm0, xmm1 );
      addps_r2r( xmm1, xmm6 ); /* xmm6 = c3, c2, c1, c0 */
      movups_m2r( *(covd3+j), xmm2 );
      mulps_r2r( xmm0, xmm2 );
      addps_r2r( xmm2, xmm7 ); /* xmm7 = d3, d2, d1, d0 */
    }
    movaps_r2r( xmm4, xmm0 );   /* xmm0 = a3 , a2 , a1 , a0  */
    movlhps_r2r( xmm6, xmm4 );  /* xmm4 = c1 , c0 , a1 , a0  */
    movhlps_r2r( xmm0, xmm6 );  /* xmm6 = c3 , c2 , a3 , a2  */
    addps_r2r( xmm4, xmm6 );    /* xmm6 = c1', c0', a1', a0' */
    movaps_r2r( xmm5, xmm0 );   /* xmm0 = b3 , b2 , b1 , b0  */
    movlhps_r2r( xmm7, xmm5 );  /* xmm5 = d1 , d0 , b1 , b0  */
    movhlps_r2r( xmm0, xmm7 );  /* xmm7 = d3 , d2 , b3 , b2  */
    addps_r2r( xmm5, xmm7 );    /* xmm7 = d1', d0', b1', b0' */
    movaps_r2r( xmm6, xmm1 );   /* xmm1 = c1', c0', a1', a0' */
    unpckhps_r2r( xmm7, xmm6 ); /* xmm6 = d1', c1', d0', c0' */
    unpcklps_r2r( xmm7, xmm1 ); /* xmm1 = b1', a1', b0', a0' */
    movaps_r2r( xmm1, xmm0 );   /* xmm0 = b1', a1', b0', a0' */
    movlhps_r2r( xmm6, xmm1 );  /* xmm1 = d0', c0', b0', a0' */
    movhlps_r2r( xmm0, xmm6 );  /* xmm6 = d1', c1', b1', a1' */
    addps_r2r( xmm1, xmm6 );    /* xmm6 = d* , c* , b* , a*  */
    for( ; j < dim; ++j ){ /* Durchf�hrung nichtparalleler Befehle  */
      h[j] = m[j] - x[j]; 
      tsum[0] = covd0[j]*h[j];
      tsum[1] = covd1[j]*h[j];
      tsum[2] = covd2[j]*h[j];
      tsum[3] = covd3[j]*h[j];
      movups_m2r( *tsum, xmm0 );
      addps_r2r( xmm0, xmm6 );
    } 
    /* Vektor-Vektor-Multiplikation */
    movups_m2r( *(h+i), xmm0 );
    mulps_r2r( xmm6, xmm0 );
    addps_r2r( xmm0, xmm3 );
  }

  /* Durchf�hrung nichtparalleler Befehle */
  tsum[0] = 0.0;
  for( ; i < dim; ++i ){
    tsum[1] = 0.0;
    covd0 = &icov[i*dim];
    for( j = 0; j < dim; ++j ){
      tsum[1] += covd0[j]*h[j];      
    }
    tsum[0] += h[i]*tsum[1];
  }

  /* Quersumme vom Ergebnisvektor mm3 */
  movhlps_r2r( xmm3, xmm0 );
  addps_r2r( xmm0, xmm3 );
  unpcklps_r2r( xmm3, xmm3 );
  movhlps_r2r( xmm3, xmm0 );
  addps_r2r( xmm0, xmm3 );  
  if( dim%4 != 0 ){ /* Durchf�hrung nichtparalleler Befehle */
    movups_m2r( *tsum, xmm0 );
    addps_r2r( xmm0, xmm3 );
  }
  movups_r2m( xmm3, *tsum );
  /* <---- Rainer's code ---- */

  if (tsum[0]<0.0) tsum[0]=0.0;                                                 /* Distance must be non-negative     */
  switch (nMode)                                                                /* Branch by operation mode          */
  {                                                                             /* >>                                */
    case GMMG_MDIST : return tsum[0];                                           /*   Mahalanobis distance            */
    case GMMG_LDENS : return delta[k] - 0.5*tsum[0];                            /*   Logarithmic probability density */
    case GMMG_NLDENS: return -(delta[k] - 0.5*tsum[0]);                         /*   Negative log. prob. density     */
    case GMMG_DENS  : return exp(delta[k] - 0.5*tsum[0]);                       /*   Probability density             */
  }                                                                             /* <<                                */
  DLPASSERT(FMSG("Unknown Gauss mode"));                                        /* Invalid value of nMode!           */
  return 0.;                                                                    /* Emergency exit                    */
}

#undef delta                                                                    /* Undefine delta                    */
#define delta ((double*)_this->m_lpDelta)                                       /* Delta vector 1)                   */
                                                                                /* 1) see release notes for details  */                                                                                
/* NO JAVADOC
 * Calculates the Mahalanobis distance or the (logarithmic) probability density
 * of a feature vector given a single Gaussian distribution. There are NO CHECKS
 * performed.
 * 
 * Complexity: 2N + N*N where N=m_nDim
 * 
 * @author Rainer Schaffer
 * @param _this
 *          Pointer to GMM instance
 * @param x
 *          Feature vector (expected to have N=m_nDim values
 * @param k
 *          Index of single Gaussian
 * @param nMode
 *          Operation mode, one of the GMMG_XXX constants
 * @return  The (logarithmic) probability density or Mahalanobis distance of x
 *          in single Gaussian k.
 */
double CGmm_GaussD_SSE2(CGmm* _this, double* x, INT32 k, INT16 nMode)
{
  INT32            dim;                                                          /* Feature space dimensionality      */
  double*         icov;                                                         /* Ptr. to inverse covariance matrix */
  double*         h;                                                            /* Ptr. to aux.buffer (2*dim doubles)*/
  double*         m;                                                            /* Ptr. to mean vector               */
  short            i = 0, j = 0;                                                 /* (Rainer's variables)              */
  volatile double tsum[2];                                                      /* (Rainer's variables)              */
  register double *covd, *covd2;                                                /* (Rainer's variables)              */

  dim  = _this->m_nN;                                                           /* Initialize Rainer's varibles      */
  m    = &((double*)CData_XAddr(_this->m_idMean,0,0))[k*dim];                   /* Initialize Rainer's varibles      */
  icov = &((double*)CData_XAddr(_this->m_idSse2Icov,0,0))[k*dim*dim];           /* Initialize Rainer's varibles      */
  h    = (double*)_this->m_lpSse2Buf;                                           /* Initialize Rainer's varibles      */
  if (*(double*)CData_XAddr(_this->m_idCdet,k,0)==0.)                           /* Covariance matrix invalid         */
    return CGmm_GetLimit(_this,nMode);                                          /*   Return limit                    */

  /* ---- Rainer's code ----> */
  if( m == NULL ) 
    return(.0);

  /* Bestimmung der ersten Werte der Matrix-Vektor-Multiplikation (i = 0) */
  covd = &icov[0];
  covd2 = &icov[dim];
  xorpd_r2r( xmm3, xmm3 );  /* Ergebnisvektor xmm3 (Vekt-Vekt-Mult) l�schen */
  movapd_r2r( xmm3, xmm6 ); /* Ergebnisvektor xmm6 (Matr-Vekt-Mult) l�schen */
  movapd_r2r( xmm3, xmm7 ); /* Ergebnisvektor xmm7 (Matr-Vekt-Mult) l�schen */
  for( j = 0; j < dim-1; j += 2 ){ 
    /* Differenz zweier Vektoren (m-x) */
    movupd_m2r( *(m+j), xmm0 );
    movupd_m2r( *(x+j), xmm1 );
    subpd_r2r( xmm1, xmm0 );
    movupd_r2m( xmm0, *(h+j) );
    /* Bestimmung der Elemente */
    movupd_m2r( *(covd+j), xmm1 );
    movupd_m2r( *(covd2+j), xmm2 );
    mulpd_r2r( xmm0, xmm1 );
    mulpd_r2r( xmm0, xmm2 );
    addpd_r2r( xmm1, xmm6 );
    addpd_r2r( xmm2, xmm7 );
  }
  movapd_r2r( xmm6, xmm0 );
  movlhps_r2r( xmm7, xmm6 );
  movhlps_r2r( xmm0, xmm7 );
  addpd_r2r( xmm6, xmm7 );
  for( ; j < dim; ++j ){ /* Durchf�hrung nichtparalleler Befehle */
    h[j] = m[j] - x[j]; 
    tsum[0] = covd[j]*h[j];
    tsum[1] = covd2[j]*h[j];
    movupd_m2r( *tsum, xmm0 );
    addpd_r2r( xmm0, xmm7 );
  } 
  /* Vektor-Vektor-Multiplikation (1. Teil) */
  movupd_m2r( *(h), xmm0 );
  mulpd_r2r( xmm7, xmm0 );
  addpd_r2r( xmm0, xmm3 );
  

  /* Bestimmung der weiteren Werte der Matrix-Vektor-Multiplikation (i > 0) */
  for( i = 2; i < dim-1; i += 2 ){
    covd = &icov[i*dim];
    covd2 = &icov[i*dim+dim];
    xorpd_r2r( xmm6, xmm6 ); /* Ergebnisvektoren xmm6 und xmm7 l�schen */
    movapd_r2r( xmm6, xmm7 );
    for( j = 0; j < dim-1; j += 2 ){ 
      /* Lesen der Differenz zweier Vektoren (m-x) */
      movupd_m2r( *(h+j), xmm0 );
      /* Bestimmung der Elemente */
      movupd_m2r( *(covd+j), xmm1 );
      movupd_m2r( *(covd2+j), xmm2 );
      mulpd_r2r( xmm0, xmm1 );
      mulpd_r2r( xmm0, xmm2 );
      addpd_r2r( xmm1, xmm6 );
      addpd_r2r( xmm2, xmm7 );
    }
    movapd_r2r( xmm6, xmm0 );
    movlhps_r2r( xmm7, xmm6 );
    movhlps_r2r( xmm0, xmm7 );
    addpd_r2r( xmm6, xmm7 );
    for( ; j < dim; ++j ){ /* Durchf�hrung nichtparalleler Befehle */
      tsum[0] = covd[j]*h[j];
      tsum[1] = covd2[j]*h[j];
      movupd_m2r( *tsum, xmm0 );
      addpd_r2r( xmm0, xmm7 );
    } 
    /* Vektor-Vektor-Multiplikation */
    movupd_m2r( *(h+i), xmm0 );
    mulpd_r2r( xmm7, xmm0 );
    addpd_r2r( xmm0, xmm3 );
  }
  /* Durchf�hrung nichtparalleler Befehle */
  tsum[0] = 0.0;
  for( ; i < dim; ++i ){
    tsum[1] = 0.0;
    covd = &icov[i*dim];
    for( j = 0; j < dim; ++j ){
      tsum[1] += covd[j]*h[j];      
    }
    tsum[0] += h[i]*tsum[1];
  }
  /* Quersumme vom Ergebnisvektor mm3 */
  movhlps_r2r( xmm3, xmm0 );
  addpd_r2r( xmm0, xmm3 );
  if( dim%2 != 0 ){ /* Durchf�hrung nichtparalleler Befehle */
    movupd_m2r( *tsum, xmm0 );
    addpd_r2r( xmm0, xmm3 );
  }
  movupd_r2m( xmm3, *tsum );
  /* <---- Rainer's code ---- */

  if (tsum[0]<0.0) tsum[0]=0.0;                                                 /* Distance must be non-negative     */
  switch (nMode)                                                                /* Branch by operation mode          */
  {                                                                             /* >>                                */
    case GMMG_MDIST : return tsum[0];                                           /*   Mahalanobis distance            */
    case GMMG_LDENS : return delta[k] - 0.5*tsum[0];                            /*   Logarithmic probability density */
    case GMMG_NLDENS: return -(delta[k] - 0.5*tsum[0]);                         /*   Negative log. prob. density     */
    case GMMG_DENS  : return exp(delta[k] - 0.5*tsum[0]);                       /*   Probability density             */
  }                                                                             /* <<                                */
  DLPASSERT(FMSG("Unknown Gauss mode"));                                        /* Invalid value of nMode!           */
  return 0.;                                                                    /* Emergency exit                    */
}

#undef delta

/* EOF */
