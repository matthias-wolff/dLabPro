/* dLabPro class CVmap (vmap)
 * - Vector mapping operator
 *
 * AUTHOR : Christian-M. Westendorf, Matthias Wolff
 * PACKAGE: dLabPro/classes
 *
 * This file was generated by dcg. DO NOT MODIFY! Modify vmap.def instead.
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
#include "dlp_vmap.h"

/* Class CVmap */

void CVmap_Constructor(CVmap* _this, const char* lpInstanceName, BOOL bCallVirtual)
{
	DEBUGMSG(-1,"CVmap_Constructor; (bCallVirtual=%d)",(int)bCallVirtual,0,0);

#ifndef __cplusplus

	/* Register instance */
	dlp_xalloc_register_object('J',_this,1,sizeof(CVmap),
		__FILE__,__LINE__,"vmap",lpInstanceName);

	/* Create base instance */
	_this->m_lpBaseInstance = calloc(1,sizeof(CDlpObject));
	CDlpObject_Constructor(_this->m_lpBaseInstance,lpInstanceName,FALSE);

	/* Override virtual member functions */
	_this->m_lpBaseInstance->AutoRegisterWords = CVmap_AutoRegisterWords;
	_this->m_lpBaseInstance->Reset             = CVmap_Reset;
	_this->m_lpBaseInstance->Init              = CVmap_Init;
	_this->m_lpBaseInstance->Serialize         = CVmap_Serialize;
	_this->m_lpBaseInstance->SerializeXml      = CVmap_SerializeXml;
	_this->m_lpBaseInstance->Deserialize       = CVmap_Deserialize;
	_this->m_lpBaseInstance->DeserializeXml    = CVmap_DeserializeXml;
	_this->m_lpBaseInstance->Copy              = CVmap_Copy;
	_this->m_lpBaseInstance->ClassProc         = CVmap_ClassProc;
	_this->m_lpBaseInstance->GetInstanceInfo   = CVmap_GetInstanceInfo;
	_this->m_lpBaseInstance->IsKindOf          = CVmap_IsKindOf;
	_this->m_lpBaseInstance->Destructor        = CVmap_Destructor;
	_this->m_lpBaseInstance->ResetAllOptions   = CVmap_ResetAllOptions;

	/* Override pointer to derived instance */
	_this->m_lpBaseInstance->m_lpDerivedInstance = _this;

	#endif /* #ifndef __cplusplus */

	dlp_strcpy(BASEINST(_this)->m_lpClassName,"vmap");
	dlp_strcpy(BASEINST(_this)->m_lpObsoleteName,"");
	dlp_strcpy(BASEINST(_this)->m_lpProjectName,"vmap");
	dlp_strcpy(BASEINST(_this)->m_version.no,"1.0.0");
	dlp_strcpy(BASEINST(_this)->m_version.date,"");
	BASEINST(_this)->m_nClStyle = CS_AUTOACTIVATE;

	if (bCallVirtual)
	{
		DLPASSERT(OK(INVOKE_VIRTUAL_0(AutoRegisterWords)));
		INVOKE_VIRTUAL_1(Init,TRUE);
	}
}

