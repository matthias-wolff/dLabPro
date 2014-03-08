// dLabPro class CFBAproc (FBAproc)
// - Frame based functions
//
// AUTHOR : Matthias Eichner
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

/*
 * Run analysis
 *
 * Derived instances of FBAproc should override method
 * Analyse() to add the desired functionality
 *
 * @param idSignal  data instance containing the signal to analyse
 * @param idPitch    data instance containing the pitch information
 * @param idReal    output data instance for real part
 * @param idImag    output data instance for imaginary part
 * @return O_K if successfull, NOT_EXEC otherwise
 */
INT16 CGEN_PUBLIC CFBAproc::Analyze(data *idSignal, data *idPitch, data *idReal, data *idImag) {
  INT32 nFrame = 0;
  CData* idFrames = NULL;
  CData* idWLen = NULL;

  // Validation
  if (!idSignal) {
    IERROR(this,ERR_NULLARG,"idSignal",0,0);
    return NOT_EXEC;
  }
  if (idSignal->IsEmpty()) {
    IERROR(idSignal,DATA_EMPTY,idSignal->m_lpInstanceName,0,0);
    return NOT_EXEC;
  }
  if (idSignal->GetNComps() > 2 || (idSignal->GetNComps() == 1 && !dlp_is_numeric_type_code(idSignal->GetCompType(0)))
      || (idSignal->GetNComps() == 2 && (!dlp_is_numeric_type_code(idSignal->GetCompType(0))
          || !dlp_is_symbolic_type_code(idSignal->GetCompType(1))))) {
    IERROR(this,FBA_BADINPUT,idSignal->m_lpInstanceName,0,0);
    return NOT_EXEC;
  }
  if (idPitch != NULL && !(idPitch->GetNComps() == 2 && dlp_is_numeric_type_code(idPitch->GetCompType(0))
      && dlp_is_numeric_type_code(idPitch->GetCompType(1)))) {
    IERROR(this,FBA_BADPITCH,idPitch->m_lpInstanceName,0,0);
    return NOT_EXEC;
  }

  InitBuffers();

  if (idPitch != NULL) m_nSync = TRUE;
  else m_nSync = FALSE;
  if ((INT32) (m_nSrate + 0.5) != (INT32) (1000. / idSignal->m_lpTable->m_fsr + 0.5)) IERROR(this,FBA_BADSAMP,idSignal->m_lpInstanceName,m_lpInstanceName,0);

  // Calculate DC
  ICREATEEX(CData,idFrames,"~frames",NULL);
  ICREATEEX(CData,idWLen,"~wlen",NULL);
  if (m_bRmdc) {
    ISETOPTION(idFrames,"/rec");
    idFrames->Aggregate(idSignal, NULL, CMPLX(0.0), "mean");
    IRESETOPTIONS(idFrames);
    m_nDC = idFrames->Dfetch(0, 0);
    idFrames->Reset();
  } else m_nDC = 0.0;

  IFIELD_RESET(CData,"labels");

  // Frame signal
  DoFraming(idSignal, idPitch, idFrames, idWLen);

  PrepareOutput(idReal);
  if (idImag) PrepareOutput(idImag);

  // append feature vector to output instances
  idReal->AddRecs(idFrames->GetNRecs(), FBA_GRANY);
  if (idImag!=NULL) idImag->AddRecs(idFrames->GetNRecs(), FBA_GRANY);

  if (m_nMatrixAnalysis == 1) {

    AnalyzeMatrix(idFrames, idReal); // (input,output)

    //IF_NOK(AnalyzeMatrix()) {
    //  IERROR(this, FBA_ANALYSE, nFrame, 0, 0);
    //  break;
    //}

  } else {

    for (nFrame = 0; nFrame < idFrames->GetNRecs(); nFrame++) {
      // Reset frame buffer
      dlp_memset(m_idRealFrame->XAddr(0, 0), 0, m_idRealFrame->GetRecLen());
      if (idImag != NULL) dlp_memset(m_idImagFrame->XAddr(0, 0), 0, m_idImagFrame->GetRecLen());

      // Copy current frame to frame buffer
      dlp_memmove(m_idRealFrame->XAddr(0, 0), idFrames->XAddr(nFrame, 0), idFrames->GetRecLen());

      // set current wLen
      if (m_nSync) m_nWlen = (INT32) idWLen->Dfetch(nFrame, 0);

      // do analysis
      IF_NOK(AnalyzeFrame()) {
        IERROR(this, FBA_ANALYSE, nFrame, 0, 0);
        break;
      }

      // append feature vector to output instances
      dlp_memmove(idReal->XAddr(nFrame, 0), m_idRealFrame->XAddr(0, 0), idReal->GetRecLen());

      if (idImag!=NULL) 
        dlp_memmove(idImag->XAddr(nFrame, 0), m_idImagFrame->XAddr(0, 0), idImag->GetRecLen());
    }
  }

  Smooth(idReal, NULL, idReal);

  // Join labels
  if (!CData_IsEmpty(m_idLabels)) {
    idReal->Join(m_idLabels);
    if(idImag!=NULL) idImag->Join(m_idLabels);
  }

  IDESTROY(idFrames);
  IDESTROY(idWLen);
  return O_K;
}

/**
 * Convert signal to sequence of frames, apply weigthing window and calculate
 * energy per frame (optional).
 *
 * @param  idSignal Input signal
 * @param  idPitch  Pitch information
 * @param  idFrames Resulting sequence of frames
 * @return TRUE if successful, FALSE otherwise
 */
