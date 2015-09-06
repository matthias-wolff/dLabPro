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

#define INT16_MAX_FLOAT 32767.
#define OLD_FUNCTION_CALLS 0
#define FUNCTION_TEST 0

#define LOG_ACTIVE 1
#define FIXED_POINT 0

/* saves data for bachelor thesis evaluation */
#if LOG_ACTIVE
DataLog logger = { .file_path = "mgcepfix_log.csv" };
#endif

/* functions from floating-point implementation */
void matinv_float(FLOAT64 *A, INT32 n);
FLOAT64 sum2x_float(FLOAT64* vek1, FLOAT64* vek2, INT32 len);
char lpc_mburg_float(FLOAT64* samples, INT32 n, FLOAT64* a, INT16 p,
		FLOAT64 lambda, FLOAT64 scale);
void gc2gc_float(FLOAT64 *c, const INT32 m, const FLOAT64 g1, const FLOAT64 g2);
void ignorm_float(FLOAT64 *c, INT32 m, const FLOAT64 g);
void filter_freqt_fir_init_float(INT32 n_in, INT32 n_out, FLOAT64 lambda,
		FLOAT64 *z, FLOAT64 norm);
void filter_freqt_fir_float(FLOAT64* in, INT32 n_in, FLOAT64* out, INT32 n_out,
		FLOAT64 *z);

/* buffers from old floating point implementation - to be replaced incrementally */
FLOAT64 *lpZo, *lpZn;
FLOAT64 *lpSx, *lpSy, *lpGx, *lpGy;
FLOAT64 *lpPsiRx, *lpPsiPx, *lpPsiQx;
FLOAT64 *lpPsiRy, *lpPsiPy, *lpPsiQy;
FLOAT64 *lpH;

/* conversion buffers */
FLOAT64 *in_float, *out_float;

/* temporary buffers */
FLOAT64 *tempX, *tempY;
INT32 *tempI32;
INT16 *tempXI16, *tempYI16;

/* new integer buffers */
INT16 *lpSxI16, *lpSyI16;
INT32 *lpSxI32, *lpSyI32;
INT16 *lpGxI16, *lpGyI16;
INT32 *lpGxI32;
//INT16 *lpZoI16, *lpZnI16;
INT16 *lpPsiRxI16, *lpPsiPxI16, *lpPsiQxI16;
INT16 *lpPsiRyI16, *lpPsiPyI16, *lpPsiQyI16;
INT32 *lpPsiRyI32, *lpPsiPyI32, *lpPsiQyI32;
INT32 *lpHI32;
INT32 *outI32;

/* structures/plans for the fixed point FFTs */
struct dlmx_fft *fft_n_fwd_plan, *fft_freqt_plan, *fft_n_inv_plan;

/* Conversion factors */
/* INT -> FLOAT: Divide by these numbers */
/* FLOAT -> INT: Multiply with these numbers */
#define CON32 2147483648.
#define CON16 32768.

/* Normierungsfaktoren */
#define SIG_NRM	1.
#define RES_NRM	8.
//#define GAMMA_INV_NRM 2048. /* --> Gamma min=0.0625 @ 16-Bit */
#define PSIRY_NRM 1.
#define PSIPY_NRM 1.
#define PSIQY_NRM 1.
#define MAT_INV_NRM 0.0000012

/* Shifts */
#define DD_INV_SHL 6
#define IN_SHR 0 	/* input scaling */
#define FFT_SHR 5
#define FIRST_SHL -13
#define GC2GC_SHL -2	/* First component overflows! */
#define GC2GC_FIRST_SHL	((FIRST_SHL) + (GC2GC_SHL))	/* -> seperate norm for first coefficient */
#define FREQT_SHL -1 /* for all int! */
#define FFT_G_SHR 1
#define GAMMA_ADD_SHR 1
#define GAMMA_INV_SHR 4 /* --> Gamma min=0.0625 @ 16-Bit */
#define A_SHL 3
#define A_INV_SHL -8
#define TMP1_FACTOR_SHL -1
#define TMP1_SHL 17
#define IFFT_PSI_P_SHR 4
#define IFFT_PSI_Q_SHR 0
#define IFFT_PSI_R_SHR 4
#define MAT_INV_SHL -25

#define SHL16TO32 16

/*---------------------------------------------------------------------------*/
/* saturating utility functions */

INT32 round32(FLOAT64 x) {
	(x >= 0) ? (x += 0.5) : (x -= 0.5);
//	x = round(x);
	if (x > INT32_MAX)
		return INT32_MAX;
	if (x < INT32_MIN)
		return INT32_MIN;
	return (INT32) x;
}

INT16 round16(FLOAT64 x) {
	(x >= 0) ? (x += 0.5) : (x -= 0.5);
//	x = round(x);
	if (x > INT16_MAX)
		return INT16_MAX;
	if (x < INT16_MIN)
		return INT16_MIN;
	return (INT16) x;
}
/*---------------------------------------------------------------------------*/
/* fixed point saturating functions and function prototypes */

/* NON-FRACTIONAL log2! */
INT16 log2_16(INT16 x) {
	return round16((FLOAT64) log2(x)); /* TODO: fixed point */
}

INT16 pow_16(INT16 base, INT16 exp, INT8 shf) {
	return round16((FLOAT64) pow(base/CON16, exp/CON16) * CON16 * pow(2, shf)); /* TODO: fixed point */
}

