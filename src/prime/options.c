/* 
 *    Programmed By: Mohammed Isam Mohammed [mohammed_isam1984@yahoo.com]
 *    Copyright 2014, 2015, 2016, 2017, 2018, 2024 (c)
 * 
 *    file: options.c
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
#include "defs.h"
#include "options.h"
#include "kbd.h"
#include <ncurses.h>
#include <pwd.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#define SET_COLOR_HIGHLIGHT()                       \
    setScreenColors(FG_COLOR[COLOR_HIGHLIGHT_TEXT], \
                    BG_COLOR[COLOR_HIGHLIGHT_TEXT])

#define SET_COLOR_WINDOW()                          \
    setScreenColors(FG_COLOR[COLOR_WINDOW], BG_COLOR[COLOR_WINDOW])

static int x, y, w, h, sel;
int oldFGColors[6];
int oldBGColors[6];

int GNU_DOS_LEVEL;

char *changeColorsDialogOptions[6] =
{
    "Window          ",
    "Highlighted text",
    "Menu bar        ",
    "Status bar      ",
    "Buttons         ",
    "Selected Button "
};

static char *findColorName(int c) 
{
    switch(c) 
    {
        case(30):case(40): return "BLACK  "; break;
        case(31):case(41): return "RED    "; break;
        case(32):case(42): return "GREEN  "; break;
        case(33):case(43): return "BROWN  "; break;
        case(34):case(44): return "BLUE   "; break;
        case(35):case(45): return "MAGENTA"; break;
        case(36):case(46): return "CYAN   "; break;
        case(37):case(47): return "WHITE  "; break;
        case(49):          return "BLACK  "; break;
    }

    return NULL;
}

static void saveOldColors(void) 
{
    int i;

    for(i = 0; i < 6; i++) 
    {
        oldFGColors[i] = FG_COLOR[i];
        oldBGColors[i] = BG_COLOR[i];
    }
}

void resetColors(void)
{
    int i;

    for(i = 0; i < 6; i++) 
    {
        FG_COLOR[i] = oldFGColors[i];
        BG_COLOR[i] = oldBGColors[i];
    }
}

static int showColorChooserDialog(void)
{
    int i;
    int x1 = (SCREEN_H/2)-5;
    int y1 = (SCREEN_W/2)-10;
    int endme = 0;
    char c;

    drawBox(x1, y1, x1+10, y1+16, " Choose color ", 1);
    for(i = 30; i < 38; i++)
        mvprintw(x1++, y1, "%s", findColorName(i));

    mvprintw(x1++, y1, "CANCEL");

    x1 = (SCREEN_H/2)-4; i = 30;
    SET_COLOR_HIGHLIGHT();

    mvprintw(x1-1, y1, "%s", findColorName(i));
    refresh();

    while(!endme) 
    {
        c = getKey();

        switch(c) 
        {
            case('p'):
                if(GNU_DOS_LEVEL < 2) break;
                if(!CTRL) break;
                goto do_up;

            case(UP_KEY):
                if(GNU_DOS_LEVEL > 1) break;
do_up:
                if(i <= 30) 
                {
                    SET_COLOR_WINDOW();
                    mvprintw(x1-1, y1, "%s", findColorName(i));
                    i = 38;
                    SET_COLOR_HIGHLIGHT();
                    mvprintw(x1+i-31, y1, "CANCEL");
                }
                else if(i == 38) 
                {
                    SET_COLOR_WINDOW();
                    mvprintw(x1+i-31, y1, "CANCEL");
                    i = 37;
                    SET_COLOR_HIGHLIGHT();
                    mvprintw(x1+i-31, y1, "%s", findColorName(i));
                } 
                else 
                {
                    SET_COLOR_WINDOW();
                    mvprintw(x1+i-31, y1, "%s", findColorName(i));
                    i--;
                    SET_COLOR_HIGHLIGHT();
                    mvprintw(x1+i-31, y1, "%s", findColorName(i));
                }

                refresh();
                endme = 0;
                break;

            case('n'):
                if(GNU_DOS_LEVEL < 2) break;
                if(!CTRL) break;
                goto do_down;

            case(DOWN_KEY):
                if(GNU_DOS_LEVEL > 1) break;
do_down:
                if(i >= 38) 
                {
                    SET_COLOR_WINDOW();
                    mvprintw(x1+i-31, y1, "CANCEL");
                    i = 30;
                    SET_COLOR_HIGHLIGHT();
                    mvprintw(x1+i-31, y1, "%s", findColorName(i));
                } 
                else if(i == 37) 
                {
                    SET_COLOR_WINDOW();
                    mvprintw(x1+i-31, y1, "%s", findColorName(i));
                    i = 38;
                    SET_COLOR_HIGHLIGHT();
                    mvprintw(x1+i-31, y1, "CANCEL");
                } 
                else 
                {
                    SET_COLOR_WINDOW();
                    mvprintw(x1+i-31, y1, "%s", findColorName(i));
                    i++;
                    SET_COLOR_HIGHLIGHT();
                    mvprintw(x1+i-31, y1, "%s", findColorName(i));
                }

                endme = 0;
                refresh();
                break;

            case(ENTER_KEY):
            case(SPACE_KEY):
                if(i == 38) { i = 0; return 0; break; }
                endme = 1;
                return i;

            case(0):
                continue;

            case('g'):
                if(GNU_DOS_LEVEL < 3) break;
                if(!CTRL) break;
                goto do_esc;

            case(ESC_KEY):
            default:
                if(GNU_DOS_LEVEL > 2) break;
do_esc:
                endme = 1;
                return 0;
        }//end switch
    }//end while

    return i;
}

static void refreshChangeColorsDialog(void)
{
    int i;

    drawBox(x, y, h, w, "Change colors", 1);

    mvprintw(x, y+15, "Foreground  Background");
    mvprintw(h-2, y+11, "  OK   RESET");

    SET_COLOR_WINDOW();

    for(i = 0; i < color_components; i++) 
    {
        mvprintw(x+i+1, y, "%s  %s  %s", changeColorsDialogOptions[i],
                                         findColorName(FG_COLOR[i]),
                                         findColorName(BG_COLOR[i]));
    }

    SET_COLOR_HIGHLIGHT();

    if(sel == 12)
        mvprintw(h-2, y+11, "  OK  ");
    else if(sel == 13)
        mvprintw(h-2, y+18, "RESET");
    else if(sel >= 0 && sel <= 5)
        mvprintw(x+sel+1, y+18, "%s", findColorName(FG_COLOR[sel]));
    else if(sel >= 6 && sel <= 11)
        mvprintw(x+(sel-6)+1, y+27, "%s", findColorName(BG_COLOR[sel-6]));

    refresh();
}


void optionsMenu_Change_Colors(void)
{
    int endme = 0;
    int enter_error = 0;

    x = (SCREEN_H/2)-5;
    y = (SCREEN_W/2)-18;
    w = y+38;
    h = x+10;
    sel = 0;    //selected item: 0-5 FG_COLORS, 6-11 BG_COLORS,
                //               12 OK, 13 RESET

    saveOldColors();
    refreshChangeColorsDialog();

    //infinite loop to get user input
    while(!endme) 
    {
        char c = getKey();

        switch(c) 
        {
            case(0):
                break;

            case('p'):
                if(GNU_DOS_LEVEL < 2) break;
                if(!CTRL) break;
                goto do_up;

            case(UP_KEY):
                if(GNU_DOS_LEVEL > 1) break;
do_up:
                if(sel == 0 || sel == 6) sel = 12;
                else if(sel == 12) sel = 5;
                else if(sel == 13) sel = 11;
                else sel--;

                refreshChangeColorsDialog();
                enter_error = 0;
                break;

            case('n'):
                if(GNU_DOS_LEVEL < 2) break;
                if(!CTRL) break;
                goto do_down;

            case(DOWN_KEY):
                if(GNU_DOS_LEVEL > 1) break;
do_down:
                if(sel == 12) sel = 0;
                else if(sel == 13) sel = 6;
                else if(sel == 5 || sel == 6) sel = 12;
                else sel++;

                refreshChangeColorsDialog();
                enter_error = 0;
                break;

            case('b'):
            case('f'):
                if(GNU_DOS_LEVEL < 2) break;
                if(!CTRL) break;
                goto do_right_left;

            case(LEFT_KEY):
            case(RIGHT_KEY):
                if(GNU_DOS_LEVEL > 1) break;
do_right_left:
            if(sel == 12) sel = 13;
            else if(sel == 13) sel = 12;
            else if(sel >= 0 && sel <= 5) sel += 6;
            else sel -= 6;

            refreshChangeColorsDialog();
            enter_error = 0;
            break;

            case('g'):
                if(GNU_DOS_LEVEL < 3) break;
                if(!CTRL) break;
                goto do_esc;

            case(ESC_KEY):
                if(GNU_DOS_LEVEL > 2) break;
do_esc:
                resetColors();
                //write_config_file();
                refreshFileView();
                refreshDirView();
                refreshBottomView();
                return;

            case(SPACE_KEY):
            case(ENTER_KEY):
                if(enter_error) continue;
                if(sel == 12) 
                {
                    write_config_file();
                    refreshFileView();
                    refreshDirView();
                    refreshBottomView();
                    return;
                }
                else if(sel == 13) 
                {
                    resetColors();
                    refreshChangeColorsDialog();
                    //write_config_file();
                }
                else
                {
                    int tmp;

                    enter_error = !enter_error;
                    if(!enter_error) continue;

                    if(sel >= 0 && sel <= 5) 
                    {
                        tmp = showColorChooserDialog();
                        if(tmp) FG_COLOR[sel] = tmp;
                        refreshChangeColorsDialog();
                        break;
                    } 
                    else if(sel >= 6 && sel <= 11) 
                    {
                        tmp = showColorChooserDialog();
                        if(tmp) BG_COLOR[sel-6] = tmp+10;
                        refreshChangeColorsDialog();
                        break;
                    }
                }
                break;
        }//end switch
    }//end while
}

/******************************************
 * Resets configuration file ~/.prime.conf
 * to default values.
 * ****************************************/
