// dLabPro class CDgen (DGen)
// - formula scanner
//
// AUTHOR : Robert Schubert
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
 The infix/prefix formula (fml) second tokenizing pass.
 Rejoin of two-character operators, split combined operators
 (<code>++, --, +=, -=, *=, /= </code>).
 */
INT16 CGEN_PROTECTED CDgen::FmlTokenize2()
{
  INT32  nIndex, nIndex2;
  char  nThisDel, nNextDel, nPrevChar, nNextChar;
  char* tx;
  char  sBuf[L_SSTR+1];

  // clean of white space etc.
  nIndex = 0;
  nIndex2 = m_idTsq->GetNRecs()-1;
  TsqTrim(&nIndex, &nIndex2);

  // reconnect double-character operator tokens, split combined operators into
  // multiple simple operators
  for (nIndex = 0; nIndex < m_idTsq->GetNRecs() - 1; nIndex++)
  {
    if (0 == dlp_strcmp(TT_DEL, m_idTsq->Sfetch(nIndex, OF_TTYP)))
    {
      nThisDel  = *m_idTsq->Sfetch(nIndex  ,OF_TOK);
      nNextDel  = *m_idTsq->Sfetch(nIndex+1,OF_TOK);
      nPrevChar = '\0';
      if (dlp_strlen((char*)m_idTsq->XAddr(nIndex-1,OF_IDEL))==0)
      {
        tx = (char*)m_idTsq->XAddr(nIndex-1,OF_TOK);
        nPrevChar = dlp_strlen(tx) ? tx[dlp_strlen(tx)-1] : '\0';
      }
      nNextChar = '\0';
      if (dlp_strlen((char*)m_idTsq->XAddr(nIndex,OF_IDEL))==0)
      {
        tx = (char*)m_idTsq->XAddr(nIndex+1,OF_TOK);
        nNextChar = dlp_strlen(tx) ? *tx : '\0';
      }

      switch(nThisDel)
      {
      case '=':
        if ('=' == nNextDel)
        {
          // glue here
          dlp_strcat((char*) m_idTsq->XAddr(nIndex, OF_TOK), "=");
          m_idTsq->m_bRec = TRUE;
          m_idTsq->Delete(m_idTsq, nIndex+1, 1);
        }
        break;
      case '!':
      case '>':
      case '<':
        if ('=' == nNextDel)
        {
          // glue here
          dlp_strcat((char*) m_idTsq->XAddr(nIndex, OF_TOK), "=");
          m_idTsq->m_bRec = TRUE;
          m_idTsq->Delete(m_idTsq, nIndex+1, 1);
        }
        break;
      case '&':
        if ('&' == nNextDel)
        {
          // glue here
          dlp_strcat((char*) m_idTsq->XAddr(nIndex, OF_TOK), "&");
          m_idTsq->m_bRec = TRUE;
          m_idTsq->Delete(m_idTsq, nIndex+1, 1);
        }
        break;
      case '|':
        if ('|' == nNextDel)
        {
          // glue here
          dlp_strcat((char*) m_idTsq->XAddr(nIndex, OF_TOK), "|");
          m_idTsq->m_bRec = TRUE;
          m_idTsq->Delete(m_idTsq, nIndex+1, 1);
        }
        break;
      case '+':
        if ('+' == nNextDel)
        {
          // xxx++ -> xxx=1+xxx
          m_idTsq->Reallocate(m_idTsq->GetNRecs() + nIndex + 1);                // add records for xstore
          dlp_strcpy((char*) m_idTsq->XAddr(nIndex, OF_TOK), "=");              // change this token to "="
          m_idTsq->m_bRec = TRUE;                                               // duplicate next token ("+")
          m_idTsq->Xstore(m_idTsq, nIndex+1, 1, nIndex+2);
          dlp_strcpy((char*) m_idTsq->XAddr(nIndex+1, OF_TOK), "1");            // change next token to "1"
          dlp_strcpy((char*) m_idTsq->XAddr(nIndex+1, OF_TTYP), TT_UNK);
          m_idTsq->m_bRec = TRUE;
          m_idTsq->Xstore(m_idTsq, 0, nIndex, nIndex+3);                        // add xxx
          m_idTsq->m_bRec = FALSE;
        }
        else if ('=' == nNextDel)
        {
          // xxx+=yyy -> xxx=xxx+yyy
          dlp_strcpy((char*) m_idTsq->XAddr(nIndex, OF_TOK), "=");              // change this token to "="
          dlp_strcpy((char*) m_idTsq->XAddr(nIndex+1, OF_TOK), "+");            // change next token to "+"
          m_idTsq->m_bRec = TRUE;                                               // rotate yyy to front
          m_idTsq->Rotate(m_idTsq, m_idTsq->GetNRecs() - (nIndex+2));           // (this token now at i+N-i-2 instead of i, xxx now at 0+N-i-2)
          m_idTsq->Reallocate(m_idTsq->GetNRecs() + nIndex);                    // add records for xstore (this now at i+N-2i-2, xxx now at 0+N-2i-2)
          m_idTsq->m_bRec = TRUE;                                               // copy next token ("+") to end
          m_idTsq->Xstore(m_idTsq, nIndex+1 + m_idTsq->GetNRecs()-2*nIndex-2, 1,
            m_idTsq->GetNRecs()-1);
          m_idTsq->m_bRec = TRUE;                                               // insert xxx at next token
          m_idTsq->Xstore(m_idTsq, m_idTsq->GetNRecs()-2*nIndex-2, nIndex,
            nIndex+1 + m_idTsq->GetNRecs()-2*nIndex-2);
          m_idTsq->m_bRec = TRUE;                                               // rotate yyy back to rear
          m_idTsq->Rotate(m_idTsq, - (m_idTsq->GetNRecs() - 2*nIndex - 2));
          m_idTsq->m_bRec = FALSE;
        }
        break;
      case '-':
        if ('-' == nNextDel)
        {
          // xxx-- -> xxx=1-xxx
          m_idTsq->Reallocate(m_idTsq->GetNRecs() + nIndex + 1);                // add records for xstore
          dlp_strcpy((char*) m_idTsq->XAddr(nIndex, OF_TOK), "=");              // change this token to "="
          m_idTsq->m_bRec = TRUE;                                               // duplicate next token ("-")
          m_idTsq->Xstore(m_idTsq, nIndex+1, 1, nIndex+2);
          dlp_strcpy((char*) m_idTsq->XAddr(nIndex+1, OF_TOK), "1");            // change next token to "1"
          dlp_strcpy((char*) m_idTsq->XAddr(nIndex+1, OF_TTYP), TT_UNK);
          m_idTsq->m_bRec = TRUE;
          m_idTsq->Xstore(m_idTsq, 0, nIndex, nIndex+3);
          m_idTsq->m_bRec = FALSE;
        }
        else if ('=' == nNextDel)
        {
          // xxx-=yyy -> xxx=xxx-yyy
          dlp_strcpy((char*) m_idTsq->XAddr(nIndex, OF_TOK), "=");              // change this token to "="
          dlp_strcpy((char*) m_idTsq->XAddr(nIndex+1, OF_TOK), "-");            // change next token to "-"
          m_idTsq->m_bRec = TRUE;                                               // rotate yyy to front
          m_idTsq->Rotate(m_idTsq, m_idTsq->GetNRecs() - (nIndex+2));           // (this token now at i+N-i-2 instead of i, xxx now at 0+N-i-2)
          m_idTsq->Reallocate(m_idTsq->GetNRecs() + nIndex);                    // add records for xstore (this now at i+N-2i-2, xxx now at 0+N-2i-2)
          m_idTsq->m_bRec = TRUE;                                               // copy next token ("-") to end
          m_idTsq->Xstore(m_idTsq, nIndex+1 + m_idTsq->GetNRecs()-2*nIndex-2, 1,
            m_idTsq->GetNRecs()-1);
          m_idTsq->m_bRec = TRUE;                                               // insert xxx at next token
          m_idTsq->Xstore(m_idTsq, m_idTsq->GetNRecs()-2*nIndex-2, nIndex,
            nIndex+1 + m_idTsq->GetNRecs()-2*nIndex-2);
          m_idTsq->m_bRec = TRUE;                                               // rotate yyy back to rear
          m_idTsq->Rotate(m_idTsq, - (m_idTsq->GetNRecs() - 2*nIndex - 2));
          m_idTsq->m_bRec = FALSE;
        }
        break;
      case '*':
        if ('.' == nPrevChar)
        {
          // Glue ".*" operator (elementwise matrix product)
          tx = (char*)m_idTsq->XAddr(nIndex-1,OF_TOK);
          tx[dlp_strlen(tx)-1]='\0';
          if (!dlp_strlen(tx))
          {
            CData_DeleteRecs(m_idTsq,nIndex-1,1);
            nIndex--;
          }
          dlp_strcpy((char*)m_idTsq->XAddr(nIndex,OF_TOK),".*");
          if ('.' == nNextChar)
          {
            // Glue ".*." operator (Kronecker product)
            tx = (char*)m_idTsq->XAddr(nIndex+1,OF_TOK);
            dlp_memmove(tx,&tx[1],dlp_strlen(tx));
            if (!dlp_strlen(tx)) CData_DeleteRecs(m_idTsq,nIndex+1,1);
            dlp_strcpy((char*)m_idTsq->XAddr(nIndex,OF_TOK),".*.");
          }
        }
        if ('=' == nNextDel)
        {
          // xxx*=yyy -> xxx=xxx*yyy
          dlp_strcpy((char*) m_idTsq->XAddr(nIndex, OF_TOK), "=");              // change this token to "="
          dlp_strcpy((char*) m_idTsq->XAddr(nIndex+1, OF_TOK),                  // change next token to "*" / ".*"
            '.'==nPrevChar ? ".*" : "*");                                       // |
          m_idTsq->m_bRec = TRUE;                                               // rotate yyy to front
          m_idTsq->Rotate(m_idTsq, m_idTsq->GetNRecs() - (nIndex+2));           // (this token now at i+N-i-2 instead of i, xxx now at 0+N-i-2)
          m_idTsq->Reallocate(m_idTsq->GetNRecs() + nIndex);                    // add records for xstore (this now at i+N-2i-2, xxx now at 0+N-2i-2)
          m_idTsq->m_bRec = TRUE;                                               // copy next token ("*" / ".*") to end
          m_idTsq->Xstore(m_idTsq, nIndex+1 + m_idTsq->GetNRecs()-2*nIndex-2, 1,
            m_idTsq->GetNRecs()-1);
          m_idTsq->m_bRec = TRUE;                                               // insert xxx at next token
          m_idTsq->Xstore(m_idTsq, m_idTsq->GetNRecs()-2*nIndex-2, nIndex,
            nIndex+1 + m_idTsq->GetNRecs()-2*nIndex-2);
          m_idTsq->m_bRec = TRUE;                                               // rotate yyy back to rear
          m_idTsq->Rotate(m_idTsq, - (m_idTsq->GetNRecs() - 2*nIndex - 2));
        }
        break;
      case '/':
        if ('.' == nPrevChar)
        {
          // Glue "./" operator (elementwise matrix division)
          tx = (char*)m_idTsq->XAddr(nIndex-1,OF_TOK);
          tx[dlp_strlen(tx)-1]='\0';
          if (!dlp_strlen(tx))
          {
            CData_DeleteRecs(m_idTsq,nIndex-1,1);
            nIndex--;
          }
          dlp_strcpy((char*)m_idTsq->XAddr(nIndex,OF_TOK),"./");
        }
        if ('=' == nNextDel)
        {
          // xxx/=yyy -> xxx=xxx/yyy
          dlp_strcpy((char*) m_idTsq->XAddr(nIndex, OF_TOK), "=");              // change this token to "="
          dlp_strcpy((char*) m_idTsq->XAddr(nIndex+1, OF_TOK),                  // change next token to "/" / "./"
            '.'==nPrevChar ? "./" : "/");                                       // |
          m_idTsq->m_bRec = TRUE;                                               // rotate yyy to front
          m_idTsq->Rotate(m_idTsq, m_idTsq->GetNRecs() - (nIndex+2));           // (this token now at i+N-i-2 instead of i, xxx now at 0+N-i-2)
          m_idTsq->Reallocate(m_idTsq->GetNRecs() + nIndex);                    // add records for xstore (this now at i+N-2i-2, xxx now at 0+N-2i-2)
          m_idTsq->m_bRec = TRUE;                                               // copy next token ("/") to end
          m_idTsq->Xstore(m_idTsq, nIndex+1 + m_idTsq->GetNRecs()-2*nIndex-2, 1,
            m_idTsq->GetNRecs()-1);
          m_idTsq->m_bRec = TRUE;                                               // insert xxx at next token
          m_idTsq->Xstore(m_idTsq, m_idTsq->GetNRecs()-2*nIndex-2, nIndex,
            nIndex+1 + m_idTsq->GetNRecs()-2*nIndex-2);
          m_idTsq->m_bRec = TRUE;                                               // rotate yyy back to rear
          m_idTsq->Rotate(m_idTsq, - (m_idTsq->GetNRecs() - 2*nIndex - 2));
        }
        break;
      }

      // Reconnect elementwise matrix operators (except ".*[=]" and "./[=]")
      if (nPrevChar=='.')
        if
        (
          dlp_strcmp(m_idTsq->Sfetch(nIndex,OF_TOK),"^"   )==0 ||
          dlp_strcmp(m_idTsq->Sfetch(nIndex,OF_TOK),"pow" )==0 ||
          dlp_strcmp(m_idTsq->Sfetch(nIndex,OF_TOK),"ln"  )==0 ||
          dlp_strcmp(m_idTsq->Sfetch(nIndex,OF_TOK),"exp" )==0 ||
          dlp_strcmp(m_idTsq->Sfetch(nIndex,OF_TOK),"sqrt")==0 ||
          dlp_strcmp(m_idTsq->Sfetch(nIndex,OF_TOK),"||"  )==0 ||
          dlp_strcmp(m_idTsq->Sfetch(nIndex,OF_TOK),"|"   )==0 ||
          dlp_strcmp(m_idTsq->Sfetch(nIndex,OF_TOK),"&&"  )==0 ||
          dlp_strcmp(m_idTsq->Sfetch(nIndex,OF_TOK),"&"   )==0 ||
          dlp_strcmp(m_idTsq->Sfetch(nIndex,OF_TOK),"=="  )==0 ||
          dlp_strcmp(m_idTsq->Sfetch(nIndex,OF_TOK),"!="  )==0 ||
          dlp_strcmp(m_idTsq->Sfetch(nIndex,OF_TOK),"<"   )==0 ||
          dlp_strcmp(m_idTsq->Sfetch(nIndex,OF_TOK),">"   )==0 ||
          dlp_strcmp(m_idTsq->Sfetch(nIndex,OF_TOK),"<="  )==0 ||
          dlp_strcmp(m_idTsq->Sfetch(nIndex,OF_TOK),">="  )==0
        )
        {
          tx = (char*)m_idTsq->XAddr(nIndex-1,OF_TOK);
          tx[dlp_strlen(tx)-1]='\0';
          if (!dlp_strlen(tx))
          {
            CData_DeleteRecs(m_idTsq,nIndex-1,1);
            nIndex--;
          }
          strcpy(sBuf,".");
          dlp_strcat(sBuf,(char*)m_idTsq->XAddr(nIndex,OF_TOK));
          dlp_strcpy((char*)m_idTsq->XAddr(nIndex,OF_TOK),sBuf);
        }
    }
  }

  return O_K;
}

/**
 * The infix/prefix formula (fml) parser (which actually does nothing as for now).
 *
 * @param nFtok First token of m_idTsq to parse
 * @param nLtok Last token of m_idTsq to parse (-1: until the end)
 */
void CGEN_PROTECTED CDgen::FmlParser(INT32 nFtok, INT32 nLtok)
{
  m_idDom->Reset();
}

// EOF
