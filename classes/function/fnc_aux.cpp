// dLabPro class CFunction (function)
// - Auxilary methods
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

#include "dlp_function.h"
#include "dlp_math.h"
static char __lpsFqid[255] = "";
static CFunction* __iRootFnc = NULL;

/**
 * Sets the static pointer to the (one and only) root function (i.e. the one
 * which was instantiated first).
 *
 * @param iFnc
 *          Pointer to the root function
 *
 * <h4>Remarks</h4>
 * <ul>
 *   <li>Do not mistake this for the (logical) root of the instance tree
 *   returned by <a class="code" href="dlpobject.html">CDlpObject::GetRoot</a>!
 * </ul>
 */
void CGEN_SPROTECTED CFunction::SetRootFnc(CFunction* iFnc)
{
  __iRootFnc = iFnc;
}

/**
 * Returnd the static pointer to the (one and only) root function (i.e. the one
 * which was instanciated first).
 *
 * @return The pointer to the root function
 *
 * <h4>Remarks</h4>
 * <ul>
 *   <li>Do not mistake this for the (logical) root of the instance tree
 *   returned by <a class="code" href="dlpobject.html">CDlpObject::GetRoot</a>!
 * </ul>
 */
CFunction* CGEN_SPUBLIC CFunction::GetRootFnc()
{
  return __iRootFnc;
}

/**
 * Returns the one and only dLabPro parser instance (located in field "par" of
 * the root function.
 *
 * @return A pointer to the dLabPro parser
 */
CDgen* CGEN_PROTECTED CFunction::GetDlpParser()
{
	if (!GetRootFnc()) return NULL;
	return GetRootFnc()->m__iPar;
}

/**
 * Prints the program source code in the vicinity of the program pointer.
 *
 * @param nEnv
 *          Lines to print before and after the current line
 * @param sPrefix
 *          Prefix string for lines of source code (for indentation)
 */
void CGEN_PROTECTED CFunction::PrintCode
(
  INT16       nEnv,
  const char* sPrefix DEFAULT(NULL)
)
{
  INT32 nT      = 0;
  INT32 nXT     = CData_GetNRecs(m_idTsq);
  INT32 nPp     = m_nPp-1;
  INT32 nLinePp = -1;
  INT32 nLine   = -1;
  BOOL bBefore = FALSE;
  char lpsBuf1[255];
  char lpsBuf2[255];

  if (nPp<0   ) { nPp = 0; bBefore=TRUE; }
  if (nPp>=nXT) nPp = nXT-1;
  if (nPp<0||nPp>=nXT) return;

  nLinePp = __LINE(nPp);
  for (nT=nPp; nT>0 && __LINE(nT-1)>=nLinePp-nEnv; nT--) { }
  for (nLine=-1; nT<nXT && __LINE(nT)<=nLinePp+nEnv; nT++)
  {
    if (__TTYP(nT)[0]=='c') continue;
    if (__LINE(nT)>nLine)
    {
      dlp_splitpath(GetSrcFile(nT),NULL,lpsBuf1);
      nLine=__LINE(nT);
      printf("\n%s",sPrefix?sPrefix:"");
      printf("(%s:%ld) %c ",lpsBuf1,(long)nLine,nLine==nLinePp?'>':' ');
    }
    dlp_strcpy(lpsBuf1,__TOK (nT)); dlp_strreplace(lpsBuf1,"\n","");
    dlp_strcpy(lpsBuf2,__IDEL(nT)); dlp_strreplace(lpsBuf2,"\n","");
    if (nT==nPp)
    {
      if (bBefore) printf("-[]-%s%s",lpsBuf1,lpsBuf2);
      else         printf("-[%s]-%s",lpsBuf1,lpsBuf2);
    }
    else
      printf("%s%s",lpsBuf1,lpsBuf2);
  }
  printf("\n");
}

/**
 * Prints the function's argument list and further information at stdout.
 */
