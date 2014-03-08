/* dLabPro mathematics library
 * - Noise reduction
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
#include <stdlib.h>
#include <math.h>
#include "dlp_kernel.h"
#include "dlp_base.h"
#include "dlp_math.h"

#ifdef __TMS
	#pragma DATA_SECTION(lpNseFlr,".farfar")
	#define dlp_memset(A,B,C)	memset(A,B,C)
	FLOAT32 *lpNse;
#endif

/* Default noise floor: average verbmobil silence */
FLOAT32 lpNseFlr[257] = {
0.161954000, 0.138808100, 0.087097290, 0.053319810, 0.039738120, 0.035971590, 0.032280070, 0.030169350,
0.029207810, 0.026985600, 0.024082020, 0.021439920, 0.019934590, 0.019606530, 0.019306190, 0.018763890,
0.018660730, 0.018868600, 0.018405720, 0.017135220, 0.015977570, 0.015327260, 0.014782830, 0.014311290,
0.013967080, 0.014134650, 0.014619610, 0.014645970, 0.013881060, 0.012943310, 0.012650170, 0.012383460,
0.011801310, 0.011126260, 0.011237070, 0.011645350, 0.011470350, 0.010648070, 0.010012900, 0.010037660,
0.010307300, 0.010560360, 0.010826240, 0.011230990, 0.011548480, 0.011713130, 0.011864640, 0.012121200,
0.012548690, 0.012915800, 0.013209080, 0.013530800, 0.013977890, 0.014286080, 0.014246560, 0.013980540,
0.013897300, 0.014112270, 0.014051640, 0.013618390, 0.013094810, 0.012922760, 0.012787440, 0.012438720,
0.011899820, 0.011465020, 0.011344340, 0.011212420, 0.011068170, 0.010915690, 0.010870440, 0.010694640,
0.010352940, 0.009919771, 0.009729687, 0.009782886, 0.009748961, 0.009555900, 0.009359871, 0.009495894,
0.009617529, 0.009486254, 0.009153674, 0.008988092, 0.009042432, 0.009057779, 0.008952527, 0.008846034,
0.008874426, 0.008925592, 0.008873015, 0.008753043, 0.008863296, 0.009000138, 0.008888244, 0.008669695,
0.008754606, 0.009118090, 0.009327990, 0.009295687, 0.009112708, 0.008932191, 0.008906096, 0.008943764,
0.008920548, 0.008966834, 0.009093383, 0.009123261, 0.009021164, 0.008888982, 0.008943185, 0.009025745,
0.009038061, 0.008975566, 0.008979289, 0.009098872, 0.009060488, 0.008860581, 0.008723239, 0.008680508,
0.008621833, 0.008558586, 0.008539896, 0.008786306, 0.009084516, 0.009073317, 0.008755412, 0.008658201,
0.008982128, 0.008928246, 0.008553685, 0.008325450, 0.008501231, 0.008744743, 0.008741410, 0.008375292,
0.008059437, 0.008209137, 0.008287535, 0.008142893, 0.007994995, 0.008229699, 0.008482670, 0.008511048,
0.008354432, 0.008209155, 0.008233760, 0.008237592, 0.008224317, 0.008125248, 0.008127621, 0.008172144,
0.008096350, 0.007889283, 0.007809284, 0.007898615, 0.007943925, 0.007784070, 0.007563225, 0.007682033,
0.007788749, 0.007604277, 0.007303666, 0.007126560, 0.007102224, 0.007055167, 0.006899626, 0.006820022,
0.006971392, 0.007109957, 0.006974834, 0.006639552, 0.006599301, 0.006734509, 0.006693611, 0.006456316,
0.006271701, 0.006319231, 0.006349160, 0.006239665, 0.006034896, 0.006052918, 0.006181432, 0.006141766,
0.005903714, 0.005789235, 0.005958806, 0.006054213, 0.005944225, 0.005679447, 0.005598520, 0.005717462,
0.005766016, 0.005619034, 0.005593348, 0.005822720, 0.005914424, 0.005720487, 0.005498089, 0.005689153,
0.005928127, 0.005884178, 0.005566215, 0.005440187, 0.005698106, 0.005833098, 0.005656996, 0.005405803,
0.005423282, 0.005462640, 0.005356275, 0.005179093, 0.005086979, 0.005148838, 0.005155287, 0.004988405,
0.004766229, 0.004729543, 0.004680867, 0.004477943, 0.004294712, 0.004412731, 0.004610531, 0.004582874,
0.004301973, 0.003996336, 0.003959208, 0.003969740, 0.003876748, 0.003769067, 0.003960441, 0.004188056,
0.004139195, 0.003841327, 0.003581396, 0.003513379, 0.003497344, 0.003453758, 0.003353729, 0.003357310,
0.003403842, 0.003321660, 0.003081041, 0.002890551, 0.002863681, 0.002776675, 0.002569334, 0.002340431,
0.002332615, 0.002326600, 0.002169296, 0.001898265, 0.001865410, 0.002042405, 0.002048883, 0.001778243,
0.001442791,
};

