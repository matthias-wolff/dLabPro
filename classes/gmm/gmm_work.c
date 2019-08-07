/* dLabPro class CGmm (gmm)
 * - Worker methods
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
#include "dlp_math.h"                                                           /* Include math header file          */
#define  GMM_FTYPE_CODE T_FLOAT                                                 /* Include float core methods        */
#include "gmm_core.c"                                                           /* |                                 */
#undef   GMM_FTYPE_CODE                                                         /* |                                 */
#define  GMM_FTYPE_CODE T_DOUBLE                                                /* Include double core methods       */
#include "gmm_core.c"                                                           /* |                                 */
#undef   GMM_FTYPE_CODE                                                         /* |                                 */
#ifdef GMM_SSE2                                                                 /* Include SSE2 opt'd core methods   */
#include "gmm_core_sse2.c"                                                      /* |                                 */
#endif                                                                          /* |                                 */

/* Clip to minimum/maximum macros */
#define CLIP_MAX(TYPE,A,B,C,D) \
  if (*(TYPE*)A>(TYPE)(B)) *(TYPE*)A=(TYPE)(B); \
  if (*(TYPE*)A<(TYPE)(C)) {*(TYPE*)A=(TYPE)(C); if(D)*D=TRUE;}
#define CLIP_MIN(TYPE,A,B,C,D) \
  if (*(TYPE*)A<(TYPE)(B)) *(TYPE*)A=(TYPE)(B); \
  if (*(TYPE*)A>(TYPE)(C)) {*(TYPE*)A=(TYPE)(C); if(D)*D=TRUE;}

/**
 * Determines if the current platform supports the SSE2 instruction set.
 *
 * @return <code>TRUE</code> SSE2 is available, <code>FALSE</code> otherwise
 */
BOOL CGEN_SPUBLIC CGmm_Sse2()
{
#ifdef GMM_SSE2
  return sse2_ok();
#else
  return FALSE;
#endif
}

/**
 * <p>Inverts a set of covariance matrices and calculates their determinants and
 * ranks. The method overwrites {@link icov m_idIcov} with the inverse
 * covariances, {@link ivar m_idIvar} with the inverse variances and
 * {@link cdet m_idCdet} with the determiniants of the covariance matrices.</p>
 * <p><code>idCov</code> may also contain inverse convariance matrices. In this
 * case the parameter <code>bIcov</code> <em>must</em> be <code>TRUE</code>.</p>
 * <h4>Remarks</h4>
 * <ul>
 *   <li>Only the upper right triangles of the inverse covariance matrices will
 *   be stored. {@link icov m_idIcov} will contain <i>C</i> records and
 *   <i>N</i>*(<i>N</i>-1)/2 components, where <i>C</i> stands for the number
 *   of blocks and <i>N</i> for the number of compoents in <code>idCov</code>
 *   (see field {@link icov m_idIcov}).</li>
 * </ul>
 *
 * @param _this
 *          Pointer to CGmm instance
 * @param idCov
 *          Set of (inverse) covariance matrices, see <code>bIcov</code>
 * @param bIcov
 *          Commit <code>TRUE</code> of <code>idCov</code> contains inverse
 *          covariance matrices and <code>FALSE</code> if <code>idCov</code>
 *          contains "normal" covariance matrices.
 * @return The number of covariance matrices of full rank
 */
