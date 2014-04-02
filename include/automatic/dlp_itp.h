// dLabPro class CItp (itp)
// - Header file
//
// AUTHOR : matthias wolff
// PACKAGE: dLabPro/sdk
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
#include "dlp_object.h"
#include "dlp_clist.h"
//}}CGEN_END

//{{CGEN_ERRORS
#undef ITPERR_NOMEM        
#undef ITPERR_NOOPEN       
#undef ITPERR_NUMPAR       
#undef ITPERR_MISSING_ONAME
#undef ITPERR_OPEN_FILE    
#undef ITPERR_MISSING_FILENAME
#undef ITPERR_MISSING_CLASSNAME
#undef ITPERR_ZERO_OBJECT  
#undef ITPERR_MISSARG      
#undef ITPERR_INTERNAL     
#undef ITPERR_NO_FIELDS    
#undef ITPERR_UNKNOWN      
#undef ITPERR_DOUBLEDEF    
#undef ITPERR_STACKEMPTY   
#undef ITPERR_ELSE         
#undef ITPERR_ENDIF        
#undef ITPERR_IFELSEENDIF  
#undef ITPERR_MISS_LABEL   
#undef ITPERR_INVALID      
#undef ITPERR_INSTANCE     
#undef ITPERR_MISSING_FIELDVAL
#undef ITPERR_CONVERT      
#undef ITPERR_NOSETFIELD   
#undef ITPERR_ISNOINST     
#undef ITPERR_ISNOINSTNAME 
#undef ITPERR_NOSTATEMENTS 
#undef ITPERR_MISS_IDASTMNT
#undef ITPERR_NOFILEWAITING
#undef ITPERR_TOOMANYARGS  
#undef ITPERR_PREPROC      
#undef ITPERR_EXPECTAFTER  
#undef ITPERR_NOSUCHSTACK  
#undef ITPERR_HASNOFACTORY 
#undef ITPERR_HASNOHANDLER 
#undef ITPERR_SYSCMD       
#undef ITPERR_REFERENCED   
#undef ITPERR_DESTROY      
#undef ITPERR_ITYPEMISMATCH
#undef ITPERR_REFERSITSELF 
#undef ITPERR_LEVEL        
#undef ITPERR_RESETONROOT  
#undef ITPERR_NOFILE       
#undef ITPERR_STDFORM      
#undef ITPERR_CMDLPARAM    
#undef ITPERR_PATH         
#undef ITPERR_CHDIR        
#undef ITPERR_HASLOCALINST 
#undef ITPERR_STREXCEEDSLINE
#undef ITPERR_NOSUCHLIST   
#undef ITPERR_CALLBACK     
#undef ITPERR_NETWORK      
#undef ITPERR_NETWORKDRVMSG
#undef ITPERR_SETSYNTAX    
#undef ITPERR_CANTEXECFIELD
#undef ITPERR_BROKENPIPE   
#undef ITPERR_DN3          
#undef ITPERR_FIELDPTRNULL 
#undef ITPERR_INVALIDAFTER 
#define ITPERR_NOMEM         -1001
#define ITPERR_NOOPEN        -1002
#define ITPERR_NUMPAR        -1003
#define ITPERR_MISSING_ONAME -1004
#define ITPERR_OPEN_FILE     -1005
#define ITPERR_MISSING_FILENAME -1006
#define ITPERR_MISSING_CLASSNAME -1007
#define ITPERR_ZERO_OBJECT   -1008
#define ITPERR_MISSARG       -1009
#define ITPERR_INTERNAL      -1010
#define ITPERR_NO_FIELDS     -1011
#define ITPERR_UNKNOWN       -1012
#define ITPERR_DOUBLEDEF     -1013
#define ITPERR_STACKEMPTY    -1014
#define ITPERR_ELSE          -1015
#define ITPERR_ENDIF         -1016
#define ITPERR_IFELSEENDIF   -1017
#define ITPERR_MISS_LABEL    -1018
#define ITPERR_INVALID       -1019
#define ITPERR_INSTANCE      -1020
#define ITPERR_MISSING_FIELDVAL -1021
#define ITPERR_CONVERT       -1022
#define ITPERR_NOSETFIELD    -1023
#define ITPERR_ISNOINST      -1024
#define ITPERR_ISNOINSTNAME  -1025
#define ITPERR_NOSTATEMENTS  -1026
#define ITPERR_MISS_IDASTMNT -1027
#define ITPERR_NOFILEWAITING -1028
#define ITPERR_TOOMANYARGS   -1029
#define ITPERR_PREPROC       -1030
#define ITPERR_EXPECTAFTER   -1031
#define ITPERR_NOSUCHSTACK   -1032
#define ITPERR_HASNOFACTORY  -1033
#define ITPERR_HASNOHANDLER  -1034
#define ITPERR_SYSCMD        -1035
#define ITPERR_REFERENCED    -1036
#define ITPERR_DESTROY       -1037
#define ITPERR_ITYPEMISMATCH -1038
#define ITPERR_REFERSITSELF  -1039
#define ITPERR_LEVEL         -1040
#define ITPERR_RESETONROOT   -1041
#define ITPERR_NOFILE        -1042
#define ITPERR_STDFORM       -1043
#define ITPERR_CMDLPARAM     -1044
#define ITPERR_PATH          -1045
#define ITPERR_CHDIR         -1046
#define ITPERR_HASLOCALINST  -1047
#define ITPERR_STREXCEEDSLINE -1048
#define ITPERR_NOSUCHLIST    -1049
#define ITPERR_CALLBACK      -1050
#define ITPERR_NETWORK       -1051
#define ITPERR_NETWORKDRVMSG -1052
#define ITPERR_SETSYNTAX     -1053
#define ITPERR_CANTEXECFIELD -1054
#define ITPERR_BROKENPIPE    -1055
#define ITPERR_DN3           -1056
#define ITPERR_FIELDPTRNULL  -1057
#define ITPERR_INVALIDAFTER  -1058
//}}CGEN_END

