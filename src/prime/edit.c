/* 
 *    Programmed By: Mohammed Isam Mohammed [mohammed_isam1984@yahoo.com]
 *    Copyright 2013, 2014, 2015, 2016, 2017, 2018, 2024 (c)
 * 
 *    file: edit.c
 *    This file is part of Prime.
 *
 *    Prime is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    Prime is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with Prime.  If not, see <http://www.gnu.org/licenses/>.
 */    

#include <ncurses.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>
#include "defs.h"
#include "edit.h"
#include "cutcopy.h"
#include "find.h"
#include "dirstruct.h"
#include "file.h"

int CONFIRM_CUT_COPY;
int ABORT_COPY;

static void copyThisFile(char *fileName, FILE *logfile);
static void copyThisDir(char *tmp, int level, FILE *logfile);
static void moveThisDir(char *tmp, int level, FILE *logfile);
static void cutOne(void);


static int one(const struct dirent *unused __attribute__((unused))) 
{
    return 1;
}

mode_t get_mode(__mode_t st_mode)
{
    mode_t mode = 00;
    if(st_mode & S_IRUSR) { mode += 0400; }
    if(st_mode & S_IWUSR) { mode += 0200; }
    if(st_mode & S_IXUSR) { mode += 0100; }
    if(st_mode & S_IRGRP) { mode +=  040; }
    if(st_mode & S_IWGRP) { mode +=  020; }
    if(st_mode & S_IXGRP) { mode +=  010; }
    if(st_mode & S_IROTH) { mode +=   04; }
    if(st_mode & S_IWOTH) { mode +=   02; }
    if(st_mode & S_IXOTH) { mode +=   01; }
    return mode;
}

void clearSelection(void)
{
    unMarkAll(DIR_WIN );
    unMarkAll(FILE_WIN);
    numCopy    = 0;
    numCut     = 0;
    numStarred = 0;
}

/********************************************
 * Marks all items in the active window with
 * a '*' mark.
 * ******************************************/
void markAll(int activeWindow)
{
    int i, j = 0;

    if(activeWindow == DIR_WIN) 
    {
        for(i = 0; i < totalDirs; i++) 
        {
            //ignore '.' and '..'
            if((strcmp(dirs[i]->name, ".") == 0) || 
               (strcmp(dirs[i]->name, "..") == 0))
               continue;

            if(dirs[i]->star == '*') continue;
            if(dirs[i]->star == '^') { removeCutDir(i); numCut--; }
            if(dirs[i]->star == '#') { removeCopyDir(i); numCopy--; }
            dirs[i]->star = '*';
            j++;
        }

        refreshDirView();
    } 
    else 
    {
        for(i = 0; i < totalFiles; i++) 
        {
            if(files[i]->type == '%') break;
            if(files[i]->star == '*') continue;
            if(files[i]->star == '^') { removeCutFile(i); numCut--; }
            if(files[i]->star == '#') { removeCopyFile(i); numCopy--; }
            files[i]->star = '*';
            j++;
        }

        refreshFileView();
    }

    numStarred += j;
    refreshBottomView();
}

/********************************************
 * Removes all marks on items in the active 
 * window.
 * ******************************************/
void unMarkAll(int activeWindow)
{
    int i, j = 0;

    if(activeWindow == DIR_WIN) 
    {
        for(i = 0; i < totalDirs; i++) 
        {
            if(dirs[i]->star == ' ') continue;
            if(dirs[i]->star == '^') { removeCutDir(i); numCut--; }
            if(dirs[i]->star == '#') { removeCopyDir(i); numCopy--; }
            dirs[i]->star = ' ';
            j++;
        }

        refreshDirView();
    } 
    else 
    {
        for(i = 0; i < totalFiles; i++) 
        {
            if(files[i]->type == '%') break;
            if(files[i]->star == ' ') continue;
            if(files[i]->star == '^') { removeCutFile(i); numCut--; }
            if(files[i]->star == '#') { removeCopyFile(i); numCopy--; }
            files[i]->star = ' ';
            j++;
        }

        refreshFileView();
    }

    numStarred -= j;
    refreshBottomView();
}

/********************************************
 * Marks one item for cut in the active 
 * window with a '^' mark.
 * ******************************************/
