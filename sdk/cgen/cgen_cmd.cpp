// dLabPro SDK class CCgen (cgen)
// - Definition file commands
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

INT16 CGEN_PROTECTED CCgen::OnMain()
{
  char lpsProjectname[L_INPUTLINE+1];
  GetNextDefToken(lpsProjectname);
  if (m_nAncestor > 0) return O_K;

  if (!dlp_strlen(lpsProjectname))
	return IERROR(this,ERR_EXPECTAFTER,"name","MAIN:",0);
    
  if (m_bAppend)
  {
    if (dlp_strncmp(m_lpsProject,lpsProjectname,dlp_strlen(lpsProjectname)) != 0 || !m_bMainProject)
    {
      IERROR(this,ERR_APPEND,"MAIN:",lpsProjectname,0);
      if (!dlp_strlen(m_lpsProject)) EXIT(ERR_EXPPROJECT,"/append MAIN:",0,0);
      return NOT_EXEC;
    }
  }

  // Set project code
  SetCreating(CR_PROJECT,NULL);
  m_bMainProject = TRUE;
  dlp_strcpy(m_lpsProject,lpsProjectname);  

  return O_K;
}

INT16 CGEN_PROTECTED CCgen::OnProject()
{
  char lpProjectname[L_INPUTLINE+1];
  GetNextDefToken(lpProjectname);

  if (!dlp_strlen(lpProjectname))
	return IERROR(this,ERR_EXPECTAFTER,"name","PROJECT:",0);
    
  // Add current def script to list of processed files
  char lpBuf[L_INPUTLINE+1];
  dlp_strcpy(lpBuf,GetCurrentDefFile());
  dlp_splitpath(lpBuf,NULL,lpBuf);
  m_defList.AddItem(lpBuf);

  if (m_bAppend)
  {
    if (dlp_strncmp(m_lpsProject,lpProjectname,dlp_strlen(lpProjectname)) != 0) 
    {
      IERROR(this,ERR_APPEND,"PROJECT:",lpProjectname,0);
      if (!dlp_strlen(m_lpsProject)) EXIT(ERR_EXPPROJECT,"/append PROJECT:",0,0);
      return NOT_EXEC;
    }
    SetCreating(CR_PROJECT,NULL);
    m_bMainProject = FALSE;
  }
  
  if (m_nAncestor >  1) return O_K;
  if (m_nAncestor == 1)
  {
    // Include base class' header file
    char lpBuf[255];
    dlp_convert_name(CN_HFILE,lpBuf,lpProjectname);
    dlp_convert_name(CN_QUOTE,lpBuf,lpBuf);
    m_includes.AddItem(lpBuf);

    dlp_convert_name(CN_DLP2CXX_CLSN,m_lpsCxxParent,lpProjectname);             // Store C++ base class name
    dlp_strcpy(m_lpsParentProject,lpProjectname);                               // Store parent project name
    return O_K;
  }

  // Set project code
  SetCreating(CR_PROJECT,NULL);
  m_bMainProject = FALSE;
  dlp_strcpy(m_lpsProject,lpProjectname);  

  // Get package name
  // TODO: Make a more educated guess
  if (!m_bAppend && dlp_strlen(GetCurrentDefFile())>0)
  {
    dlp_strcpy(lpBuf,GetCurrentDefFile());
    char* tx;
    int   n;
    for (n=0,tx=&lpBuf[dlp_strlen(lpBuf)-1]; tx>=lpBuf; tx--)
    {
      if (*tx=='\\') *tx='/';
      if (*tx=='/')
      {
        n++;
        if (n==2) *tx='\0';
        if (n==4) { tx++; break; }
      }
    }
    dlp_strcpy(m_lpsPackage,tx);
    if (dlp_strnicmp(m_lpsPackage,"dlabpro/",8)==0)                             // Normalize heading "dlabpro"
      dlp_strncpy(m_lpsPackage,"dLabPro",7);                                    // ...
  }

  return O_K;
}

INT16 CGEN_PROTECTED CCgen::OnClass()
{
  char lpClassname[L_INPUTLINE+1];
  GetNextDefToken(lpClassname);

  if (m_nAncestor > 1) return O_K;

  if (!dlp_strlen(lpClassname))
    return IERROR(this,ERR_EXPECTAFTER,"name","CLASS:",0);

  char bad = 0;
  if (!m_bNoIdcheck)
    IF_NOK(dlp_is_valid_id(IDT_CLASS,lpClassname,&bad)) 
      IERROR(this,ERR_BADIDCHAR,lpClassname,bad,0);
    
  if (m_nAncestor == 1) dlp_strcpy(m_lpsDlcParent,lpClassname);
  else                  dlp_strcpy(m_lpsClass,lpClassname);  
  return O_K;
}

INT16 CGEN_PROTECTED CCgen::OnSuperClass()
{
  char lpsDefFile[L_INPUTLINE+1];
  GetNextDefToken(lpsDefFile);

  if (!dlp_strlen(lpsDefFile))
    ERRORRET(ERR_EXPECTAFTER,"file name","SUPERCLASS:",0,NOT_EXEC);
  if (m_bMainProject)
    ERRORRET(ERR_INVALMETHOD,"SUPERCLASS:","MAIN:",0,NOT_EXEC);

  m_nAncestor++;
  IF_NOK(Include(lpsDefFile))
  {
    m_nAncestor--;
    return NOT_EXEC;
  }
  return O_K;
}

INT16 CGEN_PROTECTED CCgen::OnAutoinstance()
{
  char lpAutoname[L_INPUTLINE+1];
  GetNextDefToken(lpAutoname);

  if (m_bMainProject)
    return IERROR(this,ERR_INVALKEY,"AUTOINSTANCE:","MAIN:",0);
  if (m_nAncestor) return O_K;

  if (!dlp_strlen(lpAutoname))
    return IERROR(this,ERR_EXPECTIDAFTER,"name","AUTOINSTANCE:",0);

  dlp_strcpy(m_lpsAutoname,lpAutoname);
  m_nCSXXX |= CS_AUTOINSTANCE;

    return O_K;
}

INT16 CGEN_PROTECTED CCgen::OnParent()
{
  const char* lpParent = GetRestOfDefLine();

  if (m_bMainProject)
    return IERROR(this,ERR_INVALKEY,"PARENT:","MAIN:",0);
  if (m_nAncestor) return O_K;

  if (!dlp_strlen(lpParent))
    return IERROR(this,ERR_EXPECTAFTER,"name list","PARENT:",0);

  char* tx = (char*)strstr(lpParent,"C(");
  if (tx)
  {
    tx+=2;
    char* ty = strstr(tx,")");
    if (ty) *ty = 0;
    char lpBuf[100]; dlp_strncpy(lpBuf,tx,100);
    do dlp_strtrimleft(lpBuf); while (lpBuf[0] == ',');
    sprintf(m_lpsCxxMoreParents,", %s",lpBuf);
  }

  return O_K;
}

INT16 CGEN_PROTECTED CCgen::OnAuthor()
{
  const char* lpAuthor = GetRestOfDefLine();

  if (m_nAncestor) return O_K;

  if (!dlp_strlen(lpAuthor)) ERRORRET(ERR_EXPECTAFTER,"author name","AUTHOR:",0,NOT_EXEC);

  dlp_strcpy(m_lpsAuthor,lpAuthor);  
  return O_K;
}

INT16 CGEN_PROTECTED CCgen::OnVersion()
{
  const char* lpVersion = GetRestOfDefLine();

  if (m_nAncestor) return O_K;

  if (!dlp_strlen(lpVersion))
    ERRORRET(ERR_EXPECTAFTER,"version name","VERSION:",0,NOT_EXEC);
  if (dlp_strlen(lpVersion)>=sizeof(m_lpsVersion))
    IERROR(this,ERR_TOOLONG,GetCurrentDefFile(),GetCurrentDefLine(),"Version name");

  dlp_strncpy(m_lpsVersion,lpVersion,sizeof(m_lpsVersion));
  m_lpsVersion[sizeof(m_lpsVersion)-1]='\0';
  return O_K;
}

INT16 CGEN_PROTECTED CCgen::OnPlatform()
{
  char lpPlatform[L_INPUTLINE+1];
  GetNextDefToken(lpPlatform);

  if (m_nAncestor) return O_K;

  if (!dlp_strlen(lpPlatform)) ERRORRET(ERR_EXPECTAFTER,"platform name","PLATFORM:",0,NOT_EXEC);

  m_nPlatform = 0;
  if (dlp_strncmp(lpPlatform,"GNUC++",L_NAMES) ==0)  m_nPlatform = PF_GNUCXX;
  if (dlp_strncmp(lpPlatform,"MSDEV4",L_NAMES) ==0)  m_nPlatform = PF_MSDEV4;
  if (dlp_strncmp(lpPlatform,"MSDEV5",L_NAMES) ==0)  m_nPlatform = PF_MSDEV5;
  if (dlp_strncmp(lpPlatform,"MSDEV6",L_NAMES) ==0)  m_nPlatform = PF_MSDEV6;

  if (dlp_strncmp(lpPlatform,"MSDEV",L_NAMES) ==0)
  {
    ERRORMSG(ERR_OBSOLETEID,"MSDEV","MSDEV4",0);
    m_nPlatform = PF_MSDEV4;
  }

  if (dlp_strncmp(lpPlatform,"MSVC++",L_NAMES) ==0)
  {
    // Detect MSVC++ version
    m_nPlatform = QueryMSVCVersion(m_lpsMSDPath);
    if (!m_nPlatform) ERRORRET(ERR_NOMSVCFOUND,0,0,0,ERR_NOMSVCFOUND);
  }

  if (!m_nPlatform) ERRORRET(ERR_NOPLATFORM,lpPlatform,0,0,NOT_EXEC);  

  return O_K;
}

