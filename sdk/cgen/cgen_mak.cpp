// dLabPro SDK class CCgen (cgen)
// - Generating make files and MS VC++ project files (*.mak, *.dsw, *dsp)
//
// AUTHOR : Matthias Wolff
// PACKAGE: dLabPro/sdk
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

#include "dlp_cgen.h"

INT16 CGEN_PROTECTED CCgen::CreateMakeFile(INT16 bIfnotex)
{
  if (GetVerboseLevel()>1) printf("\nCreating project environment files...");

  FILE* f;
  
  char lpMakfn[255];  dlp_convert_name(CN_MAKEFILE,lpMakfn ,m_lpsProject);
  char lpNmakfn[255]; dlp_convert_name(CN_NMAKFILE,lpNmakfn,m_lpsProject);
  char lpDspfn[255];  dlp_convert_name(CN_DSPFILE ,lpDspfn ,m_lpsProject);
  char lpDswfn[255];  dlp_convert_name(CN_DSWFILE ,lpDswfn ,m_lpsProject);
  char lpGenfn[255];  dlp_convert_name(CN_GENFILE ,lpGenfn ,m_lpsProject);
  char lpNcbfn[255];  dlp_convert_name(CN_NCBFILE ,lpNcbfn ,m_lpsProject);

  switch (m_nPlatform)
  {
  case PF_GNUCXX:
    if (!m_bMainProject)
    {
      RealizeCopyright(m_makFileTmpl,"## ");
      ReplaceAllKeys(m_makFileTmpl);
      RealizeObjectList(m_makFileTmpl,m_files);
      RealizeCDeps(m_makFileTmpl,m_files);
      RealizeDependencies(m_makFileTmpl,m_files);
      RealizeCompiles(m_makFileTmpl,m_files);
      // Only write back if not exists or in always overwrite mode
      f = fopen("makefile","r");
      if (!bIfnotex || !f) WriteBackTemplate(m_makFileTmpl,NULL,lpMakfn);
      if (f) fclose(f);
    }
    break;
  case PF_MSDEV5:
  case PF_MSDEV6:
    ReplaceAllKeys(m_dspFileTmpl);
    RealizeDspCompiles(m_dspFileTmpl,m_files);
    RealizeDspLibraries(m_dspFileTmpl,m_icls);
    // Only write back if not exists or in always overwrite mode
    f = fopen(lpDspfn,"r");
    if (!bIfnotex || !f) WriteBackTemplate(m_dspFileTmpl,NULL,lpDspfn);
    if (f) fclose(f);

    ReplaceAllKeys(m_dswFileTmpl);
    f = fopen(lpDswfn,"r");
    if (!bIfnotex || !f) WriteBackTemplate(m_dswFileTmpl,NULL,lpDswfn);
    if (f) fclose(f);

    remove(lpNcbfn);
    // NO BREAK - fall through to ordinary MSDEV behaviour.
  case PF_MSDEV4:
    RealizeCleanupList(m_nmakFileTmpl,m_files);
    RealizeObjectList(m_nmakFileTmpl,m_files);
    RealizeDependencies(m_nmakFileTmpl,m_files);
    RealizeCompiles(m_nmakFileTmpl,m_files);
    RealizeLinks(m_nmakFileTmpl,m_icls);
    ReplaceAllKeys(m_nmakFileTmpl);
    // Only write back if not exists or in always overwrite mode
    f = fopen(lpMakfn,"r");
    if (!bIfnotex || !f) WriteBackTemplate(m_nmakFileTmpl,NULL,lpNmakfn);
    if (f) fclose(f);

//    ReplaceAllKeys(m_genFileTmpl);
//    WriteBackTemplate(m_genFileTmpl,NULL,lpGenfn);

    break;
  }
    
  return O_K;  
}

INT16 CGEN_PROTECTED CCgen::RealizeObjectList(CList<SCGStr> &tmpl, CList<SCGFile> &files)
{
  char     buf[255];
  char     fn[100];
  INT16    cppctr;
  char    *cp;
  SCGFile *pfile;
  SCGStr  *ptmpl;

  ptmpl = tmpl.m_items;
  while (ptmpl)
  {
    if ((m_nPlatform == PF_GNUCXX && strstr(ptmpl->lpName,"OBJECTS        =") !=NULL) ||
        (m_nPlatform == PF_MSDEV4 && strstr(ptmpl->lpName,"LIB32_OBJS="     ) !=NULL) ||
        (m_nPlatform == PF_MSDEV5 && strstr(ptmpl->lpName,"LIB32_OBJS="     ) !=NULL) ||
        (m_nPlatform == PF_MSDEV6 && strstr(ptmpl->lpName,"LIB32_OBJS="     ) !=NULL)  )
    {
      pfile = files.m_items; cppctr = 0; 
      while (pfile) 
      {
        if (pfile->nFType == FT_CPP || pfile->nFType == FT_C) cppctr++; 
        pfile = pfile->next;
      }
      if (cppctr > 0) strcat(ptmpl->lpName," \\");
      pfile = files.m_items;
      while (pfile)
      {
        if (pfile->nFType == FT_CPP || pfile->nFType == FT_C)
        {
          strcpy(fn,pfile->lpName);
          cp = &fn[strlen(fn)-1]; while (cp != fn && *cp != '.') cp--; *cp = '\0';
          if (m_nPlatform == PF_GNUCXX)
          {
            if (cppctr == 1) sprintf(buf,"                  $(OBJ_PATH)/%s.$(OEXT)"   ,fn);
            else             sprintf(buf,"                  $(OBJ_PATH)/%s.$(OEXT) \\",fn);
          }
          if (m_nPlatform == PF_MSDEV4 || m_nPlatform == PF_MSDEV5 || m_nPlatform == PF_MSDEV6)
          {
            if (cppctr == 1) sprintf(buf,"\t\"$(INTDIR)/%s.obj\"",fn);
            else             sprintf(buf,"\t\"$(INTDIR)/%s.obj\" \\",fn);
          }
          cppctr--;
          tmpl.InsertItem(ptmpl,buf);
          ptmpl = ptmpl->next;
        }
        pfile = pfile->next;
      }
    }
    ptmpl = ptmpl->next;
  }

  return O_K;
}

INT16 CGEN_PROTECTED CCgen::RealizeCDeps(CList<SCGStr> &tmpl, CList<SCGFile> &files)
{
  char     buf[255];
  INT16    cppctr;
  SCGFile *pfile;
  SCGStr  *ptmpl;

  ptmpl = tmpl.m_items;
  while (ptmpl)
  {
    if (m_nPlatform == PF_GNUCXX && strstr(ptmpl->lpName,"CDEPS =") !=NULL)
    {
      pfile = files.m_items; cppctr = 0; 
      while (pfile) 
      {
        if (pfile->nFType == FT_CPP || pfile->nFType == FT_C) cppctr++; 
        pfile = pfile->next;
      }
      if (cppctr > 0) strcat(ptmpl->lpName," \\");
      pfile = files.m_items;
      while (pfile)
      {
        if (pfile->nFType == FT_CPP || pfile->nFType == FT_C)
        {          
          if (cppctr == 1) sprintf(buf,"         %s"   ,pfile->lpName);
          else             sprintf(buf,"         %s \\",pfile->lpName);  
          cppctr--;
          tmpl.InsertItem(ptmpl,buf);
          ptmpl = ptmpl->next;
        }
        pfile = pfile->next;
      }
    }
    ptmpl = ptmpl->next;
  }

  return O_K;
}

INT16 CGEN_PROTECTED CCgen::RealizeDependencies(CList<SCGStr> &tmpl, CList<SCGFile> &files)
{
  char     buf[255];
  INT16    hctr;
  SCGFile *pfile;
  SCGStr  *ptmpl;
  char     cDir   = C_DIR;

  ptmpl = tmpl.m_items;
  if (m_nPlatform == PF_GNUCXX) cDir = '/';
  while (ptmpl != NULL)
  {
    if (strstr(ptmpl->lpName,"DEPENDENCIES=") !=NULL)
    {
      pfile = files.m_items; hctr = 0; 
      while (pfile) {if (pfile->nFType == FT_H) hctr++; pfile = pfile->next;}
      if (hctr > 0) strcat(ptmpl->lpName," \\");
      pfile = files.m_items;
      while (pfile)
      {
        if (pfile->nFType == FT_H)
        {
          if (hctr == 1) sprintf(buf,"             $(INC_PATH)%c%s"   ,cDir,pfile->lpName);
          else           sprintf(buf,"             $(INC_PATH)%c%s \\",cDir,pfile->lpName);
          hctr--;
          tmpl.InsertItem(ptmpl,buf);
          ptmpl = ptmpl->next;
        }
        pfile = pfile->next;
      }
    }
    ptmpl = ptmpl->next;
  }

  return O_K;
}

INT16 CGEN_PROTECTED CCgen::RealizeCompiles(CList<SCGStr> &tmpl, CList<SCGFile> &files)
{
  char     buf[255];
  char     fn[100];
  char    *cp;
  SCGFile *pfile;
  SCGStr  *ptmpl;
  INT16   bDoneReplacement = FALSE;

  ptmpl = tmpl.m_items;
  while (ptmpl)
  {
    if (bDoneReplacement) break;

    if ((m_nPlatform == PF_GNUCXX && strstr(ptmpl->lpName,"##{{CGEN_COMPILE")    != NULL) ||
        (m_nPlatform == PF_MSDEV4 && strstr(ptmpl->lpName,"# Begin Source File") != NULL) ||
        (m_nPlatform == PF_MSDEV5 && strstr(ptmpl->lpName,"# Begin Source File") != NULL) ||
        (m_nPlatform == PF_MSDEV6 && strstr(ptmpl->lpName,"# Begin Source File") != NULL)  )
    {
      bDoneReplacement = (m_nPlatform == PF_MSDEV5 || m_nPlatform == PF_MSDEV6);

      pfile = files.m_items;
      while (pfile)
      {
        if (pfile->nFType == FT_CPP || pfile->nFType == FT_C)
        {
          strcpy(fn,pfile->lpName);
          cp = &fn[strlen(fn)-1]; while (cp != fn && *cp != '.') cp--; *cp = '\0';
          switch (m_nPlatform)
          {
          case PF_GNUCXX:
            sprintf(buf,"$(OBJ_PATH)/%s.$(OEXT): %s %s\n\t$(CC) -c $(CFLAGS) $(INCL) $(CCoO)$(OBJ_PATH)/%s.$(OEXT) %s\n",
                  fn,pfile->lpName,dlp_convert_name(CN_HFILE,dlp_get_a_buffer(),m_lpsProject),fn,pfile->lpName);
            tmpl.InsertItem(ptmpl,buf);
            ptmpl = ptmpl->next;
            break;
          case PF_MSDEV4:
          case PF_MSDEV5:
          case PF_MSDEV6:
            tmpl.InsertItem(ptmpl,""); ptmpl = ptmpl->next;
            sprintf(buf,"SOURCE=.\\%s.",fn);
            if (pfile->nFType == FT_CPP) strcat(buf,"cpp");
            else                         strcat(buf,"c");
            tmpl.InsertItem(ptmpl,buf); ptmpl = ptmpl->next;
            tmpl.InsertItem(ptmpl,""); ptmpl = ptmpl->next;
            sprintf(buf,"\"$(INTDIR)\\%s.obj\" : $(SOURCE) $(DEPENDENCIES) \"$(INTDIR)\"",fn);
            tmpl.InsertItem(ptmpl,buf); ptmpl = ptmpl->next;
            tmpl.InsertItem(ptmpl,""); ptmpl = ptmpl->next;
            tmpl.InsertItem(ptmpl,""); ptmpl = ptmpl->next;
            tmpl.InsertItem(ptmpl,"# End Source File"); ptmpl = ptmpl->next;
            tmpl.InsertItem(ptmpl,"################################################################################"); 
            ptmpl = ptmpl->next;
            tmpl.InsertItem(ptmpl,"# Begin Source File"); ptmpl = ptmpl->next;
            break;
          }
        }
        pfile = pfile->next;
      }
    }
    ptmpl = ptmpl->next;
  }

  return O_K;
}

