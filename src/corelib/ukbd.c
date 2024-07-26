/* 
 *    Programmed By: Mohammed Isam Mohammed [mohammed_isam1984@yahoo.com]
 *    Copyright 2014, 2015, 2024 (c)
 * 
 *    file: ukbd.c
 *    This file is part of the GnuDOS project.
 *
 *    The GnuDOS project is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    The GnuDOS project is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with the GnuDOS project.  If not, see <http://www.gnu.org/licenses/>.
 */    
#include <ncurses.h>
#include "ukbd.h"

/*
 * Gets the next key press from buffer.
 */
char *ugetKey(void)
{
    static unsigned short mask[] = {192, 224, 240};
    static char buf[8];
    int i, c1, c2;
    int bytes = 0;

    ALT = 0; CTRL = 0; SHIFT = 0;

    // get key from ncurses
    c1 = getch();

    // check if it is a translated ncurses key, before checking for UTF-8
    c2 = filterKey(c1);

    if(c2 != c1)
    {
        //fprintf(stdout, "[%x]", c); fflush(stdout);
        // yes, it is something like an arrow key or function key
        buf[0] = c2;
        buf[1] = '\0';
        return buf;
    }

    if((c1 & mask[0]) == mask[0]) bytes++;
    if((c1 & mask[1]) == mask[1]) bytes++;
    if((c1 & mask[2]) == mask[2]) bytes++;

    if(bytes == 0)
    {
        //fprintf(stdout, "[%x, %x]", c1, c2); fflush(stdout);
        // not a UTF-8 sequence
        buf[0] = c2;
        buf[1] = '\0';
        return buf;
    }

    //for(i = 0; i < 8; i++) buf[i] = '\0';

    buf[0] = c1;
    //fprintf(stdout, "[0/%d-%x]\n", bytes, buf[0]); fflush(stdout);

    for(i = 1; i <= bytes; i++)
    {
        buf[i] = getch();
        //fprintf(stdout, "[%d/%d-%x]\n", i, bytes, buf[i]); fflush(stdout);
    }

    //for(int x = 0; x < 8; x++) fprintf(stdout, "[%x]", buf[x]);
    //fflush(stdout);

    buf[i] = '\0';
    return buf;
}

