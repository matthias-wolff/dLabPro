/* dLabPro mathematics library
 * - Vector Quantization Functions
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

#define SWAP(g,h,T) { T y=(g);(g)=(h);(h)=y; }
#define M 7
#define NSTACK 50

INT16 __dlm_centroidsCompare(FLOAT64 a, FLOAT64 b) {
  if ((a < b) || (!dlp_isnan(a) && dlp_isnan(b))) return 1;
  if ((a == b) || (dlp_isnan(a) && dlp_isnan(b))) return 0;
  else return -1;
}

INT16 __dlm_sortCentroids(FLOAT64* matrix, INT32 nC, INT32 nR) {
  INT32 i, ir = nR - 1, j, k, l = 0;
  INT32 jstack = -1;
  FLOAT64 a;
  INT32 istack[NSTACK];

  for (;;) {
    if (ir - l < M) {
      for (j = l + 1; j <= ir; j++) {
        a = matrix[j * nC];
        for (i = j - 1; i >= l; i--) {
          if (__dlm_centroidsCompare(matrix[i * nC], a) >= 0) break;
          matrix[(i + 1) * nC] = matrix[i * nC];
        }
        matrix[(i + 1) * nC] = a;
      }
      if (jstack < 0) break;
      ir = istack[jstack--];
      l = istack[jstack--];
    } else {
      k = (l + ir) >> 1;
      SWAP(matrix[k * nC], matrix[(l + 1) * nC], FLOAT64);
      if (__dlm_centroidsCompare(matrix[l * nC], matrix[ir * nC]) < 0) {
        SWAP(matrix[l * nC], matrix[ir * nC], FLOAT64);
      }
      if (__dlm_centroidsCompare(matrix[(l + 1) * nC], matrix[ir * nC]) < 0) {
        SWAP(matrix[(l + 1) * nC], matrix[ir * nC], FLOAT64);
      }
      if (__dlm_centroidsCompare(matrix[l * nC], matrix[(l + 1) * nC]) < 0) {
        SWAP(matrix[l * nC], matrix[(l + 1) * nC], FLOAT64);
      }
      i = l + 1;
      j = ir;
      a = matrix[(l + 1) * nC];
      for (;;) {
        do
          i++;
        while (__dlm_centroidsCompare(matrix[i * nC], a) > 0);
        do
          j--;
        while (__dlm_centroidsCompare(matrix[j * nC], a) < 0);
        if (j < i) break;
        SWAP(matrix[i * nC], matrix[j * nC], FLOAT64);
      }
      matrix[(l + 1) * nC] = matrix[j * nC];
      matrix[j * nC] = a;
      jstack += 2;
      if (jstack >= NSTACK) return NOT_EXEC; /*nrerror("NSTACK too small in sort.");*/
      if (ir - i + 1 >= j - l) {
        istack[jstack] = ir;
        istack[jstack - 1] = i;
        ir = j - 1;
      } else {
        istack[jstack] = j - 1;
        istack[jstack - 1] = l;
        l = i;
      }
    }
  }
  return O_K;
}

INT16 CGEN_IGNORE __dlm_getNCentroids(INT32* pNC, INT32* B, INT32 n) {
  INT32 i;
  INT16 iBits;
  INT16 nBits;

  for (i = 0; i < n; i++) {
    nBits = B[i];
    pNC[i] = 1;
    for (iBits = 0; iBits < nBits; iBits++) {
      pNC[i] *= 2;
    }
  }
  return O_K;
}