INT16 CGEN_PROTECTED CCgen::RealizeDspCompiles(CList<SCGStr> &tmpl, CList<SCGFile> &files)
{
  // Nothing to be done for sigmaLab projects
  if (m_bMainProject) return O_K;

  // Realize compiles
  char     buf[255];
  char     fn[100];
  char    *cp;
  SCGFile *pfile;
  SCGStr  *ptmpl;
  INT16   bDoneReplacement = FALSE;

  ptmpl = tmpl.m_items;
  while (ptmpl != NULL)
  {
    if (bDoneReplacement) break;

    if ((m_nPlatform == PF_MSDEV5 || m_nPlatform == PF_MSDEV6) && 
        strstr(ptmpl->lpName,"# Begin Source File") != NULL)
    {
      bDoneReplacement = (m_nPlatform == PF_MSDEV5 || m_nPlatform == PF_MSDEV6);

      pfile = files.m_items;
      while (pfile)
      {
        if (pfile->nFType == FT_CPP || pfile->nFType == FT_C)
        {
          strcpy(fn,pfile->lpName);
          cp = &fn[strlen(fn)-1]; while (cp != fn && *cp != '.') cp--; *cp = '\0';
          switch (m_nPlatform)
          {
          case PF_MSDEV5:
          case PF_MSDEV6:
            sprintf(buf,"SOURCE=.\\%s.",fn);
            if (pfile->nFType == FT_CPP) strcat(buf,"cpp");
            else                         strcat(buf,"c");
            tmpl.InsertItem(ptmpl,buf); ptmpl = ptmpl->next;
            tmpl.InsertItem(ptmpl,""); ptmpl = ptmpl->next;
            tmpl.InsertItem(ptmpl,"# End Source File"); ptmpl = ptmpl->next;
            tmpl.InsertItem(ptmpl,"# Begin Source File"); ptmpl = ptmpl->next;
            break;
          }
        }
        pfile = pfile->next;
      }
    }
    ptmpl = ptmpl->next;
  }

  return O_K;
}

INT16 CGEN_PROTECTED CCgen::RealizeDspLibraries(CList<SCGStr> &tmpl, CList<SCGIcl> &icls)
{
  // Nothing to be done for SEM projects
  if (!m_bMainProject) return O_K;

  // Realize libraries
  SCGStr* ptmpl = tmpl.m_items;
  while (ptmpl != NULL)
  {
    if (strstr(ptmpl->lpName,"# Begin Group \"Library Files\"") != NULL)
    {
      // Skip two lines
      for (INT32 i=2;i--;) ptmpl = ptmpl->next;

      // Insert libraries
      SCGIcl* lpIcl = icls.m_items;
      while (lpIcl)
      {
        switch (m_nPlatform)
        {
        case PF_MSDEV5:
        case PF_MSDEV6: {
          char lpBuf[L_PATH];
          sprintf(lpBuf,"SOURCE=%s.intel%c%s",lpIcl->lpLPath,C_DIR,lpIcl->lpLFile);
          tmpl.InsertItem(ptmpl,"# Begin Source File"); ptmpl = ptmpl->next;
          tmpl.InsertItem(ptmpl,""                   ); ptmpl = ptmpl->next;
          tmpl.InsertItem(ptmpl,lpBuf                ); ptmpl = ptmpl->next;
          tmpl.InsertItem(ptmpl,"# End Source File"  ); ptmpl = ptmpl->next;
          break;}
        }
        lpIcl = lpIcl->next;
      }

      // Insert additional libraries
      if (m_bMainProject)
      {
        SCGStr* lpLib = m_lFiles.m_items;
        while (lpLib)
        {
          switch (m_nPlatform)
          {
          case PF_MSDEV5:
          case PF_MSDEV6: {
            char lpBuf[L_PATH];
            sprintf(lpBuf,"SOURCE=%s",lpLib->lpName);
            tmpl.InsertItem(ptmpl,"# Begin Source File"); ptmpl = ptmpl->next;
            tmpl.InsertItem(ptmpl,""                   ); ptmpl = ptmpl->next;
            tmpl.InsertItem(ptmpl,lpBuf                ); ptmpl = ptmpl->next;
            tmpl.InsertItem(ptmpl,"# End Source File"  ); ptmpl = ptmpl->next;
            break;}
          }
          lpLib = lpLib->next;
        }
      }

      break;  // Stop seeking for replacements
    }
    ptmpl = ptmpl->next;
  }

  return O_K;
}
  
INT16 CGEN_PROTECTED CCgen::RealizeCleanupList(CList<SCGStr> &tmpl, CList<SCGFile> &files)
{
  char     buf[255];
  char     fn[100];
  INT16    cppctr;
  char    *cp;
  SCGFile *pfile;
  SCGStr  *ptmpl;

  ptmpl = tmpl.m_items;
  while (ptmpl != NULL)
  {
    if ((m_nPlatform == PF_MSDEV4 || m_nPlatform == PF_MSDEV5 || m_nPlatform == PF_MSDEV6) && 
        strstr(ptmpl->lpName,"CLEAN :" ) !=NULL)
    {
      pfile = files.m_items; cppctr = 0; 
      while (pfile) 
      {
        if (pfile->nFType == FT_CPP || pfile->nFType == FT_C) cppctr++; 
        pfile = pfile->next;
      }
      if (cppctr > 0) strcat(ptmpl->lpName," \\");
      pfile = files.m_items;
      while (pfile)
      {
        if (pfile->nFType == FT_CPP || pfile->nFType == FT_C)
        {
          strcpy(fn,pfile->lpName);
          cp = &fn[strlen(fn)-1]; while (cp != fn && *cp != '.') cp--; *cp = '\0';
          if (strstr(ptmpl->next->lpName,"\".\\${OPathR}.intel\\") != NULL)
            sprintf(buf,"\t-@erase \".\\${OPathR}.intel\\%s.obj\"",fn);
          if (strstr(ptmpl->next->lpName,"\".\\${OPath}.intel\\") != NULL)
            sprintf(buf,"\t-@erase \".\\${OPath}.intel\\%s.obj\"",fn);
          cppctr--;
          tmpl.InsertItem(ptmpl,buf);
          ptmpl = ptmpl->next;
        }
        pfile = pfile->next;
      }
    }
    ptmpl = ptmpl->next;
  }

  return O_K;
}

INT16 CGEN_PROTECTED CCgen::RealizeLinks(CList<SCGStr> &tmpl, CList<SCGIcl> &icls)
{
  for (SCGStr* ptmpl=tmpl.m_items; ptmpl; ptmpl=ptmpl->next)
  {
    if ((m_nPlatform == PF_MSDEV4 || m_nPlatform == PF_MSDEV5 || m_nPlatform == PF_MSDEV6) && 
        strstr(ptmpl->lpName,"LINK32_OBJS= \\"))
    {
      // -- Skip two lines
      ptmpl=ptmpl->next;

      // -- Write down SEM list
      for (SCGIcl* lpIcl=icls.m_items; lpIcl; lpIcl=lpIcl->next)
      {
        char lpBuf[255];
        char lpBuf2[255];
        sprintf(lpBuf,"\t\"%s.intel\\%s\"",lpIcl->lpLPath,lpIcl->lpLFile);
        dlp_convert_name(CN_XLATPATH,lpBuf2,lpBuf);
        strcat(lpBuf2," \\");
        tmpl.InsertItem(ptmpl,lpBuf2);
        ptmpl = ptmpl->next;
      }
    }
  }

  return O_K;
}

// -- File Templates --

#define LINEMAK(A)  m_makFileTmpl.AddItem(A);

