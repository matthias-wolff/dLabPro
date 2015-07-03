// dLabPro class CProsody (Prosody)
// - Class CProsody - PM2F0 code
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
Program:        pm2f0

Generation of F0 data object (one column) aus PM data object (2 columns)

Prinzip:
In einem Zahlenvektor mit der gleichen Laenge wie das zugehoerige PM data object
werden die Laengen aller PM's geschrieben. Der Zahlenvektor wird mit der
gewuenschten sf_f0 (nSrateF0) abgetastet und, die Laengen der PM's in f0-Werte
umgerechnet, ausgegeben.

Voraussetzung:
eine PM ist genau so lang, wie die Periode Samples im PM data object hat.
D.h. PM.Laenge = sf_bin / f0 ( PM length = nSrate / f0 value )
Bsp: bei 16000Hz ATF(sf) und einer augenblicklichen f0 von 100 Hz
ist die Periode genau 160 Samples lang, d.h. hat die PM die Laenge 160.
Note: sf_bin = sf = nSrate

Ablauf:
es werden die Laengen aller PM's addiert und ein Zahlen-Vektor mit
dieser Laenge allociert.

Fuer jede gelesene PM werden (Anzahl=Laenge) Werte in den Zahlen-Vektor
geschrieben. Welche Werte geschrieben werden, haengt von Anregungsart
der aktuellen PM und der nachfolgenden PM ab.
[aktuelle PM= pm1], [nachfolgende PM=pm2]
pm1 stl && pm2 stl -> Werte = 0
pm1 stl && pm2 sth -> Werte = 0
pm1 sth && pm2 stl -> Werte = Laenge pm1
pm1 sth && pm2 sth -> Werte = lineare Interpolation zwischen Laenge pm1 und Laenge pm2

Der Zahlen-Vektor wird im Abstand von sf_bin / sf_f0 ( nSrate / nSrateF0) abgetastet
und die Laengen-Werte in f0-Werte umgerechnet ausgegeben.
------------------------------------------------------------*/

#include "dlp_prosody.h"        // Include class header file

typedef struct {
  FLOAT64 nF0value;
} F0_CONTOUR;

// Default: nSrate = 16000; nSrateF0 = 100

