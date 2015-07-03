// dLabPro SDK class CCgen (cgen)
// - Scanning and generating HTML manual pages
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

#define LINEHTM(A) htmFileTmpl.AddItem(A);
#define ICON_RNT    10
#define ICON_FLD    28
#define ICON_FLD_P  35
#define ICON_FLD_R  36
#define ICON_FLD_I  39
#define ICON_FLD_IP 40
#define ICON_FLD_IR 41
#define ICON_OPT    29
#define ICON_OPT_I  42
#define ICON_MTH    27
#define ICON_MTH_P  32
#define ICON_MTH_R  33
#define ICON_MTH_I  38
#define ICON_ERR    30
#define ICON_ERR_W  31
#define ICON_ERR_I  43
#define ICON_ERR_IW 44

INT16 CGEN_PROTECTED CCgen::ScanHtmlManual()
{
  CList<SCGStr>* lplCman = NULL;
  CList<SCGStr>  lXman;
  FILE*          f       = NULL;
  char*          tx      = NULL;
  INT32          nInCman = -1;         /* No. of scanned custom HTML lines (-1: outside) */
  INT32          nLine   = 0;
  char           sBuf  [L_PATH];
  char           sManfn[L_PATH];
  char           sLine[L_INPUTLINE];
  char           sScan[L_INPUTLINE];

  if (GetVerboseLevel()>1) printf("\nScanning HTML manual...");

  dlp_convert_name(CN_MANFILE,sManfn,m_lpsProject);
  if(chdir(m_lpsHomePath) != 0) return IERROR(this,ERR_CHDIR,m_lpsHomePath,0,0);
  dlp_chdir(m_lpsMPath,TRUE); 
  if(getcwd(sBuf,L_PATH)==NULL) return IERROR(this,ERR_GETCWD,0,0,0);
  printf("\n%s %s%c%s",GetVerboseLevel()>1?" ":"s -",sBuf,C_DIR,sManfn);
  if ((f = fopen(sManfn,"r"))!=NULL)
  {
    while (fgets(sLine,L_INPUTLINE,f))
    {
      nLine++;
      dlp_strcpy(sScan,sLine);
      dlp_strtrimleft(dlp_strtrimright(sScan));
      if (strstr(sScan,"<div id=\"")==sScan)
      {
        memmove(sScan,&sScan[9],dlp_strlen(sScan)-8);
        tx = &sScan[dlp_strlen(sScan)-1];
        while (tx>sScan && (*tx=='>'||*tx=='\"')) *tx--='\0';
        if (strcmp(sScan,"cls")==0)
        {
          lplCman=&m_cmanual;
          IFCHECKEX(3) printf("\n    (%4ld): 0x%08X class",nLine,lplCman);
        }
        else if (strstr(sScan,"err_")==sScan)
          for (SCGErr* lpErr=m_errors.m_items; lpErr; lpErr=lpErr->next)
          {
            HtmlGetDivId(sBuf,lpErr,CR_ERROR);
            if (strcmp(sBuf,sScan)==0)
            {
              lplCman=&lpErr->lCman;
              IFCHECKEX(3) printf("\n    (%4ld): 0x%08X error %s",nLine,lplCman,lpErr->lpName);
              break;
            }
          }
        else if (strstr(sScan,"opt_")==sScan)
          for (SCGOpt* lpOpt=m_opts.m_items; lpOpt; lpOpt=lpOpt->next)
          {
            HtmlGetDivId(sBuf,lpOpt,CR_OPTION);
            if (strcmp(sBuf,sScan)==0)
            {
              lplCman=&lpOpt->lCman;
              IFCHECKEX(3) printf("\n    (%4ld): 0x%08X option %s",nLine,lplCman,lpOpt->lpName);
              break;
            }
          }
        else if (strstr(sScan,"fld_")==sScan)
          for (SCGFld* lpFld=m_flds.m_items; lpFld; lpFld=lpFld->next)
          {
            HtmlGetDivId(sBuf,lpFld,CR_FIELD);
            if (strcmp(sBuf,sScan)==0)
            {
              lplCman=&lpFld->lCman;
              IFCHECKEX(3) printf("\n    (%4ld): 0x%08X field %s",nLine,lplCman,lpFld->lpName);
              break;
            }
          }
        else if (strstr(sScan,"mth_")==sScan)
          for (SCGMth* lpMth=m_mths.m_items; lpMth; lpMth=lpMth->next)
          {
            HtmlGetDivId(sBuf,lpMth,CR_METHOD);
            if (strcmp(sBuf,sScan)==0)
            {
              lplCman=&lpMth->lCman;
              IFCHECKEX(3) printf("\n    (%4ld): 0x%08X method %s",nLine,lplCman,lpMth->lpName);
              break;
            }
          }
        else if (strstr(sScan,"rnt_")==sScan)
          for (SCGRnt* lpRnt=m_rnts.m_items; lpRnt; lpRnt=lpRnt->next)
          {
            HtmlGetDivId(sBuf,lpRnt,CR_NOTE);
            if (strcmp(sBuf,sScan)==0)
            {
              lplCman=&lpRnt->lCman;
              IFCHECKEX(3) printf("\n    (%4ld): 0x%08X release note %s",nLine,lplCman,lpRnt->lpName);
              break;
            }
          }
      }
      else if (strcmp(sScan,"<!--{{ CUSTOM_DOC -->")==0)
      {
        IFCHECKEX(3) printf(" [%ld...",(long)nLine+1);
        if (!lplCman) lplCman=&lXman;
        nInCman=0;
      }
      else if (strcmp(sScan,"<!--}} CUSTOM_DOC -->")==0)
      {
        IFCHECKEX(3) printf("%ld]",(long)nLine);
        lplCman=NULL;
        nInCman=-1;
      }
      else if ((nInCman>=0) && lplCman)
      {
        if (lplCman==&lXman && nInCman==0)
        {
          sprintf(sBuf,"--- %s(%ld) ---------------------------------------",sManfn,(long)(nLine+1));
          lXman.AddItem(sBuf);
        }
        lplCman->AddItem(dlp_strtrimright(sLine));
        nInCman++;
      }
    }
    if (nInCman>=0) ERRORMSG(ERR_UNEXPECTEDEOF,"<!--}} CUSTOM_DOC -->",0,0);
    fclose(f);

    if (lXman.Count())
    {
      sprintf(sBuf,"%s.extra",sManfn);
      ERRORMSG(ERR_EXTRA_HTML,sBuf,0,0);
      IFCHECKEX(3)
      {
        printf("\n\n  This is the extra custom HTML code:\n");
        lXman.PrintList();
        printf("  -- END\n");
      }
      if ((f=fopen(sBuf,"a+")))
      {
        for (SCGStr* page=lXman.m_items; page; page=page->next)
        {
          fputs(page->lpName,f);
          fputs("\n",f);
        }
        fclose(f);
      }
      else ERRORMSG(ERR_FILEOPEN,sBuf,"appending",0);
    }
  }
  else printf(" not found");

  return O_K;
}