void CGEN_PROTECTED CCgen::CreateMakefileTemplate()
{
  if(IsSdkResource(m_lpsHomePath)){
    LINEMAK("## dLabPro SDK class ${CxxClass} (${SLName})"                     )
  }else{
    LINEMAK("## dLabPro class ${CxxClass} (${SLName})"                         )
  }
  LINEMAK("## - Makefile"                                                      )
  LINEMAK("##"                                                                 )
  LINEMAK("## AUTHOR : ${Author}"                                              )
  LINEMAK("## PACKAGE: ${Package}"                                             )
  LINEMAK("##"                                                                 )
  LINEMAK("## This file was generated by dcg. DO NOT MODIFY! Modify ${DefFile} instead.")
  LINEMAK("${Copyright}"                                                       )
  LINEMAK(""                                                                   )
  LINEMAK("## Common settings"                                                 )
  LINEMAK("ifdef (${CGENPATH})"                                                )
  LINEMAK("  CGEN = ${CGENPATH}/dcg"                                           )
  LINEMAK("else"                                                               )
  LINEMAK("  CGEN = dcg"                                                       )
  LINEMAK("endif"                                                              )
  LINEMAK(""                                                                   )
  LINEMAK("vpath %.h ${IPath} ${IPath}/automatic"                              )
  LINEMAK("INCL = ${Include}"                                                  )
  LINEMAK(""                                                                   )
  LINEMAK("ifneq (${MACHINE},)"                                                )
  LINEMAK("  MEXT=.${MACHINE}"                                                 )
  LINEMAK("else"                                                               )
  LINEMAK("  MEXT="                                                            )
  LINEMAK("endif"                                                              )
  LINEMAK(""                                                                   )
  LINEMAK("## Compiler specific settings"                                      )
  LINEMAK("ifeq (${DLABPRO_USE_MSVC},1)"                                       )
  LINEMAK("  ## - MSVC"                                                        )
  LINEMAK("  CC       = CL"                                                    )
  LINEMAK("  CFLAGS   = -nologo -Od -Gm -EHsc -RTC1 -Wp64 -ZI -TP -D_DEBUG "
                       "-D_DLP_CPP ${MsvcFlags} ${DLABPRO_MSVC_FLAGS_DEBUG}"   )
  LINEMAK("  CCoO     = -Fo"                                                   )
  LINEMAK("  AR       = LIB"                                                   )
  LINEMAK("  ARFLAGS  = -nologo"                                               )
  LINEMAK("  ARoO     = -OUT:"                                                 )
  LINEMAK("  OEXT     = obj"                                                   )
  LINEMAK("  LEXT     = lib"                                                   )
  LINEMAK("  TOOLBOX  = MSVC"                                                  )
  LINEMAK("else"                                                               )
  LINEMAK("  ifeq (${DLABPRO_USE_MSVC},2)"                                     )
  LINEMAK("    ## - MSVC 6.0 - 32-Bit C/C++-Compiler for x86"                  )
  LINEMAK("    CC       = CL"                                                  )
  LINEMAK("    CFLAGS   = -nologo -Od -Gm -EHsc -RTC1 -ZI -TP -D_DEBUG "
                         "-D_DLP_CPP ${MsvcFlags} ${DLABPRO_MSVC_FLAGS_DEBUG}" )
  LINEMAK("    CCoO     = -Fo"                                                 )
  LINEMAK("    AR       = LIB"                                                 )
  LINEMAK("    ARFLAGS  = -nologo"                                             )
  LINEMAK("    ARoO     = -OUT:"                                               )
  LINEMAK("    OEXT     = obj"                                                 )
  LINEMAK("    LEXT     = lib"                                                 )
  LINEMAK("    TOOLBOX  = MSVC6"                                               )
  LINEMAK("  else"                                                             )
  LINEMAK("    ## - GCC"                                                       )
  LINEMAK("    CC       = gcc"                                                 )
  LINEMAK("    CFLAGS   = -g -D_DEBUG -Wall -ansi -x c++ -D_DLP_CPP ${GccFlags} "
                         "${DLABPRO_GCC_CFLAGS_DEBUG}"                         )
  LINEMAK("    CCoO     = -o"                                                  )
  LINEMAK("    AR       = ar"                                                  )
  LINEMAK("    ARFLAGS  = rvs"                                                 )
  LINEMAK("    ARoO     ="                                                     )
  LINEMAK("    OEXT     = o"                                                   )
  LINEMAK("    LEXT     = a"                                                   )
  LINEMAK("    TOOLBOX  = GCC"                                                 )
  LINEMAK("  endif"                                                            )
  LINEMAK("endif"                                                              )
  LINEMAK(""                                                                   )
  LINEMAK("## Configuration - DEBUG_CPP (default)"                             )
  LINEMAK("OBJ_PATH = ${OPath}.debug${MEXT}"                                   )
  LINEMAK("LIB_PATH = ${LPath}.debug${MEXT}"                                   )
  LINEMAK(""                                                                   )
  LINEMAK("ifeq ($(MAKECMDGOALS),)"                                            )
  LINEMAK("  MAKECMDGOALS = DEBUG_CPP"                                         )
  LINEMAK("endif"                                                              )
  LINEMAK(""                                                                   )
  LINEMAK("## Configuration - DEBUG_C"                                         )
  LINEMAK("ifeq ($(MAKECMDGOALS),DEBUG_C)"                                     )
  LINEMAK("  OBJ_PATH = ${OPath}.debug${MEXT}"                                 )
  LINEMAK("  LIB_PATH = ${LPath}.debug${MEXT}"                                 )
  LINEMAK("  ifeq (${DLABPRO_USE_MSVC},1)"                                     )
  LINEMAK("    CFLAGS = -nologo -Od -Gm -EHsc -RTC1 -Wp64 -ZI -TC -D_DEBUG "
                       "-D_DLP_C -D_DLP_C ${MsvcFlags} "
                       "${DLABPRO_MSVC_FLAGS_DEBUG}"                           )
  LINEMAK("  else"                                                             )
  LINEMAK("    ifeq (${DLABPRO_USE_MSVC},2)"                                   )
  LINEMAK("      ## - MSVC 6.0 - 32-Bit C/C++-Compiler for x86"                )
  LINEMAK("      CFLAGS = -nologo -Od -Gm -EHsc -RTC1 -ZI -TC -D_DEBUG "
                         "-D_DLP_C -D_DLP_C ${MsvcFlags} "
                         "${DLABPRO_MSVC_FLAGS_DEBUG}"                         )
  LINEMAK("    else"                                                           )
  LINEMAK("      CFLAGS = -g -D_DEBUG -Wall -ansi -x c -D_DLP_C ${GccFlags} "
                         "${DLABPRO_GCC_CFLAGS_DEBUG}"                         )
  LINEMAK("    endif"                                                          )
  LINEMAK("  endif"                                                            )
  LINEMAK("endif"                                                              )
  LINEMAK(""                                                                   )
  LINEMAK("## Configuration - RELEASE_CPP"                                     )
  LINEMAK("ifeq ($(MAKECMDGOALS),RELEASE_CPP)"                                 )
  LINEMAK("  OBJ_PATH = ${OPath}.release${MEXT}"                               )
  LINEMAK("  LIB_PATH = ${LPath}.release${MEXT}"                               )
  LINEMAK("  ifeq (${DLABPRO_USE_MSVC},1)"                                     )
  LINEMAK("    CFLAGS  = -nologo -O2 -GL -D_RELEASE -EHsc -W3 -Wp64 -TP "
                        "-D_CRT_SECURE_NO_WARNINGS -D_DLP_CPP ${MsvcFlags} "
                        "${DLABPRO_MSVC_FLAGS_RELEASE}"                        )
  LINEMAK("    ARFLAGS = -nologo -LTCG"                                        )
  LINEMAK("  else"                                                             )
  LINEMAK("    ifeq (${DLABPRO_USE_MSVC},2)"                                   )
  LINEMAK("      ## - MSVC 6.0 - 32-Bit C/C++-Compiler for x86"                )
  LINEMAK("      CFLAGS  = -nologo -O2 -D_RELEASE -EHsc -W3 -TP "
                          "-D_CRT_SECURE_NO_WARNINGS -D_DLP_CPP ${MsvcFlags} "
                          "${DLABPRO_MSVC_FLAGS_RELEASE}"                      )
  LINEMAK("    else"                                                           )
  LINEMAK("      CFLAGS  = -O2 -D_RELEASE -Wall -ansi -x c++ -D_DLP_CPP "
                          "${GccFlags} ${DLABPRO_GCC_CFLAGS_RELEASE}"          )
  LINEMAK("    endif"                                                          )
  LINEMAK("  endif"                                                            )
  LINEMAK("endif"                                                              )
  LINEMAK(""                                                                   )
  LINEMAK("## Configuration - RELEASE_C"                                       )
  LINEMAK("ifeq ($(MAKECMDGOALS),RELEASE_C)"                                   )
  LINEMAK("  OBJ_PATH = ${OPath}.release${MEXT}"                               )
  LINEMAK("  LIB_PATH = ${LPath}.release${MEXT}"                               )
  LINEMAK("  ifeq (${DLABPRO_USE_MSVC},1)"                                     )
  LINEMAK("    CFLAGS  = -nologo -O2 -GL -D_RELEASE -EHsc -W3 -Wp64 -TC "
                        "-D_CRT_SECURE_NO_WARNINGS -D_DLP_C ${MsvcFlags} "
                        "${DLABPRO_MSVC_FLAGS_RELEASE}"                        )
  LINEMAK("    ARFLAGS = -nologo -LTCG"                                        )
  LINEMAK("  else"                                                             )
  LINEMAK("    ifeq (${DLABPRO_USE_MSVC},2)"                                   )
  LINEMAK("      ## - MSVC 6.0 - 32-Bit C/C++-Compiler for x86"                )
  LINEMAK("      CFLAGS  = -nologo -O2 -D_RELEASE -EHsc -W3 -TC "
                          "-D_CRT_SECURE_NO_WARNINGS -D_DLP_C ${MsvcFlags} "
                          "${DLABPRO_MSVC_FLAGS_RELEASE}"                      )
  LINEMAK("    else"                                                           )
  LINEMAK("      CFLAGS  = -O2 -D_RELEASE -Wall -ansi -x c -D_DLP_C ${GccFlags} "
                          "${DLABPRO_GCC_CFLAGS_RELEASE}"                      )
  LINEMAK("    endif"                                                          )
  LINEMAK("  endif"                                                            )
  LINEMAK("endif"                                                              )
  LINEMAK(""                                                                   )
  LINEMAK("## Configuration - clean_release"                                   )
  LINEMAK("ifeq ($(MAKECMDGOALS),clean_release)"                               )
  LINEMAK("  OBJ_PATH = ${OPath}.release${MEXT}"                               )
  LINEMAK("  LIB_PATH = ${LPath}.release${MEXT}"                               )
  LINEMAK("endif"                                                              )
  LINEMAK(""                                                                   )
  LINEMAK("## Target settings"                                                 )
  LINEMAK("MANFILE        = ${MPath}/${ProjL}.html"                            )
  LINEMAK("LIBRARY        = $(LIB_PATH)/${Libfile}.$(LEXT)"                    )
  LINEMAK("SHARED_LIBRARY = lib${Libfile}.so"                                  )
  LINEMAK("OBJECTS        = $(OBJ_PATH)/${ProjL}.$(OEXT)"                      )
  LINEMAK("CDEPS          = "                                                  )
  LINEMAK("DEPS           = ${DefFile}"                                        )
  LINEMAK(""                                                                   )
  LINEMAK("## Rules "                                                          )
  LINEMAK("DEBUG_CPP  : ECHOCNF MKDIR $(LIBRARY)"                              )
  LINEMAK("DEBUG_C    : ECHOCNF MKDIR $(LIBRARY)"                              )
  LINEMAK("RELEASE_CPP: ECHOCNF MKDIR $(LIBRARY)"                              )
  LINEMAK("RELEASE_C  : ECHOCNF MKDIR $(LIBRARY)"                              )
  LINEMAK("SHARED     : ECHOCNF MKDIR $(SHARED_LIBRARY) LDCONF"                )
  LINEMAK(""                                                                   )
  LINEMAK("$(LIBRARY): $(OBJECTS)"                                             )
  LINEMAK("\t$(AR) $(ARFLAGS) $(ARoO)$(LIBRARY) $(OBJECTS)"                    )
  LINEMAK(""                                                                   )
  LINEMAK("$(SHARED_LIBRARY): $(OBJECTS)"                                      )
  LINEMAK("\t$(CC) -shared -Wl,-soname,$(SHARED_LIBRARY).0 $(OBJECTS) \\"      )
  LINEMAK("          -o $(LIB_PATH)/$(SHARED_LIBRARY).0.0"                     )
  LINEMAK(""                                                                   )
  LINEMAK("$(OBJ_PATH)/${ProjL}.$(OEXT): ${CPPFile} ${HFile} dlp_object.h"     )
  LINEMAK("\t$(CC) -c $(CFLAGS) $(INCL) $(CCoO)$(OBJ_PATH)/${ProjL}.$(OEXT)"
          " ${CPPFile}"                                                        )
  LINEMAK(""                                                                   )
  LINEMAK("${HFile}: $(DEPS) $(CDEPS)"                                         )
  LINEMAK("\t@-$(CGEN) ${DefFile}"                                             )
  LINEMAK(""                                                                   )
  LINEMAK("${CPPFile}: $(DEPS) $(CDEPS)"                                       )
  LINEMAK("\t@-$(CGEN) ${DefFile}"                                             )
  LINEMAK(""                                                                   )
  LINEMAK("##{{CGEN_COMPILE"                                                   )
  LINEMAK("##}}CGEN_COMPILE"                                                   )
  LINEMAK(""                                                                   )
  LINEMAK("## Additional rules"                                                )
  LINEMAK(".PHONY: ECHOCNF MKDIR LDCONF clean clean_debug clean_release"       )
  LINEMAK(""                                                                   )
  LINEMAK("# Echo current configuration"                                       )
  LINEMAK("ECHOCNF:"                                                           )
  LINEMAK("\t@echo"                                                            )
  LINEMAK("\t@echo \'// ----- Make ($(TOOLBOX)): dLabPro class ${CxxClass} "
                  "(${SLName}) -- $(MAKECMDGOALS) -----\'"                     )
  LINEMAK(""                                                                   )
  LINEMAK("# Create target directory"                                          )
  LINEMAK("MKDIR:"                                                             )
  LINEMAK("\t@-test -w $(OBJ_PATH) || mkdir $(OBJ_PATH)"                       )
  LINEMAK("\t@-test -w $(LIB_PATH) || mkdir $(LIB_PATH)"                       )
  LINEMAK(""                                                                   )
  LINEMAK("# Create links to shared lib"                                       )
  LINEMAK("LDCONF:"                                                            )
  LINEMAK("\t@-cd $(LIB_PATH) && ln -sf $(SHARED_LIBRARY).0.0 "
                    "$(SHARED_LIBRARY).0 \\"                                   )
  LINEMAK("          && ln -sf $(SHARED_LIBRARY).0 $(SHARED_LIBRARY)"          )
  LINEMAK(""                                                                   )
  LINEMAK("clean:  clean_debug"                                                )
  LINEMAK(""                                                                   )
  LINEMAK("clean_debug:"                                                       )
  LINEMAK("\t@echo '// ----- Make ($(TOOLBOX)): dLabPro class ${CxxClass} "
                   "(${SLName}) -- cleaning DEBUG -----'"                      )
  LINEMAK("\t-rm -f $(OBJECTS) $(LIBRARY)"                                     )
  LINEMAK("\t-rm -f vc80.?db"                                                  )
  LINEMAK("\t-touch -c -r ${DefFile} -d yesterday ${CPPFile}"                  )
  LINEMAK(""                                                                   )
  LINEMAK("clean_release:"                                                     )
  LINEMAK("\t@echo '// ----- Make ($(TOOLBOX)): dLabPro class ${CxxClass} "
                   "(${SLName}) -- cleaning RELEASE -----'"                    )
  LINEMAK("\t-rm -f $(OBJECTS) $(LIBRARY)"                                     )
  LINEMAK("\t-rm -f vc80.?db"                                                  )
  LINEMAK("\t-touch -c -r ${DefFile} -d yesterday ${CPPFile}"                  )
  LINEMAK(""                                                                   )
  LINEMAK("## EOF "                                                            )
}

