/* 
 *    Programmed By: Mohammed Isam Mohammed [mohammed_isam1984@yahoo.com]
 *    Copyright 2014, 2015, 2016, 2017, 2018, 2024 (c)
 * 
 *    file: main.c
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
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <poll.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include "defs.h"
#include "edit.h"
#include "file.h"
#include "dialogs.h"
#include "kbd.h"
#include "options.h"
#include "modules/modules.h"

extern char *STARTUP_FILE_NAME;     /* init.c */
extern void checkFileExtension();

static void drawMenuBar(int x, int y, int w);
static void drawScrollBar(void);
static int isKeyword(int pos, int start, int *wordlen);
static int isWhitespace(wchar_t c);
static int appendToLine(int posTo, int posFrom, int count);
static void postDeleteWord(int pos);
static void deletePrevWord(void);
static void deleteNextWord(void);
static void deletePrevChar(void);
static void deleteNextChar(void);
static void insertEnter(void);
static void insertTab(void);
static void insertChar(wchar_t ch);

static wchar_t *specialCharsString = L"[]{}()<>:;,*+-=%_!#$^&`~\\";
static sig_atomic_t end = 0;

char *open_file_name;
int MAX_CHARS_PER_LINE;
int maxLen;
int firstVisLine;
int totalVisLines;
int totalLines;
int selectedLine;
int selectedChar;
int selectedCharCarry;
int userVisibleCurLine;     // the current line number the user sees
int selectedCharIfPossible; // where the cursor is supposed to be, unless it is 
                            // past EoL


static inline int isWhitespace(wchar_t c)
{
    return (c == L' ' || c == L'\t' || c == L'\v' || c == L'\n' || c == L'\f');
}

static inline int isSpaceOrTab(wchar_t c)
{
    return (c == L' ' || c == L'\t');
}

static inline void confirmEOL(int pos)
{
    if(selectedChar >= MAX_CHARS_PER_LINE) selectedChar = MAX_CHARS_PER_LINE-1;
    if(selectedChar >= lines[pos]->charCount)
    {
        selectedChar = lines[pos]->charCount;
        if(lines[pos]->text[selectedChar-1] == L'\n') selectedChar--;
    }
}

void calcUserVisibleCurLine(void)
{
    int i, pos = firstVisLine+selectedLine;

    userVisibleCurLine = 0;

    for(i = 0; i < pos; i++)
    {
        if(!lines[i]->linkedToNext) userVisibleCurLine++;
    }
}

static void gotoNextWord(int pos, int startAt)
{
    int i;
    wchar_t *s = lines[pos]->text+startAt;
    selectedChar = startAt;
    while(isWhitespace(*s)) s++, selectedChar++;
    if(*s == L'\n' || *s == L'\0') return;
    while(*s && !isWhitespace(*s))
    {
        selectedChar++;
        s++;
    }
    if(*s == L'\n' || *s == L'\0') return;
    while(isWhitespace(*s)) s++, selectedChar++;
}

static void gotoPrevWord(int pos, int startAt)
{
    int i;
    wchar_t *s = lines[pos]->text;
    selectedChar = 0;
    while(isWhitespace(s[startAt])) startAt--;
    if(startAt <= 0) return;
    while(!isWhitespace(s[startAt])) startAt--;
    if(isWhitespace(s[startAt])) startAt++;
    if(startAt <= 0) return;
    for(i = 0; i < startAt; i++)
    {
        selectedChar++;
    }
}

void sighandler(int signo)
{
    //fprintf(stdout, "SIGNAL %d received\n", signo);
    if(signo == 2) 
    {    //CTRL-C pressed
      editMenu_Copy();
    } 
    else 
    {
      end = 1;
    }
}

static void drawMainWindow(int clearArea)
{
    if(documentTitle)
    {
        if(FILE_STATE == MODIFIED)
        {
            size_t len = wcslen(documentTitle);
            wchar_t tmp[len+2];

            wcscpy(tmp, documentTitle);
            tmp[len] = L'*';
            tmp[len+1] = L'\0';
            drawBoxWC(2, 1, SCREEN_H-1, SCREEN_W, tmp, clearArea);
        }
        else drawBoxWC(2, 1, SCREEN_H-1, SCREEN_W, documentTitle, clearArea);
    }
    else drawBoxWC(2, 1, SCREEN_H-1, SCREEN_W, documentTitle, clearArea);
}

static inline void start_selecting(void)
{
    SELECTING = 1;

    if(sel_range_start.nline == -1)
    {
        sel_range_start.nline   = firstVisLine+selectedLine;
        sel_range_start.nchar   = selectedChar;
        sel_range_end.nline     = firstVisLine+selectedLine;
        sel_range_end.nchar     = selectedChar;
        refreshBottomView();
    }
}

static inline void cancel_selecting(void)
{
    if(sel_range_start.nline != -1)
    {
        SELECTING = 0;
        sel_range_start.nline   = -1;
        sel_range_start.nchar   = -1;
        sel_range_end.nline     = -1;
        sel_range_end.nchar     = -1;
        refreshBottomView();
    }
}

static inline void end_selecting(void)
{
    if(SELECTING)
    {
        sel_range_end.nline = firstVisLine+selectedLine;
        sel_range_end.nchar = selectedChar;
    }
}

static inline void do_pg_up(void)
{
    if(SHIFT) start_selecting();
    else cancel_selecting();

    if(firstVisLine == 0) selectedLine = 0;
    else if(firstVisLine-totalVisLines < 0) 
    {
        selectedLine  = firstVisLine;
        firstVisLine  = 0;
    }
    else firstVisLine -= totalVisLines; 

    int pos = firstVisLine+selectedLine;

    selectedChar = selectedCharIfPossible;
    if(selectedChar > lines[pos]->charCount) 
        selectedChar = lines[pos]->charCount;

    end_selecting();
    calcUserVisibleCurLine();
    refreshView();
}

static inline void do_pg_down(void)
{
    if(SHIFT) start_selecting();
    else cancel_selecting();

    if(totalLines < totalVisLines)
    {
        firstVisLine = 0;
        selectedLine = totalLines-1;
    }
    else
    {
        firstVisLine += totalVisLines;

        if(firstVisLine+totalVisLines >= totalLines) 
        {
            firstVisLine = totalLines-totalVisLines;
            selectedLine = totalVisLines-1;
        }
    }

    int pos = firstVisLine+selectedLine;

    selectedChar = selectedCharIfPossible;
    if(selectedChar > lines[pos]->charCount) 
        selectedChar = lines[pos]->charCount;

    end_selecting();
    calcUserVisibleCurLine();
    refreshView();
}

static inline void do_home(char *ch)
{
    int pos, refreshAll = 0;

    if(SHIFT) start_selecting();
    else cancel_selecting();

    selectedChar = 0;
    selectedCharCarry = 0;
    selectedCharIfPossible = 0;

    if((CTRL && GNU_DOS_LEVEL < 3) ||
       (CTRL && GNU_DOS_LEVEL >= 3 && ch[0] == '>')) 
    {
        selectedLine = 0;
        firstVisLine = 0;
        refreshAll   = 1;
    }
    else
    {
        pos = firstVisLine+selectedLine;

        while(pos && lines[pos-1]->linkedToNext) pos--;

        if(pos < firstVisLine)
        {
            firstVisLine = pos;
            selectedLine = 0;
            refreshAll   = 1;
        }
        else
        {
            selectedLine = pos-firstVisLine;
        }
    }

    end_selecting();
    calcUserVisibleCurLine();
    move(selectedLine+2, 1);

    if(refreshAll) refreshView();
    else           refreshSelectedLine();
}

