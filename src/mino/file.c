/* 
 *    Programmed By: Mohammed Isam Mohammed [mohammed_isam1984@yahoo.com]
 *    Copyright 2014, 2015, 2016, 2017, 2018, 2024 (c)
 * 
 *    file: file.c
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

#define _GNU_SOURCE
#include <stdio.h>
#include "defs.h"
#include "file.h"
#include "edit.h"
#include "options.h"
#include "dialogs.h"

//boolean telling if the current file is a new or opened one
int NEW_FILE;
enum fstate FILE_STATE;


void exit_gracefully(int exit_code, char *open_file_name)
{
    if(FILE_STATE == MODIFIED) 
    {
        int i = msgBox("File has been changed. Save changes?",
                                        BUTTON_YES|BUTTON_NO, INFO);

        if(i == BUTTON_YES) 
        {
            if(NEW_FILE) 
            {
                if(!openSaveFile(SAVE, 1, open_file_name)) 
                {
                    msgBox("Failed to save file.", BUTTON_OK, ERROR);
                } else { FILE_STATE = SAVED; }
            } 
            else 
            {
                if(!openSaveFile(SAVE, 0, open_file_name)) 
                {
                    msgBox("Failed to save file.", BUTTON_OK, ERROR);
                } else { FILE_STATE = SAVED; }
            }
        } 
        else if(i == BUTTON_CANCEL) 
        { 
            FILE_STATE = MODIFIED;
        } else { FILE_STATE = IDLE; }

        if(FILE_STATE == SAVED || FILE_STATE == IDLE)
            goto good_to_go;
    } else goto good_to_go;

    refreshView();
    return;

good_to_go:

    fcloseall();
    setScreenColors(WHITE, BGBLACK);
    clearScreen(); 
    reset_attribs();
    setScreenColors(WHITE, BGBLACK);
    restoreTerminal(); 
    exit(exit_code); 
}

int checkFileModified(char *open_file_name)
{
    int res = 0;

    if(FILE_STATE == MODIFIED)
    {
        int i = msgBox("File has been changed. Save changes?",
                                    BUTTON_YES|BUTTON_NO, INFO);

        if(i == BUTTON_YES)
        {
            if(NEW_FILE)
            {
                if(!openSaveFile(SAVE, 1, open_file_name))
                {
                    msgBox("Failed to save file.", BUTTON_OK, ERROR);
                }
                else res = 1;
            }
            else
            {
                if(!openSaveFile(SAVE, 0, open_file_name))
                {
                    msgBox("Failed to save file.", BUTTON_OK, ERROR);
                }
                else res = 1;
            }
        }
        else res = 1;
    } 
    else res = 1;

    return res;
}

/************************************
 * Function called when user selects
 * New from File Menu or presses
 * CTRL+N.
 * **********************************/
char *fileMenu_New(char *open_file_name)
{
    if(FILE_STATE == NEW)
    {
        refreshView();
        return NULL;
    }

    if(checkFileModified(open_file_name))
    {
        NEW_FILE = 1;
        FILE_STATE = NEW;
    }
    
    if(FILE_STATE == NEW)
    {
        initNewDocument();
        resetLineCounters();
    }

    BG_COLOR[COLOR_WINDOW] = old_window_color;
    refreshView();

    return NULL;
}


/************************************
 * Function called when user selects
 * Open from File Menu or presses
 * CTRL+O.
 * **********************************/
char *fileMenu_Open(char *open_file_name)
{
    char *name = NULL;

    if(checkFileModified(open_file_name))
    {
        FILE_STATE = SAVED;
    }
    
    if(!(name = openSaveFile(OPEN, 1, open_file_name)))
    {
        //msgBox("Failed to open file.", BUTTON_OK, ERROR);
    }
    else
    {
        FILE_STATE = OPENED;
        NEW_FILE = 0;
        resetLineCounters();
        initEdit();
    }

    refreshView();

    return name;
}

/************************************
 * Function called when user selects
 * Save from File Menu or presses
 * CTRL+S.
 * **********************************/
char *fileMenu_Save(char *open_file_name)
{
    char *name = NULL;

    if(NEW_FILE)
    {
        //if new file is created on startup, save file with this name
        if(wcscmp(documentTitle, DEFAULT_TITLE) == 0) 
        {
            //show Save as.. dialog box
            if((name = openSaveFile(SAVE, 1, open_file_name)))
            {
                FILE_STATE = SAVED;
            }
        }
        else
        {
            //do not show dialog box
            if((name = openSaveFile(SAVE, 0, open_file_name)))
            {
                FILE_STATE = SAVED;
            }
        }
    } 
    else
    { //do not show dialog box
        if((name = openSaveFile(SAVE, 0, open_file_name)))
        {
            FILE_STATE = SAVED;
        }
    }

    refreshView();

    return name;
}

/************************************
 * Function called when user selects
 * Save As.. from File Menu.
 * **********************************/
char *fileMenu_SaveAs(char *open_file_name)
{
    char *name = NULL;

    if((name = openSaveFile(SAVE, 1, open_file_name)))
    {
        FILE_STATE = SAVED;
    }

    refreshView();

    return name;
}

char *fileMenu_Print(char *open_file_name)
{
    msgBox("Oops! This function is not currently implemented!.", BUTTON_OK, INFO);
    refreshView();
    return NULL;
}

char *fileMenu_Exit(char *open_file_name)
{
    exit_gracefully(0, open_file_name);
    return NULL;
}

