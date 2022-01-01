#include "visualization.h"

// Creates a new paddle
void new_paddle (paddle_position_t * paddle, int legth){
    paddle->x = WINDOW_SIZE/2;
    paddle->y = WINDOW_SIZE-2;
    paddle->length = legth;
}

// Moves the paddle (update its position)
void moove_paddle (paddle_position_t * paddle, int direction){
    if (direction == KEY_UP){
        if (paddle->y  != 1){
            paddle->y --;
        }
    }
    if (direction == KEY_DOWN){
        if (paddle->y  != WINDOW_SIZE-2){
            paddle->y ++;
        }
    }
    if (direction == KEY_LEFT){
        if (paddle->x - paddle->length != 1){
            paddle->x --;
        }
    }
    if (direction == KEY_RIGHT)
        if (paddle->x + paddle->length != WINDOW_SIZE-2){
            paddle->x ++;
    }
}

// Draws the paddle on the screen
void draw_paddle(WINDOW *win, paddle_position_t * paddle, int del){
    int ch;
    if(del){
        ch = '_';
    }else{
        ch = ' ';
    }
    int start_x = paddle->x - paddle->length;
    int end_x = paddle->x + paddle->length;
    for (int x = start_x; x <= end_x; x++){
        wmove(win, paddle->y, x);
        waddch(win,ch);
    }
    wrefresh(win);
}

// Creates a new ball (places it randomly on the screen)
void place_ball_random(ball_position_t * ball){
    ball->x = rand() % WINDOW_SIZE ;
    ball->y = rand() % WINDOW_SIZE ;
    ball->c = 'o';
    ball->up_hor_down = rand() % 3 -1; //  -1 up, 1 - down
    ball->left_ver_right = rand() % 3 -1 ; // 0 vertical, -1 left, 1 right
}

// Moves the ball (update its position)
void moove_ball(ball_position_t * ball){
    
    int next_x = ball->x + ball->left_ver_right;
    if( next_x == 0 || next_x == WINDOW_SIZE-1){
        ball->up_hor_down = rand() % 3 -1 ;
        ball->left_ver_right *= -1;
        mvwprintw(message_win, 3,1,"left right win");
        wrefresh(message_win);
     }else{
        ball->x = next_x;
    }

    
    int next_y = ball->y + ball->up_hor_down;
    if( next_y == 0 || next_y == WINDOW_SIZE-1){
        ball->up_hor_down *= -1;
        ball->left_ver_right = rand() % 3 -1;
        mvwprintw(message_win, 3,1,"bottom top win");
        wrefresh(message_win);
    }else{
        ball -> y = next_y;
    }
}

// Draws the ball on the screen
void draw_ball(WINDOW *win, ball_position_t * ball, int draw){
    int ch;
    if(draw){
        ch = ball->c;
    }else{
        ch = ' ';
    }
    wmove(win, ball->y, ball->x);
    waddch(win,ch);
    wrefresh(win);
}

// Update the ball movement when it is hit by the paddle
void paddle_hit_ball(ball_position_t * ball, paddle_position_t * paddle){
    
    int start_x = paddle->x - paddle->length;
    int end_x = paddle->x + paddle->length;
    // Check if y positions are the same
    if (ball->y == paddle->y){
        // Run through the whole length of the paddle
        for (int i = start_x; i <= end_x; i++){
            if (ball->x == i){
                // (ball->up_hor_down == 0) ? (ball->up_hor_down = 1) : (ball->up_hor_down *= -1);
                if (ball->up_hor_down == 0){
                    do{
                        ball->up_hor_down = rand() % 3 -1;
                    }while(ball->up_hor_down == 0);
                }
                else
                    ball->up_hor_down *= -1;
                ball->y=ball->y + ball->up_hor_down;
            }
        }
    }
}