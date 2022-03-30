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

#ifndef __NOXMLSTREAM

/* gzip flag byte */
static int const gz_magic[2] = {0x1f, 0x8b}; /* gzip magic header */
#define ASCII_FLAG   0x01 /* bit 0 set: file probably ascii text */
#define HEAD_CRC     0x02 /* bit 1 set: header CRC present */
#define EXTRA_FIELD  0x04 /* bit 2 set: extra field present */
#define ORIG_NAME    0x08 /* bit 3 set: original file name present */
#define COMMENT      0x10 /* bit 4 set: file comment present */
#define RESERVED     0xE0 /* bits 5..7: reserved */
#include "zutil.h"

INT16 uncompressbuf(void **buf,size_t *si){
  z_stream stream;
  int err;
  uint32_t *footer=(uint32_t*)((char*)*buf+*si-8);
  uint32_t crc=footer[0]/*,nsi=footer[1] wrong if >4G*/;
  memset(&stream,0,sizeof(stream));

  const size_t blk=1<<20;
  size_t nsi=blk*2;
  uint8_t *nbuf;
  if(!(nbuf=(uint8_t*)malloc(nsi))) return Z_BUF_ERROR;

  stream.next_in = (Bytef*)*buf;
  stream.avail_in = (uInt)*si;
  /* Check for source > 64K on 16-bit machine: */
  if ((uLong)stream.avail_in != *si) return Z_BUF_ERROR;
  stream.next_out = (Bytef*)nbuf;
  stream.avail_out = nsi;

  /* gz_magic */
  if(stream.next_in[0]!=gz_magic[0] || stream.next_in[1]!=gz_magic[1]) return Z_BUF_ERROR;
  stream.next_in+=2; stream.avail_in-=2;
  /* method+flags */
  int method=stream.next_in[0]; stream.next_in+=1; stream.avail_in-=1;
  int flags =stream.next_in[0]; stream.next_in+=1; stream.avail_in-=1;
  if(method!=Z_DEFLATED || (flags&RESERVED)) return Z_BUF_ERROR;
  /* time, xflags, os */
  stream.next_in+=6; stream.avail_in-=6;
  /* extra field */
  if(flags&EXTRA_FIELD){
    int len=stream.next_in[0]+(stream.next_in[1]<<8);
    stream.next_in+=len; stream.avail_in-=len;
  }
  /* original file name */
  if(flags&ORIG_NAME){ int len=strlen((char*)stream.next_in)+1; stream.next_in+=len; stream.avail_in-=len; }
  /* comment */
  if(flags&COMMENT){ int len=strlen((char*)stream.next_in)+1; stream.next_in+=len; stream.avail_in-=len; }
  /* crc */
  if(flags&HEAD_CRC){ int len=strlen((char*)stream.next_in)+1; stream.next_in+=len; stream.avail_in-=len; }

  /* inflate */
  if((err=inflateInit2(&stream,-MAX_WBITS))!=Z_OK) return err;
  while(1){
    if((err=inflate(&stream,Z_BLOCK))!=Z_OK){
      if(err==Z_STREAM_END) break;
      return err;
    }
    if(stream.avail_out<blk){
      nsi+=blk;
      if(!(nbuf=(uint8_t*)realloc(nbuf,nsi))) return Z_BUF_ERROR;
      stream.avail_out+=blk;
      stream.next_out=nbuf+stream.total_out;
    }
  }
  if(stream.total_out>nsi) return Z_BUF_ERROR;
  if(!(nbuf=(uint8_t*)realloc(nbuf,nsi=stream.total_out))) return Z_BUF_ERROR;
  if(nsi<1L<<32 && crc32(0L,(Bytef*)nbuf,nsi)!=crc) return Z_BUF_ERROR;
  if(inflateEnd(&stream)!=Z_OK) return Z_BUF_ERROR;

  *buf=nbuf;
  *si=nsi;
  return O_K;
}

INT16 compressbuf(void **buf,size_t *si){
  uLongf nsi,bsi;
  void *nbuf;
  z_stream stream;
  int err;
  uint8_t header[10]={(uint8_t)gz_magic[0],(uint8_t)gz_magic[1],
                      Z_DEFLATED, 0 /*flags*/, 0,0,0,0 /*time*/, 0 /*xflags*/, OS_CODE};
  uint32_t footer[2]={0,(uint32_t)*si};
  footer[0]=crc32(0L,(Bytef*)*buf,*si);
  memset(&stream,0,sizeof(stream));

  nsi=compressBound(*si);
  bsi=nsi+sizeof(header)+sizeof(footer);
  if(!(nbuf=malloc(bsi))) return Z_BUF_ERROR;
  memcpy(nbuf,header,sizeof(header));

  stream.next_in = (Bytef*)*buf;
  stream.avail_in = (uInt)*si;
  stream.next_out = (Bytef*)nbuf+sizeof(header);
  stream.avail_out = (uInt)nsi;

  if((err=deflateInit2(&stream,Z_DEFAULT_COMPRESSION,Z_DEFLATED,-MAX_WBITS,DEF_MEM_LEVEL,Z_DEFAULT_STRATEGY))!=Z_OK) return err;
  if((err=deflate(&stream,Z_FINISH))!=Z_STREAM_END) return err==Z_OK ? Z_BUF_ERROR : err;
  nsi=stream.total_out;
  if((err=deflateEnd(&stream))!=Z_OK) return err;

  memcpy((char*)nbuf+sizeof(header)+nsi,footer,sizeof(footer));

  free(*buf);
  *buf=nbuf;
  *si=nsi+sizeof(header)+sizeof(footer);
  return O_K;
}

