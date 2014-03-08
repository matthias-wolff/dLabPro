/* dLabPro class CDlpFile (file)
 * - Import data instances from Verbmobil2 signal files with NIST header.
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
#include "dlp_file.h"

#define _F_ERROR(A) { strcpy(sErrorMsg,#A); goto _LF_ERROR; }

/**
 * Reverses the byte order in all numeric cells in instance iDest.
 *
 * @param idDest The data instance to process
 */
void CGEN_SPRIVATE CDlpFile_FlipNumericData(CData* idDest)
{
  INT32 nComp = 0;
  INT32 nRec  = 0;
  INT32 i     = 0;

  for (nComp=0; nComp<CData_GetNComps(idDest); nComp++)
    if (dlp_is_numeric_type_code(CData_GetCompType(idDest,nComp)))
    {
      INT32 nBytes = CData_GetCompSize(idDest,nComp);
      for (nRec=0; nRec<CData_GetNRecs(idDest); nRec++)
      {
        char lpIn[255]; 
        char lpOut[255];
        memmove(lpIn,CData_XAddr(idDest,nRec,nComp),nBytes);
        for (i=0; i<nBytes; i++) lpOut[nBytes-i-1]=lpIn[i];
        memmove(CData_XAddr(idDest,nRec,nComp),lpOut,nBytes);
      }
    }
}

/**
 * Parses one line of the NIST header and returns pointers to the key, the
 * type code and the value. The content of sLine will be modified by placing
 * '\0' characters between the scanned fields.
 *
 * @param sLine  Line to be scanned
 * @param sKey   Will be set to the first character of key in sLine
 * @param sType  Will be set to the first character of type code in sLine
 * @param sValue Will be set to the first character of value in sLine
 * @return TRUE if all pointers are successfully set, FALSE otherwise
 */
BOOL CGEN_SPRIVATE CDlpFile_ParseNIST
(
  char*  sLine,
  char** sKey,
  char** sType,
  char** sValue
)
{
  char* tx = sLine;

  *sKey   = NULL;
  *sType  = NULL;
  *sValue = NULL;
  if (!sLine) return FALSE;

  while (*tx &&  iswspace(*tx)) tx++; if (*tx) *sKey=tx;
  while (*tx && !iswspace(*tx)) tx++; *tx++='\0';
  while (*tx &&  iswspace(*tx)) tx++; if (*tx) *sType=tx;
  while (*tx && !iswspace(*tx)) tx++; *tx++='\0';
  while (*tx &&  iswspace(*tx)) tx++; if (*tx) *sValue=tx;

  return (*sKey && *sType && *sValue);
}

/**
 * Import data instance from Verbmobil2 signal files with NIST header.
 *
 * @param sFilename  Name of file to import.
 * @param iDest      Destination data instance.
 * @param sFiletype  Type of file to import.
 * @return O_K if successful, an error code otherwise
 */
