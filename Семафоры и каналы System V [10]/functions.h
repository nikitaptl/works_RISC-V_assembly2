#ifndef IHW2_FUNCTIONS_H
#define IHW2_FUNCTIONS_H
#include <stdio.h>
#include <stdlib.h>

#define RED_TEXT "\033[1;31m"
#define BLUE_TEXT "\033[1;34m"
#define CYAN_TEXT "\033[0;36m"
#define RESET_TEXT "\033[0m"

#define error_message(message) printf(RED_TEXT "[ERROR]" RESET_TEXT " %s\n", message)
#define system_message(message) printf(CYAN_TEXT "[System]" RESET_TEXT " %s\n", message)
#define programmer_message(message, id) printf(BLUE_TEXT "[Programmer %d]" RESET_TEXT " %s\n", programmer_id, message)

#endif //IHW2_FUNCTIONS_H
