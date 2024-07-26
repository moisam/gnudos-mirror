/* 
 *    Programmed By: Mohammed Isam Mohammed [mohammed_isam1984@yahoo.com]
 *    Copyright 2014, 2015, 2016, 2017, 2018, 2024 (c)
 * 
 *    file: dialogs.c
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

#include "config.h"
#define _GNU_SOURCE
#define _XOPEN_SOURCE_EXTENDED
#include "dialogs.h"
#include "kbd.h"
#include "screen.h"
#ifdef HAVE_NCURSESW_NCURSES_H
# include <ncursesw/ncurses.h>
#else
# include <ncurses.h>
#endif
#include <wchar.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define SET_COLOR(i)            setScreenColors(FG_COLOR[i], BG_COLOR[i])

/* Maximum allowed length of a user input in an input box */
#define MAX_INPUT_MSG_LEN       256

int inputLen = 0;       //how long is the input?
int highlightChar = 0;  //which char is under the cursor?
int firstVisChar = 0;   //which char is the first in input line? 
                        //(used when scrolling a long input)

/* Message box width and height */
int MAX_MSG_BOX_W;
int MAX_MSG_BOX_H;

static int x, y, w, h;

/* mask values for bit pattern of first byte in multi-byte
     UTF-8 sequences: 
       192 - 110xxxxx - for U+0080 to U+07FF 
       224 - 1110xxxx - for U+0800 to U+FFFF 
       240 - 11110xxx - for U+010000 to U+1FFFFF */
static unsigned short mask[] = {192, 224, 240};

/* input string returned by the inputBoxI() function */
char input[(MAX_INPUT_MSG_LEN*2)+1];

/* input string returned by the inputBoxIWC() function */
wchar_t wcinput[MAX_INPUT_MSG_LEN+1];


/*
 * Convert a wide character string to a "normal" C-string.
 * The result is malloc'd, it is the caller's duty to free it.
 * On error, NULL is returned.
 * If reslen is not NULL, the string length is stored here.
 */
char *to_cstring(char *src, size_t *reslen)
{
    if(!src) return NULL;

    size_t slen = wcsrtombs(NULL, (const wchar_t **)&src, 0, NULL);
    char *s;

    if(slen == (size_t)-1) return NULL;
    if(!(s = malloc(slen+1))) return NULL;

    slen = wcsrtombs(s, (const wchar_t **)&src, slen, NULL);
    s[slen] = '\0';
    if(reslen) *reslen = slen;

    return s;
}

/*
 * Convert a C-string to a wide character string.
 * The result is malloc'd, it is the caller's duty to free it.
 * On error, NULL is returned.
 * If reslen is not NULL, the wide character string length is stored here.
 */
wchar_t *to_widechar(char *src, size_t *reslen)
{
    if(!src) return NULL;

    size_t wclen = mbsrtowcs(NULL, (const char **)&src, 0, NULL);
    wchar_t *wcs;

    if(wclen == (size_t)-1) return NULL;
    if(!(wcs = malloc((wclen+1) * sizeof(wchar_t)))) return NULL;
    wclen = mbsrtowcs(wcs, (const char **)&src, wclen, NULL);
    wcs[wclen] = L'\0';
    if(reslen) *reslen = wclen;

    return wcs;
}

static inline int addwstr_wrapper(const wchar_t *wstr)
{
#ifdef HAVE_NCURSESW_NCURSES_H
    // we have ncursesw, so pass the call on
    return addwstr(wstr);
#else
    // if we don't have ncursesw, convert each wchar to a multibyte char
    // and print it out
    size_t i, wstrlen = wcslen(wstr);
    size_t clen;
    char cstr[16];
    wchar_t *p, wctmp[2] = { L'\0', L'\0' };

    for(i = 0; i < wstrlen; i++)
    {
        wctmp[0] = wstr[i];
        p = wctmp;
        clen = wcsrtombs(cstr, (const wchar_t **)&p, 1, NULL);
        cstr[clen] = '\0';
        printw("%s", cstr);
    }

    return OK;
#endif
}

static inline int mvaddwstr_wrapper(int y, int x, const wchar_t *wstr)
{
#ifdef HAVE_NCURSESW_NCURSES_H
    // we have ncursesw, so pass the call on
    return mvaddwstr(y, x, wstr);
#else
    // if we don't have ncursesw, do it manually
    move(y, x);
    return addwstr_wrapper(wstr);
#endif
}