void CGEN_PROTECTED CFunction::PrintInfo(const char* lpTitle)
{
  DLPASSERT(m_idArg);
  DLPASSERT(m_idSfl);
  INT32 i = 0;

  printf("\n"); dlp_fprint_x_line(stdout,'-',dlp_maxprintcols());
  printf("\n   %s of instance",lpTitle?lpTitle:"Status");
  printf("\n   function %s%s",m_lpInstanceName,m_nXm&XM_INLINE?" (inline)":"");
  printf("\n"); dlp_fprint_x_line(stdout,'-',dlp_maxprintcols());
  if (m_idArg->GetNRecs())
    for (i=0; i<m_idArg->GetNRecs(); i++)
    {
      printf("\n   Arg %02ld         : %-16s",i,(char*)m_idArg->XAddr(i,FNC_ALIC_ID));
      if (m_idArg->Dfetch(i,FNC_ALIC_TYPE)==T_STRING)
      {
        char* lpsArg = *(char**)m_idArg->Pfetch(i,FNC_ALIC_PTR);
        printf("= \"%s\"",lpsArg?lpsArg:"");
      }
    }
  else printf("\n   (no arguments)");
  if (m_idSfl->GetNRecs())
    for (i=0; i<m_idSfl->GetNRecs(); i++)
      printf("\n   %s %s",i==0?"Source         :":"                ",
        (char*)CData_XAddr(m_idSfl,i,0));
  printf("\n   Program pointer: %ld",(long)m_nPp);
  printf("\n   Execution mode : %ld",(long)m_nXm);
  printf("\n"); dlp_fprint_x_line(stdout,'-',dlp_maxprintcols());
}

/*
 * Manual page at function.def
 */
INT16 CGEN_PUBLIC CFunction::Status()
{
  PrintInfo("Status");
  StackPrint();
  printf("\n                 ACTIVE 00: %s",m_iAi ?m_iAi ->m_lpInstanceName:"-");
  printf("\n                        01: %s",m_iAi2?m_iAi2->m_lpInstanceName:"-");
  printf("\n                        02: %s",_this->m_lpInstanceName);
  printf("\n"); dlp_fprint_x_line(stdout,'-',dlp_maxprintcols());
  printf("\n   For more information type -list help;\n");
  return O_K;
}

/**
 * Dumps the functions's call stack.
 */
INT16 CGEN_PUBLIC CFunction::PrintStackTrace()
{
  for(CFunction* f = this; f; f = f->GetCaller())
    printf("\nat %s(%ld)",f->GetSrcFile(f->m_nPp),__LINE_EX(f->m_idTsq,f->m_nPp));
  printf("\n");
  return O_K;
}

/**
 * Prints detailled description of a dLabPro identifier.
 *
 * @param lpsIdentifier
 *          Pointer to dLabPro identifier string
 * @return <code>O_K</code> if successfull, a (negative) error code otherwise.
 */
