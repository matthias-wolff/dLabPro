// dLabPro class CLCQproc (LCQproc)
// - Line cepstral quefrencies (LCQ) analysis and synthesis
//
// AUTHOR : Guntram Strecha
// PACKAGE: dLabPro/classes
//
// This file was generated by dcg. DO NOT MODIFY! Modify lcqproc.def instead.
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
#include "dlp_lcqproc.h"

// Class CLCQproc

CLCQproc::CLCQproc(const char* lpInstanceName, BOOL bCallVirtual) : inherited(lpInstanceName,0)
{
	DEBUGMSG(-1,"CLCQproc::CLCQproc; (bCallVirtual=%d)",(int)bCallVirtual,0,0);
	dlp_strcpy(m_lpClassName,"LCQproc");
	dlp_strcpy(m_lpObsoleteName,"");
	dlp_strcpy(m_lpProjectName,"LCQproc");
	dlp_strcpy(m_version.no,"1.0 DLP");
	dlp_strcpy(m_version.date,"");
	m_nClStyle = CS_AUTOACTIVATE;

	if (bCallVirtual)
	{
		DLPASSERT(OK(AutoRegisterWords()));
		Init(TRUE);
	}
}

CLCQproc::~CLCQproc()
{
  //{{CGEN_DONECODE
  DONE;
  //}}CGEN_DONECODE
}

INT16 CLCQproc::AutoRegisterWords()
{
	DEBUGMSG(-1,"CLCQproc::AutoRegisterWords",0,0,0);
	IF_NOK(inherited::AutoRegisterWords()) return NOT_EXEC;

	//{{CGEN_REGISTERWORDS
	REGISTER_METHOD("-analyze","",LPMF(CLCQproc,OnAnalyze),"Run LCQ analysis.",0,"<data idSignal> <data idPitch> <data idReal> <data idImag> <LCQproc this>","")
	REGISTER_METHOD("-status","",LPMF(CLCQproc,Status),"Display status information.",0,"<LCQproc this>","")
	REGISTER_OPTION("/syn_cep_lcq","",LPMV(m_bSynCepLcq),NULL,"Synthesis chronology: lcq->(via expand)->cepstrum->signal",0)
	REGISTER_OPTION("/syn_cep_lcq_filt","",LPMV(m_bSynCepLcqFilt),NULL,"Synthesis chronology: lcq->(via filter)->cepstrum->signal.",0)
	REGISTER_OPTION("/syn_cep_mlcq","",LPMV(m_bSynCepMlcq),NULL,"Synthesis chronology: mlcq->(via expand)->mcepstrum->cepstrum->signal.",0)
	REGISTER_OPTION("/syn_cep_mlcq_filt","",LPMV(m_bSynCepMlcqFilt),NULL,"Synthesis chronology: mlcq->(via filter)->cepstrum->signal.",0)
	REGISTER_OPTION("/syn_lcq","",LPMV(m_bSynLcq),NULL,"Synthesis chronology: (m)lcq->signal.",0)
	REGISTER_OPTION("/syn_mcep_mlcq","",LPMV(m_bSynMcepMlcq),NULL,"Synthesis chronology: mlcq->(via expand)->mcepstrum->signal.",0)
	REGISTER_OPTION("/syn_mcep_mlcq_filt","",LPMV(m_bSynMcepMlcqFilt),NULL,"Synthesis chronology: mlcq->(via filter)->mcepstrum->signal.",0)
	REGISTER_ERROR("~e9_1_0__1",EL_WARNING,ERR_N_ROOTS_UNSTABLE,"%d roots were stabilised.")
	REGISTER_ERROR("~e0_2_0__1",EL_WARNING,ERR_POLY_UNSTABLE,"Polynome is unstable.")
	//}}CGEN_REGISTERWORDS

	return O_K;
}

INT16 CLCQproc::Init(BOOL bCallVirtual)
{
	DEBUGMSG(-1,"CLCQproc::Init, (bCallVirtual=%d)",(int)bCallVirtual,0,0);
	//{{CGEN_INITCODE
  INIT;
	//}}CGEN_INITCODE

	// If last derivation call reset (do not reset members; already done by Init())
	if (bCallVirtual) return Reset(FALSE);
	else              return O_K;
}

INT16 CLCQproc::Reset(BOOL bResetMembers)
{
	DEBUGMSG(-1,"CLCQproc::Reset; (bResetMembers=%d)",(int)bResetMembers,0,0);
	//{{CGEN_RESETCODE
  return RESET;
	//}}CGEN_RESETCODE

	return O_K;
}

INT16 CLCQproc::ClassProc()
{
	//{{CGEN_CLASSCODE
  return CLASSPROC;
	//}}CGEN_CLASSCODE

	return O_K;
}

