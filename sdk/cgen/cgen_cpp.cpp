// dLabPro SDK class CCgen (cgen)
// - Scanning and generating C/C++ code
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

// -- C/C++ source scanner --

/**
 * Create function declarator.
 *
 * @param nType   Function type, one opf the CSIG_XXX constants
 * @param lpsDspc Function declaration specifiers
 * @param lpsName Fully qualified function identifier
 * @param lpsSnam Short function identifier (w/o class identifier)
 * @param lpsMdfr Function declaration modifiers
 * @param lpsPvrt "=0" for declaration of pure virtual functions, "" otherwise
 * @param idDom   DOM
 * @param nFdobA  First argument in DOM (
 * @param lpsDst  Pointer to destination buffer
 * @param nLen    Maximal number of characters to write to destination buffer
 */
void CGEN_PROTECTED CCgen::CppMakeDeclarator
(
  INT16        nType,
  const char*  lpsDspc,
  const char*  lpsName,
  const char*  lpsSnam,
  const char*  lpsMdfr,
  const char*  lpsPvrt,
  CData*       idDom,
  INT32         nFdobA,
  char*        lpsDst,
  INT16        nLen
)
{
  // Local variables
  char lpsBuf[100] = "";
  INT32 nDob        = 0;
  INT32 nArgs       = 0;

  // Initialize
  dlp_memset(lpsDst,0,nLen);

  // Count formal arguments
  for (nDob=nFdobA,nArgs=0; nDob<idDom->GetNRecs(); nDob++)
  {
    if ((INT16)idDom->Dfetch(nDob,CDM_OF_DOBT)!=CDM_OT_FFARG) break;
    nArgs++;
  }

  // Decl. specs, identifier, brace on and this pointer
  if      (nType==CSIG_CXXMEMBER      ) sprintf(lpsDst,"\t%s %s %s(",lpsMdfr,lpsDspc   ,lpsSnam);
  else if (nType==CSIG_CXXWRAPPER_HEAD) sprintf(lpsDst,"%s %s::%s(" ,lpsDspc,m_lpsCName,lpsSnam);
  else if (nType==CSIG_CXXWRAPPER_CALL)
  {
    if (strstr(lpsMdfr,"static")==0)
    {
      if (dlp_strcmp(lpsDspc,"void")==0) sprintf(lpsDst,"\t%s_%s(this"       ,m_lpsCName,lpsSnam);
      else                               sprintf(lpsDst,"\treturn %s_%s(this",m_lpsCName,lpsSnam);

      if (nArgs>0) strcat(lpsDst,", ");
    }
    else
    {
      if (dlp_strcmp(lpsDspc,"void")==0) sprintf(lpsDst,"\t%s_%s("       ,m_lpsCName,lpsSnam);
      else                               sprintf(lpsDst,"\treturn %s_%s(",m_lpsCName,lpsSnam);
    }
  }
  else if (nType==CSIG_CMEMBER)
  {
    if (strstr(lpsMdfr,"static")) sprintf(lpsDst,"%s %s(",lpsDspc,lpsName);
    else
    {
      sprintf(lpsDst,"%s %s(%s*",lpsDspc,lpsName,m_lpsCName);
      if (nArgs>0) strcat(lpsDst,", ");
    }
  }
  else
    sprintf(lpsDst,"%s %s(",lpsDspc,lpsName);

  // Remaining arguments
  for (nDob=nFdobA; nDob<nFdobA+nArgs; nDob++)
  {
    if (nType==CSIG_CXXWRAPPER_CALL)
    {
      sprintf(lpsBuf,"%s",(char*)idDom->XAddr(nDob,CDM_OF_NAME));
      strcat(lpsDst,lpsBuf);
    }
    else
    {
      if (dlp_strcmp((char*)idDom->XAddr(nDob,CDM_OF_NAME),"_this")==0)
        // Do not include identifier '_this' in declaration
        sprintf(lpsBuf,"%s",(char*)idDom->XAddr(nDob,CDM_OF_DSPC));
      else
        sprintf
        (
          lpsBuf,"%s %s%s",
          (char*)idDom->XAddr(nDob,CDM_OF_DSPC),
          (char*)idDom->XAddr(nDob,CDM_OF_NAME),
          (char*)idDom->XAddr(nDob,CDM_OF_EXT1)
        );
      strcat(lpsDst,lpsBuf);

      if (nType==CSIG_CXXMEMBER && dlp_strlen((char*)idDom->XAddr(nDob,CDM_OF_EXT2)))
      {
        sprintf(lpsBuf," = %s",(char*)idDom->XAddr(nDob,CDM_OF_EXT2));
        strcat(lpsDst,lpsBuf);
      }
    }

    if (nDob<nFdobA+nArgs-1) strcat(lpsDst,", ");
  }

  // Brace off, pure virtual specifier and semi-colon
  if      (nType==CSIG_CXXMEMBER && dlp_strlen(lpsPvrt)) sprintf(lpsBuf,")%s;",lpsPvrt);
  else if (nType==CSIG_CXXWRAPPER_HEAD                 ) sprintf(lpsBuf,")"           );
  else                                                   sprintf(lpsBuf,");"          );
  strcat(lpsDst,lpsBuf);
}

/**
 * Get signature of C/C++ functions from document object model and register in
 * code lists. This method is called by ScanPrototypes for every function
 * header entry in the docoument object inside iDG.
 *
 * @param        iDG DGen instance (document generator)
 * @param nFdobF Index of function header in document object model (iDG->m_idDom)
 * @return O_K if successfull, a (negative) error code otherwise
 */
INT16 CGEN_PROTECTED CCgen::CppFhead(CDgen* iDG, INT32 nFdobF)
{
  // Local variables
  BOOL    bDeclare         = TRUE;  // TRUE if function shall be declared and wrapper code shall be generated
  INT32    i                = 0;
  INT32    nFtokD           = 0;     // First JavaDoc token of function in iDG->m_idTsq
  INT32    nLtokD           = 0;     // Last JavaDoc token of function in iDG->m_idTsq
  INT32    nFdobA           = -1;    // First formal argument in DOM
  INT16   nFtyp            = 0;     // Function type (one of the CSIG_XXX constants)
  char*   lpsDspc          = NULL;  // Function declaration specifiers
  char*   lpsName          = NULL;  // Fully qualified function identifier
  SCGMth* lpMth            = NULL;
  char    lpsSnam[L_NAMES] = "";    // Short function identifier (w/o class identifier)
  char    lpsMdfr[L_NAMES] = "";    // Function declaration modifiers
  char    lpsPvrt[L_NAMES] = "";    // "=0" for declaration of pure virtual functions
  char    lpsKey [L_NAMES] = "";    // "CName::"
  char    lpsKey2[L_NAMES] = "";    // "CName_"
  char    lpsKey3[L_NAMES] = "";    // "CName*"
  char    lpsKey4[L_NAMES] = "";    // "name*"
  char    lpsDeclarator[512];
  CList<SCGStr> lJavaDoc;           // Copy of the function's JavaDoc

  // Initialize
  DLPASSERT(iDG);
  DLPASSERT(iDG->m_idDom);
  if (nFdobF<0) nFdobF=0;
  if (nFdobF>=iDG->m_idDom->GetNRecs()) return NOT_EXEC;
  if ((INT16)iDG->m_idDom->Dfetch(nFdobF,CDM_OF_DOBT)!=CDM_OT_FHEAD) return NOT_EXEC;

  lpsName = (char*)iDG->m_idDom->XAddr(nFdobF,CDM_OF_NAME);
  lpsDspc = (char*)iDG->m_idDom->XAddr(nFdobF,CDM_OF_DSPC);

  if (dlp_strlen(m_lpsCName) && dlp_strlen(m_lpsClass))
  {
    sprintf(lpsKey ,"%s::",m_lpsCName);
    sprintf(lpsKey2,"%s_" ,m_lpsCName);
    sprintf(lpsKey3,"%s*" ,m_lpsCName);
    sprintf(lpsKey4,"%s*" ,m_lpsClass);
  }
  if (ReplaceKey(lpsDspc,S_CGENIGNORE,"",0)) return O_K;

  // Decl-specs. --> declaration modfiers
  if      (ReplaceKey(lpsDspc,S_PROT      ,"",0)) { strcpy(lpsMdfr,"public:"           ); strcpy(lpsPvrt,""    ); }
  else if (ReplaceKey(lpsDspc,S_CGENPUB   ,"",0)) { strcpy(lpsMdfr,"public:"           ); strcpy(lpsPvrt,""    ); }
  else if (ReplaceKey(lpsDspc,S_CGENVPUB  ,"",0)) { strcpy(lpsMdfr,"public: virtual"   ); strcpy(lpsPvrt,""    ); }
  else if (ReplaceKey(lpsDspc,S_CGENVPPUB ,"",0)) { strcpy(lpsMdfr,"public: virtual"   ); strcpy(lpsPvrt," = 0"); }
  else if (ReplaceKey(lpsDspc,S_CGENSPUB  ,"",0)) { strcpy(lpsMdfr,"public: static"    ); strcpy(lpsPvrt,""    ); }
  else if (ReplaceKey(lpsDspc,S_CGENPROT  ,"",0)) { strcpy(lpsMdfr,"protected:"        ); strcpy(lpsPvrt,""    ); }
  else if (ReplaceKey(lpsDspc,S_CGENVPROT ,"",0)) { strcpy(lpsMdfr,"protected: virtual"); strcpy(lpsPvrt,""    ); }
  else if (ReplaceKey(lpsDspc,S_CGENVPPROT,"",0)) { strcpy(lpsMdfr,"protected: virtual"); strcpy(lpsPvrt," = 0"); }
  else if (ReplaceKey(lpsDspc,S_CGENSPROT ,"",0)) { strcpy(lpsMdfr,"protected: static" ); strcpy(lpsPvrt,""    ); }
  else if (ReplaceKey(lpsDspc,S_CGENPRIV  ,"",0)) { strcpy(lpsMdfr,"private:"          ); strcpy(lpsPvrt,""    ); }
  else if (ReplaceKey(lpsDspc,S_CGENSPRIV ,"",0)) { strcpy(lpsMdfr,"private: static"   ); strcpy(lpsPvrt,""    ); }
  else if (ReplaceKey(lpsDspc,S_CGENEXPORT,"",0)) { strcpy(lpsMdfr,"public:"           ); strcpy(lpsPvrt,""    ); }
  else    { bDeclare=FALSE; /* no CGEN_XXX macro*/  strcpy(lpsMdfr,"public:"           ); strcpy(lpsPvrt,""    ); }
  if (lpsDspc[dlp_strlen(lpsDspc)-1]==' ') lpsDspc[dlp_strlen(lpsDspc)-1]='\0';

  // Get short name (w/o class names)
  dlp_strncpy(lpsSnam,(char*)iDG->m_idDom->XAddr(nFdobF,CDM_OF_NAME),L_NAMES);
  if      (ReplaceKey(lpsSnam,lpsKey ,"",0)) nFtyp = CSIG_CXXMEMBER;
  else if (ReplaceKey(lpsSnam,lpsKey2,"",0)) nFtyp = CSIG_CMEMBER;
  else                                       nFtyp = CSIG_FUNCTION;

  // Handle formal argument list
  nFdobA = nFdobF+1;
  if
  (
    nFdobA<iDG->m_idDom->GetNRecs()                                 && // There is another DOM object
    (INT16)iDG->m_idDom->Dfetch(nFdobA,CDM_OF_DOBT)==CDM_OT_FFARG    // It is an argument to the current function
  )
  {
    // Check first argument of C members (may be the _this pointer)
    if (nFtyp==CSIG_CMEMBER)
    {
      if
      (
        dlp_strlen((char*)iDG->m_idDom->XAddr(nFdobA,CDM_OF_EXT1))==0           && // No array spec.
        dlp_strlen((char*)iDG->m_idDom->XAddr(nFdobA,CDM_OF_EXT2))==0           && // No default value
        (
          dlp_strcmp((char*)iDG->m_idDom->XAddr(nFdobA,CDM_OF_DSPC),lpsKey3)==0 || // Type is "CName*" -or-
          dlp_strcmp((char*)iDG->m_idDom->XAddr(nFdobA,CDM_OF_DSPC),lpsKey4)==0    // Type is "name*"
        )                                                                       &&
        dlp_strcmp((char*)iDG->m_idDom->XAddr(nFdobA,CDM_OF_NAME),"_this")==0      // Name is "_this"
      )
      {
        // Non-static C member: skip this argument and make sure m_bCProject is set
        nFdobA++;
        if (!m_bCProject)
        {
          INT32 nLine = (INT32)iDG->m_idTsq->Dfetch((INT32)iDG->m_idDom->Dfetch(nFdobF,CDM_OF_FTOK),OF_LINE);
          ERRORMSG(ERR_INFUNCTION,lpsName,iDG->m_lpsFilename,nLine);
          ERRORMSG(ERR_CPROJECT,0,0,0);
        }
        m_bCProject = TRUE;
      }
      else
      {
        // Static C member: modify modifiers :))
        ReplaceKey(lpsMdfr," virtual","",0);
        ReplaceKey(lpsMdfr,"virtual" ,"",0);
        if (!strstr(lpsMdfr,"static")) strcat(lpsMdfr," static");
      }
    }
  }

  // Verbose level 4 protocol
  if (GetVerboseLevel()>=4)
  {
    printf("\n\nFUNCTION HEADER\n");
    iDG->TsqPrint((INT32)iDG->m_idDom->Dfetch(nFdobF,CDM_OF_FTOK),(INT32)iDG->m_idDom->Dfetch(nFdobF,CDM_OF_LTOK));

    printf("\n  Function                : ");
    switch (nFtyp)
    {
    case CSIG_CXXMEMBER: printf("C++ Member"); break;
    case CSIG_CMEMBER  : printf("C Member"  ); break;
    case CSIG_FUNCTION : printf("Function"  ); break;
    default: DLPASSERT(FALSE);
    }
    CppMakeDeclarator(nFtyp,lpsDspc,lpsName,lpsSnam,lpsMdfr,lpsPvrt,iDG->m_idDom,nFdobA,lpsDeclarator,512);
    printf("\n  Declarator              : '%s'",lpsDeclarator );
    printf("\n  Modifiers               : '%s'",lpsMdfr);
    printf("\n  Decl.specs.             : '%s'",lpsDspc);
    printf("\n  Identifier              : '%s' (fully qualified: '%s')",lpsSnam,lpsName);
    if (nFtyp==CSIG_CMEMBER) printf("\n  ( 1) - Type             : this pointer");

    for (i=nFdobA; i<iDG->m_idDom->GetNRecs(); i++)
    {
      if ((INT16)iDG->m_idDom->Dfetch(i,CDM_OF_DOBT)!=CDM_OT_FFARG) break;
      printf("\n  (%2d) - Type             : '%s'",i-nFdobA+((nFtyp==CSIG_CMEMBER)?2:1),
                                                   (char*)iDG->m_idDom->XAddr(i,CDM_OF_DSPC));
      printf("\n       - Array specifier  : '%s'" ,(char*)iDG->m_idDom->XAddr(i,CDM_OF_EXT1));
      printf("\n       - Formal identifier: '%s'" ,(char*)iDG->m_idDom->XAddr(i,CDM_OF_NAME));
      printf("\n       - Default value    : '%s'" ,(char*)iDG->m_idDom->XAddr(i,CDM_OF_EXT2));
    }
  }

  if (bDeclare)
  {
    // Register in code lists
    switch (nFtyp)
    {
    case CSIG_CXXMEMBER:
      CppMakeDeclarator(CSIG_CXXMEMBER,lpsDspc,lpsName,lpsSnam,lpsMdfr,lpsPvrt,iDG->m_idDom,nFdobA,lpsDeclarator,512);
      m_exports.AddItem(lpsDeclarator);
      break;
    case CSIG_FUNCTION:
      CppMakeDeclarator(CSIG_FUNCTION,lpsDspc,lpsName,lpsSnam,lpsMdfr,lpsPvrt,iDG->m_idDom,nFdobA,lpsDeclarator,512);
      m_Cexports.AddItem(lpsDeclarator);
      break;
    case CSIG_CMEMBER:
      CppMakeDeclarator(CSIG_CMEMBER,lpsDspc,lpsName,lpsSnam,lpsMdfr,lpsPvrt,iDG->m_idDom,nFdobA,lpsDeclarator,512);
      m_Cexports.AddItem(lpsDeclarator);

      if (strstr(lpsDeclarator,"..."))
      {
        INT32 nLine = (INT32)iDG->m_idTsq->Dfetch((INT32)iDG->m_idDom->Dfetch(nFdobF,CDM_OF_FTOK),OF_LINE);
        ERRORMSG(ERR_INFUNCTION,lpsName,iDG->m_lpsFilename,nLine);
        ERRORMSG(ERR_ELLIPSIS,0,0,0);
      }
      else
      {
        CppMakeDeclarator(CSIG_CXXMEMBER,lpsDspc,lpsName,lpsSnam,lpsMdfr,lpsPvrt,iDG->m_idDom,nFdobA,lpsDeclarator,512);
        m_exports.AddItem(lpsDeclarator);

        CppMakeDeclarator(CSIG_CXXWRAPPER_HEAD,lpsDspc,lpsName,lpsSnam,lpsMdfr,lpsPvrt,iDG->m_idDom,nFdobA,lpsDeclarator,512);
        m_cxxWrappers.AddItem(lpsDeclarator);
        m_cxxWrappers.AddItem("{");
        CppMakeDeclarator(CSIG_CXXWRAPPER_CALL,lpsDspc,lpsName,lpsSnam,lpsMdfr,lpsPvrt,iDG->m_idDom,nFdobA,lpsDeclarator,512);
        m_cxxWrappers.AddItem(lpsDeclarator);
        m_cxxWrappers.AddItem("}");
        m_cxxWrappers.AddItem("");
      }
      break;
    default: DLPASSERT(FALSE); // Cannot happen
    }
  }

  if ((bDeclare || m_bClib) && (m_bXDoc || strstr(lpsMdfr,"private")==0))
  {
    // Seek in dLabPro method list; if not found --> add as C/C++ method
    for (lpMth=m_mths.m_items; lpMth; lpMth=lpMth->next)
      if (dlp_strcmp(lpMth->lpCName,lpsSnam)==0)
        break;

    if (!lpMth)
    {
      lpMth = m_cfns.AddItem(lpsName);
      dlp_strcpy(lpMth->lpCName,lpsSnam);
      dlp_strcpy(lpMth->lpReturn.lpType,lpsDspc);
    }

    // Store (new) source position of C/C++ method implementation
    INT32 nLine = (INT32)iDG->m_idTsq->Dfetch((INT32)iDG->m_idDom->Dfetch(nFdobF,CDM_OF_FTOK),OF_LINE);
    dlp_strcpy(lpMth->lpFile,iDG->m_lpsFilename);
    lpMth->nLine = nLine;

    // Use real C declarator, even for dLabPro methods
    CppMakeDeclarator(CSIG_CXXMEMBER,lpsDspc,lpsName,lpsSnam,lpsMdfr,lpsPvrt,iDG->m_idDom,nFdobA,lpsDeclarator,512);
    if (!strstr(lpsDeclarator,"();"))
    {
      strtok(lpsDeclarator,"(");
      dlp_strcpy(lpMth->lpCSyntax,strtok(NULL,")"));
    }
    else dlp_strcpy(lpMth->lpCSyntax,"");
    dlp_strcpy(lpMth->lpReturn.lpMdfr,lpsMdfr);

    // Convert JavaDoc to lpMth->lMan
    nFtokD = (INT32)iDG->m_idDom->Dfetch(nFdobF,CDM_OF_FTOKD);
    nLtokD = (INT32)iDG->m_idDom->Dfetch(nFdobF,CDM_OF_LTOKD);
    if
    (
      nFtokD>=0                        &&
      nLtokD< iDG->m_idTsq->GetNRecs() &&
      nLtokD>=nFtokD
    )
    {
      for (i=nFtokD; i<=nLtokD; i++) lJavaDoc.AddItem((char*)iDG->m_idTsq->XAddr(i,OF_TOK));
      JavaDoc2Html
      (
        &lpMth->lMan,
        dlp_strlen(lpMth->lpComment)==0?lpMth->lpComment:NULL,
        &lJavaDoc,
        iDG->m_lpsFilename,
        (INT32)iDG->m_idTsq->Dfetch(nFtokD,OF_LINE)
      );
      lpMth->bHtmlMan=TRUE;
    }
  }

  return O_K;
}

