/* 
 *    Programmed By: Mohammed Isam Mohammed [mohammed_isam1984@yahoo.com]
 *    Copyright 2018, 2024 (c)
 * 
 *    file: dirstruct.c
 *    This file is part of Prime.
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

#include <stdlib.h>
#include "dirstruct.h"

struct dirstruct *dirs[MAXENTRIES];
struct dirstruct *files[MAXENTRIES];


void freeDirStruct(struct dirstruct *dir)
{
    if(!dir) return;
    if(dir->name) free(dir->name);
    free(dir);
}

void freeDirStructs(int start, int end)
{
    int j;

    for(j = start; j < end; j++)
    {
        freeDirStruct(dirs[j]);
        dirs[j] = NULL;
    }
}

void freeFileStructs(int start, int end)
{
    int j;

    for(j = start; j < end; j++)
    {
        freeDirStruct(files[j]);
        files[j] = NULL;
    }
}

struct dirstruct *allocDirStruct(void)
{
    struct dirstruct *dir = malloc(sizeof(struct dirstruct));
    if(!dir) return NULL;
    dir->name = NULL;
    dir->namelen = 0;
    dir->star = ' ';
    dir->type = 'd';
    dir->iswcs = 0;
    return dir;
}

struct dirstruct *allocDirStructB(int bytes)
{
    struct dirstruct *dir = allocDirStruct();
    if(!dir) return NULL;
    dir->name = malloc(bytes);
    if(!dir->name) return NULL;
    return dir;
}

struct dirstruct *allocFileStruct(void)
{
    struct dirstruct *file = malloc(sizeof(struct dirstruct));
    if(!file) return NULL;
    file->name = NULL;
    file->namelen = 0;
    file->star = ' ';
    file->type = 'r';
    file->iswcs = 0;
    return file;
}

struct dirstruct *allocFileStructB(int bytes)
{
    struct dirstruct *dir = allocFileStruct();
    if(!dir) return NULL;
    dir->name = malloc(bytes);
    if(!dir->name) return NULL;
    return dir;
}