INT32 pow_32(INT32 base, INT32 exp, INT8 shf) {
	return round32(pow(((FLOAT64) base/CON32), ((FLOAT64)exp/CON32)) * CON32 * pow(2, shf)); /* TODO: fixed point */
}

INT32 mul_32(INT32 a, INT32 b, INT8 shf) {
	return round32( ((FLOAT64) a) * ((FLOAT64) b) * pow(2, shf) );
}

INT32 sqrt_32(INT32 x) {
	return round32((FLOAT64) sqrt((FLOAT64) x / CON32) * CON32);
}

void matinv_32(INT32 *A, INT32 n) {
	return matinv_float((FLOAT64*) A, n);
}

/* fixed point implementation would have too much scaling issues */
INT32 calculate_tmp1_factor_inv(INT32 a, INT16 gamma, INT8 shf) {
	return round32(pow((FLOAT64) a/CON32, -(1. + 1./ ((FLOAT64) gamma/CON16))) * CON32 * pow(2, shf));
}

// TODO: Implement!
char lpc_mburg_32(INT32* samples, INT32 n, INT32* a, INT16 p,
		INT32 lambda, INT32 scale);
void gc2gc_32(INT32 *c, const INT32 m, const INT32 g1, const INT32 g2);
void ignorm_32(INT32 *c, INT32 m, const INT32 g);
void filter_freqt_fir_init_32(INT32 n_in, INT32 n_out, INT16 lambda,
		INT32 *z, INT16 norm);
void filter_freqt_fir_32(INT32* in, INT32 n_in, INT32* out, INT32 n_out,
		INT32 *z);

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
	filter_freqt_fir_init_float(n / 2, MIN(n, 2 * m + 1), lambda_float, lpZn,
			2.);

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

	/* fixed point buffer init */
	lpSxI16 = dlp_malloc(n * sizeof(INT16));
	lpSyI16 = dlp_malloc(n * sizeof(INT16));
	lpGxI16 = dlp_malloc(n * sizeof(INT16));
	lpGyI16 = dlp_malloc(n * sizeof(INT16));

	lpPsiRxI16 = dlp_malloc(n * sizeof(INT16));
	lpPsiRyI16 = dlp_malloc(n * sizeof(INT16));
	lpPsiPxI16 = dlp_malloc(n * sizeof(INT16));
	lpPsiPyI16 = dlp_malloc(n * sizeof(INT16));
	lpPsiQxI16 = dlp_malloc(n * sizeof(INT16));
	lpPsiQyI16 = dlp_malloc(n * sizeof(INT16));

	lpHI32 = (INT32*) dlp_malloc(m * m * sizeof(INT32));

	outI32 = (INT32*) dlp_malloc(order * sizeof(INT32));

	lpSxI32 = dlp_malloc(n * sizeof(INT32));
	lpSyI32 = dlp_malloc(n * sizeof(INT32));
	lpGxI32 = dlp_malloc(n * sizeof(INT32));
	lpPsiPyI32 = dlp_malloc(n * sizeof(INT32));
	lpPsiRyI32 = dlp_malloc(n * sizeof(INT32));
	lpPsiQyI32 = dlp_malloc(n * sizeof(INT32));

	/* conversion buffers */
	in_float = dlp_malloc(n * sizeof(FLOAT64));
	out_float = dlp_malloc(order * sizeof(FLOAT64));

	/* temporary buffers */
	tempX = dlp_malloc(n * sizeof(FLOAT64));
	tempY = dlp_malloc(n * sizeof(FLOAT64));
	tempI32 = dlp_malloc(n * sizeof(INT32));
	tempXI16 = dlp_malloc(n * sizeof(INT16));
	tempYI16 = dlp_malloc(n * sizeof(INT16));

	/* init fixed point ffts */
	fft_n_fwd_plan = dlmx_fft_init(n, FALSE);
	fft_freqt_plan = dlmx_fft_init(n, FALSE);
	fft_n_inv_plan = dlmx_fft_init(n, FALSE);

#if LOG_ACTIVE
	data2csv_init(&logger);
#endif

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
	dlp_free(outI32);

	/* free conversion buffers */
	dlp_free(in_float);
	dlp_free(out_float);

	/* free temporary buffers */
	dlp_free(tempX);
	dlp_free(tempY);
	dlp_free(tempI32);
	dlp_free(tempXI16);
	dlp_free(tempYI16);

	/* free fixed point buffers */
	dlp_free(lpSxI16);
	dlp_free(lpSyI16);
	dlp_free(lpGxI16);
	dlp_free(lpGyI16);
	dlp_free(lpSxI32);
	dlp_free(lpSyI32);
	dlp_free(lpGxI32);

	dlp_free(lpPsiRxI16);
	dlp_free(lpPsiRyI16);
	dlp_free(lpPsiPxI16);
	dlp_free(lpPsiPyI16);
	dlp_free(lpPsiQxI16);
	dlp_free(lpPsiQyI16);
	dlp_free(lpPsiRyI32);
	dlp_free(lpPsiPyI32);
	dlp_free(lpPsiQyI32);
	dlp_free(lpHI32);

	/* delete the FFT plans */
	dlmx_fft_free(fft_n_fwd_plan);
	dlmx_fft_free(fft_freqt_plan);
	dlmx_fft_free(fft_n_inv_plan);

