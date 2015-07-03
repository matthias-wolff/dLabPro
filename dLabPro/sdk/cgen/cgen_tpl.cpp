// dLabPro SDK class CCgen (cgen)
// - Handling of file templates
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

#ifdef __MSVC
  #include <windows.h>
  #include <winuser.h>
#endif
#include "dlp_cgen.h"
#include <time.h>

INT16 CGEN_PROTECTED CCgen::QueryMSVCVersion(char* lpMSDPath)
{
  dlp_strcpy(lpMSDPath,"");

#ifdef __MSVC
/*
  // Get MSDev working directory from the registry
  char           RegistryKey4[] = "SOFTWARE\\Microsoft\\Developer\\Directories";
  char           RegistryKey5[] = "SOFTWARE\\Microsoft\\DevStudio\\5.0\\Products\\Microsoft Visual C++";
  char           RegistryKey6[] = "SOFTWARE\\Microsoft\\DevStudio\\6.0\\Products\\Microsoft Visual C++";
  unsigned char* msdir4; msdir4 = (unsigned char*)calloc(1,255);
  unsigned char* msdir5; msdir5 = (unsigned char*)calloc(1,255);
  unsigned char* msdir6; msdir6 = (unsigned char*)calloc(1,255);
  UINT32  bufferLength = 255;
  HKEY           key;
  INT16          nVersion;

  if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,RegistryKey4,0,KEY_QUERY_VALUE,&key) == ERROR_SUCCESS)
  {
    bufferLength = 255;
    RegQueryValueEx(key,"ProductDir",NULL,NULL,msdir4,&bufferLength);
  }
  if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,RegistryKey5,0,KEY_QUERY_VALUE,&key) == ERROR_SUCCESS)
  {
    bufferLength = 255;
    RegQueryValueEx(key,"ProductDir",NULL,NULL,msdir5,&bufferLength);
  }
  if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,RegistryKey6,0,KEY_QUERY_VALUE,&key) == ERROR_SUCCESS)
  {
    bufferLength = 255;
    RegQueryValueEx(key,"ProductDir",NULL,NULL,msdir6,&bufferLength);
  }

  if (dlp_strlen((const char*)msdir6))
  {
    dlp_strcpy(lpMSDPath,(const char*)msdir6);
    nVersion=PF_MSDEV6;
  }
  else if (dlp_strlen((const char*)msdir5))
  {
    dlp_strcpy(lpMSDPath,(const char*)msdir5);
    nVersion=PF_MSDEV5;
  }
  else if (dlp_strlen((const char*)msdir4))
  {
    dlp_strcpy(lpMSDPath,(const char*)msdir4);
    nVersion=PF_MSDEV4;
  }
  else nVersion=0;

  free(msdir4); free(msdir5); free(msdir6);
  return nVersion;
*/
#endif

  return 0;
}

INT16 CGEN_PROTECTED CCgen::CompleteProject()
{
  INT16 bOk = TRUE;

  if (dlp_strlen(m_lpsProject) <=0)
  {
    ERRORMSG(ERR_DATAUNDEFD,"PROJECT:",0,0);
    bOk = FALSE;
  }

  char lpProjL[L_NAMES]; strcpy(lpProjL,m_lpsProject); dlp_strlwr(lpProjL);

  if (dlp_strlen(m_lpsClass)   <=0) strcpy(m_lpsClass,m_lpsProject);
  if (dlp_strlen(m_lpsMPath)   <=0) strcpy(m_lpsMPath,"../../manual/automatic");
  if (dlp_strlen(m_lpsIPath)   <=0) strcpy(m_lpsIPath,"../../include");
  if (dlp_strlen(m_lpsOPath)   <=0) strcpy(m_lpsOPath,"../../obj");
  if (dlp_strlen(m_lpsLPath)   <=0) strcpy(m_lpsLPath,"../../lib");
  if (dlp_strlen(m_lpsBPath)   <=0) strcpy(m_lpsBPath,"../../bin");
  if (dlp_strlen(m_lpsHomePath)<=0) if(getcwd(m_lpsHomePath,255)==NULL) return IERROR(this,ERR_GETCWD,0,0,0);

  // No further checks needed for documentation projects
  if (m_bClib) return bOk?O_K:NOT_EXEC;

  if (!dlp_strlen(m_lpsComment) && !m_bMainProject)
    ERRORMSG(ERR_MISSCOMMENT,"project",m_lpsProject,0);

  if (m_nPlatform ==0)
  {
    ERRORMSG(ERR_DATAUNDEFD,"PLATFORM:",0,0);
    bOk = FALSE;
  }

  if (dlp_strlen(m_lpsAuthor)        <=0) ERRORMSG(ERR_DATAUNDEFDW,"AUTHOR:","PROJECT",0);
  if (dlp_strlen(m_lpsVersion)       <=0) ERRORMSG(ERR_DATAUNDEFDW,"VERSION:","PROJECT",0);
  if (dlp_strlen(m_lpsCName)         <=0) dlp_convert_name(CN_DLP2CXX_CLSN,m_lpsCName,m_lpsProject);
  if (dlp_strlen(m_lpsCStructName)   <=0) dlp_convert_name(CN_CSTRUCTNAME,m_lpsCStructName,m_lpsProject);
  if (dlp_strlen(m_lpsLibfile)       <=0) sprintf(m_lpsLibfile,"%s",lpProjL);
  if (dlp_strlen(m_lpsAutoname)      <=0) dlp_convert_name(CN_AUTONAME,m_lpsAutoname,m_lpsClass);
  if (dlp_strlen(m_lpsCxxParent)     <=0) strcpy(m_lpsCxxParent,"CDlpObject");

  char lpBuf[L_PATH];
  dlp_strcpy(lpBuf,GetCurrentDefFile());
  dlp_splitpath(lpBuf,m_lpsHomePath,NULL);

  // Verify MSVC++ version and get tool path
  if (m_nPlatform == PF_MSDEV4 || m_nPlatform == PF_MSDEV5 || m_nPlatform == PF_MSDEV6)
  {
    INT16 nMSVCVer = QueryMSVCVersion(m_lpsMSDPath);
    if (!nMSVCVer) ERRORRET(ERR_NOMSVCFOUND,0,0,0,ERR_NOMSVCFOUND);
    if (nMSVCVer != m_nPlatform)
    {
      ERRORMSG(ERR_WRONGMSVCVERSION,nMSVCVer+2,0,0);
      m_nPlatform = nMSVCVer;
    }
  }

  if (bOk) return O_K; else return NOT_EXEC;
}

INT16 CGEN_PROTECTED CCgen::CheckSysFunction(CList<SCGStr>& lCode, const char* lpKey1, const char* lpKey2)
{
  // O_K if at least one (key1 or key2) is to be found in lCode.
  // This is to check whether the user called the base class from an interface
  // code shred.

  SCGStr* page = lCode.m_items;
  while (page)
  {
    if (lpKey1 && strstr(page->lpName,lpKey1)) return O_K;
    if (lpKey2 && strstr(page->lpName,lpKey2)) return O_K;
    page = page->next;
  }

  return NOT_EXEC;
}

