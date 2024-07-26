/* 
 *    Programmed By: Mohammed Isam Mohammed [mohammed_isam1984@yahoo.com]
 *    Copyright 2013, 2014, 2015, 2016, 2017, 2018, 2024 (c)
 * 
 *    file: menu.c
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

#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <pwd.h>
#include "defs.h"
#include "options.h"
//#include "print.h"
#include "edit.h"
#include "find.h"
#include "file.h"
#include "help.h"

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

char *menu[4];
char *fileMenu[6];
char *editMenu[8];
char *optionsMenu[3];
char *helpMenu[5];

void (*fileFuncs[])() =
{
    fileMenu_CreateDir, fileMenu_Open, fileMenu_ExportTree, 
    fileMenu_Print, fileMenu_Exit
};

void (*editFuncs[])() =
{
    cutMarked, copyMarked, pasteMarked, markAll, unMarkAll, 
    clearSelection, editMenu_Find,
    editMenu_Properties
};

void (*optionsFuncs[])() =
{
    optionsMenu_Properties, optionsMenu_Change_Colors, optionsMenu_Reset_Config
};

void (*helpFuncs[])() =
{
    showReadMe, showKeybindings, showQuickReference, showAboutBox
};

struct menustruct mainMenu[] =
{
    /* File menu */
    {
        { 3, 2, 9, 19 }, fileMenu, fTotal, fileFuncs,
    },
    /* Edit menu */
    {
        { 3, 8, 11, 22 }, editMenu, eTotal, editFuncs,
    },
    /* Options menu */
    {
        { 3, 14, 7, 30 }, optionsMenu, oTotal, optionsFuncs,
    },
    /* Help menu */
    {
        { 3, 23, 8, 39 }, helpMenu, hTotal, helpFuncs,
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
        mvprintw(i+3, y1-1, "%s", menuItems[i]);
    }

    move(x1-1, y2-2);
    refresh();
}

static void clearMenuSelection(int y, int menuIndex, char **menuItems)
{
    //clear last selection
    setScreenColorsI(COLOR_WINDOW);
    mvprintw(menuIndex+3, y-1, "%s", menuItems[menuIndex]);
    refresh();
}

static void highlightSelection(int y1, int y2, int menuIndex, char **menuItems)
{
    setScreenColorsI(COLOR_HIGHLIGHT_TEXT);
    mvprintw(menuIndex+3, y1-1, "%s", menuItems[menuIndex]);
    move(menuIndex+3, y2-1);
    refresh();
}

static void doUpDown(int step)
{
    clearMenuSelection(curMenu->box[1]+1, mSelect, curMenu->menuItems);
    mSelect += step;

    if(mSelect < 0) mSelect = curMenu->menuItemCount-1;
    else if(mSelect >= curMenu->menuItemCount) mSelect = 0;

    highlightSelection(curMenu->box[1]+1, curMenu->box[3]-1, 
                       mSelect, curMenu->menuItems);
}

static void doRightLeft(int step)
{
    menuIndex += step;

    if(menuIndex < 0) menuIndex = totalMainMenus-1;
    else if(menuIndex == totalMainMenus) menuIndex = 0;

    refreshWindows();
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
    int ch;
    menuIndex = index;
    hideCursor();
    
loop:

    mSelect   = 0;
    endme     = 0;
    curMenu   = &mainMenu[menuIndex];
    drawMenuBox(curMenu);

    while(!endme) 
    {
        ch = getKey();

        switch(ch)
        {
            case('f'):                      // switch to File menu
                if(ALT)
                {
                    if(menuIndex == 0) { endme = 1; break; }
                    menuIndex = 0;
                    refreshWindows();
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
                    refreshWindows();
                    goto loop;
                }
                break;

            case('o'):                      // switch to Options menu
                if(ALT)
                {
                    if(menuIndex == 2) { endme = 1; break; }
                    menuIndex = 2;
                    refreshWindows();
                    goto loop;
                }
                break;

            case('h'):                      // switch to Help menu
                if(ALT)
                {
                    if(menuIndex == 3) { endme = 1; break; }
                    menuIndex = 3;
                    refreshWindows();
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
                curMenu->func[mSelect]();
                endme = 1;
                break;
        }
    }

    setScreenColorsI(COLOR_WINDOW);
    refreshWindows();
    hideCursor();
    refresh();
}

