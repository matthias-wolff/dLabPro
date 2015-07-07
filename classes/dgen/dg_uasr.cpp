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
 * Parse JavaDoc for one DOM item.
 * 
 * @param nDomItm
 *          DOM item index in {@link dom m_idDom}
 * @param nFtok
 *          First JavaDoc token in {@link tsq m_idTsq}
 * @param nLtok
 *          Last JavaDoc token in {@link tsq m_idTsq}
 * @param nParamExists
 *          TRUE if argument list entries are created, otherwise FALSE (=> create them)
 */
void CGEN_PROTECTED CDgen::UasrJavaDoc(INT32 nDomItm, INT32 nFtok, INT32 nLtok, bool nParamExists)
{
  INT32  nTok    = 0;                                                            // Current token in m_idTsq
  INT32  nItm    = -1;                                                           // Current DOM item in m_idDom
  INT32  nTxt    = -1;                                                           // Current record in m_idTxt
  char* tx      = NULL;                                                         // Universal character pointer
  char* lpsName = NULL;                                                         // Parameter name
  char* lpsType = NULL;                                                         // Parameter type
  char* lpsText = NULL;                                                         // HTML text
  char  sTok[L_SSTR+1];                                                         // Copy of current token

  IFCHECKEX(1)                                                                  // Protocol (verbose level 1)
    printf("\n    %03ld JavaDoc",(long)__LINE(nFtok),                           // |
      (char*)m_idDom->XAddr(nDomItm,UDM_OF_NAME));                              // |

  for (nItm=nDomItm,nTok=nFtok; nTok<=nLtok; nTok++)                            // Loop over JavaDoc tokens
  {                                                                             // >>
    DLPASSERT(__TTYP_IS(nTok,TT_DCMT));                                         //   Gimme the right tokens, yu ... !
    strcpy(sTok,__TOK(nTok));                                                   //   Copy token
    while (dlp_strlen(sTok) && sTok[0]=='#')                                    //   Trim leading '#'
      dlp_memmove(sTok,&sTok[1],dlp_strlen(sTok));                              //     ...
    if (!dlp_strlen(sTok)) continue;                                            //   Empty line
    for (tx=sTok; *tx && iswspace(*tx); tx++) {}                                //   Skip over heading white spaces
    if (dlp_strncmp(tx,"@param",6)==0 && iswspace(tx[6]))                       //   @param tag
    {                                                                           //   >>
      tx+=6;                                                                    //     Skip tag
      lpsName = strtok(tx  ," \t\r\n");                                         //     Get parameter name
      lpsType = strtok(NULL," \t\r\n");                                         //     Get parameter type (optional)
      lpsText = strtok(NULL," \t\r\n");                                         //     Get HTML text
      IFCHECKEX(2)                                                              //     Protocol (verbose level 2)
      {                                                                         //     >>
        printf("\n    %03ld - @param %s",(long)__LINE(nTok),SCSTR(lpsName));    //       Parameter line and name
        if (dlp_strlen(lpsType)) printf(", type %s",SCSTR(lpsType));            //       Parameter type (if any)
      }                                                                         //     <<
      if(nParamExists)                                                          //     Parameter entry exists => find
      {                                                                         //     >>
        for (nItm=nDomItm; nItm<m_idDom->GetNRecs(); nItm++)                    //       Seek DOM item of parameter
          if                                                                    //         DOM item must
          (                                                                     //         |
            (INT16)m_idDom->Dfetch(nItm,UDM_OF_DOBT) == UDM_OT_FFARG       &&   //         | be a function parameter ...
            dlp_strcmp((char*)m_idDom->XAddr(nItm,UDM_OF_NAME),lpsName)==0      //         | named lpsName
          )                                                                     //         |
          {                                                                     //         >>
            break;                                                              //           Gotcha!
          }                                                                     //         <<
        if (nItm>=m_idDom->GetNRecs())
        {
          IERROR(this,DG_JVD,"@param",lpsName,"does not exist. Ignored");
          nItm = -1;
          continue;
        }
      }                                                                         //     <<
      else                                                                      //     Else create parameter entry
      {                                                                         //     >>
        nItm = CData_AddRecs(m_idDom,1,20);
        m_idDom->Dstore(UDM_OT_FFARG,nItm,UDM_OF_DOBT);
        m_idDom->Sstore(lpsName     ,nItm,UDM_OF_NAME);
        m_idDom->Dstore(nTok+1      ,nItm,UDM_OF_FTOK);
        m_idDom->Dstore(nTok+1      ,nItm,UDM_OF_LTOK);
      }                                                                         //     <<
      m_idDom->Sstore(lpsType,nItm,UDM_OF_TYPE);
      if (dlp_strlen(lpsText))
      {
        m_idDom->Dstore(TxtStore(lpsText,&nTxt),nItm,UDM_OF_NJVD);
        m_idDom->Dstore(nTxt,nItm,UDM_OF_FJVD);
      }
    }                                                                           //   <<
    else if (dlp_strncmp(tx,"@return",7)==0 && iswspace(tx[7]))                 //   @return tag
    {                                                                           //   >>
      tx+=7;
      IFCHECKEX(2) printf("\n    %03ld - @return",(long)__LINE(nTok));
      nItm = nDomItm+1;
      DLPASSERT((INT16)m_idDom->Dfetch(nItm,UDM_OF_DOBT)==UDM_OT_FRETV);
      if (dlp_strlen(tx))
      {
        m_idDom->Dstore(TxtStore(tx,&nTxt),nItm,UDM_OF_NJVD);
        m_idDom->Dstore(nTxt,nItm,UDM_OF_FJVD);
      }
    }                                                                           //   <<
    else if (dlp_strncmp(tx,"@global",7)==0 && iswspace(tx[7]))                 //   @global tag
    {                                                                           //   >>
      INT32 nNewItm = -1;
      tx+=7;                                                                    //     Skip tag
      lpsName = strtok(tx  ," \t\r\n");                                         //     Get global variable name
      lpsText = strtok(NULL," \t\r\n");                                         //     Get access code
      nNewItm = m_idDom->AddRecs(1,100);
      m_idDom->Dstore(UDM_OT_FGLOB,nNewItm,UDM_OF_DOBT);
      m_idDom->Sstore(lpsName     ,nNewItm,UDM_OF_NAME);
      m_idDom->Sstore(lpsText     ,nNewItm,UDM_OF_TYPE);
      m_idDom->Dstore(-1          ,nNewItm,UDM_OF_FTOK);
      m_idDom->Dstore(-1          ,nNewItm,UDM_OF_LTOK);
    }                                                                           //   <<
    else if (dlp_strncmp(tx,"@see",4)==0 && iswspace(tx[4]))                    //   @see tag
    {                                                                           //   >>
      INT32 nNewItm = -1;                                                       //     DOM node id
      tx+=4;                                                                    //     Skip tag
      lpsName = strtok(tx  ," \t\r\n");                                         //     Get label
      lpsText = strtok(NULL," \t\r\n");                                         //     Get href
      nNewItm = m_idDom->AddRecs(1,100);
      m_idDom->Dstore(UDM_OT_FSEE,nNewItm,UDM_OF_DOBT);
      m_idDom->Sstore(lpsName    ,nNewItm,UDM_OF_NAME);
      m_idDom->Sstore(lpsText    ,nNewItm,UDM_OF_TYPE);
      m_idDom->Dstore(-1         ,nNewItm,UDM_OF_FTOK);
      m_idDom->Dstore(-1         ,nNewItm,UDM_OF_LTOK);
    }                                                                           //   <<
    else if (nItm>=0 && dlp_strlen(tx))
    {
      nTxt = (INT32)m_idDom->Dfetch(nItm,UDM_OF_NJVD);
      if (nTxt==0) m_idDom->Dstore(m_idTxt->GetNRecs(),nItm,UDM_OF_FJVD);
      m_idDom->Dstore(nTxt+TxtStore(sTok),nItm,UDM_OF_NJVD);
    }
  }                                                                             // <<
}