INT16 CGEN_PUBLIC CFunction::Explain(const char* lpsIdentifier)
{
  if (!dlp_strlen(lpsIdentifier))
  {
    CDlpObject* iAi = GetActiveInstance();
    if (iAi) iAi->PrintDictionary(PWL_SYNTAX);
    printf("  Use <-explain identifier> to get more detailled information.\n");
    return O_K;
  }

  // First try as a word
  SWord* lpWord = FindWordAi(lpsIdentifier);
  if (lpWord && lpWord->lpContainer)
  {
    switch(lpWord->nWordType)
    {
    case WL_TYPE_METHOD:
      printf("\n"); dlp_fprint_x_line(stdout,'-',dlp_maxprintcols());
      printf("\n   Explanation of method\n   %s %s",
        ((CDlpObject*)lpWord->lpContainer)->m_lpClassName,lpsIdentifier);
      printf("\n"); dlp_fprint_x_line(stdout,'-',dlp_maxprintcols());
      printf("\n\n   Syntax: %s %s %s\n\n   %s\n",
        lpWord->ex.mth.lpSyntax,lpsIdentifier,lpWord->ex.mth.lpPostsyn,
        lpWord->lpComment);
      printf("\n"); dlp_fprint_x_line(stdout,'-',dlp_maxprintcols()); printf("\n");
      break;
    case WL_TYPE_OPTION:
      printf("\n"); dlp_fprint_x_line(stdout,'-',dlp_maxprintcols());
      printf("\n   Explanation of option\n   %s %s",
        ((CDlpObject*)lpWord->lpContainer)->m_lpClassName,lpsIdentifier);
      printf("\n"); dlp_fprint_x_line(stdout,'-',dlp_maxprintcols());
      printf("\n\n   %s\n",lpWord->lpComment);
      printf("\n"); dlp_fprint_x_line(stdout,'-',dlp_maxprintcols()); printf("\n");
      break;
    case WL_TYPE_FIELD:
      printf("\n"); dlp_fprint_x_line(stdout,'-',dlp_maxprintcols());
      printf("\n   Explanation of field\n   %s %s",
        ((CDlpObject*)lpWord->lpContainer)->m_lpClassName,lpsIdentifier);
      printf("\n"); dlp_fprint_x_line(stdout,'-',dlp_maxprintcols());
      printf("\n\n   Type: %s",dlp_get_type_name(lpWord->ex.fld.nType));
      printf("\n\n   %s\n",lpWord->lpComment);
      printf("\n"); dlp_fprint_x_line(stdout,'-',dlp_maxprintcols()); printf("\n");
      break;
    case WL_TYPE_INSTANCE:
      printf("\n INSTANCE(%s) %s\n\n",
        ((CDlpObject*)lpWord->lpData)->m_lpClassName,lpsIdentifier);
      break;
    case WL_TYPE_FACTORY:
      printf("\n CLASS %s",lpsIdentifier);
      printf("\n Comment: %s\n",lpWord->lpComment);
      printf("\n Version: %s (%s)",
        dlp_strlen(lpWord->ex.fct.version.no)?
          lpWord->ex.fct.version.no:"not specified",
        dlp_strlen(lpWord->ex.fct.version.date)?
          lpWord->ex.fct.version.date:"unknown date");
      printf("\n Author : %s",
        dlp_strlen(lpWord->ex.fct.lpAuthor)?
          lpWord->ex.fct.lpAuthor:"unknown");
      printf("\n Project: %s",lpWord->ex.fct.lpProject);
      printf("\n\n Instances of class \"%s\":",lpsIdentifier);

      hscan_t  hs;
        hnode_t* hn;
      hash_scan_begin(&hs,m_lpDictionary);

      INT32 ctr = 0;
      while ((hn = hash_scan_next(&hs)))
      {
        SWord* lpWord;
        DLPASSERT((lpWord=(SWord*)hnode_get(hn)));                              // NULL entry in dictionary

        if
        (
          lpWord->nWordType == WL_TYPE_INSTANCE &&
          dlp_strcmp(((CDlpObject*)lpWord->lpData)->m_lpClassName,lpsIdentifier)==0
        )
        {
          printf("\n %3ld: ",ctr);
          printf("%s",((CDlpObject*)lpWord->lpData)->m_lpInstanceName);
          if (dlp_strlen(lpWord->lpComment)) printf("%s",lpWord->lpComment);
          ctr++;
        }
      }
      if (!ctr) printf("\n - none -");

      printf("\n\n");
      break;
    }

    return O_K;
  }

  return IERROR(this,FNC_UNDEF,lpsIdentifier,0,0);
}

/**
 * Lists memory objects, the function's token sequence or label table,
 * scalar/aggragation/string operation codes or class/type names.
 *
 * @param lpsWhat
 *          Pointer to string containing the list identifier
 * @return <code>O_K</code> if successfull, a (negative) error code otherwise.
 */
