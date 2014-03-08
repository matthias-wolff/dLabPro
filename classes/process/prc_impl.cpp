// dLabPro class CProcess (process)
// - Additional C/C++ methods
//
// AUTHOR : Matthias Wolff
// PACKAGE: dLabPro/classes
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

#include "dlp_process.h"

#ifdef __LINUX
  #include <sys/wait.h>
  #define USE_FORK
#endif

static const char __sSlaveScript[][L_SSTR] =
{
  "#!/usr/bin/env dlabpro",                                                     // For dLabPro 2.5.3
  " ",                                                                          //
  "process ~iP;",                                                               // A process manager
  "object  ~iDto;",                                                             // The data transfer object
  "var     ~i;",                                                                // Loop counter
  " ",                                                                          //
  "\"$1\" ~iDto -restore;",                                                     // Restore data transfer object
  "\"~iDto."PRC_S_IDINCL"\" \"\" ?instance if",                                 // Included transferred >>
  "  :~i=0; :~i<~iDto."PRC_S_IDINCL".nrec: while",                              //   Loop over incl. table >>
  "    \"${~iDto."PRC_S_IDINCL"[~i,0]}\" include;",                             //     Include file
  "  end",                                                                      //   <<
  "end",                                                                        // <<
  "\"~iDto."PRC_S_IDGLOB"\" \"\" ?instance if",                                 // Globals transferred >>
  "  :~i=0; :~i<~iDto."PRC_S_IDGLOB".nrec: while",                              //   Loop over globals table >>
  "    \"${~iDto."PRC_S_IDGLOB"[~i,1]}\" \"\" ?instance not if",                //     No equally name inst. >>
  "      ${~iDto."PRC_S_IDGLOB"[~i,0]} ${~iDto."PRC_S_IDGLOB"[~i,1]};",         //       Create global instance
  "      \"${~iDto."PRC_S_IDGLOB"[~i,0]}\" \"function\" == if",                 //         It's a function >>
  "        /disarm ~iDto.${~iDto."PRC_S_IDGLOB"[~i,1]} "
          "/disarm ${~iDto."PRC_S_IDGLOB"[~i,1]} =;",                           //         Copy
// TODO: will cause a crash at session end -->
//  "        /disarm ~iDto.${~iDto."PRC_S_IDGLOB"[~i,1]} -destroy;",              //         Save memory
// <--
  "       else",                                                                //       << It's not a fnc >>
  "        ~iDto.${~iDto."PRC_S_IDGLOB"[~i,1]} ${~iDto."PRC_S_IDGLOB"[~i,1]} =;",//         Copy
  "       end",                                                                 //       <<
  "    end",                                                                    //     <<
  "    ~i ++=;",                                                                //     Next global
  "  end",                                                                      //   <<
  "end",                                                                        // <<
  ":~i=~iDto."PRC_S_IDSIGN".nrec-1; :~i>0: while",                              // Loop over signature table >>
  "  ~iDto.${~iDto."PRC_S_IDSIGN"[~i,0]}",                                      //   Push argument
  "  ~i --=",                                                                   //   Next argument
  "end",                                                                        // <<
  "~iDto.${~iDto."PRC_S_IDSIGN"[0,0]} ~iDto ~iP -marshal_retval;",              // Call function
  "\"$1\" ~iDto /xml /zip -save;",                                              // Save data transfer object
  " ",                                                                          //
  "0 return;",                                                                  // Everything ok
  "\0"                                                                          // End of script
};

// == Thread functions ==                                                       // ====================================

/**
 * Thread function executing a job.
 *
 * @param pvJob void pointer to an <code>SJob</code> struct
 * @return <code>NULL</code>
 */
