/* dLabPro mathematics library
 * - Fast Wavelet Transform - unoptimized
 *
 * AUTHOR : Soeren Wittenberg
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

/* define */
#define SQRT2 1.41421356237309505  /* square of 2 */
#define SQRT3 1.73205080756887729  /* square of 3 */

const FLOAT64 wvlh[10][20]={
  { 1./SQRT2,1./SQRT2 },
  { (1.+SQRT3)/4/SQRT2,(3.+SQRT3)/4/SQRT2,(3.-SQRT3)/4/SQRT2,(1.-SQRT3)/4/SQRT2 },
  { 0.33267055,0.80689151,0.4598775,-0.13501102,-0.085441275,0.035226292 },
  { 0.23037781,0.71484657,0.63088166,-0.028054483,-0.18703481,0.030841382,0.032883011,-0.010597402 },
  { 0.1601024,0.60382927,0.72430853,0.13842814,-0.24229488,-0.032244868,0.077571488,-0.0062414901,-0.012580752,0.0033357253 },
  { 0.11154074,0.49462389,0.75113391,0.31525035,-0.22626469,-0.12976686,0.097501603,0.027522866,-0.03158204,0.0005538422,0.0047772575,-0.0010773011 },
  { 0.077852054,0.39653932,0.72913209,0.46978229,-0.143906,-0.22403618,0.071309385,0.080612613,-0.038029935,-0.016574541,0.012550997,0.00042957797,-0.0018016407,0.0003537138 },
  { 0.054415841,0.31287159,0.67563074,0.58535468,-0.015829109,-0.28401554,0.00047248457,0.12874743,-0.0173693,-0.044088256,0.013981028,0.0087460906,-0.004870353,-0.00039174037,0.00067544941,-0.00011747678 },
  { 0.038077948,0.24383467,0.60482312,0.65728807,0.13319739,-0.29327378,-0.096840784,0.14854075,0.030725685,-0.067632826,0.00025094711,0.022361665,-0.0047232048,-0.0042815037,0.0018476469,0.00023038576,-0.00025196319,-3.934732e-05 },
  { 0.02667006,0.1881768,0.52720119,0.68845904,0.28117234,-0.24984642,-0.19594628,0.12736934,0.093057367,-0.071394146,-0.029457538,0.033212671,0.0036065536,-0.010733174,0.0013953517,0.0019924053,-0.0006858567,-0.00011646686,9.358867e-05,-1.3264199e-05 }
};


 /*   Daubechies wavelet coefficient lookup function
 *
 *    INT16    di          Daubechies index
 *    return               Pointer to wavelet coefficients (di Values)
 */
const FLOAT64* CGEN_IGNORE dlm_fwt_geth(INT16 di){
  if(di<2 || di>20 || di%2) return NULL;
  return wvlh[di/2-1];
};


 /*   Daubechies wavelet transform
 *
 *    FLOAT64* sig         signal array
 *    FLOAT64* trans       transformed signal
 *    INT32    size        signal size
 *    INT16    di          Daubechies index
 *    INT16    level       detail level (max. level = -1)  
  */
INT16 CGEN_IGNORE dlm_fwt_dx(FLOAT64* sig, FLOAT64* trans, INT32 size, INT16 di, INT16 level)
{
  INT32 l,i;
  FLOAT64 *sigTemp;
  const FLOAT64 *h;
  FLOAT64 *t1, *t2;
  

  if(size<di || !(h=dlm_fwt_geth(di))) return NOT_EXEC;
  
  sigTemp = (FLOAT64*)dlp_calloc(size+size/2, sizeof(FLOAT64));
  dlp_memmove(sigTemp, sig, size * sizeof(FLOAT64));
  t1=sigTemp; t2=sigTemp+size;
  
  for(l=size;l>=di;l>>=1){
    INT32 l2=l>>1;
    for(i=0;i<l2;i++,t1+=2){
      register INT32 j;
      register FLOAT64 s=0,o=0;
      for(j=0;j<di;j++){
        register FLOAT64 v = i*2+j>=l ? t1[j-l] : t1[j];
        s+=v*h[j];
        o+=v*h[di-1-j]*(j%2?-1:1);
      }
      *t2++=s;       /* scaling function coefficients */
      trans[l2+i]=o; /* wavelet function coefficents */
    }
    if(t1>t2) t1=sigTemp; else t2=sigTemp;
    level--;
    if(level == 0) break; /* reduce detail level and exit if reaching 0; if detail level is max. (-1) nothing happens */
  }
  for(i=0;i<l;i++) trans[i]=*t1++;
  
  dlp_free(sigTemp);
  
  return O_K;
}

 /*   Inverse Daubechies wavelet transform
 *
 *    FLOAT64* trans       transformed signal
 *    FLOAT64* sig         signal array 
 *    INT32    size        transformed signal array size
 *    INT16    di          Daubechies index
 *    INT16    level       detail level (max. level = -1) that was used to transform signal  
 */
INT16 CGEN_IGNORE dlm_fwt_dx_inv(FLOAT64* trans, FLOAT64* sig, INT32 size, INT16 di, INT16 level)
{
  register INT32 i;
  register INT32 n;
  register INT32 size2;
  FLOAT64*  transTemp;
  const FLOAT64 *h;

  if(size<di || !(h=dlm_fwt_geth(di))) return NOT_EXEC;
  if(di!=4) return NOT_EXEC; /* TODO: synthesis for any Daubechies index */

  transTemp = (FLOAT64*)dlp_calloc(size, sizeof(FLOAT64));
  dlp_memmove(transTemp, trans, size * sizeof(FLOAT64));

  if(level > 0)
  {
    n = size >> (level-1);
    if(n<4) n=4;
  }
  else
    n=4;

  for (; n <= size; n <<= 1)
  {
    size2 = n >> 1;
    sig[0] = transTemp[size2-1]*h[2] + transTemp[size2+size2-1]*h[1] + transTemp[0]*h[0] + transTemp[size2]*h[3];
    sig[1] = transTemp[size2-1]*h[3] - transTemp[size2+size2-1]*h[0] + transTemp[0]*h[1] - transTemp[size2]*h[2];
        
    for (i = 0; i < size2-1; i++)
    {
      sig[2*i+2] = transTemp[i]*h[2] + transTemp[i+size2]*h[1] + transTemp[i+1]*h[0] + transTemp[i+size2+1]*h[3];
      sig[2*i+3] = transTemp[i]*h[3] - transTemp[i+size2]*h[0] + transTemp[i+1]*h[1] - transTemp[i+size2+1]*h[2];
    }
    
    dlp_memmove(transTemp, sig, n * sizeof(FLOAT64));
    level--;
    if(level == 0) break;
  }
 
  dlp_free(transTemp);
  return O_K;
}