INT16 CGEN_PUBLIC CFunction::List(const char* lpsWhat)
{
  // This goes first ...
  if (dlp_strcmp(lpsWhat,"stack")==0) return Status();
  if (dlp_strcmp(lpsWhat,"trace")==0) return PrintStackTrace();

  // General information
  PrintInfo("Data content");

  if (!dlp_strlen(lpsWhat))
  {
    if (!m_idTsq->IsEmpty())
    {
      PrintCode(5,"   ");
      dlp_fprint_x_line(stdout,'-',dlp_maxprintcols());
    }
    printf("\n   For more information type -list help;");
  }
  else if (dlp_strcmp(lpsWhat,"help"   )==0)
  {
    printf("\n   Available listings:");
    printf("\n   -list aggrops   List of aggregation operation codes");
    printf("\n   -list classes   List of installed dLabPro classes");
    printf("\n   -list const     List of constants");
    printf("\n   -list labels    List of the function's jump labels");
    printf("\n   -list matrops   List of matrix operation codes");
    printf("\n   -list sigops    List of signal operation codes");
    printf("\n   -list memory    List of allocated memory objects");
    printf("\n   -list scalops   List of scalar operation codes");
    printf("\n   -list stack     Dump the function's operand stack");
    printf("\n   -list strops    List of string operation codes");
    printf("\n   -list tokens    The function's token sequence");
    printf("\n   -list trace     Dump the function's call stack");
    printf("\n   -list types     List of dLabPro type names");
  }
  else if (dlp_strcmp(lpsWhat,"const"  )==0) dlp_constant_printtab();
  else if (dlp_strcmp(lpsWhat,"aggrops")==0) dlp_aggrop_printtab();
  else if (dlp_strcmp(lpsWhat,"classes")==0) CDlpObject_PrintClassRegistry();
  else if (dlp_strcmp(lpsWhat,"matrops")==0) dlm_matrop_printtab();
  else if (dlp_strcmp(lpsWhat,"memory" )==0) dlp_xalloc_print();
  else if (dlp_strcmp(lpsWhat,"scalops")==0) dlp_scalop_printtab();
  else if (dlp_strcmp(lpsWhat,"stack"  )==0) Status();
  else if (dlp_strcmp(lpsWhat,"strops" )==0) dlp_strop_printtab();
  else if (dlp_strcmp(lpsWhat,"types"  )==0) dlp_type_printtab();
  else if (dlp_strcmp(lpsWhat,"tokens" )==0 || dlp_strcmp(lpsWhat,"labels")==0)
  {
    BOOL bLabels = (dlp_strcmp(lpsWhat,"labels")==0);
    INT32 nLines  = 0;
    INT32 i       = 0;
    if (m_idTsq && m_idTsq->GetNRecs()>0)
    {
      if (!bLabels)
      {
        printf("\n   PP   : Program pointer");
        printf("\n   SF   : Source file index (see field sfl)");
        printf("\n   LN   : Line");
        printf("\n   {}   : Curly brace nesting level");
        printf("\n   IF   : if/while nesting level");
        printf("\n   TY   : Token type");
        printf("\n   TOKEN: Token string");
        printf("\n   PP SF LN {} IF TY  TOKEN");
      }
      for (i=0; i<m_idTsq->GetNRecs(); i++)
      {
        if (bLabels && !__TTYP_IS(i,TT_LAB)) continue;
        char lpsToken[255];
        char lpsIsdl[255];
        dlp_strreplace(dlp_strabbrv(lpsToken,__TOK(i),50),"\n","\x014");
        dlp_strquotate(lpsToken,'|','|');
        dlp_strreplace(dlp_strabbrv(lpsIsdl,__IDEL(i),25),"\n","\x014");
        dlp_strquotate(lpsIsdl,'|','|');
        printf("\n%5ld%3ld%3ld%3ld%3ld %-2s  %-52s %s",i,
          (INT32)CData_Dfetch(m_idTsq,i,OF_SRCID),__LINE(i),__BLV(i,0),
          __BLV(i,1),__TTYP(i),lpsToken,lpsIsdl);
        nLines++;
      }
      if (bLabels && !nLines) printf("\n   -- No labels defined --");
    }
    else printf("\n   -- No function body defined --");
  } else if (dlp_strcmp(lpsWhat,"sigops")==0)
  {
    CDlpObject* lpObject = FindInstance("signal");
    if(lpObject) CDlpObject_PrintAllMembers(lpObject,WL_TYPE_OPERATOR,PWL_HELP);
  }

  // Footer
  printf("\n"); dlp_fprint_x_line(stdout,'-',dlp_maxprintcols());
  printf("\n");

  return O_K;
}

/**
 * Print debug message.
 *
 * @param lpsMsg
 *          The message text (<code>printf</code>-formatting string)
 * @param ...
 *          Three value for first field in formatting string (<code>o</code> if
 *          unused)
 */
