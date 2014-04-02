// dLabPro class CFBAproc (FBAproc)
// - Frame based synthesis functions
//
// AUTHOR : Guntram Strecha
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

#include "dlp_fbaproc.h"

/////////////////////////////////////////////////////////////////////////////////////
//
// ModEx - generates excitation from pitch file
//
// CData *gain       -> gains of excitation per frame
// CData *idPm       -> both components must be type long
// CData *idExcite   -> type FBA_FLOAT
//
INT16 CGEN_PUBLIC CFBAproc::ModEx(CData *idPm, CData *idExcite) {
  // Error handling
  if (idPm == NULL)                return IERROR(this,ERR_NULLINST,0,0,0);
  if (idPm -> IsEmpty() == TRUE)   return IERROR(idPm,DATA_EMPTY,idPm->m_lpInstanceName,0,0);
  if (idExcite == NULL)            return IERROR(this,ERR_NULLINST,idExcite->m_lpInstanceName,0,0);

  DLPASSERT(idPm->GetNComps()>1);

  FLOAT64* exc = NULL;
  INT32    n_exc = 0;

  switch(m_lpsExcType[0]) {
    case 'P':
      if(dlm_pm2exc((INT16*)idPm->XAddr(0,0),(INT32)idPm->GetNRecs(),&exc,&n_exc,m_nSrate, (BOOL)TRUE,DLM_PITCH_PULSE    ) != O_K) return NOT_EXEC;
      break;
    case 'G':
      if(dlm_pm2exc((INT16*)idPm->XAddr(0,0),(INT32)idPm->GetNRecs(),&exc,&n_exc,m_nSrate, (BOOL)TRUE,DLM_PITCH_GLOTT    ) != O_K) return NOT_EXEC;
      break;
    case 'R':
      if(dlm_pm2exc((INT16*)idPm->XAddr(0,0),(INT32)idPm->GetNRecs(),&exc,&n_exc,m_nSrate, (BOOL)TRUE,DLM_PITCH_RANDPHASE) != O_K) return NOT_EXEC;
      break;
    case 'V':
      if(dlm_pm2exc((INT16*)idPm->XAddr(0,0),(INT32)idPm->GetNRecs(),&exc,&n_exc,m_nSrate, (BOOL)TRUE,DLM_PITCH_VOICED   ) != O_K) return NOT_EXEC;
      break;
    case 'U':
      if(dlm_pm2exc((INT16*)idPm->XAddr(0,0),(INT32)idPm->GetNRecs(),&exc,&n_exc,m_nSrate, (BOOL)TRUE,DLM_PITCH_UNVOICED ) != O_K) return NOT_EXEC;
      break;
    case 'C':
      if(m_idExc == NULL) return IERROR(this,ERR_BADPTR,NULL,"of CFBAproc->m_lpExc","(FBAproc.exc)");
      for(INT32 i_pm=0; i_pm < idPm->GetNRecs(); i_pm++) n_exc += (INT16)idPm->Dfetch(i_pm,0);
      if(CData_GetNRecs(m_idExc) < n_exc) return IERROR(this,FBA_BADEXCLEN,0,0,0);
      exc = (FLOAT64*)dlp_malloc(n_exc*sizeof(FLOAT64));
      dlp_memmove(exc,m_idExc->XAddr(0,0),n_exc*sizeof(FLOAT64));
      break;
    default:
      return IERROR(this,FBA_BADARG,m_lpsExcType,"exc_type","P, G, R, V, U or C");
  }

  idExcite->Reset(TRUE);
  idExcite->AddComp("exc", T_DOUBLE);
  idExcite->Allocate(n_exc);
  dlp_memmove(idExcite->XAddr(0,0), exc, n_exc*sizeof(FLOAT64));
  dlp_free(exc);

  return O_K;
}

/*
 * Resample voiced parts of pitch to match a given mean fundamential frequency.
 * The length of voiced segments is preserved to avoid loss of synchronization
 * between pitch and corresponding signal
 *
 * @param   idPitch     Source data instance containing original pitch
 * @param   idNewPitch  Target data instance containing new pitch
 * @param   nFFreq      Target fundamential frequency (mean over voiced parts)
 */
