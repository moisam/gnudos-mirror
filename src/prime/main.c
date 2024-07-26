/* 
 *    Programmed By: Mohammed Isam Mohammed [mohammed_isam1984@yahoo.com]
 *    Copyright 2013, 2014, 2015, 2016, 2017, 2018, 2024 (c)
 * 
 *    file: main.c
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

#include "config.h"
//#define NCURSES_WIDECHAR 1
#define _GNU_SOURCE
#define _XOPEN_SOURCE_EXTENDED
#include <locale.h>
#ifdef HAVE_NCURSESW_NCURSES_H
# include <ncursesw/ncurses.h>
#else
# include <ncurses.h>
#endif
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <poll.h>
#include <signal.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include "defs.h"
#include "edit.h"
#include "options.h"
#include "find.h"
#include "cutcopy.h"
#include "file.h"
#include "dirstruct.h"

#define DIRCOLOR(x)     (dirs [(x)]->type-'a')
#define FILECOLOR(x)    (files[(x)]->type-'a')
#define CURDIR          (firstVisDir+selectedDir)
#define CURFILE         (firstVisFile+selectedFile)

static sig_atomic_t end = 0;

int numVisDirs, firstVisDir, selectedDir, totalDirs;
int numVisFiles, firstVisFile, selectedFile, totalFiles;

//the hightlight bar to delineate the selected dir
char *dirHighLight;
//the hightlight bar to delineate the selected file
char *fileHighLight;
//int value indicating which window is active
int activeWindow;

//Array to store different colors that will be given
//to files of different types.. The array is indexed by
//a character indicating the type of file/dir.. Currently,
//the chars are:
//'d' for directories
//'x' for executable files
//'r' for regular files
//'l' for links
//'a' for archives
//'h' for hidden files
//'p' for picture files
//'%' special case, used to indicate that the directory is empty
char FILE_DIR_COLOR[26];

char *cwd;      // string to use when changing current working directory
int cwdlen;     // length of that string

#ifdef HAVE_NCURSESW_NCURSES_H
wchar_t *cwd_widechar;  // wide char version for output to ncurses
size_t cwdlen_widechar; // length of that string
#endif


void exit_gracefully(void)
{
    write_config_file();
    fcloseall();
    showCursor();
    setScreenColors(WHITE, BGDEFAULT);
    clearScreen();
    reset_attribs();
    //fprintf(stdout, "\x1b[0m");    /* reset settings */
    restoreTerminal();
    exit(0);
}

void update_cwd_pointers(void)
{
    if(cwd) free(cwd);
    cwd = getcwd(NULL, 0);
    if(cwd) cwdlen = strlen(cwd);

#ifdef HAVE_NCURSESW_NCURSES_H
    if(cwd_widechar) free(cwd_widechar);
    cwd_widechar = to_widechar(cwd, &cwdlen_widechar);
#endif
}

void tty_resized(void)
{
    int half_SCREEN_W;

    getScreenSize();

    half_SCREEN_W = SCREEN_W/2;

    //set defaults for Dir view//
    numVisDirs  = SCREEN_H - 9;

    //reserve enough space in memory for the highlight bar
    dirHighLight = malloc(half_SCREEN_W);
    if(!dirHighLight) goto memory_error;
    memset(dirHighLight, ' ', half_SCREEN_W-4);    //fill the bar with spaces
    dirHighLight[half_SCREEN_W-4] = '\0';

    //set defaults for File view//
    numVisFiles  = SCREEN_H - 9;
    fileHighLight = malloc(half_SCREEN_W);
    if(!fileHighLight) goto memory_error;
    memset(fileHighLight, ' ', half_SCREEN_W-2);
    fileHighLight[half_SCREEN_W-2] = '\0';

    MAX_DIR_NAME_LEN = half_SCREEN_W-4;
    MAX_FILE_NAME_LEN = half_SCREEN_W-2; 

    refreshAll();
    return;
 
memory_error:

    msgBoxH("Insufficient memory", BUTTON_OK, ERROR); 
    exit_gracefully();
}

void print_dir_highlight(int selectedDir)
{
    int j, row = selectedDir+4;

    mvprintw(row-1, 2, "%s", dirHighLight);

    if(dirs[CURDIR]->namelen >= MAX_DIR_NAME_LEN)
    {
        mvaddch(row-1, 2, dirs[CURDIR]->star);

#ifdef HAVE_NCURSESW_NCURSES_H
        if(dirs[CURDIR]->iswcs)
        {
            addnwstr((wchar_t *)dirs[CURDIR]->name, MAX_DIR_NAME_LEN-3);
            addch('.');
            addch('.');
            return;
        }
#endif

        for(j = 0; j < MAX_DIR_NAME_LEN-3; j++)
            addch(dirs[CURDIR]->name[j]);

        addch('.');
        addch('.');
    }
    else
    {
        mvaddch(row-1, 2, dirs[CURDIR]->star);

#ifdef HAVE_NCURSESW_NCURSES_H
        if(dirs[CURDIR]->iswcs)
        {
            addwstr((wchar_t *)dirs[CURDIR]->name);
            return;
        }
#endif

        printw("%s", dirs[CURDIR]->name);
    }
}

