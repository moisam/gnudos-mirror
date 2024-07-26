/* 
 *    Programmed By: Mohammed Isam Mohammed [mohammed_isam1984@yahoo.com]
 *    Copyright 2013, 2014, 2015, 2016, 2017, 2018, 2024 (c)
 * 
 *    file: find.c
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

#define _XOPEN_SOURCE   500
#include <ncurses.h>
#include <ftw.h>
#include <fnmatch.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pwd.h>
#include "find.h"

int input_len[2];   //0=length of the searched file name
                    //1=length of the path to search in
int highlight[2];   //0=highlighted char in input field 1
                    //1=highlighted char in input field 2
char findFileName[MAX_INPUT1_LEN]; //filename to be searched for
char findInDir[MAX_INPUT2_LEN];    //optional for user--path to search in

FILE *searchResultsFile;
int fileMatches;

//function called by findFile() function
void scanDirForFile(char *tmp);


static void draw_findFile_buttons(int bx, int by, int selection)
{
    if(selection == 4) setScreenColorsI(COLOR_HBUTTONS);
    else               setScreenColorsI(COLOR_BUTTONS );

    mvprintw(bx-1, by-1, "   OK   ");
    by += 12;

    if(selection == 5) setScreenColorsI(COLOR_HBUTTONS);
    else               setScreenColorsI(COLOR_BUTTONS );

    mvprintw(bx-1, by-1, " CANCEL ");

    //adjust cursor to point at selected button
    if(selection == 4)      move(bx-1, by-10);
    else if(selection == 5) move(bx-1, by);

    refresh();
}

static void input_del_char(char *input, int which, int x, int y)
{
    int i;

    if(input_len[which] == 0 || highlight[which] == 0) return;

    setScreenColorsI(COLOR_HIGHLIGHT_TEXT);

    for(i = highlight[which]; i < input_len[which]-1; i++)
        input[i] = input[i+1];

    input[i] = '\0';
    input_len[which]--;

    //adjust cursor to point at input field
    move(x-1, y-1);

    for(i = highlight[which]; i < input_len[which]; i++)  
        addch(input[i]);

    addch(' ');

    //adjust cursor to point at input field
    move(x-1, y-1);
    refresh();
}

/*************************************************
 * this function displays a dialog box for the
 * user to enter file name, where to search, and
 * OK/CANCEL buttons. it holds input from the user
 * to navigate the dialog box fields, and then 
 * calls scanThisDir() function to search.
 * ***********************************************/
