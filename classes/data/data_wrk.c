/* dLabPro class CData (data)
 * - Worker functions
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

#include "dlp_cscope.h" /* Indicate C scope */
#include "dlp_data.h"
#ifndef __NODLPMATH
  #include "dlp_math.h"
#endif /* #ifndef __NODLPMATH */

/* //////////////////////////////////////////////////////////////////// */
/* // Scalar operations                                                 */

/**
 * Execute scalar operation component vs. constant.
 *
 * @param _this   This instance (source and destination)
 * @param nConst  Constant operand for scalar operation
 * @param nOpcode Scalar operation code
 * @param nComp   Component to execute scalar operation
 * @return        O_K if successful, NOT_EXEC otherwise
 */
INT16 CGEN_PROTECTED CData_Scalop_C(CData* _this, COMPLEX64 nConst, INT16 nOpcode, INT32 nComp)
{
  /* Local variables */
  INT32 nR    = 0;                     /* Current record               */
  INT32 nXR   = 0;                     /* Number of records            */
  INT32 nRl   = 0;                     /* Record length                */
  INT16 nType = T_IGNORE;              /* Component data type          */
  BYTE* lpC   = NULL;                  /* Pointer to current data cell */

  /* Local macros */
  #define __CSO(A) for (nR=0,lpC=CData_XAddr(_this,0,nComp);nR<nXR;nR++,lpC+=nRl) \
                     *(A*)lpC=(A)dlp_scalop((FLOAT64)(*(A*)lpC),nConst.x,nOpcode);

  /* Validation */
  CHECK_THIS_RV(NOT_EXEC);
  if (nComp<0 || nComp>=CData_GetNComps(_this)) return NOT_EXEC;
  if (dlp_is_symbolic_type_code(nType=CData_GetCompType(_this,nComp))) return NOT_EXEC;
  if (_this->m_bMark && !CData_CompIsMarked(_this,nComp)) return NOT_EXEC;

  /* Operate */
  nXR = CData_GetNRecs(_this);
  nRl = CData_GetRecLen(_this);
  switch (nType)
  {
  case T_UCHAR  : __CSO(    UINT8); break;
  case T_CHAR   : __CSO(     INT8); break;
  case T_USHORT : __CSO(   UINT16); break;
  case T_SHORT  : __CSO(    INT16); break;
  case T_UINT   : __CSO(   UINT32); break;
  case T_INT    : __CSO(    INT32); break;
  case T_ULONG  : __CSO(   UINT64); break;
  case T_LONG   : __CSO(    INT64); break;
  case T_FLOAT  : __CSO(  FLOAT32); break;
  case T_DOUBLE : __CSO(  FLOAT64); break;
  case T_COMPLEX:
    for (nR=0,lpC=CData_XAddr(_this,0,nComp);nR<nXR;nR++,lpC+=nRl)
      *(COMPLEX64*)lpC=dlp_scalopC(*(COMPLEX64*)lpC,nConst,nOpcode);
    break;
  default      :
    for (nR=0,lpC=CData_XAddr(_this,0,nComp);nR<nXR;nR++,lpC+=nRl)
      CData_Cstore(_this,dlp_scalopC(CData_Cfetch(_this,nR,nComp),nConst,nOpcode),nR,nComp);
  }

  /* That's it */
  #undef __CSO
  return O_K;
}

/**
 *  Execute scalar operation
 *
 *  <h3>Operation Modes</h3>
 *  <ul>
 *    <li>Scalar mode:<br>
 *        if idConst == NULL, all elements of x are concatenated with nConst
 *        using operation nOpcode --OR-- if idConst.dim=1 and idConst.nrec=1,
 *        nConst=idConst[0,0];
 *    <li>Sequence mode:<br>
 *        if idConst.dim=1, an element idSrc[i,j] is concatenated with
 *        idConst[i,0] using operation nOpcode
 *    <li>Vector mode:<br>
 *        if idConst.nrec=1 (idConst is only a record), an element idSrc[i,j]
 *        is concatenated with dConst(j) for all j using operation nOpcode
 *    <li>Table mode:<br>
 *        if idConst.dim&gt;1 and idConst.nrec&gt;1, an element idSrc[i,j] is
 *        concatenated with idConst[i,j] for all i,j using operation nOpcode
 *    <li>if nComp &gt; -1: only this component is used, all other components are
 *        transferred from source to destination without modification
 *    <li>all components j in idConst with j&gt;=dim(idSrc) are ignored and a warning
 *        is given;
 *    <li>all records i in idConst with i&gt;=nrec(idSrc) are ignored and a warning
 *        is given;
 *    <li>if idConst.dim&lt;idSrc.dim, the missing components of idConst are set to
 *        zero and a warning is given
 *    <li>if idConst.nrec&lt;idConst.nrec, the missing records of idConst are set to
 *        zero and a warning is given
 *  </ul>
 *
 *  @param _this    This instance
 *  @param idSrc    Source instance
 *  @param nConst   Single value constant operand
 *  @param dConst   Constant operand table (may be NULL)
 *  @param nOpc     Scalar operation code
 *  @param nType    Destination data type (may be <0, i.e. do not change)
 *  @param nComp    Process only this component from idScr (-1 for processing
 *                  all components)
 */
