#ifndef PARSER_H
#define PARSER_H

#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>
#include <sys/types.h>


#define MAX_INPUT_SIZE 500000
#define MAX_OUTPUT_SIZE 1000000


typedef struct parser_state {
    int depth;
    char *input;
    char *output;
    int read_idx;
    int write_idx;
    char current_token;    
    int max_input_len;
    int max_output_len;
    int max_input_depth;
} parser_state;


// prototypes
int next_token(parser_state *p);
int match(char x, parser_state *p);
int save_token(char x, parser_state *p);
int save_newline(parser_state *p);
int skip_spaces(parser_state *p);
int save_curr_and_next_token(parser_state *p);

int value_null(parser_state *p);
int value_boolean(parser_state *p);
int value_number(parser_state *p);
int value_string(parser_state *p);
int value_array(parser_state *p);
int value_object(parser_state *p);
int prettify_json(parser_state *p);


#endif // PARSER_H