struct noisecfg {
  INT32    nLen;
  FLOAT32  nPrc;
  FLOAT32 *lpFlr;
  INT32    nNFlr;
} sNseCfg = { 300, 0.3f, lpNseFlr, 257 };

struct noisebuf {
  FLOAT32         *lpFrm;
  FLOAT32          nPow;
  BOOL             bUse;
  struct noisebuf *lpNxt;
  struct noisebuf *lpPrv;
};

struct noise {
  INT32            nDim;
  INT32            nLen;
  struct noisebuf *lpBuf;
  struct noisebuf *lpMin;
  struct noisebuf *lpMax;
  INT32            nNUse;
  INT32            nBufPos;
  FLOAT32         *lpNse;
  BOOL             bUseNxt;
} sNse = {0,0,NULL,NULL,NULL,0,0,NULL,TRUE};

INT16 CGEN_IGNORE dlm_noisesetup(INT32 nLen,FLOAT32 nPrc,FLOAT32 *lpFlr,INT32 nNFlr){
  sNseCfg.nLen=nLen;
  if(nPrc>1.f) nPrc=1.f;
  if(nPrc<0.f) nPrc=0.f;
  sNseCfg.nPrc=nPrc;
  if(!lpFlr) return O_K;
  if(sNseCfg.lpFlr==lpNseFlr) sNseCfg.lpFlr=NULL;
  sNseCfg.lpFlr=realloc(sNseCfg.lpFlr,nNFlr*sizeof(FLOAT32));
  memcpy(sNseCfg.lpFlr,lpFlr,nNFlr*sizeof(FLOAT32));
  sNseCfg.nNFlr=nNFlr;
  return O_K;
}

void CGEN_IGNORE dlm_noise_init(INT32 nDim){
  INT32 nF;
  INT32 nNUse=sNseCfg.nLen*sNseCfg.nPrc;
  if(sNse.nDim==nDim && sNse.nLen==sNseCfg.nLen && sNse.nNUse==nNUse) return;
  sNse.nDim=nDim;
  sNse.nNUse=nNUse;
  for(nF=sNseCfg.nLen;nF<sNse.nLen;nF++) free(sNse.lpBuf[nF].lpFrm);
  sNse.lpBuf=realloc(sNse.lpBuf,sizeof(struct noisebuf)*sNseCfg.nLen);
  for(nF=sNse.nLen;nF<sNseCfg.nLen;nF++) sNse.lpBuf[nF].lpFrm=NULL;
  sNse.nLen=sNseCfg.nLen;
  for(nF=0;nF<sNse.nLen;nF++){
    sNse.lpBuf[nF].lpFrm=realloc(sNse.lpBuf[nF].lpFrm,sizeof(FLOAT32)*sNse.nDim);
    dlp_memset(sNse.lpBuf[nF].lpFrm,0,sizeof(FLOAT32)*sNse.nDim);
    sNse.lpBuf[nF].nPow=0.f;
    sNse.lpBuf[nF].bUse=nF<sNse.nNUse;
    sNse.lpBuf[nF].lpNxt = nF<sNse.nLen-1 ? sNse.lpBuf+nF+1 : NULL;
    sNse.lpBuf[nF].lpPrv = nF ? sNse.lpBuf+nF-1 : NULL;
  }
  sNse.lpMin=sNse.lpBuf+0;
  sNse.lpMax=sNse.lpBuf+sNse.nLen-1;
  sNse.nBufPos=0;
#ifdef __TMS
  lpNse=
#endif
  sNse.lpNse=realloc(sNse.lpNse,sizeof(FLOAT32)*sNse.nDim);
  dlp_memset(sNse.lpNse,0,sizeof(FLOAT32)*sNse.nDim);
}

