// dLabPro class CPMproc (PMproc)
// - Class CPMproc - CHFA code
//
// AUTHOR : Frank Duckhorn, Guntram Strecha and Hussein Hussein, Dresden
// PACKAGE: dLabPro/classes
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

#define M_PI 3.1415

#include "dlp_pmproc.h"        // Include class header file


struct PeriodPM{
  INT32 period;
  char stimulation;
};

struct aopt{
  char filename[256];
  char file[256];
  char outfile[256];
  INT32 niterations;
  FLOAT64 histfak;
  FLOAT64 histpow;
  FLOAT64 filtdiffint;
  FLOAT64 diffdiffint;
  char distback;
};

struct fopt{
  FLOAT64 fa;
  FLOAT64 fmin;
  FLOAT64 fmean;
  FLOAT64 fmax;
  struct aopt aopt;
};

struct topt{
  INT32 ta;
  INT32 tmin;
  INT32 tmean;
  INT32 tmax;
  struct aopt aopt;
};

struct filt{
  FLOAT64 *a;
  INT32 na;
  FLOAT64 *b;
  INT32 nb;
};


#define HIST_SIZE  100

#undef MAX
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#undef MIN
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#undef ABS
#define ABS(a) ((a) >= 0 ? (a) : -(a))
#undef SGN
#define SGN(a)          (((a)<0) ? -1 : 1)
#undef ODD
#define ODD(n) ((n) & 1)
#undef ROUND32
#define ROUND32(X) (((X) >= 0) ? (INT32)((X)+0.5) : (INT32)((X)-0.5))
#undef ROUND16
#define ROUND16(X) (((X) >= 0) ? (INT16)((X)+0.5) : (INT16)((X)-0.5))

#define TOPT(a)    ( (a)>topt.tmax ? topt.tmax : ( (a)<topt.tmin ? topt.tmin : (a) ) )
#define ROUND(X)  (((X) >= 0) ? (INT32)((X)+0.5) : (INT32)((X)-0.5))

#define FILTER(f,i,ni,o,m)  filter((f).b,(f).nb,(f).a,(f).na,i,ni,o,m)

/* Hochpass als Vorfilter */
FLOAT64 filt_high_a0[3]={+1.0000000000000, -1.9861162115409, +0.9862119291608};
FLOAT64 filt_high_b0[3]={+0.9930820351754, -1.9861640703508, +0.9930820351754};
FLOAT64 filt_high_a1[3]={+1.0000000000000, -1.9722337291953, +0.9726139693131};
FLOAT64 filt_high_b1[3]={+0.9862119246271, -1.9724238492542, +0.9862119246271};
FLOAT64 filt_high_a2[3]={+1.0000000000000, -1.9444776577671, +0.9459779362323};
FLOAT64 filt_high_b2[3]={+0.9726138984998, -1.9452277969997, +0.9726138984998};
struct filt filt_high[3]={
    { filt_high_a0, 3, filt_high_b0, 3 },
    { filt_high_a1, 3, filt_high_b1, 3 },
    { filt_high_a2, 3, filt_high_b2, 3 }
};


/*---------------------------------------------------------------------
 --------------------------- Hauptprogramm ----------------------------
 ---------------------------------------------------------------------*/
INT16 CGEN_PUBLIC CPMproc::Chfa(data* dSignal, data* dPM)
{
  if (!dPM || !dPM->IsEmpty())
    return NOT_EXEC;

  INT32 nSamples = dSignal->GetNRecs();

  FLOAT64* samples = (FLOAT64*)dlp_calloc(nSamples, sizeof(FLOAT64));
  for(INT32 iSamples=0; iSamples<nSamples; iSamples++) samples[iSamples] = dSignal->Dfetch(iSamples,0);


   dPM->AddNcomps(T_SHORT, 2);
   dPM->SetCname(0, "pm");
   dPM->SetCname(1, "v/uv");
   ISETFIELD_RVALUE(dPM,"fsr", 1000.0/m_nSrate);

  struct PeriodPM *periods;
  INT32 nperiods = 0;

  DEBUGMSG(-1,"\nCalculate PM from CHFA ...",0,0,0);
  chfa(samples, nSamples, &periods, &nperiods);

  // AddRecs: Appends nRecs records to the end of the table
  dPM->AddRecs(nperiods, 1); // (nRealloc = 1) i.e. the table's memory block will be reallocated if necessary to hold the new records

    for(INT32 iPeriods = 0; iPeriods < nperiods; iPeriods++)
    {
      dPM->Dstore((FLOAT64)(periods+iPeriods)->period, iPeriods, 0);
      dPM->Dstore((FLOAT64)(periods+iPeriods)->stimulation, iPeriods, 1);
    }


   dlp_free(samples);
   dlp_free(periods);

  return O_K;
}


