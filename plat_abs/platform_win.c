// PLATFORM ABSTRACTION FOR WINDOWS [WIN 10/11 ONLY]
#include <stdbool.h>
#include <windows.h>

void EnableANSI() {
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD dwMode = 0;
    GetConsoleMode(hOut, &dwMode);
    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hOut, dwMode);

    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(hOut, &cursorInfo);
    cursorInfo.bVisible = FALSE;
    SetConsoleCursorInfo(hOut, &cursorInfo);
}

void ClearConsole() {
    system("cls");
}

bool GetConsoleSize(int *rows, int *columns) {
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (!GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi)) {
        return false;
    }
    *columns = csbi.srWindow.Right - csbi.srWindow.Left;
    *rows = csbi.srWindow.Bottom - csbi.srWindow.Top - 1;
    return true;
}

void sleep(int milliseconds) {
    Sleep(milliseconds);
}