INT16 CGEN_PROTECTED CCgen::OnGccFlags()
{
  const char* lpFlags = GetRestOfDefLine();
  if (m_nAncestor) return O_K;
  if (!dlp_strlen(lpFlags))
    ERRORRET(ERR_EXPECTAFTER,"compiler options","GCCFLAGS:",0,NOT_EXEC);
  dlp_strcpy(m_lpsGccFlags,lpFlags);
  return O_K;
}

INT16 CGEN_PROTECTED CCgen::OnMsvcFlags()
{
  const char* lpFlags = GetRestOfDefLine();
  if (m_nAncestor) return O_K;
  if (!dlp_strlen(lpFlags))
    ERRORRET(ERR_EXPECTAFTER,"compiler options","MSVCFLAGS:",0,NOT_EXEC);
  dlp_strcpy(m_lpsMsvcFlags,lpFlags);
  return O_K;
}

INT16 CGEN_PROTECTED CCgen::OnLfile()
{
  char lpName[L_INPUTLINE+1];
  GetNextDefToken(lpName);
  if (!dlp_strlen(lpName)) ERRORRET(ERR_EXPECTAFTER,"name","LFILE:",0,NOT_EXEC);

  switch(m_nCreating)
  {
  case CR_PROJECT: {
    if (m_nAncestor) return O_K;
    if (m_bMainProject)
    {
      char lpBuf[L_PATH];
      dlp_convert_name(CN_XLATPATH,lpBuf,lpName);
      m_lFiles.AddItem(lpBuf);
    }
    else dlp_strcpy(m_lpsLibfile,lpName);
    break;}
  case CR_ERROR:
    return IERROR(this,ERR_INVALSUBKEY,"ERROR:","LFILE:",0);
  case CR_FIELD:
    return IERROR(this,ERR_INVALSUBKEY,"FIELD:","LFILE:",0);
  case CR_OPTION:
    return IERROR(this,ERR_INVALSUBKEY,"OPTION:","LFILE:",0);
  case CR_METHOD:
    return IERROR(this,ERR_INVALSUBKEY,"METHOD:","LFILE:",0);
  case CR_NOTE:
    return IERROR(this,ERR_INVALSUBKEY,"NOTE:","LFILE:",0);
  case CR_ICLS: {
    SCGIcl* s = m_lpCreatingIcl; DLPASSERT(s);
    dlp_strcpy(s->lpLFile,lpName);
    break; }
  default:
    return IERROR(this,ERR_EXPGLOBALKEY,"LFILE:",0,0);
  }

  return O_K;
}

INT16 CGEN_PROTECTED CCgen::OnHfile()
{
  char lpName[L_INPUTLINE+1];
  GetNextDefToken(lpName);
  if (!dlp_strlen(lpName)) ERRORRET(ERR_EXPECTAFTER,"name","HFILE:",0,NOT_EXEC);

  switch(m_nCreating)
  {
  case CR_PROJECT:
    return IERROR(this,ERR_INVALSUBKEY,"PROJECT:","HFILE:",0);
  case CR_ERROR:
    return IERROR(this,ERR_INVALSUBKEY,"ERROR:","HFILE:",0);
  case CR_FIELD:
    return IERROR(this,ERR_INVALSUBKEY,"FIELD:","HFILE:",0);
  case CR_OPTION:
    return IERROR(this,ERR_INVALSUBKEY,"OPTION:","HFILE:",0);
  case CR_METHOD:
    return IERROR(this,ERR_INVALSUBKEY,"METHOD:","HFILE:",0);
  case CR_NOTE:
    return IERROR(this,ERR_INVALSUBKEY,"NOTE:","HFILE:",0);
  case CR_ICLS   : {
    SCGIcl* s = m_lpCreatingIcl; DLPASSERT(s);
    dlp_strcpy(s->lpHFile,lpName);
    break; }
  default:
    return IERROR(this,ERR_EXPGLOBALKEY,"HFILE:",0,0);
  }

  return O_K;
}

INT16 CGEN_PROTECTED CCgen::OnFile()
{
  char lpFile[L_INPUTLINE];
  GetNextDefToken(lpFile);
  dlp_strconvert(SC_UNESCAPE,lpFile,lpFile);
  dlp_strtrimleft(lpFile);
  dlp_strtrimright(lpFile);

  if (m_bMainProject) ERRORRET(ERR_INVALKEY,"FILE:","MAIN:",0,NOT_EXEC);
  if (m_nAncestor) return O_K;

  INT16 nFType = 0;

  if (!dlp_strlen(lpFile)) ERRORRET(ERR_EXPECTAFTER,"file name","FILE:",0,NOT_EXEC);
  if (m_files.FindItem(lpFile)) ERRORRET(ERR_DDEF_LIST,"FILE",lpFile,0,NOT_EXEC);

  char *ext = &lpFile[strlen(lpFile)-1];
  while (*ext != '.' && ext != lpFile) ext--;
  if (dlp_strncmp(ext,".cpp",L_NAMES) ==0) nFType = FT_CPP;
  if (dlp_strncmp(ext,".c"  ,L_NAMES) ==0) nFType = FT_C;
  if (dlp_strncmp(ext,".h"  ,L_NAMES) ==0) nFType = FT_H;
  if (!nFType) ERRORRET(ERR_FILETYPE,ext,NULL,0,NOT_EXEC);

  SCGFile* nf = m_files.AddItem(lpFile);
  if (!nf) return NOT_EXEC;
  nf->nFType = nFType;

  return O_K;
}

INT16 CGEN_PROTECTED CCgen::OnInclude()
{
  char lpInclude[L_INPUTLINE+1];
  GetNextDefToken(lpInclude);

  if (m_nAncestor) return O_K;

  if (!dlp_strlen(lpInclude)) ERRORRET(ERR_EXPECTAFTER,"file name","INCLUDE:",0,NOT_EXEC);
  if (m_includes.FindItem(lpInclude)) ERRORRET(ERR_DDEF_LIST,"INCLUDE",lpInclude,0,NOT_EXEC);

  if (m_includes.AddItem(lpInclude)) return O_K; else return NOT_EXEC;
}

INT16 CGEN_PROTECTED CCgen::OnPinclude()
{
  char lpPinclude[L_INPUTLINE+1];
  GetNextDefToken(lpPinclude);

  if (m_bMainProject) ERRORRET(ERR_INVALKEY,"PINCLUDE:","MAIN:",0,NOT_EXEC);
  if (m_nAncestor) return O_K;

  if (!dlp_strlen(lpPinclude)) ERRORRET(ERR_EXPECTAFTER,"file name","PINCLUDE:",0,NOT_EXEC);
  if (m_pincludes.FindItem(lpPinclude)) ERRORRET(ERR_DDEF_LIST,"PINCLUDE",lpPinclude,0,NOT_EXEC);

  if (m_pincludes.AddItem(lpPinclude)) return O_K; else return NOT_EXEC;
}

INT16 CGEN_PROTECTED CCgen::OnDefine()
{
  const char* lpDefine = GetRestOfDefLine();

  if (m_nAncestor) return O_K;

  if (!dlp_strlen(lpDefine)) ERRORRET(ERR_EXPECTAFTER,"directive","DEFINE:",0,NOT_EXEC);
  if (m_defines.FindItem(lpDefine)) ERRORRET(ERR_DDEF_LIST,"DEFINE",lpDefine,0,NOT_EXEC);

  if (m_defines.AddItem(lpDefine)) return O_K; else return NOT_EXEC;
}

INT16 CGEN_PROTECTED CCgen::OnFriend()
{
  char lpFriend[L_INPUTLINE+1];
  GetNextDefToken(lpFriend);

  if (m_nAncestor) return O_K;

  if (!dlp_strlen(lpFriend)) ERRORRET(ERR_EXPECTAFTER,"identifier","FRIEND:",0,NOT_EXEC);
  if (m_friends.FindItem(lpFriend)) ERRORRET(ERR_DDEF_LIST,"FRIEND",lpFriend,0,NOT_EXEC);

  if (m_friends.AddItem(lpFriend)) return O_K; else return NOT_EXEC;
}

INT16 CGEN_PROTECTED CCgen::OnMpath()
{
  char lpName[L_INPUTLINE+1];
  GetNextDefToken(lpName);
  if (!dlp_strlen(lpName)) ERRORRET(ERR_EXPECTAFTER,"name","MPATH:",0,NOT_EXEC);

  switch(m_nCreating)
  {
  case CR_PROJECT: {
    if (m_nAncestor) return O_K;
    else             dlp_convert_name(CN_XLATPATH,m_lpsMPath,lpName);
    break;}
  case CR_ERROR:
    return IERROR(this,ERR_INVALSUBKEY,"ERROR:","MPATH:",0);
  case CR_FIELD:
    return IERROR(this,ERR_INVALSUBKEY,"FIELD:","MPATH:",0);
  case CR_OPTION:
    return IERROR(this,ERR_INVALSUBKEY,"OPTION:","MPATH:",0);
  case CR_METHOD:
    return IERROR(this,ERR_INVALSUBKEY,"METHOD:","MPATH:",0);
  case CR_NOTE:
    return IERROR(this,ERR_INVALSUBKEY,"NOTE:","MPATH:",0);
  case CR_ICLS: {
    SCGIcl* s = m_lpCreatingIcl; DLPASSERT(s);
    dlp_strcpy(s->lpMPath,lpName);
    break; }
  default:
    return IERROR(this,ERR_EXPGLOBALKEY,"MPATH:",0,0);
  }

  return O_K;
}

