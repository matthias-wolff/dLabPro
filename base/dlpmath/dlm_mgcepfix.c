/* dLabPro mathematics library
 * - Generalized MEL-cepstrum analysis method
 *
 * AUTHOR : Guntram Strecha, Frank Duckhorn
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

/**
 * modified by Arnulf Becker 2015
 * See the bachelor thesis: Festkommaportierung des generalisierten Mel-Cepstrum
 */

#include "dlp_kernel.h"
#include "dlp_base.h"
#include "dlp_math.h"

#include "measurement/measurement.h"

#define INT16_MAX_FLOAT 32767.
#define OLD_FUNCTION_CALLS 0

//const char* measure_name = "measurement_1.csv";
static dataLog logger_1 = {.file_path = "meas_1.csv"};


/* functions from floating-point implementation */
void matinv_float(FLOAT64 *A, INT32 n);
FLOAT64 sum2x_float(FLOAT64* vek1, FLOAT64* vek2, INT32 len);
char lpc_mburg_float(FLOAT64* samples, INT32 n, FLOAT64* a, INT16 p, FLOAT64 lambda, FLOAT64 scale);
void gc2gc_float(FLOAT64 *c, const INT32 m, const FLOAT64 g1, const FLOAT64 g2);
void ignorm_float(FLOAT64 *c, INT32 m, const FLOAT64 g);
void filter_freqt_fir_init_float(INT32 n_in, INT32 n_out, FLOAT64 lambda, FLOAT64 *z,
		FLOAT64 norm);
void filter_freqt_fir_float(FLOAT64* in, INT32 n_in, FLOAT64* out, INT32 n_out,
		FLOAT64 *z);


/* buffers from old floating point implementation - to be replaced incrementally */
FLOAT64 *lpZo, *lpZn;
FLOAT64 *lpSx, *lpSy, *lpGx, *lpGy;
FLOAT64 *lpPsiRx, *lpPsiPx, *lpPsiQx;
FLOAT64 *lpPsiRy, *lpPsiPy, *lpPsiQy;
FLOAT64 *lpH;

/* objects for data and evaluation */


/* Normierungsfaktoren */
#define SIG_NRM	1.
#define RES_NRM	2.


/*---------------------------------------------------------------------------*/

/* Generalized Mel-Cepstral analysis init buffers
 *
 * @param n       Input feature vector dimension
 * @param order   Output feature vector dimension
 * @param lambda  Lambda parameter for analysis
 */
void dlm_mgcepfix_init(INT32 n, INT16 order, INT16 lambda) {
#if OLD_FUNCTION_CALLS
	dlm_mgcep_init(n, order, (FLOAT64) lambda / 32767.);
#else
	/* old floating-point implementation */
	FLOAT64 lambda_float = (FLOAT64) lambda / 32767.;

	INT16 m = order - 1;

	lpZo = (FLOAT64*) dlp_malloc((order - 1) * n * sizeof(FLOAT64));
	lpZn = (FLOAT64*) dlp_malloc((n/2-1)*MIN(n,2*m+1)*sizeof(FLOAT64));
	filter_freqt_fir_init_float(order, n, -lambda_float, lpZo, 1.);
	filter_freqt_fir_init_float(n / 2, MIN(n, 2 * m + 1), lambda_float, lpZn, 2.);

	lpSx = dlp_malloc(n * sizeof(double));
	lpSy = dlp_malloc(n * sizeof(double));
	lpGx = dlp_malloc(n * sizeof(double));
	lpGy = dlp_malloc(n * sizeof(double));

	lpPsiRx = dlp_malloc(n * sizeof(double));
	lpPsiRy = dlp_malloc(n * sizeof(double));
	lpPsiPx = dlp_malloc(n * sizeof(double));
	lpPsiPy = dlp_malloc(n * sizeof(double));
	lpPsiQx = dlp_malloc(n * sizeof(double));
	lpPsiQy = dlp_malloc(n * sizeof(double));

	lpH = (FLOAT64*) dlp_malloc(m * m * sizeof(FLOAT64));

	data_evaluation_init(logger_1);
#endif
}

