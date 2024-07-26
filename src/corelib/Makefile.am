# 
#    Copyright 2013, 2014 2015, 2016, 2017, 2018, 2024 (c)
#    Mohammed Isam Mohammed [mohammed_isam1984@yahoo.com]
# 
#    file: Makefile.am
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

#AUTOMAKE_OPTIONS = subdir-objects
ACLOCAL_AMFLAGS = -I m4
LIB_VERSION=1:9:0
lib_LTLIBRARIES = libgnudos.la
libgnudos_la_SOURCES = kbd.c ukbd.c screen.c dialogs.c
library_includedir = $(includedir)/console
library_include_HEADERS = kbd.h ukbd.h screen.h dialogs.h
libgnudos_la_LDFLAGS = -version-info $(LIB_VERSION)