/**
 * Scans C/C++ source files (listed in m_files) for function definitions and
 * JavaDocs. Stores the function headers and generated wrapper functions in:
 *
 * - m_Cexports
 * - m_exports
 * - m_cxxWrappers
 *
 * @return O_K if successfull, a (negative) error code otherwise
 */
INT16 CGEN_PROTECTED CCgen::CppScanFunctions()
{
  INT32  nDob = 0;
  INT32  nXD  = 0;
  char  lpsFilename[L_PATH];
  char* lpsBuf;

  lpsBuf = (char*)dlp_calloc(255,sizeof(char));

  if (GetVerboseLevel()>1) printf("\nScanning C/C++ source files...");

  SCGFile *file = m_files.m_items;
  while (file != NULL)
  {
    if (file->nFType == FT_CPP || file->nFType == FT_C)
    {
      printf("\n%s %s",GetVerboseLevel()>1?" ":"s -",file->lpName);
      dlp_strcpy(lpsFilename,file->lpName);
      dlp_convert_name(CN_XLATPATH,lpsFilename,lpsFilename);

      m_exports.AddItem(""); m_Cexports.AddItem("");
      sprintf(lpsBuf,"${/*} Taken from '%s'${*/}",file->lpName); m_Cexports.AddItem(lpsBuf);
      sprintf(lpsBuf,"${/*} Taken from '%s'${*/}",file->lpName); m_exports.AddItem(lpsBuf);

      CDgen* iDG = NULL;
      ICREATE(CDgen,iDG,NULL);

      IF_OK(iDG->Scan(lpsFilename,"cpp",NULL))
        for (nDob=0,nXD=iDG->m_idDom->GetNRecs(); nDob<=nXD; nDob++)
          if ((INT16)iDG->m_idDom->Dfetch(nDob,CDM_OF_DOBT)==CDM_OT_FHEAD)
            CppFhead(iDG,nDob);

      IDESTROY(iDG);
    }
    file = file->next;
  }

  dlp_free(lpsBuf);

  return O_K;
}

// -- C/C++ source generator --

/**
 * ERROR implementation - Undefine error code.
 */
void CGEN_PROTECTED CCgen::GC_ErrUndef(SCGErr* lpErr, SCGStr*& lpTmpl, INT16 bFirst)
{
  char lpBuf[255];
  sprintf(lpBuf,"#undef %-20s",lpErr->lpName);
  m_hFileTmpl.InsertItem(lpTmpl,lpBuf);
  lpTmpl=lpTmpl->next;
}

/**
 * ERROR implementation - Define error code.
 */
void CGEN_PROTECTED CCgen::GC_ErrDef(SCGErr* lpErr, SCGStr*& lpTmpl, INT16 bFirst)
{
  char lpBuf[255];
  sprintf(lpBuf,"#define %-20s %5hd",lpErr->lpName,-1001-lpErr->nId);
  m_hFileTmpl.InsertItem(lpTmpl,lpBuf);
  lpTmpl=lpTmpl->next;
}

/**
 * ERROR implementation - RTTI registration
 */
void CGEN_PROTECTED CCgen::GC_ErrReg(SCGErr* lpErr, SCGStr*& lpTmpl, INT16 bFirst)
{
  if (bFirst)
  {
    m_cppFileTmpl.InsertItem(lpTmpl,""); lpTmpl=lpTmpl->next;
    if(m_bCProject) m_cppFileTmpl.InsertItem(lpTmpl,"\t/* Register errors */");
    else            m_cppFileTmpl.InsertItem(lpTmpl,"\t// Register errors");
    lpTmpl=lpTmpl->next;
  }

  char lpBuf[255];
  sprintf(lpBuf,"\tREGISTER_ERROR(\"%s\",%s,%s,\"%s\")",
          dlp_errorcode2id(dlp_get_a_buffer(),-1001-lpErr->nId),
          lpErr->lpErrorLevel,lpErr->lpName,lpErr->lpComment);
  m_cppFileTmpl.InsertItem(lpTmpl,lpBuf);
  lpTmpl=lpTmpl->next;
}

/**
 * FIELD implementation - Declare fields.
 */
void CGEN_PROTECTED CCgen::GC_FldMemDec(SCGFld* lpFld, SCGStr*& lpTmpl, BOOL bFirst)
{
  if      (!m_bCProject            ) __GC_FldMemDec(lpFld,lpTmpl,bFirst,FALSE);
  else if (lpFld->nType!=T_INSTANCE) __GC_FldMemDec(lpFld,lpTmpl,bFirst,TRUE );
  // NOTE: Instance fields in C projects are declared separately
}

/**
 * FIELD implementation - Declare C instance fields.
 */
void CGEN_PROTECTED CCgen::GC_ICFldMemDec(SCGFld* lpFld, SCGStr*& lpTmpl, BOOL bFirst)
{
  if (lpFld->nType!=T_INSTANCE) return;     // Only instance fields
  __GC_FldMemDec(lpFld,lpTmpl,bFirst,TRUE);
}

/**
 * FIELD implementation - Declare C++ instance fields.
 */
void CGEN_PROTECTED CCgen::GC_ICxxFldMemDec(SCGFld* lpFld, SCGStr*& lpTmpl, BOOL bFirst)
{
  if (lpFld->nType!=T_INSTANCE) return;     // Only instance fields
  __GC_FldMemDec(lpFld,lpTmpl,bFirst,FALSE);
}

/**
 * FIELD implementation - Worker.
 */
void CGEN_PROTECTED CCgen::__GC_FldMemDec
(
  SCGFld*  lpFld,
  SCGStr*& lpTmpl,
  BOOL     bFirst,
  BOOL     bForC
)
{
  char lpBuf[255];

  if (dlp_strcmp(lpFld->lpCName,"NULL")==0)
  {
    // Remapped field -> do not declare a member variable
    return;
  }

  if (bForC && lpFld->nType==T_INSTANCE)
    // HACK: Change C-type to CDlpObject*: the syntax description
    //       of the field still contains the derived type
    sprintf(lpBuf,"\t%-16s %s%s;","CDlpObject*",lpFld->lpCName,lpFld->lpArraySpec);
  else
    sprintf(lpBuf,"\t%-16s %s%s;",lpFld->lpCType,lpFld->lpCName,lpFld->lpArraySpec);

  m_hFileTmpl.InsertItem(lpTmpl,lpBuf);
  lpTmpl = lpTmpl->next;
}

/**
 * FIELD implementation - Declare C++ field changed callback function
 */
void CGEN_PROTECTED CCgen::GC_FldFcfDec(SCGFld* lpFld, SCGStr*& lpTmpl, INT16 bFirst)
{
  if (lpFld->lCode.IsListEmpty()) return;

  char lpBuf[255];
  char lpCallback[L_NAMES];
  dlp_convert_name(CN_DLP2CXX_PUCF,lpCallback,lpFld->lpName);
  sprintf(lpBuf,"\tINT16 %s();",lpCallback);
  m_hFileTmpl.InsertItem(lpTmpl,lpBuf);
  lpTmpl = lpTmpl->next;
}

/**
 * FIELD implementation - Declare C field changed callback function
 */
void CGEN_PROTECTED CCgen::GC_CFldFcfDec(SCGFld* lpFld, SCGStr*& lpTmpl, INT16 bFirst)
{
  if (lpFld->lCode.IsListEmpty()) return;

  char lpBuf[255];
  char lpCallback[L_NAMES];
  dlp_convert_name(CN_DLP2CXX_PUCF,lpCallback,lpFld->lpName);
  sprintf(lpBuf,"INT16 %s_%s(CDlpObject*);",m_lpsCName,lpCallback);
  m_hFileTmpl.InsertItem(lpTmpl,lpBuf);
  lpTmpl = lpTmpl->next;
}

/**
 * FIELD implementation - RTTI registration
 */
void CGEN_PROTECTED CCgen::GC_FldReg(SCGFld* lpFld, SCGStr*& lpTmpl, INT16 bFirst)
{
  if (bFirst)
  {
    m_cppFileTmpl.InsertItem(lpTmpl,""); lpTmpl=lpTmpl->next;
    if(m_bCProject) m_cppFileTmpl.InsertItem(lpTmpl,"\t/* Register fields */");
    else            m_cppFileTmpl.InsertItem(lpTmpl,"\t// Register fields");
    lpTmpl=lpTmpl->next;
  }

  char lpBuf[512];
  char lpCallback[L_NAMES];

  if (lpFld->lCode.IsListEmpty()) dlp_strcpy(lpCallback,"NULL");
  else sprintf(lpCallback,"LPMF(%s,%s)",m_lpsCName,dlp_convert_name(CN_DLP2CXX_PUCF,dlp_get_a_buffer(),lpFld->lpName));

  char lpFlags[255];
  BEGIN_FLAGS(lpFlags)
  APPEND_FLAG(lpFld->nFlags,FF_HIDDEN      )
  APPEND_FLAG(lpFld->nFlags,FF_NOSET       )
  APPEND_FLAG(lpFld->nFlags,FF_NOSAVE      )
  APPEND_FLAG(lpFld->nFlags,FF_NONAUTOMATIC)
  END_FLAGS

  char lpCName[L_NAMES];
  if (dlp_strcmp(lpFld->lpCName,"NULL")) sprintf(lpCName,"LPMV(%s)",lpFld->lpCName);
  else                                   dlp_strcpy(lpCName,"NULL");

  char lpInit[255];
  if (dlp_is_numeric_type_code(lpFld->nType)) {
    if(dlp_is_complex_type_code(lpFld->nType)) {
      sprintf(lpInit,"%s",lpFld->lpInit);
    } else {
      sprintf(lpInit,"(%s)%s",lpFld->lpCType,lpFld->lpInit);
    }
  } else {
    sprintf(lpInit,"%s",lpFld->lpInit);
  }

  sprintf(lpBuf,"\tREGISTER_FIELD(\"%s\",\"%s\",%s,%s,\"%s\",%s,%hd,%d,\"%s\",%s)",
      lpFld->lpName,lpFld->lpObsolete,lpCName,lpCallback,lpFld->lpComment,
      lpFlags,(short)lpFld->nType,(int)lpFld->nArrayLen,lpFld->lpType,lpInit);

  m_cppFileTmpl.InsertItem(lpTmpl,lpBuf);
  lpTmpl=lpTmpl->next;
}

/**
 * FIELD implementation - Define C++ field changed callback function
 */
void CGEN_PROTECTED CCgen::GC_FldFcfDef(SCGFld* lpFld, SCGStr*& lpTmpl, INT16 bFirst)
{
  if (lpFld->lCode.IsListEmpty()) return;

  char lpBuf[255];

  // Function header
  char lpCallback[L_NAMES]; dlp_convert_name(CN_DLP2CXX_PUCF,lpCallback,lpFld->lpName);
  sprintf(lpBuf,"INT16 %s::%s()",m_lpsCName,lpCallback);
  m_cppFileTmpl.InsertItem(lpTmpl,lpBuf); lpTmpl = lpTmpl->next;
  m_cppFileTmpl.InsertItem(lpTmpl,"{"); lpTmpl = lpTmpl->next;

  // Implement code
  if (m_bCProject)
  {
    sprintf(lpBuf,"\treturn %s_%s(this);",m_lpsCName,lpCallback);
    m_cppFileTmpl.InsertItem(lpTmpl,lpBuf); lpTmpl = lpTmpl->next;
  }
  else
  {
    CGEN_CODE(m_cppFileTmpl,lpTmpl,lpFld->lCode)
    m_cppFileTmpl.InsertItem(lpTmpl,"" );             lpTmpl = lpTmpl->next;
    m_cppFileTmpl.InsertItem(lpTmpl,"\treturn O_K;"); lpTmpl = lpTmpl->next;
  }

  // Function footer
  m_cppFileTmpl.InsertItem(lpTmpl,"}");             lpTmpl = lpTmpl->next;
  m_cppFileTmpl.InsertItem(lpTmpl,"" );             lpTmpl = lpTmpl->next;
}

/**
 * Define C field changed callback function
 */
