/* dLabPro class CHFAproc (HFAproc)
 * - hfa working functions
 *
 * AUTHOR : Rico Petrick, Mike Lorenz
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


#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "dlp_kernel.h"
#include "dlp_base.h"
#include "dlp_math.h"
#include "dlp_hfaproc.h"


/*********************************************************************
 *   HFA main module
 *********************************************************************/

//      INT16 * pi2_test1;


/* *********************************************************/
/*!
 * @brief allocates structure and does basic inits
*/

void CGEN_PRIVATE CHFAproc::_hfa_structHFAinit(tHFA *pstrHFA)
{

  if(pstrHFA) dlp_free(pstrHFA);
  pstrHFA=(tHFA *) dlp_calloc(1,sizeof(tHFA));

}



/* *********************************************************/
/*!
 * @brief destroys HFA structure
*/
void CGEN_PRIVATE CHFAproc::_hfa_structHFAdeinit(tHFA *pstrHFA)
{

  if(pstrHFA) dlp_free(pstrHFA);

}


INT16 CGEN_PRIVATE CHFAproc::hfa_analyze_matrix(CData *idFrames, tHFA *pstrHFA , CData *idReal, FLOAT64 nMinLog, FLOAT64 nLimit)
{

  tMXr   strSIGt;    //time frames
  tMXr   strSIGf;    //spectral frames
  tMXr   strHFAf;    //HFA spectral frames
  UINT32  u4A = 0;    //number of frames
  UINT32  a;
  INT32  i;
  FLOAT64  *pr8fft_real = NULL;
  FLOAT64  *pr8fft_imag = NULL;
  FLOAT64  *pr8_out;
  FLOAT64  *pr4mel_out;
  FLOAT32  r4tmp;



  //temp. for mel
  INT16 ret = O_K;
  FLOAT64* ptr= NULL;
  //end for mel



  u4A = idFrames->GetNRecs();


     //allocate signals

        //Matrix for framed time domain signal
      strSIGt.u2MxHig   =  pstrHFA->strGPAR.u2fft_length;
      strSIGt.u2MxLen   =  u4A;
      strSIGt.strMx.len =  strSIGt.u2MxHig * strSIGt.u2MxLen;
      strSIGt.strMx.pr4 =  (FLOAT32 *)calloc(strSIGt.strMx.len, sizeof(FLOAT32));

        //Matrix for magnitude spectra
      strSIGf.u2MxHig   =  pstrHFA->strGPAR.u2fft_length/2;
      strSIGf.u2MxLen   =  u4A;
      strSIGf.strMx.len =  strSIGf.u2MxHig * strSIGf.u2MxLen;
      strSIGf.strMx.pr4 =  (FLOAT32 *)calloc(strSIGf.strMx.len, sizeof(FLOAT32));

        //Matrix for HFA output spectra
      strHFAf.u2MxHig   =  pstrHFA->strGPAR.u2fft_length/2;
      strHFAf.u2MxLen   =  u4A;
      strHFAf.strMx.len =  strHFAf.u2MxHig * strHFAf.u2MxLen;
      strHFAf.strMx.pr4 =  (FLOAT32 *)calloc(strHFAf.strMx.len, sizeof(FLOAT32));


  pr8fft_real = (FLOAT64*)dlp_calloc(pstrHFA->strGPAR.u2fft_length, sizeof(FLOAT64));
  pr8fft_imag = (FLOAT64*)dlp_calloc(pstrHFA->strGPAR.u2fft_length, sizeof(FLOAT64));

  pr4mel_out  = (FLOAT64*)dlp_calloc(pstrHFA->strMF.n_out+1 , sizeof(FLOAT64));


  /* ---------------------------------------------------------------------*/
  /* -----derive spectrogram ---------------------------------------------*/
  /* ---------------------------------------------------------------------*/




  /* counting over frames */
  for (a = 0; a<u4A; a++)
  {


      memset( pstrHFA->r4x_in, 0, pstrHFA->strGPAR.u2fft_length * sizeof(FLOAT32));


      pr8_out = (FLOAT64 *)idFrames->XAddr(a, 0);
          for (i = 0; i < pstrHFA->strGPAR.u2F_width; i++)
          {
        r4tmp                      = (FLOAT32)pr8_out[i];
        strSIGt.strMx.pr4[ strSIGt.u2MxHig*a + i ]   = r4tmp;
           pstrHFA->r4x_in[i]               = r4tmp;
          }

          /* ---------------------------------------------------------------------*/
          /* -----apply timewindow -----------------------------------------------*/
          /* ---------------------------------------------------------------------*/
          _hfa_window (  pstrHFA->strGPAR.u2F_width,
                         pstrHFA->strGPAR.u1wtypefft,
                         &strSIGt.strMx.pr4[ strSIGt.u2MxHig*a ],
                         pstrHFA->r4x_in,
                         pstrHFA->strGPAR.r4win_fft);


      for (i = 0; i < pstrHFA->strGPAR.u2fft_length; i++)
      {
          pr8fft_real[i] = (FLOAT64)pstrHFA->r4x_in[i];
          pr8fft_imag[i] = 0.0;
      }
      /* ----- DO FFT ----*/

      if((ret = dlm_fft(  pr8fft_real,
                  pr8fft_imag,
                  pstrHFA->strGPAR.u2fft_length,
                  0)) != O_K)
      {
        dlp_free(pr8fft_real);
        dlp_free(pr8fft_imag);
        return ret;
      }

      /* ----- abs spectrum ----*/

      pstrHFA->r4x_in[0] = (FLOAT32)(sqrt(pr8fft_real[0]*pr8fft_real[0] + pr8fft_imag[0]*pr8fft_imag[0]));
      for (i = 1; i <= pstrHFA->strGPAR.u2fft_length/2; i++)
      {
           pstrHFA->r4x_in[i] = (FLOAT32)(sqrt(pr8fft_real[i]*pr8fft_real[i] + pr8fft_imag[i]*pr8fft_imag[i]));
      }




          /* ---------------------------------------------------------------------*/
          /* -----remove DC        -----------------------------------------------*/
          /* ---------------------------------------------------------------------*/
          //pstrHFA->r4x_in[0]=0.;

          /* ---------------------------------------------------------------------*/
          /* -----copy spectrum to spectrogram------------------------------------*/
          /* ---------------------------------------------------------------------*/
          memcpy(&strSIGf.strMx.pr4[ strSIGf.u2MxHig*a ], pstrHFA->r4x_in, (strSIGf.u2MxHig * sizeof(FLOAT32)));
  }




  /* ---------------------------------------------------------------------*/
  /* -----hfa              -----------------------------------------------*/
  /* ---------------------------------------------------------------------*/
  if (pstrHFA->strCPAR.hfaswitch==1)
  {
    hfa( pstrHFA, &strSIGt, &strSIGf , &strHFAf);
  }
  else
  {
  for(a = 0; a<strHFAf.strMx.len; a++)
  {
    strHFAf.strMx.pr4[a] =  strSIGf.strMx.pr4[a];
  }
  }


  /* ---------------------------------------------------------------------*/
  /* -----melfilter        -----------------------------------------------*/
  /* ---------------------------------------------------------------------*/


  /* counting over frames */
  for (a = 0; a<u4A; a++)
  {

  pstrHFA->r4x_in[0] = strHFAf.strMx.pr4[ strHFAf.u2MxHig*a ];

  /* log spectrum  */
  for (i = 1; i < pstrHFA->strGPAR.u2fft_length/2; i++)
  {

    pstrHFA->r4x_in[i                    ] = log10(0.00000001f + strHFAf.strMx.pr4[ strHFAf.u2MxHig*a + i]);
    pstrHFA->r4x_in[pstrHFA->strGPAR.u2fft_length - i -1] = pstrHFA->r4x_in[i] ;

  }

  /* convolve log spectrum with convolution core */

    dlm_mf_convolve(&(pstrHFA->strMF), pstrHFA->r4x_in, pr4mel_out);

  pr8_out = (FLOAT64*)idReal->XAddr(a,0);

  /* limiting */

    for (i=0, ptr=pr4mel_out; i<=pstrHFA->strMF.n_out; i++, ptr++)
    {
      if (  *ptr> nLimit )
            *ptr = nLimit;
      if (  *ptr< -nLimit)
            *ptr = -nLimit;
      *(pr8_out++) = (FLOAT64)(*ptr);
    }

    /* compute DCT if switch == 1 */

  if ((pstrHFA->strCPAR.dctswitch)==1)
  {
    dct_ii((FLOAT64*)idReal->XAddr(a,0));
  }

//    /* compute iDFT if switch == 1 */
//
//  if ((pstrHFA->strCPAR.idftswitch)==1)
//  {
//    idft((FLOAT64*)idReal->XAddr(a,0));
//  }
  }


  free(strSIGt.strMx.pr4);
  free(strSIGf.strMx.pr4);
  free(strHFAf.strMx.pr4);

  dlp_free(pr8fft_real);
  dlp_free(pr8fft_imag);
  dlp_free(pr4mel_out);

  return(O_K);
}





