/* 
 *    Programmed By: Mohammed Isam Mohammed [mohammed_isam1984@yahoo.com]
 *    Copyright 2014, 2015, 2016, 2017, 2018, 2024 (c)
 * 
 *    file: options.c
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
 *    along with mino.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "config.h"
#define _GNU_SOURCE
#define _XOPEN_SOURCE_EXTENDED
#ifdef HAVE_NCURSESW_NCURSES_H
# include <ncursesw/ncurses.h>
#else
# include <ncurses.h>
#endif
#include <pwd.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include "defs.h"
#include "options.h"
#include "kbd.h"
#include "dialogs.h"

static void refreshChangeColorsDialog(void);
static char *findColorName(int c);
static void saveOldColors(void);
static void resetColors(void);
static int showColorChooserDialog(void);

#define SET_COLOR_WINDOW()                          \
    setScreenColors(FG_COLOR[COLOR_WINDOW], BG_COLOR[COLOR_WINDOW]);

#define SET_COLOR_HIGHLIGHT()                       \
    setScreenColors(FG_COLOR[COLOR_HIGHLIGHT_TEXT], \
                    BG_COLOR[COLOR_HIGHLIGHT_TEXT]);

int GNU_DOS_LEVEL;

static int x, y, w, h, sel;
int oldFGColors[6];
int oldBGColors[6];

//if TRUE (1), lines are wrapped, ie will not go on beyond screen limits
int WRAP_LINES;

//number of spaces in a TAB
int TAB_CHARS;

int old_window_color;

//determine if automatic highlighting is on/off
int AUTO_HIGHLIGHTING;
hmode HIGHLIGHT_MODE;

//switch auto-indentation on/off
int  AUTO_INDENT;

//the string holding the spaces/tabes to indent new lines
wchar_t *autoIndentStr;

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
        case(30):case(40): return "BLACK  ";
        case(31):case(41): return "RED    ";
        case(32):case(42): return "GREEN  ";
        case(33):case(43): return "BROWN  ";
        case(34):case(44): return "BLUE   ";
        case(35):case(45): return "MAGENTA";
        case(36):case(46): return "CYAN   ";
        case(37):case(47): return "WHITE  ";
        case(49): return "BLACK  ";
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

static void resetColors(void) 
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
    int c;

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
                if(GNU_DOS_LEVEL > 1 && CTRL) goto do_up;
                break;

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
                if(GNU_DOS_LEVEL > 1 && CTRL) goto do_down;
                break;

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
                if(i == 38) { i = 0; return 0; }
                endme = 1;
                return i;

                case(0):
                    continue;

                case('g'):
                    if(GNU_DOS_LEVEL > 2 && CTRL)
                    {   // ESC - GNU keu binding
                        return 0;
                    }
                    break;

                case(ESC_KEY):
                    if(GNU_DOS_LEVEL > 2) break;

                default:
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
        mvprintw(x+i+1, y, "%s  %s  %s",
                 changeColorsDialogOptions[i],
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
    sel = 0;//selected item: 0-5 FG_COLORS, 6-11 BG_COLORS,
            //12 OK, 13 RESET

    saveOldColors();
    refreshChangeColorsDialog();

    //infinite loop to get user input
    while(!endme) 
    {
        int c = getKey();

        switch(c)
        {
            case(0):
                break;

            case('p'):
                if(GNU_DOS_LEVEL > 1 && CTRL) goto do_up;
                break;

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
                if(GNU_DOS_LEVEL > 1 && CTRL) goto do_down;
                break;

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
                if(GNU_DOS_LEVEL > 1 && CTRL) goto do_left_right;
                break;

            case(LEFT_KEY):
            case(RIGHT_KEY):
                if(GNU_DOS_LEVEL > 1) break;
do_left_right:
                if(sel == 12) sel = 13;
                else if(sel == 13) sel = 12;
                else if(sel >= 0 && sel <= 5) sel += 6;
                else sel -= 6;
                refreshChangeColorsDialog();
                enter_error = 0;
                break;

            case('g'):
                if(GNU_DOS_LEVEL > 2 && CTRL) goto do_esc;
                break;

            case(ESC_KEY):
                if(GNU_DOS_LEVEL > 2) break;
do_esc:
                resetColors();
                //write_config_file();
                refreshView();
                endme = 1;
                return;

            case(SPACE_KEY):
            case(ENTER_KEY):
                if(enter_error) continue;
                if(sel == 12) 
                {
                    write_config_file();
                    refreshView();
                    endme = 1;
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
                }//end if
                break;
        }//end switch
    }//end while
}

/****************************************
 * Takes new definition for number of
 * spaces in a tab character and writes
 * configuration into ~/.mino.conf file.
 * **************************************/