void CGEN_IGNORE __dlm_getNearestIndex(FLOAT64* X, FLOAT64* Q, INT32 nC, INT32 nCX, INT32 nRQ, INT32* pIndices, INT32 iRX, BOOL* bChanged) {
  INT32 iRQ = 0;
  INT32 nOI = 0;
  FLOAT64 nDiff1 = 0.0;
  FLOAT64 nDiff2 = 0.0;

  nDiff1 = fabs(Q[0] - X[iRX * nCX]);
  nOI = pIndices[iRX * nCX];
  pIndices[iRX * nCX] = 0;
  for (iRQ = 1; iRQ < nC; iRQ++) {
    nDiff2 = fabs(Q[iRQ * nCX] - X[iRX * nCX]);
    if (__dlm_centroidsCompare(nDiff1, nDiff2) < 0) {
      nDiff1 = nDiff2;
      pIndices[iRX * nCX] = iRQ;
    }
  }
  if (nOI != pIndices[iRX * nCX]) {
    *bChanged = TRUE;
  }
}

BOOL CGEN_IGNORE __dlm_assignIndices(FLOAT64* X, FLOAT64* Q, INT32 nC, INT32 nRX, INT32 nCX, INT32 nRQ, INT32* pIndices) {
  INT32 iRX = 0;
  BOOL bChanged = FALSE;

  for (iRX = 0; iRX < nRX; iRX++) {
    __dlm_getNearestIndex(X, Q, nC, nCX, nRQ, pIndices, iRX, &bChanged);
  }
  return bChanged;
}

INT16 CGEN_IGNORE __dlm_getNewCentroids(FLOAT64* X, INT32* pI, INT32 nRX, INT32 nCX, FLOAT64* Q, INT32 nRQ) {
  INT32 iRX = 0;
  INT32 iRQ = 0;
  INT32* C = (INT32*) dlp_calloc(nRQ, sizeof(INT32));

  for (iRQ = 0; iRQ < nRQ; iRQ++) {
    Q[iRQ * nCX] = 0;
  }
  for (iRX = 0; iRX < nRX; iRX++) {
    Q[pI[iRX * nCX] * nCX] += X[iRX * nCX];
    C[pI[iRX * nCX]]++;
  }
  for (iRQ = 0; iRQ < nRQ; iRQ++) {
    Q[iRQ * nCX] /= (FLOAT64) C[iRQ];
  }
  dlp_free(C);
  return O_K;
}

INT16 CGEN_IGNORE __dlm_splitCentroids(FLOAT64* Q, INT32 nCX, INT32* nC) {
  INT32 iC = 0;
  FLOAT64 nEps = dlm_pow2(-DBL_MANT_DIG + 1);

  for (iC = 0; iC < *nC; iC++) {
    Q[(iC + *nC) * nCX] = (fabs(Q[iC * nCX]) < nEps) ? 1.0 : Q[iC * nCX] * (1.0 + 2 * nEps);
    Q[iC * nCX] = (fabs(Q[iC * nCX]) < nEps) ? -1.0 : Q[iC * nCX] * (1.0 - 2 * nEps);
  }
  *nC *= 2;
  return O_K;
}

INT16 CGEN_IGNORE __dlm_shrinkCentroids(FLOAT64* Q, INT32* B, INT32 nCX, INT32* nC) {
  INT32 iC = 0;
  INT32 nBit = 0;

  for (iC = *nC - 1; iC >= 0; iC--) {
    if (!dlp_isnan(Q[iC * nCX])) {
      for (nBit = 0; (1 << nBit) <= iC; nBit++);
      B[0] = nBit;
      break;
    }
  }
  *nC = iC + 1;
  return O_K;
}

/**
 *  Scalar Vector Quantization.
 *
 *  <p>This function calculates a code-book from the input vectors for each component independently. The number
 *  of columns of <code>X</code>, <code>B</code> and <code>Q</code> are equal. The bit-table determines the number of
 *  code-book entries for each component. The number of rows of <code>B</code> is <code>1</code> and of <code>Q</code>
 *  <code>max(2<sup>B</sup>)</code>. For each component <code>n</code> of <code>B</code> where
 *  <code>B[n]&lt;max(B)</code> there will be <code>NaN</code>'s for fill-up.</p>
 *
 * @param X   input containing the vectors to quantize
 * @param nRX number of input vectors
 * @param nCX dimension/components of input vectors
 * @param B   bit-table
 * @param I   output byte stream containing indices of code book
 * @param nCI Number of components of <code>I</code>
 * @param Q   output code-book
 * @param nRQ maximum number of code book entries of each single component (could be changed by shrinked codebook)
 *
 * @return <code>O_K</code> if ok, <code>NOT_EXEC</code> otherwise.
 */
