/* 
 *    Programmed By: Mohammed Isam Mohammed [mohammed_isam1984@yahoo.com]
 *    Copyright 2014, 2015, 2016, 2017, 2018, 2024 (c)
 * 
 *    file: edit.h
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

#ifndef __EDIT_H
#define __EDIT_H

#include <wchar.h>

typedef enum { 
    UNDO_ACTION_INSERT, 
    UNDO_ACTION_DELETE,
    UNDO_ACTION_REPLACE,
    UNDO_ACTION_NONE,
} undoActionType;

typedef struct
{ //structure defining a point by its position in
    int nline;     //line number (zero-based), and
    int nchar;     //char number (zero-based)
} pos_t;

struct undostruct
{
    undoActionType type;
    int   lineStart,  charStart;
    int   lineCount,  charCount;
    int  rlineCount, rcharCount;
    wchar_t *text;
    wchar_t *rtext;
    struct undostruct *prev, *next;
};

/* 
 * boolean telling if we are in selecting mode 
 * (i.e. using SHIFT+Arrow to select) 
 */
extern int SELECTING;
extern int CLIPBOARD_IS_EMPTY;
extern int total_lines_in_clipboard;
extern wchar_t *clipboard;

extern pos_t sel_range_start;//start point of selected text
extern pos_t sel_range_end;  //end point of selected text


/* functions of Edit menu */
void editMenu_Cut(void);
void editMenu_Copy(void);
void editMenu_Paste(void);
void editMenu_Undo(void);
void editMenu_Redo(void);
void editMenu_SelectAll(void);
void editMenu_DeleteLine(void);
void editMenu_Find(void);
void editMenu_Replace(void);
void editMenu_ToggleSelectMode(void);
void remove_selected_text(int recordAction);
void swap_lines(void);
void swap_chars(void);

// edit.c
void initEdit(void);
void undoAddChar(undoActionType utype, int lwhere, int cwhere, wchar_t what);
void  undoAddWChar(undoActionType utype, int lwhere, int cwhere, wchar_t *rwhat);
int may_swap_selection_range(void);
void calcTotalCharsInLine(int pos);
void calcTotalCharsInLineC(int pos, int *carry);

#endif