#if LOG_ACTIVE
	data2csv_free(&logger);
#endif

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
 *  P = abssqr(S)/abssqr(G)/abssqr(G)^(1/gamma)
 *  R = G * P
 *  Q = G * G * P/abssqr(G)   # 25ms
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

	INT16 itr1 = 2;
	INT16 itr2 = 30;
	INT16 m = order - 1;
	INT32 j;
	INT32 k;
	INT16 flag = 0;

	/* variables from floating-point implementation */
	FLOAT64 dd_float = 0.000001;
	FLOAT64 ep_float = 0.;
	FLOAT64 gamma_float = (FLOAT64) gamma / CON16; //<<
	FLOAT64 lambda_float = (FLOAT64) lambda / CON16; //<<
	FLOAT64 scale = 32768.; //<<

	/* ... and the new fixed point equivalents */
	INT32 dd = (dd_float * CON32);
	INT32 ep = 0;

	/* new variables */
//	INT16 gamma_inv = round16(1 / gamma_float * GAMMA_INV_NRM);
	INT16 gamma_inv = dlmx_rnd32(dlmx_div32(INT32_MAX, dlmx_shl32(gamma, SHL16TO32), -GAMMA_INV_SHR));
//	INT16 tmp1_pow_exp_inv = dlmx_neg16(dlmx_add16(dlmx_shl16(1, GAMMA_INV_SHR), /* represents 1 (mult. identity element)*/
//											gamma_inv)); /* -(1 + 1/gamma) */

//	INT32 debug = dlmx_mul32(INT32_MAX, 23456);
//	INT16 debug16 = dlmx_mul16(INT16_MAX, 23456);
//	data2csv_INT32(&logger, "mul32 debug", &debug, 1); //debug
//	data2csv_INT16(&logger, "mul16 debug", &debug16, 1); //debug

//#if LOG_ACTIVE
//	data2csv_INT16(&logger, "gamma_inv", &gamma_inv, 1); /*<<<<<<<<<<<<<<<<<<<<<*/
//	data2csv_INT16(&logger, "tmp1_pow_exp", &tmp1_pow_exp, 1); /*<<<<<<<<<<<<<<<<<<<<<*/
//#endif

	/*-----------------------------------------------------------------------*/
	/* old floating-point implementation */
	/* INT16 dlm_mgcep(FLOAT64* input, INT32 n, FLOAT64* output, INT16 order, FLOAT64 gamma, FLOAT64 lambda, FLOAT64 scale) */

	for (i = 0; i < n; i++) {
		in_float[i] = (FLOAT64) input[i] / CON16 * SIG_NRM;
	}
#if !FUNCTION_TEST
	/* Get input spectrum */
	for (i = n - 1; i >= 0; i--) {
		lpSx[i] = in_float[i]; //<<
		lpSy[i] = 0.; //<<
		lpSxI16[i] = dlmx_shl16(input[i], -IN_SHR);
		lpSyI16[i] = 0;
	}

	dlm_fft(lpSx, lpSy, n, FALSE); //<<
	dlmx_fft(fft_n_fwd_plan, lpSxI16, lpSyI16, FFT_SHR);

	for (i = 0; i <= n / 2; i++) {
		lpSx[i] = lpSx[i] * lpSx[i] + lpSy[i] * lpSy[i]; //<<

		lpSxI32[i] = dlmx_add32(dlmx_mul16_32(lpSxI16[i], lpSxI16[i]),
				dlmx_mul16_32(lpSyI16[i], lpSyI16[i]));
//		lpSxI16[i] = dlmx_rnd32(lpSxI32[i]);
	}

//#if LOG_ACTIVE
////	data2csv_FLOAT64(&logger, "lpSx_before", lpSx, n/2); /*<<<<<<<<<<<<<<<<<<<<<*/
////	data2csv_FLOAT64(&logger, "lpSy_before", lpSy, n/2); /*<<<<<<<<<<<<<<<<<<<<<*/
//	data2csv_FLOAT64(&logger, "lpSx", lpSx, n/2);
	data2csv_INT32(&logger, "lpSxI32", lpSxI32, n/2);
//#endif

	/* FLOATING POINT -------------------------------------------------------*/
	/* conversion from INT to FLOAT */
#if FIXED_POINT
	for (i = 0; i <= n - 1; i++) {
		lpSx[i] = (FLOAT64) lpSxI32[i] / CON32 * pow(2, FFT_SHR + FFT_SHR)
		* pow(2, IN_SHR + IN_SHR);
		lpSy[i] = (FLOAT64) lpSyI16[i] / CON16 * pow(2, FFT_SHR)
		* pow(2, IN_SHR);
	}
