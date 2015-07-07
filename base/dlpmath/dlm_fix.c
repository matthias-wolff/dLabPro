/* dLabPro mathematics library
 * - Fixed point arithmetics
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

/* Round number INT32->INT16
 *
 * @param a  Input number
 * @return   Rounded number
 */
INT16 dlmx_rnd32(INT32 a){
  if(a>=0x7fff8000) return INT16_MAX;                                          /* Saturation                            */
  return (a+0x8000)>>16;                                                       /* Truncate + Round number               */
}

/* Absolute value of INT16 number
 *
 * @param a  Input number
 * @return   Absolute value
 */
INT16 dlmx_abs16(INT16 a){
  if(a==INT16_MIN) return INT16_MAX;
  return -a;
}

/* Left/Right shift for INT32
 *
 * @param a    Number to shift
 * @param shf  Shift width (pos -> left)
 * @return     Shifted number
 */
INT32 dlmx_shl32(INT32 a,INT8 shf){
  if(shf>0){                                                                   /* Left shift ? >>                       */
    if(a>0 &&    a&(INT32_MIN>>shf)) return INT32_MAX;                         /*   Saturate positive numbers           */
    if(a<0 && (~a)&(INT32_MIN>>shf)) return INT32_MIN;                         /*   Saturate negaitve numbers           */
    return a<<shf;                                                             /*   Apply left shift                    */
  }else return a>>-shf;                                                        /* << Else: apply right shift            */
}

/* Negate INT16
 *
 * @param a  Input number
 * @return   Negated number
 */
INT16 dlmx_neg16(INT16 a){
  return a==INT16_MIN ? INT16_MAX : -a;                                        /* Negate number with saturation         */
}

/* Add INT16+INT16->INT16
 *
 * @param a  First operand
 * @param b  Second operand
 * @return   Sum of a and b
 */
INT16 dlmx_add16(INT16 a,INT16 b){
  INT32 r=a+b;                                                                 /* Add in INT32                          */
  if(r>INT16_MAX) return INT16_MAX;                                            /* Saturate to maximum                   */
  if(r<INT16_MIN) return INT16_MIN;                                            /* Saturate to minimum                   */
  return r;                                                                    /* Return converted result               */
}

/* Negate INT32
 *
 * @param a  Input number
 * @return   Negated number
 */
INT32 dlmx_neg32(INT32 a){
  return a==INT32_MIN ? INT32_MAX : -a;                                        /* Negate number with saturation         */
}

/* Add INT32+INT32->INT32
 *
 * @param a  First operand
 * @param b  Second operand
 * @return   Sum of a and b
 */
INT32 dlmx_add32(INT32 a,INT32 b){
  INT32 r=a+b;                                                                 /* Unsaturated addition                  */
  if(a>=0 && b>=0 && r<0 ) return INT32_MAX;                                   /* Saturate on negative overflow         */
  if(a<0  && b<0  && r>=0) return INT32_MIN;                                   /* Saturate on positive overflow         */
  return r;                                                                    /* No overflow                           */
}

/* Subtract INT32-INT32->INT32
 * TODO: speedup
 *
 * @param a  First operand
 * @param b  Second operand
 * @return   Difference of a and b
 */
INT32 dlmx_sub32(INT32 a,INT32 b){
  if(b==INT32_MIN) return dlmx_add32(1,dlmx_add32(INT32_MAX,a));               /* Add -INT32_MIN                        */
  else return dlmx_add32(a,-b);                                                /* Use negation and addition             */
}

/* Multiply INT16*INT16->INT32
 *
 * @param a  First operand
 * @param b  Second operand
 * @return   Product of a and b
 */
INT32 dlmx_mul16_32(INT16 a,INT16 b){
  INT32 r=((INT32)a*(INT32)b)<<1;                                              /* Shifted product of a and b            */
  if(r==(INT32)INT32_MIN) return INT32_MAX;                                    /* Saturate on overflow                  */
  return r;                                                                    /* No overflow                           */
}

/* Multiply INT16*INT16->INT16
 *
 * @param a  First operand
 * @param b  Second operand
 * @return   Product of a and b
 */
INT16 dlmx_mul16(INT16 a,INT16 b){
  return dlmx_rnd32(dlmx_mul16_32(a,b));                                       /* Use INT32 multiply and round          */
}

