/* dLabPro class CGmm (gmm)
 * - Interactive methods
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
#include "dlp_gmm.h"                                                            /* Include class header file         */

/*
 * Manual page at gmm.def
 */
INT16 CGEN_PUBLIC CGmm_SetupEx
(
  CGmm* _this,
  CData* idMean,
  CData* idCov,
  CVmap* iMmap,
  CData* idCmap,
  CData* idVar
)
{
  BOOL    bIcov = FALSE;                                                        /* Copy of /icov option              */
  INT32    c     = 0;                                                            /* Current (inv.) cov. matrix index  */
  INT32    k     = 0;                                                            /* Current Gaussian index            */
  INT32    n     = 0;                                                            /* Dimension loop counter            */
  INT32    C     = 0;                                                            /* Number of (inv.) cov. matrices    */
  INT32    K     = 0;                                                            /* Number of single Gaussians        */
  INT32    N     = 0;                                                            /* Feature space dimensionality      */
  INT32    nRB   = 0;                                                            /* Number of records per block       */
  INT32    nXC   = 0;                                                            /* Number of (numeric) components    */
  FLOAT64* lpBuf = 0;                                                            /* Pointer to data copying buffer    */
  CData*  idAux = NULL;                                                         /* Auxilary data instance #1         */
  
  /* Validate */                                                                /* --------------------------------- */
  CHECK_THIS_RV(NOT_EXEC);                                                      /* Check this pointer                */
  _this->m_nType = CGmm_GetTypeEx(_this,idMean);                                /* Auto detect floating point type   */
  if (_this->m_bDouble) _this->m_nType = T_DOUBLE;                              /* Override type by option           */
  if (_this->m_bFloat ) _this->m_nType = T_FLOAT;                               /* Override type by option           */
  bIcov = _this->m_bIcov;                                                       /* Remember /icov option             */
  K     = CData_GetNRecs(idMean);                                               /* Detect number of single Gaussians */
  N     = CData_GetNNumericComps(idMean);                                       /* Detect feature space dimension    */
  if (!K    ) return IERROR(_this,ERR_INVALARG,"idMean",0,0);                   /* There should be Gaussians ...     */
  if (!N    ) return IERROR(_this,ERR_INVALARG,"idMean",0,0);                   /* There should be dimensions ...    */
  if (!idCov) return IERROR(_this,ERR_INVALARG,"idCov" ,0,0);                   /* There should be (co-)variance data*/
  
  /* Initialize */                                                              /* --------------------------------- */
  IFIELD_RESET(CData,"mean");                                                   /* Create/reset mean vector table    */
  IFIELD_RESET(CData,"ivar");                                                   /* Create/reset inv. var. vector tab.*/
  IFIELD_RESET(CData,"cdet");                                                   /* Create/reset cov. determinant tab.*/
  IDESTROY(_this->m_idIcov);                                                    /* Destroy covarince matrices        */
  IDESTROY(_this->m_idCmap);                                                    /* Destroy covariance tying map      */
  IDESTROY(_this->m_iMmap);                                                     /* Destroy mixture map               */
  IDESTROY(_this->m_idCldet);
  
  /* Copy mean vectors */                                                       /* --------------------------------- */
  lpBuf = (FLOAT64*)dlp_calloc(N,sizeof(FLOAT64));                                /* Allocate copy buffer              */
  CData_Array(AS(CData,_this->m_idMean),_this->m_nType,N,K);                    /* Allocate mean vector table        */
  for (k=0; k<K; k++)                                                           /* Loop over Gaussians               */
  {                                                                             /* >>                                */
    CData_DrecFetch(idMean         ,lpBuf,k,N,-1);                              /*   Read one mean vector            */
    CData_DrecStore(AS(CData,_this->m_idMean),lpBuf,k,N,-1);                    /*   Write one mean vector           */
  }                                                                             /* <<                                */
  dlp_free(lpBuf);                                                              /* Free copy buffer                  */

  /* Store inverse (co-)variance data */                                        /* --------------------------------- */
  nXC = CData_GetNNumericComps(idCov);                                          /* Get numeric dimensionality        */
  C   = CData_GetNBlocks(idCov);                                                /* Get number of vectors/matrices    */
  nRB = CData_GetNRecsPerBlock(idCov);                                          /* Get row number of matrices        */
  IFCHECK printf("\n   - %ld %ldx%ld covariance matrice(s)",                    /* Protocol - verbose level 1        */
    (long)C,(long)nXC,(long)nRB);                                               /* |                                 */
  /* NO RETURNS BEYOND THIS POINT! */                                           /*                                   */
  ICREATEEX(CData,idAux,"CGmm_Setup.idAux",NULL);                               /* Create auxilary data table        */
  CData_Array(idAux,_this->m_nType,nXC,C*nRB);                                  /* Alloc. memory for copy of cov.mat.*/
  idAux->m_nblock = C;                                                          /* Set block number                  */
  lpBuf = (FLOAT64*)dlp_calloc(nXC*nRB,sizeof(FLOAT64));                        /* Alloc. copy buffer for one block  */
  for (c=0; c<C; c++)                                                           /* Loop over blocks                  */
  {                                                                             /* >>                                */
    CData_DblockFetch(idCov,lpBuf,c,nXC,nRB,-1);                                /*   Read one covariance matrix      */
    CData_DblockStore(idAux,lpBuf,c,nXC,nRB,-1);                                /*   Write one covariance matrix     */
  }                                                                             /* <<                                */
  if (nXC==1 && N>1)                                                            /* Single variance value(s)          */
  {                                                                             /* >>                                */
    CData_Array(AS(CData,_this->m_idIvar),_this->m_nType,K,N);                  /*   Allocate inv. variance vectors  */
    if (CData_GetNRecs(idCov)==1)                                               /*   One variance for all Gaussians  */
    {                                                                           /*   >>                              */
      IFCHECK printf("\n   - One variance value for all Gaussians");            /*     Protocol (verbose level 1)    */
      for (k=0; k<K; k++)                                                       /*     Loop over single Gaussians    */
        for (n=0; n<N; n++)                                                     /*       Loop over dimensions        */
          if (bIcov)                                                            /*         idCov is inverse          */
            CData_Dstore(AS(CData,_this->m_idIvar),CData_Dfetch(idCov,0,0),k,n);/*           Store inverse variance  */
          else                                                                  /*         idCov is NOT inverse      */
            CData_Dstore(AS(CData,_this->m_idIvar),1./CData_Dfetch(idCov,0,0),k,n);/*        Store inverse variance  */
    }                                                                           /*   <<                              */
    else if (CData_GetNRecs(idCov)==K)                                          /*   One variance per Gaussian       */
    {                                                                           /*   >>                              */
      IFCHECK printf("\n   - One variance value per Gaussian");                 /*     Protocol (verbose level 1)    */
      for (k=0; k<K; k++)                                                       /*     Loop over single Gaussians    */
        for (n=0; n<N; n++)                                                     /*       Loop over dimensions        */
          if (bIcov)                                                            /*         idCov is inverse          */
            CData_Dstore(AS(CData,_this->m_idIvar),CData_Dfetch(idCov,k,0),k,n);/*           Store inverse variance  */
          else                                                                  /*         idCov is NOT inverse      */
            CData_Dstore(AS(CData,_this->m_idIvar),1./CData_Dfetch(idCov,k,0),k,n);/*        Store inverse variance  */
    }                                                                           /*   <<                              */
    else                                                                        /*   Invalid # of records            */
    {                                                                           /*   >>                              */
      IERROR(_this,GMM_NOTSETUP," (wrong number of variance values)",0,0);      /*     Error message                 */
      DLPTHROW(GMM_NOTSETUP);                                                   /*     Throw exception               */
    }                                                                           /*   <<                              */
  }                                                                             /* <<                                */
  else if (nXC==N)                                                              /* Component-wise (co-)variances     */
  {                                                                             /* >>                                */
    if (nRB==1)                                                                 /*   Variance                        */
    {                                                                           /*   >>                              */
      if (idVar) IERROR(_this,GMM_IGNORE,"variance vectors (idVar)",0,0);       /*     Variances ignored!            */
      if (C==1 && C<K)                                                          /*     One shared variance vector    */
      {                                                                         /*     >>                            */
        IFCHECK printf("\n   - One variance vector for all Gaussians");         /*       Protocol (verbose level 1)  */
        IFIELD_RESET(CData,"cmap");                                             /*       Create/reset cov. tying map */
        CData_Array(AS(CData,_this->m_idCmap),T_LONG,1,K);                      /*       Allocate map                */
      }                                                                         /*     <<                            */
      else if (C==K)                                                            /*     One variance vector/Gaussian  */
      {                                                                         /*     >>                            */
        IFCHECK printf("\n   - One variance vector per Gaussian");              /*       Protocol (verbose level 1)  */
        CData_Aggregate(AS(CData,_this->m_idCdet),idCov,NULL,CMPLX(0),"prod");  /*       get cdet from variances     */
        IFIELD_RESET(CData,"cldet");
        CData_Scalop(AS(CData,_this->m_idCldet),idCov,CMPLX(0),"ln");
        CData_Aggregate(AS(CData,_this->m_idCldet),AS(CData,_this->m_idCldet),NULL,CMPLX(0),"sum");
        if (bIcov){                                                              /*       idCov is inverse            */
          CData_Copy(_this->m_idIvar,BASEINST(idCov));                          /*         Copy inv. var. vectors    */
          CData_Scalop(AS(CData,_this->m_idCdet),AS(CData,_this->m_idCdet),     /*         invert cdet               */
            CMPLX(0),".inv");                                                   /*         |                         */
          CData_Scalop(AS(CData,_this->m_idCldet),AS(CData,_this->m_idCldet),   /*         invert cdet               */
            CMPLX(0),"neg");                                                    /*         |                         */
        }else                                                                    /*       idCov is NOT inverse        */
          CData_Scalop(AS(CData,_this->m_idIvar),idCov,CMPLX(0),"inv");         /*         Invert variance vectors   */
        CData_Tconvert(AS(CData,_this->m_idCdet),AS(CData,_this->m_idCdet),     /*         Convert type of cdet      */
            _this->m_nType);                                                    /*         |                         */
        CData_Tconvert(AS(CData,_this->m_idIvar),AS(CData,_this->m_idIvar),     /*         Convert type of ivar      */
            _this->m_nType);                                                    /*         |                         */
        CData_Tconvert(AS(CData,_this->m_idCldet),AS(CData,_this->m_idCldet),   /*         Convert type of cldet     */
            _this->m_nType);                                                    /*         |                         */
      }                                                                         /*     <<                            */
      else                                                                      /*     Shared covariance matrices    */
      {                                                                         /*     >>                            */
        IFCHECK printf("\n   - Tied variance vectors");                         /*       Protocol (verbose level 1)  */
        if (!idCmap)                                                            /*       Need tying map in this mode */
        {                                                                       /*       >>                          */
          IERROR(_this,GMM_NOTSETUP,                                            /*         Error message             */
            " (wrong no. of variance vectors or need tying map)",0,0);          /*         |                         */
          DLPTHROW(GMM_NOTSETUP);                                               /*         Throw exception           */
        }                                                                       /*       <<                          */
        IFIELD_RESET(CData,"cmap");                                             /*       Create/reset cov. tying map */
        CData_Copy(_this->m_idCmap,BASEINST(idCmap));                           /*       Copy covariance tying map   */
        IF_NOK(CGmm_CheckCmap(_this))                                           /*       Check the map               */
        {                                                                       /*       >> (Check failed)           */
          IERROR(_this,GMM_NOTSETUP," (variance tying map corrupt)",0,0);       /*         Error message             */
          DLPTHROW(GMM_NOTSETUP);                                               /*         Throw exception           */
        }                                                                       /*       <<                          */
      }                                                                         /*     <<                            */
      if (C<K)                                                                  /*     1<=C<K -> variance tying modes*/
      {                                                                         /*     >>                            */
        if (idCmap) IERROR(_this,GMM_IGNORE,"variance tying map (idCmap)",0,0); /*       Cov. tying map ignored!     */
        if (bIcov)                                                              /*       idCov is inverse            */
          CData_Copy(_this->m_idIvar,BASEINST(idCov));                          /*         Copy inv. var. vectors    */
        else                                                                    /*       idCov is NOT inverse        */
          CData_Scalop(idAux,idCov,CMPLX(0),"inv");                             /*         Invert variance vectors   */
        CData_Lookup(AS(CData,_this->m_idIvar),AS(CData,_this->m_idCmap),       /*       Pass through cov. tying map */
          0,idAux,0,N);                                                         /*       |                           */
      }                                                                         /*     <<                            */
      IDESTROY(_this->m_idCmap);                                                /*     Clear covariance tying map    */
    }                                                                           /*   <<                              */
    else if (nRB==N)                                                            /*   Covariance matrices             */
    {                                                                           /*   >>                              */
      IFIELD_RESET(CData,"icov");                                               /*     Create inv. cov. matrix set   */
      CGmm_Icov(_this,idAux,bIcov);                                             /*     Invert covariance matrices    */
      if (C==1 && C<K)                                                          /*     One shared covariance matrix  */
      {                                                                         /*     >>                            */
        IFCHECK printf("\n   - One full covariance matrix for all Gaussians");  /*       Protocol (verbose level 1)  */
        if(idCmap) IERROR(_this,GMM_IGNORE,"covariance tying map (idCmap)",0,0);/*       Cov. tying map ignored!     */
        IFIELD_RESET(CData,"cmap");                                             /*       Create/reset cov. tying map */
        CData_Array(AS(CData,_this->m_idCmap),T_LONG,1,K);                      /*       Allocate map                */
      }                                                                         /*     <<                            */
      else if (C==K)                                                            /*     One covariance matrix/Gaussian*/
      {                                                                         /*     >>                            */
        IFCHECK printf("\n   - One full covariance matrix per Gaussian");       /*       Protocol (verbose level 1)  */
        if(idVar ) IERROR(_this,GMM_IGNORE,"variance vectors (idVar)",0,0);     /*       Variances ignored!          */
        if(idCmap) IERROR(_this,GMM_IGNORE,"covariance tying map (idCmap)",0,0);/*       Cov. tying map ignored!     */
      }                                                                         /*     <<                            */
      else                                                                      /*     Shared covariance matrices    */
      {                                                                         /*     >>                            */
        IFCHECK printf("\n   - Tied full covariance matrices");                 /*       Protocol (verbose level 1)  */
        if (!idCmap)                                                            /*       Need tying map in this mode */
        {                                                                       /*       >>                          */
          IERROR(_this,GMM_NOTSETUP,                                            /*         Error message             */
            " (wrong no. of cov. matrices or need tying map)",0,0);             /*         |                         */
          DLPTHROW(GMM_NOTSETUP);                                               /*         Throw exception           */
        }                                                                       /*       <<                          */
        IFIELD_RESET(CData,"cmap");                                             /*       Create/reset cov. tying map */
        CData_Copy(_this->m_idCmap,BASEINST(idCmap));                           /*       Copy covariance tying map   */
        IF_NOK(CGmm_CheckCmap(_this))                                           /*       Check the map               */
        {                                                                       /*       >> (Check failed)           */
          IERROR(_this,GMM_NOTSETUP," (covariance tying map corrupt)",0,0);     /*         Error message             */
          DLPTHROW(GMM_NOTSETUP);                                               /*         Throw exception           */
        }                                                                       /*       <<                          */
      }                                                                         /*     <<                            */
      if (C<K)                                                                  /*     1<=C<K -> cov. tying modes    */
      {                                                                         /*     >>                            */
        if (!CData_IsEmpty(idVar))                                              /*       Have individual var.vects.? */
        {                                                                       /*       <<                          */
          if (bIcov)                                                            /*         idVar is inverse          */
            CData_Copy(_this->m_idIvar,BASEINST(idVar));                        /*           Copy inv. var. vectors  */
          else                                                                  /*         idVar is NOT inverse      */
            CData_Scalop(AS(CData,_this->m_idIvar),idVar,CMPLX(0),"inv");       /*           Invert variance vectors */
          CData_SetDescr(AS(CData,_this->m_idIvar),0,1.);                       /*         Indicate individual       */
        }                                                                       /*       >>                          */
        else                                                                    /*       No individual var. vectors  */
        {                                                                       /*       >>                          */
          CData_Copy(BASEINST(idAux),_this->m_idIvar);                          /*         Pass inv. var. vectors ...*/
          CData_Lookup(AS(CData,_this->m_idIvar),AS(CData,_this->m_idCmap),     /*         ... through cov. tying map*/
            0,idAux,0,N);                                                       /*         |                         */
          CData_SetDescr(AS(CData,_this->m_idIvar),0,0.);                       /*         Indicate NOT individual   */
        }                                                                       /*       <<                          */
      }                                                                         /*     <<                            */
    }                                                                           /*   <<                              */
    else                                                                        /*   Invalid # records per block     */
    {                                                                           /*   >>                              */
      IERROR(_this,GMM_NOTSETUP," (wrong dimension of covariance matrices)",0,  /*     Error message                 */
        0);                                                                     /*     |                             */
      DLPTHROW(GMM_NOTSETUP);                                                   /*     Throw exception               */
    }                                                                           /*   <<                              */
  }                                                                             /* <<                                */
  else                                                                          /* Invalid # of components           */
  {                                                                             /* >>                                */
    IERROR(_this,GMM_NOTSETUP," (wrong dimension of covariance matrices)",0,0); /*   Error message                   */
    DLPTHROW(GMM_NOTSETUP);                                                     /*   Throw exception                 */
  }                                                                             /* <<                                */
  dlp_free(lpBuf);                                                              /* Free copy buffer                  */
  IDESTROY(idAux);                                                              /* Destroy auxilary data table       */

  /* Store mixture map */                                                       /* --------------------------------- */
  if (iMmap)                                                                    /* Map passed?                       */
  {                                                                             /* >>                                */
    IFIELD_RESET(CVmap,"mmap");                                                 /*   Create/reset mixture map        */
    CVmap_Copy(_this->m_iMmap,BASEINST(iMmap));                                 /*   Copy mixture map                */
    IF_NOK(CGmm_CheckMmap(_this))                                               /*   Check the map                   */
    {                                                                           /*   >> (Check failed)               */
      IERROR(_this,GMM_NOTSETUP," (mixture map corrupt)",0,0);                  /*     Error message                 */
      DLPTHROW(GMM_NOTSETUP);                                                   /*     Throw exception               */
    }                                                                           /*   <<                              */
  }                                                                             /* <<                                */
  
  /* Final actions */                                                           /* --------------------------------- */
  _this->m_nLDL = _this->m_bLdl && _this->m_idIcov;                             /* Get value of option ldl           */
  IF_NOK(CGmm_Precalc(_this,FALSE))                                             /* Precalculate                      */
  {                                                                             /* >> (if failed)                    */
    CGmm_Reset(BASEINST(_this),TRUE);                                           /*   Reset this instance             */
    return IERROR(_this,GMM_NOTSETUP," (CGmm_Precalc failed)",0,0);             /*   Error message and return        */
  }                                                                             /* <<                                */
  return O_K;                                                                   /* Everything went right             */
  
DLPCATCH(GMM_NOTSETUP)                                                          /* == Catch GMM_NOTSETUP exception   */
  dlp_free(lpBuf);                                                              /* Free copy buffer                  */
  IDESTROY(idAux);                                                              /* Destroy auxilary data table       */
  CGmm_Reset(BASEINST(_this),TRUE);                                             /* Reset this instance               */
  return NOT_EXEC;                                                              /* Return error code                 */
}

