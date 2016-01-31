/*
 * Robert Hamilton
 * CSE 3320
 * Lab 1
 * This is a simple Curses based text shell
 * TODO
 * Print dirs
 * Change dir
 * Print files w/ file info
 * Scrolling
 * Tab completion
 */
#include "ts.h"


void printTime();

void printDirs();

void printFiles();

int getCmd();

int getString(char *buff, size_t buffSize);

void handleError();

void initWins();

/*
 * Initialize and call all necessary functions
 */
int main(int argc, char *argv[]) {
    strcpy(curDir, getenv("PWD"));
    // Usage and invalid command line args
    if (argc > 2) {
        usage();
        return -1;
    } else if (argc == 2) {
        if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0)
            usage();
        else if (strlen(argv[1]) < PATH_MAX) {
            strcpy(curDir, argv[1]);
            errNum = chdir(curDir);
            if (errNum != 0)
                fprintf(stderr, "Error changing directory: %s\n", strerror(errNum));
        }
        else
            fprintf(stderr, "Error: input path too long\n");
    }

    initscr();  // Start curses mode
    if (stdscr == NULL) {
        fprintf(stderr, "Error: could not initialize curses\n");
        exit(-1);
    }
    cbreak();               // Disable line buffering
    keypad(stdscr, TRUE);   // Read function keys
    getmaxyx(stdscr, maxY, maxX);

    // Setup color
    start_color();
    init_pair(1, COLOR_RED, COLOR_BLACK);

    initWins();

    handleMenu();

    endwin();

    return 0;
}

void initWins() {
    int i;

    // Create windows
    cursesWins[0] = newwin(LINES - 4, COLS / 2, 2, 0);
    cursesWins[1] = newwin(LINES - 4, COLS / 2, 2, maxX - (COLS / 2));

    for (i = 0; i < 2; ++i) {
        if (cursesWins[i] == NULL) {
            fprintf(stderr, "Error: could not initialize window[%d]\n", i);
            exit(-1);
        }
        // Create border around windows
        box(cursesWins[i], 0, 0);

        // Attach panel to window
        cursesPanels[i] = new_panel(cursesWins[i]);
        if (cursesPanels[i] == NULL) {
            fprintf(stderr, "Error: could not initialize panel[%d]\n", i);
            exit(-1);
        }
    }

    // Center and color window title
    wattron(cursesWins[0], A_STANDOUT);
    wattron(cursesWins[1], A_STANDOUT);
    mvwprintw(cursesWins[0], 1, (getmaxx(cursesWins[0]) / 2) - 2, "Files");
    mvwprintw(cursesWins[1], 1, (getmaxx(cursesWins[1]) / 2) - 5, "Directories");
    wattroff(cursesWins[0], A_STANDOUT);
    wattroff(cursesWins[1], A_STANDOUT);

    update_panels();
    doupdate();
}

// Print the display
void handleMenu() {
    printTime();

    printDirs();

    //printFiles();

    while (getCmd()) {
        continue;
    }
}

