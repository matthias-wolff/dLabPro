/* dLabPro class CDlpFile (file)
 * - Import/export functions of data instances
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
#ifndef __NOLIBSNDFILE
  #include "sndfile.h"
#endif /* __NOLIBSNDFILE */
#include "dlp_matrix.h"
#include "dlp_signal.h"

#ifdef _WINDOWS
  #define snprintf _snprintf
#endif

/* PM-Header */
typedef struct PM_HEADER_T
{
  INT16  kennzahl;
  INT16  atf;
  char  methode;
  char  format;
  char  dummy;
  char  laenge;
} PM_HEADER;

/* Data format of PM files */
typedef struct PMARKEN
{

  INT16  laenge;    /* Periodenlaenge (genau 16Bit gross) */
  BYTE  anregung;  /* Anregungsart: 1 = stimmhaft, 0 = stimmlos (8Bit) */
  BYTE  dummy;    /* Dummy-Byte = Reserve (8Bit) */

} PMARKEN;

/* Prototypes of static helper functions */
static INT16 get_nextUInt16(FILE* lpFile,UINT16 *lpBuf16,INT16 bInit,INT16 nMode);
char* file_strtok(char *strToken, const char *strDelimit);

/**
 * Import of ASCII files.
 *
 * @param sFilename    Name of file to import.
 * @param iDest        Instance where to save imported data.
 * @param sFiletype    Type of file to import.
 * @return O_K if successful, an error code otherwise
 */
INT16 CGEN_PROTECTED CDlpFile_ImportAsciiToData
(
  CDlpFile*   _this,
  const char* sFilename,
  CDlpObject* iDest,
  const char* sFiletype
)
{
  INT16     bSCAN       = FALSE;
  INT16     bEmptyCells = FALSE;                                                /* Watch EVERY delimiter (e.g. CSV)  */
  INT16     nRetV       = 0;
  INT32     i           = 0;
  INT32     j           = 0;
  INT32     nLines      = 0;
  INT32     nComps      = 0;
  INT32     nLRead      = 0;
  INT32     nMaxL       = L_INPUTLINE;
  COMPLEX64 nBuf        = CMPLX(0.0);
  UINT32*   lpSLen      = NULL;
  FILE*     lpFile      = NULL;
  CData*    dDest       = NULL;
  CData*    dVirt1      = NULL;
  CData*    dVirt2      = NULL;
  char*     lpCol       = NULL;
  char*     tx          = NULL;
  char*     lpLine      = NULL;
  char      lpBuf[L_INPUTLINE];
  char      sSep[10];

  /* get pointer to derived instance */
  dDest = AS(CData,iDest);
  DLPASSERT(dDest);

  /* open file */
  lpFile = fopen(sFilename,"rt");
  if (!lpFile) return IERROR(_this,ERR_FILEOPEN,sFilename,"reading",0);

  /* initial line buffer */
  lpLine = (char*)dlp_calloc(L_INPUTLINE+1, sizeof(char));

  /* Determine number of lines and size of line buffer */
  nLines=0;j=0;
  while (fgets(lpLine,L_INPUTLINE+1,lpFile)!= NULL && !ferror(lpFile))
  {
    i=dlp_strlen(lpLine);
    j+=i;
    if ((i>0 && (lpLine[i-1]=='\n' || lpLine[i-1]=='\r')) || feof(lpFile))
    {
      if (j>nMaxL) nMaxL=j+1;
      nLines++;
      j=0;
    }
  }

  /* resize line buffer if necessary */
  if(nMaxL>L_INPUTLINE)
    lpLine = (char*)dlp_realloc(lpLine,nMaxL+1,sizeof(char));

  /* determine field separator from file type */
  if      (strcmp(sFiletype,"csv"   )==0) { bEmptyCells=TRUE; strcpy(sSep,","); }
  else if (strcmp(sFiletype,"csv_de")==0) { bEmptyCells=TRUE; strcpy(sSep,";"); }
  else strcpy(sSep,_this->m_lpsSep);

  /* /////////////////////////////////////////////////////////////////////////////// */
  /* if no components defined determine number and type of columns in file to import */
  /* /////////////////////////////////////////////////////////////////////////////// */

  if(CData_GetNComps(dDest) == 0)
  {
    char* lpCol  = NULL;
    INT32  nNCtr  = 0;
    INT32  nDCtr  = 0;
    INT32  nSCtr  = 0;
    bSCAN = TRUE;

    fseek(lpFile,0,SEEK_SET);

    /* read first line that is not a comment */
    do
    {
      if(NULL == fgets(lpLine,nMaxL,lpFile)) break;
      dlp_strtrimleft(lpLine);
      dlp_strtrimright(lpLine);
    }
    while
    (
      !dlp_strlen(lpLine) ||
      (dlp_strlen(_this->m_lpsComment) && (lpLine == strstr(lpLine,_this->m_lpsComment))) ||
      (dlp_strlen(_this->m_lpsLineFlt) && (lpLine != strstr(lpLine,_this->m_lpsLineFlt)))
    );

    /* scan columns and create components in target instance */
    lpCol = bEmptyCells ? file_strtok(lpLine,sSep) : strtok(lpLine,sSep);

    for(i=0; lpCol != NULL; i++)
    {
      /* try to read field as double */
      if (!_this->m_bStrings && dlp_sscanc(lpCol,&nBuf)==O_K)
      {
        if (strstr(lpCol,".") || strstr(lpCol,","))
        {
          sprintf(lpBuf,"d%03d",(int)(++nDCtr));
          CData_AddComp(dDest,lpBuf,(nBuf.y==0)?T_DOUBLE:T_COMPLEX);
        }
        else
        {
          sprintf(lpBuf,"n%03d",(int)(++nNCtr));
          CData_AddComp(dDest,lpBuf,T_LONG);
        }
      }
      else if(sscanf(lpCol,"%s",lpBuf))
      {
        sprintf(lpBuf,"s%03d",(int)(++nSCtr));
        CData_AddComp(dDest,lpBuf,255);
      }
      else
      {
        fclose(lpFile);
        return IERROR(_this,FIL_PROCESS,"reading",sFilename,0);
      }

      lpCol = bEmptyCells ? file_strtok(NULL,sSep) : strtok(NULL,sSep);
    }
  }


  /* ////////////////////////////////////////////////////////// */
  /* if no records allocated determine number of needed records */
  /* ////////////////////////////////////////////////////////// */

  if(CData_GetNRecs(dDest) == 0)
  {
    CData_Allocate(dDest,nLines);
  }

  fseek(lpFile,0,SEEK_SET);

  nComps = CData_GetNComps(dDest);
  lpSLen = (UINT32*)dlp_calloc(nComps,sizeof(UINT32));  /* array for maximal string length in column */


  /* ////////////////////////// */
  /* read file                  */
  /* ////////////////////////// */

  for(nLRead=0; !feof(lpFile); )
  {
    /* get next line */
    if(NULL == fgets(lpLine,nMaxL,lpFile)) break;

    /* trim trailing line break */
    if (dlp_strlen(lpLine))
    {
      tx = &lpLine[dlp_strlen(lpLine)-1];
      while (tx>lpLine && (*tx=='\n'||*tx=='\r')) {*tx=0;tx--;}
    }

    /* skip comments */
    if(dlp_strlen(_this->m_lpsComment) && (lpLine == strstr(lpLine,_this->m_lpsComment))) continue;

    /* skip lines not starting with line filter string */
    if(dlp_strlen(_this->m_lpsLineFlt) && (lpLine != strstr(lpLine,_this->m_lpsLineFlt))) continue;

    /* skip empty lines */
    if(!dlp_strlen(lpLine)) continue;
    for (tx=lpLine; *tx && iswspace(*tx); tx++) ;
    if (!*tx) continue;

    /* parse line */
    lpCol = bEmptyCells ? file_strtok(lpLine,sSep) : strtok(lpLine,sSep);
    for(j=0; j<nComps; j++)
    {
      /* skip not existing columns in line */
      /* if(bEOL) continue; */

      if (!lpCol) break;

      if (dlp_is_symbolic_type_code(CData_GetCompType(dDest,j)) == TRUE )
      {
        nRetV = sscanf(lpCol,"%s",lpBuf);
        if(nRetV != EOF)
        {
          /* Store empty string if sscanf did not succeed */
          dlp_strcpy(lpBuf,lpCol);
          if(dlp_strlen(lpBuf)>255)
          {
            IERROR(_this,FIL_FORMAT,sFilename,"ascii",
              "symbolic types can hold max 255 characters.");
            lpBuf[255]=0;
          }
          CData_Sstore(dDest, lpBuf, nLRead, j);
        }

        /* remember longest string in column */
        if(dlp_strlen(lpBuf) > lpSLen[j]) lpSLen[j] = dlp_strlen(lpBuf);
      }
      else
      {
        if (dlp_is_float_type_code(CData_GetCompType(dDest,j)))
          dlp_strreplace(lpCol,",",".");

        if(strlen(lpCol) > 0)
        {
          nRetV = dlp_sscanc(lpCol,&nBuf);
          /* Store 0 if sscanf did not succeed */
          if(nRetV != O_K) nBuf = CMPLX(0);
          CData_Cstore (dDest, nBuf, nLRead, j);
        }
      }
      lpCol = bEmptyCells ? file_strtok(NULL,sSep) : strtok(NULL,sSep);
    }
    nLRead++;
  }
  CData_SetNRecs(dDest,nLRead);
  CData_Realloc(dDest,nLRead);

  /* test for error during file read */
  if(ferror(lpFile)) IERROR(_this,FIL_PROCESS,"reading",sFilename,0);

  /* Close file */
  fclose(lpFile);

  if(bSCAN)
  {
    /* ////////////////////////// */
    /* shrink symbolic components */
    /* ////////////////////////// */

    ICREATEEX(CData,dVirt1,"~vitual1",NULL);
    ICREATEEX(CData,dVirt2,"~vitual2",NULL);

    for(i=0;i<nComps;i++)
    {
      if(dlp_is_symbolic_type_code(CData_GetCompType(dDest,i)))
      {
        INT16 nSLen      = 0;
        INT32  nReclnFrom = 0;
        INT32  nReclnTo   = 0;
        INT32  nComp      = 0;
        BYTE* lpFrom     = NULL;
        BYTE* lpTo       = NULL;

        if(lpSLen[i] <= 0)  continue;
        if(lpSLen[i] > 250) nSLen = 255; /* 250           */
        else nSLen = (INT16)lpSLen[i]+5; /*     + 5 = 255 */

        CData_AddComp(dVirt2,CData_GetCname(dDest,i),nSLen);

        nComp = CData_GetNComps(dVirt2);
        if(nComp==1) CData_Allocate(dVirt2,nLRead);

        lpTo       = CData_XAddr(dVirt2,0,nComp-1);
        lpFrom     = CData_XAddr(dDest,0,i);
        nReclnTo   = CData_GetRecLen(dVirt2);
        nReclnFrom = CData_GetRecLen(dDest);

        DLPASSERT(lpTo);
        DLPASSERT(lpFrom);

        for(j=0; j<nLRead; j++)
        {
          dlp_memmove(lpTo,lpFrom,nSLen*sizeof(char));
          lpFrom += nReclnFrom;
          lpTo   += nReclnTo;
        }
      }
      else
      {
        CData_SelectComps(dVirt1,dDest,i,1);
        CData_Join(dVirt2,dVirt1);
      }
    }

    CData_Reset(BASEINST(dDest),TRUE);
    CData_Copy(BASEINST(dDest),BASEINST(dVirt2));

    IDESTROY(dVirt1);
    IDESTROY(dVirt2);
  }

  dlp_free(lpSLen);
  dlp_free(lpLine);

  DLP_CHECK_MEMINTEGRITY;

  return O_K;
}

/**
 * Export of data instances to ASCII files.
 *
 * @param sFilename   Name of file to export.
 * @param iSrc        Instance containing data to export.
 * @param sFiletype   Type of file to export.
 * @return O_K if successful, an error code otherwise
 */
INT16 CGEN_PROTECTED CDlpFile_ExportAsciiFromData
(
  CDlpFile*     _this,
  const char*   sFilename,
  CDlpObject* iSrc,
  const char*   sFiletype
)
{
  INT32  iR      = 0;
  INT32  iC      = 0;
  INT32  nR      = 0;
  INT32  nC      = 0;
  INT32  i       = 0;
  INT32  j       = 0;
  CData* dSrc    = NULL;
  FILE*  lpFile  = NULL;
  BOOL   bGerman = FALSE; /* For CSV export */
  char   sSep[2];
  char   sBuf[256];

  /* get pointer to derived instance */
  dSrc = AS(CData,iSrc);
  DLPASSERT(dSrc);

  /* if field separator is set first character in m_lpsSep is */
  /* used as separator, otherwise '\t' is used as separator  */
  if      (strcmp(sFiletype,"csv"   )==0) { sSep[0]=','; bGerman=FALSE; }
  else if (strcmp(sFiletype,"csv_de")==0) { sSep[0]=';'; bGerman=TRUE;  }
  else sSep[0]=(dlp_strlen(_this->m_lpsSep)>0)?_this->m_lpsSep[0]:'\t';
  sSep[1] = 0;

  if(_this->m_bAppend == TRUE) {
    lpFile = fopen(sFilename,"a+t");
  } else {
    lpFile = fopen(sFilename,"wt");
  }
  if(!lpFile) return IERROR(_this,ERR_FILEOPEN,sFilename,"writing",0);

  if(_this->m_bTranspose) { nC = CData_GetNRecs(dSrc); nR = CData_GetNComps(dSrc); }
  else                    { nR = CData_GetNRecs(dSrc); nC = CData_GetNComps(dSrc); }

  for (iR=0; iR<nR; iR++)
  {
    for (iC=0; iC<nC; iC++)
    {
      if(_this->m_bTranspose) { i = iC; j = iR; }
      else                    { i = iR; j = iC; }

      if (j>0) fputs(sSep,lpFile);
      if(dlp_is_symbolic_type_code(CData_GetCompType(dSrc,j))) {
        dlp_memmove(sBuf,CData_XAddr(dSrc,i,j),CData_GetCompType(dSrc,j));
        sBuf[CData_GetCompType(dSrc,j)-1]='\0';
      } else {
        switch (CData_GetCompType(dSrc,j))
        {
          case T_UCHAR    :
          case T_USHORT   :
          case T_UINT     :
          case T_ULONG    : sprintf(sBuf,"%lu", (unsigned long)CData_Dfetch(dSrc,i,j));break;
          case T_BOOL     :
          case T_CHAR     :
          case T_SHORT    :
          case T_INT      :
          case T_LONG     : sprintf(sBuf,"%ld",(long)CData_Dfetch(dSrc,i,j));break;
          case T_FLOAT    :
          case T_DOUBLE   :
          case T_COMPLEX  : dlp_sprintc(sBuf, CData_Cfetch(dSrc,i,j),TRUE);break;
          case T_PTR      :
          case T_INSTANCE : sprintf(sBuf,"%p", CData_XAddr(dSrc,i,j));break;
          default: DLPASSERT(FMSG("Unknown component type."));
        }
      }
      if (bGerman) dlp_strreplace(sBuf,".",",");
      fputs(sBuf,lpFile);
    }
    fputs("\n",lpFile);
  }

  fclose(lpFile);

  return O_K;
}

#define SF_READWRITE_INTEGER(A,B,C,D,E,F) \
  switch(sizeof(A)) { \
    case 1: retVal = sf_ ##F## _raw  (B,(void* )C,D);break; \
    case 2: retVal = sf_ ##F## _short(B,(short*)C,D);break; \
    case 4: retVal = sf_ ##F## _int  (B,(int*  )C,D);break; \
    default: IERROR(_this,FIL_FORMAT,sFilename,sFiletype," ##F## "); IDESTROY(E); return NOT_EXEC; \
  }
