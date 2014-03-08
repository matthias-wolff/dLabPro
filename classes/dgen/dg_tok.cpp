// dLabPro class CDgen (DGen)
// - Tokenizer
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

static char __CDgen_TsqGlue_lpsBuffer[L_INPUTLINE];

/**
 * Clears in intializes a token sequence.
 *
 * @param idTsq
 *          The token sequence to be initialized.
 */
void CGEN_SPUBLIC CDgen::TsqInit(CData* idTsq)
{
  if (!idTsq) return;
  if
  (
    CData_GetCompType(idTsq,0)== T_INT  &&
    CData_GetCompType(idTsq,1)==      9 &&
    CData_GetCompType(idTsq,2)== L_SSTR &&
    CData_GetCompType(idTsq,3)==T_SHORT &&
    CData_GetCompType(idTsq,4)==T_SHORT &&
    CData_GetCompType(idTsq,5)==T_SHORT &&
    CData_GetCompType(idTsq,6)== L_SSTR &&
    CData_GetCompType(idTsq,7)== T_INT
  )
  {

  for(INT32 i = 0; i < CData_GetMaxRecs(idTsq); i++) {
    *((char*)CData_XAddr(idTsq,i,1)) = 0;
    *((char*)CData_XAddr(idTsq,i,2)) = 0;
    *((char*)CData_XAddr(idTsq,i,6)) = 0;
  }
  CData_SetNRecs(idTsq,0);

//     dlp_memset(CData_XAddr(idTsq,0,0),0,CData_GetRecLen(idTsq)*CData_GetMaxRecs(idTsq));
     return;
   }
  CData_Reset(idTsq,TRUE);
  CData_AddComp(idTsq,"line",T_INT  );    // OF_LINE
  CData_AddComp(idTsq,"ttyp",      9);    // OF_TTYP
  CData_AddComp(idTsq,"tok" ,L_SSTR );    // OF_TOK
  CData_AddComp(idTsq,"{"   ,T_SHORT);    // OF_BLV0
  CData_AddComp(idTsq,"("   ,T_SHORT);    // OF_BLV1
  CData_AddComp(idTsq,"["   ,T_SHORT);    // OF_BLV2
  CData_AddComp(idTsq,"isdl",L_SSTR );    // OF_ISDL
  CData_AddComp(idTsq,"sfl" ,T_INT  );    // OF_SRCID
}

/**
 * Prints the token sequence m_idTsq at stdout starting with token nFtok until
 * token nLtok (inclusively).
 *
 * @param nFtok First token to print
 * @param nLtok Last token to print (-1: until end of sequence)
 */
void CGEN_PROTECTED CDgen::TsqPrint(INT32 nFtok, INT32 nLtok)
{
  INT32 nTok = 0;

  if (nFtok<0                              ) nFtok = 0;
  if (nLtok<0 || nLtok>=m_idTsq->GetNRecs()) nLtok = m_idTsq->GetNRecs()-1;

  printf("\n Index {([ TT Token");
  for (nTok=nFtok; nTok<=nLtok; nTok++)
  {
    printf
    (
      "\n(%4ld) %d%d%d %-2s %-52s %-52s",(long)nTok,
      (int)__BLV(nTok,0),(int)__BLV(nTok,1),(int)__BLV(nTok,2),
      (char*)m_idTsq->XAddr(nTok,OF_TTYP),
      dlp_strquotate(dlp_strabbrv(dlp_get_a_buffer(),__TOK (nTok),50),'|','|'),
      dlp_strquotate(dlp_strabbrv(dlp_get_a_buffer(),__IDEL(nTok),50),'|','|')
    );
  }
}

/**
 * Creates a string consisting of the tokens nFtok through nLtok (inclusively)
 * from m_idTsq. The result is stored in a static buffer.
 *
 * @param nFtok First token to glue
 * @param nLtok Last token to glue
 * @param bNorm If <code>TRUE</code>, replace insignificant delimites by spaces
 * @return A pointer to a static buffer containing the resulting string
 */