void CGEN_PROTECTED CCgen::GC_CFldFcfDef(SCGFld* lpFld, SCGStr*& lpTmpl, INT16 bFirst)
{
  if (lpFld->lCode.IsListEmpty()) return;

  char lpBuf[255];

  // Function header
  char lpCallback[L_NAMES]; dlp_convert_name(CN_DLP2CXX_PUCF,lpCallback,lpFld->lpName);
  sprintf(lpBuf,"INT16 %s_%s(CDlpObject* __this)",m_lpsCName,lpCallback);
  m_cppFileTmpl.InsertItem(lpTmpl,lpBuf); lpTmpl = lpTmpl->next;
  m_cppFileTmpl.InsertItem(lpTmpl,"{"); lpTmpl = lpTmpl->next;
  sprintf(lpBuf,"\tGET_THIS_VIRTUAL_RV(%s,NOT_EXEC);",m_lpsCName);
  m_cppFileTmpl.InsertItem(lpTmpl,lpBuf); lpTmpl = lpTmpl->next;
  m_cppFileTmpl.InsertItem(lpTmpl,"\t{"); lpTmpl = lpTmpl->next;

  // Implement code
  CGEN_CODE(m_cppFileTmpl,lpTmpl,lpFld->lCode)

  // Function footer
  m_cppFileTmpl.InsertItem(lpTmpl,"\t}"); lpTmpl = lpTmpl->next;
  m_cppFileTmpl.InsertItem(lpTmpl,"" );             lpTmpl = lpTmpl->next;
  m_cppFileTmpl.InsertItem(lpTmpl,"\treturn O_K;"); lpTmpl = lpTmpl->next;
  m_cppFileTmpl.InsertItem(lpTmpl,"}");             lpTmpl = lpTmpl->next;
  m_cppFileTmpl.InsertItem(lpTmpl,"" );             lpTmpl = lpTmpl->next;
}

/**
 * OPTION implementation - Declare option member
 */
void CGEN_PROTECTED CCgen::GC_OptMemDec(SCGOpt* lpOpt, SCGStr*& lpTmpl, INT16 bFirst)
{
  char lpBuf[255];
  sprintf(lpBuf,"\tBOOL %s;",lpOpt->lpCName);
  m_hFileTmpl.InsertItem(lpTmpl,lpBuf);
  lpTmpl = lpTmpl->next;
}

/**
 * OPTION implementation - Declare C++ option update callback function
 */
void CGEN_PROTECTED CCgen::GC_OptOcfDec(SCGOpt* lpOpt, SCGStr*& lpTmpl, INT16 bFirst)
{
  if (lpOpt->lCode.IsListEmpty()) return;

  char lpBuf[255];
  char lpCallback[L_NAMES];
  dlp_convert_name(CN_DLP2CXX_OUCF,lpCallback,lpOpt->lpName);
  sprintf(lpBuf,"\tINT16 %s();",lpCallback);
  m_hFileTmpl.InsertItem(lpTmpl,lpBuf);
  lpTmpl = lpTmpl->next;
}

/**
 * OPTION implementation - Declare C option update callback function
 */
void CGEN_PROTECTED CCgen::GC_COptOcfDec(SCGOpt* lpOpt, SCGStr*& lpTmpl, INT16 bFirst)
{
  if (lpOpt->lCode.IsListEmpty()) return;

  char lpBuf[255];
  char lpCallback[L_NAMES];
  dlp_convert_name(CN_DLP2CXX_OUCF,lpCallback,lpOpt->lpName);
  sprintf(lpBuf,"INT16 %s_%s(CDlpObject*);",m_lpsCName,lpCallback);
  m_hFileTmpl.InsertItem(lpTmpl,lpBuf);
  lpTmpl = lpTmpl->next;
}

/**
 * OPTION implementation - RTTI registration
 */
void CGEN_PROTECTED CCgen::GC_OptReg(SCGOpt* lpOpt, SCGStr*& lpTmpl, INT16 bFirst)
{
  if (bFirst)
  {
    m_cppFileTmpl.InsertItem(lpTmpl,""); lpTmpl=lpTmpl->next;
    if(m_bCProject) m_cppFileTmpl.InsertItem(lpTmpl,"\t/* Register options */");
    else            m_cppFileTmpl.InsertItem(lpTmpl,"\t// Register options");
    lpTmpl=lpTmpl->next;
  }

  char lpBuf[255];
  char lpCallback[L_NAMES];

  if (lpOpt->lCode.IsListEmpty()) dlp_strcpy(lpCallback,"NULL");
  else sprintf(lpCallback,"LPMF(%s,%s)",m_lpsCName,dlp_convert_name(CN_DLP2CXX_OUCF,dlp_get_a_buffer(),lpOpt->lpName));

  char lpFlags[255];
  BEGIN_FLAGS(lpFlags)
  APPEND_FLAG(lpOpt->nFlags,OF_NONAUTOMATIC)
  END_FLAGS

  sprintf(lpBuf,"\tREGISTER_OPTION(\"%s\",\"%s\",LPMV(%s),%s,\"%s\",%s)",
          lpOpt->lpName,lpOpt->lpObsolete,lpOpt->lpCName,lpCallback,
          lpOpt->lpComment,lpFlags);

  m_cppFileTmpl.InsertItem(lpTmpl,lpBuf);
  lpTmpl=lpTmpl->next;
}

/**
 * OPTION implementation - Define C++ option changed callback function
 */
void CGEN_PROTECTED CCgen::GC_OptOcfDef(SCGOpt* lpOpt, SCGStr*& lpTmpl, INT16 bFirst)
{
  if (lpOpt->lCode.IsListEmpty()) return;

  char lpBuf[255];

  // Function header
  char lpCallback[L_NAMES]; dlp_convert_name(CN_DLP2CXX_OUCF,lpCallback,lpOpt->lpName);
  sprintf(lpBuf,"INT16 %s::%s()",m_lpsCName,lpCallback);
  m_cppFileTmpl.InsertItem(lpTmpl,lpBuf); lpTmpl = lpTmpl->next;
  m_cppFileTmpl.InsertItem(lpTmpl,"{"); lpTmpl = lpTmpl->next;

  // User code
  if (m_bCProject)
  {
    sprintf(lpBuf,"\treturn %s_%s(this);",m_lpsCName,lpCallback);
    m_cppFileTmpl.InsertItem(lpTmpl,lpBuf); lpTmpl = lpTmpl->next;
  }
  else
  {
    CGEN_CODE(m_cppFileTmpl,lpTmpl,lpOpt->lCode)
    m_cppFileTmpl.InsertItem(lpTmpl,"" );             lpTmpl = lpTmpl->next;
    m_cppFileTmpl.InsertItem(lpTmpl,"\treturn O_K;"); lpTmpl = lpTmpl->next;
  }

  // Function footer
  m_cppFileTmpl.InsertItem(lpTmpl,"}");             lpTmpl = lpTmpl->next;
  m_cppFileTmpl.InsertItem(lpTmpl,"" );             lpTmpl = lpTmpl->next;
}

/**
 * OPTION implementation - Define C option changed callback function
 */
void CGEN_PROTECTED CCgen::GC_COptOcfDef(SCGOpt* lpOpt, SCGStr*& lpTmpl, INT16 bFirst)
{
  if (lpOpt->lCode.IsListEmpty()) return;

  char lpBuf[255];

  // Function header
  char lpCallback[L_NAMES]; dlp_convert_name(CN_DLP2CXX_OUCF,lpCallback,lpOpt->lpName);
  sprintf(lpBuf,"INT16 %s_%s(CDlpObject* __this)",m_lpsCName,lpCallback);
  m_cppFileTmpl.InsertItem(lpTmpl,lpBuf); lpTmpl = lpTmpl->next;
  m_cppFileTmpl.InsertItem(lpTmpl,"{"); lpTmpl = lpTmpl->next;
  sprintf(lpBuf,"\tGET_THIS_VIRTUAL_RV(%s,NOT_EXEC);",m_lpsCName);
  m_cppFileTmpl.InsertItem(lpTmpl,lpBuf); lpTmpl = lpTmpl->next;
  m_cppFileTmpl.InsertItem(lpTmpl,"\t{"); lpTmpl = lpTmpl->next;

  // User code
  CGEN_CODE(m_cppFileTmpl,lpTmpl,lpOpt->lCode)

  // Function footer
  m_cppFileTmpl.InsertItem(lpTmpl,"\t}");           lpTmpl = lpTmpl->next;
  m_cppFileTmpl.InsertItem(lpTmpl,"" );             lpTmpl = lpTmpl->next;
  m_cppFileTmpl.InsertItem(lpTmpl,"\treturn O_K;"); lpTmpl = lpTmpl->next;
  m_cppFileTmpl.InsertItem(lpTmpl,"}");             lpTmpl = lpTmpl->next;
  m_cppFileTmpl.InsertItem(lpTmpl,"" );             lpTmpl = lpTmpl->next;
}

/**
 * OPTION implementation - Reset option
 */
void CGEN_PROTECTED CCgen::GC_OptReset(SCGOpt* lpOpt, SCGStr*& lpTmpl, INT16 bFirst)
{
  char lpBuf[255];
  if (!(lpOpt->nFlags & OF_NONAUTOMATIC))
    sprintf(lpBuf,"\t_this->%s = FALSE;",lpOpt->lpCName);
  else
    sprintf(lpBuf,"\tif (bInit) _this->%s = FALSE;",lpOpt->lpCName);

  m_cppFileTmpl.InsertItem(lpTmpl,lpBuf);
  lpTmpl=lpTmpl->next;
}

/**
 * METHOD implementation - Declare C++ primary method invocation functions
 */
void CGEN_PROTECTED CCgen::GC_MthPmiDec(SCGMth* lpMth, SCGStr*& lpTmpl, INT16 bFirst)
{
  // In C++ classes, argumentless methods with no user defined
  // return value call secondary invocation function directly
  if (!m_bCProject)
    if (!lpMth->lSyntax.m_items && dlp_strlen(lpMth->lpReturn.lpType)==0)
      return;

  char lpBuf[255];
  sprintf(lpBuf,"\tINT16 On%s();",lpMth->lpCName);
  m_hFileTmpl.InsertItem(lpTmpl,lpBuf);
  lpTmpl=lpTmpl->next;
}

/**
 * METHOD implementation - Declare C primary method invocation functions
 */
void CGEN_PROTECTED CCgen::GC_CMthPmiDec(SCGMth* lpMth, SCGStr*& lpTmpl, INT16 bFirst)
{
  // In C++ classes, argumentless methods with no user defined
  // return value call secondary invocation function directly
  if (!m_bCProject)
    if (!lpMth->lSyntax.m_items && dlp_strlen(lpMth->lpReturn.lpType)==0)
      return;

  char lpBuf[255];
  sprintf(lpBuf,"INT16 %s_On%s(CDlpObject*);",m_lpsCName,lpMth->lpCName);
  m_hFileTmpl.InsertItem(lpTmpl,lpBuf);
  lpTmpl=lpTmpl->next;
}

/**
 * METHOD implementation - Declare C++ secondary method invocation functions
 */
void CGEN_PROTECTED CCgen::GC_MthSmiDec(SCGMth* lpMth, SCGStr*& lpTmpl, INT16 bFirst)
{
  if (!lpMth->lCode.m_items)
  {
    // User supplied custom secondary invocation function
    // WARNING: The custom prototype was not checked!
    return;
  }

  char lpBuf[255];
  if (dlp_strlen(lpMth->lpReturn.lpType))
    sprintf(lpBuf,"\t%s %s(%s);",lpMth->lpReturn.lpType,lpMth->lpCName,lpMth->lpCSyntax);
  else
    sprintf(lpBuf,"\tINT16 %s(%s);",lpMth->lpCName,lpMth->lpCSyntax);
  m_hFileTmpl.InsertItem(lpTmpl,lpBuf);
  lpTmpl=lpTmpl->next;
}

/**
 * METHOD implementation - Declare C secondary method invocation functions
 */
void CGEN_PROTECTED CCgen::GC_CMthSmiDec(SCGMth* lpMth, SCGStr*& lpTmpl, INT16 bFirst)
{
  if (!lpMth->lCode.m_items)
  {
    // User supplied custom secondary invocation function
    // WARNING: The custom prototype was not checked!
    return;
  }

  char lpBuf[255];
  if (dlp_strlen(lpMth->lpCSyntax))
  {
    if (dlp_strlen(lpMth->lpReturn.lpType))
      sprintf(lpBuf,"%s %s_%s(%s*, %s);",lpMth->lpReturn.lpType,m_lpsCName,lpMth->lpCName,m_lpsCName,lpMth->lpCSyntax);
    else
      sprintf(lpBuf,"INT16 %s_%s(%s*, %s);",m_lpsCName,lpMth->lpCName,m_lpsCName,lpMth->lpCSyntax);
  }
  else
  {
    if (dlp_strlen(lpMth->lpReturn.lpType))
      sprintf(lpBuf,"%s %s_%s(%s*);",lpMth->lpReturn.lpType,m_lpsCName,lpMth->lpCName,m_lpsCName);
    else
      sprintf(lpBuf,"INT16 %s_%s(%s*);",m_lpsCName,lpMth->lpCName,m_lpsCName);
  }
  m_hFileTmpl.InsertItem(lpTmpl,lpBuf);
  lpTmpl=lpTmpl->next;
}

/**
 * METHOD implementation - RTTI registration
 */
void CGEN_PROTECTED CCgen::GC_MthReg(SCGMth* lpMth, SCGStr*& lpTmpl, INT16 bFirst)
{
  if (bFirst)
  {
    m_cppFileTmpl.InsertItem(lpTmpl,""); lpTmpl=lpTmpl->next;
    if (m_bCProject) m_cppFileTmpl.InsertItem(lpTmpl,"\t/* Register methods */");
    else             m_cppFileTmpl.InsertItem(lpTmpl,"\t// Register methods");
    lpTmpl=lpTmpl->next;
  }

  char lpBuf[1024];
  char lpCallback[L_NAMES];
  if
  (
    m_bCProject                          ||
    lpMth->lSyntax.m_items               ||
    dlp_strlen(lpMth->lpReturn.lpSupply)
  )
  {
    sprintf(lpCallback,"On%s",lpMth->lpCName);
  }
  else
    sprintf(lpCallback,"%s"  ,lpMth->lpCName);

  char lpFlags[1024];
  BEGIN_FLAGS(lpFlags)
  APPEND_FLAG(lpMth->nFlags,MF_NONDISTINCT)
  END_FLAGS

  sprintf(lpBuf,"\tREGISTER_METHOD(\"%s\",\"%s\",LPMF(%s,%s),\"%s\",%s,\"%s\",\"%s\")",
          lpMth->lpName,lpMth->lpObsolete,m_lpsCName,lpCallback,lpMth->lpComment,
          lpFlags,lpMth->lpUPNSyntax,lpMth->lpPostsyn);

  m_cppFileTmpl.InsertItem(lpTmpl,lpBuf);
  lpTmpl=lpTmpl->next;
}

/**
 * METHOD implementation - Define primary method invocation functions (C++)
 */
void CGEN_PROTECTED CCgen::GC_MthPmiDef(SCGMth* lpMth, SCGStr*& lpTmpl, INT16 bFirst)
{
  __GC_MthPmiDef(lpMth,lpTmpl,bFirst,m_bCProject);
}

/**
 * METHOD implementation - Define primary method invocation functions (C)
 */
void CGEN_PROTECTED CCgen::GC_CMthPmiDef(SCGMth* lpMth, SCGStr*& lpTmpl, INT16 bFirst)
{
  __GC_MthPmiDef(lpMth,lpTmpl,bFirst,FALSE);
}

/**
 * METHOD implementation - Define primary method invocation functions (worker)
 */
