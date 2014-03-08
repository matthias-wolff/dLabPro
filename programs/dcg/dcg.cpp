// dLabPro program dcg (dLabPro code generator)
// - Main program
//
// AUTHOR : Matthias Wolff
// PACKAGE: dLabPro/programs
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

#include "dlp_base.h"
#include "dlp_function.h"
#include "dlp_data.h"
#include "dlp_fst.h"
#include "dlp_dgen.h"
#include "dlp_cgen.h"

/**
 * Prints usage hints.
 */
void usage()
{
  printf("\n// %s",dlp_get_version_info());
  printf("\n\n   SYNOPSIS:"                                                  );
  printf(  "\n     dLabPro code generator"                                   );
  printf("\n\n   USAGE:"                                                     );
  printf(  "\n     dcg [<options1>] <def file>"                              );
  printf(  "\n     dcg <options2>"                                           );
  printf("\n\n   ARGUMENTS:"                                                 );
  printf(  "\n     def file        dLabPro class definition file"            );
  printf("\n\n   OPTIONS 1:"                                                 );
  printf(  "\n        --logo       Show logos at startup and shutdown"       );
  printf(  "\n        --verbose=N  Set verbose level"                        );
  printf("\n\n   OPTIONS 2:"                                                 );
  printf(  "\n     -h --help       Give this help and exit"                  );
  printf(  "\n        --version    Print version info and exit"              );
  printf("\n\n    (c) 1995-2013 IAS, TU Dresden, Germany"                    );
  printf(  "\n        www.ias.et.tu-dresden.de"                              );
  printf("\n\n    (c) 2011-2013 Chair of Communications Engineering,"        );
  printf(  "\n        BTU Cottbus, Germany"                                  );
  printf(  "\n        www.tu-cottbus.de/kommunikationstechnik"               );
  printf("\n\n"                                                              );
}

/**
 * Starts the dcg session.
 *
 * @param iCgen
 *          Pointer to the cgen instance.
 * @param lpArgc
 *          Pointer to the number of command line arguments (content may be
 *          changed!).
 * @param lpArgv
 *          Pointer to the array of command line arguments (content may be
 *          changed!).
 * @return <code>O_K</code> if successful, a (negative) error code otherwise.
 */
INT16 startSession(CCgen* iCgen, int* lpArgc, char** lpArgv)
{
  char lpsBuffer[255];

  DLPASSERT(iCgen);

  // Read command line options
  if (dlp_scancmdlineoption(lpArgc,lpArgv,"--logo","",NULL,TRUE))
    iCgen->m_nXm&=~XM_NOLOGO;
  if (dlp_scancmdlineoption(lpArgc,lpArgv,"--verbose","=",lpsBuffer,TRUE))
  {
    iCgen->m_nVerbose = (INT16)atoi(lpsBuffer);
    iCgen->m_nCheck = iCgen->m_nVerbose-2;
  }

  // Ignored command line options
  // TODO: Remove!
  if (dlp_scancmdlineoption(lpArgc,lpArgv,"--nologo","",NULL,TRUE)) {}

  // Print logo
  if (!(iCgen->m_nXm&XM_NOLOGO))
    printf("\n// %s",dlp_get_version_info());

  if (*lpArgc==2)
  {
    // - Change to definition script folder
    char  lpsDir[L_PATH];
    dlp_strcpy(lpsDir,lpArgv[1]);
    char* lpsDefFile = lpsDir;
    for (char* tx = &lpsDir[strlen(lpsDir)-1]; tx>=lpsDir; tx--)
      if (*tx=='/' || *tx=='\\')
      {
        *tx='\0';
        lpsDefFile = tx+1;
        break;
      }
    if (dlp_strlen(lpsDir)>0) dlp_chdir(lpsDir,FALSE);

    // - Load definition script (argument #1)
    return iCgen->Include(lpsDefFile);
  }
  else
  {
    IERROR(iCgen,ERR_INVALARGS,0,0,0);
    fprintf(stderr,"\nUse option -h to get help.");
    return ERR_INVALARGS;
  }
}

/**
 * Ends the dcg session.
 *
 * @param iRoot
 *          Pointer to the cgen instance.
 */
void endSession(CCgen* iCgen)
{
  DLPASSERT(iCgen);

  if (!(iCgen->m_nXm&XM_NOLOGO))
    printf("\n// Exiting %s...\n",dlp_get_binary_name());
  else
    printf("\n");
}

/**
 * Main function.
 *
 * @param nArgc
 *         The number of command line arguments.
 * @param lpArgv
 *         Pointer to the array of command line arguments.
 * @return 0 if successful, an error code otherwise.
 */
int main(int nArgc, char** lpArgv)
{
  CCgen* iCgen;

  // Start session
  //dlp_set_kernel_debug(DM_KERNEL);
  dlp_xalloc_init(/*XA_HEAP_INTEGRITY|XA_HEAP_MEMLEAKS|*/XA_DLP_MEMLEAKS);
  dlp_set_binary_name("dcg");

  // Help and version options
  if
  (
    dlp_scancmdlineoption(&nArgc,lpArgv,"-h"    ,"",NULL,TRUE) ||
    dlp_scancmdlineoption(&nArgc,lpArgv,"--help","",NULL,TRUE)
  )
  {
    usage();
    return 0;
  }
  if (dlp_scancmdlineoption(&nArgc,lpArgv,"--version","",NULL,TRUE))
  {
    printf("%s\n",dlp_get_version_info());
    return 0;
  }

  // Register classes
  REGISTER_CLASS(CFunction)
  REGISTER_CLASS(CData)
  REGISTER_CLASS(CFst)
  REGISTER_CLASS(CDgen)
  REGISTER_CLASS(CCgen)

  // Create root instance
  ICREATEEX(CCgen,iCgen,"cgen",NULL);
  iCgen->m_nXm|=XM_NOLOGO;
  iCgen->m_nXm|=XM_NOPROMPT;

  // Run the dcg session
  IF_OK(startSession(iCgen,&nArgc,lpArgv))
  {
    //iCgen->m_idTsq->Print();
    iCgen->Exec();
  }
  endSession(iCgen);

  // Clean up
  IDESTROY(iCgen);
  CDlpObject_UnregisterAllClasses();
  dlp_xalloc_done(TRUE);

  return dlp_get_retval();
}

// EOF