INT16 CGEN_PUBLIC CData_Scalop_Int
(
  CData*   _this,
  CData*    idSrc,
  COMPLEX64 nConst,
  CData*    idConst,
  INT16     nOpcode,
  INT16     nType,
  INT32     nComp
)
{
  /* Local variables */
  INT32      nR         = 0;              /* Current record                     */
  INT32      nC         = 0;              /* Current component                  */
  INT32      nXR        = 0;              /* Number of records                  */
  INT32      nXC        = 0;              /* Number of components               */
  INT32      nBLen      = 0;              /* Block length                       */
  INT32      k          = 0;
  INT16      nErr       = O_K;            /* Any errors?                        */
  COMPLEX64* lpParam1   = NULL;           /* Buffer #1                          */
  COMPLEX64* lpParam2   = NULL;           /* Buffer #2                          */
  CData*     idConstInt = NULL;           /* Internal copy of constant instance */

  /* Validatation */
  CHECK_THIS_RV(NOT_EXEC);

  if (!idSrc)
    return IERROR(_this,ERR_NULLINST,"idSrc",0,0);
  if (CData_IsEmpty(idSrc))
    return IERROR(_this,DATA_EMPTY,BASEINST(idSrc)->m_lpInstanceName,0,0);
  if (!dlp_scalop_name(nOpcode))
    return IERROR(_this,DATA_BADOPC,nOpcode,0,0);
  if (nComp > CData_GetNComps(idSrc))
    return IERROR(_this,DATA_BADCOMP,nComp,BASEINST(idSrc)->m_lpInstanceName,0);

  /* Overlapping argument support */
  CREATEVIRTUAL(CData,idSrc,_this);
  if (!CData_IsEmpty(idConst))
  {
    IFCHECK printf("\n idConst not empty (duplicate).");
    ICREATEEX(CData,idConstInt,"~",NULL);
    CData_Copy(BASEINST(idConstInt),BASEINST(idConst));
  }
  else IFCHECK printf("\n idConst empty (do not duplicate).");

  /* Initialize */
  nXR  = CData_GetNRecs(idSrc);
  nXC  = CData_GetNComps(idSrc);

  /* Create destination instance */
  if (nType>0 && dlp_is_valid_type_code(nType))
    CData_Tconvert(_this,idSrc,nType);
  else
    CData_Copy(BASEINST(_this),BASEINST(idSrc));

  /* == Operation modes == */
  /* One constant value either from nConst or idConstInt[0,0] */
  if (CData_IsEmpty(idConstInt) || (CData_GetNComps(idConst)==1 && CData_GetNRecs(idConstInt)==1))
  {
    if (!CData_IsEmpty(idConstInt)) nConst=CData_Cfetch(idConstInt,0,0);
    if (nComp<0)
    {
      for (nC=0; nC<nXC; nC++)
        IF_NOK(CData_Scalop_C(_this,nConst,nOpcode,nC))
          nErr=NOT_EXEC;
    }
    else nErr=CData_Scalop_C(_this,nConst,nOpcode,nComp);
  }
  /* Constant operand is a vector */
  else if (CData_GetNRecs(idConstInt)==1)
  {
    if (nComp<0)
    {
      for (nC=0; nC<nXC; nC++)
        IF_NOK(CData_Scalop_C(_this,CData_Cfetch(idConstInt,0,nC),nOpcode,nC))
          nErr=NOT_EXEC;
    }
    else nErr=CData_Scalop_C(_this,CData_Cfetch(idConstInt,0,nComp),nOpcode,nComp);
  }
  /* Constant operand is a sequence of constants */
  else if (CData_GetNComps(idConstInt) == 1)
  {
    IFCHECK printf("\n CData_Scalop_Int: Constant operand is a sequence of constants.");
    nBLen = (CData_GetNBlocks(idSrc)>1)?CData_GetNRecs(idSrc)/CData_GetNBlocks(idSrc):1;

    if
    (
      (nBLen==1 && (CData_GetNRecs(idConstInt)!=CData_GetNRecs(idSrc     ))) ||
      (nBLen!=1 && (CData_GetNBlocks(idSrc   )!=CData_GetNRecs(idConstInt)))
    )
    {
      char* lpBuf = dlp_get_a_buffer();
      sprintf(lpBuf,"%ld!=%ld",(long)CData_GetNRecs(idSrc),(long)CData_GetNRecs(idConstInt));
      IERROR(_this,DATA_DIMMISMATCH,BASEINST(idSrc)->m_lpInstanceName,BASEINST(idConst)->m_lpInstanceName,lpBuf);
      if(nBLen!=1) CData_Reallocate(idConstInt,CData_GetNBlocks(idSrc));
      if(nBLen==1) CData_Reallocate(idConstInt,CData_GetNRecs(idSrc));
    }

    lpParam1 = (COMPLEX64*)dlp_malloc(CData_GetNRecs(idConstInt)*sizeof(COMPLEX64));
    lpParam2 = (COMPLEX64*)dlp_malloc(nXR*sizeof(COMPLEX64));
    if(!lpParam1 || !lpParam2)
    {
      dlp_free(lpParam1);
      dlp_free(lpParam2);
      return IERROR(_this,ERR_NOMEM,0,0,0);
    }

    CData_CcompFetch(idConstInt,lpParam1,0,CData_GetNRecs(idConstInt));
    for (nC=0; nC<nXC; nC++)
    {
      if (dlp_is_numeric_type_code(CData_GetCompType(idSrc,nC)))
      {
        CData_CcompFetch(idSrc,lpParam2,nC,nXR);
        if (nComp<0 || nComp==nC) k=-1;
        for (nR=0; nR<nXR; nR++)
        {
          if (nR%nBLen==0) k++;
          DLPASSERT(k>=0);
          if(dlp_is_complex_type_code(CData_GetCompType(idSrc,nC)))
          {
            lpParam2[nR] = dlp_scalopC(lpParam2[nR],lpParam1[k],nOpcode);
          }
          else
          {
            lpParam2[nR] = CMPLX(dlp_scalop(lpParam2[nR].x,lpParam1[k].x,nOpcode));
          }
          IFCHECK if (nR<20) printf ("\n comp=%d rec=%d block=%d c=%g blen=%d n=%d ",nC,nR,k,lpParam1[k],nBLen,nXR);
        }
        CData_CcompStore(_this,lpParam2,nC,nXR);
      }
    }

    dlp_free(lpParam1);
    dlp_free(lpParam2);
  }
  /* Constant operand is a sequence of vectors */
  else if (CData_GetNComps(idConstInt)>1 && CData_GetNRecs(idConstInt)>1)
  {
    IFCHECK printf("\n CData_Scalop_Int: Constant operand is a sequence of vectors.");

    if
    (
      CData_GetNComps(idConstInt) != CData_GetNComps(idSrc) ||
      CData_GetNRecs(idConstInt)  != CData_GetNRecs(idSrc)
    )
    {
      char* lpBuf = dlp_get_a_buffer();
      sprintf(lpBuf,"%ldx%ld!=%ldx%ld",(long)CData_GetNRecs(idSrc),(long)CData_GetNComps(idSrc),(long)CData_GetNRecs(idConstInt),(long)CData_GetNComps(idConstInt));
      IERROR(_this,DATA_DIMMISMATCH,BASEINST(idSrc)->m_lpInstanceName,BASEINST(idConst)->m_lpInstanceName,lpBuf);
      CData_Reallocate(idConstInt,CData_GetNRecs(idSrc));
      CData_AddNcomps(idConstInt,T_DOUBLE,CData_GetNComps(idSrc)-CData_GetNComps(idConstInt));
    }

    lpParam1 = (COMPLEX64*)dlp_malloc(CData_GetNComps(idSrc)*sizeof(COMPLEX64));
    lpParam2 = (COMPLEX64*)dlp_malloc(CData_GetNComps(idConstInt)*sizeof(COMPLEX64));
    if(!lpParam1 || !lpParam2)
    {
      dlp_free(lpParam1);
      dlp_free(lpParam2);
      return IERROR(_this,ERR_NOMEM,0,0,0);
    }

    for (nR=0; nR<nXR; nR++)
    {
      CData_CrecFetch(idSrc     ,lpParam1,nR,nXC,-1);
      CData_CrecFetch(idConstInt,lpParam2,nR,nXC,-1);

      if (nComp<0)
      {
        for (nC=0; nC<nXC; nC++)
          if(dlp_is_complex_type_code(CData_GetCompType(idSrc,nC)) || dlp_is_complex_type_code(CData_GetCompType(idConstInt,nC))) {
            lpParam1[nC] = dlp_scalopC(lpParam1[nC],lpParam2[nC],nOpcode);
          } else {
            lpParam1[nC] = CMPLX(dlp_scalop(lpParam1[nC].x,lpParam2[nC].x,nOpcode));
          }
      }
      else if(dlp_is_complex_type_code(CData_GetCompType(idSrc,nC)) || dlp_is_complex_type_code(CData_GetCompType(idConstInt,nC))) {
        lpParam1[nComp] = dlp_scalopC(lpParam1[nComp],lpParam2[nComp],nOpcode);
      } else {
        lpParam1[nComp] = CMPLX(dlp_scalop(lpParam1[nComp].x,lpParam2[nComp].x,nOpcode));
      }
      CData_CrecStore(_this,lpParam1,nR,nXC,-1);
    }

    dlp_free(lpParam1);
    dlp_free(lpParam2);
  }
  else DLPASSERT(FMSG("Illegal data for CData_Scalop_Int"));

  /* Clean up */
  DESTROYVIRTUAL(idSrc,_this);
  if (!CData_IsEmpty(idConst)) IDESTROY(idConstInt);

  return nErr;
}

/* -------------------------------------------------------------------------- */
/* Aggregation operations                                                     */