void CGEN_PROTECTED CCgen::__GC_MthPmiDef(SCGMth* lpMth, SCGStr*& lpTmpl, INT16 bFirst, BOOL bWrapper)
{
  // In C++ classes, argumentless methods with no user defined
  // return value call secondary invocation function directly
  if (!m_bCProject)
    if (!lpMth->lSyntax.m_items && dlp_strlen(lpMth->lpReturn.lpType)==0)
      return;

  char lpBuf[255];
  char lpBuf2[255];

  // C++ wrapper method
  if (bWrapper)
  {
    // Function header
    sprintf(lpBuf,"INT16 %s::On%s()",m_lpsCName,lpMth->lpCName);
    m_cppFileTmpl.InsertItem(lpTmpl,lpBuf); lpTmpl = lpTmpl->next;
    m_cppFileTmpl.InsertItem(lpTmpl,"/* DO NOT CALL THIS FUNCTION FROM C++ SCOPE.     */"); lpTmpl = lpTmpl->next;
    m_cppFileTmpl.InsertItem(lpTmpl,"/* IT MAY INTERFERE WITH THE INTERPRETER SESSION */"); lpTmpl = lpTmpl->next;
    m_cppFileTmpl.InsertItem(lpTmpl,"{"); lpTmpl = lpTmpl->next;

    // Call C implementation
    sprintf(lpBuf,"\treturn %s_On%s(this);",m_lpsCName,lpMth->lpCName);
    m_cppFileTmpl.InsertItem(lpTmpl,lpBuf); lpTmpl = lpTmpl->next;

    // Function footer
    m_cppFileTmpl.InsertItem(lpTmpl,"}"); lpTmpl = lpTmpl->next;
    m_cppFileTmpl.InsertItem(lpTmpl,"" ); lpTmpl = lpTmpl->next;
  }

  // Implementation of primary method invocation callback function
  else
  {
    // Function header
    if (m_bCProject)
      sprintf(lpBuf,"INT16 %s_On%s(CDlpObject* __this)",m_lpsCName,lpMth->lpCName);
    else
      sprintf(lpBuf,"INT16 %s::On%s()",m_lpsCName,lpMth->lpCName);

    m_cppFileTmpl.InsertItem(lpTmpl,lpBuf); lpTmpl = lpTmpl->next;
    m_cppFileTmpl.InsertItem(lpTmpl,"/* DO NOT CALL THIS FUNCTION FROM C++ SCOPE.     */"); lpTmpl = lpTmpl->next;
    m_cppFileTmpl.InsertItem(lpTmpl,"/* IT MAY INTERFERE WITH THE INTERPRETER SESSION */"); lpTmpl = lpTmpl->next;
    m_cppFileTmpl.InsertItem(lpTmpl,"{"); lpTmpl = lpTmpl->next;

    // User defined primary method invocation code
    if (lpMth->lPCode.m_items)
    {
      m_cppFileTmpl.InsertItem(lpTmpl,"/* User defined code --> */"); lpTmpl = lpTmpl->next;
      CGEN_CODE(m_cppFileTmpl,lpTmpl,lpMth->lPCode);
      m_cppFileTmpl.InsertItem(lpTmpl,"/* <-- User defined code */"); lpTmpl = lpTmpl->next;
    }

    // Generate primary method invocation code
    else
    {
      m_cppFileTmpl.InsertItem(lpTmpl,"\tINT16 __nErr    = O_K;"); lpTmpl = lpTmpl->next;
      m_cppFileTmpl.InsertItem(lpTmpl,"\tINT32  __nErrCnt = 0;"  ); lpTmpl = lpTmpl->next;

      if (m_bCProject)
      {
        // Declare variables
        SCGSyn* lpSyn = lpMth->lSyntax.m_items;
        while (lpSyn)
        {
          if (dlp_strlen(lpSyn->lpSupply))
          {
            strcpy(lpBuf,lpSyn->lpSupply);
            strcpy(lpBuf2,strtok(lpBuf,"="));
            dlp_strtrimright(lpBuf2); strcat(lpBuf2,";");
            m_cppFileTmpl.InsertItem(lpTmpl,lpBuf2); lpTmpl = lpTmpl->next;
          }
          lpSyn = lpSyn->next;
        }

        sprintf(lpBuf,"\tGET_THIS_VIRTUAL_RV(%s,NOT_EXEC);",m_lpsCName);
        m_cppFileTmpl.InsertItem(lpTmpl,lpBuf); lpTmpl = lpTmpl->next;
      }

      char lpSCSyntax[512]; strcpy(lpSCSyntax,"");
      SCGSyn* lpSyn = NULL;

      // Code for checking method invocation context
      m_cppFileTmpl.InsertItem(lpTmpl,"\tMIC_CHECK;"); lpTmpl = lpTmpl->next;

      // Generate secondary invocation (in lpBuf)
      for (lpSyn=lpMth->lSyntax.m_items; lpSyn; lpSyn=lpSyn->next)
      {
        if (dlp_strlen(lpSCSyntax)) strcat(lpSCSyntax,", ");
        strcat(lpSCSyntax,lpSyn->lpName);
      }
      if (dlp_strlen(lpMth->lpReturn.lpSupply))
      {
        if (m_bCProject)
        {
          if (dlp_strlen(lpSCSyntax))
            sprintf(lpBuf,"\t%s%s_%s(_this, %s));",lpMth->lpReturn.lpSupply,m_lpsCName,lpMth->lpCName,lpSCSyntax);
          else
            sprintf(lpBuf,"\t%s%s_%s(_this));",lpMth->lpReturn.lpSupply,m_lpsCName,lpMth->lpCName);
        }
        else
          sprintf(lpBuf,"\t%s%s(%s));",lpMth->lpReturn.lpSupply,lpMth->lpCName,lpSCSyntax);
      }
      else
      {
        if (m_bCProject)
        {
          if (dlp_strlen(lpSCSyntax))
            sprintf(lpBuf,"\t__nErr = %s_%s(_this, %s);",m_lpsCName,lpMth->lpCName,lpSCSyntax);
          else
            sprintf(lpBuf,"\t__nErr = %s_%s(_this);",m_lpsCName,lpMth->lpCName);
        }
        else
          sprintf(lpBuf,"\t__nErr = %s(%s);",lpMth->lpCName,lpSCSyntax);
      }

      // Insert sec. invocation code and remember argument supply insertion point
      m_cppFileTmpl.InsertItem(lpTmpl,"\t__nErrCnt = CDlpObject_GetErrorCount();"); lpTmpl = lpTmpl->next;
      SCGStr* lpIns = lpTmpl;
      m_cppFileTmpl.InsertItem(lpTmpl,"\tif (CDlpObject_GetErrorCount()>__nErrCnt) return NOT_EXEC;"); lpTmpl = lpTmpl->next;
      m_cppFileTmpl.InsertItem(lpTmpl,lpBuf); lpTmpl = lpTmpl->next;

      // Argument supply code
      for (lpSyn=lpMth->lSyntax.m_items; lpSyn; lpSyn=lpSyn->next)
      {
        if (dlp_strlen(lpSyn->lpSupply))
        {
          if (m_bCProject)
          {

            strcpy(lpBuf,lpSyn->lpSupply);
            strcpy(lpBuf2,strtok(lpBuf,"="));
            dlp_strtrimright(lpBuf2);
            char* tx = lpBuf2 + strlen(lpBuf2)-1;
            while (*tx && !iswspace(*tx)) tx--;
            sprintf(lpBuf,"\t%s",lpSyn->lpSupply + (int)(tx - lpBuf2)+1);
            m_cppFileTmpl.InsertItem(lpIns,lpBuf);
          }
          else
            m_cppFileTmpl.InsertItem(lpIns,lpSyn->lpSupply);
        }
      }

      // Return code
      m_cppFileTmpl.InsertItem(lpTmpl,"\treturn __nErr;");
      lpTmpl = lpTmpl->next;
    }

    // Function footer
    m_cppFileTmpl.InsertItem(lpTmpl,"}"); lpTmpl = lpTmpl->next;
    m_cppFileTmpl.InsertItem(lpTmpl,"" ); lpTmpl = lpTmpl->next;
  }
}

// METHOD implementation - Define secondary method callback functions
void CGEN_PROTECTED CCgen::GC_MthSmiDef(SCGMth* lpMth, SCGStr*& lpTmpl, INT16 bFirst)
{
  __GC_MthSmiDef(lpMth,lpTmpl,bFirst,m_bCProject);
}

/**
 * METHOD implementation - Define secondary method invocation functions (C)
 */
void CGEN_PROTECTED CCgen::GC_CMthSmiDef(SCGMth* lpMth, SCGStr*& lpTmpl, INT16 bFirst)
{
  __GC_MthSmiDef(lpMth,lpTmpl,bFirst,FALSE);
}

/**
 * METHOD implementation - Define secondary method invocation functions (worker)
 */
void CGEN_PROTECTED CCgen::__GC_MthSmiDef(SCGMth* lpMth, SCGStr*& lpTmpl, INT16 bFirst, BOOL bWrapper)
{
  if (!lpMth->lCode.m_items)
  {
    // User supplied custom secondary invocation function
    // WARNING: The custom prototype was not checked!
    return;
  }

  char lpBuf[512];

  // Function header
  if (bWrapper || !m_bCProject)
  {
    if (dlp_strlen(lpMth->lpReturn.lpType))
      sprintf(lpBuf,"%s %s::%s(%s)",lpMth->lpReturn.lpType,m_lpsCName,lpMth->lpCName,lpMth->lpCSyntax);
    else
      sprintf(lpBuf,"INT16 %s::%s(%s)",m_lpsCName,lpMth->lpCName,lpMth->lpCSyntax);

    m_cppFileTmpl.InsertItem(lpTmpl,lpBuf); lpTmpl = lpTmpl->next;
    m_cppFileTmpl.InsertItem(lpTmpl,"{"); lpTmpl = lpTmpl->next;
  }
  else
  {
    if (dlp_strlen(lpMth->lpCSyntax))
    {
      if (dlp_strlen(lpMth->lpReturn.lpType))
        sprintf(lpBuf,"%s %s_%s(%s* _this, %s)",lpMth->lpReturn.lpType,m_lpsCName,lpMth->lpCName,m_lpsCName,lpMth->lpCSyntax);
      else
        sprintf(lpBuf,"INT16 %s_%s(%s* _this, %s)",m_lpsCName,lpMth->lpCName,m_lpsCName,lpMth->lpCSyntax);
    }
    else
    {
      if (dlp_strlen(lpMth->lpReturn.lpType))
        sprintf(lpBuf,"%s %s_%s(%s* _this)",lpMth->lpReturn.lpType,m_lpsCName,lpMth->lpCName,m_lpsCName);
      else
        sprintf(lpBuf,"INT16 %s_%s(%s* _this)",m_lpsCName,lpMth->lpCName,m_lpsCName);
    }

    m_cppFileTmpl.InsertItem(lpTmpl,lpBuf); lpTmpl = lpTmpl->next;
    m_cppFileTmpl.InsertItem(lpTmpl,"{"); lpTmpl = lpTmpl->next;
//    sprintf(lpBuf,"\tGET_THIS_VIRTUAL_RV(%s,NOT_EXEC);",m_lpsCName);
//    m_cppFileTmpl.InsertItem(lpTmpl,lpBuf); lpTmpl = lpTmpl->next;
//    m_cppFileTmpl.InsertItem(lpTmpl,"\t{"); lpTmpl = lpTmpl->next;
  }

  if (bWrapper)
  {
    // Argument supply code
    char lpSCSyntax[512]; strcpy(lpSCSyntax,"");
    SCGSyn* lpSyn = lpMth->lSyntax.m_items;
    while (lpSyn)
    {
      if (dlp_strlen(lpSCSyntax)) strcat(lpSCSyntax,", ");
      strcat(lpSCSyntax,lpSyn->lpName);
      lpSyn = lpSyn->next;
    }

    if (dlp_strlen(lpSCSyntax))
      sprintf(lpBuf,"\treturn %s_%s(this,%s);",m_lpsCName,lpMth->lpCName,lpSCSyntax);
    else
      sprintf(lpBuf,"\treturn %s_%s(this);",m_lpsCName,lpMth->lpCName);
    m_cppFileTmpl.InsertItem(lpTmpl,lpBuf); lpTmpl = lpTmpl->next;
  }
  else
  {
    // User code
    CGEN_CODE(m_cppFileTmpl,lpTmpl,lpMth->lCode)

    // Function footer
    dlp_strcpy(lpBuf,"\treturn O_K;");
    if (dlp_strlen(lpMth->lpReturn.lpType))
    {
      if (dlp_strlen(lpMth->lpReturn.lpDefaultVal))
        sprintf(lpBuf,"\treturn %s;",lpMth->lpReturn.lpDefaultVal);
      else if (dlp_strncmp(lpMth->lpReturn.lpType,"void",L_NAMES)==0)
        dlp_strcpy(lpBuf,"");
      else DLPASSERT(FALSE); // Inconsistent return type / default value!
    }
    if (dlp_strlen(lpBuf))
    {
      m_cppFileTmpl.InsertItem(lpTmpl,lpBuf);
      lpTmpl = lpTmpl->next;
    }
  }

//  if (!bWrapper && m_bCProject)
//  {
//    m_cppFileTmpl.InsertItem(lpTmpl,"\t}"); lpTmpl = lpTmpl->next;
//  }

  m_cppFileTmpl.InsertItem(lpTmpl,"}"); lpTmpl = lpTmpl->next;
  m_cppFileTmpl.InsertItem(lpTmpl,"" ); lpTmpl = lpTmpl->next;
}

/**
 * $XXX symbols are not longer needed in CGen implemented code.
 * So this function entirely removes them with the one exception
 * that a single $ will be replaced by the implicit 'this' pointer.
 */
INT16 CGEN_PROTECTED CCgen::KillDollars(char* lpText)
{
  ReplaceKey(lpText,"stp->",""    ,0);
  ReplaceKey(lpText,"stp"  ,"this",0);

  char *tx = strstr(lpText,"$");
  while (tx)
  {
    if ((tx[1] >='a' && tx[1] <='z') ||
      (tx[1] >='A' && tx[1] <='Z') ||
      (tx[1] == '_'))
    {
      dlp_memmove(tx,&tx[1],dlp_strlen(tx)+1);
    }
    else // that means a "single" $
    {
      dlp_memmove(&tx[3],tx,dlp_strlen(tx)+1);
      *tx++ = 't'; *tx++ = 'h'; *tx++ = 'i'; *tx = 's';
    }
    tx = strstr(lpText,"$");
  }

  return O_K;
}

/**
 * Inserts custom C/C++ code from DEF-file into file template.
 */
INT16 CGEN_PROTECTED CCgen::RealizeCode(CList<SCGStr> &lTmpl, const char *lpKey, CList<SCGStr> &lCode)
{
  SCGStr *pcode;
  SCGStr *ptmpl;

  for (ptmpl = lTmpl.m_items; ptmpl!=NULL; ptmpl = ptmpl->next)
    if (strstr(ptmpl->lpName,lpKey))
      for (pcode = lCode.m_items; pcode!=NULL; pcode = pcode->next)
      {
        lTmpl.InsertItem(ptmpl,pcode->lpName);
        ptmpl = ptmpl->next;
      }

  return O_K;
}

/**
 * Creates C/C++ source files.
 */