INT16 CGEN_PUBLIC dlm_svq(FLOAT64* X, INT32 nRX, INT32 nCX, INT32* B, INT32 nRB, BYTE* I, INT32* nCI, FLOAT64* Q, INT32* nRQ) {
  INT16 ret = O_K;
  INT32 iCX = 0;
  INT32 iCI = 0;
  INT32 iRX = 0;
  INT32 iBB = 0;
  INT32 iBI = 0;
  INT32 iRQ = 0;
  INT32 nRQNew = 0;
  INT32 nCINew = 0;
  BOOL bChanged = TRUE;
  INT32* pI = (INT32*) dlp_calloc(nCX*nRX, sizeof(INT32));
  INT32* pNC = (INT32*) dlp_calloc(nCX, sizeof(INT32));

  if ((X == NULL) || (B == NULL) || (I == NULL) || (Q == NULL)) return NOT_EXEC;

  __dlm_getNCentroids(pNC, B, nCX);

  for (iCX = 0; iCX < nCX; iCX++) {
    iRQ = 1;
    Q[iCX] = 0;

    for (iRX = 0; iRX < nRX; iRX++) {
      Q[iCX] += X[iRX * nCX + iCX];
    }
    Q[iCX] /= (FLOAT64) nRX;

    while (iRQ < dlm_pow2(B[iCX])) {
      __dlm_splitCentroids(Q + iCX, nCX, &iRQ);
      bChanged = TRUE;
      do {
        bChanged = __dlm_assignIndices(X + iCX, Q + iCX, iRQ, nRX, nCX, *nRQ, pI + iCX);
        __dlm_getNewCentroids(X + iCX, pI + iCX, nRX, nCX, Q + iCX, iRQ);
      } while (bChanged);
    }
    while (iRQ < *nRQ) {
      Q[iRQ * nCX + iCX] = 0.0 / 0.0;
      iRQ++;
    }

    __dlm_sortCentroids(Q + iCX, nCX, iRQ);
    __dlm_shrinkCentroids(Q + iCX, B + iCX, nCX, &iRQ);
    __dlm_assignIndices(X + iCX, Q + iCX, iRQ, nRX, nCX, *nRQ, pI + iCX);
    nRQNew = MAX(nRQNew, iRQ);
  }

  *nRQ = nRQNew;

  for (iRX = 0; iRX < nRX; iRX++) {
    iCX = 0;
    iCI = 0;
    iBI = 0;
    iBB = B[0] - 1;
    I[iRX * *nCI] = 0;
    while (iCI < *nCI) {
      I[iRX * *nCI + iCI] |= ((pI[iRX * nCX + iCX] >> iBB) & 0x1) << iBI;
      iBB--;
      iBI++;
      if (iBB < 0) {
        iCX++;
        if (iCX >= nCX) break;
        iBB = B[iCX] - 1;
      }
      if (iBI >= 8) {
        iCI++;
        iBI = 0;
      }
    }
    nCINew = MAX(nCINew, iCI+1);
  }

  *nCI = nCINew;

  dlp_free(pI);
  dlp_free(pNC);

  return ret;
}