INT16 CGEN_PUBLIC CData_Aggregate_Int
(
  CData* _this, CData* iSrc, CData* iMask, COMPLEX64 dParam, INT16 nOpcode
)
{
  INT32      dim,dim1,blen,nb,n;
  INT16      tp;
  INT32      i,j,l,t;
  COMPLEX64  d        = CMPLX(0);
  COMPLEX64* xp       = NULL;
  COMPLEX64* mp       = NULL;
  FLOAT64*   xpF      = NULL;
  FLOAT64*   mpF      = NULL;
  CData*     iVirt    = NULL;
  INT32      lflag    = 0;
  INT32      nout     = 0;
  INT32      naux     = 0;
  INT32      nMode    = 1;          /* Component mode is default (backward compatibility!) */
  INT16      nRetVal  = O_K;
  BOOL       bComplex = FALSE;

  CHECK_THIS_RV(NOT_EXEC);

  if (_this->m_bRec  ) nMode = 2;
  if (_this->m_bBlock) nMode = 3;

  if (CData_IsEmpty(iSrc)) return IERROR(_this,DATA_EMPTY,"iSrc",0,0);

  dim  = CData_GetNComps(iSrc);
  n    = CData_GetNRecs(iSrc);
  blen = CData_GetNRecsPerBlock(iSrc);
  nb   = CData_GetNBlocks(iSrc);

  iVirt = _this;
  if (_this==iSrc) iVirt = CData_CreateInstance("~virtual");

  /* -- mode-dependent parameter */
  switch(nMode)
  {

  case 1: nout=n;
          naux=dim;
          break;
  case 2: nout=nb;
          naux=n;
          break;
  case 3: nout=blen;
          naux=nb;
          break;
  default: DLPASSERT(FMSG("Bad aggregation mode")); /* Cannot happen */
  }

  /* --- create target data structure: */
  CData_Reset(BASEINST(iVirt),TRUE);

  switch(nMode)
  {
  case 1:
    bComplex = (dParam.y != 0) ? TRUE : FALSE;
    for(i=0;i<CData_GetNComps(iSrc);i++) {
      if(dlp_is_symbolic_type_code(CData_GetCompType(iSrc,i))) {
        lflag++;
        CData_AddComp(iVirt,CData_GetCname(iSrc,i),CData_GetCompType(iSrc,i));
      } else {
        if(dlp_is_complex_type_code(CData_GetCompType(iSrc,i))) {
          bComplex = TRUE;
        }
      }
    }
    CData_InsertComp(iVirt,"aggr",bComplex?T_COMPLEX:T_DOUBLE,0);
    break;

  case 2:
  case 3:
    for (i=0; i<dim; i++)
    {
      tp = CData_GetCompType(iSrc,i);
      if (dlp_is_numeric_type_code(tp)) {
        if((dParam.y != 0.) || dlp_is_complex_type_code(tp)) {
          tp = T_COMPLEX;
          bComplex = TRUE;
        } else {
          tp = T_DOUBLE;
        }
      }
      CData_AddComp(iVirt,CData_GetCname(iSrc,i),tp);
    }
    break;
  }

  IF_NOK(CData_Allocate(iVirt,nout))
  {
    if (iVirt!=_this) IDESTROY(iVirt);
    return NOT_EXEC;
  }

  CData_CopyDescr(iVirt,iSrc);

  /* -- allocate aux. memory */
  if (iMask) {
    if(bComplex) xp  = (COMPLEX64*)dlp_calloc(naux+CData_GetNRecs(iMask),sizeof(COMPLEX64));
    else         xpF = (FLOAT64*)dlp_calloc(naux+CData_GetNRecs(iMask),sizeof(FLOAT64));
  } else {
    if(bComplex) xp  = (COMPLEX64*)dlp_calloc(naux,sizeof(COMPLEX64));
    else         xpF = (FLOAT64*)dlp_calloc(naux,sizeof(FLOAT64));
  }
  if ((bComplex && (xp==NULL)) || (!bComplex && (xpF==NULL)))
  {
    if (iVirt!=_this) IDESTROY(iVirt);
    return IERROR(_this,ERR_NOMEM,0,0,0);
  }

  /* --- isolate mask */
  mp = NULL;
  if (iMask && !CData_IsEmpty(iMask))
  {
    if(bComplex) mp  = &xp[nb];
    else         mpF = &xpF[nb];
    i  = 0;         /* MWX 01-06-18: Bug?? */
    do
    {
      i++;
      if(bComplex) CData_CcompFetch(iMask,mp,i,CData_GetNRecs(iMask));
      else         CData_DcompFetch(iMask,mpF,i,CData_GetNRecs(iMask));
    } while (!dlp_is_numeric_type_code(CData_GetCompType(_this,i)));
  }

  switch (nMode)
  {
  case 1: /* Component aggregation */
    for (t=0; t<n; t++)
    {
      if(bComplex) {
        dim1 = CData_CrecFetch(iSrc,xp,t,dim,-1);
        if((nRetVal = dlp_aggropC(xp,mp,dParam,dim1,0,1,nOpcode,&d)) != O_K)
          IERROR(_this,DATA_NOSUPPORT,"Aggregation","",0);
        CData_Cstore(iVirt,d,t,0);
      } else {
        dim1 = CData_DrecFetch(iSrc,xpF,t,dim,-1);
        if((nRetVal = dlp_aggrop(xpF,mpF,dParam.x,dim1,0,1,nOpcode,&d.x)) != O_K)
          IERROR(_this,DATA_NOSUPPORT,"Aggregation","",0);
        CData_Dstore(iVirt,d.x,t,0);
      }
    }
    if (lflag==1) CData_CopyLabels(iVirt,iSrc);
    break;

  case 2: /* Record aggregation */
    for (j=0; j<dim; j++)
    {
      if (dlp_is_numeric_type_code(CData_GetCompType(iSrc,j)))
      {
        if(bComplex) {
          CData_CcompFetch(iSrc,xp,j,n);
          i=0;
          for (l=0; l<n; l+=blen)
          {
            if((nRetVal = dlp_aggropC(&xp[l],mp,dParam,blen,0,1,nOpcode,&d)) != O_K)
              IERROR(_this,DATA_NOSUPPORT,"Aggregation","",0);
            CData_Cstore(iVirt,d,i++,j);
          }
        } else {
          CData_DcompFetch(iSrc,xpF,j,n);
          i=0;
          for (l=0; l<n; l+=blen)
          {
            if((nRetVal = dlp_aggrop(&xpF[l],mpF,dParam.x,blen,0,1,nOpcode,&d.x)) != O_K)
              IERROR(_this,DATA_NOSUPPORT,"Aggregation","",0);
            CData_Dstore(iVirt,d.x,i++,j);
          }
        }
      }
    }
    break;

  case 3: /* Block aggregation */
    for (l=0; l<blen; l++)
      for (j=0; j<dim; j++)
      {
        if (dlp_is_numeric_type_code(CData_GetCompType(iSrc,j)))
        {
          if(bComplex) {
            CData_CijFetch(iSrc,xp,l,j,nb);
            if((nRetVal = dlp_aggropC(xp,mp,dParam,nb,0,1,nOpcode,&d)) != O_K)
              IERROR(_this,DATA_NOSUPPORT,"Aggregation","",0);
            CData_Cstore(iVirt,d,l,j);
          } else {
            CData_DijFetch(iSrc,xpF,l,j,nb);
            if((nRetVal = dlp_aggrop(xpF,mpF,dParam.x,nb,0,1,nOpcode,&d.x)) != O_K)
              IERROR(_this,DATA_NOSUPPORT,"Aggregation","",0);
            CData_Dstore(iVirt,d.x,l,j);
          }
        }
      }
      /* BUG 01-06-18: There is no check for a label component before! */
      if (lflag==1) CData_CopyLabels(iVirt,iSrc);

      break;
  }

  /* -- Clean up */
  if(bComplex) dlp_free(xp)
  else         dlp_free(xpF)

  if (iSrc==_this)
  {
    CData_Copy(BASEINST(_this),BASEINST(iVirt));
    IDESTROY(iVirt);
  }

  return O_K;
}

/* -------------------------------------------------------------------------- */
/* String operations                                                          */

/*
 * Manual page in data.def
 */