void optionsMenu_Tab_Spaces(void)
{
    char *t = inputBox("Enter number of spaces in a tab:", " Tab spaces ");

    if(t)
    {
        int i = atoi(t);
        if(i <= 0 || i >= 16) return;
        TAB_CHARS = i;
        write_config_file();
    }
}

void optionsMenu_Autoindent(void)
{
    if(AUTO_INDENT)
    {
        AUTO_INDENT = 0;
        optionsMenu[2][14] = ' ';
    }
    else
    {
        AUTO_INDENT = 1;
        optionsMenu[2][14] = '*';
    }
}

/******************************************
 * Resets configuration file ~/.mino.conf
 * to default values.
 * ****************************************/
void optionsMenu_Reset_Config(void)
{
    struct passwd *pass;
    char *config_file_name;
    FILE *config_file;

    if(!(pass = getpwuid(geteuid()))) 
    {
        msgBox("Couldn't open home directory to write config file.", 
                                                        BUTTON_OK, ERROR);
        refreshView();
        return;
    }

    config_file_name = malloc(strlen(pass->pw_dir)+11);

    if(!config_file_name)
    {
        msgBox("Insufficient memory", BUTTON_OK, ERROR);
        refreshView();
        return;
    }

    strcpy(config_file_name, pass->pw_dir);
    strcat(config_file_name, "/");
    strcat(config_file_name, ".mino.conf");

    if(!(config_file = fopen(config_file_name, "w"))) 
    {
        msgBox("Couldn't write to config file in home directory.", 
                                                        BUTTON_OK, ERROR);
        refreshView();
        return;
    }

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

    _write_config_file(config_file, 1);
    fclose(config_file);
    msgBox("Finished writing default values to ~/.mino.conf", BUTTON_OK, INFO);
    free(config_file_name);
    refreshView();
}

void write_config_file(void)
{
    struct passwd *pass;
    char *config_file_name;
    FILE *config_file;

    if(!(pass = getpwuid(geteuid()))) 
    {
        msgBox("Couldn't open home directory to write config file.", 
                                                        BUTTON_OK, ERROR);
        refreshView();
        return;
    }

    config_file_name = malloc(strlen(pass->pw_dir)+11);

    if(!config_file_name)
    {
        msgBox("Insufficient memory", BUTTON_OK, ERROR);
        refreshView();
        return;
    }

    strcpy(config_file_name, pass->pw_dir);
    strcat(config_file_name, "/");
    strcat(config_file_name, ".mino.conf");

    if(!(config_file = fopen(config_file_name, "w"))) 
    {
        msgBox("Couldn't write to config file in home directory.", 
                                                        BUTTON_OK, ERROR);
        refreshView();
        return;
    }

    _write_config_file(config_file, 0);
    fclose(config_file);
    free(config_file_name);
    refreshView();
}

