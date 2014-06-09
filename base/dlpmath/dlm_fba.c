/* dLabPro mathematics library
 * - Frame based analysis functions
 *
 * AUTHOR : Frank Duckhorn
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

INT16 CGEN_IGNORE dlm_fba_makewindow(FLOAT64 *lpWindow, INT32 nWlen, const char* lpsWtype, INT16 nWnorm) {
  INT32 i = 0;
  FLOAT64 c1 = 0.54f;
  FLOAT64 c2 = 0.46f;
  FLOAT64 c = 0.0;
  FLOAT64 norm = 0.0;

  /* Determine window type from field m_lpsWtype and calculate window */
  if (dlp_strnicmp(lpsWtype, "custom", 255)) {
    DLPASSERT(lpWindow);

    c = (FLOAT64) ((F_PI + F_PI) / (FLOAT64) (nWlen - 1));

    if (!dlp_strnicmp(lpsWtype, "hamming", 255)) for (i = 0; i < nWlen; i++)
      lpWindow[i] = c1 - c2 * (FLOAT64) cos((FLOAT64) i * c);
    else if (!dlp_strnicmp(lpsWtype, "hanning", 255)) for (i = 0; i < nWlen; i++)
      lpWindow[i] = 0.5f - 0.5f * (FLOAT64) cos((FLOAT64) i * c);
    else if (!dlp_strnicmp(lpsWtype, "rectangle", 255)) for (i = 0; i < nWlen; i++)
      lpWindow[i] = 1;
    else if (!dlp_strnicmp(lpsWtype, "triangle", 255)) for (i = 0; i < (nWlen / 2); i++) {
      lpWindow[i] = c * i;
      lpWindow[nWlen - 1 - i] = lpWindow[i];
    }
    else if (!dlp_strnicmp(lpsWtype, "gauss", 255)) {
      c = 1.f / ((nWlen) / 2);
      for (i = 0; i < (nWlen / 2); i++) {
        lpWindow[(nWlen / 2) + i] = (FLOAT64) exp(-(2 * (FLOAT64) dlm_pow((FLOAT64) i * c, 2)));
        lpWindow[(nWlen / 2) - 1 - i] = lpWindow[(nWlen / 2) + i];
      }
    } else if (!dlp_strnicmp(lpsWtype, "symexp", 255)) {
      c = 1.f / ((nWlen) / 2);
      for (i = 0; i < (nWlen / 2); i++) {
        lpWindow[(nWlen / 2) + i] = (FLOAT64) exp(-((FLOAT64) i * c));
        lpWindow[(nWlen / 2) - 1 - i] = lpWindow[(nWlen / 2) + i];
      }
    } else if (!dlp_strnicmp(lpsWtype, "blackman", 255)) for (i = 0; i < nWlen; i++)
      lpWindow[i] = 0.42 - 0.5 * (FLOAT64) cos((FLOAT64) i * c) + 0.08 * (FLOAT64) cos((FLOAT64) (i + i) * c);
    else if (!dlp_strnicmp(lpsWtype, "bartlett", 255)) {
      for (i = 0; i < nWlen / 2; i++)
        lpWindow[i] = (FLOAT64) (i) / (FLOAT64) (nWlen / 2);
      for (i = nWlen / 2; i < nWlen; i++)
        lpWindow[i] = (FLOAT64) (nWlen - 1 - i) / (FLOAT64) (nWlen / 2);
    } else return NOT_EXEC;
  }

  /* Normalize to energy */
  if (nWnorm != 0) {
    for (i = 0; i < nWlen; i++)
      norm = norm + lpWindow[i] * lpWindow[i];
    for (i = 0; i < nWlen; i++)
      lpWindow[i] = lpWindow[i] / (FLOAT64) sqrt(norm);
  }

  return O_K;
}

