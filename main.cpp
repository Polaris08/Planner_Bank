#include "display.hpp"
#include "planner.hpp"

#include <iostream>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

using namespace std;

int main(void) {
    int previous_keystroke = 0, menu, index = 0;
    const int LEDGER = 0;
    const int PLANNER = 1;

    // get current date
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    int date = tm.tm_mday;
    int month = tm.tm_mon + 1;
    int year = tm.tm_year + 1900;
    Todo todo;

    int value;
    while (1) {
        menu = chooseMenu();
        printCalendar(year, month, date);
        if (menu == LEDGER) {

            /* 가계부 */

        } else if (menu == PLANNER) {

            while (1) {
                printList(year, month, date);
                menu = choosePlannerMenu(year, month, date);
                switch (menu) {
                case CREATE:
                    todo.createEvent();
                    break;
                case DELETE:
                    index = chooseEvent(year, month, date);
                    deleteEvent(year, month, date, index);
                    break;
                case GOTODATE:
                    gotoDate(&year, &month, &date);
                    break;
                case PREVIOUS:
                    goto END;
                    break;
                }
            }
        END:;
        }
    }

    return 0;
}