#endif
//#if LOG_ACTIVE
//	data2csv_FLOAT64(&logger, "lpSx_after", lpSx, n/2); /*<<<<<<<<<<<<<<<<<<<<<*/
//	data2csv_FLOAT64(&logger, "lpSy_after", lpSy, n/2); /*<<<<<<<<<<<<<<<<<<<<<*/
//#endif

	/* Init coefficients from input signal */
	lpc_mburg_float(in_float, n, out_float, order, lambda_float, scale);
	gc2gc_float(out_float, m, -1, gamma_float); /* cepstral transformation from -1 (pure LPC) to gamma */

	output[0] = round16(out_float[0] * CON16 * pow(2, GC2GC_FIRST_SHL)); /* use different scaling for first coefficient */
	outI32[0] = round32(out_float[0] * CON32 * pow(2, GC2GC_FIRST_SHL)); /* use different scaling for first coefficient */
	for (i = order - 1; i >= 1; i--) {
		output[i] = round16(out_float[i] * CON16 * pow(2, GC2GC_SHL));
		outI32[i] = round32(out_float[i] * CON32 * pow(2, GC2GC_SHL));
	}

//#if LOG_ACTIVE
//	data2csv_FLOAT64(&logger, "out_float after gc2gc", out_float, order); /*<<<<<<<<<<<<<<<<<<<<<*/
//	data2csv_INT16(&logger, "output after gc2gc", output, order); /*<<<<<<<<<<<<<<<<<<<<<*/
//	data2csv_INT32(&logger, "outI32 after gc2gc", outI32, order); /*<<<<<<<<<<<<<<<<<<<<<*/
//#endif
	/*-----------------------------------------------------------------------*/

	/* Improve coefficients iteratively */
	for (j = 0; j < itr2 && !flag; j++) {

		ep_float = out_float[0]; //<<
		ep = outI32[0];
		out_float[0] = 1.; //<<
		output[0] = INT16_MAX; // TODO: maybe this needs scaling?
		outI32[0] = INT32_MAX;
		for (i = 1; i < order; i++) {
			out_float[i] *= gamma_float; //<<
			output[i] = dlmx_mul16(output[i], gamma); // this is just scaling
			outI32[i] = dlmx_mul3216(outI32[i], gamma);
		}
//#if LOG_ACTIVE
//		data2csv_FLOAT64(&logger, "out_float 1. before", out_float, order); /*<<<<<<<<<<<<<<<<<<<<<*/
//#endif

		/* FLOATING POINT -------------------------------------------------------*/
#if FIXED_POINT
		for (i = 0; i < order; i++) {
			out_float[i] = (FLOAT64) outI32[i] / CON32;
//			if(i != 0)
//				out_float[i] *= 15.;
		}
#endif

//#if LOG_ACTIVE
//		data2csv_FLOAT64(&logger, "out_float 2. after", out_float, order); /*<<<<<<<<<<<<<<<<<<<<<*/
//#endif
		/* Mel- + spectral transform of coefficients */
		filter_freqt_fir_float(out_float, order, lpGx, n, lpZo);
		for (i = n - 1; i >= 0; i--) {
			lpGxI16[i] = round16(lpGx[i] * CON16 * pow(2, FREQT_SHL));
		}
		/*-----------------------------------------------------------------------*/

//#if LOG_ACTIVE
//		data2csv_FLOAT64(&logger, "Gx before FFT", lpGx, n); /*<<<<<<<<<<<<<<<<<<<<<*/
//		data2csv_INT16(&logger, "GxI16 before FFT", lpGxI16, n); /*<<<<<<<<<<<<<<<<<<<<<*/
//#endif
		for (i = 0; i < n; i++) {
			lpGy[i] = 0.; //<<
			lpGyI16[i] = 0;
		}
		dlm_fft(lpGx, lpGy, n, FALSE); //<<
		dlmx_fft(fft_freqt_plan, lpGxI16, lpGyI16, FFT_G_SHR);
		/* Most of the Energy in the high Coefficients! */

//#if LOG_ACTIVE
//		data2csv_FLOAT64(&logger, "Gx after fft", lpGx, n); /*<<<<<<<<<<<<<<<<<<<<<*/
//		data2csv_FLOAT64(&logger, "Gy after fft", lpGx, n); /*<<<<<<<<<<<<<<<<<<<<<*/
//		data2csv_INT16(&logger, "GxI16 after fft", lpGxI16, n); /*<<<<<<<<<<<<<<<<<<<<<*/
//		data2csv_INT16(&logger, "GyI16 after fft", lpGyI16, n); /*<<<<<<<<<<<<<<<<<<<<<*/
//		for (i = 0; i <= n / 2; i++) {
//			tempI32[i] = dlmx_add32(dlmx_mul16_32(lpGxI16[i], lpGxI16[i]),
//					dlmx_mul16_32(lpGyI16[i], lpGyI16[i]));
//		}
//		data2csv_INT32(&logger, "FFT G abs", tempI32, n/2 + 1); /*<<<<<<<<<<<<<<<<<<<<<*/
//		for (i = 0; i <= n / 2; i++) {
//			tempI32[i] = round32(CON32 / (FLOAT64) tempI32[i] * CON32 * A_INV_NRM);
//		}
//		data2csv_INT32(&logger, "FFT_G_abs_inv = a_inv", tempI32, n/2 + 1); /*<<<<<<<<<<<<<<<<<<<<<*/
//#endif

		/* Get temporary psi-signals in spectral domain */
		for (i = 0; i <= n / 2; i++) {
			FLOAT64 a = lpGx[i] * lpGx[i] + lpGy[i] * lpGy[i]; //<< max ~1000, min ~0
			INT32 aI32 = dlmx_shl32(dlmx_add32(dlmx_mul16_32(lpGxI16[i], lpGxI16[i]),
					dlmx_mul16_32(lpGyI16[i], lpGyI16[i])), A_SHL);
			INT32 aInvI32 = dlmx_div32(INT32_MAX, aI32, A_INV_SHL);
//			aI32 = round32(a * CON32 * 0.15);//FIXME: DEBUG!!!
//#if LOG_ACTIVE
//			data2csv_FLOAT64(&logger, "a", &a, 1); /*<<<<<<<<<<<<<<<<<<<<<*/
//			data2csv_INT32(&logger, "aI32", &aI32, 1); /*<<<<<<<<<<<<<<<<<<<<<*/
//			data2csv_INT32(&logger, "aInvI32", &aInvI32, 1); /*<<<<<<<<<<<<<<<<<<<<<*/
//#endif
			FLOAT64 tmp1 = lpSx[i] / pow(a, 1. + 1. / gamma_float); //<< max ~60, min ~0
//			INT32 tmp1_factor = pow_32(aI32, dlmx_shl32(tmp1_pow_exp_inv, SHL16TO32), TMP1_FACTOR_SHL);
			INT32 tmp1_factor = calculate_tmp1_factor_inv(aI32, gamma, TMP1_FACTOR_SHL);
			INT32 tmp1I32 = dlmx_shl32(dlmx_mul32(tmp1_factor, lpSxI32[i]), TMP1_SHL);

//#if LOG_ACTIVE
			tempI32[i] = tmp1I32;
////			data2csv_FLOAT64(&logger, "tmp1", &tmp1, 1); /*<<<<<<<<<<<<<<<<<<<<<*/
//			data2csv_INT32(&logger, "tmp1I32 factor", &tmp1_factor, 1); /*<<<<<<<<<<<<<<<<<<<<<*/
////			data2csv_INT32(&logger, "tmp1I32", &tmp1I32, 1); /*<<<<<<<<<<<<<<<<<<<<<*/
//#endif

			lpPsiRx[i] = lpGx[i] * tmp1; //<<
			lpPsiRxI16[i] = dlmx_mul3216(tmp1I32, lpGxI16[i]);
			lpPsiRy[i] = lpGy[i] * tmp1; //<<
			lpPsiRyI16[i] = dlmx_mul3216(tmp1I32, lpGyI16[i]);

			lpPsiPx[i] = tmp1; //<<
			lpPsiPxI16[i] = dlmx_rnd32(tmp1I32);
			lpPsiPy[i] = 0.; //<<
			lpPsiPyI16[i] = 0;

			lpPsiQx[i] = (lpGx[i] * lpGx[i] - lpGy[i] * lpGy[i]) * tmp1 / a; //<<
			INT32 intermRes = dlmx_sub32(dlmx_mul16_32(lpGxI16[i], lpGxI16[i]),
					dlmx_mul16_32(lpGyI16[i], lpGyI16[i]));
			intermRes = dlmx_mul32(intermRes, tmp1I32);
			lpPsiQxI16[i] = dlmx_rnd32(dlmx_mul32(intermRes, aInvI32));
			lpPsiQy[i] = 2 * lpGx[i] * lpGy[i] * tmp1 / a; //<<
			lpPsiQyI32[i] = dlmx_shl32(dlmx_mul32(aInvI32, dlmx_mul32(dlmx_mul16_32(lpGxI16[i], lpGyI16[i]), tmp1I32)), 1);
			lpPsiQyI16[i] = dlmx_rnd32(lpPsiQyI32[i]);

//#if LOG_ACTIVE
//			data2csv_FLOAT64(&logger, "lpPsiQy", &lpPsiQy[i], 1); /*<<<<<<<<<<<<<*/
//			data2csv_INT16(&logger, "lpPsiQyI16", &lpPsiQyI16[i], 1); /*<<<<<<<<<<<<<*/
//#endif

#endif !FUNCTION_TEST /*-----------------------------------------------------------------------------------------------*/
#if FUNCTION_TEST
			printf("FUNCTION TESTING ACTIVE!\n");
			/* fill buffers with test data */
			const FLOAT64 scaling = 0.5; /* -> a min = 0.1 */
			FLOAT64 tmp1 = .8;
			FLOAT64 a = 0.5;
			INT32 aI32 = a * CON32;
			INT32 aInvI32 = round32(CON32 / (FLOAT64) aI32 * CON32 * scaling);
			INT32 tmp1I32 = tmp1 * CON32;
			for (i = 1, k = 5000; i <= n/2; i++, k += 4) {
				lpGy[i] = (FLOAT64) k / CON16;
				lpGyI16[i] = k;
				lpGx[i] = (FLOAT64) (k - 500) / CON16;
				lpGxI16[i] = k - 500;
			}
			FLOAT64 mygamma = -0.5;
			gamma = round16((FLOAT64) (INT16_MAX + 1.) * mygamma);
			gamma_float = mygamma;
			for (i = 0; i <= n / 2; i++) { //!!!!!!!!!!!!!!!!!!!!!
#endif
/*---------------------------------------------------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------------------------------------------------*/
#if FUNCTION_TEST
		} //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		for (i = 0; i <= n/2; i++) {
			tempX[i] = (FLOAT64) lpPsiQyI16[i] / CON16 / scaling;
		}
		data2csv_FLOAT64(&logger, "lpPsiQy", lpPsiQy, n/2); /*<<<<<<<<<<<<<*/
		data2csv_FLOAT64(&logger, "lpPsiQyI16 scaled", tempX, n/2); /*<<<<<<<<<<<<<*/
		data2csv_INT16(&logger, "lpPsiQyI16", lpPsiQyI16, n/2); /*<<<<<<<<<<<<<*/
//		data2csv_INT32(&logger, "aI32", &aI32, 1); /*<<<<<<<<<<<<<*/
//		data2csv_INT32(&logger, "aInvI32", &aInvI32, 1); /*<<<<<<<<<<<<<*/
#endif
#if !FUNCTION_TEST /*--------------------------------------------------------------------------------------------------*/

			if (i > 0 && i < n / 2) { /* = not executed in the last iteration */
				lpPsiRx[n - i] = lpPsiRx[i]; //<<
				lpPsiRxI16[n - i] = lpPsiRxI16[i];
				lpPsiRy[n - i] = -lpPsiRy[i]; //<<
				lpPsiRyI16[n - i] = dlmx_neg16(lpPsiRyI16[i]);
				lpPsiPx[n - i] = lpPsiPx[i]; //<<
				lpPsiPxI16[n - i] = lpPsiPxI16[i];
				lpPsiPy[n - i] = 0.; //<<
				lpPsiPyI16[n - i] = 0;
				lpPsiQx[n - i] = lpPsiQx[i]; //<<
				lpPsiQxI16[n - i] = lpPsiQxI16[i];
				lpPsiQy[n - i] = -lpPsiQy[i]; //<<
				lpPsiQyI32[n - i] = dlmx_neg32(lpPsiQyI32[i]);
				lpPsiQyI16[n - i] = dlmx_neg16(lpPsiQyI16[i]);
			}
		}

