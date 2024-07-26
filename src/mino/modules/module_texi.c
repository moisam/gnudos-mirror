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

char *ext_texi[] = { "texi", "txi", "texinfo" };
wchar_t *keyword_texi[] = {
      //TEXI keywords
      L"@!",
      L"@\"",
      L"@'",
      L"@*",
      L"@,",
      L"@-",
      L"@.",
      L"@/",
      L"@:",
      L"@=",
      L"@?",
      L"@@",
      L"@atchar",
      L"@\\",
      L"@^",
      L"@`",
      L"@{",
      L"@lbracechar",
      L"@}",
      L"@rbracechar",
      L"@~",
      L"@AA",
      L"@aa",
      L"@abbr",
      L"@acronym",
      L"@AE",
      L"@ae",
      L"@afivepaper",
      L"@afourlatex",
      L"@afourpaper",
      L"@afourwide",
      L"@alias",
      L"@allowcodebreaks",
      L"@anchor",
      L"@appendixsection",
      L"@appendixsec",
      L"@appendixsubsec",
      L"@appendixsubsubsec",
      L"@appendix",
      L"@arrow",
      L"@asis",
      L"@author",
      L"@bullet",
      L"@bye",
      L"@b",
      L"@caption",
      L"@cartouche",
      L"@centerchap",
      L"@center",
      L"@chapheading",
      L"@chapter",
      L"@cindex",
      L"@cite",
      L"@clear",
      L"@clicksequence",
      L"@clickstyle",
      L"@click",
      L"@codequotebacktick",
      L"@codequoteundirected",
      L"@code",
      L"@command",
      L"@comma",
      L"@comment",
      L"@contents",
      L"@copying",
      L"@copyright",
      L"@c",
      L"@defcodeindex",
      L"@defcvx",
      L"@defcv",
      L"@deffnx",
      L"@deffn",
      L"@defindex",
      L"@definfoenclose",
      L"@defivarx",
      L"@defivar",
      L"@defmacx",
      L"@defmac",
      L"@defmethodx",
      L"@defmethod",
      L"@defoptx",
      L"@defopt",
      L"@defopx",
      L"@defop",
      L"@defspecx",
      L"@defspec",
      L"@deftpx",
      L"@deftp",
      L"@deftypecvx",
      L"@deftypecv",
      L"@deftypefnnewline",
      L"@deftypefnx",
      L"@deftypefn",
      L"@deftypefunx",
      L"@deftypefun",
      L"@deftypeivarx",
      L"@deftypeivar",
      L"@deftypemethodx",
      L"@deftypemethod",
      L"@deftypeopx",
      L"@deftypeop",
      L"@deftypevarx",
      L"@deftypevar",
      L"@deftypevrx",
      L"@deftypevr",
      L"@defunx",
      L"@defun",
      L"@defvarx",
      L"@defvar",
      L"@defvrx",
      L"@defvr",
      L"@detailmenu",
      L"@dfn",
      L"@DH",
      L"@dh",
      L"@dircategory",
      L"@direntry",
      L"@display",
      L"@dmn",
      L"@docbook",
      L"@documentdescription",
      L"@documentlanguage",
      L"@dotaccent",
      L"@dotless",
      L"@dots",
      L"@email",
      L"@emph",
      L"@enddots",
      L"@end",
      L"@enumerate",
      L"@env",
      L"@equiv",
      L"@errormsg",
      L"@error",
      L"@euro",
      L"@evenfooting",
      L"@evenheading",
      L"@exampleindent",
      L"@example",
      L"@exclamdown",
      L"@exdent",
      L"@expansion",
      L"@file",
      L"@finalout",
      L"@findex",
      L"@firstparagraphindent",
      L"@float",
      L"@flushleft",
      L"@flushright",
      L"@fonttextsize",
      L"@footnotestyle",
      L"@footnote",
      L"@format",
      L"@frenchspacing",
      L"@ftable",
      L"@geq",
      L"@group",
      L"@guillemetleft",
      L"@guillemetright",
      L"@guillemotleft",
      L"@guillemotright",
      L"@guilsinglleft",
      L"@guilsinglright",
      L"@H",
      L"@hashchar",
      L"@headings",
      L"@heading",
      L"@headitemfont",
      L"@headitem",
      L"@html",
      L"@hyphenation",
      L"@ifcommanddefined",
      L"@ifcommandnotdefined",
      L"@ifdocbook",
      L"@ifhtml",
      L"@ifinfo",
      L"@ifnotdocbook",
      L"@ifnothtml",
      L"@ifnotplaintext",
      L"@ifnottex",
      L"@ifnotxml",
      L"@ifnotinfo",
      L"@ifplaintext",
      L"@ifset",
      L"@iftex",
      L"@ifxml",
      L"@ignore",
      L"@image",
      L"@include",
      L"@indentedblock",
      L"@indent",
      L"@indicateurl",
      L"@inforef",
      L"@inlinefmtifelse",
      L"@inlinefmt",
      L"@inlineifclear",
      L"@inlineifset",
      L"@inlineraw",
      L"\\input",
      L"@insertcopying",
      L"@itemize",
      L"@itemx",
      L"@item",
      L"@i",
      L"@kbdinputstyle",
      L"@kbd",
      L"@key",
      L"@kindex",
      L"@LaTeX",
      L"@leq",
      L"@lisp",
      L"@listoffloats",
      L"@lowersections",
      L"@L",
      L"@l",
      L"@macro",
      L"@majorheading",
      L"@math",
      L"@menu",
      L"@minus",
      L"@multitable",
      L"@need",
      L"@node",
      L"@noindent",
      L"@novalidate",
      L"@oddfooting",
      L"@oddheading",
      L"@OE",
      L"@oe",
      L"@ogonek",
      L"@option",
      L"@ordf",
      L"@ordm",
      L"@O",
      L"@o",
      L"@pagesizes",
      L"@page",
      L"@paragraphindent",
      L"@part",
      L"@pindex",
      L"@point",
      L"@pounds",
      L"@printindex",
      L"@print",
      L"@pxref",
      L"@questiondown",
      L"@quotation",
      L"@quotedblleft",
      L"@quotedblright",
      L"@quoteleft",
      L"@quoteright",
      L"@quotedblbase",
      L"@quotesinglbase",
      L"@raggedright",
      L"@raisesections",
      L"@refill",
      L"@ref",
      L"@registeredsymbol",
      L"@result",
      L"@ringaccent",
      L"@r",
      L"@samp",
      L"@sansserif",
      L"@sc",
      L"@section",
      L"@setchapternewpage",
      L"@setcontentsaftertitlepage",
      L"@setfilename",
      L"@setshortcontentsaftertitlepage",
      L"@settitle",
      L"@set",
      L"@shortcaption",
      L"@shortcontents",
      L"@shorttitlepage",
      L"@slanted",
      L"@smallbook",
      L"@smalldisplay",
      L"@smallexample",
      L"@smallformat",
      L"@smallindentedblock",
      L"@smalllisp",
      L"@smallquotation",
      L"@sp",
      L"@ss",
      L"@strong",
      L"@subheading",
      L"@subsection",
      L"@subsubheading",
      L"@subsubsection",
      L"@subtitle",
      L"@summarycontents",
      L"@syncodeindex",
      L"@synindex",
      L"@table",
      L"@tab",
      L"@textdegree",
      L"@TeX",
      L"@tex",
      L"@thischapter",
      L"@thischaptername",
      L"@thischapternum",
      L"@thisfile",
      L"@thispage",
      L"@thistitle",
      L"@TH",
      L"@th",
      L"@tieaccent",
      L"@tie",
      L"@tindex",
      L"@titlefont",
      L"@titlepage",
      L"@title",
      L"@today",
      L"@top",
      L"@t",
      L"@ubaraccent",
      L"@udotaccent",
      L"@unmacro",
      L"@unnumberedsec",
      L"@unnumberedsubsec",
      L"@unnumberedsubsubsec",
      L"@unnumbered",
      L"@url",
      L"@urefbreakstyle",
      L"@uref",
      L"@u",
      L"@value",
      L"@var",
      L"@verbatiminclude",
      L"@verbatim",
      L"@verb",
      L"@vindex",
      L"@vskip",
      L"@vtable",
      L"@v",
      L"@w",
      L"@xml",
      L"@xrefautomaticsectiontitle",
      L"@xref",
      L"@",
};

struct modulestruct module_texi =
{
    .extsCount      = 3,
    .exts           = ext_texi,
    .keywordCount   = 340,
    .keywords       = keyword_texi,
    .mlCommentStart = NULL,
    .mlCommentEnd   = NULL,
    .slCommentStart = L"@comment",
    .caseSensitive  = 1,
};

