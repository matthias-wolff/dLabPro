/***********************************************************************
************************************************************************

Module    : HEADER
Title    : working with PhonDat headers

Author    : F. Schiel
Date/revision   : 30.03.93 / 11.01,96 alpha

Description  : 
Functions for reading, writing, displaying PhonDat headers.
You need an ANSI C compiler to use this module.
A special funktion is required to read to original Phondat files on a UNIX 
mashine, because long and short have a different format to VMS mashines.
Functions *_sun are for Motorola architectures (Sun, Apollo, HP, NeXT),
functions *_vms for Intel architectures (DOS, DEC).

Fixed Bugs:
- The dynamic buff buff was not freed before leaving the functions 
without an error.
11.01.96 : In the (seldom) case that the header information of
           a PhonDat 2 Header is exactly a multiple of 512, an extra
           block of 512 \0 are written after the header section.
           Then the number of total blocks in the file is higher than
           given in the header and at the beginning of the speech
           section are 256 samples of absolute silence.
           Bug fixed for all writing functions.

Link modules and libraries:


Contained functions:
read_header_2_vms  : reads information of PhonDat header version 2
read_header_2_sun  : reads information of PhonDat header version 2
read_header_1_sun  : reads information of PhonDat header version 1 
read_header_1_vms  : reads information of PhonDat header version 1 
read_header_vms    : reads information of PhonDat header 
read_header_sun    : reads information of PhonDat header 
write_header_sun  : writes information of PhonDat header 
write_header_vms  : writes information of PhonDat header 
write_header_1_sun  : writes information of PhonDat header version 2
                          (only fixed part) = PhonDat header 1
write_header_1_vms  : writes information of PhonDat header version 2
                          (only fixed part) = PhonDat header 1
disp_header_v2    : displays content of structure Phon_header_2

*************************************************************************/
#include "dlp_cscope.h"
#include "dlp_base.h"
#include "dlp_object.h"
#include "dlp_alloc_extern.h"
#include "ipkclib.h"

/* DEFINES, only used within this module ********************************/
# define FIXEDHEADERLENGTH 512    /* length of header with fix infos*/

/*----------------------------------------------------------------------
Name            : read_header_2_vms
Module          : HEADER
Title           : reads information of header of PhonDat file version 2

Description:
Sets pointer to beginning of file, reads all relevant 
information in header version 
2 (if not version 2 returns error), creates structure Phon_header_2 with 
fixed information (first 512 bytes of header), returns pointer to this 
structure, reads orthographic and canonical informations (2nd and following 
512 blocks), writes pointer to strings with orthographic and canonical 
information into adresses ortho and cano, positions the file pointer to the 
first byte after the header blocks.
ATTENTION: The string elements in structure Phon_header_2 are NOT 
terminated by '\0' except the element ifl.
The memory of structure and strings can be set free by using free() to 
corresponding pointers.

Parameters:
phon_file  : File pointer to Phondat file version 2 
ortho    : orthographic information
cano    : canonical information
    : 

Return-Value    : pointer to structure with header information
      NULL, if error occurs
-------------------------------------------------------------------------*/
Phon_header_2 *read_header_2_vms(FILE *phon_file,char **ortho,char **cano)
{
  char *buff,*buffp,buffer[512];
  int len;
  Phon_header_2 *header;  /* pointer to header structure    */

/* rewind file to beginning            */
    rewind(phon_file);

/* allocate space for fixed header          */
    if((header = (Phon_header_2 *)calloc(1,sizeof(Phon_header_2))) == NULL)
    {
  fprintf(stderr,"read_header_2_vms: cannot allocate space for fixed header\n");
  perror("read_header_2_vms");
  *ortho = NULL;
  *cano = NULL;
  return(NULL);
    }

    if(fread(buffer,FIXEDHEADERLENGTH,1,phon_file) != 1)
    {
  fprintf(stderr,"read_header_2_vms: read error for 512 bytes (fixed header)\n");
  free((char *)header);
  *ortho = NULL;
  *cano = NULL;
  return(NULL);
    }

/* test, if header is version 2            */
    if(buffer[293] != 2)
    {
  fprintf(stderr,"read_header_2_vms: header is not PhonDat version 2\n");
  free((char *)header);
  *ortho = NULL;
  *cano = NULL;
  return(NULL);
    }

/* read items of Phondatheader in structure        */
    buffp = buffer + 20;
    header->nspbk = *((long*)buffp);
    buffp = buffer + 24;
    header->anz_header = *((long*)buffp);
    buffp = buffer + 48;
    header->sprk[0] = buffp[0];
    header->sprk[1] = buffp[1];
    buffp = buffer + 50;
    header->swdh = *((short*)buffp);
    buffp = buffer + 52;
    header->ifld1[0] = *((long*)buffp);
    buffp = buffer + 56;
    header->ifld1[1] = *((long*)buffp);
    buffp = buffer + 60;
    header->ifld1[2] = *((long*)buffp);
    buffp = buffer + 88;
    header->kenn1[0] = buffp[0];
    header->kenn1[1] = buffp[1];
    buffp = buffer + 92;
    header->kenn2[0] = buffp[0];
    header->kenn2[1] = buffp[1];
    buffp = buffer + 96;
    header->kenn3[0] = buffp[0];
    header->kenn3[1] = buffp[1];
    buffp = buffer + 100;
    header->kenn4[0] = buffp[0];
    header->kenn4[1] = buffp[1];
    buffp = buffer + 244;
    header->isf = *((long*)buffp);
    buffp = buffer + 248;
    header->flagtype = *((long*)buffp);
    buffp = buffer + 252;
    header->flaginit = *((long*)buffp);
    buffp = buffer + 256;
    strcpy(header->ifl,buffp);
    buffp = buffer + 288;
    header->day = *buffp;
    buffp = buffer + 289;
    header->month = *buffp;
    buffp = buffer + 290;
    header->year = *((short*)buffp);
    buffp = buffer + 292;
    header->sex = *buffp;
    buffp = buffer + 293;
    header->version = *buffp;
    buffp = buffer + 294;
    header->adc_bits = *((short*)buffp);
    buffp = buffer + 296;
    header->words = *((short*)buffp);
    buffp = buffer + 498;
    header->wdh = *((short*)buffp);
    buffp = buffer + 500;
    header->abs_ampl = *((short*)buffp);
    buffp = buffer + 502;
    strncpy(header->not_usedb,buffp,10);

/*if(iolog(0)) disp_header_v2(header);
*/

/* read following blocks with orthographic and canonical texts    */
    if((buff = (char *)calloc(header->anz_header - 1,512)) == NULL)
    {
  fprintf(stderr,"read_header_2_vms: cannot allocate space for rest of header\n");
  perror("read_header_2_vms");
  free((char *)header);
  *ortho = NULL;
  *cano = NULL;
  return(NULL);
    }
    if(fread(buff,512,header->anz_header - 1,phon_file) != 
          (size_t)(header->anz_header - 1))
    {
  fprintf(stderr,"read_header_2_vms: read error for dynamic part of header\n");
  free((char *)header);
  free((char *)buff);
  *ortho = NULL;
  *cano = NULL;
  return(NULL);
    }

/* find beginning of orthographic text, beginning marker = "ort\0"
          ending marker    = "\0oend::\0"  */
    buffp = buff;
    while(1)
    {
  if(strcmp(buffp,"ort") == 0) break;
  buffp++;
  if(buffp == buff + (header->anz_header - 1) * 512)
      {
    fprintf(stderr,
    "read_header_2_vms: can't find orthographic text\n");
    free((char *)buff);
    *ortho = NULL;
    *cano = NULL;
    return(header);
      }

    }    
    buffp += 4;

/*if(iolog(0)) printf("read_header_v2: orthographic text: %s\n",buffp);
*/
/* copy orthographic text in allocated space        */
    len = strlen(buffp);
    if((*ortho = (char *)calloc(1,len+1)) == NULL)
    {
  fprintf(stderr,
  "read_header_2_vms: cannot allocate space for orthographic text\n");
  perror("read_header_2_vms");
  free((char *)buff);
  *ortho = NULL;
  *cano = NULL;
  return(header);
    }
    strcpy(*ortho,buffp);
    buffp += len + 1;

/* find beginning of canonical text, beginning marker = "kan\0"
          ending marker    = "\0kend::\0"  */
    while(1)
    {
  if(strcmp(buffp,"kan") == 0) break;
  buffp++;
  if(buffp == buff + (header->anz_header - 1) * 512)
      {
    fprintf(stderr,
    "read_header_2_vms: can't find canonical text\n");
    free((char *)buff);
    *cano = NULL;
    return(header);
      }

    }    
    buffp += 4;

/*if(iolog(0)) printf("read_header_v2: canonical text: %s\n",buffp);
*/
/* copy canonical text in allocated space        */
    len = strlen(buffp);
    if((*cano = (char *)calloc(1,len+1)) == NULL)
    {
  fprintf(stderr,
  "read_header_2_vms: cannot allocate space for canonical text\n");
  perror("read_header_2_vms");
  free((char *)buff);
  *cano = NULL;
  return(header);
    }
    strcpy(*cano,buffp);

    free((char *)buff);
    return(header);
 
} /* end subroutine : read_header_2_vms  */

