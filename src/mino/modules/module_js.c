/* 
 *    Programmed By: Mohammed Isam Mohammed [mohammed_isam1984@yahoo.com]
 *    Copyright 2018, 2024 (c)
 * 
 *    file: module_js.c
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

char *ext_js[] = { "js", "JS" };
wchar_t *keyword_js[] = {
      //JavaScript language keywords
      //As in version 5 (ECMAScript 5)
      L"break",
      L"const",
      L"continue",
      L"delete",
      L"double",
      L"while",
      L"export",
      L"for",
      L"int",
      L"function",
      L"if",
      L"else",
      L"import",
      L"instanceof",
      L"label",
      L"let",
      L"new",
      L"return",
      L"switch",
      L"this",
      L"throws",
      L"try",
      L"catch",
      L"typeof",
      L"var",
      L"void",
      L"with",
      L"yield",
      ///////////////////////////////////
      L"abstract",
      L"arguments",
      L"boolean",
      L"byte",
      L"case",
      L"catch",
      L"char",
      L"class",
      L"debugger",
      L"default",
      L"do",
      L"else",
      L"enum",
      L"eval",
      L"extends",
      L"false",
      L"finally",
      L"final",
      L"float",
      L"goto",
      L"implements",
      L"interface",
      L"in",
      L"long",
      L"native",
      L"null",
      L"package",
      L"private",
      L"protected",
      L"public",
      L"short",
      L"static",
      L"super",
      L"synchronized",
      L"throw",
      L"transient",
      L"true",
      L"volatile",
      ///////////////////////////////////
      L"array",
      L"date",
      L"hasownproperty",
      L"infinity",
      L"isfinite",
      L"isnan",
      L"isprototypeof",
      L"length",
      L"math",
      L"nan",
      L"name",
      L"number",
      L"object",
      L"prototype",
      L"string",
      L"tostring",
      L"undefined",
      L"valueof",
};

struct modulestruct module_js =
{
    .extsCount      = 2,
    .exts           = ext_js,
    .keywordCount   = 84,
    .keywords       = keyword_js,
    .mlCommentStart = L"/*",
    .mlCommentEnd   = L"*/",
    .slCommentStart = L"//",
    .caseSensitive  = 1,
};