char* CGEN_PROTECTED CDgen::TsqGlue(INT32 nFtok, INT32 nLtok, BOOL bNorm)
{
  INT32 nTok;

  dlp_memset(__CDgen_TsqGlue_lpsBuffer,0,L_INPUTLINE);

  if (nFtok<0                   ) nFtok = 0;
  if (nLtok<0                   ) nLtok = 0;
  if (nFtok>=m_idTsq->GetNRecs()) nFtok = m_idTsq->GetNRecs();
  if (nLtok>=m_idTsq->GetNRecs()) nLtok = m_idTsq->GetNRecs();
  if (nLtok<nFtok               ) return __CDgen_TsqGlue_lpsBuffer;

  for (nTok=nFtok; nTok<=nLtok; nTok++)                                         // Loop over tokens to be glued
  {                                                                             // >>
    if (__TTYP_IS(nTok,TT_STR)) dlp_strcat(__CDgen_TsqGlue_lpsBuffer,"\"");     //   Restore leading double quot. marks
    if (__TTYP_IS(nTok,TT_CHR)) dlp_strcat(__CDgen_TsqGlue_lpsBuffer,"\'");     //   Restore leading single quot. marks
    dlp_strcat(__CDgen_TsqGlue_lpsBuffer,(char*)m_idTsq->XAddr(nTok,OF_TOK));   //   Restore tokens
    if (__TTYP_IS(nTok,TT_STR)) dlp_strcat(__CDgen_TsqGlue_lpsBuffer,"\"");     //   Restore tailing double quot. marks
    if (__TTYP_IS(nTok,TT_CHR)) dlp_strcat(__CDgen_TsqGlue_lpsBuffer,"\'");     //   Restore tailing single quot. marks
    if (nTok<nLtok)                                                             //   For all but the last token
      dlp_strcat(__CDgen_TsqGlue_lpsBuffer,bNorm?" ":__IDEL(nTok));             //     Restore insigificant delimiters
  }                                                                             // <<
  return __CDgen_TsqGlue_lpsBuffer;                                             // Return pointer to static glue buffer
}

/**
 * Removes heading and trailing comments, white spaces and empty lines from the
 * token sequence nFtok through nLtok.
 *
 * @param lpFtok Pointer to index of first token of the sequence
 * @param lpLtok Pointer to index of last token of the sequence
 */
void CGEN_PROTECTED CDgen::TsqTrim(INT32* lpFtok, INT32* lpLtok)
{
  if (!lpFtok                     ) return;
  if (!lpLtok                     ) return;
  if (*lpFtok<0                   ) *lpFtok = 0;
  if (*lpLtok<0                   ) *lpLtok = 0;
  if (*lpFtok>=m_idTsq->GetNRecs()) *lpFtok = m_idTsq->GetNRecs()-1;
  if (*lpLtok>=m_idTsq->GetNRecs()) *lpLtok = m_idTsq->GetNRecs()-1;
  if (*lpFtok>=*lpLtok            ) return;

  for (; *lpFtok<=*lpLtok; (*lpFtok)++)
  {
    if (__TTYP_IS(*lpFtok,TT_LCMT)) continue;
    if (__TTYP_IS(*lpFtok,TT_BCMT)) continue;
    if (__TTYP_IS(*lpFtok,TT_DCMT)) continue;
    if (__TTYP_IS(*lpFtok,TT_WSPC)) continue;
    if (__TTYP_IS(*lpFtok,TT_ELIN)) continue;
    if (__TTYP_IS(*lpFtok,TT_DRCT)) continue;
    break;
  }
  for (; *lpLtok>=*lpFtok; (*lpLtok)--)
  {
    if (__TTYP_IS(*lpLtok,TT_LCMT)) continue;
    if (__TTYP_IS(*lpLtok,TT_BCMT)) continue;
    if (__TTYP_IS(*lpLtok,TT_DCMT)) continue;
    if (__TTYP_IS(*lpLtok,TT_WSPC)) continue;
    if (__TTYP_IS(*lpLtok,TT_ELIN)) continue;
    if (__TTYP_IS(*lpFtok,TT_DRCT)) continue;
    break;
  }
}