/* *********************************************************/
/*!
 * @brief deinits HFA structure
*/
void CGEN_PRIVATE CHFAproc::hfa_destroy(tHFA *pstrHFA)
{

    INT16 vad;

    for (vad = 0; vad<2 ; vad++)
    {
      if (pstrHFA->strSYN.strFad[vad].pr4win)
      {
        dlp_free(pstrHFA->strSYN.strFad[vad].pr4win);
      }
    }
}


/* *********************************************************/
/*!
 * @brief HFA run time function
*/
void CGEN_PRIVATE CHFAproc::hfa(tHFA *pstrHFA, tMXr *pstrSIGt, tMXr *pstrSIGf, tMXr *pstrOUT)
{
      str_r4arr   strF0;
      str_i2arr   strVP;
      tMXi        strHIdx;


      pstrHFA->u4A = pstrSIGt->u2MxLen;


   //allocate signals


        //array for fundamental freq
      strF0.len           = pstrHFA->u4A;
      strF0.pr4           = (FLOAT32 *)calloc( pstrHFA->u4A , sizeof(FLOAT32));

        //matrix for harmonic indices
      strHIdx.u2MxHig     = pstrHFA->strHAR.N_HarmMax;
      strHIdx.u2MxLen     = pstrHFA->u4A;
      strHIdx.strMx.len   = strHIdx.u2MxHig * strHIdx.u2MxLen;
      strHIdx.strMx.pi2   = (INT16 *)calloc( strHIdx.strMx.len , sizeof(INT16));

        //array for VAD decission
      strVP.len           = pstrHFA->u4A;
      strVP.pi2           = (INT16 *)calloc( strVP.len , sizeof(INT16));


   //run sub modules



      _hfa_F0E(pstrHFA, pstrSIGt, &strF0);
      _hfa_HAR(pstrHFA, &strHIdx, &strF0);
      _hfa_VAD(pstrHFA, &strHIdx, pstrSIGf, &strVP);
      _hfa_SYN(pstrHFA, &strHIdx, pstrSIGf, &strVP, pstrOUT);




   //deallocate signals

      free(strF0.pr4);
      free(strHIdx.strMx.pi2);
      free(strVP.pi2);

}


