// dLabPro class CPMproc (PMproc)
// - Class CPMproc - PM wrapper code
//
// AUTHOR : Guntram Strecha, Dresden
// PACKAGE: dLabPro/classes
// 
// Copyright 2013 dLabPro contributors and others (see COPYRIGHT file) 
// - Chair of System Theory and Speech Technology, TU Dresden
// - Chair of Communications Engineering, BTU Cottbus
// 
// This file is part of dLabPro.
// 
// dLabPro is free software: you can redistribute it and/or modify it under the
// terms of the GNU Lesser General Public License as published by the Free
// Software Foundation, either version 3 of the License, or (at your option)
// any later version.
// 
// dLabPro is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
// details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with dLabPro. If not, see <http://www.gnu.org/licenses/>.


#include "dlp_pmproc.h"
#include "dlp_math.h"

INT16 CGEN_PROTECTED CPMproc::Analyze(data* dSignal, data* dPM) {
  INT16  ret = NOT_EXEC;
  INT16  bLabels  = FALSE;
  data*  dLabels  = NULL;

  ICREATEEX(CData,dLabels,"~lab",NULL);

  if(dlp_is_symbolic_type_code(dSignal->GetCompType(dSignal->GetNComps()-1))) {
    dLabels->SelectComps(dSignal,dSignal->GetNComps()-1,1);
    dSignal->DeleteComps(dSignal->GetNComps()-1,1);
    bLabels=TRUE;
  }
  if(m_bChfa == true) {
    ret = Chfa(dSignal, dPM);
  } else if(m_bGcida  == true) {
    ret = Gcida(dSignal, dPM);
  } else if(m_bEpochdetect  == true) {
    ret = Epochdetect(dSignal, dPM);
  } else {
    ret = Hybrid(dSignal, dPM);
  }

  if(bLabels==TRUE) {
    dSignal->Join(dLabels);
  }
  IDESTROY(dLabels);

  return ret;
}

/**
 *  Expand/reduce number of pitch markers to fit new target sum of period length.
 *
 * @param idSrc        source object
 * @param idDst        target object
 * @param n            target length
 * @return O_K if sucessfull, not exec otherwise
 */
INT16 CGEN_PUBLIC CPMproc::ExpandPm(CData *idSrc, CData* idDst, INT32 n)
{
  if(CData_IsEmpty(idSrc) == TRUE) return NOT_EXEC;
  if(n <= 0)                       return NOT_EXEC;

  CREATEVIRTUAL(CData,idSrc,idDst);

  DLPASSERT(CData_GetNComps(idSrc) == 2);

  CData_Reset(idDst, TRUE);
  CData_Scopy(idDst,idSrc);

  INT32    nRecsNew = 0;
  INT32    nRecs    = (INT32)CData_GetNRecs(idSrc);
  INT16* pm_new = NULL;

  if(dlm_pm_expand((INT16*)CData_XAddr(idSrc,0,0), nRecs, &pm_new, &nRecsNew, n) != O_K) {
    return IERROR(this,ERR_NULLARG, "", NULL, NULL);
  }
  CData_Allocate(idDst,nRecsNew);
  dlp_memmove(CData_XAddr(idDst,0,0), pm_new, nRecsNew*2*sizeof(INT16));
  dlp_free(pm_new);
  CData_CopyDescr(idDst,idSrc);

  /* clean up */
  DESTROYVIRTUAL(idSrc,idDst)

  return(O_K);
}

/**
 *  Expand/reduce number of pitch markers to new number.
 *
 * @param idSrc        source object
 * @param idDst        target object
 * @param n            target number of pitch markers
 * @return O_K if sucessfull, not exec otherwise
 */
INT16 CGEN_PUBLIC CPMproc::CompressPm(CData *idSrc, CData* idDst, INT32 n)
{
  if(CData_IsEmpty(idSrc) == TRUE) return NOT_EXEC;
  if(n <= 0)                       return NOT_EXEC;

  CREATEVIRTUAL(CData,idSrc,idDst);

  DLPASSERT(CData_GetNComps(idSrc) == 2);
  CData_Reset(idDst, TRUE);
  CData_Scopy(idDst,idSrc);
  CData_Allocate(idDst, n);

  if(dlm_pm_compress((INT16*)CData_XAddr(idSrc,0,0),
                     CData_GetNRecs(idSrc),
                     (INT16*)CData_XAddr(idDst,0,0),
                     CData_GetNRecs(idDst)) != O_K) {
    return IERROR(this,ERR_NULLARG, "", NULL, NULL);
  }
  CData_CopyDescr(idDst,idSrc);

  /* clean up */
  DESTROYVIRTUAL(idSrc,idDst)

  return(O_K);
}

/**
 *  Add periods to reach desired sum od periods.
 *
 * @param idSrc        source object
 * @param idDst        target object
 * @param n            target number of pitch markers
 * @param method       fill method
 * @return O_K if sucessfull, not exec otherwise
 */
