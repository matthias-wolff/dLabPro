// dLabPro class CFWTproc (FWTproc)
// - Class CFWTproc - Fast Wavelet Transformation
//
// AUTHOR : Soeren Wittenberg
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


#include "dlp_fwtproc.h"
#include "dlp_math.h"

/**
 * Synthesis
 *
 *
 * @return O_K if successfull, NOT_EXEC otherwise
 */
INT16 CGEN_PROTECTED CFWTproc::Synthesize(CData* idTrans, CData* idSignal)
{  
  INT32   nNrOfCoeff;   /* number of signal values */
  INT16  ret = NOT_EXEC;

  if (idTrans == NULL)    return IERROR(this,ERR_NULLINST,0,0,0);
  if (idTrans->IsEmpty())   return IERROR(idTrans,DATA_EMPTY,idTrans->m_lpInstanceName,0,0);
  if (idTrans->GetCompType(0)!=T_DOUBLE)
    return IERROR(idTrans,DATA_BADCOMPTYPE,0,idTrans->m_lpInstanceName,"double");

  // remove symbolic component
  if(dlp_is_symbolic_type_code(idTrans->GetCompType(idTrans->GetNComps()-1)))
  {
    idTrans->DeleteComps(idTrans->GetNComps()-1,1);
  }
  
  nNrOfCoeff = (INT32) idTrans->GetNComps();                                     /* get number of coefficients */
  // maybe the records containing the data
  if(nNrOfCoeff == 1)
  { 
    nNrOfCoeff = (INT32) idTrans->GetNRecs();                                    /* get number of coefficients */
  }
 
  /* allocate output data objects */
  idSignal->Reset(TRUE);
  idSignal->AddNcomps(T_DOUBLE, 1);
  idSignal->Alloc(nNrOfCoeff);
  idSignal->SetNRecs(nNrOfCoeff);

  if(m_bHaar == true)
  {
    ret = dlm_fwt_haar_inv((FLOAT64*)idTrans->XAddr(0,0),
                           (FLOAT64*)idSignal->XAddr(0,0),
                           (INT32)nNrOfCoeff,
                           (INT16)m_nLevel);
  }
  else
  {
    ret = dlm_fwt_d4_inv((FLOAT64*)idTrans->XAddr(0,0),
                         (FLOAT64*)idSignal->XAddr(0,0),
                         (INT32)nNrOfCoeff,
                         (INT16)m_nLevel);
  }  
  return ret;
}
