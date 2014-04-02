/* dLabPro class CDlpFile (file)
 * - Header file
 *
 * AUTHOR : M. Wolff, M. Eichner and M. Cuevas
 * PACKAGE: dLabPro/classes
 *
 * This file was generated by dcg. DO NOT MODIFY! Modify file.def instead.
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
#include "ipkclib.h"
#include "kzl_list.h"
#include "dlp_data.h"
/*}}CGEN_END */

/*{{CGEN_ERRORS */
#undef FIL_NEEDFL          
#undef FIL_FILENAME        
#undef FIL_NOIMEX          
#undef FIL_IMPORT          
#undef FIL_NOEXPORT        
#undef FIL_EXPORT          
#undef FIL_PROCESS         
#undef FIL_FORMAT          
#undef FIL_FORMATW         
#undef FIL_FORMATCOMPS     
#undef FIL_FSR             
#undef FIL_PHD_SMM         
#undef FIL_PHD_EMPTY_HEADER
#undef FIL_NOTIMPL         
#undef FIL_BADTYPE         
#undef FIL_BADCOMPS        
#undef IC_TIS_NOT_SET      
#undef FIL_DLP             
#undef FIL_REMOVE          
#undef FIL_EXEC            
#undef FIL_NOTFOUND        
#undef FIL_WAV_MISS        
#undef FIL_INVALARG        
#undef FIL_OPTION          
#define FIL_NEEDFL           -1001
#define FIL_FILENAME         -1002
#define FIL_NOIMEX           -1003
#define FIL_IMPORT           -1004
#define FIL_NOEXPORT         -1005
#define FIL_EXPORT           -1006
#define FIL_PROCESS          -1007
#define FIL_FORMAT           -1008
#define FIL_FORMATW          -1009
#define FIL_FORMATCOMPS      -1010
#define FIL_FSR              -1011
#define FIL_PHD_SMM          -1012
#define FIL_PHD_EMPTY_HEADER -1013
#define FIL_NOTIMPL          -1014
#define FIL_BADTYPE          -1015
#define FIL_BADCOMPS         -1016
#define IC_TIS_NOT_SET       -1017
#define FIL_DLP              -1018
#define FIL_REMOVE           -1019
#define FIL_EXEC             -1020
#define FIL_NOTFOUND         -1021
#define FIL_WAV_MISS         -1022
#define FIL_INVALARG         -1023
#define FIL_OPTION           -1024
/*}}CGEN_END */

/* C/C++ language abstraction layer */
#undef file_par
#define file_par CDlpFile

/* dLabPro/C++ language abstraction layer */
#undef file
#define file CDlpFile

/*{{CGEN_DEFINE */
#define IMPORT_FILE  1
#define EXPORT_FILE  2
#define ADD_FILETYPE(A,B,C,D,E) {   lnode_t* newNode = NULL;   CFILE_FTYPE* lpGhost = NULL;   lpGhost = (CFILE_FTYPE*)dlp_calloc(1,sizeof(CFILE_FTYPE));   if (lpGhost)   {     dlp_strncpy(lpGhost->lpName,A,L_NAMES-1);     dlp_strncpy(lpGhost->lpClassName,B,L_NAMES-1);     dlp_strncpy(lpGhost->lpDescr,D,2*L_NAMES-1);     lpGhost->nMode = C;     lpGhost->FilterFunc = E;   }   else return IERROR(_this,ERR_NOMEM,0,0,0);   newNode = lnode_create(lpGhost);   list_append(_this->m_lpFtypes, newNode); }
#define MAX_INVDESCR_LINE_LENGTH 1024
/*}}CGEN_DEFINE */

#ifndef __FILE_H
#define __FILE_H

/*{{CGEN_HEADERCODE */

  #ifdef __cplusplus
    class CDlpFile;
  #else
    struct CDlpFile;
  #endif

  typedef struct tagCFILE_FTYPE
  {
    char  lpName[L_NAMES];
    char  lpClassName[L_NAMES];
    char  lpDescr[2*L_NAMES];
    INT16 nMode;
  #ifdef __cplusplus
    INT16 (*FilterFunc)(CDlpFile*, const char* lpFilename, CDlpObject* iDest, const char* lpFiletype);
  #else
    INT16 (*FilterFunc)(struct CDlpFile*, const char* lpFilename, CDlpObject* iDest, const char* lpFiletype);
  #endif
  }CFILE_FTYPE;

