/* dLabPro program recognizer (dLabPro recognizer)
 * - Recognition parameter config
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
#ifndef CONFIG_MELPROC_H_
#define CONFIG_MELPROC_H_

#define FEAPROC CMELproc
#define N_DIM 31
#define N_MID_G 4                                          /* uasr.am.gbg = 4 */
#define N_MID_S 3                                          /* uasr.am.sil = 3 */
#define N_TNAD  0.05                                       /* uasr.defnad = 0.05 */
#define N_TNED  0.75                                       /* uasr.defned = 0.60 */
#define B_FORCE FALSE

INT32 delta_table_pattern[N_DIM][2] = {
  { 1, 1 },
  { 0, 0 },
  { 1, 1 },
  { 0, 0 },
  { 1, 1 },
  { 0, 0 },
  { 1, 1 },
  { 0, 0 },
  { 1, 1 },
  { 0, 0 },
  { 1, 1 },
  { 0, 0 },
  { 1, 1 },
  { 0, 0 },
  { 1, 1 },
  { 0, 0 },
  { 1, 1 },
  { 0, 0 },
  { 1, 1 },
  { 0, 0 },
  { 1, 1 },
  { 0, 0 },
  { 1, 1 },
  { 0, 0 },
  { 1, 1 },
  { 0, 0 },
  { 1, 1 },
  { 0, 0 },
  { 1, 1 },
  { 0, 0 },
  { 1, 1 } };

#endif /*CONFIG_MELPROC_H_*/
