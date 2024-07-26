/* 
 *    Programmed By: Mohammed Isam Mohammed [mohammed_isam1984@yahoo.com]
 *    Copyright 2014, 2015, 2016, 2017, 2018, 2024 (c)
 * 
 *    file: edit.c
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
#include "defs.h"
#include "edit.h"
#include "kbd.h"
#include "dialogs.h"
#include "options.h"
#include <locale.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>

#define tostr(x)    #x

struct undostruct *firstUndo, *lastUndo, *firstRedo;
wchar_t undoBuf[4096];
int  undoBufIndex    = 0;
int  undoBufChars    = 0;
wchar_t undoBufRep[4096];
int  undoBufRepIndex = 0;
int  undoBufRepChars = 0;
int  clipboardSize   = 0;

pos_t find_result_pos[1024];
int   total_find_results;

int SELECTING;
int CLIPBOARD_IS_EMPTY;
int total_lines_in_clipboard;
wchar_t *clipboard;

pos_t sel_range_start;//start point of selected text
pos_t sel_range_end;  //end point of selected text

//definitions for the find/replace functions
static void _replace(int pos, wchar_t *f, wchar_t *r);
static void _do_replace(int pos, wchar_t *f, wchar_t *r);
static void _find(wchar_t *f);
static int fixViewPostUndo(int first);


/*
static wchar_t *wcscasecmp(const wchar_t *haystack, const wchar_t *needle)
{
    wchar_t c1, c2;
    local_t curlocale = uselocale(0);

    if(s1 == s2) return 0;

    do
    {
        c1 = tolower_l(*s1++, curlocale);
        c2 = tolower_l(*s2++, curlocale);

        if(c1 == L'\0') break;
    } while(c1 == c2);

    return c1-c2;
}
*/

/*
 * As wcscasestr() is not part of the standard libc, provide our own
 * implementation here.
 */
static wchar_t *wcscasestr(const wchar_t *haystack, const wchar_t *needle)
{
    if(!haystack || !needle) return NULL;

    size_t len = wcslen(needle);
    //locale_t curlocale = uselocale(0);

    for( ; *haystack; haystack++)
        if(!wcsncasecmp(haystack, needle, len))
        //if(!wcsncasecmp_l(haystack, needle, len, curlocale))
            return (wchar_t *)haystack;

    return NULL;
}

static void deleteUndoList(struct undostruct *first)
{
    while(first)
    {
        struct undostruct *undo = first->prev;
        if(first->text) free(first->text);
        free(first);
        first = undo;
    }
}

static void deleteRedoList(void)
{
    struct undostruct *first = firstRedo;

    while(first)
    {
        struct undostruct *undo = first->next;
        if(first->text) free(first->text);
        free(first);
        first = undo;
    }

    firstRedo = NULL;
}

void initEdit(void)
{
    firstUndo = NULL;
    deleteUndoList(lastUndo);
    deleteRedoList();
    undoBufChars = 0;
    undoBufIndex = 0;
    undoBufRepChars = 0;
    undoBufRepIndex = 0;

    if(clipboardSize == 0)
    {
        clipboardSize = 1024;
        clipboard = malloc(clipboardSize * sizeof(wchar_t));
    }
}

static struct undostruct *allocNewUndo(void)
{
    if(lastUndo && lastUndo->type == UNDO_ACTION_NONE) return lastUndo;

    struct undostruct *undo = malloc(sizeof(struct undostruct));
    if(!undo)
    {
        msgBox("Insufficient memory", BUTTON_OK, ERROR);
        return NULL;
    }

    undo->lineCount  = 0;
    undo->charCount  = 0;
    undo->rcharCount = 0;
    undo->rlineCount = 0;
    undo->lineStart  = -1;
    undo->charStart  = -1;
    undo->text  = NULL;
    undo->rtext = NULL;
    undo->type  = UNDO_ACTION_NONE;
    undo->next  = NULL;

    if(lastUndo)
    {
        undo->prev = lastUndo;
        lastUndo->next = undo;
        lastUndo = undo;
    }
    else
    {
        undo->prev = NULL;
        lastUndo = undo;
        firstUndo = undo;
    }

    deleteRedoList();
    return undo;
}

static struct undostruct *getLastUndo(int allocIfNull)
{
    if(lastUndo) return lastUndo;
    if(!allocIfNull) return NULL;
    return allocNewUndo();
}

static void initUndoAction(struct undostruct *undo, undoActionType utype,
                           int lwhere, int cwhere)
{
    undo->type = utype;
    undo->lineStart = lwhere;
    undo->charStart = cwhere;
}

static int copyFromBuf(wchar_t **dest, wchar_t *buf, int bcount)
{
    if(bcount == 0) return 1;
    *dest = malloc((bcount+1) * sizeof(wchar_t));
    if(!*dest) return 0;
    wcsncpy(*dest, buf, bcount);
    (*dest)[bcount] = L'\0';
    return 1;
}