/**
 * Determines if a character is a token delimiter.
 *
 * @param nChar The character
 * @return DL_NONE   if the character is no delimiter
 *         DL_SIGN   if the character is a significant delimiter (a token by itself)
 *         DL_INSIGN if the character is an insignificant delimiter (to be deleted)
 */
INT16 CGEN_PROTECTED CDgen::TsqIsDelimiter(char nChar)
{
  char* p = NULL;

  for (p=m_lpsSdel; *p; p++) if (nChar==*p) return DL_SIGN;
  for (p=m_lpsIdel; *p; p++) if (nChar==*p) return DL_INSIGN;
  return DL_NONE;
}

/**
 * Stores one token in the token sequence m_idTsq.
 *
 * @param lpsToken
 *          The token (max. 256 characters incl. terminal '\0')
 * @param nLen
 *          The number of characters to be stored, may be -1 to store the entire
 *          string stored in lpsToken
 * @param lpsFile
 *          The scanned source file
 * @param nLine
 *          The current line in the scanned source file
 * @param lpsTtype
 *          The token type (max. 8 characters incl. the terminal '\0')
 */
void CGEN_PROTECTED CDgen::TsqStore
(
  const char* lpsToken,
  INT32        nLen,
  const char* lpsFile,
  INT32        nLine,
  const char* lpsTtype,
  const INT16 lpBlv[3]
)
{
  char lpsTtypeP[8];

  if (!lpsToken    ) return;
  if (nLen<0       ) nLen=(INT32)dlp_strlen(lpsToken);
  if (nLen>L_SSTR-1) { nLen=L_SSTR-1; IERROR(this,DG_TOOLONG,lpsFile,(long)nLine,"Token"); }

  // Reclassify token
  strcpy(lpsTtypeP,lpsTtype);
  if (dlp_strlen(m_lpsDcmt) && strstr(lpsToken,m_lpsDcmt)==lpsToken)
    strcpy(lpsTtypeP,TT_DCMT);

  // Store into token stream
  INT32 nRec        = CData_AddRecs(m_idTsq,1,m_nGrany);
  char* lpTokStore = (char*)CData_XAddr(m_idTsq,nRec,OF_TOK);

  CData_Dstore(m_idTsq,nLine    ,nRec,OF_LINE);
  CData_Sstore(m_idTsq,lpsTtypeP,nRec,OF_TTYP);
  CData_Dstore(m_idTsq,lpBlv[0] ,nRec,OF_BLV0);
  CData_Dstore(m_idTsq,lpBlv[1] ,nRec,OF_BLV1);
  CData_Dstore(m_idTsq,lpBlv[2] ,nRec,OF_BLV2);

  dlp_memmove(lpTokStore,lpsToken,nLen);
  lpTokStore[nLen] = '\0';

  IFCHECKEX(2)
  {
    char sBuf[L_SSTR+1];
    dlp_strconvert(SC_ESCAPE,sBuf,lpTokStore);
    printf("\n      %03ld %d%d%d %-3s |%s|",(long)nLine,(int)lpBlv[0],(int)lpBlv[1],(int)lpBlv[2],
      lpsTtypeP,sBuf);
  }
}

/**
 * Stores the string of insignificant delimiters following the most recently
 * stored token in m_dTsq.
 *
 * @param lpsIdel
 *          The insignificant delimiter string (max. 255 characters)
 * @param nLen
 *          The number of characters to be stored, may be -1 to store the entire
 *          string stored in <code>lpsIdel</code>
 * @param lpsFile
 *          The scanned source file
 * @param nLine
 *          The current line in the scanned source file
 */
