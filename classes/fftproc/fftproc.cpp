// dLabPro class CFFTproc (FFTproc)
// - FFT class based on FBAproc
//
// AUTHOR : Matthias Eichner
// PACKAGE: dLabPro/classes
//
// This file was generated by dcg. DO NOT MODIFY! Modify fftproc.def instead.
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
#include "dlp_fftproc.h"

// Class CFFTproc

CFFTproc::CFFTproc(const char* lpInstanceName, BOOL bCallVirtual) : inherited(lpInstanceName,0)
{
	DEBUGMSG(-1,"CFFTproc::CFFTproc; (bCallVirtual=%d)",(int)bCallVirtual,0,0);
	dlp_strcpy(m_lpClassName,"FFTproc");
	dlp_strcpy(m_lpObsoleteName,"");
	dlp_strcpy(m_lpProjectName,"FFTproc");
	dlp_strcpy(m_version.no,"1.0 DLP");
	dlp_strcpy(m_version.date,"");
	m_nClStyle = CS_AUTOACTIVATE;

	if (bCallVirtual)
	{
		DLPASSERT(OK(AutoRegisterWords()));
		Init(TRUE);
	}
}

CFFTproc::~CFFTproc()
{
  //{{CGEN_DONECODE
  DONE;
  //}}CGEN_DONECODE
}

INT16 CFFTproc::AutoRegisterWords()
{
	DEBUGMSG(-1,"CFFTproc::AutoRegisterWords",0,0,0);
	IF_NOK(inherited::AutoRegisterWords()) return NOT_EXEC;

	//{{CGEN_REGISTERWORDS
	REGISTER_METHOD("-analyze","",LPMF(CFFTproc,OnAnalyze),"Run FFT.",0,"<data idSignal> <data idPitch> <data idReal> <data idImag> <FFTproc this>","")
	REGISTER_METHOD("-warp","",LPMF(CFFTproc,OnWarp),"Warping",0,"<data in> <data out> <double lambda> <int odim> <FFTproc this>","")
	REGISTER_OPTION("/lmag","",LPMV(m_bLmag),LPMF(CFFTproc,OnLmagSet),"Compute logarithm of magnitude spectrum rather than complex spectrum.",0)
	REGISTER_OPTION("/mag","",LPMV(m_bMag),LPMF(CFFTproc,OnMagSet),"Compute magnitude spectrum rather than complex spectrum.",0)
	REGISTER_OPTION("/nse","",LPMV(m_bNse),NULL,"Enable noise reduction",0)
	REGISTER_FIELD("order","",LPMV(m_nOrder),LPMF(CFFTproc,OnOrderChanged),"FFT analysis order. Adjusts analysis length to 2^order if set.",0,2002,1,"short",(INT16)9)
	//}}CGEN_REGISTERWORDS

	return O_K;
}

INT16 CFFTproc::Init(BOOL bCallVirtual)
{
	DEBUGMSG(-1,"CFFTproc::Init, (bCallVirtual=%d)",(int)bCallVirtual,0,0);
	//{{CGEN_INITCODE
  INIT;
	//}}CGEN_INITCODE

	// If last derivation call reset (do not reset members; already done by Init())
	if (bCallVirtual) return Reset(FALSE);
	else              return O_K;
}

INT16 CFFTproc::Reset(BOOL bResetMembers)
{
	DEBUGMSG(-1,"CFFTproc::Reset; (bResetMembers=%d)",(int)bResetMembers,0,0);
	//{{CGEN_RESETCODE
  return RESET;
	//}}CGEN_RESETCODE

	return O_K;
}

INT16 CFFTproc::ClassProc()
{
	//{{CGEN_CLASSCODE
  return CLASSPROC;
	//}}CGEN_CLASSCODE

	return O_K;
}

#define CODE_DN3 /* check this for xml specific save code */
#define SAVE  SAVE_DN3
INT16 CFFTproc::Serialize(CDN3Stream* lpDest)
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
INT16 CFFTproc::SerializeXml(CXmlStream* lpDest)
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
INT16 CFFTproc::Deserialize(CDN3Stream* lpSrc)
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
INT16 CFFTproc::DeserializeXml(CXmlStream* lpSrc)
{
	//{{CGEN_RESTORECODE
  return RESTORE;
	//}}CGEN_RESTORECODE

	return O_K;
}
#undef  RESTORE
#undef  CODE_XML

