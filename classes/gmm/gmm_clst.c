/* dLabPro class CGmm (gmm)
 * - Clustering methods
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

/**
 * Splits one Gaussian.
 * 
 * @param _this
 *          Pointer to CGmm instance
 * @param lpMean
 *          Pointer to parent mean vector (N doubles)
 * @param lpCov
 *          Pointer to parent covariance matrix (N*N doubles)
 * @param lpMeanS1
 *          Pointer to buffer for mean vector of 1st child (N doubles)
 * @param lpCovS1
 *          Pointer to buffer for covariance matrix of 1st child (N*N doubles)
 * @param lpMeanS2
 *          Pointer to buffer for mean vector of 2nd child (N doubles)
 * @param lpCovS2
 *          Pointer to buffer for covariance matrix of 2nd child (N*N doubles)
 * @param N
 *          Gaussian dimensionality (see CGmm_GetDim)
 * @return Greatest Eigenvalue (= greatest variance; proportional to distance by
 *         which the child mean vectors where moved)
 */
FLOAT64 CGEN_PRIVATE CGmm_Split_Icov
(
  CGmm*   _this,
  FLOAT64* lpMean,
  FLOAT64* lpCov,
  FLOAT64* lpMeanS1,
  FLOAT64* lpCovS1,
  FLOAT64* lpMeanS2,
  FLOAT64* lpCovS2,
  INT32    N
)
{
  INT32    i    = 0;                                                            /* Dimension loop counter            */
  FLOAT64* lpM1 = NULL;                                                         /* Auxilary matrix #1                */
  FLOAT64* lpM2 = NULL;                                                         /* Auxilary matrix #2                */
  FLOAT64* lpM3 = NULL;                                                         /* Auxilary matrix #3                */
  FLOAT64  nVar = 0.;                                                           /* Greatest Eigenvalue               */
  FLOAT64  u025 = 0.674489750;                                                  /* Constant factor (mean offset)     */
  FLOAT64  u009 = 1.370046544;                                                  /* Constant factor (var. shrink)     */

  /* Initialize */                                                              /* --------------------------------- */
  lpM1 = (FLOAT64*)dlp_calloc(N*N,sizeof(FLOAT64));                             /* Allocate auxilary matrix #1       */
  lpM2 = (FLOAT64*)dlp_calloc(N*N,sizeof(FLOAT64));                             /* Allocate auxilary matrix #2       */
  lpM3 = (FLOAT64*)dlp_calloc(N*N,sizeof(FLOAT64));                             /* Allocate auxilary matrix #3       */

  /* Determine axis of greatest variance */                                     /* --------------------------------- */
  dlp_memmove(lpM1,lpCov,N*N*sizeof(FLOAT64));                                  /* lpCov -> lpM1                     */
  dlm_eigen_jac(lpM1,lpM3,N,FALSE);                                             /* Eigenvals.->lpM1,Eigenvects.->lpM3*/
  dlm_transpose(lpM2,lpM3,N,N);                                                 /* Transpose Eigenvects.->lpM2       */
  nVar = sqrt(lpM1[0]);                                                         /* Sqrt. of greatest element of lpM1 */

  /* Compute new mean vectors */                                                /* --------------------------------- */
  for (i=0; i<N; i++)                                                           /* Loop over dimensions              */
  {                                                                             /* >>                                */
    if (lpMeanS1) lpMeanS1[i] = lpMean[i] - u025*nVar*lpM2[i*N];                /*   Offset 1st child's mean vector  */
    if (lpMeanS2) lpMeanS2[i] = lpMean[i] + u025*nVar*lpM2[i*N];                /*   Offset 2nd child's mean vector  */
  }                                                                             /* <<                                */

  /* Compute new covariance matrices */                                         /* --------------------------------- */
  lpM1[0] = lpM1[0]/u009/u009;                                                  /* Shrink greatest Eigenvalue        */
  if (lpCovS1) dlm_mult_akat(lpCovS1,lpM2,N,N,lpM1);                            /* Spin Eigenvects. to original pos. */
  if (lpCovS2) dlm_mult_akat(lpCovS2,lpM2,N,N,lpM1);                            /* Spin Eigenvects. to original pos. */

  /* Clean up */                                                                /* --------------------------------- */
  dlp_free(lpM1);                                                               /* Free auxilary matrix #1           */
  dlp_free(lpM2);                                                               /* Free auxilary matrix #2           */
  dlp_free(lpM3);                                                               /* Free auxilary matrix #3           */
  return nVar;                                                                  /* Return greatest variance          */
}

