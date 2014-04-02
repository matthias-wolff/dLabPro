/* dLabPro class CSignal (signal)
 * - Signal operations
 *
 * AUTHOR : Guntram Strecha
 * PACKAGE: dLabPro/classes
 *
 * This file was generated by dcg. DO NOT MODIFY! Modify signal.def instead.
 * 
 * Copyright 2013 dLabPro contributors and others (see COPYRIGHT file) 
 * - Chair of System Theory and Speech Technology, TU Dresden
 * - Chair of Communications Engineering, BTU Cottbus
 * 
 * This file is part of dLabPro.
 * 
 * dLabPro is free software: you can redistribute it and/or modify it under the
 * terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option)
 * any later version.
 * 
 * dLabPro is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
 * details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with dLabPro. If not, see <http://www.gnu.org/licenses/>.
 */

#include "dlp_cscope.h" /* Indicate C scope */
/*{{CGEN_INCLUDE */
/*}}CGEN_END */
#include "dlp_signal.h"

/* Class CSignal */

void CSignal_Constructor(CSignal* _this, const char* lpInstanceName, BOOL bCallVirtual)
{
	DEBUGMSG(-1,"CSignal_Constructor; (bCallVirtual=%d)",(int)bCallVirtual,0,0);

#ifndef __cplusplus

	/* Register instance */
	dlp_xalloc_register_object('J',_this,1,sizeof(CSignal),
		__FILE__,__LINE__,"signal",lpInstanceName);

	/* Create base instance */
	_this->m_lpBaseInstance = calloc(1,sizeof(CDlpObject));
	CDlpObject_Constructor(_this->m_lpBaseInstance,lpInstanceName,FALSE);

	/* Override virtual member functions */
	_this->m_lpBaseInstance->AutoRegisterWords = CSignal_AutoRegisterWords;
	_this->m_lpBaseInstance->Reset             = CSignal_Reset;
	_this->m_lpBaseInstance->Init              = CSignal_Init;
	_this->m_lpBaseInstance->Serialize         = CSignal_Serialize;
	_this->m_lpBaseInstance->SerializeXml      = CSignal_SerializeXml;
	_this->m_lpBaseInstance->Deserialize       = CSignal_Deserialize;
	_this->m_lpBaseInstance->DeserializeXml    = CSignal_DeserializeXml;
	_this->m_lpBaseInstance->Copy              = CSignal_Copy;
	_this->m_lpBaseInstance->ClassProc         = CSignal_ClassProc;
	_this->m_lpBaseInstance->GetInstanceInfo   = CSignal_GetInstanceInfo;
	_this->m_lpBaseInstance->IsKindOf          = CSignal_IsKindOf;
	_this->m_lpBaseInstance->Destructor        = CSignal_Destructor;
	_this->m_lpBaseInstance->ResetAllOptions   = CSignal_ResetAllOptions;

	/* Override pointer to derived instance */
	_this->m_lpBaseInstance->m_lpDerivedInstance = _this;

	#endif /* #ifndef __cplusplus */

	dlp_strcpy(BASEINST(_this)->m_lpClassName,"signal");
	dlp_strcpy(BASEINST(_this)->m_lpObsoleteName,"");
	dlp_strcpy(BASEINST(_this)->m_lpProjectName,"signal");
	dlp_strcpy(BASEINST(_this)->m_version.no,"1.0.0");
	dlp_strcpy(BASEINST(_this)->m_version.date,"");
	BASEINST(_this)->m_nClStyle = CS_SINGLETON;

	if (bCallVirtual)
	{
		DLPASSERT(OK(INVOKE_VIRTUAL_0(AutoRegisterWords)));
		INVOKE_VIRTUAL_1(Init,TRUE);
	}
}

void CSignal_Destructor(CDlpObject* __this)
{
	GET_THIS_VIRTUAL(CSignal);
	{
	/*{{CGEN_DONECODE */
  DONE;
	/*}}CGEN_DONECODE */
	}

#ifndef __cplusplus

	/* Destroy base instance */
	CDlpObject_Destructor(_this->m_lpBaseInstance);
	dlp_free(_this->m_lpBaseInstance);
	_this->m_lpBaseInstance = NULL;

#endif /* #ifndef __cplusplus */
}

