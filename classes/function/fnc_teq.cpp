// dLabPro class CFunction (function)
// - Token execution queue implementation
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

#if (!defined __NOREADLINE && defined __LINUX)
#  include <readline/readline.h>
#  include <readline/history.h>
#endif

static char __CFunction_EmptyUserInput[L_INPUTLINE+1];                          // Most recent user input

/**
 * Breaks execution in /step mode.
 */
BOOL CGEN_PROTECTED CFunction::StepBreak()
{
  CFunction* iRoot = GetRootFnc();                                              // Get function
  if (!iRoot || !(iRoot->m_nXm & XM_STEP)) return FALSE;                        // Not in step mode -> forget it!
  iRoot->m_nXm &= ~XM_STEP;                                                     // Clear step mode
  PrintCode(0);                                                                 // Display current code line
  m_nXm |= XM_BREAK;                                                            // Set break mode
  return TRUE;                                                                  // Indicate step mode to caller
}

BOOL CGEN_PROTECTED CFunction::Interrupt()
{
  if(dlp_get_interrupt() && !dlp_get_nonstop_mode()) {
    m_nXm |= XM_BREAK;
    m_nPp--;
    PrintStackTrace();
    m_nPp++;
    return TRUE;
  }
  return FALSE;
}

/**
 * Tokenizes a dLabPro command string (a snippet of dLabPro script code)
 * and stores the resulting token sequence into the token execution queue.
 *
 * <h3>Notes</h3>
 * <ul>
 *   <li>In contrast to {@link SendCommand} this method works asynchroneously,
 *       i.e. returns immediately without waiting for the commmand to be
 *       executed.</li>
 * </ul>
 *
 * @param lpsCommand
 *          Null terminated string containing the dLabPro command
 * @param lpsIdel
 *          String of insignificant delimiters (white spaces) following the
 *          command (e.g. in a source file); may be <code>NULL</code>. This
 *          delimiter string will be stored in the token execution queue thus
 *          allowing a precise reconstruction of a source file during execution.
 * @param nLine
 *          Current line in the source file (the default value of -1 indicates
 *          "no source file")
 * @param bHead
 *          If <code>TRUE</code> the method will place the parsed tokens at the
 *          head of the execution queue (for immediate processing). The default
 *          behaviour is appending the tokens at the end of the queue.
 *
 * @return The number of posted tokens.
 *
 * @see teq Token execution queue (field teq)
 * @see PostTokens
 * @see SendCommand
 */
INT32 CGEN_PUBLIC CFunction::PostCommand
(
  const char* lpsCommand,
  const char* lpsIdel   DEFAULT(NULL),
  INT32       nLine     DEFAULT(-1),
  BOOL        bHead     DEFAULT(FALSE)
)
{
  return PostTokensInt(NULL,lpsCommand,lpsIdel,nLine,bHead);
}

/**
 * Stores a token sequence into the token execution queue.
 *
 * <h3>Notes</h3>
 * <ul>
 *   <li>In contrast to {@link SendTokens} this method works asynchroneously,
 *       i.e. returns immediately without waiting for the commmand to be
 *       executed.</li>
 * </ul>
 *
 * @param idTsq
 *          Pointer to a CData instance containing the token sequence
 * @param nLine
 *          Current line in the source file (the default value of -1 indicates
 *          "no source file")
 * @param bHead
 *          If <code>TRUE</code> the method will place the parsed tokens at the
 *          head of the execution queue (for immediate processing). The default
 *          behaviour is appending the tokens at the end of the queue.
 * @return The number of posted tokens.
 *
 * @see teq Token execution queue (field teq)
 * @see SendTokens
 * @see PostCommand
 * @see SendCommand
 */
INT32 CGEN_PROTECTED CFunction::PostTokens
(
  CData* idTsq,
  INT32   nLine DEFAULT(-1),
  BOOL   bHead DEFAULT(FALSE)
)
{
  return PostTokensInt(idTsq,NULL,NULL,nLine,bHead);
}

/**
 * Internally used by PostCommand and PostTokens.
 * @see PostCommand
 * @see PostTokens
 */