void* CGEN_EXPORT DoJob(void* pvProcess)
{
  CProcess* pProcess;                                                           // This pointer

  // Validate and initialize                                                    // ------------------------------------
  if (!(pProcess=(CProcess*)pvProcess) ) return NULL;                           // No process, no service!
  if (pProcess->m_nState & PRC_COMPLETE) return NULL;                           // Don't do it over

  // Execute job                                                                // ------------------------------------
  pProcess->m_nState |= PRC_RUNNING;                                            // Set running flag
  pProcess->m_nRetVal = dlp_system(pProcess->m_psCmdLine);                      // Execute process (blocking)
  pProcess->m_nState &= ~PRC_RUNNING;                                           // Clear running flag
  pProcess->m_nState |= PRC_COMPLETE;                                           // Set completed flag

  return NULL;                                                                  // Anyway ...
}

INT32 CGEN_PRIVATE CProcess::DoJobFork(CFunction *iCaller, CFunction *iFnc)
{
  CData*      idSign = NULL;                                                    // Signature table in iDto
  CDlpObject* iArg   = NULL;                                                    // Argument Object
  INT32       nArg   = -1;                                                      // Argument loop counter
  char        sArg[L_NAMES];                                                    // Current argument name
  char        lpsCmd[256];                                                      // String buffer

  // Initialize                                                                 // ------------------------------------
  idSign = (CData*)CDlpObject_OfKind("data",                                    // Get signature table
    CDlpObject_FindInstance(m_iDto,PRC_S_IDSIGN));                              // |

  // Push arguments on Stack                                                    // ------------------------------------
  for (nArg=CData_GetNRecs(idSign)-1; nArg>0; nArg--)                           // Loop over signature table
  {                                                                             // >>
    dlp_strcpy(sArg,CData_Sfetch(idSign,nArg,0));                               //   Get argument name
    iArg = CDlpObject_FindInstance(m_iDto,sArg);                                //   Find argument in iDto
    if (CDlpObject_OfKind("var",iArg)) switch(AS(CVar,iArg)->m_nType){          //   Primitve data types >>
      case T_BOOL   : iCaller->PushLogic (AS(CVar,iArg)->m_bBVal); break;       //     Push bool
      case T_COMPLEX: iCaller->PushNumber(AS(CVar,iArg)->m_nNVal); break;       //     Push number
      case T_STRING : iCaller->PushString(AS(CVar,iArg)->m_lpsSVal); break;     //     Push string
      default: iCaller->PushNumber(CMPLX(0.)); break;                           //     Default: push something
    }else iCaller->PushInstance(iArg);                                          //   Push instances
  }                                                                             // <<

  // Post commands in queue                                                     // ------------------------------------
  iCaller->PostCommand(CData_Sfetch(idSign,0,0),NULL,-1,FALSE);                 // Put in queue: run the function
  snprintf(lpsCmd,255,"%s.dto %s -marshal_retval;",                             // Put in queue: save return value
      this->m_lpInstanceName,this->m_lpInstanceName);                           // |
  iCaller->PostCommand(lpsCmd,NULL,-1,FALSE);                                   // |
  snprintf(lpsCmd,255,"\"%s.xml\" %s.dto /xml /zip -save;",                     // Put in queue: save data transfer object
      m_psTmpFile,this->m_lpInstanceName);                                      // |
  iCaller->PostCommand(lpsCmd,NULL,-1,FALSE);                                   // |
  iCaller->PostCommand("quit;",NULL,-1,FALSE);                                  // Put in queue: Quit the client process
  return O_K;                                                                   // Ok, run the queue
}

// == Argument marshaling functions ==                                          // ====================================

/**
 * Packs a stack item into the data transfer object <code>iDto</code>. If the
 * stack item is an instance, the function will create a copy in
 * <code>iDto</code>. If the stack item is a boolean, numeric or string value,
 * the function will create a variable holding that value in <code>iDto</code>.
 *
 * @param iDto
 *          the data transfer object (must not be <code>NULL</code>)
 * @param idSign
 *          the signature table (may be <code>NULL</code>, if not
 *          <code>NULL</code> the item will be added to the signature)
 * @param pStkItm
 *          the stack item to pack (see documentation of <code>CFunction</code>)
 * @param psName
 *          the object name (if <code>NULL</code> or empty, a default name will
 *          be used   )
 */