/**
 * Inverse Scalar Vector Quantization
 *
 * <p>This is the inverse of {@link dlm_svq}. The according to the coded input indices stream <code>I</code> and the
 * code book <code>Q</code> the output vector sequence <code>Y</code> is restored.
 *
 * @param Q Code book
 * @param nCQ Number of compnents of <code>Q</code>
 * @param nRQ Number of records of <code>Q</code>
 * @param I Input byte stream containing the indices to the code book
 * @param nRI Number of records of <code>I</code>
 * @param nCI Number of components of <code>I</code>
 * @param B Bit table containing the number of bits of the indices of each component
 * @param nRB Number of records of <code>B</code>
 * @param Y restored <code>nCQ&times;nRI</code> vector sequence
 */
INT16 CGEN_PUBLIC dlm_isvq(FLOAT64* Q, INT32 nCQ, INT32 nRQ, BYTE* I, INT32 nRI, INT32 nCI, INT32* B, INT32 nRB, FLOAT64* Y) {
  INT16 ret = O_K;
  INT32 iRI = 0;
  INT32 iCQ = 0;
  INT32 iCI = 0;
  INT32 iBB = 0;
  INT32 iBI = 0;
  INT32* pI = (INT32*) dlp_calloc(nCQ*nRI, sizeof(INT32));

  if ((Q == NULL) || (B == NULL) || (Y == NULL)) return NOT_EXEC;

  for (iRI = 0; iRI < nRI; iRI++) {
    iCQ = 0;
    iCI = 0;
    iBI = 0;
    iBB = B[0] - 1;
    pI[iRI * nCQ] = 0;
    while (iCI < nCI) {
      pI[iRI * nCQ + iCQ] |= ((I[iRI * nCI + iCI] >> iBI) & 0x1) << iBB;
      iBB--;
      iBI++;
      if (iBB < 0) {
        iCQ++;
        if (iCQ >= nCQ) break;
        iBB = B[iCQ] - 1;
      }
      if (iBI >= 8) {
        iCI++;
        iBI = 0;
      }
    }
  }

  for (iRI = 0; iRI < nRI; iRI++) {
    for (iCQ = 0; iCQ < nCQ; iCQ++) {
      if (pI[iRI * nCQ + iCQ] < nRQ) {
        Y[iRI * nCQ + iCQ] = Q[pI[iRI * nCQ + iCQ] * nCQ + iCQ];
      } else {
        ret = NOT_EXEC;
      }
    }
  }

  dlp_free(pI);

  return ret;
}

FLOAT64 dlm_pam_norm2(FLOAT64* A, FLOAT64* B, INT32 n) {
  INT32 i;
  FLOAT64 d = A[0]-B[0];
  FLOAT64 norm2 = d*d;
  for(i = 1; i < n; i++) {
    d = A[i]-B[i];
    norm2 += d*d;
  }
  return norm2;
}

FLOAT64 dlm_pam_corr(FLOAT64* A, FLOAT64* B, INT32 n) {
  INT32 i;
  FLOAT64 ener = A[0]*A[0]+B[0]*B[0];
  FLOAT64 corr = -A[0]*B[0];
  for(i = 1; i < n; i++) {
    ener += A[i]*A[i]+B[i]*B[i];
    corr -= A[i]*B[i];
  }
  return corr /ener;
}

FLOAT64 dlm_pam_assign(FLOAT64* X, INT32 nC, INT32* classify, INT32 nRX, INT32* clusters, INT32 nRQ) {
  FLOAT64 error = 0.0;
  INT32 iRX = 0;
  INT32 iRQ = 0;
  FLOAT64 min = 0.0;
  FLOAT64 distance = 0.0;
  FLOAT64* pX;

  for(iRX = 0; iRX < nRX; iRX++) {
    min = T_DOUBLE_MAX;
    pX = X+iRX*nC;
    for(iRQ = 0; iRQ < nRQ; iRQ++) {
      distance = dlm_pam_norm2(pX,X+clusters[iRQ]*nC,nC);
      if(distance < min) {
        min = distance;
        classify[iRX] = iRQ;
      }
    }
    error += min;
  }
  return error;
}

