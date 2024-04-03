#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#if defined(__APPLE__)
#include <mach/mach_time.h>
#else /* Assume POSIX environments */
#include <time.h>
#endif

#include "console.h"
#include "report.h"

#include "agents/mcts.h"
#include "game.h"
/* Record the order of moves */
static int move_record[N_GRIDS];
static int move_count = 0;

/* indicates the current mode of ttt */
static enum game_mode ttt_mode = PVE;

static bool do_jack(int argc, char *argv[])
{
    char *something = (argc >= 2) ? argv[1] : "";
    printf("%s jack\n", something);
    return true;
}

static void record_move(int move)
{
    move_record[move_count++] = move;
}

static void print_moves()
{
    printf("Moves: ");
    for (int i = 0; i < move_count; i++) {
        printf("%c%d", 'A' + GET_COL(move_record[i]),
               1 + GET_ROW(move_record[i]));
        if (i < move_count - 1) {
            printf(" -> ");
        }
    }
    printf("\n");
}

static int get_input(char player)
{
    char *line = NULL;
    size_t line_length = 0;
    int parseX = 1;

    int x = -1, y = -1;
    while (x < 0 || x > (BOARD_SIZE - 1) || y < 0 || y > (BOARD_SIZE - 1)) {
        printf("%c> ", player);
        int r = getline(&line, &line_length, stdin);
        if (r == -1)
            exit(1);
        if (r < 2)
            continue;
        x = 0;
        y = 0;
        parseX = 1;
        for (int i = 0; i < (r - 1); i++) {
            if (isalpha(line[i]) && parseX) {
                x = x * 26 + (tolower(line[i]) - 'a' + 1);
                if (x > BOARD_SIZE) {
                    // could be any value in [BOARD_SIZE + 1, INT_MAX]
                    x = BOARD_SIZE + 1;
                    printf("Invalid operation: index exceeds board size\n");
                    break;
                }
                continue;
            }
            // input does not have leading alphabets
            if (x == 0) {
                printf("Invalid operation: No leading alphabet\n");
                y = 0;
                break;
            }
            parseX = 0;
            if (isdigit(line[i])) {
                y = y * 10 + line[i] - '0';
                if (y > BOARD_SIZE) {
                    // could be any value in [BOARD_SIZE + 1, INT_MAX]
                    y = BOARD_SIZE + 1;
                    printf("Invalid operation: index exceeds board size\n");
                    break;
                }
                continue;
            }
            // any other character is invalid
            // any non-digit char during digit parsing is invalid
            // TODO: Error message could be better by separating these two cases
            printf("Invalid operation\n");
            x = y = 0;
            break;
        }
        x -= 1;
        y -= 1;
    }
    free(line);
    return GET_INDEX(y, x);
}

static bool do_ttt(int argc, char *argv[])
{
    if (argc > 2) {
        printf("%s takes too much arguments\n", argv[0]);
        return false;
    }
    if (argc == 2) {
        if (!strcmp(argv[1], "PVE"))
            ttt_mode = PVE;
        else if (!strcmp(argv[1], "EVE"))
            ttt_mode = EVE;
        else {
            printf("%s takes error arguments %s\n", argv[0], argv[1]);
            return false;
        }
    }

    srand(time(NULL));
    char table[N_GRIDS];
    memset(table, ' ', N_GRIDS);
    char turn = 'X';
    char ai = 'O';
    char ai_2 = 'X';

    while (1) {
        char win = check_win(table);
        if (win == 'D') {
            draw_board(table);
            printf("It is a draw!\n");
            break;
        } else if (win != ' ') {
            draw_board(table);
            printf("%c won!\n", win);
            break;
        }

        if (turn == ai) {
            int move = mcts(table, ai);
            if (move != -1) {
                table[move] = ai;
                record_move(move);
            }
        } else {
            draw_board(table);
            if (ttt_mode == PVE) {
                int move;
                while (1) {
                    move = get_input(turn);
                    if (table[move] == ' ') {
                        break;
                    }
                    printf("Invalid operation: the position has been marked\n");
                }
                table[move] = turn;
                record_move(move);
            } else {
                int move = mcts(table, ai_2);
                if (move != -1) {
                    table[move] = ai_2;
                    record_move(move);
                }
            }
        }
        turn = turn == 'X' ? 'O' : 'X';
    }
    print_moves();
    move_count = 0;

    return 0;
}



static void console_init()
{
    ADD_COMMAND(jack, "Say something to jack", "[something]");
    ADD_COMMAND(ttt,
                "Start a Tic-Tac-Toe game. Play against the computer if str "
                "equals PVE. "
                "If str equals EVE, the computer plays against the computer.",
                "[str]");
}

int main(int argc, char *argv[])
{
    init_cmd();
    console_init();
    run_console();
}