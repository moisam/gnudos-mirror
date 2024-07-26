/* 
 *    Programmed By: Mohammed Isam Mohammed [mohammed_isam1984@yahoo.com]
 *    Copyright 2014, 2015, 2016, 2017, 2018 (c)
 * 
 *    file: file.h
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
 *    along with mino. If not, see <http://www.gnu.org/licenses/>.
 */    
#ifndef __FILE_MENU_H
#define __FILE_MENU_H

#include "defs.h"
#include <string.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

int checkFileModified(char *open_file_name);
char *fileMenu_New(char *open_file_name);
char *fileMenu_Open(char *open_file_name);
char *fileMenu_Save(char* open_file_name);
char *fileMenu_SaveAs(char *open_file_name);
char *fileMenu_Print(char* open_file_name);
char *fileMenu_Exit(char* open_file_name);

#endif
