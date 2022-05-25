/* dLabPro class CVmap (vmap)
 * - Implementation
 *
 * AUTHOR : Christian-M. Westendorf, Matthias Wolff
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
#include "dlp_vmap.h"                                                           /* Include class header file         */

INT16 CGEN_PUBLIC CTmx_IsCompressed(CData *idTmx){
  if(CData_GetNComps(idTmx)!=3) return 0;
  if(CData_GetCompType(idTmx,0)!=T_LONG) return 0;
  if(CData_GetCompType(idTmx,1)!=T_LONG) return 0;
  if(CData_GetCompType(idTmx,2)==T_DOUBLE) return T_DOUBLE;
  if(CData_GetCompType(idTmx,2)==T_FLOAT) return T_FLOAT;
  return 0;
}

#define VMAP_FTYPE_CODE  T_FLOAT
#include "vmap_impl_core.c"
#undef VMAP_FTYPE_CODE
#define VMAP_FTYPE_CODE  T_DOUBLE
#include "vmap_impl_core.c"
#undef VMAP_FTYPE_CODE

/**
 * Returns the input dimensionality of this mapping operator. The input
 * dimensionality is defined by {@link tmx m_idTmx}-&gt;<a
 * href="data.html#fld_dim" class="code">dim</a>.
 * 
 * @param _this
 *          Pointer to this vector mapping operator
 * @return The input dimensionality
 */
INT32 CGEN_PUBLIC CVmap_GetInDim(CVmap* _this)
{
  CHECK_THIS_RV(0);                                                             /* Check this pointer                */
  if(CTmx_IsCompressed(AS(CData,_this->m_idTmx))){
    INT64 mx=0,n=CData_GetNRecs(AS(CData,_this->m_idTmx));
    BYTE *p=CData_XAddr(AS(CData,_this->m_idTmx),0,0);
    INT32 nr=CData_GetRecLen(AS(CData,_this->m_idTmx));
    for(;n;n--,p+=nr) if(*(INT64*)p>mx) mx=*(INT64*)p;
    return mx+1;
  }
  return CData_GetNComps(AS(CData,_this->m_idTmx));                             /* Return # comps. of trafo. matrix  */
}

/**
 * Returns the output dimensionality of this mapping operator. The output
 * dimensionality is defined by {@link tmx m_idTmx}-&gt;<a
 * href="data.html#fld_nrec" class="code">nrec</a>.
 * 
 * @param _this
 *          Pointer to this vector mapping operator
 * @return The output dimensionality
 */
INT32 CGEN_PUBLIC CVmap_GetOutDim(CVmap* _this)
{
  CHECK_THIS_RV(0);                                                             /* Check this pointer                */
  if(CTmx_IsCompressed(AS(CData,_this->m_idTmx))){
    INT64 mx=0,n=CData_GetNRecs(AS(CData,_this->m_idTmx));
    BYTE *p=CData_XAddr(AS(CData,_this->m_idTmx),0,1);
    INT32 nr=CData_GetRecLen(AS(CData,_this->m_idTmx));
    for(;n;n--,p+=nr) if(*(INT64*)p>mx) mx=*(INT64*)p;
    return mx+1;
  }
  return CData_GetNRecs(AS(CData,_this->m_idTmx));                              /* Return # recs. of trafo. matrix   */
}

/*
 * Manual page at vmap.def
 */