void cutOne(void)
{
    int pos;

    if(activeWindow == DIR_WIN) 
    {
        pos = firstVisDir+selectedDir;

        //ignore . & ..
        if((strcmp(dirs[pos]->name, ".") == 0) || 
           (strcmp(dirs[pos]->name, "..") == 0))
            return;

        if(dirs[pos]->star == '^') return;
        else if(dirs[pos]->star == '*') numStarred--;
        else if(dirs[pos]->star == '#') 
        {
            removeCopyDir(pos); 
            numCopy--; 
        }

        dirs[pos]->star = '^'; 
        saveCutDir(pos);
        numCut++;
    } 
    else 
    { 
        pos = firstVisFile+selectedFile;

        if(files[pos]->type == '%') return;
        if(files[pos]->star == '^') return;
        else if(files[pos]->star == '*') numStarred--;
        else if(files[pos]->star == '#') 
        {
            removeCopyFile(pos); 
            numCopy--;
        }

        files[pos]->star = '^'; 
        saveCutFile(pos);
        numCut++; 
    }

    refreshFileView();
    refreshDirView();
    refreshBottomView();
}

/********************************************
 * Sets marked items for cut in the active 
 * window with a '^' mark.
 * ******************************************/
void cutMarked(void)
{
    if(numStarred == 0) 
    {        //if no starred items, just mark the selected item
        cutOne();
        return;
    } 
    else 
    {        //mark the starred items by changing '*' to '^'
        int i;

        for(i = 0; i < totalDirs; i++) 
        { 
            if(dirs[i]->star == '*') 
            { 
                dirs[i]->star = '^';
                numCut++;
                saveCutDir(i);
            }
        }

        for(i = 0; i < totalFiles; i++) 
        { 
            if(files[i]->star == '*') 
            { 
                files[i]->star = '^';
                numCut++; 
                saveCutFile(i);
            }
        }

        numStarred = 0;
        refreshFileView();
        refreshDirView();
        refreshBottomView();
    }
}

/********************************************
 * Marks one item for copy in the active 
 * window with a '#' mark.
 * ******************************************/
void copyOne(void)
{
    int pos;

    if(activeWindow == DIR_WIN) 
    {
        pos = firstVisDir+selectedDir;

        //ignore . & ..
        if((strcmp(dirs[pos]->name, ".") == 0) || 
           (strcmp(dirs[pos]->name, "..") == 0))
            return;

        if(dirs[pos]->star == '#') return;
        else if(dirs[pos]->star == '*') numStarred--;
        else if(dirs[pos]->star == '^') 
        {
            removeCutDir(pos); 
            numCut--;
        }

        dirs[pos]->star = '#'; 
        saveCopyDir(pos); 
        numCopy++;
    }
    else 
    { 
        pos = firstVisFile+selectedFile;

        if(files[pos]->type == '%') return;
        if(files[pos]->star == '#') return;
        else if(files[pos]->star == '*') numStarred--;
        else if(files[pos]->star == '^') 
        {
            removeCutFile(pos);
            numCut--;
        }

        files[pos]->star = '#'; 
        saveCopyFile(pos);
        numCopy++; 
    }

    refreshFileView();
    refreshDirView();
    refreshBottomView();
}


/********************************************
 * Sets marked items for copy in the active 
 * window with a '^' mark.
 * ******************************************/
void copyMarked(void)
{
    if(numStarred == 0) 
    {        //if no starred items, just mark the selected item
        //redraw main window but don't clear the area
        drawBox(1, 1, SCREEN_H, SCREEN_W, " Prime File Manager ", 0);
        copyOne();
        return;
    }
    else 
    {        //mark the starred items by changing '*' to '^'
        int i;

        for(i = 0; i < totalDirs; i++)
        { 
            if(dirs[i]->star == '*') 
            { 
                dirs[i]->star = '#';
                numCopy++; 
                saveCopyDir(i);
            }
        }

        for(i = 0; i < totalFiles; i++)
        { 
            if(files[i]->star == '*') 
            { 
                files[i]->star = '#';
                numCopy++; 
                saveCopyFile(i);
            }
        }

        numStarred = 0;
        refreshFileView();
        refreshDirView();
        refreshBottomView();
        //redraw main window but don't clear the area
        drawBox(1, 1, SCREEN_H, SCREEN_W, " Prime File Manager ", 0);
    }
}

/********************************************
 * this function pastes all the files/dirs
 * marked for cut/copy.. it puts them in the
 * cwd path.
 * ******************************************/