INT32 CGEN_PROTECTED CGmm_Icov(CGmm* _this, CData* idCov, BOOL bIcov)
{
  INT32   c      = 0;                                                            /* Current covariance matrix         */
  INT32   i      = 0;                                                            /* Triangular inv. cov. matrix cntr. */
  INT32   n      = 0;                                                            /* Feature component counter (rows)  */
  INT32   m      = 0;                                                            /* Feature component counter (cols)  */
  INT32   N      = 0;                                                            /* Feature space dimensionality      */
  INT32   C      = 0;                                                            /* Number of covariance matrices     */
  INT32   nValid = 0;                                                            /* Number of full ranked matrices    */
  CData* idIcov = NULL;                                                         /* Auxilary table for inv. cov. matr.*/
  CData* idRank = NULL;                                                         /* Auxilary table for ranks          */

  CHECK_THIS_RV(0);                                                             /* Check this porinter               */
  DLPASSERT(idCov);                                                             /* Must have covariance matrices     */
  DLPASSERT(_this->m_idIcov);                                                   /* Must have inv. covariance table   */
  DLPASSERT(_this->m_idIvar);                                                   /* Must have inv. variance table     */
  DLPASSERT(_this->m_idCdet);                                                   /* Must have determinants table      */

  ICREATEEX(CData,idRank,"CGmm_Icov.idRank",NULL);                              /* Create rank table                 */
  ICREATEEX(CData,idIcov,"CGmm_Icov.idIcov",NULL);                              /* Create inverse cov. matrices      */
  N = CData_GetNNumericComps(idCov);                                            /* Get feature space dimensionality  */
  C = CData_GetNBlocks(idCov);                                                  /* Get number of covariance matrices */
  CMatrix_Invert(idCov,idIcov,AS(CData,_this->m_idCdet));                       /* Inv. cov. mats. (MUST do for det!)*/
  if (bIcov) CData_Copy(BASEINST(idIcov),BASEINST(idCov));                      /* idCov was already inverse :)      */
  for (c=0; c<C; c++)                                                           /* Loop over matrices                */
  {                                                                             /* >>                                */
    if (bIcov)                                                                  /*   idCov was already inverse       */
      CData_Dstore(AS(CData,_this->m_idCdet),                                   /*     det(inv(A))=1./det(A)!        */
        1./CData_Dfetch(AS(CData,_this->m_idCdet),c,0),c,0);                    /*     |                             */
    CData_Dstore(AS(CData,_this->m_idCdet),                                     /*     abs of det                    */
      fabs(CData_Dfetch(AS(CData,_this->m_idCdet),c,0)),c,0);                   /*     |                             */
    if                                                                          /*   Cov. matrix has no full rank    */
    (                                                                           /*   |                               */
      CData_Dfetch(AS(CData,_this->m_idCdet),c,0) < _this->m_nMindet   ||       /*   | (Det. is unreasonably small)  */
      CData_Dfetch(AS(CData,_this->m_idCdet),c,0) > 1./_this->m_nMindet         /*   | (Det. is unreasonably large)  */
    )                                                                           /*   |                               */
    {                                                                           /*   >>                              */
      IERROR(_this,GMM_RANK,c,CData_Dfetch(AS(CData,_this->m_idCdet),c,0),0);   /*     Warning message               */
      CData_Dstore(AS(CData,_this->m_idCdet),0.,c,0);                           /*     Set determinant exactly to 0  */
    }                                                                           /*   <<                              */
    else                                                                        /*   Cov.matrix successfully inverted*/
      nValid++;                                                                 /*     Count valid Gaussians         */
  }                                                                             /* <<                                */
  CData_Array(AS(CData,_this->m_idIcov),_this->m_nType,N*(N-1)/2,C);            /* Allocate triangular inv. cov. tab.*/
  CData_Array(AS(CData,_this->m_idIvar),_this->m_nType,N,C);                    /* Allocate inv. variance vectors    */
  for (c=0; c<C; c++)                                                           /* Loop over inv. cov. matrices      */
  {                                                                             /* >>                                */
    for (i=0,n=0; n<N-1; n++)                                                   /*   Loop over upper right triangle  */
      for (m=n+1; m<N; m++,i++)                                                 /*   |                               */
        CData_Dstore(AS(CData,_this->m_idIcov),CData_Dfetch(idIcov,c*N+n,m),c,i);/*    Copy inverse covariances      */
    for (n=0; n<N; n++)                                                         /*   Loop over diagonal              */
      CData_Dstore(AS(CData,_this->m_idIvar),CData_Dfetch(idIcov,c*N+n,n),c,n); /*     Copy inverse variances        */
  }                                                                             /* <<                                */
  if (CGmm_GetTypeEx(_this,AS(CData,_this->m_idCdet))!=_this->m_nType)          /* Convert type of determinants      */
    CData_Tconvert(AS(CData,_this->m_idCdet),AS(CData,_this->m_idCdet),         /* |                                 */
      _this->m_nType);                                                          /* |                                 */
  CData_SetDescr(AS(CData,_this->m_idIvar),0,0.);                               /* Indicate NO individual inv. vars. */
  IDESTROY(idIcov);                                                             /* Destroy auxilary inv. cov. matr.  */
  IDESTROY(idRank);                                                             /* Destroy auxilary rank table       */
  return nValid;                                                                /* Return no. of valid matrices      */
}

/**
 * Clears the gamma vector buffer (see manual).
 * @param _this
 *          Pointer to GMM instance
 */