// C/C++ language abstraction layer
#undef itp_par
#define itp_par CItp

// dLabPro/C++ language abstraction layer
#undef itp
#define itp CItp

//{{CGEN_DEFINE
#define S_LABEL "label"
#define S_NOPREPROC "/nopreproc"
#define S_QERROR "?error"
#define S_IF "if"
#define S_ELSE "else"
#define S_ENDIF "endif"
#define S_STDFORM "stdform"
#define S_QUIT "quit"
#define SM_NONSTOP 0x0001
#define SM_STEP 0x0002
#define SM_NOPREPROC 0x0004
#define SM_NOFORMULAE 0x0008
#define SM_FILEPROMPT 0x0010
#define SM_PIPEMODE 0x0020
#define SM_NOLOGO 0x0040
#define SM_QUIT 0x4000
#define RUN_IF 0x0001
#define RUN_ELSE 0x0002
#define WAIT_ELSE 0x0003
#define WAIT_ENDIF 0x0004
#define SCH_PRIMARY 0x0000
#define SCH_SECONDARY 0x0001
#define SCH_ROOT 0x0002
#define REMAP_FIELD_DLPI(A) { SWord* lpWord; DLPASSERT((lpWord = CDlpObject_FindWord(BASEINST(_this),A,WL_TYPE_FIELD))); lpWord->lpData = CDlpObject_GetStaticFieldPtr(A); }
//}}CGEN_DEFINE

#ifndef __ITP_H
#define __ITP_H