/*********************************************************************
 *   _hfa_F0E() estimation of the fundamental frequency
 *********************************************************************/


/* *********************************************************/
/*!
 * @brief run time function for F0E detection
 *
 *  input:  tHFA *pstrHFA     module structure
 *          tMXr *pstrSIGt    input signal matrix
 *          str_r4arr *pstrF0 array for sequence of F0
 *  output: str_r4arr *pstrF0 sequence of F0
*/
void CGEN_PRIVATE CHFAproc::_hfa_F0E(tHFA *pstrHFA, tMXr *pstrSIGt, str_r4arr *pstrF0)
{

  UINT16 a,i;
  FLOAT32 *px; //helper pointer

  UINT16 max_idx = 0;   // max index for max search

  for (a= 0; a<pstrSIGt->u2MxLen; a++  )
  {

      //pointer to a-th frame
      px = &pstrSIGt->strMx.pr4[pstrSIGt->u2MxHig * a];

      //AKF
      akf_positiv(  px,
                    pstrHFA->strF0E.r4akf,
                    pstrHFA->strF0E.u2akf_len);

      //pointer to akf frame
      px = pstrHFA->strF0E.r4akf;


      //maximum search

      max_idx = pstrHFA->strF0E.u2T0_min;
      for(  i = pstrHFA->strF0E.u2T0_min; i < pstrHFA->strF0E.u2T0_max;    i++)
      {
          if( px[i] > px[max_idx] )
          {
              max_idx = i;
          }
      }

      pstrF0->pr4[a] = ((FLOAT32)pstrHFA->strGPAR.u4f_s) / ( (FLOAT32)max_idx);
  }



  //post processing

  _hfa_F0E_postProc(pstrF0, pstrHFA->strF0E.r4Thresh);

}