void print_file_highlight(int selectedFile)
{
    int j, row = selectedFile+4, col = (SCREEN_W/2)+1;

    mvprintw(row-1, col-1, "%s", fileHighLight);

    if(files[CURFILE]->namelen >= MAX_FILE_NAME_LEN)
    {
        mvaddch(row-1, col-1, files[CURFILE]->star);

#ifdef HAVE_NCURSESW_NCURSES_H
        if(files[CURFILE]->iswcs)
        {
            addnwstr((wchar_t *)files[CURFILE]->name, MAX_FILE_NAME_LEN-3);
            addch('.');
            addch('.');
            return;
        }
#endif

        for(j = 0; j < MAX_FILE_NAME_LEN-3; j++) 
            addch(files[CURFILE]->name[j]);

        addch('.');
        addch('.');
    } 
    else 
    { 
        mvaddch(row-1, col-1, files[CURFILE]->star);

#ifdef HAVE_NCURSESW_NCURSES_H
        if(files[CURFILE]->iswcs)
        {
            addwstr((wchar_t *)files[CURFILE]->name);
            return;
        }
#endif

        printw("%s", files[CURFILE]->name);
    }
}

void showErrorMsgBox(char *msg, char *arg)
{
    char tmp[strlen(msg)+strlen(arg)+1];

    strcpy(tmp, msg);
    strcat(tmp, arg);
    msgBoxH(tmp, BUTTON_OK, ERROR); 
}

void shiftDirsUp(int pos)
{
    if(dirs[pos]->name) free(dirs[pos]->name);

    free(dirs[pos]);

    for( ; pos < totalDirs-1; pos++)
    {
        dirs[pos] = dirs[pos+1];
    }

    dirs[pos] = NULL;
    totalDirs--;
}

void shiftFilesUp(int pos)
{
    if(files[pos]->name) free(files[pos]->name);

    free(files[pos]);

    for( ; pos < totalFiles-1; pos++)
    {
        files[pos] = files[pos+1];
    }

    files[pos] = NULL;
    totalFiles--;
}

/***************************************
 * main(): 
 * Main program loop.
 * *************************************/
