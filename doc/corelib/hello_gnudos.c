/* 
 *    Programmed By: Mohammed Isam Mohammed [mohammed_isam1984@yahoo.com]
 *    Copyright 2014, 2024 (c)
 * 
 *    file: hello_gnudos.c
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
#include "console/dialogs.h"
#include "console/screen.h"
#include "console/kbd.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int main(int argc, char *argv[]) 
{
    if(!initTerminal()) 
    {
        fprintf(stderr, "Error initializing keyboard. Aborting.\n");
        exit(1);
    }
 
    // clear the screen
    clearScreen();

    // set screen colors
    setScreenColors(FG_COLOR[COLOR_WINDOW], BG_COLOR[COLOR_WINDOW]);

    // draw a box with given coordinates, title, and set
    // clearing of box area to YES
    getScreenSize();
    drawBox(2, 1, SCREEN_H-1, SCREEN_W, " Example ", YES);
 
    // loop until we get a hit on ENTER
    while(1) 
    {
        if(getKey() == ENTER_KEY) break;
    }

    // very important to restore keyboard state to its
    // previous state before exiting
    restoreTerminal();

    exit(0);
}//end main

