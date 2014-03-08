/* dLabPro class CData (data)
 * - Content initialization
 *
 * AUTHOR : Matthias Wolff
 * PACKAGE: dLabPro/classes
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

#include "dlp_cscope.h" /* Indicate C scope */
#include "dlp_data.h"

/**
 * Initializes the instance from the interpreter command line.
 * <h3>Note</h3>
 * <p>You must setup the method invocation context <code>m_lpMic</code> (see 
 * <a href="dlpinst.html"><code class="link">CDlpInstance</code></a>) before
 * calling this method. Otherwise it will do noting.</p>
 * 
 * @param _this  This instance
 * @param nRec   First record to read
 * @param nComp  First component to read
 * @param nCount Number of items to read
 * @return O_K if successfull, an error code otherwise
 */
INT16 CGEN_PUBLIC CData_InitializeEx(CData* _this, INT32 nRec, INT32 nComp, INT32 nCount)
{
  INT32        nXXR     = 0;
  INT32        nXC      = 0;
  INT32        nR       = 0;
  INT32        nC       = 0;
  INT32        nCtr     = 0;
  BOOL        bRead    = 0;
  const char* lpsToken = NULL;

  if (_this && !CDlpObject_MicGet(BASEINST(_this)))
    return IERROR(_this,ERR_GENERIC,"No method invocation context",0,0);

  if (!_this || CData_IsEmpty(_this))
  {
    /* _this == NULL is ok, just remove the tokens between { and } */
    do
    {
      lpsToken = MIC_NEXTTOKEN_FORCE;
      if (!lpsToken) return IERROR(_this,DATA_HERESCAN,"}",0,0);
    } 
    while (dlp_strcmp(lpsToken,"}")!=0);
    return O_K;
  }

  /* Actual setup */
  nXXR  = CData_GetMaxRecs(_this);
  nXC   = CData_GetNComps(_this);
  nR    = nRec;
  nC    = nComp;
  bRead = TRUE;

  for (;;)
  {
    lpsToken = MIC_NEXTTOKEN_FORCE;
    if (!lpsToken) return IERROR(_this,DATA_HERESCAN,"}",0,0);
    if (dlp_strcmp(lpsToken,"}")==0) break;
    if (!bRead) continue;

    if (nR==nXXR || (nCount>=0 && nCtr>=nCount))
    {
      /* Ignore further initializers */
      bRead = FALSE;
      continue;
    }

    /* Initialize cell value; skip cell on wildcard '*' initializer */
    if (dlp_strcmp(lpsToken,"*")!=0)
      IF_NOK(dlp_sscanx(lpsToken,CData_GetCompType(_this,nC),CData_XAddr(_this,nR,nC)))
        IERROR(_this,DATA_BADINITIALIZER,lpsToken,(int)nR,(int)nC);

    nCtr++;
    nC++;
    if (nC==nXC) { nC=nComp; nR++; }
  } 

  if (nCount>=0&&nCtr<nCount) return IERROR(_this,DATA_INITIALIZERS,"few" ,0,0);
  if (!bRead                ) return IERROR(_this,DATA_INITIALIZERS,"many",0,0);
  return O_K;
}

/**
 * Get data initializer '{ ... }' from command line and store into token list
 * lpsInit.
 * <h3>Note</h3>
 * <p>You must setup the method invocation context <code>m_lpMic</code> (see 
 * <a href="dlpinst.html"><code class="link">CDlpInstance</code></a>) before
 * calling this method. Otherwise it will do noting.</p>
 *
 * @param lpsInit Pointer to character buffer to store initializer in. The
 *                tokens will be separated by nulls '\0', opening and closing
 *                curly braces will not be included! The token list will be
 *                terminated with a double null "\0\0".
 * @param nLen    Maximal number of characters to place on lpsInit (including
 *                the terminal double null)
 * @param bForce  If TRUE the method does not seek for the opening curly brace
 *                but starts reading initializers until it encounters a closing
 *                curly brace '}'. A leading opening curly brace will, however,
 *                be tolerated. If FALSE the method <i>expects</i> an opening
 *                curly brace and does not read any initializers if no leading
 *                opening brace is found.
 * @return The number of initializers read
 */
