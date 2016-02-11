/* dLabPro class CSignal (signal)
 * - Interactive methods
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
#include "dlp_matrix.h"

/**
 * <p>Cepstrum to LPC transform.</p>
 *
 * <p>This operation transforms Cepstrum coefficients to <code>n</code> Linear Predictive Coding coefficients.
 *
 * @param idA Output data instance.
 * @param idG Square root of estimation of the variance of the residual signal.
 * @param idC Input data instance.
 * @param n Number of LPC coefficients to calculate.
 */
INT16 CGEN_PUBLIC CSignal_Cep2Lpc(CData* idA, CData* idG, CData* idC, INT32 n) {
  return CSignal_MCep2MLpc(idA, idG, idC, 0.0, n);
}

/**
 * <p>Cepstrum to Mel-Cepstrum transform.</p>
 *
 * <p>This operation transforms Cepstrum coefficients to <code>n</code> Mel-Cepstrum coefficients with the given target
 * warping factor &lambda;.
 *
 * @param idC2 Output data instance.
 * @param idC1 Input data instance.
 * @param nLambda2 Target warping factor.
 * @param n Number of Mel-Cepstrum coefficients to calculate.
 */
INT16 CGEN_PUBLIC CSignal_Cep2MCep(CData* idC2, CData* idC1, FLOAT64 nLambda2, INT32 n) {
  return CSignal_MCep2MCep(idC2, idC1, nLambda2, n);
}

/**
 * <p>Cepstrum analysis.</p>
 *
 * <p>This operation calculates <code>nCoeff</code> Cepstrum coefficients from each frame given in the records of
 * <code>idX</code> using method Unbiased Estimator of Log-Spectrum (UELS) method.</p>
 *
 * @param idY Output data instance.
 * @param idX Input data instance.
 * @param nCoeff Number of Cepstrum coefficients to calculate.
 *
 */
INT16 CGEN_PUBLIC CSignal_Cep(CData* idY, CData* idX, INT32 nCoeff) {
  return CSignal_MCep(idY, idX, 0.0, nCoeff);
}

/**
 * <p>Inverse operation of {@link CSignal_Frame}.</p>
 *
 * <p>This operation concatenates the input frames cutting each frame at <code>nLen</code>. If <code>nLen&le;0</code>
 * the frames will not be cutted. Each block of the input data instance <code>idX</code> containing the frames produces
 * one component at the output data instance <code>idY</code>.</p>
 *
 * @param idY The output data instance.
 * @param idX The input data instance.
 * @param nLen Amount of samples taken from each frame.
 * @return <code>O_K</code>
 *
 * @see CSignal_Frame
 *
 */
INT16 CGEN_PUBLIC CSignal_DeFrame(CData* idY, CData* idX, INT32 nLen) {
  INT32 i, j, k, l, m;
  INT32 nBS = 0;
  INT32 nBRS = 0;
  INT32 nRS = 0;
  INT32 nCS = 0;
  INT32 nCL = 0;
  INT32 nRR = 0;
  INT32 nL = 0;
  INT32 nW = 0;
  INT32 iW = 0;
  FLOAT64 nC = 0;
  FLOAT64 nS = 0;
  CData* idS = NULL;
  CData* idL = NULL;
  CData* idR = NULL;
  COMPLEX64* pW = NULL;

  FOP_PRECALC(idX, idY, idS, idR, idL);

  nBS = CData_GetNBlocks(idS);
  nBRS = CData_GetNRecsPerBlock(idS);
  nRS = CData_GetNRecs(idS);
  nCS = CData_GetNComps(idS);
  nCL = CData_GetNComps(idL);
  nC = CData_GetDescr(idS,RWID);
  nS = idS->m_nCinc;
  if (nLen <= 0) nLen = nCS;
  nRR = nLen * nBRS;
  nL = MIN(nLen,nCS);
  nW = MIN(MAX(0,(INT32)(nC/nS+0.5)-nLen),nLen);

  pW = (COMPLEX64*) dlp_calloc(2*nW,sizeof(COMPLEX64));
  if ((nW > 0) && !pW) return ERR_MEM;

  for (iW = 0; iW < nW; iW++) {
    (pW + iW)->x = cos((iW+1) * F_PI / (2 * nW));
    (pW + nW + iW)->x = 1.0 - (pW + iW)->x;
  }

  CData_AddNcomps(idR, CData_GetCompType(idS, 0), nBS);
  for (k = 0; k < CData_GetNComps(idL); k++)
    CData_AddComp(idR, CData_GetCname(idL, k), CData_GetCompType(idL, k));
  CData_Allocate(idR, nRR);
  CData_SetNBlocks(idR, 1);

  for (k = 0; k < nRS; k++) {
    j = k / nBRS;
    l = k % nBRS;
    if (k > 0) {
      for (i = 0; i < nW; i++) {
        COMPLEX64 nV = CMPLX_PLUS(CMPLX_MULT(CData_Cfetch(idS, k, i), *(pW+nW+i)), CMPLX_MULT(CData_Cfetch(idS, k-1, i+nL), *(pW+i)));
        CData_Cstore(idR, nV, l * nLen + i, j);
        for (m = 0; m < nCL; m++) {
          CData_Sstore(idR, CData_Sfetch(idL, k, m), l * nLen + i, nBS + m);
        }
      }
      for (; i < nL; i++) {
        CData_Cstore(idR, CData_Cfetch(idS, k, i), l * nLen + i, j);
        for (m = 0; m < nCL; m++) {
          CData_Sstore(idR, CData_Sfetch(idL, k, m), l * nLen + i, nBS + m);
        }
      }
    } else {
      for (i = 0; i < nL; i++) {
        CData_Cstore(idR, CData_Cfetch(idS, 0, i), l * nLen + i, j);
        for (m = 0; m < nCL; m++) {
          CData_Sstore(idR, CData_Sfetch(idL, 0, m), l * nLen + i, nBS + m);
        }
      }
    }
  }

  if(pW) dlp_free(pW);
  CData_Reset(BASEINST(idL), FALSE);
  FOP_POSTCALC(idX, idY, idS, idR, idL);

  CData_SetDescr(idY,RINC,CData_GetDescr(idX,RINC) / nLen);
  CData_SetDescr(idY,RWID,CData_GetDescr(idX,RWID));

  return O_K;
}

/**
 * <p>Inverse operation of {@link CSignal_Scale}.</p>
 * This operation reads <code>idX.nScale</code> and undo scaling. Furthermore it converts the data instance to the type
 * given in the field <code>idX.nType</code>. This does nothing if the corresponding fields not exist.</p>
 *
 * @param idX Input data instance.
 * @param idY Output data instance.
 * @return    <code>O_K</code>
 * @see CSignal_Scale
 *
 */
INT16 CGEN_PUBLIC CSignal_DeScale(CData* idY, CData* idX) {
  COMPLEX64 nScale = CMPLX(0);
  COMPLEX64 nQuant = CMPLX(0);

  if (OK(CSignal_GetVar(idX,"nScale",&nScale)) && !CMPLX_EQUAL(nScale, CMPLX(0))) {
    CSignal_ScaleImpl(idY, idX, dlp_scalopC(nScale, CMPLX(0), OP_INVT));
    CSignal_SetVar(idY, "nScale", CMPLX(1) );
  } else {
    CData_Copy(BASEINST(idY), BASEINST(idX));
  }
  if (OK(CSignal_GetVar(idX,"nType",&nQuant)) && !CMPLX_EQUAL(nQuant, CMPLX(0))) {
    CMatrix_Op(idY, idY, T_INSTANCE, NULL, T_IGNORE, OP_ROUND);
    CData_Tconvert(idY, idY, (INT16) nQuant.x);
  }
  return O_K;
}

/**
 * <p>Amplitude distribution</p>
 * This operation counts the occurrence of amplitude values of the (complex) samples <code>idX</code>
 * falling in the intervals given in <code>idP</code>. The output <code>idY</code> are complex values
 * where the real part contains the counts for the real part of <code>idX</code> and the imaginary part
 * the counts for the imaginary part of <code>idY</code>.
 *
 * @param idY Output complex data instance
 * @param idX Input (complex) data instance containing the samples
 * @param idP Input data instance defining the intervals
 * @return <code>O_K</code>
 */
INT16 CGEN_PUBLIC CSignal_Distribution(CData* idY, CData* idX, CData* idP) {
  INT32 nCS = 0;
  INT32 nRS = 0;
  INT32 nCP = 0;
  CData* idR = NULL;
  CData* idS = NULL;
  CData* idL = NULL;
  INT16 nTS = -1;
  INT16 nTP = -1;

  FOP_PRECALC(idX, idY, idS, idR, idL);

  nCS = CData_GetNComps(idS);
  nRS = CData_GetNRecs(idS);
  nCP = CData_GetNComps(idP);
  nTS = CData_GetCompType(idS, 0);
  nTP = CData_GetCompType(idP, 0);

  if (!CData_IsEmpty(idS) && dlp_is_numeric_type_code(nTS)) {
    if (dlp_is_complex_type_code(nTS)) {
      CData_Tconvert(idP, idP, T_COMPLEX);
      CData_Array(idR, T_COMPLEX, nCP, nRS);
      dlm_distributionC((COMPLEX64*) CData_XAddr(idR, 0, 0), (COMPLEX64*) CData_XAddr(idS, 0, 0), (COMPLEX64*) CData_XAddr(idP, 0, 0), nRS, nCS, nCP);
    } else {
      CData_Tconvert(idS, idS, T_DOUBLE);
      CData_Tconvert(idP, idP, T_DOUBLE);
      CData_Array(idR, T_DOUBLE, nCP, nRS);
      dlm_distribution((FLOAT64*) CData_XAddr(idR, 0, 0), (FLOAT64*) CData_XAddr(idS, 0, 0), (FLOAT64*) CData_XAddr(idP, 0, 0), nRS, nCS, nCP);
      CData_Tconvert(idS, idS, nTS);
    }
    CData_Tconvert(idP, idP, nTP);
  }
  CData_SetNBlocks(idR, CData_GetNBlocks(idS));

  FOP_POSTCALC(idX, idY, idS, idR, idL);

  return O_K;
}

/**
 * <p> Dynamic Time Warping.</p>
 * This operation calculates the alignment path of the two input vector sequences <code>idS</code> and <code>idD</code>
 * by means of the minimum accumulated vector difference norm. The output consists of the index sequence related to
 * the vectors of <code>idS</code>, the corresponding index sequence related to vectors of <code>idD</code> and the
 * error at the corresponding point of the path.
 *
 * @param idS Input data instance containing the vector sequence of the source (reference).
 * @param idD Input data instance containing the vector sequence of the target.
 * @param idP Output data instance containing the alignment path and errors.
 * @return <code>O_K</code>
 *
 */
INT16 CGEN_PUBLIC CSignal_Dtw(CData* idP, CData* idS, CData* idD) {
  CData* idSR;
  CData* idDR;
  CData* idE;
  INT32 nRS;
  INT32 nCS;
  INT32 nRD;
  INT32 nCD;
  INT32 nRP;

  ICREATEEX(CData, idSR, "~src", NULL);
  ICREATEEX(CData, idDR, "~dst", NULL);
  CData_Select(idSR, idS, 0, CData_GetNNumericComps(idS));
  CData_Select(idDR, idD, 0, CData_GetNNumericComps(idD));
  CData_Tconvert(idSR, idSR, T_COMPLEX);
  CData_Tconvert(idDR, idDR, T_COMPLEX);

  nRS = CData_GetNRecs(idSR);
  nCS = CData_GetNComps(idSR);
  nRD = CData_GetNRecs(idDR);
  nCD = CData_GetNComps(idDR);
  nRP = nRS + nRD;

  DLPASSERT(nCS == nCD);

  ICREATEEX(CData, idE, "~tmp_idE", NULL);
  CData_Array(idE, T_DOUBLE, 1, nRP);
  CData_SetCname(idE, 0, "err");
  CData_Array(idP, T_INT, 2, nRP);
  CData_SetCname(idP, 0, "src");
  CData_SetCname(idP, 1, "dst");

  dlm_dtwC((COMPLEX64*) CData_XAddr(idSR, 0, 0), nRS, (COMPLEX64*) CData_XAddr(idDR, 0, 0), nRD, nCS, (INT32*) CData_XAddr(idP, 0, 0), &nRP, (FLOAT64*) CData_XAddr(idE, 0, 0));

  CData_Join(idP, idE);
  CData_Realloc(idP, nRP);

  IDESTROY(idE);
  IDESTROY(idSR);
  IDESTROY(idDR);

  return O_K;
}

/**
 * <p>Get framed excitation signal from F0 contour.</p>
 *
 * @param F  Input data instance containing F0 contour.
 * @param sT String for choosing excitation type.
 * @param nF Number of frames to produce.
 * @param nL frame length.
 * @param E  Output data instance containing excitation signal.
 * @return <code>O_K</code>
 *
 * <h4>Currently supported excitation types</h4>
 * <table>
 *   <tr><td><code>pulse<code></td>    <td>Mix of pulse train (voiced) and white noise (unvoiced)</td></tr>
 *   <tr><td><code>glott<code></td>    <td>Mix of glottal function (voiced) and white noise (unvoiced)</td></tr>
 *   <tr><td><code>randphase<code></td><td>Mix of pulse train (lower frequencies) and noise (higher frequencies)</td></tr>
 *   <tr><td><code>voiced<code></td>   <td>Pulse train</td></tr>
 *   <tr><td><code>unvoiced<code></td> <td>White noise</td></tr>
 *   <tr><td><code>ccepstrum<code></td><td>Complex cepstrum phase</td></tr>
 * </table>
 *
 * @see <a href="dlpmath.html#dlm_getExcPeriod"><code class="link">dlm_getExcPeriod</code></a>
 *
 */
INT16 CGEN_PUBLIC CSignal_F02Exc(CData* idE, CData* idF, INT32 nF, INT32 nL, const char* sT) {
  INT8 nT = -1;
  INT32 iF = 0;
  INT32 nSR = 0;
  INT32 nRF = 0;
  INT32 iRF = 0;
  INT32 iPP = 0;
  INT32 nBT = 0;
  INT32 nBF = 0;
  BOOL bV = TRUE;
  FLOAT64* excFrame = NULL;
  FLOAT64* excPeriod = NULL;
  COMPLEX64 nScale = CMPLX(1.0);
  CData* idR = NULL;
  CData* idS = NULL;
  CData* idL = NULL;

  DLPASSERT((idE != NULL) && (idF != NULL));

  nRF = CData_GetNRecs(idF);
  nSR = (INT32) (1.0 / idF->m_nCinc + 0.5);
  CSignal_GetVar(idF, "nScale", &nScale);

  FOP_PRECALC(idF, idE, idS, idR, idL);

  if (!dlp_strcmp(sT, "pulse")) nT = DLM_PITCH_PULSE;
  else if (!dlp_strcmp(sT, "glott")) nT = DLM_PITCH_GLOTT;
  else if (!dlp_strcmp(sT, "randphase")) nT = DLM_PITCH_RANDPHASE;
  else if (!dlp_strcmp(sT, "voiced")) nT = DLM_PITCH_VOICED;
  else if (!dlp_strcmp(sT, "unvoiced")) nT = DLM_PITCH_UNVOICED;
  else {
    CERROR(CSignal, FOP_ERR_INVARG, "sT", sT, "");
    return NOT_EXEC;
  }

  CData_AddNcomps(idR, T_DOUBLE, nL);
  CData_Allocate(idR, nF);
  excPeriod = (FLOAT64*) dlp_calloc(nSR*1000, sizeof(FLOAT64));
  if (!idR || !excPeriod) return ERR_MEM;

  for (iF = 0; iF < nF; iF++) {

    excFrame = (FLOAT64*) CData_XAddr(idR, iF, 0);

    iRF = (INT32) ((UINT64) iF * (UINT64) nRF / (UINT64) nF);

    dlp_memmove(excFrame, excPeriod + (nBT - iPP), MIN(nL,iPP) * sizeof(FLOAT64));

    nBF = CData_Dfetch(idS, iRF, 0);
    if (nBF < 0) {
      nBF = -nBF;/*MAX(0,nL - iPP);*/
      bV = FALSE;
    } else {
      bV = TRUE;
    }
    if (nBF == 0) {
      nBT = MAX(1,nL - iPP);
      bV = FALSE;
    } else {
      nBT = (INT32) (1000.0 * nSR / nBF + 0.5);
    }

    while (iPP < (nL - nBT)) {
      dlm_getExcPeriod(nBT, bV, nT, nScale.x, nSR * 1000, excFrame + iPP);
      iPP += nBT;
    }
    if (iPP < nL) {
      dlm_getExcPeriod(nBT, bV, nT, nScale.x, nSR * 1000, excPeriod);
      dlp_memmove(excFrame + iPP, excPeriod, (nL - iPP) * sizeof(FLOAT64));
      iPP = nBT - (nL - iPP);
    } else {
      iPP = 0;
    }
  }

  if (excPeriod != NULL) dlp_free(excPeriod);

  FOP_POSTCALC(idF, idE, idS, idR, idL);
  CData_SetDescr(idE,RWID,(nL/nSR));

  return O_K;
}