/*----------------------------------------------------------------------
Name            : read_header_2_sun
Module          : HEADER
Title           : reads information of header of PhonDat file version 2

Description:
Sets pointer to beginning of file, reads all relevant 
information in header version 
2 (if not version 2 returns error), creates structure Phon_header_2 with 
fixed information (first 512 bytes of header), returns pointer to this 
structure, reads orthographic and canonical informations (2nd and following 
512 blocks), writes pointer to strings with orthographic and canonical 
information into adresses ortho and cano, positions the file pointer to the 
first byte after the header blocks.
ATTENTION: The string elements in structure Phon_header_2 are NOT 
terminated by '\0' except the element ifl.
The memory of structure and strings can be set free by using free() to 
corresponding pointers.

Parameters:
phon_file  : File pointer to Phondat file version 2 
ortho    : orthographic information
cano    : canonical information
    : 

Return-Value    : pointer to structure with header information
      NULL, if error occurs
-------------------------------------------------------------------------*/
Phon_header_2 *read_header_2_sun(FILE *phon_file,char **ortho,char **cano)
{
  char *buff,*buffp,buffer[512];
  int len;
  Phon_header_2 *header;  /* pointer to header structure    */

/* rewind file to beginning            */
    rewind(phon_file);

/* allocate space for fixed header          */
    if((header = (Phon_header_2 *)calloc(1,sizeof(Phon_header_2))) == NULL)
    {
  fprintf(stderr,"read_header_2_sun: cannot allocate space for fixed header\n");
  perror("read_header_2_sun");
  *ortho = NULL;
  *cano = NULL;
  return(NULL);
    }

    if(fread(buffer,FIXEDHEADERLENGTH,1,phon_file) != 1)
    {
  fprintf(stderr,"read_header_2_sun: read error for 512 bytes (fixed header)\n");
  free((char *)header);
  *ortho = NULL;
  *cano = NULL;
  return(NULL);
    }

/* test, if header is version 2            */
    if(buffer[293] != 2)
    {
  fprintf(stderr,"read_header_2_sun: header is not PhonDat version 2\n");
  free((char *)header);
  *ortho = NULL;
  *cano = NULL;
  return(NULL);
    }

/* read items of Phondatheader in structure        */
    buffp = buffer + 20;
    header->nspbk = *((long*)buffp);
    longvms2sun(&(header->nspbk));
    buffp = buffer + 24;
    header->anz_header = *((long*)buffp);
    longvms2sun(&(header->anz_header));
    buffp = buffer + 48;
    header->sprk[0] = buffp[0];
    header->sprk[1] = buffp[1];
    buffp = buffer + 50;
    header->swdh = *((short*)buffp);
    shortvms2sun(&(header->swdh));
    buffp = buffer + 52;
    header->ifld1[0] = *((long*)buffp);
    longvms2sun(&(header->ifld1[0]));
    buffp = buffer + 56;
    header->ifld1[1] = *((long*)buffp);
    longvms2sun(&(header->ifld1[1]));
    buffp = buffer + 60;
    header->ifld1[2] = *((long*)buffp);
    longvms2sun(&(header->ifld1[2]));
    buffp = buffer + 88;
    header->kenn1[0] = buffp[0];
    header->kenn1[1] = buffp[1];
    buffp = buffer + 92;
    header->kenn2[0] = buffp[0];
    header->kenn2[1] = buffp[1];
    buffp = buffer + 96;
    header->kenn3[0] = buffp[0];
    header->kenn3[1] = buffp[1];
    buffp = buffer + 100;
    header->kenn4[0] = buffp[0];
    header->kenn4[1] = buffp[1];
    buffp = buffer + 244;
    header->isf = *((long*)buffp);
    longvms2sun(&(header->isf));
    buffp = buffer + 248;
    header->flagtype = *((long*)buffp);
    longvms2sun(&(header->flagtype));
    buffp = buffer + 252;
    header->flaginit = *((long*)buffp);
    longvms2sun(&(header->flaginit));
    buffp = buffer + 256;
    strcpy(header->ifl,buffp);
    buffp = buffer + 288;
    header->day = *buffp;
    buffp = buffer + 289;
    header->month = *buffp;
    buffp = buffer + 290;
    header->year = *((short*)buffp);
    shortvms2sun(&(header->year));
    buffp = buffer + 292;
    header->sex = *buffp;
    buffp = buffer + 293;
    header->version = *buffp;
    buffp = buffer + 294;
    header->adc_bits = *((short*)buffp);
    shortvms2sun(&(header->adc_bits));
    buffp = buffer + 296;
    header->words = *((short*)buffp);
    shortvms2sun(&(header->words));
    buffp = buffer + 498;
    header->wdh = *((short*)buffp);
    shortvms2sun(&(header->wdh));
    buffp = buffer + 500;
    header->abs_ampl = *((short*)buffp);
    shortvms2sun(&(header->abs_ampl));
    buffp = buffer + 502;
    strncpy(header->not_usedb,buffp,10);

/*if(iolog(0)) disp_header_v2(header);
*/

/* read following blocks with orthographic and canonical texts    */
    if((buff = (char *)calloc(header->anz_header - 1,512)) == NULL)
    {
  fprintf(stderr,"read_header_2_sun: cannot allocate space for rest of header\n");
  perror("read_header_2_sun");
  free((char *)header);
  *ortho = NULL;
  *cano = NULL;
  return(NULL);
    }
    if(fread(buff,512,header->anz_header - 1,phon_file) != 
          (size_t)(header->anz_header - 1))
    {
  fprintf(stderr,"read_header_2_sun: read error for dynamic part of header\n");
  free((char *)header);
  free((char *)buff);
  *ortho = NULL;
  *cano = NULL;
  return(NULL);
    }

/* find beginning of orthographic text, beginning marker = "ort\0"
          ending marker    = "\0oend::\0"  */
    buffp = buff;
    while(1)
    {
  if(strcmp(buffp,"ort") == 0) break;
  buffp++;
  if(buffp == buff + (header->anz_header - 1) * 512)
      {
    fprintf(stderr,
    "read_header_2_sun: can't find orthographic text\n");
    free((char *)buff);
    *ortho = NULL;
    *cano = NULL;
    return(header);
      }

    }    
    buffp += 4;

/*if(iolog(0)) printf("read_header_v2: orthographic text: %s\n",buffp);
*/
/* copy orthographic text in allocated space        */
    len = strlen(buffp);
    if((*ortho = (char *)calloc(1,len+1)) == NULL)
    {
  fprintf(stderr,
  "read_header_2_sun: cannot allocate space for orthographic text\n");
  perror("read_header_2_sun");
  free((char *)buff);
  *ortho = NULL;
  *cano = NULL;
  return(header);
    }
    strcpy(*ortho,buffp);
    buffp += len + 1;

/* find beginning of canonical text, beginning marker = "kan\0"
          ending marker    = "\0kend::\0"  */
    while(1)
    {
  if(strcmp(buffp,"kan") == 0) break;
  buffp++;
  if(buffp == buff + (header->anz_header - 1) * 512)
      {
    fprintf(stderr,
    "read_header_2_sun: can't find canonical text\n");
    free((char *)buff);
    *cano = NULL;
    return(header);
      }

    }    
    buffp += 4;

/*if(iolog(0)) printf("read_header_v2: canonical text: %s\n",buffp);
*/
/* copy canonical text in allocated space        */
    len = strlen(buffp);
    if((*cano = (char *)calloc(1,len+1)) == NULL)
    {
  fprintf(stderr,
  "read_header_2_sun: cannot allocate space for canonical text\n");
  perror("read_header_2_sun");
  free((char *)buff);
  *cano = NULL;
  return(header);
    }
    strcpy(*cano,buffp);

    free((char *)buff);
    return(header);
 
} /* end subroutine : read_header_2_sun  */

/*----------------------------------------------------------------------
Name            : read_header_1_sun
Module          : HEADER
Title           : reads information of header of PhonDat file version 1

Description:
Sets pointer to beginning of file, reads all relevant 
information in header version 
2 (if not version 2 returns error), creates structure Phon_header_2 with 
fixed information (first 512 bytes of header), returns pointer to this 
structure.
Positions the file pointer to the first byte after the header blocks.
ATTENTION: The string elements in structure Phon_header_2 are NOT 
terminated by '\0' except the element ifl.
The memory of structure can be set free by using free() to 
corresponding pointer.

Parameters:
phon_file  : File pointer to Phondat file version 2 

Return-Value    : pointer to structure with header information
      NULL, if error occurs
-------------------------------------------------------------------------*/
Phon_header_2 *read_header_1_sun(FILE *phon_file)
{
  char *buffp,buffer[512];
  Phon_header_2 *header;  /* pointer to header structure    */

/* rewind file to beginning            */
    rewind(phon_file);

/* allocate space for fixed header          */
    if((header = (Phon_header_2 *)calloc(1,sizeof(Phon_header_2))) == NULL)
    {
  fprintf(stderr,"read_header_1_sun: cannot allocate space for fixed header\n");
  perror("read_header_1_sun");
  return(NULL);
    }

    if(fread(buffer,FIXEDHEADERLENGTH,1,phon_file) != 1)
    {
  fprintf(stderr,"read_header_1_sun: read error for 512 bytes (fixed header)\n");
  free((char *)header);
  return(NULL);
    }

/* test, if header is version 1            */
    if(buffer[293] != 1)
    {
  fprintf(stderr,"read_header_1_sun: header is not PhonDat version 1\n");
  free((char *)header);
  return(NULL);
    }

/* read items of Phondatheader in structure        */
    buffp = buffer + 20;
    header->nspbk = *((long*)buffp);
    longvms2sun(&(header->nspbk));
    buffp = buffer + 24;
    header->anz_header = *((long*)buffp);
    longvms2sun(&(header->anz_header));
    buffp = buffer + 48;
    header->sprk[0] = buffp[0];
    header->sprk[1] = buffp[1];
    buffp = buffer + 50;
    header->swdh = *((short*)buffp);
    shortvms2sun(&(header->swdh));
    buffp = buffer + 52;
    header->ifld1[0] = *((long*)buffp);
    longvms2sun(&(header->ifld1[0]));
    buffp = buffer + 56;
    header->ifld1[1] = *((long*)buffp);
    longvms2sun(&(header->ifld1[1]));
    buffp = buffer + 60;
    header->ifld1[2] = *((long*)buffp);
    longvms2sun(&(header->ifld1[2]));
    buffp = buffer + 88;
    header->kenn1[0] = buffp[0];
    header->kenn1[1] = buffp[1];
    buffp = buffer + 92;
    header->kenn2[0] = buffp[0];
    header->kenn2[1] = buffp[1];
    buffp = buffer + 96;
    header->kenn3[0] = buffp[0];
    header->kenn3[1] = buffp[1];
    buffp = buffer + 100;
    header->kenn4[0] = buffp[0];
    header->kenn4[1] = buffp[1];
    buffp = buffer + 244;
    header->isf = *((long*)buffp);
    longvms2sun(&(header->isf));
    buffp = buffer + 248;
    header->flagtype = *((long*)buffp);
    longvms2sun(&(header->flagtype));
    buffp = buffer + 252;
    header->flaginit = *((long*)buffp);
    longvms2sun(&(header->flaginit));
    buffp = buffer + 256;
    strcpy(header->ifl,buffp);
    buffp = buffer + 288;
    header->day = *buffp;
    buffp = buffer + 289;
    header->month = *buffp;
    buffp = buffer + 290;
    header->year = *((short*)buffp);
    shortvms2sun(&(header->year));
    buffp = buffer + 292;
    header->sex = *buffp;
    buffp = buffer + 293;
    header->version = *buffp;
    buffp = buffer + 294;
    header->adc_bits = *((short*)buffp);
    shortvms2sun(&(header->adc_bits));
    buffp = buffer + 296;
    header->words = *((short*)buffp);
    shortvms2sun(&(header->words));
    buffp = buffer + 498;
    header->wdh = *((short*)buffp);
    shortvms2sun(&(header->wdh));
    buffp = buffer + 500;
    header->abs_ampl = *((short*)buffp);
    shortvms2sun(&(header->abs_ampl));
    buffp = buffer + 502;
    strncpy(header->not_usedb,buffp,10);

    return(header);
 
} /* end subroutine : read_header_1_sun  */