INT16 CGEN_PUBLIC CData_Strop
(
  CData* _this, CData* idSrc, const char* sParam, const char* sOpname
)
{
  INT16 nOpc = -1;
  INT16 nErr = O_K;

  CHECK_THIS_RV(NOT_EXEC);

  if ((nOpc=dlp_strop_code(sOpname))<0)
    return IERROR(_this,DATA_OPCODE,sOpname,"string",0);
  if (!idSrc)
  {
    CData_Reset(BASEINST(_this),TRUE);
    return O_K;
  }

  /* NOTE: MWX 2004-01-30
           This is the reference implementation of /mark support. It demonstrates
           a universal procedure of handling marks by splicing the input data and
           handle marked and unmarked parts separately.
  --> */
  if (_this->m_bMark)
  {
    /* Local variables */
    INT32   nSelm = -1;       /* Start element */
    INT32   nEelm = -1;       /* End element   */
    BOOL   bMark = FALSE;
    CData* idAux = NULL;

    /* Validation and initialization */
    if (!idSrc->m_markMap)
      return
        IERROR(_this,DATA_NOMARK,"elements",BASEINST(idSrc)->m_lpInstanceName,
          0);
    CREATEVIRTUAL(CData,idSrc,_this);
    ICREATE(CData,idAux,NULL);

    /* Handle mark modes */
    switch(idSrc->m_markMode)
    {
    case CDATA_MARK_RECS:
      CData_Reset(BASEINST(_this),TRUE);
      CData_CopyDescr(_this,idSrc);
      for (nSelm=0, nEelm=1; nEelm<=CData_GetNRecs(idSrc); )
      {
        bMark=CData_RecIsMarked(idSrc,nSelm);
        while (nEelm<CData_GetNRecs(idSrc) && CData_RecIsMarked(idSrc,++nEelm)==bMark) {};
        IFCHECK printf("\n Recs %ld..%ld: %smarked",(long)nSelm,(long)nEelm-1,bMark?"":"not ");
        ISETOPTION(idAux,"/rec");
        CData_Select(idAux,idSrc,nSelm,nEelm-nSelm);
        IRESETOPTIONS(idAux);
        if (bMark) CData_Strop(idAux,idAux,sParam,sOpname);
        CData_Cat(_this,idAux);
        nSelm=nEelm++;
      }
      break;
    case CDATA_MARK_COMPS:
      CData_Reset(BASEINST(_this),TRUE);
      CData_CopyDescr(_this,idSrc);
      for (nSelm=0, nEelm=1; nEelm<=CData_GetNComps(idSrc); )
      {
        bMark=CData_CompIsMarked(idSrc,nSelm);
        while (nEelm<CData_GetNComps(idSrc) && CData_CompIsMarked(idSrc,++nEelm)==bMark) {};
        IFCHECK printf("\n Comps %ld..%ld: %smarked",(long)nSelm,(long)nEelm-1,bMark?"":"not ");
        CData_Select(idAux,idSrc,nSelm,nEelm-nSelm);
        if (bMark) CData_Strop(idAux,idAux,sParam,sOpname);
        CData_Join(_this,idAux);
        nSelm=nEelm++;
      }
      break;
    case CDATA_MARK_BLOCKS: nErr=DATA_BADMARK; IERROR(_this,DATA_BADMARK,"blocks",0,0); break;
    case CDATA_MARK_CELLS : nErr=DATA_BADMARK; IERROR(_this,DATA_BADMARK,"cells" ,0,0); break;
    default               : DLPASSERT(FMSG("Unknown mark mode."));
    }

    /* Clean up */
    IDESTROY(idAux);
    CData_CopyMark(_this,idSrc);
    DESTROYVIRTUAL(idSrc,_this);
    return nErr;
  }
  /* <-- */
  else
  {
    char sBuf[L_SSTR+1];
    char sBuf2[L_SSTR+1];
    INT32 nXRec  = 0;
    INT32 nXCmp  = 0;
    INT32 i      = 0;
    INT32 j      = 0;
    BOOL bLwr   = FALSE;
    BOOL bRight = FALSE;

    CREATEVIRTUAL(CData,idSrc,_this);

    switch (nOpc)
    {

    case SOP_LEN:
    {
      /* Make destination structure */
      nXRec = CData_GetNRecs(idSrc);
      nXCmp = CData_GetNComps(idSrc);

      CData_Reset(BASEINST(_this),TRUE);
      CData_AddNcomps(_this,T_SHORT,nXCmp);
      CData_AllocateUninitialized(_this,nXRec);
      CData_CopyCnames(_this,idSrc,0,0,nXCmp);
      CData_CopyDescr(_this,idSrc);

      /* Calculate string lengths */
      for (i=0; i<nXRec; i++)
        for (j=0; j<nXCmp; j++)
        {
          INT16 nCType = CData_GetCompType(idSrc,j);
          if (!dlp_is_symbolic_type_code(nCType)) CData_Dstore(_this,-1,i,j);
          else
          {
            INT16 nLen = (INT16)dlp_strlen((char*)CData_XAddr(idSrc,i,j));
            if (nLen>nCType) nLen=nCType;
            CData_Dstore(_this,nLen,i,j);
          }
        }
      break;
    }

    case SOP_HASH:
    case SOP_CHASH:
    {
#ifndef __NODLPMATH

      if (dlp_strcmp(sParam,"CRC-32")==0)
      {
        void*     lpH    = NULL;
        UINT32 nCrc32 = 0;

        /* Make destination structure */
        nXRec = CData_GetNRecs(idSrc);
        nXCmp = CData_GetNComps(idSrc);

        CData_Reset(BASEINST(_this),TRUE);
        if (nOpc==SOP_CHASH)
        {
          CData_AddNcomps(_this,10,nXCmp);
          CData_AllocateUninitialized(_this,nXRec);
          CData_CopyCnames(_this,idSrc,0,0,nXCmp);
          CData_CopyDescr(_this,idSrc);
        }
        else
        {
          CData_AddComp(_this,"CRC",10);
          CData_AllocateUninitialized(_this,1);
        }

        /* Calculate CRC32 hashes */
        lpH = dlm_crc32_init();
        for (i=0; i<nXRec; i++)
          for (j=0; j<nXCmp; j++)
          {
            INT16 nCType = CData_GetCompType(idSrc,j);
            if (!dlp_is_symbolic_type_code(nCType))
            {
              if (nOpc==SOP_CHASH) CData_Dstore(_this,-1,i,j);
            }
            else
            {
              dlp_memset(sBuf,0,L_SSTR+1);
              dlp_memmove(sBuf,CData_XAddr(idSrc,i,j),nCType);
              dlm_crc32_add(lpH,(const BYTE*)sBuf,dlp_strlen(sBuf));
              if (nOpc==SOP_CHASH)
              {
                sprintf(sBuf2,"%08lX",(unsigned long)dlm_crc32(sBuf));
                CData_Sstore(_this,sBuf2,i,j);
              }
            }
          }
        dlm_crc32_finalize(lpH,(BYTE*)&nCrc32,sizeof(nCrc32));
        if (nOpc==SOP_HASH)
        {
          sprintf(sBuf2,"%08lX",(unsigned long)nCrc32);
          CData_Sstore(_this,sBuf2,0,0);
        }
        break;
      }
      else IERROR(_this,DATA_OPCODE,sParam,"hashing",0);

#else /* #ifndef __NODLPMATH */

      IERROR(_this,ERR_NOTSUPPORTED,"String operation 'CRC-32'",0,0);

#endif /* #ifndef __NODLPMATH */
      break;
    }

    case SOP_CMP:
    case SOP_SEARCH:
    {
      /* Make destination structure */
      nXRec = CData_GetNRecs(idSrc);
      nXCmp = CData_GetNComps(idSrc);

      CData_Reset(BASEINST(_this),TRUE);
      CData_AddNcomps(_this,T_SHORT,nXCmp);
      CData_AllocateUninitialized(_this,nXRec);
      CData_CopyCnames(_this,idSrc,0,0,nXCmp);
      CData_CopyDescr(_this,idSrc);

      /* Compare strings */
      for (i=0; i<nXRec; i++)
        for (j=0; j<nXCmp; j++)
        {
          INT16 nCType = CData_GetCompType(idSrc,j);
          if (!dlp_is_symbolic_type_code(nCType)) CData_Dstore(_this,-1,i,j);
          else if (nOpc==SOP_CMP)
            CData_Dstore(_this,dlp_strcmp(sParam,(char*)CData_XAddr(idSrc,i,j)),i,j);
          else
          {
            char* lpPos = strstr((char*)CData_XAddr(idSrc,i,j),sParam);
            CData_Dstore(_this,lpPos==NULL?-1:lpPos-(char*)CData_XAddr(idSrc,i,j),i,j);
          }
        }
      break;
    }

    case SOP_LWR: bLwr=TRUE;
    /* no break */
    case SOP_UPR:
    {
      CData_Copy(BASEINST(_this),BASEINST(idSrc));
      nXRec = CData_GetNRecs(idSrc);
      nXCmp = CData_GetNComps(idSrc);

      for (i=0; i<nXRec; i++)
        for (j=0; j<nXCmp; j++)
        {
          INT16 nCType = CData_GetCompType(idSrc,j);
          if (dlp_is_symbolic_type_code(nCType))
          {
            dlp_strcpy(sBuf,(char*)CData_XAddr(idSrc,i,j));
            if (bLwr) dlp_strlwr(sBuf); else dlp_strupr(sBuf);
            CData_Sstore(_this,sBuf,i,j);
          }
        }
      break;
    }

    case SOP_RIGHT: bRight=TRUE;
    /* no break */
    case SOP_LEFT :
    {
      short   nLen  = 0;
      INT16 nSize = 0;

      if (sscanf(sParam,"%hd",&nLen)!=1) return NOT_EXEC;
      if (nLen<-L_SSTR || nLen>L_SSTR) return NOT_EXEC;

      CData_Copy(BASEINST(_this),BASEINST(idSrc));
      nXRec = CData_GetNRecs(idSrc);
      nXCmp = CData_GetNComps(idSrc);

      for (i=0; i<nXRec; i++)
        for (j=0; j<nXCmp; j++)
        {
          INT16 nCType = CData_GetCompType(idSrc,j);
          if (dlp_is_symbolic_type_code(nCType))
          {
            dlp_strcpy(sBuf,(char*)CData_XAddr(idSrc,i,j));
            nSize = (INT16)(nLen < 0 ? (INT16)dlp_strlen(sBuf)+nLen : nLen);
            if (nSize<nCType)
            {
              if (bRight) memmove(sBuf,&sBuf[strlen(sBuf)-nSize],nSize+1);
              else sBuf[nSize]=0;
              CData_Sstore(_this,sBuf,i,j);
            }
          }
        }
      break;
    }

    case SOP_REPLACE:
    {
      /* HACK: Does NOT make sure that replaced string fits into buffer!!! */
      char  cDel[2];
      char* lpKey;
      char* lpRpl;

      /* First character of sParam is interpreted as delimiter between key and replacement string */
      if (dlp_strlen(sParam)<2) return NOT_EXEC;
      cDel[0] = sParam[0];
      cDel[1] = 0;

      dlp_strncpy(sBuf2,&sParam[1],255);
      lpKey=strtok(sBuf2,cDel);
      lpRpl=strtok(NULL,cDel);

      CData_Copy(BASEINST(_this),BASEINST(idSrc));
      nXRec = CData_GetNRecs(idSrc);
      nXCmp = CData_GetNComps(idSrc);

      for (i=0; i<nXRec; i++)
        for (j=0; j<nXCmp; j++)
        {
          INT16 nCType = CData_GetCompType(idSrc,j);
          if (dlp_is_symbolic_type_code(nCType))
          {
            dlp_strcpy(sBuf,(char*)CData_XAddr(idSrc,i,j));
            dlp_strreplace(sBuf,lpKey,lpRpl);
            CData_Sstore(_this,sBuf,i,j);
          }
        }
      break;
    }

    case SOP_RCAT:
    {
      CData_Reset(BASEINST(_this),TRUE);
      nXRec = CData_GetNRecs(idSrc);
      nXCmp = CData_GetNComps(idSrc);

      /* Aggregated string may be much longer than component
         --> go it comp by comp. and adjust symbolic type */
      for (j=0; j<nXCmp; j++)
      {
        if (dlp_is_numeric_type_code(CData_GetCompType(idSrc,j)))
        {
          CData_AddComp(_this,CData_GetCname(idSrc,j),CData_GetCompType(idSrc,j));
          if (CData_GetNRecs(_this)==0) CData_Allocate(_this,1);
        }
        else
        {
          /* Aggregate strings in component j into sBuf */
          dlp_memset(sBuf,0,256);
          for (i=0; i<nXRec; i++)
          {
            dlp_memset(sBuf2,0,256);
            dlp_strncpy(sBuf2,(char*)CData_XAddr(idSrc,i,j),CData_GetCompType(idSrc,j));
            if (dlp_strlen(sBuf2)+dlp_strlen(sBuf)<254) dlp_strcat(sBuf,sBuf2);
            else break;
          }
          CData_AddComp(_this,CData_GetCname(idSrc,(INT32)j),(INT16)(dlp_strlen(sBuf)+1));
          if (CData_GetNRecs(_this)==0) CData_Allocate(_this,1);
          CData_Sstore(_this,sBuf,0,j);
        }
      }
      break;
    }

    case SOP_CCAT:
    {
      CData_Reset(BASEINST(_this),TRUE);
      nXRec = CData_GetNRecs(idSrc);
      nXCmp = CData_GetNComps(idSrc);
      CData_AddComp(_this,"ccat",L_SSTR);
      CData_AllocateUninitialized(_this,nXRec);
      for (i=0; i<nXRec; i++)
      {
        sBuf[0]='\0';
        for (j=0; j<nXCmp; j++)
        {
          if (j>0 && dlp_strlen(sParam)) strncat(sBuf,sParam,L_SSTR);
          dlp_printx(sBuf2,CData_XAddr(idSrc,i,j),CData_GetCompType(idSrc,j)
            ,0,0,TRUE);
          dlp_strtrimleft(dlp_strtrimright(sBuf2));
          if (dlp_strlen(sBuf2)) strncat(sBuf,sBuf2,L_SSTR);
        }
        CData_Sstore(_this,sBuf,i,0);
      }
      break;
    }

    case SOP_SPLIT:
    case SOP_SPLITALL:
    case SOP_SPLITD:
    case SOP_SPLITP:
    {
      CData* iAux = NULL;

      ICREATE(CData,iAux,NULL);
      CData_Reset(BASEINST(_this),TRUE);
      nXRec = CData_GetNRecs(idSrc);
      nXCmp = CData_GetNComps(idSrc);

      for (j=0; j<nXCmp; j++)
      {
        if (dlp_is_numeric_type_code(CData_GetCompType(idSrc,j)))
        {
          /* Copy numeric components */
          CData_Select(iAux,idSrc,j,1);
          CData_Join(_this,iAux);
        }
        else
        {
          INT32 nCmpDst = 0;
          INT32 nToken  = 0;
          char* tx      = NULL;

          /* Split symbolic components */
          CData_Reset(BASEINST(iAux),TRUE);
          CData_AddComp(_this,CData_GetCname(idSrc,j),CData_GetCompType(idSrc,j));
          if (CData_GetNRecs(_this)==0) CData_Allocate(_this,CData_GetNRecs(idSrc));
          nCmpDst = CData_GetNComps(_this)-1;

          if (nOpc==SOP_SPLITP)
            CData_AddComp(_this,CData_GetCname(idSrc,j),CData_GetCompType(idSrc,j));

          for (i=0; i<nXRec; i++)
          {
            char* lpBuf = sBuf;
            char  del[2];
            strncpy(lpBuf,(const char*)CData_XAddr(idSrc,i,j),255);
            if (dlp_strlen(lpBuf)==0) continue;
            if (nOpc==SOP_SPLIT || nOpc==SOP_SPLITD || nOpc==SOP_SPLITALL)
            {
              tx = (nOpc==SOP_SPLITALL||nOpc==SOP_SPLITD) ? dlp_strsep(&lpBuf,sParam,del) : strtok(lpBuf,sParam);
              nToken = 0;
              while (tx)
              {
                if (nOpc==SOP_SPLITALL || nOpc==SOP_SPLIT || dlp_strlen(tx)>0)
                {
                  if (nCmpDst+nToken>=CData_GetNComps(_this))
                    CData_AddComp(_this,CData_GetCname(idSrc,j),CData_GetCompType(idSrc,j));

                  CData_Sstore(_this,tx,i,nCmpDst+nToken);
                  nToken++;
                }

                if (nOpc==SOP_SPLITD && del[0]!='\0')
                {
                  del[1]='\0';
                  if (nCmpDst+nToken>=CData_GetNComps(_this))
                    CData_AddComp(_this,CData_GetCname(idSrc,j),CData_GetCompType(idSrc,j));

                  CData_Sstore(_this,del,i,nCmpDst+nToken);
                  nToken++;
                }

                tx = (nOpc==SOP_SPLITALL||nOpc==SOP_SPLITD) ? dlp_strsep(&lpBuf,sParam,del) : strtok(NULL,sParam);
              }
            }
            else
            {
              char sPath[L_PATH];
              char sFile[L_PATH];
              dlp_splitpath(lpBuf,sPath,sFile);
              CData_Sstore(_this,sPath,i,nCmpDst  );
              CData_Sstore(_this,sFile,i,nCmpDst+1);
            }
          }
        }
      }

      IDESTROY(iAux);
      break;
    }
    case SOP_TRIM:
    {
      CData_Copy(BASEINST(_this),BASEINST(idSrc));
      nXRec = CData_GetNRecs(idSrc);
      nXCmp = CData_GetNComps(idSrc);

      for (j=0; j<nXCmp; j++)
      {
        INT16 nCtype = CData_GetCompType(idSrc,j);
        if (dlp_is_symbolic_type_code(nCtype))
          for (i=0; i<nXRec; i++)
          {
            char* tx = NULL;
            char  sBuf[L_SSTR+1];
            dlp_strcpy(sBuf,CData_Sfetch(_this,i,j));
            sBuf[nCtype]='\0';
            for (; *sBuf; )
              if (dlp_charin(*sBuf,sParam))
                memmove(sBuf,&sBuf[1],dlp_strlen(sBuf));
              else break;
            for (tx=&sBuf[dlp_strlen(sBuf)-1]; tx>sBuf; tx--)
              if (dlp_charin(*tx,sParam))
                *tx='\0';
              else break;
            CData_Sstore(_this,sBuf,i,j);
          }
      }
      break;
    }
    default:
      IERROR(_this,DATA_OPCODE,sOpname,"string",0);
      nErr=DATA_OPCODE;
    }

    DESTROYVIRTUAL(idSrc,_this);
    return nErr;
  }
}

