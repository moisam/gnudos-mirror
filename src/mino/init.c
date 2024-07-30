/* 
 *    Programmed By: Mohammed Isam Mohammed [mohammed_isam1984@yahoo.com]
 *    Copyright 2014, 2015, 2016, 2017, 2018, 2024 (c)
 * 
 *    file: init.c
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
#include <stdio.h>
#include <getopt.h>
#include <signal.h>
#include <asm/types.h>
#include "defs.h"
#include "kbd.h"
#include "edit.h"
#include "options.h"
#include "help.h"
#include "dialogs.h"

int     NEW_GNU_DOS_LEVEL;
char   *STARTUP_FILE_NAME;
char   *mino_ver = "2.1";
char   *copyrightNotice =
    "Mino for GNU/Linux is developed by Mohammed Isam\n"
    "Copyright (C) Mohammed Isam 2014, 2015, 2016, 2017, 2018, 2024\n"
    "This is a GNU software, part of the GnuDOS package\n";

//whether to show README file on startup
int  SHOW_README;

wchar_t *DEFAULT_TITLE;
int  open_file_at_startup;


static inline void outOfMemory(void)
{
    fprintf(stderr, "Fatal error: Insufficient memory\n");
    exit(1);
}

static void catchSignals(void) 
{
    if(signal(SIGINT, sighandler) == SIG_ERR) 
    {
        printf("Error interrupting SIGINT.\n");
        exit(1);
    }

    if(signal(SIGQUIT, sighandler) == SIG_ERR) 
    {
        printf("Error interrupting SIGQUIT.\n");
        exit(1);
    }

    if(signal(SIGABRT, sighandler) == SIG_ERR) 
    {
        printf("Error interrupting SIGABRT.\n");
        exit(1);
    }

    if(signal(SIGTERM, sighandler) == SIG_ERR) 
    {
        printf("Error interrupting SIGTERM.\n");
        exit(1);
    }

    if(signal(SIGTSTP, sighandler) == SIG_ERR) 
    {
      //exit(1);
    }
}

void parseLineArgs(int argc, char **argv) 
{
    struct  passwd *pass;
    char *config_file_name;
    FILE *config_file;

    GNU_DOS_LEVEL = 1;
    NEW_FILE = 1;
    FILE_STATE = NEW;
    open_file_at_startup = 0;
    AUTO_HIGHLIGHTING = 0;
    NEW_GNU_DOS_LEVEL = 0;

    ///////////////////////////////////////
    //parse command line arguments
    ///////////////////////////////////////
    int c;
    static struct option long_options[] =
    {
        {"reset-config", no_argument,            0,  'r'},
        {"help",         no_argument,            0,  'h'},
        {"level",  required_argument,            0,  'l'},
        {"version",      no_argument,            0,  'v'},
        {0, 0, 0, 0}
    };

    while(1)
    {
        int option_index = 0;
        c = getopt_long(argc, argv, "rhl:v", long_options, &option_index);
        if(c==-1) break;    //end of options

        switch(c)
        {
            case 0:
                break;

            case 'r':    //reset config file
                if(!(pass = getpwuid(geteuid()))) 
                {
                    printf("Error: couldn't open home directory to write "
                           "configuration file.\n");
                    printf("Aborting.\n");
                    exit(1);
                }

                config_file_name = malloc(strlen(pass->pw_dir)+12);

                if(!config_file_name)
                {
                    fprintf(stderr, "Insufficient memory\n");
                    exit(1);
                }

                strcpy(config_file_name, pass->pw_dir);
                strcat(config_file_name, "/");
                strcat(config_file_name, ".mino.conf");

                if(!(config_file = fopen(config_file_name, "w"))) 
                {
                    printf("Error: couldn't write to configuration file in "
                           "your home directory.\n");
                    printf("Aborting.\n");
                    exit(1);
                }

                printf("Resetting program configuration..\n");
                _write_config_file(config_file, 1);
                fclose(config_file);
                printf("Finished writing default values to ~/.mino.conf\n");
                free(config_file_name);
                exit(0);
                break;

            case 'h':    //show program help
                printf("\nCommand line help for mino\n");
                printf("%s\n", copyrightNotice);
                printf("Usage: %s [filename][options]\n\n", argv[0]);
                printf("Options:\n");
                printf("\tfilename: File to be opened by mino (optional)\n");
                printf("\t--reset-config, -r\tReset program configuration. "
                        "Writes default\n");
                printf("\t                  \tvalues to .mino.conf file\n");
                printf("\t--help, -h        \tShow this command line help\n");
                printf("\t--version, -v     \tShow program version\n");
                printf("\t--levelX, -lX     \tSet the experience level, "
                                             "where X is 1-6\n");
                printf("For more information, see 'info mino' or the README"
                        " file in mino help menu\n");
                exit(0);
                break;

            case 'v':    //show program version
                printf("%s\n", mino_ver);
                exit(0);
                break;

            case 'l':    //set GNU_DOS level
                ;
                int i = atoi(optarg);
                if(i < 1 || i > 6)
                {
                    printf("Unrecognised level. See 'man mino' or "
                            "'info mino' for information about possible"
                            " levels.\n");
                    exit(1); break;
                }
                NEW_GNU_DOS_LEVEL = i;
                break;

            case '?':
                break;

            default:
                abort();
        }
    }

    ///////////////////////////////////////
    //parse the remaining arguments
    ///////////////////////////////////////
    STARTUP_FILE_NAME = NULL;

    if(optind < argc)
    {
        NEW_FILE = 0;
        FILE_STATE = OPENED;
        open_file_at_startup = 1;
        STARTUP_FILE_NAME = argv[optind];
    }
}

void init_error(void) 
{
    printf("Error: couldn't open home directory and read configuration file.\n");
    printf("Will use built-in defaults. This means if you have set any\n");
    printf("preferences (for example, display colors), they will be of\n");
    printf("no effect.\n");
    printf("You can reset your config file to program defaults by invoking:\n");
    printf("mino --reset-config\n\nPress any key to continue..\n");
    getchar();
}

int __initNewDocument(void)
{
    if(totalLines)
    {
        int i;
        for(i = 0; i < totalLines; i++)
        {
            freeLineStruct(lines[i]);
            lines[i] = NULL;
        }

        totalLines = 0;
    }

    lines[0] = allocLineStructB(maxLen);
    if(!lines[0]) return 0;
    totalLines = 1;
    initEdit();
    return 1;
}

int initNewDocument(void)
{
    if(documentTitle) free(documentTitle);
    documentTitle = wcsdup(DEFAULT_TITLE);
    if(!documentTitle) return 0;
    return __initNewDocument();
}

void resetLineCounters(void)
{
    selectedLine           = 0;
    selectedChar           = 0;
    selectedCharCarry      = 0;
    userVisibleCurLine     = 0;
    selectedCharIfPossible = 0;
    totalVisLines          = SCREEN_H-4;
    firstVisLine           = 0;
}

void __init()
{
    if(!initTerminal()) 
    {
        fprintf(stderr, "Fatal error: Failed to initialize the terminal.\r\n"
                        "Aborting.\n");
        exit(1);
    }

    getScreenSize();

    if(!SCREEN_H || !SCREEN_W)
    {
        fprintf(stderr, "Error: Failed to get terminal size\n");
        exit(1);
    }

    //1. Main Menu//
    menu[0] = "&File";
    menu[1] = "&Edit";
    menu[2] = "&Options";
    menu[3] = "&Help";
    //2. File Menu//
    fileMenu[0] = "New..         ^N";
    fileMenu[1] = "Open file     ^O";
    fileMenu[2] = "Save file     ^S";
    fileMenu[3] = "Save as..       ";
    fileMenu[4] = "Print         ^P";
    fileMenu[5] = "Exit          ^Q";
    //3. Edit Menu//
    editMenu[0] = "Cut                ^X";
    editMenu[1] = "Copy               ^C";
    editMenu[2] = "Paste              ^V";
    editMenu[3] = "Select all         ^A";
    editMenu[4] = "Undo               ^Z";
    editMenu[5] = "Redo               ^Y";
    editMenu[6] = "Delete Line        ^D";
    editMenu[7] = "Find..             ^F";
    editMenu[8] = "Replace..          ^R";
    editMenu[9] = "Toggle select mode ^E";
    //4. Options Menu//
    optionsMenu[0] = "Change colors  ";
    optionsMenu[1] = "Tab spaces     ";
    optionsMenu[2] = "Autoindent     ";
    optionsMenu[3] = "Reset config   ";
    //5. Help Menu//
    helpMenu[0] = "View README    ";
    helpMenu[1] = "GNU Keybindings";
    helpMenu[2] = "Quick reference";
    helpMenu[3] = "About Mino..   ";
    
    AUTO_INDENT = 1;
    autoIndentStr = malloc(512 * sizeof(wchar_t));
    if(!autoIndentStr) outOfMemory();
    MAX_MSG_BOX_W      = SCREEN_W - 4;
    MAX_MSG_BOX_H      = SCREEN_H - 4;
    MAX_CHARS_PER_LINE = SCREEN_W - 2;
    maxLen             = MAX_CHARS_PER_LINE;
    CAPS               = 0;
    INSERT             = 0;
    SELECTING          = 0;
    CLIPBOARD_IS_EMPTY = 1;
    resetLineCounters();

    //initialize color arrays
    FG_COLOR[COLOR_WINDOW]         = 37;
    FG_COLOR[COLOR_HIGHLIGHT_TEXT] = 34;
    FG_COLOR[COLOR_MENU_BAR]       = 34;
    FG_COLOR[COLOR_STATUS_BAR]     = 34;
    FG_COLOR[COLOR_BUTTONS]        = 37;
    FG_COLOR[COLOR_HBUTTONS]       = 32;
    BG_COLOR[COLOR_WINDOW]         = 44;
    BG_COLOR[COLOR_HIGHLIGHT_TEXT] = 47;
    BG_COLOR[COLOR_MENU_BAR]       = 47;
    BG_COLOR[COLOR_STATUS_BAR]     = 47;
    BG_COLOR[COLOR_BUTTONS]        = 41;
    BG_COLOR[COLOR_HBUTTONS]       = 41;
    old_window_color = BG_COLOR[COLOR_WINDOW];

    GNU_DOS_LEVEL = 1;
    WRAP_LINES    = 1;//TRUE -- restric line width to (screen width-2)
    TAB_CHARS     = 8;    //define 8 spaces in a tab
    SHOW_README   = 0;
}

static void post_config_file(char *open_file_name)
{
    if(AUTO_INDENT) optionsMenu[2] = strdup("Autoindent    *");
    else optionsMenu[2] = strdup("Autoindent     ");
 
    if(NEW_GNU_DOS_LEVEL) GNU_DOS_LEVEL = NEW_GNU_DOS_LEVEL;
    if(GNU_DOS_LEVEL > 5 || GNU_DOS_LEVEL < 1) GNU_DOS_LEVEL = 1;
 
    //set defaults for Dir view//
    initDirView();
    totalLines = 0;

    if(NEW_FILE) 
    {
        if(!initNewDocument())
        {
            fprintf(stderr, "Fatal error: Insufficient memory\n");
            exit(1);
        }
    }
    else
    {
        char *slash = strrchr(open_file_name, '/');

        if(slash) documentTitle = to_widechar(slash+1, NULL);
        else      documentTitle = to_widechar(open_file_name, NULL);

        if(!documentTitle)
        {
            fprintf(stderr, "Fatal error: Insufficient memory\n");
            exit(1);
        }
    }
 
    catchSignals();
}

/***************************************
 * init(): 
 * Procedure containing code that init-
 * ializes various data structures.
 * **************************************/
