/* dLabPro mathematics library
 * - Basic matrix functions
 *
 * AUTHOR : Matthias Wolff,
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

#ifdef _B
  #undef _B                                                                     /* Because of /usr/include/ctype.h   */
#endif
#define _A(r,c) A[c*nXRa+r]                                                     /* Element of matrix A (l/r-value)   */
#define _B(r,c) B[c*nXRb+r]                                                     /* Element of matrix B (l/r-value)   */
#define _Z(r,c) Z[c*nXRz+r]                                                     /* Element of matrix Z (l/r-value)   */
#define _DMS(A,n,v) { INT32 i; for (i=0; i<n; i++) A[i]=v;        }             /* Memset for FLOAT64 array          */

/* ---------------------------------------------------------------------------*/
/* Operation table                                                            */

/**
 * Matrix operation table
 *
 * Signature declares input and output type:
 *   - Upper letters -> matrix
 *   - Lower letters -> scalar
 *   - o             -> operation,
 *   - R,r           -> real matrix, scalar,
 *   - N,n           -> real integer matrix, scalar
 *   - C,c           -> complex matrix, scalar,
 *   - D,d           -> input: auto-determine,
 *                   -> output depends on input.
 */
static const opcode_table __mtab[] =
{
  { OP_REAL        ,1,1,"R:oD" ,"Real part of (complex) matrix"      ,"real"      },
  { OP_IMAG        ,1,1,"R:oD" ,"Imaginary part of (complex) matrix" ,"imag"      },
  { OP_CONJ        ,1,1,"D:oD" ,"Complex conjugate"                  ,"conj"      },
  { OP_TRANSPOSE   ,1,1,"D:Do" ,"Transpose"                          ,"\'"        },
  { OP_INVT        ,1,1,"D:oD" ,"Inversion"                          ,"inv"       },
  { OP_ADD         ,1,2,"D:DoD","Addition"                           ,"+"         },
  { OP_DIFF        ,1,2,"D:DoD","Difference"                         ,"-"         },
  { OP_ABSDIFF     ,1,2,"R:DoD","Absolute difference"                ,"absdiff"   },
  { OP_QABSDIFF    ,1,2,"R:DoD","Quadratic absolute difference"      ,"qabsdiff"  },
  { OP_MULT        ,1,2,"D:DoD","Multiplication"                     ,"*"         },
  { OP_MULT_SPARSE ,1,2,"D:oDD","Sparse multiplication"              ,"musp"      },
  { OP_ABS         ,1,1,"R:oD" ,"Elementwise absolute values"        ,"abs"       },
  { OP_CHOLF       ,1,1,"D:oD" ,"Cholesky factorization"             ,"cholf"     },
  { OP_CONV        ,1,2,"D:DoD","Convolution"                        ,"conv"      },
  { OP_LN          ,1,1,"D:oD" ,"Natural logarithm"                  ,"ln"        },
  { OP_LOG         ,1,1,"D:oD" ,"Decatic logarithm"                  ,"log"       },
  { OP_EXP         ,1,1,"D:oD" ,"Exponential function"               ,"exp"       },
  { OP_POW         ,1,2,"D:Don","Power"                              ,"^"         },
  { OP_POW         ,1,2,"D:Don","Power"                              ,"pow"       },
  { OP_SQRT        ,1,1,"D:oD" ,"Square root"                        ,"sqrt"      },
  { OP_DIAG        ,1,1,"D:oD" ,"Make diagonal matrix"               ,"diag"      },
  { OP_MDIAG       ,1,1,"D:oD" ,"Get main diagonal"                  ,"mdiag"     },
  { OP_DET         ,1,1,"n:oD" ,"Determinant"                        ,"det"       },
  { OP_TRACE       ,1,1,"n:oD" ,"Trace"                              ,"tr"        },
  { OP_RANK        ,1,1,"D:oD" ,"Rank"                               ,"rank"      },
  { OP_MULT_EL     ,1,2,"D:DoD","Elementwise multiplication"         ,".*"        },
  { OP_DIV_EL      ,1,2,"D:DoD","Elementwise division"               ,"./"        },
  { OP_MULT_KRON   ,1,2,"D:DoD","Kronecker product"                  ,".*."       },
  { OP_MULT_AKAT   ,1,2,"D:oDD","Matrix product A*K*A'"              ,"akat"      },
  { OP_LOG_EL      ,1,1,"D:oD" ,"Elementwise decadic log."           ,".log"      },
  { OP_LOG2_EL     ,1,1,"D:oD" ,"Elementwise binary log."            ,".log2"     },
  { OP_LN_EL       ,1,1,"D:oD" ,"Elementwise natural log."           ,".ln"       },
  { OP_EXP_EL      ,1,1,"D:oD" ,"Elementwise exponential fnct."      ,".exp"      },
  { OP_SQRT_EL     ,1,1,"D:oD" ,"Elementwise square root"            ,".sqrt"     },
  { OP_POW_EL      ,1,2,"D:DoD","Elementwise power"                  ,".pow"      },
  { OP_POW_EL      ,1,2,"D:DoD","Elementwise power"                  ,".^"        },
  { OP_OR_EL       ,1,2,"D:DoD","Elementwise logical or"             ,".||"       },
  { OP_BITOR_EL    ,1,2,"D:DoD","Elementwise bitwise or"             ,".|"        },
  { OP_AND_EL      ,1,2,"D:DoD","Elementwise logical and"            ,".&&"       },
  { OP_BITAND_EL   ,1,2,"D:DoD","Elementwise bitwise and"            ,".&"        },
  { OP_EQUAL_EL    ,1,2,"D:DoD","Elementwise equal"                  ,".=="       },
  { OP_NEQUAL_EL   ,1,2,"D:DoD","Elementwise not equal"              ,".!="       },
  { OP_LESS_EL     ,1,2,"D:DoD","Elementwise less than"              ,".<"        },
  { OP_GREATER_EL  ,1,2,"D:DoD","Elementwise greater than"           ,".>"        },
  { OP_LEQ_EL      ,1,2,"D:DoD","Elementwise less or equal"          ,".<="       },
  { OP_GEQ_EL      ,1,2,"D:DoD","Elementwise greater or equal"       ,".>="       },
  { OP_BITOR       ,1,2,"D:DoD","Elementwise bitwise or"             ,"|"         },
  { OP_BITAND      ,1,2,"D:DoD","Elementwise bitwise and"            ,"&"         },
  { OP_ZEROS       ,1,2,"D:onn","0-matrix (dim. x by y)"             ,"zeros"     },
  { OP_ONES        ,1,2,"D:onn","1-matrix (dim. x by y)"             ,"ones"      },
  { OP_NOISE       ,1,2,"D:onn","White noise matrix (dim. x by y)"   ,"noise"     },
  { OP_UNITMAT     ,1,1,"D:on" ,"Unit matrix (dim. x)"               ,"unit"      },
  { OP_HILBMAT     ,1,1,"D:on" ,"Hilbert matrix (dim. x)"            ,"hilb"      },
  { OP_IHLBMAT     ,1,1,"D:on" ,"Inv. Hilbert mat. (dim. x)"         ,"ihlb"      },
  { OP_CCF         ,1,2,"D:DoD" ,"Cross correlation function"        ,"ccf"       },
  { OP_SOLV_LU     ,1,2,"D:oDD" ,"Solve AX=B using LU decomposition" ,"solve_lud" },

  /* do not change the following line!               */
  /* The values are used for end of table detection. */
  { -1             ,0,0,""      ,"?"                                 ,""     }
};

/* ---------------------------------------------------------------------------*/
/* Worker functions (local scope)                                             */

/**
 * Debugging use only. Prints a matrix.
 *
 * @param A
 *          Pointer to matrix
 * @param nXR
 *          Number of rows in <code>A</code>
 * @param nXC
 *          Number of columns in <code>A</code>
 */
void dlm_print(const FLOAT64* A, INT32 nXR, INT32 nXC)
{
  INT32 nR,nC;
  for (nR=0; nR<nXR; nR++)
  {
    printf("\n");
    for (nC=0; nC<nXC; nC++) printf("%4g ",(double)A[nC*nXR+nR]);
  }
}

/*
 * Matrix transpose: Z=A'
 */
INT16 CGEN_IGNORE dlm_transpose(FLOAT64* Z, const FLOAT64* A, INT32 nXR, INT32 nXC)
{
  INT32 nR = 0;
  INT32 nC = 0;

  for (nC=0; nC<nXC; nC++)
    for (nR=0; nR<nXR; nR++)
      Z[nR*nXC+nC] = A[nC*nXR+nR];

  return O_K;
}

/*
 * Complex variant of dlm_transpose
 */
INT16 CGEN_IGNORE dlm_transposeC(COMPLEX64* Z, const COMPLEX64* A, INT32 nXR, INT32 nXC)
{
  INT32 nR = 0;
  INT32 nC = 0;

  for (nC=0; nC<nXC; nC++) {
    for (nR=0; nR<nXR; nR++) {
      Z[nR*nXC+nC] = CMPLXY(A[nC*nXR+nR].x,-A[nC*nXR+nR].y);
    }
  }
  return O_K;
}

/*
 * Matrix multiplication: Z=A*B
 *
 * REMARK: Implementation is N^3, there is an algorithm N^(log2 7), for N=1024
 *         this would make a speed up down to 25%, see "Numerical Recipes in C"
 */
INT16 CGEN_IGNORE dlm_mult
(
  FLOAT64*       Z,
  const FLOAT64* A,
  INT32          nXRa,
  INT32          nXCa,
  const FLOAT64* B,
  INT32          nXRb,
  INT32          nXCb
)
{
  const FLOAT64* a    = NULL;
  const FLOAT64* b    = NULL;
  FLOAT64*       z    = NULL;
  const FLOAT64* a0   = NULL;
  const FLOAT64* b0   = NULL;
  INT32          i    = 0;
  INT32          nR   = 0;
  INT32          nC   = 0;

  /* Validate */                                                                /* --------------------------------- */
  DLPASSERT(nXCa==nXRb);                                                        /* Dimension error!                  */
  DLPASSERT(Z!=A && Z!=B);                                                      /* Result undefined!                 */
#ifndef __NOXALLOC
  DLPASSERT(dlp_size(Z)>=nXRa*nXCb*sizeof(FLOAT64));                            /* Result buffer too small!          */
#endif

  /* Do matrix multiplication */                                                /* --------------------------------- */
  for (z=Z,b0=B,nC=0; nC<nXCb; nC++,b0+=nXRb)
    for (nR=0,a0=A; nR<nXRa; nR++,z++,a0++)
      for (*z=0.0f,a=a0,b=b0,i=nXCa; i;i--,a+=nXRa,b++)
        *z+=*a**b;

  return O_K;
}

/*
 * Complex variant of dlm_multC
 */
INT16 CGEN_IGNORE dlm_multC
(
  COMPLEX64*       Z,
  const COMPLEX64* A,
  INT32            nXRa,
  INT32            nXCa,
  const COMPLEX64* B,
  INT32            nXRb,
  INT32            nXCb
)
{
  const COMPLEX64* a     = NULL;
  const COMPLEX64* b     = NULL;
  COMPLEX64*       z     = NULL;
  const COMPLEX64* a0    = NULL;
  const COMPLEX64* b0    = NULL;
  INT32            i     = 0;
  INT32            nR    = 0;
  INT32            nC    = 0;

  /* Validate */                                                                /* --------------------------------- */
  DLPASSERT(nXCa==nXRb);                                                        /* Dimension error!                  */
  DLPASSERT(Z!=A && Z!=B);                                                      /* Result undefined!                 */
#ifndef __NOXALLOC
  DLPASSERT(dlp_size(Z)>=nXRa*nXCb*sizeof(COMPLEX64));                          /* Result buffer too small!          */
#endif

  /* Do matrix multiplication */                                                /* --------------------------------- */
  for (z=Z,b0=B,nC=0; nC<nXCb; nC++,b0+=nXRb) {
    for (nR=0,a0=A; nR<nXRa; nR++,z++,a0++) {
      for (*z=CMPLX(0.0f),a=a0,b=b0,i=nXCa; i;i--,a+=nXRa,b++) {
        *z=dlp_scalopC(*z,dlp_scalopC(*a,*b,OP_MULT),OP_ADD);
      }
    }
  }
  return O_K;
}

/*
 * Creates special matrices.
 */
INT16 CGEN_IGNORE dlm_constant(FLOAT64* Z, INT32 nXRz, INT32 nXCz, INT16 nOpcode)
{
  INT32    nR = 0;                                                              /* Current row                       */
  INT32    nC = 0;                                                              /* Current column                    */
  FLOAT64* z  = NULL;                                                           /* Pointer to current element of Z   */

  DLPASSERT(dlp_size(Z)>=nXRz*nXCz*sizeof(FLOAT64));                            /* Result buffer too small!          */

  /* Generate constant matrices */                                              /* --------------------------------- */
  for (z=Z,nC=0;nC<nXCz; nC++)                                                  /* Loop over columns                 */
    for (nR=0;nR<nXRz; nR++,z++)                                                /*   Loop over rows                  */
      switch (nOpcode)                                                          /*     Branch for operation code     */
      {                                                                         /*     >>                            */
        case OP_UNITMAT:                                                        /*       Unit matrix                 */
          *z = (nR==nC)?1.:0.;                                                  /*         Compute z_ik              */
          break;                                                                /*       ==                          */
        case OP_HILBMAT:                                                        /*       Hilbert matrix              */
          *z = 1./((FLOAT64)(nR+nC+1));                                         /*         Compute z_ik              */
          break;                                                                /*       ==                          */
        case OP_IHLBMAT:                                                        /*       Inverse Hilbert matrix      */
          *z = dlp_scalop(-1.,(nR+nC+2),OP_POW) * (FLOAT64)(nR+nC+1)            /*         Compute z_ik              */
             * dlp_scalop(nXRz+nR,nXRz-nC-1,OP_NOVERK)                          /*         |                         */
             * dlp_scalop(nXRz+nC,nXRz-nR-1,OP_NOVERK)                          /*         |                         */
             * dlp_scalop(dlp_scalop(nR+nC,nR,OP_NOVERK),2.,OP_POW);            /*         |                         */
          break;                                                                /*       ==                          */
        case OP_ONES:                                                           /*       1-matrix                    */
          *z = 1.;                                                              /*          Fill with ones           */
          break;                                                                /*       ==                          */
        case OP_NOISE:                                                          /*       White noise matrix          */
          *z = dlp_frand();                                                     /*          Fill with random numbers */
          break;                                                                /*       ==                          */
        case OP_ZEROS:                                                          /*       0-matrix                    */
        default:                                                                /*       Default case                */
          *z = 0.;                                                              /*          Fill with zeros          */
      }                                                                         /*     <<                            */

  return O_K;                                                                   /* What ever...                      */
}

