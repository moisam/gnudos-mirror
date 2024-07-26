/* 
 *    Programmed By: Mohammed Isam Mohammed [mohammed_isam1984@yahoo.com]
 *    Copyright 2014, 2015, 2016, 2017, 2018, 2024 (c)
 * 
 *    file: screen.c
 *    This file is part of the GnuDOS project.
 *
 *    GnuDOS is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    GnuDOS is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with GnuDOS.  If not, see <http://www.gnu.org/licenses/>.
 */    

#include "screen.h"
#include <ncurses.h>
#include <stdio.h>
#include <string.h>
//#include <sys/ioctl.h>	//included for terminal size query

/* array to save foreground and background colors */
int FG_COLOR[color_components];
int BG_COLOR[color_components];

/* Screen width and height */
int SCREEN_W;
int SCREEN_H;

/* Strings representing screen colors (e.g., Black, White, ...) */
char *screen_colors[16];

static int ncurses_colors[] =
{
    COLOR_BLACK, COLOR_RED, COLOR_GREEN, COLOR_YELLOW,
    COLOR_BLUE, COLOR_MAGENTA, COLOR_CYAN, COLOR_WHITE,
};


void initScreenColors(void)
{
    int i, j, index = 1;

    start_color();          // init color mode

    // we have to define an ncurses color pair for every possible combination
    for(i = 0; i < 8; i++)
    {
        for(j = 0; j < 8; j++, index++)
        {
            init_pair(index, ncurses_colors[i], ncurses_colors[j]);
        }
    }
}

/* 
 * Set Screen Colors to ForeGround (FG) and BackGround (BG) colors 
 */
void setScreenColors(int FG, int BG) 
{
    if(BG == BGDEFAULT) BG = BGBLACK;
    if(FG < BLACK || FG > WHITE || BG < BGBLACK || BG > BGWHITE) return;

    // calculate index and tell ncurses to change the color
    int index = ((FG - BLACK) * 8) + (BG - BGBLACK) + 1;
    attron(COLOR_PAIR(index));
}

void setScreenColorsI(int colorIndex) 
{
    int FG = FG_COLOR[colorIndex];
    int BG = BG_COLOR[colorIndex];

    if(BG == BGDEFAULT) BG = BGBLACK;

    // calculate index and tell ncurses to change the color
    int index = ((FG - BLACK) * 8) + (BG - BGBLACK) + 1;
    attron(COLOR_PAIR(index));
}

/*
 * Get Screen Size
 */
void getScreenSize(void) 
{
    SCREEN_H = LINES;
    SCREEN_W = COLS;
}

/* 
 * Clear the screen with the specified colors 
 */
void clearScreenC(int FG, int BG) 
{
    setScreenColors(FG, BG);

    // tell ncurses to clear the screen
    clear();
    refresh();
}

/* 
 * Clear the screen 
 */
void clearScreen(void)
{
    // tell ncurses to clear the screen
    clear();
    refresh();
}

/* 
 * Set the cursor at the given row and column 
 */
void locate(int row, int col) 
{
    // tell ncurses to move the cursor (our indexing is 1-based, ncurses is
    // zero-based)
    move(row - 1, col - 1);
    refresh();
}

/* 
 * Get Screen Colors 
 */
void getScreenColors(void) 
{
    screen_colors[0] = "BLACK";
    screen_colors[1] = "RED";
    screen_colors[2] = "GREEN";
    screen_colors[3] = "BROWN";
    screen_colors[4] = "BLUE";
    screen_colors[5] = "MAGENTA";
    screen_colors[6] = "CYAN";
    screen_colors[7] = "WHITE";
    screen_colors[8] = "BGBLACK";
    screen_colors[9] = "BGRED";
    screen_colors[10] = "BGGREEN";
    screen_colors[11] = "BGBROWN";
    screen_colors[12] = "BGBLUE";
    screen_colors[13] = "BGMAGENTA";
    screen_colors[14] = "BGCYAN";
    screen_colors[15] = "BGWHITE";
}

/* 
 * Load Default Colors into the Color Array 
 */
void loadDefaultColors(void) 
{
    FG_COLOR[COLOR_WINDOW]         = 37;
    FG_COLOR[COLOR_HIGHLIGHT_TEXT] = 34;
    FG_COLOR[COLOR_MENU_BAR]       = 34;
    FG_COLOR[COLOR_STATUS_BAR]     = 34;
    FG_COLOR[COLOR_BUTTONS]        = 37;
    FG_COLOR[COLOR_HBUTTONS]       = 32;
    BG_COLOR[COLOR_WINDOW]         = 49;
    BG_COLOR[COLOR_HIGHLIGHT_TEXT] = 47;
    BG_COLOR[COLOR_MENU_BAR]       = 47;
    BG_COLOR[COLOR_STATUS_BAR]     = 47;
    BG_COLOR[COLOR_BUTTONS]        = 41;
    BG_COLOR[COLOR_HBUTTONS]       = 41;
}

void showCursor(void)
{
    curs_set(1);
    refresh();
}

void hideCursor(void)
{
    curs_set(0);
    refresh();
}

void reset_attribs(void)
{
    attroff(A_STANDOUT|A_UNDERLINE|A_REVERSE|A_BLINK|A_DIM|A_BOLD|A_INVIS);
    refresh();
}