static inline void do_end(char *ch)
{
    int pos, refreshAll = 0;

    if(SHIFT) start_selecting();
    else cancel_selecting();

    if((CTRL && GNU_DOS_LEVEL < 3) ||
       (CTRL && GNU_DOS_LEVEL >= 3 && ch[0] == '<')) 
    {
        if(totalLines <= totalVisLines) 
        {
            selectedLine = totalLines-1; firstVisLine = 0;
        } 
        else
        {
            firstVisLine = totalLines-totalVisLines;
            selectedLine = totalVisLines-1;
            refreshAll   = 1;
        }
    }
    else
    {
        pos = firstVisLine+selectedLine;

        while(lines[pos]->linkedToNext) pos++;

        if(pos > firstVisLine+totalVisLines)
        {
            selectedLine = totalVisLines-1;
            firstVisLine = pos-totalVisLines;
            refreshAll   = 1;
        }
        else
        {
            selectedLine = pos-firstVisLine;
        }
    }

    pos = firstVisLine+selectedLine;
    selectedChar = lines[pos]->charCount;
    confirmEOL(pos);
    selectedCharIfPossible = selectedChar;
    calcCharCarry(pos);
    calcUserVisibleCurLine();
    move(selectedLine+2, selectedChar+1+selectedCharCarry);
    end_selecting();

    if(refreshAll) refreshView();
    else           refreshSelectedLine();
}

static inline void do_right(void)
{
    int pos = firstVisLine+selectedLine;
    int refreshAll = 0;

    if(SHIFT) start_selecting();
    else
    {
        if(SELECTING) refreshAll = 1;
        cancel_selecting();
    }

    if(WRAP_LINES)
    {
        int oldChar = selectedChar;
        int len = lines[pos]->charCount;

        if(len && lines[pos]->text[len-1] == L'\n') len--;

        if(selectedChar >= len ||
           selectedChar+selectedCharCarry >= MAX_CHARS_PER_LINE-1) 
        {
            if((selectedLine+firstVisLine) >= (totalLines-1)) return;

            selectedCharCarry = 0;

            if(selectedLine == totalVisLines-1) 
            {
                firstVisLine++;
            }
            else 
            {
                selectedLine++;
                refreshAll = 1;
            }

            if((CTRL && GNU_DOS_LEVEL == 1) || 
               (ALT  && GNU_DOS_LEVEL >  1))
            {//if using CTRL, goto next word
                gotoNextWord(pos+1, 0);
            }
            else selectedChar = 0;

            pos = firstVisLine+selectedLine;

            confirmEOL(pos);
            calcCharCarry(pos);
            end_selecting();
            calcUserVisibleCurLine();
        } 
        else 
        {
            if((CTRL && GNU_DOS_LEVEL == 1) || (ALT  && GNU_DOS_LEVEL >  1))
            {//if using CTRL, goto next word
                gotoNextWord(pos, selectedChar+1);
            }
            else selectedChar++;

            confirmEOL(pos);
            calcCharCarry(pos);
            end_selecting();
        }//if #2
    }//if #1

    if(refreshAll) refreshView();
    else
    {
        refreshSelectedLine();
        refreshBottomView();
    }

    selectedCharIfPossible = selectedChar;
}

static inline void do_left(void)
{
    int pos = firstVisLine+selectedLine;
    int refreshAll = 0;

    if(SHIFT) start_selecting();
    else
    {
        if(SELECTING) refreshAll = 1;
        cancel_selecting();
    }

    if(selectedChar == 0) 
    {
        if(selectedLine == 0) 
        {
            if(firstVisLine == 0) return;

            firstVisLine--;

            if((CTRL && GNU_DOS_LEVEL == 1) || (ALT  && GNU_DOS_LEVEL >  1)) 
            {//if using CTRL, goto previous word
                gotoPrevWord(pos-1, lines[pos-1]->charCount-1);
                if(selectedChar < 0) selectedChar = 0;
            } 
            else
            {
                selectedChar = lines[pos-1]->charCount;
                if(lines[pos-1]->text[selectedChar-1] == L'\n') selectedChar--;
            }

            end_selecting();
            //calculate character offset
            calcCharCarry(pos-1);
            calcUserVisibleCurLine();
            refreshAll = 1;
        }
        else 
        {
            selectedLine--;

            if((CTRL && GNU_DOS_LEVEL == 1) || (ALT  && GNU_DOS_LEVEL >  1))
            {//if using CTRL, goto previous word
                gotoPrevWord(pos-1, lines[pos-1]->charCount-1);
                if(selectedChar < 0) selectedChar = 0;
            }
            else 
            {
                selectedChar = lines[pos-1]->charCount;

                if(selectedChar && lines[pos-1]->text[selectedChar-1] == L'\n') 
                    selectedChar--;
            }

            //calculate character offset
            calcCharCarry(pos-1);

            if(selectedChar+selectedCharCarry >= MAX_CHARS_PER_LINE)
                selectedChar--;

            if(lines[pos-1]->charCount >= MAX_CHARS_PER_LINE)
                 move(selectedLine+2,  selectedChar+selectedCharCarry);
            else move(selectedLine+2, selectedChar+1+selectedCharCarry);

            end_selecting();
            calcUserVisibleCurLine();
        } //end if#2
    }
    else 
    {
        if((CTRL && GNU_DOS_LEVEL == 1) || (ALT  && GNU_DOS_LEVEL >  1)) 
        {//if using CTRL, goto previous word
            gotoPrevWord(pos, selectedChar-1);
            if(selectedChar < 0) selectedChar = 0;
        }
        else selectedChar--;

        end_selecting();
        calcCharCarry(pos);
    }

    if(refreshAll) refreshView();
    else
    {
        refreshSelectedLine();
        refreshBottomView();
    }

    selectedCharIfPossible = selectedChar;
}

static inline void do_up(void)
{
    if(!selectedLine && !firstVisLine) return;

    if(SHIFT) start_selecting();
    else cancel_selecting();

    if(selectedLine > 0) selectedLine--;
    else firstVisLine--;

    int pos = firstVisLine+selectedLine;

    selectedChar = selectedCharIfPossible;
    confirmEOL(pos);
    calcCharCarry(pos);
    move(selectedLine+2, selectedChar+1+selectedCharCarry);
    end_selecting();
    calcUserVisibleCurLine();
    refreshView();
}

