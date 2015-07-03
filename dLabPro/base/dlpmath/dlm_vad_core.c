/* dLabPro mathematics library
 * - Voice activity detection core functions
 *
 * AUTHOR : Frank Duckhorn
 * PACKAGE: dLabPro/base
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
#if VAD_FTYPE_CODE == T_FLOAT
  #define VAD_FTYPE          FLOAT32
  #define VAD_SINGLE_SIGTHR  dlm_vad_single_sigengF
  #define VAD_SINGLE_PFATHR  dlm_vad_single_pfaengF
#elif VAD_FTYPE_CODE == T_DOUBLE
  #define VAD_FTYPE          FLOAT64
  #define VAD_SINGLE_SIGTHR dlm_vad_single_sigengD
  #define VAD_SINGLE_PFATHR  dlm_vad_single_pfaengD
#else
  #error VAD_FTYPE_CODE must be T_FLOAT or T_DOUBLE
#endif

BOOL CGEN_IGNORE VAD_SINGLE_SIGTHR(VAD_FTYPE *lpSig,INT32 nLen,VAD_FTYPE nThr)
{
  INT32 nI;
  VAD_FTYPE nPow=0.;
  for(nI=0;nI<nLen;nI++) nPow+=lpSig[nI]*lpSig[nI];
  return nPow>nThr;
}

BOOL CGEN_IGNORE VAD_SINGLE_PFATHR(VAD_FTYPE *lpFrame,INT32 nDim,VAD_FTYPE nThr)
{
  INT32 nI;
  VAD_FTYPE nPow=0.;
  for(nI=0;nI<nDim;nI++) nPow+=exp(lpFrame[nI])*exp(lpFrame[nI]);
  return nPow/nDim>nThr;
}

#undef VAD_SINGLE_SIGTHR
#undef VAD_SINGLE_PFATHR
#undef VAD_FTYPE
