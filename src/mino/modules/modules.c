/* 
 *    Programmed By: Mohammed Isam Mohammed [mohammed_isam1984@yahoo.com]
 *    Copyright 2018 (c)
 * 
 *    file: modules.c
 *    This file is part of mino.
 *
 *    mino is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    mino is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with mino.  If not, see <http://www.gnu.org/licenses/>.
 */    
#include "modules.h"

extern struct modulestruct module_c;
extern struct modulestruct module_cpp;
extern struct modulestruct module_perl;
extern struct modulestruct module_sh;
extern struct modulestruct module_texi;
extern struct modulestruct module_asm;
extern struct modulestruct module_py;
extern struct modulestruct module_html;
extern struct modulestruct module_js;
extern struct modulestruct module_bas;
extern struct modulestruct module_pas;
extern struct modulestruct module_f77;

struct modulestruct *module[NUM_MODULES] =
{
    &module_c,
    &module_cpp,
    &module_perl,
    &module_sh,
    &module_texi,
    &module_asm,
    &module_py,
    &module_html,
    &module_js,
    &module_bas,
    &module_pas,
    &module_f77,
};

struct modulestruct dummyModule = 
{
    .extsCount      = 0,
    .exts           = NULL,
    .keywordCount   = 0,
    .keywords       = NULL,
    .mlCommentStart = NULL,
    .mlCommentEnd   = NULL,
    .slCommentStart = NULL,
    .caseSensitive  = 0,
};

struct modulestruct *curmodule = &dummyModule;
