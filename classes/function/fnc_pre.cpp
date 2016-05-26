// dLabPro class CFunction (function)
// - Token preprocessor
//
// AUTHOR : Matthias Wolff
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

#include "dlp_function.h"
#include "dlp_dgen.h"

/**
 * Preprocess a dLabPro token. The method will
 * - replace $$ with the number of command line arguments (nArgc),
 * - replace $n[n] with the respective command line argument (lpArgv[nn]),
 * - replace $xxx with the respective environment variable,
 * - replace ${xxx} with the string the formula xxx evaluates to, and
 * - replace "\$" with "$"
 *
 * "$..." constructions will be replaced inside AND outside strings. If "$..."
 * does not evaluate to anything, it will be left untouched.
 *
 * This static method may be used independently of CFunction by any program that
 * needs to realize a string replacement using formulas, command line arguments
 * and environment variables.
 *
 * @param lpsTok      The command line to preprocess (content will be modified)
 * @param nLen        The maximal number of characters (incl. the terminal 0)
 *                    lpsTok can hold
 * @param lpsTt       The token type
 * @param iArgs       Table of arguments (may be NULL)
 * @param lpsInfile   The name of the source file (may be NULL)
 * @param nInline     The line number of lpCmd in the source file (may be <0)
 * @return <code>TRUE</code> if the token has been modified, <code>FALSE</code>
 *         otherwise.
 */
