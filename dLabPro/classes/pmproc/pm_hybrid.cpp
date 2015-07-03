// dLabPro class CPMproc (PMproc)
// - Class CPMproc - HYBRID code
//
// AUTHOR : Hussein Hussein, Dresden
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


#include "dlp_pmproc.h"        // Include class header file
#include "dlp_fst.h"
#include "dlp_file.h"

INT16 CGEN_PRIVATE CPMproc::Hybrid(data *dSignal, data *dPM) {
  /* Initialization */
  INT32 i = 0;
  INT32 j = 0;
  INT32 jj = 0;
  INT32 nSrate = 0; // Sample Rate
  INT32 nSamples = 0; // Number of samples in WAV-signal (number of records in data instance)
  FLOAT64 *samples = NULL; // WAV Signal in double
  INT32 nSmoothWindow = 20; // Length of Window for smoothing the speech signal (Number of samples)
  INT32 nEnergyWindow = 160; // Length of Window for calculating Energy
  FLOAT64 *dSmoothSignal = NULL; // Smoothed Signal
  FLOAT64 *dEnergyContour = NULL; // Short Time Energy Contour
  FLOAT64 nMeanEnergy = 0; // The Mean of Energy Contour
  FLOAT64 nThresholdEnergy = 0; // Threshold for Energy Contour


  /* Validation of data instance dPM */
  if(!dPM || !dPM->IsEmpty()) // If dPM not exist or empty then return false
  return NOT_EXEC; // NOT_EXEC = -1 (Generic error)

  /* Definition of data instance dPM (The result of HYBRID-Algorithm) */
  // PM format of TU Dresden (first column is the number of samples between two PM, second column is Anregung)

  dPM->AddNcomps(T_SHORT, 2);
  dPM->SetCname(0, "pm");
  dPM->SetCname(1, "v/uv");
  ISETFIELD_RVALUE(dPM,"fsr", 1000.0/m_nSrate);

  /* Compute Sample Rate (SR) */
  nSrate = m_nSrate;
  //fprintf(stdout,"Sample Rate:   %d Hz\n\n",nSrate);

  /* Number of samples in WAV-signal */
  nSamples = dSignal->GetNRecs(); // GetNRecs returns the number of valid records in the data instance
  //fprintf(stdout,"Number of samples in WAV-signal:   %i\n\n",nSamples);


  /* Copies data from memory area to another memory area */
  samples = (FLOAT64*)dlp_calloc(nSamples, sizeof(FLOAT64)); // dlp_calloc(NUM,SIZE) Allocates zero-initialize memory
  for(i=0; i<nSamples; i++) {
    samples[i] = dSignal->Dfetch(i,0);
  }

  /*
   fprintf(stdout,"\n The samples of speech signal are: \n");  // show data
   for(i=0; i<nSamples; i++)
   fprintf(stdout,"Signal Sample %i = %f\n",i,samples[i]);
   */

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////// Norming the signal of (1) ////////////////////////////////


  /*
   FLOAT64 *dNormalizedSig = NULL;  // Normalized Signal
   // Convert Signal (dSignal)from INT16 [using (-import) to read the speech signal] to double (dSignalDouble) //
   CData  *dSignalDouble = NULL; // Initialization
   ICREATEEX(CData,dSignalDouble,"CPMproc::Hybrid.dSignalDouble",NULL); //Create buffer for signal in type bouble
   CData_Array(dSignalDouble,T_DOUBLE,1,nSamples); //Resets the instance, defines nComps componentes of type nCType, allocates memory for nRecs records

   CData_Tconvert(dSignal,dSignalDouble,T_DOUBLE); //Copies and convert numeric components



   // Norming the signal //
   FLOAT64 nSigMax = 0;       // Maximum value of the signal
   FLOAT64 nSigMin = 0;       // Minimum value of the signal
   FLOAT64 MinAbsolut = 0;       // Absolute Value of Minimum
   FLOAT64 nAuxMax = 0;       // Auxiliary variable of Maximum
   FLOAT64 nAuxDfetch = 0;

   // Search Maximum Value of Signal
   for (i = 0; i < nSamples; i++)
   {
   nAuxDfetch = CData_Dfetch(dSignalDouble,i,0);
   if ( nSigMax < nAuxDfetch )
   nSigMax = nAuxDfetch;
   }


   // Search Minimum Value of Signal
   for (i = 0; i < nSamples; i++)
   {
   if ( nSigMin > CData_Dfetch(dSignalDouble,i,0) )
   nSigMin = CData_Dfetch(dSignalDouble,i,0);
   }


   MinAbsolut = -(nSigMin); // Absolute Value of Minimum

   nAuxMax = nSigMax; // Auxiliary Variable

   if (MinAbsolut >= nAuxMax)
   nAuxMax = MinAbsolut; // Search for maximum (from negative and positive values)




   CData  *dSignalNormalized = NULL;  // Initialization
   ICREATEEX(CData,dSignalNormalized,"CPMproc::Hybrid.dSignalNormalized",NULL); //Create buffer for signal in type bouble
   CData_Array(dSignalNormalized,T_DOUBLE,1,nSamples); // CData::Array(INT16 nCType, INT32 nComps, INT32 nRecs);


   FLOAT64 nAuxCopy = 0;

   for (i = 0; i < nSamples; i++)
   {
   nAuxCopy = CData_Dfetch(dSignalDouble,i,0) / nAuxMax;
   CData_Dstore(dSignalNormalized,nAuxCopy,i,0);
   }

   IDESTROY(dSignalDouble);    // Destroy dSignalDouble
   IDESTROY(dSignalNormalized);  // Destroy dSignalNormalized

   // End of Norming //
   */

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////// Calculate PM from CHFA (PMA1) and GCIDA (PMA2) ///////////////////////

  /* CHFA - PMA1 */
  //fprintf(stdout,"Calculate PM from CHFA ...\n");
  CData *idPM1 = NULL; // Buffer for PM of CHFA (PMA1)
  ICREATEEX(CData,idPM1,"CPMproc::Hybrid.idPM1",NULL); // Create CHFA-PM buffer

  m_bChfa = true; //  Set /chfa option
  Analyze(dSignal, idPM1); // Calculate PM with CHFA
  m_bChfa = false; // Clear /chfa option


  /* GCIDA - PMA2 */
  //fprintf(stdout,"Calculate PM from GCIDA ...\n");
  CData *idPM2 = NULL; // Buffer for PM of GCIDA (PMA2)
  ICREATEEX(CData,idPM2,"CPMproc::Hybrid.idPM2",NULL); // Create GCIDA-PM buffer


  m_bGcida = true; //  Set /gcida option or ( ISETOPTION(idPM2,"/gcida"); )
  Analyze(dSignal, idPM2); // Calculate PM with GCIDA
  m_bGcida = false; // Clear /gcida option ( IRESETOPTIONS(idPM2); )


  DEBUGMSG(-1,"\nCalculate Hybrid PM ...",0,0,0);

  /* Convert PM data instance (2 column in the instance [nPer][anreg]) to two separate columns */
  /*-----------------------------------------------------------------------
   Important Information:
   The output of the method (-analyze) for CHFA and GCIDA is a PM data instance of type short.
   See the definition in (pm_chfa.cpp) and (pm_gcida.cpp)
   nPer  : is the distance between PM in samples
   anreg : is the Anregung [ - (voiceless or stimmlos) , + (voiced or stimmhaft)]
   *** Important ***
   * The data object of PMproc class or data objects with more than one component
   * were stored in memory (record after record) (the first line, second line, third line, ...)
   * i.e. [r,c] = {[0,0],[0,1],[1,0],[1,1],[2,0],[2,1],....}
   ---------------------------------------------------------------------*/
  INT16 *idPM1nPer = NULL; // PMA1 [nPer] = column 1
  INT16 *idPM1Anreg = NULL; // PMA1 [anreg] = column 2

  INT16 *idPM2nPer = NULL; // PMA2 [nPer] = column 1
  INT16 *idPM2Anreg = NULL; // PMA2 [anreg] = column 2

  INT32 nRecIdPM1 = 0; // Number of PM from CHFA
  INT32 nRecIdPM2 = 0; // Number of PM from GCIDA

  nRecIdPM1 = idPM1->GetNRecs(); // GetNRecs returns the number of valid records in data instance (idPM1)
  nRecIdPM2 = idPM2->GetNRecs(); // GetNRecs returns the number of valid records in data instance (idPM2)


  idPM1nPer = (INT16*)dlp_calloc(nRecIdPM1, sizeof(INT16)); // Allocates zero-initialize memory
  idPM1Anreg = (INT16*)dlp_calloc(nRecIdPM1, sizeof(INT16)); // Allocates zero-initialize memory

  idPM2nPer = (INT16*)dlp_calloc(nRecIdPM2, sizeof(INT16)); // Allocates zero-initialize memory
  idPM2Anreg = (INT16*)dlp_calloc(nRecIdPM2, sizeof(INT16)); // Allocates zero-initialize memory


  FLOAT64 dValue = 0; //Double Value

  // Copy PMA1
  for(i = 0; i < nRecIdPM1; i++) {
    dValue = CData_Dfetch(idPM1, i, 0);
    idPM1nPer[i] = (INT16)dValue;
    //fprintf(stdout,"Double Value = %f\t",dValue);
    //fprintf(stdout,"CHFA-nPer %i = %i\n",i,idPM1nPer[i]);

    dValue = CData_Dfetch(idPM1, i, 1);
    idPM1Anreg[i] = (INT16)dValue;
    //fprintf(stdout,"Double Value = %f\t",dValue);
    //fprintf(stdout,"CHFA-Anreg %i = %i\n",i,idPM1Anreg[i]);
  }

  // Copy PMA2
  for(i = 0; i < nRecIdPM2; i++) {
    dValue = CData_Dfetch(idPM2, i, 0);
    idPM2nPer[i] = (INT16)dValue;
    //fprintf(stdout,"Double Value = %f\t",dValue);
    //fprintf(stdout,"GCIDA-nPer %i = %i\n",i,idPM2nPer[i]);

    dValue = CData_Dfetch(idPM2, i, 1);
    //sValue = (INT16)dValue;
    idPM2Anreg[i] = (INT16)dValue;
    //fprintf(stdout,"Double Value = %f\t",dValue);
    //fprintf(stdout,"GCIDA-Anreg %i = %i\n",i,idPM2Anreg[i]);
  }

  /* Convert PM in column 1 [nPer] to sum of samples from the beginning (0) */
  INT32 *idPM1nPerSumSamples = NULL; // PM from CHFA in samples [ sum of samples from (0) ]
  INT32 *idPM2nPerSumSamples = NULL; // PM from GCIDA in samples [ sum of samples from (0) ]


  idPM1nPerSumSamples = (INT32*)dlp_calloc(nRecIdPM1, sizeof(INT32)); // Allocates zero-initialize memory
  idPM2nPerSumSamples = (INT32*)dlp_calloc(nRecIdPM2, sizeof(INT32)); // Allocates zero-initialize memory


  PMConvertSumSamples(idPM1nPer, nRecIdPM1, idPM1nPerSumSamples); // PM1 as sum of samples from (0)
  PMConvertSumSamples(idPM2nPer, nRecIdPM2, idPM2nPerSumSamples); // PM2 as sum of samples from (0)


  /* Extract Voiced PM */
  INT32 nPM1Voiced = 0; // Number of voiced PM1
  INT32 nPM2Voiced = 0; // Number of voiced PM2

  INT32 *idPM1VoicedSumSamples = NULL; // PM from CHFA in voiced segments as sum of samples from (0)
  INT32 *idPM2VoicedSumSamples = NULL; // PM from GCIDA in voiced segments as sum of samples from (0)


  idPM1VoicedSumSamples = (INT32*)dlp_calloc(nRecIdPM1, sizeof(INT32)); // length < (nRecIdPM1)
  idPM2VoicedSumSamples = (INT32*)dlp_calloc(nRecIdPM2, sizeof(INT32)); // length < (nRecIdPM2)


  nPM1Voiced = ExtractVoicedPM(idPM1nPerSumSamples, idPM1Anreg, nRecIdPM1, idPM1VoicedSumSamples);
  nPM2Voiced = ExtractVoicedPM(idPM2nPerSumSamples, idPM2Anreg, nRecIdPM2, idPM2VoicedSumSamples);

  idPM1VoicedSumSamples = (INT32*)dlp_realloc(idPM1VoicedSumSamples, (nPM1Voiced), sizeof(INT32));
  idPM2VoicedSumSamples = (INT32*)dlp_realloc(idPM2VoicedSumSamples, (nPM2Voiced), sizeof(INT32));

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////// Hybrid Algorithm for Pitch Marking ///////////////////////////////

  /*-----------------------------------------------------------
   ------------ Preprocessing - Vorverarbeitung --------------
   ------------------------------------------------------------*/

  /* Smooth the signal and calculate Short Time Energy Contour */
  //SignalNorming(samples, nSamples, dNormalizedSig); // Normalized Signal (double)


  dSmoothSignal = (FLOAT64*)dlp_calloc(nSamples, sizeof(FLOAT64));
  dEnergyContour = (FLOAT64*)dlp_calloc(nSamples, sizeof(FLOAT64));

  MovingAverage(samples, nSamples, nSmoothWindow, dSmoothSignal); // Smoothing the speech signal with Moving Average (Gleitende Mittelung)
  ShortTimeEnergy(dSmoothSignal, nSamples, nEnergyWindow, dEnergyContour); // Calculate the Short Time Energy Contour

  nMeanEnergy = MeanValue(dEnergyContour, nSamples); // The Mean of Energy Contour
  nThresholdEnergy = 0.02 * nMeanEnergy; // Threshold for Energy Contour (Threshold = 2% from Mean of Energy)


  //INT16 rSmoothing;      // Returned value for MovingAverage-function
  //rSmoothing = MovingAverage(samples, nSamples, nSmoothWindow, dSmoothSignal);  // Smoothing the speech signal with Moving Average (Gleitende Mittelung)
  //if (rSmoothing == 1)
  //{
  //  IERROR(this,PM_HYBRID_MOVINGAVERAGE,0,0,0);
  //  return NOT_EXEC;
  //}

  //INT16 rEnergy;  // Returned value for ShortTimeEnergy-function
  //rEnergy = ShortTimeEnergy(dSmoothSignal, nSamples, nEnergyWindow, dEnergyContour);     // Calculate the Short Time Energy Contour
  //if (rEnergy == 1)
  //{
  //  IERROR(this,PM_HYBRID_STEC,0,0,0);
  //  return NOT_EXEC;
  //}


  /*  Delete PM that have an energy unter the threshold of Short Time Energy Contour
   i.e. Delete the PM in voiceless segments  */

  INT32 *dPM1VoicedEner = NULL; // Voiced PM from CHFA with higher energy [samples from (0)]
  INT32 *dPM2VoicedEner = NULL; // Voiced PM from GCIDA with higher energy [samples from (0)]

  dPM1VoicedEner = (INT32*)dlp_calloc(nPM1Voiced, sizeof(INT32)); // length < (nPM1Voiced)
  dPM2VoicedEner = (INT32*)dlp_calloc(nPM2Voiced, sizeof(INT32)); // length < (nPM2Voiced)

  INT32 nPM1VoicedEner = 0; // Number of voiced PM1 with higher energy
  INT32 nPM2VoicedEner = 0; // Number of voiced PM2 with higher energy


  nPM1VoicedEner = DeleteSmallEnergyPM(dEnergyContour, nSamples, idPM1VoicedSumSamples, nPM1Voiced, nThresholdEnergy,
      dPM1VoicedEner); // Delete PM-CHFA with small energy
  nPM2VoicedEner = DeleteSmallEnergyPM(dEnergyContour, nSamples, idPM2VoicedSumSamples, nPM2Voiced, nThresholdEnergy,
      dPM2VoicedEner); // Delete PM-GCIDA with small energy

  dPM1VoicedEner = (INT32*)dlp_realloc(dPM1VoicedEner, (nPM1VoicedEner), sizeof(INT32));
  dPM2VoicedEner = (INT32*)dlp_realloc(dPM2VoicedEner, (nPM2VoicedEner), sizeof(INT32));

  /*
   fprintf(stdout,"\n The voiced PM: \n");  // show data
   for(i=0; i<nPM1VoicedEner; i++)
   fprintf(stdout,"voiced PM  %i = %i\n",i,dPM1VoicedEner[i]);
   */

  /*----------------------------------------------------------------
   -------------- Alignment Module - Anpassung Modul --------------
   ----------------------------------------------------------------*/

  /* Alignment (Shift) the PM to the nearest negative point of signal (local minimum) in domain [-16, +16 sample] = [-1, + 1 msec]*/
  FLOAT64 nThresholdLowEnergy = 0; // Threshold for Energy Contour for low energy segments (Begin and End of voiced segments)
  nThresholdLowEnergy = 0.10 * nMeanEnergy; // Threshold = 10% from Mean of Energy

  INT16 SearchDomain = 0; // Search domain for the nearest negative value of signal [(n) ist eine gerade Anzahl]
  SearchDomain = 16; // Search domain  (e.g.: [-12, +12 samples] = 0.75 msec)

  INT32 *dPM1VoicedShift = NULL; // Voiced PM from CHFA with higher energy and with alignment (shift) to the nearest negative value of signal [samples from (0)]
  INT32 *dPM2VoicedShift = NULL; // Voiced PM from GCIDA with higher energy and with alignment (shift) to the nearest negative value of signal [samples from (0)]


  INT16 *dPM1DistanceMinPM = NULL; // Distance between PM1 and nearest negative point of signal (local minimum) [Distance in samples]
  INT16 *dPM2DistanceMinPM = NULL; // Distance between PM2 and nearest negative point of signal (local minimum) [Distance in samples]


  dPM1VoicedShift = (INT32*)dlp_calloc(nPM1VoicedEner, sizeof(INT32)); // length < nPM1VoicedEner
  dPM2VoicedShift = (INT32*)dlp_calloc(nPM2VoicedEner, sizeof(INT32)); // length < nPM2VoicedEner

  dPM1DistanceMinPM = (INT16*)dlp_calloc(nPM1VoicedEner, sizeof(INT16)); // length < nPM1VoicedEner
  dPM2DistanceMinPM = (INT16*)dlp_calloc(nPM2VoicedEner, sizeof(INT16)); // length < nPM2VoicedEner

  INT32 nPM1VoicedEnerNew = 0;
  INT32 nPM2VoicedEnerNew = 0;

  nPM1VoicedEnerNew = ShiftPMnearestNegativePeak(samples, nSamples, dPM1VoicedEner, nPM1VoicedEner, dEnergyContour,
      nThresholdLowEnergy, SearchDomain, dPM1VoicedShift, dPM1DistanceMinPM);
  nPM2VoicedEnerNew = ShiftPMnearestNegativePeak(samples, nSamples, dPM2VoicedEner, nPM2VoicedEner, dEnergyContour,
      nThresholdLowEnergy, SearchDomain, dPM2VoicedShift, dPM2DistanceMinPM);

  dPM1VoicedShift = (INT32*)dlp_realloc(dPM1VoicedShift, (nPM1VoicedEnerNew), sizeof(INT32));
  dPM2VoicedShift = (INT32*)dlp_realloc(dPM2VoicedShift, (nPM2VoicedEnerNew), sizeof(INT32));

  dPM1DistanceMinPM = (INT16*)dlp_realloc(dPM1DistanceMinPM, (nPM1VoicedEnerNew), sizeof(INT16));
  dPM2DistanceMinPM = (INT16*)dlp_realloc(dPM2DistanceMinPM, (nPM2VoicedEnerNew), sizeof(INT16));

  /*
   fprintf(stdout,"\n The shifted PM1 to local Minimum: \n");  // show data
   //for(i=0; i<nPM1VoicedEnerNew; i++)
   for(i=0; i<50; i++)
   fprintf(stdout,"Shifted PM1  %i = %i\n",i,dPM1VoicedShift[i]);

   //fprintf(stdout,"\n The Distance between PM and Local Minimum: \n");  // show data
   //for(i=0; i<nPM1VoicedEnerNew; i++)
   //fprintf(stdout,"Distance %i = %i\n",i,dPM1DistanceMinPM[i]);


   fprintf(stdout,"\n The shifted PM2 to local Minimum: \n");  // show data
   //for(i=0; i<nPM2VoicedEnerNew; i++)
   for(i=0; i<50; i++)
   fprintf(stdout,"Shifted PM2  %i = %i\n",i,dPM2VoicedShift[i]);

   //fprintf(stdout,"\n The Distance between PM and Local Minimum: \n");  // show data
   //for(i=0; i<nPM2VoicedEnerNew; i++)
   //fprintf(stdout,"Distance %i = %i\n",i,dPM2DistanceMinPM[i]);
   */

  /*-----------------------------------------------------------------------------
   ---- Calculation of Confidence Score - Berechnung des Konfidenz Punktes -----
   ------------------------------------------------------------------------------*/

  /*-------------------------------- First Confidence Score ------------------------------------
   ----------------- Frequency of Occurance of PM (Haeufigkeit des Auftretens) ---------------*/
  FLOAT64 *dPM1ConfidenceOccurance = NULL;
  FLOAT64 *dPM2ConfidenceOccurance = NULL;

  dPM1ConfidenceOccurance = (FLOAT64*)dlp_calloc(nPM1VoicedEnerNew, sizeof(FLOAT64)); // Allocates zero-initialize memory
  dPM2ConfidenceOccurance = (FLOAT64*)dlp_calloc(nPM2VoicedEnerNew, sizeof(FLOAT64)); // Allocates zero-initialize memory


  ConfidenceOccurancePM(dPM1VoicedShift, nPM1VoicedEnerNew, dPM2VoicedShift, nPM2VoicedEnerNew,
      dPM1ConfidenceOccurance, dPM2ConfidenceOccurance);

  /*
   fprintf(stdout,"\n The Confidence (Frequency of Occurance) for CHFA: \n");  // show data
   for(i=0; i<nPM1VoicedEnerNew; i++)
   fprintf(stdout,"Confidence (Frequency of Occurance)  %i = %f\n",i,dPM1ConfidenceOccurance[i]);


   fprintf(stdout,"\n The Confidence (Frequency of Occurance) for GCIDA: \n");  // show data
   for(i=0; i<nPM2VoicedEnerNew; i++)
   fprintf(stdout,"Confidence (Frequency of Occurance)  %i = %f\n",i,dPM2ConfidenceOccurance[i]);
   */

  /*--------------------- Second Confidence Score ------------------------
   --------- Distance between PM and the nearest negative peak --------*/
  FLOAT64 *dPM1ConfidenceMinPM = NULL;
  FLOAT64 *dPM2ConfidenceMinPM = NULL;

  dPM1ConfidenceMinPM = (FLOAT64*)dlp_calloc(nPM1VoicedEnerNew, sizeof(FLOAT64)); // Allocates zero-initialize memory
  dPM2ConfidenceMinPM = (FLOAT64*)dlp_calloc(nPM2VoicedEnerNew, sizeof(FLOAT64)); // Allocates zero-initialize memory


  ConfidenceDistanceMinPM(dPM1DistanceMinPM, nPM1VoicedEnerNew, SearchDomain, dPM1ConfidenceMinPM);
  ConfidenceDistanceMinPM(dPM2DistanceMinPM, nPM2VoicedEnerNew, SearchDomain, dPM2ConfidenceMinPM);

  /*
   fprintf(stdout,"\n The Confidence for CHFA: \n");  // show data
   for(i=0; i<nPM1VoicedEnerNew; i++)
   fprintf(stdout,"Confidence  %i = %f\n",i,dPM1ConfidenceMinPM[i]);


   fprintf(stdout,"\n The Confidence for GCIDA: \n");  // show data
   for(i=0; i<nPM2VoicedEnerNew; i++)
   fprintf(stdout,"Confidence  %i = %f\n",i,dPM2ConfidenceMinPM[i]);
   */

  /*-----------------------------------------------------------
   --------------------- Selection Algorithm -----------------
   ------------------------------------------------------------*/

  /*---------------- First Selection Algorithm -----------------
   ----- Selection according the frequency of occurance -----*/

  /* Select only PM with (confidence score (Haeufigkeit) = 1) */
  INT32 nSelectedOccurancePM = 0; // Number of selected PM

  INT32 *dPMSelectOccurance = NULL;

  dPMSelectOccurance = (INT32*)dlp_calloc(nPM1VoicedEnerNew, sizeof(INT32)); // length < (nPM1VoicedEnerNew)


  nSelectedOccurancePM = PMSelectConfidenceOccurance(dPM1VoicedShift, dPM1ConfidenceOccurance, nPM1VoicedEnerNew,
      dPMSelectOccurance);

  dPMSelectOccurance = (INT32*)dlp_realloc(dPMSelectOccurance, (nSelectedOccurancePM), sizeof(INT32));

  //fprintf(stdout,"\n\nThe selected PM (frequency of occurance): \n");  // show data
  //for(i=0; i<nSelectedOccurancePM; i++)
  //  fprintf(stdout,"Selected PM %i = %i\n",i,dPMSelectOccurance[i]);


  /*-----------------------------------------------------------
   ------------ Postprocessing - Nachbearbeitung --------------
   ------------------------------------------------------------*/

  INT32 nOccurancePostprocessPM = 0; // Final Length of PM

  INT32 *dOccurancePostprocess = NULL; // Final PM
  dOccurancePostprocess = (INT32*)dlp_calloc(nSelectedOccurancePM, sizeof(INT32)); // length < (nSelectedOccurancePM)

  nOccurancePostprocessPM = PostprocessingPM(samples, nSrate, nMeanEnergy, dPMSelectOccurance, nSelectedOccurancePM,
      dOccurancePostprocess);

  dOccurancePostprocess = (INT32*)dlp_realloc(dOccurancePostprocess, (nOccurancePostprocessPM), sizeof(INT32));

  //fprintf(stdout,"Number of Hybrid PM (Freq. of Occurance):   %d \n",nOccurancePostprocessPM);


  /*
   // Write: PM-File //
   Period *periods = NULL;  // (periods) is an object of struct (Period)
   INT32 nPeriods = 0;       // Number of selected PM

   INT16 nPMLengthSample = 0;   // [nPer] Distance between two successive PM in samples
   INT16 stimulation = 1;       // [anreg] (Anregung=1 for voiced segments)

   periods = (Period*)dlp_calloc(nOccurancePostprocessPM, sizeof(Period));  // Allocates zero-initialize memory


   // Copy the first PM
   (periods + nPeriods)->nPer = dOccurancePostprocess[0];
   (periods + nPeriods)->stimulation = stimulation;


   //fprintf(stdout,"nPer  %i = %i\n",nPeriods,dOccurancePostprocess[0]);
   //fprintf(stdout,"anreg  %i = %i\n",nPeriods,stimulation);


   // Copy the second PM until the end
   for(nPeriods = 1; nPeriods < nOccurancePostprocessPM; nPeriods++)
   {
   nPMLengthSample = dOccurancePostprocess[nPeriods] - dOccurancePostprocess[nPeriods - 1];
   (periods + nPeriods)->nPer = nPMLengthSample;
   //fprintf(stdout,"nPer  %i = %i\n",nPeriods,nPMLengthSample);
   (periods + nPeriods)->stimulation = stimulation;
   //fprintf(stdout,"anreg  %i = %i\n",nPeriods,stimulation);
   }


   // Copy PM from (struct periods) to output data object (dPM) //
   dPM->AddRecs(nOccurancePostprocessPM, 1); // AddRecs: Appends (n) records to the end of the table (data object)
   for(i = 0; i < nOccurancePostprocessPM; i++)
   {
   dPM->Dstore((FLOAT64)(periods+i)->nPer, i, 0);
   dPM->Dstore((FLOAT64)(periods+i)->stimulation, i, 1);
   }
   */

  /*
   INT16 nAuxCopy = 0;

   for (i = 0; i < nOccurancePostprocessPM; i++)
   {
   nAuxCopy = (INT16)CData_Dfetch(dPM,i,0);
   fprintf(stdout,"Selected PM with (frequency of occurance) %i = %i\n",i,nAuxCopy);
   }
   */

  /*------------------ Second Selection Algorithm Using FST (Dynamic Programming) ----------------
   --------- Selection according the Distance between PM and the nearest negative peak ---------*/

  /* Combining the PM from both PMA and Sorting the PM into ascending order */
  INT32 nCombinedPM = 0; // Number of Combined PM from both PMA

  INT32 *dCombinedPM = NULL;
  dCombinedPM = (INT32*)dlp_calloc(nPM1VoicedEnerNew + nPM2VoicedEnerNew, sizeof(INT32)); // length = (nCombinedPM)

  nCombinedPM = PMSorting(dPM1VoicedShift, nPM1VoicedEnerNew, dPM2VoicedShift, nPM2VoicedEnerNew, dCombinedPM);

  dCombinedPM = (INT32*)dlp_realloc(dCombinedPM, (nCombinedPM), sizeof(INT32));

  /*
   fprintf(stdout,"\n\nThe sorted PM in ascending order: \n");  // show data
   for(i=0; i<nCombinedPM; i++)
   fprintf(stdout,"Combined PM %i = %i\n",i,dCombinedPM[i]);
   */

  /* Divide the (combinedPM) to small units because the calculation of best path for whole signal require a lot of time.
   Therefore we use the pauses with minimal time (40 msec = 640 samples) to divide the graphs*/
  INT32 nPauseLeng = 640; // Length of pause
  INT32 nInitPM = 0; // Number of Initial states
  INT32 nTermPM = 0; // Number of Terminal states

  INT32 *InitialPM = NULL; // PM of (dCombinedPM) which has the initial state of graph
  INT32 *TerminalPM = NULL; // PM of (dCombinedPM) which has the terminal state of graph


  for(i = 0; i < nCombinedPM; i++) {
    if(i == 0) // The first PM in signal is initial state of first graph (first unit)
    {
      InitialPM = (INT32*)dlp_realloc(InitialPM, (nInitPM + 1), sizeof(INT32));
      InitialPM[nInitPM] = dCombinedPM[i];
      nInitPM = nInitPM + 1;
    } else if(i > 0 && i < (nCombinedPM - 1)) // Determine the initial and terminal state of graph by detection of pauses
    {
      if((dCombinedPM[i] - dCombinedPM[i - 1]) >= nPauseLeng) // There is a pause between two PM
      {
        TerminalPM = (INT32*)dlp_realloc(TerminalPM, (nTermPM + 1), sizeof(INT32));
        TerminalPM[nTermPM] = dCombinedPM[i - 1];
        nTermPM = nTermPM + 1;

        InitialPM = (INT32*)dlp_realloc(InitialPM, (nInitPM + 1), sizeof(INT32));
        InitialPM[nInitPM] = dCombinedPM[i];
        nInitPM = nInitPM + 1;
      }
    } else if(i == (nCombinedPM - 1)) // The final PM in signal is terminal state of last graph
    {
      TerminalPM = (INT32*)dlp_realloc(TerminalPM, (nTermPM + 1), sizeof(INT32));
      TerminalPM[nTermPM] = dCombinedPM[i];
      nTermPM = nTermPM + 1;
    }
  }

  /*
   fprintf(stdout,"\n\nThe initial and terminal PM of graphs: \n");  // show data
   for(i=0; i<nInitPM; i++)
   fprintf(stdout,"Initial PM %i = %i \t Terminal PM %i = %i \n",i,InitialPM[i],i,TerminalPM[i]);
   */

  // Calculate the number of states per unit
  INT32 *dStatesNumber = NULL; // Array indicates the number of states per unit (without the final state)

  dStatesNumber = (INT32*)dlp_calloc(nInitPM, sizeof(INT32)); // Allocates zero-initialize memory

  StatesNumberPerUnit(InitialPM, TerminalPM, nInitPM, dCombinedPM, nCombinedPM, dStatesNumber);

  /*
   fprintf(stdout,"\n\nThe number of states without final state per unit: \n");  // show data
   for(i=0; i<nInitPM; i++)
   fprintf(stdout,"Number of states without final state %i = %i\n",i,dStatesNumber[i]);
   */

  // Calculate the number of transitions per unit
  INT32 *dTransitionNumberPM1 = NULL; // Array indicates the number of transitions for PMA1 per unit
  INT32 *dTransitionNumberPM2 = NULL; // Array indicates the number of transitions for PMA2 per unit

  dTransitionNumberPM1 = (INT32*)dlp_calloc(nInitPM, sizeof(INT32)); // Allocates zero-initialize memory
  dTransitionNumberPM2 = (INT32*)dlp_calloc(nInitPM, sizeof(INT32)); // Allocates zero-initialize memory

  TransitionNumberPerUnit(dPM1VoicedShift, nPM1VoicedEnerNew, dPM2VoicedShift, nPM2VoicedEnerNew, InitialPM,
      TerminalPM, nInitPM, nSamples, dTransitionNumberPM1, dTransitionNumberPM2);

  /*
   fprintf(stdout,"\n\nThe number of transitions for PMA1 and PMA2: \n");  // show data
   for(i=0; i<nInitPM; i++)
   fprintf(stdout,"Number of transitions PMA1 %i = %i \t Number of transitions PMA2 %i = %i\n",i,dTransitionNumberPM1[i],i,dTransitionNumberPM2[i]);
   */

  // Save the states, transitions, and confidence scores in FST object
  CFst *itFst = NULL;
  ICREATEEX(CFst,itFst,"CPMproc::Hybrid.itFst",NULL); //Create buffer

  char PMunitName[nInitPM];

  INT32 nFirstPM1 = 0; // Index for first PM1 in the unit
  INT32 nLastPM1 = 0; // Index for last PM1 in the unit

  INT32 nFirstPM2 = 0; // Index for first PM2 in the unit
  INT32 nLastPM2 = 0; // Index for last PM2 in the unit


  CData_AddComp(AS(CData,itFst->td), "PMLoc", T_LONG); // Add a component to define PM Position (user defined component)


  INT32 nValuePM1 = 0;
  INT32 nIndexCombPM1 = 0;
  FLOAT64 nConfPM1 = 0;

  INT32 nValuePM2 = 0;
  INT32 nIndexCombPM2 = 0;
  FLOAT64 nConfPM2 = 0;

  // Important: State indices per unit start from 0 for the first state
  INT32 nInitialStateIndex = 0; // Index of initial state per unit
  INT32 nTransIndex = 0; // Index of transition in (td) (Index start with (0) until infinite)

  INT32 nMaxIndexCombPM = 0;

  for(i = 0; i < nInitPM; i++) {
    snprintf(PMunitName, nInitPM - 1, "PM%ld", (long)i); //To save a name for each unit (i.e. PM0, PM1, PM2, ...)
    //fprintf(stdout,PMunitName);

    // Add unit
    itFst->m_bFsa = TRUE; // Acceptor (only input, but the input in the hybrid method is (0))
    itFst->m_bLsr = TRUE; // negative logarithmic transition probability (logarithmic semiring)
    CFst_AddunitIam(itFst, PMunitName); // Adds one unit
    itFst->m_bFsa = FALSE;
    itFst->m_bLsr = FALSE;

    // Add the number of states
    CFst_AddstatesIam(itFst, i, dStatesNumber[i]); // Adds new states in unit (i)
    itFst->m_bFinal = TRUE; // Add final state
    CFst_AddstatesIam(itFst, i, 1);
    itFst->m_bFinal = FALSE;

    //CData_AddComp(AS(CData,itFst->td),"PM",T_LONG); // Add a component to define PM Position

    // Add transitions from PMA1
    if(dTransitionNumberPM1[i] != 0) // Process only if there is transitions
    {
      nInitialStateIndex = 0;
      nLastPM1 = nLastPM1 + dTransitionNumberPM1[i];
      for(j = nFirstPM1; j < nLastPM1; j++) {
        nValuePM1 = dPM1VoicedShift[j];
        for(jj = nIndexCombPM1; jj < nCombinedPM; jj++) {
          if(nValuePM1 == dCombinedPM[jj]) {
            CFst_AddtransIam(itFst, i, nInitialStateIndex, nInitialStateIndex + 1);
            nConfPM1 = -dPM1ConfidenceMinPM[j];
            CData_Dstore(AS(CData,itFst->td), 0, nTransIndex, 3); // Store (0) for input (Normal component is 2)
            CData_Dstore(AS(CData,itFst->td), nConfPM1, nTransIndex, 4); // Store transition weight (confidence score) (Normal component is 3)
            CData_Dstore(AS(CData,itFst->td), dCombinedPM[jj], nTransIndex, 2); // Store Position of PM
            nInitialStateIndex = nInitialStateIndex + 1;
            nTransIndex = nTransIndex + 1;
            nIndexCombPM1 = jj + 1;
            break;
          } else if(nValuePM1 != dCombinedPM[jj]) {
            nInitialStateIndex = nInitialStateIndex + 1;
          }
        }
      }
      nFirstPM1 = nFirstPM1 + dTransitionNumberPM1[i];
    }

    // Add transitions from PMA2
    if(dTransitionNumberPM2[i] != 0) // Process only if there is transitions
    {
      nInitialStateIndex = 0;
      nLastPM2 = nLastPM2 + dTransitionNumberPM2[i];
      for(j = nFirstPM2; j < nLastPM2; j++) {
        nValuePM2 = dPM2VoicedShift[j];
        for(jj = nIndexCombPM2; jj < nCombinedPM; jj++) {
          if(nValuePM2 == dCombinedPM[jj]) {
            CFst_AddtransIam(itFst, i, nInitialStateIndex, nInitialStateIndex + 1);
            nConfPM2 = -dPM2ConfidenceMinPM[j];
            CData_Dstore(AS(CData,itFst->td), 0, nTransIndex, 3); // Store (0) for input
            CData_Dstore(AS(CData,itFst->td), nConfPM2, nTransIndex, 4); // Store transition weight
            CData_Dstore(AS(CData,itFst->td), dCombinedPM[jj], nTransIndex, 2); // Store Position of PM
            nInitialStateIndex = nInitialStateIndex + 1;
            nTransIndex = nTransIndex + 1;
            //CData_Dstore(AS(CData,itFst->td->m_lpsNCTDLSR),nConfPM2,j,3);
            nIndexCombPM2 = jj + 1;
            break;
          } else if(nValuePM2 != dCombinedPM[jj]) {
            nInitialStateIndex = nInitialStateIndex + 1;
          }
        }
      }
      nFirstPM2 = nFirstPM2 + dTransitionNumberPM2[i];
    }

    nMaxIndexCombPM = MAX(nIndexCombPM1,nIndexCombPM2);
    nIndexCombPM1 = nMaxIndexCombPM;
    nIndexCombPM2 = nMaxIndexCombPM;

    //CFst_Print(itFst);
  }

  /*
   AS(CData,itFst->sd)->m_bList = TRUE;
   AS(CData,itFst->td)->m_bList = TRUE;
   CData_Print(itFst->ud);
   CData_Print(itFst->sd);
   CData_Print(itFst->td);
   AS(CData,itFst->sd)->m_bList = FALSE;
   AS(CData,itFst->td)->m_bList = FALSE;
   // or:
   //CFst_Print(itFst);
   */

  /* Use the first best path within the graph */
  CFst *itFstFirstBest = NULL; // FST object for Calculating the first best path
  ICREATEEX(CFst,itFstFirstBest,"CPMproc::Hybrid.itFstFirstBest",NULL); //Create buffer

  INT32 nSelectedBestPathPM = 0; // Number of PM after best path

  INT32 *dPMSelectBestPath = NULL;
  dPMSelectBestPath = (INT32*)dlp_calloc(nCombinedPM, sizeof(INT32));

  INT32 nTransUnitBestPM = 0; // Number of transition per unit (Number of combined PM per unit)
  INT32 nIndexCombinedPM = 0; // Index for combined PM
  FLOAT64 nAuxConf = 0;
  INT32 nAuxValuPM = 0;
  INT32 nAuxValuPM1 = 0;
  INT32 nAuxValuPM2 = 0;

  INT32 jjj = 0;

  INT32 nTransPMA1 = 0; // Number of transitions per unit from PMA1 (Number of PM1)
  INT32 nTransPMA2 = 0; // Number of transitions per unit from PMA2 (Number of PM2)

  INT32 nIndPM1 = 0; // Index for PM1
  INT32 nIndPM2 = 0; // Index for PM2
  INT32 nIndAuxPM1 = 0; // Index for PM1
  INT32 nIndAuxPM2 = 0; // Index for PM2

  /*
   INT32 nCorrPM1 = 0;    // Number of correct PM1
   INT32 nCorrPM2 = 0;    // Number of correct PM2
   */

  for(i = 0; i < nInitPM; i++) {
    if(dStatesNumber[i] > 1) // Only for units with two transitions at least
    {
      itFstFirstBest->m_bLocal = true;
      CFst_BestN(itFstFirstBest, itFst, i, 1, 0); // Calculate the first best path
      itFstFirstBest->m_bLocal = false;

      nTransUnitBestPM = (AS(CData,itFstFirstBest->td))->GetNRecs();

      //CFst_Print(itFstFirstBest);

      nIndAuxPM1 = nIndPM1;
      nIndAuxPM2 = nIndPM2;

      for(j = nIndexCombinedPM; j < nIndexCombinedPM + nTransUnitBestPM; j++) {
        //nAuxConf = CData_Dfetch(AS(CData,itFstFirstBest->td),j-nIndexCombinedPM,3); // Confidence score of transition (Normal component is 3)
        nAuxConf = CData_Dfetch(AS(CData,itFstFirstBest->td), j - nIndexCombinedPM, 4); // Confidence score of transition
        nAuxValuPM = (INT32)CData_Dfetch(AS(CData,itFstFirstBest->td), j - nIndexCombinedPM, 2); //


        nTransPMA1 = dTransitionNumberPM1[i];
        nTransPMA2 = dTransitionNumberPM2[i];

        /*
         // Accuracy of both Algorithms
         if(nTransPMA1 != 0)
         {
         for(jj = nIndPM1; jj < nIndPM1+nTransPMA1; jj++)
         {
         if(dPM1VoicedShift[jj] == nAuxValuPM && dPM1ConfidenceMinPM[jj] == -nAuxConf)
         {
         nCorrPM1 = nCorrPM1 + 1;
         break;
         }
         }
         }

         if(nTransPMA2 != 0)
         {
         for(jjj = nIndPM2; jjj < nIndPM2+nTransPMA2; jjj++)
         {
         if(dPM2VoicedShift[jjj] == nAuxValuPM && dPM2ConfidenceMinPM[jjj] == -nAuxConf)
         {
         nCorrPM2 = nCorrPM2 + 1;
         break;
         }
         }
         }
         */

        // Select the PM
        if(nAuxConf < -0.1) // Copy only PM with confidence score < -0.1
        {
          nAuxValuPM1 = nIndAuxPM1 + nTransPMA1;
          nAuxValuPM2 = nIndAuxPM2 + nTransPMA2;
          INT16 nOkPM1 = 0; // Indicate if PM belong to PMA1 (nOkPM1 = 1, when OK)

          if(nTransPMA1 != 0) {
            for(jj = nIndAuxPM1; jj < nAuxValuPM1; jj++) {
              if(jj < nPM1VoicedEnerNew && dPM1VoicedShift[jj] == nAuxValuPM && dPM1ConfidenceMinPM[jj] == -nAuxConf) {
                dPMSelectBestPath[nSelectedBestPathPM] = dPM1VoicedShift[jj];
                nSelectedBestPathPM = nSelectedBestPathPM + 1;
                nIndAuxPM1 = jj + 1;
                nOkPM1 = 1; // PM belong to PMA1
                break;
              }
            }
          }

          if(nTransPMA2 != 0 && nOkPM1 == 0) {
            for(jjj = nIndAuxPM2; jjj < nAuxValuPM2; jjj++) {
              if(jjj < nPM2VoicedEnerNew && dPM2VoicedShift[jjj] == nAuxValuPM && dPM2ConfidenceMinPM[jjj] == -nAuxConf) {
                dPMSelectBestPath[nSelectedBestPathPM] = dPM2VoicedShift[jjj];
                nSelectedBestPathPM = nSelectedBestPathPM + 1;
                nIndAuxPM2 = jjj + 1;
                break;
              }
            }
          }

        }
      }

      nIndPM1 = nIndPM1 + nTransPMA1;
      nIndPM2 = nIndPM2 + nTransPMA2;
      nIndexCombinedPM = nIndexCombinedPM + nTransUnitBestPM;
    }

    else if(dStatesNumber[i] == 1) // For units with only one transition (one PM)
    {
      nTransUnitBestPM = 1;

      nIndAuxPM1 = nIndPM1;
      nIndAuxPM2 = nIndPM2;

      for(j = nIndexCombinedPM; j < nIndexCombinedPM + nTransUnitBestPM; j++) {
        //nAuxConf = CData_Dfetch(AS(CData,itFstFirstBest->td),j-nIndexCombinedPM,4); // Confidence score of transition
        //nAuxValuPM = (INT32)CData_Dfetch(AS(CData,itFstFirstBest->td),j-nIndexCombinedPM,2); //
        nAuxValuPM = dCombinedPM[nIndexCombinedPM];

        nTransPMA1 = dTransitionNumberPM1[i];
        nTransPMA2 = dTransitionNumberPM2[i];

        /*
         // Accuracy of both Algorithms
         if(nTransPMA1 != 0)
         {
         for(jj = nIndPM1; jj < nIndPM1+nTransPMA1; jj++)
         {
         if(dPM1VoicedShift[jj] == nAuxValuPM)
         {
         nCorrPM1 = nCorrPM1 + 1;
         break;
         }
         }
         }

         if(nTransPMA2 != 0)
         {
         for(jjj = nIndPM2; jjj < nIndPM2+nTransPMA2; jjj++)
         {
         if(dPM2VoicedShift[jjj] == nAuxValuPM)
         {
         nCorrPM2 = nCorrPM2 + 1;
         break;
         }
         }
         }
         */

        // Select the PM
        nAuxValuPM1 = nIndAuxPM1 + nTransPMA1;
        nAuxValuPM2 = nIndAuxPM2 + nTransPMA2;
        INT16 nOkPM1 = 0; // Indicate if PM belong to PMA1 (nOkPM1 = 1, when OK)

        if(nTransPMA1 != 0) {
          for(jj = nIndAuxPM1; jj < nAuxValuPM1; jj++) {
            if(jj < nPM1VoicedEnerNew && dPM1VoicedShift[jj] == nAuxValuPM) {
              dPMSelectBestPath[nSelectedBestPathPM] = dPM1VoicedShift[jj];
              nSelectedBestPathPM = nSelectedBestPathPM + 1;
              nIndAuxPM1 = jj + 1;
              nOkPM1 = 1; // PM belong to PMA1
              break;
            }
          }
        }

        if(nTransPMA2 != 0 && nOkPM1 == 0) {
          for(jjj = nIndAuxPM2; jjj < nAuxValuPM2; jjj++) {
            if(jjj < nPM2VoicedEnerNew && dPM2VoicedShift[jjj] == nAuxValuPM) {
              dPMSelectBestPath[nSelectedBestPathPM] = dPM2VoicedShift[jjj];
              nSelectedBestPathPM = nSelectedBestPathPM + 1;
              nIndAuxPM2 = jjj + 1;
              break;
            }
          }
        }

      }

      nIndPM1 = nIndPM1 + nTransPMA1;
      nIndPM2 = nIndPM2 + nTransPMA2;
      nIndexCombinedPM = nIndexCombinedPM + nTransUnitBestPM;
    }
  }

  //fprintf(stdout,"Number of correct PMA1:   %d \n",nCorrPM1);
  //fprintf(stdout,"Number of correct PMA2:   %d \n",nCorrPM2);


  //CFst_Print(itFstFirstBest);

  //CFst_Print(itFst);
  //CFst_Sdp(itFstDP, itFst, 0, NULL);    // Best Path using (SDP)


  /*
   fprintf(stdout,"\n\nThe selected PM (Best Path): \n");  // show data
   //for(i=0; i<nSelectedBestPathPM; i++)
   for(i=0; i<100; i++)
   fprintf(stdout,"Selected PM %i = %i\n",i,dPMSelectBestPath[i]);
   */

  /*-----------------------------------------------------------
   ------------ Postprocessing - Nachbearbeitung --------------
   ------------------------------------------------------------*/

  INT32 nBestPathPostprocessPM = 0; // Length of PM after postprocessing

  INT32 *dBestPathPostprocess = NULL; // PM after postprocessing
  dBestPathPostprocess = (INT32*)dlp_calloc(nSelectedBestPathPM, sizeof(INT32)); // The length of this array must be (nBestPathPostprocessPM)


  nBestPathPostprocessPM = PostprocessingPM(samples, nSrate, nMeanEnergy, dPMSelectBestPath, nSelectedBestPathPM,
      dBestPathPostprocess);

  dBestPathPostprocess = (INT32*)dlp_realloc(dBestPathPostprocess, (nBestPathPostprocessPM), sizeof(INT32));

  //fprintf(stdout,"Number of Hybrid PM (Best Path):   %d \n",nBestPathPostprocessPM);

  /*
   fprintf(stdout,"\n\nThe selected PM (Best Path Postprocessing): \n");  // show data
   //for(i=0; i<nSelectedBestPathPM; i++)
   for(i=0; i<80; i++)
   fprintf(stdout,"Selected PM %i = %i\n",i,dBestPathPostprocess[i]);
   */

  // Add Unvoiced PM
  INT32 nFSTfinalPM = 0; // Final Length of PM with unvoiced PM
  INT32 nAuxAllPM = 0;
  nAuxAllPM = nBestPathPostprocessPM + nSamples / 70; // Distance between unvoiced PM is (80 sample)

  INT32 *dFSTfinalPM = NULL; // Final PM with unvoiced PM
  dFSTfinalPM = (INT32*)dlp_calloc(nAuxAllPM, sizeof(INT32)); // The length of this array must be (nFSTfinalPM)

  nFSTfinalPM = AddUnvoicedPM(dBestPathPostprocess, nBestPathPostprocessPM, nSamples, nSrate, dFSTfinalPM);

  dFSTfinalPM = (INT32*)dlp_realloc(dFSTfinalPM, (nFSTfinalPM), sizeof(INT32));

  /*
   fprintf(stdout,"\n\nThe Final PM with unvoiced PM: \n");  // show data
   for(i=0; i<200; i++)
   fprintf(stdout,"Final PM with unvoiced PM %i = %i\n",i,dFSTfinalPM[i]);
   */

  // Write: PM-File //
  PERIOD *periods = NULL; // (periods) is an object of struct (Period)
  INT32 nPeriods = 0; // Number of selected PM

  INT16 nPMLengthSample = 0; // [nPer] Distance between two successive PM in samples
  INT16 stimulation = 0; // [anreg] (Anregung=1 for voiced segments, Anregung=0 for unvoiced segments)

  periods = (PERIOD*)dlp_calloc(nFSTfinalPM, sizeof(PERIOD)); // Allocates zero-initialize memory

  // Copy the first PM
  (periods + nPeriods)->nPer = abs(dFSTfinalPM[0]);
  if(dFSTfinalPM[0] > 0) // Positive PM
  {
    stimulation = 1;
    (periods + nPeriods)->stimulation = stimulation;
  } else // Negative PM
  {
    stimulation = 0;
    (periods + nPeriods)->stimulation = stimulation;
  }

  //fprintf(stdout,"nPer  %i = %i\n",nPeriods,dFSTfinalPM[0]);
  //fprintf(stdout,"anreg  %i = %i\n",nPeriods,stimulation);


  // Copy the second PM until the end
  for(nPeriods = 1; nPeriods < nFSTfinalPM; nPeriods++) {
    nPMLengthSample = abs(dFSTfinalPM[nPeriods]) - abs(dFSTfinalPM[nPeriods - 1]);
    (periods + nPeriods)->nPer = nPMLengthSample;
    //fprintf(stdout,"nPer  %i = %i\n",nPeriods,nPMLengthSample);

    if(dFSTfinalPM[nPeriods] > 0) // Positive PM
    {
      stimulation = 1;
      (periods + nPeriods)->stimulation = stimulation;
    } else // Negative PM
    {
      stimulation = 0;
      (periods + nPeriods)->stimulation = stimulation;
    }
    //fprintf(stdout,"anreg  %i = %i\n",nPeriods,stimulation);
  }

  // Copy PM from (struct periods) to output data object (dPM)
  dPM->AddRecs(nFSTfinalPM, 1); // AddRecs: Appends (n) records to the end of the table (data object)
  for(i = 0; i < nFSTfinalPM; i++) {
    dPM->Dstore((FLOAT64)(periods + i)->nPer, i, 0);
    dPM->Dstore((FLOAT64)(periods + i)->stimulation, i, 1);
  }

  /*
   INT16 nAuxCopy = 0;

   for (i = 0; i < nFSTfinalPM; i++)
   {
   nAuxCopy = (INT16)CData_Dfetch(dPM,i,0);
   fprintf(stdout,"Selected PM with (Best Path) %i = %i\n",i,nAuxCopy);
   }
   */

  /*------------------ Third Selection Algorithm Using OR-Gate ----------------
   ------ Selection according the Sum of PM1 + PM2 -----*/

  /* Combining the PM from both PMA and Sorting the PM into ascending order */
  INT32 nSumPM = 0; // Number of Combined PM from both PMA

  INT32 *dSumPM = NULL;
  dSumPM = (INT32*)dlp_calloc(nPM1VoicedEnerNew + nPM2VoicedEnerNew, sizeof(INT32)); // The length of this array must be (nSumPM)

  nSumPM = PMSorting(dPM1VoicedShift, nPM1VoicedEnerNew, dPM2VoicedShift, nPM2VoicedEnerNew, dSumPM);

  /*-----------------------------------------------------------
   ------------ Postprocessing - Nachbearbeitung --------------
   ------------------------------------------------------------*/

  //INT32 nSumPostprocessPM = 0; // Final Length of PM

  INT32 *dSumPostprocess = NULL; // Final PM
  dSumPostprocess = (INT32*)dlp_calloc(nSumPM, sizeof(INT32)); // The length of this array must be (nSumPostprocessPM)


  //nSumPostprocessPM = PostprocessingPM(samples, nSrate, nMeanEnergy, dSumPM, nSumPM, dSumPostprocess);

  //fprintf(stdout,"Number of Hybrid PM (OR-Gate):   %d \n",nSumPostprocessPM);


  /*
   // Write: PM-File //
   Period *periods = NULL;  // (periods) is an object of struct (Period)
   INT32 nPeriods = 0;       // Number of selected PM

   INT16 nPMLengthSample = 0;   // [nPer] Distance between two successive PM in samples
   INT16 stimulation = 1;       // [anreg] (Anregung=1 for voiced segments)

   periods = (Period*)dlp_calloc(nSumPostprocessPM, sizeof(Period));  // Allocates zero-initialize memory

   // Copy the first PM
   (periods + nPeriods)->nPer = dSumPostprocess[0];
   (periods + nPeriods)->stimulation = stimulation;


   //fprintf(stdout,"nPer  %i = %i\n",nPeriods,dSumPostprocess[0]);
   //fprintf(stdout,"anreg  %i = %i\n",nPeriods,stimulation);


   // Copy the second PM until the end
   for(nPeriods = 1; nPeriods < nSumPostprocessPM; nPeriods++)
   {
   nPMLengthSample = dSumPostprocess[nPeriods] - dSumPostprocess[nPeriods - 1];
   (periods + nPeriods)->nPer = nPMLengthSample;
   //fprintf(stdout,"nPer  %i = %i\n",nPeriods,nPMLengthSample);
   (periods + nPeriods)->stimulation = stimulation;
   //fprintf(stdout,"anreg  %i = %i\n",nPeriods,stimulation);
   }


   // Copy PM from (struct periods) to output data object (dPM)
   dPM->AddRecs(nSumPostprocessPM, 1); // AddRecs: Appends (n) records to the end of the table (data object)
   for(i = 0; i < nSumPostprocessPM; i++)
   {
   dPM->Dstore((FLOAT64)(periods+i)->nPer, i, 0);
   dPM->Dstore((FLOAT64)(periods+i)->stimulation, i, 1);
   }
   */

  /*
   INT16 nAuxCopy = 0;

   for (i = 0; i < nSumPostprocessPM; i++)
   {
   nAuxCopy = (INT16)CData_Dfetch(dPM,i,0);
   fprintf(stdout,"Selected PM with (Best Path) %i = %i\n",i,nAuxCopy);
   }
   */

  /*-----------------------------------------------------------
   ------- Export data from data object to ASCII file --------
   ------------------------------------------------------------*/

  CDlpFile *lpFile = NULL;

  ICREATEEX(CDlpFile,lpFile,"CPMproc::Hybrid.lpFile",NULL); //Create buffer


  CData *dIntervalMinPM1 = NULL; // Data ogject for distance between PM and local Minimum (For Histogram)
  ICREATEEX(CData,dIntervalMinPM1,"CPMproc::Hybrid.dIntervalMinPM1",NULL); //Create buffer
  CData_Array(dIntervalMinPM1, T_DOUBLE, 1, nPM1VoicedEnerNew); // CData::Array(INT16 nCType, INT32 nComps, INT32 nRecs);


  CData *dIntervalMinPM2 = NULL; // Data ogject for distance between PM and local Minimum (For Histogram)
  ICREATEEX(CData,dIntervalMinPM2,"CPMproc::Hybrid.dIntervalMinPM2",NULL); //Create buffer
  CData_Array(dIntervalMinPM2, T_DOUBLE, 1, nPM2VoicedEnerNew); // CData::Array(INT16 nCType, INT32 nComps, INT32 nRecs);


  INT16 nAuxVar = 0;
  FLOAT64 nAuxDouble = 0;

  // Copy Distance (PMA1)
  for(i = 0; i < nPM1VoicedEnerNew; i++) {
    nAuxVar = dPM1DistanceMinPM[i];
    nAuxDouble = (FLOAT64)nAuxVar;
    CData_Dstore(dIntervalMinPM1, nAuxDouble, i, 0); //CData_Dstore(CData _this, FLOAT64 dVal, INT32 nIRec, INT32 nIComp);
  }

  /*
   dIntervalMinPM1->m_bList = TRUE;  // option (/list)
   CData_Print(dIntervalMinPM1);
   dIntervalMinPM1->m_bList = FALSE;
   */

  // Copy Distance (PMA2)
  for(i = 0; i < nPM2VoicedEnerNew; i++) {
    nAuxVar = dPM2DistanceMinPM[i];
    nAuxDouble = (FLOAT64)nAuxVar;
    CData_Dstore(dIntervalMinPM2, nAuxDouble, i, 0); //CData_Dstore(CData _this, FLOAT64 dVal, INT32 nIRec, INT32 nIComp);
  }

  /*
   dIntervalMinPM2->m_bList = TRUE;  // option (/list)
   CData_Print(dIntervalMinPM2);
   dIntervalMinPM2->m_bList = FALSE;
   */

  /*
   const char *sFilename1 = "Distance_Min_PM1.txt";
   const char *sFilename2 = "Distance_Min_PM2.txt";
   const char *sFilter = "ascii";


   CDlpFile_Export(lpFile, sFilename1, sFilter, dIntervalMinPM1); //CDlpFile_Export(CDlpFile _this, const char* sFilename, const char* sFilter, CDlpObject* iInst);
   CDlpFile_Export(lpFile, sFilename2, sFilter, dIntervalMinPM2); //CDlpFile_Export(CDlpFile _this, const char* sFilename, const char* sFilter, CDlpObject* iInst);
   */

  /* Destroys an instance of a dLabPro class (data class) */
  IDESTROY(idPM1); // Destroy idPM1
  IDESTROY(idPM2); // Destroy idPM2
  IDESTROY(itFst); // Destroy
  IDESTROY(itFstFirstBest); // Destroy
  IDESTROY(lpFile); // Destroy
  IDESTROY(dIntervalMinPM1); // Destroy
  IDESTROY(dIntervalMinPM2); // Destroy

  // free
  dlp_free(samples);
  dlp_free(dSmoothSignal);
  dlp_free(dEnergyContour);
  dlp_free (idPM1nPer);
  dlp_free (idPM1Anreg);
  dlp_free (idPM2nPer);
  dlp_free (idPM2Anreg);
  dlp_free (idPM1nPerSumSamples);
  dlp_free (idPM2nPerSumSamples);
  dlp_free (idPM1VoicedSumSamples);
  dlp_free (idPM2VoicedSumSamples);
  dlp_free(dPM1VoicedEner);
  dlp_free(dPM2VoicedEner);
  dlp_free(dPM1VoicedShift);
  dlp_free(dPM2VoicedShift);
  dlp_free(dPM1DistanceMinPM);
  dlp_free(dPM2DistanceMinPM);
  dlp_free(dPM1ConfidenceOccurance);
  dlp_free(dPM2ConfidenceOccurance);
  dlp_free(dPM1ConfidenceMinPM);
  dlp_free(dPM2ConfidenceMinPM);
  dlp_free(dPMSelectOccurance);
  dlp_free(dOccurancePostprocess);
  dlp_free(dCombinedPM);
  dlp_free(InitialPM);
  dlp_free(TerminalPM);
  dlp_free(dStatesNumber);
  dlp_free(dTransitionNumberPM1);
  dlp_free(dTransitionNumberPM2);
  dlp_free(dPMSelectBestPath);
  dlp_free(dBestPathPostprocess);
  dlp_free(dSumPM);
  dlp_free(dSumPostprocess);
  dlp_free(periods);
  dlp_free(dFSTfinalPM);

  return O_K;
}