/**
 * <p>Discrete Fourier Transform.</p>
 *
 * @param idY Input data instance.
 * @param idX Output data instance
 * @return <code>O_K</code>
 *
 * <h4>Remarks</h4>
 * <ul>
 *   <li>The output <code>idY</code> will always of complex type.</li>
 *   <li>The input will be processed record wise.<\li>
 *   <li>If the size of one input record, i.e. amount of numeric columns, is power of two the FFT is called.</li>
 * </ul>
 *
 * @see <a href="dlpmath.html#dlm_dftC"><code class="link">dlm_dftC</code></a>
 * @see <a href="dlpmath.html#dlm_fft_C"><code class="link">dlm_fft_C</code></a>
 *
 */
INT16 CGEN_PUBLIC CSignal_Fft(CData* idY, CData* idX) {
  return CSignal_FftImpl(idY, idX, FALSE);
}

/**
 * <p>ARMA filtering. Filters signal frames (frames must given at rows of <code>X</code>)
 * using ARMA filter with coefficients <code>B</code> and <code>A</code> and filter states
 * <code>M</code> (can be <code>NULL</code>) and outputs the result to <code>Y</code>:</p>
 * <p>Y(z)=B(z)A<sup>-1</sup>(z)X(z).</p>
 * <p>If the number of records of <code>X</code> and <code>B</code> and <code>A</code>
 * are equal the <code>i<sup>th</sup></code> record of <code>X</code> will be filtered with filter coefficients of the
 * <code>i<sup>th</sup></code> record of <code>B</code> and <code>A</code>. If the number of records of <code>B</code>
 * and <code>A</code> equals <code>1</code> all records of <code>X</code> will be filtered with the coefficients given
 * in this single record of <code>B</code> and <code>A</code>.</p>
 * <img src="../resources/base/dlp_math/filter.gif" align="absmiddle">
 * <h4>Remarks</h4>
 * <ul>
 * <li>If <code>A</code>=<code>NULL</code> the filter is implemented as a FIR
 * (MA) filter, i.e. <code>A</code> is set to <code>1</code>.</li>
 * <li>If <code>B</code>=<code>NULL</code> the filter is implemented as a pure
 * IIR (AR) filter, i.e. <code>B</code> is set to <code>1</code>.</li>
 * <li><code>B</code> must be a column vector.</li>
 * <li><code>A</code> must be a column vector.</li>
 * <li>If a<sub>0</sub> is not equal to 1 then the result is normalized by
 * a<sub>0</sub></li>
 * <li>If filter states non-<code>NULL</code> then <code>M</code> must be a
 * column vector of size <code>MAX(length(B),length(A))-1</code>.</li>
 * </ul>
 *
 * @param Y Destination
 * @param X Source
 * @param B Vector of filter coefficients
 * @param A Vector of filter coefficients
 * @param M Filter states
 *
 */
INT16 CGEN_PUBLIC CSignal_Filter(CData* Y, CData* X, CData* B, CData* A, CData* M) {
  return CSignal_MFilter(Y, X, B, A, 0.0, M);
}

/**
 * <p>FIR filter</p>
 *
 * <p>This operation filters the input signals given in the records of the input data instance <code>X</code> with
 * the filter coefficients given in <code>B</code>. If the number of records of <code>X</code> and <code>B</code>
 * are equal the <code>i<sup>th</sup></code> record of <code>X</code> will be filtered with filter coefficients of the
 * <code>i<sup>th</sup></code> record of <code>B</code>. If the number of records of <code>B</code> equals
 * <code>1</code> all records of <code>X</code> will be filtered with the coefficients given in this single record
 * of <code>B</code>. If <code>M</code> is not <code>NULL</code> the filter states were not reset at consecutive
 * records but on consecutive blocks. Otherwise the filter states were reset at each new record.</p>
 * <img src="../resources/base/dlp_math/filter_fir.gif" align="absmiddle">
 *
 * @param Y Output data instance holding filtered signals.
 * @param X Input data instance holding signals to filter.
 * @param B Filter coefficients.
 * @param M Filter states (can be <code>NULL</code>).
 * @return Equal to {@link CSignal_MFir}.
 *
 * @see CSignal_MFir CSignal_Iir CSignal_MIir CSignal_Filter CSignal_MFilter
 */
INT16 CGEN_PUBLIC CSignal_Fir(CData* Y, CData* X, CData* B, CData* M) {
  return CSignal_MFir(Y, X, B, 0.0, M);
}

/**
 * <p>Splits the signal into frames.</p>
 *
 * <p>The input signal can have more than one channel. Each channel has to be one component of the input data instance.
 * The frames of the channels stored to blocks of the output data instance with one frame per record and
 * <code>nLen</code> columns plus possibly existent label components. Every <code>nStep</code> samples a frame of
 * <code>nLen</code> samples length is copied to one output frame. If <code>nStep &le; 0</code> the values will be set
 * to <code>nLen</code>.</p>
 *
 * @param idX   Input data instance containing the signal. The time axis of the signal is expected to be the records of
 *              the data object. If there are more than one channels (numeric columns) the output frames of every
 *              channel are written to blocks of the output data instance.
 * @param nLen  The desired frame lenght.
 * @param nStep The desired continuous rate.
 * @param idY   Output data instance.
 * @return      <code>O_K</code>
 *
 * @see CSignal_DeFrame
 *
 */
INT16 CGEN_PUBLIC CSignal_Frame(CData* idY, CData* idX, INT32 nLen, INT32 nStep) {
  INT32 i, j, k;
  INT32 nOff = 0;
  INT32 nRS = 0;
  INT32 nCS = 0;
  INT32 nCL = 0;
  INT32 nRR = 0;
  INT32 nCR = 0;
  INT32 nCRr = 0;
  CData* idS = NULL;
  CData* idL = NULL;
  CData* idR = NULL;

  if (nStep <= 0) nStep = nLen;

  FOP_PRECALC(idX, idY, idS, idR, idL);

  nRS = CData_GetNRecs(idS);
  nCS = CData_GetNComps(idS);
  nCL = CData_GetNComps(idL);
  nRR = nRS / nStep;
  if ((nRS % nStep) > 0) nRR++;
  nCR = nLen;

  CData_Clear(idR);
  CData_AddNcomps(idR, CData_GetCompType(idS, 0), nLen);
  for (k = 0; k < CData_GetNComps(idL); k++)
    CData_AddComp(idR, CData_GetCname(idL, k), CData_GetCompType(idL, k));
  CData_Allocate(idR, nRR * nCS);
  CData_SetNBlocks(idR, nCS);

  for (k = 0; k < nCS; k++) {
    nOff = 0;
    for (i = 0; i < nRR; i++) {
      nCRr = ((nOff + nCR) > nRS) ? nRS - nOff : nCR;
      for (j = 0; j < nCRr; j++)
        CData_Cstore(idR, CData_Cfetch(idS, nOff + j, k), nRR * k + i, j);
      for (j = 0; j < nCL; j++)
        CData_Sstore(idR, CData_Sfetch(idL, nOff + nCRr / 2, j), nRR * k + i, nLen + j);
      nOff += nStep;
    }
  }

  CData_Reset(BASEINST(idL), FALSE);
  FOP_POSTCALC(idX, idY, idS, idR, idL);

  idY->m_lpTable->m_fsr = idX->m_lpTable->m_fsr * nStep;
  idY->m_lpTable->m_zf = idX->m_lpTable->m_fsr * nLen;
  idY->m_nCinc = idX->m_lpTable->m_fsr;
  idY->m_nCofs = idX->m_lpTable->m_ofs;
  dlp_strcpy(idY->m_lpCunit, idX->m_lpRunit);

  return O_K;
}

/**
 * <p>Generalized Cepstrum  analysis.</p>
 *
 * <p>This operation calculates <code>nCoeff</code> Generalized Cepstrum coefficients from each frame given in the
 * records of <code>idX</code>.</p>
 *
 * @param idY Output data instance.
 * @param idX Input data instance.
 * @param nGamma Target &gamma; of G-Cepstrum.
 * @param nCoeff Number of G-Cepstrum-coefficients to calculate.
 * @see <a href="dlpmath.html#dlm_gcep"><code class="link">dlm_gcep</code></a>
 */
INT16 CGEN_PUBLIC CSignal_GCep(CData* idY, CData* idX, FLOAT64 nGamma, INT32 nCoeff) {
  return CSignal_MGCep(idY, idX, nGamma, 0.0, nCoeff);
}

/**
 * <p>Generalized Cepstrum transform</p>
 * <p>This operation transforms the Generalized Cepstrum to the Generalized Cepstrum with &gamma;<sub>2</sub>. The
 * Generalized Cepstrum is assumed to be in non-normalized form. This function is equal to call
 * {@link CSignal_MGCep2MGCep} with <code>nLambda2=0</code>.
 *
 * @param idY Output data instance (non-normalized Generalized Cepstrum).
 * @param idX Input data instance (non-normalized Generalized Cepstrum).
 * @param nGamma2 Generalized cepstrum factor &gamma; of output.
 * @param n Number of Mel-Generalized Cepstrum coeffizients of output.
 * @return <code>O_K</code>
 * @see <a href="dlpmath.html#dlm_gcep2gcep"><code class="link">dlm_gcep2gcep</code></a>
 */
INT16 CGEN_PUBLIC CSignal_GCep2GCep(CData* idY, CData* idX, FLOAT64 nGamma2, INT32 n) {
  return CSignal_MGCep2MGCep(idY, idX, nGamma2, 0.0, n);
}

/**
 * <p>Generalized Cepstrum to Linear Predictive Coding transform.</p>
 *
 * <p>This operation converts Generalized-Cepstrum coefficients to <code>n</code> LPC coefficients. This function calls
 * {@link CSignal_MGCep2MLpc} with <code>nLambda2=0</code>.
 *
 * @param idG Square root of estimation of the variance of the residual signal.
 * @param idY Output data instance.
 * @param idX Input data instance.
 * @param n Number of Mel-LPC-coefficients to calculate.
 */
INT16 CGEN_PUBLIC CSignal_GCep2Lpc(CData* idY, CData* idG, CData* idX, INT32 n) {
  return CSignal_MGCep2MLpc(idY, idG, idX, 0.0, n);
}

/**
 * <p>Generalized Cepstrum to Mel-Linear Predictive Coding transform.</p>
 *
 * <p>This operation converts Generalized-Cepstrum coefficients to <code>n</code> Mel-LPC coefficients with target
 * warping factor &lambda;. This function is equal to {@link CSignal_MGCep2MLpc}.
 *
 * @param idG Square root of estimation of the variance of the residual signal.
 * @param idY Output data instance.
 * @param idX Input data instance.
 * @param nLambda2 Target warping factor.
 * @param n Number of Mel-LPC-coefficients to calculate.
 */
INT16 CGEN_PUBLIC CSignal_GCep2MLpc(CData* idY, CData* idG, CData* idX, FLOAT64 nLambda2, INT32 n) {
  return CSignal_MGCep2MLpc(idY, idG, idX, nLambda2, n);
}

/**
 * <p>Gain normalization</p>
 * <p><table><tr><th>&gamma;</th><th>Corresponding feature</th></tr>
 * <tr><td><code>-1</code></td><td>(Mel-)Linear Predictive Coding (LPC)</td></tr>
 * <tr><td><code>-1&lt;&gamma;&lt;0</code></td><td>(Mel-)Generalized cepstrum</td></tr>
 * <tr><td><code>0</code></td><td>(Mel-)Cepstrum</td></tr></table></p>
 * @param idG Output square root of estimation of the variance of the residual signal.
 * @param idY Output data instance.
 * @param idX Input data instance.
 * @return <code>O_K</code>
 * @see <a href="dlpmath.html#dlm_gnorm"><code class="link">dlm_gnorm</code></a>
 */
INT16 CGEN_PUBLIC CSignal_GCepNorm(CData* idY, CData* idG, CData* idX) {
  INT32 iR = 0;
  INT32 nCS = 0;
  INT32 nRS = 0;
  COMPLEX64 nGamma = CMPLX(0.0);
  CData* idR = NULL;
  CData* idS = NULL;
  CData* idL = NULL;
  INT16 nTS = -1;

  DLPASSERT(idG!=NULL);

  CSignal_GetVar(idX, "nGamma", &nGamma);
  FOP_PRECALC(idX, idY, idS, idR, idL);

  nCS = CData_GetNComps(idS);
  nRS = CData_GetNRecs(idS);
  nTS = CData_GetCompType(idS, 0);

  CData_Tconvert(idS, idS, T_DOUBLE);
  CData_Array(idR, T_DOUBLE, nCS, nRS);
  CData_SetNBlocks(idR, CData_GetNBlocks(idS));

  for (iR = 0; iR < nRS; iR++)
    dlm_gnorm((FLOAT64*) CData_XAddr(idS, iR, 0), (FLOAT64*) CData_XAddr(idR, iR, 0), nCS, nGamma.x);

  CData_Select(idG, idR, 0, 1);
  CMatrix_CopyLabels(idG, idX);
  for (iR = 0; iR < nRS; iR++)
    CData_Dstore(idR, 1.0, iR, 0);

  CData_Tconvert(idS, idS, nTS);
  FOP_POSTCALC(idX, idY, idS, idR, idL);
  CSignal_SetVar(idY, "nGamma", nGamma);

  return O_K;
}

/**
 * <p>Calculates F0 from framed input signal.</p>
 *
 * <p>For each frame an F0 value (in Hz) will be calculated in the range of <code>[nMin,nMax]</code> using the algorithm given in <code>sTyp</code>. Currently supported algorithms are:
 * <ul><li>&quot;cepstrum&quot</li></ul></p>
 *
 * @param idY  Output F0
 * @param idX  Framed input signal
 * @param nMin Minimum F0
 * @param nMax Maximum F0
 * @param sTyp Algorithm used for calculation
 */
INT16 CGEN_PUBLIC CSignal_GetF0(CData* idY, CData* idX, INT32 nMin, INT32 nMax, const char* sTyp) {
  INT16 ret = O_K;
  INT16 nTS = -1;
  INT32 nC = 0;
  INT32 nR = 0;
  INT32 nW = 0;
  INT32 iR = 0;
  INT32 nMinT = 0;
  INT32 nMaxT = 0;
  INT32 nSR = 0;
  CData* idR = NULL;
  CData* idS = NULL;
  CData* idL = NULL;

  DLPASSERT((idY != NULL) && (sTyp != NULL) && (idX != NULL));

  if (nMax <= 0) nMax = 400;
  if (nMin <= 0) nMin = 80;

  nC = CData_GetNComps(idX);
  nR = CData_GetNRecs(idX);
  nSR = (INT32) (1.0 / idX->m_nCinc + 0.5);
  nW = CData_GetDescr(idX, RWID);
  nW = (nW <= 0) ? nC : nSR * nW;
  nMaxT = (INT32) (1000 * nSR / (FLOAT64) nMin + 0.5);
  nMinT = (INT32) (1000 * nSR / (FLOAT64) nMax + 0.5);
  CData_AddNcomps(idY, T_INT, 1);
  CData_Allocate(idY, nR);

  FOP_PRECALC(idX, idY, idS, idR, idL);

  nTS = CData_GetCompType(idS, 0);
  CData_Tconvert(idS, idS, T_DOUBLE);

  for (iR = 0; iR < nR; iR++) {
    if ((ret = dlm_getF0Cepstrum((FLOAT64*) CData_XAddr(idS, iR, 0), nW, nMinT, nMaxT, (INT32*) CData_XAddr(idR, iR, 0))) != O_K) break;
    if (*(INT32*) CData_XAddr(idR, iR, 0) > 0) {
      *(INT32*) CData_XAddr(idR, iR, 0) = (INT32) (1000.0 * nSR / *(INT32*) CData_XAddr(idR, iR, 0) + 0.5);
    } else if (*(INT32*) CData_XAddr(idR, iR, 0) < 0) {
      *(INT32*) CData_XAddr(idR, iR, 0) = -(INT32) (1000.0 * nSR / -*(INT32*) CData_XAddr(idR, iR, 0) + 0.5);
    }
  }

  CData_Tconvert(idS, idS, nTS);
  FOP_POSTCALC(idX, idY, idS, idR, idL);

  return O_K;
}

