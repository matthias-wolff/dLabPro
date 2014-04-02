/* dLabPro class CMatrix (matrix)
 * - Class CMatrix worker methods
 *
 * AUTHOR : Matthias Wolff
 * PACKAGE: dLabPro/classes
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

#include "dlp_cscope.h"                                                         /* Indicate C scope                  */
#include "dlp_matrix.h"
#include "dlp_math.h"

/* == Static methods == */                                                      /* ================================= */
#undef  dlp_calloc
#define dlp_calloc(A,B) __dlp_calloc(A,B,__FILE__,__LINE__,"CMatrix_...",NULL)

/**
 * Creates and copies symbolic components. Like <code>CData_CopyLabels</code>
 * but creates new symbolic components in <code>idDst</code> first.
 */
INT16 CGEN_SPRIVATE CMatrix_CopyLabels(CData* idDst, CData* idSrc)
{
  INT32 nC  = 0;                                                                 /* Current component                 */
  INT32 nR  = 0;                                                                 /* Current record                    */
  INT32 nXR = 0;                                                                 /* Number of source records          */
  
  if (!idDst      ) return NOT_EXEC;                                            /* No destination, no service        */
  if (!idSrc      ) return O_K;                                                 /* No source, nothing to do          */
  if (idSrc==idDst) return O_K;                                                 /* Src.and dest.equal, nothing to do */

  nXR = CData_GetNRecs(idSrc);                                                  /* Get number of records             */
  for (nC=0; nC<CData_GetNComps(idSrc); nC++)                                   /* Loop over source components       */
    if (dlp_is_symbolic_type_code(CData_GetCompType(idSrc,nC)))                 /*   If component is symbolic        */
    {                                                                           /*   >>                              */
      CData_AddComp(idDst,CData_GetCname(idSrc,nC),                             /*     Add it to destination         */
        CData_GetCompType(idSrc,nC));                                           /*     |                             */
      if(CData_GetNRecs(idDst)==0) CData_Allocate(idDst,nXR);                   /*     If not allocated -> do it     */
      for (nR=0; nR<nXR; nR++)                                                  /*     Loop over records             */
        CData_Sstore(idDst,(char*)CData_XAddr(idSrc,nR,nC),nR,                  /*       Copy labels                 */
          CData_GetNComps(idDst)-1);                                            /*       |                           */
    }                                                                           /*   <<                              */
    
  return O_K;                                                                   /* That's it                         */
}

/**
 * Implementation of eigenvector transform.
 */
INT16 CGEN_SPRIVATE CMatrix_EigenInt
(
  CData* A,
  CData* V,
  CData* L,
  BOOL   bNorm
)
{
  INT32    nB     = 0;                                                           /* Current block (=matrix)           */
  INT32    nXB    = 0;                                                           /* Number of blocks (=matrices)      */
  INT32    nXD    = 0;                                                           /* Dimensions of (square) matrices   */
  FLOAT64* lpnV = NULL;                                                          /* Eigenvector buffer                */
  FLOAT64* lpnL = NULL;                                                          /* Eigenvalue buffer                 */

  if (!V && !L) return NOT_EXEC;                                                /* No result, no service             */
  if (CData_IsEmpty(A))                                                         /* Source empty (nothing to be done) */
  {                                                                             /* >>                                */
    CData_Reset(BASEINST(V),TRUE);                                              /*   Reset eigenvalue matrix         */
    CData_Reset(BASEINST(L),TRUE);                                              /*   Reset eigenvector matrix        */
    return NOT_EXEC;                                                            /*   Leave it                        */
  }                                                                             /* <<                                */
  nXB = CData_GetNBlocks(A);                                                    /* Get number of matrices in src.    */
  nXD = CData_GetNRecsPerBlock(A);                                              /* Get dimension of src. (# columns) */
  if (nXD!=CData_GetNNumericComps(A))                                           /* Check if src. matrices are square */
  {                                                                             /* >> (NO)                           */
    CData_Reset(BASEINST(V),TRUE);                                              /*   Reset eigenvalue matrix         */
    CData_Reset(BASEINST(L),TRUE);                                              /*   Reset eigenvector matrix        */
    return IERROR(A,DATA_MDIM," (not a square matrix)",0,0);                    /*   Matrix dimension error          */
  }                                                                             /* <<                                */
  if(dlp_is_complex_type_code(CData_GetCompType(A,0))) {                        /* TODO: Eigenvalues and -vectors    */
    return IERROR(A,DATA_NOSUPPORT,"Operation for complex matrices","",0);      /*       for complex matrices        */
  }                                                                             /*       currently not implemented   */
  if (L==V) L=NULL;                                                             /* Results must not be identical     */
  CData_Array(V,T_DOUBLE,nXD,nXD*nXB); CData_SetNBlocks(V,nXB);                 /* Prepare eigenvector matrix        */
  CData_Array(L,T_DOUBLE,nXD,nXD*nXB); CData_SetNBlocks(L,nXB);                 /* Prepare eigenvalue matrix         */
  lpnV = (FLOAT64*)dlp_calloc(nXD*nXD,sizeof(FLOAT64));                         /* Allocate eigenvector buffer       */
  lpnL = (FLOAT64*)dlp_calloc(nXD*nXD,sizeof(FLOAT64));                         /* Allocate eigenvalue buffer        */
  for (nB=0; nB<nXB; nB++)                                                      /* Loop over blocks                  */
  {                                                                             /* >>                                */
    CData_DblockFetch(A,lpnL,nB,nXD,nXD,-1);                                    /*   Fetch source matrix             */
    switch (dlm_eigen_jac(lpnL,lpnV,nXD,bNorm))                                 /*   Call computation function       */
    {                                                                           /*   >> (any errors?)                */
      case ERR_MNEG:                                                            /*     All elements negative:        */
        IERROR(A,DATA_MDATA_WARN,(long)nB," (all matrix elements negative)",0); /*       Warning                     */
        break;                                                                  /*       ==                          */
      case ERR_MASYM:                                                           /*     Matrix asymmetric:            */
        IERROR(A,DATA_MDATA_WARN,(long)nB," (matrix asymmetric)",0);            /*       Warning                     */
        break;                                                                  /*       ==                          */
    }                                                                           /*   <<                              */
    CData_DblockStore(V,lpnV,nB,nXD,nXD,-1);                                    /*   Store eigenvector matrix        */
    CData_DblockStore(L,lpnL,nB,nXD,nXD,-1);                                    /*   Store eigenvalue matric         */
  }                                                                             /* <<                                */
  dlp_free(lpnV);                                                               /* Free eigenvector buffer           */
  dlp_free(lpnL);                                                               /* Free eigenvalue buffer            */
  return O_K;                                                                   /* All done                          */
}

