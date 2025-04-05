#include "prettify.h"


int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <input file> <output file>\n", argv[0]);
        return -1;
    }

    FILE *input_fp = fopen(argv[1], "rb");
    if (input_fp == NULL) {
        fprintf(stderr, "Can not open input file %s: %s\n", argv[1], strerror(errno));
        goto close_fps;
    }
    FILE *output_fp = fopen(argv[2], "wb");
    if (output_fp == NULL) {
        fprintf(stderr, "Can not open output file %s: %s\n", argv[2], strerror(errno));
        goto close_fps;
    }

    fseek(input_fp, 0, SEEK_END);
    ssize_t filesize = ftell(input_fp);
    fseek(input_fp, 0, SEEK_SET);
    if (filesize == -1) {
        fprintf(stderr, "Can not read filesize %s: %s\n", argv[1], strerror(errno));
        goto close_fps;
    }
    if (filesize >= MAX_INPUT_SIZE) {
        fprintf(stderr, "Input file is too large %s\n", argv[1]);
        goto close_fps;
    }

    char input_buf[MAX_INPUT_SIZE];
    char output_buf[MAX_OUTPUT_SIZE];
    size_t nbytes = fread(input_buf, filesize, (size_t)(1), input_fp);
    if (nbytes == -1) {
        fprintf(stderr, "Unable to read from input file %s: %s\n", argv[1], strerror(errno));
        goto close_fps;
    }
    input_buf[MAX_INPUT_SIZE-1] = '\0';

    parser_state p;
    p.depth = 0;
    p.read_idx = 0;
    p.write_idx = 0;
    p.input = input_buf;
    p.output = output_buf;
    p.max_input_len = MAX_INPUT_SIZE;
    p.max_output_len = MAX_INPUT_SIZE;
    p.max_input_depth = 100;
    
    next_token(&p);
    if (prettify_json(&p)) {
        fprintf(stderr, "prettify_json returned a non-zero value.\n");
        goto close_fps;
    }

    if (p.input[p.read_idx++] == '\0') {
        p.output[p.write_idx++] = '\0';
        nbytes = fwrite(output_buf, strlen(output_buf), (size_t)(1), output_fp);
        if (nbytes == -1) {
            fprintf(stderr, "Unable to write to output file %s: %s\n", argv[2], strerror(errno));
            goto close_fps;
        }
        fprintf(stdout, "Done\n");
    } else {
        fprintf(stderr, "prettify_json didn't reach end of string.\n");
        goto close_fps;
    }

    fclose(input_fp);
    fclose(output_fp);
    return 0;

close_fps:
    fclose(input_fp);
    fclose(output_fp);
    return 1;
}