static inline void do_down(void)
{
    if((firstVisLine+selectedLine) >= totalLines-1) return;

    if(SHIFT) start_selecting();
    else cancel_selecting();

    if(selectedLine < totalVisLines-1) selectedLine++;
    else 
    { 
        if(firstVisLine < totalLines-totalVisLines) firstVisLine++;
        else return;
    }

    int pos = firstVisLine+selectedLine;

    selectedChar = selectedCharIfPossible;
    confirmEOL(pos);
    calcCharCarry(pos);
    move(selectedLine+2, selectedChar+1+selectedCharCarry);
    end_selecting();
    calcUserVisibleCurLine();
    refreshView();
}

static inline void do_del(void)
{
    FILE_STATE    = MODIFIED;

    if(SELECTING) remove_selected_text(1);
    else 
    { 
        if((CTRL && GNU_DOS_LEVEL < 4) || ALT) deleteNextWord();
        else deleteNextChar();
    }
}

static inline void open_file(void)
{
    char *newfile;

    if((newfile = fileMenu_Open(open_file_name)) && newfile != open_file_name)
    {
        if(open_file_name) free(open_file_name);
        open_file_name = newfile;
    }
}

static inline void save_file(void)
{
    char *newfile;

    if((newfile = fileMenu_Save(open_file_name)) && newfile != open_file_name)
    {
        if(open_file_name) free(open_file_name);
        open_file_name = newfile;
    }
}

static inline void new_file(void)
{
    fileMenu_New(open_file_name);

    if(FILE_STATE == NEW)
    {
        if(open_file_name) free(open_file_name);
        open_file_name = NULL;
    }
}

int main(int argc, char **argv) 
{
    //some initialization code
    setlocale(LC_ALL, "");
    parseLineArgs(argc, argv);  
    init(STARTUP_FILE_NAME);

    sel_range_start.nline   = -1;
    sel_range_start.nchar   = -1;
    sel_range_end.nline     = -1;
    sel_range_end.nchar     = -1;

    //clear the screen
    clearScreen();
    setScreenColorsI(COLOR_WINDOW);

    //draw main menu bar
    drawMenuBar(1, 1, SCREEN_W);

    //draw main window
    drawMainWindow(1);
    refreshBottomView();

    if(open_file_at_startup) 
    {
        open_file_name = malloc(strlen(STARTUP_FILE_NAME)+1);
        if(!open_file_name) goto memerr;
        strcpy(open_file_name, STARTUP_FILE_NAME);

        if(!openSaveFile(OPEN, 0, open_file_name))
        {//failed to open file
            __initNewDocument();
            NEW_FILE = 1; FILE_STATE = NEW;
            checkFileExtension(open_file_name);
        }

        refreshView();
    }
  
    if(SHOW_README) showREADMEOnStartup();

    move(2, 1);
    refresh();
 
    int char_inserted = 0;
    char *ch;

    while(!end) 
    {    //infinite program loop//
        ch = ugetKey();

        switch(ch[0]) 
        {
            case(ESC_KEY):
                if(GNU_DOS_LEVEL > 2) break;
                SELECTING = 0;
                refreshView();
                break;

            case(INS_KEY):
                INSERT        = !INSERT;
                char_inserted = 0;
                refreshBottomView();
                break;

            case(CAPS_KEY):
                CAPS          = !CAPS;
                char_inserted = 0;
                refreshBottomView();
                break;

            case(PGUP_KEY):
                if(GNU_DOS_LEVEL > 2) break;
                char_inserted = 0;
                do_pg_up();
                break;

            case(PGDOWN_KEY):
                if(GNU_DOS_LEVEL > 2) break;
                char_inserted = 0;
                do_pg_down();
                break;

            case(HOME_KEY):
                if(GNU_DOS_LEVEL > 2) break;
                char_inserted = 0;
                do_home(ch);
                break;

            case(END_KEY):
                if(GNU_DOS_LEVEL > 2) break;
                char_inserted = 0;
                do_end(ch);
                break;

            case(RIGHT_KEY):
                if(GNU_DOS_LEVEL > 1) break;
                char_inserted = 0;
                do_right();
                break;

            case(LEFT_KEY):
                if(GNU_DOS_LEVEL > 1) break;
                char_inserted = 0;
                do_left();
                break;

            case(UP_KEY):
                if(GNU_DOS_LEVEL > 1) break;
                char_inserted = 0;
                do_up();
                break;

            case(DOWN_KEY):
                if(GNU_DOS_LEVEL > 1) break;
                char_inserted = 0;
                do_down();
                break;

            case(BACKSPACE_KEY):
                FILE_STATE    = MODIFIED;
                char_inserted = 0;

                if(SELECTING) remove_selected_text(1);
                else 
                { 
                    if((CTRL && GNU_DOS_LEVEL < 4) || ALT) deletePrevWord();
                    else deletePrevChar();
                }
                break;

            case(DEL_KEY):
                if(GNU_DOS_LEVEL > 3) break;
                char_inserted = 0;
                do_del();
                break;

            case(ENTER_KEY):
                FILE_STATE    = MODIFIED;
                char_inserted = 1;
                if(SELECTING) remove_selected_text(1);
                insertEnter();
                break;

            case(TAB_KEY):
                FILE_STATE    = MODIFIED;
                if(SELECTING) remove_selected_text(1);
                char_inserted = 1;
                insertTab();
                break;

            case(SPACE_KEY):
            default:
                if(ALT)
                {
                    if(ch[0] == 'f')
                    {
                        if(GNU_DOS_LEVEL > 1) //GNU key binding
                        {
                            char_inserted = 0;
                            do_right();
                        }
                        else
                        {
                            setScreenColorsI(COLOR_WINDOW);
                            showMenu(0);
                        }
                    } 
                    else if(ch[0] == 'e') 
                    {
                        if(GNU_DOS_LEVEL > 1) break;
                        setScreenColorsI(COLOR_WINDOW);
                        showMenu(1);
                    } 
                    else if(ch[0] == 'h') 
                    {
                        if(GNU_DOS_LEVEL > 1) break;
                        setScreenColorsI(COLOR_WINDOW);
                        showMenu(3);
                    } 
                    else if(ch[0] == 'o') 
                    {
                        if(GNU_DOS_LEVEL > 1) break;
                        setScreenColorsI(COLOR_WINDOW);
                        showMenu(2);
                    } 
                    else if(ch[0] == 'b') 
                    { 
                        if(GNU_DOS_LEVEL > 1) //GNU key binding
                        {
                            char_inserted = 0;
                            do_left();
                        }
                    }
                    else if(ch[0] == 'v')
                    { 
                        if(GNU_DOS_LEVEL > 2) //GNU key binding
                        {
                            char_inserted = 0;
                            do_pg_up();
                        }
                    }
                }
                else if(CTRL)
                {
                    if(ch[0] == 'W') 
                    { 
                        if(GNU_DOS_LEVEL < 4) break;
                        FILE_STATE    = MODIFIED;
                        char_inserted = 0;
                        if(SELECTING) remove_selected_text(1);
                    }
                    else if(ch[0] == ' ') 
                    { 
                        if(GNU_DOS_LEVEL > 3) //GNU key binding
                        {   // SHIFT down
                            start_selecting();
                        }
                    }
                    else if(ch[0] == 'G') 
                    { 
                        if(GNU_DOS_LEVEL > 2) //GNU key binding
                        {   // ESC
                            SELECTING = 0;
                            refreshView();
                            break;
                        }
                    }
                    else if(ch[0] == 'V')
                    { 
                        if(GNU_DOS_LEVEL > 2) //GNU key binding
                        {
                            char_inserted = 0;
                            do_pg_down();
                        }
                        else editMenu_Paste();
                    }
                    else if(ch[0] == 'C')
                    { 
                        editMenu_Copy();
                    }
                    else if(ch[0] == '/')
                    { 
                        if(GNU_DOS_LEVEL > 4) editMenu_Undo();//GNU key binding
                    }
                    else if(ch[0] == '_')
                    { 
                        if(GNU_DOS_LEVEL > 4) editMenu_Undo();//GNU key binding
                    }
                    else if(ch[0] == 'A')
                    { 
                        if(GNU_DOS_LEVEL > 2) //GNU key binding
                        {
                            char_inserted = 0;
                            do_home(ch);
                        }
                        else editMenu_SelectAll();
                    }
                    else if(ch[0] == 'Z')
                    { 
                        if(GNU_DOS_LEVEL > 4) break;
                        editMenu_Undo();
                    }
                    else if(ch[0] == 'Y')
                    { 
                        if(GNU_DOS_LEVEL > 4) //GNU key binding
                            editMenu_Paste();
                        else editMenu_Redo();
                    }
                    else if(ch[0] == 'R') 
                    { 
                        editMenu_Replace();
                    }
                    else if(ch[0] == 'F')
                    { 
                        if(GNU_DOS_LEVEL > 1)
                        {
                            char_inserted = 0;
                            do_right();
                        }
                        else editMenu_Find();
                    }
                    else if(ch[0] == 'E') 
                    { 
                        if(GNU_DOS_LEVEL > 2)
                        {
                            char_inserted = 0;
                            do_end(ch);
                        }
                        else editMenu_ToggleSelectMode();
                    }
                    else if(ch[0] == 'O') 
                    {
                        open_file(); 
                    }
                    else if(ch[0] == 'S') 
                    { 
                        if(GNU_DOS_LEVEL > 4) editMenu_Find();
                        else save_file(); 
                    }
                    else if(ch[0] == 'N') 
                    { 
                        if(GNU_DOS_LEVEL > 1)
                        {
                            char_inserted = 0;
                            do_down();
                        }
                        else new_file();
                    }
                    else if(ch[0] == 'P')
                    { 
                        if(GNU_DOS_LEVEL > 1)
                        {
                            char_inserted = 0;
                            do_up();
                        }
                        else fileMenu_Print(open_file_name); 
                    }
                    else if(ch[0] == 'Q') 
                    { 
                        if(GNU_DOS_LEVEL > 4) break;
                        fileMenu_Exit(open_file_name); 
                    }
                    else if(ch[0] == 'D') 
                    { 
                        if(GNU_DOS_LEVEL > 3)
                        {
                            char_inserted = 0;
                            do_del();
                        }
                        else deleteLine(); 
                    }
                    else if(ch[0] == 'K') 
                    { 
                        if(GNU_DOS_LEVEL > 3) deleteLine(); 
                    }
                    else if(ch[0] == 'B') 
                    { 
                        if(GNU_DOS_LEVEL > 1)
                        {
                            char_inserted = 0;
                            do_left();
                        }
                    }
                    else if(ch[0] == 'X')
                    { 
                        if(GNU_DOS_LEVEL <= 4)
                        {
                            editMenu_Cut();
                            break;
                        }

                        //GNU key binding
                        setScreenColorsI(COLOR_STATUS_BAR);
                        mvprintw(SCREEN_H, 0, "[C-f] [C-e] [C-o] [C-h] [C-c] "
                                              "[C-s] [C-u] [C-g]");

                        while(1)
                        {
                            ch = ugetKey();

                            if(!CTRL) continue;

                            if(ch[0] == 'f')
                            { showMenu(0); break; }
                            else if(ch[0] == 'e')
                            { showMenu(1); break; }
                            else if(ch[0] == 'o')
                            { showMenu(2); break; }
                            else if(ch[0] == 'h')
                            { showMenu(3); break; }
                            else if(ch[0] == 'c')
                            { fileMenu_Exit(open_file_name); break; }
                            else if(ch[0] == 's')
                            { save_file(); break; }
                            else if(ch[0] == 'u')
                            { editMenu_Undo(); break; }
                            else if(ch[0] == 'g') { break; }
                        }//end while

                        refreshBottomView();
                        break;
                    }
                }
                else 
                {
                    FILE_STATE = MODIFIED;

                    if(SELECTING)
                    {
                        //if(sel_range_start.nchar == sel_range_end.nchar)
                        //    if(sel_range_start.nline != sel_range_end.nline)
                        if(sel_range_start.nline != -1 && sel_range_end.nline != -1)
                                remove_selected_text(1);
                        cancel_selecting();
                        //SELECTING = 0;
                    }

                    wchar_t wch;
                    int utf8_bytes;

                    for(utf8_bytes = 0; ch[utf8_bytes]; utf8_bytes++) ;
                    mbrtowc(&wch, ch, utf8_bytes, NULL);
                    insertChar(wch);

                    char_inserted = 1;
                    break;
                }//end if #1
                break;
        }//end switch
    }//end while
 
    restoreTerminal();
    fcloseall();
    exit(0);
 
memerr:

    fprintf(stderr, "Fatal error: Insufficient memory\n");
    restoreTerminal();
    fcloseall();
    exit(1);
}//end main