INT16 CGEN_PROTECTED CCgen::CompleteSysFunctions()
{
  if (m_classCode.IsListEmpty()  ) m_classCode.AddItem  ("  return CLASSPROC;");
  if (m_copyCode.IsListEmpty()   ) m_copyCode.AddItem   ("  return COPY;"     );
  if (m_doneCode.IsListEmpty()   ) m_doneCode.AddItem   ("  DONE;"            );
  if (m_installCode.IsListEmpty()) m_installCode.AddItem("  return INSTALL;"  );
  if (m_initCode.IsListEmpty()   ) m_initCode.AddItem   ("  INIT;"            );
  if (m_resetCode.IsListEmpty()  ) m_resetCode.AddItem  ("  return RESET;"    );
  if (m_restoreCode.IsListEmpty()) m_restoreCode.AddItem("  return RESTORE;"  );
  if (m_saveCode.IsListEmpty()   ) m_saveCode.AddItem   ("  return SAVE;"     );

  IF_NOK(CheckSysFunction(m_classCode  ,"CLASSPROC;","::ClassProc("  )) ERRORMSG(ERR_NOBASECALL,"CLASSPROC;","CLASSCODE:"   ,0);
  IF_NOK(CheckSysFunction(m_copyCode   ,"COPY;"     ,"::Copy("       )) ERRORMSG(ERR_NOBASECALL,"COPY;"     ,"COPYCODE:"    ,0);
  IF_NOK(CheckSysFunction(m_doneCode   ,"DONE;"     ,"::Done("       )) ERRORMSG(ERR_NOBASECALL,"DONE;"     ,"DONECODE:"    ,0);
  IF_NOK(CheckSysFunction(m_initCode   ,"INIT;"     ,"::Init("       )) ERRORMSG(ERR_NOBASECALL,"INIT;"     ,"INITCODE:"    ,0);
  IF_NOK(CheckSysFunction(m_installCode,"INSTALL;"  ,"::InstallProc(")) ERRORMSG(ERR_NOBASECALL,"INSTALL;"  ,"INSTALLCODE:" ,0);
  IF_NOK(CheckSysFunction(m_resetCode  ,"RESET;"    ,"::Reset("      )) ERRORMSG(ERR_NOBASECALL,"RESET;"    ,"RESETCODE:"   ,0);
  IF_NOK(CheckSysFunction(m_restoreCode,"RESTORE;"  ,"::Restore("    )) ERRORMSG(ERR_NOBASECALL,"RESTORE;"  ,"RESTORECODE:" ,0);
  IF_NOK(CheckSysFunction(m_saveCode   ,"SAVE;"     ,"::Save("       )) ERRORMSG(ERR_NOBASECALL,"SAVE;"     ,"SAVECODE:"    ,0);

  return O_K;
}

INT16 CGEN_PROTECTED CCgen::CompleteError(SCGErr* lpErr)
{
  DLPASSERT(lpErr);

  if (!dlp_strlen(lpErr->lpErrorLevel)) dlp_strcpy(lpErr->lpErrorLevel,"EL_ERROR");
  if (!dlp_strlen(lpErr->lpComment))
  {
    ERRORMSG(ERR_MISSCOMMENT,"error",lpErr->lpName,0);
    dlp_strcpy(lpErr->lpComment,"");
  }

  return O_K;
}

INT16 CGEN_PROTECTED CCgen::CompleteField(SCGFld* lpFld)
{
  DLPASSERT(lpFld);

  // Comment not specified...
  if (!dlp_strlen(lpFld->lpComment) && !(lpFld->nFlags & FF_HIDDEN))
  {
    ERRORMSG(ERR_MISSCOMMENT,"field",lpFld->lpName,0);
    dlp_strcpy(lpFld->lpComment,"");
  }

  // C type not specified...
  if (!dlp_strlen(lpFld->lpCType))
  {
    ERRORMSG(ERR_MISSTYPE,"field",lpFld->lpName,0);
    lpFld->nType     = T_SHORT;
    lpFld->nArrayLen = 1;
    dlp_strcpy(lpFld->lpCType    ,"INT16");
    dlp_strcpy(lpFld->lpType     ,""     );
    dlp_strcpy(lpFld->lpArraySpec,""     );
  }

  // C++ member variable name not specified...
  if (!dlp_strlen(lpFld->lpCName))
  {
    if (m_bCxxNconv)
    {
      if (lpFld->nType > 0 && lpFld->nType <= 256)
      {
        dlp_convert_name(CN_DLP2CXX_SPAR,lpFld->lpCName,lpFld->lpName);
      }
      else switch(lpFld->nType)
      {
      case T_PTR   :
        dlp_convert_name(CN_DLP2CXX_LPAR,lpFld->lpCName,lpFld->lpName);
        break;
      case T_STRING:
      case T_TEXT  :
        dlp_convert_name(CN_DLP2CXX_SPAR,lpFld->lpCName,lpFld->lpName);
        break;
      case T_BOOL:
        dlp_convert_name(CN_DLP2CXX_BPAR,lpFld->lpCName,lpFld->lpName);
        break;
      case T_IGNORE:
        dlp_convert_name(CN_DLP2CXX_PAR ,lpFld->lpCName,lpFld->lpName);
        break;
      case T_INSTANCE:
        if      (dlp_strncmp(lpFld->lpType,"data",L_NAMES)==0) dlp_convert_name(CN_DLP2CXX_DPAR,lpFld->lpCName,lpFld->lpName);
        else if (dlp_strncmp(lpFld->lpType,"fst" ,L_NAMES)==0) dlp_convert_name(CN_DLP2CXX_TPAR,lpFld->lpCName,lpFld->lpName);
        else    dlp_convert_name(CN_DLP2CXX_IPAR,lpFld->lpCName,lpFld->lpName);
        break;
      default:
        dlp_convert_name(CN_DLP2CXX_NPAR,lpFld->lpCName,lpFld->lpName);
      }
    }
    else dlp_convert_name(CN_DLP2CXX_PAR,lpFld->lpCName,lpFld->lpName);
  }

  // Init value not specified...
  if (!dlp_strlen(lpFld->lpInit))
  {
    if (lpFld->nType>0 && lpFld->nType<=256)
    {
      ERRORMSG(ERR_MISSINIT,"field",lpFld->lpName,"\"\"");
      dlp_strcpy(lpFld->lpInit,"\"\"");
    }
    else
    {
      switch(lpFld->nType) {
      case T_INSTANCE:
      case T_PTR     :
      case T_STRING  :
      case T_TEXT    :
        {
          ERRORMSG(ERR_MISSINIT,"field",lpFld->lpName,"NULL");
          dlp_strcpy(lpFld->lpInit,"NULL");
          break;
        }
      case T_FLOAT   :
        {
          ERRORMSG(ERR_MISSINIT,"field",lpFld->lpName,"0.");
          dlp_strcpy(lpFld->lpInit,"0.f");
          break;
        }
      case T_DOUBLE  :
        {
          ERRORMSG(ERR_MISSINIT,"field",lpFld->lpName,"0.");
          dlp_strcpy(lpFld->lpInit,"0.");
          break;
        }
      case T_COMPLEX :
        {
          ERRORMSG(ERR_MISSINIT,"field",lpFld->lpName,"0.");
          dlp_strcpy(lpFld->lpInit,"CMPLX(0.)");
          break;
        }
      default:
        {
          ERRORMSG(ERR_MISSINIT,"field",lpFld->lpName,"0");
          dlp_strcpy(lpFld->lpInit,"0");
        }
      }
    }
  }

  //RTTI not present
  if (!dlp_strlen(lpFld->lpType)) dlp_strcpy(lpFld->lpType,"");

  return O_K;
}