INT16 CGEN_IGNORE dlm_fba_doframing_maxframelen(INT32 nXSamples, INT64 *lpPitch, INT32 nXPitch, INT16 nNPeriods, INT32 nLen, INT32 nCrate, INT32 nWlen, INT32 *nMaxFrameLength, INT32 *nXFrames) {
  INT32 i = 0;
  INT32 nOffset = 0;
  INT32 nFrame = 0;
  INT32 nPSOffset = 0; /* Offset in samples (accumulated number of already processed periods) */
  INT32 nPPOffset = 0; /* Offset in periods (record number in dPitch) */
  INT32 nFrameLength = 0;
  INT32 nPeriodLength = 0;
  INT32 nPrevFrameLength = 0;
  char bLastFrame = FALSE;
  for (nFrame = 0; bLastFrame == FALSE; nFrame++) {
    /* Last frame? */
    if ((nOffset + nPrevFrameLength) >= nXSamples) bLastFrame = TRUE;

    if (nPPOffset < nXPitch) {
      if (nOffset >= nPSOffset) {
        nPeriodLength = lpPitch[nPPOffset];
        /* The frame may contain n following frames (as much as fit completely in the analysis window) */
        for (i = 1, nFrameLength = nPeriodLength; i <= nNPeriods; i++) {
          INT32 nNext = lpPitch[nPPOffset + i];
          if ((nFrameLength + nNext) < nLen) nFrameLength += nNext;
          else break;
        }
        nPPOffset += 1;
        nPSOffset += nPeriodLength;
        if (nOffset > nPSOffset) nOffset = nOffset - nCrate + nPeriodLength;
      }
    } else nFrameLength = nWlen;
    if (nFrameLength > *nMaxFrameLength) *nMaxFrameLength = nFrameLength;
    nPrevFrameLength = nFrameLength;
  }
  *nXFrames = nFrame;
  return O_K;
}

INT16 CGEN_IGNORE dlm_fba_doframing_initparam(struct dlm_fba_doframing_param *lpParam) {
  lpParam->nLambda = 0.0;
  lpParam->nCrate = 100;
  lpParam->nDC = 0.0;
  lpParam->nLen = 512;
  lpParam->nMinLog = log(dlm_pow(2, -16 + 1));
  lpParam->nNPeriods = 2;
  lpParam->nPreem = 0.0;
  lpParam->nWlen = 400;
  lpParam->nWnorm = 0;
  strcpy(lpParam->lpsWtype, "Hamming");
  lpParam->bEnergy = FALSE;
  lpParam->bLogEnergy = FALSE;
  lpParam->bTimeDomainWarping = FALSE;
  return O_K;
}

INT16 CGEN_IGNORE dlm_fba_preemphasis(FLOAT64* lpIn, INT32 nXIn, FLOAT64 nPreem) {
  INT32 i = 0;
  FLOAT64 nPrev = 0.0;

  /* init memory */
  nPrev = (lpIn[0] + lpIn[1]) / 2.0;

  /* Apply preemphasis to signal */
  for (i = 0; i < nXIn; i++) {
    lpIn[i] = lpIn[i] - nPreem * nPrev;
    nPrev = lpIn[i];
  }

  return O_K;
}

/**
 * Grab one signal frame, apply window and calculate energy
 *
 * <h3>Notes</h3>
 * <ul>
 *   <li>This function performs no checking of dSignal!</li>
 * </ul>
 *
 * @param dSignal  signal data instance
 * @param nOffset  Grap frame starting from offset
 * @param nLength  ... and length nLength
 * @param nEnergy  Return energy of grabbed frame
 * @return new offset, or -1 if there is no more data
 */
INT32 CGEN_IGNORE dlm_fba_grabframe(FLOAT64 *lpSignal, INT32 nXSamples, INT32 nOffset, INT32 nFrameLength, INT32 nLen, FLOAT64 nMinLog, FLOAT64 nLambda, char bTimeDomainWarping, FLOAT64 nDC, FLOAT64 *lpFrame, FLOAT64 *lpWindow, FLOAT64* lpEnergy) {
  INT32 i = 0;
  INT32 nRecs = 0;
  FLOAT64 nSample = 0.;
  FLOAT64 nMinLogExp = exp(nMinLog);

  if (nFrameLength > nLen) nFrameLength = nLen; /* Truncate signal window to analysis window length */
  if ((nOffset + nFrameLength) > (nRecs = nXSamples)) /* Limit length to number of remaining samples */
  nFrameLength = nRecs - nOffset;

  *lpEnergy = 0.0;

  /* adding white noise below */
  for (i = 0; i < nFrameLength; i++)
    lpFrame[i] = lpSignal[nOffset + i] + (dlp_rand() / (FLOAT64) RAND_MAX - 0.5) * nMinLogExp;

  /* apply time domain warping (before windowing!) */
  if ((nLambda != 0.0) && (bTimeDomainWarping == TRUE)) {
    dlm_freqt(lpFrame, nFrameLength, lpFrame, nFrameLength, nLambda);
  }

  /* Apply window, remove DC and calculate energy of frame */
  for (i = 0; i < nFrameLength; i++) {
    nSample = lpFrame[i];
    if (nDC != 0.0) nSample -= nDC;
    lpFrame[i] = nSample * lpWindow[i];
    *lpEnergy += nSample * nSample;
  }

  /* fill zeros */
  for (; i < nLen; i++)
    lpFrame[i] = (dlp_rand() / (FLOAT64) RAND_MAX - 0.5) * nMinLogExp;

  return nFrameLength;
}

