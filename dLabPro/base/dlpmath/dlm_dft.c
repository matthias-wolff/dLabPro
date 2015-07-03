/* dLabPro mathematics library
 * - DFT and DCT
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

INT16 dlm_dct1(FLOAT64 x[], INT32 n, FLOAT64 y[]) {
  register INT32 i;
  register INT32 j;
  FLOAT64 arg = F_PI / (FLOAT64)(n-1);
  
  if((x == NULL) || (y == NULL) || (x==y)) return NOT_EXEC;
  
  for(i = 0; i < n; i++) {
    y[i] = x[0] / 2.0;
    for(j = 1; j < n-1; j++) {
      y[i] += x[j] * cos(arg * (FLOAT64)(i*j));
    }
    y[i] += ((i/2*2 == i) ? x[j] : -x[j]) / 2.0;
  }
  return O_K;
}

INT16 dlm_idct1(FLOAT64 x[], INT32 n, FLOAT64 y[]) {
  INT32 i;
  FLOAT64 n2 = 2.0 / (FLOAT64)(n-1);

  if(dlm_dct1(x,n,y) != O_K) return NOT_EXEC;
  
  for (i = 0; i < n; i++) y[i] = y[i] * n2;
  
  return O_K;
}

INT16 dlm_dct2(FLOAT64 x[], INT32 n, FLOAT64 y[]) {
  register INT32 i;
  register INT32 j;
  FLOAT64 arg = F_PI / (FLOAT64)n;
  
  if((x == NULL) || (y == NULL) || (x==y)) return NOT_EXEC;
  
  for(i = 0; i < n; i++) {
    y[i] = 0.0;
    for(j = 0; j < n; j++) {
      y[i] += x[j] * cos(arg * ((FLOAT64)j + 0.5) * (FLOAT64)i);
    }
  }
  return O_K;
}

INT16 dlm_idct2(FLOAT64 x[], INT32 n, FLOAT64 y[]) {
  INT32 i;
  FLOAT64 n2 = 2.0 / (FLOAT64)n;

  if(dlm_dct3(x,n,y) != O_K) return NOT_EXEC;  /* this is true: IDCT2(DCT3) */

  for (i = 0; i < n; i++) y[i] = y[i] * n2;

  return O_K;
}

INT16 dlm_dct3(FLOAT64 x[], INT32 n, FLOAT64 y[]) {
  register INT32 i;
  register INT32 j;
  FLOAT64 arg = F_PI / (FLOAT64)n;
  
  if((x == NULL) || (y == NULL) || (x==y)) return NOT_EXEC;
  
  for(i = 0; i < n; i++) {
    y[i] = x[0]/2.0;
    for(j = 1; j < n; j++) {
      y[i] += x[j] * cos(arg * ((FLOAT64)i + 0.5) * (FLOAT64)j);
    }
  }
  return O_K;
}

INT16 dlm_idct3(FLOAT64 x[], INT32 n, FLOAT64 y[]) {
  INT32 i;
  FLOAT64 n2 = 2.0 / (FLOAT64)n;

  if(dlm_dct2(x,n,y) != O_K) return NOT_EXEC; /* this is true: IDCT3(DCT2) */

  for (i = 0; i < n; i++) y[i] = y[i] * n2;

  return O_K;
}

INT16 dlm_dct4(FLOAT64 x[], INT32 n, FLOAT64 y[]) {
  register INT32 i;
  register INT32 j;
  FLOAT64 arg = F_PI / (FLOAT64)n;
  
  if((x == NULL) || (y == NULL) || (x==y)) return NOT_EXEC;
  
  for(i = 0; i < n; i++) {
    y[i] = x[0];
    for(j = 1; j < n; j++) {
      y[i] += x[j] * cos(arg * ((FLOAT64)j + 0.5) * ((FLOAT64)i+0.5));
    }
  }
  return O_K;
}

INT16 dlm_idct4(FLOAT64 x[], INT32 n, FLOAT64 y[]) {
  INT32 i;
  FLOAT64 n2 = 2.0 / (FLOAT64)n;

  if(dlm_dct2(x,n,y) != O_K) return NOT_EXEC;

  for (i = 0; i < n; i++) y[i] = y[i] * n2;

  return O_K;
}

/**
 * <p id=dlm_dftC>Computes the complex (inverse) Fourier transform (double precision).</p>
 *
 * @param x Pointer to input array.
 * @param n Length of signal, i.e. length of x and y.
 * @param bInv If non-zero the function computes the inverse complex Fourier transform
 * @param y Pointer to output array.
 * @return ERR_MEM on allocation fault, O_K otherwise.
 *
 * <h4>Remarks</h4>
 * <ul><li><code>x</code> and <code>y</code> may be identical.
 * </li></ul>
 */
INT16 dlm_dftC(COMPLEX64 x[], INT32 n, BOOL bInv, COMPLEX64 y[]) {
  INT32      k;
  INT32      l;
  FLOAT64    arg;
  COMPLEX64* z = NULL;
  BOOL       need_free = FALSE;
  
  if(y==x) {
    z = (COMPLEX64*)dlp_calloc(n,sizeof(COMPLEX64));
    if(!z) return ERR_MEM;
    need_free = TRUE;
  } else {
    z = y;
  }
  for(k = 0; k < n; k++) {
    z[k] = CMPLX(0);
    arg = 2.0 * F_PI * (FLOAT64)k / (FLOAT64)n;
    if(bInv) {
      for(l = 0; l < n; l++) {
        z[k] = CMPLX_PLUS(z[k],CMPLXY(x[l].x*cos((FLOAT64)l*arg)-x[l].y*sin((FLOAT64)l*arg),x[l].x*sin((FLOAT64)l*arg)+x[l].y*cos((FLOAT64)l*arg)));
      }
      z[k] = CMPLX_DIV_R(z[k],(FLOAT64)n);
    } else {
      for(l = 0; l < n; l++) {
        z[k] = CMPLX_PLUS(z[k],CMPLXY(x[l].x*cos((FLOAT64)l*arg)+x[l].y*sin((FLOAT64)l*arg),-x[l].x*sin((FLOAT64)l*arg)+x[l].y*cos((FLOAT64)l*arg)));
      }
    }
  }
  if(need_free) {
    dlp_memmove(y,z,n*sizeof(COMPLEX64));
    dlp_free(z);
  }
  return O_K;
}