INT16 CSignal_AutoRegisterWords(CDlpObject* __this)
{
	GET_THIS_VIRTUAL_RV(CSignal,NOT_EXEC);
	DEBUGMSG(-1,"CSignal_AutoRegisterWords",0,0,0);

	/* Call base class implementation */
	IF_NOK(INVOKE_BASEINST_0(AutoRegisterWords)) return NOT_EXEC;

	/*{{CGEN_REGISTERWORDS */

	/* Register errors */
	REGISTER_ERROR("~e1_0_0__1",EL_ERROR,FOP_ERR_FAIL,"Execution of \"%s\" failed.")
	REGISTER_ERROR("~e2_0_0__1",EL_ERROR,FOP_ERR_NOTSUPP,"Operation %s is not implemented yet or not supported.")
	REGISTER_ERROR("~e3_0_0__1",EL_ERROR,FOP_ERR_INVARG,"Argument %s = \"%s\" is invalid%s.")
	REGISTER_ERROR("~e4_0_0__1",EL_ERROR,FOP_ERR_INVDESCR,"Value of %s is not valid.")
	/*}}CGEN_REGISTERWORDS */

	return O_K;
}

INT16 CSignal_Init(CDlpObject* __this, BOOL bCallVirtual)
{
	GET_THIS_VIRTUAL_RV(CSignal,NOT_EXEC);
	DEBUGMSG(-1,"CSignal_Init, (bCallVirtual=%d)",(int)bCallVirtual,0,0);
	{
	/*{{CGEN_INITCODE */
  INIT;

  /* Register operators */
  INT16 i = 0;
  while(dlp_sigop_entry(i)->opc >= 0) {
    REGISTER_OPERATOR(dlp_sigop_entry(i)->sym,&CSignal_Op,dlp_sigop_entry(i)->opc,dlp_sigop_entry(i)->res,dlp_sigop_entry(i)->ops,dlp_sigop_entry(i)->sig,dlp_sigop_entry(i)->nam);
    i++;
  }
	/*}}CGEN_INITCODE */
	}

	/* If last derivation call reset (do not reset members; already done by Init()) */
#ifndef __NORTTI
	if (bCallVirtual) return INVOKE_VIRTUAL_1(Reset,FALSE); else
#endif
	                  return O_K;
}

INT16 CSignal_Reset(CDlpObject* __this, BOOL bResetMembers)
{
	GET_THIS_VIRTUAL_RV(CSignal,NOT_EXEC);
	DEBUGMSG(-1,"CSignal_Reset; (bResetMembers=%d)",(int)bResetMembers,0,0);
	{
	/*{{CGEN_RESETCODE */
  return RESET;
	/*}}CGEN_RESETCODE */
	}

	return O_K;
}

INT16 CSignal_ClassProc(CDlpObject* __this)
{
	GET_THIS_VIRTUAL_RV(CSignal,NOT_EXEC);
	{
	/*{{CGEN_CLASSCODE */
  return CLASSPROC;
	/*}}CGEN_CLASSCODE */
	}

	return O_K;
}

#define CODE_DN3 /* check this for xml specific save code */
#define SAVE  SAVE_DN3
INT16 CSignal_Serialize(CDlpObject* __this, CDN3Stream* lpDest)
{
	GET_THIS_VIRTUAL_RV(CSignal,NOT_EXEC);
	{
	/*{{CGEN_SAVECODE */
  return SAVE;
	/*}}CGEN_SAVECODE */
	}

	return O_K;
}
#undef  SAVE
#undef  CODE_DN3

#define CODE_XML /* check this for xml specific save code */
#define SAVE  SAVE_XML
INT16 CSignal_SerializeXml(CDlpObject* __this, CXmlStream* lpDest)
{
	GET_THIS_VIRTUAL_RV(CSignal,NOT_EXEC);
	{
	/*{{CGEN_SAVECODE */
  return SAVE;
	/*}}CGEN_SAVECODE */
	}

	return O_K;
}
#undef  SAVE
#undef  CODE_XML

#define CODE_DN3 /* check this for dn3 specific restore code */
#define RESTORE  RESTORE_DN3
INT16 CSignal_Deserialize(CDlpObject* __this, CDN3Stream* lpSrc)
{
	GET_THIS_VIRTUAL_RV(CSignal,NOT_EXEC);
	{
	/*{{CGEN_RESTORECODE */
  return RESTORE;
	/*}}CGEN_RESTORECODE */
	}

	return O_K;
}
#undef  RESTORE
#undef  CODE_DN3