// -- MS VC++ Project File Templates --
// TODO: These are project files of old versions (5.0, 6.0). Support newer
//       versions or remove!

#define LINENMAK(A) m_nmakFileTmpl.AddItem(A);
#define LINEDSP(A)  m_dspFileTmpl.AddItem(A);
#define LINEDSW(A)  m_dswFileTmpl.AddItem(A);
#define LINEBSI(A)  m_bsiFileTmpl.AddItem(A);

void CGEN_PROTECTED CCgen::CreateDSPReleaseConfig(const char* lpCode)
{
  LINEDSP(""                                                                                       )
  LINEDSP("# PROP BASE Use_MFC 0"                                                                  )
  LINEDSP("# PROP BASE Use_Debug_Libraries 0"                                                      )
  LINEDSP("# PROP BASE Output_Dir \"${OPath}.${Release}intel\""                                    )
  LINEDSP("# PROP BASE Intermediate_Dir \"${OPath}.${Release}intel\""                              )
  LINEDSP("# PROP BASE Target_Dir \"\""                                                            )
  LINEDSP("# PROP Use_MFC 0"                                                                       )
  LINEDSP("# PROP Use_Debug_Libraries 0"                                                           )
  if (m_bMainProject) LINEDSP("# PROP Output_Dir \"${BPath}.${Release}intel\""                     )
  else                LINEDSP("# PROP Output_Dir \"${LPath}.${Release}intel\""                     )
  LINEDSP("# PROP Intermediate_Dir \"${OPath}.${Release}intel\""                                   )
  LINEDSP("# PROP Target_Dir \"\""                                                                 )
  if (m_bMainProject)
  {
    if (dlp_strcmp(lpCode,"_CPP")==0)
    {
      LINEDSP("# ADD BASE CPP /nologo /W3 /GX /O2 /D \"WIN32\" /D \"NDEBUG\" /D \"_WINDOWS\" /YX /c")
      LINEDSP("# ADD CPP /nologo /W3 /GX /O2 ${Include} /D \"WIN32\" /D \"NDEBUG\" /D \"_WINDOWS\" /D \"_RELEASE\" /D \"_DLP_CPP\" /FD /TP /Gy /c")
    }
    else if (dlp_strcmp(lpCode,"_C")==0)
    {
      LINEDSP("# ADD BASE CPP /nologo /W3 /GX /O2 /D \"WIN32\" /D \"NDEBUG\" /D \"_WINDOWS\" /YX /c")
      LINEDSP("# ADD CPP /nologo /W3 /GX /O2 ${Include} /D \"WIN32\" /D \"NDEBUG\" /D \"_WINDOWS\" /D \"_RELEASE\" /D \"_DLP_C\" /FD /TC /Gy /c")
    }
    else
    {
      LINEDSP("# ADD BASE CPP /nologo /W3 /GX /O2 /D \"WIN32\" /D \"NDEBUG\" /D \"_WINDOWS\" /YX /c")
      LINEDSP("# ADD CPP /nologo /W3 /GX /O2 ${Include} /D \"WIN32\" /D \"NDEBUG\" /D \"_WINDOWS\" /D \"_RELEASE\" /FD /Gy /c")
    }
  }
  else
  {
    if (dlp_strcmp(lpCode,"_CPP")==0)
    {
      LINEDSP("# ADD BASE CPP /nologo /W3 /GX /O2 /D \"WIN32\" /D \"NDEBUG\" /D \"_WINDOWS\" /D \"_LIB\" /YX /c")
      LINEDSP("# ADD CPP /nologo /W3 /GX /O2 ${Include} /D \"WIN32\" /D \"NDEBUG\" /D \"_WINDOWS\" /D \"_LIB\" /D \"_RELEASE\" /D \"_DLP_CPP\" /FD /TP /Gy /c ${CFlags}")
    }
    else if (dlp_strcmp(lpCode,"_C")==0)
    {
      LINEDSP("# ADD BASE CPP /nologo /W3 /GX /O2 /D \"WIN32\" /D \"NDEBUG\" /D \"_WINDOWS\" /D \"_LIB\" /YX /c")
      LINEDSP("# ADD CPP /nologo /W3 /GX /O2 ${Include} /D \"WIN32\" /D \"NDEBUG\" /D \"_WINDOWS\" /D \"_LIB\" /D \"_RELEASE\" /D \"_DLP_C\" /FD /TC /Gy /c ${CFlags}")
    }
    else
    {
      LINEDSP("# ADD BASE CPP /nologo /W3 /GX /O2 /D \"WIN32\" /D \"NDEBUG\" /D \"_WINDOWS\" /D \"_LIB\" /YX /c")
      LINEDSP("# ADD CPP /nologo /W3 /GX /O2 ${Include} /D \"WIN32\" /D \"NDEBUG\" /D \"_WINDOWS\" /D \"_LIB\" /D \"_RELEASE\" /FD /Gy /c ${CFlags}")
    }
  }
  if (m_nPlatform==PF_MSDEV6)
  {
    if (m_bMainProject)
    {
      LINEDSP("# ADD BASE MTL /nologo /D \"NDEBUG\" /mktyplib203 /o \"NUL\" /win32"                )
      LINEDSP("# ADD MTL /nologo /D \"NDEBUG\" /mktyplib203 /o \"NUL\" /win32"                     )
      LINEDSP("# ADD BASE RSC /l 0x407 /d \"NDEBUG\""                                              )
      LINEDSP("# ADD RSC /l 0x407 /d \"NDEBUG\""                                                   )
    }
    else
    {
      LINEDSP("# ADD BASE RSC /l 0x407"                                                            )
      LINEDSP("# ADD RSC /l 0x407"                                                                 )
    }
  }
  LINEDSP("BSC32=bscmake.exe"                                                                      )
  LINEDSP("# ADD BASE BSC32 /nologo"                                                               )
  LINEDSP("# ADD BSC32 /nologo"                                                                    )
  if (m_bMainProject)
  {
    LINEDSP("LINK32=link.exe"                                                                      )
    LINEDSP("# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib /nologo /subsystem:windows /machine:I386")
    LINEDSP("# ADD LINK32 kernel32.lib user32.lib gdi32.lib /nologo /subsystem:windows /machine:I386")
  }
  else
  {
    LINEDSP("LIB32=link.exe -lib"                                                                  )
    LINEDSP("# ADD BASE LIB32 /nologo"                                                             )
    LINEDSP("# ADD LIB32 /nologo"                                                                  )
  }
}

void CGEN_PROTECTED CCgen::CreateDSPDebugConfig(const char* lpCode)
{
  LINEDSP("# PROP BASE Use_MFC 0"                                                                  )
  LINEDSP("# PROP BASE Use_Debug_Libraries 1"                                                      )
  LINEDSP("# PROP BASE Output_Dir \"${OPath}.${Debug}intel\""                                      )
  LINEDSP("# PROP BASE Intermediate_Dir \"${OPath}.${Debug}intel\""                                )
  LINEDSP("# PROP BASE Target_Dir \"\""                                                            )
  LINEDSP("# PROP Use_MFC 0"                                                                       )
  LINEDSP("# PROP Use_Debug_Libraries 1"                                                           )
  if (m_bMainProject) LINEDSP("# PROP Output_Dir \"${BPath}.${Debug}intel\""                       )
  else                LINEDSP("# PROP Output_Dir \"${LPath}.${Debug}intel\""                       )
  LINEDSP("# PROP Intermediate_Dir \"${OPath}.${Debug}intel\""                                     )
  if (m_bMainProject) LINEDSP("# PROP Ignore_Export_Lib 0"                                         )
  LINEDSP("# PROP Target_Dir \"\""                                                                 )
  if (m_bMainProject)
  {
    if (dlp_strcmp(lpCode,"_CPP")==0)
    {
      LINEDSP("# ADD BASE CPP /nologo /W3 /Gm /GX /Z7 /Od /D \"WIN32\" /D \"_DEBUG\" /D \"_WINDOWS\" /YX /FD /c")
      LINEDSP("# ADD CPP /nologo /W3 /Gm /GX /Z7 /Od ${Include} /D \"WIN32\" /D \"_DEBUG\" /D \"_WINDOWS\" /D \"_DLP_CPP\" /FD /TP /Gy /c")
    }
    else if (dlp_strcmp(lpCode,"_C")==0)
    {
      LINEDSP("# ADD BASE CPP /nologo /W3 /Gm /GX /Z7 /Od /D \"WIN32\" /D \"_DEBUG\" /D \"_WINDOWS\" /YX /FD /c")
      LINEDSP("# ADD CPP /nologo /W3 /Gm /GX /Z7 /Od ${Include} /D \"WIN32\" /D \"_DEBUG\" /D \"_WINDOWS\" /D \"_DLP_C\" /FD /TC /Gy /c")
    }
    else
    {
      LINEDSP("# ADD BASE CPP /nologo /W3 /Gm /GX /Z7 /Od /D \"WIN32\" /D \"_DEBUG\" /D \"_WINDOWS\" /YX /FD /c")
      LINEDSP("# ADD CPP /nologo /W3 /Gm /GX /Z7 /Od ${Include} /D \"WIN32\" /D \"_DEBUG\" /D \"_WINDOWS\" /FD /c")
    }
  }
  else
  {
    if (dlp_strcmp(lpCode,"_CPP")==0)
    {
      LINEDSP("# ADD BASE CPP /nologo /W3 /GX /Z7 /Od /D \"WIN32\" /D \"_DEBUG\" /D \"_WINDOWS\" /D \"_LIB\" /YX /c")
      LINEDSP("# ADD CPP /nologo /W3 /GX /Z7 /Od /Ob1 ${Include} /D \"WIN32\" /D \"_DEBUG\" /D \"_WINDOWS\" /D \"_LIB\" /D \"_DLP_CPP\" /FD /TP /Gy /c ${CFlags}")
    }
    else if (dlp_strcmp(lpCode,"_C")==0)
    {
      LINEDSP("# ADD BASE CPP /nologo /W3 /GX /Z7 /Od /D \"WIN32\" /D \"_DEBUG\" /D \"_WINDOWS\" /D \"_LIB\" /YX /c")
      LINEDSP("# ADD CPP /nologo /W3 /GX /Z7 /Od /Ob1 ${Include} /D \"WIN32\" /D \"_DEBUG\" /D \"_WINDOWS\" /D \"_LIB\" /D \"_DLP_C\" /FD /TC /Gy /c ${CFlags}")
    }
    else
    {
      LINEDSP("# ADD BASE CPP /nologo /W3 /GX /Z7 /Od /D \"WIN32\" /D \"_DEBUG\" /D \"_WINDOWS\" /D \"_LIB\" /YX /c")
      LINEDSP("# ADD CPP /nologo /W3 /GX /Z7 /Od /Ob1 ${Include} /D \"WIN32\" /D \"_DEBUG\" /D \"_WINDOWS\" /D \"_LIB\" /FD /Gy /c ${CFlags}")
    }
  }
  if (m_nPlatform==PF_MSDEV6)
  {
    if (m_bMainProject)
    {
      LINEDSP("# ADD BASE MTL /nologo /D \"_DEBUG\" /mktyplib203 /o \"NUL\" /win32"                )
      LINEDSP("# ADD MTL /nologo /D \"_DEBUG\" /mktyplib203 /o \"NUL\" /win32"                     )
      LINEDSP("# ADD BASE RSC /l 0x407 /d \"_DEBUG\""                                              )
      LINEDSP("# ADD RSC /l 0x407 /d \"_DEBUG\""                                                   )
    }
    else
    {
      LINEDSP("# ADD BASE RSC /l 0x407"                                                            )
      LINEDSP("# ADD RSC /l 0x407"                                                                 )
    }
  }
  LINEDSP("BSC32=bscmake.exe"                                                                      )
  LINEDSP("# ADD BASE BSC32 /nologo"                                                               )
  LINEDSP("# ADD BSC32 /nologo"                                                                    )
  if (m_bMainProject)
  {
    LINEDSP("LINK32=link.exe"                                                                      )
    LINEDSP("# ADD BASE LINK32 kernel32.lib user32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept")
    LINEDSP("# ADD LINK32 kernel32.lib user32.lib /nologo /profile /debug /machine:I386"           )
  }
  else
  {
    LINEDSP("LIB32=link.exe -lib"                                                                  )
    LINEDSP("# ADD BASE LIB32 /nologo"                                                             )
    LINEDSP("# ADD LIB32 /nologo"                                                                  )
  }
}

