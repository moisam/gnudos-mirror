/* 
 *    Programmed By: Mohammed Isam Mohammed [mohammed_isam1984@yahoo.com]
 *    Copyright 2018, 2024 (c)
 * 
 *    file: module_py.c
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
#include "modules.h"

char *ext_py[] = { "py", "PY" };
wchar_t *keyword_py[] = {
      //Python language keywords
      L"print",
      L"while",
      L"for",
      L"break",
      L"continue",
      L"if",
      L"elif",
      L"else",
      L"is",
      L"not",
      L"and",
      L"or",
      L"import",
      L"as",
      L"from",
      L"def",
      L"return",
      L"lambda",
      L"global",
      L"try",
      L"except",
      L"finally",
      L"raise",
      L"del",
      L"pass",
      L"assert",
      L"class",
      L"exec",
      L"yield",
};

struct modulestruct module_py =
{
    .extsCount      = 2,
    .exts           = ext_py,
    .keywordCount   = 29,
    .keywords       = keyword_py,
    .mlCommentStart = L"'''",
    .mlCommentEnd   = L"'''",
    .slCommentStart = L"#",
    .caseSensitive  = 1,
};