INT16 CGEN_PROTECTED CCgen::OnIpath()
{
  char lpName[L_INPUTLINE+1];
  GetNextDefToken(lpName);
  if (!dlp_strlen(lpName)) ERRORRET(ERR_EXPECTAFTER,"name","IPATH:",0,NOT_EXEC);

  switch(m_nCreating)
  {
  case CR_PROJECT: {
    if (m_nAncestor) return O_K;
    else             dlp_convert_name(CN_XLATPATH,m_lpsIPath,lpName);
    break;}
  case CR_ERROR:
    return IERROR(this,ERR_INVALSUBKEY,"ERROR:","IPATH:",0);
  case CR_FIELD:
    return IERROR(this,ERR_INVALSUBKEY,"FIELD:","IPATH:",0);
  case CR_OPTION:
    return IERROR(this,ERR_INVALSUBKEY,"OPTION:","IPATH:",0);
  case CR_METHOD:
    return IERROR(this,ERR_INVALSUBKEY,"METHOD:","IPATH:",0);
  case CR_NOTE:
    return IERROR(this,ERR_INVALSUBKEY,"NOTE:","IPATH:",0);
  case CR_ICLS: {
    SCGIcl* s = m_lpCreatingIcl; DLPASSERT(s);
    dlp_strcpy(s->lpIPath,lpName);
    break; }
  default:
    return IERROR(this,ERR_EXPGLOBALKEY,"IPATH:",0,0);
  }

  return O_K;
}

INT16 CGEN_PROTECTED CCgen::OnOpath()
{
  char lpName[L_INPUTLINE+1];
  GetNextDefToken(lpName);
  if (!dlp_strlen(lpName)) ERRORRET(ERR_EXPECTAFTER,"name","OPATH:",0,NOT_EXEC);

  switch(m_nCreating)
  {
  case CR_PROJECT: {
    if (m_nAncestor) return O_K;
    dlp_convert_name(CN_XLATPATH,m_lpsOPath,lpName);
    break;}
  case CR_ERROR:
    return IERROR(this,ERR_INVALSUBKEY,"ERROR:","OPATH:",0);
  case CR_FIELD:
    return IERROR(this,ERR_INVALSUBKEY,"FIELD:","OPATH:",0);
  case CR_OPTION:
    return IERROR(this,ERR_INVALSUBKEY,"OPTION:","OPATH:",0);
  case CR_METHOD:
    return IERROR(this,ERR_INVALSUBKEY,"METHOD:","OPATH:",0);
  case CR_NOTE:
    return IERROR(this,ERR_INVALSUBKEY,"NOTE:","OPATH:",0);
  case CR_ICLS:
    return IERROR(this,ERR_INVALSUBKEY,"ICLS:","OPATH:",0);
  default:
    return IERROR(this,ERR_EXPGLOBALKEY,"OPATH:",0,0);
  }

  return O_K;
}

INT16 CGEN_PROTECTED CCgen::OnLpath()
{
  char lpName[L_INPUTLINE+1];
  GetNextDefToken(lpName);
  if (!dlp_strlen(lpName)) ERRORRET(ERR_EXPECTAFTER,"name","LPATH:",0,NOT_EXEC);

  switch(m_nCreating)
  {
  case CR_PROJECT: {
    if (m_nAncestor) return O_K;
    else             dlp_convert_name(CN_XLATPATH,m_lpsLPath,lpName);
    break;}
  case CR_ERROR:
    return IERROR(this,ERR_INVALSUBKEY,"ERROR:","LPATH:",0);
  case CR_FIELD:
    return IERROR(this,ERR_INVALSUBKEY,"FIELD:","LPATH:",0);
  case CR_OPTION:
    return IERROR(this,ERR_INVALSUBKEY,"OPTION:","LPATH:",0);
  case CR_METHOD:
    return IERROR(this,ERR_INVALSUBKEY,"METHOD:","LPATH:",0);
  case CR_NOTE:
    return IERROR(this,ERR_INVALSUBKEY,"NOTE:","LPATH:",0);
  case CR_ICLS: {
    SCGIcl* s = m_lpCreatingIcl; DLPASSERT(s);
    dlp_strcpy(s->lpLPath,lpName);
    break; }
  default:
    return IERROR(this,ERR_EXPGLOBALKEY,"LPATH:",0,0);
  }

  return O_K;
}

INT16 CGEN_PROTECTED CCgen::OnHeadercode()
{
  if (m_nAncestor || m_bMainProject)
  {
    CList<SCGStr> dummy;
    FillIn(dummy,S_ENDCODE);
    if (m_bMainProject) ERRORRET(ERR_INVALKEY,"HEADERCODE:","MAIN:",0,NOT_EXEC)
    else                return O_K;
  }

  FillIn(m_headerCode,S_ENDCODE);
  return O_K;
}

INT16 CGEN_PROTECTED CCgen::OnInitcode()
{
  if (m_nAncestor || m_bMainProject)
  {
    CList<SCGStr> dummy;
    FillIn(dummy,S_ENDCODE);
    if (m_bMainProject) ERRORRET(ERR_INVALKEY,"INITCODE:","MAIN:",0,NOT_EXEC)
    else                return O_K;
  }

  FillIn(m_initCode,S_ENDCODE);
  return O_K;
}

INT16 CGEN_PROTECTED CCgen::OnDonecode()
{
  if (m_nAncestor || m_bMainProject)
  {
    CList<SCGStr> dummy;
    FillIn(dummy,S_ENDCODE);
    if (m_bMainProject) ERRORRET(ERR_INVALKEY,"DONECODE:","MAIN:",0,NOT_EXEC)
    else                return O_K;
  }

  FillIn(m_doneCode,S_ENDCODE);
  return O_K;
}

INT16 CGEN_PROTECTED CCgen::OnResetcode()
{
  if (m_nAncestor || m_bMainProject)
  {
    CList<SCGStr> dummy;
    FillIn(dummy,S_ENDCODE);
    if (m_bMainProject) ERRORRET(ERR_INVALKEY,"RESETCODE:","MAIN:",0,NOT_EXEC)
    else                return O_K;
  }

  FillIn(m_resetCode,S_ENDCODE);
  return O_K;
}

INT16 CGEN_PROTECTED CCgen::OnSavecode()
{
  if (m_nAncestor || m_bMainProject)
  {
    CList<SCGStr> dummy;
    FillIn(dummy,S_ENDCODE);
    if (m_bMainProject) ERRORRET(ERR_INVALKEY,"SAVECODE:","MAIN:",0,NOT_EXEC)
    else                return O_K;
  }

  FillIn(m_saveCode,S_ENDCODE);
  return O_K;
}

INT16 CGEN_PROTECTED CCgen::OnRestorecode()
{
  if (m_nAncestor || m_bMainProject)
  {
    CList<SCGStr> dummy;
    FillIn(dummy,S_ENDCODE);
    if (m_bMainProject) ERRORRET(ERR_INVALKEY,"RESTORECODE:","MAIN:",0,NOT_EXEC)
    else                return O_K;
  }

  FillIn(m_restoreCode,S_ENDCODE);
  return O_K;
}

INT16 CGEN_PROTECTED CCgen::OnCopycode()
{
  if (m_nAncestor || m_bMainProject)
  {
    CList<SCGStr> dummy;
    FillIn(dummy,S_ENDCODE);
    if (m_bMainProject) ERRORRET(ERR_INVALKEY,"COPYCODE:","MAIN:",0,NOT_EXEC)
    else                return O_K;
  }

  FillIn(m_copyCode,S_ENDCODE);
  return O_K;
}

INT16 CGEN_PROTECTED CCgen::OnClasscode()
{
  if (m_nAncestor || m_bMainProject)
  {
    CList<SCGStr> dummy;
    FillIn(dummy,S_ENDCODE);
    if (m_bMainProject) ERRORRET(ERR_INVALKEY,"CLASSCODE:","MAIN:",0,NOT_EXEC)
    else                return O_K;
  }

  FillIn(m_classCode,S_ENDCODE);
  return O_K;
}

INT16 CGEN_PROTECTED CCgen::OnInstallcode()
{
  if (m_nAncestor || m_bMainProject)
  {
    CList<SCGStr> dummy;
    FillIn(dummy,S_ENDCODE);
    if (m_bMainProject) ERRORRET(ERR_INVALKEY,"INSTALLCODE:","MAIN:",0,NOT_EXEC)
    else                return O_K;
  }

  FillIn(m_installCode,S_ENDCODE);
  return O_K;
}