void CGEN_PROTECTED CProcess::Pack
(
  CDlpObject* iDto,
  CData*      idSign,
  StkItm*     pStkItm,
  const char* psName
)
{
  CDlpObject* iDst = 0;                                                         // Packed instance
  INT32        nArg = -1;                                                        // Argument index in signature
  char        sBuf[L_NAMES];                                                    // Packed instance name buffer

  // Validate and initialize                                                    // ------------------------------------
  if (!iDto   ) return;                                                         // Need data transfer object!
  if (!pStkItm) return;                                                         // Need stack item!
  dlp_strcpy(sBuf,psName);                                                      // Copy custom instance name
  if (idSign) nArg = CData_AddRecs(idSign,1,10);                                // Add element to signature

  // Pack                                                                       // ------------------------------------
  if (pStkItm->nType==T_INSTANCE && pStkItm->val.i!=NULL)                       // Pack an instance
  {                                                                             // >>
    CDlpObject* iSrc = pStkItm->val.i;                                          //   Get pointer to instance
    if (dlp_strlen(sBuf)==0) dlp_strcpy(sBuf,iSrc->m_lpInstanceName);           //   No custom name -> use inst. name
    iDst = CDlpObject_Instantiate(iDto,iSrc->m_lpClassName,sBuf,FALSE);         //   Created packed instance
    iDst->Copy(iSrc);                                                           //   Copy content
  }                                                                             // <<
  else                                                                          // Pack a boolean, number or string
  {                                                                             // >>
    if (dlp_strlen(sBuf)==0) sprintf(sBuf,"arg%ld",(long)nArg);                 //   No custom name -> make a name
    iDst = CDlpObject_Instantiate(iDto,"var",sBuf,FALSE);                       //   Create a variable to pack into
    switch (pStkItm->nType)                                                     //   Branch for variable type
    {                                                                           //   |
    case T_BOOL   : ((CVar*)iDst)->Bset(pStkItm->val.b);   break;               //   Pack a boolean
    case T_COMPLEX: ((CVar*)iDst)->Vset(pStkItm->val.n);   break;               //   Pack a number
    case T_STRING : ((CVar*)iDst)->Sset(pStkItm->val.s);   break;               //   Pack a string
    }                                                                           //   |
  }                                                                             // <<
  if (idSign) CData_Sstore(idSign,sBuf,nArg,0);                                 // Write signature
}

/**
 * Packs <code>nArgs<code> function arguments plus the zero-th argument (the
 * function itself) from <code>iCaller</code>'s stack into the data transfer
 * object <code>iDto</code>.
 */
