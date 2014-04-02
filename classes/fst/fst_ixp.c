/* dLabPro class CFst (fst)
 * - Import/export methods
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
#include "dlp_fst.h"

/**
 * Appends units from a source automaton instance to this instance.
 *
 * @param _this      Pointer to destination automaton instance
 * @param itSrc      Pointer to source automaton instance
 * @param nFirstUnit Index of first unit of <code>itSrc</code> to append to this instance
 * @param nCount     Number of units to append
 * @return O_K if successfull, a (negative) error code otherwise
 */
INT16 CGEN_PUBLIC CFst_CatEx(CFst* _this, CFst* itSrc, INT32 nFirstUnit, INT32 nCount)
{
  INT32 nU    = 0;  /* Current unit */
  INT32 nXUd  = 0;  /* Number of units in destination */
  INT32 nFSs  = 0;  /* First source state to append */
  INT32 nXSs  = 0;  /* Number of source states to append */
  INT32 nFTs  = 0;  /* First source transition to append */
  INT32 nXTs  = 0;  /* Number of source transitions to append */
  INT32 nXXSd = 0;  /* Total number of states in destination */
  INT32 nXXTd = 0;  /* Total number of transitions in destination */

  /* Validate */
  CHECK_THIS_RV(NOT_EXEC);
  CFst_Check(_this);
  CFst_Check(itSrc);

  if (nFirstUnit        < 0            ) nFirstUnit = 0;
  if (nFirstUnit+nCount > UD_XXU(itSrc)) nCount     = UD_XXU(itSrc)-nFirstUnit;
  if (nCount            <=0            ) return NOT_EXEC;

  /* Initialize */
  CREATEVIRTUAL(CFst,itSrc,_this);

  /* If destination empty copy all properties from source */
  if (UD_XXU(_this)==0)
  {
    CFst_Copy(BASEINST(_this),BASEINST(itSrc));
    CData_SetNRecs(AS(CData,_this->ud),0);
    CData_SetNRecs(AS(CData,_this->sd),0);
    CData_SetNRecs(AS(CData,_this->td),0);
    CData_Reset(_this->is,0);
    CData_Reset(_this->os,0);
  }

  /* Get some metrics */
  nXUd  = UD_XXU(_this);
  nXXSd = UD_XXS(_this);
  nXXTd = UD_XXT(_this);
  nFSs  = UD_FS(itSrc,nFirstUnit);
  nFTs  = UD_FT(itSrc,nFirstUnit);
  for (nU=nFirstUnit,nXSs=0,nXTs=0; nU<nFirstUnit+nCount; nU++)
  {
    nXSs+=UD_XS(itSrc,nU);
    nXTs+=UD_XT(itSrc,nU);
  }

  /* Cat descriptor tables */
  CDlpTable_CatEx(AS(CData,_this->ud)->m_lpTable,AS(CData,itSrc->ud)->m_lpTable,nFirstUnit,nCount);
  CDlpTable_CatEx(AS(CData,_this->sd)->m_lpTable,AS(CData,itSrc->sd)->m_lpTable,nFSs      ,nXSs  );
  CDlpTable_CatEx(AS(CData,_this->td)->m_lpTable,AS(CData,itSrc->td)->m_lpTable,nFTs      ,nXTs  );

  /* Adjust unit descriptions */
  for (nU=nXUd; nU<nXUd+nCount; nU++)
  {
    UD_FS(_this,nU) += (nXXSd-nFSs);
    UD_FT(_this,nU) += (nXXTd-nFTs);
  }

  /* Copy input and output symbol table components */
  if(!IS_XXS(_this) && IS_XXS(itSrc)) CData_Copy(_this->is,itSrc->is);
  if(!OS_XXS(_this) && OS_XXS(itSrc)) CData_Copy(_this->os,itSrc->os);

  /* Finalize */
  DESTROYVIRTUAL(itSrc,_this);
  CFst_Check(_this); /* TODO: Remove after debugging */
  return O_K;
}

/*
 * Manual page at fst_man.def
 */
INT16 CGEN_PUBLIC CFst_Cat(CFst* _this, CFst* itSrc)
{
  /* Validate */
  CHECK_THIS_RV(NOT_EXEC);
  CFst_Check(_this);
  CFst_Check(itSrc);

  return CFst_CatEx(_this,itSrc,0,UD_XXU(itSrc));
}

/*
 * Manual page at fst_man.def
 */
INT16 CGEN_PUBLIC CFst_CopyUi(CFst* _this, CFst* itSrc, CData* idIndex, INT32 nPar)
{
  INT32 i  = 0;
  INT32 nU = 0;

  /* Validate */
  CHECK_THIS_RV(NOT_EXEC);
  CFst_Check(_this);
  CFst_Check(itSrc);

  if (idIndex)
  {
    if (CData_IsEmpty(idIndex)) return NOT_EXEC;
    if (nPar<0)
      for (i=0; i<CData_GetNComps(idIndex); i++)
        if (dlp_is_numeric_type_code(CData_GetCompType(idIndex,i)))
          { nPar=i; break; }

    if
    (
      nPar<0 || nPar>=CData_GetNComps(idIndex) ||
      !dlp_is_numeric_type_code(CData_GetCompType(idIndex,nPar))
    )
    {
      return IERROR(_this,FST_BADID,"component",nPar,0);
    }
  }

  /* Initialize */
  CREATEVIRTUAL(CFst,itSrc,_this);
  CFst_Reset(BASEINST(_this),TRUE);
  
  if (idIndex)
  {
    /* Loop over records of idIndex */
    for (i=0; i<CData_GetNRecs(idIndex); i++)
    {
      nU = (INT32)CData_Dfetch(idIndex,i,nPar);
      if (nU>=0 && nU<UD_XXU(itSrc)) {
        DLPASSERT(OK(CFst_CatEx(_this,itSrc,nU,1)))
      } else IERROR(_this,FST_BADID2,"unit",nU,0);
    }
  }
  else if (nPar<0)
  {
    /* Loop over all Units */
    for (nU=0; nU<UD_XXU(itSrc); nU++)
      DLPASSERT(OK(CFst_CatEx(_this,itSrc,nU,1)));
  }
  else
  {
    /* Copy unit nPar */
    if (nPar>=0 && nPar<UD_XXU(itSrc)) { DLPASSERT(OK(CFst_CatEx(_this,itSrc,nPar,1))) }
    else IERROR(_this,FST_BADID2,"unit",nPar,0);
  }

  /* Copy input and output symbol table (if not unit specific) */               /* --------------------------------- */
  if(IS_XXS(itSrc)!=UD_XXU(itSrc)) CData_Copy(_this->is,itSrc->is);             /* Copy input symbol table           */
  if(OS_XXS(itSrc)!=UD_XXU(itSrc)) CData_Copy(_this->os,itSrc->os);             /* Copy output symbol table          */

  /* Finalize */
  DESTROYVIRTUAL(itSrc,_this);
  CFst_Check(_this); /* TODO: Remove after debugging */
  return O_K;
}

/* EOF */