INT16 CGEN_PROTECTED CCgen::CreateHtmlManual()
{
  char lpLine[2048] = "";

  // Not needed for a main project
  if (m_bMainProject) return O_K;

  if (GetVerboseLevel()>1) printf("\nCreating manual files...");
  char lpManfn[255]; dlp_convert_name(CN_MANFILE,lpManfn,m_lpsProject);

  // Create site map
  char lpSme[255]; sprintf(lpSme,"%s %s %s [",lpManfn,m_lpsClass,m_lpsCName);
  m_clSiteMap.AddItem(lpSme);

  // Create HTML page
  CList<SCGStr> htmFileTmpl;
  LINEHTM("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0 Transitional//EN\">");
  if(m_lpsCName[0]){
    if(IsSdkResource(m_lpsHomePath)){
      LINEHTM("<!-- dLabPro SDK class ${CxxClass} (${SLName})");
    }else{
      LINEHTM("<!-- dLabPro class ${CxxClass} (${SLName})");
    }
  }else{
    LINEHTM("<!-- ${Comment}");
  }
  LINEHTM("<    - Manual file");
  LINEHTM("<   ");
  LINEHTM("<    AUTHOR : ${Author}");
  LINEHTM("<    PACKAGE: ${Package}");
  LINEHTM("<   ");
  LINEHTM("<    This file was generated by dcg. DO NOT MODIFY! Modify ${DefFile} instead.");
  LINEHTM("<    ");
  LINEHTM("<    Copyright 2013 dLabPro contributors and others (see COPYRIGHT file) ");
  LINEHTM("<    - Chair of System Theory and Speech Technology, TU Dresden");
  LINEHTM("<    - Chair of Communications Engineering, BTU Cottbus");
  LINEHTM("<    ");
  LINEHTM("<    This file is part of dLabPro.");
  LINEHTM("<    ");
  LINEHTM("<    dLabPro is free software: you can redistribute it and/or modify it under the");
  LINEHTM("<    terms of the GNU Lesser General Public License as published by the Free");
  LINEHTM("<    Software Foundation, either version 3 of the License, or (at your option)");
  LINEHTM("<    any later version.");
  LINEHTM("<    ");
  LINEHTM("<    dLabPro is distributed in the hope that it will be useful, but WITHOUT ANY");
  LINEHTM("<    WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS");
  LINEHTM("<    FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more");
  LINEHTM("<    details.");
  LINEHTM("<    ");
  LINEHTM("<    You should have received a copy of the GNU Lesser General Public License");
  LINEHTM("<    along with dLabPro. If not, see <http://www.gnu.org/licenses/>.");
  LINEHTM("-->");
  LINEHTM("<html>"                                                            );
  LINEHTM("<head>"                                                            );
  LINEHTM("  <meta http-equiv=\"Content-Type\" content=\"text/html; charset=iso-8859-1\">");
  LINEHTM("  <meta http-equiv=\"Content-Style-Type\" content=\"text/css\">"   );
  LINEHTM("  <meta name=\"description\" content=\"dLabPro documentation - ${PType} ${SLName}\">");
  LINEHTM("  <meta name=\"author\" content=\"${Author}\">"                    );
  LINEHTM("  <meta name=\"generator\" content=\"dLabPro/cGen\">"              );
  LINEHTM("  <title>${PType} ${SLName}</title>"                               );
  LINEHTM("  <link rel=stylesheet type=\"text/css\" href=\"../default.css\">" );
  LINEHTM("</head>"                                                           );
  LINEHTM("<script type=\"text/javascript\">"                                 );
  LINEHTM("  if (top==self)"                                                  );
  LINEHTM("  {"                                                               );
  sprintf(lpLine,"    var sLocation = \"../index.html?automatic/%s\";",lpManfn);
  LINEHTM(lpLine                                                              );
  LINEHTM("    if (location.hash.length>0)"                                   );
  LINEHTM("      sLocation += \";\"+location.hash.substr(1);"                 );
  LINEHTM("    top.location = sLocation;"                                     );
  LINEHTM("  }"                                                               );
  LINEHTM("</script>"                                                         );
  LINEHTM("<script type=\"text/javascript\" src=\"../default.js\"></script>"  );
  LINEHTM("<body>"                                                            );
  LINEHTM("<!--"                                                              );
  LINEHTM("{{ BEGIN_SITEMAP"                                                  );
  LINEHTM("}} END_SITEMAP"                                                    );
  LINEHTM("//{{ BEGIN_TOC"                                                    );
  LINEHTM("//}} END_TOC"                                                      );
  LINEHTM("-->"                                                               );
  LINEHTM(""                                                                  );
  HtmlMember(htmFileTmpl,NULL,CR_PROJECT);
  LINEHTM(""                                                                  );
  HtmlAllMembers(htmFileTmpl,CR_NOTE  );
  HtmlAllMembers(htmFileTmpl,CR_FIELD );
  HtmlAllMembers(htmFileTmpl,CR_OPTION);
  HtmlAllMembers(htmFileTmpl,CR_METHOD);
  HtmlAllMembers(htmFileTmpl,CR_CFUNC );
  HtmlAllMembers(htmFileTmpl,CR_ERROR );
  LINEHTM(""                                                                  );
  LINEHTM("<div class=\"footer\">End of page</div>"                           );
  LINEHTM("</body>"                                                           );
  LINEHTM("</html>"                                                           );

  // Finish and write HTML page
  m_clSiteMap.AddItem("]");
  RealizeStrList(htmFileTmpl,m_clSiteMap,"{{ BEGIN_SITEMAP","","");
  RealizeStrList(htmFileTmpl,m_clToc    ,"//{{ BEGIN_TOC"  ,"","");

  JdSee(&htmFileTmpl);
  JdIndex(&htmFileTmpl);
  ReplaceAllKeys(htmFileTmpl);
  WriteBackTemplate(htmFileTmpl,m_lpsMPath,lpManfn);

  return O_K;
}