INT16 CGEN_IGNORE dlm_fba_doframing(FLOAT64 *lpSignal, INT32 nXSamples, INT64 *lpPitch, INT32 nXPitch, FLOAT64 *lpWindow, const char **lpLabIn, char **lpLabOut, INT16 *lpFrameLen, FLOAT64 *lpFrames, INT32 nXFrames, FLOAT64 *lpEnergy, struct dlm_fba_doframing_param *lpParam) {
  INT32 i = 0;
  INT32 nOffset = 0;
  INT32 nFrame = 0;
  FLOAT64 nEnergy = 0.0;
  INT32 nPSOffset = 0; /* Offset in samples (accumulated number of already processed periods) */
  INT32 nPPOffset = 0; /* Offset in periods (record number in dPitch) */
  INT32 nFrameLength = 0;
  INT32 nPeriodLength = 0;
  INT32 nPrevFrameLength = 0;
  INT32 nRealFrameLength;
  INT32 nCRate = 0;
  char bLastFrame = FALSE;
  if (!lpPitch) nPrevFrameLength = lpParam->nCrate;

  /* apply preemphasis */
  if (lpParam->nPreem != 0.0) dlm_fba_preemphasis(lpSignal, nXSamples, lpParam->nPreem);

  srand(0);

  for (nFrame = 0; lpPitch ? bLastFrame == FALSE : nFrame < nXFrames; nFrame++) {
    /* Last frame? */
    if ((nOffset + nPrevFrameLength) >= nXSamples) bLastFrame = TRUE;

    /* Lookup length of next period */
    if (lpPitch && nPPOffset < nXPitch) {
      if (nOffset >= nPSOffset) {
        nPeriodLength = lpPitch[nPPOffset];
        /* The frame may contain n following frames (as much as fit completely in the analysis window) */
        for (i = 1, nFrameLength = nPeriodLength; i <= lpParam->nNPeriods; i++) {
          INT32 nNext = lpPitch[nPPOffset + i];
          if ((nFrameLength + nNext) < lpParam->nLen) nFrameLength += nNext;
          else break;
        }
        nPPOffset += 1;
        nPSOffset += nPeriodLength;
        nCRate = nPeriodLength;
        if (nOffset > nPSOffset) nOffset = nOffset - lpParam->nCrate + nPeriodLength;
      }
    } else {
      nFrameLength = lpParam->nWlen;
      nCRate = lpParam->nCrate;
    }

    /* Recalculate window if necessary */
    if (lpPitch && nFrameLength != nPrevFrameLength) {
      lpParam->nWlen = nFrameLength;
      IF_NOK(dlm_fba_makewindow(lpWindow,lpParam->nWlen,lpParam->lpsWtype,lpParam->nWnorm)) return NOT_EXEC;
    }
    nPrevFrameLength = nFrameLength;

    /* Grab signal frame starting from nOffset of length nFrameLength, the energy of the frame is returned in nEnergy */
    nRealFrameLength =
        dlm_fba_grabframe(lpSignal, nXSamples, nOffset, nFrameLength, lpParam->nLen, lpParam->nMinLog, lpParam->nLambda, lpParam->bTimeDomainWarping, lpParam->nDC, lpFrames + lpParam->nLen * nFrame, lpWindow, &nEnergy);
    if (lpFrameLen) lpFrameLen[nFrame] = nRealFrameLength;

    /* append label */
    if (lpLabOut) strcpy(lpLabOut[nFrame], lpLabIn[nOffset + nRealFrameLength / 2]);

    /* Save energy :) */
    if (lpParam->bEnergy || lpParam->bLogEnergy) {
      if (lpParam->bLogEnergy) lpEnergy[nFrame] =
          log(nEnergy) < (FLOAT64) lpParam->nMinLog ? (FLOAT64) lpParam->nMinLog : log(nEnergy);
      else lpEnergy[nFrame] = nEnergy;
    }

    nOffset += nCRate;
  }

  return O_K;
}

