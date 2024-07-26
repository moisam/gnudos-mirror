/* 
 *    Programmed By: Mohammed Isam Mohammed [mohammed_isam1984@yahoo.com]
 *    Copyright 2013, 2014, 2015, 2016, 2017, 2018, 2024 (c)
 * 
 *    file: cutcopy.h
 *    This file contains function prototypes and variable declarations
 *    for the cutcopy.c file. This file is part of Prime.
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
#ifndef __CUT_COPY_H
#define __CUT_COPY_H

extern int numCutFiles;
extern int numCopyFiles;
extern int numCutDirs;
extern int numCopyDirs;

extern int numStarred;
extern int numCut;
extern int numCopy;

#define MAX_CUT_COPY    256
extern char *cutFiles [MAX_CUT_COPY];
extern char *cutDirs  [MAX_CUT_COPY];
extern char *copyFiles[MAX_CUT_COPY];
extern char *copyDirs [MAX_CUT_COPY];

void saveCutFile(int i);
void saveCopyFile(int i);
void saveCutDir(int i);
void saveCopyDir(int i);
void removeCutFile(int i);
void removeCopyFile(int i);
void removeCutDir(int i);
void removeCopyDir(int i);
int checkCutOrCopyDir(int i);
int checkCutOrCopyFile(int i);

#endif
