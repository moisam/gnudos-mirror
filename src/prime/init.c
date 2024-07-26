/* 
 *    Programmed By: Mohammed Isam Mohammed [mohammed_isam1984@yahoo.com]
 *    Copyright 2013, 2014, 2015, 2016, 2017, 2018, 2024 (c)
 * 
 *    file: init.c
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

#include <unistd.h>
#include <pwd.h>
#include "defs.h"
#include "cutcopy.h"
#include "find.h"
#include "options.h"
#include "dirstruct.h"

int NEW_GNU_DOS_LEVEL = 0;

int MAX_DIR_NAME_LEN;
int MAX_FILE_NAME_LEN;

/***************************************
 * init(): 
 * Procedure containing code that init-
 * ializes various data structures.
 * **************************************/
void init(void)
{
    int half_SCREEN_W;

    initTerminal();
    getScreenSize();

    if(!SCREEN_H || !SCREEN_W)
    {
        fprintf(stderr, "Error: Failed to get terminal size\n");
        exit(1);
    }

    //define colors that will be used to colorize the view
    FILE_DIR_COLOR['d'-'a'] = FG_COLOR[COLOR_WINDOW];    // directories
    FILE_DIR_COLOR['x'-'a'] = GREEN;    // exec files
    FILE_DIR_COLOR['r'-'a'] = FG_COLOR[COLOR_WINDOW];    // regular files
    FILE_DIR_COLOR['l'-'a'] = CYAN;     // links
    FILE_DIR_COLOR['a'-'a'] = RED;      // archives
    FILE_DIR_COLOR['h'-'a'] = BROWN;    // hidden files
    FILE_DIR_COLOR['p'-'a'] = MAGENTA;  // image files

    //1. Main Menu//
    menu[0] = "&File";
    menu[1] = "&Edit";
    menu[2] = "&Options";
    menu[3] = "&Help";

    //2. File Menu//
    fileMenu[0] = "New directory ^N";
    fileMenu[1] = "Open location ^O";
    fileMenu[2] = "Export tree   ^E";
    fileMenu[3] = "Print         ^P";
    fileMenu[4] = "Exit          ^Q";

    //3. Edit Menu//
    editMenu[0] = "Cut        ^X";
    editMenu[1] = "Copy       ^C";
    editMenu[2] = "Paste      ^V";
    editMenu[3] = "Mark all   ^A";
    editMenu[4] = "Unmark all ^Z";
    editMenu[5] = "Clear sel. ^W";
    editMenu[6] = "Find..     ^F";

    //4. Options Menu//
    optionsMenu[0] = "Properties     ";
    optionsMenu[1] = "Change colors  ";
    optionsMenu[2] = "Reset config   ";

    //5. Help Menu//
    helpMenu[0] = "View README    ";
    helpMenu[1] = "GNU keybindings";
    helpMenu[2] = "Quick reference";
    helpMenu[3] = "About Prime..  ";

    //set defaults for Dir view//
    numVisDirs  = SCREEN_H - 9;
    firstVisDir = 0;
    selectedDir = -1;
    totalDirs   = 0;

    half_SCREEN_W = SCREEN_W/2;

    //reserve enough space in memory for the highlight bar
    dirHighLight = malloc(half_SCREEN_W);
    if(!dirHighLight) goto memory_error;
    memset(dirHighLight, ' ', half_SCREEN_W-4);    //fill the bar with spaces
    dirHighLight[half_SCREEN_W-4] = '\0';

    //set defaults for File view//
    numVisFiles  = SCREEN_H - 9;
    firstVisFile = 0;
    selectedFile = -1;
    totalFiles   = 0;
    fileHighLight = malloc(half_SCREEN_W);
    if(!fileHighLight) goto memory_error;
    memset(fileHighLight, ' ', half_SCREEN_W-2);
    fileHighLight[half_SCREEN_W-2] = '\0';
 
    activeWindow = DIR_WIN;
    MAX_DIR_NAME_LEN = half_SCREEN_W-4;
    MAX_FILE_NAME_LEN = half_SCREEN_W-2; 
    
    memset(dirs, 0, sizeof(dirs));
    memset(files, 0, sizeof(files));

    numStarred   = 0;
    numCut       = 0;
    numCopy      = 0;
    numCutFiles  = 0;
    numCopyFiles = 0;
    numCutDirs   = 0;
    numCopyDirs  = 0;
    
    memset(cutFiles , 0, sizeof(cutFiles ));
    memset(copyFiles, 0, sizeof(copyFiles));
    memset(cutDirs  , 0, sizeof(cutDirs  ));
    memset(copyDirs , 0, sizeof(copyDirs ));

    //numPrinters = -1;
    //selectedPrinter = -1;
    //PRINTING = 0;

    return;
 
memory_error:

    fprintf(stderr, "Insufficient memory\n");
    exit(1);
}


