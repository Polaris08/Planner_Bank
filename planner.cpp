#include "planner.hpp"
#include "display.hpp"

#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>

#define INCOMPLETE "incomplete.txt"
#define COMPLETE "complete.txt"

using namespace std;

Todo::Todo() {}
Todo::Todo(int year, int month, int date, string title) {
    when.year = year;
    when.month = month;
    when.date = date;
    this->title = title;
}
int Todo::getDate() { return when.date; }
int Todo::getMonth() { return when.month; }
int Todo::getYear() { return when.year; }
string Todo::getTitle() { return title; }
void Todo::setDate(int year, int month, int date) {
    when.year = year;
    when.month = month;
    when.date = date;
}
void Todo::setTitle(string title) { this->title = title; }
string convert(int date) {
    if (date < 10)
        return '0' + to_string(date);
    else
        return to_string(date);
}
void Todo::inputEvent() {

    while (1) {
        resetDisplay(97, 7, 28, 3);

        gotoxy(97, 7);
        printWithBg(whte, blck, "date: ");
        cin >> when.year >> when.month >> when.date;
        if (when.month < 0 || when.month > 13 || when.date < 0 ||
            when.date > numberOfDays(when.month, when.year)) {
            resetDisplay(9, 7, 28, 3);
            gotoxy(95, 8);
            printWithBg(whte, blck, "wrong input! try again");
            continue;
        } else
            break;
    }

    gotoxy(97, 9);
    printWithBg(whte, blck, "title: ");
    cin >> title;

    setDate(when.year, when.month, when.date);
    setTitle(title);
    cin.ignore();
}

void Todo::createEvent() {
    inputEvent();
    int fd;
    string dirname =
        convert(getYear()) + convert(getMonth()) + convert(getDate());

    string pathname = dirname + "/" + INCOMPLETE;

    char buf[MAX_BUF_SIZE + 1] = {
        '\0',
    };
    if (!checkFileExists(dirname)) {
        if (mkdir(dirname.c_str(), PERMS) < 0) {
            perror("mkdir() error!");
        }
    }

    if ((fd = open(pathname.c_str(), O_RDWR | O_CREAT | O_APPEND, PERMS)) < 0) {
        perror("open() error!");
        exit(-1);
    }

    if ((write(fd, getTitle().c_str(), MAX_BUF_SIZE)) < 0) {
        perror("write() error!");
        exit(-2);
    }
    close(fd);
}
bool is_stringEmpty(char *buf) {
    for (int i = 0; i < MAX_BUF_SIZE; i++) {
        if (buf[i] == '\0')
            return false;
    }
}