static void flushUndoBuffer(struct undostruct *undo)
{
    if(undoBufIndex == 0 || undo == NULL) return;
    if(!copyFromBuf(&undo->text, undoBuf, undoBufIndex)) goto memerr;
    undo->charCount  = undoBufIndex;
    if(!copyFromBuf(&undo->rtext, undoBufRep, undoBufRepIndex)) goto memerr;
    undo->rcharCount = undoBufRepIndex;
    undoBufIndex = 0;
    undoBufChars = 0;
    undoBufRepChars = 0;
    undoBufRepIndex = 0;
    return;
    
memerr:

    msgBox("Insufficient memory", BUTTON_OK, ERROR);
}

// check if the new character to be added/deleted is continuous
// with the rest of the undo action.
static inline int isContinuous(struct undostruct *undo, int l2, int c2)
{
    int l1 = undo->lineStart;
    if(l1 != l2) return 0;
    int c1 = undo->charStart;
    if(c1 == c2 || c2 == c1-1) return 1;
    if(c2 == c1+undoBufChars) return 1;
    return 0;
}

/*
 * lwhere : line where undo action occurred
 * cwhere : char where undo action occurred
 * what   : char to be added to the undo action
 * rwhat  : replacement char (only if utype == UNDO_ACTION_REPLACE)
 */
void undoAddChar(undoActionType utype, int lwhere, int cwhere, wchar_t what)
{
    struct undostruct *undo = getLastUndo(1);

    if(undo == NULL) goto memerr;

    if(undo->type != utype || !isContinuous(undo, lwhere, cwhere) ||
       what == L'\n' || (undo->text && undo->text[0] == L'\n'))
    {
        flushUndoBuffer(undo);

        if(undo->type != UNDO_ACTION_NONE)
        {
            undo = allocNewUndo();
            if(undo == NULL) goto memerr;
        }

        if(undo->lineStart == -1)
            initUndoAction(undo, utype, lwhere, cwhere);
    }

    /*
    if(cwhere == undo->charStart)
    {
        int i;

        for(i = undoBufIndex; i > 0; i--)
            undoBuf[undoBufIndex] = undoBuf[undoBufIndex-1];
        undoBuf[0] = what;
    }
    else */ undoBuf[undoBufIndex] = what;
    //undoBuf[undoBufIndex+1] = L'\0';

    undoBufIndex++;
    undoBufChars++;

    /*
    if(what == L'\n')
    {
        flushUndoBuffer(undo);
        allocNewUndo();
    }
    */

    return;
    
memerr:

    msgBox("Insufficient memory", BUTTON_OK, ERROR);
}

// adds one wide char to the undo action.
// returns the number of chars added to the undo action.
void undoAddWChar(undoActionType utype, int lwhere, int cwhere, wchar_t *rwhat)
{
    int save = cwhere;
    wchar_t *what, c;

    if(utype == UNDO_ACTION_INSERT) what = rwhat;
    else what = lines[lwhere]->text+cwhere;

    undoAddChar(utype, lwhere, cwhere, *what);

    if(utype == UNDO_ACTION_REPLACE)
    {
        while((c = *rwhat++) != L'\0') undoBufRep[undoBufRepIndex++] = c;
    }
}

static int extendClipboardSize(int newSize)
{
    if(newSize < clipboardSize) return 1;
    clipboardSize = newSize+1;
    if(clipboard) free(clipboard);
    clipboard = malloc(clipboardSize * sizeof(wchar_t));
    if(!clipboard)
    {
        msgBox("Insufficient memory", BUTTON_OK, ERROR);
    }
}

int may_swap_selection_range(void)
{
    int swap = 0;

    if(sel_range_start.nline > sel_range_end.nline)
    {
        swap = 1;
        swap_lines();
    }
    else if(sel_range_start.nline == sel_range_end.nline &&
            sel_range_start.nchar > sel_range_end.nchar)
    {
        swap = 2;
        swap_chars();
    }

    return swap;
}

void swap_lines(void)
{
    int tmp;

    tmp = sel_range_end.nline;
    sel_range_end.nline = sel_range_start.nline;
    sel_range_start.nline = tmp;
    tmp = sel_range_end.nchar;
    sel_range_end.nchar = sel_range_start.nchar;
    sel_range_start.nchar = tmp;
}

void swap_chars(void)
{
    int tmp;

    tmp = sel_range_end.nchar;
    sel_range_end.nchar = sel_range_start.nchar;
    sel_range_start.nchar = tmp;
}

void editMenu_DeleteLine(void) 
{
    deleteLine();
}

/*****************************************
 * Toggles select mode ON/OFF. Select mode
 * is useful when running under X, to
 * emulate SHIFT-selection.
 * ***************************************/
void editMenu_ToggleSelectMode(void)
{
    SELECTING = !SELECTING;
    refreshBottomView();

    if(SELECTING) 
    {
      sel_range_start.nline = firstVisLine+selectedLine;
      sel_range_start.nchar = selectedChar;
      sel_range_end.nline = firstVisLine+selectedLine;
      sel_range_end.nchar = selectedChar;
    }
}

/*****************************************
 * Copies selection to clipboard.
 * ***************************************/