static inline void addwch_wrapper(wchar_t ch)
{
    wchar_t wctmp[2] = { ch, L'\0' };

#ifdef HAVE_NCURSESW_NCURSES_H
    addwstr(wctmp);
#else
    size_t clen;
    char cstr[16];
    clen = wcsrtombs(cstr, (const wchar_t **)&wctmp, 1, NULL);
    cstr[clen] = '\0';
    printw("%s", cstr);
#endif
}

/*
 * Common prologue used by all dialog functions.
 */
static void __dialog_prologueWC(int *msgW, int *msgH, wchar_t *msg,
                                wchar_t *title, int *x, int *y)
{
    int i, j, l;
    size_t len = wcslen(msg);

    MAX_MSG_BOX_H = SCREEN_H-2;
    MAX_MSG_BOX_W = SCREEN_W-2;

    SET_COLOR(COLOR_WINDOW);
    j = 0;

    for(i = 0; i < len; i++) 
    {
        if(msg[i] == L'\n') { (*msgH)++; j = 0; }
        else 
        { 
            if(j > MAX_MSG_BOX_W) { j = 0; (*msgH)++; }
            j++; 
            if(j > (*msgW)) (*msgW) = j;
        }
    }

    *msgH += 4;
    *msgW += 3;    //adjust box size

    if(*msgW < 34) *msgW = 34;
    if(*msgH % 2) (*msgH)++;
    if(*msgH > MAX_MSG_BOX_H) *msgH = MAX_MSG_BOX_H;

    //draw the empty window first//
    *x = (SCREEN_H/2)-((*msgH)/2);
    *y = (SCREEN_W-(*msgW))/2;

    drawBoxWC(*x, *y, (SCREEN_H/2)+((*msgH)/2),
                      (SCREEN_W/2)+((*msgW)/2), title, 1);

    (*y) += 2; j = (*x)+1; (*x)++;
    l = (*y);
    move(j-1, l-1);

    for(i = 0; i < len; i++) 
    {
        if(msg[i] == L'\n') { l=(*y); j++; move(j-1, l-1); }
        else 
        { 
            if(l >= MAX_MSG_BOX_W)
            {
                l = (*y);
                move(j-1, l-1);
            }

            addwch_wrapper(msg[i]);
        }
    }

    refresh();
}

static void __draw_dialog_buttons(int *msgW, int *msgH, int buttons, 
                                  int *x, int *y, int sel)
{
    int bx, by;

    if(buttons == 5) 
    {    // OK/CANCEL combination
        bx = *x + ((*msgH)-2);
        by = *y + (((*msgW)-16)/2) - 2;

        if(sel == 0) SET_COLOR(COLOR_HBUTTONS);
        else SET_COLOR(COLOR_BUTTONS);

        mvprintw(bx-1, by-1, "   OK   ");
        by += 12;

        if(sel == 1) SET_COLOR(COLOR_HBUTTONS);
        else SET_COLOR(COLOR_BUTTONS);

        mvprintw(bx-1, by-1, " CANCEL ");
        move(bx-1, by-10);
    }
    else if(buttons == 10) 
    {    // YES/NO combination
        bx = *x + ((*msgH)-2);
        by = *y + (((*msgW)-16)/2) - 2;

        if(sel == 0) SET_COLOR(COLOR_HBUTTONS);
        else SET_COLOR(COLOR_BUTTONS);

        mvprintw(bx-1, by-1, "   YES  ");
        by += 12;

        if(sel == 1) SET_COLOR(COLOR_HBUTTONS);
        else SET_COLOR(COLOR_BUTTONS);

        mvprintw(bx-1, by-1, "   NO   ");
        move(bx-1, by-10);
    }
    else if(buttons == 26) 
    {    // YES/ALL/NO combination
        bx = *x + ((*msgH)-2);
        by = *y + (((*msgW)-24)/2) - 2;

        if(sel == 0) SET_COLOR(COLOR_HBUTTONS);
        else SET_COLOR(COLOR_BUTTONS);

        mvprintw(bx-1, by-1, "   YES  ");
        by += 10;

        if(sel == 1) SET_COLOR(COLOR_HBUTTONS);
        else SET_COLOR(COLOR_BUTTONS);

        mvprintw(bx-1, by-1, "   ALL  ");
        by += 10;

        if(sel == 2) SET_COLOR(COLOR_HBUTTONS);
        else SET_COLOR(COLOR_BUTTONS);

        mvprintw(bx-1, by-1, "   NO   ");
        move(bx-1, by-18);
    }
    else 
    {            // OK only
        bx = *x + ((*msgH)-2);
        by = *y + (((*msgW)-8)/2) - 2;

        if(sel == 0) SET_COLOR(COLOR_HBUTTONS);
        else SET_COLOR(COLOR_BUTTONS);

        mvprintw(bx-1, by-1, "   OK   ");
        move(bx-1, by+2);
    }

    refresh();
}

