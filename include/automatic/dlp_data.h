/* dLabPro class CData (data)
 * - Header file
 *
 * AUTHOR : g. strecha, dresden
 * PACKAGE: dLabPro/classes
 *
 * This file was generated by dcg. DO NOT MODIFY! Modify data.def instead.
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
#include "dlp_table.h"
/*}}CGEN_END */

/*{{CGEN_ERRORS */
#undef DATA_INTERNAL       
#undef DATA_EMPTY          
#undef DATA_BADCOMP        
#undef DATA_BADCOMPTYPE    
#undef DATA_BADOPC         
#undef DATA_INITIALIZERS   
#undef DATA_BADINITIALIZER 
#undef DATA_HERESCAN       
#undef DATA_MTHINCOMPL     
#undef DATA_OPCODE         
#undef DATA_BADSORTMODE    
#undef DATA_BADSORTTYPE    
#undef DATA_TRUNCATE       
#undef DATA_DIMMISMATCH    
#undef DATA_NOMARK         
#undef DATA_BADMARK        
#undef DATA_HOMOGEN        
#undef DATA_DESERIALIZE    
#undef DATA_CNVT           
#undef DATA_NOSUPPORT      
#undef DATA_MDIM           
#undef DATA_MDATA_WARN     
#undef DATA_NOTFOUND_ERR   
#undef DATA_SIZE           
#undef DATA_NOTFOUND       
#undef DATA_AMBIGUOUS      
#define DATA_INTERNAL        -1001
#define DATA_EMPTY           -1002
#define DATA_BADCOMP         -1003
#define DATA_BADCOMPTYPE     -1004
#define DATA_BADOPC          -1005
#define DATA_INITIALIZERS    -1006
#define DATA_BADINITIALIZER  -1007
#define DATA_HERESCAN        -1008
#define DATA_MTHINCOMPL      -1009
#define DATA_OPCODE          -1010
#define DATA_BADSORTMODE     -1011
#define DATA_BADSORTTYPE     -1012
#define DATA_TRUNCATE        -1013
#define DATA_DIMMISMATCH     -1014
#define DATA_NOMARK          -1015
#define DATA_BADMARK         -1016
#define DATA_HOMOGEN         -1017
#define DATA_DESERIALIZE     -1018
#define DATA_CNVT            -1019
#define DATA_NOSUPPORT       -1020
#define DATA_MDIM            -1021
#define DATA_MDATA_WARN      -1022
#define DATA_NOTFOUND_ERR    -1023
#define DATA_SIZE            -1024
#define DATA_NOTFOUND        -1025
#define DATA_AMBIGUOUS       -1026
/*}}CGEN_END */

/* C/C++ language abstraction layer */
#undef data_par
#define data_par CData

/* dLabPro/C++ language abstraction layer */
#undef data
#define data CData

/*{{CGEN_DEFINE */
#define DESCR0    9900
#define DESCR1    9901
#define DESCR2    9902
#define DESCR3    9903
#define DESCR4    9904
#define RINC      9905
#define RWID      9906
#define ROFS      9907
#define CHECK_DATA(A) {DLPASSERT(A!=NULL); DLPASSERT(A->m_lpTable);}
#define CDATA_XADDR(THIS,R,C)                    (THIS->m_lpTable->m_theDataPointer         +    (size_t)THIS->m_lpTable->m_reclen * (size_t)R             +    (size_t)THIS->m_lpTable->m_compDescrList[C].offset)
#define CDATA_SORT_UP    1
#define CDATA_SORT_DOWN  2
#define CDATA_MARK_RECS     0
#define CDATA_MARK_COMPS    1
#define CDATA_MARK_BLOCKS   2
#define CDATA_MARK_CELLS    3
#define PRINTBREAK       -1
#define PRINTNEXT        -2
#define AXES_DESCRLEN    10
/*}}CGEN_DEFINE */

#ifndef __DATA_H
#define __DATA_H

/*{{CGEN_HEADERCODE */
/*}}CGEN_HEADERCODE */

/* Class CData */

#ifdef __cplusplus

