/* dLabPro class CDlpFile (file)
 * - Auxilary and information methods
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

#include "dlp_cscope.h"                                                         /* Indicate C scope                  */
#include "dlp_file.h"                                                           /* Include class header file         */

/*
 * Manual page at file.def
 */
BOOL CGEN_PUBLIC CDlpFile_Exists(CDlpFile* _this, const char* sFilename)
{
  BOOL bRet = FALSE;
  if (_this->m_bDir)
  {
#ifndef __TMS
    char sBuf[L_PATH+1];
    if(getcwd(sBuf,L_PATH) == NULL) return IERROR(_this,ERR_GETCWD,0,0,0);
    bRet = (chdir(sFilename)==0);
    if(chdir(sBuf) != 0) return IERROR(_this,ERR_CHDIR,sBuf,0,0);
#endif
    return bRet;
  }
  else
  {
    FILE* f = fopen(sFilename,"r");
    bRet = (f!=NULL);
    if (f) fclose(f);
    return bRet;
  }
}

/*
 * Manual page at file.def
 */
const char* CGEN_PUBLIC CDlpFile_GetRootClass(CDlpFile* _this,const char* lpsFilename)
{
  static char lpName[L_NAMES] = "";                                             /* Returned buffer for class name    */
  char bResultOk = 1;                                                           /* Class name found ?                */
  if(!lpsFilename)                                                              /* Check given filename              */
  {                                                                             /* >>                                */
    IERROR(_this,ERR_FILEOPEN,"reading (no filename)",0,0);                     /*   No filename found => Error      */
    return NULL;                                                                /*   Nothing to do                   */
  }
#ifndef __NOXMLSTREAM                                                           /* <<                                */
  if(CXmlStream_CheckIsXml(lpsFilename,XMLS_READ))                              /* Check if file is Xml-File         */
  {                                                                             /* >>                                */
    CXmlStream* fSrc            = NULL;                                         /*   Stream object                   */
    SDomObject* lpDo            = NULL;                                         /*   Root object of stream           */
    fSrc = CXmlStream_CreateInstance(lpsFilename,XMLS_READ);                    /*   Create stream instance          */
    if(!fSrc)                                                                   /*   Error in creation ?             */
    {                                                                           /*   >>                              */
      IERROR(_this,ERR_FILEOPEN,"reading (or parse error)",lpsFilename,0);      /*     Show error                    */
      return NULL;                                                              /*     Nothing to do                 */
    }                                                                           /*   <<                              */
    lpDo=CXmlStream_FindObject(fSrc,"");                                        /*   Get root object from stream     */
    if(lpDo) strncpy(lpName,lpDo->lpsType,L_NAMES);                             /*   If found => copy class name     */
    else                                                                        /*   No root object found            */
    {                                                                           /*   >>                              */
      IFCHECK printf(" *** ROOT OBJECT NOT FOUND ***");                         /*     Protocol                      */
      IERROR(_this,ERR_STREAMOBJ,"",0,0);                                       /*     Show error                    */
      bResultOk = 0;                                                            /*     Nothing to return             */
    }                                                                           /*   <<                              */
    IF_NOK(CXmlStream_DestroyInstance(fSrc))                                    /*   Destroy stream instance         */
    {                                                                           /*   >>                              */
      IERROR(_this,ERR_FILECLOSE,lpsFilename,0,0);                              /*     Show error                    */
      return NULL;                                                              /*     Nothing to do                 */
    }                                                                           /*   <<                              */
  }                                                                             /* <<                                */
  else                                                                          /* Assume DN3 file format            */
#endif /* #ifndef __NOXMLSTREAM */
  {                                                                             /* >>                                */
#ifndef __NODN3STREAM
    CDN3Stream* fSrc = NULL;                                                    /*   Stream object                   */
    BOOL  bUnzipped       = FALSE;                                              /*   Unzipped the file               */
    const char *lpsTempFilename = NULL;                                         /*   Temporary file name             */
#ifndef __NOZLIB
    bUnzipped = dlp_funzip((char*)lpsFilename, &lpsTempFilename);               /*   Zipped -> unzip to temp. file   */
#endif  /* #ifndef __NOZLIB */
    if(bUnzipped)                                                               /*   Unzipped ?                      */
      fSrc = CDN3Stream_CreateInstance(lpsTempFilename,CDN3_READ,NULL);         /*   Create DN3 stream from unzip f. */
    else                                                                        /*   Else                            */
      fSrc = CDN3Stream_CreateInstance(lpsFilename,CDN3_READ,NULL);             /*   Create DN3 stream               */
    if(!fSrc)                                                                   /*   Error in creation ?             */
    {                                                                           /*   >>                              */
      IERROR(_this,ERR_FILEOPEN,"reading (or parse error)",lpsFilename,0);      /*     Show error                    */
      return NULL;                                                              /*     Nothing to do                 */
    }                                                                           /*   <<                              */
    IF_NOK(CDN3Stream_GetFileClass(fSrc,lpName))                                /*   Get file class name from stream */
    {                                                                           /*   >>                              */
      IFCHECK printf(" *** ROOT OBJECT NOT FOUND ***");                         /*     Protocol                      */
      IERROR(_this,ERR_STREAMOBJ,"",0,0);                                       /*     Show error                    */
      bResultOk = 0;                                                            /*     Nothing to return             */
    }                                                                           /*   <<                              */
    IF_NOK(CDN3Stream_DestroyInstance(fSrc))                                    /*   Destroy stream instance         */
    {                                                                           /*   >>                              */
      IERROR(_this,ERR_FILECLOSE,lpsFilename,0,0);                              /*     Show error                    */
      bResultOk = 0;                                                            /*     Nothing to return             */
    }                                                                           /*   <<                              */
#ifndef __NOZLIB
    if(bUnzipped) unlink(lpsTempFilename);                                      /*   Remove temporary file           */
#endif  /* #ifndef __NOZLIB */
#endif
  }                                                                             /* <<                                */
  return bResultOk ? lpName : NULL;                                             /* Return class name if found        */
}