string deleteEvent(int year, int month, int date, int index) {

    if (numOfEvents(year, month, date) == 0) {
        gotoxy(103, 16);
        printWithBg(whte, blue, "nothing to delete");
        sleep(2);
        return NULL;
    }
    int fd, fd1, num = 1;
    int numOfIncomplete = numOfIncompletEvents(year, month, date);

    string dirname = convert(year) + convert(month) + convert(date);
    string pathname;
    string pathname1 = dirname + '/' + "temp.txt";
    ssize_t rSize = 0;
    string eventToDelete = "";
    char *buf = (char *)malloc(MAX_BUF_SIZE);

    if (numOfIncomplete >= index)
        pathname = dirname + '/' + INCOMPLETE;

    else {
        pathname = dirname + '/' + COMPLETE;
        index -= numOfIncomplete;
    }

    if ((fd = open(pathname.c_str(), O_RDWR | O_CREAT, PERMS)) < 0 ||
        (fd1 = open(pathname1.c_str(), O_RDWR | O_CREAT, PERMS)) < 0) {
        perror("open() error!");
        exit(-1);
    }
    do {
        memset(buf, '\0', MAX_BUF_SIZE);
        if ((rSize = read(fd, buf, MAX_BUF_SIZE)) < 0) {
            perror("read() error!");
            exit(-3);
        }

        if (rSize < 1)
            break;
        if (num++ == index) {
            eventToDelete = buf;
            continue;
        }

        if ((write(fd1, buf, MAX_BUF_SIZE)) < 0) {
            perror("write() error!");
            exit(-2);
        }

    } while (rSize > 0);

    close(fd);
    close(fd1);

    remove(pathname.c_str());
    rename(pathname1.c_str(), pathname.c_str());

    return eventToDelete;
}
void printList(int year, int month, int date) {
    int fd, y = 12;
    string dirname = convert(year) + convert(month) + convert(date);
    string pathname = dirname + '/' + INCOMPLETE;
    ssize_t rSize = 0;
    struct stat statBuf;

    resetDisplay(95, 12, 33, 20);

    if (!checkFileExists(dirname)) {
        printNothingToDo();
        return;
    }
    char *buf = (char *)malloc(MAX_BUF_SIZE);

    if ((fd = open(pathname.c_str(), O_RDONLY, PERMS)) < 0) {
        perror("open() error!");
        exit(-1);
    }
    gotoxy(102, y);
    y += 2;
    printWithBg(blue, blck, "[ TO-DO LIST ]");

    do {
        memset(buf, '\0', MAX_BUF_SIZE);
        if ((rSize = read(fd, buf, MAX_BUF_SIZE)) < 0) {
            perror("read() error!");
            exit(-3);
        }
        if (rSize < 1)
            break;
        gotoxy(95, y);
        y += 2;
        printWithBg(whte, blck, "☐  ");
        printWithBg(whte, blck, buf);

    } while (rSize > 0);

    close(fd);

    pathname = dirname + '/' + COMPLETE;
    if (!checkFileExists(pathname))
        return;

    if ((fd = open(pathname.c_str(), O_RDONLY, PERMS)) < 0) {
        perror("open() error!");
        exit(-1);
    }

    do {
        memset(buf, '\0', MAX_BUF_SIZE);
        if ((rSize = read(fd, buf, MAX_BUF_SIZE)) < 0) {
            perror("read() error!");
            exit(-3);
        }
        if (rSize < 1)
            break;
        gotoxy(95, y);
        y += 2;
        printWithBg(whte, blck, "✓  ");
        printWithBg(whte, blck, buf);

    } while (rSize > 0);

    close(fd);
}

void gotoDate(int *year, int *month, int *date) {
    resetDisplay(97, 7, 28, 3);
    int temp = *date;

    gotoxy(97, 7);
    printWithBg(whte, blck, "date: ");
    cin >> *year >> *month >> *date;

    printCalendar(*year, *month, *date);
    cin.ignore();
}

void markEvent(int year, int month, int date, int index) {
    string eventToMark = deleteEvent(year, month, date, index);
    string dirname = convert(year) + convert(month) + convert(date);
    string pathname;
    int fd;

    char *buf = (char *)malloc(MAX_BUF_SIZE);

    if (numOfIncompletEvents(year, month, date) >= index)
        pathname = dirname + '/' + COMPLETE;
    else {
        pathname = dirname + '/' + INCOMPLETE;
        index -= numOfIncompletEvents(year, month, date);
    }

    if ((fd = open(pathname.c_str(), O_RDWR | O_CREAT | O_APPEND, PERMS)) < 0) {
        perror("open() error!");
        exit(-1);
    }

    if ((write(fd, eventToMark.c_str(), MAX_BUF_SIZE)) < 0) {
        perror("write() error!");
        exit(-2);
    }
    close(fd);
}