void CGEN_PRIVATE CGmm_ClearGamma(CGmm* _this)
{
  INT32 k = 0;                                                                   /* Current single Gaussian index     */
  INT32 K = 0;                                                                   /* Number of Gaussians               */
  if (!_this->m_lpGamma) return;                                                /* No gammas -> nothing to be done   */
  K = CGmm_GetNGauss(_this);                                                    /* Get number of single Gaussians    */
  DLPASSERT(                                                                    /* Check gamma buffer size           */
    (INT32)(dlp_size(_this->m_lpGamma)/dlp_get_type_size(_this->m_nType))==K);   /* |                                 */
  if (_this->m_nType==T_FLOAT)                                                  /* Single precision mode             */
    for (k=0; k<K; k++) ((FLOAT32*)_this->m_lpGamma)[k] = T_FLOAT_MAX;            /*   Initialize gamma buffer         */
  else if (_this->m_nType==T_DOUBLE)                                            /* Double precision mode             */
    for (k=0; k<K; k++) ((FLOAT64*)_this->m_lpGamma)[k] = T_DOUBLE_MAX;          /*   Initialize gamma buffer         */
  else DLPASSERT(FMSG("Invalid floating point type"));                          /* Cannot happen                     */
}

/**
 * Precalculates feature vector independent parts of density/Mahalanobis
 * distance computation. The precalculation comprises the class dependent A and
 * B vectors (see manual) and an index of the inverse covariance matrices
 * (field m_idIcov).
 *
 * @param _this
 *          Pointer to GMM instance
 * @param bCleanup
 *          If <code>TRUE</code> the method will only free the memory of the
 *          precalculated data
 * @return <code>O_K</code> if successfull, a (negative) error code otherwise.
 */
INT16 CGEN_PROTECTED CGmm_Precalc(CGmm* _this, BOOL bCleanup)
{
  CHECK_THIS_RV(NOT_EXEC);
  if (!bCleanup) DLPASSERT(_this->m_nType==CGmm_GetType(_this));
  if (_this->m_nType==T_FLOAT ) return CGmm_PrecalcF(_this,bCleanup);
  if (_this->m_nType==T_DOUBLE) return CGmm_PrecalcD(_this,bCleanup);
  DLPASSERT(FALSE);
  return NOT_EXEC;

}

/**
 * <p>Calculates the Mahalanobis distances or the (logarithmic) probability
 * densities of a set of feature vectors for all Gaussians.</p>
 */