int main(int argc, char **argv) 
{
    int ch, i;
    char *tmp;

    setlocale(LC_ALL, "");
    //setlocale(LC_ALL, "C-UTF-8");
    parse_args(argc, argv);
    update_cwd_pointers();

    if(!cwd)
    {
        fprintf(stderr, "Error: unable to detect current working directory\n");
        exit(1);
    }

    loadDefaultColors();
    read_config_file();
    init();
    clearScreen();
    hideCursor();
    setScreenColorsI(COLOR_WINDOW);

    //draw main window
    drawBox(1, 1, SCREEN_H, SCREEN_W, " Prime File Manager ", 1);
    drawMenuBar(2, 2, SCREEN_W-2);

    //draw right sub-window
    drawBox(3, (int)(SCREEN_W/2), SCREEN_H-5, SCREEN_W-1, " File view ", 1);

    //draw bottom sub-window
    drawBox(SCREEN_H-4, 2, SCREEN_H-1, SCREEN_W-1, " Quick functions ", 1);

    //draw left sub-window
    drawBox(3, 2, SCREEN_H-5, (int)(SCREEN_W/2)-1, " Directory view ", 1);

    //redraw main window but don't clear the area
    drawBox(1, 1, SCREEN_H, SCREEN_W, " Prime File Manager ", 0);

    refreshBottomView();
    scanDir(".");

    while(!end) 
    {    //infinite program loop//
        ch = getKey();

        switch(ch)
        {
            case(KEY_RESIZE):
                tty_resized();
                break;

            case(DEL_KEY):
                if(GNU_DOS_LEVEL > 3) break;
do_del:
                if(numStarred == 0) 
                {
                    if(activeWindow == DIR_WIN)
                    {
                        if(CURDIR < 2) break;    // ignore '.' and '..'

                        if(msgBoxH("Are you sure you want to delete\n"
                                   "this directory and all its "
                                   "subdirectories?", 
                                   BUTTON_YES|BUTTON_NO, CONFIRM) == BUTTON_YES)
                        {
                            FILE *logfile = tmpfile();

                            // if the name is widechar, convert to a C-string
                            if(dirs[CURDIR]->iswcs)
                            {
                                if((tmp = 
                                      to_cstring(dirs[CURDIR]->name, NULL)))
                                {
                                    deleteThisDir(tmp, 0, logfile);
                                    free(tmp);
                                }
                            }
                            else deleteThisDir(dirs[CURDIR]->name, 0, logfile);

                            purgeLogFile(logfile);
                            shiftDirsUp(CURDIR);

                            if(CURDIR == totalDirs)
                            {
                                if(totalDirs <= numVisDirs) selectedDir--;
                                else firstVisDir--;
                            }
                        }
                    } 
                    else 
                    {
                        // ignore empty dirs
                        if(files[CURFILE]->type == '%') break;

                        if(msgBoxH("Are you sure you want to delete "
                                   "this file?", 
                                   BUTTON_YES|BUTTON_NO, CONFIRM) == BUTTON_YES)
                        {
                            // if the name is widechar, convert to a C-string
                            if(files[CURFILE]->iswcs)
                            {
                                if((tmp = 
                                      to_cstring(files[CURFILE]->name, NULL)))
                                {
                                    remove(tmp);
                                    free(tmp);
                                }
                            }
                            else remove(files[CURFILE]->name);

                            shiftFilesUp(CURFILE);

                            if(CURFILE == totalFiles)
                            {
                                if(totalFiles <= numVisFiles) selectedFile--;
                                else firstVisFile--;
                            }
                        }
                    }

                    refreshWindows();
                    refreshBottomView();
                }
                else 
                {
                    deleteMarked();
                    scanDir(cwd);
                }
                break;

            case(UP_KEY):
                if(GNU_DOS_LEVEL > 1) break;
do_up:
                if(activeWindow == DIR_WIN) 
                {
                    selectedDir--; 

                    if(selectedDir < 0) 
                    {
                        selectedDir = 0;

                        if(firstVisDir > 0)
                        {
                            firstVisDir--;
                            refreshDirView();
                        }
                    }
                    else 
                    {
                        setScreenColors(FILE_DIR_COLOR[DIRCOLOR(CURDIR+1)],
                                        BG_COLOR[COLOR_WINDOW]);
                        print_dir_highlight(selectedDir+1);
                        setScreenColorsI(COLOR_HIGHLIGHT_TEXT);
                        print_dir_highlight(selectedDir);
                    }
                    refresh();
                }
                else 
                {
                    if(files[0]->type == '%') break;

                    selectedFile--;

                    if(selectedFile < 0) 
                    {
                        selectedFile = 0;

                        if(firstVisFile > 0)
                        {
                            firstVisFile--;
                            refreshFileView();
                        }
                    }
                    else 
                    {
                        setScreenColors(FILE_DIR_COLOR[FILECOLOR(CURFILE+1)],
                                        BG_COLOR[COLOR_WINDOW]);
                        print_file_highlight(selectedFile+1);
                        setScreenColorsI(COLOR_HIGHLIGHT_TEXT);
                        print_file_highlight(selectedFile);
                    }
                    refresh();
                }
                break;

            case(DOWN_KEY):
                if(GNU_DOS_LEVEL > 1) break;
do_down:
                if(activeWindow == DIR_WIN) 
                {
                    selectedDir++;

                    if(selectedDir >= totalDirs) { selectedDir--; break; }

                    if(selectedDir >= numVisDirs) 
                    {
                        selectedDir = numVisDirs-1;

                        if((firstVisDir+numVisDirs) < totalDirs) 
                        { 
                            firstVisDir++;
                            refreshDirView(); 
                        }
                    }
                    else 
                    {
                        setScreenColors(FILE_DIR_COLOR[DIRCOLOR(CURDIR-1)],
                                        BG_COLOR[COLOR_WINDOW]);
                        print_dir_highlight(selectedDir-1);
                        setScreenColorsI(COLOR_HIGHLIGHT_TEXT);
                        print_dir_highlight(selectedDir);
                    }
                    refresh();
                }
                else 
                {
                    if(files[0]->type == '%') break;

                    selectedFile++;

                    if(selectedFile >= totalFiles) { selectedFile--; break; }

                    if(selectedFile >= numVisFiles)
                    {
                        selectedFile = numVisFiles-1;

                        if((firstVisFile+numVisFiles) < totalFiles)
                        {
                            firstVisFile++;
                            refreshFileView();
                        }
                    }
                    else 
                    {
                        setScreenColors(FILE_DIR_COLOR[FILECOLOR(CURFILE-1)],
                                        BG_COLOR[COLOR_WINDOW]);
                        print_file_highlight(selectedFile-1);
                        setScreenColorsI(COLOR_HIGHLIGHT_TEXT);
                        print_file_highlight(selectedFile);
                    }
                    refresh();
                }
                break;

            case(TAB_KEY):    //toggle active window with TAB press
                if(activeWindow == DIR_WIN) activeWindow = FILE_WIN;
                else                        activeWindow = DIR_WIN ;

                refreshFileView();
                refreshDirView();
                break;

            case(ENTER_KEY):
                if(ALT)
                {
                    if(activeWindow == FILE_WIN && files[0]->type == '%')
                        break;
                    showPropertiesDialog();
                    scanDir(cwd);
                }
                else
                {
                    if(activeWindow == DIR_WIN) 
                    {    //navigate to the selected directory
#ifdef HAVE_NCURSESW_NCURSES_H
                        if(dirs[CURDIR]->iswcs)
                        {
                            // convert widechar string to C-string
                            if((tmp = to_cstring(dirs[CURDIR]->name, NULL)))
                            {
                                scanDir(tmp);
                                free(tmp);
                                break;
                            }
                        }
#endif
                        scanDir(dirs[CURDIR]->name);
                    }
                    else
                    {
                        if(files[0]->type != '%')
                        {
                            showPropertiesDialog();
                            scanDir(cwd);
                        }
                    }
                }
                break;

            case(SPACE_KEY):        //toggle select/unselect file or directory
                toggleSelected();
                break;

            case('s'):
            case('S'):
                if(CTRL)
                {
                    if(GNU_DOS_LEVEL <= 4) break;
                    findFile();
                    hideCursor();
                    scanDir(cwd);
                }
                else goto insert_char;
                break;

            case('f'):
            case('F'):
                if(ALT) 
                {
                    showMenu(0);
                } 
                else if(CTRL) 
                {
                    if(GNU_DOS_LEVEL > 4) break;
                    findFile();
                    update_cwd_pointers();
                    scanDir(cwd);
                }
                else goto insert_char;
                break;

            case('e'):
            case('E'):
                if(CTRL) 
                {
                    if(GNU_DOS_LEVEL > 2) goto do_end;
                    exportTree();
                    hideCursor();
                    scanDir(cwd);
                }
                else if(ALT) 
                {
                    showMenu(1);
                } 
                else goto insert_char;
                break;

            case('h'):
            case('H'):
                if(ALT) 
                {
                    showMenu(3);
                }
                else goto insert_char;
                break;

            case('x'):
            case('X'):
                if(CTRL) 
                {
                    if(GNU_DOS_LEVEL > 4)
                    {
                        int k=(int)SCREEN_H/2;
                        int l=(int)SCREEN_W/2;
                        int loop = 1;

                        drawBox(k-1, l-23, k+1, l+23, NULL, 1);
                        setScreenColorsI(COLOR_HIGHLIGHT_TEXT);
                        mvprintw(k-1, l-23, "%s",
                                 "[C-c] Quit  [C-f] Open location "
                                 "[C-g] Cancel");
                        refresh();

                        while(loop)
                        {
                            ch = getKey();

                            if(!CTRL) continue;

                            if(ch == 'c') goto do_exit;
                            else if(ch == 'f')
                            {
                                file_open_location();
                                loop = 0;
                                break;
                            }
                            else if(ch == 'g')
                            {
                                refreshWindows();
                                loop = 0;
                                break;
                            }
                        }
                    }
                    else cutMarked();
                }
                else goto insert_char;
                break;

            case('v'):
            case('V'):
                if(CTRL)
                {
                    if(GNU_DOS_LEVEL > 3) break;
                    pasteMarked();
                } 
                else goto insert_char;
                break;

            case('p'):
            case('P'):
                if(CTRL)
                {
                    if(GNU_DOS_LEVEL > 1) goto do_up;
                    msgBoxH("Oops! This function is currently "
                            "not implemented.", BUTTON_OK, INFO);
                    //showPrintDialogBox();
                    refreshWindows();
                }
                else goto insert_char;
                break;

            case('a'):
            case('A'):
                if(CTRL)
                {
                    if(GNU_DOS_LEVEL > 2) goto do_home;
                    markAll(activeWindow);
                    refreshAll();
                } 
                else goto insert_char;
                break;

            case('w'):
            case('W'):
                if(CTRL) 
                {
                    if(GNU_DOS_LEVEL < 4) clearSelection();
                    else cutMarked();
                } 
                else goto insert_char;
                break;

            case('o'):
            case('O'):
                if(CTRL) 
                {
                    if(GNU_DOS_LEVEL > 4) break;
                    file_open_location();
                    refreshWindows();
                } 
                else if(ALT)
                {//open options menu
                    showMenu(2);
                }
                else goto insert_char;
                break;

            case(HOME_KEY):
                if(GNU_DOS_LEVEL > 1) break;
do_home:
                if(activeWindow == DIR_WIN) 
                {
                    firstVisDir = 0;
                    selectedDir = 0;
                    refreshDirView();
                } 
                else if(activeWindow == FILE_WIN) 
                {
                    firstVisFile = 0;
                    selectedFile = 0;
                    refreshFileView();
                }
                break;

            case(END_KEY):
                if(GNU_DOS_LEVEL > 1) break;
do_end:
                if(activeWindow == DIR_WIN) 
                {
                    if(totalDirs <= numVisDirs) 
                    {
                        selectedDir = totalDirs-1;
                    }
                    else 
                    {
                        firstVisDir = totalDirs-numVisDirs;
                        selectedDir = numVisDirs-1;
                    }

                    refreshDirView();
                } 
                else 
                {
                    if(totalFiles <= numVisFiles) 
                    {
                        selectedFile = totalFiles-1;
                    }
                    else 
                    {
                        firstVisFile = totalFiles-numVisFiles;
                        selectedFile = numVisFiles-1;
                    }

                    refreshFileView();
                }
                break;

            case('g'):
            case('G'):
                if(CTRL) 
                {
                    if(GNU_DOS_LEVEL < 3) break;
                    goto do_exit;
                } 
                else goto insert_char;
                break;

            case('d'):
            case('D'):
                if(CTRL) 
                {
                    if(GNU_DOS_LEVEL > 3) goto do_del;
                } 
                else goto insert_char;
                break;

            case('q'):
            case('Q'):
                if(CTRL) 
                {
                    if(GNU_DOS_LEVEL > 4) break;
do_exit:
                    if(msgBoxH("Are you sure you want to exit?", 
                                       BUTTON_YES | BUTTON_NO, CONFIRM) 
                                            == BUTTON_YES)
                        exit_gracefully();
                    else refreshWindows();
                }
                else goto insert_char;
                break;

            case('c'):
            case('C'):
                if(CTRL) copyMarked();
                else goto insert_char;
                break;

            case('/'):
                if(CTRL && GNU_DOS_LEVEL > 4) 
                {
                    unMarkAll(activeWindow);
                }
                break;

            case('z'):
            case('Z'):
                if(CTRL) 
                {
                    if(GNU_DOS_LEVEL > 4) break;
                    unMarkAll(activeWindow);
                } 
                else goto insert_char;
                break;

            case('n'):
            case('N'):
                if(CTRL) 
                {
                    if(GNU_DOS_LEVEL > 1) goto do_down;
                    char *res = getUserInput("Enter directory name to create:",
                                             "New Directory");
                    hideCursor();

                    if(!res) break;

                    struct stat st;

                    if(stat(res, &st) == -1) 
                    {
                        if(mkdir(res, 0775) == -1)
                        {
                            showErrorMsgBox("Error creating directory:", res);
                        }
                    }
                    else 
                    {
                        msgBoxH("Directory already exists!", BUTTON_OK, ERROR);
                    }

                    free(res);
                    scanDir(cwd);
                    refreshFileView();
                    refreshDirView();
                }
                else goto insert_char;
                break;

                ////////////////////////////////////////////////////
            default:
                //browse to the first entry starting with the entered character
insert_char:
                if(ch >= 33 && ch <= 126) 
                {
                    int i, x = -1;

                    if(activeWindow == DIR_WIN) 
                    {
                        //search from this point to the end
                        for(i = CURDIR+1; i < totalDirs; i++)
                        {
                            if(dirs[i]->name[0] == ch || 
                               dirs[i]->name[0] == ch-32)
                            {
                                x = i;
                                break;
                            }
                        }

                        //if the previous loop didn't find anything, try again
                        //starting from the top to the current point
                        if(i >= totalDirs)
                        {
                            for(i = 0; i <= CURDIR; i++)
                            {
                                if(dirs[i]->name[0] == ch || 
                                   dirs[i]->name[0] == ch-32)
                                {
                                    x = i;
                                    break;
                                }
                            }
                        }

                        //check to see if we found any result
                        if(x >= 0)
                        {
                            selectedDir = x-firstVisDir;

                            if(totalDirs <= numVisDirs)
                            {
                                refreshDirView();
                                continue;
                            }

                            if(selectedDir < 0)
                            {
                                firstVisDir += selectedDir;
                                selectedDir = 0;
                            }
                            else if(selectedDir >= numVisDirs) 
                            { 
                                firstVisDir += selectedDir-numVisDirs+1;
                                selectedDir = numVisDirs-1;
                            }

                            if(totalDirs-firstVisDir < numVisDirs) 
                            { 
                                selectedDir = firstVisDir;
                                firstVisDir = totalDirs-numVisDirs; 
                                selectedDir -= firstVisDir; 
                            } 

                            refreshDirView();
                        }
                    } 
                    else if(activeWindow == FILE_WIN) 
                    {
                        //search from this point to the end
                        for(i = CURFILE+1; i < totalFiles; i++)
                        {
                            if(files[i]->name[0] == ch || 
                               files[i]->name[0] == ch-32)
                            {
                                x = i;
                                break;
                            }
                        }

                        //if the previous loop didn't find anything, try again
                        //starting from the top to the current point
                        if(i >= totalFiles)
                        {
                            for(i = 0; i <= CURFILE; i++)
                            {
                                if(files[i]->name[0] == ch || 
                                   files[i]->name[0] == ch-32)
                                {
                                    x = i;
                                    break;
                                }
                            }
                        }

                        //check to see if we found any result
                        if(x >= 0) 
                        {
                            selectedFile = x-firstVisFile;

                            if(totalFiles <= numVisFiles)
                            {
                                refreshFileView();
                                continue;
                            }

                            if(selectedFile < 0)
                            {
                                firstVisFile += selectedFile;
                                selectedFile = 0;
                            }
                            else if(selectedFile >= numVisFiles)
                            { 
                                firstVisFile += selectedFile-numVisFiles+1;
                                selectedFile = numVisFiles-1;
                            } 

                            if(totalFiles-firstVisFile < numVisFiles) 
                            { 
                                selectedFile = firstVisFile;
                                firstVisFile = totalFiles-numVisFiles; 
                                selectedFile -= firstVisFile;
                            } 
                            refreshFileView();
                        }
                    }
                }
                break;
        }

        setScreenColorsI(COLOR_WINDOW);
    }

    exit_gracefully();
    return 0;
}


