// dLabPro class CFFTproc (FFTproc)
// - Class CFFTproc - FFT wrapper code
//
// AUTHOR : Matthias Eichner, Dresden
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


#include "dlp_fftproc.h"
#include "dlp_math.h"

/**
 * Analyse
 *
 * Derived instances of FBAproc should override method
 * Analyse() to add the desired functionality
 *
 * @return O_K if successfull, NOT_EXEC otherwise
 */
INT16 CGEN_PUBLIC CFFTproc::AnalyzeFrame()
{
  INT16 ret = O_K;

  if((ret = dlm_fft((FLOAT64*)m_idRealFrame->XAddr(0,0),(FLOAT64*)m_idImagFrame->XAddr(0,0),m_nLen,FALSE)) != O_K) {
    if(ret == ERR_MDIM) IERROR(this, FBA_BADFRAMELEN, m_nLen, "FFT", 0);
    return NOT_EXEC;
  }

  MAG();
  if(m_bNse) dlm_noiserdc((FLOAT64*)m_idRealFrame->XAddr(0,0),m_nLen,TRUE);
  if(m_bLmag) LOG10();

  return O_K;
}

/**
 * Frequency warping using phase of allpass (1/z-a)/(1-a*1/z)
 * (bilinear transform)
 *
 * w2 = w1 + 2*arctan((a*sin(w1))/(1-a*cos(w1)))
 *
 * This warping can be used for vocal tract length normalization
 * or for mel spektrum approximation.
 *
 * @param dIn     Input vector sequence to warp
 * @param dOut    Output vector sequence
 * @param nLambda Warping constant a
 * @param nDim    Output vector dimension
 * @return O_K if successfull, NOT_EXEC otherwise
 */

INT16 CGEN_PUBLIC CFFTproc::Warp(CData* dIn, CData* dOut, FLOAT64 nLambda, INT32 nDim)
{
  INT16 i;

  if (dIn ==NULL)     return IERROR(this,ERR_NULLARG,0,0,0);
  if (dOut==NULL)     return IERROR(this,ERR_NULLARG,0,0,0);
  if (dIn->IsEmpty()) return IERROR(dIn ,DATA_EMPTY,dIn->m_lpInstanceName,0,0);
  if (dIn->GetCompType(0)!=T_DOUBLE)
    return IERROR(dIn,DATA_BADCOMPTYPE,dIn->m_lpInstanceName,"double",0);

  CREATEVIRTUAL(CData,dIn,dOut);
  dOut->Reset(TRUE);
  ISETFIELD_LVALUE(this,"lambda",nLambda);
  m_nOutDim=(INT16)nDim;
  //DLPASSERT(FALSE);

  for(i=0;i<dIn->GetNRecs();i++)
  {
    m_idRealFrame->SelectRecs(dIn,i,1);
    WARP();
    dOut->Cat(m_idRealFrame);
  }

  DESTROYVIRTUAL(dIn,dOut);

  return O_K;
}

/**
 * Prepare output instance for analysis.
 */
void CGEN_VPUBLIC CFFTproc::PrepareOutput(CData* dResult)
{
  if (dResult)
  {
    CFBAproc::PrepareOutput(dResult);

    // Set component increment in [kHz]
    ISETFIELD_RVALUE(dResult,"cinc" ,m_nSrate/1000.0/dlm_pow(2,m_nOrder));
    ISETFIELD_SVALUE(dResult,"cunit","kHz"                              );
  }
}

/* EOF */
