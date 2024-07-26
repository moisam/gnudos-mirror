/* 
 *    Programmed By: Mohammed Isam Mohammed [mohammed_isam1984@yahoo.com]
 *    Copyright 2013, 2014, 2015, 2016, 2017, 2018, 2024 (c)
 * 
 *    file: find.h
 *    This file contains function prototypes and variable declarations
 *    for the find.c file. This file is part of Prime.
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
#ifndef __FIND_H
#define __FIND_H

#include "defs.h"
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>

//max length of the searched file name
#define MAX_INPUT1_LEN  43

//max length of the path to search in
#define MAX_INPUT2_LEN  23

void findFile(void);

#endif