/* Generalized Mel-Cepstral analysis free buffers */
void dlm_mgcepfix_free() {
#if OLD_FUNCTION_CALLS
	dlm_mgcep_free();
#else
	dlp_free(lpZo);
	dlp_free(lpZn);
	dlp_free(lpSx);
	dlp_free(lpSy);
	dlp_free(lpGx);
	dlp_free(lpGy);
	dlp_free(lpPsiRx);
	dlp_free(lpPsiRy);
	dlp_free(lpPsiPx);
	dlp_free(lpPsiPy);
	dlp_free(lpPsiQx);
	dlp_free(lpPsiQy);
	dlp_free(lpH);

	data_evaluation_free(logger_1);
#endif
}

/* Single vector generalized Mel-Cepstral analysis
 *
 * @param input   Buffer with input features
 * @param n       Input feature vector dimension
 * @param output  Buffer for output features
 * @param order   Output feature vector dimension
 * @param gamma   Gamma parameter for analysis (-1<gamma<0)
 * @param lambda  Lambda parameter for analysis
 */
/* 
 * S   = fft(in)            # 3ms
 * out = gc2gc(mburg(in))   # 93ms + 1ms
 * 
 * while(){
 *  G = fft(filter(out))    # 38ms + 8ms
 *
 *  p = abssqr(S)/abssqr(G)/abssqr(G)^(1/gamma)
 *  R = G * p
 *  Q = G*G * p/abssqr(G)   # 25ms
 *
 *  p = filter(ifft(P))
 *  r = filter(ifft(R))
 *  q = filter(ifft(Q))     # 20ms + 75ms
 *
 *  H = inv(p x q)          # 62ms (p x q: pos semidefinit - empirisch ermittelt)
 *  out = out/gamma + H*r   # 3ms
 * }
 * 
 * out = ignorm(out)        # 0ms
 * 
 */
