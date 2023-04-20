

#ifndef SHELL_PARSER_H
#define SHELL_PARSER_H
#endif

char *delete_comment(char *line);

char *cmsh_read_line( void );

char **cmsh_split_line(const char* line, char* delim);

char* remplace_command_again(char* command);
