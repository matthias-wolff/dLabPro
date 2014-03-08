// dLabPro SDK class CCgen (cgen)
// - Processing control methods
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

// -- Auxiliary functions --

/**
 * Scans the argument for the next line break (\n, \r, or \r\n) and replaces the
 * line break character(s) by '\0'.
 *
 * @param tx
 *          The string to scan.
 * @return
 *   A pointer to the first character after the line break (which may be another
 *   line break), or <code>NULL</code> if the argument is <code>NULL</code> or
 *   an empty string.
 */
char* __lbsc(char* tx)
{
  if (tx==NULL || *tx=='\0') return NULL;
  char* ty;
  for (ty=tx; *ty!='\0'; ty++)
  {
    if (*ty=='\r' && *(ty+1)=='\n')
    {
      *ty++='\0'; *ty++='\0';
      return ty;
    }
    if (*ty=='\n' || *ty=='\r')
    {
      *ty++='\0';
      return ty;
    }
  }
  return NULL;
}

/**
 * Removes trailing white spaces until (and including) the last line break in a
 * text.
 *
 * @param lpsText
 *          The text (content will be modified!)
 */
void __rllb(char* lpsText)
{
  if (dlp_strlen(lpsText)==0) return;

  for (char* tx=&lpsText[dlp_strlen(lpsText)-1]; tx>=lpsText; tx--)
  {
    if (!iswspace(*tx)) return;
    if (*tx=='\n' && tx>lpsText && *(tx-1)=='\r')
    {
      *tx--='\0';
      *tx  ='\0';
      return;
    }
    if (*tx=='\n' || *tx=='\r')
    {
      *tx='\0';
      return;
    }
    *tx='\0';
  }
}

// -- General getters and setters --

/**
 * Returns the verbose level.
 */
INT16 CGEN_PROTECTED CCgen::GetVerboseLevel()
{
  return m_nVerbose;
}

/**
 * Determines if a source file belongs to the dLabPro sdk package.
 *
 * @param lpsSourcePath
 *          The relative or absolute path name.
 */
BOOL CGEN_SPROTECTED CCgen::IsSdkResource(const char* lpsSourcePath)
{
  if (strstr(lpsSourcePath,"/sdk/"  )) return TRUE;                             // Path names
  if (strstr(lpsSourcePath,"\\sdk\\")) return TRUE;                             // Windows path names
  return FALSE;                                                                 // No SDK file
}

// -- Host token sequence methods --

/**
 * Returns the CCgen singleton instance.
 */
CCgen* CGEN_SPUBLIC CCgen::GetSingleton()
{
  return (CCgen*)CFunction::GetRootFnc()->FindInstance("cgen");
}

/**
 * Returns the current definition file name.
 */
const char* CGEN_PROTECTED CCgen::GetCurrentDefFile()
{
  return GetSrcFile(m_nTeqOffset,m_idTeq);
}

/**
 * Returns the current line number in the current definition file.
 */
INT32 CGEN_PROTECTED CCgen::GetCurrentDefLine()
{
  return __LINE_EX(m_idTeq,m_nTeqOffset);
}

/**
 * Gets the next token from the current definition script.
 *
 * @param lpsBuffer
 *          A pointer to a buffer to be filled with the token, must <em>not</em>
 *          be <code>NULL</code>. The size should be at least
 *          <code>L_INPUTLINE+1</code> characters.
 * @param lpsTtyp
 *          A pointer to a string pointer to be filled with a pointer to the
 *          token type string, can be <code>NULL</code>.
 * @return <code>lpsBuffer</code> or <code>NULL</code> if token could be read.
 */