/**
 * <p>Multiplies (Mel-)Generalized Cepstrum coefficients by its &gamma;. If the input are (Mel-)LPC coefficients
 * <code>a<sub>k>0</sub></code> will be multiplied by <code>-1</code>. This is useful for getting (Mel-)LPC residual
 * signal using {@link CSignal_MFilter}.
 *
 * @param idY Output data instance.
 * @param idX Input data instance.
 * @return <code>O_K</code>
 */
INT16 CGEN_PUBLIC CSignal_GMult(CData* idY, CData* idX) {
  INT16 nT = -1;
  INT32 iR = 0;
  INT32 nC = 0;
  INT32 nR = 0;
  COMPLEX64 nGamma = CMPLX(0.0);

  nT = CData_GetCompType(idX, 0);
  CData_Tconvert(idX, idX, T_DOUBLE);

  CData_Copy(BASEINST(idY), BASEINST(idX));

  CSignal_GetVar(idY, "nGamma", &nGamma);
  nC = CData_GetNComps(idY);
  nR = CData_GetNRecs(idY);

  for (iR = 0; iR < nR; iR++) {
    dlm_gmult((FLOAT64*) CData_XAddr(idX, iR, 0), (FLOAT64*) CData_XAddr(idY, iR, 0), nC, nGamma.x);
  }

  CData_Tconvert(idX, idX, nT);

  return O_K;
}

/**
 * <p>Inverse Discrete Fourier Transform.</p>
 *
 * @see <a href="dlpmath.html#dlm_dftC"><code class="link">dlm_fft_C</code></a>
 *
 */
INT16 CGEN_PUBLIC CSignal_IFft(CData* idY, CData* idX) {
  return CSignal_FftImpl(idY, idX, TRUE);
}

/**
 * <p>Inverse gain normalization</p>
 * <p>This operation is the inverse of <code>gnorm</code>.
 * @param idY Output data instance.
 * @param idG Input square root of estimation of the variance of the residual signal.
 * @param idX Input data instance.
 * @return <code>O_K</code>
 * @see <a href="dlpmath.html#dlm_ignorm"><code class="link">dlm_ignorm</code></a>
 */
INT16 CGEN_PUBLIC CSignal_IGCepNorm(CData* idY, CData* idG, CData* idX) {
  INT32 iR = 0;
  INT32 nCS = 0;
  INT32 nRS = 0;
  COMPLEX64 nGamma = CMPLX(0.0);
  CData* idR = NULL;
  CData* idS = NULL;
  CData* idL = NULL;

  DLPASSERT(idG!=NULL);

  CSignal_GetVar(idX, "nGamma", &nGamma);
  FOP_PRECALC(idX, idY, idS, idR, idL);

  nCS = CData_GetNComps(idS);
  nRS = CData_GetNRecs(idS);

  CData_Xstore(idS, idG, 0, 1, 0);
  CData_Array(idR, T_DOUBLE, nCS, nRS);
  CData_SetNBlocks(idR, CData_GetNBlocks(idS));

  for (iR = 0; iR < nRS; iR++)
    dlm_ignorm((FLOAT64*) CData_XAddr(idS, iR, 0), (FLOAT64*) CData_XAddr(idR, iR, 0), nCS, nGamma.x);

  FOP_POSTCALC(idX, idY, idS, idR, idL);
  CSignal_SetVar(idY, "nGamma", nGamma);

  return O_K;
}

/**
 * <p>Divides (Mel-)Generalized Cepstrum coefficients by its &gamma;. If the input are (Mel-)LPC coefficients
 * <code>a<sub>k>0</sub></code> will be divided by <code>-1</code>. This is the inverse operation to {@link CSignal_GMult}.
 *
 * @param idY Output data instance.
 * @param idX Input data instance.
 * @return <code>O_K</code>
 */
INT16 CGEN_PUBLIC CSignal_IGMult(CData* idY, CData* idX) {
  INT32 iR = 0;
  INT32 nC = 0;
  INT32 nR = 0;
  COMPLEX64 nGamma = CMPLX(0.0);

  CData_Copy(BASEINST(idY), BASEINST(idX));

  CSignal_GetVar(idY, "nGamma", &nGamma);
  nC = CData_GetNComps(idY);
  nR = CData_GetNRecs(idY);

  for (iR = 0; iR < nR; iR++) {
    dlm_ignorm((FLOAT64*) CData_XAddr(idY, iR, 0), (FLOAT64*) CData_XAddr(idY, iR, 0), nC, nGamma.x);
  }
  return O_K;
}

/**
 * <p>IIR filter</p>
 *
 * <p>This operation filters the input signals given in the records of the input data instance <code>X</code> with
 * the filter coefficients given in <code>A</code>. If the number of records of <code>X</code> and <code>A</code>
 * are equal the <code>i<sup>th</sup></code> record of <code>X</code> will be filtered with filter coefficients of the
 * <code>i<sup>th</sup></code> record of <code>A</code>. If the number of records of <code>A</code> equals
 * <code>1</code> all records of <code>X</code> will be filtered with the coefficients given in this single record
 * of <code>A</code>. If <code>M</code> is not <code>NULL</code> the filter states were not reset at consecutive
 * records but on consecutive blocks. Otherwise the filter states were reset at each new record.</p>
 * <img src="../resources/base/dlp_math/filter_iir.gif" align="absmiddle">
 *
 * @param Y Output data instance holding filtered signals.
 * @param X Input data instance holding signals to filter.
 * @param A Filter coefficients.
 * @param M Filter states (can be <code>NULL</code>).
 * @return Equal to {@link CSignal_MIir}.
 *
 * @see CSignal_MFir CSignal_MIir CSignal_MIir CSignal_Filter CSignal_MFilter
 */
INT16 CGEN_PUBLIC CSignal_Iir(CData* Y, CData* X, CData* A, CData* M) {
  return CSignal_MIir(Y, X, A, 0.0, M);
}

/**
 * <p>Inverse Mel-Cepstrum (aka Mel-Cepstrum synthesis)
 *
 * <This operation synthesizes the given Mel-Cepstrum sequence to a speech signal using Mel-Cepstrum synthesis filter
 * excited with generated signal of given base frequency in <code>idF</code>. The given <code>nLambda</code> sets the
 * frequency warping of the output and can be used for changing voice characteristic. A value of <code>0.0</code> means
 * no frequency warping (no change of voice characteristic). The output format will be equivalent to a speech signal
 * framed by the signal operation <code>frame</code> and can be de-framed by signal operation <code>deframe</code>.</p>
 *
 * @param nLambda Target frequency warping factor.
 * @param idF Input base frequency
 * @param idC Input Mel-Cepstrum sequence
 * @param idY Output (framed) speech signal
 */
INT16 CGEN_PUBLIC CSignal_IMCep(CData* idY, CData* idC, CData* idE, FLOAT64 nLambda) {
  INT32 nCE = 0;
  INT32 nRC = 0;
  INT32 nRY = 0;
  INT32 nRE = 0;
  INT32 nCC = 0;
  INT32 iRC = 0;
  CData* idR = NULL;
  CData* idS = NULL;
  CData* idL = NULL;
  FLOAT64* mem = NULL;
  COMPLEX64 nLambda0 = CMPLX(0.0);
  COMPLEX64 nLambda1 = CMPLX(0.0);

  DLPASSERT((idC != NULL) && (idE != NULL) && (idY != NULL));

  nCE = CData_GetNComps(idE);
  nRC = CData_GetNRecs(idC);
  nCC = CData_GetNComps(idC);
  nRE = CData_GetNRecs(idE);
  nRY = nRC;
  CSignal_GetVar(idC, "nLambda", &nLambda0);
  nLambda1.x = (nLambda0.x - nLambda) / (1 - nLambda0.x * nLambda);

  if (nRE < nRC) return IERROR(idE,DATA_SIZE, "records", "at least", nRC);
  if (nRE > nRC) return IERROR(idE,DATA_TRUNCATE, "records", BASEINST(idE)->m_lpInstanceName, BASEINST(idC)->m_lpInstanceName);

  FOP_PRECALC(idC, idY, idS, idR, idL);

  CData_AddNcomps(idR, T_DOUBLE, nCE);
  CData_Allocate(idR, nRY);

  for (iRC = 0; iRC < nRC; iRC++) {
    dlm_mcep_synthesize_mlsadf((FLOAT64*) CData_XAddr(idC, iRC, 0), nCC, (FLOAT64*) CData_XAddr(idE, iRC, 0), nCE, nLambda1.x, 5, (FLOAT64*) CData_XAddr(idR, iRC, 0), &mem);
  }

  if (mem != NULL) {
    dlp_free(mem);
  }

  FOP_POSTCALC(idC, idY, idS, idR, idL);
  CData_CopyDescr(idY,idE);

  return O_K;
}

/**
 * <p>Inverse Modulated Lapped Transform (MLT)</p>
 *
 * <p>This operation reconstructs the signal frames from Modulated Lapped Transform coefficients.</p>
 *
 * @param idY Output data instance holding the framed signal.
 * @param idX Input data instance holding the MLT coefficients.
 * @param nLen frame length of output signal.
 */
INT16 CGEN_PUBLIC CSignal_IMlt(CData* idY, CData* idX) {
  INT32 i = 0;
  INT32 nR = 0;
  INT32 nC = 0;
  INT16 nErr = O_K;
  COMPLEX64 nScale = CMPLX(1);
  CData* idR = NULL;
  CData* idS = NULL;
  CData* idL = NULL;

  CSignal_GetVar(idX, "nScale", &nScale);
  FOP_PRECALC(idX, idY, idS, idR, idL);

  nR = CData_GetNRecs(idS);
  nC = CData_GetNComps(idS);

  CData_Array(idR, T_DOUBLE, nC, nR);
  CData_SetNBlocks(idR, CData_GetNBlocks(idS));

  nErr = dlm_imlt((FLOAT64*) CData_XAddr(idR, i, 0), (FLOAT64*) CData_XAddr(idS, i, 0), nC, nR);

  FOP_POSTCALC(idX, idY, idS, idR, idL);

  idY->m_nCinc = 1. / idX->m_nCinc / (nC * 2); /* Set phys. unit of comp. axis      */
  if (dlp_strcmp(idX->m_lpCunit, "Hz") == 0) dlp_strcpy(idY->m_lpCunit, "s");/* Rename abscissa "Hz"   -> "s"     */
  else if (dlp_strcmp(idX->m_lpCunit, "kHz") == 0) dlp_strcpy(idY->m_lpCunit, "ms");/* Rename abscissa "kHz"  -> "ms"    */
  else dlp_strcpy(idY->m_lpCunit, "");/* None of the above units -> clear  */
  return nErr;
}

/**
 * <p>Linear Predictive Coding analysis.</p>
 *
 * <p>This operation calculates <code>nCoeff</code> LPC-coefficients and the square root of the estimation of the
 * variance of the residual signal from each frame given in the records of <code>idX</code> using method
 * <code>lpsMethod</code> . The following methods currently implemented:
 * <dl><dt><code>"Burg"</code><dd><name>Burg</name>'s Method and
 *     <dt><code>"Levinson"</code><dd><name>Levinson</name>-<name>Durbin</name>-recursion.
 * </dl></p>
 *
 * @param idY Output data instance.
 * @param idG Square root of estimation of the variance of the residual signal.
 * @param idX Input data instance.
 * @param nCoeff Number of LPC-coefficients to calculate.
 * @param lpsMethod Calculation method.
 *
 */
INT16 CGEN_PUBLIC CSignal_Lpc(CData* idY, CData* idG, CData* idX, INT32 nCoeff, const char* lpsMethod) {
  return CSignal_MLpc(idY, idG, idX, 0.0, nCoeff, lpsMethod);
}

/**
 * <p>Linear Predictive Coding to Cepstrum transform.</p>
 *
 * <p>This operation converts LPC coefficients to <code>n</code> Cepstrum coefficients.
 * This operation is equal to call {@link CSignal_MLpc2MCep} with &lambda;<sub>2</sub>=0.</p>
 *
 * @param idC Output data instance.
 * @param idG Square root of estimation of the variance of the residual signal.
 * @param idA Input data instance.
 * @param n Number of G-Cepstrum coefficients to calculate.
 */
INT16 CGEN_PUBLIC CSignal_Lpc2Cep(CData* idC, CData* idG, CData* idA, INT32 n) {
  return CSignal_MLpc2MCep(idC, idG, idA, 0.0, n);
}

/**
 * <p>Linear Predictive Coding to Generalized Cepstrum transform.</p>
 *
 * <p>This operation converts LPC coefficients to <code>n</code> Generalized-Cepstrum coefficients of the given &gamma;.
 * This operation is equal to call {@link CSignal_MLpc2MGCep} with &lambda;<sub>2</sub>=0.</p>
 * <p>Note: The LPC coefficients are equal to the normalized Generalized-Cepstrum with &gamma;=-1.</p>
 *
 * @param idY Output data instance.
 * @param idG Square root of estimation of the variance of the residual signal.
 * @param idX Input data instance.
 * @param nGamma2 Target Generalized Cepstrum factor.
 * @param n Number of G-Cepstrum coefficients to calculate.
 */
INT16 CGEN_PUBLIC CSignal_Lpc2GCep(CData* idY, CData* idG, CData* idX, FLOAT64 nGamma2, INT32 n) {
  return CSignal_MLpc2MGCep(idY, idG, idX, nGamma2, 0.0, n);
}

/**
 * <p>Linear Predictive Coding to Mel-Generalized Cepstrum transform.</p>
 *
 * <p>This operation converts LPC coefficients to <code>n</code> Mel-Generalized-Cepstrum coefficients of the given
 * &gamma; and warping factor &lambda;. This operation is equal to call {@link CSignal_MLpc2MGCep}.</p>

 * @param idY Output data instance.
 * @param idG Square root of estimation of the variance of the residual signal.
 * @param idX Input data instance.
 * @param nGamma2 Target Generalized Cepstrum factor.
 * @param nLambda2 Target warping factor.
 * @param n Number of G-Cepstrum coefficients to calculate.
 */
INT16 CGEN_PUBLIC CSignal_Lpc2MGCep(CData* idY, CData* idG, CData* idX, FLOAT64 nGamma2, FLOAT64 nLambda2, INT32 n) {
  return CSignal_MLpc2MGCep(idY, idG, idX, nGamma2, nLambda2, n);
}

/**
 * <p>Linear Predictive Coding to Mel-Linear Predictive Coding transform.</p>
 *
 * <p>This operation transforms LPC coefficients to <code>n</code> Mel-LPC coefficients with the given warping factor
 * &lambda;. This operation is equal to call {@link CSignal_MLpc2MLpc}.
 *
 * @param idG2 Square root of estimation of the variance of the residual signal.
 * @param idA2 Output data instance.
 * @param idG1 Square root of estimation of the variance of the residual signal.
 * @param idA1 Input data instance.
 * @param nLambda2 Target warping factor.
 * @param n Number of Mel-LPC coefficients to calculate.
 */
INT16 CGEN_PUBLIC CSignal_Lpc2MLpc(CData* idA2, CData* idG2, CData* idG1, CData* idA1, FLOAT64 nLambda2, INT32 n) {
  return CSignal_MLpc2MLpc(idA2, idG2, idG1, idA1, nLambda2, n);
}

/**
 * <p>Line Spectral Frequencies to polynomial transform</p>
 *
 * <p>This operation transforms Line Spectral Frequencies to polynomila coefficients.</p>
 *
 * @param idY Output data instance.
 * @param idX Input data instance.
 */
INT16 CGEN_PUBLIC CSignal_Lsf2Poly(CData* idY, CData* idX) {
  INT32 iR = 0;
  INT32 nCS = 0;
  INT32 nRS = 0;
  CData* idR = NULL;
  CData* idS = NULL;
  CData* idL = NULL;

  FOP_PRECALC(idX, idY, idS, idR, idL);

  nCS = CData_GetNComps(idS);
  nRS = CData_GetNRecs(idS);

  CData_Copy(BASEINST(idR), BASEINST(idS));
  CData_InsertNcomps(idR, T_DOUBLE, 0, 1);
  CData_SetNBlocks(idR, CData_GetNBlocks(idS));

  for (iR = 0; iR < nRS; iR++) {
    CData_Dstore(idR, 1.0, iR, 0);
    dlm_lsf2poly((FLOAT64*) CData_XAddr(idR, iR, 0), (FLOAT64*) CData_XAddr(idR, iR, 0), nCS + 1);
  }

  FOP_POSTCALC(idX, idY, idS, idR, idL);

  return O_K;
}