INT16 CGEN_PROTECTED CFBAproc::DoFraming(data *idSignal, data *idPitch, data *idFrames, data *idWLen) {
  INT32 i = 0;
  INT32 j = 0;
  const INT32 nLabCompIn = 1;
  INT32 nXFrames = 0;
  INT32 nXSamples = CData_GetNRecs(idSignal);
  FLOAT64 *lpSignal;
  char bFreeSignal = FALSE;
  const char **lpLabIn = NULL;
  char **lpLabOut = NULL;
  INT32 nXPitch = idPitch ? idPitch->GetNRecs() : 0;
  INT64 *lpPitch = NULL;
  char bFreePitch = FALSE;
  char bLabelDiffer = FALSE;
  struct dlm_fba_doframing_param lpParam;
  lpParam.nLambda = m_nPfaLambda;
  lpParam.nCrate = m_nCrate;
  lpParam.nDC = m_nDC;
  lpParam.nLen = m_nLen;
  lpParam.nMinLog = m_nMinLog;
  lpParam.nNPeriods = m_nNPeriods;
  lpParam.nPreem = m_nPreem;
  lpParam.nWlen = m_nWlen;
  lpParam.nWnorm = m_nWnorm;
  strcpy(lpParam.lpsWtype, m_lpsWtype);
  lpParam.bEnergy = m_bEnergy;
  lpParam.bLogEnergy = m_bLogEnergy;
  lpParam.bTimeDomainWarping = m_bTimeDomainWarping;

  // Get signal buffer
  if (CData_GetNComps(idSignal) == 1 && CData_GetCompType(idSignal, 0) == T_DOUBLE) lpSignal
      = (FLOAT64 *) CData_XAddr(idSignal, 0, 0);
  else {
    bFreeSignal = TRUE;
    lpSignal = (FLOAT64 *) dlp_malloc(sizeof(FLOAT64)*nXSamples);
    for (i = 0; i < nXSamples; i++)
      lpSignal[i] = (FLOAT64) CData_Dfetch(idSignal, i, 0);
  }

  if (idPitch) {
    INT32 nMaxFrameLength;
    // Get pitch buffer
    if (CData_GetNComps(idPitch) == 1 && CData_GetCompType(idPitch, 0) == T_INT) lpPitch
        = (INT64*) CData_XAddr(idPitch, 0, 0);
    else {
      bFreePitch = TRUE;
      lpPitch = (INT64*) dlp_malloc(sizeof(INT64)*nXPitch);
      for (i = 0; i < nXPitch; i++)
        lpPitch[i] = (INT32) CData_Dfetch(idPitch, i, 0);
    }
    // Get nXFrames and maximal window size
    dlm_fba_doframing_maxframelen(nXSamples, lpPitch, nXPitch, m_nNPeriods, m_nLen, m_nCrate, m_nWlen, &nMaxFrameLength, &nXFrames);
    if (dlp_strncmp(dlp_strlwr(m_lpsWtype), "custom", 255)) CData_Allocate(m_idWindow, nMaxFrameLength);
  } else {
    // How many frames to process in synchonous mode?
    nXFrames = nXSamples / m_nCrate;
    if (nXSamples % m_nCrate != 0.0) nXFrames += 1;
    // Create window
    MakeWindow(m_nWlen);
  }

  // Prepare output instance(s)
  CREATEVIRTUAL(CData,idSignal,idFrames);
  idFrames->Reset(TRUE);
  idFrames->AddNcomps(T_DOUBLE,m_nLen);
  idFrames->Allocate(nXFrames);

  if (m_bEnergy || m_bLogEnergy) {
    m_idEnergy->Reset(TRUE);
    if (m_bEnergy) m_idEnergy->AddComp("ener", T_DOUBLE);
    else if (m_bLogEnergy) m_idEnergy->AddComp("loge", T_DOUBLE);
    CData_Allocate(m_idEnergy, nXFrames);
  }

  // create frame length object
  CData_Reset(idWLen, TRUE);
  CData_AddComp(idWLen, "len", T_SHORT);
  CData_Allocate(idWLen, nXFrames);

  // Get input labels & create output label object
  CData_Reset(m_idLabels, TRUE);
  if (dlp_is_symbolic_type_code(idSignal->GetCompType(nLabCompIn))) {
    CData_AddComp(m_idLabels, "lab", idSignal->GetCompType(nLabCompIn));
    CData_Allocate(m_idLabels, nXFrames);
    lpLabIn = (const char **) dlp_malloc(sizeof(const char *)*nXSamples);
    lpLabOut = (char **) dlp_malloc(sizeof(char *)*nXFrames);
    for (i = 0; i < nXSamples; i++)
      lpLabIn[i] = CData_Sfetch(idSignal, i, nLabCompIn);
    for (i = 0; i < nXFrames; i++)
      lpLabOut[i] = (char *) CData_XAddr(m_idLabels, i, 0);
  }

  IFCHECK
  {
    printf("\n\n -----------------------------------");
    if(!idPitch) {
      printf("\n->Pitch asynchronuous analysis mode");
      printf("\n  Output frames             :\t%d",(int)nXFrames);
    } else {
      printf("\n->Pitch synchronuous analysis mode");
      printf("\n  Output frames             :\tunknown");
    }
    printf("\n  Output components         :\t%d",(int)m_nOutDim);
    if(lpLabIn) {
      printf("\n  Label component in input  :\t%d",(int)nLabCompIn);
      printf("\n  Label component in output :\t%d",0);
    } else {
      printf("\n  No label component found");
    }
    if(m_nPreem>0) {
      printf("\n  Preemphasis               :\t%f",(double)m_nPreem);
    } else {
      printf("\n  Preemphasis               :\toff");
    }
    if(m_bTimeDomainWarping==TRUE) {
      printf("\n  Time Domain Warping       :\t%f",m_nPfaLambda);
    } else {
      printf("\n  Time Domain Warping       :\toff");
    }
    printf("\n  Window Type               :\t%s",m_lpsWtype);
    printf("\n -----------------------------------\n\n");
  }

  IF_NOK(dlm_fba_doframing(
      lpSignal, nXSamples,
      lpPitch, nXPitch,
      (FLOAT64 *)CData_XAddr(m_idWindow,0,0),
      lpLabIn, lpLabOut,
      (INT16 *)CData_XAddr(idWLen,0,0),
      (FLOAT64 *)CData_XAddr(idFrames,0,0), nXFrames,
      (m_bEnergy||m_bLogEnergy)?(FLOAT64 *)CData_XAddr(m_idEnergy,0,0):NULL,
          &lpParam
  )) IERROR(this,FBA_ANALYSE,-1,0,0);

  i = 0;
  j = 0;
  if((lpLabIn != NULL) && (lpLabOut != NULL)) {
    while((i < nXSamples) && (j < nXFrames)) {
      if(dlp_strcmp(lpLabIn[i], lpLabOut[j])) {
        bLabelDiffer = TRUE;
        break;
      }
      i++; j++;
      while((i < nXSamples) && !strcmp(lpLabIn[i], lpLabIn[i-1])) i++;
      while((j < nXFrames) && !strcmp(lpLabOut[j], lpLabOut[j-1])) j++;
    }
  }
  if(bLabelDiffer) {
    IERROR(this, FBA_LABEL, 0, 0, 0);
  }

  if(bFreeSignal) dlp_free(lpSignal);
  if(bFreePitch) dlp_free(lpPitch);
  if(lpLabIn) dlp_free(lpLabIn);
  if(lpLabOut) dlp_free(lpLabOut);
  DESTROYVIRTUAL(idSignal,idFrames);

  return TRUE;
}

