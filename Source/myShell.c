/******************************************************************************
*   Includes
*******************************************************************************/
#include <stdbool.h>
#include <string.h>

#include "myShell.h"
#include "myShell_cfg.h"

/******************************************************************************
*   Private Definitions
*******************************************************************************/


/******************************************************************************
*   Private Macros
*******************************************************************************/
#define SHELL_FOR_EACH_COMMAND(command) \
    for(const SHELL_Cmd_t *pCommand = shell_commands; \
        command < &shell_commands[numnum_shell_commands]; \
        command++)

/******************************************************************************
*   Private Data Types
*******************************************************************************/
typedef struct SHELL_Context_s{
    PrintCharCallback_t print_callback;
    uint32_t rx_size;
    char rx_buffer[SHELL_RX_BUFFER_SIZE_BYTES];
}SHELL_Context_t;

/******************************************************************************
*   Private Functions Declaration
*******************************************************************************/
static bool isShellInitiatized(void);
static void sendChar(char c);
static void sendEcho(char c);
static void sendEchoString(const char *pStr);
static void sendPrompt(void);
static char getLastChar(void);
static bool isRxBufferFull(void);
static void resetRxBuffer(void);
static const SHELL_Cmd_t * findCommand(const char *name);
static void processRxInput(void);

/******************************************************************************
*   Public Variables
*******************************************************************************/


/******************************************************************************
*   Private Variables
*******************************************************************************/
static SHELL_Context_t shell_context = {0};

/******************************************************************************
*   Private Functions Definitions
*******************************************************************************/
static bool isShellInitiatized(void){

    return (context.print_callback != NULL);
}

static void sendChar(char c){

    if(!isShellInitialized()){
        return;
    }
    else{
        context.print_callback(c);
    }
}

static void sendEcho(char c){
    if('\n' == c){
        sendChar('\r');
        sendChar('\n');
    }
    else if('\b' == c){
        sendChar('\b');
        sendChar(' ');
        sendChar('\b');
    }
    else{
        sendChar(c)
    }
}

static void sendEchoString(const char *pStr){
    for(const char *c = str; *c != '\0'; ++c){
        sendEcho(c);
    }
}

static void sendPrompt(void){
    sendEchoString(SHELL_PROMPT);
}

static char getLastChar(void){

    return shell_context.rx_buffer[shell_context.rx_size-1];
}

static bool isRxBufferFull(void){

    return (shell_context.rx_size >= SHELL_RX_BUFFER_SIZE_BYTES);
}

static void resetRxBuffer(void){

    memset(shell_context.rx_buffer, 0, sizeof(shell_context.rx_buffer));
    shell_context.rx_size = 0;
}

static const SHELL_Cmd_t * findCommand(const char *name){
    SHELL_FOR_EACH_COMMAND(command){
        if(strcmp(command->cmd_name, name) == 0){
            return command;
        }
    }

    return NULL;
}

static void processRxInput(void){

    if(getLastChar() != '\n' && isRxBufferFull()){
        return;
    }

    char *argv[SHELL_MAX_ARGS] = {0};
    uint8_t argc = 0;

    char *next_arg = NULL;
    for(uint16_t i=0; i < shell_context.rx_size && argc < SHELL_MAX_ARGS; ++i){
        char *const c = &shell_context.rx_buffer[i];
        if(*c == ' ' || *c == '\n' || i == shell_context.rx_size - 1){
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
            //Do notthing...
        }
    }

    if(shell_context.rx_size == SHELL_RX_BUFFER_SIZE){
        sendEcho('\n');
    }

    if(argc >= 1){
        
        const SHELL_Cmd_t *command = findCommand(argv[0]);
        if(!command){
            sendEchoString("Unknown command: ");
            sendEchoString(argv[0]);
            sendEcho('\n');
            sendEchoString("Type 'help' to list all commands\n");
        }
        else{
            command->cmd_func(argc, argv);
        }
    }

    resetRxBuffer();
    sendPrompt();
}

/******************************************************************************
*   Public Functions Definitions
*******************************************************************************/
SHELL_Ret_t SHELL_Init(PrintCharCallback_t print_c){

    if(print_c == NULL){
        return SHELL_STATUS_ERROR;
    }

    resetRxBuffer();
    sendEchoString("\n" SHELL_PROMPT);

    return SHELL_STATUS_OK;
}

void SHELL_RecvChar(char c){

    if(c == '\r' || isRxBufferFull() || !isShellInitiatized()){
        return;
    }

    sendEcho(c);

    if(c == '\b'){
        shell_context.rx_buffer[--shell_context.rx_size] = '\0';
        return;
    }

    shell_context.rx_buffer[shell_context.rx_size++] = c;

    processRxInput();
}

SHELL_Ret_t SHELL_PutLine(const char *str){

    if(str == NULL){
        SHELL_STATUS_ERROR;
    }

    sendEchoString(str);
    sendEcho('\n');

    return SHELL_STATUS_OK;
}

SHELL_Ret_t SHELL_HelpHandler(uint32_t argc, char *argv[]){

    SHELL_FOR_EACH_COMMAND(command){
        sendEchoString(command->cmd_name);
        sendEchoString(": ");
        sendEchoString(command->help_comment);
        sendEcho('\n');
    }

    return SHELL_STATUS_OK;
}


/******************************************************************************
*   Interrupts
*******************************************************************************/