void CGEN_PROTECTED CCgen::CreateDSPTemplate()
{
  // MSDEV Project file (5.0/6.0)
  LINEDSP("# Microsoft Developer Studio Project File - Name=\"${ProjL}\" - Package Owner=<4>"      )
  if (m_nPlatform==PF_MSDEV6) LINEDSP("# Microsoft Developer Studio Generated Build File, Format Version 6.00")
  else                        LINEDSP("# Microsoft Developer Studio Generated Build File, Format Version 5.00")
  LINEDSP("# ** DO NOT EDIT **"                                                                    )
  LINEDSP(""                                                                                       )
  if (m_bMainProject) LINEDSP("# TARGTYPE \"Win32 (x86) Application\" 0x0101"                      )
  else                LINEDSP("# TARGTYPE \"Win32 (x86) Static Library\" 0x0104"                   )
  LINEDSP(""                                                                                       )
  LINEDSP("CFG=${ProjL} - Win32 Release"                                                           )
  LINEDSP("!MESSAGE This is not a valid makefile. To build this project using NMAKE,"              )
  LINEDSP("!MESSAGE use the Export Makefile command and run"                                       )
  LINEDSP("!MESSAGE "                                                                              )
  LINEDSP("!MESSAGE NMAKE /f \"${NMAKFile}\"."                                                     )
  LINEDSP("!MESSAGE "                                                                              )
  LINEDSP("!MESSAGE You can specify a configuration when running NMAKE"                            )
  LINEDSP("!MESSAGE by defining the macro CFG on the command line. For example:"                   )
  LINEDSP("!MESSAGE "                                                                              )
  LINEDSP("!MESSAGE NMAKE /f \"${NMAKFile}\" CFG=\"${Proj} - Win32 Release\""                      )
  LINEDSP("!MESSAGE "                                                                              )
  LINEDSP("!MESSAGE Possible choices for configuration are:"                                       )
  LINEDSP("!MESSAGE "                                                                              )
  if (m_bMainProject)
  {
    if (m_bCProject)
    {
      LINEDSP("!MESSAGE \"${ProjL} - Win32 Release_CPP\" (based on \"Win32 (x86) Application\")"   )
      LINEDSP("!MESSAGE \"${ProjL} - Win32 Debug_CPP\" (based on \"Win32 (x86) Application\")"     )
      LINEDSP("!MESSAGE \"${ProjL} - Win32 Release_C\" (based on \"Win32 (x86) Application\")"     )
      LINEDSP("!MESSAGE \"${ProjL} - Win32 Debug_C\" (based on \"Win32 (x86) Application\")"       )
    }
    else
    {
      LINEDSP("!MESSAGE \"${ProjL} - Win32 Release\" (based on \"Win32 (x86) Application\")"       )
      LINEDSP("!MESSAGE \"${ProjL} - Win32 Debug\" (based on \"Win32 (x86) Application\")"         )
    }
  }
  else
  {
    if (m_bCProject)
    {
      LINEDSP("!MESSAGE \"${ProjL} - Win32 Release_CPP\" (based on \"Win32 (x86) Static Library\")")
      LINEDSP("!MESSAGE \"${ProjL} - Win32 Debug_CPP\" (based on \"Win32 (x86) Static Library\")"  )
      LINEDSP("!MESSAGE \"${ProjL} - Win32 Release_C\" (based on \"Win32 (x86) Static Library\")"  )
      LINEDSP("!MESSAGE \"${ProjL} - Win32 Debug_C\" (based on \"Win32 (x86) Static Library\")"    )
    }
    else
    {
      LINEDSP("!MESSAGE \"${ProjL} - Win32 Release\" (based on \"Win32 (x86) Static Library\")"    )
      LINEDSP("!MESSAGE \"${ProjL} - Win32 Debug\" (based on \"Win32 (x86) Static Library\")"      )
    }
  }
  LINEDSP("!MESSAGE "                                                                              )
  LINEDSP(""                                                                                       )
  LINEDSP("# Begin Project"                                                                        )
  if (m_nPlatform==PF_MSDEV6) LINEDSP("#PROP AllowPerConfigDependencies 0"                         )
  LINEDSP("# PROP Scc_ProjName \"\""                                                               )
  LINEDSP("# PROP Scc_LocalPath \"\""                                                              )
  LINEDSP("CPP=cl.exe"                                                                             )
  if (m_nPlatform==PF_MSDEV6)
  {
    if (m_bMainProject) LINEDSP("MTL=midl.exe"                                                     )
    LINEDSP("RSC=rc.exe"                                                                           )
  }

  if (m_bCProject)
  {
    LINEDSP(""                                                                                     )
    LINEDSP("!IF  \"$(CFG)\" == \"${ProjL} - Win32 Release_CPP\""                                  )
    CreateDSPReleaseConfig("_CPP");

    LINEDSP(""                                                                                     )
    LINEDSP("!ELSEIF  \"$(CFG)\" == \"${ProjL} - Win32 Release_C\""                                )
    CreateDSPReleaseConfig("_C");

    LINEDSP(""                                                                                     )
    LINEDSP("!ELSEIF  \"$(CFG)\" == \"${ProjL} - Win32 Debug_CPP\""                                )
    CreateDSPDebugConfig("_CPP");

    LINEDSP(""                                                                                     )
    LINEDSP("!ELSEIF  \"$(CFG)\" == \"${ProjL} - Win32 Debug_C\""                                  )
    CreateDSPDebugConfig("_C");
  }
  else
  {
    LINEDSP(""                                                                                     )
    LINEDSP("!IF  \"$(CFG)\" == \"${ProjL} - Win32 Release\""                                      )
    CreateDSPReleaseConfig("");

    LINEDSP(""                                                                                     )
    LINEDSP("!ELSEIF  \"$(CFG)\" == \"${ProjL} - Win32 Debug\""                                    )
    CreateDSPDebugConfig("");
  }
  LINEDSP(""                                                                                       )
  LINEDSP("!ENDIF "                                                                                )

  LINEDSP(""                                                                                       )
  LINEDSP("# Begin Target"                                                                         )
  LINEDSP(""                                                                                       )
  if (m_bCProject)
  {
    LINEDSP("# Name \"${ProjL} - Win32 Release_CPP\""                                              )
    LINEDSP("# Name \"${ProjL} - Win32 Release_C\""                                                )
    LINEDSP("# Name \"${ProjL} - Win32 Debug_CPP\""                                                )
    LINEDSP("# Name \"${ProjL} - Win32 Debug_C\""                                                  )
  }
  else
  {
    LINEDSP("# Name \"${ProjL} - Win32 Release\""                                                  )
    LINEDSP("# Name \"${ProjL} - Win32 Debug\""                                                    )
  }
  LINEDSP("# Begin Group \"Source Files\""                                                         )
  LINEDSP(""                                                                                       )
  LINEDSP("# PROP Default_Filter \"cpp;c;cxx;rc;def;r;odl;hpj;bat;for;f90\""                       )
  LINEDSP("# Begin Source File"                                                                    )
  LINEDSP(""                                                                                       )
  LINEDSP("SOURCE=.\\${CPPFile}"                                                                   )
  LINEDSP("# End Source File"                                                                      )
  LINEDSP("# End Group"                                                                            )
  if (!m_bMainProject)
  {
    LINEDSP("# Begin Group \"Header Files\""                                                       )
    LINEDSP(""                                                                                     )
    LINEDSP("# PROP Default_Filter \"h;hpp;hxx;hm;inl;fi;fd\""                                     )
    LINEDSP("# Begin Source File"                                                                  )
    LINEDSP(""                                                                                     )
    LINEDSP("SOURCE=${IPath}${/}automatic${/}${HFile}"                                             )
    LINEDSP("# End Source File"                                                                    )
    LINEDSP("# End Group"                                                                          )
  }
  LINEDSP("# End Target"                                                                           )
  LINEDSP("# End Project"                                                                          )
}

void CGEN_PROTECTED CCgen::CreateDSWTemplate()
{
  // MSDEV Project workspace (5.0/6.0)
  if (m_nPlatform==PF_MSDEV6) LINEDSW("Microsoft Developer Studio Workspace File, Format Version 6.00")
  else                        LINEDSW("Microsoft Developer Studio Workspace File, Format Version 5.00")
  LINEDSW("# WARNING: DO NOT EDIT OR DELETE THIS WORKSPACE FILE!"                          )
  LINEDSW(""                                                                               )
  LINEDSW("###############################################################################")
  LINEDSW(""                                                                               )
  LINEDSW("Project: \"${ProjL}\"=.\\${ProjL}.dsp - Package Owner=<4>"                      )
  LINEDSW(""                                                                               )
  LINEDSW("Package=<5>"                                                                    )
  LINEDSW("{{{"                                                                            )
  LINEDSW("}}}"                                                                            )
  LINEDSW(""                                                                               )
  LINEDSW("Package=<4>"                                                                    )
  LINEDSW("{{{"                                                                            )
  LINEDSW("}}}"                                                                            )
  LINEDSW(""                                                                               )
  LINEDSW("###############################################################################")
  LINEDSW(""                                                                               )
  LINEDSW("Global:"                                                                        )
  LINEDSW(""                                                                               )
  LINEDSW("Package=<5>"                                                                    )
  LINEDSW("{{{"                                                                            )
  LINEDSW("}}}"                                                                            )
  LINEDSW(""                                                                               )
  LINEDSW("Package=<3>"                                                                    )
  LINEDSW("{{{"                                                                            )
  LINEDSW("}}}"                                                                            )
  LINEDSW(""                                                                               )
  LINEDSW("###############################################################################")
}

