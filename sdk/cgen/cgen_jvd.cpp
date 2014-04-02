// dLabPro SDK class CCgen (cgen)
// - JavaDoc support
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

typedef struct tagSJDStr
{
  char       lpName[L_NAMES];
  INT16      nId;
  char       lpText[L_INPUTLINE];
  tagSJDStr* next;
} SJDStr;

/**
 * Creates HTML links for preprocessed JavaDoc @see tags. <a
 * href="#cfn_JavaDoc2Html"><code class="link">JavaDoc2Html</code></a>
 * coverts JavaDoc "<code>@<b>see</b></code> <i>reference</i>" tags into
 * CGen replacement marks <code><b>${@see</b> <i>reference</i><b>}</b></code>.
 * <code>JdSee</code> scans the HTML documentation template
 * (<code>m_lHtml</code>) for these marks and creates HTML links for
 * references within the same class or library (i.e. within the same HTML file).
 *
 * @param lHtml A pointer to the HTML file template to modify.
 */
void CGEN_PROTECTED CCgen::JdSee(CList<SCGStr>* lHtml)
{
  // Local variables
  INT16   nItyp   = -1;
  SCGStr* lpLine  = NULL;
  void*   lpItem  = NULL;
  char*   lpsFc   = NULL;         // First character of current @see/@link tag
  char*   lpsLc   = NULL;         // Last character of current @see/@link tag
  char*   lpsFcS  = NULL;         // First character of next @see tag
  char*   lpsFcL  = NULL;         // First character of next @link tag
  char    lpsTag[L_INPUTLINE];    // The extracted current @see/@link tag
  char    lpsLnk[L_INPUTLINE];    // The generated HTML link for the current @see/@link tag
  char    lpsId [L_INPUTLINE];    // The referenced identifier extracted from current tag
  char*   lpsText = NULL;         // The label text (pointer into lpsId)
  char    lpsManfn[L_PATH];

  char    lpsBuf[L_NAMES+20];

  if (!lHtml) return;

  // Loop over HTML code
  for (lpLine=lHtml->m_items; lpLine; lpLine=lpLine->next)
  {
    while (TRUE)
    {
      // Seek "${@see" or "{@link" which ever comes first
      lpsFc  = NULL;
      lpsLc  = NULL;
      lpsFcS = strstr(lpLine->lpName,"${@see");
      lpsFcL = strstr(lpLine->lpName,"{@link");

      if (lpsFcS                            ) lpsFc = lpsFcS;
      if (lpsFcL && (!lpsFc || lpsFc>lpsFcL)) lpsFc = lpsFcL;
      if (!lpsFc) break;

      // Seek closing curly brace
      // NOTE: no closing curly braces allowed inside tags; use &#125; instead!
      lpsLc = strstr(lpsFc,"}");
      DLPASSERT(lpsFc);
      if (!lpsLc) break;

      // Copy tag
      DLPASSERT(lpsLc-lpsFc+1<L_INPUTLINE); // Tag too long --> should be checked before
      dlp_memmove(lpsTag,lpsFc,lpsLc-lpsFc+1);
      lpsTag[lpsLc-lpsFc+1]='\0';

      // Make link - Trim
      dlp_strcpy(lpsLnk,lpsTag);
      dlp_strreplace(lpsLnk,"${@see","");
      dlp_strreplace(lpsLnk,"{@link","");
      dlp_strreplace(lpsLnk,"}"     ,"");
      dlp_strtrimleft(lpsLnk);
      dlp_strtrimright(lpsLnk);
      
      // Make link - Leave strings and hand-crafted links alone
      if      (lpsLnk[0]=='\"') dlp_strunquotate(lpsLnk,'\"','\"');
      else if (lpsLnk[0]!='<' )
      {
        if (lpsLnk[0]=='#') dlp_strunquotate(lpsLnk,'#','#');

        // Get first token of @see text
        dlp_strcpy(lpsId,lpsLnk);
        for (lpsText=lpsId; *lpsText; lpsText++)
          if (iswspace(*lpsText))
            { *lpsText++='\0'; break; }
        dlp_strtrimleft(lpsText);
        if (!dlp_strlen(lpsText)) lpsText=lpsId;

        // Try to find referred class member
        for (lpItem=m_cfns.m_items, nItyp=CR_CFUNC; lpItem; lpItem=((SCGMth*)lpItem)->next)
        {
          if (dlp_strcmp(((SCGMth*)lpItem)->lpName ,lpsId)==0) break;
          if (dlp_strcmp(((SCGMth*)lpItem)->lpCName,lpsId)==0) break;
        }
        if (!lpItem) for (lpItem=m_mths.m_items, nItyp=CR_METHOD; lpItem; lpItem=((SCGMth*)lpItem)->next)
        {
          if (dlp_strcmp(((SCGMth*)lpItem)->lpName ,lpsId)==0) break;
          if (dlp_strcmp(((SCGMth*)lpItem)->lpCName,lpsId)==0) break;
        }
        if (!lpItem) for (lpItem=m_flds.m_items, nItyp=CR_FIELD; lpItem; lpItem=((SCGFld*)lpItem)->next)
        {
          if (dlp_strcmp(((SCGFld*)lpItem)->lpName ,lpsId)==0) break;
          if (dlp_strcmp(((SCGFld*)lpItem)->lpCName,lpsId)==0) break;
        }
        if (!lpItem) for (lpItem=m_opts.m_items, nItyp=CR_OPTION; lpItem; lpItem=((SCGOpt*)lpItem)->next)
        {
          if (dlp_strcmp(((SCGOpt*)lpItem)->lpName ,lpsId)==0) break;
          if (dlp_strcmp(((SCGOpt*)lpItem)->lpCName,lpsId)==0) break;
        }
        if (!lpItem) for (lpItem=m_rnts.m_items, nItyp=CR_NOTE; lpItem; lpItem=((SCGRnt*)lpItem)->next)
        {
          if (dlp_strcmp(((SCGRnt*)lpItem)->lpName,lpsLnk)==0) break;
        }

        // Make HTML link
        if (lpItem)
        {
          if (nItyp==CR_NOTE)
          {
            // NOTE: the condition beautifies "sub-release-notes", whose names
            //       start with some "-" followed by a white space.
            if (strstr(lpsId,"--")!=lpsId) lpsText=lpsId;
          }
          sprintf
          (
            lpsLnk,
            "<a href=\"#%s\"><code class=\"link\">%s</code></a>",
            HtmlGetDivId(lpsBuf,lpItem,nItyp),lpsText
          );
        }
        else
        {
          dlp_convert_name(CN_MANFILE,lpsManfn,m_lpsProject);
          ERRORMSG(ERR_INLINE,lpsManfn,lpLine->nId,0);
          ERRORMSG(ERR_RESJDLINK,lpsTag,0,0);
        }
      }

      // Replace lpsTag with lpsLnk
      // printf("\n    m_lHtml(%4ld): |%s| --> |%s|",lpLine->nId,lpsTag,lpsLnk);
      dlp_strreplace(lpLine->lpName,lpsTag,lpsLnk);
    }
  }
}