INT16 CGEN_PROTECTED CCgen::OnError()
{
  SetCreating(CR_IGNORE,NULL);

  char lpName[L_INPUTLINE+1];
  GetNextDefToken(lpName);

  if (m_bMainProject) ERRORRET(ERR_INVALKEY,"ERROR:","MAIN:",0,NOT_EXEC);

  if (!dlp_strlen(lpName)) ERRORRET(ERR_EXPECTAFTER,"name","ERROR:",0,NOT_EXEC);
  if (!dlp_strlen(m_lpsProject)) EXIT(ERR_EXPPROJECT,"ERROR:",0,0);
  if (m_bAppend)
  {
    SCGErr* lpErr = m_errors.FindItem(lpName);
    if (!lpErr) ERRORRET(ERR_APPEND,"ERROR:",lpName,0,NOT_EXEC);
    SetCreating(CR_ERROR,lpErr);
    return O_K;
  }
  if (m_errors.FindItem(lpName)) ERRORRET(ERR_DDEF_LIST,"ERROR",lpName,0,NOT_EXEC);

  char bad = 0;
  if (!m_bNoIdcheck)
    IF_NOK(dlp_is_valid_id(IDT_ERROR,lpName,&bad)) 
      ERRORMSG(ERR_BADIDCHAR,lpName,bad,0);

  SCGErr* lpErr = m_errors.AddItem(lpName);
  if (!lpErr) ERRORRET(ERR_CANNOTCREATE,"ERROR",lpName,0,NOT_EXEC);

  dlp_strncpy(lpErr->lpFile,GetCurrentDefFile(),L_PATH);
  lpErr->nLine = GetCurrentDefLine();

  lpErr->bInherited = (m_nAncestor > 0);
  SetCreating(CR_ERROR,lpErr);

  return O_K;
}

INT16 CGEN_PROTECTED CCgen::OnField()
{
  SetCreating(CR_IGNORE,NULL);

  char lpName[L_INPUTLINE+1];
  GetNextDefToken(lpName);

  if (m_bMainProject) ERRORRET(ERR_INVALKEY,"FIELD:","MAIN:",0,NOT_EXEC);

  if (!dlp_strlen(lpName)) ERRORRET(ERR_EXPECTAFTER,"name","FIELD:",0,NOT_EXEC);
  if (!dlp_strlen(m_lpsProject)) EXIT(ERR_EXPPROJECT,"FIELD:",0,0);
  if (m_bAppend)
  {
    SCGFld* lpFld = m_flds.FindItem(lpName);
    if (!lpFld) ERRORRET(ERR_APPEND,"FIELD:",lpName,0,NOT_EXEC);
    SetCreating(CR_FIELD,lpFld);
    return O_K;
  }

  if (m_flds.FindItem(lpName)) ERRORRET(ERR_DDEF_LIST,"FIELD",lpName,0,NOT_EXEC);

  char bad = 0;
  if (!m_bNoIdcheck)
    IF_NOK(dlp_is_valid_id(IDT_FIELD,lpName,&bad)) 
      ERRORMSG(ERR_BADIDCHAR,lpName,bad,0);

  SCGFld* lpFld = m_flds.AddItem(lpName);
  if (!lpFld) ERRORRET(ERR_CANNOTCREATE,"FIELD",lpName,0,NOT_EXEC);

  dlp_strncpy(lpFld->lpFile,GetCurrentDefFile(),L_PATH);
  lpFld->nLine = GetCurrentDefLine();

  lpFld->nType = -1;
  lpFld->bInherited = (m_nAncestor > 0);
  SetCreating(CR_FIELD,lpFld);

  return O_K;
}

INT16 CGEN_PROTECTED CCgen::OnOption()
{
  SetCreating(CR_IGNORE,NULL);

  char lpName[L_INPUTLINE+1];
  GetNextDefToken(lpName);

  if (m_bMainProject) ERRORRET(ERR_INVALKEY,"OPTION:","MAIN:",0,NOT_EXEC);

  if (!dlp_strlen(lpName)) ERRORRET(ERR_EXPECTAFTER,"name","OPTION:",0,NOT_EXEC);
  if (!dlp_strlen(m_lpsProject)) EXIT(ERR_EXPPROJECT,"OPTION:",0,0);
  if (m_bAppend)
  {
    SCGOpt* lpOpt = m_opts.FindItem(lpName);
    if (!lpOpt) ERRORRET(ERR_APPEND,"OPTION:",lpName,0,NOT_EXEC);
    SetCreating(CR_OPTION,lpOpt);
    return O_K;
  }
  if (m_opts.FindItem(lpName)) ERRORRET(ERR_DDEF_LIST,"OPTION",lpName,0,NOT_EXEC);

  if (!m_bNoIdcheck)
  {
    char bad;
    switch(dlp_is_valid_id(IDT_OPTION,lpName,&bad))
    {
    case IVIR_BADCHAR: ERRORMSG(ERR_BADIDCHAR,lpName,bad     ,0  ); break;
    case IVIR_BADLEAD: ERRORMSG(ERR_BADLEAD  ,lpName,"option",bad); break;
    }
  }


  SCGOpt* lpOpt = m_opts.AddItem(lpName);
  if (!lpOpt) ERRORRET(ERR_CANNOTCREATE,"OPTION",lpName,0,NOT_EXEC);

  dlp_strncpy(lpOpt->lpFile,GetCurrentDefFile(),L_PATH);
  lpOpt->nLine = GetCurrentDefLine();

  lpOpt->bInherited = (m_nAncestor > 0);
  SetCreating(CR_OPTION,lpOpt);

  return O_K;
}

INT16 CGEN_PROTECTED CCgen::OnMethod()
{
  SetCreating(CR_IGNORE,NULL);

  char lpName[L_INPUTLINE+1];
  GetNextDefToken(lpName);

  if (m_bMainProject) ERRORRET(ERR_INVALKEY,"METHOD:","MAIN:",0,NOT_EXEC);

  if (!dlp_strlen(lpName)) ERRORRET(ERR_EXPECTAFTER,"name","METHOD:",0,NOT_EXEC);
  if (!dlp_strlen(m_lpsProject)) EXIT(ERR_EXPPROJECT,"METHOD:",0,0);

  if (m_bAppend)
  {
    SCGMth* lpMth = m_mths.FindItem(lpName);
    if (!lpMth) ERRORRET(ERR_APPEND,"METHOD:",lpName,0,NOT_EXEC);
    SetCreating(CR_METHOD,lpMth);
    return O_K;
  }

  if (!m_bNoIdcheck)
  {
    char bad;
    switch(dlp_is_valid_id(IDT_METHOD,lpName,&bad))
    {
    case IVIR_BADCHAR: ERRORMSG(ERR_BADIDCHAR,lpName,bad     ,0  ); break;
    case IVIR_BADLEAD: ERRORMSG(ERR_BADLEAD  ,lpName,"method",bad); break;
    }
  }

  SCGMth* lpMth = m_mths.FindItem(lpName);
  if (lpMth)
  {
    if (!lpMth->bInherited)
      // Not allowed twice in one file
      {ERRORRET(ERR_DDEF_LIST,"METHOD",lpName,0,NOT_EXEC);}
    else
      // Overwriting is allowed
      // NOTE: The overwritten method will inherit all 
      //       properties not defined else!
      lpMth->bOverwritten = TRUE;
  }

  if (!lpMth) 
  {
    lpMth = m_mths.AddItem(lpName);
    lpMth->bOverwritten = FALSE;
  }
  if (!lpMth) ERRORRET(ERR_CANNOTCREATE,"METHOD",lpName,0,NOT_EXEC);

  dlp_strncpy(lpMth->lpFile,GetCurrentDefFile(),L_PATH);
  lpMth->nLine = GetCurrentDefLine();

  lpMth->bInherited = (m_nAncestor > 0);
  SetCreating(CR_METHOD,lpMth);
  
  return O_K;
}

INT16 CGEN_PROTECTED CCgen::OnNote()
{
  SetCreating(CR_IGNORE,NULL);

  const char* lpName = GetRestOfDefLine();
  if (!dlp_strlen(lpName)) ERRORRET(ERR_EXPECTAFTER,"title","NOTE:",0,NOT_EXEC);

  if (m_bAppend)
  {
    SCGRnt* lpRnt = m_rnts.FindItem(lpName);
    if (!lpRnt) ERRORRET(ERR_APPEND,"NOTE:",lpName,0,NOT_EXEC);
    SetCreating(CR_NOTE,lpRnt);
    return O_K;
  }

  if (m_rnts.FindItem(lpName)) ERRORRET(ERR_DDEF_LIST,"NOTE",lpName,0,NOT_EXEC);

  SCGRnt* lpRnt = m_rnts.AddItem(lpName);
  if (!lpRnt) ERRORRET(ERR_CANNOTCREATE,"NOTE",lpName,0,NOT_EXEC);

  dlp_strncpy(lpRnt->lpFile,GetCurrentDefFile(),L_PATH);
  lpRnt->nLine = GetCurrentDefLine();

  lpRnt->bInherited = (m_nAncestor > 0);
  SetCreating(CR_NOTE,lpRnt);
  
  return O_K;
}