INT16 CGEN_PUBLIC CFBAproc::ResamplePitch(CData *idPitch, CData *idNewPitch, INT32 nFFreq)
{
  INT16  bVoiced    = FALSE;
  INT32   i          = 0;
  INT32   k          = 0;
  INT32   nCount     = 0;
  INT32   nStartL    = 0;
  FLOAT32  nTargetPeriodLength = (FLOAT32)m_nSrate/(FLOAT32)nFFreq;
  FLOAT32  nMeanPeriodLengthL  = 0.0;
  FLOAT32  nMeanPeriodLength   = 0.0;
  CData* idVoiced   = NULL;
  CData* idAux      = NULL;

  // Validation
  if(idPitch == NULL)       return IERROR(this,ERR_NULLINST,0,0,0);
  if(idPitch->IsEmpty())    return IERROR(idPitch,DATA_EMPTY,idPitch->m_lpInstanceName,0,0);
  if(nFFreq<50||nFFreq>500) return IERROR(this,FBA_BADARG,nFFreq,"nFFreq","a value between 50 and 500");
  if
  (
    idPitch->GetNComps()!=2                            ||
    !dlp_is_numeric_type_code(idPitch->GetCompType(0)) ||
    !dlp_is_numeric_type_code(idPitch->GetCompType(1))
  )
  {
    return IERROR(this,FBA_BADARG,idPitch,"idPitch","contains invalid data.");
  }

  // Initialization
  CREATEVIRTUAL(CData,idPitch,idNewPitch);
  ICREATEEX(CData,idVoiced,"~idVoiced",NULL);
  ICREATEEX(CData,idAux   ,"~idAux"   ,NULL);
  idNewPitch->Reset();
  idVoiced->AddComp("start",T_INT);
  idVoiced->AddComp("count",T_INT);
  idVoiced->AddComp("mplen",T_FLOAT);
  idVoiced->Alloc(10);

  // Determine start and length of voiced parts and mean of periods in samples
  for(i=0; i<idPitch->GetNRecs(); i++)
  {
    if(idPitch->Dfetch(i,1)>0)
    {
      if(bVoiced==FALSE)    // Start of new voiced segment
      {
        bVoiced=TRUE;
        nStartL=i;
        nMeanPeriodLengthL=0;
        if(idVoiced->GetNRecs()==idVoiced->GetMaxRecs())
          idVoiced->Realloc(idVoiced->GetNRecs()+10);
        idVoiced->IncNRecs(1);
        idVoiced->Dstore(i,idVoiced->GetNRecs()-1,0);
      }
      nMeanPeriodLength+=(INT32)idPitch->Dfetch(i,0);
      nMeanPeriodLengthL+=(INT32)idPitch->Dfetch(i,0);
      nCount++;
    }
    else if(bVoiced==TRUE)  // End of voiced segment
    {
      bVoiced=FALSE;
      nMeanPeriodLengthL=nMeanPeriodLengthL/(FLOAT32)(i-nStartL);
      idVoiced->Dstore(i-nStartL,idVoiced->GetNRecs()-1,1);
      idVoiced->Dstore(nMeanPeriodLengthL,idVoiced->GetNRecs()-1,2);
    }
  }
  nMeanPeriodLength=nMeanPeriodLength/(FLOAT32)nCount;

  IFCHECK idVoiced->Print();
  IFCHECK printf("\n Input mean period length in voiced parts: %f",nMeanPeriodLength);
  IFCHECK printf("\n Target mean period length: %f",nTargetPeriodLength);

  // Resample
  for(i=0,nStartL=0; i<idVoiced->GetNRecs(); i++)
  {
    INT32 j       = 0;
    INT32 nSum    = 0;
    INT32 nSumNew = 0;
    INT32 nDiff   = 0;

    // Copy unvoiced
    idAux->SelectRecs(idPitch,nStartL,(INT32)idVoiced->Dfetch(i,0)-nStartL);
    idNewPitch->Cat(idAux);
    nStartL=(INT32)idVoiced->Dfetch(i,0)+(INT32)idVoiced->Dfetch(i,1);

    // Resample voiced
    idAux->SelectRecs(idPitch,(INT32)idVoiced->Dfetch(i,0),(INT32)idVoiced->Dfetch(i,1));
    for(j=0,nSum=0;j<idAux->GetNRecs();j++) nSum+=(INT32)idAux->Dfetch(j,0);  // Target sum
    idAux->Resample(idAux,nMeanPeriodLength/nTargetPeriodLength);
    idAux->Tconvert(idAux,T_FLOAT);
    idAux->Scalop(idAux,CMPLX(nTargetPeriodLength/nMeanPeriodLength),"mult");
    idAux->Tconvert(idAux,T_INT);
    //DLPASSERT(FALSE);
    do
    {
      nSumNew=0;
      for(j=0,nSumNew=0;j<idAux->GetNRecs();j++)
        nSumNew+=(INT32)idAux->Dfetch(j,0); // New sum
      nDiff=nSumNew-nSum;                  // Distribute difference
      IFCHECK printf("\n Distribute difference d=%ld",(long)nDiff);
      for(j=0;j<idAux->GetNRecs()&&j<abs(nDiff);j++)
      {
        INT32 nValue = (INT32)idAux->Dfetch(j,0);
        if(nDiff<0)      nValue+=1;
        else if(nDiff>0) nValue-=1;
        idAux->Dstore(nValue,j,0);
      }
    }
    while(nDiff!=0);

    //idAux->Fill_Int(1.0,0.0,1);
    for(k=0;k<idAux->GetNRecs();k++) idAux->Dstore(1.0,k,1);

    idNewPitch->Cat(idAux);
  }

  // Append last unvoiced segment
  nStartL = (INT32)idVoiced->Dfetch(idVoiced->GetNRecs()-1,0)+(INT32)idVoiced->Dfetch(idVoiced->GetNRecs()-1,1);
  idAux->SelectRecs(idPitch,nStartL,idPitch->GetNRecs()-nStartL);
  idNewPitch->Cat(idAux);


  DESTROYVIRTUAL(idPitch,idNewPitch);
  IDESTROY(idVoiced);
  IDESTROY(idAux);

  return O_K;
}