/**
 * Preliminary implementation. Removes ${cgen:index ...} tags.
 *
 * @param lHtml A pointer to the HTML file template to modify.
 */
void CGEN_PROTECTED CCgen::JdIndex(CList<SCGStr>* lHtml)
{
  // Local variables
  SCGStr* lpLine  = NULL;
  char*   lpsFc   = NULL;         // First character of current @see/@link tag
  char*   lpsLc   = NULL;         // Last character of current @see/@link tag
  char    lpsTag[L_INPUTLINE];    // The extracted current @see/@link tag

  if (!lHtml) return;

  // Loop over HTML code
  for (lpLine=lHtml->m_items; lpLine; lpLine=lpLine->next)
  {
    while (TRUE)
    {
      // Seek "{@cgen:index"
      lpsFc = strstr(lpLine->lpName,"{@cgen:index");
      lpsLc = NULL;

      if (!lpsFc) break;

      // Seek closing curly brace
      // NOTE: no closing curly braces allowed inside tags; use &#125; instead!
      lpsLc = strstr(lpsFc,"}");
      DLPASSERT(lpsFc);
      if (!lpsLc) break;

      // Copy tag
      DLPASSERT(lpsLc-lpsFc+1<L_INPUTLINE); // Tag too long --> should be checked before
      dlp_memmove(lpsTag,lpsFc,lpsLc-lpsFc+1);
      lpsTag[lpsLc-lpsFc+1]='\0';

      // TODO: Parse tag and store in index

      // Replace lpsTag with nothing
      //printf("\n    m_lHtml(%4ld): |%s| --> ||",lpLine->nId,lpsTag);
      dlp_strreplace(lpLine->lpName,lpsTag,"");
    }
  }
}

