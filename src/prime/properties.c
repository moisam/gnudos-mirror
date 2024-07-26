/* 
 *    Programmed By: Mohammed Isam Mohammed [mohammed_isam1984@yahoo.com]
 *    Copyright 2013, 2014, 2015, 2016, 2017, 2018, 2024 (c)
 * 
 *    file: properties.c
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

#include "config.h"
#define _GNU_SOURCE
#define _XOPEN_SOURCE_EXTENDED
#ifdef HAVE_NCURSESW_NCURSES_H
# include <ncursesw/ncurses.h>
#else
# include <ncurses.h>
#endif
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include "defs.h"
#include "dirstruct.h"
#include "kbd.h"

int UREAD, UWRITE, UEXECUTE;
int GREAD, GWRITE, GEXECUTE;
int OREAD, OWRITE, OEXECUTE;
static int x, y, w, h;
static int sel;


#ifdef HAVE_NCURSESW_NCURSES_H
static inline void refreshNameField(char *input_str, wchar_t *wcstr, 
                                                     size_t wclen)
#else
static inline void refreshNameField(char *input_str)
#endif
{
    int i;

#ifdef HAVE_NCURSESW_NCURSES_H
    if(wcstr)
    {
        if(wclen > 26)
        {
            addnwstr(wcstr, 24);
            addch('.');
            addch('.');
        }
        else addwstr(wcstr);
    }
    else
#endif

    if(strlen(input_str) > 26) 
    {
        for(i = 0; i < 24; i++) addch(input_str[i]);
        addch('.');
        addch('.');
    }
    else printw("%s", input_str);
}

#ifdef HAVE_NCURSESW_NCURSES_H
static void refreshPropertiesDialog(char *input_str, wchar_t *wcstr, 
                                                     size_t wclen)
#else
static void refreshPropertiesDialog(char *input_str)
#endif
{
    //set the user interface
    setScreenColorsI(COLOR_WINDOW);
    move(x, y+1);

    if(activeWindow == DIR_WIN) printw("Dir name:  ");
    else printw("File name: ");

#ifdef HAVE_NCURSESW_NCURSES_H
    refreshNameField(input_str, wcstr, wclen);
#else
    refreshNameField(input_str);
#endif

    mvprintw(x+2, y+1, "Permissions:");
    mvprintw(x+3, y+1, "Owner:      Group:      Others:    ");
    mvprintw(x+4, y+1, "[ ] Read    [ ] Read    [ ] Read   ");
    mvprintw(x+5, y+1, "[ ] Write   [ ] Write   [ ] Write  ");
    mvprintw(x+6, y+1, "[ ] Execute [ ] Execute [ ] Execute");
    setScreenColorsI(COLOR_BUTTONS);
    mvprintw(x+8, y+9, "   OK   ");
    mvprintw(x+8, y+21, " CANCEL ");
    setScreenColorsI(COLOR_WINDOW);

    if(UREAD   ) { mvaddch(x+4, y+3, 'X'); }
    if(UWRITE  ) { mvaddch(x+5, y+3, 'X'); }
    if(UEXECUTE) { mvaddch(x+6, y+3, 'X'); }
    if(GREAD   ) { mvaddch(x+4, y+14, 'X'); }
    if(GWRITE  ) { mvaddch(x+5, y+14, 'X'); }
    if(GEXECUTE) { mvaddch(x+6, y+14, 'X'); }
    if(OREAD   ) { mvaddch(x+4, y+26, 'X'); }
    if(OWRITE  ) { mvaddch(x+5, y+26, 'X'); }
    if(OEXECUTE) { mvaddch(x+6, y+26, 'X'); }
  
    setScreenColorsI(COLOR_HIGHLIGHT_TEXT);

    switch(sel)
    {
        case 0:
            move(x, y+12);

#ifdef HAVE_NCURSESW_NCURSES_H
            refreshNameField(input_str, wcstr, wclen);
#else
            refreshNameField(input_str);
#endif

            break;

        case 1:
            mvprintw(x+4, y+5, "Read");
            break;

        case 2:
            mvprintw(x+4, y+17, "Read");
            break;

        case 3:
            mvprintw(x+4, y+29, "Read");
            break;

        case 4:
            mvprintw(x+5, y+5, "Write");
            break;

        case 5:
            mvprintw(x+5, y+17, "Write");
            break;

        case 6:
            mvprintw(x+5, y+29, "Write");
            break;

        case 7:
            mvprintw(x+6, y+5, "Execute");
            break;

        case 8:
            mvprintw(x+6, y+17, "Execute");
            break;

        case 9:
            mvprintw(x+6, y+29, "Execute");
            break;

        case 10:
            setScreenColorsI(COLOR_HBUTTONS);
            mvprintw(x+8, y+9, "   OK   ");
            break;

        case 11:
            setScreenColorsI(COLOR_HBUTTONS);
            mvprintw(x+8, y+21, " CANCEL ");
            break;
    }

    refresh();
}

void showPropertiesDialog(void)
{
    int c;
    int endme = 0;
    int NAME_CHANGED = 0;
    char *tmp, *cname, *cnewname = NULL;
    mode_t mode;
    struct stat statbuf;
    struct dirstruct *dir;

#ifdef HAVE_NCURSESW_NCURSES_H
    wchar_t *widechar_name = NULL, *widechar_newname = NULL;
    size_t widechar_namelen = 0, widechar_newnamelen = 0;
#endif

    if(activeWindow == DIR_WIN) dir = dirs[firstVisDir+selectedDir];
    else dir = files[firstVisFile+selectedFile];
 
    UREAD = 0; UWRITE = 0; UEXECUTE = 0;
    GREAD = 0; GWRITE = 0; GEXECUTE = 0;
    OREAD = 0; OWRITE = 0; OEXECUTE = 0;
    sel = 0;
    h = 10; w = 40;
    x = (SCREEN_H/2)-(h/2);
    y = (SCREEN_W/2)-(w/2);

#ifdef HAVE_NCURSESW_NCURSES_H
    if(dir->iswcs)
    {
        if(!(cname = to_cstring(dir->name, NULL))) cname = dir->name;
        else
        {
            widechar_name = (wchar_t *)dir->name;
            widechar_namelen = dir->namelen;
        }
    }
    else
#endif
    cname = dir->name;

    //******read dir/file permissions ******//
    if(lstat(cname, &statbuf) == -1)
    {
        msgBoxH(strerror(errno), BUTTON_OK, ERROR);
        return;
    }

    UREAD    = !!(statbuf.st_mode & S_IRUSR);
    UWRITE   = !!(statbuf.st_mode & S_IWUSR);
    UEXECUTE = !!(statbuf.st_mode & S_IXUSR);
    GREAD    = !!(statbuf.st_mode & S_IRGRP);
    GWRITE   = !!(statbuf.st_mode & S_IWGRP);
    GEXECUTE = !!(statbuf.st_mode & S_IXGRP);
    OREAD    = !!(statbuf.st_mode & S_IROTH);
    OWRITE   = !!(statbuf.st_mode & S_IWOTH);
    OEXECUTE = !!(statbuf.st_mode & S_IXOTH);

    //draw window
    setScreenColorsI(COLOR_WINDOW);
    drawBox(x, y, x+h, w+y, " Properties ", 1);