void init(char *open_file_name)
{
    struct  passwd *pass;
    char *config_file_name;
    FILE *config_file;

    DEFAULT_TITLE = to_widechar(__DEFAULT_TITLE, NULL);
    __init();
  
    if(!(pass = getpwuid(geteuid()))) 
    {
        init_error();
        post_config_file(open_file_name);
        return;
    }

    config_file_name = malloc(strlen(pass->pw_dir)+12);
    if(!config_file_name) outOfMemory();
    strcpy(config_file_name, pass->pw_dir);
    strcat(config_file_name, "/");
    strcat(config_file_name, ".mino.conf");

    if((config_file = fopen(config_file_name, "r"))) 
    {
        char buf[100];
        //read configuration file
        while(fgets(buf, sizeof(buf), config_file)) 
        {
            if(buf[0] == '#' || buf[0] == '\n') continue;
            if (strstr(buf, "GNU_DOS_LEVEL")) 
                GNU_DOS_LEVEL = atoi((strchr(buf, '=')+2));
            else if (strstr(buf, "TAB_CHARS")) 
            {
                TAB_CHARS = atoi(strchr(buf, '=')+2);
                if(TAB_CHARS < 1 || TAB_CHARS > SCREEN_W-3) TAB_CHARS = 8;
            }
            else if (strstr(buf, "DEFAULT_TITLE")) 
            {
                char *title = strchr(buf, '=')+2;
                size_t titlelen = strlen(title);
                DEFAULT_TITLE = to_widechar(title, NULL);

                if(!DEFAULT_TITLE)
                {
                    fprintf(stderr, "Fatal error: Insufficient memory\n");
                    exit(1);
                }

                if(DEFAULT_TITLE[titlelen-1] == L'\n')
                    DEFAULT_TITLE[titlelen-1] = L'\0';
            }
            else if (strstr(buf, "WRAP_LINES")) 
            {
                if(strstr(buf, "TRUE")) WRAP_LINES = 1;
                else WRAP_LINES = 0;
            }
            else if (strstr(buf, "CAPS")) 
            {
                if(strstr(buf, "OFF")) CAPS = 0;
                else CAPS = 1;
            }
            else if (strstr(buf, "INSERT")) 
            {
                if(strstr(buf, "OFF")) INSERT = 0;
                else INSERT = 1;
            }
            else if (strstr(buf, "FG_COLOR_WIN")) 
                FG_COLOR[COLOR_WINDOW] = atoi(strchr(buf, '=')+2);
            else if (strstr(buf, "FG_COLOR_HLT")) 
                FG_COLOR[COLOR_HIGHLIGHT_TEXT] = atoi(strchr(buf, '=')+2);
            else if (strstr(buf, "FG_COLOR_MBAR")) 
                FG_COLOR[COLOR_MENU_BAR] = atoi(strchr(buf, '=')+2);
            else if (strstr(buf, "FG_COLOR_SBAR")) 
                FG_COLOR[COLOR_STATUS_BAR] = atoi(strchr(buf, '=')+2);
            else if (strstr(buf, "FG_COLOR_HBUT")) 
                FG_COLOR[COLOR_HBUTTONS] = atoi(strchr(buf, '=')+2);
            else if (strstr(buf, "FG_COLOR_BUT")) 
                FG_COLOR[COLOR_BUTTONS] = atoi(strchr(buf, '=')+2);
            else if (strstr(buf, "BG_COLOR_WIN"))
            {
                BG_COLOR[COLOR_WINDOW] = atoi(strchr(buf, '=')+2);
                old_window_color = BG_COLOR[COLOR_WINDOW];
            }
            else if (strstr(buf, "BG_COLOR_HLT")) 
                BG_COLOR[COLOR_HIGHLIGHT_TEXT] = atoi(strchr(buf, '=')+2);
            else if (strstr(buf, "BG_COLOR_MBAR")) 
                BG_COLOR[COLOR_MENU_BAR] = atoi(strchr(buf, '=')+2);
            else if (strstr(buf, "BG_COLOR_SBAR")) 
                BG_COLOR[COLOR_STATUS_BAR] = atoi(strchr(buf, '=')+2);
            else if (strstr(buf, "BG_COLOR_HBUT")) 
                BG_COLOR[COLOR_HBUTTONS] = atoi(strchr(buf, '=')+2);
            else if (strstr(buf, "BG_COLOR_BUT")) 
                BG_COLOR[COLOR_BUTTONS] = atoi(strchr(buf, '=')+2);
            else if (strstr(buf, "SHOW_README")) 
                SHOW_README = 1;
            else if (strstr(buf, "AUTO_INDENT")) 
                AUTO_INDENT = atoi(strchr(buf, '=')+2);
        }

        fclose(config_file);
    }
    else write_config_file();

    free(config_file_name); 
    post_config_file(open_file_name);
}