INT16 dlm_mgcepfix(INT16* input, INT32 n, INT16* output, INT16 order,
		INT16 gamma, INT16 lambda) {
	INT16 ret;
	INT32 i;

	/* variables from floating-point implementation */
	INT16 itr1 = 2;
	INT16 itr2 = 30;
	INT16 m = order - 1;
//	INT32 i;
	INT32 j;
	INT32 k;
	INT16 flag = 0;
	FLOAT64 dd = 0.000001;
	FLOAT64 ep = 0.;

	/* conversion buffers */
	FLOAT64 *in_float = dlp_malloc(n * sizeof(FLOAT64));
	FLOAT64 *out_float = dlp_malloc(order * sizeof(FLOAT64));

	for (i = 0; i < n; i++)
		in_float[i] = (FLOAT64) input[i] / 32767. * SIG_NRM;

	/*-----------------------------------------------------------------------*/

#if OLD_FUNCTION_CALLS
	ret = dlm_mgcep(in, n, out, order, (FLOAT64) gamma / 32767.,
			(FLOAT64) lambda / 32767., 32768.);
#else
	/*-----------------------------------------------------------------------*/
	/* old floating-point implementation */
	/* INT16 dlm_mgcep(FLOAT64* input, INT32 n, FLOAT64* output, INT16 order, FLOAT64 gamma, FLOAT64 lambda, FLOAT64 scale) */

	FLOAT64 gamma_float = (FLOAT64) gamma / 32767.;
	FLOAT64 lambda_float = (FLOAT64) lambda / 32767.;
	FLOAT64 scale = 32768.;

	/* Get input spectrum */
	for (i = n - 1; i >= 0; i--) {
		lpSx[i] = in_float[i];
		lpSy[i] = 0.;
	}

	data_evaluation_FLOAT64(logger_1, "in_float", in_float, n); /* DEBUG */

	dlm_fft(lpSx, lpSy, n, FALSE); /* TODO: input real, output n/2+1 */
	for (i = 0; i <= n / 2; i++)
		lpSx[i] = lpSx[i] * lpSx[i] + lpSy[i] * lpSy[i];

	/* Init coefficients from input signal */
	lpc_mburg_float(in_float, n, out_float, order, lambda_float, scale);
	gc2gc_float(out_float, m, -1, gamma_float);

	/* Improve coefficients iteratively */
	for (j = 0; j < itr2 && !flag; j++) {

		ep = out_float[0];
		out_float[0] = 1.;
		for (i = 1; i < order; i++)
			out_float[i] *= gamma_float;

		/* Mel- + spectral transform of coefficients */
		filter_freqt_fir_float(out_float, order, lpGx, n, lpZo);
		for (i = 0; i < n; i++)
			lpGy[i] = 0.;
		dlm_fft(lpGx, lpGy, n, FALSE); /* TODO: input real, output n/2+1 */

		/* Get temporary psi-signals in spectral domain */
		for (i = 0; i <= n / 2; i++) {
			FLOAT64 a = lpGx[i] * lpGx[i] + lpGy[i] * lpGy[i];
			FLOAT64 tmp1 = lpSx[i] / pow(a, 1. + 1. / gamma_float);
			lpPsiRx[i] = lpGx[i] * tmp1;
			lpPsiRy[i] = lpGy[i] * tmp1;
			lpPsiPx[i] = tmp1;
			lpPsiPy[i] = 0.;
			lpPsiQx[i] = (lpGx[i] * lpGx[i] - lpGy[i] * lpGy[i]) * tmp1 / a;
			lpPsiQy[i] = 2 * lpGx[i] * lpGy[i] * tmp1 / a;
			if (i > 0 && i < n / 2) {
				lpPsiRx[n - i] = lpPsiRx[i];
				lpPsiRy[n - i] = -lpPsiRy[i];
				lpPsiPx[n - i] = lpPsiPx[i];
				lpPsiPy[n - i] = 0.;
				lpPsiQx[n - i] = lpPsiQx[i];
				lpPsiQy[n - i] = -lpPsiQy[i];
			}
		}

		/* Transform psi-signals in time domain */
		dlm_fft(lpPsiRx, lpPsiRy, n, TRUE); /* TODO: input n/2+1, output real n/2 */
		dlm_fft(lpPsiPx, lpPsiPy, n, TRUE); /* TODO: input real n/2+1, output real n/2 */
		dlm_fft(lpPsiQx, lpPsiQy, n, TRUE); /* TODO: input n/2+1, output real n/2 */

		/* Inverse Mel-transform of psi-signals */
		if (lambda_float != 0.0) {
			filter_freqt_fir_float(lpPsiRx, n / 2, lpPsiRy, MIN(n, m + 1), lpZn);
			filter_freqt_fir_float(lpPsiPx, n / 2, lpPsiPy, MIN(n, m), lpZn);
			filter_freqt_fir_float(lpPsiQx, n / 2, lpPsiQy, MIN(n, 2 * m + 1), lpZn);
			for (i = 1; i <= m; i++)
				lpPsiRy[i] *= .5;
			for (i = 1; i < m; i++)
				lpPsiPy[i] *= .5;
			for (i = 1; i <= 2 * m; i++)
				lpPsiQy[i] *= .5 * (1. + gamma_float);
		} else {
			for (i = 1; i <= m; i++)
				lpPsiRy[i] = lpPsiRx[i] / (FLOAT64) n;
			for (i = 1; i < m; i++)
				lpPsiPy[i] = lpPsiPx[i] / (FLOAT64) n;
			for (i = 1; i <= 2 * m; i++)
				lpPsiQy[i] = lpPsiQx[i] * (1. + gamma_float) / (FLOAT64) n;
		}

		/* Combine to H matrix and invert it */
		for (i = 0; i < m; i++)
			for (k = 0; k <= i; k++)
				lpH[i + k * m] = lpPsiPy[abs(k - i)] + lpPsiQy[k + i + 2];
		matinv_float(lpH, m);

		/* Update coefficients from H matrix */
		for (i = 0; i < m; i++) {
			FLOAT64 s = 0.;
			for (k = 0; k <= i; k++)
				s += lpH[i + k * m] * lpPsiRy[k + 1];
			for (; k < m; k++)
				s += lpH[k + i * m] * lpPsiRy[k + 1];
			out_float[i + 1] = out_float[i + 1] / gamma_float + s;
		}

		/* Update normalization coefficient */
		out_float[0] = lpPsiRy[0];
		for (i = 1; i <= m; i++)
			out_float[0] += gamma_float * out_float[i] * lpPsiRy[i];
		out_float[0] = sqrt(out_float[0]) * scale;

		if (j > itr1 && (ep - out_float[0]) / out_float[0] < dd)
			flag = 1;
	}

	/* Denormalize coefficients */
	ignorm_float(out_float, m, gamma_float);

	/* end - old floating-point implementation */
	ret = flag;
	/*-----------------------------------------------------------------------*/
#endif

	for (i = 0; i < order; i++)
		output[i] = (INT16) round(out_float[i] * 32767. / RES_NRM);

	dlp_free(in_float);
	dlp_free(out_float);
	return ret;
}