//#if LOG_ACTIVE
		data2csv_INT32(&logger, "tmp1I32_accumulated", tempI32, n/2);
//		data2csv_FLOAT64(&logger, "PsiRx", lpPsiRx, n/2);
//		data2csv_FLOAT64(&logger, "PsiRy", lpPsiRy, n);
//		data2csv_FLOAT64(&logger, "PsiPx", lpPsiPx, n/2);
//		data2csv_FLOAT64(&logger, "PsiQx", lpPsiQx, n/2);
//		data2csv_FLOAT64(&logger, "PsiQy", lpPsiQy, n/2);
		data2csv_INT16(&logger, "FreqDomain PsiRxI16", lpPsiRxI16, n/2);
		data2csv_INT16(&logger, "FreqDomain PsiRyI16", lpPsiRyI16, n);
		data2csv_INT16(&logger, "FreqDomain PsiPxI16", lpPsiPxI16, n/2);
		data2csv_INT16(&logger, "FreqDomain PsiQxI16", lpPsiQxI16, n/2);
		data2csv_INT16(&logger, "FreqDomain PsiQyI16", lpPsiQyI32, n/2);
//		data2csv_INT32(&logger, "FreqDomain PsiQyI32", lpPsiQyI32, n/2);
//#endif

		/* Transform psi-signals in time domain */
		dlm_fft(lpPsiRx, lpPsiRy, n, TRUE); //<< /* TODO: input n/2+1, output real n/2 */
		dlmx_fft(fft_n_inv_plan, lpPsiRxI16, lpPsiRyI16, IFFT_PSI_R_SHR);
		dlm_fft(lpPsiPx, lpPsiPy, n, TRUE); //<< /* TODO: input real n/2+1, output real n/2 */
		dlmx_fft(fft_n_inv_plan, lpPsiPxI16, lpPsiPyI16, IFFT_PSI_P_SHR);
		dlm_fft(lpPsiQx, lpPsiQy, n, TRUE); //<< /* TODO: input n/2+1, output real n/2 */
		dlmx_fft(fft_n_inv_plan, lpPsiQxI16, lpPsiQyI16, IFFT_PSI_Q_SHR);

