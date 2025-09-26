#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include "parser.h"

static volatile pid_t child_pid = 0;

void alarm_handler(int sig) {
    if (child_pid > 0) kill(child_pid, SIGKILL);
}

void run_miprof(char *line) {
    // quitar "miprof " y parsear
    char *cmdline = line + 6;
    while (*cmdline == ' ') cmdline++;

    char *token = strtok(cmdline, " ");
    if (!token) return;

    if (strcmp(token, "ejec") == 0 || strcmp(token, "ejecsave") == 0) {
        int save = strcmp(token, "ejecsave") == 0;
        char *file = NULL;
        if (save) {
            file = strtok(NULL, " ");
            if (!file) { printf("Debe indicar archivo\n"); return; }
        }
        char *rest = strtok(NULL, "");
        if (!rest) { printf("Debe indicar comando\n"); return; }
        char **argv = parse_args(rest);

        struct timespec t0, t1;
        struct rusage usage;
        clock_gettime(CLOCK_MONOTONIC, &t0);
        pid_t pid = fork();
        if (pid == 0) {
            execvp(argv[0], argv);
            perror("execvp");
            exit(1);
        } else {
            child_pid = pid;
            int status;
            wait4(pid, &status, 0, &usage);
            clock_gettime(CLOCK_MONOTONIC, &t1);
            child_pid = 0;

            double real = (t1.tv_sec - t0.tv_sec) + (t1.tv_nsec - t0.tv_nsec)/1e9;
            double user = usage.ru_utime.tv_sec + usage.ru_utime.tv_usec/1e6;
            double sys  = usage.ru_stime.tv_sec + usage.ru_stime.tv_usec/1e6;

            FILE *out = stdout;
            if (save) out = fopen(file, "a");
            fprintf(out, "[miprof] comando: %s\n", rest);
            fprintf(out, "real: %.3fs\nuser: %.3fs\nsys: %.3fs\nmaxrss: %ld KB\n\n",
                    real, user, sys, usage.ru_maxrss);
            if (save) fclose(out);
        }
    }

    else if (strcmp(token, "maxtiempo") == 0) {
        char *time_str = strtok(NULL, " ");
        char *rest = strtok(NULL, "");
        if (!time_str || !rest) { printf("Uso: miprof maxtiempo <segundos> comando\n"); return; }
        int timeout = atoi(time_str);
        char **argv = parse_args(rest);

        struct sigaction sa = {0}, old;
        sa.sa_handler = alarm_handler;
        sigaction(SIGALRM, &sa, &old);

        struct timespec t0, t1;
        struct rusage usage;
        clock_gettime(CLOCK_MONOTONIC, &t0);
        pid_t pid = fork();
        if (pid == 0) {
            execvp(argv[0], argv);
            perror("execvp");
            exit(1);
        } else {
            child_pid = pid;
            alarm(timeout);
            int status;
            wait4(pid, &status, 0, &usage);
            alarm(0);
            clock_gettime(CLOCK_MONOTONIC, &t1);
            child_pid = 0;
            sigaction(SIGALRM, &old, NULL);

            double real = (t1.tv_sec - t0.tv_sec) + (t1.tv_nsec - t0.tv_nsec)/1e9;
            double user = usage.ru_utime.tv_sec + usage.ru_utime.tv_usec/1e6;
            double sys  = usage.ru_stime.tv_sec + usage.ru_stime.tv_usec/1e6;

            printf("[miprof maxtiempo] comando: %s\n", rest);
            printf("real: %.3fs\nuser: %.3fs\nsys: %.3fs\nmaxrss: %ld KB\n\n",
                   real, user, sys, usage.ru_maxrss);
        }
    }

    else {
        printf("Uso: miprof [ejec|ejecsave archivo|maxtiempo segundos] comando args...\n");
    }
}