INT16 CGEN_PROTECTED CCgen::OnInstallClass()
{
  SetCreating(CR_IGNORE,NULL);

  char lpName[L_INPUTLINE+1];
  GetNextDefToken(lpName);
  char lpsLwrName[L_NAMES];

  if (!m_bMainProject) ERRORRET(ERR_INVALKEY,"INSTALL:","PROJECT:",0,NOT_EXEC);

  if (!dlp_strlen(lpName)) ERRORRET(ERR_EXPECTAFTER,"name","INSTALL:",0,NOT_EXEC);
  if (!dlp_strlen(m_lpsProject)) EXIT(ERR_EXPPROJECT,"INSTALL:",0,0);

  if (m_bAppend)
  {
    SCGIcl* lpIcl = m_icls.FindItem(lpName);
    if (!lpIcl) ERRORRET(ERR_APPEND,"INSTALL:",lpName,0,NOT_EXEC);
    SetCreating(CR_ICLS,lpIcl);
    return O_K;
  }

  if (m_icls.FindItem(lpName)) ERRORRET(ERR_DDEF_LIST,"INSTALL",lpName,0,NOT_EXEC);

  SCGIcl* lpIcl = m_icls.AddItem(lpName);
  if (!lpIcl) ERRORRET(ERR_CANNOTCREATE,"INSTALL",lpName,0,NOT_EXEC);

  dlp_strncpy(lpIcl->lpFile,GetCurrentDefFile(),L_PATH);
  lpIcl->nLine = GetCurrentDefLine();

  // -- Initialize file and path specifications
  dlp_convert_name(CN_HFILE,lpIcl->lpHFile,lpName);
  dlp_strcpy(lpsLwrName,lpName);
  dlp_strlwr(lpsLwrName);
  switch (m_nPlatform)
  {
    case PF_MSDEV4:
    case PF_MSDEV5:
    case PF_MSDEV6: sprintf(lpIcl->lpLFile,"%s.lib",lpsLwrName); break;
    default       : sprintf(lpIcl->lpLFile,"%s.a",lpsLwrName);
  }
  
//  dlp_convert_name(CN_XLATPATH,lpIcl->lpIPath,m_lpIPath);
  dlp_convert_name(CN_XLATPATH,lpIcl->lpLPath,m_lpsLPath);

  SetCreating(CR_ICLS,lpIcl);

  return O_K;
}

INT16 CGEN_PROTECTED CCgen::OnObsolete()
{
  char lpName[L_INPUTLINE+1];
  GetNextDefToken(lpName);
  if (!dlp_strlen(lpName))
    return IERROR(this,ERR_EXPECTAFTER,"name","OBSOLETE:",0);

  switch(m_nCreating)
  {
  case CR_PROJECT: {
    if (m_bMainProject) ERRORMSG(ERR_INVALKEY,"OBSOLETE:","MAIN:",0);
    else dlp_strcpy(m_lpsObsolete,lpName); 
    break;}
  case CR_ERROR:
    return IERROR(this,ERR_INVALSUBKEY,"ERROR:","OBSOLETE:",0);
  case CR_FIELD: {
    SCGFld* lpFld = m_lpCreatingFld; DLPASSERT(lpFld);
    dlp_strcpy(lpFld->lpObsolete,lpName);
    break;}
  case CR_OPTION: {
    SCGOpt* lpOpt = m_lpCreatingOpt; DLPASSERT(lpOpt);
    dlp_strcpy(lpOpt->lpObsolete,lpName);
    break;}
  case CR_METHOD: {
    SCGMth* lpMth = m_lpCreatingMth; DLPASSERT(lpMth);
    dlp_strcpy(lpMth->lpObsolete,lpName);
    break;}
  case CR_NOTE:
    return IERROR(this,ERR_INVALSUBKEY,"NOTE:","OBSOLETE:",0);
  case CR_ICLS:
    return IERROR(this,ERR_INVALSUBKEY,"ICLS:","OBSOLETE:",0);
  default:
    return IERROR(this,ERR_EXPGLOBALKEY,"OBSOLETE:",0,0);
  }

  return O_K;
}

INT16 CGEN_PROTECTED CCgen::OnComment()
{
  const char* lpText = GetRestOfDefLine();
  if (!dlp_strlen(lpText)) ERRORRET(ERR_EXPECTAFTER,"text","COMMENT:",0,NOT_EXEC);

  switch(m_nCreating)
  {
  case CR_PROJECT: {
    if (m_nAncestor==0)
      dlp_strcpy(m_lpsComment,lpText); 
    break;}
  case CR_ERROR: {
    SCGErr* lpErr = m_lpCreatingErr; DLPASSERT(lpErr);
    dlp_strcpy(lpErr->lpComment,lpText);
    dlp_strconvert(SC_UNESCAPE,lpErr->lpComment,lpErr->lpComment);
    break;}
  case CR_FIELD: {
    SCGFld* lpFld = m_lpCreatingFld; DLPASSERT(lpFld);
    dlp_strcpy(lpFld->lpComment,lpText);
    dlp_strconvert(SC_UNESCAPE,lpFld->lpComment,lpFld->lpComment);
    break;}
  case CR_OPTION: {
    SCGOpt* lpOpt = m_lpCreatingOpt; DLPASSERT(lpOpt);
    dlp_strcpy(lpOpt->lpComment,lpText);
    dlp_strconvert(SC_UNESCAPE,lpOpt->lpComment,lpOpt->lpComment);
    break;}
  case CR_METHOD: {
    SCGMth* lpMth = m_lpCreatingMth; DLPASSERT(lpMth);
    dlp_strcpy(lpMth->lpComment,lpText);
    dlp_strconvert(SC_UNESCAPE,lpMth->lpComment,lpMth->lpComment);
    break;}
  case CR_NOTE: {
    SCGRnt* lpRnt = m_lpCreatingRnt; DLPASSERT(lpRnt);
    dlp_strcpy(lpRnt->lpComment,lpText);
    dlp_strconvert(SC_UNESCAPE,lpRnt->lpComment,lpRnt->lpComment);
    break;}
  case CR_ICLS:
    return IERROR(this,ERR_INVALSUBKEY,"INSTALL:","COMMENT:",0);
  default:
    return IERROR(this,ERR_EXPGLOBALKEY,"COMMENT:",0,0);
  }

  return O_K;
}

INT16 CGEN_PROTECTED CCgen::OnCname()
{
  char lpName[L_INPUTLINE+1];
  GetNextDefToken(lpName);
  if (!dlp_strlen(lpName))
    return IERROR(this,ERR_EXPECTAFTER,"name","CNAME:",0);

  switch(m_nCreating)
  {
  case CR_PROJECT: {
    if (m_bMainProject) ERRORMSG(ERR_INVALKEY,"CNAME:","MAIN:",0);
    else dlp_strcpy(m_lpsCName,lpName);
    break;}
  case CR_ERROR:
    return IERROR(this,ERR_INVALSUBKEY,"ERROR:","CNAME:",0);
  case CR_FIELD: {
    SCGFld* lpFld = m_lpCreatingFld; DLPASSERT(lpFld);
    if (dlp_strcmp(lpName,"0")==0) dlp_strcpy(lpFld->lpCName,"NULL"); // Field remapped
    else                           dlp_strcpy(lpFld->lpCName,lpName);
    break;}
  case CR_OPTION: {
    SCGOpt* lpOpt = m_lpCreatingOpt; DLPASSERT(lpOpt);
    dlp_strcpy(lpOpt->lpCName,lpName);
    break;}
  case CR_METHOD: {
    SCGMth* lpMth = m_lpCreatingMth; DLPASSERT(lpMth);
    dlp_strcpy(lpMth->lpCName,lpName);
    break;}
  case CR_NOTE:
    return IERROR(this,ERR_INVALSUBKEY,"NOTE:","CNAME:",0);
  case CR_ICLS:
    return IERROR(this,ERR_INVALSUBKEY,"ICLS:","CNAME:",0);
  default:
   return IERROR(this,ERR_EXPGLOBALKEY,"CNAME:",0,0);
  }

  return O_K;
}

INT16 CGEN_PROTECTED CCgen::OnCstructname()
{
  char lpName[L_INPUTLINE+1];
  GetNextDefToken(lpName);
  if (!dlp_strlen(lpName)) ERRORRET(ERR_EXPECTAFTER,"name","CSTRUCTNAME:",0,NOT_EXEC);

  switch(m_nCreating)
  {
  case CR_PROJECT: {
    if (m_bMainProject) ERRORMSG(ERR_INVALKEY,"CSTRUCTNAME:","MAIN:",0);
    else dlp_strcpy(m_lpsCStructName,lpName);
    break;}
  case CR_ERROR:
    return IERROR(this,ERR_INVALSUBKEY,"ERROR:","CSTRUCTNAME:",0);
  case CR_FIELD:
    return IERROR(this,ERR_INVALSUBKEY,"FIELD:","CSTRUCTNAME:",0);
  case CR_OPTION:
    return IERROR(this,ERR_INVALSUBKEY,"OPTION:","CSTRUCTNAME:",0);
  case CR_METHOD:
    return IERROR(this,ERR_INVALSUBKEY,"METHOD:","CSTRUCTNAME:",0);
  case CR_NOTE:
    return IERROR(this,ERR_INVALSUBKEY,"NOTE:","CSTRUCTNAME:",0);
  case CR_ICLS:
    return IERROR(this,ERR_INVALSUBKEY,"INSTALL:","CSTRUCTNAME:",0);
  default:
    return IERROR(this,ERR_EXPGLOBALKEY,"CSTRUCTNAME:",0,0);
  }

  return O_K;
}