/**************************************************************************
 * inputBoxI(): 
 * Procedure to show an input box containing a message, OK/CANCEL buttons,
 * input field with inputValue as starting value, and a caption.
 * It returns the input string if the user presses OK, or a NULL pointer if
 * they pressed Cancel or entered an empty string.
 * ************************************************************************/
char *inputBoxI(char *msg, char *inputValue, char *title) 
{
    // convert everything to wide char strings
    wchar_t *wctitle = to_widechar(title, NULL);
    wchar_t *wcinput = to_widechar(inputValue, NULL);
    wchar_t *wcmsg = to_widechar(msg, NULL);
    wchar_t *res;
    size_t len;

    // and let inputBoxIWC() do the heavy lifting
    res = inputBoxIWC(wcmsg, wcinput, wctitle);

    if(wctitle) free(wctitle);
    if(wcinput) free(wcinput);
    if(wcmsg) free(wcmsg);

    // if we have a result, convert it back to a multibyte string before
    // passing it back to the user
    if(res)
    {
        len = wcsrtombs(input, (const wchar_t **)&res, 
                                        MAX_INPUT_MSG_LEN*2, NULL);

        if(len == (size_t)-1) return NULL;

        input[len] = '\0';
        return input;
    }

    return NULL;
}

/**************************************************************************
 * inputBoxIWC(): 
 * Procedure to show an input box containing a message, OK/CANCEL buttons,
 * input field with inputValue as starting value, and a caption.
 * It returns the input string if the user presses OK, or a NULL pointer if
 * they pressed Cancel or entered an empty string.
 *
 * This function is similar to inputBoxI(), except that its input is in
 * wide character format, as well as the returned string. This is useful
 * for getting non-Latin input. The function relies on the wide character
 * functionality provided by the ncursesw library.
 * ************************************************************************/