void CGEN_PROTECTED CCgen::HtmlMemberOverview(CList<SCGStr>& htmFileTmpl, INT16 nMemberType)
{
  char lpLine[2048]      = "";
  char lpId[255]         = "";
  char lpDivId[255]      = "";
  char lpSignature[1024] = "";
  char lpComment[1024]   = "";
  char lpManfn[255]      = "";
  char lpsPManfn[255]    = "";
  char lpsInherited[255] = "";
  INT32 nCtr              = 0;

  dlp_convert_name(CN_MANFILE,lpManfn,m_lpsProject);
  dlp_convert_name(CN_MANFILE,lpsPManfn,m_lpsParentProject);
  sprintf(lpsInherited,"[<a href=\"%s\" class=\"code\">inherited</a>]",lpsPManfn);

  switch (nMemberType)
  {
    case CR_ERROR: 
    {
      if (m_bClib) break;
      nCtr = 0;
      LINEHTM("    <tr>"                                                     );
      LINEHTM("      <td class=\"rowgroup\"><a name=\"err\">Errors</a></td>" );
      LINEHTM("      <td class=\"rowgroup\" style=\"text-align:right\">"     );
      HtmlNaviBar(htmFileTmpl);
      LINEHTM("      </td>"                                                  );
      LINEHTM("    </tr>"                                                    );
      if (!m_errors.IsListEmpty())
      {
        sprintf(lpLine,"  %s#err Errors - [",lpManfn);
        m_clSiteMap.AddItem(lpLine);
        sprintf(lpLine,"        nErr = top.TC1.InsertItem(\"Errors\",\"\",8,9,"
          "0,\"automatic/%s#err\",\"CONT\",nCls);",lpManfn);
        m_clToc.AddItem(lpLine);
        SCGErr* page = NULL;
        for (page=m_errors.m_items; page; page=page->next)
        {
          if (page->bInherited) continue;
          nCtr++;
          HtmlStrCnvt(lpId     ,page->lpName   );
          HtmlStrCnvt(lpComment,page->lpComment);
          sprintf(lpLine,"    <tr><td><a href=\"%s#%s\"><code class=\"link\">"
            "%s%04hd</code></a></td><td>%s %s</td></tr>",
            page->bInherited?lpsPManfn:"",HtmlGetDivId(lpDivId,page,CR_ERROR),
            m_lpsClass,page->nId+1001,page->bInherited?lpsInherited:"",
            lpComment);
          LINEHTM(lpLine);

          // Sitemap entry
          sprintf(lpLine,"    %s#%s \"%s%04hd\" \"%s\"",
            page->bInherited?lpsPManfn:lpManfn,
            HtmlGetDivId(lpDivId,page,CR_ERROR),m_lpsClass,page->nId+1001,
            page->lpName);
          m_clSiteMap.AddItem(lpLine);

          // TOC entry
          INT16 nIcon = page->bInherited ? ICON_ERR_I : ICON_ERR;
          if (strstr(page->lpErrorLevel,"EL_WARNING"))
            nIcon = page->bInherited ? ICON_ERR_IW : ICON_ERR_W;
          dlp_strreplace(lpComment,"\"","��");
          dlp_strreplace(lpComment,"\'","`");
          sprintf(lpLine,"        top.TC1.InsertItem(\"%s%04hd\",\"%s %s\",%d,"
            "%d,0,\"automatic/%s#%s\",\"CONT\",nErr);",m_lpsClass,
            page->nId+1001,page->lpName,lpComment,nIcon,nIcon,
            page->bInherited?lpsPManfn:lpManfn,
            HtmlGetDivId(lpDivId,page,CR_ERROR));
          m_clToc.AddItem(lpLine);
        }
        m_clSiteMap.AddItem("  ]");
      }
      if (nCtr==0)
      {
        LINEHTM("    <tr><td colspan=\"2\">This class does not have errors."
          "</td></tr>");
      }
      if (dlp_strlen(m_lpsParentProject))
      {
        sprintf(lpLine,"<tr><td colspan=\"2\">[<a href=\"%s#err\" "
          "class=\"code\">Inherited errors</a>]</td></tr>",lpsPManfn);
        LINEHTM(lpLine);
        
      }
      LINEHTM("    <tr><td colspan=\"2\" class=\"rowempty\">&nbsp;</td></tr>");

      break;
    }
    case CR_FIELD:
    {
      if (m_bClib) break;

      // Check for non-hidden fields
      BOOL    bHasFields = FALSE;
      SCGFld* page       = NULL;
      for (page=m_flds.m_items; page; page=page->next)
        if (m_bXDoc || (page->nFlags&FF_HIDDEN)==0) { bHasFields=TRUE; break; }
      nCtr = 0;

      // Write table
      LINEHTM("    <tr>"                                                     );
      LINEHTM("      <td class=\"rowgroup\"><a name=\"fld\">Fields</a></td>" );
      LINEHTM("      <td class=\"rowgroup\" style=\"text-align:right\">"     );
      HtmlNaviBar(htmFileTmpl);
      LINEHTM("      </td>"                                                  );
      LINEHTM("    </tr>"                                                    );
      if (bHasFields)
      {
        sprintf(lpLine,"  %s#fld Fields - [",lpManfn);
        m_clSiteMap.AddItem(lpLine);
        for (page=m_flds.m_items; page; page=page->next)
        {
          if (m_bXDoc || (page->nFlags & FF_HIDDEN)==0)
          {
            if (page->bInherited) continue;
            nCtr++;
            HtmlStrCnvt(lpId     ,page->lpName   );
            HtmlStrCnvt(lpComment,page->lpComment);
            sprintf(lpLine,"    <tr><td><a href=\"%s#%s\"><code class=\"link\">"
              "%s</code></a></td><td>%s %s</td></tr>",
              page->bInherited?lpsPManfn:"",HtmlGetDivId(lpDivId,page,CR_FIELD),
              lpId,page->bInherited?lpsInherited:"",lpComment);
            LINEHTM(lpLine);

            // Sitemap entry
            sprintf(lpLine,"    %s#%s \"%s\" \"%s\"",
              page->bInherited?lpsPManfn:lpManfn,
              HtmlGetDivId(lpDivId,page,CR_FIELD),page->lpName,page->lpCName);
            m_clSiteMap.AddItem(lpLine);

            // TOC entry
            INT16 nIcon = page->bInherited ? ICON_FLD_I : ICON_FLD;
            if (page->nFlags & FF_HIDDEN)
              nIcon = page->bInherited ? ICON_FLD_IR : ICON_FLD_R;
            else if (page->nFlags & FF_NOSET)
              nIcon = page->bInherited ? ICON_FLD_IP : ICON_FLD_P;
            dlp_strreplace(lpComment,"\"","��");
            dlp_strreplace(lpComment,"\'","`");
            sprintf(lpLine,"        top.TC1.InsertItem(\"%s\",\"%s    %s\",%d,"
              "%d,0,\"automatic/%s#%s\",\"CONT\",nCls);",lpId,page->lpType,
              lpComment,nIcon,nIcon,page->bInherited?lpsPManfn:lpManfn,
              HtmlGetDivId(lpDivId,page,CR_FIELD));
            m_clToc.AddItem(lpLine);
          }
        }
        m_clSiteMap.AddItem("  ]");
      }
      if (nCtr==0)
      {
        LINEHTM("    <tr><td colspan=\"2\">This class does not have visible "
          "fields.</td></tr>");
      }
      if (dlp_strlen(m_lpsParentProject))
      {
        sprintf(lpLine,"<tr><td colspan=\"2\">[<a href=\"%s#fld\" "
          "class=\"code\">Inherited fields</a>]</td></tr>",lpsPManfn);
        LINEHTM(lpLine);
        
      }
      LINEHTM("    <tr><td colspan=\"2\" class=\"rowempty\">&nbsp;</td></tr>");
      
      break;
    }
    case CR_OPTION: 
    {
      if (m_bClib) break;
      nCtr = 0;

      LINEHTM("    <tr>"                                                     );
      LINEHTM("      <td class=\"rowgroup\"><a name=\"opt\">Options</a></td>");
      LINEHTM("      <td class=\"rowgroup\" style=\"text-align:right\">"     );
      HtmlNaviBar(htmFileTmpl);
      LINEHTM("      </td>"                                                  );
      LINEHTM("    </tr>"                                                    );
      if (!m_opts.IsListEmpty())
      {
        sprintf(lpLine,"  %s#opt Options - [",lpManfn);
        m_clSiteMap.AddItem(lpLine);
        SCGOpt* page = NULL;
        for (page=m_opts.m_items; page; page=page->next)
        {
          if (page->bInherited) continue;
          nCtr++;
          HtmlStrCnvt(lpId     ,page->lpName   );
          HtmlStrCnvt(lpComment,page->lpComment);
          sprintf(lpLine,"    <tr><td><a href=\"%s#%s\"><code class=\"link\">"
            "%s</code></a></td><td>%s %s</td></tr>",
            page->bInherited?lpsPManfn:"",HtmlGetDivId(lpDivId,page,CR_OPTION),
            lpId,page->bInherited?lpsInherited:"",lpComment);
          LINEHTM(lpLine);
          
          // Sitemap entry
          sprintf(lpLine,"    %s#%s \"%s\" \"%s\"",
            page->bInherited?lpsPManfn:lpManfn,
            HtmlGetDivId(lpDivId,page,CR_OPTION),page->lpName,page->lpCName);
          m_clSiteMap.AddItem(lpLine);

          // TOC entry
          dlp_strreplace(lpComment,"\"","��");
          dlp_strreplace(lpComment,"\'","`");
          sprintf(lpLine,"        top.TC1.InsertItem(\"%s\",\"%s\",%d,%d,0,"
            "\"automatic/%s#%s\",\"CONT\",nCls);",lpId,lpComment,
            page->bInherited ? ICON_OPT_I : ICON_OPT,
            page->bInherited ? ICON_OPT_I : ICON_OPT,
            page->bInherited?lpsPManfn:lpManfn,
            HtmlGetDivId(lpDivId,page,CR_OPTION));
          m_clToc.AddItem(lpLine);
        }
        m_clSiteMap.AddItem("  ]");
      }
      if (nCtr==0)
      {
        LINEHTM("    <tr><td colspan=\"2\">This class does not have options."
          "</td></tr>");
      }
      if (dlp_strlen(m_lpsParentProject))
      {
        sprintf(lpLine,"<tr><td colspan=\"2\">[<a href=\"%s#opt\" "
          "class=\"code\">Inherited oprtions</a>]</td></tr>",lpsPManfn);
        LINEHTM(lpLine);
        
      }
      LINEHTM("    <tr><td colspan=\"2\" class=\"rowempty\">&nbsp;</td></tr>");

      break;
    }
    case CR_METHOD:
    {
      if (m_bClib) break;
      nCtr = 0;

      LINEHTM("    <tr>"                                                     );
      LINEHTM("      <td class=\"rowgroup\"><a name=\"mth\">Methods</a></td>");
      LINEHTM("      <td class=\"rowgroup\" style=\"text-align:right\">"     );
      HtmlNaviBar(htmFileTmpl);
      LINEHTM("      </td>"                                                  );
      LINEHTM("    </tr>"                                                    );
      if (!m_mths.IsListEmpty())
      {
        sprintf(lpLine,"  %s#mth Methods - [",lpManfn);
        m_clSiteMap.AddItem(lpLine);
        SCGMth* page = NULL;
        for (page=m_mths.m_items; page; page=page->next)
        {
          if (page->bInherited) continue;
          nCtr++;
          HtmlStrCnvt(lpId     ,page->lpName   );
          HtmlStrCnvt(lpComment,page->lpComment);
          sprintf(lpLine,"    <tr><td><a href=\"%s#%s\"><code class=\"link\">%s"
            "</code></a></td><td><code>%s</code><br>%s %s</td></tr>",
            page->bInherited?lpsPManfn:"",HtmlGetDivId(lpDivId,page,CR_METHOD),
            lpId,HtmlGetUpnSignature(lpSignature,page,CR_METHOD),
            page->bInherited?lpsInherited:"",lpComment);
          LINEHTM(lpLine);
          
          // Sitemap entry
          sprintf
          (
            lpLine,"    %s#%s \"%s\" \"%s\"",
            page->bInherited?lpsPManfn:lpManfn,
            HtmlGetDivId(lpDivId,page,CR_METHOD),page->lpName,page->lpName
          );
          m_clSiteMap.AddItem(lpLine);
          
          // TOC entry
          dlp_strreplace(lpComment,"\"","��");
          dlp_strreplace(lpComment,"\'","`");
          sprintf(lpLine,"        top.TC1.InsertItem(\"%s\",\"%s    %s\",%d,%d,"
            "0,\"automatic/%s#%s\",\"CONT\",nCls);",lpId,
            dlp_strconvert(SC_STRIPHTML,lpSignature,lpSignature),lpComment,
            page->bInherited?ICON_MTH_I:ICON_MTH,
            page->bInherited?ICON_MTH_I:ICON_MTH,
            page->bInherited?lpsPManfn:lpManfn,
            HtmlGetDivId(lpDivId,page,CR_METHOD));
          m_clToc.AddItem(lpLine);
        }
        m_clSiteMap.AddItem("  ]");
      }
      if (nCtr==0)
      {
        LINEHTM("    <tr><td colspan=\"2\">This class does not have methods."
          "</td></tr>");
      }
      if (dlp_strlen(m_lpsParentProject))
      {
        sprintf(lpLine,"<tr><td colspan=\"2\">[<a href=\"%s#mth\" "
          "class=\"code\">Inherited methods</a>]</td></tr>",lpsPManfn);
        LINEHTM(lpLine);
        
      }
      LINEHTM("    <tr><td colspan=\"2\" class=\"rowempty\">&nbsp;</td></tr>");

      break;
    }
    case CR_CFUNC:
    {
      LINEHTM("    <tr>"                                                     );
      LINEHTM("      <td class=\"rowgroup\"><a name=\"cfn\">C/C++ API</a></td>");
      LINEHTM("      <td class=\"rowgroup\" style=\"text-align:right\">"     );
      HtmlNaviBar(htmFileTmpl);
      LINEHTM("      </td>"                                                  );
      LINEHTM("    </tr>"                                                    );
      if (!m_mths.IsListEmpty() || dlp_strlen(m_lpsDlcParent))
      {
        LINEHTM("<tr><td class=\"rowgroup\" colspan=\"2\" style="
          "\"font-weight:normal\"> (see ");
        if (!m_mths.IsListEmpty())
        {
          LINEHTM("<a href=\"#mth\"><u>method list</u></a>");
          if (dlp_strlen(m_lpsDlcParent)) LINEHTM(" and ");
        }
        if (dlp_strlen(m_lpsDlcParent))
        {
          sprintf(lpLine,"<a href=\"%s#cfn\"><u>base class</u></a>",lpsPManfn);
          LINEHTM(lpLine);
        }
        LINEHTM(" for additional C/C++ functions)");
        LINEHTM("</td></tr>");
      }
      if (!m_cfns.IsListEmpty())
      {
        sprintf(lpLine,"  %s#cfn C/C++ API - [",lpManfn);
        m_clSiteMap.AddItem(lpLine);
        if (!m_bClib)
        {
          sprintf(lpLine,"        nCfn = top.TC1.InsertItem(\"C/C++ API\",\"\","
            "8,9,0,\"automatic/%s#cfn\",\"CONT\",nCls);",lpManfn);
          m_clToc.AddItem(lpLine);
        }
        nCtr = 0;
        SCGMth* page = NULL;
        for (page=m_cfns.m_items; page; page=page->next)
        {
          if (page->bInherited) continue;
          nCtr++;
          HtmlStrCnvt(lpId     ,page->lpName   );
          HtmlStrCnvt(lpComment,page->lpComment);
          sprintf(lpLine,"    <tr><td><a href=\"%s#%s\"><code class=\"link\">"
            "%s</code></a></td><td><code>%s</code><br>%s %s</td></tr>",
            page->bInherited?lpsPManfn:"",HtmlGetDivId(lpDivId,page,CR_CFUNC),
            lpId,HtmlGetCSignature(lpSignature,page,CR_CFUNC),
            page->bInherited?lpsInherited:"",lpComment);
          LINEHTM(lpLine);

          // Sitemap entry
          sprintf(lpLine,"    %s#%s \"%s\" \"%s\"",
            page->bInherited?lpsPManfn:lpManfn,
            HtmlGetDivId(lpDivId,page,CR_CFUNC),page->lpName,page->lpCName);
          m_clSiteMap.AddItem(lpLine);

          // TOC entry
          INT16 nIcon = ICON_MTH;
          if (strstr(lpSignature,"protected")) nIcon = ICON_MTH_P;
          if (strstr(lpSignature,"private"  )) nIcon = ICON_MTH_R;
          char* lpSCSig = strtok(lpSignature,";");
          dlp_strreplace(lpComment,"\"","��");
          dlp_strreplace(lpComment,"\'","`");
          sprintf(lpLine,"        top.TC1.InsertItem(\"%s\",\"%s    %s\",%d,%d,"
            "0,\"automatic/%s#%s\",\"CONT\",%s);",lpId,
            dlp_strconvert(SC_STRIPHTML,lpSCSig,lpSCSig),lpComment,nIcon,nIcon,
            page->bInherited?lpsPManfn:lpManfn,
            HtmlGetDivId(lpDivId,page,CR_CFUNC),m_bClib?"nCls":"nCfn");
          m_clToc.AddItem(lpLine);
        }
        m_clSiteMap.AddItem("  ]");
      }
      if (nCtr==0)
      {
        LINEHTM("    <tr><td colspan=\"2\">This class does not have an "
          "additional C/C++ API.</td></tr>");
      }
      LINEHTM("    <tr><td colspan=\"2\" class=\"rowempty\">&nbsp;</td></tr>");

      break;
    }
    case CR_NOTE: 
    {
      if (!m_rnts.IsListEmpty())
      {
        sprintf(lpLine,"  %s#rnt \"Release Notes\" - [",lpManfn);
        m_clSiteMap.AddItem(lpLine);
        sprintf(lpLine,"        nRnt = top.TC1.InsertItem(\"Release Notes\","
          "\"\",13,13,0,\"automatic/%s#rnt\",\"CONT\",nCls);",lpManfn);
        m_clToc.AddItem(lpLine);
        SCGRnt* page = NULL;
        for (page=m_rnts.m_items; page; page=page->next)
        {
          if (page->bInherited && !m_bXDoc) continue;
          nCtr++;
          HtmlStrCnvt(lpId,page->lpName);
          sprintf(lpLine,"    <tr><td class=\"hidden\">&nbsp;&nbsp;&nbsp;</td>"
            "<td class=\"hidden\"><a href=\"#%s\">%s</a></td></tr>",
            HtmlGetDivId(lpDivId,page,CR_NOTE),lpId);
          LINEHTM(lpLine);
          // Sitemap entry
          sprintf(lpLine,"    %s#%s \"%s\" \"%s\"",lpManfn,
            HtmlGetDivId(lpDivId,page,CR_NOTE),page->lpName,page->lpName);
          m_clSiteMap.AddItem(lpLine);
          dlp_strreplace(lpId     ,"\"","��");
          dlp_strreplace(lpId     ,"\'","`");
          dlp_strreplace(lpComment,"\"","��");
          dlp_strreplace(lpComment,"\'","`");
          sprintf(lpLine,"        top.TC1.InsertItem(\"%s\",\"%s\",%d,%d,0,"
            "\"automatic/%s#%s\",\"CONT\",nRnt);",lpId,lpComment,ICON_RNT,
            ICON_RNT,lpManfn,HtmlGetDivId(lpDivId,page,CR_NOTE));
          m_clToc.AddItem(lpLine);
        }
        m_clSiteMap.AddItem("  ]");
      }

      break;
    }
    default: DLPASSERT(FALSE); // Do not call this function for non-members
  }
}