/** Apply windowing to frame
 *
 * @param X Input frame to window
 * @param n Length of input frame
 * @param sWindow Window to apply
 * @param bNorm Apply normalized window if <code>TRUE</code>
 *
 * @return <code>O_K</code> if successful, <code>ERR_MEM</code>/<code>NOT_EXEC</code> otherwise;
 */
INT16 CGEN_PUBLIC dlm_fba_window(FLOAT64* X, INT32 n, const char* sWindow, BOOL bNorm) {
  INT32 i = 0;
  FLOAT64* W = NULL;
  if (!X) return NOT_EXEC;

  W = (FLOAT64*) dlp_calloc(n, sizeof(FLOAT64));
  if (!W) return ERR_MEM;

  dlm_fba_makewindow(W, n, sWindow, bNorm);
  for (i = 0; i < n; i++) {
    X[i] *= W[i];
  }
  dlp_free(W);
  return O_K;
}

/**
 * Computes the weighted difference of a time series. The function realizes an
 * FIR filter with the symmetric impulse response <code>W</code>.
 *
 * @param Z   Pointer to result buffer, must be capable of holding at least
 *            <code>nXZ</code> values
 * @param A   Pointer to time series to process
 * @param nXC Length of time series
 * @param bRA If true, A is a ring buffer
 * @param nDim Size of one record in Z and A (>1 => skip some entries)
 * @param W   Pointer to weights array
 * @param L   Difference window length, <code>W</code> is expected to contain
 *            exactly 2&middot;<code>L</code>+1 values
 * @return <code>O_K</code> if sucessfull, a (negative) error code otherwise
 * @cgen:TODO: move to <code>dlp_math</code>?
 */
INT16 CGEN_IGNORE dlm_fba_delta(FLOAT64* Z, FLOAT64* A, INT32 nXC, BOOL bRA, INT32 nDim, FLOAT64* W, INT16 L) {
  INT32 z = 0;
  INT32 w = 0;

  if (!A || !Z) return NOT_EXEC;

  for (z = 0; z < nXC; z++) {
    Z[z * nDim] = 0.;
    for (w = -L; w <= L; w++)
      Z[z * nDim] +=
          W[w + L] * A[(bRA ? (z + w + nXC) % nXC : (z + w < 0 ? 0 : (z + w >= nXC ? nXC - 1 : z + w))) * nDim];
  }

  return O_K;
}

INT16 CGEN_IGNORE dlm_fba_deltafba(FLOAT64 *lpFrames, FLOAT64 *lpDeltaT, FLOAT64 *lpDeltaW, INT32 nXFrames, BOOL bFRing, INT32 nDeltaWL, INT32 nFDim, INT32 *nVDim, INT32 *nADim, INT32 nDim) {
  INT32 nC;
  char bV;
  char bA;
  FLOAT64 *lpF;
  FLOAT64 *lpV;
  FLOAT64 *lpA = NULL;
  FLOAT64 *lpT = NULL;
  INT32 nV = 0;
  INT32 nA = 0;

  if (!lpFrames) *nVDim = *nADim = 0;
  else {
    nV = nFDim;
    nA = nFDim + (*nVDim);
  }

  for (nC = 0; nC < nFDim; nC++) {
    bV = (char) lpDeltaT[nC];
    bA = (char) lpDeltaT[nFDim + nC];

    if (!bV && !bA) continue;

    if (!lpFrames) {
      if (bV) (*nVDim)++;
      if (bA) (*nADim)++;
      continue;
    }

    lpF = lpFrames + nC;
    if (bA) lpA = lpFrames + nA;
    if (bV) lpV = lpFrames + nV;
    else {
      if (!lpT) lpT = (FLOAT64*) dlp_malloc(sizeof(FLOAT64)*nXFrames);
      lpV = lpT;
    }
    dlm_fba_delta(lpV, lpF, nXFrames, bFRing, nDim, lpDeltaW, nDeltaWL);
    if (bA) dlm_fba_delta(lpA, lpV, nXFrames, bFRing, nDim, lpDeltaW, nDeltaWL);

    if (bV) nV++;
    if (bA) nA++;
  }

  if (lpT) dlp_free(lpT);
  return O_K;
}
