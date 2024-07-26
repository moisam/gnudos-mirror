/* 
 *    Programmed By: Mohammed Isam Mohammed [mohammed_isam1984@yahoo.com]
 *    Copyright 2014, 2015, 2016, 2017, 2018, 2024 (c)
 * 
 *    file: opensave.c
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

#include "config.h"
#define _GNU_SOURCE
#define _XOPEN_SOURCE_EXTENDED
#ifdef HAVE_NCURSESW_NCURSES_H
# include <ncursesw/ncurses.h>
#else
# include <ncurses.h>
#endif
#include "defs.h"
#include "kbd.h"
#include "options.h"
#include "edit.h"
#include "modules/modules.h"
#include "dialogs.h"
#include <pwd.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>


int numVisDirs, firstVisDir, selectedDir, totalDirs;
int numVisFiles, firstVisFile, selectedFile, totalFiles;
char **dirs;
char **files;

FILE *open_file;
wchar_t *documentTitle;    //document title is the file name

int MAX_DIR_NAME_LEN;

static int x, y, w, h;

static void refreshDirView(char *cwd);

// These are called by openSaveFile() to actually do the work of opening
// or saving the file. They return 1 if successful, 0 if error occurs.
static int _openFile(char *open_file_name);
static int _saveFile(char *open_file_name);


static inline int addwstr_wrapper(const wchar_t *wstr)
{
#ifdef HAVE_NCURSESW_NCURSES_H
    // we have ncursesw, so pass the call on
    return addwstr(wstr);
#else
    // if we don't have ncursesw, convert each wchar to a multibyte char
    // and print it out
    size_t i, wstrlen = wcslen(wstr);
    size_t clen;
    char cstr[16];
    wchar_t *p, wctmp[2] = { L'\0', L'\0' };

    for(i = 0; i < wstrlen; i++)
    {
        wctmp[0] = wstr[i];
        p = wctmp;
        clen = wcsrtombs(cstr, (const wchar_t **)&p, 1, NULL);
        cstr[clen] = '\0';
        printw("%s", cstr);
    }

    return OK;
#endif
}

static inline void WRAP_DIR_OUTPUT(int pos)
{
    if(pos >= totalDirs)
    {
        if(selectedDir >= numVisDirs)
        {
            mvprintw(selectedDir-numVisDirs+2, x+MAX_DIR_NAME_LEN,
                                                "%s", files[pos-totalDirs]);
        }
        else
        {
            mvprintw(selectedDir+2, x, "%s", files[pos-totalDirs]);
        }
    }
    else
    {
        attron(A_BOLD|A_UNDERLINE);
        if(selectedDir >= numVisDirs)
        {
            mvprintw(selectedDir-numVisDirs+2, x+MAX_DIR_NAME_LEN, 
                                                "%s", dirs[pos]);
        }
        else
        {
            mvprintw(selectedDir+2, x, "%s", dirs[pos]);
        }
        attroff(A_BOLD|A_UNDERLINE);
    }
}

static inline void OUTPUT_NAME(char *name)
{
    if(selectedDir >= numVisDirs)
    {
        mvprintw(selectedDir-numVisDirs+2, x+MAX_DIR_NAME_LEN, "%s", name);
    }
    else
    {
        mvprintw(selectedDir+2, x, "%s", name);
    }
}

void initDirView(void)
{
    numVisDirs  = SCREEN_H-9;
    firstVisDir = 0;
    selectedDir = -1;
    totalDirs   = 0;
    MAX_DIR_NAME_LEN = (SCREEN_W/2)-8;
}

static char *pathAppend(char *head, wchar_t *tail, int tail_iswcs)
{
    size_t len, taillen;
    char *str;

    if(!tail) return NULL;

    if(!head)
    {
        if(tail[0] != '/') return NULL;
        goto tail_only;
    }
    else if(tail[0] == '/')
    {
        goto tail_only;
    }

    if(tail_iswcs)
    {
        char *tmp = to_cstring((char *)tail, &taillen);
        if(!tmp) return NULL;
        len = strlen(head)+taillen;
        str = malloc(len+2);
        if(!str) { free(tmp); return NULL; }
        strcpy(str, head);
        strcat(str, "/");
        strcat(str, tmp);
        free(tmp);
    }
    else
    {
        len = strlen(head)+strlen((char *)tail);
        str = malloc(len+2);
        if(!str) return NULL;
        strcpy(str, head);
        strcat(str, "/");
        strcat(str, (char *)tail);
    }

    return str;
    
tail_only:

    if(tail_iswcs)
    {
        return to_cstring((char *)tail, NULL);
    }
    else
    {
        len = strlen((char *)tail);
        str = malloc(len+1);
        if(!str) return NULL;
        strcpy(str, (char *)tail);
        return str;
    }
}

static wchar_t *createDocTitle(char *open_file_name)
{
    char *slash = strrchr(open_file_name, '/');
    wchar_t *name;

    if(!slash) name = to_widechar(open_file_name, NULL);
    else name = to_widechar(slash+1, NULL);

    if(!name)
    {
        msgBox("Insufficient memory", BUTTON_OK, ERROR);
        return documentTitle;
    }

    if(documentTitle && wcscmp(documentTitle, name) == 0)
    {
        free(name);
        return documentTitle;
    }

    if(documentTitle) free(documentTitle);
    documentTitle = name;

    return documentTitle;
}


static inline void refreshNameField(int y, int sel, int selChar,
                                    wchar_t *inputName, int inputlen)
{
    setScreenColorsI(COLOR_WINDOW);
    attron(A_BOLD);
    mvprintw(h-3, y, "File: ");
    attroff(A_BOLD);
    setScreenColorsI(COLOR_HIGHLIGHT_TEXT);
    addwstr_wrapper(inputName);
    printw("%*s", (int)(w-y-inputlen-9), " ");
    if(sel == 1) move(h-3, y+selChar+6);
    refresh();
}

static inline int printFileOrDir(int pos, wchar_t *inputName, int inputlen)
{
    size_t newlen;
    wchar_t *tmp;

    if(pos >= totalDirs) 
    {
        OUTPUT_NAME(files[pos-totalDirs]);

        if(!(tmp = to_widechar(files[pos-totalDirs], &newlen)))
        {
            msgBox("Insufficient memory", BUTTON_OK, ERROR);
            return -1;
        }

        wcscpy(inputName, tmp);
        free(tmp);
        return (int)newlen;
    } 
    else 
    {
        attron(A_BOLD|A_UNDERLINE);
        OUTPUT_NAME(dirs[pos]);
        attroff(A_BOLD|A_UNDERLINE);
        return inputlen;
    }
}

#define CHDIR_AND_RETURN(errmsg)                                \
{                                                               \
    if(orig_cwd) { if(!chdir(orig_cwd)) {;} free(orig_cwd); }   \
    if(errmsg) msgBox(errmsg, BUTTON_OK, ERROR);                \
    return NULL;                                                \
}

/*****************************************
 * openSaveFile():
 * This function shows a dialog box that
 * contains the directory listing of the
 * current dir. It takes over user input
 * to enable navigation through the dir
 * tree and selecting a file to open/save.
 * It then calls _openFile() or _saveFile()
 * to actually open/save the file.
 * ***************************************/