/*
 * Complex variant of dlm_constant.
 */
INT16 CGEN_IGNORE dlm_constantC(COMPLEX64* Z, INT32 nXRz, INT32 nXCz, INT16 nOpcode)
{
  INT32      nR = 0;                                                            /* Current row                       */
  INT32      nC = 0;                                                            /* Current column                    */
  COMPLEX64* z  = NULL;                                                         /* Pointer to current element of Z   */

  DLPASSERT(dlp_size(Z)>=nXRz*nXCz*sizeof(COMPLEX64));                          /* Result buffer too small!          */

  /* Generate constant matrices */                                              /* --------------------------------- */
  for (z=Z,nC=0;nC<nXCz; nC++) {                                                /* Loop over columns                 */
    for (nR=0;nR<nXRz; nR++,z++) {                                              /*   Loop over rows                  */
      switch (nOpcode) {                                                        /*     Branch for operation code     */
        case OP_UNITMAT: {                                                      /*       Unit matrix                 */
          *z = (nR==nC)?CMPLX(1.):CMPLX(0.);                                    /*           |                       */
          break; }                                                              /*       ==                          */
        case OP_HILBMAT: {                                                      /*       Hilbert matrix              */
          *z = CMPLX(1./((FLOAT64)(nR+nC+1)));                                  /*         |                         */
          break; }                                                              /*         |                         */
        case OP_IHLBMAT: {                                                      /*       Inverse Hilbert matrix      */
          *z = CMPLX(                                                           /*         |                         */
              dlp_scalop (-1.,(nR+nC+2),OP_POW)                                 /*         |                         */
              * (FLOAT64)(nR+nC+1)                                              /*         |                         */
              * dlp_scalop(nXRz+nR,nXRz-nC-1,OP_NOVERK)                         /*         |                         */
              * dlp_scalop(nXRz+nC,nXRz-nR-1,OP_NOVERK)                         /*         |                         */
              * dlp_scalop(dlp_scalop(nR+nC,nR,OP_NOVERK),2.,OP_POW));          /*         |                         */
          break; }
        case OP_ONES : { *z = CMPLX(1.); break; }                               /*       1-matrix                    */
        case OP_NOISE: { *z = CMPLX(dlp_frand()); break; }                      /*       White noise matrix          */
        case OP_ZEROS:                                                          /*       0-matrix                    */
        default      : { *z = CMPLX(0.); break; }                               /*       Default case                */
      }                                                                         /*     <<                            */
    }                                                                           /*   <<                              */
  }                                                                             /* <<                                */
  return O_K;                                                                   /* What ever...                      */
}

/*
 * Matrix-scalar operation: "left" Z=op(x,A) or "right" Z=op(A,x)
 */
INT16 CGEN_IGNORE dlm_scalop
(
  FLOAT64*       Z,
  const FLOAT64* A,
  INT32          nXR,
  INT32          nXC,
  FLOAT64        x,
  INT16         nOpcode,
  INT16         bLeft
)
{
  INT32          nR = 0;                                                        /* Current row                       */
  INT32          nC = 0;                                                        /* Current column                    */
  FLOAT64*       z  =  NULL;                                                    /* Pointer to current element of Z   */
  const FLOAT64* a  =  NULL;                                                    /* Pointer to current element of A   */

  /* Validate */                                                                /* --------------------------------- */
  DLPASSERT(Z!=A);                                                              /* Result undefined!                 */
  DLPASSERT(dlp_size(Z)>=nXR*nXC*sizeof(FLOAT64));                              /* Result buffer too small!          */

  /* z_ik = op(a_ik,x) or z_ik = op(x,a_ik) */                                  /* --------------------------------- */
  for (z=Z,a=A,nC=0; nC<nXC; nC++)                                              /* Loop over columns                 */
    for (nR=0; nR<nXR; nR++,z++,a++)                                            /*   Loop over rows                  */
      if (bLeft)                                                                /*     "Left": z_ik = op(x,a_ik)     */
        *z = dlp_scalop(x,*a,nOpcode);                                          /*       Compute result element      */
      else                                                                      /*     "Right": z_ik = op(a_ik,x)    */
        *z = dlp_scalop(*a,x,nOpcode);                                          /*       Compute result element      */

  return O_K;                                                                   /* Ok                                */
}

/*
 * Complex variant of dlm_scalop
 */
INT16 CGEN_IGNORE dlm_scalopC
(
  COMPLEX64*       Z,
  const COMPLEX64* A,
  INT32            nXR,
  INT32            nXC,
  const COMPLEX64* x,
  INT16            nOpcode,
  INT16            bLeft
)
{
  INT32       nR = 0;                                                           /* Current row                       */
  INT32       nC = 0;                                                           /* Current column                    */
  COMPLEX64*       z  =  NULL;                                                  /* Pointer to current element of Z   */
  const COMPLEX64* a  =  NULL;                                                  /* Pointer to current element of A   */

  /* Validate */                                                                /* --------------------------------- */
  DLPASSERT(Z!=A);                                                              /* Result undefined!                 */
  DLPASSERT(dlp_size(Z)>=nXR*nXC*sizeof(COMPLEX64));                            /* Result buffer too small!          */

  /* z_ik = op(a_ik,x) or z_ik = op(x,a_ik) */                                  /* --------------------------------- */
  for (z=Z,a=A,nC=0; nC<nXC; nC++) {                                            /* Loop over columns                 */
    for (nR=0; nR<nXR; nR++,z++,a++) {                                          /*   Loop over rows                  */
      if (bLeft) {                                                              /*     "Left": z_ik = op(x,a_ik)     */
        *z = dlp_scalopC(*x,*a,nOpcode);                /*       |                           */
      } else {                                                                  /*     "Right": z_ik = op(a_ik,x)    */
        *z = dlp_scalopC(*a,*x,nOpcode);                /*       |                           */
      }
    }
  }
  return O_K;                                                                   /* Ok                                */
}

/*
 * Elementwise matrix-matrix operation: (z_ik)=op(a_ik,b_ik)
 */
INT16 CGEN_IGNORE dlm_elemop
(
  FLOAT64*       Z,
  const FLOAT64* A,
  INT32          nXRa,
  INT32          nXCa,
  const FLOAT64* B,
  INT32          nXRb,
  INT32          nXCb,
  INT16          nOpcode
)
{
  INT32    nR   = 0;                                                            /* Current row                       */
  INT32    nC   = 0;                                                            /* Current column                    */
  INT32    nXRz = 0;                                                            /* Number of rows in result          */
  INT32    nXCz = 0;                                                            /* Number of columns in result       */
  FLOAT64* z    =  NULL;                                                        /* Pointer to current element of Z   */

  /* Initialize */                                                              /* --------------------------------- */
  nXRz = MAX(nXRa,nXRb);                                                        /* Number of rows in result          */
  nXCz = MAX(nXCa,nXCb);                                                        /* Number of columns in result       */

  /* Validate */                                                                /* --------------------------------- */
  DLPASSERT(Z!=A && Z!=B);                                                      /* Result undefined!                 */
  DLPASSERT(dlp_size(Z)>=nXRz*nXCz*sizeof(FLOAT64));                            /* Result buffer too small!          */

  /* z_ik = op(a_ik,b_ik) */                                                    /* --------------------------------- */
  for (z=Z,nC=0; nC<nXCz; nC++)                                                 /* Loop over columns                 */
    for (nR=0; nR<nXRz; nR++,z++)                                               /*   Loop over rows                  */
      *z = dlp_scalop(nR<nXRa&&nC<nXCa?_A(nR,nC):0.,                            /*     Compute result element        */
        nR<nXRb&&nC<nXCb?_B(nR,nC):0,nOpcode);                                  /*     |                             */

  return O_K;                                                                   /* Ok                                */
}

/*
 * Complex variant of dlm_elemop
 */
INT16 CGEN_IGNORE dlm_elemopC
(
  COMPLEX64*       Z,
  const COMPLEX64* A,
  INT32            nXRa,
  INT32            nXCa,
  const COMPLEX64* B,
  INT32            nXRb,
  INT32            nXCb,
  INT16            nOpcode
)
{
  INT32      nR   = 0;                                                          /* Current row                       */
  INT32      nC   = 0;                                                          /* Current column                    */
  INT32      nXRz = 0;                                                          /* Number of rows in result          */
  INT32      nXCz = 0;                                                          /* Number of columns in result       */
  COMPLEX64* z    =  NULL;                                                      /* Pointer to current element of Z   */

  /* Initialize */                                                              /* --------------------------------- */
  nXRz = MAX(nXRa,nXRb);                                                        /* Number of rows in result          */
  nXCz = MAX(nXCa,nXCb);                                                        /* Number of columns in result       */

  /* Validate */                                                                /* --------------------------------- */
  DLPASSERT(Z!=A && Z!=B);                                                      /* Result undefined!                 */
  DLPASSERT(dlp_size(Z)>=nXRz*nXCz*sizeof(COMPLEX64));                          /* Result buffer too small!          */

  /* z_ik = op(a_ik,b_ik) */                                                    /* --------------------------------- */
  for (z=Z,nC=0; nC<nXCz; nC++) {                                               /* Loop over columns                 */
    for (nR=0; nR<nXRz; nR++,z++) {                                             /*   Loop over rows                  */
      *z=dlp_scalopC(nR<nXRa&&nC<nXCa?                                          /*     |                             */
          _A(nR,nC):CMPLX(0.),                                   /*     |                             */
          nR<nXRb&&nC<nXCb?_B(nR,nC):CMPLX(0),nOpcode);          /*     |                             */
    }                                                                           /*     <<                            */
  }                                                                             /* <<                                */
  return O_K;                                                                   /* Ok                                */
}

/*
 * Elementwise vector-matrix/matrix-vector operation: (z_ik)=op(a_ik,b_i)
 */
INT16 CGEN_IGNORE dlm_vectop
(
  FLOAT64*       Z,
  const FLOAT64* A,
  INT32          nXR,
  INT32          nXC,
  const FLOAT64* B,
  INT16          nOpcode,
  INT16          bLeft,
  INT16          bInverse
)
{
  INT32          nR = 0;                                                        /* Current row                       */
  INT32          nC = 0;                                                        /* Current column                    */
  const FLOAT64* a  =  NULL;                                                    /* Pointer to current element of A   */
  const FLOAT64* b  =  NULL;                                                    /* Pointer to current element of B   */
  FLOAT64*       z  =  NULL;                                                    /* Pointer to current element of Z   */

  /* Validate */                                                                /* --------------------------------- */
  DLPASSERT(Z!=A && Z!=B);                                                      /* Result undefined!                 */
  DLPASSERT(dlp_size(Z)>=nXR*nXC*sizeof(FLOAT64));                              /* Result buffer too small!          */

  if(bInverse)
  {
    /* z_ik = op(a_ik,b_k) */                                                   /* --------------------------------- */
    for (z=Z,a=A,b=B,nC=0; nC<nXC; nC++,b++)                                    /* Loop over columns                 */
      for (nR=0; nR<nXR; nR++,z++,a++)                                          /*   Loop over rows                  */
        if (bLeft) *z = dlp_scalop(*b,*a,nOpcode);                              /*     vector/matrix operation       */
        else       *z = dlp_scalop(*a,*b,nOpcode);                              /*     matrix/vector operation       */
  }
  else
  {
  /* z_ik = op(a_ik,b_i) */                                                     /* --------------------------------- */
  for (z=Z,a=A,nC=0; nC<nXC; nC++)                                              /* Loop over columns                 */
    for (b=B,nR=0; nR<nXR; nR++,z++,a++,b++)                                    /*   Loop over rows                  */
      if (bLeft) *z = dlp_scalop(*b,*a,nOpcode);                                /*     vector/matrix operation       */
      else       *z = dlp_scalop(*a,*b,nOpcode);                                /*     matrix/vector operation       */
  }

  return O_K;                                                                   /* Ok                                */
}

/*
 * Complex variant of dlm_vectop
 */
