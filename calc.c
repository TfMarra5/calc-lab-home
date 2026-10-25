// Thomas Fernandes Araujo 251ADB193
// Repository: https://github.com/TfMarra5/calc-lab-home
// Compile with: gcc -O2 -Wall -Wextra -std=c17 -o calc calc.c -lm
// Resources used: course slides by Professor Evalds Masalskis, W3Schools C Tutorial, previous C labs, OpenAI ChatGPT/Codex (guided explanations).
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <stdlib.h>

// Argument handling based on lecture 5, slide 27.
// AI assistance: OpenAI ChatGPT/Codex, prompts "next" and follow-up
// code review messages. Help with variable scope, braces and -o handling.
// Conversation history: to be supplied before submission.

// Argv - Argument Vector. An array of strings that contain each argument.
int main(int argc, char *argv[]) {
    const char *output_dir = NULL;
    const char *input_file = NULL;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-o") == 0) {
            if (i + 1 < argc) {
                output_dir = argv[i + 1];
                i++;
            } else {
                fprintf(stderr, "Error: No directory provided after -o!\n");
                return 1;
            }
        }
        else if (argv[i][0] == '-') {
            fprintf(stderr, "Error: Unknown option: %s\n", argv[i]);
                return 1;
        } else if (input_file == NULL) {
            input_file = argv[i];
        } else {
        fprintf(stderr, "Error: Too many input files.\n");
        return 1;
}
    }
        if (input_file == NULL) {
        fprintf(stderr, "Error: No input file provided.\n");
        return 1;
    }

        const char *input_name = strrchr(input_file, '/');

    if (input_name != NULL) {
        input_name++;
    } else {
        input_name = input_file;
    }

    printf("Input filename: %s\n", input_name);

        size_t base_length = strlen(input_name);

    if (base_length >= 4 &&
        strcmp(input_name + base_length - 4, ".txt") == 0) {
        base_length -= 4;
    }

    printf("Input base: ");
    fwrite(input_name, 1, base_length, stdout);
    printf("\n");

        const char suffix[] = "_thomas_fernandes_araujo_251adb193.txt";
    char *output_name = malloc(base_length + sizeof(suffix));

    if (output_name == NULL) {
        fprintf(stderr, "Error: Memory allocation failed.\n");
        return 1;
    }

    memcpy(output_name, input_name, base_length);
    memcpy(output_name + base_length, suffix, sizeof(suffix));

    printf("Output filename: %s\n", output_name);
    free(output_name);
    
    // AI assistance: OpenAI ChatGPT/Codex, prompt "proximo".
    // Help with mkdir, EEXIST and checking that an existing path is a directory.
    if (output_dir != NULL) {
        if (mkdir(output_dir, 0755) != 0) {
            if (errno != EEXIST) { // will inform the error reason
                perror(output_dir);
                return 1;
            }

            struct stat info;
            if (stat(output_dir, &info) != 0) {
                perror(output_dir);
                return 1;
            }

            if (!S_ISDIR(info.st_mode)) {
                fprintf(stderr, "Error: Output path is not a directory.\n");
                return 1;
            }
        }
    }

    printf("Input file: %s\n", input_file);
    if (output_dir != NULL) {
        printf("Output directory: %s\n", output_dir);
    }
    // r opens existent file for reading
        FILE *input = fopen(input_file, "r");

    if (input == NULL) {
        perror(input_file);
        return 1;
    }

    printf("Input file opened successfully.\n");

    char buffer[256];

    while (fgets(buffer, sizeof(buffer), input) != NULL) {
        printf("%s", buffer);
    }

    if (ferror(input)) {
        fprintf(stderr, "Error reading input file.\n");
        fclose(input);
        return 1;
    }

    if (fclose(input) != 0) {
        perror(input_file);
        return 1;
    }

    return 0;
}