// dLabPro SDK class CCgen (cgen)
// - Generating manual (*.man and *.html) files
//
// AUTHOR : Matthias Wolff
// PACKAGE: dLabPro/sdk
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

#include "dlp_cgen.h"

#define UNQUOTATE(A) if (A[0] == '"') memmove(&A[0],&A[1],strlen(A)); \
                     if (A[strlen(A)-1] == '"') A[strlen(A)-1] = '\0';

INT16 CGEN_PROTECTED CCgen::CreateManualFile()
{
  // Not needed for a sigmaLab project
  if (m_bMainProject) return O_K;

  if (((CItp*)GetItp())->m_nVerbose>1) printf("\nCreating manual files...");

  SCGStr* lpTmpl;
  char buf[255],text[255];
  char lpManfn[255]; dlp_convert_name(CN_MANFILE,lpManfn,m_lpProject);

  ReplaceAllKeys(m_manFileTmpl);

  // Write class' manual
  lpTmpl = m_manFileTmpl.m_items;
  while (lpTmpl)
  {
    if (strstr(lpTmpl->lpName,"CLASS DESCRIPTION:") !=NULL)
    {
      SCGStr* plm = m_manual.m_items;
      while (plm)
      {
        m_manFileTmpl.InsertItem(lpTmpl,plm->lpName); 
        lpTmpl = lpTmpl->next;
        plm   = plm->next;
      }
      m_manFileTmpl.InsertItem(lpTmpl,""); lpTmpl = lpTmpl->next;
    }
    lpTmpl = lpTmpl->next;
  }

  // Write fields' manual
  SCGFld* lpFld;
  lpTmpl = m_manFileTmpl.m_items;
  while (lpTmpl)
  {
    if (strstr(lpTmpl->lpName,"##{{CGEN_FIELDS") !=NULL)
    {
      lpFld = m_flds.m_items;
      while (lpFld)
      {
        if (!(lpFld->nFlags & FF_HIDDEN)) // Not for hidden fields
        {
          m_manFileTmpl.InsertItem(lpTmpl,""); lpTmpl = lpTmpl->next;
          sprintf(buf,"FIELD:     %s",lpFld->lpName); m_manFileTmpl.InsertItem(lpTmpl,buf); lpTmpl = lpTmpl->next;
          m_manFileTmpl.InsertItem(lpTmpl,""); lpTmpl = lpTmpl->next;
          strcpy(text,lpFld->lpComment); UNQUOTATE(text);
          sprintf(buf,"COMMENT:   %s",text); m_manFileTmpl.InsertItem(lpTmpl,buf); lpTmpl = lpTmpl->next;
          m_manFileTmpl.InsertItem(lpTmpl,""); lpTmpl = lpTmpl->next;
          if (lpFld->nType == T_INSTANCE) sprintf(buf,"TYPE   :   INSTANCE(%s)",lpFld->lpType); 
          else                           sprintf(buf,"TYPE   :   %s",lpFld->lpCType); 
          m_manFileTmpl.InsertItem(lpTmpl,buf); lpTmpl = lpTmpl->next;
          if (dlp_strlen(lpFld->lpInit) > 0) sprintf(buf,"DEFAULT:   %s",lpFld->lpInit); 
          else
          {
            INT16 nType = dlp_get_type_code(lpFld->lpType);
            if      (dlp_is_numeric_type_code(nType))  sprintf(buf,"DEFAULT:   0"); 
            else if (dlp_is_symbolic_type_code(nType)) sprintf(buf,"DEFAULT:   \"\""); 
            else                                       sprintf(buf,"DEFAULT:   NULL");
          }
          m_manFileTmpl.InsertItem(lpTmpl,buf); lpTmpl = lpTmpl->next;
          m_manFileTmpl.InsertItem(lpTmpl,""); lpTmpl = lpTmpl->next;
          if (lpFld->lMan.m_items)
          {
            m_manFileTmpl.InsertItem(lpTmpl,"EXPLANATION:"); lpTmpl = lpTmpl->next;
            SCGStr *plm = lpFld->lMan.m_items;
            while (plm)
            {
              m_manFileTmpl.InsertItem(lpTmpl,plm->lpName); 
              lpTmpl = lpTmpl->next;
              plm   = plm->next;
            }
            m_manFileTmpl.InsertItem(lpTmpl,""); lpTmpl = lpTmpl->next;
          }
          m_manFileTmpl.InsertItem(lpTmpl,"#------------------------------------------------------------------"); 
          lpTmpl = lpTmpl->next;
        }
        lpFld = lpFld->next;
      }
    }
    lpTmpl = lpTmpl->next;
  }

  // Write options' manual
  SCGOpt* lpOpt;
  lpTmpl = m_manFileTmpl.m_items;
  while (lpTmpl)
  {
    if (strstr(lpTmpl->lpName,"##{{CGEN_OPTIONS") !=NULL)
    {
      lpOpt = m_opts.m_items;
      while (lpOpt)
      {
        m_manFileTmpl.InsertItem(lpTmpl,""); lpTmpl = lpTmpl->next;
        sprintf(buf,"OPTION: %s",lpOpt->lpName); m_manFileTmpl.InsertItem(lpTmpl,buf); lpTmpl = lpTmpl->next;
        m_manFileTmpl.InsertItem(lpTmpl,""); lpTmpl = lpTmpl->next;
        strcpy(text,lpOpt->lpComment); UNQUOTATE(text);
        sprintf(buf,"COMMENT:   %s",text); m_manFileTmpl.InsertItem(lpTmpl,buf); lpTmpl = lpTmpl->next;
        m_manFileTmpl.InsertItem(lpTmpl,""); lpTmpl = lpTmpl->next;
        if (lpOpt->lMan.m_items)
        {
          m_manFileTmpl.InsertItem(lpTmpl,"EXPLANATION:"); lpTmpl = lpTmpl->next;
          SCGStr *plm = lpOpt->lMan.m_items;
          while (plm)
          {
            m_manFileTmpl.InsertItem(lpTmpl,plm->lpName); 
            lpTmpl = lpTmpl->next;
            plm   = plm->next;
          }
          m_manFileTmpl.InsertItem(lpTmpl,""); lpTmpl = lpTmpl->next;
        }
        m_manFileTmpl.InsertItem(lpTmpl,"#------------------------------------------------------------------"); 
        lpTmpl = lpTmpl->next;

        lpOpt = lpOpt->next;
      }
    }
    lpTmpl = lpTmpl->next;
  }

  // Write methods' manual
  SCGMth* lpMth;
  lpTmpl = m_manFileTmpl.m_items;
  while (lpTmpl)
  {
    if (strstr(lpTmpl->lpName,"##{{CGEN_METHODS") !=NULL)
    {
      lpMth = m_mths.m_items;
      while (lpMth)
      {
        m_manFileTmpl.InsertItem(lpTmpl,""); lpTmpl = lpTmpl->next;
        sprintf(buf,"METHOD:    %s",lpMth->lpName); m_manFileTmpl.InsertItem(lpTmpl,buf); lpTmpl = lpTmpl->next;
        m_manFileTmpl.InsertItem(lpTmpl,""); lpTmpl = lpTmpl->next;
        strcpy(text,lpMth->lpComment); UNQUOTATE(text);
        sprintf(buf,"COMMENT:   %s",text); m_manFileTmpl.InsertItem(lpTmpl,buf); lpTmpl = lpTmpl->next;
        m_manFileTmpl.InsertItem(lpTmpl,""); lpTmpl = lpTmpl->next;
        strcpy(text,lpMth->lpUPNSyntax); UNQUOTATE(text);
        sprintf(buf,"SYNTAX:    %s %s %s",text,lpMth->lpName,lpMth->lpPostsyn); 
        m_manFileTmpl.InsertItem(lpTmpl,buf); lpTmpl = lpTmpl->next;
        m_manFileTmpl.InsertItem(lpTmpl,""); lpTmpl = lpTmpl->next;
        if (lpMth->lMan.m_items)
        {
          m_manFileTmpl.InsertItem(lpTmpl,"EXPLANATION:"); lpTmpl = lpTmpl->next;
          SCGStr *plm = lpMth->lMan.m_items;
          while (plm)
          {
            m_manFileTmpl.InsertItem(lpTmpl,plm->lpName); 
            lpTmpl = lpTmpl->next;
            plm   = plm->next;
          }
          m_manFileTmpl.InsertItem(lpTmpl,""); lpTmpl = lpTmpl->next;
        }
        m_manFileTmpl.InsertItem(lpTmpl,"#------------------------------------------------------------------"); 
        lpTmpl = lpTmpl->next;

        lpMth = lpMth->next;
      }
    }
    lpTmpl = lpTmpl->next;
  }

  WriteBackTemplate(m_manFileTmpl,m_lpMPath,lpManfn);
  PostTerminalMsg(SLM_SLCVTSGNL,SLCVT_RELOADMAN,0);

  return O_K;
}

// EOF
