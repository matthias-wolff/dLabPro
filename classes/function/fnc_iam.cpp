// dLabPro class CFunction (function)
// - Interactive methods
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

/*
 * Manual page at function.def
 */
INT16 CGEN_PUBLIC CFunction::OnGet()
{
  // Delegate to running function                                               // ------------------------------------
  FNC_DELEGATE OnGet();                                                         // Use a weird macro (see function.def)

  // Initialize                                                                 // ------------------------------------
  CDlpObject* iCont = GetActiveInstance();                                      // Determine field container
  const char* lpsId = GetNextToken(TRUE);                                       // Determine field name

  // Validate                                                                   // ------------------------------------
  DLPASSERT(iCont);                                                             // Check set target
  if (!dlp_strlen(lpsId))                                                       // If no field name committed
    return IERROR(this,FNC_EXPECT,"field identifier after -get",0,0);           //   Error
  SWord* lpWrd = iCont->FindWord(lpsId,WL_TYPE_FIELD);                          // Find field in container
  if (!lpWrd)                                                                   // If not found
  {                                                                             // >>
    iCont = this;                                                               //   Use this instance as container
    lpWrd = FindWord(lpsId,WL_TYPE_FIELD);                                      //   And seek again
  }                                                                             // <<
  if (!lpWrd) return IERROR(this,ERR_NOTFIELD,lpsId,0,0);                       // If still not found --> Error

  // Push field value                                                           // ------------------------------------
  switch (lpWrd->ex.fld.nType)                                                  // Branch for field variable type
  {                                                                             // >>
    case T_BOOL    : { PushLogic   (      *(     BOOL*)lpWrd->lpData);  break; }// - Boolean
    case T_UCHAR   : { PushNumber  (CMPLX(*(    UINT8*)lpWrd->lpData)); break; }// - Unsigned character
    case T_CHAR    : { PushNumber  (CMPLX(*(     INT8*)lpWrd->lpData)); break; }// - Signed character
    case T_USHORT  : { PushNumber  (CMPLX(*(   UINT16*)lpWrd->lpData)); break; }// - Unsigned short integer
    case T_SHORT   : { PushNumber  (CMPLX(*(    INT16*)lpWrd->lpData)); break; }// - Signed short integer
    case T_UINT    : { PushNumber  (CMPLX(*(   UINT32*)lpWrd->lpData)); break; }// - Unsigned integer
    case T_INT     : { PushNumber  (CMPLX(*(    INT32*)lpWrd->lpData)); break; }// - Signed integer
    case T_ULONG   : { PushNumber  (CMPLX(*(   UINT64*)lpWrd->lpData)); break; }// - Unsigned long integer
    case T_LONG    : { PushNumber  (CMPLX(*(    INT64*)lpWrd->lpData)); break; }// - Signed long integer
    case T_FLOAT   : { PushNumber  (CMPLX(*(  FLOAT32*)lpWrd->lpData)); break; }// - Single precision floating point
    case T_DOUBLE  : { PushNumber  (CMPLX(*(  FLOAT64*)lpWrd->lpData)); break; }// - Double precision floating point
    case T_COMPLEX : { PushNumber  (      *(COMPLEX64*)lpWrd->lpData);  break; }// - Double precision complex floating point
    case T_INSTANCE: { PushInstance(*(CDlpObject**) lpWrd->lpData);     break; }// - Instance
    case T_TEXT    : /* Fall through */                                         // - Text (deprecated type!)
    case T_CSTRING : /* Fall through */                                         // - Constant string
    case T_STRING  : { PushString(*(char**)         lpWrd->lpData); break;     }// - String
    default        : {                                                          // - Other types
      if (lpWrd->ex.fld.nType > 0 && lpWrd->ex.fld.nType <= 256)                //     Character array?
        PushString((char*)lpWrd->lpData);                                       //       Push value
      else                                                                      //     Type unknown!
        DLPASSERT(FMSG("Unknown field type"));                                  //       Error
    }                                                                           //   <<
  }                                                                             // <<

  return O_K;                                                                   // Done.
}

/*
 * Manual page at function.def
 */
