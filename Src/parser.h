#ifndef PARSER_H
#define PARSER_H

char **split_pipes(char *line, int *count);
char **parse_args(char *cmdline);
void free_commands(char **cmds, int n);

#endif
