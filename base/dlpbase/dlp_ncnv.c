/* dLabPro base library
 * - Implementation of the dLabPro naming conventions
 *
 * AUTHOR : Matthias Wolff
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

#include "dlp_kernel.h"
#include "dlp_base.h"

/**
 * Converts one identifier into another accoring to the dLabPro
 * naming conventions.
 *
 * @param how     The type of identifier <string2> shall be 
 *                converted to, one of the CN_XXX constants
 * @param string1 The destination string
 * @param string2 The source string
 * @return        The destination string, no error report
 */
char* dlp_convert_name(INT16 how, char* string1, const char* string2)
{
  char  lpSBuf[255];
  char* tx;

  /* Validation */
  if (!string1) return NULL;

  /* Save source string */
  dlp_strcpy(lpSBuf,string2);

  /* Convert string */
  switch(how)
  {

  case CN_QUOTE       : { /* <lpSrc>: Some text */
    if (lpSBuf[0]!='\"') {memmove(&lpSBuf[1],lpSBuf,strlen(lpSBuf)+1); lpSBuf[0]='\"';}
    tx = &lpSBuf[strlen(lpSBuf)-1];
    if (*tx++!='\"') {*tx++='\"'; *tx='\0';}
    memmove(string1,lpSBuf,strlen(lpSBuf)+1);
    return string1; }

  case CN_UNQUOTE       : { /* <lpSrc>: Some text */
    if (lpSBuf[0]=='\"') memmove(lpSBuf,&lpSBuf[1],strlen(lpSBuf));
    tx = &lpSBuf[strlen(lpSBuf)-1];
    if (*tx=='\"') *tx=0;
    memmove(string1,lpSBuf,strlen(lpSBuf)+1);
    return string1; }

  case CN_XLATPATH      : { /* <lpSrc>: Path name */
    tx = lpSBuf;
    while (*tx++) if (*tx=='/' || *tx=='\\') *tx = C_DIR;
    memmove(string1,lpSBuf,strlen(lpSBuf)+1);
    return string1; }

  case CN_HFILE       : { /* <lpSrc>: Project name */
    sprintf(string1,"dlp_%s.h",dlp_strlwr(lpSBuf));
    return string1; }

  case CN_CFILE       : { /* <lpSrc>: Project name */
    sprintf(string1,"%s.c",dlp_strlwr(lpSBuf));
    return string1; }

  case CN_CPPFILE     : { /* <lpSrc>: Project name */
    sprintf(string1,"%s.cpp",dlp_strlwr(lpSBuf));
    return string1; }

  case CN_BSIFILE     : { /* <lpSrc>: Project name */
    sprintf(string1,"%s.bsi",dlp_strlwr(lpSBuf));
    return string1; }

  case CN_MANFILE     : { /* <lpSrc>: Project name */
    sprintf(string1,"%s.html",dlp_strlwr(lpSBuf));
    return string1; }

  case CN_MAKEFILE    : { /* <lpSrc>: Project name */
      /* sprintf(string1,"make%s",dlp_strlwr(lpSBuf)); */
      sprintf(string1,"makefile");
    return string1; }

  case CN_NMAKFILE    : { /* <lpSrc>: Project name */
    sprintf(string1,"%s.mak",dlp_strlwr(lpSBuf));
    return string1; }

  case CN_DSWFILE    : { /* <lpSrc>: Project name */
    sprintf(string1,"%s.dsw",dlp_strlwr(lpSBuf));
    return string1; }

  case CN_DSPFILE    : { /* <lpSrc>: Project name */
    sprintf(string1,"%s.dsp",dlp_strlwr(lpSBuf));
    return string1; }

  case CN_GENFILE   : { /* <lpSrc>: Project name */
    sprintf(string1,"gen%s.bat",dlp_strlwr(lpSBuf));
    return string1; }

  case CN_NCBFILE   : { /* <lpSrc>: Project name */
  sprintf(string1,"%s.ncb",dlp_strlwr(lpSBuf));
    return string1; }

  case CN_SIDFILE     : { /* <lpSrc>: Project name */
    sprintf(string1,"%s.js",dlp_strlwr(lpSBuf));
    return string1; }

  case CN_DLP2CXX_CCF :
  case CN_DLP2CXX_PUCF:
  case CN_DLP2CXX_OUCF:
  case CN_DLP2CXX_BPAR:
  case CN_DLP2CXX_NPAR:
  case CN_DLP2CXX_IPAR:
  case CN_DLP2CXX_DPAR:
  case CN_DLP2CXX_SPAR:
  case CN_DLP2CXX_LPAR:
  case CN_DLP2CXX_PAR : { /* <lpSrc>: word name */
    char* tx = lpSBuf;
    while(*tx)
    {
      if (*tx=='/' || *tx=='-' || *tx=='_') 
      {
        memmove(tx,&tx[1],strlen(tx));
        CHARUPR(*tx);
      }
      tx++;
    }
    if (how != CN_DLP2CXX_PAR) CHARUPR(*lpSBuf);
    switch(how)
    {
    case CN_DLP2CXX_CCF : {sprintf(string1,"%s"         ,lpSBuf); return string1;}
    case CN_DLP2CXX_OUCF: {sprintf(string1,"On%sSet"    ,lpSBuf); return string1;}
    case CN_DLP2CXX_PUCF: {sprintf(string1,"On%sChanged",lpSBuf); return string1;}
    case CN_DLP2CXX_BPAR: {sprintf(string1,"m_b%s"      ,lpSBuf); return string1;}
    case CN_DLP2CXX_NPAR: {sprintf(string1,"m_n%s"      ,lpSBuf); return string1;}
    case CN_DLP2CXX_LPAR: {sprintf(string1,"m_lp%s"     ,lpSBuf); return string1;}
    case CN_DLP2CXX_SPAR: {sprintf(string1,"m_lps%s"    ,lpSBuf); return string1;}
    case CN_DLP2CXX_IPAR: {sprintf(string1,"m_i%s"      ,lpSBuf); return string1;}
    case CN_DLP2CXX_DPAR: {sprintf(string1,"m_id%s"     ,lpSBuf); return string1;}
    case CN_DLP2CXX_TPAR: {sprintf(string1,"m_it%s"     ,lpSBuf); return string1;}
    case CN_DLP2CXX_PAR : {sprintf(string1,"m_%s"       ,lpSBuf); return string1;}
    }
    return NULL;}

  case CN_DLP2CXX_CLSN: { /* <lpSrc>: Project name */
    CHARUPR(lpSBuf[0]);
    sprintf(string1,"C%s",lpSBuf);
    return string1; }

  case CN_CSTRUCTNAME : { /* <lpSrc>: Project name */
    sprintf(string1,"%s_par",dlp_strlwr(lpSBuf));
    return string1; }

  case CN_AUTONAME    : { /* <lpSrc>: dLabPro class name */
    sprintf(string1,"std%s",lpSBuf);
    return string1; }

  default: return NULL;
  }
}