INT16 CGEN_PUBLIC CFunction::OnSet()
{
  // Delegate to running function                                               // ------------------------------------
  FNC_DELEGATE OnSet();                                                         // Use a weird macro (see function.def)

  // Initialize                                                                 // ------------------------------------
  const char* lpsId = GetNextToken(TRUE);                                       // Determine field name
  CDlpObject* iCont = GetActiveInstance();                                      // Get first instance argument
  if (m_nStackLen<=0)                                                           // Stack is to contain the new value
    return IERROR(this,FNC_STACKUNDERFLOW," on method ","-set",0);              // |
  if (m_aStack[0].nType==T_INSTANCE && m_aStack[0].val.i==iCont) PopInstance(1);// Pop the active instance

  // Validate                                                                   // ------------------------------------
  if (!dlp_strlen(lpsId))                                                       // If no field name committed
    return IERROR(this,FNC_EXPECT,"field identifier after -set",0,0);           //   Error
  SWord* lpWrd = iCont->FindWord(lpsId,WL_TYPE_FIELD);                          // Find field in container
  if (!lpWrd)                                                                   // If not found
  {                                                                             // >>
    iCont = this;                                                               //   Use this instance as container
    lpWrd = FindWord(lpsId,WL_TYPE_FIELD);                                      //   And seek again
  }                                                                             // <<
  if (!lpWrd                  ) return IERROR(this,ERR_NOTFIELD,lpsId,0,0);     // If still not found --> error
  if (lpWrd->nFlags & FF_NOSET) return IERROR(this,FNC_NOSET   ,lpsId,0,0);     // If write-protected --> error

  // Set new value                                                              // ------------------------------------
  switch (lpWrd->ex.fld.nType)                                                  // Branch for field variable type
  {                                                                             // >>
    case T_BOOL    : {      BOOL bTmp=(   BOOL)PopLogic(2);   iCont->SetField(lpWrd,(void*)&bTmp); } break;// - Boolean
    case T_UCHAR   : {     UINT8 nTmp=(  UINT8)PopNumber(2).x;iCont->SetField(lpWrd,(void*)&nTmp); } break;// - Unsigned character
    case T_CHAR    : {      INT8 nTmp=(   INT8)PopNumber(2).x;iCont->SetField(lpWrd,(void*)&nTmp); } break;// - Signed character
    case T_USHORT  : {    UINT16 nTmp=( UINT16)PopNumber(2).x;iCont->SetField(lpWrd,(void*)&nTmp); } break;// - Unsigned short integer
    case T_SHORT   : {     INT16 nTmp=(  INT16)PopNumber(2).x;iCont->SetField(lpWrd,(void*)&nTmp); } break;// - Signed short integer
    case T_UINT    : {    UINT32 nTmp=( UINT32)PopNumber(2).x;iCont->SetField(lpWrd,(void*)&nTmp); } break;// - Unsigned integer
    case T_INT     : {     INT32 nTmp=(  INT32)PopNumber(2).x;iCont->SetField(lpWrd,(void*)&nTmp); } break;// - Signed integer
    case T_ULONG   : {    UINT64 nTmp=( UINT64)PopNumber(2).x;iCont->SetField(lpWrd,(void*)&nTmp); } break;// - Unsigned long integer
    case T_LONG    : {     INT64 nTmp=(  INT64)PopNumber(2).x;iCont->SetField(lpWrd,(void*)&nTmp); } break;// - Signed long integer
    case T_FLOAT   : {   FLOAT32 nTmp=(FLOAT32)PopNumber(2).x;iCont->SetField(lpWrd,(void*)&nTmp); } break;// - Single precision floating point
    case T_DOUBLE  : {   FLOAT64 nTmp=(FLOAT64)PopNumber(2).x;iCont->SetField(lpWrd,(void*)&nTmp); } break;// - Double precision floating point
    case T_COMPLEX : { COMPLEX64 nTmp=         PopNumber(2);  iCont->SetField(lpWrd,(void*)&nTmp); } break;// - Double precision complex floating point
    case T_TEXT    : /* Fall through */                                                                    // - Text (depreciated type!)
    case T_CSTRING : /* Fall through */                                                                    // - Constant string
    case T_STRING  : {   char* lpsTmp =        PopString(2);  iCont->SetField(lpWrd,(void*)&lpsTmp);}break;// - String
    case T_INSTANCE:                                                            // - Instance
    {                                                                           //   >>
      CDlpObject* iVal = PopInstance(2);                                        //     Get new value
      if                                                                        //     Check instance type
      (                                                                         //     |
        dlp_strlen(lpWrd->ex.fld.lpType)>0     &&                               //     - Typed instance field?
        iVal != NULL                           &&                               //     - New value non-NULL?
        OfKind(lpWrd->ex.fld.lpType,iVal)==NULL                                 //     - Type cast impossible?
      )                                                                         //     |
      {                                                                         //     >>
        return                                                                  //       Error
          IERROR(this,FNC_TYPECAST,0,iVal->m_lpClassName,lpWrd->ex.fld.lpType); //       |
      }                                                                         //     <<
      iCont->SetField(lpWrd,(void*)&iVal);                                      //     Set new value
      break;                                                                    //     .
    }                                                                           //   <<
    default:                                                                    // - Other types
      if (lpWrd->ex.fld.nType>0 && lpWrd->ex.fld.nType<=256) {                  //     Character array?
        char* lpsTmp=PopString(2);                                              //       Set new value
        iCont->SetField(lpWrd,&lpsTmp);                                         //       |
      } else                                                                    //     Type unknown!
        DLPASSERT(FMSG("Unknown field type"));                                  //       Error
  }                                                                             // <<

  return O_K;                                                                   // Done.
}