INT16 CGEN_PUBLIC CVmap_Setup
(
  CVmap*      _this,
  CData*      idTmx,
  const char* sAop,
  const char* sWop,
  FLOAT64      nZero
)
{
  INT32    m     = 0;                                                            /* Record loop counter               */
  INT32    n     = 0;                                                            /* Record index counter              */
  INT32    N     = 0;                                                            /* Input vector dimensionality       */
  INT32    M     = 0;                                                            /* Output vector dimensionality      */
  INT32    nUsed = 0;                                                            /* Number of used cells in tmx       */
  FLOAT64* lpBuf = NULL;                                                         /* Copy buffer                       */
  CData*  idAux = NULL;                                                         /* Auxilary data instance            */
  INT32 nMaxUsed=0;                                                              /* Max. number of used in's per out. */
  CHECK_THIS_RV(NOT_EXEC);                                                      /* Check this pointer                */
  IFIELD_RESET(CData,"tmx");                                                    /* Create/reset transformation matrix*/
  IFIELD_RESET(CData,"weak_tmx");                                               /* Create/reset weak tmx             */
  _this->m_nAop  = dlp_scalop_code(sAop);                                       /* Get aggregation operation code    */
  _this->m_nWop  = dlp_scalop_code(sWop);                                       /* Get weighting operation code      */
  _this->m_nZero = nZero;                                                       /* Store aggregation neutral element */
  _this->m_nType = CTmx_IsCompressed(idTmx);
  if(!_this->m_nType) _this->m_nType=CData_IsHomogen(idTmx);                    /* Auto detect floating point type   */
  if(_this->m_bDouble) _this->m_nType = T_DOUBLE;                               /* Override type by option           */
  if(_this->m_bFloat ) _this->m_nType = T_FLOAT;                                /* Override type by option           */
  if(_this->m_nType==T_FLOAT)                                                   /* Convert nZero to float            */
  {                                                                             /* >>                                */
    FLOAT32 nZero=_this->m_nZero;                                                 /*   Get nZero as float              */
    if(_this->m_nZero>T_FLOAT_MAX) nZero=T_FLOAT_MAX;                           /*   Upper bound                     */
    if(_this->m_nZero<T_FLOAT_MIN) nZero=T_FLOAT_MIN;                           /*   Lower bound                     */
    _this->m_nZero=nZero;                                                       /*   Copy result back                */
  }                                                                              /* <<                                */
  if (_this->m_nAop<0) return IERROR(_this,VMP_OPCODE,sAop,"scalar",0);         /* Check aggregation operation code  */
  if (_this->m_nWop<0) return IERROR(_this,VMP_OPCODE,sWop,"scalar",0);         /* Check weighting operation code    */
  if(CTmx_IsCompressed(idTmx)){ CData_Copy(_this->m_idTmx,BASEINST(idTmx)); return O_K; }
  N = CData_GetNNumericComps(idTmx);                                            /* Get input vector dimensionality   */
  M = CData_GetNRecs(idTmx);                                                    /* Get output vector dimensionality  */
  lpBuf = (FLOAT64*)dlp_calloc(N,sizeof(FLOAT64));                                /* Allocate copy buffer              */
  CData_Array(AS(CData,_this->m_idTmx),_this->m_nType,N,M);                     /* Allocate transformation matrix    */
  for (m=0; m<M; m++)                                                           /* Loop over records of trafo. matr. */
  {                                                                             /* >>                                */
    CData_DrecFetch(idTmx,lpBuf,m,N,-1);                                        /*   Fetch record from parameter     */
    CData_DrecStore(AS(CData,_this->m_idTmx),lpBuf,m,N,-1);                     /*   Store record to field           */
    CData_DrecFetch(AS(CData,_this->m_idTmx),lpBuf,m,N,-1);                     /*   Refetch record from field       */
    for(n=0;n<N;n++) if(lpBuf[n]!=_this->m_nZero) nUsed++;                      /*   Update used number from refetch */
  }                                                                             /* <<                                */
  dlp_free(lpBuf);                                                              /* Free the copy buffer              */
  /* Weak used tmx matrix ? */                                                  /* --------------------------------- */
  if(nUsed>N*M*_this->m_nWeakThrsh) return O_K;                                 /* Done.                             */
  CData_Reset(_this->m_idWeakTmx,TRUE);                                         /* Reset weak tmx                    */
  idAux=AS(CData,_this->m_idWeakTmx);                                           /* Get Adress of instance for weaktmx*/
  for(m=0;m<M;m++)                                                              /* Loop over all output dim.         */
  {                                                                             /* >>                                */
    char lpsName[16];                                                           /*   Buffer for name of component    */
    snprintf(lpsName,15,"Id%li",(long)m); CData_AddComp(idAux,lpsName,T_INT);   /*   Create index component          */
    snprintf(lpsName,15,"W%li",(long)m); CData_AddComp(idAux,lpsName,_this->m_nType); /*   Create weight component         */
    for(n=0;n<nMaxUsed;n++) *((INT32 *)CData_XAddr(idAux,n,m*2)) = -1;        /*   Initialize index components     */
    for(nUsed=n=0;n<N;n++)                                                      /*   Loop over all input dim.        */
    {                                                                           /*   >>                              */
      FLOAT64 nVal=CData_Dfetch(AS(CData,_this->m_idTmx),m,n);                   /*     Read tmx value                */
      if(nVal==_this->m_nZero) continue;                                        /*     Value not used -> cont.       */
      if(nMaxUsed<=nUsed)                                                       /*     New max. number ?             */
      {                                                                         /*     >>                            */
        INT32 mm;                                                             /*       Second output index         */
        CData_Reallocate(idAux,nUsed+1);                                        /*       Resize instance recordnum.  */
        for(;nMaxUsed<=nUsed;nMaxUsed++) for(mm=0;mm<m;mm++)                    /*       Initialize new index comp.'s*/
          *((INT32 *)CData_XAddr(idAux,nMaxUsed,mm*2)) = -1;                  /*       |                           */
      }                                                                         /*     <<                            */
      *((INT32 *)CData_XAddr(idAux,nUsed,m*2)) = n;                           /*     Set index of input vector     */
      CData_Dstore(idAux,nVal,nUsed,m*2+1);                                     /*     Set map weight                */
      nUsed++;                                                                  /*     Update used number            */
    }                                                                           /*   <<                              */
  }                                                                             /* <<                                */
  return O_K;                                                                   /* Done.                             */
}