void editMenu_Copy(void)
{
    int i, j, k, l;
    int swap = 0;

    if(!SELECTING) return;
    if(!clipboard) goto memerr;

    clipboard[0] = L'\0';

    //swap the select range boundaries if needed
    swap = may_swap_selection_range();

    total_lines_in_clipboard = sel_range_end.nline-sel_range_start.nline;

    if(total_lines_in_clipboard == 0)
    {
        i = sel_range_end.nchar-sel_range_start.nchar;
        if(!extendClipboardSize(i)) goto memerr;
        wcsncpy(clipboard, lines[sel_range_start.nline]->text+j, i);
        clipboard[i] = L'\0';
    }
    else
    {
        j = sel_range_start.nchar;
        l = (lines[sel_range_start.nline]->charCount-j);
        k = sel_range_end.nchar;
        l += k;

        for(i = sel_range_start.nline+1; i < sel_range_end.nline; i++)
            l += lines[i]->charCount;

        if(!extendClipboardSize(l)) goto memerr;
        wcscpy(clipboard, lines[sel_range_start.nline]->text+j);

        for(i = sel_range_start.nline+1; i < sel_range_end.nline; i++)
            wcscat(clipboard, lines[i]->text);

        wcsncat(clipboard, lines[sel_range_end.nline]->text, k);
        clipboard[l] = L'\0';
    }

    CLIPBOARD_IS_EMPTY = 0;
  
    if(swap == 1) swap_lines();//return them back to normal
    if(swap == 2) swap_chars();//return them back to normal
    return;

memerr:

    msgBox("Insufficient memory", BUTTON_OK, ERROR);
}

/*********************************************
 * Copies selection to clipboard then cuts it.
 * *******************************************/
void editMenu_Cut(void)
{
    if(SELECTING) 
    {
        editMenu_Copy();
        remove_selected_text(1);
    }
}

/*****************************************
 * Removes the selected range of text.
 * ***************************************/
void remove_selected_text(int recordAction)
{
    int swap = may_swap_selection_range();

    int i, j, k, l;
    int refreshAll = 0;
    int first = sel_range_start.nline;
    int last  = sel_range_end.nline  ;
    int diff  = last-first;
    struct undostruct *undo = NULL;

    if(recordAction)
    {
        flushUndoBuffer(getLastUndo(0));
        undo = allocNewUndo();
        undo->type = UNDO_ACTION_DELETE;
        undo->lineStart = sel_range_start.nline;
        undo->charStart = sel_range_start.nchar;
        undo->lineCount = diff;
    }
    
    if(diff == 0)
    {
        j = sel_range_start.nchar;
        k = sel_range_end.nchar;
        i = k-j;

        if(recordAction)
        {
            undo->text = malloc((i+1) * sizeof(wchar_t));
            if(!undo->text) goto memerr;
            wcsncpy(undo->text, lines[first]->text+j, i);
            undo->text[i] = L'\0';
            undo->charCount = i;
        }

        if(lines[sel_range_start.nline]->linkedToNext) refreshAll = 1;
        copyInLine(sel_range_start.nline, j, k, 0);
        checkLineBounds(sel_range_start.nline);
    }
    else
    {
        j = sel_range_start.nchar;
        l = (lines[first]->charCount-j);
        k = sel_range_end.nchar;
        l += k;

        if(recordAction)
        {
            for(i = first+1; i < last; i++)
                l += lines[i]->charCount;

            undo->text = malloc((l+1) * sizeof(wchar_t));
            if(!undo->text) goto memerr;

            wcscpy(undo->text, lines[first]->text+j);

            for(i = first+1; i < last; i++)
                wcscat(undo->text, lines[i]->text);

            if(lines[last] && lines[last]->text)
                wcsncat(undo->text, lines[last]->text, k);
            undo->text[l] = L'\0';
            undo->charCount = l;
        }

        // handle the special case of deleting the last line
        if(first == totalLines-1 && last == first+1)
        {
            if(j == 0 && first > 0)
            {
                if(lines[first]->text) free(lines[first]->text);
                free(lines[first]);
                lines[first] = NULL;
                totalLines--;
                first--;
            }
            else
            {
                lines[first]->text[j] = '\0';
                // then we can check our line's length
                checkLineBounds(first);
            }
        }
        else
        {
            // prepare the first line to amalgamate the last line to.
            i = lines[last]->charCount-k;
            if(!extendLineText(first, j+lines[last]->charCount+2)) goto memerr;

            wcscpy(lines[first]->text+j, lines[last]->text+k);
            calcTotalCharsInLine(first);
            lines[first]->linkedToNext = lines[last]->linkedToNext;

            if(lines[first]->linkedToNext &&
               lines[first]->text[lines[first]->charCount-1] == L'\n')
            {
                lines[first]->text[--lines[first]->charCount] = L'\0';
            }

            // shift lines up by the difference between first and last lines
            move_lines_upd(first+1, last-first);

            // then we can check our line's length
            checkLineBounds(first);
        }

        refreshAll = 1;
    }

    SELECTING = 0;
    FILE_STATE = MODIFIED;
    selectedChar = sel_range_start.nchar;
    selectedCharIfPossible = selectedChar;
    refreshAll |= fixViewPostUndo(first);

    if(refreshAll) refreshView();
    else refreshSelectedLine();

    if(swap == 1) swap_lines();//return them back to normal
    if(swap == 2) swap_chars();//return them back to normal
    return;
    
memerr:

    msgBox("Insufficient memory", BUTTON_OK, ERROR);
}

