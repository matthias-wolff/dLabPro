// dLabPro class CHistogram (histogram)
// - Histogram display methods (partly defunct)
//
// AUTHOR : Christian-M. Westendorf, Matthias Wolff
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

#include "dlp_histogram.h"

/**
 * Print histogram parameters.
 *
 * @param ih Histogram index
 */
INT16 CGEN_PUBLIC CHistogram::PrintPar(INT32 ih)
{
  INT32    i,k;
  FLOAT64  sum;
  FLOAT64  min = 0.;
  FLOAT64  max = 0.;
  FLOAT64* mean;
  FLOAT64* std;

  if (CheckHisto()   != O_K)  return(NOT_EXEC); 
  if (CheckConsist() != O_K)  return(NOT_EXEC); 

  mean = (FLOAT64*)dl_calloc (2*m_hdim, sizeof(FLOAT64),"CVh::PrintPar");
  std  = &mean[m_hdim];

  if (m_hmode==1||m_hmode==2) 
    MeanVarComp((FLOAT64*)xaddr(m_hist,ih*m_bins,0),(FLOAT64*)data_ptr(m_minmax),mean,std);
  if (m_hmode==3)
    MeanVarComp((FLOAT64*)xaddr(m_hist,ih*m_bins,0),(FLOAT64*)xaddr(m_minmax,2*ih,0),mean,std);

  printf ("\n histogram %d", ih);
  if (data_empty(m_labtab) != TRUE)
  {
    printf (" label  = %s",(char*)xaddr(m_labtab,ih,0) );
  }

  printf ("\n\n component        min      max       sum      mean    stddev");
  printf ("\n --------------------------------------------------------------\n");

  for (i=0; i<m_hdim; i++)
  {
    printf("\n %3d : %-6s", i, comp_text(m_hist,i));

    sum = 0.;
    for (k=0; k<m_bins; k++) sum += dfetch(m_hist,ih*m_bins+k,i);

    switch(m_hmode)
    {
    case 1:
      min = dfetch(m_minmax,0,0);
      max = dfetch(m_minmax,1,0);
      break;
    case 2:
      min = dfetch(m_minmax,0,i);
      max = dfetch(m_minmax,1,i);
      break;
    case 3: 
      min = dfetch(m_minmax,2*ih,i);
      max = dfetch(m_minmax,2*ih+1,i);
      break;
    }
    printf ("%9g %9g %9g %9g %9g",min,max,sum,mean[i],sqrt(std[i]));
  }

  printf ("\n");
  printf ("\n ------------------------------------------------------------- \n");

  dl_free (mean);
  return O_K;
}

/**
 * Copy histogram to data instance for displaying.
 *
 * @param tab  label table (may be NULL)
 * @param buff data buffer
 * @param c    dnv control record
 */
INT16 CGEN_PUBLIC CHistogram::ShowEx
(
  INT32       ir,
  INT32       ic,
  const char* name,
  CData*      idOut
)
{
  char  txt[40];

  if(!idOut               ) return NOT_EXEC;
  if(CheckHisto()   != O_K) return NOT_EXEC;
  if(CheckConsist() != O_K) return NOT_EXEC;

  strcpy(txt, "histogram:  ");
  strcat(txt, name);
  if (SelectBlock(idOut,ir,ic)!=O_K) return NOT_EXEC;
  if (PrintPar(ir)            !=O_K) return NOT_EXEC;

  return O_K;
}

// EOF
