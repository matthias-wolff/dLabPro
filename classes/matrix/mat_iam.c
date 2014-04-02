/* dLabPro class CMatrix (matrix)
 * - Class CMatrix interactive methods (user defined PMI/SMI functions)
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

#define __FFT_SET_CUNIT(NEW) {   \
  dlp_strcpy(RE->m_lpCunit,NEW); \
  dlp_strcpy(IM->m_lpCunit,NEW); }

/*
 * Implementation of primary method callback function for -op
 */
INT16 CGEN_PUBLIC CMatrix_OnOp_Impl(CMatrix* _this)
{
  StkItm si1;
  StkItm si2;
  CData* idPar1  = NULL;
  CData* idPar2  = NULL;
  CData* idRes   = NULL;
  char*  sOpname = NULL;
  INT32   nOpcode = -1;
  INT16  nErr    = O_K;

  if (!CDlpObject_MicGet(BASEINST(_this))->GetX)
  {
    CERROR(data,DATA_NOSUPPORT,"Polymorphic methods"," by caller",0);
    return DATA_NOSUPPORT;
  }

  sOpname = MIC_GET_S(1,0);
  idRes = MIC_GET_I_EX(idRes,data,2,1);
  if (!idRes) return NOT_EXEC;
  if (!MIC_GET_X(3,&si2)) return NOT_EXEC;
  if (!MIC_GET_X(4,&si1)) return NOT_EXEC;

  if (si1.nType==T_INSTANCE)
  {
    idPar1=(CData*)CDlpObject_OfKind("data",si1.val.i);
    if (si1.val.i && !idPar1) return IERROR(_this,ERR_INVALARG,"param1",0,0);
  }
  if (si2.nType==T_INSTANCE)
  {
    idPar2=(CData*)CDlpObject_OfKind("data",si2.val.i);
    if (si2.val.i && !idPar2) return IERROR(_this,ERR_INVALARG,"param2",0,0);
  }

  nOpcode = dlm_matrop_code(sOpname,NULL);
  if (nOpcode<0) nOpcode = dlp_scalop_code(sOpname);
  if (nOpcode<0) return IERROR(idRes,DATA_OPCODE,SCSTR(sOpname),"matrop",0);

  IFCHECK printf("\n -matrop %ld: ",(long)nOpcode);
  if (si1.nType==T_INSTANCE && si2.nType==T_INSTANCE)
  {
    IFCHECK  printf("%s,%s",idPar1?"data":"NULL",idPar2?"data":"NULL");
    nErr = CMatrix_Op(idRes,idPar1,T_INSTANCE,idPar2,T_INSTANCE,nOpcode);
  }
  else if (si1.nType==T_INSTANCE && si2.nType==T_COMPLEX)
  {
    if(si2.val.n.y==0)
    {
      IFCHECK printf("\n%s,number",idPar1?"data":"NULL");
      nErr = CMatrix_Op(idRes,idPar1,T_INSTANCE,&si2.val.n,T_DOUBLE,nOpcode);
    }
    else
    {
      IFCHECK printf("\n%s,complex number",idPar1?"data":"NULL");
      nErr = CMatrix_Op(idRes,idPar1,T_INSTANCE,&si2.val.n,T_COMPLEX,nOpcode);
    }
  }
  else if (si1.nType==T_COMPLEX && si2.nType==T_INSTANCE)
  {
    if(si1.val.n.y==0)
    {
      IFCHECK printf("\nnumber,%s",idPar2?"data":"NULL");
      nErr = CMatrix_Op(idRes,&si1.val.n,T_DOUBLE,idPar2,T_INSTANCE,nOpcode);
    }
    else
    {
      IFCHECK printf("\ncomplex number,%s",idPar2?"data":"NULL");
      nErr = CMatrix_Op(idRes,&si1.val.n,T_COMPLEX,idPar2,T_INSTANCE,nOpcode);
    }
  }
  else
    return IERROR(idRes,DATA_NOSUPPORT,"Parameter combination","",0);

  return nErr;
}

/* == Static methods == */                                                      /* ================================= */
#undef  dlp_calloc
#define dlp_calloc(A,B) __dlp_calloc(A,B,__FILE__,__LINE__,"CMatrix_...",NULL)

/*
 * Manual page at matrix.def
 */