void plannerMenuCreate() {
    resetDisplay(97, 7, 32, 3);
    gotoxy(97, 7);
    printWithBg(blue, blck, "> Create");
    gotoxy(97, 9);
    printWithBg(whte, blck, "  Go to date");
    gotoxy(112, 7);
    printWithBg(whte, blck, "  Delete");
    gotoxy(112, 9);
    printWithBg(whte, blck, "  Previous");
}
void plannerMenuGotoDate() {
    resetDisplay(97, 7, 32, 3);

    gotoxy(97, 7);
    printWithBg(whte, blck, "  Create");
    gotoxy(97, 9);
    printWithBg(blue, blck, "> Go to date");
    gotoxy(112, 7);
    printWithBg(whte, blck, "  Delete");
    gotoxy(112, 9);
    printWithBg(whte, blck, "  Previous");
}
void plannerMenuPrevious() {
    resetDisplay(97, 7, 32, 3);
    gotoxy(97, 7);
    printWithBg(whte, blck, "  Create");
    gotoxy(97, 9);
    printWithBg(whte, blck, "  Go to date");
    gotoxy(112, 7);
    printWithBg(whte, blck, "  Delete");
    gotoxy(112, 9);
    printWithBg(blue, blck, "> Previous");
}
void plannerMenuDelete() {
    resetDisplay(97, 7, 32, 3);

    gotoxy(97, 7);
    printWithBg(whte, blck, "  Create");
    gotoxy(97, 9);
    printWithBg(whte, blck, "  Go to date");
    gotoxy(112, 7);
    printWithBg(blue, blck, "> Delete");
    gotoxy(112, 9);
    printWithBg(whte, blck, "  Previous");
}

int numOfEvents(int year, int month, int date) {
    string dirname = convert(year) + convert(month) + convert(date);
    string pathname = dirname + "/" + INCOMPLETE;
    char *buf = (char *)malloc(MAX_BUF_SIZE);
    int fd;
    ssize_t rSize = 0;
    int num = 0;

    if (!checkFileExists(dirname)) {
        return 0;
    }
    if ((fd = open(pathname.c_str(), O_RDONLY, PERMS)) < 0) {
        perror("open() error!");
        exit(-1);
    }

    do {
        memset(buf, '\0', MAX_BUF_SIZE);
        if ((rSize = read(fd, buf, MAX_BUF_SIZE)) < 0) {
            perror("read() error!");
            exit(-3);
        }
        if (rSize < 1)
            break;
        num++;
    } while (rSize > 0);

    pathname = dirname + "/" + COMPLETE;
    if (!checkFileExists(pathname)) {
        return num;
    }

    if ((fd = open(pathname.c_str(), O_RDONLY, PERMS)) < 0) {
        perror("open() error!");
        exit(-1);
    }

    do {
        memset(buf, '\0', MAX_BUF_SIZE);
        if ((rSize = read(fd, buf, MAX_BUF_SIZE)) < 0) {
            perror("read() error!");
            exit(-3);
        }
        if (rSize < 1)
            break;
        num++;
    } while (rSize > 0);

    return num;
}
int numOfIncompletEvents(int year, int month, int date) {
    string dirname = convert(year) + convert(month) + convert(date);
    string pathname = dirname + "/" + INCOMPLETE;
    char *buf = (char *)malloc(MAX_BUF_SIZE);
    int fd;
    ssize_t rSize = 0;
    int num = 0;

    if (!checkFileExists(dirname)) {
        return 0;
    }
    if ((fd = open(pathname.c_str(), O_RDONLY, PERMS)) < 0) {
        perror("open() error!");
        exit(-1);
    }

    do {
        memset(buf, '\0', MAX_BUF_SIZE);
        if ((rSize = read(fd, buf, MAX_BUF_SIZE)) < 0) {
            perror("read() error!");
            exit(-3);
        }
        if (rSize < 1)
            break;
        num++;
    } while (rSize > 0);

    return num;
}

bool checkFileExists(std::string filename) {
    if (access(filename.c_str(), F_OK) < 0) {
        return 0;
    }
    return 1;
}

void printNothingToDo() {
    gotoxy(102, 14);
    printWithBg(blue, blck, "[ TO-DO LIST ]");
    gotoxy(103, 16);
    printWithBg(whte, blck, "nothing to do");
}