char* CGEN_PROTECTED CCgen::GetNextDefToken
(
  char*        lpsBuffer,
  const char** lpsTtyp DEFAULT(NULL)
)
{
  if (lpsBuffer==NULL) return NULL;                                             // No buffer, no service

  const char* lpsTok = GetNextToken(FALSE);                                     // Get pointer to next token
  const char* lpsTty = __TTYP_EX(m_idTeq,m_nTeqOffset);                         // Get pointer to token type

  if (lpsTok==NULL) return NULL;                                                // No more tokens --> exit
  dlp_strcpy(lpsBuffer,lpsTok);                                                 // Copy token to buffer
  if (dlp_strcmp(lpsTty,TT_STR)==0)                                             // If token is a string literal
  {                                                                             // >>
    dlp_memmove(&lpsBuffer[1],lpsBuffer,dlp_strlen(lpsBuffer)+1);               //   Restore quotation marks
    *lpsBuffer='\"';                                                            //   ...
    dlp_strcat(lpsBuffer,"\"");                                                 //   ...
  }                                                                             // <<
  dlp_strreplace(lpsBuffer,"\\#","#");                                          // Restore "\#" -> "#"
  dlp_strreplace(lpsBuffer,"\\$","$");                                          // Restore "\$" -> "$"

  if (lpsTtyp!=NULL) *lpsTtyp = lpsTty;                                         // Copy pointer to token type
  return lpsBuffer;                                                             // Return pointer to buffer
}

/**
 * Returns a string representing the remaining tokens (including delimiters) in
 * the current line of the current definition file and advances the program
 * pointer to the first token of the following line. This implementation is
 * <em>not</em> thread safe!
 *
 * @return
 *   A pointer to a static buffer containing the remaining input line without
 *   trailing delimiters. The string is empty if there are no more tokens on the
 *   line. The method returns <code>NULL</code> if there are no more tokens at
 *   all.
 */
const char* CGEN_PROTECTED CCgen::GetRestOfDefLine()
{
  const char* lpsSrc   = GetCurrentDefFile();                                   // Source file of line to read
  INT32       nLine    = GetCurrentDefLine();                                   // Number of line to read
  static char __lpsOut[L_INPUTLINE+1];                                          // Static output buffer
  char        lpsTok[L_INPUTLINE+1];                                            // Current token
  const char* lpsTtyp  = NULL;                                                  // Pointer to token type

  dlp_memset(__lpsOut,0,L_INPUTLINE+1);                                         // Clear output buffer
  while (GetNextDefToken(lpsTok,&lpsTtyp)!=NULL)                                // As long as there are tokens...
  {                                                                             // >>
    if (dlp_strcmp(lpsSrc,GetCurrentDefFile())!=0 || nLine!=GetCurrentDefLine())//   Token not from the same line
    {                                                                           //   >>
      RefuseToken();                                                            //     Push it back
      break;                                                                    //     That was all on this line
    }                                                                           //   <<
    if (dlp_strcmp(lpsTtyp,TT_BCMT)==0) break;                                  //   Ignore block comment tokens
    if (dlp_strcmp(lpsTtyp,TT_DCMT)==0) break;                                  //   Ignore documentation comment tokens
    if (dlp_strcmp(lpsTtyp,TT_LCMT)==0) break;                                  //   Ignore line comment tokens
    dlp_strcat(__lpsOut,lpsTok);                                                //   Append token to output buffer
    dlp_strcat(__lpsOut,GetNextTokenDel());                                     //   Append delimiters following token
  }                                                                             // <<
  if (lpsTtyp==NULL && dlp_strlen(__lpsOut)==0) return NULL;                    // There were no more tokens at all

  dlp_strtrimright(__lpsOut);                                                   // Trim white spaces
//  printf("\n*** %s(%d) |",lpsSrc,nLine); __puts(__TOK_EX(iHost->m_idTeq,0));
//  __puts("| >>> |"); __puts(__lpsOut); __puts("|");
  return __lpsOut;                                                              // Return pointer to output buffer
}

/**
 * Copies source code lines from the current position in the definition file
 * into a string list. Any present contents of the list will be discarded.
 *
 * @param lList
 *          The string list to fill in.
 * @param lpsEndMark
 *          The end mark token, e. g. <code>S_ENDCODE</code> or
 *          <code>S_ENDMAN</code>.
 * @return <code>O_K</code> if successful, a (negative) error code otherwise.
 */
