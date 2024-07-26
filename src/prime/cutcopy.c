/* 
 *    Programmed By: Mohammed Isam Mohammed [mohammed_isam1984@yahoo.com]
 *    Copyright 2013, 2014, 2015, 2016, 2017, 2018, 2024 (c)
 * 
 *    file: cutcopy.c
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

#include <string.h>
#include <stdlib.h>
#include "defs.h"
#include "cutcopy.h"
#include "dirstruct.h"

int numCutFiles;
int numCopyFiles;
int numCutDirs;
int numCopyDirs;

//variables used in editing of dirs and files//
int numStarred;    //number of items starred (marked)
int numCut;        //number of items set to be cut
int numCopy;       //number of items selected for copy

#define MAX_CUT_COPY    256
char *cutFiles [MAX_CUT_COPY];
char *cutDirs  [MAX_CUT_COPY];
char *copyFiles[MAX_CUT_COPY];
char *copyDirs [MAX_CUT_COPY];

/*
 * Helper function.
 */
static char *makeFullPath(struct dirstruct *entry)
{
    char *name, *str;
    size_t namelen;

    // if the name is widechar, convert to a C-string
    if(entry->iswcs) name = to_cstring(entry->name, &namelen);
    else
    {
        name = entry->name;
        namelen = entry->namelen;
    }

    if(!name) return NULL;

    if(!(str = malloc(namelen+cwdlen+2)))
    {
        if(name != entry->name) free(name);
        return NULL;
    }

    strcpy(str, cwd);
    strcat(str, "/");
    strcat(str, name);

    if(name != entry->name) free(name);

    return str;
}

/************************************************
 * this function checks if a directory is set
 * to be cut or copied. Pass it the number of
 * the directory 'i' in the global dirs[] array.
 * RETURNS:
 *     1 if the dir is set to be cut
 *     2 if the dir is set to be copied
 *     0 if neither
 * **********************************************/
int checkCutOrCopyDir(int i)
{
    if(numCutDirs == 0 && numCopyDirs == 0) return 0;

    int j;
    char *str = makeFullPath(dirs[i]);

    if(!str) return 0;

    for(j = 0; j < numCutDirs; j++) 
    {
        if(strcmp(cutDirs[j], str) == 0) { free(str); return 1; } 
    }
    
    for(j = 0; j < numCopyDirs; j++) 
    {
        if(strcmp(copyDirs[j], str) == 0) { free(str); return 2; }
    }

    free(str);
    return 0;
}

/************************************************
 * this function checks if a file is set
 * to be cut or copied. Pass it the number of
 * the file 'i' in the global files[] array.
 * RETURNS:
 *     1 if the file is set to be cut
 *     2 if the file is set to be copied
 *     0 if neither
 * **********************************************/
int checkCutOrCopyFile(int i) 
{
    if(numCutFiles == 0 && numCopyFiles == 0) return 0;

    int j;
    char *str = makeFullPath(files[i]);

    if(!str) return 0;

    for(j = 0; j < numCutFiles; j++) 
    {
        if(strcmp(cutFiles[j], str) == 0) { return 1; free(str); }
    }

    for(j = 0; j < numCopyFiles; j++) 
    {
        if(strcmp(copyFiles[j], str) == 0) { return 2; free(str); }
    }

    free(str);
    return 0;
}

void __removeCutCopyItem(struct dirstruct **fileDirItems, int fileDirIndex,
                         char **cutCopyItems, int *cutCopyItemCount)
{
    if(*cutCopyItemCount == 0) return;

    int j, k;
    char *str = makeFullPath(fileDirItems[fileDirIndex]);

    if(!str)
    {
        msgBoxH("Insufficient memory", BUTTON_OK, ERROR);
        return;
    }

    for(j = 0; j < *cutCopyItemCount; j++) 
    {
        if(strcmp(str, cutCopyItems[j]) == 0)
        {
            //found a match
            free(cutCopyItems[j]);

            for(k = j; k < (*cutCopyItemCount)-1; k++) 
            {
                cutCopyItems[k] = cutCopyItems[k+1];
            }

            (*cutCopyItemCount)--;
            cutCopyItems[k] = NULL;
            break;
        }
    }

    free(str);
}

/*****************************************************
 * this function removes a file name from the 
 * array of files set to be cut 'cutFiles[]'. Pass 
 * it the file number 'i' in the global files[] array.
 * ***************************************************/
void removeCutFile(int i) 
{
    __removeCutCopyItem(files, i, cutFiles, &numCutFiles);
}

/*****************************************************
 * this function removes a file name from the 
 * array of files set to be copied 'copyFiles[]'. Pass 
 * it the file number 'i' in the global files[] array.
 * ***************************************************/
void removeCopyFile(int i) 
{
    __removeCutCopyItem(files, i, copyFiles, &numCopyFiles);
}

/*****************************************************
 * this function removes a dir name from the 
 * array of dirs set to be cut 'cutDirs[]'. Pass 
 * it the dir number 'i' in the global dirs[] array.
 * ***************************************************/
void removeCutDir(int i) 
{
    __removeCutCopyItem(dirs, i, cutDirs, &numCutDirs);
}

/*****************************************************
 * this function removes a dir name from the 
 * array of dirs set to be copied 'copyDirs[]'. Pass 
 * it the dir number 'i' in the global dirs[] array.
 * ***************************************************/
void removeCopyDir(int i) 
{
    __removeCutCopyItem(dirs, i, copyDirs, &numCopyDirs);
}

void __saveCutCopyItem(struct dirstruct **fileDirItems, int fileDirIndex,
                         char **cutCopyItems, int *cutCopyItemCount)
{
    int k = *cutCopyItemCount;

    if(k >= MAX_CUT_COPY)
    {
        msgBoxH("Unable to perform operation.\nClipboard is full.", BUTTON_OK, ERROR);
        return;
    }

    if(cutCopyItems[k]) free(cutCopyItems[k]);

    char *str = makeFullPath(fileDirItems[fileDirIndex]);

    if(!str)
    {
        msgBoxH("Insufficient memory", BUTTON_OK, ERROR);
        return;
    }

    cutCopyItems[k] = str;
    (*cutCopyItemCount)++;
}

/*****************************************************
 * this function adds a file to the array of files set 
 * to be cut 'cutFiles[]'. Pass it the file number 'i' 
 * in the global files[] array.
 * ***************************************************/
void saveCutFile(int i) 
{
    __saveCutCopyItem(files, i, cutFiles, &numCutFiles);
}

/*****************************************************
 * this function adds a file to the array of files set 
 * to be copied 'copyFiles[]'. Pass it the file number 
 * 'i' in the global files[] array.
 * ***************************************************/
void saveCopyFile(int i) 
{
    __saveCutCopyItem(files, i, copyFiles, &numCopyFiles);
}

/*****************************************************
 * this function adds a dir to the array of dirs set 
 * to be cut 'cutDirs[]'. Pass it the dir number 'i' 
 * in the global dirs[] array.
 * ***************************************************/
void saveCutDir(int i) 
{
    __saveCutCopyItem(dirs, i, cutDirs, &numCutDirs);
}

/*****************************************************
 * this function adds a dir to the array of dirs set 
 * to be copied 'copyDirs[]'. Pass it the dir number 'i' 
 * in the global dirs[] array.
 * ***************************************************/
void saveCopyDir(int i) 
{
    __saveCutCopyItem(dirs, i, copyDirs, &numCopyDirs);
}