static void _do_paste(wchar_t *src, int srcLineCount, int recordAction)
{
    int i = 0;
    int j = firstVisLine+selectedLine;
    int k = selectedChar;
    int l = srcLineCount;

    if((srcLineCount+totalLines) >= MAX_LINES)
    {
        msgBox("Unable to paste text. Line count will exceed the maximum of "
                tostr(MAX_LINES) ".", BUTTON_OK, ERROR);
        return;
    }

    if(recordAction)
    {
        struct undostruct *undo;

        flushUndoBuffer(getLastUndo(0));
        undo = allocNewUndo();
        undo->type = UNDO_ACTION_INSERT;
        undo->lineStart = j;
        undo->charStart = k;
        undo->lineCount = l;
        undo->text = malloc((wcslen(src)+1) * sizeof(wchar_t));

        if(!undo->text)
        {
            msgBox("Insufficient memory", BUTTON_OK, ERROR);
            return;
        }

        wcscpy(undo->text, src);
    }

    // handle the case where the line is a new last line
    if(!lines[j]) lines[j] = allocLineStructB(maxLen);

    //if pasting in the middle of a line, save the rest of the line
    int tmpLen = lines[j]->charCount-k;
    wchar_t tmp[tmpLen+1];
    int lastLineLinked = lines[j]->linkedToNext;

    if(tmpLen) wcscpy(tmp, lines[j]->text+k);
    else      tmp[0] = L'\0';

    if(*src == L'\n') l++;
    for(i = totalLines+l-1; i > j+l; i--) copyLineStruct(i, i-l);
    for( ; i > j; i--) lines[i] = allocLineStructB(maxLen);

    i = 0;
    l += j+1;
    
    wchar_t *line = lines[j]->text+k;
    wchar_t *clip = src;

    while(j <= l)
    {
        if(k >= MAX_CHARS_PER_LINE) 
        {
            //move_lines_down(totalLines, j+1);
            //srcLineCount++;
            lines[j]->linkedToNext = 1;
            *line = L'\0';
            k = 0;
            calcTotalCharsInLine(j++);

            if(lines[j] == NULL && !(lines[j] = allocLineStructB(maxLen)))
            {
                msgBox("Insufficient memory", BUTTON_OK, ERROR);
                return;
            }

            line = lines[j]->text;
            selectedLine++;
        }

        if(*clip == '\0')
        {
            *line = L'\0';
            calcTotalCharsInLine(j);
            selectedLine++;
            break;
        }
        else if(*clip == L'\n')
        {
            lines[j]->linkedToNext = 0;
            *line++ = L'\n';
            *line = L'\0';
            k = 0;
            calcTotalCharsInLine(j++);

            if(lines[j] == NULL && !(lines[j] = allocLineStructB(maxLen)))
            {
                msgBox("Insufficient memory", BUTTON_OK, ERROR);
                return;
            }

            line = lines[j]->text;
            clip++;
            selectedLine++;
        }
        else
        {
            if(*clip == L'\t')
            {
                i = TABSPACES(k+1);
                k += i;
            }
            else k++;
            *line++ = *clip++;
        }
    }

    totalLines += srcLineCount;
  
    //if there is text in tmp, append it to the last pasted line
    if(tmpLen)
    {
        i = lines[j]->charCount+tmpLen;
        lines[j]->linkedToNext = lastLineLinked;

        if(i >= maxLen)
        {
            if(!extendLineText(j, i+1))
            {
                msgBox("Insufficient memory", BUTTON_OK, ERROR);
                return;
            }
        }

        wcscat(lines[j]->text, tmp);
        checkLineBounds(j);
    }

    if(*src == L'\n')
    {
        totalLines++;
        //firstVisLine++;
    }

    /*
    //adjust the view
    if(selectedLine >= totalVisLines)
    {
        int diff = selectedLine-totalVisLines+1;
        firstVisLine += diff;
        selectedLine -= diff;
    }

    if(totalLines <= totalVisLines) 
    {
        firstVisLine = 0;
        selectedLine = totalLines-1;
    } 
    else if((totalLines-j) < totalVisLines) 
    {
        firstVisLine = totalLines-totalVisLines;
        selectedLine = totalVisLines-(totalLines-j)-1;
    }
    */

    selectedChar = k;
    //calcUserVisibleCurLine();
}

/*****************************************
 * Pastes whatever in the clipboard into
 * the current position.
 * ***************************************/
void editMenu_Paste(void)
{
    if(CLIPBOARD_IS_EMPTY) return;
    _do_paste(clipboard, total_lines_in_clipboard, 1);
    SELECTING = 0;
    FILE_STATE = MODIFIED;
    refreshView();
}