void pasteMarked(void)
{
    if(numCut == 0 && numCopy == 0) 
    {
        msgBoxH("Nothing marked for Cut/Copy.", BUTTON_OK, INFO);
        return;
    }
    
    CONFIRM_CUT_COPY = 0;
    ABORT_COPY = 0;

    int i;
    FILE *logfile = tmpfile();

    /********************************************
     * First copy files marked for copying
     * ******************************************/  
    for(i = 0; i < numCopyFiles; i++) 
    {
        if(ABORT_COPY) goto finish;
        copyThisFile(copyFiles[i], logfile);
    }

    numCopyFiles = 0;

    /********************************************
     * Then move files marked for moving
     * ******************************************/  
    for(i = 0; i < numCutFiles; i++) 
    {
        if(ABORT_COPY) goto finish;
        copyThisFile(cutFiles[i], logfile);

        if(remove(cutFiles[i])) 
        {    //success returns 0, otherwise err value
            fprintf(logfile, "Unable to remove file: %s\n", cutFiles[i]);
            //showErrorMsgBox("Unable to remove file!. ", cutFiles[i]);
        }
    }

    numCutFiles = 0;

    /********************************************
     * Now copy dirs marked for copying
     * ******************************************/  
    for(i = 0; i < numCopyDirs; i++) 
    {
        if(ABORT_COPY) goto finish;
        copyThisDir(copyDirs[i], 0, logfile);
    }  

    numCopyDirs = 0;
  
    /********************************************
     * And move dirs marked for moving
     * ******************************************/  
    for(i = 0; i < numCutDirs; i++) 
    {
        if(ABORT_COPY) goto finish;
        moveThisDir(cutDirs[i], 0, logfile);
    }  

    numCutDirs = 0;
  
    numCopy = 0;
    numCut = 0;

    purgeLogFile(logfile);

finish:

    CONFIRM_CUT_COPY = 0;
    ABORT_COPY = 0;
    scanDir(cwd);
}

/****************************************
 * copies the file sent as fileName into
 * the CWD with the same fileName.
 * **************************************/
static void copyThisFile(char *fileName, FILE *logfile)
{
    char *str;
    FILE *inFile;
    FILE *outFile;
    char buf[4096];
    int  read_size;
    int z;
    struct stat statbuf;

    //find the last '/' in the path, add one for the start of the filename
    str = strrchr(fileName, '/');
    if(str) str++;
    else str = fileName;

    // check if the file exists
    if(lstat(str, &statbuf) != -1)
    {
        if(!CONFIRM_CUT_COPY) 
        {
            char *s = malloc(strlen(fileName)+34);

            if(!s)
            {
                msgBoxH("Insufficient memory", BUTTON_OK, ERROR);
                return;
            }

            strcpy(s, "File:\n");
            strcat(s, fileName);
            strcat(s, "\nalready exists. Overwrite?");

            z = msgBoxH(s, BUTTON_YES|BUTTON_NO|BUTTON_ALL, CONFIRM);
            free(s);

            if(z == BUTTON_NO) return;
            else if(z == BUTTON_ABORT)
            {
                ABORT_COPY = 1;
                return;
            }
            else if(z == BUTTON_ALL) CONFIRM_CUT_COPY = 1;
        }
    }

    // open the input file
    if(!(inFile = fopen(fileName, "rb"))) 
    {
        if(logfile)
             fprintf(logfile, "Error opening input file: %s\n", fileName);
        else showErrorMsgBox("Error opening input file:\n", fileName);
        return;
    }

    // open out file for writing
    if(!(outFile = fopen(str, "wb")))
    {
        fclose(inFile);

        if(logfile)
        {
            fprintf(logfile, "Error opening output file: %s\n", fileName);
        }
        else
        {
            char *s = malloc(strlen(fileName)+37);

            if(!s)
            {
                msgBoxH("Insufficient memory", BUTTON_OK, ERROR);
                return;
            }

            strcpy(s, "Error opening write file:\n");
            strcat(s, fileName);
            strcat(s, "\nAborting.");
            msgBoxH(s, BUTTON_OK, ERROR); 
            free(s);
        }

        return;
    }
    
    // start copying infile to outfile in chunks
    while((read_size = fread(buf, sizeof(char), sizeof(buf), inFile))) 
    {
        fwrite(buf, sizeof(char), read_size, outFile);
    }
    
    fclose(inFile);
    fclose(outFile);
    
    if(lstat(fileName, &statbuf) == -1)
    {
        if(logfile) 
             fprintf(logfile, "Error: %s: %s\n", strerror(errno), fileName);
        else msgBoxH(strerror(errno), BUTTON_OK, ERROR);
        return;
    }

    // Set the file mode
    if(chmod(str, get_mode(statbuf.st_mode)) != 0) 
    {
        if(logfile) 
             fprintf(logfile, "Error: %s: %s\n", strerror(errno), fileName);
        else msgBoxH(strerror(errno), BUTTON_OK, ERROR);
    }
}

