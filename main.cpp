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

#include "dlp_base.h"
#include "dlp_data.h"
#include "dlp_dgen.h"
#include "dlp_file.h"
#include "dlp_fst.h"
#include "dlp_gmm.h"
#include "dlp_signal.h"
#include "dlp_matrix.h"
#include "dlp_profile.h"
#include "dlp_var.h"
#include "dlp_vmap.h"

/**
 * Run function.
 *
 * @param nArgc
 *         The number of command line arguments.
 * @param lpArgv
 *         Pointer to the array of command line arguments.
 */
void run(int nArgc, char** lpArgv){
  CFst *itX;
  ICREATEEX(CFst,itX,"x",NULL);
  ISETOPTION(itX,"/fst");
  ISETOPTION(itX,"/lsr");
  CFst_Addunit(itX,"xy");
  CFst_Addstates(itX,0,1,FALSE);
  CFst_Addstates(itX,0,1,TRUE);
  CFst_AddtransEx(itX,0,0,1,3,4,15);
  CFst_Print(itX);
  IDESTROY(itX);
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
  REGISTER_CLASS(CData);
  REGISTER_CLASS(CDlpFile);
  REGISTER_CLASS(CFst);
  REGISTER_CLASS(CGmm);
  REGISTER_CLASS(CVar);
  REGISTER_CLASS(CSignal);
  REGISTER_CLASS(CMatrix);
  REGISTER_CLASS(CProfile);
  REGISTER_CLASS(CVmap);

  // Run interessing stuff
  run(nArgc,lpArgv);

  // Clean up
  CDlpObject_UnregisterAllClasses();
  dlm_fft_cleanup();
  dlp_arith_cleanup();
  dlp_xalloc_done(bTml);

  // That's it folks ...
  return dlp_get_retval();
}

// EOF
