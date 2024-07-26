/* 
 *    Programmed By: Mohammed Isam Mohammed [mohammed_isam1984@yahoo.com]
 *    Copyright 2014, 2015, 2016, 2017, 2018, 2024 (c)
 * 
 *    file: options.h
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
#ifndef __OPTIONS_H
#define __OPTIONS_H

#include <wchar.h>
#include "defs.h"

//special colors for syntax highlighting
#define COLOR_HCOMMENT                  36
#define COLOR_HKEYWORD                  33
#define COLOR_HSTRING                   31
#define COLOR_HDIRECTIVE                35
#define COLOR_HPARAMETERS               36
#define COLOR_HBRACES                   32
#define COLOR_HWINDOW                   40

//different highlight modes according to opened file
typedef enum 
{
    C_MODE,
    CPP_MODE,
    PERL_MODE,
    SHELL_MODE,
    TEXI_MODE,
    ASM_MODE,
    PYTHON_MODE,
    HTML_MODE,
    JAVASCRIPT_MODE,
    BASIC_MODE,
    PASCAL_MODE,
    F77_MODE,
    NO_MODE,
} hmode;

extern int old_window_color;

//determine if automatic highlighting is on/off
extern int AUTO_HIGHLIGHTING;
extern hmode HIGHLIGHT_MODE;

//switch auto-indentation on/off
extern int  AUTO_INDENT;

//the string holding the spaces/tabes to indent new lines
extern wchar_t *autoIndentStr;


void optionsMenu_Change_Colors(void);
void optionsMenu_Tab_Spaces(void);
void optionsMenu_Reset_Config(void);
void optionsMenu_Autoindent(void);

void write_config_file(void);
void _write_config_file(FILE *file, int write_defaults);

#endif
