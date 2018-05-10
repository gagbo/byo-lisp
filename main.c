#include <stdlib.h>
#include <stdio.h>

#include <editline/readline.h>
#include <histedit.h>

int main(int argc, char** argv) {
    puts("Lispy Version 0.0.0.0.1");
    puts("Press Ctrl+C to exit\n");

    while(1) {
        char* input = readline("lispy> ");

        add_history(input);

        printf("You just typed %s\n", input);

        free(input);
    }

    return EXIT_SUCCESS;
}