INT16 CGEN_PROTECTED CCgen::CompleteOption(SCGOpt* lpOpt)
{
  DLPASSERT(lpOpt);

  // Comment not specified...
  if (!dlp_strlen(lpOpt->lpComment))
  {
    ERRORMSG(ERR_MISSCOMMENT,"option",lpOpt->lpName,0);
    dlp_strcpy(lpOpt->lpComment,"");
  }

  // C++ member variable name not specified...
  if (!dlp_strlen(lpOpt->lpCName))
  {
    dlp_convert_name(CN_DLP2CXX_BPAR,lpOpt->lpCName,lpOpt->lpName);
  }

  return O_K;
}

INT16 CGEN_PROTECTED CCgen::CompleteMethod(SCGMth* lpMth)
{
  DLPASSERT(lpMth);

  // Comment not specified...
  if (!dlp_strlen(lpMth->lpComment))
  {
    ERRORMSG(ERR_MISSCOMMENT,"method",lpMth->lpName,0);
    dlp_strcpy(lpMth->lpComment,"");
  }

  // Syntax not specified...
  if (!dlp_strlen(lpMth->lpUPNSyntax))
  {
    ERRORMSG(ERR_MISSSYNTAX,"method",lpMth->lpName,0);
    sprintf(lpMth->lpUPNSyntax,"<%s this>",m_lpsClass);
    strcpy(lpMth->lpCSyntax,"()");
  }

  // C++ member variable name not specified...
  if (!dlp_strlen(lpMth->lpCName))
  {
    dlp_convert_name(CN_DLP2CXX_CCF,lpMth->lpCName,lpMth->lpName);
  }

  return O_K;
}

INT16 CGEN_PROTECTED CCgen::CompleteNote(SCGRnt* lpRnt)
{
  DLPASSERT(lpRnt);

  // Comment not specified...
  if (!dlp_strlen(lpRnt->lpComment))
  {
    ERRORMSG(ERR_MISSCOMMENT,"note",lpRnt->lpName,0);
    dlp_strcpy(lpRnt->lpComment,"");
  }

  // No manual text!
  if (!lpRnt->lMan.m_items) ERRORMSG(ERR_MISSNOTE,lpRnt->lpName,0,0);

  return O_K;
}

INT16 CGEN_PROTECTED CCgen::CompleteIcls(SCGIcl* lpIcl)
{
  DLPASSERT(lpIcl);

  return O_K;
}

INT16 CGEN_PROTECTED CCgen::SetCreating(INT16 nWhat, void* lpItem)
{
  // Check completeness of most recently created section
  switch (m_nCreating)
  {
  case CR_PROJECT: CompleteProject();               break;
  case CR_ERROR  : CompleteError (m_lpCreatingErr); break;
  case CR_FIELD  : CompleteField (m_lpCreatingFld); break;
  case CR_OPTION : CompleteOption(m_lpCreatingOpt); break;
  case CR_METHOD : CompleteMethod(m_lpCreatingMth); break;
  case CR_NOTE   : CompleteNote  (m_lpCreatingRnt); break;
  case CR_ICLS   : CompleteIcls  (m_lpCreatingIcl); break;
  }

  m_nCreating = nWhat;
  m_lpCreatingErr = NULL;
  m_lpCreatingFld = NULL;
  m_lpCreatingOpt = NULL;
  m_lpCreatingMth = NULL;
  m_lpCreatingRnt = NULL;
  m_lpCreatingIcl = NULL;
  switch (m_nCreating)
  {
  case CR_ERROR : m_lpCreatingErr = (SCGErr*) lpItem; break;
  case CR_FIELD : m_lpCreatingFld = (SCGFld*) lpItem; break;
  case CR_METHOD: m_lpCreatingMth = (SCGMth*) lpItem; break;
  case CR_OPTION: m_lpCreatingOpt = (SCGOpt*) lpItem; break;
  case CR_NOTE  : m_lpCreatingRnt = (SCGRnt*) lpItem; break;
  case CR_ICLS  : m_lpCreatingIcl = (SCGIcl*) lpItem; break;
  }

  return O_K;
}