INT16 CGEN_PROTECTED CDlpFile_ImportDataFromVm2Nist
(
  CDlpFile*   _this,
  const char* sFilename,
  CDlpObject* iDst,
  const char* sFiletype
)
{
  char        sHeader[16001];
  char        sErrorMsg[64];
  char        sBuf[256];
  INT32       n            = 0;
  FILE*       f            = NULL;
  CData*      idDest       = NULL;
  char*       tx           = NULL;
  const char* sFns         = NULL;
  char*       sKey         = NULL;
  char*       sType        = NULL;
  char*       sValue       = NULL;
  short       nSampleBytes = 0;
  short       nStreams     = 0;
  int         nHeaderSize  = 1024;
  int         nRead        = 0;
  int         nSamples     = 0;
  int         nSampleRate  = 0;
  char        sByteOrder[] = "        ";
  BOOL        bSwap        = FALSE;

  /* Initialize */
  sFns=&sFilename[dlp_strlen(sFilename)-1];
  while (sFns>sFilename && *sFns!='/' && *sFns!='\\') sFns--;
  if (*sFns!='/' || *sFns!='\\') sFns++;

  idDest=(CData*)iDst;
  CData_Reset(BASEINST(idDest),TRUE);

  IFCHECKEX(1)
  {
    printf("\n -------------------------------------------------------------------------");
    printf("\n  CDlpFile_ImportDataFromVm2Nist"                                          );
    printf("\n  "                                                                        );
    printf("\n  Source file: \"%s\"",sFilename?sFilename:"(null)"                        );
    printf("\n  Destination: %s %s",iDst->m_lpClassName,iDst->m_lpInstanceName           );
    printf("\n -------------------------------------------------------------------------");
  }

  /* Open file and read NIST header */
  f=fopen(sFilename,"rb");
  if (!f) return IERROR(_this,ERR_FILEOPEN,sFilename,"reading",0);
  if(fread(sHeader,sizeof(char),1024,f)!=1024) _F_ERROR(reading file);
  sHeader[1024]=0;

  /* Scan header */
  tx = strtok(sHeader,"\r\n");
  n  = 0;
  while (tx)
  {
    if (dlp_strlen(dlp_strtrimleft(dlp_strtrimright(tx))))
    {
      /* Verify preamble */
      if (n==0)
      {
        if   (strcmp(tx,"NIST_1A")!=0) _F_ERROR(missing preamble)
        else IFCHECKEX(1) printf("\n  000 %-20s = %-20s","Preamble",tx);
      }
      if (n==1)
      {
        sscanf(tx,"%d",&nHeaderSize);
        IFCHECKEX(1) printf("\n  001 %-20s = %d","Header size",nHeaderSize);
        if (nHeaderSize<1024) _F_ERROR(Header too small);
        if (nHeaderSize>(int)sizeof(sHeader)-1) _F_ERROR(header too large);
        if (nHeaderSize>1024)
        {
          nRead = nHeaderSize-1024;
          if ((int)fread(&sHeader[1025],sizeof(char),nRead,f)!=nRead)
            _F_ERROR(file shorter than specified by header);
          sHeader[nHeaderSize+1]='\0';
        }
      }
      /* Scan fields */
      if (n>1 && CDlpFile_ParseNIST(tx,&sKey,&sType,&sValue))
      {
        IFCHECKEX(1) printf("\n  %03d %-20s = %-20s",(int)n,sKey,sValue);

        if (dlp_strcmp(sKey,"sample_coding")==0)
        {
          if (dlp_strcmp(sValue,"pcm")!=0 && dlp_strcmp(sValue,"linear")!=0) _F_ERROR(non-linear sample coding not supported)
          else IFCHECKEX(1) printf(" - linear coding, ok");
        }

        if (dlp_strcmp(sKey,"sample_n_bytes"    )==0) sscanf(sValue,"%hd",&nSampleBytes);
        if (dlp_strcmp(sKey,"channel_count"     )==0) sscanf(sValue,"%hd",&nStreams    );
        if (dlp_strcmp(sKey,"sample_count"      )==0) sscanf(sValue,"%d", &nSamples    );
        if (dlp_strcmp(sKey,"sample_rate"       )==0) sscanf(sValue,"%d", &nSampleRate );
        if (dlp_strcmp(sKey,"sample_byte_format")==0) strncpy(sByteOrder,sValue,sizeof(sByteOrder));
      }

      n++;
    }
    tx = strtok(NULL,"\r\n");
  }

  /* Verifiy scanned header information */
  IFCHECKEX(1)
  {
    printf("\n\n  Scanned file information"                                         );
    printf("\n  - Number of audio streams: %hd"     ,nStreams                       );
    printf("\n  - Bytes per sample       : %hd"     ,nSampleBytes                   );
    printf("\n  - Byte order             : %s"      ,sByteOrder                     );
    printf("\n  - Sample interval        : %0.4g ms",1000./(double)nSampleRate      );
    printf("\n  - Length                 : %g s"    ,1./(double)nSampleRate*nSamples);
  }
  if (nStreams<=0                      ) _F_ERROR(no audio streams               )
  if (nSampleBytes<=0 || nSampleBytes>2) _F_ERROR(sample byte count not supported)
  if (nSampleRate<=0                   ) _F_ERROR(no sample rate specifed        )
  if (nSamples<=0                      ) _F_ERROR(no samples                     )

  if (strcmp(sByteOrder,"01")!=0 && strcmp(sByteOrder,"10")!=0 && strcmp(sByteOrder,"1")!=0)
    IERROR(_this,FIL_FORMATW,sFns,"VM2 NIST","unknown byte order, assuming little endian");

  /* Prepare destination */
  for (n=0; n<nStreams; n++)
  {
    sprintf(sBuf,"ch%02ld",(long)n);
    CData_AddComp(idDest,sBuf,nSampleBytes==2?T_SHORT:T_CHAR);
  }
  CData_AllocateUninitialized(idDest,nSamples);
  CData_SetDescr(idDest,RINC,1000./(FLOAT64)nSampleRate);

  /* Read sample values */
  IFCHECKEX(1) printf("\n\n  Reading file ...");
  n = fread(CData_XAddr(idDest,0,0),nSampleBytes,nSamples*nStreams,f);
  IFCHECKEX(1) printf(" done\n  %ld samples read.",(long)n);
  if(fread(&sBuf[0],1,1,f) != 1) {};
  if (n<nSamples*nStreams) IERROR(_this,FIL_FORMATW,sFns,"VM2 NIST","too few samples in file" );
  if (!feof(f)           ) IERROR(_this,FIL_FORMATW,sFns,"VM2 NIST","too many samples in file");

  /* Flip bytes */
#if defined __I386 || defined __TMS
  bSwap = (strcmp(sByteOrder,"10")==0);
#else
  IERROR(_this,FIL_FORMATW,sFns,"VM2 NIST","cannot detect host byte order");
#endif
  if (bSwap) CDlpFile_FlipNumericData(idDest);

  /* Clean up */
  fclose(f);

  IFCHECKEX(1)
    printf("\n -------------------------------------------------------------------------\n");

  return O_K;

_LF_ERROR:
  fclose(f);
  return IERROR(_this,FIL_FORMAT,sFns,"VM2 NIST",sErrorMsg);
}

/* EOF */
