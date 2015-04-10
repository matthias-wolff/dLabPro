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
#include "dlp_base.h"

/**
 * Get Daubechies index from m_lpsWvlType;
 *
 * @return Daubechies index if successfull, zero otherwise
 */
INT16 CGEN_PRIVATE CFWTproc::GetDindex()
{
  if(!dlp_strncmp(m_lpsWvltype,"haar",4)) return 2;
  else if(m_lpsWvltype[0]=='d') return atoi(m_lpsWvltype+1);
  else return 0;
}

/**
 * Get scaling function coefficients
 *
 * @return O_K if successfull, NOT_EXEC otherwise
 */
INT16 CGEN_PUBLIC CFWTproc::GetCoef(CData *idCoef)
{
  INT16  di=GetDindex();
  const FLOAT64 *h;
  if(!di) return IERROR(this,ERR_INVALARG,"wvltype invalid",0,0);
  if(!(h=dlm_fwt_geth(di))) return NOT_EXEC;
  CData_Array(idCoef,T_DOUBLE,1,di);
  memcpy(CData_XAddr(idCoef,0,0),h,sizeof(FLOAT64)*di);
  return O_K;
}

/**
 * Analyse a frame
 *
 * Derived instances of FBAproc should override method
 * Analyse() to add the desired functionality
 *
 * @return O_K if successfull, NOT_EXEC otherwise
 */
INT16 CGEN_PROTECTED CFWTproc::AnalyzeFrame()
{
  INT16  ret = NOT_EXEC;
  INT16  di=GetDindex();

  if(dlm_log2_i(m_nLen) == -1)                                               /* handle to short signal array */
  {
    return ret = FWT_DIM_ERROR;
  }
  if(!di) return IERROR(this,ERR_INVALARG,"wvltype invalid",0,0);

  return dlm_fwt_dx((FLOAT64*)m_idRealFrame->XAddr(0,0),
                       (FLOAT64*)m_idRealFrame->XAddr(0,0),
                       m_nLen,
                       di,
                       m_nLevel);
}