/* *********************************************************/
/*!
 * @brief run time function for postprocessing of F0 detection
 *        deletes sigle or double impulses
 *
 *  input:  str_r4arr *pstrIN   in array, implies length
 *          float r4Thresh threshold
 *  output: float *pr4In   out array
*/
void CGEN_PRIVATE CHFAproc::_hfa_F0E_postProc(str_r4arr *pstrIN, FLOAT32 r4Thresh)
{
    UINT32 i;
    FLOAT32 r4delta1;
    FLOAT32 r4delta2;
    FLOAT32 r4delta3;

    FLOAT32 r4previous = 0;

    FLOAT32 *pr4In   = pstrIN->pr4;
    UINT32  u4L     = pstrIN->len;


    for(i = 1; i < u4L-2; i++)
    {
        r4delta1 = pr4In[i]  - r4previous;

        if(fabs(r4delta1) > r4Thresh)
        {
            //if first delta has reached the threshold derive second and third delta
            r4delta2 = pr4In[i+1] - r4previous;
             r4delta3 = pr4In[i+2] - r4previous;

            if(fabs(r4delta2) < r4Thresh) //single impulse
            {
                pr4In[i] = r4previous;  //gets deleted (deleted means set previos in the gap)
            }
            else                        //double impulse
            {


              if(  (r4delta1 * r4delta2) >= 0 )   // sign(delta1) == sign(delta2)
              {
                if(fabs(r4delta3) < r4Thresh)
                {
                    //first gets deleted, (deleted means set previos in the gap)
                    pr4In[i] = r4previous;
                    //second remains as single impuls and next sample appears as single and gets deleted
                }
                else
                {
                  if( (r4delta1 * r4delta3) < 0 )  //sign(r4delta1) != sign(r4delta3)
                  {
                    pr4In[i] = r4previous;
                  }
                }
              }
              else
              {
                //if delta sign changes, two impulsies in oposite
                //directions, but no step
                //first gets deleted, (deleted means set previos in the gap)
                pr4In[i] = r4previous;
                //second remains as single impuls and next sample appears as single and gets deleted
              }
            }
        }

        r4previous = pr4In[i];
    }
}





/* *********************************************************/
/*!
 * @brief derives the positive half of the akf
 *  input:  float *pr4x,   in array
 *          float *pr4akf, pointer to the out array
 *         UINT16 u2len    length
 *  output: float *pr4akf   out array
*/
void CGEN_PRIVATE CHFAproc::akf_positiv(FLOAT32 *pr4x, FLOAT32 *pr4akf, UINT16 u2len)
{
      INT16 t,k;
      FLOAT32 r4sum;

      for( t = 0; t<u2len; t++  )
      {
          r4sum = 0;

          for( k = 0; k<(u2len - t); k++)
          {
            r4sum += pr4x[k]*pr4x[k+t];
          }

          pr4akf[t] = r4sum;
      }
}