/**
 * <p>Converts a token list containing JavaDoc (<code>lJavaDoc</code>)
 * into HTML code (<code>lHtml</code>).</p>
 * <p><b>NOTE:</b> <code>lpsShortDescr</code> Must be at least 256
 * bytes long!</p>
 *
 * @param lHtml         Pointer to string list to fill with HTML code
 * @param lpsShortDescr Pointer to string to fill with first sentence of
 *                      JavaDoc (may be NULL)
 * @param lJavaDoc      Pointer to string list containing JavaDoc
 * @param lpsFilename   Name of the JavaDoc's source file
 * @param nLine         Line number of the JavaDoc in source file
 * @return O_K if successfull, a (negative) error code otherwise
 */
INT16 CGEN_PROTECTED CCgen::JavaDoc2Html
(
  CList<SCGStr>* lHtml,
  char*          lpsShortDescr,
  CList<SCGStr>* lJavaDoc,
  const char*    lpsFilename,
  INT32 nLine
)
{
  // Local variables
  INT32          nCnt                = 0;
  INT32          nCopy               = 0;
  char          lpsBuf[L_INPUTLINE] = "";
  char          lpsTag[256]         = "";
  SCGStr*       lpTok               = NULL;
  char*         p                   = NULL;
  char*         q                   = NULL;
  char*         r                   = NULL;
  SJDStr*       lpJdItem            = NULL;
  CList<SJDStr> lJdc;

  // Validation
  DLPASSERT(lHtml   );
  DLPASSERT(lJavaDoc);

  lHtml->Delete();
  if (lpsShortDescr) lpsShortDescr[0]='\0';

  // Loop over JavaDoc lines
  lpTok = lJavaDoc->m_items;
  while (lpTok)
  {
    // Trim JavaDoc line
    for (p=lpTok->lpName,r=p; *p; p++)
    {
      if (iswspace(*p)) continue;
      if (lpTok==lJavaDoc->m_items && strstr(p,"/**")==p) { p+=3; r=p; break; }
      else if (*p=='*')
      {
        p++; r=p;
        if (*p=='/') { p++; r=p; break; }
      }
      for (q=p; *q; q++)
        if (!iswspace(*q))
          break;
      if (*q=='@')
      {
        p=q;
        while (*p && !iswspace(*p)) p++;
        if (p!=q)
        {
          dlp_strncpy(lpsTag,q,p-q);
          lpsTag[p-q]='\0';
          lpJdItem = lJdc.AddItem(lpsTag);
          while (*p && iswspace(*p)) p++;
          if
          (
            dlp_strcmp(lpJdItem->lpName,"@cgen:index"       ) &&
            dlp_strcmp(lpJdItem->lpName,"@cgen:experimental") &&
            dlp_strcmp(lpJdItem->lpName,"@cgen:option"      ) &&
            dlp_strcmp(lpJdItem->lpName,"@cgen:TODO:"       ) &&
            dlp_strcmp(lpJdItem->lpName,"@deprecated"       ) &&
            dlp_strcmp(lpJdItem->lpName,"@link"             ) &&
            dlp_strcmp(lpJdItem->lpName,"@param"            ) &&
            dlp_strcmp(lpJdItem->lpName,"@return"           ) &&
            dlp_strcmp(lpJdItem->lpName,"@see"              ) 
          )
          {
            ERRORMSG(ERR_INLINE ,lpsFilename  ,nLine ,0        );
            ERRORMSG(ERR_UNKNOWN,"JavaDoc tag",lpsTag,"Ignored");
          }
        }
      }
      break;
    }
    dlp_strtrimright(p);

    if (dlp_strlen(r) || lJdc.Count()>1)
    {
      // Store the rest (collect tag text into one item each; do not collect main text)
      if (!dlp_strlen(lpJdItem->lpName))
      {
        lpJdItem = lJdc.AddItem("");
        dlp_strcpy(lpJdItem->lpText,r);

        if (lpsShortDescr)
        {
          nCopy = (INT32)dlp_strlen(p);
          if (dlp_strlen(lpsShortDescr)+nCopy+1>=255) nCopy=255-(INT32)dlp_strlen(lpsShortDescr)-1;
          if (nCopy>0)
          {
            if (dlp_strlen(lpsShortDescr)) strcat(lpsShortDescr," ");
            strncat(lpsShortDescr,p,nCopy);
            lpsShortDescr[255]='\0';
          }
        }
      }
      else
      {
        DLPASSERT(lpJdItem);
        nCopy = (INT32)dlp_strlen(p);
        if (dlp_strlen(lpJdItem->lpText)+nCopy+1>=L_INPUTLINE)
        {
          nCopy=L_INPUTLINE-(INT32)dlp_strlen(lpJdItem->lpText)-1;
          ERRORMSG(ERR_TOOLONG,lpsFilename,nLine,"@param tag");
        }
        if (nCopy>0)
        {
          if (dlp_strlen(lpJdItem->lpText)) strcat(lpJdItem->lpText,"\n");
          strncat(lpJdItem->lpText,p,nCopy);
          lpJdItem->lpText[L_INPUTLINE-1]='\0';
        }
      }
    }

    // Goto next token
    lpTok=lpTok->next;
    nLine++;
  }

  // No JavaDoc --> leave
  if (!lJdc.m_items) return NOT_EXEC;

  // Finish short description
  if (lpsShortDescr)
    for (p=lpsShortDescr; *p; p++)
      if (*p=='.')
        p[1]='\0';
  dlp_strconvert(SC_STRIPHTML,lpsShortDescr,lpsShortDescr);

  // Write HTML - Main text
  for (lpJdItem=lJdc.m_items,nCnt=0; lpJdItem; lpJdItem=lpJdItem->next)
  {
    if (!dlp_strlen(lpJdItem->lpName))
    {
      sprintf(lpsBuf,"%s",lpJdItem->lpText);
      lHtml->AddItem(lpsBuf);
      nCnt++;
    }
  }

  // Write HTML - @cgen:experimental
  for (lpJdItem=lJdc.m_items,nCnt=0; lpJdItem; lpJdItem=lpJdItem->next)
  {
    if (dlp_strcmp(lpJdItem->lpName,"@cgen:experimental")==0)
    {
      if (!nCnt) lHtml->AddItem("\t<h3 style=\"color:red\">EXPERIMENTAL!</h3>");
      lHtml->AddItem("\t<p>");
      lHtml->AddItem((char*)(dlp_strlen(lpJdItem->lpText)?lpJdItem->lpText:
                     "This API is experimental and may be essentially modified or removed without notice."));
      lHtml->AddItem("\t</p>");
      nCnt++;
    }
  }

  // Write HTML - @deprecated
  for (lpJdItem=lJdc.m_items,nCnt=0; lpJdItem; lpJdItem=lpJdItem->next)
  {
    if (dlp_strcmp(lpJdItem->lpName,"@deprecated")==0)
    {
      if (!nCnt) lHtml->AddItem("\t<h3 style=\"color:red\">DEPRECATED!</h3>");
      lHtml->AddItem("\t<p>");
      lHtml->AddItem((char*)(dlp_strlen(lpJdItem->lpText)?lpJdItem->lpText:
                     "This API is deprecated and will be removed."));
      lHtml->AddItem("\t</p>");
      nCnt++;
    }
  }

  // Write HTML - @param
  for (lpJdItem=lJdc.m_items,nCnt=0; lpJdItem; lpJdItem=lpJdItem->next)
  {
    if (dlp_strcmp(lpJdItem->lpName,"@param")==0)
    {
      if (!nCnt)
      {
        lHtml->AddItem("\t<h3>Parameters</h3>");
        lHtml->AddItem("\t<table>");
      }
      p=strtok(lpJdItem->lpText," \t");
      if (dlp_strlen(p))
      {
        lHtml->AddItem("\t\t<tr>"                                    );
        lHtml->AddItem("\t\t\t<td class=\"hidden\">&nbsp;&nbsp;</td>");
        lHtml->AddItem("\t\t\t<td class=\"hidden\"><b><code>"        );
        lHtml->AddItem(p                                             );
        lHtml->AddItem("</code></b></td>"                            );
        lHtml->AddItem("\t\t\t<td class=\"hidden\">&nbsp;</td>"      );
        lHtml->AddItem("\t\t\t<td class=\"hidden\">"                 );
        lHtml->AddItem(&p[dlp_strlen(p)+1]                           );
        lHtml->AddItem("\t\t\t</td>"                                  );
        lHtml->AddItem("\t\t</tr>"                                   );
      }
      nCnt++;
    }
  }
  if (nCnt) lHtml->AddItem("\t</table>");

  // Write HTML - @cgen:option
  for (lpJdItem=lJdc.m_items,nCnt=0; lpJdItem; lpJdItem=lpJdItem->next)
  {
    if (dlp_strcmp(lpJdItem->lpName,"@cgen:option")==0)
    {
      if (!nCnt)
      {
        lHtml->AddItem("\t<h3>Options</h3>");
        lHtml->AddItem("\t<table>");
      }
      p=strtok(lpJdItem->lpText," \t");
      if (dlp_strlen(p))
      {
        lHtml->AddItem("\t\t<tr>"                                    );
        lHtml->AddItem("\t\t\t<td class=\"hidden\">&nbsp;&nbsp;</td>");
        lHtml->AddItem("\t\t\t<td class=\"hidden\">"                 );
        sprintf(lpsBuf,"{@link %s}",p);
        lHtml->AddItem(lpsBuf                                        );
        lHtml->AddItem("</td>"                                       );
        lHtml->AddItem("\t\t\t<td class=\"hidden\">&nbsp;</td>"      );
        lHtml->AddItem("\t\t\t<td class=\"hidden\">"                 );
        lHtml->AddItem(&p[dlp_strlen(p)+1]                           );
        lHtml->AddItem("\t\t\t</td>"                                  );
        lHtml->AddItem("\t\t</tr>"                                   );
      }
      nCnt++;
    }
  }
  if (nCnt) lHtml->AddItem("\t</table>");

  // Write HTML - @return
  for (lpJdItem=lJdc.m_items,nCnt=0; lpJdItem; lpJdItem=lpJdItem->next)
  {
    if (dlp_strcmp(lpJdItem->lpName,"@return")==0)
    {
      if (!nCnt) lHtml->AddItem("\t<h3>Return value</h3>");
      lHtml->AddItem("\t<p>");
      lHtml->AddItem(lpJdItem->lpText);
      lHtml->AddItem("\t</p>");
      nCnt++;
    }
  }

  // Write HTML - @see
  for (lpJdItem=lJdc.m_items,nCnt=0; lpJdItem; lpJdItem=lpJdItem->next)
  {
    if (dlp_strcmp(lpJdItem->lpName,"@see")==0)
    {
      if (!nCnt)
      {
        lHtml->AddItem("\t<h3>See also</h3>");
        lHtml->AddItem("\t<table>");
      }
      lHtml->AddItem("\t\t<tr>"                                    );
      lHtml->AddItem("\t\t\t<td class=\"hidden\">&nbsp;&nbsp;</td>");
      lHtml->AddItem("\t\t\t<td class=\"hidden\">"                 );
      // Make sure there are no "}"'s in the @see text -->
      sprintf(lpsBuf,"${@see %s",lpJdItem->lpText);
      dlp_strreplace(lpsBuf,"}","&#125;");
      dlp_strcat(lpsBuf,"}");
      // <--
      lHtml->AddItem(lpsBuf);
      lHtml->AddItem("\t\t\t</td>"                                 );
      lHtml->AddItem("\t\t</tr>"                                   );
      nCnt++;
    }
  }
  if (nCnt) lHtml->AddItem("\t</table>");

  // Write HTML - @cgen:TODO:
  for (lpJdItem=lJdc.m_items,nCnt=0; lpJdItem; lpJdItem=lpJdItem->next)
  {
    if (dlp_strcmp(lpJdItem->lpName,"@cgen:TODO:")==0)
    {
      if (!nCnt)
      {
        lHtml->AddItem("\t<h3>Developers' TODO List</h3>");
        lHtml->AddItem("\t<ul>");
      }
      lHtml->AddItem("\t<li>");
      lHtml->AddItem(lpJdItem->lpText);
      lHtml->AddItem("\t</li>");
      nCnt++;
    }
  }
  if (nCnt) lHtml->AddItem("\t</ul>");

  return O_K;
}

// EOF