/**
 * Implementation of inverse eigenvector transform.
 */
INT16 CGEN_SPRIVATE CMatrix_IeigenInt
(
  CData* A,
  CData* V,
  CData* L,
  BOOL   bNorm
)
{
  INT32    nB     = 0;                                                           /* Current block (=matrix)           */
  INT32    nXB    = 0;                                                           /* Number of blocks (=matrices)      */
  INT32    nXD    = 0;                                                           /* Dimensions of (square) matrices   */
  FLOAT64* lpnA   = NULL;                                                        /* Destination buffer                */
  FLOAT64* lpnV = NULL;                                                          /* Eigenvector buffer                */
  FLOAT64* lpnL = NULL;                                                          /* Eigenvalue buffer                 */

  if (!A) return NOT_EXEC;                                                      /* No result, no service             */
  if (CData_IsEmpty(V)) return IERROR(A,DATA_EMPTY,"idEvc",0,0);                /* Need eigenvectors                 */
  if (CData_IsEmpty(L)) return IERROR(A,DATA_EMPTY,"idEvl",0,0);                /* Need eigenvalues                  */
  nXB = CData_GetNBlocks(V);                                                    /* Get number of matrices in src.    */
  nXD = CData_GetNRecsPerBlock(V);                                              /* Get dimension of src. (# columns) */
  if (nXB!=CData_GetNBlocks(L))                                                 /* Check number of eigenval. matrs.  */
  {                                                                             /* >> (not nXD x nXD)                */
    CData_Reset(BASEINST(A),TRUE);                                              /*   Reset destination matrix        */
    return IERROR(L,DATA_MDIM," (bad number of eigenvalue matrices)",0,0);      /*   Matrix dimension error          */
  }                                                                             /* <<                                */
  if (nXD!=CData_GetNNumericComps(V))                                           /* Check if evl. matrices are square */
  {                                                                             /* >> (NO)                           */
    CData_Reset(BASEINST(A),TRUE);                                              /*   Reset destination matrix        */
    return IERROR(V,DATA_MDIM," (not a square matrix)",0,0);                    /*   Matrix dimension error          */
  }                                                                             /* <<                                */
  if (nXD!=CData_GetNNumericComps(L) || nXD!=CData_GetNRecsPerBlock(L))         /* Check eigenvalue diagonal matrix  */
  {                                                                             /* >> (not nXD x nXD)                */
    CData_Reset(BASEINST(A),TRUE);                                              /*   Reset destination matrix        */
    return IERROR(L,DATA_MDIM," (eigenvalue matrix)",0,0);                      /*   Matrix dimension error          */
  }                                                                             /* <<                                */
  CData_Array(A,T_DOUBLE,nXD,nXD*nXB); CData_SetNBlocks(A,nXB);                 /* Prepare destination matrix        */
  lpnA = (FLOAT64*)dlp_calloc(nXD*nXD,sizeof(FLOAT64));                           /* Allocate destination buffer       */
  lpnV = (FLOAT64*)dlp_calloc(nXD*nXD,sizeof(FLOAT64));                           /* Allocate eigenvector buffer       */
  lpnL = (FLOAT64*)dlp_calloc(nXD*nXD,sizeof(FLOAT64));                           /* Allocate eigenvalue buffer        */
  for (nB=0; nB<nXB; nB++)                                                      /* Loop over blocks                  */
  {                                                                             /* >>                                */
    CData_DblockFetch(V,lpnV,nB,nXD,nXD,-1);                                    /*   Fetch eigenvector matrix        */
    CData_DblockFetch(L,lpnL,nB,nXD,nXD,-1);                                    /*   Fetch eigenvalue matric         */
    dlm_ieigen(lpnA,lpnL,lpnV,nXD,bNorm);                                       /*   Call computation function       */
    CData_DblockStore(A,lpnA,nB,nXD,nXD,-1);                                    /*   Store destination matrix        */
  }                                                                             /* <<                                */
  dlp_free(lpnA);                                                               /* Free destination buffer           */
  dlp_free(lpnV);                                                               /* Free eigenvector buffer           */
  dlp_free(lpnL);                                                               /* Free eigenvalue buffer            */
  return O_K;                                                                   /* All done                          */
}

/* EOF */
