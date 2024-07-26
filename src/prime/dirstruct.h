/* 
 *    Programmed By: Mohammed Isam Mohammed [mohammed_isam1984@yahoo.com]
 *    Copyright 2018, 2024 (c)
 * 
 *    file: dirstruct.h
 *
 *    Prime is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    Prime is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with Prime.  If not, see <http://www.gnu.org/licenses/>.
 */    
#ifndef __DIRSTRUCT_H
#define __DIRSTRUCT_H

#include "defs.h"

struct dirstruct
{
    char *name;
    size_t namelen;
    char star;
    char type;
    char iswcs;     //1=name is widechare, 0=name is "normal" C-string
};

extern struct dirstruct *dirs[MAXENTRIES];
extern struct dirstruct *files[MAXENTRIES];

void freeDirStruct(struct dirstruct *dir);
void freeDirStructs(int start, int end);
void freeFileStructs(int start, int end);
struct dirstruct *allocDirStruct(void);
struct dirstruct *allocFileStruct(void);
struct dirstruct *allocDirStructB(int bytes);
struct dirstruct *allocFileStructB(int bytes);

#endif