INT32 CGEN_PROTECTED CData_ReadInitializer
(
  CData* _this,
  char*  lpsInit,
  INT32   nLen,
  BOOL   bForce
)
{
  const char* lpsToken;                                                         /* Current token                     */
  INT32        nCnt;                                                             /* Token counter                     */
  INT32        nPos;                                                             /* Position in output buffer         */
  BOOL        bStor;                                                            /* Store next token                  */

  /* Initialize */                                                              /* --------------------------------- */
  if (!CDlpObject_MicGet(BASEINST(_this))) return 0;                            /* Need invocation context           */
  if (!lpsInit) return 0;                                                       /* Need output buffer                */
  dlp_memset(lpsInit,0,nLen);                                                   /* Clear output buffer               */

  /* Do initializers follow? */                                                 /* --------------------------------- */
  lpsToken = MIC_NEXTTOKEN_FORCE;                                                /* Ruthlessly get next token         */
  if (!lpsToken || dlp_strcmp(lpsToken,"{")!=0)                                 /* No or not the "{" token           */
  {                                                                             /* >>                                */
    if (bForce) IERROR(_this,DATA_NOTFOUND,"Constant initializer ","'{'",0);    /*   If there should be some-> error */
    if (lpsToken) MIC_REFUSETOKEN;                                              /*   Wrong token -> push it back     */
    return 0;                                                                   /*   Now get out'a here              */
  }                                                                             /* <<                                */

  /* Get 'em tokens */                                                          /* --------------------------------- */
  for (nCnt=0,nPos=0,bStor=TRUE; ; nCnt++)                                      /* Infinitely ...                    */
  {                                                                             /* >>                                */
    lpsToken = MIC_NEXTTOKEN_FORCE;                                             /*   Ruthlessly get next token       */
    if (!lpsToken) return IERROR(_this,DATA_HERESCAN,"}",0,0);                  /*   No more tokens -> forget it     */
    if (dlp_strcmp(lpsToken,"}")==0) break;                                     /*   End token -> get out'a here     */

    if (nLen<nPos+(INT32)dlp_strlen(lpsToken)+2) bStor = FALSE;                  /*   Cannot store more tokens :(     */
    if (!bStor) continue;                                                       /*   Ignore ALL following tokens     */

    dlp_strcpy(&lpsInit[nPos],lpsToken);                                        /*   Store token                     */
    nPos+=(INT32)dlp_strlen(&lpsInit[nPos])+1;                                   /*   Set next token pointer          */
  }                                                                             /* <<                                */

  return nCnt;                                                                  /* Return number of tokens read      */
}

/**
 * Initialize one record from a token list.
 *
 * @param lpsInit Null ('\0') separated list of tokens. A double null "\0\0"
 *                is expected as list terminator!
 * @param nRec    Record index of first cell to initialize
 * @param nComp   Component index of first cell to initialize
 * @return O_K if successfull, a negative error code otherwise
 */
INT16 CGEN_PUBLIC CData_InitializeRecordEx
(
  CData*      _this,
  const char* lpsInit,
  INT32        nRec,
  INT32        nComp
)
{
  const char* lpsToken = NULL;
  INT32  nXC            = 0;
  INT32  nC             = 0;

  nXC = CData_GetNComps(_this);
  nC  = nComp;

  if (nC<0   || nC>=nXC                    ) return NOT_EXEC;
  if (nRec<0 || nRec>=CData_GetNRecs(_this)) return NOT_EXEC;

  for (lpsToken=lpsInit; *lpsToken && nC<nXC; lpsToken+=dlp_strlen(lpsToken)+1, nC++)
    if (dlp_strcmp(lpsToken,"*")!=0)
      IF_NOK(dlp_sscanx(lpsToken,CData_GetCompType(_this,nC),CData_XAddr(_this,nRec,nC)))
        IERROR(_this,DATA_BADINITIALIZER,lpsToken,(int)nRec,(int)nC);

  if (*lpsToken) return IERROR(_this,DATA_INITIALIZERS,"many",0,0);
  if (nC<nXC   ) IERROR(_this,DATA_INITIALIZERS,"few" ,0,0);
  return O_K;
}

/*
 * Manual page in data.def
 */
INT16 CGEN_PUBLIC CData_Initialize(CData* _this)
{
  const char* lpsToken = NULL;
  
  lpsToken = MIC_NEXTTOKEN_FORCE;
  if (!lpsToken) return 0;
  if (dlp_strcmp(lpsToken,"{")!=0)
  {
    MIC_REFUSETOKEN;
    return IERROR(_this,DATA_NOTFOUND,"Constant initializer ","'{'",0); 
  }
  return CData_InitializeEx(_this,0,0,-1);
}

/*
 * Manual page in data.def
 */
INT16 CGEN_PUBLIC CData_InitializeRecord(CData* _this, INT32 nRec)
{
  char lpsInit[L_INPUTLINE];

  if (!CData_ReadInitializer(_this,lpsInit,L_INPUTLINE,TRUE)) return NOT_EXEC;
  return CData_InitializeRecordEx(_this,lpsInit,nRec,0);
}

/* EOF */