void deleteLine(void)
{
    int pos               = selectedLine+firstVisLine;
    int chr               = selectedChar;
    FILE_STATE            = MODIFIED;

    selectedChar          = 0;
    sel_range_start.nline = pos;
    sel_range_end.nline   = pos+1;
    sel_range_start.nchar = 0;
    sel_range_end.nchar   = 0;
    remove_selected_text(1);

    // remove_selected_text() might have freed the line struct
    if(lines[pos])
    {
        selectedChar = chr;
        if(selectedChar > lines[pos]->charCount)
            selectedChar = lines[pos]->charCount;
    }
    else selectedChar = 0;

    calcUserVisibleCurLine();
    refreshView();
}

static inline int findNextChar(int pos, int selChar)
{
    return (lines[pos]->text[selChar] == L'\0') ? selChar : selChar+1;
}

static inline int findPrevChar(int pos, int selChar)
{
    return (selChar == 0) ? 0 : selChar-1;
}

void copyInLine(int pos, int to, int from, int calcTotalChars)
{
    int len = lines[pos]->charCount-from;

    if(len <= 0)
    {
        lines[pos]->text[to] = L'\0';
    }
    else
    {
        wchar_t tmp[len+1];
        wcscpy(tmp, lines[pos]->text+from);
        wcscpy(lines[pos]->text+to, tmp);
    }

    if(calcTotalChars) calcTotalCharsInLine(pos);
}

int extendLineText(int pos, int newSize)
{
    if(lines[pos]->charsAlloced >= newSize) return 1;
    wchar_t *s = realloc(lines[pos]->text, newSize * sizeof(wchar_t));
    if(!s) return 0;
    lines[pos]->text = s;
    lines[pos]->text[lines[pos]->charCount] = L'\0';
    lines[pos]->charsAlloced = newSize;
    return 1;
}