/*****************************************
 * 
 * ***************************************/
void editMenu_SelectAll(void)
{
    SELECTING = 1;
    sel_range_start.nline = 0;
    sel_range_end.nline   = totalLines-1;
    sel_range_start.nchar = 0;
    sel_range_end.nchar   = lines[totalLines-1]->charCount;

    if(sel_range_end.nchar < 0) sel_range_end.nchar = 0;

    if(totalLines <= totalVisLines) 
    {
        firstVisLine = 0;
        selectedLine = totalLines-1;
        selectedChar = lines[selectedLine]->charCount;
    } 
    else 
    {
        firstVisLine = totalLines-totalVisLines;
        selectedLine = totalVisLines-1;
        selectedChar = lines[firstVisLine+selectedLine]->charCount;
    }

    calcCharCarry(firstVisLine+selectedLine);
    calcUserVisibleCurLine();
    //SELECTING = 0;
    refreshView();
}

static int fixViewPostUndo(int first)
{
    //int refreshAll = 0;

    selectedLine = first;
    firstVisLine = 0;

    if(selectedLine >= totalVisLines)
    {
        int diff = selectedLine-totalVisLines+1;
        firstVisLine += diff;
        selectedLine -= diff;
        return 1;
    }

    /*
    if(first < firstVisLine)
    {
        selectedLine = 0;
        firstVisLine = first;
        refreshAll = 1;
    }
    else
    {
        selectedLine = first-firstVisLine;
        if(selectedLine >= totalVisLines)
        {
            firstVisLine += (selectedLine-totalVisLines+1);
            selectedLine = totalVisLines-1;
        }
    }

    // fix the view if needed
    if(firstVisLine+totalVisLines > totalLines)
    {
        if(totalLines > totalVisLines)
        {
            int i = firstVisLine;
            firstVisLine = totalLines-totalVisLines;
            selectedLine += (firstVisLine-i);
            refreshAll = 1;
        }
    }
    */

    selectedCharIfPossible = selectedChar;
    calcUserVisibleCurLine();

    //return refreshAll;
    return 0;
}

/************************************************
 * Undo the last action done. Last action can be:
 * UNDO_ACTION_REPLACE: The user replaced a
 *                      character with INSERT on
 * UNDO_ACTION_INSERT: The user typed regularly
 * UNDO_ACTION_DELETE: The user deleted using
 *                     DEL or BACKSPACE
 * **********************************************/
void editMenu_Undo(void)
{
    struct undostruct *undo = getLastUndo(0);
    if(undo == NULL) return;

    flushUndoBuffer(undo);

    int first = undo->lineStart;
    int i = undo->charCount;
    int j = undo->rcharCount;
    int k, l;
    
    if(undo->type == UNDO_ACTION_REPLACE)
    {
        k = undo->charStart;

        if(undo->lineCount == 0)
        {
            if(i > j)
            {
                l = i-j;
                if(!extendLineText(first, lines[first]->charCount+l+1)) 
                    goto memerr;
                copyInLine(first, k+i, k+j, 0);
                wcsncpy(lines[first]->text+k, undo->text, i);
            }
            else if(i < j)
            {
                wcsncpy(lines[first]->text+k, undo->text, i);
                copyInLine(first, k+i, k+j, 0);
            }
            else
            {
                wcsncpy(lines[first]->text+k, undo->text, i);
            }

            selectedChar = undo->charStart;
            checkLineBounds(first);
            calcCharCarry(first);
        }
        else
        {
            // amalgamate first line (after first char) to 
            // last line (before last char).
            wchar_t *s = wcsrchr(undo->rtext, L'\n');
            if(!s) return;
            k += wcslen(s);
            if(!extendLineText(first, k+1)) goto memerr;
            wcscat(lines[first]->text, s+1);
            lines[first]->linkedToNext = 
                        lines[first+undo->lineCount]->linkedToNext;
            move_lines_upd(first+1, undo->lineCount);
            checkLineBounds(first);

            // then paste our text
            firstVisLine = undo->lineStart;
            selectedLine = 0;
            selectedChar = undo->charStart;
            _do_paste(undo->text, undo->lineCount, 0);
        }
    } else if(undo->type == UNDO_ACTION_INSERT) {
        sel_range_start.nline = undo->lineStart;
        sel_range_end.nline   = undo->lineStart+undo->lineCount;
        sel_range_start.nchar = undo->charStart;
        wchar_t *s = wcsrchr(undo->text, L'\n');
        if(!s) s = undo->text-1;
        i = wcslen(s+1);
        sel_range_end.nchar = i;
        
        if(sel_range_start.nline == sel_range_end.nline)
        {
            if(undo->text[0] == L'\n') sel_range_end.nline++;
            else sel_range_end.nchar += undo->charStart;
        }
        remove_selected_text(0);
    } else if(undo->type == UNDO_ACTION_DELETE) {
        if(undo->text[0] == L'\n' && undo->text[1] == L'\0') undo->lineCount = 1;
        // deletion was in one line
        if(undo->lineCount == 0)
        {
            k = lines[first]->charCount+i;
            if(!extendLineText(first, k+1)) goto memerr;
            copyInLine(first, undo->charStart+i, undo->charStart, 0);
            wcsncpy(lines[first]->text+undo->charStart, undo->text, i);
            selectedChar = undo->charStart+i;
            checkLineBounds(first);
            calcCharCarry(first);
        }
        else
        {
            firstVisLine = undo->lineStart;
            selectedLine = 0;
            selectedChar = undo->charStart;
            _do_paste(undo->text, undo->lineCount, 0);
        }
    } else {
        return;
    }
    
    fixViewPostUndo(undo->lineStart);
    firstRedo = undo;
    lastUndo = undo->prev;
    calcCharCarry(undo->lineStart);
    refreshView();
    return;

memerr:

    msgBox("Insufficient memory", BUTTON_OK, ERROR);
}