/*---------------------------------------------------------------------------*/
/* functions from floating-point implementation */
/* Invert symmetric positive definite matrix by LDL factorization
 *
 * @param A  Input and output buffer (size: n*n)
 * @param n  Matrix dimension
 */
/* dgetrf for pos.def. matrix
 *
 * (L'*D) * L = A
 *
 *   ( 1   0   0 )   ( D0 0  0  )   ( 1 L10 L20 )
 *   ( L10 1   0 ) * ( 0  D1 0  ) * ( 0 1   L21 )
 *   ( L20 L21 1 )   ( 0  0  D2 )   ( 0 0   1   )
 *
 *   ( D0     0      0  )   ( 1 L10 L20 )
 * = ( D0*L10 D1     0  ) * ( 0 1   L21 )
 *   ( D0*L20 D1*L21 D2 )   ( 0 0   1   )
 *
 *   ( D0     sym               sym                  )
 * = ( D0*L10 D0*L10^2+D1       sym                  )
 *   ( D0*L20 D0*L10*L20+D1*L21 D0*L20^2+D1*L21^2+D2 )
 */
/* A = (L^-1) * (D^-1) * (L^-1)'
 *
 * ##### L^-1 #####
 *
 * 0 ( 1   0   0   0 ) ( 1 0 0 0 )
 * 1 ( L10 1   0   0 ) ( 0 1 0 0 )
 * 2 ( L20 L21 1   0 ) ( 0 0 1 0 )
 * 3 ( L30 L31 L32 1 ) ( 0 0 0 1 )
 *
 * 0 ( 1   0   0   0 ) ( 1                   0            0    0 ) # = 0
 * 1 ( 0   1   0   0 ) ( -L10                1            0    0 ) # -L10*0
 * 2 ( 0   L21 1   0 ) ( -L20                0            1    0 ) # -L20*0
 * 2 ( 0   0   1   0 ) ( -L20+L21*L10        -L21         1    0 ) # -L21*1
 * 3 ( 0   L31 L32 1 ) ( -L30                0            0    1 ) # -L30*0
 * 3 ( 0   0   L32 1 ) ( -L30+L31*L10        -L31         0    1 ) # -L31*1
 * 3 ( 0   0   0   1 ) ( -L30+L31*L10        -L31+L32*L21 -L32 1 ) # -L32*2
 *                       -L32*(-L20+L21*L10)
 *
 * 0 ( 1   0   0   0 ) ( 1                   0            0    0 )
 * 1 ( 0   1   0   0 ) ( -L10                1            0    0 )
 * 2 ( 0   0   1   0 ) ( -L20+L21*L10        -L21         1    0 )
 * 3 ( 0   0   0   1 ) ( -L30+L31*L10        -L31+L32*L21 -L32 1 )
 *                       -L32*(-L20+L21*L10)
 *
 * ##### L*D*L' #####
 *
 *   ( D0     sym               sym                  )
 * = ( D0*L10 D0*L10^2+D1       sym                  )
 *   ( D0*L20 D0*L10*L20+D1*L21 D0*L20^2+D1*L21^2+D2 )
 */