/*
 * Manual page at gmm.def
 */
INT16 CGEN_PUBLIC CGmm_Extract(CGmm* _this,  CData* idMean, CData* idIcov)
{
  INT32 c = 0;                                                                   /* Covariance matrix index           */
  INT32 i = 0;                                                                   /* Triangular inv. cov. matrix cntr. */
  INT32 k = 0;                                                                   /* Single Gaussian loop index        */
  INT32 n = 0;                                                                   /* Feature vector comp. loop index   */
  INT32 m = 0;                                                                   /* Feature vector comp. loop index   */
  INT32 N = 0;                                                                   /* Feature space dimensionality      */
  INT32 K = 0;                                                                   /* Number of single Gaussians        */

  CHECK_THIS_RV(NOT_EXEC);                                                      /* Check this pointer                */
  if (!CGmm_CheckMean(_this))                                                   /* Check mean vectors                */
    return IERROR(_this,GMM_NOTSETUP," (mean vectors missing or corrupt)",0,0); /* |                                 */
  if (!CGmm_CheckIvar(_this))                                                   /* Check inverse variance vectors    */
    return                                                                      /* |                                 */
      IERROR(_this,GMM_NOTSETUP,                                                /* |                                 */
        " (inverse variance vectors missing or corrupt)",0,0);                  /* |                                 */
  
  CData_Copy(BASEINST(idMean),_this->m_idMean);                                 /* Copy mean vectors                 */
  if (!idIcov) return O_K;                                                      /* If no inv. covs. wanted that's it */
  
  N = CGmm_GetDim(_this);                                                       /* Get feature space dimensionality  */
  K = CGmm_GetNGauss(_this);                                                    /* Get number of single Gaussians    */
  CData_Array(idIcov,_this->m_nType,N,K*N);                                     /* Allocate inverse covariance matr. */
  CData_SetNBlocks(idIcov,K);                                                   /* Set number of blocks              */
  for (k=0; k<K; k++)                                                           /* Loop over single Gaussians        */
    for (n=0; n<N; n++)                                                         /*   Loops over dimensions           */
      CData_Dstore(idIcov,CData_Dfetch(AS(CData,_this->m_idIvar),k,n),k*N+n,n); /*     Copy inverse variances        */
  if (!_this->m_idIcov) return O_K;                                             /* No inv. cov. matr. -> that's it   */
  
  if (!CGmm_CheckIcov(_this))                                                   /* Check inverse covariance matr.    */
    return                                                                      /* |                                 */
      IERROR(_this,GMM_NOTSETUP," (inverse covariance matrices corrupt)",0,0);  /* |                                 */
  for (k=0; k<K; k++)                                                           /* Loop over single Gaussians        */
  {                                                                             /* >>                                */
    c = _this->m_idCmap ? (INT32)CData_Dfetch(AS(CData,_this->m_idCmap),k,0) : k;/*   Cov. mat. idx. for Gaussian k   */
    for (n=0; n<N; n++)                                                         /*   Loop over dimensions            */
      for (m=0; m<N; m++)                                                       /*     Loop over dimensions          */
      {                                                                         /*     >>                            */
        if (m==n) continue;                                                     /*       Can't get no var's. here    */
        if (m>n) i = n*N - 2*n - n*(n-1)/2 + m - 1;                             /*       Calc index if I[n,m] in     */
        else     i = m*N - 2*m - m*(m-1)/2 + n - 1;                             /*       | triangular icov. matrix   */
        CData_Dstore(idIcov,CData_Dfetch(AS(CData,_this->m_idIcov),c,i),k*N+n,m);/*      Copy inverse covariances    */
      }                                                                          /*     <<                            */
  }                                                                             /* <<                                */
  return O_K;                                                                   /* Yo says DJ Tonne                  */
}

/* EOF */
