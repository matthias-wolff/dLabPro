/* dLabPro class CGmm (gmm)
 * - Computation core
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

#if GMM_FTYPE_CODE == T_FLOAT                                                   /* Compile single precision version: */
  #define GMM_FTYPE FLOAT32                                                     /*   GMM_FTYPE = float               */
#elif GMM_FTYPE_CODE == T_DOUBLE                                                /* Compile double precision version: */
  #define GMM_FTYPE FLOAT64                                                     /*   GMM_FTYPE = double              */
#else                                                                           /* Type code GMM_FTYPE not supported */
  #error GMM_FTYPE_CODE must be T_FLOAT (for float) or T_DOUBLE (for double).   /*   Error                           */
#endif                                                                          /*                                   */

#ifdef GMM_FTYPE                                                                /* Compile only if GMM_FTYPE defined */

#define  alpha   ((GMM_FTYPE* )_this->m_lpAlpha)                                /* Alpha vector 1)                   */
#define  beta    ((GMM_FTYPE* )_this->m_lpBeta )                                /* Beta vectors 1)                   */
#define  gamma   ((GMM_FTYPE* )_this->m_lpGamma)                                /* Gamma vector 1)                   */
#define  delta   ((GMM_FTYPE* )_this->m_lpDelta)                                /* Delta vector 1)                   */
#define  I       ((GMM_FTYPE**)_this->m_lpI    )                                /* Inverse covariance matrices       */
#define  V       ((GMM_FTYPE**)_this->m_lpV    )                                /* Inverse variance vectors          */
#ifndef __TMS
#define  mu(K,N) (GMM_FTYPE)CData_Dfetch(AS(CData,_this->m_idMean),K,N)         /* Mean vector comp. N of Gaussian K */
#else
#define  mu(k,n)  (mean[(k)*N+(n)])
#endif
#define  L(N,k,n)    (((GMM_FTYPE*)_this->m_lpLdlL)[(k)*(N)*((N)-1)/2+(n)])     /* L matrix of LDL factorization     */
#define  Li(N,k,n,m) (L(N,k,(n)*(2*(N)-3-(n))/2+(m)-1))                         /* L matrix with both indizies (m>n) */
#define  D(N,k,n)    (((GMM_FTYPE*)_this->m_lpLdlD)[(k)*(N)+(n)])               /* D matrix of LDL factorization     */
                                                                                /* 1) see release notes for details  */                                                                                
#if GMM_FTYPE_CODE == T_FLOAT
int cf_absfloat_down(const void* a, const void* b)                              /* Compare function for absolute val.*/
#else
int cf_absdouble_down(const void* a, const void* b)                             /* Compare function for absolute val.*/
#endif
{                                                                               /* >>                                */
  GMM_FTYPE da=fabs(*(GMM_FTYPE*)a);                                            /*   Get abs. of value a             */
  GMM_FTYPE db=fabs(*(GMM_FTYPE*)b);                                            /*   Get abs. of value b             */
  if(da > db) return -1;                                                        /*   If a is abs. greater than b ?   */
  if(da < db) return  1;                                                        /*   If b is abs. greater than a ?   */
  return 0;                                                                     /*   Abs. val. of a and b equal ?    */
}                                                                               /* <<                                */

/* NO JAVADOC
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
#if GMM_FTYPE_CODE == T_FLOAT
INT16 CGmm_PrecalcF(CGmm* _this, BOOL bCleanup)
#else
INT16 CGmm_PrecalcD(CGmm* _this, BOOL bCleanup)
#endif
{
  INT32      i       = 0;                                                        /* Triangular inv. cov. matrix cntr. */
  INT32      c       = 0;                                                        /* Index of inv. covariance matrix   */
  INT32      k       = 0;                                                        /* Gaussian loop counter             */
  INT32      n       = 0;                                                        /* Dimension loop counter            */
  INT32      m       = 0;                                                        /* Dimension loop counter            */
  INT32      K       = 0;                                                        /* Number of single Gaussians        */
  INT32      N       = 0;                                                        /* Feature space dimensionality      */
  GMM_FTYPE nDelta1 = 0.;                                                       /* Class indep. term of delta const. */
#ifdef __TMS
  GMM_FTYPE *mean = (GMM_FTYPE*)CData_XAddr(AS(CData,_this->m_idMean),0,0);