////////////////////////////////////////////////////////////////////////////////////
//////////////////////////// Definition of Function ////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

/*------------------------------------------------------------
 Function:        MaximumValue

 Maximum value of vector data

 Parameter:
 a    Speech signal
 n   Length of signal

 ------------------------------------------------------------*/
FLOAT64 CGEN_PRIVATE CPMproc::MaximumValue(FLOAT64 *a, INT32 n) {
  FLOAT64 x = 0;
  INT32 i = 0;

  for(i = 0; i < n; i++) {
    if(x < a[i]) x = a[i];
  }

  return (x);
}

/*------------------------------------------------------------
 Function:        MinimumValue

 Minimum value of vector data

 Parameter:
 a    Speech signal
 n   Length of signal

 ------------------------------------------------------------*/
FLOAT64 CGEN_PRIVATE CPMproc::MinimumValue(FLOAT64 *a, INT32 n) {
  FLOAT64 x = 0;
  INT32 i = 0;

  for(i = 0; i < n; i++) {
    if(x > a[i]) x = a[i];
  }

  return (x);
}

/*------------------------------------------------------------
 Function:        SignalNorming

 Norming the Signal at (1.0) (Normierung)

 Parameter:
 a    Speech signal
 n   Length of signal
 b   Normalized signal

 ------------------------------------------------------------*/
