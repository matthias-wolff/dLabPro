/* dLabPro class CDlpFile (file)
 * - Import functions of midi notes (data) instances
 *
 * AUTHOR : S. Huebler
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
/**
 * Import midi notes of a midifile into data, needs external program midiconvert
 *
 * @param lpsFilename Name of file to import
 * @param iDst        Pointer to instance to import
 * @param lpsFiletype Type of file to import
 * @return <code>O_K</code> if successful, a (negative) error code otherwise
 */
INT16 CGEN_PROTECTED CDlpFile_Midi_ImportMidi
(
  CDlpFile*   _this,
  const char* lpsFilename,
  CDlpObject* iDst,
  const char* lpsFiletype
)
{
   char  lpsTempFile[L_PATH];
   char  lpsCmdline    [3*L_PATH] = "";
   INT16 nErr                     =O_K;

   strcpy(lpsTempFile,dlp_tempnam(NULL,"dlabpro_midi_import"));
   sprintf(lpsCmdline,"midiconvert %s %s", lpsFilename, lpsTempFile);

   if (system(lpsCmdline)!=0) { nErr=IERROR(_this,FIL_EXEC,lpsCmdline,0,0); }
   else {
      CData *idDst = AS(CData,iDst);
      /*Prepare data*/
      CData_Reset(iDst,TRUE);
      CData_AddComp(idDst,"CHAN",T_UCHAR);
      CData_AddComp(idDst,"VOL",T_UCHAR);
      CData_AddComp(idDst,"INST",T_UCHAR);
      CData_AddComp(idDst,"NOTE",T_UCHAR);
      CData_AddComp(idDst,"TIME",T_UINT);
      CData_AddComp(idDst,"LGTH",T_UINT);
      CData_AddComp(idDst,"VEL",T_UCHAR);
      /*import*/
      IF_NOK(CDlpFile_ImportAsciiToData(_this,lpsTempFile,iDst,"csv"))
        nErr = IERROR(iDst,FIL_IMPORT,lpsTempFile,"csv",0);
      /*Set midifilter specific data descriptions and clean data*/
      CData_SetDescr(idDst, DESCR0, CData_Dfetch(idDst,0,5));
      CData_DeleteRecs(idDst,0,1);
   }
   if (remove(lpsTempFile)==-1) nErr=IERROR(_this,FIL_REMOVE,"temporary ",lpsTempFile,0);
   /* Clean up */
   return nErr;
}

/**
 * Export midi notes of  data instance into midifile, needs external program midiconvert
 *
 * @param lpsFilename Name of file to export
 * @param iSst        Pointer to instance to export
 * @param lpsFiletype Type of file to export
 * @return <code>O_K</code> if successful, a (negative) error code otherwise
 */
INT16 CGEN_PROTECTED CDlpFile_Midi_ExportMidi
(
  CDlpFile*   _this,
  const char* lpsFilename,
  CDlpObject* iSrc,
  const char* lpsFiletype
)
{
  char  lpsTempFile[L_PATH];
  char  lpsCmdline    [3*L_PATH] = "";
  INT16 nErr                     =O_K;

  strcpy(lpsTempFile,dlp_tempnam(NULL,"dlabpro_midi_export"));
  sprintf(lpsCmdline,"midiconvert %s %s", lpsTempFile, lpsFilename);

  CData *idSrc = AS(CData,iSrc);
  CData_InsertRecs(idSrc, 0, 1, 1);
  CData_Dstore(idSrc, CData_GetDescr(idSrc,DESCR0),0,5);
  IF_NOK(CDlpFile_ExportAsciiFromData(_this,lpsTempFile,iSrc,"csv"))
       nErr = IERROR(iSrc,FIL_EXPORT,lpsTempFile,"csv",0);
  CData_DeleteRecs(idSrc,0,1);

  if (system(lpsCmdline)!=0) { nErr=IERROR(_this,FIL_EXEC,lpsCmdline,0,0); }

  if (remove(lpsTempFile)==-1) nErr=IERROR(_this,FIL_REMOVE,"temporary ",lpsTempFile,0);

  /* Clean up */
  return nErr;
}

/* EOF */
