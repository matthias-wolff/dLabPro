// dLabPro class CFunction (function)
// - Source scanner
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
#ifndef __NOITP
  #include "dlp_itp.h"
#endif // #ifndef __NOITP

#define TSQ_CONVERT(TSQ,C,S) \
  CData_Mark(TSQ,C,1); \
  ISETOPTION(TSQ,"/mark"); ISETOPTION(TSQ,"/force"); \
  CData_Tconvert(TSQ,TSQ,S); \
  IRESETOPTIONS(TSQ); \
  CData_Unmark(TSQ); \

/**
 * Optimizes the token sequence. Saves memory by stripping comments and empty
 * lines from a token sequence. Removes unnecessary white spaces and reduces the
 * sizes of the token and white space components.
 *
 * <p>If the token sequence contains here script tags (<code>&lt;%</code> and
 * <code>&gt;%</code>), the method does nothing in order to preserve the
 * here script.</p>
 *
 * @param idTsq
 *          The token sequence (no checks performed!)
 */
void CGEN_SPROTECTED CFunction::TsqStrip(CData* idTsq)
{
  INT32 nLen    = 0;
  INT32 nMaxLen = 0;

  if (!idTsq) return;

  // Pass I -- Analyze token sequence
  for (INT32 nTok=0; nTok<idTsq->GetNRecs(); nTok++)
  {
  	if (__TOK_IS_EX(idTsq,nTok,"<%")) return;                                   //   Do not strip here scripts!
  	if (__TOK_IS_EX(idTsq,nTok,"%>")) return;                                   //   Do not strip here scripts!
  }

  // Pass II -- Remove comment tokens and blank lines
  for (INT32 nTok=0; nTok<idTsq->GetNRecs(); )
  {
  	// Delete comments and while lines
    if
    (
      __TTYP_IS_EX(idTsq,nTok,TT_LCMT) ||
      __TTYP_IS_EX(idTsq,nTok,TT_BCMT) ||
      __TTYP_IS_EX(idTsq,nTok,TT_DCMT) ||
      __TTYP_IS_EX(idTsq,nTok,TT_ELIN)
    )
    {
      idTsq->DeleteRecs(nTok,1);
      continue;
    }

    // Get maximal token length
    nLen = dlp_strlen(__TOK_EX(idTsq,nTok));
    if (nLen>nMaxLen) { nMaxLen = nLen; }

    // Minimize delimiter characters
    dlp_strcpy(__IDEL_EX(idTsq,nTok)," ");

    nTok++;
  }

  // Shrink components of token sequence
  TSQ_CONVERT(idTsq,OF_TOK,nMaxLen+1);
  TSQ_CONVERT(idTsq,OF_IDEL,2);

}

/**
 * Adds a file name to the functions source file table.
 *
 * @param lpsFilename
 *          Pointer to a null-terminated string containing the file name
 * @return The index of the added file name in the source file table
 * @see sfl field m_idSfl
 */
INT32 CGEN_PROTECTED CFunction::AddSrcFile(const char* lpsFilename)
{
  INT32 nSrc = 0;                                                               // Index of current source file

  DLPASSERT(m_idSfl);                                                           // Need source file table
  if (CData_GetCompType(m_idSfl,0)!=L_SSTR)                                     // Source file table corrupt?
  {                                                                             // >>
    CData_Reset(m_idSfl,TRUE);                                                  //   Clear source file list
    CData_AddComp(m_idSfl,"file",L_SSTR);                                       //   Add file component to source list
  }                                                                             // <<

  char* lpsBuf = (char*)dlp_calloc(dlp_strlen(lpsFilename)+1,sizeof(char));     // Create a string buffer
  dlp_strcpy(lpsBuf,lpsFilename);                                               // Copy file name
  dlp_strreplace(lpsBuf,"\\","/");                                              // Convert to canonical form
  for (nSrc=0; nSrc<CData_GetNRecs(m_idSfl); nSrc++)                            // Loop over source files
    if (dlp_strpcmp(lpsBuf,(const char*)CData_XAddr(m_idSfl,nSrc,0))==0)        //   File name already listed
    {                                                                           //   >>
      dlp_free(lpsBuf);                                                         //     Free string buffer
      return nSrc;                                                              //     Well...
    }                                                                           //   <<

  nSrc = CData_AddRecs(m_idSfl,1,10);                                           // Add entry to source file table
  CData_Sstore(m_idSfl,lpsBuf,nSrc,0);                                          // Store file name
  dlp_free(lpsBuf);                                                             // Free string buffer
  return nSrc;                                                                  // Return index of new table entry
}