/*----------------------------------------------------------------------
Name            : read_header_1_vms
Module          : HEADER
Title           : reads information of header of PhonDat file version 1

Description:
Sets pointer to beginning of file, reads all relevant 
information in header version 
2 (if not version 2 returns error), creates structure Phon_header_2 with 
fixed information (first 512 bytes of header), returns pointer to this 
structure.
Positions the file pointer to the first byte after the header blocks.
ATTENTION: The string elements in structure Phon_header_2 are NOT 
terminated by '\0' except the element ifl.
The memory of structure can be set free by using free() to 
corresponding pointer.

Parameters:
phon_file  : File pointer to Phondat file version 2 

Return-Value    : pointer to structure with header information
      NULL, if error occurs
-------------------------------------------------------------------------*/
Phon_header_2 *read_header_1_vms(FILE *phon_file)
{
  char *buffp,buffer[512];
  Phon_header_2 *header;  /* pointer to header structure    */

/* rewind file to beginning            */
    rewind(phon_file);

/* allocate space for fixed header          */
    if((header = (Phon_header_2 *)calloc(1,sizeof(Phon_header_2))) == NULL)
    {
  fprintf(stderr,"read_header_1_vms: cannot allocate space for fixed header\n");
  perror("read_header_1_vms");
  return(NULL);
    }

    if(fread(buffer,FIXEDHEADERLENGTH,1,phon_file) != 1)
    {
  fprintf(stderr,"read_header_1_vms: read error for 512 bytes (fixed header)\n");
  free((char *)header);
  return(NULL);
    }

/* test, if header is version 1            */
    if(buffer[293] != 1)
    {
  fprintf(stderr,"read_header_1_vms: header is not PhonDat version 1\n");
  free((char *)header);
  return(NULL);
    }

/* read items of Phondatheader in structure        */
    buffp = buffer + 20;
    header->nspbk = *((long*)buffp);
    buffp = buffer + 24;
    header->anz_header = *((long*)buffp);
    buffp = buffer + 48;
    header->sprk[0] = buffp[0];
    header->sprk[1] = buffp[1];
    buffp = buffer + 50;
    header->swdh = *((short*)buffp);
    buffp = buffer + 52;
    header->ifld1[0] = *((long*)buffp);
    buffp = buffer + 56;
    header->ifld1[1] = *((long*)buffp);
    buffp = buffer + 60;
    header->ifld1[2] = *((long*)buffp);
    buffp = buffer + 88;
    header->kenn1[0] = buffp[0];
    header->kenn1[1] = buffp[1];
    buffp = buffer + 92;
    header->kenn2[0] = buffp[0];
    header->kenn2[1] = buffp[1];
    buffp = buffer + 96;
    header->kenn3[0] = buffp[0];
    header->kenn3[1] = buffp[1];
    buffp = buffer + 100;
    header->kenn4[0] = buffp[0];
    header->kenn4[1] = buffp[1];
    buffp = buffer + 244;
    header->isf = *((long*)buffp);
    buffp = buffer + 248;
    header->flagtype = *((long*)buffp);
    buffp = buffer + 252;
    header->flaginit = *((long*)buffp);
    buffp = buffer + 256;
    strcpy(header->ifl,buffp);
    buffp = buffer + 288;
    header->day = *buffp;
    buffp = buffer + 289;
    header->month = *buffp;
    buffp = buffer + 290;
    header->year = *((short*)buffp);
    buffp = buffer + 292;
    header->sex = *buffp;
    buffp = buffer + 293;
    header->version = *buffp;
    buffp = buffer + 294;
    header->adc_bits = *((short*)buffp);
    buffp = buffer + 296;
    header->words = *((short*)buffp);
    buffp = buffer + 498;
    header->wdh = *((short*)buffp);
    buffp = buffer + 500;
    header->abs_ampl = *((short*)buffp);
    buffp = buffer + 502;
    strncpy(header->not_usedb,buffp,10);

    return(header);
 
} /* end subroutine : read_header_1_vms  */

/*----------------------------------------------------------------------
Name            : write_header_1_sun
Module          : HEADER
Title           : writes fixed information of header in file (version 1)

Description:
Writes the fixed information of header version 
1 (if not version 1 returns error) from structure Phon_header_2
(first 512 bytes of header). 
ATTENTION: The string elements in structure Phon_header_2 are NOT 
terminated by '\0' except the element ifl.

Parameters:
phon_file  : File pointer to Phondat file version 1
header          : pointer to structure with header information

Return-Value    : 1 : writing successful
                  0 : error during write
-------------------------------------------------------------------------*/
int write_header_1_sun(FILE *phon_file,Phon_header_2 *header)
{
  char buffer[512];
        short s;
  int i;
  long l;


    for(i=0;i<512;i++) buffer[i] = '\0';

    if(header == NULL) 
      {
      fprintf(stderr,"write_header_1_sun: header structure empty\n");
      return(0);
      } 

/* test, if header is version 1            */
    if(header->version != 1)
    {
  fprintf(stderr,"write_header_1_sun: header is not PhonDat version 1\n");
  return(0);
    }

   if(header->anz_header != 1) header->anz_header = 1;

/* write items of Phondatheader in file, not used items are padded with zero */
    if(fwrite(buffer,sizeof(char),20,phon_file) != 20)
      {
      fprintf(stderr,"write_header_1_sun: cannot write first item\n");
      return(0);
      }

    l = header->nspbk;
    longvms2sun(&l);
    if(fwrite(&l,sizeof(long),1,phon_file)!=1) return(0);
    l = header->anz_header;
    longvms2sun(&l);
    if(fwrite(&l,sizeof(long),1,phon_file)!=1) return(0);
    if(fwrite(buffer,sizeof(char),20,phon_file)!=20) return(0);
    if(fwrite(header->sprk,sizeof(char),2,phon_file)!=2) return(0);
    s = header->swdh;
    shortvms2sun(&s);
    if(fwrite(&s,sizeof(short),1,phon_file)!=1) return(0);
    l = header->ifld1[0];
    longvms2sun(&l);
    if(fwrite(&l,sizeof(long),1,phon_file)!=1) return(0);
    l = header->ifld1[1];
    longvms2sun(&l);
    if(fwrite(&l,sizeof(long),1,phon_file)!=1) return(0);
    l = header->ifld1[2];
    longvms2sun(&l);
    if(fwrite(&l,sizeof(long),1,phon_file)!=1) return(0);
    if(fwrite(buffer,sizeof(char),24,phon_file)!=24) return(0);
    if(fwrite(header->kenn1,sizeof(char),2,phon_file)!=2) return(0);
    if(fwrite(buffer,sizeof(char),2,phon_file)!=2) return(0);
    if(fwrite(header->kenn2,sizeof(char),2,phon_file)!=2) return(0);
    if(fwrite(buffer,sizeof(char),2,phon_file)!=2) return(0);
    if(fwrite(header->kenn3,sizeof(char),2,phon_file)!=2) return(0);
    if(fwrite(buffer,sizeof(char),2,phon_file)!=2) return(0);
    if(fwrite(header->kenn4,sizeof(char),2,phon_file)!=2) return(0);
    if(fwrite(buffer,sizeof(char),142,phon_file)!=142) return(0);
    l = header->isf;
    longvms2sun(&l);
    if(fwrite(&l,sizeof(long),1,phon_file)!=1) return(0);
    l = header->flagtype;
    longvms2sun(&l);
    if(fwrite(&l,sizeof(long),1,phon_file)!=1) return(0);
    l = header->flaginit;
    longvms2sun(&l);
    if(fwrite(&l,sizeof(long),1,phon_file)!=1) return(0);
    if(fwrite(header->ifl,sizeof(char),strlen(header->ifl),phon_file)!=strlen(header->ifl)) return(0);
    if(fwrite(buffer,sizeof(char),32-strlen(header->ifl),phon_file)!=32-strlen(header->ifl)) return(0);
    if(fwrite(&(header->day),sizeof(char),1,phon_file)!=1) return(0);
    if(fwrite(&(header->month),sizeof(char),1,phon_file)!=1) return(0);
    s = header->year;
    shortvms2sun(&s);
    if(fwrite(&s,sizeof(short),1,phon_file)!=1) return(0);
    if(fwrite(&(header->sex),sizeof(char),1,phon_file)!=1) return(0);
    if(fwrite(&(header->version),sizeof(char),1,phon_file)!=1) return(0);
    s = header->adc_bits;
    shortvms2sun(&s);
    if(fwrite(&s,sizeof(short),1,phon_file)!=1) return(0);
    s = header->words;
    shortvms2sun(&s);
    if(fwrite(&s,sizeof(short),1,phon_file)!=1) return(0);
    if(fwrite(buffer,sizeof(char),200,phon_file)!=200) return(0);
    s = header->wdh;
    shortvms2sun(&s);
    if(fwrite(&s,sizeof(short),1,phon_file)!=1) return(0);
    s = header->abs_ampl;
    shortvms2sun(&s);
    if(fwrite(&s,sizeof(short),1,phon_file)!=1) return(0);
    if(fwrite(buffer,sizeof(char),10,phon_file)!=10) return(0);

    return(1);
 
} /* end subroutine : write_header_1_sun  */