INT16 CGEN_PROTECTED CCgen::CreateSourceFiles()
{
  // Get file names for source files
  char lpHfn[255]; dlp_convert_name(CN_HFILE,lpHfn,m_lpsProject);
  char lpCfn[255];
  if (m_bCProject) dlp_convert_name(CN_CFILE  ,lpCfn,m_lpsProject);
  else             dlp_convert_name(CN_CPPFILE,lpCfn,m_lpsProject);

  // Replace keys in source file templates
  ReplaceAllKeys(m_hFileTmpl);
  ReplaceAllKeys(m_cppFileTmpl);

  // Insert copyright, includes, defines and friend classes
  if (m_bCProject)
  {
    RealizeCopyright(m_hFileTmpl  ," * ");
    RealizeCopyright(m_cppFileTmpl," * ");
    RealizeStrList(m_hFileTmpl  ,m_includes ,"/*{{CGEN_INCLUDE","#include "      ,"" );
    RealizeStrList(m_cppFileTmpl,m_pincludes,"/*{{CGEN_INCLUDE","#include "      ,"" );
    RealizeStrList(m_hFileTmpl  ,m_defines  ,"/*{{CGEN_DEFINE" ,"#define "       ,"" );
    RealizeStrList(m_hFileTmpl  ,m_friends  ,"/*{{CGEN_FRIENDS","\tfriend class ",";");
  }
  else
  {
    RealizeCopyright(m_hFileTmpl  ,"// ");
    RealizeCopyright(m_cppFileTmpl,"// ");
    RealizeStrList(m_hFileTmpl  ,m_includes ,"//{{CGEN_INCLUDE","#include "      ,"" );
    RealizeStrList(m_cppFileTmpl,m_pincludes,"//{{CGEN_INCLUDE","#include "      ,"" );
    RealizeStrList(m_hFileTmpl  ,m_defines  ,"//{{CGEN_DEFINE" ,"#define "       ,"" );
    RealizeStrList(m_hFileTmpl  ,m_friends  ,"//{{CGEN_FRIENDS","\tfriend class ",";");
  }

  if (m_bMainProject)
  {
    CList<SCGStr> lIncludes;
    CList<SCGStr> lSems;

    for (SCGIcl* page=m_icls.m_items; page; page=page->next)
    {
      char lpBuf[L_PATH];
      if (dlp_strlen(page->lpIPath))
        sprintf(lpBuf,"%s%c%s",page->lpIPath,C_DIR,page->lpHFile);
      else
        dlp_strcpy(lpBuf,page->lpHFile);
      lIncludes.AddItem(lpBuf);
      dlp_convert_name(CN_DLP2CXX_CLSN,lpBuf,page->lpName); lSems.AddItem(lpBuf);
    }

    if (m_bCProject)
    {
      RealizeStrList(m_cppFileTmpl,lIncludes,"/*{{CGEN_INCLUDE"         ,"#include \""        ,"\"");
      RealizeStrList(m_cppFileTmpl,lSems    ,"/*{{CGEN_REGISTER_CLASS"  ,"  REGISTER_CLASS("  ,")" );
      RealizeStrList(m_cppFileTmpl,lSems    ,"/*{{CGEN_UNREGISTER_CLASS","  UNREGISTER_CLASS(",")" );
    }
    else
    {
      RealizeStrList(m_cppFileTmpl,lIncludes,"//{{CGEN_INCLUDE"          ,"#include \""       ,"\"");
      RealizeStrList(m_cppFileTmpl,lSems    ,"//{{CGEN_REGISTER_CLASS"  ,"  REGISTER_CLASS("  ,")" );
      RealizeStrList(m_cppFileTmpl,lSems    ,"//{{CGEN_UNREGISTER_CLASS","  UNREGISTER_CLASS(",")" );
    }
  }

  // Insert members & custom code
  if (m_bCProject)
  {
    CGEN_WORDS(m_hFileTmpl  ,"/*{{CGEN_ERRORS"       ,SCGErr,m_errors,GC_ErrDef       )
    CGEN_WORDS(m_hFileTmpl  ,"/*{{CGEN_ERRORS"       ,SCGErr,m_errors,GC_ErrUndef     )
    CGEN_WORDS(m_cppFileTmpl,"/*{{CGEN_REGISTERWORDS",SCGErr,m_errors,GC_ErrReg       )

    CGEN_WORDS(m_hFileTmpl  ,"/*{{CGEN_FIELDS"       ,SCGFld,m_flds  ,GC_FldMemDec    )
    CGEN_WORDS(m_hFileTmpl  ,"/*{{CGEN_IC_FIELDS"    ,SCGFld,m_flds  ,GC_ICFldMemDec  )
    CGEN_WORDS(m_hFileTmpl  ,"/*{{CGEN_ICXX_FIELDS"  ,SCGFld,m_flds  ,GC_ICxxFldMemDec)
    CGEN_WORDS(m_hFileTmpl  ,"/*{{CGEN_FCCF"         ,SCGFld,m_flds  ,GC_FldFcfDec    )
    CGEN_WORDS(m_hFileTmpl  ,"/*{{CGEN_CFCCF"        ,SCGFld,m_flds  ,GC_CFldFcfDec   )
    CGEN_WORDS(m_cppFileTmpl,"/*{{CGEN_REGISTERWORDS",SCGFld,m_flds  ,GC_FldReg       )
    CGEN_WORDS(m_cppFileTmpl,"/*{{CGEN_FCCF"         ,SCGFld,m_flds  ,GC_FldFcfDef    )
    CGEN_WORDS(m_cppFileTmpl,"/*{{CGEN_CFCCF"        ,SCGFld,m_flds  ,GC_CFldFcfDef   )

    CGEN_WORDS(m_hFileTmpl  ,"/*{{CGEN_OPTIONS"        ,SCGOpt,m_opts  ,GC_OptMemDec    )
    CGEN_WORDS(m_hFileTmpl  ,"/*{{CGEN_OCCF"           ,SCGOpt,m_opts  ,GC_OptOcfDec    )
    CGEN_WORDS(m_hFileTmpl  ,"/*{{CGEN_COCCF"          ,SCGOpt,m_opts  ,GC_COptOcfDec   )
    CGEN_WORDS(m_cppFileTmpl,"/*{{CGEN_REGISTERWORDS"  ,SCGOpt,m_opts  ,GC_OptReg       )
    CGEN_WORDS(m_cppFileTmpl,"/*{{CGEN_OCCF"           ,SCGOpt,m_opts  ,GC_OptOcfDef    )
    CGEN_WORDS(m_cppFileTmpl,"/*{{CGEN_COCCF"          ,SCGOpt,m_opts  ,GC_COptOcfDef   )
    CGEN_WORDS(m_cppFileTmpl,"/*{{CGEN_RESETALLOPTIONS",SCGOpt,m_opts  ,GC_OptReset     )

    CGEN_WORDS(m_hFileTmpl  ,"/*{{CGEN_PMIC"         ,SCGMth,m_mths  ,GC_MthPmiDec    )
    CGEN_WORDS(m_hFileTmpl  ,"/*{{CGEN_CPMIC"        ,SCGMth,m_mths  ,GC_CMthPmiDec   )
    CGEN_WORDS(m_hFileTmpl  ,"/*{{CGEN_SMIC"         ,SCGMth,m_mths  ,GC_MthSmiDec    )
    CGEN_WORDS(m_hFileTmpl  ,"/*{{CGEN_CSMIC"        ,SCGMth,m_mths  ,GC_CMthSmiDec   )
    CGEN_WORDS(m_cppFileTmpl,"/*{{CGEN_REGISTERWORDS",SCGMth,m_mths  ,GC_MthReg       )
    CGEN_WORDS(m_cppFileTmpl,"/*{{CGEN_PMIC"         ,SCGMth,m_mths  ,GC_MthPmiDef    )
    CGEN_WORDS(m_cppFileTmpl,"/*{{CGEN_CPMIC"        ,SCGMth,m_mths  ,GC_CMthPmiDef   )
    CGEN_WORDS(m_cppFileTmpl,"/*{{CGEN_SMIC"         ,SCGMth,m_mths  ,GC_MthSmiDef    )
    CGEN_WORDS(m_cppFileTmpl,"/*{{CGEN_CSMIC"        ,SCGMth,m_mths  ,GC_CMthSmiDef   )

    RealizeCode(m_hFileTmpl  ,"/*{{CGEN_HEADERCODE" ,m_headerCode );
    RealizeCode(m_cppFileTmpl,"/*{{CGEN_CLASSCODE"  ,m_classCode  );
    RealizeCode(m_cppFileTmpl,"/*{{CGEN_INITCODE"   ,m_initCode   );
    RealizeCode(m_cppFileTmpl,"/*{{CGEN_DONECODE"   ,m_doneCode   );
    RealizeCode(m_cppFileTmpl,"/*{{CGEN_RESETCODE"  ,m_resetCode  );
    RealizeCode(m_cppFileTmpl,"/*{{CGEN_SAVECODE"   ,m_saveCode   );
    RealizeCode(m_cppFileTmpl,"/*{{CGEN_RESTORECODE",m_restoreCode);
    RealizeCode(m_cppFileTmpl,"/*{{CGEN_COPYCODE"   ,m_copyCode   );
    RealizeCode(m_cppFileTmpl,"/*{{CGEN_INSTALLCODE",m_installCode);
  }
  else
  {
    CGEN_WORDS(m_hFileTmpl  ,"//{{CGEN_ERRORS"         ,SCGErr,m_errors,GC_ErrDef   )
    CGEN_WORDS(m_hFileTmpl  ,"//{{CGEN_ERRORS"         ,SCGErr,m_errors,GC_ErrUndef )
    CGEN_WORDS(m_cppFileTmpl,"//{{CGEN_REGISTERWORDS"  ,SCGErr,m_errors,GC_ErrReg   )

    CGEN_WORDS(m_hFileTmpl  ,"//{{CGEN_FIELDS"         ,SCGFld,m_flds  ,GC_FldMemDec)
    CGEN_WORDS(m_hFileTmpl  ,"//{{CGEN_FCCF"           ,SCGFld,m_flds  ,GC_FldFcfDec)
    CGEN_WORDS(m_cppFileTmpl,"//{{CGEN_REGISTERWORDS"  ,SCGFld,m_flds  ,GC_FldReg   )
    CGEN_WORDS(m_cppFileTmpl,"//{{CGEN_FCCF"           ,SCGFld,m_flds  ,GC_FldFcfDef)

    CGEN_WORDS(m_hFileTmpl  ,"//{{CGEN_OPTIONS"        ,SCGOpt,m_opts  ,GC_OptMemDec)
    CGEN_WORDS(m_hFileTmpl  ,"//{{CGEN_OCCF"           ,SCGOpt,m_opts  ,GC_OptOcfDec)
    CGEN_WORDS(m_cppFileTmpl,"//{{CGEN_REGISTERWORDS"  ,SCGOpt,m_opts  ,GC_OptReg   )
    CGEN_WORDS(m_cppFileTmpl,"//{{CGEN_OCCF"           ,SCGOpt,m_opts  ,GC_OptOcfDef)
    CGEN_WORDS(m_cppFileTmpl,"//{{CGEN_RESETALLOPTIONS",SCGOpt,m_opts  ,GC_OptReset     )

    CGEN_WORDS(m_hFileTmpl  ,"//{{CGEN_PMIC"           ,SCGMth,m_mths  ,GC_MthPmiDec)
    CGEN_WORDS(m_hFileTmpl  ,"//{{CGEN_SMIC"           ,SCGMth,m_mths  ,GC_MthSmiDec)
    CGEN_WORDS(m_cppFileTmpl,"//{{CGEN_REGISTERWORDS"  ,SCGMth,m_mths  ,GC_MthReg   )
    CGEN_WORDS(m_cppFileTmpl,"//{{CGEN_PMIC"           ,SCGMth,m_mths  ,GC_MthPmiDef)
    CGEN_WORDS(m_cppFileTmpl,"//{{CGEN_SMIC"           ,SCGMth,m_mths  ,GC_MthSmiDef)

    RealizeCode(m_hFileTmpl  ,"//{{CGEN_HEADERCODE" ,m_headerCode );
    RealizeCode(m_cppFileTmpl,"//{{CGEN_CLASSCODE"  ,m_classCode  );
    RealizeCode(m_cppFileTmpl,"//{{CGEN_INITCODE"   ,m_initCode   );
    RealizeCode(m_cppFileTmpl,"//{{CGEN_DONECODE"   ,m_doneCode   );
    RealizeCode(m_cppFileTmpl,"//{{CGEN_RESETCODE"  ,m_resetCode  );
    RealizeCode(m_cppFileTmpl,"//{{CGEN_SAVECODE"   ,m_saveCode   );
    RealizeCode(m_cppFileTmpl,"//{{CGEN_RESTORECODE",m_restoreCode);
    RealizeCode(m_cppFileTmpl,"//{{CGEN_COPYCODE"   ,m_copyCode   );
    RealizeCode(m_cppFileTmpl,"//{{CGEN_INSTALLCODE",m_installCode);
  }

  // Prototypes and C++ wrapper functions
  if (!m_bMainProject) {
    if (m_bCProject)
    {
        // Make and insert C++ wrapper functions
        RealizeStrList(m_cppFileTmpl,m_cxxWrappers,"/*{{CGEN_CXXWRAP","","");

        // Insert prototypes into header
        RealizeStrList(m_hFileTmpl,m_exports ,"/*{{CGEN_EXPORT" ,"","");
        RealizeStrList(m_hFileTmpl,m_Cexports,"/*{{CGEN_CEXPORT","","");
    }
    else
    {
        // Insert prototypes into header
        RealizeStrList(m_hFileTmpl,m_exports ,"//{{CGEN_EXPORT" ,"","");
        RealizeStrList(m_hFileTmpl,m_Cexports,"//{{CGEN_CEXPORT","","");
    }
  }

  // Replace comment keys
  ReplaceAllKeys2(m_hFileTmpl);

  // Write source files
  if (GetVerboseLevel()>1) printf("\nCreating C/C++ source files...");
  char lpBuf[512]; strcpy(lpBuf,m_lpsIPath);
  char lpPrimaryIPath[L_PATH]; sprintf(lpPrimaryIPath,"%s/automatic",strtok(lpBuf,",; \t"));
  if (!m_bMainProject) WriteBackTemplate(m_hFileTmpl,lpPrimaryIPath,lpHfn);
  WriteBackTemplate(m_cppFileTmpl,NULL,lpCfn);

  return O_K;
}

// -- File Templates --

#define LINEH(A)   m_hFileTmpl.AddItem(A);
#define LINECPP(A) m_cppFileTmpl.AddItem(A);

/**
 * Creates the C/C++ header file template. The result will be stored in field
 * {@link m_hFileTmpl}.
 */