/*
 * Manual page at function.def
 */
INT16 CGEN_PUBLIC CFunction::OnSee()
{
  // Delegate to running function                                               // ------------------------------------
  FNC_DELEGATE OnSee();                                                         // Use a weird macro (see function.def)

  // Initialize                                                                 // ------------------------------------
  CDlpObject* iCont = GetActiveInstance();                                      // Determine field container
  const char*   lpsId = GetNextToken(TRUE);                                     // Determine field name

  // Validate                                                                   // ------------------------------------
  DLPASSERT(iCont);                                                             // Check set target
  if (!dlp_strlen(lpsId))                                                       // If no field name committed
    return IERROR(this,FNC_EXPECT,"field identifier or * after -see",0,0);      //   Error

  // Print                                                                      // ------------------------------------
  return CDlpObject_PrintField(iCont,lpsId,FALSE);                              // Print selected field(s)
}

/*
 * Manual page at function.def
 */
INT16 CGEN_PUBLIC CFunction::OnReset()
{
  // Delegate to running function                                               // ------------------------------------
  FNC_DELEGATE OnReset();                                                       // Use a weird macro (see function.def)

  SWord*   lpWrd = NULL;
  hscan_t  hs;
  hnode_t* hn;

  // Initialize                                                                 // ------------------------------------
  CDlpObject* iCont = GetActiveInstance();                                      // Determine field container
  if (iCont==this) return NOT_EXEC;                                             // Never ever!!
  const char* lpsId = GetNextToken(TRUE);                                       // Determine field name

  // Validate                                                                   // ------------------------------------
  DLPASSERT(iCont);                                                             // Check set target

  // Do reset                                                                   // ------------------------------------
  if      (!dlp_strlen(lpsId)) iCont->Reset();                                  // Reset entire instance
  else if (!dlp_strcmp(lpsId,"*"))                                              // Reset all (writable) fields
  {                                                                             // >>
    hash_scan_begin(&hs,iCont->m_lpDictionary);                                 //   Initialize enumerating dictionary
    while ((hn=hash_scan_next(&hs)))                                            //   Loop over all dictionary entries
    {                                                                           //   >>
      DLPASSERT((lpWrd=(SWord*)hnode_get(hn)));                                 //     NULL entry in dictionary
      if (lpWrd->nWordType==WL_TYPE_FIELD && !(lpWrd->nFlags & FF_NOSET))       //     Not write-protected
        iCont->ResetField(lpWrd);                                               //       Reset single field
    }                                                                           //   <<
  }                                                                             // <<
  else                                                                          // Reset single field
  {                                                                             // >>
    lpWrd = iCont->FindWord(lpsId,WL_TYPE_FIELD);                               //   Find field
    if (!lpWrd) return IERROR(this,ERR_NOTFIELD,lpsId,0,0);                     //   Not found --> error
    if (!(lpWrd->nFlags & FF_NOSET)) iCont->ResetField(lpWrd);                  //   If not write-protected --> reset
    else return IERROR(this,FNC_NOSET,lpsId,0,0);                               //   If write-protected --> error
  }                                                                             // <<

  return O_K;                                                                   // Done.
}

