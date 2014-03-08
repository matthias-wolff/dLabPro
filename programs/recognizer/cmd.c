/* dLabPro program recognizer (dLabPro recognizer)
 * - Command line steering interface
 *
 * AUTHOR : Frank Duckhorn
 * PACKAGE: dLabPro/programs
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
#include "recognizer.h"

extern INT64 nFrame;
extern INT64 nLastActive;

void setopt(char *lpsCmd){
  char *lpsOpt=dlp_strsep(&lpsCmd," \t",NULL);
  if(lpsCmd){
    while(lpsCmd[0]==' ' || lpsCmd[0]=='\t' || lpsCmd[0]=='=') lpsCmd++;
    lpsCmd=dlp_strsep(&lpsCmd," \t\n\r",NULL);
  }
  setoption(lpsOpt,lpsCmd,TRUE);
  if (rCfg.eOut==O_gui) optdump();
}

void rexit(char *lpsCmd){
  rCfg.bExit=TRUE;
}

/**
 * Command handler for the "dlgupd" command: performs a dialog FST transition.
 *
 * @param lpsCmd
 *          The command in the format "dlgupd &lt;state&gt;", where &lt;state&gt;
  *         is the input label of the dialog transition to perform.
 */
void dlgupd(char* lpsCmd){
  char *lpsRes=dlp_strsep(&lpsCmd," \t",NULL);
  if (dlp_strcmp(lpsRes,"__WAKEUP__")==0)
    nLastActive = nFrame;
  dlg_upd(lpsRes);
}

typedef void (*recoruncmd)(char*);

#define CMDQUEUELEN 32
struct recocmdqueue {
  INT32 nRd,nWr;
  struct {
    recoruncmd lpFnc;
    char       lpsArg[STR_LEN];
  } lpCmd[CMDQUEUELEN];
} rCmdQueue = { .nRd=0, .nWr=0 };

struct recocmd {
  const char *lpsCmd;
  BOOL  bArg;
  recoruncmd lpFnc;
} rRecoCmd[]={
  { "set",        TRUE,  setopt  },
  { "exit",       FALSE, rexit   },
  { "dlgupd",     TRUE,  dlgupd  },
  { "dump",       FALSE, optdump },
  { NULL,         FALSE, NULL    },
};

void cmdqueueget()
{
  if(rCmdQueue.nWr==rCmdQueue.nRd) return;
  (*rCmdQueue.lpCmd[rCmdQueue.nRd].lpFnc)(rCmdQueue.lpCmd[rCmdQueue.nRd].lpsArg);
  rCmdQueue.nRd=(rCmdQueue.nRd+1)%CMDQUEUELEN;
}

void cmdqueueput(recoruncmd lpFnc,char *lpsArg)
{
  INT32 nWrNx = (rCmdQueue.nWr+1)%CMDQUEUELEN;
  if(nWrNx==rCmdQueue.nRd) rerror("Command queue full");
  rCmdQueue.lpCmd[rCmdQueue.nWr].lpFnc=lpFnc;
  if(lpsArg) snprintf(rCmdQueue.lpCmd[rCmdQueue.nWr].lpsArg,STR_LEN,lpsArg);
  else rCmdQueue.lpCmd[rCmdQueue.nWr].lpsArg[0]='\0';
  rCmdQueue.lpCmd[rCmdQueue.nWr].lpsArg[STR_LEN-1]='\0';
  rCmdQueue.nWr=nWrNx;
}

void *cmdthread(void *lpDat)
{
  char lpBuf[STR_LEN];
  INT32 nI;
  while(!rCfg.bExit && (!feof(stdin) && fgets(lpBuf,STR_LEN,stdin))){
    for(nI=0;rRecoCmd[nI].lpsCmd;nI++){
      INT32 nLen=strlen(rRecoCmd[nI].lpsCmd);
      if(!strncmp(lpBuf,rRecoCmd[nI].lpsCmd,nLen)){
        char *lpsS=NULL;
        if(rRecoCmd[nI].bArg){
          lpsS=lpBuf+nLen+1;
          if(lpBuf[nLen]!=' ') continue;
          while(*lpsS==' '  || *lpsS=='\t') lpsS++;
          lpsS=dlp_strsep(&lpsS,"\n\r",NULL);
        }else if(lpBuf[nLen]!='\n' && lpBuf[nLen]!='\r' && lpBuf[nLen]!='\0') continue;
        cmdqueueput(rRecoCmd[nI].lpFnc,lpsS);
        break;
      }
    }
    if(!rRecoCmd[nI].lpsCmd) rerror("Unknown command: \"%s\"",lpBuf);
  }
  if (!rCfg.bExit){
    rerror("Input pipe closed -> exiting");
    exit(-1);
  }
  return NULL;
}