#define SF_READWRITE_FLOAT(A,B,C,D,E,F) \
  switch(sizeof(A)) { \
    case 4: retVal = sf_ ##F## _float (B,(float* )C,D);break; \
    case 8: retVal = sf_ ##F## _double(B,(double*)C,D);break; \
    default: IERROR(_this,FIL_FORMAT,sFilename,sFiletype," ##F## "); IDESTROY(E); return NOT_EXEC; \
  }

/**
 * Import of sound files using libsndfile.
 *
 * @param sFilename    Name of file to import.
 * @param iDest        Instance where to save imported data.
 * @param sFiletype    Type of file to import.
 * @return O_K if successful, an error code otherwise
 */
INT16 CGEN_PROTECTED CDlpFile_LibsndfileImport
(
  CDlpFile*     _this,
  const char*   sFilename,
  CDlpObject*   iDest,
  const char*   sFiletype
)
{
#ifndef __NOLIBSNDFILE
  INT32 nChn = 0;
  CData* dDest = NULL;
  char   sBuf2[16];
  sf_count_t retVal = 0;
  SF_INFO sf_info = { 0, 0, 0, 0, 0, 0 };
  SNDFILE* file;

  /* get pointer to derived instance */
  dDest = AS(CData,iDest);
  DLPASSERT(dDest);

  file = sf_open(sFilename, SFM_READ, &sf_info);
  if(!file) return IERROR(_this,ERR_FILEOPEN,sFilename,"reading",0);

  /* prepare destination instance */
  CData_Reset(iDest,TRUE);
  for (nChn=0; nChn<sf_info.channels; nChn++)
  {
    sprintf(sBuf2,"ch%ld",(long)nChn);
    switch(sf_info.format & SF_FORMAT_SUBMASK) {
    case SF_FORMAT_PCM_U8: { CData_AddComp(dDest,sBuf2,T_UCHAR); break; }
    case SF_FORMAT_PCM_S8: { CData_AddComp(dDest,sBuf2,T_CHAR);  break; }
    case SF_FORMAT_PCM_16: { CData_AddComp(dDest,sBuf2,T_SHORT); break; }
    case SF_FORMAT_PCM_24: { CData_AddComp(dDest,sBuf2,T_INT);   CSignal_SetScale(dDest,CMPLX(1./32768.)); break; }
    case SF_FORMAT_PCM_32: { CData_AddComp(dDest,sBuf2,T_INT);   break; }
    case SF_FORMAT_FLOAT : { CData_AddComp(dDest,sBuf2,T_FLOAT); break; }
    case SF_FORMAT_DOUBLE: { CData_AddComp(dDest,sBuf2,T_DOUBLE);break; }
      default:
        IERROR(_this,FIL_FORMAT,sFilename,sFiletype,0);
        return NOT_EXEC;
    }
  }

  CData_AllocateUninitialized(dDest, sf_info.frames);

  switch(CData_GetCompType(dDest,0)) {
    case T_UCHAR : SF_READWRITE_INTEGER(  UINT8,file,CData_XAddr(dDest,0,0), sf_info.frames*sf_info.channels,dDest,read);break;
    case T_CHAR  : SF_READWRITE_INTEGER(   INT8,file,CData_XAddr(dDest,0,0), sf_info.frames*sf_info.channels,dDest,read);break;
    case T_USHORT: SF_READWRITE_INTEGER( UINT16,file,CData_XAddr(dDest,0,0), sf_info.frames*sf_info.channels,dDest,read);break;
    case T_SHORT : SF_READWRITE_INTEGER(  INT16,file,CData_XAddr(dDest,0,0), sf_info.frames*sf_info.channels,dDest,read);break;
    case T_UINT  : SF_READWRITE_INTEGER( UINT32,file,CData_XAddr(dDest,0,0), sf_info.frames*sf_info.channels,dDest,read);break;
    case T_INT   : SF_READWRITE_INTEGER(  INT32,file,CData_XAddr(dDest,0,0), sf_info.frames*sf_info.channels,dDest,read);break;
    case T_ULONG : SF_READWRITE_INTEGER( UINT64,file,CData_XAddr(dDest,0,0), sf_info.frames*sf_info.channels,dDest,read);break;
    case T_LONG  : SF_READWRITE_INTEGER(  INT64,file,CData_XAddr(dDest,0,0), sf_info.frames*sf_info.channels,dDest,read);break;
    case T_FLOAT:  SF_READWRITE_FLOAT  (FLOAT32,file,CData_XAddr(dDest,0,0), sf_info.frames*sf_info.channels,dDest,read);break;
    case T_DOUBLE: SF_READWRITE_FLOAT  (FLOAT64,file,CData_XAddr(dDest,0,0), sf_info.frames*sf_info.channels,dDest,read);break;
    default:
      IERROR(_this,FIL_FORMAT,sFilename,sFiletype,0);
      IDESTROY(dDest);
      return NOT_EXEC;
  }
  if(retVal!= sf_info.frames*sf_info.channels) {
    IERROR(_this,FIL_IMPORT,sFilename,sFiletype,0);
    sf_close(file);
    return NOT_EXEC;
  }

  /* remember sampling rate */
  ISETFIELD_RVALUE(dDest,"fsr",1000.0/sf_info.samplerate);

  /* close soundstream */
  sf_close(file);

  return TRUE;
#else /* __NOLIBSNDFILE */
  return CDlpFile_ImportWaveToData(_this,sFilename,iDest,sFiletype);
#endif /* __NOLIBSNDFILE */
}

/**
 * Export of sound files using libsndfile.
 *
 * @param sFilename   Name of file to export.
 * @param iSrc        Instance containing data to export.
 * @param sFiletype   Type of file to export.
 * @return O_K if successful, an error code otherwise
 */
INT16 CGEN_PROTECTED CDlpFile_LibsndfileExport
(
  CDlpFile*     _this,
  const char*   sFilename,
  CDlpObject*   iSrc,
  const char*   sFiletype
)
{
#ifndef __NOLIBSNDFILE
  INT32    i       = 0;
  INT32    nRecs   = 0;
  CData*     dSrc    = NULL;
  CData*     dVirt   = NULL;
  CData*     dAux    = NULL;
  char       _sFilename[L_PATH];
  char       _sFiletype[48];
  sf_count_t retVal = 0;
  SF_INFO    sf_info;
  SNDFILE*   file;

  /* initialize */
  dlp_strcpy(_sFilename,sFilename);
  dlp_strcpy(_sFiletype,sFiletype);

  /* get pointer to derived instance */
  dSrc = AS(CData,iSrc);
  DLPASSERT(dSrc);

  ICREATEEX(CData,dVirt,"~virt",NULL);
  ICREATEEX(CData,dAux,"~aux",NULL);

  /* Seek first numeric component */
  if (CData_GetNNumericComps(dSrc)==0)
    IERROR(_this,FIL_NOTFOUND,"a numeric component in source",0,0);
  for (i=0; i<CData_GetNComps(dSrc); i++) {
    if (dlp_is_numeric_type_code(CData_GetCompType(dSrc,i))) {
      if(CData_GetCompType(dSrc,i) != CData_GetCompType(dSrc,0)) {
        IERROR(_this,FIL_FORMATCOMPS,BASEINST(dSrc)->m_lpInstanceName,0,0);
        IDESTROY(dVirt);
        IDESTROY(dAux);
        return NOT_EXEC;
      }
      CData_SelectComps(dAux,dSrc,i,1);
      CData_Join(dVirt, dAux);
    }
  }
  IDESTROY(dAux);
  DLPASSERT(CData_GetNComps(dVirt)>0);

  if     (!strcmp(sFiletype, "wav"))  sf_info.format = SF_FORMAT_WAV;
  else if(!strcmp(sFiletype, "au"))   sf_info.format = SF_FORMAT_AU;
  else if(!strcmp(sFiletype, "aiff")) sf_info.format = SF_FORMAT_AIFF;
  else {
    IERROR(_this,FIL_FORMAT,sFilename,sFiletype,0);
    IDESTROY(dVirt);
    return NOT_EXEC;
  }

  switch(CData_GetCompType(dVirt,0)) {
    case T_UCHAR : sf_info.format |= (sizeof(  UINT8)==1 ? SF_FORMAT_PCM_U8 : sizeof( UINT8)==2 ? SF_FORMAT_PCM_16 : sizeof( UINT8)==4 ? SF_FORMAT_PCM_32 : 0);break;
    case T_CHAR  : sf_info.format |= (sizeof(   INT8)==1 ? SF_FORMAT_PCM_S8 : sizeof(  INT8)==2 ? SF_FORMAT_PCM_16 : sizeof(  INT8)==4 ? SF_FORMAT_PCM_32 : 0);break;
    case T_USHORT: sf_info.format |= (sizeof( UINT16)==1 ? SF_FORMAT_PCM_U8 : sizeof(UINT16)==2 ? SF_FORMAT_PCM_16 : sizeof(UINT16)==4 ? SF_FORMAT_PCM_32 : 0);break;
    case T_SHORT : sf_info.format |= (sizeof(  INT16)==1 ? SF_FORMAT_PCM_S8 : sizeof( INT16)==2 ? SF_FORMAT_PCM_16 : sizeof( INT16)==4 ? SF_FORMAT_PCM_32 : 0);break;
    case T_UINT  : sf_info.format |= (sizeof( UINT32)==1 ? SF_FORMAT_PCM_U8 : sizeof(UINT32)==2 ? SF_FORMAT_PCM_16 : sizeof(UINT32)==4 ? SF_FORMAT_PCM_32 : 0);break;
    case T_INT   : sf_info.format |= (sizeof(  INT32)==1 ? SF_FORMAT_PCM_S8 : sizeof( INT32)==2 ? SF_FORMAT_PCM_16 : sizeof( INT32)==4 ? SF_FORMAT_PCM_32 : 0);break;
    case T_ULONG : sf_info.format |= (sizeof( UINT64)==1 ? SF_FORMAT_PCM_U8 : sizeof(UINT64)==2 ? SF_FORMAT_PCM_16 : sizeof(UINT64)==4 ? SF_FORMAT_PCM_32 : 0);break;
    case T_LONG  : sf_info.format |= (sizeof(  INT64)==1 ? SF_FORMAT_PCM_S8 : sizeof( INT64)==2 ? SF_FORMAT_PCM_16 : sizeof( INT64)==4 ? SF_FORMAT_PCM_32 : 0);break;
    case T_FLOAT : sf_info.format |= (sizeof(FLOAT32)==4 ? SF_FORMAT_FLOAT : sizeof(FLOAT32)==8 ? SF_FORMAT_DOUBLE : 0);break;
    case T_DOUBLE: sf_info.format |= (sizeof(FLOAT64)==4 ? SF_FORMAT_FLOAT : sizeof(FLOAT64)==8 ? SF_FORMAT_DOUBLE : 0);break;
    default:
      IERROR(_this,FIL_FORMAT,sFilename,sFiletype,0);
      IDESTROY(dVirt);
      return NOT_EXEC;
  }

  if(_this->m_bLittle) sf_info.format |= SF_ENDIAN_LITTLE;
  else if(_this->m_bBig) sf_info.format |= SF_ENDIAN_BIG;
  else sf_info.format |= SF_ENDIAN_FILE;

  sf_info.channels = CData_GetNComps(dVirt);
  sf_info.samplerate = (int)((1000.0/CData_GetDescr(dSrc,RINC)) + 0.5);         /* +0.5 forces cast to round up or down */
  sf_info.seekable = 1;
  sf_info.sections = 0;

  nRecs  = CData_GetNRecs(dVirt);

  if(!sf_format_check(&sf_info)) {
    IERROR(_this,FIL_FORMAT,sFilename,sFiletype,"unsupported");
    IDESTROY(dVirt);
    return NOT_EXEC;
  }

  file = sf_open(sFilename, SFM_WRITE, &sf_info);

  if (!file) {
    IERROR(_this,ERR_FILEOPEN,sFilename,"writing",0);
    IDESTROY(dVirt);
    return ERR_FILEOPEN;
  }

  switch(CData_GetCompType(dVirt,0)) {
    case T_UCHAR : SF_READWRITE_INTEGER(  UINT8,file,CData_XAddr(dVirt,0,0), nRecs*sf_info.channels,dVirt,write);break;
    case T_CHAR  : SF_READWRITE_INTEGER(   INT8,file,CData_XAddr(dVirt,0,0), nRecs*sf_info.channels,dVirt,write);break;
    case T_USHORT: SF_READWRITE_INTEGER( UINT16,file,CData_XAddr(dVirt,0,0), nRecs*sf_info.channels,dVirt,write);break;
    case T_SHORT : SF_READWRITE_INTEGER(  INT16,file,CData_XAddr(dVirt,0,0), nRecs*sf_info.channels,dVirt,write);break;
    case T_UINT  : SF_READWRITE_INTEGER( UINT32,file,CData_XAddr(dVirt,0,0), nRecs*sf_info.channels,dVirt,write);break;
    case T_INT   : SF_READWRITE_INTEGER(  INT32,file,CData_XAddr(dVirt,0,0), nRecs*sf_info.channels,dVirt,write);break;
    case T_ULONG : SF_READWRITE_INTEGER( UINT64,file,CData_XAddr(dVirt,0,0), nRecs*sf_info.channels,dVirt,write);break;
    case T_LONG  : SF_READWRITE_INTEGER(  INT64,file,CData_XAddr(dVirt,0,0), nRecs*sf_info.channels,dVirt,write);break;
    case T_FLOAT:  SF_READWRITE_FLOAT  (FLOAT32,file,CData_XAddr(dVirt,0,0), nRecs*sf_info.channels,dVirt,write);break;
    case T_DOUBLE: SF_READWRITE_FLOAT  (FLOAT64,file,CData_XAddr(dVirt,0,0), nRecs*sf_info.channels,dVirt,write);break;
    default:
      IERROR(_this,FIL_FORMAT,sFilename,sFiletype,0);
      IDESTROY(dVirt);
      return NOT_EXEC;
  }
  if(retVal!= nRecs*sf_info.channels) {
    IERROR(_this,FIL_EXPORT,sFilename,sFiletype,0);
    IDESTROY(dVirt);
    sf_close(file);
    return NOT_EXEC;
  }

  /* cleanup */
  sf_close(file);
  IDESTROY(dVirt);

  return O_K;
#else /*__NOLIBSNDFILE */
  return  NOT_EXEC;
#endif /*__NOLIBSNDFILE */
}



/**
 * Import of PhonDat files.
 *
 * @param sFilename    Name of file to import.
 * @param iDest        Instance where to save imported data.
 * @param sFiletype    Type of file to import.
 * @return O_K if successful, an error code otherwise
 */
