/* 
 *    Programmed By: Mohammed Isam Mohammed [mohammed_isam1984@yahoo.com]
 *    Copyright 2014, 2015, 2016, 2017, 2018, 2024 (c)
 * 
 *    file: screen.h
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

#ifndef __SCREEN_H
#define __SCREEN_H

/* Screen width and height */
extern int SCREEN_W;
extern int SCREEN_H;

/* Definitions for colors */
#define BLACK      30      //set black foreground
#define RED        31      //set red foreground
#define GREEN      32      //set green foreground
#define BROWN      33      //set brown foreground
#define BLUE       34      //set blue foreground
#define MAGENTA    35      //set magenta foreground
#define CYAN       36      //set cyan foreground
#define WHITE      37      //set white foreground
#define BGBLACK    40      //set black background
#define BGRED      41      //set red background
#define BGGREEN    42      //set green background
#define BGBROWN    43      //set brown background
#define BGBLUE     44      //set blue background
#define BGMAGENTA  45      //set magenta background
#define BGCYAN     46      //set cyan background
#define BGWHITE    47      //set white background
#define BGDEFAULT  49      //set default background color

#define color_components        6
/* values used as index into the color array */
#define COLOR_WINDOW            0
#define COLOR_HIGHLIGHT_TEXT    1
#define COLOR_MENU_BAR          2
#define COLOR_STATUS_BAR        3
#define COLOR_BUTTONS           4
#define COLOR_HBUTTONS          5

/* array to save foreground and background colors */
extern int FG_COLOR[color_components];
extern int BG_COLOR[color_components];

/* Strings representing screen colors (e.g., Black, White, ...) */
extern char *screen_colors[16];

/* Set Screen Colors to ForeGround (FG) and BackGround (BG) colors */
void setScreenColors(int FG, int BG);
void setScreenColorsI(int colorIndex);

/* Get Screen Size */
void getScreenSize();

/* Clear the screen */
void clearScreen();

/* Clear the screen with the specified colors */
void clearScreenC(int FG, int BG);

/* Set the cursor at the given row and column */
void locate(int row, int col);

/* Get Screen Colors */
void getScreenColors();

/* Load Default Colors into the Color Array */
void loadDefaultColors();

/* Turn on the cursor */
void showCursor();

/* Turn off the cursor */
void hideCursor();

/* reset font attributes */
void reset_attribs();

#endif /* __SCREEN_H */
