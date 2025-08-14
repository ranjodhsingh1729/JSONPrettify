#include "prettify.h"

// Performance optimization: Use larger buffer size for file I/O
#define IO_BUFFER_SIZE (1024 * 1024)  // 1MB buffer

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "[ERROR] Missing arguments. Usage: %s <input file> <output file>\n", argv[0]);
        return -1;
    }

    FILE *input_fp = fopen(argv[1], "rb");
    if (input_fp == NULL) {
        fprintf(stderr, "[ERROR] Failed to open input file '%s': %s\n", argv[1], strerror(errno));
        return 1;
    }
    
    // Set buffer for better I/O performance
    setvbuf(input_fp, NULL, _IOFBF, IO_BUFFER_SIZE);

    FILE *output_fp = fopen(argv[2], "wb");
    if (output_fp == NULL) {
        fprintf(stderr, "[ERROR] Failed to open output file '%s': %s\n", argv[2], strerror(errno));
        fclose(input_fp);
        return 1;
    }
    
    // Set buffer for better I/O performance
    setvbuf(output_fp, NULL, _IOFBF, IO_BUFFER_SIZE);

    fseek(input_fp, 0, SEEK_END);
    ssize_t filesize = ftell(input_fp);
    fseek(input_fp, 0, SEEK_SET);
    if (filesize == -1) {
        fprintf(stderr, "[ERROR] Failed to determine size of input file '%s': %s\n", argv[1], strerror(errno));
        goto close_fps;
    }

    if (filesize >= MAX_INPUT_SIZE) {
        fprintf(stderr, "[ERROR] Input file '%s' exceeds maximum size limit of %d bytes (actual: %zd bytes)\n", 
                argv[1], MAX_INPUT_SIZE, filesize);
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

    state p;
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
        fprintf(stderr, "[ERROR] JSON prettification failed\n");
        goto close_fps;
    }

    if (p.input[p.read_idx++] == '\0') {
        p.output[p.write_idx++] = '\0';
        nbytes = fwrite(output_buf, strlen(output_buf), (size_t)(1), output_fp);
        if (nbytes == -1) {
            fprintf(stderr, "[ERROR] Failed to write to output file '%s': %s\n", argv[2], strerror(errno));
            goto close_fps;
        }
        // fprintf(stdout, "[SUCCESS] JSON prettified successfully: '%s' -> '%s'\n", argv[1], argv[2]);
    } else {
        fprintf(stderr, "[ERROR] JSON parsing incomplete - reached unexpected end of input\n");
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