static void __copyMoveDir(char *tmp, int level, int moving, FILE *logfile)
{
    char *str;
    int n;
    struct dirent **eps;
    struct stat st;
    int h = SCREEN_H/2;
    int w = SCREEN_W/2;
    char *msg = moving ? " Moving " : " Copying ";
    int tmplen = strlen(tmp);
    mode_t oldmode;

    //find the last '/' in the path, add one for the start of the dirname
    str = strrchr(tmp, '/');
    if(str) str++; 
    else str = tmp;
  
    //show progress to the user
    drawBox(h-2, w-30, h+2, w+30, msg, 1);
    move(h-2, w-30);

    if((int)strlen(str) > w-2)
    {
        for(n = 0; n < w-2; n++) addch(str[n]);
    }
    else printw("%s", str);

    refresh();
  
    //check if it doesn't exist, create it
    if(stat(str, &st) == -1)
    {
        mkdir(str, 0700);
        oldmode = 0700;
    }
    else 
    {
        oldmode = st.st_mode;

        if(!CONFIRM_CUT_COPY) 
        {
            char *s = malloc(strlen(str)+39);

            if(!s)
            {
                msgBoxH("Insufficient memory", BUTTON_OK, ERROR);
                return;
            }

            strcpy(s, "Directory:\n");
            strcat(s, str);
            strcat(s, "\nalready exists. Overwrite?");

            n = msgBoxH(s, BUTTON_YES|BUTTON_NO|BUTTON_ALL, CONFIRM);
            free(s);

            if(n == BUTTON_NO) return;
            else if(n == BUTTON_ABORT) { ABORT_COPY = 1; return; }
            else if(n == BUTTON_ALL) CONFIRM_CUT_COPY = 1;
        }
    }
  
    if(chdir(str) == -1)
    {
        if(logfile)
             fprintf(logfile, "Error changing directory: %s\n", strerror(errno));
        else showErrorMsgBox("Error changing directory: ", strerror(errno));
        return;
    }
  
    n = scandir(tmp, &eps, one, alphasort);

    if(n >= 0) 
    {
        int cnt;

        for(cnt = 0; cnt < n; ++cnt) 
        {
            char *d = eps[cnt]->d_name;

            //ignore "." & ".."
            if(strcmp(d, ".") == 0 || strcmp(d, "..") == 0) continue;

            char *cc = malloc(tmplen+strlen(d)+2);

            if(!cc)
            {
                msgBoxH("Insufficient memory", BUTTON_OK, ERROR);
                return;
            }

            strcpy(cc, tmp);
            strcat(cc, "/");
            strcat(cc, d);
            lstat(cc, &st);

            if(S_ISDIR(st.st_mode)) 
            {
                if(moving) moveThisDir(cc, level+1, logfile);
                else       copyThisDir(cc, level+1, logfile);
            }
            else 
            {
                copyThisFile(cc, logfile);
                if(moving) remove(cc);
            }

            free(cc);
            //free(eps[cnt]->d_name);
            free(eps[cnt]);
        }

        free(eps);
    }
  
    if(chdir("..") == -1)
    {
        if(logfile)
             fprintf(logfile, "Error changing directory: %s\n", strerror(errno));
        else showErrorMsgBox("Error changing directory:\n", strerror(errno));
        return;
    }

    // Set the Directory mode
    if(chmod(str, oldmode) != 0) 
    {
        if(logfile)
            fprintf(logfile, "Error changing directory mode for %s: %s\n", 
                             str, strerror(errno));
        else showErrorMsgBox("Error changing directory mode:\n", 
                             strerror(errno));
    }

    if(moving) rmdir(tmp);
}