INT16 CGEN_PROTECTED CCgen::OnType()
{
  const char* lpText = GetRestOfDefLine();
  if (!dlp_strlen(lpText)) ERRORRET(ERR_EXPECTAFTER,"type id","TYPE:",0,NOT_EXEC);

  switch(m_nCreating)
  {
  case CR_PROJECT:
    return IERROR(this,ERR_INVALSUBKEY,"PROJECT:","TYPE:",0);
  case CR_ERROR:
    return IERROR(this,ERR_INVALSUBKEY,"ERROR:","TYPE:",0);
  case CR_FIELD: {
    SCGFld* lpFld = m_lpCreatingFld; DLPASSERT(lpFld);
    char  lpBuf[255]; dlp_strcpy(lpBuf,lpText);
    char* lpArray = strstr(lpBuf,"[");
    if (lpArray) *lpArray++=0;

    // Analyze type code
    char* tx;
    dlp_strcpy(lpFld->lpArraySpec,""    );
    dlp_strcpy(lpFld->lpType     ,"NULL");
    lpFld->nArrayLen = 1;
    lpFld->nType = dlp_get_type_code(lpBuf);
    if (lpFld->nType > 0)
    {
      if (lpFld->nType <= 256) 
      {
        lpFld->nArrayLen = lpFld->nType;
        dlp_strcpy(lpFld->lpCType,"char");
      }
      else if (lpFld->nType == T_TEXT || lpFld->nType == T_STRING)
      {
        dlp_strcpy(lpFld->lpCType,"char*");
      }
      else dlp_strcpy(lpFld->lpCType,dlp_get_c_type(lpFld->nType));
      dlp_strcpy(lpFld->lpType,dlp_get_type_name(lpFld->nType));
    }
    else if (dlp_strncmp(lpBuf,"data",L_NAMES) ==0)
    {
      lpFld->nType = T_INSTANCE;
      dlp_strcpy(lpFld->lpCType,"CData*");
      dlp_strcpy(lpFld->lpType,"data");
    }
    else if (dlp_strncmp(lpBuf,"structure",L_NAMES) ==0)
    {
      lpFld->nType = T_INSTANCE;
      dlp_strcpy(lpFld->lpCType,"CStr*");
      dlp_strcpy(lpFld->lpType,"structure");
    }
    else if (dlp_strncmp(lpBuf,"DATA",L_NAMES) ==0)
    {
      lpFld->nType = T_INSTANCE;
      dlp_strcpy(lpFld->lpCType,"CData*");
      dlp_strcpy(lpFld->lpType,"data");
    }
    else if (dlp_strncmp(lpBuf,"STRUCTURE",L_NAMES) ==0)
    {
      lpFld->nType = T_INSTANCE;
      dlp_strcpy(lpFld->lpCType,"CStr*");
      dlp_strcpy(lpFld->lpType,"structure");
    }
    else if ((tx = strstr(lpBuf,"INSTANCE")))
    {
      if (tx != lpBuf) ERRORMSG(ERR_PARSEERR,"field type","INSTANCE",0);
      tx+=8;
      if (*tx=='(') tx++; else ERRORMSG(ERR_EXPECT,"("," after INSTANCE",0);
      char* ty = strstr(tx,")");
      if (!ty) ERRORMSG(ERR_EXPECT,")"," after 'INSTANCE('",0);
      else {*ty++=0; if (*ty) ERRORMSG(ERR_PARSEERR,"field type",lpBuf,0);}
      dlp_strtrimleft(tx); dlp_strtrimright(tx);
      dlp_strconvert(SC_UNESCAPE,tx,tx);
      dlp_strtrimleft(tx); dlp_strtrimright(tx);
      if (dlp_strlen(tx)) sprintf(lpFld->lpCType,"%s*",tx);
      else                sprintf(lpFld->lpCType,"CDlpObject*");
      dlp_strcpy(lpFld->lpType,tx);
      lpFld->nType = T_INSTANCE;
    }
    else if (lpBuf[0]=='(' && lpBuf[dlp_strlen(lpBuf)-1] == ')')
    {
      // T_IGNORE!
      lpFld->nType = T_IGNORE;
      lpBuf[dlp_strlen(lpBuf)-1] = 0;
      dlp_strcpy(lpFld->lpCType,&lpBuf[1]);
      dlp_strcpy(lpFld->lpType ,&lpBuf[1]);
    }
    else
    {
      if (lpBuf[dlp_strlen(lpBuf)-1] != '*') ERRORMSG(ERR_UNKNOWNTYPE,lpBuf,0,0);
      lpFld->nType = T_PTR;

      char lpBuf1[L_NAMES];
      INT16 nType;

      dlp_strcpy(lpBuf1,lpBuf);
      *(strchr(lpBuf1,'*')) = '\0';
      nType = dlp_get_type_code(lpBuf1);

      if(nType < 0) {
        dlp_strcpy(lpFld->lpCType,lpBuf);
      } else {
        dlp_strcpy(lpFld->lpCType, dlp_get_c_type(nType));
        for(INT16 i = (INT16)(strrchr(lpBuf,'*')-strchr(lpBuf,'*')); i>=0; i--) {
          dlp_strcat(lpFld->lpCType, "*");
        }
      }
      dlp_strcpy(lpFld->lpType ,lpBuf);
    }

    // Analyze array length
    if (lpArray && dlp_strlen(lpArray))
    {
      // Write it down, whatever it may be!
      sprintf(lpFld->lpArraySpec,"[%s",lpArray);
      tx = strstr(lpArray,"]");
      if (tx)
      {
        *tx++=0;
        long nArrayLen;
        if (!sscanf(lpArray,"%ld",&nArrayLen))
        {
          ERRORMSG(ERR_INVALARRLEN,tx,0,0);
          lpFld->nArrayLen = 1;
        }
        lpFld->nArrayLen = nArrayLen;
        if (*tx)
        {
          ERRORMSG(ERR_MULTIARRAY,0,0,0);
        }
      }
      else ERRORMSG(ERR_EXPECT,"]"," after '[' in array specification",0);
    }
    else if (lpFld->nArrayLen > 1) sprintf(lpFld->lpArraySpec,"[%ld]",(long)lpFld->nArrayLen);
    break;}
  case CR_OPTION:
    return IERROR(this,ERR_INVALSUBKEY,"OPTION:","TYPE:",0);
  case CR_METHOD:
    return IERROR(this,ERR_INVALSUBKEY,"METHOD:","TYPE:",0);
  case CR_NOTE:
    return IERROR(this,ERR_INVALSUBKEY,"NOTE:","TYPE:",0);
  case CR_ICLS:
    return IERROR(this,ERR_INVALSUBKEY,"INSTALL:","TYPE:",0);
  default:
    return IERROR(this,ERR_EXPGLOBALKEY,"TYPE:",0,0);
  }

  return O_K;
}

INT16 CGEN_PROTECTED CCgen::OnInit()
{
 char lpText[L_INPUTLINE+1];
 GetNextDefToken(lpText);
  if (!dlp_strlen(lpText)) ERRORRET(ERR_EXPECTAFTER,"text","INIT:",0,NOT_EXEC);

  switch(m_nCreating)
  {
  case CR_PROJECT:
    return IERROR(this,ERR_INVALSUBKEY,"PROJECT:","INIT:",0);
  case CR_ERROR:
    return IERROR(this,ERR_INVALSUBKEY,"ERROR:","INIT:",0);
  case CR_FIELD: {
    SCGFld* lpFld = m_lpCreatingFld; DLPASSERT(lpFld);
    if (lpFld->nType <=0) ERRORRET(ERR_EXPECT,"TYPE:"," before 'INIT:'",0,NOT_EXEC);
    if (dlp_is_pointer_type_code(lpFld->nType)) 
    {
      ERRORMSG(ERR_POINTERINIT,lpFld->lpName,0,0);
      dlp_strcpy(lpFld->lpInit,"NULL");
    }
    dlp_strcpy(lpFld->lpInit,lpText);
    if ((lpFld->nType == T_STRING)           || 
        (lpFld->nType == T_TEXT)             || 
        (lpFld->nType>0 && lpFld->nType <=256) )
    {
      dlp_convert_name(CN_QUOTE,lpFld->lpInit,lpFld->lpInit);
    }
    break;}
  case CR_OPTION:
    return IERROR(this,ERR_INVALSUBKEY,"OPTION:","INIT:",0);
  case CR_METHOD:
    return IERROR(this,ERR_INVALSUBKEY,"METHOD:","INIT:",0);
  case CR_NOTE:
    return IERROR(this,ERR_INVALSUBKEY,"NOTE:","INIT:",0);
  case CR_ICLS:
    return IERROR(this,ERR_INVALSUBKEY,"INSTALL:","INIT:",0);
  default:
    return IERROR(this,ERR_EXPGLOBALKEY,"INIT:",0,0);
  }

  return O_K;
}

INT16 CGEN_PROTECTED CCgen::OnSyntax()
{
  char lpText[L_INPUTLINE+1];
  dlp_strcpy(lpText,GetRestOfDefLine());
  if (!dlp_strlen(lpText)) ERRORRET(ERR_EXPECTAFTER,"text","SYNTAX:",0,NOT_EXEC);

  switch(m_nCreating)
  {
  case CR_PROJECT:
    return IERROR(this,ERR_INVALSUBKEY,"PROJECT:","SYNTAX:",0);
  case CR_ERROR:
    return IERROR(this,ERR_INVALSUBKEY,"ERROR:","SYNTAX:",0);
  case CR_FIELD:
    return IERROR(this,ERR_INVALSUBKEY,"FIELD:","SYNTAX:",0);
  case CR_OPTION:
    return IERROR(this,ERR_INVALSUBKEY,"OPTION:","SYNTAX:",0);
  case CR_METHOD: {
    SCGMth* lpMth = m_lpCreatingMth; DLPASSERT(lpMth);
    AnalyzeSyntax(lpMth->lpName,lpMth->lSyntax,lpText,&lpMth->lpReturn,lpMth->lpUPNSyntax,lpMth->lpCSyntax);
    break;}
  case CR_NOTE:
    return IERROR(this,ERR_INVALSUBKEY,"NOTE:","SYNTAX:",0);
  case CR_ICLS:
    return IERROR(this,ERR_INVALSUBKEY,"INSTALL:","SYNTAX:",0);
  default:
    return IERROR(this,ERR_EXPGLOBALKEY,"SYNTAX:",0,0);
  }

  return O_K;
}

