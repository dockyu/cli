#include "coro_ttt.h"

#include <setjmp.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "list.h"

#include "agents/mcts.h"
#include "game.h"

struct task {
    jmp_buf env;
    struct list_head list;
    char task_name[30];
};

struct arg {
    char *task_name;
};

static LIST_HEAD(tasklist);
static void (**tasks)(void *);
static struct arg *args;
static int ntasks;
static jmp_buf sched;
static struct task *cur_task;

/* ttt game */
static char table[N_GRIDS];
static int move_record[N_GRIDS];
static int move_count = 0;
static int rounds = 0;
static bool round_end = false;

static void task_add(struct task *task)
{
    list_add_tail(&task->list, &tasklist);
}

static void task_switch()
{
    if (!list_empty(&tasklist)) {
        struct task *t = list_first_entry(&tasklist, struct task, list);
        list_del(&t->list);
        cur_task = t;
        longjmp(t->env, 1);
    }
}

void schedule(void)
{
    static int i;

    setjmp(sched);

    while (ntasks-- > 0) {
        struct arg arg = args[i];
        tasks[i++](&arg);
        printf("Never reached\n");
    }

    task_switch();
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

static void ai_1_task(void *arg)
{
    /* Initial ai_1_task */
    struct task *task = malloc(sizeof(struct task));
    strcpy(task->task_name, ((struct arg *) arg)->task_name);
    INIT_LIST_HEAD(&task->list);
    // printf("Initial %s\n", task->task_name);

    if (setjmp(task->env) == 0) {
        task_add(task);
        longjmp(sched, 1);
    }

    task = cur_task;
    // printf("resume %s\n", task->task_name);

    if (round_end) {
        round_end = false;
    }

    if (rounds <= 0) {
        /* do not add task back to tasklist */
        longjmp(sched, 1);
    }

    char ai = 'X';
    int move = mcts(table, ai);
    if (move != -1) {
        table[move] = ai;
        record_move(move);
    }

    /* next task */
    task_add(task);
    task_switch();
}

static void ai_2_task(void *arg)
{
    /* Initial ai_2_task */
    struct task *task = malloc(sizeof(struct task));
    strcpy(task->task_name, ((struct arg *) arg)->task_name);
    INIT_LIST_HEAD(&task->list);
    // printf("Initial %s\n", task->task_name);

    if (setjmp(task->env) == 0) {
        task_add(task);
        longjmp(sched, 1);
    }

    task = cur_task;
    // printf("resume %s\n", task->task_name);

    if (round_end) {
        task_add(task);
        task_switch();
    }

    if (rounds <= 0) {
        /* do not add task back to tasklist */
        longjmp(sched, 1);
    }

    char ai = 'O';
    int move = mcts(table, ai);
    if (move != -1) {
        table[move] = ai;
        record_move(move);
    }

    /* next task */
    task_add(task);
    task_switch();
}

static void check_win_task(void *arg)
{
    /* Initial check_win_task */
    struct task *task = malloc(sizeof(struct task));
    strcpy(task->task_name, ((struct arg *) arg)->task_name);
    INIT_LIST_HEAD(&task->list);
    // printf("Initial %s\n", task->task_name);

    if (setjmp(task->env) == 0) {
        task_add(task);
        longjmp(sched, 1);
    }

    task = cur_task;
    // printf("resume %s\n", task->task_name);

    if (round_end) {
        task_add(task);
        task_switch();
    }

    if (rounds <= 0) {
        /* do not add task back to tasklist */
        longjmp(sched, 1);
    }

    char win = check_win(table);
    if (win == 'D') {
        draw_board(table);
        printf("It is a draw!\n");
        round_end = true;
    } else if (win != ' ') {
        draw_board(table);
        printf("%c won!\n", win);
        round_end = true;
    }

    if (round_end) {
        print_moves();
        rounds--;
        memset(table, ' ', N_GRIDS);
        move_count = 0;
    }

    /* next task */
    task_add(task);
    task_switch();
}

static void draw_board_task(void *arg)
{
    /* Initial draw_board_task */
    struct task *task = malloc(sizeof(struct task));
    strcpy(task->task_name, ((struct arg *) arg)->task_name);
    INIT_LIST_HEAD(&task->list);
    // printf("Initial %s\n", task->task_name);

    if (setjmp(task->env) == 0) {
        task_add(task);
        longjmp(sched, 1);
    }

    task = cur_task;
    // printf("resume %s\n", task->task_name);

    if (round_end) {
        task_add(task);
        task_switch();
    }

    if (rounds <= 0) {
        /* do not add task back to tasklist */
        longjmp(sched, 1);
    }

    draw_board(table);

    /* next task */
    task_add(task);
    task_switch();
}

static void keyboard_listen_task(void *arg)
{
    /* Initial keyboard_listen_task */
    struct task *task = malloc(sizeof(struct task));
    strcpy(task->task_name, ((struct arg *) arg)->task_name);
    INIT_LIST_HEAD(&task->list);
    printf("Initial %s\n", task->task_name);

    if (setjmp(task->env) == 0) {
        task_add(task);
        longjmp(sched, 1);
    }

    task = cur_task;
    // printf("resume %s\n", task->task_name);

    if (rounds <= 0) {
        /* do not add task back to tasklist */
        longjmp(sched, 1);
    }

    /* TODO: keyboard listen detection */
    // printf("keyboard_listen_task");

    /* next task */
    task_add(task);
    task_switch();
}

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))
void coro_ttt(int times)
{
    rounds = times;
    memset(table, ' ', N_GRIDS);

    void (*registered_task[])(void *) = {ai_1_task, check_win_task, ai_2_task, check_win_task, draw_board_task, keyboard_listen_task};
    struct arg arg0 = {.task_name = "ai_1_task"};
    struct arg arg1 = {.task_name = "check_win_task"};
    struct arg arg2 = {.task_name = "ai_2_task"};
    struct arg arg3 = {.task_name = "check_win_task"};
    struct arg arg4 = {.task_name = "draw_board_task"};
    struct arg arg5 = {.task_name = "keyboard_listen_task"};
    struct arg registered_arg[] = {arg0, arg1, arg2, arg3, arg4, arg5};
    tasks = registered_task;
    args = registered_arg;
    ntasks = ARRAY_SIZE(registered_task);
    schedule();
}