INT16 CGEN_IGNORE dlm_vectopC
(
  COMPLEX64*       Z,
  const COMPLEX64* A,
  INT32            nXR,
  INT32            nXC,
  const COMPLEX64* B,
  INT16            nOpcode,
  INT16            bLeft,
  INT16            bInverse
)
{
  INT32            nR = 0;                                                      /* Current row                       */
  INT32            nC = 0;                                                      /* Current column                    */
  const COMPLEX64* a  =  NULL;                                                  /* Pointer to current element of A   */
  const COMPLEX64* b  =  NULL;                                                  /* Pointer to current element of B   */
  COMPLEX64*       z  =  NULL;                                                  /* Pointer to current element of Z   */

  /* Validate */                                                                /* --------------------------------- */
  DLPASSERT(Z!=A && Z!=B);                                                      /* Result undefined!                 */
  DLPASSERT(dlp_size(Z)>=nXR*nXC*sizeof(COMPLEX64))                             /* Result buffer too small!          */

  if(bInverse)
  {
    /* z_ik = op(a_ik,b_ik) */                                                  /* --------------------------------- */
    for (z=Z,a=A,b=B,nC=0; nC<nXC; nC++,b++) {                                  /* Loop over columns                 */
      for (nR=0; nR<nXR; nR++,z++,a++) {                                        /*   Loop over rows                  */
        if (bLeft) {
          *z = dlp_scalopC(*b,*a,nOpcode);                                      /*     vector/matrix operation       */
        } else {
          *z = dlp_scalopC(*a,*b,nOpcode);                                      /*     vector/matrix operation       */
        }
      }
    }
  }
  /* z_ik = op(a_ik,b_ik) */                                                    /* --------------------------------- */
  for (z=Z,a=A,nC=0; nC<nXC; nC++) {                                            /* Loop over columns                 */
    for (b=B,nR=0; nR<nXR; nR++,z++,a++,b++) {                                  /*   Loop over rows                  */
      if (bLeft) {
        *z = dlp_scalopC(*b,*a,nOpcode);                                        /*     vector/matrix operation       */
      } else {
        *z = dlp_scalopC(*a,*b,nOpcode);                                        /*     vector/matrix operation       */
      }
    }
  }
  return O_K;                                                                   /* Ok                                */
}

/**
 * Checks if a square matrix is diagonal.
 *
 * @param A
 *          Pointer to matrix to be checked
 * @param nXD
 *          Dimensions of matrix (number of rows and columns)
 * @return A non-zero value if the matrix is diagonal, 0 otherwise
 */
BOOL dlm_is_diag(const FLOAT64* A, INT32 nXD)
{
  const FLOAT64* a     = NULL;                                                  /* Pointer into matrix               */
  FLOAT64        nMaxD = 0.;                                                    /* Max. abs. diagonal value          */
  FLOAT64        nMaxM = 0.;                                                    /* Max. abs. off-diagonal value      */
  INT32          nR    = 0;                                                     /* Current row                       */
  INT32          nC    = 0;                                                     /* Current column                    */

  for (a=A,nC=0; nC<nXD; nC++)                                                  /* Loop over columns                 */
    for (nR=0; nR<nXD; nR++,a++)                                                /*   Loop over rows                  */
      if (nR==nC) nMaxD = MAX(nMaxD,fabs(*a));                                  /*     Get max. abs. diagonal value  */
      else        nMaxM = MAX(nMaxM,fabs(*a));                                  /*     Get max. abs. off-diag. value */

  if (nMaxD==0.) return nMaxM==0.;                                              /* Diag. is 0 -> off-diag. must be 0 */
  return nMaxM/nMaxD<F_TINY;                                                    /* Off-diagonal values negligible    */
}

/**
 * Complex variant of dlm_is_diag
 *
 */
BOOL dlm_is_diagC(const COMPLEX64* A, INT32 nXD)
{
  const COMPLEX64* a     = NULL;                                                /* Pointer into matrix               */
  FLOAT64          nMaxD = 0.;                                                  /* Max. abs. diagonal value          */
  FLOAT64          nMaxM = 0.;                                                  /* Max. abs. off-diagonal value      */
  INT32            nR    = 0;                                                   /* Current row                       */
  INT32            nC    = 0;                                                   /* Current column                    */

  for (a=A,nC=0; nC<nXD; nC++) {                                                /* Loop over columns                 */
    for (nR=0; nR<nXD; nR++,a++) {                                              /*   Loop over rows                  */
      if (nR==nC) nMaxD = MAX(nMaxD,CMPLX_ABS(*a));                             /*     Get max. abs. diagonal value  */
      else        nMaxM = MAX(nMaxM,CMPLX_ABS(*a));                             /*     Get max. abs. off-diag. value */
    }
  }
  if (nMaxD==0.) return nMaxM==0.;                                              /* Diag. is 0 -> off-diag. must be 0 */
  return nMaxM/nMaxD<F_TINY;                                                    /* Off-diagonal values negligible    */
}

/*
 * Diagonalized matrix operation performed on eigenvalues.
 */
INT16 CGEN_IGNORE dlm_diagop
(
  FLOAT64*       Z,
  const FLOAT64* A,
  INT32          nXD,
  FLOAT64        x,
  INT16          nOpcode,
  INT16          bLeft
)
{
  BOOL    bIsDiag = FALSE;                                                      /* Input-is-diagonal flag            */
  INT16   nErr    = O_K;                                                        /* Error state of eigen transform    */
  INT32    i       = 0;                                                         /* Universal loop counter            */
  FLOAT64* lpnEvc  = NULL;                                                      /* Eigenvector buffer                */
  FLOAT64* lpnEvl  = NULL;                                                      /* Eigenvalue buffer                 */

  /* Validate */                                                                /* --------------------------------- */
  DLPASSERT(Z!=A);                                                              /* Result undefined!                 */
  DLPASSERT(dlp_size(Z)>=nXD*nXD*sizeof(FLOAT64));                              /* Result buffer too small!          */

  lpnEvc = (FLOAT64*)dlp_calloc(nXD*nXD,sizeof(FLOAT64));                       /* Allocate eigenvector buffer       */
  lpnEvl = (FLOAT64*)dlp_malloc(nXD*nXD*sizeof(FLOAT64));                       /* Allocate eigenvalue buffer        */

  if (lpnEvc && lpnEvl)                                                         /* Buffers ok                        */
  {                                                                             /* >>                                */
    dlp_memmove(lpnEvl,A,nXD*nXD*sizeof(FLOAT64));                              /*   Copy input matrix               */
    bIsDiag = dlm_is_diag(A,nXD);                                               /*   Is input matrix diagonal?       */
    if (!bIsDiag) nErr = dlm_eigen_jac(lpnEvl,lpnEvc,nXD,FALSE);                /*   Eigenvector transform           */
    if (bIsDiag || OK(nErr))                                                    /*   Everything ok                   */
    {                                                                           /*   >>                              */
      for (i=0; i<nXD; i++)                                                     /*     Loop over eigenvalues         */
        if (bLeft) lpnEvl[i*nXD+i] = dlp_scalop(x,lpnEvl[i*nXD+i],nOpcode);     /*       "Left" : evl_i = op(x,evl_i)*/
        else       lpnEvl[i*nXD+i] = dlp_scalop(lpnEvl[i*nXD+i],x,nOpcode);     /*       "Right": evl_i = op(evl_i,x)*/
      if (!bIsDiag) dlm_ieigen(Z,lpnEvl,lpnEvc,nXD,FALSE);                      /*     Inverse eigenvector transform */
    }                                                                           /*   <<                              */
    else _DMS(Z,nXD,0.);                                                        /*   Evec. trafo not ok -> reset dst.*/
  }                                                                             /* <<                                */

  dlp_free(lpnEvc);                                                             /* Free eigenvector buffer           */
  dlp_free(lpnEvl);                                                             /* Free eigenvalue buffer            */
  return O_K;                                                                   /* Ok                                */
}

/*
 * Make diagonal matrix
 */
INT16 CGEN_IGNORE dlm_diag(FLOAT64* Z, const FLOAT64* A, INT32 nXR, INT32 nXC)
{
  INT32 nR  = 0;
  INT32 nC  = 0;
  INT32 nXD = nXR;

  DLPASSERT(nXR==nXC || nXC==1);
  for (nC=0; nC<nXD; nC++)
    for (nR=0; nR<nXD; nR++)
      Z[nR*nXD+nC] = nR==nC ? A[nXC==1?nR:nR*nXD+nC] : 0.;

  return O_K;
}

/*
 * Matrix multiplication: Z=A*B
 *
 * REMARK: Faster multiplication for sparse matrices
 *
 */
INT16 CGEN_IGNORE dlm_mult_sparse
(
  FLOAT64*       Z,                                /* Pointer to first element */
  const FLOAT64* A,                                /* Matrices are saved transposed! */
  INT32          nXRa,                                /* R = column (= spalte), since transposed */
  INT32          nXCa,                                /* C = row (=zeile), since transposed */
  const FLOAT64* B,
  INT32          nXRb,
  INT32          nXCb
)
{
  const FLOAT64* a    = NULL;
  const FLOAT64* b    = NULL;
  FLOAT64*       z    = NULL;
  const FLOAT64* a0   = NULL;
  const FLOAT64* b0   = NULL;
  INT32          i    = 0;
  INT32       nElem = 0;                            /* Number of elements to compute */
  INT32          nR   = 0;                              /* Counter columns */
  INT32          nC   = 0;                              /* Counter Rows */
  INT32        bStart = 0;                            /* "boolean": reached first elem. !=0 ? */
  INT32        bEnd   = 0;                            /* "boolean": reached a zero? */
  INT32*      nStartA;                              /* first non-zero element in every row */
  INT32*      nStartB;                              /* first non-zero element in every column */
  INT32*      nEndA;                              /* previous elem. was last non-zero (in each row) */
  INT32*      nEndB;                              /* previous elem. was last non-zero (in each column) */
  INT32*       nS;                                    /* start to compute */
  INT32*      nE;                                 /* end to compute */

  /* Validate */                                                                /* --------------------------------- */
  DLPASSERT(nXCa==nXRb);                                                        /* Dimension error!                  */
  DLPASSERT(Z!=A && Z!=B);                                                      /* Result undefined!                 */
  DLPASSERT(dlp_size(Z)>=nXRa*nXCb*sizeof(FLOAT64));                             /* Result buffer too small!          */

  nStartA = (INT32*)dlp_calloc(nXRa+nXCb+nXRa+nXCb+nXRa*nXCb+nXRa*nXCb, sizeof(INT32));
  nStartB = nStartA + nXRa;
  nEndA   = nStartB + nXCb;
  nEndB   = nEndA   + nXRa;
  nS      = nEndB   + nXCb;
  nE      = nS      + nXRa * nXCb;

  /* Search for blocks != 0 in A */
  for (nR=0; nR<nXRa; nR++) {                            /* hold row and search in columns */
    bStart=0;
    bEnd=0;
    for (nC=0; nC<nXCa; nC++)
    {
      if ( (*(A+nC*nXRa+nR)!=0) && (bStart==0) )                /* columns are saved as rows! */
      {
        nStartA[nR]=nC;                              /* restore number of column */
        bStart=1;
      }
      if (nC==nXCa-1)
        if ( (*(A+nC*nXRa+nR)!=0) && (bStart==1) ) nEndA[nR]=nC+1;
      if ( (*(A+nC*nXRa+nR)==0) && (bStart==1) )
      {
        if (bEnd==0)
        {
          bEnd=1;
          nEndA[nR]=nC;                            /* restore number of column */
        }
      }
      if ( (*(A+nC*nXRa+nR)!=0) && (bEnd==1) ) bEnd=0;
    }
  }
  /* Search for blocks != 0 in B (not the same as for A!)*/
  for (nC=0; nC<nXCb; nC++)                          /* hold column and search in rows */
  {
    bStart=0;
    bEnd=0;
    for (nR=0; nR<nXRb; nR++)
    {
      if ( (*(B+nC*nXRb+nR)!=0) && (bStart==0) )
      {
        nStartB[nC]=nR;                              /* restore number of row */
        bStart=1;
      }
      if (nR==nXRb-1)
        if ( (*(B+nC*nXRb+nR)!=0) && (bStart==1) ) nEndB[nC]=nR+1;
      if ( (*(B+nC*nXRb+nR)==0) && (bStart==1) )
      {
        if (bEnd==0)
        {
          bEnd=1;
          nEndB[nC]=nR;
        }
      }
      if ( (*(B+nC*nXRb+nR)!=0) && (bEnd==1) ) bEnd=0;
    }
  }
  /* Compare starts and ends -> nXCa==nXRb */
  for (nR=0; nR<nXRa; nR++)                            /* hold "column" (row for real) and compare each "row" */
    for (nC=0; nC<nXCb; nC++)
    {
      if (nStartA[nR] >= nStartB[nC]) nS[nR*nXCb+nC]=nStartA[nR];
      else nS[nR*nXCb+nC]=nStartB[nC];
      if (nEndA[nR] <= nEndB[nC]) nE[nR*nXCb+nC]=nEndA[nR];
      else nE[nR*nXCb+nC]=nEndB[nC];
    }

  /* Do matrix multiplication */                                                /* --------------------------------- */
  z=Z;
  b0=B;
  for (nC=0; nC<nXCb; nC++)    /* over all rows of B */
  {
    a0=A;
    for (nR=0; nR<nXRa; nR++)            /* over all columns of A */
    {
      *z=0.;
      a=a0+nS[nR*nXCb+nC]*nXRa;
      b=b0+nS[nR*nXCb+nC];
      nElem=nE[nR*nXCb+nC]-nS[nR*nXCb+nC];
      for (i=0; i<nElem; i++)              /* over all columns of B and rows of A */
      {
        *z+=*a**b;
        a+=nXRa;
        b++;
      }
      z++;
      a0++;
    }
    b0+=nXRb;
  }

  dlp_free(nStartA);
  return O_K;                                                                   /* Jo!                               */
}

/*
 * Complex variant of dlm_mult_sparse
 */