#endif


  /* Clean up precalculated data */                                             /* --------------------------------- */
  dlp_free(_this->m_lpAlpha);                                                   /* Clear alpha vector                */
  dlp_free(_this->m_lpBeta );                                                   /* Clear beta vectors                */
  dlp_free(_this->m_lpGamma);                                                   /* Clear gamma vector                */
  dlp_free(_this->m_lpDelta);                                                   /* Clear delta vector                */
  dlp_free(_this->m_lpI    );                                                   /* Clear inv. cov. pointer array     */
  dlp_free(_this->m_lpV    );                                                   /* Clear inverse variance array      */
  if(_this->m_idSse2Icov){
    CData *idSse2Icov=AS(CData,_this->m_idSse2Icov);
    IDESTROY(idSse2Icov);                                                     /* Destroy inv.covs. for SSE2 opt.   */
    _this->m_idSse2Icov=NULL;
  }
  dlp_free(_this->m_lpSse2Buf);                                                 /* Clear aux. buffer for SSE2 opt.   */
  dlp_free(_this->m_lpLdlL);                                                    /* Clear L-matrix buffer for LDL     */
  dlp_free(_this->m_lpLdlD);                                                    /* Clear D-vector buffer for LDL     */
  _this->m_nN = 0;                                                              /* Clear feature space dimensionality*/
  _this->m_nK = 0;                                                              /* Clear number of single Gaussians  */
  if (bCleanup) return O_K;                                                     /* When cleaning up that's it        */

  /* Initialize */                                                              /* --------------------------------- */
  K       = CGmm_GetNGauss(_this);                                              /* Get nuumber of single Gaussians   */
  N       = CGmm_GetDim(_this);                                                 /* Get feature space dimensionality  */
  nDelta1 = -N/2*log(2*F_PI);                                                   /* Compute part of delta term        */

  /* Basic validation */                                                        /* --------------------------------- */
  IF_NOK(CGmm_CheckMean(_this)) DLPTHROW(GMM_NOTSETUP);                         /* Check mean vectors                */
  IF_NOK(CGmm_CheckIvar(_this)) DLPTHROW(GMM_NOTSETUP);                         /* Check inverse variance vectors    */
  IF_NOK(CGmm_CheckCdet(_this)) DLPTHROW(GMM_NOTSETUP);                         /* Check (co-)variance determinants  */

  /* Basic dimensions */                                                        /* --------------------------------- */
  _this->m_nN = N;                                                              /* Feature space dimensionality      */
  _this->m_nK = K;                                                              /* Number of single Gaussians        */

  /* Create inverse covariance matrix map */                                    /* --------------------------------- */
  if (_this->m_idIcov)                                                          /* If using covariances              */
  {                                                                             /* >>                                */
    IF_NOK(CGmm_CheckIcov(_this)) DLPTHROW(GMM_NOTSETUP);                       /*   Inverse covariance data corrupt */
    _this->m_lpI = dlp_calloc(K,sizeof(GMM_FTYPE*));                            /*   Allocate map                    */
    if (!_this->m_lpI) DLPTHROW(ERR_NOMEM);                                     /*   Out of memory                   */
    for (k=0; k<K; k++)                                                         /*   Loop over Gaussians             */
    {                                                                           /*   >>                              */
      c=_this->m_idCmap ? (INT32)CData_Dfetch(AS(CData,_this->m_idCmap),k,0) : k;/*     Cov. mat. idx. for Gaussian k */
      I[k] = (GMM_FTYPE*)CData_XAddr(AS(CData,_this->m_idIcov),c,0);            /*     Store ptr. to inv. cov. matrix*/
    }                                                                           /*   <<                              */
  }                                                                             /* <<                                */

  /* Create inverse variance vector map */                                      /* --------------------------------- */
  IF_NOK(CGmm_CheckIvar(_this)) DLPTHROW(GMM_NOTSETUP);                         /*   Inverse covariance data corrupt */
  _this->m_lpV = dlp_calloc(K,sizeof(GMM_FTYPE*));                              /*   Allocate map                    */
  if (!_this->m_lpV) DLPTHROW(ERR_NOMEM);                                       /*   Out of memory                   */
  for (k=0; k<K; k++)                                                           /*   Loop over Gaussians             */
    V[k] = (GMM_FTYPE*)CData_XAddr(AS(CData,_this->m_idIvar),k,0);              /*     Store ptr. to inv. cov. matrix*/

  /* LDL-factorization */                                                       /* --------------------------------- */
  if(_this->m_nLDL)                                                             /* LDL factorization used            */
  {                                                                             /* >>                                */
    /* TODO: GMM_TYPE is float => dlm_factldl in float */                       /*   ------------------------------- */
    _this->m_lpLdlL = dlp_malloc(K*N*(N-1)/2*sizeof(GMM_FTYPE));                /*   Alloc L matrix                  */
    _this->m_lpLdlD = dlp_malloc(K*N*sizeof(GMM_FTYPE));                        /*   Alloc D vector                  */
    if (!_this->m_lpLdlL || !_this->m_lpLdlD) DLPTHROW(ERR_NOMEM);              /*   Out of memory                   */
    {
    FLOAT64 *lpAux1 = (FLOAT64 *)dlp_malloc(N*N*sizeof(FLOAT64));                  /*   Alloc auxilary matrix #1        */
    FLOAT64 *lpAux2 = (FLOAT64 *)dlp_malloc(N*N*sizeof(FLOAT64));                  /*   Alloc auxilary matrix #2        */
    if (!lpAux1 || !lpAux2) DLPTHROW(ERR_NOMEM);                                /*   Out of memory                   */
    for(k=0;k<K;k++)                                                            /*   Loop over all Gaussians         */
    {                                                                           /*   >>                              */
      for(n=0;n<N;n++)                                                          /*     Loop over Gaussian dimension  */
      {                                                                         /*     >>                            */
        lpAux1[n*N+n] = V[k][n];                                                /*       Copy inverse variance values*/
        for(m=0;m<n;m++) lpAux1[n*N+m] = lpAux1[m*N+n] =                        /*       Copy inverse covariance val.*/
          I?I[k][m*N-2*m-m*(m-1)/2+n-1]:0.;                                     /*       |                           */
      }                                                                         /*     <<                            */
      dlm_factldl(lpAux1,lpAux2,N);                                             /*     Factorize matrix              */
      for(n=0;n<N;n++)                                                          /*     Loop over Gaussian dimension  */
      {                                                                         /*     >>                            */
        for(m=n+1;m<N;m++) Li(N,k,n,m) = lpAux1[n*N+m];                         /*       Store L matrix values       */
        D(N,k,n) = lpAux2[n*N+n];                                               /*       Store D vector values       */
      }                                                                         /*     <<                            */
      if(_this->m_nLdlCoef>=0 && _this->m_nLdlCoef<N*(N-1)/2)
      {
        FLOAT64 nMinAbs;
        memcpy(lpAux1,&L(N,k,0),N*(N-1)/2);
#if GMM_FTYPE_CODE == T_FLOAT
        qsort(lpAux1,N*(N-1)/2,sizeof(FLOAT64),cf_absfloat_down);
#else
        qsort(lpAux1,N*(N-1)/2,sizeof(FLOAT64),cf_absdouble_down);
#endif
        nMinAbs=fabs(lpAux1[_this->m_nLdlCoef]);
        for(n=0;n<N*(N-1)/2;n++) if(fabs(L(N,k,n))<=nMinAbs) L(N,k,n)=0.;
      }
    }                                                                           /*   <<                              */
    dlp_free(lpAux1);                                                           /*   Free auxilary matrix #1         */
    dlp_free(lpAux2);                                                           /*   Free auxilary matrix #2         */
    }
  }                                                                             /* <<                                */

  /* Precalculate alpha and beta vectors */                                     /* --------------------------------- */
  if(!_this->m_nLDL)                                                            /* No LDL factorization used         */
  {                                                                             /* >>                                */
    _this->m_lpAlpha = dlp_calloc(K,sizeof(GMM_FTYPE));                         /*   Allocate buffer                 */
    _this->m_lpBeta  = dlp_calloc(K*N,sizeof(GMM_FTYPE));                       /*   Allocate buffer                 */
    if (!_this->m_lpAlpha || !_this->m_lpBeta) DLPTHROW(ERR_NOMEM);             /*   Out of memory                   */
    for (k=0; k<K; k++)                                                         /*   Loop over Gaussians             */
      for (n=0; n<N; n++)                                                       /*     Loop over dimensions          */
        for (m=0; m<N; m++)                                                     /*       Loop over dimensions        */
          if (m==n)                                                             /*         Main diagonal (variances)?*/
          {                                                                     /*         >>                        */
            alpha[k]     += V[k][n]*mu(k,m)*mu(k,n);                            /*           Sum up alpha val. (n==m)*/
            beta [k*N+n] += V[k][n]*mu(k,m);                                    /*           Sum up beta value (n==m)*/
          }                                                                     /*         <<                        */
          else if (_this->m_lpI)                                                /*         Otherwise cov.(if present)*/
          {                                                                     /*         >>                        */
            if (m>n) i = n*N - 2*n - n*(n-1)/2 + m - 1;                         /*           Calc index if I[n,m] in */
            else     i = m*N - 2*m - m*(m-1)/2 + n - 1;                         /*           | triangular icov. mat. */
            alpha[k]     += I[k][i]*mu(k,m)*mu(k,n);                            /*           Sum up alpha val. (n!=m)*/
            beta [k*N+n] += I[k][i]*mu(k,m);                                    /*           Sum up beta value (n!=m)*/
          }                                                                     /*         <<                        */
  }                                                                             /* <<                                */
  else                                                                          /* LDL factorization used            */
  {                                                                             /* >>                                */
    _this->m_lpAlpha = NULL;                                                    /*   No alpha needed here            */
    _this->m_lpBeta  = dlp_calloc(K*N,sizeof(GMM_FTYPE));                       /*   Allocate buffer                 */
    if (!_this->m_lpBeta) DLPTHROW(ERR_NOMEM);                                  /*   Out of memory                   */
    for(k=0;k<K;k++) for(n=0;n<N;n++)                                           /*   Loop over Gaussians & dimensions*/
    {                                                                           /*   >>                              */
      beta[k*N+n] = mu(k,n);                                                    /*     Init beta with mean value     */
      for(m=n+1;m<N;m++) beta[k*N+n] += mu(k,m)*Li(N,k,n,m);                    /*     Calculate beta from L*mu      */
    }                                                                           /*   <<                              */
  }                                                                             /* <<                                */

  /* Allocate gamma vector */                                                   /* --------------------------------- */
  /* NOTE: If this fails there will just be no lazy computation -> no big deal*//*                                   */
  if (_this->m_idCmap && _this->m_idIcov && _this->m_lpI)                       /* Wanna do lazy computation?        */
    if (CData_GetDescr(AS(CData,_this->m_idIvar),0)==0.)                        /*   Only if variances are tied too  */
      _this->m_lpGamma = dlp_calloc(K,sizeof(GMM_FTYPE));                       /*     Allocate buffer               */

  /* Precalculate delta vector */                                               /* --------------------------------- */
  _this->m_lpDelta = dlp_calloc(K,sizeof(GMM_FTYPE));                           /* Allocate buffer                   */
  if (!_this->m_lpDelta) DLPTHROW(ERR_NOMEM);                                   /* Out of memory                     */
  for (k=0; k<K; k++)                                                           /* Loop over Gaussians               */
    delta[k] = nDelta1-(GMM_FTYPE)(0.5*log(                                     /*   Calculate delta[k]              */
      CData_Dfetch(AS(CData,_this->m_idCdet),k,0)));                            /*   |                               */

  /* SSE2 precalculated objects */                                              /* --------------------------------- */
