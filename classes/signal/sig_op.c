/* dLabPro class CSignal (signal)
 * - Operator method
 *
 * AUTHOR : strecha
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

#include "dlp_cscope.h"                                                         /* Indicate C scope                  */
#include "dlp_signal.h"
#include "dlp_base.h"


/**
 * <p>Wrapper for signal operations</p>
 *
 * @param X   Input data instance.
 * @param R   Pointer to output instance(s).
 * @param P   Pointer to input instance(s).
 * @return <code>O_K</code> if successful, a (negative) error code otherwise.
 */
INT16 CGEN_PUBLIC CSignal_Op(INT16 nOpc, StkItm* R, StkItm* P) {
  INT16 nErr = O_K;

  switch(nOpc) {
  case FOP_APS         :                                                                                                                                                                                                    break; /* Auto-power spectrum               */
  case FOP_CCF         :                                                                                                                                                                                                    break; /* Cross correlation function        */
  case FOP_CEP2LPC     : nErr = CSignal_Cep2Lpc     ((CData*)R[1].val.i,(CData*)R[0].val.i,                                                                                    (CData*)P[1].val.i,    (INT32)P[0].val.n.x); break; /* Cepstrum to LPC transform         */
  case FOP_CEP2MCEP    : nErr = CSignal_Cep2MCep    (                   (CData*)R[0].val.i,                                                               (CData*)P[2].val.i,          P[1].val.n.x,  (INT32)P[0].val.n.x); break; /* Cepstrum to M-Cepstrum transform  */
  case FOP_CEP         : nErr = CSignal_Cep         (                   (CData*)R[0].val.i,                                                                                    (CData*)P[1].val.i,    (INT32)P[0].val.n.x); break; /* Cesptrum                          */
  case FOP_DEFRAME     : nErr = CSignal_DeFrame     (                   (CData*)R[0].val.i,                                                                                    (CData*)P[1].val.i,    (INT32)P[0].val.n.x); break; /* De-Framing                        */
  case FOP_DENOISE     : nErr = CSignal_Denoise     (                   (CData*)R[0].val.i,                                          (CData*)P[3].val.i,   (INT32)P[2].val.n.x,        P[1].val.n.x,         P[0].val.s  ); break; /* Denoising                         */
  case FOP_DESCALE     : nErr = CSignal_DeScale     (                   (CData*)R[0].val.i,                                                                                                          (CData*)P[0].val.i  ); break; /* De-Scaling                        */
  case FOP_DISTRIBUTION: nErr = CSignal_Distribution(                   (CData*)R[0].val.i,                                                                                    (CData*)P[1].val.i,   (CData*)P[0].val.i  ); break; /* Value distribution                */
  case FOP_DTW         : nErr = CSignal_Dtw         (                   (CData*)R[0].val.i,                                                                                    (CData*)P[1].val.i,   (CData*)P[0].val.i  ); break; /* Dynamic Time Warping              */
  case FOP_F02EXC      : nErr = CSignal_F02Exc      (                   (CData*)R[0].val.i,                                          (CData*)P[3].val.i,   (INT32)P[2].val.n.x, (INT32)P[1].val.n.x,         P[0].val.s  ); break; /* Get excitation from F0 contour    */
  case FOP_FFT         : nErr = CSignal_Fft         (                   (CData*)R[0].val.i,                                                                                                          (CData*)P[0].val.i  ); break; /* Fast Fourier Transform            */
  case FOP_FILTER      : nErr = CSignal_Filter      (                   (CData*)R[0].val.i,                                          (CData*)P[3].val.i,  (CData*)P[2].val.i,  (CData*)P[1].val.i,   (CData*)P[0].val.i  ); break; /* Filtering                         */
  case FOP_FIR         : nErr = CSignal_Fir         (                   (CData*)R[0].val.i,                                                               (CData*)P[2].val.i,  (CData*)P[1].val.i,   (CData*)P[0].val.i  ); break; /* Finite Impulse Response filter    */
  case FOP_FRAME       : nErr = CSignal_Frame       (                   (CData*)R[0].val.i,                                                               (CData*)P[2].val.i,   (INT32)P[1].val.n.x,  (INT32)P[0].val.n.x); break; /* Framing                           */
  case FOP_SFRAME      : nErr = CSignal_Sframe      (                   (CData*)R[0].val.i,                     (CData*)P[4].val.i,   (INT32)P[3].val.n.x, (INT32)P[2].val.n.x,(CData*)P[1].val.i,    (INT32)P[0].val.n.x); break; /* Synchron framing                  */
  case FOP_GCEP        : nErr = CSignal_GCep        (                   (CData*)R[0].val.i,                                                               (CData*)P[2].val.i,          P[1].val.n.x,  (INT32)P[0].val.n.x); break; /* Generalized Cepstrum              */
  case FOP_GCEP2GCEP   : nErr = CSignal_GCep2GCep   (                   (CData*)R[0].val.i,                                                               (CData*)P[2].val.i,          P[1].val.n.x,  (INT32)P[0].val.n.x); break; /* Generalized Cepstrum transform    */
  case FOP_GCEP2LPC    : nErr = CSignal_GCep2Lpc    ((CData*)R[1].val.i,(CData*)R[0].val.i,                                                                                    (CData*)P[1].val.i,    (INT32)P[0].val.n.x); break; /* GCEP to LPC transform             */
  case FOP_GCEP2MLPC   : nErr = CSignal_GCep2MLpc   ((CData*)R[1].val.i,(CData*)R[0].val.i,                                                               (CData*)P[2].val.i,          P[1].val.n.x,  (INT32)P[0].val.n.x); break; /* M-GCEP to M-LPC transform         */
  case FOP_GCEPNORM    : nErr = CSignal_GCepNorm    ((CData*)R[1].val.i,(CData*)R[0].val.i,                                                                                                          (CData*)P[0].val.i  ); break; /* Gain normalization                */
  case FOP_GETF0       : nErr = CSignal_GetF0       (                   (CData*)R[0].val.i,                                          (CData*)P[3].val.i, (INT32)P[2].val.n.x,   (INT32)P[1].val.n.x,         P[0].val.s  ); break; /* F0 Estimation                     */
  case FOP_GMULT       : nErr = CSignal_GMult       (                   (CData*)R[0].val.i,                                                                                                          (CData*)P[0].val.i  ); break; /* Multiply by gamma                 */
  case FOP_IFFT        : nErr = CSignal_IFft        (                   (CData*)R[0].val.i,                                                                                                          (CData*)P[0].val.i  ); break; /* Inverse Fast Fourier Transform    */
  case FOP_IGCEPNORM   : nErr = CSignal_IGCepNorm   (                   (CData*)R[0].val.i,                                                                                    (CData*)P[1].val.i,   (CData*)P[0].val.i  ); break; /* Inverse gain normalization        */
  case FOP_IGMULT      : nErr = CSignal_IGMult      (                   (CData*)R[0].val.i,                                                                                                          (CData*)P[0].val.i  ); break; /* Divide by gamma                   */
  case FOP_IIR         : nErr = CSignal_Iir         (                   (CData*)R[0].val.i,                                                               (CData*)P[2].val.i,  (CData*)P[1].val.i,   (CData*)P[0].val.i  ); break; /* Infinite Impluse Response filter  */
  case FOP_IMCEP       : nErr = CSignal_IMCep       (                   (CData*)R[0].val.i,                                                               (CData*)P[2].val.i,  (CData*)P[1].val.i,  (FLOAT64)P[0].val.n.x); break; /* Inverse Mel-Cepstrum              */
  case FOP_IMLT        : nErr = CSignal_IMlt        (                   (CData*)R[0].val.i,                                                                                                          (CData*)P[0].val.i  ); break; /* Inverse Modulated Lapped Transform*/
  case FOP_ISVQ        : nErr = CSignal_ISvq        (                   (CData*)R[0].val.i,                                                                                    (CData*)P[1].val.i,   (CData*)P[0].val.i  ); break; /* Inverse Scalar Vector Quantization*/
  case FOP_IVQ         : nErr = CSignal_IVq         (                   (CData*)R[0].val.i,                                                                                    (CData*)P[1].val.i,   (CData*)P[0].val.i  ); break; /* Inverse Vector Quantization       */
  case FOP_LPC         : nErr = CSignal_Lpc         ((CData*)R[1].val.i,(CData*)R[0].val.i,                                                               (CData*)P[2].val.i,   (INT32)P[1].val.n.x,         P[0].val.s  ); break; /* LPC                               */
  case FOP_LPC2CEP     : nErr = CSignal_Lpc2Cep     (                   (CData*)R[0].val.i,                                                               (CData*)P[2].val.i,  (CData*)P[1].val.i,    (INT32)P[0].val.n.x); break; /* LPC to Cepstrum transform         */
  case FOP_LPC2GCEP    : nErr = CSignal_Lpc2GCep    (                   (CData*)R[0].val.i,                                          (CData*)P[3].val.i,  (CData*)P[2].val.i,          P[1].val.n.x,  (INT32)P[0].val.n.x); break; /* LPC to GCEP transform             */
  case FOP_LPC2MGCEP   : nErr = CSignal_Lpc2MGCep   (                   (CData*)R[0].val.i,                     (CData*)P[4].val.i,  (CData*)P[3].val.i,          P[2].val.n.x,        P[1].val.n.x,  (INT32)P[0].val.n.x); break; /* LPC to M-GCEP transform           */
  case FOP_LPC2MLPC    : nErr = CSignal_Lpc2MLpc    ((CData*)R[1].val.i,(CData*)R[0].val.i,                                          (CData*)P[3].val.i,  (CData*)P[2].val.i,          P[1].val.n.x,  (INT32)P[0].val.n.x); break; /* LPC to M-LPC transform            */
  case FOP_LSF2POLY    : nErr = CSignal_Lsf2Poly    (                   (CData*)R[0].val.i,                                                                                                          (CData*)P[0].val.i  ); break; /* LSF to Polynomial transform       */
  case FOP_MCEP2CEP    : nErr = CSignal_MCep2Cep    (                   (CData*)R[0].val.i,                                                                                    (CData*)P[1].val.i,    (INT32)P[0].val.n.x); break; /* M-Cepstrum to Cepstrum transform  */
  case FOP_MCEP2MCEP   : nErr = CSignal_MCep2MCep   (                   (CData*)R[0].val.i,                                                               (CData*)P[2].val.i,          P[1].val.n.x,  (INT32)P[0].val.n.x); break; /* M-Cepstrum to M-Cepstrum transform*/
  case FOP_MCEP2MLPC   : nErr = CSignal_MCep2MLpc   ((CData*)R[1].val.i,(CData*)R[0].val.i,                                                               (CData*)P[2].val.i,          P[1].val.n.x,  (INT32)P[0].val.n.x); break; /* M-GCEP to M-LPC transform         */
  case FOP_MCEP        : nErr = CSignal_MCep        (                   (CData*)R[0].val.i,                                                               (CData*)P[2].val.i,          P[1].val.n.x,  (INT32)P[0].val.n.x); break; /* M-Cepstrum                        */
  case FOP_MCEPENHANCE : nErr = CSignal_MCepEnhance (                   (CData*)R[0].val.i,                                                                                                          (CData*)P[0].val.i  ); break; /* M-Cepstrum enhancement            */
  case FOP_MFB         : nErr = CSignal_MFb         (                   (CData*)R[0].val.i,                                          (CData*)P[3].val.i,          P[2].val.n.x, (INT32)P[1].val.n.x,         P[0].val.s  ); break; /* MelFilter                         */
  case FOP_MFBS        : nErr = CSignal_MFbs        (                   (CData*)R[0].val.i,                                          (CData*)P[3].val.i,          P[2].val.n.x, (INT32)P[1].val.n.x,         P[0].val.s  ); break; /* MelFilter in spectral domain      */
  case FOP_MFFT        : nErr = CSignal_Mfft        (                   (CData*)R[0].val.i,                                                                                    (CData*)P[1].val.i  ,         P[0].val.n.x); break; /* Mel-Fast Fourier Transform        */
  case FOP_MFILTER     : nErr = CSignal_MFilter     (                   (CData*)R[0].val.i,                     (CData*)P[4].val.i,  (CData*)P[3].val.i,  (CData*)P[2].val.i,          P[1].val.n.x, (CData*)P[0].val.i  ); break; /* Mel-Filtering                     */
  case FOP_MFIR        : nErr = CSignal_MFir        (                   (CData*)R[0].val.i,                                          (CData*)P[3].val.i,  (CData*)P[2].val.i,          P[1].val.n.x, (CData*)P[0].val.i  ); break; /* Mel-Filtering                     */
  case FOP_MGCEP       : nErr = CSignal_MGCep       (                   (CData*)R[0].val.i,                                          (CData*)P[3].val.i,          P[2].val.n.x,        P[1].val.n.x,  (INT32)P[0].val.n.x); break; /* Mel-Generalized Cepstrum          */
  case FOP_MGCEP2LPC   : nErr = CSignal_MGCep2Lpc   ((CData*)R[1].val.i,(CData*)R[0].val.i,                                                                                    (CData*)P[1].val.i,    (INT32)P[0].val.n.x); break; /* M-GCEP to M-LPC transform         */
  case FOP_MGCEP2MGCEP : nErr = CSignal_MGCep2MGCep (                   (CData*)R[0].val.i,                                          (CData*)P[3].val.i,          P[2].val.n.x,        P[1].val.n.x,  (INT32)P[0].val.n.x); break; /* Mel-Generalized Cepstrum transform*/
  case FOP_MGCEP2MLPC  : nErr = CSignal_MGCep2MLpc  ((CData*)R[1].val.i,(CData*)R[0].val.i,                                                               (CData*)P[2].val.i,          P[1].val.n.x,  (INT32)P[0].val.n.x); break; /* M-GCEP to M-LPC transform         */
  case FOP_MIIR        : nErr = CSignal_MIir        (                   (CData*)R[0].val.i,                                          (CData*)P[3].val.i,  (CData*)P[2].val.i,          P[1].val.n.x, (CData*)P[0].val.i  ); break; /* Mel-Filtering                     */
  case FOP_MLPC        : nErr = CSignal_MLpc        ((CData*)R[1].val.i,(CData*)R[0].val.i,                                          (CData*)P[3].val.i,          P[2].val.n.x, (INT32)P[1].val.n.x,         P[0].val.s  ); break; /* M-LPC                             */
  case FOP_MLPC2GCEP   : nErr = CSignal_MLpc2GCep   (                   (CData*)R[0].val.i,                                          (CData*)P[3].val.i,  (CData*)P[2].val.i,          P[1].val.n.x,  (INT32)P[0].val.n.x); break; /* M-LPC to GCEP transform           */
  case FOP_MLPC2LPC    : nErr = CSignal_MLpc2Lpc    ((CData*)R[1].val.i,(CData*)R[0].val.i,                                                               (CData*)P[2].val.i,  (CData*)P[1].val.i,    (INT32)P[0].val.n.x); break; /* M-LPC to LPC transform            */
  case FOP_MLPC2MCEP   : nErr = CSignal_MLpc2MCep   (                   (CData*)R[0].val.i,                                          (CData*)P[3].val.i,  (CData*)P[2].val.i,          P[1].val.n.x,  (INT32)P[0].val.n.x); break; /* M-LPC to M-CEP transform          */
  case FOP_MLPC2MGCEP  : nErr = CSignal_MLpc2MGCep  (                   (CData*)R[0].val.i,                     (CData*)P[4].val.i,  (CData*)P[3].val.i,          P[2].val.n.x,        P[1].val.n.x,  (INT32)P[0].val.n.x); break; /* M-LPC to M-GCEP transform         */
  case FOP_MLPC2MLPC   : nErr = CSignal_MLpc2MLpc   ((CData*)R[1].val.i,(CData*)R[0].val.i,                                          (CData*)P[3].val.i,  (CData*)P[2].val.i,          P[1].val.n.x,  (INT32)P[0].val.n.x); break; /* M-LPC to M-LPC transform          */
  case FOP_MLSF2MLSF   : nErr = CSignal_MLsf2MLsf   (                   (CData*)R[0].val.i,                                                                                    (CData*)P[1].val.i,  (FLOAT64)P[0].val.n.x); break; /* M-LSF to M-LSF transform          */
  case FOP_MLT         : nErr = CSignal_Mlt         (                   (CData*)R[0].val.i,                                                                                                          (CData*)P[0].val.i  ); break; /* Modulated Lapped Transform        */
  case FOP_NOISIFY     : nErr = CSignal_Noisify     (                   (CData*)R[0].val.i,                                                                                                          (CData*)P[0].val.i  ); break; /* Signal noisifying                 */
  case FOP_PITCHMARK   : nErr = CSignal_Pitchmark   (                   (CData*)R[0].val.i,                     (CData*)P[4].val.i,          P[3].val.s,    (INT32)P[2].val.n.x,(INT32)P[1].val.n.x,  (INT32)P[0].val.n.x); break; /* Pitch marking                     */
  case FOP_POLY2LSF    : nErr = CSignal_Poly2Lsf    (                   (CData*)R[0].val.i,                                                                                                          (CData*)P[0].val.i  ); break; /* Polynomial to LSF transform       */
  case FOP_RMDC        : nErr = CSignal_Rmdc        (                   (CData*)R[0].val.i,                                                                                                          (CData*)P[0].val.i  ); break; /* Remove DC                         */
  case FOP_ROOTS       : nErr = CSignal_Roots       (                   (CData*)R[0].val.i,                                                                                                          (CData*)P[0].val.i  ); break; /* Roots of polynomial               */
  case FOP_SCALE       : nErr = CSignal_Scale       (                   (CData*)R[0].val.i,                                                                                    (CData*)P[1].val.i,           P[0].val.n  ); break; /* Scaling                           */
  case FOP_SVQ         : nErr = CSignal_Svq         ((CData*)R[1].val.i,(CData*)R[0].val.i,                                                                                    (CData*)P[1].val.i,   (CData*)P[0].val.i  ); break; /* Scalar Vector Quantization        */
  case FOP_WINDOW      : nErr = CSignal_Window      (                   (CData*)R[0].val.i,                     (CData*)P[4].val.i, (INT32)P[3].val.n.x,   (INT32)P[2].val.n.x,        P[1].val.s,           P[0].val.b  ); break; /* Windowing                         */
  case FOP_VQ          : nErr = CSignal_Vq          ((CData*)R[1].val.i,(CData*)R[0].val.i,                                                               (CData*)P[2].val.i,   (INT32)P[1].val.n.x,         P[0].val.s  ); break; /* Vector Quantization               */
  case FOP_UNWRAP      : nErr = CSignal_Unwrap      (                   (CData*)R[0].val.i,                                                                                                          (CData*)P[0].val.i  ); break; /* Unwrap phase                      */
  case FOP_ZCR         : nErr = CSignal_Zcr         (                   (CData*)R[0].val.i,                                                                                    (CData*)P[1].val.i,   (CData*)P[0].val.i  ); break; /* Zeroc crossing                    */
  default:
    CERROR(CSignal,FOP_ERR_NOTSUPP,dlp_sigop_sym(nOpc),0,0);
  }

  IF_NOK(nErr) {
    CERROR(CSignal,FOP_ERR_FAIL,dlp_sigop_sym(nOpc),0,0);
  }
  return nErr;
}
