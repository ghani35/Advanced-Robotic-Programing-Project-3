#include <ncurses.h>
#include <string.h>
#include <unistd.h> 
#include <math.h>
#include <time.h>
#include <stdlib.h>

void init_console_ui() {

    // Initialize curses mode
    initscr();		 
	start_color();

    // Disable line buffering...
	cbreak();

    // Disable char echoing and blinking cursos
    noecho();
    nodelay(stdscr, TRUE);
    curs_set(0);

    init_pair(1, COLOR_BLACK, COLOR_GREEN);
    init_pair(2, COLOR_WHITE, COLOR_BLUE);

    // Activate input listening (keybord + mouse events ...)
    keypad(stdscr, TRUE);

    refresh();
}


void reset_console_ui() {

    // Clear screen
    erase();
    refresh();
}