/**
 * Analyse
 *
 * Derived instances of FBAproc should override method
 * Analyse() to add the desired functionality
 *
 * @return O_K if successfull, NOT_EXEC otherwise
 */
INT16 CGEN_VPUBLIC CFBAproc::AnalyzeFrame() {
  return O_K;
}

/**
 * Analyse
 *
 * Derived instances of FBAproc should override method
 * Analyzematrix() to add the desired functionality
 *
 * @return O_K if successfull, NOT_EXEC otherwise
 */
INT16 CGEN_VPUBLIC CFBAproc::AnalyzeMatrix(CData* idFrames, CData* idReal) {
  return O_K;
}

/**
 * Checks the contents of field <code>delta_weights</code> and repairs it if
 * necessary.
 *
 * @param bSilent If <code>TRUE</code> repair silently, otherwise print a
 *                warning
 * @return <code>O_K</code> if successfull, a (negative) error code otherwise
 */
INT16 CGEN_PROTECTED CFBAproc::DeltaCheckWeights(BOOL bSilent DEFAULT(FALSE)) {
  INT16 bCheck = TRUE;
  INT16 nC = 0;

  if (!m_idDeltaWeights)IFIELD_RESET(CData,"delta_weights");
  if (CData_GetNRecs(m_idDeltaWeights) != 1) bCheck = FALSE;
  if (CData_GetNComps(m_idDeltaWeights) % 2 != 1) bCheck = FALSE;

  for (nC = 0; nC < CData_GetNComps(m_idDeltaWeights) / 2 + 1; nC++)
    if (fabs(CData_Dfetch(m_idDeltaWeights, 0, nC)
        + CData_Dfetch(m_idDeltaWeights, 0, CData_GetNComps(m_idDeltaWeights) - nC - 1)) > 1E-10) bCheck = FALSE;

  if (bCheck) return O_K;

  CData_Array(m_idDeltaWeights, T_DOUBLE,7, 1);
  CData_Dstore(m_idDeltaWeights, -1., 0, 0);
  CData_Dstore(m_idDeltaWeights, 1., 0, 6);
  if (!bSilent) IERROR(this,FBA_BADDELTAWEIGHTS,0,0,0);

  return O_K;
}

/**
 * Compute dynamic features
 *
 * @param  idSrc Input data (fetaure vector sequence)
 * @param  idDst data instance containing analysis result
 * @return O_K if successfull, NOT_EXEC otherwise
 */