INT16 CGEN_PUBLIC CFBAproc::CompressPitch(CData *idPitch, CData *idNewPitch, FLOAT32 factor) {
  INT16  bVoiced    = FALSE;
  INT32   i          = 0;
  INT32   k          = 0;
  INT32   nCount     = 0;
  INT32   nStartL    = 0;
  FLOAT32  nMeanPeriodLengthL  = 0.0;
  FLOAT32  nMeanPeriodLength   = 0.0;
  CData* idVoiced   = NULL;
  CData* idAux      = NULL;

  // Validation
  if(idPitch == NULL)       return IERROR(this,ERR_NULLINST,0,0,0);
  if(idPitch->IsEmpty())    return IERROR(idPitch,DATA_EMPTY,idPitch->m_lpInstanceName,0,0);
  if
  (
    idPitch->GetNComps()!=2                            ||
    !dlp_is_numeric_type_code(idPitch->GetCompType(0)) ||
    !dlp_is_numeric_type_code(idPitch->GetCompType(1))
  )
  {
    return IERROR(this,FBA_BADARG,idPitch,"idPitch","contains invalid data.");
  }

  // Initialization
  CREATEVIRTUAL(CData,idPitch,idNewPitch);
  ICREATEEX(CData,idVoiced,"~idVoiced",NULL);
  ICREATEEX(CData,idAux   ,"~idAux"   ,NULL);
  idNewPitch->Reset();
  idVoiced->AddComp("start",T_INT);
  idVoiced->AddComp("count",T_INT);
  idVoiced->AddComp("mplen",T_FLOAT);
  idVoiced->Alloc(10);

  // Determine start and length of voiced parts and mean of periods in samples
  for(i=0; i<idPitch->GetNRecs(); i++)
  {
    if(idPitch->Dfetch(i,1)>0)
    {
      if(bVoiced==FALSE)    // Start of new voiced segment
      {
        bVoiced=TRUE;
        nStartL=i;
        nMeanPeriodLengthL=0;
        if(idVoiced->GetNRecs()==idVoiced->GetMaxRecs())
          idVoiced->Realloc(idVoiced->GetNRecs()+10);
        idVoiced->IncNRecs(1);
        idVoiced->Dstore(i,idVoiced->GetNRecs()-1,0);
      }
      nMeanPeriodLength+=(INT32)idPitch->Dfetch(i,0);
      nMeanPeriodLengthL+=(INT32)idPitch->Dfetch(i,0);
      nCount++;
    }
    else if(bVoiced==TRUE)  // End of voiced segment
    {
      bVoiced=FALSE;
      nMeanPeriodLengthL=nMeanPeriodLengthL/(FLOAT32)(i-nStartL);
      idVoiced->Dstore(i-nStartL,idVoiced->GetNRecs()-1,1);
      idVoiced->Dstore(nMeanPeriodLengthL,idVoiced->GetNRecs()-1,2);
    }
  }
  nMeanPeriodLength=nMeanPeriodLength/(FLOAT32)nCount;

  IFCHECK idVoiced->Print();
  IFCHECK printf("\n Input mean period length in voiced parts: %f",nMeanPeriodLength);

  // Resample
  for(i=0,nStartL=0; i<idVoiced->GetNRecs(); i++)
  {
    INT32 j       = 0;
    INT32 nSum    = 0;
    INT32 nSumNew = 0;
    INT32 nDiff   = 0;

    // Copy unvoiced
    idAux->SelectRecs(idPitch,nStartL,(INT32)idVoiced->Dfetch(i,0)-nStartL);
    idNewPitch->Cat(idAux);
    nStartL=(INT32)idVoiced->Dfetch(i,0)+(INT32)idVoiced->Dfetch(i,1);

    // Resample voiced
    idAux->SelectRecs(idPitch,(INT32)idVoiced->Dfetch(i,0),(INT32)idVoiced->Dfetch(i,1));
    for(j=0,nSum=0;j<idAux->GetNRecs();j++) nSum+=(INT32)idAux->Dfetch(j,0);  // Target sum
    for(j=0,nSumNew=0;j<idAux->GetNRecs();j++) {
      INT32 tmp = (INT32)idAux->Dfetch(j,0);
      tmp = (INT32)(nMeanPeriodLength + ((FLOAT64)tmp - nMeanPeriodLength) * factor + 0.5);
      tmp = (INT32)MAX(m_nSrate/500, tmp);
      tmp = (INT32)MIN(m_nSrate/50, tmp);
      idAux->Dstore(tmp,j,0);
      nSumNew += tmp;
    }

    idAux->Resample(idAux, (FLOAT64)nSum / (FLOAT64)nSumNew);

    for(j=0;j<idAux->GetNRecs();j++) {
      INT32 tmp = (INT32)idAux->Dfetch(j,0);
      tmp = (INT32)MAX(m_nSrate/500, tmp);
      tmp = (INT32)MIN(m_nSrate/50, tmp);
      idAux->Dstore(tmp,j,0);
    }

    do {
      nSumNew=0;
      for(j=0,nSumNew=0;j<idAux->GetNRecs();j++)
        nSumNew+=(INT32)idAux->Dfetch(j,0); // New sum
      nDiff=nSumNew-nSum;                  // Distribute difference
      IFCHECK printf("\n Distribute difference d=%ld",(long)nDiff);
      for(j=0;j<idAux->GetNRecs()&&j<abs(nDiff);j++)
      {
        INT32 nValue = (INT32)idAux->Dfetch(j,0);
        if(nDiff<0)      nValue+=1;
        else if(nDiff>0) nValue-=1;
        idAux->Dstore(nValue,j,0);
      }
    }
    while(nDiff!=0);

    //idAux->Fill_Int(1.0,0.0,1);
    for(k=0;k<idAux->GetNRecs();k++) idAux->Dstore(1.0,k,1);

    idNewPitch->Cat(idAux);
  }

  // Append last unvoiced segment
  nStartL = (INT32)idVoiced->Dfetch(idVoiced->GetNRecs()-1,0)+(INT32)idVoiced->Dfetch(idVoiced->GetNRecs()-1,1);
  idAux->SelectRecs(idPitch,nStartL,idPitch->GetNRecs()-nStartL);
  idNewPitch->Cat(idAux);


  DESTROYVIRTUAL(idPitch,idNewPitch);
  IDESTROY(idVoiced);
  IDESTROY(idAux);
  return O_K;
}