wchar_t *inputBoxIWC(wchar_t *msg, wchar_t *inputValue, wchar_t *title) 
{
    int msgW = 0;
    int msgH = 0;
    int i = 0;
    int x, y;
    int bx, by;
    int sel = 2;    //selection: 0=OK, 1=CANCEL, 2=INPUT FIELD
    char *ch;
    int utf8_bytes;

    showCursor();
    __dialog_prologueWC(&msgW, &msgH, msg, title, &x, &y);
    memset(wcinput, 0, MAX_INPUT_MSG_LEN);
    firstVisChar = 0; highlightChar = 0; inputLen = 0;

    //if passed an initial input value, load it into 'wcinput'
    if(inputValue)
    {
        if(wcslen(inputValue) > MAX_INPUT_MSG_LEN)
        {
            wcsncpy(wcinput, inputValue, MAX_INPUT_MSG_LEN);
            wcinput[MAX_INPUT_MSG_LEN] = L'\0';
        }
        else wcscpy(wcinput, inputValue);
    }
    else
    {
        wcinput[0] = L'\0';
    }

    inputLen = wcslen(wcinput);
 
    //put an empty field for user entry
    SET_COLOR(COLOR_HIGHLIGHT_TEXT);
    move(x+(msgH-5), y-1);

    if(inputLen > msgW-3) addnwstr(wcinput, msgW-3);
    else
    {
        addwstr_wrapper(wcinput);
        for(i = 0; i < (msgW-3-inputLen); i++) addch(' ');
    }
 
    //then draw button(s)//
    __draw_dialog_buttons(&msgW, &msgH, 5 /* OK|CANCEL */, &x, &y, -1);
    bx = x + (msgH-2);
    by = y + ((msgW-16)/2) + 10;

    //adjust cursor to point at input field
    move(bx-3, y-1);
    refresh();

    //wait for user response//
    while(1) 
    {    //infinite program loop//
        ch = ugetKey();

        switch(ch[0]) 
        {
            case(ESC_KEY):
                wcinput[0] = L'\0';
                return NULL;

            case(SPACE_KEY):
                if(sel == 2) 
                {    //if pressed space in input field, insert the space
                    goto enterInputChar;
                }    //if pressed space on a button, fall through to ENTER

            case(ENTER_KEY):
                if(sel == 0 || sel == 2)
                {   //pressed ENTER on OK button or on the input field
                    //if no input entered, return NULL
                    if(inputLen <= 0) { wcinput[0] = L'\0'; return NULL; }
                    return wcinput;     //otherwise return the input
                }
                else if(sel == 1) 
                {
                    wcinput[0] = L'\0';
                    return NULL;        //return NULL also if selected CANCEL
                }
                break;

            case(RIGHT_KEY):
                //already at last char
                if(firstVisChar+highlightChar >= inputLen) break;

                SET_COLOR(COLOR_HIGHLIGHT_TEXT);

                if(highlightChar >= msgW-4) 
                {   //need to scroll string
                    //can't go further right
                    if(inputLen <= firstVisChar+msgW-4) break;

                    //adjust cursor to point at input field
                    move(bx-3, y-1);

                    for(i = ++firstVisChar; i <= (firstVisChar+msgW-4); i++)
                    {
                        if(wcinput[i] == L'\0') addch(' ');
                        else addwch_wrapper(wcinput[i]);
                    }

                    //adjust cursor to point at input field
                    move(bx-3, y+highlightChar-1);
                }
                else 
                {        //no need to scroll string, just output the char
                    highlightChar++;
                    //adjust cursor to point at input field
                    move(bx-3, y+highlightChar-1);
                }

                refresh();
                break;

            case(LEFT_KEY):
                if(firstVisChar == 0 && highlightChar == 0) 
                    break;    //already at first char

                SET_COLOR(COLOR_HIGHLIGHT_TEXT);

                if(highlightChar == 0 && firstVisChar != 0)
                {   //need to scroll string
                    //adjust cursor to point at input field
                    move(bx-3, y-1);

                    for(i = --firstVisChar; i <= (firstVisChar+msgW-4); i++)
                        addwch_wrapper(wcinput[i]);

                    //adjust cursor to point at input field
                    move(bx-3, y-1);
                }
                else 
                {        //no need to scroll string, just output the char
                    highlightChar--;
                    //adjust cursor to point at input field
                    move(bx-3, y+highlightChar-1);
                }

                refresh();
                break;

            case(TAB_KEY):
                if(sel == 0)
                {
                    sel = 1;
                    __draw_dialog_buttons(&msgW, &msgH, 5 /* OK|CANCEL */, 
                                                                &x, &y, sel);
                    //adjust cursor to point at "CANCEL"
                    move(bx-1, by);
                } 
                else if(sel == 1) 
                {
                    sel = 2;
                    __draw_dialog_buttons(&msgW, &msgH, 5 /* OK|CANCEL */, 
                                                                &x, &y, sel);
                    //adjust cursor to point at input field
                    move(bx-3, y+highlightChar-1);
                } 
                else 
                {
                    sel = 0;
                    __draw_dialog_buttons(&msgW, &msgH, 5 /* OK|CANCEL */, 
                                                                &x, &y, sel);
                    //adjust cursor to point at "OK"
                    move(bx-1, by-10);
                }

                refresh();
                break;

            case(DEL_KEY):
                if((firstVisChar+highlightChar) == (inputLen))
                    break;    //can't delete.. at the last char

                for(i = firstVisChar+highlightChar; i < inputLen-1; i++) 
                    wcinput[i] = wcinput[i+1];

                wcinput[--inputLen] = L'\0';
                SET_COLOR(COLOR_HIGHLIGHT_TEXT);

                //adjust cursor to point at input field
                move(bx-3, y+highlightChar-1);

                for(i = highlightChar; i < (msgW-3); i++) 
                {
                    if(wcinput[firstVisChar+i] == '\0') addch(' ');
                    else addwch_wrapper(wcinput[firstVisChar+i]);
                }

                //adjust cursor to point at input field
                move(bx-3, y+highlightChar-1);
                refresh();
                break;

            case(BACKSPACE_KEY):
                if(highlightChar == 0) 
                {
                    if(firstVisChar == 0) break;    //at first char
                    firstVisChar--;

                    for(i = firstVisChar; i < inputLen-1; i++) 
                        wcinput[i] = wcinput[i+1];

                    wcinput[--inputLen] = L'\0';
                }
                else 
                { 
                    highlightChar--;

                    //shift the chars one place to the left
                    for(i = firstVisChar+highlightChar; i < inputLen-1; i++)
                        wcinput[i] = wcinput[i+1];

                    wcinput[--inputLen] = L'\0';
                }

                SET_COLOR(COLOR_HIGHLIGHT_TEXT);

                //adjust cursor to point at input field
                move(bx-3, y-1);

                for(i = firstVisChar; i < (firstVisChar+msgW-3); i++) 
                {
                    if(wcinput[i] == '\0') addch(' ');
                    else addwch_wrapper(wcinput[i]);
                }

                //adjust cursor to point at input field
                move(bx-3, y+highlightChar-1);
                refresh();
                break;

            default:
                // check for a UTF-8 sequence
                for(utf8_bytes = 0; ch[utf8_bytes]; utf8_bytes++) ;

                // UTF-8 or alphanumeric ASCII char
                if((utf8_bytes > 1) ||
                   (ch[0] >= 'a' && ch[0] <= 'z') || 
                   (ch[0] >= 'A' && ch[0] <= 'Z') ||
                   (ch[0] >= 32 && ch[0] <= 64) ||
                   (ch[0] >=123 && ch[0] <= 126))
                {
enterInputChar: ;
                    wchar_t wch;

                    if(inputLen >= MAX_INPUT_MSG_LEN) continue;
                    SET_COLOR(COLOR_HIGHLIGHT_TEXT);

                    // inserting in the middle of a string means we need to
                    // shift all chars one position to the right before
                    // inserting the new char at the highlighted position.
                    if(wcinput[highlightChar] != L'\0') 
                    {
                        for(i = inputLen; i > firstVisChar+highlightChar; i--)
                            wcinput[i] = wcinput[i-1];
                    }

                    mbrtowc(&wch, ch, utf8_bytes, NULL);
                    wcinput[firstVisChar+(highlightChar++)] = wch;
                    inputLen++;

finishEnterChar:
                    if(highlightChar >= msgW-3)
                    {   //need to scroll string
                        //adjust cursor to point at input field
                        move(bx-3, y-1);
                        highlightChar--;

                        for(i = ++firstVisChar; i <= (firstVisChar+msgW-4); i++) 
                            addwch_wrapper(wcinput[i]);

                        //adjust cursor to point at input field
                        move(bx-3, y+msgW-5);
                    } 
                    else 
                    {        //no need to scroll string, just output the char
                        addwch_wrapper(wcinput[highlightChar-1]);

                        //adjust cursor to point at input field
                        move(bx-3, y+highlightChar-1);

                        if(inputLen > firstVisChar+highlightChar) 
                        {    //there are some chars to the right side
                            for(i = highlightChar; i < (msgW-4); i++) 
                            {
                                if(wcinput[firstVisChar+i] == L'\0') addch(' ');
                                else addwch_wrapper(wcinput[firstVisChar+i]);
                            }
                        }

                        //adjust cursor to point at input field
                        move(bx-3, y+highlightChar-1);
                    }
                }

                refresh();
        }
    }

    SET_COLOR(COLOR_WINDOW);
    refresh();
    wcinput[0] = L'\0';

    return NULL;
}