BOOL CGEN_PUBLIC CFunction::PreprocessToken
(
  char*       lpsTok,
  INT16       nLen,
  const char* lpsTt,
  CData*      iArgs,
  const char* lpsInfile,
  INT32       nInline
)
{
  char* p                     = NULL;
  char* tx                    = NULL;
  INT32 i                     = 0;
  INT16 nReplaced             = 0;
  char  lpBuf [L_INPUTLINE+1];
  char  lpBuf2[L_INPUTLINE+1];
  char  lpBuf3[L_INPUTLINE+1];

  if (!lpsTok) return FALSE;
  if (dlp_strcmp(lpsTt,TT_BCMT)==0) return FALSE;
  if (dlp_strcmp(lpsTt,TT_DCMT)==0) return FALSE;
  if (dlp_strcmp(lpsTt,TT_LCMT)==0) return FALSE;
  if (dlp_strcmp(lpsTt,TT_LAB )==0) return FALSE;
  if (dlp_strcmp(lpsTt,TT_ELIN)==0) return FALSE;
  
  // Prescan (no $'s -> nothing to be done)
  for (p=lpsTok;*p;p++)
  {
    if (*p=='$') break;
  }
  if (!*p) return FALSE;

  // Move away quoted dollars "\$"
  // FIXME: replace back is unsafe!
  dlp_strreplace(lpsTok,"\\$","#D#");

  // Replace command line arguments
  if (iArgs)
  {
    sprintf(lpBuf,"%ld",(long)iArgs->GetNRecs());
    nReplaced += dlp_strreplace(lpsTok,"$$",lpBuf);
    for (i=iArgs->GetNRecs(); i>0; i--)
    {
      if (iArgs->Dfetch(i-1,FNC_ALIC_TYPE)!=T_STRING) continue;
      const char* lpsVal = *(const char**)iArgs->Pfetch(i-1,FNC_ALIC_PTR);
      sprintf(lpBuf,"$%ld",(long)i);
      nReplaced += dlp_strreplace(lpsTok,lpBuf,lpsVal);
    }
  }
  else nReplaced += dlp_strreplace(lpsTok,"$$","0");

  // Scan for "$..."
  for (p=lpsTok;*p;p++)
  {
    if (*p!='$') continue;

    // ${xxx}
    if (p[1]=='{')
    {
      for (i=2;p[i]&&p[i]!='}';i++) {};
      if (p[i]=='}')
      {
        dlp_strncpy(lpBuf ,p    ,i+1); lpBuf [i+1]='\0'; // ${xxx} (to replace)
        dlp_strncpy(lpBuf2,&p[2],i-1); lpBuf2[i-2]='\0'; // xxx (to evaluate)
        if (getenv(lpBuf2)) // xxx is an environment variable
        {
          nReplaced += dlp_strreplace_ex(p,lpBuf,getenv(lpBuf2),TRUE);
        }
        else
        {
          if (!m_lpsLastFml) m_lpsLastFml = (char*)dlp_malloc(L_INPUTLINE);
          IF_NOK(Formula2RPN(lpBuf2,m_lpsLastFml,L_INPUTLINE-1))
            nReplaced += dlp_strreplace_ex(p,lpBuf,"",TRUE);
          else
          {
            IF_NOK(SendCommand(m_lpsLastFml,lpBuf3,L_INPUTLINE,m_nPp))
              nReplaced += dlp_strreplace_ex(p,lpBuf,"",TRUE);
            else
              nReplaced += dlp_strreplace_ex(p,lpBuf,lpBuf3,TRUE);
          }
        }
      }
    }
    // $[xxx]
    else if (p[1]=='[')
    {
      for (i=2;p[i]&&p[i]!=']';i++) {};
      if (p[i]==']')
      {
        dlp_strncpy(lpBuf ,p    ,i+1); lpBuf [i+1]='\0'; // $[xxx] (to replace)
        dlp_strncpy(lpBuf2,&p[2],i-1); lpBuf2[i-2]='\0'; // xxx (to evaluate)
        IF_OK(SendCommand(lpBuf2,lpBuf3,L_INPUTLINE+1,nInline))
          nReplaced += dlp_strreplace_ex(p,lpBuf,lpBuf3,TRUE);
      }
    }
    // $xxx
    else
    {
      for (i=1;!isspace(p[i])&&(isalnum(p[i])||p[i]=='_');i++) {}
      dlp_strncpy(lpBuf,p,i);
      lpBuf[i]='\0';

      // pseudo-environment variables $__FILE__,$__SFILE__
      if (strcmp(lpBuf,"$__FILE__")==0 || strcmp(lpBuf,"$__SFILE__")==0)
      {
        strcpy(lpBuf2,lpsInfile?lpsInfile:"");
        dlp_strreplace(lpBuf2,"\\","/");
        if (strcmp(lpBuf,"$__SFILE__")==0)
        {
          dlp_splitpath(lpBuf2,NULL,lpBuf);
          tx = &lpBuf[dlp_strlen(lpBuf)-1];
          while (tx>lpBuf && *tx!='.') tx--;
          if (*tx=='.') *tx='\0';
          nReplaced += dlp_strreplace_ex(p,"$__SFILE__",lpBuf,TRUE);
        }
        else
          nReplaced += dlp_strreplace_ex(p,"$__FILE__",lpBuf2,TRUE);
      }
      // pseudo-environment variables $__MAIN__,$__SMAIN__
      if (strcmp(lpBuf,"$__MAIN__")==0 || strcmp(lpBuf,"$__SMAIN__")==0)
      {
      	char* lpsMain = GetRootFnc()->GetSrcFile(0,NULL);
        strcpy(lpBuf2,lpsMain?lpsMain:"");
        dlp_strreplace(lpBuf2,"\\","/");
        if (strcmp(lpBuf,"$__SMAIN__")==0)
        {
          dlp_splitpath(lpBuf2,NULL,lpBuf);
          tx = &lpBuf[dlp_strlen(lpBuf)-1];
          while (tx>lpBuf && *tx!='.') tx--;
          if (*tx=='.') *tx='\0';
          nReplaced += dlp_strreplace_ex(p,"$__SMAIN__",lpBuf,TRUE);
        }
        else
          nReplaced += dlp_strreplace_ex(p,"$__MAIN__",lpBuf2,TRUE);
      }
      // pseudo-environment variable $__LINE__
      else if (strcmp(lpBuf,"$__LINE__")==0)
      {
        if (lpsInfile) sprintf(lpBuf,"%ld",(long)nInline); else lpBuf[0]='\0';
        nReplaced += dlp_strreplace_ex(p,"$__LINE__",lpBuf,TRUE);
      }
      // test for environment variable
      else
        nReplaced += dlp_strreplace_env(p,TRUE);
    }
  }

  // Restore escaped dollars
  dlp_strreplace(lpsTok,"#D#","\\$");

  return nReplaced>0; // No need to re-tokenize when unchanged
}

// EOF