INT16 CGEN_PROTECTED CCgen::ReplaceAllKeys(CList<SCGStr> &tmpl)
{
  INT16 i = 0;
  char  cd[10]          ; sprintf(cd,"%c",C_DIR);
  char  lpProjL[L_NAMES]; dlp_strcpy(lpProjL,m_lpsProject); dlp_strlwr(lpProjL);
  char  lpProjU[L_NAMES]; dlp_strcpy(lpProjU,m_lpsProject); dlp_strupr(lpProjU);
  char  lpDefL[L_NAMES];  lpDefL[0] = 0;
  char  lpDefFile[L_NAMES];  lpDefL[0] = 0;
  char  tmpbuf[128];
  SCGStr* lpDef = NULL;


  // Class factory base class
  char lpFcParent[L_NAMES];
  if (dlp_strcmp(m_lpsCxxParent,"CDlpObject")==0) strcpy(lpFcParent,"CDlpClFactory");
  else                                            sprintf(lpFcParent,"F%s",m_lpsCxxParent);

  // Class flags string
  char cs[255];
  BEGIN_FLAGS(cs)
  APPEND_FLAG(m_nCSXXX,CS_AUTOINSTANCE)
  APPEND_FLAG(m_nCSXXX,CS_AUTOACTIVATE)
  APPEND_FLAG(m_nCSXXX,CS_SINGLETON   )
  APPEND_FLAG(m_nCSXXX,CS_SECONDARY   )
  APPEND_FLAG(m_nCSXXX,CS_CCLASS      )
  END_FLAGS

  // Assemble include paths and files
  char incl[1024]; char iPath[255]; char lpBuf[1024]; char lpIncBuf[1024];
  dlp_memset(incl,0,1024);
  strcpy(lpIncBuf,m_lpsIPath); // This is the primary include path in which the .h file will be placed.
  bool bPrimary = TRUE;
  char* tx = strtok(lpIncBuf,",; \t"); dlp_strcpy(iPath,tx);
  while (tx)
  {
    if (m_nPlatform == PF_GNUCXX) sprintf(lpBuf,"-I %s ",tx);
    else                          sprintf(lpBuf,"/I \"%s\" ",tx);
    strcat(incl,lpBuf);
    if (bPrimary)
    {
      // Add automatic include path
            if (m_nPlatform == PF_GNUCXX) sprintf(lpBuf,"-I %s/automatic ",tx);
            else                          sprintf(lpBuf,"/I \"%s/automatic\" ",tx);
      bPrimary = FALSE;
      strcat(incl,lpBuf);
    }
    tx = strtok(NULL,",; \t");
  }
  if (m_nPlatform != PF_GNUCXX)
    {
      sprintf(lpBuf,"/I \"%s\\INCLUDE\" /I \"%s\\INCLUDE\\SYS\"",m_lpsMSDPath,m_lpsMSDPath);
      strcat(incl,lpBuf);
    }

#ifdef __MSVC
  _strdate(tmpbuf);
#else
  time_t t; time(&t); strcpy(tmpbuf,ctime(&t));
  dlp_memmove(&tmpbuf[0],&tmpbuf[4],21);
  dlp_memmove(&tmpbuf[7],&tmpbuf[16],21);
  tmpbuf[11] = '\0';
#endif

  // Distinguish release and debug version
  char lpDebug[]   = "";
  char lpRelease[] = "release.";

  // Assemble list of processed def files
  dlp_strcpy(lpDefFile,m_defList.FindItem(0)->lpName);  // first entry in list is top-level def script
  for(i=0;(lpDef = m_defList.FindItem(i));i++)
  {
      if(strlen(lpDefL)+strlen(lpDef->lpName) < L_NAMES-1)
      {
         dlp_strcat(lpDefL,lpDef->lpName);
         dlp_strcat(lpDefL," ");
      }
      else
        break;
  }

  // Do replacement
  SCGStr *page = tmpl.m_items;
  while (page)
  {
    if(NULL != strstr(page->lpName,"${DefL}"))
      printf(" ");

    ReplaceKey(page->lpName,"${/}"          ,cd                ,0          );
    ReplaceKey(page->lpName,"${Author}"     ,m_lpsAuthor       ,0          );
    ReplaceKey(page->lpName,"${AutoName}"   ,(char*)(m_nCSXXX&CS_AUTOINSTANCE?m_lpsAutoname:""),0);
    ReplaceKey(page->lpName,"${BPath}"      ,m_lpsBPath        ,0          );
    ReplaceKey(page->lpName,"${GccFlags}"   ,m_lpsGccFlags     ,0          );
    ReplaceKey(page->lpName,"${MsvcFlags}"  ,m_lpsMsvcFlags    ,0          );
    ReplaceKey(page->lpName,"${CLStyle}"    ,cs                ,0          );
    ReplaceKey(page->lpName,"${Comment}"    ,m_lpsComment      ,0          );
    ReplaceKey(page->lpName,"${CxxClass}"   ,m_lpsCName        ,0          );
    ReplaceKey(page->lpName,"${Debug}"      ,lpDebug           ,0          );
    ReplaceKey(page->lpName,"${HFile}"      ,m_lpsProject      ,CN_HFILE   );
    ReplaceKey(page->lpName,"${homePath}"   ,m_lpsHomePath     ,0          );
    ReplaceKey(page->lpName,"${IPath}"      ,iPath             ,0          );
    ReplaceKey(page->lpName,"${Include}"    ,incl              ,0          );
    ReplaceKey(page->lpName,"${Libfile}"    ,m_lpsLibfile      ,0          );
    ReplaceKey(page->lpName,"${LPath}"      ,m_lpsLPath        ,0          );
    ReplaceKey(page->lpName,"${MAKEFile}"   ,m_lpsProject      ,CN_MAKEFILE);
    ReplaceKey(page->lpName,"${MoreParents}",m_lpsCxxMoreParents,0         );
    ReplaceKey(page->lpName,"${MPath}"      ,m_lpsMPath        ,0          );
    ReplaceKey(page->lpName,"${MSDPath}"    ,m_lpsMSDPath      ,0          );
    ReplaceKey(page->lpName,"${NMAKFile}"   ,m_lpsProject      ,CN_NMAKFILE);
    ReplaceKey(page->lpName,"${ObsName}"    ,m_lpsObsolete     ,0          );
    ReplaceKey(page->lpName,"${OPath}"      ,m_lpsOPath        ,0          );
    ReplaceKey(page->lpName,"${Package}"    ,m_lpsPackage      ,0          );
    ReplaceKey(page->lpName,"${Parent}"     ,m_lpsCxxParent    ,0          );
    ReplaceKey(page->lpName,"${FcParent}"   ,lpFcParent        ,0          );
    ReplaceKey(page->lpName,"${Proj}"       ,m_lpsProject      ,0          );
    ReplaceKey(page->lpName,"${ProjL}"      ,lpProjL           ,0          );
    ReplaceKey(page->lpName,"${ProjU}"      ,lpProjU           ,0          );
    ReplaceKey(page->lpName,"${Release}"    ,lpRelease         ,0          );
    ReplaceKey(page->lpName,"${SLBaseName}" ,(char*)(dlp_strlen(m_lpsDlcParent)?m_lpsDlcParent:"-"),0);
    ReplaceKey(page->lpName,"${SLName}"     ,m_lpsClass        ,0          );
    ReplaceKey(page->lpName,"${Today}"      ,tmpbuf            ,0          );
    ReplaceKey(page->lpName,"${TypeName}"   ,m_lpsCStructName  ,0          );
    ReplaceKey(page->lpName,"${Version}"    ,m_lpsVersion      ,0          );
    ReplaceKey(page->lpName,"${DefFile}"    ,lpDefFile         ,0          );
    ReplaceKey(page->lpName,"${DefL}"       ,lpDefL            ,0          );
    ReplaceKey(page->lpName,"${PType}"      ,(char*)(m_bClib?"Library":"Class") ,0   );

    if (m_bCProject) ReplaceKey(page->lpName,"${CPPFile}",m_lpsProject,CN_CFILE  );
    else             ReplaceKey(page->lpName,"${CPPFile}",m_lpsProject,CN_CPPFILE);

    if (m_bCProject) ReplaceKey(page->lpName,"${SExt}","c",  0);
    else             ReplaceKey(page->lpName,"${SExt}","cpp",0);

    // Some special
    if      (m_nPlatform == PF_MSDEV5) ReplaceKey(page->lpName,"${callvcvars}","call vcvars32",0);
    else if (m_nPlatform == PF_MSDEV6) ReplaceKey(page->lpName,"${callvcvars}","call vcvars32",0);
    else                               ReplaceKey(page->lpName,"${callvcvars}",""             ,0);

    // Insert code for cvs keyword substitution
    ReplaceKey(page->lpName,"${Id}","$Id",0);

    page = page->next;
  }

  return O_K;
}