INT16 CGEN_PROTECTED CFBAproc::DeltaFBA(CData* idSrc, CData* idDst) {
  CData* idLab = NULL; // Label component(s) buffer
  CData* idAux = NULL; // Auxilary data buffer
  FLOAT64* lpDeltaT = NULL; // Delta weighting vector buffer
  FLOAT64* lpDeltaW = NULL; // Delta weighting vector buffer
  INT32 nDeltaWL = 0; // Delta window length
  INT32 nC = 0; // Current component index
  INT32 nXFrames;
  INT32 nFDim;
  INT32 nVDim;
  INT32 nADim;
  char bNonDouble = 0;

  if (!idDst) return IERROR(this,ERR_NULLARG,"idDst",0,0); // No destination instance
  if (!idSrc || CData_IsEmpty(idSrc)) // No source data
  { // >>
    IERROR(this,ERR_NULLINST,"idSrc",0,0); //   Print warning
    return CData_Reset(idDst, TRUE); //   Reset destination and return
  } // <<

  CREATEVIRTUAL(CData,idSrc,idDst); // Overlapping arguments support
  ICREATEEX(CData,idLab,"CFBAproc::DeltaFBA.idLab",NULL); // Create label components buffer
  ICREATEEX(CData,idAux,"CFBAproc::DeltaFBA.idAux",NULL); // Create an auxilary data instance

  // Initialize destination instance (separate features and labels)             // ------------------------------------
  CData_Copy(idDst, idSrc); // Copy source to destination
  for (nC = 0; nC < CData_GetNComps(idDst);)
    // Loop over destination components
    if (!dlp_is_numeric_type_code(CData_GetCompType(idDst, nC))) //   Current component not numeric
    { //   >> (move it away)
      CData_SelectComps(idAux, idDst, nC, 1); //     Copy component to auxilary inst.
      CData_DeleteComps(idDst, nC, 1); //     Delete component
      CData_Join(idLab, idAux); //     Join component to label instance
    } //   <<
    else //   Current component numeric
    {
      if (CData_GetCompType(idDst, nC) != T_DOUBLE) bNonDouble = 1;
      nC++; //     Fine
    }
  if (bNonDouble) CData_Tconvert(idDst, idDst, T_DOUBLE);
  nFDim = CData_GetNComps(idDst);
  nXFrames = CData_GetNRecs(idDst);

  DeltaCheckWeights(); // Check/repair delta weighting vector
  nDeltaWL = (INT16) (CData_GetNComps(m_idDeltaWeights) / 2); // Get delta window length
  lpDeltaW = (FLOAT64*) dlp_calloc(2*nDeltaWL+1,sizeof(FLOAT64)); // Allocat delta weigts vector
  CData_DrecFetch(m_idDeltaWeights, lpDeltaW, 0, 2 * nDeltaWL + 1, -1); // Get delta weights vector
  lpDeltaT = (FLOAT64*) dlp_calloc(nFDim*2,sizeof(FLOAT64));
  CData_DblockFetch(m_idDeltaTable, lpDeltaT, 0, nFDim, 2, -1);

  // Get memory size for dst object                                             // ------------------------------------
  dlm_fba_deltafba(NULL,lpDeltaT, lpDeltaW, nXFrames, FALSE,nDeltaWL, nFDim, &nVDim, &nADim, 0);
  CData_AddNcomps(idDst, T_DOUBLE,nVDim + nADim);

  // Compute dynamic features                                                   // ------------------------------------
  dlm_fba_deltafba((FLOAT64*) CData_XAddr(idDst, 0, 0), lpDeltaT, lpDeltaW, nXFrames, FALSE,nDeltaWL, nFDim, &nVDim, &nADim, nFDim
      + nVDim + nADim);

  // Finish destination instance                                                // ------------------------------------
  CData_Join(idDst, idLab); // Join labels to destination

  // Clean up                                                                   // ------------------------------------
  dlp_free(lpDeltaT); // Destroy delta table vector buf.
  dlp_free(lpDeltaW); // Destroy delta weighting vector buf.
  IDESTROY(idLab); // Destroy label components buffer
  IDESTROY(idAux); // Destroy axilary data instance;
  DESTROYVIRTUAL(idSrc,idDst); // Overlapping arguments support
  return O_K; // Return error code
}

/**
 * Delta computation function taken from original melfilter implementation
 * written by C.Westendorf
 *
 * @param  idIn Input data (fetaure vector sequence)
 * @param  idOut data instance containing analysis result
 * @return O_K if successfull, NOT_EXEC otherwise
 */