INT16 CGEN_IGNORE dlm_mult_sparseC
(
  COMPLEX64*       Z,                                /* Pointer to first element */
  const COMPLEX64* A,                                /* Matrices are saved transposed! */
  INT32            nXRa,                             /* R = column (= spalte), since transposed */
  INT32            nXCa,                             /* C = row (=zeile), since transposed */
  const COMPLEX64* B,
  INT32            nXRb,
  INT32            nXCb
)
{
  const COMPLEX64* a    = NULL;
  const COMPLEX64* b    = NULL;
  COMPLEX64*       z    = NULL;
  const COMPLEX64* a0   = NULL;
  const COMPLEX64* b0   = NULL;
  INT32            i    = 0;
  INT32            nElem = 0;                        /* Number of elements to compute */
  INT32            nR   = 0;                         /* Counter columns */
  INT32            nC   = 0;                         /* Counter Rows */
  INT32            bStart = 0;                       /* "boolean": reached first elem. !=0 ? */
  INT32            bEnd   = 0;                       /* "boolean": reached a zero? */
  INT32*           nStartA;                          /* first non-zero element in every row */
  INT32*           nStartB;                          /* first non-zero element in every column */
  INT32*           nEndA;                            /* previous elem. was last non-zero (in each row) */
  INT32*           nEndB;                            /* previous elem. was last non-zero (in each column) */
  INT32*           nS;                               /* start to compute */
  INT32*           nE;                               /* end to compute */

  /* Validate */                                                                /* --------------------------------- */
  DLPASSERT(nXCa==nXRb);                                                        /* Dimension error!                  */
  DLPASSERT(Z!=A && Z!=B);                                                      /* Result undefined!                 */
  DLPASSERT(dlp_size(Z)>=nXRa*nXCb*sizeof(COMPLEX64));                             /* Result buffer too small!          */

  nStartA = (INT32*)dlp_calloc(nXRa+nXCb+nXRa+nXCb+nXRa*nXCb+nXRa*nXCb, sizeof(INT32));
  nStartB = nStartA + nXRa;
  nEndA   = nStartB + nXCb;
  nEndB   = nEndA   + nXRa;
  nS      = nEndB   + nXCb;
  nE      = nS      + nXRa * nXCb;

  /* Search for blocks != 0 in A */
  for (nR=0; nR<nXRa; nR++) {                            /* hold row and search in columns */
    bStart=0;
    bEnd=0;
    for (nC=0; nC<nXCa; nC++)
    {
      if ( !CMPLX_EQUAL(*(A+nC*nXRa+nR),CMPLX(0)) && (bStart==0) )                /* columns are saved as rows! */
      {
        nStartA[nR]=nC;                              /* restore number of column */
        bStart=1;
      }
      if (nC==nXCa-1)
        if ( !CMPLX_EQUAL(*(A+nC*nXRa+nR),CMPLX(0)) && (bStart==1) ) nEndA[nR]=nC+1;
      if ( CMPLX_EQUAL(*(A+nC*nXRa+nR),CMPLX(0)) && (bStart==1) )
      {
        if (bEnd==0)
        {
          bEnd=1;
          nEndA[nR]=nC;                            /* restore number of column */
        }
      }
      if ( !CMPLX_EQUAL(*(A+nC*nXRa+nR),CMPLX(0)) && (bEnd==1) ) bEnd=0;
    }
  }
  /* Search for blocks != 0 in B (not the same as for A!)*/
  for (nC=0; nC<nXCb; nC++)                          /* hold column and search in rows */
  {
    bStart=0;
    bEnd=0;
    for (nR=0; nR<nXRb; nR++)
    {
      if ( !CMPLX_EQUAL(*(B+nC*nXRb+nR),CMPLX(0)) && (bStart==0) )
      {
        nStartB[nC]=nR;                              /* restore number of row */
        bStart=1;
      }
      if (nR==nXRb-1)
        if ( !CMPLX_EQUAL(*(B+nC*nXRb+nR),CMPLX(0)) && (bStart==1) ) nEndB[nC]=nR+1;
      if ( CMPLX_EQUAL(*(B+nC*nXRb+nR),CMPLX(0)) && (bStart==1) )
      {
        if (bEnd==0)
        {
          bEnd=1;
          nEndB[nC]=nR;
        }
      }
      if ( !CMPLX_EQUAL(*(B+nC*nXRb+nR),CMPLX(0)) && (bEnd==1) ) bEnd=0;
    }
  }
  /* Compare starts and ends -> nXCa==nXRb */
  for (nR=0; nR<nXRa; nR++)                            /* hold "column" (row for real) and compare each "row" */
    for (nC=0; nC<nXCb; nC++)
    {
      if (nStartA[nR] >= nStartB[nC]) nS[nR*nXCb+nC]=nStartA[nR];
      else nS[nR*nXCb+nC]=nStartB[nC];
      if (nEndA[nR] <= nEndB[nC]) nE[nR*nXCb+nC]=nEndA[nR];
      else nE[nR*nXCb+nC]=nEndB[nC];
    }

  /* Do matrix multiplication */                                                /* --------------------------------- */
  z=Z;
  b0=B;
  for (nC=0; nC<nXCb; nC++)    /* over all rows of B */
  {
    a0=A;
    for (nR=0; nR<nXRa; nR++)            /* over all columns of A */
    {
      *z=CMPLX(0.);
      a=a0+nS[nR*nXCb+nC]*nXRa;
      b=b0+nS[nR*nXCb+nC];
      nElem=nE[nR*nXCb+nC]-nS[nR*nXCb+nC];
      for (i=0; i<nElem; i++)              /* over all columns of B and rows of A */
      {
        *z=CMPLX_PLUS(*z,CMPLX_MULT(*a,*b));
        a+=nXRa;
        b++;
      }
      z++;
      a0++;
    }
    b0+=nXRb;
  }

  dlp_free(nStartA);
  return O_K;                                                                   /* Jo!                               */
}

/*
 * Kronecker product: Z=a_ik*B
 */
INT16 CGEN_IGNORE dlm_mult_kron
(
  FLOAT64*       Z,
  const FLOAT64* A,
  INT32          nXRa,
  INT32          nXCa,
  const FLOAT64* B,
  INT32          nXRb,
  INT32          nXCb
)
{
  const FLOAT64* a   = NULL;
  const FLOAT64* b   = NULL;
  FLOAT64*       z   = NULL;
  FLOAT64*       z0  = NULL;
  INT32          nRa = 0;
  INT32          nRb = 0;
  INT32          nCa = 0;
  INT32          nCb = 0;

  /* Validate */                                                                /* --------------------------------- */
  DLPASSERT(Z!=A && Z!=B);                                                      /* Result undefined!                 */
  DLPASSERT(dlp_size(A)>=nXRa*nXCa*sizeof(FLOAT64));                             /* Left operand buffer too small!    */
  DLPASSERT(dlp_size(B)>=nXRb*nXCb*sizeof(FLOAT64));                             /* Left operand buffer too small!    */
  DLPASSERT(dlp_size(Z)>=nXRa*nXRb*nXCa*nXCb*sizeof(FLOAT64));                   /* Result buffer too small!          */

  /* Compute Kronecker product */                                               /* --------------------------------- */
  for (a=A,nCa=0; nCa<nXCa; nCa++)                                              /* Loop over columns of A            */
    for (nRa=0; nRa<nXRa; nRa++,a++)                                            /*   Loop over rows of A             */
      for (b=B,z0=&Z[nCa*nXCb*nXRa*nXRb+nRa*nXRb],nCb=0; nCb<nXCb; nCb++)       /*     Loop over columns of B        */
        for(nRb=0,z=z0+nXRa*nXRb*nCb; nRb<nXRb; nRb++,b++,z++)                  /*       Loop over rows of B         */
          *z=*a**b;                                                             /*         Multiply elements         */

  return O_K;                                                                   /* All done                          */
}

/*
 * Complex version of dlm_mult_kron
 */
INT16 CGEN_IGNORE dlm_mult_kronC
(
  COMPLEX64*          Z,
  const COMPLEX64*    A,
  INT32               nXRa,
  INT32               nXCa,
  const COMPLEX64*    B,
  INT32               nXRb,
  INT32               nXCb
)
{
  const COMPLEX64* a   = NULL;
  const COMPLEX64* b   = NULL;
  COMPLEX64*       z   = NULL;
  COMPLEX64*       z0  = NULL;
  INT32            nRa = 0;
  INT32            nRb = 0;
  INT32            nCa = 0;
  INT32            nCb = 0;

  /* Validate */                                                                /* --------------------------------- */
  DLPASSERT(Z!=A && Z!=B);                                                      /* Result undefined!                 */
  DLPASSERT(dlp_size(A)>=nXRa*nXCa*sizeof(COMPLEX64));                          /* Left operand buffer too small!    */
  DLPASSERT(dlp_size(B)>=nXRb*nXCb*sizeof(COMPLEX64));                          /* Left operand buffer too small!    */
  DLPASSERT(dlp_size(Z)>=nXRa*nXRb*nXCa*nXCb*sizeof(COMPLEX64));                /* Result buffer too small!          */

  /* Compute Kronecker product */                                               /* --------------------------------- */
  for (a=A,nCa=0; nCa<nXCa; nCa++)                                              /* Loop over columns of A            */
    for (nRa=0; nRa<nXRa; nRa++,a++)                                            /*   Loop over rows of A             */
      for (b=B,z0=Z+nCa*nXCb*nXRa*nXRb+nRa*nXRb,nCb=0; nCb<nXCb; nCb++)         /*  Loop over columns of B        */
        for(nRb=0,z=z0+nXRa*nXRb*nCb; nRb<nXRb; nRb++,b++,z++)                  /*       Loop over rows of B         */
          *z=dlp_scalopC(*a,*b,OP_MULT);    /*         |                         */

  return O_K;                                                                   /* All done                          */
}

/*
 * Matrix multiplication: Z=A*K*A'
 */
INT16 CGEN_IGNORE dlm_mult_akat
(
  FLOAT64* Z,
  const FLOAT64* A,
  INT32 nXR,
  INT32 nXC,
  const FLOAT64* K
)
{
  INT32   nC1;
  INT32   nC2;
  INT32   nR1;
  INT32   nR2;
  FLOAT64 nSum;

  for (nC1=0; nC1<nXC; nC1++)
    for (nC2=0; nC2<nXC; nC2++)
    {
      for (nSum=0.,nR1=0; nR1<nXR; nR1++)
        for (nR2=0; nR2 < nXR; nR2++)
          nSum += A[nC1*nXR+nR2]*K[nR2*nXR+nR1]*A[nC2*nXR+nR1];
      Z[nC1*nXC+nC2] = nSum;
    }

  return O_K;
}

/*
 * Complex variant of dlm_mult_akat
 */
INT16 CGEN_IGNORE dlm_mult_akatC
(
  COMPLEX64*       Z,
  const COMPLEX64* A,
  INT32            nXR,
  INT32            nXC,
  const COMPLEX64* K
)
{
  INT32     nC1;
  INT32     nC2;
  INT32     nR1;
  INT32     nR2;
  COMPLEX64 nSumC;

  for (nC1=0; nC1<nXC; nC1++) {
    for (nC2=0; nC2<nXC; nC2++) {
      for (nSumC=CMPLX(0.),nR1=0; nR1<nXR; nR1++) {
        for (nR2=0; nR2 < nXR; nR2++) {
          nSumC = dlp_scalopC(nSumC,
              dlp_scalopC(A[nC1*nXR+nR2],
                  dlp_scalopC((K)[nR2*nXR+nR1],A[nC2*nXR+nR1],OP_MULT),OP_MULT),OP_ADD);
        }
        Z[nC1*nXC+nC2] = nSumC;
      }
    }
  }

  return O_K;
}

/*
 * Positive interger power of square matrix.
 */
INT16 CGEN_IGNORE dlm_intpower
(
  FLOAT64*       Z,
  const FLOAT64* A,
  INT32          nXD,
  UINT32         nPower
)
{
  FLOAT64* lpnBuf = NULL;                                                        /* Multiplication buffer             */

  DLPASSERT(Z!=A);                                                              /* Result undefined!                 */
  DLPASSERT(dlp_size(A)>=nXD*nXD*sizeof(FLOAT64));                               /* Result buffer too small!          */
  DLPASSERT(dlp_size(Z)>=nXD*nXD*sizeof(FLOAT64));                               /* Result buffer too small!          */

  if (nPower==0)                                                                /* A^0 = E                           */
  {                                                                             /* >>                                */
    dlm_constant(Z,nXD,nXD,OP_UNITMAT);                                         /*   Create unit matrix              */
    return O_K;                                                                 /*   Ok                              */
  }                                                                             /* <<                                */

  lpnBuf = (FLOAT64*)dlp_calloc(nXD*nXD,sizeof(FLOAT64));                         /* Create multiplication buffer      */
  dlp_memmove(Z,A,nXD*nXD*sizeof(FLOAT64));                                      /* Copy A -> Z (A^1)                 */
  for (;nPower>1; nPower--)                                                     /* nPower > 1                        */
  {                                                                             /* >>                                */
    dlp_memmove(lpnBuf,Z,nXD*nXD*sizeof(FLOAT64));                               /*   Move prev. result to buffer     */
    dlm_mult(Z,lpnBuf,nXD,nXD,A,nXD,nXD);                                       /*   Z = lpnBuf * A                  */
  }                                                                             /* <<                                */
  dlp_free(lpnBuf);                                                             /* Destroy multiplication buffer     */

  return O_K;                                                                   /* Ok                                */
}

