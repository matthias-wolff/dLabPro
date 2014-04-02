/* dLabPro class CFst (fst)
 * - Transducer composition and intersection
 *
 * AUTHOR : Maximiliano Cuevas, Matthias Wolff
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
#include "kzl_hash.h"
#include "dlp_fst.h"

#include "fst_cps_imp.c"

/*
 * Manual page at fst_man.def
 */
INT16 CGEN_PUBLIC CFst_Compose
(
  CFst* _this,
  CFst* itSrc1,
  CFst* itSrc2,
  INT32  nUnit1,
  INT32  nUnit2
)
{
  BOOL          bNoeps;                                                         /* No implicit insertion of e.-loops */
  BOOL          bNoint;                                                         /* No intermediate symbols flag      */
  INT16         nCheck  = 0;                                                    /* Verbose level                     */
  INT16         nVirt   = 0;                                                    /* Auxiliary: patching ovrl.args. bug*/
  const char *err;

  /* Validate */
  CHECK_THIS_RV(NOT_EXEC);
  CFst_Check(itSrc1);
  CFst_Check(itSrc2);
  if (itSrc1==NULL || itSrc2==NULL)
  {
    CFst_Reset(BASEINST(_this),TRUE);
    return O_K;
  }
  if (nUnit1<0 || nUnit1>=UD_XXU(itSrc1)) return IERROR(_this,FST_BADID,"unit",nUnit1,0);
  if (nUnit2<0 || nUnit2>=UD_XXU(itSrc2)) return IERROR(_this,FST_BADID,"unit",nUnit2,0);

  if (CFst_Wsr_GetType(itSrc1,NULL)!=CFst_Wsr_GetType(itSrc2,NULL))
    return IERROR(_this,FST_INCOMPATIBLE,"weight semirings","itSrc1 and itSrc2",0);
  if (CData_FindComp(AS(CData,itSrc1->td),NC_TD_TIS)<0)
    return IERROR(_this,FST_MISS,"input symbol component" ,"","transition table of itSrc1");
  if (CData_FindComp(AS(CData,itSrc1->td),NC_TD_TOS)<0)
    return IERROR(_this,FST_MISS,"output symbol component","","transition table of itSrc1");
  if (CData_FindComp(AS(CData,itSrc2->td),NC_TD_TIS)<0)
    return IERROR(_this,FST_MISS,"input symbol component" ,"","transition table of itSrc2");
  if (CData_FindComp(AS(CData,itSrc2->td),NC_TD_TOS)<0)
    return IERROR(_this,FST_MISS,"output symbol component","","transition table of itSrc2");
  bNoeps = _this->m_bNoeps;
  bNoint = _this->m_bNoint;
  nCheck = BASEINST(_this)->m_nCheck;

  /* Initialize - NO RETURNS BEYOND THIS POINT */
  /* HACK: Multiple overlapping arguments on _this not handled correctly by
           CREATEVIRTUAL --> */
  if (_this==itSrc1 && _this==itSrc2) return IERROR(_this,ERR_GENERIC,"Invalid arguments",0,0);
  if      (itSrc1==_this) { CREATEVIRTUAL(CFst,itSrc1,_this); nVirt=1; }
  else if (itSrc2==_this) { CREATEVIRTUAL(CFst,itSrc2,_this); nVirt=2; }
  /* <-- */

  if((err=fstc_compose(itSrc1,nUnit1,itSrc2,nUnit2,
      bNoeps,bNoint,nCheck,
      _this)))
    return IERROR(_this,FST_INVALID,err,0,0);

  /* Copy input and output symbol tables */
  CData_SelectComps(AS(CData,_this->is),AS(CData,itSrc1->is),IS_XXS(itSrc1)==UD_XXU(itSrc1)?nUnit1:0,1);
  CData_SelectComps(AS(CData,_this->os),AS(CData,itSrc2->os),OS_XXS(itSrc2)==UD_XXU(itSrc2)?nUnit2:0,1);

  /* Clean up */
  if (nVirt==1) { DESTROYVIRTUAL(itSrc1,_this); }
  if (nVirt==2) { DESTROYVIRTUAL(itSrc2,_this); }
  CFst_Check(_this);
  CFst_TrimStates(_this,0);

  return O_K;
}

/*
 * Manual page at fst_man.def
 */