/*********************************************************************
 *   _hfa_HAR() finding of the harmonic indices
 *********************************************************************/


/* *********************************************************/
/*!
 * @brief run time function for HAR
 *
 *  input:  tHFA *pstrHFA     module structure
 *          str_r4arr *pstrF0 array for sequence of F0
 *  output: tMXi  *pstrHIdx   matrix for indices
 */
void CGEN_PRIVATE CHFAproc::_hfa_HAR(tHFA *pstrHFA, tMXi  *pstrHIdx, str_r4arr   *pstrF0)
{
    INT16 *pi2;
    INT16 a,i2Hf;
    FLOAT32 r4BaseIdx;

    for(a = 0 ; a< pstrHFA->u4A; a++)
    {
       pi2 = &pstrHIdx->strMx.pi2[ pstrHIdx->u2MxHig * a ];

       r4BaseIdx = pstrF0->pr4[a] * pstrHFA->strGPAR.u2fft_length * pstrHFA->strGPAR.r4T_s;

       i2Hf = 1;

       //derive and fill the harmonic indices
       while( i2Hf * pstrF0->pr4[a] < pstrHFA->strHAR.r4CutFreq    &&   i2Hf <= pstrHFA->strHAR.N_HarmMax )
       {
          pi2[ i2Hf - 1 ] = (INT16)(( r4BaseIdx * i2Hf ) + 0.5 );  //+0.5 for rounding mathematically

          i2Hf++;
       }
    }
}







/*********************************************************************
 *   _hfa_VAD() voiced unvoiced Detection
 *********************************************************************/

/* *********************************************************/
/*!
 * @brief run time function for VAD
 *
 *  input:  tHFA *pstrHFA     module structure
 *          tMXi  *pstrHIdx   matrix for indices
 *          tMXr *pstrSIGf    original Spectrogram
 *
 *  output: str_i2arr *pstrVP array for VAD decission
 */
void CGEN_PRIVATE CHFAproc::_hfa_VAD(tHFA *pstrHFA, tMXi  *pstrHIdx, tMXr *pstrSIGf, str_i2arr   *pstrVP)
{


  INT16 *pi2Idx;
  FLOAT32 *pr4Org;
  INT16 a,i2Hf;
  FLOAT32 r4tmp;
  FLOAT32 r4sum;
  FLOAT32 r4first;
  FLOAT32 r4last;
  FLOAT32 r4mean;
  FLOAT32 r4prev;
  FLOAT32 r4curr;

  FLOAT32 *pr4ARR;

  pr4ARR = (FLOAT32 *)calloc( pstrHFA->u4A , sizeof(FLOAT32));


  //build power of harmonics///////////////////////////

  for(a= 0; a< pstrHFA->u4A; a++ )
  {

    pi2Idx  = &pstrHIdx->strMx.pi2[ pstrHIdx->u2MxHig * a ];
    pr4Org  = &pstrSIGf->strMx.pr4[ pstrSIGf->u2MxHig * a ];

    i2Hf  = 0;
    r4sum = 0;

    while( pi2Idx[ i2Hf ] > 0    &&  i2Hf < pstrHFA->strHAR.N_HarmMax)
    {

      r4tmp  = pr4Org[pi2Idx[i2Hf]];   //harmonic spectral component

      r4sum += r4tmp * r4tmp;                   //accu and square


      i2Hf++;
    }

    pr4ARR[a] = r4sum;

  }


  //smooth power of harmonics//////////////////////////

  r4first = 1.5 * (pr4ARR[0]            + pr4ARR[1]);
  r4last  = 1.5 * (pr4ARR[pstrHFA->u4A-1] + pr4ARR[pstrHFA->u4A - 2]);
  r4mean  = 0;
  r4prev  = pr4ARR[0];
  for(a= 1; a< pstrHFA->u4A-1; a++ )
  {

    r4curr    = pr4ARR[a];

    pr4ARR[a] = r4prev + pr4ARR[a] + pr4ARR[a+1];

    r4prev    = r4curr;

    r4mean += pr4ARR[a];
  }

  pr4ARR[0             ] = r4first;
  pr4ARR[pstrHFA->u4A-1] = r4last;
  r4mean   += r4first + r4last;



  //derive threshold

  r4mean  /=   pstrHFA->u4A;
  r4mean  *=   pstrHFA->strVAD.r4MeanFaktor;


  // actual decission
  for(a= 0; a< pstrHFA->u4A; a++ )
  {
    if( pr4ARR[a] >= r4mean)
    {
      pstrVP->pi2[a] = VOICE;
    }
    else
    {
      pstrVP->pi2[a] = PAUSE;
    }
  }


  //smooth power of harmonics//////////////////////////


  free(pr4ARR);
}