void CGEN_PROTECTED CProcess::Marshal
(
  CDlpObject* iDto,
  CFunction*  iCaller,
  INT32       nArgs
)
{
  CData*      idSign  = NULL;                                                   // Signature table
  StkItm*     pStkItm = NULL;                                                   // Stack item to marshal
  INT32       nArg    = 0;                                                      // Argument loop counter
#ifndef USE_FORK
  CDlpObject* iRoot   = NULL;
  CDlpObject* iSrc    = NULL;
  CDlpObject* iDst    = NULL;
  CData*      idGlob  = NULL;                                                   // Globals table
  CData*      idIncl  = NULL;                                                   // Includes table
  hscan_t     hs;
  hnode_t*    hn;
  SWord*      lpWord  = NULL;
  char*       psClsName;
  char*       psInsName;
#endif

  // Marshal function arguments                                                 // ------------------------------------
  idSign = (CData*)CDlpObject_Instantiate(iDto,"data",PRC_S_IDSIGN,FALSE);      // Create signature table
  CData_AddComp(idSign,"name",L_SSTR);                                          // Add component to table
  for (nArg=0; nArg<=nArgs; nArg++)                                             // Loop over arguments
  {                                                                             // >>
    pStkItm = iCaller->StackGet(0);                                             //   Get stack top
    Pack(iDto,idSign,pStkItm,NULL);                                             //   Pack into data transfer object
    iCaller->Pop(FALSE);                                                        //   Remove stack top
  }                                                                             // <<

#ifndef USE_FORK
  // Marshal global variables                                                   // ------------------------------------
  iRoot = GetRoot();                                                            // Get root function
  if (m_bGlobal && iRoot)                                                       // /global option set
  {                                                                             // >>
    idGlob = (CData*)CDlpObject_Instantiate(iDto,"data",PRC_S_IDGLOB,FALSE);    //   Create globals table
    CData_AddComp(idGlob,"clas",L_SSTR);                                        //   Add component to table
    CData_AddComp(idGlob,"name",L_SSTR);                                        //   Add component to table
    hash_scan_begin(&hs,iRoot->m_lpDictionary);                                 //   Initialize
    while ((hn = hash_scan_next(&hs))!=NULL)                                    //   Loop over root function's dict.
    {                                                                           //   >>
      lpWord = (SWord*)hnode_get(hn);                                           //     Get pointer to SWord struct
      if ((lpWord->nWordType == WL_TYPE_INSTANCE) && lpWord->lpData)            //     Entry is a non-NULL instance
      {                                                                         //     >>
        iSrc = (CDlpObject*)lpWord->lpData;                                     //       Get instance pointer
        psClsName = iSrc->m_lpClassName;                                        //       Get pointer to class name
        psInsName = iSrc->m_lpInstanceName;                                     //       Get pointer to instance name
        //if (CDlpObject_OfKind("function",iSrc)) continue;                     //       No functions, please
        if (iSrc==iCaller) continue;                                            //       Not the caller!
        if (iSrc->m_nClStyle & CS_SINGLETON) continue;                          //       No singletons!
        if (CDlpObject_FindInstanceWord(m_iDto,psInsName,NULL)) continue;       //       Shadowed by an argument
        iDst = CDlpObject_Instantiate(m_iDto,psClsName,psInsName,FALSE);        //       Create copy to pack
        iDst->Copy(iSrc);                                                       //       Copy content
        CData_AddRecs(idGlob,1,10);                                             //       Add index entry
        CData_Sstore(idGlob,psClsName,CData_GetNRecs(idGlob)-1,0);              //       Write index entry
        CData_Sstore(idGlob,psInsName,CData_GetNRecs(idGlob)-1,1);              //       Write index entry
      }                                                                         //     <<
    }                                                                           //   <<
  }                                                                             // <<

  // Marshal includes                                                           // ------------------------------------
  if (m_bInclude && iRoot)                                                      // /include option set
  {                                                                             // >>
    idIncl = (CData*)CDlpObject_Instantiate(iDto,"data",PRC_S_IDINCL,FALSE);    //   Create include table
    CData_Copy(idIncl,((CFunction*)iRoot)->m_idSfl);                            //   Copy includes
  }                                                                             // <<
#endif
}

/**
 * Unpacks the data transfer object <code>iDto</code> into <code>iCaller</code>.
 */
