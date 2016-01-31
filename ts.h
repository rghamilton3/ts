/*
 * Robert Hamilton
 * CSE 3320
 * Lab 1
 * Header file for main.c
 * Buffer sizes use PATH_MAX as determined by limits.h to ensure portability and compliance with POSIX standards
*/

#ifndef TS_TS_H
#define TS_TS_H

// Dependent headers
#include <curses.h>
#include <dirent.h>
#include <limits.h>
#include <panel.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

// Error handling
#define NO_INPUT -1
#define OVERFLOW 1


// Variable Declarations
WINDOW *cursesWins[2];          // Curses Windows
PANEL *cursesPanels[2];         // Curses Panels
int maxY, maxX, linesToPrint;   // Window sizes
pid_t child;                    // Process ID
DIR *dir;                       // Directory Stream
struct dirent *dirEntry;        // Directory Entry
char curDir[PATH_MAX];          // Buffer for Current Working Directory
char cmd[PATH_MAX + 256];       // Command
char inputStr[PATH_MAX];        // User Input String
int inputChar;                  // User Input Char
time_t curTime;                 // Time
struct winsize termSize;        // Terminal Lines and Columns
int errNum;                     // Placeholder for error checking

// Function Declarations
void usage();

void handleMenu();


#endif //TS_TS_H