// Handle the user input
int getCmd() {
    // Print commands
    attron(A_STANDOUT);
    mvprintw(LINES - 2, 0, "e: edit file\tr: run program\tc: change directory\tq: quit");
    // Highlight entire line
    chgat(-1, A_REVERSE, 0, NULL);
    attroff(A_STANDOUT);

    // Move to next line, clear it and print user input prompt
    move(LINES - 1, 0);
    clrtoeol();
    printw("Command>> ");
    refresh();

    // Get and process user input
    inputChar = getch();
    switch (inputChar) {
        case 'q':   // Quit
            return 0;
        case 'e':   // Edit
            mvprintw(LINES - 1, 0, "Input file number or name: ");
            refresh();
            errNum = getstr(inputStr);
            if (errNum == 0) {
                // Create cmd string using env vars
                char *editor = getenv("EDITOR");
                strcpy(cmd, editor);
                strcat(cmd, " ");
                strcat(cmd, inputStr);

                // Temporarily leave curses mode and run syscall
                def_prog_mode();
                endwin();
                errNum = system(cmd);

                // Return to curses mode
                reset_prog_mode();
                refresh();

                if (errNum != 0) {
                    attron(COLOR_PAIR(1));
                    mvprintw(LINES - 1, 0, "Error editing file: %s\n", strerror(errNum));
                    attroff(COLOR_PAIR(1));
                } else {
                    attron(COLOR_PAIR(1));
                    mvprintw(LINES - 1, 0, "Error getting user input: %s\n", strerror(errNum));
                    attroff(COLOR_PAIR(1));
                }
            }
            return 1;
        case 'r':   // Run
            mvprintw(LINES - 1, 0, "Input file number or name: ");
            refresh();
            errNum = getstr(cmd);
            if (errNum == 0) {
                // Temporarily leave curses mode and run syscall
                def_prog_mode();
                endwin();
                errNum = system(cmd);

                // Return to curses mode
                reset_prog_mode();
                refresh();

                if (errNum != 0) {
                    attron(COLOR_PAIR(1));
                    mvprintw(LINES - 1, 0, "Error running file: %s\n", strerror(errNum));
                    attroff(COLOR_PAIR(1));
                    refresh();
                    getch();
                }
            }
            return 1;
        default:
            move(LINES - 1, 0);
            clrtoeol();
            attron(COLOR_PAIR(1));
            mvprintw(LINES - 1, 0, "Invalid command");
            attroff(COLOR_PAIR(1));
            refresh();
            return 1;
    }
}

// Display error message to user
void handleError() {
    if (errNum == NO_INPUT)
        fprintf(stderr, "Error: no input\n");
    else if (errNum == OVERFLOW)
        fprintf(stderr, "Error: input too long\n");
    else
        fprintf(stderr, "Error: unknown error %d\n", errNum);

}

/*
 * Safely handle the user input
 * Thanks to paxdiablo for the code provide at:
 * https://stackoverflow.com/questions/7672560/reading-in-a-variable-length-string-user-input-in-c
 */
int getString(char *buff, size_t buffSize) {
    int ch, extra;

    // Handle no user input
    if (getstr(buff) == NULL)
        return NO_INPUT;

    // If user input is too long, there'll be no newline. In that case, we flush
    // to end of line so that excess doesn't affect the next call.
    if (buff[strlen(buff) - 1] != '\n') {
        extra = 0;
        while (((ch = getchar()) != '\n') && (ch != EOF))
            extra = 1;
        return (extra == 1) ? OVERFLOW : 0;
    }

    // Otherwise remove newline and give string back to caller.
    buff[strlen(buff) - 1] = '\0';
    return 0;
}

void printFiles() {

}

// Get and display the current directories
void printDirs() {
    int i, count = 0;

    getcwd(curDir, PATH_MAX);
    attron(A_STANDOUT);
    printw("Current Working Directory: %s", curDir);
    attroff(A_STANDOUT);

    // Open and iterate through all directories printing them
/*    if (!(dir = opendir(curDir)))
        fprintf(stderr, "Error: could not open current directory\n");

    while ((dirEntry = readdir(dir))) {
        if ((dirEntry->d_type) & DT_DIR)
            printf("%d. %s\n", count++, dirEntry->d_name);
    }
    closedir(dir);

    for (i = 0; i < termSize.ws_col; ++i)
        printf("-");
    printf("\n");*/
}

// Get and display the current time
void printTime() {
    // Get current time
    curTime = time(NULL);

    // Convert to human format and print
    attron(A_STANDOUT);
    printw("Time: %s", ctime(&curTime));
    attroff(A_STANDOUT);
    // Highlight entire line
    chgat(-1, A_REVERSE, 0, NULL);
}

// Print out the proper usage of the program
void usage() {
    printf("Usage: ts -h|--help <directory>\n");
}