#if LOG_ACTIVE
		data2csv_INT16(&logger, "TimeDomain PsiPxI16", lpPsiPxI16, n);
		data2csv_INT16(&logger, "TimeDomain PsiPyI16", lpPsiPyI16, n);
		data2csv_INT16(&logger, "TimeDomain PsiRxI16", lpPsiRxI16, n);
		data2csv_INT16(&logger, "TimeDomain PsiRyI16", lpPsiRyI16, n);
		data2csv_INT16(&logger, "TimeDomain PsiQxI16", lpPsiQxI16, n);
		data2csv_INT16(&logger, "TimeDomain PsiQyI16", lpPsiQyI16, n);
#endif

		/* Inverse Mel-transform of psi-signals */
		if (lambda != 0) {
			/* FLOATING POINT ---------------------------------------------------*/
#if FIXED_POINT
		for (i = 0; i < n/2; i++) {
			lpPsiRx[i] = (FLOAT64) lpPsiRxI16[i] / CON16;
			lpPsiPx[i] = (FLOAT64) lpPsiPxI16[i] / CON16;
			lpPsiQx[i] = (FLOAT64) lpPsiQxI16[i] / CON16;
		}
#endif
			filter_freqt_fir_float(lpPsiRx, n / 2, lpPsiRy, MIN(n, m + 1),
					lpZn);
			filter_freqt_fir_float(lpPsiPx, n / 2, lpPsiPy, MIN(n, m), lpZn);

			filter_freqt_fir_float(lpPsiQx, n / 2, lpPsiQy, MIN(n, 2 * m + 1),
					lpZn);
			/* get fixed point values */
			for (i = MIN(n, m + 1) - 1; i >= 0; i--) {
				lpPsiRyI32[i] = round32(lpPsiRy[i] * CON32 * PSIRY_NRM);
			}
			for (i = MIN(n, m) - 1; i >= 0; i--) {
				lpPsiPyI32[i] = round32(lpPsiPy[i] * CON32 * PSIPY_NRM);
			}
			for (i = MIN(n, 2 * m + 1) - 1; i >= 0; i--) {
				lpPsiQyI32[i] = round16(lpPsiQy[i] * CON32 * PSIQY_NRM);
			}
			/*-------------------------------------------------------------------*/
//#if LOG_ACTIVE
//		data2csv_INT32(&logger, "PsiPy after INV MEL", lpPsiPyI32, MIN(n, m + 1));
//		data2csv_INT32(&logger, "PsiRy after INV MEL", lpPsiRyI32, MIN(n, m));
//		data2csv_INT32(&logger, "PsiQy after INV MEL", lpPsiQyI32, MIN(n, 2 * m + 1));
//#endif

			for (i = 1; i <= m; i++) {
				lpPsiRy[i] *= .5; //<<
				lpPsiRyI32[i] = dlmx_shl32(lpPsiRyI16[i], -1); /* divide by 2 */
			}
			for (i = 1; i < m; i++) {
				lpPsiPy[i] *= .5; //<<
				lpPsiPyI32[i] = dlmx_shl16(lpPsiPyI32[i], -1); /* divide by 2 */
			}
			for (i = 1; i <= 2 * m; i++) {
				lpPsiQy[i] *= .5 * (1. + gamma_float); //<<
				lpPsiQyI32[i] = dlmx_shl32(lpPsiQyI32[i], -1); /* divide by 2 */
				lpPsiQyI32[i] = dlmx_mul3216(lpPsiQyI32[i],
										dlmx_add16(dlmx_shl16(INT16_MAX, -GAMMA_ADD_SHR),
												dlmx_shl16(gamma, -GAMMA_ADD_SHR)));
				lpPsiQyI32[i] = dlmx_shl32(lpPsiQyI32[i], GAMMA_ADD_SHR); /* scale back */
			}

		} else { // TODO: replace with 32 bit!
			for (i = 1; i <= m; i++) {
				lpPsiRy[i] = lpPsiRx[i] / (FLOAT64) n; //<<
				lpPsiRyI16[i] = dlmx_shl32(lpPsiRxI16[i], -log2_16(n));
			}
			for (i = 1; i < m; i++) {
				lpPsiPy[i] = lpPsiPx[i] / (FLOAT64) n; //<<
				lpPsiPyI16[i] = dlmx_shl16(lpPsiPxI16[i], -log2_16(n));
			}
			for (i = 1; i <= 2 * m; i++) {
				lpPsiQy[i] = lpPsiQx[i] * (1. + gamma_float) / (FLOAT64) n; //<<
				lpPsiQyI16[i] = dlmx_mul16(lpPsiQxI16[i],
						dlmx_add16(dlmx_shl16(INT16_MAX, -GAMMA_ADD_SHR),
								dlmx_shl16(gamma, -GAMMA_ADD_SHR))); /* FIXME: Scale! very small numbers in the following line! */
				lpPsiQyI16[i] = dlmx_shl16(lpPsiQyI16[i], -(log2_16(n) - GAMMA_ADD_SHR)); /* log + scale back *//*TODO: implement log fixed point method */
			}
		}