#define CODE_XML /* check this for xml specific restore code */
#define RESTORE  RESTORE_XML
INT16 CSignal_DeserializeXml(CDlpObject* __this, CXmlStream* lpSrc)
{
	GET_THIS_VIRTUAL_RV(CSignal,NOT_EXEC);
	{
	/*{{CGEN_RESTORECODE */
  return RESTORE;
	/*}}CGEN_RESTORECODE */
	}

	return O_K;
}
#undef  RESTORE
#undef  CODE_XML

INT16 CSignal_Copy(CDlpObject* __this, CDlpObject* __iSrc)
{
	GET_THIS_VIRTUAL_RV(CSignal,NOT_EXEC);
	{
	/*{{CGEN_COPYCODE */
  return COPY;
	/*}}CGEN_COPYCODE */
	}

	return O_K;
}

/* Runtime class type information and class factory */
INT16 CSignal_InstallProc(void* lpItp)
{
	{
	/*{{CGEN_INSTALLCODE */
  return INSTALL;
	/*}}CGEN_INSTALLCODE */
	}

	return O_K;
}

CSignal* CSignal_CreateInstance(const char* lpName)
{
	CSignal* lpNewInstance;
	ICREATEEX(CSignal,lpNewInstance,lpName,NULL);
	return lpNewInstance;
}

INT16 CSignal_GetClassInfo(SWord* lpClassWord)
{
	if (!lpClassWord) return NOT_EXEC;
	dlp_memset(lpClassWord,0,sizeof(SWord));

	lpClassWord->nWordType          = WL_TYPE_FACTORY;
	lpClassWord->nFlags             = CS_SINGLETON;

#ifdef __cplusplus

	lpClassWord->ex.fct.lpfFactory  = (LP_FACTORY_PROC)CSignal::CreateInstance;
	lpClassWord->ex.fct.lpfInstall  = CSignal::InstallProc;

#else /* #ifdef __DLP_CSCOPE */

	lpClassWord->ex.fct.lpfFactory  = (LP_FACTORY_PROC)CSignal_CreateInstance;
	lpClassWord->ex.fct.lpfInstall  = CSignal_InstallProc;

#endif /* #ifdef __DLP_CSCOPE */

	lpClassWord->ex.fct.lpProject   = "signal";
	lpClassWord->ex.fct.lpBaseClass = "-";
	lpClassWord->lpComment          = "Signal operations";
	lpClassWord->ex.fct.lpAutoname  = "";
	lpClassWord->ex.fct.lpCname     = "CSignal";
	lpClassWord->ex.fct.lpAuthor    = "Guntram Strecha";

	dlp_strcpy(lpClassWord->lpName             ,"signal");
	dlp_strcpy(lpClassWord->lpObsname          ,"");
	dlp_strcpy(lpClassWord->ex.fct.version.no  ,"1.0.0");

	return O_K;
}

INT16 CSignal_GetInstanceInfo(CDlpObject* __this, SWord* lpClassWord)
{
	return CSignal_GetClassInfo(lpClassWord);
}

BOOL CSignal_IsKindOf(CDlpObject* __this, const char* lpClassName)
{
	GET_THIS_VIRTUAL_RV(CSignal,NOT_EXEC);

  if (dlp_strncmp(lpClassName,"signal",L_NAMES) == 0) return TRUE;
	else return INVOKE_BASEINST_1(IsKindOf,lpClassName);
}

INT16 CSignal_ResetAllOptions(CDlpObject* __this, BOOL bInit)
{
	GET_THIS_VIRTUAL_RV(CSignal,NOT_EXEC);
	DEBUGMSG(-1,"CSignal_ResetAllOptions;",0,0,0);
	{
	/*{{CGEN_RESETALLOPTIONS*/
	/*}}CGEN_RESETALLOPTIONS*/
	}

	return INVOKE_BASEINST_1(ResetAllOptions,bInit);
}

/* Generated primary method invocation functions */

#ifndef __NOITP
/*{{CGEN_CPMIC */
/*}}CGEN_CPMIC */
#endif /* #ifndef __NOITP */


/* Generated secondary method invocation functions */

/*{{CGEN_CSMIC */
/*}}CGEN_CSMIC */


/* Generated option change callback functions */