/*}}CGEN_HEADERCODE */

/* Class CDlpFile */

#ifdef __cplusplus

class CDlpFile : public CDlpObject 
{

typedef CDlpObject inherited;
typedef CDlpFile thisclass;

/*{{CGEN_FRIENDS */
/*}}CGEN_FRIENDS */
public:
	CDlpFile(const char* lpInstanceName, BOOL bCallVirtual = 1);
	virtual ~CDlpFile();

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
	static  CDlpFile* CreateInstance(const char* lpName);
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
	INT16 OnExists();
	INT16 OnExport();
	INT16 OnGetRootClass();
	INT16 OnGetlen();
	INT16 OnImport();
	INT16 OnList();
	INT16 OnNext();
	INT16 OnPartition();
/*}}CGEN_PMIC */
#endif /* #ifndef __NOITP */

/* Secondary method invocation functions */
public:
/*{{CGEN_SMIC */
	INT16 Export(const char* sFilename, const char* sFilter, CDlpObject* iInst);
	INT32 Getlen();
	INT16 Import(const char* sFilename, const char* sFilter, CDlpObject* iInst);
	INT16 List();
/*}}CGEN_SMIC */

/* Option changed callback functions */
public:
/*{{CGEN_OCCF */
/*}}CGEN_OCCF */

/* Field changed callback functions */
public:
/*{{CGEN_FCCF */
	INT16 OnExtChanged();
	INT16 OnFlistChanged();
	INT16 OnFlistDataChanged();
	INT16 OnPathChanged();
/*}}CGEN_FCCF */

/* Scanned member functions */
/*{{CGEN_EXPORT */

/* Taken from 'file_aux.c' */
	public: BOOL Exists(const char* sFilename);
	public: const char* GetRootClass(const char* lpsFilename);
	public: const char* Next();
	public: INT16 Partition(CDlpFile* iSrc, FLOAT64 nPartSize, INT32 nPartNum);

/* Taken from 'file_data.c' */
	protected: INT16 ImportAsciiToData(const char* sFilename, CDlpObject* iDest, const char* sFiletype);
	protected: INT16 ExportAsciiFromData(const char* sFilename, CDlpObject* iSrc, const char* sFiletype);
	protected: INT16 LibsndfileImport(const char* sFilename, CDlpObject* iDest, const char* sFiletype);
	protected: INT16 LibsndfileExport(const char* sFilename, CDlpObject* iSrc, const char* sFiletype);
	protected: INT16 ImportPhDToData(const char* sFilename, CDlpObject* iDest, const char* sFiletype);
	protected: INT16 ImportEspsLabToData(const char* sFilename, CDlpObject* iDest, const char* sFiletype);
	protected: INT16 ExportEspsLabFromData(const char* sFilename, CDlpObject* iSrc, const char* sFiletype);
	protected: INT16 ImportPmToData(const char* sFilename, CDlpObject* iDest, const char* sFiletype);
	protected: INT16 ImportPmTxtToData(const char* sFilename, CDlpObject* iDest, const char* sFiletype);
	protected: INT16 ExportPmFromData(const char* sFilename, CDlpObject* iSrc, const char* sFiletype);
	protected: INT16 ExportPmTxtFromData(const char* sFilename, CDlpObject* iSrc, const char* sFiletype);
	protected: INT16 ImportIntoToData(const char* sFilename, CDlpObject* iDest, const char* sFiletype);
	protected: INT16 ExportIntoFromData(const char* sFilename, CDlpObject* iSrc, const char* sFiletype);
	protected: INT16 ImportRawToData(const char* sFilename, CDlpObject* iDest, const char* sFiletype);
	protected: INT16 ImportIzfpRsToData(const char* sFilename, CDlpObject* iDest, const char* sFiletype);
	protected: INT16 ExportRawFromData(const char* sFilename, CDlpObject* iSrc, const char* sFiletype);
	protected: INT16 ImportWaveToData(const char* sFilename, CDlpObject* iDest, const char* sFiletype);
	protected: INT16 ImportInvDescrToData(const char* sFilename, CDlpObject* iSrc, const char* sFiletype);
	protected: INT16 ExportInvDescrFromData(const char* sFilename, CDlpObject* iSrc, const char* sFiletype);
	protected: INT16 ExportPstricksFromData(const char* sFilename, CDlpObject* iSrc, const char* sFiletype);

/* Taken from 'file_gph.c' */
	protected: INT16 Gph_ExportDot(const char* lpsFilename, CDlpObject* iSrc, const char* lpsFiletype);
	protected: INT16 Gph_ExportDotTx(const char* lpsFilename, CDlpObject* iSrc, const char* lpsFiletype);
	protected: INT16 Gph_ExportFsm(const char* lpsFilename, CDlpObject* iSrc, const char* lpsFiletype);
	protected: INT16 Gph_ImportFsm(const char* lpsFilename, CDlpObject* iDst, const char* lpsFiletype);
	private: static void Gph_ExportItp_WriteCI(FILE* fDst, CData* idSrc, INT32 nFC, INT32 nR);
	protected: INT16 Gph_ExportItp(const char* lpsFilename, CDlpObject* iSrc, const char* lpsFiletype);

/* Taken from 'file_hmm.c' */
	protected: INT16 Hmm_ExportHtk(const char* lpsFilename, CDlpObject* iSrc, const char* lpsFiletype);

/* Taken from 'file_imex.c' */
	protected: INT16 ImportExport(const char* sFilename, const char* sFilter, CDlpObject* iInst, INT16 nMode);
	private: static void FreeFTypeList(list_t* lpList, lnode_t* lpNode, void* context);
	private: static int CompareFTypeList(const void* key, const void* node);

/* Taken from 'file_vm2n.c' */
	private: static void FlipNumericData(CData* idDest);
	private: static BOOL ParseNIST(char* sLine, char** sKey, char** sType, char** sValue);
	protected: INT16 ImportDataFromVm2Nist(const char* sFilename, CDlpObject* iDst, const char* sFiletype);

/* Taken from 'file_midi.c' */
	protected: INT16 Midi_ImportMidi(const char* lpsFilename, CDlpObject* iDst, const char* lpsFiletype);
	protected: INT16 Midi_ExportMidi(const char* lpsFilename, CDlpObject* iSrc, const char* lpsFiletype);
/*}}CGEN_EXPORT */

/* Member variables */
public:
/*{{CGEN_ICXX_FIELDS */
	CData*           m_idFlistData;
	CData*           m_idRecfile;
/*}}CGEN_ICXX_FIELDS */

#else  /* #ifdef __cplusplus */

typedef struct CDlpFile
{
  /* Pointer to C base instance */
  struct CDlpObject* m_lpBaseInstance;

/*{{CGEN_IC_FIELDS */
	CDlpObject*      m_idFlistData;
	CDlpObject*      m_idRecfile;
/*}}CGEN_IC_FIELDS */

#endif /* #ifdef __cplusplus */

/*{{CGEN_FIELDS */
	char             m_lpsComment[10];
	char             m_lpsExt[32];
	char             m_lpsFlist[255];
	list_t*          m_lpFtypes;
	INT32            m_nLen;
	char             m_lpsLineFlt[32];
	INT32            m_nNfile;
	char             m_lpsPath[255];
	INT64            m_nRawHead;
	char             m_lpsSep[10];
	char             m_lpsSfile[255];
	char             m_lpsSfileFq[255];
/*}}CGEN_FIELDS */

/*{{CGEN_OPTIONS */
	BOOL m_bAppend;
	BOOL m_bBig;
	BOOL m_bCompress;
	BOOL m_bDir;
	BOOL m_bExecute;
	BOOL m_bLittle;
	BOOL m_bPstComma;
	BOOL m_bPstContour;
	BOOL m_bPstHalfspectrum;
	BOOL m_bPstLegend;
	BOOL m_bPstTriglabels;
	BOOL m_bPstXYPlot;
	BOOL m_bReverse;
	BOOL m_bStrings;
	BOOL m_bTranspose;
	BOOL m_bZip;
/*}}CGEN_OPTIONS */
}

#ifndef __cplusplus
CDlpFile
#endif
;

/* Class CDlpFile (C functions)*/

/* Virtual function overrides */
void  CDlpFile_Constructor(CDlpFile*, const char* lpInstanceName, BOOL bCallVirtual);
void  CDlpFile_Destructor(CDlpObject*);
INT16 CDlpFile_AutoRegisterWords(CDlpObject*);
INT16 CDlpFile_Reset(CDlpObject*, BOOL bResetMembers);
INT16 CDlpFile_Init(CDlpObject*, BOOL bCallVirtual);
INT16 CDlpFile_Serialize(CDlpObject*, CDN3Stream* lpDest);
INT16 CDlpFile_SerializeXml(CDlpObject*, CXmlStream* lpDest);
INT16 CDlpFile_Deserialize(CDlpObject*, CDN3Stream* lpSrc);
INT16 CDlpFile_DeserializeXml(CDlpObject*, CXmlStream* lpSrc);
INT16 CDlpFile_Copy(CDlpObject*, CDlpObject* __iSrc);
INT16 CDlpFile_ClassProc(CDlpObject*);
INT16 CDlpFile_InstallProc(void* lpItp);
CDlpFile* CDlpFile_CreateInstance(const char* lpName);
INT16 CDlpFile_GetClassInfo(SWord* lpClassWord);
INT16 CDlpFile_GetInstanceInfo(CDlpObject*, SWord* lpClassWord);
BOOL  CDlpFile_IsKindOf(CDlpObject*, const char* lpClassName);
INT16 CDlpFile_ResetAllOptions(CDlpObject*, BOOL bInit);

/* Primary method invocation functions             */
/* DO NOT CALL THESE FUNCTIONS FROM C SCOPE.       */
/* THEY MAY INTERFERE WITH THE INTERPRETER SESSION */
#ifndef __NOITP
/*{{CGEN_CPMIC */
INT16 CDlpFile_OnExists(CDlpObject*);
INT16 CDlpFile_OnExport(CDlpObject*);
INT16 CDlpFile_OnGetRootClass(CDlpObject*);
INT16 CDlpFile_OnGetlen(CDlpObject*);
INT16 CDlpFile_OnImport(CDlpObject*);
INT16 CDlpFile_OnList(CDlpObject*);
INT16 CDlpFile_OnNext(CDlpObject*);
INT16 CDlpFile_OnPartition(CDlpObject*);
/*}}CGEN_CPMIC */
#endif /* #ifndef __NOITP */

/* Secondary method invocation functions */
/*{{CGEN_CSMIC */
INT16 CDlpFile_Export(CDlpFile*, const char* sFilename, const char* sFilter, CDlpObject* iInst);
INT32 CDlpFile_Getlen(CDlpFile*);
INT16 CDlpFile_Import(CDlpFile*, const char* sFilename, const char* sFilter, CDlpObject* iInst);
INT16 CDlpFile_List(CDlpFile*);
/*}}CGEN_CSMIC */

/* Option changed callback functions */
/*{{CGEN_COCCF */
/*}}CGEN_COCCF */

/* Field changed callback functions */
/*{{CGEN_CFCCF */
INT16 CDlpFile_OnExtChanged(CDlpObject*);
INT16 CDlpFile_OnFlistChanged(CDlpObject*);
INT16 CDlpFile_OnFlistDataChanged(CDlpObject*);
INT16 CDlpFile_OnPathChanged(CDlpObject*);
/*}}CGEN_CFCCF */

/* Scanned C (member) functions */
/*{{CGEN_CEXPORT */

/* Taken from 'file_aux.c' */
BOOL CDlpFile_Exists(CDlpFile*, const char* sFilename);
const char* CDlpFile_GetRootClass(CDlpFile*, const char* lpsFilename);
const char* CDlpFile_Next(CDlpFile*);
INT16 CDlpFile_Partition(CDlpFile*, CDlpFile* iSrc, FLOAT64 nPartSize, INT32 nPartNum);

/* Taken from 'file_data.c' */
INT16 CDlpFile_ImportAsciiToData(CDlpFile*, const char* sFilename, CDlpObject* iDest, const char* sFiletype);
INT16 CDlpFile_ExportAsciiFromData(CDlpFile*, const char* sFilename, CDlpObject* iSrc, const char* sFiletype);
INT16 CDlpFile_LibsndfileImport(CDlpFile*, const char* sFilename, CDlpObject* iDest, const char* sFiletype);
INT16 CDlpFile_LibsndfileExport(CDlpFile*, const char* sFilename, CDlpObject* iSrc, const char* sFiletype);
INT16 CDlpFile_ImportPhDToData(CDlpFile*, const char* sFilename, CDlpObject* iDest, const char* sFiletype);
INT16 CDlpFile_ImportEspsLabToData(CDlpFile*, const char* sFilename, CDlpObject* iDest, const char* sFiletype);
INT16 CDlpFile_ExportEspsLabFromData(CDlpFile*, const char* sFilename, CDlpObject* iSrc, const char* sFiletype);
INT16 CDlpFile_ImportPmToData(CDlpFile*, const char* sFilename, CDlpObject* iDest, const char* sFiletype);
INT16 CDlpFile_ImportPmTxtToData(CDlpFile*, const char* sFilename, CDlpObject* iDest, const char* sFiletype);
INT16 CDlpFile_ExportPmFromData(CDlpFile*, const char* sFilename, CDlpObject* iSrc, const char* sFiletype);
INT16 CDlpFile_ExportPmTxtFromData(CDlpFile*, const char* sFilename, CDlpObject* iSrc, const char* sFiletype);
INT16 CDlpFile_ImportIntoToData(CDlpFile*, const char* sFilename, CDlpObject* iDest, const char* sFiletype);
INT16 CDlpFile_ExportIntoFromData(CDlpFile*, const char* sFilename, CDlpObject* iSrc, const char* sFiletype);
INT16 CDlpFile_ImportRawToData(CDlpFile*, const char* sFilename, CDlpObject* iDest, const char* sFiletype);
INT16 CDlpFile_ImportIzfpRsToData(CDlpFile*, const char* sFilename, CDlpObject* iDest, const char* sFiletype);
INT16 CDlpFile_ExportRawFromData(CDlpFile*, const char* sFilename, CDlpObject* iSrc, const char* sFiletype);
INT16 CDlpFile_ImportWaveToData(CDlpFile*, const char* sFilename, CDlpObject* iDest, const char* sFiletype);
INT16 CDlpFile_ImportInvDescrToData(CDlpFile*, const char* sFilename, CDlpObject* iSrc, const char* sFiletype);
INT16 CDlpFile_ExportInvDescrFromData(CDlpFile*, const char* sFilename, CDlpObject* iSrc, const char* sFiletype);
INT16 CDlpFile_ExportPstricksFromData(CDlpFile*, const char* sFilename, CDlpObject* iSrc, const char* sFiletype);

/* Taken from 'file_gph.c' */
INT16 CDlpFile_Gph_ExportDot(CDlpFile*, const char* lpsFilename, CDlpObject* iSrc, const char* lpsFiletype);
INT16 CDlpFile_Gph_ExportDotTx(CDlpFile*, const char* lpsFilename, CDlpObject* iSrc, const char* lpsFiletype);
INT16 CDlpFile_Gph_ExportFsm(CDlpFile*, const char* lpsFilename, CDlpObject* iSrc, const char* lpsFiletype);
INT16 CDlpFile_Gph_ImportFsm(CDlpFile*, const char* lpsFilename, CDlpObject* iDst, const char* lpsFiletype);
void CDlpFile_Gph_ExportItp_WriteCI(FILE* fDst, CData* idSrc, INT32 nFC, INT32 nR);
INT16 CDlpFile_Gph_ExportItp(CDlpFile*, const char* lpsFilename, CDlpObject* iSrc, const char* lpsFiletype);

/* Taken from 'file_hmm.c' */
INT16 CDlpFile_Hmm_ExportHtk(CDlpFile*, const char* lpsFilename, CDlpObject* iSrc, const char* lpsFiletype);

/* Taken from 'file_imex.c' */
INT16 CDlpFile_ImportExport(CDlpFile*, const char* sFilename, const char* sFilter, CDlpObject* iInst, INT16 nMode);
void CDlpFile_FreeFTypeList(list_t* lpList, lnode_t* lpNode, void* context);
int CDlpFile_CompareFTypeList(const void* key, const void* node);

/* Taken from 'file_vm2n.c' */
void CDlpFile_FlipNumericData(CData* idDest);
BOOL CDlpFile_ParseNIST(char* sLine, char** sKey, char** sType, char** sValue);
INT16 CDlpFile_ImportDataFromVm2Nist(CDlpFile*, const char* sFilename, CDlpObject* iDst, const char* sFiletype);

/* Taken from 'file_midi.c' */
INT16 CDlpFile_Midi_ImportMidi(CDlpFile*, const char* lpsFilename, CDlpObject* iDst, const char* lpsFiletype);
INT16 CDlpFile_Midi_ExportMidi(CDlpFile*, const char* lpsFilename, CDlpObject* iSrc, const char* lpsFiletype);
/*}}CGEN_CEXPORT */

#endif /*#ifndef __FILE_H */


/* EOF */
