// dLabPro class CFunction (function)
// - Argument list
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

#include "dlp_var.h"
#include "dlp_function.h"

/**
 * <p>Creates or clears the function's argument table. The method frees all
 * memory associated with the present argument table and creates a new table
 * with the following component structure:</p>
 * <table>
 *   <tr><th>No.</th><th>Name</th><th>Type</th><th>Description</th></tr>
 *   <tr><td>0</td><td>ID</td><td><code>L_NAMES</code> (symbolic)</td>
 *     <td>Identifier of (formal) argument</td></tr>
 *   <tr><td>1</td><td>TYPE</td><td><code>short</code></td>
 *     <td>formal argument: <code>T_INSTANCE</code><br>
 *         command line argument: <code>T_STRING</code></td></tr>
 *   <tr><td>2</td><td>PTR</td><td><code>T_ULONG</code></td>
 *     <td>formal argument: pointer to variable instance<br>
 *         command line argument: pointer to string</td></tr>
 * </table>
 */
INT16 CGEN_PRIVATE CFunction::ArgInit()
{
  // Free memory associated with arguments
  ArgClear();

  // Reset (or create) argument table
  IFIELD_RESET(CData,"arg");
  m_idArg->AddComp("ID"  ,L_NAMES);
  m_idArg->AddComp("TYPE",T_SHORT);
  m_idArg->AddComp("PTR" ,T_PTR);
  return O_K;
}

/**
 * Clears actual arguments associated with formal arguments. The method frees
 * all memory associated with the current <em>actual</em> arguments.
 */
void CGEN_PRIVATE CFunction::ArgClear()
{
  if (!m_idArg) return;

  for (INT32 nArg=0; nArg<m_idArg->GetNRecs(); nArg++)
    if ((INT16)m_idArg->Dfetch(nArg,FNC_ALIC_TYPE)==T_STRING)
    {
      __dlp_free(*(BYTE**)m_idArg->Pfetch(nArg,FNC_ALIC_PTR));
      m_idArg->Pstore(NULL,nArg,FNC_ALIC_PTR);
    }
}

/**
 * Destroys the function's argument table and frees all associated memory.
 */
void CGEN_PRIVATE CFunction::ArgDestroy()
{
  if (!m_idArg) return;
  ArgClear();
  IDESTROY(m_idArg);
  m_idArg = NULL;
}

/**
 * Initializes the function argument list from a command line
 */
INT16 CGEN_PUBLIC CFunction::ArgCmdline(INT32 argc, char** argv)
{
  ArgInit();                                                                    // Zero-initialize argument list
  for (INT32 nArg=0; nArg<argc; nArg++)
  {
    char* lpsArg     = (char*)dlp_malloc((dlp_strlen(argv[nArg])+1)*sizeof(char));
    char  lpsNam[16];
    dlp_strcpy(lpsArg,argv[nArg]);
    sprintf(lpsNam,"$%d",(int)(nArg+1));
    m_idArg->AddRecs(1,10);
    m_idArg->Sstore(lpsNam,nArg,0);
    m_idArg->Dstore(T_STRING,nArg,1);
    m_idArg->Pstore(lpsArg,nArg,2);
  }
  return O_K;
}

/**
 * Pops the actual values for this function's formal arguments from the calling
 * function's stack.
 * 
 * @return <code>O_K</code> if successfull, a (negative) error code otherwise
 */