INT32 CGEN_PRIVATE CFunction::PostTokensInt
(
  CData*      idTsq,
  const char* lpsCommand,
  const char* lpsIdel,
  INT32        nLine      DEFAULT(-1),
  BOOL        bHead      DEFAULT(FALSE)
)
{
  INT32  nTokens = 0;                                                           // Number of posted tokens (return v.)
  CDgen* iPar    = GetDlpParser();                                              // Get the dLabPro document parser

  DLPASSERT(iPar   );                                                           // Need document parser
  DLPASSERT(m_idTeq);                                                           // Need token sequence
  DLPASSERT((idTsq && !lpsCommand) || (!idTsq && lpsCommand));                  // Either or!

  if (lpsCommand)                                                               // String input
  {                                                                             // >>
    if (!dlp_strlen(lpsCommand)) return 0;                                      //   Not nice but ok
    iPar->TsqInit(iPar->m_idTsq);                                               //   Clear token queue in doc. parser
    iPar->TokenizeString(lpsCommand,nLine);                                     //   Tokenize command string (common)
    iPar->Tokenize2("dlp",TRUE);                                                //   Tokenize command string (dLabPro)
    idTsq = iPar->m_idTsq;                                                      //   Copy pointer to token queue
  }                                                                             // <<
  DLPASSERT(idTsq);                                                             // Now must have a valid token queue
  nTokens = CData_GetNRecs(idTsq);                                              // Count parsed tokens
  if (!nTokens) return 0;                                                       // If none, return
  dlp_strcpy(__IDEL_EX(idTsq,nTokens-1),lpsIdel);                               // Store insignificant delimiter string
  if (bHead)                                                                    // Insert at queue head
  {                                                                             // >>
    CData_Cat(idTsq,m_idTeq);                                                   //   Cat execution queue to parsed seq.
    CData_Copy(m_idTeq,idTsq);                                                  //   Replace exe. queue by parsed seq.
  }                                                                             // <<
  else                                                                          // Append at end of queue
    CData_Cat(m_idTeq,idTsq);                                                   //   Cat parsed seq. to execution queue
  return nTokens;                                                               // Yo!
}

/**
 * Tokenizes a dLabPro command string (a snippet of dLabPro script code)
 * and evaluates it. The result of the evaluation is stored as string in
 * <code>lpsResult</code>.
 *
 * <h3>Notes</h3>
 * <ul>
 *   <li>In contrast to {@link PostCommand} this method works synchronously,
 *       i.e. it does not return until <code>lpsCommand</code> has been
 *       executed. The command is executed immediately prior to all other
 *       tokens waiting in the {@link teq token execution queue}.</li>
 *   <li style="color:red">EXPERIMENTAL and sufficiently dangerous!</li>
 * </ul>
 *
 * @param lpsCommand
 *          Null terminated string containing the dLabPro command
 * @param lpsResult
 *          Buffer to be filled with result string
 * @param nLength
 *          Maximal number of characters (including the terminal null) be stored
 *          into <code>lpsResult</code>
 * @param nLine
 *          Current line in the source file (the default value of -1 indicates
 *          "no source file")
 * @return <code>O_K</code> if successful, a (negative) error code otherwise.
 *
 * @see PostCommand
 * @see SendTokens
 * @see PostTokens
 */
INT16 CGEN_PROTECTED CFunction::SendCommand
(
  const char* lpsCommand,
  char*       lpsResult,
  INT32        nLength,
  INT32        nLine      DEFAULT(-1)
)
{
  if (!dlp_strlen(lpsCommand)) return O_K;
  return SendTokensInt(NULL,lpsCommand,lpsResult,nLength,nLine);
}

/**
 * Evaluates a token sequence. The result of the evaluation is stored as string
 * in <code>lpsResult</code>.
 *
 * <h3>Notes</h3>
 * <ul>
 *   <li>In contrast to {@link PostTokens} this method works synchronously,
 *       i.e. it does not return until <code>idTsq</code> has been
 *       executed. The command is executed immediately prior to all other
 *       tokens waiting in the {@link teq token execution queue}.</li>
 *   <li style="color:red">EXPERIMENTAL and sufficiently dangerous!</li>
 * </ul>
 *
 * @param idTsq
 *          Pointer to a CData instance containing the token sequence
 * @param lpsResult
 *          Buffer to be filled with result string
 * @param nLength
 *          Maximal number of characters (including the terminal null) be stored
 *          into <code>lpsResult</code>
 * @param nLine
 *          Current line in the source file (the default value of -1 indicates
 *          "no source file")
 * @return <code>O_K</code> if successful, a (negative) error code otherwise.
 *
 * @see SendCommand
 * @see PostTokens
 * @see PostCommand
 */
INT16 CGEN_PROTECTED CFunction::SendTokens
(
  CData* idTsq,
  char*  lpsResult,
  INT32   nLength,
  INT32   nLine      DEFAULT(-1)
)
{
  return SendTokensInt(idTsq,NULL,lpsResult,nLength,nLine);
}