INT16 CGEN_PROTECTED CFBAproc::DeltaMF(CData* dIn, CData* dOut) {
  INT16 bNodelta = FALSE;
  INT16 bLabels = FALSE;
  INT32 i = 0;
  INT32 j = 0;
  INT32 nRec = 0;
  INT32 nComp = 0;
  INT32 nDelay = 0;
  FLOAT64* lpX = NULL;
  FLOAT64* lpXL = NULL;
  FLOAT64* lpXR = NULL;
  FLOAT64* lpD = NULL;
  FLOAT64* lpDD = NULL;
  CData* lpDelta = NULL;
  CData* lpDDelta = NULL;

  // Verify input
  if (!dIn) {
    IERROR(this,ERR_NULLARG,"dIn",0,0);
    return NOT_EXEC;
  }
  if (dIn->IsEmpty()) {
    IERROR(dIn,DATA_EMPTY,dIn->m_lpInstanceName,0,0);
    return NOT_EXEC;
  }

  // Initialize
  CREATEVIRTUAL(CData,dIn,dOut);
  nRec = dIn->GetNRecs();
  nComp = dIn->GetNComps();
  dOut->Copy(dIn);

  // Scan for label component
  if (dlp_is_symbolic_type_code(dIn->GetCompType(nComp - 1))) {
    bLabels = TRUE;
    if (m_idLabels == NULL)ICREATEEX(CData,m_idLabels,"~lab",NULL);
    m_idLabels->Select(dIn, nComp - 1, 1);
    dOut->Delete(dOut, nComp - 1, 1);
    nComp -= 1;
  }

  // Verify delta component lookup table
  if (NOT_EXEC==VerifyDeltaTable(nComp, &bNodelta)) return NOT_EXEC;
  if (bNodelta) {
    dOut->Copy(dIn);
    DESTROYVIRTUAL(dIn,dOut);
    return O_K;
  }

  // Test for homogeneous input of type double
  if (T_DOUBLE!=dOut->IsHomogen()) {
    DESTROYVIRTUAL(dIn,dOut);
    IERROR(this,FBA_BADINPUT,dIn->m_lpInstanceName,0,0);
    return NOT_EXEC;
  }

  // Initialize instances holding deltas and delta-deltas
  nDelay = CData_GetNComps(m_idDeltaWeights) / 2;
  if (nDelay == 0) nDelay = 3;
  ICREATEEX(CData,lpDelta ,"~delta" ,NULL);
  ICREATEEX(CData,lpDDelta,"~ddelta",NULL);
  lpDelta->Copy(dOut);
  lpDelta->Allocate(dOut->GetNRecs());
  lpDDelta->Copy(dOut);
  lpDDelta->Allocate(dOut->GetNRecs());

  // compute delta and delta delta
  // x'(t)  = x(t) - x(t-delay)
  // x''(t) = x'(t+delay) - x'(t)
  //        = x (t+delay) - x (t) - x'(t)
  for (i = 0; i < nRec; i++) {
    lpX = (FLOAT64*) dOut->XAddr(i, 0);
    lpD = (FLOAT64*) lpDelta->XAddr(i, 0);
    lpDD = (FLOAT64*) lpDDelta->XAddr(i, 0);

    if (i >= nDelay) lpXL = (FLOAT64*) dOut->XAddr(i - nDelay, 0);
    else lpXL = (FLOAT64*) dOut->XAddr(0, 0);

    if (i < nRec - nDelay) lpXR = (FLOAT64*) dOut->XAddr(i + nDelay, 0);
    else lpXR = (FLOAT64*) dOut->XAddr(nRec - 1, 0);

    for (j = 0; j < nComp; j++) {
      *lpD = *lpX - *lpXL;
      *lpDD = *lpXR - *lpX - *lpD;
      lpX++;
      lpXL++;
      lpXR++;
      lpD++;
      lpDD++;
    }
  }

  // Join selected components to feature vector
  for (i = m_idDeltaTable->GetNComps() - 1; i >= 0; i--) {
    if (m_idDeltaTable->Dfetch(0, i) == 0) lpDelta->DeleteComps(i, 1);
    if (m_idDeltaTable->Dfetch(1, i) == 0) lpDDelta->DeleteComps(i, 1);
  }

  dOut->Join(lpDelta);
  dOut->Join(lpDDelta);

  if (bLabels == TRUE) dOut->Join(m_idLabels);

  IDESTROY(lpDelta);
  IDESTROY(lpDDelta);
  DESTROYVIRTUAL(dIn,dOut);

  return TRUE;
}

/**
 * Verify delta table.
 *
 * @return O_K if delta table is valid, NOT_EXEC otherwise
 */
INT16 CGEN_PROTECTED CFBAproc::VerifyDeltaTable(INT16 nComp, INT16 *bNodelta) {
  INT16 i = 0;
  INT16 j = 0;

  if (m_idDeltaTable != NULL) {
    if (m_idDeltaTable->GetNRecs() != 2 || m_idDeltaTable->GetNComps() != nComp
        || !dlp_is_numeric_type_code(m_idDeltaTable->GetCompType(0)) || !m_idDeltaTable->IsHomogen()) {
      IERROR(this,FBA_BADDELTATABLE,nComp,0,0);
      return NOT_EXEC;
    }

    for (*bNodelta = TRUE, i = 0; i < 2; i++)
      for (j = 0; j < CData_GetNComps(m_idDeltaTable); j++)
        if (CData_Dfetch(m_idDeltaTable, i, j) != 0.) {
          *bNodelta = FALSE;
          break;
        }
  } else *bNodelta = TRUE;

  return O_K;
}

/**
 * Weightning Window Generation.
 * The window is stored in field m_idWindow.
 *
 * @param  nWlen Window length in samples
 * @return O_K if successful, NOT_EXEC otherwise
 */
INT16 CGEN_PROTECTED CFBAproc::MakeWindow(INT32 nWlen) {

  DLPASSERT(m_idWindow);

  if (dlp_strncmp(dlp_strlwr(m_lpsWtype), "custom", 255)) CData_Allocate(m_idWindow, nWlen);

  IF_NOK(dlm_fba_makewindow((FLOAT64 *)CData_XAddr(m_idWindow,0,0),nWlen,m_lpsWtype,m_nWnorm))return IERROR(this,FBA_BADNAME,m_lpsWtype,"window",0);

  return O_K;
}

/**
 * Initialize internal buffers. Derived classes may overwrite
 * this method.
 */
INT16 CGEN_PROTECTED CFBAproc::InitBuffers() {
  if(!m_idWindow) {
    if (m_nWlen > m_nLen) m_nWlen = m_nLen;
    IFIELD_RESET(CData,"window" );
    m_idWindow->AddComp("w", T_DOUBLE);
    MakeWindow(m_nWlen);
  }

  if(!m_idRealFrame) {
    IFIELD_RESET(CData,"real_frame");
    m_idRealFrame->AddNcomps(T_DOUBLE,m_nLen);
    m_idRealFrame->Allocate(1);
  }

  if(!m_idImagFrame) {
    IFIELD_RESET(CData,"imag_frame");
    m_idImagFrame->AddNcomps(T_DOUBLE,m_nLen);
    m_idImagFrame->Allocate(1);
  }

  if (m_nCrate > m_nLen) IERROR(this, FBA_CRATE, m_nCrate, m_nLen, NULL);

  return O_K;
}

/**
 * Prepare output instance for analysis. Derived classes may overwrite
 * this method.
 */
