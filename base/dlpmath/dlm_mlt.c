/* dLabPro mathematics library
 * - MLT
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
#include <stdlib.h>
#include <math.h>
#include "dlp_kernel.h"
#include "dlp_base.h"
#include "dlp_math.h"

INT16 CGEN_IGNORE dlm_mlt(FLOAT64* lpMlt, FLOAT64* lpSig, INT32 nC, INT32 nR) {
  INT32 jC;
  INT32 iC;
  INT32 iR;
  FLOAT64* lpS = lpSig;
  FLOAT64* lpM = lpMlt;
  FLOAT64* lpA = (FLOAT64*)dlp_calloc(nC/2, sizeof(FLOAT64));
  FLOAT64 norm = dlp_scalop(4.0 / (FLOAT64) nC, 0, OP_SQRT);
  FLOAT64 si_arg0 = F_PI / (FLOAT64) nC;
  FLOAT64 co_arg0 = F_PI / (FLOAT64) (nC/2);
  FLOAT64 co_arg1;

  for (iR = 0; iR < nR; iR++) {
    for (iC = 0; iC < nC/4; iC++) {
      *(lpA        + iC) = sin(si_arg0 * (nC/4 - iC - 0.5)) * *(lpS + nC/4 - iC - 1) + sin(si_arg0 * (nC/4 + iC + 0.5)) * *(lpS + nC/4 + iC    );
      *(lpA + nC/4 + iC) = sin(si_arg0 * (nC/2 - iC - 0.5)) * *(lpS + nC/2 + iC    ) - sin(si_arg0 * (       iC + 0.5)) * *(lpS + nC   - iC - 1);
    }
    for (jC = 0; jC < nC/2; jC++) {
      co_arg1 = co_arg0 * (jC + 0.5);
      *(lpM + jC) = 0.0;
      for (iC = 0; iC < nC/2; iC++) {
        *(lpM + jC) += cos(co_arg1 * (iC + 0.5)) * *(lpA + iC);
      }
      *(lpM + jC) *= norm;
    }
    lpM += nC/2;
    lpS += nC;
  }

  dlp_free(lpA);
  return O_K;
}

INT16 CGEN_IGNORE dlm_imlt(FLOAT64* lpSig, FLOAT64* lpMlt, INT32 nC, INT32 nR) {
  INT32 jC;
  INT32 iC;
  INT32 iR;
  FLOAT64* lpS = lpSig;
  FLOAT64* lpM = lpMlt;
  FLOAT64* lpA1 = (FLOAT64*)dlp_calloc(3*nC/2, sizeof(FLOAT64));
  FLOAT64* lpA2 = lpA1 + nC;
  FLOAT64 norm = dlp_scalop(2.0 / (FLOAT64) nC, 0, OP_SQRT);
  FLOAT64 si_arg0 = F_PI / (FLOAT64) (2*nC);
  FLOAT64 co_arg0 = F_PI / (FLOAT64) nC;
  FLOAT64 co_arg1;

  for (iR = 0; iR < nR; iR++) {
    dlp_memmove(lpA2, lpA1 + nC/2, nC/2*sizeof(FLOAT64));
    for (iC = 0; iC < nC; iC++) {
      co_arg1 = co_arg0 * (iC + 0.5);
      *(lpA1 + iC) = 0.0;
      for (jC = 0; jC < nC; jC++) {
        *(lpA1 + iC) += cos(co_arg1 * (jC + 0.5)) * *(lpM + jC);
      }
      *(lpA1 + iC) *= norm;
    }
    for (iC = 0; iC < nC/2; iC++) {
      *(lpS        + iC) = sin(si_arg0 * (     iC + 0.5)) * *(lpA1 + nC/2 - iC - 1) + sin(si_arg0 * (nC   - iC - 0.5)) * *(lpA2        + iC    );
      *(lpS + nC/2 + iC) = sin(si_arg0 * (nC + iC + 0.5)) * *(lpA1 +      + iC    ) - sin(si_arg0 * (nC/2 - iC - 0.5)) * *(lpA2 + nC/2 - iC - 1);
    }
    lpM += nC;
    lpS += nC;
  }

  dlp_free(lpA1);
  return O_K;
}

INT16 CGEN_IGNORE dlm_mlt_band(FLOAT64* lpMlt, FLOAT64* lpSig, INT32 nLen, INT32 nOut) {
  INT32 i;
  INT32 j;
  INT32 k;
  FLOAT64 sum;
  FLOAT64 si_arg0 = F_PI / (FLOAT64) nLen;
  FLOAT64 co_arg0 = F_PI / (FLOAT64) nLen / 2;
  FLOAT64 co_arg1;

  for (k = 0; k < nOut; k++) {
    *(lpMlt + k) = 0.0;
    for (j = k * nLen / (2 * nOut); j < (k + 1) * nLen / (2 * nOut); j++) {
      sum = 0.0;
      co_arg1 = co_arg0 * (j + 0.5);
      for (i = 0; i < nLen; i++) {
        sum += sin(si_arg0 * (i + 0.5)) * cos(co_arg1 * (i - (nOut + 0.5))) * *(lpSig + i);
      }
      *(lpMlt + k) += 4.0 * sum * sum / (FLOAT64) nLen;
    }
    *(lpMlt + k) = dlp_scalop(*(lpMlt + k) / (FLOAT64) (nLen / (2 * nOut)), 0.0, OP_LOG);
  }
  return O_K;
}
