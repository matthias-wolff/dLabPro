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
    dlmx_mul16_32(a,b)                                                         /*  - Low part of a and b                */
  );                                                                           /*                                       */
}

/* Absolute value of complex INT32 number: sqrt(a^2+b^2)
 * TODO: speedup
 *
 * @param a  Real part
 * @param b  Imaginary part
 * @return   Absolute value
 */
INT32 dlmx_cabINT32(INT32 a,INT32 b){
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