void CGEN_PROTECTED CFunction::Msg(const char* lpsMsg, ...)
{
  // Local variables
  va_list ap;
  void*   lpArg1     = NULL;
  void*   lpArg2     = NULL;
  void*   lpArg3     = NULL;
  char    lpBuf[255];
  INT32   nRecsTeq   = m_idTeq!=NULL?m_idTeq->GetNRecs():-1;

  // Get items of the parameter list
  va_start(ap,lpsMsg);
  lpArg1 = va_arg(ap,void*);
  lpArg2 = va_arg(ap,void*);
  lpArg3 = va_arg(ap,void*);
  va_end(ap);

  // Print message
  dlp_strcpy(lpBuf,m_iAi?m_iAi->m_lpInstanceName:"-");
  if (m_iAi2)
  {
    dlp_strcat(lpBuf,",");
    dlp_strcat(lpBuf,m_iAi2->m_lpInstanceName);
  }
  printf("\n%5ld%5ld%3ld %c%-13s: ",(long)m_nPp,(long)nRecsTeq,(long)m_nStackLen,
    !m_iAi||m_bAiUsed?' ':'*',lpBuf);
  printf(lpsMsg,lpArg1,lpArg2,lpArg3);
}

/**
 * Check if a newly created instance can be named lpsName.
 *
 * @param lpsToken
 *          Pointer to name to be checked
 * @return <code>O_K</code> if name is valid, a (negative) error code otherwise.
 */
INT16 CGEN_PUBLIC CFunction::IsValidInstanceName(CDlpObject* iCont, const char* lpsToken)
{
  // General checks
  if (!dlp_strlen(lpsToken)) return NOT_EXEC;                                   // Must be non-empty
  if (isdigit(lpsToken[0]) ) return NOT_EXEC;                                   // Because of numbers
  if (lpsToken[0] == '-'   ) return NOT_EXEC;                                   // Because of commands
  if (lpsToken[0] == '/'   ) return NOT_EXEC;                                   // Because of options
  if (strstr(lpsToken,".") ) return NOT_EXEC;                                   // Member

  // Known identifier?
  if (m_iAi && m_iAi->FindWord(lpsToken)) return FNC_DBL;                       // Member of active instance
  if (iCont->FindWord(lpsToken)         ) return FNC_DBL;                       // Member of this function

  // TODO: Positive character check (a-z,A-Z,0-9,_)

  // Everything ok
  return O_K;
}

/**
 * Determines the function instance which called this instance. This method
 * provides that an method invocation context has been set.
 *
 * @return A pointer to the calling function instance or <code>NULL</code> if
 *         no such instance could be determined.
 */
CFunction* CGEN_PROTECTED CFunction::GetCaller()
{
  return (CFunction*)CDlpObject_OfKind("function",m_iCaller);
}

/**
 * <p>Determines the active instance. The method returns:</p>
 * <ol>
 *   <li>{@link ai m_iAi} if not <code>NULL</code>,</li>
 *   <li>{@link ai2 m_iAi2} if not <code>NULL</code> or</li>
 *   <li><code>this</code> if no other case applies.</li>
 * </ol>
 *
 * @return A pointer to the currently active instance
 */
CDlpObject* CGEN_PROTECTED CFunction::GetActiveInstance()
{
  if (m_iAi ) return m_iAi;
  if (m_iAi2) return m_iAi2;
  return this;
}

/**
 * Creates a new instance of a dLabPro class and registers it with this
 * function. This is the standard procedure of the interpreter instanciating
 * classes.
 *
 * @param lpsClassName    Pointer to dLabPro class identifier
 * @param lpsInstanceName Pointer to identifier for instance to be create
 * @return A pointer to the newly created instance or <code>NULL</code> in case
 *         of errors
 */
