/*
Fun Fact: There are 95 returns in this code. I know this and can live with it.
*/

#include "prettify.h"


// utility functions
int next_token(parser_state *p) {
    if (p->read_idx >= p->max_input_len) {
        fprintf(stderr, "[ERROR] Buffer overflow: Input exceeds maximum length at position %d\n", p->read_idx);
        return -1;
    }

    p->current_token = p->input[p->read_idx++];
    return 0;
}

int match(char x, parser_state *p) {
    if (x != p->current_token) {
        fprintf(stderr, "[ERROR] Syntax error at position %d: Expected '%c', found '%c'\n", 
                p->read_idx, x, p->current_token);
        return -1;
    }
    
    next_token(p);
    return 0;
}

int save_token(char x, parser_state *p) {
    if (p->write_idx >= p->max_output_len) {
        fprintf(stderr, "[ERROR] Buffer overflow: Output exceeds maximum length at position %d\n", p->write_idx);
        return -1;
    }

    if (x != '\0') {
        p->output[p->write_idx++] = x;
    } else {
        p->output[p->write_idx++] = p->current_token;
    }
    return 0;
}

int skip_spaces(parser_state *p) {
    while (isspace(p->current_token))
        if (next_token(p))
            return -1;

    return 0;
}

int save_newline(parser_state *p) {
    if (save_token('\n', p))
        return -1;
    for (int i = 0; i < p->depth; i++)
        if (save_token('\t', p))
            return -1;

    return 0;
}

int save_curr_and_next_token(parser_state *p) {
    if (save_token(0, p))
        return -1;
    if (next_token(p))
        return -1;

    return 0;
}


// primitive types
int value_null(parser_state *p) {
    char *val = "null";
    for (int i = 0; val[i]; i++) {
        if (save_token(0, p))
            return -1;
        if (match(val[i], p))
            return -1;
    }
    return 0;
}

int value_boolean(parser_state *p) {
    char *val = p->current_token == 't' ? "true" : "false";
    for (int i = 0; val[i]; i++) {
        if (save_token(0, p))
            return -1;
        if (match(val[i], p))
            return -1;
    }
    return 0;
}

int value_number(parser_state *p) {
    if (p->current_token == '-') {
        if (save_curr_and_next_token(p))
            return -1;
    }

    if (!isdigit(p->current_token)) {
        fprintf(stderr, "[ERROR] Invalid number format at position %d: Expected digit, found '%c'\n", 
                p->read_idx, p->current_token);
        return -1;
    }

    if (p->current_token != '0') {
        while (isdigit(p->current_token)) {
            if (save_curr_and_next_token(p))
                return -1;
        }
    } else {
        if (save_curr_and_next_token(p))
            return -1;
    }

    if (p->current_token == '.') {
        if (save_curr_and_next_token(p))
            return -1;
        if (!isdigit(p->current_token)) {
            fprintf(stderr, "[ERROR] Invalid number format at position %d: Expected digit, found '%c'\n", 
                    p->read_idx, p->current_token);
            return -1;
        }
        while (isdigit(p->current_token)) {
            if (save_curr_and_next_token(p))
                return -1;
        }
    }

    if (tolower(p->current_token) == 'e') {
        if (save_curr_and_next_token(p))
            return -1;
        if (p->current_token == '-' || p->current_token == '+') {
            if (save_curr_and_next_token(p))
                return -1;
        }
        if (!isdigit(p->current_token)) {
            fprintf(stderr, "[ERROR] Invalid number format at position %d: Expected digit, found '%c'\n", 
                    p->read_idx, p->current_token);
            return -1;
        }
        while (isdigit(p->current_token)) {
            if (save_curr_and_next_token(p))
                return -1;
        }
    }

    if (isdigit(p->current_token)) {
        fprintf(stderr, "JSON Token Mismatch in %s at %d:", __FILE__, __LINE__);
        fprintf(stderr, "Expecting a non digit, Found %c\n", p->current_token);
        return -1;
    }
    return 0;
}