INT16 CGEN_PUBLIC CFBAproc::AdjustSpeechRate(CData *idPitch, CData *idNewPitch, CData* idFea, CData* idNewFea, FLOAT32 rate) {
  INT16  bVoiced    = FALSE;
  INT32   i          = 0;
  INT32   k          = 0;
  INT32   nCount     = 0;
  INT32   nStartL    = 0;
  FLOAT32  nMeanPeriodLengthL  = 0.0;
  FLOAT32  nMeanPeriodLength   = 0.0;
  CData* idVoiced   = NULL;
  CData* idAuxP     = NULL;
  CData* idAuxF     = NULL;

  // Validation
  if(idPitch == NULL)       return IERROR(this,ERR_NULLINST,0,0,0);
  if(idPitch->IsEmpty())    return IERROR(idPitch,DATA_EMPTY,idPitch->m_lpInstanceName,0,0);
  if
  (
    idPitch->GetNComps()!=2                            ||
    !dlp_is_numeric_type_code(idPitch->GetCompType(0)) ||
    !dlp_is_numeric_type_code(idPitch->GetCompType(1))
  )
  {
    return IERROR(this,FBA_BADARG,idPitch,"idPitch","contains invalid data.");
  }

  // Initialization
  CREATEVIRTUAL(CData,idPitch,idNewPitch);
  CREATEVIRTUAL(CData,idFea,idNewFea);
  ICREATEEX(CData,idVoiced,"~idVoiced",NULL);
  ICREATEEX(CData,idAuxP  ,"~idAuxP"  ,NULL);
  ICREATEEX(CData,idAuxF  ,"~idAuxF"  ,NULL);
  idNewPitch->Reset();
  idVoiced->AddComp("start",T_INT);
  idVoiced->AddComp("count",T_INT);
  idVoiced->AddComp("mplen",T_FLOAT);
  idVoiced->Alloc(10);

  AlignFramesToPitch(idPitch, idFea, idFea);

  // Determine start and length of voiced parts and mean of periods in samples
  for(i=0; i<idPitch->GetNRecs(); i++)
  {
    if(idPitch->Dfetch(i,1)>0)
    {
      if(bVoiced==FALSE)    // Start of new voiced segment
      {
        bVoiced=TRUE;
        nStartL=i;
        nMeanPeriodLengthL=0;
        if(idVoiced->GetNRecs()==idVoiced->GetMaxRecs())
          idVoiced->Realloc(idVoiced->GetNRecs()+10);
        idVoiced->IncNRecs(1);
        idVoiced->Dstore(i,idVoiced->GetNRecs()-1,0);
      }
      nMeanPeriodLength+=(INT32)idPitch->Dfetch(i,0);
      nMeanPeriodLengthL+=(INT32)idPitch->Dfetch(i,0);
      nCount++;
    }
    else if(bVoiced==TRUE)  // End of voiced segment
    {
      bVoiced=FALSE;
      nMeanPeriodLengthL=nMeanPeriodLengthL/(FLOAT32)(i-nStartL);
      idVoiced->Dstore(i-nStartL,idVoiced->GetNRecs()-1,1);
      idVoiced->Dstore(nMeanPeriodLengthL,idVoiced->GetNRecs()-1,2);
    }
  }
  nMeanPeriodLength=nMeanPeriodLength/(FLOAT32)nCount;

  IFCHECK idVoiced->Print();
  IFCHECK printf("\n Input mean period length in voiced parts: %f",nMeanPeriodLength);

  // Resample
  for(i=0,nStartL=0; i<idVoiced->GetNRecs(); i++)
  {
    INT32 j       = 0;
    INT32 nSum    = 0;
    INT32 nRecOld = 0;
    INT32 nRecNew = 0;

    // Copy unvoiced
    idAuxP->SelectRecs(idPitch,nStartL,(INT32)idVoiced->Dfetch(i,0)-nStartL);
    idNewPitch->Cat(idAuxP);
    idAuxF->SelectRecs(idFea,nStartL,(INT32)idVoiced->Dfetch(i,0)-nStartL);
    idNewFea->Cat(idAuxF);
    nStartL=(INT32)idVoiced->Dfetch(i,0)+(INT32)idVoiced->Dfetch(i,1);

    // Resample voiced
    idAuxP->SelectRecs(idPitch,(INT32)idVoiced->Dfetch(i,0),(INT32)idVoiced->Dfetch(i,1));
    nRecOld = idAuxP->GetNRecs();

    for(j=0,nSum=0;j<nRecOld;j++) nSum+=(INT32)idAuxP->Dfetch(j,0);  // Target sum

    idAuxP->Resample(idAuxP, rate);

    nRecNew = idAuxP->GetNRecs();

    for(j=0;j<nRecNew;j++) {
      INT32 tmp = (INT32)idAuxP->Dfetch(j,0);
      tmp = (INT32)MAX(m_nSrate/500, tmp);
      tmp = (INT32)MIN(m_nSrate/50, tmp);
      idAuxP->Dstore(tmp,j,0);
    }

    //idAux->Fill_Int(1.0,0.0,1);
    for(k=0;k<nRecNew;k++) idAuxP->Dstore(1.0,k,1);

    for(k=0;k<nRecNew;k++) {
      j = (INT32)((FLOAT64)k * (FLOAT64)nRecOld / (FLOAT64)nRecNew + 0.5);
      idAuxF->SelectRecs(idFea,j+(INT32)idVoiced->Dfetch(i,0),1);
      idNewFea->Cat(idAuxF);
    }

    idNewPitch->Cat(idAuxP);
  }

  // Append last unvoiced segment
  nStartL = (INT32)idVoiced->Dfetch(idVoiced->GetNRecs()-1,0)+(INT32)idVoiced->Dfetch(idVoiced->GetNRecs()-1,1);
  idAuxP->SelectRecs(idPitch,nStartL,idPitch->GetNRecs()-nStartL);
  idNewPitch->Cat(idAuxP);
  idAuxF->SelectRecs(idFea,nStartL,idPitch->GetNRecs()-nStartL);
  idNewFea->Cat(idAuxF);


  DESTROYVIRTUAL(idPitch,idNewPitch);
  DESTROYVIRTUAL(idFea,idNewFea);
  IDESTROY(idVoiced);
  IDESTROY(idAuxP);
  IDESTROY(idAuxF);

  return O_K;
}

INT16 CGEN_PUBLIC CFBAproc::AlignFramesToPitch(CData *idPitch, CData *idFea, CData* idNewFea) {
  CData* idAuxF  = NULL;
  INT32 nSamplesP = 0;
  INT32  nFea      = 0;
  INT32  nPer      = 0;
  INT32  i         = 0;
  INT32  j         = 0;

  if(m_nSync) return O_K;

  if(idFea == NULL)       return IERROR(this,ERR_NULLINST,0,0,0);
  if(idFea->IsEmpty())    return IERROR(idPitch,DATA_EMPTY,idPitch->m_lpInstanceName,0,0);

  if(idPitch == NULL)       return IERROR(this,ERR_NULLINST,0,0,0);
  if(idPitch->IsEmpty())    return IERROR(idPitch,DATA_EMPTY,idPitch->m_lpInstanceName,0,0);
  if(idPitch->GetNComps()!=2                            ||
     !dlp_is_numeric_type_code(idPitch->GetCompType(0)) ||
     !dlp_is_numeric_type_code(idPitch->GetCompType(1))) {
    return IERROR(this,FBA_BADARG,idPitch,"idPitch","contains invalid data.");
  }

  CREATEVIRTUAL(CData,idFea,idNewFea);
  ICREATEEX(CData,idAuxF  ,"~idAuxF"  ,NULL);

  if(idFea->m_lpTable->m_fsr <= 0.0) {
    idFea->m_lpTable->m_fsr = 1000.0 * (FLOAT64)m_nCrate / (FLOAT64)m_nSrate;
  }

  nFea = idFea->GetNRecs();

  for(i = 0, j = 0, nSamplesP = 0; i < idPitch->GetNRecs(); i++, nSamplesP+=nPer) {
    nPer = (INT32)idPitch->Dfetch(i,0);

    while((idFea->m_lpTable->m_fsr * (FLOAT64)(j+0.5)) < (1000.0 * (FLOAT64)nSamplesP / (FLOAT64)m_nSrate)) j++;

    j = (j>=nFea) ? nFea-1 : j;

    idAuxF->SelectRecs(idFea,j,1);
    idNewFea->Cat(idAuxF);
  }

  IDESTROY(idAuxF);
  DESTROYVIRTUAL(idFea, idNewFea);
  return O_K;
}