INT16 CGEN_PRIVATE CPMproc::SignalNorming(FLOAT64 *a, INT32 n, FLOAT64 *b) {
  INT32 i = 0;
  FLOAT64 maximum = 0;
  FLOAT64 minimum = 0;

  maximum = MaximumValue(a, n);
  minimum = MinimumValue(a, n);

  FLOAT64 MinAbsolut = fabs(minimum); //Absoluter Wert

  FLOAT64 v = maximum; // Auxiliary Variable

  if(MinAbsolut >= v) v = MinAbsolut; //Suche nach Maximum ( von negativen und positiven Werten )


  for(i = 0; i < n; i++)
    b[i] = a[i] / v;

  return O_K;
}

/*------------------------------------------------------------
 Function:        MovingAverage

 Arithmetische gleitende Durchschnitte (Simple Moving Average, SMA)

 Parameter:
 a    Speech signal
 n   Length of signal
 m   Length of smoothing window
 b   Smoothed signal

 ------------------------------------------------------------*/
INT16 CGEN_PRIVATE CPMproc::MovingAverage(FLOAT64 *a, INT32 n, INT32 m, FLOAT64 *b) {
  INT32 i = 0;
  INT32 j = 0;
  FLOAT64 s = 0;

  FLOAT64 *c = NULL;
  c = (FLOAT64*)dlp_calloc(n+m, sizeof(FLOAT64)); // Allocates zero-initialize memory

  //FLOAT64 *c;            // auxiliary data vector
  //c=new FLOAT64 [n+m];   // length = length of signal + length of smoothing window

  for(i = 0; i < (n + m); i++) // initial the auxiliary data vector (c) with (0)
    c[i] = 0;

  for(i = 0; i < n; i++) // copy the original signal (a) to the auxiliary data vector
    c[i] = a[i];

  for(i = 0; i < n; i++) // Smoothing the signal
  {
    s = 0;
    for(j = i; j < (i + m); j++) {
      s += c[j];
    }
    b[i] = s / m;
  }

  dlp_free(c);

  //delete []c;

  return O_K;
}