char *openSaveFile(OPEN_SAVE openSave, int showDialog, char *open_file_name)
{
    x = 2; y = 2;
    h = SCREEN_H-1; w = SCREEN_W-1;

    //the input file name entered in the field
    wchar_t inputName[MAX_DIR_NAME_LEN];
    int inputlen;
    int selChar;    //the selected char in input field
    int sel;    //0=dir tree, 1=input field
    char *slash = NULL;
    char *name;
    char *cwd, *orig_cwd = getcwd(NULL, 0);
    int i, iswcs;
    struct  passwd *pass;
  
    if(showDialog) 
    {
        if(dirs ) { free(dirs ); dirs  = NULL; }
        if(files) { free(files); files = NULL; }

        totalDirs = 0; totalFiles = 0;
        iswcs = 1;

        //try to open the document directory, if failed open home 
        // directory, if also failed, just open the current working 
        // directory.
        if(open_file_name) slash = strrchr(open_file_name, '/');

        if(open_file_name && slash)
        {
            char *tmp;
            tmp = malloc(slash-open_file_name+1);

            if(!tmp)
            {
                msgBox("Insufficient memory", BUTTON_OK, ERROR);
                return NULL;
            }

            memcpy(tmp, open_file_name, slash-open_file_name);
            tmp[slash-open_file_name] = '\0';
            int res = scanDir(tmp, &dirs, &files, &totalDirs, &totalFiles);
            free(tmp);
            if(!res) CHDIR_AND_RETURN(NULL);
        }
        else 
        {
            if(!scanDir(".", &dirs, &files, &totalDirs, &totalFiles))
            {
                if((pass = getpwuid(geteuid())))
                {
                    if(!scanDir(pass->pw_dir, &dirs, &files, 
                                        &totalDirs, &totalFiles))
                        CHDIR_AND_RETURN(NULL);
                } else CHDIR_AND_RETURN("Failed to get home directory.");
            }
        }

        sel = (openSave == SAVE) ? 1 : 0;

        //if(cwd) free(cwd);
        cwd = getcwd(NULL, 0);

        if(openSave == OPEN) inputName[0] = L'\0';
        else 
        {
            //if there is a document title, save the title
            if(documentTitle) wcscpy(inputName, documentTitle);
            else              wcscpy(inputName, DEFAULT_TITLE);
        }

        selChar = wcslen(inputName);
        inputlen = selChar;
  
        numVisDirs = h-x-3;
        firstVisDir = 0;
        selectedDir = 0;
        refreshDirView(cwd);

        //set the input field
        refreshNameField(y, 1, inputlen, inputName, inputlen);

        //reset cursor if it is an OPEN file dialog
        if(openSave == OPEN) 
        {
            setScreenColorsI(COLOR_HIGHLIGHT_TEXT);
            WRAP_DIR_OUTPUT(firstVisDir+selectedDir);
        }

        refresh();

        /***************************************************/
        //take control over user input
        /***************************************************/
        while(1) 
        {
            char *ch = ugetKey();

            switch(ch[0]) 
            {
                case('a'):
                    if(GNU_DOS_LEVEL > 2 && CTRL) goto do_home;
                    goto do_enter_char; break;

                case(HOME_KEY):
                    if(GNU_DOS_LEVEL > 2) break;
do_home:
                    if(sel != 1) break;
                    selChar = 0;
                    move(h-3, y+selChar+6);
                    refresh();
                    break;

                case('e'):
                    if(GNU_DOS_LEVEL > 2 && CTRL) goto do_end;
                    goto do_enter_char; break;

                case(END_KEY):
                    if(GNU_DOS_LEVEL > 2) break;
do_end:
                    if(sel != 1) break;
                    selChar = inputlen;
                    if(sel > w-y-9) sel--;
                    move(h-3, y+selChar+6);
                    refresh();
                    break;

                case('d'):
                    if(GNU_DOS_LEVEL > 3 && CTRL) goto do_del;
                    goto do_enter_char; break;

                case(DEL_KEY):
                    if(GNU_DOS_LEVEL > 3) break;
do_del:
                    if(sel != 1) break;
                    if(selChar == inputlen) break;
                    for(i = selChar; i < inputlen-1; i++)
                        inputName[i] = inputName[i+1];
                    inputName[i] = L'\0';
                    inputlen--;
                    setScreenColorsI(COLOR_HIGHLIGHT_TEXT);
                    mvprintw(h-3, y+6, "%*s", w-y-9, " ");
                    move(h-3, y+6);
                    addwstr_wrapper(inputName);
                    move(h-3, y+selChar+6);
                    refresh();
                    break;

                /***************************************************/
                /***************************************************/
                /***************************************************/
                case(BACKSPACE_KEY):
                    if(sel != 1) break;
                    if(selChar == 0) break;
                    if(selChar == inputlen)
                    {
                        i = inputlen-1;
                    } 
                    else 
                    {
                        for(i = selChar; i <= inputlen; i++)
                            inputName[i-1] = inputName[i];
                    }

                    inputName[i] = L'\0';
                    selChar--;
                    inputlen--;
                    setScreenColorsI(COLOR_HIGHLIGHT_TEXT);
                    mvprintw(h-3, y+6, "%*s", w-y-9, " ");
                    move(h-3, y+6);
                    addwstr_wrapper(inputName);
                    move(h-3, y+selChar+6);
                    refresh();
                    break;

                /***************************************************/
                /***************************************************/
                /***************************************************/
                case(TAB_KEY):
                    sel = (sel == 0) ? 1 : 0;
                    int pos = firstVisDir+selectedDir;

                    if(sel == 1) 
                    {
                        setScreenColorsI(COLOR_WINDOW);
                        WRAP_DIR_OUTPUT(pos);
                        move(h-3, y+selChar+6);
                    } 
                    else 
                    {
                        setScreenColorsI(COLOR_HIGHLIGHT_TEXT);
                        WRAP_DIR_OUTPUT(pos);
                    }
                    break;

                /***************************************************/
                /***************************************************/
                /***************************************************/
                case('b'):
                    if(GNU_DOS_LEVEL > 1 && CTRL) goto do_left;
                    goto do_enter_char; break;

                case(LEFT_KEY):
                    if(GNU_DOS_LEVEL > 1) break;
do_left:
                    if(sel == 1) 
                    {
                        if(selChar == 0) break;
                        selChar--;
                        move(h-3, y+selChar+6);
                        break;
                    }
        
                    if(selectedDir >= numVisDirs) 
                    {
                        int pos = firstVisDir+selectedDir;

                        setScreenColorsI(COLOR_WINDOW);
                        WRAP_DIR_OUTPUT(pos);
                        selectedDir -= numVisDirs;
                        pos -= numVisDirs;
                        setScreenColorsI(COLOR_HIGHLIGHT_TEXT);

                        if((inputlen = printFileOrDir(pos, inputName, 
                                                           inputlen)) == -1)
                            CHDIR_AND_RETURN(NULL);
                    }
                    else 
                    {
                        if(firstVisDir == 0) break;
                        selectedDir += numVisDirs;
                        firstVisDir -= numVisDirs;
                        refreshDirView(cwd);
                    } 

                    refreshNameField(y, sel, selChar, inputName, inputlen);
                    break;

                /***************************************************/
                /***************************************************/
                /***************************************************/
                case('f'):
                    if(GNU_DOS_LEVEL > 1 && CTRL) goto do_right;
                    goto do_enter_char; break;

                case(RIGHT_KEY):
                    if(GNU_DOS_LEVEL > 1) break;
do_right:
                    if(sel == 1) 
                    {
                        if(selChar == inputlen) break;
                        if(selChar >= w-y-9) break;
                        selChar++;
                        move(h-3, y+selChar+6);
                        break;
                    }
        
                    if(selectedDir < numVisDirs) 
                    {
                        int pos = firstVisDir+selectedDir;

                        setScreenColorsI(COLOR_WINDOW);
                        WRAP_DIR_OUTPUT(pos);
                        selectedDir += numVisDirs;
                        pos += numVisDirs;
        
                        if(pos >= (totalDirs+totalFiles))
                        {
                            int h = (totalDirs+totalFiles)-pos-1;
                            pos += h;  // h is negative, so add it in
                                       // order to subtract it!!
                            selectedDir += h;
                        }

                        setScreenColorsI(COLOR_HIGHLIGHT_TEXT);
                        if((inputlen = printFileOrDir(pos, inputName, 
                                                           inputlen)) == -1)
                            CHDIR_AND_RETURN(NULL);
                    }
                    else 
                    {
                        if((firstVisDir+(numVisDirs*2)) >= 
                                    (totalDirs+totalFiles)) break;
                        selectedDir -= numVisDirs;
                        firstVisDir += numVisDirs;
                        refreshDirView(cwd);
                    }

                    refreshNameField(y, sel, selChar, inputName, inputlen);
                    break;

                /***************************************************/
                /***************************************************/
                /***************************************************/
                case('g'):
                    if(GNU_DOS_LEVEL > 2 && CTRL)
                    {   // ESC - GNU key binding
                        refreshView();
                        CHDIR_AND_RETURN(NULL);
                    }
                    goto do_enter_char; break;

                case(ESC_KEY):
                    if(GNU_DOS_LEVEL > 2) break;
                    refreshView();
                    CHDIR_AND_RETURN(NULL);

                /***************************************************/
                /***************************************************/
                /***************************************************/
                case(ENTER_KEY):
                    if(sel == 1) 
                    {
                        if(openSave != SAVE)
                        {
                            name = pathAppend(cwd, inputName, 1);
                            goto openFileAndGo;
                        }

                        struct stat st;
                        char *tmp = to_cstring((char *)inputName, NULL);

                        if(!tmp)
                        {
                            CHDIR_AND_RETURN("Insufficient memory.");
                        }

                        if(lstat(tmp, &st) != -1 && S_ISDIR(st.st_mode))
                        {
                            if(msgBox("File already exists. Overwrite?",
                                            BUTTON_YES|BUTTON_NO, CONFIRM) == 
                                                    BUTTON_NO)
                            {
                                refreshDirView(cwd); 
                                refreshNameField(y, sel, selChar, 
                                                 inputName, inputlen);
                                free(tmp);
                                break;
                            }
                        }

                        name = pathAppend(cwd, inputName, 1);
                        free(tmp);
                        goto saveFileAnGo;
                    }
                    
                    int index = firstVisDir+selectedDir;
                    if(index < totalDirs) 
                    {
                        //selected a directory.. navigate to it
                        scanDir(dirs[index], &dirs, &files, 
                                        &totalDirs, &totalFiles);
                        firstVisDir = 0;
                        selectedDir = 0;
                        if(cwd) free(cwd);
                        cwd = getcwd(NULL, 0);
                        refreshDirView(cwd);
                        refreshNameField(y, sel, selChar, inputName, inputlen);
                    }
                    else 
                    {
                        //selected a file.. open/save it
                        if(openSave == OPEN)
                        {
                            name = pathAppend(cwd, inputName, 1);
                            goto openFileAndGo;
                        }
                        else 
                        {
                            if(msgBox("File already exists. Overwrite?",
                                            BUTTON_YES|BUTTON_NO, CONFIRM) == 
                                                    BUTTON_NO) 
                            {
                                refreshDirView(cwd); 
                                refreshNameField(y, sel, selChar, 
                                                 inputName, inputlen);
                                break;
                            }
                        }

                        name = pathAppend(cwd, inputName, 1);
                        goto saveFileAnGo;
                    }

                    break;

                /***************************************************/
                /***************************************************/
                /***************************************************/
                //Navigate up through dirs and files
                case('p'):
                    if(GNU_DOS_LEVEL > 1 && CTRL) goto do_up;
                    goto do_enter_char; break;

                case(UP_KEY):
                    if(GNU_DOS_LEVEL > 1) break;
do_up:
                    if(sel == 1) break;
                    if(selectedDir == 0) 
                    {
                        if(firstVisDir == 0) break;
                        firstVisDir -= numVisDirs;
                        selectedDir = numVisDirs-1;
                        refreshDirView(cwd);
                    } 
                    else 
                    {
                        int pos = firstVisDir+selectedDir;
                        setScreenColorsI(COLOR_WINDOW);
                        WRAP_DIR_OUTPUT(pos);
                        selectedDir--; pos--;
                        setScreenColorsI(COLOR_HIGHLIGHT_TEXT);
                        WRAP_DIR_OUTPUT(pos);
                        if((inputlen = printFileOrDir(pos, inputName, 
                                                           inputlen)) == -1)
                            CHDIR_AND_RETURN(NULL);
                    }

                    refreshNameField(y, sel, selChar, inputName, inputlen);
                    break;

                /***************************************************/
                /***************************************************/
                /***************************************************/
                //Navigate down through dirs and files
                case('n'):
                    if(GNU_DOS_LEVEL > 1 && CTRL) goto do_down;
                    goto do_enter_char; break;

                case(DOWN_KEY):
                    if(GNU_DOS_LEVEL > 1) break;
do_down:
                    if(sel == 1) break;
                    if(selectedDir == (numVisDirs*2)-1) 
                    {
                        if((firstVisDir+(numVisDirs*2)) < 
                                        (totalDirs+totalFiles))
                        {
                            firstVisDir += numVisDirs;
                            selectedDir = numVisDirs;
                            refreshDirView(cwd);

                            //if SAVE dialog, redraw the input field
                            if(openSave != SAVE) 
                            {
                                refresh();
                                break;
                            }

                            refreshNameField(y, sel, selChar, 
                                             inputName, inputlen);
                            break;
                        }
                    }
                    else 
                    {
                        int pos = firstVisDir+selectedDir;
                        if(pos >= (totalDirs+totalFiles-1)) break;
                        setScreenColorsI(COLOR_WINDOW);
                        WRAP_DIR_OUTPUT(pos);
                        selectedDir++; pos++;
                        setScreenColorsI(COLOR_HIGHLIGHT_TEXT);
                        if((inputlen = printFileOrDir(pos, inputName, 
                                                           inputlen)) == -1)
                            CHDIR_AND_RETURN(NULL);
                    }

                    refreshNameField(y, sel, selChar, inputName, inputlen);
                    break;

                default:
do_enter_char:
                    if(sel != 1) break;
                    if(inputlen >= w-y-9) break;

                    // check for a UTF-8 sequence
                    int utf8_bytes;
                    for(utf8_bytes = 0; ch[utf8_bytes]; utf8_bytes++) ;

                    // UTF-8 or alphanumeric ASCII char
                    if((utf8_bytes > 1) ||
                       (ch[0] >= 'a' && ch[0] <= 'z') || 
                       (ch[0] >= 'A' && ch[0] <= 'Z') ||
                       (ch[0] >= 32 && ch[0] <= 64) ||
                       (ch[0] >=123 && ch[0] <= 126))
                    {
                        wchar_t wch;
                        inputName[inputlen+1] = '\0';
                        for(i = inputlen; i > selChar; i--)
                            inputName[i] = inputName[i-1];
                        inputlen++;
                        mbrtowc(&wch, ch, utf8_bytes, NULL);
                        inputName[selChar++] = wch;
                        refreshNameField(y, sel, selChar, inputName, inputlen);
                    }
                    break;
            } //end switch
        } //end while
    }
    else
    {
        //strcpy((char *)inputName, open_file_name);
        //iswcs = 0;
        name = open_file_name;
        if(openSave == OPEN) goto openFileAndGo;
        else goto saveFileAnGo;
    } //end outer if

saveFileAnGo:

    if(orig_cwd) { if(!chdir(orig_cwd)) {;} free(orig_cwd); }

    //name = pathAppend(cwd, inputName, iswcs);
    if(!name)
    {
        msgBox("Insufficient memory.", BUTTON_OK, ERROR);
        return NULL;
    }

    if(!_saveFile(name))
    {
        free(name);
        return NULL;
    }

    FILE_STATE = SAVED;
    NEW_FILE = 0;
    return name;

openFileAndGo:

    if(orig_cwd) { if(!chdir(orig_cwd)) {;} free(orig_cwd); }

    //name = pathAppend(cwd, inputName, iswcs);
    if(!name)
    {
        msgBox("Insufficient memory.", BUTTON_OK, ERROR);
        return NULL;
    }

    if(!_openFile(name))
    {
        free(name);
        return NULL;
    }

    FILE_STATE = OPENED;
    NEW_FILE = 0;
    return name;
}


