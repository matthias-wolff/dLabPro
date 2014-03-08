// dLabPro class CDgen (DGen)
// - C(++) scanner
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

/**
 * Move leading asterisk(s) of lpsSrc to end of lpsDst.
 *
 * @param lpsDst Pointer to string to append leading asterisks of lpsSrc to
 * @param lpsSrc Pointer to string to remove leading asterisks from 
 */
void CGEN_PROTECTED CDgen::CppMoveAsterisk(char* lpsDst, char* lpsSrc)
{
  if (!lpsDst) return;
  if (!lpsSrc) return;

  while (lpsSrc[0]=='*')
  {
    dlp_memmove(lpsSrc,&lpsSrc[1],dlp_strlen(lpsSrc));
    dlp_strcat(lpsDst,"*");
  }
}

/**
 * Parses a token sequence from m_idTsq as formal argument of a function and
 * appends the argument to the DOM (m_idDom). The token sequence must not
 * contain the leading and trailing list delimiters (',').
 *
 * @param nFtokA The first token of the argument in {@link m_idTsq}
 * @param nLtokA The last token of the argument in {@link m_idTsq}
 * @param bLastA TRUE to indicate the last item of the formal argument list
 * @return O_K if successfull, a (negative) error code otherwise
 */
INT16 CGEN_PROTECTED CDgen::CppParseArg(INT32 nFtokA, INT32 nLtokA, BOOL bLastA)
{
  INT32 nTok  = 0;      // Current token
  INT32 nLtok = -1;     // Last token of a group
  INT32 nArg  = 0;      // DOM index of current formal argument

  IFCHECKEX(2)
    printf("\n      Parsing arg. (%4ld->%4ld): |%s|",(long)nFtokA,(long)nLtokA,
      TsqGlue(nFtokA,nLtokA,TRUE));

  // Trim token sequence, return if empty
  TsqTrim(&nFtokA,&nLtokA);
  if (nFtokA>nLtokA)
  {
    IERRORAT(this,m_lpsFilename,(INT32)m_idTsq->Dfetch(nFtokA,OF_LINE),DG_SYNTAX,0,0,0);
    return DG_SYNTAX;
  }

  // Add a new DOM object
  nArg = m_idDom->AddRecs(1,50);
  m_idDom->Dstore(CDM_OT_FFARG,nArg,CDM_OF_DOBT );
  m_idDom->Dstore(nFtokA      ,nArg,CDM_OF_FTOK );
  m_idDom->Dstore(nLtokA      ,nArg,CDM_OF_LTOK );
  m_idDom->Dstore(-1          ,nArg,CDM_OF_FTOKD);
  m_idDom->Dstore(-1          ,nArg,CDM_OF_LTOKD);

  // Parse backward: default value, array specifier and identifier
  nTok=nLtokA;

  // Is ellipsis?
  if (__TOK_IS(nTok,"..."))
  {
    if (bLastA)
    {
      if (nFtokA==nLtokA)
      {
        m_idDom->Sstore("...",nArg,CDM_OF_NAME);
        m_idDom->Sstore(""   ,nArg,CDM_OF_DSPC);
        return O_K;
      }
      // '...' must be the only token of the argument
      else
      {
        IERRORAT(this,m_lpsFilename,(INT32)m_idTsq->Dfetch(nTok,OF_LINE),
          DG_SYNTAX2,"Ellipis",0,0);
        return DG_SYNTAX2;
      }
    }
    // '...' must be the last argument
    else
    {
      IERRORAT(this,m_lpsFilename,(INT32)m_idTsq->Dfetch(nTok,OF_LINE),
        DG_SYNTAX2,"Ellipis must be last argument",0,0);
      return DG_SYNTAX2;
    }
  }

  // Parse backward - Handle dLabPro's default argument declaration DEFAULT(xxx)
  if (__BLV(nTok,1)==2 && __TOK_IS(nTok,")"))
  {
    for (nLtok=nTok; nTok>=nFtokA; nTok--) if
      (__BLV(nTok,1)==1)
        break;
    if (__TOK_IS(nTok,"(") && __TOK_IS(nTok-1,"DEFAULT"))
      m_idDom->Sstore(TsqGlue(nTok+1,nLtok-1,TRUE),nArg,CDM_OF_EXT2);
    else
    {
      // It is not a default argument --> don't care about it!
//      IERRORAT(this,m_lpsFilename,(INT32)m_idTsq->Dfetch(nTok,OF_LINE),
//        DG_SYNTAX2,"Invalid default argument",0,0);
//      return DG_SYNTAX;
    }
    nTok-=2;
  }

  // Parse backward - Handle array spec
  if (__BLV(nTok,2)==1 && __TOK_IS(nTok,"]"))
  {
    for (nLtok=nTok; nTok>nFtokA; nTok--)
      if (__BLV(nTok,2)==0)
        break;
    if (__TOK_IS(nTok,"["))
      m_idDom->Sstore(TsqGlue(nTok,nLtok,TRUE),nArg,CDM_OF_EXT1);
    else
    {
      IERRORAT(this,m_lpsFilename,(INT32)m_idTsq->Dfetch(nTok,OF_LINE),
        DG_SYNTAX2,"Invalid array declaration",0,0);
      return DG_SYNTAX2;
    }
    nTok--;
  }

  // Parse backward - Store identifier
  if (__TTYP_IS(nTok,TT_UNK))
    m_idDom->Sstore(TsqGlue(nTok,nTok,TRUE),nArg,CDM_OF_NAME);
  else
  {
    IERRORAT(this,m_lpsFilename,(INT32)m_idTsq->Dfetch(nTok,OF_LINE),
      DG_SYNTAX2,"Missing function argument identifier",0,0);
    return DG_SYNTAX;
  }

  // Any remaining tokens are decl-specs. (there must be at least one token)
  if (nLtokA<nFtokA)
  {
    IERRORAT(this,m_lpsFilename,(INT32)m_idTsq->Dfetch(nFtokA,OF_LINE),
      DG_SYNTAX2,"Missing decl-specs. of formal argument",0,0);
    return DG_SYNTAX2;
  }

  m_idDom->Sstore(TsqGlue(nFtokA,nTok-1,TRUE),nArg,CDM_OF_DSPC);
  CppMoveAsterisk((char*)m_idDom->XAddr(nArg,CDM_OF_DSPC),
    (char*)m_idDom->XAddr(nArg,CDM_OF_NAME));

  return O_K;
}

