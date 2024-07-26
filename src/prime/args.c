/* 
 *    Programmed By: Mohammed Isam Mohammed [mohammed_isam1984@yahoo.com]
 *    Copyright 2013, 2014, 2015, 2016, 2017, 2018, 2024 (c)
 * 
 *    file: args.c
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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <asm/types.h>
#include <limits.h>
#include <string.h>
#include <pwd.h>
#include <time.h>
#include <sys/stat.h>
#include "defs.h"
#include "file.h"

char *prime_ver = "2.1";
char *copyright_notice = 
    "Copyright Mohammed Isam, 2013, 2014, 2015, 2016, 2017, 2018, 2024\n"
    "Prime is a GNU software, part of the GnuDOS package\n"
    "Prime is a file manager for the GNU/Linux console/xterm.\n"
    "It was developed with ease of use and simplicity in mind.\n";

// defined in init.c
extern int NEW_GNU_DOS_LEVEL;


void parse_args(int argc, char **argv)
{
    if(argc == 1)
    {
        return;
    }

    ///////////////////////////////////////
    //parse command line arguments
    ///////////////////////////////////////
    int c;
    int EXPORT_FLAG = 0;
    char *EXPORT_DIR = NULL;

    static struct option long_options[] =
    {
        {"reset-config", no_argument, 0,  'r'},
        {"help",         no_argument, 0,  'h'},
        {"level",  required_argument, 0,  'l'},
        {"export", required_argument, 0,  'e'},
        {"version",      no_argument, 0,  'v'},
        {0, 0, 0, 0}
    };

    while(1)
    {
        int option_index = 0;
        c = getopt_long(argc, argv, "vhl:e:r", long_options, &option_index);

        if(c==-1) break;    //end of options

        switch(c)
        {
            case 0:
                break;

            case 'r':    //reset config file
                if(!write_config_file_defaults())
                {
                    fprintf(stderr, "Error writing default config file: "
                                    "~/.prime.conf.\n");
                    exit(1);
                }

                printf("Configuration has been reset. "
                       "Changes were written to ~/.prime.conf\n");
                exit(0);

            case 'e':    //export directory tree
                EXPORT_DIR = optarg;
                //set the flag for later
                EXPORT_FLAG = 1;
                break;

            case 'l':    //set GNU_DOS level
                c = atoi(optarg);

                if(c < 1 || c > 6)
                {
                    fprintf(stderr, "Unrecognised level. See 'man prime' or "
                                    "'info prime' for information about "
                                    "possible levels.\n");
                    exit(1);
                }

                NEW_GNU_DOS_LEVEL = c;
                break;

            case 'v':    //show version & exit
                printf("%s\n", prime_ver);
                exit(0);

            case 'h':    //show help & exit
                printf("Prime File Manager, Version %s\n"
                       "%s\nUsage: %s [options] [dir-name]\n"
                       "\nOptions:\n"
                       "  [-e, --export] dir file:\n"
                       "  \texport directory tree of 'dir' into 'file'\n"
                       "  [-h, --help]:\n"
                       "  \tshow this help\n"
                       "  [-l,--level] GNU DOS level:\n"
                       "  \tthe GNU DOS level of experience to be used\n"
                       "  [-r, --reset-config]:\n"
                       "  \treset the configuration file to it's defaults\n"
                       "  [-v, --version]:\n"
                       "  \tprint software version and exit\n"
                       "  dir-name: is the name of directory to load into "
                       "Prime at startup\n\n",
                       prime_ver, copyright_notice, argv[0]);
                exit(0);

            case '?':
                exit(1);

            default:
                abort();
        }
    }

    ///////////////////////////////////////
    //check for a missing argument
    ///////////////////////////////////////
    if(optind >= argc && EXPORT_FLAG)
    {
        fprintf(stderr, "Error: Missing argument: export-filename\n"
               "Try %s -h or %s --help\n", argv[0], argv[0]);
        exit(1);
    }

    ///////////////////////////////////////
    //parse the remaining arguments
    ///////////////////////////////////////
    if(optind < argc)
    {
        if(EXPORT_FLAG)
        {
            exportTreeFromCommandLine(EXPORT_DIR, argv[optind]);
            exit(0);
        }

        //******Start with directory dir
        //check for '~'in dir name
        struct stat st;

        if(stat(argv[optind], &st) == -1) 
        {
            fprintf(stderr, "Directory '%s' doesn't exist.\n", argv[optind]);
            fprintf(stderr, "Starting prime from current working directory.\n");
            fprintf(stderr, "Press any key..");
            getchar();
        }
        else if(!S_ISDIR(st.st_mode))
        {
            fprintf(stderr, "Error: '%s' is not a directory.\n", argv[optind]);
            fprintf(stderr, "Starting prime from current working directory.\n");
            fprintf(stderr, "Press any key..");
            getchar();
        }
        else
        {
            if(chdir(argv[optind]) == -1)
            {
                fprintf(stderr, "Error changing directory to '%s'\n", argv[optind]);
                exit(1);
            }
        }
    }
}