INT16 CGEN_PUBLIC CFst_Intersect
(
  CFst* _this,
  CFst* itSrc1,
  CFst* itSrc2,
  INT32  nUnit1,
  INT32  nUnit2
)
{
  CFst*  itAux1  = NULL;
  CFst*  itAux2  = NULL;
  CData* idAux   = NULL;
  INT32   nIcTis1 = -1;
  INT32   nIcTos1 = -1;
  INT32   nIcTis2 = -1;
  INT32   nIcTos2 = -1;
  INT16  nErr    = O_K;

  nIcTis1 = CData_FindComp(AS(CData,itSrc1->td),NC_TD_TIS);
  nIcTos1 = CData_FindComp(AS(CData,itSrc1->td),NC_TD_TOS);
  nIcTis2 = CData_FindComp(AS(CData,itSrc2->td),NC_TD_TIS);
  nIcTos2 = CData_FindComp(AS(CData,itSrc2->td),NC_TD_TOS);

  if (nIcTis1<0) return IERROR(_this,FST_MISS,"input symbol component","","transition table of left operand");
  if (nIcTis2<0) return IERROR(_this,FST_MISS,"input symbol component","","transition table of right operand");

  ICREATEEX(CFst ,itAux1,"CFst_Intersect.itAux1",NULL);
  ICREATEEX(CFst ,itAux2,"CFst_Intersect.itAux2",NULL);
  ICREATEEX(CData,idAux ,"CFst_Intersect.idAux" ,NULL);
  CFst_Copy(BASEINST(itAux1),BASEINST(itSrc1));
  CFst_Copy(BASEINST(itAux2),BASEINST(itSrc2));

  if (nIcTos1>=0)
  {
    CData_DeleteComps(AS(CData,itAux1->td),nIcTos1,1);
    CData_SelectComps(idAux,AS(CData,itAux1->td),nIcTis1,1);
    CData_SetCname(idAux,0,NC_TD_TOS);
    CData_Join(AS(CData,itAux1->td),idAux);
  }
  if (nIcTos2>=0)
  {
    CData_DeleteComps(AS(CData,itAux2->td),nIcTos2,1);
    CData_SelectComps(idAux,AS(CData,itAux2->td),nIcTis2,1);
    CData_SetCname(idAux,0,NC_TD_TOS);
    CData_Join(AS(CData,itAux2->td),idAux);
  }

  nErr = CFst_Compose(_this,itAux1,itAux2,nUnit1,nUnit2);

  IDESTROY(itAux1);
  IDESTROY(itAux2);
  IDESTROY(idAux);

  return nErr;
}

/*
 * Manual page at fst_man.def
 */