/*----------------------------------------------------------------------
Name            : write_header_1_vms
Module          : HEADER
Title           : writes fixed information of header in file (version 1)

Description:
Writes the fixed information of header version 
1 (if not version 1 returns error) from structure Phon_header_2
(first 512 bytes of header). 
ATTENTION: The string elements in structure Phon_header_2 are NOT 
terminated by '\0' except the element ifl.

Parameters:
phon_file  : File pointer to Phondat file version 1
header          : pointer to structure with header information

Return-Value    : 1 : writing successful
                  0 : error during write
-------------------------------------------------------------------------*/
int write_header_1_vms(FILE *phon_file,Phon_header_2 *header)
{
  char buffer[512];
        short s;
  int i;
  long l;


    for(i=0;i<512;i++) buffer[i] = '\0';

    if(header == NULL) 
      {
      fprintf(stderr,"write_header_1_vms: header structure empty\n");
      return(0);
      } 

/* test, if header is version 1            */
    if(header->version != 1)
    {
  fprintf(stderr,"write_header_1_vms: header is not PhonDat version 1\n");
  return(0);
    }

   if(header->anz_header != 1) header->anz_header = 1;

/* write items of Phondatheader in file, not used items are padded with zero */
    if(fwrite(buffer,sizeof(char),20,phon_file) != 20)
      {
      fprintf(stderr,"write_header_1_vms: cannot write first item\n");
      return(0);
      }

    l = header->nspbk;
    if(fwrite(&l,sizeof(long),1,phon_file)!=1) return(0);
    l = header->anz_header;
    if(fwrite(&l,sizeof(long),1,phon_file)!=1) return(0);
    if(fwrite(buffer,sizeof(char),20,phon_file)!=20) return(0);
    if(fwrite(header->sprk,sizeof(char),2,phon_file)!=2) return(0);
    s = header->swdh;
    if(fwrite(&s,sizeof(short),1,phon_file)!=1) return(0);
    l = header->ifld1[0];
    if(fwrite(&l,sizeof(long),1,phon_file)!=1) return(0);
    l = header->ifld1[1];
    if(fwrite(&l,sizeof(long),1,phon_file)!=1) return(0);
    l = header->ifld1[2];
    if(fwrite(&l,sizeof(long),1,phon_file)!=1) return(0);
    if(fwrite(buffer,sizeof(char),24,phon_file)!=24) return(0);
    if(fwrite(header->kenn1,sizeof(char),2,phon_file)!=2) return(0);
    if(fwrite(buffer,sizeof(char),2,phon_file)!=2) return(0);
    if(fwrite(header->kenn2,sizeof(char),2,phon_file)!=2) return(0);
    if(fwrite(buffer,sizeof(char),2,phon_file)!=2) return(0);
    if(fwrite(header->kenn3,sizeof(char),2,phon_file)!=2) return(0);
    if(fwrite(buffer,sizeof(char),2,phon_file)!=2) return(0);
    if(fwrite(header->kenn4,sizeof(char),2,phon_file)!=2) return(0);
    if(fwrite(buffer,sizeof(char),142,phon_file)!=142) return(0);
    l = header->isf;
    if(fwrite(&l,sizeof(long),1,phon_file)!=1) return(0);
    l = header->flagtype;
    if(fwrite(&l,sizeof(long),1,phon_file)!=1) return(0);
    l = header->flaginit;
    if(fwrite(&l,sizeof(long),1,phon_file)!=1) return(0);
    if(fwrite(header->ifl,sizeof(char),strlen(header->ifl),phon_file)!=strlen(header->ifl)) return(0);
    if(fwrite(buffer,sizeof(char),32-strlen(header->ifl),phon_file)!=32-strlen(header->ifl)) return(0);
    if(fwrite(&(header->day),sizeof(char),1,phon_file)!=1) return(0);
    if(fwrite(&(header->month),sizeof(char),1,phon_file)!=1) return(0);
    s = header->year;
    if(fwrite(&s,sizeof(short),1,phon_file)!=1) return(0);
    if(fwrite(&(header->sex),sizeof(char),1,phon_file)!=1) return(0);
    if(fwrite(&(header->version),sizeof(char),1,phon_file)!=1) return(0);
    s = header->adc_bits;
    if(fwrite(&s,sizeof(short),1,phon_file)!=1) return(0);
    s = header->words;
    if(fwrite(&s,sizeof(short),1,phon_file)!=1) return(0);
    if(fwrite(buffer,sizeof(char),200,phon_file)!=200) return(0);
    s = header->wdh;
    if(fwrite(&s,sizeof(short),1,phon_file)!=1) return(0);
    s = header->abs_ampl;
    if(fwrite(&s,sizeof(short),1,phon_file)!=1) return(0);
    if(fwrite(buffer,sizeof(char),10,phon_file)!=10) return(0);

    return(1);
 
} /* end subroutine : write_header_1_vms  */

/*----------------------------------------------------------------------
Name            : write_header_sun
Module          : HEADER
Title           : writes Phondat header in file (version 1 or 2)

Description:
Writes the Phondat header version 1 or 2 from structure Phon_header_2
If the Pointers to the orthographic and canonical strings are
If the header structure is Version 1, a header Phondat
version 1 will be written (512 Bytes, the orthographical and canonical 
strings are ignored). Else, a header Phondat 2
will be written. If the string pointers and the header structure
conflict, an error is returned.
The number of header blocks is calculated by the function. Not
filled up header blocks are padded with zero. The filepointer is
positioned to the first byte of the first data block.
ATTENTION: The string elements in structure Phon_header_2 are NOT 
terminated by '\0' except the element ifl.

Parameters:
phon_file  : File pointer to Phondat file version 1
header          : pointer to structure with header information
ortho    : orthographical string
cano    : canonical string

Return-Value    : 1 : writing successful
                  0 : error during write
-------------------------------------------------------------------------*/
int write_header_sun
(
  FILE*          phon_file,
  Phon_header_2* header,
  char*          ortho,
  char*          cano
)
{
  char  buffer[512];
  short s         = 0;
  int   i         = 0;
  int   anz_bytes = 0;
  long  l         = 0;
  long  lenortho  = 0;
  long  lencano   = 0;


  for(i=0;i<512;i++) buffer[i] = '\0';

  if(header == NULL) 
  {
    fprintf(stderr,"write_header_sun: header structure empty\n");
    return(0);
  }

  /*fseek(phon_file,0,0); */

  /*test, if information is consistent        */
  if(header->version == 1) header->anz_header = 1;
  if(ortho != NULL       ) lenortho = strlen(ortho);
  if(cano  != NULL       ) lencano  = strlen(cano);

  /*calculated length of header in 512 Bytes blocks version 2 */
  if(header->version == 2)
  {
    anz_bytes = 512;                             /* fixed header block */
    anz_bytes += 4;                              /* 'ort\0' */
    anz_bytes += lenortho;                       /* orthographic string */
    anz_bytes += 8;                              /* '\0oend::\0' */
    anz_bytes += 4;                              /* 'kan\0' */
    anz_bytes += lencano;                        /* canonical string */
    anz_bytes += 8;                              /* '\0kend::\0' */
    header->anz_header = anz_bytes / 512;
    if((anz_bytes % 512) != 0) header->anz_header++;
  }

  /* write items of Phondatheader in file, not used items are padded with zero */
  if(fwrite(buffer,sizeof(char),20,phon_file) != 20)
  {
    fprintf(stderr,"write_header_sun: cannot write first item\n");
    return(0);
  }

  l = header->nspbk;
  longvms2sun(&l);
  if(fwrite(&l,sizeof(long),1,phon_file)!=1) return(0);
  l = header->anz_header;
  longvms2sun(&l);
  if(fwrite(&l,sizeof(long),1,phon_file)!=1) return(0);
  if(fwrite(buffer,sizeof(char),20,phon_file)!=20) return(0);
  if(fwrite(header->sprk,sizeof(char),2,phon_file)!=2) return(0);
  s = header->swdh;
  shortvms2sun(&s);
  if(fwrite(&s,sizeof(short),1,phon_file)!=1) return(0);
  l = header->ifld1[0];
  longvms2sun(&l);
  if(fwrite(&l,sizeof(long),1,phon_file)!=1) return(0);
  l = header->ifld1[1];
  longvms2sun(&l);
  if(fwrite(&l,sizeof(long),1,phon_file)!=1) return(0);
  l = header->ifld1[2];
  longvms2sun(&l);
  if(fwrite(&l,sizeof(long),1,phon_file)!=1) return(0);
  if(fwrite(buffer,sizeof(char),24,phon_file)!=24) return(0);
  if(fwrite(header->kenn1,sizeof(char),2,phon_file)!=2) return(0);
  if(fwrite(buffer,sizeof(char),2,phon_file)!=2) return(0);
  if(fwrite(header->kenn2,sizeof(char),2,phon_file)!=2) return(0);
  if(fwrite(buffer,sizeof(char),2,phon_file)!=2) return(0);
  if(fwrite(header->kenn3,sizeof(char),2,phon_file)!=2) return(0);
  if(fwrite(buffer,sizeof(char),2,phon_file)!=2) return(0);
  if(fwrite(header->kenn4,sizeof(char),2,phon_file)!=2) return(0);
  if(fwrite(buffer,sizeof(char),142,phon_file)!=142) return(0);
  l = header->isf;
  longvms2sun(&l);
  if(fwrite(&l,sizeof(long),1,phon_file)!=1) return(0);
  l = header->flagtype;
  longvms2sun(&l);
  if(fwrite(&l,sizeof(long),1,phon_file)!=1) return(0);
  l = header->flaginit;
  longvms2sun(&l);
  if(fwrite(&l,sizeof(long),1,phon_file)!=1) return(0);
  if(fwrite(header->ifl,sizeof(char),strlen(header->ifl),phon_file)!=strlen(header->ifl)) return(0);
  if(fwrite(buffer,sizeof(char),32-strlen(header->ifl),phon_file)!=32-strlen(header->ifl)) return(0);
  if(fwrite(&(header->day),sizeof(char),1,phon_file)!=1) return(0);
  if(fwrite(&(header->month),sizeof(char),1,phon_file)!=1) return(0);
  s = header->year;
  shortvms2sun(&s);
  if(fwrite(&s,sizeof(short),1,phon_file)!=1) return(0);
  if(fwrite(&(header->sex),sizeof(char),1,phon_file)!=1) return(0);
  if(fwrite(&(header->version),sizeof(char),1,phon_file)!=1) return(0);
  s = header->adc_bits;
  shortvms2sun(&s);
  if(fwrite(&s,sizeof(short),1,phon_file)!=1) return(0);
  s = header->words;
  shortvms2sun(&s);
  if(fwrite(&s,sizeof(short),1,phon_file)!=1) return(0);
  if(fwrite(buffer,sizeof(char),200,phon_file)!=200) return(0);
  s = header->wdh;
  shortvms2sun(&s);
  if(fwrite(&s,sizeof(short),1,phon_file)!=1) return(0);
  s = header->abs_ampl;
  shortvms2sun(&s);
  if(fwrite(&s,sizeof(short),1,phon_file)!=1) return(0);
  if(fwrite(buffer,sizeof(char),10,phon_file)!=10) return(0);

  /* if header version 2 writes the orthographical and canonical strings
     in the following blocks if the pointers to the strings are empty,
     null strings are written */
  if(header->version == 2)
  {
    if(fwrite("ort",sizeof(char),4,phon_file)!=4) return(0);
    if(ortho != NULL) if(fwrite(ortho,sizeof(char),lenortho,phon_file)!=(size_t)(lenortho)) return(0);
    if(fwrite("\0oend::",sizeof(char),8,phon_file)!=8) return(0);
    if(fwrite("kan",sizeof(char),4,phon_file)!=4) return(0);
    if(cano != NULL) if(fwrite(cano,sizeof(char),lencano,phon_file)!=(size_t)(lencano)) return(0);
    if(fwrite("\0kend::",sizeof(char),8,phon_file)!=8) return(0);
    if ((anz_bytes%512) != 0)
      if(fwrite(buffer,sizeof(char),512-(anz_bytes%512),phon_file)!=(size_t)(512-(anz_bytes%512))) return(0);
  }

  return 1;
} /* end subroutine : write_header_sun  */

