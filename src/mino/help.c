/* 
 *    Programmed By: Mohammed Isam Mohammed [mohammed_isam1984@yahoo.com]
 *    Copyright 2014, 2015, 2016, 2017, 2018, 2024 (c)
 * 
 *    file: help.c
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
#include "help.h"
#include "kbd.h"
#include "dialogs.h"

void showAboutBox(void)
{
    showAbout(copyrightNotice);
}

void showQuickReference(void)
{
    drawBox(3, 5, 24, SCREEN_W-5, "Quick Reference", 1);
    mvprintw(3, 6, "Basic functions:");
    mvprintw(4, 8, "Arrow keys: move around");
    mvprintw(5, 8, "ALT+F: Open File menu");
    mvprintw(6, 8, "ALT+E: Open Edit menu");
    mvprintw(7, 8, "ALT+O: Open Options menu");
    mvprintw(8, 8, "ALT+H: Open Help menu");
    mvprintw(10, 6, "Shortcut keys:");
    mvprintw(11, 8, "CTRL+O: Open file dialog");
    mvprintw(12, 8, "CTRL+S: Save file");
    mvprintw(13, 8, "CTRL+Q: Exit mino");
    mvprintw(14, 8, "CTRL+X: Cut selection");
    mvprintw(15, 8, "CTRL+C: Copy selection");
    mvprintw(16, 8, "CTRL+V: Paste selection");
    mvprintw(17, 8, "CTRL+Z: Undo");
    mvprintw(18, 8, "CTRL+Y: Redo");
    mvprintw(19, 8, "CTRL+A: Select All");
    mvprintw(20, 8, "CTRL+F: Find");
    mvprintw(21, 8, "CTRL+R: Find & Replace");
    mvprintw(22, 6, "Press any key to continue..");
    refresh();

    do {;} while(!getKey());
}

////////////////////////////////////////////////////////
//shows a window with KEYBINDINGS file as its contents//
////////////////////////////////////////////////////////
void showKeybindings(void) 
{
    FILE *file;

    if(!(file = fopen("/usr/share/doc/gnudos/mino/keybindings", "r")))
    {
        if(!(file = fopen("/usr/local/share/doc/gnudos/mino/keybindings", "r")))
        {
            msgBoxH("Failed to open keybindings file!.", BUTTON_OK, ERROR);
            drawBoxWC(2, 1, SCREEN_H-1, SCREEN_W, documentTitle, 1);
            refreshView();
            return;
        }
    }

    showReadme(file, " KEYBINDINGS ", GNU_DOS_LEVEL);
    fclose(file);

    drawBoxWC(2, 1, SCREEN_H-1, SCREEN_W, documentTitle, 1);
    refreshView();
}

///////////////////////////////////////////////////
//shows a window with README file as its contents//
///////////////////////////////////////////////////
void showReadMe(void) 
{
    FILE *file;

    if(!(file = fopen("/usr/share/doc/gnudos/mino/README", "r")))
    {
        if(!(file = fopen("/usr/local/share/doc/gnudos/mino/README", "r")))
        {
            msgBoxH("Failed to open README file!.", BUTTON_OK, ERROR);
            drawBoxWC(2, 1, SCREEN_H-1, SCREEN_W, documentTitle, 1);
            refreshView();
            return;
        }
    }

    showReadme(file, " README ", GNU_DOS_LEVEL);
    fclose(file);

    drawBoxWC(2, 1, SCREEN_H-1, SCREEN_W, documentTitle, 1);
    refreshView();
}