/**
 *
 * @param nFtok First token of m_idTsq to parse
 * @param nLtok Last token of m_idTsq to parse (-1: untill the end)
 */
void CGEN_PROTECTED CDgen::UasrParser(CFst* itDeps, INT32 nFtok, INT32 nLtok)
{
  INT32  nVl     = 0;
  INT32  nSec    = SEC_SYNOP;
  INT32  nItm    = 0;
  INT32  nTok    = 0;
  INT32  nTokJS  = -1;
  INT32  nTokJE  = -1;
  INT32  nRec    = 0;
//  INT32  nOwnN   = -1;                                                          // - for call dependency graph -
  INT32  nNjvd   = 0;                                                           // No. of JavaDoc recs. fo cur. item
  INT32  i       = 0;
  BOOL  bNewArg = FALSE;
  BOOL  bInHead = TRUE;
  char* tx      = NULL;
  char* ty      = NULL;
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

  if (itDeps)
  {
    if (itDeps->sd->GetNComps()<IC_SD_DATA+2)
    {
      IERROR(this,DG_TOOFEWCOMPS,itDeps->m_lpInstanceName,0,0);
      itDeps=NULL;
    }
    else if (!dlp_is_symbolic_type_code(itDeps->sd->GetCompType(IC_SD_DATA+1)))
    {
      IERROR(this,DG_NOTSYMBCOMPTYPE,IC_SD_DATA+1,itDeps->m_lpInstanceName,0);
      itDeps=NULL;
    }
    else
    {
      for (INT32 i=1; i<itDeps->sd->GetNRecs(); i++)
        if (strcmp((char*)itDeps->sd->XAddr(i,IC_SD_DATA+1),lpName)==0)
        { /*nOwnN = i;*/ break; }
    }
  }
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
        __TOK_IS(nTok,"function")                                               //       | named "function"
      )                                                                         //       |
      {                                                                         //       >>
        strcpy(sBuf,__TOK(nTok+1));
        nItm = -1;

        // Check closing bracket
        if (sBuf[dlp_strlen(sBuf)-1]!=')')
        {
          IERRORAT(this,m_lpsFilename,__LINE(nTok),DG_SYNTAX2,"expect \")\"",0,
            0);
          continue;
        }
        sBuf[dlp_strlen(sBuf)-1]='\0';
        
        // Isolate function identifier
        for (tx=sBuf; *tx && *tx!='('; tx++)
        if (!*tx)
        {
          IERRORAT(this,m_lpsFilename,__LINE(nTok),DG_SYNTAX2,
            "expect \"(\" after \"function\"",0,0);
          continue;
        }
        *tx++='\0';
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
        
        // Parse formal argument list
        ty = strtok(tx,",");
        while (ty)
        {
          if (dlp_strlen(ty))
          {
            IFCHECKEX(2) printf("\n        - Argument %s",ty);
            nRec = CData_AddRecs(m_idDom,1,20);
            m_idDom->Dstore(UDM_OT_FFARG,nRec,UDM_OF_DOBT);
            m_idDom->Sstore(ty          ,nRec,UDM_OF_NAME);
            m_idDom->Dstore(nTok+1      ,nRec,UDM_OF_FTOK);
            m_idDom->Dstore(nTok+1      ,nRec,UDM_OF_LTOK);
          }
          else
          {
            IERRORAT(this,m_lpsFilename,__LINE(nTok),DG_SYNTAX2,
              "expect argument identifier after \",\"",0,0);
          }
          ty = strtok(NULL,",");
        }
        
        // Scan JavaDoc                                                         //         - - - - - - - - - - - - - -
        nTokJS = nTok - 1; nTokJE = nTok - 1;                                   //         Docu. comment start and end
        if (__TTYP_IS(nTokJE,TT_UNK) && __TOK_IS(nTokJE,"/inline"))             //         /inline preceeds function >>
        {                                                                       //         >>
          nTokJS--; nTokJE--;                                                   //           Skip it
        }                                                                       //         <<
        for (; __TTYP_IS(nTokJS,TT_DCMT) && nTokJS>=0; nTokJS--) {};            //         Seek beginning of JavaDoc
        nTokJS++;                                                               //         Actually at next token
        if (nTokJS<=nTokJE) UasrJavaDoc(nItm,nTokJS,nTokJE,TRUE);               //         Do JavaDoc
      }                                                                         //       <<