////////////////////////////////////////////////////////////////////////////////////
//////////////////////////// Definition of Function ////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

/*------------------------------------------------------------
  Function:        CHFA Algorithm

  Calculate PM using Complex Harmonic Filter Analysis (CHFA)
 ------------------------------------------------------------*/
INT16 CGEN_PRIVATE CPMproc::chfa(FLOAT64* samples, INT32 nSamples, struct PeriodPM** periods, INT32* nperiods)
{
  struct topt topt;
  struct filt  filt_high2;
  struct filt *filt_r = NULL;
  struct filt *filt_i = NULL;
  struct filt *filt_freq = NULL;
  *periods = NULL;
  *nperiods = 0;
  FLOAT64  *sam = NULL;
  FLOAT64  *fsam = NULL;
  FLOAT64  *fsam_e = NULL;
  FLOAT64  *fsam_p = NULL;
  char  *fsam_v = NULL;
  FLOAT64  abs_voiceless = 0;
  FLOAT64  phase=M_PI;
  FLOAT64  *times = NULL;
  FLOAT64  *freq = NULL;
  INT32    nsam = 0;
  INT32    iteration = 0;
  INT32    i = 0;

  /* Compute Sample Rate (SR) */
  INT32 nSrate = 0;         // Sample Rate
  nSrate = m_nSrate;

  /* Default-Optionen */
  struct fopt fopt;
  fopt.fa    = nSrate;
  fopt.fmin  = 50;
  fopt.fmean  = 120;
  fopt.fmax  = 300;
  fopt.aopt.niterations  = 2;
  fopt.aopt.histfak    = 20.0;
  fopt.aopt.histpow    = 1.0;
  fopt.aopt.filtdiffint  = -1.0;
  fopt.aopt.diffdiffint  = -1.0;
  fopt.aopt.distback    = 0;

  topt=fopt2topt(fopt);


  /* bei Binaer-Daten nur Laenge der Datei auslesen */
  sam = samples;
  nsam = (INT32)nSamples;


  initfilthigh(&filt_high2,topt);


  times=(FLOAT64*)dlp_malloc(sizeof(FLOAT64)*nsam);
  freq=(FLOAT64*)dlp_malloc(sizeof(FLOAT64)*nsam);
  fsam=(FLOAT64*)dlp_malloc(sizeof(FLOAT64)*nsam);
  fsam_e=(FLOAT64*)dlp_malloc(sizeof(FLOAT64)*nsam);
  fsam_p=(FLOAT64*)dlp_malloc(sizeof(FLOAT64)*nsam);
  fsam_v=(char*)dlp_malloc(sizeof(char)*nsam);
  filt_r=(struct filt*)dlp_malloc(sizeof(struct filt)*(topt.tmax-topt.tmin+1));
  filt_i=(struct filt*)dlp_malloc(sizeof(struct filt)*(topt.tmax-topt.tmin+1));


  filthigh(fsam,sam,nsam,filt_high2);


  initfilt(filt_r,filt_i,topt);
  filt_freq=(struct filt*)dlp_malloc(sizeof(struct filt)*(topt.tmax-topt.tmin+1));
  initfiltfreq(filt_freq,topt);



  for(iteration=0;iteration<=topt.aopt.niterations;iteration++)
  {
    /* Frequenz bestimmen oder festlegen */
    if(!iteration) setfreq(times,nsam,topt.tmean); /* zuerst mit tmean initialisieren */
    else if(iteration==1)
    {
      FLOAT64 meanfreq;
      getperiods2(periods,nperiods,sam,fsam_p,fsam_v,nsam,topt);
      meanfreq=getmeanfreq(*periods,*nperiods)*1.2;
      setfreq(times,(INT32)nsam,(INT32)meanfreq);
      dlp_free(*periods);
    }
    else
    { /* dann aus fsam_p berechnen */
      getfreq_filt(times,fsam_p,nsam,filt_freq,topt); break;
    }

    for(i=0;i<nsam;i++) freq[i]=fopt.fa/times[i];


    /* Signal mit Frequenzfilter filtern */
    filtersignal(fsam_e,fsam_p,fsam,times,nsam,filt_r,filt_i,phase,topt);
    smoothabs(fsam_e,nsam);

    /* stimmhafte Bereiche bestimmen */
    getabsvoiceless(&abs_voiceless,fsam_e,nsam,topt);
    markvoiced2(fsam_v,fsam_e,nsam,abs_voiceless,topt);
    smoothvoiced(fsam_v,nsam,topt);

  }

  /* Periodenmarken ausgeben */
  getperiods2(periods,nperiods,sam,fsam_p,fsam_v,nsam,topt);



  dlp_free(fsam);
  dlp_free(fsam_e);
  dlp_free(fsam_p);
  dlp_free(fsam_v);
  dlp_free(freq);
  dlp_free(times);
  freefilt(filt_r,filt_i,topt);
  dlp_free(filt_i);
  dlp_free(filt_r);
  freefiltfreq(filt_freq,topt);
  dlp_free(filt_freq);
  dlp_free(filt_high2.a);
  dlp_free(filt_high2.b);
  //freefilthigh(&filt_high2);

  return O_K;
}