/**
 * Internally used by SendCommand and SendTokens.
 * @see SendCommand
 * @see SendTokens
 */
INT16 CGEN_PRIVATE CFunction::SendTokensInt
(
  CData*      idTsq,
  const char* lpsCommand,
  char*       lpsResult,
  INT32        nLength,
  INT32        nLine      DEFAULT(-1)
)
{
  INT32        nTeqOffsetSave  = 0;                                           // Copy of teq offset for sync. exec.
  BOOL        bAiUsedSave     = FALSE;                                          // Copy of act. inst. used flag
  CDlpObject* iAiSave         = NULL;                                           // Copy of active instance pointer
  CDlpObject* iAi2Save        = NULL;                                           // Copy of secondary act. inst. pointer
  BOOL        bDisarm         = FALSE;                                          // Copy of /disarm option

  DLPASSERT((idTsq && !lpsCommand) || (!idTsq && lpsCommand));                  // Either or!
  if (!lpsResult) return NOT_EXEC;                                              // No result buffer, no service!
  if (nLength<=0) return NOT_EXEC;                                              // Dito
  lpsResult[0]='\0';                                                            // Clear result buffer
  if (lpsCommand && !dlp_strlen(lpsCommand)) return O_K;                        // No command, nothing to be done!

  char lpsBuf[L_INPUTLINE+1];                                                   // String manipulation buffer
  INT32 nTokens               = 0;                                               // Number of parsed tokens

  PushBrake();                                                                  // Push a stack brake
  if (lpsCommand)                                                               // String input:
  {                                                                             // >>
    dlp_strcpy(lpsBuf,lpsCommand);                                              //   Copy command string
    dlp_strreplace(lpsBuf,"\\ "," ");                                           //   Unquote spaces
    FNC_MSG(2,"  - Sending command |%s|",lpsBuf,0,0,0,0);                       //   Protocol (verbose level 2)
    nTokens = PostTokensInt(NULL,lpsBuf,NULL,nLine,TRUE);                       //   Post cmd. to head of exe. queue
  }                                                                             // <<
  else                                                                          // Token sequence input:
  {                                                                             // >>
    FNC_MSG(2,"  - Sending %ld tokens",(long)CData_GetNRecs(idTsq),0,0,0,0);    //   Protocol (verbose level 2)
    nTokens = PostTokensInt(idTsq,NULL,NULL,nLine,TRUE);                        //   Post cmd. to head of exe. queue
  }                                                                             // <<
  FNC_MSG(2,">>> Synchroneous execution >>>>>>>>>>",0,0,0,0,0);                 // Protocol (verbose level 2)
  nTeqOffsetSave = m_nTeqOffset;                                                // Save execution queue offset
  iAiSave        = m_iAi;                                                       // Save active instance
  iAi2Save       = m_iAi2;                                                      // Save secondary active instance
  bAiUsedSave    = m_bAiUsed;                                                   // Save active instance used flag
  bDisarm        = m_bDisarm;                                                   // Save /disarm option
  m_nTeqOffset   = 0;                                                           // Sync. execution queue offset is 0
  m_iAi          = NULL;                                                        // No sync. active instance so far
  m_iAi2         = NULL;                                                        // No sync. 2ndary active inst. so far
  m_bAiUsed      = FALSE;                                                       // No sync. instance used so far
  m_bDisarm      = FALSE;                                                       // Clear /disarm option
  for ( ; nTokens; nTokens--) ExecuteToken();                                   // Execute parsed tokens
  m_nTeqOffset   = nTeqOffsetSave;                                              // Restore execution queue offset
  m_iAi          = iAiSave;                                                     // Restore active instance
  m_iAi2         = iAi2Save;                                                    // Restore secondray active instance
  m_bAiUsed      = bAiUsedSave;                                                 // Restore active instance used flag
  m_bDisarm      = bDisarm;                                                     // Restore /disarm option
  FNC_MSG(2,"<<< Synchroneous execution <<<<<<<<<<",0,0,0,0,0);                 // Protocol (verbose level 2)
  if (m_aStack[0].nType!=T_STKBRAKE)                                            // Something useful on the stack?
  {                                                                             // >>
    dlp_strncpy(lpsResult,PopString(-1),nLength);                               //   Get value on stack top as string
    lpsResult[nLength]='\0';                                                    //   Force terminal null
  }                                                                             // <<
  StackClearBrake();                                                            // Dump stack remains and stack brake
  if(lpsCommand) {
    FNC_MSG(2,"  - |%s| evaluates to |%s|",lpsCommand,lpsResult,0,0,0);         // Protocol (verbose level 2)
  } else {
    FNC_MSG(2,"  - Tokens evaluate to |%s|",lpsResult,0,0,0,0);                 // Protocol (verbose level 2)
  }
  IFCHECKEX(2) StackPrint();                                                    // Dump stack (verbose level 2)

  return O_K;                                                                   // Done well
}

