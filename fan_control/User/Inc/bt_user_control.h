#ifndef BT_USER_CONTROL
#define BT_USER_CONTROL

#define COMMAND_BUFFER_SIZE 5
#define STRING_SIZE 255

typedef struct
{
  char buffer[COMMAND_BUFFER_SIZE][STRING_SIZE];
  volatile unsigned int head;
  volatile unsigned int tail;
} ring_command_buffer;

/* BEGIN Prototypes */
void CommandBuf_init(void);
void bt_user_control(void);

/* END Prototypes */

#endif /* BT_USER_CONTROL */