//{{CGEN_HEADERCODE
  #ifndef ERRORRET
    #define ERRORRET(ERR,A,B,C,RET) { IERROR(_this,ERR,A,B,C); return RET; }
    #define ERRORMSG(ERR,A,B,C)       IERROR(_this,ERR,A,B,C)
  #endif
  // -- Global static members of CDlpInstance
  //extern INT16       __nElevel;
  //extern INT64       __nErrors;
  //extern INT64       __nWarnings;
  //extern CDlpInstance* __lpLastErrorInst;
  //extern INT16       __nLastError;
  //extern char          __lpTraceError[255];
  // -- Struct SLABEL
  typedef struct SLABEL
  {
    char           lpName[L_NAMES];
    INT64        nFPos;
    INT64        nLine;
    struct SLABEL* next;
  } SLABEL;
  // -- SLFILE
  class SLFILE
  {
    // Methods
    public:  SLFILE();
    public:  ~SLFILE();
    // Members
    BOOL     bTemporary;     /* Delete file after using it!     */
    char*    lpFName;        /* File name                       */
    char*    lpRunFName;     /* Currently interpreted file!     */
    SLABEL*  lpLabels;       /* List of labels in file          */
    FILE*    lpFILE;         /* The file pointer                */
    INT16  argc;           /* Number of command line args.    */
    char**   argv;           /* Command line arguments          */
    INT64  nLine;          /* Current line (counted)          */
    BOOL     bDelArgv;       /* Delete command line atfer using */
  };
//}}CGEN_HEADERCODE

// Class CItp