/* ---------------------------------------------------------------------------*/
/*
 * Complex variant of dlm_intpower
 */
INT16 CGEN_IGNORE dlm_intpowerC
(
  COMPLEX64*       Z,
  const COMPLEX64* A,
  INT32            nXD,
  UINT32           nPower
)
{
  COMPLEX64* lpnBuf = NULL;                                                     /* Multiplication buffer             */

  DLPASSERT(Z!=A);                                                              /* Result undefined!                 */
  DLPASSERT(dlp_size(A)>=nXD*nXD*sizeof(COMPLEX64));                            /*   |                               */
  DLPASSERT(dlp_size(Z)>=nXD*nXD*sizeof(COMPLEX64));                            /*   |                               */

  if (nPower==0)                                                                /* A^0 = E                           */
  {                                                                             /* >>                                */
    dlm_constantC(Z,nXD,nXD,OP_UNITMAT);                                        /*   Create unit matrix              */
    return O_K;                                                                 /*   Ok                              */
  }                                                                             /* <<                                */

  lpnBuf = dlp_calloc(nXD*nXD,sizeof(COMPLEX64));                               /* Create multiplication buffer      */
  dlp_memmove(Z,A,nXD*nXD*sizeof(COMPLEX64));                                   /* Copy A -> Z (A^1)                 */
  for (;nPower>1; nPower--)                                                     /* nPower > 1                        */
  {                                                                             /* >>                                */
    dlp_memmove(lpnBuf,Z,nXD*nXD*sizeof(COMPLEX64));                            /*   Move prev. result to buffer     */
    dlm_multC(Z,lpnBuf,nXD,nXD,A,nXD,nXD);                                      /*   Z = lpnBuf * A                  */
  }                                                                             /* <<                                */
  dlp_free(lpnBuf);                                                             /* Destroy multiplication buffer     */

  return O_K;                                                                   /* Ok                                */
}

/* Library scope functions                                                    */

const opcode_table* dlm_matrop_entry(INT16 nEntry) {
  return &__mtab[nEntry];
}

/**
 * Prints a table of matrix operations at stdout.
 */
void dlm_matrop_printtab()
{
  INT32 i = -1;
  char sBuf[255];

  printf ("\n   Table of matrix operators");
  for (i=0; __mtab[i].opc!=-1; i++)
  {
    printf("\n   %2hd: %5hd  ",(short)i,(short)__mtab[i].opc);
    if (strlen(__mtab[i].sym) > 0)
    {
      /* TODO: print signature! */
      sprintf(sBuf,"%s",__mtab[i].sym);
      printf("%-13s", sBuf);
    }
    else printf("             ");
    printf("  %s",__mtab[i].nam);
  }
  printf("\n   Scalar operations also apply (elementwise)!");
}

/**
 * Get matrix operation code from symbol.
 *
 * @param lpsOpname
 *          The operation name of the requested operation.
 * @param lpnOps
 *          Pointer to a buffer to be filled with the number of operands (may be
 *          <code>NULL</code>)
 * @return The operation code or -1 if no such operation exisis.
 */
INT16 dlm_matrop_code(const char* lpsOpname, INT16* lpnOps)
{
  INT32 i = 0;
  if (lpnOps) *lpnOps=-1;
  if (!dlp_strlen(lpsOpname)) return -1;
  for (i=0; __mtab[i].opc!=-1; i++)
    if (0==strcmp(lpsOpname,__mtab[i].sym))
    {
      if (lpnOps) *lpnOps = __mtab[i].ops;
      return __mtab[i].opc;
    }
  return -1;
}

/**
 * Get matrix operation name for operation code.
 *
 * @param nOpcode
 *          The operation code
 * @return Pointer to a constant string containing the operation name.
 */
const char* dlm_matrop_name(INT16 nOpcode)
{
  INT32 i = 0;
  if (nOpcode<0) return "?";
  for (i=0; __mtab[i].opc!=-1; i++)
    if (__mtab[i].opc==nOpcode)
      return __mtab[i].sym;
  return "?";
}

/**
 * Get signature of matrix operation for operation code.
 *
 * @param nOpcode
 *          The operation code
 * @return Pointer to a constant string containing the signature.
 */
const char* dlm_matrop_signature(INT16 nOpcode)
{
  INT32 i = 0;
  if (nOpcode<0) return NULL;
  for (i=0; __mtab[i].opc!=-1; i++)
    if (__mtab[i].opc==nOpcode)
      return __mtab[i].sig;
  return NULL;
}

/**
 * <p>Performs matrix operations with two arguments.</p>
 *
 * @param Z
 *          Pointer to result matrix (may be <code>NULL</code> in order to just
 *          determine the dimensions of the result matrix without doing further
 *          computations)
 * @param lpnXRz
 *          Pointer to a long value to be filled with number of rows of the
 *          result (may be <code>NULL</code>)
 * @param lpnXCz
 *          Pointer to a long value to be filled with number of columns of the
 *          result (may be <code>NULL</code>)
 * @param A
 *          Pointer to operand matrix 1 (may be identical with <code>Z</code>,
 *          may be <code>NULL</code> for returning matrix constants or
 *          determining result dimensions without further computation)
 * @param nXRa
 *          Number of rows in operand matrix 1
 * @param nXCa
 *          Number of columns in operand matrix 1
 * @param B
 *          Pointer to operand matrix 1 (may be identical with <code>Z</code>,
 *          may be <code>NULL</code> for unary operations, matrix constants or
 *          determining result dimensions without further computation)
 * @param nXRb
 *          Number of rows in operand matrix 2
 * @param nXCb
 *          Number of columns in operand matrix 2
 * @param nOpcode
 *          Operation code
 * @return <code>O_K</code> if sucessfull, a (negative) error code otherwise
 * @see dlm_matrop_code
 */