void _write_config_file(FILE *file, int write_defaults)
{
    //write default values to the configuration file
    fprintf(file, "#Configuration file for the mino program\n");
    fprintf(file, "#Please do not modify this file by hand\n");
    fprintf(file, "#Use the Options menu in mino to change configuration.\n\n");
    //fprintf(file, "#Maximum number of dirs that can be handled in dialog boxes\n");
    //fprintf(file, "MAXDIRS = 255\n");
    //fprintf(file, "#Maximum number of files that can be handled in dialog boxes\n");
    //fprintf(file, "MAXFILES = 255\n");
    //fprintf(file, "#Maximum number of permitted lines in each document\n");
    //fprintf(file, "MAXLINES = 100\n\n");
    fprintf(file, "#Line Wrapping\n");
    fprintf(file, "#If TRUE, lines cannot be longer than the screen width\n");
    fprintf(file, "#If FALSE, lines cannot be upto MAX_CHARS_PER_LINE characters\n");
    fprintf(file, "#Note this value is omitted if WRAP_LINES is TRUE\n");
    fprintf(file, "MAX_CHARS_PER_LINE = 100\n");
    fprintf(file, "WRAP_LINES = TRUE\n\n");
    fprintf(file, "#Default title for newly opened documents, no quotes\n");
    fprintf(file, "DEFAULT_TITLE = untitled\n\n");
    fprintf(file, "#Start mino with CAPS and INSERT set to OFF/ON\n");
    fprintf(file, "CAPS = OFF\n");
    fprintf(file, "INSERT = OFF\n\n");
    fprintf(file, "#Number of spaces to insert when user presses TAB key\n");

    if(write_defaults)
    {
        fprintf(file, "TAB_CHARS = 8\n\n");
        fprintf(file, "#Display colors\n");
        fprintf(file, "FG_COLOR_WIN = 37\n");
        fprintf(file, "FG_COLOR_HLT = 34\n");
        fprintf(file, "FG_COLOR_MBAR = 34\n");
        fprintf(file, "FG_COLOR_SBAR = 34\n");
        fprintf(file, "FG_COLOR_HBUT = 32\n");
        fprintf(file, "FG_COLOR_BUT = 37\n");
        fprintf(file, "BG_COLOR_WIN = 44\n");
        fprintf(file, "BG_COLOR_HLT = 47\n");
        fprintf(file, "BG_COLOR_MBAR = 47\n");
        fprintf(file, "BG_COLOR_SBAR = 47\n");
        fprintf(file, "BG_COLOR_HBUT = 41\n");
        fprintf(file, "BG_COLOR_BUT = 41\n\n");
        fprintf(file, "#Show README on startup\n");
        fprintf(file, "SHOW_README\n\n");
        fprintf(file, "#GnuDOS Level\n");
        fprintf(file, "GNU_DOS_LEVEL = 1\n\n");
        fprintf(file, "#Auto-indentation\n");
        fprintf(file, "AUTO_INDENT = 1\n");
    }
    else
    {
        fprintf(file, "TAB_CHARS = %d\n\n", TAB_CHARS);
        fprintf(file, "#Display colors\n");
        fprintf(file, "FG_COLOR_WIN = %d\n", FG_COLOR[COLOR_WINDOW]);
        fprintf(file, "FG_COLOR_HLT = %d\n", FG_COLOR[COLOR_HIGHLIGHT_TEXT]);
        fprintf(file, "FG_COLOR_MBAR = %d\n", FG_COLOR[COLOR_MENU_BAR]);
        fprintf(file, "FG_COLOR_SBAR = %d\n", FG_COLOR[COLOR_STATUS_BAR]);
        fprintf(file, "FG_COLOR_HBUT = %d\n", FG_COLOR[COLOR_HBUTTONS]);
        fprintf(file, "FG_COLOR_BUT = %d\n", FG_COLOR[COLOR_BUTTONS]);
        fprintf(file, "BG_COLOR_WIN = %d\n", BG_COLOR[COLOR_WINDOW]);
        fprintf(file, "BG_COLOR_HLT = %d\n", BG_COLOR[COLOR_HIGHLIGHT_TEXT]);
        fprintf(file, "BG_COLOR_MBAR = %d\n", BG_COLOR[COLOR_MENU_BAR]);
        fprintf(file, "BG_COLOR_SBAR = %d\n", BG_COLOR[COLOR_STATUS_BAR]);
        fprintf(file, "BG_COLOR_HBUT = %d\n", BG_COLOR[COLOR_HBUTTONS]);
        fprintf(file, "BG_COLOR_BUT = %d\n", BG_COLOR[COLOR_BUTTONS]);

        if(SHOW_README) 
        {
            fprintf(file, "#Show README on startup\n");
            fprintf(file, "SHOW_README\n");
        }

        fprintf(file, "\n#GnuDOS Level\n");
        fprintf(file, "GNU_DOS_LEVEL = %d\n\n", GNU_DOS_LEVEL);
        fprintf(file, "#Auto-indentation\n");
        fprintf(file, "AUTO_INDENT = %d\n", AUTO_INDENT);
    }
}