INT16 CGEN_SPUBLIC CMatrix_Op
(
  CData* idDst,
  void*  lpPar1,
  INT16  nType1,
  void*  lpPar2,
  INT16  nType2,
  INT16  nOpcode
)
 {
  CData*      idPar1    = NULL;                                                 /* Parameter 1 as data instance      */
  CData*      idPar2    = NULL;                                                 /* Parameter 2 as data instance      */
  CData*      idRes     = NULL;                                                 /* Pointer to result data instance   */
  INT32       nB       = 0;                                                     /* Current block                     */
  INT32       nXB      = 0;                                                     /* Number of blocks in result        */
  INT32       nXB1     = 0;                                                     /* Number of blocks in idPar1        */
  INT32       nXB2     = 0;                                                     /* Number of blocks in idPar1        */
  INT32       nC       = 0;                                                     /* Component counter                 */
  INT32       nXC      = 0;                                                     /* Number of numeric COMPS. in dest. */
  INT32       nXC1     = 0;                                                     /* Number of numeric COMPS. in idPar1*/
  INT32       nXC2     = 0;                                                     /* Number of numeric COMPS. in idPar2*/
  INT32       nR       = 0;                                                     /* Record counter                    */
  INT32       nXR      = 0;                                                     /* Number of RECORDS/block of dest.  */
  INT32       nXR1     = 0;                                                     /* Number of RECORDS/block of idPar1 */
  INT32       nXR2     = 0;                                                     /* Number of RECORDS/block of idPar2 */
  FLOAT64     nDummyF  = 0.;                                                    /* Dummy scalar value                */
  COMPLEX64   nDummyC  = CMPLX(0.);                                             /* Dummy complex scalar value        */
  void*       A        = NULL;                                                  /* Matrix A (left operand)           */
  void*       B        = NULL;                                                  /* Matrix B (right operand)          */
  void*       Z        = NULL;                                                  /* Matrix Z (result)                 */
  INT32       nXRa     = 0;                                                     /* Number of ROWS in matrix A        */
  INT32       nXRb     = 0;                                                     /* Number of ROWS in matrix B        */
  INT32       nXRz     = 0;                                                     /* Number of ROWS in matrix Z        */
  INT32       nXCa     = 0;                                                     /* Number of COLUMNS in matrix A     */
  INT32       nXCb     = 0;                                                     /* Number of COLUMNS in matrix B     */
  INT32       nXCz     = 0;                                                     /* Number of COLUMNS in matrix Z     */
  INT16       nCheck   = 0;                                                     /* Verbose level copy buffer         */
  INT32       i        = 0;                                                     /* Universal loop counter            */
  INT32       j        = 0;                                                     /* Second universal loop counter     */
  INT16       nErr     = O_K;                                                   /* Matrix operation error status     */
  BOOL        bCmplxA  = FALSE;                                                 /* Indicate complex input 1          */
  BOOL        bCmplxB  = FALSE;                                                 /* Indicate complex input 2          */
  BOOL        bCmplxZ  = FALSE;                                                 /* Indicate complex output           */
  BOOL        bSymblc  = FALSE;                                                 /* Indicate pure symbolic matrix op. */
  const char* lpsSignature = dlm_matrop_signature(nOpcode);                     /* Signature of opcode               */
  char        sMsg[L_SSTR];                                                     /* (Error) message buffer            */

  /* Validate */                                                                /* --------------------------------- */
  if (!idDst) return NOT_EXEC;                                                  /* No dest. instance, no service     */
  nCheck = BASEINST(idDst)->m_nCheck;                                           /* Save verbose level                */
  if (nCheck>0)                                                                 /* On verbose level 1                */
  {                                                                             /* >>                                */
    printf("\n"); dlp_fprint_x_line(stdout,'-',dlp_maxprintcols());             /*   Print delimiter                 */
    printf("\n   Method\n   matrix -op");                                       /*   Identify method                 */
    printf("\n   - Opcode: %ld",(long)nOpcode);                                 /*   Report operation type           */
    printf("\n   - Result: data %s",BASEINST(idDst)->m_lpInstanceName);         /*   Report on destination instrance */
  }                                                                             /* <<                                */

  /* Initialize - Resolve parameter pointers */                                 /* --------------------------------- */
  if (lpPar1==NULL)                                                             /* No param. 1                       */
    if(nType1 == T_COMPLEX) { bCmplxA = TRUE;  A = &nDummyC; }                  /* -> A is dummy complex scalar      */
    else                    { bCmplxA = FALSE; A = &nDummyF; }                  /* -> A is dummy scalar              */
  else switch (nType1)                                                          /* Have param. 1 -> branch for type  */
  {                                                                             /* >>                                */
    case T_INSTANCE: idPar1 = (CData* )lpPar1; break;                           /*   (CData) instance                */
    case T_DOUBLE  : bCmplxA = FALSE; A = lpPar1; break;                        /*   Single double value             */
    case T_COMPLEX : bCmplxA = TRUE;  A = lpPar1; break;                        /*   Single complex double value     */
    default        : return NOT_EXEC;                                           /*   Nothing else acceptable         */
  }                                                                             /* <<                                */
  if (lpPar2==NULL)                                                             /* No param. 2                       */
    if(nType2 == T_COMPLEX) { bCmplxB = TRUE;  B = &nDummyC; }                  /* -> B is dummy complex scalar      */
    else                    { bCmplxB = FALSE; B = &nDummyF; }                  /* -> B is dummy scalar              */
  else switch (nType2)                                                          /* Have param. 2 -> branch for type  */
  {                                                                             /* >>                                */
    case T_INSTANCE: idPar2 = (CData*  )lpPar2; break;                          /*   (CData) instance                */
    case T_DOUBLE  : bCmplxB = FALSE; B = lpPar2; break;                        /*   Single double value             */
    case T_COMPLEX : bCmplxB = TRUE;  B = lpPar2; break;                        /*   Single complex double value     */
    case T_IGNORE  : bCmplxB = FALSE; B = &nDummyF; break;                                    /*   Nothing                         */
    default        : return NOT_EXEC;                                           /*   Nothing else acceptable         */
  }                                                                             /* <<                                */

  /* Initialize - Handle overlapping arguments */                               /* --------------------------------- */
  if      (idPar1==idDst) { ICREATE(CData,idRes,NULL); }                        /* Result is identical with param. 1 */
  else if (idPar2==idDst) { ICREATE(CData,idRes,NULL); }                        /* Result is identical with param. 2 */
  else                      idRes = idDst;                                      /* Result is none of the parameters  */

  if((idPar1!=NULL) && (CData_GetNComplexComps(idPar1)>0)) bCmplxA = TRUE;      /* Determine complex data of Param. 1*/
  if((idPar2!=NULL) && (CData_GetNComplexComps(idPar2)>0)) bCmplxB = TRUE;      /* Determine complex data of Param. 2*/
  bCmplxZ = bCmplxA || bCmplxB || ((lpsSignature!=NULL)&&(*lpsSignature=='C')); /* Determine complex output          */
  if(!idPar1 && !A && bCmplxZ) A=&nDummyC;                                      /* Ensure complex dummy A            */
  if(!idPar2 && !B && bCmplxZ) B=&nDummyC;                                      /* Ensure complex dummy B            */

  /* Initialize - Determine all kinds of dimensions */                          /* --------------------------------- */
  nXB1 = CData_GetNBlocks(idPar1);                                              /* How many matrices A? (0 = scalar) */
  nXB2 = CData_GetNBlocks(idPar2);                                              /* How many matrices B? (0 = scalar) */
  nXB  = MAX(nXB1,nXB2); if (nXB<=0) nXB=1;                                     /* How many matrices Z?              */
  nXC1 = CData_GetNNumericComps(idPar1);                                        /* No. of numeric comps. in idPar1   */
  nXC2 = CData_GetNNumericComps(idPar2);                                        /* No. of numeric comps. in idPar2   */
  nXR1 = nXB1 ? CData_GetNRecsPerBlock(idPar1) : 1;                             /* No. of records/block of idPar1    */
  nXR2 = nXB2 ? CData_GetNRecsPerBlock(idPar2) : 1;                             /* No. of records/block of idPar2    */
  nXRa = nXB1 ? nXC1 : 1;                                                       /* Number of rows in matrix A        */
  nXCa = nXB1 ? nXR1 : 1;                                                       /* Number of columns in matrix A     */
  nXRb = nXB2 ? nXC2 : 1;                                                       /* Number of rows in matrix B        */
  nXCb = nXB2 ? nXR2 : 1;                                                       /* Number of columns in matrix B     */
  if(bCmplxZ) dlm_matropC(                                                      /* Determine target matrix size      */
      NULL,&nXRz,&nXCz,(COMPLEX64*)A,nXRa,nXCa,(COMPLEX64*)B,nXRb,nXCb,nOpcode);/*   |                               */
  else        dlm_matrop (                                                      /*   |                               */
      NULL,&nXRz,&nXCz,(  FLOAT64*)A,nXRa,nXCa,(  FLOAT64*)B,nXRb,nXCb,nOpcode);/*   |                               */
  nXC  = nXRz;                                                                  /* Number of components in result    */
  nXR  = nXCz;                                                                  /* Number of records/block in result */

  /* Determine pure symbolic matrix operation */                                /* --------------------------------- */
  bSymblc = (nXC1==0) && (CData_GetNComps(idPar1)>0);                           /* Matrix A is purely symbolic       */
  if (idPar2!=NULL)                                                             /* Matrix B exists ...               */
    bSymblc &= (nXC1==0) && (CData_GetNComps(idPar2)>0);                        /* ... and is purely symbolic        */

  /* Check protocol */                                                          /* --------------------------------- */
  if (nCheck>0)                                                                 /* On verbose level 1                */
  {                                                                             /* >>                                */
    printf("\n   - Param1: ");                                                  /*   Report on param 1 ...           */
    if (nXB1)                                                                   /*   It's a data instance            */
      printf("data %s - %ld x %ld x %ld (blks x rows x cols)",                  /*     Protocol                      */
        BASEINST(idPar1)->m_lpInstanceName,(long)nXB1,(long)nXRa,(long)nXCa);   /*     |                             */
    else                                                                        /*   It's a number                   */
      if(bCmplxA) printf("%g+%gi",((COMPLEX64*)A)->x, ((COMPLEX64*)A)->y);      /*     Protocol                      */
      else        printf("%g",*(FLOAT64*)A);                                    /*     Protocol                      */
    printf("\n   - Param2: ");                                                  /*   Report on param 2 ...           */
    if (nXB2)                                                                   /*   It's a data instance            */
      printf("data %s - %ld x %ld x %ld (blks x rows x cols)",                  /*     Protocol                      */
        BASEINST(idPar2)->m_lpInstanceName,(long)nXB2,(long)nXRb,(long)nXCb);   /*     |                             */
    else                                                                        /*   It's a number                   */
      if(bCmplxB) printf("%g+%gi",((COMPLEX64*)B)->x, ((COMPLEX64*)B)->y);      /*     Protocol                      */
      else        printf("%g",*(FLOAT64*)B);                                    /*     Protocol                      */
    printf("\n   - Result: data %s - %ld x %ld x %ld (blks x rows x cols)",     /*   Report on expected result       */
        BASEINST(idDst)->m_lpInstanceName,(long)nXB,(long)nXRz,(long)nXCz);     /*   |                               */
    printf("\n"); dlp_fprint_x_line(stdout,'-',dlp_maxprintcols());             /*   Delimiter                       */
  }                                                                             /* <<                                */

  /* Initialize - Allocate buffers */                                           /* --------------------------------- */
  if(bCmplxZ) {                                                                 /* On any complex input              */
    if (nXB1) A = (COMPLEX64*)dlp_calloc(nXCa*nXRa,sizeof(COMPLEX64));          /*   Alloc. copy buffer for matrix A */
    if (nXB2) B = (COMPLEX64*)dlp_calloc(nXCb*nXRb,sizeof(COMPLEX64));          /*   Alloc. copy buffer for matrix B */
    Z = (COMPLEX64*)dlp_calloc(nXCz*nXRz,sizeof(COMPLEX64));                    /*   Alloc. copy buffer for matrix Z */
  } else {                                                                      /* Inputs are non complex            */
    if (nXB1) A = (  FLOAT64*)dlp_calloc(nXCa*nXRa,sizeof(  FLOAT64));          /*   Alloc. copy buffer for matrix A */
    if (nXB2) B = (  FLOAT64*)dlp_calloc(nXCb*nXRb,sizeof(  FLOAT64));          /*   Alloc. copy buffer for matrix B */
    Z = (  FLOAT64*)dlp_calloc(nXCz*nXRz,sizeof(  FLOAT64));                    /*   Alloc. copy buffer for matrix Z */
  }                                                                             /* <<                                */

  /* Initialize - Prepare result */                                             /* --------------------------------- */
  if (!bCmplxZ && nOpcode==OP_MULT && idPar1 && idPar2)                         /* Non-trivial matrix product        */
  {                                                                             /* >>                                */
    j=0;                                                                        /*   reset loop counter              */
    for (i=0; i<CData_GetNComps(idPar1); i++)                                   /*   Copy numeric components ...     */
      if (dlp_is_numeric_type_code(CData_GetCompType(idPar1,i)))                /*   ... from idPar1                 */
      {                                                                         /*                                   */
        for(; j<CData_GetNComps(idPar2); j++)                                   /*   find next idPar2 numeric comp.  */
          if(dlp_is_numeric_type_code(CData_GetCompType(idPar2,j)))             /*                                   */
            break;                                                              /*                                   */
        if(CData_GetCompType(idPar2,j) > CData_GetCompType(idPar1,i))           /*   precision of data type idPar2   */
          CData_AddComp(idRes,CData_GetCname(idPar1,i),                         /*   great than of idPar1?           */
            CData_GetCompType(idPar2,j));                                       /*                                   */
        else                                                                    /*                                   */
          CData_AddComp(idRes,CData_GetCname(idPar1,i),                         /*                                   */
            CData_GetCompType(idPar1,i));                                       /*                                   */
      }                                                                         /*                                   */
    CData_Allocate(idRes,nXR*nXB);                                              /*   Allo. result records (columns)  */
    CMatrix_CopyLabels(idRes,idPar2);                                           /*   Copy labels from idPar2         */
    CData_CopyDescr(idRes,idPar2);                                              /*   Copy descriptions for idPar2    */
    idRes->m_nCinc = idPar1->m_lpTable->m_fsr;                                  /*   ... except component ...        */
    idRes->m_nCofs = idPar1->m_lpTable->m_zf;                                   /*   ... descriptions to be ...      */
    dlp_strcpy(idRes->m_lpCunit,idPar1->m_lpRunit);                             /*   ... copied from idPar1          */
  }                                                                             /* <<                                */
  else if (nOpcode==OP_TRANSPOSE && idPar1)                                     /* Matrix transpose                  */
  {                                                                             /* >>                                */
    if (!bSymblc)                                                               /*   Numeric                         */
      CData_Array(idRes,bCmplxZ?T_COMPLEX:T_DOUBLE,nXC,nXR*nXB);                /*     Make all new data instance    */
    else                                                                        /*   Symbolic                        */
    {                                                                           /*   >>                              */
      INT16 nType = 0;                                                          /*     Default string type           */
      for (nC=0; nC<CData_GetNComps(idPar1); nC++)                              /*     Loop over components          */
      {                                                                         /*     >>                            */
        INT16 nCtype = CData_GetCompType(idPar1,nC);                            /*       Get component type          */
        if (nCtype<=L_SSTR && nCtype>nType) nType = nCtype;                     /*       Get greates string length   */
      }                                                                         /*     <<                            */
      nXC = CData_GetNRecsPerBlock(idPar1);                                     /*     Get no. of output components  */
      nXR = CData_GetNComps(idPar1);                                            /*     Get no. of output records     */
      nXB = CData_GetNBlocks(idPar1);                                           /*     Get no. of output blocks      */
      CData_Array(idRes,nType,nXC,nXR*nXB);                                     /*     Make all new data instance    */
    }                                                                           /*   <<                              */
    CData_CopyDescr(idRes,idPar1);                                              /*   Copy descriptions at least      */
    idRes->m_lpTable->m_fsr = idPar1->m_nCinc;                                  /*   Transpose rec. axis increment   */
    idRes->m_lpTable->m_ofs = idPar1->m_nCofs;                                  /*   Transpose rec. axis offset      */
    idRes->m_nCinc          = idPar1->m_lpTable->m_fsr;                         /*   Transpose comp. axis increment  */
    idRes->m_nCofs          = idPar1->m_lpTable->m_ofs;                         /*   Transpose comp. axis offset     */
    dlp_strcpy(idRes->m_lpRunit,idPar1->m_lpCunit);                             /*   Transpose rec. axis phys. unit  */
    dlp_strcpy(idRes->m_lpCunit,idPar1->m_lpRunit);                             /*   Transpose comp. axis phys. unit */
    if (!bSymblc)                                                               /*   Numeric                         */
      for (nC=0; nC<CData_GetNComps(idPar1); nC++)                              /*     Loop over source components   */
        if (dlp_is_symbolic_type_code(CData_GetCompType(idPar1,nC)))            /*       Seek first symbolic comp.   */
        {                                                                       /*       >>                          */
          for (nR=0; nR<CData_GetNComps(idRes); nR++)                           /*         Loop over source records  */
            CData_SetCname(idRes,nR,CData_Sfetch(idPar1,nR,nC));                /*           Set target comp.'s name */
          break;                                                                /*         Do it only once!          */
        }                                                                       /*       <<                          */
  }                                                                             /* <<                                */
  else if(nOpcode==OP_CCF && idPar1 && idPar2)                                  /* Cross Correlation                 */
  {                                                                             /* >>                                */
    CData_Copy(BASEINST(idRes),BASEINST(idPar2));                               /*   Copy structure of B (idPar2)    */
    CData_Tconvert(idRes,idRes,bCmplxZ?T_COMPLEX:T_DOUBLE);                     /*   Convert all numeric data type   */
  }                                                                             /* <<                                */
  else if (idPar1 && nXRz==nXRa && nXCz==nXCa)                                  /* Result structure matches A        */
  {                                                                             /* >>                                */
    CData_Copy(BASEINST(idRes),BASEINST(idPar1));                               /*   Copy (numbers to be overwritten)*/
    if(bCmplxZ) CData_Tconvert(idRes,idRes,T_COMPLEX);                          /*   Convert all numeric data type   */
  }                                                                             /* <<                                */
  else if (idPar2 && nXRz==nXRb && nXCz==nXCb)                                  /* Result structure matches B        */
  {                                                                             /* >>                                */
    CData_Copy(BASEINST(idRes),BASEINST(idPar2));                               /*   Copy (numbers to be overwritten)*/
    if(bCmplxZ) CData_Tconvert(idRes,idRes,T_COMPLEX);                          /*   Convert all numeric data type   */
  }                                                                             /* <<                                */
  else                                                                          /* Result has other structure        */
  {                                                                             /* >>                                */
    CData_Array(idRes,bCmplxZ?T_COMPLEX:T_DOUBLE,nXC,nXR*nXB);                  /*   Make all new data instance      */
    if (idPar1 || idPar2) CData_CopyDescr(idRes,idPar1?idPar1:idPar2);          /*   Copy descriptions at least      */
    if      (nXCz==nXCa && idPar1) CMatrix_CopyLabels(idRes,idPar1);            /*   Copy labels form idPar1         */
    else if (nXCz==nXCb && idPar2) CMatrix_CopyLabels(idRes,idPar2);            /*   Copy labels form idPar2         */
  }                                                                             /* <<                                */
  CData_SetNBlocks(idRes,nXB);                                                  /* Set block number                  */
  BASEINST(idDst)->m_nCheck = nCheck;                                           /* Restore verbose level             */

  /* Symbolic operations */                                                     /* --------------------------------- */
  if (bSymblc)                                                                  /* Symbolic (only!) operation        */
  {                                                                             /* >>                                */
    if (nOpcode==OP_TRANSPOSE)                                                  /*   Symbolic transpose              */
    {                                                                           /*   >>                              */
      INT32 nR01 = 0;                                                           /*     Block offset in source        */
      INT32 nR0D = 0;                                                           /*     Block offset in destination   */
      CData_Allocate(idRes,nXR*nXB);                                            /*     Allocate destination memory   */
      for (nB=0; nB<nXB; nB++)                                                  /*     Loop over blocks              */
      {                                                                         /*     >>                            */
        for (nR=0; nR<nXR; nR++)                                                /*       Loop over dest. records     */
          for (nC=0; nC<nXC; nC++)                                              /*         Loop over dest. components*/
            CData_Sstore(idRes,CData_Sfetch(idPar1,nC+nR01,nR),nR+nR0D,nC);     /*           Copy data               */
        nR01 += CData_GetNRecsPerBlock(idPar1);                                 /*       Adjust source block offset  */
        nR0D += CData_GetNRecsPerBlock(idRes);                                  /*       Adjust dest. block offset   */
      }                                                                         /*     <<                            */
    }                                                                           /*   <<                              */
    else                                                                        /*   No symbolic operation done      */
      bSymblc = FALSE;                                                          /*     Fall back to numeric mode     */
  }                                                                             /* <<                                */

  /* Numeric operations */                                                      /* --------------------------------- */
  if (!bSymblc)                                                                 /* Numeric computation               */
  {                                                                             /* >>                                */
    if (nXB1==1)                                                                /*   Fetch single block only once (A)*/
    {                                                                           /*   ...                             */
      if (bCmplxZ) CData_CblockFetch(idPar1,(COMPLEX64*)A,0,nXC1,nXR1,-1);      /*   ...                             */
      else         CData_DblockFetch(idPar1,(  FLOAT64*)A,0,nXC1,nXR1,-1);      /*   ...                             */
    }                                                                           /*   ...                             */
    if (nXB2==1)                                                                /*   Fetch single block only once (B)*/
    {                                                                           /*   ...                             */
      if (bCmplxZ) CData_CblockFetch(idPar2,(COMPLEX64*)B,0,nXC2,nXR2,-1);      /*   ...                             */
      else         CData_DblockFetch(idPar2,(  FLOAT64*)B,0,nXC2,nXR2,-1);      /*   ...                             */
    }                                                                           /*   ...                             */
    for (nB=0; nB<nXB; nB++)                                                    /*   Loop over blocks                */
    {                                                                           /*   >>                              */
      if (nCheck>0)                                                             /*     On verbose level 1            */
      {                                                                         /*     >>                            */
        if (nXB1)                                                               /*       Report on left operand      */
          printf("\n   - A%ld",(long)(nXB1==1?0:nB));                           /*       ...                         */
        else if (bCmplxA)                                                       /*       ...                         */
          printf("\n   - %g+%gi",(double)((COMPLEX64*)A)->x,                    /*       ...                         */
            (double)((COMPLEX64*)A)->y);                                        /*       ...                         */
        else                                                                    /*       ...                         */
          printf(  " vs. %g",(double)*(FLOAT64*)B);                             /*       ...                         */
        if (nXB2)                                                               /*       Report on right operand     */
          printf(  " vs. B%ld",(long)(nXB2==1?0:nB));                           /*       ...                         */
        else if (bCmplxB)                                                       /*       ...                         */
          printf(  " vs. %g+%gi",(double)((COMPLEX64*)B)->x,                    /*       ...                         */
            (double)((COMPLEX64*)B)->y);                                        /*       ...                         */
        else                                                                    /*       ...                         */
          printf(  " vs. %g",(double)*(FLOAT64*)B);                             /*       ...                         */
      }                                                                         /*     <<                            */

      /* Fetch matrices and do computation */                                   /*     - - - - - - - - - - - - - - - */
      if (nXB1>1)                                                               /*     Fetch A if lpPar1 is a matrix */
      {                                                                         /*     ...                           */
        if (bCmplxZ) CData_CblockFetch(idPar1,(COMPLEX64*)A,nB,nXC1,nXR1,-1);   /*     ...                           */
        else         CData_DblockFetch(idPar1,(  FLOAT64*)A,nB,nXC1,nXR1,-1);   /*     ...                           */
      }                                                                         /*     ...                           */
      if (nXB2>1)                                                               /*     Fetch B if lpPar2 is a matrix */
      {                                                                         /*     ...                           */
        if (bCmplxZ) CData_CblockFetch(idPar2,(COMPLEX64*)B,nB,nXC2,nXR2,-1);   /*     ...                           */
        else         CData_DblockFetch(idPar2,(  FLOAT64*)B,nB,nXC2,nXR2,-1);   /*     ...                           */
      }                                                                         /*     ...                           */
      if(bCmplxZ) nErr = dlm_matropC((COMPLEX64*)Z,NULL,NULL,                   /*     Do complex matrix operation   */
          (COMPLEX64*)A,nXRa,nXCa,(COMPLEX64*)B,nXRb,nXCb,nOpcode);             /*       |                           */
      else        nErr = dlm_matrop ((  FLOAT64*)Z,NULL,NULL,                   /*     Do matrix operation           */
          (  FLOAT64*)A,nXRa,nXCa,(  FLOAT64*)B,nXRb,nXCb,nOpcode);             /*       |                           */

      /* Handle errors */                                                       /*     - - - - - - - - - - - - - - - */
      IF_NOK(nErr) sprintf(sMsg," (in operation %s",dlm_matrop_name(nOpcode));  /*     Prepare add'tl error info     */
      switch (nErr)                                                             /*     Branch for error codes        */
      {                                                                         /*     >>                            */
      case ERR_MEM:                                                             /*       Memory error                */
        CERROR(data,ERR_MEM,0,0,0);                                             /*         Error message             */
        break;                                                                  /*       ==                          */
      case ERR_MDIM:                                                            /*       Dimension error             */
        strcat(sMsg,")");                                                       /*         Finish add'tl error info  */
        CERROR(data,DATA_MDIM,sMsg,0,0);                                        /*         Error message             */
        break;                                                                  /*       ==                          */
      case ERR_MSQR:                                                            /*       Dimension error             */
        strcat(sMsg,", matrix not square)");                                    /*         Finish add'tl error info  */
        CERROR(data,DATA_MDIM,sMsg,0,0);                                        /*         Error message             */
        break;                                                                  /*       ==                          */
      case ERR_MSGL:                                                            /*       Singular matrix error       */
        strcat(sMsg,", matrix singular)");                                      /*         Finish add'tl error info  */
        CERROR(data,DATA_MDATA_WARN,(long)nB,sMsg,0);                           /*         Error message             */
        break;                                                                  /*       ==                          */
      case ERR_MTYPE:                                                           /*       Wrong matrix type           */
        strcat(sMsg,")");                                                       /*         Finish add'tl error info  */
        CERROR(data,DATA_NOSUPPORT,"Matrix operation for numeric type",sMsg,0); /*         Error message             */
        break;                                                                  /*       ==                          */
      case ERR_MSUPP:                                                           /*       Wrong matrix type           */
        strcat(sMsg,")");                                                       /*         Finish add'tl error info  */
        CERROR(data,DATA_NOSUPPORT,"Matrix operation",sMsg,0);                  /*         Error message             */
        break;                                                                  /*       ==                          */
      }                                                                         /*     <<                            */

      /* Store output */                                                        /*     - - - - - - - - - - - - - - - */
      if (bCmplxZ)                                                              /*     Complex output                */
      {                                                                         /*     >>                            */
        CData_CblockStore(idRes,(COMPLEX64*)Z,nB,nXC,nXR,-1);                   /*       Store result                */
        if (lpsSignature!= NULL)                                                /*       Is given by __mtab ? >>     */
          switch(*lpsSignature)                                                 /*         Whats given?              */
          {                                                                     /*         >>                        */
            case 'R':                                                           /*           Real matrix             */
            case 'r': CData_Tconvert(idRes,idRes,T_DOUBLE);break;               /*           Real scalar             */
          }                                                                     /*         <<                        */
      }                                                                         /*     <<                            */
      else                                                                      /*     Real content                  */
        CData_DblockStore(idRes,(FLOAT64*)Z,nB,nXC,nXR,-1);                     /*       Store result                */
    }                                                                           /*   <<                              */
  }                                                                             /* <<                                */

  /* Clean up */                                                                /* --------------------------------- */
  if (nXB1) dlp_free(A);                                                        /* Free copy buffer for matrix A     */
  if (nXB2) dlp_free(B);                                                        /* Free copy buffer for matrix B     */
  dlp_free(Z);                                                                  /* Free copy buffer for matrix Z     */
  if (idRes!=idDst)                                                             /* Result was identical with a param.*/
  {                                                                             /* >>                                */
    CData_Copy(BASEINST(idDst),BASEINST(idRes));                                /*   Copy result instance            */
    IDESTROY(idRes);                                                            /*   Destroy result buffer           */
  }                                                                             /* <<                                */
  if (nCheck>0)                                                                 /* On verbose level 1                */
  {                                                                             /* >>                                */
    printf("\n"); dlp_fprint_x_line(stdout,'-',dlp_maxprintcols());             /*   Print delimiter                 */
    printf("\n");                                                               /*   ...                             */
  }
                                                                                   /* <<                                */
  return O_K;                                                                   /* Poooh!                            */
}