void CGEN_PROTECTED CProcess::Unmarshal(CDlpObject* iDto, CFunction* iCaller)
{
  CData*      idSign = NULL;                                                    // Signature table in iDto
  CDlpObject* iSrc   = NULL;                                                    // Source object
  CDlpObject* iDst   = NULL;                                                    // Destination object
  INT32        nArg   = -1;                                                      // Argument loop counter
  char        sArg[L_NAMES];                                                    // Current argument name

  // Initialize                                                                 // ------------------------------------
  idSign = (CData*)CDlpObject_OfKind("data",                                    // Get signature table
    CDlpObject_FindInstance(iDto,PRC_S_IDSIGN));                                // |

  // Unmarshal arguments                                                        // ------------------------------------
  for (nArg=1; nArg<CData_GetNRecs(idSign); nArg++)                             // Loop over signature table
  {                                                                             // >>
    dlp_strcpy(sArg,CData_Sfetch(idSign,nArg,0));                               //   Get argument name
    iSrc = CDlpObject_FindInstance(iDto,sArg);                                  //   Find argument in iDto
    if (CDlpObject_OfKind("var",iSrc)) continue;                                //   Ignore primitve data types
    iDst = CDlpObject_FindInstance(iCaller,sArg);                               //   Find argument in iCaller
    if (!iDst) continue;                                                        //   Not there -> nothing to do
    iDst->Copy(iSrc);                                                           //   Copy unmarshaled arg. to caller
  }                                                                             // <<

  // Unmarshal return value                                                     // ------------------------------------
  iSrc = CDlpObject_FindInstance(iDto,PRC_S_RETV);                              // Find return value in iDto
  if (CDlpObject_OfKind("var",iSrc))                                            // Return value is a primitive
    switch (((CVar*)iSrc)->m_nType)                                             //   Branch for type
    {                                                                           //   |
    case T_BOOL   : iCaller->PushLogic (((CVar*)iSrc)->m_bBVal  ); break;       //   Push a boolean
    case T_DOUBLE :                                                             //   Push a number
    case T_COMPLEX: iCaller->PushNumber(((CVar*)iSrc)->m_nNVal  ); break;       //   Push a complex number
    case T_STRING : iCaller->PushString(((CVar*)iSrc)->m_lpsSVal); break;       //   Push a string
    }                                                                           //   |
  else if (iSrc)                                                                // Return value is an instance
    iCaller->PushInstance(iSrc);                                                //   Push it
}

/**
 * Writes the data transfer file of this process. This method is called
 * immediately before starting the operating system process.
 */
INT16 CGEN_PUBLIC CProcess::SendData()
{
  DLP_FILE* pfScr;                                                              // Slave script file pointer
  INT32     i;                                                                  // Loop counter
  char      sScrFn[L_PATH];                                                     // Slave script file name
  char      sDtoFn[L_PATH];                                                     // Data transfer file name

  if (m_nState & PRC_DATASENT) return NOT_EXEC;                                 // Data have already been sent
  if (m_iDto)                                                                   // This process is a function call
  {                                                                             // >>
    sprintf(sScrFn,"%s.xtp",m_psTmpFile);                                       //   Get slave script file name
    sprintf(sDtoFn,"%s.xml",m_psTmpFile);                                       //   Get data transfer fine name
    pfScr = dlp_fopen(sScrFn,"w");                                              //   Open temporary script file
    for (i=0; dlp_strcmp(__sSlaveScript[i],"\0")!=0; i++)                       //   Loop over slave script lines
    {                                                                           //   >>
      dlp_fwrite(__sSlaveScript[i],1,dlp_strlen(__sSlaveScript[i]),pfScr);      //     Write a line
      dlp_fwrite("\n",1,1,pfScr);                                               //     Write a line break
    }                                                                           //   <<
    dlp_fclose(pfScr);                                                          //   Close temporary script file
    CDlpObject_Save(m_iDto,sDtoFn,SV_XML|SV_ZIP);                               //   Save data transfer object
    sprintf(m_psCmdLine,"%s %s %s",dlp_get_binary_path(),sScrFn,sDtoFn);        //   Change the command line
    // -- ???? -->
    IDESTROY(m_iDto); IFIELD_RESET(CDlpObject,"dto");                           //   Save memory!
    // <----------
  }                                                                             // <<
  m_nState |= PRC_DATASENT;                                                     // Remember data have been sent
  return O_K;
}

/**
 * Reads the data transfer file of this process. This method is called after the
 * operating system process has completed.
 */
