#define WINDOW_SIZE 20
#define PADLE_SIZE 2

#define SOCK_ADDRESS "/tmp/sock_16"


typedef struct message
{   
    char msg_type;  // 'c' - Connect; 'r' - Release_ball; 's' - Send_ball; 'm' - Move_ball; 'd' - Disconnect
}message;

/*    
    ball_position_t ball_position;
    paddle_position_t paddle_position; 

*/    

