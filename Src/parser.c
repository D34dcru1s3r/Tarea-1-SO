#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.h"

// dividir entrada en comandos separados por '|'
char **split_pipes(char *line, int *count) {
    char *token, *saveptr;
    int cap = 8, n = 0;
    char **cmds = malloc(cap * sizeof(char*));

    token = strtok_r(line, "|", &saveptr);
    while (token) {
        while (*token == ' ') token++; // trim inicial
        cmds[n++] = strdup(token);
        if (n >= cap) {
            cap *= 2;
            cmds = realloc(cmds, cap * sizeof(char*));
        }
        token = strtok_r(NULL, "|", &saveptr);
    }
    *count = n;
    return cmds;
}

// dividir comando en args (argv[])
char **parse_args(char *cmdline) {
    char *token, *saveptr;
    int cap = 8, n = 0;
    char **argv = malloc(cap * sizeof(char*));

    token = strtok_r(cmdline, " \t", &saveptr);
    while (token) {
        argv[n++] = strdup(token);
        if (n >= cap) {
            cap *= 2;
            argv = realloc(argv, cap * sizeof(char*));
        }
        token = strtok_r(NULL, " \t", &saveptr);
    }
    argv[n] = NULL;
    return argv;
}

void free_commands(char **cmds, int n) {
    for (int i = 0; i < n; i++) free(cmds[i]);
    free(cmds);
}
