# 
#    Copyright 2013, 2014 2015, 2016, 2017, 2018, 2024 (c)
#    Mohammed Isam Mohammed [mohammed_isam1984@yahoo.com]
# 
#    file: configure.ac
#    This file is part of GnuDOS.
#
#    GnuDOS is free software: you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation, either version 3 of the License, or
#    (at your option) any later version.
#
#    GnuDOS is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with GnuDOS.  If not, see <http://www.gnu.org/licenses/>.
#    

#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([GnuDOS], [2.0], [mohammed_isam1984@yahoo.com])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE([-Wall -Werror gnu])
#AC_CHECK_LIB([cups], [cupsGetJobs])
AM_PROG_AR
LT_INIT

# Checks for programs.
AC_PROG_CC

# Checks for libraries.
AC_SEARCH_LIBS([initscr], [ncursesw ncurses], [],
               [AC_MSG_ERROR([Unable to find ncursesw or ncurses])])

# Checks for header files.
AC_CHECK_HEADERS([fcntl.h stdlib.h string.h sys/ioctl.h termios.h unistd.h])
AC_CHECK_HEADERS([ncursesw/ncurses.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL

# Checks for library functions.
AC_FUNC_LSTAT_FOLLOWS_SLASHED_SYMLINK
AC_FUNC_MALLOC
AC_CHECK_FUNCS([getcwd memset mkdir rmdir strchr strdup strrchr strstr])

AC_CONFIG_FILES([Makefile
                 src/corelib/Makefile
                 src/mino/Makefile
                 src/prime/Makefile
                 doc/Makefile
                 doc/corelib/Makefile
                 doc/mino/Makefile
                 doc/prime/Makefile
                 info/Makefile
                 man/Makefile])
AC_OUTPUT
