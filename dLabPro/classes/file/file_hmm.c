/* dLabPro class CDlpFile (file)
 * - Import/export functions of hmms
 *
 * AUTHOR : Frank Duckhorn
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
#include "dlp_file.h"
#ifdef __cplusplus
#include "dlp_hmm.h"
#endif

/**
 * Export hmm in HTK format hmmdefs.
 *
 * @param lpsFilename Name of file to export
 * @param iSrc        Pointer to instance to export
 * @param lpsFiletype Type of file to export
 * @return <code>O_K</code> if successful, a (negative) error code otherwise
 */
INT16 CGEN_PROTECTED CDlpFile_Hmm_ExportHtk
(
  CDlpFile*   _this,
  const char* lpsFilename,
  CDlpObject* iSrc,
  const char* lpsFiletype
)
{
#ifndef __cplusplus
#else
  CHmm *iHmm = (CHmm*)iSrc;
  FILE *fDst = NULL;
  INT32  nXU = UD_XXU(iHmm);
  INT32  nU;
  INT32  nDim = iHmm->m_iGm->GetDim();
  if(!(fDst=fopen(lpsFilename,"w"))) return NOT_EXEC;
  fprintf(fDst,"~o\n");
  fprintf(fDst,"<STREAMINFO> 1 24\n");
  fprintf(fDst,"<VecSize> %li<NULLD><MFCC><%s>\n",(long)nDim,iHmm->m_iGm->m_idIcov?"FULLC":"DIAGC");
  for(nU=0;nU<nXU;nU++){
    INT32 nXS=UD_XS(iHmm,nU);
    INT32 nS;
    FST_TID_TYPE *lpTI = CFst_STI_Init(iHmm,nU,FSTI_SORTTER);
    fprintf(fDst,"~h \"%s\"\n",iHmm->ud->Sfetch(nU,0));
    fprintf(fDst,"<BeginHMM>\n");
    fprintf(fDst,"<NumStates> %li\n",(long)nXS);
    for(nS=0;nS<nXS;nS++){
      INT32 nM=-1;
      BYTE *lpT=NULL;
      while((lpT=CFst_STI_TtoS(lpTI,nS,lpT))!=NULL){
        INT32 nI=*CFst_STI_TTis(lpTI,lpT);
        if(nI<0) continue;
        if(nM<0) nM=nI;
        else if(nM!=nI) return IERROR(_this,FIL_INVALARG,"Only one Gaussian per state supported => use Mixtures",0,0);
      }
      if(nM<0) continue;
      if(!iHmm->m_iGm->m_iMmap)
        return IERROR(_this,FIL_INVALARG,"Export of HMM without Mixtures not implemented",0,0);
      CVmap *iMap = iHmm->m_iGm->m_iMmap;
      CData *idTmx = iMap->m_idTmx;
      INT32 nXG=0,nG,nK;
      for(nG=0;nG<idTmx->GetNComps();nG++) if(idTmx->Dfetch(nM,nG)!=iMap->m_nZero) nXG++;
      fprintf(fDst,"<State> %li <NumMixes> %li\n",(long)(nS+1),(long)nXG);
      for(nK=nG=0;nG<idTmx->GetNComps();nG++) if(idTmx->Dfetch(nM,nG)!=iMap->m_nZero){
        INT32 nI,nJ,nC=0;
        fprintf(fDst,"<Mixture> %li %g\n",(long)(++nK),exp(-idTmx->Dfetch(nM,nG)));
        fprintf(fDst,"<Mean> %li\n",(long)nDim);
        for(nI=0;nI<nDim;nI++) fprintf(fDst," %g",iHmm->m_iGm->m_idMean->Dfetch(nG,nI));
        fprintf(fDst,"\n");
        fprintf(fDst,"<%s> %li\n",iHmm->m_iGm->m_idIcov?"InvCovar":"Variance",(long)nDim);
        for(nI=0;nI<nDim;nI++){
          FLOAT64 nVar=iHmm->m_iGm->m_idIvar->Dfetch(nG,nI);
          if(!iHmm->m_iGm->m_idIcov) nVar=1./nVar;
          fprintf(fDst," %g",nVar);
          if(iHmm->m_iGm->m_idIcov){
            for(nJ=0;nJ<nDim-nI-1;nJ++)
              fprintf(fDst," %g",iHmm->m_iGm->m_idIcov->Dfetch(nG,nC++));
            fprintf(fDst,"\n");
          }
        }
        if(!iHmm->m_iGm->m_idIcov) fprintf(fDst,"\n");
      }
    }
    FLOAT64 *lpTransP=(FLOAT64*)dlp_calloc(nXS*nXS,sizeof(FLOAT64));
    for(nS=0;nS<nXS;nS++){
      BYTE *lpT=NULL;
      while((lpT=CFst_STI_TtoS(lpTI,nS,lpT))!=NULL)
        lpTransP[nS*nXS+*CFst_STI_TIni(lpTI,lpT)]=exp(-*CFst_STI_TW(lpTI,lpT));
    }
    CFst_STI_Done(lpTI);
    fprintf(fDst,"<TransP> %li\n",(long)nXS);
    for(nS=0;nS<nXS;nS++){
      INT32 nSd;
      for(nSd=0;nSd<nXS;nSd++) fprintf(fDst," %g",lpTransP[nSd*nXS+nS]);
      fprintf(fDst,"\n");
    }
    dlp_free(lpTransP);
    fprintf(fDst,"<EndHMM>\n");
  }
  fclose(fDst);
#endif
  return O_K;
}

/* EOF */