void toggleSelected(void)
{
    setScreenColorsI(COLOR_HIGHLIGHT_TEXT);

    switch(activeWindow) 
    {
        case(DIR_WIN):
            //ignore '.' and '..'
            if((strcmp(dirs[CURDIR]->name, ".") == 0) ||
               (strcmp(dirs[CURDIR]->name, "..") == 0)) break;
            
            if(dirs[CURDIR]->star == '*')
            {    //if selected, un-select
                dirs[CURDIR]->star = ' ';
                numStarred--;
            } 
            else 
            {    //otherwise, select it
                if(dirs[CURDIR]->star == '^') 
                {
                    numCut--;    //if marked for cut, remove cut
                    removeCutDir(CURDIR);
                } 
                else if(dirs[CURDIR]->star == '#') 
                {
                    numCopy--;    //if marked for copy, remove copy
                    removeCopyDir(CURDIR);
                }

                dirs[CURDIR]->star = '*';
                numStarred++;
            }

            mvaddch(selectedDir+3, 2, dirs[CURDIR]->star);
            refreshBottomView();
            break;

        case(FILE_WIN):
            if(files[CURFILE]->type == '%') break;

            if(files[CURFILE]->star == '*') 
            {   //if selected, unselect
                files[CURFILE]->star = ' ';
                numStarred--;
            } 
            else 
            {    //otherwise, select it
                if(files[CURFILE]->star == '^') 
                {
                    numCut--; //if marked for cut, remove cut
                    removeCutFile(CURFILE);
                } 
                else if(files[CURFILE]->star == '#') 
                {
                    numCopy--; //if marked for cut, remove cut
                    removeCopyFile(CURFILE);
                }

                files[CURFILE]->star = '*';
                numStarred++;
            }

            mvaddch(selectedFile+3, (SCREEN_W/2), files[CURFILE]->star);
            refreshBottomView();
            break;
    }
}

