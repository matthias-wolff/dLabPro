// dLabPro program dLabPro (dlabpro)
// - Main program
//
// AUTHOR : Matthias Wolff
// PACKAGE: dLabPro/programs
// 
// Copyright 2013-2014 dLabPro contributors and others (see COPYRIGHT file)
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

#include "dlp_svnrev.h"
#include "dlp_base.h"
#include "dlp_fftproc.h"
#include "dlp_cpproc.h"
#include "dlp_lpcproc.h"
#include "dlp_lsfproc.h"
#include "dlp_lcqproc.h"
#include "dlp_pmproc.h"
#include "dlp_fwtproc.h"
#include "dlp_data.h"
#include "dlp_dgen.h"
#include "dlp_fbaproc.h"
#include "dlp_file.h"
#include "dlp_fst.h"
#include "dlp_fsttools.h"
#include "dlp_fstsearch.h"
#include "dlp_function.h"
#include "dlp_gmm.h"
#include "dlp_helloworld.h"
#include "dlp_histogram.h"
#include "dlp_hmm.h"
#include "dlp_signal.h"
#include "dlp_matrix.h"
#include "dlp_melproc.h"
#include "dlp_fttproc.h"
#include "dlp_vadproc.h"
#include "dlp_hfaproc.h"
#include "dlp_process.h"
#include "dlp_profile.h"
#include "dlp_prosody.h"
#include "dlp_statistics.h"
#include "dlp_svm.h"
#include "dlp_var.h"
#include "dlp_vmap.h"

#if (!defined __NOREADLINE && defined __LINUX)
#  include <readline/readline.h>
#  include <readline/history.h>
#endif

#ifdef __DLP_DEPRECATED
#endif

/**
 * Prints help on usage.
 */
void usage()
{
  dlp_set_binary_name("dLabPro");
  printf("\n// %s",dlp_get_version_info());
  printf("\n\n   A signal processing and pattern recognition toolbox."       );
  printf("\n\n   USAGE:"                                                     );
  printf(  "\n     dlabpro <options> <script file> <script args...>"         );
  printf("\n\n   ARGUMENTS:"                                                 );
  printf(  "\n     script file     dLabPro script file"                      );
  printf(  "\n     script args     White space sep. list of args. to script" );
  printf("\n\n   OPTIONS:"                                                   );
  printf(  "\n        --cd=DIR     Change working directory before starting" );
  printf(  "\n     -h --help       Give this help"                           );
  printf(  "\n        --in-IDE     Called from within an IDE (e.g. Eclipse)" );
  printf(  "\n        --name=XXX   Name session 'XXX' instead of 'dLabPro'"  );
  printf(  "\n        --logo       Print logos at startup and shutdown"      );
  printf(  "\n        --pipemode   Put a line break at the end of prompts"   );
  printf(  "\n        --trace-mem  Print list of memory leaks at shutdown"   );
  printf(  "\n        --verbose=N  Set verbose level of root function"       );
  printf(  "\n        --version    Print version info and exit"              );
  printf("\n\n    Copyright 2013-2014 dLabPro contributors and others (see"  );
  printf(  "\n    COPYRIGHT file)"                                           );
  printf("\n\n"                                                              );
}

/**
 * Starts the dLabPro session.
 *
 * @param iRoot
 *          Pointer to the root function.
 * @param lpArgc
 *          Pointer to the number of command line arguments (content may be
 *          changed!).
 * @param lpArgv
 *          Pointer to the array of command line arguments (content may be
 *          changed!).
 * @return <code>O_K</code> if successful, a (negative) error code otherwise.
 */
INT16 StartSession(CFunction* iRoot, INT32* lpArgc, char** lpArgv)
{
  char lpsBuffer[255] = "";

  iRoot->m_nXm|=XM_NOLOGO;

  // Process command line options
  if (dlp_scancmdlineoption(lpArgc,lpArgv,"--name","=",lpsBuffer,TRUE))
    dlp_strcpy(iRoot->m_lpInstanceName,lpsBuffer);
  if (dlp_scancmdlineoption(lpArgc,lpArgv,"--verbose","=",lpsBuffer,TRUE))
    iRoot->m_nCheck=(short)atoi(lpsBuffer);
  if (dlp_scancmdlineoption(lpArgc,lpArgv,"--logo","",NULL,TRUE))
    iRoot->m_nXm&=~XM_NOLOGO;
  if (dlp_scancmdlineoption(lpArgc,lpArgv,"--pipemode","",NULL,TRUE))
    iRoot->m_nXm|=XM_PIPEMODE;
  if (dlp_scancmdlineoption(lpArgc,lpArgv,"--in-IDE","",NULL,TRUE))
    iRoot->m_nXm|=XM_IN_IDE;
  if (dlp_scancmdlineoption(lpArgc,lpArgv,"--cd","=",lpsBuffer,TRUE))
    dlp_chdir(lpsBuffer,FALSE);
  if (dlp_scancmdlineoption(lpArgc,lpArgv,"--color","",NULL,TRUE))
    dlp_set_color_mode(TRUE);
  if (dlp_scancmdlineoption(lpArgc,lpArgv,"-c","",NULL,FALSE))
    dlp_set_color_mode(TRUE);
 dlp_set_binary_path(lpArgv[0]);
  dlp_set_binary_name(iRoot->m_lpInstanceName);

  // Print logo
  if (!(iRoot->m_nXm&XM_NOLOGO))
    printf("\n// %s",dlp_get_version_info());

  // Load script file
  if (*lpArgc>=2)
  {
    if (!(iRoot->m_nXm&XM_NOLOGO)) printf("\n// Executing %s",lpArgv[1]);
    IF_NOK(iRoot->Load(lpArgv[1])) return NOT_EXEC;
    iRoot->ArgCmdline(*lpArgc-2,&lpArgv[2]);
  }
  if (!(iRoot->m_nXm&XM_NOLOGO)) printf("\n");

#if (!defined __NOREADLINE && defined __LINUX)
  {
    char* lpHome;
    char lpHistory[L_PATH];
    lpHome=getenv("HOME");
    /* Import history from previous sessions */
    if(lpHome!=NULL)
    {
      snprintf(lpHistory,L_PATH-1,"%s/.dlabpro_history",lpHome);
      using_history();
      read_history(lpHistory);
    }
  }
#endif

  return O_K;
}