CDlpObject* CGEN_PROTECTED CFunction::Instantiate
(
  const char* lpsClassName,
  const char* lpsInstanceName
)
{
  /* If lpsInstanceName is qualified split in container and instance name */
  char  lpsBuffer[256];
  char* lpsContainer   = NULL;
  char* tx             = NULL;
  dlp_strcpy(lpsBuffer,lpsInstanceName);
  for (tx=&lpsBuffer[dlp_strlen(lpsBuffer)-1]; tx>=lpsBuffer; tx--)
    if (*tx=='.')
    {
      *tx++='\0';
      lpsInstanceName = tx;
      lpsContainer    = lpsBuffer;
    }

  /* Determine container instance (Variables not allowed as containers!) */
  CDlpObject* iCont = this;
  if (lpsContainer)
  {
    if (dlp_strlen(lpsContainer)==0) iCont = GetRoot();
    else iCont = FindInstanceWord(lpsContainer,NULL);
    if (!iCont) { IERROR(this,FNC_UNDEF,lpsContainer,0,0); return NULL; }
    if (iCont->IsKindOf("var")) iCont=iCont->m_iAliasInst;
    if (!iCont)
    {
      IERROR(this,ERR_ILLEGALQUALI,lpsInstanceName,lpsContainer,0);
      return NULL;
    }
  }

  /* Check new instance name */
  switch (IsValidInstanceName(iCont,lpsInstanceName))
  {
    case FNC_INVALID:
      IERROR(this,FNC_INVALID,"identifier",lpsInstanceName,0);
      return NULL;
    case FNC_DBL:
      IERROR(this,FNC_DBL,lpsInstanceName,"defined",0);
      return NULL;
  }

  /* Create new instance */
  FNC_MSG(2,"  - Creating instance %s \"%s\" in %s",lpsClassName,
    lpsInstanceName,CDlpObject_GetFQName(iCont,__lpsFqid,FALSE),0,0);
  CDlpObject* iInst = CDlpObject_Instantiate(iCont,lpsClassName,lpsInstanceName,this!=GetRoot());
  if (!iInst)
  {
    IERROR(this,FNC_INTERNAL,"Instantiation error",__FILE__,__LINE__);
    return NULL;
  }
  SWord cwrd; iInst->GetInstanceInfo(&cwrd);
  if ((cwrd.nFlags & CS_AUTOACTIVATE) && (cwrd.nFlags & CS_SECONDARY)==0)
    if (dlp_strncmp(iInst->m_lpInstanceName,cwrd.ex.fct.lpAutoname,L_NAMES)!=0) //   Not the auto-instance
    {                                                                           //   >>
      PushInstance(iInst);
      m_iAi     = iInst;
      m_bAiUsed = FALSE;
      //IFCHECKEX(2) printf(", activating instance");
      //if (!m_bDisarm)
      //{
      //  CDlpObject_MicSet(iInst,&m_mic);
      //  iInst->ClassProc();
      //  CDlpObject_MicSet(iInst,NULL);
      //}
      //m_bDisarm = FALSE;
    }                                                                           //   <<

  return iInst;
}

/**
 * Destroys an instance of a dLabPro class and removes references from the
 * instance tree. This is the standard procedure of the interpreter destroying
 * an instance.
 *
 * @param iInst
 *          The instance to destroy
 * @param nMode
 *          <p>Controls the removal of references to <code>iInst</code>. Use modes
 *          0 and 1 if (and only if) it is sure there are no references outside
 *          the scanned parts of the instance tree (see table below).</p>
 *          <table>
 *            <tr><th><code>nMode</code></th><th>Description</th></tr>
 *            <tr><td>0</td><td>Remove only references in this instance</td></tr>
 *            <tr><td>1</td><td>Remove references in this instance and all nested
 *                              instances (recursively descents into field and
 *                              instance words starting with this</td></tr>
 *            <tr><td>2</td><td>Removes all references from the entire instance
 *                              tree (recursively descents into field and instance
 *                              words starting with the root instance</td></tr>
 *          </table>
 * @return <code>O_K</code> if successful, a (negative) error code otherwise
 */
INT16 CGEN_PROTECTED CFunction::Deinstanciate(CDlpObject* iInst, INT16 nMode)
{
  CFunction* iRoot = (CFunction*)CDlpObject_OfKind("function",GetRoot());
  if (!iInst     ) return NOT_EXEC;                                             // Nothing to be done
  if (iInst==this) return NOT_EXEC;                                             // Won't kill ourselves!
  if ((iInst->m_nClStyle&CS_SINGLETON) && iRoot && (iRoot->m_nXm&XM_QUIT)==0)   // Won't destroy singletons
    return IERROR(this,FNC_DESTROY,iInst->m_lpInstanceName,0,0);                // |

  FNC_MSG(2,"  - Destroying instance \"%s\"",iInst->GetFQName(__lpsFqid),0,0,0,0);// Protocol (verbose level 2)
  DerefInstance(nMode>=2?GetRoot():this,iInst,nMode>=1,m_nCheck);               // Remove references to iInst
  IDESTROY(iInst);                                                              // Destroy iInst
  return O_K;                                                                   // That's it
}