INT16 CGEN_PROTECTED CCgen::ReplaceAllKeys2(CList<SCGStr> &tmpl)
{
  SCGStr *page = tmpl.m_items;
  while (page)
  {
    if (m_bCProject)
    {
      ReplaceKey(page->lpName,"${/*}","/*" ,0);
      ReplaceKey(page->lpName,"${*/}"," */",0);
      ReplaceKey(page->lpName,"${**}"," *" ,0);
    }
    else
    {
      ReplaceKey(page->lpName,"${/*}","//" ,0);
      ReplaceKey(page->lpName,"${*/}",""   ,0);
      ReplaceKey(page->lpName,"${**}","//" ,0);
    }

    page = page->next;
  }

  return O_K;
}

INT16 CGEN_PROTECTED CCgen::RealizeStrList(CList<SCGStr> &tmpl, CList<SCGStr> &str, const char *lpKey, const char *lpPrefix, const char *lpSuffix)
{
  char buf[L_INPUTLINE];

  SCGStr *pstr;
  SCGStr *ptmpl = tmpl.m_items;
  while (ptmpl)
  {
    if (strstr(ptmpl->lpName,lpKey))
    {
      pstr = str.m_items;
      while (pstr)
      {
        sprintf(buf,"%s%s%s",lpPrefix,pstr->lpName,lpSuffix);
        tmpl.InsertItem(ptmpl,buf);
        ptmpl = ptmpl->next;
        pstr = pstr->next;
      }
    }
    ptmpl = ptmpl->next;
  }

  return O_K;
}

/**
 * Creates the copyright and copyleft header comments.
 */
INT16 CGEN_PROTECTED CCgen::RealizeCopyright(CList<SCGStr> &tmpl, const char* lpPrefix)
{
  CList<SCGStr> lCopyright;
  lCopyright.AddItem("Copyright 2013 dLabPro contributors and others (see COPYRIGHT file) ");
  lCopyright.AddItem("- Chair of System Theory and Speech Technology, TU Dresden");
  lCopyright.AddItem("- Chair of Communications Engineering, BTU Cottbus");
  lCopyright.AddItem("");
  lCopyright.AddItem("This file is part of dLabPro.");
  lCopyright.AddItem("");
  lCopyright.AddItem("dLabPro is free software: you can redistribute it and/or modify it under the");
  lCopyright.AddItem("terms of the GNU Lesser General Public License as published by the Free");
  lCopyright.AddItem("Software Foundation, either version 3 of the License, or (at your option)");
  lCopyright.AddItem("any later version.");
  lCopyright.AddItem("");
  lCopyright.AddItem("dLabPro is distributed in the hope that it will be useful, but WITHOUT ANY");
  lCopyright.AddItem("WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS");
  lCopyright.AddItem("FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more");
  lCopyright.AddItem("details.");
  lCopyright.AddItem("");
  lCopyright.AddItem("You should have received a copy of the GNU Lesser General Public License");
  lCopyright.AddItem("along with dLabPro. If not, see <http://www.gnu.org/licenses/>.");

  INT16 nErr = RealizeStrList(tmpl,lCopyright,"${Copyright}",lpPrefix,"");
  for (SCGStr* page = tmpl.m_items; page!=NULL; page=page->next)
    ReplaceKey(page->lpName,"${Copyright}",lpPrefix,0);

  return nErr;
}

INT16 CGEN_PROTECTED CCgen::WriteBackTemplate(CList<SCGStr> &tmpl, char* lpPath, char *lpFName, INT16 bPrint DEFAULT(1))
{
  char    buf[255];
  SCGStr *page;
  FILE   *f;

  if(chdir(m_lpsHomePath) != 0) return IERROR(this,ERR_CHDIR,m_lpsHomePath,0,0);
  dlp_chdir(lpPath,TRUE);
  if(getcwd(buf,255)==NULL) return IERROR(this,ERR_GETCWD,0,0,0);
  if (bPrint) printf("\n%s %s%c%s",GetVerboseLevel()>1?" ":"g -",buf,C_DIR,lpFName);
  f = fopen(lpFName,"wb");
  if (!f) ERRORRET(ERR_CREATEFILE,lpFName,0,0,NOT_EXEC);
  page =tmpl.m_items;
  while (page)
  {
    fputs(page->lpName,f); fputs("\n",f);
    page = page->next;
  }
  fclose(f);
  m_nFgen++;
  if (chdir(m_lpsHomePath)!=0)
    return IERROR(this,ERR_CHDIR,m_lpsHomePath,0,0);

  return O_K;
}