int value_string(parser_state *p) {
    if (save_token(0, p))
        return -1;
    if (match('"', p))
        return -1;

    while (p->current_token != '"') {
        if (p->current_token == '\\') {
            if (save_curr_and_next_token(p))
                return -1;
        }
        if (save_curr_and_next_token(p))
            return -1;
    }

    if (save_token(0, p))
        return -1;
    if (match('"', p))
        return -1;

    return 0;
}

// structured types
int value_array(parser_state *p) {
    if (save_token(0, p))
        return -1;
    if (match('[', p))
        return -1;

    p->depth++;
    if (p->depth > p->max_input_depth) {
        fprintf(stderr, "[ERROR] Maximum nesting depth exceeded (%d) at position %d\n", 
                p->max_input_depth, p->read_idx);
        return -1;
    }
    if (save_newline(p))
        return -1;

    if (skip_spaces(p))
        return -1;
    if (p->current_token != ']') {
        if (prettify_json(p))
            return -1;
        if (skip_spaces(p))
            return -1;
    }

    while (p->current_token != ']') {
        if (save_token(0, p))
            return -1;
        if (match(',', p))
            return -1;
        if (save_newline(p))
            return -1;
        if (skip_spaces(p))
            return -1;
        if (prettify_json(p))
            return -1;
        if (skip_spaces(p))
            return -1;
    }

    p->depth--;
    if (save_newline(p))
        return -1;

    if (save_token(0, p))
        return -1;
    if (match(']', p))
        return -1;

    return 0;
}

int value_object(parser_state *p) {
    if (save_token(0, p))
        return -1;
    if (match('{', p))
        return -1;

    p->depth++;
    if (p->depth > p->max_input_depth) {
        fprintf(stderr, "JSON Max Depth Reached in %s at %d\n", __FILE__, __LINE__);
        return -1;
    }
    if (save_newline(p))
        return -1;
    
    if (skip_spaces(p))
        return -1;
    if (p->current_token != '}') {
        if (value_string(p))
            return -1;
        if (skip_spaces(p))
            return -1;
        if (save_token(0, p))
            return -1;
        if (match(':', p))
            return -1;
        if (save_token(' ', p))
            return -1;
        if (skip_spaces(p))
            return -1;
        if (prettify_json(p))
            return -1;
        if (skip_spaces(p)) 
            return -1;
    }

    while (p->current_token != '}') {
        if (save_token(0, p))
            return -1;
        if (match(',', p))
            return -1;
        if (save_newline(p))
            return -1;
        if (skip_spaces(p))
            return -1;
        if (value_string(p))
            return -1;
        if (skip_spaces(p))
            return -1;
        if (save_token(0, p))
            return -1;
        if (match(':', p))
            return -1;
        if (save_token(' ', p))
            return -1;
        if (skip_spaces(p))
            return -1;
        if (prettify_json(p))
            return -1;
        if (skip_spaces(p))
            return -1;
    }

    p->depth--;
    if (save_newline(p))
        return -1;

    if (save_token(0, p))
        return -1;
    if (match('}', p))
        return -1;

    return 0;
}

int prettify_json(parser_state *p) {
    if (skip_spaces(p))
        return -1;

    switch (p->current_token)
    {
    case '{':
        // object
        if (value_object(p))
            return -1;
        break;
    case '[':
        // array
        if (value_array(p))
            return -1;
        break;
    case '"':
        // string
        if (value_string(p))
            return -1;
        break;
    case 'n':
        // null
        if (value_null(p))
            return -1;
        break;
    case 't':
    case 'f':
        // boolean
        if (value_boolean(p))
            return -1;
        break;
    default:
        // number
        if (value_number(p))
            return -1;
        break;
    }

    if (skip_spaces(p))
        return -1;

    return 0;
}