int addDirEntry(int pos, char *name)
{
#ifdef HAVE_NCURSESW_NCURSES_H
    size_t wclen = 0;
    wchar_t *wcs = to_widechar(name, &wclen);

    if(wcs)
    {
        if(!dirs[pos])
        {
            if(!(dirs[pos] = allocDirStruct())) return 0;
        }
        else free(dirs[pos]->name);

        dirs[pos]->name = (char *)wcs;
        dirs[pos]->namelen = wclen;
        dirs[pos]->iswcs = 1;
        return 1;
    }
#endif

    int len = strlen(name);

    if(!dirs[pos])
    {
        if(!(dirs[pos] = allocDirStructB(len+1))) return 0;
    }
    else
    {
        free(dirs[pos]->name);
        if(!(dirs[pos]->name = malloc(len+1))) return 0;
    }

    strcpy(dirs[pos]->name, name);
    dirs[pos]->namelen = len;
    dirs[pos]->iswcs = 0;
    return 1;
}

int addFileEntry(int pos, char *name)
{
#ifdef HAVE_NCURSESW_NCURSES_H
    size_t wclen = 0;
    wchar_t *wcs = to_widechar(name, &wclen);

    if(wcs)
    {
        if(!files[pos])
        {
            if(!(files[pos] = allocFileStruct())) return 0;
        }
        else if(files[pos]->type != '%') free(files[pos]->name);

        files[pos]->name = (char *)wcs;
        files[pos]->namelen = wclen;
        files[pos]->iswcs = 1;
        return 1;
    }
#endif

    int len = strlen(name);

    if(!files[pos])
    {
        if(!(files[pos] = allocFileStructB(len+1))) return 0;
    }
    else
    {
        if(files[pos]->type != '%') free(files[pos]->name);
        if(!(files[pos]->name = malloc(len+1))) return 0;
    }

    strcpy(files[pos]->name, name);
    files[pos]->namelen = len;
    files[pos]->iswcs = 0;
    return 1;
}