/* Multiply INT32*INT32->INT32
 * TODO: speedup
 *
 * @param a  First operand
 * @param b  Second operand
 * @return   Product of a and b
 */
INT32 dlmx_mul32(INT32 a,INT32 b){
  return dlmx_add32(                                                           /* Sum over pratial products:            */
    dlmx_add32(                                                                /*                                       */
      dlmx_mul16_32(dlmx_rnd32(a),dlmx_rnd32(b)),                              /*  - High parts of a and b              */
      dlmx_mul16_32(a,dlmx_rnd32(b))                                           /*  - Low part of a and high part of b   */
    ),                                                                         /*                                       */
    dlmx_mul16(dlmx_rnd32(a),b)                                                /*  - High part of a and low part of b   */
  );                                                                           /*                                       */
}

/* Multiply INT32*INT16->INT32
 * TODO: speedup
 *
 * @param a  First operand
 * @param b  Second operand
 * @return   Product of a and b
 */
INT32 dlmx_mul3216(INT32 a,INT16 b){
  return dlmx_add32(                                                           /* Sum over pratial products:            */
    dlmx_mul16_32(dlmx_rnd32(a),b),                                            /*  - High part of a and b               */
    dlmx_mul16(a,b)                                                            /*  - Low part of a and b                */
  );                                                                           /*                                       */
}

/* Absolute value of complex INT32 number: sqrt(a^2+b^2)
 * TODO: speedup
 *
 * @param a  Real part
 * @param b  Imaginary part
 * @return   Absolute value
 */
INT32 dlmx_cabs32(INT32 a,INT32 b){
  return (INT32)round(sqrt((double)a*(double)a+(double)b*(double)b));          /* Calc using floating point conversion  */
}


/* Multiply matrix (INT16) and vector (INT16) to vector (INT16)
 *
 * @param m  First matrix dimension = Input vector dimension
 * @param n  Second matrix dimension = Output vector dimension
 * @param a  Matrix buffer
 * @param b  Input vector buffer
 * @param c  Output vector buffer
 */
void dlmx_matmul16(UINT16 m,UINT16 n,INT16 *a,INT16 *b,INT16 *c){
  UINT16 i;                                                                    /* Output dim. index                     */
  for(i=n;i;i--){                                                              /* Loop over output dim. >>              */
    register INT32 s=0;                                                        /*   Aggregated output value             */
    register UINT16 j;                                                         /*   Input dim. index                    */
    INT16 *bj=b;                                                               /*   Input vector reading pos            */
    for(j=m;j;j--) s=dlmx_add32(s,dlmx_mul16_32(*a++,*bj++));                  /*   Loop over input dim. + calc.        */
    *c++=dlmx_rnd32(s);                                                        /*   Round output value to tmp. buf.     */
  }                                                                            /* <<                                    */
}

/* Multiply two vectors (INT16)
 *
 * @param n  Vector dimension
 * @param a  First vector
 * @param b  Second vector
 */
INT16 dlmx_vecmul16(UINT16 n,INT16 *a,INT16 *b,INT8 shf){
  register INT32 s=0;                                                          /* Aggregated output value               */
  for(;n;n--,a++,b++) s=dlmx_add32(s,dlmx_mul16_32(*a++,*b++));                /* Mult. + aggr. vector components       */
  return dlmx_rnd32(dlmx_shl32(s,shf));
}

/* Add two vectors (INT16)
 *
 * @param n  Vector dimension
 * @param a  First vector
 * @param bc Second vector = output vector
 */
void dlmx_vecadd16(UINT16 n,INT16 *a,INT16 *bc){
  for(;n;n--,a++,bc++) *bc=dlmx_add16(*a,*bc);                                 /* Add vector components                 */
}


/* Fixed FFT parameters */
struct dlmx_fft {
  UINT8  xe;    /* Oder of transformation */
  INT8   inv;   /* Invertation flag       */
  INT16 *id;    /* Resorting id's         */
  INT16 *arg;   /* Tranformation factors  */
  INT32 *buf;   /* Tranformation buffer   */
};

/* Fixed FFT initialization
 *
 * @param len  Tranformation dimension (should be a power of 2)
 * @param inv  Invertation flag
 * @return     Parameter structure
 */
