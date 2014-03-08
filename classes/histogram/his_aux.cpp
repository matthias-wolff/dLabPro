// dLabPro class CHistogram (histogram)
// - Auxiliary methods
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
 * Print histogram status information at stdout.
 */
INT16 CGEN_PUBLIC CHistogram::PrintStatus()
{
  printf("\n  ");
  dlp_fprint_x_line(stdout,'-',60);
  printf("\n histogram %s \n",m_lpInstanceName);
  dlp_fprint_x_line(stdout,'-',60);
  printf("\n   histogram bins         %6d ", m_bins);
  printf("\n   histogram number       %6d ", m_nhist);
  printf("\n   histogram components   %6d ", m_hdim);

  printf("\n   histogram mode         %6d ", m_hmode);
  if (m_hmode == 1)
  {
    printf(" min/max = const. :");
    printf("\n     min(lower border)    %6g", m_min);
    printf("\n     max(upper border)    %6g", m_max);
  }
  if (m_hmode == 2)
    printf(" min/max = f(component)");
  if (m_hmode == 3)
    printf(" min/max = f(index/component)");
  if (m_hmode != 1)
    printf("\n    (use -print_minmax to see the min/max values)");

  if (m_wflag == 1)
    printf("\n     windowed update");

  if (m_hist != NULL)
    printf("\n   histogram                   %s", m_hist->m_lpInstanceName);

  if (m_minmax != NULL)
    printf("\n   min-max data                %s", m_minmax->m_lpInstanceName);

  printf("\n  ");
  if (m_cname != NULL)
    if (strlen(m_cname) > 0)
      printf("\n   index comp. name       %s", m_cname);
  printf("\n   index component        %6d", m_icomp);
  printf("\n   update calls           %6d", m_calls);
  printf("\n   last used records      %6d", m_count);
  printf("\n   overall sample size    %6d", m_ssize);
  printf("\n");
  dlp_fprint_x_line(stdout,'-',60);
  printf("\n");
  return (O_K);
}

/**
 * The aux. routine corrects the PHONDAT label by removing spaces at the end of
 * the Label.
 */
static INT16 CHistogram_CorrectLabel(CData* x)
{
  INT32 i, j, k, nc, T;
  char *p;

  if (CData_IsEmpty(x) == TRUE)
    return (NOT_EXEC);

  T = CData_GetNRecs(x);
  for (i = 0; i < CData_GetNComps(x); i++)
  {
    nc = CData_GetCompType(x,i);
    if (nc < 256)
      for (j = 0; j < T; j++)
      {
        p = (char*)CData_XAddr(x,j,i);
        k = nc - 1;
        while (p[k] == ' ' && k >= 0)
        {
          p[k] = 0;
          k--;
        }
      }
  }

  return (O_K);
}
       
/**
 * Update histogram description.
 */
void CGEN_PROTECTED CHistogram::UpdateHdesc()
{
  m_nhist = CData_GetNBlocks(m_hist);
  m_hmode = (INT16)CData_GetDescr(m_hist,DESCR1);
  m_hdim  = CData_GetNComps(m_hist);
  m_calls++;
  m_ssize += m_count;
}

/**
 * Find index component.
 */
INT32 CGEN_PROTECTED CHistogram::FindIndexcomp(CData* x)
{
  if (m_cname == NULL)
    return (0);
  if (strlen(m_cname) == 0)
    return (0);
  m_icomp = CData_FindComp(x, m_cname);
  return (m_icomp);
}

/**
 * Generate index list.
 */
INT32 CGEN_PROTECTED CHistogram::GenIndexlist(CData* x, CData* tab)
{
  INT32 i;
  INT32 T = CData_GetNRecs(x);

  if (T == 0)
    return (NOT_EXEC);

  if (m_icomp > -1)
  {
    CData_Reset(BASEINST(m_indexlist),1);
    CData_AddComp(m_indexlist,"indx",T_LONG);
    CData_Alloc(m_indexlist, T);
    for (i = 0; i < T; i++)
      CData_Dstore(m_indexlist,CData_Dfetch(x,i,m_icomp),i,0);
    return (O_K);
  }

  if (CData_IsEmpty(tab) == TRUE)
  {
    CData_Reset(BASEINST(m_indexlist),1);
    CData_Reset(BASEINST(m_labtab   ),1);
    return (O_K);
  }

  if (CData_GetCompType(tab,0) <= 256)
  {
    CData_SelectComps(m_labtab,tab,0,1);
    CHistogram_CorrectLabel(x);
    CData_GenLabIndex(x, NULL, m_indexlist, m_labtab );
    return (O_K);
  }

  CData_SelectComps(m_indexlist,tab,0,1);
  if (CData_GetNRecs(m_indexlist) < T)
    return IERROR(this,HIS_SHORTLIST,T,0,0);

  return (O_K);
}

/**
 * Setup minmax object
 */
INT16 CGEN_PROTECTED CHistogram::SetDefMinmax()
{
  if (m_minm == NULL)
  {
    m_minm = CData_CreateInstance("minm");
    CData_AddComp(m_minm, "    ", T_DOUBLE);
    CData_Allocate(m_minm, 2);
    if (m_minmax)
      IDESTROY(m_minmax);
    m_minmax = m_minm;
  }

  if (m_minmax == m_minm)
  {
    if (m_min < m_max)
    {
      CData_Dstore(m_minm, m_min, 0, 0);
      CData_Dstore(m_minm, m_max, 1, 0);
      m_hmode = 1;
    } else
    {
      CData_Dstore(m_minm, 0., 0, 0);
      CData_Dstore(m_minm, 1., 1, 0);
      m_hmode = 1;
      m_min = 0.;
      m_max = 1.;
      return IERROR(this,HIS_MINMAXERR,m_min,m_max,0);
    }
  }
  return (O_K);
}

// EOF