INT16 CGEN_PROTECTED CCgen::AnalyzeSyntax
(
  char*          lpName,
  CList<SCGSyn>& lSyntax,
  char*          lpSyntax,
  SCGSyn*        lpReturn,
  char*          lpSDescr,
  char*          lpCDescr
)
{
  INT16 nOCtr = 1;
  INT16 nNCtr = 0;
  INT16 nSCtr = 0;
  INT16 nBCtr = 0;
  INT16 bRecog;
  INT16 bInstance;
  char  lpBuf[255]; dlp_strcpy(lpBuf,lpSyntax);
  char  lpBuf2[255];
  char  n[10], o[10], s[10], b[10];
  char* lpArgs = NULL;
  char* lpRetv = NULL;
  BOOL  bCustom = FALSE;
  dlp_strcpy(lpSDescr,"");
  dlp_strcpy(lpCDescr,"");

  // Reset syntax table
  lSyntax.Delete();

  // Split syntax description into return value and argument list
  dlp_strtrimleft(lpBuf);
  dlp_strtrimright(lpBuf);

  // Detect and handle custom syntax (not to be parsed)
  if (dlp_strlen(lpBuf)>1)
    bCustom = (lpBuf[0]=='\"' && lpBuf[dlp_strlen(lpBuf)-1]=='\"');
  if (bCustom)
    dlp_strcpy(lpBuf,"()");

  char* c = lpBuf;
  while (*c != '\0' && *c != '(' && !iswspace(*c)) c++;
  if (c != lpBuf) lpRetv = lpBuf;
  dlp_strtrimleft(c);
  if (*c == '(')
  {
    if (c[dlp_strlen(c)-1] == ')')
    {
      *c = '\0';
      lpArgs = ++c;
      lpArgs[dlp_strlen(lpArgs)-1] = '\0';
    }
    else ERRORRET(ERR_BADSYNTAX,0,0,0,ERR_BADSYNTAX);
  }

  // Retval ok?
  if (dlp_strlen(lpRetv))
  {
    if (dlp_strncmp(lpRetv,"complex",L_NAMES) ==0) {
      dlp_strcpy(lpReturn->lpType,"COMPLEX64");
      dlp_strcpy(lpReturn->lpSupply,"MIC_PUT_C(");
      dlp_strcpy(lpReturn->lpDefaultVal,"CMPLX(0)");
    }
    else if (dlp_strncmp(lpRetv,"double",L_NAMES) ==0) {
      dlp_strcpy(lpReturn->lpType,"FLOAT64");
      dlp_strcpy(lpReturn->lpSupply,"MIC_PUT_N(");
      dlp_strcpy(lpReturn->lpDefaultVal,"0");
    }
    else if(dlp_strncmp(lpRetv,"float" ,L_NAMES) ==0)
    {
      dlp_strcpy(lpReturn->lpType,"FLOAT32");
      dlp_strcpy(lpReturn->lpSupply,"MIC_PUT_N(");
      dlp_strcpy(lpReturn->lpDefaultVal,"0");
    }
    else if (dlp_strncmp(lpRetv,"char"          ,L_NAMES) ==0) {
      dlp_strcpy(lpReturn->lpType,"INT8");
      dlp_strcpy(lpReturn->lpSupply,"MIC_PUT_N(");
      dlp_strcpy(lpReturn->lpDefaultVal,"0");
    }
    else if(dlp_strncmp(lpRetv,"unsigned char" ,L_NAMES) ==0) {
      dlp_strcpy(lpReturn->lpType,"UINT8");
      dlp_strcpy(lpReturn->lpSupply,"MIC_PUT_N(");
      dlp_strcpy(lpReturn->lpDefaultVal,"0");
    }
    else if(dlp_strncmp(lpRetv,"short"         ,L_NAMES) ==0) {
      dlp_strcpy(lpReturn->lpType,"INT16");
      dlp_strcpy(lpReturn->lpSupply,"MIC_PUT_N(");
      dlp_strcpy(lpReturn->lpDefaultVal,"0");
    }
    else if(dlp_strncmp(lpRetv,"unsigned short",L_NAMES) ==0) {
      dlp_strcpy(lpReturn->lpType,"UINT16");
      dlp_strcpy(lpReturn->lpSupply,"MIC_PUT_N(");
      dlp_strcpy(lpReturn->lpDefaultVal,"0");
    }
    else if(dlp_strncmp(lpRetv,"int"          ,L_NAMES) ==0) {
      dlp_strcpy(lpReturn->lpType,"INT32");
      dlp_strcpy(lpReturn->lpSupply,"MIC_PUT_N(");
      dlp_strcpy(lpReturn->lpDefaultVal,"0");
    }
    else if(dlp_strncmp(lpRetv,"unsigned int"          ,L_NAMES) ==0) {
      dlp_strcpy(lpReturn->lpType,"UINT32");
      dlp_strcpy(lpReturn->lpSupply,"MIC_PUT_N(");
      dlp_strcpy(lpReturn->lpDefaultVal,"0");
    }
    else if(dlp_strncmp(lpRetv,"long"          ,L_NAMES) ==0) {
      dlp_strcpy(lpReturn->lpType,"INT64");
      dlp_strcpy(lpReturn->lpSupply,"MIC_PUT_N(");
      dlp_strcpy(lpReturn->lpDefaultVal,"0");
    }
    else if(dlp_strncmp(lpRetv,"unsigned long" ,L_NAMES) ==0) {
      dlp_strcpy(lpReturn->lpType,"UINT64");
      dlp_strcpy(lpReturn->lpSupply,"MIC_PUT_N(");
      dlp_strcpy(lpReturn->lpDefaultVal,"0");
    }
    else if (dlp_strncmp(lpRetv,"void",L_NAMES) ==0)
    {
      dlp_strcpy(lpReturn->lpType,lpRetv);
      dlp_strcpy(lpReturn->lpSupply,"");
      dlp_strcpy(lpReturn->lpDefaultVal,"");
    }
    else if (dlp_strncmp(lpRetv,"ptr",L_NAMES) ==0)
    {
      dlp_strcpy(lpReturn->lpType,"void*");
      dlp_strcpy(lpReturn->lpSupply,"MIC_PUT_S((const char*)");
      dlp_strcpy(lpReturn->lpDefaultVal,"NULL");
    }
    else if (dlp_strncmp(lpRetv,"boolean",L_NAMES) ==0 ||
             dlp_strncmp(lpRetv,"BOOL"   ,L_NAMES) ==0  )
    {
      dlp_strcpy(lpReturn->lpType,"BOOL");
      dlp_strcpy(lpReturn->lpSupply,"MIC_PUT_B(");
      dlp_strcpy(lpReturn->lpDefaultVal,"FALSE");
    }
    else if (dlp_strncmp(lpRetv,"string",L_NAMES) ==0 ||
           dlp_strncmp(lpRetv,"char*" ,L_NAMES) ==0  )
    {
      dlp_strcpy(lpReturn->lpType,"char*");
      dlp_strcpy(lpReturn->lpSupply,"MIC_PUT_S(");
      dlp_strcpy(lpReturn->lpDefaultVal,"NULL");
    }
    else if (dlp_strncmp(lpRetv,"cstring",L_NAMES) ==0 ||
           dlp_strncmp(lpRetv,"const char*" ,L_NAMES) ==0  )
    {
      dlp_strcpy(lpReturn->lpType,"const char*");
      dlp_strcpy(lpReturn->lpSupply,"MIC_PUT_S(");
      dlp_strcpy(lpReturn->lpDefaultVal,"NULL");
    }
    else ERRORMSG(ERR_BADRETTYPE,lpRetv,0,0);
  }

  // Break into argument strings
  char *tx = strtok(lpArgs,",");
  while (tx)
  {
    lSyntax.AddItem(tx);
    tx = strtok(NULL,",");
  }

  // Remove obsolete THIS symbol
//  if (lSyntax.m_items && strstr(lSyntax.m_items->lpName,"THIS"))
//  {
//    ERRORMSG(ERR_THIS,0,0,0);
//    lSyntax.Delete();
//  }
  SCGSyn *page = lSyntax.m_items;
//  while (page && page->next)
//  {
//    if (strstr(page->next->lpName,"THIS"))
//    {
//      ERRORMSG(ERR_THIS,0,0,0);
//      delete(page->next);
//      page->next = NULL;
//      break;
//    }
//    page = page->next;
//  }

  // Analyze argument strings
  INT16 nArgs = (INT16)lSyntax.Count();
  page = lSyntax.m_items;
  while (page)
  {
    dlp_strcpy(lpBuf,page->lpName);
    bRecog    = FALSE;
    bInstance = FALSE;

    // What is obsolete?
    if (ReplaceKey(lpBuf,"text","string",0))
      IERROR(this,ERR_DEPRECATED,"text (for argument)","Use string instead.",0);

    // Scan and remove INSTANCE[()] (no RTTI!)
    ReplaceKey(lpBuf,"INSTANCE()"    ,"instance",0);
    ReplaceKey(lpBuf,"INSTANCE(\"\")","instance",0);

    // Scan and remove INSTANCE(*) constructs
    if (ReplaceKey(lpBuf,"INSTANCE(","",0)) bRecog = TRUE;
    ReplaceKey(lpBuf,")","",0);
    ReplaceKey(lpBuf,"\"","",0);

    // Scan and remove INSTANCE[()] (no RTTI!)
    ReplaceKey(lpBuf,"INSTANCE" ,"instance" ,0);
    ReplaceKey(lpBuf,"DATA"     ,"data"     ,0);
    ReplaceKey(lpBuf,"STRUCTURE","structure",0);                                // Deprecated

    // Analyze optional argument type (>,<,=) & variable type
    page->nArgFlags = AF_INOUT;
    tx = strtok(lpBuf," \n\t");
    if (dlp_strncmp(tx,">",1) == 0)
    {
      page->nArgFlags = AF_IN;
      tx = strtok(NULL," \n\t");
    }
    else if (dlp_strncmp(tx,"<",1) == 0)
    {
      page->nArgFlags = AF_OUT;
      tx = strtok(NULL," \n\t");
    }
    else if (dlp_strncmp(tx,"<>",2) == 0 ||
             dlp_strncmp(tx,"><",2) == 0 ||
             dlp_strncmp(tx,"=" ,1) == 0  )
    {
      tx = strtok(NULL," \n\t");
    }
    dlp_strcpy(page->lpType,tx);
    // Get name
    tx = strtok(NULL," \n\t");
    if (dlp_strlen(tx) !=0) dlp_strcpy(page->lpName,tx);
    else ERRORMSG(ERR_EXPECTIDAFTER,page->lpType,lpName,0);
    // Let there be no other tokens
    if(strtok(NULL," \n\t") != NULL)
      ERRORMSG(ERR_TOOMANYPARS,page->lpType,lpName,0);
    // Adjust supplement to variable type
    if (dlp_strncmp(page->lpType,"cstring",L_NAMES) ==0  )
    {
      sprintf(page->lpSupply,"\tconst char* %s = MIC_GET_S(%d,${s});",page->lpName,nArgs-page->nId);
      nSCtr++; bRecog = TRUE;
    }
    else if (dlp_strncmp(page->lpType,"text"  ,L_NAMES) ==0 ||
      dlp_strncmp(page->lpType,"string",L_NAMES) ==0  )
    {
      sprintf(page->lpSupply,"\tchar* %s = MIC_GET_S(%d,${s});",page->lpName,nArgs-page->nId);
      nSCtr++; bRecog = TRUE;
    }
    else if (dlp_strncmp(page->lpType,"ptr"  ,L_NAMES) ==0)
    {
      sprintf(page->lpSupply,"\tvoid* %s = MIC_GET_S(%d,${s});",page->lpName,nArgs-page->nId);
      nSCtr++; bRecog = TRUE;
    }
    else if (dlp_strncmp(page->lpType,"instance",L_NAMES) ==0)
    {
      // Any instance accepted
      if (page->nArgFlags == AF_IN)
        sprintf(page->lpSupply,"\tconst CDlpObject* %s = MIC_GET_I(%d,${o});",page->lpName,nArgs-page->nId);
      else
        sprintf(page->lpSupply,"\tCDlpObject* %s = MIC_GET_I(%d,${o});",page->lpName,nArgs-page->nId);
      nOCtr++; bRecog = TRUE; bInstance = TRUE;
    }
    else if (dlp_strncmp(page->lpType,"boolean",L_NAMES) ==0)
    {
      sprintf(page->lpSupply,"\tBOOL %s = MIC_GET_B(%d,${b});",page->lpName,nArgs-page->nId);
      dlp_strcpy(page->lpType,"BOOL");
      nBCtr++; bRecog = TRUE;
    }
    else if (dlp_strncmp(page->lpType,"complex",L_NAMES) ==0)
    {
      sprintf(page->lpSupply,"\tCOMPLEX64 %s = MIC_GET_C(%d,${n});",page->lpName,nArgs-page->nId);
      nNCtr++; bRecog = TRUE;
    }
    else if (dlp_strncmp(page->lpType,"double",L_NAMES) ==0)
    {
      sprintf(page->lpSupply,"\tFLOAT64 %s = MIC_GET_N(%d,${n});",page->lpName,nArgs-page->nId);
      nNCtr++; bRecog = TRUE;
    }
    else if (dlp_strncmp(page->lpType,"float",L_NAMES) ==0)
    {
      sprintf(page->lpSupply,"\tFLOAT32 %s = (FLOAT32)MIC_GET_N(%d,${n});",page->lpName,nArgs-page->nId);
      nNCtr++; bRecog = TRUE;
    }
    else if (dlp_strncmp(page->lpType,"char",L_NAMES) ==0)
    {
      sprintf(page->lpSupply,"\tINT8 %s = (INT8)MIC_GET_N(%d,${n});",page->lpName,nArgs-page->nId);
      nNCtr++; bRecog = TRUE;
    }
    else if (dlp_strncmp(page->lpType,"unsigned char",L_NAMES) ==0)
    {
      sprintf(page->lpSupply,"\tUINT8 %s = (UINT8)MIC_GET_N(%d,${n});",page->lpName,nArgs-page->nId);
      nNCtr++; bRecog = TRUE;
    }
    else if (dlp_strncmp(page->lpType,"short",L_NAMES) ==0)
    {
      sprintf(page->lpSupply,"\tINT16 %s = (INT16)MIC_GET_N(%d,${n});",page->lpName,nArgs-page->nId);
      nNCtr++; bRecog = TRUE;
    }
    else if (dlp_strncmp(page->lpType,"unsigned short",L_NAMES) ==0)
    {
      sprintf(page->lpSupply,"\tUINT16 %s = (UINT16)MIC_GET_N(%d,${n});",page->lpName,nArgs-page->nId);
      nNCtr++; bRecog = TRUE;
    }
    else if (dlp_strncmp(page->lpType,"int",L_NAMES) ==0)
    {
      sprintf(page->lpSupply,"\tINT32 %s = (INT32)MIC_GET_N(%d,${n});",page->lpName,nArgs-page->nId);
      nNCtr++; bRecog = TRUE;
    }
    else if (dlp_strncmp(page->lpType,"unsigned int",L_NAMES) ==0)
    {
      sprintf(page->lpSupply,"\tUINT32 %s = (UINT32)MIC_GET_N(%d,${n});",page->lpName,nArgs-page->nId);
      nNCtr++; bRecog = TRUE;
    }
    else if (dlp_strncmp(page->lpType,"long",L_NAMES) ==0)
    {
      sprintf(page->lpSupply,"\tINT64 %s = (INT64)MIC_GET_N(%d,${n});",page->lpName,nArgs-page->nId);
      nNCtr++; bRecog = TRUE;
    }
    else if (dlp_strncmp(page->lpType,"unsigned long",L_NAMES) ==0)
    {
      sprintf(page->lpSupply,"\tUINT64 %s = (UINT64)MIC_GET_N(%d,${n});",page->lpName,nArgs-page->nId);
      nNCtr++; bRecog = TRUE;
    }
    else
    {
      // Anything else is considered to be an instance
      if (dlp_strncmp(page->lpType,"data"        ,L_NAMES)==0) bRecog=TRUE;
      if (dlp_strncmp(page->lpType,"structure"   ,L_NAMES)==0) bRecog=TRUE;     // Deprecated
      if (dlp_strncmp(page->lpType,m_lpsClass    ,L_NAMES)==0) bRecog=TRUE;
      if (dlp_strncmp(page->lpType,m_lpsDlcParent,L_NAMES)==0) bRecog=TRUE;

      if (!bRecog) ERRORMSG(ERR_UNKNOWNARGTYPE,page->lpType,0,0);
      if (page->nArgFlags == AF_IN)
        sprintf(page->lpSupply,"\tconst %s* %s = MIC_GET_I_EX(%s,%s,%d,${o});",page->lpType,page->lpName,page->lpName,page->lpType,nArgs-page->nId);
      else
        sprintf(page->lpSupply,"\t%s* %s = MIC_GET_I_EX(%s,%s,%d,${o});",page->lpType,page->lpName,page->lpName,page->lpType,nArgs-page->nId);
      nOCtr++; bRecog = TRUE; bInstance = TRUE;
    }

    // Assemble UPN syntax descriptor string
    if (dlp_strlen(lpSDescr)) sprintf(lpBuf2," <%s %s>",page->lpType,page->lpName);
    else                     sprintf(lpBuf2,"<%s %s>" ,page->lpType,page->lpName);
    strcat(lpSDescr,lpBuf2);

    // Assemble C++ syntax descriptor string
    if (bInstance)
    {
      if (dlp_strlen(lpCDescr)) sprintf(lpBuf2,", %s* %s",page->lpType,page->lpName);
      else                      sprintf(lpBuf2,"%s* %s"  ,page->lpType,page->lpName);
      ReplaceKey(lpBuf2,"instance*","CDlpObject*",0);
    }
    else
    {
      char lpCType[255];
      if (dlp_strcmp(page->lpType,"string")==0) strcpy(lpCType,"char*");
      else if (dlp_strcmp(page->lpType,"cstring")==0) strcpy(lpCType,"const char*");
      else if (dlp_strcmp(page->lpType,"complex")==0) strcpy(lpCType,"COMPLEX64");
      else if (dlp_strcmp(page->lpType,"double")==0) strcpy(lpCType,"FLOAT64");
      else if (dlp_strcmp(page->lpType,"float")==0) strcpy(lpCType,"FLOAT32");
      else if (dlp_strcmp(page->lpType,"unsigned long")==0) strcpy(lpCType,"UINT64");
      else if (dlp_strcmp(page->lpType,"long")==0) strcpy(lpCType,"INT64");
      else if (dlp_strcmp(page->lpType,"unsigned int")==0) strcpy(lpCType,"UINT32");
      else if (dlp_strcmp(page->lpType,"int")==0) strcpy(lpCType,"INT32");
      else if (dlp_strcmp(page->lpType,"unsigned short")==0) strcpy(lpCType,"UINT16");
      else if (dlp_strcmp(page->lpType,"short")==0) strcpy(lpCType,"INT16");
      else if (dlp_strcmp(page->lpType,"unsigned char")==0) strcpy(lpCType,"UINT8");
      else if (dlp_strcmp(page->lpType,"char")==0) strcpy(lpCType,"INT8");
      else                                      strcpy(lpCType,page->lpType);
      if (dlp_strlen(lpCDescr)) sprintf(lpBuf2,", %s %s",lpCType,page->lpName);
      else                      sprintf(lpBuf2,"%s %s"  ,lpCType,page->lpName);
    }
    strcat(lpCDescr,lpBuf2);

    // Turn over a new leaf...
    page = page->next;
  }
  nOCtr--;
  nNCtr--;
  nSCtr--;
  nBCtr--;
  page = lSyntax.m_items;
  while (page)
  {
    sprintf(n,"%hd",nNCtr);
    sprintf(o,"%hd",nOCtr);
    sprintf(s,"%hd",nSCtr);
    sprintf(b,"%hd",nBCtr);
    nNCtr -= ReplaceKey(page->lpSupply,"${n}",n,0);
    nSCtr -= ReplaceKey(page->lpSupply,"${s}",s,0);
    nBCtr -= ReplaceKey(page->lpSupply,"${b}",b,0);
    if (strstr(page->lpSupply,"${o}"))
    {
      // By agreement object[0] equals 'this', c.e. the client instance itself
      DLPASSERT(nOCtr>0);
      if (nOCtr>0) nOCtr -= ReplaceKey(page->lpSupply,"${o}",o,0);
    }
    page = page->next;
  }

  // Finish UPN syntax desription
  if (dlp_strlen(lpSDescr)) sprintf(lpBuf2," <%s this>",m_lpsClass);
  else                      sprintf(lpBuf2,"<%s this>" ,m_lpsClass);
  strcat(lpSDescr,lpBuf2);
  if (bCustom)
  {
    dlp_strcpy(lpSDescr,lpSyntax);
    dlp_strunquotate(lpSDescr,'\"','\"');
  }

  return O_K;
}