void CGEN_PROTECTED CDgen::TsqStoreIdel
(
  const char* lpsIdel,
  INT32        nLen,
  const char* lpsFile,
  INT32        nLine
)
{
  INT32  nTln   = 0;
  char* lpsDst = (char*)CData_XAddr(m_idTsq,CData_GetNRecs(m_idTsq)-1,OF_IDEL);
  if (!lpsIdel) return;
  if (!lpsDst ) return;

  if (nLen<0) nLen = (INT32)dlp_strlen(lpsIdel);
  if (nLen+dlp_strlen(lpsDst)>=L_SSTR)
  {
    IERROR(this,DG_TOOLONG,lpsFile,(long)nLine,"White space string");
    nLen = L_SSTR-(INT32)dlp_strlen(lpsDst)-1;
    if (nLen<=0) return;
  }
  nTln = (INT32)dlp_strlen(lpsDst)+nLen;
  DLPASSERT(nTln<L_SSTR);
  strncat(lpsDst,lpsIdel,nLen);
  lpsDst[nTln]='\0';
}

/**
 * Tokenizes a string (typically one line of source code) and appends the result
 * to the token sequence.
 *
 * @param lpsLine
 *          Pointer to the string to be tokenized
 * @param nLine
 *          Line index in source file (if applicable, else -1)
 * @param lpbInCmt
 *          Multiline comment tracking flag (may be <code>NULL</code>)
 * @param lpbInDcmt
 *          Multiline documentation comment tracking flag (may be
 *          <code>NULL</code>)
 * @param lpanBlv
 *          Bracelevel tracking array (may be <code>NULL</code>)
 * @return <code>O_K</code> if successfull, a (negative) error code otherwise
 * @see Tokenize
 * @see tsq
 */
