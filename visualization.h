#include <stdlib.h>
#include <ncurses.h> 

/* change window and paddle size */
#define WINDOW_SIZE 35
#define PADDLE_SIZE 3

WINDOW * message_win;

/* ball position structure */
typedef struct ball_position_t{
    int x, y;
    int up_hor_down; //  -1 up, 0 horizontal, 1 down
    int left_ver_right; //  -1 left, 0 vertical,1 right
    char c;
} ball_position_t;

/* paddle position structure */
typedef struct paddle_position_t{
    int x, y;
    int length;
} paddle_position_t;

/* Functions */
void new_paddle (paddle_position_t * paddle, int legth);
void moove_paddle (paddle_position_t * paddle, int direction);
void draw_paddle(WINDOW *win, paddle_position_t * paddle, int del);
void place_ball_random(ball_position_t * ball);
void moove_ball(ball_position_t * ball);
void draw_ball(WINDOW *win, ball_position_t * ball, int draw);
void paddle_hit_ball(ball_position_t * ball, paddle_position_t * paddle);