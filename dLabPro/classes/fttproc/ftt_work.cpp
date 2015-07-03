// dLabPro class CFTTproc (FTTproc)
// - Class CFTTproc - Fourier-t-transform
//
// AUTHOR : Steffen Kuerbis, TU Dresden
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
#include "dlp_fttproc.h"

/**
 * Analyse
 *
 * Derived instances of FBAproc should override method
 * Analyse() to add the desired functionality
 *
 * @return O_K if successful, NOT_EXEC otherwise
 */
INT16 CGEN_PUBLIC CFTTproc::AnalyzeFrame()
{
  return dlm_ftt_analyze(m_lpFtt, (FLOAT64*)m_idRealFrame->XAddr(0,0), m_nLen, m_nCoeff, m_nMinLog);
}

void CGEN_VPUBLIC CFTTproc::PrepareOutput(CData* dResult)
{
  CFBAproc::PrepareOutput(dResult);

  // Set component offset and increment in bark
  FLOAT32 f = m_nStartfreq/1000.;
  FLOAT32 b = 13*atan(0.76*f)+3.5*pow(atan(f/7.5),2);
  ISETFIELD_RVALUE(dResult,"cofs" ,b      );
  ISETFIELD_RVALUE(dResult,"cinc" ,m_nFinc);
  ISETFIELD_SVALUE(dResult,"cunit","bark" );
}


INT16 CGEN_PRIVATE CFTTproc::InitFTT(INT16 noreset) {
  if(m_lpFtt == NULL) {
    m_lpFtt = (MLP_FTT_TYPE*)dlp_calloc(1,sizeof(MLP_FTT_TYPE));             // Alloc. convolution core data struct
  }
  if (!m_lpFtt) return NOT_EXEC;                                               // Check memory objects

  m_lpFtt->atf = m_nSrate;
  m_lpFtt->start_freq = m_nStartfreq;
  m_lpFtt->bw_coeff = m_nBandwidth;
  m_lpFtt->finc_coeff = m_nFinc;
  m_lpFtt->sm_coeff = m_nSmCoeff;
  m_lpFtt->norm_coeff = m_nNormCoeff;
  m_lpFtt->log = m_nLog;
  m_lpFtt->max_value = m_nMaxval;
  dlp_strcpy(m_lpFtt->type,m_lpsFttType);

  if (NOK(dlm_ftt_init(m_lpFtt,m_nCoeff,noreset))) {            // Initialize frequency/bandwidth pairs
    dlm_ftt_done(m_lpFtt, FALSE);
    return NOT_EXEC;
  }                                                                      // <<
  InitBuffers();
  return O_K;
}

  /**
   * Initialize internal buffers. Classes overwrites
   * CFBAproc::InitBuffers .
   */
  INT16 CGEN_PROTECTED CFTTproc::InitBuffers()
  {
    IFIELD_RESET(CData,"window" );
    IFIELD_RESET(CData,"real_frame");
    IFIELD_RESET(CData,"imag_frame");
    IFIELD_RESET(CData,"energy" );

    m_idWindow->AddComp("w",T_DOUBLE);
    if (m_nLen > m_nCoeff)
    {   m_idRealFrame->AddNcomps(T_DOUBLE,m_nLen);
        m_idImagFrame->AddNcomps(T_DOUBLE,m_nLen);
    }
    else
    {   m_idRealFrame->AddNcomps(T_DOUBLE,m_nCoeff);
        m_idImagFrame->AddNcomps(T_DOUBLE,m_nCoeff);
    }
    MakeWindow(m_nWlen);

    // Allocate buffers
    m_idRealFrame->Allocate(1);
    m_idImagFrame->Allocate(1);

    return O_K;
  }

/* EOF */
