/* 
 *    Programmed By: Mohammed Isam Mohammed [mohammed_isam1984@yahoo.com]
 *    Copyright 2014, 2015, 2016, 2017, 2018, 2024 (c)
 * 
 *    file: dir.c
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

#include "defs.h"
#include "options.h"
#include "dialogs.h"
#include <sys/ioctl.h>    //included for terminal size query
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

static struct dirent **eps;
static int epscount;

#define DIR_RETURN_ERROR()                      \
do {                                            \
    msgBox(strerror(errno), BUTTON_OK, ERROR);  \
    return 0;                                   \
} while(0)

static int one(const struct dirent *unused __attribute__((unused)))
{
    return 1;
}

int scanDir(char *dir, char ***dirs, char ***files, 
            int *totalDirs, int *totalFiles)
{
    int dcount = 0;
    int fcount = 0;
    int n;
    struct stat st;
    char *ldirs[MAXDIRS];
    char *lfiles[MAXFILES];

    if(eps != NULL)
    {
        for(n = 0; n < epscount; n++) free(eps[n]);
        free(eps);
        eps = NULL;
    }

    if(lstat(dir, &st) == -1) DIR_RETURN_ERROR();
    if(chdir(dir) == -1) DIR_RETURN_ERROR();
 
    if(S_ISDIR(st.st_mode)) 
    {
        n = scandir(".", &eps, one, alphasort);
        if(n >= 0) 
        {
            int cnt;

            for(cnt = 0; cnt < n; ++cnt) 
            {
                if(lstat(eps[cnt]->d_name,&st) == -1) DIR_RETURN_ERROR();

                if(S_ISDIR(st.st_mode))
                {
                    if(strcmp(eps[cnt]->d_name, ".") == 0) //ignore "."
                      continue;
                    ldirs[dcount++] = eps[cnt]->d_name;
                } 
                else 
                {
                    lfiles[fcount++] = eps[cnt]->d_name;
                }
            }
        }

        *totalDirs = dcount;
        *totalFiles = fcount;
        *dirs = malloc(sizeof(char **)*dcount);
        if(!*dirs) DIR_RETURN_ERROR();
        memcpy(*dirs, ldirs, sizeof(char **)*dcount);
        *files = malloc(sizeof(char **)*fcount);
        if(!*files) DIR_RETURN_ERROR();
        memcpy(*files, lfiles, sizeof(char **)*fcount);
        return 1;
    } 
    else 
    {
        char *tmp = malloc(strlen(dir)+21);
        if(!tmp) { msgBox("Insufficient memory", BUTTON_OK, ERROR); return 0; }
        sprintf(tmp, "Error opening dir:\n%s", dir);
        msgBox(tmp, BUTTON_OK, ERROR);
        free(tmp);
        return 0;
    }
}