INT16 CGEN_PUBLIC CDgen::TokenizeString
(
  const char* lpsLine,
  INT32       nLine     DEFAULT(-1),
  BOOL*       lpbInCmt  DEFAULT(NULL),
  BOOL*       lpbInDcmt DEFAULT(NULL),
  INT16*      lpanBlv   DEFAULT(NULL)
)
{
  BOOL  bSeekBcmt      = dlp_strlen(m_lpsBcmton)>0 && dlp_strlen(m_lpsBcmtoff)>0;
  BOOL  bSeekDrct      = dlp_strlen(m_lpsDrct)>0;
  BOOL  bDcmt          = (lpbInDcmt && *lpbInDcmt);
  INT16 nEsc           = 0;
  INT16 nErr           = O_K;
  const char* lpInStr  = NULL;
  const char* lpInChr  = NULL;
  const char* lpInTok  = NULL;
  const char* lpInCmt  = (lpbInCmt && *lpbInCmt)?lpsLine:NULL;;
  const char* lpInIdel = NULL;
  const char* tx       = NULL;
  INT16 anBlv[3]       = {0,0,0};

  IFCHECKEX(3) printf("\n   Tokenizing |%s|",lpsLine);
  if (m_idTsq->GetNComps()==0) TsqInit(m_idTsq);

  // Load brace levels
  if (lpanBlv) dlp_memmove(anBlv,lpanBlv,3*sizeof(INT16));

  // Empty lines
  if (dlp_strlen(lpsLine)==0 || dlp_strcmp(lpsLine,"\n")==0)
  {
    TsqStore("",0,m_lpsFilename,nLine,lpInCmt?(bDcmt?TT_DCMT:TT_BCMT):TT_ELIN,anBlv);
    return O_K;
  }

  // Go character by character
  for (tx=lpsLine;;)
  {
    // Start escape sequences
    if (nEsc==0 && *tx=='\\')
    {
      nEsc=2;
    }
    // Collect insignificant delimiter strings
    if (nEsc!=1 && TsqIsDelimiter(*tx)==DL_INSIGN && lpInIdel==NULL) lpInIdel=tx;
    if (TsqIsDelimiter(*tx)!=DL_INSIGN && lpInIdel!=NULL)
    {
      TsqStoreIdel(lpInIdel,(INT32)(tx-lpInIdel),m_lpsFilename,nLine);
      lpInIdel = NULL;
    }
    // Comments
    if (nEsc!=1 && !lpInStr && !lpInChr && lpInCmt && bSeekBcmt && strstr(tx,m_lpsBcmtoff)==tx)
    {
      tx+=dlp_strlen(m_lpsBcmtoff);
      TsqStore(lpInCmt,(INT32)(tx-lpInCmt),m_lpsFilename,nLine,bDcmt?TT_DCMT:TT_BCMT,anBlv);
      lpInCmt = NULL;
      bDcmt   = FALSE;
    }
    else if
    (
      nEsc!=1 && !lpInStr && !lpInChr && !lpInCmt &&
      (
        strstr(tx,m_lpsLcmt)==tx                  ||
        (bSeekDrct && strstr(tx,m_lpsDrct  )==tx) ||
        (bSeekBcmt && strstr(tx,m_lpsBcmton)==tx)
      )
    )
    {
      TsqStore(lpInTok,(INT32)(tx-lpInTok),m_lpsFilename,nLine,TT_UNK,anBlv);
      bDcmt=(strstr(tx,m_lpsDcmt)==tx);
      if (strstr(tx,m_lpsLcmt)==tx)
      {
        TsqStore(tx,-1,m_lpsFilename,nLine,bDcmt?TT_DCMT:TT_LCMT,anBlv);
        break;
      }
      else if (strstr(tx,m_lpsDrct)==tx)
      {
        TsqStore(tx,-1,m_lpsFilename,nLine,TT_DRCT,anBlv);
        break;
      }
      else
      {
        lpInCmt=tx;
        tx+=dlp_strlen(m_lpsBcmton);
      }
    }
    // String constants "..."
    else if (m_bStrs && nEsc!=1 && !lpInCmt && !lpInChr && *tx=='\"')
    {
      if (!lpInStr)
      {
        TsqStore(lpInTok,(INT32)(tx-lpInTok),m_lpsFilename,nLine,TT_UNK,anBlv);
        lpInTok=NULL;
        lpInStr=tx+1;
      }
      else
      {
        TsqStore(lpInStr,(INT32)(tx-lpInStr),m_lpsFilename,nLine,TT_STR,anBlv);
        lpInStr=NULL;
      }
      bDcmt = FALSE;
      tx++;
    }
    // Character (alt. string) constants '...'
    else if (m_bChrs && nEsc!=1 && !lpInCmt && !lpInStr && *tx=='\'')
    {
      if (!lpInChr)
      {
        lpInChr=tx+1;
        TsqStore(lpInTok,(INT32)(tx-lpInTok),m_lpsFilename,nLine,TT_UNK,anBlv);
        lpInTok=NULL;
      }
      else
      {
        TsqStore(lpInChr,(INT32)(tx-lpInChr),m_lpsFilename,nLine,TT_CHR,anBlv);
        lpInChr=NULL;
      }
      bDcmt = FALSE;
      tx++;
    }
    // All kinds of token delimiters
    else if (nEsc!=1 && !lpInCmt && !lpInStr && !lpInChr && TsqIsDelimiter(*tx))
    {
      bDcmt = FALSE;
      TsqStore(lpInTok,(INT32)(tx-lpInTok),m_lpsFilename,nLine,TT_UNK,anBlv);
      if (TsqIsDelimiter(*tx)==DL_SIGN)
      {
        TsqStore(tx,1,m_lpsFilename,nLine,TT_DEL,anBlv);
        // Count braces
        switch (*tx)
        {
        case '{': anBlv[0]++; break;
        case '(': anBlv[1]++; break;
        case '[': anBlv[2]++; break;
        case '}':
          anBlv[0]--;
          if (anBlv[0]<0)
          {
            if (lpanBlv)
            {
              nErr=DG_SYNTAX2;
              IERRORAT(this,m_lpsFilename,nLine,DG_SYNTAX2,"too may '}'",0,0);
            }
            anBlv[0]=0;
          }
          break;
        case ')':
          anBlv[1]--;
          if (anBlv[1]<0)
          {
            if (lpanBlv)
            {
              nErr=DG_SYNTAX2;
              IERRORAT(this,m_lpsFilename,nLine,DG_SYNTAX2,"too may ')'",0,0);
            }
            anBlv[1]=0;
          }
          break;
        case ']':
          anBlv[2]--;
          if (anBlv[2]<0)
          {
            if (lpanBlv)
            {
              nErr=DG_SYNTAX2;
              IERRORAT(this,m_lpsFilename,nLine,DG_SYNTAX2,"too may ']'",0,0);
            }
            anBlv[2]=0;
          }
          break;
        }
      }
      lpInTok=NULL;
      tx++;
    }
    // Start a new token
    else if
    (
      nEsc!=1 && !TsqIsDelimiter(*tx) &&
      !lpInTok && !lpInStr && !lpInChr && !lpInCmt &&
      *tx && (!m_bStrs || *tx!='\"') && (!m_bChrs || *tx!='\'')
    )
    {
      bDcmt   = FALSE;
      lpInTok = tx++;
    }
    // Ignore, go to next character
    else if (*tx) tx++;
    // End of line
    else
    {
      TsqStore(lpInTok,-1,m_lpsFilename,nLine,TT_UNK               ,anBlv);
      TsqStore(lpInCmt,-1,m_lpsFilename,nLine,bDcmt?TT_DCMT:TT_BCMT,anBlv);
      if (lpInStr)
      {
        IERRORAT(this,m_lpsFilename,nLine,DG_STREXCEEDSLINE,0,0,0);
        nErr = DG_STREXCEEDSLINE;
        TsqStore(lpInStr,-1,m_lpsFilename,nLine,TT_STR,anBlv);
      }
      if (lpInChr)
      {
        IERRORAT(this,m_lpsFilename,nLine,DG_STREXCEEDSLINE,0,0,0);
        nErr = DG_STREXCEEDSLINE;
        TsqStore(lpInChr,-1,m_lpsFilename,nLine,TT_CHR,anBlv);
      }
      break;
    }

    // Decrement escape flag
    if (nEsc>0) nEsc--;
  }

  // Track tokenizer state
  if (lpbInCmt ) *lpbInCmt  = (lpInCmt!=NULL);
  if (lpbInDcmt) *lpbInDcmt = bDcmt;
  if (lpanBlv  ) dlp_memmove(lpanBlv,anBlv,3*sizeof(INT16));

  return nErr;
}

