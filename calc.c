// Thomas Fernandes Araujo 251ADB193
// Repository: https://github.com/TfMarra5/calc-lab-home
// Compile with: gcc -O2 -Wall -Wextra -std=c17 -o calc calc.c -lm
// Resources used: course slides by Professor Evalds Masalskis, W3Schools C Tutorial, previous C labs, OpenAI ChatGPT/Codex (guided explanations).
#include <stdio.h>
#include <string.h>

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

    if (fclose(input) != 0) {
        perror(input_file);
        return 1;
    }

    return 0;
}