/**
 */
INT16 CGEN_PUBLIC CData_Compress(CData* _this, CData* iSrc, INT32 nComp)
{
  INT16     nTypeCode = 0;
  INT32     i         = 0;
  INT32     nRecs     = 0;
  INT32     nCount    = 0;
  INT32     nIdx      = 0;
  COMPLEX64 dVal1     = CMPLX(0);
  COMPLEX64 dVal2     = CMPLX(0);
  char*     lpVal1    = NULL;
  char*     lpVal2    = NULL;

  CHECK_THIS_RV(NOT_EXEC);

  if (CData_IsEmpty(iSrc))
    return IERROR(_this,DATA_EMPTY,BASEINST(iSrc)->m_lpInstanceName,0,0);
  if (CData_GetNComps(iSrc)<=nComp)
    return IERROR(_this,DATA_BADCOMP,nComp,BASEINST(iSrc)->m_lpInstanceName,0);

  /* Input and output instances identical? */
  CREATEVIRTUAL(CData,iSrc,_this);

  nRecs     = CData_GetNRecs(iSrc);
  nTypeCode = CData_GetCompType(iSrc,nComp);

  /* prepare destination data instance */
  CData_Reset(BASEINST(_this),TRUE);
  CData_AddComp(_this,CData_GetCname(iSrc,nComp),nTypeCode);
  CData_AddComp(_this,"ind_",T_LONG);
  CData_AddComp(_this,"n___",T_LONG);
  CData_Allocate(_this,nRecs);

  /* compress data depending on type of component */
  if(dlp_is_numeric_type_code(nTypeCode))
  {
    dVal1 = CData_Cfetch(iSrc,0,nComp);
    CData_Cstore(_this,dVal1,0,0);
    CData_Dstore(_this,0,0,1);

    for(i=0; i<nRecs; i++)
    {
      dVal2 = CData_Cfetch(iSrc,i,nComp);

      if(!CMPLX_EQUAL(dVal1,dVal2))
      {
        dVal1 = dVal2;
        CData_Dstore(_this,nCount,nIdx++,2);
        CData_Cstore(_this,dVal1,nIdx,0);
        CData_Dstore(_this,i,nIdx,1);
        nCount = 0;
      }
      nCount++;
    }
    CData_Cstore(_this,CData_Cfetch(_this,1,1),0,2);
    CData_Dstore(_this,nCount,nIdx,2);
  }
  else
  {
    lpVal1 = (char*)CData_XAddr(iSrc,0,nComp);
    CData_Sstore(_this,lpVal1,0,0);
    CData_Dstore(_this,0,0,1);

    for(i=0; i<nRecs; i++)
    {
      lpVal2 = (char*)CData_XAddr(iSrc,i,nComp);
      if(strncmp(lpVal1,lpVal2,nTypeCode) != NULL)
      {
        lpVal1 = lpVal2;
        CData_Dstore(_this,nCount,nIdx++,2);
        CData_Sstore(_this,lpVal1,nIdx,0);
        CData_Dstore(_this,i,nIdx,1);
        nCount = 0;
      }
      nCount++;
    }
    CData_Cstore(_this,CData_Cfetch(_this,1,1),0,2);
    CData_Dstore(_this,nCount,nIdx,2);
  }

  CData_CopyDescr(_this,iSrc);
  CData_SetNRecs(_this,nIdx+1);
  /* CData_Realloc(_this,nIdx); */

  DESTROYVIRTUAL(iSrc,_this);

  return (O_K);
}