void showREADMEOnStartup(void)
{
    int x = SCREEN_H/2-3;
    int y = SCREEN_W/2-20;
    int w = y+40;
    int h = x+6;
  
    setScreenColorsI(COLOR_WINDOW);
    drawBox(x, y, h, w, " Welcome to mino ", 1);
    mvprintw(x, y+1, "Welcome to mino!");
    mvprintw(x+1, y+1, "README file will be shown to help you");
    mvprintw(x+2, y+1, "start using mino.");
    setScreenColorsI(COLOR_HBUTTONS);
    mvprintw(x+4, y+1, "  OK  ");
    setScreenColorsI(COLOR_BUTTONS);
    mvprintw(x+4, y+11, " Don't show README again ");
    move(x+4, y+1);
    refresh();

    int sel = 0;
    char *c;

    while((c = ugetKey())) 
    {
        switch(c[0]) 
        {
            case(RIGHT_KEY):
            case(LEFT_KEY):
            case(TAB_KEY):
                if(sel == 0) 
                {
                    setScreenColors(FG_COLOR[COLOR_BUTTONS], BG_COLOR[COLOR_BUTTONS]);
                    mvprintw(x+4, y+1, "  OK  ");
                    setScreenColors(FG_COLOR[COLOR_HBUTTONS], BG_COLOR[COLOR_HBUTTONS]);
                    mvprintw(x+4, y+11, " Don't show README again ");
                    move(x+4, y+11);
                    sel = 1;
                } 
                else 
                {
                    setScreenColors(FG_COLOR[COLOR_HBUTTONS], BG_COLOR[COLOR_HBUTTONS]);
                    mvprintw(x+4, y+1, "  OK  ");
                    setScreenColors(FG_COLOR[COLOR_BUTTONS], BG_COLOR[COLOR_BUTTONS]);
                    mvprintw(x+4, y+11, " Don't show README again ");
                    move(x+4, y+1);
                    sel = 0;
                } break;

            case(ENTER_KEY):
            case(SPACE_KEY):
                if(sel == 0) SHOW_README = 1;
                else SHOW_README = 0;

                showReadMe();
                write_config_file();
                refreshView();
                return;
        }//end switch

        refresh();
    }//end while
}