void checkLineBounds(int pos)
{
    int len, carry = 0;

    calcTotalCharsInLineC(pos, &carry);
    len = lines[pos]->charCount+carry;

    if(len > MAX_CHARS_PER_LINE)
    {
        pos++;
        move_lines_down(totalLines, pos);
        lines[pos]->linkedToNext = lines[pos-1]->linkedToNext;
        lines[pos-1]->linkedToNext = 1;

        int selCharCarry = 0;
        int i = 0, j;
        wchar_t *s = lines[pos-1]->text;

        for( ; i < MAX_CHARS_PER_LINE; s++)
        {
            if(*s == '\t')
            {
                j = TABSPACES(i+selCharCarry+1);
                selCharCarry += j;
                i += j;
            }
            else i++;
        }

        j = s-lines[pos-1]->text;
        wcscpy(lines[pos]->text, lines[pos-1]->text+j);
        lines[pos-1]->text[j] = L'\0';
        calcTotalCharsInLine(pos-1);
        checkLineBounds(pos);
    }
    else
    {
        postDeleteWord(pos);
        calcTotalCharsInLine(pos);
    }
}

static inline void addwch_wrapper(wchar_t ch);

// count is the number of chars to copy. return value
// is the number of chars copied.
static int appendToLine(int posTo, int posFrom, int count)
{
    extendLineText(posTo, maxLen+1);

    wchar_t *to    = lines[posTo]->text+lines[posTo]->charCount;
    wchar_t *from  = lines[posFrom]->text;
    int     copied = 0;

    while(count-- > 0 && *from != L'\0')
    {
        *to++ = *from++;
    }

    *to = L'\0';
    copied = from-lines[posFrom]->text;
    lines[posTo]->charCount += copied;
    return copied;
}

static void postDeleteWord(int pos)
{
    int j;

    while(lines[pos]->linkedToNext) 
    {
        int carry = 0;
        calcTotalCharsInLineC(pos, &carry);
        j = MAX_CHARS_PER_LINE-(carry+lines[pos]->charCount);
        j = appendToLine(pos, pos+1, j);
        copyInLine(pos+1, 0, j, 1);
        if(++pos >= totalLines) break;
    }

    if(pos > 0 && lines[pos]->charCount == 0) 
    {
        if(lines[pos]->text == NULL || lines[pos]->text[0] != L'\n')
        {
            lines[pos-1]->linkedToNext = 0;
            move_lines_up(pos, totalLines-1);
        }
    }
}

static void deleteNextWord(void)
{
    int j, pos = firstVisLine+selectedLine;
    int oF  = firstVisLine;
    int oS  = selectedLine;
    int oC  = selectedChar;

    if(selectedChar >= lines[pos]->charCount) 
    {
        if(pos >= totalLines-1) return;
        lines[pos]->linkedToNext = 1;
        pos++;
        selectedChar = 0;
        selectedCharIfPossible = 0;
        firstVisLine++;
    }

    FILE_STATE = MODIFIED;
    j = selectedChar;
    selectedChar = findNextChar(pos, selectedChar);  

    while(isWhitespace(lines[pos]->text[selectedChar]))
    {
        undoAddWChar(UNDO_ACTION_DELETE, pos, selectedChar, NULL);
        selectedChar++;
    }

    while(!isWhitespace(lines[pos]->text[selectedChar]))
    {
        undoAddWChar(UNDO_ACTION_DELETE, pos, selectedChar, NULL);
        selectedChar++;
    }
    
    if(selectedChar >= lines[pos]->charCount)
         lines[pos]->text[selectedChar] = L'\0';
    else copyInLine(pos, j, selectedChar, 1);
  
    firstVisLine = oF; selectedLine = oS; selectedChar = oC;
    postDeleteWord(firstVisLine+selectedLine);
    refreshView();
}

static void deletePrevWord(void)
{
    int i, j, pos = firstVisLine+selectedLine;
    int       oF  = firstVisLine;
    int       oS  = selectedLine;
    int       oC  = selectedChar;

    if(selectedChar <= 0)
    {
        if(pos <= 0) return;

        lines[--pos]->linkedToNext = 1;
        selectedChar = lines[pos]->charCount-1;

        if(selectedLine == 0) firstVisLine--;
        else selectedLine--;

        if(lines[pos]->text[selectedChar] == L'\n')
        {
            lines[pos]->text[selectedChar] = L'\0';
            lines[pos]->charCount--;
        }

        calcUserVisibleCurLine();
    }

    FILE_STATE = MODIFIED;
    j = selectedChar;

    while(selectedChar && isWhitespace(lines[pos]->text[selectedChar-1])) 
        selectedChar--;

    while(selectedChar && !isWhitespace(lines[pos]->text[selectedChar-1]))
        selectedChar--;

    if(selectedChar < 0) selectedChar = 0;

    for(i = selectedChar; i < j; i++)
    {
        undoAddWChar(UNDO_ACTION_DELETE, pos, i, NULL);
    }

    selectedCharIfPossible = selectedChar;
    copyInLine(pos, selectedChar, j, 1);
    postDeleteWord(pos);
    calcCharCarry(pos);
    refreshView();
}

static void deleteNextChar(void) 
{
    int i, pos = firstVisLine+selectedLine;
    char deleteNL = 0;
    char refreshAll = 0;
    wchar_t c = lines[pos]->text[selectedChar];

    if(c == L'\n' || c == L'\0') deleteNL = 1;
    undoAddWChar(UNDO_ACTION_DELETE, pos, selectedChar, NULL);
    copyInLine(pos, selectedChar, findNextChar(pos, selectedChar), 1);

    if(deleteNL)
    {
        if(pos == totalLines-1) return;

        //is the next line an empty line?
        if(lines[pos+1]->charCount == L'\0' || lines[pos+1]->text[0] == L'\n')
        {
            lines[pos]->linkedToNext = 0;
            move_lines_up(pos+1, totalLines-1);
            refreshAll = 1;
        }
        else
        {
            lines[pos]->linkedToNext = 1;
        }
    }

    if(lines[pos]->linkedToNext) 
    {
        postDeleteWord(pos);
        refreshAll = 1;
    }

    if(refreshAll) refreshView();
    else refreshSelectedLine();
}