void CGEN_IGNORE dlm_noise_insert(void* lpFrm,BOOL bDouble){
  FLOAT32 nPow = 0.;
  INT32   nI;
  BOOL    bUseOld;
  struct noisebuf *lpIns;
  /* Calculate Energy */
  for(nI=0;nI<sNse.nDim;nI++)
    if(bDouble) nPow+=((FLOAT64*)lpFrm)[nI]*((FLOAT64*)lpFrm)[nI];
    else        nPow+=((FLOAT32*)lpFrm)[nI]*((FLOAT32*)lpFrm)[nI];
  nPow=sqrt(nPow/(FLOAT32)sNse.nDim);
  /* Remove old element from noise buffer */
  if(sNse.lpBuf[sNse.nBufPos].bUse)
    for(nI=0;nI<sNse.nDim;nI++) sNse.lpNse[nI]-=sNse.lpBuf[sNse.nBufPos].lpFrm[nI];
  if(sNse.lpBuf[sNse.nBufPos].lpPrv) sNse.lpBuf[sNse.nBufPos].lpPrv->lpNxt=sNse.lpBuf[sNse.nBufPos].lpNxt;
  else sNse.lpMin=sNse.lpBuf[sNse.nBufPos].lpNxt;
  if(sNse.lpBuf[sNse.nBufPos].lpNxt) sNse.lpBuf[sNse.nBufPos].lpNxt->lpPrv=sNse.lpBuf[sNse.nBufPos].lpPrv;
  else sNse.lpMax=sNse.lpBuf[sNse.nBufPos].lpPrv;
  /* Find insert position */
  for(lpIns=sNse.lpMin;lpIns && lpIns->nPow<nPow;) lpIns=lpIns->lpNxt;
  /* Insert new element in noise buffer */
  if(lpIns){
    sNse.lpBuf[sNse.nBufPos].lpNxt=lpIns;
    sNse.lpBuf[sNse.nBufPos].lpPrv=lpIns->lpPrv;
    if(lpIns->lpPrv) lpIns->lpPrv->lpNxt=sNse.lpBuf+sNse.nBufPos;
    else sNse.lpMin=sNse.lpBuf+sNse.nBufPos;
    lpIns->lpPrv=sNse.lpBuf+sNse.nBufPos;
  }else{
    sNse.lpBuf[sNse.nBufPos].lpNxt=NULL;
    sNse.lpBuf[sNse.nBufPos].lpPrv=sNse.lpMax;
    sNse.lpMax->lpNxt=sNse.lpBuf+sNse.nBufPos;
    sNse.lpMax=sNse.lpBuf+sNse.nBufPos;
  }
  sNse.lpBuf[sNse.nBufPos].nPow=nPow;
  for(nI=0;nI<sNse.nDim;nI++)
    if(bDouble) sNse.lpBuf[sNse.nBufPos].lpFrm[nI]=((FLOAT64*)lpFrm)[nI]/(FLOAT32)sNse.nNUse;
    else        sNse.lpBuf[sNse.nBufPos].lpFrm[nI]=((FLOAT32*)lpFrm)[nI]/(FLOAT32)sNse.nNUse;
  /* Update used flag and noise buffer */
  bUseOld=sNse.lpBuf[sNse.nBufPos].bUse;
  sNse.lpBuf[sNse.nBufPos].bUse=FALSE;
  if(!sNse.lpBuf[sNse.nBufPos].lpPrv || sNse.lpBuf[sNse.nBufPos].lpPrv->bUse){
    if(sNse.lpBuf[sNse.nBufPos].lpNxt && sNse.lpBuf[sNse.nBufPos].lpNxt->bUse)
      sNse.lpBuf[sNse.nBufPos].bUse=TRUE;
    else sNse.lpBuf[sNse.nBufPos].bUse=bUseOld;
  }
  if(sNse.lpBuf[sNse.nBufPos].bUse) for(nI=0;nI<sNse.nDim;nI++) sNse.lpNse[nI]+=sNse.lpBuf[sNse.nBufPos].lpFrm[nI];
  /* Check for change in use flag */
  if(sNse.lpBuf[sNse.nBufPos].bUse && !bUseOld){ /* add one */
    for(lpIns=sNse.lpMin;lpIns && lpIns->bUse;) lpIns=lpIns->lpNxt;
    if(!lpIns) lpIns=sNse.lpMax; else if(lpIns->lpPrv) lpIns=lpIns->lpPrv;
    if(lpIns && lpIns->bUse){
      for(nI=0;nI<sNse.nDim;nI++) sNse.lpNse[nI]-=lpIns->lpFrm[nI];
      lpIns->bUse=0;
    }
  }
  if(!sNse.lpBuf[sNse.nBufPos].bUse && bUseOld){ /* remove one */
    for(lpIns=sNse.lpMin;lpIns && lpIns->bUse;) lpIns=lpIns->lpNxt;
    if(lpIns && !lpIns->bUse){
      for(nI=0;nI<sNse.nDim;nI++) sNse.lpNse[nI]+=lpIns->lpFrm[nI];
      lpIns->bUse=1;
    }
  }
  /* Update noise buffer pos */
  if(++sNse.nBufPos==sNse.nLen) sNse.nBufPos=0;
}

