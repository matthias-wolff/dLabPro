/* dLabPro program recognizer (dLabPro recognizer)
 * - Floating type mapping
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
#ifndef _RECOGNIZER_H
#define _RECOGNIZER_H

#include "dlp_config.h"

#if __RECO_FTYPE_CODE == T_FLOAT
  #define RECO_FTYPE  FLOAT32
  #define SSMG_FTYPE  FLOAT32
#elif __RECO_FTYPE_CODE == T_DOUBLE
  #define RECO_FTYPE  FLOAT64
  #define SSMG_FTYPE  FLOAT64
#else
  #error RECO_FTYPE must be T_FLOAT or T_DOUBLE
#endif

#endif