static void deletePrevChar(void)
{
    int i, pos;
    int selChar;
    char refreshAll = 0;
    wchar_t c;
  
    if(selectedChar == 0) 
    {
        if(selectedLine == 0)
        {
            if(firstVisLine == 0) return;
            firstVisLine--;      
        }
        else
        {
            selectedLine--;
        }

        pos = firstVisLine+selectedLine;
        selectedChar = lines[pos]->charCount-1;
        if(selectedChar < 0) selectedChar = 0;

        undoAddWChar(UNDO_ACTION_DELETE, pos, selectedChar, NULL);
        c = L'\n';

        // deleting an empty line
        if(lines[pos]->text[selectedChar] == L'\0' ||
           (lines[pos]->text[selectedChar] == L'\n' && !selectedChar))
        {
            move_lines_up(pos, totalLines-1);
            selectedCharIfPossible = 0;
            selectedCharCarry = 0;
            refreshView();
            return;
        }

        refreshAll = 1;

        if(lines[pos]->linkedToNext || lines[pos]->text[selectedChar] == L'\n')
        {
            lines[pos]->text[selectedChar] = L'\0';
            lines[pos]->charCount--;
        }

        lines[pos]->linkedToNext = 1;
        calcUserVisibleCurLine();
    }
    else
    {
        pos = firstVisLine+selectedLine;
        selectedChar--;
        refreshAll = 0;

        undoAddWChar(UNDO_ACTION_DELETE, pos, selectedChar, NULL);
        c = lines[pos]->text[selectedChar];
        i = findNextChar(pos, selectedChar);

        copyInLine(pos, selectedChar, i, 1);
        if(lines[pos]->linkedToNext) refreshAll = 1;
    }

    postDeleteWord(pos);
    selectedCharIfPossible = selectedChar;

    if(c == L'\t' || c == L'\n') calcCharCarry(pos);

    if(refreshAll) refreshView();
    else refreshSelectedLine();
}

static void insertEnter(void) 
{
    int i = 0, j, k;
    int pos = firstVisLine+selectedLine;
    int autoIndentLen = 0;
    int moveCurDown;

    if(AUTO_INDENT)
    {
        for( ; i < lines[userVisibleCurLine]->charCount; i++)
        {
            if(isSpaceOrTab(lines[userVisibleCurLine]->text[i]))
            {
                autoIndentStr[i] = lines[userVisibleCurLine]->text[i];
            }
            else break;
        }

        autoIndentStr[i] = L'\0';
        autoIndentLen = i;
    }

    j = selectedChar;
    undoAddChar(UNDO_ACTION_INSERT, pos, j, L'\n');

    int count = lines[pos]->charCount-j+i;
    wchar_t *newLine = malloc((count+1) * sizeof(wchar_t));

    if(!newLine) return;
    newLine[0] = L'\0';

    if(AUTO_INDENT)
    {
        wcscpy(newLine, autoIndentStr);
        k = j;

        while(i--)
        {
            undoAddChar(UNDO_ACTION_INSERT, pos+1, k, autoIndentStr[k-j]);
            k++;
        }
    }

    if(lines[pos]->text[j] != L'\0') wcscat(newLine, lines[pos]->text+j);

    lines[pos]->text[j  ] = L'\n';
    lines[pos]->text[j+1] = L'\0';
    calcTotalCharsInLine(pos);

    // if Enter is inserted in the middle of a linked line, replace the linked line,
    // otherwise move lines down and add a new line
    if(j == 0 && pos > 0 && lines[pos-1]->linkedToNext)
    {
        if(lines[pos]->text) free(lines[pos]->text);
        lines[pos]->text = newLine;
        lines[pos]->charsAlloced = count+1;
        moveCurDown = 0;
    }
    else
    {
        pos++;
        move_lines_downl(totalLines+1, pos, newLine);
        moveCurDown = 1;
    }

    lines[pos]->linkedToNext = lines[pos-1]->linkedToNext;
    lines[pos-1]->linkedToNext = 0;

    if(count > maxLen)
    {
        pos++;
        move_lines_down(totalLines+1, pos);
        lines[pos]->linkedToNext = lines[pos-1]->linkedToNext;
        lines[pos-1]->linkedToNext = 1;
        j = maxLen;
        wcscpy(lines[pos]->text, lines[pos-1]->text+j);
        lines[pos-1]->text[j] = L'\0';
        calcTotalCharsInLine(pos-1);
    }
    else calcTotalCharsInLine(pos);

    postDeleteWord(pos);

    if(moveCurDown)
    {
        if(selectedLine == totalVisLines-1) firstVisLine++;
        else selectedLine++;
    }

    selectedChar = autoIndentLen;
    selectedCharIfPossible = selectedChar;
    calcCharCarry(pos);
    calcUserVisibleCurLine();
    refreshView();
}


static inline void insertTab(void) 
{
    insertChar(L'\t');
    refreshBottomView();
}

static void insertChar(wchar_t ch) 
{
    int pos = firstVisLine+selectedLine;
    int refreshAll = 0;

    if(WRAP_LINES)
    {
        if(INSERT) 
        {//replace current character
            wchar_t c1 = lines[pos]->text[selectedChar];
            wchar_t c2[2] = { ch, L'\0' };

            undoAddWChar(UNDO_ACTION_REPLACE, pos, selectedChar, c2);
            lines[pos]->text[selectedChar] = ch;

            if(c1 == L'\t' || ch == L'\t')
            {
                checkLineBounds(pos);
                refreshAll = 1;
            }
        }
        else
        {
            undoAddWChar(UNDO_ACTION_INSERT, pos, selectedChar, &ch);
            if(!extendLineText(pos, lines[pos]->charCount+2)) return;
            copyInLine(pos, selectedChar+1, selectedChar, 0);
            lines[pos]->text[selectedChar] = ch;
            checkLineBounds(pos);
            if(lines[pos]->linkedToNext) refreshAll = 1;
        }
    }

    selectedChar++;

    if(selectedChar+selectedCharCarry >= MAX_CHARS_PER_LINE)
    {
        if(selectedLine == totalVisLines-1) firstVisLine--;
        else selectedLine++;

        calcUserVisibleCurLine();
        selectedChar = 0;
        refreshAll = 1;
    }

    selectedCharIfPossible = selectedChar;
    calcCharCarry(firstVisLine+selectedLine);

    if(refreshAll) refreshView();
    else refreshSelectedLine();
}

void refreshSelectedLine(void)
{
    int pos = firstVisLine+selectedLine;
    refreshViewLines(pos, pos, selectedLine);
    refreshBottomView();
    refresh();
}

static inline void outputEmptyLine(int i)
{
    mvprintw(i+2, 1, "%*s", MAX_CHARS_PER_LINE, " ");
}

static inline void padLineWithSpaces(int len)
{
    if(len < MAX_CHARS_PER_LINE)
        printw("%*s", MAX_CHARS_PER_LINE-len, " ");
}

static inline void addwch_wrapper(wchar_t ch)
{
    wchar_t wctmp[2] = { ch, L'\0' };

#ifdef HAVE_NCURSESW_NCURSES_H
    addwstr(wctmp);
#else
    size_t clen;
    char cstr[16];
    clen = wcsrtombs(cstr, (const wchar_t **)&wctmp, 1, NULL);
    cstr[clen] = '\0';
    printw("%s", cstr);
#endif
}

static inline void putwcchar(int pos, int index, int *carry)
{
    if(lines[pos]->text[index] == L'\n') return;

    // check for tabs
    if(lines[pos]->text[index] == L'\t')
    {
        int k = TABSPACES(index+(*carry)+1);
        (*carry) += k;
        if(k) printw("%*s", k+1, " ");
        return;
    }

    addwch_wrapper(lines[pos]->text[index]);
}

static inline int isBraceChar(wchar_t c)
{
    wchar_t *s = specialCharsString;

    while(*s)
    {
        if(c == *s++) return 1;
    }

    return 0;
}

