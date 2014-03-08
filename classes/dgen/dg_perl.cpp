// dLabPro class CDgen (DGen)
// - Perl script scanner
//
// AUTHOR : Frank Duckhorn
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

// Section ID's
#define SEC_NONE   0
#define SEC_ARG    1
#define SEC_DESCR  2
#define SEC_SYNOP  3

// Ordinal property ID's
#define NP_PACKAGE 0
#define NP_NAME    1
#define NP_SNAME   2
#define NP_SYNOP   3
#define NP_AUTHOR  4

/**
 *
 * @param nFtok First token of m_idTsq to parse
 * @param nLtok Last token of m_idTsq to parse (-1: untill the end)
 */
void CGEN_PROTECTED CDgen::PerlParser(INT32 nFtok, INT32 nLtok)
{
  INT32  nVl     = 0;
  INT32  nSec    = SEC_SYNOP;
  INT32  nItm    = 0;
  INT32  nTok    = 0;
  INT32  nTokJ   = 0;
  INT32  nRec    = 0;
  INT32  nNjvd   = 0;                                                            // No. of JavaDoc recs. fo cur. item
  INT32  i       = 0;
  BOOL  bNewArg = FALSE;
  BOOL  bInHead = TRUE;
  char* tx      = NULL;
  const char* lpsVal = NULL;
  char  sBuf[L_INPUTLINE+1];

  // Check token sequence
  DLPASSERT(m_idTsq);
  if (nFtok< 0                      ) nFtok = 0;
  if (nFtok>=CData_GetNRecs(m_idTsq)) nFtok = CData_GetNRecs(m_idTsq)-1;
  if (nLtok< 0                      ) nLtok = CData_GetNRecs(m_idTsq)-1;
  if (nLtok>=CData_GetNRecs(m_idTsq)) nLtok = CData_GetNRecs(m_idTsq)-1;

  // Prepapre DOM instance
  DLPASSERT(m_idDom);
  DLPASSERT(L_NAMES<256);              // Component allocation will fail!
  if
  (
    m_idDom->GetNComps()>=8                                      &&
    dlp_is_numeric_type_code (m_idDom->GetCompType(UDM_OF_DOBT)) &&
    dlp_is_symbolic_type_code(m_idDom->GetCompType(UDM_OF_NAME)) &&
    dlp_is_symbolic_type_code(m_idDom->GetCompType(UDM_OF_TYPE)) &&
    dlp_is_symbolic_type_code(m_idDom->GetCompType(UDM_OF_EXT1)) &&
    dlp_is_numeric_type_code (m_idDom->GetCompType(UDM_OF_FTOK)) &&
    dlp_is_numeric_type_code (m_idDom->GetCompType(UDM_OF_LTOK)) &&
    dlp_is_numeric_type_code (m_idDom->GetCompType(UDM_OF_FJVD)) &&
    dlp_is_numeric_type_code (m_idDom->GetCompType(UDM_OF_NJVD))
  )
  {
    // Preserve user-defined component types
    m_idDom->Clear();
  }
  else
  {
    m_idDom->Reset();
    m_idDom->AddComp("dobt",T_SHORT);    // 0  UDM_OF_DOBT  Type of DOM object
    m_idDom->AddComp("name",L_NAMES);    // 1  UDM_OF_NAME  Identifier
    m_idDom->AddComp("type",L_NAMES);    // 2  UDM_OF_TYPE  Type (class, variable type, etc.)
    m_idDom->AddComp("ext1",L_NAMES);    // 3  UDM_OF_EXT1  Extra 1: - reserved - 
    m_idDom->AddComp("ftok",T_INT  );    // 4  UDM_OF_FTOK  First token index in m_idTsq
    m_idDom->AddComp("ltok",T_INT  );    // 5  UDM_OF_LTOK  Last token index in m_idTsq
    m_idDom->AddComp("fjvd",T_INT  );    // 6  UDM_OF_FJVD  Index of first JavaDoc string in m_idTxt
    m_idDom->AddComp("njvd",T_INT  );    // 7  UDM_OF_NJVD  Number of JavaDoc strings in m_idTxt
  }
  
  // Add DOM root item
  CData_AddRecs(m_idDom,1,20);
  m_idDom->Dstore(-1,0,UDM_OF_DOBT);

  // -- Store package and script names, seek own node in dependency graph --
  m_idSpl->Realloc(10); // Make sure that the pointers we obtain next remain valid
  char* lpPckg = (char*)m_idSpl->XAddr(StoreProperty("PACKAGE" ,"",NP_PACKAGE),1);
  char* lpName = (char*)m_idSpl->XAddr(StoreProperty("NAME"    ,"",NP_NAME   ),1);
  char* lpSnam = (char*)m_idSpl->XAddr(StoreProperty("SNAME"   ,"",NP_SNAME  ),1);
  char* lpSynp = (char*)m_idSpl->XAddr(StoreProperty("SYNOPSIS","",NP_SYNOP  ),1);
  StoreProperty("AUTHOR","n/a",NP_AUTHOR);

  dlp_splitpath(m_lpsFilename,lpPckg,lpName);
  tx = &lpPckg[dlp_strlen(lpPckg)-1];
  while (tx>lpPckg && *tx=='/' && *tx=='\\') *tx--='\0';
  while (tx>lpPckg && *tx!='/' && *tx!='\\') tx--;
  if (*tx=='/' || *tx=='\\') memmove(lpPckg,&tx[1],strlen(tx));

  strcpy(lpSnam,lpName);
  dlp_strreplace(lpSnam,".itp","");
  dlp_strreplace(lpSnam,".xtp","");

  // -- Scan token sequence --
  for (nTok=nFtok; nTok<=nLtok; nTok++)
  {
    INT32  nSl  = (INT32 )CData_Dfetch(m_idTsq,nTok,0);
    char* lpTt = (char*)CData_XAddr (m_idTsq,nTok,1);
    char* lpTk = (char*)CData_XAddr (m_idTsq,nTok,2);

    if (strcmp(lpTt,TT_ELIN)!=0 && strcmp(lpTt,TT_DCMT)!=0 && (nVl>0 || strcmp(lpTt,TT_LCMT)!=0)) bInHead=FALSE;
    if (strcmp(lpTt,TT_DCMT)==0) nVl++;
    
    // Do not scan anything except documentation comments
    if (strcmp(lpTt,TT_ELIN)==0) nSec=SEC_NONE;
    if (bInHead && strcmp(lpTt,TT_DCMT)!=0) continue;

    if (bInHead)
    {
      // -- SCAN THE DOCUMENT HEADER --

      // Remove comment marker and leading blanks (the latter except in descriptions!)
      memmove(lpTk,&lpTk[dlp_strlen(m_lpsDcmt)],dlp_strlen(lpTk)-dlp_strlen(m_lpsDcmt)+1);
      while(*lpTk && nSec!=SEC_DESCR && iswspace(*lpTk)) memmove(lpTk,&lpTk[1],strlen(lpTk));

      // Scan for keys
      if      ((lpsVal=ScanKey(lpTk,"AUTHOR"     ))) { StoreProperty("AUTHOR",lpsVal,NP_AUTHOR); nSec=SEC_NONE; }
      else if ((lpsVal=ScanKey(lpTk,"ARGUMENTS"  ))) { if (!strlen(lpsVal)) nSec = SEC_ARG;   }
      else if ((lpsVal=ScanKey(lpTk,"SYNOPSIS"   ))) { if (!strlen(lpsVal)) nSec = SEC_DESCR; }
      else if ((lpsVal=ScanKey(lpTk,"DESCRIPTION"))) { if (!strlen(lpsVal)) nSec = SEC_DESCR; }
      else if (nSec == SEC_SYNOP && nVl>1)
      {
        if (dlp_strlen(lpSynp)+dlp_strlen(lpTk)+1<256)
        {
          char* tx = lpTk;
          if (dlp_strlen(lpSynp)) dlp_strcat(lpSynp," ");
          else if (lpTk[0]=='-') tx=&lpTk[1];
          dlp_strcat(lpSynp,dlp_strtrimleft(tx));
        }
        else IERROR(this,DG_TOOLONG,lpName,(long)nSl,"Synopsis");
      }
      else if (nSec == SEC_ARG)
      {
        // This line describes an argument
        bNewArg = FALSE;
        for (i=1; i<32; i++)
        {
          sprintf(sBuf,"$%ld",(long)i);
          if ((lpsVal=ScanKey(lpTk,sBuf)))
          {
            // Add new argument to list
            bNewArg=TRUE;
            nItm++;
            nRec = CData_AddRecs(m_idDom,1,20);
            m_idDom->Dstore(UDM_OT_CLARG,nRec,UDM_OF_DOBT);
            m_idDom->Sstore(sBuf        ,nRec,UDM_OF_NAME);

            // Rest: description
            if (dlp_strlen(lpsVal))
            {
              m_idDom->Dstore(TxtStore(lpsVal,&nNjvd),nRec,UDM_OF_NJVD);
              m_idDom->Dstore(nNjvd,nRec,UDM_OF_FJVD);
            }
          }
        }
        if (!bNewArg && nRec>=0)
          if (dlp_strlen(lpTk))
          {
            // Description continued
            nNjvd = (INT32)m_idDom->Dfetch(nRec,UDM_OF_NJVD);
            m_idDom->Dstore(nNjvd+TxtStore(lpTk),nRec,UDM_OF_NJVD);
          }
      }
      else if (nSec == SEC_DESCR)
      {
        // This line belongs to the description
        if (dlp_strlen(lpTk))
        {
          nNjvd = (INT32)m_idDom->Dfetch(0,UDM_OF_NJVD);
          if (nNjvd==0) m_idDom->Dstore(m_idTxt->GetNRecs(),0,UDM_OF_FJVD);
          m_idDom->Dstore(nNjvd+TxtStore(lpTk),0,UDM_OF_NJVD);
        }
      }

      IFCHECKEX(1)
      {
        char sBuf[L_SSTR+1];
        printf("\n    %03ld(%03ld) %-3s ",(long)nSl,(long)nVl,lpTt);
        switch (nSec)
        {
        case SEC_SYNOP: printf("S"); break;
        case SEC_NONE : printf("N"); break;
        case SEC_DESCR: printf("D"); break;
        case SEC_ARG  : if (nItm>0) printf("%hd",(short)nItm); else printf("A"); break;
        default       : printf("?");
        }
        printf(": |%s|",dlp_strconvert(SC_ESCAPE,sBuf,lpTk));
      }
    }
    else
    {
      // -- SCAN THE DOCUMENT BODY --
      if                                                                        //       Function header
      (                                                                         //       |
        __TTYP_IS(nTok,TT_UNK)    &&                                            //       | unknown token
        __BLV(nTok,0)==0          &&                                            //       | on curly brace level 0
        __TOK_IS(nTok,"sub")                                                    //       | named "function"
      )                                                                         //       |
      {                                                                         //       >>
        strcpy(sBuf,__TOK(nTok+1));
        nItm = -1;

        if (dlp_strlen(sBuf))
        {
          IFCHECKEX(1) printf("\n    %03ld function %s",__LINE(nTok),sBuf);
          nRec = CData_AddRecs(m_idDom,1,20);                                   //           Add function DOM item
          m_idDom->Dstore(UDM_OT_FHEAD,nRec,UDM_OF_DOBT);
          m_idDom->Sstore(sBuf        ,nRec,UDM_OF_NAME);
          m_idDom->Dstore(nTok        ,nRec,UDM_OF_FTOK);
          m_idDom->Dstore(nTok+1      ,nRec,UDM_OF_LTOK);
          nItm = nRec;
          nRec = CData_AddRecs(m_idDom,1,20);                                   //           Add return value DOM item
          m_idDom->Dstore(UDM_OT_FRETV,nRec,UDM_OF_DOBT);
          m_idDom->Dstore(-1          ,nRec,UDM_OF_FTOK);
          m_idDom->Dstore(-1          ,nRec,UDM_OF_LTOK);
        }
        else
        {
          IERRORAT(this,m_lpsFilename,__LINE(nTok),DG_SYNTAX2,
            "expect function identifier",0,0);
          continue;
        }
        
        // Scan JavaDoc                                                         //         - - - - - - - - - - - - - -
        for (nTokJ=nTok-1; __TTYP_IS(nTokJ,TT_DCMT) && nTokJ>0; nTokJ--) {};    //         Seek beginning of JavaDoc
        nTokJ++;                                                                //         Actually at next token
        if (nTokJ<nTok) UasrJavaDoc(nItm,nTokJ,nTok-1,FALSE);                   //         Have JavaDoc tokens
      }                                                                         //       <<

    }
  }
}

/**
 * The perl second tokenizing pass.
 * 
 * @param bFragment
 *          <code>TRUE</code> for a document fragment. This suppresses all
 *          parity errors
 */
void CGEN_PROTECTED CDgen::PerlTokenize2(BOOL bFragment)
{
  INT32 nTok2          = 0;
  INT32 nTok           = 0;
  INT32 nLine          = 0;
  INT32 nIfLv          = 0;
  INT32 nBlv0          = 0;
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
        IERROR(this,DG_TOOLONG,m_lpsFilename,nLine,"Formula");
      
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
    
    else if (__TOK_IS(nTok,"{"))
    {
      nBlv0++;
    }

    // Store if level
    m_idTsq->Dstore(nBlv0,nTok,OF_BLV0);
    m_idTsq->Dstore(nIfLv,nTok,OF_BLV1);

    // Count endifs (after storing if level!)
    if (__TOK_IS(nTok,"}"))
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
    if (nBlv0>0) IERRORAT(this,m_lpsFilename,nLine,DG_EXPECT,"}"    ,0,0);
  }
}

// EOF