/*----------------------------------------------------------------------
Name            : write_header_vms
Module          : HEADER
Title           : writes Phondat header in file (version 1 or 2)

Description:
Writes the Phondat header version 1 or 2 from structure Phon_header_2
If the Pointers to the orthographic and canonical strings are
If the header structure is Version 1, a header Phondat
version 1 will be written (512 Bytes, the orthographical and canonical 
strings are ignored). Else, a header Phondat 2
will be written. If the string pointers and the header structure
conflict, an error is returned.
The number of header blocks is calculated by the function. Not
filled up header blocks are padded with zero. The filepointer is
positioned to the first byte of the first data block.
ATTENTION: The string elements in structure Phon_header_2 are NOT 
terminated by '\0' except the element ifl.

Parameters:
phon_file  : File pointer to Phondat file version 1
header          : pointer to structure with header information
ortho    : orthographical string
cano    : canonical string

Return-Value    : 1 : writing successful
                  0 : error during write
-------------------------------------------------------------------------*/
int write_header_vms
(
  FILE*          phon_file,
  Phon_header_2* header,
  char*          ortho,
  char*          cano
)
{
  char  buffer[512];
  short s = 0;
  int   i = 0;
  int   anz_bytes = 0;
  long  l         = 0;
  long  lenortho  = 0;
  long  lencano   = 0;


  for(i=0;i<512;i++) buffer[i] = '\0';

  if(header == NULL) 
  {
    fprintf(stderr,"write_header_vms: header structure empty\n");
    return(0);
  }

  /* test, if information is consistent        */
  if(header->version == 1) header->anz_header = 1;
  if(ortho != NULL) lenortho = strlen(ortho);
  if(cano  != NULL) lencano  = strlen(cano);

  /* calculated length of header in 512 Bytes blocks version 2 */
  if(header->version == 2)
  {
    anz_bytes = 512;      /* fixed header block */
    anz_bytes += 4;    /* 'ort\0' */
    anz_bytes += lenortho; /* orthographic string */
    anz_bytes += 8;    /* '\0oend::\0' */
    anz_bytes += 4;    /* 'kan\0' */
    anz_bytes += lencano;  /* canonical string */
    anz_bytes += 8;    /* '\0kend::\0' */
    header->anz_header = anz_bytes / 512;
    if((anz_bytes % 512) != 0) header->anz_header++;
  }

  /* write items of Phondatheader in file, not used items are padded with zero */
  if(fwrite(buffer,sizeof(char),20,phon_file) != 20)
  {
    fprintf(stderr,"write_header_vms: cannot write first item\n");
    return(0);
    }

  l = header->nspbk;
  if(fwrite(&l,sizeof(long),1,phon_file)!=1) return(0);
  l = header->anz_header;
  if(fwrite(&l,sizeof(long),1,phon_file)!=1) return(0);
  if(fwrite(buffer,sizeof(char),20,phon_file)!=20) return(0);
  if(fwrite(header->sprk,sizeof(char),2,phon_file)!=2) return(0);
  s = header->swdh;
  if(fwrite(&s,sizeof(short),1,phon_file)!=1) return(0);
  l = header->ifld1[0];
  if(fwrite(&l,sizeof(long),1,phon_file)!=1) return(0);
  l = header->ifld1[1];
  if(fwrite(&l,sizeof(long),1,phon_file)!=1) return(0);
  l = header->ifld1[2];
  if(fwrite(&l,sizeof(long),1,phon_file)!=1) return(0);
  if(fwrite(buffer,sizeof(char),24,phon_file)!=24) return(0);
  if(fwrite(header->kenn1,sizeof(char),2,phon_file)!=2) return(0);
  if(fwrite(buffer,sizeof(char),2,phon_file)!=2) return(0);
  if(fwrite(header->kenn2,sizeof(char),2,phon_file)!=2) return(0);
  if(fwrite(buffer,sizeof(char),2,phon_file)!=2) return(0);
  if(fwrite(header->kenn3,sizeof(char),2,phon_file)!=2) return(0);
  if(fwrite(buffer,sizeof(char),2,phon_file)!=2) return(0);
  if(fwrite(header->kenn4,sizeof(char),2,phon_file)!=2) return(0);
  if(fwrite(buffer,sizeof(char),142,phon_file)!=142) return(0);
  l = header->isf;
  if(fwrite(&l,sizeof(long),1,phon_file)!=1) return(0);
  l = header->flagtype;
  if(fwrite(&l,sizeof(long),1,phon_file)!=1) return(0);
  l = header->flaginit;
  if(fwrite(&l,sizeof(long),1,phon_file)!=1) return(0);
  if(fwrite(header->ifl,sizeof(char),strlen(header->ifl),phon_file)!=strlen(header->ifl)) return(0);
  if(fwrite(buffer,sizeof(char),32-strlen(header->ifl),phon_file)!=32-strlen(header->ifl)) return(0);
  if(fwrite(&(header->day),sizeof(char),1,phon_file)!=1) return(0);
  if(fwrite(&(header->month),sizeof(char),1,phon_file)!=1) return(0);
  s = header->year;
  if(fwrite(&s,sizeof(short),1,phon_file)!=1) return(0);
  if(fwrite(&(header->sex),sizeof(char),1,phon_file)!=1) return(0);
  if(fwrite(&(header->version),sizeof(char),1,phon_file)!=1) return(0);
  s = header->adc_bits;
  if(fwrite(&s,sizeof(short),1,phon_file)!=1) return(0);
  s = header->words;
  if(fwrite(&s,sizeof(short),1,phon_file)!=1) return(0);
  if(fwrite(buffer,sizeof(char),200,phon_file)!=200) return(0);
  s = header->wdh;
  if(fwrite(&s,sizeof(short),1,phon_file)!=1) return(0);
  s = header->abs_ampl;
  if(fwrite(&s,sizeof(short),1,phon_file)!=1) return(0);
  if(fwrite(buffer,sizeof(char),10,phon_file)!=10) return(0);

  /* if header version 2 writes the orthographical and canonical strings
     in the following blocks if the pointers to the string are NULL, null
     strings are written */

  if(header->version == 2)
  {
    if(fwrite("ort",sizeof(char),4,phon_file)!=4) return(0);
    if(ortho != NULL) if(fwrite(ortho,sizeof(char),lenortho,phon_file)!=(size_t)lenortho) return(0);
    if(fwrite("\0oend::",sizeof(char),8,phon_file)!=8) return(0);
    if(fwrite("kan",sizeof(char),4,phon_file)!=4) return(0);
    if(cano != NULL) if(fwrite(cano,sizeof(char),lencano,phon_file)!=(size_t)lencano) return(0);
    if(fwrite("\0kend::",sizeof(char),8,phon_file)!=8) return(0);
    if ((anz_bytes%512) != 0 )
      if(fwrite(buffer,sizeof(char),512-(anz_bytes%512),phon_file)!=(size_t)(512-(anz_bytes%512))) return(0);
  }

  return(1);
} /* end subroutine : write_header_vms  */

