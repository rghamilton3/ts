/*
 * Robert Hamilton
 * CSE 3320
 * Lab 1
 * This is a simple Curses based text shell
 * TODO
 * Change dir (path as an input arg)
 * Tab completion
 */
#include "ts.h"


/*
 * Initialize and call all necessary functions
 */
int main(int argc, char *argv[]) {
    int i;

    strcpy(path, getenv("PWD"));
    // Usage and invalid command line args
    if (argc > 2) {
        usage();
        return -1;
    } else if (argc == 2) {
        if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0)
            usage();
        else if (strlen(argv[1]) < PATH_MAX) {
            strcpy(path, argv[1]);
            errNum = chdir(path);
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
    noecho();               // Disable user input echoing
    keypad(stdscr, TRUE);   // Read function keys

    // Setup color
    start_color();
    init_pair(1, COLOR_RED, COLOR_BLACK);

    current = FILES;
    updateDisplay();
    handleUserInput();

    // Clean up memory
    unpost_menu(menus[0]);
    free_menu(menus[0]);
    for (i = 0; i < numFiles; i++)
        free_item(fileItems[i]);
    unpost_menu(menus[1]);
    free_menu(menus[1]);
    for (i = 0; i < numDirs; i++)
        free_item(dirItems[i]);
    endwin();

    return 0;
}

void updateDisplay() {
    erase();
    initWins();

    // Get files and directories
    getEntries();
    printHead();

    printMenus();
}

void printMenus() {
    int i;
    MENU *fileMenu, *dirMenu;

    // Create items
    fileItems = malloc(numFiles * sizeof(ITEM *));
    for (i = 0; i < numFiles; i++) {
        fileItems[i] = new_item(fileEntries[i]->d_name, NULL);
        if (fileItems[i] == NULL)
            break;
    }
    fileItems[numFiles] = NULL;

    dirItems = malloc(numDirs * sizeof(ITEM *));
    for (i = 0; i < numDirs; i++) {
        dirItems[i] = new_item(dirEntries[i]->d_name, NULL);
        if (dirItems[i] == NULL)
            break;
    }
    dirItems[numDirs] = NULL;


    // Create menus
    fileMenu = new_menu(fileItems);
    dirMenu = new_menu(dirItems);
    menus = malloc(2 * sizeof(MENU *));
    menus[0] = fileMenu;
    menus[1] = dirMenu;

    if (fileMenu == NULL || dirMenu == NULL) {
        fprintf(stderr, "Error creating menus\n");
    }

    // Associate windows and menus
    set_menu_win(fileMenu, cursesWins[0]);
    set_menu_sub(fileMenu, derwin(cursesWins[0], LINES / 2, (COLS / 2) - 6, 3, 1));
    set_menu_format(fileMenu, (LINES / 2), 1);

    set_menu_win(dirMenu, cursesWins[1]);
    set_menu_sub(dirMenu, derwin(cursesWins[1], LINES / 2, (COLS / 2) - 6, 3, 1));
    set_menu_format(dirMenu, (LINES / 2), 1);

    post_menu(fileMenu);
    wrefresh(cursesWins[0]);
    post_menu(dirMenu);
    wrefresh(cursesWins[1]);
}

void getEntries() {
    numDirs = scandir(path, &dirEntries, dirSelect, alphasort);
    numFiles = scandir(path, &fileEntries, fileSelect, alphasort);

    if (numDirs == 0 || numFiles == 0) {
        fprintf(stderr, "Error: no files in this directory\n");
        exit(-1);
    }

}

int dirSelect(const struct dirent *entry) {
    if (entry->d_type == DT_DIR && (strcmp(entry->d_name, ".") != 0))
        return true;
    else
        return false;
}

int fileSelect(const struct dirent *entry) {
    if (entry->d_type == DT_REG && (strcmp(entry->d_name, ".") != 0))
        return true;
    else
        return false;
}

void initWins() {
    int maxX, i;


    // Get main window size
    maxX = getmaxx(stdscr);

    // Create windows
    cursesWins[0] = newwin(LINES - 4, COLS / 2, 2, 0);
    cursesWins[1] = newwin(LINES - 4, COLS / 2, 2, maxX - (COLS / 2));

    for (i = 0; i < 2; ++i) {
        if (cursesWins[i] == NULL) {
            fprintf(stderr, "Error: could not initialize window[%d]\n", i);
            exit(-1);
        }
        keypad(cursesWins[i], TRUE);

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

    // Print commands
    attron(A_STANDOUT);
    mvprintw(LINES - 4, 0, "Menu commands:");
    chgat(-1, A_REVERSE, 0, NULL);
    mvprintw(LINES - 3, 0, "Tab: change menu  Arrows: highlight item  PgUp/PgDown: change menu page");
    chgat(-1, A_REVERSE, 0, NULL);
    mvprintw(LINES - 2, 0, "Highlighted item commands:");
    chgat(-1, A_REVERSE, 0, NULL);
    mvprintw(LINES - 1, 0, "e: edit file  r: run program  c: change to directory  q: quit");
    // Highlight entire line
    chgat(-1, A_REVERSE, 0, NULL);
    attroff(A_STANDOUT);

    refresh();
}

// Handle the user input
int handleUserInput() {
    char *editor, *newPath;
    // Get and process user input
    while ((inputChar = getch()) != 'q') {
        switch (inputChar) {
            case 'e':   // Edit
                // Create cmd string using env vars
                editor = getenv("EDITOR");
                if (editor != NULL) {
                    strcpy(cmd, editor);
                    strcat(cmd, " ");
                } else
                    strcpy(cmd, "nano ");
                strcat(cmd, item_name(current_item(menus[FILES])));

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
                }
                break;
            case 'r':   // Run
                // Temporarily leave curses mode and run syscall
                def_prog_mode();
                endwin();
                errNum = system(item_name(current_item(menus[FILES])));

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
                break;
            case 'c':   // Change directory
                strcpy(cmd, "cd ");
                strcat(cmd, item_name(current_item(menus[DIRS])));

                errNum = system(cmd);
                if (errNum != 0) {
                    attron(COLOR_PAIR(1));
                    mvprintw(LINES - 1, 0, "Error changing directory: %s\n", strerror(errNum));
                    attroff(COLOR_PAIR(1));
                } else {
                    newPath = getenv("PWD");
                    strcpy(path, newPath);
                    updateDisplay();
                }
                break;
            case KEY_DOWN:
                menu_driver(menus[current], REQ_DOWN_ITEM);
                wrefresh(cursesWins[current]);
                break;
            case KEY_UP:
                menu_driver(menus[current], REQ_UP_ITEM);
                wrefresh(cursesWins[current]);
                break;
            case KEY_NPAGE:
                menu_driver(menus[current], REQ_SCR_DPAGE);
                wrefresh(cursesWins[current]);
                break;
            case KEY_PPAGE:
                menu_driver(menus[current], REQ_SCR_UPAGE);
                wrefresh(cursesWins[current]);
                break;
            case '\t':
                if (current == FILES)
                    current = DIRS;
                else
                    current = FILES;
                break;
            default:
                move(LINES - 1, 0);
                clrtoeol();
                attron(COLOR_PAIR(1));
                mvprintw(LINES - 1, 0, "Invalid command");
                attroff(COLOR_PAIR(1));
                refresh();
                break;
        }
    }
}

// Get and display the current time
void printHead() {
    // Get current time
    time_t curTime;
    curTime = time(NULL);

    // Convert time to human format and print
    mvprintw(0, 0, "Time: %s", ctime(&curTime));
    // Print current working directory
    mvprintw(1, 0, "Current Directory: %s", path);
}

// Print out the proper usage of the program
void usage() {
    printf("Usage: ts -h|--help <directory>\n");
}