/**
 * <p>Mel-Cepstrum to Cepstrum transform.</p>
 *
 * <p>This operation transforms Mel-Cepstrum coefficients to <code>n</code> Cepstrum coefficients.
 *
 * @param idC2 Output data instance.
 * @param idC1 Input data instance.
 * @param n Number of Cepstrum coefficients to calculate.
 */
INT16 CGEN_PUBLIC CSignal_MCep2Cep(CData* idC2, CData* idC1, INT32 n) {
  return CSignal_MCep2MCep(idC2, idC1, 0.0, n);
}

/**
 * <p>Mel-Cepstrum to Mel-Cepstrum transform.</p>
 *
 * <p>This operation transforms Mel-Cepstrum coefficients to <code>n</code> Mel-Cepstrum coefficients with the given target
 * warping factor &lambda;.
 *
 * @param idC2 Output data instance.
 * @param idC1 Input data instance.
 * @param nLambda2 Target warping factor.
 * @param n Number of Mel-Cepstrum coefficients to calculate.
 */
INT16 CGEN_PUBLIC CSignal_MCep2MCep(CData* idC2, CData* idC1, FLOAT64 nLambda2, INT32 n) {
  FLOAT64 nLambda = 0.0;
  COMPLEX64 nLambda1 = CMPLX(0.0);
  INT32 iR = 0;
  INT32 nCS = 0;
  INT32 nRS = 0;
  CData* idR = NULL;
  CData* idS = NULL;
  CData* idL = NULL;

  DLPASSERT(n > 0);

  CSignal_GetVar(idC1, "nLambda", &nLambda1);
  nLambda = (nLambda2 - nLambda1.x) / (1.0 - nLambda1.x * nLambda2);

  FOP_PRECALC(idC1, idC2, idS, idR, idL);

  nCS = CData_GetNComps(idS);
  nRS = CData_GetNRecs(idS);

  CData_Array(idR, T_DOUBLE, n, nRS);
  CData_SetNBlocks(idR, CData_GetNBlocks(idS));

  for (iR = 0; iR < nRS; iR++) {
    dlm_mcep2mcep((FLOAT64*) CData_XAddr(idS, iR, 0), nCS, (FLOAT64*) CData_XAddr(idR, iR, 0), n, 0, nLambda, NULL);
  }

  FOP_POSTCALC(idC1, idC2, idS, idR, idL);
  CSignal_SetVar(idC2, "nLambda", CMPLX(nLambda2) );
  CSignal_SetVar(idC2, "nGamma", CMPLX(0.0) );

  return O_K;
}

/**
 * <p>Mel-Generalized Cepstrum to Mel-Linear Predictive Coding transform.</p>
 *
 * <p>This operation converts Mel-Generalized-Cepstrum coefficients to <code>n</code> Mel-LPC coefficients with target
 * warping factor &lambda;.
 *
 * @param idG Square root of estimation of the variance of the residual signal.
 * @param idY Output data instance.
 * @param idX Input data instance.
 * @param nLambda2 Target warping factor.
 * @param n Number of Mel-LPC-coefficients to calculate.
 */
INT16 CGEN_PUBLIC CSignal_MCep2MLpc(CData* idY, CData* idG, CData* idX, FLOAT64 nLambda2, INT32 n) {
  DLPASSERT((idY!=NULL)&&(idG!=NULL)&&(idX!=NULL)&&(n>0));

  CData_Copy(BASEINST(idY), BASEINST(idX));

  CSignal_MGCep2MGCep(idY, idY, -1, nLambda2, n);
  CSignal_GCepNorm(idY, idG, idY);
  return O_K;
}

/**
 * <p>Mel-Cepstrum analysis.</p>
 *
 * <p>This operation calculates <code>nCoeff</code> Mel-Cepstrum coefficients from each frame given in the records of
 * <code>idX</code> using method Unbiased Estimator of Log-Spectrum (UELS) method.</p>
 *
 * @param idY Output data instance.
 * @param idX Input data instance.
 * @param nCoeff Number of Cepstrum coefficients to calculate.
 * @param nLambda Warping factor.
 *
 */
INT16 CGEN_PUBLIC CSignal_MCep(CData* idY, CData* idX, FLOAT64 nLambda, INT32 nCoeff) {
  INT16 nTS = -1;
  INT32 iR = 0;
  INT32 nCS = 0;
  INT32 nRS = 0;
  COMPLEX64 nScale = CMPLX(1.0);
  CData* idR = NULL;
  CData* idS = NULL;
  CData* idL = NULL;

  DLPASSERT(nCoeff > 0);
  FOP_PRECALC(idX, idY, idS, idR, idL);

  CSignal_GetVar(idX, "nScale", &nScale);

  nCS = CData_GetNComps(idS);
  nRS = CData_GetNRecs(idS);
  nTS = CData_GetCompType(idS, 0);

  CData_Tconvert(idS, idS, T_DOUBLE);
  CData_Array(idR, T_DOUBLE, nCoeff, nRS);
  CData_SetNBlocks(idR, CData_GetNBlocks(idS));

  for (iR = 0; iR < nRS; iR++)
    dlm_calcmcep((FLOAT64*) CData_XAddr(idS, iR, 0), nCS, (FLOAT64*) CData_XAddr(idR, iR, 0), nCoeff, nLambda, nScale.x, DLM_CALCCEP_METHOD_S_MCEP_UELS);

  CData_Tconvert(idS, idS, nTS);
  FOP_POSTCALC(idX, idY, idS, idR, idL);
  CSignal_SetVar(idY, "nLambda", CMPLX(nLambda) );
  CSignal_SetVar(idY, "nGamma", CMPLX(0.0) );

  return O_K;
}

/**
 * <p>Denoising.</p>
 *
 * <p>This operation tries to denoise the input. The type of input must be specified by <code>lpsType</code>. The
 * following type of inputs are currently supported:
 * <ul>
 *   <li>"cepstrum" - Cepstral Mean Substraction
 *     <blockquote>The moving average of the last <code>nT</code> milliseconds of cepstra with a 0<sup>th</sup> cepstrum
 *     coefficients less than <code>nParam2<code> is calculated and subtracted from the current cepstrum.
 *     </blockquote>
 *   </li>
 *   <li>"spectrum" - Spectral Mean Substraction
 *     <blockquote>The moving average of the last <code>nT</code> milliseconds of magnitude spectra is calculated,
 *     weighted by <code>nP</code> and subtracted from the current magnitude spectrum frame. It is ensured to get a
 *     non-negative magnitude spectra.
 *     </blockquote>
 *   </li>
 * </ul></p>
 *
 * @param idY Output data instance.
 * @param idX Input data instance.
 * @param nT time constant in milliseconds
 * @param nP Parameter depends on input type
 * @param lpsType Type of input (signal, spectrum, cepstrum)
 *
 */
INT16 CGEN_PUBLIC CSignal_Denoise(CData* idY, CData* idX, INT32 nT, FLOAT64 nP, const char* lpsType) {
  INT32 nCC = 0;
  INT32 nRC = 0;
  INT32 iRC = 0;
  INT32 nF  = 0;
  INT32 nFL = 0;
  INT32 nFC = 0;
  INT32 nFR = 0;
  FLOAT64 nSR = 0.0;
  CData* idS = NULL;
  CData* idR = NULL;
  CData* idL = NULL;
  CData* idA = NULL;
  COMPLEX64 nD = CMPLX(0.);

  DLPASSERT((idX != NULL) && (idY != NULL));

  FOP_PRECALC(idX,idY,idS,idR,idL);

  nSR = CData_GetDescr(idS,RINC);
  nRC = CData_GetNRecs(idS);
  nCC = CData_GetNComps(idS);
  nF = MAX(1,nT / nSR);

  if(!dlp_strcmp("signal", lpsType)) {
    nFL = (INT32)(16.0/nSR+0.5);
    nFC = (INT32)(8.0/nSR+0.5);
    ICREATE(CData,idA,FALSE);
    CSignal_Frame(idA,idS,nFL,nFC);
    nFR = CData_GetNRecs(idA);
    CData_Tconvert(idA,idA,T_COMPLEX);
    for(iRC = 0; iRC < nFR; iRC++) {
      dlm_fftC((COMPLEX64*)CData_XAddr(idA,iRC,0),nFL,FALSE);
    }
    CData_Copy(BASEINST(idR),BASEINST(idA));
    dlm_matropC((COMPLEX64*)CData_XAddr(idR,0,0),NULL,NULL,(COMPLEX64*)CData_XAddr(idA,0,0),nFR,nFL,&nD,1,1,OP_ABS);
    dlm_matropC((COMPLEX64*)CData_XAddr(idA,0,0),NULL,NULL,(COMPLEX64*)CData_XAddr(idA,0,0),nFR,nFL,(COMPLEX64*)CData_XAddr(idR,0,0),nFR,nFL,OP_DIV_EL);
    CData_Tconvert(idR,idR,T_DOUBLE);
    CSignal_Denoise(idR,idR,nT,nP,"spectrum");
    CData_Tconvert(idR,idR,T_COMPLEX);
    dlm_matropC((COMPLEX64*)CData_XAddr(idR,0,0),NULL,NULL,(COMPLEX64*)CData_XAddr(idR,0,0),nFR,nFL,(COMPLEX64*)CData_XAddr(idA,0,0),nFR,nFL,OP_MULT_EL);
    for(iRC = 0; iRC < nFR; iRC++) {
      dlm_fftC((COMPLEX64*)CData_XAddr(idR,iRC,0),nFL,TRUE);
    }
    dlm_matropC((COMPLEX64*)CData_XAddr(idR,0,0),NULL,NULL,(COMPLEX64*)CData_XAddr(idR,0,0),nFR,nFL,&nD,1,1,OP_REAL);
    CData_Tconvert(idR,idR,T_DOUBLE);
    CSignal_DeFrame(idR,idR,(INT32)(8.0/nSR+0.5));
    IDESTROY(idA);
  } else if(!dlp_strcmp("cepstrum", lpsType)) {
    CData_Copy(BASEINST(idR), BASEINST(idS));
    dlm_mcep_denoise((FLOAT64*) CData_XAddr(idS, iRC, 0), (FLOAT64*) CData_XAddr(idR, iRC, 0), nRC, nCC, nF, nP);
  } else if(!dlp_strcmp("spectrum", lpsType)) {
    CData_Copy(BASEINST(idR), BASEINST(idS));
    dlm_spec_denoise((FLOAT64*) CData_XAddr(idS, iRC, 0), (FLOAT64*) CData_XAddr(idR, iRC, 0), nRC, nCC, nF, nP);
  } else {
    CERROR(CSignal, ERR_INVALARG, "lpsType",0,0);
    return NOT_EXEC;
  }

  FOP_POSTCALC(idX,idY,idS,idR,idL);

  return O_K;
}

/**
 * <p>Mel-Cepstrum enhancement.</p>
 *
 * <This operation manipulates the mel-cepstrum to reduce buzziness of synthesis.</p>
 *
 * @param idC Input Mel-Cepstrum sequence
 * @param idY Output Mel-Cepstrum sequence
 */
INT16 CGEN_PUBLIC CSignal_MCepEnhance(CData* idY, CData* idC) {
  INT32 nCC = 0;
  INT32 nRC = 0;
  INT32 iRC = 0;
  COMPLEX64 nLambda = CMPLX(0.0);

  DLPASSERT((idC != NULL) && (idY != NULL));

  nRC = CData_GetNRecs(idC);
  nCC = CData_GetNComps(idC);
  CSignal_GetVar(idC, "nLambda", &nLambda);
  CData_Copy(BASEINST(idY), BASEINST(idC));

  for (iRC = 0; iRC < nRC; iRC++) {
    dlm_mcep_enhance((FLOAT64*) CData_XAddr(idC, iRC, 0), (FLOAT64*) CData_XAddr(idY, iRC, 0), nCC, nLambda.x);
  }

  return O_K;
}

/**
 * <p>Mel-Filter-Bank analysis.</p>
 *
 * <p>This operation calculates <code>nCoeff</code> MFB-coefficients with warping factor <code>nLambda</code> and
 * method <code>lpsMethod</code> from each frame given in the records of <code>idX</code>. The following methods
 * currently implemented:
 * <ul><li>"BS" - Bark scale warping, sinc frequency window (i.e. cepstral smoothing),</li>
 *     <li>"BT" - Bark scale warping, triangular frequency window,</li>
 *     <li>"MS" - Bilinear warping, sinc frequency window (i.e. cepstral smoothing),</li>
 *     <li>"MT" - Bilinear warping, triangular frequency window,</li>
 * </ul></p>
 *
 * @param idY Output data instance.
 * @param idX Input data instance.
 * @param nCoeff Number of MFB-coefficients to calculate.
 * @param nLambda Warping factor.
 * @param lpsMethod Calculation method.
 *
 */
INT16 CGEN_PUBLIC CSignal_MFb(CData* idY, CData* idX, FLOAT64 nLambda, INT32 nCoeff, const char* lpsMethod) {
  INT16 nRetVal = O_K;
  INT16 nTS = -1;
  INT32 nCS = 0;
  INT32 nRS = 0;
  INT32 iR = 0;
  FLOAT64 nMinLog = 0.0;
  COMPLEX64 nQuant = CMPLX(T_DOUBLE);
  COMPLEX64 nScale = CMPLX(1.0);
  CData* idR = NULL;
  CData* idS = NULL;
  CData* idL = NULL;
  MLP_CNVC_TYPE lpCnvc = { 0, 0, 0, NULL, { NULL, NULL }, NULL, NULL, NULL, "\0", nLambda };

  CSignal_GetVar(idX, "nType", &nQuant);
  CSignal_GetVar(idX, "nScale", &nScale);
  FOP_PRECALC(idX, idY, idS, idR, idL);

  nMinLog = dlp_scalop(CSignal_GetMinQuant((INT16) nQuant.x, nScale).x, 0.0, OP_LN);
  nCS = CData_GetNComps(idS);
  nRS = CData_GetNRecs(idS);
  nTS = CData_GetCompType(idS, 0);

  CData_Tconvert(idS, idS, T_DOUBLE);

  if (nCoeff > 0) {
    CData_AddNcomps(idR, CData_GetCompType(idS, 0), nCoeff);
    CData_Allocate(idR, nRS);

    dlp_strcpy(lpCnvc.type, (lpsMethod == NULL) ? "MT" : lpsMethod);
    dlm_mf_init(&lpCnvc, nCS, nCoeff, -dlp_scalop(nScale.x, 0, OP_LN));

    for (iR = 0; iR < nRS; iR++)
      dlm_mf_analyze(&lpCnvc, (FLOAT64*) CData_XAddr(idS, iR, 0), (FLOAT64*) CData_XAddr(idR, iR, 0), nCS, nCoeff, nMinLog);

    dlm_mf_done(&lpCnvc);
  }
  CData_SetNBlocks(idR, CData_GetNBlocks(idS));
  CData_Tconvert(idS, idS, nTS);

  FOP_POSTCALC(idX, idY, idS, idR, idL);

  return nRetVal;
}

/**
 * <p>Mel-Filter-Bank analysis in spectral domain.</p>
 *
 * <p>This operation calculates <code>nCoeff</code> MFB-coefficients with warping factor <code>nLambda</code> and
 * method <code>lpsMethod</code> from each frame given in the records of <code>idX</code>. The following methods
 * currently implemented:
 * <ul><li>"BS" - Bark scale warping, sinc frequency window (i.e. cepstral smoothing),</li>
 *     <li>"BT" - Bark scale warping, triangular frequency window,</li>
 *     <li>"MS" - Bilinear warping, sinc frequency window (i.e. cepstral smoothing),</li>
 *     <li>"MT" - Bilinear warping, triangular frequency window,</li>
 * </ul></p>
 *
 * @param idY Output data instance.
 * @param idX Input data instance.
 * @param nCoeff Number of MFB-coefficients to calculate.
 * @param nLambda Warping factor.
 * @param lpsMethod Calculation method.
 *
 */