/*----------------------------------------------------------------------
Name            : read_header_vms
Module          : HEADER
Title           : reads information of header of PhonDat file 

Description:
Sets pointer to beginning of file, reads all relevant 
information in header version 2 or 1, creates structure Phon_header_2 with 
fixed information (first 512 bytes of header), returns pointer to this 
structure, reads orthographic and canonical informations (2nd and following 
512 blocks), writes pointer to strings with orthographic and canonical 
information into adresses ortho and cano, positions the file pointer to the 
first byte after the header blocks.
If file contains header version 1, the pointers *ortho and *cano are NULL.
ATTENTION: The string elements in structure Phon_header_2 are NOT 
terminated by '\0' except the element ifl.
The memory of structure and strings can be set free by using free() to 
corresponding pointers.

Parameters:
phon_file  : File pointer to Phondat file version 2 
ortho    : orthographic information
cano    : canonical information
    : 

Return-Value    : pointer to structure with header information
      NULL, if error occurs
-------------------------------------------------------------------------*/
Phon_header_2 *read_header_vms(FILE *phon_file,char **ortho,char **cano)
{
  char* buff            = NULL;
  char* buffp           = NULL;
  char  buffer[512];
  int  len              = 0;
  int  vflag            = 0;
  Phon_header_2* header = NULL; /* pointer to header structure */

  /* rewind file to beginning */
  rewind(phon_file);

  /* allocate space for fixed header */
  if((header = (Phon_header_2 *)calloc(1,sizeof(Phon_header_2))) == NULL)
  {
    fprintf(stderr,"read_header_vms: cannot allocate space for fixed header\n");
    fflush(stderr);
    perror("read_header_vms");
    *ortho = NULL;
    *cano = NULL;
    return(NULL);
  }

  if(fread(buffer,FIXEDHEADERLENGTH,1,phon_file) != 1)
  {
    fprintf(stderr,"read_header_vms: read error for 512 bytes (fixed header)\n");
    fflush(stderr);
    free((char *)header);
    *ortho = NULL;
    *cano = NULL;
    return(NULL);
  }

  /* read items of Phondatheader in structure */
  buffp = buffer + 20;
  header->nspbk = *((long*)buffp);
  buffp = buffer + 24;
  header->anz_header = *((long*)buffp);
  buffp = buffer + 48;
  header->sprk[0] = buffp[0];
  header->sprk[1] = buffp[1];
  buffp = buffer + 50;
  header->swdh = *((short*)buffp);
  buffp = buffer + 52;
  header->ifld1[0] = *((long*)buffp);
  buffp = buffer + 56;
  header->ifld1[1] = *((long*)buffp);
  buffp = buffer + 60;
  header->ifld1[2] = *((long*)buffp);
  buffp = buffer + 88;
  header->kenn1[0] = buffp[0];
  header->kenn1[1] = buffp[1];
  buffp = buffer + 92;
  header->kenn2[0] = buffp[0];
  header->kenn2[1] = buffp[1];
  buffp = buffer + 96;
  header->kenn3[0] = buffp[0];
  header->kenn3[1] = buffp[1];
  buffp = buffer + 100;
  header->kenn4[0] = buffp[0];
  header->kenn4[1] = buffp[1];
  buffp = buffer + 244;
  header->isf = *((long*)buffp);
  buffp = buffer + 248;
  header->flagtype = *((long*)buffp);
  buffp = buffer + 252;
  header->flaginit = *((long*)buffp);
  buffp = buffer + 256;
  strcpy(header->ifl,buffp);
  buffp = buffer + 288;
  header->day = *buffp;
  buffp = buffer + 289;
  header->month = *buffp;
  buffp = buffer + 290;
  header->year = *((short*)buffp);
  buffp = buffer + 292;
  header->sex = *buffp;
  buffp = buffer + 293;
  header->version = *buffp;
  buffp = buffer + 294;
  header->adc_bits = *((short*)buffp);
  buffp = buffer + 296;
  header->words = *((short*)buffp);
  buffp = buffer + 498;
  header->wdh = *((short*)buffp);
  buffp = buffer + 500;
  header->abs_ampl = *((short*)buffp);
  buffp = buffer + 502;
  strncpy(header->not_usedb,buffp,10);

  /*if(iolog(0)) disp_header_v2(header);*/
  /* testing constistency */
  if(header->anz_header <= 0)
  {
    fprintf(stderr," read_header_vms: header information inconsistent - assuming fixed header is present only\n");
    fflush(stderr);
    *ortho = NULL;
    *cano = NULL;
    header->anz_header=1;
    return(header);
  }
  else if(header->anz_header == 1)
  {
    vflag = 1;
    if(header->version != 1)
      fprintf(stderr,"read_header_vms: header information inconsistent\n assuming header version 1\n");
  }
  else if(header->anz_header > 1)
  {
    vflag = 2;
    if(header->version != 2)
      fprintf(stderr,"read_header_vms: header information inconsistent\n assuming header version 2\n");
  }

  if(vflag == 2)
  {
    /* read following blocks with orthographic and canonical texts    */
    if((buff = (char *)calloc(header->anz_header - 1,512)) == NULL)
    {
      fprintf(stderr,"read_header_vms: cannot allocate space for rest of header\n");
      perror("read_header_vms");
      free((char *)header);
      *ortho = NULL;
      *cano = NULL;
      return(NULL);
    }
    if(fread(buff,512,header->anz_header - 1,phon_file) != (size_t)(header->anz_header - 1))
    {
      fprintf(stderr,"read_header_vms: read error for dynamic part of header\n");
      free((char *)header);
      free((char *)buff);
      *ortho = NULL;
      *cano = NULL;
      return(NULL);
    }

    /* find beginning of orthographic text, beginning marker = "ort\0"
       ending marker    = "\0oend::\0" */
    buffp = buff;
    while(1)
    {
      if(strcmp(buffp,"ort") == 0) break;
      buffp++;
      if(buffp == buff + (header->anz_header - 1) * 512)
      {
        fprintf(stderr,"read_header_vms: can't find orthographic text\n");
        free((char *)buff);
        *ortho = NULL;
        *cano = NULL;
        return(header);
      }
    }
    buffp += 4;

    /*if(iolog(0)) printf("read_header_v2: orthographic text: %s\n",buffp);*/
    /* copy orthographic text in allocated space */
    len = strlen(buffp);
    if((*ortho = (char *)calloc(1,len+1)) == NULL)
    {
      fprintf(stderr,"read_header_vms: cannot allocate space for orthographic text\n");
      perror("read_header_vms");
      free((char *)buff);
      *ortho = NULL;
      *cano = NULL;
      return(header);
    }
    strcpy(*ortho,buffp);
    buffp += len + 1;

    /* find beginning of canonical text, beginning marker = "kan\0"
       ending marker    = "\0kend::\0" */
    while(1)
    {
      if(strcmp(buffp,"kan") == 0) break;
      buffp++;
      if(buffp == buff + (header->anz_header - 1) * 512)
      {
        fprintf(stderr,"read_header_vms: can't find canonical text\n");
        free((char *)buff);
        *cano = NULL;
        return(header);
      }
    }
    buffp += 4;

    /*if(iolog(0)) printf("read_header_v2: canonical text: %s\n",buffp);*/
    /* copy canonical text in allocated space */
    len = strlen(buffp);
    if((*cano = (char *)calloc(1,len+1)) == NULL)
    {
      fprintf(stderr,"read_header_vms: cannot allocate space for canonical text\n");
      perror("read_header_vms");
      free((char *)buff);
      *cano = NULL;
      return(header);
    }
    strcpy(*cano,buffp);

    free((char *)buff);

  } /* end if vflag == 2 */
  else
  {
    *ortho = NULL;
    *cano = NULL;
  }

  return(header);
} /* end subroutine : read_header_vms  */

