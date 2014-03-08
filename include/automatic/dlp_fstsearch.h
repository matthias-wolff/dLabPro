/* dLabPro class CFstsearch (fstsearch)
 * - Header file
 *
 * AUTHOR : frank.duckhorn
 * PACKAGE: dLabPro/classes
 *
 * This file was generated by dcg. DO NOT MODIFY! Modify fstsearch.def instead.
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

/*{{CGEN_INCLUDE */
#include "dlp_config.h"
#include "dlp_object.h"
#include "dlp_data.h"
#include "dlp_fst.h"
/*}}CGEN_END */

/*{{CGEN_ERRORS */
#undef FSTS_STR            
#define FSTS_STR             -1001
/*}}CGEN_END */

/* C/C++ language abstraction layer */
#undef fstsearch_par
#define fstsearch_par CFstsearch

/* dLabPro/C++ language abstraction layer */
#undef fstsearch
#define fstsearch CFstsearch

/*{{CGEN_DEFINE */
/*}}CGEN_DEFINE */

#ifndef __FSTSEARCH_H
#define __FSTSEARCH_H

/*{{CGEN_HEADERCODE */
/*}}CGEN_HEADERCODE */

/* Class CFstsearch */

#ifdef __cplusplus

class CFstsearch : public CDlpObject 
{

typedef CDlpObject inherited;
typedef CFstsearch thisclass;

/*{{CGEN_FRIENDS */
/*}}CGEN_FRIENDS */
public:
	CFstsearch(const char* lpInstanceName, BOOL bCallVirtual = 1);
	virtual ~CFstsearch();

/* Virtual and static function overrides */
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
	static  CFstsearch* CreateInstance(const char* lpName);
	static  INT16 GetClassInfo(SWord* lpClassWord);
	virtual INT16 GetInstanceInfo(SWord* lpClassWord);
	virtual BOOL  IsKindOf(const char* lpClassName);
	virtual INT16 ResetAllOptions(BOOL bInit = 0);

/* Primary method invocation functions             */
/* DO NOT CALL THESE FUNCTIONS FROM C SCOPE.       */
/* THEY MAY INTERFERE WITH THE INTERPRETER SESSION */
#ifndef __NOITP
public:
/*{{CGEN_PMIC */
	INT16 OnBacktrack();
	INT16 OnIsearch();
	INT16 OnLoad();
	INT16 OnRestart();
	INT16 OnSearch();
/*}}CGEN_PMIC */
#endif /* #ifndef __NOITP */

/* Secondary method invocation functions */
public:
/*{{CGEN_SMIC */
/*}}CGEN_SMIC */

/* Option changed callback functions */
public:
/*{{CGEN_OCCF */
/*}}CGEN_OCCF */

/* Field changed callback functions */
public:
/*{{CGEN_FCCF */
	INT16 OnAlgoChanged();
	INT16 OnAsAheutypeChanged();
	INT16 OnAsPrnfChanged();
	INT16 OnAsPrnwChanged();
	INT16 OnAsQsizeChanged();
	INT16 OnAsSheutypeChanged();
	INT16 OnBtChanged();
	INT16 OnLatprnChanged();
	INT16 OnNumpathsChanged();
	INT16 OnSdpEpsremoveChanged();
	INT16 OnSdpFwdChanged();
	INT16 OnSdpPrnChanged();
	INT16 OnStkprnChanged();
	INT16 OnTpPrnhChanged();
	INT16 OnTpPrnwChanged();
	INT16 OnTpThreadsChanged();
/*}}CGEN_FCCF */

/* Scanned member functions */
/*{{CGEN_EXPORT */

/* Taken from 'fsts_glob.c' */
	public: INT16 Load(CFst* itSrc, long nUnit);
	public: INT16 Isearch(CData* idWeights);
	public: INT16 Backtrack(CFst* itDst);
	public: INT16 Unload();
	public: INT16 Restart();
	public: INT16 Search(CFst* itSrc, long nUnit, CData* idWeights, CFst* itDst);

/* Taken from 'fsts_cfg.c' */

/* Taken from 'fsts_fst.c' */

/* Taken from 'fsts_mem.c' */

/* Taken from 'fsts_w.c' */

/* Taken from 'fsts_bt.c' */

/* Taken from 'fsts_lat.c' */

/* Taken from 'fsts_h.c' */

/* Taken from 'fsts_as.c' */

/* Taken from 'fsts_as_ls.c' */

/* Taken from 'fsts_as_q.c' */

/* Taken from 'fsts_as_s.c' */

/* Taken from 'fsts_sdp.c' */

/* Taken from 'fsts_sdp_imp.c' */

/* Taken from 'fsts_tp.c' */

/* Taken from 'fsts_tp_s.c' */

/* Taken from 'fsts_tp_hist.c' */

/* Taken from 'fsts_tp_ls.c' */
/*}}CGEN_EXPORT */

/* Member variables */
public:
/*{{CGEN_ICXX_FIELDS */
/*}}CGEN_ICXX_FIELDS */

#else  /* #ifdef __cplusplus */

typedef struct CFstsearch
{
  /* Pointer to C base instance */
  struct CDlpObject* m_lpBaseInstance;

/*{{CGEN_IC_FIELDS */
/*}}CGEN_IC_FIELDS */

#endif /* #ifdef __cplusplus */

/*{{CGEN_FIELDS */
	char*            m_lpsAlgo;
	char*            m_lpsAsAheutype;
	INT64            m_nAsPrnf;
	FLOAT64          m_nAsPrnw;
	INT64            m_nAsQsize;
	char*            m_lpsAsSheutype;
	char*            m_lpsBt;
	void*            m_lpGlob;
	FLOAT64          m_nLatprn;
	BOOL             m_bLoaded;
	FLOAT64          m_nMem;
	INT64            m_nNumpaths;
	BOOL             m_bSdpEpsremove;
	BOOL             m_bSdpFwd;
	FLOAT64          m_nSdpPrn;
	BOOL             m_bStkprn;
	FLOAT64          m_nTime;
	INT64            m_nTpPrnh;
	FLOAT64          m_nTpPrnw;
	INT64            m_nTpThreads;
/*}}CGEN_FIELDS */

/*{{CGEN_OPTIONS */
	BOOL m_bFast;
	BOOL m_bFinal;
/*}}CGEN_OPTIONS */
}

#ifndef __cplusplus
CFstsearch
#endif
;

/* Class CFstsearch (C functions)*/

/* Virtual function overrides */
void  CFstsearch_Constructor(CFstsearch*, const char* lpInstanceName, BOOL bCallVirtual);
void  CFstsearch_Destructor(CDlpObject*);
INT16 CFstsearch_AutoRegisterWords(CDlpObject*);
INT16 CFstsearch_Reset(CDlpObject*, BOOL bResetMembers);
INT16 CFstsearch_Init(CDlpObject*, BOOL bCallVirtual);
INT16 CFstsearch_Serialize(CDlpObject*, CDN3Stream* lpDest);
INT16 CFstsearch_SerializeXml(CDlpObject*, CXmlStream* lpDest);
INT16 CFstsearch_Deserialize(CDlpObject*, CDN3Stream* lpSrc);
INT16 CFstsearch_DeserializeXml(CDlpObject*, CXmlStream* lpSrc);
INT16 CFstsearch_Copy(CDlpObject*, CDlpObject* __iSrc);
INT16 CFstsearch_ClassProc(CDlpObject*);
INT16 CFstsearch_InstallProc(void* lpItp);
CFstsearch* CFstsearch_CreateInstance(const char* lpName);
INT16 CFstsearch_GetClassInfo(SWord* lpClassWord);
INT16 CFstsearch_GetInstanceInfo(CDlpObject*, SWord* lpClassWord);
BOOL  CFstsearch_IsKindOf(CDlpObject*, const char* lpClassName);
INT16 CFstsearch_ResetAllOptions(CDlpObject*, BOOL bInit);

/* Primary method invocation functions             */
/* DO NOT CALL THESE FUNCTIONS FROM C SCOPE.       */
/* THEY MAY INTERFERE WITH THE INTERPRETER SESSION */
#ifndef __NOITP
/*{{CGEN_CPMIC */
INT16 CFstsearch_OnBacktrack(CDlpObject*);
INT16 CFstsearch_OnIsearch(CDlpObject*);
INT16 CFstsearch_OnLoad(CDlpObject*);
INT16 CFstsearch_OnRestart(CDlpObject*);
INT16 CFstsearch_OnSearch(CDlpObject*);
/*}}CGEN_CPMIC */
#endif /* #ifndef __NOITP */

/* Secondary method invocation functions */
/*{{CGEN_CSMIC */
/*}}CGEN_CSMIC */

/* Option changed callback functions */
/*{{CGEN_COCCF */
/*}}CGEN_COCCF */

/* Field changed callback functions */
/*{{CGEN_CFCCF */
INT16 CFstsearch_OnAlgoChanged(CDlpObject*);
INT16 CFstsearch_OnAsAheutypeChanged(CDlpObject*);
INT16 CFstsearch_OnAsPrnfChanged(CDlpObject*);
INT16 CFstsearch_OnAsPrnwChanged(CDlpObject*);
INT16 CFstsearch_OnAsQsizeChanged(CDlpObject*);
INT16 CFstsearch_OnAsSheutypeChanged(CDlpObject*);
INT16 CFstsearch_OnBtChanged(CDlpObject*);
INT16 CFstsearch_OnLatprnChanged(CDlpObject*);
INT16 CFstsearch_OnNumpathsChanged(CDlpObject*);
INT16 CFstsearch_OnSdpEpsremoveChanged(CDlpObject*);
INT16 CFstsearch_OnSdpFwdChanged(CDlpObject*);
INT16 CFstsearch_OnSdpPrnChanged(CDlpObject*);
INT16 CFstsearch_OnStkprnChanged(CDlpObject*);
INT16 CFstsearch_OnTpPrnhChanged(CDlpObject*);
INT16 CFstsearch_OnTpPrnwChanged(CDlpObject*);
INT16 CFstsearch_OnTpThreadsChanged(CDlpObject*);
/*}}CGEN_CFCCF */

/* Scanned C (member) functions */
/*{{CGEN_CEXPORT */

/* Taken from 'fsts_glob.c' */
INT16 CFstsearch_Load(CFstsearch*, CFst* itSrc, long nUnit);
INT16 CFstsearch_Isearch(CFstsearch*, CData* idWeights);
INT16 CFstsearch_Backtrack(CFstsearch*, CFst* itDst);
INT16 CFstsearch_Unload(CFstsearch*);
INT16 CFstsearch_Restart(CFstsearch*);
INT16 CFstsearch_Search(CFstsearch*, CFst* itSrc, long nUnit, CData* idWeights, CFst* itDst);

/* Taken from 'fsts_cfg.c' */

/* Taken from 'fsts_fst.c' */

/* Taken from 'fsts_mem.c' */

/* Taken from 'fsts_w.c' */

/* Taken from 'fsts_bt.c' */

/* Taken from 'fsts_lat.c' */

/* Taken from 'fsts_h.c' */

/* Taken from 'fsts_as.c' */

/* Taken from 'fsts_as_ls.c' */

/* Taken from 'fsts_as_q.c' */

/* Taken from 'fsts_as_s.c' */

/* Taken from 'fsts_sdp.c' */

/* Taken from 'fsts_sdp_imp.c' */

/* Taken from 'fsts_tp.c' */

/* Taken from 'fsts_tp_s.c' */

/* Taken from 'fsts_tp_hist.c' */

/* Taken from 'fsts_tp_ls.c' */
/*}}CGEN_CEXPORT */

#endif /*#ifndef __FSTSEARCH_H */


/* EOF */