/*
 * Manual page at gmm.def
 */
INT16 CGEN_PUBLIC CGmm_Split(CGmm* _this, FLOAT64 nParam, CData* idMap)
{
  INT32     b        = 0;                                                        /* Current write block index         */
  INT32     k        = 0;                                                        /* Single Gaussian loop counter      */
  INT32     m        = 0;                                                        /* Cur. record idx. in splitting map */
  INT32     K        = 0;                                                        /* Number of single Gaussians        */
  INT32     N        = 0;                                                        /* Gaussian dimensionality           */
  FLOAT64*  lpMean   = NULL;                                                     /* Buffer for parent's mean vector   */
  FLOAT64*  lpMeanS1 = NULL;                                                     /* Buffer for 1st child's mean vect. */
  FLOAT64*  lpMeanS2 = NULL;                                                     /* Buffer for 2nd child's mean vect. */
  FLOAT64*  lpCov    = NULL;                                                     /* Buffer for parent's cov. matrix   */
  FLOAT64*  lpCovS1  = NULL;                                                     /* Buffer for 1st child's cov. matrix*/
  FLOAT64*  lpCovS2  = NULL;                                                     /* Buffer for 2nd child's cov. matrix*/
  FLOAT64   nVar     = 0.;                                                       /* Greatest variance of Gaussian     */
  CData*   idCov    = NULL;                                                     /* Parent's covariance matrix set    */
  CData*   idCovS   = NULL;                                                     /* Split covariance matrix set       */
  CData*   idMeanS  = NULL;                                                     /* Split mean vector set             */
  CData*   idSmap   = NULL;                                                     /* Splitting map                     */
  CData*   idGmap   = NULL;                                                     /* Children-to-parents map           */
  CVmap*   iMmap    = NULL;                                                     /* Mixture map                       */
  
  /* Initialize */                                                              /* --------------------------------- */
  CHECK_THIS_RV(NOT_EXEC);                                                      /* Check this pointer                */
  IFCHECKEX(1)                                                                  /* On verbose level 1                */
  {                                                                             /* >>                                */
    printf("\n"); dlp_fprint_x_line(stdout,'-',dlp_maxprintcols());             /*   Print protocol header           */
    printf("\n CGmm_Split(%s,%lg,%s)",BASEINST(_this)->m_lpInstanceName,nParam, /*   ...                             */
      idMap?BASEINST(idMap)->m_lpInstanceName:"NULL");                          /*   ...                             */
    printf("\n"); dlp_fprint_x_line(stdout,'-',dlp_maxprintcols());             /*   ...                             */
    printf("\n");                                                               /*   ...                             */
  }                                                                             /* <<                                */
  K        = CGmm_GetNGauss(_this);                                             /* Get number of single Gaussians    */
  N        = CGmm_GetDim(_this);                                                /* Get dimensionality                */
  lpMean   = (FLOAT64*)dlp_calloc(N  ,sizeof(FLOAT64));                           /* Allocate buffer                   */
  lpMeanS1 = (FLOAT64*)dlp_calloc(N  ,sizeof(FLOAT64));                           /* Allocate buffer                   */
  lpMeanS2 = (FLOAT64*)dlp_calloc(N  ,sizeof(FLOAT64));                           /* Allocate buffer                   */
  lpCov    = (FLOAT64*)dlp_calloc(N*N,sizeof(FLOAT64));                           /* Allocate buffer                   */
  lpCovS1  = (FLOAT64*)dlp_calloc(N*N,sizeof(FLOAT64));                           /* Allocate buffer                   */
  lpCovS2  = (FLOAT64*)dlp_calloc(N*N,sizeof(FLOAT64));                           /* Allocate buffer                   */
  ICREATEEX(CData,idCov  ,"CGmm_Split.idCov"  ,NULL);                           /* Create parent cov. matrix set     */
  ICREATEEX(CData,idCovS ,"CGmm_Split.idCovS" ,NULL);                           /* Create split cov. matrix set      */
  ICREATEEX(CData,idMeanS,"CGmm_Split.idMeanS",NULL);                           /* Create split mean vector set      */
  ICREATEEX(CData,idSmap ,"CGmm_Split.idSmap" ,NULL);                           /* Create splitting map              */
  ICREATEEX(CData,idGmap ,"CGmm_Split.idGmap" ,NULL);                           /* Create children-to-parent map     */
  CGmm_Extract(_this,NULL,idCovS);                                              /* Extract inv. covariance matrices  */
  CMatrix_Invert(idCovS,idCov,NULL);                                            /* Compute covariance matrices       */
  if (dlp_is_numeric_type_code(CData_GetCompType(idMap,0)))                     /* User defined splitting map        */
  {                                                                             /* >>                                */
    CData_SelectComps(idSmap,idMap,0,1);                                        /*   Copy it                    IC=0 */
    CData_Realloc(idSmap,K);                                                    /*   Make it the proper length       */
  }                                                                             /* <<                                */
  else                                                                          /* Make automatic splitting map      */
  {                                                                             /* >>                                */
    CData_Array(idSmap,T_SHORT,1,K);                                            /*   Create splitting map       IC=0 */
    CData_Fill(idSmap,CMPLX(1.),CMPLX(0.));                                     /*   Split 'em all                   */
  }                                                                             /* <<                                */
  CData_AddComp(idSmap,"k"  ,T_LONG  );                                         /* Add index component          IC=1 */
  CData_AddComp(idSmap,"var",T_DOUBLE);                                         /* Add variance component       IC=2 */
  
  /* Analyze Gaussians */                                                       /* --------------------------------- */
  IFCHECKEX(1) printf("\n\n   Analyzing Gaussians");                            /* Protocol (verbose level 1)        */
  for (k=0,b=0; k<K; k++)                                                       /* Loop over single Gaussians        */
  {                                                                             /* >>                                */
    IFCHECKEX(1) printf("\n   - %4ld: ",k);                                     /*   Protocol (verbose level 1)      */
    if (CData_Dfetch(idSmap,b,0)!=0.)                                           /*   Gaussian not locked (may split) */
    {                                                                           /*   >>                              */
      CData_DrecFetch(AS(CData,_this->m_idMean),lpMean,k,N,-1);                 /*     Fetch parent's mean vector    */
      CData_DblockFetch(idCov,lpCov,k,N,N,-1);                                  /*     Fetch parent's cov. matrix    */
      nVar = CGmm_Split_Icov(_this,lpMean,lpCov,NULL,NULL,NULL,NULL,N);         /*     Get greatest variance         */
      IFCHECKEX(1) printf("var=%lg ",nVar);                                     /*     Protocol (verbose level 1)    */
      CData_Dstore(idSmap,k   ,b,1);                                            /*     Store Gaussian index          */
      CData_Dstore(idSmap,nVar,b,2);                                            /*     Store greates variance        */
      b++;                                                                      /*     Increment map index           */
    }                                                                           /*   <<                              */
    else                                                                        /*   Gaussian locked                 */
    {                                                                           /*   >>                              */
      IFCHECKEX(1) printf("LOCKED",nVar);                                       /*     Protocol (verbose level 1)    */
      CData_DeleteRecs(idSmap,b,1);                                             /*     Delete map item               */
    }                                                                           /*   <<                              */
  }                                                                             /* <<                                */
  
  /* Make splitting map */                                                      /* --------------------------------- */
  if (nParam>0 && nParam<K)                                                     /* Split only nParam Gaussians       */
  {                                                                             /* >>                                */
    IFCHECKEX(1) printf("\n\n   Selecting %ld Gaussians by greatest variance"); /*   Protocol (verbose level 1)      */
    CData_Sortdown(idSmap,idSmap,2);                                            /*   Sort by variance                */
    CData_Realloc(idSmap,(INT32)nParam);                                         /*   Clip map                        */
  }                                                                             /* <<                                */
    
  /* Split Gaussians */                                                         /* --------------------------------- */
  CData_Array(idGmap,T_LONG,1,2*K);                                             /* Allocate children-to-parents map  */
  CData_Array(idMeanS,_this->m_nType,N,2*K  );                                  /* Allocate preliminary split means  */
  CData_Array(idCovS ,_this->m_nType,N,2*K*N);                                  /* Allocate preliminary split covs.  */
  CData_SetNBlocks(idCovS,2*K);                                                 /* Set block length                  */
  IFCHECKEX(1) printf("\n\n   Splitting Gaussians");                            /*   Protocol (verbose level 1)      */
  for (k=0,b=0; k<K; k++)                                                       /* Loop over single Gaussians        */
  {                                                                             /* >>                                */
    CData_DrecFetch(AS(CData,_this->m_idMean),lpMean,k,N,-1);                   /*   Fetch parent's mean vector      */
    CData_DblockFetch(idCov,lpCov,k,N,N,-1);                                    /*   Fetch parent's cov. matrix      */
    m = CData_Find(idSmap,0,CData_GetNRecs(idSmap),1,1,k);                      /*   Find index k in splitting map   */
    IFCHECKEX(1) printf("\n   - %4ld [map=%4ld]: ",(long)k,(long)m);            /*   Protocol (verbose level 1)      */
    if (m>=0)                                                                   /*   Split Gaussian                  */
    {                                                                           /*   >>                              */
      DLPASSERT(CData_Dfetch(idSmap,m,0)!=0.);                                  /*     Locked Gaussian in split map  */
      IFCHECKEX(1) printf("split ");                                            /*     Protocol (verbose level 1)    */
      CGmm_Split_Icov(_this,lpMean,lpCov,lpMeanS1,lpCovS1,lpMeanS2,lpCovS2,N);  /*     Split Gaussian                */
      CData_DrecStore  (idMeanS,lpMeanS1,b  ,N  ,-1);                           /*     Store 1st child's mean vector */
      CData_DblockStore(idCovS ,lpCovS1 ,b  ,N,N,-1);                           /*     Store 1st child's cov. matrix */
      CData_DrecStore  (idMeanS,lpMeanS2,b+1,N  ,-1);                           /*     Store 2nd child's mean vector */
      CData_DblockStore(idCovS ,lpCovS2, b+1,N,N,-1);                           /*     Store 2nd child's cov. matrix */
      CData_Dstore(idGmap,k,b  ,0);                                             /*     Update children-2-parent map  */
      CData_Dstore(idGmap,k,b+1,0);                                             /*     Update children-2-parent map  */
      IFCHECKEX(1) printf("-> %ld, %ld",(long)b,(long)b+1);                     /*     Protocol (verbose level 1)    */
      b+=2;                                                                     /*     Increment child counter       */
    }                                                                           /*   <<                              */
    else                                                                        /*   Don't split, just copy          */
    {                                                                           /*   >>                              */
      IFCHECKEX(1) printf("keep  -> %ld",(long)b);                              /*     Protocol (verbose level 1)    */
      CData_DrecStore(idMeanS,lpMean,b,N,-1);                                   /*     Store single child's mean vec.*/
      CData_DblockStore(idCovS,lpCov,b,N,N,-1);                                 /*     Store single child's cov. mat.*/
      CData_Dstore(idGmap,k,b,0);                                               /*     Update children-2-parent map  */
      b++;                                                                      /*     Increment child counter       */
    }                                                                           /*   <<                              */
  }                                                                             /* <<                                */
  CData_DeleteRecs  (idMeanS,b,2*K-b);                                          /* Delete unused mean vectors        */
  CData_DeleteBlocks(idCovS ,b,2*K-b);                                          /* Delete unused covariance matrices */
  CData_DeleteRecs  (idGmap ,b,2*K-b);                                          /* Delete unused map entries         */
  IFCHECKEX(1) printf("\n   done (%ld Gaussians split)\n",(long)b/2);           /*   Protocol (verbose level 1)      */

  if(_this->m_iMmap && !CData_IsEmpty(AS(CData,AS(CVmap,_this->m_iMmap)->m_idTmx))){
    CVmap *iMmapO=AS(CVmap,_this->m_iMmap);
    CData *idMmap;
    FLOAT64 *lpComp;
    INT32 nNR = CData_GetNRecs(AS(CData,iMmapO->m_idTmx));
    ICREATEEX(CVmap,iMmap,"CHmm::SetupGmm.~iMmap",NULL);
    ICREATEEX(CData,idMmap ,"CGmm_Split.idMmap" ,NULL);                           /* Create children-to-parent map     */
    lpComp=(FLOAT64*)dlp_malloc(nNR*sizeof(FLOAT64));
    CData_Array(idMmap,_this->m_nType,b,nNR);
    for(k=0;k<b;k++)
    {
      CData_DcompFetch(AS(CData,iMmapO->m_idTmx),lpComp,(INT32)CData_Dfetch(idGmap,k,0),nNR);
      CData_DcompStore(idMmap,lpComp,k,nNR);
    }  
    CVmap_Setup(iMmap,idMmap,
      dlp_scalop_sym(iMmapO->m_nAop),
      dlp_scalop_sym(iMmapO->m_nWop),
      iMmapO->m_nZero);
    IDESTROY(idMmap);
    dlp_free(lpComp);
  }

  /* Aftermath */                                                               /* --------------------------------- */
  CGmm_Setup(_this,idMeanS,idCovS,iMmap);                                       /* Resetup Gaussians                 */
  if (idMap) CData_Copy(BASEINST(idMap),BASEINST(idGmap));                      /* Copy split map for caller         */  
  IFCHECKEX(1)                                                                  /* On verbose level 1                */
  {                                                                             /* >>                                */
    printf("\n\n CGmm_Split done.\n");                                          /*   Print protocol footer           */
    dlp_fprint_x_line(stdout,'-',dlp_maxprintcols());                           /*   ...                             */
    printf("\n");                                                               /*   ...                             */
  }                                                                             /* <<                                */

  /* Clean up */                                                                /* --------------------------------- */
  dlp_free(lpMean  );                                                           /* Free buffer                       */
  dlp_free(lpMeanS1);                                                           /* Free buffer                       */
  dlp_free(lpMeanS2);                                                           /* Free buffer                       */
  dlp_free(lpCov   );                                                           /* Free buffer                       */
  dlp_free(lpCovS1 );                                                           /* Free buffer                       */
  dlp_free(lpCovS2 );                                                           /* Free buffer                       */
  IDESTROY(idCov   );                                                           /* Destroy temporary instance        */
  IDESTROY(idCovS  );                                                           /* Destroy temporary instance        */
  IDESTROY(idMeanS );                                                           /* Destroy temporary instance        */
  IDESTROY(idSmap  );                                                           /* Destroy temporary instance        */
  IDESTROY(idGmap  );                                                           /* Destroy temporary instance        */
  IDESTROY(iMmap   );                                                           /* Destroy temporary instance        */
  return O_K;                                                                   /* All done                          */
}

/* EOF */