/**************************************************************************
 * inputBox(): 
 * Procedure to show an input box containing a message, specific buttons,
 * empty field for user input, and a caption (according to passed 
 * msgType). It returns the input string if the user presses OK, or a NULL 
 * pointer if he pressed Cancel or entered an empty string.
 * ************************************************************************/
char *inputBox(char *msg, char *title) 
{
    return inputBoxI(msg, NULL, title);
}


char *getUserInput(char *msg, char *title)
{
    inputBox(msg, title);
    int len1 = strlen(input);
    if(!len1) return NULL;
    char *res = malloc(len1+1);
    if(!res) return NULL;
    strcpy(res, input);
    return res;
}


wchar_t *getUserInputWC(wchar_t *msg, wchar_t *title)
{
    inputBoxIWC(msg, NULL, title);
    int len1 = wcslen(wcinput);
    if(!len1) return NULL;
    wchar_t *res = malloc((len1+1) * sizeof(wchar_t));
    if(!res) return NULL;
    wcscpy(res, wcinput);
    return res;
}


int _msg_box(char *msg, char *title, int buttons)
{
    int msgW = 0;
    int msgH = 0;
    int bx, by;
    int sel = 0;
    int x, y, ch;

    // convert everything to wide char strings
    wchar_t *wctitle = to_widechar(title, NULL);
    wchar_t *wcmsg = to_widechar(msg, NULL);

    __dialog_prologueWC(&msgW, &msgH, wcmsg, wctitle, &x, &y);

    //then draw button(s)//
    __draw_dialog_buttons(&msgW, &msgH, buttons, &x, &y, sel);

    if(wctitle) free(wctitle);
    if(wcmsg) free(wcmsg);

    //wait for user response//
    while(1) 
    {    //infinite program loop//
        ch = getKey();

        switch(ch) 
        {
            case(ESC_KEY):
                return BUTTON_ABORT;

            case(SPACE_KEY):
            case(ENTER_KEY): 
                //remember, buttons = 5 for OK/CANEL, and = 10 for YES/NO
                if(sel == 0 && buttons == 5)  return BUTTON_OK;
                if(sel == 0 && buttons == 10) return BUTTON_YES;
                if(sel == 1 && buttons == 5)  return BUTTON_CANCEL;
                if(sel == 1 && buttons == 10) return BUTTON_NO;
                if(sel == 0 && buttons == 1)  return BUTTON_OK;
                if(sel == 0 && buttons == 26) return BUTTON_YES;
                if(sel == 1 && buttons == 26) return BUTTON_ALL;
                if(sel == 2 && buttons == 26) return BUTTON_NO;
                if(buttons == 1) return BUTTON_OK;
                break;

            case(RIGHT_KEY):
            case(LEFT_KEY):
            case(TAB_KEY):
                bx = x + (msgH-2);
                by = y + ((msgW-16)/2) - 2;
                by += 12;

                if(sel == 0 && buttons == 26) 
                {
                    sel = 1;
                    __draw_dialog_buttons(&msgW, &msgH, 26 /* YES|ALL|NO */, 
                                                                &x, &y, sel);
                    //adjust cursor to point at "ALL"
                    move(bx-1, by-8);
                } 
                else if(sel == 1 && buttons == 26) 
                {
                    sel = 2;
                    __draw_dialog_buttons(&msgW, &msgH, 26 /* YES|ALL|NO */, 
                                                                &x, &y, sel);
                    //adjust cursor to point at "NO"
                    move(bx-1, by+2);
                } 
                else if(sel == 2 && buttons == 26) 
                {
                    sel = 0;
                    __draw_dialog_buttons(&msgW, &msgH, 26 /* YES|ALL|NO */, 
                                                                &x, &y, sel);
                    //adjust cursor to point at "YES"
                    move(bx-1, by-18);
                } 
                else if(sel == 0 && buttons == 5) 
                {
                    sel = 1;
                    __draw_dialog_buttons(&msgW, &msgH, 5 /* OK|CANCEL */, 
                                                                &x, &y, sel);
                    //adjust cursor to point at "CANCEL"
                    move(bx-1, by);
                } 
                else if(sel == 0 && buttons == 10) 
                {
                    sel = 1;
                    __draw_dialog_buttons(&msgW, &msgH, 10 /* YES|NO */, 
                                                                &x, &y, sel);
                    //adjust cursor to point at "NO"
                    move(bx-1, by);
                } 
                else if(sel == 1 && buttons == 5) 
                {
                    sel = 0;
                    __draw_dialog_buttons(&msgW, &msgH, 5 /* OK|CANCEL */, 
                                                                &x, &y, sel);
                    //adjust cursor to point at "OK"
                    move(bx-1, by-10);
                } 
                else if(sel == 1 && buttons == 10)
                {
                    sel = 0;
                    __draw_dialog_buttons(&msgW, &msgH, 10 /* YES|NO */, 
                                                                &x, &y, sel);
                    //adjust cursor to point at "YES"
                    move(bx-1, by-10);
                }

                refresh();
                break;
        }
    }

    setScreenColors(FG_COLOR[COLOR_WINDOW], BG_COLOR[COLOR_WINDOW]);
    refresh();
    return(0);
}

