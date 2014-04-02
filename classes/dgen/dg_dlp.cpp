// dLabPro class CDgen (DGen)
// - UASR script scanner
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
 * The dLabPro (dlp) second tokenizing pass.
 *
 * @param bFragment
 *          <code>TRUE</code> for a document fragment. This suppresses all
 *          parity errors
 */
void CGEN_PROTECTED CDgen::DlpTokenize2(BOOL bFragment)
{
  INT32 nTok2          = 0;
  INT32 nTok           = 0;
  INT32 nLine          = 0;
  INT32 nIfLv          = 0;
  INT32 nBlv0          = 0;
  INT32 nWhl           = 0;
  char sBuf[L_SSTR+1];

  // Rename component OF_BLV1 (it's counting "if" instead of "(")
  m_idTsq->SetCname(OF_BLV1,"if");
  m_idTsq->SetCname(OF_BLV2,"n/a");

  // Glue tokens
  for (nTok=0; nTok<m_idTsq->GetNRecs(); nTok++)
  {
    nLine = __LINE(nTok);

    // Glue formulas (:...: or :...;) - Seek colon tokens
    if (__TTYP_IS(nTok,"d") && __TOK_IS(nTok,":"))
    {
      // Seek the next semicolon token
      for (nTok2=nTok+1; nTok2<m_idTsq->GetNRecs(); nTok2++)
        if (__TTYP_IS(nTok2,"d") && (__TOK_IS(nTok2,";")| __TOK_IS(nTok2,":")))
          break;

      if (nTok2==m_idTsq->GetNRecs())
        IERROR(this,DG_EXPECT,"':' or ';' at end of formula",0,0);

      // Glue formula
      char* lpsFormula = TsqGlue(nTok,nTok2,FALSE);
      if (dlp_strlen(lpsFormula)>255)
        IERROR(this,DG_TOOLONG,m_lpsFilename,(long)nLine,"Formula");

      // Replace token(s)
      dlp_strcpy(sBuf,__IDEL(nTok2));
      m_idTsq->DeleteRecs(nTok+1,nTok2-nTok);
      m_idTsq->Sstore(TT_FORM   ,nTok,OF_TTYP);
      m_idTsq->Sstore(lpsFormula,nTok,OF_TOK );
      m_idTsq->Sstore(sBuf      ,nTok,OF_IDEL);
    }

    // Restore string tokens
    else if (__TTYP_IS(nTok,TT_STR))
    {
      sprintf(sBuf,"\"%s\"",__TOK(nTok));
      m_idTsq->Sstore(sBuf,nTok,OF_TOK);
    }
    else if (__TTYP_IS(nTok,TT_CHR))
    {
      sprintf(sBuf,"\'%s\'",__TOK(nTok));
      m_idTsq->Sstore(sBuf,nTok,OF_TOK);
    }

    // Glue label definitions (label XXX)
    else if (__TTYP_IS(nTok,"?") && __TOK_IS(nTok,"label"))
    {
      if (dlp_strcmp(__TTYP(nTok+1),"?")!=0 || __LINE(nTok+1)!=nLine)
      {
        IERRORAT(this,m_lpsFilename,nLine,DG_EXPECT,"label name",0,0);
      }
      else
      {
        m_idTsq->DeleteRecs(nTok,1);
        m_idTsq->Sstore(TT_LAB,nTok,OF_TTYP);
        m_idTsq->Dstore(nIfLv,nTok,OF_BLV1);
      }
    }

    // While
    else if (__TOK_IS(nTok,"while"))
    {
      for (nTok2=nTok-1; nTok2>=0; nTok2--)
        if
        (
          __TOK_IS(nTok2,";") ||
          __TOK_IS(nTok2,"end") ||
          (__TTYP_IS(nTok2,TT_FORM) && __TOK(nTok2)[dlp_strlen(__TOK(nTok2))-1]==';')
        )
          break;
      nTok2++;
      if (nTok2<0) nTok2=0;
      sprintf(sBuf,"~WHL%ld",(long)nWhl++);
      m_idTsq->InsertRecs(nTok2,1,this->m_nGrany);
      m_idTsq->Sstore(sBuf,nTok2,OF_TOK);
      m_idTsq->Sstore(TT_LAB,nTok2,OF_TTYP);
      m_idTsq->Dstore(nIfLv,nTok2,OF_BLV1);
      nIfLv++;
      nTok++;
    }

    // Count if/else
    else if (__TOK_IS(nTok,"if"))
    {
      nIfLv++;
    }
    else if (__TOK_IS(nTok,"else"))
    {
      if (nIfLv<=0 && !bFragment)
        IERRORAT(this,m_lpsFilename,nLine,DG_AWOB,"else","if",0);
    }
    else if (__TOK_IS(nTok,"{"))
    {
      nBlv0++;
    }

    // Store if level
    m_idTsq->Dstore(nBlv0,nTok,OF_BLV0);
    m_idTsq->Dstore(nIfLv,nTok,OF_BLV1);

    // Backward compatibility: endif -> end
    if (__TOK_IS(nTok,"endif")) dlp_strcpy(__TOK(nTok),"end");

    // Count ends (after storing if level!)
    if (__TOK_IS(nTok,"end"))
    {
      if (nIfLv<=0)
      {
        if (!bFragment)
          IERRORAT(this,m_lpsFilename,nLine,DG_AWOB,"end","if",0)
      }
      else
      {
        // If or While?
        for (nTok2=nTok-1; nTok2>=0; nTok2--)
          if (__BLV(nTok2,1)<nIfLv)
          {
            nTok2++;
            break;
          }
        // While ...
        if (__TOK_IS(nTok2,"while"))
        {
          // Seek while label
          for (;nTok2>=0; nTok2--)
            if (__TTYP_IS(nTok2,TT_LAB))
              break;
          // Create a goto
          if (__TTYP_IS(nTok2,TT_LAB))
          {
            m_idTsq->InsertRecs(nTok,2,this->m_nGrany);
            m_idTsq->Sstore("goto",nTok,OF_TOK);
            m_idTsq->Sstore(TT_UNK,nTok,OF_TTYP);
            m_idTsq->Dstore(nIfLv,nTok,OF_BLV1);
            m_idTsq->Sstore(__TOK(nTok2),nTok+1,OF_TOK);
            m_idTsq->Sstore(TT_UNK,nTok+1,OF_TTYP);
            m_idTsq->Dstore(nIfLv,nTok+1,OF_BLV1);
            nTok+=2;
          }
          else
            IERRORAT(this,m_lpsFilename,nLine,ERR_INTERNAL,
              " (cannot find while label)",__FILE__,__LINE__);
        }
        nIfLv--;
      }
    }
    else if (__TOK_IS(nTok,"}"))
    {
      if (nBlv0<=0)
      {
        if (!bFragment)
          IERRORAT(this,m_lpsFilename,nLine,DG_AWOB,"}","{",0)
      }
      else nBlv0--;
    }
  }

  if (!bFragment)
  {
    if (nIfLv>0) IERRORAT(this,m_lpsFilename,nLine,DG_EXPECT,"end",0,0);
    if (nBlv0>0) IERRORAT(this,m_lpsFilename,nLine,DG_EXPECT,"}"    ,0,0);
  }
}

/**
 * The dLabPro (dlp) parser (which actually does nothing as for now).
 *
 * @param nFtok First token of m_idTsq to parse
 * @param nLtok Last token of m_idTsq to parse (-1: untill the end)
 */
void CGEN_PROTECTED CDgen::DlpParser(CFst* itDeps, INT32 nFtok, INT32 nLtok)
{
  m_idDom->Reset();
}

// EOF
