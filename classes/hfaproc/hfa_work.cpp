// dLabPro class CHFAproc (HFAproc)
// - Class HFAproc - HFA - a matrix based analysis
//
// AUTHOR : Rico Petrick, Mike Lorenz TU Dresden
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


#include "dlp_hfaproc.h"


INT16 CGEN_PUBLIC CHFAproc::AnalyzeMatrix(CData* idFrames, CData* idReal)
{
  static BYTE first_time = 1 ;


  if(first_time) 
  {
    first_time = 0;
    hfa_init(m_lpStrHFA);
  }

  hfa_analyze_matrix(idFrames, m_lpStrHFA , idReal, log10(m_nMinLog), m_nLimit);
  
  return(O_K);
}

void CGEN_VPUBLIC CHFAproc::PrepareOutput(CData* dResult)
{
  m_nOutDim = m_nCoeff+1;
  CFBAproc::PrepareOutput(dResult);
}

INT16 CGEN_PROTECTED CHFAproc::hfa_done(tHFA  *pstrHFA)
{
  hfa_destroy(pstrHFA);        // destroy HFA-struct substructures
  dlm_mf_done(&(pstrHFA->strMF));    // destroy convolution-core substructures

  //_hfa_structHFAdeinit(pstrHFA); pstrHFA darf hier nicht ge-free()-t werden !! dies wird derzeit automatisch erledigt, weil es zur klasse gehört
  
  return(O_K);
}