INT16 dlm_matrop
(
  FLOAT64*       Z,
  INT32*         lpnXRz,
  INT32*         lpnXCz,
  const FLOAT64* A,
  INT32          nXRa,
  INT32          nXCa,
  const FLOAT64* B,
  INT32          nXRb,
  INT32          nXCb,
  INT16         nOpcode
)
{
  BOOL   bElemws = TRUE;                                                        /* Elementwise (or scalar) computn.  */
  INT16  nErr    = O_K;                                                         /* Be optimistic! :)                 */
  BOOL   bZisA   = FALSE;                                                       /* Z==A flag                         */
  BOOL   bZisB   = FALSE;                                                       /* Z==B flag                         */
  FLOAT64 nBuf    = 0.;                                                          /* Double buffer                    */
  INT32   nXDa    = 0;                                                           /* "Dimension" of A                 */
  INT32   nXDb    = 0;                                                           /* "Dimension" of B                 */

  /* Validate */                                                                /* --------------------------------- */
  if (A&&(nXCa>1||nXRa>1)&&dlp_size(A)<nXCa*nXRa*sizeof(FLOAT64)) return ERR_MEM;/* Wrong dimensions for matrix A    */
  if (B&&(nXCb>1||nXRb>1)&&dlp_size(B)<nXCb*nXRb*sizeof(FLOAT64)) return ERR_MEM;/* Wrong dimensions for matrix B    */

  /* Initialize */                                                              /* --------------------------------- */
  if (Z && Z==A)                                                                /* Result identical with A           */
  {                                                                             /* >>                                */
    bZisA = TRUE;                                                               /*   Remember that                   */
    Z = (FLOAT64*)dlp_calloc(dlp_size(A)/sizeof(FLOAT64),sizeof(FLOAT64));         /*   Create tmp. array of equal size */
  }                                                                             /* <<                                */
  else if (Z && Z==B)                                                           /* Result not A but identical with B */
  {                                                                             /* >>                                */
    bZisB = TRUE;                                                               /*   Remember that                   */
    Z = (FLOAT64*)dlp_calloc(dlp_size(B)/sizeof(FLOAT64),sizeof(FLOAT64));         /*   Create tmp. array of equal size */
  }                                                                             /* <<                                */

  switch (nOpcode)                                                              /* Branch for opcode                 */
  {                                                                             /* >>                                */
  /* Dedicated matrix operations */                                             /* --------------------------------- */
  case OP_REAL: /* FALL THROUGH */                                              /* Real part                         */
  case OP_IMAG: /* FALL THROUGH */                                              /* Imaginary part                    */
  case OP_CONJ: /* FALL THROUGH */                                              /* Complex conjugate                 */
  case OP_ABS:  break;                                                          /* Absolute values                   */
  case OP_TRANSPOSE:                                                            /* Transpose:                        */
    if (Z && A) nErr    = dlm_transpose(Z,A,nXRa,nXCa);                         /*   Call computation function       */
    if (lpnXRz) *lpnXRz = nXCa;                                                 /*   Store no. of rows of result     */
    if (lpnXCz) *lpnXCz = nXRa;                                                 /*   Store no. of columns of result  */
    bElemws = FALSE; break;                                                     /*   == (no elementwise computation) */
  case OP_INVT:                                                                 /* Inversion:                        */
    bElemws = FALSE;                                                            /*   No elementwise computation!     */
    if (nXRa!=nXCa) { nErr = ERR_MSQR; break; }                                 /*   == A must be a square matrix    */
    if (Z && A)                                                                 /*   Have input and output matrices  */
    {                                                                           /*   >>                              */
      dlp_memmove(Z,A,nXRa*nXRa*sizeof(FLOAT64));                                /*     Copy input to output         */
      nErr = dlm_invert_gel(Z,nXRa,&nBuf);                                      /*     Invert through Gaussian elim. */
      IF_NOK(nErr)  { _DMS(Z,nXRa*nXRa,0.); break; }                            /*     == Failed -> clear result     */
      if (nBuf==0.) { _DMS(Z,nXRa*nXRa,0.); nErr=ERR_MSGL; break; }             /*     == A singular -> clear result */
    }                                                                           /*   <<                              */
    if (lpnXRz) *lpnXRz = nXRa;                                                 /*   Store no. of rows of result     */
    if (lpnXCz) *lpnXCz = nXRa;                                                 /*   Store no. of columns of result  */
    break;                                                                      /*   ==                              */
  case OP_CHOLF:                                                                /* Cholesky factorization:           */
    bElemws = FALSE;                                                            /*   No elementwise computation!     */
    if (nXRa!=nXCa) { nErr = ERR_MSQR; break; }                                 /*   == A must be a square matrix    */
    /* TODO: Check A is symmetric! */                                           /*                                   */
    if (Z && A)                                                                 /*   Have input and output matrices  */
    {                                                                           /*   >>                              */
      nErr = dlm_cholf(Z,A,nXRa);                                               /*     Do Cholesky factorization     */
      IF_NOK(nErr)  { _DMS(Z,nXRa*nXRa,0.); break; }                            /*     == Failed -> clear result     */
    }                                                                           /*   <<                              */
    if (lpnXRz) *lpnXRz = nXRa;                                                 /*   Store no. of rows of result     */
    if (lpnXCz) *lpnXCz = nXRa;                                                 /*   Store no. of columns of result  */
    break;                                                                      /*   ==                              */
  case OP_SOLV_LU:                                                              /* Solve Ax=B due LU decomposition   */
    bElemws = FALSE;                                                            /*   No elementwise computation!     */
    if (nXRa!=nXCa) { nErr = ERR_MSQR; break; }                                 /*   == A must be a square matrix    */
    if (nXRa!=nXRb) { nErr = ERR_MDIM; break; }                                 /*   == Dimension must fit           */
    if (Z && A)                                                                 /*   Have input and output matrices  */
    {                                                                           /*   >>                              */
      dlp_memmove(Z,B,nXRa*nXCb*sizeof(FLOAT64));                               /*     Adaption to worker function   */
      nErr = dlm_solve_lud((FLOAT64*)A,nXRa,Z,nXCb);                            /*     Do Cholesky factorization     */
      IF_NOK(nErr)  { _DMS(Z,nXRa*nXRa,0.); break; }                            /*     == Failed -> clear result     */
    }                                                                           /*   <<                              */
    if (lpnXRz) *lpnXRz = nXRa;                                                 /*   Store no. of rows of result     */
    if (lpnXCz) *lpnXCz = nXCb;                                                 /*   Store no. of columns of result  */
    break;                                                                      /*   ==                              */
  case OP_TRACE: /* FALL THROUGH */                                             /* Trace:                            */
  case OP_MDIAG:                                                                /* Main diagonal:                    */
    bElemws = FALSE;                                                            /*   No elementwise computation!     */
    if (nXRa!=nXCa) { nErr = ERR_MSQR; break; }                                 /*   Only for square matrices        */
    if (Z && A)                                                                 /*   Have input and output matrices  */
    {                                                                           /*   >>                              */
      INT32 i = 0;                                                               /*     Loop counter                 */
      for (i=0; i<nXRa; i++)                                                    /*     Loop over matrix dimension    */
        if (nOpcode==OP_MDIAG) Z[i] =A[i*nXRa+i];                               /*       Extract main diagonal       */
        else                   Z[0]+=A[i*nXRa+i];                               /*       Compute trace               */
    }                                                                           /*   <<                              */
    if (lpnXRz) *lpnXRz = nOpcode==OP_MDIAG ? nXRa : 1;                         /*   Store no. of rows of result     */
    if (lpnXCz) *lpnXCz = 1;                                                    /*   Store no. of columns of result  */
    break;                                                                      /*   ==                              */
  case OP_DIAG:                                                                 /* Make diagonal matrix              */
    bElemws = FALSE;                                                            /*   No elementwise computation!     */
    if (nXCa!=1 && nXCa!=nXRa) { nErr = ERR_MDIM; break; }                      /*   A must be a vector              */
    if (Z && A) dlm_diag(Z,A,nXRa,nXCa);                                        /*   Have input and output matrices  */
    if (lpnXRz) *lpnXRz = nXRa;                                                 /*   Store no. of rows of result     */
    if (lpnXCz) *lpnXCz = nXRa;                                                 /*   Store no. of columns of result  */
    break;
  case OP_RANK: /* FALL THROUGH */                                              /* Rank:                             */
  case OP_DET:                                                                  /* Determinant:                      */
    bElemws = FALSE;                                                            /*   No elementwise computation!     */
    if (nXRa!=nXCa) { nErr = ERR_MSQR; break; }                                 /*   == A must be a square matrix    */
    if (Z && A)                                                                 /*   Have input and output matrices  */
    {                                                                           /*   >>                              */
      FLOAT64* lpnBuf = (FLOAT64*)dlp_calloc(nXRa*nXRa,sizeof(FLOAT64));        /*     Aux. buffer for inversion     */
      dlp_memmove(lpnBuf,A,nXRa*nXRa*sizeof(FLOAT64));                          /*     Copy matrix A into buffer     */
      if (nOpcode==OP_DET) nErr = dlm_det_lud(lpnBuf,nXRa,Z);                   /*     Calculate                     */
      dlp_free(lpnBuf);                                                         /*     Free auxilary buffer          */
    }                                                                           /*   <<                              */
    if (lpnXRz) *lpnXRz = 1;                                                    /*   Store no. of rows of result     */
    if (lpnXCz) *lpnXCz = 1;                                                    /*   Store no. of columns of result  */
    break;                                                                      /*   ==                              */
  case OP_MULT:                                                                 /* Multiplication:                   */
    if (nXCa==nXRb)                                                             /*   Matrix product                  */
    {                                                                           /*   >>                              */
      if (Z && A && B) nErr = dlm_mult(Z,A,nXRa,nXCa,B,nXRb,nXCb);              /*     Call computation function     */
      if (lpnXRz) *lpnXRz = nXRa;                                               /*     Store no. of rows of result   */
      if (lpnXCz) *lpnXCz = nXCb;                                               /*     Store no. of columns of result*/
      bElemws = FALSE;                                                          /*     No elementwise computation!   */
    }                                                                           /*   <<                              */
    else if ((nXRa>1 || nXCa>1) && (nXRb>1 || nXCb>1))                          /*   Neither matrix nor scalar prod. */
    {                                                                           /*   >>                              */
      nErr = ERR_MDIM;                                                          /*     Could not do anything         */
      if (lpnXRz) *lpnXRz = 0;                                                  /*     Store no. of rows of result   */
      if (lpnXCz) *lpnXCz = 0;                                                  /*     Store no. of columns of result*/
      bElemws = FALSE;                                                          /*     No elementwise computation!   */
    }                                                                           /*   <<                              */
    break;                                                                      /*   == (do elementwise computation) */
  case OP_MULT_SPARSE:                                                            /* Matrix product with sparse mat.:*/
    if (nXCa==nXRb)                                                             /*   Matrix product                  */
    {                                                                           /*   >>                              */
      if (Z && A && B) nErr = dlm_mult_sparse(Z,A,nXRa,nXCa,B,nXRb,nXCb);       /*     Call computation function     */
      if (lpnXRz) *lpnXRz = nXRa;                                               /*     Store no. of rows of result   */
      if (lpnXCz) *lpnXCz = nXCb;                                               /*     Store no. of columns of result*/
      bElemws = FALSE;                                                          /*     No elementwise computation!   */
    }                                                                           /*   <<                              */
    else if ((nXRa>1 || nXCa>1) && (nXRb>1 || nXCb>1))                          /*   Neither matrix nor scalar prod. */
    {                                                                           /*   >>                              */
      nErr = ERR_MDIM;                                                          /*     Could not do anything         */
      if (lpnXRz) *lpnXRz = 0;                                                  /*     Store no. of rows of result   */
      if (lpnXCz) *lpnXCz = 0;                                                  /*     Store no. of columns of result*/
      bElemws = FALSE;                                                          /*     No elementwise computation!   */
    }                                                                           /*   <<                              */
    break;                                                                      /*   == (do elementwise computation) */
  case OP_MULT_KRON:                                                            /* Kronecker product:                */
    if (Z && A && B) nErr = dlm_mult_kron(Z,A,nXRa,nXCa,B,nXRb,nXCb);           /*   Call computation function       */
    if (lpnXRz) *lpnXRz = nXRa*nXRb;                                            /*   Store no. of rows of result     */
    if (lpnXCz) *lpnXCz = nXCa*nXCb;                                            /*   Store no. of columns of result  */
    bElemws = FALSE;                                                            /*   No elementwise computation!     */
    break;                                                                      /*   == (do elementwise computation) */
  case OP_MULT_AKAT:                                                              /* Kronecker product:              */
    if (Z && A && B) nErr = dlm_mult_akat(Z,A,nXCa,nXRa,B);                         /*   Call computation function   */
    if (lpnXRz) *lpnXRz = nXRa;                                                     /*   Store no. of rows of result */
    if (lpnXCz) *lpnXCz = nXRa;                                                 /*   Store no. of columns of result  */
    bElemws = FALSE;                                                            /*   No elementwise computation!     */
    break;                                                                      /*   == (do elementwise computation) */
  case OP_CONV:                                                                 /* Convolution                       */
    bElemws = FALSE;                                                            /*   No elementwise computation!     */
    if      (nXCa==1 && nXCb==1) { nXDa=nXRa; nXDb=nXRb; }                      /*   Signals in columns --> ok       */
    else if (nXRa==1 && nXRb==1) { nXDa=nXCa; nXDb=nXCb; }                      /*   Signals in rows --> ok          */
    else                         { nErr = ERR_MDIM; break; }                    /*   == No go!                       */
    if (Z && A && B) dlm_convol_t(Z,A,nXDa,B,nXDb);                             /*     Call computation function     */
    if (lpnXRz) *lpnXRz = nXRa==1 ? 1 : nXRa+nXRb;                              /*     Store no. of rows of result   */
    if (lpnXCz) *lpnXCz = nXCa==1 ? 1 : nXCa+nXCb;                              /*     Store no. of columns of result*/
    break;                                                                      /*   ==                              */
  case OP_LOG:  /* FALL THROUGH */                                              /* Decadic logarithm                 */
  case OP_LOG2: /* FALL THROUGH */                                              /* Binary logarithm                  */
  case OP_LN:   /* FALL THROUGH */                                              /* Natural logarithm                 */
  case OP_EXP:  /* FALL THROUGH */                                              /* Exponential function              */
  case OP_SQRT: /* FALL THROUGH */                                              /* Square root                       */
    bElemws = FALSE;                                                            /*   No elementwise computation!     */
    if (nXRa!=nXCa) { nErr = ERR_MSQR; break; }                                 /*   == A must be a square matrix    */
    if (Z && A) dlm_diagop(Z,A,nXRa,0,nOpcode,FALSE);                           /*   Diagonalized computation        */
    if (lpnXRz) *lpnXRz = nXRa;                                                 /*   Store no. of rows of result     */
    if (lpnXCz) *lpnXCz = nXRa;                                                 /*   Store no. of columns of result  */
    break;                                                                      /*   ==                              */
  case OP_POW:                                                                  /* Matrix power:                     */
    bElemws = FALSE;                                                            /*   No elementwise computation!     */
    if (nXRb==1 && nXCb==1)                                                     /*   A^b                             */
    {                                                                           /*   >>                              */
      if (nXRa!=nXCa) { nErr = ERR_MSQR; break; }                               /*     == A must be a square matrix  */
      if (Z && A && B)                                                          /*     Do computation                */
      {                                                                         /*     >>                            */
        if ((FLOAT64)((UINT32)*B)==*B)                                          /*       Positive integer power      */
          nErr = dlm_intpower(Z,A,nXRa,(UINT32)*B);                             /*         Successive multiplication */
        else                                                                    /*       Other powers                */
          nErr = dlm_diagop(Z,A,nXRa,*B,nOpcode,FALSE);                         /*         Diagonalized operation    */
      }                                                                         /*     <<                            */
      if (lpnXRz) *lpnXRz = nXRa;                                               /*     Store no. of rows of result   */
      if (lpnXCz) *lpnXCz = nXRa;                                               /*     Store no. of columns of result*/
    }                                                                           /*   <<                              */
    else if (nXRa==1 && nXCa==1)                                                /*   a^B                             */
    {                                                                           /*   >>                              */
      if (nXRb!=nXCb) { nErr = ERR_MSQR; break; }                               /*     == B must be a square matrix  */
      if (Z && A && B) nErr = dlm_diagop(Z,B,nXRb,*A,nOpcode,TRUE);             /*     Diagonalized computation      */
      if (lpnXRz) *lpnXRz = nXRb;                                               /*     Store no. of rows of result   */
      if (lpnXCz) *lpnXCz = nXRb;                                               /*     Store no. of columns of result*/
    }                                                                           /*   <<                              */
    else { nErr = ERR_MDIM; break; }                                            /*   One operand must be a scalar    */
    break;                                                                      /*   ==                              */
  case OP_MULT_EL   : nOpcode = OP_MULT;    break;                              /*   Elementwise multiplication      */
  case OP_DIV_EL    : nOpcode = OP_DIV;     break;                              /*   Elementwise division            */
  case OP_LOG_EL    : nOpcode = OP_LOG;     break;                              /*   Elementwise decadic logaritm    */
  case OP_LOG2_EL   : nOpcode = OP_LOG2;    break;                              /*   Elementwise binary logaritm     */
  case OP_LN_EL     : nOpcode = OP_LN;      break;                              /*   Elementwise natural logaritm    */
  case OP_EXP_EL    : nOpcode = OP_EXP;     break;                              /*   Elementwise exponential function*/
  case OP_SQRT_EL   : nOpcode = OP_SQRT;    break;                              /*   Elementwise square root         */
  case OP_POW_EL    : nOpcode = OP_POW;     break;                              /*   Elementwise power               */
  case OP_OR_EL     : nOpcode = OP_OR;      break;                              /*   Elementwise logical or          */
  case OP_BITOR_EL  : nOpcode = OP_BITOR;   break;                              /*   Elementwise bitwise or          */
  case OP_AND_EL    : nOpcode = OP_AND;     break;                              /*   Elementwise logical and         */
  case OP_BITAND_EL : nOpcode = OP_BITAND;  break;                              /*   Elementwise bitwise and         */
  case OP_EQUAL_EL  : nOpcode = OP_EQUAL;   break;                              /*   Elementwise equal               */
  case OP_NEQUAL_EL : nOpcode = OP_NEQUAL;  break;                              /*   Elementwise not equal           */
  case OP_LESS_EL   : nOpcode = OP_LESS;    break;                              /*   Elementwise less than           */
  case OP_GREATER_EL: nOpcode = OP_GREATER; break;                              /*   Elementwise greater than        */
  case OP_LEQ_EL    : nOpcode = OP_LEQ;     break;                              /*   Elementwise less or equal       */
  case OP_GEQ_EL    : nOpcode = OP_GEQ;     break;                              /*   Elementwise greater or equal    */

  /* Signal operations */                                                       /* --------------------------------- */
  case OP_CCF:                                                                  /* Cross correlation function        */
    bElemws = FALSE;                                                            /*   No elementwise computation!     */
    if      (nXCa==1 && nXCb==1) { nXDa=nXRa; nXDb=nXRb; }                      /*   Signals in columns --> ok       */
    else if (nXRa==1 && nXRb==1) { nXDa=nXCa; nXDb=nXCb; }                      /*   Signals in rows --> ok          */
    else                         { nErr = ERR_MDIM; break; }                    /*   == No go!                       */
    if (Z && A && B) dlm_ccf(Z,A,nXDa,B,nXDb);                                  /*     Call computation function     */
    if (lpnXRz) *lpnXRz = nXRb;                                                 /*     Store no. of rows of result   */
    if (lpnXCz) *lpnXCz = nXCb;                                                 /*     Store no. of columns of result*/
    break;                                                                      /*   ==                              */

  /* Spectral transforms */                                                     /* --------------------------------- */
  /* TODO: case OP_APS: */                                                      /* Auto-power spectrum               */

  /* Matrix constants */                                                        /* --------------------------------- */
  case OP_ZEROS:                                                                /* 0-matrix                          */
  case OP_ONES:                                                                 /* 1-matrix                          */
  case OP_NOISE:                                                                /* White noise matrix                */
    if (A && B && Z) nErr = dlm_constant(Z,A[0],B[0],nOpcode);                  /*   Get matrix constant             */
    if (A && lpnXRz) *lpnXRz = A[0];                                            /*   Store no. of rows of result     */
    if (B && lpnXCz) *lpnXCz = B[0];                                            /*   Store no. of columns of result  */
    bElemws = FALSE; break;                                                     /*   == (no elementwise computation) */
  case OP_UNITMAT: /* FALL THROUGH */                                           /* Unit matrix                       */
  case OP_HILBMAT: /* FALL THROUGH */                                           /* Hilbert matrix                    */
  case OP_IHLBMAT:                                                              /* Inverse Hilbert matrix            */
    if (A && Z     ) nErr = dlm_constant(Z,A[0],A[0],nOpcode);                  /*   Get matrix constant             */
    if (A && lpnXRz) *lpnXRz = A[0];                                            /*   Store no. of rows of result     */
    if (A && lpnXCz) *lpnXCz = A[0];                                            /*   Store no. of columns of result  */
    bElemws = FALSE; break;                                                     /*   == (no elementwise computation) */
  }                                                                             /* <<                                */

  /* General scalar or elementwise operations */                                /* --------------------------------- */
  if (bElemws)                                                                  /* Elementwise (or scalar) computn.  */
  {                                                                             /* >>                                */
    if (nXRa==1 && nXCa==1)                                                     /*   Matrix A is a single value      */
    {                                                                           /*   >>                              */
      if (Z && A && B) nErr = dlm_scalop(Z,B,nXRb,nXCb,A[0],nOpcode,TRUE);      /*     Call computation function     */
      if (lpnXRz) *lpnXRz = nXRb;                                               /*     Store no. of rows of result   */
      if (lpnXCz) *lpnXCz = nXCb;                                               /*     Store no. of columns of result*/
    }                                                                           /*   <<                              */
    else if (nXRb==1 && nXCb==1)                                                /*   Matrix B is a single value      */
    {                                                                           /*   >>                              */
      if (Z && A && B) nErr = dlm_scalop(Z,A,nXRa,nXCa,B[0],nOpcode,FALSE);     /*     Call computation function     */
      if (lpnXRz) *lpnXRz = nXRa;                                               /*     Store no. of rows of result   */
      if (lpnXCz) *lpnXCz = nXCa;                                               /*     Store no. of columns of result*/
    }                                                                           /*   <<                              */
    else if (nXRb==nXRa && nXCb==1)                                             /*   Matrix B is a nXRa-dim.-vector  */
    {                                                                           /*   >>                              */
      if (Z && A && B) nErr = dlm_vectop(Z,A,nXRa,nXCa,B,nOpcode,FALSE, FALSE); /*     Call computation function     */
      if (lpnXRz) *lpnXRz = nXRa;                                               /*     Store no. of rows of result   */
      if (lpnXCz) *lpnXCz = nXCa;                                               /*     Store no. of columns of result*/
    }                                                                           /*   <<                              */
    else if (nXRa==nXRb && nXCa==1)                                             /*   Matrix A is a nXRb-dim.-vector  */
    {                                                                           /*   >>                              */
      if (Z && A && B) nErr = dlm_vectop(Z,B,nXRb,nXCb,A,nOpcode,TRUE, FALSE);  /*     Call computation function     */
      if (lpnXRz) *lpnXRz = nXRb;                                               /*     Store no. of rows of result   */
      if (lpnXCz) *lpnXCz = nXCb;                                               /*     Store no. of columns of result*/
    }                                                                           /*   <<                              */
    else if (nXCb==nXCa && nXRb==1)                                             /*   Matrix B is a nXCa-dim.-vector  */
    {                                                                           /*   >>                              */
      if (Z && A && B) nErr = dlm_vectop(Z,A,nXRa,nXCa,B,nOpcode,FALSE, TRUE);  /*     Call computation function     */
      if (lpnXRz) *lpnXRz = nXRa;                                               /*     Store no. of rows of result   */
      if (lpnXCz) *lpnXCz = nXCa;                                               /*     Store no. of columns of result*/
    }                                                                           /*   <<                              */
    else if (nXCa==nXCb && nXRa==1)                                             /*   Matrix A is a nXCb-dim.-vector  */
    {                                                                           /*   >>                              */
      if (Z && A && B) nErr = dlm_vectop(Z,B,nXRb,nXCb,A,nOpcode,TRUE, TRUE);   /*     Call computation function     */
      if (lpnXRz) *lpnXRz = nXRb;                                               /*     Store no. of rows of result   */
      if (lpnXCz) *lpnXCz = nXCb;                                               /*     Store no. of columns of result*/
    }                                                                           /*   <<                              */
    else                                                                        /*   Elementwise operation           */
    {                                                                           /*   >>                              */
      if (Z && A && B) nErr = dlm_elemop(Z,A,nXRa,nXCa,B,nXRb,nXCb,nOpcode);    /*     Call computation function     */
      if (lpnXRz) *lpnXRz = MAX(nXRa,nXRb);                                     /*     Store no. of rows of result   */
      if (lpnXCz) *lpnXCz = MAX(nXCa,nXCb);                                     /*     Store no. of columns of result*/
    }                                                                           /*   <<                              */
  }                                                                             /* <<                                */

  /* Aftermath */                                                               /* --------------------------------- */
  if (bZisA)
  {
    dlp_memmove((FLOAT64*)A,Z,dlp_size(A));
    dlp_free(Z);
    return nErr;
  }
  if (bZisB)
  {
    dlp_memmove((FLOAT64*)B,Z,dlp_size(B));
    dlp_free(Z);
    return nErr;
  }
  return nErr;
}

