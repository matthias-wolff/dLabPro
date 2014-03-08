// dLabPro class CProsody (Prosody)
// - Class CProsody - ADD-RMS code
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

/*------------------------------------------------------------
Program:        add-rms
------------------------------------------------------------*/

#include "dlp_prosody.h"        // Include class header file

typedef struct {
  FLOAT64 nF0value;
  INT16 nStimulation;
  FLOAT64 nRMS;
  INT16 nNULL;
} F0_RMS;

// Default: nSrate = 16000 Hz; nSrateF0 = 100Hz

INT16 CGEN_PRIVATE CProsody::AddRms(data *dSignal, data *dF0, INT32 nSrate, INT32 nSrateF0, data *dF0rms)    
{
  /* Initialization */
  INT32 i = 0;
  INT32 t = 0;
  INT32 jj = 0;
  INT32 nSamples = 0;           // Number of samples in WAV-signal (number of records in data instance)
  FLOAT64 *samples = NULL;           // WAV Signal in double
  INT32 nNumberF0 = 0;         // Number of F0 values  
  INT32 nSrate_Ratio = 0;        // Sample Rate Ratio = nSrate / nSrateF0
    
  FLOAT64 nAuxValue = 0;         // Auxiliary value
  //INT32 nSumSamplePM = 0;       // Sum of PM from (0) to last PM in (samples)
  INT16 nHalfLengWind = 0;      // Half length of window for calculating the energy
  INT16 nTotalLengWind = 0;      // Total length of window for calculating the energy
  FLOAT64 nEnergy = 0;          // Energy for every window (Root Mean Square - RMS)
  FLOAT64 nValueF0 = 0;        // F0 value
  INT32 nF0rmsNumber = 0;        // Number of F0 values with RMS 
  
  /* Validation of data instance dF0rms */
  if (!dF0rms || !dF0rms->IsEmpty())    // If dF0rms not exist or empty then return false
    return NOT_EXEC;                // NOT_EXEC = -1 (Generic error)
 
 
  /* Definition of data instance dF0rms */
  // One column for F0 values (Hz)  
    dF0rms->AddNcomps(T_DOUBLE, 4);
    dF0rms->SetCname(0, "nF0");
    dF0rms->SetCname(1, "anreg");
    dF0rms->SetCname(2, "RMS");
    dF0rms->SetCname(3, "Extra");

  
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

  
     /* Number of F0 values */
  nNumberF0  = dF0->GetNRecs();  // GetNRecs returns the number of valid records in the data object 
  //fprintf(stdout,"\nNumber of F0 values:   %i\n",nNumberF0);

  /* Validation of data instance dPM */
  if ( nNumberF0 == 0 )    // If there is no Pitch mark
  {
    //fprintf(stdout,"\nError: There is no F0 values.");
    //return NOT_EXEC;            // NOT_EXEC = -1 (Generic error)
    return IERROR(this,NO_F0,0,0,0);
  }  



  /* Convert F0 data object to one column */
  FLOAT64 *idF0 = NULL;  // F0
  
  idF0 = (FLOAT64*)dlp_calloc(nNumberF0, sizeof(FLOAT64));  // Allocates zero-initialize memory


  for (i = 0; i < nNumberF0; i++)
  {
    nAuxValue = CData_Dfetch(dF0,i,0);
    idF0[i] = (FLOAT64)nAuxValue;
    //fprintf(stdout,"F0_value %i = %f",i,idF0[i]);
  }
  /*
  fprintf(stdout,"\n\nThe F0 values: \n");  // show data 
  //for(i=0; i<nNumberF0; i++)
  for(i=0; i<100; i++)  
    fprintf(stdout,"F0_value %i = %f\n",i,idF0[i]);
  */



    
  /* Validation of sample rate */
    if(nSrate == 0)
    {
      //fprintf(stdout, "\nError: Sample rate for speech signal is not specified");
      //return NOT_EXEC;
      return IERROR(this,NO_SRate,0,0,0);
    }

  /* Calculate Sample Rate Ratio */
  // Sample Rate Ratio = nSrate / nSrateF0;
  if ( nSrateF0 == 0 )      // Divide by zero (false !!!)
  {
      //fprintf(stdout, "\nError: Divide by zero.");
      //return NOT_EXEC;
      return IERROR(this,Div_Zero,0,0,0);
  }
  else
  {
    nSrate_Ratio = (INT32) nSrate / nSrateF0;
  }
  //fprintf(stdout,"\nSample Rate Ratio:   %i\n",nSrate_Ratio);




  /* Define the length of window for calculating the Energy */
  nHalfLengWind = (INT16)nSrate_Ratio / 2;        
  nTotalLengWind = nHalfLengWind * 2;    // Total Length of window



  
  /******** Calculate the Energy (Root Mean Square - RMS) ********/
  
  /* Define an object from the struct F0_RMS */
    F0_RMS *f0_rms = NULL;  // (f0_rms) is an object of struct (F0_RMS)

  f0_rms = (F0_RMS*)dlp_calloc(nNumberF0, sizeof(F0_RMS));  // Allocates zero-initialize memory

  
  jj = 0;        // Index for F0 values
  for (t = nSrate_Ratio; t < nSamples - nSrate_Ratio/2; t=t+nSrate_Ratio)
  {
    if(jj<nNumberF0)
    {
      /* Energy for every window */
      nEnergy = 0;
        // Mean of squared sample values 
        for(i = t-nHalfLengWind; i <= t+nHalfLengWind; i++) 
        {
            nEnergy = nEnergy + (samples[i]*samples[i]) / (FLOAT64)nTotalLengWind;
            //nEnergy = nEnergy + (samples[i]*samples[i]);           
        }
      //nEnergy = nEnergy / (FLOAT64)nTotalLengWind;
    
        // Square Root 
        nEnergy = (FLOAT64) sqrt(nEnergy);
    
      /* Read F0 value */
      nValueF0 = idF0[jj];

      /* Write (F0 and RMS) in struct */
      if(nValueF0 > 0)      // voiced segment
      {
            (f0_rms + jj)->nF0value = nValueF0;
            //fprintf( stdout,"F0 Value  %i = %f\t\t",jj,nValueF0 );
            (f0_rms + jj)->nStimulation = 1;
            //fprintf( stdout,"Anregung  %i = %d\t\t",jj,1 );
            (f0_rms + jj)->nRMS = nEnergy;
            //fprintf( stdout,"Energy  %i = %f\t\t",jj,nEnergy );
            (f0_rms + jj)->nNULL = 0;
            //fprintf( stdout,"Null  %i = %d\n",jj,0 ); 
      }
      else            // voicelos segment (F0 = 0)
      {
            (f0_rms + jj)->nF0value = nValueF0;
            //fprintf( stdout,"F0 Value  %i = %f\t\t",jj,nValueF0 );
            (f0_rms + jj)->nStimulation = 0;
            //fprintf( stdout,"Anregung  %i = %d\t\t",jj,0 );
            (f0_rms + jj)->nRMS = nEnergy;
            //fprintf( stdout,"Energy  %i = %f\t\t",jj,nEnergy );
            (f0_rms + jj)->nNULL = 0;
            //fprintf( stdout,"Null  %i = %d\n",jj,0 ); 
      }      
      jj = jj + 1;
      nF0rmsNumber = nF0rmsNumber + 1;
    }
  }
  //fprintf(stdout,"\nNumber of F0 values with RMS:   %i\n",nF0rmsNumber);
  
  
  /* Copy (F0 and RMS) from (struct f0_rms) to output data object (dF0rms) */
  dF0rms->AddRecs(nF0rmsNumber, 1); // AddRecs: Appends (n) records to the end of the table (data object)  
  for( i = 0; i < nF0rmsNumber; i++ )
  {
      dF0rms->Dstore((FLOAT64)(f0_rms+i)->nF0value, i, 0);
      dF0rms->Dstore((FLOAT64)(f0_rms+i)->nStimulation, i, 1);
      dF0rms->Dstore((FLOAT64)(f0_rms+i)->nRMS, i, 2);
      dF0rms->Dstore((FLOAT64)(f0_rms+i)->nNULL, i, 3);
  }
  

  // free
  dlp_free(samples);
  dlp_free(idF0);
  dlp_free(f0_rms);
  
      
  return O_K;
}



/* EOF */