void CGEN_IGNORE dlm_noise_reduce(void* lpFrm,INT32 nDim,BOOL bDouble){
  INT32 nI;
  for(nI=0;nI<sNse.nDim;nI++){
    FLOAT32 nFlr = nI<sNseCfg.nNFlr ? sNseCfg.lpFlr[nI] : 0.f;
    if(bDouble){ if((((FLOAT64*)lpFrm)[nI]-=sNse.lpNse[nI])<nFlr) ((FLOAT64*)lpFrm)[nI]=nFlr; }
    else       { if((((FLOAT32*)lpFrm)[nI]-=sNse.lpNse[nI])<nFlr) ((FLOAT32*)lpFrm)[nI]=nFlr; }
  }
}

INT16 CGEN_IGNORE dlm_noisechk(INT32 nDim){
  int i,n;
  struct noisebuf *lpIns;
  if(sNse.nDim!=nDim) abort();
  if(sNse.nLen!=sNseCfg.nLen) abort();
  if(sNse.nNUse!=sNseCfg.nLen*sNseCfg.nPrc) abort();
  if(sNse.nNUse<sNse.nLen) abort();
  if(sNseCfg.nNFlr && !sNseCfg.lpFlr) abort();
  if(!sNse.lpBuf) abort();
  if(!sNse.lpNse) abort();
  for(n=i=0;i<sNse.nLen;i++) if(sNse.lpBuf[i].bUse) n++;
  if(n!=sNse.nNUse) abort();
  for(n=i=0,lpIns=sNse.lpMin;lpIns;lpIns=lpIns->lpNxt){
    if(lpIns!=sNse.lpMin && !lpIns->lpPrv) abort();
    if(lpIns!=sNse.lpMin &&  lpIns->lpPrv->lpNxt!=lpIns) abort();
    if(lpIns==sNse.lpMin &&  lpIns->lpPrv) abort();
    if(lpIns!=sNse.lpMax && !lpIns->lpNxt) abort();
    if(lpIns!=sNse.lpMax &&  lpIns->lpNxt->lpPrv!=lpIns) abort();
    if(lpIns==sNse.lpMax &&  lpIns->lpNxt) abort();
    if(lpIns->lpNxt && !lpIns->bUse && lpIns->lpNxt->bUse) abort();
    if(lpIns->lpNxt && lpIns->nPow>lpIns->lpNxt->nPow) abort();
    n++;
    if(lpIns->bUse) i++;
  }
  if(n!=sNse.nLen) abort();
  if(i!=sNse.nNUse) abort();
  return O_K;
}