void findFile(void)
{
    int x = 5;
    int w = 46;
    int h = 12+x;
    int y = (SCREEN_W-w)/2;
    int i, sel = 0;    //the selected control in the window
                       //sel:     0=first input field    1=first option
                       //        2=second option        3=third option
                       //        4=second input field    5=OK    6=CANCEL
    int osel = 0;      //the option marked with [X] in front of it
                       //osel: 0=first option, 1=second, 2=third
    int bx, by, ch;

    w += y;
    memset(findFileName, 0, sizeof(findFileName));
    memset(findInDir   , 0, sizeof(findInDir   ));
    input_len[0] = 0; input_len[1] = 0;
    highlight[0] = 0; highlight[1] = 0;
  
    if(!(searchResultsFile = tmpfile()))
    {
        msgBoxH("Error opening temporary file. Aborting.", BUTTON_OK, ERROR);
        setScreenColorsI(COLOR_WINDOW);
        return;
    }

    //Set the user interface//
    setScreenColorsI(COLOR_WINDOW);
    drawBox(x, y, h, w, " Find File ", 1);
    mvprintw(x+1, y+1, "Enter file name to search for:");
    setScreenColorsI(COLOR_HIGHLIGHT_TEXT);
    mvprintw(x+2, y+1, "%*s", MAX_INPUT1_LEN, " ");
    mvprintw(x+8, y+21, "%*s", MAX_INPUT2_LEN, " ");
    setScreenColorsI(COLOR_WINDOW);
    mvprintw(x+5, y+1, "Search in:");
    mvprintw(x+6, y+1, "[X] Only current working directory");
    mvprintw(x+7, y+1, "[ ] All the filesystem (slower)");
    mvprintw(x+8, y+1, "[ ] Under this path:");

    //Draw the buttons//
    showCursor();
    bx = h-1;
    by = y + ((w-y-16)/2) - 2;
    draw_findFile_buttons(bx, by, sel);
    //adjust cursor to be at input field
    move(x+2, y+1);
    refresh();

    //wait for user response//
    while(1)
    {    //infinite program loop//
        ch = getKey();

        switch(ch) 
        {
            case(ESC_KEY):
                setScreenColorsI(COLOR_WINDOW);
                hideCursor();
                fclose(searchResultsFile);
                return;

            case(SPACE_KEY):
                if(sel == 0 || sel == 4) 
                {    //if pressed space in input field, insert the space
                    goto enterNewChar;
                    break;
                }

            //if pressed space on a button or option, fall through to ENTER
            case(ENTER_KEY):
                switch(sel)
                {
                    case 1:
                    case 2:
                    case 3:
                        //select this option if not already selected
                        setScreenColorsI(COLOR_WINDOW);
                        if(osel != sel-1)
                            mvaddch(x+6+osel, y+2, ' ');

                        mvaddch(x+5+sel, y+2, 'X');
                        move(x+5+sel, y+2);

                        osel = sel-1;
                        break;

                    case 0:
                    case 4:
                    case 5:    //pressed ENTER on OK button or on the 
                               //first input field
                        if(input_len[0] <= 0)
                        {
                            hideCursor();
                            fclose(searchResultsFile);
                            return;    //if no input entered, just return
                        }

                        findFileName[input_len[0]] = '\0';

                        /////////////////////////////////////////////
                        /////////////////////////////////////////////
                        if(osel == 0) 
                        {    //search in current working directory
                            scanDirForFile(cwd);
                        }
                        else if (osel == 1) 
                        {    //search starting from '/'
                            scanDirForFile("/");
                        } 
                        else if (osel == 2) 
                        {    //search in selected path
                            if(input_len[1] == 0)
                            {
                                showErrorMsgBox("Invalid path:\n", findInDir);
                            }
                            else 
                            {
                                scanDirForFile(findInDir);
                            }
                        }

                        if(chdir(cwd)) showErrorMsgBox(strerror(errno), cwd);
                        fclose(searchResultsFile);
                        hideCursor();
                        return;                //otherwise return the input

                    case 6:
                        fclose(searchResultsFile);
                        hideCursor();
                        return;        //return NULL also if selected CANCEL
                }
                refresh();
                break;

            case(RIGHT_KEY):
                if(sel == 0) 
                {    //first input field
                    if(highlight[0] >= input_len[0]) break; //already at last char
                    if(highlight[0] == MAX_INPUT1_LEN-1) break; //ditto
                    highlight[0]++;
                    //adjust cursor to point at input field
                    move(x+2, y+highlight[0]+1);
                } 
                else if(sel == 4) 
                {    //second input field
                    if(highlight[1] >= input_len[1]) break; //already at last char
                    if(highlight[1] == MAX_INPUT2_LEN-1) break; //ditto
                    highlight[1]++;
                    //adjust cursor to point at input field
                    move(x+8, y+highlight[1]+21);
                } 
                refresh();
                break;

            case(LEFT_KEY):
                if(sel == 0)
                {    //first input field
                    if(highlight[0] <= 0) break; //already at first char
                    highlight[0]--;
                    //adjust cursor to point at input field
                    move(x+2, y+highlight[0]+1);
                }
                else if(sel == 4)
                {    //second input field
                    if(highlight[1] <= 0) break; //already at first char
                    highlight[1]--;
                    //adjust cursor to point at input field
                    move(x+8, y+highlight[1]+21);
                } 
                refresh();
                break;

            case(TAB_KEY):
                bx = h-1;
                by = y + ((w-y-16)/2) - 2;

                switch(sel)
                {
                    case 0:
                    case 1:
                    case 2:
                        move(x+6+sel, y+2);
                        sel++;
                        break;

                    case 3:
                        move(x+8, y+highlight[1]+21);
                        sel = 4;
                        break;

                    case 4:
                    case 5:
                        draw_findFile_buttons(bx, by, sel);
                        sel++;
                        break;

                    default:
                        draw_findFile_buttons(bx, by, sel);
                        sel = 0;
                        //adjust cursor to point at input field
                        move(x+2, y+highlight[0]+1);
                }

                setScreenColorsI(COLOR_WINDOW);
                refresh();
                break;

            case(BACKSPACE_KEY):
                if(sel == 0)
                {
                    if(highlight[0] == 0) break;

                    highlight[0]--;
                    input_del_char(findFileName, 0, x+3, y+highlight[0]+2);
                } 
                else if(sel == 4)
                {
                    if(highlight[1] == 0) break;

                    highlight[1]--;
                    input_del_char(findInDir, 1, x+9, y+highlight[1]+22);
                }
                break;

            case(DEL_KEY):
                if(sel == 0)
                {
                    input_del_char(findFileName, 0, x+3, y+highlight[0]+2);
                } 
                else if(sel == 4)
                {
                    input_del_char(findInDir, 1, x+9, y+highlight[1]+22);
                }
                break;

            default:
                if((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') ||
                   (ch >= 32 && ch<= 64) || (ch >=123 && ch <= 126))
                {    //if it is alphanumeric
enterNewChar:
                    if(sel == 0) 
                    {
                        if(strlen(findFileName) >= MAX_INPUT1_LEN) break;
                        setScreenColorsI(COLOR_HIGHLIGHT_TEXT);

                        // inserting in the middle of a string means we need 
                        // to shift all chars one position to the right before
                        // inserting the new char at the highlighted position.
                        if(findFileName[highlight[0]] != '\0') 
                        {
                            for(i = input_len[0]; i > highlight[0]; i--) 
                                findFileName[i] = findFileName[i-1];
                        }

                        findFileName[highlight[0]] = ch;
                        input_len[0]++;
                        addch(findFileName[highlight[0]++]);

                        if(input_len[0] > highlight[0]) 
                        {    //there are some chars to the right side
                             //adjust cursor to point at input field
                            for(i = highlight[0]; i < input_len[0]; i++) 
                                addch(findFileName[i]);
                        }

                        if(highlight[0] >= MAX_INPUT1_LEN) 
                            highlight[0] = MAX_INPUT1_LEN-1;

                        //adjust cursor to point at input field
                        move(x+2, y+highlight[0]+1);
                    }
                    else if(sel == 4) 
                    {    //the second input field
                        if(osel != 2)
                        {
                            setScreenColorsI(COLOR_WINDOW);
                            mvaddch(x+6+osel, y+2, ' ');
                            mvaddch(x+8, y+2, ' ');
                            osel = 2;
                            move(x+8, y+highlight[1]+21);
                        }

                        if(strlen(findInDir) >= MAX_INPUT2_LEN) break;
                        setScreenColorsI(COLOR_HIGHLIGHT_TEXT);

                        // inserting in the middle of a string means we need 
                        // to shift all chars one position to the right before
                        // inserting the new char at the highlighted position.
                        if(findInDir[highlight[1]] != '\0') 
                        {
                            for(i = input_len[1]; i > highlight[1]; i--) 
                                findInDir[i] = findInDir[i-1];
                        }

                        findInDir[highlight[1]] = ch;
                        input_len[1]++;
                        addch(findInDir[highlight[1]++]);

                        if(input_len[1] > highlight[1]) 
                        {    //there are some chars to the right side
                                //adjust cursor to point at input field
                            for(i = highlight[1]; i < input_len[1]; i++) 
                                addch(findInDir[i]);
                        }

                        if(highlight[1] >= MAX_INPUT2_LEN) 
                            highlight[1] = MAX_INPUT2_LEN-1;

                        //adjust cursor to point at input field
                        move(x+8, y+highlight[1]+21);
                    }
                }

                refresh();
                break;
        }
    }

    setScreenColorsI(COLOR_WINDOW);
    fclose(searchResultsFile);
}

// callback for nftw
static int match_filename(const char *fpath, const struct stat *sb,
                          int tflag, struct FTW *ftwbuf)
{
    char *filename;

    // match the basename, not the whole path
    if((filename = strrchr(fpath, '/'))) filename++;
    else filename = (char *)fpath;

    if(fnmatch(findFileName, filename, FNM_PERIOD) == 0)
    {
        fprintf(searchResultsFile, "%s\n", fpath);
        fileMatches++;
    }

    return 0;
}

/********************************************
 * this function searches for the requested
 * file in the directory passed as tmp[] and
 * it's subdirectories. NOT to be called
 * directly, instead, call from findFile().
 * PARAMETERS:
 *     tmp[]: name of directory to search in
 *     level: current depth, used in recursive
 *         function calling
 * ******************************************/
void scanDirForFile(char *tmp) 
{
    fileMatches = 0;

    if(nftw(tmp, match_filename, 20, FTW_PHYS) == -1)
    {
        showErrorMsgBox("Search: ", strerror(errno));
        return;
    }

    if(fileMatches == 0)
    {
        msgBoxH("Search: No matches were found.", BUTTON_OK, INFO);
    }
    else
    {
        fprintf(searchResultsFile, "Total results: %d\n", fileMatches);
        fflush(searchResultsFile);
        showReadme(searchResultsFile, " Search results: ", GNU_DOS_LEVEL);
    }

    setScreenColorsI(COLOR_WINDOW);
    refresh();
}