/**
 * Determines the source file name of a token in a token sequence.
 *
 * @param nTok
 *          The token index in <code>idTsq</code>
 * @param idTsq
 *          The token sequence (may be <code>NULL</code>, in this case the method
 *          will use the function's token sequence {@link tsq m_isTsq})
 * @return A pointer to a null-terminated string containing the source file name
 *         or <code>NULL</code> in case of errors.
 */
char* CGEN_PUBLIC CFunction::GetSrcFile(INT32 nTok, CData* idTsq DEFAULT(NULL))
{
  if (!idTsq) idTsq = m_idTsq;
  return (char*)CData_XAddr(m_idSfl,(INT32)CData_Dfetch(idTsq,nTok,OF_SRCID),0);
}

/**
 * Tokenizes a source file and inserts the tokens at the specified position.
 *
 * @param lpsFilename
 *          Path to source file (absolute or relative to the source file of
 *          token <code>nPos</code>-1).
 * @param nPos
 *          Index in {@link tsq} to insert the new token sequence at.
 * @param iParser
 *          The parser to tokenize the source file with.
 * @param lpsParserType
 *          The parser type for the second tokenizing pass.
 * @return <code>O_K</code> if successful, a (negative) error code otherwise
 */
INT16 CGEN_PROTECTED CFunction::IncludeEx
(
  const char* lpsFilename,
  INT32       nPos,
  CDgen*      iParser DEFAULT(NULL),
  const char* lpsParserType DEFAULT("dlp")
)
{
  INT32  nTok             = 0;                                                  // Current token
  INT32  nXTok            = 0;                                                  // Total number of included tokens
  INT32  nCS              = 0;                                                  // Size of string components
  CDgen* iPar             = iParser!=NULL?iParser:GetDlpParser();               // Get dLabPro parser
  char   lpsRenvFn[L_PATH];                                                     // Include file name (env. replaced)
  char   lpsInclFn[L_PATH];                                                     // Include file name (fully qualified)
  char   lpsCurrFn[L_PATH];                                                     // Current file name

  // Preprocess file name                                                       // ------------------------------------
  dlp_strcpy(lpsRenvFn,lpsFilename);                                            // Copy name of file to include
  dlp_strreplace_env(lpsRenvFn,FALSE);                                          // Replace reference to env. variables
  if (!dlp_fullpath(lpsInclFn,lpsRenvFn,L_PATH))                                // No valid absolute path
  {                                                                             // >>
    char lpsDir[L_PATH];                                                        //   Buffer for directory name
    char lpsPath[L_PATH];                                                       //   Buffer for alternative path
    dlp_strcpy(lpsCurrFn,GetSrcFile(nPos-1));                                   //   Get current file name
    dlp_splitpath(lpsCurrFn,lpsDir,NULL);                                       //   Get directory of current file
    sprintf(lpsPath,"%s%c%s",lpsDir,C_DIR,lpsFilename);                         //   Make relative path to include
    if (!dlp_fullpath(lpsInclFn,lpsPath,L_PATH))                                //   Try converting to abs. path
      return IERROR(this,ERR_FILEOPEN,lpsFilename,"reading",0);                 //     Failed: error message
  }                                                                             // <<

  // Validate                                                                   // ------------------------------------
  DLPASSERT(iPar);                                                              // Need tokenizer instance
  DLPASSERT(m_idTsq);                                                           // Need token sequence instance
  DLPASSERT(m_idSfl);                                                           // Need source file table

  // Store source file in table                                                 // ------------------------------------
  INT32 nSrc = AddSrcFile(lpsInclFn);                                           // Add record to source file table
  if (nSrc!=CData_GetNRecs(m_idSfl)-1)                                          // File already in source list
    return IERROR(this,FNC_DBL,lpsInclFn,"included",0);                         //   Prevent from including twice

  // Tokenize                                                                   // ------------------------------------
  INT16 nErr1 = iPar->Tokenize(lpsInclFn);                                      // Tokenize file (pass I)
  INT16 nErr2 = iPar->Tokenize2(lpsParserType);                                 // Tokenize file (pass II)
  IF_NOK(nErr1) return nErr1;                                                   // (Late) return on pass I errors
  IF_NOK(nErr2) return nErr2;                                                   // (Late) return on pass II errors
  nXTok = CData_GetNRecs(iPar->m_idTsq);                                        // Count new tokens
  if (!nXTok) return O_K;                                                       // No new tokens -> forget it

  // Insert tokens into token sequence                                          // ------------------------------------
  for (nTok=nXTok-1; nTok>=0; nTok--)                                           // Loop over new tokens
  {                                                                             // >>
    if (nPos>=0 && nPos<CData_GetNRecs(m_idTsq))                                //   If including into existing seq.
    {                                                                           //   >>
      __BLV_EX(iPar->m_idTsq,nTok,0)+=__BLV(nPos,0);                            //     Adjust curly brace level
      __BLV_EX(iPar->m_idTsq,nTok,1)+=__BLV(nPos,1);                            //     Adjust if level
      __BLV_EX(iPar->m_idTsq,nTok,2)+=__BLV(nPos,2);                            //     Adjust if level
    }                                                                           //   <<
    CData_Dstore(iPar->m_idTsq,nSrc,nTok,OF_SRCID);                             //   Store source file index
  }                                                                             // <<

  if(!CData_IsEmpty(m_idTsq))                                                   // If there are records in token queue
  {                                                                             // >>
    nCS = MAX(CData_GetCompType(      m_idTsq,OF_TOK),                          //   Get maximum token size
              CData_GetCompType(iPar->m_idTsq,OF_TOK));                         //   |
    TSQ_CONVERT(      m_idTsq,OF_TOK,nCS);                                      //   Make string comp. length equal
    TSQ_CONVERT(iPar->m_idTsq,OF_TOK,nCS);                                      //   |
    nCS = MAX(CData_GetCompType(      m_idTsq,OF_IDEL),                         //   Get maximum del. size
              CData_GetCompType(iPar->m_idTsq,OF_IDEL));                        //   |
    TSQ_CONVERT(      m_idTsq,OF_IDEL,nCS);                                     //   Make them equal
    TSQ_CONVERT(iPar->m_idTsq,OF_IDEL,nCS);                                     //   |
  }                                                                             // <<

  nPos = CData_InsertRecs(m_idTsq,nPos,nXTok,100);                              // Insert records in token sequence
  ISETOPTION(m_idTsq,"/rec");                                                   // Switch record option on in m_idTsq
  CData_Xstore(m_idTsq,iPar->m_idTsq,0,nXTok,nPos);                             // Store the new tokens
  IRESETOPTIONS(m_idTsq);                                                       // Reset all options of m_idTsq
  CData_AllocateUninitialized(iPar->m_idTsq,0);                                 // Clear tokenizer data

  return O_K;                                                                   // All done
}