INT16 CGEN_PROTECTED CDlpFile_ImportPhDToData
(
  CDlpFile*     _this,
  const char*   sFilename,
  CDlpObject* iDest,
  const char*   sFiletype
)
{
  INT16*         sample      = NULL;
  char           chr         = 0;
  INT32          swap        = 0;
  INT32          samplecount = 0;
  INT32          i           = 0;
  char*          ortho       = NULL;
  char*          cano        = NULL;
  char*          charpoint   = NULL;
  CData*         dDest       = NULL;
  FILE*          lpFile      = NULL;
  Phon_header_2* header      = NULL;

  /* get pointer to derived instance */
  dDest = AS(CData,iDest);
  DLPASSERT(dDest);

  /* prepare destination instance */
  CData_Reset(iDest,TRUE);
  CData_AddComp(dDest,"phd",T_SHORT);

  lpFile = fopen(sFilename, "rb");
  if (!lpFile) return IERROR(_this,ERR_FILEOPEN,sFilename,"reading",0);

  /* read the phondat header */
#ifdef __SPARC
  if((header = read_header_sun(lpFile,&ortho,&cano)) == NULL)
    return IERROR(_this,FIL_PROCESS,"reading",sFilename,0);
#else
  if((header = read_header_vms(lpFile,&ortho,&cano)) == NULL)
    return IERROR(_this,FIL_PROCESS,"reading",sFilename,0);
#endif

  /* Test for valid sample frequency in header */
  if(header->isf<=0)
  {
    IERROR(_this,FIL_PHD_EMPTY_HEADER,0,0,0);
    /* HACK: Assume 16kHz if header is empty */
    header->isf = 16000;
  }

  /* set sampling rate */
  ISETFIELD_RVALUE(dDest,"fsr",1000.0/header->isf);

  /* copy samples */
  fseek(lpFile,0L,SEEK_END);
  samplecount = (ftell(lpFile) - header->anz_header*512) / sizeof(INT16);
  fseek(lpFile,header->anz_header*512,SEEK_SET);

  sample = (INT16*)dlp_calloc(samplecount, sizeof(INT16));

  if(fread(sample,sizeof(INT16),samplecount,lpFile) != (size_t)samplecount)  {
    IERROR(_this,FIL_PROCESS,"reading",sFilename,0);
  }
  if(CData_GetNRecs(dDest) < samplecount) {
    CData_Realloc(dDest,samplecount);
  }
  /* swap samples if necessary */
  if(swap) {
    for(i = 0; i < samplecount; i++) {
      charpoint = (char *)(&sample[i]);
      chr = charpoint[0];
      charpoint[0] = charpoint[1];
      charpoint[1] = chr;
    }
  }
  for(i = 0; i < samplecount; i++) {
    CData_Dstore(dDest,(FLOAT64)sample[i],i,0);
  }
  CData_IncNRecs(dDest,samplecount);

  IFCHECK {fprintf(stderr,"%s: read %ld bytes\n","Import PhonDat", (long)samplecount*2);fflush(stderr);}
  if (samplecount != header->nspbk*256)
    IERROR(_this,FIL_PHD_SMM,samplecount,header->nspbk*256,0);

  fclose(lpFile);
  dlp_free(sample);
  dlp_free(header);
  dlp_free(ortho);
  dlp_free(cano);

  return O_K;
}

/**
 * <p>Import of ESPS label files.</p>
 * <h3>Note</h3><p>More than one label channels are supported.</p>
 *
 * @param sFilename    Name of file to import.
 * @param iDest        Instance where to save imported data.
 * @param sFiletype    Type of file to import.
 * @return O_K if successful, an error code otherwise
 */
INT16 CGEN_PROTECTED CDlpFile_ImportEspsLabToData
(
  CDlpFile*     _this,
  const char*   sFilename,
  CDlpObject*   iDest,
  const char*   sFiletype
)
{
  INT16   nRet   = 0;
  INT16   bSCAN  = TRUE;
  INT32   i      = 0;
  INT32   j      = 0;
  INT32   nComps = 0;
  INT32   nLRead = 0;
  UINT32* lpSLen = NULL;
  FLOAT64 nRinc  = 0.0;
  double  nBuf   = 0.0;
  FILE*   lpFile = NULL;
  CData*  dDest  = NULL;
  CData*  dVirt1 = NULL;
  CData*  dVirt2 = NULL;
  char*   lpCol  = NULL;
  char*   tx     = NULL;
  char    lpBuf[L_INPUTLINE];
  char    lpLine[L_INPUTLINE];
  char    lpSep[2] = " ";


  /* get pointer to derived instance */
  dDest = AS(CData,iDest);
  DLPASSERT(dDest);
  nRinc = 1000.0/dDest->m_lpTable->m_fsr;
  CData_Reset(iDest,TRUE);

  lpFile = fopen(sFilename,"rt");
  if (!lpFile) return IERROR(_this,ERR_FILEOPEN,sFilename,"reading",0);

  /* read the header */
  /* TODO: Save separator character in dDest->m_ftext */
  /*       evaluate alle header information           */
  do{
    if(NULL == fgets(lpLine,L_INPUTLINE-1,lpFile)) break;
    nRet=sscanf(lpLine,"separator %c",lpSep);lpSep[1]=0;
    if(nRet>0 && BASEINST(_this)->m_nCheck>0)
      printf("\nSeparator '%s' found in header.\n",lpSep);
  }while(!feof(lpFile) && lpLine[0] != '#');

  if (feof(lpFile))
  {
    fclose(lpFile);
    return
      IERROR(_this,FIL_FORMAT,sFilename,sFiletype,"end of header not found");
  }

  /* first component is time in ms, second component color */
  CData_AddComp(dDest,"time",T_DOUBLE);
  CData_AddComp(dDest,"colo",T_INT);

  /* scan columns and create label components in target instance */
  if(!fgets(lpLine,L_INPUTLINE-1,lpFile))
  {
    fclose(lpFile);
    return IERROR(_this,FIL_FORMAT,sFilename,sFiletype,"No data found");
  }
  lpCol = strtok(lpLine," ");
  lpCol = strtok(NULL," ");   /* skip time and color */
  lpCol = strtok(NULL," ");

  for(i=0; lpCol != NULL; i++)
  {
    if(sscanf(lpCol,"%s",lpBuf))
    {
      sprintf(lpBuf,"lab%1ld",(long)i);
      CData_AddComp(dDest,lpBuf,255);
    }
    else
    {
      fclose(lpFile);
      return IERROR(_this,FIL_PROCESS,"reading",sFilename,0);
    }

    lpCol = strtok(NULL," ");
  }

  fseek(lpFile,0,SEEK_SET);


  /* ////////////////////////////////////////////////////////// */
  /* determine number of records needed                         */
  /* ////////////////////////////////////////////////////////// */

  /* skip header */
  do{
    if(NULL == fgets(lpLine,L_INPUTLINE-1,lpFile)) break;
  }while(dlp_strlen(lpLine) && !feof(lpFile) && lpLine[0] != '#');

  for(i=0; fgets(lpLine,L_INPUTLINE-1,lpFile) != NULL && !ferror(lpFile); i++) ;
  CData_Allocate(dDest,i);
  fseek(lpFile,0,SEEK_SET);

  IFCHECK CData_Status(dDest);

  /* ////////////////////////// */
  /* read file                  */
  /* ////////////////////////// */

  nComps = CData_GetNComps(dDest);
  lpSLen = (UINT32*)dlp_calloc(nComps,sizeof(UINT32));  /* array for maximal string length in column */

  /* skip header */
  do{
    if(NULL == fgets(lpLine,L_INPUTLINE-1,lpFile)) break;
  }while(dlp_strlen(lpLine) && !feof(lpFile) && lpLine[0] != '#');

  for(nLRead=0; !feof(lpFile); )
  {
    /* get next line */
    if(NULL == fgets(lpLine,L_INPUTLINE-1,lpFile)) break;

    /* trim trailing line break */
    if (dlp_strlen(lpLine))
    {
      tx = &lpLine[dlp_strlen(lpLine)-1];
      while (tx>lpLine && (*tx=='\n'||*tx=='\r')){*tx=0;tx--;}
    }

    /* skip empty lines */
    if(!dlp_strlen(lpLine)          ) continue;
    for (tx=lpLine; *tx && iswspace(*tx); tx++) ;
    if (!*tx) continue;

    lpCol = strtok(lpLine," ");

    /* parse line */
    for(j=0; j<nComps; j++)
    {
      /* skip not existing columns in line */
      /* if(bEOL) continue; */

      if(!lpCol) break;

      if (dlp_is_symbolic_type_code(CData_GetCompType(dDest,j)) == TRUE )
      {
        if(sscanf(lpCol,"%s",lpBuf) != EOF)
        {
          dlp_strreplace(lpBuf,lpSep,"");
          if(!dlp_strlen(lpBuf))
          {
            j--;
            lpCol = strtok(NULL," ");
            continue;
          }
          CData_Sstore(dDest, lpBuf, nLRead, j);
        }

        /* remember longest string in column */
        if(dlp_strlen(lpBuf) > lpSLen[j]) lpSLen[j] = dlp_strlen(lpBuf);
      }
      else
      {
        if(sscanf(lpCol,"%lG",&nBuf) != EOF)
          CData_Dstore (dDest, (FLOAT64)nBuf, nLRead, j);
      }
      lpCol = strtok(NULL," ");
    }
    nLRead++;
  }
  CData_SetNRecs(dDest,nLRead);
  CData_Realloc(dDest,nLRead);

  /* test for error during file read */
  if (ferror(lpFile)) IERROR(_this,FIL_PROCESS,"reading",sFilename,0);

  /* Close file */
  fclose(lpFile);

  if(bSCAN)
  {
    /* ////////////////////////// */
    /* shrink symbolic components */
    /* ////////////////////////// */

    ICREATEEX(CData,dVirt1,"~vitual1",NULL);
    ICREATEEX(CData,dVirt2,"~vitual2",NULL);

    for(i=0;i<nComps;i++)
    {
      if(dlp_is_symbolic_type_code(CData_GetCompType(dDest,i)))
      {
        INT16 nSLen      = 0;
        INT32  nReclnFrom = 0;
        INT32  nReclnTo   = 0;
        INT32  nComp      = 0;
        BYTE* lpFrom     = NULL;
        BYTE* lpTo       = NULL;

        if(lpSLen[i] <= 0)  continue;
        if(lpSLen[i] > 255) nSLen = 255;
        else nSLen = (INT16)lpSLen[i]+5;

        CData_AddComp(dVirt2,CData_GetCname(dDest,i),nSLen);

        nComp = CData_GetNComps(dVirt2);
        if(nComp==1) CData_Allocate(dVirt2,nLRead);

        lpTo       = CData_XAddr(dVirt2,0,nComp-1);
        lpFrom     = CData_XAddr(dDest,0,i);
        nReclnTo   = CData_GetRecLen(dVirt2);
        nReclnFrom = CData_GetRecLen(dDest);

        DLPASSERT(lpTo);
        DLPASSERT(lpFrom);

        for(j=0; j<nLRead; j++)
        {
          dlp_memmove(lpTo,lpFrom,nSLen*sizeof(char));
          lpFrom += nReclnFrom;
          lpTo   += nReclnTo;
        }
      }
      else
      {
        CData_SelectComps(dVirt1,dDest,i,1);
        CData_Join(dVirt2,dVirt1);
      }
    }

    CData_Reset(BASEINST(dDest),TRUE);
    CData_Copy(BASEINST(dDest),BASEINST(dVirt2));

    if(_this->m_bCompress && (nRinc > 0)) {
      CData_SelectComps(dVirt1,dDest,0,1);
      CMatrix_Op(dVirt1,dVirt1,T_INSTANCE,&nRinc,T_DOUBLE,OP_MULT_EL);
      if(CData_Dfetch(dVirt1,0,0) != 0) {
        CData_InsertRecs(dVirt1,0,1,1);
      }
      ISETOPTION(dVirt2,"/rec");
      CData_Shift(dVirt2,dVirt1,-1);
      CMatrix_Op(dVirt2,dVirt2,T_INSTANCE,dVirt1,T_INSTANCE,OP_DIFF);
      CData_Dstore(dVirt2,1,CData_GetNRecs(dVirt2)-1,0);
      CData_DeleteComps(dDest,0,2);
      CData_Join(dVirt1,dVirt2);
      CMatrix_Op(dVirt1,dVirt1,T_INSTANCE,NULL,T_IGNORE,OP_ROUND);
      CData_Join(dDest,dVirt1);
      CData_SetCname(dDest,1,"start");
      CData_SetCname(dDest,2,"count");
    }
    IDESTROY(dVirt1);
    IDESTROY(dVirt2);
  }

  dlp_free(lpSLen);

  return O_K;
}

/**
 * Export of ESPS label files.
 *
 * @param sFilename   Name of file to export.
 * @param iSrc        Instance containing data to export.
 * @param sFiletype   Type of file to export.
 * @return O_K if successful, an error code otherwise
 */
INT16 CGEN_PROTECTED CDlpFile_ExportEspsLabFromData
(
  CDlpFile*   _this,
  const char* sFilename,
  CDlpObject* iSrc,
  const char* sFiletype
)
{
  INT32   i     = 0;                                                             /* Universal loop counter            */
  INT32   nIcLab = -1;                                                           /* Index of symbolic (=label) comp.  */
  FLOAT64 nRinc  = -1.0;                                                         /* Continuation rate [s]             */
  FLOAT64 nRofs  = 0.0;                                                          /* Offset of first sample [s]        */
  CData* idSrc  = NULL;                                                         /* Source data instance              */
  FILE*  lpFile = NULL;                                                         /* Output file                       */
  char   sCurrLab[L_SSTR+1];                                                    /* Current label                     */
  char   sPrevLab[L_SSTR+1];                                                    /* Previous label                    */

  /* Initialize */                                                              /* --------------------------------- */
  idSrc = (CData*)CDlpObject_OfKind("data",iSrc);                               /* Get pointer to derived instance   */
  DLPASSERT(idSrc);                                                             /* This cannot happen                */
  for (i=0; i<CData_GetNComps(idSrc); i++)                                      /* Search first symbolic component   */
    if (dlp_is_symbolic_type_code(CData_GetCompType(idSrc,i)))                  /*   Current compontent is symbolic  */
    {                                                                           /*   >>                              */
      nIcLab = i;                                                               /*     Remember component index      */
      break;                                                                    /*     Stop                          */
    }                                                                           /*   <<                              */
  if (nIcLab<0)                                                                 /* No label component found          */
    return IERROR(_this,FIL_NOTFOUND,"a symbolic component in source",0,0);     /*   Forget it                       */
  nRinc = idSrc->m_lpTable->m_fsr / 1000;                                       /* Get continuation rate in sec.     */
  if (nRinc<=0.0) {
    lpFile = fopen(sFilename,"wt");                                             /* Open file                         */
    if (!lpFile) return IERROR(_this,ERR_FILEOPEN,sFilename,"writing",0);       /* Need an open file :(              */
    fprintf(lpFile, "Created by dLabPro.\n");                                     /* Write file header                 */
    fprintf(lpFile, "File format: ESPS/waves+\n");                              /* Write file header                 */
    fprintf(lpFile,"#\n");                                                        /* # marks the beginning of labels   */

    for (i=0; i<CData_GetNRecs(idSrc); i++)                                     /* Loop over labels                  */
    {                                                                           /* >>                                */
      dlp_strcpy(sCurrLab,(char*)CData_XAddr(idSrc,i,nIcLab));                  /*   Get current label               */
      fprintf(lpFile,"%12.8f %3d %s\n",
          (double)CData_Dfetch(idSrc,i,0),                                      /*     Write label time              */
          (int)CData_Dfetch(idSrc,i,1),                                         /*     Write label color             */
          CData_Sfetch(idSrc,i,2));                                             /*     Write label name              */
    }                                                                           /* <<                                */
    fclose(lpFile);                                                             /* Close file                        */
  } else {
    nRofs = idSrc->m_lpTable->m_ofs / 1000;                                       /* Get offset of first record in sec.*/

    /* Write ESPS label file */                                                   /* --------------------------------- */
    lpFile = fopen(sFilename,"wt");                                               /* Open file                         */
    if (!lpFile) return IERROR(_this,ERR_FILEOPEN,sFilename,"writing",0);         /* Need an open file :(              */
    fprintf(lpFile, "Created by dLabPro.");                                       /* Write file header                 */
    fprintf(lpFile, "File format:                ESPS/waves+");                   /* Write file header                 */
    fprintf(lpFile, "Continuation rate [s]:      %G \n",(double)nRinc);                   /* Write file header                 */
    fprintf(lpFile, "Offset of first record [s]: %G \n",(double)nRofs);                   /* Write file header                 */
    fprintf(lpFile, "Lenght [s]:                 %G \n",                          /* Write file header                 */
        ((double)(nRinc*CData_GetNRecs(idSrc))));                                   /* |                                 */
    fprintf(lpFile,"#");                                                          /* # marks the beginning of labels   */

    dlp_strcpy(sPrevLab,(char*)CData_XAddr(idSrc,0,nIcLab));                      /* Remember previous label           */
    for (i=0; i<CData_GetNRecs(idSrc); i++)                                       /* Loop over labels                  */
    {                                                                             /* >>                                */
      dlp_strcpy(sCurrLab,(char*)CData_XAddr(idSrc,i,nIcLab));                    /*   Get current label               */
      if (dlp_strcmp(sCurrLab,sPrevLab)!=0)                                       /*   Current != previous label       */
      {                                                                           /*   >>                              */
        fprintf(lpFile,"\n %G  121 ",(double)(nRinc*i + nRofs));                          /*     Write label time and color    */
        fputs(dlp_strtrimright(sPrevLab),lpFile);                                 /*     Write label name              */
      }                                                                           /*   <<                              */
      if (i==CData_GetNRecs(idSrc)-1)                                             /*   Special for last record         */
      {                                                                           /*   >>                              */
        fprintf(lpFile,"\n %G  121 ",(double)(nRinc*(i+1) + nRofs));                      /*     Write label time and color    */
        fputs(dlp_strtrimright(sCurrLab),lpFile);                               /*     Write label name              */
      }                                                                           /*   <<                              */
      dlp_strcpy(sPrevLab,sCurrLab);                                              /*   Remember previous label         */
    }                                                                             /* <<                                */
    fclose(lpFile);                                                               /* Close file                        */
  }

  return O_K;                                                                   /* All done successfully             */
}