/**
 * (Recursively) removes references to an instance from the dictionary of
 * another instance. The method does not destroy the instance.
 *
 * @param iCont
 *          Pointer to container instance to scan for references to
 *          <code>iInst</code>
 * @param iInst
 *          Pointer to instance to remove references to
 * @param bDescent
 *          If <code>TRUE</code> recursively check (inner) instances and fields
 * @param nVerbose
 *          Verbose level (default is 0)
 * @return The return value is used to control the recursion. Otherwise it has
 *         no meaning. During the recusion <code>TRUE</code> indicates that the
 *         recursion shall be continued, <code>FALSE</code> causes the recursion
 *         to stop immediately.
 */
BOOL CGEN_SPRIVATE CFunction::DerefInstance
(
  CDlpObject* iCont,
  CDlpObject* iInst,
  BOOL        bDescent,
  INT16       nVerbose DEFAULT(0)
)
{
  // Validate                                                                   // ------------------------------------
  if (!iInst) return FALSE;                                                     // Stop here and do not proceed
  if (!iCont) return TRUE;                                                      // Stop here but proceed

  if (nVerbose>=2) printf("\n%32sDereference %s in %s","",iInst->m_lpInstanceName,iCont->m_lpInstanceName);

  // Scan iCont's dictionary for references to iInst                            // ------------------------------------
  SWord*   lpWord = NULL;                                                       // Pointer to currently examined word
  hscan_t  hs;                                                                  // Hash scan data struct
  hnode_t* hn     = NULL;                                                       // Pointer to current hash node
  hash_scan_begin(&hs,iCont->m_lpDictionary);                                   // Initialize scanning dictionary
  while ((hn = hash_scan_next(&hs))!=NULL)                                      // Scan dictionary
  {                                                                             // >>
    DLPASSERT((lpWord = (SWord*)hnode_get(hn))!=NULL);                          //   NULL entry in dictionary
    if (lpWord->nWordType == WL_TYPE_INSTANCE)                                  //   Is current word is an instance?
    {                                                                           //   >>
      if ((CDlpObject*)lpWord->lpData == iInst)                                 //     Does it hold a ref. to iInst
      {                                                                         //     >>
        if (nVerbose>=2)                                                        //       On verbose level 2
          printf("\n%32sUnregister word \"%s\" in \"%s\"","",lpWord->lpName,    //         Protocol
            iCont->GetFQName(__lpsFqid));                                       //         |
        iCont->UnregisterWord(lpWord,FALSE);                                    //       Remove that big bad word
      }                                                                         //     <<
      else if (bDescent)                                                        //     Otherwise if descending
      {                                                                         //     >>
        if (nVerbose>=2) printf("\n%32sDescending to %s.%s","",iCont->m_lpInstanceName,lpWord->lpName);
        if (!DerefInstance((CDlpObject*)lpWord->lpData,iInst,TRUE,nVerbose))    //       Do descent
          return FALSE;                                                         //         and stop if required
      }                                                                         //     <<
    }                                                                           //   <<
    else if                                                                     //   Is current word an instance field?
    (                                                                           //   |
      lpWord->nWordType    == WL_TYPE_FIELD &&                                  //   | - Current word is field ...
      lpWord->ex.fld.nType == T_INSTANCE                                        //   | - of type instance
    )                                                                           //   |
    {                                                                           //   >>
      CHECK_IPTR(*(CDlpObject**)lpWord->lpData,lpWord->ex.fld.nISerialNum);     //     If ptr. invalid make it NULL
      if (*(CDlpObject**)lpWord->lpData == iInst)                               //     Does it holds a ref. to iInst?
      {                                                                         //     >>
        if (nVerbose>=2)                                                        //       On verbose level 2
          printf("\n%32sSet field \"%s\" in \"%s\" to NULL","",lpWord->lpName,  //         Protocol
            iCont->GetFQName(__lpsFqid));                                       //         |
        *(CDlpObject**)lpWord->lpData = NULL;                                   //       Remove reference
      }                                                                         //     <<
      else if (bDescent && (!lpWord->nFlags&FF_NONAUTOMATIC))                   //     Otherwise if descending
      {                                                                         //     >>
        if (nVerbose>=2) printf("\n%32sDescending to %s.%s","",iCont->m_lpInstanceName,lpWord->lpName);
        if (!DerefInstance(*(CDlpObject**)lpWord->lpData,iInst,TRUE,nVerbose))  //       Do descent
          return FALSE;                                                         //         and stop if required
      }                                                                         //     <<
    }                                                                           //   <<
  }                                                                             // <<

  // If iCont is a function scan stack and active instances                     // ------------------------------------
  CFunction* ifCont = (CFunction*)OfKind("function",iCont);                     // Try to cast iCont to CFunction
  if (ifCont)                                                                   // If successfull
    for (INT32 n=0; n<ifCont->m_nStackLen; n++)                                 //   Loop over ifCont's stack items
      if (ifCont->m_aStack[n].nType==T_INSTANCE)                                //     Current item is an instance
        if (ifCont->m_aStack[n].val.i==iInst)                                   //       More precisely: it is iInst!
        {                                                                       //       >>
          if (nVerbose>=2)                                                      //         On verbose level 2
            printf("\n%32sSet stack item %ld in \"%s\" to NULL","",(long)n,     //           Protocol
              iCont->GetFQName(__lpsFqid));                                     //           |
          ifCont->m_aStack[n].val.i=NULL;                                       //         Remove that reference
        }                                                                       //       <<

  // Check if iCont is aliased to iInst                                         // ------------------------------------
  if (iCont->m_iAliasInst == iInst)                                             // iCont aliased to iInst?
  {                                                                             // >> Yes
    if (nVerbose>=2)                                                            //   On verbose level 2
      printf("\n%32sSet alias instance of \"%s\" to NULL","",                   //     Protocol
        iCont->GetFQName(__lpsFqid));                                           //     |
    iCont->m_iAliasInst = NULL;                                                 //   Remove alias
  }                                                                             // <<

  return TRUE;                                                                  // Go on ...
}

