/* 
 *    Programmed By: Mohammed Isam Mohammed [mohammed_isam1984@yahoo.com]
 *    Copyright 2018, 2024 (c)
 * 
 *    file: module_sh.c
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

char *ext_sh[] = { "sh" };
wchar_t *keyword_sh[] = {
      //shell keywords
      L"!",
      L"case",
      L"do",
      L"done",
      L"elif",
      L"else",
      L"esac",
      L"fi",
      L"for",
      L"function",
      L"if",
      L"in",
      L"select",
      L"then",
      L"until",
      L"while",
      L"time",
      L"[[",
      L"]]",
      //shell builtin functions
      L".",
      L":",
      L"[",
      L"alias",
      L"bg",
      L"bind",
      L"break",
      L"builtin",
      L"caller",
      L"cd",
      L"command",
      L"compgen",
      L"complete",
      L"compopt",
      L"continue",
      L"declare",
      L"dirs",
      L"disown",
      L"echo",
      L"enable",
      L"eval",
      L"exec",
      L"exit",
      L"export",
      L"false",
      L"fc",
      L"fg",
      L"getopts",
      L"hash",
      L"help",
      L"history",
      L"jobs",
      L"kill",
      L"let",
      L"local",
      L"logout",
      L"mapfile",
      L"popd",
      L"printf",
      L"pushd",
      L"pwd",
      L"read",
      L"readarray",
      L"readonly",
      L"return",
      L"set",
      L"shift",
      L"shopt",
      L"source",
      L"suspend",
      L"test",
      L"times",
      L"trap",
      L"true",
      L"type",
      L"typeset",
      L"ulimit",
      L"umask",
      L"unalias",
      L"unset",
      L"wait",
      L"{",
      L"}",
};

struct modulestruct module_sh =
{
    .extsCount      = 1,
    .exts           = ext_sh,
    .keywordCount   = 82,
    .keywords       = keyword_sh,
    .mlCommentStart = NULL,
    .mlCommentEnd   = NULL,
    .slCommentStart = L"#",
    .caseSensitive  = 1,
};

