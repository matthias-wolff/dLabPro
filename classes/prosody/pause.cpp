// dLabPro class CProsody (Prosody)
// - Class CProsody - PAUSE code
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
  INT32 nPosition;
  INT32 nLength;
} PAUSE;


INT16 CGEN_PRIVATE CProsody::PauseDetect(data *dSignal, data *dPause)  
{
  /* Initialization */
  INT32 i = 0;
  INT32 nSrate = 0;             // Sample Rate
  INT32 nSamples = 0;         // Number of samples in WAV-signal (number of records in data instance)
  FLOAT64 *samples = NULL;         // WAV Signal in double
    INT32 nEnergyWindow = 160;      // Length of Window for calculating Short Time Energy Contour  
  INT32 nSmoothWindow = 60;      // Length of Window for smoothing the speech signal (Number of samples)  
    FLOAT64 *dEnergyContour = NULL;  // Short Time Energy Contour
    FLOAT64 *dSmoothEnergy = NULL;   // Smoothed Signal
  FLOAT64 nMeanEnergy = 0;       // The Mean of Energy Contour
  FLOAT64 nThresholdEnergy = 0;   // Threshold for Energy Contour
  FLOAT64 nAux = 0;         // Auxiliary variable

  
  /* Validation of data instance dPause */
  if (!dPause || !dPause->IsEmpty())    // If dPause not exist or empty then return false
    return NOT_EXEC;                // NOT_EXEC = -1 (Generic error)
 
  /* Definition of data instance dPause */
  // First column is the begin of pause in samples , second column is the length of pause in sample)  
    dPause->AddNcomps(T_LONG, 2);
    dPause->SetCname(0, "nPos");
    dPause->SetCname(1, "nLeng");

  
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


   
  /* Calculate Short Time Energy Contour and smooth the Short Time Energy Contour */
  dEnergyContour = (FLOAT64*)dlp_calloc(nSamples, sizeof(FLOAT64));
  dSmoothEnergy = (FLOAT64*)dlp_calloc(nSamples, sizeof(FLOAT64));


  ShortTimeEnergy(samples, nSamples, nEnergyWindow, dEnergyContour);      // Calculate the Short Time Energy Contour  
  MovingAverage(dEnergyContour, nSamples, nSmoothWindow, dSmoothEnergy);   // Smoothing the energy with Moving Average (Gleitende Mittelung)
  
  nMeanEnergy = MeanValue(dSmoothEnergy, nSamples);  // The Mean of smoothed energy contour
  nThresholdEnergy = 0.01 * nMeanEnergy;             // Threshold for Energy Contour (Threshold = 2% from Mean of Energy)
  //fprintf(stdout,"Threshold for Energy Contour:   %f\n",nThresholdEnergy);


  /*    
    fprintf(stdout,"\n The samples of smoothed energy signal are: \n");  // show data 
  for(i=0; i<10; i++)               
    fprintf(stdout,"Smoothed energy Sample %i = %f\n",i,dSmoothEnergy[i]);

    fprintf(stdout,"\n The samples of smoothed energy signal are: \n");  // show data 
  for(i=nSamples-10; i<nSamples; i++)               
    fprintf(stdout,"Smoothed energy Sample %i = %f\n",i,dSmoothEnergy[i]);
  */


  /* Modify the first and last values of smoothed short time energy signal */
  // Set the value of first sample and last sample of smoothed short time energy contour greater than the threshold of energy.
  nAux = 2 * nThresholdEnergy; 
  
  if( dSmoothEnergy[0] <= nThresholdEnergy ) // Modify the first value
  {
    dSmoothEnergy[0] = nAux;
  }
  if( dSmoothEnergy[nSamples-1] <= nThresholdEnergy ) // Modify the last value
  {
    dSmoothEnergy[nSamples-1] = nAux;
  }

  /*
    fprintf(stdout,"\n The samples of smoothed energy signal are: \n");  // show data 
  for(i=0; i<10; i++)               
    fprintf(stdout,"Smoothed energy Sample %i = %f\n",i,dSmoothEnergy[i]);
  
    fprintf(stdout,"\n The samples of smoothed energy signal are: \n");  // show data 
  for(i=nSamples-10; i<nSamples; i++)               
    fprintf(stdout,"Smoothed energy Sample %i = %f\n",i,dSmoothEnergy[i]);
  */




  /* Detect the begin (t1) and end (t2) of pauses */   
  INT16 nInit = 0;       // Number of Initial pauses
  INT16 nTerm = 0;       // Number of Terminal pauses

    
  INT32 *dInitialPause = NULL;  // Array of initial pauses
  INT32 *dTerminalPause = NULL; // Array of terminal pauses

  
  for( i = 0; i < nSamples-1; i++ )
  {
    if( dSmoothEnergy[i] > nThresholdEnergy && dSmoothEnergy[i+1] <= nThresholdEnergy ) // Detection the initial pauses
    {
      dInitialPause = (INT32*)dlp_realloc(dInitialPause, (nInit + 1), sizeof(INT32));
      dInitialPause[nInit] = i+1; // Begin a pause (position of Pause in sample)
      nInit = nInit + 1;
    }
      
    else if ( dSmoothEnergy[i] <= nThresholdEnergy && dSmoothEnergy[i+1] > nThresholdEnergy ) // Detection the terminal pauses
    {
      dTerminalPause = (INT32*)dlp_realloc(dTerminalPause, (nTerm + 1), sizeof(INT32));
      dTerminalPause[nTerm] = i; // End a pause (position of Pause in sample)
      nTerm = nTerm + 1;
    }        
  }
  

  /*  
    fprintf(stdout,"\n The initial pauses are: \n");  // show data 
  for(i=0; i<nInit; i++)               
    fprintf(stdout,"Initial Pause %i = %d\n",i,dInitialPause[i]);

    fprintf(stdout,"\n The terminal pauses are: \n");  // show data 
  for(i=0; i<nTerm; i++)               
    fprintf(stdout,"Terminal Pause %i = %d\n",i,dTerminalPause[i]);
  */


  // Return error and exit if number of initial pauses is not equal the number of terminal pauses
  if( nInit != nTerm )
  {
    //fprintf(stdout,"\nERROR: Number of initial pauses is not equal to the number of terminal pauses\n");
    //return NOT_EXEC;
    return IERROR(this,FALSE_PAUSE,0,0,0);
  }
  

  
  // Calculate the length of pause
  for( i = 0; i < nInit; i++ )
  {
    if( dTerminalPause[i] > dInitialPause[i] )
    {
      dTerminalPause[i] = dTerminalPause[i] - dInitialPause[i] + 1; // length of pause = t2 - t1
    }
    else if( dTerminalPause[i] <= dInitialPause[i] )
    {
      dTerminalPause[i] = 0;
    }
  }

  /*  
    fprintf(stdout,"\n The length of pauses are: \n");  // show data 
  for(i=0; i<nTerm; i++)               
    fprintf(stdout,"Length of Pause %i = %d\n",i,dTerminalPause[i]);
  */    
      

  /* Delete pauses which are smaller than certain length */
  INT32 nMinPauseLeng = 0;  // Minimal length of pause
  
  nMinPauseLeng = nSrate/10; // Minimal length of pause is 100 msec (100 msec = 1600 sample for 16kHz)
  //fprintf(stdout,"Minimal length of pause:   %d sample\n",nMinPauseLeng);
  

  INT16 nBegin = 0;       // Final number of (begin) pauses
  INT16 nEnd = 0;       // Final number of (end) pauses
      
  INT32 *dBeginPause = NULL; // Array indicates the begin of pause in samples
  INT32 *dEndPause = NULL; // Array indicates the end of pause in samples
  

  for( i = 0; i < nInit; i++ )
  {
    if( dTerminalPause[i] >= nMinPauseLeng )
    {
      // Begin
      dBeginPause = (INT32*)dlp_realloc(dBeginPause, (nBegin + 1), sizeof(INT32));
      dBeginPause[nBegin] = dInitialPause[i]; // Begin a pause (position of Pause in sample)
      nBegin = nBegin + 1;
      // End
      dEndPause = (INT32*)dlp_realloc(dEndPause, (nEnd + 1), sizeof(INT32));
      dEndPause[nEnd] = dTerminalPause[i]; // End a pause (length of Pause in sample)
      nEnd = nEnd + 1;
    }    
  }  
  

  /* Write Pause in struct */
    PAUSE *PausesArray = NULL;  // (PausesArray) is an object of struct (PAUSE)
    INT32 nPauseNum = 0;         // Final number of pauses 

  PausesArray = (PAUSE*)dlp_calloc(nBegin, sizeof(PAUSE));  // Allocates zero-initialize memory


  for(nPauseNum = 0; nPauseNum < nBegin; nPauseNum++)  
  {
        (PausesArray + nPauseNum)->nPosition = dBeginPause[nPauseNum];
        //fprintf( stdout,"nPosition  %i = %d\n",nPauseNum,dBeginPause[nPauseNum] );
        (PausesArray + nPauseNum)->nLength = dEndPause[nPauseNum];
        //fprintf( stdout,"anreg  %i = %d\n",nPauseNum,dEndPause[nPauseNum] );
  }


    
  /* Copy pauses from (struct PausesArray) to output data object (dPause) */
  dPause->AddRecs(nBegin, 1); // AddRecs: Appends (n) records to the end of the table (data object)  
  for( i = 0; i < nBegin; i++ )
  {
      dPause->Dstore((FLOAT64)(PausesArray+i)->nPosition, i, 0);
      dPause->Dstore((FLOAT64)(PausesArray+i)->nLength, i, 1);
  }




  /* Destroys an instance of a dLabPro class (data class) */
  //IDESTROY(idPause);                // Destroy idPause
  
  /* free  */            
  dlp_free(samples);
  dlp_free(dEnergyContour);
  dlp_free(dSmoothEnergy);
  dlp_free(dInitialPause);
  dlp_free(dTerminalPause);
  dlp_free(dBeginPause);
  dlp_free(dEndPause);
  dlp_free(PausesArray);
  

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
FLOAT64 CGEN_PRIVATE CProsody::MaximumValue(FLOAT64 *a, INT32 n)
{
  FLOAT64 x = 0;
  INT32 i = 0; 
  
  for(i = 0; i < n; i++)
  {
    if(x < a[i])
      x = a[i];
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
FLOAT64 CGEN_PRIVATE CProsody::MinimumValue(FLOAT64 *a, INT32 n)
{
  FLOAT64 x = 0;
  INT32 i = 0; 
  
  for(i = 0; i < n; i++)
  {
    if(x > a[i])
      x = a[i];
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
INT16 CGEN_PRIVATE CProsody::SignalNorming(FLOAT64 *a, INT32 n, FLOAT64 *b)
{
  INT32 i = 0;
  FLOAT64 maximum = 0;
  FLOAT64 minimum = 0;

  maximum = MaximumValue(a, n); 
  minimum = MinimumValue(a, n); 


  FLOAT64 MinAbsolut = fabs(minimum);  //Absoluter Wert

  FLOAT64 v = maximum;  // Auxiliary Variable

  if (MinAbsolut >= v)
    v = MinAbsolut;     //Suche nach Maximum ( von negativen und positiven Werten ) 


  for (i = 0; i < n; i++)
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
INT16 CGEN_PRIVATE CProsody::MovingAverage(FLOAT64 *a, INT32 n, INT32 m, FLOAT64 *b)
{
    INT32 i = 0; 
    INT32 j = 0; 
    FLOAT64 s = 0;

  FLOAT64 *c = NULL;
  c = (FLOAT64*)dlp_calloc(n+m, sizeof(FLOAT64));  // Allocates zero-initialize memory
  
  //FLOAT64 *c;            // auxiliary data vector
  //c=new FLOAT64 [n+m];   // length = length of signal + length of smoothing window
  
  for (i = 0; i < (n+m); i++)    // initial the auxiliary data vector (c) with (0)  
    c[i] = 0; 

  
  for (i = 0; i < n; i++)    // copy the original signal (a) to the auxiliary data vector 
    c[i] = a[i]; 

  
  for (i = 0; i < n; i++)   // Smoothing the signal
  {
    s = 0; 
    for (j = i; j < (i+m); j++) 
    {
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
INT16 CGEN_PRIVATE CProsody::ShortTimeEnergy(FLOAT64 *a, INT32 n, INT32 m, FLOAT64 *b)
{
  INT32 i = 0; 
  INT32 j = 0;   
  INT32 nNullAdd = 0; // Add zeros in the beginning and end of signal 
                     // (H�lfte der Fenasterlaenge beim Anfang und H�lfte der Fenasterlaenge beim Ende)
    FLOAT64 QuadValue = 0;
    FLOAT64 EnergValue = 0;
                     
    nNullAdd = m / 2; 
 
   FLOAT64 *c = NULL;
  c = (FLOAT64*)dlp_calloc(n+m, sizeof(FLOAT64));  // Allocates zero-initialize memory
     
    //FLOAT64 *c;            // Auxiliary data vector
  //c=new FLOAT64 [n+m];   // length = length of signal + length of energy window
    
  
  for (i = 0; i < (n+m); i++)    // Initial the auxiliary data vector (c) with (0)  
    c[i] = 0; 

  
  for (i = 0; i < n; i++)    // Copy the original signal (a) to the auxiliary data vector with offset of (nNullAdd) 
    c[i+nNullAdd] = a[i]; 

  
    
  for (i = nNullAdd; i < (n+nNullAdd); i++)        // Calculating the energy 
  {    
    EnergValue = 0; 
    for (j=i-nNullAdd; j<(i+nNullAdd); j++) 
    {
      QuadValue = 0; 
      QuadValue = c[j]*c[j]; 
      EnergValue += QuadValue;     
    }     
    b[i-nNullAdd] = EnergValue; 
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
FLOAT64 CGEN_PRIVATE CProsody::MeanValue(FLOAT64 *a, INT32 n)
{
  INT32 i = 0;
  FLOAT64 x = 0;
  FLOAT64 xx = 0;
  FLOAT64 b = 0;

  for(i=0;i<n;i++)
    x = x + a[i];            // Die Summe des gesamten Signals 

  xx = x / n;          //Mittelwert fuer das gesamte Signal 
  b = xx;

  return (b);
}


/* EOF */
