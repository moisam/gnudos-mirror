/* 
 *    Programmed By: Mohammed Isam Mohammed [mohammed_isam1984@yahoo.com]
 *    Copyright 2014, 2015, 2016, 2017, 2018, 2024 (c)
 * 
 *    file: dialogs.h
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

#ifndef __DIALOGS_H
#define __DIALOGS_H

#include <wchar.h>
#include <signal.h>
#include <stdio.h>
#include "kbd.h"
#include "screen.h"

/* buttons used in message boxes */
#define BUTTON_OK      1    //00000001
#define BUTTON_YES     2    //00000010
#define BUTTON_CANCEL  4    //00000100
#define BUTTON_NO      8    //00001000
#define BUTTON_ALL     16   //00010000
#define BUTTON_ABORT   32   //00100000
//button combinations-->  00000101    BUTTON_OK/BUTTON_CANCEL  = 5
//                        00001010    BUTTON_YES/BUTTON_NO     = 10
//                        00011010    BUTTON_YES/BUTTON_ALL/BUTTON_NO = 26

/* Define a point */
typedef struct { int row; int col; } point;

/* types of message displayed in message boxes */
typedef enum msgt { INFO, ERROR, CONFIRM } msgtype;

//Function prototypes//

/* Draw an empty box, with an optional title
 * clearArea is a flag to indicate whether to clear the window with spaces
 */
void drawBox(int x1, int y1, int x2, int y2, char *title, int clearArea);
void drawBoxWC(int x1, int y1, int x2, int y2, wchar_t *title, int clearArea);
void drawBoxP(point p1, point p2, char *title, int clearArea);

/* Draw a message box with the given message and preset buttons */
int msgBox(char *msg, int buttons, msgtype tmsg);

/* Same as above, but hide the cursor afterwards */
int msgBoxH(char *msg, int buttons, msgtype tmsg);

/* Draw an input box with a message and a title */
char *inputBox(char *msg, char *title);

wchar_t *inputBoxIWC(wchar_t *msg, wchar_t *inputValue, wchar_t *title);

/* Draw an input box with a message, a title and an initial input value */
char *inputBoxI(char *msg, char *inputValue, char *title);

/* Draw an input box with a message, a title and an initial input value.
   Returns an malloc'ed string  */
char *getUserInput(char *msg, char *title);

/* Similar to the above but works with wide chars */
wchar_t *getUserInputWC(wchar_t *msg, wchar_t *title);

char *to_cstring(char *src, size_t *reslen);

wchar_t *to_widechar(char *src, size_t *reslen);

/* Show about dialog box */
int showAbout(char *msg);

/* Show ReadMe window:
 * If you don't know what GNU_DOS_LEVEL is, or you don't want to
 * use it, pass 0. Also, if you pass NULL for title the window
 * title will be set to README by default.
 */
int showReadme(FILE *readme, char *title, int GNU_DOS_LEVEL);

#endif /* __DIALOGS_H */