/*********************************************************************
 *   _hfa_SYN() spectral synthesis
 *********************************************************************/



/* *********************************************************/
/*!
 * @brief run time function for SYN
 *
 *  input:  tHFA *pstrHFA     module structure
 *          tMXi  *pstrHIdx   matrix for indices
 *          tMXr *pstrSIGf    original Spectrogram
 *          str_i2arr *pstrVP array for VAD decission
 *
 *  output: tMXr *pstrOUT     Synthesized hfa output spectrum
 */
void CGEN_PRIVATE CHFAproc::_hfa_SYN(tHFA *pstrHFA, tMXi  *pstrHIdx, tMXr *pstrSIGf, str_i2arr   *pstrVP, tMXr *pstrOUT)
{

  FLOAT32 r4Syn[N_FFT_MAX/2];
  INT16 *pi2Idx;
  FLOAT32 *pr4Org;
  FLOAT32 *pr4Out;
  INT16 a,i,k,i2Hf,width,actIdx;
  UINT16 vad;
  tFade *pstrFad;
  INT16 lower;
  INT16 upper;

 UINT16 u2fft_length = pstrHFA->strGPAR.u2fft_length;




  for(a= 0; a< pstrHFA->u4A; a++ )
  {

      pr4Org  = &pstrSIGf->strMx.pr4[ pstrSIGf->u2MxHig * a ];
      pr4Out  = &pstrOUT ->strMx.pr4[ pstrOUT ->u2MxHig * a ];



      vad     = pstrVP->pi2[a];



   //make synthetic spectr or zero//////////////////////////////


      if(vad == VOICE)
      {//voiced frame...synthesis
          for (i = 0; i<u2fft_length/2 ; i++)
          {
            r4Syn[i] = 0.0;
          }


          pi2Idx  = &pstrHIdx->strMx.pi2[ pstrHIdx->u2MxHig * a ];


          i2Hf  = 0;
          width = pi2Idx[0]+1;

          while(  pi2Idx[ i2Hf ] > 0    &&  i2Hf < pstrHFA->strHAR.N_HarmMax)
          {
            actIdx = pi2Idx[i2Hf];


            lower = actIdx - width + 1;
            upper = actIdx + width - 1;
            if(lower <  0      ) lower = 0;
            if(upper >= u2fft_length/2) upper = u2fft_length/2 - 1;


            for(i = lower, k= 0; i <= upper ; i++, k++)
            {
              r4Syn[i] += ( pr4Org[actIdx] * pstrHFA->strSYN.r4win[width][k] ) ;
            }

            i2Hf++;
          }
      }
      else
      {//unvoiced frame...zero
          for (i = 0; i<u2fft_length/2 ; i++)
          {
            r4Syn[i] = 0.0;
          }
      }




   //combine synt spec and org spec//////////////////////////////

      pstrFad = &pstrHFA->strSYN.strFad[vad];

      //make noisefloor
      for (i = 0                          ; i < pstrOUT->u2MxHig      ; i++)
      {



        pr4Out[i] = 0.002;  //dummy make_noise;
      }

#if 1


      //synthetic in lower freqs
      for (i = 0                          ; i < pstrFad->u2wBinSta  ; i++)
      {
        pr4Out[i] =      r4Syn[i]
                    +   pr4Out[i];
      }

      //intersection between synthetic org spec
      for (i = pstrFad->u2wBinSta, k = 0  ; i < pstrFad->u2wBinSto +1    ; i++, k++)
      {
        pr4Out[i] =     pstrFad->pr4win[pstrFad->u2wlen-1-k] *  r4Syn[i]
                    +   pstrFad->pr4win[k                  ] * pr4Org[i]
                    +   pr4Out[i];
      }

      //org spec in higher freqs
      for (i = pstrFad->u2wBinSto + 1     ; i < pstrOUT->u2MxHig      ; i++)
      {
        pr4Out[i] =     pr4Org[i]
                    +   pr4Out[i];
      }

#endif


  } // block loop

}