void CVmap_Destructor(CDlpObject* __this)
{
	GET_THIS_VIRTUAL(CVmap);
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

INT16 CVmap_AutoRegisterWords(CDlpObject* __this)
{
	GET_THIS_VIRTUAL_RV(CVmap,NOT_EXEC);
	DEBUGMSG(-1,"CVmap_AutoRegisterWords",0,0,0);

	/* Call base class implementation */
	IF_NOK(INVOKE_BASEINST_0(AutoRegisterWords)) return NOT_EXEC;

	/*{{CGEN_REGISTERWORDS */

	/* Register methods */
	REGISTER_METHOD("-map","",LPMF(CVmap,OnMap),"Transform vectors.",0,"<data idSrc> <data idDst> <vmap this>","")
	REGISTER_METHOD("-setup","",LPMF(CVmap,OnSetup),"Setup with vector transformation matrix.",0,"<data idTmx> <string sAop> <string sWop> <double nZero> <vmap this>","")
	REGISTER_METHOD("-setup_i","",LPMF(CVmap,OnSetupI),"Setup with index map and weight table.",0,"<data idImap> <data idWtab> <string sAop> <string sWop> <double nZero> <double nOne> <vmap this>","")
	REGISTER_METHOD("-status","",LPMF(CVmap,OnStatus),"Prints status information of the vector mapping operator.",0,"<vmap this>","")

	/* Register options */
	REGISTER_OPTION("/double","",LPMV(m_bDouble),NULL,"Use double precision floating point numbers.",0)
	REGISTER_OPTION("/float","",LPMV(m_bFloat),NULL,"Use single precision floating point numbers.",0)

	/* Register fields */
	REGISTER_FIELD("aop","",LPMV(m_nAop),NULL,"Aggregation operation (scalar operation code)",FF_NOSET,2002,1,"short",(INT16)-1)
	REGISTER_FIELD("tmx","",LPMV(m_idTmx),NULL,"The vector transformation matrix",FF_NOSET,6002,1,"data",NULL)
	REGISTER_FIELD("type","",LPMV(m_nType),NULL,"Floating point type code for calculations.",FF_NOSET,2002,1,"short",(INT16)T_DOUBLE)
	REGISTER_FIELD("weak_thrsh","",LPMV(m_nWeakThrsh),NULL,"Precentage threshold for usage of weak used matrix algorithm",0,3008,1,"double",(FLOAT64)0.1)
	REGISTER_FIELD("weak_tmx","",LPMV(m_idWeakTmx),NULL,"Vector transformation informations for weak tmx matrix",FF_NOSET,6002,1,"data",NULL)
	REGISTER_FIELD("wop","",LPMV(m_nWop),NULL,"Weighting operation (scalar operation code)",FF_NOSET,2002,1,"short",(INT16)-1)
	REGISTER_FIELD("zero","",LPMV(m_nZero),NULL,"Neutral element of aggregation operation",FF_NOSET,3008,1,"double",(FLOAT64)0.)

	/* Register errors */
	REGISTER_ERROR("~e1_0_0__1",EL_ERROR,VMP_OPCODE,"'%s' is not a valid %s operation name.")
	REGISTER_ERROR("~e2_0_0__1",EL_ERROR,VMP_NOTSETUP,"Mapping operator not properly set up%s.")
	/*}}CGEN_REGISTERWORDS */

	return O_K;
}

INT16 CVmap_Init(CDlpObject* __this, BOOL bCallVirtual)
{
	GET_THIS_VIRTUAL_RV(CVmap,NOT_EXEC);
	DEBUGMSG(-1,"CVmap_Init, (bCallVirtual=%d)",(int)bCallVirtual,0,0);
	{
	/*{{CGEN_INITCODE */
  INIT;
	/*}}CGEN_INITCODE */
	}

	/* If last derivation call reset (do not reset members; already done by Init()) */
#ifndef __NORTTI
	if (bCallVirtual) return INVOKE_VIRTUAL_1(Reset,FALSE); else
#endif
	                  return O_K;
}

INT16 CVmap_Reset(CDlpObject* __this, BOOL bResetMembers)
{
	GET_THIS_VIRTUAL_RV(CVmap,NOT_EXEC);
	DEBUGMSG(-1,"CVmap_Reset; (bResetMembers=%d)",(int)bResetMembers,0,0);
	{
	/*{{CGEN_RESETCODE */
  return RESET;
	/*}}CGEN_RESETCODE */
	}

	return O_K;
}

INT16 CVmap_ClassProc(CDlpObject* __this)
{
	GET_THIS_VIRTUAL_RV(CVmap,NOT_EXEC);
	{
	/*{{CGEN_CLASSCODE */
  return CLASSPROC;
	/*}}CGEN_CLASSCODE */
	}

	return O_K;
}

#define CODE_DN3 /* check this for xml specific save code */
#define SAVE  SAVE_DN3
INT16 CVmap_Serialize(CDlpObject* __this, CDN3Stream* lpDest)
{
	GET_THIS_VIRTUAL_RV(CVmap,NOT_EXEC);
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
INT16 CVmap_SerializeXml(CDlpObject* __this, CXmlStream* lpDest)
{
	GET_THIS_VIRTUAL_RV(CVmap,NOT_EXEC);
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
INT16 CVmap_Deserialize(CDlpObject* __this, CDN3Stream* lpSrc)
{
	GET_THIS_VIRTUAL_RV(CVmap,NOT_EXEC);
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
INT16 CVmap_DeserializeXml(CDlpObject* __this, CXmlStream* lpSrc)
{
	GET_THIS_VIRTUAL_RV(CVmap,NOT_EXEC);
	{
	/*{{CGEN_RESTORECODE */
  return RESTORE;
	/*}}CGEN_RESTORECODE */
	}

	return O_K;
}
#undef  RESTORE
#undef  CODE_XML

INT16 CVmap_Copy(CDlpObject* __this, CDlpObject* __iSrc)
{
	GET_THIS_VIRTUAL_RV(CVmap,NOT_EXEC);
	{
	/*{{CGEN_COPYCODE */
  return COPY;
	/*}}CGEN_COPYCODE */
	}

	return O_K;
}

/* Runtime class type information and class factory */
INT16 CVmap_InstallProc(void* lpItp)
{
	{
	/*{{CGEN_INSTALLCODE */
  return INSTALL;
	/*}}CGEN_INSTALLCODE */
	}

	return O_K;
}

CVmap* CVmap_CreateInstance(const char* lpName)
{
	CVmap* lpNewInstance;
	ICREATEEX(CVmap,lpNewInstance,lpName,NULL);
	return lpNewInstance;
}

INT16 CVmap_GetClassInfo(SWord* lpClassWord)
{
	if (!lpClassWord) return NOT_EXEC;
	dlp_memset(lpClassWord,0,sizeof(SWord));

	lpClassWord->nWordType          = WL_TYPE_FACTORY;
	lpClassWord->nFlags             = CS_AUTOACTIVATE;

#ifdef __cplusplus

	lpClassWord->ex.fct.lpfFactory  = (LP_FACTORY_PROC)CVmap::CreateInstance;
	lpClassWord->ex.fct.lpfInstall  = CVmap::InstallProc;

#else /* #ifdef __DLP_CSCOPE */

	lpClassWord->ex.fct.lpfFactory  = (LP_FACTORY_PROC)CVmap_CreateInstance;
	lpClassWord->ex.fct.lpfInstall  = CVmap_InstallProc;

#endif /* #ifdef __DLP_CSCOPE */

	lpClassWord->ex.fct.lpProject   = "vmap";
	lpClassWord->ex.fct.lpBaseClass = "-";
	lpClassWord->lpComment          = "Vector mapping operator";
	lpClassWord->ex.fct.lpAutoname  = "";
	lpClassWord->ex.fct.lpCname     = "CVmap";
	lpClassWord->ex.fct.lpAuthor    = "Christian-M. Westendorf, Matthias Wolff";

	dlp_strcpy(lpClassWord->lpName             ,"vmap");
	dlp_strcpy(lpClassWord->lpObsname          ,"");
	dlp_strcpy(lpClassWord->ex.fct.version.no  ,"1.0.0");

	return O_K;
}

INT16 CVmap_GetInstanceInfo(CDlpObject* __this, SWord* lpClassWord)
{
	return CVmap_GetClassInfo(lpClassWord);
}

BOOL CVmap_IsKindOf(CDlpObject* __this, const char* lpClassName)
{
	GET_THIS_VIRTUAL_RV(CVmap,NOT_EXEC);

  if (dlp_strncmp(lpClassName,"vmap",L_NAMES) == 0) return TRUE;
	else return INVOKE_BASEINST_1(IsKindOf,lpClassName);
}

INT16 CVmap_ResetAllOptions(CDlpObject* __this, BOOL bInit)
{
	GET_THIS_VIRTUAL_RV(CVmap,NOT_EXEC);
	DEBUGMSG(-1,"CVmap_ResetAllOptions;",0,0,0);
	{
	/*{{CGEN_RESETALLOPTIONS*/
	_this->m_bDouble = FALSE;
	_this->m_bFloat = FALSE;
	/*}}CGEN_RESETALLOPTIONS*/
	}

	return INVOKE_BASEINST_1(ResetAllOptions,bInit);
}

/* Generated primary method invocation functions */

#ifndef __NOITP
/*{{CGEN_CPMIC */
INT16 CVmap_OnMap(CDlpObject* __this)
/* DO NOT CALL THIS FUNCTION FROM C++ SCOPE.     */
/* IT MAY INTERFERE WITH THE INTERPRETER SESSION */
{
	INT16 __nErr    = O_K;
	INT32  __nErrCnt = 0;
	data* idSrc;
	data* idDst;
	GET_THIS_VIRTUAL_RV(CVmap,NOT_EXEC);
	MIC_CHECK;
	__nErrCnt = CDlpObject_GetErrorCount();
	idDst = MIC_GET_I_EX(idDst,data,1,1);
	idSrc = MIC_GET_I_EX(idSrc,data,2,2);
	if (CDlpObject_GetErrorCount()>__nErrCnt) return NOT_EXEC;
	__nErr = CVmap_Map(_this, idSrc, idDst);
	return __nErr;
}

INT16 CVmap_OnSetup(CDlpObject* __this)
/* DO NOT CALL THIS FUNCTION FROM C++ SCOPE.     */
/* IT MAY INTERFERE WITH THE INTERPRETER SESSION */
{
	INT16 __nErr    = O_K;
	INT32  __nErrCnt = 0;
	data* idTmx;
	char* sAop;
	char* sWop;
	FLOAT64 nZero;
	GET_THIS_VIRTUAL_RV(CVmap,NOT_EXEC);
	MIC_CHECK;
	__nErrCnt = CDlpObject_GetErrorCount();
	nZero = MIC_GET_N(1,0);
	sWop = MIC_GET_S(2,0);
	sAop = MIC_GET_S(3,1);
	idTmx = MIC_GET_I_EX(idTmx,data,4,1);
	if (CDlpObject_GetErrorCount()>__nErrCnt) return NOT_EXEC;
	__nErr = CVmap_Setup(_this, idTmx, sAop, sWop, nZero);
	return __nErr;
}

INT16 CVmap_OnSetupI(CDlpObject* __this)
/* DO NOT CALL THIS FUNCTION FROM C++ SCOPE.     */
/* IT MAY INTERFERE WITH THE INTERPRETER SESSION */
{
	INT16 __nErr    = O_K;
	INT32  __nErrCnt = 0;
	data* idImap;
	data* idWtab;
	char* sAop;
	char* sWop;
	FLOAT64 nZero;
	FLOAT64 nOne;
	GET_THIS_VIRTUAL_RV(CVmap,NOT_EXEC);
	MIC_CHECK;
	__nErrCnt = CDlpObject_GetErrorCount();
	nOne = MIC_GET_N(1,0);
	nZero = MIC_GET_N(2,1);
	sWop = MIC_GET_S(3,0);
	sAop = MIC_GET_S(4,1);
	idWtab = MIC_GET_I_EX(idWtab,data,5,1);
	idImap = MIC_GET_I_EX(idImap,data,6,2);
	if (CDlpObject_GetErrorCount()>__nErrCnt) return NOT_EXEC;
	__nErr = CVmap_SetupI(_this, idImap, idWtab, sAop, sWop, nZero, nOne);
	return __nErr;
}

INT16 CVmap_OnStatus(CDlpObject* __this)
/* DO NOT CALL THIS FUNCTION FROM C++ SCOPE.     */
/* IT MAY INTERFERE WITH THE INTERPRETER SESSION */
{
	INT16 __nErr    = O_K;
	INT32  __nErrCnt = 0;
	GET_THIS_VIRTUAL_RV(CVmap,NOT_EXEC);
	MIC_CHECK;
	__nErrCnt = CDlpObject_GetErrorCount();
	if (CDlpObject_GetErrorCount()>__nErrCnt) return NOT_EXEC;
	__nErr = CVmap_Status(_this);
	return __nErr;
}

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

CVmap::CVmap(const char* lpInstanceName, BOOL bCallVirtual) : inherited(lpInstanceName,0)
{
	CVmap_Constructor(this,lpInstanceName,bCallVirtual);
}

CVmap::~CVmap()
{
	CVmap_Destructor(this);
}

INT16 CVmap::AutoRegisterWords()
{
	return CVmap_AutoRegisterWords(this);
}

INT16 CVmap::Init(BOOL bCallVirtual)
{
	return CVmap_Init(this,bCallVirtual);
}

INT16 CVmap::Reset(BOOL bResetMembers)
{
	return CVmap_Reset(this,bResetMembers);
}

INT16 CVmap::ClassProc()
{
	return CVmap_ClassProc(this);
}

INT16 CVmap::Serialize(CDN3Stream* lpDest)
{
	return CVmap_Serialize(this,lpDest);
}

INT16 CVmap::SerializeXml(CXmlStream* lpDest)
{
	return CVmap_SerializeXml(this,lpDest);
}

INT16 CVmap::Deserialize(CDN3Stream* lpSrc)
{
	return CVmap_Deserialize(this,lpSrc);
}

INT16 CVmap::DeserializeXml(CXmlStream* lpSrc)
{
	return CVmap_DeserializeXml(this,lpSrc);
}

INT16 CVmap::Copy(CDlpObject* __iSrc)
{
	return CVmap_Copy(this,__iSrc);
}

INT16 CVmap::InstallProc(void* lpItp)
{
	return CVmap_InstallProc(lpItp);
}

CVmap* CVmap::CreateInstance(const char* lpName)
{
	return CVmap_CreateInstance(lpName);
}

INT16 CVmap::GetClassInfo(SWord* lpClassWord)
{
	return CVmap_GetClassInfo(lpClassWord);
}

INT16 CVmap::GetInstanceInfo(SWord* lpClassWord)
{
	return CVmap_GetInstanceInfo(this,lpClassWord);
}

BOOL CVmap::IsKindOf(const char* lpClassName)
{
	return CVmap_IsKindOf(this,lpClassName);
}

INT16 CVmap::ResetAllOptions(BOOL bInit)
{
	return CVmap_ResetAllOptions(this,bInit);
}

#ifndef __NOITP
/*{{CGEN_PMIC */
INT16 CVmap::OnMap()
/* DO NOT CALL THIS FUNCTION FROM C++ SCOPE.     */
/* IT MAY INTERFERE WITH THE INTERPRETER SESSION */
{
	return CVmap_OnMap(this);
}

INT16 CVmap::OnSetup()
/* DO NOT CALL THIS FUNCTION FROM C++ SCOPE.     */
/* IT MAY INTERFERE WITH THE INTERPRETER SESSION */
{
	return CVmap_OnSetup(this);
}

INT16 CVmap::OnSetupI()
/* DO NOT CALL THIS FUNCTION FROM C++ SCOPE.     */
/* IT MAY INTERFERE WITH THE INTERPRETER SESSION */
{
	return CVmap_OnSetupI(this);
}

INT16 CVmap::OnStatus()
/* DO NOT CALL THIS FUNCTION FROM C++ SCOPE.     */
/* IT MAY INTERFERE WITH THE INTERPRETER SESSION */
{
	return CVmap_OnStatus(this);
}

/*}}CGEN_PMIC */
#endif /* #ifndef __NOITP */

/*{{CGEN_SMIC */
/*}}CGEN_SMIC */

/*{{CGEN_OCCF */
/*}}CGEN_OCCF */

/*{{CGEN_FCCF */
/*}}CGEN_FCCF */

/*{{CGEN_CXXWRAP */
INT32 CVmap::GetInDim()
{
	return CVmap_GetInDim(this);
}

INT32 CVmap::GetOutDim()
{
	return CVmap_GetOutDim(this);
}

INT16 CVmap::Setup(CData* idTmx, const char* sAop, const char* sWop, FLOAT64 nZero)
{
	return CVmap_Setup(this, idTmx, sAop, sWop, nZero);
}

INT16 CVmap::SetupI(CData* idImap, CData* idWtab, const char* sAop, const char* sWop, FLOAT64 nZero, FLOAT64 nOne)
{
	return CVmap_SetupI(this, idImap, idWtab, sAop, sWop, nZero, nOne);
}

INT16 CVmap::Status()
{
	return CVmap_Status(this);
}

INT16 CVmap::Map(CData* idSrc, CData* idDst)
{
	return CVmap_Map(this, idSrc, idDst);
}

void CVmap::MapVector(BYTE* lpX, BYTE* lpY, INT32 nXdim, INT32 nYdim, INT16 nFtype)
{
	CVmap_MapVector(this, lpX, lpY, nXdim, nYdim, nFtype);
}

/*}}CGEN_CXXWRAP */

#endif /* #ifdef __cplusplus */

/* EOF */