/***************************************
 * msgBoxH(): 
 * Procedure to show a message box 
 * containing a message, specific buttons,
 * and a caption (according to passed 
 * msgType).
 * **************************************/
int msgBox(char *msg, int buttons, msgtype tmsg)
{
    char *title;

    switch(tmsg) 
    {
        case(INFO): title = " INFORMATION "; break;
        case(ERROR): title = " ERROR "; break;
        case(CONFIRM): title = " CONFIRMATION "; break;
        default: title = " MESSAGE "; break;
    }

    return _msg_box(msg, title, buttons);
}

/* same as msgBoxH(), just shows the cursor before displaying
 * the msg box, and hides it afterwards.
 */
int msgBoxH(char *msg, int buttons, msgtype tmsg)
{
    showCursor();
    int res = msgBox(msg, buttons, tmsg);
    hideCursor();
    return res;
}

/*********************************************************************
 * drawBox(): 
 * Procedure to draw a box with the given coordinates, title, and a 
 * flag indicating whether to clear the window area or not (passed as
 * 1 or 0).
 *********************************************************************/
void drawBox(int x1, int y1, int x2, int y2, char *title, int clearArea) 
{
    size_t wclen = 0;
    wchar_t *wctitle = to_widechar(title, &wclen);

    // let drawBoxWC() do the heavy lifting
    drawBoxWC(x1, y1, x2, y2, wctitle, clearArea);

    if(wctitle) free(wctitle);
}