static inline int isQuoteChar(wchar_t c)
{
    return (c == L'"' || c == L'\'');
}

static inline int isSpaceChar(wchar_t c)
{
    return (c == L' ' || c == L'\t' || c == L'\f' || c == L'\r' || c == L'\v');
}

int indexOf(wchar_t *str, wchar_t *substr)
{
    if(!str || ! substr) return -1;
    wchar_t *s = wcsstr(str, substr);
    return s ? s-str : -1;
}

/********************************************************
 * This function colorizes the text according to the
 * predefined highlight colors. It searches for keywords,
 * braces, comments and strings in each line.
 * ******************************************************/
void refreshSelectedLineInColor(int pos, int *incomment)
{
    int j, carry = 0;
    char STRING_STARTED = 0;    // to indicate if we are inside a string
    wchar_t c, quoteChar = L'\0';
    wchar_t *line = lines[pos]->text;
    int len = lines[pos]->charCount;
    int mcstart = indexOf(line, curmodule->mlCommentStart);
    int mcend   = indexOf(line, curmodule->mlCommentEnd  );
    int scstart = indexOf(line, curmodule->slCommentStart);

    for(j = 0; j < lines[pos]->charCount; j++) 
    {
       // is it the start of a multiline comment?
       if(mcstart >= 0 && j == mcstart)
       {
            setScreenColors(COLOR_HCOMMENT, BG_COLOR[COLOR_WINDOW]);
            putwcchar(pos, j, &carry);
            *incomment = 1;
            continue;
       }
       // is it the end of a multiline comment?
       else if(mcend >= 0 && j == mcend)
       {
            wchar_t *mc = curmodule->mlCommentEnd;
            while(*mc++) putwcchar(pos, j++, &carry);
            j--;
            setScreenColorsI(COLOR_WINDOW);
            *incomment = 0;
            continue;
       }
       // are we in a comment?
       else if(*incomment)
       {
           putwcchar(pos, j, &carry);
           continue;
       }
       // is it the start of a single-line comment?
       else if(scstart >= 0 && j == scstart)
       {
            setScreenColors(COLOR_HCOMMENT, BG_COLOR[COLOR_WINDOW]);
            for( ; j < lines[pos]->charCount; j++) 
            {
                putwcchar(pos, j, &carry);
            }
            break;
       }

       // are we inside a string?
       if(STRING_STARTED)
       {
           putwcchar(pos, j, &carry);
           if(quoteChar == line[j])
           {
               STRING_STARTED = 0;
               setScreenColorsI(COLOR_WINDOW);
           }
       }
       else
       {
           c = line[j];

           if(c == L'\n') continue;
           else if(isBraceChar(c) || isSpaceChar(c))
           {
               setScreenColors(COLOR_HBRACES, BG_COLOR[COLOR_WINDOW]);
               putwcchar(pos, j, &carry);
               setScreenColorsI(COLOR_WINDOW);
           }
           else if(isQuoteChar(c))
           {
                if(STRING_STARTED)
                {
                    STRING_STARTED = 0;
                    putwcchar(pos, j, &carry);
                    setScreenColorsI(COLOR_WINDOW);
                }
                else
                {
                    STRING_STARTED = 1;
                    quoteChar = c;
                    setScreenColors(COLOR_HSTRING, BG_COLOR[COLOR_WINDOW]);
                    putwcchar(pos, j, &carry);
                }
           }
           else
           {
                int i;

                if(isKeyword(pos, j, &i))
                {
                    setScreenColors(COLOR_HKEYWORD, BG_COLOR[COLOR_WINDOW]);
                    while(i--) putwcchar(pos, j++, &carry);
                    j--;
                    setScreenColorsI(COLOR_WINDOW);
                }
                else
                {
                    while(i--) putwcchar(pos, j++, &carry);
                    j--;
                }
           }
       }        
    }

    setScreenColorsI(COLOR_WINDOW);
    padLineWithSpaces(len+carry);
    refresh();
}

// This function tells refreshSelectedLineInColor() whether
// we are standing on a keyword (to give it keyword color)
// or not. If yes, it returns the length of the keyword.
int isKeyword(int pos, int start, int *wordlen)
{
    int i = 0;
    int result = 0;
    wchar_t *p = lines[pos]->text+start;

    while(*p && !isSpaceChar(*p) && !isBraceChar(*p)) p++, i++;
    if(p[-1] == L'\n') i--;

    wchar_t word[i+1];

    wcsncpy(word, lines[pos]->text+start, i);
    word[i] = L'\0';
    *wordlen = wcslen(word);

    for(i = 0; i < curmodule->keywordCount; i++)
    {
        if(*wordlen != wcslen(curmodule->keywords[i])) continue;
        if(curmodule->caseSensitive) result = wcscmp(word, curmodule->keywords[i]);
        else result = wcscasecmp(word, curmodule->keywords[i]);

        if(result == 0) return 1;
    }

    return 0;
}

void refreshView(void)
{
    int i;

    hideCursor();

    if(totalLines-firstVisLine < totalVisLines && totalLines > totalVisLines)
    {
        i = firstVisLine;
        firstVisLine = totalLines-totalVisLines;
        selectedLine += i-firstVisLine;
    }

    setScreenColorsI(COLOR_WINDOW);

    if(WRAP_LINES)
    {
        if(totalLines < totalVisLines) 
        {
            refreshViewLines(0, totalLines-1, 0);
            setScreenColorsI(COLOR_WINDOW);
            for(i = totalLines; i < totalVisLines; i++) outputEmptyLine(i);
        }
        else 
        {
            refreshViewLines(firstVisLine, firstVisLine+totalVisLines-1, 0);
        }

        move(selectedLine+2, selectedChar+1+selectedCharCarry);
    }

    drawMenuBar(1, 1, SCREEN_W);
    drawMainWindow(0);
    drawScrollBar();
    refreshBottomView();
    showCursor();
    refresh();
}

static int commentStatus(int pos)
{
    int cspos, cepos;

    if(!curmodule->mlCommentStart) return 0;
    if(pos) pos--;

    for(cspos = -1, cepos = -1; pos >= 0; pos--)
    {
        if(cspos == -1)
        {
            if(wcsstr(lines[pos]->text, curmodule->mlCommentStart)) cspos = pos;
        }

        if(cepos == -1)
        {
            if(wcsstr(lines[pos]->text, curmodule->mlCommentEnd)) cepos = pos;
        }

        if(cepos != -1) return 0;
        if(cspos != -1) return 1;
    }

    return 0;
}