#ifdef GMM_SSE2                                                                 /* Use SSE2?                         */
  IFIELD_RESET(CData,"sse2_icov");                                              /* Create/reset SSE2 inv.cov. matrs. */
  CGmm_Extract(_this,NULL,_this->m_idSse2Icov);                                 /* ... and go get 'em                */
  _this->m_lpSse2Buf = dlp_calloc(2*N,sizeof(GMM_FTYPE));                       /* Allocate auxilary buffer          */
  if (!_this->m_lpSse2Buf) DLPTHROW(ERR_NOMEM);                                 /* Out of memory                     */
#endif                                                                          /* #ifdef GMM_SSE2                   */

  /* Final checkup */                                                           /* --------------------------------- */
#ifndef __NOXALLOC
  IF_NOK(CGmm_CheckPrecalc(_this)) DLPTHROW(GMM_NOTSETUP);                      /* Check precalculated data          */
#endif
  return O_K;                                                                   /* That's it folks ...               */

DLPCATCH(GMM_NOTSETUP)                                                          /* On NOT_EXEC exception             */
DLPCATCH(ERR_NOMEM)                                                             /* On ERR_NOMEM exception            */
#if GMM_FTYPE_CODE == T_FLOAT
  CGmm_PrecalcF(_this,TRUE);                                                    /* - Free memory of precalc'd. data  */