/**
 * Parses the text file sFilename and stores the tokens into the token sequence
 * m_idTsq.
 *
 * @param lpsFilename
 *          The file to parse (may be <code>NULL</code>: in this case the token
 *          sequence will be initialized)
 * @return <code>O_K</code> if successfull, a (negative) error code otherwise
 * @see TokenizeString
 * @see tsq
 */
INT16 CGEN_PUBLIC CDgen::Tokenize(const char* lpsFilename)
{
  INT16 nErr                   = O_K;
  INT16 nErr2                  = O_K;
  INT32  nLine                  = 0;
  INT32  n                      = 0;
  INT32  nGranySave             = 10;
  BOOL  bInCmt                 = FALSE;
  BOOL  bInDcmt                = FALSE;
  char  lpsLine[L_INPUTLINE+1];
  INT16 anBlv[3]               = {0,0,0};
  char* tx                     = NULL;

  // Initialize
  TsqInit(m_idTsq);
  if (dlp_strlen(lpsFilename)==0) return O_K;

  // Remember file name
  dlp_strcpy(m_lpsFilename,lpsFilename);

  // Open source file
  IFCHECKEX(1) printf("\n    Opening source file ...");
  FILE* f = fopen(lpsFilename,"rt");
  if (!f) return IERROR(this,ERR_FILEOPEN,lpsFilename?lpsFilename:"(null)","reading",0);
  nGranySave = m_nGrany; m_nGrany = 10000;
  IFCHECKEX(1) printf(" ok");
  IFCHECKEX(2) printf("\n    This is the token stream:\n      Ln  {([ Tt  Token");

  // Read lines
  while (dlp_fgetl(lpsLine,L_INPUTLINE,f,&n))
  {
    // Count lines and normalize line breaks (to "\n")
    nLine+=n;
    if (dlp_strlen(lpsLine))
      for (tx=&lpsLine[dlp_strlen(lpsLine)-1];*tx;tx--)
      {
        if (*tx=='\n' || *tx=='\r') *tx='\0';
        else break;
      }
    dlp_strcat(lpsLine,"\n");

    // Tokenize line
    nErr2 = TokenizeString(lpsLine,nLine,&bInCmt,&bInDcmt,anBlv);
    IF_OK(nErr) nErr = nErr2;
  }

  // Final checks
  tx = (char*)m_idTsq->XAddr(m_idTsq->GetNRecs()-1,OF_TOK);
  n  = m_idTsq->GetNRecs()-1;
  if (m_idTsq->Dfetch(n,OF_BLV0) > ((dlp_strcmp(tx,"}")==0)?1:0))
  {
    nErr=DG_SYNTAX2;
    IERRORAT(this,m_lpsFilename,nLine,DG_SYNTAX2,"too many '{'",0,0);
  }
  if (m_idTsq->Dfetch(n,OF_BLV1) > ((dlp_strcmp(tx,")")==0)?1:0))
  {
    nErr=DG_SYNTAX2;
    IERRORAT(this,m_lpsFilename,nLine,DG_SYNTAX2,"too many '('",0,0);
  }
  if (m_idTsq->Dfetch(n,OF_BLV2) > ((dlp_strcmp(tx,"]")==0)?1:0))
  {
    nErr=DG_SYNTAX2;
    IERRORAT(this,m_lpsFilename,nLine,DG_SYNTAX2,"too many '['",0,0);
  }

  // Close source file
  IFCHECKEX(2) printf("\n     (EOF)");
  IFCHECKEX(1) printf("\n    Closing source file ...");
  fclose(f);
  IFCHECKEX(1) printf(OK(nErr)?" OK":" PASSED WITH ERRORS");
  m_nGrany = nGranySave;

  // That's it
  return nErr;
}

