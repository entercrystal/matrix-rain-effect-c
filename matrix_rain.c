// The Matrix Rain Effect Generator
// by entercrystal, Jan. 2026

#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include "tools.h"

#define PRINT_CHAR "*" // character to print if RANDOM_CHARACTERS is false
#define LINE_BREAK_CHAR '\n'
#define RANDOM_CHARACTERS true // true = random characters instead of print_char, false = print_char only

#define RANDOM_CHARACTERS_USE_FIXED_SET false // if RANDOM_CHARACTERS is true, set this to true as well in order to use a fixed set of characters instead of fully random ASCII characters
#define RANDOM_FIXED_CHARACTER_SET "@#/.,^&*()[]{}<>~!?\\|" // random character set if both RANDOM_CHARACTERS and RANDOM_CHARACTERS_USE_FIXED_SET are true

#define CLEAR_CHARACTER "\033[H" // ANSI escape code to clear the console
#define WHITE        "\033[97m"
#define BRIGHT_GREEN "\033[92m"  // almost neon
#define GREEN1       "\033[32;1m" // bright-ish green
#define GREEN2       "\033[32m"   // normal green
#define GREEN3       "\033[32;2m" // dim green
#define GREEN4       "\033[32;90m" // very dim / almost black
#define BLACK        "\033[30m"   // optional, just in case
#define RESET "\033[0m"

#define LENGTH_MIN 6 // minimum length of a rain drop
#define LENGTH_MAX 11 // maximum length of a rain drop

#define HEAD_FRAMES_MIN 2.0f // 2.0f
#define HEAD_FRAMES_MAX 12.5f // 5.0f

#define DROP_VELOCITY_MIN 6.0f // min velocity (higher = faster), 3.0f
#define DROP_VELOCITY_MAX 22.5f // max velocity (higher = faster), 17.5f

#define SPEED_CHANGE_CHANCE_ENABLED true
#define SPEED_CHANGE_CHANCE 500 // 1 in X chance per frame of receiving a speed increase for each drop

#define FRAMERATE 120 // average framerate target (higher = smoother, but more CPU usage)
#define SHOW_FPS false // enable or disable fps counter

#define GRAVITY 5.0f // row per second squared (gravity acceleration, put 0 to disable gravity effect)

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

