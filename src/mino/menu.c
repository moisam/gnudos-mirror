/* 
 *    Programmed By: Mohammed Isam Mohammed [mohammed_isam1984@yahoo.com]
 *    Copyright 2014, 2015, 2016, 2017, 2018, 2024 (c)
 * 
 *    file: menu.c
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

#include "config.h"
#define _GNU_SOURCE
#define _XOPEN_SOURCE_EXTENDED
#ifdef HAVE_NCURSESW_NCURSES_H
# include <ncursesw/ncurses.h>
#else
# include <ncurses.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include "defs.h"
#include "kbd.h"
#include "file.h"
#include "edit.h"
#include "options.h"
#include "file.h"
#include "help.h"
#include "dialogs.h"

// defined in main.c
extern char *open_file_name;

//Menu items//
char *menu[4];
char *fileMenu[6];
char *editMenu[10];
char *optionsMenu[4];
char *helpMenu[4];

struct menustruct
{
    int  box[4];
    char **menuItems;
    int  menuItemCount;
    void (**func)();
};

int mSelect;
int endme;
int menuIndex;
struct menustruct *curMenu;

char *(*fileFuncs[])() =
{
    fileMenu_New, fileMenu_Open, fileMenu_Save, fileMenu_SaveAs, 
    fileMenu_Print, fileMenu_Exit
};

void (*editFuncs[])() =
{
    editMenu_Cut, editMenu_Copy, editMenu_Paste, editMenu_SelectAll, 
    editMenu_Undo, editMenu_Redo, editMenu_DeleteLine, editMenu_Find, 
    editMenu_Replace, editMenu_ToggleSelectMode,
};

void (*optionsFuncs[])() =
{
    optionsMenu_Change_Colors, optionsMenu_Tab_Spaces, optionsMenu_Autoindent,
    optionsMenu_Reset_Config
};

void (*helpFuncs[])() =
{
    showReadMe, showKeybindings, showQuickReference, showAboutBox
};

struct menustruct mainMenu[] =
{
    /* File menu */
    {
        { 2, 2, 9, 19 }, fileMenu, fTotal, NULL /* fileFuncs */,
    },
    /* Edit menu */
    {
        { 2, 8, 13, 30 }, editMenu, eTotal, editFuncs,
    },
    /* Options menu */
    {
        { 2, 14, 7, 31 }, optionsMenu, oTotal, optionsFuncs,
    },
    /* Help menu */
    {
        { 2, 23, 7, 40 }, helpMenu, hTotal, helpFuncs,
    },
};
    
static void drawMenuBox(struct menustruct *curMenu)
{
    int i;
    int x1 = curMenu->box[0];
    int y1 = curMenu->box[1];
    int x2 = curMenu->box[2];
    int y2 = curMenu->box[3];
    char **menuItems = curMenu->menuItems;
    int menuItemCount = curMenu->menuItemCount;

    setScreenColorsI(COLOR_WINDOW);
    drawBox(x1++, y1++, x2, y2, NULL, 1);
    setScreenColorsI(COLOR_HIGHLIGHT_TEXT);
    mvprintw(x1-1, y1-1, "%s", menuItems[0]);
    setScreenColorsI(COLOR_WINDOW);

    for(i = 1; i < menuItemCount; i++) 
    {
        mvprintw(i+2, y1-1, "%s", menuItems[i]);
    }

    move(x1-1, y2-2);
    refresh();
}

static void clearSelection(int y, int menuIndex, char **menuItems)
{
    //clear last selection
    setScreenColorsI(COLOR_WINDOW);
    mvprintw(menuIndex+2, y-1, "%s", menuItems[menuIndex]);
}

static void highlightSelection(int y1, int y2, int menuIndex, char **menuItems)
{
    setScreenColorsI(COLOR_HIGHLIGHT_TEXT);
    mvprintw(menuIndex+2, y1-1, "%s", menuItems[menuIndex]);
    move(menuIndex+2, y2-1);
    refresh();
}