/*
 * Manual page at data.def
 */
INT16 CGEN_PUBLIC CData_Expand
(
  CData* _this,
  CData* idSrc,
  INT32   nIcE,
  INT32   nIcS,
  INT32   nIcL
)
{
  INT32        nC       = -1;
  INT32        nXC      = -1;
  const char* lpsIname = NULL;
  INT16       nErr     = O_K;
  CData*      idAux    = NULL;

  CHECK_THIS_RV(NOT_EXEC);

  /* Initialize */
  nXC      = CData_GetNComps(idSrc);
  lpsIname = BASEINST(idSrc)->m_lpInstanceName;

  /* Validate */
  if (CData_IsEmpty(idSrc))  return IERROR(_this,DATA_EMPTY,lpsIname,0,0);
  if (nIcE >= nXC         ) return IERROR(_this,DATA_BADCOMP,nIcE,lpsIname,0);
  if (nIcS >= nXC         ) return IERROR(_this,DATA_BADCOMP,nIcS,lpsIname,0);
  if (nIcL >= nXC         ) return IERROR(_this,DATA_BADCOMP,nIcL,lpsIname,0);
  if (nIcS < 0 && nIcL < 0) return IERROR(_this,DATA_BADCOMP,nIcS,lpsIname,0);


  /* NO RETURNS BEYOND THIS POINT! */
  CREATEVIRTUAL(CData,idSrc,_this);
  CData_Reset(BASEINST(_this),TRUE);

  if (nIcE >= 0)
    nErr = CData_ExpandComp(_this,idSrc,nIcE,nIcS,nIcL);
  else
  {
    ICREATEEX(CData,idAux,"CData_Expand.idAux",NULL);
    for (nC=0; nC<nXC; nC++)
      if (nC!=nIcS && nC!=nIcL)
      {
        if (OK(CData_ExpandComp(idAux,idSrc,nC,nIcS,nIcL)))
          CData_Join(_this,idAux);
        else
          nErr = NOT_EXEC;
      }
    IDESTROY(idAux);
  }

  CData_CopyDescr(_this,idSrc);
  DESTROYVIRTUAL(idSrc,_this);
  DLP_CHECK_MEMINTEGRITY;      /* Check for memory demages */
  return nErr;
}