class CData : public CDlpObject 
{

typedef CDlpObject inherited;
typedef CData thisclass;

/*{{CGEN_FRIENDS */
/*}}CGEN_FRIENDS */
public:
	CData(const char* lpInstanceName, BOOL bCallVirtual = 1);
	virtual ~CData();

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
	static  CData* CreateInstance(const char* lpName);
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
	INT16 OnInitializeRecord();
	INT16 OnAddComp();
	INT16 OnAddNcomps();
	INT16 OnAggregate();
	INT16 OnAllocate();
	INT16 OnArray();
	INT16 OnCat();
	INT16 OnChecksum();
	INT16 OnClear();
	INT16 OnCompress();
	INT16 OnCopyCnames();
	INT16 OnCopyDescr();
	INT16 OnCopyLabels();
	INT16 OnDelete();
	INT16 OnDequantize();
	INT16 OnDfetch();
	INT16 OnDmark();
	INT16 OnDstore();
	INT16 OnExpand();
	INT16 OnFetch();
	INT16 OnFill();
	INT16 OnFindComp();
	INT16 OnGenIndex();
	INT16 OnGetCname();
	INT16 OnGetCnames();
	INT16 OnGetCompType();
	INT16 OnInitialize();
	INT16 OnInsertComp();
	INT16 OnInsertNcomps();
	INT16 OnIsEmpty();
	INT16 OnJoin();
	INT16 OnLookup();
	INT16 OnLookup2();
	INT16 OnMark();
	INT16 OnPfetch();
	INT16 OnPrint();
	INT16 OnPstore();
	INT16 OnQuantize();
	INT16 OnReallocate();
	INT16 OnRepmat();
	INT16 OnResample();
	INT16 OnReshape();
	INT16 OnRindex();
	INT16 OnRotate();
	INT16 OnScalop();
	INT16 OnScalopD();
	INT16 OnScopy();
	INT16 OnSelect();
	INT16 OnSetCname();
	INT16 OnSetCnames();
	INT16 OnSfetch();
	INT16 OnShift();
	INT16 OnSortdown();
	INT16 OnSortup();
	INT16 OnSstore();
	INT16 OnStatus();
	INT16 OnStore();
	INT16 OnStrop();
	INT16 OnTconvert();
	INT16 OnUnmark();
	INT16 OnXfetch();
	INT16 OnXstore();
	INT16 OnArrOp();
/*}}CGEN_PMIC */
#endif /* #ifndef __NOITP */

/* Secondary method invocation functions */
public:
/*{{CGEN_SMIC */
	INT16 Checksum(char* sAlgo, INT32 nIc);
	INT16 Reallocate(INT32 nRecs);
	INT16 Resample(data* iSrc, FLOAT64 nRate);
	INT16 Rindex(const char* sCname, INT32 nIc);
	INT16 Scalop(data* idSrc, COMPLEX64 nConst, const char* sOpname);
	INT16 ScalopD(data* idSrc, data* idConst, const char* sOpname);
	INT16 Sortdown(data* x, INT32 j);
	INT16 Sortup(data* x, INT32 j);
/*}}CGEN_SMIC */

/* Option changed callback functions */
public:
/*{{CGEN_OCCF */
/*}}CGEN_OCCF */

/* Field changed callback functions */
public:
/*{{CGEN_FCCF */
/*}}CGEN_FCCF */

/* Scanned member functions */
/*{{CGEN_EXPORT */

/* Taken from 'data_aci.c' */
	public: BYTE* XAddr(INT32 nRec, INT32 nComp);
	public: INT16 Alloc(INT32 nRecs);
	public: INT16 AllocUninitialized(INT32 nRecs);
	public: INT16 AllocateUninitialized(INT32 nRecs);
	public: INT16 Realloc(INT32 nRecs);
	public: INT32 AddRecs(INT32 nRecs, INT32 nRealloc);
	public: INT32 InsertRecs(INT32 nInsertAt, INT32 nRecs, INT32 nRealloc);
	public: INT32 GetNRecs();
	public: INT32 GetNComps();
	public: FLOAT64 GetFsr();
	public: INT32 GetNNumericComps();
	public: INT32 GetNComplexComps();
	public: INT32 GetNBlocks();
	public: INT32 GetNRecsPerBlock();
	public: INT32 SetNBlocks(INT32 nBlocks);
	public: INT32 IncNRecs(INT32 nRecs);
	public: INT32 SetNRecs(INT32 nRecs);
	public: INT32 GetMaxRecs();
	public: INT32 GetCompSize(INT32 nComp);
	public: INT32 GetCompOffset(INT32 nComp);
	public: INT32 GetRecLen();
	public: INT32 FindRec(INT32 nComp, char* sWhat);
	public: INT32 CcompFetch(COMPLEX64* dBuffer, INT32 nComp, INT32 nMaxRecs);
	public: INT32 DcompFetch(FLOAT64* dBuffer, INT32 nComp, INT32 nMaxRecs);
	public: INT32 CcompStore(COMPLEX64* dBuffer, INT32 nComp, INT32 nMaxRecs);
	public: INT32 DcompStore(FLOAT64* dBuffer, INT32 nComp, INT32 nMaxRecs);
	public: INT32 CrecFetch(COMPLEX64* lpBuffer, INT32 nRec, INT32 nMaxComps, INT32 nCompIgnore);
	public: INT32 DrecFetch(FLOAT64* lpBuffer, INT32 nRec, INT32 nMaxComps, INT32 nCompIgnore);
	public: INT32 CrecFetchInterpol(COMPLEX64* dBuffer, FLOAT64 nRec, INT32 nMaxComps, INT32 nCompIgnore, INT16 nMode);
	public: INT32 CrecStore(COMPLEX64* dBuffer, INT32 nRec, INT32 nMaxComps, INT32 nCompIgnore);
	public: INT32 DrecStore(FLOAT64* dBuffer, INT32 nRec, INT32 nMaxComps, INT32 nCompIgnore);
	public: INT32 CblockFetch(COMPLEX64* dBuffer, INT32 nBlock, INT32 nMaxComps, INT32 nMaxBrecs, INT32 nCompIgnore);
	public: INT32 DblockFetch(FLOAT64* dBuffer, INT32 nBlock, INT32 nMaxComps, INT32 nMaxBrecs, INT32 nCompIgnore);
	public: INT32 CblockStore(COMPLEX64* dBuffer, INT32 nBlock, INT32 nMaxComps, INT32 nMaxBrecs, INT32 nCompIgnore);
	public: INT32 DblockStore(FLOAT64* dBuffer, INT32 nBlock, INT32 nMaxComps, INT32 nMaxBrecs, INT32 nCompIgnore);
	public: INT32 DijFetch(FLOAT64* dBuffer, INT32 nRec, INT32 nComp, INT32 nMaxBlocks);
	public: INT32 CijFetch(COMPLEX64* dBuffer, INT32 nRec, INT32 nComp, INT32 nMaxBlocks);
	public: COMPLEX64 CfetchInterpol(FLOAT64 nRec, INT32 nComp, INT16 nMode);
	public: INT16 SelectRecs(data* iSrc, INT32 from, INT32 count);
	public: INT16 SelectBlocks(data* iSrc, INT32 from, INT32 count);
	public: INT16 SelectComps(data* iSrc, INT32 from, INT32 count);
	public: INT16 DeleteRecs(INT32 from, INT32 count);
	public: INT16 DeleteBlocks(INT32 from, INT32 count);
	public: INT16 DeleteComps(INT32 nFirst, INT32 nCount);
	public: INT16 CopyMarked(CData* idSrc, BOOL bPositive);
	public: INT16 CheckCompType(INT16 nType);
	public: FLOAT64 GetDescr(INT16 nDescr);
	public: void SetDescr(INT16 nDescr, FLOAT64 nValue);
	public: INT32 GenIndexList(data* iSrc, data* iTab, data* iLTab, INT32 nIdx);
	public: INT16 GenLabIndex(CData* iLabel, CData* iIndex, CData* iLTab);
	public: INT16 CopyComps(CData* iSrc, INT32 is, INT32 it, INT32 n);
	public: INT16 ResampleInt(CData* iSrc, FLOAT64 nRate, INT16 nMode);

/* Taken from 'data_iam.c' */
	public: INT16 AddComp(const char* lpsName, INT16 nType);
	public: INT16 AddNcomps(INT16 nCType, INT32 nCount);
	public: INT16 InsertComp(const char* sCName, INT16 nCType, INT32 nInsertAt);
	public: INT16 InsertNcomps(INT16 nCType, INT32 nInsertAt, INT32 nCount);
	public: INT16 GetCompType(INT32 nComp);
	public: INT16 Scopy(data* lpSrc);
	public: INT16 CopyCnames(data* iSrc, INT32 jx, INT32 jy, INT32 n);
	public: INT16 SetCnames(data* x, INT32 jx);
	public: INT16 GetCnames(CData* idSrc);
	public: INT16 SetCname(INT32 nIComp, const char* sCName);
	public: const char* GetCname(INT32 nIComp);
	public: INT32 FindComp(const char* lpName);
	public: INT16 Allocate(INT32 n);
	public: INT16 Array(INT16 nCType, INT32 nComps, INT32 nRecs);
	public: BOOL IsEmpty();
	public: COMPLEX64 Cfetch(INT32 nIRec, INT32 nIComp);
	public: FLOAT64 Dfetch(INT32 nIRec, INT32 nIComp);
	public: void* Pfetch(INT32 nIRec, INT32 nIComp);
	public: const char* Sfetch(INT32 nIRec, INT32 nIComp);
	public: INT16 Cstore(COMPLEX64 dVal, INT32 nIRec, INT32 nIComp);
	public: INT16 Dstore(FLOAT64 dVal, INT32 nIRec, INT32 nIComp);
	public: INT16 Pstore(void* lpVal, INT32 nIRec, INT32 nIComp);
	public: INT16 Sstore(const char* sVal, INT32 nIRec, INT32 nIComp);
	public: INT16 Clear();
	public: INT16 Fill(COMPLEX64 dStart, COMPLEX64 dDelta);
	public: INT16 Lookup(data* dSel, INT32 nSelComp, data* dTab, INT32 nTabComp, INT32 nCount);
	public: INT16 Lookup2(data* dSel1, INT32 nSel1Comp, data* dSel2, INT32 nSel2Comp, data* dTab);
	public: INT16 Aggregate(CData* iSrc, CData* iMask, COMPLEX64 dParam, const char* lpOpname);
	public: INT16 GenIndex(data* iSrc, data* iTab, INT32 nSrcIdx, INT32 nTabIdx);
	public: INT16 Tconvert(data* idSrc, INT16 nCType);
	public: INT16 CopyDescr(CData* iSrc);
	public: INT16 CopyMark(CData* iSrc);
	public: INT16 CopyLabels(CData* iSrc);
	public: INT16 Join(data* x);
	public: INT16 NJoin(data* x, INT32 jx, INT32 n);
	public: INT16 Cat(data* x);
	public: INT16 Select(CData* idSrc, INT32 nFirst, INT32 nCount);
	public: INT16 Delete(CData* idSrc, INT32 nFirst, INT32 nCount);
	public: INT16 Mark(INT32 jx, INT32 n);
	public: INT16 Dmark(CData* idMark);
	public: INT16 Unmark();
	public: INT16 Xfetch(INT32 nFirst, INT32 nCount);
	public: BOOL Xstore(CData* idSrc, INT32 nFirst, INT32 nCount, INT32 nPos);
	public: BOOL Reshape(CData* iSrc, INT32 nRec, INT32 nComp);
	public: BOOL Repmat(CData* iSrc, INT32 nRec, INT32 nComp);
	public: INT16 Shift(CData* iSrc, INT32 nCount);
	public: INT16 Rotate(CData* iSrc, INT32 nCount);
	public: INT16 IsHomogen();
	public: INT16 Print();
	public: INT16 Status();
	public: INT16 ArrOp();
	public: INT16 Quantize(CData* dIn);
	public: INT16 Dequantize(CData* dIn);

/* Taken from 'data_ini.c' */
	public: INT16 InitializeEx(INT32 nRec, INT32 nComp, INT32 nCount);
	protected: INT32 ReadInitializer(char* lpsInit, INT32 nLen, BOOL bForce);
	public: INT16 InitializeRecordEx(const char* lpsInit, INT32 nRec, INT32 nComp);
	public: INT16 Initialize();
	public: INT16 InitializeRecord(INT32 nRec);

/* Taken from 'data_int.c' */
	protected: INT16 VerifyMarkMap();
	protected: INT16 MarkElem(INT32 nElem);
	protected: BOOL IsMarked(INT32 nElem);
	protected: BOOL RecIsMarked(INT32 nRec);
	protected: BOOL CompIsMarked(INT32 nComp);
	protected: BOOL BlockIsMarked(INT32 nBlock);
	protected: BOOL CellIsMarked(INT32 nCell);
	private: INT32 MicGetIc(INT16 nArg, INT16 nPos);

/* Taken from 'data_prt.c' */
	private: INT16 PrintText();
	public: INT16 PrintRec(INT32 nRec, INT32 nIcFirst, INT32 nComps, INT16 nIndent);
	public: INT16 PrintList();
	private: INT16 PrintVectors_GetColWidth(INT32 nR0, INT32 nC0, INT32* lpnWI, INT32* lpnW0, INT32* lpnW);
	private: INT32 PrintVectors_Block(INT32 nBlock);
	public: INT16 PrintVectors();

/* Taken from 'data_wrk.c' */
	protected: INT16 Scalop_C(COMPLEX64 nConst, INT16 nOpcode, INT32 nComp);
	public: INT16 Scalop_Int(CData* idSrc, COMPLEX64 nConst, CData* idConst, INT16 nOpcode, INT16 nType, INT32 nComp);
	public: INT16 Aggregate_Int(CData* iSrc, CData* iMask, COMPLEX64 dParam, INT16 nOpcode);
	public: INT16 Strop(CData* idSrc, const char* sParam, const char* sOpname);
	public: INT16 Compress(CData* iSrc, INT32 nComp);
	public: INT16 Expand(CData* idSrc, INT32 nIcE, INT32 nIcS, INT32 nIcL);
	private: INT16 ExpandComp(CData* idSrc, INT32 nIcE, INT32 nIcS, INT32 nIcL);
	public: INT16 SortInt(CData* iSrc, CData* iIdx, INT32 nComp, INT16 nMode);
	private: INT32 ChecksumInt(char* sAlgo, INT32 nIc);
/*}}CGEN_EXPORT */

/* Member variables */
public:
/*{{CGEN_ICXX_FIELDS */
/*}}CGEN_ICXX_FIELDS */

#else  /* #ifdef __cplusplus */

typedef struct CData
{
  /* Pointer to C base instance */
  struct CDlpObject* m_lpBaseInstance;

/*{{CGEN_IC_FIELDS */
/*}}CGEN_IC_FIELDS */

#endif /* #ifdef __cplusplus */

/*{{CGEN_FIELDS */
	FLOAT64          m_nCinc;
	FLOAT64          m_nCofs;
	char             m_lpCunit[10];
	char*            m_ftext;
	CDlpTable*       m_lpTable;
	char*            m_markMap;
	INT32            m_markMapSize;
	INT32            m_markMode;
	INT32            m_nblock;
	INT32            m_noffset;
	char             m_lpRunit[10];
	char             m_lpVunit[10];
/*}}CGEN_FIELDS */

/*{{CGEN_OPTIONS */
	BOOL m_bBlock;
	BOOL m_bCell;
	BOOL m_bComp;
	BOOL m_bExact;
	BOOL m_bFast;
	BOOL m_bForce;
	BOOL m_bLabel;
	BOOL m_bList;
	BOOL m_bMark;
	BOOL m_bMatrix;
	BOOL m_bNoise;
	BOOL m_bNumeric;
	BOOL m_bNz;
	BOOL m_bRec;
/*}}CGEN_OPTIONS */
}

#ifndef __cplusplus
CData
#endif
;

/* Class CData (C functions)*/

/* Virtual function overrides */
void  CData_Constructor(CData*, const char* lpInstanceName, BOOL bCallVirtual);
void  CData_Destructor(CDlpObject*);
INT16 CData_AutoRegisterWords(CDlpObject*);
INT16 CData_Reset(CDlpObject*, BOOL bResetMembers);
INT16 CData_Init(CDlpObject*, BOOL bCallVirtual);
INT16 CData_Serialize(CDlpObject*, CDN3Stream* lpDest);
INT16 CData_SerializeXml(CDlpObject*, CXmlStream* lpDest);
INT16 CData_Deserialize(CDlpObject*, CDN3Stream* lpSrc);
INT16 CData_DeserializeXml(CDlpObject*, CXmlStream* lpSrc);
INT16 CData_Copy(CDlpObject*, CDlpObject* __iSrc);
INT16 CData_ClassProc(CDlpObject*);
INT16 CData_InstallProc(void* lpItp);
CData* CData_CreateInstance(const char* lpName);
INT16 CData_GetClassInfo(SWord* lpClassWord);
INT16 CData_GetInstanceInfo(CDlpObject*, SWord* lpClassWord);
BOOL  CData_IsKindOf(CDlpObject*, const char* lpClassName);
INT16 CData_ResetAllOptions(CDlpObject*, BOOL bInit);

/* Primary method invocation functions             */
/* DO NOT CALL THESE FUNCTIONS FROM C SCOPE.       */
/* THEY MAY INTERFERE WITH THE INTERPRETER SESSION */
#ifndef __NOITP
/*{{CGEN_CPMIC */
INT16 CData_OnInitializeRecord(CDlpObject*);
INT16 CData_OnAddComp(CDlpObject*);
INT16 CData_OnAddNcomps(CDlpObject*);
INT16 CData_OnAggregate(CDlpObject*);
INT16 CData_OnAllocate(CDlpObject*);
INT16 CData_OnArray(CDlpObject*);
INT16 CData_OnCat(CDlpObject*);
INT16 CData_OnChecksum(CDlpObject*);
INT16 CData_OnClear(CDlpObject*);
INT16 CData_OnCompress(CDlpObject*);
INT16 CData_OnCopyCnames(CDlpObject*);
INT16 CData_OnCopyDescr(CDlpObject*);
INT16 CData_OnCopyLabels(CDlpObject*);
INT16 CData_OnDelete(CDlpObject*);
INT16 CData_OnDequantize(CDlpObject*);
INT16 CData_OnDfetch(CDlpObject*);
INT16 CData_OnDmark(CDlpObject*);
INT16 CData_OnDstore(CDlpObject*);
INT16 CData_OnExpand(CDlpObject*);
INT16 CData_OnFetch(CDlpObject*);
INT16 CData_OnFill(CDlpObject*);
INT16 CData_OnFindComp(CDlpObject*);
INT16 CData_OnGenIndex(CDlpObject*);
INT16 CData_OnGetCname(CDlpObject*);
INT16 CData_OnGetCnames(CDlpObject*);
INT16 CData_OnGetCompType(CDlpObject*);
INT16 CData_OnInitialize(CDlpObject*);
INT16 CData_OnInsertComp(CDlpObject*);
INT16 CData_OnInsertNcomps(CDlpObject*);
INT16 CData_OnIsEmpty(CDlpObject*);
INT16 CData_OnJoin(CDlpObject*);
INT16 CData_OnLookup(CDlpObject*);
INT16 CData_OnLookup2(CDlpObject*);
INT16 CData_OnMark(CDlpObject*);
INT16 CData_OnPfetch(CDlpObject*);
INT16 CData_OnPrint(CDlpObject*);
INT16 CData_OnPstore(CDlpObject*);
INT16 CData_OnQuantize(CDlpObject*);
INT16 CData_OnReallocate(CDlpObject*);
INT16 CData_OnRepmat(CDlpObject*);
INT16 CData_OnResample(CDlpObject*);
INT16 CData_OnReshape(CDlpObject*);
INT16 CData_OnRindex(CDlpObject*);
INT16 CData_OnRotate(CDlpObject*);
INT16 CData_OnScalop(CDlpObject*);
INT16 CData_OnScalopD(CDlpObject*);
INT16 CData_OnScopy(CDlpObject*);
INT16 CData_OnSelect(CDlpObject*);
INT16 CData_OnSetCname(CDlpObject*);
INT16 CData_OnSetCnames(CDlpObject*);
INT16 CData_OnSfetch(CDlpObject*);
INT16 CData_OnShift(CDlpObject*);
INT16 CData_OnSortdown(CDlpObject*);
INT16 CData_OnSortup(CDlpObject*);
INT16 CData_OnSstore(CDlpObject*);
INT16 CData_OnStatus(CDlpObject*);
INT16 CData_OnStore(CDlpObject*);
INT16 CData_OnStrop(CDlpObject*);
INT16 CData_OnTconvert(CDlpObject*);
INT16 CData_OnUnmark(CDlpObject*);
INT16 CData_OnXfetch(CDlpObject*);
INT16 CData_OnXstore(CDlpObject*);
INT16 CData_OnArrOp(CDlpObject*);
/*}}CGEN_CPMIC */
#endif /* #ifndef __NOITP */

/* Secondary method invocation functions */
/*{{CGEN_CSMIC */
INT16 CData_Checksum(CData*, char* sAlgo, INT32 nIc);
INT16 CData_Reallocate(CData*, INT32 nRecs);
INT16 CData_Resample(CData*, data* iSrc, FLOAT64 nRate);
INT16 CData_Rindex(CData*, const char* sCname, INT32 nIc);
INT16 CData_Scalop(CData*, data* idSrc, COMPLEX64 nConst, const char* sOpname);
INT16 CData_ScalopD(CData*, data* idSrc, data* idConst, const char* sOpname);
INT16 CData_Sortdown(CData*, data* x, INT32 j);
INT16 CData_Sortup(CData*, data* x, INT32 j);
/*}}CGEN_CSMIC */

/* Option changed callback functions */
/*{{CGEN_COCCF */
/*}}CGEN_COCCF */

/* Field changed callback functions */
/*{{CGEN_CFCCF */
/*}}CGEN_CFCCF */

/* Scanned C (member) functions */
/*{{CGEN_CEXPORT */

/* Taken from 'data_aci.c' */
BYTE* CData_XAddr(CData*, INT32 nRec, INT32 nComp);
INT16 CData_Alloc(CData*, INT32 nRecs);
INT16 CData_AllocUninitialized(CData*, INT32 nRecs);
INT16 CData_AllocateUninitialized(CData*, INT32 nRecs);
INT16 CData_Realloc(CData*, INT32 nRecs);
INT32 CData_AddRecs(CData*, INT32 nRecs, INT32 nRealloc);
INT32 CData_InsertRecs(CData*, INT32 nInsertAt, INT32 nRecs, INT32 nRealloc);
INT32 CData_GetNRecs(CData*);
INT32 CData_GetNComps(CData*);
FLOAT64 CData_GetFsr(CData*);
INT32 CData_GetNNumericComps(CData*);
INT32 CData_GetNComplexComps(CData*);
INT32 CData_GetNBlocks(CData*);
INT32 CData_GetNRecsPerBlock(CData*);
INT32 CData_SetNBlocks(CData*, INT32 nBlocks);
INT32 CData_IncNRecs(CData*, INT32 nRecs);
INT32 CData_SetNRecs(CData*, INT32 nRecs);
INT32 CData_GetMaxRecs(CData*);
INT32 CData_GetCompSize(CData*, INT32 nComp);
INT32 CData_GetCompOffset(CData*, INT32 nComp);
INT32 CData_GetRecLen(CData*);
INT32 CData_FindRec(CData*, INT32 nComp, char* sWhat);
INT32 CData_CcompFetch(CData*, COMPLEX64* dBuffer, INT32 nComp, INT32 nMaxRecs);
INT32 CData_DcompFetch(CData*, FLOAT64* dBuffer, INT32 nComp, INT32 nMaxRecs);
INT32 CData_CcompStore(CData*, COMPLEX64* dBuffer, INT32 nComp, INT32 nMaxRecs);
INT32 CData_DcompStore(CData*, FLOAT64* dBuffer, INT32 nComp, INT32 nMaxRecs);
INT32 CData_CrecFetch(CData*, COMPLEX64* lpBuffer, INT32 nRec, INT32 nMaxComps, INT32 nCompIgnore);
INT32 CData_DrecFetch(CData*, FLOAT64* lpBuffer, INT32 nRec, INT32 nMaxComps, INT32 nCompIgnore);
INT32 CData_CrecFetchInterpol(CData*, COMPLEX64* dBuffer, FLOAT64 nRec, INT32 nMaxComps, INT32 nCompIgnore, INT16 nMode);
INT32 CData_CrecStore(CData*, COMPLEX64* dBuffer, INT32 nRec, INT32 nMaxComps, INT32 nCompIgnore);
INT32 CData_DrecStore(CData*, FLOAT64* dBuffer, INT32 nRec, INT32 nMaxComps, INT32 nCompIgnore);
INT32 CData_CblockFetch(CData*, COMPLEX64* dBuffer, INT32 nBlock, INT32 nMaxComps, INT32 nMaxBrecs, INT32 nCompIgnore);
INT32 CData_DblockFetch(CData*, FLOAT64* dBuffer, INT32 nBlock, INT32 nMaxComps, INT32 nMaxBrecs, INT32 nCompIgnore);
INT32 CData_CblockStore(CData*, COMPLEX64* dBuffer, INT32 nBlock, INT32 nMaxComps, INT32 nMaxBrecs, INT32 nCompIgnore);
INT32 CData_DblockStore(CData*, FLOAT64* dBuffer, INT32 nBlock, INT32 nMaxComps, INT32 nMaxBrecs, INT32 nCompIgnore);
INT32 CData_DijFetch(CData*, FLOAT64* dBuffer, INT32 nRec, INT32 nComp, INT32 nMaxBlocks);
INT32 CData_CijFetch(CData*, COMPLEX64* dBuffer, INT32 nRec, INT32 nComp, INT32 nMaxBlocks);
COMPLEX64 CData_CfetchInterpol(CData*, FLOAT64 nRec, INT32 nComp, INT16 nMode);
INT16 CData_SelectRecs(CData*, data* iSrc, INT32 from, INT32 count);
INT16 CData_SelectBlocks(CData*, data* iSrc, INT32 from, INT32 count);
INT16 CData_SelectComps(CData*, data* iSrc, INT32 from, INT32 count);
INT16 CData_DeleteRecs(CData*, INT32 from, INT32 count);
INT16 CData_DeleteBlocks(CData*, INT32 from, INT32 count);
INT16 CData_DeleteComps(CData*, INT32 nFirst, INT32 nCount);
INT16 CData_CopyMarked(CData*, CData* idSrc, BOOL bPositive);
INT16 CData_CheckCompType(CData*, INT16 nType);
FLOAT64 CData_GetDescr(CData*, INT16 nDescr);
void CData_SetDescr(CData*, INT16 nDescr, FLOAT64 nValue);
INT32 CData_GenIndexList(CData*, data* iSrc, data* iTab, data* iLTab, INT32 nIdx);
INT16 CData_GenLabIndex(CData*, CData* iLabel, CData* iIndex, CData* iLTab);
INT16 CData_CopyComps(CData*, CData* iSrc, INT32 is, INT32 it, INT32 n);
INT16 CData_ResampleInt(CData*, CData* iSrc, FLOAT64 nRate, INT16 nMode);

/* Taken from 'data_iam.c' */
INT16 CData_AddComp(CData*, const char* lpsName, INT16 nType);
INT16 CData_AddNcomps(CData*, INT16 nCType, INT32 nCount);
INT16 CData_InsertComp(CData*, const char* sCName, INT16 nCType, INT32 nInsertAt);
INT16 CData_InsertNcomps(CData*, INT16 nCType, INT32 nInsertAt, INT32 nCount);
INT16 CData_GetCompType(CData*, INT32 nComp);
INT16 CData_Scopy(CData*, data* lpSrc);
INT16 CData_CopyCnames(CData*, data* iSrc, INT32 jx, INT32 jy, INT32 n);
INT16 CData_SetCnames(CData*, data* x, INT32 jx);
INT16 CData_GetCnames(CData*, CData* idSrc);
INT16 CData_SetCname(CData*, INT32 nIComp, const char* sCName);
const char* CData_GetCname(CData*, INT32 nIComp);
INT32 CData_FindComp(CData*, const char* lpName);
INT16 CData_Allocate(CData*, INT32 n);
INT16 CData_Array(CData*, INT16 nCType, INT32 nComps, INT32 nRecs);
BOOL CData_IsEmpty(CData*);
COMPLEX64 CData_Cfetch(CData*, INT32 nIRec, INT32 nIComp);
FLOAT64 CData_Dfetch(CData*, INT32 nIRec, INT32 nIComp);
void* CData_Pfetch(CData*, INT32 nIRec, INT32 nIComp);
const char* CData_Sfetch(CData*, INT32 nIRec, INT32 nIComp);
INT16 CData_Cstore(CData*, COMPLEX64 dVal, INT32 nIRec, INT32 nIComp);
INT16 CData_Dstore(CData*, FLOAT64 dVal, INT32 nIRec, INT32 nIComp);
INT16 CData_Pstore(CData*, void* lpVal, INT32 nIRec, INT32 nIComp);
INT16 CData_Sstore(CData*, const char* sVal, INT32 nIRec, INT32 nIComp);
INT16 CData_Clear(CData*);
INT16 CData_Fill(CData*, COMPLEX64 dStart, COMPLEX64 dDelta);
INT16 CData_Lookup(CData*, data* dSel, INT32 nSelComp, data* dTab, INT32 nTabComp, INT32 nCount);
INT16 CData_Lookup2(CData*, data* dSel1, INT32 nSel1Comp, data* dSel2, INT32 nSel2Comp, data* dTab);
INT16 CData_Aggregate(CData*, CData* iSrc, CData* iMask, COMPLEX64 dParam, const char* lpOpname);
INT16 CData_GenIndex(CData*, data* iSrc, data* iTab, INT32 nSrcIdx, INT32 nTabIdx);
INT16 CData_Tconvert(CData*, data* idSrc, INT16 nCType);
INT16 CData_CopyDescr(CData*, CData* iSrc);
INT16 CData_CopyMark(CData*, CData* iSrc);
INT16 CData_CopyLabels(CData*, CData* iSrc);
INT16 CData_Join(CData*, data* x);
INT16 CData_NJoin(CData*, data* x, INT32 jx, INT32 n);
INT16 CData_Cat(CData*, data* x);
INT16 CData_Select(CData*, CData* idSrc, INT32 nFirst, INT32 nCount);
INT16 CData_Delete(CData*, CData* idSrc, INT32 nFirst, INT32 nCount);
INT16 CData_Mark(CData*, INT32 jx, INT32 n);
INT16 CData_Dmark(CData*, CData* idMark);
INT16 CData_Unmark(CData*);
INT16 CData_Xfetch(CData*, INT32 nFirst, INT32 nCount);
BOOL CData_Xstore(CData*, CData* idSrc, INT32 nFirst, INT32 nCount, INT32 nPos);
BOOL CData_Reshape(CData*, CData* iSrc, INT32 nRec, INT32 nComp);
BOOL CData_Repmat(CData*, CData* iSrc, INT32 nRec, INT32 nComp);
INT16 CData_Shift(CData*, CData* iSrc, INT32 nCount);
INT16 CData_Rotate(CData*, CData* iSrc, INT32 nCount);
INT16 CData_IsHomogen(CData*);
INT16 CData_Print(CData*);
INT16 CData_Status(CData*);
INT16 CData_ArrOp(CData*);
INT16 CData_Quantize(CData*, CData* dIn);
INT16 CData_Dequantize(CData*, CData* dIn);

/* Taken from 'data_ini.c' */
INT16 CData_InitializeEx(CData*, INT32 nRec, INT32 nComp, INT32 nCount);
INT32 CData_ReadInitializer(CData*, char* lpsInit, INT32 nLen, BOOL bForce);
INT16 CData_InitializeRecordEx(CData*, const char* lpsInit, INT32 nRec, INT32 nComp);
INT16 CData_Initialize(CData*);
INT16 CData_InitializeRecord(CData*, INT32 nRec);

/* Taken from 'data_int.c' */
INT32 CData_Find(CData*, INT32 nRecStart, INT32 nRecEnd, INT32 nCountComp,  ...);
INT16 CData_VerifyMarkMap(CData*);
INT16 CData_MarkElem(CData*, INT32 nElem);
BOOL CData_IsMarked(CData*, INT32 nElem);
BOOL CData_RecIsMarked(CData*, INT32 nRec);
BOOL CData_CompIsMarked(CData*, INT32 nComp);
BOOL CData_BlockIsMarked(CData*, INT32 nBlock);
BOOL CData_CellIsMarked(CData*, INT32 nCell);
INT32 CData_MicGetIc(CData*, INT16 nArg, INT16 nPos);

/* Taken from 'data_prt.c' */
INT16 CData_PrintText(CData*);
INT16 CData_PrintRec(CData*, INT32 nRec, INT32 nIcFirst, INT32 nComps, INT16 nIndent);
INT16 CData_PrintList(CData*);
INT16 CData_PrintVectors_GetColWidth(CData*, INT32 nR0, INT32 nC0, INT32* lpnWI, INT32* lpnW0, INT32* lpnW);
INT32 CData_PrintVectors_Block(CData*, INT32 nBlock);
INT16 CData_PrintVectors(CData*);

/* Taken from 'data_wrk.c' */
INT16 CData_Scalop_C(CData*, COMPLEX64 nConst, INT16 nOpcode, INT32 nComp);
INT16 CData_Scalop_Int(CData*, CData* idSrc, COMPLEX64 nConst, CData* idConst, INT16 nOpcode, INT16 nType, INT32 nComp);
INT16 CData_Aggregate_Int(CData*, CData* iSrc, CData* iMask, COMPLEX64 dParam, INT16 nOpcode);
INT16 CData_Strop(CData*, CData* idSrc, const char* sParam, const char* sOpname);
INT16 CData_Compress(CData*, CData* iSrc, INT32 nComp);
INT16 CData_Expand(CData*, CData* idSrc, INT32 nIcE, INT32 nIcS, INT32 nIcL);
INT16 CData_ExpandComp(CData*, CData* idSrc, INT32 nIcE, INT32 nIcS, INT32 nIcL);
INT16 CData_SortInt(CData*, CData* iSrc, CData* iIdx, INT32 nComp, INT16 nMode);
INT32 CData_ChecksumInt(CData*, char* sAlgo, INT32 nIc);
/*}}CGEN_CEXPORT */

#endif /*#ifndef __DATA_H */


/* EOF */