/*{{CGEN_COCCF */
/*}}CGEN_COCCF */


/* Generated field change callback functions */

/*{{CGEN_CFCCF */
/*}}CGEN_CFCCF */


/* C++ wrapper functions */
#ifdef __cplusplus

#define _this this

CSignal::CSignal(const char* lpInstanceName, BOOL bCallVirtual) : inherited(lpInstanceName,0)
{
	CSignal_Constructor(this,lpInstanceName,bCallVirtual);
}

CSignal::~CSignal()
{
	CSignal_Destructor(this);
}

INT16 CSignal::AutoRegisterWords()
{
	return CSignal_AutoRegisterWords(this);
}

INT16 CSignal::Init(BOOL bCallVirtual)
{
	return CSignal_Init(this,bCallVirtual);
}

INT16 CSignal::Reset(BOOL bResetMembers)
{
	return CSignal_Reset(this,bResetMembers);
}

INT16 CSignal::ClassProc()
{
	return CSignal_ClassProc(this);
}

INT16 CSignal::Serialize(CDN3Stream* lpDest)
{
	return CSignal_Serialize(this,lpDest);
}

INT16 CSignal::SerializeXml(CXmlStream* lpDest)
{
	return CSignal_SerializeXml(this,lpDest);
}

INT16 CSignal::Deserialize(CDN3Stream* lpSrc)
{
	return CSignal_Deserialize(this,lpSrc);
}

INT16 CSignal::DeserializeXml(CXmlStream* lpSrc)
{
	return CSignal_DeserializeXml(this,lpSrc);
}

INT16 CSignal::Copy(CDlpObject* __iSrc)
{
	return CSignal_Copy(this,__iSrc);
}

INT16 CSignal::InstallProc(void* lpItp)
{
	return CSignal_InstallProc(lpItp);
}

CSignal* CSignal::CreateInstance(const char* lpName)
{
	return CSignal_CreateInstance(lpName);
}

INT16 CSignal::GetClassInfo(SWord* lpClassWord)
{
	return CSignal_GetClassInfo(lpClassWord);
}

INT16 CSignal::GetInstanceInfo(SWord* lpClassWord)
{
	return CSignal_GetInstanceInfo(this,lpClassWord);
}

BOOL CSignal::IsKindOf(const char* lpClassName)
{
	return CSignal_IsKindOf(this,lpClassName);
}

INT16 CSignal::ResetAllOptions(BOOL bInit)
{
	return CSignal_ResetAllOptions(this,bInit);
}

#ifndef __NOITP
/*{{CGEN_PMIC */
/*}}CGEN_PMIC */
#endif /* #ifndef __NOITP */

/*{{CGEN_SMIC */
/*}}CGEN_SMIC */

/*{{CGEN_OCCF */
/*}}CGEN_OCCF */

/*{{CGEN_FCCF */
/*}}CGEN_FCCF */

/*{{CGEN_CXXWRAP */
INT16 CSignal::Op(INT16 nOpc, StkItm* R, StkItm* P)
{
	return CSignal_Op(nOpc, R, P);
}

INT16 CSignal::Cep2Lpc(CData* idA, CData* idG, CData* idC, INT32 n)
{
	return CSignal_Cep2Lpc(idA, idG, idC, n);
}

INT16 CSignal::Cep2MCep(CData* idC2, CData* idC1, FLOAT64 nLambda2, INT32 n)
{
	return CSignal_Cep2MCep(idC2, idC1, nLambda2, n);
}

INT16 CSignal::Cep(CData* idY, CData* idX, INT32 nCoeff)
{
	return CSignal_Cep(idY, idX, nCoeff);
}

INT16 CSignal::DeFrame(CData* idY, CData* idX, INT32 nLen)
{
	return CSignal_DeFrame(idY, idX, nLen);
}

INT16 CSignal::DeScale(CData* idY, CData* idX)
{
	return CSignal_DeScale(idY, idX);
}

INT16 CSignal::Distribution(CData* idY, CData* idX, CData* idP)
{
	return CSignal_Distribution(idY, idX, idP);
}

INT16 CSignal::Dtw(CData* idP, CData* idS, CData* idD)
{
	return CSignal_Dtw(idP, idS, idD);
}

