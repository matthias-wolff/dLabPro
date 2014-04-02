// dLabPro class CProcess (process)
// - Parallel computing processor
//
// AUTHOR : Matthias Wolff
// PACKAGE: dLabPro/classes
//
// This file was generated by dcg. DO NOT MODIFY! Modify process.def instead.
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


//{{CGEN_INCLUDE
//}}CGEN_END
#include "dlp_process.h"

// Class CProcess

CProcess::CProcess(const char* lpInstanceName, BOOL bCallVirtual) : inherited(lpInstanceName,0)
{
	DEBUGMSG(-1,"CProcess::CProcess; (bCallVirtual=%d)",(int)bCallVirtual,0,0);
	dlp_strcpy(m_lpClassName,"process");
	dlp_strcpy(m_lpObsoleteName,"");
	dlp_strcpy(m_lpProjectName,"process");
	dlp_strcpy(m_version.no,"1.0.0");
	dlp_strcpy(m_version.date,"");
	m_nClStyle = CS_AUTOACTIVATE;

	if (bCallVirtual)
	{
		DLPASSERT(OK(AutoRegisterWords()));
		Init(TRUE);
	}
}

CProcess::~CProcess()
{
  //{{CGEN_DONECODE
  DONE;
  //}}CGEN_DONECODE
}

INT16 CProcess::AutoRegisterWords()
{
	DEBUGMSG(-1,"CProcess::AutoRegisterWords",0,0,0);
	IF_NOK(inherited::AutoRegisterWords()) return NOT_EXEC;

	//{{CGEN_REGISTERWORDS

	// Register methods
	REGISTER_METHOD("-marshal_retval","",LPMF(CProcess,MarshalRetval),"-- For internal use only! --",0,"[...] <object iDto> <process this>","")
	REGISTER_METHOD("-start","",LPMF(CProcess,Start),"Starts (a function as) a new process and returns immediately",0,"[...] <process this>","")
	REGISTER_METHOD("-status","",LPMF(CProcess,Status),"Print status information",0,"<process this>","")
	REGISTER_METHOD("-wait","",LPMF(CProcess,Wait),"Waits for the process to be completed",0,"<process this>","")
	REGISTER_METHOD("?complete","",LPMF(CProcess,OnIsComplete),"Returns TRUE if the process has completed",0,"<process this>","")
	REGISTER_METHOD("?running","",LPMF(CProcess,OnIsRunning),"Returns TRUE if the process is running",0,"<process this>","")

	// Register options
	REGISTER_OPTION("/global","",LPMV(m_bGlobal),NULL,"Make global instances available to function processes",0)
	REGISTER_OPTION("/include","",LPMV(m_bInclude),NULL,"Make includes available to function processes",0)

	// Register fields
	REGISTER_FIELD("cmd_line","",LPMV(m_psCmdLine),NULL,"Command line of the processes executable",FF_NOSET | FF_NOSAVE,5000,1,"string",NULL)
	REGISTER_FIELD("dto","",LPMV(m_iDto),NULL,"Data transfer object",FF_NOSET | FF_NOSAVE,6002,1,"",NULL)
	REGISTER_FIELD("pid","",LPMV(m_hPid),NULL,"Process id of client thread (for fork)",FF_HIDDEN | FF_NOSET | FF_NOSAVE | FF_NONAUTOMATIC,10000,1,"PROCESSID",0)
	REGISTER_FIELD("ret_val","",LPMV(m_nRetVal),NULL,"Return value of the processes executable",FF_NOSET | FF_NOSAVE,2004,1,"int",(INT32)-1)
	REGISTER_FIELD("state","",LPMV(m_nState),NULL,"Process state",FF_NOSET | FF_NOSAVE,1002,1,"unsigned short",(UINT16)0)
	REGISTER_FIELD("thread","",LPMV(m_hThread),NULL,"Watcher thread's handle",FF_HIDDEN | FF_NOSET | FF_NOSAVE | FF_NONAUTOMATIC,10000,1,"THREADHANDLE",0)
	REGISTER_FIELD("tmp_file","",LPMV(m_psTmpFile),NULL,"Prefix of this processes temporary files",FF_NOSET | FF_NOSAVE,5000,1,"string",NULL)

	// Register errors
	REGISTER_ERROR("~e1_0_0__1",EL_ERROR,PRC_TOOFEWARGS,"Too few arguments.")
	REGISTER_ERROR("~e2_0_0__1",EL_ERROR,PRC_CANTSTART,"Cannot start process. Reason: %s.")
	REGISTER_ERROR("~e3_0_0__1",EL_ERROR,PRC_CANTWAIT,"Cannot wait for process. Reason: %s.")
	//}}CGEN_REGISTERWORDS

	return O_K;
}