/*
 * Manual page at matrix.def
 */
INT16 CGEN_SPUBLIC CMatrix_Invert
(
  CData* A,
  CData* Z,
  CData* Y
)
{
  INT32    nB   = 0;                                                             /* Current block (=matrix)           */
  INT32    nXB  = 0;                                                             /* Number of blocks (=matrices)      */
  INT32    nXD  = 0;                                                             /* Dimensions of (square) matrices   */
  FLOAT64* lpnZ = NULL;                                                          /* Matrix processing buffer          */
  FLOAT64  nDr  = 0.;                                                            /* Double buffer                     */

  if (!Z) return NOT_EXEC;                                                      /* No dest. instance, no service     */
  if (CData_IsEmpty(A))                                                         /* Source empty (nothing to be done) */
  {                                                                             /* >>                                */
    CData_Reset(BASEINST(Z),TRUE);                                              /*   Reset destination instance      */
    return NOT_EXEC;                                                            /*   Return                          */
  }                                                                             /* <<                                */
  nXB = CData_GetNBlocks(A);                                                    /* Get number of matrices in src.    */
  nXD = CData_GetNRecsPerBlock(A);                                              /* Get dimension of src. (# columns) */
  if (nXD!=CData_GetNNumericComps(A))                                           /* Check if src. matrices are square */
  {                                                                             /* >> (NO)                           */
    CData_Reset(BASEINST(Z),TRUE);                                              /*   Reset destination instance      */
    CDlpObject_Error(BASEINST(A),__FILE__,__LINE__,DATA_MDIM,                   /*   Matrix dimension error          */
      " (not a square matrix)",0,0);                                            /*   |                               */
    return NOT_EXEC;                                                            /*   Return                          */
  }                                                                             /* <<                                */
  if (Y==A || Y==Z) Y=NULL;                                                     /* 2nd result cannot be idDst/idSrc  */
  if (Z!=A) CData_Copy(BASEINST(Z),BASEINST(A));                                /* Copy (numbers will be overwritten)*/
  CData_Array(Y,T_DOUBLE,1,nXB); CData_SetNBlocks(Y,nXB);                       /* Prepare second result instance    */
  lpnZ = (FLOAT64*)dlp_calloc(nXD*nXD,sizeof(FLOAT64));                         /* Create matrix processing buffer   */
  for (nB=0; nB<nXB; nB++)                                                      /* Loop over blocks                  */
  {                                                                             /* >>                                */
    CData_DblockFetch(A,lpnZ,nB,nXD,nXD,-1);                                    /*   Fetch source matrix             */
    dlm_invert_gel(lpnZ,nXD,&nDr);                                              /*   Do Gaussian elimination inversn.*/
    CData_DblockStore(Z,lpnZ,nB,nXD,nXD,-1);                                    /*   Store inverse matrix            */
    CData_Dstore(Y,nDr,nB,0);                                                   /*   Store 2nd result (rank or det)  */
  }                                                                             /* <<                                */
  dlp_free(lpnZ);                                                               /* Free matrix processing buffer     */
  return O_K;                                                                   /* All done                          */
}