/**
 * EXPERIMENTAL
 * TODO: Find solution for non-Windows OSes!
 * TODO: Move to dlp_base!?
 */
INT16 __kbhit()
{

#ifndef __WIN32__                                                               /* #ifndef __WIN32__                 */

  fd_set rfds;                                                                  /* File descriptor set               */
  struct timeval tv;                                                            /* Time value struct                 */
  FD_ZERO(&rfds);                                                               /* Clear file descriptor set         */
  FD_SET(0, &rfds);                                                             /* Add stdin (fd 0)                  */
  tv.tv_sec = 0; tv.tv_usec = 0;                                                /* Dont't wait                       */
  return select(1, &rfds, NULL, NULL, &tv)>0;                                   /* Return if something has changes   */

#elif !defined __CYGWIN__                                                        /* #ifndef __WIN32__                 */

  return filelength(STDIN_FILENO)>0;                                            /* Return file length of stdin >0    */

#endif                                                                          /* #ifndef __WIN32__                 */

}

/**
 * Pumps one token from the function's token sequence into its execution queue.
 * The method calls {@link PreprocessToken} on the token at the program
 * pointer's ({@link pp}) position in the function token sequence ({@link tsq}).
 * This may may cause the token to split up (because of $... replacements).
 * Hence pumping one function token may result in enqueueing several tokens
 * {@link teq}. The {@link pp program pointer} will be adjusted to point to the
 * next function token.
 *
 * @return <code>O_K</code> if successfull, a (negative) error code otherwise.
 */