void CGEN_PROTECTED CCgen::CreateHTemplate()
{
  if(IsSdkResource(m_lpsHomePath)){
    LINEH("${/*} dLabPro SDK class ${CxxClass} (${SLName})"                          )
  }else{
    LINEH("${/*} dLabPro class ${CxxClass} (${SLName})"                          )
  }
  LINEH("${**} - Header file"                                                  )
  LINEH("${**}"                                                                )
  LINEH("${**} AUTHOR : ${Author}"                                             )
  LINEH("${**} PACKAGE: ${Package}"                                            )
  LINEH("${**}"                                                                )
  LINEH("${**} This file was generated by dcg. DO NOT MODIFY! Modify ${DefFile} instead.")
  LINEH("${Copyright}"                                                         )
  LINEH("${*/}"                                                                )
  LINEH(""                                                                     )
  LINEH("${/*}{{CGEN_INCLUDE${*/}"                                             )
  LINEH("${/*}}}CGEN_END${*/}"                                                 )
  LINEH(""                                                                     )
  LINEH("${/*}{{CGEN_ERRORS${*/}"                                              )
  LINEH("${/*}}}CGEN_END${*/}"                                                 )
  LINEH(""                                                                     )
  LINEH("${/*} C/C++ language abstraction layer${*/}"                          )
  LINEH("#undef ${TypeName}"                                                   )
  LINEH("#define ${TypeName} ${CxxClass}"                                      )
  LINEH(""                                                                     )
  LINEH("${/*} dLabPro/C++ language abstraction layer${*/}"                    )
  LINEH("#undef ${SLName}"                                                     )
  LINEH("#define ${SLName} ${CxxClass}"                                        )
  LINEH(""                                                                     )
  LINEH("${/*}{{CGEN_DEFINE${*/}"                                              )
  LINEH("${/*}}}CGEN_DEFINE${*/}"                                              )
  LINEH(""                                                                     )
  LINEH("#ifndef __${ProjU}_H"                                                 )
  LINEH("#define __${ProjU}_H"                                                 )
  LINEH(""                                                                     )
  LINEH("${/*}{{CGEN_HEADERCODE${*/}"                                          )
  LINEH("${/*}}}CGEN_HEADERCODE${*/}"                                          )
  LINEH(""                                                                     )
  LINEH("${/*} Class ${CxxClass}${*/}"                                         )
  LINEH(""                                                                     )
  if (m_bCProject)
  {
    LINEH("#ifdef __cplusplus"                                                 )
    LINEH(""                                                                   )
  }
  LINEH("class ${CxxClass} : public ${Parent} ${MoreParents}"                  )
  LINEH("{"                                                                    )
  LINEH(""                                                                     )
  LINEH("typedef ${Parent} inherited;"                                         )
  LINEH("typedef ${CxxClass} thisclass;"                                       )
  LINEH(""                                                                     )
  LINEH("${/*}{{CGEN_FRIENDS${*/}"                                             )
  LINEH("${/*}}}CGEN_FRIENDS${*/}"                                             )
  LINEH("public:"                                                              )
  LINEH("\t${CxxClass}(const char* lpInstanceName, BOOL bCallVirtual = 1);"    )
  LINEH("\tvirtual ~${CxxClass}();"                                            )
  LINEH(""                                                                     )
  LINEH("${/*} Virtual and static function overrides${*/}"                     )
  LINEH("public:"                                                              )
  LINEH("\tvirtual INT16 AutoRegisterWords();"                                 )
  LINEH("\tvirtual INT16 Init(BOOL bCallVirtual = 0);"                         )
  LINEH("\tvirtual INT16 Reset(BOOL bResetMembers = 1);"                       )
  LINEH("\tvirtual INT16 Serialize(CDN3Stream* lpDest);"                       )
  LINEH("\tvirtual INT16 SerializeXml(CXmlStream* lpDest);"                    )
  LINEH("\tvirtual INT16 Deserialize(CDN3Stream* lpSrc);"                      )
  LINEH("\tvirtual INT16 DeserializeXml(CXmlStream* lpSrc);"                   )
  LINEH("\tvirtual INT16 Copy(CDlpObject* iSrc);"                              )
  LINEH("\tvirtual INT16 ClassProc();"                                         )
  LINEH("\tstatic  INT16 InstallProc(void* lpItp);"                            )
  LINEH("\tstatic  ${CxxClass}* CreateInstance(const char* lpName);"           )
  LINEH("\tstatic  INT16 GetClassInfo(SWord* lpClassWord);"                    )
  LINEH("\tvirtual INT16 GetInstanceInfo(SWord* lpClassWord);"                 )
  LINEH("\tvirtual BOOL  IsKindOf(const char* lpClassName);"                   )
  LINEH("\tvirtual INT16 ResetAllOptions(BOOL bInit = 0);"                     )
  LINEH(""                                                                     )
  LINEH("${/*} Primary method invocation functions            ${*/}"           )
  LINEH("${/*} DO NOT CALL THESE FUNCTIONS FROM C SCOPE.      ${*/}"           )
  LINEH("${/*} THEY MAY INTERFERE WITH THE INTERPRETER SESSION${*/}"           )
  LINEH("#ifndef __NOITP"                                                      )
  LINEH("public:"                                                              )
  LINEH("${/*}{{CGEN_PMIC${*/}"                                                )
  LINEH("${/*}}}CGEN_PMIC${*/}"                                                )
  LINEH("#endif ${/*} #ifndef __NOITP${*/}"                                    )
  LINEH(""                                                                     )
  LINEH("${/*} Secondary method invocation functions${*/}"                     )
  LINEH("public:"                                                              )
  LINEH("${/*}{{CGEN_SMIC${*/}"                                                )
  LINEH("${/*}}}CGEN_SMIC${*/}"                                                )
  LINEH(""                                                                     )
  LINEH("${/*} Option changed callback functions${*/}"                         )
  LINEH("public:"                                                              )
  LINEH("${/*}{{CGEN_OCCF${*/}"                                                )
  LINEH("${/*}}}CGEN_OCCF${*/}"                                                )
  LINEH(""                                                                     )
  LINEH("${/*} Field changed callback functions${*/}"                          )
  LINEH("public:"                                                              )
  LINEH("${/*}{{CGEN_FCCF${*/}"                                                )
  LINEH("${/*}}}CGEN_FCCF${*/}"                                                )
  LINEH(""                                                                     )
  LINEH("${/*} Scanned member functions${*/}"                                  )
  LINEH("${/*}{{CGEN_EXPORT${*/}"                                              )
  LINEH("${/*}}}CGEN_EXPORT${*/}"                                              )
  LINEH(""                                                                     )
  LINEH("${/*} Member variables${*/}"                                          )
  LINEH("public:"                                                              )
  if (m_bCProject)
  {
    LINEH("/*{{CGEN_ICXX_FIELDS */"                                            )
    LINEH("/*}}CGEN_ICXX_FIELDS */"                                            )
    LINEH(""                                                                   )
    LINEH("#else  /* #ifdef __cplusplus */"                                    )
    LINEH(""                                                                   )
    LINEH("typedef struct ${CxxClass}"                                         )
    LINEH("{"                                                                  )
    LINEH("  /* Pointer to C base instance */"                                 )
    LINEH("  struct CDlpObject* m_lpBaseInstance;"                             )
    LINEH(""                                                                   )
    LINEH("/*{{CGEN_IC_FIELDS */"                                              )
    LINEH("/*}}CGEN_IC_FIELDS */"                                              )
    LINEH(""                                                                   )
    LINEH("#endif /* #ifdef __cplusplus */"                                    )
  }
  LINEH(""                                                                     )
  LINEH("${/*}{{CGEN_FIELDS${*/}"                                              )
  LINEH("${/*}}}CGEN_FIELDS${*/}"                                              )
  LINEH(""                                                                     )
  LINEH("${/*}{{CGEN_OPTIONS${*/}"                                             )
  LINEH("${/*}}}CGEN_OPTIONS${*/}"                                             )
  LINEH("}"                                                                    )
  LINEH(""                                                                     )
  if (m_bCProject)
  {
    LINEH("#ifndef __cplusplus"                                                )
    LINEH("${CxxClass}"                                                        )
    LINEH("#endif"                                                             )
  }
  LINEH(";"                                                                    )
  LINEH(""                                                                     )
  if (m_bCProject)
  {
    LINEH("/* Class ${CxxClass} (C functions)*/"                               )
    LINEH(""                                                                   )
    LINEH("/* Virtual function overrides */"                                   )
    LINEH("void  ${CxxClass}_Constructor(${CxxClass}*, const char* "
                 "lpInstanceName, BOOL bCallVirtual);"                         )
    LINEH("void  ${CxxClass}_Destructor(CDlpObject*);"                         )
    LINEH("INT16 ${CxxClass}_AutoRegisterWords(CDlpObject*);"                  )
    LINEH("INT16 ${CxxClass}_Reset(CDlpObject*, BOOL bResetMembers);"          )
    LINEH("INT16 ${CxxClass}_Init(CDlpObject*, BOOL bCallVirtual);"            )
    LINEH("INT16 ${CxxClass}_Serialize(CDlpObject*, CDN3Stream* lpDest);"      )
    LINEH("INT16 ${CxxClass}_SerializeXml(CDlpObject*, CXmlStream* lpDest);"   )
    LINEH("INT16 ${CxxClass}_Deserialize(CDlpObject*, CDN3Stream* lpSrc);"     )
    LINEH("INT16 ${CxxClass}_DeserializeXml(CDlpObject*, CXmlStream* lpSrc);"  )
    LINEH("INT16 ${CxxClass}_Copy(CDlpObject*, CDlpObject* __iSrc);"           )
    LINEH("INT16 ${CxxClass}_ClassProc(CDlpObject*);"                          )
    LINEH("INT16 ${CxxClass}_InstallProc(void* lpItp);"                        )
    LINEH("${CxxClass}* ${CxxClass}_CreateInstance(const char* lpName);"       )
    LINEH("INT16 ${CxxClass}_GetClassInfo(SWord* lpClassWord);"                )
    LINEH("INT16 ${CxxClass}_GetInstanceInfo(CDlpObject*, SWord* lpClassWord);")
    LINEH("BOOL  ${CxxClass}_IsKindOf(CDlpObject*, const char* lpClassName);"  )
    LINEH("INT16 ${CxxClass}_ResetAllOptions(CDlpObject*, BOOL bInit);"        )
    LINEH(""                                                                   )
    LINEH("/* Primary method invocation functions             */"              )
    LINEH("/* DO NOT CALL THESE FUNCTIONS FROM C SCOPE.       */"              )
    LINEH("/* THEY MAY INTERFERE WITH THE INTERPRETER SESSION */"              )
    LINEH("#ifndef __NOITP"                                                    )
    LINEH("/*{{CGEN_CPMIC */"                                                  )
    LINEH("/*}}CGEN_CPMIC */"                                                  )
    LINEH("#endif /* #ifndef __NOITP */"                                       )
    LINEH(""                                                                   )
    LINEH("/* Secondary method invocation functions */"                        )
    LINEH("/*{{CGEN_CSMIC */"                                                  )
    LINEH("/*}}CGEN_CSMIC */"                                                  )
    LINEH(""                                                                   )
    LINEH("/* Option changed callback functions */"                            )
    LINEH("/*{{CGEN_COCCF */"                                                  )
    LINEH("/*}}CGEN_COCCF */"                                                  )
    LINEH(""                                                                   )
    LINEH("/* Field changed callback functions */"                             )
    LINEH("/*{{CGEN_CFCCF */"                                                  )
    LINEH("/*}}CGEN_CFCCF */"                                                  )
    LINEH(""                                                                   )
  }
  LINEH("${/*} Scanned C (member) functions${*/}"                              )
  LINEH("${/*}{{CGEN_CEXPORT${*/}"                                             )
  LINEH("${/*}}}CGEN_CEXPORT${*/}"                                             )
  LINEH(""                                                                     )
  LINEH("#endif ${/*}#ifndef __${ProjU}_H${*/}"                                )
  LINEH(""                                                                     )
  LINEH(""                                                                     )
  LINEH("${/*} EOF${*/}"                                                       )
  ReplaceAllKeys2(m_hFileTmpl);
}

/**
 * Creates the C/C++ source file template. The result will be stored in field
 * {@link m_cppFileTmpl}.
 */