INT16 CGEN_PROTECTED CFunction::ArgCommit()
{
  // Validate                                                                   // ------------------------------------
  DLPASSERT(m_idArg);                                                           // Need argument list

  // Check if there is a caller and if it is a function instance                // ------------------------------------
  CFunction* iCaller = GetCaller();                                             // Get calling function
  if (!iCaller)                                                                 // None?
  {                                                                             // >>
    return NOT_EXEC;                                                            //   Why don't you go outside and
  }                                                                             // <<  play hide and go ....

  // Initialize                                                                 // ------------------------------------
  IFCHECKEX(1)                                                                  // On verbose level 1
  {                                                                             // >>
    printf("\n Committing actual arguments");                                   //   ...
    printf("\n   from function %s",iCaller->GetFQName(dlp_get_a_buffer()));     //   ...
    printf("\n   to   function %s",GetFQName(dlp_get_a_buffer()));              //   ...
  }                                                                             // <<
  iCaller->Pop();                                                               // Remove this instance from caller stk
  if (iCaller->m_iAi==this)  iCaller->m_iAi = NULL;                              // Deactivate
  if (m_idArg->GetNRecs()==0) return O_K;                                       // Nothing to be done
  if (m_idArg->GetNRecs()>iCaller->m_nStackLen)                                 // Not enough arguments
  {                                                                             // >>
    char sBuf[L_SSTR]; sprintf(sBuf," (expect %ld args)",(long)m_idArg->GetNRecs());  //   Make extra error info
    return IERROR(this,FNC_STACKUNDERFLOW," on function call",sBuf,0);          //   Stack underflow error
  }                                                                             // <<

  // Go get actual arguments (in reverse order)                                 // ------------------------------------
  for (INT32 nArg=m_idArg->GetNRecs()-1; nArg>=0; nArg--)                        // Loop over formal arguments
  {                                                                             // >>
    // Create variable instance for  argument                                   //   - - - - - - - - - - - - - - - - - 
    CVar*       iArgVar  = NULL;                                                //   Variable instance for argument
    const char* lpsArgId = (const char*)m_idArg->XAddr(nArg,0);                 //   Get argument(=variable) identifier
    DLPASSERT(dlp_strlen(lpsArgId));                                            //   Handle error on parsing arg. list!
    IFCHECKEX(2) printf("\n   %02ld %-20s:",nArg,lpsArgId);                     //   Protocol (verbose lebel 2)
    ICREATEEX(CVar,iArgVar,lpsArgId,NULL);                                      //   Create variable
    m_idArg->Dstore(T_INSTANCE            ,nArg,FNC_ALIC_TYPE);                 //   Store argument type in arg. list
    m_idArg->Pstore(iArgVar,nArg,FNC_ALIC_PTR );                                //   Store variable ptr in arg. list

    // Get actual value                                                         //   - - - - - - - - - - - - - - - - - 
    if (iCaller->m_nStackLen>0)                                                 //   Pop item from caller's stack
    {                                                                           //   >>
      switch(iCaller->m_aStack[0].nType)                                        //     Depending in stk. item type
      {                                                                         //     >>
        case T_BOOL:                                                            //       Logic
          iArgVar->Bset(iCaller->m_aStack[0].val.b);                            //         Set variable
          IFCHECKEX(2)printf(" B %s",iCaller->m_aStack[0].val.b?"TRUE":"FALSE");//         Protocol (verbose level 2)
          break;                                                                //         .
        case T_COMPLEX:                                                         //       Numeric
          iArgVar->Vset(iCaller->m_aStack[0].val.n);                            //         Set variable
          IFCHECKEX(2) printf(" N %lg+%lg",iCaller->m_aStack[0].val.n.x,        //         Protocol (verbose level 2)
              iCaller->m_aStack[0].val.n.y);                                    //         |
          break;                                                                //         .
        case T_STRING:                                                          //       String
          iArgVar->Sset(iCaller->m_aStack[0].val.s);                            //         Set variable
          IFCHECKEX(2) printf(" S \"%s\"",iCaller->m_aStack[0].val.s);          //         Protocol (verbose level 2)
          break;                                                                //         .
        case T_INSTANCE:                                                        //       Instance
          iArgVar->Iset(iCaller->m_aStack[0].val.i);                            //         Set variable
          IFCHECKEX(2)                                                          //         On verbose level 2
            printf(" I %s",CDlpObject_GetFQName(iCaller->m_aStack[0].val.i,     //           Protocol
              dlp_get_a_buffer(),FALSE));                                       //           |
          break;                                                                //         .
        default:                                                                //       Unknown type
          DLPASSERT(FMSG("Unknown stack item type"));                           //         Assertion failure
      }                                                                         //     <<
      iCaller->Pop();                                                           //     Remove stack top
    }                                                                           //   <<

    // Register with function's dictionary                                      //   - - - - - - - - - - - - - - - - - 
    // FIXME: This code is a partial duplicate of CFunction::ItpAsWord          //   FIXME
    SWord newword; memset(&newword,0,sizeof(SWord));                            //   Create new dictionary entry
    newword.nWordType = WL_TYPE_INSTANCE;                                       //   ... of type instance
    newword.lpData    = iArgVar;                                                //   ... denoting argument variable
    strcpy(newword.lpName,lpsArgId);                                            //   ... named after the argument
    RegisterWord(&newword);                                                     //   Register new dictionary entry
  }                                                                             // <<
  
  return O_K;                                                                   // Ok
}