/*
 * Manual page at vmap.def
 */
INT16 CGEN_PUBLIC CVmap_SetupI
(
  CVmap*      _this,
  CData*      idImap,
  CData*      idWtab,
  const char* sAop,
  const char* sWop,
  FLOAT64      nZero,
  FLOAT64      nOne
)
{
  INT32  i     = 0;                                                             /* Loop counter                      */
  INT32  M     = 0;                                                             /* Output dimension of map           */
  INT16  nErr  = O_K;                                                           /* Error state                       */
  CData* idAux = NULL;                                                          /* Temporary transformation matrix   */
  CHECK_THIS_RV(NOT_EXEC);                                                      /* Check this pointer                */
  if (CData_IsEmpty(idImap)) return IERROR(_this,ERR_INVALARG,"idImap",0,0);    /* Urgently need index map :(        */
  ICREATEEX(CData,idAux,"CVmap_SetupI.idAux",NULL);                             /* Create temporary table            */
  for (i=0; i<(INT32)CData_GetNRecs(idImap); i++)                                /* Loop over index map               */
    if ((INT32)CData_Dfetch(idImap,i,0)>M)                                       /*   Seek maximum index              */
      M = (INT32)CData_Dfetch(idImap,i,0);                                       /*   ...                             */
  M++;                                                                          /* Dimensionality = max. index +1 !  */
  CData_Array(idAux,T_DOUBLE,CData_GetNRecs(idImap),M);                         /* Allocate temp. trafo. matrix      */
  CData_Fill(idAux,CMPLX(nZero),CMPLX(0.));                                     /* Fill it up with zero weights      */
  for (i=0; i<(INT32)CData_GetNRecs(idImap); i++)                                /* Loop over map records             */
    CData_Dstore(idAux,                                                         /*   Store weight or semiring 1 into */
      CData_IsEmpty(idWtab)?nOne:CData_Dfetch(idWtab,i,0),                      /*   | temp. transformation matrix   */
      (INT32)CData_Dfetch(idImap,i,0),i);                                        /*   |                               */
  nErr = CVmap_Setup(_this,idAux,sAop,sWop,nZero);                              /* Setup                             */
  IDESTROY(idAux);                                                              /* Destroy temp. trafo. matrix       */
  return nErr;                                                                   /* Return CVmap_Setup error state    */
}

/*
 * Manual page at vmap.def
 */
