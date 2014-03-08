/* dLabPro SDK class CHelloworld (Helloworld)
 * - Simple example for a dLabPro class
 *
 * AUTHOR : m.eichner
 * PACKAGE: dLabPro/sdk
 *
 * This file was generated by dcg. DO NOT MODIFY! Modify helloworld.def instead.
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
#include "dlp_helloworld.h"

/* Class CHelloworld */

void CHelloworld_Constructor(CHelloworld* _this, const char* lpInstanceName, BOOL bCallVirtual)
{
	DEBUGMSG(-1,"CHelloworld_Constructor; (bCallVirtual=%d)",(int)bCallVirtual,0,0);

#ifndef __cplusplus

	/* Register instance */
	dlp_xalloc_register_object('J',_this,1,sizeof(CHelloworld),
		__FILE__,__LINE__,"Helloworld",lpInstanceName);

	/* Create base instance */
	_this->m_lpBaseInstance = calloc(1,sizeof(CDlpObject));
	CDlpObject_Constructor(_this->m_lpBaseInstance,lpInstanceName,FALSE);

	/* Override virtual member functions */
	_this->m_lpBaseInstance->AutoRegisterWords = CHelloworld_AutoRegisterWords;
	_this->m_lpBaseInstance->Reset             = CHelloworld_Reset;
	_this->m_lpBaseInstance->Init              = CHelloworld_Init;
	_this->m_lpBaseInstance->Serialize         = CHelloworld_Serialize;
	_this->m_lpBaseInstance->SerializeXml      = CHelloworld_SerializeXml;
	_this->m_lpBaseInstance->Deserialize       = CHelloworld_Deserialize;
	_this->m_lpBaseInstance->DeserializeXml    = CHelloworld_DeserializeXml;
	_this->m_lpBaseInstance->Copy              = CHelloworld_Copy;
	_this->m_lpBaseInstance->ClassProc         = CHelloworld_ClassProc;
	_this->m_lpBaseInstance->GetInstanceInfo   = CHelloworld_GetInstanceInfo;
	_this->m_lpBaseInstance->IsKindOf          = CHelloworld_IsKindOf;
	_this->m_lpBaseInstance->Destructor        = CHelloworld_Destructor;
	_this->m_lpBaseInstance->ResetAllOptions   = CHelloworld_ResetAllOptions;

	/* Override pointer to derived instance */
	_this->m_lpBaseInstance->m_lpDerivedInstance = _this;

	#endif /* #ifndef __cplusplus */

	dlp_strcpy(BASEINST(_this)->m_lpClassName,"Helloworld");
	dlp_strcpy(BASEINST(_this)->m_lpObsoleteName,"");
	dlp_strcpy(BASEINST(_this)->m_lpProjectName,"helloworld");
	dlp_strcpy(BASEINST(_this)->m_version.no,"1.00 SLC22");
	dlp_strcpy(BASEINST(_this)->m_version.date,"");
	BASEINST(_this)->m_nClStyle = CS_AUTOACTIVATE;

	if (bCallVirtual)
	{
		DLPASSERT(OK(INVOKE_VIRTUAL_0(AutoRegisterWords)));
		INVOKE_VIRTUAL_1(Init,TRUE);
	}
}

