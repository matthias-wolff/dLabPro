/* dLabPro mathematics library
 * - Dynamic Time Warping implementation
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
 * <p>Dynamic time warping.</p>
 * This function calculates the alignment path of the two input vector sequences <code>lpSrc</code> and <code>lpDst</code>
 * by means of the minimum accumulated vector difference norm. The output <code>lpResult</code> consists of the index
 * sequence related to the vectors of <code>lpSrc</code> and the corresponding index sequence related to vectors of
 * <code>lpDst</code>. The output <code>lpError</code> contains the error at the corresponding point of the alignment path.
 *
 * @param lpSrc Pointer to reference (source) input vector sequence.
 * @param nSrcRows Number of vectors given in <code>lpSrc</code>.
 * @param lpDst Pointer to (destination) input vector sequence.
 * @param nDstRows Number of vectors given in <code>lpDst</code>.
 * @param nCols Number of vector elements of <code>lpSrc</code> and <code>lpDst</code>.
 * @param lpResult Pointer to alignment path to be calculated (must be at least <code>2*(nSrcRows+nDstRows)</code>.
 * @param nResult Pointer to the value filled with the actual size of the alignment path, so that <code>lpResult</code> and <code>lpError</code> can be reallocated by <code>nResult</code>.
 * @param lpError Pointer to the error sequence (must be at least <code>nSrcRows+nDstRows</code>.
 */
INT16 dlm_dtwC(COMPLEX64* lpSrc, INT32 nSrcRows, COMPLEX64* lpDst, INT32 nDstRows, INT32 nCols, INT32* lpResult, INT32* nResult, FLOAT64* lpError) {
  INT32 nCol;
  INT32 nSrcRow;
  INT32 nDstRow;
  FLOAT64* lpMatrix = NULL;

  DLPASSERT((lpSrc!=NULL) && (lpDst!=NULL) && (lpResult!=NULL) && (nResult!=NULL) && (lpError!=NULL));

  lpMatrix = (FLOAT64*)dlp_calloc(nSrcRows*nDstRows, sizeof(FLOAT64));

  if(lpDst != NULL) {
    for(nSrcRow = 0; nSrcRow < nSrcRows; nSrcRow++) {
      for(nDstRow = 0; nDstRow < nDstRows; nDstRow++) {
        for(nCol = 0; nCol < nCols; nCol++) {
          lpMatrix[nSrcRow*nDstRows+nDstRow] += dlp_scalopC(lpSrc[nSrcRow*nCols+nCol],lpDst[nDstRow*nCols+nCol], OP_QABSDIFF).x;
        }
        lpMatrix[nSrcRow*nDstRows+nDstRow] = sqrt(lpMatrix[nSrcRow*nDstRows+nDstRow]);
      }
    }
  }

  for(nSrcRow = 0; nSrcRow < nSrcRows; nSrcRow++) {
    for(nDstRow = 0; nDstRow < nDstRows; nDstRow++) {
      if(nSrcRow > 0) {
        if(nDstRow > 0) {
          if(lpMatrix[(nSrcRow)*nDstRows+nDstRow-1] <= lpMatrix[(nSrcRow-1)*nDstRows+nDstRow]) {
            if(lpMatrix[(nSrcRow)*nDstRows+nDstRow-1] < lpMatrix[(nSrcRow-1)*nDstRows+nDstRow-1]) {
              lpMatrix[nSrcRow*nDstRows+nDstRow] += lpMatrix[(nSrcRow)*nDstRows+nDstRow-1];
            } else {
              lpMatrix[nSrcRow*nDstRows+nDstRow] = lpMatrix[nSrcRow*nDstRows+nDstRow] + lpMatrix[(nSrcRow-1)*nDstRows+nDstRow-1];
            }
          } else if(lpMatrix[(nSrcRow-1)*nDstRows+nDstRow] < lpMatrix[(nSrcRow-1)*nDstRows+nDstRow-1]) {
            lpMatrix[nSrcRow*nDstRows+nDstRow] += lpMatrix[(nSrcRow-1)*nDstRows+nDstRow];
          } else {
            lpMatrix[nSrcRow*nDstRows+nDstRow] += lpMatrix[(nSrcRow-1)*nDstRows+nDstRow-1];
          }
        } else {
          lpMatrix[nSrcRow*nDstRows+nDstRow] += lpMatrix[(nSrcRow-1)*nDstRows+nDstRow];
        }
      } else if(nDstRow > 0) {
        lpMatrix[nSrcRow*nDstRows+nDstRow] += lpMatrix[(nSrcRow)*nDstRows+nDstRow-1];
      }
    }
  }

  nSrcRow = nSrcRows-1;
  nDstRow = nDstRows-1;
  *nResult = nSrcRows+nDstRows-1;
  lpResult[*nResult*2] = nSrcRow;
  lpResult[*nResult*2+1] = nDstRow;
  lpError[*nResult] = lpMatrix[nSrcRow*nDstRows+nDstRow];
  while(nSrcRow || nDstRow) {
    if(nSrcRow) {
      if(nDstRow) {
        if(lpMatrix[(nSrcRow-1)*nDstRows+nDstRow-1] <= lpMatrix[(nSrcRow-1)*nDstRows+nDstRow]) {
          if(lpMatrix[(nSrcRow-1)*nDstRows+nDstRow-1] <= lpMatrix[(nSrcRow)*nDstRows+nDstRow-1]) {
            nSrcRow--;
            nDstRow--;
          } else {
            nDstRow--;
          }
        } else if(lpMatrix[(nSrcRow-1)*nDstRows+nDstRow] < lpMatrix[(nSrcRow)*nDstRows+nDstRow-1]) {
          nSrcRow--;
        } else {
          nDstRow--;
        }
      } else {
        nSrcRow--;
      }
    } else if(nDstRow) {
      nDstRow--;
    }
    (*nResult)--;
    lpResult[*nResult*2] = nSrcRow;
    lpResult[*nResult*2+1] = nDstRow;
    lpError[*nResult] = lpMatrix[nSrcRow*nDstRows+nDstRow];
  }

  dlp_memmove(lpResult, lpResult+2**nResult, 2*(nSrcRows+nDstRows-*nResult)*sizeof(INT32));
  dlp_memmove(lpError,  lpError +  *nResult,   (nSrcRows+nDstRows-*nResult)*sizeof(FLOAT64));
  *nResult = nSrcRows+nDstRows - *nResult;

  dlp_free(lpMatrix);

  return O_K;
}
