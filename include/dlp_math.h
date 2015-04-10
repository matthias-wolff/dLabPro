/* dLabPro mathematics library
 * - Header file
 *
 * AUTHOR : Matthias Wolff
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

#ifndef __DLPMATH_H
#define __DLPMATH_H

/* Functions */                                                                 /* --------------------------------- */
#ifdef __cplusplus                                                              /* C++ compiler?                     */
extern "C" {                                                                    /* >> Enable C linkage               */
#endif                                                                          /* #ifdef __cplusplus                */

#define MAX_PADE_ORDER 7

#define DLM_CALCCEP_METHOD_S_FFT_CEP              1                             /* Methods for dlm_calccep           */
#define DLM_CALCCEP_METHOD_S_FFT_MFFT_MCEP        2                             /* |                                 */
#define DLM_CALCCEP_METHOD_S_FFT_CEP_MCEP         3                             /* |                                 */
#define DLM_CALCCEP_METHOD_S_CEP_UELS             4                             /* |                                 */
#define DLM_CALCCEP_METHOD_S_MCEP_UELS            5                             /* |                                 */
#define DLM_CALCCEP_METHOD_S_LPC_BURG_CEP         6                             /* |                                 */
#define DLM_CALCCEP_METHOD_S_LPC_LEVI_CEP         7                             /* |                                 */
#define DLM_CALCCEP_METHOD_S_LPC_BURG_MCEP        8                             /* |                                 */
#define DLM_CALCCEP_METHOD_S_LPC_LEVI_MCEP        9                             /* |                                 */
#define DLM_CALCCEP_METHOD_S_LPC_BURG_CEP_MCEP   10                             /* |                                 */
#define DLM_CALCCEP_METHOD_S_LPC_LEVI_CEP_MCEP   11                             /* |                                 */
#define DLM_CALCCEP_METHOD_S_LPC_BURG_MLPC_MCEP  12                             /* |                                 */
#define DLM_CALCCEP_METHOD_S_LPC_LEVI_MLPC_MCEP  13                             /* |                                 */
#define DLM_CALCCEP_METHOD_S_MLPC_BURG_MCEP      14                             /* |                                 */
#define DLM_CALCCEP_METHOD_S_MLPC_LEVI_MCEP      15                             /* |                                 */

#define DLM_ROOTS_STABLE                    0
#define DLM_ROOTS_UNSTABLE                  1

#define DLM_PITCH_PULSE                     0                                   /* Type of excitation generation     */
#define DLM_PITCH_GLOTT                     1                                   /* |                                 */
#define DLM_PITCH_RANDPHASE                 2                                   /* |                                 */
#define DLM_PITCH_VOICED                    3                                   /* |                                 */
#define DLM_PITCH_UNVOICED                  4                                   /* |                                 */
#define DLM_PITCH_CUSTOM                    5                                   /* |                                 */

/* Functions - dlm_arith.c */                                                   /* --------------------------------- */
BOOL  dlm_is_diag(const FLOAT64* A, INT32 nXD);                                 /* Check if matrix is diagonal       */
void  dlm_print(const FLOAT64* A, INT32 nXC, INT32 nXR);                        /* Print matrix                      */
const opcode_table* dlm_matrop_entry(INT16 nEntry);                             /* Get an entry of matrix table      */
void  dlm_matrop_printtab();                                                    /* Print table of matrix operations  */
INT16 dlm_matrop_code(const char* lpsOpname, INT16* lpnOps);                    /* Matrix operation name to opcode   */
const char* dlm_matrop_name(INT16 nOpcode);                                     /* Matrix opcode to operation name   */
const char* dlm_matrop_signature(INT16 nOpcode);                                /* Matrix opcode to signature        */
INT16 dlm_matrop(FLOAT64* Z,INT32* lpnXCz, INT32* lpnXRz, const FLOAT64* A,     /* Matrix operation wrapper function */
                  INT32 nXCa, INT32 nXRa, const FLOAT64* B, INT32 nXCb,         /* |                                 */
                  INT32 nXRb, INT16 nOpcode);                                   /* |                                 */
INT16 dlm_matropC(COMPLEX64* Z,INT32* lpnXCz, INT32* lpnXRz, const COMPLEX64* A,/* Matrix operation wrapper function */
                  INT32 nXCa, INT32 nXRa, const COMPLEX64* B, INT32 nXCb,       /* |                                 */
                  INT32 nXRb, INT16 nOpcode);                                   /* |                                 */

/* Functions - dlm_chf.c */                                                     /* --------------------------------- */
INT16 dlm_cholf(FLOAT64* Z, const FLOAT64* A, INT32 nXD);                       /* Cholesky factorization            */
INT16 dlm_cholfC(COMPLEX64* Z, const COMPLEX64* A, INT32 nXD);                  /* Cholesky factorization            */

/* Functions - dlm_comb.c */                                                    /* --------------------------------- */
INT64         dlm_n_over_k(INT32 n, INT32 k);                                   /* n over k                          */

/* Functions - dlm_crc32.c */                                                   /* --------------------------------- */
void*         dlm_crc32_init();                                                 /* Create CRC-32 hash generator      */
void          dlm_crc32_add(void* lpH, const BYTE* lpBuffer, INT32 nLen);       /* Feed CRC-32 hash generator        */
INT16         dlm_crc32_finalize(void* lpH, BYTE* lpHash, INT32 nLen);          /* Compute hash and destroy generator*/
UINT64        dlm_crc32(const char* lpsText);                                   /* CRC-32 all-in-one                 */

/* Functions - dlm_eigen.c */                                                   /* --------------------------------- */
INT16 dlm_eigen_jac(FLOAT64* A, FLOAT64* B, INT32 nXD, INT16 bNorm);            /* Eigenvectors/-vals.(Jacobi method)*/
INT16 dlm_ieigen(FLOAT64* Z, FLOAT64* EVL, FLOAT64* EVC, INT32 nXD,             /* Inverse eigenvec./-val. compuation*/
                 INT16 bNorm);                                                  /* |                                 */

/* Functions - dlm_gel.c */                                                     /* --------------------------------- */
FLOAT64 dlm_get_det_trf(FLOAT64* A, INT32 nXA, void* ipiv);                     /* Get determinant after LU decomp.  */
COMPLEX64 dlm_get_det_trfC(COMPLEX64* A, INT32 nXA, void* ipiv);                /* Get determinant after LU decomp.  */
INT16 dlm_invert_gel(FLOAT64* A, INT32 n, FLOAT64* lpnDet);                     /* Matrix inversion by Gaussian elim.*/
INT16 dlm_invert_gelC(COMPLEX64* A, INT32 n, COMPLEX64* lpnDet);                /* Matrix inversion by Gaussian elim.*/

/* Functions - dlm_sig.c */                                                     /* --------------------------------- */
INT16 dlm_convol_t(FLOAT64* Z, const FLOAT64* A, INT32 nLa, const FLOAT64* B,   /* Time domain convolution           */
                   INT32 nLb);                                                  /* |                                 */
INT16 dlm_ccf(FLOAT64* Z, const FLOAT64* A, INT32 nLa, const FLOAT64* B,        /* Cross correlation function        */
                   INT32 nLb);                                                  /* |                                 */
INT16 dlm_distributionC(COMPLEX64*,COMPLEX64*,COMPLEX64*,INT32,INT32,INT32);    /* Distribution                      */
INT16 dlm_distribution (FLOAT64*,  FLOAT64*,  FLOAT64*,  INT32,INT32,INT32);    /* Distribution                      */

/* Functions - dlm_stat.c */                                                    /* --------------------------------- */
FLOAT64       dlm_studt(FLOAT64 x, FLOAT64 k);                                  /* Student's t-density               */
FLOAT64       dlm_gamma(FLOAT64 x);                                             /* Gamma function                    */
FLOAT64       dlm_lgamma(FLOAT64 x);                                            /* Log. gamma function               */
FLOAT64       dlm_beta(FLOAT64 a, FLOAT64 b);                                   /* Euler's beta function             */
FLOAT64       dlm_betadens(FLOAT64 x, FLOAT64 alpha, FLOAT64 beta);             /* Beta density                      */
FLOAT64       dlm_betaquant(FLOAT64 P, FLOAT64 alpha, FLOAT64 beta);            /* P-quantile of Beta CDF            */

/* Functions - dlm_xft.c */                                                     /* --------------------------------- */
INT16 dlm_fft_mag(FLOAT64*, FLOAT64*, INT32);                                   /* Magnitude spectrum                */
INT16 dlm_fft_log10(FLOAT64*, FLOAT64*, INT32, FLOAT64);                        /* Log10 spectrum                    */
INT16 dlm_fft_ln(FLOAT64*, FLOAT64*, INT32, FLOAT64);                           /* Ln spectrum                       */
INT16 dlm_fft_warp(FLOAT64*, FLOAT64*, INT32, FLOAT64);                         /* Magnitude spectrum warping        */
INT16 dlm_fft_warpC(COMPLEX64*, COMPLEX64*, INT32, FLOAT64);                    /* Complex spectrum warping          */
void  dlm_fft_cleanup();                                                        /* Deallocs. sine and cosing tables  */
INT16 dlm_fft(FLOAT64* RE, FLOAT64* IM, INT32 nXL, INT16 bInv);                 /* Complex FFT (FLOAT64 precision)   */
INT16 dlm_fftC(COMPLEX64* C, INT32 nXL, INT16 bInv);                            /* Complex FFT (FLOAT64 precision)   */
INT16 dlm_unwrapC(COMPLEX64*, INT32);                                           /* Phase unwrap                      */

/* Functions - dlm_dft.c */
void  dlm_fct1(FLOAT64*, INT32);
void  dlm_ifct1(FLOAT64*, INT32);
void  dlm_fct2(FLOAT64*, INT32);
void  dlm_ifct2(FLOAT64*, INT32);
INT16 dlm_dct1(FLOAT64*, INT32, FLOAT64*);
INT16 dlm_idct1(FLOAT64*, INT32, FLOAT64*);
INT16 dlm_dct2(FLOAT64*, INT32, FLOAT64*);
INT16 dlm_idct2(FLOAT64*, INT32, FLOAT64*);
INT16 dlm_dct3(FLOAT64*, INT32, FLOAT64*);
INT16 dlm_idct3(FLOAT64*, INT32, FLOAT64*);
INT16 dlm_dct4(FLOAT64*, INT32, FLOAT64*);
INT16 dlm_idct4(FLOAT64*, INT32, FLOAT64*);
INT16 dlm_dftC(COMPLEX64*, INT32, BOOL, COMPLEX64*);

/* Functions - dlm_fba.c */
struct dlm_fba_doframing_param {
  FLOAT64  nLambda;
  INT32   nCrate;
  FLOAT64 nDC;
  INT32   nLen;
  FLOAT64 nMinLog;
  INT16  nNPeriods;
  FLOAT64 nPreem;
  INT32   nWlen;
  INT16  nWnorm;
  char   lpsWtype[255];
  char   bEnergy;
  char   bLogEnergy;
  char   bTimeDomainWarping;
};
INT16 dlm_fba_window(FLOAT64* X, INT32 n, const char* sWindow, BOOL bNorm);
INT16 dlm_fba_preemphasis(FLOAT64* lpIn, INT32 nXIn, FLOAT64 nPreem);
INT16 dlm_fba_makewindow(FLOAT64 *lpWindow, INT32 nWlen, const char* lpsWtype, INT16 nWnorm);
INT32 dlm_fba_grabframe(FLOAT64 *lpSignal,INT32 nXSamples,INT32 nOffset,INT32 nFrameLength,INT32 nLen,FLOAT64 nMinLog,FLOAT64 nLambda,char bTimeDomainWarping,FLOAT64 nDC,FLOAT64 *lpFrame,FLOAT64 *lpWindow,FLOAT64* lpEnergy);
INT16 dlm_fba_doframing_maxframelen(INT32 nXSamples,INT64 *lpPitch,INT32 nXPitch,INT16 nNPeriods,INT32 nLen,INT32 nCrate,INT32 nWlen,INT32 *nMaxFrameLength,INT32 *nXFrames);
INT16 dlm_fba_doframing_initparam(struct dlm_fba_doframing_param *lpParam);
INT16 dlm_fba_doframing(FLOAT64 *lpSignal,INT32 nXSamples,INT64 *lpPitch,INT32 nXPitch,FLOAT64 *lpWindow,const char **lpLabIn,char **lpLabOut,INT16 *lpFrameLen,FLOAT64 *lpFrames,INT32 nXFrames,FLOAT64 *lpEnergy,struct dlm_fba_doframing_param *lpParam);
INT16 dlm_fba_deltafba(FLOAT64 *lpFrames,FLOAT64 *lpDeltaT,FLOAT64 *lpDeltaW,INT32 nXFrames,BOOL bFRing,INT32 nDeltaWL,INT32 nFDim,INT32 *nVDim,INT32 *nADim,INT32 nDim);
INT16 dlm_fba_delta(FLOAT64* Z, FLOAT64* A, INT32 nXC, BOOL bRA, INT32 nDim, FLOAT64* W, INT16 L);

/* Functions - dlm_filter.c */                                                  /* ----------------------------------*/
INT16 dlm_filter(FLOAT64* b, INT32 n_b, FLOAT64* a, INT32 n_a,                  /* IIR Filter                        */
                FLOAT64* input, FLOAT64* output, INT32 n,                       /* |                                 */
                FLOAT64* mem, INT32 n_mem);                                     /* |                                 */
INT16 dlm_filter_fir(FLOAT64* b, INT32 n_b, FLOAT64* input,                     /* FIR Filter                        */
                     FLOAT64* output, INT32 n,                                  /* |                                 */
                     FLOAT64* mem, INT32 n_mem);                                /* |                                 */
INT16 dlm_filter_iir(FLOAT64* a, INT32 n_a, FLOAT64* input, FLOAT64* output,    /* Pure IIR Filter                   */
                     INT32 n, FLOAT64* mem, INT32 n_mem);                       /* |                                 */
INT16 dlm_filter_m(FLOAT64* b, INT32 n_b, FLOAT64* a, INT32 n_a,                /* Mel-IIR Filter                    */
                   FLOAT64* input, FLOAT64* output, INT32 n,                    /* |                                 */
                   FLOAT64* mem, INT32 n_mem, FLOAT64 lambda);                  /* |                                 */
INT16 dlm_filter_mfir(FLOAT64* b, INT32 n_b, FLOAT64* input, FLOAT64* output,   /* MFIR Filter                       */
                      INT32 n, FLOAT64* mem, INT32 n_m, FLOAT64);               /* |                                 */
INT16 dlm_filter_miir(FLOAT64* a, INT32 n_a, FLOAT64* input, FLOAT64* output,   /* MIIR Filter                       */
                      INT32 n, FLOAT64* mem, INT32 n_m, FLOAT64);               /* |                                 */
INT16 dlm_freqt(FLOAT64* b, INT32 n_b, FLOAT64* input, INT32 n, FLOAT64);       /* Frequency transformation          */
INT16 dlm_freqtC(COMPLEX64*, INT32, COMPLEX64*, INT32, FLOAT64);                /* Frequency transformation          */
INT16 dlm_c2b(FLOAT64* c, FLOAT64* b, INT32 m, FLOAT64 lambda);                 /* Transform to realizable coeff.    */
INT16 dlm_filter_freqt_fir(FLOAT64*, INT32, FLOAT64*, INT32, FLOAT64);          /* Frequency transform FIR style     */

/* Functions - dlm_melfilter.c */
typedef struct {
  INT16     n_in;        /* input channels */
  INT16     n_out;       /* output channels */
  FLOAT64   quant_energ; /* quant. error of energy comp. */
  FLOAT64*  mid;         /* array of mid frequ. */
  FLOAT64*  width[2];    /* array of conv. core width (left and right) */
  FLOAT64*  norm;        /* array of normal. coeff. */
  FLOAT64*  z;
  FLOAT64** a;           /* matrix of core coeff. */
  char      type[255];   /* MF_TUD or MF_BILINEAR */
  FLOAT64   lambda;      /* warping coeff. */
} MLP_CNVC_TYPE;
INT16 dlm_mf_analyze (MLP_CNVC_TYPE*, FLOAT64*, FLOAT64*, INT32, INT32, FLOAT64);
INT16 dlm_mf_convolve(MLP_CNVC_TYPE*, FLOAT64*, FLOAT64*);
INT16 dlm_mf_init    (MLP_CNVC_TYPE*, INT16, INT16, FLOAT64);
INT16 dlm_mf_done    (MLP_CNVC_TYPE*);

/* Functions - dlm_noiserdc.c */
INT16 dlm_noisesetup(INT32, FLOAT32, FLOAT32*, INT32);
INT16 dlm_noiserdc(void*, INT32, BOOL);
INT16 dlm_noisefrc(BOOL);
INT16 dlm_spec_denoise(FLOAT64*,FLOAT64*,INT32,INT32,INT32,FLOAT64);            /* Spectrum denoising (spec.mean.sub)*/
INT16 dlm_mcep_denoise(FLOAT64*,FLOAT64*,INT32,INT32,INT32,FLOAT64);            /* Cepstrum denoising (cep.mean.sub) */

/* Functions - dlm_ftt.c */
typedef struct {
  FLOAT64*  midfreq;     /* array of mid frequencies */
  FLOAT64*  bandwidth;   /* array of bandwidths */
  FLOAT64*  real_d;      /* real part of spectrum */
  FLOAT64*  imag_d;      /* imag part of spectrum */
  FLOAT64*  smooth_f;     /* array of last-values */
  FLOAT64   b3khz;       /* bandwidth at 3 kHz */
  FLOAT64   atf;       /* sampling frequency in Hz*/
  FLOAT64   start_freq;  /* first mid-frequency*/
  FLOAT64   bw_coeff;    /* rel. bandwidth (mel-scale)*/
  FLOAT64   finc_coeff;  /* rel. frequency increment */
  FLOAT64   sm_coeff;    /* smooth coefficient*/
  FLOAT64   norm_coeff;  /* normalize coefficient*/
  FLOAT64   log;         /* Dynamik range in dB */
  FLOAT64   log_min;     /* min. value for log */
  FLOAT64   log_scale;   /* scale value after log */
  FLOAT64   max_value;   /* max Value before log */
  char    type[255];     /* M: mel scaled spectrum */
} MLP_FTT_TYPE;
INT16 dlm_ftt_analyze (MLP_FTT_TYPE*, void*, INT32, INT32, FLOAT64);
INT16 dlm_ftt_init    (MLP_FTT_TYPE*, INT16, INT16);
INT16 dlm_ftt_done    (MLP_FTT_TYPE*, INT16);

/* Functions - dlm_root.c */
INT16  dlm_routh       (FLOAT64*, INT16);                                       /* Test stability of poly            */
INT16  dlm_roots       (FLOAT64*, COMPLEX64*, INT16);                           /* Roots of a polynomial             */
INT16  dlm_roots_track_homotopy(FLOAT64*,FLOAT64*,FLOAT64*,FLOAT64*,            /* Roots of polynomial by tracking   */
                        FLOAT64*,FLOAT64*,INT16,INT16,INT16, FLOAT64);          /* |                                 */
FLOAT64 dlm_roots_track_match_poly(COMPLEX64*,COMPLEX64*,COMPLEX64*,COMPLEX64*, /* Roots of polynomial by tracking   */
                        INT16,INT16,FLOAT64);                                   /* |                                 */
INT16  dlm_poly        (FLOAT64*, FLOAT64*, INT16, FLOAT64*, FLOAT64*);         /* Polynomial of roots               */
INT16  dlm_polyC(COMPLEX64*, INT16, COMPLEX64*);                                /* Polynomial of roots               */
INT32  dlm_stabilise(FLOAT64*, INT32);                                          /* Stabilize polynomial              */
INT32  dlm_roots_cart2pol(FLOAT64*, FLOAT64*, INT32);                           /* Cartesian to polar conversion     */
INT32  dlm_roots_pol2cart(FLOAT64*, FLOAT64*, INT32);                           /* Polar to Cartesian conversion     */
INT16  dlm_balance(FLOAT64* M, INT32 n);                                        /* Matrix balance                    */
INT16  dlm_hess(FLOAT64* M, INT32 n);                                           /* Matrix to Hessenberg form         */
INT16  dlm_hessC(COMPLEX64* M, INT32 n);                                        /* Complex variant of dlm_hess       */
INT16  dlm_hqr(FLOAT64* M, COMPLEX64* Z, INT32 n);                              /* Eigenvalues of Hessenberg matrix  */

/* Functions - dlm_lpc.c */
INT16 dlm_lpc_burg  (FLOAT64*, INT32, FLOAT64*, INT16, FLOAT64);                /* LPC analysis via Burg             */
INT16 dlm_lpc_mburg (FLOAT64*, INT32, FLOAT64*, INT16, FLOAT64, FLOAT64);       /* Mel-LPC analysis via Burg         */
INT16 dlm_lpc_lev   (FLOAT64*, INT32, FLOAT64*, INT32, FLOAT64);                /* LPC analysis via Levinson         */
INT16 dlm_lpc_mlev  (FLOAT64*, INT32, FLOAT64*, INT32, FLOAT64, FLOAT64);       /* Mel-LPC analysis via Levinson     */
INT16 dlm_lpc2mlpc  (FLOAT64*, INT16, FLOAT64*, INT16, FLOAT64);                /* Bilinear transform of LPC         */
INT16 dlm_mlpc2lpc  (FLOAT64*, INT16, FLOAT64*, INT16, FLOAT64);                /* Bilinear back-transform of Mel-LPC*/
INT16 dlm_parcor2lpc(FLOAT64*,FLOAT64*,INT16);                                  /* PARCOR to LPC                     */
INT16 dlm_mcep2lpc  (FLOAT64*, INT16, FLOAT64*, INT16, FLOAT64);                /* Mel-cepstrum to LPC               */

/* Functions - dlm_lsf.c */
INT16   dlm_poly2lsf            (FLOAT64*,FLOAT64*,INT16);                      /* LSF transform                     */
INT16   dlm_lsf2poly            (FLOAT64*,FLOAT64*,INT16);                      /* LSF back-transform                */
void    dlm_lsf_interpolate     (FLOAT64*,FLOAT64*,FLOAT64*,INT16,INT32,INT32); /* LSF interpolation                 */
void    dlm_lsf2lsp             (FLOAT64*,FLOAT64*,INT16);                      /* LSF-LSP transform                 */
INT16   dlm_lsf2mlsf            (FLOAT64*,FLOAT64*,INT16,FLOAT64);              /* LSF-MLSF transform                */
INT16   dlm_lsf2poly_filt       (FLOAT64*,INT16,FLOAT64*,INT16,FLOAT64**);      /* LSF back-transform via FIR filter */
INT16   dlm_mlsf2poly_filt      (FLOAT64*,INT16,FLOAT64*,INT16,FLOAT64,         /* MLSF back-transform via FIR filter*/
                                 FLOAT64**);                                    /* |                                 */
INT16   dlm_lsf_synthesize      (FLOAT64*,INT16,FLOAT64*,INT32,FLOAT64*,        /* LSF synthesis                     */
                                 FLOAT64**);                                    /* |                                 */
FLOAT64 dlm_lsf_synthesize_step (FLOAT64*,INT16,FLOAT64**,FLOAT64);
INT16   dlm_mlsf_synthesize     (FLOAT64*,INT16,FLOAT64*,INT32,FLOAT64,         /* Mel-LSF synthesis filter          */
                                 FLOAT64*,FLOAT64**);                           /* |                                 */
FLOAT64 dlm_mlsf_synthesize_step(FLOAT64**,INT16,FLOAT64* e,FLOAT64**,FLOAT64,
                                 FLOAT64);
/* Functions - dlm_lcq.c */
INT16 dlm_lcq_synthesize  (FLOAT64*,INT16,FLOAT64*,INT32,INT16,FLOAT64*,        /* LCQ synthesis                     */
                           FLOAT64**);                                          /* |                                 */
INT16 dlm_mlcq_synthesize (FLOAT64*,INT16,FLOAT64*,INT32,INT16,FLOAT64,FLOAT64*,/* Mel-LCQ synthesis                 */
                           FLOAT64**);                                          /* |                                 */

/* Functions - dlm_cep.c */
INT16 dlm_lpc2mcep(FLOAT64*, INT16, FLOAT64*, INT16, FLOAT64);                  /* Cepstrum conversion from LPC      */
INT16 dlm_calcmcep(FLOAT64*, INT32, FLOAT64*, INT16, FLOAT64, FLOAT64, INT16);  /* Cepstrum calculation              */
INT16 dlm_cep2mcep(FLOAT64*, INT32, FLOAT64*, INT32, FLOAT64, FLOAT64*);        /* Bilinear transformation of cep.   */
INT16 dlm_mcep2cep(FLOAT64*, INT32, FLOAT64*, INT32, FLOAT64, FLOAT64*);        /* Bilinear transformation of cep.   */
INT16 dlm_mcep2mcep(FLOAT64*,INT32,FLOAT64*,INT32,FLOAT64,FLOAT64,FLOAT64*);    /* Bilinear transformation of cep.   */
INT16 dlm_mcep2b(FLOAT64*, FLOAT64*, INT32, FLOAT64);                           /* Conversion ceps. to filter coeff. */
INT16 dlm_b2mcep(FLOAT64*, FLOAT64*, INT32, FLOAT64);                           /* Conversion filter coeff. to ceps. */
INT16 dlm_mcep_uels(FLOAT64*, INT32, FLOAT64*, INT32, FLOAT64, FLOAT64);        /* Cepstrum calculation using UELS   */
INT16 dlm_mcep_uels_sptk(FLOAT64*, INT32, FLOAT64*, INT32, FLOAT64, FLOAT64);   /* Cepstrum calculation using UELS   */
INT16 dlm_cep_synthesize(FLOAT64*, INT16, FLOAT64*, INT32,                      /* Mel-Cepstrum synthesis filter     */
                         INT16, FLOAT64*, FLOAT64**);                           /* |                                 */
INT16 dlm_mcep_synthesize(FLOAT64*, INT16, FLOAT64*, INT32, FLOAT64,            /* Mel-Cepstrum synthesis filter     */
                         INT16, FLOAT64*, FLOAT64**);                           /* |                                 */
INT16 dlm_mcep_synthesize_mlsadf(FLOAT64*, INT16, FLOAT64*, INT32, FLOAT64,     /* Mel-Cepstrum synthesis filter     */
                              INT16, FLOAT64*, FLOAT64**);                      /* SPTK mlsadf implementation        */
INT16 dlm_mcep_enhance(FLOAT64*, FLOAT64*, INT16, FLOAT64);                     /* Cepstrum enhancement (postfilter) */
INT16 dlm_cep_quantize(FLOAT64*, INT16, INT16);                                 /* Cepstrum quantize and upscale     */
INT16 dlm_cep_dequantize(FLOAT64*, INT16, INT16);                               /* Cepstrum downscale and dequantize */

/* Functions - dlm_gcep.c */
INT16 dlm_mgcep      (FLOAT64*, INT32, FLOAT64*, INT16, FLOAT64, FLOAT64, FLOAT64);
INT16 dlm_gmult      (FLOAT64*, FLOAT64*, INT32, FLOAT64);
INT16 dlm_igmult     (FLOAT64*, FLOAT64*, INT32, FLOAT64);
INT16 dlm_gnorm      (FLOAT64*, FLOAT64*, INT32, FLOAT64);
INT16 dlm_ignorm     (FLOAT64*, FLOAT64*, INT32, FLOAT64);
INT16 dlm_gcep2gcep  (FLOAT64*, INT32, FLOAT64, FLOAT64*, INT32, FLOAT64);
INT16 dlm_mgcep2mgcep(FLOAT64*, INT32, FLOAT64, FLOAT64, FLOAT64*, INT32, FLOAT64, FLOAT64);
INT16 dlm_gc2gc      (FLOAT64*, INT32, FLOAT64, FLOAT64*, INT32, FLOAT64);

/* Functions - dlm_pow.c */
INT64     dlm_log2_i(UINT64 n);
FLOAT64   dlm_pow2(INT64 n);
FLOAT64   dlm_pow(FLOAT64 x, FLOAT64 y);
COMPLEX64 dlm_powC(COMPLEX64 x, COMPLEX64 y);
FLOAT64   dlm_pow10(FLOAT64);
COMPLEX64 dlm_logC(COMPLEX64);
COMPLEX64 dlm_log_bC(COMPLEX64 b, COMPLEX64 z);
COMPLEX64 dlm_expC(COMPLEX64 z);

typedef struct {
  INT16 nPer;
  char stimulation;
  char dummy;
} PERIOD;

/* Functions - dlm_pitch.c */
INT16 dlm_pm_expand     (INT16*,   INT32, INT16**,   INT32*,  INT32                         );
INT16 dlm_pm_expand_impl(INT16*,   INT32, INT16**,   INT32*,  INT16, INT32                  );
INT16 dlm_pm_compress   (INT16*,   INT32, INT16*,    INT32                                  );
INT16 dlm_getExcPeriod  (INT16,    BOOL,  INT8,      FLOAT64, INT32, FLOAT64*               );
INT16 dlm_pm2exc        (INT16*,   INT32, FLOAT64**, INT32*,  INT32, BOOL,    INT8          );
INT16 dlm_pm2f0         (INT16*,   INT32, FLOAT64*,  INT32,   INT32                         );
INT16 dlm_f02pm         (FLOAT64*, INT32, INT16**,   INT32*,  INT32, INT32                  );
INT16 dlm_getF0Cepstrum (FLOAT64*, INT32, INT32,     INT32,   INT32*                        );
INT16 dlm_gcida         (FLOAT64*, INT32, PERIOD**,  INT32*,  INT32, INT32,   INT32,   INT32);

/* Functions - dlm_ldl.c */
INT16 dlm_factldl(FLOAT64 *l,FLOAT64 *d,INT32 N);
INT16 dlm_factldlC(COMPLEX64 *l,COMPLEX64 *d,INT32 N);

/* Functions - dlm_lud.c */
INT16 dlm_ludcmp(FLOAT64*, INT32, INT32*, FLOAT64*);                            /* LU decomposition                  */
INT16 dlm_ludcmpC(COMPLEX64*, INT32, INT32*, COMPLEX64*);                       /* LU decomposition (complex version)*/
void  dlm_lubksb(FLOAT64*, INT32, INT32*, FLOAT64*);                            /* Solve AX=B                        */
void  dlm_lubksbC(COMPLEX64*, INT32, INT32*, COMPLEX64*);                       /* Solve AX=B       (complex version)*/
INT16 dlm_solve_lud(FLOAT64*, INT32, FLOAT64*, INT32);                          /* Solve AX=B                        */
INT16 dlm_solve_ludC(COMPLEX64*, INT32, COMPLEX64*, INT32);                     /* Solve AX=B       (complex version)*/
INT16 dlm_det_lud(FLOAT64* A, INT32 n, FLOAT64* d);                             /* Determinant                       */
INT16 dlm_det_ludC(COMPLEX64* A, INT32 n, COMPLEX64* d);                        /* Determinant      (complex version)*/

/* Functions - dlm_fwt.c */
const FLOAT64* dlm_fwt_geth(INT16 di);                                          /* Get wavelet coefficients          */
INT16 dlm_fwt_dx(FLOAT64* sig, FLOAT64* trans, INT32 size, INT16 di, INT16 level); /* fast wavelet transform         */
INT16 dlm_fwt_dx_inv(FLOAT64* trans, FLOAT64* sig, INT32 size, INT16 di, INT16 level); /* inverse fast wavelet transf. */

/* Functions - dlm_vad.c */
struct dlm_vad_param {
  INT32        nPre;    /* Maximal number of frames before speech to mark as speech */
  INT32        nPost;   /* Number of frames after speech to mark as speech (must be less or equal nMinSi) */
  INT32        nMinSp;  /* Minimal speech length */
  INT32        nMinSi;  /* Minimal silence length */
  INT32        nMaxSp;  /* Maximal speech length */
};
struct dlm_vad_state {
  struct dlm_vad_param lpPVad;
  enum {VAD_SI,VAD_SP_PRE,VAD_SP,VAD_SP_POST} nState;
  INT32 nInState;       /* Number of frames in current state */
  INT32 nPre;           /* Number of frames to mark as speech before current speech segment */
  BOOL bVadSfa;         /* Current output */
  INT32 nChange;        /* Change output in nChange frames (<0: change to silence, >0: change to speech) */
  INT32 nDelay;         /* Output delay (= MAX(nMinDelay, nPre+nMinSp, nMinSi) */
};
BOOL dlm_vad_single_sigengF(FLOAT32  *lpSig,INT32 nLen,FLOAT32  nThr);
BOOL dlm_vad_single_sigengD(FLOAT64 *lpSig,INT32 nLen,FLOAT64 nThr);
BOOL dlm_vad_single_pfaengF(FLOAT32  *lpFrame,INT32 nDim,FLOAT32  nThr);
BOOL dlm_vad_single_pfaengD(FLOAT64 *lpFrame,INT32 nDim,FLOAT64 nThr);
void dlm_vad_initparam(struct dlm_vad_param *lpPVad);
void dlm_vad_initstate(struct dlm_vad_state *lpSVad,struct dlm_vad_param *lpPVad,INT32 nMinDelay);
BOOL dlm_vad_process(BOOL bVadPfa,struct dlm_vad_state *lpSVad);

/* Functions - dlm_rand.c */
FLOAT64 dlm_rand_gauss_polar(void);     /* Gaussian distributed random values (Polar-Method)     */
FLOAT64 dlm_rand_gauss_bm(void);        /* Gaussian distributed random values (Box-Muller-Method)*/

/* Functions - dlm_dtw.c */
INT16 dlm_dtwC(COMPLEX64*, INT32, COMPLEX64*, INT32, INT32, INT32*, INT32*, FLOAT64*);

/* Functions - dlm_mlt.c */
INT16 dlm_mlt(FLOAT64*, FLOAT64*, INT32, INT32);                                      /* Modulated Lapped Transform */
INT16 dlm_imlt(FLOAT64*, FLOAT64*, INT32, INT32);                             /* Inverse Modulated Lapped Transform */

/* Functions - dlm_trig.c */
FLOAT32 dlm_sinus(FLOAT32);                                                              /* sinus by table lookup    */
FLOAT32 dlm_cosinus(FLOAT32);                                                            /* cosinus by table lookup  */

/* Functions - dlm_vq.c */
INT16 CGEN_PUBLIC dlm_svq(FLOAT64*,INT32,INT32,INT32*,INT32, BYTE*,INT32*,      /* Scalar Vector Quantization        */
                          FLOAT64*,INT32*);                                     /* |                                 */
INT16 CGEN_PUBLIC dlm_isvq(FLOAT64*,INT32,INT32,BYTE*,INT32,INT32,INT32*,INT32, /* Inverse Scalar Vector Quantization*/
                          FLOAT64*);                                            /* |                                 */
INT16 dlm_pam(FLOAT64*,INT32,INT32,FLOAT64*,INT32 nRQ);                         /* Clustering via PAM-Algorithm      */
INT16 dlm_lbg(FLOAT64*,INT32,INT32,FLOAT64*,INT32 nRQ);                         /* Wrapper for SPTK-LBG-Algorithm    */
INT16 dlm_vq(FLOAT64*,INT32,FLOAT64*,INT32,INT32,INT32*);                       /* Wrapper for SPTK-VQ-Algorithm     */
INT16 dlm_ivq(FLOAT64*,INT32,FLOAT64*,INT32,INT32,INT32*);                      /* Wrapper for SPTK-IVQ-Algorithm    */

/* Undocumented functions */                                                    /* --------------------------------- */
INT16 dlm_constant(FLOAT64* Z, INT32 nXRz, INT32 nXCz, INT16 nOpcode);          /* in dlm_arith.c                    */
INT16 dlm_scalop(FLOAT64* Z, const FLOAT64* A, INT32 nXR, INT32 nXC, FLOAT64 x, /* in dlm_arith.c                    */
                 INT16 nOpcode, INT16 bLeft);                                   /* |                                 */
INT16 dlm_elemop(FLOAT64* Z, const FLOAT64* A, INT32 nXRa, INT32 nXCa,          /* in dlm_arith.c                    */
                 const FLOAT64* B, INT32 nXRb, INT32 nXCb, INT16 nOpcode);      /* |                                 */
INT16 dlm_vectop(FLOAT64* Z, const FLOAT64* A, INT32 nXR, INT32 nXC,            /* in dlm_arith.c                    */
                 const FLOAT64* B, INT16 nOpcode, INT16 bLeft, INT16 nInverse); /* |                                 */
INT16 dlm_diagop(FLOAT64* Z, const FLOAT64* A, INT32 nXD, FLOAT64 x,            /* in dlm_arith.c                    */
                 INT16 nOpcode, INT16 bLeft);                                   /* |                                 */
INT16 dlm_diag(FLOAT64* Z, const FLOAT64* A, INT32 nXR, INT32 nXC);             /* in dlm_arith.c                    */
INT16 dlm_transpose(FLOAT64* Z, const FLOAT64* A, INT32 nXR, INT32 nXC);        /* in dlm_arith.c                    */
INT16 dlm_transposeC(COMPLEX64* Z, const COMPLEX64* A, INT32 nXR, INT32 nXC);   /* in dlm_arith.c                    */
INT16 dlm_mult(FLOAT64* Z,  const FLOAT64* A, INT32 nXRa, INT32 nXCa,           /* in dlm_arith.c                    */
               const FLOAT64* B, INT32 nXRb, INT32 nXCb);                       /* |                                 */
INT16 dlm_mult_kron(FLOAT64* Z,const FLOAT64* A, INT32 nXRa, INT32 nXCa,        /* in dlm_arith.c                    */
                    const FLOAT64* B, INT32 nXRb, INT32 nXCb);                  /* |                                 */
INT16 dlm_mult_akat(FLOAT64* Z, const FLOAT64* A, INT32 nXR, INT32 nXC,         /* in dlm_arith.c                    */
                    const FLOAT64* K);                                          /* |                                 */
INT16 dlm_intpower(FLOAT64* Z, const FLOAT64* A, INT32 nXD, UINT32 nPower);     /* in dlm_arith.c                    */

#ifdef __cplusplus                                                              /* C++ compiler?                     */
}                                                                               /* << Enable C linkage               */
#endif                                                                          /* #ifdef __cplusplus                */
#endif                                                                          /* #if !defined __DLPMATH_H          */

/* EOF */
