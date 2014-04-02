/* dLabPro mathematics library
 * - Voice activity detection functions
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
#include <stdio.h>
#include <string.h>

#include "dlp_config.h"
#include "dlp_kernel.h"
#include "dlp_base.h"
#include "dlp_math.h"

#define VAD_FTYPE_CODE  T_FLOAT
#include "dlm_vad_core.c"
#undef VAD_FTYPE_CODE
#define VAD_FTYPE_CODE  T_DOUBLE
#include "dlm_vad_core.c"
#undef VAD_FTYPE_CODE

void CGEN_IGNORE dlm_vad_initparam(struct dlm_vad_param *lpPVad)
{
  lpPVad->nPre   = 3;
  lpPVad->nPost  = 4;
  lpPVad->nMinSp = 4;
  lpPVad->nMinSi = 10;
  lpPVad->nMaxSp = 500;
}

void CGEN_IGNORE dlm_vad_initstate(struct dlm_vad_state *lpSVad,struct dlm_vad_param *lpPVad,INT32 nMinDelay)
{
  lpSVad->lpPVad   = *lpPVad;
  if(lpSVad->lpPVad.nPost>lpSVad->lpPVad.nMinSi) lpSVad->lpPVad.nMinSi=lpSVad->lpPVad.nPost;
  lpSVad->nState   = VAD_SI,
  lpSVad->nInState = 1,
  lpSVad->nPre     = 0,
  lpSVad->bVadSfa  = FALSE,
  lpSVad->nChange  = -1,
  lpSVad->nDelay   = MAX(nMinDelay, MAX(lpPVad->nPre+lpPVad->nMinSp, lpPVad->nMinSi));
}

BOOL CGEN_IGNORE dlm_vad_process(BOOL bVadPfa,struct dlm_vad_state *lpSVad)
{
  switch(lpSVad->nState){
    case VAD_SI:
      if(!bVadPfa){ if(lpSVad->nPre<lpSVad->lpPVad.nPre) lpSVad->nPre++; }else{
        lpSVad->nState=VAD_SP_PRE;
        lpSVad->nInState=1;
      }
    break;
    case VAD_SP_PRE:
      if(!bVadPfa){
        lpSVad->nState=VAD_SI;
      }else{
        if(lpSVad->nPre<lpSVad->lpPVad.nPre) lpSVad->nPre++;
        if(lpSVad->nInState<lpSVad->lpPVad.nMinSp) lpSVad->nInState++; else {
          lpSVad->nState=VAD_SP;
          lpSVad->nChange=lpSVad->nDelay-lpSVad->nPre-lpSVad->nInState+1;
        }
      }
    break;
    case VAD_SP:
      if(!bVadPfa){
        lpSVad->nState=VAD_SP_POST;
        lpSVad->nInState=1;
      }
    break;
    case VAD_SP_POST:
      if(!bVadPfa){
        if(lpSVad->nInState<lpSVad->lpPVad.nMinSi) lpSVad->nInState++; else{
          lpSVad->nState=VAD_SI;
          lpSVad->nPre=1;
          lpSVad->nChange=-lpSVad->nDelay+(lpSVad->lpPVad.nMinSi-lpSVad->lpPVad.nPost)-1;
        }
      }else lpSVad->nState=VAD_SP;
    break;
  }
  if(!lpSVad->nChange) return lpSVad->bVadSfa;
  if(lpSVad->nChange<0){
    if(!++lpSVad->nChange) lpSVad->bVadSfa=FALSE;
  }else{
    if(!--lpSVad->nChange) lpSVad->bVadSfa=TRUE;
  }
  return lpSVad->bVadSfa;
}