INT16 CProcess::Init(BOOL bCallVirtual)
{
	DEBUGMSG(-1,"CProcess::Init, (bCallVirtual=%d)",(int)bCallVirtual,0,0);
	//{{CGEN_INITCODE
  INIT;
	//}}CGEN_INITCODE

	// If last derivation call reset (do not reset members; already done by Init())
	if (bCallVirtual) return Reset(FALSE);
	else              return O_K;
}

INT16 CProcess::Reset(BOOL bResetMembers)
{
	DEBUGMSG(-1,"CProcess::Reset; (bResetMembers=%d)",(int)bResetMembers,0,0);
	//{{CGEN_RESETCODE
  RESET;
  if (m_iDto) IDESTROY(m_iDto); m_iDto=NULL;
  m_psTmpFile = (char*)dlp_realloc(m_psTmpFile,L_PATH     ,sizeof(char));
  m_psCmdLine = (char*)dlp_realloc(m_psCmdLine,L_INPUTLINE,sizeof(char));
	//}}CGEN_RESETCODE

	return O_K;
}

INT16 CProcess::ClassProc()
{
	//{{CGEN_CLASSCODE
  return CLASSPROC;
	//}}CGEN_CLASSCODE

	return O_K;
}

#define CODE_DN3 /* check this for xml specific save code */
#define SAVE  SAVE_DN3
INT16 CProcess::Serialize(CDN3Stream* lpDest)
{
	//{{CGEN_SAVECODE
  return SAVE;
	//}}CGEN_SAVECODE

	return O_K;
}
#undef  SAVE
#undef  CODE_DN3

#define CODE_XML /* check this for xml specific save code */
#define SAVE  SAVE_XML
INT16 CProcess::SerializeXml(CXmlStream* lpDest)
{
	//{{CGEN_SAVECODE
  return SAVE;
	//}}CGEN_SAVECODE

	return O_K;
}
#undef  SAVE
#undef  CODE_XML

#define CODE_DN3 /* check this for dn3 specific restore code */
#define RESTORE  RESTORE_DN3
INT16 CProcess::Deserialize(CDN3Stream* lpSrc)
{
	//{{CGEN_RESTORECODE
  return RESTORE;
	//}}CGEN_RESTORECODE

	return O_K;
}
#undef  RESTORE
#undef  CODE_DN3

#define CODE_XML /* check this for xml specific restore code */
#define RESTORE  RESTORE_XML
INT16 CProcess::DeserializeXml(CXmlStream* lpSrc)
{
	//{{CGEN_RESTORECODE
  return RESTORE;
	//}}CGEN_RESTORECODE

	return O_K;
}
#undef  RESTORE
#undef  CODE_XML

INT16 CProcess::Copy(CDlpObject* __iSrc)
{
	//{{CGEN_COPYCODE
  return COPY;
	//}}CGEN_COPYCODE

	return O_K;
}

// Runtime class type information and class factory
INT16 CProcess::InstallProc(void* lpItp)
{
	//{{CGEN_INSTALLCODE
  return INSTALL;
	//}}CGEN_INSTALLCODE

	return O_K;
}