/*
 * Manual page at file.def
 */
const char* CGEN_PUBLIC CDlpFile_Next(CDlpFile* _this)
{
  char  lpsBuffer[L_PATH] = "";
  char  lpsFName[L_PATH]  = "";
  char* tx                = NULL;

  _this->m_lpsSfile[0]   = '\0';
  _this->m_lpsSfileFq[0] = '\0';

  /* Get the next non-blank line from file list */
  while(dlp_strlen(lpsBuffer)==0)
  {
    _this->m_nNfile++;
    if
    (
      CData_GetNRecs(AS(CData,_this->m_idFlistData))<=0 ||
      _this->m_nNfile>=CData_GetNRecs(AS(CData,_this->m_idFlistData))
    )
    {
      return NULL;
    }
    dlp_strncpy(lpsBuffer,CData_Sfetch(AS(CData,_this->m_idFlistData),_this->m_nNfile,0),L_PATH);
    CData_SelectRecs(AS(CData,_this->m_idRecfile),AS(CData,_this->m_idFlistData),_this->m_nNfile,1);
    /*MW: NO!!! CData_DeleteRecs(AS(CData,_this->m_idFlistData),0,1);*/
    dlp_strtrimleft(lpsBuffer);
    dlp_strtrimright(lpsBuffer);
    for (tx=lpsBuffer; *tx; tx++)
      if (dlp_charin(*tx,_this->m_lpsSep))
      {
        *tx='\0';
        break;
      }
  }
  dlp_strncpy(_this->m_lpsSfile,lpsBuffer,255);

  /* Finish file name */
  if (dlp_strlen(_this->m_lpsPath))
  {
    if (dlp_strlen(_this->m_lpsExt)) sprintf(lpsFName,"%s%c%s.%s",_this->m_lpsPath,'/',lpsBuffer,_this->m_lpsExt);
    else                             sprintf(lpsFName,"%s%c%s"   ,_this->m_lpsPath,'/',lpsBuffer);
  }
  else
  {
    if (dlp_strlen(_this->m_lpsExt)) sprintf(lpsFName,"%s.%s",lpsBuffer,_this->m_lpsExt);
    else                             sprintf(lpsFName,"%s",lpsBuffer);
  }
  dlp_convert_name(CN_XLATPATH,lpsFName,lpsFName);
  dlp_strncpy(_this->m_lpsSfileFq,lpsFName,255);

  return _this->m_lpsSfileFq;
}

/*
 * Manual page at file.def
 */
INT16 CGEN_PUBLIC CDlpFile_Partition(CDlpFile* _this, CDlpFile* iSrc, FLOAT64 nPartSize, INT32 nPartNum)
{
	INT32 nFirst = 0;
	INT32 nCount = 0;
	INT32 nSize  = 0;

	CHECK_THIS_RV(NOT_EXEC);
	if (iSrc==_this)
		return IERROR(_this,
	    FIL_INVALARG,"source and destination instance cannot be identical",0,0);

	nSize = CData_GetNRecs(AS(CData,iSrc->m_idFlistData));
	if (nSize<=0) return NOT_EXEC;

	if (nPartSize<1.)
	{
		nFirst = (INT32)((FLOAT64)nSize*nPartSize*nPartNum+0.5);
		nCount = (INT32)((FLOAT64)nSize*nPartSize*(nPartNum+1)+0.5)-nFirst;
	}
	else
	{
		nFirst = (INT32)nSize*nPartNum;
		nCount = (INT32)nSize*(nPartNum+1) - nFirst;
	}
	if (nFirst       <0    ) nFirst = 0;
	if (nFirst+nCount>nSize) nCount = nSize - nFirst;

	if ((nSize-nFirst-nCount > 0) && (nSize-nFirst-nCount < (INT32)(0.1*nCount)+2))
	{
		/* Do not make "rest" partitions smaller than 10% of the partition size
		 * or two files                                                          */
		nCount = nSize - nFirst;
	}

	CDlpFile_Copy(BASEINST(_this),BASEINST(iSrc));                                /* Copy file list                    */
	_this->m_lpsFlist[0]='\0';                                                    /* Forget file list file name        */
	CData_SelectRecs(AS(CData,_this->m_idFlistData),                              /* Cut partition                     */
	  AS(CData,_this->m_idFlistData),nFirst,nCount);                              /* |                                 */
	CDlpFile_Reset(BASEINST(_this),TRUE);                                         /* Rewind                            */

	return O_K;
}

/* EOF */