/*!
 * @brief makes a table of a window function
 *
 * makes a table of a window function
 * which is used to multiply with a cut
 * time or frequency window. therefor a
 * memory array of the size of "length_window"
 * is allocated and filled with the window
 * values. the return value is a pointer
 * to the window table
 *
 * @param length_window -  length of window in samples (i)
 * @param type_of_window - window type                  (i)
 *                - 0 = Hamming
 *                - 1 = Hanning
 *                - 2 = Blackman
 *                - 3 = Bartlett
 *                - other = rectangular
 *
 * @return float pointer to table of function values of window
 */
void CGEN_PRIVATE CHFAproc::_hfa_makeWindowTable( UINT32 length_window, BYTE type_of_window,
                           FLOAT32 *pr4window_table)
{
  UINT32 i    = 0;
  FLOAT32 step = ( 2 * (FLOAT32)F_PI / (FLOAT32)(length_window - 1) );


  if(type_of_window > 3)return;

  /* apply hamming-window */
  if ( type_of_window == 0 )
  {
    for (i=0; i<length_window; i++)
    {
      pr4window_table[i] = (FLOAT32)( 0.54 - 0.46 * cos( (FLOAT32)i * step ) );
    }
  }

  /* apply hanning-window */
  else if (type_of_window==1)
  {
    for (i=0; i<length_window; i++)
    {
      pr4window_table[i] = (FLOAT32)( 0.5 - 0.5 * cos( (FLOAT32)i * step ) );
    }
  }

  /* apply blackman-window */
  else if (type_of_window==2)
  {
    for (i=0; i<length_window; i++)
    {
      pr4window_table[i] = (FLOAT32)( 0.42 - 0.5 * cos( (FLOAT32)i * step ) +
                0.08 * cos( 2* (FLOAT32)i * step ) );
    }
  }

  /* apply bartlett-window */
  else if (type_of_window==3)
  {
    for (i=0; i< (length_window/2); i++)
    {
      pr4window_table[i] =   (2*(FLOAT32)i/(FLOAT32)length_window);
    }
    for (i = (length_window/2)+1; i< length_window; i++)
    {
      pr4window_table[i] = 2-(2*(FLOAT32)i/(FLOAT32)length_window);
    }
  }

  /* do nothing when applying Rectangular window */
  return;
}






/*!
 * @brief apply windowing function to sample sequence of samples
 *
 * @param length_window - size in samples of one window
 * @param type_of_window - sample sequence; window
 *                 - 0       : hamming - window
 *                 - 1       : hanning - window
 *                 - 2       : blackman - window
 *                 - 3       : bartlett - window
 *                 - default : rectangle - window
 * @param *frame - sample sequence; window
 * @param *pr4window_table - list with window
 *
 * @return signed long  - OKAY                 - error free processing

 * @attention   input-sequence and output-sequence use same array
 *              rectangle-windowing-function will be performed by
 *              applying no operation to sample sequence
 */