/*------------------------------------------------------------
  Function:        fopt2topt

  Optionen konvertieren
 ------------------------------------------------------------*/
struct topt CGEN_PRIVATE CPMproc::fopt2topt(struct fopt fopt)
{
  struct topt topt;
  topt.ta=(INT32)fopt.fa;
  topt.tmin = ROUND(fopt.fa/fopt.fmax );
  topt.tmean= ROUND(fopt.fa/fopt.fmean);
  topt.tmax = ROUND(fopt.fa/fopt.fmin );
  topt.aopt = fopt.aopt;
  return topt;
}


/*------------------------------------------------------------
  Function:        setfreq

  Frequenzen auf Mittelfrequenzen setzten
 ------------------------------------------------------------*/
void CGEN_PRIVATE CPMproc::setfreq(FLOAT64 *times,INT32 nsam,INT32 tmean)
{
  INT32 i=0;
  for(i=0;i<nsam;i++)
    times[i]=tmean;
}


/*------------------------------------------------------------
  Function:        filthigh

  H�henfilter anwenden
 ------------------------------------------------------------*/
void CGEN_PRIVATE CPMproc::filthigh(FLOAT64 *fsam,FLOAT64 *sam,INT32 nsam,struct filt filt)
{

  FILTER(filt,sam,nsam,fsam,0);

}




/*------------------------------------------------------------
  Function:        butterworth

  Butterworth-Filter 2.Ord erzeugen
 ------------------------------------------------------------*/
struct filt CGEN_PRIVATE CPMproc::butterworth(FLOAT64 dt)
{
  struct filt bw;
  FLOAT64 ak=0;
  bw.a=(FLOAT64*)dlp_malloc(sizeof(FLOAT64)*3);
  bw.b=(FLOAT64*)dlp_malloc(sizeof(FLOAT64)*3);
  bw.na=bw.nb=3;
  ak=sqrt(2.0); /* 2.0*cos(M_PI/4.0); */

  bw.a[0]=dt*dt + 2.0*ak*dt + 4.0;
  bw.a[1]=2.0*dt*dt - 8.0;
  bw.a[2]=dt*dt - 2.0*ak*dt + 4.0;
  bw.b[0]=dt*dt;
  bw.b[1]=2.0*dt*dt;
  bw.b[2]=dt*dt;

  /* Filter normieren */
  bw.b[2]/=bw.a[0];
  bw.b[1]/=bw.a[0];
  bw.b[0]/=bw.a[0];
  bw.a[2]/=bw.a[0];
  bw.a[1]/=bw.a[0];
  bw.a[0]=1.0;

  return bw;
}


/*------------------------------------------------------------
  Function:        initfilthigh

  Hochpass-Vorfilter erzeugen
 ------------------------------------------------------------*/
void CGEN_PRIVATE CPMproc::initfilthigh(struct filt *filt,struct topt topt)
{
  INT32 id=0;
  FLOAT64 power=0;
  if(topt.ta<12000){
    id=2; power=8000.0/(FLOAT64)topt.ta;
  }else if(topt.ta<20000){
    id=1; power=16000.0/(FLOAT64)topt.ta;
  }else{
    id=0; power=32000.0/(FLOAT64)topt.ta;
  }

  filt->a=(FLOAT64*)dlp_malloc(sizeof(FLOAT64)*5);
  filt->b=(FLOAT64*)dlp_malloc(sizeof(FLOAT64)*5);
  filt->na=filt->nb=3;

  filt->a[0]=1.0;
  filt->b[0]=pow(filt_high[id].b[0],power);
  filt->a[1]=-2.0*pow(-filt_high[id].a[1]/2.0,power);
  filt->b[1]=-2.0*pow(-filt_high[id].b[1]/2.0,power);
  filt->a[2]=filt->b[0]*filt->b[0];
  filt->b[2]=filt->b[0];

}