/*********************************************************************
 * drawBoxWC(): 
 * Procedure to draw a box with the given coordinates, title, and a 
 * flag indicating whether to clear the window area or not (passed as
 * 1 or 0).
 *
 * Similar to drawBox() except it works with wide char strings.
 *********************************************************************/
void drawBoxWC(int x1, int y1, int x2, int y2, wchar_t *title, int clearArea) 
{
    if(y2 <= y1) return;
    if(x2 <= x1) return;

    wchar_t spaces[y2-y1];
    int i;

    for(i = 0; i < y2-y1-1; i++) spaces[i] = L' ';
    spaces[i] = L'\0';

    reset_attribs();
    SET_COLOR(COLOR_WINDOW);

    if(clearArea)
    {
        for(i = 1; i < (x2-x1); i++)
        {
            mvaddwstr_wrapper(x1+i-1, y1, spaces);
        }
    }
  
    //Draw the box first//
    move(x1-1, y1-1);
    refresh();

    addch(ACS_ULCORNER);            //print the upper-left corner

    for(i = 0; i < (y2-y1)-1; i++)
    {
        addch(ACS_HLINE);            //print the horizontal upper bar
    }

    addch(ACS_URCORNER);              //print the upper-right corner

    for(i = 0; i < (x2-x1)-1; i++) 
    {
        //move cursor to left window edge
        mvaddch(x1+i, y1-1, ACS_VLINE);
        mvaddch(x1+i, y2-1, ACS_VLINE);
    }

    mvaddch(x2-1, y1-1, ACS_LLCORNER);  //print the lower-left corner

    for(i = 0; i < (y2-y1)-1; i++) 
    {
        addch(ACS_HLINE);            //print the horizontal lower bar
    }

    addch(ACS_LRCORNER);  
    refresh();

    //Then put on the box title, if any//
    if(title != NULL) 
    {
        int tmp1=(y2-y1)/2;
        int tmp2=wcslen(title)/2;

        mvaddwstr_wrapper(x1-1, y1+tmp1-tmp2-1, title);
    }

    refresh();
}

/***************************************
 * drawBoxP():
 * same as drawBox(), except it accepts
 * coordinates as 'point' structures.
 * *************************************/
void drawBoxP(point p1, point p2, char* title, int clearArea) 
{
    drawBox(p1.row, p1.col, p2.row, p2.col, title, clearArea);
}

/******************************************
 * showAbout(): 
 * Procedure to show an 'About' dialog box 
 * containing a message, OK button,
 * and a caption (About).
 * ****************************************/