INT16 CGEN_PUBLIC CFBAproc::GenPitch(CData *idProso, CData *idPitch)
{
  INT32  i           = 0;
  INT32  j           = 0;
  INT32  nRecs       = 0;
  INT32  nLastSample = 0;
  INT16  bVoiced     = 0;
  INT32  nF0         = 0;
  INT32  nSamples    = 0;
  INT32  nNextPeak   = 0;
  INT32  nPm         = 0;

   // Validation
  if (idPitch == NULL)            return IERROR(this,ERR_NULLINST,0,0,0);
  if (idProso == NULL)            return IERROR(this,ERR_NULLINST,0,0,0);
  if (idProso->IsEmpty() == TRUE) return IERROR(idProso,DATA_EMPTY,idProso->m_lpInstanceName,0,0);
  if (idProso->GetNComps()!=3 ||
      !dlp_is_numeric_type_code(idProso->GetCompType(0)) ||
      !dlp_is_numeric_type_code(idProso->GetCompType(1)) ||
      !dlp_is_numeric_type_code(idProso->GetCompType(2))
      )
    return IERROR(idProso,FBA_BADINTO,idProso->m_lpInstanceName,0,0);

  // Initialization
  idPitch->Reset();
  idPitch->AddComp("pm", T_SHORT);
  idPitch->AddComp("v/uv",T_SHORT);
  nRecs = idProso->GetNRecs();

  // For every record in idProso
  for (i=0; i<nRecs; i++)
  {
    bVoiced     = (INT16)      idProso->Dfetch(i ,2 );              // unvoiced = 0
    nF0         = (INT32)  idProso->Dfetch(i ,1 );              // nF0 in hz
    nSamples    = (INT32)  idProso->Dfetch(i ,0 ) * m_nCrate ;  // duration in samples
    nPm         = (INT32)  (m_nSrate / nF0);
    nLastSample = nLastSample + nSamples;

    while (nLastSample > nNextPeak)
    {
      IFCHECK printf("\n Write %s pitch starting from %ld %ld samples long.",bVoiced==0?"unvoiced":"bVoiced",(long)nNextPeak,(long)nPm);
      idPitch->AddRecs(1,FBA_GRANY);
      idPitch->Dstore(nPm, j, 0);
      idPitch->Dstore(bVoiced, j, 1);
      j++;
      nNextPeak= nPm + nNextPeak;
    }
  }

  return O_K;
}

/*
 * Synthesis
 *
 * @return O_K if successfull, NOT_EXEC otherwise
 */
INT16 CGEN_PUBLIC CFBAproc::Synthesize(data *idFea, data *idControl, data *idSyn) {
  if((idFea != NULL) && (idSyn != NULL)) {
    if (idControl == NULL)               return IERROR(this,ERR_NULLINST,0,0,0);
    if (idFea     == NULL)               return IERROR(this,ERR_NULLINST,0,0,0);
    if (idSyn     == NULL)               return IERROR(this,ERR_NULLINST,0,0,0);
    if (idControl->IsEmpty())            return IERROR(idControl,DATA_EMPTY,idControl->m_lpInstanceName,0,0);
    if (idFea->IsEmpty())                return IERROR(idFea,DATA_EMPTY,idFea->m_lpInstanceName,0,0);
    if (idFea->GetCompType(0)!=T_DOUBLE) return IERROR(idFea,DATA_BADCOMPTYPE,0,idFea->m_lpInstanceName,"double");
    if(dlp_is_numeric_type_code(idControl->GetCompType(0)) &&
       dlp_is_numeric_type_code(idControl->GetCompType(1))) {
      if(SynthesizeUsingPM(idFea, idControl, idSyn) != O_K) return NOT_EXEC;
    } else if(dlp_is_symbolic_type_code(idControl->GetCompType(0)) &&
              dlp_is_numeric_type_code (idControl->GetCompType(1)) &&
              dlp_is_symbolic_type_code(idControl->GetCompType(2)) &&
              dlp_is_numeric_type_code (idControl->GetCompType(3)) &&
              dlp_is_numeric_type_code (idControl->GetCompType(4)) &&
              dlp_is_symbolic_type_code(idControl->GetCompType(5)) &&
              dlp_is_numeric_type_code (idControl->GetCompType(6)) &&
              dlp_is_numeric_type_code (idControl->GetCompType(7))) {
      if(SynthesizeUsingInto(idFea, idControl, idSyn) != O_K) return NOT_EXEC;
    } else {
      IERROR(this,FBA_BADPITCH,idControl->m_lpInstanceName,0,0);
      return IERROR(this,FBA_BADINTO, idControl->m_lpInstanceName,0,0);
    }

    ISETFIELD_RVALUE(idSyn,"rinc",1000/m_nSrate);
    for(INT32 i = 0; i < idFea->GetNComps(); i++) {
      if(dlp_is_symbolic_type_code(idFea->GetCompType(i))) {
        idSyn->AddComp(idFea->GetCname(i),idFea->GetCompType(i));
      }
    }
    idSyn->CopyLabels(idFea);
  }

  return O_K;
}

/*
 * Synthesis using pitch marks.
 *
 * Derived instances of FBAproc should override method
 * SynthesizePM() to add the desired functionality
 *
 * @return O_K if successfull, NOT_EXEC otherwise
 */
INT16 CGEN_VPROTECTED CFBAproc::SynthesizeUsingPM(data *idFea, data *idPm, data *idSyn) {
  CData   *idExcite    = NULL;
  FLOAT64 *excitation  = NULL;
  FLOAT64 *synthesis   = NULL;
  INT32    nSamples    = 0;
  INT16    nPer        = 0;
  INT32    nCompVuv    = idFea->FindComp("v/uv");
  INT16    nCompFea    = idFea->GetNNumericComps() - (nCompVuv >= 0 ? 1 : 0);
  INT32    iRecFea     = 0;
  INT32    nRecFea     = 0;

  AlignFramesToPitch(idPm, idFea, idFea);

  nRecFea = idFea->GetNRecs();

  if(m_bSynEnhancement) {
    for(iRecFea = 0; iRecFea < nRecFea; iRecFea++) {
      FeaEnhancement((FLOAT64*)idFea->XAddr(iRecFea,0), nCompFea);
    }
  }

  Smooth(idFea, idPm, idFea);

  // Generate modulated excitation
  ICREATEEX(CData,idExcite,"~idExcite",NULL);
  DLPASSERT(O_K==ModEx(idPm,idExcite));
  DLPASSERT(!idExcite->IsEmpty());

  nSamples = idExcite->GetNRecs();

  idSyn->Reset();
  idSyn->AddComp("syn", T_DOUBLE);
  idSyn->Allocate(nSamples);

  excitation = (FLOAT64*)idExcite->XAddr(0, 0);
  synthesis = (FLOAT64*)idSyn->XAddr(0, 0);

  for(INT32 currSample = 0, currPer = 0; (currSample < nSamples) && (currPer < idFea->GetNRecs()); currSample += nPer, currPer++) {
    nPer = (INT16)idPm->Dfetch(currPer,0);

    if(SynthesizeFrame((FLOAT64*)idFea->XAddr(currPer,0), nCompFea, excitation+currSample, nPer, synthesis+currSample) != O_K) {
      IDESTROY(idExcite);
      return IERROR(this,FBA_SYNTHESISE, currPer, 0,0);
    }
  }

  IDESTROY(idExcite);

  if (m_nMinLog != 0.0) for (INT32 i=0; i<idSyn->GetNRecs(); i++) *((FLOAT64*)idSyn->XAddr(0, 0)+i) *= exp(m_nMinLog);

  return O_K;
}

