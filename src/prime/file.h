/* 
 *    Programmed By: Mohammed Isam Mohammed [mohammed_isam1984@yahoo.com]
 *    Copyright 2013, 2014, 2015, 2016, 2017, 2018, 2024 (c)
 * 
 *    file: file.h
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
#ifndef __FILE_H
#define __FILE_H

void fileMenu_CreateDir();
void fileMenu_Open();
void fileMenu_ExportTree();
void fileMenu_Print();
void fileMenu_Exit();
char *file_open_location();
void purgeLogFile(FILE *logfile);

//function called by exportTree() function
void scanThisDir(char *tmp, FILE *out, int level, int showProgress);

void exportTree(void); 
void exportTreeFromCommandLine(char *d, char *f);

#endif
