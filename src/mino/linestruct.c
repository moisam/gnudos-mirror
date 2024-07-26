/* 
 *    Programmed By: Mohammed Isam Mohammed [mohammed_isam1984@yahoo.com]
 *    Copyright 2018, 2024 (c)
 * 
 *    file: linestruct.c
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

#include <stdlib.h>
#include <string.h>
#include "defs.h"
#include "dialogs.h"

struct linestruct *lines[MAX_LINES];


struct linestruct *allocLineStruct(void)
{
    struct linestruct *line = malloc(sizeof(struct linestruct));
    if(!line) { msgBox("Insufficient memory", BUTTON_OK, ERROR); return NULL; }
    //memset((void *)line, 0, sizeof(struct linestruct));
    line->charCount = 0;
    line->charsAlloced = 0;
    line->linkedToNext = 0;
    line->text = NULL;
    return line;
}

struct linestruct *allocLineStructB(int chars)
{
    struct linestruct *line = allocLineStruct();

    if(line)
    {
        line->text = malloc((chars+1) * sizeof(wchar_t));

        if(!line->text)
        {
            msgBox("Insufficient memory", BUTTON_OK, ERROR);
            free(line);
            return NULL;
        }

        line->text[0] = L'\0';
        line->charCount = 0;
        line->linkedToNext = 0;
        line->charsAlloced = chars+1;
    }

    return line;
}


void copyLineStruct(int pos1, int pos2)
{
    struct linestruct *line1 = lines[pos1];
    struct linestruct *line2 = lines[pos2];

    if(!line2) return;
    if(!line1)
    {
        line1 = allocLineStruct();
        if(!line1) return;
        lines[pos1] = line1;
    }

    if(line1->text) free(line1->text);
    memcpy(line1, line2, sizeof(struct linestruct));
    line2->text = NULL;
}

void freeLineStruct(struct linestruct *line)
{
    if(!line) return;
    if(line->text) free(line->text);
    free(line);
}