class CItp : public CDlpObject 
{

typedef CDlpObject inherited;
typedef CItp thisclass;

//{{CGEN_FRIENDS
	friend class CCgen;
	friend class CForm;
	friend class CFunction;
//}}CGEN_FRIENDS
public:
	CItp(const char* lpInstanceName, BOOL bCallVirtual = 1);
	virtual ~CItp();

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
	static  CItp* CreateInstance(const char* lpName);
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
	INT16 OnShell();
	INT16 OnCd();
	INT16 OnCmdCopy();
	INT16 OnEcho();
	INT16 OnIsInstance();
	INT16 OnCmdRestore();
	INT16 OnCmdSave();
	INT16 OnSystem();
	INT16 OnPlatform();
	INT16 OnBreak();
	INT16 OnCmdLoad();
//}}CGEN_PMIC
#endif // #ifndef __NOITP

// Secondary method invocation functions
public:
//{{CGEN_SMIC
	INT16 CmdError();
	INT16 Label();
	INT16 CmdLoad(char* commandline);
//}}CGEN_SMIC

// Option changed callback functions
public:
//{{CGEN_OCCF
	INT16 OnNonstopSet();
	INT16 OnStopSet();
//}}CGEN_OCCF

// Field changed callback functions
public:
//{{CGEN_FCCF
	INT16 OnScriptdirChanged();
//}}CGEN_FCCF

// Scanned member functions
//{{CGEN_EXPORT

// Taken from 'itp_pars.cpp'
	public: void PreprocCommand(char* lpCmd, INT16 nLen, char** lpArgv, INT16 nArgc, const char* lpInfile, INT64 nInline, BOOL bUseForms = TRUE);
	protected: static INT16 ParseCommand(char* lpCmd, BOOL bForms);
	protected: static char* UnparseCommand(char* lpBuffer);
	public: static char* GetNextToken();
	public: char* GetNextTokenForce();
	public: static char* GetRemainingCmd();
	public: static void RefuseToken();

// Taken from 'itp_itp.cpp'
	public: INT16 StartSession(int argc, char** argv);
	public: INT16 TerminateSession();
	public: INT16 Run();
	public: INT16 SendCommand(CList<SStr>& list);
	public: INT16 SendCommand(char* lpText);
	public: INT16 PostCommand(const char* lpText);
	protected: INT16 PeekCommand(INT16 bNoPreproc = 0, INT16 bAutoQuit = 0);
	protected: INT16 PumpCommand();
	protected: INT16 ItpAsStatement(char* lpText);
	protected: INT16 ItpAsWord(char* lpText);
	protected: INT16 ItpAsNumber(char* lpText);
	protected: INT16 ItpAsString(char* lpText);
	protected: INT16 ItpAsFormula(char* lpText);
	protected: INT16 ItpAsLoadfile(char* lpText);
	public: INT16 ExecuteWord(SWord* lpWord);
	protected: INT16 IgnoreByCondition(char* lpText);
	protected: INT16 IsWordOrConstant(const char* lpText);
	protected: INT16 IsValidInstanceName(const char* lpText);
	protected: INT16 CopyFile(char* lpSrcFilename, char* lpDestFilename);
	protected: INT16 LoadFile(char* lpFileName, INT16 argc, char** argv, INT16 bDelArgv, INT16 bTemp);
	protected: INT16 DoneFile();
	protected: INT16 PreprocFile();
	protected: INT16 RegisterLabel(char* lpName);
	protected: SLABEL* FindLabel(SLFILE* f, const char* lpName);
	protected: INT64 GetCurrentLineID(SLFILE* f);
	protected: SWord* FindWordInHandlers(const char* lpText);
	public: INT16 RunsInteractive();
	public: INT64 SetSysMode(INT64 lMask, INT16 nValue);
	public: INT16 GetSysMode(INT64 lMask);
	protected: INT16 SetCmdHandler(INT16 nPriority, CDlpObject* lpInstance);
	protected: CDlpObject* GetTopmostCmdHandler();
	protected: INT16 PrintPrompt();
	protected: char* GetPrompt();
	protected: INT16 PrintVersionInfo();
	private: static BOOL IsExternalInstance(CDlpObject* iInst);

// Taken from 'itp_cmd.cpp'
	public: INT16 _See(const char* lpName, BOOL bInline);
	public: INT16 Help();
	public: INT16 Syntax();
	public: INT16 Explain();
	public: INT16 Root();
	public: INT16 Quit();
	public: INT16 Break(INT16 nId, char* lpText);
	public: INT16 Cont();
	public: INT16 Step();
	public: INT16 Load(const char* lpScriptName, const char* lpCmdLine);
	public: INT16 Leave();
	public: INT16 Not();
	public: INT16 If();
	public: INT16 Else();
	public: INT16 Endif();
	public: INT16 Goto();
	public: INT16 CmdSave(char* lpFilename);
	public: INT16 CmdRestore(char* lpFilename);
	public: INT16 CmdShow();
	public: INT16 CmdCopy(CDlpObject* iSrc);
	public: INT16 CmdDestroy();
	public: INT16 CmdReset();
	public: INT16 CmdSet();
	public: INT16 CmdGet();
	public: INT16 Internalize();
	public: INT16 See();
	public: INT16 SeeInline();
	public: INT16 Stack();
	public: INT16 Ps();
	public: INT16 Echo(char* lpText);
	public: INT16 List();
	public: INT16 System(char* lpCommand);
	public: INT16 Up();
	public: INT16 CmdNull();
	public: INT16 CmdTrue();
	public: INT16 CmdFalse();
	public: INT16 Type();
	public: INT16 Comment();
	public: INT16 Platform(char* lpPID);
	public: INT16 Cd(char* lpDir);
	public: INT16 Shell();
	public: INT16 Decrement();
	public: INT16 Increment();
	public: INT16 TopInstanceIsNull();
	public: INT16 IsInstance(char* sInstId, char* sClassId);

// Taken from 'itp_stk.cpp'
	public: void PushInstance(CDlpObject* lpInst);
	public: CDlpObject* PopInstance();
	public: CDlpObject* StackInstance(INT16 nPos);
	public: void PushNumber(COMPLEX64 dVal);
	public: COMPLEX64 PopNumber();
	public: COMPLEX64 StackNumber(INT16 nPos);
	public: void PushString(const char* lpString);
	public: char* PopString(char* lpBuffer, INT16 nBufferLen);
	public: char* StackString(INT16 nPos);
	public: void PushLogic(BOOL bVal);
	public: BOOL PopLogic();
	public: BOOL StackLogic(INT16 nPos);
	public: void PushCondition(INT16 nVal);
	public: INT16 PopCondition();
	public: INT16 StackCondition(INT16 nPos);
	public: void PushFile(SLFILE* lpFile);
	public: SLFILE* PopFile();
	public: SLFILE* StackFile(INT16 nPos);

// Taken from 'itp_mic.cpp'
	public: static BOOL StackLogic(CDlpObject* __this, INT16 nArg, INT16 nPos);
	public: static COMPLEX64 StackNumber(CDlpObject* __this, INT16 nArg, INT16 nPos);
	public: static CDlpObject* StackInstance(CDlpObject* __this, INT16 nArg, INT16 nPos);
	public: static char* StackString(CDlpObject* __this, INT16 nArg, INT16 nPos);
	public: static void PushLogic(CDlpObject* __this, BOOL bVal);
	public: static void PushNumber(CDlpObject* __this, COMPLEX64 nVal);
	public: static void PushInstance(CDlpObject* __this, CDlpObject* iVal);
	public: static void PushString(CDlpObject* __this, const char* lpsVal);
	public: static const char* NextToken(CDlpObject* __this, BOOL bSameLine);
	public: static const char* NextTokenDel(CDlpObject* __this);
	public: static void RefuseToken(CDlpObject* __this);
	public: static CItp* GetItp(CDlpObject* iInst);
//}}CGEN_EXPORT

// Member variables
public:

//{{CGEN_FIELDS
	INT32            m_nArgc;
	char**           m_lpArgv;
	char*            m_lpsCmd;
	CDlpObject*      m_iCmdHdlr[3];
	CList<SStr>      m_cmdQueue;
	INT16            m_nIfsWait;
	SLFILE*          m_lpInFile;
	SMic             m_mic;
	char             m_lpsScriptdir[255];
	INT16            m_nStackCond[10];
	INT16            m_nStackCondL;
	SLFILE*          m_lpStackFile[10];
	INT16            m_nStackFileL;
	CDlpObject*      m_iStackInst[10];
	INT16            m_nStackInstL;
	BOOL             m_bStackLogic[10];
	INT16            m_nStackLogicL;
	COMPLEX64        m_nStackNumber[10];
	INT16            m_nStackNumberL;
	char*            m_lpsStackString[10];
	INT16            m_nStackStringL;
	INT64            m_nSysMode;
	INT16            m_nVerbose;
//}}CGEN_FIELDS

//{{CGEN_OPTIONS
	BOOL m_bClose;
	BOOL m_bDisarm;
	BOOL m_bLocal;
	BOOL m_bNonstop;
	BOOL m_bStop;
	BOOL m_bXml;
//}}CGEN_OPTIONS
}

