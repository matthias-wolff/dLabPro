/* dLabPro program recognizer (dLabPro recognizer)
 * - Helper functions
 *
 * AUTHOR : Frank Duckhorn
 * PACKAGE: dLabPro/programs
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
#include "recognizer.h"

INT16 restore(
  CDlpObject *iInst,
  const char *lpsFilename,
  INT16 (*deserialize)(CDlpObject* __this, CDN3Stream* lpSrc),
  INT16 (*deserializeXml)(CDlpObject* __this, CXmlStream* lpSrc)
)
{
#if ! defined __NOXMLSTREAM || ! defined __NODN3STREAM
  BOOL  bUnzipped       = FALSE;
  const char *lpsTempFilename = NULL;
  BOOL  bFormatXml;
  
  /* Validate */                                                                /* ------------------------------------ */
  if (!iInst                  ) return NOT_EXEC;                                /* Need instance to deserialize */
  if (!dlp_strlen(lpsFilename)) return NOT_EXEC;                                /* Need file name */

#ifdef  __NOXMLSTREAM
  bFormatXml=FALSE;                                                              /*   Only Dn3 compiled in */
#else /* #ifndef __NOXMLSTREAM */
#  ifdef  __NODN3STREAM
  bFormatXml=TRUE;                                                               /*   Only Xml compiled in */
#  endif /* #ifndef __NODN3STREAM */
  bFormatXml=CXmlStream_CheckIsXml(lpsFilename,XMLS_READ);                       /*  Check file for type */
#endif /* #ifndef __NOXMLSTREAM */

  if (bFormatXml)                                                               /* XML-mode */
  {                                                                             /* >> */
#ifndef __NOXMLSTREAM
    INT16 nErr1;
    INT16 nErr2;
    CXmlStream* fSrc = NULL;
    fSrc = CXmlStream_CreateInstance(lpsFilename,XMLS_READ);                    /*   Create XML input stream */
    if(!fSrc)                                                                   /*   Failed + no alternative? */
      return IERROR(iInst,ERR_FILEOPEN,"reading (or parse error)",lpsFilename,0);/*   Return with error */
    nErr1 = deserializeXml(iInst,fSrc);                                         /*   Deserialize instance */
    nErr2 = CXmlStream_DestroyInstance(fSrc);                                   /*   Close stream */
    IF_NOK(nErr1) { IERROR(iInst,ERR_DN3,0,0,0); return nErr1; }                /*   Error on deserialization? */
    IF_NOK(nErr2) { IERROR(iInst,ERR_FILECLOSE,lpsFilename,0,0); return nErr2;} /*   Error on closing stream? */
#else /* #ifndef __NOXMLSTREAM */
    return IERROR(iInst,ERR_NOTSUPPORTED,"/xml -restore",0,0);
#endif /* #ifndef __NOXMLSTREAM */
  }                                                                             /* << */
  else                                                                          /* DNorm3 mode */
  {                                                                             /* >> */
    CDN3Stream* fSrc = NULL;                                                    /*    */
    INT16 nErr1;
    INT16 nErr2;
    short Elvl;

#ifndef __NOZLIB
    bUnzipped = dlp_funzip((char*)lpsFilename, &lpsTempFilename);               /*   Zipped -> unzip to temporary file */
#endif  /* #ifndef __NOZLIB */

    if(bUnzipped) 
      fSrc = CDN3Stream_CreateInstance(lpsTempFilename,CDN3_READ,iInst->m_lpClassName); /* Create Dnorm3 input stream from unzipped file */
    else
      fSrc = CDN3Stream_CreateInstance(lpsFilename,CDN3_READ,iInst->m_lpClassName); /* Create Dnorm3 input stream */
    if(!fSrc&&bUnzipped) unlink(lpsTempFilename);                               /*   Remove temporary file */
    if(!fSrc) return IERROR(iInst,ERR_FILEOPEN,lpsFilename,"reading",0);        /*   Failed --> return with error */
    Elvl=CDlpObject_SetErrorLevel(1);
    nErr1 = deserialize(iInst,fSrc);                                            /*   Deserialize instance */
    CDlpObject_SetErrorLevel(Elvl);
    nErr2 = CDN3Stream_DestroyInstance(fSrc);                                   /*   Close stream */
#ifndef __NOZLIB
    if((nErr1<0||nErr2<0)&&bUnzipped) unlink(lpsTempFilename);                  /*   Remove temporary file */
#endif  /* #ifndef __NOZLIB */
    IF_NOK(nErr1) { IERROR(iInst,ERR_DN3,0,0,0); return nErr1; }                /*   Error on deserialization? */
    IF_NOK(nErr2) { IERROR(iInst,ERR_FILECLOSE,lpsFilename,0,0); return nErr2;} /*   Error on closing stream? */
#ifndef __NOZLIB
    if(bUnzipped) unlink(lpsTempFilename);                                      /*   Remove temporary file */
#endif  /* #ifndef __NOZLIB */
  }                                                                             /* << */

#endif
  return O_K;                                                                   /* No error. */

}

char *env_replace(char *lpsVal){
  static char lpsRet[STR_LEN*2+128];
  char *lpsP=lpsRet;
  snprintf(lpsRet,STR_LEN*2+128,lpsVal);
  while((lpsP=strchr(lpsP,'$'))){
    INT32 nI,nL;
    char *lpsV=NULL;
    char lpsE[32];
    for(nI=1;nI<32;nI++){
      snprintf(lpsE,nI+1,lpsP+1);
      if((lpsV=getenv(lpsE))) break;
    }
    if(!lpsV){ lpsP++; continue; }
    nL=strlen(lpsV);
    memmove(lpsP+nL,lpsP+1+nI,strlen(lpsP+1+nI)+1);
    memcpy(lpsP,lpsV,nL);
  }
  return lpsRet;
}