int showAbout(char *msg)
{
    return _msg_box(msg, " ABOUT ", BUTTON_OK);
}

/******************************************
 * showReadme(): 
 * Procedure to show a 'ReadMe' window
 * containing the contents of 'README' file,
 * passed as *readme. You can pass GNU_DOS_LEVEL
 * if u don't need it. Title is provided as a
 * convenience, for example GnuDOS programs
 * use this same function to display readme
 * and keybindings windows.
 * ****************************************/
int showReadme(FILE *README, char *title, int GNU_DOS_LEVEL)
{
    static char *readme_err[] =
    {
        "Failed to open README file!.",
        "Error reading README file!.",
        "Insufficient memory!.",
    };

    int err = -1, i;
    char *buf;        //buffer to hold data
    long buf_len = 0;

    if(!README)
    {
        msgBoxH(readme_err[0], BUTTON_OK, ERROR);
        return 1;
    }
  
    i = fseek(README, 0, SEEK_END);
    buf_len = ftell(README);
    rewind(README);
    if(i == -1 || buf_len == -1) { err = 1; goto return_err; }
    buf_len += 512;

    buf = malloc(buf_len);
    if(!buf) { err = 2; goto return_err; }

    int x = 3;
    int y = 3;
    int w = SCREEN_W-4;
    int h = SCREEN_H-4;
    int firstVisLine = 0;
    int ch;
    int lines = 0;
    char moreLines = 1;    //used as boolean to indicate if still more lines
    int index = 0;
    int inc;
    long total_lines = 0;
    long first_char = 0;

    i = 0;

    while((inc = fgetc(README)) != EOF)
    {
        buf[index++] = inc;
        if(inc == '\n') { i = 0; total_lines++; }
        else i++;

        if(i == w)
        {
            buf[index++] = '\n';
            buf_len++;
            total_lines++;
            i = 0;
        }

        buf_len++;
    }

    buf[index] = '\0';
    buf_len = index;

    if(!total_lines)
    {
        if(index) total_lines = 1;
        else
        {
            err = 1; goto return_err;
        }
    }

    if(total_lines < h) moreLines = 0;
    else moreLines = 1;

    firstVisLine = 0;
    first_char = 0;

read:

    //redraw the box with its contents
    if(title) drawBox(x-1, y-1, h+x, w+y, title, 1);
    else      drawBox(x-1, y-1, h+x, w+y, " README ", 1);

    move(x-1, y-1);
    lines = 0;
    i = first_char;

    while(i < buf_len)
    {
        if(buf[i] == '\n')
        {
            lines++;
            move(x+lines-1, y-1);
            if(lines >= h) break;
        }
        else addch(buf[i]);

        i++;
    }

    refresh();

    if(firstVisLine+lines < total_lines) moreLines = 1;
    else moreLines = 0;
  
    while(1) 
    {
        ch = getKey();

        switch(ch) 
        {
            case('g'):
                if(GNU_DOS_LEVEL < 3) break;
                if(!CTRL) break;
                goto end;

            case(ESC_KEY):
                if(GNU_DOS_LEVEL > 2) break;

            case(ENTER_KEY):
            case(SPACE_KEY):
                goto end;

            case('p'):
                if(GNU_DOS_LEVEL < 2) break;
                if(!CTRL) break;
                goto do_up;

            case(UP_KEY):
                if(GNU_DOS_LEVEL > 1) break;
do_up:
                /* go up */
                if(firstVisLine == 0) break;
                i = first_char-1;

                while(i >= 0)
                {
                    i--;
                    if(buf[i] == '\n') break;
                }

                i++;
                first_char = i;
                firstVisLine--;
                goto read;

            case('n'):
                if(GNU_DOS_LEVEL < 2) break;
                if(!CTRL) break;
                goto do_down;

            case(DOWN_KEY):
                if(GNU_DOS_LEVEL > 1) break;
do_down:
                /* go down */
                if(!moreLines) break;
                i = first_char;

                while(i < buf_len)
                {
                    if(buf[i] == '\n' || buf[i] == '\0') break;
                    i++;
                }

                if(i < buf_len) i++;
                first_char = i;
                firstVisLine++;
                goto read;
        }    //end of switch
    }    //end of outer while
  
end:

    return 0;

return_err:

    msgBoxH(readme_err[err], BUTTON_OK, ERROR);
    return 1;
}