INT16 CGEN_PROTECTED CCgen::OnPostsyn()
{
  const char* lpText = GetRestOfDefLine();
  if (!dlp_strlen(lpText)) ERRORRET(ERR_EXPECTAFTER,"text","POSTSYN:",0,NOT_EXEC);

  switch(m_nCreating)
  {
  case CR_PROJECT:
    return IERROR(this,ERR_INVALSUBKEY,"PROJECT:","POSTSYN:",0);
  case CR_ERROR:
    return IERROR(this,ERR_INVALSUBKEY,"ERROR:","POSTSYN:",0);
  case CR_FIELD:
    return IERROR(this,ERR_INVALSUBKEY,"FIELD:","POSTSYN:",0);
  case CR_OPTION:
    return IERROR(this,ERR_INVALSUBKEY,"OPTION:","POSTSYN:",0);
  case CR_METHOD: {
    SCGMth* lpMth = m_lpCreatingMth; DLPASSERT(lpMth);
    dlp_strcpy(lpMth->lpPostsyn,lpText);
    dlp_strconvert(SC_UNESCAPE,lpMth->lpPostsyn,lpMth->lpPostsyn);
    break;}
  case CR_NOTE:
    return IERROR(this,ERR_INVALSUBKEY,"NOTE:","POSTSYN:",0);
  case CR_ICLS:
    return IERROR(this,ERR_INVALSUBKEY,"INSTALL:","POSTSYN:",0);
  default:
    return IERROR(this,ERR_EXPGLOBALKEY,"POST:",0,0);
  }

  return O_K;
}

INT16 CGEN_PROTECTED CCgen::OnLevel()
{
  char lpText[L_INPUTLINE+1];
  GetNextDefToken(lpText);

  if (!dlp_strlen(lpText)) ERRORRET(ERR_EXPECTAFTER,"text","LEVEL:",0,NOT_EXEC);

  switch(m_nCreating)
  {
  case CR_PROJECT:
    return IERROR(this,ERR_INVALSUBKEY,"PROJECT:","LEVEL:",0);
  case CR_ERROR  : {
    SCGErr* lpErr = m_lpCreatingErr; DLPASSERT(lpErr);
    dlp_strncpy(lpErr->lpErrorLevel,"EL_ERROR",L_NAMES);
    if (dlp_strncmp(lpText,"EL_WARNING",L_NAMES) == 0)       dlp_strncpy(lpErr->lpErrorLevel,"EL_WARNING" ,L_NAMES);
    else if (dlp_strncmp(lpText,"EL_WARNING2",L_NAMES) == 0) dlp_strncpy(lpErr->lpErrorLevel,"EL_WARNING2",L_NAMES);
    else if (dlp_strncmp(lpText,"EL_WARNING3",L_NAMES) == 0) dlp_strncpy(lpErr->lpErrorLevel,"EL_WARNING3",L_NAMES);
    else if (dlp_strncmp(lpText,"EL_FATAL",L_NAMES) == 0)    dlp_strncpy(lpErr->lpErrorLevel,"EL_FATAL"   ,L_NAMES);
    else if (dlp_strncmp(lpText,"EL_ERROR",L_NAMES) != 0)  ERRORRET(ERR_EXPECTAFTER,"error level","LEVEL:",0,FALSE);
    break;}
  case CR_FIELD:
    return IERROR(this,ERR_INVALSUBKEY,"FIELD:","LEVEL:",0);
  case CR_OPTION:
    return IERROR(this,ERR_INVALSUBKEY,"OPTION:","LEVEL:",0);
  case CR_METHOD:
    return IERROR(this,ERR_INVALSUBKEY,"METHOD:","LEVEL:",0);
  case CR_NOTE:
    return IERROR(this,ERR_INVALSUBKEY,"NOTE:","LEVEL:",0);
  case CR_ICLS:
    return IERROR(this,ERR_INVALSUBKEY,"INSTALL:","LEVEL:",0);
  default:
    return IERROR(this,ERR_EXPGLOBALKEY,"LEVEL:",0,0);
  }

  return O_K;
}

INT16 CGEN_PROTECTED CCgen::OnFlags()
{
  char lpText[L_INPUTLINE+1];
  dlp_strcpy(lpText,GetRestOfDefLine());
  if (!dlp_strlen(lpText)) ERRORRET(ERR_EXPECTAFTER,"flag identifier(s)","FLAGS:",0,NOT_EXEC);
  if (m_bMainProject) ERRORRET(ERR_INVALKEY,"FLAGS:","MAIN:",0,NOT_EXEC);

  INT32 nFF = 0;         // Flags on fields
  INT32 nOF = 0;         // Flags on options
  INT32 nMF = 0;         // Flags on methods
  INT32 nCF = m_nCSXXX;  // Flags on classes
  
  // Scan lpText for flag identifiers
  char* tx = strtok(lpText," ;,\n\r\t");
  while (tx)
  {
    if      (!strcmp(tx,"/hidden"         )) nFF |= FF_HIDDEN;
    else if (!strcmp(tx,"/noset"          )) nFF |= FF_NOSET;
    else if (!strcmp(tx,"/nosave"         )) nFF |= FF_NOSAVE;
    else if (!strcmp(tx,"/nonautomatic"   )) {nFF |= FF_NONAUTOMATIC; nOF |= OF_NONAUTOMATIC;}
    else if (!strcmp(tx,"/noautoactivate" )) nCF &= ~CS_AUTOACTIVATE;
    else if (!strcmp(tx,"/singleton"      )) nCF |= CS_SINGLETON;
    else if (!strcmp(tx,"/secondary"      )) nCF |= CS_SECONDARY;
    else if (!strcmp(tx,"/nondistinct"    )) nMF |= MF_NONDISTINCT;
    else if (!strcmp(tx,"/jealous"        ))
    {
      IERROR(this,ERR_DEPRECATED,"/jealous","Use /singleton instead!",0);
      nCF |= CS_SINGLETON;
    }

    // Bad flag
    else ERRORMSG(ERR_BADIDENTIFIER,"Flag",tx,0);
    tx = strtok(NULL," ;,\n\r\t");
  }

  // Implement the flags
  switch(m_nCreating)
  {
  case CR_PROJECT: {
    m_nCSXXX = nCF; 
    FORBIDFLAG(0,nFF,FF_HIDDEN      ,"PROJECT:","/hidden"      );
    FORBIDFLAG(0,nFF,FF_NOSET       ,"PROJECT:","/noset"       );
    FORBIDFLAG(0,nFF,FF_NOSAVE      ,"PROJECT:","/nosave"      );
    FORBIDFLAG(0,nFF,FF_NONAUTOMATIC,"PROJECT:","/nonAutomatic");
    FORBIDFLAG(0,nMF,MF_NONDISTINCT ,"PROJECT:","/nondistinct" );
    break;}
  case CR_ERROR:
    return IERROR(this,ERR_INVALSUBKEY,"ERROR:","FLAGS:",0);
  case CR_FIELD: {
    SCGFld* lpFld = m_lpCreatingFld; DLPASSERT(lpFld);
    lpFld->nFlags = nFF;
    FORBIDFLAG(m_nCSXXX,nCF,CS_AUTOACTIVATE,"FIELD:","/noautoactivate");
    FORBIDFLAG(m_nCSXXX,nCF,CS_SINGLETON   ,"FIELD:","/singleton"     );
    FORBIDFLAG(m_nCSXXX,nCF,CS_SECONDARY   ,"FIELD:","/secondary"     );
    FORBIDFLAG(0       ,nMF,MF_NONDISTINCT ,"FIELD:","/nondistinct"   );
    break;}
  case CR_OPTION: {
    SCGOpt* lpOpt = m_lpCreatingOpt; DLPASSERT(lpOpt);
    lpOpt->nFlags = nOF;
    FORBIDFLAG(m_nCSXXX,nCF,CS_AUTOACTIVATE,"OPTION:","/noautoactivate");
    FORBIDFLAG(m_nCSXXX,nCF,CS_SINGLETON   ,"OPTION:","/singleton"     );
    FORBIDFLAG(m_nCSXXX,nCF,CS_SECONDARY   ,"OPTION:","/secondary"     );
    FORBIDFLAG(0       ,nFF,FF_HIDDEN      ,"OPTION:","/hidden"        );
    FORBIDFLAG(0       ,nFF,FF_NOSET       ,"OPTION:","/noset"         );
    FORBIDFLAG(0       ,nFF,FF_NOSAVE      ,"OPTION:","/nosave"        );
    FORBIDFLAG(0       ,nMF,MF_NONDISTINCT ,"OPTION:","/nondistinct"   );
    break;}
  case CR_METHOD: {
    SCGMth* lpMth = m_lpCreatingMth; DLPASSERT(lpMth);
    lpMth->nFlags = nMF;
    FORBIDFLAG(m_nCSXXX,nCF,CS_AUTOACTIVATE,"METHOD:","/noautoactivate");
    FORBIDFLAG(m_nCSXXX,nCF,CS_SINGLETON   ,"METHOD:","/singleton"     );
    FORBIDFLAG(m_nCSXXX,nCF,CS_SECONDARY   ,"METHOD:","/secondary"     );
    FORBIDFLAG(0       ,nFF,FF_HIDDEN      ,"METHOD:","/hidden"        );
    FORBIDFLAG(0       ,nFF,FF_NOSET       ,"METHOD:","/noset"         );
    FORBIDFLAG(0       ,nFF,FF_NOSAVE      ,"METHOD:","/nosave"        );
    FORBIDFLAG(0       ,nFF,FF_NONAUTOMATIC,"METHOD:","/nonAutomatic"  );
    break;}
  case CR_NOTE:
    return IERROR(this,ERR_INVALSUBKEY,"NOTE:","FLAGS:",0);
  case CR_ICLS:
    return IERROR(this,ERR_INVALSUBKEY,"INSTALL:","FLAGS:",0);
  default:
    return IERROR(this,ERR_EXPGLOBALKEY,"FLAGS:",0,0);
  }

  return O_K;
}
    