INT16 CGEN_PROTECTED CFunction::PumpToken(BOOL bPostSyn DEFAULT(FALSE))
{
  char        lpsBuf[256];                                                      // String buffer
  char        lpsToken[L_INPUTLINE+1];                                          // Token buffer
  char        lpsIdel[256];                                                     // Insignificant delimiters buffer
  const char* lpsTt;                                                            // Token type
  char*       lpsSrc                  = NULL;                                   // Current source file
  INT32       nLine                   = -1;                                     // Current line number
  INT32       nTok                    = -1;                                     // Index of new teq token
  CData*      idArg                   = NULL;                                   // Command line arguments
  CFunction*  iRoot                   = NULL;                                   // Get root function

  // Validation                                                                 // ------------------------------------
  DLPASSERT(m_idTsq);                                                           // Need token sequence
  DLPASSERT(m_nXm & XM_EXEC);                                                   // Must be in execution state
  FNC_MSG(1,"- Pumping token",0,0,0,0,0);                                       // Protocol (verbose level 1)

  lpsIdel[0] = '\0';                                                            // Avoid read of uninitialized memory
  iRoot = (CFunction*)CDlpObject_OfKind("function",GetRoot());                  // Get root function
  idArg = GetRootFnc() ? GetRootFnc()->m_idArg : m_idArg;                       // MWX: WHY???

  // Get next token, preprocess                                                 // ------------------------------------
  if (!(m_nXm & XM_BREAK))                                                      // Not in break state
  {                                                                             // >>
    if (iRoot && (iRoot->m_nXm&XM_IN_IDE)!=0 && __kbhit()) m_nXm |= XM_STEP;    //   Break point requested via stdin
    dlp_strcpy(lpsToken,__TOK (m_nPp));                                         //   Copy token
    dlp_strcpy(lpsIdel ,__IDEL(m_nPp));                                         //   Copy insignificant delimiters
    lpsTt = __TTYP(m_nPp);                                                      //   Get token type
    IFCHECKEX(1) dlp_puts_ex(3," \"",lpsToken,"\"");                            //   Print the pumped token
    if (m_nPp<m_idTsq->GetNRecs())                                              //   Got new tokens
    {                                                                           //   >>
      lpsSrc = GetSrcFile(m_nPp);                                               //     Get current source file
      nLine  = __LINE(m_nPp);                                                   //     Get current line in source file
      CDlpObject_SetErrorPos(lpsSrc,nLine);                                     //     Remember src. pos. (error msgs.)
      if (!PreprocessToken(lpsToken,L_INPUTLINE,lpsTt,idArg,lpsSrc,nLine))      //     Preprocess the new token
      {                                                                         //     >> Token unchanged
        nTok = CData_AddRecs(m_idTeq,1,1);                                      //       Add token to execution queue
        dlp_memmove(CData_XAddr(m_idTeq,nTok,0),CData_XAddr(m_idTsq,m_nPp,0),   //       Copy token at program pointer
          CData_GetRecLen(m_idTsq));                                            //       |
        m_nPp++;                                                                //       Increment program pointer
        return O_K;                                                             //       All done
      }                                                                         //     <<
    }                                                                           //   <<
    else                                                                        //   Did not get new tokens
    {                                                                           //   >>
      if (GetCaller()) return NOT_EXEC;                                         //     Sub-function -> End of function
      else if (m_nXm & XM_NOPROMPT)                                             //     No user prompts mode
      {                                                                         //     >>
        m_nXm |= XM_QUIT;                                                       //       Session will terminate
        return NOT_EXEC;                                                        //       End of session
      }                                                                         //     <<
      else m_nXm |= (XM_BREAK | XM_AUTOBREAK);                                  //     Root function -> Break mode
    }                                                                           //   <<
  }                                                                             // <<
  if (m_nXm & XM_BREAK)                                                         // In break mode
  {                                                                             // >>
    char lpsPrompt[L_INPUTLINE];
#if (!defined __NOREADLINE && defined __LINUX)
    char *lpsInput;
#endif // #if (!defined __NOREADLINE && defined __LINUX)
    if (iRoot && (iRoot->m_nXm&XM_PIPEMODE)!=0)                                 //   Root function in pipe mode
      snprintf(lpsPrompt,L_INPUTLINE-1,"\npipemode>\n");                        //     Pipe mode prompt
    else                                                                        //   Root function not in pipe mode
    {                                                                           //   >>
      GetActiveInstance()->GetFQName(lpsBuf);                                   //     Generate prompt
      snprintf(lpsPrompt,L_INPUTLINE-1,"%s> ",bPostSyn==0?lpsBuf:"");           //     ...
    }                                                                           //   <<
    if(m_bTime) {                                                               //   Measure time option set?
      time_t t = dlp_time();                                                    //     Get current time
      dlp_message(GetSrcFile(0,m_idTeq),__LINE_EX(m_idTeq,0),                   //     Output
        "Elapsed time: %1.16gs\n",(double)(t-m_time)/1000.0);                   //     |
    }                                                                           //   <<
    if (ferror(stdin) || feof(stdin) ||                                         //   Broken standard input pipe
#if (!defined __NOREADLINE && defined __LINUX)
        (lpsInput=readline(lpsPrompt))==NULL                                    //     Read command line via readline
#else // #if (!defined __NOREADLINE && defined __LINUX)
        printf(lpsPrompt)<0 ||                                                  //     Show prompt
        fgets(lpsToken,L_INPUTLINE,stdin)==NULL                                 //     Read command line
#endif // #if (!defined __NOREADLINE && defined __LINUX)
    )
    {                                                                           //   >>
      printf(lpsPrompt);                                                        //     Show prompt
      IERROR(this,FNC_STDIN,0,0,0);                                             //     Warning
      if ((m_nXm & XM_AUTOBREAK) || m_nPp>=CData_GetNRecs(m_idTsq))             //     This is no break point
      {                                                                         //     >>
        printf(" Terminating program.");                                        //       Protocol
        exit(0);                                                                //       End program
      }                                                                         //     <<
      else                                                                      //     This is a break point
      {                                                                         //     >>
        printf(" Continuing program.");                                         //       Protocol
        strcpy(lpsToken,"cont");                                                //       Automatically continue
      }                                                                         //     <<
    }                                                                           //   <<
    else                                                                        //   Valid standard input pipe
    {                                                                           //   >>
      m_time = dlp_time();                                                      //   Save current clocks
      dlp_set_interrupt(FALSE);                                                 //   Clear user interrupt
#if (!defined __NOREADLINE && defined __LINUX)
      strncpy(lpsToken,lpsInput,L_INPUTLINE);                                   //     Copy command line
      if(lpsInput[0]) add_history(lpsInput);                                    //     Add to history if not empty
      free(lpsInput);                                                           //     Free memory
#endif // #if (!defined __NOREADLINE && defined __LINUX)
      dlp_strtrimleft(dlp_strtrimright(lpsToken));                              //     Trim white spaces
      EmptyUserInput(lpsToken);                                                 //     Remember/auto-insert token
    }                                                                           //   <<
    dlp_strtrimleft(dlp_strtrimright(lpsToken));                                //   Trim white spaces
    dlp_strcat(lpsToken,"\n");                                                  //   Append EOL
    PreprocessToken(lpsToken,L_INPUTLINE,TT_UNK,idArg,NULL,-1);                 //   Preprocess input
  }                                                                             // <<
  m_nXm &= ~XM_AUTOBREAK;                                                       // Clear auto-break flag

  // Post token and ajust program pointer                                       // ------------------------------------
  IFCHECKEX(1) dlp_puts_ex(3," --> RE-TOKENIZE \"",lpsToken,"\"");
  PostCommand(lpsToken,lpsIdel,nLine);                                          // Re-tokenize
  if (!(m_nXm & XM_BREAK)) m_nPp++;                                             // Increment program pointer

  return O_K;                                                                   // Done well.
}