#define CODE_DN3 /* check this for xml specific save code */
#define SAVE  SAVE_DN3
INT16 CLCQproc::Serialize(CDN3Stream* lpDest)
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
INT16 CLCQproc::SerializeXml(CXmlStream* lpDest)
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
INT16 CLCQproc::Deserialize(CDN3Stream* lpSrc)
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
INT16 CLCQproc::DeserializeXml(CXmlStream* lpSrc)
{
	//{{CGEN_RESTORECODE
  return RESTORE;
	//}}CGEN_RESTORECODE

	return O_K;
}
#undef  RESTORE
#undef  CODE_XML

INT16 CLCQproc::Copy(CDlpObject* __iSrc)
{
	//{{CGEN_COPYCODE
  return COPY;
	//}}CGEN_COPYCODE

	return O_K;
}

// Runtime class type information and class factory
INT16 CLCQproc::InstallProc(void* lpItp)
{
	//{{CGEN_INSTALLCODE
  return INSTALL;
	//}}CGEN_INSTALLCODE

	return O_K;
}

CLCQproc* CLCQproc::CreateInstance(const char* lpName)
{
	CLCQproc* lpNewInstance;
	ICREATEEX(CLCQproc,lpNewInstance,lpName,NULL);
	return lpNewInstance;
}

INT16 CLCQproc::GetClassInfo(SWord* lpClassWord)
{
	if (!lpClassWord) return NOT_EXEC;
	dlp_memset(lpClassWord,0,sizeof(SWord));

	lpClassWord->nWordType          = WL_TYPE_FACTORY;
	lpClassWord->nFlags             = CS_AUTOACTIVATE;
	lpClassWord->ex.fct.lpfFactory  = (LP_FACTORY_PROC)CLCQproc::CreateInstance;
	lpClassWord->ex.fct.lpfInstall  = CLCQproc::InstallProc;
	lpClassWord->ex.fct.lpProject   = "LCQproc";
	lpClassWord->ex.fct.lpBaseClass = "CPproc";
	lpClassWord->lpComment          = "Line cepstral quefrencies (LCQ) analysis and synthesis";
	lpClassWord->ex.fct.lpAutoname  = "";
	lpClassWord->ex.fct.lpCname     = "CLCQproc";
	lpClassWord->ex.fct.lpAuthor    = "Guntram Strecha";

	dlp_strcpy(lpClassWord->lpName             ,"LCQproc");
	dlp_strcpy(lpClassWord->lpObsname          ,"");
	dlp_strcpy(lpClassWord->ex.fct.version.no  ,"1.0 DLP");

	return O_K;
}

INT16 CLCQproc::GetInstanceInfo(SWord* lpClassWord)
{
	return CLCQproc::GetClassInfo(lpClassWord);
}

BOOL CLCQproc::IsKindOf(const char* lpClassName)
{
  if (dlp_strncmp(lpClassName,"LCQproc",L_NAMES) == 0) return TRUE;
  else return inherited::IsKindOf(lpClassName);
}

INT16 CLCQproc::ResetAllOptions(BOOL bInit)
{
	DEBUGMSG(-1,"CLCQproc::ResetAllOptions;",0,0,0);
	//{{CGEN_RESETALLOPTIONS
	_this->m_bSynCepLcq = FALSE;
	_this->m_bSynCepLcqFilt = FALSE;
	_this->m_bSynCepMlcq = FALSE;
	_this->m_bSynCepMlcqFilt = FALSE;
	_this->m_bSynLcq = FALSE;
	_this->m_bSynMcepMlcq = FALSE;
	_this->m_bSynMcepMlcqFilt = FALSE;
	//}}CGEN_RESETALLOPTIONS

	return inherited::ResetAllOptions(bInit);
}

// Generated primary method invocation functions

#ifndef __NOITP
//{{CGEN_PMIC
INT16 CLCQproc::OnAnalyze()
/* DO NOT CALL THIS FUNCTION FROM C++ SCOPE.     */
/* IT MAY INTERFERE WITH THE INTERPRETER SESSION */
{
	INT16 __nErr    = O_K;
	INT32  __nErrCnt = 0;
	MIC_CHECK;
	__nErrCnt = CDlpObject_GetErrorCount();
	data* idImag = MIC_GET_I_EX(idImag,data,1,1);
	data* idReal = MIC_GET_I_EX(idReal,data,2,2);
	data* idPitch = MIC_GET_I_EX(idPitch,data,3,3);
	data* idSignal = MIC_GET_I_EX(idSignal,data,4,4);
	if (CDlpObject_GetErrorCount()>__nErrCnt) return NOT_EXEC;
	__nErr = Analyze(idSignal, idPitch, idReal, idImag);
	return __nErr;
}

//}}CGEN_PMIC
#endif /* #ifndef __NOITP */


// Generated secondary method invocation functions

//{{CGEN_SMIC
//}}CGEN_SMIC


// Generated option change callback functions

//{{CGEN_OCCF
//}}CGEN_OCCF


// Generated field change callback functions

//{{CGEN_FCCF
//}}CGEN_FCCF


// EOF