INT32 CGEN_PRIVATE CHFAproc::_hfa_window(UINT32  length_window, BYTE  type_of_window,
          FLOAT32 *px_in, FLOAT64 *px_out, FLOAT32 *pr4window_table)
{
  UINT32 i    = 0;

  if (type_of_window == 4)
  {
    /* do nothing when applying Rectangular window */
      return(OKAY);
  }
  else
  {
    for (i = 0; i < length_window; i++)
    {
      px_out[i] = px_in[i] * pr4window_table[i];
    }
  }
  return(OKAY);
}


BYTE CGEN_PRIVATE CHFAproc::_hfa_get_win_type(char * pi1Wtype)
{
    if(!dlp_strncmp(dlp_strlwr(pi1Wtype),"hamming",255))
      return(0);
    else if(!dlp_strncmp(dlp_strlwr(pi1Wtype),"hanning",255))
      return(1) ;
    else if(!dlp_strncmp(dlp_strlwr(pi1Wtype),"blackman",255))
      return(2) ;
    else if(!dlp_strncmp(dlp_strlwr(pi1Wtype),"triangle",255))
      return(3) ;
    else if(!dlp_strncmp(dlp_strlwr(pi1Wtype),"bartlett",255))
      return(3) ;
    else if(!dlp_strncmp(dlp_strlwr(pi1Wtype),"rectangle",255))
      return(4);
    else
      return(4);
}

//
// dct_ii
// calculating 1-dimensional DCT II for MFCC
//
// Input: mel-filtered features (size: 30 elements)
//
// shortening the argument: cos((pi*k*(2*n+1))/(2*nFilters)) = cos((pi/nFilters)*k*(n+0.5));
//

void CGEN_PRIVATE CHFAproc::dct_ii(FLOAT64  *pr8_out)
{
    INT32 n;              // counter
    INT32 k;              // counter for number of cepstral coefficents
    INT16 nFilters;          // number of filters
  FLOAT64 tmp_pi;

    FLOAT64* real = (FLOAT64*)pr8_out;
    FLOAT64* tmp;

  tmp = (FLOAT64*) dlp_calloc(30,sizeof(FLOAT64));

  nFilters = 30;

  for (n = 0; n < 30; n++)    //counting over mel-filter coefficients
  {
    real[n] += real[30];    // re-addition of the average energy stored in element number 30
  }

  tmp_pi=F_PI/nFilters;      // creating temporary value for faster computation

     for (n = 0; n < nFilters; n++)  // counting over filters
     {
    tmp[n]=0;
    for (k = 0; k < 30; k++)
    {
      tmp[n] += real[k]* cos(tmp_pi*k*(n+0.5));
    }
     }

    for (n = 1; n < 30; n++)
    {
    real[n] = tmp[n];
  }

  dlp_free(tmp);
}

// idft for realpart only
// unused. just for testing

void CGEN_PRIVATE CHFAproc::idft(FLOAT64  *pr8_out)
{
    INT32 n;              // counter
    INT32 k;              // counter for number of cepstral coefficents
    INT16 nFilters;          // number of filters
  FLOAT64 tmp_pi;

    FLOAT64* real = (FLOAT64*)pr8_out;
    FLOAT64* tmp;

  tmp = (FLOAT64*) dlp_calloc(30,sizeof(FLOAT64));

  nFilters = 30;

  for (n = 0; n < 30; n++)    //counting over mel-filter coefficients
  {
    real[n] += real[30];
  }

  tmp_pi=2*F_PI/nFilters;

     for (n = 0; n < nFilters; n++)  // counting over filters
     {
    tmp[n]=0;
    for (k = 0; k < 30; k++)
    {
      tmp[n] += real[k]* cos(tmp_pi*k*n);
    }
     }

    for (n = 1; n < 30; n++)
    {
    real[n] = tmp[n]/nFilters;
  }

  dlp_free(tmp);
}



/* ------ EOF hfa.c --------------------------------------------------*/