/**
 * Executes the next token in the token execution queue. If the token execution
 * queue is empty, the method calls {@link PumpToken} to obtain additional
 * tokens from the function token sequence (field {@link tsq}).
 *
 * @return <code>O_K</code> if successful, <code>FNC_NOMORETOKENS</code> if
 * there are no more tokens in the token sequence or <code>FNC_UNKNOWN</code> if
 * the token could not be interpreted.
 */
INT16 CGEN_PROTECTED CFunction::ExecuteToken()
{
  INT16 nErr                    = O_K;                                          // Error status (return value)
  BOOL  bSemicolon              = FALSE;                                        // Formula token ends with semicolon
  char  lpsToken[L_INPUTLINE+1];                                                // Token buffer

  DLPASSERT(m_idTeq);                                                           // Must have token execution queue
  FNC_MSG(1,"Execute",0,0,0,0,0);                                               // Protocol (verbose level 1)
  if (m_idTeq->GetNRecs()==0)                                                   // Execution queue empty?
  {                                                                             // >>
    while (m_idTeq->GetNRecs()==0)                                              //   While execution queue empty
      IF_NOK(PumpToken())                                                       //     Pump tokens from m_idTsq
        break;                                                                  //       Until we can't get no more
    if (m_idTeq->GetNRecs()==0) return FNC_NOMORETOKENS;                        //   Did we come up with something?
  }                                                                             // <<

  const char* lpsFile = GetSrcFile(0,m_idTeq);
  const INT32 nLine   = __LINE_EX(m_idTeq,0);
  CDlpObject_SetErrorPos(lpsFile,nLine);                                        // Remember source pos. (error msgs.)
  dlp_strcpy(lpsToken,__TOK_EX(m_idTeq,0));                                     // Copy token string
  FNC_MSG(1,"- Executing (%s,\"%s\")",__TTYP_EX(m_idTeq,0),lpsToken,0,0,0);     // Protocol (verbose level 1)

  // Remove some escape sequences                                               // ------------------------------------
  dlp_strreplace(lpsToken,"\\#","#");                                           // "\#" -> "#"
  dlp_strreplace(lpsToken,"\\$","$");                                           // "\$" -> "$"

  // Execute the token                                                          // ------------------------------------
  if                                                                            // Token is...
  (                                                                             //
    __TTYP_IS_EX(m_idTeq,0,TT_BCMT) ||                                          //   ... a block comment OR
    __TTYP_IS_EX(m_idTeq,0,TT_DCMT) ||                                          //   ... a documentation comment OR
    __TTYP_IS_EX(m_idTeq,0,TT_LCMT) ||                                          //   ... a line comment OR
    __TTYP_IS_EX(m_idTeq,0,TT_ELIN) ||                                          //   ... an empty line OR
    __TTYP_IS_EX(m_idTeq,0,TT_WSPC) ||                                          //   ... a white string OR
    __TTYP_IS_EX(m_idTeq,0,TT_LAB )                                             //   ... a label definition
  )                                                                             //
  {                                                                             // >>
    // Do nothing                                                               //   Block intentionally empty
  }                                                                             // <<
  else if (__TTYP_IS_EX(m_idTeq,0,TT_FORM))                                     // Token is a formula
  {                                                                             // >>
    bSemicolon = FALSE;                                                         //   Suppose it ends with ':'
    if (lpsToken[dlp_strlen(lpsToken)-1]==';')                                  //   Ends with ';'
    {                                                                           //   >>
      lpsToken[dlp_strlen(lpsToken)-1]=':';                                     //     ';' -> ':'
      bSemicolon = TRUE;                                                        //     Remember that
    }                                                                           //   <<
    ItpAsFormula(dlp_strunquotate(lpsToken,':',':'));                           //   Interpret formula
    if (bSemicolon)                                                             //   Formula ends with semicolon
    {                                                                           //   >>
      if (StackGetLength()>0)                                                   //     Stack not empty
        IERROR(this,FNC_STACKOVERFLOW_WARNING,StackGetLength(),0,0);            //       Warning!
      StackClear();                                                             //     Clear stack
    }                                                                           //   <<
  }                                                                             // <<
  else if (__TTYP_IS_EX(m_idTeq,0,TT_STR ))                                     // Token is a string constant ("...")
    PushString(dlp_strconvert(SC_UNESCAPE,lpsToken,                             //   Push it
      dlp_strunquotate(lpsToken,'\"','\"')));                                   //   |
  else if (__TTYP_IS_EX(m_idTeq,0,TT_CHR ))                                     // Token is a char. constant ('...')
    PushString(dlp_strconvert(SC_UNESCAPE,lpsToken,                             //   Push it
      dlp_strunquotate(lpsToken,'\'','\'')));                                   //   |
  else if (                                                                     // Tokens that
    dlp_strlen(lpsToken)>=2 &&                                                  //   - are a least two characters long
    dlp_strstartswith(lpsToken,"\"") &&                                         //   - and start with dbl. quot. marks
    dlp_strendswith(lpsToken,"\"")                                              //   - and end with dbl. quot. marks
  )                                                                             // also go as strings...
  {                                                                             // >>
    PushString(dlp_strconvert(SC_UNESCAPE,lpsToken,                             //   Push it
      dlp_strunquotate(lpsToken,'\"','\"')));                                   //   |
  }                                                                             // <<
  else if (__TOK_IS_EX (m_idTeq,0,";"))                                         // End-of-instruction token
  {                                                                             // >>
    if (m_iAi) m_bAiUsed = TRUE;                                                //   Mark active instance used
    if (StackGetLength()>0)                                                     //   Stack not empty
    {                                                                           //   >>
      StkItm* lpSi = StackGet(0);                                               //     Get top of stack
      if (lpSi->nType!=T_INSTANCE || lpSi->val.i!=m_iAi)                        //     This happens on instantiations
        /*IERROR(this,FNC_STACKOVERFLOW_WARNING,StackGetLength(),0,0)*/{};      //       Warning!
    }                                                                           //   <<
    StackClear();                                                               //   Clear stack
    m_iAi     = NULL;                                                           //   Clear activate instance
    m_bAiUsed = FALSE;                                                          //   Declare virginal
    m_iAi2    = NULL;                                                           //   Clear secondary activate instance
    StepBreak();                                                                //   In step mode break here
    Interrupt();                                                                //   TODO: Comment this line!
  }                                                                             // <<
  else if (__TOK_IS_EX (m_idTeq,0,"(")) {}                                      // Ignore single (
  else if (__TOK_IS_EX (m_idTeq,0,")")) {}                                      // Ignore single )
  else if (__TTYP_IS_EX(m_idTeq,0,TT_UNK ))                                     // All other tokens:
  {                                                                             // >>
    if (ItpAsDirective(lpsToken)!=O_K)                                          //   Directive : must be ok
      if (ItpAsList(lpsToken)!=O_K)                                             //   List      : must be ok
        if (ItpAsOpAssign(lpsToken)!=O_K)                                       //   Assignment: must be ok
          if (ItpAsWord(lpsToken)==FNC_UNDEF)                                   //   Word      : must have been found
            if (ItpAsNumber(lpsToken)!=O_K)                                     //   Number    : must be ok
              if (ItpAsOperator(lpsToken)!=O_K)                                 //   Operator  : must be ok
              {                                                                 //   >> (none of the above)
                IERROR(this,FNC_UNDEF,lpsToken,0,0);                            //     Error message
                nErr = FNC_UNDEF;                                               //     Token is unknown
              }                                                                 //   <<
  }                                                                             // <<
  IFCHECKEX(2) StackPrint();                                                    // Dump stack (verbose level 2)

  TokenProcessed();                                                             // Token(s) have been processed
  DLP_CHECK_MEMINTEGRITY;                                                       // Check heap (DEBUG mode)
  DLP_CHECK_MEMLEAKS;                                                           // Check for memory leaks (DEBUG mode)

  return nErr;                                                                  // Return error code
}