INT16 CGEN_IGNORE dlm_noiserdc(void* lpFrm, INT32 nDim, BOOL bDouble){
  dlm_noise_init(nDim);
  if(sNse.bUseNxt) dlm_noise_insert(lpFrm,bDouble);
  dlm_noise_reduce(lpFrm,nDim,bDouble);
#ifdef __DEBUG
  dlm_noisechk(nDim);
#endif
  return O_K;
}

INT16 CGEN_IGNORE dlm_noisefrc(BOOL bUseNxt){
  sNse.bUseNxt=bUseNxt;
  return O_K;
}

INT16 CGEN_PUBLIC dlm_mcep_denoise(FLOAT64* C, FLOAT64* R, INT32 nR, INT32 nC, INT32 nF, FLOAT64 nE) {
  INT32    iC      = 0;
  INT32    iR      = 0;
  INT32    iH      = 0;
  FLOAT64  mean    = 0.0;
  FLOAT64* pC      = NULL;
  FLOAT64* pR      = NULL;
  FLOAT64* hist    = NULL;

  hist = (FLOAT64*)dlp_calloc(nF, sizeof(FLOAT64));
  if(!hist) return ERR_MEM;

  for(iC = 1; iC < nC; iC++) {
    pC = C + iC;
    pR = R + iC;
    mean = 0.0;
    iH = 0;
    dlp_memset(hist, 0L, nF*sizeof(FLOAT64));
    for(iR = 0; iR < nR; iR ++) {
      if((*(C+iR*nC) < nE) && ((*(C+iR*nC+1) < 0.0))) {
        mean = mean * iH - *(hist + nF-1);
        dlp_memmove(hist+1, hist, (nF-1)*sizeof(FLOAT64));
        *hist = *(pC + iR*nC);
        iH = (iH < nF) ? iH+1 : iH;
        mean = (mean + *hist) / iH;
      }
      *(pR + iR*nC) = *(pC + iR*nC) - mean;
    }
  }

  dlp_free(hist);

  return O_K;
}

INT16 CGEN_PUBLIC dlm_spec_denoise(FLOAT64* S, FLOAT64* R, INT32 nR, INT32 nC, INT32 nF, FLOAT64 nW) {
  INT32    iS      = 0;
  INT32    iR      = 0;
  INT32    iH      = 0;
  FLOAT64  mean    = 0.0;
  FLOAT64* pS      = NULL;
  FLOAT64* pR      = NULL;
  FLOAT64* hist    = NULL;

  if(!S || !R) return NOT_EXEC;

  hist = (FLOAT64*)dlp_calloc(nF, sizeof(FLOAT64));
  if(!hist) return ERR_MEM;

  for(iS = 1; iS < nC; iS++) {
    pS = S + iS;
    pR = R + iS;
    mean = 0.0;
    iH = 0;
    dlp_memset(hist, 0L, nF*sizeof(FLOAT64));
    for(iR = 0; iR < nR; iR ++) {
      mean = mean * iH - *(hist + nF-1);
      dlp_memmove(hist+1, hist, (nF-1)*sizeof(FLOAT64));
      *hist = *(pS + iR*nC);
      iH = (iH < nF) ? iH+1 : iH;
      mean = (mean + *hist) / iH;
      *(pR + iR*nC) = MAX(0,*(pS + iR*nC) - nW * mean);
    }
  }

  dlp_free(hist);

  return O_K;
}