void CGEN_PROTECTED CCgen::HtmlAllMembers(CList<SCGStr>& htmFileTmpl, INT16 nMemberType)
{
  switch (nMemberType)
  {
    case CR_ERROR:
    {
      for (SCGErr* page=m_errors.m_items; page; page=page->next)
        if (!page->bInherited)
          HtmlMember(htmFileTmpl,page,CR_ERROR);
      break;
    }
    case CR_FIELD:
    {
      for (SCGFld* page=m_flds.m_items; page; page=page->next)
        if (m_bXDoc || (page->nFlags & FF_HIDDEN)==0)
          if (!page->bInherited)
            HtmlMember(htmFileTmpl,page,CR_FIELD);
      break;
    }
    case CR_OPTION:
    {
      for (SCGOpt* page=m_opts.m_items; page; page=page->next)
        if (!page->bInherited)
          HtmlMember(htmFileTmpl,page,CR_OPTION);
      break;
    }
    case CR_METHOD:
    {
      for (SCGMth* page=m_mths.m_items; page; page=page->next)
        if (!page->bInherited)
          HtmlMember(htmFileTmpl,page,CR_METHOD);
      break;
    }
    case CR_CFUNC:
    {
      for (SCGMth* page=m_cfns.m_items; page; page=page->next)
        if (!page->bInherited)
          HtmlMember(htmFileTmpl,page,CR_CFUNC);
      break;
    }
    case CR_NOTE:
    {
      for (SCGRnt* page=m_rnts.m_items; page; page=page->next)
        if (!page->bInherited)
          HtmlMember(htmFileTmpl,page,CR_NOTE);
      break;
    }
    default: DLPASSERT(FALSE); // Do not call this function for non-members
  }
}