/*------------------------------------------------------------
 Function:        ShortTimeEnergy

 Short Time Energy - Kurzzeit-Energie

 Parameter:
 a    Speech signal or smoothed signal
 n   Length of signal
 m   Length of window
 b   Short Time Energy Contour

 ------------------------------------------------------------*/
INT16 CGEN_PRIVATE CPMproc::ShortTimeEnergy(FLOAT64 *a, INT32 n, INT32 m, FLOAT64 *b) {
  INT32 i = 0;
  INT32 j = 0;
  INT32 nNullAdd = 0; // Add zeros in the beginning and end of signal
  // (H�lfte der Fenasterlaenge beim Anfang und H�lfte der Fenasterlaenge beim Ende)
  FLOAT64 QuadValue = 0;
  FLOAT64 EnergValue = 0;

  nNullAdd = m / 2;

  FLOAT64 *c = NULL;
  c = (FLOAT64*)dlp_calloc(n+m, sizeof(FLOAT64)); // Allocates zero-initialize memory

  //FLOAT64 *c;            // Auxiliary data vector
  //c=new FLOAT64 [n+m];   // length = length of signal + length of energy window


  for(i = 0; i < (n + m); i++) // Initial the auxiliary data vector (c) with (0)
    c[i] = 0;

  for(i = 0; i < n; i++) // Copy the original signal (a) to the auxiliary data vector with offset of (nNullAdd)
    c[i + nNullAdd] = a[i];

  for(i = nNullAdd; i < (n + nNullAdd); i++) // Calculating the energy
  {
    EnergValue = 0;
    for(j = i - nNullAdd; j < (i + nNullAdd); j++) {
      QuadValue = 0;
      QuadValue = c[j] * c[j];
      EnergValue += QuadValue;
    }
    b[i - nNullAdd] = EnergValue;
  }

  dlp_free(c);

  return O_K;
}