/*------------------------------------------------------------
  Function:        freefilthigh

  Hochpass-Vorfilter freigeben
 ------------------------------------------------------------*/
void CGEN_PRIVATE CPMproc::freefilthigh(struct filt *filt)
{
  dlp_free(filt->a);
  dlp_free(filt->b);
}

/*------------------------------------------------------------
  Function:        gfunc

 ------------------------------------------------------------*/
FLOAT64 CGEN_PRIVATE CPMproc::gfunc(FLOAT64 x)
{

  return cos(2*M_PI*x);

}


/*------------------------------------------------------------
  Function:        initfilt

  Frequenzfilter erzeugen
 ------------------------------------------------------------*/
void CGEN_PRIVATE CPMproc::initfilt(struct filt *filt_r,struct filt *filt_i,struct topt topt)
{
  INT32 t,i=0;
  INT32 tcur=0;
  FLOAT64 *a=(FLOAT64*)dlp_malloc(sizeof(FLOAT64));
  a[0]=1.0;
  for(t=0;t<=topt.tmax-topt.tmin;t++,filt_r++,filt_i++){
    tcur=topt.tmin+t;
    filt_r->na=filt_i->na=1;
    filt_r->a=filt_i->a=a;
    filt_r->nb=filt_i->nb=tcur;
    filt_r->b=(FLOAT64*)dlp_malloc(sizeof(FLOAT64)*tcur*2);
    filt_i->b=(FLOAT64*)dlp_malloc(sizeof(FLOAT64)*tcur*2);
    for(i=0;i<tcur*2;i++){
      filt_r->b[i]=gfunc((FLOAT64)i/(FLOAT64)tcur);
      filt_i->b[i]=gfunc((FLOAT64)i/(FLOAT64)tcur-0.25);
    }
  }
}

/*------------------------------------------------------------
  Function:        freefilt

  Frequenzfilter freigeben
 ------------------------------------------------------------*/
void CGEN_PRIVATE CPMproc::freefilt(struct filt *filt_r,struct filt *filt_i,struct topt topt)
{
  INT32 t=0;
  dlp_free(filt_r->a);
  for(t=0;t<=topt.tmax-topt.tmin;t++,filt_r++,filt_i++){
    dlp_free(filt_r->b);
    dlp_free(filt_i->b);
  }
}

/*------------------------------------------------------------
  Function:        initfiltfreq

  Filter f�r Phasenableitung erzeugen
 ------------------------------------------------------------*/
void CGEN_PRIVATE CPMproc::initfiltfreq(struct filt *filt,struct topt topt)
{
  INT32 t=0;
  /* generate Butterworth-Filter 2.Ord dt=f0/fa*5 */
  for(t=topt.tmin;t<=topt.tmax;t++)
    filt[t-topt.tmin]=butterworth(5.0/(FLOAT64)t);
}

/*------------------------------------------------------------
  Function:        freefiltfreq

  Filter f�r Phasenableitung freigeben
 ------------------------------------------------------------*/
void CGEN_PRIVATE CPMproc::freefiltfreq(struct filt *filt,struct topt topt)
{
  INT32 t=0;
  for(t=topt.tmin;t<=topt.tmax;t++){
    dlp_free(filt[t-topt.tmin].a);
    dlp_free(filt[t-topt.tmin].b);
  }
}



/*------------------------------------------------------------
  Function:        filtersignal

  Signal mit Frequenzfilter filtern
 ------------------------------------------------------------*/