/**********************************************
 * copies the dir sent as tmp[] recursively
 * making new subdirs in the CWD if necessary.
 * ********************************************/
static void copyThisDir(char *tmp, int level, FILE *logfile)
{
    __copyMoveDir(tmp, level, 0, logfile);
}

/**********************************************
 * moves the dir sent as tmp[] recursively
 * making new subdirs in the CWD if necessary.
 * ********************************************/
static void moveThisDir(char *tmp, int level, FILE *logfile) 
{
    __copyMoveDir(tmp, level, 1, logfile);
}

#define chdirError(dir)                                             \
do {                                                                \
    if(logfile)                                                     \
        fprintf(logfile,                                            \
        "Error changing directory (%s): %s\n", dir, strerror(errno));\
    else showErrorMsgBox("Error changing directory:\n", dir);       \
} while(0)

/**********************************************
 * deletes the dir sent as tmp[] and all its
 * child dirs and contained files recursively.
 * ********************************************/
void deleteThisDir(char *tmp, int level, FILE *logfile) 
{
    int n;
    struct dirent **eps;
    int h = SCREEN_H/2;
    int w = SCREEN_W/2;
    struct stat statbuf;

    //show progress to the user
    drawBox(h-2, w-30, h+2, w+30, " Deleting ", 1);
    move(h-2, w-30);

    if((int)strlen(tmp) > w-2)
    {
        for(n = 0; n < w-2; n++) addch(tmp[n]);
    }
    else printw("%s", tmp);

    refresh();

    n = scandir(tmp, &eps, one, alphasort);

    if(n >= 0) 
    {
        if(chdir(tmp) == -1)
        {
            chdirError(tmp);
            return;
        }

        int cnt;

        for(cnt = 0; cnt < n; ++cnt) 
        {
            char *d = eps[cnt]->d_name;

            //ignore "." & ".."
            if(strcmp(d, ".") == 0 || strcmp(d, "..") == 0) continue;

            lstat(d, &statbuf);

            if(S_ISDIR(statbuf.st_mode)) 
            {
                deleteThisDir(d, level+1, logfile);
            } 
            else 
            {
                remove(d);
            }

            free(eps[cnt]);
        }

        free(eps);

        if(chdir("..") == -1)
        {
            chdirError("..");
            return;
        }
    } 
    else 
    {
        if(level == 0)
        {
            if(logfile) fprintf(logfile, "Error opening directory: %s\n", tmp);
            else showErrorMsgBox("Error opening directory:\n", tmp);
            return;
        }
    }
  
    if(rmdir(tmp) == -1)
    {
        if(logfile) fprintf(logfile, "Error removing directory (%s): %s\n", 
                                     tmp, strerror(errno));
        else showErrorMsgBox("Error removing directory:\n", strerror(errno));
    }
}

/********************************************
 * this function deletes all the files/dirs
 * marked with a star.. if no items marked,
 * it deletes the highlighted file/dir.
 * ******************************************/
/********************************************
 * Removes marked items in the active window.
 * ******************************************/
void deleteMarked(void)
{
    int i;
    char *tmp;

    if(msgBoxH("Delete all selected files/dirs?", 
            BUTTON_YES|BUTTON_NO, CONFIRM) == BUTTON_NO)
        return;

    for(i = 0; i < totalFiles; i++) 
    {
        if(files[i]->star == '*')
        {
            // if the name is widechar, convert to a C-string
            if(files[i]->iswcs)
            {
                if((tmp = to_cstring(files[i]->name, NULL)))
                {
                    remove(tmp);
                    free(tmp);
                }
            }
            else remove(files[i]->name);
        }
    }

    for(i = 0; i < totalDirs; i++) 
    {
        if(dirs[i]->star == '*') 
        {
            // if the name is widechar, convert to a C-string
            if(dirs[i]->iswcs)
            {
                if((tmp = to_cstring(dirs[i]->name, NULL)))
                {
                    deleteThisDir(tmp, 0, NULL);
                    free(tmp);
                }
            }
            else deleteThisDir(dirs[i]->name, 0, NULL);
        }
    }

    numStarred = 0;
}

void editMenu_Find(void)
{
    findFile();
    hideCursor();
    scanDir(cwd);
}

void editMenu_Properties(void)
{
    showPropertiesDialog();
    scanDir(cwd); 
}