INT16 CGEN_PROTECTED CCgen::FillIn(CList<SCGStr> &lList, const char* lpsEndMark)
{
  DLPASSERT(lpsEndMark!=NULL);                                                  // Need end mark
  lList.Delete();                                                               // Clear string list

  char*       lpsTxt;                                                           // Text buffer
  char*       t0      = NULL;                                                   // Auxiliary character pointer
  char*       tx;                                                               // Auxiliary character pointer
  char        lpsTok[L_INPUTLINE+1];                                            // Current token
  const char* lpsDel;                                                           // Delimiters following current token
  const char* lpsFile = GetCurrentDefFile();                                    // Remember definition file
  INT32       nLine   = GetCurrentDefLine();                                    // Remember line in definition file
  INT16       nErr    = O_K;                                                    // Return value

  // Collect text from host function's token stream                             // ------------------------------------
  lpsTxt = (char*)dlp_calloc(10000,sizeof(char));                               // Allocate initial text buffer
  dlp_strcpy(lpsTok,GetNextTokenDel());                                         // Copy delimiter before first token
  dlp_strcat(lpsTxt,__lbsc(lpsTok));                                            // Add chars. AFTER first line break
  while (GetNextDefToken(lpsTok,NULL)!=NULL)                                    // Read tokens
  {                                                                             // >>
    IFCHECKEX(2)
      dlp_puts_ex(3,"\n                            :   - |",lpsTok,"|");
    if (dlp_strcmp(lpsTok,lpsEndMark)==0) break;                                //   End mark found --> break loop
    lpsDel = GetNextTokenDel();                                                 //   Get delimiters following token
    if                                                                          //   Need to enlarge text buffer
    (                                                                           //   ...
      dlp_strlen(lpsTxt)+dlp_strlen(lpsTok)+dlp_strlen(lpsDel)                  //   ...
      >=dlp_size(lpsTxt)                                                        //   ...
    )                                                                           //   ...
    {                                                                           //   >>
      lpsTxt = (char*)dlp_realloc(lpsTxt,dlp_size(lpsTxt)+10000,sizeof(char));  //     Reallocate text buffer
    }                                                                           //   <<
    dlp_strcat(lpsTxt,lpsTok);                                                  //   Append token
    if (__TTYP_IS_EX(m_idTeq,m_nTeqOffset,TT_ELIN))                             //   If current token is an empty line
      dlp_strcat(lpsTxt,"\n");                                                  //     Append line break
    dlp_strcat(lpsTxt,lpsDel);                                                  //   Append delimiters
  }                                                                             // <<
  if (lpsTok==NULL)                                                             // No end mark until end of stream
  {                                                                             // >>
    CDlpObject_SetErrorPos(lpsFile,nLine);                                      //   Rewind error position
    nErr = IERROR(this,ERR_EXPECTENDMARK,lpsEndMark,0,0);                       //   Error message
  }                                                                             // <<
  __rllb(lpsTxt);                                                               // Remove (one) trailing white line

  // Break collected text into lines                                            // ------------------------------------
  for (tx=__lbsc(lpsTxt),t0=lpsTxt; t0!=NULL; t0=tx,tx=__lbsc(tx))              // Break into lines...
    lList.AddItem(t0);                                                          //   ... and add to list

  dlp_free(lpsTxt);                                                             // Free text buffer
  return nErr;
}

// -- Implementations of processing control commands --

/**
 * Loads a definition script into the token list of a host function.
 *
 * @param lpsDefFile
 *          The file name of the definition script relative to the current
 *          working directory.
 * @return <code>O_K</code> if successful, a (negative) error code otherwise.
 */
INT16 CGEN_VPUBLIC CCgen::Include(const char* lpsDefFile)
{
  printf("\n%s %s",GetVerboseLevel()>1?" ":"i -",lpsDefFile);
  if (IncludeEx(lpsDefFile,m_nPp,m_iParser,"def")>0) return O_K;
  return NOT_EXEC;
}

