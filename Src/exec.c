#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include "parser.h"
#include "exec.h"

void run_pipeline(char **cmds, int n) {
    int prev_fd[2] = {-1, -1};
    pid_t pids[n];

    for (int i = 0; i < n; i++) {
        int fd[2];
        if (i < n - 1 && pipe(fd) == -1) {
            perror("pipe");
            exit(1);
        }

        pid_t pid = fork();
        if (pid == 0) {
            // hijo
            if (i > 0) {
                dup2(prev_fd[0], STDIN_FILENO);
                close(prev_fd[0]);
                close(prev_fd[1]);
            }
            if (i < n - 1) {
                close(fd[0]);
                dup2(fd[1], STDOUT_FILENO);
                close(fd[1]);
            }

            // parsear args
            char **argv = parse_args(cmds[i]);
            execvp(argv[0], argv);
            perror("execvp");
            exit(1);
        } else if (pid > 0) {
            pids[i] = pid;
            if (i > 0) {
                close(prev_fd[0]);
                close(prev_fd[1]);
            }
            if (i < n - 1) {
                prev_fd[0] = fd[0];
                prev_fd[1] = fd[1];
            }
        } else {
            perror("fork");
            exit(1);
        }
    }

    for (int i = 0; i < n; i++) {
        waitpid(pids[i], NULL, 0);
    }
}