INT16 CGEN_PROTECTED CGmm_Gauss
(
  CGmm*  _this,
  CData* idX,
  CData* idXmap,
  CData* idDest,
  INT16  nMode
)
{
  INT32   i        = 0;                                                          /* Feature vector index              */
  INT32   k        = 0;                                                          /* Single Gaussian index             */
  INT32   n        = 0;                                                          /* Feature vecotr component index    */
  INT32   I        = 0;                                                          /* Number of feature vectors         */
  INT32   K        = 0;                                                          /* Number of single Gaussians        */
  INT32   N        = 0;                                                          /* Feature space dimensionality      */
  INT32   M        = 0;                                                          /* Number of mixture models          */
  INT32   c        = 0;                                                          /* Current component in input record */
  INT32   C        = 0;                                                          /* No. of components in input record */
  void*  lpX      = NULL;                                                       /* Feature/input vector buffer       */
  BYTE*  lpDest   = NULL;                                                       /* Destination write pointer         */
  BYTE*  lpXmap   = NULL;                                                       /* Gaussian computation map          */
  INT32   nDestInc = 0;                                                          /* Destination write ptr. increment  */
  FLOAT64 nLm1     = 0.;                                                         /* Lower distance limit              */
  FLOAT64 nLm2     = 0.;                                                         /* Upper distance limit              */
  BOOL*  lpbErr   = NULL;                                                       /* Invalid density/distance detected */
  BOOL   bMhomo   = FALSE;                                                      /* Homogeneous Gaussian comp. map    */
  BOOL   bXhomo   = FALSE;                                                      /* Homogeneous feature vectors       */

  /* Validate */                                                                /* --------------------------------- */
  CHECK_THIS_RV(NOT_EXEC);                                                      /* Check this pointer                */
  if (!idDest) return IERROR(_this,ERR_NULLARG,"idDest",0,0);                   /* Dest. instance must not be NULL   */
  IF_NOK(CGmm_Check(_this))                                                     /* Check GMM setup                   */
    return IERROR(_this,GMM_NOTSETUP," (-status for details)",0,0);             /* |                                 */
  if (nMode<0 || nMode>3)                                                       /* Check operation mode              */
    return IERROR(_this,ERR_EXCEPTION,-1,__FILE__,__LINE__);                    /* |                                 */
  I = CData_GetNRecs(idX);                                                      /* Get number of feature vectors     */
  N = CGmm_GetDim(_this);                                                       /* Get feature space dimensionality  */
  M = CGmm_GetNMix(_this);                                                      /* Get number of mixtures            */
  K = CGmm_GetNGauss(_this);                                                    /* Get number of single Gaussians    */
  C = CData_GetNComps(idX);                                                     /* Get number of input components    */
  if (CData_GetNNumericComps(idX)!=N)                                           /* Check feature dimensionality      */
    return IERROR(_this,GMM_DIM,"feature vector",N,0);                          /* |                                 */

  /* Initialize */                                                              /* --------------------------------- */
  CREATEVIRTUAL(CData,idX,idDest);                                              /* idX and idDest may be identical   */
  CData_Array(idDest,_this->m_nType,K,CData_GetNRecs(idX));                     /* Allocate destination instance     */
  lpDest   = (BYTE*)CData_XAddr(idDest,0,0);                                    /* Initialize dest. write pointer    */
  nDestInc = CData_GetCompSize(idDest,0);                                       /* Get dest. write ptr. increment    */
  lpX      = dlp_calloc(N,dlp_get_type_size(_this->m_nType));                   /* Allocate feature vector buffer    */

  /* Initialize - Check if idX is homogeneously of type _this->m_nType */       /* --------------------------------- */
  for (c=0,bXhomo=TRUE; c<N; c++)                                               /* Loop over vector components       */
    if (CData_GetCompType(idX,c)!=_this->m_nType)                               /*   Component type is not m_nType   */
      bXhomo = FALSE;                                                           /*     :'(                           */

  /* Initialize - Prepare Gaussian computation map */                           /* --------------------------------- */
  if (idXmap)                                                                   /* Have Gaussian computation map?    */
  {                                                                             /* >> (Yes)                          */
    for (k=0,bMhomo=TRUE; k<K; k++)                                             /*   Loop over Gaussians             */
      if (CData_GetCompType(idXmap,k)!=T_UCHAR)                                 /*     Gaussian map comp. not BYTE   */
        bMhomo = FALSE;                                                         /*       :'(                         */
    bMhomo &= CData_GetNRecs(idXmap)==I && CData_GetNComps(idXmap)==K;          /*   Right no. of recs. and comps.?  */
    if (bMhomo) lpXmap = CData_XAddr(idXmap,0,0);                               /*   Quick map reading possible :))  */
  }                                                                             /* <<                                */

  /* Compute distances/densities */                                             /* --------------------------------- */
  for (i=0; i<I; i++)                                                           /* Loop over feature vectors         */
  {                                                                             /* >>                                */
#ifdef __TMS
    if((i+1)%25==0) printf(".");
#endif
    CGmm_ClearGamma(_this);                                                     /*   Clear lazy computation buffer   */
    if (_this->m_nType==T_FLOAT)                                                /*   Single precision mode           */
    {                                                                           /*   >>                              */
      if (bXhomo)                                                               /*     Homogeneous FLOAT32 vectors     */
        dlp_memmove(lpX,CData_XAddr(idX,i,0),N*sizeof(FLOAT32));                  /*       Copy feature vector         */
      else                                                                      /*     Inhomogeneous vectors         */
        for (n=0,c=0; n<N && c<C; c++)                                          /*       Loop over input record      */
          if (dlp_is_numeric_type_code(CData_GetCompType(idX,c)))               /*         Current comp. numeric?    */
          {                                                                     /*         >> (Yes)                  */
            ((FLOAT32*)lpX)[n] = (FLOAT32)CData_Dfetch(idX,i,c);                    /*           Fetch float value       */
            n++;                                                                /*           Increment dimension ctr.*/
          }                                                                     /*         <<                        */
      for (k=0; k<K; k++,lpDest+=nDestInc)                                      /*     Loop over single Gaussians    */
        if (!idXmap || (lpXmap ? lpXmap[i*K+k] : CData_Dfetch(idXmap,i,k)!=0.)) /*     Shall compute N(i,k) ?        */
        {                                                                       /*     >>                            */
#ifdef GMM_SSE2                                                                 /* Have SSE2?                        */
          if (_this->m_bSse2)                                                   /*       Use SSE2                    */
            *(FLOAT32*)lpDest = CGmm_GaussF_SSE2(_this,(FLOAT32*)lpX,k,nMode);      /*         Compute distance/density  */
          else                                                                  /*       Don't use SSE2              */
            *(FLOAT32*)lpDest = CGmm_GaussF(_this,(FLOAT32*)lpX,k,nMode);           /*         Compute distance/density  */
#else                                                                           /* #ifdef GMM_SSE2                   */
          *(FLOAT32*)lpDest = CGmm_GaussF(_this,(FLOAT32*)lpX,k,nMode);             /*       Compute distance/density    */
#endif                                                                          /* #ifdef GMM_SSE2                   */
        }                                                                       /*     <<                            */
        else switch (nMode)                                                     /*     else just store limit values  */
        {                                                                       /*     >>                            */
          case GMMG_MDIST : *(FLOAT32*)lpDest=     (FLOAT32)_this->m_nDceil; break; /*       Maximal distance            */
          case GMMG_LDENS : *(FLOAT32*)lpDest=-0.5*(FLOAT32)_this->m_nDceil; break; /*       Minimal log. density        */
          case GMMG_NLDENS: *(FLOAT32*)lpDest= 0.5*(FLOAT32)_this->m_nDceil; break; /*       Maximal neg. log. density   */
        }                                                                       /*     <<                            */
    }                                                                           /*   <<                              */
    else                                                                        /*   Double precision mode           */
    {                                                                           /*   >>                              */
      if (bXhomo)                                                               /*     Homogeneous double vectors    */
        dlp_memmove(lpX,CData_XAddr(idX,i,0),N*sizeof(FLOAT64));                 /*       Copy feature vector         */
      else                                                                      /*     Inhomogeneous vectors         */
        for (n=0,c=0; n<N && c<C; c++)                                          /*       Loop over input record      */
          if (dlp_is_numeric_type_code(CData_GetCompType(idX,c)))               /*         Current comp. numeric?    */
          {                                                                     /*         >> (Yes)                  */
            ((FLOAT64*)lpX)[n] = CData_Dfetch(idX,i,c);                          /*           Fetch double value      */
            n++;                                                                /*           Increment dimension ctr.*/
          }                                                                     /*         <<                        */
      for (k=0; k<K; k++,lpDest+=nDestInc)                                      /*     Loop over single Gaussians    */
      {
        if (!idXmap || (lpXmap ? lpXmap[i*K+k] : CData_Dfetch(idXmap,i,k)!=0.)) /*     Shall compute N(i,k) ?        */
        {                                                                       /*     >>                            */
#ifdef GMM_SSE2                                                                 /* Have SSE2?                        */
          if (_this->m_bSse2)                                                   /*       Use SSE2                    */
            *(FLOAT64*)lpDest = CGmm_GaussD_SSE2(_this,(FLOAT64*)lpX,k,nMode);    /*         Compute distance/density  */
          else                                                                  /*       Don't use SSE2              */
            *(FLOAT64*)lpDest = CGmm_GaussD(_this,(FLOAT64*)lpX,k,nMode);         /*         Compute distance/density  */
#else                                                                           /* #ifdef GMM_SSE2                   */
          *(FLOAT64*)lpDest = CGmm_GaussD(_this,(FLOAT64*)lpX,k,nMode);           /*         Compute distance/density  */
#endif                                                                          /* #ifdef GMM_SSE2                   */
        }                                                                       /*     <<                            */
        else switch (nMode)                                                     /*     else just store limit values  */
        {                                                                       /*     >>                            */
          case GMMG_MDIST : *(FLOAT64*)lpDest =      _this->m_nDceil; break;     /*       Maximal distance            */
          case GMMG_LDENS : *(FLOAT64*)lpDest = -0.5*_this->m_nDceil; break;     /*       Minimal log. density        */
          case GMMG_NLDENS: *(FLOAT64*)lpDest =  0.5*_this->m_nDceil; break;     /*       Maximal neg. log. density   */
        }                                                                       /*     <<                            */
      }
    }                                                                           /*   <<                              */
  }                                                                             /* <<                                */
  dlp_free(lpX);                                                                /* Free feature vector buffer        */

  /* Map idDest through mixture map */                                          /* --------------------------------- */
  if (_this->m_iMmap && !_this->m_bNomix)                                       /* Use mixture map?                  */
  {                                                                             /* >>                                */
    INT32 nRecLen = CData_GetRecLen(idDest);                                     /*   Length of one record            */
    INT32 nYSize  = nRecLen/K*M;                                                 /*   Size of output vector           */
    BYTE *lpX    = (BYTE*)CData_XAddr(idDest,0,0);                              /*   Input vector pointer            */
    BYTE *lpY    = (BYTE*)dlp_malloc(nYSize);                                   /*   Output vector buffer            */
    for (i=0; i<I; i++, lpX+=nRecLen)                                           /*   Loop over dist./dens. vectors   */
    {                                                                           /*   >>                              */
      CVmap_MapVector(AS(CVmap,_this->m_iMmap),lpX,lpY,K,M,_this->m_nType);     /*     Map current record            */
      memcpy(lpX,lpY,nYSize);                                                   /*     Copy result over input vector */
    }                                                                           /*   <<                              */
    dlp_free(lpY);                                                              /*   Free output vector buffer       */
    CData_DeleteComps(idDest,M,K-M);                                            /*   Delete supernumerary components */
  }else M=K;                                                                    /* <<                                */

  /* Implement limits */                                                        /* --------------------------------- */
  lpDest = CData_XAddr(idDest,0,0);                                             /* Initialize destination write ptr. */
  nLm1   = CGmm_GetLimit(_this,nMode);                                          /* Get upper distance limit          */
  nLm2   = (nMode==GMMG_DENS||nMode==GMMG_LDENS) ? T_DOUBLE_MAX : T_DOUBLE_MIN; /* TODO: Any reasonable bounds?      */
  lpbErr = (BOOL*)dlp_calloc(M,sizeof(BOOL));                                   /* Allocate clipped Gaussians flags  */
  for (i=0; i<I; i++)                                                           /*   Loop over dist./dens. vectors   */
    for (k=0; k<M; k++,lpDest+=nDestInc)                                        /*     Loop over vector components   */
      if (_this->m_nType==T_FLOAT)                                              /*       Single precision float      */
        switch (nMode)                                                          /*         Branch for comp. mode     */
        {                                                                       /*         >>                        */
          case GMMG_MDIST : CLIP_MAX(FLOAT32 ,lpDest,nLm1,nLm2,&lpbErr[k]); break;/*           Limit distance          */
          case GMMG_LDENS : CLIP_MIN(FLOAT32 ,lpDest,nLm1,nLm2,&lpbErr[k]); break;/*           Limit log.prob. density */
          case GMMG_NLDENS: CLIP_MAX(FLOAT32 ,lpDest,nLm1,nLm2,&lpbErr[k]); break;/*           Limit neg.log.prob.dens.*/
          case GMMG_DENS  : CLIP_MIN(FLOAT32 ,lpDest,nLm1,nLm2,&lpbErr[k]); break;/*           Limit prob. density     */
        }                                                                       /*         <<                        */
      else                                                                      /*       Double precision float      */
        switch (nMode)                                                          /*         Branch for comp. mode     */
        {                                                                       /*         >>                        */
          case GMMG_MDIST : CLIP_MAX(FLOAT64,lpDest,nLm1,nLm2,&lpbErr[k]); break;/*           Limit distance          */
          case GMMG_LDENS : CLIP_MIN(FLOAT64,lpDest,nLm1,nLm2,&lpbErr[k]); break;/*           Limit log.prob. density */
          case GMMG_NLDENS: CLIP_MAX(FLOAT64,lpDest,nLm1,nLm2,&lpbErr[k]); break;/*           Limit neg.log.prob.dens.*/
          case GMMG_DENS  : CLIP_MIN(FLOAT64,lpDest,nLm1,nLm2,&lpbErr[k]); break;/*           Limit prob. density     */
        }                                                                       /*         <<                        */

  /* Report invalid densities/distances */
  for (k=0; k<M; k++)                                                           /*     Loop over vector components   */
    if(lpbErr[k]) IERROR(_this,GMM_INVALD,k,0,0);

  /* Clean up */                                                                /* --------------------------------- */
  dlp_free(lpbErr);                                                             /* Destroy invalid flags buffer      */
  CData_CopyDescr(idDest,idX);                                                  /* Copy data descriptions            */
  dlp_strcpy(idDest->m_lpCunit,""); idDest->m_nCinc=0.; idDest->m_nCofs=0.;     /* Clear component descriptions      */
  DESTROYVIRTUAL(idX,idDest);                                                   /* Clear overlapping arguments       */
  return O_K;                                                                   /* Ok                                */
}

/* EOF */