/**
 * Import of period marker files (pm).
 *
 * @param sFilename    Name of file to import.
 * @param iDest        Instance where to save imported data.
 * @param sFiletype    Type of file to import.
 * @return O_K if successful, an error code otherwise
 */
INT16 CGEN_PROTECTED CDlpFile_ImportPmToData
(
  CDlpFile*     _this,
  const char*   sFilename,
  CDlpObject* iDest,
  const char*   sFiletype
)
{
  INT16  nExite      = 0;
  INT32   i           = 0;
  BOOL   bSwap       = FALSE;
  INT16* lpDest      = NULL;
  CData* dDest       = NULL;
  FILE*  lpFile      = NULL;
  PM_HEADER pm_header;
  PMARKEN einzel_PM;

  /* get pointer to derived instance */
  dDest = AS(CData,iDest);
  DLPASSERT(dDest);

  /* prepare destination instance */
  CData_Reset(iDest,TRUE);
  CData_AddComp(dDest,"pm",T_SHORT);
  CData_AddComp(dDest,"v/uv",T_SHORT);
  CData_AllocUninitialized(dDest,1000);
  lpDest = (INT16*)CData_XAddr(dDest,0,0);

  lpFile = fopen(sFilename,"rb");
  if (!lpFile) return IERROR(_this,ERR_FILEOPEN,sFilename,"reading",0);

  /* read the pm header */
  if( fread((void *)&pm_header, sizeof(char), 8, lpFile) != 8)
  {
    fclose(lpFile);
    return
      IERROR(_this,FIL_FORMAT,sFilename,sFiletype,"error while reading header");
  }

  /* TODO: eval complete header information */
  /* set sampling rate */
  ISETFIELD_RVALUE(dDest,"fsr",1000.0/pm_header.atf);

#ifdef sparc
  bSwap = _this->m_bLittle;
#else
  bSwap = _this->m_bBig;
#endif

  /* read data */
  for(i=0;!feof(lpFile);i++)
  {
    if(fread(&einzel_PM, sizeof(PMARKEN), 1, lpFile))
    {
      if(bSwap)
        einzel_PM.laenge = ( (einzel_PM.laenge << 8) & 0xff00 ) | ( (einzel_PM.laenge >> 8) & 0x00ff );
      if(CData_GetMaxRecs(dDest)<=i+1)
      {
        CData_Realloc(dDest,CData_GetMaxRecs(dDest)+1000);
        lpDest = (INT16*)CData_XAddr(dDest,i,0);
      }
      nExite = (INT16)einzel_PM.anregung;
      dlp_memmove(lpDest++,&einzel_PM.laenge,sizeof(INT16));
      dlp_memmove(lpDest++,&nExite,sizeof(INT16));
    }
  }

  CData_Realloc(dDest,i-1);
  CData_SetNRecs(dDest,i-1);

  fclose(lpFile);
  return(O_K);
}

/**
 * Import of period marker files for wavesurfer (pm.txt).
 *
 * ATTENTION: the pitch markers are located at the end of the period!!!
 *
 * @param sFilename    Name of file to import.
 * @param iDest        Instance where to save imported data.
 * @param sFiletype    Type of file to import.
 * @return O_K if successful, an error code otherwise
 */
INT16 CGEN_PROTECTED CDlpFile_ImportPmTxtToData
(
  CDlpFile*     _this,
  const char*   sFilename,
  CDlpObject* iDest,
  const char*   sFiletype
)
{
  INT16   nExite      = 0;
  INT16   laenge      = 0;
  INT32   i           = 0;
  INT32   sum         = 0;
  float  value       = 0.0;
  FLOAT64 fsr         = 0.0;
  BOOL    bSwap       = FALSE;
  INT16*  lpDest      = NULL;
  CData*  dDest       = NULL;
  FILE*   lpFile      = NULL;

  /* get pointer to derived instance */
  dDest = AS(CData,iDest);
  DLPASSERT(dDest);

  if((fsr = dDest->m_lpTable->m_fsr) <= 0.0) {
    return IERROR(_this, ERR_ILLEGALMEMBERVAL, "%s.rinc=%f", BASEINST(dDest)->m_lpInstanceName, dDest->m_lpTable->m_fsr);
  }
  /* prepare destination instance */
  CData_Reset(iDest,TRUE);
  dDest->m_lpTable->m_fsr = fsr;
  CData_AddComp(dDest,"pm",T_SHORT);
  CData_AddComp(dDest,"v/uv",T_SHORT);
  CData_AllocUninitialized(dDest,1000);
  lpDest = (INT16*)CData_XAddr(dDest,0,0);

  lpFile = fopen(sFilename,"rt");
  if (!lpFile) return IERROR(_this,ERR_FILEOPEN,sFilename,"reading",0);

#ifdef sparc
  bSwap = _this->m_bLittle;
#else
  bSwap = _this->m_bBig;
#endif

  /* read data */
  for(i=0;!feof(lpFile);i++)
  {
    if(fscanf(lpFile, "%f", &value) == 1)
    {
      if(value < 0.0) {
        nExite = 0;
        value = -value;
      } else if(value > 0.0) {
        nExite = 1;
      } else {
        continue;
      }

      value = value/(float)dDest->m_lpTable->m_fsr*1000.0;
      laenge = (INT16)(value+0.5)-sum;
      sum += laenge;

      if(bSwap)
        laenge = ( (laenge << 8) & 0xff00 ) | ( (laenge >> 8) & 0x00ff );
      if(CData_GetMaxRecs(dDest)<=i+1)
      {
        CData_Realloc(dDest,CData_GetMaxRecs(dDest)+1000);
        lpDest = (INT16*)CData_XAddr(dDest,i,0);
      }
      *(lpDest++) = (INT16)laenge;
      *(lpDest++) = (INT16)nExite;
    }
  }

  CData_Realloc(dDest,i-1);
  CData_SetNRecs(dDest,i-1);

  fclose(lpFile);
  return(O_K);
}

/**
 * Export of data instances to period marker files (pm).
 *
 * @param sFilename   Name of file to export.
 * @param iSrc        Instance containing data to export.
 * @param sFiletype   Type of file to export.
 * @return O_K if successful, an error code otherwise
 */
INT16 CGEN_PROTECTED CDlpFile_ExportPmFromData
(
  CDlpFile*     _this,
  const char*   sFilename,
  CDlpObject* iSrc,
  const char*   sFiletype
)
{
  INT32   i        = 0;
  BOOL   bSwap       = FALSE;
  BOOL   bRet        = O_K;
  INT16* lpSrc     = NULL;
  CData* dSrc        = NULL;
  FILE*  lpFile      = NULL;
  PM_HEADER pm_header;
  PMARKEN einzel_PM;

  /* get pointer to derived instance */
  dSrc = AS(CData,iSrc);
  DLPASSERT(dSrc);

  /* Check source instance */
  if(CData_GetNComps(dSrc)!=2)
    return IERROR(_this,FIL_BADCOMPS,iSrc->m_lpInstanceName,2,0);
  if(T_SHORT != CData_GetCompType(dSrc,0))
    return IERROR(_this,FIL_BADTYPE,0,iSrc->m_lpInstanceName,"short");
  if(T_SHORT != CData_GetCompType(dSrc,1))
    return IERROR(_this,FIL_BADTYPE,1,iSrc->m_lpInstanceName,"short");
  lpSrc = (INT16*)CData_XAddr(dSrc,0,0);
  if(lpSrc == NULL) return IERROR(_this,FIL_PROCESS,"writing",sFilename,0);

  /* Open file */
  lpFile = fopen(sFilename,"wb");
  if (!lpFile) return IERROR(_this,ERR_FILEOPEN,sFilename,"writing",0);

  /* TODO: Write complete header information */
  pm_header.atf      = (INT16)(1000.0/dSrc->m_lpTable->m_fsr);
  pm_header.dummy    = 0;
  pm_header.format   = 0;
  pm_header.kennzahl = 0;
  pm_header.laenge   = 0;
  pm_header.methode  = 0;

  /* write the pm header */
  if( fwrite((void *)&pm_header, sizeof(char), 8, lpFile) != 8)
  {
    fclose(lpFile);
    return
      IERROR(_this,FIL_FORMAT,sFilename,sFiletype,"error while writing header");
  }

#ifdef sparc
  bSwap = _this->m_bLittle;
#else
  bSwap = _this->m_bBig;
#endif


  /* write data */
  for(i=0;i<CData_GetNRecs(dSrc);i++)
  {
    dlp_memmove(&einzel_PM.laenge,lpSrc++,sizeof(INT16));
    dlp_memmove(&einzel_PM.anregung,lpSrc++,sizeof(INT16));
    einzel_PM.dummy = 0;

    if(bSwap)
      einzel_PM.laenge = ( (einzel_PM.laenge << 8) & 0xff00 ) | ( (einzel_PM.laenge >> 8) & 0x00ff );

    if(1 != (fwrite(&einzel_PM, sizeof(PMARKEN), 1, lpFile)))
    {
      bRet = FALSE;
      IERROR(_this,FIL_PROCESS,"writing",sFilename,0);
      break;
    }
  }

  fclose(lpFile);
  return(bRet);
}

/**
 * Export of data instances to period marker file for wavesurfer (pm.txt).
 *
 * ATTENTION: the pitch markers are located at the end of the period!!!
 *
 * @param sFilename   Name of file to export.
 * @param iSrc        Instance containing data to export.
 * @param sFiletype   Type of file to export.
 * @return O_K if successful, an error code otherwise
 */
INT16 CGEN_PROTECTED CDlpFile_ExportPmTxtFromData
(
  CDlpFile*     _this,
  const char*   sFilename,
  CDlpObject*   iSrc,
  const char*   sFiletype
)
{
  INT32   i           = 0;
  INT32   sum         = 0;
  FLOAT32  value       = 0.0;
  BOOL   bSwap       = FALSE;
  BOOL   bRet        = O_K;
  INT16* lpSrc       = NULL;
  CData* dSrc        = NULL;
  FILE*  lpFile      = NULL;
  PMARKEN einzel_PM;

  /* get pointer to derived instance */
  dSrc = AS(CData,iSrc);
  DLPASSERT(dSrc);

  /* Check source instance */
  if(CData_GetNComps(dSrc)!=2)
    return IERROR(_this,FIL_BADCOMPS,iSrc->m_lpInstanceName,2,0);
  if(T_SHORT != CData_GetCompType(dSrc,0))
    return IERROR(_this,FIL_BADTYPE,0,iSrc->m_lpInstanceName,"short");
  if(T_SHORT != CData_GetCompType(dSrc,1))
    return IERROR(_this,FIL_BADTYPE,1,iSrc->m_lpInstanceName,"short");
  lpSrc = (INT16*)CData_XAddr(dSrc,0,0);
  if(lpSrc == NULL) return IERROR(_this,FIL_PROCESS,"writing",sFilename,0);

  /* Open file */
  lpFile = fopen(sFilename,"wt");
  if (!lpFile) return IERROR(_this,ERR_FILEOPEN,sFilename,"writing",0);

#ifdef sparc
  bSwap = _this->m_bLittle;
#else
  bSwap = _this->m_bBig;
#endif


  /* write data */
  for(i=0;i<CData_GetNRecs(dSrc);i++)
  {
    dlp_memmove(&einzel_PM.laenge,lpSrc++,sizeof(INT16));
    dlp_memmove(&einzel_PM.anregung,lpSrc++,sizeof(INT16));
    einzel_PM.dummy = 0;

    if(bSwap)
      einzel_PM.laenge = ( (einzel_PM.laenge << 8) & 0xff00 ) | ( (einzel_PM.laenge >> 8) & 0x00ff );

    sum +=  einzel_PM.laenge;
    value = (FLOAT32)sum*dSrc->m_lpTable->m_fsr/1000.0;
    value = ((einzel_PM.anregung == 0) ? -value : value);
    if((fprintf(lpFile,"%f\n", (double)value)) <= 0)
    {
      bRet = FALSE;
      IERROR(_this,FIL_PROCESS,"writing",sFilename,0);
      break;
    }
  }

  fclose(lpFile);
  return(bRet);
}

#define MAX_UNIT_CHARS        64
#define MAX_PHON_CHARS         4
#define MAX_INTOLINE_CHARS 32768
/**
 * Import of control files for speech synthesis (into).
 *
 * @param sFilename    Name of file to import.
 * @param iDest        Instance where to save imported data.
 * @param sFiletype    Type of file to import.
 * @return O_K if successful, an error code otherwise
 */
