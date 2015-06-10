/* dLabPro mathematics library
 * - Generalized cepstrum analysis methods
 *
 * AUTHOR : Guntram Strecha
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
 * <p id=dlm_gmult>Gamma multiply.</p>
 *
 * <p>This function multiplies the Generalized Cepstrum coefficients by gamma, except the zero<sup>th</sup>
 * coefficient.</p>
 *
 * @param gc_in  Pointer to input buffer.
 * @param gc_out Pointer to output buffer.
 * @param n      Number of coefficients, i.e. length of in/out buffer.
 * @param gamma  Generalized cepstrum factor.
 * @return <code>O_K</code>
 */
INT16 dlm_gmult(FLOAT64* gc_in, FLOAT64* gc_out, INT32 n, FLOAT64 gamma) {
  INT32 i = 0;
  for (i = n-1; i > 0; i--)
    gc_out[i] = gc_in[i] * gamma;
  gc_out[0] = gc_in[0];
  return O_K;
}

/**
 * <p id=dlm_gmult>Gamma multiply.</p>
 *
 * <p>This function divides the Generalized Cepstrum coefficients by gamma, except the zero<sup>th</sup>
 * coefficient.</p>
 *
 * @param gc_in  Pointer to input buffer.
 * @param gc_out Pointer to output buffer.
 * @param n      Number of coefficients, i.e. length of in/out buffer.
 * @param gamma  Generalized cepstrum factor.
 * @return <code>O_K</code>
 */
INT16 dlm_igmult(FLOAT64* gc_in, FLOAT64* gc_out, INT32 n, FLOAT64 gamma) {
  return dlm_gmult(gc_in, gc_out, n, 1.0/gamma);
}

/**
 * <p id=dlm_gnorm>Gain normalization.</p>
 *
 * <p>This function normalizes Generalized cepstrum coefficients using the gain given in the zero<sup>th</sup>
 * coefficient.</p>
 *
 * @param gc_in  Pointer to input buffer.
 * @param gc_out Pointer to output buffer.
 * @param n      Number of coefficients, i.e. length of in/out buffer.
 * @param gamma  Generalized cepstrum factor.
 * @return <code>O_K</code>
 */
INT16 dlm_gnorm(FLOAT64* gc_in, FLOAT64* gc_out, INT32 n, FLOAT64 gamma) {
  extern void gnorm(FLOAT64*,FLOAT64*,INT32,const FLOAT64);

  gnorm(gc_in, gc_out, n, gamma);
  return O_K;
}

/**
 * <p id=dlm_ignorm>Inverse gain normalization.</p>
 *
 * <p>This function denormalizes the Generalized Cepstrum coefficients using the gain given in the zero<sup>th</sup>
 * coefficient. This function is the inverse of {@link dlm_gnorm}.</p>
 *
 * @param gc_in  Pointer to input buffer.
 * @param gc_out Pointer to output buffer.
 * @param n      Number of coefficients, i.e. length of in/out buffer.
 * @param gamma  Generalized cepstrum factor.
 * @return <code>O_K</code>
 */
INT16 dlm_ignorm(FLOAT64* gc_in, FLOAT64* gc_out, INT32 n, FLOAT64 gamma) {
  extern void ignorm(FLOAT64*,FLOAT64*,INT32,const FLOAT64);

  ignorm(gc_in, gc_out, n, gamma);
  return O_K;
}

INT16 dlm_gc2gc(FLOAT64* gc_in, INT32 n_in, FLOAT64 gamma_in, FLOAT64* gc_out, INT32 n_out, FLOAT64 gamma_out) {
  extern void gc2gc(FLOAT64*,const INT32,const FLOAT64,FLOAT64*,const INT32,const FLOAT64);

  gc2gc(gc_in, n_in, gamma_in, gc_out, n_out, gamma_out);
  return O_K;
}

/**
 * <p id=dlm_mgcep2mgcep>Mel-Generalized Cepstral Transform.</p>
 *
 * <p>This function transforms the Mel-Generalized Cepstrum with warping factor &lambda;<sub>1</sub> and &gamma;<sub>1</sub>
 * to Mel-Generalized Cepstrum with warping factor &lambda;<sub>2</sub> and &gamma;<sub>2</sub>.</p>
 * <p>The Mel-Generalized Cepstrum is assumed to be in normalized form.
 *
 * @param mgc_in     Pointer to input buffer.
 * @param n_in       Number of coefficients, i.e. length of mgc_in buffer.*
 * @param gamma_in   Generalized cepstrum factor of mgc_in.
 * @param lambda_in  Warping factor of mgc_in.
 * @param mgc_out    Pointer to output buffer.
 * @param n_out      Number of coefficients, i.e. length of mgc_out buffer.
 * @param gamma_out  Generalized cepstrum factor of mgc_out.
 * @param lambda_out Warping factor of mgc_out.
 * @return <code>O_K</code>
 */
INT16 dlm_mgcep2mgcep(FLOAT64* mgc_in, INT32 n_in, FLOAT64 gamma_in, FLOAT64 lambda_in, FLOAT64* mgc_out, INT32 n_out, FLOAT64 gamma_out, FLOAT64 lambda_out) {
  FLOAT64 lambda = (lambda_out - lambda_in) / (1 - lambda_in * lambda_out);

  dlm_gnorm(mgc_in, mgc_in, n_in, gamma_in);
  if (lambda == 0) {
     dlm_gc2gc(mgc_in, n_in, gamma_in, mgc_out, n_out, gamma_out);
  } else {
     dlm_freqt(mgc_in, n_in, mgc_out, n_out, lambda);
     dlm_gc2gc(mgc_out, n_out, gamma_in, mgc_out, n_out, gamma_out);
  }
  dlm_ignorm(mgc_out, mgc_out, n_out, gamma_out);
  return O_K;
}

/**
 * <p id=dlm_gcep2gcep>Generalized Cepstral Transform.</p>
 *
 * <p>This function transforms the Generalized Cepstrum with &gamma;<sub>1</sub>
 * to Generalized Cepstrum with &gamma;<sub>2</sub>.</p>
 * <p>The Generalized Cepstrum is assumed to be in normalized form.
 *
 * @param mgc_in     Pointer to input buffer.
 * @param n_in       Number of coefficients, i.e. length of mgc_in buffer.*
 * @param gamma_in   Generalized cepstrum factor of mgc_in.
 * @param mgc_out    Pointer to output buffer.
 * @param n_out      Number of coefficients, i.e. length of mgc_out buffer.
 * @param gamma_out  Generalized cepstrum factor of mgc_out.
 * @return <code>ERR_MEM</code> on memory allocation failure, <code>O_K</code> otherwise.
 */
INT16 dlm_gcep2gcep(FLOAT64* gc_in, INT32 n_in, FLOAT64 gamma_in, FLOAT64* gc_out, INT32 n_out, FLOAT64 gamma_out) {
  return dlm_mgcep2mgcep(gc_in, n_in, gamma_in, 0.0, gc_out, n_out, gamma_out, 0.0);
}