struct dlmx_fft *dlmx_fft_init(UINT16 len,INT8 inv){
  UINT16 i;                                                                    /* Loop index                            */
  UINT8 e;                                                                     /* Order index                           */
  INT16 *arg;                                                                  /* Current factor pointer                */
  struct dlmx_fft *fp=dlp_malloc(sizeof(struct dlmx_fft));                     /* Alloc parameter structure             */
  for(fp->xe=0;len>1;len>>=1) fp->xe++;                                        /* Get Order from dimension              */
  len=1<<fp->xe;                                                               /* Update dimension to a power of 2      */
  fp->inv=inv;                                                                 /* Copy invertation flag                 */
  fp->id=dlp_malloc(len*sizeof(INT16));                                        /* Alloc sort id buffer                  */
  arg=fp->arg=dlp_malloc((len-2)*2*sizeof(INT16));                             /* Alloc factor buffer                   */
  fp->buf=dlp_malloc(len*2*sizeof(INT32));                                     /* Alloc temporary buffer                */
  for(i=0;i<len;i++){                                                          /* Loop over dimension >>                */
    INT16 ix,j=0,jx=1<<(fp->xe-1);                                             /*   Init sort id calculation            */
    for(ix=i;ix;ix>>=1,jx>>=1) if(ix&1) j|=jx;                                 /*   Calc sort id by bit reversal        */
    fp->id[i]=j;                                                               /*   Save sort id                        */
  }                                                                            /* <<                                    */
  for(e=2;e<fp->xe;e++){                                                       /* Loop over FFT order >>                */
    for(i=1;i<1<<e;i++,arg+=2){                                                /*   Loop over order dimension           */
      arg[0]=(INT16)round(cos(M_PI*(double)i/(double)(1<<e))*INT16_MAX);       /*     Calc real part of factor          */
      arg[1]=(INT16)round(sin(M_PI*(double)i/(double)(1<<e))*INT16_MAX         /*     Calc imaginary part of factor     */
        *(fp->inv?1.:-1.));                                                    /*     |                                 */
    }                                                                          /*   <<                                  */
  }                                                                            /* <<                                    */
  return fp;                                                                   /* Return parameter structure            */
}

/* Fixed FFT free parameter function
 *
 * @param fp   Parameter structure
 */
void dlmx_fft_free(struct dlmx_fft *fp){
  dlp_free(fp->id);                                                            /* Free sort id buffer                   */
  dlp_free(fp->arg);                                                           /* Free factor buffer                    */
  dlp_free(fp->buf);                                                           /* Free temporary buffer                 */
  dlp_free(fp);                                                                /* Free parameter structure              */
}

/* Fixed FFT free parameter function
 *
 * @param fp   Parameter structure
 * @param re   Array containing real part of input and output data
 * @param im   Array containing imaginary part of input and output data
 * @param shr  Right shift offset (applied within transformation)
 */