/*----------------------------------------------------------------------
Name            : read_header_sun
Module          : HEADER
Title           : reads information of header of PhonDat file version 2

Description:
Sets pointer to beginning of file, reads all relevant 
information in header version 1 or 2, creates structure Phon_header_2 with 
fixed information (first 512 bytes of header), returns pointer to this 
structure, reads orthographic and canonical informations (2nd and following 
512 blocks), writes pointer to strings with orthographic and canonical 
information into adresses ortho and cano, positions the file pointer to the 
first byte after the header blocks.
If the file contains a header version 1, the pointers *orth and *cano will 
be NULL.
ATTENTION: The string elements in structure Phon_header_2 are NOT 
terminated by '\0' except the element ifl.
The memory of structure and strings can be set free by using free() to 
corresponding pointers.

Parameters:
phon_file  : File pointer to Phondat file version 2 
ortho    : orthographic information
cano    : canonical information
    : 

Return-Value    : pointer to structure with header information
      NULL, if error occurs
-------------------------------------------------------------------------*/
Phon_header_2 *read_header_sun(FILE *phon_file,char **ortho,char **cano)
{
  char*          buff   = 0;
  char*          buffp  = 0;
  char           buffer[512];
  int            len    = 0;
  int            vflag  = 0;
  Phon_header_2* header = NULL; /* pointer to header structure */

/* rewind file to beginning            */
    rewind(phon_file);

/* allocate space for fixed header          */
    if((header = (Phon_header_2 *)calloc(1,sizeof(Phon_header_2))) == NULL)
    {
  fprintf(stderr,"read_header_sun: cannot allocate space for fixed header\n");
  perror("read_header_sun");
  *ortho = NULL;
  *cano = NULL;
  return(NULL);
    }

    if(fread(buffer,FIXEDHEADERLENGTH,1,phon_file) != 1)
    {
  fprintf(stderr,"read_header_sun: read error for 512 bytes (fixed header)\n");
  free((char *)header);
  *ortho = NULL;
  *cano = NULL;
  return(NULL);
    }

/* read items of Phondatheader in structure        */
    buffp = buffer + 20;
    header->nspbk = *((long*)buffp);
    longvms2sun(&(header->nspbk));
    buffp = buffer + 24;
    header->anz_header = *((long*)buffp);
    longvms2sun(&(header->anz_header));
    buffp = buffer + 48;
    header->sprk[0] = buffp[0];
    header->sprk[1] = buffp[1];
    buffp = buffer + 50;
    header->swdh = *((short*)buffp);
    shortvms2sun(&(header->swdh));
    buffp = buffer + 52;
    header->ifld1[0] = *((long*)buffp);
    longvms2sun(&(header->ifld1[0]));
    buffp = buffer + 56;
    header->ifld1[1] = *((long*)buffp);
    longvms2sun(&(header->ifld1[1]));
    buffp = buffer + 60;
    header->ifld1[2] = *((long*)buffp);
    longvms2sun(&(header->ifld1[2]));
    buffp = buffer + 88;
    header->kenn1[0] = buffp[0];
    header->kenn1[1] = buffp[1];
    buffp = buffer + 92;
    header->kenn2[0] = buffp[0];
    header->kenn2[1] = buffp[1];
    buffp = buffer + 96;
    header->kenn3[0] = buffp[0];
    header->kenn3[1] = buffp[1];
    buffp = buffer + 100;
    header->kenn4[0] = buffp[0];
    header->kenn4[1] = buffp[1];
    buffp = buffer + 244;
    header->isf = *((long*)buffp);
    longvms2sun(&(header->isf));
    buffp = buffer + 248;
    header->flagtype = *((long*)buffp);
    longvms2sun(&(header->flagtype));
    buffp = buffer + 252;
    header->flaginit = *((long*)buffp);
    longvms2sun(&(header->flaginit));
    buffp = buffer + 256;
    strcpy(header->ifl,buffp);
    buffp = buffer + 288;
    header->day = *buffp;
    buffp = buffer + 289;
    header->month = *buffp;
    buffp = buffer + 290;
    header->year = *((short*)buffp);
    shortvms2sun(&(header->year));
    buffp = buffer + 292;
    header->sex = *buffp;
    buffp = buffer + 293;
    header->version = *buffp;
    buffp = buffer + 294;
    header->adc_bits = *((short*)buffp);
    shortvms2sun(&(header->adc_bits));
    buffp = buffer + 296;
    header->words = *((short*)buffp);
    shortvms2sun(&(header->words));
    buffp = buffer + 498;
    header->wdh = *((short*)buffp);
    shortvms2sun(&(header->wdh));
    buffp = buffer + 500;
    header->abs_ampl = *((short*)buffp);
    shortvms2sun(&(header->abs_ampl));
    buffp = buffer + 502;
    strncpy(header->not_usedb,buffp,10);

/*if(iolog(0)) disp_header_v2(header);
*/

/* testing constistency */
    if(header->anz_header <= 0)
  {
    fprintf(stderr," read_header_vms: header information inconsistent - assuming fixed header is present only\n");
    fflush(stderr);
    *ortho = NULL;
    *cano = NULL;
    return(header);
  }
    if(header->anz_header == 1)
      {
      vflag = 1;
      if(header->version != 1)
        fprintf(stderr,"read_header_sun: header information inconsistent\n assuming header version 1\n");
      }
    else if(header->anz_header > 1)
      {
      vflag = 2;
      if(header->version != 2)                                              
        fprintf(stderr,"read_header_sun: header information inconsistent\n assuming header version 2\n");
      }                                                                     
    
    if(vflag == 2)
    {
/* read following blocks with orthographic and canonical texts    */
    if((buff = (char *)calloc(header->anz_header - 1,512)) == NULL)
    {
  fprintf(stderr,"read_header_sun: cannot allocate space for rest of header\n");
  perror("read_header_sun");
  free((char *)header);
  *ortho = NULL;
  *cano = NULL;
  return(NULL);
    }
    if(fread(buff,512,header->anz_header - 1,phon_file) != 
          (size_t)(header->anz_header - 1))
    {
  fprintf(stderr,"read_header_sun: read error for dynamic part of header\n");
  free((char *)header);
  free((char *)buff);
  *ortho = NULL;
  *cano = NULL;
  return(NULL);
    }

/* find beginning of orthographic text, beginning marker = "ort\0"
          ending marker    = "\0oend::\0"  */
    buffp = buff;
    while(1)
    {
  if(strcmp(buffp,"ort") == 0) break;
  buffp++;
  if(buffp == buff + (header->anz_header - 1) * 512)
      {
    fprintf(stderr,
    "read_header_sun: can't find orthographic text\n");
    free((char *)buff);
    *ortho = NULL;
    *cano = NULL;
    return(header);
      }

    }    
    buffp += 4;

/*if(iolog(0)) printf("read_header_v2: orthographic text: %s\n",buffp);
*/
/* copy orthographic text in allocated space        */
    len = strlen(buffp);
    if((*ortho = (char *)calloc(1,len+1)) == NULL)
    {
  fprintf(stderr,
  "read_header_sun: cannot allocate space for orthographic text\n");
  perror("read_header_sun");
  free((char *)buff);
  *ortho = NULL;
  *cano = NULL;
  return(header);
    }
    strcpy(*ortho,buffp);
    buffp += len + 1;

/* find beginning of canonical text, beginning marker = "kan\0"
          ending marker    = "\0kend::\0"  */
    while(1)
    {
  if(strcmp(buffp,"kan") == 0) break;
  buffp++;
  if(buffp == buff + (header->anz_header - 1) * 512)
      {
    fprintf(stderr,
    "read_header_sun: can't find canonical text\n");
    free((char *)buff);
    *cano = NULL;
    return(header);
      }

    }    
    buffp += 4;

/*if(iolog(0)) printf("read_header_v2: canonical text: %s\n",buffp);
*/
/* copy canonical text in allocated space        */
    len = strlen(buffp);
    if((*cano = (char *)calloc(1,len+1)) == NULL)
    {
  fprintf(stderr,
  "read_header_sun: cannot allocate space for canonical text\n");
  perror("read_header_sun");
  free((char *)buff);
  *cano = NULL;
  return(header);
    }
    strcpy(*cano,buffp);

    free((char *)buff);
   
    } /* end if vflag == 2 */

    return(header);
 
} /* end subroutine : read_header_sun  */

/*----------------------------------------------------------------------
Name            : disp_header_v2
Module    : HEADER
Title           : displays the content of structure Phon_header_2 to screen

Description:

Parameters:
header    : pointer to struture 

Return-Value    : void
-------------------------------------------------------------------------*/
void disp_header_v2(Phon_header_2 *header)
{ 
  char buff[256];

printf("structure Phon_header_2:\n");

printf("not_used1[0]:       %ld\n",header->not_used1[0]);
printf("nspbk:              %ld\n",header->nspbk);
          /* # of data blocks (512 bytes)  */
printf("anz_header:         %ld\n",header->anz_header);
          /* # of header blocks (512 bytes)*/
printf("not_used2[0]:       %ld\n",header->not_used2[0]);
strncpy(buff,header->sprk,2);
buff[2] = '\0',
printf("sprk:               %s\n",buff);/* speaker id      */
printf("swdh:               %hd\n",header->swdh);
          /* session repetition    */
printf("ifld1[0]:           %ld\n",header->ifld1[0]);
          /* ILS        */
printf("not_used3[0]:       %ld\n",header->not_used3[0]);
strncpy(buff,header->kenn1,2);
buff[2] = '\0',
printf("kenn1[2]:           %s\n",buff);/* ILS text characters 1 - 8  */
printf("not_used4: %d\n",header->not_used4);
strncpy(buff,header->kenn2,2);
buff[2] = '\0',
printf("kenn2[2]:           %s\n",buff);
printf("not_used5: %d\n",header->not_used5);
strncpy(buff,header->kenn3,2);
buff[2] = '\0',
printf("kenn3[2]:           %s\n",buff);
printf("not_used6: %d\n",header->not_used6);
strncpy(buff,header->kenn4,2);
buff[2] = '\0',
printf("kenn4[2]:           %s\n",buff);
printf("not_used7:          %hd\n",header->not_used7);
printf("not_used8[0]:       %ld\n",header->not_used8[0]);
printf("isf:                %ld\n",header->isf);
          /* sampling rate in Hz    */
printf("flagtype:          %ld\n",header->flagtype);
          /* ILS: -32000 if sampling file  */
printf("flaginit:          %ld\n",header->flaginit);
          /* ILS: 32149 if init    */
strncpy(buff,header->ifl,32);
buff[32] = '\0',
printf("ifl[32]:            %s\n",buff);/* filename (terminated by /0)  */
printf("day:                %hhd\n",header->day);
          /* # of day      */
printf("month:              %hhd\n",header->month);
          /* # of month      */
printf("year:               %hd\n",header->year);
          /* # of year      */
printf("sex:                %c\n",header->sex);
          /* sex of speaker    */
printf("version:            %hhd\n",header->version);
          /* header version:    */ 
printf("adc_bits:           %hd\n",header->adc_bits);
          /* resolution of adc    */
printf("words:              %hd\n",header->words);
          /* # of words in text    */
printf("not_useda[0]:       %ld\n",header->not_useda[0]);
printf("wdh:                %hd\n",header->wdh);
          /* # of repetition    */
printf("abs_ampl:           %hd\n",header->abs_ampl);
          /* maximum of amplitude    */
strncpy(buff,header->not_usedb,10);
buff[10] = '\0',
printf("not_usedb:          %s\n",buff);

return;

} /* end subroutine : disp_header_v2 */

/*----------------------------------------------------------------------
Name            : shortvms2sun
Module    : header.c
Title           : transforms a short in VMS to SUN format

Description:

Parameters:
s    : adress of short

Return-Value    : void 
-------------------------------------------------------------------------*/
void shortvms2sun(short *s)
{
  char buff,*charpoint;
 
/* conversion to sun: change higher and lower byte       */
    charpoint = (char *)s;
    buff = charpoint[0];
    charpoint[0] = charpoint[1];
    charpoint[1] = buff;

    return;

} /* end subroutine : shortvms2sun  */

