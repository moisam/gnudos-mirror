/* 
 *    Programmed By: Mohammed Isam Mohammed [mohammed_isam1984@yahoo.com]
 *    Copyright 2013, 2014, 2015, 2016, 2017, 2018, 2024 (c)
 * 
 *    file: help.c
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
#include "defs.h"
#include "help.h"

////////////////////////////////////////////////////////
//shows a window with KEYBINDINGS file as its contents//
////////////////////////////////////////////////////////
void showKeybindings(void)
{
    FILE *file;

    if(!(file = fopen("/usr/share/doc/gnudos/prime/keybindings", "r")))
    {
        if(!(file = fopen("/usr/local/share/doc/gnudos/prime/keybindings", "r")))
        {
            msgBoxH("Failed to open keybindings file!.", BUTTON_OK, ERROR);
            drawMenuBar(2, 2, SCREEN_W-2);    //draw main menu bar
            refreshBottomView();
            return;
        }
    }

    showReadme(file, " KEYBINDINGS ", GNU_DOS_LEVEL);
    fclose(file);

    drawMenuBar(2, 2, SCREEN_W-2);    //draw main menu bar
    refreshBottomView();
}


/******************************************
 * this function shows a window with 
 * README file as its contents.
 * ****************************************/
void showReadMe(void)
{
    FILE *file;

    if(!(file = fopen("/usr/share/doc/gnudos/prime/README", "r")))
    {
        if(!(file = fopen("/usr/local/share/doc/gnudos/prime/README", "r")))
        {
            msgBoxH("Failed to open README file!.", BUTTON_OK, ERROR);
            drawMenuBar(2, 2, SCREEN_W-2);    //draw main menu bar
            refreshBottomView();
            return;
        }
    }

    showReadme(file, " README ", GNU_DOS_LEVEL);
    fclose(file);

    drawMenuBar(2, 2, SCREEN_W-2);    //draw main menu bar
    refreshBottomView();
}

void showQuickReference(void)
{
    drawBox(3, 5, 18, SCREEN_W-5, "Quick Reference", 1);

    mvprintw(3, 6, "Basic functions:");
    mvprintw(4, 8, "SPACEBAR: Toggle select/unselect");
    mvprintw(5, 8, "ENTER: Navigate to directory");
    mvprintw(6, 8, "TAB: Navigate between dir/file view windows");
    mvprintw(7, 8, "Arrow keys: Navigate up/down");
    mvprintw(9, 6, "Editing:");
    mvprintw(10, 8, "DEL: Delete marked directories/files");
    mvprintw(11, 8, "CTRL+X: Cut marked directories/files");
    mvprintw(12, 8, "CTRL+C: Copy marked directories/files");
    mvprintw(13, 8, "CTRL+V: Paste marked directories/files");
    mvprintw(14, 8, "CTRL+F: Find files");
    mvprintw(15, 8, "CTRL+Q: Quit the program");
    mvprintw(16, 6, "Press any key to continue..");
    refresh();

    do {;} while(!getKey());

    refreshBottomView();
}

void showAboutBox(void)
{
    showAbout(copyright_notice);
}

