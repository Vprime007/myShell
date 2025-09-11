#ifndef _MY_SHELL_H
#define _MY_SHELL_H

#include <stdlib.h>
#include <stdint.h>

/******************************************************************************
*   Public Definitions
*******************************************************************************/


/******************************************************************************
*   Public Macros
*******************************************************************************/
typedef int32_t(*SHELL_PrintCallback_t)(char c);

/******************************************************************************
*   Public Data Types
*******************************************************************************/
typedef struct SHELL_Command_s{
    const char * name;
    int32_t (*handler)(int32_t argc, char *argv[]);    
    const char *help;
}SHELL_Command_t;

typedef enum SHELL_Ret_e{
    SHELL_STATUS_ERROR,
    SHELL_STATUS_OK,
}SHELL_Ret_t;

/******************************************************************************
*   Public Variables
*******************************************************************************/


/******************************************************************************
*   Error Check
*******************************************************************************/


/******************************************************************************
*   Public Functions
*******************************************************************************/
SHELL_Ret_t SHELL_Init(SHELL_PrintCallback_t print_callback);

SHELL_Ret_t SHELL_RecvChar(char c);

int32_t SHELL_HelpHandler(int32_t argc, char *argv[]);

void SHELL_PutLine(const char *str);


#endif//_MY_SHELL_H