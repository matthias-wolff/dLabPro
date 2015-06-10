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

#include "dlp_kernel.h"
#include "dlp_base.h"
#include "dlp_math.h"

/* Normierungsfaktoren */
#define SIG_NRM	1.
#define RES_NRM	2.

/* Generalized Mel-Cepstral analysis init buffers
 *
 * @param n       Input feature vector dimension
 * @param order   Output feature vector dimension
 * @param lambda  Lambda parameter for analysis
 */
void dlm_mgcepfix_init(INT32 n, INT16 order, INT16 lambda){
  dlm_mgcep_init(n,order,(FLOAT64)lambda/32767.);
}

/* Generalized Mel-Cepstral analysis free buffers */
void dlm_mgcepfix_free(){
  dlm_mgcep_free();
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
INT16 dlm_mgcepfix(INT16* input, INT32 n, INT16* output, INT16 order, INT16 gamma, INT16 lambda){
  INT16 ret;
  INT32 i;

  FLOAT64 *in=dlp_malloc(n*sizeof(FLOAT64));
  FLOAT64 *out=dlp_malloc(order*sizeof(FLOAT64));

  for(i=0;i<n;i++) in[i]=(FLOAT64)input[i]/32767.*SIG_NRM;
  ret=dlm_mgcep(in,n,out,order,(FLOAT64)gamma/32767.,(FLOAT64)lambda/32767.,32768.);
  for(i=0;i<order;i++) output[i]=(INT16)round(out[i]*32767./RES_NRM);


  dlp_free(in);
  dlp_free(out);
  return ret;
}