void matinv_float(FLOAT64 *A, INT32 n) {
	INT32 i, j, k;
	/* Split L,D,L' */
	for (i = 1; i < n; i++) {
		for (j = 0; j < i; j++) {
			for (k = 0; k < j; k++)
				A[j * n + i] -= A[k * n + i] * A[k * n + j] * A[k * n + k];
			A[j * n + i] /= A[j * n + j];
			A[i * n + i] -= A[j * n + i] * A[j * n + i] * A[j * n + j];
		}
	}
	for (i = n - 1; i >= 1; i--) {
		/* Invert D */
		A[i * n + i] = 1. / A[i * n + i];
		/* Invert L */
		A[(i - 1) * n + i] = -A[(i - 1) * n + i];
		for (j = i - 2; j >= 0; j--) {
			A[j * n + i] = -A[j * n + i];
			for (k = j + 1; k < i; k++)
				A[j * n + i] -= A[k * n + i] * A[j * n + k];
		}
	}
	A[0] = 1. / A[0];
	/* Mult L*D*L' */
	for (i = 0; i < n; i++) {
		for (k = i + 1; k < n; k++)
			A[i * n + i] += A[k * n + k] * A[i * n + k] * A[i * n + k];
		for (j = i + 1; j < n; j++) {
			A[i * n + j] *= A[j * n + j];
			for (k = j + 1; k < n; k++)
				A[i * n + j] += A[k * n + k] * A[i * n + k] * A[j * n + k];
		}
	}
}

FLOAT64 sum2x_float(FLOAT64* vek1, FLOAT64* vek2, INT32 len) {
	INT32 i;
	FLOAT64 sum = 0.;
	for (i = 0; i < len; i++, vek1++, vek2++)
		sum += *vek1 * *vek2;
	return sum;
}

/* LPC parameter estimation via Burg method
 *
 * @param samples Input samples to be analysed
 * @param n       Number of samples in samples
 * @param a       Output LPC coefficients
 * @param p       Order of LPC analysis = number of output lpc coefficients in a
 * @param lambda  Warping factor.
 * @param scale   Signal scaling factor.
 * @return        0 on error, 1 on success
 */
/*
 * eb = ef = samples
 * a_0 = ef*ef
 *
 * for(m=1..p-1){
 *
 *   ew(m..n-1) = eb_v - lam*(ew_v-eb_c)
 *
 *   rc = -2*ew*ef/(ef*ef+ew*ew+1e-20)
 *   eb = ew + rc*ef
 *   ef = ef + rc*ew
 *
 *   a_0 *= 1-rc*rc
 *   aa(1..m-1) = a
 *   a(1..m-1) = aa + rc*rev(aa)
 *   a_m = -rc
 * }
 *
 * a_0 = sqrt(a_0)*scale
 */
char lpc_mburg_float(FLOAT64* samples, INT32 n, FLOAT64* a, INT16 p, FLOAT64 lambda,
		FLOAT64 scale) {
	register INT32 i;
	INT16 m;
	FLOAT64* aa;
	FLOAT64* eb;
	FLOAT64* ef;
	FLOAT64* ew;

	if (!samples || !a)
		return 0;
	if (p - 1 > n || p - 1 < 1)
		return 0;

	aa = (FLOAT64*) dlp_calloc(p + n * 3, sizeof(FLOAT64));
	eb = aa + p;
	ef = eb + n;
	ew = ef + n;

	for (i = 0; i < n; i++)
		eb[i] = ef[i] = samples[i];
	a[0] = sum2x_float(ef, ef, n);

	for (m = 1; m <= p - 1; m++) {
		FLOAT64 sf, sw, swf, rc;
		ew[m] = eb[m - 1] - lambda * eb[m];
		sf = ef[m] * ef[m];
		sw = ew[m] * ew[m];
		swf = ef[m] * ew[m];
		for (i = m + 1; i < n; i++) {
			ew[i] = eb[i - 1] + lambda * (ew[i - 1] - eb[i]);
			sf += ef[i] * ef[i];
			sw += ew[i] * ew[i];
			swf += ef[i] * ew[i];
		}

		rc = -2 * swf / (sf + sw + 1e-20);
		for (i = m; i < n; i++) {
			eb[i] = ew[i] + rc * ef[i];
			ef[i] += rc * ew[i];
		}

		a[0] *= 1. - rc * rc;
		for (i = 1; i < m; i++)
			aa[i] = a[i];
		for (i = 1; i < m; i++)
			a[i] = aa[i] + rc * aa[m - i];
		a[m] = -rc;
	}

	a[0] = sqrt(a[0]) * scale;

	dlp_free(aa);
	return 1;
}

