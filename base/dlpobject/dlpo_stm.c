/* dLabPro class CDlpObject (object)
 * - Common serialization / deserialization
 *
 * AUTHOR : Matthias Wolff
 * PACKAGE: dLabPro/base
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
#include "dlp_object.h"
#ifndef __NOZLIB
  #include "zlib.h"
#endif

/**
 * Serializes this instance into a file.
 *
 * @param _this
 *          This instance
 * @param lpsFilename
 *          Pointer to file name
 * @param nFormat
 *          File format, a combination of <code>SV_XXX</code> constants
 * @return <code>O_K</code> if successful, a (negative) error code otherwise
 */
INT16 CDlpObject_Save
(
  CDlpObject* _this,
  const char* lpsFilename,
  INT16       nFormat
)
{
  BOOL bXml;                                                                    /* Save as XML file (vs. DN3)        */
  BOOL bZip;                                                                    /* Zip file                          */
  char lpsNewDir[L_PATH];                                                       /* Target directory                  */
  char lpsCurDir[L_PATH];                                                       /* Current directory                 */

  #if (defined __NOXMLSTREAM && defined __NODN3STREAM)                          /* # XML and DN3 turned off -->      */
    return IERROR(_this,ERR_NOTSUPPORTED,"Restore",0,0);                        /*   Turn one on in dlp_config.h!    */
  #endif /* #if (defined __NOXMLSTREAM && defined __NODN3STREAM) */             /* # <--                             */

  /* Validate */                                                                /* --------------------------------- */
  if (!_this                  ) return NOT_EXEC;                                /* Need this instance                */
  if (!dlp_strlen(lpsFilename)) return NOT_EXEC;                                /* Need target file name             */

  /* Initialize */                                                              /* --------------------------------- */
  bZip = (nFormat&SV_ZIP);                                                      /* Decide on zipping target file     */
  #ifndef __NOZLIB                                                              /* # Using zlib -->                  */
    gzeof(NULL);                                                                /*   HACK: Makes linker find zlib    */
  #else /* #ifndef __NOZLIB */                                                  /* # <-- Not using zlib -->          */
    if (bZip) return IERROR(_this,ERR_NOTSUPPORTED,"Save zipped",0,0);          /*   Turn on in dlp_config.h!        */
  #endif /* #ifndef __NOZLIB */                                                 /* # <--                             */
  switch (nFormat&SV_FILEFORMAT)                                                /* Decide on file format             */
  {                                                                             /* >>                                */
  case SV_XML:                                                                  /* SV_XML set                        */
    bXml=TRUE;                                                                  /*   Decide for XML                  */
    break;                                                                      /*   Get out...                      */
  case SV_DN3:                                                                  /* SV_DN3 set                        */
    bXml=FALSE;                                                                 /*   Decode for DN3                  */
    break;                                                                      /*   Get out...                      */
  default:                                                                      /* Neither or both set               */
    #ifdef __DEFAULT_FILEFORMAT_XML                                             /*   # XML is default -->            */
      bXml = TRUE;                                                              /*     Decide for XML                */
    #elif defined __DEFAULT_FILEFORMAT_DN3                                      /*   # <-- DN3 is default -->        */
      bXml = FALSE;                                                             /*     Decide for DN3                */
    #else                                                                       /*   # <-- no default specified -->  */
      bXml = TRUE;                                                              /*     Cannot happen                 */
    #endif                                                                      /*   # <--                           */
  }                                                                             /* <<                                */

  /* Create target directory if necessary */                                    /* --------------------------------- */
  if (getcwd(lpsCurDir,L_PATH)==NULL)                                           /* Get current directory             */
    return IERROR(_this,ERR_GETCWD,0,0,0);                                      /*   Something is terribly wrong     */
  dlp_splitpath(lpsFilename,lpsNewDir,NULL);                                    /* Get target directory              */
  if (dlp_strlen(lpsNewDir))                                                    /* If there is a path in lpsFilename */
  {                                                                             /* >>                                */
    if (dlp_chdir(lpsNewDir,TRUE)!=0)                                           /*   Go there creating missing dirs. */
    {                                                                           /*   >> (failed)                     */
      dlp_chdir(lpsCurDir,FALSE);                                               /*     Go back to current directory  */
      return IERROR(_this,ERR_CHDIR,lpsFilename,0,0);                           /*     Return with error             */
    }                                                                           /*   <<                              */
    dlp_chdir(lpsCurDir,FALSE);                                                 /*   Go back to current directory    */
  }                                                                             /* <<                                */

  /* Do serialization */                                                        /* --------------------------------- */
  if (bXml)                                                                     /* XML-mode                          */
  {                                                                             /* >>                                */
  #ifndef __NOXMLSTREAM                                                         /* # XML turned on -->               */
    CXmlStream* fDest = CXmlStream_CreateInstance(lpsFilename,                  /*   Create XML output stream        */
      XMLS_WRITE|(bZip?XMLS_ZIPPED:0));                                         /*   |                               */
    if (!fDest) return IERROR(_this,ERR_FILEOPEN,lpsFilename,"writing",0);      /*   Failed --> return with error    */
    CXmlStream_BeginInstance(fDest,_this->m_lpInstanceName,                     /*   Initialize stream               */
      _this->m_lpClassName);                                                    /*   |                               */
    INT16 nErr1 = INVOKE_VIRTUAL_1(SerializeXml,fDest);                         /*   Serialize instance              */
    INT16 nErr2 = CXmlStream_EndInstance(fDest);                                /*   Finish stream                   */
    INT16 nErr3 = CXmlStream_DestroyInstance(fDest);                            /*   Close stream                    */
    IF_NOK(nErr1) { IERROR(_this,ERR_DN3,0,0,0); return nErr1; }                /*   Error on serialization?         */
    IF_NOK(nErr2) { IERROR(_this,ERR_DN3,0,0,0); return nErr1; }                /*   Error on finishing stream?      */
    IF_NOK(nErr3) { IERROR(_this,ERR_FILECLOSE,lpsFilename,0,0); return nErr2; }/*   Error on closing stream?        */
  #else /* #ifndef __NOXMLSTREAM */                                             /* # <-- XML turned off -->          */
    return IERROR(_this,ERR_NOTSUPPORTED,"Save as XML",0,0);                    /*   Turn on in dlp_config.h!        */
  #endif /* #ifndef __NOXMLSTREAM */                                            /* # <--                             */
  }                                                                             /* <<                                */
  else                                                                          /* DN3-mode                          */
  {                                                                             /* >>                                */
  #ifndef __NODN3STREAM                                                         /* # DN3 turned on -->               */
    CDN3Stream* fDest =                                                         /*   Create DNorm3 output stream     */
      CDN3Stream_CreateInstance(lpsFilename,CDN3_WRITE,_this->m_lpClassName);   /*   |                               */
    if (!fDest) return IERROR(_this,ERR_FILEOPEN,lpsFilename,"writing",0);      /*   Return with error               */
    INT16 nErr1 = INVOKE_VIRTUAL_1(Serialize,fDest);                            /*   Serialize instance              */
    INT16 nErr2 = CDN3Stream_DestroyInstance(fDest);                            /*   Close stream                    */
    IF_NOK(nErr1) { IERROR(_this,ERR_DN3,0,0,0); return nErr1; }                /*   Error on serialization?         */
    IF_NOK(nErr2) { IERROR(_this,ERR_FILECLOSE,lpsFilename,0,0); return nErr2; }/*   Error on closing stream?        */
    if (bZip) dlp_fzip(lpsFilename,"wb6");                                      /*   Zip file                        */
  #else                                                                         /* # DN3 turned off -->              */
    return IERROR(_this,ERR_NOTSUPPORTED,"Save as DN3",0,0);                    /*   Turn on in dlp_config.h!        */
  #endif  /* #ifndef __NODN3STREAM */                                           /* # <--                             */
  }                                                                             /* <<                                */

  return O_K;                                                                   /* No error.                         */
}