INT16 CGEN_PRIVATE CProsody::PmFo(data *dPM, INT32 nSrate, INT32 nSrateF0, data *dF0)    
{
  /* Initialization */
  INT32 i = 0;
  INT32 ii = 0;
  INT32 j = 0;  
  INT32 nPitchMark = 0;         // Number of Pitch Marks  
  FLOAT64 nAuxValue = 0;         // Auxiliary value
  INT32 nSumSamplePM = 0;         // Sum of PM from (0) to last PM in (samples)
  INT32 nSrate_Ratio = 0;        // Sample Rate Ratio = nSrate / nSrateF0


  
  /* Validation of data instance dF0 */
  if (!dF0 || !dF0->IsEmpty())    // If dF0 not exist or empty then return false
    return NOT_EXEC;            // NOT_EXEC = -1 (Generic error)
 
 
  /* Definition of data instance dF0 */
  // One column for F0 values (Hz)  
    dF0->AddNcomps(T_DOUBLE, 1);
    dF0->SetCname(0, "nF0");

  
     /* Number of Pitch Marks */
  nPitchMark  = dPM->GetNRecs();  // GetNRecs returns the number of valid records in the data object 
  //fprintf(stdout,"\nNumber of Pitch Marks:   %i\n",nPitchMark);

  /* Validation of data instance dPM */
  if ( nPitchMark == 0 )    // If there is no Pitch mark
  {
    //fprintf(stdout,"\nError: There is no Pitch Marks in the speech signal.");
    //return NOT_EXEC;            // NOT_EXEC = -1 (Generic error)
    return IERROR(this,NO_PM,0,0,0);
  }  

    
  /* Validation of sample rate */
    if(nSrate == 0)
    {
      //fprintf(stdout, "\nError: Sample rate for speech signal (PM file) is not specified");
      //return NOT_EXEC;
      return IERROR(this,NO_SRate,0,0,0);
    }


  /* Convert PM data object (2 columns [nPer][anreg]) to two separate columns */
  /*----------------------------------------------------------------------- 
     Important Information:
     The output of the method (-analyze) for PMA is a PM data object of type short.
     This data object consist of two separate columns:
     nPer  : is the distance between PM in samples
       anreg : is the Anregung [ (0) for (voiceless or stimmlos) and (1) for (voiced or stimmhaft)] 
                       *** Important ***
       * The data object of PMproc class or data objects with more than one component
       * were stored in memory (record after record) (the first line, second line, third line, ...)
       * i.e. [r,c] = {[0,0],[0,1],[1,0],[1,1],[2,0],[2,1],....}
       ---------------------------------------------------------------------*/
  INT16 *idPMnPer = NULL;  // PMA [nPer] = column 1 
  INT16 *idPManreg = NULL; // PMA [anreg] = column 2
  
  idPMnPer = (INT16*)dlp_calloc(nPitchMark, sizeof(INT16));  // Allocates zero-initialize memory
  idPManreg = (INT16*)dlp_calloc(nPitchMark, sizeof(INT16));  // Allocates zero-initialize memory
  
  

  for (i = 0; i < nPitchMark; i++)
  {
    nAuxValue = CData_Dfetch(dPM,i,0);
    idPMnPer[i] = (INT16)nAuxValue;
    //fprintf(stdout,"PMA-nPer %i = %i",i,idPMnPer[i]);
      
    nAuxValue = CData_Dfetch(dPM,i,1);
    idPManreg[i] = (INT16)nAuxValue;
    //fprintf(stdout,"\t\tPMA-Anreg %i = %i\n",i,idPManreg[i]);  
  }
  
  /*
  fprintf(stdout,"\n\nThe nPer and Anreg of PM: \n");  // show data 
  //for(i=0; i<nPitchMark; i++)
  for(i=0; i<100; i++)  
    fprintf(stdout,"nPer %i = %i \t\t Anreg %i = %i \n",i,idPMnPer[i],i,idPManreg[i]);
  */


  /* Calculate the length of PM from (0) to last PM in (samples) 
     and allocate a vector with this length */
  for (i=0; i<nPitchMark; i++)
  {
    nSumSamplePM = nSumSamplePM + idPMnPer[i];
  }
  //fprintf(stdout,"\n\nNumber of samples from (0) to last PM:   %i\n",nSumSamplePM);

  
  // Allocate memory
  INT32 *idExpandPM = NULL; 
  idExpandPM = (INT32*)dlp_calloc(nSumSamplePM, sizeof(INT32));  // Allocates zero-initialize memory

  /*----- Produce Zahlenvektor for PMs -----*/
  INT16 nCurrentPMnPer = 0;    // Length of current PM in samples
  INT16 nCurrentPManreg = 0;    // Anregung of current PM
  INT16 nNextPManreg = 0;      // Anregung of next PM


  for (i = 0; i < nPitchMark-1; i++)
  {
    nCurrentPMnPer = idPMnPer[i];    // Length of current PM in samples
    nCurrentPManreg = idPManreg[i];    // Anregung of current PM
    nNextPManreg = idPManreg[i+1];    // Anregung of next PM
    
     /*----- Produce Zahlenvektor for (nPitchMark-1) PMs -----*/
     // PM current is unvoiced && PM next is unvoiced  =>  Values = 0
     if ( nCurrentPManreg == 0 && nNextPManreg == 0 )
     {
       for( ii = 0; ii < nCurrentPMnPer ; ii++ )
       {
         idExpandPM[j] = (INT32) 0;
         j++;
       }
     }
     // PM current is unvoiced && PM next is voiced  =>  Values = 0     
     else if ( nCurrentPManreg == 0 && nNextPManreg == 1 )
     {
       for( ii = 0; ii < nCurrentPMnPer ; ii++ )
       {
         idExpandPM[j] = (INT32) 0;
         j++;
       }
     }     
     // PM current is voiced && PM next is unvoiced  =>  Values = nCurrentPMnPer
     else if ( nCurrentPManreg == 1 && nNextPManreg == 0 )
     {
       for( ii = 0; ii < nCurrentPMnPer ; ii++ )
       {
         idExpandPM[j] = (INT32) nCurrentPMnPer;
         //idExpandPM[j] = (INT32) 0;
         j++;
       }
     }     
     // PM current is voiced && PM next is voiced  =>  Values = nCurrentPMnPer
     else   // else if( nCurrentPManreg == 1 && nNextPManreg == 1 )
     {
       for( ii = 0; ii < nCurrentPMnPer ; ii++ )
       {
         idExpandPM[j] = (INT32) nCurrentPMnPer;
         j++;
       }
     }     
  }



    /*----- Produce Zahlenvektor for last PM -----*/
  nCurrentPMnPer = idPMnPer[nPitchMark-1];    // Length of last PM in samples
  nCurrentPManreg = idPManreg[nPitchMark-1];    // Anregung of last PM
  
  if ( nCurrentPManreg == 1 )
  {
     for( ii = 0; ii < nCurrentPMnPer ; ii++ )
     {
       idExpandPM[j] = (INT32) nCurrentPMnPer;
       j++;
     }
  }
  else 
  {
     for( ii = 0; ii < nCurrentPMnPer ; ii++ )
     {
       idExpandPM[j] = (INT32) 0;
       j++;
     }    
  }  
  
  /*  
  fprintf(stdout,"\n\nThe expand values of PM: \n");  // show data 
  //for(i=0; i<nPitchMark; i++)
  for(i=0; i<300; i++)  
    fprintf(stdout,"Expand values of PM %i = %i \n",i,idExpandPM[i]);
  */
  
  

  /*---------- Calculate F0 values ----------*/
  // Calculate Sample Rate Ratio = nSrate / nSrateF0;
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



  // Allocate memory for F0 values
  FLOAT64 *idContourF0 = NULL;

  INT32 nNumberF0 = 0;  // Number of F0 values in file

  for ( i = 0; i < (nSumSamplePM-nSrate_Ratio); i+=nSrate_Ratio )
  {
    if( idExpandPM[i+nSrate_Ratio] == 0 )
    {
      idContourF0 = (FLOAT64*)dlp_realloc(idContourF0, (nNumberF0+1), sizeof(FLOAT64));
      idContourF0[nNumberF0] = (FLOAT64) 0;
      nNumberF0 = nNumberF0 + 1;
    }
    else
    {
      idContourF0 = (FLOAT64*)dlp_realloc(idContourF0, (nNumberF0+1), sizeof(FLOAT64));
      //idContourF0[nNumberF0] = (FLOAT64) nSrate / idExpandPM[i+nSrate_Ratio];
      nAuxValue =  (FLOAT64) nSrate / idExpandPM[i+nSrate_Ratio];
      idContourF0[nNumberF0] = nAuxValue;
      nNumberF0 = nNumberF0 + 1;
    }    
    
  }

  // Delete single F0 value
  for ( i=1; i<nNumberF0-2; i++)
  {
    if(idContourF0[i]!=0 && idContourF0[i-1]==0 && idContourF0[i+1]==0)
    {
      idContourF0[i]=0;
    }
  }
  

  /* Write F0 values in struct */
    F0_CONTOUR *F0_contour = NULL;  // (F0_contour) is an object of struct (F0_CONTOUR)


  F0_contour = (F0_CONTOUR*)dlp_calloc(nNumberF0, sizeof(F0_CONTOUR));  // Allocates zero-initialize memory

  for(i = 0; i < nNumberF0; i++)  
  {
        (F0_contour + i)->nF0value = idContourF0[i];
        //fprintf( stdout,"nF0value  %i = %f\n",i,idContourF0[i] );
  }


    
  /* Copy F0 values from (struct F0_contour) to output data object (dF0) */
  dF0->AddRecs(nNumberF0, 1); // AddRecs: Appends (n) records to the end of the table (data object)  
  for( i = 0; i < nNumberF0; i++ )
  {
      dF0->Dstore((FLOAT64)(F0_contour + i)->nF0value, i, 0);
  }


  /*
  FLOAT64 nAuxCopy = 0;
  
  for (i = 0; i < nNumberF0; i++)
  {
    nAuxCopy = (FLOAT64)CData_Dfetch(dF0,i,0);
    fprintf(stdout,"F0 value %i = %f\n",i,nAuxCopy);  
  }
  */  
  
  dlp_free(idPMnPer);
  dlp_free(idPManreg);  
  dlp_free(idExpandPM);    
  dlp_free(idContourF0);
  dlp_free(F0_contour);    
      
  return O_K;
}



/* EOF */
