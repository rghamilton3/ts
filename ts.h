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
#include <menu.h>
#include <panel.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

// Variable Declarations
typedef enum {
    FILES, DIRS
} curMenu;
curMenu current;
WINDOW *cursesWins[2];                          // Curses Windows
PANEL *cursesPanels[2];                         // Curses Panels
ITEM **fileItems, **dirItems;                   // Menu items
MENU **menus;                                   // Menus
struct dirent **dirEntries, **fileEntries;      // Directory Entries
int numDirs, numFiles;                          // Number of entries
char cmd[PATH_MAX + 256];                       // Command
char path[PATH_MAX];                            // Current path
int inputChar;                                  // User Input Char
int errNum;                                     // Placeholder for error checking

// Function Declarations
int dirSelect(const struct dirent *entry);

int fileSelect(const struct dirent *entry);

void getEntries();

void usage();

void printHead();

int handleUserInput();

void initWins();

void updateDisplay();

void printMenus();

#endif //TS_TS_H
