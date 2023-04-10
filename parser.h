

#ifndef SHELL_PARSER_H
#define SHELL_PARSER_H
#endif



#define CMSH_TOK_BUFF_SIZE 1024
#define CMSH_TOK_DELIM " \t\r\n\a"

char *delete_comment(char *line);

char *cmsh_read_line( void );

char **cmsh_split_line(char * line);

char* cmsh_read_file( char* file );

//char** cmsh_split_lines(char** lines);

char** add_new_args_from_file(char* command, char* file);