static void doUpDown(int step)
{
    clearSelection(curMenu->box[1]+1, mSelect, curMenu->menuItems);
    mSelect += step;
    if(mSelect < 0) mSelect = curMenu->menuItemCount-1;
    else if(mSelect >= curMenu->menuItemCount) mSelect = 0;
    highlightSelection(curMenu->box[1]+1, curMenu->box[3]-1, mSelect, 
                       curMenu->menuItems);
}

static void doRightLeft(int step)
{
    menuIndex += step;
    if(menuIndex < 0) menuIndex = totalMainMenus-1;
    else if(menuIndex == totalMainMenus) menuIndex = 0;
    refreshView();
}

/***********************************************************************
 * This procedure shows the requested menu under the main menu bar. 
 * It also takes control of the user input to navigate the menu
 * with the arrow keys and to select menu items with ENTER. Pressing 
 * right or left arrows navigate to next menu on the left and the right 
 * respectively.
 ***********************************************************************/
void showMenu(int index)
{
    char *ch;
    menuIndex = index;
    hideCursor();
    
loop:

    mSelect   = 0;
    endme     = 0;
    curMenu   = &mainMenu[menuIndex];
    drawMenuBox(curMenu);

    while(!endme) 
    {
        ch = ugetKey();

        switch(ch[0]) 
        {
            case('f'):                      // switch to File menu
                if(ALT)
                {
                    if(menuIndex == 0) { endme = 1; break; }
                    menuIndex = 0;
                    refreshView();
                    goto loop;
                }
                else if(CTRL)
                {
                    if(GNU_DOS_LEVEL > 1)
                    {
                        doRightLeft(1);
                        goto loop;
                    }
                }
                break;

            case('e'):                      // switch to Edit menu
                if(ALT)
                {
                    if(menuIndex == 1) { endme = 1; break; }
                    menuIndex = 1;
                    refreshView();
                    goto loop;
                }
                break;

            case('o'):                      // switch to Options menu
                if(ALT)
                {
                    if(menuIndex == 2) { endme = 1; break; }
                    menuIndex = 2;
                    refreshView();
                    goto loop;
                }
                break;

            case('h'):                      // switch to Help menu
                if(ALT)
                {
                    if(menuIndex == 3) { endme = 1; break; }
                    menuIndex = 3;
                    refreshView();
                    goto loop;
                }
                break;

            case('g'):
                if(GNU_DOS_LEVEL < 3) break;
                if(!CTRL) break;
                endme = 1;
                break;

            case(ESC_KEY):
                if(GNU_DOS_LEVEL > 2) break;
                endme = 1;
                break;

            case('p'):
                if(GNU_DOS_LEVEL < 2) break;
                if(!CTRL) break;
                doUpDown(-1);
                break;

            case(UP_KEY):
                if(GNU_DOS_LEVEL > 1) break;
                doUpDown(-1);
                break;

            case('n'):
                if(GNU_DOS_LEVEL < 2) break;
                if(!CTRL) break;
                doUpDown(1);
                break;

            case(DOWN_KEY):
                if(GNU_DOS_LEVEL > 1) break;
                doUpDown(1);
                break;

            case(RIGHT_KEY):
                if(GNU_DOS_LEVEL > 1) break;
                doRightLeft(1);
                goto loop;
                break;

            case('b'):
                if(GNU_DOS_LEVEL < 2) break;
                if(!CTRL) break;
                doRightLeft(-1);
                goto loop;
                break;

            case(LEFT_KEY):
                if(GNU_DOS_LEVEL > 1) break;
                doRightLeft(-1);
                goto loop;
                break;

            case(ENTER_KEY):
                showCursor();
                if(menuIndex == 0)      // File menu has special treatment
                {
                    char *newfile;

                    if((newfile = fileFuncs[mSelect](open_file_name)) &&
                        newfile != open_file_name)
                    {
                        if(open_file_name) free(open_file_name);
                        open_file_name = newfile;
                    }

                    // new file
                    if(mSelect == 0)
                    {
                        if(open_file_name) free(open_file_name);
                        open_file_name = NULL;
                    }
                }
                else
                {
                    curMenu->func[mSelect]();
                }
                endme = 1;
                break;
        }
    }

    setScreenColorsI(COLOR_WINDOW);
    refreshView();
    showCursor();
    refresh();
}

