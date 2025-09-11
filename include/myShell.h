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
SHELL_Ret_t SHELL_Init(SHELL_PrintCallback_t print_callback);

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
SHELL_Ret_t SHELL_RecvChar(char c);

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
int32_t SHELL_HelpHandler(int32_t argc, char *argv[]);

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
void SHELL_PutLine(const char *str);


#endif//_MY_SHELL_H