INT16 CGEN_PROTECTED CCgen::ReplaceKey(char* lpText, const char* lpKey, const char* lpReplace, INT16 nCnvt)
{
  if (!dlp_strlen(lpKey )) return 0;
  if (!dlp_strlen(lpText)) return 0;

  char lpBuf[2048]="";
  if (nCnvt == 0) dlp_strcpy(lpBuf,lpReplace);
  else            dlp_convert_name(nCnvt,lpBuf,lpReplace);

  INT16 nReplaced =0;
  char *tx = strstr(lpText,lpKey);
  while (tx != NULL)
  {
    nReplaced++;
    memmove(tx,&tx[dlp_strlen(lpKey)],dlp_strlen(tx)-dlp_strlen(lpKey)+1);
    memmove(&tx[dlp_strlen(lpBuf)],tx,dlp_strlen(tx)+1);
    memmove(tx,lpBuf,dlp_strlen(lpBuf));
    tx = strstr(lpText,lpKey);
  }
  return nReplaced;
}

/**
 * Creates all file templates.
 */
void CGEN_PROTECTED CCgen::CreateTemplates()
{
  if (m_bMainProject)                                                           // dLabPro application
  {                                                                             // >>
    IERROR(this,ERR_CANNOTCREATE,"Source file templates for project type",      //   Unsupported!
      "main",0);                                                                //   |
  }                                                                             // <<
  else                                                                          // dLabPro Class
  {                                                                             // >>
    CreateHTemplate();                                                          //   Header file
    CreateCPPTemplate();                                                        //   C/C++ implementation file
    CreateMakefileTemplate();                                                   //   Make file
    CreateNMakeTemplate();                                                      //   MS Nmake file
    CreateDSPTemplate();                                                        //   MS VC++ 6.0 project file
    CreateDSWTemplate();                                                        //   MS VC++ 6.0 workspace file
  }                                                                             // <<
}

// EOF