/*------------------------------------------------------------
 Function:        MeanValue

 Mean value (Mittelwert)

 Parameter:
 a    Speech signal
 n   Length of signal

 ------------------------------------------------------------*/
FLOAT64 CGEN_PRIVATE CPMproc::MeanValue(FLOAT64 *a, INT32 n) {
  INT32 i = 0;
  FLOAT64 x = 0;
  FLOAT64 xx = 0;
  FLOAT64 b = 0;

  for(i = 0; i < n; i++)
    x = x + a[i]; // Die Summe des gesamten Signals

  xx = x / n; //Mittelwert fuer das gesamte Signal
  b = xx;

  return (b);
}

/*------------------------------------------------------------
 Function:        PMConvertSumSamples

 The PM format of TU Dresden (two columns) is:
 nPer  : is the distance between PM in samples.
 anreg : is the Anregung [ - (voiceless or stimmlos) , + (voice or stimmhaft)].

 This function calculate the PM as the sum of samples from (0) to the position of PM.

 Parameter:
 a    First column in PM data instance [nPer]
 m   Number of PM
 b   PM in samples [ sum of samples from (0) ]

 ------------------------------------------------------------*/
INT16 CGEN_PRIVATE CPMproc::PMConvertSumSamples(INT16 *a, INT32 m, INT32 *b) {
  INT32 i = 0;
  INT32 x = 0;

  for(i = 0; i < m; i++) {
    x = x + a[i]; // Number of samples from (0) to the position of PM
    b[i] = x; // PM in samples from (0)
  }

  return O_K;
}