INT16 CGEN_PUBLIC CPMproc::Fill(CData *idSrc, CData* idDst, INT32 n, const char* method)
{
  INT32       i               = 0;
  INT32       n_new           = 0;
  INT32       sum             = 0;
  INT16      mean_s          = 0;
  FLOAT64     mean            = 0.0;
  const char* method_default = "mean";

  if(CData_IsEmpty(idSrc) == TRUE) return NOT_EXEC;
  if(n <= 0)                       return NOT_EXEC;
  if(method == NULL)               method = method_default;

  CREATEVIRTUAL(CData,idSrc,idDst);

  DLPASSERT(CData_GetNComps(idSrc) == 2);
  CData_Reset(idDst, TRUE);
  CData_Copy(idDst,idSrc);

  for(i = 0; i < idSrc->GetNRecs(); i++) {
    sum += (INT32)idSrc->Dfetch(i,0);
  }

  if(!strcmp(method, "mean")) {
    mean = (FLOAT64)sum / (FLOAT64)idSrc->GetNRecs();
    n_new = (INT32)((FLOAT64)(n-sum) / mean);
    if(n_new >= 0) {
      n_new = MAX(n_new,1);
      i = idDst->GetNRecs();
      idDst->AddRecs(n_new, 1);
      while((sum<n) && (i < idDst->GetNRecs())) {
        mean_s = (INT16)((FLOAT64)(n-sum) / (FLOAT64)(idDst->GetNRecs()-i));
        idDst->Dstore(mean_s,i,0);
        idDst->Dstore(0,i,1);
        i++;
        sum += mean_s;
      }
    }
  }

  while((n-sum) > idDst->Dfetch(idDst->GetNRecs()-1,0)) {
    sum -= (INT32)idDst->Dfetch(idDst->GetNRecs()-1,0);
    idDst->Delete(idDst,idDst->GetNRecs()-1,1);
  }
  idDst->Dstore(idDst->Dfetch(idDst->GetNRecs()-1,0)-(sum-n),idDst->GetNRecs()-1,0);

  CData_CopyDescr(idDst,idSrc);

  /* clean up */
  DESTROYVIRTUAL(idSrc,idDst)

  return(O_K);
}

/**
 *  Convert (unequal spaced) pitch markers to f0-contour with equal spaced
 * sampling points.
 *
 * @param idSrc        source pitch marks
 * @param idDst        target fo-contour
 * @param n            target number of sampling points of f0-contour
 * @param srate        sampling rate
 * @return O_K if sucessfull, not exec otherwise
 */
INT16 CGEN_PUBLIC CPMproc::Pm2f0(CData *idSrc, CData* idDst, INT32 n, INT32 srate) {

  if(CData_IsEmpty(idSrc) == TRUE) return NOT_EXEC;
  if(n <= 0)                       return NOT_EXEC;

  DLPASSERT(CData_GetNComps(idSrc) == 2);

  CREATEVIRTUAL(CData,idSrc,idDst);

  CData_Reset(idDst, TRUE);
  CData_AddComp(idDst, "~F0", T_DOUBLE);

  CData_Allocate(idDst, n);

  dlm_pm2f0((INT16*)idSrc->XAddr(0,0), idSrc->GetNRecs(), (FLOAT64*)idDst->XAddr(0,0), idDst->GetNRecs(), srate);

  ISETFIELD_RVALUE(idDst, "fsr", 1000.0/srate);

  /* clean up */
  DESTROYVIRTUAL(idSrc,idDst)

  return(O_K);
}

/**
 *  Convert f0-contour with equal spaced sampling points to pitch markers.
 *
 * @param idSrc        source f0-contour
 * @param idDst        target pitch marker
 * @param n            target sum of pitch marker lengths
 * @param srate        sampling rate
 * @return O_K if sucessfull, not exec otherwise
 */
INT16 CGEN_PUBLIC CPMproc::F02pm(CData *idSrc, CData* idDst, INT32 n, INT32 srate) {

  INT16* p_pm = NULL;
  INT32    n_pm = 0;

  if(CData_IsEmpty(idSrc) == TRUE) return NOT_EXEC;
  if(n <= 0)                       return NOT_EXEC;

  DLPASSERT(CData_GetNComps(idSrc) == 1);

  CREATEVIRTUAL(CData,idSrc,idDst);

  CData_Reset(idDst, TRUE);

  dlm_f02pm((FLOAT64*)idSrc->XAddr(0,0), idSrc->GetNRecs(), &p_pm, &n_pm, n, srate);

  CData_AddComp(idDst, "pm",   T_SHORT);
  CData_AddComp(idDst, "v/uv", T_SHORT);
  CData_Allocate(idDst, n_pm);
  ISETFIELD_RVALUE(idDst, "fsr", 1000.0/srate);

  dlp_memmove(idDst->XAddr(0,0), p_pm, 2*n_pm*sizeof(INT16));

  /* clean up */
  DESTROYVIRTUAL(idSrc,idDst)

  dlp_free(p_pm);

  return(O_K);
}