/*
 * Manual page at function.def
 */
INT16 CGEN_PUBLIC CFunction::OnInternalize()
{
  // Delegate to running function                                               // ------------------------------------
  FNC_DELEGATE OnInternalize();                                                 // Use a weird macro (see function.def)

  // Initialize                                                                 // ------------------------------------
  CDlpObject* iCont = GetActiveInstance();                                      // Determine field container
  const char*   lpsId = GetNextToken(TRUE);                                     // Determine field name

  // Validate                                                                   // ------------------------------------
  DLPASSERT(iCont);                                                             // Check set target
  if (!dlp_strlen(lpsId))                                                       // If no field name committed
    return                                                                      //   Error
      IERROR(this,FNC_EXPECT,"field identifier or * after -internalize",0,0);   //   |

  // Initialize                                                                 // ------------------------------------
  INT16    bAll  = (!dlp_strcmp(lpsId,"*"));                                    // Determine if processing all fields
  SWord*   lpWrd = NULL;                                                        // Pointer to current field word
  hscan_t  hs;                                                                  // Pointer to hash table scan struct
  hnode_t* hn    = NULL;                                                        // Pointer to current hash table node
  hash_scan_begin(&hs,iCont->m_lpDictionary);                                   // Initialize enumerating dictionary

  if (bAll)                                                                     // Internalize all fields?
  {                                                                             // >>
    hn = hash_scan_next(&hs);                                                   //   Get pointer to first word
    DLPASSERT((lpWrd=(SWord*)hnode_get(hn)));                                   //   NULL entry in dictionary
  }                                                                             // <<
  else                                                                          // Internalize single field
  {                                                                             // >>
    lpWrd = iCont->FindWord(lpsId,WL_TYPE_FIELD);                               //   Find field word
    if (!lpWrd) return IERROR(this,ERR_NOTFIELD,lpsId,0,0);                     //   Not found --> error
  }                                                                             // <<

  // Do internalize                                                             // ------------------------------------
  while (lpWrd)                                                                 // Loop over words
  {                                                                             // >>
    if                                                                          //   Word must:
    (                                                                           //   |
      lpWrd->nWordType==WL_TYPE_FIELD &&                                        //   - be field ...
      lpWrd->ex.fld.nType==T_INSTANCE &&                                        //   - ... of instance type
      lpWrd->lpData                                                             //   - have data
    )                                                                           //   |
    {                                                                           //   >>
      CHECK_IPTR(*(CDlpObject**)lpWrd->lpData,lpWrd->ex.fld.nISerialNum);       //     If ptr. invalid, make it NULL
      CDlpObject* iInst = *(CDlpObject**)lpWrd->lpData;                         //     Get pointer to instance
      if (iInst)                                                                //     Pointer is valid
      {                                                                         //     >>
        SWord* lpCcnt = iInst->m_lpContainer;                                   //       Get current instance container
        if (lpCcnt && lpCcnt->nWordType==WL_TYPE_INSTANCE)                      //       If curr. cont. is inst. word
        {                                                                       //       >> (take over ownership)
          if (lpCcnt->lpContainer)                                              //         Current container instance?
            ((CDlpObject*)lpCcnt->lpContainer)->UnregisterWord(lpCcnt,FALSE);   //           Unregister (don't destroy)
          iInst->m_lpContainer = lpWrd;                                         //         Set curr. word as container
        }                                                                       //       <<
      }                                                                         //     <<
    }                                                                           //   <<

    if (!bAll) break;                                                           //   Processing single field: stop
    if (!(hn = hash_scan_next(&hs))) break;                                     //   Get next dictionary entry
    DLPASSERT((lpWrd=(SWord*)hnode_get(hn)));                                   //   NULL entry in dictionary
  }                                                                             // <<

  return O_K;                                                                   // Done.
}

/*
 * Manual page at function.def
 */