/**
 * RLE expansion of one component. There are no checks performed.
 */
INT16 CGEN_PRIVATE CData_ExpandComp
(
  CData* _this,
  CData* idSrc,
  INT32   nIcE,
  INT32   nIcS,
  INT32   nIcL
)
{
  INT32  i        = 0;
  INT32  k        = 0;
  INT32  iFrom    = 0;
  INT32  nFrom    = 0;
  INT32  nRecs    = 0;
  INT32  nRecsNew = 0;
  INT32  nRln     = 0;
  INT32  nRlnNew  = 0;
  BYTE* lpFrom   = NULL;
  BYTE* lpTo     = NULL;

  CHECK_THIS_RV(NOT_EXEC);
  DLPASSERT(_this!=idSrc);


  nRecs = CData_GetNRecs(idSrc);

  /* calculate new number of records */
  if (nIcS >= 0 && nIcL >= 0)
    nRecsNew = (INT32)CData_Dfetch(idSrc,nRecs-1,nIcS)+(INT32)CData_Dfetch(idSrc,nRecs-1,nIcL);
  else if (nIcS >= 0 && nIcL < 0)
    nRecsNew = (INT32)CData_Dfetch(idSrc,nRecs-1,nIcS);
  else if (nIcS <  0 && nIcL >= 0)
  {
    FLOAT64 d=0;
    for (i=0; i<nRecs;i++) d += CData_Dfetch(idSrc,i,nIcL);
    nRecsNew = (INT32)d;
  }
  else
    return IERROR(_this,DATA_BADCOMP,nIcS,BASEINST(idSrc)->m_lpInstanceName,0);

  /* prepare output instance */
  CData_Reset(BASEINST(_this),TRUE);
  CData_AddComp(_this,CData_GetCname(idSrc,nIcE),CData_GetCompType(idSrc,nIcE));
  CData_Allocate(_this,nRecsNew);

  /* do expand */
  lpFrom  = CData_XAddr(idSrc,0,nIcE);
  nRlnNew = CData_GetRecLen(_this);
  nRln    = CData_GetRecLen(idSrc);

  for (i=0; i<nRecs; i++)
  {
    if (nIcS >= 0)   iFrom = (INT32) CData_Dfetch (idSrc,i,nIcS);
    else             iFrom += nFrom;

    if (nIcL >= 0) nFrom = (INT32) CData_Dfetch (idSrc,i,nIcL);
    else             nFrom = (INT32) CData_Dfetch (idSrc,i+1,nIcS)-iFrom;

    lpTo = CData_XAddr  (_this,iFrom,0);

    if (lpTo != NULL)
      for (k=0; k<nFrom;k++)
      {
        if(lpTo>=CData_XAddr(_this,0,0)+nRecsNew*nRlnNew)
        {
          IERROR(_this,DATA_INTERNAL,"Inconsistent data.",0,0);
          break;
        }
        dlp_memmove(lpTo,lpFrom,nRlnNew);
        lpTo += nRlnNew;
      }
    lpFrom += nRln;
  }
  return O_K;
}


/*
 * Routines for comparing differen data types in different modes.
 * Used by CData_SortInt.
 *
 */

int cf_uchar_up(const void* a, const void* b)
{
  if(*(unsigned char*)a > *(unsigned char*)b) return  1;
  if(*(unsigned char*)a < *(unsigned char*)b) return -1;

  return 0;
}

int cf_uchar_down(const void* a, const void* b)
{
  if(*(unsigned char*)a > *(unsigned char*)b) return -1;
  if(*(unsigned char*)a < *(unsigned char*)b) return  1;

  return 0;
}

int cf_char_up(const void* a, const void* b)
{
  if(*(char*)a > *(char*)b) return  1;
  if(*(char*)a < *(char*)b) return -1;

  return 0;
}

int cf_char_down(const void* a, const void* b)
{
  if(*(char*)a > *(char*)b) return -1;
  if(*(char*)a < *(char*)b) return  1;

  return 0;
}

int cf_ushort_up(const void* a, const void* b)
{
  if(*(UINT16*)a > *(UINT16*)b) return  1;
  if(*(UINT16*)a < *(UINT16*)b) return -1;

  return 0;
}

int cf_ushort_down(const void* a, const void* b)
{
  if(*(UINT16*)a > *(UINT16*)b) return -1;
  if(*(UINT16*)a < *(UINT16*)b) return  1;

  return 0;
}

int cf_short_up(const void* a, const void* b)
{
  if(*(INT16*)a > *(INT16*)b) return  1;
  if(*(INT16*)a < *(INT16*)b) return -1;

  return 0;
}

int cf_short_down(const void* a, const void* b)
{
  if(*(INT16*)a > *(INT16*)b) return -1;
  if(*(INT16*)a < *(INT16*)b) return  1;

  return 0;
}

int cf_uint_up(const void* a, const void* b)
{
  if(*(UINT32*)a > *(UINT32*)b) return  1;
  if(*(UINT32*)a < *(UINT32*)b) return -1;

  return 0;
}

int cf_uint_down(const void* a, const void* b)
{
  if(*(UINT32*)a > *(UINT32*)b) return -1;
  if(*(UINT32*)a < *(UINT32*)b) return  1;

  return 0;
}

int cf_int_up(const void* a, const void* b)
{
  if(*(INT32*)a > *(INT32*)b) return  1;
  if(*(INT32*)a < *(INT32*)b) return -1;

  return 0;
}

int cf_int_down(const void* a, const void* b)
{
  if(*(INT32*)a > *(INT32*)b) return -1;
  if(*(INT32*)a < *(INT32*)b) return  1;

  return 0;
}

int cf_ulong_up(const void* a, const void* b)
{
  if(*(UINT64*)a > *(UINT64*)b) return  1;
  if(*(UINT64*)a < *(UINT64*)b) return -1;

  return 0;
}

int cf_ulong_down(const void* a, const void* b)
{
  if(*(UINT64*)a > *(UINT64*)b) return -1;
  if(*(UINT64*)a < *(UINT64*)b) return  1;

  return 0;
}

int cf_long_up(const void* a, const void* b)
{
  if(*(INT64*)a > *(INT64*)b) return  1;
  if(*(INT64*)a < *(INT64*)b) return -1;

  return 0;
}

int cf_long_down(const void* a, const void* b)
{
  if(*(INT64*)a > *(INT64*)b) return -1;
  if(*(INT64*)a < *(INT64*)b) return  1;

  return 0;
}

int cf_float_up(const void* a, const void* b)
{
  if(*(FLOAT32*)a > *(FLOAT32*)b) return  1;
  if(*(FLOAT32*)a < *(FLOAT32*)b) return -1;

  return 0;
}

int cf_float_down(const void* a, const void* b)
{
  if(*(FLOAT32*)a > *(FLOAT32*)b) return -1;
  if(*(FLOAT32*)a < *(FLOAT32*)b) return  1;

  return 0;
}

int cf_double_up(const void* a, const void* b)
{
  if(*(FLOAT64*)a > *(FLOAT64*)b) return  1;
  if(*(FLOAT64*)a < *(FLOAT64*)b) return -1;

  return 0;
}