INT16 CGEN_PRIVATE CFBAproc::SynthesizeUsingIntoGetFeaIntoLab(CData* idFea, CData* idInto, SLAB** sFeaLab, SLAB** sIntoLab, INT32* nSamples, INT32* nLab) {
  INT32         nSamplingPoints               = idInto->GetNRecs();
  INT32         iSamplingPoints1              = 0;
  INT32         iSamplingPoints2              = 0;
  INT32         nCompPho                      = -1;
  INT32         iFrames                       = 0;
  INT32         nFrames                       = idFea->GetNRecs();
  INT32         iLab                          = 0;
  const char*   lastUnit                      = NULL;
  const char*   currUnit                      = NULL;
  FLOAT64       currDura                      = 0.0;

  nCompPho = idFea->FindComp("~PHO");
  if(nCompPho < 0) nCompPho = idFea->FindComp("lab");
  if(nCompPho < 0) return IERROR(this,FBA_SYNTHESISE,0,"Missing label track.",0);

  iLab = 0;
  iFrames = 1;
  lastUnit = idFea->Sfetch(0, nCompPho);
  while(iFrames < nFrames) {
    currUnit = idFea->Sfetch(iFrames, nCompPho);
    if(dlp_strcmp(currUnit, lastUnit)) {
      (*sFeaLab) = (SLAB*)dlp_realloc((*sFeaLab), iLab+1, sizeof(SLAB));
      (*sFeaLab)[iLab].phoneme = lastUnit;
      (*sFeaLab)[iLab].pos = iFrames*m_nCrate;
      lastUnit = currUnit;
      iLab++;
    }
    iFrames++;
  }
  (*sFeaLab) = (SLAB*)dlp_realloc((*sFeaLab), iLab+1, sizeof(SLAB));
  (*sFeaLab)[iLab].phoneme = lastUnit;
  (*sFeaLab)[iLab].pos = iFrames*m_nCrate;
  iLab++;

  *nLab = 0;
  lastUnit = "";
  iSamplingPoints1 = 0;
  while(iSamplingPoints1 < nSamplingPoints) {
    currUnit  = idInto->Sfetch(iSamplingPoints1, 0);
    currDura  = idInto->Dfetch(iSamplingPoints1, 1);
    if((currDura <= 0) || (idInto->m_lpCunit[0] == '%')) {
      if(*nLab < iLab) {
        if((currDura >= 0) && (idInto->m_lpCunit[0] == '%')) {
          currDura = (((*sFeaLab)[*nLab].pos - ((*nLab==0)? 0: (*sFeaLab)[*nLab-1].pos)) * 1000 / m_nSrate) * (currDura / 100.0);
        } else {
          currDura = ((*sFeaLab)[*nLab].pos - ((*nLab==0)? 0: (*sFeaLab)[*nLab-1].pos)) * 1000 / m_nSrate;
        }
        iSamplingPoints2 = iSamplingPoints1;
        while((iSamplingPoints2 < nSamplingPoints) && !dlp_strcmp(idInto->Sfetch(iSamplingPoints2, 0), currUnit) && (idInto->Dfetch(iSamplingPoints2,1) <= 0)) {
          idInto->Dstore(currDura, iSamplingPoints2, 1);
          iSamplingPoints2++;
        }
      } else {
        dlp_free(*sFeaLab);
        return IERROR(this, FBA_SYNINTOLAB, currUnit, iLab-1, 0);
      }
    }
    if(dlp_strcmp(currUnit, lastUnit)) {
      if(dlp_strcmp(currUnit, (*sFeaLab)[*nLab].phoneme)) {
        dlp_free(*sFeaLab);
        return IERROR(this, FBA_SYNINTOLAB, lastUnit, iLab, 0);
      }
      lastUnit = currUnit;
      *nSamples += (INT32)(currDura * (FLOAT64)m_nSrate / 1000.0 +0.5);
      (*sIntoLab) = (SLAB*)dlp_realloc((*sIntoLab), *nLab+1, sizeof(SLAB));
      (*sIntoLab)[*nLab].phoneme = currUnit;
      (*sIntoLab)[*nLab].pos = *nSamples;
      (*nLab)++;
    }
    iSamplingPoints1++;
  }
  return O_K;
}
/*
 * Replaces phoneme durations with durations from inventory, if
 * durations are equal to zero.
 * idFea is supposed to contain the concatenated inventory units.
 * *
 * @return O_K if successfull, NOT_EXEC otherwise
 */
INT16 CGEN_VPROTECTED CFBAproc::GetDurationsFromInventory(data *idFea, data *idInto) {
  INT16 ret      = O_K;
  INT32 nSamples = 0;
  INT32 nLab     = 0;
  SLAB* sFeaLab  = NULL;
  SLAB* sIntoLab = NULL;

  ret = SynthesizeUsingIntoGetFeaIntoLab(idFea, idInto, &sFeaLab, &sIntoLab, &nSamples, &nLab);
  dlp_free(sFeaLab);
  dlp_free(sIntoLab);
  return ret;
}

/*
 * Synthesis using into control.
 * *
 * @return O_K if successfull, NOT_EXEC otherwise
 */  INT32         nSamples                      = 0;

