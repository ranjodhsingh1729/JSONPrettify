#ifndef PRETTIFY_H
#define PRETTIFY_H

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


typedef struct state {
    int depth;
    char *input;
    char *output;
    int read_idx;
    int write_idx;
    char current_token;    
    int max_input_len;
    int max_output_len;
    int max_input_depth;
} state;


// prototypes
int next_token(state *p);
int match(char x, state *p);
int save_token(char x, state *p);
int save_newline(state *p);
int skip_spaces(state *p);
int save_curr_and_next_token(state *p);

int value_null(state *p);
int value_boolean(state *p);
int value_number(state *p);
int value_string(state *p);
int value_array(state *p);
int value_object(state *p);
int prettify_json(state *p);


#endif // PRETTIFY_H