INT16 CGEN_PUBLIC CFst_Product
(
  CFst* _this,
  CFst* itSrc1,
  CFst* itSrc2,
  INT32  nUnit1,
  INT32  nUnit2
)
{
  INT32      nC           = 0;                                                   /* Current component                  */
  INT32      nFCS2        = 0;                                                   /* 1st data comp. of state tab.itSrc2 */
  INT32      nXCS1        = 0;                                                   /* No. of comps. of state tab. itSrc1 */
  INT32      nXCS2        = 0;                                                   /* No. of comps. of state tab. itSrc2 */
  INT32      nFCT2        = 0;                                                   /* 1st data comp. of trans.tab.itSrc2 */
  INT32      nXCT1        = 0;                                                   /* No. of comps. of trans.tab. itSrc1 */
  INT32      nXCT2        = 0;                                                   /* No. of comps. of trans.tab. itSrc2 */
  INT32      nRls1        = 0;                                                   /* Rec. len. of state table of itSrc1 */
  INT32      nRls2        = 0;                                                   /* Rec. len. of state table of itSrc2 */
  INT32      nRlt1        = 0;                                                   /* Rec. len. of trans.table of itSrc1 */
  INT32      nRlt2        = 0;                                                   /* Rec. len. of trans.table of itSrc2 */
  FST_ITYPE nS1          = 0;
  FST_ITYPE nS2          = 0;
  FST_ITYPE nFS1         = 0;
  FST_ITYPE nFS2         = 0;
  FST_ITYPE nXS1         = 0;
  FST_ITYPE nXS2         = 0;
  FST_ITYPE nT           = 0;
  FST_ITYPE nIni         = 0;
  FST_ITYPE nTer         = 0;
  FST_ITYPE nT1          = 0;
  FST_ITYPE nT2          = 0;
  FST_ITYPE nFT1         = 0;
  FST_ITYPE nFT2         = 0;
  FST_ITYPE nXT1         = 0;
  FST_ITYPE nXT2         = 0;
  char*     lpsBuf       = NULL;                                                /* String buffer                      */
  INT16     nVirt        = 0;                                                   /* Auxiliary: patching ovrl.args. bug */
  BOOL      bNoloopsSave = FALSE;                                               /* Save buffer for /noloops option    */

  /* Validate */
  CHECK_THIS_RV(NOT_EXEC);
  if (itSrc1==NULL || itSrc2==NULL)
  {
    CFst_Reset(BASEINST(_this),TRUE);
    return O_K;
  }
  if (nUnit1<0 || nUnit1>=UD_XXU(itSrc1)) return IERROR(_this,FST_BADID,"unit",nUnit1,0);
  if (nUnit2<0 || nUnit2>=UD_XXU(itSrc2)) return IERROR(_this,FST_BADID,"unit",nUnit2,0);

  /* Initialize */
  /* HACK: Multiple overlapping arguments on _this not handled correctly by
           CREATEVIRTUAL --> */
  if (_this==itSrc1 && _this==itSrc2) return IERROR(_this,ERR_GENERIC,"Invalid arguments",0,0);
  if      (itSrc1==_this) { CREATEVIRTUAL(CFst,itSrc1,_this); nVirt=1; }
  else if (itSrc2==_this) { CREATEVIRTUAL(CFst,itSrc2,_this); nVirt=2; }
  /* <-- */
  bNoloopsSave = _this->m_bNoloops;
  CFst_Reset(BASEINST(_this),TRUE);
  _this->m_bNoloops = bNoloopsSave;

  /* Initialize destination state table */
  nFS1  = UD_FS(itSrc1,nUnit1);
  nXS1  = UD_XS(itSrc1,nUnit1);
  nFS2  = UD_FS(itSrc2,nUnit2);
  nXS2  = UD_XS(itSrc2,nUnit2);
  nRls1 = CData_GetRecLen(AS(CData,itSrc1->sd)) - CData_GetCompOffset(AS(CData,itSrc1->sd),IC_SD_DATA);
  nRls2 = CData_GetRecLen(AS(CData,itSrc2->sd)) - CData_GetCompOffset(AS(CData,itSrc2->sd),IC_SD_DATA);
  nXCS1 = CData_GetNComps(AS(CData,itSrc1->sd)) - IC_SD_DATA;
  nXCS2 = CData_GetNComps(AS(CData,itSrc2->sd)) - IC_SD_DATA;
  nFCS2 = IC_SD_DATA + nXCS1;
  for (nC=IC_SD_DATA; nC<IC_SD_DATA+nXCS1; nC++)
    CData_AddComp
    (
      AS(CData,_this->sd),
      CData_GetCname(AS(CData,itSrc1->sd),nC),
      CData_GetCompType(AS(CData,itSrc1->sd),nC)
    );
  for (nC=IC_SD_DATA; nC<IC_SD_DATA+nXCS2; nC++)
    CData_AddComp
    (
      AS(CData,_this->sd),
      CData_GetCname(AS(CData,itSrc2->sd),nC),
      CData_GetCompType(AS(CData,itSrc2->sd),nC)
    );

  /* Initialize destination transition table */
  nFT1  = UD_FT(itSrc1,nUnit1);
  nXT1  = UD_XT(itSrc1,nUnit1);
  nFT2  = UD_FT(itSrc2,nUnit2);
  nXT2  = UD_XT(itSrc2,nUnit2);
  nRlt1 = CData_GetRecLen(AS(CData,itSrc1->td)) - CData_GetCompOffset(AS(CData,itSrc1->td),IC_TD_DATA);
  nRlt2 = CData_GetRecLen(AS(CData,itSrc2->td)) - CData_GetCompOffset(AS(CData,itSrc2->td),IC_TD_DATA);
  nXCT1 = CData_GetNComps(AS(CData,itSrc1->td)) - IC_TD_DATA;
  nXCT2 = CData_GetNComps(AS(CData,itSrc2->td)) - IC_TD_DATA;
  nFCT2 = IC_TD_DATA + nXCT1;
  for (nC=IC_TD_DATA; nC<IC_TD_DATA+nXCT1; nC++)
    CData_AddComp
    (
      AS(CData,_this->td),
      CData_GetCname(AS(CData,itSrc1->td),nC),
      CData_GetCompType(AS(CData,itSrc1->td),nC)
    );
  for (nC=IC_TD_DATA; nC<IC_TD_DATA+nXCT2; nC++)
    CData_AddComp
    (
      AS(CData,_this->td),
      CData_GetCname(AS(CData,itSrc2->td),nC),
      CData_GetCompType(AS(CData,itSrc2->td),nC)
    );

  /* Copy input and output symbol tables */
  CData_Copy(_this->is,itSrc1->is);
  CData_Copy(_this->os,itSrc2->os);

  /* Initialize destination unit */
  lpsBuf = (char*)dlp_calloc
  (
    CData_GetCompType(AS(CData,itSrc1->ud),IC_UD_NAME) +
    CData_GetCompType(AS(CData,itSrc2->ud),IC_UD_NAME) + 2,
    sizeof(char)
  );
  sprintf
  (
    lpsBuf,"%s.%s",
    (const char*)CData_XAddr(AS(CData,itSrc1->ud),nUnit1,IC_UD_NAME),
    (const char*)CData_XAddr(AS(CData,itSrc2->ud),nUnit2,IC_UD_NAME)
  );
  CFst_Addunit(_this,lpsBuf);
  dlp_free(lpsBuf);

  /* Add product states */
  CFst_Addstates(_this,0,nXS1*nXS2,FALSE);

  /* Copy state qualification (including final state flag) */
  for (nS1=0; nS1<nXS1; nS1++)
    for (nS2=0; nS2<nXS2; nS2++)
    {
      if ((SD_FLG(itSrc1,nS1+nFS1)&0x01)==0x01 && (SD_FLG(itSrc2,nS2+nFS2)&0x01)==0x01)
        SD_FLG(_this,nS1*nXS2+nS2) |= 0x01;
      if (nRls1>0)
        dlp_memmove
        (
          CData_XAddr(AS(CData,_this ->sd),nS1*nXS2+nS2,IC_SD_DATA),
          CData_XAddr(AS(CData,itSrc1->sd),nS1+nFS1    ,IC_SD_DATA),
          nRls1
        );
      if (nRls2>0)
        dlp_memmove
        (
          CData_XAddr(AS(CData,_this ->sd),nS1*nXS2+nS2,nFCS2     ),
          CData_XAddr(AS(CData,itSrc2->sd),nS2+nFS2    ,IC_SD_DATA),
          nRls2
        );
    }

  CData_AddRecs(AS(CData,_this->td),nXT1*nXT2,_this->m_nGrany);
  /* Loop over all transitions of both factors */
  for (nT=0, nT1=nFT1; nT1<nFT1+nXT1; nT1++)
    for (nT2=nFT2; nT2<nFT2+nXT2; nT2++, nT++)
    {
      /* Get product of initial and terminal state */
      nIni = TD_INI(itSrc1,nT1)*nXS2 + TD_INI(itSrc2,nT2);
      nTer = TD_TER(itSrc1,nT1)*nXS2 + TD_TER(itSrc2,nT2);
      if (_this->m_bNoloops && nIni==nTer) continue;

      /* Add product transition */
      *(FST_ITYPE*)CData_XAddr(AS(CData,_this->td),nT,IC_TD_INI) = nIni;
      *(FST_ITYPE*)CData_XAddr(AS(CData,_this->td),nT,IC_TD_TER) = nTer;

      /* Copy transition qualification */
      dlp_memmove
      (
        CData_XAddr(AS(CData,_this ->td),nT ,IC_TD_DATA),
        CData_XAddr(AS(CData,itSrc1->td),nT1,IC_TD_DATA),
        nRlt1
      );
      dlp_memmove
      (
        CData_XAddr(AS(CData,_this ->td),nT ,nFCT2     ),
        CData_XAddr(AS(CData,itSrc2->td),nT2,IC_TD_DATA),
        nRlt2
      );
    }

  /* Finish destination instance */
  CData_SelectRecs(AS(CData,_this->td),AS(CData,_this->td),0,nT);
  UD_XT(_this,0) = nT;
  /* TODO: Trim result !? */

  /* Clean up */
  if (nVirt==1) { DESTROYVIRTUAL(itSrc1,_this); }
  if (nVirt==2) { DESTROYVIRTUAL(itSrc2,_this); }
  return O_K;
}

/* EOF */