void CGEN_PROTECTED CCgen::CreateNMakeReleaseConfig(const char* lpCode)
{
  if (m_nPlatform==PF_MSDEV4)
  {
    LINENMAK("# PROP BASE Use_MFC 0"                                                       )
    LINENMAK("# PROP BASE Use_Debug_Libraries 0"                                           )
    LINENMAK("# PROP BASE Output_Dir \"${LPath}.${Release}intel\""                         )
    LINENMAK("# PROP BASE Intermediate_Dir \"${OPath}.${Release}intel\""                   )
    LINENMAK("# PROP BASE Target_Dir \"\""                                                 )
    LINENMAK("# PROP Use_MFC 0"                                                            )
    LINENMAK("# PROP Use_Debug_Libraries 0"                                                )
    LINENMAK("# PROP Output_Dir \"${LPath}.${Release}intel\""                              )
    LINENMAK("# PROP Intermediate_Dir \"${OPath}.${Release}intel\""                        )
    LINENMAK("# PROP Target_Dir \"\""                                                      )
  }
  if (m_bMainProject) LINENMAK("OUTDIR=${BPath}.${Release}intel"                           )
  else                LINENMAK("OUTDIR=${LPath}.${Release}intel"                           )
  LINENMAK("INTDIR=.\\${OPath}.${Release}intel"                                            )
  LINENMAK(""                                                                              )
  if (m_bMainProject)
  {
    LINENMAK("ALL : \"$(OUTDIR)\\${ProjL}.exe\""                                           )
    LINENMAK(""                                                                            )
    LINENMAK("CLEAN : "                                                                    )
    LINENMAK("\t-@erase \"$(INTDIR)\\${ProjL}.obj\""                                       )
    if (m_nPlatform==PF_MSDEV6) LINENMAK("\t-@erase \"$(INTDIR)\\vc60.idb\""               )
    else                        LINENMAK("\t-@erase \"$(INTDIR)\\vc50.idb\""               )
    LINENMAK("\t-@erase \"$(OUTDIR)\\${ProjL}.exe\""                                       )
  }
  else
  {
    LINENMAK("ALL : \"$(OUTDIR)\\${ProjL}.lib\""                                           )
    LINENMAK(""                                                                            )
    LINENMAK("CLEAN : "                                                                    )
    LINENMAK("\t-@erase \".\\${OPath}.${Release}intel\\${ProjL}.lib\""                     )
    LINENMAK("\t-@erase \".\\${OPath}.${Release}intel\\${ProjL}.obj\""                     )
  }
  LINENMAK(""                                                                              )
  LINENMAK("\"$(OUTDIR)\" :"                                                               )
  LINENMAK("    if not exist \"$(OUTDIR)/$(NULL)\" mkdir \"$(OUTDIR)\""                    )
  if (m_bMainProject)
  {
    LINENMAK(""                                                                            )
    LINENMAK("\"$(INTDIR)\" :"                                                             )
    LINENMAK("    if not exist \"$(INTDIR)/$(NULL)\" mkdir \"$(INTDIR)\""                  )
  }
  if (m_nPlatform != PF_MSDEV4) LINENMAK("CPP=cl.exe"                                      )
  LINENMAK(""                                                                              )
  if (m_bMainProject)
  {
    if (dlp_strcmp(lpCode,"_CPP")==0)
      LINENMAK("CPP_PROJ=/nologo /ML /W3 /GX /O2 ${Include} /D \"WIN32\" /D \"NDEBUG\" /D \"_WINDOWS\" /D \"_RELEASE\" /D \"_DLP_CPP\" /Fo\"$(INTDIR)\\\\\" /Fd\"$(INTDIR)\\\\\" /FD /TP /Gy /c ")
    else if (dlp_strcmp(lpCode,"_C")==0)
      LINENMAK("CPP_PROJ=/nologo /ML /W3 /GX /O2 ${Include} /D \"WIN32\" /D \"NDEBUG\" /D \"_WINDOWS\" /D \"_RELEASE\" /D \"_DLP_C\" /Fo\"$(INTDIR)\\\\\" /Fd\"$(INTDIR)\\\\\" /FD /TC /Gy /c ")
    else
      LINENMAK("CPP_PROJ=/nologo /ML /W3 /GX /O2 ${Include} /D \"WIN32\" /D \"NDEBUG\" /D \"_WINDOWS\" /D \"_RELEASE\" /Fo\"$(INTDIR)\\\\\" /Fd\"$(INTDIR)\\\\\" /FD /Gy /c ")

    LINENMAK(""                                                                            )
    LINENMAK(".c{$(INTDIR)}.obj::"                                                         )
    LINENMAK("   $(CPP) @<<"                                                               )
    LINENMAK("   $(CPP_PROJ) $< "                                                          )
    LINENMAK("<<"                                                                          )
    LINENMAK(""                                                                            )
    LINENMAK(".cpp{$(INTDIR)}.obj::"                                                       )
    LINENMAK("   $(CPP) @<<"                                                               )
    LINENMAK("   $(CPP_PROJ) $< "                                                          )
    LINENMAK("<<"                                                                          )
    LINENMAK(""                                                                            )
    LINENMAK(".cxx{$(INTDIR)}.obj::"                                                       )
    LINENMAK("   $(CPP) @<<"                                                               )
    LINENMAK("   $(CPP_PROJ) $< "                                                          )
    LINENMAK("<<"                                                                          )
    LINENMAK(""                                                                            )
    LINENMAK(".c{$(INTDIR)}.sbr::"                                                         )
    LINENMAK("   $(CPP) @<<"                                                               )
    LINENMAK("   $(CPP_PROJ) $< "                                                          )
    LINENMAK("<<"                                                                          )
    LINENMAK(""                                                                            )
    LINENMAK(".cpp{$(INTDIR)}.sbr::"                                                       )
    LINENMAK("   $(CPP) @<<"                                                               )
    LINENMAK("   $(CPP_PROJ) $< "                                                          )
    LINENMAK("<<"                                                                          )
    LINENMAK(""                                                                            )
    LINENMAK(".cxx{$(INTDIR)}.sbr::"                                                       )
    LINENMAK("   $(CPP) @<<"                                                               )
    LINENMAK("   $(CPP_PROJ) $< "                                                          )
    LINENMAK("<<"                                                                          )
    LINENMAK(""                                                                            )
    LINENMAK("MTL=midl.exe"                                                                )
    LINENMAK("MTL_PROJ=/nologo /D \"NDEBUG\" /mktyplib203 /o \"NUL\" /win32 "              )
    LINENMAK("RSC=rc.exe"                                                                  )
  }
  else
  {
    if (dlp_strcmp(lpCode,"_CPP")==0)
    {
      if (m_nPlatform==PF_MSDEV4)
    {
      LINENMAK("# ADD BASE CPP /nologo /W3 /GX /O2 /D \"WIN32\" /D \"NDEBUG\" /D \"_WINDOWS\" /YX /c")
      LINENMAK("# ADD CPP /nologo /W3 /GX /O2 /Ob1 ${Include} /D \"WIN32\" /D \"NDEBUG\" /D \"_WINDOWS\" /D \"_RELEASE\" /D \"_DLP_CPP\" /TP /c")
    }
    LINENMAK("CPP_PROJ=/nologo /ML /W3 /GX /O2 /Ob1 ${Include} /D \"WIN32\" /D \"NDEBUG\" /D \"_WINDOWS\" /D \"_RELEASE\" /Fo\"$(INTDIR)/\" /D \"_DLP_CPP\" /TP /c ")
    }
    else if (dlp_strcmp(lpCode,"_C")==0)
    {
      if (m_nPlatform==PF_MSDEV4)
    {
      LINENMAK("# ADD BASE CPP /nologo /W3 /GX /O2 /D \"WIN32\" /D \"NDEBUG\" /D \"_WINDOWS\" /YX /c")
      LINENMAK("# ADD CPP /nologo /W3 /GX /O2 /Ob1 ${Include} /D \"WIN32\" /D \"NDEBUG\" /D \"_WINDOWS\" /D \"_RELEASE\" /D \"_DLP_C\" /TC /c")
    }
    LINENMAK("CPP_PROJ=/nologo /ML /W3 /GX /O2 /Ob1 ${Include} /D \"WIN32\" /D \"NDEBUG\" /D \"_WINDOWS\" /D \"_RELEASE\" /Fo\"$(INTDIR)/\" /D \"_DLP_C\" /TC /c ")
    }
    else
    {
      if (m_nPlatform==PF_MSDEV4)
    {
      LINENMAK("# ADD BASE CPP /nologo /W3 /GX /O2 /D \"WIN32\" /D \"NDEBUG\" /D \"_WINDOWS\" /YX /c")
      LINENMAK("# ADD CPP /nologo /W3 /GX /O2 /Ob1 ${Include} /D \"WIN32\" /D \"NDEBUG\" /D \"_WINDOWS\" /D \"_RELEASE\" /c")
    }
    LINENMAK("CPP_PROJ=/nologo /ML /W3 /GX /O2 /Ob1 ${Include} /D \"WIN32\" /D \"NDEBUG\" /D \"_WINDOWS\" /D \"_RELEASE\" /Fo\"$(INTDIR)/\" /c ")
    }
    LINENMAK("CPP_OBJS=.\\${OPath}.${Release}intel/"                                       )
    LINENMAK("CPP_SBRS="                                                                   )
  }
  LINENMAK("BSC32=bscmake.exe"                                                             )
  if (m_nPlatform==PF_MSDEV4)
  {
    LINENMAK("# ADD BASE BSC32 /nologo"                                                    )
    LINENMAK("# ADD BSC32 /nologo"                                                         )
  }
  LINENMAK("BSC32_FLAGS=/nologo /o\"$(OUTDIR)\\${ProjL}.bsc\" "                            )
  LINENMAK("BSC32_SBRS= \\"                                                                )
  LINENMAK("\t"                                                                            )
  if (m_bMainProject)
  {
    LINENMAK("LINK32=link.exe"                                                             )
    LINENMAK("LINK32_FLAGS=kernel32.lib user32.lib gdi32 /nologo /subsystem:windows /incremental:no /pdb:\"$(OUTDIR)\\slc.pdb\" /machine:I386 /out:\"$(OUTDIR)\\slc.exe\" ")
    LINENMAK("LINK32_OBJS= \\"                                                             )
    LINENMAK("\t\"$(INTDIR)\\${ProjL}.obj\""                                               )
    LINENMAK(""                                                                            )
    LINENMAK("\"$(OUTDIR)\\${ProjL}.exe\" : \"$(OUTDIR)\" $(DEF_FILE) $(LINK32_OBJS)"      )
    LINENMAK("    $(LINK32) @<<"                                                           )
    LINENMAK("  $(LINK32_FLAGS) $(LINK32_OBJS)"                                            )
  }
  else
  {
    LINENMAK("LIB32=link.exe -lib"                                                         )
    LINENMAK("# ADD BASE LIB32 /nologo"                                                    )
    LINENMAK("# ADD LIB32 /nologo"                                                         )
    LINENMAK("LIB32_FLAGS=/nologo /out:\"$(OUTDIR)/${ProjL}.lib\" "                        )
    LINENMAK("LIB32_OBJS=\"$(INTDIR)/${ProjL}.obj\""                                       )
    LINENMAK(""                                                                            )
    LINENMAK("\"$(OUTDIR)\\${ProjL}.lib\" : \"$(OUTDIR)\" $(DEF_FILE) $(LIB32_OBJS)"       )
    LINENMAK("    $(LIB32) @<<"                                                            )
    LINENMAK("  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)"                                 )
  }
  LINENMAK("<<"                                                                            )
  LINENMAK(""                                                                              )
}