/************************************************
 * Redo the last action done. Last action can be:
 * UNDO_ACTION_REPLACE: The user replaced a
 *             character with INSERT on
 * UNDO_ACTION_INSERT: The user typed regularly
 * UNDO_ACTION_DELETE: The user deleted using
 *             DEL or BACKSPACE
 * **********************************************/
void editMenu_Redo(void)
{
    struct undostruct *undo = firstRedo;
    if(undo == NULL) return;

    int first = undo->lineStart;
    int last = first + undo->lineCount;
    int i = undo->rcharCount;
    int j = undo->charCount;
    int k, l;
    
    if(undo->type == UNDO_ACTION_REPLACE)
    {
        k = undo->charStart;

        if(undo->lineCount == 0)
        {
            if(i > j)
            {
                l = i-j;
                if(!extendLineText(first, lines[first]->charCount+l+1)) 
                    goto memerr;
                copyInLine(first, k+i, k+j, 0);
                wcsncpy(lines[first]->text+k, undo->rtext, i);
            }
            else if(i < j)
            {
                wcsncpy(lines[first]->text+k, undo->rtext, i);
                copyInLine(first, k+i, k+j, 0);
            }
            else
            {
                wcsncpy(lines[first]->text+k, undo->rtext, i);
            }
            selectedChar = undo->charStart+undo->rcharCount;
            checkLineBounds(first);
            calcCharCarry(first);
        }
        else
        {
            // amalgamate first line (after first char) to 
            // last line (before last char).
            wchar_t *s = wcsrchr(undo->text, L'\n');
            if(!s) return;
            k += wcslen(s);
            if(!extendLineText(first, k+1)) goto memerr;
            wcscat(lines[first]->text, s+1);
            lines[first]->linkedToNext = 
                            lines[first+undo->lineCount]->linkedToNext;
            move_lines_upd(first+1, undo->lineCount);
            checkLineBounds(first);

            // then paste our text
            firstVisLine = undo->lineStart;
            selectedLine = 0;
            selectedChar = undo->charStart;
            _do_paste(undo->rtext, undo->lineCount, 0);
        }
    } else if(undo->type == UNDO_ACTION_INSERT) {
        // insertion was in one line
        if(undo->lineCount == 0 && undo->text[0] != L'\n')
        {
            k = lines[first]->charCount+j;
            if(!extendLineText(first, k+1)) goto memerr;
            i = undo->charStart;
            copyInLine(first, i+j, i, 0);
            wcsncpy(lines[first]->text+i, undo->text, j);
            selectedChar = i+j;
            checkLineBounds(first);
            calcCharCarry(first);
        }
        else
        {
            firstVisLine = undo->lineStart;
            selectedLine = 0;
            selectedChar = undo->charStart;
            _do_paste(undo->text, undo->lineCount, 0);
            if(undo->text[0] == L'\n') last++;
        }
    } else if(undo->type == UNDO_ACTION_DELETE) {
        sel_range_start.nline = undo->lineStart;
        sel_range_end.nline   = undo->lineStart+undo->lineCount;
        sel_range_start.nchar = undo->charStart;
        wchar_t *s = wcsrchr(undo->text, L'\n');
        i = s ? wcslen(s+1) : 0;
        //if(!s) s = undo->text-1;
        //i = wcslen(s+1);
        sel_range_end.nchar = i;
        if(sel_range_start.nline == sel_range_end.nline)
            sel_range_end.nchar += undo->charStart;
        remove_selected_text(0);
        last = first;
    } else {
        return;
    }
    
    fixViewPostUndo(last);
    firstRedo = undo->next;
    lastUndo = undo;
    calcCharCarry(last);
    refreshView();
    return;

memerr:

    msgBox("Insufficient memory", BUTTON_OK, ERROR);
}


/*****************************************
 * Finds a string text in the file.
 * ***************************************/