INT16 CGEN_PUBLIC CVmap_Status(CVmap* _this)
{
  CHECK_THIS_RV(NOT_EXEC);                                                      /* Check this pointer                */
  printf("\n"); dlp_fprint_x_line(stdout,'-',dlp_maxprintcols());               /* Print protocol header             */
  printf("\n   Status of instance\n   vmap %s",                                 /* ...                               */
    BASEINST(_this)->m_lpInstanceName);                                         /* |                                 */
  printf("\n"); dlp_fprint_x_line(stdout,'-',dlp_maxprintcols());               /* ...                               */
  printf("\n   Floating point type  : %ld",(long)_this->m_nType);               /* Report floating point type        */
  printf("\n   Input dimensionality : %ld",(long)CVmap_GetInDim(_this)    );    /* Report input dimensionality       */
  printf("\n   Output dimensionality: %ld",(long)CVmap_GetOutDim(_this)   );    /* Report output dimensionality      */
  printf("\n   Aggregation operation: %d" ,(int)_this->m_nAop             );    /* Report aggregation operation      */
  printf(" (%s)"                          ,dlp_scalop_name(_this->m_nAop) );    /* ...                               */
  printf("\n   - Neutral element    : %e" ,(double)_this->m_nZero         );    /* Report neutral element of aggr.op.*/
  printf("\n   Weighting operation  : %d" ,(int)_this->m_nWop             );    /* Report weighting operation        */
  printf(" (%s)"                          ,dlp_scalop_name(_this->m_nWop) );    /* ...                               */
  printf("\n"                                                             );    /* Print a blank line                */
  if (CData_IsEmpty(AS(CData,_this->m_idTmx)) || (                              /* Setup ok                          */
      CTmx_IsCompressed(AS(CData,_this->m_idTmx))!=_this->m_nType &&            /* |                                 */
      CData_IsHomogen(AS(CData,_this->m_idTmx))!=_this->m_nType))               /* |                                 */
    printf("\n   Mapping operator is CORRUPT");                                 /*   Report it                       */
  else                                                                          /* Setup not ok                      */
    printf("\n   Mapping operator is correctly setup.");                        /*   Report it                       */
  printf("\n"); dlp_fprint_x_line(stdout,'-',dlp_maxprintcols());               /* Print protocol footer             */
  if (CData_IsEmpty(AS(CData,_this->m_idTmx)) ||                                /* Setup ok                          */
      CData_IsHomogen(AS(CData,_this->m_idTmx))!=_this->m_nType)                /* |                                 */
    printf("\n   Use <-setup> or <-setup_i> to initialize.");                   /*   Print further-info-hint         */
  else                                                                          /* Setup not ok                      */
    printf("\n   Type <%s.tmx -print;> to see the transformation matrix.",      /*   Print further-info-hint         */
      BASEINST(_this)->m_lpInstanceName);                                       /*   |                               */
  printf("\n");                                                                 /* ...                               */
  return O_K;                                                                   /* Done.                             */
}

/*
 * Manual page at vmap.def
 */
