// dLabPro class CDgen (DGen)
// - Interactive methods
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

#include "dlp_dgen.h"

/*
 * Manual page at dgen_man.def
 */
INT16 CGEN_PUBLIC CDgen::Setup(const char* lpsParser)
{
  // Parser specific setup (leave settings alone if no parser specified)
  if (dlp_strcmp(lpsParser,"uasr")==0 || dlp_strcmp(lpsParser,"dlp")==0 || dlp_strcmp(lpsParser,"perl")==0)
  {
    dlp_strcpy(m_lpsLcmt   ,"#"    );
    dlp_strcpy(m_lpsBcmton ,""     );
    dlp_strcpy(m_lpsBcmtoff,""     );
    dlp_strcpy(m_lpsDcmt   ,"##"   );
    dlp_strcpy(m_lpsDrct   ,""     );
    dlp_strcpy(m_lpsSdel   ,":;"   );
    dlp_strcpy(m_lpsIdel   ," \t\n");
    m_bChrs = FALSE;
  }
  else if (dlp_strcmp(lpsParser,"def")==0)
  {
    dlp_strcpy(m_lpsLcmt   ,"#"     );
    dlp_strcpy(m_lpsBcmton ,""      );
    dlp_strcpy(m_lpsBcmtoff,""      );
    dlp_strcpy(m_lpsDcmt   ,"##"    );
    dlp_strcpy(m_lpsDrct   ,""      );
    dlp_strcpy(m_lpsSdel   ,";"     );
    dlp_strcpy(m_lpsIdel   ," \t\n");
    m_bChrs = FALSE;
    //m_bStrs = FALSE;
  }
  else if (dlp_strcmp(lpsParser,"cpp")==0)
  {
    dlp_strcpy(m_lpsLcmt   ,"//"      );
    dlp_strcpy(m_lpsBcmton ,"/*"      );
    dlp_strcpy(m_lpsBcmtoff,"*/"      );
    dlp_strcpy(m_lpsDcmt   ,"/**"     );
    dlp_strcpy(m_lpsDrct   ,"#"       );
    dlp_strcpy(m_lpsSdel   ,",;(){}[]");
    dlp_strcpy(m_lpsIdel   ," \r\n\t" );
    m_bChrs = TRUE;
  }
  else if (dlp_strcmp(lpsParser,"fml")==0)
  {
    dlp_strcpy(m_lpsLcmt   ,"#"                    );
    dlp_strcpy(m_lpsBcmton ,""                     );
    dlp_strcpy(m_lpsBcmtoff,""                     );
    dlp_strcpy(m_lpsDcmt   ,"##"                   );
    dlp_strcpy(m_lpsSdel   ,"+-*/^,&|!=<>()[]{}\'%");
    dlp_strcpy(m_lpsIdel   ,":; \r\t\n"            );
    m_bChrs = FALSE;
  }
  else return IERROR(this,DG_PARSER,lpsParser,0,0);
  return O_K;
}

/*
 * Manual page at dgen_man.def
 */
INT16 CGEN_PUBLIC CDgen::Scan(const char* lpsFilename, const char* lpsParser, CFst* itDeps)
{
  IF_NOK(Setup    (lpsParser  )) return NOT_EXEC;                               // Load default setup
  IF_NOK(Tokenize (lpsFilename)) return NOT_EXEC;                               // Tokenizing (1st pass)
  IF_NOK(Tokenize2(lpsParser  )) return NOT_EXEC;                               // Tokenizing (2nd pass)
  return Parse(lpsParser,itDeps,-1,-1);                                         // Parsing
}

/*
 * Manual page at dgen_man.def
 */
INT16 CGEN_PUBLIC CDgen::Parse(const char* lpsParser, CFst* itDeps, INT32 nFtok, INT32 nLtok)
{
  if (nFtok< 0                      ) nFtok = 0;
  if (nFtok>=CData_GetNRecs(m_idTsq)) nFtok = CData_GetNRecs(m_idTsq)-1;
  if (nLtok< 0                      ) nLtok = CData_GetNRecs(m_idTsq)-1;
  if (nLtok>=CData_GetNRecs(m_idTsq)) nLtok = CData_GetNRecs(m_idTsq)-1;

  IFCHECKEX(1)
  {
    printf("\n -------------------------------------------------------------------------");
    printf("\n  CDgen::Parse"                                                            );
    printf("\n  "                                                                        );
    printf("\n  Parser          : \"%s\"",lpsParser?lpsParser:"(null)"                   );
    printf("\n  Dependency graph: %s",itDeps?itDeps->m_lpInstanceName:"(null)"           );
    printf("\n  First token     : %ld",(long)nFtok                                       );
    printf("\n  Last token      : %ld",(long)nLtok                                       );
    printf("\n -------------------------------------------------------------------------");
  }

  if (dlp_strcmp(lpsParser,"dlp")==0)
  {
    IFCHECKEX(1) printf("\n  Invoking CDgen::DlpParser");
    DlpParser(itDeps,nFtok,nLtok);
    m_idDom->Reset();
  }
  if (dlp_strcmp(lpsParser,"uasr")==0)
  {
    IFCHECKEX(1) printf("\n  Invoking CDgen::UasrParser");
    UasrParser(itDeps,nFtok,nLtok);
  }
  else if (dlp_strcmp(lpsParser,"cpp")==0)
  {
    IFCHECKEX(1) printf("\n  Invoking CDgen::CppParser");
    CppParser(nFtok,nLtok);
  }
  else if (dlp_strcmp(lpsParser,"fml")==0)
  {
    IFCHECKEX(1) printf("\n  Invoking CDgen::FmlParser");
    FmlParser(nFtok,nLtok);
  }
  else if (dlp_strcmp(lpsParser,"perl")==0)
  {
    IFCHECKEX(1) printf("\n  Invoking CDgen::PerlParser");
    PerlParser(nFtok,nLtok);
  }
  else return IERROR(this,DG_PARSER,lpsParser,0,0);

  IFCHECKEX(1)
    printf("\n -------------------------------------------------------------------------\n");

  return O_K;
}

// EOF