void editMenu_Find(void)
{
    char *c;
    int i;
    wchar_t *f = getUserInputWC(L"Enter text to find:  ", L" Find ");

    if(!f)
    {
        refreshView();
        return;
    }
  
    _find(f);
  
    if(!total_find_results) 
    {
        free(f);
        msgBox("No matches were found.", BUTTON_OK, INFO);
        refreshView();
        return;
    }
  
    //infinite loop to get user input
    i = 0;

    while(1) 
    {
        int x = find_result_pos[i].nline-firstVisLine;

        if(x < 0)
        {
            firstVisLine += x;
            selectedLine = 0;
        }
        else
        {
            if(x >= totalVisLines)
            {
                x -= totalVisLines;
                firstVisLine += (x+1);
                selectedLine = totalVisLines-1;
            }
            else selectedLine = x;
        }

        selectedChar = find_result_pos[i].nchar;
        selectedCharIfPossible = selectedChar;
        calcCharCarry(find_result_pos[i].nline);
        calcUserVisibleCurLine();
        refreshView();
        setScreenColorsI(COLOR_STATUS_BAR);

        if(GNU_DOS_LEVEL > 2)
            mvprintw(SCREEN_H-1, 0,
                     "Find(%d/%d): [C-p] Prev [C-n] Next [C-g] Cancel",
                     i+1, total_find_results);
        else if(GNU_DOS_LEVEL > 1)
            mvprintw(SCREEN_H-1, 0,
                     "Find(%d/%d): [C-p] Prev [C-n] Next [ESC] Cancel",
                     i+1, total_find_results);
        else
            mvprintw(SCREEN_H-1, 0,
                     "Find(%d/%d): [Up] Prev [Down] Next [ESC] Cancel",
                     i+1, total_find_results);

        move(selectedLine+2, selectedChar+1+selectedCharCarry);
        refresh();

        c = ugetKey();

        switch(c[0]) 
        {
            case('p'):
                if(GNU_DOS_LEVEL < 2) break;
                if(!CTRL) break;
                // Up - GNU key binding
                if(i <= 0) i = total_find_results-1;
                else i--;
                break;

            case(UP_KEY):
                if(GNU_DOS_LEVEL > 1) break;
                if(i <= 0) i = total_find_results-1;
                else i--;
                break;

            case('n'):
                if(GNU_DOS_LEVEL < 2) break;
                if(!CTRL) break;
                // Down - GNU key binding
                if(i >= total_find_results-1) i = 0;
                else i++;
                break;

            case(DOWN_KEY):
                if(GNU_DOS_LEVEL > 1) break;

            case(ENTER_KEY):
            case(SPACE_KEY):
                if(i >= total_find_results-1) i = 0;
                else i++;
                break;

            case('g'):
                if(GNU_DOS_LEVEL < 3) break;
                if(!CTRL) break;
                // ESC - GNU key binding
                refreshView();
                free(f);
                return;

            case(ESC_KEY):
                if(GNU_DOS_LEVEL > 2) break;
                refreshView();
                free(f);
                return;

            default:
                break;
        }
    }
}


/*****************************************
 * Replaces Find text with Replace text.
 * ***************************************/
void editMenu_Replace(void)
{
    char *c;
    int i;
    wchar_t *f = getUserInputWC(L"Enter text to find:  ", L" Find ");

    if(!f)
    {
        refreshView();
        return;
    }
  
    wchar_t *r = getUserInputWC(L"Enter replacement text: ", L" Replace ");

    if(!r)
    {
        free(f);
        refreshView();
        return;
    }
  
    _find(f);
  
    if(!total_find_results) 
    {
        free(f);
        free(r);
        msgBox("No matches were found.", BUTTON_OK, INFO);
        refreshView();
        return;
    }
  
    //infinite loop to get user input
    i = 0;

    while(1)
    {
        int x = find_result_pos[i].nline-firstVisLine;

        if(x < 0)
        {
            firstVisLine += x;
            selectedLine = 0;
        }
        else
        {
            if(x >= totalVisLines)
            {
                x -= totalVisLines;
                firstVisLine += (x+1);
                selectedLine = totalVisLines-1;
            }
            else selectedLine = x;
        }

        selectedChar = find_result_pos[i].nchar;
        selectedCharIfPossible = selectedChar;
        calcCharCarry(find_result_pos[i].nline);
        calcUserVisibleCurLine();
        refreshView();
        setScreenColorsI(COLOR_STATUS_BAR);

        if(GNU_DOS_LEVEL > 2)
            mvprintw(SCREEN_H-1, 0,
                     "Find(%d/%d): [ENTER] Replace [A] Replace All [C-g] Cancel",
                     i+1, total_find_results);
        else
            mvprintw(SCREEN_H-1, 0,
                     "Find(%d/%d): [ENTER] Replace [A] Replace All [ESC] Cancel",
                     i+1, total_find_results);

        move(selectedLine+2, selectedChar+1+selectedCharCarry);
        refresh();

get_key:
        c = ugetKey();

        switch(c[0]) 
        {
            case('p'):
                if(GNU_DOS_LEVEL < 2) break;
                if(!CTRL) break;
                // Up - GNU key binding
                if(i <= 0) i = total_find_results-1;
                else i--;
                break;

            case(UP_KEY):
                if(GNU_DOS_LEVEL > 1) break;
                if(i <= 0) i = total_find_results-1;
                else i--;
                break;

            case('n'):
                if(GNU_DOS_LEVEL < 2) break;
                if(!CTRL) break;
                // Down - GNU key binding
                if(i >= total_find_results-1) i = 0;
                else i++;
                break;

            case(DOWN_KEY):
                if(GNU_DOS_LEVEL > 1) break;
                if(i >= total_find_results-1) i = 0;
                else i++;
                break;

            case('g'):
                if(GNU_DOS_LEVEL < 3) break;
                if(!CTRL) break;
                // ESC - GNU key binding
                goto finish;

            case(ESC_KEY):
                if(GNU_DOS_LEVEL > 2) break;
                goto finish;

            case(SPACE_KEY):
            case(ENTER_KEY):
                _replace(i, f, r);
                if(i >= total_find_results) i--;
                if(total_find_results <= 0) goto finish;
                break;

            case('a'):
                _replace(-1, f, r);
                total_find_results = 0;
                goto finish;
                break;

            default:
                goto get_key;
                break;
        }
    }
    
finish:

    free(f);
    free(r);
    refreshView();
}