INT16 CGEN_PROTECTED CCgen::Cgen()
{
  // -- Finish previous section
  SetCreating(CR_NOTHING,NULL);

  // -- Do nothing if interpreted as part of a derived class
  if (m_nAncestor)
  {
    // Post-process ancestor DOM
    m_rnts.Delete();                                                            // Delete ancestor's release notes

    // -- reset option /cProject
    m_bCProject = FALSE;
    return O_K;
  }

  // -- Get options from command line
  //    TODO: Use standard option mechanism
  INT16 flags = 0;
  char tx[L_INPUTLINE+1];
  while (GetNextDefToken(tx)!=NULL)
  {
    if (dlp_strncmp(tx,"/noHFile"  ,255) ==0) flags |= MAK_NOHFILE;
    if (dlp_strncmp(tx,"/noCppFile",255) ==0) flags |= MAK_NOCPPFILE;
    if (dlp_strncmp(tx,"/noMakFile",255) ==0) flags |= MAK_NOMAKFILE;
    if (dlp_strncmp(tx,"/noManFile",255) ==0) flags |= MAK_NOMANFILE;
    if (dlp_strncmp(tx,"/noHTMLMan",255) ==0) flags |= MAK_NOHTMLMAN;
    else if (dlp_strncmp(tx,"/ifnotexists",255) ==0) flags |= MAK_IFNOTEX;
  }

  // -- Complete project information
  IF_NOK (CompleteSysFunctions()) ERRORRET(ERR_VERIFYDATA,0,0,0,NOT_EXEC);
  IF_NOK (CompleteProject()) ERRORRET(ERR_VERIFYDATA,0,0,0,NOT_EXEC);
  if (!m_bNoStdincl)
  {
    m_includes.InsertItem(NULL,"\"dlp_object.h\"");
    m_includes.InsertItem(NULL,"\"dlp_config.h\"");
  }

  // -- Stop on errors occurred
  if (CDlpObject::GetErrorCount() > 0)
  {
    CDlpObject::ErrorLog();
    return NOT_EXEC;
  }

  // -- Print project information
  if (GetVerboseLevel()>1 && !m_bClib)
  {
    if (!m_bMainProject)
    {
      printf("\nClass directory is '%s'...",m_lpsHomePath);
      printf("\nClass %s",m_lpsClass);
      if (dlp_strlen(m_lpsDlcParent)) printf(":%s",m_lpsDlcParent);
      else printf(" (base class)");
    }

    printf("\nDevelopment platform is ");
    switch(m_nPlatform)
    {
    case PF_MSDEV4: printf("Microsoft Developer Studio / Visual C++ 4.0"); break;
    case PF_MSDEV5: printf("Microsoft Visual Studio / Visual C++ 5.0"); break;
    case PF_MSDEV6: printf("Microsoft Visual Studio / Visual C++ 6.0"); break;
    case PF_GNUCXX: printf("GNU C++"); break;
    default:        printf("unknown");
    }
    if (m_bCProject) printf("\nProject mode is 'ANSI C/C++ Compatibility Mode'.");
    else             printf("\nProject mode is 'ANSI C++'.");
  }

  // -- Scan additional source files and create file templates
  CppScanFunctions();
  ScanHtmlManual();
  CreateTemplates();

  // -- Sort lists
  m_flds.SortUp();
  m_opts.SortUp();
  m_mths.SortUp();
  m_cfns.SortUp();

  // -- Create files
  //printf("\n*** NOTE %s(%d): Skipped file generation",__FILE__,__LINE__);
  if((!(flags & MAK_NOHFILE) || !(flags & MAK_NOCPPFILE))  && !m_bClib)
    IF_NOK (CreateSourceFiles())
      {CDlpObject::ErrorLog(); return NOT_EXEC;}

  if (!(flags & MAK_NOMANFILE))
    if (!OK(CreateHtmlManual()))
      {CDlpObject::ErrorLog(); return NOT_EXEC;}

  if (!(flags & MAK_NOMAKFILE) && !m_bClib)
    IF_NOK (CreateMakeFile(flags & MAK_IFNOTEX))
      {CDlpObject::ErrorLog(); return NOT_EXEC;}

  // -- Print error log
  CDlpObject::ErrorLog();
  if      (m_nFgen==1) printf("1 file generated.");
  else if (m_nFgen> 1) printf("%d files generated.",(short)m_nFgen);
  else                 printf("NO file generated.");

  // -- Whatever...
  return O_K;
}

INT16 CGEN_PROTECTED CCgen::Quit()
{
  // TODO: Why is CCgen::Quit() not executed?
  //printf("\n*** CCgen::Quit()");
  m_defList.AddItem(GetCurrentDefFile());

  if (m_nAncestor<=0)
    m_nXm |= XM_QUIT;
  else
    m_nAncestor--;
  SetCreating(CR_PROJECT,NULL);

  return O_K;
}

// EOF
