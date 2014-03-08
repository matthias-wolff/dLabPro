// dLabPro class CVADproc (VADproc)
// - Frame based functions
//
// AUTHOR : Frank Duckhorn
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

#include "dlp_vadproc.h"

INT16 CGEN_PUBLIC CVADproc::InitVAD()
{
  struct dlm_vad_param lpVadParam;
  if(m_lpVadState) dlp_free(m_lpVadState);

  lpVadParam.nPre   = m_nPreSp;
  lpVadParam.nPost  = m_nPostSp;
  lpVadParam.nMinSp = m_nMinSp;
  lpVadParam.nMinSi = m_nMinSi;
  lpVadParam.nMaxSp = m_nMaxSp;

  dlm_vad_initparam(&lpVadParam);
  m_lpVadState=dlp_malloc(sizeof(struct dlm_vad_state));
  dlm_vad_initstate((struct dlm_vad_state *)m_lpVadState,&lpVadParam,0);
  
  return O_K;
}

BOOL CGEN_PUBLIC CVADproc::VadPrimary(CData *idFrames,INT32 nR)
{
  INT32    nDim = 0;
  INT32    nC;
  FLOAT64* lpFrame = NULL;
  BOOL    bFreeFrame = FALSE;
  BOOL    bVad = FALSE;

  if(!dlp_strcmp("pow",m_lpsVadType)){
    for(nC=0;nC<CData_GetNComps(idFrames);nC++){
      switch(CData_GetCompType(idFrames,nC)){
      case T_FLOAT: bFreeFrame = TRUE;
      case T_DOUBLE:
        if(nDim!=nC) bFreeFrame = TRUE;
        nDim++;
      break;
      }
    }

    if(bFreeFrame){
      lpFrame = (FLOAT64 *)dlp_malloc(sizeof(FLOAT64)*nDim);
      for(nC=0;nC<CData_GetNComps(idFrames);nC++){
        switch(CData_GetCompType(idFrames,nC)){
        case T_FLOAT:
        case T_DOUBLE:
          lpFrame[nC] = CData_Dfetch(idFrames,nR,nC);
        break;
        }
      }
    }else lpFrame = (FLOAT64 *)CData_XAddr(idFrames,nR,0);
  }
  
  if(!dlp_strcmp("pow",m_lpsVadType)){
    bVad = dlm_vad_single_pfaengD(lpFrame,nDim,1000000.);
  }else if(!dlp_strcmp("gmm",m_lpsVadType)){
    FLOAT64 nMax=0.;
    INT32   nCMax=-1;
    for(nC=0;nC<CData_GetNComps(idFrames);nC++) if(nCMax<0 || CData_Dfetch(idFrames,nR,nC)<nMax){ nMax=CData_Dfetch(idFrames,nR,nC); nCMax=nC; }
    bVad = (BOOL)CData_Dfetch(m_idGmmLab,0,nCMax);
/*    FLOAT64 nSumV=0.;
    FLOAT64 nSumU=0.;
    FLOAT64 nVal;
    for(nC=0;nC<CData_GetNComps(idFrames);nC++) if(CData_Dfetch(m_idGmmLab,0,nC)) nSumV+=exp(CData_Dfetch(idFrames,nR,nC)); else nSumU+=exp(CData_Dfetch(idFrames,nR,nC));
    nVal=nSumV/(nSumV+nSumU);
    bVad = nVal <0.96;*/
  }else if(!dlp_strcmp("exists",m_lpsVadType)){
    bVad = (BOOL)CData_Dfetch(idFrames,nR,0);
  }else if(!dlp_strcmp("none",m_lpsVadType)){
    bVad = TRUE;
  }else IERROR(this,VAD_UNKOWNMETH,m_lpsVadType,0,0);

  if(bFreeFrame) dlp_free(lpFrame);

  return bVad;
}

INT16 CGEN_PUBLIC CVADproc::Vad(CData *idFrames)
{
  INT32 nXF  = CData_GetNRecs(idFrames);
  INT32 nF;
  struct dlm_vad_state *lpVadState = (struct dlm_vad_state *)m_lpVadState;
  CData *idLab;
  CData *idNld = NULL;

  ICREATEEX(data,idLab,"idLab",NULL);
  CData_Array(idLab,T_SHORT,1,nXF);
  CData_SetCname(idLab,0,"VAD");

  if(!dlp_strcmp("gmm",m_lpsVadType)){
    ICREATEEX(data,idNld,"idNld",NULL);
    ISETOPTION(m_iGmm,"/neglog");
    CGmm_Density(m_iGmm,idFrames,NULL,idNld);
  }

  for(nF=0;nF<nXF+lpVadState->nDelay;nF++){
    BOOL bVad = FALSE;
    if(nF<nXF) bVad = VadPrimary(idNld ? idNld : idFrames,nF);
/*    printf("pF%4i: V:%i =>",nF,bVad); */
    bVad = dlm_vad_process(bVad,lpVadState);
/*    printf(" VS:%i ViS:%2i VP:%2i VC:%2i => sF%4i V:%i\n",lpVadState->nState,lpVadState->nInState,lpVadState->nPre,lpVadState->nChange,nF-lpVadState->nDelay,bVad); */
    if(nF-lpVadState->nDelay>=0) CData_Dstore(idLab,bVad,nF-lpVadState->nDelay,0);
  }

  if(idNld) IDESTROY(idNld);
  CData_Join(idFrames,idLab);
  IDESTROY(idLab);

  return O_K;
}

BOOL CGEN_PUBLIC CVADproc::VadOne(CData *idFrame,INT32 nR)
{
  struct dlm_vad_state *lpVadState = (struct dlm_vad_state *)m_lpVadState;
  BOOL bVad;
  CData *idNld = NULL;

  if(!dlp_strcmp("gmm",m_lpsVadType)){
    ICREATEEX(data,idNld,"idNld",NULL);
    ISETOPTION(m_iGmm,"/neglog");
    CGmm_Density(m_iGmm,idFrame,NULL,idNld);
  }

  bVad = VadPrimary(idNld ? idNld : idFrame,nR);
  bVad = dlm_vad_process(bVad,lpVadState);

  if(idNld) IDESTROY(idNld);
  return bVad;
}

INT32 CGEN_PUBLIC CVADproc::GetDelay()
{
  return ((struct dlm_vad_state *)m_lpVadState)->nDelay;
}

// EOF