void CGEN_PROTECTED CCgen::CreateNMakeDebugConfig(const char* lpCode)
{
  if (m_nPlatform==PF_MSDEV4)
  {
    LINENMAK("# PROP BASE Use_MFC 0"                                                       )
    LINENMAK("# PROP BASE Use_Debug_Libraries 1"                                           )
    LINENMAK("# PROP BASE Output_Dir \"${LPath}.${Debug}intel\""                           )
    LINENMAK("# PROP BASE Intermediate_Dir \"${OPath}.${Debug}intel\""                     )
    LINENMAK("# PROP BASE Target_Dir \"\""                                                 )
    LINENMAK("# PROP Use_MFC 0"                                                            )
    LINENMAK("# PROP Use_Debug_Libraries 1"                                                )
    LINENMAK("# PROP Output_Dir \"${LPath}.${Debug}intel\""                                )
    LINENMAK("# PROP Intermediate_Dir \"${OPath}.${Debug}intel\""                          )
    LINENMAK("# PROP Target_Dir \"\""                                                      )
  }
  if (m_bMainProject) LINENMAK("OUTDIR=${BPath}.${Debug}intel"                             )
  else                LINENMAK("OUTDIR=${LPath}.${Debug}intel"                             )
  LINENMAK("INTDIR=.\\${OPath}.${Debug}intel"                                              )
  LINENMAK(""                                                                              )
  if (m_bMainProject)
  {
    LINENMAK("ALL : \"$(OUTDIR)\\${ProjL}.exe\""                                           )
    LINENMAK(""                                                                            )
    LINENMAK("CLEAN : "                                                                    )
    LINENMAK("\t-@erase \"$(INTDIR)\\${ProjL}.obj\""                                       )
    if (m_nPlatform==PF_MSDEV6)
    {
      LINENMAK("\t-@erase \"$(INTDIR)\\vc60.idb\""                                         )
      LINENMAK("\t-@erase \"$(INTDIR)\\vc60.pdb\""                                         )
    }
    else
    {
      LINENMAK("\t-@erase \"$(INTDIR)\\vc50.idb\""                                         )
      LINENMAK("\t-@erase \"$(INTDIR)\\vc50.pdb\""                                         )
    }
    LINENMAK("\t-@erase \"$(OUTDIR)\\${ProjL}.exe\""                                       )
  }
  else
  {
    LINENMAK("ALL : \"$(OUTDIR)\\${ProjL}.lib\""                                           )
    LINENMAK(""                                                                            )
    LINENMAK("CLEAN : "                                                                    )
    LINENMAK("\t-@erase \".\\${OPath}.${Debug}intel\\${ProjL}.lib\""                       )
    LINENMAK("\t-@erase \".\\${OPath}.${Debug}intel\\${ProjL}.obj\""                       )
  }
  LINENMAK(""                                                                              )
  LINENMAK("\"$(OUTDIR)\" :"                                                               )
  LINENMAK("    if not exist \"$(OUTDIR)/$(NULL)\" mkdir \"$(OUTDIR)\""                    )
  if (m_bMainProject)
  {
    LINENMAK(""                                                                            )
    LINENMAK("\"$(INTDIR)\" :"                                                             )
    LINENMAK("    if not exist \"$(INTDIR)/$(NULL)\" mkdir \"$(INTDIR)\""                  )
  }
  if (m_bMainProject)
  {
    LINENMAK("CPP=cl.exe"                                                                  )
    if (dlp_strcmp(lpCode,"_CPP")==0)
      LINENMAK("CPP_PROJ=/nologo /MLd /W3 /Gm /GX /Z7 /Od ${Include} /D \"WIN32\" /D \"_DEBUG\" /D \"_WINDOWS\" /D \"_DLP_CPP\" /Fo\"$(INTDIR)\\\\\" /Fd\"$(INTDIR)\\\\\" /FD /TP /c ")
    else if (dlp_strcmp(lpCode,"_C")==0)
      LINENMAK("CPP_PROJ=/nologo /MLd /W3 /Gm /GX /Z7 /Od ${Include} /D \"WIN32\" /D \"_DEBUG\" /D \"_WINDOWS\" /D \"_DLP_C\" /Fo\"$(INTDIR)\\\\\" /Fd\"$(INTDIR)\\\\\" /FD /TC /c ")
    else
      LINENMAK("CPP_PROJ=/nologo /MLd /W3 /Gm /GX /Z7 /Od ${Include} /D \"WIN32\" /D \"_DEBUG\" /D \"_WINDOWS\" /Fo\"$(INTDIR)\\\\\" /Fd\"$(INTDIR)\\\\\" /FD /c ")

    LINENMAK(""                                                                            )
    LINENMAK(".c{$(INTDIR)}.obj::"                                                         )
    LINENMAK("   $(CPP) @<<"                                                               )
    LINENMAK("   $(CPP_PROJ) $< "                                                          )
    LINENMAK("<<"                                                                          )
    LINENMAK(""                                                                            )
    LINENMAK(".cpp{$(INTDIR)}.obj::"                                                       )
    LINENMAK("   $(CPP) @<<"                                                               )
    LINENMAK("   $(CPP_PROJ) $< "                                                          )
    LINENMAK("<<"                                                                          )
    LINENMAK(""                                                                            )
    LINENMAK(".cxx{$(INTDIR)}.obj::"                                                       )
    LINENMAK("   $(CPP) @<<"                                                               )
    LINENMAK("   $(CPP_PROJ) $< "                                                          )
    LINENMAK("<<"                                                                          )
    LINENMAK(""                                                                            )
    LINENMAK(".c{$(INTDIR)}.sbr::"                                                         )
    LINENMAK("   $(CPP) @<<"                                                               )
    LINENMAK("   $(CPP_PROJ) $< "                                                          )
    LINENMAK("<<"                                                                          )
    LINENMAK(""                                                                            )
    LINENMAK(".cpp{$(INTDIR)}.sbr::"                                                       )
    LINENMAK("   $(CPP) @<<"                                                               )
    LINENMAK("   $(CPP_PROJ) $< "                                                          )
    LINENMAK("<<"                                                                          )
    LINENMAK(""                                                                            )
    LINENMAK(".cxx{$(INTDIR)}.sbr::"                                                       )
    LINENMAK("   $(CPP) @<<"                                                               )
    LINENMAK("   $(CPP_PROJ) $< "                                                          )
    LINENMAK("<<"                                                                          )
    LINENMAK("MTL=midl.exe"                                                                )
    LINENMAK("MTL_PROJ=/nologo /D \"_DEBUG\" /mktyplib203 /o \"NUL\" /win32 "              )
    LINENMAK("RSC=rc.exe"                                                                  )
  }
  else
  {
    if (dlp_strcmp(lpCode,"_CPP")==0)
    {
      if (m_nPlatform==PF_MSDEV4)
    {
      LINENMAK("# ADD BASE CPP /nologo /W3 /GX /Z7 /Od /D \"WIN32\" /D \"_DEBUG\" /D \"_WINDOWS\" /c"           )
      LINENMAK("# ADD CPP /nologo /W3 /GX /Z7 /Od /Ob1 ${Include} /D \"WIN32\" /D \"_DEBUG\" /D \"_WINDOWS\" /D \"_DLP_CPP\" /TP /c")
    }
      LINENMAK("CPP_PROJ=/nologo /MLd /W3 /GX /Z7 /Od /Ob1 ${Include} /D \"WIN32\" /D \"_DEBUG\" /D \"_WINDOWS\" /Fo\"$(INTDIR)/\" /D \"_DLP_CPP\" /TP /c ")
    }
    else if (dlp_strcmp(lpCode,"_C")==0)
    {
      if (m_nPlatform==PF_MSDEV4)
    {
      LINENMAK("# ADD BASE CPP /nologo /W3 /GX /Z7 /Od /D \"WIN32\" /D \"_DEBUG\" /D \"_WINDOWS\" /c"           )
      LINENMAK("# ADD CPP /nologo /W3 /GX /Z7 /Od /Ob1 ${Include} /D \"WIN32\" /D \"_DEBUG\" /D \"_WINDOWS\" /D \"_DLP_C\" /TC /c")
    }
      LINENMAK("CPP_PROJ=/nologo /MLd /W3 /GX /Z7 /Od /Ob1 ${Include} /D \"WIN32\" /D \"_DEBUG\" /D \"_WINDOWS\" /Fo\"$(INTDIR)/\" /D \"_DLP_C\" /TC /c ")
    }
    else
    {
      if (m_nPlatform==PF_MSDEV4)
    {
      LINENMAK("# ADD BASE CPP /nologo /W3 /GX /Z7 /Od /D \"WIN32\" /D \"_DEBUG\" /D \"_WINDOWS\" /c"           )
      LINENMAK("# ADD CPP /nologo /W3 /GX /Z7 /Od /Ob1 ${Include} /D \"WIN32\" /D \"_DEBUG\" /D \"_WINDOWS\" /c")
    }
      LINENMAK("CPP_PROJ=/nologo /MLd /W3 /GX /Z7 /Od /Ob1 ${Include} /D \"WIN32\" /D \"_DEBUG\" /D \"_WINDOWS\" /Fo\"$(INTDIR)/\" /c ")
    }
    LINENMAK("CPP_OBJS=.\\${OPath}.${Debug}intel/"                                         )
    LINENMAK("CPP_SBRS="                                                                   )
  }
  LINENMAK("BSC32=bscmake.exe"                                                             )
  if (m_nPlatform==PF_MSDEV4)
  {
    LINENMAK("# ADD BASE BSC32 /nologo"                                                    )
    LINENMAK("# ADD BSC32 /nologo"                                                         )
  }
  LINENMAK("BSC32_FLAGS=/nologo /o\"$(OUTDIR)\\${ProjL}.bsc\" "                            )
  LINENMAK("BSC32_SBRS= \\"                                                                )
  LINENMAK("\t"                                                                            )
  if (m_bMainProject)
  {
    LINENMAK("LINK32=link.exe"                                                             )
    LINENMAK("LINK32_FLAGS=kernel32.lib user32.lib /nologo /profile /debug /machine:I386 /out:\"$(OUTDIR)\\${ProjL}.exe\" ")
    LINENMAK("LINK32_OBJS= \\"                                                             )
    LINENMAK("\t\"$(INTDIR)\\${ProjL}.obj\""                                               )
    LINENMAK(""                                                                            )
    LINENMAK("\"$(OUTDIR)\\${ProjL}.exe\" : \"$(OUTDIR)\" $(DEF_FILE) $(LINK32_OBJS)"      )
    LINENMAK("    $(LINK32) @<<"                                                           )
    LINENMAK("  $(LINK32_FLAGS) $(LINK32_OBJS)"                                            )
  }
  else
  {
    LINENMAK("LIB32=link.exe -lib"                                                         )
    LINENMAK("# ADD BASE LIB32 /nologo"                                                    )
    LINENMAK("# ADD LIB32 /nologo"                                                         )
    LINENMAK("LIB32_FLAGS=/nologo /out:\"$(OUTDIR)/${ProjL}.lib\" "                        )
    LINENMAK("LIB32_OBJS=\"$(INTDIR)/${ProjL}.obj\""                                       )
    LINENMAK(""                                                                            )
    LINENMAK("\"$(OUTDIR)\\${ProjL}.lib\" : \"$(OUTDIR)\" $(DEF_FILE) $(LIB32_OBJS)"       )
    LINENMAK("    $(LIB32) @<<"                                                            )
    LINENMAK("  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)"                                 )
  }
  LINENMAK("<<"                                                                            )
  LINENMAK(""                                                                              )
}