/**
 * For postfix syntax: retrieves the text of the next non-blank and non-comment
 * token.
 *
 * @param bSameInstr
 *          If <code>TRUE</code> only get tokens within the same instruction
 *          (i.e. before the next '<code>;</code>')
 * @return Pointer to the token text or <code>NULL</code> if no next token
 *         exists
 */
const char* CGEN_PUBLIC CFunction::GetNextToken(BOOL bSameInstr DEFAULT(FALSE))
{
  DLPASSERT(m_idTeq);                                                           // Need token execution queue
  INT32 nOffset = m_nTeqOffset+1;                                                // Current offset from program pointer

  // Pump tokens till we have enough                                            // ------------------------------------
  while (nOffset>=m_idTeq->GetNRecs())                                          // Offset greater than exec. queue ?
    IF_NOK(PumpToken(TRUE))                                                     //   Pump another token; failed?
      break;                                                                    //     Forget it...
  if (nOffset>=m_idTeq->GetNRecs()) return NULL;                                // Did we pump enough tokens?

  // Check same instruction                                                     // ------------------------------------
  if (bSameInstr)                                                               // Only tokens of same "instruction" ?
  {                                                                             // >>
    if (__TTYP_IS_EX(m_idTeq,nOffset,TT_DEL            )) return NULL;          //   Delimiter token         --> beyond
    if (dlp_strcnt(__IDEL_EX(m_idTeq,m_nTeqOffset),'\n')) return NULL;          //   Line break before token --> beyond
  }                                                                             // <<

  // Ok - adjust token queue offset and return token text                       // ------------------------------------
  m_nTeqOffset = nOffset;                                                       // Remember token offset
  FNC_MSG(2,"  - Postfix argument #%ld \"%s\"",(long)m_nTeqOffset,              // Protocol on verbose level 2
    __TOK_EX(m_idTeq,m_nTeqOffset),0,0,0);                                      // |
  return __TOK_EX(m_idTeq,m_nTeqOffset);                                        // Return the token
}