/**
 * Ends the dLabPro session.
 *
 * @param iRoot
 *          Pointer to the root function.
 */
void EndSession(CFunction* iRoot)
{
  if (!(iRoot->m_nXm&XM_NOLOGO))
    printf("\n// Exiting %s...\n",iRoot->m_lpInstanceName);

#if (!defined __NOREADLINE && defined __LINUX)
  {
    char* lpHome;
    char lpHistory[L_PATH];
    lpHome=getenv("HOME");
    /* Export history from previous sessions */
    if(lpHome!=NULL)
    {
      snprintf(lpHistory,L_PATH-1,"%s/.dlabpro_history",lpHome);
      write_history(lpHistory);
    }
  }
#endif // #if (!defined __NOREADLINE && defined __LINUX)
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
  CFunction* iRoot = NULL;

  // Help and version options
  if
  (
    nArgc == 2 &&
    (
      dlp_scancmdlineoption(&nArgc,lpArgv,"-h"    ,"",NULL,FALSE) ||
      dlp_scancmdlineoption(&nArgc,lpArgv,"--help","",NULL,FALSE)
    )
  )
  {
    usage();
    return 0;
  }
  if (nArgc==2&&dlp_scancmdlineoption(&nArgc,lpArgv,"--version","",NULL,FALSE))
  {
    printf("%s\n",dlp_get_version_info());
    return 0;
  }

  // Initialize
#ifdef _DEBUG
  //dlp_set_kernel_debug(DM_KERNEL);
  dlp_xalloc_init(XA_DLP_MEMLEAKS|XA_HEAP_INTEGRITY/*|XA_HEAP_MEMLEAKS*/);
#else
  dlp_xalloc_init(XA_DLP_MEMLEAKS);
#endif

  BOOL bTml = dlp_scancmdlineoption(&nArgc,lpArgv,"--trace-mem","",NULL,TRUE);

  // Register classes
  REGISTER_CLASS(CDlpObject);
  REGISTER_CLASS(FFTproc);
  REGISTER_CLASS(CCPproc);
  REGISTER_CLASS(CLPCproc);
  REGISTER_CLASS(CLSFproc);
  REGISTER_CLASS(CLCQproc);
  REGISTER_CLASS(CPMproc);
  REGISTER_CLASS(CFWTproc);
  REGISTER_CLASS(CData);
  REGISTER_CLASS(CDgen);
  REGISTER_CLASS(CFBAproc);
  REGISTER_CLASS(CDlpFile);
  REGISTER_CLASS(CFst);
  REGISTER_CLASS(CFsttools);
  REGISTER_CLASS(CFstsearch);
  REGISTER_CLASS(CFunction);
  REGISTER_CLASS(CGmm);
  REGISTER_CLASS(CHelloworld);
  REGISTER_CLASS(CHistogram);
  REGISTER_CLASS(CHmm);
  REGISTER_CLASS(CSignal);
  REGISTER_CLASS(CMatrix);
  REGISTER_CLASS(CMELproc);
  REGISTER_CLASS(CFTTproc);
  REGISTER_CLASS(CVADproc);
  REGISTER_CLASS(CHFAproc);
  REGISTER_CLASS(CProcess);
  REGISTER_CLASS(CProfile);
  REGISTER_CLASS(CProsody);
  REGISTER_CLASS(CStatistics);
  REGISTER_CLASS(CSvm);
  REGISTER_CLASS(CVar);
  REGISTER_CLASS(CVmap);

  // Run dLabPro session
  ICREATEEX(CFunction,iRoot,"dLabPro",NULL);
  IF_OK(StartSession(iRoot,&nArgc,lpArgv)) iRoot->Exec();
  EndSession(iRoot);
  IDESTROY(iRoot);

  // Clean up
  CDlpObject_UnregisterAllClasses();
  dlm_fft_cleanup();
  dlp_arith_cleanup();
  dlp_xalloc_done(bTml);

  // That's it folks ...
  return dlp_get_retval();
}

// EOF