INT16 CSignal::F02Exc(CData* idE, CData* idF, INT32 nF, INT32 nL, const char* sT)
{
	return CSignal_F02Exc(idE, idF, nF, nL, sT);
}

INT16 CSignal::Fft(CData* idY, CData* idX)
{
	return CSignal_Fft(idY, idX);
}

INT16 CSignal::Filter(CData* Y, CData* X, CData* B, CData* A, CData* M)
{
	return CSignal_Filter(Y, X, B, A, M);
}

INT16 CSignal::Fir(CData* Y, CData* X, CData* B, CData* M)
{
	return CSignal_Fir(Y, X, B, M);
}

INT16 CSignal::Frame(CData* idY, CData* idX, INT32 nLen, INT32 nStep)
{
	return CSignal_Frame(idY, idX, nLen, nStep);
}

INT16 CSignal::GCep(CData* idY, CData* idX, FLOAT64 nGamma, INT32 nCoeff)
{
	return CSignal_GCep(idY, idX, nGamma, nCoeff);
}

INT16 CSignal::GCep2GCep(CData* idY, CData* idX, FLOAT64 nGamma2, INT32 n)
{
	return CSignal_GCep2GCep(idY, idX, nGamma2, n);
}

INT16 CSignal::GCep2Lpc(CData* idY, CData* idG, CData* idX, INT32 n)
{
	return CSignal_GCep2Lpc(idY, idG, idX, n);
}

INT16 CSignal::GCep2MLpc(CData* idY, CData* idG, CData* idX, FLOAT64 nLambda2, INT32 n)
{
	return CSignal_GCep2MLpc(idY, idG, idX, nLambda2, n);
}

INT16 CSignal::GCepNorm(CData* idY, CData* idG, CData* idX)
{
	return CSignal_GCepNorm(idY, idG, idX);
}

INT16 CSignal::GetF0(CData* idY, CData* idX, INT32 nMin, INT32 nMax, const char* sTyp)
{
	return CSignal_GetF0(idY, idX, nMin, nMax, sTyp);
}

INT16 CSignal::GMult(CData* idY, CData* idX)
{
	return CSignal_GMult(idY, idX);
}

INT16 CSignal::IFft(CData* idY, CData* idX)
{
	return CSignal_IFft(idY, idX);
}

INT16 CSignal::IGCepNorm(CData* idY, CData* idG, CData* idX)
{
	return CSignal_IGCepNorm(idY, idG, idX);
}

INT16 CSignal::IGMult(CData* idY, CData* idX)
{
	return CSignal_IGMult(idY, idX);
}

INT16 CSignal::Iir(CData* Y, CData* X, CData* A, CData* M)
{
	return CSignal_Iir(Y, X, A, M);
}

INT16 CSignal::IMCep(CData* idY, CData* idC, CData* idE, FLOAT64 nLambda)
{
	return CSignal_IMCep(idY, idC, idE, nLambda);
}

INT16 CSignal::IMlt(CData* idY, CData* idX)
{
	return CSignal_IMlt(idY, idX);
}

INT16 CSignal::Lpc(CData* idY, CData* idG, CData* idX, INT32 nCoeff, const char* lpsMethod)
{
	return CSignal_Lpc(idY, idG, idX, nCoeff, lpsMethod);
}

INT16 CSignal::Lpc2Cep(CData* idC, CData* idG, CData* idA, INT32 n)
{
	return CSignal_Lpc2Cep(idC, idG, idA, n);
}

INT16 CSignal::Lpc2GCep(CData* idY, CData* idG, CData* idX, FLOAT64 nGamma2, INT32 n)
{
	return CSignal_Lpc2GCep(idY, idG, idX, nGamma2, n);
}

INT16 CSignal::Lpc2MGCep(CData* idY, CData* idG, CData* idX, FLOAT64 nGamma2, FLOAT64 nLambda2, INT32 n)
{
	return CSignal_Lpc2MGCep(idY, idG, idX, nGamma2, nLambda2, n);
}

INT16 CSignal::Lpc2MLpc(CData* idA2, CData* idG2, CData* idG1, CData* idA1, FLOAT64 nLambda2, INT32 n)
{
	return CSignal_Lpc2MLpc(idA2, idG2, idG1, idA1, nLambda2, n);
}

INT16 CSignal::Lsf2Poly(CData* idY, CData* idX)
{
	return CSignal_Lsf2Poly(idY, idX);
}