/**
 * Checks if an instance is referrable from a calling function independently
 * of this function instance.
 * 
 * @param iInst   Pointer to instance to be checked
 * @param iCaller Pointer to calling function instance to check reference for
 * @return <code>TRUE</FALSE> if <code>iInst</code> is referrable from
 *         <code>iCaller</code> (except thorugh <code>this</code>-instance),
 *         <code>FALSE</code> otherwise
 * @see ArgReturnVal
 */
BOOL CGEN_PRIVATE CFunction::ArgCheckInstanceRef
(
  CDlpObject* iInst,
  CFunction*  iCaller
)
{
  while (iInst)                                                                 // Traverse nested instances
  {                                                                             // >>
    if (iInst==this          ) return FALSE;                                    //   Ref. only thru this   --> negative
    if (iInst==iCaller       ) return TRUE;                                     //   Ref. except thru this --> positive
    if (!iInst->m_lpContainer) return TRUE;                                     //   No more parents       --> "root.x"
    iInst = iInst->m_lpContainer->lpContainer;                                  //   Continue with parent instance
  }                                                                             // <<
  return FALSE;                                                                 // Ain't look good
}

/*
 * Pops the top of this functions stack and pushes it to the calling function's
 * stack.
 * 
 * @return <code>O_K</code> if successfull, a (negative) error code otherwise
 */