void CGEN_PROTECTED CCgen::CreateCPPTemplate()
{
  if(IsSdkResource(m_lpsHomePath)){
    LINECPP("${/*} dLabPro SDK class ${CxxClass} (${SLName})"                          )
  }else{
    LINECPP("${/*} dLabPro class ${CxxClass} (${SLName})"                        )
  }
  LINECPP("${**} - ${Comment}"                                                 )
  LINECPP("${**}"                                                              )
  LINECPP("${**} AUTHOR : ${Author}"                                           )
  LINECPP("${**} PACKAGE: ${Package}"                                          )
  LINECPP("${**}"                                                              )
  LINECPP("${**} This file was generated by dcg. DO NOT MODIFY! Modify ${DefFile} instead.")
  LINECPP("${Copyright}"                                                       )
  LINECPP("${*/}"                                                              )
  LINECPP(""                                                                   )
  if (m_bCProject) LINECPP("#include \"dlp_cscope.h\" /* Indicate C scope */"  )
  LINECPP("${/*}{{CGEN_INCLUDE${*/}"                                           )
  LINECPP("${/*}}}CGEN_END${*/}"                                               )
  LINECPP("#include \"${HFile}\""                                              )
  LINECPP(""                                                                   )
  LINECPP("${/*} Class ${CxxClass}${*/}"                                       )
  LINECPP(""                                                                   )

  // Constructor implementation
  if (m_bCProject)
  {
    LINECPP("void ${CxxClass}_Constructor(${CxxClass}* _this, const char* "
            "lpInstanceName, BOOL bCallVirtual)"                               )
    LINECPP("{"                                                                )
    LINECPP("\tDEBUGMSG(-1,\"${CxxClass}_Constructor; (bCallVirtual=%d)\","
            "(int)bCallVirtual,0,0);"                                          )
    LINECPP(""                                                                 )
    LINECPP("#ifndef __cplusplus"                                              )
    LINECPP(""                                                                 )
    LINECPP("\t/* Register instance */"                                        )
    LINECPP("\tdlp_xalloc_register_object('J',_this,1,sizeof(${CxxClass}),"    )
    LINECPP("\t\t__FILE__,__LINE__,\"${SLName}\",lpInstanceName);"             )
    LINECPP(""                                                                 )
    LINECPP("\t/* Create base instance */"                                     )
    LINECPP("\t_this->m_lpBaseInstance = calloc(1,sizeof(CDlpObject));"        )
    LINECPP("\tCDlpObject_Constructor(_this->m_lpBaseInstance,lpInstanceName,"
            "FALSE);"                                                          )
    LINECPP(""                                                                 )
    LINECPP("\t/* Override virtual member functions */"                        )
    LINECPP("\t_this->m_lpBaseInstance->AutoRegisterWords = ${CxxClass}_AutoRegisterWords;")
    LINECPP("\t_this->m_lpBaseInstance->Reset             = ${CxxClass}_Reset;"            )
    LINECPP("\t_this->m_lpBaseInstance->Init              = ${CxxClass}_Init;"             )
    LINECPP("\t_this->m_lpBaseInstance->Serialize         = ${CxxClass}_Serialize;"        )
    LINECPP("\t_this->m_lpBaseInstance->SerializeXml      = ${CxxClass}_SerializeXml;"     )
    LINECPP("\t_this->m_lpBaseInstance->Deserialize       = ${CxxClass}_Deserialize;"      )
    LINECPP("\t_this->m_lpBaseInstance->DeserializeXml    = ${CxxClass}_DeserializeXml;"   )
    LINECPP("\t_this->m_lpBaseInstance->Copy              = ${CxxClass}_Copy;"             )
    LINECPP("\t_this->m_lpBaseInstance->ClassProc         = ${CxxClass}_ClassProc;"        )
    LINECPP("\t_this->m_lpBaseInstance->GetInstanceInfo   = ${CxxClass}_GetInstanceInfo;"  )
    LINECPP("\t_this->m_lpBaseInstance->IsKindOf          = ${CxxClass}_IsKindOf;"         )
    LINECPP("\t_this->m_lpBaseInstance->Destructor        = ${CxxClass}_Destructor;"       )
    LINECPP("\t_this->m_lpBaseInstance->ResetAllOptions   = ${CxxClass}_ResetAllOptions;"  )
    LINECPP(""                                                                 )
    LINECPP("\t/* Override pointer to derived instance */"                     )
    LINECPP("\t_this->m_lpBaseInstance->m_lpDerivedInstance = _this;"          )
    LINECPP(""                                                                 )
    LINECPP("\t#endif /* #ifndef __cplusplus */"                               )
    LINECPP(""                                                                 )
    LINECPP("\tdlp_strcpy(BASEINST(_this)->m_lpClassName,\"${SLName}\");"      )
    LINECPP("\tdlp_strcpy(BASEINST(_this)->m_lpObsoleteName,\"${ObsName}\");"  )
    LINECPP("\tdlp_strcpy(BASEINST(_this)->m_lpProjectName,\"${Proj}\");"      )
    LINECPP("\tdlp_strcpy(BASEINST(_this)->m_version.no,\"${Version}\");"      )
    LINECPP("\tdlp_strcpy(BASEINST(_this)->m_version.date,\"\");"              )
    LINECPP("\tBASEINST(_this)->m_nClStyle = ${CLStyle};"                      )
    LINECPP(""                                                                 )
    LINECPP("\tif (bCallVirtual)"                                              )
    LINECPP("\t{"                                                              )
    LINECPP("\t\tDLPASSERT(OK(INVOKE_VIRTUAL_0(AutoRegisterWords)));"          )
    LINECPP("\t\tINVOKE_VIRTUAL_1(Init,TRUE);"                                 )
    LINECPP("\t}"                                                              )
    LINECPP("}"                                                                )
  }
  else
  {
    LINECPP("${CxxClass}::${CxxClass}(const char* lpInstanceName, "
            "BOOL bCallVirtual) : inherited(lpInstanceName,0)"                 )
    LINECPP("{"                                                                )
    LINECPP("\tDEBUGMSG(-1,\"${CxxClass}::${CxxClass}; (bCallVirtual=%d)\","
            "(int)bCallVirtual,0,0);"                                          )
    LINECPP("\tdlp_strcpy(m_lpClassName,\"${SLName}\");"                       )
    LINECPP("\tdlp_strcpy(m_lpObsoleteName,\"${ObsName}\");"                   )
    LINECPP("\tdlp_strcpy(m_lpProjectName,\"${Proj}\");"                       )
    LINECPP("\tdlp_strcpy(m_version.no,\"${Version}\");"                       )
    LINECPP("\tdlp_strcpy(m_version.date,\"\");"                               )
    LINECPP("\tm_nClStyle = ${CLStyle};"                                       )
    LINECPP(""                                                                 )
    LINECPP("\tif (bCallVirtual)"                                              )
    LINECPP("\t{"                                                              )
    LINECPP("\t\tDLPASSERT(OK(AutoRegisterWords()));"                          )
    LINECPP("\t\tInit(TRUE);"                                                  )
    LINECPP("\t}"                                                              )
    LINECPP("}"                                                                )
  }
  LINECPP(""                                                                   )

  // Destructor implementation
  if (m_bCProject)
  {
    LINECPP("void ${CxxClass}_Destructor(CDlpObject* __this)"                  )
    LINECPP("{"                                                                )
    LINECPP("\tGET_THIS_VIRTUAL(${CxxClass});"                                 )
    LINECPP("\t{"                                                              )
    LINECPP("\t/*{{CGEN_DONECODE */"                                           )
    LINECPP("\t/*}}CGEN_DONECODE */"                                           )
    LINECPP("\t}"                                                              )
    LINECPP(""                                                                 )
    LINECPP("#ifndef __cplusplus"                                              )
    LINECPP(""                                                                 )
    LINECPP("\t/* Destroy base instance */"                                    )
    LINECPP("\tCDlpObject_Destructor(_this->m_lpBaseInstance);"                )
    LINECPP("\tdlp_free(_this->m_lpBaseInstance);"                             )
    LINECPP("\t_this->m_lpBaseInstance = NULL;"                                )
    LINECPP(""                                                                 )
    LINECPP("#endif /* #ifndef __cplusplus */"                                 )
    LINECPP("}"                                                                )
  }
  else
  {
    LINECPP("${CxxClass}::~${CxxClass}()"                                      )
    LINECPP("{"                                                                )
    LINECPP("  //{{CGEN_DONECODE"                                              )
    LINECPP("  //}}CGEN_DONECODE"                                              )
    LINECPP("}"                                                                )
  }
  LINECPP(""                                                                   )

  // Implementation of dictionary registration
  if (m_bCProject)
  {
    LINECPP("INT16 ${CxxClass}_AutoRegisterWords(CDlpObject* __this)"          )
    LINECPP("{"                                                                )
    LINECPP("\tGET_THIS_VIRTUAL_RV(${CxxClass},NOT_EXEC);"                     )
    LINECPP("\tDEBUGMSG(-1,\"${CxxClass}_AutoRegisterWords\",0,0,0);"          )
    LINECPP(""                                                                 )
    LINECPP("\t/* Call base class implementation */"                           )
    LINECPP("\tIF_NOK(INVOKE_BASEINST_0(AutoRegisterWords)) return NOT_EXEC;"  )
    LINECPP(""                                                                 )
    LINECPP("\t/*{{CGEN_REGISTERWORDS */"                                      )
    LINECPP("\t/*}}CGEN_REGISTERWORDS */"                                      )
    LINECPP(""                                                                 )
    LINECPP("\treturn O_K;"                                                    )
    LINECPP("}"                                                                )
  }
  else
  {
    LINECPP("INT16 ${CxxClass}::AutoRegisterWords()"                           )
    LINECPP("{"                                                                )
    LINECPP("\tDEBUGMSG(-1,\"${CxxClass}::AutoRegisterWords\",0,0,0);"         )
    LINECPP("\tIF_NOK(inherited::AutoRegisterWords()) return NOT_EXEC;"        )
    LINECPP(""                                                                 )
    LINECPP("\t//{{CGEN_REGISTERWORDS"                                         )
    LINECPP("\t//}}CGEN_REGISTERWORDS"                                         )
    LINECPP(""                                                                 )
    LINECPP("\treturn O_K;"                                                    )
    LINECPP("}"                                                                )
  }
  LINECPP(""                                                                   )

  // Implementation of the Init method
  if (m_bCProject)
  {
    LINECPP("INT16 ${CxxClass}_Init(CDlpObject* __this, BOOL bCallVirtual)"    )
    LINECPP("{"                                                                )
    LINECPP("\tGET_THIS_VIRTUAL_RV(${CxxClass},NOT_EXEC);"                     )
    LINECPP("\tDEBUGMSG(-1,\"${CxxClass}_Init, (bCallVirtual=%d)\","
            "(int)bCallVirtual,0,0);"                                          )
    LINECPP("\t{"                                                              )
    LINECPP("\t/*{{CGEN_INITCODE */"                                           )
    LINECPP("\t/*}}CGEN_INITCODE */"                                           )
    LINECPP("\t}"                                                              )
    LINECPP(""                                                                 )
    LINECPP("\t/* If last derivation call reset (do not reset members; "
            "already done by Init()) */"                                       )
    LINECPP("#ifndef __NORTTI"                                                 )
    LINECPP("\tif (bCallVirtual) return INVOKE_VIRTUAL_1(Reset,FALSE); else"   )
    LINECPP("#endif"                                                           )
    LINECPP("\t                  return O_K;"                                  )
    LINECPP("}"                                                                )
  }
  else
  {
    LINECPP("INT16 ${CxxClass}::Init(BOOL bCallVirtual)"                       )
    LINECPP("{"                                                                )
    LINECPP("\tDEBUGMSG(-1,\"${CxxClass}::Init, (bCallVirtual=%d)\","
            "(int)bCallVirtual,0,0);"                                          )
    LINECPP("\t//{{CGEN_INITCODE"                                              )
    LINECPP("\t//}}CGEN_INITCODE"                                              )
    LINECPP(""                                                                 )
    LINECPP("\t// If last derivation call reset (do not reset members; "
            "already done by Init())"                                          )
    LINECPP("\tif (bCallVirtual) return Reset(FALSE);"                         )
    LINECPP("\telse              return O_K;"                                  )
    LINECPP("}"                                                                )
  }
  LINECPP(""                                                                   )

  // Implementation of the Reset method
  if (m_bCProject)
  {
    LINECPP("INT16 ${CxxClass}_Reset(CDlpObject* __this, BOOL bResetMembers)")
    LINECPP("{"                                                                )
    LINECPP("\tGET_THIS_VIRTUAL_RV(${CxxClass},NOT_EXEC);"                     )
    LINECPP("\tDEBUGMSG(-1,\"${CxxClass}_Reset; (bResetMembers=%d)\","
            "(int)bResetMembers,0,0);"                                         )
    LINECPP("\t{"                                                              )
    LINECPP("\t/*{{CGEN_RESETCODE */"                                          )
    LINECPP("\t/*}}CGEN_RESETCODE */"                                          )
    LINECPP("\t}"                                                              )
    LINECPP(""                                                                 )
    LINECPP("\treturn O_K;"                                                    )
    LINECPP("}"                                                                )
  }
  else
  {
    LINECPP("INT16 ${CxxClass}::Reset(BOOL bResetMembers)"                     )
    LINECPP("{"                                                                )
    LINECPP("\tDEBUGMSG(-1,\"${CxxClass}::Reset; (bResetMembers=%d)\","
            "(int)bResetMembers,0,0);"                                         )
    LINECPP("\t//{{CGEN_RESETCODE"                                             )
    LINECPP("\t//}}CGEN_RESETCODE"                                             )
    LINECPP(""                                                                 )
    LINECPP("\treturn O_K;"                                                    )
    LINECPP("}"                                                                )
  }
  LINECPP(""                                                                   )

  // Implementation of the instance activation method
  if (m_bCProject)
  {
    LINECPP("INT16 ${CxxClass}_ClassProc(CDlpObject* __this)"                  )
    LINECPP("{"                                                                )
    LINECPP("\tGET_THIS_VIRTUAL_RV(${CxxClass},NOT_EXEC);"                     )
    LINECPP("\t{"                                                              )
    LINECPP("\t/*{{CGEN_CLASSCODE */"                                          )
    LINECPP("\t/*}}CGEN_CLASSCODE */"                                          )
    LINECPP("\t}"                                                              )
    LINECPP(""                                                                 )
    LINECPP("\treturn O_K;"                                                    )
    LINECPP("}"                                                                )
  }
  else
  {
    LINECPP("INT16 ${CxxClass}::ClassProc()"                                   )
    LINECPP("{"                                                                )
    LINECPP("\t//{{CGEN_CLASSCODE"                                             )
    LINECPP("\t//}}CGEN_CLASSCODE"                                             )
    LINECPP(""                                                                 )
    LINECPP("\treturn O_K;"                                                    )
    LINECPP("}"                                                                )
  }
  LINECPP(""                                                                   )

  // Instance serialization implementation
  if (m_bCProject)
  {
    LINECPP("#define CODE_DN3 /* check this for xml specific save code */"     )
    LINECPP("#define SAVE  SAVE_DN3"                                           )
    LINECPP("INT16 ${CxxClass}_Serialize(CDlpObject* __this, CDN3Stream* "
            "lpDest)"                                                          )
    LINECPP("{"                                                                )
    LINECPP("\tGET_THIS_VIRTUAL_RV(${CxxClass},NOT_EXEC);"                     )
    LINECPP("\t{"                                                              )
    LINECPP("\t/*{{CGEN_SAVECODE */"                                           )
    LINECPP("\t/*}}CGEN_SAVECODE */"                                           )
    LINECPP("\t}"                                                              )
    LINECPP(""                                                                 )
    LINECPP("\treturn O_K;"                                                    )
    LINECPP("}"                                                                )
    LINECPP("#undef  SAVE"                                                     )
    LINECPP("#undef  CODE_DN3"                                                 )
    LINECPP(""                                                                 )
    LINECPP("#define CODE_XML /* check this for xml specific save code */"     )
    LINECPP("#define SAVE  SAVE_XML"                                           )
    LINECPP("INT16 ${CxxClass}_SerializeXml(CDlpObject* __this, CXmlStream* "
            "lpDest)"                                                          )
    LINECPP("{"                                                                )
    LINECPP("\tGET_THIS_VIRTUAL_RV(${CxxClass},NOT_EXEC);"                     )
    LINECPP("\t{"                                                              )
    LINECPP("\t/*{{CGEN_SAVECODE */"                                           )
    LINECPP("\t/*}}CGEN_SAVECODE */"                                           )
    LINECPP("\t}"                                                              )
    LINECPP(""                                                                 )
    LINECPP("\treturn O_K;"                                                    )
    LINECPP("}"                                                                )
    LINECPP("#undef  SAVE"                                                     )
    LINECPP("#undef  CODE_XML"                                                 )
  }
  else
  {
    LINECPP("#define CODE_DN3 /* check this for xml specific save code */"     )
    LINECPP("#define SAVE  SAVE_DN3"                                           )
    LINECPP("INT16 ${CxxClass}::Serialize(CDN3Stream* lpDest)"                 )
    LINECPP("{"                                                                )
    LINECPP("\t//{{CGEN_SAVECODE"                                              )
    LINECPP("\t//}}CGEN_SAVECODE"                                              )
    LINECPP(""                                                                 )
    LINECPP("\treturn O_K;"                                                    )
    LINECPP("}"                                                                )
    LINECPP("#undef  SAVE"                                                     )
    LINECPP("#undef  CODE_DN3"                                                 )
    LINECPP(""                                                                 )
    LINECPP("#define CODE_XML /* check this for xml specific save code */"     )
    LINECPP("#define SAVE  SAVE_XML"                                           )
    LINECPP("INT16 ${CxxClass}::SerializeXml(CXmlStream* lpDest)"              )
    LINECPP("{"                                                                )
    LINECPP("\t//{{CGEN_SAVECODE"                                              )
    LINECPP("\t//}}CGEN_SAVECODE"                                              )
    LINECPP(""                                                                 )
    LINECPP("\treturn O_K;"                                                    )
    LINECPP("}"                                                                )
    LINECPP("#undef  SAVE"                                                     )
    LINECPP("#undef  CODE_XML"                                                 )
  }
  LINECPP(""                                                                   )

  // Instance deserialization implementation
  if (m_bCProject)
  {
    LINECPP("#define CODE_DN3 /* check this for dn3 specific restore code */"  )
    LINECPP("#define RESTORE  RESTORE_DN3"                                     )
    LINECPP("INT16 ${CxxClass}_Deserialize(CDlpObject* __this, CDN3Stream* "
            "lpSrc)"                                                           )
    LINECPP("{"                                                                )
    LINECPP("\tGET_THIS_VIRTUAL_RV(${CxxClass},NOT_EXEC);"                     )
    LINECPP("\t{"                                                              )
    LINECPP("\t/*{{CGEN_RESTORECODE */"                                        )
    LINECPP("\t/*}}CGEN_RESTORECODE */"                                        )
    LINECPP("\t}"                                                              )
    LINECPP(""                                                                 )
    LINECPP("\treturn O_K;"                                                    )
    LINECPP("}"                                                                )
    LINECPP("#undef  RESTORE"                                                  )
    LINECPP("#undef  CODE_DN3"                                                 )
    LINECPP(""                                                                 )
    LINECPP("#define CODE_XML /* check this for xml specific restore code */"  )
    LINECPP("#define RESTORE  RESTORE_XML"                                     )
    LINECPP("INT16 ${CxxClass}_DeserializeXml(CDlpObject* __this, CXmlStream* "
            "lpSrc)"                                                           )
    LINECPP("{"                                                                )
    LINECPP("\tGET_THIS_VIRTUAL_RV(${CxxClass},NOT_EXEC);"                     )
    LINECPP("\t{"                                                              )
    LINECPP("\t/*{{CGEN_RESTORECODE */"                                        )
    LINECPP("\t/*}}CGEN_RESTORECODE */"                                        )
    LINECPP("\t}"                                                              )
    LINECPP(""                                                                 )
    LINECPP("\treturn O_K;"                                                    )
    LINECPP("}"                                                                )
    LINECPP("#undef  RESTORE"                                                  )
    LINECPP("#undef  CODE_XML"                                                 )
  }
  else
  {
    LINECPP("#define CODE_DN3 /* check this for dn3 specific restore code */"  )
    LINECPP("#define RESTORE  RESTORE_DN3"                                     )
    LINECPP("INT16 ${CxxClass}::Deserialize(CDN3Stream* lpSrc)"                )
    LINECPP("{"                                                                )
    LINECPP("\t//{{CGEN_RESTORECODE"                                           )
    LINECPP("\t//}}CGEN_RESTORECODE"                                           )
    LINECPP(""                                                                 )
    LINECPP("\treturn O_K;"                                                    )
    LINECPP("}"                                                                )
    LINECPP("#undef  RESTORE"                                                  )
    LINECPP("#undef  CODE_DN3"                                                 )
    LINECPP(""                                                                 )
    LINECPP("#define CODE_XML /* check this for xml specific restore code */"  )
    LINECPP("#define RESTORE  RESTORE_XML"                                     )
    LINECPP("INT16 ${CxxClass}::DeserializeXml(CXmlStream* lpSrc)"             )
    LINECPP("{"                                                                )
    LINECPP("\t//{{CGEN_RESTORECODE"                                           )
    LINECPP("\t//}}CGEN_RESTORECODE"                                           )
    LINECPP(""                                                                 )
    LINECPP("\treturn O_K;"                                                    )
    LINECPP("}"                                                                )
    LINECPP("#undef  RESTORE"                                                  )
    LINECPP("#undef  CODE_XML"                                                 )
  }
  LINECPP(""                                                                   )

  // -- Instance copy implementation
  if (m_bCProject)
  {
    LINECPP("INT16 ${CxxClass}_Copy(CDlpObject* __this, CDlpObject* __iSrc)"   )
    LINECPP("{"                                                                )
    LINECPP("\tGET_THIS_VIRTUAL_RV(${CxxClass},NOT_EXEC);"                     )
    LINECPP("\t{"                                                              )
    LINECPP("\t/*{{CGEN_COPYCODE */"                                           )
    LINECPP("\t/*}}CGEN_COPYCODE */"                                           )
    LINECPP("\t}"                                                              )
    LINECPP(""                                                                 )
    LINECPP("\treturn O_K;"                                                    )
    LINECPP("}"                                                                )
  }
  else
  {
    LINECPP("INT16 ${CxxClass}::Copy(CDlpObject* __iSrc)"                      )
    LINECPP("{"                                                                )
    LINECPP("\t//{{CGEN_COPYCODE"                                              )
    LINECPP("\t//}}CGEN_COPYCODE"                                              )
    LINECPP(""                                                                 )
    LINECPP("\treturn O_K;"                                                    )
  LINECPP("}"                                                                  )
  }
  LINECPP(""                                                                   )
  LINECPP("${/*} Runtime class type information and class factory${*/}"        )

  // Implemetation of the class installation method
  if (m_bCProject)
  {
    LINECPP("INT16 ${CxxClass}_InstallProc(void* lpItp)"                       )
    LINECPP("{"                                                                )
    LINECPP("\t{"                                                              )
    LINECPP("\t/*{{CGEN_INSTALLCODE */"                                        )
    LINECPP("\t/*}}CGEN_INSTALLCODE */"                                        )
    LINECPP("\t}"                                                              )
    LINECPP(""                                                                 )
    LINECPP("\treturn O_K;"                                                    )
    LINECPP("}"                                                                )
  }
  else
  {
    LINECPP("INT16 ${CxxClass}::InstallProc(void* lpItp)"                      )
    LINECPP("{"                                                                )
    LINECPP("\t//{{CGEN_INSTALLCODE"                                           )
    LINECPP("\t//}}CGEN_INSTALLCODE"                                           )
    LINECPP(""                                                                 )
    LINECPP("\treturn O_K;"                                                    )
    LINECPP("}"                                                                )
  }
  LINECPP(""                                                                   )

  // Implementation of the class factory
  if (m_bCProject) LINECPP("${CxxClass}* ${CxxClass}_CreateInstance(const "
                           "char* lpName)"                                     )
  else             LINECPP("${CxxClass}* ${CxxClass}::CreateInstance(const "
                           "char* lpName)"                                     )
  LINECPP("{"                                                                  )
  LINECPP("\t${CxxClass}* lpNewInstance;"                                      )
  LINECPP("\tICREATEEX(${CxxClass},lpNewInstance,lpName,NULL);"                )
  LINECPP("\treturn lpNewInstance;"                                            )
  LINECPP("}"                                                                  )
  LINECPP(""                                                                   )

  // Implementation of the class information method
  if(m_bCProject) LINECPP("INT16 ${CxxClass}_GetClassInfo(SWord* lpClassWord)")
  else            LINECPP("INT16 ${CxxClass}::GetClassInfo(SWord* lpClassWord)")
  LINECPP("{"                                                                  )
  LINECPP("\tif (!lpClassWord) return NOT_EXEC;"                               )
  LINECPP("\tdlp_memset(lpClassWord,0,sizeof(SWord));"                         )
  LINECPP(""                                                                   )
  LINECPP("\tlpClassWord->nWordType          = WL_TYPE_FACTORY;"               )
  LINECPP("\tlpClassWord->nFlags             = ${CLStyle};"                    )
  if (m_bCProject)
  {
    LINECPP(""                                                                 )
    LINECPP("#ifdef __cplusplus"                                               )
    LINECPP(""                                                                 )
  }
  LINECPP("\tlpClassWord->ex.fct.lpfFactory  = "
          "(LP_FACTORY_PROC)${CxxClass}::CreateInstance;"                      )
  LINECPP("\tlpClassWord->ex.fct.lpfInstall  = ${CxxClass}::InstallProc;"      )
  if (m_bCProject)
  {
    LINECPP(""                                                                 )
    LINECPP("#else /* #ifdef __DLP_CSCOPE */"                                  )
    LINECPP(""                                                                 )
    LINECPP("\tlpClassWord->ex.fct.lpfFactory  = "
            "(LP_FACTORY_PROC)${CxxClass}_CreateInstance;"                     )
    LINECPP("\tlpClassWord->ex.fct.lpfInstall  = ${CxxClass}_InstallProc;"     )
    LINECPP(""                                                                 )
    LINECPP("#endif /* #ifdef __DLP_CSCOPE */"                                 )
    LINECPP(""                                                                 )
  }
  LINECPP("\tlpClassWord->ex.fct.lpProject   = \"${Proj}\";"                   )
  LINECPP("\tlpClassWord->ex.fct.lpBaseClass = \"${SLBaseName}\";"             )
  LINECPP("\tlpClassWord->lpComment          = \"${Comment}\";"                )
  LINECPP("\tlpClassWord->ex.fct.lpAutoname  = \"${AutoName}\";"               )
  LINECPP("\tlpClassWord->ex.fct.lpCname     = \"${CxxClass}\";"               )
  LINECPP("\tlpClassWord->ex.fct.lpAuthor    = \"${Author}\";"                 )
  LINECPP(""                                                                   )
  LINECPP("\tdlp_strcpy(lpClassWord->lpName             ,\"${SLName}\");"      )
  LINECPP("\tdlp_strcpy(lpClassWord->lpObsname          ,\"${ObsName}\");"     )
  LINECPP("\tdlp_strcpy(lpClassWord->ex.fct.version.no  ,\"${Version}\");"     )
  LINECPP(""                                                                   )
  LINECPP("\treturn O_K;"                                                      )
  LINECPP("}"                                                                  )
  LINECPP(""                                                                   )

  // Implementation of the instance information method
  if (m_bCProject)
  {
    LINECPP("INT16 ${CxxClass}_GetInstanceInfo(CDlpObject* __this, SWord* "
            "lpClassWord)"                                                     )
    LINECPP("{"                                                                )
    LINECPP("\treturn ${CxxClass}_GetClassInfo(lpClassWord);"                  )
    LINECPP("}"                                                                )
  }
  else
  {
    LINECPP("INT16 ${CxxClass}::GetInstanceInfo(SWord* lpClassWord)"           )
    LINECPP("{"                                                                )
    LINECPP("\treturn ${CxxClass}::GetClassInfo(lpClassWord);"                 )
    LINECPP("}"                                                                )
  }
  LINECPP(""                                                                   )

  // RTTI implementation
  if (m_bCProject)
  {
    LINECPP("BOOL ${CxxClass}_IsKindOf(CDlpObject* __this, const char* "
            "lpClassName)"                                                     )
    LINECPP("{"                                                                )
    LINECPP("\tGET_THIS_VIRTUAL_RV(${CxxClass},NOT_EXEC);"                     )
    LINECPP(""                                                                 )
    LINECPP("  if (dlp_strncmp(lpClassName,\"${SLName}\",L_NAMES) == 0) "
            "return TRUE;"                                                     )
    LINECPP("\telse return INVOKE_BASEINST_1(IsKindOf,lpClassName);"           )
    LINECPP("}"                                                                )
  }
  else
  {
    LINECPP("BOOL ${CxxClass}::IsKindOf(const char* lpClassName)"              )
    LINECPP("{"                                                                )
    LINECPP("  if (dlp_strncmp(lpClassName,\"${SLName}\",L_NAMES) == 0) "
            "return TRUE;"                                                     )
    LINECPP("  else return inherited::IsKindOf(lpClassName);"                  )
    LINECPP("}"                                                                )
  }
  LINECPP(""                                                                   )

  // Implementation of the ResetAllOptions method
  if (m_bCProject)
  {
    LINECPP("INT16 ${CxxClass}_ResetAllOptions(CDlpObject* __this, BOOL bInit)")
    LINECPP("{"                                                                )
    LINECPP("\tGET_THIS_VIRTUAL_RV(${CxxClass},NOT_EXEC);"                     )
    LINECPP("\tDEBUGMSG(-1,\"${CxxClass}_ResetAllOptions;\",0,0,0);"           )
    LINECPP("\t{"                                                              )
    LINECPP("\t/*{{CGEN_RESETALLOPTIONS*/"                                     )
    LINECPP("\t/*}}CGEN_RESETALLOPTIONS*/"                                     )
    LINECPP("\t}"                                                              )
    LINECPP(""                                                                 )
    LINECPP("\treturn INVOKE_BASEINST_1(ResetAllOptions,bInit);"               )
    LINECPP("}"                                                                )
  }
  else
  {
    LINECPP("INT16 ${CxxClass}::ResetAllOptions(BOOL bInit)"                   )
    LINECPP("{"                                                                )
    LINECPP("\tDEBUGMSG(-1,\"${CxxClass}::ResetAllOptions;\",0,0,0);"          )
    LINECPP("\t//{{CGEN_RESETALLOPTIONS"                                       )
    LINECPP("\t//}}CGEN_RESETALLOPTIONS"                                       )
    LINECPP(""                                                                 )
    LINECPP("\treturn inherited::ResetAllOptions(bInit);"                      )
    LINECPP("}"                                                                )
  }
  LINECPP(""                                                                   )

  // Generated callback functions
  if(m_bCProject) LINECPP("/* Generated primary method invocation functions */")
  else            LINECPP("// Generated primary method invocation functions"   )
  LINECPP(""                                                                   )
  LINECPP("#ifndef __NOITP"                                                    )
  if (m_bCProject)
  {
    LINECPP("/*{{CGEN_CPMIC */"                                                )
    LINECPP("/*}}CGEN_CPMIC */"                                                )
  }
  else
  {
    LINECPP("//{{CGEN_PMIC"                                                    )
    LINECPP("//}}CGEN_PMIC"                                                    )
  }
  LINECPP("#endif /* #ifndef __NOITP */"                                       )
  LINECPP(""                                                                   )
  LINECPP(""                                                                   )
  if (m_bCProject) LINECPP("/* Generated secondary method invocation "
                           "functions */"                                      )
  else             LINECPP("// Generated secondary method invocation functions")
  LINECPP(""                                                                   )
  if (m_bCProject)
  {
    LINECPP("/*{{CGEN_CSMIC */"                                                )
    LINECPP("/*}}CGEN_CSMIC */"                                                )
  }
  else
  {
    LINECPP("//{{CGEN_SMIC"                                                    )
    LINECPP("//}}CGEN_SMIC"                                                    )
  }
  LINECPP(""                                                                   )
  LINECPP(""                                                                   )
  if (m_bCProject) LINECPP("/* Generated option change callback functions */"  )
  else             LINECPP("// Generated option change callback functions"     )
  LINECPP(""                                                                   )
  if (m_bCProject)
  {
    LINECPP("/*{{CGEN_COCCF */"                                                )
    LINECPP("/*}}CGEN_COCCF */"                                                )
  }
  else
  {
    LINECPP("//{{CGEN_OCCF"                                                    )
    LINECPP("//}}CGEN_OCCF"                                                    )
  }
  LINECPP(""                                                                   )
  LINECPP(""                                                                   )
  if (m_bCProject) LINECPP("/* Generated field change callback functions */"   )
  else             LINECPP("// Generated field change callback functions"      )
  LINECPP(""                                                                   )
  if (m_bCProject)
  {
    LINECPP("/*{{CGEN_CFCCF */"                                                )
    LINECPP("/*}}CGEN_CFCCF */"                                                )
  }
  else
  {
    LINECPP("//{{CGEN_FCCF"                                                    )
    LINECPP("//}}CGEN_FCCF"                                                    )
  }
  LINECPP(""                                                                   )
  LINECPP(""                                                                   )
  if (m_bCProject)
  {
    LINECPP("/* C++ wrapper functions */"                                      )
    LINECPP("#ifdef __cplusplus"                                               )
    LINECPP(""                                                                 )
    LINECPP("#define _this this"                                               )
    LINECPP(""                                                                 )
    LINECPP("${CxxClass}::${CxxClass}(const char* lpInstanceName, BOOL "
            "bCallVirtual) : inherited(lpInstanceName,0)"                      )
    LINECPP("{"                                                                )
    LINECPP("\t${CxxClass}_Constructor(this,lpInstanceName,bCallVirtual);"     )
    LINECPP("}"                                                                )
    LINECPP(""                                                                 )
    LINECPP("${CxxClass}::~${CxxClass}()"                                      )
    LINECPP("{"                                                                )
    LINECPP("\t${CxxClass}_Destructor(this);"                                  )
    LINECPP("}"                                                                )
    LINECPP(""                                                                 )
    LINECPP("INT16 ${CxxClass}::AutoRegisterWords()"                           )
    LINECPP("{"                                                                )
    LINECPP("\treturn ${CxxClass}_AutoRegisterWords(this);"                    )
    LINECPP("}"                                                                )
    LINECPP(""                                                                 )
    LINECPP("INT16 ${CxxClass}::Init(BOOL bCallVirtual)"                       )
    LINECPP("{"                                                                )
    LINECPP("\treturn ${CxxClass}_Init(this,bCallVirtual);"                    )
    LINECPP("}"                                                                )
    LINECPP(""                                                                 )
    LINECPP("INT16 ${CxxClass}::Reset(BOOL bResetMembers)"                     )
    LINECPP("{"                                                                )
    LINECPP("\treturn ${CxxClass}_Reset(this,bResetMembers);"                  )
    LINECPP("}"                                                                )
    LINECPP(""                                                                 )
    LINECPP("INT16 ${CxxClass}::ClassProc()"                                   )
    LINECPP("{"                                                                )
    LINECPP("\treturn ${CxxClass}_ClassProc(this);"                            )
    LINECPP("}"                                                                )
    LINECPP(""                                                                 )
    LINECPP("INT16 ${CxxClass}::Serialize(CDN3Stream* lpDest)"                 )
    LINECPP("{"                                                                )
    LINECPP("\treturn ${CxxClass}_Serialize(this,lpDest);"                     )
    LINECPP("}"                                                                )
    LINECPP(""                                                                 )
    LINECPP("INT16 ${CxxClass}::SerializeXml(CXmlStream* lpDest)"              )
    LINECPP("{"                                                                )
    LINECPP("\treturn ${CxxClass}_SerializeXml(this,lpDest);"                  )
    LINECPP("}"                                                                )
    LINECPP(""                                                                 )
    LINECPP("INT16 ${CxxClass}::Deserialize(CDN3Stream* lpSrc)"                )
    LINECPP("{"                                                                )
    LINECPP("\treturn ${CxxClass}_Deserialize(this,lpSrc);"                    )
    LINECPP("}"                                                                )
    LINECPP(""                                                                 )
    LINECPP("INT16 ${CxxClass}::DeserializeXml(CXmlStream* lpSrc)"             )
    LINECPP("{"                                                                )
    LINECPP("\treturn ${CxxClass}_DeserializeXml(this,lpSrc);"                 )
    LINECPP("}"                                                                )
    LINECPP(""                                                                 )
    LINECPP("INT16 ${CxxClass}::Copy(CDlpObject* __iSrc)"                      )
    LINECPP("{"                                                                )
    LINECPP("\treturn ${CxxClass}_Copy(this,__iSrc);"                          )
    LINECPP("}"                                                                )
    LINECPP(""                                                                 )
    LINECPP("INT16 ${CxxClass}::InstallProc(void* lpItp)"                      )
    LINECPP("{"                                                                )
    LINECPP("\treturn ${CxxClass}_InstallProc(lpItp);"                         )
    LINECPP("}"                                                                )
    LINECPP(""                                                                 )
    LINECPP("${CxxClass}* ${CxxClass}::CreateInstance(const char* lpName)"     )
    LINECPP("{"                                                                )
    LINECPP("\treturn ${CxxClass}_CreateInstance(lpName);"                     )
    LINECPP("}"                                                                )
    LINECPP(""                                                                 )
    LINECPP("INT16 ${CxxClass}::GetClassInfo(SWord* lpClassWord)"              )
    LINECPP("{"                                                                )
    LINECPP("\treturn ${CxxClass}_GetClassInfo(lpClassWord);"                  )
    LINECPP("}"                                                                )
    LINECPP(""                                                                 )
    LINECPP("INT16 ${CxxClass}::GetInstanceInfo(SWord* lpClassWord)"           )
    LINECPP("{"                                                                )
    LINECPP("\treturn ${CxxClass}_GetInstanceInfo(this,lpClassWord);"          )
    LINECPP("}"                                                                )
    LINECPP(""                                                                 )
    LINECPP("BOOL ${CxxClass}::IsKindOf(const char* lpClassName)"              )
    LINECPP("{"                                                                )
    LINECPP("\treturn ${CxxClass}_IsKindOf(this,lpClassName);"                 )
    LINECPP("}"                                                                )
    LINECPP(""                                                                 )
    LINECPP("INT16 ${CxxClass}::ResetAllOptions(BOOL bInit)"                   )
    LINECPP("{"                                                                )
    LINECPP("\treturn ${CxxClass}_ResetAllOptions(this,bInit);"                )
    LINECPP("}"                                                                )
    LINECPP(""                                                                 )
    LINECPP("#ifndef __NOITP"                                                  )
    LINECPP("/*{{CGEN_PMIC */"                                                 )
    LINECPP("/*}}CGEN_PMIC */"                                                 )
    LINECPP("#endif /* #ifndef __NOITP */"                                     )
    LINECPP(""                                                                 )
    LINECPP("/*{{CGEN_SMIC */"                                                 )
    LINECPP("/*}}CGEN_SMIC */"                                                 )
    LINECPP(""                                                                 )
    LINECPP("/*{{CGEN_OCCF */"                                                 )
    LINECPP("/*}}CGEN_OCCF */"                                                 )
    LINECPP(""                                                                 )
    LINECPP("/*{{CGEN_FCCF */"                                                 )
    LINECPP("/*}}CGEN_FCCF */"                                                 )
    LINECPP(""                                                                 )
    LINECPP("/*{{CGEN_CXXWRAP */"                                              )
    LINECPP("/*}}CGEN_CXXWRAP */"                                              )
    LINECPP(""                                                                 )
    LINECPP("#endif /* #ifdef __cplusplus */"                                  )
    LINECPP(""                                                                 )
    LINECPP("/* EOF */"                                                        )
  }
  else
    LINECPP("// EOF"                                                           )
  ReplaceAllKeys2(m_cppFileTmpl);
}

// EOF