/***************************************
 * refreshDirView(): 
 * Procedure to refresh the left window
 * showing directory tree.
 * **************************************/
static void refreshDirView(char *cwd)
{
    if(strlen(cwd) > w-y-5) 
    {
        char *tmp;
        tmp = malloc(w-y+1);
        if(!tmp) { msgBox("Insufficient memory", BUTTON_OK, ERROR); return; }
   
        int i, k, j = w-y-5;
        for(i = strlen(cwd), k = j; k > 1; i--, k--) tmp[k] = cwd[i];
        tmp[0] = '.';
        tmp[1] = '.';
        drawBox(x, y, h, w, tmp, 1);
        free(tmp);
    } else drawBox(x, y, h, w, cwd, 1);
 
    //show control message at the bottom
    move(h-2, y);
    setScreenColorsI(COLOR_WINDOW);
    attron(A_BOLD);

    if(GNU_DOS_LEVEL > 2)
        printw("[ENTER]: Open dir/file [C-p C-n C-f C-b]: "
               "Navigate [C-g]: Cancel");
    else if(GNU_DOS_LEVEL > 1)
        printw("[ENTER]: Open dir/file [C-p C-n C-f C-b]: "
               "Navigate [ESC]: Cancel");
    else
        printw("[ENTER]: Open dir/file  [ARROWS]: "
               "Navigate  [ESC]: Cancel");

    mvprintw(h-3, y, "File: ");
    attroff(A_BOLD);
 
    int i = firstVisDir;
    int j = 0; int k = 0;
    int curX, curY;
    int savedSelectedDir = selectedDir;

    attron(A_BOLD|A_UNDERLINE);

    while(i < totalDirs) 
    {
        if((i-firstVisDir) == selectedDir)
        {
            setScreenColorsI(COLOR_HIGHLIGHT_TEXT);
            selectedDir = j;
            OUTPUT_NAME(dirs[i]);
            curX = (j >= numVisDirs) ? j-numVisDirs+3 : j+3;
            curY = (j >= numVisDirs) ? x+MAX_DIR_NAME_LEN+strlen(dirs[i]) : 
                                       x+strlen(dirs[i]);
            curY += 3;
        } 
        else 
        {
            setScreenColorsI(COLOR_WINDOW);
            selectedDir = j;
            OUTPUT_NAME(dirs[i]);
        }

        if(j >= (numVisDirs*2)-1) break;

        i++; j++; k++;
    }

    attroff(A_BOLD|A_UNDERLINE);

    //if there is more room, show the files
    if((i >= totalDirs) && (j < numVisDirs*2)) 
    {
        if(firstVisDir > totalDirs) j = firstVisDir-totalDirs;
        else j = 0;
    
        while(j < totalFiles) 
        {
            if((i-firstVisDir) == selectedDir) 
            {
                setScreenColorsI(COLOR_HIGHLIGHT_TEXT);
                selectedDir = k;
                OUTPUT_NAME(files[j]);
                curX = (j >= numVisDirs) ? j-numVisDirs+3 : j+3;
                curY = (j >= numVisDirs) ? 
                            x+MAX_DIR_NAME_LEN+strlen(files[j]) : 
                            x+strlen(files[j]);
                curY += 3;
            } 
            else 
            {
                setScreenColorsI(COLOR_WINDOW);
                selectedDir = k;
                OUTPUT_NAME(files[j]);
            }
            if(j >= totalFiles) break;
            if(k >= (numVisDirs*2)-1) break;
            i++; j++; k++;
        }//end while
    }//end if 

    selectedDir = savedSelectedDir;

    //reposition the cursor
    move(curX-1, curY-1);
    refresh();
}