void CGEN_PROTECTED CProcess::ReceiveData()
{
  char sScrFn[L_PATH];                                                          // Slave script file name
  char sDtoFn[L_PATH];                                                          // Data transfer file name

  if (!(m_nState & PRC_COMPLETE))  return;                                      // Process not complete
  if (m_nState & PRC_DATARECEIVED) return;                                      // Data have already been received
  if (m_iDto)                                                                   // This process is a function call
  {                                                                             // >>
    sprintf(sScrFn,"%s.xtp",m_psTmpFile);                                       //   Get slave script file name
    sprintf(sDtoFn,"%s.xml",m_psTmpFile);                                       //   Get data transfer fine name
    CDlpObject_Restore(m_iDto,sDtoFn,SV_XML|SV_ZIP);                            //   Restore data transfer object
    remove(sDtoFn);                                                             //   Remove temporary file
    remove(sScrFn);                                                             //   Remove temporary file
  }                                                                             // <<
  m_nState |= PRC_DATARECEIVED;                                                 // Remember data have been received
}

// == Secondary method invocation functions ==                                  // ====================================

/*
 * Manual page at process.def
 */
INT32 CGEN_PUBLIC CProcess::Start()
{
  const SMic* pMic    = NULL;                                                   // Method invocation context of Start()
  CFunction*  iCaller = NULL;                                                   // Function calling Start()
  CFunction*  iFnc    = NULL;                                                   // Process function
  StkItm*     pStkItm = NULL;                                                   // Stack item
  INT32       nArgs   = 0;                                                      // Number of process function arguments

  // Validate and initialize                                                    // ------------------------------------
  if (m_nState!=0)                                                              // Not virginal
    return IERROR(this,PRC_CANTSTART,"multiple starts not allowed",0,0);        //   Forget it!
  if (!(pMic = CDlpObject_MicGet(_this))) return -1;                            // Get method invocation context
  iCaller = (CFunction*)CDlpObject_OfKind("function",pMic->iCaller);            // Get calling CFunction
  if (!iCaller) return -1;                                                      // Must be a function!

  // Initialize process                                                         // ------------------------------------
  sprintf(m_psTmpFile,"%s%ld",dlp_tempnam(NULL,"~dLabPro#process#"),(long)dlp_time());// Initialize temp. file name prefix

  // Marshal arguments                                                          // ------------------------------------
  if (!(pStkItm=iCaller->StackGet(0))) return IERROR(this,PRC_TOOFEWARGS,0,0,0);// Get stack top
  if (pStkItm->nType==T_INSTANCE)                                               // Stack top is an instance
    iFnc = (CFunction*)CDlpObject_OfKind("function",pStkItm->val.i);            //   Get function to be called
  if (iFnc)                                                                     // This process is a function call
  {                                                                             // >>
    IFIELD_RESET(CDlpObject,"dto");                                             //   Create data transfer object
    nArgs = CData_GetNRecs(iFnc->m_idArg);                                      //   Get number of function arguments
    Marshal(m_iDto,iCaller,nArgs);                                              //   Marshal arguments for transfer
  }                                                                             // <<
  else                                                                          // This process is a program call
    dlp_strcpy(m_psCmdLine,iCaller->PopString(0));                              //   Get program command line

#ifdef USE_FORK
  if (iFnc)                                                                     // This process is a function call
  {                                                                             // >>
    m_hPid=fork();                                                              //   Fork the process
    if(m_hPid>0){                                                               //   Parent process >>
      m_nState |= PRC_DATASENT;                                                 //     Remember data have been sent
      m_nState |= PRC_RUNNING;                                                  //     Set running flag
      m_hThread = 0;                                                            //     Clear thread handle
      return O_K;                                                               //     Everything is fine
    }                                                                           //   <<
    if(m_hPid==0) return DoJobFork(iCaller,iFnc);                               //   The child process runs the function
    return IERROR(this,PRC_CANTSTART,"fork() failed",0,0);                      //   On error (fid<0) we return
  }                                                                             // <<
#endif
  // Start job in watcher thread                                                // ------------------------------------
  m_hPid = 0;                                                                   // Reset process id
  SendData();                                                                   // Send transfer data
  m_hThread = dlp_create_thread(DoJob,this);                                    // Do the job and watch it

  return O_K;                                                                   // Yo!
}