/*
      if (itDeps || itDeps->sd->GetNRecs()) for (nRec=0; nRec<itDeps->sd->GetNRecs(); nRec++)
      {
        char lpDep1[128]; sprintf(lpDep1,"%s"      ,(char*)itDeps->sd->XAddr(nRec,IC_SD_DATA+1));
        char lpDep2[128]; sprintf(lpDep2,"%s"      ,(char*)itDeps->sd->XAddr(nRec,IC_SD_DATA+1));
        char lpDep3[128]; sprintf(lpDep3,"../%s/%s",(char*)itDeps->sd->XAddr(nRec,IC_SD_DATA+0),(char*)itDeps->sd->XAddr(nRec,IC_SD_DATA+1));
        char lpDep4[128]; sprintf(lpDep4,"../%s/%s",(char*)itDeps->sd->XAddr(nRec,IC_SD_DATA+0),(char*)itDeps->sd->XAddr(nRec,IC_SD_DATA+1));

        dlp_strreplace(lpDep2,".itp",""); dlp_strreplace(lpDep2,".xtp",""); 
        dlp_strreplace(lpDep4,".itp",""); dlp_strreplace(lpDep4,".xtp","");

        if
        (
          strcmp(lpTk,lpDep1)==0 ||
          (strcmp(lpTk,lpDep2)==0 && strcmp(lpTt,"s")!=0) ||
          strcmp(lpTk,lpDep3)==0 ||
          strcmp(lpTk,lpDep4)==0
        )
        {
          INT32 nT     = 0;
          BOOL bFound = FALSE;
          for (nT=UD_FT(itDeps,0); nT<UD_FT(itDeps,0)+UD_XT(itDeps,0); nT++)
            if (TD_INI(itDeps,nT)==nOwnN && TD_TER(itDeps,nT)==nRec)
            {
              bFound = TRUE;
              break;
            }
            
          if (!bFound) itDeps->Addtrans(0,nOwnN,nRec);
          IFCHECKEX(1) printf("\n    %03ld(%03ld) %-3s L: Dependency %3ld -->%3ld |%s|",
                              (long)nSl,(long)nVl,lpTt,(long)nOwnN,(long)nRec,lpTk);
        }
      }
    */
    }
  }
}

// EOF