/*------------------------------------------------------------
 Function:        ExtractVoicedPM

 Read only PM in voiced segments (anreg = 1) and delete PM in voiceless segments (anreg = 0)

 Parameter:
 a    nPer of PM [ sum of samples from (0) ] (column 1)
 b    anreg of PM (column 2)
 m   Number of PM (voiced and voiceless)
 n   Number of voiced PM
 c   PM with only voiced PM

 ------------------------------------------------------------*/
INT32 CGEN_PRIVATE CPMproc::ExtractVoicedPM(INT32 *a, INT16 *b, INT32 m, INT32 *c) {
  INT32 i = 0;
  INT32 j = 0;
  INT32 n = 0;

  for(i = 0; i < m; i++) {
    if(b[i] == 1) // Copy only voiced PM (anreg = 1)
    {
      c[j] = a[i];
      j = j + 1;
    }
  }

  n = j; // Number of voiced PM

  return n;
}

/*------------------------------------------------------------
 Function:        DeleteSmallEnergyPM

 Delete PM that have an energy unter the threshold of Short Time Energy Contour

 Parameter:
 a    Short Time Energy Contour of normalized and smoothed signal
 k    Number of samples for (Short Time Energy Contour)
 b    Voiced PM [ sum of samples from (0) ]
 m   Number of voiced PM
 thr  Threshold for Energy Contour
 n   Number of voiced PM that have an energy which is higher than a threshold
 c   New voiced PM (sum of samples) which have an energy that is bigger than a threshold

 ------------------------------------------------------------*/
INT32 CGEN_PRIVATE CPMproc::DeleteSmallEnergyPM(FLOAT64 *a, INT32 k, INT32 *b, INT32 m, FLOAT64 thr, INT32 *c) {
  INT32 i = 0; // Counter for voiced PM
  INT32 j = 0; // Counter for voiced PM with an energy that is bigger than a threshold
  INT32 nPMSamples = 0; // The position of PM in samples
  INT32 n = 0;

  for(i = 0; i < m; i++) {
    nPMSamples = b[i];
    if(nPMSamples >= 0 && nPMSamples < k && a[nPMSamples] >= thr) // Check if the energy contour of PM is bigger than the threshold
    {
      c[j] = b[i]; // Copy the PM with high energy
      j = j + 1;
    }
  }

  n = j; // Number of voiced PM that have an energy which is higher than a threshold

  return n;
}

/*------------------------------------------------------------
 Function:        ShiftPMnearestNegativePeak

 Shift the PM to the nearest negative point of signal in domain [-n, +n sample]
 Alignment (Zuordnung) the PM
 [-16, +16 sample] , [-1, + 1 msec] for SR=16000 Hz

 Parameter:
 a    Speech signal or normalized speech signal
 nSam  Number of samples of signal
 b    Voiced PM with higher energy [samples]
 m   Number of voiced PM with higher energy
 ener  Short Time Energy Contour
 thr  Threshold for Energy Contour for low energy segments
 n    Search domain for the nearest negative value of signal [(n) ist eine gerade Anzahl]
 cc   return New shifted PM (Local minimum) (new position in samples)
 dd  return Distance between PM (voiced PM with higher energy) and nearest negative point of signal [Distance in samples]
 nPM   return New number of shifted PM
 ------------------------------------------------------------*/
INT32 CGEN_PRIVATE CPMproc::ShiftPMnearestNegativePeak(FLOAT64 *a, INT32 nSam, INT32 *b, INT32 m, FLOAT64 *ener,
    FLOAT64 thr, INT16 n, INT32 *cc, INT16 *dd) {
  INT32 i = 0;
  INT32 j = 0;
  INT32 nPositionPM = 0; // Position of PM in samples
  FLOAT64 LocalMin = 0; // Value of local minimum
  INT32 nPositionLocalMin = 0; // Position of local minimum
  INT16 nDistanceMinPM = 0; // Distance between local minimum and PM in samples
  INT32 nAux = 0;

  nAux = (INT32)n;

  INT32 *c = NULL; // shifted PM
  c = (INT32*)dlp_calloc(m, sizeof(INT32)); // Allocates zero-initialize memory

  INT16 *d = NULL; // Distance between PM and nearest negative point of signal
  d = (INT16*)dlp_calloc(m, sizeof(INT16)); // Allocates zero-initialize memory


  for(i = 0; i < m; i++) // Counter for the number of voiced PM with higher energy
  {
    nPositionPM = b[i];
    LocalMin = a[nPositionPM]; // We assume that PM are positioned on negative-going peaks
    nPositionLocalMin = nPositionPM;

    if(ener[nPositionPM] >= thr) // Energy of PM is bigger than a threshold (voiced segments)
    {
      for(j = nPositionPM - nAux; j < nPositionPM + nAux; j++) // Search for Minimum around PM in domain of (n) samples
      {
        if(j >= 0 && j < nSam) // Test if the counter is inside the domain of signal
        {
          if(a[j] < LocalMin) {
            LocalMin = a[j];
            nPositionLocalMin = j;
          }
        }
      }
      c[i] = nPositionLocalMin;
      nDistanceMinPM = nPositionPM - nPositionLocalMin;
      d[i] = nDistanceMinPM;
    } // End if

    else if(ener[nPositionPM] < thr) // Begin and end of voiced segments
    {
      for(j = nPositionPM - (nAux / 2); j < nPositionPM + (nAux / 2); j++) // Search for Minimum around PM in domain of (n/2) samples
      {
        if(j >= 0 && j < nSam) // Test if the counter is inside the domain of signal
        {
          if(a[j] < LocalMin) {
            LocalMin = a[j];
            nPositionLocalMin = j;
          }
        }
      }
      c[i] = nPositionLocalMin;
      nDistanceMinPM = nPositionPM - nPositionLocalMin;
      d[i] = nDistanceMinPM;
    } // End else if
  } // End for


  /* Delete the equal shifted PM after the alignment */

  // Add new element at the end of array with the value (0)
  INT32 *c1 = NULL; // shifted PM
  c1 = (INT32*)dlp_calloc(m+1, sizeof(INT32)); // Allocates zero-initialize memory

  INT16 *d1 = NULL;
  d1 = (INT16*)dlp_calloc(m+1, sizeof(INT16)); // Allocates zero-initialize memory


  // Copy shifted PM
  for(i = 0; i < m; i++) {
    c1[i] = c[i];
  }
  c1[m] = 0; // Last PM = 0

  // Copy Distance between PM and minimum peak
  for(i = 0; i < m; i++) {
    d1[i] = d[i];
  }
  d1[m] = 0; // Last Distance = 0


  INT32 nPM = 0; // Counter for the new shifted PM and after delete the equal shifted PM

  for(i = 0; i < m; i++) {
    if(c1[i] != c1[i + 1]) {
      if(c1[i] < c1[i + 1]) // After shift the PM, perhaps the neighbour PMs have decreased values
      { // Therefore, we take only increased PM
        cc[nPM] = c1[i];
        dd[nPM] = d1[i];
        nPM = nPM + 1;
      }
    }
  }

  dlp_free(c);
  dlp_free(d);
  dlp_free(c1);
  dlp_free(d1);

  return nPM;
}