INT16 CGEN_PROTECTED CDlpFile_ImportIntoToData (
  CDlpFile*     _this,
  const char*   sFilename,
  CDlpObject* iDest,
  const char*   sFiletype
) {
  CData* dDest                        = NULL;
  FILE*  lpFile                       = NULL;
  char*  token                        = NULL;
  char   line[MAX_INTOLINE_CHARS]     = "";
  char   lineF[MAX_INTOLINE_CHARS]    = "";
  char   lineI[MAX_INTOLINE_CHARS]    = "";
  char   pho[MAX_PHON_CHARS]          = "";
  char   msg[32]                      = "";
  FLOAT64 pos                          = 0.0;
  FLOAT64 val                          = 0.0;
  INT32   i                            = 0;
  INT32   j                            = 0;
  INT32   n                            = 0;
  INT32   nF                           = 0;
  INT32   nI                           = 0;
  INT32   nL                           = 0;
  /* get pointer to derived instance */
  dDest = AS(CData,iDest);
  DLPASSERT(dDest);

  /* prepare destination instance */
  CData_Reset(iDest,TRUE);
  CData_AddComp(dDest,"unit",MAX_UNIT_CHARS);
  CData_AddComp(dDest,"dura",T_SHORT);
  CData_AddComp(dDest,"fpho",MAX_PHON_CHARS);
  CData_AddComp(dDest,"fval",T_DOUBLE);
  CData_AddComp(dDest,"fpos",T_DOUBLE);
  CData_AddComp(dDest,"ipho",MAX_PHON_CHARS);
  CData_AddComp(dDest,"ival",T_DOUBLE);
  CData_AddComp(dDest,"ipos",T_DOUBLE);

  lpFile = fopen(sFilename,"rt");
  if (!lpFile) return IERROR(_this,ERR_FILEOPEN,sFilename,"reading",0);


  /* read data */
  i = 0;
  nL = 0;
  while(!feof(lpFile)) {
    if(fgets(line, MAX_INTOLINE_CHARS, lpFile)) {
      nL++;
      if((line[0] == '{') && (strlen(line) > 2)) continue;
      if((i+1) > CData_GetNRecs(dDest)) {
        CData_AddRecs(dDest, 1, 1000);
      }
      CData_Sstore(dDest, dlp_strunquotate(line,'\n','\n'), i, 0);
      if(fgets(line, MAX_INTOLINE_CHARS, lpFile)) {
        nL++;
        if(strlen(dlp_strunquotate(line, '\n', '\n')) > 0) {
          token = strtok(line,",");
          token = strtok(NULL,",");
          if(token == NULL) {
            sprintf(msg, "line: %ld", (long)nL);
            IERROR(_this, FIL_FORMAT, sFilename, sFiletype, msg);
          } else {
            if(token[dlp_strlen(token)-1] == '%') {
              if((dlp_strlen(dDest->m_lpCunit) > 0) && dlp_strcmp(dDest->m_lpCunit, "%")) {
                sprintf(msg, "line: %ld", (long)nL);
                IERROR(_this, FIL_FORMAT, sFilename, sFiletype, msg);
              }
              dlp_strcpy(dDest->m_lpCunit, "%");
            } else {
              dlp_strcpy(dDest->m_lpCunit, "ms");
            }
            CData_Dstore(dDest, (FLOAT64)atoi(token), i, 1);
          }
        } else {
          CData_Dstore(dDest, -1, i, 1);
        }
        if(fgets(line, MAX_INTOLINE_CHARS, lpFile)) {
          nL++;
          nF = 0;
          if(strlen(dlp_strunquotate(line, '\n', '\n')) > 0) {
            dlp_strcpy(lineF, line);
            token = strtok(lineF," ");
            while(token != NULL) {
              nF++;
              token = strtok(NULL, " ");
            }
            dlp_strcpy(lineF, line);
          }
        }
        if(fgets(line, MAX_INTOLINE_CHARS, lpFile)) {
          nL++;
          nI = 0;
          if(strlen(dlp_strunquotate(line, '\n', '\n')) > 0) {
            dlp_strcpy(lineI, line);
            token = strtok(lineI," ");
            while(token != NULL) {
              nI++;
              token = strtok(NULL, " ");
            }
            dlp_strcpy(lineI, line);
          }
        }
        n = MAX(nF, nI);
        if((i+n) > CData_GetNRecs(dDest)) {
          CData_AddRecs(dDest, n-1, 1000);
        }
        j = 0;
        token = strtok(lineF, ",");
        while(j < n) {
          if(token != NULL) {
            dlp_strcpy(pho, token);
            token = strtok(NULL, ",");
            if(token != NULL) {
              val = atof(token);
              token = strtok(NULL, " ");
              if(token != NULL) {
                pos = atof(token);
                if(j) {
                  CData_Sstore(dDest, CData_Sfetch(dDest, i+j-1, 0), i+j, 0);
                  CData_Dstore(dDest, CData_Dfetch(dDest, i+j-1, 1), i+j, 1);
                }
                CData_Sstore(dDest, pho, i+j, 2);
                CData_Dstore(dDest, val, i+j, 3);
                CData_Dstore(dDest, pos, i+j, 4);
                j++;
              } else {
                sprintf(msg, "line: %ld", (long)nL);
                IERROR(_this, FIL_FORMAT, sFilename, sFiletype, msg);
              }
            } else {
              sprintf(msg, "line: %ld", (long)nL);
              IERROR(_this, FIL_FORMAT, sFilename, sFiletype, msg);
            }
          } else {
            while(j < n) {
              CData_Dstore(dDest, -1.0, i+j, 3);
              CData_Dstore(dDest, -1.0, i+j, 4);
              j++;
            }
            break;
          }
          token = strtok(NULL, ",");
        }
        j = 0;
        token = strtok(lineI, ",");
        while(j < n) {
          if(token != NULL) {
            dlp_strcpy(pho, token);
            token = strtok(NULL, ",");
            if(token != NULL) {
              val = atof(token);
              token = strtok(NULL, ", ");
              if(token != NULL) {
                pos = atof(token);
                if(j) {
                  CData_Sstore(dDest, CData_Sfetch(dDest, i+j-1, 0), i+j, 0);
                  CData_Dstore(dDest, CData_Dfetch(dDest, i+j-1, 1), i+j, 1);
                }
                CData_Sstore(dDest, pho, i+j, 5);
                CData_Dstore(dDest, val, i+j, 6);
                CData_Dstore(dDest, pos, i+j, 7);
                j++;
              } else {
                sprintf(msg, "line: %ld", (long)nL);
                IERROR(_this, FIL_FORMAT, sFilename, sFiletype, msg);
              }
            } else {
              sprintf(msg, "line: %ld", (long)nL);
              IERROR(_this, FIL_FORMAT, sFilename, sFiletype, msg);
            }
          } else {
            while(j < n) {
              CData_Dstore(dDest, -1.0, i+j, 6);
              CData_Dstore(dDest, -1.0, i+j, 7);
              j++;
            }
            break;
          }
          token = strtok(NULL, ",");
        }
        if(n == 0) {
          CData_Dstore(dDest, -1.0, i, 3);
          CData_Dstore(dDest, -1.0, i, 4);
          CData_Dstore(dDest, -1.0, i, 6);
          CData_Dstore(dDest, -1.0, i, 7);
        } else {
          i += n-1;
        }
      } else {
        sprintf(msg, "line: %ld", (long)nL);
        IERROR(_this, FIL_FORMAT, sFilename, sFiletype, msg);
      }
      i++;
    }
  }

  fclose(lpFile);

  return O_K;
}

/**
 * Import of control files for speech synthesis (into).
 *
 * @param sFilename    Name of file to export.
 * @param iSrc         Instance to export.
 * @param sFiletype    Type of file to export.
 * @return O_K if successful, an error code otherwise
 */
INT16 CGEN_PROTECTED CDlpFile_ExportIntoFromData (
  CDlpFile*     _this,
  const char*   sFilename,
  CDlpObject*   iSrc,
  const char*   sFiletype
) {
  CData*       dSrc                         = NULL;
  FILE*        lpFile                       = NULL;
  const char*  pho                          = NULL;
  FLOAT64    pos                          = 0.0;
  FLOAT64    val                          = 0.0;
  INT16      dur                          = 0;
  INT32      nRecs                        = 0;
  INT32      iRecs1                       = 0;
  INT32      iRecs2                       = 0;

  /* get pointer to derived instance */
  dSrc = AS(CData,iSrc);
  DLPASSERT(dSrc);

  lpFile = fopen(sFilename,"wt");
  if (!lpFile) return IERROR(_this,ERR_FILEOPEN,sFilename,"writing",0);

  nRecs = CData_GetNRecs(dSrc);

  iRecs1 = 0;
  while(iRecs1 < nRecs) {

    fprintf(lpFile, "%s\n", CData_Sfetch(dSrc,iRecs1, 0));

    dur = (INT16) CData_Dfetch(dSrc, iRecs1, 1);
    if(dur >= 0) {
      if(dSrc->m_lpCunit[0] == '%') {
        fprintf(lpFile, "%s,%d%%", CData_Sfetch(dSrc,iRecs1, 0), (int)dur);
      } else {
        fprintf(lpFile, "%s,%d", CData_Sfetch(dSrc,iRecs1, 0), (int)dur);
      }
    }
    fprintf(lpFile, "\n");

    iRecs2 = 0;
    do {
      pho = CData_Sfetch(dSrc, iRecs1+iRecs2, 2);
      val = CData_Dfetch(dSrc, iRecs1+iRecs2, 3);
      pos = CData_Dfetch(dSrc, iRecs1+iRecs2, 4);
      if((pho != NULL) && (strlen(pho) > 0) && (val >= 0.0) && (pos >= 0.0) && (pos <= 1.0)) {
        fprintf(lpFile, "%s,%.2f,%.2f ", pho, (double)val, (double)pos);
      }
      iRecs2++;
    } while(!dlp_strcmp(CData_Sfetch(dSrc,iRecs1+iRecs2, 0), CData_Sfetch(dSrc,iRecs1, 0)));
    fprintf(lpFile, "\n");

    iRecs2 = 0;
    do {
      pho = CData_Sfetch(dSrc, iRecs1+iRecs2, 5);
      val = CData_Dfetch(dSrc, iRecs1+iRecs2, 6);
      pos = CData_Dfetch(dSrc, iRecs1+iRecs2, 7);
      if((pho != NULL) && (strlen(pho) > 0) && (val >= 0.0) && (pos >= 0.0) && (pos <= 1.0)) {
        fprintf(lpFile, "%s,%.2f,%.2f ", pho, (double)val, (double)pos);
      }
      iRecs2++;
    } while(!dlp_strcmp(CData_Sfetch(dSrc,iRecs1+iRecs2, 0), CData_Sfetch(dSrc,iRecs1, 0)));
    fprintf(lpFile, "\n");

    iRecs1 += iRecs2;
  }

  fclose(lpFile);

  return O_K;
}

/**
 * Import of binary files (raw).
 *
 * @param sFilename    Name of file to import.
 * @param iDest        Instance where to save imported data.
 * @param sFiletype    Type of file to import.
 * @return O_K if successful, an error code otherwise
 */
INT16 CGEN_PROTECTED CDlpFile_ImportRawToData
(
  CDlpFile*     _this,
  const char*   sFilename,
  CDlpObject* iDest,
  const char*   sFiletype
)
{
  INT64  i      = 0;
  INT64  n      = 0;
  FILE* lpFile = NULL;
  BYTE* lpDest = NULL;
  CData* dDest = NULL;

  /* get pointer to derived instance */
  dDest = AS(CData,iDest);
  DLPASSERT(dDest);
  if(CData_GetRecLen(dDest) <= 0)   return(NOT_EXEC);

  lpFile = fopen(sFilename,"rb");
  if (!lpFile) return IERROR(_this,ERR_FILEOPEN,sFilename,"reading",0);

  if (CData_GetMaxRecs(dDest) == 0)
  {
    fseek(lpFile,0,SEEK_END);
    i=ftell(lpFile);
    n=i/CData_GetRecLen(dDest);
    CData_AllocateUninitialized(dDest, n);
  }
  fseek(lpFile,_this->m_nRawHead>0 ? _this->m_nRawHead : 0,SEEK_SET);
  lpDest = CData_XAddr(dDest,0,0);
  if(lpDest == NULL)
  {
    fclose(lpFile);
    return IERROR(_this,FIL_PROCESS,"reading",sFilename,0);
  }

  n=fread(lpDest, CData_GetRecLen(dDest), CData_GetMaxRecs(dDest), lpFile);
  IFCHECK
    printf("\n Imported %ld bytes in %ld records.\n",(long)n,(long)CData_GetMaxRecs(dDest));

  fclose(lpFile);

  if (_this->m_bReverse) CDlpFile_FlipNumericData(dDest);

  return(O_K);
}

/**
 * Import of Fraunhofer (IZFP) files.
 *
 * <p>IMPORTANT: This implementation works for Intel byteorder only!</p>
 *
 * @param sFilename    Name of file to import.
 * @param iDest        Instance where to save imported data.
 * @param sFiletype    Type of file to import.
 * @return O_K if successful, an error code otherwise
 */