INT16 CSignal::MCep2Cep(CData* idC2, CData* idC1, INT32 n)
{
	return CSignal_MCep2Cep(idC2, idC1, n);
}

INT16 CSignal::MCep2MCep(CData* idC2, CData* idC1, FLOAT64 nLambda2, INT32 n)
{
	return CSignal_MCep2MCep(idC2, idC1, nLambda2, n);
}

INT16 CSignal::MCep2MLpc(CData* idY, CData* idG, CData* idX, FLOAT64 nLambda2, INT32 n)
{
	return CSignal_MCep2MLpc(idY, idG, idX, nLambda2, n);
}

INT16 CSignal::MCep(CData* idY, CData* idX, FLOAT64 nLambda, INT32 nCoeff)
{
	return CSignal_MCep(idY, idX, nLambda, nCoeff);
}

INT16 CSignal::Denoise(CData* idY, CData* idX, INT32 nT, FLOAT64 nP, const char* lpsType)
{
	return CSignal_Denoise(idY, idX, nT, nP, lpsType);
}

INT16 CSignal::MCepEnhance(CData* idY, CData* idC)
{
	return CSignal_MCepEnhance(idY, idC);
}

INT16 CSignal::MFb(CData* idY, CData* idX, FLOAT64 nLambda, INT32 nCoeff, const char* lpsMethod)
{
	return CSignal_MFb(idY, idX, nLambda, nCoeff, lpsMethod);
}

INT16 CSignal::Mfft(CData* idY, CData* idX, FLOAT64 nLambda)
{
	return CSignal_Mfft(idY, idX, nLambda);
}

INT16 CSignal::MFilter(CData* idY, CData* idX, CData* idB, CData* idA, FLOAT64 nLambda, CData* idM)
{
	return CSignal_MFilter(idY, idX, idB, idA, nLambda, idM);
}

INT16 CSignal::MFir(CData* Y, CData* X, CData* B, FLOAT64 nLambda, CData* M)
{
	return CSignal_MFir(Y, X, B, nLambda, M);
}

INT16 CSignal::MGCep(CData* idY, CData* idX, FLOAT64 nGamma, FLOAT64 nLambda, INT32 nCoeff)
{
	return CSignal_MGCep(idY, idX, nGamma, nLambda, nCoeff);
}

INT16 CSignal::MGCep2Lpc(CData* idY, CData* idG, CData* idX, INT32 n)
{
	return CSignal_MGCep2Lpc(idY, idG, idX, n);
}

INT16 CSignal::MGCep2MGCep(CData* idY, CData* idX, FLOAT64 nGamma2, FLOAT64 nLambda2, INT32 n)
{
	return CSignal_MGCep2MGCep(idY, idX, nGamma2, nLambda2, n);
}

INT16 CSignal::MGCep2MLpc(CData* idY, CData* idG, CData* idX, FLOAT64 nLambda2, INT32 n)
{
	return CSignal_MGCep2MLpc(idY, idG, idX, nLambda2, n);
}

INT16 CSignal::MIir(CData* Y, CData* X, CData* A, FLOAT64 nLambda, CData* M)
{
	return CSignal_MIir(Y, X, A, nLambda, M);
}

INT16 CSignal::MLpc(CData* idY, CData* idG, CData* idX, FLOAT64 nLambda, INT32 nCoeff, const char* lpsMethod)
{
	return CSignal_MLpc(idY, idG, idX, nLambda, nCoeff, lpsMethod);
}

INT16 CSignal::MLpc2GCep(CData* idY, CData* idG, CData* idX, FLOAT64 nGamma2, INT32 n)
{
	return CSignal_MLpc2GCep(idY, idG, idX, nGamma2, n);
}

INT16 CSignal::MLpc2Lpc(CData* idA2, CData* idG2, CData* idG1, CData* idA1, INT32 n)
{
	return CSignal_MLpc2Lpc(idA2, idG2, idG1, idA1, n);
}

INT16 CSignal::MLpc2MCep(CData* idY, CData* idG, CData* idX, FLOAT64 nLambda2, INT32 n)
{
	return CSignal_MLpc2MCep(idY, idG, idX, nLambda2, n);
}

INT16 CSignal::ISvq(CData* idY, CData* idI, CData* idQ)
{
	return CSignal_ISvq(idY, idI, idQ);
}