INT16 CGEN_PROTECTED CCgen::CreateNMakeTemplate()
{
  if (m_nPlatform == PF_MSDEV4)
  {
    LINENMAK("# Microsoft Developer Studio Generated NMAKE File, Format Version 4.00"        )
    LINENMAK("# ** DO NOT EDIT **"                                                           )
    LINENMAK(""                                                                              )
    if (m_bMainProject) LINENMAK("# TARGTYPE \"Win32 (x86) Application\" 0x0101"             )
    else                LINENMAK("# TARGTYPE \"Win32 (x86) Static Library\" 0x0104"          )
  }
  else LINENMAK("# Microsoft Developer Studio Generated NMAKE File, Based on ${ProjL}.dsp"     )
  LINENMAK(""                                                                                  )
  LINENMAK("!IF \"$(CFG)\" == \"\""                                                            )
  LINENMAK("CFG=${ProjL} - Win32 Debug"                                                        )
  LINENMAK("!MESSAGE No configuration specified.  Defaulting to ${ProjL} - Win32 Debug."       )
  LINENMAK("!ENDIF "                                                                           )
  LINENMAK(""                                                                                  )
  LINENMAK("!IF \"$(CFG)\" != \"${ProjL} - Win32 Release\" && \"$(CFG)\" !=\"${ProjL} - Win32 Debug\"")
//  LINENMAK("!IF \"$(CFG)\" != \"${ProjL} - Win32 Release\" && \"$(CFG)\" !=\\"                 )
//  LINENMAK(" \"${ProjL} - Win32 Debug\""                                                       )
  LINENMAK("!MESSAGE Invalid configuration \"$(CFG)\" specified."                              )
  LINENMAK("!MESSAGE You can specify a configuration when running NMAKE on this makefile"      )
  LINENMAK("!MESSAGE by defining the macro CFG on the command line.  For example:"             )
  LINENMAK("!MESSAGE "                                                                         )
  LINENMAK("!MESSAGE NMAKE /f \"${NMAKFile}\" CFG=\"${Proj} - Win32 Debug\""                   )
  LINENMAK("!MESSAGE "                                                                         )
  LINENMAK("!MESSAGE Possible choices for configuration are:"                                  )
  LINENMAK("!MESSAGE "                                                                         )
  if (m_bMainProject)
  {
    if (m_bCProject)
    {
      LINENMAK("!MESSAGE \"${ProjL} - Win32 Release_CPP\" (based on \"Win32 (x86) Application\")")
      LINENMAK("!MESSAGE \"${ProjL} - Win32 Release_C\" (based on \"Win32 (x86) Application\")"  )
      LINENMAK("!MESSAGE \"${ProjL} - Win32 Debug_CPP\" (based on \"Win32 (x86) Application\")"  )
      LINENMAK("!MESSAGE \"${ProjL} - Win32 Debug_C\" (based on \"Win32 (x86) Application\")"    )
    }
    else
    {
      LINENMAK("!MESSAGE \"${ProjL} - Win32 Release\" (based on \"Win32 (x86) Application\")"    )
      LINENMAK("!MESSAGE \"${ProjL} - Win32 Debug\" (based on \"Win32 (x86) Application\")"      )
    }
  }
  else
  {
    if (m_bCProject)
    {
      LINENMAK("!MESSAGE \"${ProjL} - Win32 Release_CPP\" (based on \"Win32 (x86) Static Library\")")
      LINENMAK("!MESSAGE \"${ProjL} - Win32 Release_C\" (based on \"Win32 (x86) Static Library\")"  )
      LINENMAK("!MESSAGE \"${ProjL} - Win32 Debug_CPP\" (based on \"Win32 (x86) Static Library\")"  )
      LINENMAK("!MESSAGE \"${ProjL} - Win32 Debug_C\" (based on \"Win32 (x86) Static Library\")"    )
    }
    else
    {
      LINENMAK("!MESSAGE \"${ProjL} - Win32 Release\" (based on \"Win32 (x86) Static Library\")")
      LINENMAK("!MESSAGE \"${ProjL} - Win32 Debug\" (based on \"Win32 (x86) Static Library\")"  )
    }
  }
  LINENMAK("!MESSAGE "                                                                         )
  LINENMAK("!ERROR An invalid configuration is specified."                                     )
  LINENMAK("!ENDIF "                                                                           )
  LINENMAK(""                                                                                  )
  LINENMAK("!IF \"$(OS)\" == \"Windows_NT\""                                                   )
  LINENMAK("NULL="                                                                             )
  LINENMAK("!ELSE "                                                                            )
  LINENMAK("NULL=nul"                                                                          )
  LINENMAK("!ENDIF "                                                                           )
  if (m_nPlatform==PF_MSDEV4)
  {
    LINENMAK("################################################################################")
    LINENMAK("# Begin Project"                                                                 )
    LINENMAK("CPP=cl.exe"                                                                      )
  }

  if (m_bCProject)
  {
    LINENMAK(""                                                                                )
    LINENMAK("!IF  \"$(CFG)\" == \"${ProjL} - Win32 Release_CPP\""                             )
    CreateNMakeReleaseConfig("_CPP");

    LINENMAK(""                                                                                )
    LINENMAK("!ELSEIF  \"$(CFG)\" == \"${ProjL} - Win32 Release_C\""                           )
    CreateNMakeReleaseConfig("_C");

    LINENMAK(""                                                                                )
    LINENMAK("!ELSEIF  \"$(CFG)\" == \"${ProjL} - Win32 Debug_CPP\""                           )
    CreateNMakeDebugConfig("_CPP");

    LINENMAK(""                                                                                )
    LINENMAK("!ELSEIF  \"$(CFG)\" == \"${ProjL} - Win32 Debug_C\""                             )
    CreateNMakeDebugConfig("_C");
  }
  else
  {
    LINENMAK(""                                                                                )
    LINENMAK("!IF  \"$(CFG)\" == \"${ProjL} - Win32 Release\""                                 )
    CreateNMakeReleaseConfig("");

    LINENMAK(""                                                                                )
    LINENMAK("!ELSEIF  \"$(CFG)\" == \"${ProjL} - Win32 Debug\""                               )
    CreateNMakeDebugConfig("");
  }
  LINENMAK("!ENDIF "                                                                           )
  LINENMAK(""                                                                                  )

  if (m_bMainProject)
  {
    LINENMAK(""                                                                                )
    LINENMAK("!IF \"$(NO_EXTERNAL_DEPS)\" != \"1\""                                            )
    LINENMAK("!IF EXISTS(\"${ProjL}.dep\")"                                                    )
    LINENMAK("!INCLUDE \"${ProjL}.dep\""                                                       )
    LINENMAK("!ELSE "                                                                          )
    LINENMAK("!MESSAGE Warning: cannot find \"${ProjL}.dep\""                                  )
    LINENMAK("!ENDIF "                                                                         )
    LINENMAK("!ENDIF "                                                                         )
    LINENMAK(""                                                                                )
    LINENMAK(""                                                                                )
    if (m_bCProject)
    {
      LINENMAK("!IF \"$(CFG)\" == \"${ProjL} - Win32 Release_CPP\" ||  \"$(CFG)\" == \"${ProjL} - Win32 Release_C\" ||\"$(CFG)\" == \"${ProjL} - Win32 Debug_CPP\" ||  \"$(CFG)\" == \"${ProjL} - Win32 Debug_C\"")
      LINENMAK("SOURCE=.\\${ProjL}.c"                                                          )
    }
    else
    {
      LINENMAK("!IF \"$(CFG)\" == \"${ProjL} - Win32 Release\" || \"$(CFG)\" == \"${ProjL} - Win32 Debug\"")
      LINENMAK("SOURCE=.\\${ProjL}.cpp"                                                        )
    }
    LINENMAK(""                                                                                )
    LINENMAK("\"$(INTDIR)\\${ProjL}.obj\" : $(SOURCE) \"$(INTDIR)\""                           )
    LINENMAK(""                                                                                )
    LINENMAK("!ENDIF "                                                                         )
  }
  else
  {
    LINENMAK(".c{$(CPP_OBJS)}.obj:"                                                            )
    LINENMAK("   $(CPP) $(CPP_PROJ) $<  "                                                      )
    LINENMAK(""                                                                                )
    LINENMAK(".cpp{$(CPP_OBJS)}.obj:"                                                          )
    LINENMAK("   $(CPP) $(CPP_PROJ) $<  "                                                      )
    LINENMAK(""                                                                                )
    LINENMAK(".cxx{$(CPP_OBJS)}.obj:"                                                          )
    LINENMAK("   $(CPP) $(CPP_PROJ) $<  "                                                      )
    LINENMAK(""                                                                                )
    LINENMAK(".c{$(CPP_SBRS)}.sbr:"                                                            )
    LINENMAK("   $(CPP) $(CPP_PROJ) $<  "                                                      )
    LINENMAK(""                                                                                )
    LINENMAK(".cpp{$(CPP_SBRS)}.sbr:"                                                          )
    LINENMAK("   $(CPP) $(CPP_PROJ) $<  "                                                      )
    LINENMAK(""                                                                                )
    LINENMAK(".cxx{$(CPP_SBRS)}.sbr:"                                                          )
    LINENMAK("   $(CPP) $(CPP_PROJ) $<  "                                                      )
    LINENMAK(""                                                                                )
    LINENMAK("################################################################################")
    LINENMAK("# Begin Target"                                                                  )
    LINENMAK(""                                                                                )
    if (m_bCProject)
    {
      LINENMAK("# Name \"${ProjL} - Win32 Release_CPP\""                                       )
      LINENMAK("# Name \"${ProjL} - Win32 Release_C\""                                         )
      LINENMAK("# Name \"${ProjL} - Win32 Debug_CPP\""                                         )
      LINENMAK("# Name \"${ProjL} - Win32 Debug_C\""                                           )
      LINENMAK(""                                                                              )
      LINENMAK("!IF  \"$(CFG)\" == \"${ProjL} - Win32 Release_CPP\""                           )
      LINENMAK(""                                                                              )
      LINENMAK("!ELSEIF  \"$(CFG)\" == \"${ProjL} - Win32 Release_C\""                         )
      LINENMAK(""                                                                              )
      LINENMAK("!ELSEIF  \"$(CFG)\" == \"${ProjL} - Win32 Debug_CPP\""                         )
      LINENMAK(""                                                                              )
      LINENMAK("!ELSEIF  \"$(CFG)\" == \"${ProjL} - Win32 Debug_C\""                           )
    }
    else
    {
      LINENMAK("# Name \"${ProjL} - Win32 Release\""                                           )
      LINENMAK("# Name \"${ProjL} - Win32 Debug\""                                             )
      LINENMAK(""                                                                              )
      LINENMAK("!IF  \"$(CFG)\" == \"${ProjL} - Win32 Release\""                               )
      LINENMAK(""                                                                              )
      LINENMAK("!ELSEIF  \"$(CFG)\" == \"${ProjL} - Win32 Debug\""                             )
    }
    LINENMAK(""                                                                                )
    LINENMAK("!ENDIF "                                                                         )
    LINENMAK(""                                                                                )
    LINENMAK("DEPENDENCIES=\"${IPath}${/}automatic${/}${HFile}\""                              )
    LINENMAK(""                                                                                )
    LINENMAK("################################################################################")
    LINENMAK("# Begin Source File"                                                             )
    LINENMAK(""                                                                                )
    LINENMAK("SOURCE=.\\${CPPFile}"                                                            )
    LINENMAK(""                                                                                )
    LINENMAK("\"$(INTDIR)\\${ProjL}.obj\" : $(SOURCE) $(DEPENDENCIES) \"$(INTDIR)\""           )
    LINENMAK(""                                                                                )
    LINENMAK(""                                                                                )
    LINENMAK("# End Source File"                                                               )
    LINENMAK("# End Target"                                                                    )
    LINENMAK("# End Project"                                                                   )
    LINENMAK("################################################################################")
  }

/*
  // -- Windows batch script to call nmake (Win32 Debug[_CPP] configuration) --
  LINEGEN("@ECHO OFF"                                                                          )
  LINEGEN(""                                                                                   )
  LINEGEN("PATH \"${MSDPath}\\BIN\""                                                           )
  LINEGEN("${callvcvars}"                                                                      )
  LINEGEN("if not exist .\\${OPath}.${Debug}intel mkdir .\\${OPath}.${Debug}intel"             )
  if (m_bCProject) LINEGEN("nmake /NOLOGO /S /f \"${NMAKFile}\" CFG=\"${ProjL} - Win32 Debug_CPP\"")
  else             LINEGEN("nmake /NOLOGO /S /f \"${NMAKFile}\" CFG=\"${ProjL} - Win32 Debug\"")
*/
  return O_K;
}

// EOF