/**
 *
 */
INT16 CGEN_PUBLIC CFunction::Check()
{
  INT16 nErr = O_K;
  if (GetRoot()) nErr = GetRoot()->Check(CFM_RECURSIVE);                        // If root exists, delegate call
  return nErr;
}


/**
 * Finds a word by its (qualified) indentifier. The method searches (1) in the
 * active instance {@link ai m_iAi}, (2) in the secondary active instance
 * {@link ai2 m_iAi2}, in this instance and, finally, global instances in the
 * calling function(s).
 *
 * @param _this        This instance
 * @param lpIdentifier Identifier of word to search
 * @param nMask        Word types to search. Any combination of WL_TYPE_XXX
 *                     codes. Default is WL_TYPE_DONTCARE.
 * @return A pointer to the word or NULL if identifier was not found.
 */
SWord* CGEN_PROTECTED CFunction::FindWordAi(const char* lpsIdentifier)
{
  CFunction*  iFnc             = NULL;
  CDlpObject* iRinst           = NULL;
  SWord*      lpWord           = NULL;
  SWord*      lpRwrd           = NULL;
  char*       lpsId            = NULL;
  char        lpsBuf[L_NAMES];

  if (m_iAi            ) lpWord = m_iAi->FindWord(lpsIdentifier);               // First look in primary active inst.
  if (!lpWord && m_iAi2) lpWord = m_iAi2->FindWord(lpsIdentifier);              // Then look in secondary active inst.
  if (!lpWord          ) lpWord = FindWord(lpsIdentifier);                      // Then look anywhere else
  if (!lpWord)                                                                  // Still not found
    for (iFnc=this; !lpWord; iFnc=iFnc->GetCaller())                            //   Traverse calling functions
    {                                                                           //   >>
      if (!iFnc) break;                                                         //     No (further) caller -> stop
      if (!(lpWord = iFnc->FindWord(lpsIdentifier))) continue;                  //     Find word in caller or continue
      strcpy(lpsBuf,lpsIdentifier);                                             //     Copy identifier
      lpsId  = strtok(lpsBuf,".");                                              //     Get its root identifer
      lpRwrd = iFnc->FindWord(lpsId);                                           //     Get the root word
      if (lpRwrd && lpRwrd->nWordType==WL_TYPE_INSTANCE)                        //     Get the root instance
        iRinst = (CDlpObject*)lpRwrd->lpData;                                   //     ...
      if (!iRinst || (!iRinst->m_nInStyle&IS_GLOBAL)) lpWord = NULL;            //     Not a global inst. -> don't use
    }                                                                           //   <<
  return lpWord;                                                                // Return the word
}

// EOF