CProcess* CProcess::CreateInstance(const char* lpName)
{
	CProcess* lpNewInstance;
	ICREATEEX(CProcess,lpNewInstance,lpName,NULL);
	return lpNewInstance;
}

INT16 CProcess::GetClassInfo(SWord* lpClassWord)
{
	if (!lpClassWord) return NOT_EXEC;
	dlp_memset(lpClassWord,0,sizeof(SWord));

	lpClassWord->nWordType          = WL_TYPE_FACTORY;
	lpClassWord->nFlags             = CS_AUTOACTIVATE;
	lpClassWord->ex.fct.lpfFactory  = (LP_FACTORY_PROC)CProcess::CreateInstance;
	lpClassWord->ex.fct.lpfInstall  = CProcess::InstallProc;
	lpClassWord->ex.fct.lpProject   = "process";
	lpClassWord->ex.fct.lpBaseClass = "-";
	lpClassWord->lpComment          = "Parallel computing processor";
	lpClassWord->ex.fct.lpAutoname  = "";
	lpClassWord->ex.fct.lpCname     = "CProcess";
	lpClassWord->ex.fct.lpAuthor    = "Matthias Wolff";

	dlp_strcpy(lpClassWord->lpName             ,"process");
	dlp_strcpy(lpClassWord->lpObsname          ,"");
	dlp_strcpy(lpClassWord->ex.fct.version.no  ,"1.0.0");

	return O_K;
}

INT16 CProcess::GetInstanceInfo(SWord* lpClassWord)
{
	return CProcess::GetClassInfo(lpClassWord);
}

BOOL CProcess::IsKindOf(const char* lpClassName)
{
  if (dlp_strncmp(lpClassName,"process",L_NAMES) == 0) return TRUE;
  else return inherited::IsKindOf(lpClassName);
}

INT16 CProcess::ResetAllOptions(BOOL bInit)
{
	DEBUGMSG(-1,"CProcess::ResetAllOptions;",0,0,0);
	//{{CGEN_RESETALLOPTIONS
	_this->m_bGlobal = FALSE;
	_this->m_bInclude = FALSE;
	//}}CGEN_RESETALLOPTIONS

	return inherited::ResetAllOptions(bInit);
}

// Generated primary method invocation functions

#ifndef __NOITP
//{{CGEN_PMIC
INT16 CProcess::OnIsComplete()
/* DO NOT CALL THIS FUNCTION FROM C++ SCOPE.     */
/* IT MAY INTERFERE WITH THE INTERPRETER SESSION */
{
	INT16 __nErr    = O_K;
	INT32  __nErrCnt = 0;
	MIC_CHECK;
	__nErrCnt = CDlpObject_GetErrorCount();
	if (CDlpObject_GetErrorCount()>__nErrCnt) return NOT_EXEC;
	MIC_PUT_B(IsComplete());
	return __nErr;
}

INT16 CProcess::OnIsRunning()
/* DO NOT CALL THIS FUNCTION FROM C++ SCOPE.     */
/* IT MAY INTERFERE WITH THE INTERPRETER SESSION */
{
	INT16 __nErr    = O_K;
	INT32  __nErrCnt = 0;
	MIC_CHECK;
	__nErrCnt = CDlpObject_GetErrorCount();
	if (CDlpObject_GetErrorCount()>__nErrCnt) return NOT_EXEC;
	MIC_PUT_B(IsRunning());
	return __nErr;
}

//}}CGEN_PMIC
#endif /* #ifndef __NOITP */


// Generated secondary method invocation functions

//{{CGEN_SMIC
BOOL CProcess::IsComplete()
{
    return (m_nState & PRC_COMPLETE);
	return FALSE;
}

BOOL CProcess::IsRunning()
{
    return (m_nState & PRC_RUNNING);
	return FALSE;
}

//}}CGEN_SMIC


// Generated option change callback functions

//{{CGEN_OCCF
//}}CGEN_OCCF


// Generated field change callback functions

//{{CGEN_FCCF
//}}CGEN_FCCF


// EOF