INT16 CGEN_PROTECTED CFunction::OnCopy()
{
  FNC_DELEGATE OnCopy();                                                        // Delegate to running function
  CDlpObject* iDst = PopInstance(1);                                            // Get first instance argument
  CDlpObject* iSrc = PopInstance(2);                                            // Get second instance argument
  if (!iDst     ) return NOT_EXEC;                                              // No destination instance, no service
  if (iSrc==iDst) return O_K;                                                   // Instances equal: nothing to be done
  if (!iSrc) { iDst->Reset(); return O_K; }                                     // Source NULL: reset destination
  if (!iSrc->IsKindOf(iDst->m_lpClassName))                                     // Cannot cast src. to dst. type
    return IERROR(this,FNC_TYPECAST,1,iSrc->m_lpClassName,iDst->m_lpClassName); //   Error
  return iDst->Copy(iSrc);                                                      // Do copying
}

/*
 * Manual page at function.def
 */
INT16 CGEN_PROTECTED CFunction::OnSave()
{
  // Delegate to running function                                               // ------------------------------------
  FNC_DELEGATE OnSave();                                                        // Use a weird macro (see function.def)

  // Get arguments                                                              // ------------------------------------
  CDlpObject* iInst = GetActiveInstance();                                      // Get first instance argument
  if (m_nStackLen<=0)                                                           // Stack at least to contain file name
    return IERROR(this,FNC_STACKUNDERFLOW," on method ","-save",0);             //   Not so? Error!
  if (m_aStack[0].nType==T_INSTANCE) PopInstance(1);                            // Pop the active instance
  const char* lpsFilename = PopString(2);                                       // Get the file name

  // Prevent from saving other than the root function                           // ------------------------------------
  if (iInst && iInst->IsKindOf("function") && iInst!=GetRootFnc())              // Saving a function other than root
    return IERROR(this,FNC_NOTALLOWED,"-save","for non-root functions",0);      //   No go!

  // Do save                                                                    // ------------------------------------
  INT16 nFormat = 0;                                                            // The file format
  if (m_bXml) nFormat |= SV_XML;                                                // /xml selected
  if (m_bDn3) nFormat |= SV_DN3;                                                // /dn3 selected
  if (m_bZip) nFormat |= SV_ZIP;                                                // /zip selected
  return CDlpObject_Save(iInst,lpsFilename,nFormat);                            // Save the instance
}

/*
 * Manual page at function.def
 */
INT16 CGEN_PROTECTED CFunction::OnRestore()
{
  // Delegate to running function                                               // ------------------------------------
  FNC_DELEGATE OnRestore();                                                     // Use a weird macro (see function.def)

  // Get arguments                                                              // ------------------------------------
  CDlpObject* iInst = GetActiveInstance();                                      // Get first instance argument
  if (m_nStackLen<=0)                                                           // Stack at least to contain file name
    return IERROR(this,FNC_STACKUNDERFLOW," on method ","-restore",0);          //   Not so? Error!
  if (m_aStack[0].nType==T_INSTANCE) PopInstance(1);                            // Pop the active instance
  const char* lpsFilename = PopString(2);                                       // Get the file name

  // Prevent from restoring other than the root function                        // ------------------------------------
  if (iInst && iInst->IsKindOf("function") && iInst!=GetRootFnc())              // Restoring a function other than root
    return IERROR(this,FNC_NOTALLOWED,"-restore","for non-root functions",0);   //   No go!

  // Do restore                                                                 // ------------------------------------
  INT16 nFormat = 0;                                                            // The file format
  if (m_bXml) nFormat |= SV_XML;                                                // /xml selected
  if (m_bDn3) nFormat |= SV_DN3;                                                // /dn3 selected
  if (m_bZip) nFormat |= SV_ZIP;                                                // /zip selected
  return CDlpObject_Restore(iInst,lpsFilename,nFormat);                         // Restore the instance
}

/*
 * Manual page at function.def
 */
INT16 CGEN_PROTECTED CFunction::OnDestroy()
{
  FNC_DELEGATE OnDestroy();                                                     // Delegate to running function
  return Deinstanciate(GetActiveInstance(),2);                                  // Do deinstanciation
}

/* -- Methods - Help -- */

/*
 * Manual page at function.def
 */
INT16 CGEN_PUBLIC CFunction::OnHelp()
{
  FNC_DELEGATE OnHelp();                                                        // Delegate to running function
  GetActiveInstance()->PrintDictionary(PWL_HELP);                               // Print word list
  return O_K;                                                                   // Ok
}