void CGEN_PRIVATE CPMproc::filtersignal(FLOAT64 *fsam_e,FLOAT64 *fsam_p,FLOAT64 *sam,FLOAT64 *times,INT32 nsam,struct filt *filt_r,struct filt *filt_i,FLOAT64 phase,struct topt topt)
{
  INT32 i=0;
  INT32 tcur=0;
  FLOAT64 xr,xi=0;
  struct filt fr;
  struct filt fi;
  INT32 phase_offset=0;
  for(i=0;i<nsam;i++){
    tcur=TOPT(ROUND(times[i]));
    phase_offset=(INT32)((phase/M_PI/2.0+0.5)*(FLOAT64)tcur);
    while(phase_offset<0) phase_offset+=tcur;
    phase_offset%=tcur;
    fr=filt_r[tcur-topt.tmin];
    fi=filt_i[tcur-topt.tmin];
    fr.b+=phase_offset;
    fi.b+=phase_offset;
    FILTER(fr,sam+i,1,&xr,i);
    FILTER(fi,sam+i,1,&xi,i);
    xr/=MIN(i+1,tcur);
    xi/=MIN(i+1,tcur);
    if(fsam_e) fsam_e[i]=sqrt(xr*xr+xi*xi);
    if(fsam_p) fsam_p[i]=atan2(xi,xr);
  }
}


/*------------------------------------------------------------
  Function:        smoothabs

 ------------------------------------------------------------*/
void CGEN_PRIVATE CPMproc::smoothabs(FLOAT64* fsam_e, INT32 nsam)
{
  FLOAT64  filt_a[21] = { +1.000000000, -0.090083402, -0.147701902,
    -0.152230595, -0.156850495, -0.008113369, -0.135820630,
    -0.077455698, -0.085926469, -0.082876414, -0.000845939,
    -0.015919733, +0.015798032, -0.001951903, -0.055996457,
    -0.123836066, +0.009640244, +0.112471222, +0.071679452,
    +0.010604710, -0.001834216 };
  FLOAT64  filt_b[1] = { 1.0 };
  FLOAT64* fsam_e_smooth = (FLOAT64*)dlp_calloc(nsam, sizeof(FLOAT64));
  INT32     i;
  filter(filt_b, 1, filt_a, 21, fsam_e, nsam, fsam_e_smooth, 0);
  for(i = 0; i < nsam; i++) {
    *(fsam_e + i) = *(fsam_e_smooth + i) / 8.0;
  }
  dlp_free(fsam_e_smooth);
}


/*------------------------------------------------------------
  Function:        filter

  FILTER_H_
 ------------------------------------------------------------*/
INT32 CGEN_PRIVATE CPMproc::filter(FLOAT64* b, INT32 nb, FLOAT64* a, INT32 na, FLOAT64* input, INT32 nInput, FLOAT64* output, INT32 have_mem)
{
  INT32      iInput    = 0;
  INT32      ia        = 0;
  INT32      ib        = 0;
  INT32      na1       = 0;
  INT32      nb1       = 0;
  FLOAT64*  p1_input  = NULL;
  FLOAT64*  p2_input  = NULL;
  FLOAT64*  p1_output = NULL;
  FLOAT64*  p2_output = NULL;
  FLOAT64*  p_a       = NULL;
  FLOAT64*  p_b       = NULL;

  a++;
  na--;

  p_b = b;
  p1_input = input;
  p1_output = output;
  for(iInput = 0; iInput < nInput; iInput++, p1_output++, p1_input++, p_b = b) {
    *p1_output = 0;
    p2_input   = p1_input;
    nb1 = MIN(nb, iInput + have_mem + 1);
    for(ib = 0; ib < nb1; ib++, p2_input--, p_b++) {
      *p1_output += *p2_input * *p_b;
    }
  }

  p_a = a;
  p1_output = output;
  for(iInput = 0; iInput < nInput; iInput++, p1_output++, p_a = a) {
    p2_output = p1_output - 1;
    na1 = MIN(na, iInput + have_mem);
    for(ia = 0; ia < na1; ia++, p2_output--, p_a++) {
      *p1_output -= *p2_output * *p_a;
    }
  }
  return EXIT_SUCCESS;
}


/*------------------------------------------------------------
  Function:        getfreq_diff

  Frequenz aus Phase durch Ableiten bestimmen
 ------------------------------------------------------------*/
