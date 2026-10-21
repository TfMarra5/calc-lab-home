// Thomas Fernandes Araujo 251ADB193
// Repository: https://github.com/TfMarra5/calc-lab-home
// Compile with: gcc -O2 -Wall -Wextra -std=c17 -o calc calc.c -lm
// Resources used: course slides by Professor Evalds Masalskis, previous C labs, OpenAI ChatGPT/Codex (guided explanations).
#include <stdio.h>

int main(int argc, char *argv[]) {
    printf("Number of arguments: %d\n", argc);
    for (int i = 0; i < argc; i++) {
        printf("argv[%d] = %s\n", i, argv[i]);
    }
    return 0;
}