/***************************************************************
 * This function saves the file specified in open_file_name.
 * It is called indirectly through the function openSaveFile().
 * *************************************************************/
int _saveFile(char *open_file_name) 
{
    int i, j;
    char *tmp;

    if(!(open_file = fopen(open_file_name, "w+"))) 
    {
        msgBox("Save: Failed to open file", BUTTON_OK, ERROR);
        return 0;
    }

    // we have to convert the lines back to UTF-8 before outputting them
    // to file
    for(i = 0; i < totalLines; i++) 
    {
        if(!(tmp = to_cstring((char *)lines[i]->text, NULL)))
        {
            msgBox("Save: Insufficient memory", BUTTON_OK, ERROR);
            return 0;
        }

        fprintf(open_file, "%s", tmp);
        free(tmp);
    }

    FILE_STATE = SAVED;

    if(!createDocTitle(open_file_name))
    {
        msgBox("Save: Failed to create document title", BUTTON_OK, ERROR);
        return 0;
    }

    fflush(open_file); 

    checkFileExtension(open_file_name);
    return 1;
}

int copyBufToLine(int i, int k, char *buffer, int linked)
{
    size_t linelen;
    buffer[k] = '\0';
    lines[i] = allocLineStruct();
    if(!lines[i]) return 0;
    lines[i]->linkedToNext = linked;
    lines[i]->text = to_widechar(buffer, &linelen);
    if(!lines[i]->text) { free(lines[i]); return 0; }
    lines[i]->text[linelen] = L'\0';
    lines[i]->charsAlloced = linelen+1;
    calcTotalCharsInLine(i);
    return 1;
}