void CGEN_PRIVATE CPMproc::getfreq_diff(FLOAT64 *times,FLOAT64 *sam_p,INT32 nsam,struct topt topt)
{
  INT32 i1,i2=0;
  INT32 diffint=0;
  FLOAT64 korr=0;
  FLOAT64 diff=0;
  for(i1=0;i1<nsam;i1++){
    if(topt.aopt.diffdiffint<0) diffint=ROUND(-times[i1]*topt.aopt.diffdiffint);
    else diffint=ROUND(topt.aopt.diffdiffint);
    diffint=MIN(diffint,nsam-i1-1);
    if(diffint==0){
      times[i1]=topt.tmean;
      continue;
    }
    korr=0.0;
    for(i2=i1+1;i2<=i1+diffint;i2++){
      if(sam_p[i2]-sam_p[i2-1]>1.5*M_PI) korr-=2.0*M_PI;
      if(sam_p[i2]-sam_p[i2-1]<-1.5*M_PI) korr+=2.0*M_PI;
    }
    diff=fabs(sam_p[i1+diffint]+korr-sam_p[i1]);
    times[i1]=2.0*M_PI*(FLOAT64)diffint/diff;
    times[i1]=TOPT(times[i1]);
    /*printf("%i %.3f\n",diffint,times[i1]);*/
  }
}


/*------------------------------------------------------------
  Function:        getfreq_filt

  Frequenz aus Phase durch Abstand zur n�chsten Periode bestimmen
 ------------------------------------------------------------*/
void CGEN_PRIVATE CPMproc::getfreq_filt(FLOAT64 *times,FLOAT64 *sam_p,INT32 nsam,struct filt *filt,struct topt topt)
{
  INT32 i=0;
  FLOAT64 *sam_w=(FLOAT64*)dlp_malloc(sizeof(FLOAT64)*nsam);
  dlp_memmove(sam_w,times,sizeof(FLOAT64)*nsam);
  getfreq_diff(sam_w,sam_p,nsam,topt);
  FILTER(filt[topt.tmean-topt.tmin],sam_w,nsam,times,0);
  for(i=0;i<nsam;i++) times[i]=TOPT(times[i]);
  dlp_free(sam_w);
}


/*------------------------------------------------------------
  Function:        getmeanfreq

  Mittlere Frequenz der stimmhaften Bereiche bestimmmen
 ------------------------------------------------------------*/
FLOAT64 CGEN_PRIVATE CPMproc::getmeanfreq(struct PeriodPM *periods,INT32 nperiods)
{
  INT32 i=0;
  INT32 psum=0;
  INT32 pcount=0;
  for(i=0;i<nperiods;i++) if(periods[i].stimulation){
    psum+=periods[i].period;
    pcount++;
  }
  return (FLOAT64)psum/(FLOAT64)pcount;
}


/*------------------------------------------------------------
  Function:        getperiods2

 ------------------------------------------------------------*/