INT16 CGEN_IGNORE dlm_pam(FLOAT64* X, INT32 nC, INT32 nRX, FLOAT64* Q, INT32 nRQ) {
  INT32* clusters = (INT32*) dlp_calloc(nRQ,sizeof(INT32));
  INT32* classify = (INT32*) dlp_calloc(nRX,sizeof(INT32));
  INT32 iRX1 = 0;
  INT32 iRX2 = 0;
  INT32 iRQ = 0;
  FLOAT64 min = T_DOUBLE_MAX;
  FLOAT64 distance = 0.0;
  FLOAT64 error = T_DOUBLE_MAX;
  FLOAT64 error_old = T_DOUBLE_MAX;
  FLOAT64* pX = X;


  for(iRQ = 0; iRQ < nRQ; iRQ++) {
    clusters[iRQ] = iRQ * nRX / nRQ;
  }
  while(1) {
    error_old = error;
    error = dlm_pam_assign(X,nC,classify,nRX,clusters,nRQ);
    if(error_old <= error) break;
    for(iRQ = 0; iRQ < nRQ; iRQ++) {
      min = T_DOUBLE_MAX;
      for(iRX1 = 0; iRX1 < nRX; iRX1++) {
        if(classify[iRX1] != iRQ) continue;
        pX = X+iRX1*nC;
        distance = 0.0;
        for(iRX2 = 0; iRX2 < nRX; iRX2++) {
          if(classify[iRX2] != iRQ) continue;
          distance += dlm_pam_norm2(pX,X+iRX2*nC,nC);
        }
        if(min > distance) {
          min = distance;
          clusters[iRQ] = iRX1;
        }
      }
    }
  }
  for(iRQ = 0; iRQ < nRQ; iRQ++) {
    dlp_memmove(Q+iRQ*nC,X+clusters[iRQ]*nC,nC*sizeof(FLOAT64));
  }
  dlp_free(clusters);
  dlp_free(classify);

  return O_K;
}

INT16 CGEN_IGNORE dlm_lbg(FLOAT64* X, INT32 nC, INT32 nRX, FLOAT64* Q, INT32 nRQ) {
  FLOAT64* icb = (FLOAT64*) dlp_calloc(nC,sizeof(FLOAT64));
  INT32 iC = 0;
  INT32 iRX = 0;
  extern void lbg(FLOAT64*, const INT32, const INT32, FLOAT64*, INT32, FLOAT64*, const INT32, const INT32, const INT32, const INT32, const INT32, const FLOAT64, const FLOAT64);

  for (iRX = 0; iRX < nRX; iRX++) {
    for (iC = 0; iC < nC; iC++) {
      icb[iC] += X[iRX * nC + iC];
    }
  }
  for (iC = 0; iC < nC; iC++) {
    icb[iC] /= (FLOAT64) nRX;
  }

  lbg(X, nC, nRX, icb, 1, Q, nRQ, 1000, 1, 1, 1, 0.0001, 0.0001);

  dlp_free(icb);
  return O_K;
}

INT16 CGEN_IGNORE dlm_vq(FLOAT64* X, INT32 nRX, FLOAT64* Q, INT32 nCQ, INT32 nRQ, INT32* I) {
  INT32 iRX = 0;
  extern int vq(FLOAT64*, FLOAT64*, const INT32, const INT32);

  for (iRX = 0; iRX < nRX; iRX++) {
    I[iRX] = vq(X + iRX * nCQ, Q, nCQ, nRQ);
  }
  return O_K;
}

INT16 CGEN_IGNORE dlm_ivq(FLOAT64* Y, INT32 nRY, FLOAT64* Q, INT32 nCQ, INT32 nRQ, INT32* I) {
  INT32 iR = 0;
  extern void ivq(const INT32, FLOAT64*, const INT32, FLOAT64*);

  for (iR = 0; iR < nRY; iR++) {
    ivq(I[iR], Q, nCQ, Y + iR * nCQ);
  }
  return O_K;
}

#undef SWAP
#undef M
#undef NSTACK