INT16 CGEN_VPROTECTED CFBAproc::SynthesizeUsingInto(data *idFea, data *idInto, data *idSyn) {
  INT32         nSamplingPoints               = idInto->GetNRecs();
  INT32         iSamplingPoints1              = 0;
  INT32         iSamples                      = 0;
  INT32         iSamplesOld                   = 0;
  INT32         nSamples                      = 0;
  INT32         nSamplesOfLastUnits           = 0;
  INT32         nSampleOfLastF0SamplingPoint  = 0;
  INT32         nSampleOfNextF0SamplingPoint  = 0;
  INT32         nPer                          = 0;
  INT32         iFrames                       = 0;
  INT32         iFramesOld                    = 0;
  INT32         nFrames                       = idFea->GetNRecs();
  INT32         iLab                          = 0;
  INT32         nLab                          = 0;
  INT32         iFea                          = 0;
  INT32         nCompVuv                      = idFea->FindComp("v/uv");
  INT32         nFea                          = idFea->GetNNumericComps() - (nCompVuv >= 0 ? 1 : 0);
  const char*   nextF0Pho                     = NULL;
  FLOAT64       lastF0Val                     = 1.0;
  FLOAT64       currF0Val                     = 1.0;
  FLOAT64       nextF0Val                     = 1.0;
  FLOAT64       nextF0Pos                     = 0.0;
  FLOAT64       lastI0Val                     = 1.0;
  FLOAT64       currI0Val                     = 1.0;
  FLOAT64       nextI0Val                     = 1.0;
  FLOAT64*      exc                           = NULL;
  FLOAT64*      fea                           = NULL;
  FLOAT64*      feaBefore                     = NULL;
  FLOAT64*      feaUsed                       = NULL;
  FLOAT64*      feaUsedBefore                 = NULL;
  BOOL          isVoiceless                   = FALSE;
  BOOL          isSmoothable                  = FALSE;
  BOOL          isSmoothableBefore            = FALSE;
  SLAB*         sIntoLab                      = NULL;
  SLAB*         sFeaLab                       = NULL;

  if(nSamplingPoints <= 0) {
    return O_K;
  }

  if(SynthesizeUsingIntoGetFeaIntoLab(idFea, idInto, &sFeaLab, &sIntoLab, &nSamples, &nLab) != O_K) {
    if(sFeaLab != NULL) dlp_free(sFeaLab);
    if(sIntoLab != NULL) dlp_free(sIntoLab);
    return NOT_EXEC;
  }

  exc       = (FLOAT64*)dlp_malloc(nSamples * sizeof(FLOAT64));
  fea       = (FLOAT64*)dlp_malloc(nFea * sizeof(FLOAT64));
  feaBefore = (FLOAT64*)dlp_malloc(nFea * sizeof(FLOAT64));

  idSyn->Reset(TRUE);
  idSyn->AddComp("syn", T_DOUBLE);
  idSyn->Allocate(nSamples);

  iFrames = 0;
  iLab = 0;
  iSamplingPoints1 = 0;
  iSamples = 0;
  iSamplesOld = 0;
  iFramesOld = 0;
  nSamplesOfLastUnits = 0;
  while(iSamples < nSamples) {
    if(iSamples >= nSampleOfNextF0SamplingPoint) {
      lastF0Val = nextF0Val;
      lastI0Val = nextI0Val;
      nSampleOfLastF0SamplingPoint = nSampleOfNextF0SamplingPoint;
      while(iSamples >= nSampleOfNextF0SamplingPoint) {
        for(; iSamplingPoints1 < nSamplingPoints; iSamplingPoints1++) {
          nextF0Pho = idInto->Sfetch(iSamplingPoints1, 2);
          nextF0Val = idInto->Dfetch(iSamplingPoints1, 3);
          nextI0Val = idInto->Dfetch(iSamplingPoints1, 6);
          nextF0Pos = idInto->Dfetch(iSamplingPoints1, 4);
          if(iSamplingPoints1 && dlp_strcmp(idInto->Sfetch(iSamplingPoints1-1, 0), idInto->Sfetch(iSamplingPoints1, 0))) {
            nSamplesOfLastUnits += (INT32)idInto->Dfetch(iSamplingPoints1-1, 1);
          }
          if((nextF0Val > 0.0) && (nextF0Pos >= 0.0) && (nextF0Pho != NULL) && (dlp_strlen(nextF0Pho) > 0)) {
            break;
          } else {
            if(this->m_nCheck > 0) {
              dlp_message(__FILE__,__LINE__,"no next f0 value");
            }
          }

        }
        if(iSamplingPoints1 >= nSamplingPoints) {
          nextF0Val = 1.0;
          nextI0Val = 1.0;
          nSampleOfNextF0SamplingPoint = nSamples;
        } else {
          nSampleOfNextF0SamplingPoint = (INT32)(((FLOAT64)nSamplesOfLastUnits + idInto->Dfetch(iSamplingPoints1, 1) * nextF0Pos) * (FLOAT64)m_nSrate / 1000.0 + 0.5);
          iSamplingPoints1++;
        }
      }
    }

    currF0Val = lastF0Val + (nextF0Val - lastF0Val) * (iSamples - nSampleOfLastF0SamplingPoint) / (nSampleOfNextF0SamplingPoint+1 - nSampleOfLastF0SamplingPoint);
    nPer = (INT32)((FLOAT64)m_nSrate / (currF0Val * (FLOAT64)m_nBaseF0) + 0.5);
    if((iSamples+nPer) > nSamples) nPer = nSamples-iSamples;

    currI0Val = lastI0Val + (nextI0Val - lastI0Val) * (iSamples - nSampleOfLastF0SamplingPoint) / (nSampleOfNextF0SamplingPoint+1 - nSampleOfLastF0SamplingPoint);

    while((iLab < nLab) && (sIntoLab[iLab].pos < iSamples)) {
      iSamplesOld = sIntoLab[iLab].pos;
      iFramesOld = INT32((FLOAT64)sFeaLab[iLab].pos / (FLOAT64)m_nCrate + 0.5);
      iLab++;
    }
    while((iFrames < nFrames) &&
        (((FLOAT64)(((iFrames-iFramesOld+1L))*m_nCrate)/((iLab>0)?(FLOAT64)(sFeaLab[iLab].pos -sFeaLab[iLab-1].pos ):sFeaLab[iLab].pos )) <
          ((FLOAT64)(iSamples-iSamplesOld)              /((iLab>0)?(FLOAT64)(sIntoLab[iLab].pos-sIntoLab[iLab-1].pos):sIntoLab[iLab].pos))))
      iFrames++;

    dlp_memmove(fea, (FLOAT64*)idFea->XAddr(iFrames, 0), nFea*sizeof(FLOAT64));
    if(m_bSynEnhancement) {
      FeaEnhancement(fea, nFea);
    }

    if(nCompVuv>=0) {
      isVoiceless  = (*(INT16*)idFea->XAddr(iFrames, nCompVuv) & 1) == 0;
      isSmoothable = (*(INT16*)idFea->XAddr(iFrames, nCompVuv) & 2) == 0;
    } else {
      isVoiceless = IsFeaVoiceless(fea, nFea);
      isSmoothable = 1;
    }

    if(feaUsed == NULL) {
      feaUsed = (FLOAT64*)dlp_malloc(nFea*sizeof(FLOAT64));
      dlp_memmove(feaUsed, fea, nFea*sizeof(FLOAT64));
    }

    if(m_bSynSmoothFea) {
      if(feaUsedBefore == NULL) {
        feaUsedBefore = (FLOAT64*)dlp_malloc(nFea*sizeof(FLOAT64));
        dlp_memmove(feaUsedBefore, fea, nFea*sizeof(FLOAT64));
      } else {
        if(isSmoothableBefore && isSmoothable) {
          for(iFea = 0; iFea < nFea; iFea++) {
            feaUsed[iFea] = 0.29289322 * (feaBefore[iFea] + fea[iFea]) + 0.41421356 * feaUsedBefore[iFea];
          }
        } else if(isSmoothableBefore && !isSmoothable) {
          for(iFea = 0; iFea < nFea; iFea++) {
            feaUsed[iFea] = 0.75 * feaBefore[iFea] + 0.25 * fea[iFea];
          }
        } else if(!isSmoothableBefore && isSmoothable) {
          for(iFea = 0; iFea < nFea; iFea++) {
            feaUsed[iFea] = 0.25 * feaBefore[iFea] + 0.75 * fea[iFea];
          }
        } else {
          dlp_memmove(feaUsed, fea, nFea*sizeof(FLOAT64));
        }
      }
      isSmoothableBefore = isSmoothable;
      dlp_memmove(feaUsedBefore, feaUsed, nFea*sizeof(FLOAT64));
    } else {
      dlp_memmove(feaUsed, fea, nFea*sizeof(FLOAT64));
    }
    dlp_memmove(feaBefore, fea, nFea*sizeof(FLOAT64));

    FLOAT64 gain = ((m_nWnorm == 0 ) ? 1.0 : (FLOAT64)sqrt((double)nPer)) * exp(m_nBaseI0) * currI0Val;

    switch(m_lpsExcType[0]) {
    case 'P':
      dlm_getExcPeriod(nPer, !isVoiceless, DLM_PITCH_PULSE, 1.0/gain, m_nSrate, exc+iSamples);
      break;
    case 'G':
      dlm_getExcPeriod(nPer, !isVoiceless, DLM_PITCH_GLOTT, 1.0/gain, m_nSrate, exc+iSamples);
      break;
    case 'R':
      dlm_getExcPeriod(nPer, !isVoiceless, DLM_PITCH_RANDPHASE, 1.0/gain, m_nSrate, exc+iSamples);
      break;
    case 'V':
      dlm_getExcPeriod(nPer, !isVoiceless, DLM_PITCH_VOICED, 1.0/gain, m_nSrate, exc+iSamples);
      break;
    case 'U':
      dlm_getExcPeriod(nPer, !isVoiceless, DLM_PITCH_UNVOICED, 1.0/gain, m_nSrate, exc+iSamples);
      break;
    default:
      return IERROR(this,FBA_BADARG,m_lpsExcType,"exc_type","P, G, R, V or U");
    }

    if(this->m_nCheck > 0) {
      dlp_message(__FILE__,__LINE__,"nPer=%5d, uv/v=%1d, s/ns=%ld, pos=%7.1f, F0=%4.2f/%4.2f/%4.2f, idFea (%3d) %5s, Fea (%3d): %5s (%3d), Into (%3d) %5s (%3d)", \
          (int)nPer,
          (int)!isVoiceless,
          (int)isSmoothable,
          (double)((FLOAT64)iSamples*1000.0/(FLOAT64)m_nSrate),
          (double)lastF0Val,
          (double)currF0Val,
          (double)nextF0Val,
          (int)iFrames,
          idFea->Sfetch(iFrames, nFea),
          (int)iLab, sFeaLab[iLab].phoneme,
          (int)sFeaLab[iLab].pos,
          (int)iLab, sIntoLab[iLab].phoneme,
          (int)sIntoLab[iLab].pos);
    }

    if(SynthesizeFrame(feaUsed, nFea, &exc[iSamples], nPer, (FLOAT64*)idSyn->XAddr(iSamples, 0)) != O_K) return IERROR(this,FBA_SYNTHESISE, iFrames, 0,0);

    iSamples += nPer;
  }

  dlp_free(exc);
  dlp_free(sIntoLab);
  dlp_free(sFeaLab);
  dlp_free(fea);
  dlp_free(feaBefore);
  dlp_free(feaUsed);
  if(m_bSynSmoothFea) {
    dlp_free(feaUsedBefore);
  }

  if (m_nMinLog != 0.0) for (INT32 i=0; i<idSyn->GetNRecs(); i++) *((FLOAT64*)idSyn->XAddr(0, 0)+i) *= exp(m_nMinLog);

  return O_K;
}