INT16 CSignal::IVq(CData* idY, CData* idI, CData* idQ)
{
	return CSignal_IVq(idY, idI, idQ);
}

INT16 CSignal::MLpc2MGCep(CData* idY, CData* idG, CData* idX, FLOAT64 nGamma2, FLOAT64 nLambda2, INT32 n)
{
	return CSignal_MLpc2MGCep(idY, idG, idX, nGamma2, nLambda2, n);
}

INT16 CSignal::MLpc2MLpc(CData* idA2, CData* idG2, CData* idG1, CData* idA1, FLOAT64 nLambda2, INT32 n)
{
	return CSignal_MLpc2MLpc(idA2, idG2, idG1, idA1, nLambda2, n);
}

INT16 CSignal::MLsf2MLsf(CData* idY, CData* idX, FLOAT64 nLambda2)
{
	return CSignal_MLsf2MLsf(idY, idX, nLambda2);
}

INT16 CSignal::Mlt(CData* idY, CData* idX)
{
	return CSignal_Mlt(idY, idX);
}

INT16 CSignal::Noisify(CData* idY, CData* idX)
{
	return CSignal_Noisify(idY, idX);
}

INT16 CSignal::Pitchmark(CData* idY, CData* idX, char* sMethod, INT32 nMin, INT32 nMean, INT32 nMax)
{
	return CSignal_Pitchmark(idY, idX, sMethod, nMin, nMean, nMax);
}

INT16 CSignal::Poly2Lsf(CData* idY, CData* idX)
{
	return CSignal_Poly2Lsf(idY, idX);
}

INT16 CSignal::Rmdc(CData* idY, CData* idX)
{
	return CSignal_Rmdc(idY, idX);
}

INT16 CSignal::Roots(CData* idY, CData* idX)
{
	return CSignal_Roots(idY, idX);
}

INT16 CSignal::Scale(CData* idY, CData* idX, COMPLEX64 nScale)
{
	return CSignal_Scale(idY, idX, nScale);
}

INT16 CSignal::Svq(CData* idQ, CData* idI, CData* idX, CData* idB)
{
	return CSignal_Svq(idQ, idI, idX, idB);
}

INT16 CSignal::Unwrap(CData* idY, CData* idX)
{
	return CSignal_Unwrap(idY, idX);
}

INT16 CSignal::Vq(CData* idQ, CData* idI, CData* idX, INT32 nBits, const char* sMethod)
{
	return CSignal_Vq(idQ, idI, idX, nBits, sMethod);
}

INT16 CSignal::Window(CData* idY, CData* idX, INT32 nLenIn, INT32 nLenOut, const char* lpsWindow, BOOL bNorm)
{
	return CSignal_Window(idY, idX, nLenIn, nLenOut, lpsWindow, bNorm);
}

INT16 CSignal::Zcr(CData* idY, CData* idX, CData* idP)
{
	return CSignal_Zcr(idY, idX, idP);
}

COMPLEX64 CSignal::GetMinQuant(INT16 nType, COMPLEX64 nScale)
{
	return CSignal_GetMinQuant(nType, nScale);
}

INT16 CSignal::GetVar(CData* idSrc, const char* sName, COMPLEX64* nValue)
{
	return CSignal_GetVar(idSrc, sName, nValue);
}

INT16 CSignal::SetVar(CData* idDst, const char* sName, COMPLEX64 nValue)
{
	return CSignal_SetVar(idDst, sName, nValue);
}

INT16 CSignal::GetData(CData* idSrc, const char* sName, CData** iData)
{
	return CSignal_GetData(idSrc, sName, iData);
}

INT16 CSignal::SetData(CData* idDst, const char* sName, CData* iData)
{
	return CSignal_SetData(idDst, sName, iData);
}

INT16 CSignal::ScaleImpl(CData* idY, CData* idX, COMPLEX64 nScale)
{
	return CSignal_ScaleImpl(idY, idX, nScale);
}

INT16 CSignal::SetScale(CData* idDst, COMPLEX64 nScale)
{
	return CSignal_SetScale(idDst, nScale);
}

INT16 CSignal::FftImpl(CData* idY, CData* idX, BOOL bInv)
{
	return CSignal_FftImpl(idY, idX, bInv);
}

/*}}CGEN_CXXWRAP */

#endif /* #ifdef __cplusplus */

/* EOF */