//#if LOG_ACTIVE
//		data2csv_INT32(&logger, "PsiPy after INV MEL and Division", lpPsiPyI32, MIN(n, m + 1));
//		data2csv_INT32(&logger, "PsiRy after INV MEL and Division", lpPsiRyI32, MIN(n, m));
//		data2csv_INT32(&logger, "PsiQy after INV MEL and Division", lpPsiQyI32, MIN(n, 2 * m + 1));
//#endif

		/* Combine to H matrix and invert it */
		for (i = 0; i < m; i++) {
			for (k = 0; k <= i; k++) {
				lpH[i + k * m] = lpPsiPy[abs(k - i)] + lpPsiQy[k + i + 2]; //<<
				lpHI32[i + k * m] = dlmx_add32(lpPsiPyI32[dlmx_abs16(k - i)],
						lpPsiQyI32[k + i + 2]);
			}
		}
		/* FLOATING POINT ---------------------------------------------------*/
#if FIXED_POINT
		for (i = 0; i < m*m; i++) {
			lpH[i] = (FLOAT64) lpHI32[i] / CON32;
		}
#endif
		matinv_float(lpH, m);
		for (i = 0; i < m; i++) {
			for (k = 0; k < m; k++) {
				lpHI32[k + m * i] = round32(lpH[k + m * i] * CON32 * pow(2, MAT_INV_SHL));
			}
		}
		/*-------------------------------------------------------------------*/