/**
 * Second, parser type dependent, tokenizing pass. The second pass will modify
 * the token sequence (field {@link tsq}).
 *
 * @param lpsParser
 *          Parser ID
 * @param bFragment
 *          <code>TRUE</code> for a document fragment. This suppresses all
 *          parity errors
 * @return dLabPro error code
 */
INT16 CGEN_PROTECTED CDgen::Tokenize2
(
  const char* lpsParser,
  BOOL        bFragment DEFAULT(FALSE)
)
{
  // Setup for parsers
  if (dlp_strcmp(lpsParser,"dlp")==0 || dlp_strcmp(lpsParser,"uasr")==0)
  {
    DlpTokenize2(bFragment);
    return O_K;
  }
  else if (dlp_strcmp(lpsParser,"perl")==0)
  {
    PerlTokenize2(bFragment);
    return O_K;
  }
  else if (dlp_strcmp(lpsParser,"cpp")==0)
  {
    // No secondary tokenizer
    return O_K;
  }
  else if (dlp_strcmp(lpsParser,"fml")==0)
  {
    return FmlTokenize2(); // bracket check may fail
  }
  else if (dlp_strcmp(lpsParser,"def")==0)
  {
    // No secondary tokenizer
    return O_K;
  }
  else return IERROR(this,DG_PARSER,lpsParser,0,0);
}

// EOF