/*
 * Manual page at function.def
 */
INT16 CGEN_PROTECTED CFunction::OnList()
{
  FNC_DELEGATE OnList();                                                        // Delegate to running function
  if (CDlpObject_OfKind("function",m_iAi))                                      // Is active instance a function
    return ((CFunction*)m_iAi)->List(GetNextToken(TRUE));                       //   Do listing for active function
  return List(GetNextToken(TRUE));                                              // Do the listing
}

/*
 * Manual page at function.def
 */
INT16 CGEN_PUBLIC CFunction::OnExplain()
{
  FNC_DELEGATE OnExplain();                                                     // Delegate to running function
  return Explain(GetNextToken(TRUE));
}

/* -- Methods - Constants -- */
/*
INT16 CGEN_PROTECTED CFunction::OnNull()
{
  FNC_DELEGATE OnNull();
  PushInstance(NULL);
  return O_K;
}

INT16 CGEN_PROTECTED CFunction::OnTrue()
{
  FNC_DELEGATE OnTrue();
  PushLogic(TRUE);
  return O_K;
}

INT16 CGEN_PROTECTED CFunction::OnFalse()
{
  FNC_DELEGATE OnFalse();
  PushLogic(FALSE);
  return O_K;
}

INT16 CGEN_PROTECTED CFunction::OnInf()
{
  FNC_DELEGATE OnInf();
  PushNumber(dlp_scalop(1.,0.,OP_DIV));
  return O_K;
}

INT16 CGEN_PROTECTED CFunction::OnMinf()
{
  FNC_DELEGATE OnMinf();
  PushNumber(-dlp_scalop(1.,0.,OP_DIV));
  return O_K;
}*/

/*
 * Manual page at function.def
 */
INT16 CGEN_PROTECTED CFunction::OnType()
{
  FNC_DELEGATE OnType();
  const char* lpsTypecode = GetNextToken(TRUE);
  INT16       nTypecode   = dlp_get_type_code(lpsTypecode);
  if (nTypecode<=0)
    return IERROR(this,FNC_INVALID,"elementary type name",lpsTypecode,0);
  PushNumber(CMPLX(nTypecode));
  return O_K;
}

/* -- Methods - Miscellaneous -- */

/*
 * Manual page at function.def
 */
INT16 CGEN_VPUBLIC CFunction::Include(const char* lpsFilename)
{
  FNC_DELEGATE Include(lpsFilename);                                            // Use a weird macro (see function.def)
  return IncludeEx(lpsFilename,m_nPp);                                          // Do include script
}

/*
 * Manual page at function.def
 */
const char* CGEN_PROTECTED CFunction::Argv(INT32 nArg)
{
  CData* idArg = NULL;
  idArg = GetRootFnc() ? GetRootFnc()->m_idArg : m_idArg;
  if (nArg<1 || nArg>CData_GetNRecs(idArg)) return NULL;
  if (CData_Dfetch(idArg,nArg-1,FNC_ALIC_TYPE)!=T_STRING) return NULL;
  return *(const char**)CData_Pfetch(idArg,nArg-1,FNC_ALIC_PTR);
}

/*
 * Manual page at function.def
 */
INT16 CGEN_PUBLIC CFunction::Echo(const char* lpsMessage)
{
//  char lpsBuf[L_INPUTLINE];
//  printf("%s",dlp_strconvert(SC_UNESCAPE,lpsBuf,dlp_strcpy(lpsBuf,lpsMessage)));
  printf("%s",lpsMessage);
  return O_K;
}

/*
 * Manual page at function.def
 */
INT16 CGEN_PUBLIC CFunction::Prompt(const char* lpsMessage)
{
  char sBuf[L_SSTR+1];
  printf(lpsMessage); printf("> ");
  if(fgets(sBuf,L_SSTR,stdin) == NULL) return NOT_EXEC;
  for (char* tx = &sBuf[dlp_strlen(sBuf)-1]; *tx; tx--)
    if (*tx=='\n' || *tx=='\r') *tx='\0';
    else break;
  PushString(sBuf);
  return O_K;
}

/*
 * Manual page at function.def
 */