#endif

INT16 CDlpObject_SaveBuffer(CDlpObject* _this, void **buf, size_t *si, INT16 nFormat){
  #if defined __NOXMLSTREAM
    return IERROR(_this,ERR_NOTSUPPORTED,"Restore",0,0);                        /*   Turn one on in dlp_config.h!    */
  #else
  BOOL bZip;                                                                    /* Zip file                          */

  /* Validate */                                                                /* --------------------------------- */
  if (!_this                  ) return NOT_EXEC;                                /* Need this instance                */
  if (!buf || !si             ) return NOT_EXEC;                                /* Need target file name             */

  /* Initialize */                                                              /* --------------------------------- */
  bZip = (nFormat&SV_ZIP);                                                      /* Decide on zipping target file     */

  /* Do serialization */                                                      /* --------------------------------- */
  CXmlStream* fDest = CXmlStream_CreateInstance(NULL,XMLS_WRITE);             /*   Create XML output stream        */
  if (!fDest) return IERROR(_this,ERR_FILEOPEN,"Buffer","writing",0);         /*   Failed --> return with error    */
  CXmlStream_BeginInstance(fDest,_this->m_lpInstanceName,                     /*   Initialize stream               */
    _this->m_lpClassName);                                                    /*   |                               */
  INT16 nErr1 = INVOKE_VIRTUAL_1(SerializeXml,fDest);                         /*   Serialize instance              */
  INT16 nErr2 = CXmlStream_EndInstance(fDest);                                /*   Finish stream                   */
  CXmlStream_GetBuffer(fDest,buf,si);
  INT16 nErr3 = CXmlStream_DestroyInstance(fDest);                            /*   Close stream                    */
  IF_NOK(nErr1) { IERROR(_this,ERR_DN3,0,0,0); return nErr1; }                /*   Error on serialization?         */
  IF_NOK(nErr2) { IERROR(_this,ERR_DN3,0,0,0); return nErr1; }                /*   Error on finishing stream?      */
  IF_NOK(nErr3) { IERROR(_this,ERR_FILECLOSE,"Buffer",0,0); return nErr2; }   /*   Error on closing stream?        */

  if(bZip && compressbuf(buf,si)!=O_K) return NOT_EXEC;

  return O_K;
  #endif /* #if (defined __NOXMLSTREAM && defined __NODN3STREAM) */             /* # <--                             */
}

INT16 CDlpObject_RestoreBuffer(CDlpObject* _this,void *buf,size_t si){
  #if defined __NOXMLSTREAM
    return IERROR(_this,ERR_NOTSUPPORTED,"Restore",0,0);                        /*   Turn one on in dlp_config.h!    */
  #else

  /* Validate */                                                                /* --------------------------------- */
  if (!_this                  ) return NOT_EXEC;                                /* Need this instance                */
  if (!buf || !si             ) return NOT_EXEC;                                /* Need target file name             */

  /* Zipped? */
  char freebuf=0;
  if(*(uint16_t*)buf==0x8b1f){
    /* TODO: Block wise uncompress + block wise CXmlStream_SetBuffer */
    if(uncompressbuf(&buf,&si)!=O_K) return NOT_EXEC;
    freebuf=1;
  }

  /* Check if buffer match xml-head    */
  if(strncmp((char*)buf,"<?xml",5)) return NOT_EXEC;
  
  CXmlStream* fSrc = CXmlStream_CreateInstance(NULL,XMLS_READ); /*   Create XML input stream         */
  CXmlStream_SetBuffer(fSrc,buf,si);
  if(!fSrc) return NOT_EXEC;
  INT16 nErr1 = INVOKE_VIRTUAL_1(DeserializeXml,fSrc);                        /*   Deserialize instance            */
  INT16 nErr2 = CXmlStream_DestroyInstance(fSrc);                             /*   Close stream                    */
  IF_NOK(nErr1) { IERROR(_this,ERR_DN3,0,0,0); return nErr1; }                /*   Error on deserialization?       */
  IF_NOK(nErr2) { IERROR(_this,ERR_FILECLOSE,"Buffer",0,0); return nErr2;} /*   Error on closing stream?        */

  /* Free if zipped */
  if(freebuf) free(buf);

  return O_K;
  #endif /* #if (defined __NOXMLSTREAM && defined __NODN3STREAM) */             /* # <--                             */
}
/* EOF */
