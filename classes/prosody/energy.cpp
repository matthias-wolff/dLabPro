// dLabPro class CProsody (Prosody)
// - Class CProsody - ENERGY code
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


#include "dlp_prosody.h"        // Include class header file

typedef struct {
  FLOAT64 nEnergyValue;
} STEnergy_CONTOUR;      // Short Time Energy Contour


INT16 CGEN_PRIVATE CProsody::EnergyContour(data *dSignal, data *dEnergy)
{
  /* Initialization */
  INT32 i = 0;
  INT32 j = 0;
  INT32 nSrate = 0;             // Sample Rate
  INT32 nSamples = 0;         // Number of samples in WAV-signal (number of records in data instance)
  INT32 nWindowShift = 0;      // Shift of Windows (samples)
  INT32 nWindowLength = 0;      // Length of Window (samples)
  INT32 nShiftNumber = 0;      // Number of Shifts in the signal
  INT32 nWindowNumber = 0;      // Number of windows
  FLOAT64 nAuxEner = 0;      // Auxiliary variable to calculate the energy for one window
  FLOAT64 *samples = NULL;         // WAV Signal in double
  FLOAT64 *dEnergyContour = NULL;  // Short Time Energy Contour
  
  
  /* Validation of data instance dEnergy */
  if (!dEnergy || !dEnergy->IsEmpty())    // If dEnergy not exist or empty then return false
    return NOT_EXEC;                  // NOT_EXEC = -1 (Generic error)
 
  /* Definition of data instance dEnergy */
  // One column is the values of energy contour  
    dEnergy->AddNcomps(T_DOUBLE, 1);
    dEnergy->SetCname(0, "nEnergy");


  
  /* Compute Sample Rate (SR) */
  //nSrate = m_nSrate;
  //nSrate  = (INT32)(1000.0/CData_GetDescr(dSignal,RINC));
  
  //# "\n Distance between two Samples = ${idSig.rinc} [msec] "  -echo;
  //# 1000 idSig.rinc / nSrate =;          # Compute Sample Rate (SR)  
  nSrate = (INT32)((1000.0/CData_GetDescr(dSignal,RINC)) + 0.5);  /* +0.5 forces cast to round up or down */    
  //fprintf(stdout,"Sample Rate:   %d Hz\n\n",nSrate);
  
  
     /* Number of samples in WAV-signal */
  nSamples  = dSignal->GetNRecs();  // GetNRecs returns the number of valid records in the data instance
  //fprintf(stdout,"Number of samples in WAV-signal:   %i\n\n",nSamples);


  /* Copies data from memory area to another memory area */
  samples = (FLOAT64*)dlp_calloc(nSamples, sizeof(FLOAT64));  // dlp_calloc(NUM,SIZE) Allocates zero-initialize memory
    dlp_memmove(samples, dSignal->XAddr(0,0), nSamples * sizeof(FLOAT64));
      
    /*
    fprintf(stdout,"\n The samples of speech signal are: \n");  // show data 
  for(i=0; i<nSamples; i++)               
    fprintf(stdout,"Signal Sample %i = %f\n",i,samples[i]);
  */

  /* Calculate the Window-Length and the Window-Shift */
  nWindowShift = nSrate / 100;      // Shift = 10 msec = 16000 / 100 = 160 samples
  nWindowLength = nWindowShift * 3;  // Window Length = 30 msec = Window Shift * 3
  
  
  nShiftNumber = nSamples / nWindowShift;    // Number of Shifts in the whole signal
  nWindowNumber = nShiftNumber - 2;      // Number of Windows
  
  if (nWindowNumber < 1)            // There is no window (signal is too short)
  {
    nWindowNumber = 1;            // Set number of windows to 1
    nWindowLength = nSamples;
  }
  
  dEnergyContour = (FLOAT64*)dlp_calloc(nWindowNumber, sizeof(FLOAT64));
  
  
  /* Fensterung des Signals und Berechnung der Energy f�r jedes Fensters */
  for (i=0; i<nWindowNumber; i++)
  {
    nAuxEner = 0;
    for (j=0; j<nWindowLength; j++)
    {
      nAuxEner = nAuxEner + samples[i*nWindowShift+j]*samples[i*nWindowShift+j];  
    }
    dEnergyContour[i] = nAuxEner;
  }
    
    

  /* Write Energy values in struct */
    STEnergy_CONTOUR *Energy_contour = NULL;  // (Energy_contour) is an object of struct (STEnergy_CONTOUR)


  Energy_contour = (STEnergy_CONTOUR*)dlp_calloc(nWindowNumber, sizeof(STEnergy_CONTOUR));  // Allocates zero-initialize memory

  for(i = 0; i < nWindowNumber; i++)  
  {
        (Energy_contour + i)->nEnergyValue = dEnergyContour[i];
        //fprintf( stdout,"nEnergyValue  %i = %f\n",i,dEnergyContour[i] );
  }


    
  /* Copy Energy values from (struct Energy_contour) to output data object (dEnergy) */
  dEnergy->AddRecs(nWindowNumber, 1); // AddRecs: Appends (n) records to the end of the table (data object)  
  for( i = 0; i < nWindowNumber; i++ )
  {
      dEnergy->Dstore((FLOAT64)(Energy_contour + i)->nEnergyValue, i, 0);
  }


  /*
  FLOAT64 nAuxCopy = 0;
  
  for (i = 0; i < nWindowNumber; i++)
  {
    nAuxCopy = (FLOAT64)CData_Dfetch(dEnergy,i,0);
    fprintf(stdout,"F0 value %i = %f\n",i,nAuxCopy);  
  }
  */  
  
  dlp_free(samples);    
  dlp_free(dEnergyContour);
  dlp_free(Energy_contour);    
      
  return O_K;
}


/* EOF */