INT16 CGEN_PROTECTED CDlpFile_ImportIzfpRsToData
(
  CDlpFile*   _this,
  const char* sFilename,
  CDlpObject* iDest,
  const char* sFiletype
)
{
  INT16   i              = 0;
  INT16   nMode          = 0;
  UINT32  n              = 0;
  UINT32  s              = 0;
  UINT32  nRec           = 0;
  UINT16 nRot           = 0;
  UINT16 nBuf16         = 0;
  UINT16 nAbtastrate    = 0;
  UINT16 nSeq1          = 0;
  UINT16 nSeq2          = 0;
  UINT16 nGain          = 0;
  UINT16 nPreTrig       = 0;
  UINT16 nUmdrehungen   = 0;
  UINT16 nTrigMode      = 0;
  UINT16 nStop          = 0;
  UINT16 nLevel         = 0;
  UINT16 n_PreTrig      = 0;
  UINT16 nfClk          = 0;
  UINT16 nLWLDaten      = 0;
  UINT16 nLWLMode       = 0;
  UINT16 nGainNF        = 0;
  FILE*     lpFile         = NULL;
  UINT16* lpDest         = NULL;
  UINT16* lpRotIdx       = NULL;
  CData*    dDest          = NULL;
  CData*    dRotIdx        = NULL;

  /* Get import mode (IntRas or FIRS04A) from file type */
  if(!dlp_strcmp(sFiletype,"IntRas"))       nMode=0;
  else if(!dlp_strcmp(sFiletype,"FIRS04A")) nMode=1;

  /* Initialize instances */
  dDest = AS(CData,iDest);
  DLPASSERT(dDest);
  CData_Reset(iDest,TRUE);
  CData_AddNcomps(dDest,T_USHORT,8);
  ICREATEEX(CData,dRotIdx,"RotIdx",NULL);
  CData_AddNcomps(dRotIdx,T_USHORT,1);

  lpFile = fopen(sFilename,"rb");
  if (!lpFile) return IERROR(_this,ERR_FILEOPEN,sFilename,"reading",0);

  /* Read header */
  n =fread(&nBuf16,sizeof(UINT16),1,lpFile); nAbtastrate  = BSWAP_16(nBuf16);
  n+=fread(&nBuf16,sizeof(UINT16),1,lpFile); nSeq1        = BSWAP_16(nBuf16);
  n+=fread(&nBuf16,sizeof(UINT16),1,lpFile); nSeq2        = BSWAP_16(nBuf16);
  n+=fread(&nBuf16,sizeof(UINT16),1,lpFile); nGain        = BSWAP_16(nBuf16);
  n+=fread(&nBuf16,sizeof(UINT16),1,lpFile); nPreTrig     = BSWAP_16(nBuf16);
  n+=fread(&nBuf16,sizeof(UINT16),1,lpFile); nUmdrehungen = BSWAP_16(nBuf16);
  n+=fread(&nBuf16,sizeof(UINT16),1,lpFile); nTrigMode    = BSWAP_16(nBuf16);
  n+=fread(&nBuf16,sizeof(UINT16),1,lpFile); nStop        = BSWAP_16(nBuf16);
  n+=fread(&nBuf16,sizeof(UINT16),1,lpFile); nLevel       = BSWAP_16(nBuf16);
  n+=fread(&nBuf16,sizeof(UINT16),1,lpFile); n_PreTrig    = BSWAP_16(nBuf16);
  n+=fread(&nBuf16,sizeof(UINT16),1,lpFile); nfClk        = BSWAP_16(nBuf16);
  n+=fread(&nBuf16,sizeof(UINT16),1,lpFile); nLWLDaten    = BSWAP_16(nBuf16);
  n+=fread(&nBuf16,sizeof(UINT16),1,lpFile); nLWLMode     = BSWAP_16(nBuf16);
  if(nMode==1)
    n+=fread(&nBuf16,sizeof(UINT16),1,lpFile); nGainNF    = BSWAP_16(nBuf16);

  if((nMode==0&&n!=13)||(nMode==1&&n!=14))
  {
    fclose(lpFile);
    return IERROR(_this,FIL_FORMAT,sFilename,sFiletype,"import");
  }

  /* Print header information if check mode is set */
  IFCHECKEX(1) printf("\n\n");
  IFCHECKEX(1) dlp_fprint_x_line(stdout,'-',40);
  if(nMode==1){
    IFCHECKEX(1) printf("\n\tFIRS04A Header\n");
  }else{
    IFCHECKEX(1) printf("\n\tIntRas Header\n");
  }
  IFCHECKEX(1) dlp_fprint_x_line(stdout,'-',40);
  IFCHECKEX(1) printf("\nAbtastrate\t: %u*10 kHz",(unsigned int)nAbtastrate);
  IFCHECKEX(1) printf("\nSEQ1\t\t: %u",(unsigned int)nSeq1);
  IFCHECKEX(1) printf("\nSEQ2\t\t: %u",(unsigned int)nSeq2);
  IFCHECKEX(1) printf("\nGain\t\t: %u",(unsigned int)nGain);
  IFCHECKEX(1) printf("\nPreTrig (Samples): %u",(unsigned int)nPreTrig);
  IFCHECKEX(1) printf("\nUmdrehungen\t: %u",(unsigned int)nUmdrehungen);
  IFCHECKEX(1) printf("\nTrigMode (Start): %u",(unsigned int)nTrigMode);
  IFCHECKEX(1) printf("\nStop\t\t: %u",(unsigned int)nStop);
  IFCHECKEX(1) printf("\nLevel\t\t: %u",(unsigned int)nLevel);
  IFCHECKEX(1) printf("\n\%PreTrig\t\t: %u",(unsigned int)n_PreTrig);
  IFCHECKEX(1) printf("\nf_Clk\t\t: %u kHz",(unsigned int)nfClk);
  IFCHECKEX(1) printf("\n#LWLDaten\t: %u*16",(unsigned int)nLWLDaten);
  IFCHECKEX(1) printf("\nLWLMode\t\t: %u",(unsigned int)nLWLMode);
  if(nMode==1)
    IFCHECKEX(1) printf("\nGainNF\t\t: %u",(unsigned int)nGainNF);
  IFCHECKEX(1) printf("\n");
  IFCHECKEX(1) dlp_fprint_x_line(stdout,'-',40);
  IFCHECKEX(1) printf("\n\n");

  /* Determine number of records (approx) */
  fseek(lpFile,0,SEEK_END);
  n=ftell(lpFile)-128+8*2;     /* Lenght of data block including separations minus header */
  nRec=n/(8*sizeof(UINT16));

  /* Allocate records */
  IFCHECKEX(2) printf("\n\nAllocate %d records",nRec);
  CData_Alloc(dDest, nRec);
  lpDest = (UINT16*)CData_XAddr(dDest,0,0);
  DLPASSERT(lpDest)
  CData_Alloc(dRotIdx, nRec);
  lpRotIdx = (UINT16*)CData_XAddr(dRotIdx,0,0);
  DLPASSERT(lpRotIdx)

  /* Skip 128 byte header (32 bit alignment!) and initialize get-method */
  fseek(lpFile,128,SEEK_SET);
  get_nextUInt16(lpFile,&nBuf16,TRUE,nMode);

  /*
  Dump file for debugging
  dlp_init_printstop();
  for(n=0;!feof(lpFile);)
  {
    n+=get_nextUInt16(lpFile,&nBuf16,FALSE,nMode);
    printf("\n%d value=0x%04X",n-1,nBuf16);
    dlp_if_printstop();
  }
   */

  /* Scan for start sequence and skip it */
  for(s=0;!feof(lpFile);)
  {
    s+=get_nextUInt16(lpFile,&nBuf16,FALSE,nMode);
    if(nBuf16!=0x0&&nBuf16!=0xE&&nBuf16!=0xF&&nBuf16!=0xAAAA) break;
    IFCHECKEX(2) if(s>0) printf("\nSeparation at %d (value=0x%04X)",(int)(s-1),(unsigned int)nBuf16);
  }

  s=1;
  if(nBuf16!=0x1234) s=0;
  else IFCHECKEX(2) if(s>0) printf("\nStart      at %d (value=0x%04X)",(int)(s-1),(unsigned int)nBuf16);
  s+=get_nextUInt16(lpFile,&nBuf16,FALSE,nMode);
  if(nBuf16!=0x9ABC) s=0;
  else IFCHECKEX(2) if(s>0) printf("\nStart      at %d (value=0x%04X)",(int)(s-1),(unsigned int)nBuf16);
  s+=get_nextUInt16(lpFile,&nBuf16,FALSE,nMode);
  if(nBuf16!=0x5678) s=0;
  else IFCHECKEX(2) if(s>0) printf("\nStart      at %d (value=0x%04X)",(int)(s-1),(unsigned int)nBuf16);
  s+=get_nextUInt16(lpFile,&nBuf16,FALSE,nMode);
  if((nMode==0&&nBuf16!=0x5555)||(nMode==1&&nBuf16!=0x5554)) s=0;
  else IFCHECKEX(2) if(s>0) printf("\nStart      at %d (value=0x%04X)",(int)(s-1),(unsigned int)nBuf16);
  if(s!=4)
  {
    fclose(lpFile);
    IDESTROY(dRotIdx);
    CData_Reset(iDest,TRUE);
    return IERROR(_this,FIL_FORMAT,sFilename,sFiletype,"import");
  }

  /* Skip additional information in FIRS04A mode */
  if(nMode==1)
  {
    s+=get_nextUInt16(lpFile,&nBuf16,FALSE,nMode);
    IFCHECKEX(2) printf("\nMessnummer0 at %d (value=0x%04X)",(int)(s-1),(unsigned int)nBuf16);
    s+=get_nextUInt16(lpFile,&nBuf16,FALSE,nMode);
    IFCHECKEX(2) printf("\nMessnummer1 at %d (value=0x%04X)",(int)(s-1),(unsigned int)nBuf16);
    s+=get_nextUInt16(lpFile,&nBuf16,FALSE,nMode);
    IFCHECKEX(2) printf("\nDrehzahl    at %d (value=0x%04X)",(int)(s-1),(unsigned int)nBuf16);
    s+=get_nextUInt16(lpFile,&nBuf16,FALSE,nMode);
    IFCHECKEX(2) printf("\nStatus A    at %d (value=0x%04X)",(int)(s-1),(unsigned int)nBuf16);
  }

  /* Read data */
  for(nRot=0,n=0,s=0,nRec=0;!feof(lpFile);)
  {
    n+=get_nextUInt16(lpFile,&nBuf16,FALSE,nMode);

    if(n==1)
    {
      /*IFCHECKEX(2) printf("\nBegin of rotation %u at sample %d (record %u, value=%u)",nRot,s+n-1,nRec,(unsigned int)nBuf16);*/
      IFCHECKEX(2) printf("\nData       at %d (value=%u)",(int)(s+n-1),(unsigned int)nBuf16);
    }

    /* Separation? */
    if((nBuf16==0x0||nBuf16==0xE||nBuf16==0xAAAA||nBuf16==0xF)&&!feof(lpFile))
    {
      do
      {
        IFCHECKEX(2) printf("\nSeparation at %d (value=0x%04X)",(int)(s+n-1),(unsigned int)nBuf16);
        s+=get_nextUInt16(lpFile,&nBuf16,FALSE,nMode);
      }
      while((nBuf16==0x0||nBuf16==0xE||nBuf16==0xAAAA||nBuf16==0xF)&&!feof(lpFile));
      if(feof(lpFile)) break;

      /* Skip additional information in FIRS04A mode */
      if(nMode==1)   for(i=0;i<4;i++) get_nextUInt16(lpFile,&nBuf16,FALSE,nMode);

      nRot++;
      /*IFCHECKEX(2) printf("\nBegin of rotation %u at sample %d (record %u, value=%u/0x%04X)",(unsigned int)nRot,(int)(s+n-1),(unsigned int)nRec,(unsigned int)nBuf,(unsigned int)nBuf16);*/
      IFCHECKEX(2) printf("\nData       at %d (value=%u)",(int)(s+n-1),(unsigned int)nBuf16);
    }

    /* Save sample */
    *lpDest++=(UINT16)nBuf16;

    /* Save rotation index of current record */
    if(!(n%8))
    {
      *lpRotIdx=(UINT16)nRot;
      lpRotIdx++;
      nRec++;
    }
  }

  fclose(lpFile);

  if (_this->m_bReverse) CDlpFile_FlipNumericData(dDest);

  CData_SetNRecs(dDest,nRec);
  CData_SetNRecs(dRotIdx,nRec);
  CData_Join(dDest,dRotIdx);
  ISETFIELD_RVALUE(dDest,"rinc",1.0/(nAbtastrate*10.0));
  IDESTROY(dRotIdx);
  IFCHECK(printf("\n Imported %ld bytes in %ld records.\n",(long)(n*sizeof(UINT16)),(long)CData_GetNRecs(dDest)));

  return(O_K);
}

/**
 * Export of binary files (raw).
 *
 * @param sFilename    Name of file to export.
 * @param iSrc        Instance of data to export.
 * @param sFiletype    Type of file to export.
 * @return O_K if successful, an error code otherwise
 */
INT16 CGEN_PROTECTED CDlpFile_ExportRawFromData
(
  CDlpFile*   _this,
  const char* sFilename,
  CDlpObject* iSrc,
  const char* sFiletype
)
{
  INT32   n      = 0;
  FILE*  lpFile = NULL;
  BYTE*  lpSrc  = NULL;
  CData* dSrc   = NULL;

  /* get pointer to derived instance */
  dSrc = AS(CData,iSrc);
  DLPASSERT(dSrc);
  if(CData_GetRecLen(dSrc) <= 0)  return(NOT_EXEC);
  if(CData_GetNRecs(dSrc) <= 0)   return(NOT_EXEC);

  lpSrc = CData_XAddr(dSrc,0,0);
  if(lpSrc == NULL) return IERROR(_this,FIL_PROCESS,"writing",sFilename,0);

  lpFile = fopen(sFilename,_this->m_bAppend?"ab":"wb");
  if (!lpFile) return IERROR(_this,ERR_FILEOPEN,sFilename,"writing",0);

  n=fwrite(lpSrc, CData_GetRecLen(dSrc), CData_GetNRecs(dSrc), lpFile);
  IFCHECK(printf("\n Exported %ld bytes in %ld records.\n",(long)n,(long)CData_GetMaxRecs(dSrc)));

  fclose(lpFile);
  return(O_K);
}

/* Static helper functions */

/**
 * Get next 16bit unsigned integers from file that is organized in 32bit motorola
 * format
 *
 * @param lpFile  File pointer
 * @param lpBuf16 Pointer to buffer for return value
 * @param bInit   Init buffers, does not read the first value!
 * @param nMode   Operation mode: 0 - normal, 1 - mask 0'th bit
 */
static INT16 get_nextUInt16(FILE* lpFile,UINT16 *lpBuf16,INT16 bInit,INT16 nMode)
{
  INT16            n = 0;
  static UINT32 nBuf32;
  static INT16     bNewBlock;

  if(bInit)
  {
    bNewBlock = FALSE;
    n=fread(&nBuf32,sizeof(UINT32),1,lpFile);
    nBuf32=BSWAP_32(nBuf32);
    return 0;
  }

  if(bNewBlock)
  {
    *lpBuf16=(UINT16)(nBuf32>>16);
    n=fread(&nBuf32,sizeof(UINT32),1,lpFile);
    nBuf32=BSWAP_32(nBuf32);
    bNewBlock=FALSE;
  }
  else
  {
    n=1;
    *lpBuf16=(UINT16)nBuf32;
    bNewBlock=TRUE;
  }
  *lpBuf16=BSWAP_16(*lpBuf16);
  if(nMode==1) *lpBuf16&=0xFFFE;

  return n;
}

/**
 *
 */
char* file_strtok(char* lpsTok, const char* lpsDlm)
{
  static char* lpsNtk = NULL;
  const char*  lpsCdl = NULL;
  char*        lpsRes = NULL;

  if (lpsTok) lpsNtk = lpsTok;                                                  /* Set new token string              */
  if (!*lpsNtk) return NULL;                                                    /* No (more) tokens -> return NULL   */
  for (lpsRes=lpsNtk; *lpsNtk; lpsNtk++)                                        /* Traverse token str. from curr.pos.*/
    for (lpsCdl=lpsDlm; *lpsCdl; lpsCdl++)                                      /*   Loop over delimiters            */
      if (*lpsNtk==*lpsCdl)                                                     /*     Delimiter in token string     */
      {                                                                       /*     >>                            */
        *lpsNtk='\0';                                                         /*       Terminate token             */
        lpsNtk++;                                                             /*       Cont. later with next char. */
        return lpsRes;                                                        /*       Return pointer to token     */
      }                                                                       /*     <<                            */
  return lpsRes;                                                                /* No (more) delimiters found        */
}

/**
 * Import an audio data file. It reads the chunks of a MS Wave file and gets
 * all needed audio format information (RIFF-Header). Currently only MS Wave-
 * files with 16 bit, 1 channel and PCM are supported.
 *
 * @param sFilename    Name of file to import.
 * @param iDest        Instance where to save imported data.
 * @param sFiletype    Type of file to import.
 *
 * @return processing status; O_K if successful or an error code otherwise
 */