void CGEN_PROTECTED CHFAproc::hfa_init(tHFA    *pstrHFA)
{
    INT16 width;
    INT16 vad,i;
    FLOAT32 *pr4;
    FLOAT32 r4HzPerBin;
    INT16 pow;
 
    //_hfa_structHFAinit(pstrHFA); // edit: wird schon in der *.def allokiert
    

    // read from HFAproc member values ( e.g.: taken from config file in fea.itp(-FEA_pfa_init))
    pstrHFA->strCPAR.exp            = m_nExp;
    pstrHFA->strCPAR.v_offset       = m_nVOffset;
    pstrHFA->strCPAR.v_width        = m_nVWidth;
    pstrHFA->strCPAR.p_offset       = m_nPOffset;
    pstrHFA->strCPAR.p_width        = m_nPWidth;
    pstrHFA->strCPAR.r4F0_max       = m_nF0max;
    pstrHFA->strCPAR.r4F0_min       = m_nF0min;
    pstrHFA->strCPAR.r4VADmean_fac  = m_nVADmeanFac;
    pstrHFA->strCPAR.r4Thresh       = m_nVADthresh;
    pstrHFA->strCPAR.hfaswitch      = m_nHFAswitch;
  pstrHFA->strCPAR.dctswitch      = m_nDCTswitch;
  //pstrHFA->strCPAR.idftswitch      = m_nIDFTswitch; // just for testing
  //pstrHFA->strCPAR.atype      = m_lpsAtype; // doesn't work yet - TODO: fix this
  
    // read from FBAproc member values
    pstrHFA->strGPAR.r4T_s        = (1/m_nSrate)         ;
    pstrHFA->strGPAR.u2F_rate     = m_nCrate        ;
    pstrHFA->strGPAR.u2F_width    = m_nWlen        ;
    pstrHFA->strGPAR.u4f_s        = (UINT32)m_nSrate      ;
    pstrHFA->strGPAR.u2fft_length = m_nLen          ;

  IFCHECK {
      printf("\n T_s = %f",  pstrHFA->strGPAR.r4T_s);
      printf("\n F_rate = %i",  pstrHFA->strGPAR.u2F_rate);
      printf("\n F_width = %i",  pstrHFA->strGPAR.u2F_width);
      printf("\n Fs = %i",  pstrHFA->strGPAR.u4f_s);
      printf("\n FFT_length = %i\n",  pstrHFA->strGPAR.u2fft_length);
      
      if (!(pstrHFA->strCPAR.hfaswitch))
      {
        printf("\nHFA = OFF\n");
      }
      else
      {
        if(dlp_strncmp(dlp_strlwr(m_lpsWtype),"rectangle",255)) 
        {
               { printf( "\n abort in HFA module\n  uasr.pfa.wtype has to be \"Rectangle\" for HFA\n use uasr.hfa.wtypefft instead\n" );   exit(-1); }
        }
        printf("\nHFA = ON\n");   
        printf("\nHFA Parameters:");
        printf("\n exp = %i",pstrHFA->strCPAR.exp);
        printf("\n v_offset = %i",pstrHFA->strCPAR.v_offset);
        printf("\n v_width = %i",pstrHFA->strCPAR.v_width);
        printf("\n p_offset = %i",pstrHFA->strCPAR.p_offset);
        printf("\n p_width = %i",pstrHFA->strCPAR.p_width);
        printf("\n F0_max = %f",pstrHFA->strCPAR.r4F0_max);
        printf("\n F0_min = %f",pstrHFA->strCPAR.r4F0_min);
        printf("\n VADmean_fac = %f",pstrHFA->strCPAR.r4VADmean_fac);
        printf("\n VADthresh = %f\n",pstrHFA->strCPAR.r4Thresh);

      //printf("\n Analysistype = %s",pstrHFA->strCPAR.atype);
      printf("\nFilter = %s\n",m_lpsAtype);
      
      if (pstrHFA->strCPAR.dctswitch)
        printf("\nDCT = ON\n");
      else printf("\nDCT = OFF\n");
    }
  };
  
  pstrHFA->strGPAR.u1wtypefft  = _hfa_get_win_type(m_lpsWtypefft)       ;

    DLPASSERT(pstrHFA->strGPAR.u2fft_length <= N_FFT_MAX);

  //init normal primary feature analysis

    _hfa_makeWindowTable(  pstrHFA->strGPAR.u2F_width,
                           pstrHFA->strGPAR.u1wtypefft, 
                           pstrHFA->strGPAR.r4win_fft);
            
//  pstrHFA->strMF = (MLP_CNVC_TYPE*)dlp_calloc(1,sizeof(MLP_CNVC_TYPE));            
               
//  dlm_mf_init(  pstrHFA->strMF,
           
  #ifdef __MELPROC_DEPRECATED
  #warning using deprecated melfilter
  dlm_mf_init(  &(pstrHFA->strMF),
             (INT16)(pstrHFA->strGPAR.u2fft_length),
           m_nCoeff,
           16);
  #else
  dlm_mf_init(  &(pstrHFA->strMF),
             (INT16)(pstrHFA->strGPAR.u2fft_length),
           m_nCoeff,
           m_nMinLog);
  #endif
  
  
   /******* F0E **************************************************/

      // pstrHFA->strF0E.r4F0_max = pstrHFA->strCPAR.r4F0_max;
      // pstrHFA->strF0E.r4F0_min = pstrHFA->strCPAR.r4F0_min;

      //max and min idxes T0.. represents basically an index of the akf
      //equ T0 = fs/f0
      //the flooring is implicitly UINT16

      pstrHFA->strF0E.u2T0_max = UINT16( ((FLOAT32)pstrHFA->strGPAR.u4f_s) / pstrHFA->strF0E.r4F0_min );
      pstrHFA->strF0E.u2T0_min = UINT16( ((FLOAT32)pstrHFA->strGPAR.u4f_s) / pstrHFA->strF0E.r4F0_max );
  
      pstrHFA->strF0E.u2akf_len = pstrHFA->strGPAR.u2F_width;
      pstrHFA->strF0E.r4Thresh  = pstrHFA->strCPAR.r4Thresh;
      
   /******* HAR **************************************************/
      
      pstrHFA->strHAR.N_HarmMax = UINT16(0.5 * (FLOAT32)pstrHFA->strGPAR.u4f_s / pstrHFA->strF0E.r4F0_min);
      pstrHFA->strHAR.r4CutFreq =       (0.5 * (FLOAT32)pstrHFA->strGPAR.u4f_s);

   /******* VAD **************************************************/
     
      pstrHFA->strVAD.r4MeanFaktor = pstrHFA->strCPAR.r4VADmean_fac;

     
   /******* SYN **************************************************/
     
      pstrHFA->strSYN.exp = pstrHFA->strCPAR.exp;

      for(width = 0; width < MAX_WIDTH ; width++ )
      {
          for(i = 0; i < (2*MAX_WIDTH-1) ; i++ )
          {
             pstrHFA->strSYN.r4win[width][i]  = 0;
          }
      }


      for(width = 2; width < MAX_WIDTH ; width++ )
      {
        _hfa_makeWindowTable(  2*width-1,                       //width
                                1,                              //hanning
                               &pstrHFA->strSYN.r4win[width][0]); //start of window

        ///exponent of window function
        pow = 1;
        while( pow < pstrHFA->strSYN.exp )
        {
          for(i = 0; i < 2*MAX_WIDTH-1 ; i++ )
          {
             pstrHFA->strSYN.r4win[width][i]  *= pstrHFA->strSYN.r4win[width][i];
          }

          pow = pow<<1;
        }
      }



      r4HzPerBin = (FLOAT32)( (FLOAT32)(pstrHFA->strGPAR.u4f_s)  /  ( (FLOAT32)pstrHFA->strGPAR.u2fft_length ) );


      //flooring is implicitly to the cast
      pstrHFA->strSYN.strFad[VOICE].u2wBinSta = UINT16(  ((FLOAT32)(pstrHFA->strCPAR.v_offset                           )) /r4HzPerBin);
      pstrHFA->strSYN.strFad[VOICE].u2wBinSto = UINT16(  ((FLOAT32)(pstrHFA->strCPAR.v_offset + pstrHFA->strCPAR.v_width)) /r4HzPerBin);
      pstrHFA->strSYN.strFad[VOICE].u2wlen    = pstrHFA->strSYN.strFad[VOICE].u2wBinSto - pstrHFA->strSYN.strFad[VOICE].u2wBinSta + 1;

      pstrHFA->strSYN.strFad[PAUSE].u2wBinSta = UINT16(  ((FLOAT32)(pstrHFA->strCPAR.p_offset                           )) /r4HzPerBin);
      pstrHFA->strSYN.strFad[PAUSE].u2wBinSto = UINT16(  ((FLOAT32)(pstrHFA->strCPAR.p_offset + pstrHFA->strCPAR.p_width)) /r4HzPerBin);
      pstrHFA->strSYN.strFad[PAUSE].u2wlen    = pstrHFA->strSYN.strFad[PAUSE].u2wBinSto - pstrHFA->strSYN.strFad[PAUSE].u2wBinSta + 1;


      for (vad = 0; vad<2 ; vad++)
      {
        //this complicated structure is because the make_window funktion makes a full window
        //but we need only a half
        // pr4                                   = (FLOAT32 * )calloc( 2 * pstrHFA->strSYN.strFad[vad].u2wlen - 1 ,sizeof(FLOAT32)); 
        pr4                                   = (FLOAT32 * )dlp_calloc( 2 * pstrHFA->strSYN.strFad[vad].u2wlen ,sizeof(FLOAT32));        // edit: in make_windowtable wird -1 beachtet 
        pstrHFA->strSYN.strFad[vad].pr4win    = (FLOAT32 * )dlp_calloc(     pstrHFA->strSYN.strFad[vad].u2wlen     ,sizeof(FLOAT32));
        
      
        //make provisoric full window
        // _hfa_makeWindowTable( 2 * pstrHFA->strSYN.strFad[vad].u2wlen - 1,     //width
        _hfa_makeWindowTable( (UINT32) (2 * pstrHFA->strSYN.strFad[vad].u2wlen),  //edit: width
                                0,                                              //hamming
                                pr4);                                           //start of window

        //copy first half to the target array
        for (i = 0; i < pstrHFA->strSYN.strFad[vad].u2wlen; i++)
        {
          pstrHFA->strSYN.strFad[vad].pr4win[i] = pr4[i];
        }

        //free provisoric array
        dlp_free(pr4);

      }


        

    // return(pstrHFA);
}