void scanDir(char *dir) 
{
    int dcount = 0, fcount = 0;
    int n, j;
    struct dirent **eps;
    struct stat statbuf;

    n = scandir(dir, &eps, one, alphasort);

    if(strcmp(dir, ".") && strcmp(dir, cwd))
    {
        if(chdir(dir) == -1)
        {
            showErrorMsgBox("Error changing directory:", dir);
            refreshAll();
            return;
        }

        update_cwd_pointers();
    }
    
    if(n >= 0) 
    {
        int cnt;

        for(cnt = 0; cnt < n; ++cnt) 
        {
            char *d = eps[cnt]->d_name;

            if(lstat(d, &statbuf) == -1)
            {
                char tmp[strlen(d)+2];
                strcpy(tmp, ": ");
                strcat(tmp, d);
                showErrorMsgBox(strerror(errno), tmp);
                refreshAll();
                return;
            }
      
            if(S_ISDIR(statbuf.st_mode)) 
            {
                if(dcount >= MAXENTRIES) break;

                if(!addDirEntry(dcount, d))
                {
                    msgBoxH("Insufficient memory.", BUTTON_OK, ERROR);
                    refreshAll();
                    return;
                }

                if(strcmp(d, "..") == 0 || strcmp(d, ".") == 0) j = 0;
                else j = checkCutOrCopyDir(dcount);
                
                switch(j)
                {
                    case 1 : dirs[dcount]->star = '^'; break;
                    case 2 : dirs[dcount]->star = '#'; break;
                    default: dirs[dcount]->star = ' '; break;
                }
                
                //check if it is a hidden dir or not
                if(dirs[dcount]->name[0] == '.') dirs[dcount]->type = 'h';
                else dirs[dcount]->type = 'd';
                dcount++;
            }
            else 
            {
                if(fcount >= MAXENTRIES) break;

                if(!addFileEntry(fcount, d))
                {
                    msgBoxH("Insufficient memory.", BUTTON_OK, ERROR);
                    refreshAll();
                    return;
                }

                j = checkCutOrCopyFile(fcount);

                switch(j)
                {
                    case 1 : files[fcount]->star = '^'; break;
                    case 2 : files[fcount]->star = '#'; break;
                    default: files[fcount]->star = ' '; break;
                }

                //check to see the file type
                if(d[0] == '.')    //is it hidden?
                    files[fcount]->type = 'h';
                //is it a link?
                else if(S_ISLNK(statbuf.st_mode))
                    files[fcount]->type = 'l';
                //is executable?
                else if(statbuf.st_mode & (S_IXUSR|S_IXGRP|S_IXOTH))
                    files[fcount]->type = 'x';
                else
                {
                    char *ext = strrchr(d, '.');

                    if(!ext) files[fcount]->type = 'r';
                    else
                    {
                        //is it an archive?
                        if(strcasecmp(ext, ".tar") == 0  || 
                           strcasecmp(ext, ".gz") == 0   ||
                           strcasecmp(ext, ".xz"  ) == 0 || 
                           strcasecmp(ext, ".Z"  ) == 0  ||
                           strcasecmp(ext, ".rar" ) == 0 || 
                           strcasecmp(ext, ".zip") == 0  ||
                           strcasecmp(ext, ".bz2" ) == 0 || 
                           strcasecmp(ext, ".7z" ) == 0  ||
                           strcasecmp(ext, ".lzma") == 0 || 
                           strcasecmp(ext, ".lha") == 0  ||
                           strcasecmp(ext, ".jar" ) == 0)
                                files[fcount]->type = 'a';
                        //is it a picture?
                        else if(strcasecmp(ext, ".bmp") == 0  || 
                                strcasecmp(ext, ".png") == 0  ||
                                strcasecmp(ext, ".jpg") == 0  || 
                                strcasecmp(ext, ".jpeg") == 0 ||
                                strcasecmp(ext, ".pcx") == 0  || 
                                strcasecmp(ext, ".ico" ) == 0 ||
                                strcasecmp(ext, ".gif") == 0  || 
                                strcasecmp(ext, ".tiff") == 0)
                            files[fcount]->type = 'p';
                        else    //just a regular file
                            files[fcount]->type = 'r';
                    }
                }

                fcount++;
            }

            free(eps[cnt]);
        }

        free(eps);
    }
    else 
    {
        showErrorMsgBox("Failed to open directory:", strerror(errno));
        refreshAll();
        return;
    }

    freeFileStructs(fcount, totalFiles);
    totalFiles = fcount;

    //so the refreshFileView() will work properly
    if(totalFiles < numVisFiles)
        files[totalFiles] = NULL;

    firstVisFile = 0;
    selectedFile = 0;
    
    freeDirStructs(dcount, totalDirs);
    totalDirs = dcount;

    //so the refreshDirView() will work properly
    if(totalDirs < numVisDirs) 
        dirs[totalDirs  ] = NULL;

    firstVisDir = 0;
    selectedDir = 0;
    numStarred = 0;
  
    refreshAll();
}