void optionsMenu_Reset_Config(void)
{
    if(!write_config_file_defaults())
    {
        msgBoxH("Couldn't write to config file in home directory.", BUTTON_OK, ERROR);
    }
    else
    {
        msgBoxH("Finished writing default values to ~/.prime.conf", BUTTON_OK, INFO);
        loadDefaultColors();
    }

    refreshFileView();
    refreshDirView();
    refreshBottomView();
}

void write_config_file(void)
{
    struct passwd *pass;
    char *config_file_name;
    FILE *config_file;

    if(!(pass = getpwuid(geteuid()))) 
    {
        msgBoxH("Couldn't open home directory to write config file.", BUTTON_OK, ERROR);
        refreshFileView();
        refreshDirView();
        return;
    }

    config_file_name = malloc(strlen(pass->pw_dir)+13);

    if(!config_file_name)
    {
        msgBoxH("Insufficient memory", BUTTON_OK, ERROR);
        refreshFileView();
        refreshDirView();
        return;
    }

    strcpy(config_file_name, pass->pw_dir);
    strcat(config_file_name, "/");
    strcat(config_file_name, ".prime.conf");

    if(!(config_file = fopen(config_file_name, "w"))) 
    {
        msgBoxH("Couldn't write to config file in home directory.", BUTTON_OK, ERROR);
        refreshFileView();
        refreshDirView();
        free(config_file_name);
        return;
    }

    fprintf(config_file, "#Configuration file for the prime program\n");
    fprintf(config_file, "#Please do not modify this file by hand\n\n");
    fprintf(config_file, "#Display colors\n");
    fprintf(config_file, "FG_COLOR_WIN = %d\n", FG_COLOR[COLOR_WINDOW]);
    fprintf(config_file, "FG_COLOR_HLT = %d\n", FG_COLOR[COLOR_HIGHLIGHT_TEXT]);
    fprintf(config_file, "FG_COLOR_MBAR = %d\n", FG_COLOR[COLOR_MENU_BAR]);
    fprintf(config_file, "FG_COLOR_SBAR = %d\n", FG_COLOR[COLOR_STATUS_BAR]);
    fprintf(config_file, "FG_COLOR_HBUT = %d\n", FG_COLOR[COLOR_HBUTTONS]);
    fprintf(config_file, "FG_COLOR_BUT = %d\n", FG_COLOR[COLOR_BUTTONS]);
    fprintf(config_file, "BG_COLOR_WIN = %d\n", BG_COLOR[COLOR_WINDOW]);
    fprintf(config_file, "BG_COLOR_HLT = %d\n", BG_COLOR[COLOR_HIGHLIGHT_TEXT]);
    fprintf(config_file, "BG_COLOR_MBAR = %d\n", BG_COLOR[COLOR_MENU_BAR]);
    fprintf(config_file, "BG_COLOR_SBAR = %d\n", BG_COLOR[COLOR_STATUS_BAR]);
    fprintf(config_file, "BG_COLOR_HBUT = %d\n", BG_COLOR[COLOR_HBUTTONS]);
    fprintf(config_file, "BG_COLOR_BUT = %d\n\n", BG_COLOR[COLOR_BUTTONS]);
    fprintf(config_file, "#GnuDOS Level\n");
    fprintf(config_file, "GNU_DOS_LEVEL = %d\n", GNU_DOS_LEVEL);

    fclose(config_file);
    free(config_file_name);

    refreshFileView();
    refreshDirView();
}

void optionsMenu_Properties(void)
{
    showPropertiesDialog();
    SET_COLOR_WINDOW();
    scanDir(cwd);
}