/**
 * <p>Performs matrix operations with two complex arguments.</p>
 *
 * @param Z
 *          Pointer to complex result matrix (may be <code>NULL</code> in order
 *          to just determine the dimensions of the result matrix without doing
 *          further computations)
 * @param lpnXRz
 *          Pointer to a long value to be filled with number of rows of the
 *          result (may be <code>NULL</code>)
 * @param lpnXCz
 *          Pointer to a long value to be filled with number of columns of the
 *          result (may be <code>NULL</code>)
 * @param A
 *          Pointer to complex operand matrix 1 (may be identical with
 *          <code>Z</code>, may be <code>NULL</code> for returning matrix
 *          constants or determining result dimensions without further
 *          computation)
 * @param nXRa
 *          Number of rows in operand matrix 1
 * @param nXCa
 *          Number of columns in operand matrix 1
 * @param B
 *          Pointer to complex operand matrix 1 (may be identical with
 *          <code>Z</code>, may be <code>NULL</code> for unary operations,
 *          matrix constants or determining result dimensions without further
 *          computation)
 * @param nXRb
 *          Number of rows in operand matrix 2
 * @param nXCb
 *          Number of columns in operand matrix 2
 * @param nOpcode
 *          Operation code
 * @return <code>O_K</code> if successful, a (negative) error code otherwise
 * @see dlm_matrop_code
 */