void CGEN_PROTECTED CCgen::HtmlMember(CList<SCGStr>& htmFileTmpl, void* lpSCGMember, INT16 nMemberType)
{
  SCGStr* lpFLine = htmFileTmpl.FindLastItem();
  char    lpBuf[2048]            = "";
  char    lpMemberType[24]       = "";
  char    lpMemberId[64]         = "";
  char    lpModuleType[24]       = "";
  char    lpModuleId[64]         = "";
  char    lpCmpRemarks[128]      = "";
  char    lpsSrcFileAndLine[512] = "-";

  // Begin member section
  LINEHTM("  <div id=\"${HTML_DivId}\">"                                      );

  // Member navigation bar  
  LINEHTM("  <table width=\"100%\" cellpadding=\"0\" cellspacing=\"0\" "
          "border=\"0\">"                                                     );
  LINEHTM("    <tr>"                                                          );
  LINEHTM("      <td nowrap width=\"25%\" class=\"navbar\"><a "               );
  LINEHTM("        name=\"${HTML_DivId}\"></a>${HTML_MemberType} <span "      );
  LINEHTM("        class=\"mid\">${HTML_MemberId}</span></td>"                );
  LINEHTM("      <td nowrap width=\"75%\" class=\"navbar\" style=\"text-align:"
          "right;\">"                                                         );
  LINEHTM("        <a class=\"navbar\" href=\"javascript:__PrintSection("
          "\'${HTML_DivId}\',\'${HTML_ModuleType} ${HTML_ModuleId}\');\"><img "
          "src=\"../resources/print.gif\" width=\"16\" height=\"16\" border="
          "\"0\" style=\"vertical-align:middle\" alt=\"Print\"></a>"          );
  HtmlNaviBar(htmFileTmpl);
  /*
  LINEHTM("        <a class=\"navbar\" href=\"../home.html\">Home</a>"        );
  LINEHTM("        <a class=\"navbar\" href=\"#cls\">Top</a>"                 );
  if (!m_bClib)
  {
    LINEHTM("        <a class=\"navbar\" href=\"#err\">Errors</a>"            );
    LINEHTM("        <a class=\"navbar\" href=\"#fld\">Fields</a>"            );
    LINEHTM("        <a class=\"navbar\" href=\"#opt\">Options</a>"           );
    LINEHTM("        <a class=\"navbar\" href=\"#mth\">Methods</a>"           );
  }
  LINEHTM("        <a class=\"navbar\" href=\"#cfn\">C/C++</a>"               );
*/
  LINEHTM("      </td>"                                                       );
  LINEHTM("    </tr>"                                                         );
  LINEHTM("  </table>"                                                        );

  // Synopsis table
  switch (nMemberType)
  {
    case CR_PROJECT:
    {
      LINEHTM("  <div class=\"mframe\"><table cellpadding=\"0\" cellspacing="
              "\"0\" border=\"0\">"                                           );
      if (!m_bClib)
      {
        LINEHTM("    <tr>"                                                    );
        LINEHTM("      <td class=\"hidden\">dLabPro</td>"                     );
        LINEHTM("      <td class=\"hidden\">&nbsp;&nbsp;&nbsp;</td>"          );
        LINEHTM("      <td class=\"hidden\">Identifier</td>"                  );
        LINEHTM("      <td class=\"hidden\">:&nbsp;&nbsp;&nbsp;</td>"         );
        LINEHTM("      <td class=\"hidden\"><code><b>${SLName}</b></code></td>");
        LINEHTM("    </tr>"                                                   );
        LINEHTM("    <tr>"                                                    );
        LINEHTM("      <td class=\"hidden\"> </td>"                           );
        LINEHTM("      <td class=\"hidden\">&nbsp;&nbsp;&nbsp;</td>"          );
        LINEHTM("      <td class=\"hidden\">Base class</td>"                  );
        LINEHTM("      <td class=\"hidden\">:&nbsp;&nbsp;&nbsp;</td>"         );
        LINEHTM("      <td class=\"hidden\"><code>${SLBaseName}</code></td>"  );
        LINEHTM("    </tr>"                                                   );
        LINEHTM("    <tr>"                                                    );
        LINEHTM("      <td class=\"hidden\"> </td>"                           );
        LINEHTM("      <td class=\"hidden\">&nbsp;&nbsp;&nbsp;</td>"          );
        LINEHTM("      <td class=\"hidden\">Properties</td>"                  );
        LINEHTM("      <td class=\"hidden\">:&nbsp;&nbsp;&nbsp;</td>"         );
        LINEHTM("      <td class=\"hidden\">${CLStyle}</td>"                  );
        LINEHTM("    </tr>"                                                   );
        LINEHTM("    <tr>"                                                    );
        LINEHTM("      <td class=\"hidden\"> </td>"                           );
        LINEHTM("      <td class=\"hidden\">&nbsp;&nbsp;&nbsp;</td>"          );
        LINEHTM("      <td class=\"hidden\">Default Instance</td>"            );
        LINEHTM("      <td class=\"hidden\">:&nbsp;&nbsp;&nbsp;</td>"         );
        LINEHTM("      <td class=\"hidden\">${AutoName}</td>"                 );
        LINEHTM("    </tr>"                                                   );
        LINEHTM("    <tr>"                                                    );
        LINEHTM("      <td class=\"hidden\"> </td>"                           );
        LINEHTM("      <td class=\"hidden\">&nbsp;&nbsp;&nbsp;</td>"          );
        LINEHTM("      <td class=\"hidden\">Compatibility</td>"               );
        LINEHTM("      <td class=\"hidden\">:&nbsp;&nbsp;&nbsp;</td>"         );
        LINEHTM("      <td class=\"hidden\">${HTML_DlpCompat}<br>"
                "${HTML_CmpRemarks}</td>"                                     );
        LINEHTM("    </tr>"                                                   );
        LINEHTM("    <tr>"                                                    );
        LINEHTM("      <td class=\"hidden\" colspan=\"5\">&nbsp;</td>"        );
        LINEHTM("    </tr>"                                                   );
        LINEHTM("    <tr>"                                                    );
        LINEHTM("      <td class=\"hidden\">C/C++</td>"                       );
        LINEHTM("      <td class=\"hidden\">&nbsp;&nbsp;&nbsp;</td>"          );
        LINEHTM("      <td class=\"hidden\">Wrapper Class</td>"               );
        LINEHTM("      <td class=\"hidden\">:&nbsp;&nbsp;&nbsp;</td>"         );
        LINEHTM("      <td class=\"hidden\"><code>"                           );
        LINEHTM(HtmlGetCSignature(lpBuf,NULL,CR_PROJECT)                      );
        LINEHTM("</code></td>"                                                );
        LINEHTM("    </tr>"                                                   );
        LINEHTM("    <tr>"                                                    );
        LINEHTM("      <td class=\"hidden\"> </td>"                           );
        LINEHTM("      <td class=\"hidden\">&nbsp;&nbsp;&nbsp;</td>"          );
        LINEHTM("      <td class=\"hidden\">Compatibility</td>"               );
        LINEHTM("      <td class=\"hidden\">:&nbsp;&nbsp;&nbsp;</td>"         );
        LINEHTM("      <td class=\"hidden\">${HTML_CxxCompat}</td>"           );
        LINEHTM("    </tr>"                                                   );
        LINEHTM("    <tr>"                                                    );
        LINEHTM("      <td class=\"hidden\" colspan=\"5\">&nbsp;</td>"        );
        LINEHTM("    </tr>"                                                   );
      }
      LINEHTM("    <tr>"                                                      );
      LINEHTM("      <td class=\"hidden\">General</td>"                       );
      LINEHTM("      <td class=\"hidden\">&nbsp;&nbsp;&nbsp;</td>"            );
      LINEHTM("      <td class=\"hidden\">Author</td>"                        );
      LINEHTM("      <td class=\"hidden\">:&nbsp;&nbsp;&nbsp;</td>"           );
      LINEHTM("      <td class=\"hidden\">${Author}</td>"                     );
      LINEHTM("    </tr>"                                                     );
      LINEHTM("    <tr>"                                                      );
      LINEHTM("      <td class=\"hidden\"> </td>"                             );
      LINEHTM("      <td class=\"hidden\">&nbsp;&nbsp;&nbsp;</td>"            );
      LINEHTM("      <td class=\"hidden\">Version</td>"                       );
      LINEHTM("      <td class=\"hidden\">:&nbsp;&nbsp;&nbsp;</td>"           );
      LINEHTM("      <td class=\"hidden\">${Version}</td>"                    );
      LINEHTM("    </tr>"                                                     );
      LINEHTM("  </table></div>"                                              );
      break;
    }
    case CR_ERROR:
    {
      SCGErr* lpErr = (SCGErr*)lpSCGMember;
      LINEHTM("  <div class=\"mframe\"><table cellpadding=\"0\" cellspacing="
              "\"0\" border=\"0\">"                                           );
      LINEHTM("    <tr>"                                                      );
      LINEHTM("      <td class=\"hidden\">dLabPro</td>"                       );
      LINEHTM("      <td class=\"hidden\">&nbsp;&nbsp;&nbsp;</td>"            );
      LINEHTM("      <td class=\"hidden\">Identifier</td>"                    );
      LINEHTM("      <td class=\"hidden\">:&nbsp;&nbsp;&nbsp;</td>"           );
      LINEHTM("      <td class=\"hidden\"><code><b>${HTML_MemberId}</b></code>"
              "</td>"                                                         );
      LINEHTM("    </tr>"                                                     );
      LINEHTM("    <tr>"                                                      );
      LINEHTM("      <td class=\"hidden\">&nbsp;</td>"                        );
      LINEHTM("      <td class=\"hidden\">&nbsp;&nbsp;&nbsp;</td>"            );
      LINEHTM("      <td class=\"hidden\">Error Level</td>"                   );
      LINEHTM("      <td class=\"hidden\">:&nbsp;&nbsp;&nbsp;</td>"           );
      sprintf(lpBuf,"      <td class=\"hidden\">%s</td>",lpErr->lpErrorLevel  );
      LINEHTM(lpBuf                                                           );
      LINEHTM("    </tr>"                                                     );
      LINEHTM("    <tr>"                                                      );
      LINEHTM("      <td class=\"hidden\" colspan=\"5\">&nbsp;</td>"          );
      LINEHTM("    </tr>"                                                     );
      LINEHTM("    <tr>"                                                      );
      LINEHTM("      <td class=\"hidden\">C/C++</td>"                         );
      LINEHTM("      <td class=\"hidden\">&nbsp;&nbsp;&nbsp;</td>"            );
      LINEHTM("      <td class=\"hidden\">Symbol</td>"                        );
      LINEHTM("      <td class=\"hidden\">:&nbsp;&nbsp;&nbsp;</td>"           );
      sprintf(lpBuf,"      <td class=\"hidden\"><code>#define %s -%d</code>"
              "</td>",lpErr->lpName,lpErr->nId+1001                           );
      LINEHTM(lpBuf                                                           );
      LINEHTM("    </tr>"                                                     );
      LINEHTM("  </table></div>"                                              );
      break;
    }
    case CR_FIELD:
    {
      SCGFld* lpFld = (SCGFld*)lpSCGMember;
      LINEHTM("  <div class=\"mframe\"><table cellpadding=\"0\" cellspacing="
              "\"0\" border=\"0\">"                                           );
      LINEHTM("    <tr>"                                                      );
      LINEHTM("      <td class=\"hidden\">dLabPro</td>"                       );
      LINEHTM("      <td class=\"hidden\">&nbsp;&nbsp;&nbsp;</td>"            );
      LINEHTM("      <td class=\"hidden\">Identifier</td>"                    );
      LINEHTM("      <td class=\"hidden\">:&nbsp;&nbsp;&nbsp;</td>"           );
      LINEHTM("      <td class=\"hidden\"><code><b>${HTML_MemberId}</b></code>"
              "</td>"                                                         );
      LINEHTM("    </tr>"                                                     );
      LINEHTM("    <tr>"                                                      );
      LINEHTM("      <td class=\"hidden\">&nbsp;</td>"                        );
      LINEHTM("      <td class=\"hidden\">&nbsp;&nbsp;&nbsp;</td>"            );
      LINEHTM("      <td class=\"hidden\">Type</td>"                          );
      LINEHTM("      <td class=\"hidden\">:&nbsp;&nbsp;&nbsp;</td>"           );
      LINEHTM("      <td class=\"hidden\">"                                   );
      if (lpFld->nType==T_INSTANCE) strcpy(lpBuf,lpFld->lpType);
      else dlp_strcpy(lpBuf,dlp_get_type_name(lpFld->nType));
      LINEHTM(lpBuf                                                           );
      LINEHTM("</td>"                                                         );
      LINEHTM("    </tr>"                                                     );
      LINEHTM("    <tr>"                                                      );
      LINEHTM("      <td class=\"hidden\">&nbsp;</td>"                        );
      LINEHTM("      <td class=\"hidden\">&nbsp;&nbsp;&nbsp;</td>"            );
      LINEHTM("      <td class=\"hidden\">Default</td>"                       );
      LINEHTM("      <td class=\"hidden\">:&nbsp;&nbsp;&nbsp;</td>"           );
      LINEHTM("      <td class=\"hidden\">"                                   );
      LINEHTM(lpFld->lpInit                                                   );
      LINEHTM("</td>"                                                         );
      LINEHTM("    </tr>"                                                     );
      LINEHTM("    <tr>"                                                      );
      LINEHTM("      <td class=\"hidden\"> </td>"                             );
      LINEHTM("      <td class=\"hidden\">&nbsp;&nbsp;&nbsp;</td>"            );
      LINEHTM("      <td class=\"hidden\">Properties</td>"                    );
      LINEHTM("      <td class=\"hidden\">:&nbsp;&nbsp;&nbsp;</td>"           );
      LINEHTM("      <td class=\"hidden\">"                                   );
      BEGIN_FLAGS(lpBuf)
      APPEND_FLAG(lpFld->nFlags,FF_NOSET       )
      APPEND_FLAG(lpFld->nFlags,FF_HIDDEN      )
      APPEND_FLAG(lpFld->nFlags,FF_NONAUTOMATIC)
      APPEND_FLAG(lpFld->nFlags,FF_NOSAVE      )
      END_FLAGS
      if (strcmp(lpBuf,"0")==0) strcpy(lpBuf,"-");
      LINEHTM(lpBuf                                                           );
      LINEHTM("</td>"                                                         );
      LINEHTM("    </tr>"                                                     );
      LINEHTM("    <tr>"                                                      );
      LINEHTM("      <td class=\"hidden\" colspan=\"5\">&nbsp;</td>"          );
      LINEHTM("    </tr>"                                                     );
      LINEHTM("    <tr>"                                                      );
      LINEHTM("      <td class=\"hidden\">C/C++</td>"                         );
      LINEHTM("      <td class=\"hidden\">&nbsp;&nbsp;&nbsp;</td>"            );
      LINEHTM("      <td class=\"hidden\">Member variable</td>"               );
      LINEHTM("      <td class=\"hidden\">:&nbsp;&nbsp;&nbsp;</td>"           );
      LINEHTM("      <td class=\"hidden\"><code>"                             );
      LINEHTM(HtmlGetCSignature(lpBuf,lpFld,CR_FIELD)                         );
      LINEHTM("</code></td>"                                                  );
      LINEHTM("    </tr>"                                                     );
      LINEHTM("  </table></div>"                                              );
      break;
    }
    case CR_OPTION:
    {
      SCGOpt* lpOpt = (SCGOpt*)lpSCGMember;
      LINEHTM("  <div class=\"mframe\"><table cellpadding=\"0\" cellspacing="
              "\"0\" border=\"0\">"                                           );
      LINEHTM("    <tr>"                                                      );
      LINEHTM("      <td class=\"hidden\">dLabPro</td>"                       );
      LINEHTM("      <td class=\"hidden\">&nbsp;&nbsp;&nbsp;</td>"            );
      LINEHTM("      <td class=\"hidden\">Identifier</td>"                    );
      LINEHTM("      <td class=\"hidden\">:&nbsp;&nbsp;&nbsp;</td>"           );
      LINEHTM("      <td class=\"hidden\"><code><b>${HTML_MemberId}</b></code>"
              "</td>"                                                         );
      LINEHTM("    </tr>"                                                     );
      LINEHTM("    <tr>"                                                      );
      LINEHTM("      <td class=\"hidden\" colspan=\"5\">&nbsp;</td>"          );
      LINEHTM("    </tr>"                                                     );
      LINEHTM("    <tr>"                                                      );
      LINEHTM("      <td class=\"hidden\">C/C++</td>"                         );
      LINEHTM("      <td class=\"hidden\">&nbsp;&nbsp;&nbsp;</td>"            );
      LINEHTM("      <td class=\"hidden\">Member variable</td>"               );
      LINEHTM("      <td class=\"hidden\">:&nbsp;&nbsp;&nbsp;</td>"           );
      LINEHTM("      <td class=\"hidden\"><code>BOOL <b>"                     );
      LINEHTM(lpOpt->lpCName);
      LINEHTM("</b>;</code></td>"                                             );
      LINEHTM("    </tr>"                                                     );
      LINEHTM("  </table></div>"                                              );
      break;
    }
    case CR_METHOD:
    {
      SCGMth* lpMth = (SCGMth*)lpSCGMember;
      LINEHTM("  <div class=\"mframe\"><table cellpadding=\"0\" cellspacing="
              "\"0\" border=\"0\">"                                           );
      LINEHTM("    <tr>"                                                      );
      LINEHTM("      <td class=\"hidden\">dLabPro</td>"                       );
      LINEHTM("      <td class=\"hidden\">&nbsp;&nbsp;&nbsp;</td>"            );
      LINEHTM("      <td class=\"hidden\">Identifier</td>"                    );
      LINEHTM("      <td class=\"hidden\">:&nbsp;&nbsp;&nbsp;</td>"           );
      LINEHTM("      <td class=\"hidden\"><code><b>${HTML_MemberId}</b></code>"
              "</td>"                                                         );
      LINEHTM("    </tr>"                                                     );
      LINEHTM("    <tr>"                                                      );
      LINEHTM("      <td class=\"hidden\">&nbsp;</td>"                        );
      LINEHTM("      <td class=\"hidden\">&nbsp;&nbsp;&nbsp;</td>"            );
      LINEHTM("      <td class=\"hidden\">Syntax</td>"                        );
      LINEHTM("      <td class=\"hidden\">:&nbsp;&nbsp;&nbsp;</td>"           );
      LINEHTM("      <td class=\"hidden\"><code>"                             );
      LINEHTM(HtmlGetUpnSignature(lpBuf,lpMth,CR_METHOD)                      );
      LINEHTM("</code></td>"                                                  );
      LINEHTM("    </tr>"                                                     );
      LINEHTM("    <tr>"                                                      );
      LINEHTM("      <td class=\"hidden\" colspan=\"5\">&nbsp;</td>"          );
      LINEHTM("    </tr>"                                                     );
      LINEHTM("    <tr>"                                                      );
      LINEHTM("      <td class=\"hidden\">C/C++</td>"                         );
      LINEHTM("      <td class=\"hidden\">&nbsp;&nbsp;&nbsp;</td>"            );
      LINEHTM("      <td class=\"hidden\">Member function</td>"               );
      LINEHTM("      <td class=\"hidden\">:&nbsp;&nbsp;&nbsp;</td>"           );
      LINEHTM("      <td class=\"hidden\"><code>"                             );
      LINEHTM(HtmlGetCSignature(lpBuf,lpMth,CR_METHOD)                        );
      LINEHTM("</code></td>"                                                  );
      LINEHTM("    </tr>"                                                     );
      LINEHTM("    <tr>"                                                      );
      LINEHTM("      <td class=\"hidden\">&nbsp;</td>"                        );
      LINEHTM("      <td class=\"hidden\">&nbsp;&nbsp;&nbsp;</td>"            );
      LINEHTM("      <td class=\"hidden\">Source</td>"                        );
      LINEHTM("      <td class=\"hidden\">:&nbsp;&nbsp;&nbsp;</td>"           );
      LINEHTM("      <td class=\"hidden\">${HTML_SrcFileAndLine}</td>"        );
      LINEHTM("    </tr>"                                                     );
      LINEHTM("  </table></div>"                                              );
      break;
    }
    case CR_CFUNC:
    {
      SCGMth* lpFnc = (SCGMth*)lpSCGMember;
      LINEHTM("  <div class=\"mframe\"><table cellpadding=\"0\" cellspacing="
              "\"0\" border=\"0\">"                                           );
      LINEHTM("    <tr>"                                                      );
      LINEHTM("      <td class=\"hidden\"></td>"                              );
      LINEHTM("      <td class=\"hidden\">&nbsp;&nbsp;&nbsp;</td>"            );
      LINEHTM("      <td class=\"hidden\">Signature</td>"                     );
      LINEHTM("      <td class=\"hidden\">:&nbsp;&nbsp;&nbsp;</td>"           );
      LINEHTM("      <td class=\"hidden\"><code>"                             );
      LINEHTM(HtmlGetCSignature(lpBuf,lpFnc,CR_CFUNC)                         );
      LINEHTM("</code></td>"                                                  );
      LINEHTM("    </tr>"                                                     );
      LINEHTM("    <tr>"                                                      );
      LINEHTM("      <td class=\"hidden\">&nbsp;</td>"                        );
      LINEHTM("      <td class=\"hidden\">&nbsp;&nbsp;&nbsp;</td>"            );
      LINEHTM("      <td class=\"hidden\">Source</td>"                        );
      LINEHTM("      <td class=\"hidden\">:&nbsp;&nbsp;&nbsp;</td>"           );
      LINEHTM("      <td class=\"hidden\">${HTML_SrcFileAndLine}</td>"        );
      LINEHTM("    </tr>"                                                     );
      LINEHTM("  </table></div>"                                              );
      break;
    }
  }

  // Synopsis text and description
  SCGStr* lMan  = NULL;
  SCGStr* lCman = NULL;
  char*   lpCmt = NULL;
  INT16   bHtml = FALSE;

  strcpy(lpModuleType,m_bClib?"Library":"Class");
  strcpy(lpModuleId  ,m_lpsClass);
  switch (nMemberType)
  {
    case CR_PROJECT:
    {
      lpCmt = m_lpsComment;
      lCman = m_cmanual.m_items;
      lMan  = m_manual.m_items;
      bHtml = m_bHtmlMan;
      strcpy(lpMemberType,m_bClib?"Library":"Class");
      strcpy(lpMemberId  ,m_lpsClass);
      break;
    }
    case CR_ERROR:
    {
      SCGErr* lpErr = (SCGErr*)lpSCGMember;
      lpCmt = lpErr->lpComment;
      lCman = lpErr->lCman.m_items;
      lMan  = lpErr->lMan.m_items;
      bHtml = lpErr->bHtmlMan;
      if (strstr(lpErr->lpErrorLevel,"EL_FATAL")==lpErr->lpErrorLevel)
        strcpy(lpMemberType,"Fatal error");
      else if (strstr(lpErr->lpErrorLevel,"EL_WARNING")==lpErr->lpErrorLevel)
        strcpy(lpMemberType,"Warning");
      else 
        strcpy(lpMemberType,"Error");
      sprintf(lpMemberId,"%s%d",m_lpsClass,lpErr->nId+1001);
      break;
    }
    case CR_FIELD:
    {
      SCGFld* lpFld = (SCGFld*)lpSCGMember;
      lpCmt = lpFld->lpComment;
      lCman = lpFld->lCman.m_items;
      lMan  = lpFld->lMan.m_items;
      bHtml = lpFld->bHtmlMan;
      strcpy(lpMemberType,"Field");
      strcpy(lpMemberId  ,lpFld->lpName);
      break;
    }
    case CR_OPTION:
    {
      SCGOpt* lpOpt = (SCGOpt*)lpSCGMember;
      lpCmt = lpOpt->lpComment;
      lCman = lpOpt->lCman.m_items;
      lMan  = lpOpt->lMan.m_items;
      bHtml = lpOpt->bHtmlMan;
      strcpy(lpMemberType,"Option");
      strcpy(lpMemberId  ,lpOpt->lpName);
      break;
    }
    case CR_METHOD:
    case CR_CFUNC :
    {
      SCGMth* lpMth = (SCGMth*)lpSCGMember;
      lpCmt = lpMth->lpComment;
      lCman = lpMth->lCman.m_items;
      lMan  = lpMth->lMan.m_items;
      bHtml = lpMth->bHtmlMan;
      strcpy(lpMemberType,nMemberType==CR_METHOD?"Method":"C/C++ Function");
      strcpy(lpMemberId  ,lpMth->lpName);
      if (dlp_strlen(lpMth->lpFile))
      {
        char lpsBuf[FILENAME_MAX+1];
        dlp_splitpath(lpMth->lpFile,NULL,lpsBuf);
        sprintf(lpsSrcFileAndLine,"%s(%ld)",lpsBuf,(long)lpMth->nLine);
      }
      break;
    }
    case CR_NOTE:
    {
      SCGRnt* lpRnt = (SCGRnt*)lpSCGMember;
      lpCmt = lpRnt->lpComment;
      lCman = lpRnt->lCman.m_items;
      lMan  = lpRnt->lMan.m_items;
      bHtml = lpRnt->bHtmlMan;
      strcpy(lpMemberType,"");
      strcpy(lpMemberId  ,lpRnt->lpName);
      break;
    }
  }

  LINEHTM("  <div class=\"mframe2\">");
  if (dlp_strlen(lpCmt) && nMemberType!=CR_CFUNC)
  {
    LINEHTM("  <h3>Synopsis</h3>"   );
    LINEHTM("  <p>"                 );
    LINEHTM(HtmlStrCnvt(lpBuf,lpCmt));
    LINEHTM("  </p>"                );
  }
  if (lMan)
  {
    LINEHTM("  <h3>Description</h3>");
    if (!bHtml) LINEHTM("<pre>");
    while (lMan)
    {
      LINEHTM(bHtml?lMan->lpName:HtmlStrCnvt(lpBuf,lMan->lpName));
      lMan=lMan->next;
    }
    if (!bHtml) LINEHTM("</pre>");
  }
  LINEHTM("  <!--{{ CUSTOM_DOC -->");
  while (lCman)
  {
    LINEHTM(lCman->lpName);
    lCman=lCman->next;
  }
  LINEHTM("  <!--}} CUSTOM_DOC -->");

  // Write member overview
  if (nMemberType==CR_PROJECT)
  {
    LINEHTM(""                                                                );
    if (!m_rnts.IsListEmpty())
    {
      LINEHTM("  <h3><a name=\"rnt\">Notes</a></h3>"                          );
      LINEHTM("  <table cellpadding=\"4\" cellspacing=\"0\" border=\"0\">"    );
      HtmlMemberOverview(htmFileTmpl,CR_NOTE);
      LINEHTM("  </table>"                                                    );
    }
    LINEHTM(""                                                                );
    LINEHTM("  <h3><a name=\"cls_members\">${PType} Members</a></h3>"         );
    LINEHTM("  <table cellpadding=\"4\" cellspacing=\"0\" border=\"0\">"      );
    HtmlMemberOverview(htmFileTmpl,CR_FIELD );
    HtmlMemberOverview(htmFileTmpl,CR_OPTION);
    HtmlMemberOverview(htmFileTmpl,CR_METHOD);
    HtmlMemberOverview(htmFileTmpl,CR_CFUNC );
    HtmlMemberOverview(htmFileTmpl,CR_ERROR );
    LINEHTM("  </table>"                                                      );
  }

  LINEHTM("  </div>"                                                          );
  if (nMemberType!=CR_PROJECT || m_rnts.IsListEmpty())
    LINEHTM("  <img src=\"../resources/blank_stc.gif\" height=\"10\">"        );
  LINEHTM("  </div>");

  // Member type specific key replacements
  switch (nMemberType)
  {
    case CR_PROJECT:
    {
      if (!m_bCxxNconv) strcpy(lpCmpRemarks,"C++ naming conventions disabled");
      if (m_bNoIdcheck)
      {
        if (strlen(lpCmpRemarks)) strcat(lpCmpRemarks,"; ");
        strcat(lpCmpRemarks,"dLabPro naming conventions disabled");
      }
      break;
    }
  }

  // General key replacements
  for (lpFLine=lpFLine->next; lpFLine; lpFLine=lpFLine->next)
  {
    ReplaceKey(lpFLine->lpName,"${HTML_DivId}"         ,HtmlGetDivId(lpBuf,lpSCGMember,nMemberType),0);
    ReplaceKey(lpFLine->lpName,"${HTML_ModuleType}"    ,lpModuleType                               ,0);
    ReplaceKey(lpFLine->lpName,"${HTML_ModuleId}"      ,lpModuleId                                 ,0);
    ReplaceKey(lpFLine->lpName,"${HTML_MemberType}"    ,lpMemberType                               ,0);
    ReplaceKey(lpFLine->lpName,"${HTML_MemberId}"      ,lpMemberId                                 ,0);
    ReplaceKey(lpFLine->lpName,"${HTML_CxxCompat}"     ,(char*)(m_bCProject?"ANSI-C, ANSI C++":"ANSI-C++")  ,0);
    ReplaceKey(lpFLine->lpName,"${HTML_DlpCompat}"     ,(char*)("dLabPro")                         ,0);
    ReplaceKey(lpFLine->lpName,"${HTML_CmpRemarks}"    ,lpCmpRemarks                               ,0);
    ReplaceKey(lpFLine->lpName,"${HTML_SrcFileAndLine}",lpsSrcFileAndLine                          ,0);
  }

}