/*
 * Manual page at function.def
 */
INT16 CGEN_PUBLIC CFunction::Load(const char* lpsFilename)
{
  // Validate                                                                   // ------------------------------------
  DLPASSERT(m_idSfl);                                                           // Need source file table
  if (m_nXm&XM_EXEC)                                                            // Function running?
    return IERROR(this,FNC_NOTALLOWED,"-load","in running function",0);         //   Load not allowed!

  // Clear previous function body                                               // ------------------------------------
  CDgen::TsqInit(m_idTsq);                                                      // Clear token sequence
  CDgen::TsqInit(m_idTeq);                                                      // Clear token execution queue

  // Include source file                                                        // ------------------------------------
  if (IncludeEx(lpsFilename,0)>0) return O_K;                                   // Include source at position 0
  return NOT_EXEC;                                                              // Something happened on the way...
}

/*
 * Manual page at function.def
 */
INT16 CGEN_PUBLIC CFunction::Define()
{
  INT16        nErr = O_K;                                                      // Return value
  const SMic* lpMic = CDlpObject_MicGet(this);                                  // Get method invocation context
  CFunction* iCaller =                                                          // Get calling function
    (CFunction*)CDlpObject_OfKind("function",lpMic->iCaller);                   // |

  if (!iCaller                 ) return NOT_EXEC;                               // Need caller
  if (!iCaller->m_nXm & XM_EXEC) return NOT_EXEC;                               // Caller must be running
  DLPASSERT(iCaller->m_idTsq);                                                  // This cannot happen

  if (iCaller->m_bInline) m_nXm |= XM_INLINE;                                   // Set inline execution mode
  iCaller->m_bInline = FALSE;
  char* lpsFilename = iCaller->GetSrcFile(iCaller->m_nPp);                      // Get source file name from caller
  INT32 nTokS       = iCaller->m_nPp;                                           // This is the first token to copy
  INT32 nTokE       = -1;                                                       // And this will be the last
  INT32 nBlv        = __BLV_EX(iCaller->m_idTsq,nTokS-1,0);                     // The current curly brace lavel
  for (nTokE=nTokS; nTokE<iCaller->m_idTsq->GetNRecs(); nTokE++)                // Seek closing curly brace
  {                                                                             // >>
    if                                                                          //   This is:
    (                                                                           //   |
      __BLV_EX    (iCaller->m_idTsq,nTokE,0  )==nBlv &&                         //   | a token on the same brace level,
      __TTYP_IS_EX(iCaller->m_idTsq,nTokE,"?")       &&                         //   | of type "?",
      __TOK_IS_EX (iCaller->m_idTsq,nTokE,"}")                                  //   | whose string equals to "}"
    )                                                                           //   |
    {                                                                           //   >>
      break;                                                                    //     Gotcha ...
    }                                                                           //   <<
  }                                                                             // <<
  iCaller->m_nPp = nTokE+1;                                                     // Move caller's program pointer
  if (nTokE>=iCaller->m_idTsq->GetNRecs())                                      // Past end of token sequence?
    return IERROR(this,FNC_EXPECT,"} at end of function definition",0,0);       //   oops
  if (m_nXm&XM_EXEC)                                                            // Function running?
    return IERROR(this,FNC_NOTALLOWED,"{","in running function",0);             //   Load not allowed!

  nErr = m_idTsq->SelectRecs(iCaller->m_idTsq,nTokS,nTokE-nTokS);               // Copy tokens from caller
  CDgen::TsqInit(m_idTeq);                                                      // Clear token execution queue
  TsqStrip(m_idTsq);                                                            // Save memory
  CData_Reset(m_idSfl,TRUE);                                                    // Clear source file table
  AddSrcFile(lpsFilename);                                                      // Add source file entry

  for (nTokS=0; nTokS<m_idTsq->GetNRecs(); nTokS++)                             // Loop over acquired tokens
  {                                                                             // >>
    m_idTsq->Dstore(__BLV(nTokS,0)-nBlv,nTokS,OF_BLV0 );                        //   Adjust curly brace level
    m_idTsq->Dstore(0                  ,nTokS,OF_SRCID);                        //   Adjust source file index
  }                                                                             // <<

  return nErr;                                                                  // Return error state
}

// EOF
