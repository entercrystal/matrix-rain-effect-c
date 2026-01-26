#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <stdbool.h>

#define PRINT_CHAR "*"
#define EMPTY_CHAR " "
#define LINE_BREAK_CHAR "\n"

#define ROWS 45 // 27
#define COLUMNS 156 // 220

// #define CLEAR_CHARACTER "\033[H"

#define WHITE        "\033[97m"
#define BRIGHT_GREEN "\033[92m"  // almost neon
#define GREEN1       "\033[32;1m" // bright-ish green
#define GREEN2       "\033[32m"   // normal green
#define GREEN3       "\033[32;2m" // dim green
#define GREEN4       "\033[32;90m" // very dim / almost black
#define BLACK        "\033[30m"   // optional, just in case
#define RESET "\033[0m"

#define REFRESH_SPEED 20 // milliseconds

#define LENGTH_MIN 6
#define LENGTH_MAX 11

int ranint(int min, int max) {
    if (min > max) return -1;
    if (min == max) return min;
    return rand() % (max - min + 1) + min;
}

float ranfloat(float min, float max) {
    if (min > max) return -1;
    if (min == max) return min;
    return min + (max - min) * (rand() / (float) RAND_MAX);
}

void enable_ansi() {
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

int main() {
    setvbuf(stdout, NULL, _IONBF, 0); // disable stdout buffering
    system("cls");
    
    printf("Green Modern Matrix Rain Effect Generator\nEnter a valid seed (integer, 0 for default): ");
    
    int seed_;
    scanf(" %d", &seed_);

    if (seed_ == 0 || !seed_) {
        seed_ = time(NULL);
        srand(seed_);
    } else srand(seed_);
    
    printf("Seed: %d\nPress ctrl + c to terminate any time\n", seed_);

    system("pause");
    system("cls");

    int velocities[COLUMNS];
    static float heads[COLUMNS]; // static float so it's zero-initialised
    static float tails[COLUMNS]; // static float again

    for (int column = 0; column < COLUMNS; column++) {
        velocities[column] = ranint(3, 15);
        heads[column] = -ranint(2, 5);
        tails[column] = heads[column] - ranint(LENGTH_MIN, LENGTH_MAX);
    }

    enable_ansi();
    float delta = REFRESH_SPEED / 1000.0f * 3.0f;

    const int frame_size = ROWS * COLUMNS * 300 + ROWS + 1;
    
    char *frame = malloc(frame_size); // + 1 for the null terminator
    frame[0] = '\0'; // initialise as empty string

    while (true) {
        char *p = frame;
        *p = '\0';

        for (int column = 0; column < COLUMNS; column++) {
            if (ranint(1, 500) == 1) velocities[column]++;
            const float vel = velocities[column] * delta;

            heads[column] += vel;
            tails[column] += vel;

            if (heads[column] < 0 && tails[column] < 0) continue; // the head and tail are out of bounds, so we skip it completely
            if (heads[column] > ROWS - 1 && tails[column] > ROWS - 1) {
                heads[column] = -ranfloat(4.0f, 8.0f);
                tails[column] = heads[column] - ranint(LENGTH_MIN, LENGTH_MAX);
                continue;
            } // reset and skipped as the drop is out of bounds
        }

        for (int row = 0; row < ROWS; row++) {
            for (int column = 0; column < COLUMNS; column++) {
                int character = 0;

                const float head_pos = heads[column];
                const float tail_pos = tails[column];
                
                const int hpos = (int) head_pos;
                const int tpos = (int) tail_pos;
                
                if (row <= hpos && row >= tpos) {
                    // current row is within the head and tail of the rain drop
                    if (row == hpos) character = 2;
                    else if (row == tpos) character = 3;
                    else if (row == tpos + 1) character = -4;
                    else if (row == tpos + 2) character = -3;
                    else if (row == tpos + 3) character = -2;
                    else if (row == tpos + 4) character = -1;
                    else character = 1;
                }
                
                switch (character) {
                    /* CODES:
                    0: nothing
                    1: bright green
                    2: white
                    3: black
                    -1: green1
                    -2: green2
                    -3: green3
                    -4: green4

                    ALL EXCEPT 0 INCLUDE RESET CHARACTER
                    */

                    case 1:
                        p += sprintf(p, "%s%s%s", BRIGHT_GREEN, PRINT_CHAR, RESET);
                        break;

                    case 2:
                        p += sprintf(p, "%s%s%s", WHITE, PRINT_CHAR, RESET);
                        break;

                    case 3:
                        p += sprintf(p, "%s%s%s", BLACK, PRINT_CHAR, RESET);
                        break;

                    case -1:
                        p += sprintf(p, "%s%s%s", GREEN1, PRINT_CHAR, RESET);
                        break;

                    case -2:
                        p += sprintf(p, "%s%s%s", GREEN2, PRINT_CHAR, RESET);
                        break;

                    case -3:
                        p += sprintf(p, "%s%s%s", GREEN3, PRINT_CHAR, RESET);
                        break;

                    case -4:
                        p += sprintf(p, "%s%s%s", GREEN4, PRINT_CHAR, RESET);
                        break;

                    case 0:
                    default: // just safe i guess?
                        *p++ = ' '; // EMPTY_CHAR
                        break;
                }
            }
            *p++ = '\n';
        }
        
        *p = '\0';
        
        printf("\033[H");
        printf("%s", frame);

        Sleep(REFRESH_SPEED);
    }

    free(frame);
    return 0;
}
