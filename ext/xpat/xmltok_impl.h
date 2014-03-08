/* dLabPro EXTERNAL SOURCE
 * - EXPAT library - Header file
 *
 * AUTHOR  : Thai Open Source Software Center Ltd
 * UPDATE  : $Date: 2013-12-03 11:43:12 +0100 (Di, 03 Dez 2013) $, $Author: wolff $
 *           $Revision: 2003 $
 * PACKAGE : dlabpro/base
 * RCS-ID  : $Id: xmltok_impl.h 2003 2013-12-03 10:43:12Z wolff $
 */ 

/*
Copyright (c) 1998, 1999 Thai Open Source Software Center Ltd
See the file COPYING for copying permission.
*/

enum {
  BT_NONXML,
  BT_MALFORM,
  BT_LT,
  BT_AMP,
  BT_RSQB,
  BT_LEAD2,
  BT_LEAD3,
  BT_LEAD4,
  BT_TRAIL,
  BT_CR,
  BT_LF,
  BT_GT,
  BT_QUOT,
  BT_APOS,
  BT_EQUALS,
  BT_QUEST,
  BT_EXCL,
  BT_SOL,
  BT_SEMI,
  BT_NUM,
  BT_LSQB,
  BT_S,
  BT_NMSTRT,
  BT_COLON,
  BT_HEX,
  BT_DIGIT,
  BT_NAME,
  BT_MINUS,
  BT_OTHER, /* known not to be a name or name start character */
  BT_NONASCII, /* might be a name or name start character */
  BT_PERCNT,
  BT_LPAR,
  BT_RPAR,
  BT_AST,
  BT_PLUS,
  BT_COMMA,
  BT_VERBAR
};

#include <stddef.h>