void refreshViewLines(int start, int end, int startOutputAt)
{
    int swap = 0;
    int i;

    if(SELECTING) 
    {
        //////////////////////////////////////////
        //If in Selecting Mode
        //////////////////////////////////////////
        swap = may_swap_selection_range();

        for(i = startOutputAt; start <= end; i++, start++)
        {
            int len = lines[start]->charCount;
            int j, k, l;
            int carry = 0;

            move(i+2, 1);

            if(start == sel_range_start.nline)
            { 
                k = sel_range_start.nchar;
                if(sel_range_start.nline == sel_range_end.nline)
                     l = sel_range_end.nchar;
                else l = len-1;
            } 
            else if(start == sel_range_end.nline) 
            { 
                k = 0; l = sel_range_end.nchar;
            } 
            else 
            { 
                k = 0; 
                if(start > sel_range_start.nline && start < sel_range_end.nline)
                     l = len-1;
                else l = -1;
            }

            for(j = 0; j < lines[start]->charCount; j++) 
            {
                if(j >= k && j <= l) setScreenColorsI(COLOR_HIGHLIGHT_TEXT);
                else setScreenColorsI(COLOR_WINDOW);
                putwcchar(start, j, &carry);
            }

            len += carry;
            setScreenColorsI(COLOR_WINDOW);
            if(len < MAX_CHARS_PER_LINE)
                printw("%*s", MAX_CHARS_PER_LINE-len, " ");
        }

        if(swap == 1) swap_lines();//return them back to normal
        if(swap == 2) swap_chars();//return them back to normal
    } 
    else 
    {
        //////////////////////////////////////////
        //If in Regular Mode
        //////////////////////////////////////////
        int incomment = commentStatus(start);
        int j;

        setScreenColorsI(COLOR_WINDOW);

        for(i = startOutputAt; start <= end; i++, start++)
        {
            int carry = 0;
            int len = lines[start]->charCount;

            move(i+2, 1);

            if(len && lines[start]->text[len-1] == L'\n') len--;

            if(AUTO_HIGHLIGHTING)
            {
                if(incomment) setScreenColors(COLOR_HCOMMENT, BG_COLOR[COLOR_WINDOW]);
                else setScreenColorsI(COLOR_WINDOW);

                refreshSelectedLineInColor(firstVisLine+i, &incomment);
            }
            else
            {
                for(j = 0; j < len; j++) 
                {
                    putwcchar(start, j, &carry);
                }

                setScreenColorsI(COLOR_WINDOW);
                padLineWithSpaces(len+carry);
            }
        }
    }
}

static void drawScrollBar(void)
{
    int h = SCREEN_H-5;
    int i;
    double h2;

    setScreenColorsI(COLOR_MENU_BAR);

    for(i = 0; i <= h; i++)
        mvaddch(i+2, SCREEN_W-1, ' ');

    h2  = firstVisLine+selectedLine+1;
    h2 /= totalLines;
    h2 *= h;

    if(h2 < 0) h2 = 0;
    if(h2 > (SCREEN_H-5)) h2 = SCREEN_H-5;

    setScreenColorsI(COLOR_WINDOW);
    mvaddch((int)(h2)+1, SCREEN_W-1, ACS_BULLET);
    move(selectedLine+2, selectedChar+1+selectedCharCarry);
}

void refreshBottomView(void)
{
    int pos = firstVisLine+selectedLine;
    int chr = selectedChar;

    setScreenColorsI(COLOR_STATUS_BAR);
    mvprintw(SCREEN_H-1, 0, "%*s", SCREEN_W, " ");

    if(pos > 0 && lines[pos-1]->linkedToNext)
    {
        // check if this is part of linked line(s)
        while(--pos >= 0 && lines[pos]->linkedToNext)
        {
            chr += lines[pos]->charCount;
        }
    }

    mvprintw(SCREEN_H-1, SCREEN_W-20, "| LINE:%-3d COL:%-3d", userVisibleCurLine+1, chr+1);  

    if(CAPS     ) mvprintw(SCREEN_H-1, SCREEN_W-25, "CAPS");
    if(INSERT   ) mvprintw(SCREEN_H-1, SCREEN_W-29, "INS");
    if(SELECTING) mvprintw(SCREEN_H-1, SCREEN_W-33, "SEL");

    move(SCREEN_H-1, 1);

    switch(FILE_STATE) 
    {
        case(MODIFIED): printw("Modified"); break;
        case(NEW):      printw("New");      break;
        case(SAVED):    printw("Saved");    break;
        case(OPENED):   printw("Opened");   break;
        case(IDLE):     printw("Idle");     break;
    }

    if(selectedChar+selectedCharCarry > MAX_CHARS_PER_LINE-1)
         move(selectedLine+2, selectedChar+selectedCharCarry);
    else move(selectedLine+2, selectedChar+1+selectedCharCarry);

    refresh();
}

/***************************************
 * drawMenuBar(): 
 * Procedure to draw the main menu bar.
 ***************************************/
static void drawMenuBar(int x, int y, int w) 
{
    int i, j;

    setScreenColorsI(COLOR_MENU_BAR);
    move(x-1, y-1);        //reposition the cursor
    for(i = 0; i < w; i++) addch(' ');    //Draw empty menu bar
    move(x-1, y-1);        //reposition the cursor

    for(i = 0; i < totalMainMenus; i++) 
    {
        j=0;
        addch(' ');

        while(menu[i][j] != '\0') 
        {
            if(menu[i][j] == '&') 
            {    //turn on underline feature to print the shortcut key
                attron(A_UNDERLINE);
                setScreenColorsI(COLOR_HIGHLIGHT_TEXT);
                addch(menu[i][++j]);
                attroff(A_UNDERLINE);
                setScreenColorsI(COLOR_MENU_BAR);
            }
            //print normal chars (other than the shortcut key)
            else addch(menu[i][j]);

            j++;
        }
        addch(' ');
    }

    setScreenColorsI(COLOR_WINDOW);
    reset_attribs();
    refresh();
}

void move_lines_up(int first, int last)
{
    int i;

    if(first == last) return;

    for(i = first; i < last; i++)
    {
        copyLineStruct(i, i+1);
    }

    if(lines[last]->text) free(lines[last]->text);
    free(lines[last]);
    lines[last] = NULL;
    totalLines--;
}

// shift lines up by the difference between first and last lines
void move_lines_upd(int first, int diff)
{
    int i = first;

    if(diff == 0) return;

    if(first != totalLines-diff)
    {
        for( ; i < totalLines-diff; i++)
        {
            copyLineStruct(i, i+diff);
        }
    }

    for( ; i < totalLines; i++)
    {
        if(lines[i]->text) free(lines[i]->text);
        free(lines[i]);
        lines[i] = NULL;
    }

    totalLines -= diff;
}

void move_lines_downl(int first, int last, wchar_t *newLineText)
{
    int i;

    if(first == last) return;

    for(i = first; i > last; i--)
    {
        copyLineStruct(i, i-1);
    }

    totalLines++;

    if(newLineText)
    {
        lines[last] = allocLineStruct();
        lines[last]->text = newLineText;
        calcTotalCharsInLine(last);
        lines[last]->charsAlloced = lines[last]->charCount;
    }
    else lines[last] = allocLineStructB(maxLen);
}

void move_lines_down(int first, int last)
{
    move_lines_downl(first, last, NULL);
}

void calcCharCarry(int pos)
{
    int i = 0, j;
    wchar_t *s = lines[pos]->text;

    selectedCharCarry = 0;

    for( ; i < selectedChar; i++, s++)
    {
        if(*s == L'\t')
        {
            j = TABSPACES(i+selectedCharCarry+1);
            selectedCharCarry += j;
        }
    }
}