#else
  CGmm_PrecalcD(_this,TRUE);                                                    /* - Free memory of precalc'd. data  */
#endif
  return NOT_EXEC;                                                              /* - Indicate failure                */
}

/* NO JAVADOC
 * Calculates the Mahalanobis distance or the (logarithmic) probability density
 * of a feature vector given a single Gaussian distribution. There are NO CHECKS
 * performed.
 * 
 * Complexity: N + N*N/2 where N=m_nDim
 * 
 * @param _this
 *          Pointer to GMM instance
 * @param x
 *          Feature vector (expected to have N=m_nDim values)
 * @param k
 *          Index of single Gaussian
 * @param nMode
 *          Operation mode, one of the GMMG_XXX constants
 * @return  The (logarithmic) probability density or Mahalanobis distance of x
 *          in single Gaussian k.
 */
#if GMM_FTYPE_CODE == T_FLOAT
GMM_FTYPE CGmm_GaussF(CGmm* _this, GMM_FTYPE* x, INT32 k, INT16 nMode)
#else
GMM_FTYPE CGmm_GaussD(CGmm* _this, GMM_FTYPE* x, INT32 k, INT16 nMode)
#endif
{
  INT32       i      = 0;                                                       /* Triangular inv. cov. matrix cntr. */
  INT32       n      = 0;                                                       /* Feature vector comp. loop index   */
  INT32       m      = 0;                                                       /* Feature vector comp. loop index   */
  INT32       c      = 0;                                                       /* Covariance matrix index           */
  INT32       N      = 0;                                                       /* Feature dimensionality            */
  INT32       K      = 0;                                                       /* Number of Gaussians               */
  GMM_FTYPE  nTermB = 0.;                                                       /* Term (B), see manual              */
  GMM_FTYPE  nTermE = 0.;                                                       /* Term (E), see manual              */
  GMM_FTYPE  nTermC = 0.;                                                       /* Term (D), see manual              */
  GMM_FTYPE  nSum   = 0.;                                                       /* Sum up buffer                     */
  GMM_FTYPE  nMdist = 0.;                                                       /* Mahalanobis distance              */
  GMM_FTYPE* bk     = NULL;                                                     /* Address of beta[k,0]              */
  GMM_FTYPE* Ik     = NULL;                                                     /* Address of I[k,0]                 */
  GMM_FTYPE* Vk     = NULL;                                                     /* Address of V[k,0]                 */
  INT32*      cmap   = NULL;                                                    /* Covariance tying map              */

  N    = _this->m_nN;                                                           /* Get feature dimensionality        */
  K    = _this->m_nK;                                                           /* Get feature dimensionality        */
  bk   = &beta[k*N];                                                            /* Class dependent beta vector       */
  Ik   = I?I[k]:NULL;                                                           /* Class dep. inv. covariance matrix */
  Vk   = V[k];                                                                  /* Class dep. inv. variance vector   */
  cmap =_this->m_idCmap?(INT32*)CDATA_XADDR(AS(CData,_this->m_idCmap),0,0):NULL;/* Get covariance tying map          */
  if (Ik && *(GMM_FTYPE*)CDATA_XADDR(AS(CData,_this->m_idCdet),k,0)==0.)        /* Covariance matrix invalid         */
    return (GMM_FTYPE)CGmm_GetLimit(_this,nMode);                               /*   Return limit                    */
  if(!_this->m_nLDL)                                                            /* No LDL factorization used         */
  {                                                                             /* >>                                */
    for (n=0; n<N; n++)                                                         /*   Loop over feature vector comps. */
    {                                                                           /*   >>                              */
      nTermB += bk[n]*x[n];                                                     /*     Sum up term B                 */
      nTermE += Vk[n]*x[n]*x[n];                                                /*     Sum up term E                 */
    }                                                                           /*   <<                              */
    if (Ik)                                                                     /*   Have full covariance matrix?    */
    {                                                                           /*   >> YES                          */
#if GMM_FTYPE_CODE == T_FLOAT                                                   /* ## FLOAT ##                       */
      if (gamma && gamma[k]<T_FLOAT_MAX)                                        /*     If already calc'd. this term C*/
#else                                                                           /* ## DOUBLE ##                      */
      if (gamma && gamma[k]<T_DOUBLE_MAX)                                       /*     If already calc'd. this term C*/
#endif                                                                          /* ##                                */
        nTermC = gamma[k];                                                      /*       ... just lazily reuse it    */
      else                                                                      /*     If not yet calc'd. this term C*/
      {                                                                         /*     >>                            */
        for (n=0,i=0; n<N-1; n++)                                               /*       Loop over upper right triang*/
        {                                                                       /*       >>                          */
          for (m=n+1,nSum=0.; m<N; m++,i++) nSum += Ik[i]*x[m];                 /*         L.o.u.r.t and sum up      */
          nTermC += nSum*x[n];                                                  /*         Sum up term C             */
        }                                                                       /*       <<                          */
        if (gamma)                                                              /*       Lazy mode -> remember C term*/
        {                                                                       /*       >>                          */
          c = cmap[k];                                                          /*         Get cov. matrix Gaussian k*/
          for (i=0; i<K; i++)                                                   /*         Loop over cov. tying map  */
            if (cmap[i]==c)                                                     /*           i shares Gaussian c?    */
              gamma[i]=nTermC;                                                  /*             So it has same gamma  */
        }                                                                       /*       <<                          */
      }                                                                         /*     <<                            */
    }                                                                           /*   << (otherwise nTermC=0!         */
    nMdist = alpha[k] - 2.0*nTermB + nTermE + 2.0*nTermC;                       /*   Calculate Mahalanobis distance  */
  }                                                                             /* <<                                */
  else                                                                          /* LDL factorization used            */
  {                                                                             /* >>                                */
    Ik=((GMM_FTYPE *)_this->m_lpLdlL)+k*N*(N-1)/2;                              /*   Get first index of L matrix     */
    Vk=((GMM_FTYPE *)_this->m_lpLdlD)+k*N;                                      /*   Get first index of D vector     */
    for(n=0;n<N;n++,Vk++)                                                       /*   Loop over feature vector comps. */
    {                                                                           /*   >>                              */
#if GMM_FTYPE_CODE == T_FLOAT                                                   /* ## FLOAT ##                       */
      if (gamma && gamma[k]<T_FLOAT_MAX)                                        /*     If already calc'd. this sum   */
#else                                                                           /* ## DOUBLE ##                      */
      if (gamma && gamma[k]<T_DOUBLE_MAX)                                       /*     If already calc'd. this sum   */
#endif                                                                          /* ##                                */
        nSum=gamma[k];                                                          /*       ... just lazily reuse it    */
      else                                                                      /*     If not yet calc'd. this sum   */
      {                                                                         /*     >>                            */
        for(nSum=x[n],m=n+1;m<N;m++,Ik++) nSum+=Ik[0]*x[m];                     /*       Calculate sum               */
        if(gamma)                                                               /*       Lazy mode -> remember sum   */
        {                                                                       /*       >>                          */
          c=cmap[k];                                                            /*         Get cov. matrix Guassian k*/
          for(i=0;i<K;i++) if(cmap[i]==c) gamma[i]=nSum;                        /*         Set values to remember    */
        }                                                                       /*       <<                          */
      }                                                                         /*     <<                            */
      nSum-=bk[n];                                                              /*     Subtract beta                 */
      nMdist+=nSum*nSum*Vk[0];                                                  /*     Update Mahalanobis distance   */
    }                                                                           /*   <<                              */
  }                                                                             /* <<                                */

  if (nMdist<0.) nMdist = 0.;                                                   /* Must be non-negative              */
  switch (nMode)                                                                /* Branch by operation mode          */
  {                                                                             /* >>                                */
    case GMMG_MDIST : return nMdist;                                            /*   Mahalanobis distance            */
    case GMMG_LDENS : return delta[k] - 0.5*nMdist;                             /*   Logarithmic probability density */
    case GMMG_NLDENS: return -(delta[k] - 0.5*nMdist);                          /*   Negative log. prob. density     */
    case GMMG_DENS  : return exp(delta[k] - 0.5*nMdist);                        /*   Probability density             */
  }                                                                             /* <<                                */
  DLPASSERT(FMSG("Unknown Gauss mode"));                                        /* Invalid value of nMode!           */
  return 0.;                                                                    /* Emergency exit                    */
}

#undef GMM_FTYPE                                                                /* Undefined for 2nd include         */
#undef alpha                                                                    /* Undefined for 2nd include         */
#undef beta                                                                     /* Undefined for 2nd include         */
#undef gamma                                                                    /* Undefined for 2nd include         */
#undef delta                                                                    /* Undefined for 2nd include         */
#undef I                                                                        /* Undefined for 2nd include         */
#undef V                                                                        /* Undefined for 2nd include         */
#undef mu                                                                       /* Undefined for 2nd include         */

#endif                                                                          /* #ifndef GMM_FTYPE                 */

/* EOF */