INT16 dlm_matropC
(
  COMPLEX64*       Z,
  INT32*           lpnXRz,
  INT32*           lpnXCz,
  const COMPLEX64* A,
  INT32            nXRa,
  INT32            nXCa,
  const COMPLEX64* B,
  INT32            nXRb,
  INT32            nXCb,
  INT16            nOpcode
)
{
  BOOL      bElemws = TRUE;                                                     /* Elementwise (or scalar) computn.  */
  INT16     nErr    = O_K;                                                      /* Be optimistic! :)                 */
  COMPLEX64 nBuf    = CMPLX(0.);                                                /* Complex buffer                    */
  BOOL      bZisA   = FALSE;                                                    /* Z==A flag                         */
  BOOL      bZisB   = FALSE;                                                    /* Z==B flag                         */

  /* Validate */                                                                /* --------------------------------- */
  if (A&&(nXCa>1||nXRa>1)&&dlp_size(A)<nXCa*nXRa*sizeof(COMPLEX64)) return ERR_MEM;/* Wrong dimensions for matrix A  */
  if (B&&(nXCb>1||nXRb>1)&&dlp_size(B)<nXCb*nXRb*sizeof(COMPLEX64)) return ERR_MEM;/* Wrong dimensions for matrix B  */

  /* Initialize */                                                              /* --------------------------------- */
  if (Z && Z==A)                                                                /* Result identical with A           */
  {                                                                             /* >>                                */
    bZisA = TRUE;                                                               /*   Remember that                   */
    Z = (COMPLEX64*)dlp_calloc(dlp_size(A)/sizeof(COMPLEX64),sizeof(COMPLEX64));/*   Create tmp. array of equal size */
  }                                                                             /* <<                                */
  else if (Z && Z==B)                                                           /* Result not A but identical with B */
  {                                                                             /* >>                                */
    bZisB = TRUE;                                                               /*   Remember that                   */
    Z = (COMPLEX64*)dlp_calloc(dlp_size(B)/sizeof(COMPLEX64),sizeof(COMPLEX64));/*   Create tmp. array of equal size */
  }                                                                             /* <<                                */

  switch (nOpcode)                                                              /* Branch for opcode                 */
  {                                                                             /* >>                                */
  /* Dedicated matrix operations */                                             /* --------------------------------- */
  case OP_REAL: /* FALL THROUGH */                                              /* Real part                         */
  case OP_IMAG: /* FALL THROUGH */                                              /* Imaginary part                    */
  case OP_CONJ: /* FALL THROUGH */                                              /* Complex conjugate                 */
  case OP_ABS:  break;                                                          /* Absolute values                   */
  case OP_TRANSPOSE:                                                            /* Transpose:                        */
    if (Z && A) nErr    = dlm_transposeC(Z,A,nXRa,nXCa);                        /*   Call computation function       */
    if (lpnXRz) *lpnXRz = nXCa;                                                 /*   Store no. of rows of result     */
    if (lpnXCz) *lpnXCz = nXRa;                                                 /*   Store no. of columns of result  */
    bElemws = FALSE; break;                                                     /*   == (no elementwise computation) */
  case OP_INVT:                                                                 /* Inversion:                        */
    bElemws = FALSE;                                                            /*   No elementwise computation!     */
    if (nXRa!=nXCa) { nErr = ERR_MSQR; break; }                                 /*   == A must be a square matrix    */
    if (Z && A)                                                                 /*   Have input and output matrices  */
    {                                                                           /*   >>                              */
      dlp_memmove(Z,A,nXRa*nXRa*sizeof(COMPLEX64));                             /*     Copy input to output          */
      nErr = dlm_invert_gelC(Z,nXRa,&nBuf);                                     /*     Invert through Gaussian elim. */
      IF_NOK(nErr)  { _DMS(Z,nXRa*nXRa,CMPLX(0.)); break; }                     /*     == Failed -> clear result     */
      if (CMPLX_EQUAL(nBuf,CMPLX(0.))) {                                        /*     == A singular -> clear result */
        _DMS(Z,nXRa*nXRa,CMPLX(0.)); nErr=ERR_MSGL; break; }                    /*     |                             */
    }                                                                           /*   <<                              */
    if (lpnXRz) *lpnXRz = nXRa;                                                 /*   Store no. of rows of result     */
    if (lpnXCz) *lpnXCz = nXRa;                                                 /*   Store no. of columns of result  */
    break;                                                                      /*   ==                              */
  case OP_CHOLF:                                                                /* Cholesky factorization:           */
    bElemws = FALSE;                                                            /*   No elementwise computation!     */
    if (nXRa!=nXCa) { nErr = ERR_MSQR; break; }                                 /*   == A must be a square matrix    */
    /* TODO: Check A is symmetric! */                                           /*                                   */
    if (Z && A)                                                                 /*   Have input and output matrices  */
    {                                                                           /*   >>                              */
      nErr = dlm_cholfC(Z,A,nXRa);                                              /*     Do Cholesky factorization     */
      IF_NOK(nErr)  { _DMS(Z,nXRa*nXRa,CMPLX(0.)); break; }                     /*     == Failed -> clear result     */
    }                                                                           /*   <<                              */
    if (lpnXRz) *lpnXRz = nXRa;                                                 /*   Store no. of rows of result     */
    if (lpnXCz) *lpnXCz = nXRa;                                                 /*   Store no. of columns of result  */
    break;                                                                      /*   ==                              */
  case OP_SOLV_LU:                                                              /* Solve Ax=B due LU decomposition   */
    bElemws = FALSE;                                                            /*   No elementwise computation!     */
    if (nXRa!=nXCa) { nErr = ERR_MSQR; break; }                                 /*   == A must be a square matrix    */
    if (nXRa!=nXRb) { nErr = ERR_MDIM; break; }                                 /*   == Dimension must fit           */
    if (Z && A)                                                                 /*   Have input and output matrices  */
    {                                                                           /*   >>                              */
      dlp_memmove(Z,B,nXRa*nXCb*sizeof(FLOAT64));                               /*     Adaption to worker function   */
      nErr = dlm_solve_ludC((COMPLEX64*)A,nXRa,Z,nXCb);                         /*     Do Cholesky factorization     */
      IF_NOK(nErr)  { _DMS(Z,nXRa*nXRa,CMPLX(0.)); break; }                     /*     == Failed -> clear result     */
    }                                                                           /*   <<                              */
    if (lpnXRz) *lpnXRz = nXRa;                                                 /*   Store no. of rows of result     */
    if (lpnXCz) *lpnXCz = nXCb;                                                 /*   Store no. of columns of result  */
    break;                                                                      /*   ==                              */
  case OP_TRACE: /* FALL THROUGH */                                             /* Trace:                            */
  case OP_MDIAG:                                                                /* Main diagonal:                    */
    bElemws = FALSE;                                                            /*   No elementwise computation!     */
    if (nXRa!=nXCa) { nErr = ERR_MSQR; break; }                                 /*   Only for square matrices        */
    if (Z && A)                                                                 /*   Have input and output matrices  */
    {                                                                           /*   >>                              */
      INT32 i = 0;                                                              /*     Loop counter                 */
      for (i=0; i<nXRa; i++)                                                    /*     Loop over matrix dimension    */
        if (nOpcode==OP_MDIAG) Z[i] =A[i*nXRa+i];                               /*       Extract main diagonal       */
        else                   Z[0] =CMPLX_PLUS(Z[0],A[i*nXRa+i]);              /*       Compute trace               */
    }                                                                           /*   <<                              */
    if (lpnXRz) *lpnXRz = nOpcode==OP_MDIAG ? nXRa : 1;                         /*   Store no. of rows of result     */
    if (lpnXCz) *lpnXCz = 1;                                                    /*   Store no. of columns of result  */
    break;                                                                      /*   ==                              */
  case OP_DIAG:                                                                 /* Make diagonal matrix              */
    return ERR_MTYPE; /* TODO: Implement! */
  case OP_RANK: /* FALL THROUGH */                                              /* Rank:                             */
  case OP_DET:                                                                  /* Determinant:                      */
    bElemws = FALSE;                                                            /*   No elementwise computation!     */
    if (nXRa!=nXCa) { nErr = ERR_MSQR; break; }                                 /*   == A must be a square matrix    */
    if (Z && A)                                                                 /*   Have input and output matrices  */
    {                                                                           /*   >>                              */
      COMPLEX64* lpnBuf = (COMPLEX64*)dlp_calloc(nXRa*nXRa,sizeof(COMPLEX64));  /*     Aux. buffer for inversion     */
      dlp_memmove(lpnBuf,A,nXRa*nXRa*sizeof(COMPLEX64));                        /*     Copy matrix A into buffer     */
      if (nOpcode==OP_DET) nErr = dlm_det_ludC(lpnBuf,nXRa,Z);                  /*     Calculate                     */
      dlp_free(lpnBuf);                                                         /*     Free auxilary buffer          */
    }                                                                           /*   <<                              */
    if (lpnXRz) *lpnXRz = 1;                                                    /*   Store no. of rows of result     */
    if (lpnXCz) *lpnXCz = 1;                                                    /*   Store no. of columns of result  */
    break;                                                                      /*   ==                              */
  case OP_MULT:                                                                 /* Multiplication:                   */
    if (nXCa==nXRb)                                                             /*   Matrix product                  */
    {                                                                           /*   >>                              */
      if (Z && A && B) nErr = dlm_multC(Z,A,nXRa,nXCa,B,nXRb,nXCb);             /*     Call computation function     */
      if (lpnXRz) *lpnXRz = nXRa;                                               /*     Store no. of rows of result   */
      if (lpnXCz) *lpnXCz = nXCb;                                               /*     Store no. of columns of result*/
      bElemws = FALSE;                                                          /*     No elementwise computation!   */
    }                                                                           /*   <<                              */
    else if ((nXRa>1 || nXCa>1) && (nXRb>1 || nXCb>1))                          /*   Neither matrix nor scalar prod. */
    {                                                                           /*   >>                              */
      nErr = ERR_MDIM;                                                          /*     Could not do anything         */
      if (lpnXRz) *lpnXRz = 0;                                                  /*     Store no. of rows of result   */
      if (lpnXCz) *lpnXCz = 0;                                                  /*     Store no. of columns of result*/
      bElemws = FALSE;                                                          /*     No elementwise computation!   */
    }                                                                           /*   <<                              */
    break;                                                                      /*   == (do elementwise computation) */
  case OP_MULT_SPARSE:                                                          /* Matrix product with sparse mat.:  */
    if (nXCa==nXRb)                                                             /*   Matrix product                  */
    {                                                                           /*   >>                              */
      if (Z&&A&&B) nErr=dlm_mult_sparseC(Z,A,nXRa,nXCa,B,nXRb,nXCb);            /*     Call computation function     */
      if (lpnXRz) *lpnXRz = nXRa;                                               /*     Store no. of rows of result   */
      if (lpnXCz) *lpnXCz = nXCb;                                               /*     Store no. of columns of result*/
      bElemws = FALSE;                                                          /*     No elementwise computation!   */
    }                                                                           /*   <<                              */
    else if ((nXRa>1 || nXCa>1) && (nXRb>1 || nXCb>1))                          /*   Neither matrix nor scalar prod. */
    {                                                                           /*   >>                              */
      nErr = ERR_MDIM;                                                          /*     Could not do anything         */
      if (lpnXRz) *lpnXRz = 0;                                                  /*     Store no. of rows of result   */
      if (lpnXCz) *lpnXCz = 0;                                                  /*     Store no. of columns of result*/
      bElemws = FALSE;                                                          /*     No elementwise computation!   */
    }                                                                           /*   <<                              */
    break;                                                                      /*   == (do elementwise computation) */
  case OP_MULT_KRON:                                                            /* Kronecker product:                */
    if (Z && A && B) nErr = dlm_mult_kronC(Z,A,nXRa,nXCa,B,nXRb,nXCb);          /*   Call computation function       */
    if (lpnXRz) *lpnXRz = nXRa*nXRb;                                            /*   Store no. of rows of result     */
    if (lpnXCz) *lpnXCz = nXCa*nXCb;                                            /*   Store no. of columns of result  */
    bElemws = FALSE;                                                            /*   No elementwise computation!     */
    break;                                                                      /*   == (do elementwise computation) */
  case OP_MULT_AKAT:                                                            /* Kronecker product:                */
    if (Z && A && B) nErr = dlm_mult_akatC(Z,A,nXCa,nXRa,B);                    /*   Call computation function       */
    if (lpnXRz) *lpnXRz = nXRa;                                                 /*   Store no. of rows of result     */
    if (lpnXCz) *lpnXCz = nXRa;                                                 /*   Store no. of columns of result  */
    bElemws = FALSE;                                                            /*   No elementwise computation!     */
    break;                                                                      /*   == (do elementwise computation) */
  case OP_CONV:                                                                 /* Convolution                       */
    return ERR_MTYPE; /* TODO: Implement! */
  case OP_LOG:  /* FALL THROUGH */                                              /* Decadic logarithm                 */
  case OP_LOG2: /* FALL THROUGH */                                              /* Binary logarithm                  */
  case OP_LN:   /* FALL THROUGH */                                              /* Natural logarithm                 */
  case OP_EXP:  /* FALL THROUGH */                                              /* Exponential function              */
  case OP_SQRT: /* FALL THROUGH */                                              /* Square root                       */
  case OP_POW:                                                                  /* Matrix power:                     */
    return ERR_MTYPE; /* TODO: Implement! */                                    /*   ==                              */
  case OP_MULT_EL   : nOpcode = OP_MULT;    break;                              /*   Elementwise multiplication      */
  case OP_DIV_EL    : nOpcode = OP_DIV;     break;                              /*   Elementwise division            */
  case OP_LOG_EL    : nOpcode = OP_LOG;     break;                              /*   Elementwise decadic logaritm    */
  case OP_LOG2_EL   : nOpcode = OP_LOG2;    break;                              /*   Elementwise binary logaritm     */
  case OP_LN_EL     : nOpcode = OP_LN;      break;                              /*   Elementwise natural logaritm    */
  case OP_EXP_EL    : nOpcode = OP_EXP;     break;                              /*   Elementwise exponential function*/
  case OP_SQRT_EL   : nOpcode = OP_SQRT;    break;                              /*   Elementwise square root         */
  case OP_POW_EL    : nOpcode = OP_POW;     break;                              /*   Elementwise power               */
  case OP_OR_EL     : nOpcode = OP_OR;      break;                              /*   Elementwise logical or          */
  case OP_BITOR_EL  : nOpcode = OP_BITOR;   break;                              /*   Elementwise bitwise or          */
  case OP_AND_EL    : nOpcode = OP_AND;     break;                              /*   Elementwise logical and         */
  case OP_BITAND_EL : nOpcode = OP_BITAND;  break;                              /*   Elementwise bitwise and         */
  case OP_EQUAL_EL  : nOpcode = OP_EQUAL;   break;                              /*   Elementwise equal               */
  case OP_NEQUAL_EL : nOpcode = OP_NEQUAL;  break;                              /*   Elementwise not equal           */
  case OP_LESS_EL   : nOpcode = OP_LESS;    break;                              /*   Elementwise less than           */
  case OP_GREATER_EL: nOpcode = OP_GREATER; break;                              /*   Elementwise greater than        */
  case OP_LEQ_EL    : nOpcode = OP_LEQ;     break;                              /*   Elementwise less or equal       */
  case OP_GEQ_EL    : nOpcode = OP_GEQ;     break;                              /*   Elementwise greater or equal    */
  /* Matrix constants */                                                        /* --------------------------------- */
  case OP_ZEROS:                                                                /* 0-matrix                          */
  case OP_ONES:                                                                 /* 1-matrix                          */
  case OP_NOISE:                                                                /* White noise matrix                */
    if (A && B && Z) nErr = dlm_constantC(Z,A->x,B->x,nOpcode);                 /*   Get matrix constant             */
    if (A && lpnXRz) *lpnXRz = A->x;                                            /*   Store no. of rows of result     */
    if (B && lpnXCz) *lpnXCz = B->x;                                            /*   Store no. of columns of result  */
    bElemws = FALSE; break;                                                     /*   == (no elementwise computation) */
  case OP_UNITMAT: /* FALL THROUGH */                                           /* Unit matrix                       */
  case OP_HILBMAT: /* FALL THROUGH */                                           /* Hilbert matrix                    */
  case OP_IHLBMAT:                                                              /* Inverse Hilbert matrix            */
    if (A && Z     ) nErr = dlm_constantC(Z,A->x,A->x,nOpcode);                 /*   Get matrix constant             */
    if (A && lpnXRz) *lpnXRz = A->x;                                            /*   Store no. of rows of result     */
    if (A && lpnXCz) *lpnXCz = A->x;                                            /*   Store no. of columns of result  */
    bElemws = FALSE; break;                                                     /*   == (no elementwise computation) */
  }                                                                             /* <<                                */


  /* General scalar or elementwise operations */                                /* --------------------------------- */
  if (bElemws)                                                                  /* Elementwise (or scalar) computn.  */
  {                                                                             /* >>                                */
    if (nXRa==1 && nXCa==1)                                                     /*   Matrix A is a single value      */
    {                                                                           /*   >>                              */
      if (Z && A && B) nErr=dlm_scalopC(Z,B,nXRb,nXCb,A,nOpcode,TRUE);          /*     Call computation function     */
      if (lpnXRz) *lpnXRz = nXRb;                                               /*     Store no. of rows of result   */
      if (lpnXCz) *lpnXCz = nXCb;                                               /*     Store no. of columns of result*/
    }                                                                           /*   <<                              */
    else if (nXRb==1 && nXCb==1)                                                /*   Matrix B is a single value      */
    {                                                                           /*   >>                              */
      if (Z && A && B) nErr=dlm_scalopC(Z,A,nXRa,nXCa,B,nOpcode,FALSE);         /*     Call computation function     */
      if (lpnXRz) *lpnXRz = nXRa;                                               /*     Store no. of rows of result   */
      if (lpnXCz) *lpnXCz = nXCa;                                               /*     Store no. of columns of result*/
    }                                                                           /*   <<                              */
    else if (nXRb==nXRa && nXCb==1)                                             /*   Matrix B is a nXRa-dim.-vector  */
    {                                                                           /*   >>                              */
      if (Z && A && B) nErr=dlm_vectopC(Z,A,nXRa,nXCa,B,nOpcode,FALSE, FALSE);  /*     Call computation function     */
      if (lpnXRz) *lpnXRz = nXRa;                                               /*     Store no. of rows of result   */
      if (lpnXCz) *lpnXCz = nXCa;                                               /*     Store no. of columns of result*/
    }                                                                           /*   <<                              */
    else if (nXRa==nXRb && nXCa==1)                                             /*   Matrix A is a nXRb-dim.-vector  */
    {                                                                           /*   >>                              */
      if (Z && A && B) nErr=dlm_vectopC(Z,B,nXRb,nXCb,A,nOpcode,TRUE, FALSE);   /*     Call computation function     */
      if (lpnXRz) *lpnXRz = nXRb;                                               /*     Store no. of rows of result   */
      if (lpnXCz) *lpnXCz = nXCb;                                               /*     Store no. of columns of result*/
    }                                                                           /*   <<                              */
    else if (nXCb==nXCa && nXRb==1)                                             /*   Matrix B is a nXCa-dim.-vector  */
    {                                                                           /*   >>                              */
      if (Z && A && B) nErr=dlm_vectopC(Z,A,nXRa,nXCa,B,nOpcode,FALSE, FALSE);  /*     Call computation function     */
      if (lpnXRz) *lpnXRz = nXRa;                                               /*     Store no. of rows of result   */
      if (lpnXCz) *lpnXCz = nXCa;                                               /*     Store no. of columns of result*/
    }                                                                           /*   <<                              */
    else if (nXCa==nXCb && nXRa==1)                                             /*   Matrix A is a nXCb-dim.-vector  */
    {                                                                           /*   >>                              */
      if (Z && A && B) nErr=dlm_vectopC(Z,B,nXRb,nXCb,A,nOpcode,TRUE, FALSE);   /*     Call computation function     */
      if (lpnXRz) *lpnXRz = nXRb;                                               /*     Store no. of rows of result   */
      if (lpnXCz) *lpnXCz = nXCb;                                               /*     Store no. of columns of result*/
    }                                                                           /*   <<                              */
    else                                                                        /*   Elementwise operation           */
    {                                                                           /*   >>                              */
      if (Z && A && B) nErr=dlm_elemopC(Z,A,nXRa,nXCa,B,nXRb,nXCb,nOpcode);     /*     Call computation function     */
      if (lpnXRz) *lpnXRz = MAX(nXRa,nXRb);                                     /*     Store no. of rows of result   */
      if (lpnXCz) *lpnXCz = MAX(nXCa,nXCb);                                     /*     Store no. of columns of result*/
    }                                                                           /*   <<                              */
  }                                                                             /* <<                                */

  /* Aftermath */                                                               /* --------------------------------- */
  if (bZisA)
  {
    dlp_memmove((COMPLEX64*)A,Z,dlp_size(A));
    dlp_free(Z);
    return nErr;
  }
  if (bZisB)
  {
    dlp_memmove((COMPLEX64*)B,Z,dlp_size(B));
    dlp_free(Z);
    return nErr;
  }
  return nErr;
}

/* EOF */