#ifdef HAVE_NCURSESW_NCURSES_H
    refreshPropertiesDialog(cname, widechar_name, widechar_namelen);
#else
    refreshPropertiesDialog(cname);
#endif

    /////////////////////////////////////
    //Loop for user input  
    /////////////////////////////////////
    while(!endme) 
    {
        c = getKey();

        switch(c) 
        {
            case(TAB_KEY):
                if(sel >= 0) sel++;
                if(sel > 11) sel = 0;
                break;

            case('p'):
                if(GNU_DOS_LEVEL > 1 && CTRL) goto do_up;
                break;

            case(UP_KEY):
                if(GNU_DOS_LEVEL > 1) break;
do_up:
                if(sel == 0) sel = 10;
                else if(sel >= 10) sel = 7;
                else if(sel >= 1 && sel <= 3) sel = 0;
                else if(sel >= 4 && sel <= 9) sel -= 3;
                break;

            case('n'):
                if(GNU_DOS_LEVEL > 1 && CTRL) goto do_down;
                break;

            case(DOWN_KEY):
                if(GNU_DOS_LEVEL > 1) break;
do_down:
                if(sel == 0) sel = 1;
                else if(sel >= 10) sel = 0;
                else if(sel >= 7 && sel <= 9) sel = 10;
                else if(sel >= 1 && sel <= 6) sel += 3;
                break;

            case('f'):
                if(GNU_DOS_LEVEL > 1 && CTRL) goto do_right;
                break;

            case(RIGHT_KEY):
                if(GNU_DOS_LEVEL > 1) break;
do_right:
                if(sel > 0) sel++;
                if(sel > 11) sel = 0;
                break;

            case('b'):
                if(GNU_DOS_LEVEL > 1 && CTRL) goto do_left;
                break;

            case(LEFT_KEY):
                if(GNU_DOS_LEVEL > 1) break;
do_left:
                if(sel > 0) sel--;
                break;

            case('g'):
                if(GNU_DOS_LEVEL < 3) break;
                if(!CTRL) break;
                goto do_esc;

            case(ESC_KEY):
                if(GNU_DOS_LEVEL > 2) break;
do_esc:
                endme = 1;
                break;

            case(SPACE_KEY):
            case(ENTER_KEY):
                switch(sel)
                {
                    case 0:
                        tmp = inputBoxI("Enter new name: ", 
                                            cname, " Rename... " );

                        if(!tmp) break;

                        int len = strlen(tmp);

                        if(len)
                        {
                            if(cnewname) free(cnewname);
                            cnewname = malloc(len+1);

                            if(!cnewname)
                            {
                                msgBoxH("Insufficient memory!", BUTTON_OK, ERROR);
                                return;
                            }

                            strcpy(cnewname, tmp);

#ifdef HAVE_NCURSESW_NCURSES_H
                            widechar_newname = to_widechar(cnewname, 
                                                    &widechar_newnamelen);
#endif

                            NAME_CHANGED = 1;
                        }

                        setScreenColorsI(COLOR_WINDOW);
                        drawBox(x, y, x+h, w+y, " Properties ", 1);
                        break;

                    case 1: UREAD = !UREAD; break;
                    case 2: GREAD = !GREAD; break;
                    case 3: OREAD = !OREAD; break;
                    case 4: UWRITE = !UWRITE; break;
                    case 5: GWRITE = !GWRITE; break;
                    case 6: OWRITE = !OWRITE; break;
                    case 7: UEXECUTE = !UEXECUTE; break;
                    case 8: GEXECUTE = !GEXECUTE; break;
                    case 9: OEXECUTE = !OEXECUTE; break;

                    case 11:
                        endme = 1;
                        break;

                    case 10:
                        mode = 00;
                        if(UREAD   ) mode += 0400;
                        if(UWRITE  ) mode += 0200;
                        if(UEXECUTE) mode += 0100;
                        if(GREAD   ) mode += 040;
                        if(GWRITE  ) mode += 020;
                        if(GEXECUTE) mode += 010;
                        if(OREAD   ) mode += 04;
                        if(OWRITE  ) mode += 02;
                        if(OEXECUTE) mode += 01;

                        if(chmod(cname, mode) != 0) 
                        {
                            msgBoxH(strerror(errno), BUTTON_OK, ERROR);
                        }

                        if(NAME_CHANGED) 
                        {
                            if(rename(cname, cnewname) != 0)
                                msgBoxH(strerror(errno), BUTTON_OK, ERROR);
                        }
 
                        endme = 1;
                        break;
                }
                break;
        }

#ifdef HAVE_NCURSESW_NCURSES_H
        if(cnewname) 
             refreshPropertiesDialog(cnewname, widechar_newname, 
                                                        widechar_newnamelen);
        else refreshPropertiesDialog(cname, widechar_name, widechar_namelen);
#else
        if(cnewname) refreshPropertiesDialog(cnewname);
        else refreshPropertiesDialog(cname);
#endif
    }

    if(cnewname) free(cnewname);

#ifdef HAVE_NCURSESW_NCURSES_H
    if(widechar_newname) free(widechar_newname);
#endif

    hideCursor();
}