int read_config_file(void)
{
    struct passwd *pass;
    char *config_file_name;
    FILE *config_file;

    GNU_DOS_LEVEL = 1;

    if(!(pass = getpwuid(geteuid()))) 
    {
        fprintf(stderr, "Error: couldn't open home directory to read "
                        "configuration file.\n");
        fprintf(stderr, "Aborting.\n");
        exit(1);
    }

    config_file_name = malloc(strlen(pass->pw_dir)+12);

    if(!config_file_name)
    {
        printf("Insufficient memory\n");
        exit(1);
    }

    strcpy(config_file_name, pass->pw_dir);
    strcat(config_file_name, "/");
    strcat(config_file_name, ".prime.conf");

    if(!(config_file = fopen(config_file_name, "r"))) 
    {
        fprintf(stderr, "Error: couldn't read configuration file in "
                        "home directory.\n");
        fprintf(stderr, "Resetting configuration file.\n");
        write_config_file_defaults();
        config_file = fopen(config_file_name, "r");
    }

    char buf[256];

    //read configuration file
    while(fgets(buf, sizeof(buf), config_file)) 
    {
        if(buf[0] == '#' || buf[0] == '\n') continue;
        else if(strstr(buf, "GNU_DOS_LEVEL")) 
        {
            GNU_DOS_LEVEL = atoi((strchr(buf, '=')+2));
        }
        else if(strstr(buf, "FG_COLOR_WIN")) 
        {
            FG_COLOR[COLOR_WINDOW] = atoi(strchr(buf, '=')+2);
        }
        else if(strstr(buf, "FG_COLOR_HLT")) 
        {
            FG_COLOR[COLOR_HIGHLIGHT_TEXT] = atoi(strchr(buf, '=')+2);
        }
        else if(strstr(buf, "FG_COLOR_MBAR")) 
        {
            FG_COLOR[COLOR_MENU_BAR] = atoi(strchr(buf, '=')+2);
        }
        else if(strstr(buf, "FG_COLOR_SBAR")) 
        {
            FG_COLOR[COLOR_STATUS_BAR] = atoi(strchr(buf, '=')+2);
        }
        else if(strstr(buf, "FG_COLOR_HBUT")) 
        {
            FG_COLOR[COLOR_HBUTTONS] = atoi(strchr(buf, '=')+2);
        }
        else if(strstr(buf, "FG_COLOR_BUT")) 
        {
            FG_COLOR[COLOR_BUTTONS] = atoi(strchr(buf, '=')+2);
        }
        else if(strstr(buf, "BG_COLOR_WIN")) 
        {
            BG_COLOR[COLOR_WINDOW] = atoi(strchr(buf, '=')+2);
        }
        else if(strstr(buf, "BG_COLOR_HLT")) 
        {
            BG_COLOR[COLOR_HIGHLIGHT_TEXT] = atoi(strchr(buf, '=')+2);
        }
        else if(strstr(buf, "BG_COLOR_MBAR")) 
        {
            BG_COLOR[COLOR_MENU_BAR] = atoi(strchr(buf, '=')+2);
        }
        else if(strstr(buf, "BG_COLOR_SBAR")) 
        {
            BG_COLOR[COLOR_STATUS_BAR] = atoi(strchr(buf, '=')+2);
        }
        else if(strstr(buf, "BG_COLOR_HBUT")) 
        {
            BG_COLOR[COLOR_HBUTTONS] = atoi(strchr(buf, '=')+2);
        }
        else if(strstr(buf, "BG_COLOR_BUT")) 
        {
            BG_COLOR[COLOR_BUTTONS] = atoi(strchr(buf, '=')+2);
        }
    }//end while

    fclose(config_file);
    free(config_file_name);

    if(NEW_GNU_DOS_LEVEL)
    GNU_DOS_LEVEL = NEW_GNU_DOS_LEVEL;
    if(GNU_DOS_LEVEL > 6 || GNU_DOS_LEVEL < 1) GNU_DOS_LEVEL = 1;

    return 1;
}

int write_config_file_defaults(void)
{
    struct passwd *pass;
    char *config_file_name;
    FILE *config_file;

    if(!(pass = getpwuid(geteuid()))) 
    {
        return 0;
    }

    config_file_name = malloc(strlen(pass->pw_dir)+12);

    if(!config_file_name)
    {
        return 0;
    }

    strcpy(config_file_name, pass->pw_dir);
    strcat(config_file_name, "/");
    strcat(config_file_name, ".prime.conf");

    if(!(config_file = fopen(config_file_name, "w"))) 
    {
        free(config_file_name);
        return 0;
    }

    //printf("Resetting program configuration..\n");
    //write default values to the configuration file
    fprintf(config_file, "#Configuration file for the prime program\n");
    fprintf(config_file, "#Please do not modify this file by hand\n\n");
    fprintf(config_file, "#Display colors\n");
    fprintf(config_file, "FG_COLOR_WIN = 37\n");
    fprintf(config_file, "FG_COLOR_HLT = 34\n");
    fprintf(config_file, "FG_COLOR_MBAR = 34\n");
    fprintf(config_file, "FG_COLOR_SBAR = 34\n");
    fprintf(config_file, "FG_COLOR_HBUT = 32\n");
    fprintf(config_file, "FG_COLOR_BUT = 37\n");
    fprintf(config_file, "BG_COLOR_WIN = 49\n");
    fprintf(config_file, "BG_COLOR_HLT = 47\n");
    fprintf(config_file, "BG_COLOR_MBAR = 47\n");
    fprintf(config_file, "BG_COLOR_SBAR = 47\n");
    fprintf(config_file, "BG_COLOR_HBUT = 41\n");
    fprintf(config_file, "BG_COLOR_BUT = 41\n\n");
    fprintf(config_file, "#GnuDOS Level\n");
    fprintf(config_file, "GNU_DOS_LEVEL = %d\n", GNU_DOS_LEVEL);

    fclose(config_file);
    free(config_file_name);

    return 1;
}