/*
 * Manual page at matrix.def
 */
INT16 CGEN_SPUBLIC CMatrix_Eigen
(
  CData* A,
  CData* V,
  CData* L,
  BOOL   bInv,
  BOOL   bNorm
)
{
  if (bInv) return CMatrix_IeigenInt(A,V,L,bNorm);
  else      return CMatrix_EigenInt (A,V,L,bNorm);
}

/*
 * Manual page at matrix.def
 */
/*
 * Manual page at matrix.def
 */
INT16 CGEN_SPUBLIC CMatrix_Submat
(
  CData* A,
  INT32   i,
  INT32   k,
  INT32   n,
  INT32   m,
  CData* Z
)
{
  INT32 nC0     = 0;                                                             /* i - First data component to select*/
  INT32 nR0     = 0;                                                             /* k - First data record to select   */
  INT32 nXC     = 0;                                                             /* n - No. of data comps. to select  */
  INT32 nXR     = 0;                                                             /* m - No. of data recs. to select   */
  INT32 nXB     = 0;                                                             /* Number of matrices                */
  INT32 nRpb    = 0;                                                             /* Number of records per block (A)   */
  INT32 nB      = 0;                                                             /* Current block                     */
  INT32 nR      = 0;                                                             /* Current record                    */
  INT32 nC      = 0;                                                             /* Current component                 */
  INT32 nCa     = 0;                                                             /* Current source component          */
  BOOL bAisZ   = FALSE;                                                         /* Src. and dst. instance identical  */

  nC0 = i;                                                                      /* First data component to select    */
  nR0 = k;                                                                      /* First data record to select       */
  nXC = n;                                                                      /* No. of data comps. to select      */
  nXR = m;                                                                      /* No. of data recs. to select       */

  /* Validate */                                                                /* --------------------------------- */
  if (!Z) { CERROR(data,ERR_NULLARG,"Z",0,0); return ERR_NULLARG; }             /* No destination inst., no service  */

  /* Initialize - Handle overlapping arguments */                               /* --------------------------------- */
  if (A==Z)                                                                     /* Src. and dst. inst. are identical */
  {                                                                             /* >>                                */
    ICREATEEX(CData,Z,"CMatrix_Submat.~Z",NULL);                                /*   Create tmp. dst. instance       */
    bAisZ = TRUE;                                                               /*   Remember that                   */
  }                                                                             /* <<                                */

  /* Initialize - Validate arguments */                                         /* --------------------------------- */
  if (nR0<0) { nXR+=nR0; nR0=0; }                                               /* Min. first record is 0            */
  if (nC0<0) { nXC+=nC0; nC0=0; }                                               /* Min. first component is 0         */
  if (nR0+nXR>CData_GetNRecs (A)) nXR = CData_GetNRecs (A)-nR0;                 /* Max. last record is length        */
  if (nC0+nXC>CData_GetNComps(A)) nXC = CData_GetNComps(A)-nC0;                 /* Max. last component is dimension  */

  /* Initialize - Prepare destination instance */                               /* --------------------------------- */
  CData_Reset(BASEINST(Z),TRUE);                                                /* Reset destination                 */
  for (nC=nC0; nC<nXC+nC0; nC++)                                                /* Loop over src. components to copy */
    if (dlp_is_numeric_type_code(CData_GetCompType(A,nC)))                      /*   Check if they are numeric       */
      CData_AddComp(Z,CData_GetCname(A,nC),CData_GetCompType(A,nC));            /*     Create destination components */
  for (nC=0; nC<CData_GetNComps(A); nC++)                                       /* Loop over all source components   */
    if (dlp_is_symbolic_type_code(CData_GetCompType(A,nC)))                     /*   Check if they are symbolic      */
    {                                                                           /*   >>                              */
      CData_AddComp(Z,CData_GetCname(A,nC),CData_GetCompType(A,nC));            /*     Create destination components */
    }                                                                           /*   <<                              */
  nXB  = CData_GetNBlocks(A);                                                   /* Get number of matrices            */
  nRpb = CData_GetNRecsPerBlock(A);                                             /* Get number of records per block   */
  CData_Allocate(Z,nXB*nXR);                                                    /* Allocate destination records      */
  CData_SetNBlocks(Z,nXB);                                                      /* Set number of matrices            */

  /* Final check */                                                             /* --------------------------------- */
  if (nXR<=0 || nXC<=0 || CData_GetNNumericComps(Z)==0)                         /* No items to be copied             */
  {                                                                             /* >>                                */
    CData_Reset(BASEINST(Z),TRUE);                                              /*   Reset destination               */
    if (bAisZ) { CData_Copy(BASEINST(A),BASEINST(Z)); IDESTROY(Z); }            /*   Destroy tmp. instance           */
    return O_K;                                                                 /*   That's kind of ok               */
  }                                                                             /* <<                                */

  /* Copy data */                                                               /* --------------------------------- */
  for (nC=0; nC<nXC; nC++)                                                      /* Loop over components to copy      */
    if (dlp_is_numeric_type_code(CData_GetCompType(A,nC+nC0)))                  /*   Check source comp. is numeric   */
      for (nB=0; nB<nXB; nB++)                                                  /*     Loop over blocks              */
        for (nR=0; nR<nXR; nR++)                                                /*       Loop over records to copy   */
          CData_Cstore(Z,CData_Cfetch(A,nR+nB*nRpb+nR0,nC+nC0),nR,nC);          /*         Copy single numeric value */
  for (nCa=0; nCa<CData_GetNComps(A); nCa++)                                    /* Loop over source components       */
    if (dlp_is_symbolic_type_code(CData_GetCompType(A,nCa)))                    /*   Source component is symbolic    */
    {                                                                           /*   >>                              */
      for (nB=0; nB<nXB; nB++)                                                  /*     Loop over blocks              */
        for (nR=0; nR<nXR; nR++)                                                /*       Loop over records to copy   */
          CData_Sstore(Z,CData_Sfetch(A,nR+nB*nRpb+nR0,nCa),nR,nC);             /*         Copy single string value  */
      nC++;                                                                     /*     Next destination component    */
    }                                                                           /*   <<                              */

  /* Clean up */                                                                /* --------------------------------- */
  if (bAisZ) { CData_Copy(BASEINST(A),BASEINST(Z)); IDESTROY(Z); }              /* Destroy tmp. instance             */
  return O_K;                                                                   /* All done                          */
}