char buf[4096];

/***************************************************************
 * This function opens the file specified in open_file_name.
 * It is called indirectly through the function openSaveFile().
 * *************************************************************/
int _openFile(char *open_file_name) 
{
    int i;
    int j = 0;
    int k = 0, l = 0;
    
    if(!(open_file = fopen(open_file_name, "r")))
    {
        msgBox("Open: Failed to open file", BUTTON_OK, ERROR);
        return 0;
    }
  
    for(i = 0; i < totalLines; i++)
    {
        if(lines[i])
        {
            if(lines[i]->text) free(lines[i]->text);
            free(lines[i]);
            lines[i] = NULL;
        }
    }

    totalLines = 0;
    i = 0;

    while(!feof(open_file))
    {
        if(totalLines >= MAX_LINES) break;

        j = fgetc(open_file);

        if(j == -1) break;
        if(l >= MAX_CHARS_PER_LINE)
        {
            if(!copyBufToLine(i, k, buf, 1))
            {
                fclose(open_file);
                msgBox("Open: Insufficient memory", BUTTON_OK, ERROR);
                return 0;
            }

            i++; k = 0; l = 0;
            totalLines++;
        }

        buf[k++] = j;

        if(j == '\n')
        {
            if(!copyBufToLine(i, k, buf, 0))
            {
                fclose(open_file);
                msgBox("Open: Insufficient memory", BUTTON_OK, ERROR);
                return 0;
            }

            i++; k = 0; l = 0;
            totalLines++;
        }
        else if(j == '\t')
        {
            l += TABSPACES(l+1)+1;
        }
        else
        {
            if((j & 0xc0) != 0x80) l++;
        }
    }
    // some remaining chars?
    if(k != 0)
    {
        if(!copyBufToLine(i, k, buf, 0))
        {
            fclose(open_file);
            msgBox("Open: Insufficient memory", BUTTON_OK, ERROR);
            return 0;
        }

        totalLines++;
    }

    if(!createDocTitle(open_file_name))
    {
        fclose(open_file);
        msgBox("Open: Insufficient memory", BUTTON_OK, ERROR);
        return 0;
    }

    resetLineCounters();
    checkFileExtension(open_file_name);
    fclose(open_file);
    open_file = NULL;
    return 1;
}