/*******************************************
 * Function is called by minoMenu_Replace()
 * to replace find result number 'pos' with
 * the replacement text.
 * *****************************************/
void _replace(int pos, wchar_t *f, wchar_t *r)
{
    //pass position to replace in find_result_pos[] array,
    //or -1 to replace all find results.
    int i = wcslen(f);
    int j = wcslen(r);
  
    int old_firstVisLine = firstVisLine;
    int old_selectedLine = selectedLine;
    int old_selectedChar = selectedChar;
    int old_selectedCharCarry = selectedCharCarry;

    if(pos >= 0) 
    {
        _do_replace(pos, f, r);
        FILE_STATE = MODIFIED;
    }
    else
    {    //-1 means replace all
        if(i == j)
        {
            for(pos = 0; pos < total_find_results; pos++)
            {
                _do_replace(pos, f, r);
            }
        }
        else
        {
            while(total_find_results)
            {
                _do_replace(0, f, r);
            }
        }
    }

    firstVisLine = old_firstVisLine;
    selectedLine = old_selectedLine;
    selectedChar = old_selectedChar;
    selectedCharCarry = old_selectedCharCarry;
}

void _find(wchar_t *f)
{
    int i, k = 0;
    wchar_t *j;
    int flen = wcslen(f);

    total_find_results = 0;

    for(i = 0; i < totalLines; i++)
    {
        wchar_t *line = lines[i]->text;

        while((j = wcscasestr(line, f)))
        {
            find_result_pos[k].nline = i; 
            find_result_pos[k++].nchar = (j-lines[i]->text);
            total_find_results++;
            line = j+flen;
        }
    }
}

void _do_replace(int pos, wchar_t *f, wchar_t *r)
{
    int i = wcslen(f);
    int j = wcslen(r);
    int k, l, n, m;
    wchar_t *f2, c;

    k = find_result_pos[pos].nchar;
    l = find_result_pos[pos].nline;
    n = 0;
    firstVisLine = l;
    selectedLine = 0;
    selectedChar = k;
    k = selectedChar;

    // add the original text to the undo buffer
    f2 = f;
    while(*f2 != L'\0')
    {
        undoAddChar(UNDO_ACTION_REPLACE, l, k, *f2);
        k++; f2++;
    }

    k = selectedChar;

    // then add the replacement text
    f2 = r;
    while((c = *f2++) != L'\0') undoBufRep[undoBufRepIndex++] = c;
    
    if(i == j)
    {
        /*
         * find & replace of same length
         */
        wcsncpy(lines[l]->text+k, r, j);
    }
    else if(i > j)
    {
        /*
         * find is longer than replace
         */
        wcsncpy(lines[l]->text+k, r, j);
        copyInLine(l, k+j, k+i, 0);
    }
    else
    {
        /*
         * replace is longer than find
         */
        if(!extendLineText(l, lines[l]->charCount+j-i+1))
        {
            msgBox("Insufficient memory!", BUTTON_OK, ERROR);
            return;
        }

        copyInLine(l, k+j, k+i, 0);
        wcsncpy(lines[l]->text+k, r, j);
    }

    FILE_STATE = MODIFIED;
    checkLineBounds(l);
    /* we will need to update search results */
    _find(f);
}

void calcTotalCharsInLineC(int pos, int *carry)
{
    int totalCharsInLine = 0;
    int k;
    wchar_t *c = lines[pos]->text;

    *carry = 0;

    while(*c != L'\0')
    {
        if(*c == L'\r' || *c == L'\n') break;
        else if(*c == L'\t')
        {
            k = TABSPACES(totalCharsInLine+(*carry)+1);
            (*carry) += k;
        }

        totalCharsInLine++;
        c++;
    }
    lines[pos]->charCount = totalCharsInLine;
}

void calcTotalCharsInLine(int pos)
{
    lines[pos]->charCount = wcslen(lines[pos]->text);
}

