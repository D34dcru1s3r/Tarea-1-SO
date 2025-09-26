#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.h"
#include "exec.h"
#include "miprof.h"

int main(void) {
    char input[4096];

    while (1) {
        printf("$ ");
        fflush(stdout);

        if (fgets(input, sizeof(input), stdin) == NULL) {
            printf("\n");
            break; // EOF
        }

        // quitar newline final
        input[strcspn(input, "\n")] = '\0';

        // si está vacío → prompt otra vez
        if (strlen(input) == 0) continue;

        // dividir por pipes
        int num_cmds = 0;
        char **cmds = split_pipes(input, &num_cmds);
        if (num_cmds == 0) {
            free_commands(cmds, num_cmds);
            continue;
        }

        // comando interno: exit
        if (strcmp(cmds[0], "exit") == 0) {
            free_commands(cmds, num_cmds);
            break;
        }

        // comando interno: miprof
        if (strncmp(cmds[0], "miprof", 6) == 0) {
            run_miprof(cmds[0]);
        } else {
            run_pipeline(cmds, num_cmds);
        }

        free_commands(cmds, num_cmds);
    }

    return 0;
}