void refreshAll(void)
{
    setScreenColorsI(COLOR_WINDOW);
    drawBox(1, 1, SCREEN_H, SCREEN_W, " Prime File Manager ", 0);
    drawMenuBar(2, 2, SCREEN_W-2);
    refreshFileView();
    refreshDirView();
    refreshBottomView();
}

void refreshWindows(void)
{
    setScreenColorsI(COLOR_WINDOW);
    refreshFileView();
    refreshDirView();
}

/***************************************
 * refreshDirView(): 
 * Procedure to refresh the left window
 * showing directory tree.
 * **************************************/
void refreshDirView(void) 
{
    int i, j, k;

    //if(activeWindow == DIR_WIN) setScreenColors(GREEN, BG_COLOR[COLOR_WINDOW]);

    //draw left sub-window
    drawBox(3, 2, SCREEN_H-5, (int)(SCREEN_W/2)-1, " Directory view ", 1);

    if(totalDirs == 0) 
    {
        mvprintw(3, 2, "%s", dirHighLight);
        mvprintw(3, 2, "No directory entries found!");
        refresh();
        return;
    }

    for(i = 0; i < numVisDirs; i++) 
    {
        k = firstVisDir+i;
        if(dirs[k] == NULL) break; 
        if(i == selectedDir && activeWindow == DIR_WIN)
            setScreenColorsI(COLOR_HIGHLIGHT_TEXT);
        else
            setScreenColors(FILE_DIR_COLOR[DIRCOLOR(k)], BG_COLOR[COLOR_WINDOW]);
        print_dir_highlight(i);
    }

    //redraw main window but don't clear the area
    setScreenColorsI(COLOR_WINDOW);
    drawBox(1, 1, SCREEN_H, SCREEN_W, " Prime File Manager ", 0);
    refresh();
}