/*----------------------------------------------------------------------
Name            : longvms2sun
Module    : header.c
Title           : transforms long in VMS to SUN format

Description:

Parameters:
l    : adress of long
  
Return-Value    : void
-------------------------------------------------------------------------*/
void longvms2sun(long *l)
{ 
  char buff,*charpoint;

/* reverse sequenz of bytes in long word       */  
    charpoint = (char *)l;
    buff = charpoint[0];
    charpoint[0] = charpoint[3];
    charpoint[3] = buff;
    buff = charpoint[1];
    charpoint[1] = charpoint[2];
    charpoint[2] = buff; 

    return;

} /* end subroutine : longvms2sun */



/***********************************************************************
************************************************************************

Module    : DATA
Title    : reading data from PhonDat files version 2

Author    : F. Schiel
Date/revision   : 16.12.93 / 15.11.93

Description  : 
contains function for reading and converting data from PhonDat files 
version 2. You need an ANSI C compiler to use this module. As the data 
formats differ on various operating systems, you must choose the approriate 
function for your system.

Link modules and libraries:
header

Contained functions:
read_data_v2_vms  : reads PhonDat data under VMS operating system
                          (Header version 2 only !)
read_data_v2_sun  : reads PhonDat data under SUN operating system
                          (Header version 2 only !)
read_data_sun    : reads PhonDat data under SUN operating system

*************************************************************************/

/*----------------------------------------------------------------------
Name            : read_data_v2_vms
Module    : DATA
Title           : reads PhonDat data under VMS operating system

Description:
reads speech data from a PhonDat data file version 2 under a VMS operating 
system. the data is stored in dynamic array of signed short, the memory of
this array can be freed by using free() to the returned pointer. before
invoking this function you should read the header information of the data
file by the function read_header_v2_vms() in module phondat_header and 
give the pointer to the header structure Phon_header_2 to this function. the
functions returns the pointer to the speech data and the number of read
samples, if an error occurs it writes an error message to stderr and
returns NULL. 

Parameters:
header    : pointer to header structure version 2 
samples    : pointer to # of read samples 
file    : pointer to PhonDat file

Return-Value    : pointer to speech data
      Null if error occurs
-------------------------------------------------------------------------*/
short *read_data_v2_vms(Phon_header_2 *header,int *samples,FILE *file)
{ 
  short *data;

/* check if header structure is present          */
    if(header == NULL)
    {
  fprintf(stderr,"read_data_v2_vms: no header structure present\n");
  return(NULL);
    }

/* check version in header structure          */
    if(header->version != 2)
    {
  fprintf(stderr,"read_data_v2_vms: no header structure version 2\n");
  return(NULL);
    }

/* read # of header blocks (512 byte), skip file pointer to first data  */
    if(fseek(file,header->anz_header*512,0) == EOF)
    {
  fprintf(stderr,"read_data_v2_vms: error operating with filepointer\n");
  perror("read_data_v2_vms");
  return(NULL);
    }

printf("read_data_v2_vms: file pointer set to %d\n",ftell(file));

/* allocate memory for speech data          */
    *samples = header->nspbk*256;

printf("read_data_v2_vms: number of data samples: %d\n",*samples);

    if((data = (short *)calloc(*samples,sizeof(short))) == NULL)
    {
  fprintf(stderr,
  "read_data_v2_vms: cannot allocate memory for speech data\n");
  perror("read_data_v2_vms");
  return(NULL);
    }

/* read speech data              */
    if(fread(data,sizeof(short),*samples,file) != (size_t)(*samples))
    {
  fprintf(stderr,
  "read_data_v2_vms: error reading speech data from file\n");
  perror("read_data_v2_vms");
  free((char *)data);
  return(NULL);
    }

    return(data);

} /* end subroutine : read_data_v2_vms */

/*----------------------------------------------------------------------
Name            : read_data_v2_sun
Module    : DATA
Title           : reads PhonDat data under SUN operating system

Description:
reads speech data from a PhonDat data file version 2 under a SUN operating 
system. the data is stored in dynamic array of signed short, the memory of
this array can be freed by using free() to the returned pointer. before
invoking this function you should read the header information of the data
file by the function read_header_v2_sun() in module phondat_header and give the
pointer to the header structure Phon_header_2 to this function. the
functions returns the pointer to the speech data and the number of read
samples, if an error occurs it writes an error message to stderr and
returns NULL. 

Parameters:
header    : pointer to header structure version 2 
samples    : pointer to # of read samples 
file    : pointer to PhonDat file

Return-Value    : pointer to speech data
      Null if error occurs
-------------------------------------------------------------------------*/
short *read_data_v2_sun(Phon_header_2 *header,int *samples,FILE *file)
{ 
  char buff,*charpoint;
  short *data;
  int i;

/* check if header structure is present          */
    if(header == NULL)
    {
  fprintf(stderr,"read_data_v2_sun: no header structure present\n");
  return(NULL);
    }

/* check version in header structure          */
    if(header->version != 2)
    {
  fprintf(stderr,"read_data_v2_sun: no header structure version 2\n");
  return(NULL);
    }

/* read # of header blocks (512 byte), skip file pointer to first data  */
    if(fseek(file,header->anz_header*512,0) == EOF)
    {
  fprintf(stderr,"read_data_v2_sun: error operating with filepointer\n");
  perror("read_data_v2_sun");
  return(NULL);
    }

/* allocate memory for speech data          */
    *samples = header->nspbk*256;

    if((data = (short *)calloc(*samples,sizeof(short))) == NULL)
    {
  fprintf(stderr,
  "read_data_v2_sun: cannot allocate memory for speech data\n");
  perror("read_data_v2_sun");
  return(NULL);
    }

/* read speech data              */
    if(fread(data,sizeof(short),*samples,file) != (size_t)(*samples))
    {
  fprintf(stderr,
  "read_data_v2_sun: error reading speech data from file\n");
  perror("read_data_v2_sun");
  free((char *)data);
  return(NULL);
    }

/* conversion to sun: change higher and lower byte in each sample  */
    charpoint = (char *)data;
    for(i=0;i<*samples;i++)
    {
  buff = charpoint[0];
  charpoint[0] = charpoint[1];
  charpoint[1] = buff;
  charpoint += 2;
    }  

    return(data);

} /* end subroutine : read_data_v2_sun */

/*----------------------------------------------------------------------
Name            : read_data_sun
Module    : DATA
Title           : reads PhonDat data under SUN operating system

Description:
reads speech data from a PhonDat data file version 1 or 2 under a SUN operating 
system. The data is stored in dynamic array of signed short, the memory of
this array can be freed by using free() to the returned pointer. before
invoking this function you should read the header information of the data
file by the function read_header_sun() in module phondat_header and give the
pointer to the header structure Phon_header_2 to this function. the
functions returns the pointer to the speech data and the number of read
samples, if an error occurs it writes an error message to stderr and
returns NULL. 

Parameters:
header    : pointer to header structure version 2 
samples    : pointer to # of read samples 
file    : pointer to PhonDat file

Return-Value    : pointer to speech data
      Null if error occurs
-------------------------------------------------------------------------*/
short *read_data_sun(Phon_header_2 *header,int *samples,FILE *file)
{ 
  char buff,*charpoint;
  short *data;
  int i;

/* check if header structure is present          */
    if(header == NULL)
    {
  fprintf(stderr,"read_data_sun: no header structure present\n");
  return(NULL);
    }

/* check version in header structure and skip the header blocks  */
    if(header->version == 1)
      fseek(file,512,0);
    if(header->version == 2)
      fseek(file,header->anz_header*512,0);

/* allocate memory for speech data          */
    *samples = header->nspbk*256;

    if((data = (short *)calloc(*samples,sizeof(short))) == NULL)
    {
  fprintf(stderr,
  "read_data_sun: cannot allocate memory for speech data\n");
  perror("read_data_sun");
  return(NULL);
    }

/* read speech data              */
    if(fread(data,sizeof(short),*samples,file) != (size_t)(*samples))
    {
  fprintf(stderr,
  "read_data_sun: error reading speech data from file\n");
  perror("read_data_sun");
  free((char *)data);
  return(NULL);
    }

/* conversion to sun: change higher and lower byte in each sample  */
    charpoint = (char *)data;
    for(i=0;i<*samples;i++)
    {
  buff = charpoint[0];
  charpoint[0] = charpoint[1];
  charpoint[1] = buff;
  charpoint += 2;
    }  

    return(data);

} /* end subroutine : read_data_sun */

/*----------------------------------------------------------------------
Name            : read_data_vms
Module    : DATA
Title           : reads PhonDat data under VMS operating system

Description:
reads speech data from a PhonDat data file version 1 or 2 under a VMS operating 
system. The data is stored in dynamic array of signed short, the memory of
this array can be freed by using free() to the returned pointer. before
invoking this function you should read the header information of the data
file by the function read_header_sun() in module phondat_header and give the
pointer to the header structure Phon_header_2 to this function. the
functions returns the pointer to the speech data and the number of read
samples, if an error occurs it writes an error message to stderr and
returns NULL. 

Parameters:
header    : pointer to header structure version 2 
samples    : pointer to # of read samples 
file    : pointer to PhonDat file

Return-Value    : pointer to speech data
      Null if error occurs
-------------------------------------------------------------------------*/
short *read_data_vms(Phon_header_2 *header,int *samples,FILE *file)
{ 
  short *data;

/* check if header structure is present          */
    if(header == NULL)
    {
  fprintf(stderr,"read_data_vms: no header structure present\n");
  return(NULL);
    }

/* check version in header structure and skip the header blocks  */
    if(header->version == 1)
      fseek(file,512,0);
    if(header->version == 2)
      fseek(file,header->anz_header*512,0);

/* allocate memory for speech data          */
    *samples = header->nspbk*256;

    if((data = (short *)calloc(*samples,sizeof(short))) == NULL)
    {
  fprintf(stderr,
  "read_data_vms: cannot allocate memory for speech data\n");
  perror("read_data_vms");
  return(NULL);
    }

/* read speech data              */
    if(fread(data,sizeof(short),*samples,file) != (size_t)(*samples))
    {
  fprintf(stderr,
  "read_data_vms: error reading speech data from file\n");
  perror("read_data_vms");
  free((char *)data);
  return(NULL);
    }

    return(data);

} /* end subroutine : read_data_vms */