INT16 CGEN_PROTECTED CFunction::ArgReturnVal()
{
  CFunction* iCaller = GetCaller();                                             // Get calling function
  if (!iCaller)                                                                 // Root function (main program)
  {                                                                             // >>
    dlp_set_retval((INT32)PopNumber().x);                                       //   Remember return value
    return O_K;                                                                 //   So far, so good ...
  }                                                                             // <<

  // Initialize                                                                 // ------------------------------------
  char lpsFqid[255];                                                            // Buffer for fully qualified ids.
  char lpsBuf[255];                                                             // Buffer for temporary instance name
  IFCHECKEX(1)                                                                  // On verbose level 1
  {                                                                             // >>
    printf("\n Committing return value");                                       //   ...
    printf("\n   from function %s",GetFQName(lpsFqid));                         //   ...
    printf("\n   to   function %s",iCaller->GetFQName(lpsFqid));                //   ...
  }                                                                             // <<
  
  // Do return value                                                            // ------------------------------------
  if (m_nStackLen>0)                                                            // Stack not empty?
  {                                                                             // >>
    switch(m_aStack[0].nType)                                                   //   Depending in stk. item type
    {                                                                           //   >>
      case T_BOOL    : iCaller->PushLogic (m_aStack[0].val.b);   break;         //     Push boolean stack item
      case T_COMPLEX : iCaller->PushNumber(m_aStack[0].val.n);   break;         //     Push complex numeric stack item
      case T_STRING  : iCaller->PushString(m_aStack[0].val.s);   break;         //     Push string stack item
      case T_INSTANCE: {                                                        //     Instance stack item:
        CDlpObject* iInst = m_aStack[0].val.i;                                  //       Copy instance ptr.(may change)
        if (iInst==NULL) { iCaller->PushInstance(NULL); break; }                //       Push NULL instance                                                                       //      <<
        if (ArgCheckInstanceRef(iInst,iCaller))                                 //       Inst. referrable by caller?
        {                                                                       //       >>
          iCaller->PushInstance(iInst);                                         //         Just push instance
          iCaller->m_iAi = iInst;                                               //         Activate instance
        }                                                                       //       <<
        else                                                                    //       Instance is local
        {                                                                       //       >>
          sprintf(lpsBuf,"#RETVAL#%s",m_lpInstanceName);                        //         Make temp. instance name
          CDlpObject* iTmp = CreateInstanceOf(iInst->m_lpClassName,lpsBuf);     //         Instanciate tmporary oject
          if (!iTmp) return IERROR(this,ERR_NOMEM,0,0,0);                       //         Not successful? --> error
          iTmp->Copy(iInst);                                                    //         Copy contents
          iCaller->PushInstance(iTmp);                                          //         Push copied instance
          iCaller->m_iAi = iTmp;                                                //         Activate instance
        }                                                                       //       <<
        break; }                                                                //       .
      default:                                                                  //     Unknown type
        DLPASSERT(FMSG("Unknown stack item type"));                             //       Assertion failure
        return NOT_EXEC;                                                        //       Return
    }                                                                           //   <<
    Pop();                                                                      //   Remove stack top
  }                                                                             // <<
  else return NOT_EXEC;                                                         // Nothing to return!

  // Final checks                                                               // ------------------------------------
  DLPASSERT(iCaller->m_nStackLen>0);
  IFCHECKEX(1)                                                                  // On verbose level 1
    switch(iCaller->m_aStack[0].nType)                                          //   Branch for type of clr.'s stk. top
    {                                                                           //   >>
      case T_BOOL:
        printf("\n   B %s",iCaller->m_aStack[0].val.b?"TRUE":"FALSE");
        break;
      case T_COMPLEX:
        printf("\n   N %lg+%lgi",(double)iCaller->m_aStack[0].val.n.x, (double)iCaller->m_aStack[0].val.n.y);
        break;
      case T_STRING:
        printf("\n   S \"%s\"",iCaller->m_aStack[0].val.s);
        break;
      case T_INSTANCE: {
        char lpFqid[255];
        CDlpObject_GetFQName(iCaller->m_aStack[0].val.i,lpFqid,FALSE);
        printf("\n   I %s",lpFqid);
        break; }
      default: DLPASSERT(FMSG("Unknown stack item type"));
    }                                                                           //   <<

  return O_K;                                                                   // Done.
}

/**
 * Get signature from raw instance name. The method is called during instance
 * initialization to obtain the bare function identifier and the function's
 * argument list (field {@link arg}) from the raw identifier (including the
 * formal argument list in braces). Note: this method changes the field
 * <code>m_lpInstanceName</code>.
 */
INT16 CGEN_PRIVATE CFunction::ArgParse()
{
  char* tx        = m_lpInstanceName;
  char* lpsArgs   = NULL;
  BOOL  bBraceOff = FALSE;

  ArgInit();                                                                    // Zero-initialize argument list

  // Validation
  DLPASSERT(m_idArg     );
  DLPASSERT(L_NAMES<=256);

  // Seek formal argument list (opening brace)
  for (; *tx; tx++)
    if (*tx=='(')
    {
      *tx++   = '\0';
      lpsArgs = tx;
      break;
    }
  if (!lpsArgs || !dlp_strlen(lpsArgs)) return O_K;                             // No argument list; ok

  // Seek end of formal argument list (closing brace)
  for (tx=lpsArgs,bBraceOff=FALSE; *tx; tx++)
    if (*tx==')')
    {
      bBraceOff = TRUE;
      *tx++ = '\0';
      if (*tx) IERROR(this,FNC_EXTRACHARS,")","function signature",0);
      break;
    }
  if (!bBraceOff) return IERROR(this,FNC_EXPECT,")",0,0);
  if (!lpsArgs || !dlp_strlen(lpsArgs)) return O_K;                             // Empty argument list; ok

  // Tokenize argument list
  tx = strtok(lpsArgs,",");
  while (tx)
  {
    if (dlp_strlen(tx))
    {
      INT32 nArg = m_idArg->AddRecs(1,10);
      m_idArg->Sstore(tx,nArg,0);
      m_idArg->Dstore(T_INSTANCE,nArg,1);
    }
    tx = strtok(NULL,",");
  }
  return O_K;
}

// EOF