void CGEN_PROTECTED CCgen::HtmlNaviBar(CList<SCGStr>& htmFileTmpl)
{
  LINEHTM("        <a class=\"navbar\" href=\"../home.html\">Home</a>"        );
  LINEHTM("        <a class=\"navbar\" href=\"#cls\">Top</a>"                 );
  if (!m_bClib)
  {
    LINEHTM("        <a class=\"navbar\" href=\"#fld\">Fields</a>"            );
    LINEHTM("        <a class=\"navbar\" href=\"#opt\">Options</a>"           );
    LINEHTM("        <a class=\"navbar\" href=\"#mth\">Methods</a>"           );
  }
  LINEHTM("        <a class=\"navbar\" href=\"#cfn\">C/C++</a>"               );
  if (!m_bClib)
  {
    LINEHTM("        <a class=\"navbar\" href=\"#err\">Errors</a>"            );
  }
}

char* CGEN_PROTECTED CCgen::HtmlStrCnvt(char* lpDest, char* lpSrc)
{
  if (lpSrc!=lpDest) dlp_strcpy(lpDest,lpSrc);
  dlp_strreplace(lpDest,"&","&amp;");
  dlp_strreplace(lpDest,"<","&lt;" );
  dlp_strreplace(lpDest,">","&gt;" );
  return lpDest;
}