void checkFileExtension(char *open_file_name)
{
    AUTO_HIGHLIGHTING = 0;

    //search for the last dot in the filename
    char *ext = strrchr(open_file_name, '.');

    if(ext != NULL)    //was there a dot?
    {
        int i, j;

        //check what was the file extension
        ext++;

        for(i = 0; i < NUM_MODULES; i++)
        {
            for(j = 0; j < module[i]->extsCount; j++)
            {
                if(strcmp(ext, module[i]->exts[j]) == 0)
                {
                    AUTO_HIGHLIGHTING = 1;
                    BG_COLOR[COLOR_WINDOW] = COLOR_HWINDOW;
                    HIGHLIGHT_MODE = j;
                    curmodule = module[i];
                    break;
                }
            }

            if(AUTO_HIGHLIGHTING) break;
        }
    }
    
    if(!AUTO_HIGHLIGHTING)
    {
        /*
         * No dot in the file name, or file extension not recognized.
         * Check file content.
         */
        wchar_t *line = lines[0]->text;

        //is it a shell script file?
        if((wcsstr(line, L"/bin/csh")) || (wcsstr(line, L"/bin/ksh" )) ||
           (wcsstr(line, L"/bin/sh" )) || (wcsstr(line, L"/bin/bash")))
        {
            AUTO_HIGHLIGHTING = 1;
            BG_COLOR[COLOR_WINDOW] = COLOR_HWINDOW;
            HIGHLIGHT_MODE = SHELL_MODE;
            curmodule = module[SHELL_MODE];
        } 
        //is it a PERL script file?
        else if(wcsstr(line, L"/bin/perl"))
        {
            AUTO_HIGHLIGHTING = 1;
            BG_COLOR[COLOR_WINDOW] = COLOR_HWINDOW;
            HIGHLIGHT_MODE = PERL_MODE;
            curmodule = module[PERL_MODE];
        }
        //NONE of the above...
        else 
        { 
            BG_COLOR[COLOR_WINDOW] = old_window_color;
            HIGHLIGHT_MODE = NO_MODE;
            curmodule = &dummyModule;
        }
    }
}

