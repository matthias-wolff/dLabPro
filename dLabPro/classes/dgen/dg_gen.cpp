// dLabPro class CDgen (DGen)
// - Code generator methods
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

#include "dlp_dgen.h"

/**
 * Adds text to the generated document.
 * 
 * @param lpsText
 *          Pointer to the text to be added
 * @param lpsDel
 *          Pointer to delimiter string to be inserted between text items
 *          (default is "")
 * @see gen field gen
 */
INT16 CGEN_PROTECTED CDgen::AddText
(
  const char* lpsText,
  const char* lpsDel DEFAULT("")
)
{
  if (!lpsText) return (INT16)CData_AddRecs(m_idGen,1,100);                     /* NULL text --> new line            */

  char* lpsBuf = (char*)dlp_malloc((dlp_strlen(lpsText)+1)*sizeof(char));       /* NO RETURNS BEYOND THIS POINT!     */
  dlp_memmove(lpsBuf,lpsText,(dlp_strlen(lpsText)+1)*sizeof(char));
  
  INT32  nRec    = m_idGen->GetNRecs()-1;
  char* lpsDst  = (char*)m_idGen->XAddr(nRec,0);
  char* lpsLine = lpsBuf;
  char* tx      = NULL;
  for (tx=lpsLine; *tx; tx++)
  {
    if (*tx=='\n' || tx[1]=='\0')
    {
      BOOL bEOL = FALSE;
      if (*tx=='\n') { bEOL = TRUE; *tx='\0'; }
      if (nRec<0 || dlp_strlen(lpsDst)+dlp_strlen(lpsLine)>=L_SSTR)
      {
        nRec   = CData_AddRecs(m_idGen,1,100);
        lpsDst = (char*)m_idGen->XAddr(nRec,0);
      }
      if (dlp_strlen(lpsDst) && dlp_strlen(lpsDel)) dlp_strcat(lpsDst,lpsDel);
      if (lpsDst && lpsLine) strcat(lpsDst,lpsLine);
      if (bEOL)
      {
        nRec   = CData_AddRecs(m_idGen,1,100);
        lpsDst = (char*)m_idGen->XAddr(nRec,0);
      }
      lpsLine=tx+1;
    }
  }

  dlp_free(lpsBuf);
  return O_K;
}

/*
 * Manual page at dgen_man.def (dLabPro identifier is "<%")
 */
INT16 CGEN_PUBLIC CDgen::HereScript()
{
  if (!CDlpObject_MicGet(this)->iCaller) return NOT_EXEC;

  const char* lpsToken       = NULL;
  char        lpsBuf[L_SSTR];

  if ((lpsToken=CDlpObject_MicNextTokenDel(this))) AddText(lpsToken);           // Add delimiters following "<%"
  for (;;)                                                                      // Get next token loop
  {                                                                             // >>
    lpsToken = CDlpObject_MicNextToken(this,FALSE);                             //   Get next token (force!)
    if (!lpsToken) return IERROR(this,DG_HERESCRIPT,"%>",0,0);                  //   There must be one!
    if (dlp_strcmp(lpsToken,"%>")==0) break;                                    //   End of here script
    dlp_strcpy(lpsBuf,lpsToken);                                                //   Copy token to a buffer
    AddText(lpsBuf);                                                            //   Add token
    if ((lpsToken=CDlpObject_MicNextTokenDel(this))) AddText(lpsToken);         //   Add delimiters following token
  }                                                                             // <<

  return O_K;
}

/*
 * Manual page at dgen_man.def
 */
INT16 CGEN_PUBLIC CDgen::Table(CData* idTab, char* sPre, char* sDel, char* sSuf)
{
  char sBuf[256];

  if (!idTab || idTab->IsEmpty()) return O_K;

  for (INT32 nRec=0; nRec<idTab->GetNRecs(); nRec++)
  {
    if (dlp_strlen(sPre) && (!m_bBare || nRec>0)) AddText(sPre);
    for (INT32 nComp=0; nComp<idTab->GetNComps(); nComp++)
    {
      if (nComp>0 && dlp_strlen(sDel)) AddText(sDel);
      dlp_printx(sBuf,idTab->XAddr(nRec,nComp),idTab->GetCompType(nComp),0,FALSE,TRUE);
      AddText(sBuf);
    }
    if (dlp_strlen(sSuf) && (!m_bBare||nRec<idTab->GetNRecs()-1)) AddText(sSuf);
  }

  return O_K;
}

