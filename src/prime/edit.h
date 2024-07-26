/* 
 *    Programmed By: Mohammed Isam Mohammed [mohammed_isam1984@yahoo.com]
 *    Copyright 2013, 2014, 2015, 2016, 2017, 2018, 2024 (c)
 * 
 *    file: edit.h
 *    This file contains function prototypes and variable declarations
 *    for the edit.c file. This file is part of Prime.
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
#ifndef __EDIT_H
#define __EDIT_H

//functions of edit//
void cutMarked(void);
void copyMarked(void);
void pasteMarked(void);
void deleteMarked(void);
void markAll(int activeWindow);
void unMarkAll(int activeWindow);
void clearSelection(void);
void deleteThisDir(char *tmp, int level, FILE *logfile);

void editMenu_Find(void);
void editMenu_Properties(void);

#endif