BOOL CGEN_VPUBLIC CFBAproc::IsFeaVoiceless(FLOAT64* fea, INT16 n_fea) {
  return FALSE;
}

INT16 CGEN_VPUBLIC CFBAproc::FeaEnhancement(FLOAT64* fea, INT16 n_fea) {
  return NOT_EXEC;
}

/*
 * Convert feature vector to speech signal.
 *
 * Derived instances of FBAproc should override method
 * SynthesizeFrameImpl() to add the desired functionality
 *
 * @return O_K if successfull, NOT_EXEC otherwise
 */
INT16 CGEN_PUBLIC CFBAproc::SynthesizeFrame(FLOAT64* fea, INT16 n_fea, FLOAT64* exc, INT32 n_exc, FLOAT64* syn) {
  return SynthesizeFrameImpl(fea, n_fea, exc, n_exc, m_nPfaLambda, m_nSynLambda, syn);
}

INT16 CGEN_VPROTECTED CFBAproc::SynthesizeFrameImpl(FLOAT64* fea, INT16 n_fea, FLOAT64* exc, INT32 n_exc, FLOAT64 nPfaLambda, FLOAT64 nSynLambda, FLOAT64* syn) {
  return IERROR(this, ERR_NOTSUPPORTED, "SynthesizeFrame of class CFBAproc", 0, 0);
}

//EOF
