// dLabPro class CLCQproc (LCQproc)
// - Header file
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
#include "dlp_config.h"
#include "dlp_object.h"
#include "dlp_cpproc.h"
#include "dlp_data.h"
//}}CGEN_END

//{{CGEN_ERRORS
#undef ERR_N_ROOTS_UNSTABLE
#undef ERR_POLY_UNSTABLE   
#define ERR_N_ROOTS_UNSTABLE -1019
#define ERR_POLY_UNSTABLE    -1020
//}}CGEN_END

// C/C++ language abstraction layer
#undef lcqproc_par
#define lcqproc_par CLCQproc

// dLabPro/C++ language abstraction layer
#undef LCQproc
#define LCQproc CLCQproc

//{{CGEN_DEFINE
//}}CGEN_DEFINE

#ifndef __LCQPROC_H
#define __LCQPROC_H

//{{CGEN_HEADERCODE
//}}CGEN_HEADERCODE

// Class CLCQproc

class CLCQproc : public CCPproc 
{

typedef CCPproc inherited;
typedef CLCQproc thisclass;

//{{CGEN_FRIENDS
//}}CGEN_FRIENDS
public:
	CLCQproc(const char* lpInstanceName, BOOL bCallVirtual = 1);
	virtual ~CLCQproc();

// Virtual and static function overrides
public:
	virtual INT16 AutoRegisterWords();
	virtual INT16 Init(BOOL bCallVirtual = 0);
	virtual INT16 Reset(BOOL bResetMembers = 1);
	virtual INT16 Serialize(CDN3Stream* lpDest);
	virtual INT16 SerializeXml(CXmlStream* lpDest);
	virtual INT16 Deserialize(CDN3Stream* lpSrc);
	virtual INT16 DeserializeXml(CXmlStream* lpSrc);
	virtual INT16 Copy(CDlpObject* iSrc);
	virtual INT16 ClassProc();
	static  INT16 InstallProc(void* lpItp);
	static  CLCQproc* CreateInstance(const char* lpName);
	static  INT16 GetClassInfo(SWord* lpClassWord);
	virtual INT16 GetInstanceInfo(SWord* lpClassWord);
	virtual BOOL  IsKindOf(const char* lpClassName);
	virtual INT16 ResetAllOptions(BOOL bInit = 0);

// Primary method invocation functions            
// DO NOT CALL THESE FUNCTIONS FROM C SCOPE.      
// THEY MAY INTERFERE WITH THE INTERPRETER SESSION
#ifndef __NOITP
public:
//{{CGEN_PMIC
	INT16 OnAnalyze();
//}}CGEN_PMIC
#endif // #ifndef __NOITP

// Secondary method invocation functions
public:
//{{CGEN_SMIC
//}}CGEN_SMIC

// Option changed callback functions
public:
//{{CGEN_OCCF
//}}CGEN_OCCF

// Field changed callback functions
public:
//{{CGEN_FCCF
//}}CGEN_FCCF

// Scanned member functions
//{{CGEN_EXPORT

// Taken from 'lcq_work.cpp'
	public: INT16 AnalyzeFrame();
	private: INT16 Normalize(FLOAT64* lcq, INT16 n_lcq);
	public: void PrepareOutput(CData* dResult);

// Taken from 'lcq_synt.cpp'
	protected: virtual INT16 SynthesizeFrameImpl(FLOAT64* lcq, INT16 n_lcq, FLOAT64* exc, INT32 n_exc, FLOAT64 nPfaLambda, FLOAT64 nSynLambda, FLOAT64* syn);
	private: INT16 DeNormalize(FLOAT64* lcq, INT16 n_lcq);
	private: INT16 Lcq2Mcep(FLOAT64* lcq, INT16 n_lcq, FLOAT64* mcep);
	protected: virtual BOOL IsFeaVoiceless(FLOAT64* lcq, INT16 n_lcq);

// Taken from 'lcq_stat.cpp'
	public: void Status();
//}}CGEN_EXPORT

// Member variables
public:

//{{CGEN_FIELDS
//}}CGEN_FIELDS

//{{CGEN_OPTIONS
	BOOL m_bSynCepLcq;
	BOOL m_bSynCepLcqFilt;
	BOOL m_bSynCepMlcq;
	BOOL m_bSynCepMlcqFilt;
	BOOL m_bSynLcq;
	BOOL m_bSynMcepMlcq;
	BOOL m_bSynMcepMlcqFilt;
//}}CGEN_OPTIONS
}

;

// Scanned C (member) functions
//{{CGEN_CEXPORT

// Taken from 'lcq_work.cpp'

// Taken from 'lcq_synt.cpp'

// Taken from 'lcq_stat.cpp'
//}}CGEN_CEXPORT

#endif //#ifndef __LCQPROC_H


// EOF