/**
 * Deserializes this instance from a file.
 *
 * @param _this
 *          This instance
 * @param lpsFilename
 *          Pointer to file name
 * @param nFormat
 *          File format, a combination of <code>SV_XXX</code> constants
 * @return <code>O_K</code> if successful, a (negative) error code otherwise
 */
INT16 CDlpObject_Restore
(
  CDlpObject* _this,
  const char* lpsFilename,
  INT16       nFormat
)
{
  BOOL bXml;                                                                    /* Save as XML file (vs. DN3)        */
  BOOL bZip;                                                                    /* Zip file                          */

  #if (defined __NOXMLSTREAM && defined __NODN3STREAM)                          /* # XML and DN3 turned off -->      */
    return IERROR(_this,ERR_NOTSUPPORTED,"Restore",0,0);                        /*   Turn one on in dlp_config.h!    */
  #endif /* #if (defined __NOXMLSTREAM && defined __NODN3STREAM) */             /* # <--                             */

  /* Validate */                                                                /* --------------------------------- */
  if (!_this                  ) return NOT_EXEC;                                /* Need this instance                */
  if (!dlp_strlen(lpsFilename)) return NOT_EXEC;                                /* Need target file name             */

  /* Initialize */                                                              /* --------------------------------- */
  bZip = (nFormat&SV_ZIP);                                                      /* Decide on zipping target file     */
  #ifdef __NOZLIB                                                               /* # zlib turned off -->             */
    if (bZip) return IERROR(_this,ERR_NOTSUPPORTED,"Restore zipped",0,0);       /*   Turn on in dlp_config.h!        */
  #endif /* #ifndef __NOZLIB */                                                 /* # <--                             */

  switch (nFormat&SV_FILEFORMAT)                                                /* Decide on file format             */
  {                                                                             /* >>                                */
  case SV_XML:                                                                  /* SV_XML set                        */
    bXml=TRUE;                                                                  /*   Decide for XML                  */
    break;                                                                      /*   Get out...                      */
  case SV_DN3:                                                                  /* SV_DN3 set                        */
    bXml=FALSE;                                                                 /*   Decode for DN3                  */
    break;                                                                      /*   Get out...                      */
  default:                                                                      /* Neither or both set               */
    #ifndef  __NOXMLSTREAM                                                      /*   # XML turned on -->             */
      bXml = CXmlStream_CheckIsXml(lpsFilename,XMLS_READ|(bZip?XMLS_ZIPPED:0)); /*     Auto-detect                   */
    #else /* #ifndef  __NOXMLSTREAM */                                          /*   # <-- XML turned off -->        */
      bXml = FALSE;                                                             /*     XML not compiled in...        */
    #endif /* #ifndef  __NOXMLSTREAM */                                         /*   # <--                           */
  }                                                                             /* <<                                */

  /* Do deserialization */                                                      /* --------------------------------- */
  if (bXml)                                                                     /* XML-mode                          */
  {                                                                             /* >>                                */
  #ifndef __NOXMLSTREAM                                                         /* # XML turned on -->               */
    CXmlStream* fSrc =                                                          /*   Create XML input stream         */
       CXmlStream_CreateInstance(lpsFilename, XMLS_READ|(bZip?XMLS_ZIPPED:0));  /*   |                               */
    if (!fSrc)                                                                  /*   Failed                          */
      return                                                                    /*     Return with error             */
        IERROR(_this,ERR_FILEOPEN,"reading (or parse error)",lpsFilename,0);    /*     |                             */
    INT16 nErr1 = INVOKE_VIRTUAL_1(DeserializeXml,fSrc);                        /*   Deserialize instance            */
    INT16 nErr2 = CXmlStream_DestroyInstance(fSrc);                             /*   Close stream                    */
    IF_NOK(nErr1) { IERROR(_this,ERR_DN3,0,0,0); return nErr1; }                /*   Error on deserialization?       */
    IF_NOK(nErr2) { IERROR(_this,ERR_FILECLOSE,lpsFilename,0,0); return nErr2;} /*   Error on closing stream?        */
  #else /* #ifndef __NOXMLSTREAM */                                             /* # <-- XML turned off -->          */
    return IERROR(_this,ERR_NOTSUPPORTED,"Restore from XML",0,0);               /*   To be turned on in dlp_config.h!*/
  #endif /* #ifndef __NOXMLSTREAM */                                            /* # <--                             */
  }                                                                             /* <<                                */
  else                                                                          /* DNorm3 mode                       */
  {                                                                             /* >>                                */
  #ifndef __NODN3STREAM                                                         /* # DN3 turned on -->               */
    const char* lpsTempFilename = NULL;                                         /*   A temporary file name           */
    CDN3Stream* fSrc            = NULL;                                         /*   The DN3 stream                  */
    BOOL        bUnzipped       = FALSE;                                        /*   File has been unzipped          */
    #ifndef __NOZLIB                                                            /*   # zlib linked -->               */
      bUnzipped = dlp_funzip((char*)lpsFilename,&lpsTempFilename);              /*     Zipped -> unzip to temp. file */
    #endif  /* #ifndef __NOZLIB */                                              /*   # <--                           */
    fSrc = CDN3Stream_CreateInstance(bUnzipped?lpsTempFilename:lpsFilename,     /*   Create DN3 stream               */
      CDN3_READ,_this->m_lpClassName);                                          /*   |                               */
    if (!fSrc)                                                                  /*   Failed                          */
    {                                                                           /*   >>                              */
      if (bUnzipped) unlink(lpsTempFilename);                                   /*     Remove temporary file         */
      return IERROR(_this,ERR_FILEOPEN,lpsFilename,"reading",0);                /*     Return with error             */
    }                                                                           /*   <<                              */
    INT16 nErr1 = INVOKE_VIRTUAL_1(Deserialize,fSrc);                           /*   Deserialize instance            */
    INT16 nErr2 = CDN3Stream_DestroyInstance(fSrc);                             /*   Close stream                    */
    if (bUnzipped) unlink(lpsTempFilename);                                     /*   Remove temporary file           */
    IF_NOK(nErr1) { IERROR(_this,ERR_DN3,0,0,0); return nErr1; }                /*   Error on deserialization?       */
    IF_NOK(nErr2) { IERROR(_this,ERR_FILECLOSE,lpsFilename,0,0); return nErr2;} /*   Error on closing stream?        */
  #else /* #ifndef __NODN3STREAM */                                             /* # <-- DN3 turned off -->          */
    return IERROR(_this,ERR_NOTSUPPORTED,"Restore from DN3",0,0);               /*   To be turned on in dlp_config.h!*/
  #endif /* #ifndef __NODN3STREAM */                                            /* # <--                             */
  }                                                                             /* <<                                */

  return O_K;                                                                   /* Everything's all right            */
}

/* EOF */