/*------------------------------------------------------------
 Function:        ConfidenceOccurancePM

 Calculate the Confidence Score based on the frequency of occurance of PM between PMAs
 Confidence Score = [0 -> 1], whereas:
 (0) is no confidence
 (1) is total confidence


 Number of occurance of PMi
 Confidence Score(PMi) = ----------------------------
 Number of combined PMAs

 Parameter:
 a    Shifted PM from PMA1 (CHFA)
 m   Number of PM from the first PMA
 b   Shifted PM from PMA2 (GCIDA)
 n   Number of PM from the second PMA
 c   Confidence Score of first PMA (Frequency of Occurance of PM)
 d   Confidence Score of second PMA (Frequency of Occurance of PM)

 ------------------------------------------------------------*/
INT16 CGEN_PRIVATE CPMproc::ConfidenceOccurancePM(INT32 *a, INT32 m, INT32 *b, INT32 n, FLOAT64 *c, FLOAT64 *d) {
  INT32 i = 0;
  INT32 j = 0;

  /* Initialize the confidence score of both PMA with (0.5)
   Assume that PM occure only in one PMA (frequency of occurance = 0.5)*/

  for(i = 0; i < m; i++)
    c[i] = 0.5;

  for(i = 0; i < n; i++)
    d[i] = 0.5;

  /* Calculate the confidence score
   (confidence score = 1) when PMi has a same value in PMA1 and PMA2 */

  for(i = 0; i < m; i++) {
    for(j = 0; j < n; j++) {
      if(a[i] == b[j]) {
        c[i] = 1;
        break;
      }
    }
  }

  for(i = 0; i < n; i++) {
    for(j = 0; j < m; j++) {
      if(b[i] == a[j]) {
        d[i] = 1;
        break;
      }
    }
  }

  return O_K;
}

/*------------------------------------------------------------
 Function:        PMSelectConfidenceOccurance

 Select PM with (confidence score = 1) by frequency of occurance as confidence score

 Parameter:
 a   Shifted PM from PMA1 or PMA2
 b    Confidence Score of first or second PMA (Frequency of Occurance of PM)
 m   Number of PM from the first or second PMA (length of confidence score array)
 c   Selected PM with confidence score = 1
 n   Number of selected PM

 ------------------------------------------------------------*/
INT32 CGEN_PRIVATE CPMproc::PMSelectConfidenceOccurance(INT32 *a, FLOAT64 *b, INT32 m, INT32 *c) {
  INT32 i = 0;
  INT32 j = 0;
  INT32 n = 0; // Number of selected PM

  for(i = 0; i < m; i++) {
    if(b[i] == 1) {
      c[j] = a[i];
      j = j + 1;
    }
  }

  n = j;
  return n;
}

/*------------------------------------------------------------
 Function:        ConfidenceDistanceMinPM

 Calculate the Confidence Score based on the distance between PM and the nearest negative point of signal
 Confidence Score = [0 -> 1], whereas:
 (0) is no confidence
 (1) is total confidence


 distance between PMi and the nearest negative peak in samples
 Confidence Score(PMi) = 1 - -----------------------------------------------------------------
 Search domain in samples

 Parameter:
 a    Distance between PM and nearest negative peak of signal [Distance in samples]
 m   Number of shifted PM (Number of voiced PM with higher energy)
 n    Search domain for the nearest negative value of signal [(n) ist eine gerade Anzahl]
 b   Confidence Score (distance between PM and the nearest negative point of signal) [Probability]

 ------------------------------------------------------------*/
INT16 CGEN_PRIVATE CPMproc::ConfidenceDistanceMinPM(INT16 *a, INT32 m, INT16 n, FLOAT64 *b) {
  INT32 i = 0;
  FLOAT64 nAux = 0.0;
  FLOAT64 Conf = 0.0;

  for(i = 0; i < m; i++) {
    nAux = (FLOAT64)a[i] / (FLOAT64)n;
    Conf = 1.0 - fabs(nAux);
    b[i] = Conf;
  }

  return O_K;
}

/*------------------------------------------------------------
 Function:        PMSorting

 Sorting (arrange) PM from the both PM Algorithms in one array in ascending order
 and by equivalent PM use only one value (from PMA1)

 Parameter:
 a    Shifted PM from PMA1 (CHFA)
 m   Number of PM from the first PMA
 b   Shifted PM from PMA2 (GCIDA)
 n   Number of PM from the second PMA
 c   Shifted PM for both PM Algorithms
 p   Number of sorted PM from both PM Algorithms

 ------------------------------------------------------------*/
INT32 CGEN_PRIVATE CPMproc::PMSorting(INT32 *a, INT32 m, INT32 *b, INT32 n, INT32 *c) {

  INT32 i = 0;
  INT32 j = 0;
  INT32 nAux = 0;
  INT32 nCombinedPM = 0; // Number of Combined PM (Number of states - 1)
  INT32 pass = 0; // Number of passes for sorting an array' values
  INT32 hold = 0;
  INT32 p = 0;

  INT32 *CommonPM = NULL; // Copy PMA1 and PMA2 in one array (length PMA1 + length PMA2)
  CommonPM = (INT32*)dlp_calloc(m+n, sizeof(INT32)); // Allocates zero-initialize memory

  /* Copy PM from PMA1 and PMA2 to the array (CommonPM) */
  for(i = 0; i < m; i++) {
    nAux = a[i];
    CommonPM[i] = nAux;
  }

  for(i = 0; i < n; i++) {
    nAux = b[i];
    CommonPM[i + m] = nAux;
  }

  /* Convert the PM in PMA2 which have the same value with PM in PMA1 to (Null) */
  for(i = 0; i < m; i++) {
    for(j = m; j < m + n; j++) {
      if(CommonPM[i] == CommonPM[j]) {
        CommonPM[j] = 0;
        break;
      }
    }
  }

  /* Delete (0) values from array */
  for(i = 0; i < m + n; i++) {
    if(CommonPM[i] != 0) {
      c[nCombinedPM] = CommonPM[i];
      nCombinedPM = nCombinedPM + 1;
    }
  }

  /* Sort an array's values into ascending order */
  for(pass = 1; pass < nCombinedPM; pass++) // passes
    for(i = 0; i < nCombinedPM - 1; i++) // one pass
      if(c[i] > c[i + 1]) // one comparison
      {
        hold = c[i]; // one swap
        c[i] = c[i + 1];
        c[i + 1] = hold;
      }

  dlp_free(CommonPM);

  p = nCombinedPM;
  return p;
}

/*------------------------------------------------------------
 Function:        StatesNumberPerUnit

 Calculate the number of states in one unit without final state (Number of PM between two pauses)

 Parameter:
 a    Array of Initial PM
 b   Array of Terminal PM
 m   Number of PM in Initial or Terminal PM
 c   Array of Combined PM (output of function (PMSorting))
 n   Number of Combined PM
 p    Array for the number of states (without final state)

 ------------------------------------------------------------*/
INT16 CGEN_PRIVATE CPMproc::StatesNumberPerUnit(INT32 *a, INT32 *b, INT32 m, INT32 *c, INT32 n, INT32 *p) {
  INT32 i = 0;
  INT32 j = 0;
  INT32 Start = 0; // Start and End PM
  INT32 End = 0;
  INT32 StartIndex = 0; // Index of Start and End PM in Combined PM
  INT32 EndIndex = 0;
  INT32 k = 0; // Number of PM between two pauses (Number of states without final state)

  for(i = 0; i < m; i++) {
    Start = a[i];
    End = b[i];
    for(j = EndIndex; j < n; j++) {
      if(c[j] == Start) StartIndex = j;
      //else if (c[j]==End)
      if(c[j] == End) {
        EndIndex = j;
        break;
      }
    }
    k = EndIndex - StartIndex + 1; // Number of PM between two pauses

    p[i] = k;

  }

  return O_K;
}

/*------------------------------------------------------------
 Function:        TransitionNumberPerUnit

 Calculate the number of transitions in one unit (Number of PM in PMA1 and number of PM in PMA2 in one unit (between two pauses)).
 Number of transitions per unit = Number of PM per unit

 Parameter:
 a    PM from PMA1 (dPM1VoicedShift)
 m   Number of PM1 (nPM1VoicedEnerNew)
 b   PM from PMA2 (dPM2VoicedShift)
 n   Number of PM2 (nPM2VoicedEnerNew)
 c   Array of Initial PM
 d   Array of Terminal PM
 k   Number of initial or terminal PM
 nsam  Number of samples in speech signal
 e    Array for number of transitions for PMA1 per unit
 f    Array for number of transitions for PMA2 per unit

 ------------------------------------------------------------*/
INT16 CGEN_PRIVATE CPMproc::TransitionNumberPerUnit(INT32 *a, INT32 m, INT32 *b, INT32 n, INT32 *c, INT32 *d, INT32 k,
    INT32 nsam, INT32 *e, INT32 *f) {
  INT32 i = 0;
  INT32 j = 0;
  INT32 jj = 0;
  INT32 nAux = 0;
  INT32 nPMStartUnit = 0; // Start and End PM
  INT32 nPMEndUnit = 0;
  INT32 nStartIndexPM1 = 0; // Index of Start and End of PM1
  INT32 nEndIndexPM1 = 0;
  INT32 nStartIndexPM2 = 0; // Index of Start and End of PM2
  INT32 nEndIndexPM2 = 0;
  INT32 nPM1Unit = 0; // Number of PM1 per unit
  INT32 nPM2Unit = 0; // Number of PM2 per unit
  INT32 *aa = NULL; // New array for (a) with one extra PM
  INT32 *bb = NULL; // New array for (b) with one extra PM

  // Add one PM to the end of array PMA1 and PMA2 (the value of extra PM is the number of samples in signal)
  aa = (INT32*)dlp_calloc(m+1, sizeof(INT32)); // Allocates zero-initialize memory
  bb = (INT32*)dlp_calloc(n+1, sizeof(INT32)); // Allocates zero-initialize memory

  // Copy PM1
  for(i = 0; i < m; i++) {
    nAux = a[i];
    aa[i] = nAux;
  }
  aa[m] = nsam; // Last PM = Number of samples

  // Copy PM2
  for(i = 0; i < n; i++) {
    nAux = b[i];
    bb[i] = nAux;
  }
  bb[n] = nsam; // Last PM = Number of samples

  /////////////////////////////////////////////////
  // Calculate the number of transitions per unit
  for(i = 0; i < k; i++) {
    nPMStartUnit = c[i];
    nPMEndUnit = d[i];

    /////////////////////////////////////////////////
    // Calculate the number of PM in PMA1

    INT32 nNoBeginPM1 = 0;
    INT32 nNoEndPM1 = 0;
    INT32 nExitPM1 = 0;

    for(j = nEndIndexPM1; j < m; j++) {
      nAux = aa[j];
      nExitPM1 = 1;
      if(aa[j] == nPMStartUnit) {
        nStartIndexPM1 = j;
        break;
      } else if(aa[j] > nPMStartUnit && aa[j] <= nPMEndUnit) {
        nStartIndexPM1 = j;
        break;
      } else if(aa[j] < nPMStartUnit || aa[j] > nPMEndUnit) {
        nNoBeginPM1 = -1;
        break;
      }

    }

    for(j = nEndIndexPM1; j < m; j++) {
      nAux = aa[j];
      nExitPM1 = 1;
      if(aa[j] == nPMEndUnit) {
        nEndIndexPM1 = j + 1;
        break;
      } else if(aa[j] < nPMEndUnit && aa[j + 1] > nPMEndUnit) {
        nEndIndexPM1 = j + 1;
        break;
      } else if(aa[j] < nPMStartUnit || aa[j] > nPMEndUnit) {
        nNoEndPM1 = -1;
        break;
      }
    }

    nPM1Unit = nEndIndexPM1 - nStartIndexPM1; // number of PM1 per unit

    if(nExitPM1 == 0) // Exceed the index of PM1
    {
      nNoBeginPM1 = -1;
      nNoEndPM1 = -1;
    }

    if(nNoBeginPM1 == -1 && nNoEndPM1 == -1) // There is no transitions in unit
    nPM1Unit = 0;

    e[i] = nPM1Unit;

    /////////////////////////////////////////////////
    // Calculate the number of PM in PMA2
    INT32 nNoBeginPM2 = 0;
    INT32 nNoEndPM2 = 0;
    INT32 nExitPM2 = 0;

    for(jj = nEndIndexPM2; jj < n; jj++) {
      nAux = bb[jj];
      nExitPM2 = 1;
      if(bb[jj] == nPMStartUnit) {
        nStartIndexPM2 = jj;
        break;
      } else if(bb[jj] > nPMStartUnit && bb[jj] <= nPMEndUnit) {
        nStartIndexPM2 = jj;
        break;
      } else if(bb[jj] < nPMStartUnit || bb[jj] > nPMEndUnit) {
        nNoBeginPM2 = -1;
        break;
      }

    }

    for(jj = nEndIndexPM2; jj < n; jj++) {
      nAux = bb[jj];
      nExitPM2 = 1;
      if(bb[jj] == nPMEndUnit) {
        nEndIndexPM2 = jj + 1;
        break;
      } else if(bb[jj] < nPMEndUnit && bb[jj + 1] > nPMEndUnit) {
        nEndIndexPM2 = jj + 1;
        break;
      } else if(bb[jj] < nPMStartUnit || bb[jj] > nPMEndUnit) {
        nNoEndPM2 = -1;
        break;
      }
    }

    nPM2Unit = nEndIndexPM2 - nStartIndexPM2; // number of PM2 per unit

    if(nExitPM2 == 0) // Exceed the index of PM2
    {
      nNoBeginPM2 = -1;
      nNoEndPM2 = -1;
    }

    if(nNoBeginPM2 == -1 && nNoEndPM2 == -1) // There is no transitions in unit
    nPM2Unit = 0;

    f[i] = nPM2Unit;
  }

  dlp_free(aa);
  dlp_free(bb);

  return O_K;
}