char* CGEN_PROTECTED CCgen::HtmlGetDivId(char* lpStr, void* lpSCGMember, INT16 nMemberType)
{
  lpStr[0]=0;
  switch (nMemberType)
  {
    case CR_METHOD:
    {
      SCGMth* lpMth = (SCGMth*)lpSCGMember;
      sprintf(lpStr,"mth_%s",lpMth->lpName);
      break;
    }
    case CR_OPTION:
    {
      SCGOpt* lpOpt = (SCGOpt*)lpSCGMember;
      sprintf(lpStr,"opt_%s",lpOpt->lpName);
      break;
    }
    case CR_FIELD:
    {
      SCGFld* lpFld = (SCGFld*)lpSCGMember;
      sprintf(lpStr,"fld_%s",lpFld->lpName);
      break;
    }
    case CR_ERROR:
    {
      SCGErr* lpErr = (SCGErr*)lpSCGMember;
      sprintf(lpStr,"err_%s",lpErr->lpName);
      break;
    }
    case CR_NOTE:
    {
      SCGRnt* lpRnt = (SCGRnt*)lpSCGMember;
      sprintf(lpStr,"rnt_%03d",lpRnt->nId);
      break;
    }
    case CR_CFUNC:
    {
      SCGMth* lpFnc = (SCGMth*)lpSCGMember;
      sprintf(lpStr,"cfn_%03d",lpFnc->nId);
      break;
    }
    case CR_PROJECT:
    {
      strcpy(lpStr,"cls");
      break;
    }
    default: return lpStr;
  }

  dlp_strreplace(lpStr,"/","_");
  dlp_strreplace(lpStr,"<","_");
  dlp_strreplace(lpStr,">","_");
  dlp_strreplace(lpStr,"?","_");
  dlp_strreplace(lpStr,"#","_");

  return lpStr;
}

