/* 
 *    Programmed By: Mohammed Isam Mohammed [mohammed_isam1984@yahoo.com]
 *    Copyright 2013, 2014, 2015, 2016, 2017, 2018, 2024 (c)
 * 
 *    file: defs.h
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

/**********************************************
  This file contains definitions for constants 
  and functions used by the Prime program.
 **********************************************/
#ifndef __Prime_H
#define __Prime_H

#include <stdio.h>
#include <errno.h>
#include <dirent.h>
#include <sys/stat.h>
#include "dialogs.h"
#include "kbd.h"

#define MAXENTRIES      2048

//Main menu items//
#define totalMainMenus  4    //total items in main menu bar
#define fTotal          5    //total items in file menu
#define eTotal          7    //total items in edit menu
#define oTotal          3    //total items in options menu
#define hTotal          4    //total items in help menu

//values used in the activeWindow variable//
#define DIR_WIN         1
#define FILE_WIN        2
#define FILE_MENU       3
#define EDIT_MENU       4
#define HELP_MENU       5

// defined in init.c
extern int MAX_DIR_NAME_LEN;
extern int MAX_FILE_NAME_LEN;

// defined in main.c
extern int numVisDirs, firstVisDir, selectedDir, totalDirs;
extern int numVisFiles, firstVisFile, selectedFile, totalFiles;
extern char *dirHighLight;
extern char *fileHighLight;
extern int activeWindow;
extern char *cwd;
extern int cwdlen;
extern char FILE_DIR_COLOR[26];

// defined in menu.c
extern char *menu[4];
extern char *fileMenu[6];
extern char *editMenu[8];
extern char *optionsMenu[3];
extern char *helpMenu[5];

// defined in options.c
extern int GNU_DOS_LEVEL;

// defined in args.c
extern char *prime_ver;
extern char *copyright_notice;

static int one(const struct dirent *unused) 
{
    return 1;
}

/******************************
 * Function prototypes
 ******************************/

// init.c
void init(void);
int read_config_file(void);
int write_config_file_defaults(void);

// main.c
void scanDir(char *dir);
void drawMenuBar(int x, int y, int w);
void showErrorMsgBox(char *msg, char *arg);
void toggleSelected(void);
void exit_gracefully(void);
void refreshAll(void);
void refreshWindows(void);
void refreshDirView(void);
void refreshFileView(void);
void refreshBottomView(void);

// properties.c
void showPropertiesDialog(void);

// menu.c
void showMenu(int index);

// args.c
void parse_args(int argc, char **argv);

#endif