int cf_double_down(const void* a, const void* b)
{
  if(*(FLOAT64*)a > *(FLOAT64*)b) return -1;
  if(*(FLOAT64*)a < *(FLOAT64*)b) return  1;

  return 0;
}

int cf_complex_up(const void* a, const void* b)
{
  if(CMPLX_GREATER(*(COMPLEX64*)a,*(COMPLEX64*)b)) return  1;
  if(CMPLX_LESS   (*(COMPLEX64*)a,*(COMPLEX64*)b)) return -1;

  return 0;
}

int cf_complex_down(const void* a, const void* b)
{
  if(CMPLX_GREATER(*(COMPLEX64*)a,*(COMPLEX64*)b)) return -1;
  if(CMPLX_LESS   (*(COMPLEX64*)a,*(COMPLEX64*)b)) return  1;

  return 0;
}

int cf_string_up(const void* a, const void* b)
{
  if(dlp_strcmp((const char*)a,(const char*)b) > 0) return  1;
  if(dlp_strcmp((const char*)a,(const char*)b) < 0) return -1;

  return 0;
}

int cf_string_down(const void* a, const void* b)
{
  if(dlp_strcmp((const char*)a,(const char*)b) > 0) return -1;
  if(dlp_strcmp((const char*)a,(const char*)b) < 0) return  1;

  return 0;
}

int cf_strnum_up(const void* a, const void* b)
{
  if(atof((const char*)a) > atof((const char*)b)) return  1;
  if(atof((const char*)a) < atof((const char*)b)) return -1;

  /* if numeric equal sort compare as strings */
  return cf_string_up(a, b);
}

int cf_strnum_down(const void* a, const void* b)
{
  if(atof((const char*)a) > atof((const char*)b)) return -1;
  if(atof((const char*)a) < atof((const char*)b)) return  1;

  /* if numeric equal sort compare as strings */
  return cf_string_down(a, b);
}

/**
 *  Sort data using qsort.
 *
 *  @param _this Destination for sorted data
 *  @param iSrc  Input data
 *  @param iIdx  Index array, contains sort index after calling sort (may be
 *               NULL)
 *  @param nComp Component to sort
 *  @param nMode Operation mode for sort (ascending, descending)
 *  @return O_K if successfull, NOT_EXEC otherwise
 */
INT16 CGEN_PUBLIC CData_SortInt(CData* _this, CData* iSrc, CData* iIdx, INT32 nComp, INT16 nMode)
{
  BOOL bNumeric = _this->m_bNumeric;
  int (*compfunc)(const void* a, const void* b) = NULL;

  CHECK_THIS_RV(NOT_EXEC);

  /* check consistency of arguments */
  if (CData_IsEmpty(iSrc))            return IERROR(_this,DATA_EMPTY,BASEINST(iSrc)->m_lpInstanceName,0,0);
  if (CData_GetNComps(iSrc) <= nComp) return IERROR(_this,DATA_BADCOMP,nComp,BASEINST(iSrc)->m_lpInstanceName,0);
  if (nMode != CDATA_SORT_UP && nMode != CDATA_SORT_DOWN) return IERROR(_this,DATA_BADSORTMODE,nMode,0,0);
  if (!dlp_is_numeric_type_code(CData_GetCompType(iSrc,nComp)) &&
    !dlp_is_symbolic_type_code(CData_GetCompType(iSrc,nComp))  )
    return IERROR(_this,DATA_BADSORTTYPE,nComp,dlp_get_type_name(CData_GetCompType(iSrc,nComp)),0);

  /* Input and output instances identical? */
  CREATEVIRTUAL(CData,iSrc,_this);

  /* copy input and insert index component at the beginning */
  CData_Select(_this,iSrc,nComp,1);
  if(iIdx)
  {
    CData_Reset(BASEINST(iIdx),TRUE);
    CData_AddComp(iIdx,"idx",T_LONG);
    CData_AllocateUninitialized(iIdx,CData_GetNRecs(iSrc));
    CData_Fill(iIdx,CMPLX(0),CMPLX(1));
    CData_Join(_this,iIdx);
  }
  CData_Join(_this,iSrc);

  /* select comparition routine depending on type of data and sort mode */
  switch(CData_GetCompType(_this,0))
  {
  case T_UCHAR  : (nMode == CDATA_SORT_UP) ? (compfunc = cf_uchar_up  ) : (compfunc = cf_uchar_down  ); break;
  case T_CHAR   : (nMode == CDATA_SORT_UP) ? (compfunc = cf_char_up   ) : (compfunc = cf_char_down   ); break;
  case T_USHORT : (nMode == CDATA_SORT_UP) ? (compfunc = cf_ushort_up ) : (compfunc = cf_ushort_down ); break;
  case T_SHORT  : (nMode == CDATA_SORT_UP) ? (compfunc = cf_short_up  ) : (compfunc = cf_short_down  ); break;
  case T_UINT   : (nMode == CDATA_SORT_UP) ? (compfunc = cf_uint_up   ) : (compfunc = cf_uint_down   ); break;
  case T_INT    : (nMode == CDATA_SORT_UP) ? (compfunc = cf_int_up    ) : (compfunc = cf_int_down    ); break;
  case T_ULONG  : (nMode == CDATA_SORT_UP) ? (compfunc = cf_ulong_up  ) : (compfunc = cf_ulong_down  ); break;
  case T_LONG   : (nMode == CDATA_SORT_UP) ? (compfunc = cf_long_up   ) : (compfunc = cf_long_down   ); break;
  case T_FLOAT  : (nMode == CDATA_SORT_UP) ? (compfunc = cf_float_up  ) : (compfunc = cf_float_down  ); break;
  case T_DOUBLE : (nMode == CDATA_SORT_UP) ? (compfunc = cf_double_up ) : (compfunc = cf_double_down ); break;
  case T_COMPLEX: (nMode == CDATA_SORT_UP) ? (compfunc = cf_complex_up) : (compfunc = cf_complex_down); break;
  default       :
    if(bNumeric) {
      (nMode == CDATA_SORT_UP) ? (compfunc = cf_strnum_up) : (compfunc = cf_strnum_down);
    } else {
      (nMode == CDATA_SORT_UP) ? (compfunc = cf_string_up) : (compfunc = cf_string_down);
    }
    break;
  }

  /* sort data using qsort (stdlib) */
  dlpsort(_this->m_lpTable->m_theDataPointer,CData_GetNRecs(_this),CData_GetRecLen(_this),compfunc);

  /* remove index component */
  if(iIdx)
  {
    CData_Select(iIdx,_this,1,1);
    CData_Delete(_this,_this,1,1);
  }
  CData_Delete(_this,_this,0,1);

  /* Cleanup */
  DESTROYVIRTUAL(iSrc,_this);

  return O_K;
}

INT32 CGEN_PRIVATE CData_ChecksumInt(CData* _this, char *sAlgo, INT32 nIc)
{
#ifndef __NODLPMATH
  CHECK_THIS_RV(NOT_EXEC);

  if(!dlp_strcmp(sAlgo,"CRC-32")){
    void* lpH = dlm_crc32_init();
    INT32 nR,nNR = CData_GetNRecs(_this);
    INT32 nC,nNC = CData_GetNComps(_this);
    UINT32 nCrc32 = 0;
    for(nR=0;nR<nNR;nR++) for(nC=0;nC<nNC;nC++) if(nC!=nIc)
      dlm_crc32_add(lpH,CData_XAddr(_this,nR,nC),CData_GetCompSize(_this,nC));
    dlm_crc32_finalize(lpH,(BYTE*)&nCrc32,sizeof(nCrc32));
    return nCrc32;
  }else IERROR(_this,DATA_OPCODE,sAlgo,"checksum",0);
  return 0;
#else
  return IERROR(_this,ERR_NOTSUPPORTED,"Method -checksum in __NODLPMATH mode",0,0);
#endif /* #ifndef __NODLPMATH */
}

/* EOF */