/**
 * Parses a token sequence from m_idTsq as formal argument list of a function
 * and appends the arguments to the DOM (m_idDom). The token sequence must not
 * contain the leading and trailing braces.
 *
 * @param nFtokL The first token of the argument list in m_idTsq
 * @param nLtokL The last token of the argument list in m_idTsq
 * @return The number of formal argument found
 */
INT32 CGEN_PROTECTED CDgen::CppParseArgl(INT32 nFtokL, INT32 nLtokL)
{
  INT32 nTok   = 0;      // Current token
  INT32 nFtokA = -1;     // First token of current formal argument
  INT32 nArgs  = 0;      // Number of arguments found (return value)

  IFCHECKEX(2)
    printf("\n      Parsing list (%4ld->%4ld): |%s|",nFtokL,nLtokL,
      TsqGlue(nFtokL,nLtokL,TRUE));

  // Trim token sequence, return if empty
  TsqTrim(&nFtokL,&nLtokL);
  if (nFtokL>nLtokL) return 0;

  // Seek list separators (',')
  for (nTok=nFtokL,nFtokA=nFtokL; nTok<=nLtokL; nTok++)
    if
    (
      (__TOK_IS(nTok,",") && __BLV(nTok,0)==0 && (__BLV(nTok,1)==1 && __BLV(nTok,2)==0)) ||
      nTok==nLtokL
    )
    {
      CppParseArg(nFtokA,nTok-(nTok==nLtokL?0:1),nTok==nLtokL);
      nFtokA=nTok+1;
      nArgs++;
    }

  return nArgs;
}

/**
 * C++ scanner. Scans token sequence m_idTsq (filled by CDgen::Tokenize) and
 * builds DOM (m_idDom).
 *
 * @param nFtok First token of m_idTsq to parse
 * @param nLtok Last token of m_idTsq to parse (-1: untill the end)
 * @see -tokenize
 */