INT16 CGEN_PROTECTED CCgen::OnCode()
{
  CList<SCGStr> dummy;
  switch (m_nCreating)
  {
  case CR_PROJECT:
    FillIn(dummy,S_ENDCODE);
    return IERROR(this,ERR_INVALSUBKEY,"PROJECT:","CODE:",0);
  case CR_ERROR:
    FillIn(dummy,S_ENDCODE);
    return IERROR(this,ERR_INVALSUBKEY,"ERROR:","CODE:",0);
  case CR_FIELD: {
    SCGFld* lpFld = m_lpCreatingFld; DLPASSERT(lpFld);
    FillIn(lpFld->lCode,S_ENDCODE);
    break;}
  case CR_OPTION : {
    SCGOpt* lpOpt = m_lpCreatingOpt; DLPASSERT(lpOpt);
    FillIn(lpOpt->lCode,S_ENDCODE);
    break;}
  case CR_METHOD: {
    SCGMth* lpMth = m_lpCreatingMth; DLPASSERT(lpMth);
    if (m_bPrimary) FillIn(lpMth->lPCode,S_ENDCODE);
    else            FillIn(lpMth->lCode,S_ENDCODE);
    break;}
  case CR_NOTE:
    FillIn(dummy,S_ENDCODE);
    return IERROR(this,ERR_INVALSUBKEY,"NOTE:","CODE:",0);
  case CR_ICLS:
    FillIn(dummy,S_ENDCODE);
    return IERROR(this,ERR_INVALSUBKEY,"INSTALL:","CODE:",0);
  default:
    FillIn(dummy,S_ENDCODE);
    return IERROR(this,ERR_EXPGLOBALKEY,"CODE:",0,0);
  }

  return O_K;
}

INT16 CGEN_PROTECTED CCgen::OnMan()
{
  CList<SCGStr> lMan;
  const char* lpDefFile = GetCurrentDefFile();
  INT32 nDefLine = GetCurrentDefLine();

  /*
  if (m_nAncestor>0)                                                            // Base class
  {                                                                             // >>
    FillInManualText(lMan);                                                     //   Ignore manual text
    return O_K;                                                                 //   ...
  }                                                                             // <<
  */
  switch (m_nCreating)
  {
    case CR_PROJECT:
    {
      m_bHtmlMan = m_bHtml;
      FillIn(m_bHtml?lMan:m_manual,S_ENDMAN);
      if (m_bHtml) JavaDoc2Html(&m_manual,NULL,&lMan,lpDefFile,nDefLine);
      break;
    }
    case CR_ERROR:
    {
      SCGErr* lpErr = m_lpCreatingErr; DLPASSERT(lpErr);
      lpErr->bHtmlMan = m_bHtml;
      FillIn(m_bHtml?lMan:lpErr->lMan,S_ENDMAN);
      if (m_bHtml) JavaDoc2Html(&lpErr->lMan,NULL,&lMan,lpDefFile,nDefLine);
      break;
    }
    case CR_FIELD:
    {
      SCGFld* lpFld = m_lpCreatingFld; DLPASSERT(lpFld);
      lpFld->bHtmlMan = m_bHtml;
      FillIn(m_bHtml?lMan:lpFld->lMan,S_ENDMAN);
      if (m_bHtml) JavaDoc2Html(&lpFld->lMan,NULL,&lMan,lpDefFile,nDefLine);
      break;
    }
    case CR_OPTION:
    {
      SCGOpt* lpOpt = m_lpCreatingOpt; DLPASSERT(lpOpt);
      lpOpt->bHtmlMan = m_bHtml;
      FillIn(m_bHtml?lMan:lpOpt->lMan,S_ENDMAN);
      if (m_bHtml) JavaDoc2Html(&lpOpt->lMan,NULL,&lMan,lpDefFile,nDefLine);
      break;
    }
    case CR_METHOD:
    {
      SCGMth* lpMth = m_lpCreatingMth; DLPASSERT(lpMth);
      lpMth->bHtmlMan = m_bHtml;
      FillIn(m_bHtml?lMan:lpMth->lMan,S_ENDMAN);
      if (m_bHtml) JavaDoc2Html(&lpMth->lMan,NULL,&lMan,lpDefFile,nDefLine);
      break;
    }
    case CR_NOTE:
    {
      SCGRnt* lpRnt = m_lpCreatingRnt; DLPASSERT(lpRnt);
      lpRnt->bHtmlMan = m_bHtml;
      FillIn(m_bHtml?lMan:lpRnt->lMan,S_ENDMAN);
      if (m_bHtml) JavaDoc2Html(&lpRnt->lMan,NULL,&lMan,lpDefFile,nDefLine);
      break;
    }
    case CR_ICLS:
    {
      FillIn(lMan,S_ENDMAN);
      ERRORRET(ERR_INVALSUBKEY,"INSTALL:","MAN:",0,NOT_EXEC);
      break;
    }
    default:
    {
      FillIn(lMan,S_ENDMAN);
      ERRORRET(ERR_EXPGLOBALKEY,"MAN:",0,0,NOT_EXEC);
      break;
    }
  }

  return O_K;
}

INT16 CGEN_PROTECTED CCgen::Template()
{
  char lpName[L_INPUTLINE+1];
  GetNextDefToken(lpName);
  if (!dlp_strlen(lpName)) ERRORRET(ERR_EXPECTAFTER,"name","-template",0,NOT_EXEC);

  if (dlp_strncmp(lpName,"h",100) ==0)
  {
    printf("\n Actual state of h-File Template:\n");
    m_hFileTmpl.PrintList();
    return O_K;
  }
  if (dlp_strncmp(lpName,"cpp",100) ==0)
  {
    printf("\n Actual state of cpp-File Template:\n");
    m_cppFileTmpl.PrintList();
    return O_K;
  }
  return NOT_EXEC;
}

INT16 CGEN_PROTECTED CCgen::Listwords()
{
  char lpName[L_INPUTLINE+1];
  GetNextDefToken(lpName);
  if (!dlp_strlen(lpName)) ERRORRET(ERR_EXPECTAFTER,"name","-listwords",0,NOT_EXEC);

  INT16 all = (lpName ==NULL);
  // Source files
  if (dlp_strncmp(lpName,"files",100) ==0 || all)
  {
    printf("\n--- list of registered files -----------\n");
    m_files.PrintList();
  }
  // Defines
  if (dlp_strncmp(lpName,"defines",100) ==0 || all)
  {
    printf("\n--- list of registered defines ---------\n");
    m_defines.PrintList();
  }
  // Includes
  if (dlp_strncmp(lpName,"includes",100) ==0 || all)
  {
    printf("\n--- list of registered include files ---\n");
    m_includes.PrintList();
  }
  // Friends
  if (dlp_strncmp(lpName,"friends",100) ==0 || all)
  {
    printf("\n--- list of registered friend classes ---\n");
    m_friends.PrintList();
  }
  // Errors
  if (dlp_strncmp(lpName,"errors",100) ==0 || all)
  {
    printf("\n--- list of registered errors ----------\n");
    m_errors.PrintList();
  }
  // Fields
  if (dlp_strncmp(lpName,"fields",100) ==0 || all)
  {
    printf("\n--- list of registered fields ------\n");
    m_flds.PrintList();
  }
  // Methods
  if (dlp_strncmp(lpName,"methods",100) ==0 || all)
  {
    printf("\n--- list of registered methods --------\n");
    m_mths.PrintList();
  }
  // Functions
  if (dlp_strncmp(lpName,"functions",100) ==0 || all)
  {
    printf("\n--- list of registered C/C++ functions --------\n");
    m_cfns.PrintList();
  }
  // Release notes
  if (dlp_strncmp(lpName,"notes",100) ==0 || all)
  {
    printf("\n--- list of registered release notes --------\n");
    m_rnts.PrintList();
  }
  // Classes to install
  if (dlp_strncmp(lpName,"installs",100) ==0 || all)
  {
    printf("\n--- list of classes to install --------\n");
    m_icls.PrintList();
  }
  return O_K;
}

// -- TODO: Deprecated API --

INT16 CGEN_PROTECTED CCgen::OnCompiler()
{
  char lpsBuf[L_INPUTLINE+1];
  GetNextDefToken(lpsBuf);
  return O_K;
}

INT16 CGEN_PROTECTED CCgen::OnAr()
{
  char lpsBuf[L_INPUTLINE+1];
  GetNextDefToken(lpsBuf);
  return O_K;
}

INT16 CGEN_PROTECTED CCgen::LoadAncestor()
{
  IERROR(this,ERR_DEPRECATED,"-loadAncestor","Use SUPERCLASS: instead!",0);
  return OnSuperClass();
}

// EOF
