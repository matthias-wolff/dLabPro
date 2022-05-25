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

#if VMAP_FTYPE_CODE == T_FLOAT
  #define VMAP_FTYPE  FLOAT32
  #define DLP_SCALOP  dlp_scalopF
#elif VMAP_FTYPE_CODE == T_DOUBLE
  #define VMAP_FTYPE  FLOAT64
  #define DLP_SCALOP  dlp_scalop
#endif

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
 */
#if VMAP_FTYPE_CODE == T_FLOAT
void CGEN_PUBLIC CVmap_MapVectorF
#else
void CGEN_PUBLIC CVmap_MapVectorD
#endif
(
  CVmap*  _this,
  VMAP_FTYPE* lpX,
  VMAP_FTYPE* lpY,
  INT32    nXdim,
  INT32    nYdim
)
{
  BOOL    f  = 1;                                                               /* First non-zero summand flag       */
  INT32    n  = 0;                                                               /* Input dimension loop counter      */
  INT32    m  = 0;                                                               /* Output dimension loop counter     */
  INT32    N  = 0;                                                               /* Input dimensionality of mapping   */
  INT32    M  = 0;                                                               /* Output dimensionality of mapping  */
  VMAP_FTYPE* W  = NULL;                                                        /* Pointer to transformation matrix  */
  VMAP_FTYPE* Wm = NULL;                                                        /* Pointer to column m of trafo.mtx. */

  /* Initialize */                                                              /* --------------------------------- */
  CHECK_THIS();                                                                 /* Check this pointer                */
  if (!lpY) return;                                                             /* No output buffer, no service      */
  DLPASSERT(lpX!=lpY);                                                          /* Check in-/output ptrs. not equal  */
  for (m=0; m<nYdim; m++) lpY[m]=_this->m_nZero;                                /* Initialize output vector          */
  if (!lpX) return;                                                             /* If no input vector -> all done    */
  N = CVmap_GetInDim(_this);                                                    /* Get mapping input dimensionality  */
  M = CVmap_GetOutDim(_this);                                                   /* Get mapping output dimensionality */
  if (nXdim>N) nXdim = N;                                                       /* Clip input dim. to mapping dim.   */
  if (nYdim>M) nYdim = M;                                                       /* Clip output dim. to mapping dim.  */
  if(CTmx_IsCompressed(AS(CData,_this->m_idTmx))){
    BYTE *bi=(BYTE*)CData_XAddr(AS(CData,_this->m_idTmx),0,0);
    BYTE *bo=(BYTE*)CData_XAddr(AS(CData,_this->m_idTmx),0,1);
    BYTE *bw=(BYTE*)CData_XAddr(AS(CData,_this->m_idTmx),0,2);
    INT64  n=CData_GetNRecs(AS(CData,_this->m_idTmx));
    INT32 nr=CData_GetRecLen(AS(CData,_this->m_idTmx));
    for(;n;n--,bi+=nr,bo+=nr,bw+=nr)
      lpY[*(INT64*)bo] = DLP_SCALOP(lpY[*(INT64*)bo],DLP_SCALOP(*(FLOAT64*)bw,lpX[*(INT64*)bi],_this->m_nWop),_this->m_nAop);
  
  }else if(CData_IsEmpty(AS(CData,_this->m_idWeakTmx)))                               /* Do not use weak tmx               */
  {                                                                             /* >>                                */
    W = (VMAP_FTYPE*)CData_XAddr(AS(CData,_this->m_idTmx),0,0);                 /*   Get pointer to trafo. matrix    */
     if (!W) return;                                                             /*   No trafo. mat., also no service */

    /* Compute generalized scalar product for each y[m] */                      /*   ------------------------------- */
    for (m=0; m<nYdim; m++)                                                     /*   Loop over output dimensions     */
      for (n=0,f=1,Wm=&W[m*N]; n<nXdim; n++)                                    /*     Loop over input dimensions    */
        if (Wm[n]!=_this->m_nZero)                                              /*       Weight of comp. n non-zero  */
        {                                                                       /*       >>                          */
          if (f)                                                                /*         First summand             */
          {                                                                     /*         >>                        */
            lpY[m] = DLP_SCALOP(Wm[n],lpX[n],_this->m_nWop);                    /*           Skip aggregation op     */
            f      = 0;                                                         /*           This WAS the first one..*/
          }                                                                     /*         <<                        */
          else                                                                  /*         Second and further summd's*/
            lpY[m] = DLP_SCALOP(lpY[m],DLP_SCALOP(Wm[n],lpX[n],_this->m_nWop),  /*           Weight & aggregate      */
              _this->m_nAop);                                                   /*           |                       */
        }                                                                       /*       <<                          */
  }else{                                                                        /* << Use weak tmx >>                */
    BYTE*   Id = NULL;                                                          /*   Pointer to index vector         */
    INT32 nRecLen = CData_GetRecLen(AS(CData,_this->m_idWeakTmx));               /*   Get size of one record          */
    INT16 nIType = CData_GetCompType(AS(CData,_this->m_idWeakTmx),0);
    nXdim = CData_GetNRecs(AS(CData,_this->m_idWeakTmx));                       /*   Get max. numb. of out's per in  */
    for (m=0;m<nYdim;m++)                                                       /*   Loop over output dimensions     */
    {                                                                           /*   >>                              */
      Id=CData_XAddr(AS(CData,_this->m_idWeakTmx),0,m*2);                       /*     Get start adress of component */
      switch(nIType){
      case T_LONG:
        for (n=0;n<nXdim && *((INT64*)Id)>=0;n++,Id+=nRecLen)                    /*       Loop over in's for this out */
          if(!n) lpY[m]=DLP_SCALOP(*((VMAP_FTYPE *)(Id+sizeof(INT64))),          /*         First input =>            */
            lpX[*((INT64*)Id)],_this->m_nWop);                                   /*         | Calc out from in        */
          else lpY[m]=DLP_SCALOP(lpY[m],                                        /*         Else => Calc out from     */
            DLP_SCALOP(*((VMAP_FTYPE *)(Id+sizeof(INT64))),                      /*         |                         */
            lpX[*((INT64*)Id)],_this->m_nWop),_this->m_nAop);                    /*         | prev. out and in        */
      break;
      case T_INT:
        for (n=0;n<nXdim && *((INT32*)Id)>=0;n++,Id+=nRecLen)                     /*       Loop over in's for this out */
          if(!n) lpY[m]=DLP_SCALOP(*((VMAP_FTYPE *)(Id+sizeof(INT32))),           /*         First input =>            */
            lpX[*((INT32*)Id)],_this->m_nWop);                                    /*         | Calc out from in        */
          else lpY[m]=DLP_SCALOP(lpY[m],                                        /*         Else => Calc out from     */
            DLP_SCALOP(*((VMAP_FTYPE *)(Id+sizeof(INT32))),                       /*         |                         */
            lpX[*((INT32*)Id)],_this->m_nWop),_this->m_nAop);                     /*         | prev. out and in        */
      break;
      default:
        IERROR(_this,ERR_INVALARG,"weaktmx int type is wheter int nor long",0,0);
      }
    }                                                                           /*   <<                              */
  }                                                                             /* <<                                */
}

#undef VMAP_FTYPE
#undef DLP_SCALOP
