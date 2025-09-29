// /**
//  * MyShell - Custom Remote Shell Implementation (Phase 1)
//  * 
//  * This program implements a basic shell that can:
//  * - Execute simple commands with and without arguments
//  * - Handle input, output, and error redirection
//  * - Provide basic error handling
//  * 
//  * Author: Mahmoud Kassem (Person A)
//  * Course: Operating Systems - Fall 2025
//  * Due: October 2nd, 2025
//  */

// #include "shell_utils.h"

// /* Global constants */
// const char* SHELL_PROMPT = "$ ";
// const char* EXIT_COMMAND = "exit";

// /**
//  * Main function - implements the shell loop
//  * 
//  * The shell continuously:
//  * 1. Displays a prompt ($)
//  * 2. Reads user input
//  * 3. Parses and executes the command
//  * 4. Waits for command completion
//  * 5. Repeats until user enters 'exit'
//  */
// int main(void) {
//     char input[MAX_INPUT_SIZE];
//     command_t* cmd;
//     int status;
    
//     /* Print welcome message */
//     printf("MyShell v1.0 - Custom Remote Shell\n");
//     printf("Type 'exit' to quit.\n\n");
    
//     /* Main shell loop */
//     while (1) {
//         /* Display prompt and flush output buffer */
//         printf("%s", SHELL_PROMPT);
//         fflush(stdout);
        
//         /* Read input from user */
//         if (fgets(input, sizeof(input), stdin) == NULL) {
//             /* Handle EOF (Ctrl+D) */
//             printf("\n");
//             break;
//         }
        
//         /* Remove trailing newline character */
//         input[strcspn(input, "\n")] = '\0';
        
//         /* Skip empty input or whitespace-only input */
//         if (is_empty_string(input)) {
//             continue;
//         }
        
//         /* Handle exit command */
//         if (strcmp(trim_whitespace(input), EXIT_COMMAND) == 0) {
//             printf("Goodbye!\n");
//             break;
//         }
        
//         /* Parse the command line */
//         cmd = parse_command_line(input);
//         if (cmd == NULL) {
//             handle_error(ERROR_INVALID_COMMAND, input);
//             continue;
//         }
        
//         /* Execute the command */
//         status = execute_simple_command(cmd);
//         if (status != 0) {
//             /* Error already handled in execute_simple_command */
//         }
        
//         /* Clean up memory */
//         free_command(cmd);
//     }
    
//     return 0;
// }




/**
 * MyShell - Custom Remote Shell Implementation (Phase 1)
 *
 * This program implements a basic shell that can:
 * - Execute simple commands with and without arguments
 * - Handle input, output, and error redirection
 * - Provide basic error handling
 */
#include "shell_utils.h"

// these values will be global constants
const char* SHELL_PROMPT = "$ ";
const char* EXIT_COMMAND = "exit";

/**
 * Main function - implements the shell loop
 *
 * The shell continuously:
 * 1. Displays a prompt ($)
 * 2. Reads user input
 * 3. Parses and executes the command
 * 4. Waits for command completion
 * 5. Repeats until user enters 'exit'
 */
// main function for implementing the shell loop
// shell -- displays prompt, reads input, parses and executes commands
// continues until user types 'exit'
int main(void) {

    char input[MAX_INPUT_SIZE];
    //for pipes
    pipeline_t* pipeline;
    int status;

    // welcome message and exit instructions -- commenting it out since not required -- go directly to prompt
    // printf("Custom Remote Shell\n");
    // printf("Type 'exit' to quit.\n\n");

    // main shell loop
    while (1) {

        // display prompt and flush output buffer
        printf("%s", SHELL_PROMPT);
        fflush(stdout);

        // need to read input from user
        if (fgets(input, sizeof(input), stdin) == NULL) {
            // handle EOF (Ctrl+D)
            printf("\n");
            break;
        }

        // remove trailing newline character
        input[strcspn(input, "\n")] = '\0';

        // skip empty input or whitespace only input
        if (is_empty_string(input)) {
            continue;
        }

        // Handle exit command
        if (strcmp(trim_whitespace(input), EXIT_COMMAND) == 0) {
            // commenting out exit message since not required -- go directly to exit
            // printf("Goodbye!\n");
            break;
        }

        // changed to use parse_pipeline instead of parse_command_line -- handle both simple commands and pipes
        
        // parse the command line (handles both simple commands and pipes)
        pipeline = parse_pipeline(input);
        if (pipeline == NULL) {
            handle_error(ERROR_INVALID_COMMAND, input);
            continue;
        }

        // execute the pipeline (works for single commands too)
        status = execute_pipeline(pipeline);
        if (status != 0) {
            // error already handled in execute_pipeline
        }

        // clean up memory
        free_pipeline(pipeline);
    }

    return 0;
}