char* CGEN_PROTECTED CCgen::HtmlGetUpnSignature(char* lpStr, void* lpSCGMember, INT16 nMemberType)
{
  char lpBuf[1024];
  char lpBuf2[1024];
  char lpBuf3[L_NAMES+1];

  lpStr[0]=0;
  switch (nMemberType)
  {
    case CR_METHOD:
    {
      SCGMth* lpMth = (SCGMth*)lpSCGMember;
      HtmlStrCnvt(lpBuf ,lpMth->lpUPNSyntax);
      HtmlStrCnvt(lpBuf2,lpMth->lpPostsyn  );
      HtmlStrCnvt(lpBuf3,lpMth->lpName     );
      sprintf(lpStr,"%s <b>%s</b> %s",lpBuf,lpBuf3,lpBuf2);
      break;
    }
    default: DLPASSERT(FALSE); // Do not call this function for non-members
  }
  return lpStr;
}

char* CGEN_PROTECTED CCgen::HtmlGetCSignature(char* lpStr, void* lpSCGMember, INT16 nMemberType)
{
  char lpBuf[1024];

  lpStr[0]=0;
  switch (nMemberType)
  {
    case CR_PROJECT:
    {
      if (m_bCProject) strcpy(lpStr,"typedef struct <b>${CxxClass}</b><br>");
      strcat(lpStr,"class <b>${CxxClass}</b> : public ${Parent} ${MoreParents}");
      break;
    }
    case CR_FIELD:
    {
      SCGFld* lpFld = (SCGFld*)lpSCGMember;
      if (lpFld->nArrayLen>1) sprintf(lpBuf,"[%ld]",(long)lpFld->nArrayLen); else strcpy(lpBuf,"");
      sprintf(lpStr,"%s%s <b>%s</b>%s;",m_bCProject?"":"public ",
              lpFld->lpCType,lpFld->lpCName,lpBuf);
      break;
    }
    case CR_METHOD:
    case CR_CFUNC :
    {
      SCGMth* lpMth = (SCGMth*)lpSCGMember;
      if (dlp_strlen(m_lpsCName))
      {
        lpStr[0]='\0';
        if (m_bCProject)
        {
          if (strstr(lpMth->lpReturn.lpMdfr," static")==0)
          {
            sprintf(lpBuf,"%s _this",m_lpsCName);
            if (dlp_strlen(lpMth->lpCSyntax)) strcat(lpBuf,", ");
          }
          else lpBuf[0]='\0';
          sprintf(lpStr,"%s <b>%s_%s</b>(%s%s);<br>",
                  dlp_strlen(lpMth->lpReturn.lpType)?lpMth->lpReturn.lpType:"INT16",
                  m_lpsCName,lpMth->lpCName,lpBuf,lpMth->lpCSyntax);
        }
        sprintf(lpBuf,"%s %s <b>%s::%s</b>(%s);<br>",
                lpMth->lpReturn.lpMdfr,
                dlp_strlen(lpMth->lpReturn.lpType)?lpMth->lpReturn.lpType:"INT16",
                m_lpsCName,lpMth->lpCName,lpMth->lpCSyntax);
        strcat(lpStr,lpBuf);
      }
      else
        sprintf(lpStr,"%s <b>%s</b>(%s);<br>",
                dlp_strlen(lpMth->lpReturn.lpType)?lpMth->lpReturn.lpType:"INT16",
                lpMth->lpCName,lpMth->lpCSyntax);
      break;
    }
    default: DLPASSERT(FALSE); // Do not call this function for non-members
  }
  return lpStr;
}

// EOF
