/* 
 *    Programmed By: Mohammed Isam Mohammed [mohammed_isam1984@yahoo.com]
 *    Copyright 2014, 2015, 2016, 2017, 2018, 2024 (c)
 * 
 *    file: kbd.c
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

#include <stdlib.h>
#include <ncurses.h>
#include "kbd.h"

/* Control (Meta) Keys */
int ALT, CTRL, SHIFT, CAPS, INSERT;

/* Some extended keys */
static int CTRL_UP, CTRL_DOWN, CTRL_LEFT, CTRL_RIGHT;
static int CTRL_HOME, CTRL_END, CTRL_DEL;

extern void initScreenColors(void);

static inline int get_ext_key(char *description)
{
    // See "Terminal Capability Functions" under:
    //     https://invisible-island.net/ncurses/man/curs_terminfo.3x.html
    // And the "Extended Key Definitions" under:
    //     https://invisible-island.net/ncurses/man/user_caps.5.html#h3-Extended-key-definitions
    char *str = tigetstr(description);
    
    if(str == (char *)-1 ||     // not a string capability
       str == NULL)             // capability cancelled or absent
    {
        return 0;
    }
    
    return key_defined(str);
}

/*
 * Initialize the terminal, setting the proper flags.
 */
int initTerminal()
{
    // force ncurses to output unicode chars for box drawing
    setenv("NCURSES_NO_UTF8_ACS", "1", 1);

    initscr();              // init curses
    raw();                  // disable line buffering
    noecho();               // no echo
    keypad(stdscr, TRUE);   // get function and arrow keys

    initScreenColors();     // init colors

    // see if some extended keys are defined
    CTRL_UP = get_ext_key("kUP5");
    CTRL_DOWN = get_ext_key("kDN5");
    CTRL_LEFT = get_ext_key("kLFT5");
    CTRL_RIGHT = get_ext_key("kRIT5");
    CTRL_HOME = get_ext_key("kHOM5");
    CTRL_END = get_ext_key("kEND5");
    CTRL_DEL = get_ext_key("kDC5");

    ALT   = 0; 
    CTRL  = 0; 
    SHIFT = 0;

    return 1;
}

/*
 * Restore the terminal to it's previous state.
 * MUST be called before your program exits!.
 */
void restoreTerminal()
{
    endwin();               // end curses mode
}

int filterKey(int c)
{
    switch(c)
    {
        case 9            : c = TAB_KEY; break;
        case 10           : c = ENTER_KEY; break;
        case KEY_BACKSPACE: c = BACKSPACE_KEY; break;
        case KEY_ENTER    : c = ENTER_KEY; break;
        case KEY_UP       : c = UP_KEY; break;
        case KEY_DOWN     : c = DOWN_KEY; break;
        case KEY_LEFT     : c = LEFT_KEY; break;
        case KEY_RIGHT    : c = RIGHT_KEY; break;
        case KEY_DC       : c = DEL_KEY; break;
        case KEY_HOME     : c = HOME_KEY; break;
        case KEY_END      : c = END_KEY; break;
        case KEY_IC       : c = INS_KEY; break;
        case KEY_PPAGE    : c = PGUP_KEY; break;
        case KEY_NPAGE    : c = PGDOWN_KEY; break;
        case KEY_F(1)     : c = F1_KEY; break;
        case KEY_F(2)     : c = F2_KEY; break;
        case KEY_F(3)     : c = F3_KEY; break;
        case KEY_F(4)     : c = F4_KEY; break;
        case KEY_F(5)     : c = F5_KEY; break;
        case KEY_F(6)     : c = F6_KEY; break;
        case KEY_F(7)     : c = F7_KEY; break;
        case KEY_F(8)     : c = F8_KEY; break;
        case KEY_F(9)     : c = F9_KEY; break;
        case KEY_F(10)    : c = F10_KEY; break;
        case KEY_F(11)    : c = F11_KEY; break;
        case KEY_F(12)    : c = F12_KEY; break;

        // SHIFT-keys
        case KEY_BTAB     : c = TAB_KEY; SHIFT = 1; break;
        case KEY_SHOME    : c = HOME_KEY; SHIFT = 1; break;
        case KEY_SEND     : c = END_KEY; SHIFT = 1; break;
        case KEY_SLEFT    : c = LEFT_KEY; SHIFT = 1; break;
        case KEY_SRIGHT   : c = RIGHT_KEY; SHIFT = 1; break;
        case KEY_SR       : c = UP_KEY; SHIFT = 1; break;
        case KEY_SF       : c = DOWN_KEY; SHIFT = 1; break;
        case KEY_SPREVIOUS: c = PGUP_KEY; SHIFT = 1; break;
        case KEY_SNEXT    : c = PGDOWN_KEY; SHIFT = 1; break;
        case KEY_SDC      : c = DEL_KEY; SHIFT = 1; break;

        case ESC_KEY:
            // check for ALT-key
            timeout(10);
            c = getch();
            timeout(-1);

            if(c == ERR) c = ESC_KEY;
            else ALT = 1;

            if(c == 10) c = ENTER_KEY;
            break;

        default:
            if(c < 0x20)
            {
                CTRL = 1;
                c += 0x40;
                
                // special treatment for CTRL-backspace
                if(c == 'H') c = BACKSPACE_KEY;
            }
            else
            {
                if(c == CTRL_UP) { c = UP_KEY; CTRL = 1; }
                else if(c == CTRL_DOWN) { c = DOWN_KEY; CTRL = 1; }
                else if(c == CTRL_LEFT) { c = LEFT_KEY; CTRL = 1; }
                else if(c == CTRL_RIGHT) { c = RIGHT_KEY; CTRL = 1; }
                else if(c == CTRL_HOME) { c = HOME_KEY; CTRL = 1; }
                else if(c == CTRL_END) { c = END_KEY; CTRL = 1; }
                else if(c == CTRL_DEL) { c = DEL_KEY; CTRL = 1; }
            }
            break;
    }

    return c;
}

int getKey(void)
{
    ALT = 0; CTRL = 0; SHIFT = 0;

    // get key from ncurses and map it to our internal values
    return filterKey(getch());
}