/*
 * Manual page at fst.def
 */
INT16 CGEN_SPRIVATE CMatrix_FactLDL(CData *A,CData *L,CData *D)
{
  void *lpL=NULL;                                                                /* Buffer for matrix L               */
  void *lpD=NULL;                                                                /* Buffer for matrix D               */
  INT32 N;                                                                       /* Dimension of matrizies            */
  INT32 b;                                                                       /* Current block index               */
  INT32 B;                                                                       /* Number of blocks                  */
  BOOL isComplex = FALSE;                                                        /* Flag input is complex matrix      */

  N=CData_GetNComps(A);                                                          /* Get matrix dimension              */
  B=CData_GetNBlocks(A);                                                         /* Get number of blocks              */

  /* Validation */                                                               /* --------------------------------- */
  if (CData_IsEmpty(A)) return IERROR(A,DATA_EMPTY,"idA",0,0);                   /* Need matrix to factorize          */
  if (N!=CData_GetNRecsPerBlock(A)) return IERROR(A,DATA_MDIM,"idA",0,0);        /* A should be quadratic matrix      */
  if(dlp_is_complex_type_code(CData_GetCompType(A,0))) isComplex = TRUE;         /* Test for complex matrix           */
  if(isComplex) {                                                                /* Is complex >>                     */
    lpL=(COMPLEX64*)dlp_calloc(N*N,sizeof(COMPLEX64));                           /*   Alloc complex matrix L          */
    lpD=(COMPLEX64*)dlp_calloc(N*N,sizeof(COMPLEX64));                           /*   Alloc complex matrix D          */
  } else {                                                                       /* << else                           */
    lpL=(FLOAT64*)dlp_calloc(N*N,sizeof(FLOAT64));                               /*   Alloc real matrix L             */
    lpD=(FLOAT64*)dlp_calloc(N*N,sizeof(FLOAT64));                               /*   Alloc real matrix D             */
  }
  CData_Copy(BASEINST(L),BASEINST(A));                                           /* Initialize matrix L               */
  CData_Copy(BASEINST(D),BASEINST(A));                                           /* Initialize matrix D               */
  for(b=0;b<B;b++){                                                              /* Loop over all blocks >>           */
    if(isComplex) {                                                              /*   Is complex >>                   */
      CData_CblockFetch(A,(COMPLEX64*)lpL,b,N,N,-1);                             /*     Read current block from matrix*/
      dlm_factldlC((COMPLEX64*)lpL,(COMPLEX64*)lpD,N);                           /*     Calculation LDL-Factorization */
      CData_CblockStore(L,(COMPLEX64*)lpL,b,N,N,-1);                             /*     Save lower triag. mat. in L   */
      CData_CblockStore(D,(COMPLEX64*)lpD,b,N,N,-1);                             /*     Save diagonal matrix in D     */
    } else {                                                                     /*   else                            */
      CData_DblockFetch(A,(FLOAT64*)lpL,b,N,N,-1);                               /*     Read current block from matrix*/
      dlm_factldl((FLOAT64*)lpL,(FLOAT64*)lpD,N);                                /*     Calculation LDL-Factorization */
      CData_DblockStore(L,(FLOAT64*)lpL,b,N,N,-1);                               /*     Save lower triag. mat. in L   */
      CData_DblockStore(D,(FLOAT64*)lpD,b,N,N,-1);                               /*     Save diagonal matrix in D     */
    }                                                                            /*   <<                              */
  }                                                                              /* <<                                */
  dlp_free(lpL);                                                                 /* Free buffer L                     */
  dlp_free(lpD);                                                                 /* Free buffer D                     */
  return O_K;                                                                    /* All done                          */
}