;

// Scanned C (member) functions
//{{CGEN_CEXPORT

// Taken from 'itp_pars.cpp'

// Taken from 'itp_itp.cpp'

// Taken from 'itp_cmd.cpp'

// Taken from 'itp_stk.cpp'

// Taken from 'itp_mic.cpp'
BOOL CItp_StackLogic(CDlpObject* __this, INT16 nArg, INT16 nPos);
COMPLEX64 CItp_StackNumber(CDlpObject* __this, INT16 nArg, INT16 nPos);
CDlpObject* CItp_StackInstance(CDlpObject* __this, INT16 nArg, INT16 nPos);
char* CItp_StackString(CDlpObject* __this, INT16 nArg, INT16 nPos);
void CItp_PushLogic(CDlpObject* __this, BOOL bVal);
void CItp_PushNumber(CDlpObject* __this, COMPLEX64 nVal);
void CItp_PushInstance(CDlpObject* __this, CDlpObject* iVal);
void CItp_PushString(CDlpObject* __this, const char* lpsVal);
const char* CItp_NextToken(CDlpObject* __this, BOOL bSameLine);
const char* CItp_NextTokenDel(CDlpObject* __this);
void CItp_RefuseToken(CDlpObject* __this);
//}}CGEN_CEXPORT

#endif //#ifndef __ITP_H


// EOF