INT16 CGEN_PROTECTED CFunction::Parent()
{
  // Delegate to running function                                               // ------------------------------------
  FNC_DELEGATE Parent();                                                        // Use a weird macro (see function.def)

  // Ascend to container instance of active instance                            // ------------------------------------
  if (m_iAi)                                                                    // Have active instance?
  {                                                                             // >>
    m_iAi     = m_iAi->m_lpContainer ? m_iAi->m_lpContainer->lpContainer : NULL;//   Activate container instance
    m_bAiUsed = FALSE;                                                          //   Declare virginal
  }                                                                             // <<
  else if (m_iAi2)                                                              // Or have active secondary instance?
    m_iAi2 = m_iAi2->m_lpContainer ? m_iAi2->m_lpContainer->lpContainer : NULL; //   Activate container instance

  return O_K;                                                                   // Have done
}

/*
 * Manual page at function.def
 */
INT16 CGEN_PROTECTED CFunction::This()
{
  FNC_DELEGATE This();                                                          // Use a weird macro (see function.def)
  m_iAi     = NULL;                                                             // Clear activate instance
  m_bAiUsed = FALSE;                                                            // Declare virginal
  m_iAi2    = NULL;                                                             // Clear secondary activate instance
  return O_K;                                                                   // Have done
}

/*
 * Manual page at function.def
 */
INT16 CGEN_PUBLIC CFunction::Cd(const char* lpsDir)
{
  if (dlp_chdir(lpsDir,FALSE)==0) return O_K;                                   // Change directory
  return IERROR(this,ERR_CHDIR,lpsDir,0,0);                                     // Failed? --> error message
}

/*
 * Manual page at function.def
 */
const char* CGEN_PUBLIC CFunction::Cwd()
{
  static char cwd[L_PATH];
  if (getcwd(cwd, sizeof(cwd))!=NULL)
  {
    dlp_strreplace(cwd,"\\","/");
    return cwd;
  }
  else
    IERROR(this,ERR_GETCWD,0,0,0);
  return NULL;
}

/*
 * Manual page at function.def
 */
INT32 CGEN_PUBLIC CFunction::System(const char* lpsCmd)
{
  char lpsBuf[L_INPUTLINE+1];                                                   // String manipulation buffer
  return                                                                        // Copy, unescape and execute
    dlp_system(dlp_strconvert(SC_UNESCAPE,lpsBuf,dlp_strcpy(lpsBuf,lpsCmd)));    // |
}

/*
 * Manual page at function.def
 */
BOOL CGEN_PUBLIC CFunction::Platform(const char* lpsPlatformId)
{
  if (dlp_strcmp(lpsPlatformId,"WIN32")==0)
  {
#if (defined __WIN32 && !defined __CYGWIN__)
    return TRUE;
#else
    return FALSE;
#endif
  }
  if (dlp_strcmp(lpsPlatformId,"LINUX")==0)
  {
#ifdef __LINUX
    return TRUE;
#else
    return FALSE;
#endif
  }
  if (dlp_strcmp(lpsPlatformId,"SPARC")==0)
  {
#ifdef __SPARC
    return TRUE;
#else
    return FALSE;
#endif
  }

  // Deprecated platform IDs
  if (dlp_strcmp(lpsPlatformId,"_WINDOWS")==0)
  {
    IERROR(this,ERR_OBSOLETEID,"_WINDOWS","WIN32",0);
#ifdef __WIN32
    return TRUE;
#else
    return FALSE;
#endif
  }
  if (dlp_strcmp(lpsPlatformId,"_LINUX")==0)
  {
    IERROR(this,ERR_OBSOLETEID,"_LINUX","LINUX",0);
#ifdef __LINUX
    return TRUE;
#else
    return FALSE;
#endif
  }

  return FALSE;
}

/*
 * Manual page at function.def
 */
BOOL CGEN_PUBLIC CFunction::IsInstance(const char* sInstanceId, const char* sClassId)
{
  if (!dlp_strlen(sInstanceId)) return FALSE;
  SWord* lpWord = FindWordAi(sInstanceId);
  if (!lpWord || lpWord->nWordType!=WL_TYPE_INSTANCE) return FALSE;
  if (!dlp_strlen(sClassId)) return TRUE;
  return ((CDlpObject*)lpWord->lpData)->IsKindOf(sClassId);
}

// EOF