INT16 CGEN_PUBLIC CSignal_MFbs(CData* idY, CData* idX, FLOAT64 nLambda, INT32 nCoeff, const char* lpsMethod) {
  INT16 nRetVal = O_K;
  INT16 nTS = -1;
  INT32 nCS = 0;
  INT32 nRS = 0;
  INT32 iR = 0;
  FLOAT64 nMinLog = 0.0;
  COMPLEX64 nQuant = CMPLX(T_DOUBLE);
  COMPLEX64 nScale = CMPLX(1.0);
  CData* idR = NULL;
  CData* idS = NULL;
  CData* idL = NULL;
  MLP_CNVC_TYPE lpCnvc = { 0, 0, 0, NULL, { NULL, NULL }, NULL, NULL, NULL, "\0", nLambda };

  CSignal_GetVar(idX, "nType", &nQuant);
  CSignal_GetVar(idX, "nScale", &nScale);
  FOP_PRECALC(idX, idY, idS, idR, idL);

  nMinLog = dlp_scalop(CSignal_GetMinQuant((INT16) nQuant.x, nScale).x, 0.0, OP_LN);
  nCS = CData_GetNComps(idS);
  nRS = CData_GetNRecs(idS);
  nTS = CData_GetCompType(idS, 0);

  CData_Tconvert(idS, idS, T_DOUBLE);

  if (nCoeff > 0) {
    CData_AddNcomps(idR, CData_GetCompType(idS, 0), nCoeff);
    CData_Allocate(idR, nRS);

    dlp_strcpy(lpCnvc.type, (lpsMethod == NULL) ? "MT" : lpsMethod);
    dlm_mf_init(&lpCnvc, nCS, nCoeff, -dlp_scalop(nScale.x, 0, OP_LN));

    for (iR = 0; iR < nRS; iR++)
      dlm_mf_convolve(&lpCnvc, (FLOAT64*) CData_XAddr(idS, iR, 0), (FLOAT64*) CData_XAddr(idR, iR, 0));

    dlm_mf_done(&lpCnvc);
  }
  CData_SetNBlocks(idR, CData_GetNBlocks(idS));
  CData_Tconvert(idS, idS, nTS);

  FOP_POSTCALC(idX, idY, idS, idR, idL);

  return nRetVal;
}

/**
 * <p>Warped Discrete Fourier Transform.</p>
 *
 * <p>This operation calls {@link CSignal_Fft} and warps the resulting
 * complex spectrum.
 *
 * @param idY Input data instance.
 * @param idX Output data instance
 * @param nLambda Warping factor.
 * @return <code>O_K</code>
 *
 * @see CSignal_Fft
 *
 */
INT16 CGEN_PUBLIC CSignal_Mfft(CData* idY, CData* idX, FLOAT64 nLambda) {
  INT32 nC = 0;
  INT32 nR = 0;
  INT32 iR = 0;
  INT16 nErr = O_K;
  CData* idR = NULL;
  CData* idS = NULL;
  CData* idL = NULL;
  CData* idT = NULL;

  ICREATE(CData, idT, FALSE);

  nErr = CSignal_FftImpl(idT, idX, FALSE);

  FOP_PRECALC(idT, idY, idS, idR, idL);

  CData_Copy(BASEINST(idR), BASEINST(idS));
  nR = CData_GetNRecs(idS);
  nC = CData_GetNComps(idS);

  for (iR = 0; iR < nR; iR++) {
    nErr = dlm_fft_warpC((COMPLEX64*) CData_XAddr(idS, iR, 0), (COMPLEX64*) CData_XAddr(idR, iR, 0), nC, nLambda);
    if (nErr != O_K) break;
  }

  FOP_POSTCALC(idT, idY, idS, idR, idL);

  IDESTROY(idT);

  return nErr;
}

/**
 * <p>ARMA filtering. Filters signal frames (frames must given at rows of <code>X</code>)
 * using ARMA filter with coefficients <code>B</code> and <code>A</code> and filter states
 * <code>M</code> (can be <code>NULL</code>) and outputs the result to <code>Y</code>:</p>
 * <p>Y(z)=B(z)A<sup>-1</sup>(z)X(z).</p>
 * <p>If the number of records of <code>X</code> and <code>B</code> and <code>A</code>
 * are equal the <code>i<sup>th</sup></code> record of <code>X</code> will be filtered with filter coefficients of the
 * <code>i<sup>th</sup></code> record of <code>B</code> and <code>A</code>. If the number of records of <code>B</code>
 * and <code>A</code> equals <code>1</code> all records of <code>X</code> will be filtered with the coefficients given
 * in this single record of <code>B</code> and <code>A</code>.</p>
 * <p>If <code>lambda</code>&ne;0</code> the filter coefficients <code>B</code> and <code>A</code> are intended to be
 * warped on the mel-scale with warping coeffizient &lambda;. The coefficients were transformed to filter coefficients
 * using <code><a href="dlpmath.html#dlm_c2b">dlm_c2b</code></a>.</p>
 * <img src="../resources/base/dlp_math/filter_m.gif" align="absmiddle">
 * <h4>Remarks</h4>
 * <ul>
 * <li>If <code>lambda</code>=0.0
 * <ul>
 * <li>If <code>A</code>=<code>NULL</code> the filter is implemented as a FIR
 * (MA) filter, i.e. <code>A</code> is set to <code>1</code>.</li>
 * <li>If <code>B</code>=<code>NULL</code> the filter is implemented as a pure
 * IIR (AR) filter, i.e. <code>B</code> is set to <code>1</code>.</li></ul>
 * <li><code>B</code> must be a column vector.</li>
 * <li><code>A</code> must be a column vector.</li>
 * <li>If a<sub>0</sub> is not equal to 1 then the result is normalized by
 * a<sub>0</sub></li>
 * <li>If filter states non-<code>NULL</code> then <code>M</code> mus be a
 * column vector of size <code>MAX(length(B),length(A))-1</code>.</li>
 * </ul>
 *
 * @param Y Destination
 * @param X Source
 * @param B Vector of filter coefficients
 * @param A Vector of filter coefficients
 * @param lambda Warping coefficient &lambda;
 * @param M Filter states
 *
 */
INT16 CGEN_PUBLIC CSignal_MFilter(CData* idY, CData* idX, CData* idB, CData* idA, FLOAT64 nLambda, CData* idM) {
  BYTE* lpB = NULL; /* Buffer for vector B               */
  BYTE* lpA = NULL; /* Buffer for vector A               */
  FLOAT64* lpS = NULL; /* Buffer for matrix S               */
  FLOAT64* lpW = NULL; /* Buffer for matrix W               */
  FLOAT64* lpM = NULL; /* Buffer for vector M               */
  FLOAT64* lpR = NULL; /* Buffer for matrix R               */
  COMPLEX64 nScale = CMPLX(0);
  COMPLEX64 nScaleS = CMPLX(0);
  COMPLEX64 nScaleA = CMPLX(0);
  COMPLEX64 nScaleB = CMPLX(0);
  INT16 nTS; /* Type of input */
  INT32 nBS; /* Number of blocks of matrix S      */
  INT32 nBRS; /* Records per block of matrix S     */
  INT32 nCB; /* Dimension of matrix B             */
  INT32 nRB; /* Records of matrix B               */
  INT32 nRLB; /* Record length of matrix B         */
  INT32 nCA; /* Dimension of matrix A             */
  INT32 nRA; /* Records of matrix A               */
  INT32 nRLA; /* Record length of matrix A         */
  INT32 nCS; /* Dimension of matrix X and Y       */
  INT32 nRS; /* Number of records                 */
  INT32 nM; /* Dimension of matrix X             */
  INT32 iR; /* Current record index              */
  INT32 iB; /* Current record index              */
  BOOL bNoMem = FALSE;
  CData* idR = NULL;
  CData* idS = NULL;
  CData* idL = NULL;

  FOP_PRECALC(idX, idY, idS, idR, idL);

  /* Validation *//* ---------------------------------*/
  if (CData_IsEmpty(idS)) return IERROR(idS,DATA_EMPTY,"idX",0,0); /* Need signal to filter            */

  nCB = CData_GetNNumericComps(idB); /* Get number of filter coefficients */
  nCA = CData_GetNNumericComps(idA); /* Get number of filter coefficients */
  nCS = CData_GetNNumericComps(idS); /* Get matrix dimension              */
  nRB = CData_GetNRecs(idB); /* Get number of records             */
  nRA = CData_GetNRecs(idA); /* Get number of records             */
  nRS = CData_GetNRecs(idS); /* Get number of records             */
  nRLB = CData_GetRecLen(idB); /* Get record length                 */
  nRLA = CData_GetRecLen(idA); /* Get record length                 */
  nBS = CData_GetNBlocks(idS);
  nBRS = CData_GetNRecsPerBlock(idS);

  DLPASSERT((nRA != 0) || (nRB != 0));
  if (nRA == 0)
  DLPASSERT((nRB == 1) || (nRB == nRS));
  if (nRB == 0)
  DLPASSERT((nRA == 1) || (nRA == nRS));
  if ((nRA != 0) && (nRB != 0))
  DLPASSERT((nRA == 1) || (nRA == nRS) || (nRB == 1) || (nRB == nRS));

  if (CSignal_GetVar(idS, "nScale", &nScaleS) == O_K) {
    if (idA && (CSignal_GetVar(idA, "nScale", &nScaleA) == O_K)) {
      if (idB && (CSignal_GetVar(idB, "nScale", &nScaleB) == O_K)) {
        if (CMPLX_EQUAL(nScaleA,nScaleB) && CMPLX_EQUAL(nScaleA,nScaleS)) {
          nScale = nScaleS;
          CSignal_ScaleImpl(idS, idS, dlp_scalopC(nScale, CMPLX(0), OP_INVT));
        }
      } else if (CMPLX_EQUAL(nScaleA, nScaleS)) {
        nScale = nScaleS;
        CSignal_ScaleImpl(idS, idS, dlp_scalopC(nScale, CMPLX(0), OP_INVT));
      }
    } else if (idB && (CSignal_GetVar(idB, "nScale", &nScaleB) == O_K)) {
      if (CMPLX_EQUAL(nScaleB, nScaleS)) {
        nScale = nScaleS;
        CSignal_ScaleImpl(idS, idS, dlp_scalopC(nScale, CMPLX(0), OP_INVT));
      }
    }
  }

  nTS = CData_GetCompType(idS, 0);
  CData_Tconvert(idS, idS, T_DOUBLE);

  if (idM != NULL) {
    if (CData_GetNComps(idM) < (MAX(nCB,nCA) - 1)) {
      CData_Reset(BASEINST(idM), FALSE);
      CData_AddNcomps(idM, T_DOUBLE, MAX(nCB,nCA) - 1);
      CData_Allocate(idM, 1);
    }
    nM = CData_GetNComps(idM);
  } else {
    nM = 0;
    bNoMem = TRUE;
  }

  lpB = CData_XAddr(idB, 0, 0); /* Get address of coefficients B    */
  lpA = CData_XAddr(idA, 0, 0); /* Get address of coefficients A    */
  lpS = (FLOAT64*) CData_XAddr(idS, 0, 0); /* Get address of input             */
  lpM = (FLOAT64*) CData_XAddr(idM, 0, 0); /* Get address of memory            */
  lpW = (FLOAT64*) dlp_calloc(nRS*nCS,sizeof(FLOAT64)); /* Alloc memory for work matrix W   */
  dlp_memmove(lpW, lpS, nRS * nCS * sizeof(FLOAT64)); /* Copy input to working matrix     */
  lpR = (FLOAT64*) dlp_calloc(nRS*nCS,sizeof(FLOAT64)); /* Alloc memory for matrix Y        */
  CData_Copy(BASEINST(idR), BASEINST(idS)); /* Initialize matrix Y              */

  if (nLambda == 0.0) {
    for (iR = 0; iR < nRS; iR++) { /* Loop over all records >>          */
      if (bNoMem || !(iR % nBRS)) dlp_memset(lpM, 0, nM * sizeof(FLOAT64));
      dlm_filter((FLOAT64*) lpB, nCB, (FLOAT64*) lpA, nCA, lpW + iR * nCS, lpR + iR * nCS, nCS, lpM, nM);
      if (nRA > 1) lpA += nRLA;
      if (nRB > 1) lpB += nRLB;
    } /* <<                                */
  } else {
    for (iR = 0; iR < nRS; iR++) { /* Loop over all records >>          */
      if (!(iR % nBRS)) dlp_memset(lpM, 0, nM * sizeof(FLOAT64));
      dlm_filter_m((FLOAT64*) lpB, nCB, (FLOAT64*) lpA, nCA, lpW + iR * nCS, lpR + iR * nCS, nCS, lpM, nM, nLambda); /*   Calculate                       */
      if (nRA > 1) lpA += nRLA;
      if (nRB > 1) lpB += nRLB;
    } /* <<                                */
  }
  for (iB = 0; iB < nBS; iB++)
    CData_DblockStore(idR, lpR + iB * nBRS * nCS, iB, nCS, nBRS, -1);
  dlp_free(lpR); /* Free buffer Y                     */
  dlp_free(lpW); /* Free buffer W                     */

  if (!CMPLX_EQUAL(nScale,CMPLX(0))) CSignal_ScaleImpl(idR, idR, nScale);
  CData_Tconvert(idS, idS, nTS);

  FOP_POSTCALC(idX, idY, idS, idR, idL);
  return O_K;
}

/**
 * <p>Mel-FIR filter</p>
 *
 * <p>This operation filters the input signals given in the records of the input data instance <code>X</code> with
 * the filter coefficients given in <code>B</code>. If the number of records of <code>X</code> and <code>B</code>
 * are equal the <code>i<sup>th</sup></code> record of <code>X</code> will be filtered with filter coefficients of the
 * <code>i<sup>th</sup></code> record of <code>B</code>. If the number of records of <code>B</code> equals
 * <code>1</code> all records of <code>X</code> will be filtered with the coefficients given in this single record
 * of <code>B</code>. If <code>M</code> is not <code>NULL</code> the filter states were not reset at consecutive
 * records but on consecutive blocks. Otherwise the filter states were reset at each new record.
 * <p>If <code>lambda</code>&ne;0</code> the filter coefficients <code>B</code> and <code>A</code> are intended to be
 * warped on the mel-scale with warping coeffizient &lambda;. The coefficients were transformed to filter coefficients
 * using <code><a href="dlpmath.html#dlm_c2b">dlm_c2b</code></a>.</p>
 * <img src="../resources/base/dlp_math/filter_mfir.gif" align="absmiddle">
 *
 * @param Y Output data instance holding filtered signals.
 * @param X Input data instance holding signals to filter.
 * @param B Filter coefficients.
 * @param nLambda Warping factor.
 * @param M Filter states (can be <code>NULL</code>).
 * @return Equal to {@link CSignal_MFilter}.
 *
 * @see CSignal_Fir CSignal_Iir CSignal_MIir CSignal_Filter CSignal_MFilter
 */
INT16 CGEN_PUBLIC CSignal_MFir(CData* Y, CData* X, CData* B, FLOAT64 nLambda, CData* M) {
  return CSignal_MFilter(Y, X, B, NULL, nLambda, M);
}

/**
 * <p>Mel-Generalized-Cepstrum analysis.</p>
 *
 * <p>This operation calculates <code>nCoeff</code> Mel-Generalized-Cepstrum coefficients from each frame given in the records of
 * <code>idX</code>. If <code>nGamma</code> equals to zero Mel-Cepstrum is calculated by the UELS method</p>
 *
 * @param idY Output data instance.
 * @param idX Input data instance.
 * @param nGamma Generalized cepstrum factor &gamma; of output.
 * @param nLambda Warping factor.
 * @param nCoeff Number of Mel-Generalized-Cepstrum coefficients to calculate.
 *
 */
INT16 CGEN_PUBLIC CSignal_MGCep(CData* idY, CData* idX, FLOAT64 nGamma, FLOAT64 nLambda, INT32 nCoeff) {
  INT16 nTS = -1;
  INT32 iR = 0;
  INT32 nCS = 0;
  INT32 nRS = 0;
  COMPLEX64 nScale = CMPLX(1.0);
  CData* idR = NULL;
  CData* idS = NULL;
  CData* idL = NULL;

  DLPASSERT((nGamma >= -1) && (nGamma <= 0.0));

  CSignal_GetVar(idX, "nScale", &nScale);
  FOP_PRECALC(idX, idY, idS, idR, idL);

  nCS = CData_GetNComps(idS);
  nRS = CData_GetNRecs(idS);
  nTS = CData_GetCompType(idS, 0);

  CData_Tconvert(idS, idS, T_DOUBLE);
  CData_Array(idR, T_DOUBLE, nCoeff, nRS);
  CData_SetNBlocks(idR, CData_GetNBlocks(idS));

  dlm_mgcep_init(nCS,nCoeff,nLambda);
  for (iR = 0; iR < nRS; iR++) {
    if (nGamma == 0.0) {
      dlm_mcep_uels_sptk((FLOAT64*) CData_XAddr(idS, iR, 0), nCS, (FLOAT64*) CData_XAddr(idR, iR, 0), nCoeff, nLambda, nScale.x);
    } else {
      dlm_mgcep((FLOAT64*) CData_XAddr(idS, iR, 0), nCS, (FLOAT64*) CData_XAddr(idR, iR, 0), nCoeff, nGamma, nLambda, nScale.x);
    }
  }
  dlm_mgcep_free();

  CData_Tconvert(idS, idS, nTS);
  FOP_POSTCALC(idX, idY, idS, idR, idL);
  CSignal_SetVar(idY, "nGamma", CMPLX(nGamma) );

  return O_K;
}