/* Generalized cepstral transformation
 *
 * This function transforms generalized cepstral coefficients from
 * one cepstrum factor g1 to another one g2.
 *
 * @param c  Input and output cepstral coefficients
 * @param m  Number of coefficients, i.e. length of in/out buffer
 * @param g1 Input generalized cepstrum factor
 * @param g2 Output generalized cepstrum factor
 */
void gc2gc_float(FLOAT64 *c, const INT32 m, const FLOAT64 g1, const FLOAT64 g2) {
	INT32 i, k; /* Loop indices                       */
	FLOAT64 *cin = dlp_malloc((m + 1) * sizeof(FLOAT64)); /* Buffer of input coefficients       */
	for (i = 1; i <= m; i++)
		cin[i] = c[i]; /* Copy input coefficients            */
	for (i = 1; i <= m; i++)
		for (k = 1; k <= i - 1; k++) { /* Loop over output index + subindex>>*/
			FLOAT64 cc = cin[k] * c[i - k]; /*   Precalc input/output product     */
			c[i] += (g2 * k * cc - g1 * (i - k) * cc) / (FLOAT64) i; /*   Update output coefficient        */
		} /* >>                                 */
	dlp_free(cin); /* Free input buffer                  */
}

/* Inverse gain normalization.
 *
 * This function denormalizes the Generalized Cepstrum coefficients using the gain given in the zero_th
 * coefficient. This function is the inverse of. It calculates:
 *   c0' = (c0^g-1)/g
 *   ck' = ck * c0^g
 *
 * @param c  Input and output buffer
 * @param m  Number of coefficients, i.e. length of in/out buffer
 * @param g  Generalized cepstrum factor
 */
void ignorm_float(FLOAT64 *c, INT32 m, const FLOAT64 g) {
	if (g != 0.) { /* Check if first coef. is not zero >>*/
		FLOAT64 k = pow(c[0], g); /*   Get normalization factor         */
		*c++ = (k - 1.) / g; /*   Update first coefficient         */
		for (; m >= 1; m--)
			*c++ *= k; /*   Update remaining coefficients    */
	} else
		*c = log(*c); /* >> else update only first coef.    */
}

/* Frequency transformation filter initialization
 *
 * This function initializes file coefficients for frequency transformation
 * of autocorrelation signal according the Mel-scale.
 *
 * @param n_in    Number of samples in input buffer
 * @param n_out   Number of samples in output buffer
 * @param lambda  Warping factor (only for compatibility)
 * @param z       Filter coefficients
 * @param norm    Output normalization factor
 */
void filter_freqt_fir_init_float(INT32 n_in, INT32 n_out, FLOAT64 lambda, FLOAT64 *z,
		FLOAT64 norm) {
	INT32 k, l;
	FLOAT64 *za = z, *zb = z;
	zb[0] = lambda;
	for (l = n_in - 2; l; l--, zb++)
		zb[1] = zb[0] * lambda;
	zb++;
	for (k = n_out - 1; k; k--, za++, zb++) {
		zb[0] = -lambda * za[0];
		if (k == n_out - 1)
			zb[0] += 1.;
		for (l = n_in - 2; l; l--, za++, zb++)
			zb[1] = za[0] + lambda * (zb[0] - za[1]);
	}
	for (k = n_out, za = z; k; k--)
		for (l = n_in - 1; l; l--, za++)
			*za *= norm;
}

/* Frequency transformation filter
 *
 * This function performs a frequency transformation of
 * autocorrelation signal.
 *
 * @param in      Input buffer
 * @param n_in    Number of samples in input buffer
 * @param out     Output buffer
 * @param n_out   Number of samples in output buffer
 * @param z       Filter coefficients
 */
void filter_freqt_fir_float(FLOAT64* in, INT32 n_in, FLOAT64* out, INT32 n_out,
		FLOAT64 *z) {
	INT32 k, l;
	FLOAT64 *o = out, *i;
	for (k = n_out; k; k--, o++)
		for (l = n_in - 1, *o = 0., i = in + 1; l; l--)
			*o += *i++ * *z++;
	out[0] += in[0];
}