void CGEN_VPUBLIC CFBAproc::PrepareOutput(CData* dResult) {
  if (dResult) {
    dResult->Reset();
    dResult->AddNcomps(T_DOUBLE,m_nOutDim);

    // Set record increment and span width in [msec]
    ISETFIELD_RVALUE(dResult,"rinc",1000*m_nCrate/m_nSrate);
    ISETFIELD_RVALUE(dResult,"rwid",1000*m_nWlen/m_nSrate);
  }
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

INT16 CGEN_PUBLIC CFBAproc::Warp(CData* dIn, CData* dOut, FLOAT64 nLambda, INT32 nDim) {
  INT16 i;

  if (dIn == NULL) return IERROR(this,ERR_NULLARG,0,0,0);
  if (dOut == NULL) return IERROR(this,ERR_NULLARG,0,0,0);
  if (dIn->IsEmpty()) return IERROR(dIn ,DATA_EMPTY,dIn->m_lpInstanceName,0,0);
  if (dIn->GetCompType(0) != T_DOUBLE) return IERROR(dIn,DATA_BADCOMPTYPE,dIn->m_lpInstanceName,"FLOAT64",0);

  CREATEVIRTUAL(CData,dIn,dOut);
  dOut->Reset(TRUE);
  ISETFIELD_LVALUE(this,"pfa_lambda",nLambda);
  m_nOutDim = (INT16) nDim;
  //DLPASSERT(FALSE);

  for (i = 0; i < dIn->GetNRecs(); i++) {
    m_idRealFrame->SelectRecs(dIn, i, 1);
    WARP();
    dOut->Cat(m_idRealFrame);
  }

  DESTROYVIRTUAL(dIn,dOut);

  return O_K;
}

/**
 * Calculate warped magnitude spectrum from magnitude.
 *
 * @return O_K if successfull, NOT_EXEC otherwise
 */
INT16 CGEN_PROTECTED CFBAproc::WARP() {
  INT32 nLen2 = m_nLen / 2;
  INT16 i = 0;
  INT16 j = 0;
  FLOAT64 *warp = NULL;
  FLOAT64 *real_in = NULL;
  FLOAT64 *imag_in = NULL;
  FLOAT64 *real_out = NULL;
  FLOAT64 *imag_out = NULL;

  if (m_nPfaLambda == 0.0) return O_K;

  if ((m_idWarp == NULL) || (m_idWarp->GetNRecs() != nLen2)) {
    GenBilinearWarpMatrix(m_nPfaLambda, nLen2);
  }

  real_in = (FLOAT64*) m_idRealFrame->XAddr(0, 0);
  imag_in = (FLOAT64*) m_idImagFrame->XAddr(0, 0);
  warp = (FLOAT64*) m_idWarp->XAddr(0, 0);

  real_out = (FLOAT64*) dlp_calloc(2*m_nLen, sizeof(FLOAT64));
  imag_out = real_out + m_nLen;

  for (i = 0; i < nLen2; i++) {
    j = (INT16) warp[0];
    real_out[i] = warp[1] * real_in[j] + warp[2] * real_in[j + 1];
    real_out[m_nLen - 1 - i] = real_out[i];
    imag_out[i] = warp[1] * imag_in[j] + warp[2] * imag_in[j + 1];
    imag_out[m_nLen - 1 - i] = imag_out[i];
    warp += 3;
  }

  dlp_memmove(real_in, real_out, m_nLen * sizeof(FLOAT64));
  dlp_memmove(imag_in, imag_out, m_nLen * sizeof(FLOAT64));

  dlp_free(real_out);
  return O_K;
}

/**
 * Calculation of transfomation matrix for bilinear transform.
 *
 * @param  nLambda Warping constant a
 * @param  nDim    Dimension of magnitude spectrum to warp
 * @return O_K
 */
INT16 CGEN_PROTECTED CFBAproc::GenBilinearWarpMatrix(FLOAT64 nLambda, INT32 nDim) {
  INT32 i = 0;
  INT32 c1 = 0;
  FLOAT64 c = 0.0;
  FLOAT64 v1 = 0.0;
  FLOAT64 v2 = 0.0;
  FLOAT64 w1 = 0.0;
  FLOAT64 w2 = 0.0;
  CData *lpWM = NULL;

  IFCHECK printf("\n Create warping matrix.");
  ICREATEEX(CData,lpWM,"~warp",NULL);
  lpWM->AddNcomps(T_DOUBLE,3);
  lpWM->Allocate(nDim);
  for (i = 0; i < nDim; i++) {
    w1 = (FLOAT64) i / (FLOAT64) (nDim - 1) * (FLOAT64) F_PI;
    w2 = w1 + 2.0 * atan2(-nLambda * sin(w1), 1 + nLambda * cos(w1));
    c = w2 / F_PI * (FLOAT64) (nDim - 1);
    c1 = (INT32) c;
    v2 = c - c1; // linear interpolation between left and right value
    v1 = 1 - v2;
    lpWM->Dstore(c1, i, 0);
    lpWM->Dstore(v1, i, 1);
    lpWM->Dstore(v2, i, 2);
  }

  ISETFIELD_LVALUE(this,"warp",lpWM);

  return O_K;
}

INT16 CGEN_VPUBLIC CFBAproc::Smooth(data* idIn, data* idPm, data* idOut) {
  if(m_bAnaSmoothRoots || m_bSynSmoothRoots) {
    if(!dlp_strcmp(this->m_lpClassName, "LPCproc") ||
       !dlp_strcmp(this->m_lpClassName, "CPproc")) {
      return SmoothRoots(idIn, idPm, idOut);
    }
    return IERROR(this,ERR_NOTSUPPORTED,"Calculating roots from this kind of features",0,0);
  }
  else if(m_bAnaSmoothFea || m_bSynSmoothFea) {
    return SmoothFea(idIn, idPm, idOut);
  }
  return O_K;
}

INT16 CGEN_PROTECTED CFBAproc::SmoothRoots(data* idIn, data* idPm, data* idOut) {
  INT32   iComp       = 0;
  INT32   nComp       = 0;
  INT32   iRec        = 0;
  INT32   nRec        = 0;
  CData* idRoots      = NULL;
  CData* idVuV        = NULL;

  if (!idIn) {
    IERROR(this,ERR_NULLARG,"dIn",0,0);
    return NOT_EXEC;
  }
  if (idIn->IsEmpty()) {
    IERROR(idIn,DATA_EMPTY,idIn->m_lpInstanceName,0,0);
    return NOT_EXEC;
  }

  CREATEVIRTUAL(CData, idIn, idOut);
  idOut->Copy(idIn);

  ICREATEEX(CData, idRoots, "idRoots",  0);
  ICREATEEX(CData, idVuV,   "idVuV", 0);
  if(idPm != NULL) idVuV->Copy(idPm);

  nRec = idOut->GetNRecs();
  nComp = idOut->GetNComps();

  RootsTrackImpl(idIn, idRoots, idVuV);

  SmoothFea(idRoots, idVuV, idRoots);

  for(iRec=0; iRec<nRec; iRec++) {
    dlm_polyC((COMPLEX64*)idRoots->XAddr(iRec,1), nComp-1, (COMPLEX64*)idOut->XAddr(iRec,0));
    for(iComp=0; iComp<nComp; iComp++) {
      *((COMPLEX64*)idOut->XAddr(iRec,iComp)) = CMPLX_MULT(*((COMPLEX64*)idOut->XAddr(iRec,iComp)),*((COMPLEX64*)idRoots->XAddr(iRec,0)));
    }
  }

  IDESTROY(idRoots);
  IDESTROY(idVuV);

  DESTROYVIRTUAL(idIn,idOut);

  return O_K;
}

INT16 CGEN_PROTECTED CFBAproc::SmoothFea(data* idIn, data* idPm, data* idOut) {
  INT32 iComp       = 0;
  INT32 nComp       = 0;
  INT32 iRec        = 0;
  INT32 nRec        = 0;
  INT32 nCompVuv    = 0;
  FLOAT64 val       = 0.0;
  FLOAT64* pFeat    = 0;
  FLOAT64 filt_b[2] = { 0.29289322, 0.29289322 };
  FLOAT64 filt_a[2] = { 1.00000000, -0.41421356 };

  if (!idIn) {
    IERROR(this,ERR_NULLARG,"dIn",0,0);
    return NOT_EXEC;
  }
  if (idIn->IsEmpty()) {
    IERROR(idIn,DATA_EMPTY,idIn->m_lpInstanceName,0,0);
    return NOT_EXEC;
  }

  CREATEVIRTUAL(CData, idIn, idOut);
  idOut->Copy(idIn);

  nRec = idOut->GetNRecs();
  nComp = idOut->GetNComps();

  if (nRec > 1) {

    // Scan for label component
    if (dlp_is_symbolic_type_code(idOut->GetCompType(nComp - 1))) nComp -= 1;

    if (idPm == NULL) {

      pFeat = (FLOAT64*) dlp_calloc(nRec + 1, sizeof(FLOAT64));
      for (iComp = 0; iComp < nComp; iComp++) {
        for (iRec = 0; iRec < nRec; iRec++) {
          pFeat[iRec] = idOut->Dfetch(iRec, iComp);
        }
        pFeat[iRec++] = pFeat[nRec - 1];
        dlm_filter(filt_b, 2, filt_a, 2, pFeat, pFeat, nRec + 1, NULL, 0);
        for (iRec = 0; iRec < nRec; iRec++) {
          idOut->Dstore(pFeat[iRec + 1], iRec, iComp);
        }
      }
      dlp_free(pFeat);

    } else {

      if (idPm->IsEmpty()) {
        IERROR(idIn,DATA_EMPTY,idIn->m_lpInstanceName,0,0);
        return NOT_EXEC;
      }
      nCompVuv = idPm->FindComp("v/uv");
      if (nCompVuv == NOT_EXEC) {
        IERROR(idPm, DATA_NOTFOUND, "v/uv component in pitch mark data instance",0,0);
        return NOT_EXEC;
      }

      for (iComp = 0; iComp < nComp; iComp++) {
        for (iRec = 1; iRec < nRec; iRec++) {
          if ((idPm->Dfetch(iRec, nCompVuv) != 0) && (idPm->Dfetch(iRec - 1, nCompVuv) != 0)) {
            val = 0.29289322 * (idIn->Dfetch(iRec - 1, iComp) + idIn->Dfetch(iRec, iComp)) + 0.41421356
                * idOut->Dfetch(iRec - 1, iComp);
            idOut->Dstore(val, iRec, iComp);
          }
        }
      }
    }
  }

  DESTROYVIRTUAL(idIn, idOut);

  return TRUE;
}

INT16 CGEN_VPROTECTED CFBAproc::RootsTrack(CData* idFea, CData* idRootsReal, CData* idRootsImag, CData* idVUV) {
  return IERROR(this, ERR_NOTSUPPORTED, "Calculating roots by this analysis type", 0, 0);
}

INT16 CGEN_PROTECTED CFBAproc::RootsTrackImpl(CData* idFea, CData* idRoots, CData* idVUV) {
  INT32    iRecs         = 0;
  INT32    nRecs         = 0;
  INT32    iComps        = 0;
  INT32    nRoots        = 0;
  INT16   nIter         = 1000;
  FLOAT64  eps           = 1.0e-10;
  COMPLEX64* rootsInit = NULL;
  COMPLEX64* polyInit      = NULL;
  CData* idFeaC          = NULL;

  if(!idFea || !idRoots) return NOT_EXEC;
  if((idFea == idRoots) || (idFea==idVUV) || (idRoots==idVUV))
    return IERROR(this, ERR_GENERIC, "Duplicate arguments", NULL, NULL);
  if(idFea->IsEmpty() || (idFea->GetNComps()<2)) return NOT_EXEC;

  ICREATEEX(CData,idFeaC,"idFeaC",NULL);
  CData_Copy(idFeaC,idFea);
  CData_Tconvert(idFeaC,idFea,T_COMPLEX);

  nRecs = idFeaC->GetNRecs();
  nRoots = idFeaC->GetNComps()-1;

  if(idVUV != NULL) {
    if(idVUV->GetNRecs()!=nRecs) {
      idVUV->Reset();
      idVUV->AddComp("v/uv", T_SHORT);
      idVUV->Allocate(nRecs);
    }
    for(iRecs=0; iRecs<nRecs; iRecs++) {
      idVUV->Dstore(1.0, iRecs, idVUV->FindComp("v/uv"));
    }
  }
  rootsInit = (COMPLEX64*)dlp_calloc(2*nRoots+1, sizeof(COMPLEX64));
  polyInit  = rootsInit + nRoots;

  if(idRoots->IsEmpty() || (idRoots->GetNComps()!=nRoots)) {
    dlm_roots((FLOAT64*)idFea->XAddr(0,0), rootsInit, nRoots+1);
  } else {
    dlp_memmove(rootsInit, idRoots->XAddr(0,0), nRoots*sizeof(COMPLEX64));
    idRoots->Reset();
  }
  idRoots->AddNcomps(T_COMPLEX,nRoots+1);
  idRoots->Allocate(nRecs);

  dlm_polyC(rootsInit, nRoots, polyInit);
  for(iComps=0; iComps<=nRoots; iComps++) {
    polyInit[iComps] = CMPLX_MULT(polyInit[iComps],idFeaC->Cfetch(0,0));
  }

  if(dlm_roots_track_match_poly(polyInit, rootsInit, (COMPLEX64*)idFeaC->XAddr(0,0), (COMPLEX64*)idRoots->XAddr(0,1), nRoots, nIter, eps) != O_K) {
    dlp_memmove(idRoots->XAddr(0,1), rootsInit, nRoots*sizeof(COMPLEX64));
  }
  idRoots->Cstore(idFeaC->Cfetch(0,0),0,0);
  if(idVUV) *((INT16*)idVUV->XAddr(0,0)) = 0;
  for(iRecs=1; iRecs<nRecs; iRecs++) {
    if(dlm_roots_track_match_poly((COMPLEX64*)idFeaC->XAddr(iRecs-1,0), (COMPLEX64*)idRoots->XAddr(iRecs-1,1), (COMPLEX64*)idFeaC->XAddr(iRecs,0), (COMPLEX64*)idRoots->XAddr(iRecs,1), nRoots, nIter, eps) > 1.0) {
      IERROR(this, FBA_ANALYSE, iRecs, 0, 0);
      dlm_roots((FLOAT64*)idFea->XAddr(iRecs,0), (COMPLEX64*)idRoots->XAddr(iRecs,1), nRoots+1);
      if(idVUV) *((INT16*)idVUV->XAddr(iRecs,0)) = 0;
    }
    idRoots->Cstore(idFeaC->Cfetch(iRecs,0),iRecs,0);
  }

  dlp_free(rootsInit);
  return O_K;
}

INT16 CGEN_PUBLIC CFBAproc::Poly(CData* idRootsReal, CData* idRootsImag, CData* idPoly) {
  INT32    iRecs         = 0;
  INT32    nRecs         = 0;
  INT32    nRoots        = 0;
  INT32    iComps        = 0;
  INT32    nComps        = 0;
  FLOAT64  gain          = 0.0;
  FLOAT64* pPoly         = NULL;

  if(!idPoly || !idRootsReal || !idRootsImag) return NOT_EXEC;
  if(idRootsReal->IsEmpty() || idRootsImag->IsEmpty()) return NOT_EXEC;

  nRecs = idRootsReal->GetNRecs();
  nComps = idRootsReal->GetNComps();
  nRoots = nComps - 1;

  if((nRecs != idRootsImag->GetNRecs()) || (nRoots != idRootsImag->GetNComps()-1)) return NOT_EXEC;

  idPoly->Reset();
  idPoly->AddNcomps(T_DOUBLE, nRoots+1);
  idPoly->Allocate(nRecs);

  for(iRecs=0; iRecs<nRecs; iRecs++) {
    if(dlm_poly((FLOAT64*)idRootsReal->XAddr(iRecs,1), (FLOAT64*)idRootsReal->XAddr(iRecs,1), nRoots, (FLOAT64*)idPoly->XAddr(iRecs,0), NULL) != O_K) {
      return IERROR(this, FBA_ANALYSE, iRecs, 0, 0);
    }
    pPoly = (FLOAT64*)idPoly->XAddr(iRecs,0);
    gain  = idRootsReal->Dfetch(iRecs,0);
    for(iComps=0; iComps<nComps; iComps++) pPoly[iComps] *= gain;
  }
  return O_K;
}

INT16 CGEN_VPROTECTED CFBAproc::OnPfaLambdaChangedImpl() {
  if (m_idWarp != NULL)IDESTROY(m_idWarp);
  return GenBilinearWarpMatrix(m_nPfaLambda, m_nLen / 2);
}

// EOF