/**
 * <p>Mel-Generalized Cepstrum to Linear Predictive Coding transform.</p>
 *
 * <p>This operation converts Mel-Generalized-Cepstrum coefficients to <code>n</code> LPC coefficients. This function
 * calls {@link CSignal_MGCep2MLpc} with <code>nLambda2=0</code>.
 *
 * @param idG Square root of estimation of the variance of the residual signal.
 * @param idY Output data instance.
 * @param idX Input data instance.
 * @param n Number of Mel-LPC-coefficients to calculate.
 */
INT16 CGEN_PUBLIC CSignal_MGCep2Lpc(CData* idY, CData* idG, CData* idX, INT32 n) {
  return CSignal_MGCep2MLpc(idY, idG, idX, 0.0, n);
}

/**
 * <p>Mel-Generalized Cepstrum transform</p>
 * <p>This operation transforms the Mel-Generalized Cepstrum with warping factor &lambda;<sub>1</sub> and
 * &gamma;<sub>1</sub> to the Mel-Generalized Cepstrum with warping factor &lambda;<sub>2</sub> and &gamma;<sub>2</sub>.
 * The Mel-Generalized Cepstrum is assumed to be in non-normalized form.
 *
 * @param idY Output data instance (non-normalized Mel-Generalized Cepstrum).
 * @param idX Input data instance (non-normalized Mel-Generalized Cepstrum).
 * @param nGamma2 Generalized cepstrum factor &gamma; of output.
 * @param nLambda2 Warpinmg factor &lambda; of output.
 * @param n Number of Mel-Generalized Cepstrum coeffizients of output.
 * @return <code>O_K</code>
 * @see <a href="dlpmath.html#dlm_mgcep2mgcep"><code class="link">dlm_mgcep2mgcep</code></a>
 */
INT16 CGEN_PUBLIC CSignal_MGCep2MGCep(CData* idY, CData* idX, FLOAT64 nGamma2, FLOAT64 nLambda2, INT32 n) {
  INT32 iR = 0;
  INT32 nCS = 0;
  INT32 nRS = 0;
  COMPLEX64 nLambda1 = CMPLX(0.0);
  COMPLEX64 nGamma1 = CMPLX(0.0);
  CData* idR = NULL;
  CData* idS = NULL;
  CData* idL = NULL;

  DLPASSERT(n > 0);
  FOP_PRECALC(idX, idY, idS, idR, idL);
  CSignal_GetVar(idX, "nLambda", &nLambda1);
  CSignal_GetVar(idX, "nGamma", &nGamma1);

  nCS = CData_GetNComps(idS);
  nRS = CData_GetNRecs(idS);

  CData_Array(idR, T_DOUBLE, n, nRS);
  CData_SetNBlocks(idR, CData_GetNBlocks(idS));

  for (iR = 0; iR < nRS; iR++) {
    dlm_mgcep2mgcep((FLOAT64*) CData_XAddr(idS, iR, 0), nCS, nGamma1.x, nLambda1.x, (FLOAT64*) CData_XAddr(idR, iR, 0), n, nGamma2, nLambda2);
  }

  FOP_POSTCALC(idX, idY, idS, idR, idL);
  CSignal_SetVar(idY, "nLambda", CMPLX(nLambda2) );
  CSignal_SetVar(idY, "nGamma", CMPLX(nGamma2) );

  return O_K;
}

/**
 * <p>Mel-Generalized Cepstrum to Mel-Linear Predictive Coding transform.</p>
 *
 * <p>This operation converts Mel-Generalized-Cepstrum coefficients to <code>n</code> Mel-LPC coefficients with target
 * warping factor &lambda;.
 *
 * @param idG Square root of estimation of the variance of the residual signal.
 * @param idY Output data instance.
 * @param idX Input data instance.
 * @param nLambda2 Target warping factor.
 * @param n Number of Mel-LPC-coefficients to calculate.
 */
INT16 CGEN_PUBLIC CSignal_MGCep2MLpc(CData* idY, CData* idG, CData* idX, FLOAT64 nLambda2, INT32 n) {
  COMPLEX64 nLambda1 = CMPLX(0.0);
  COMPLEX64 nGamma1 = CMPLX(0.0);

  DLPASSERT((idY!=NULL)&&(idG!=NULL)&&(idX!=NULL)&&(n>0));

  CData_Copy(BASEINST(idY), BASEINST(idX));

  if ((nGamma1.x != -1) || (nLambda1.x != nLambda2)) {
    CSignal_MGCep2MGCep(idY, idY, -1, nLambda2, n);
  }
  CSignal_GCepNorm(idY, idG, idY);
  return O_K;
}

/**
 * <p>Mel-IIR filter</p>
 *
 * <p>This operation filters the input signals given in the records of the input data instance <code>X</code> with
 * the filter coefficients given in <code>A</code>. If the number of records of <code>X</code> and <code>A</code>
 * are equal the <code>i<sup>th</sup></code> record of <code>X</code> will be filtered with filter coefficients of the
 * <code>i<sup>th</sup></code> record of <code>A</code>. If the number of records of <code>A</code> equals
 * <code>1</code> all records of <code>X</code> will be filtered with the coefficients given in this single record
 * of <code>A</code>. If <code>M</code> is not <code>NULL</code> the filter states were not reset at consecutive
 * records but on consecutive blocks. Otherwise the filter states were reset at each new record.
 * <p>If <code>lambda</code>&ne;0</code> the filter coefficients <code>A</code> and <code>A</code> are intended to be
 * warped on the mel-scale with warping coeffizient &lambda;. The coefficients were transformed to filter coefficients
 * using <code><a href="dlpmath.html#dlm_c2b">dlm_c2b</code></a>.</p>
 * <img src="../resources/base/dlp_math/filter_miir.gif" align="absmiddle">
 *
 * @param Y Output data instance holding filtered signals.
 * @param X Input data instance holding signals to filter.
 * @param A Filter coefficients.
 * @param nLambda Warping factor.
 * @param M Filter states (can be <code>NULL</code>).
 * @return Equal to {@link CSignal_MFilter}.
 *
 * @see CSignal_Fir CSignal_Iir CSignal_MIir CSignal_Filter CSignal_MFilter
 */
INT16 CGEN_PUBLIC CSignal_MIir(CData* Y, CData* X, CData* A, FLOAT64 nLambda, CData* M) {
  return CSignal_MFilter(Y, X, NULL, A, nLambda, M);
}

/**
 * <p>Mel-Linear Predictive Coding analysis.</p>
 *
 * <p>This operation calculates <code>nCoeff</code> Mel-LPC-coefficients and the square root of the estimation of the
 * variance of the residual signal from each frame given in the records of <code>idX</code> using warping factor
 * <code>nLambda</code> and method <code>lpsMethod</code>. The following methods
 * currently implemented:
 * <dl><dt><code>"Burg"</code><dd><name>Burg</name>'s Method and
 *     <dt><code>"Levinson"</code><dd><name>Levinson</name>-<name>Durbin</name>-recursion.
 * </dl></p>
 *
 * @param idY Output data instance.
 * @param idG Square root of estimation of the variance of the residual signal.
 * @param idX Input data instance.
 * @param nCoeff Number of LPC-coefficients to calculate.
 * @param nLambda Warping factor.
 * @param lpsMethod Calculation method.
 *
 */
INT16 CGEN_PUBLIC CSignal_MLpc(CData* idY, CData* idG, CData* idX, FLOAT64 nLambda, INT32 nCoeff, const char* lpsMethod) {
  INT16 nTS = -1;
  INT32 iR = 0;
  INT32 nCS = 0;
  INT32 nRS = 0;
  COMPLEX64 nScale = CMPLX(1.0);
  CData* idR = NULL;
  CData* idS = NULL;
  CData* idL = NULL;

  CSignal_GetVar(idX, "nScale", &nScale);
  FOP_PRECALC(idX, idY, idS, idR, idL);

  nCS = CData_GetNComps(idS);
  nRS = CData_GetNRecs(idS);
  nTS = CData_GetCompType(idS, 0);

  CData_Tconvert(idS, idS, T_DOUBLE);
  CData_Array(idR, T_DOUBLE, nCoeff, nRS);
  CData_Array(idG, T_DOUBLE, 1, nRS);
  CData_SetNBlocks(idR, CData_GetNBlocks(idS));
  CData_SetNBlocks(idG, CData_GetNBlocks(idS));

  if (!dlp_stricmp(lpsMethod, "burg")) {
    for (iR = 0; iR < nRS; iR++) {
      dlm_lpc_mburg((FLOAT64*) CData_XAddr(idS, iR, 0), nCS, (FLOAT64*) CData_XAddr(idR, iR, 0), nCoeff, nLambda, nScale.x);
      CData_Dstore(idG, CData_Dfetch(idR, iR, 0), iR, 0);
      CData_Dstore(idR, 1.0, iR, 0);
    }
  } else {
    for (iR = 0; iR < nRS; iR++) {
      dlm_lpc_mlev((FLOAT64*) CData_XAddr(idS, iR, 0), nCS, (FLOAT64*) CData_XAddr(idR, iR, 0), nCoeff, nLambda, nScale.x);
      CData_Dstore(idG, CData_Dfetch(idR, iR, 0), iR, 0);
      CData_Dstore(idR, 1.0, iR, 0);
    }
  }

  CData_CopyDescr(idG, idX);
  CDlpObject_CopySelective(BASEINST(idG), BASEINST(idX), WL_TYPE_INSTANCE);
  CData_Join(idG, idL);
  CData_Tconvert(idS, idS, nTS);
  FOP_POSTCALC(idX, idY, idS, idR, idL);
  CSignal_SetVar(idY, "nLambda", CMPLX(nLambda) );
  CSignal_SetVar(idY, "nGamma", CMPLX(-1.0) );

  return O_K;
}

/**
 * <p>Mel-Linear Predictive Coding to Generalized Cepstrum transform.</p>
 *
 * <p>This operation converts Mel-LPC coefficients to <code>n</code> Generalized-Cepstrum coefficients of the given
 * &gamma;. This operation is equal to call {@link CSignal_MLpc2MGCep} with &lambda;<sub>2</sub>=0.</p>

 * @param idY Output data instance.
 * @param idG Square root of estimation of the variance of the residual signal.
 * @param idX Input data instance.
 * @param nGamma2 Target Generalized Cepstrum factor.
 * @param n Number of G-Cepstrum coefficients to calculate.
 */
INT16 CGEN_PUBLIC CSignal_MLpc2GCep(CData* idY, CData* idG, CData* idX, FLOAT64 nGamma2, INT32 n) {
  return CSignal_MLpc2MGCep(idY, idG, idX, nGamma2, 0.0, n);
}

/**
 * <p>Mel-Linear Predictive Coding to Linear Predictive Coding transform.</p>
 *
 * <p>This operation transforms Mel-LPC coefficients to <code>n</code> LPC coefficients. This function is equal to call
 * {@link CSignal_MLpc2MLpc} with &lambda;<sub>2</sub>=0.
 *
 * @param idG2 Square root of estimation of the variance of the residual signal.
 * @param idA2 Output data instance.
 * @param idG1 Square root of estimation of the variance of the residual signal.
 * @param idA1 Input data instance.
 * @param n Number of Mel-LPC coefficients to calculate.

 */
INT16 CGEN_PUBLIC CSignal_MLpc2Lpc(CData* idA2, CData* idG2, CData* idG1, CData* idA1, INT32 n) {
  return CSignal_MLpc2MLpc(idA2, idG2, idG1, idA1, 0.0, n);
}

/**
 * <p>Mel-Linear Predictive Coding to Mel-Cepstrum transform.</p>
 *
 * <p>This operation converts Mel-LPC coefficients to <code>n</code> Mel-Cepstrum coefficients with warping factor &lambda;.

 * @param idY Output data instance.
 * @param idG Square root of estimation of the variance of the residual signal.
 * @param idX Input data instance.
 * @param nLambda2 Target warping factor.
 * @param n Number of Mel-Cepstrum coefficients to calculate.
 */
INT16 CGEN_PUBLIC CSignal_MLpc2MCep(CData* idY, CData* idG, CData* idX, FLOAT64 nLambda2, INT32 n) {
  DLPASSERT((idY!=NULL)&&(idG!=NULL)&&(idX!=NULL)&&(n>0));

  CData_Copy(BASEINST(idY), BASEINST(idX));

  CSignal_IGCepNorm(idY, idG, idY);
  CSignal_MGCep2MGCep(idY, idY, 0, nLambda2, n);
  return O_K;
}

/**
 * <p>Inverse Scalar Vector Quantization</p>
 *
 * <p>This is the inverse of {@link CSignal_Svq}. This function restores the input vector <code>idX</code> of
 * {@link CSignal_Svq} using the codebook given in <code>idQ</code> and the coded representation of <code>idX</code>
 * given in <code>idI</code>. The output <code>idY</code> generally differs from <code>idX</code> due to the
 * quantization.
 */
INT16 CGEN_PUBLIC CSignal_ISvq(CData* idY, CData* idI, CData* idQ) {
  CData* idBQ = NULL;
  CData* idBI = NULL;
  CData* idS = NULL;
  CData* idR = NULL;
  CData* idL = NULL;
  INT32 nCBQ = 0;
  INT32 nRBQ = 0;
  INT32 nCBI = 0;
  INT32 nRBI = 0;
  INT32 nCQ = 0;
  INT32 nCI = 0;
  INT32 nRI = 0;
  INT32 nRQ = 0;
  INT32 iC = 0;
  INT32 iR = 0;
  INT16 ret = O_K;

  DLPASSERT((idQ!=NULL) && (idI!=NULL));

  CSignal_GetData(idQ, "idBitTable", &idBQ);
  CSignal_GetData(idI, "idBitTable", &idBI);
  DLPASSERT((idBQ!=NULL) && (idBI!=NULL));

  nCI = CData_GetNComps(idI);
  nRI = CData_GetNRecs(idI);
  nRQ = CData_GetNRecs(idQ);
  nCQ = CData_GetNComps(idQ);
  nCBQ = CData_GetNComps(idBQ);
  nRBQ = CData_GetNRecs(idBQ);
  nCBI = CData_GetNComps(idBI);
  nRBI = CData_GetNRecs(idBI);
  DLPASSERT((nCBQ == nCBI) && (nCBQ == nCQ) && (nRBQ == nRBI));

  for (iC = 0; iC < nCBQ; iC++) {
    for (iR = 0; iR < nRBQ; iR++) {
      DLPASSERT(CData_Dfetch(idBQ, iR, iC) == CData_Dfetch(idBI, iR, iC));
    }
  }

  FOP_PRECALC(idI, idY, idS, idR, idL);

  CData_AddNcomps(idR, CData_GetCompType(idQ, 0), nCQ);
  CData_Allocate(idR, nRI);

  ret =
      dlm_isvq((FLOAT64*) CData_XAddr(idQ, 0, 0), nCQ, nRQ, (BYTE*) CData_XAddr(idS, 0, 0), nRI, nCI, (INT32*) CData_XAddr(idBQ, 0, 0), nRBQ, (FLOAT64*) CData_XAddr(idR, 0, 0));
  FOP_POSTCALC(idI, idY, idS, idR, idL);

  return ret;
}

/**
 * <p id=CSignal_IVq>Inverse Vector Quantization</p>
 *
 * <p>This function generates a vector stream according to the given index stream by codebook lookup.</p>
 *
 * @param idQ Input codebook
 * @param idI Input index sequence
 * @param idX Output vector sequence
 * @return <code>O_K</code> if successfull, <code>DATA_MDIM</code> if index exceeds codebook size, <code>ERR_MEM</code> if memory allocation failed.
 *
 * @see CSignal_Vq
 */
