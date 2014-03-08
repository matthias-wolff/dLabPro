/*  header fuer subroutinen der library ipkclib

  F. Schiel  15.11.90/15.12.92        */

/* 20131203 Matthias Wolff: removed all unused defines and declarations */

#ifndef IPKCLIB
#define IPKCLIB

#ifdef __cplusplus
extern "C" {
#endif

#ifndef FILE
# include <stdio.h>
#endif 
#ifndef SEEK_SET 
# include <unistd.h>
#endif 


/*######################################################################*/

/* Type-Definitionen die von mehr als einem Modul verwendet werden, bzw. in 
   Aufrufen von Funktionen der Library auftauchen in alphabetischer 
   Reihenfolge mit Angabe der Herkunft          */
/* -- removed -- */

/* Phon_header_2 HEADER */
typedef struct {
                long    not_used1[5],
                        nspbk,          /* # of data blocks (512 bytes) */
                        anz_header,     /* # of header blocks (512 bytes)
                                           allways odd !                */
                        not_used2[5];
                char    sprk[2];        /* speaker id                   */
                short   swdh;           /* session repetition           */
                long    ifld1[3],       /* ILS                          */
                        not_used3[6];
                char    kenn1[2];       /* ILS text characters 1 - 8    */
                short   not_used4;
                char    kenn2[2];
                short   not_used5;
                char    kenn3[2];
                short   not_used6;
                char    kenn4[2];
                short   not_used7;
                long    not_used8[35],
                        isf,            /* sampling rate in Hz          */
                        flagtype,       /* ILS: -32000 if sampling file
                                                -29000 if param. file   */
                        flaginit;       /* ILS: 32149 if init           */
                char    ifl[32],        /* filename (terminated by /0)  */
                        day,            /* # of day                     */
                        month;          /* # of month                   */
                short   year;           /* # of year                    */
                char    sex,            /* sex of speaker               */
                        version;        /* header version:
                                           0 = extended ILS
                                           1 = phondat version 1
                                           2 = phondat version 2        */
                short   adc_bits,       /* resolution of adc            */
                        words;          /* # of words in text           */
                long    not_useda[50];
                short   wdh,            /* # of repetition              */
                        abs_ampl;       /* maximum of amplitude         */
                char    not_usedb[10];
                } Phon_header_2;


/* #####################################################################*/

/* funktionsprototypen  (nach Modulen geordnet)        */

/* ABSTAND    */
/* -- removed -- */

/* AUDIO    */
/* -- removed -- */

/* CLUSTER_UTI    */
/* -- removed -- */

/* DATA                  */
/* -- removed -- */

/* HEADER               */
Phon_header_2 *read_header_2_vms(FILE *phon_file,char **ortho,char **cano);
Phon_header_2 *read_header_2_sun(FILE *phon_file,char **ortho,char **cano);
Phon_header_2 *read_header_1_sun(FILE *phon_file);
Phon_header_2 *read_header_1_vms(FILE *phon_file);
Phon_header_2 *read_header_sun(FILE *phon_file,char **ortho,char **cano);
Phon_header_2 *read_header_vms(FILE *phon_file,char **ortho,char **cano);
int write_header_1_sun(FILE *phon_file,Phon_header_2 *header);
int write_header_1_vms(FILE *phon_file,Phon_header_2 *header);
int write_header_sun(FILE *phon_file,Phon_header_2 *header,char *ortho,
                     char *cano);
int write_header_vms(FILE *phon_file,Phon_header_2 *header,char *ortho,
                     char *cano);
void disp_header_v2(Phon_header_2 *header);
void longvms2sun(long *l);
void shortvms2sun(short *s);

/* ICSIARGS    */
/* -- removed -- */

/* IOUTIL    */
/* -- removed -- */

/* LABEL    */
/* -- removed -- */

/* LINUXAUDIO         */
/* -- removed -- */

/* MATRIX    */
/* -- removed -- */

/* PREPROCESS */
/* -- removed -- */

/* STATISTIK    */
/* -- removed -- */

/* STROP */
/* -- removed -- */

/* TIEFPASS    */
/* -- removed -- */

/* VEKMATH    */
/* -- removed -- */

#ifdef __cplusplus
}
#endif

#endif /* #ifndef IPKCLIB */