/**
 * For postfix syntax: retrieves the string of insignificant token delimiters
 * following the token most recently obtained by GetNextToken.
 *
 * @return Pointer to the delimiter string or <code>NULL</code> if no such
 * string exists.
 */
const char* CGEN_PUBLIC CFunction::GetNextTokenDel()
{
  DLPASSERT(m_idTeq);
  if (m_nTeqOffset<0 || m_nTeqOffset>=m_idTeq->GetNRecs()) return NULL;
  return __IDEL_EX(m_idTeq,m_nTeqOffset);
}

/**
 * Pushes an extra token fetched by <code>GetNextToken</code> back to the
 * execution queue.
 *
 * @return <code>O_K</code> if successfull, a (negative) error code otherwise.
 */
INT16 CGEN_PUBLIC CFunction::RefuseToken()
{
  if (m_nTeqOffset<0)
  {
    IERROR(this,FNC_INTERNAL,"No tokens to be refused.",__FILE__,__LINE__);
    m_nTeqOffset = 0;
    return NOT_EXEC;
  }

  m_nTeqOffset--;
  return O_K;
}

/**
 * Cleans the head of the token execution queue. The method removes the head
 * plus {@link teq_offset} token(s) from the {@link teq token execution queue).
 * Called by {@link ExecuteToken} after having processed one token (which might
 * have caused calls to {@link GetNextToken} thus consuming further tokens).
 *
 * @return <code>O_K</code> if successfull, a (negative) error code otherwise.
 */
INT16 CGEN_PROTECTED CFunction::TokenProcessed()
{
  INT16 nErr = O_K;
  DLPASSERT(m_idTeq);

  // Clean up token execution buffer
  nErr = m_idTeq->DeleteRecs(0,m_nTeqOffset+1);
  m_nTeqOffset=0;
  return nErr;
}

/**
 * EXPERIMENTAL FEATURE!
 */
void CGEN_PROTECTED CFunction::EmptyUserInput(char* lpsInput)
{
  if (dlp_strlen(lpsInput))
  {
    dlp_strcpy(__CFunction_EmptyUserInput,lpsInput);
    return;
  }
  if ((dlp_strcmp(__CFunction_EmptyUserInput,"step")==0))
    dlp_strcpy(lpsInput,"step");
  else if (dlp_strcmp(__CFunction_EmptyUserInput,"cont")==0)
    dlp_strcpy(lpsInput,"cont");
}

// EOF