/***************************************
 * refreshFileView(): 
 * Procedure to refresh the right window
 * showing file entries.
 * **************************************/
void refreshFileView() 
{
    int i, j, k;

    //if(activeWindow == FILE_WIN) setScreenColors(GREEN, BG_COLOR[COLOR_WINDOW]);

    //draw right sub-window
    drawBox(3, (int)(SCREEN_W/2), SCREEN_H-5, SCREEN_W-1, " File view ", 1);

    if(totalFiles == 0) 
    {
        addFileEntry(0, "(Empty folder)");
        files[0]->type = '%';
        files[1] = NULL;
        totalFiles = 1;
    }

    for(i = 0; i < numVisFiles; i++) 
    {
        k = firstVisFile+i;
        if(files[k] == NULL) break; 
        if(i == selectedFile && activeWindow == FILE_WIN)
            setScreenColorsI(COLOR_HIGHLIGHT_TEXT);
        else
            setScreenColors(FILE_DIR_COLOR[FILECOLOR(k)], BG_COLOR[COLOR_WINDOW]);
        print_file_highlight(i);
    }
 
    //redraw main window but don't clear the area
    setScreenColorsI(COLOR_WINDOW);
    drawBox(1, 1, SCREEN_H, SCREEN_W, " Prime File Manager ", 0);
    refresh();
}

/***************************************
 * refreshBottomView(): 
 * Procedure to refresh the bottom window
 * showing CWD and statistics.
 * *************************************/
void refreshBottomView() 
{
    //draw bottom sub-window
    setScreenColorsI(COLOR_WINDOW);
    drawBox(SCREEN_H-4, 2, SCREEN_H-1, SCREEN_W-1, " Quick reference ", 1);
    move(SCREEN_H-3, 3);

#ifdef HAVE_NCURSESW_NCURSES_H
    if(cwd_widechar != NULL)
    {
        printw("CWD: ");

        if(cwdlen_widechar > SCREEN_W-7) 
        {    //check if cwd length is more than display width
            //show just enought chars of the string
            addnwstr(cwd_widechar, SCREEN_W-12);

            //and seal it with '..'
            addch('.');
            addch('.');
        }
        else
        {    //the length is less than display width
            addwstr(cwd_widechar);     //so spit it all out
        }
    }
    else
#endif

    if(cwd != NULL)
    {
        if(cwdlen > SCREEN_W-7) 
        {    //check if cwd length is more than display width
            int i;

            printw("CWD: ");

            for(i = 0; i < SCREEN_W-12; i++) 
                addch(cwd[i]);    //show just enought chars of the string

            addch('.');             //and seal it with '..'
            addch('.');
        }
        else
        {    //the length is less than display width
            printw("CWD: %s", cwd);     //so spit it all out
        }
    }

    move(SCREEN_H-4, 3);
    if(numStarred > 0) printw("Marked (%d) ", numStarred);
    if(numCut     > 0) printw("Cut (%d) ", numCut);
    if(numCopy    > 0) printw("Copy (%d) ", numCopy);
    refresh();

    //redraw main window but don't clear the area
    drawBox(1, 1, SCREEN_H, SCREEN_W, " Prime File Manager ", 0);
    refresh();
}

/***************************************
 * drawMenuBar(): 
 * Procedure to draw the main menu bar.
 * **************************************/
void drawMenuBar(int x, int y, int w) 
{
    int i, j;

    reset_attribs();
    setScreenColorsI(COLOR_MENU_BAR);
    move(x-1, y-1);
    for(i = 0; i < w; i++) addch(' ');   //Draw empty menu bar
    move(x-1, y-1);

    for(i = 0; i < totalMainMenus; i++) 
    {
        j = 0;
        addch(' ');

        while(menu[i][j] != '\0') 
        {
            if(menu[i][j] == '&') 
            {   //turn on underline feature to print the shortcut key
                addch(menu[i][++j]);
            }
            else
            {
                //print normal chars (other than the shortcut key)
                addch(menu[i][j]);
            }

            j++;
        }

        addch(' ');
    }

    setScreenColorsI(COLOR_WINDOW);
    refresh();
}