INT16 CGEN_PROTECTED CDlpFile_ImportWaveToData
(
  CDlpFile*    _this,
  const char*  sFilename,
  CDlpObject*  iDest,
  const char*  sFiletype
)
{
  UINT32 nRiffLength    = 0;                                                    /* length of data after RIFF chunk in bytes */
  UINT32 nFormat        = 0;                                                    /* audio format (1 == PCM) */
  UINT32 nChannels      = 0;                                                    /* number of channels */
  UINT32 nSamplesPerSec = 0;                                                    /* samples per second */
  UINT32 nBitsPerSample = 0;                                                    /* bits per sample */
  UINT32 nChunkSize     = 0;                                                    /* size of a chunk */
  UINT32 nNrOfSamples   = 0;                                                    /* number of samples */
  FILE*  lpFile;                                                                /* pointer to audio file */
  char   ch[4];                                                                 /* char array for chunk names */
  CData* dDest          = NULL;
  INT32  nPos           = 0;
  UINT32 chFmt[16];                                                             /* initialise a array for format chunk bytes */
  UINT16 i              = 0;

  lpFile = fopen(sFilename,"rb");                                               /* open audio file for binary reading */
  if(lpFile == NULL)                                                            /* audio file exists? */
    return IERROR(_this,FIL_NOTFOUND,sFilename,"reading",0);

  for(i=0; i<4; i++)                                                            /* loop to initialise the character */
  {                                                                             /* array for chunk names */
    if(!feof(lpFile))
      ch[i] = fgetc(lpFile);                                                    /* read next byte from file */
    else
    {
      fclose(lpFile);                                                           /* close file at EOF */
      return IERROR(_this,FIL_PROCESS,"reading",sFilename,0);                                                            /* exit function       */
    }
  }

  while(!feof(lpFile))                                                          /* read next bytes and look for */
  {                                                                             /* RIFF chunk */
    if(ch[nPos%4]=='R' && ch[(nPos+1)%4]=='I' &&                                /* continue further processing if RIFF */
       ch[(nPos+2)%4]=='F' && ch[(nPos+3)%4]=='F') break;                       /* is found */
    ch[nPos%4] = fgetc(lpFile);                                                 /* read next byte into the circle memory */
    nPos ++;
  }

  if(feof(lpFile))                                                              /* exit function if "RIFF" is not in file or */
  {                                                                             /* there isn't data following */
    fclose(lpFile);
    return IERROR(_this,FIL_WAV_MISS,sFilename,"a RIFF-header",0);
  }

  nRiffLength = nRiffLength | fgetc(lpFile);                                    /* next four bytes holding the size of the  */
  nRiffLength = nRiffLength | (fgetc(lpFile) << 8);                             /* following data in bytes */
  nRiffLength = nRiffLength | (fgetc(lpFile) << 16);                            /* shift the bytes to get the 32Bit integer */
  nRiffLength = nRiffLength | (fgetc(lpFile) << 24);                            /* value */

  while(!feof(lpFile))                                                          /* search for the "WAVE" chunk next */
  {
    if(ch[nPos%4]=='W' && ch[(nPos+1)%4]=='A' &&                                /* continue further processing if WAVE is found */
       ch[(nPos+2)%4]=='V' && ch[(nPos+3)%4]=='E') break;
    ch[nPos%4] = fgetc(lpFile);                                                 /* read next byte into the circle memory */
    nPos ++;
  }

  if(feof(lpFile))                                                              /* exit function if "WAVE" is not in file or */
  {                                                                             /* there isn't data following */
    fclose(lpFile);
    return IERROR(_this,FIL_WAV_MISS,sFilename,"MS Wave identifier",0);
  }

  while(!feof(lpFile))                                                          /* search for wave format (fmt ) subchunk next */
  {
    if(ch[nPos%4]=='f' && ch[(nPos+1)%4]=='m' &&                                /* continue further processing if fmt  is found */
       ch[(nPos+2)%4]=='t' && ch[(nPos+3)%4]==' ') break;
    ch[nPos%4] = fgetc(lpFile);                                                 /* read next byte into the circle memory */
    nPos ++;
  }

  if(feof(lpFile))                                                              /* exit function if "WAVE" is not in file or */
  {                                                                             /* there isn't data following */
    fclose(lpFile);
    return IERROR(_this,FIL_WAV_MISS,sFilename,"a format chunk",0);
  }

  for(i=0; i<4; i++)                                                            /* find out chunk size of format chunk */
    nChunkSize = nChunkSize | (fgetc(lpFile) << (i*8));

  if(nChunkSize>=16)                                                            /* all standard fields available ? */
  {
    UINT32 i;
    for(i=0; i<nChunkSize; i++)
    {
      chFmt[i] = fgetc(lpFile);                                                 /* get format chunk bytes  */
    }
    nFormat = nFormat | chFmt[0];                                               /* find out audio format */
    nFormat = nFormat | (chFmt[1] << 8);                                        /*  | 1==PCM 3==IEEE FLOAT */
    nChannels = nChannels | chFmt[2];                                           /* find out number of channels */
    nChannels = nChannels | (chFmt[3] << 8);
    nSamplesPerSec = nSamplesPerSec | chFmt[4];                                 /* find out samples per second */
    nSamplesPerSec = nSamplesPerSec | (chFmt[5] << 8);
    nSamplesPerSec = nSamplesPerSec | (chFmt[6] << 16);
    nSamplesPerSec = nSamplesPerSec | (chFmt[7] << 24);
    nBitsPerSample = nBitsPerSample | chFmt[14];                                /* find out bits per sample */
    nBitsPerSample = nBitsPerSample | (chFmt[15] << 8);
  }
  else
  {
    return IERROR(_this,FIL_FORMAT,sFilename,sFiletype,"No PCM-data");          /* exit function if there a less then 16 chunk bytes */
  }

  for(i=0; i<4; i++)                                                            /* fill character array with the next four byte after the */
  {                                                                             /* format chunk to find data chunk */
    if(!feof(lpFile))
      ch[i] = fgetc(lpFile);
    else                                                                        /* exit if there is not enough data in file to fill */
    {                                                                           /* array */
      fclose(lpFile);                                                           /* close file at EOF */
      return IERROR(_this,FIL_PROCESS,"reading",sFilename,0);                   /* exit with a failure */
    }
  }

  nPos = 0;                                                                     /* reset position in circle memory */
  while(!feof(lpFile))                                                          /* search for "data" in the remaining file data */
  {
    if(ch[nPos%4]=='d' && ch[(nPos+1)%4]=='a' &&
       ch[(nPos+2)%4]=='t' && ch[(nPos+3)%4]=='a') break;
    ch[nPos%4] = fgetc(lpFile);                                                 /* read next byte into the circle memory */
    nPos ++;                                                                    /* next position in circle memory */
  }

  if(feof(lpFile))                                                              /* exit function if "data" is not in file or */
  {                                                                             /* there isn't data following */
    fclose(lpFile);
    return IERROR(_this,FIL_WAV_MISS,sFilename,"data",0);                                                             /* exit function       */
  }

  nChunkSize = 0;
  for(i=0; i<4; i++)                                                            /* next four bytes contain length (bytes) of data chunk */
    nChunkSize = nChunkSize | (fgetc(lpFile) << (i*8));


  IFCHECKEX(1) printf("\n   - Loaded audio file: %s",sFilename);             /* output file name */
  IFCHECKEX(1) printf("\n   - File format:");                                   /* output some file format information */
  switch(nFormat)
  {
    case 1:  IFCHECKEX(1) printf("\n      - Audio format:            PCM"); break;
    case 3:  IFCHECKEX(1) printf("\n      - Audio format:            IEEE float"); break;
    default: IFCHECKEX(1) printf("\n      - Audio format:            unknown"); break;
  }
  IFCHECKEX(1) printf("\n      - Number of channels:      %lu",(unsigned long)nChannels);
  IFCHECKEX(1) printf("\n      - Samples per second [Hz]: %lu",(unsigned long)nSamplesPerSec);
  IFCHECKEX(1) printf("\n      - Bits per sample:         %lu",(unsigned long)nBitsPerSample);
  IFCHECKEX(1) printf("\n      - Signal length [ms]:      %lu",(unsigned long)((nChunkSize*8*1000)/(nBitsPerSample*nChannels*nSamplesPerSec)));

  if(nBitsPerSample == 16 && nChannels == 1 && nFormat == 1)
  {
    nNrOfSamples = nChunkSize >> 1;                                             /* calc. nr of samples from the size of the data chunk */
    /* get pointer to derived instance */
    dDest = AS(CData,iDest);
    DLPASSERT(dDest);

    /* prepare destination instance */
    CData_Reset(iDest,TRUE);                                                    /* reset instance where to save imported data  */
    CData_AddComp(dDest, "ch0", T_SHORT);                                       /* add a comp. */
    CData_Allocate(dDest,nNrOfSamples);                                         /* allocate memory */
    if(fread(CData_XAddr(dDest, 0, 0), 2, nNrOfSamples, lpFile) != nNrOfSamples)/* fetch data  */
    { fclose(lpFile); return NOT_EXEC; }                                      /* failed read */
    dDest->m_lpTable->m_fsr = 1000. / nSamplesPerSec;                           /* set distance [ms] between samples */
  }
  else if(nBitsPerSample == 32 && nChannels == 1 && nFormat == 3)
  {
    nNrOfSamples = nChunkSize >> 2;                                             /* calc. nr of samples from the size of the data chunk */
    /* get pointer to derived instance */
    dDest = AS(CData,iDest);
    DLPASSERT(dDest);

    /* prepare destination instance */
    CData_Reset(iDest,TRUE);                                                    /* reset instance where to save imported data  */
    CData_AddComp(dDest, "ch0", T_FLOAT);                                       /* add a comp. */
    CData_Allocate(dDest,nNrOfSamples);                                         /* allocate memory */
    if(fread(CData_XAddr(dDest, 0, 0), 4, nNrOfSamples, lpFile) != nNrOfSamples)/* fetch data  */
    { fclose(lpFile); return NOT_EXEC; }                                        /* failed read */
    dDest->m_lpTable->m_fsr = 1000. / nSamplesPerSec;                           /* set distance [ms] between samples */
  }
  else
  {
    fclose(lpFile);                                                                /* close file */
    return IERROR(_this,FIL_FORMAT,sFilename,sFiletype,"Audio format not supported");                                                             /* exit function       */
  }

  fclose(lpFile);                                                                /* close file */
  return O_K;
}

/**
 * <p>Import inventory description file (.inv).<p>
 *
 * <p>This function imports the inventory description from file <code>sFilename</code> serving the .inv
 * format and stores it in the <code>iSrc</code> data instance . This file is mainly used by synthesizers
 * working with packed inventories.<p>
 *
 * @param sFilename    Name of file to export.
 * @param iSrc         Instance of data to export.
 * @param sFiletype    Type of file to export.
 * @return O_K if successful, an error code otherwise
 *
 */
INT16 CGEN_PROTECTED CDlpFile_ImportInvDescrToData
(
    CDlpFile*    _this,
    const char*  sFilename,
    CDlpObject*  iSrc,
    const char*  sFiletype
)
{
  CData* dSrc                            = NULL;
  FILE*  fpFile                          = NULL;
  INT32  nTmp                            = 0;
  INT32  iPho                            = 0;
  INT32  nPho                            = 0;
  INT32  nR                              = 0;
  char   sLine[MAX_INVDESCR_LINE_LENGTH] = "\0";
  char   sCName[COMP_DESCR_LEN]          = "\0";
  char*  sTok                            = NULL;

  dSrc = AS(CData,iSrc);
  DLPASSERT(dSrc);

  fpFile = fopen(sFilename, "rt");
  if (!fpFile)
    return IERROR(_this,ERR_FILEOPEN,sFilename,"reading",0);

  CData_Reset(BASEINST(dSrc), TRUE);
  CData_AddComp(dSrc, "UNIT", 10);
  CData_AddComp(dSrc, "POS_SIG", T_INT);
  CData_AddComp(dSrc, "CNT_PER", T_INT);
  CData_AddComp(dSrc, "POS_PER", T_INT);
  CData_AddComp(dSrc, "CNT_PHO", T_INT);

  while (fgets(sLine, MAX_INVDESCR_LINE_LENGTH, fpFile) != NULL) {
    sTok = sLine;
    sTok = dlp_strtrimleft(sTok);
    sTok = dlp_strsep(&sTok, " ", NULL);
    if (sTok != NULL) {
      CData_AddRecs(dSrc, 1, 1);
      CData_Sstore(dSrc, sTok, nR, 0);
      sTok = dlp_strtrimleft(sTok+strlen(sTok)+1);
      sTok = dlp_strsep(&sTok, " ", NULL);
      if ((sTok != NULL) && dlp_sscanx(sTok, T_INT, &nTmp)) {
        CData_Dstore(dSrc, (FLOAT64) nTmp, nR, 1);
        sTok = dlp_strtrimleft(sTok+strlen(sTok)+1);
        sTok = dlp_strsep(&sTok, " ", NULL);
        if ((sTok != NULL) && dlp_sscanx(sTok, T_INT, &nTmp)) {
          CData_Dstore(dSrc, (FLOAT64) nTmp, nR, 2);
          sTok = dlp_strtrimleft(sTok+strlen(sTok)+1);
          sTok = dlp_strsep(&sTok, " ", NULL);
          if ((sTok != NULL) && dlp_sscanx(sTok, T_INT, &nTmp)) {
            CData_Dstore(dSrc, (FLOAT64) nTmp, nR, 3);
            sTok = dlp_strtrimleft(sTok+strlen(sTok)+1);
            sTok = dlp_strsep(&sTok, " ", NULL);
            if ((sTok != NULL) && dlp_sscanx(sTok, T_INT, &nPho)) {
              CData_Dstore(dSrc, (FLOAT64) nPho, nR, 4);
              for (iPho = 0; iPho < nPho; iPho++) {
                sTok = dlp_strtrimleft(sTok+strlen(sTok)+1);
                sTok = dlp_strsep(&sTok, " ", NULL);
                if (sTok != NULL) {
                  if (CData_GetNComps(dSrc) <= (5 + iPho * 4)) {
                    sprintf(sCName, "PHO_%d", (int) iPho);
                    CData_AddComp(dSrc, sCName, 5);
                  }
                  CData_Sstore(dSrc, sTok, nR, (5 + iPho * 4));
                  sTok = dlp_strtrimleft(sTok+strlen(sTok)+1);
                  sTok = dlp_strsep(&sTok, " ", NULL);
                  if ((sTok != NULL) && dlp_sscanx(sTok, T_INT, &nTmp)) {
                    if (CData_GetNComps(dSrc) <= (6 + iPho * 4)) {
                      sprintf(sCName, "NP_%d", (int) iPho);
                      CData_AddComp(dSrc, sCName, T_INT);
                    }
                    CData_Dstore(dSrc, (FLOAT64) nTmp, nR, (6 + iPho * 4));
                    sTok = dlp_strtrimleft(sTok+strlen(sTok)+1);
                    sTok = dlp_strsep(&sTok, " ", NULL);
                    if ((sTok != NULL) && dlp_sscanx(sTok, T_INT, &nTmp)) {
                      if (CData_GetNComps(dSrc) <= (7 + iPho * 4)) {
                        sprintf(sCName, "IP40_%d", (int) iPho);
                        CData_AddComp(dSrc, sCName, T_INT);
                      }
                      CData_Dstore(dSrc, (FLOAT64) nTmp, nR, (7 + iPho * 4));
                      sTok = dlp_strtrimleft(sTok+strlen(sTok)+1);
                      sTok = dlp_strsep(&sTok, " ", NULL);
                      if ((sTok != NULL) && dlp_sscanx(sTok, T_INT, &nTmp)) {
                        if (CData_GetNComps(dSrc) <= (8 + iPho * 4)) {
                          sprintf(sCName, "IP60_%d", (int) iPho);
                          CData_AddComp(dSrc, sCName, T_INT);
                        }
                        CData_Dstore(dSrc, (FLOAT64) nTmp, nR, (8 + iPho * 4));
                      }
                    }
                  }
                }
              }
            } else {
              break;
            }
          } else {
            break;
          }
        } else {
          break;
        }
      } else {
        break;
      }
      nR++;
    } else {
      break;
    }
  }

  fclose(fpFile);

  return O_K;
}

/**
 * <p>Export inventory description file (.inv).<p>
 *
 * <p>This function exports the inventory description stored in <code>iSrc</code> data instance to a file with the .inv
 * format. This file is mainly used by synthesizers working with packed inventories.<p>
 *
 * @param sFilename    Name of file to export.
 * @param iSrc         Instance of data to export.
 * @param sFiletype    Type of file to export.
 * @return O_K if successful, an error code otherwise
 *
 */
INT16 CGEN_PROTECTED CDlpFile_ExportInvDescrFromData
(
    CDlpFile*    _this,
    const char*  sFilename,
    CDlpObject*  iSrc,
    const char*  sFiletype
)
{
  CData* dSrc                            = NULL;
  FILE*  fpFile                          = NULL;
  INT32  iPho                            = 0;
  INT32  nPho                            = 0;
  INT32  nR                              = 0;
  INT32  iR                              = 0;
  char   sCName[COMP_DESCR_LEN]          = "\0";

  dSrc = AS(CData,iSrc);
  DLPASSERT(dSrc);

  fpFile = fopen(sFilename, "wt");
  if (!fpFile)
    return IERROR(_this,ERR_FILEOPEN,sFilename,"writing",0);

  nR = CData_GetNRecs(dSrc);

  for(iR = 0; iR < nR; iR++) {
    nPho = (int)CData_Dfetch(dSrc,iR,CData_FindComp(dSrc, "CNT_PHO"));
    fprintf(fpFile, "%-10s% 11d% 4d% 7d%2d",
        CData_Sfetch(dSrc,iR,CData_FindComp(dSrc, "UNIT")),
        (int)CData_Dfetch(dSrc,iR,CData_FindComp(dSrc, "POS_SIG")),
        (int)CData_Dfetch(dSrc,iR,CData_FindComp(dSrc, "CNT_PER")),
        (int)CData_Dfetch(dSrc,iR,CData_FindComp(dSrc, "POS_PER")),
        (int)nPho);
    for(iPho = 0; iPho < nPho; iPho++) {
      sprintf(sCName, "PHO_%d", (int) iPho);
      fprintf(fpFile, " %-4s",CData_Sfetch(dSrc,iR,CData_FindComp(dSrc, sCName)));
      sprintf(sCName, "NP_%d", (int) iPho);
      fprintf(fpFile, "% 4d",(int)CData_Dfetch(dSrc,iR,CData_FindComp(dSrc, sCName)));
      sprintf(sCName, "IP40_%d", (int) iPho);
      fprintf(fpFile, "% 4d",(int)CData_Dfetch(dSrc,iR,CData_FindComp(dSrc, sCName)));
      sprintf(sCName, "IP60_%d", (int) iPho);
      fprintf(fpFile, "% 4d",(int)CData_Dfetch(dSrc,iR,CData_FindComp(dSrc, sCName)));
    }
    fprintf(fpFile,"\n");
  }

  fclose(fpFile);

  return O_K;
}