INT16 CFFTproc::Copy(CDlpObject* __iSrc)
{
	//{{CGEN_COPYCODE
  return COPY;
	//}}CGEN_COPYCODE

	return O_K;
}

// Runtime class type information and class factory
INT16 CFFTproc::InstallProc(void* lpItp)
{
	//{{CGEN_INSTALLCODE
  return INSTALL;
	//}}CGEN_INSTALLCODE

	return O_K;
}

CFFTproc* CFFTproc::CreateInstance(const char* lpName)
{
	CFFTproc* lpNewInstance;
	ICREATEEX(CFFTproc,lpNewInstance,lpName,NULL);
	return lpNewInstance;
}

INT16 CFFTproc::GetClassInfo(SWord* lpClassWord)
{
	if (!lpClassWord) return NOT_EXEC;
	dlp_memset(lpClassWord,0,sizeof(SWord));

	lpClassWord->nWordType          = WL_TYPE_FACTORY;
	lpClassWord->nFlags             = CS_AUTOACTIVATE;
	lpClassWord->ex.fct.lpfFactory  = (LP_FACTORY_PROC)CFFTproc::CreateInstance;
	lpClassWord->ex.fct.lpfInstall  = CFFTproc::InstallProc;
	lpClassWord->ex.fct.lpProject   = "FFTproc";
	lpClassWord->ex.fct.lpBaseClass = "FBAproc";
	lpClassWord->lpComment          = "FFT class based on FBAproc";
	lpClassWord->ex.fct.lpAutoname  = "";
	lpClassWord->ex.fct.lpCname     = "CFFTproc";
	lpClassWord->ex.fct.lpAuthor    = "Matthias Eichner";

	dlp_strcpy(lpClassWord->lpName             ,"FFTproc");
	dlp_strcpy(lpClassWord->lpObsname          ,"");
	dlp_strcpy(lpClassWord->ex.fct.version.no  ,"1.0 DLP");

	return O_K;
}

INT16 CFFTproc::GetInstanceInfo(SWord* lpClassWord)
{
	return CFFTproc::GetClassInfo(lpClassWord);
}

BOOL CFFTproc::IsKindOf(const char* lpClassName)
{
  if (dlp_strncmp(lpClassName,"FFTproc",L_NAMES) == 0) return TRUE;
  else return inherited::IsKindOf(lpClassName);
}

INT16 CFFTproc::ResetAllOptions(BOOL bInit)
{
	DEBUGMSG(-1,"CFFTproc::ResetAllOptions;",0,0,0);
	//{{CGEN_RESETALLOPTIONS
	_this->m_bLmag = FALSE;
	_this->m_bMag = FALSE;
	_this->m_bNse = FALSE;
	//}}CGEN_RESETALLOPTIONS

	return inherited::ResetAllOptions(bInit);
}

// Generated primary method invocation functions

#ifndef __NOITP
//{{CGEN_PMIC
INT16 CFFTproc::OnAnalyze()
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

INT16 CFFTproc::OnWarp()
/* DO NOT CALL THIS FUNCTION FROM C++ SCOPE.     */
/* IT MAY INTERFERE WITH THE INTERPRETER SESSION */
{
	INT16 __nErr    = O_K;
	INT32  __nErrCnt = 0;
	MIC_CHECK;
	__nErrCnt = CDlpObject_GetErrorCount();
	INT32 odim = (INT32)MIC_GET_N(1,0);
	FLOAT64 lambda = MIC_GET_N(2,1);
	data* out = MIC_GET_I_EX(out,data,3,1);
	data* in = MIC_GET_I_EX(in,data,4,2);
	if (CDlpObject_GetErrorCount()>__nErrCnt) return NOT_EXEC;
	__nErr = Warp(in, out, lambda, odim);
	return __nErr;
}

//}}CGEN_PMIC
#endif /* #ifndef __NOITP */


// Generated secondary method invocation functions

//{{CGEN_SMIC
//}}CGEN_SMIC


// Generated option change callback functions

//{{CGEN_OCCF
INT16 CFFTproc::OnLmagSet()
{
    m_bMag=FALSE;

	return O_K;
}

INT16 CFFTproc::OnMagSet()
{
    m_bLmag=FALSE;

	return O_K;
}

//}}CGEN_OCCF


// Generated field change callback functions

//{{CGEN_FCCF
INT16 CFFTproc::OnOrderChanged()
{
    m_nLen=(INT16)dlm_pow(2,m_nOrder);
    m_nOutDim = m_nLen/2;

	return O_K;
}

//}}CGEN_FCCF


// EOF
