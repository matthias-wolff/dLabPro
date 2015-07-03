/* dLabPro mathematics library
 * - Fast Fourier-t transformation
 *
 * AUTHOR : Frank Duckhorn
 * PACKAGE: dLabPro/base
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
#include <math.h>
#include "dlp_kernel.h"
#include "dlp_base.h"
#include "dlp_math.h"

INT16 CGEN_IGNORE dlm_ftt_analyze(MLP_FTT_TYPE* lpFtt, void* frame, INT32 nLen, INT32 nOut, FLOAT64 nMinLog) {
  INT32   k,i;
  FLOAT64  fx,a,Tg,D=0.0;
  FLOAT64 x=0.0,y,rr,ri;
  FLOAT64* sigTemp=NULL;

  sigTemp = (FLOAT64*)dlp_calloc(nLen, sizeof(FLOAT64));
  dlp_memmove(sigTemp, frame, nLen * sizeof(FLOAT64));

  for (k=0; k<nOut; k++)
  { fx = 2.0f * F_PI * lpFtt->midfreq[k]/lpFtt->atf;
    a= lpFtt->bandwidth[k] * F_PI;
    y=(-a /(lpFtt->atf));
    y=exp(y);

    if(lpFtt->sm_coeff!=0)
    {
       if(lpFtt->midfreq[k]< 3000.f)
          Tg = lpFtt->sm_coeff/a ;
       else
          Tg = lpFtt->sm_coeff/(lpFtt->b3khz*F_PI);
       D= 1/(lpFtt->atf * Tg);
    }
    for (i=0;i<nLen;i++)
    {
      rr=y*cos(fx)*lpFtt->real_d[k]-y*sin(fx)*lpFtt->imag_d[k]+a*(sigTemp)[i]/lpFtt->atf;
     ri=y*cos(fx)*lpFtt->imag_d[k]+y*sin(fx)*lpFtt->real_d[k];
       lpFtt->real_d[k]=rr;
       lpFtt->imag_d[k]=ri;
       x = rr*rr + ri*ri;
       if(lpFtt->sm_coeff!=0)
           x = D*x + exp(-D)*lpFtt->smooth_f[k];
       lpFtt->smooth_f[k]=x;
    }
    x = x /(lpFtt->norm_coeff * lpFtt->norm_coeff);

  if(dlp_charin('M', lpFtt->type))
    ((FLOAT64*)frame)[k] = MAX(nMinLog,0.5L * log(x));
  else
  {  if(x>lpFtt->max_value) x = lpFtt->max_value;        /* limit values */
     if((INT32)lpFtt->log)                     /* calculate log? */
     { if(x<lpFtt->log_min) x = lpFtt->log_min;
       x = lpFtt->log_scale * log10(x/lpFtt->log_min);
     }
     ((FLOAT64*)frame)[k] = x;
  }
  }
  if(sigTemp) dlp_free(sigTemp);
  return O_K;
}

#define DLM_FTT_WTYPE_MEL         1
#define DLM_FTT_WTYPE_MELC        2


/**
 * Initialize frequency/bandwidth pairs.
 *
 * @param lpFtt
 *          Pointer to parameter struct
 * @param nOut
 *          Number of output channels
  * @return <code>O_K</code> if successful, a (negative) error code otherwise
 */
INT16 CGEN_IGNORE dlm_ftt_init(MLP_FTT_TYPE* lpFtt, INT16 nOut, INT16 noreset) {
  INT16 k;
  INT16 nErr= O_K;
  INT16 wtype;
  FLOAT32 freq;

  DLPASSERT(lpFtt);
  if (!lpFtt)
    return NOT_EXEC;

  /* Free present data */
  dlm_ftt_done(lpFtt,noreset);

  /* Basic initialization and memory allocation */
  lpFtt->midfreq     = (FLOAT64*) dlp_calloc(nOut,sizeof(FLOAT64));
  lpFtt->bandwidth   = (FLOAT64*) dlp_calloc(nOut,sizeof(FLOAT64));
  if (!noreset){
   lpFtt->smooth_f    = (FLOAT64*) dlp_calloc(nOut,sizeof(FLOAT64));
   lpFtt->real_d      = (FLOAT64*) dlp_calloc(nOut, sizeof(FLOAT64));
   lpFtt->imag_d      = (FLOAT64*) dlp_calloc(nOut, sizeof(FLOAT64));
  }


  /* Check memory */
  if (!lpFtt->midfreq || !lpFtt->bandwidth || !lpFtt->real_d || !lpFtt->imag_d || !lpFtt->smooth_f
      || nErr==ERR_MEM) {
    dlm_ftt_done(lpFtt,FALSE);
    DLPASSERT(0);
  }

  if(dlp_charin('M', lpFtt->type))
  {
    wtype = DLM_FTT_WTYPE_MEL;
  }
  else
    if (dlp_charin('C', lpFtt->type))
    wtype = DLM_FTT_WTYPE_MELC;
      else
      {
        dlm_ftt_done(lpFtt,FALSE);
        return NOT_EXEC;
      }

  /* Generate frequency/bandwidth pairs */
  switch(wtype) {
   case DLM_FTT_WTYPE_MELC:
  lpFtt->log_min = pow(10.,(FLOAT64)(-lpFtt->log/10.))*lpFtt->max_value;         /* Ampl.- Quadrat! */
  lpFtt->log_scale = lpFtt->max_value/log10(lpFtt->max_value/lpFtt->log_min);
  /* no break */

   case DLM_FTT_WTYPE_MEL:
  freq = lpFtt->start_freq/1000.0f;      /* freq in kHz */
    for (k=0; k<nOut; k++)
    {  lpFtt->midfreq[k] = freq*1000.f;   /* midfreq in Hz */
      lpFtt->bandwidth[k] = lpFtt->bw_coeff*(25.0f+75.0f*(FLOAT32)pow((1.0f+1.4f*freq*freq),.69f));
      freq=freq + lpFtt->finc_coeff*(FLOAT32)(25.0f+75.0f*pow(1+1.4*freq*freq,0.69f))/1000.0f;
    }
  lpFtt->b3khz = lpFtt->bw_coeff*(25.0f+75.0f*(FLOAT32)pow((1.0f+1.4f*3.0f*3.0f),.69f));
    break;

   default:
    dlm_ftt_done(lpFtt,FALSE);
    return NOT_EXEC;
  }
  return O_K;
}

/**
 * Deallocate core data structure.
 *
 * @param lpFtt
 *          Pointer to parameter structure
 */
INT16 CGEN_IGNORE dlm_ftt_done(MLP_FTT_TYPE* lpFtt,INT16 noreset) {

  DLPASSERT(lpFtt);
  if (!lpFtt)
    return NOT_EXEC;

  if (lpFtt->midfreq)   dlp_free(lpFtt->midfreq );
  if (lpFtt->bandwidth) dlp_free(lpFtt->bandwidth);
  if (!noreset){
    if (lpFtt->smooth_f)  dlp_free(lpFtt->smooth_f);
    if (lpFtt->real_d)    dlp_free(lpFtt->real_d );
    if (lpFtt->imag_d)    dlp_free(lpFtt->imag_d );
  }
  return O_K;
}