int main() {
    const float FRAMERATE_FLOAT = 1 / (float) FRAMERATE * 1000.0f;
    const int REFRESH_SPEED = (int) (FRAMERATE_FLOAT); // in milliseconds
    // printf("Refresh speed set to %d ms (%d FPS target)\n", REFRESH_SPEED, FRAMERATE); // for debugging
    
    setvbuf(stdout, NULL, _IOFBF, 0); // switch to full buffering mode
    ClearConsole();
    
    printf("Green Modern Matrix Rain Effect Generator\nEnter a valid seed (integer, 0 for default): ");
    
    int seed_;
    scanf(" %d", &seed_);

    if (seed_ == 0 || !seed_) {
        seed_ = time(NULL);
        srand(seed_);
    } else srand(seed_);
    
    printf("Seed: %d\nPress ctrl + c to terminate any time\n", seed_);

    system("pause");
    ClearConsole();

    int COLUMNS, ROWS;
    if (!GetConsoleSize(&ROWS, &COLUMNS)) {
        printf("Could not resolve console size...\n");
        return 1;
    }

    int *velocities = malloc(sizeof(int) * COLUMNS);
    float *heads = calloc(COLUMNS, sizeof(float));
    float *tails = calloc(COLUMNS, sizeof(float)); // calloc so they are zero-initialized

    for (int column = 0; column < COLUMNS; column++) {
        velocities[column] = ranint(DROP_VELOCITY_MIN, DROP_VELOCITY_MAX);
        heads[column] = -ranfloat(HEAD_FRAMES_MIN, HEAD_FRAMES_MAX);
        tails[column] = heads[column] - ranint(LENGTH_MIN, LENGTH_MAX);
    }

    EnableANSI();
    float delta = 0.0f;
    clock_t last_time = clock();

    const int starting_frame_size = ROWS * COLUMNS * 75 + ROWS + 1;
    
    char *frame = malloc(starting_frame_size); // + 1 for the null terminator
    frame[0] = '\0'; // initialise as empty string

    while (true) {
        const int old_rows = ROWS, old_columns = COLUMNS;

        if (!GetConsoleSize(&ROWS, &COLUMNS)) {
            printf("Failed to get console size...\n");
            break;
        }

        if (old_rows != ROWS || old_columns != COLUMNS) {
            free(velocities);
            free(heads);
            free(tails);

            velocities = malloc(sizeof(int) * COLUMNS);
            heads = calloc(COLUMNS, sizeof(float));
            tails = calloc(COLUMNS, sizeof(float)); // calloc so they are zero-initialized

            for (int column = 0; column < COLUMNS; column++) {
                velocities[column] = ranfloat(DROP_VELOCITY_MIN, DROP_VELOCITY_MAX);
                heads[column] = -ranfloat(HEAD_FRAMES_MIN, HEAD_FRAMES_MAX);
                tails[column] = heads[column] - ranint(LENGTH_MIN, LENGTH_MAX);
            }
            
            free(frame);

            const int frame_size = ROWS * COLUMNS * 75 + ROWS + 1;
            frame = malloc(frame_size); // + 1 for the null terminator

            frame[0] = '\0'; // initialise as empty string
            ClearConsole();
        }

        char *p = frame;
        *p = '\0';

        for (int column = 0; column < COLUMNS; column++) {
            if (SPEED_CHANGE_CHANCE_ENABLED && ranint(1, SPEED_CHANGE_CHANCE) == 1) velocities[column]++; // 1 in x chance to increase speed randomly (per drop) if functionality is enabled
            const float vel = velocities[column] * delta;

            heads[column] += vel; // update positions according to velocity and delta time
            tails[column] += vel;

            if (GRAVITY > 0.0f) {
                velocities[column] += GRAVITY * delta; // apply gravity effect (acceleration) if enabled
            }

            if (heads[column] < 0 && tails[column] < 0) continue; // the head and tail are out of bounds, so we skip it completely
            if (heads[column] > ROWS - 1 && tails[column] > ROWS - 1) {
                velocities[column] = ranfloat(DROP_VELOCITY_MIN, DROP_VELOCITY_MAX);
                heads[column] = -ranfloat(HEAD_FRAMES_MIN, HEAD_FRAMES_MAX);
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

                char USE_CHAR[3];
                
                if (RANDOM_CHARACTERS)
                    if (!RANDOM_CHARACTERS_USE_FIXED_SET)
                        sprintf(USE_CHAR, "%c", (char)ranint(33, 126));
                    else
                        sprintf(USE_CHAR, "%c", RANDOM_FIXED_CHARACTER_SET[ranint(0, (int)strlen(RANDOM_FIXED_CHARACTER_SET) - 1)]);
                else
                    sprintf(USE_CHAR, "%s", PRINT_CHAR);
                
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
                        p += sprintf(p, "%s%s%s", BRIGHT_GREEN, USE_CHAR, RESET);
                        break;

                    case 2:
                        p += sprintf(p, "%s%s%s", WHITE, USE_CHAR, RESET);
                        break;

                    case 3:
                        p += sprintf(p, "%s%s%s", BLACK, USE_CHAR, RESET);
                        break;

                    case -1:
                        p += sprintf(p, "%s%s%s", GREEN1, USE_CHAR, RESET);
                        break;

                    case -2:
                        p += sprintf(p, "%s%s%s", GREEN2, USE_CHAR, RESET);
                        break;

                    case -3:
                        p += sprintf(p, "%s%s%s", GREEN3, USE_CHAR, RESET);
                        break;

                    case -4:
                        p += sprintf(p, "%s%s%s", GREEN4, USE_CHAR, RESET);
                        break;

                    case 0:
                    default: // just safe i guess?
                        *p++ = ' '; // EMPTY_CHAR
                        break;
                }
            }
            *p++ = LINE_BREAK_CHAR;
        }
        
        *p = '\0';
        
        printf(CLEAR_CHARACTER); // bring cursor to home position
        printf("%s", frame); // print out the entire frame at once

        fflush(stdout);
        sleep(REFRESH_SPEED);
        
        clock_t current_time = clock();
        delta = (float)(current_time - last_time) / CLOCKS_PER_SEC;
        
        last_time = current_time;
        // printf("Delta time: %.4f seconds\n", delta); // for debugging
        printf(SHOW_FPS ? "FPS: %d\n" : "", (int) (1 / delta));
    }

    free(frame);
    free(velocities);
    free(heads);
    free(tails);
    
    ClearConsole();
    return 0;
}
