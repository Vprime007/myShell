/******************************************************************************
*   Includes
*******************************************************************************/
#include <stdbool.h>
#include <string.h>

#include "myShell_cfg.h"
#include "myShell.h"

/******************************************************************************
*   Private Definitions
*******************************************************************************/
#define SHELL_FOR_EACH_COMMAND(command) \
    for (const SHELL_Command_t *command = commands_table; \
         command < &commands_table[nb_commands]; \
         ++command)

/******************************************************************************
*   Private Macros
*******************************************************************************/


/******************************************************************************
*   Private Data Types
*******************************************************************************/


/******************************************************************************
*   Private Functions Declaration
*******************************************************************************/
static bool shell_booted(void);

static void send_char(char c);
static void send_echo(char c);
static void send_echo_str(const char *str);
static void send_prompt(void);

static char last_char(void);
static bool is_rx_buffer_full(void);
static void reset_rx_buffer(void);

static const SHELL_Command_t *find_command(const char *name);
static void process_char(void);

/******************************************************************************
*   Public Variables
*******************************************************************************/


/******************************************************************************
*   Private Variables
*******************************************************************************/
static SHELL_Command_t *commands_table = NULL;
static uint32_t nb_commands = 0;

static SHELL_PrintCallback_t shell_print_callback = NULL;
static uint32_t rx_size = 0;
static char rx_buffer[SHELL_RX_BUFFER_SIZE] = {0};

/******************************************************************************
*   Private Functions Definitions
*******************************************************************************/
static bool shell_booted(void){
    return (shell_print_callback != NULL);
}

static void send_char(char c){

    if(!shell_booted()){
        return;
    }

    shell_print_callback(c);
}

static void send_echo(char c){

    if('\n' == c){
        send_char('\r');
        send_char('\n');
    }
    else if('\b' == c){
        send_char('\b');
        send_char(' ');
        send_char('\b');
    }
    else{
        send_char(c);
    }
}

static void send_echo_str(const char *str){
    for(const char *c=str; *c != '\0'; c++){
        send_echo(*c);
    }
}

static void send_prompt(void){
    send_echo_str(SHELL_PROMPT);
}

static char last_char(void){
    return rx_buffer[rx_size-1];
}

static bool is_rx_buffer_full(void){
    return rx_size >= SHELL_RX_BUFFER_SIZE;
}

static void reset_rx_buffer(void){
    memset(rx_buffer, 0, sizeof(rx_buffer));
    rx_size = 0;
}

static const SHELL_Command_t *find_command(const char *name){

    SHELL_FOR_EACH_COMMAND(command){
        if(strcmp(command->name, name) == 0){
            return command;
        }
    }

    return NULL;
}

static void process_char(void){

    if((last_char() != '\n') && (!is_rx_buffer_full())){
        return;
    }

    char *argv[SHELL_MAX_ARGS] = {0};
    uint32_t argc = 0;

    char *next_arg = NULL;
    for(uint32_t i=0; i<rx_size && argc < SHELL_MAX_ARGS; ++i){
        
        char *const c = &rx_buffer[i];
        if(*c == ' ' || *c == '\n' || i == rx_size-1){

            *c = '\0';
            if(next_arg){
                argv[argc++] = next_arg;
                next_arg = NULL;
            }
        }
        else if(!next_arg){
            next_arg = c;
        }
        else{
            //Do nothing...
        }
    }

    if(rx_size == SHELL_RX_BUFFER_SIZE){
        send_echo('\n');
    }

    if(argc >= 1){
        
        const SHELL_Command_t *command = find_command(argv[0]);
        if(!command){
            send_echo_str("Unknown command: ");
            send_echo_str(argv[0]);
            send_echo('\n');
            send_echo_str("Type 'help' to list all commands\n");
        }
        else{
            command->handler(argc, argv);
        }
    }

    reset_rx_buffer();
    send_prompt();
}

/******************************************************************************
*   Public Functions Definitions
*******************************************************************************/
SHELL_Ret_t SHELL_Init(SHELL_PrintCallback_t print_callback){

    //Check if param is valid
    if(print_callback == NULL){
        return SHELL_STATUS_ERROR;
    }

    ESP_LOGI(TAG, "myShell initialization");

    //Fetch commands table and number of command
    SHELL_Commands_Context_t table_context = SHELL_CFG_GetCommandTable();
    commands_table = table_context.pTable;
    nb_commands = table_context.nb_command;

    ESP_LOGI(TAG, "Cmd table of %u command(s)", nb_commands);

    //register print callback
    shell_print_callback = print_callback;

    //Reset reception buffer
    reset_rx_buffer();
    send_prompt();

    return SHELL_STATUS_OK;
}

SHELL_Ret_t SHELL_RecvChar(char c){

    if(c == '\r' || is_rx_buffer_full() || !shell_booted()){
        return SHELL_STATUS_ERROR;
    }
    send_echo(c);

    if(c == '\b'){
        rx_buffer[--rx_size] = '\0';
        return SHELL_STATUS_OK;
    }
    rx_buffer[rx_size++] = c;

    process_char();

    return SHELL_STATUS_OK;
}

int32_t SHELL_HelpHandler(int32_t argc, char *argv[]){

    SHELL_FOR_EACH_COMMAND(command){
        send_echo_str(command->name);
        send_echo_str(": ");
        send_echo_str(command->help);
        send_echo('\n');
    }

    return 0;
}

void SHELL_PutLine(const char *str){

    send_echo_str(str);
    send_echo('\n');
}

/******************************************************************************
*   Interrupts
*******************************************************************************/

