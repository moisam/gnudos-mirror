/* 
 *    Programmed By: Mohammed Isam Mohammed [mohammed_isam1984@yahoo.com]
 *    Copyright 2014, 2015, 2016, 2017, 2018, 2024 (c)
 * 
 *    file: defs.h
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
#ifndef __Mino_PRO_H
#define __Mino_PRO_H

#include <stdio.h>
#include <errno.h>
#include <wchar.h>

#define MAXDIRS             1024
#define MAXFILES            1024

#define __DEFAULT_TITLE     "untitled"

#define MAX_LINES           5000

#define TABSPACES(index)    (TAB_CHARS-((TAB_CHARS+(index))%TAB_CHARS))

//Menu items//
#define totalMainMenus  4   //total items in main menu bar
#define fTotal          6   //total items in file menu
#define eTotal          10  //total items in mino menu
#define oTotal          4   //total items in options menu
#define hTotal          4   //total items in help menu

//enumeration determining the state of the open file
enum fstate { MODIFIED, NEW, SAVED, OPENED, IDLE };

//openSave determines if the function is to open or save a file
//showDialog: if YES, show open dialogbox, if NO, don't
typedef enum opensaveE { OPEN, SAVE } OPEN_SAVE;

struct linestruct
{
    wchar_t *text;
    int  charCount;
    int  linkedToNext;
    int  charsAlloced;
};

// defined in linestruct.c
extern struct linestruct *lines[MAX_LINES];

// defined in opensave.c
extern int MAX_DIR_NAME_LEN;
extern wchar_t *documentTitle;    //document title is the file name

// defined in msgbox.c
extern int MAX_MSG_BOX_W;
extern int MAX_MSG_BOX_H;

// defined in file.c
extern int NEW_FILE;
extern enum fstate FILE_STATE;

// defined in menu.c
extern char *menu[4];
extern char *fileMenu[6];
extern char *editMenu[10];
extern char *optionsMenu[4];
extern char *helpMenu[4];

// defined in options.c
extern int WRAP_LINES;
extern int TAB_CHARS;
extern int GNU_DOS_LEVEL;

// defined in init.c
extern int  open_file_at_startup;
extern wchar_t *DEFAULT_TITLE;
extern int  SHOW_README;
extern char *copyrightNotice;

// defined in main.c
extern int MAX_CHARS_PER_LINE;
extern int maxLen;
extern int firstVisLine;
extern int totalVisLines;
extern int totalLines;
extern int selectedLine;
extern int selectedChar;
extern int selectedCharCarry;
extern int userVisibleCurLine;
extern int selectedCharIfPossible;

/**********************************
 * Function prototypes
 **********************************/

// init.c
void init(char *open_file_name);
void parseLineArgs(int argc, char **argv);
void showREADMEOnStartup(void);
int  initNewDocument(void);
int __initNewDocument(void);
void resetLineCounters(void);

// main.c
void sighandler(int signo);
void deleteLine(void);
void copyInLine(int pos, int to, int from, int calcTotalChars);
void checkLineBounds(int pos);
int extendLineText(int pos, int newSize);
void calcCharCarry(int pos);
void calcUserVisibleCurLine(void);

void refreshSelectedLine(void);
void refreshSelectedLineInColor(int pos, int *incomment);
void refreshView(void);
void refreshViewLines(int start, int end, int startOutputAt);
void refreshBottomView(void);

void move_lines_up(int first, int last);
void move_lines_upd(int first, int diff);
void move_lines_down(int first, int last);
void move_lines_downl(int first, int last, wchar_t *newLineText);

// opensave.c
void checkFileExtension(char *open_file_name);
char *openSaveFile(OPEN_SAVE openSave, int showDialog, char *open_file_name);
void initDirView(void);

// dir.c
int scanDir(char *dir, char ***dirs, char ***files, 
            int *totalDirs, int *totalFiles);

// linestruct.c
struct linestruct *allocLineStruct(void);
struct linestruct *allocLineStructB(int chars);
void copyLineStruct(int pos1, int pos2);
void freeLineStruct(struct linestruct *line);

// menu.c
void showMenu(int index);

#endif