/*------------------------------------------------------------
 Function:        FirstLastPMinUnits

 Detect the first PM and Last PM per units for PMA

 Parameter:
 a    PM from PMA (dPM1VoicedShift)
 m   Number of PM (nPM1VoicedEnerNew)
 c   Array of Initial PM
 d   Array of Terminal PM
 k   Number of initial or terminal PM
 nsam  Number of samples in speech signal
 e    Array for first PM in units in PMA
 f    Array for last PM in units in PMA

 ------------------------------------------------------------*/
/*INT16 CGEN_PRIVATE CPMproc::FirstLastPMinUnits(INT32 *a, INT32 m, INT32 *c, INT32 *d, INT32 k, INT32 nsam, INT32 *e, INT32 *f)
 {
 INT32 i = 0;
 INT32 j = 0;
 INT32 nAux = 0;
 INT32 nPMStartUnit = 0; // Start and End PM
 INT32 nPMEndUnit = 0;
 INT32 nStartIndexPM1 = 0; // Index of Start and End of PM1
 INT32 nEndIndexPM1 = 0;

 INT32 *aa = NULL;  // New array for (a) with one extra PM


 // Add one PM to the end of array PMA (the value of extra PM is the number of samples in signal)
 aa = (INT32*)dlp_calloc(m+1, sizeof(INT32));  // Allocates zero-initialize memory


 // Copy PM
 for(i = 0; i < m; i++)
 {
 nAux = a[i];
 aa[i] = nAux;
 }
 aa[m] = nsam; // Last PM = Number of samples



 // Calculate the number of transitions per unit
 for(i = 0; i < k; i++)
 {
 nPMStartUnit = c[i];
 nPMEndUnit = d[i];

 // Detect the first and last PM
 for(j = nEndIndexPM1; j < m; j++)
 {
 if(aa[j] == nPMStartUnit || aa[j] > nPMStartUnit)
 {
 nStartIndexPM1 = j;
 break;
 }
 }

 for(j = nEndIndexPM1; j < m; j++)
 {
 if(aa[j] == nPMEndUnit)
 {
 nEndIndexPM1 = j;
 break;
 }
 else if(aa[j] < nPMEndUnit && aa[j+1] > nPMEndUnit)
 {
 nEndIndexPM1 = j;
 break;
 }
 }

 e[i] = nStartIndexPM1;
 f[i] = nEndIndexPM1;
 }

 dlp_free(aa);

 return O_K;
 }
 */

/*------------------------------------------------------------
 Function:        PostprocessingPM

 Delete false PM

 Parameter:
 a    Speech signal
 sr  Sample Rate
 MEn  The Mean of Energy Contour
 b   Selected PM with (Confidence 1 or Confidence 2)
 n   Number of selected PM
 c   Return Selected PM after Postprocessing
 k   Return Number of PM after Postprocessing

 ------------------------------------------------------------*/
INT32 CGEN_PRIVATE CPMproc::PostprocessingPM(FLOAT64 *a, INT32 sr, FLOAT64 MEn, INT32 *b, INT32 n, INT32 *c) {
  FLOAT64 nEnerThre = 0; // Threshold of energy contour
  INT32 k = 0; // Return Number of PM after Postprocessing
  INT32 k1 = 0; // Number of PM after delete the PM in positive values of signal
  INT32 i = 0;
  INT32 nAuxPM = 0; // Position of PM insamples
  FLOAT64 nAuxVal = 0; // Value of signal by PM
  INT32 nFalseDistancePM = 0; // We assume that the distance between two PM is false, if this distance is smaller than (2 msec)
  INT32 k2 = 0; // Number of PM after delete the PM, when the distance is smaller than (2 msec)
  INT32 nAuxDif = 0;
  INT32 nAuxValue1 = 0;
  INT32 nAuxValue2 = 0;
  FLOAT64 nAuxSig1 = 0;
  FLOAT64 nAuxSig2 = 0;

  INT32 *cc = NULL;
  cc = (INT32*)dlp_calloc(n, sizeof(INT32));

  // Delete PM in positive values of speech signal and if signal value is very small
  nEnerThre = -0.03 * MEn; // Threshold of energy contour (negative values)

  for(i = 0; i < n; i++) {
    nAuxPM = b[i];
    nAuxVal = a[nAuxPM];
    if(nAuxVal < 0 && nAuxVal < nEnerThre) // Copy only PM in negative values of signal and smaller than negative Threshold
    {
      cc[k1] = b[i];
      k1 = k1 + 1;
    }
  }

  INT32 *ccc = NULL;
  ccc = (INT32*)dlp_calloc(k1, sizeof(INT32));

  // Delete PM, if the distance between two PM is smaller than (2 msec)
  nFalseDistancePM = (2 * sr) / 1000; // Distance = 2 msec

  for(i = 1; i < k1; i++) {
    nAuxValue1 = cc[i - 1];
    nAuxValue2 = cc[i];
    nAuxDif = nAuxValue2 - nAuxValue1;
    if(nAuxDif >= nFalseDistancePM) {
      ccc[k2] = cc[i - 1];
      k2 = k2 + 1;
    } else if(nAuxDif < nFalseDistancePM) {
      nAuxSig1 = a[nAuxValue1];
      nAuxSig2 = a[nAuxValue2];
      if(nAuxSig1 < nAuxSig2) {
        ccc[k2] = cc[i - 1];
        k2 = k2 + 1;
        cc[i] = cc[i - 1];
      } else if(nAuxSig1 >= nAuxSig2) {
        ccc[i - 1] = cc[i];
      }
    }
    // Process the final PM
    if(i == k1 - 1 && nAuxDif >= nFalseDistancePM) {
      ccc[k2] = cc[k1 - 1];
      k2 = k2 + 1;
    }
  }

  // Delete the (0) Values
  INT32 k3 = 0;

  INT32 *cccc = NULL;
  cccc = (INT32*)dlp_calloc(k2, sizeof(INT32));

  for(i = 0; i < k2; i++) {
    if(ccc[i] != 0) {
      cccc[k3] = ccc[i];
      k3 = k3 + 1;
    }
  }

  // Delete one of the equal values
  INT32 k4 = 0;

  INT32 *ccccc = NULL;
  ccccc = (INT32*)dlp_calloc(k3, sizeof(INT32));

  for(i = 1; i < k3; i++) {
    if(cccc[i] != cccc[i - 1]) {
      ccccc[k4] = cccc[i - 1];
      k4 = k4 + 1;
    }
    if(i == k3 - 1 && cccc[k3 - 1] != cccc[k3 - 2]) {
      ccccc[k4] = cccc[k3 - 1];
      k4 = k4 + 1;
    }
  }

  for(i = 0; i < k4; i++) {
    c[i] = ccccc[i];
  }

  dlp_free(cc);
  dlp_free(ccc);
  dlp_free(cccc);
  dlp_free(ccccc);

  k = k4;

  return k;
}

/*------------------------------------------------------------
 Function:        AddUnvoicedPM

 Add PM in unvoiced segments

 Parameter:
 a    Selected PM after Postprocessing (voiced)
 m   Number of selected PM (voiced)
 n   Number of samples in speech signal
 sr  Sample Rate
 b   Return Selected PM (voiced and unvoiced)
 k   Return Number of PM (voiced and unvoiced)

 ------------------------------------------------------------*/
INT32 CGEN_PRIVATE CPMproc::AddUnvoicedPM(INT32 *a, INT32 m, INT32 n, INT32 sr, INT32 *b) {
  INT32 k = 0; // Return Number of PM (voiced and unvoiced)
  INT32 i = 0;
  INT32 ii = 0;
  INT32 j = 0; // Number of (voiced and unvoiced) PM
  INT32 nMinDistancePM = 0; // Minimum Distance between two voiced PM, where there is no PM
  INT32 nUnvoicedDistPM = 0; // Distance between two unvoiced PM
  INT32 nNumUnvoicedPM = 0; // Number of unvoiced PM between two voiced PM
  INT32 nVoicedDiff = 0; // Difference between two voiced PM

  INT32 *c = NULL;

  // Minimum Distance between two voiced PM is (15 msec), where there is no PM
  nMinDistancePM = (15 * sr) / 1000; // Distance = 15 msec = 240 sample for 16000 Hz

  // Distance between two unvoiced PM
  nUnvoicedDistPM = 80; // Distance = 5 msec = 80 sample


  /* If there is no voiced PM then add only unvoiced PM */
  if(m == 0) {
    /* Add unvoiced PM in the speech signal */
    if(n >= nMinDistancePM) {
      nNumUnvoicedPM = n / nUnvoicedDistPM; // Number of unvoiced PM between two voiced PM
      for(ii = 0; ii < nNumUnvoicedPM - 1; ii++) {
        b[j] = -(nUnvoicedDistPM * (ii + 1));
        j = j + 1;
      }
    }
    k = j;
    return k;
  }

  /* Add unvoiced PM at the begin of speech signal */
  if(a[0] >= nMinDistancePM) {
    nNumUnvoicedPM = a[0] / nUnvoicedDistPM; // Number of unvoiced PM between two voiced PM

    for(ii = 0; ii < nNumUnvoicedPM - 1; ii++) {
      c = (INT32*)dlp_realloc(c, (j+1), sizeof(INT32));
      c[j] = -(nUnvoicedDistPM * (ii + 1));
      j = j + 1;
    }

    // Add the last voiced PM
    c = (INT32*)dlp_realloc(c, (j+1), sizeof(INT32));
    c[j] = a[0];
    j = j + 1;
  } else {
    c = (INT32*)dlp_realloc(c, (j+1), sizeof(INT32));
    c[j] = a[0];
    j = j + 1;
  }

  /* Add unvoiced PM within the speech signal */
  for(i = 1; i < m; i++) {
    // Add unvoiced PMs
    nVoicedDiff = a[i] - a[i - 1];
    if(nVoicedDiff >= nMinDistancePM) {
      nNumUnvoicedPM = nVoicedDiff / nUnvoicedDistPM; // Number of unvoiced PM between two voiced PM

      for(ii = 0; ii < nNumUnvoicedPM - 1; ii++) {
        c = (INT32*)dlp_realloc(c, (j+1), sizeof(INT32));
        c[j] = -((nUnvoicedDistPM * (ii + 1)) + a[i - 1]);
        j = j + 1;
      }

      // Add the last voiced PM
      c = (INT32*)dlp_realloc(c, (j+1), sizeof(INT32));
      c[j] = a[i];
      j = j + 1;
    } else {
      c = (INT32*)dlp_realloc(c, (j+1), sizeof(INT32));
      c[j] = a[i];
      j = j + 1;
    }
  }

  /* Add unvoiced PM at the end of speech signal */
  if(a[m - 1] < n && (n - a[m - 1]) > nMinDistancePM) // Distance between last voiced PM and last sample in signal is big
  {

    nNumUnvoicedPM = (n - a[m - 1]) / nUnvoicedDistPM; // Number of unvoiced PM between two voiced PM

    for(ii = 0; ii < nNumUnvoicedPM - 1; ii++) {
      c = (INT32*)dlp_realloc(c, (j+1), sizeof(INT32));
      c[j] = -((nUnvoicedDistPM * (ii + 1)) + a[m - 1]);
      j = j + 1;
    }

  }

  for(i = 0; i < j; i++) {
    b[i] = c[i];
  }

  dlp_free(c);

  k = j;
  return k;
}

/* EOF */