void dlmx_fft(struct dlmx_fft *fp,INT16 *re,INT16 *im,INT8 shr){
  const UINT16 len=1<<fp->xe;                                                  /* Get FFT dimension                     */
  UINT16 e,a,i;                                                                /* Loop indizies                         */
  INT16 *arg=fp->arg;                                                          /* Current factor pointer                */
  INT32 *buf=fp->buf;                                                          /* Current buffer position               */
  INT8 shfi=16-shr-(fp->inv?fp->xe:0);                                         /* Initial shift offset                  */
  INT8 shfo=shfi/3; shfi-=shfo;                                                /* 1/3 shift at output ; 2/3 at input    */
  /* Resorting + first two interations */
  for(i=0;i<len;i+=4,buf+=8){                                                  /* Loop over blocks of 4 cmplx. values   */
    INT32 x1=dlmx_shl32(re[fp->id[i+0]],shfi);                                 /*   Get 1. real part                    */
    INT32 x2=dlmx_shl32(re[fp->id[i+1]],shfi);                                 /*   Get 2. real part                    */
    INT32 x3=dlmx_shl32(re[fp->id[i+2]],shfi);                                 /*   Get 3. real part                    */
    INT32 x4=dlmx_shl32(re[fp->id[i+3]],shfi);                                 /*   Get 4. real part                    */
    INT32 y1=dlmx_shl32(im[fp->id[i+0]],shfi);                                 /*   Get 1. imaginary part               */
    INT32 y2=dlmx_shl32(im[fp->id[i+1]],shfi);                                 /*   Get 2. imaginary part               */
    INT32 y3=dlmx_shl32(im[fp->id[i+2]],shfi);                                 /*   Get 3. imaginary part               */
    INT32 y4=dlmx_shl32(im[fp->id[i+3]],shfi);                                 /*   Get 4. imaginary part               */
    INT32 v1=dlmx_add32(x1,x2);                                                /*   Temporary value                     */
    INT32 v2=dlmx_add32(x3,x4);                                                /*   Temporary value                     */
    buf[0]=dlmx_add32(v1,v2);                                                  /*   Calc 1. real part                   */
    buf[4]=dlmx_sub32(v1,v2);                                                  /*   Calc 3. real part                   */
    v1=dlmx_add32(y1,y2);                                                      /*   Temporary value                     */
    v2=dlmx_add32(y3,y4);                                                      /*   Temporary value                     */
    buf[1]=dlmx_add32(v1,v2);                                                  /*   Calc 1. imaginary part              */
    buf[5]=dlmx_sub32(v1,v2);                                                  /*   Calc 3. imaginary part              */
    v1=dlmx_sub32(x1,x2);                                                      /*   Temporary value                     */
    v2=dlmx_sub32(y3,y4);                                                      /*   Temporary value                     */
    buf[fp->inv?6:2]=dlmx_add32(v1,v2);                                        /*   Calc 2./4. real part                */
    buf[fp->inv?2:6]=dlmx_sub32(v1,v2);                                        /*   Calc 4./2. real part                */
    v1=dlmx_sub32(y1,y2);                                                      /*   Temporary value                     */
    v2=dlmx_sub32(x3,x4);                                                      /*   Temporary value                     */
    buf[fp->inv?7:3]=dlmx_sub32(v1,v2);                                        /*   Calc 2./4. imaginary part           */
    buf[fp->inv?3:7]=dlmx_add32(v1,v2);                                        /*   Calc 4./2. imaginary part           */
  }                                                                            /* <<                                    */
  /* Remaining interations */
  for(e=2;e<fp->xe;arg+=((1<<(e++))-1)*2){                                     /* Loop over remaining interations >>    */
    for(a=0;a<1<<(fp->xe-e-1);a++){                                            /*   Loop over blocks >>                 */
      INT32 *x=fp->buf+(1<<(e+2))*a, *y=x+(1<<(e+1));                          /*     Get 1. and 2. block position      */
      INT16 *z=arg;                                                            /*     Get factor iterator               */
      /* First butterfly */
      INT32 yzr=y[0];                                                          /*     Copy 2. real part                 */
      INT32 yzi=y[1];                                                          /*     Copy 2. imaginary part            */
      y[0]=dlmx_sub32(x[0],yzr);                                               /*     Update 2. real part               */
      y[1]=dlmx_sub32(x[1],yzi);                                               /*     Update 2. imaginary part          */
      x[0]=dlmx_add32(x[0],yzr);                                               /*     Update 1. real part               */
      x[1]=dlmx_add32(x[1],yzi);                                               /*     Update 1. imaginary part          */
      /* Remaining butterfly's */
      for(i=1,x+=2,y+=2;i<1<<e;i++,x+=2,y+=2,z+=2){                            /*     Loop over remaining butterfly's >>*/
        INT32 yzr=dlmx_sub32(dlmx_mul3216(y[0],z[0]),dlmx_mul3216(y[1],z[1])); /*       Calc 1. argument                */
        INT32 yzi=dlmx_add32(dlmx_mul3216(y[0],z[1]),dlmx_mul3216(y[1],z[0])); /*       Calc 2. argument                */
        y[0]=dlmx_sub32(x[0],yzr);                                             /*       Update 2. real part             */
        y[1]=dlmx_sub32(x[1],yzi);                                             /*       Update 2. imaginary part        */
        x[0]=dlmx_add32(x[0],yzr);                                             /*       Update 1. real part             */
        x[1]=dlmx_add32(x[1],yzi);                                             /*       Update 1. imaginary part        */
      }                                                                        /*     <<                                */
    }                                                                          /*   <<                                  */
  }                                                                            /* <<                                    */
  /* Convert output values */
  for(i=0,buf=fp->buf;i<len;i++,buf+=2){                                       /* Loop over dimension >>                */
    re[i]=dlmx_rnd32(dlmx_shl32(buf[0],shfo));                                 /*   Calc real output                    */
    im[i]=dlmx_rnd32(dlmx_shl32(buf[1],shfo));                                 /*   Calc imaginary output               */
  }                                                                            /* <<                                    */
}


