/* dLabPro program dcg (dLabPro code generator)
 * - Build configuration
 *
 * AUTHOR : Matthias Wolff
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

#ifndef __DLP_CONFIG_H
#define __DLP_CONFIG_H

/*#define __NOITP         / * Do not register methods in object's dictionary */
/*#define __NORTTI        / * Do not register anything (Warning: some object functions will not work correctly) */
#define __NOXMLSTREAM                                                           /* No XML serialization              */
#define __NODN3STREAM                                                           /* No DNorm3 serialization           */
#define __UNENTANGLE_FST                                                        /* Stand alone class CFst            */
#ifndef __NOREADLINE                                                            /* If not defined already >>         */
  #define __NOREADLINE                                                          /*   Do not use readline library     */
#endif /* #ifndef __NOREADLINE */                                               /* <<                                */
#define __NOZLIB                                                                /* Do not use file compression (zlib)*/
#define __NODLPMATH                                                             /* Do not use dlpmath library        */

#endif /* #ifndef __DLP_CONFIG_H */

/* EOF */