FLOAT64 CGEN_PRIVATE CPMproc::getperiods2(struct PeriodPM **periods,INT32 *nperiods,FLOAT64 *sam,FLOAT64 *sam_p,char *sam_vin,INT32 nsam,struct topt topt)
{
  INT32 i1,i2,ir=0;
  INT32 p1,p2,plast,pback,pres=0;
  INT32 pe1,pe2=0;
  INT32 *times;
  INT32 told=topt.tmean;
  INT32 tnext=0;
  INT32 tcur=0;
  struct PeriodPM *pers=NULL;
  INT32 npers=0;
  char *sam_v=(char*)dlp_malloc(sizeof(char)*nsam);
  dlp_memmove(sam_v,sam_vin,sizeof(char)*nsam);
  times=(INT32*)dlp_malloc(sizeof(INT32)*nsam);
  dlp_memset(times,0,sizeof(INT32)*nsam);
  /* stimmlose Anteile setzen */
  for(p1=0;p1<nsam;) if(sam_v[p1]){
    for(; p1<nsam && fabs(sam_p[p1]-sam_p[p1-1])<1.8*M_PI ; p1++) sam_v[p1]=0;
    plast=pback=-1;
    p2=p1;
    while(1){
      for(p2++; p2<nsam && sam_v[p2] && fabs(sam_p[p2]-sam_p[p2-1])<1.8*M_PI ;) p2++;
      if(p2>=nsam || !sam_v[p2]) break;
      /* tmax-tmin + r�cksprung */
      if(sam_p[p2]-sam_p[p2-1]>0){
        if(plast>=0) pback=p2;
        continue;
      }
      /* Abstand speichern */
      if(pback>=0){
        INT32 pnew=(p2+p1)/2;
        times[plast]=pnew-plast;
        p1=pnew;
        pback=-1;
      }else{
        times[p1]=p2-p1;
        plast=p1;
        p1=p2;
      }
    }
    for(; p1<nsam && p1<=p2 ;p1++) sam_v[p1]=0;
  }else p1++;
  /* zu kurze Perioden rausgl�tten */
  for(p1=plast=0;p1<nsam;) if(sam_v[p1]){
    if(times[p1]<topt.tmin && sam_v[p1+times[p1]]){
      if(plast && sam_v[p1+times[p1]+times[p1+times[p1]]]){
        INT32 pnew;
        p1+=times[p1];
        p1+=times[p1];
        pnew = (plast+p1)/2; /* Mitte von vorheriger und n�chster Marke */
        times[plast]=pnew-plast;
        times[pnew]=p1-pnew;
        plast=pnew;
        p1=pnew;
      }else{
        times[p1]+=times[p1+times[p1]]; /* zweite Marke nehmen */
        plast=p1;
        p1+=times[p1];
      }
    }else{
      plast=p1; /* nix ver�ndern */
      p1+=times[p1];
    }
  }else{ p1++; plast=0; }
  /* Signalminima suchen */
  FLOAT64 phasesumx=0.0, phasesumy=0.0;
  INT32 palt = 0;
  INT32 pnew = -1;
  INT32 pintstart = 0;
  INT32 pintend = 0;
  INT32 intlenalt = 0;;
  INT32 *timesnew=(INT32*)dlp_calloc(sizeof(INT32),nsam);
  char *sam_vnew=(char*)dlp_malloc(sizeof(char)*nsam);
  dlp_memmove(sam_vnew,sam_v,sizeof(char)*nsam);
  for(palt=0,pnew=-1;palt<nsam;) if(sam_v[palt]){
    INT32 i;
    INT32 pmin=0;
    FLOAT64 min=1000.0;
    intlenalt = times[palt];
    pintstart = pnew>=0 ? pintend : palt-intlenalt/2;
    pintend = ( pnew>=0 ? pnew+timesnew[pnew] : palt ) + intlenalt/2;
    if(pintend>palt+intlenalt+intlenalt/2) pintend=palt+intlenalt+intlenalt/2;
    if(pintend<=pintstart){ palt+=times[palt]; continue; }
    for(i=pintstart;i<pintend;i++) if(i>=0 && i<nsam){
      FLOAT64 val=sam[i];
      INT32 j;
      for(j=0;j>-10;j--) if(i+j>=0) val+=sam[i+j]*(FLOAT64)(10+j)/2.0;
      if(val<min){ min=val; pmin=i; }
    }
    phasesumx+=sin(sam_p[pmin]); phasesumy+=cos(sam_p[pmin]);
    if(pnew>=0){
      timesnew[pnew]=pmin-pnew;
      pnew=pmin;
      timesnew[pnew]=times[palt];
      sam_vnew[pnew]=1;
      if(pnew+timesnew[pnew]<nsam)
        sam_vnew[pnew+timesnew[pnew]]=0;
    }else{
      pnew=pmin;
      timesnew[pnew]=times[palt];
      if(pnew<palt) for(i=pnew;i<palt;i++) sam_vnew[i]=1;
      if(pnew>palt) for(i=palt;i<pnew;i++) sam_vnew[i]=0;
    }
    palt+=times[palt];
  }else{
    if(pnew>=0){ pnew+=timesnew[pnew]; while(pnew<palt) sam_vnew[pnew++]=0; }
    palt++;
    pnew=-1;
    pintend=0;
  }
  dlp_memmove(times,timesnew,sizeof(INT32)*nsam);
  dlp_memmove(sam_v,sam_vnew,sizeof(char)*nsam);
  dlp_free(timesnew);
  dlp_free(sam_vnew);
  /* stimmlose Anteile als �bergang  und  Perioden speichern */
  for(p1=0;p1<nsam;) if(sam_v[p1]){
    pers=(struct PeriodPM*)dlp_realloc(pers,(npers+1),sizeof(struct PeriodPM));
    pers[npers].period=times[p1]+pres;
    pers[npers].stimulation=1;
    pres=0;
    npers++;
    p1+=times[p1];
  }else{
    for(p2=p1+1; p2<nsam && !sam_v[p2] ;) p2++;
    if(p2==nsam) tnext=told;
    else tnext=times[p2];
    /* Anfang:p1,told Ende:p2,tnext */
    for(i1=pe2=0; i1<p2-p1 ;i1+=tcur,pe2++){
      tcur=(INT32)(told+(FLOAT64)(tnext-told)*(FLOAT64)i1/(FLOAT64)(p2-p1));
      pers=(struct PeriodPM*)dlp_realloc(pers,(npers+pe2+1),sizeof(struct PeriodPM));
      pers[npers+pe2].period=tcur;
      pers[npers+pe2].stimulation=0;
    }
    ir=(p2-p1)-i1; /* Rest verteilen */
    for(pe1=0;pe1<pe2;pe1++){
      i1=ROUND((FLOAT64)ir*((FLOAT64)pe1-0.5)/(FLOAT64)pe2);
      i2=ROUND((FLOAT64)ir*((FLOAT64)pe1+0.5)/(FLOAT64)pe2);
      pers[npers+pe1].period+=i2-i1;
    }
    npers+=pe2;
    p1=p2;
  }
  dlp_free(times);
  dlp_free(sam_v);
  *nperiods=npers;
  *periods=pers;
  return atan2(phasesumy,phasesumx);
}