/**
 * Checks if a string matches the dLabPro naming conventions for
 * identifiers.
 *
 * @param how    Type of identifier, one of the IDT_XXX constants
 * @param string The indentifier to check
 * @param faulty Buffer for the part of <string> which violates 
 *               the naming conventions, may be NULL
 * @return       TRUE if the naming conventions are met, FALSE
 *               otherwise
 */
INT16 dlp_is_valid_id(INT16 nHow, const char* lpsStr, char* lpsFaulty)
{
  INT16       bFirstBad = FALSE;
  const char* tx;

  /* Validation */
  if (!lpsStr) return IVIR_BADSTRING;

  tx = lpsStr;
  while (*tx)
  {
    if (*tx < 'a' || *tx > 'z')
      if (*tx < 'A' || *tx > 'Z')
        if (*tx < '0' || *tx > '9')
          if (*tx != '_')
          {
            if (lpsFaulty) *lpsFaulty = *tx;
            if (tx != lpsStr) return IVIR_BADCHAR;
            else              bFirstBad = TRUE;
          }
    tx++;
  }

  switch(nHow)
  {
  case IDT_OPTION :
    if (*lpsStr == '/') return O_K; 
    else if (!bFirstBad) {if (lpsFaulty) *lpsFaulty = '/'; return IVIR_BADLEAD;}
    else return IVIR_BADCHAR;
    break;
  case IDT_METHOD: 
    if (*lpsStr == '-' || *lpsStr == '?') return O_K; 
    else if (!bFirstBad) {if (lpsFaulty) *lpsFaulty = '-'; return IVIR_BADLEAD;}
    else if (*lpsStr == '/') return IVIR_BADLEAD;
    else return IVIR_BADCHAR;
    break;
  case IDT_ERROR   :
  case IDT_FIELD   :
  case IDT_CLASS   :
  case IDT_INSTANCE:
    if (!bFirstBad) return O_K;
    else            return IVIR_BADCHAR;
    break;
  }

  return IVIR_BADTYPE;
}

char* dlp_errorcode2id(char* string1, INT16 errorcode)
{
  char lpBuf[255]; sprintf(lpBuf,"%d",(int)abs(errorcode));
  sprintf(string1,"~e%c_%c_%c__%c",lpBuf[3],lpBuf[2],lpBuf[1],lpBuf[0]);

  return string1;
}

/* EOF */