/**
 * <p>Export of PSTricks code for use with LaTeX.<p>
 *
 * <p>This function exports the contents of a data instance to PSTricks code to include into LaTeX file.
 * For each block a graph is produced. All records of each block will be plotted to one graph. The components
 * belongs to the abscissa.<p> *
 *
 * @param sFilename    Name of file to export.
 * @param iSrc        Instance of data to export.
 * @param sFiletype    Type of file to export.
 * @return O_K if successful, an error code otherwise
 *
 * @see /pst_comma
 * @see /pst_contour
 * @see /pst_legend
 * @see /pst_halfspectrum
 * @see /pst_triglabels
 */
INT16 CGEN_PROTECTED CDlpFile_ExportPstricksFromData
(
  CDlpFile*    _this,
  const char*  sFilename,
  CDlpObject*  iSrc,
  const char*  sFiletype
)
{
  INT32   nR       = 0;
  INT32   iR       = 0;
  INT32   nC       = 0;
  INT32   iC       = 0;
  INT32   nB       = 0;
  INT32   iB       = 0;
  INT32   nDxL     = 0;
  INT32   nDyL     = 0;
  INT32   nChars   = 0;
  FLOAT64 nDx      = 0;
  FLOAT64 nDy      = 0;
  FLOAT64 nMinValX   = T_DOUBLE_MAX;
  FLOAT64 nMaxValX   = T_DOUBLE_MIN;
  FLOAT64 nMinValY   = T_DOUBLE_MAX;
  FLOAT64 nMaxValY   = T_DOUBLE_MIN;
  FLOAT64 nMinValXr  = 0;
  FLOAT64 nMinValYr  = 0;
  FLOAT64 nCinc    = 1.0;
  FLOAT64 nCofs    = 0.0;
  FLOAT64 nRinc    = 1.0;
  FILE*   lpFile   = NULL;
  BYTE*   lpSrc    = NULL;
  char    lpsInstanceName[255];
  char    lpsLab[255];
  CData*  dSrc     = NULL;
  CData*  dLab     = NULL;
  CData*  dDat     = NULL;
  CData*  dDatBlk  = NULL;

  /* get pointer to derived instance */
  dSrc = AS(CData,iSrc);
  DLPASSERT(dSrc);
  if(CData_GetRecLen(dSrc) <= 0)  return(NOT_EXEC);
  if(CData_GetNRecs(dSrc) <= 0)   return(NOT_EXEC);

  lpSrc = CData_XAddr(dSrc,0,0);
  if(lpSrc == NULL) return IERROR(_this,FIL_PROCESS,"writing",sFilename,0);

  lpFile = fopen(sFilename,"wt");
  if (!lpFile) return IERROR(_this,ERR_FILEOPEN,sFilename,"writing",0);

  dlp_strcpy(lpsInstanceName,((CDlpObject*)dSrc)->m_lpInstanceName);
  dlp_strreplace(lpsInstanceName,"#","\\#");
  dlp_strreplace(lpsInstanceName,"_","\\_");

  ICREATE(CData,dLab,FALSE);
  ICREATE(CData,dDat,FALSE);
  ICREATE(CData,dDatBlk,FALSE);

  CMatrix_CopyLabels(dLab,dSrc);
  CData_Select(dDat,dSrc,0,CData_GetNNumericComps(dSrc));

  if(!CData_IsHomogen(dDat)) return IERROR(_this,FIL_BADCOMPS,lpsInstanceName,"homogen",0);
  if(dlp_is_complex_type_code(CData_GetCompType(dDat,0))) return IERROR(_this,FIL_BADCOMPS,lpsInstanceName,"non-complex",0);

  nB = CData_GetNBlocks(dDat);
  nC = CData_GetNNumericComps(dDat);
  nR = CData_GetNRecsPerBlock(dDat);
  nRinc = (dDat->m_lpTable->m_fsr == 0.0) ? nRinc : dDat->m_lpTable->m_fsr;
  nCinc = (dDat->m_nCinc == 0.0) ? nCinc : dDat->m_nCinc;
  nCofs = dDat->m_nCofs;

  if(!dlp_strendswith(dSrc->m_lpCunit,"Hz")) {
    if(_this->m_bPstTriglabels) {
      IERROR(_this,FIL_OPTION,"/pst_triglabels","components don't have proper unit",0);
    }
  } else {
    if(_this->m_bPstTriglabels) {
      nCinc = 10.0/(double)nC;
    }
    if(_this->m_bPstHalfspectrum) nC /= 2;
  }

  for(iB = 0; iB < nB; iB++) {
    CData_SelectBlocks(dDatBlk,dDat,iB,1);
    nMaxValY = T_DOUBLE_MIN;
    nMinValY = T_DOUBLE_MAX;
    if(_this->m_bPstXYPlot) {
      nMinValX = T_DOUBLE_MAX;
      nMaxValX = T_DOUBLE_MIN;
      for(iR = 0; iR < nR; iR++) {
        for(iC = 0; iC < nC; iC+=2) {
          nMinValX = CData_Dfetch(dDatBlk,iR,iC) < nMinValX ? CData_Dfetch(dDatBlk,iR,iC) : nMinValX;
          nMaxValX = CData_Dfetch(dDatBlk,iR,iC) > nMaxValX ? CData_Dfetch(dDatBlk,iR,iC) : nMaxValX;
          nMinValY = CData_Dfetch(dDatBlk,iR,iC+1) < nMinValY ? CData_Dfetch(dDatBlk,iR,iC+1) : nMinValY;
          nMaxValY = CData_Dfetch(dDatBlk,iR,iC+1) > nMaxValY ? CData_Dfetch(dDatBlk,iR,iC+1) : nMaxValY;
        }
      }
    } else {
      for(iR = 0; iR < nR; iR++) {
        for(iC = 0; iC < nC; iC++) {
          nMinValY = CData_Dfetch(dDatBlk,iR,iC) < nMinValY ? CData_Dfetch(dDatBlk,iR,iC) : nMinValY;
          nMaxValY = CData_Dfetch(dDatBlk,iR,iC) > nMaxValY ? CData_Dfetch(dDatBlk,iR,iC) : nMaxValY;
        }
      }
    }
    if(_this->m_bPstContour) {
      fprintf(lpFile,"\\documentclass{scrreprt}\n\n\\usepackage{pgfplots}\n\n\\begin{document}\n\n\\begin{tikzpicture}\n  \\pgfplotsset{width=\\textwidth,height=\\textheight}\n  \\begin{axis}[grid=major,axis lines=center,view={0}{90},point meta min=%f,point meta max=%f]\n  \\addplot3[surf,shader=flat] table {",nMinValY, nMaxValY);
      for(iR = 0; iR < nR; iR++) {
        for(iC = 0; iC < nC; iC++) {
          nChars += fprintf(lpFile,"\n    %+.6le %+.6le %+.6le", (double)(iR*nRinc+dDat->m_lpTable->m_ofs),(double)(iC*nCinc+nCofs),(double)CData_Dfetch(dDatBlk,iR,iC));
        }
        fprintf(lpFile,"\n");
      }
      fprintf(lpFile,"  };\n  \\end{axis}\n\\end{tikzpicture}\n\n\\end{document}\n");
    } else {
      nMinValY = floor(nMinValY+dDat->m_lpTable->m_ofs);
      nMaxValY += dDat->m_lpTable->m_ofs;
      nDyL = ((INT32)(ceil(log10(ABS(nMaxValY-nMinValY))))-1)/3*3;
      nDy = dlm_pow10(nDyL);
      nMinValYr = floor(nMinValY/nDy)*nDy;
      fprintf(lpFile,"\\newlength{\\figureSizeX}\\newlength{\\figureSizeY}\n");
      fprintf(lpFile,"\\setlength{\\figureSizeX}{\\textwidth}\n\\setlength{\\figureSizeY}{\\textheight}\n");
      if(_this->m_bPstXYPlot) {
        nMinValX = floor(nMinValX);
        nDxL = ((INT32)(ceil(log10(ABS(nMaxValX-nMinValX))))-1)/3*3;
        nDx = dlm_pow10(nDxL);
        nMinValXr = floor(nMinValX/nDx)*nDx;
        fprintf(lpFile,"\\psset{xunit=%f\\figureSizeX,yunit=%f\\figureSizeY,llx=-3.5em,lly=-5.5ex,xAxisLabel=%s,yAxisLabel=%s,yAxisLabelPos={-3em,c},xAxisLabelPos={c,-5.5ex}}\n", 1.0/(nMaxValX-nMinValX), 1.0/(nMaxValY-nMinValY),dDatBlk->m_lpCunit,lpsInstanceName);
        fprintf(lpFile,"\\pstScalePoints(%f,%f){%f sub}{%f sub}\n", 1.0/nDx, 1.0/nDy, nMinValX, nMinValY);
        fprintf(lpFile,"\\begin{psgraph}[tickwidth=0.25pt,comma=%s,Ox=%d,Oy=%d,Dx=%1.1f,Dy=%1.1f,dx=%1.1f,dy=%1.1f",_this->m_bPstComma ? "true" : "false",(int)(nMinValXr/nDx),(int)(nMinValYr/nDy),ceil(10.0*(nMaxValX-nMinValX)/nDx)/40.0,ceil(10.0*(nMaxValY-nMinValY)/nDy)/40.0,ceil(10.0*(nMaxValX-nMinValX)/nDx)/40.0,ceil(10*(nMaxValY-nMinValY)/nDy)/40.0);
        if(nDxL!=0) fprintf(lpFile,",xlabelFactor=\\cdot 10^{%d}",nDxL);
        if(nDyL!=0) fprintf(lpFile,",ylabelFactor=\\cdot 10^{%d}",nDyL);
        if(_this->m_bPstTriglabels) fprintf(lpFile,",trigLabels");
        fprintf(lpFile,"]{->}(0,0)(0,0)(%1.1f,%1.1f){\\figureSizeX}{\\figureSizeY}\n",1.1*(nMaxValX-nMinValX)/nDx,1.1*(nMaxValY-nMinValY)/nDy);
        fprintf(lpFile,"\\psgrid[gridlabels=0,subgriddiv=1,griddots=10,gridwidth=0.25pt,subgridwidth=0.2pt,gridcolor=black,subgridcolor=black](0,0)(%d,%d)\n",(int)ceil((nMaxValX-nMinValX)/nDx),(int)ceil((nMaxValY-nMinValY)/nDy));
        for(iC = 0; iC < nC; iC+=2) {
          nChars = fprintf(lpFile,"  \\savedata{\\data}[{");
          for(iR = 0; iR < nR; iR++) {
            if(nR < 2) {
              nChars += fprintf(lpFile,"{%+.6le,%+.6le}", CData_Dfetch(dDatBlk,iR,iC), CData_Dfetch(dDatBlk,iR,iC+1));
              nChars += fprintf(lpFile,"{%+.6le,%+.6le}", CData_Dfetch(dDatBlk,iR,iC), CData_Dfetch(dDatBlk,iR,iC+1));
            } else {
              nChars += fprintf(lpFile,"{%+.6le,%+.6le}", CData_Dfetch(dDatBlk,iR,iC), CData_Dfetch(dDatBlk,iR,iC+1));
            }
            if((nChars>120)&&(iC<(nC-1))) nChars = fprintf(lpFile,"\n                    ");
          }
          fprintf(lpFile,"}]\n");
          fprintf(lpFile,"  \\listplot[showpoints=true,linecolor=black!%d]{\\data}\n",100-iC*100/nC);
        }
      } else {
        nDxL = ((INT32)(ceil(log10(ABS(nC*nCinc))))-1)/3*3;
        nDx = dlm_pow10(nDxL);
        nMinValXr = floor(nCofs/nDx)*nDx;
        fprintf(lpFile,"\\psset{xunit=%f\\figureSizeX,yunit=%f\\figureSizeY,llx=-3.5em,lly=-5.5ex,xAxisLabel=%s,yAxisLabel=%s,yAxisLabelPos={-3em,c},xAxisLabelPos={c,-5.5ex}}\n", 1.0/(nC*nCinc), 1.0/(nMaxValY-nMinValY),dDatBlk->m_lpCunit,lpsInstanceName);
        fprintf(lpFile,"\\pstScalePoints(%f,%f){}{%f sub}\n", 1.0/nDx, 1.0/nDy, nMinValY);
        fprintf(lpFile,"\\begin{psgraph}[tickwidth=0.25pt,comma=%s,Ox=%d,Oy=%d,Dx=%1.1f,Dy=%1.1f,dx=%1.1f,dy=%1.1f",_this->m_bPstComma ? "true" : "false",(int)(nMinValXr/nDx),(int)(nMinValYr/nDy),ceil(10.0*(nC*nCinc)/nDx)/40.0,ceil(10.0*(nMaxValY-nMinValY)/nDy)/40.0,ceil(10.0*(nC*nCinc)/nDx)/40.0,ceil(10*(nMaxValY-nMinValY)/nDy)/40.0);
        if(nDxL!=0) fprintf(lpFile,",xlabelFactor=\\cdot 10^{%d}",nDxL);
        if(nDyL!=0) fprintf(lpFile,",ylabelFactor=\\cdot 10^{%d}",nDyL);
        if(_this->m_bPstTriglabels) fprintf(lpFile,",trigLabels");
        fprintf(lpFile,"]{->}(0,0)(0,0)(%1.1f,%1.1f){\\figureSizeX}{\\figureSizeY}\n",1.1*ceil((nC*nCinc)/nDx),1.1*ceil((nMaxValY-nMinValY)/nDy));
        fprintf(lpFile,"\\psgrid[gridlabels=0,subgriddiv=1,griddots=10,gridwidth=0.25pt,subgridwidth=0.2pt,gridcolor=black,subgridcolor=black](0,0)(%d,%d)\n",(int)ceil((nC*nCinc)/nDx),(int)ceil((nMaxValY-nMinValY)/nDy));
        for(iR = 0; iR < nR; iR++) {
          nChars = fprintf(lpFile,"  \\savedata{\\data}[{");
          for(iC = 0; iC < nC; iC++) {
            nChars += fprintf(lpFile,"{%+.6le,%+.6le}", (iC*nCinc),CData_Dfetch(dDatBlk,iR,iC));
            if((nChars>120)&&(iC<(nC-1))) nChars = fprintf(lpFile,"\n                    ");
          }
          fprintf(lpFile,"}]\n");
          fprintf(lpFile,"  \\listplot[showpoints=false,linecolor=black!%d]{\\data}\n",100-iR*100/nR);
        }
      }

      if(_this->m_bPstLegend) {
        fprintf(lpFile,"  \\rput[rt](%d,%d){\\psframebox[fillstyle=solid,fillcolor=white,linewidth=0.5pt]{\\footnotesize\\setlength{\\tabcolsep}{2pt}\n",(int)ceil((nC*nCinc)/nDx),(int)ceil((nMaxValY-nMinValY)/nDy));
        fprintf(lpFile,"    \\begin{tabular}[t]{@{}ll@{}}\n");
        for(iR = 0; iR < nR; iR++) {
          if(iR) fprintf(lpFile,"\\\\\n");
          if(CData_GetNRecsPerBlock(dLab) == nR) dlp_strcpy(lpsLab,CData_Sfetch(dLab,iR,0));
          else sprintf(lpsLab,"%d",iR);
          fprintf(lpFile,"      \\color{black!%d}\\rule[1ex]{2em}{1pt}&%s",100-iR*100/nR,(dlp_strlen(lpsLab)==0)? "???" : lpsLab);
        }
        fprintf(lpFile,"\n    \\end{tabular}}}\n");
      }
      fprintf(lpFile,"\\end{psgraph}\n");
    }
  }

  IDESTROY(dLab);
  IDESTROY(dDat);
  IDESTROY(dDatBlk);
  fclose(lpFile);

  return O_K;
}

/* EOF */