/*
 * Manual page at dgen_man.def
 */
INT16 CGEN_PUBLIC CDgen::Load(char* sFilename)
{
  char* tx = NULL;
  char  sLine[256];

  FILE* f = fopen(sFilename,"rt");
  if (!f)
    return IERROR(this,ERR_FILEOPEN,sFilename?sFilename:"(null)","reading",0);

  m_idGen->Allocate(0);
  while (fgets(sLine,255,f))
  {
    tx = &sLine[dlp_strlen(sLine)-1];
    while (tx>=sLine && (*tx=='\n' || *tx=='\r')) *tx++='\0';
    m_idGen->Sstore(sLine,CData_AddRecs(m_idGen,1,100),0);
  }
  
  fclose(f);
  return O_K;
}

/*
 * Manual page at dgen_man.def
 */
INT16 CGEN_PUBLIC CDgen::EditSection(char* lpMarkOn, char* lpMarkOff, CData* idText, char* lpOpname)
{
  INT32 nLine    = 0;
  INT32 nMarkOn  = -1;
  INT32 nMarkOff = -1;
  char sLine[256];

  // Validation and initialization
  if (!idText) return IERROR(this,ERR_NULLINST,"idText",0,0);
  if (strcmp(lpOpname,"copy")==0)
  {
    idText->Reset();
  }

  // Scan m_idGen for marks
  for (nLine=0; nLine<m_idGen->GetNRecs(); nLine++)
  {
    dlp_strcpy(sLine,(char*)m_idGen->XAddr(nLine,0)); sLine[255]='\0';
    dlp_strtrimleft(dlp_strtrimright(sLine));
    if (strcmp(sLine,lpMarkOn)==0) nMarkOn=nLine;
    if (nMarkOn>=0 && strcmp(sLine,lpMarkOff)==0) { nMarkOff=nLine; break; }
  }
  if (nMarkOn <0) return NOT_EXEC;
  if (nMarkOff<0) return IERROR(this,DG_HERESCRIPT,lpMarkOff,0,0);

  // Perform operation
  if (strcmp(lpOpname,"copy")==0)
  {
    idText->Reset();
    idText->SelectRecs(m_idGen,nMarkOn+1,nMarkOff-nMarkOn-1);
  }
  else if (strcmp(lpOpname,"replace")==0)
  {
    CData* idAux = NULL;
    ICREATE(CData,idAux,NULL);
    idAux ->SelectRecs(m_idGen,nMarkOff,m_idGen->GetNRecs()-nMarkOff);
    m_idGen->DeleteRecs(nMarkOn+1,m_idGen->GetNRecs()-nMarkOn-1);
    m_idGen->Cat(idText);
    m_idGen->Cat(idAux);
    IDESTROY(idAux);
  }
  else return IERROR(this,DG_OPCODE,lpOpname,"editing",0);

  return O_K;
}

/*
 * Manual page at dgen_man.def
 */
INT16 CGEN_PUBLIC CDgen::Write(char* sFilename)
{
  char lpNewDir[L_PATH]; 
  char lpCurDir[L_PATH]; 

  // Create target directory
  if(getcwd(lpCurDir,L_PATH) == NULL) return IERROR(this,ERR_GETCWD,0,0,0);
  dlp_splitpath(sFilename,lpNewDir,NULL);
  if(0<dlp_strlen(lpNewDir))
  {
    if(dlp_chdir(lpNewDir,TRUE)) 
    {
      dlp_chdir(lpCurDir,FALSE);
      return IERROR(this,ERR_FILEOPEN,sFilename,"writing (failed to create directory)",0);
    }
    dlp_chdir(lpCurDir,FALSE);
  }

  // Write file
  FILE* f = fopen(sFilename,"wt");
  if (!f) return IERROR(this,ERR_FILEOPEN,sFilename?sFilename:"(null)","writing",0);
  for (INT32 nRec=0; nRec<m_idGen->GetNRecs(); nRec++) fprintf(f,"%s\n",(char*)m_idGen->XAddr(nRec,0));
  fclose(f);

  return O_K;
}

// EOF