INT16 CGEN_PUBLIC CSignal_IVq(CData* idY, CData* idI, CData* idQ) {
  INT32 iR = 0;
  INT32 iC = 0;
  INT32 nCQ = 0;
  INT32 nCI = 0;
  INT32 nRQ = 0;
  INT32 nRI = 0;
  CData* idR = NULL;
  CData* idS = NULL;
  CData* idL = NULL;
  INT32* lpI = NULL;

  DLPASSERT((idQ != NULL) && (idY != NULL) && (idI != NULL));

  nCQ = CData_GetNComps(idQ);
  nRQ = CData_GetNRecs(idQ);
  nCI = CData_GetNComps(idI);
  nRI = CData_GetNRecs(idI);

  lpI = (INT32*) dlp_calloc(nRI,sizeof(INT32));
  if (!lpI) return ERR_MEM;

  FOP_PRECALC(idI, idY, idS, idR, idL);

  CData_Tconvert(idQ, idQ, T_DOUBLE); /* TODO: preserve original type */
  CData_AddNcomps(idR, CData_GetCompType(idQ, 0), nCQ);
  CData_Allocate(idR, nRI);

  for (iR = 0; iR < nRI; iR++) {
    for (iC = 0; iC < nCI; iC++) {
      lpI[iR] |= (INT32) CData_Dfetch(idI, iR, iC) << (iC * 8);
    }
    if (lpI[iR] > nRQ) {
      CERROR(CSignal, DATA_MDIM, ". index exceeds codebook size", 0, 0);
      dlp_free(lpI);
      return NOT_EXEC;
    }
  }

  dlm_ivq((FLOAT64*) CData_XAddr(idR, 0, 0), nRI, (FLOAT64*) CData_XAddr(idQ, 0, 0), nCQ, nRQ, lpI);

  FOP_POSTCALC(idI, idY, idS, idR, idL);

  dlp_free(lpI);
  return O_K;
}

/**
 * <p>Mel-Linear Predictive Coding to Mel-Generalized Cepstrum transform.</p>
 *
 * <p>This operation converts Mel-LPC coefficients to <code>n</code> Mel-Generalized-Cepstrum coefficients of the given
 * &gamma; and warping factor &lambda;.

 * @param idY Output data instance.
 * @param idG Square root of estimation of the variance of the residual signal.
 * @param idX Input data instance.
 * @param nGamma2 Target Generalized Cepstrum factor.
 * @param nLambda2 Target warping factor.
 * @param n Number of G-Cepstrum coefficients to calculate.
 */
INT16 CGEN_PUBLIC CSignal_MLpc2MGCep(CData* idY, CData* idG, CData* idX, FLOAT64 nGamma2, FLOAT64 nLambda2, INT32 n) {
  COMPLEX64 nLambda1 = CMPLX(0.0);

  DLPASSERT((idY!=NULL)&&(idG!=NULL)&&(idX!=NULL)&&(n>0));

  CSignal_GetVar(idX, "nLambda", &nLambda1);
  CData_Copy(BASEINST(idY), BASEINST(idX));

  CSignal_IGCepNorm(idY, idG, idY);
  if ((nGamma2 != -1) || (nLambda1.x != nLambda2)) {
    CSignal_MGCep2MGCep(idY, idY, nGamma2, nLambda2, n);
  }
  return O_K;
}

/**
 * <p>Mel-Linear Predictive Coding to Mel-Linear Predictive Coding transform.</p>
 *
 * <p>This operation transforms Mel-LPC coefficients to <code>n</code> Mel-LPC coefficients with the given target
 * warping factor &lambda;.
 *
 * @param idG2 Square root of estimation of the variance of the residual signal.
 * @param idA2 Output data instance.
 * @param idG1 Square root of estimation of the variance of the residual signal.
 * @param idA1 Input data instance.
 * @param nLambda2 Target warping factor.
 * @param n Number of Mel-LPC coefficients to calculate.
 */
INT16 CGEN_PUBLIC CSignal_MLpc2MLpc(CData* idA2, CData* idG2, CData* idG1, CData* idA1, FLOAT64 nLambda2, INT32 n) {
  FLOAT64 nLambda = 0.0;
  COMPLEX64 nLambda1 = CMPLX(0.0);
  INT32 iR = 0;
  INT32 iC = 0;
  INT32 nCS = 0;
  INT32 nRS = 0;
  CData* idR = NULL;
  CData* idS = NULL;
  CData* idL = NULL;

  DLPASSERT((idG1!=NULL)&&(idG2!=NULL)&&(n>0));

  CSignal_GMult(idA1, idA1);

  CSignal_GetVar(idA1, "nLambda", &nLambda1);
  nLambda = (nLambda2 - nLambda1.x) / (1.0 - nLambda1.x * nLambda2);

  FOP_PRECALC(idA1, idA2, idS, idR, idL);

  nCS = CData_GetNComps(idS);
  nRS = CData_GetNRecs(idS);

  CData_Array(idR, T_DOUBLE, n, nRS);
  CData_SetNBlocks(idR, CData_GetNBlocks(idS));
  CData_Copy(BASEINST(idG2), BASEINST(idG1));

  for (iR = 0; iR < nRS; iR++) {
    dlm_lpc2mlpc((FLOAT64*) CData_XAddr(idS, iR, 0), nCS, (FLOAT64*) CData_XAddr(idR, iR, 0), n, nLambda);
    *(FLOAT64*) CData_XAddr(idG2, iR, 0) = *(FLOAT64*) CData_XAddr(idG1, iR, 0) / *(FLOAT64*) CData_XAddr(idR, iR, 0);
    for (iC = n - 1; iC >= 0; iC--) {
      *(FLOAT64*) CData_XAddr(idR, iR, iC) = *(FLOAT64*) CData_XAddr(idR, iR, iC) / *(FLOAT64*) CData_XAddr(idR, iR, 0);
    }
  }

  FOP_POSTCALC(idA1, idA2, idS, idR, idL);
  CSignal_SetVar(idA2, "nLambda", CMPLX(nLambda2) );
  CSignal_SetVar(idA2, "nGamma", CMPLX(-1.0) );

  CSignal_IGMult(idA2, idA2);

  return O_K;
}

/**
 * <p>Mel-Line Spectrum Frequencies to Mel-Line Spectrum Frequencies transform
 *
 * <p>This operation transforms Mel-LSF with a particular source warping factor &lambda;<sub>1>/sub> to Mel-LSF parameters with the given target
 * warping factor &lambda;<sub>2</sub>.
 *
 * @param idY Output data instance holding the target LSF.
 * @param idX Input data instance holding the source LSF.
 * @param nLambda2 Target warping factor.
 */
INT16 CGEN_PUBLIC CSignal_MLsf2MLsf(CData* idY, CData* idX, FLOAT64 nLambda2) {
  FLOAT64 nLambda = 0.0;
  COMPLEX64 nGamma = CMPLX(0.0);
  COMPLEX64 nLambda1 = CMPLX(0.0);
  INT32 iR = 0;
  INT32 nCS = 0;
  INT32 nRS = 0;
  CData* idR = NULL;
  CData* idS = NULL;
  CData* idL = NULL;

  DLPASSERT((idY!=NULL)&&(idX!=NULL));

  CSignal_GetVar(idX, "nLambda", &nLambda1);
  CSignal_GetVar(idX, "nGamma", &nGamma);
  nLambda = (nLambda2 - nLambda1.x) / (1.0 - nLambda1.x * nLambda2);

  FOP_PRECALC(idX, idY, idS, idR, idL);

  nCS = CData_GetNComps(idS);
  nRS = CData_GetNRecs(idS);

  CData_Array(idR, T_DOUBLE, nCS, nRS);
  CData_SetNBlocks(idR, CData_GetNBlocks(idS));
  CData_Copy(BASEINST(idY), BASEINST(idX));

  for (iR = 0; iR < nRS; iR++) {
    dlm_lsf2mlsf((FLOAT64*) CData_XAddr(idS, iR, 0), (FLOAT64*) CData_XAddr(idR, iR, 0), nCS, nLambda);
  }

  FOP_POSTCALC(idX, idY, idS, idR, idL);
  CSignal_SetVar(idY, "nLambda", CMPLX(nLambda2) );

  return O_K;
}

/**
 * <p>Modulated Lapped Transform (MLT)
 *
 * <p>This operation calculates the Modulated Lapped Transform coefficients from framed input signal.
 *
 * @param idY Output data instance holding the MLT.
 * @param idX Input data instance holding the framed signal.
 */
INT16 CGEN_PUBLIC CSignal_Mlt(CData* idY, CData* idX) {
  INT16 nTS = -1;
  INT32 i = 0;
  INT32 nR = 0;
  INT32 nC = 0;
  INT16 nErr = O_K;
  COMPLEX64 nScale = CMPLX(1);
  CData* idR = NULL;
  CData* idS = NULL;
  CData* idL = NULL;

  CSignal_GetVar(idX, "nScale", &nScale);
  FOP_PRECALC(idX, idY, idS, idR, idL);

  nR = CData_GetNRecs(idS);
  nC = CData_GetNComps(idS);
  nTS = CData_GetCompType(idS, 0);

  CData_Tconvert(idS, idS, T_DOUBLE);
  CData_Array(idR, T_DOUBLE, nC / 2, nR);
  CData_SetNBlocks(idR, CData_GetNBlocks(idS));

  nErr = dlm_mlt((FLOAT64*) CData_XAddr(idR, i, 0), (FLOAT64*) CData_XAddr(idS, i, 0), nC, nR);

  CData_Tconvert(idS, idS, nTS);
  FOP_POSTCALC(idX, idY, idS, idR, idL);

  idY->m_nCinc = 1.0 / idX->m_nCinc / nC; /* Set phys. unit of comp. axis      */
  if (dlp_strcmp(idX->m_lpCunit, "s") == 0) dlp_strcpy(idY->m_lpCunit, "Hz");/* Rename abscissa "s"   -> "Hz"     */
  else if (dlp_strcmp(idX->m_lpCunit, "ms") == 0) dlp_strcpy(idY->m_lpCunit, "kHz");/* Rename abscissa "ms"  -> "kHz"    */
  else dlp_strcpy(idY->m_lpCunit, "");/* None of the above units -> clear  */
  return nErr;
}

/**
 * <p>Noisifying signal</p>
 *
 * <p>This operation add white noise to the signal. The magnitude of the noise is less than the quantization error
 * of the data type given in the field of type <code>var</code> <code>idX.nType</code>.
 *
 * @param idY Output data instance.
 * @param idX Input data instance.
 */
INT16 CGEN_PUBLIC CSignal_Noisify(CData* idY, CData* idX) {
  INT32 iR;
  INT32 iC;
  INT32 nR;
  INT32 nC;
  FLOAT64 nRand;
  COMPLEX64 nRandC;
  COMPLEX64 nQuant;
  COMPLEX64 nType = CMPLX(T_DOUBLE);
  COMPLEX64 nScale = CMPLX(1);

  DLPASSERT((idY!=NULL)&&(idX!=NULL));

  CSignal_GetVar(idX, "nType", &nType);
  CSignal_GetVar(idX, "nScale", &nScale);

  nQuant = CSignal_GetMinQuant((INT16) nType.x, nScale);

  CData_Copy(BASEINST(idY), BASEINST(idX));
  nR = CData_GetNRecs(idY);
  nC = CData_GetNComps(idY);

  for (iC = 0; iC < nC; iC++) {
    if (!dlp_is_symbolic_type_code(CData_GetCompType(idY, iC))) {
      if (dlp_is_complex_type_code(CData_GetCompType(idY, iC))) {
        for (iR = 0; iR < nR; iR++) {
          nRandC = CMPLXY(dlp_rand()/(FLOAT64)RAND_MAX-0.5,dlp_rand()/(FLOAT64)RAND_MAX-0.5);
          nRandC = CMPLX_MULT(nRandC,nQuant);
          CData_Cstore(idY, dlp_scalopC(CData_Cfetch(idY, iR, iC), nRandC, OP_ADD), iR, iC);
        }
      } else {
        for (iR = 0; iR < nR; iR++) {
          nRand = (dlp_rand() / (FLOAT64) RAND_MAX - 0.5) * nQuant.x;
          CData_Dstore(idY, CData_Dfetch(idY, iR, iC) + nRand, iR, iC);
        }
      }
    }
  }
  return O_K;
}

/**
 * <p>Pitch marking.</p>
 *
 * <p>This operation calculates pitch marks from the input signal stored in <code>idS</code>. You can force the
 * algorithm to search within minimum and maximum fundamental frequency by setting <code>nMin</code> and
 * <code>nMax</code> to the frequencies in Hz. Unvoiced periods lengths are set according to <code>nMean</code>
 * specified in Hz. There are several pitch marking algorithms selectable by <code>sMethod</code>:
 * <ul>
 *   <li>"gcida"  - Glottal Closure Instance Detection Algorithm</li>
 *   <li>"chfa"   - CURRENTLY NOT IMPLEMENTED</li>
 *   <li>"epoch"  - CURRENTLY NOT IMPLEMENTED</li>
 *   <li>"hybrid" - CURRENTLY NOT IMPLEMENTED</li>
 * </ul>
 *
 * @param idY     - Output pitchmarks
 * @param idX     - Input signal
 * @param sMethod - Algorithm
 * @param nMin    - Minimum fundamental frequency
 * @param nMean   - Mean fundamental frequency (unvoiced markers set according to this value
 * @param nMax    - Maximum fundamental frequency
 */
INT16 CGEN_PUBLIC CSignal_Pitchmark(CData* idY, CData* idX, char* sMethod, INT32 nMin, INT32 nMean, INT32 nMax) {
  INT32   nR  = 0;
  INT32   nSR = 0;
  INT32   nP  = 0;
  INT32   iP  = 0;
  CData*  idS = NULL;
  CData*  idR = NULL;
  CData*  idL = NULL;
  PERIOD* pP  = NULL;

  DLPASSERT((idY!=NULL)&&(idX!=NULL));

  if(CData_GetDescr(idX,RINC) <= 0.0) {
    CERROR(CSignal,FOP_ERR_INVDESCR,"sampling rate of input signal",0,0);
    return NOT_EXEC;
  }

  FOP_PRECALC(idX, idY, idS, idR, idL);
  nSR = (INT32)(1000.0 / CData_GetDescr(idS,RINC) + 0.5);
  nR = CData_GetNRecs(idS);
  CData_Tconvert(idS, idS, T_DOUBLE);

  if((sMethod == NULL) || (dlp_strlen(sMethod) <= 0) || !dlp_strcmp("gcida",sMethod)) {
    dlm_gcida((FLOAT64*)CData_XAddr(idS,0,0),nR,&pP, &nP, nSR, nMin, nMean, nMax);
  }

  CData_Reset(BASEINST(idR),TRUE);
  CData_CopyDescr(idR,idS);
  CData_AddComp(idR,"pm",T_SHORT);
  CData_AddComp(idR,"v/uv",T_SHORT);
  CData_Allocate(idR, nP);

  for(iP = 0; iP < nP; iP++) {
    CData_Dstore(idR,pP[iP].nPer, iP, 0);
    CData_Dstore(idR,pP[iP].stimulation & 1, iP, 1);
  }
  if(pP != NULL) dlp_free(pP);

  FOP_POSTCALC(idX, idY, idS, idR, idL);

  return O_K;
}

/**
 * <p>Polynomial to Line Spectral Frequencies transform</p>
 *
 * <p>This operation transforms a polynomial (e. g. LPC) to Line Spectral Frequencies.</p>
 *
 * @param idY Output data instance.
 * @param idX Input data instance.
 */
INT16 CGEN_PUBLIC CSignal_Poly2Lsf(CData* idY, CData* idX) {
  INT16 ret = O_K;
  INT16 nTS = -1;
  INT32 iR = 0;
  INT32 nCS = 0;
  INT32 nRS = 0;
  CData* idR = NULL;
  CData* idS = NULL;
  CData* idL = NULL;

  FOP_PRECALC(idX, idY, idS, idR, idL);

  nCS = CData_GetNComps(idS);
  nRS = CData_GetNRecs(idS);
  nTS = CData_GetCompType(idS, 0);

  CData_Tconvert(idS, idS, T_DOUBLE);
  CData_Array(idR, T_DOUBLE, nCS, nRS);
  CData_SetNBlocks(idR, CData_GetNBlocks(idS));

  for (iR = 0; iR < nRS; iR++) {
    if (dlm_poly2lsf((FLOAT64*) CData_XAddr(idS, iR, 0), (FLOAT64*) CData_XAddr(idR, iR, 0), nCS) < 0) ret = NOT_EXEC;
  }

  CData_Delete(idR, idR, 0, 1);
  CData_Tconvert(idS, idS, nTS);

  FOP_POSTCALC(idX, idY, idS, idR, idL);

  return ret;
}