/*------------------------------------------------------------
  Function:        getabsvoiceless

  Grenzenergie f�r stimmlose Bereiche bestimmen
 ------------------------------------------------------------*/
void CGEN_PRIVATE CPMproc::getabsvoiceless(FLOAT64 *abs_voiceless,FLOAT64 *sam_e,INT32 nsam,struct topt topt)
{
  INT32 i=0;
  FLOAT64 min_e=1.0,max_e=0.0;
  FLOAT64 hist_e[HIST_SIZE+1];
  FLOAT64 min,max=0;
  FLOAT64 min_ii=0;
  INT32 maxi=0;
  /* Aussteuerungsbereich bestimmen */
  for(i=0;i<nsam;i++){
    if(sam_e[i]<min_e) min_e=sam_e[i];
    if(sam_e[i]>max_e) max_e=sam_e[i];
  }
  /* Histogramm berechnen */
  dlp_memset(hist_e,0,(HIST_SIZE+1)*sizeof(FLOAT64));
  for(i=0;i<nsam;i++) hist_e[(INT32)((sam_e[i]-min_e)/(max_e-min_e)*(FLOAT64)HIST_SIZE)]++;
  /* H�ufigkeitsmaximum suchen */
  max=hist_e[0]; maxi=0;
  for(i=1;i<=HIST_SIZE;i++) if(hist_e[i]>max){ max=hist_e[i]; maxi=i; }

  /* k�rzesten Abstand zu Ursprung bestimmen */
  min=1.0;
  for(i=maxi;i<=HIST_SIZE;i++){
    FLOAT64 ii=(FLOAT64)i/(FLOAT64)HIST_SIZE;
    hist_e[i] *= topt.aopt.histfak/max/pow((FLOAT64)i,topt.aopt.histpow); /* Normieren */

    hist_e[i] = sqrt( hist_e[i]*hist_e[i] + ii*ii ); /* Abstand zu Ursprung */
    if(min>hist_e[i]){
      min=hist_e[i];
      min_ii=ii;
    }

  }

  /* Energieausgabe */
  *abs_voiceless=min_ii*(max_e-min_e)+min_e;
}


/*------------------------------------------------------------
  Function:        markvoiced2

 ------------------------------------------------------------*/
void CGEN_PRIVATE CPMproc::markvoiced2(char *sam_v,FLOAT64 *sam_e,INT32 nsam,FLOAT64 abs_voiceless,struct topt topt)
{
  INT32 i,j=0;
  FLOAT64 sum=0;
  FLOAT64 count=0;
  for(i=0;i<nsam;i++){
    sum=count=0.0;
    for(j=i-topt.tmax/2;j<=i+topt.tmax/2;j++) if(j>=0 && j<nsam){
      sum+=sam_e[j];
      count++;
    }
    sam_v[i]=sum/count>abs_voiceless*0.5;
  }
}


/*------------------------------------------------------------
  Function:        smoothvoiced

  Stimmlosmarkierung gl�tten
 ------------------------------------------------------------*/
void CGEN_PRIVATE CPMproc::smoothvoiced(char *sam_v,INT32 nsam,struct topt topt)
{
  INT32 p1,p2,i1=0;
  for(p1=0;p1<nsam;p1=p2){
    /* Suche nach Abschnitt mit Mindesl�nge tmax und gleicher sam_v */
    for(p2=i1=p1; i1<nsam && i1-p2<topt.tmax ;){
      while( i1<nsam && sam_v[i1]==sam_v[p1] ) i1++;
      p2=i1;
      while( i1<nsam && sam_v[i1]!=sam_v[p1] ) i1++;
    }
    /* abweichende sam_v im Abschnitt anpassen */
    for(i1=p1+1;i1<p2;i1++) sam_v[i1]=sam_v[p1];
  }
}



/* EOF */
