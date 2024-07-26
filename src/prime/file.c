/* 
 *    Programmed By: Mohammed Isam Mohammed [mohammed_isam1984@yahoo.com]
 *    Copyright 2013, 2014, 2015, 2016, 2017, 2018, 2024 (c)
 * 
 *    file: file.c
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

#include <ncurses.h>
#include <string.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pwd.h>
#include "defs.h"
#include "file.h"

void purgeLogFile(FILE *logfile)
{
    fseek(logfile, 0, SEEK_END);

    if(ftell(logfile) > 0)
    {
        showReadme(logfile, " Error log ", GNU_DOS_LEVEL);
    }

    fclose(logfile);
}


/***********************************************
 * This function exports the tree of directory
 * passed as argv[1] into file passed as argv[2]
 * from the command line.
 * *********************************************/
void exportTreeFromCommandLine(char *d, char *f) 
{
    FILE *file;

    if(!(file = fopen(f, "w"))) 
    {
        fprintf(stderr, "Error opening export file: '%s'.\nAborting.\n", f);
        exit(1);
    }

    fprintf(stdout, "Reading directory tree. Please wait..\n");
    fprintf(file, "\nDirectory tree of '%s/':", d);
    scanThisDir(d, file, 0, 0);

    fprintf(stdout, "Finished writing directory tree of '%s' to '%s'.\n", d, f);
    fclose(file);
}

/***********************************************
 * This function exports the tree under the
 * current working directory into an external
 * file as input by the user.
 * *********************************************/
void exportTree(void) 
{
    FILE *file;

    char *f = getUserInput("The directory tree of the current dir will "
                           "be exported.\n"
                           "Enter export file name:", "Export");

    if(f == NULL) return;

    if(!(file = fopen(f, "w"))) 
    {
        msgBoxH("Error opening export file. Aborting.", BUTTON_OK, ERROR);
        free(f);
        refreshWindows();
        return;
    }

    free(f);

    fprintf(file, "Directory tree of '%s/':", cwd);
    scanThisDir(cwd, file, 0, 1);
    fclose(file);

    if(chdir(cwd) == -1)
    {
        showErrorMsgBox("Error changing directory: ", strerror(errno));
    }
}

/***********************************************
 * This function scans the directory passed to
 * it as tmp[] for exporting. Not to be called
 * directly. Instead, called from exportTree().
 * *********************************************/
void scanThisDir(char *tmp, FILE *out, int level, int showProgress) 
{
    static int nf = 0;
    static int nd = 0;    //static: total num of files and dirs
    int n;
    struct dirent **eps;  //structure used in exportTree() function
    struct stat statbuf;
    int h = SCREEN_H/2;
    int w = SCREEN_W/2;
  
    //variables used in displaying progress message to the user
    static char exportingString[] = "Exporting %d of %d";
  
    n = scandir(tmp, &eps, one, alphasort);

    if(n >= 0) 
    {
        int cnt;

        if(chdir(tmp) == -1)
        {
            fprintf(out, "\n%*s (Error: %s)", level, " ", strerror(errno));
            return;
        }

        for(cnt = 0; cnt < n; ++cnt) 
        {
            char *d = eps[cnt]->d_name;

            if(level == 0 && showProgress) 
            {
                drawBox(h-2, w-12, h+1, w+12, NULL, 1);
                mvprintw(h-2, w-12, exportingString, cnt+1, n);
            }

            if(lstat(d, &statbuf) == -1)
            {
                if(showProgress) showErrorMsgBox(strerror(errno), d);
                if(chdir("..")) showErrorMsgBox(strerror(errno), "..");
                return;
            }
      
            if(S_ISDIR(statbuf.st_mode)) 
            {
                //ignore "." & ".."
                if(strcmp(d, ".") == 0 || strcmp(d, "..") == 0)
                    continue;
                fprintf(out, "\n%*s|---- %s", level, " ", eps[cnt]->d_name);
                
                nd++;
                scanThisDir(d, out, level+4, showProgress);
            } 
            else 
            {
                fprintf(out, "\n%*s|-[f] %s", level, " ", eps[cnt]->d_name);
                nf++;
            }
        }

        if(chdir("..") == -1)
        {
            fprintf(out, "\n%*s (Error: %s)", level, " ", strerror(errno));
        }
    }
    else 
    {
        fprintf(out, " (Failed to open directory)");
    }

    if(level == 0) 
    {
        fprintf(out, "\n---------------------------------\n");
        fprintf(out, "Total dirs: %d, Total files: %d\n", nd, nf);
    }
}

char *file_open_location(void)
{
    char *dir = getUserInput("Enter directory path to open:", "Open Location");
    hideCursor();

    if(!dir) return NULL;

    scanDir(dir);
    refreshWindows();
    return dir;
}

void fileMenu_CreateDir(void)
{
    struct stat st;

    char *dir = getUserInput("Enter directory name to create:", "New Directory");
    hideCursor();

    if(!dir) return;

    if(stat(dir, &st) == -1) 
    {
        mkdir(dir, 0775);
    } 
    else 
    {
        msgBoxH("Directory already exists!", BUTTON_OK, ERROR);
    }

    free(dir);
    scanDir(cwd);
    refreshWindows();
}

void fileMenu_Open(void)
{
    file_open_location();
}

void fileMenu_ExportTree(void)
{
    exportTree();
    setScreenColorsI(COLOR_WINDOW);
    scanDir(cwd);
}

void fileMenu_Print(void)
{
    msgBoxH("Oops! This function is currently not implemented.", BUTTON_OK, INFO);
    //showPrintDialogBox();
    refreshFileView();
    refreshDirView();
}

void fileMenu_Exit(void)
{
    if(msgBoxH("Are you sure you want to exit?", 
                BUTTON_YES | BUTTON_NO, CONFIRM) == BUTTON_YES)
    {
        exit_gracefully();
    }
}