void CGEN_PROTECTED CDgen::CppParser(INT32 nFtok, INT32 nLtok)
{
  // Local variables
  INT32 nTok   = 0;                     // Current token index
  INT32 nFunc  = 0;                     // DOM index of current function
  INT32 nState = 0;                     // Function header state machine
  INT32 nFtokF = -1;                    // First token of function header
  INT32 nLtokF = -1;                    // Last token of function header
  INT32 nFtokL = -1;                    // First token of formal argument list
  INT32 nLtokL = -1;                    // Last token of formal argument list

  // Check token sequence
  DLPASSERT(m_idTsq);
  if (nFtok< 0                      ) nFtok = 0;
  if (nFtok>=CData_GetNRecs(m_idTsq)) nFtok = CData_GetNRecs(m_idTsq)-1;
  if (nLtok< 0                      ) nLtok = CData_GetNRecs(m_idTsq)-1;
  if (nLtok>=CData_GetNRecs(m_idTsq)) nLtok = CData_GetNRecs(m_idTsq)-1;

  // Initialize - DOM instance
  DLPASSERT(m_idDom);
  DLPASSERT(L_NAMES<256);              // Component allocation will fail!
  if
  (
    m_idDom->GetNComps()>=9                            &&
    dlp_is_numeric_type_code (m_idDom->GetCompType(0)) &&
    dlp_is_symbolic_type_code(m_idDom->GetCompType(1)) &&
    dlp_is_symbolic_type_code(m_idDom->GetCompType(2)) &&
    dlp_is_symbolic_type_code(m_idDom->GetCompType(3)) &&
    dlp_is_symbolic_type_code(m_idDom->GetCompType(4)) &&
    dlp_is_numeric_type_code (m_idDom->GetCompType(5)) &&
    dlp_is_numeric_type_code (m_idDom->GetCompType(6)) &&
    dlp_is_numeric_type_code (m_idDom->GetCompType(7)) &&
    dlp_is_numeric_type_code (m_idDom->GetCompType(8))
  )
  {
    // Preserve user-defined component types
    m_idDom->Clear();
  }
  else
  {
    m_idDom->Reset();
    m_idDom->AddComp("dobt",T_SHORT);    // 0  CDM_OF_DOBT   Type of DOM object
    m_idDom->AddComp("dspc",L_NAMES);    // 1  CDM_OF_DSPC   Declaration specfifiers
    m_idDom->AddComp("name",L_NAMES);    // 2  CDM_OF_NAME   Identifier
    m_idDom->AddComp("ext1",L_NAMES);    // 3  CDM_OF_EXT1   Extra 1: sv-modifiers, array specifier
    m_idDom->AddComp("ext2",L_NAMES);    // 4  CDM_OF_EXT2   Extra 2: default value
    m_idDom->AddComp("ftok",T_INT  );    // 5  CDM_OF_FTOK   First token index in m_idTsq
    m_idDom->AddComp("ltok",T_INT  );    // 6  CDM_OF_LTOK   Last token index in m_idTsq
    m_idDom->AddComp("ftkd",T_INT  );    // 7  CDM_OF_FTOKD  First JavaDoc token index in m_idTsq
    m_idDom->AddComp("ltkd",T_INT  );    // 8  CDM_OF_LTOKD  Last JavaDoc token index in m_idTsq
  }

  // Loop over tokens
  for  (nTok=0; nTok<m_idTsq->GetNRecs(); nTok++)
  {
    // Curly brace on at all brace levels 0 --> This might be a function definition
    if
    (
      __BLV(nTok,0)==0 && __BLV(nTok,1)==0 && __BLV(nTok,2)==0 &&    // All brace levels 0
      __TTYP_IS(nTok,TT_DEL) && __TOK_IS(nTok,"{")                   // Token is the delimiter '{'
    )
    {
      // Add a new function
      nFunc = m_idDom->AddRecs(1,50);
      m_idDom->Dstore(CDM_OT_FHEAD,nFunc,CDM_OF_DOBT );
      m_idDom->Dstore(-1          ,nFunc,CDM_OF_FTOKD);
      m_idDom->Dstore(-1          ,nFunc,CDM_OF_LTOKD);

      // Reset markers
      nLtokF = -1;
      nFtokL = -1;
      nLtokL = -1;

      // Seek backward for first token of function header
      for (nState=0,nFtokF=nTok-1; nFtokF; nFtokF--)
      {
        if (__TOK_IS (nFtokF,";"    )) break;    // Stop at previous expression
        if (__TTYP_IS(nFtokF,TT_DRCT)) break;    // Skip at compiler directives
        if (__TTYP_IS(nFtokF,TT_LCMT)) continue; // Skip line comments
        if (__TTYP_IS(nFtokF,TT_BCMT)) continue; // Skip block comments
        if (__TTYP_IS(nFtokF,TT_ELIN)) continue; // Skip empty lines

        // Handle documentation comments
        if (__TTYP_IS(nFtokF,TT_DCMT))
        {
          // Useful only before function header
          if (nState==2)
          {
            if (*(INT32*)m_idDom->XAddr(nFunc,CDM_OF_LTOKD)<0) *(INT32*)m_idDom->XAddr(nFunc,CDM_OF_LTOKD)=nFtokF;
            *(INT32*)m_idDom->XAddr(nFunc,CDM_OF_FTOKD) = nFtokF;
          }
          continue;
        }

        if (nState==0)
        {
          // Skip sv-modifiers
          if
          (
            __TOK_IS(nFtokF,"const"   ) ||
            __TOK_IS(nFtokF,"volatile")
          )
          {
            dlp_strcpy((char*)m_idDom->XAddr(nFunc,CDM_OF_EXT1),(char*)m_idTsq->XAddr(nFtokF,OF_TOK));
            if (nLtokF<0) nLtokF=nFtokF;
            continue;
          }

          // Previous other non-empty and non-comment token must be ')'
          if (!__TOK_IS(nFtokF,")")) break;
          else
          {
            nState=1;
            nLtokL=nFtokF-1;
            if (nLtokF<0) nLtokF=nFtokF;
          }
        }
        else if (nState==1)
        {
          // Seek beginning of formal argument list ('(')
          if (__BLV(nFtokF,1)==0) { nState=2; nFtokL=nFtokF+1; }
        }
        else if (nState==2)
        {
          // Go back unil end of statement (';') or any brace level >0
          if (__BLV(nFtokF,0)!=0) break;
          if (__BLV(nFtokF,1)!=0) break;
          if (__BLV(nFtokF,2)!=0) break;
          if (__TOK_IS(nFtokF,";")) break;
          if (__TTYP_IS(nFtokF,TT_DRCT)) break;
        }
      }
  
      // Seek forward
      if (nState==2)
      {
        for (nFtokF++; nFtokF<nTok; nFtokF++)
        {
          // Skip comments and white spaces
          if (__TTYP_IS(nFtokF,TT_LCMT)) continue;
          if (__TTYP_IS(nFtokF,TT_BCMT)) continue;
          if (__TTYP_IS(nFtokF,TT_DCMT)) continue;
          if (__TTYP_IS(nFtokF,TT_ELIN)) continue;
          if (__TTYP_IS(nFtokF,TT_WSPC)) continue;
          break;
        }

        // If there are any tokens left --> found function header
        if (nFtokF<nTok)
        {
          // Store token indices, identifier and decl-specs.
          m_idDom->Dstore(nFtokF,nFunc,CDM_OF_FTOK);
          m_idDom->Dstore(nLtokF,nFunc,CDM_OF_LTOK);
          m_idDom->Sstore(TsqGlue(nFtokF  ,nFtokL-3,TRUE),nFunc,CDM_OF_DSPC);
          m_idDom->Sstore(TsqGlue(nFtokL-2,nFtokL-2,TRUE),nFunc,CDM_OF_NAME);
          CppMoveAsterisk((char*)m_idDom->XAddr(nFunc,CDM_OF_DSPC),
            (char*)m_idDom->XAddr(nFunc,CDM_OF_NAME));

          // Parse argument  list
          CppParseArgl(nFtokL,nLtokL);
        }
        else m_idDom->DeleteRecs(nFunc,1);
      }
      else m_idDom->DeleteRecs(nFunc,1);
    }
  }
}

// EOF
