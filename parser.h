

#ifndef SHELL_PARSER_H
#define SHELL_PARSER_H
#endif

char *delete_comment(char *line);

char *cmsh_read_line( void );

char **cmsh_split_line(const char* line, char* delim);

char** add_new_args_from_file(char* command, char* file);