//#if LOG_ACTIVE
//		data2csv_FLOAT64(&logger, "MATRIX lpH", lpH, m*m); /*<<<<<<<<<<<<<<<<<<<<<*/
//		data2csv_INT32(&logger, "MATRIX lpHI32", lpHI32, m*m); /*<<<<<<<<<<<<<<<<<<<<<*/
//#endif


		/* Update coefficients from H matrix */
		for (i = 0; i < m; i++) {
			FLOAT64 s = 0.; //<<
			INT32 sI32 = 0;
			for (k = 0; k <= i; k++) {
				s += lpH[i + k * m] * lpPsiRy[k + 1]; //<<
				sI32 = dlmx_add32(sI32,
						dlmx_mul3216(lpHI32[i + k * m], lpPsiRyI16[k + 1]));
			}
			for (; k < m; k++) {
				s += lpH[k + i * m] * lpPsiRy[k + 1]; //<<
				sI32 = dlmx_add32(sI32,
						dlmx_mul3216(lpHI32[k + i * m], lpPsiRyI16[k + 1]));
			}
			out_float[i + 1] = out_float[i + 1] / gamma_float + s; //<<
			output[i + 1] = dlmx_rnd32(
					dlmx_add32(dlmx_mul16_32(output[i + 1], gamma_inv), sI32));
//#if LOG_ACTIVE
//			data2csv_FLOAT64(&logger, "s", &s, 1); /*<<<<<<<<<<<<<<<<<<<<<*/
//			data2csv_INT32(&logger, "sI32", &sI32, 1); /*<<<<<<<<<<<<<<<<<<<<<*/
//#endif
		}

//#if LOG_ACTIVE
//		data2csv_FLOAT64(&logger, "out_float[0] before update", &out_float[0], 1); /*<<<<<<<<<<<<<<<<<<<<<*/
//		data2csv_INT16(&logger, "output[0] before update", &output[0], 1); /*<<<<<<<<<<<<<<<<<<<<<*/
//#endif

		/* Update normalization coefficient */
		out_float[0] = lpPsiRy[0]; //<<
		outI32[0] = dlmx_shl32(lpPsiRyI16[0], SHL16TO32);
		for (i = 1; i <= m; i++) {
			out_float[0] += gamma_float * out_float[i] * lpPsiRy[i]; //<<
			outI32[0] = dlmx_add32(outI32[0], // TEST!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
					dlmx_mul32(dlmx_mul16_32(INT16_MAX, gamma),
							dlmx_mul16_32(output[i], lpPsiRyI16[i])));
			output[0] = dlmx_rnd32(outI32[0]);
		}

		out_float[0] = sqrt(out_float[0]) * scale; //<<
		outI32[0] = sqrt_32(outI32[0]); /* No scale needed!*/
		output[0] = dlmx_rnd32(outI32[0]);

//#if LOG_ACTIVE
//		data2csv_FLOAT64(&logger, "out_float[0] after update", &out_float[0], 1); /*<<<<<<<<<<<<<<<<<<<<<*/
//		data2csv_INT16(&logger, "output[0] after update", &output[0], 1); /*<<<<<<<<<<<<<<<<<<<<<*/
//		data2csv_INT32(&logger, "outI32[0] for debug", &outI32[0], 1);
//#endif

		/* DEBUG: see if break condition works*/
		ep = round32((FLOAT64) ep_float * CON32 * 0.00001); //xxx
		outI32[0] = round32((FLOAT64) out_float[0] * CON32 * 0.00001); //xxx

#if 1 //FIXED_POINT
		if (j > itr1 && (dlmx_sub32(ep, outI32[0])) < (dlmx_mul32(outI32[0], dd))) { /* ASSERT: out[0] >= 0, valid because of sqrt */
			flag = 1;
		}
#else
		/* FLOATING POINT ---------------------------------------------------*/
		if (j > itr1 && (ep_float - out_float[0]) / out_float[0] < dd_float) {
			flag = 1;
		}
		/*-------------------------------------------------------------------*/
#endif
	}

	/* FLOATING POINT -------------------------------------------------------*/
#if FIXED_POINT
	for(i = 0; i < m; i++) {
		out_float[i] = (FLOAT64) output[i] / CON16;
	}
#endif
	/* Denormalize coefficients */
	ignorm_float(out_float, m, gamma_float); /* DEBUG!*/
#if FIXED_POINT
	for (i = 0; i < m; i++) {
		output[i] = round16(out_float[i] * CON16);
	}
#endif
	/*-----------------------------------------------------------------------*/

	/* end - old floating-point implementation */
	ret = flag;
	/*-----------------------------------------------------------------------*/
#if LOG_ACTIVE
#if FIXED_POINT
	data2csv_INT16(&logger, "out_cepstrum", output, order); /*<<<<<<<<<<<<<*/
#else
	data2csv_FLOAT64(&logger, "out_cepstrum", out_float, order); /*<<<<<<<<<<<<<*/
#endif
	data2csv_INT32(&logger, "j", &j, 1); /*<<<<<<<<<<<<<*/
#endif

	for (i = 0; i < order; i++) {
		output[i] = round16(out_float[i] * CON16 / RES_NRM);
	}
#endif !FUNCTION_TEST

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
char lpc_mburg_float(FLOAT64* samples, INT32 n, FLOAT64* a, INT16 p,
		FLOAT64 lambda, FLOAT64 scale) {
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
void filter_freqt_fir_init_float(INT32 n_in, INT32 n_out, FLOAT64 lambda,
		FLOAT64 *z, FLOAT64 norm) {
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

