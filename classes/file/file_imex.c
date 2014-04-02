/* dLabPro class CDlpFile (file)
 * - Import and export of non-native file formats
 *
 * AUTHOR : Matthias Eichner
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

#include "dlp_cscope.h"
#include "dlp_file.h"

/**
 * Import and export of non-native file formats.
 *
 * @param _this       This instance.
 * @param sFilename   Name of file to import.
 * @param sFilter     Filter to use for import.
 * @param iInst       Instance where to save imported data.
 * @param nMode       Import or export mode.
 * @return <code>O_K</code> if successful, a (negative) error code otherwise
 */
INT16 CGEN_PROTECTED CDlpFile_ImportExport
(
  CDlpFile*   _this,
  const char* sFilename,
  const char* sFilter,
  CDlpObject* iInst,
  INT16 nMode
)
{
  INT16    retVal     = O_K;
  lnode_t* lpNode     = NULL;
  CFILE_FTYPE* lpType = NULL;
  FILE* lpFile        = NULL;
  char lpsFilenameLoc[L_PATH];
  char lpsCmd[L_PATH*2];

  CHECK_THIS_RV(FALSE);

  if(!dlp_strlen(sFilename)) return IERROR(_this,FIL_FILENAME,0     ,0,0);
  if(!iInst                ) return IERROR(_this,ERR_NULLARG,"iInst",0,0);

  dlp_strcpy(lpsFilenameLoc,sFilename);

  /* create target directory if necessary */
  if(nMode==EXPORT_FILE)
  {
    char lpNewDir[L_PATH] = "";
    char lpCurDir[L_PATH] = "";

#ifndef __TMS
    if(!_this->m_bExecute)
    {
      if(getcwd(lpCurDir,L_PATH) == NULL) return IERROR(_this,ERR_GETCWD,0,0,0);
      dlp_splitpath(lpsFilenameLoc,lpNewDir,NULL);
      if(0<dlp_strlen(lpNewDir))
      {
        if(dlp_chdir(lpNewDir,TRUE))
        {
          dlp_chdir(lpCurDir,FALSE);
          return
          IERROR(_this,ERR_FILEOPEN,lpsFilenameLoc,
              "writing (failed to create directory)",0);
        }
        dlp_chdir(lpCurDir,FALSE);
      }
    }
#endif
  }

#ifndef __TMS
  /* Execute lpsFilenameLoc (initialization & execute if import) */
  if(_this->m_bExecute)
  {
    char lpsBase[L_PATH];
    char *lpsTmp;
    dlp_splitpath(lpsFilenameLoc,NULL,lpsBase);
    if(strchr(lpsBase,' ')) *strchr(lpsBase,' ')='\0';
    lpsTmp=dlp_tempnam(NULL,lpsBase);
    snprintf(lpsCmd, L_PATH*2-1, "%s %c %s", lpsFilenameLoc, nMode==EXPORT_FILE?'<':'>', lpsTmp);
    dlp_strcpy(lpsFilenameLoc,lpsTmp);
    if(nMode==IMPORT_FILE) if(dlp_system(lpsCmd)!=0) return IERROR(_this,ERR_FILEOPEN,lpsCmd,"executing",0);
  }
#endif

  /* test if file is readable/writable */
  if(nMode==IMPORT_FILE)
    lpFile = fopen(lpsFilenameLoc,"r");
  else if(nMode==EXPORT_FILE)
    lpFile = fopen(lpsFilenameLoc,"a");
  else DLPASSERT(FMSG("Unknown operation mode."));

  if(!lpFile)
  {
    if (nMode==IMPORT_FILE)
      return IERROR(_this,ERR_FILEOPEN,sFilename,"reading",0);
    else if (nMode==EXPORT_FILE)
      return IERROR(_this,ERR_FILEOPEN,sFilename,"writing",0);
  }
  fclose(lpFile);

  /* lookup filter function from list */
  lpType = (CFILE_FTYPE*)dlp_calloc(1,sizeof(CFILE_FTYPE));
  if(!lpType) return IERROR(_this,ERR_NOMEM,0,0,0);

  dlp_strncpy(lpType->lpName,sFilter,L_NAMES);
  dlp_strncpy(lpType->lpClassName,iInst->m_lpClassName,L_NAMES);
  lpType->nMode = nMode;

  lpNode = list_find(_this->m_lpFtypes,lpType,CDlpFile_CompareFTypeList);
  if(!lpNode)
  {
    dlp_free(lpType);
    return
      IERROR(_this,FIL_NOIMEX,nMode==IMPORT_FILE?"import":"export",sFilter,
        iInst->m_lpClassName);
  }
  dlp_free(lpType);
  lpType = NULL;

  /* Invoke import function */
  lpType = (CFILE_FTYPE*)lnode_get(lpNode);
  retVal = lpType->FilterFunc(_this,lpsFilenameLoc,iInst,lpType->lpName);

#ifndef __TMS
  /* Execute lpsFilenameLoc (execute if export & finalization) */
  if(_this->m_bExecute)
  {
    if(nMode==EXPORT_FILE) dlp_system(lpsCmd);
    unlink(lpsFilenameLoc);
  }else
#endif
  if(nMode==EXPORT_FILE && _this->m_bZip) dlp_fzip(lpsFilenameLoc,"wb6");

  if(retVal != O_K)
  {
    if(nMode==IMPORT_FILE) return IERROR(_this,FIL_IMPORT,lpsFilenameLoc,sFilter,0);
    else                   return IERROR(_this,FIL_EXPORT,lpsFilenameLoc,sFilter,0);
  }

  return O_K;
}

/**
 * Callback function freeing list of file types.
 */
void CGEN_SPRIVATE CDlpFile_FreeFTypeList(list_t* lpList, lnode_t* lpNode, void* context)
{
  dlp_free(lnode_get(lpNode));
}

/**
 * Callback function for looking up a import/export method.
 */
int CGEN_SPRIVATE CDlpFile_CompareFTypeList(const void *key, const void *node)
{
  CFILE_FTYPE* lpKey  = (CFILE_FTYPE*)key;
  CFILE_FTYPE* lpNode = (CFILE_FTYPE*)node;

  if(!dlp_strncmp(lpKey->lpName,     lpNode->lpName,     L_NAMES) &&
     !dlp_strncmp(lpKey->lpClassName,lpNode->lpClassName,L_NAMES) &&
     lpKey->nMode == lpNode->nMode)
    return 0;

  return 1;
}

/* EOF */