/*
 * Manual page at process.def
 */
INT16 CGEN_PUBLIC CProcess::Wait()
{
  const SMic* pMic    = NULL;                                                   // Method invocation context of Wait()
  CFunction*  iCaller = NULL;                                                   // Function instance calling Wait()

  // Validate and initialize                                                    // ------------------------------------
  if (!((m_nState & PRC_RUNNING)|PRC_COMPLETE))                                 // Not running
    return IERROR(this,PRC_CANTWAIT,"not started",0,0);                         //   Forget it
  if (!(pMic = CDlpObject_MicGet(_this))) return -1;                            // Get method invocation context
  iCaller = (CFunction*)CDlpObject_OfKind("function",pMic->iCaller);            // Get calling CFunction
  if (!iCaller) return -1;                                                      // Must be a function!

  // Wait for job to be completed                                               // ------------------------------------
#ifdef USE_FORK
  if(!m_hThread && m_hPid){                                                     // If there is a child process by fork >>
    waitpid(m_hPid,NULL,0);                                                     //   Wait for child process
    m_nState &= ~PRC_RUNNING;                                                   //   Clear running flag
    m_nState |= PRC_COMPLETE;                                                   //   Set completed flag
  }else                                                                         // <<
#endif
  dlp_join_thread(m_hThread);                                                   // Wait for the watcher thread to end
  ReceiveData();                                                                // Receive transfer data

  // Aftermath                                                                  // ------------------------------------
  if (m_iDto)                                                                   // This job was a function call
    Unmarshal(m_iDto,iCaller);                                                  //   Unmarshal transfer data to caller
  else                                                                          // This job was a program call
    MIC_PUT_N(m_nRetVal);                                                       //   Push process return value

  return O_K;                                                                   // Ok
}

/*
 * Manual page at process.def
 */