INT16 CGEN_PUBLIC CMatrix_Expand
(
  CMatrix*    _this,
  CData*      idSrc,
  INT32       nIcR,
  INT32       nIcC,
  INT32       nIcV,
  INT32       nRecs,
  INT32       nComps,
  const char* sOp,
  CData*      idDst
)
{
	INT32     nRs     = 0;
	INT32     nXRs    = 0;
	INT32     nR      = 0;
	INT32     nC      = 0;
	COMPLEX64 nVs     = CMPLX(0.);
	COMPLEX64 nVd     = CMPLX(0.);
	INT16     nOpcode = OP_NOOP;

  if (!idDst) return NOT_EXEC;
  if (nIcR<0 || nIcR>=CData_GetNComps(idSrc))
    return IERROR(idSrc,DATA_BADCOMP,(int)nIcR,BASEINST(idSrc)->m_lpInstanceName,0);
  if (nIcC<0 || nIcC>=CData_GetNComps(idSrc))
    return IERROR(idSrc,DATA_BADCOMP,(int)nIcC,BASEINST(idSrc)->m_lpInstanceName,0);
  if (nIcV>=CData_GetNComps(idSrc))
    return IERROR(idSrc,DATA_BADCOMP,(int)nIcV,BASEINST(idSrc)->m_lpInstanceName,0);
  if (dlp_strlen(sOp))
  {
    nOpcode = dlp_scalop_code(sOp);
    if (nOpcode<0) return IERROR(idSrc,DATA_OPCODE,sOp,"scalar",0);
  }

  CREATEVIRTUAL(CData,idSrc,idDst);
  CData_Array(idDst,T_DOUBLE,nComps,nRecs);
  if (!idSrc || CData_IsEmpty(idSrc))
  {
    DESTROYVIRTUAL(idSrc,idDst);
    return O_K;
  }

  nXRs = CData_GetNRecs(idSrc);
  for (nRs=0; nRs<nXRs; nRs++)
  {
    nR  = (INT32)CData_Dfetch(idSrc,nRs,nIcR);
    nC  = (INT32)CData_Dfetch(idSrc,nRs,nIcC);
    nVs = nIcV>=0?CData_Cfetch(idSrc,nRs,nIcV):CMPLX(1.);
    nVd = CData_Cfetch(idDst,nR,nC);
    CData_Cstore(idDst,dlp_scalopC(nVs,nVd,nOpcode),nR,nC);
  }

  DESTROYVIRTUAL(idSrc,idDst);
  return O_K;
}
/* EOF */
