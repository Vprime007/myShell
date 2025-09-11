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
static bool is_shell_init(void);

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
/***************************************************************************//*!
*  \brief Is shell initialized.
*
*   This function return TRUE if the Shell is initialized and ready to use.
*   It will return FALSE otherwise.
*   
*   Preconditions: None.
*
*   Side Effects: None.
*
*   \return     (Init -> true / Not init -> false)
*
*******************************************************************************/
static bool is_shell_init(void){
    return (shell_print_callback != NULL);
}

/***************************************************************************//*!
*  \brief Send char
*
*   This function is used to send char to the Shell dedicated UART port.
*   
*   Preconditions: None.
*
*   Side Effects: None.
*
*   \param[in]  c                   Char to send
*
*******************************************************************************/
static void send_char(char c){

    if(!is_shell_init()){
        return;
    }

    shell_print_callback(c);
}

/***************************************************************************//*!
*  \brief Send echo.
*
*   This function is used to send Shell echo to the dedicated UART port.
*   
*   Preconditions: None.
*
*   Side Effects: None.
*
*   \param[in]  c                   Char to echo.
*
*******************************************************************************/
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

/***************************************************************************//*!
*  \brief Send string echo. 
*
*   This function is used to send Shell string echo to the dedicated UART port.
*   
*   Preconditions: None.
*
*   Side Effects: None.
*
*   \param[in]  str                 String to echo.
*
*******************************************************************************/
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
/***************************************************************************//*!
*  \brief Shell initialization.
*
*   This function perform the shell module initialization. The print_callback
*   parameter contain a pointer to the function that the shell will use to
*   print out to the UART port.
*   
*   Preconditions: None.
*
*   Side Effects: None.
*
*   \param[in]  print_callback          Callback to print Shell output.
*
*   \return     Operation status
*
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

/***************************************************************************//*!
*  \brief Shell Receive char.
*
*   This function is use to pass incoming char data from the 
*   UART port to the shell.
*   
*   Preconditions: None.
*
*   Side Effects: None.
*
*   \param[in]  c                   Incoming char.
*
*   \return     Operation status
*
*******************************************************************************/
SHELL_Ret_t SHELL_RecvChar(char c){

    if(c == '\r' || is_rx_buffer_full() || !is_shell_init()){
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

/***************************************************************************//*!
*  \brief Shell Help handler.
*
*   Shell 'help' function handler use to print out all available commands
*   with their respective descriptions. 
*   
*   Preconditions: None.
*
*   Side Effects: None.
*
*   \param[in]  argc                Number of command params.
*   \param[in]  argv                Pointer to command params values.
*
*   \return     Operation status
*
*******************************************************************************/
int32_t SHELL_HelpHandler(int32_t argc, char *argv[]){

    SHELL_FOR_EACH_COMMAND(command){
        send_echo_str(command->name);
        send_echo_str(": ");
        send_echo_str(command->help);
        send_echo('\n');
    }

    return 0;
}

/***************************************************************************//*!
*  \brief Shell Put line
*
*   This function is use to write string via the Shell.
*   It is mostly use to respond to the shell commands.
*   
*   Preconditions: None.
*
*   Side Effects: None.
*
*   \param[in]  str             String to send.
*
*******************************************************************************/
void SHELL_PutLine(const char *str){

    send_echo_str(str);
    send_echo('\n');
}

/******************************************************************************
*   Interrupts
*******************************************************************************/