void CHelloworld_Destructor(CDlpObject* __this)
{
	GET_THIS_VIRTUAL(CHelloworld);
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

INT16 CHelloworld_AutoRegisterWords(CDlpObject* __this)
{
	GET_THIS_VIRTUAL_RV(CHelloworld,NOT_EXEC);
	DEBUGMSG(-1,"CHelloworld_AutoRegisterWords",0,0,0);

	/* Call base class implementation */
	IF_NOK(INVOKE_BASEINST_0(AutoRegisterWords)) return NOT_EXEC;

	/*{{CGEN_REGISTERWORDS */

	/* Register methods */
	REGISTER_METHOD("-dm","",LPMF(CHelloworld,OnDm),"Delta modulation example",0,"<data idSrc> <long nOversample> <data idDst> <Helloworld this>","")
	REGISTER_METHOD("-sayhello","",LPMF(CHelloworld,OnSayhello),"Says hello to the user.",0,"<Helloworld this>","")

	/* Register options */
	REGISTER_OPTION("/adm","",LPMV(m_bAdm),NULL,"Adaptive delta modulation (method -dm)",0)
	REGISTER_OPTION("/german","",LPMV(m_bGerman),LPMF(CHelloworld,OnGermanSet),"switch to german",0)

	/* Register fields */
	REGISTER_FIELD("field_bool","",LPMV(m_fieldBool),NULL,"Just a field of type bool.",0,1000,1,"bool",(BOOL)0)
	REGISTER_FIELD("field_char","",LPMV(m_fieldChar),NULL,"Just a field of type char.",0,2001,1,"char",(char)0)
	REGISTER_FIELD("field_data","",LPMV(m_fieldData),NULL,"Just a field of type data.",0,6002,1,"data",NULL)
	REGISTER_FIELD("field_double","",LPMV(m_fieldDouble),NULL,"Just a field of type double.",0,3008,1,"double",(FLOAT64)0.0)
	REGISTER_FIELD("field_float","",LPMV(m_fieldFloat),NULL,"Just a field of type float.",0,3004,1,"float",(FLOAT32)0.0)
	REGISTER_FIELD("field_long","",LPMV(m_fieldLong),NULL,"Just a field of type long.",0,2008,1,"long",(INT64)0)
	REGISTER_FIELD("field_short","",LPMV(m_fieldShort),NULL,"Just a field of type short.",0,2002,1,"short",(INT16)0)
	REGISTER_FIELD("field_uchar","",LPMV(m_fieldUchar),NULL,"Just a field of type unsigned char.",0,1001,1,"unsigned char",(UINT8)0)
	REGISTER_FIELD("field_ulong","",LPMV(m_fieldUlong),NULL,"Just a field of type unsigned long.",0,1008,1,"unsigned long",(UINT64)0)
	REGISTER_FIELD("field_ushort","",LPMV(m_fieldUshort),NULL,"Just a field of type unsigned short.",0,1002,1,"unsigned short",(UINT16)0)
	REGISTER_FIELD("greeting_english","",LPMV(m_greetingEnglish),LPMF(CHelloworld,OnGreetingEnglishChanged),"English greeting text.",0,5000,1,"string","Hello World!")
	REGISTER_FIELD("greeting_german","",LPMV(m_greetingGerman),LPMF(CHelloworld,OnGreetingGermanChanged),"German greeting text.",0,5000,1,"string","Hallo Welt!")

	/* Register errors */
	REGISTER_ERROR("~e1_0_0__1",EL_ERROR,PAR_ERROR,"Failed to set parameter %s.")
	/*}}CGEN_REGISTERWORDS */

	return O_K;
}

INT16 CHelloworld_Init(CDlpObject* __this, BOOL bCallVirtual)
{
	GET_THIS_VIRTUAL_RV(CHelloworld,NOT_EXEC);
	DEBUGMSG(-1,"CHelloworld_Init, (bCallVirtual=%d)",(int)bCallVirtual,0,0);
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

INT16 CHelloworld_Reset(CDlpObject* __this, BOOL bResetMembers)
{
	GET_THIS_VIRTUAL_RV(CHelloworld,NOT_EXEC);
	DEBUGMSG(-1,"CHelloworld_Reset; (bResetMembers=%d)",(int)bResetMembers,0,0);
	{
	/*{{CGEN_RESETCODE */
  RESET;
	/*}}CGEN_RESETCODE */
	}

	return O_K;
}

INT16 CHelloworld_ClassProc(CDlpObject* __this)
{
	GET_THIS_VIRTUAL_RV(CHelloworld,NOT_EXEC);
	{
	/*{{CGEN_CLASSCODE */
  return CLASSPROC;
	/*}}CGEN_CLASSCODE */
	}

	return O_K;
}

#define CODE_DN3 /* check this for xml specific save code */
#define SAVE  SAVE_DN3
INT16 CHelloworld_Serialize(CDlpObject* __this, CDN3Stream* lpDest)
{
	GET_THIS_VIRTUAL_RV(CHelloworld,NOT_EXEC);
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
INT16 CHelloworld_SerializeXml(CDlpObject* __this, CXmlStream* lpDest)
{
	GET_THIS_VIRTUAL_RV(CHelloworld,NOT_EXEC);
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
INT16 CHelloworld_Deserialize(CDlpObject* __this, CDN3Stream* lpSrc)
{
	GET_THIS_VIRTUAL_RV(CHelloworld,NOT_EXEC);
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
INT16 CHelloworld_DeserializeXml(CDlpObject* __this, CXmlStream* lpSrc)
{
	GET_THIS_VIRTUAL_RV(CHelloworld,NOT_EXEC);
	{
	/*{{CGEN_RESTORECODE */
  return RESTORE;
	/*}}CGEN_RESTORECODE */
	}

	return O_K;
}
#undef  RESTORE
#undef  CODE_XML

INT16 CHelloworld_Copy(CDlpObject* __this, CDlpObject* __iSrc)
{
	GET_THIS_VIRTUAL_RV(CHelloworld,NOT_EXEC);
	{
	/*{{CGEN_COPYCODE */
  return COPY;
	/*}}CGEN_COPYCODE */
	}

	return O_K;
}

/* Runtime class type information and class factory */
INT16 CHelloworld_InstallProc(void* lpItp)
{
	{
	/*{{CGEN_INSTALLCODE */
  return INSTALL;
	/*}}CGEN_INSTALLCODE */
	}

	return O_K;
}

CHelloworld* CHelloworld_CreateInstance(const char* lpName)
{
	CHelloworld* lpNewInstance;
	ICREATEEX(CHelloworld,lpNewInstance,lpName,NULL);
	return lpNewInstance;
}

INT16 CHelloworld_GetClassInfo(SWord* lpClassWord)
{
	if (!lpClassWord) return NOT_EXEC;
	dlp_memset(lpClassWord,0,sizeof(SWord));

	lpClassWord->nWordType          = WL_TYPE_FACTORY;
	lpClassWord->nFlags             = CS_AUTOACTIVATE;

#ifdef __cplusplus

	lpClassWord->ex.fct.lpfFactory  = (LP_FACTORY_PROC)CHelloworld::CreateInstance;
	lpClassWord->ex.fct.lpfInstall  = CHelloworld::InstallProc;

#else /* #ifdef __DLP_CSCOPE */

	lpClassWord->ex.fct.lpfFactory  = (LP_FACTORY_PROC)CHelloworld_CreateInstance;
	lpClassWord->ex.fct.lpfInstall  = CHelloworld_InstallProc;

#endif /* #ifdef __DLP_CSCOPE */

	lpClassWord->ex.fct.lpProject   = "helloworld";
	lpClassWord->ex.fct.lpBaseClass = "-";
	lpClassWord->lpComment          = "Simple example for a dLabPro class";
	lpClassWord->ex.fct.lpAutoname  = "";
	lpClassWord->ex.fct.lpCname     = "CHelloworld";
	lpClassWord->ex.fct.lpAuthor    = "m.eichner";

	dlp_strcpy(lpClassWord->lpName             ,"Helloworld");
	dlp_strcpy(lpClassWord->lpObsname          ,"");
	dlp_strcpy(lpClassWord->ex.fct.version.no  ,"1.00 SLC22");

	return O_K;
}

INT16 CHelloworld_GetInstanceInfo(CDlpObject* __this, SWord* lpClassWord)
{
	return CHelloworld_GetClassInfo(lpClassWord);
}

BOOL CHelloworld_IsKindOf(CDlpObject* __this, const char* lpClassName)
{
	GET_THIS_VIRTUAL_RV(CHelloworld,NOT_EXEC);

  if (dlp_strncmp(lpClassName,"Helloworld",L_NAMES) == 0) return TRUE;
	else return INVOKE_BASEINST_1(IsKindOf,lpClassName);
}

INT16 CHelloworld_ResetAllOptions(CDlpObject* __this, BOOL bInit)
{
	GET_THIS_VIRTUAL_RV(CHelloworld,NOT_EXEC);
	DEBUGMSG(-1,"CHelloworld_ResetAllOptions;",0,0,0);
	{
	/*{{CGEN_RESETALLOPTIONS*/
	_this->m_bAdm = FALSE;
	_this->m_bGerman = FALSE;
	/*}}CGEN_RESETALLOPTIONS*/
	}

	return INVOKE_BASEINST_1(ResetAllOptions,bInit);
}

/* Generated primary method invocation functions */

#ifndef __NOITP
/*{{CGEN_CPMIC */
INT16 CHelloworld_OnDm(CDlpObject* __this)
/* DO NOT CALL THIS FUNCTION FROM C++ SCOPE.     */
/* IT MAY INTERFERE WITH THE INTERPRETER SESSION */
{
	INT16 __nErr    = O_K;
	INT32  __nErrCnt = 0;
	data* idSrc;
	INT64 nOversample;
	data* idDst;
	GET_THIS_VIRTUAL_RV(CHelloworld,NOT_EXEC);
	MIC_CHECK;
	__nErrCnt = CDlpObject_GetErrorCount();
	idDst = MIC_GET_I_EX(idDst,data,1,1);
	nOversample = (INT64)MIC_GET_N(2,0);
	idSrc = MIC_GET_I_EX(idSrc,data,3,2);
	if (CDlpObject_GetErrorCount()>__nErrCnt) return NOT_EXEC;
	__nErr = CHelloworld_Dm(_this, idSrc, nOversample, idDst);
	return __nErr;
}

INT16 CHelloworld_OnSayhello(CDlpObject* __this)
/* DO NOT CALL THIS FUNCTION FROM C++ SCOPE.     */
/* IT MAY INTERFERE WITH THE INTERPRETER SESSION */
{
	INT16 __nErr    = O_K;
	INT32  __nErrCnt = 0;
	GET_THIS_VIRTUAL_RV(CHelloworld,NOT_EXEC);
	MIC_CHECK;
	__nErrCnt = CDlpObject_GetErrorCount();
	if (CDlpObject_GetErrorCount()>__nErrCnt) return NOT_EXEC;
	__nErr = CHelloworld_Sayhello(_this);
	return __nErr;
}

/*}}CGEN_CPMIC */
#endif /* #ifndef __NOITP */


/* Generated secondary method invocation functions */

/*{{CGEN_CSMIC */
INT16 CHelloworld_Sayhello(CHelloworld* _this)
{
    if (_this->m_bGerman)
      printf("\n\n%s\n\n",_this->m_greetingGerman);
    else
      printf("\n\n%s\n\n",_this->m_greetingEnglish);
	return O_K;
}

/*}}CGEN_CSMIC */


/* Generated option change callback functions */

/*{{CGEN_COCCF */
INT16 CHelloworld_OnGermanSet(CDlpObject* __this)
{
	GET_THIS_VIRTUAL_RV(CHelloworld,NOT_EXEC);
	{
  	printf(   "\nMake it German, baby!");
	}

	return O_K;
}

/*}}CGEN_COCCF */


/* Generated field change callback functions */

/*{{CGEN_CFCCF */
INT16 CHelloworld_OnGreetingEnglishChanged(CDlpObject* __this)
{
	GET_THIS_VIRTUAL_RV(CHelloworld,NOT_EXEC);
	{
    if (!dlp_strlen(_this->m_greetingEnglish))
      return IERROR(_this,PAR_ERROR,"greeting_english",0,0);
	}

	return O_K;
}

INT16 CHelloworld_OnGreetingGermanChanged(CDlpObject* __this)
{
	GET_THIS_VIRTUAL_RV(CHelloworld,NOT_EXEC);
	{
    if (!dlp_strlen(_this->m_greetingGerman))
      return IERROR(_this,PAR_ERROR,"greeting_german",0,0);
	}

	return O_K;
}

/*}}CGEN_CFCCF */


/* C++ wrapper functions */
#ifdef __cplusplus

#define _this this

CHelloworld::CHelloworld(const char* lpInstanceName, BOOL bCallVirtual) : inherited(lpInstanceName,0)
{
	CHelloworld_Constructor(this,lpInstanceName,bCallVirtual);
}

CHelloworld::~CHelloworld()
{
	CHelloworld_Destructor(this);
}

INT16 CHelloworld::AutoRegisterWords()
{
	return CHelloworld_AutoRegisterWords(this);
}

INT16 CHelloworld::Init(BOOL bCallVirtual)
{
	return CHelloworld_Init(this,bCallVirtual);
}

INT16 CHelloworld::Reset(BOOL bResetMembers)
{
	return CHelloworld_Reset(this,bResetMembers);
}

INT16 CHelloworld::ClassProc()
{
	return CHelloworld_ClassProc(this);
}

INT16 CHelloworld::Serialize(CDN3Stream* lpDest)
{
	return CHelloworld_Serialize(this,lpDest);
}

INT16 CHelloworld::SerializeXml(CXmlStream* lpDest)
{
	return CHelloworld_SerializeXml(this,lpDest);
}

INT16 CHelloworld::Deserialize(CDN3Stream* lpSrc)
{
	return CHelloworld_Deserialize(this,lpSrc);
}

INT16 CHelloworld::DeserializeXml(CXmlStream* lpSrc)
{
	return CHelloworld_DeserializeXml(this,lpSrc);
}

INT16 CHelloworld::Copy(CDlpObject* __iSrc)
{
	return CHelloworld_Copy(this,__iSrc);
}

INT16 CHelloworld::InstallProc(void* lpItp)
{
	return CHelloworld_InstallProc(lpItp);
}

CHelloworld* CHelloworld::CreateInstance(const char* lpName)
{
	return CHelloworld_CreateInstance(lpName);
}

INT16 CHelloworld::GetClassInfo(SWord* lpClassWord)
{
	return CHelloworld_GetClassInfo(lpClassWord);
}

INT16 CHelloworld::GetInstanceInfo(SWord* lpClassWord)
{
	return CHelloworld_GetInstanceInfo(this,lpClassWord);
}

BOOL CHelloworld::IsKindOf(const char* lpClassName)
{
	return CHelloworld_IsKindOf(this,lpClassName);
}

INT16 CHelloworld::ResetAllOptions(BOOL bInit)
{
	return CHelloworld_ResetAllOptions(this,bInit);
}

#ifndef __NOITP
/*{{CGEN_PMIC */
INT16 CHelloworld::OnDm()
/* DO NOT CALL THIS FUNCTION FROM C++ SCOPE.     */
/* IT MAY INTERFERE WITH THE INTERPRETER SESSION */
{
	return CHelloworld_OnDm(this);
}

INT16 CHelloworld::OnSayhello()
/* DO NOT CALL THIS FUNCTION FROM C++ SCOPE.     */
/* IT MAY INTERFERE WITH THE INTERPRETER SESSION */
{
	return CHelloworld_OnSayhello(this);
}

/*}}CGEN_PMIC */
#endif /* #ifndef __NOITP */

/*{{CGEN_SMIC */
INT16 CHelloworld::Sayhello()
{
	return CHelloworld_Sayhello(this);
}

/*}}CGEN_SMIC */

/*{{CGEN_OCCF */
INT16 CHelloworld::OnGermanSet()
{
	return CHelloworld_OnGermanSet(this);
}

/*}}CGEN_OCCF */

/*{{CGEN_FCCF */
INT16 CHelloworld::OnGreetingEnglishChanged()
{
	return CHelloworld_OnGreetingEnglishChanged(this);
}

INT16 CHelloworld::OnGreetingGermanChanged()
{
	return CHelloworld_OnGreetingGermanChanged(this);
}

/*}}CGEN_FCCF */

/*{{CGEN_CXXWRAP */
short CHelloworld::Dm(CData* idSrc, long nOversample, CData* idDst)
{
	return CHelloworld_Dm(this, idSrc, nOversample, idDst);
}

/*}}CGEN_CXXWRAP */

#endif /* #ifdef __cplusplus */

/* EOF */