INT16 CGEN_PUBLIC CProcess::Status()
{
  CData* d;
  char   s[L_INPUTLINE];
  INT32   i;

  printf("\n"); dlp_fprint_x_line(stdout,'-',dlp_maxprintcols());               // Protocol
  printf("\n   Status of instance");                                            // Protocol
  printf("\n   process %s",BASEINST(_this)->m_lpInstanceName);                  // Protocol
  printf("\n"); dlp_fprint_x_line(stdout,'-',dlp_maxprintcols());               // Protocol
  printf("\n   State       : 0x%04X",m_nState);                                 // Protocol
  if (m_nState!=0)                                                              // In any but the maiden state
  {                                                                             // >>
    BOOL b = 0;                                                                 //   Comma flag
    printf(" (");                                                               //   Protocol
    if (m_nState&PRC_DATASENT    ) { if(b) printf(", "); printf("data sent"    ); b=1; }//...
    if (m_nState&PRC_RUNNING     ) { if(b) printf(", "); printf("running"      ); b=1; }//...
    if (m_nState&PRC_COMPLETE    ) { if(b) printf(", "); printf("complete"     ); b=1; }//...
    if (m_nState&PRC_KILLED      ) { if(b) printf(", "); printf("killed"       ); b=1; }//...
    if (m_nState&PRC_DATARECEIVED) { if(b) printf(", "); printf("data received"); b=1; }//...
    printf(")");                                                                //   Protocol
  }                                                                             // <<
  printf("\n   Return value: %ld",(long)m_nRetVal  );                           // Protocol
  if (dlp_strlen(m_psTmpFile)) printf("\n   Temp. files : %s*",m_psTmpFile);    // Protocol
  dlp_strcpy(s,m_psCmdLine); dlp_strreplace(s,m_psTmpFile,"<tmpfile>");         // Abbreviate command line
  if (dlp_strlen(m_psCmdLine)) printf("\n   Command line: %s" ,s);              // Protocol

  // Show transferred data                                                      // ------------------------------------
  if (m_iDto)                                                                   // Have data transfer object
  {                                                                             // >>
    printf("\n"); dlp_fprint_x_line(stdout,'-',dlp_maxprintcols());             //   Protocol
    printf("\n   Transferred data");                                            //   Protocol
    d = (CData*)CDlpObject_FindInstanceWord(m_iDto,PRC_S_IDSIGN,NULL);          //   Get signature table
    printf("\n   - function  : %s",(char*)CData_XAddr(d,0,0));                  //   Protocol (job function name)
    for (i=1; i<CData_GetNRecs(d); i++)                                         //   Loop over function arguemnts
      printf("\n   %13s %s",i==1?"- arguments :":"",(char*)CData_XAddr(d,i,0)); //     Protocol (job function arg.)
    d = (CData*)CDlpObject_FindInstanceWord(m_iDto,PRC_S_IDGLOB,NULL);          //   Get list of global instances
    if (d)                                                                      //   Have one
      for (i=0; i<CData_GetNRecs(d); i++)                                       //     Loop over entries
        printf("\n   %13s %-8s %s",i==0?"- globals   :":"",                     //       Protocol (global instance)
          (char*)CData_XAddr(d,i,0),(char*)CData_XAddr(d,i,1));                 //       |
    printf("\n"); dlp_fprint_x_line(stdout,'-',dlp_maxprintcols());             //   Protocol
    printf("\n   Transferred program");                                         //   Protocol
    for (i=0; dlp_strlen(__sSlaveScript[i]); i++)                               //   Loop over slave script lines
    {                                                                           //   >>
      dlp_strcpy(s,__sSlaveScript[i]);                                          //     Get a line
      if (strstr(s,"##"))*(strstr(s,"##"))='\0';                                //     Truncate at comment
      printf("\n     (%02ld) %s",i,dlp_strtrimright(s));                        //     Protocol (script line)
    }                                                                           //   <<
  }                                                                             // <<

  printf("\n"); dlp_fprint_x_line(stdout,'-',dlp_maxprintcols()); printf("\n"); // Protocol
  return O_K;                                                                   // All done
}

/*
 * Manual page at process.def
 */
INT16 CGEN_PUBLIC CProcess::MarshalRetval()
{
  CDlpObject* iDto    = NULL;                                                   // Data transfer object
  const SMic* pMic    = NULL;                                                   // Current method invocation context
  CFunction*  iCaller = NULL;                                                   // Calling function
  StkItm*     pStkItm = NULL;                                                   // Stack item to marshal

  // Validate and initialize                                                    // ------------------------------------
  if (!(pMic = CDlpObject_MicGet(_this))) return -1;                            // Get method invocation context
  iCaller = (CFunction*)CDlpObject_OfKind("function",pMic->iCaller);            // Get calling CFunction
  if (!iCaller) return -1;                                                      // Must be a function!

  // Pack second stack element into the CDlpObject on the stack top             // ------------------------------------
  iDto = iCaller->PopInstance(0);                                               // Get instance on stack top
  if (!iDto) return NOT_EXEC;                                                   // No instance on stack top --> puuh!
  if (iCaller->StackGetLength()==0) return O_K;                                 // Nothing more on the stack --> ok
  pStkItm = iCaller->StackGet(0);                                               // Get stack top
  Pack(iDto,NULL,pStkItm,PRC_S_RETV);                                           // Pack it as "~iRetv"
  iCaller->Pop(0);                                                              // Remove stack top

  return O_K;                                                                   // Ok
}

// EOF