INT16 CGEN_PUBLIC CVmap_Map(CVmap* _this, CData* idSrc, CData* idDst)
{
  CData*   idAux = NULL;                                                        /* Auxilary data instance #1         */
  FLOAT64* lpX   = NULL;                                                        /* Input vector buffer               */
  FLOAT64* lpY   = NULL;                                                        /* Output vector buffer              */
  INT32    i     = 0;                                                           /* Loop counter                      */
  INT32    I     = 0;                                                           /* Numeric dimensionality of idSrc   */
  INT32    K     = 0;                                                           /* Number of records in idSrc        */
  INT32    M     = 0;                                                           /* Output dimensionality of mapping  */
  
  /* Validate */                                                                /* --------------------------------- */
  CHECK_THIS_RV(NOT_EXEC);                                                      /* Check this pointer                */
  if (!idSrc) return IERROR(_this,ERR_NULLARG,"idSrc",0,0);                     /* Check input vector sequence       */
  if (CData_IsEmpty(AS(CData,_this->m_idTmx)))                                  /* Check transformation matrix       */
    return IERROR(_this,VMP_NOTSETUP," (transformation matrix empty)",0,0);     /* ...                               */
  if (CData_IsHomogen(AS(CData,_this->m_idTmx))!=T_DOUBLE &&                    /* Check transformation matrix       */
      CTmx_IsCompressed(AS(CData,_this->m_idTmx))!=T_DOUBLE)
    return IERROR(_this,VMP_NOTSETUP," (transformation matrix corrupt)",0,0);   /* ...                               */
  I = CData_GetNNumericComps(idSrc);                                            /* Get numeric dimensionlty. of input*/
  K = CData_GetNRecs(idSrc);                                                    /* Get number of input vectors       */
  M = CVmap_GetOutDim(_this);                                                   /* Get output dimensionality of map  */
  if (I==0)                                                                     /* No numeric input components       */
  {                                                                             /* >>                                */
    CData_Copy(BASEINST(idDst),BASEINST(idSrc));                                /*   Just copy input to output       */
    return O_K;                                                                 /*   Nothing more 2 B done for this  */
  }                                                                             /* <<                                */
  if(!CData_IsEmpty(AS(CData,_this->m_idWeakTmx)) &&                            /* Weak tmx to use and               */
    CData_GetNComps(AS(CData,_this->m_idWeakTmx))!=M*2)                         /*   Number of comp. don't match     */
    return IERROR(_this,VMP_NOTSETUP," (weak_tmx components don't match)",0,0); /* ...                               */

  /* Initialize */                                                              /* --------------------------------- */
  CREATEVIRTUAL(CData,idSrc,idDst);                                             /* Identical arguments support       */
  lpX = (FLOAT64*)dlp_calloc(I,sizeof(FLOAT64));                                  /* Allocate input vector buffer      */
  lpY = (FLOAT64*)dlp_calloc(M,sizeof(FLOAT64));                                  /* Allocate output vector buffer     */
  ICREATEEX(CData,idAux,"CVmap_Map.idAux",NULL);                                /* Create auxilary data instance #1  */
  CData_Array(idDst,T_DOUBLE,M,K);                                              /* Allocate numeric part of dest.    */
  CData_Fill(idDst,CMPLX(_this->m_nZero),CMPLX(0.));                            /* Initialize it with semiring-zero  */
  for (i=0; i<I; i++)                                                           /* Loop over input components        */
    if (!dlp_is_numeric_type_code(CData_GetCompType(idSrc,i)))                  /*   Current comp. is not numeric    */
    {                                                                           /*   >>                              */
      CData_Select(idAux,idSrc,i,1);                                            /*     Copy component to idAux       */
      CData_Join(idDst,idAux);                                                  /*     Append to destination         */
    }                                                                           /*   <<                              */
    
  /* Vector transformation */                                                   /* --------------------------------- */
  for (i=0; i<K; i++)                                                           /* Loop over vectors                 */
  {                                                                             /* >>                                */
    CData_DrecFetch(idSrc,lpX,i  ,I,-1);                                        /*   Fetch input vector              */
    CVmap_MapVectorD(_this,lpX,lpY,I,M);                                        /*   Transform intput -> output      */
    CData_DrecStore(idDst,lpY,i  ,M,-1);                                        /*   Store output vector             */
  }                                                                             /* <<                                */
  
  /* Clean up */                                                                /* --------------------------------- */
  DESTROYVIRTUAL(idSrc,idDst);                                                  /* Identical arguments support       */
  dlp_free(lpX);                                                                /* Free input vector buffer          */
  dlp_free(lpY);                                                                /* Free output vector buffer         */
  IDESTROY(idAux);                                                              /* Destroy auxilary data instance #1 */
  return O_K;                                                                   /* Done.                             */
}

/**
 * Transforms one vector. Performs the vector transformation as described for
 * method {@link -map CVmap_Map} for one single vector.
 * 
 * @param _this
 *          Pointer to this vector mapping operator
 * @param lpX
 *          Pointer to a buffer containing the input vector
 * @param lpY
 *          Pointer to a buffer to be filled with the output vector, must
 *          <em>not</em> be identical with <code>lpX</code> (in such a case the
 *          behaviour of the method is undefined)
 * @param nXdim
 *          Number of elements in <code>lpX</code>
 * @param nYdim
 *          Number of elements in <code>lpY</code>
 * @param nFtype
 *          Code for floating point type in lpX and lpY
 */
void CGEN_PUBLIC CVmap_MapVector
(
  CVmap*  _this,
  BYTE*   lpX,
  BYTE*   lpY,
  INT32    nXdim,
  INT32    nYdim,
  INT16   nFtype
)
{
  if(nFtype!=_this->m_nType) IERROR(_this,ERR_INVALARG,"nFtype!=_this->m_nType",0,0);
  switch(nFtype){
    case T_FLOAT:  CVmap_MapVectorF(_this,(FLOAT32*)lpX,(FLOAT32*)lpY,nXdim,nYdim); break;
    case T_DOUBLE: CVmap_MapVectorD(_this,(FLOAT64*)lpX,(FLOAT64*)lpY,nXdim,nYdim); break;
  }
}

/* EOF */