/**
 * <p>Remove direct component from signal.</p>
 *
 * <p>This operation calculates the mean of the samples of each record and removes them.</p>
 *
 * @param idY Output data instance.
 * @param idX Input data instance.
 * @return <code>O_K</code>
 */
INT16 CGEN_PUBLIC CSignal_Rmdc(CData* idY, CData* idX) {
  INT32 iR = 0;
  INT32 iC = 0;
  INT32 nCS = 0;
  INT32 nRS = 0;
  COMPLEX64 sum = CMPLX(0.);
  CData* idR = NULL;
  CData* idS = NULL;
  CData* idL = NULL;

  FOP_PRECALC(idX, idY, idS, idR, idL);

  nCS = CData_GetNComps(idS);
  nRS = CData_GetNRecs(idS);

  CData_Array(idR, T_DOUBLE, nCS, nRS);
  CData_SetNBlocks(idR, CData_GetNBlocks(idS));

  for (iR = 0; iR < nRS; iR++) {
    sum = CMPLX(0.0);
    for (iC = 0; iC < nCS; iC++)
      sum = CMPLX_PLUS(sum,CData_Cfetch(idS, iR, iC));
    sum = CMPLX_DIV_R(sum, nCS);
    for (iC = 0; iC < nCS; iC++)
      CData_Cstore(idR, CMPLX_MINUS(CData_Cfetch(idS, iR, iC),sum), iR, iC);
  }

  FOP_POSTCALC(idX, idY, idS, idR, idL);

  return O_K;
}

/**
 * <p>Roots of polynomials.</p>
 *
 * <p>This operation calculates the roots of the polynomials of each record.</p>
 *
 * @param idY Output data instance.
 * @param idX Input data instance.
 * @return <code>O_K</code>
 */
INT16 CGEN_PUBLIC CSignal_Roots(CData* idY, CData* idX) {
  INT16 nTS = -1;
  INT32 iR = 0;
  INT32 nCS = 0;
  INT32 nRS = 0;
  CData* idR = NULL;
  CData* idS = NULL;
  CData* idL = NULL;

  FOP_PRECALC(idX, idY, idS, idR, idL);

  nCS = CData_GetNComps(idS);
  nRS = CData_GetNRecs(idS);
  nTS = CData_GetCompType(idS, 0);

  CData_Tconvert(idS, idS, T_DOUBLE);
  CData_Array(idR, T_COMPLEX, nCS - 1, nRS);
  CData_SetNBlocks(idR, CData_GetNBlocks(idS));

  for (iR = 0; iR < nRS; iR++) {
    dlm_roots((FLOAT64*) CData_XAddr(idS, 0, 0), (COMPLEX64*) CData_XAddr(idR, 0, 0), nCS);
  }

  CData_Tconvert(idS, idS, nTS);
  FOP_POSTCALC(idX, idY, idS, idR, idL);

  return O_K;
}

/**
 * <p>Scales the signal by factor <code>nScale</code>. If the input data instance <code>idY</code> is not homogeneous or
 * the first non-symbolic component is not of float/complex-type the output will converted to double type. In all cases
 * two instances of type var will be added to the output data instance: <code>idY.nScale</code> contains the value of
 * <code>nScale</code> multiplied by a possibly existent field <code>idX.nScale</code>. If <code>idX.nType</code>
 * exists it will be copied to <code>idY.nType</code>, otherwise <code>idY.nType</code> contains the type of the first
 * non-symbolic component of <code>idX</code>.</p>
 *
 * @param idY    Output data instance.
 * @param idX    Input data instance.
 * @param nScale Scale factor
 * @return       <code>O_K</code>
 *
 * @see CSignal_DeScale
 *
 */
INT16 CGEN_PUBLIC CSignal_Scale(CData* idY, CData* idX, COMPLEX64 nScale) {
  return CSignal_ScaleImpl(idY, idX, nScale);
}

/**
 * <p>Scalar Vector Quantization</p>
 *
 * <p>Each of the <code>N</code> components of the <code>M</code> vectors of <code>idX</code> were independently
 * quantized. The input <code>idB</code> contains the bit-table of length <code>N</code>. The resulting codebook is
 * written to <code>idQ</code> and consists of <code>N</code> vectors of length <code>L</code>, were <code>L</code> is
 * <code>2<sup>max(idB)</sup></code>. The data instance <code>idQ</code> could contain <code>NaN</code>'s if the number
 * of quantized values of a component <code>n</code> is less than <code>L</code>, i.e.
 * <code>2<sup>idB[n]</sup>&lt;2<sup>max(idB)</sup></code>. The output <code>idI</code> consists of the coded
 * representation of the input <code>idX</code>.
 */
INT16 CGEN_PUBLIC CSignal_Svq(CData* idQ, CData* idI, CData* idX, CData* idB) {
  INT16 ret = O_K;
  INT16 nTS = -1;
  INT32 nCB = 0;
  INT32 nCX = 0;
  INT32 nRX = 0;
  INT32 nRQ = 0;
  INT32 nRB = 0;
  INT32 iC = 0;
  INT32 nBit = 0;
  INT32 nCR = 0;
  INT32 nBits = 0;
  CData* idR = NULL;
  CData* idS = NULL;
  CData* idL = NULL;

  DLPASSERT((idB != NULL) && (idX != NULL));

  nCB = CData_GetNComps(idB);
  nRB = CData_GetNRecs(idB);
  nCX = CData_GetNComps(idX);
  nRX = CData_GetNRecs(idX);

  DLPASSERT(nCB == nCX);
  DLPASSERT(nRB > 0);

  CData_Tconvert(idB, idB, T_INT);

  for (iC = 0; iC < nCB; iC++) {
    nBit = (INT32) CData_Dfetch(idB, 0, iC);
    nRQ = MAX(dlm_pow2(nBit), nRQ);
    nBits += nBit;
  }
  nCR = MAX(ceil(nBits / 8.0), 1);

  FOP_PRECALC(idX, idI, idS, idR, idL);

  nTS = CData_GetCompType(idS, 0);
  CData_Tconvert(idS, idS, T_DOUBLE);
  CData_AddNcomps(idR, T_BYTE, nCR);
  CData_AddNcomps(idQ, T_DOUBLE, nCX);
  CData_Allocate(idR, nRX);
  CData_Allocate(idQ, nRQ);

  if ((ret =
      dlm_svq((FLOAT64*) CData_XAddr(idX, 0, 0), nRX, nCX, (INT32*) CData_XAddr(idB, 0, 0), nRB, (BYTE*) CData_XAddr(idR, 0, 0), &nCR, (FLOAT64*) CData_XAddr(idQ, 0, 0), &nRQ)) == O_K) {
    CData_Reallocate(idQ, nRQ);
    CData_Select(idR, idR, 0, nCR);
    CSignal_SetData(idI, "idBitTable", idB);
    CSignal_SetData(idQ, "idBitTable", idB);
  }

  CData_Tconvert(idS, idS, nTS);
  CData_Tconvert(idQ, idQ, nTS);
  FOP_POSTCALC(idX, idI, idS, idR, idL);

  return ret;
}

/**
 * <p id=CSignal_Unwrap>Unwraps radian phases given in imaginary part of <code>idY</code> to their 2&pi; complement if the
 * phase jumps greater than &pi;.</p>
 *
 * @param idX Input complex (radian) data instance containing wrapped phase in imaginary part
 * @param idY Output complex (radian) data instance containing unwrapped phase in imaginary part
 * @return <code>O_K</code> if successfull, a (negative) error code otherwise
 */
INT16 CGEN_PUBLIC CSignal_Unwrap(CData* idY, CData* idX) {
  INT16 ret = O_K;
  INT32 iR = 0;
  INT32 nCS = 0;
  INT32 nRS = 0;
  CData* idR = NULL;
  CData* idS = NULL;
  CData* idL = NULL;

  FOP_PRECALC(idX, idY, idS, idR, idL);

  nCS = CData_GetNComps(idS);
  nRS = CData_GetNRecs(idS);

  CData_Copy(BASEINST(idR), BASEINST(idS));

  for (iR = 0; iR < nRS; iR++) {
    if ((ret = dlm_unwrapC((COMPLEX64*) CData_XAddr(idR, iR, 0), nCS)) != O_K) break;
  }

  FOP_POSTCALC(idX, idY, idS, idR, idL);

  return ret;
}

/**
 * <p id=CSignal_Vq>Vector Quantization</p>
 *
 * <p>The vectors of <code>idX</code> where used to generate <code>2<sup>nBits</sup></code> codebook
 * vectors using LGB-Algorithm. The codebook is stored to <code>idQ</code>. At the end <code>idI</code>
 * contains the index sequence of the nearest codebook vectors compared to <code>idX</code>. The index
 * sequence is stored byte-wise spanning the components of <code>idI</code>.</p>
 *
 * @param idQ Output codebook
 * @param idI Output index sequence mapping <code>idQ</code> and <code>idX</code>
 * @param idX Input training vectors
 * @param nBits Input bit width of the indices of <code>idI</code>, e.g. number of codebook vectors = <code>2<sup>nBits</sup></code>
 * @param sMethod String indicating cluster method: <code>LBG</code> or <code>PAM</code> currently supported.
 * @return <code>O_K</code> if successfull, <code>FOP_ERR_INVARG</code> if <code>2<sup>nBits</sup>>idX.nrec</code>, <code>ERR_MEM</code> if memory allocation failed.
 */
INT16 CGEN_PUBLIC CSignal_Vq(CData* idQ, CData* idI, CData* idX, INT32 nBits, const char* sMethod) {
  INT16 ret = O_K;
  INT16 nTX = -1;
  INT32 iRX = 0;
  INT32 iC = 0;
  INT32 nCR = 0;
  INT32 nCX = 0;
  INT32 nRX = 0;
  INT32 nRQ = 0;
  CData* idR = NULL;
  CData* idS = NULL;
  CData* idL = NULL;
  INT32* lpI = NULL;
  char sBits[16] = "\0";

  DLPASSERT(idX != NULL);

  nCX = CData_GetNComps(idX);
  nRX = CData_GetNRecs(idX);
  nTX = CData_GetCompType(idX, 0);

  CData_Tconvert(idX, idX, T_DOUBLE);
  nCR = MAX(ceil(nBits / 8.0), 1);
  nRQ = dlm_pow2(nBits);

  if (nRQ > nRX) {
    sprintf(sBits, "%d", nBits);
    CERROR(CSignal, FOP_ERR_INVARG, "nBits", sBits, ": Number of input vectors (nRX) less than number of codebook vectors (2^nBits)");
    return NOT_EXEC;
  }

  lpI = (INT32*) dlp_calloc(nRX,sizeof(INT32));
  if (!lpI) return ERR_MEM;

  FOP_PRECALC(idX, idI, idS, idR, idL);

  CData_AddNcomps(idR, T_BYTE, nCR);
  CData_AddNcomps(idQ, T_DOUBLE, nCX);
  CData_Allocate(idR, nRX);
  CData_Allocate(idQ, nRQ);

  if (!dlp_strcmp("PAM", sMethod)) {
    dlm_pam((FLOAT64*) CData_XAddr(idS, 0, 0), nCX, nRX, (FLOAT64*) CData_XAddr(idQ, 0, 0), nRQ);
  } else {
    dlm_lbg((FLOAT64*) CData_XAddr(idS, 0, 0), nCX, nRX, (FLOAT64*) CData_XAddr(idQ, 0, 0), nRQ);
  }
  dlm_vq((FLOAT64*) CData_XAddr(idS, 0, 0), nRX, (FLOAT64*) CData_XAddr(idQ, 0, 0), nCX, nRQ, lpI);

  for (iRX = 0; iRX < nRX; iRX++) {
    for (iC = 0; iC < nCR; iC++) {
      CData_Dstore(idI, (lpI[iRX] >> (iC * 8)) & 0xFF, iRX, iC);
    }
  }

  FOP_POSTCALC(idX, idI, idS, idR, idL);
  CData_Tconvert(idX, idX, nTX);
  CData_Tconvert(idQ, idQ, nTX);

  dlp_free(lpI);
  return ret;
}

/**
 * <p>Frame windowing.</p>
 *
 * <p>The Frames given in the records of <code>idX</code> were windowed by the window of length <code>nLenIn</code> given
 * in <code>lpsWindow</code>. If <code>bNorm</code> is <code>TRUE</code> the window will be normalized. If
 * <code>nLenIn&le;0</code> or <code>nLenIn</code> greater than the the amount of numeric components of <code>idX</code>,
 * <code>nLenIn</code> will be set to this amount. The resulting windowed frames are zero padded to length
 * <code>nLenOut</code>.</p>
 *
 * @param idY Output data instance.
 * @param idX Input data instance.
 * @param nLenIn Length of window.
 * @param nLenOut Output frame length
 * @param lpsWindow Window name.
 * @param bNorm Window normalization.
 * @return <code>O_K</code>
 */
INT16 CGEN_PUBLIC CSignal_Window(CData* idY, CData* idX, INT32 nLenIn, INT32 nLenOut, const char* lpsWindow, BOOL bNorm) {
  INT16 nRetVal = O_K;
  INT32 i, j;
  INT32 nCS = 0;
  INT32 nRS = 0;
  FLOAT64* lpWindow = NULL;
  CData* idR = NULL;
  CData* idS = NULL;
  CData* idL = NULL;

  FOP_PRECALC(idX, idY, idS, idR, idL);

  nCS = CData_GetNComps(idS);
  nRS = CData_GetNRecs(idS);

  if ((nLenIn <= 0) || (nLenIn > nCS)) nLenIn = nCS;
  if (nLenOut < nLenIn) nLenOut = nLenIn;

  lpWindow = (FLOAT64*) dlp_calloc(nLenIn,sizeof(FLOAT64));
  if (lpWindow) {
    CData_Copy(BASEINST(idR), BASEINST(idS));
    dlm_fba_makewindow(lpWindow, nLenIn, lpsWindow, bNorm);
    for (i = 0; i < nRS; i++) {
      for (j = 0; j < nLenIn; j++) {
        CData_Cstore(idR, CMPLX_MULT_R(CData_Cfetch(idR,i,j),lpWindow[j]), i, j);
      }
    }
    CData_AddNcomps(idR, CData_GetCompType(idS, 0), nLenOut - nLenIn);
    dlp_free(lpWindow);
  } else nRetVal = ERR_MEM;

  FOP_POSTCALC(idX, idY, idS, idR, idL);

  return nRetVal;
}

/**
 * <p>Zero-crossing counter</p>
 * This operation counts the zero-crossing instances selective to their step sizes falling in the intervals given in
 * <code>idP</code>. Zero crossing for complex signals are consecutive values with phase step of &#8805;&pi;
 *
 * @param idY Output data instance containing the counts of each interval
 * @param idX Input (complex) data instance containing the samples
 * @param idP Input data instance defining the intervals
 * @return <code>O_K</code>
 */
INT16 CGEN_PUBLIC CSignal_Zcr(CData* idY, CData* idX, CData* idP) {
  INT32 iR = 0;
  INT32 iC = 0;
  INT32 iCP = 0;
  INT32 nCS = 0;
  INT32 nRS = 0;
  INT32 nCP = 0;
  FLOAT64 dist = 0.0;
  COMPLEX64 val0 = CMPLX(0.0);
  COMPLEX64 val1 = CMPLX(0.0);
  CData* idR = NULL;
  CData* idS = NULL;
  CData* idL = NULL;

  FOP_PRECALC(idX, idY, idS, idR, idL);

  nCS = CData_GetNComps(idS);
  nRS = CData_GetNRecs(idS);
  nCP = CData_GetNComps(idP);

  CData_Array(idR, T_INT, nCP, nRS);
  CData_SetNBlocks(idR, CData_GetNBlocks(idS));

  for (iR = 0; iR < nRS; iR++) {
    for (iC = 1; iC < nCS; iC++) {
      val0 = CData_Cfetch(idS, iR, iC - 1);
      val1 = CData_Cfetch(idS, iR, iC);
      if (ABS(CMPLX_ANGLE(val0)-CMPLX_ANGLE(val1)) >= F_PI) {
        dist = CMPLX_ABS(CMPLX_MINUS(val1,val0));
        for (iCP = 0; iCP < nCP; iCP++) {
          if (dist > CData_Dfetch(idP, 0, iCP)) {
            CData_Dstore(idR, CData_Dfetch(idR, iR, iCP) + 1, iR, iCP);
          }
        }
      }
    }
  }

  FOP_POSTCALC(idX, idY, idS, idR, idL);

  return O_K;
}
