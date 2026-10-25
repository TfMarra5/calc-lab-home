// Thomas Fernandes Araujo 251ADB193
// Repository: https://github.com/TfMarra5/calc-lab-home
// Compile with: gcc -O2 -Wall -Wextra -std=c17 -o calc calc.c -lm
// Resources used: course slides by Professor Evalds Masalskis, W3Schools C Tutorial, previous C labs, OpenAI ChatGPT/Codex (guided explanations).
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <stdlib.h>

// Help with token types and storing token positions.

typedef enum {
    TOKEN_END,
    TOKEN_NEWLINE,
    TOKEN_NUMBER,
    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_MULTIPLY,
    TOKEN_DIVIDE,
    TOKEN_POWER,
    TOKEN_LEFT_PAREN,
    TOKEN_RIGHT_PAREN,
    TOKEN_INVALID
} TokenType;

typedef struct {
    TokenType type;
    long long integer_value;
    size_t position;
} Token;

typedef struct {
    const char *text;
    size_t index;
    Token current;
    size_t error_position;
} Parser;

Token next_token(const char *text, size_t *index) {
    // Skip spaces without losing their contribution to file positions.
    while (text[*index] == ' ' || text[*index] == '\t') {
        (*index)++;
    }

    Token token = {TOKEN_INVALID, 0, *index + 1};
    char ch = text[*index];

    if (ch == '\0') {
        token.type = TOKEN_END;
        return token;
    }

    if (ch >= '0' && ch <= '9') {
        token.type = TOKEN_NUMBER;

        while (text[*index] >= '0' && text[*index] <= '9') {
            token.integer_value =
                token.integer_value * 10 + (text[*index] - '0');
            (*index)++;
        }

        return token;
    }

    switch (ch) {
        case '+':
            token.type = TOKEN_PLUS;
            break;
        case '-':
            token.type = TOKEN_MINUS;
            break;
                    case '*':
            if (text[*index + 1] == '*') {
                token.type = TOKEN_POWER;
                (*index)++;
            } else {
                token.type = TOKEN_MULTIPLY;
            }
            break;
        case '/':
            token.type = TOKEN_DIVIDE;
            break;
        case '(':
            token.type = TOKEN_LEFT_PAREN;
            break;
        case ')':
            token.type = TOKEN_RIGHT_PAREN;
            break;
        case '\n':
            token.type = TOKEN_NEWLINE;
            break;
        default:
            token.type = TOKEN_INVALID;
            break;
    }

    (*index)++;
    return token;
}

// Help with parser state and an initial integer addition/subtraction parser.
// The operator loop evaluates addition and subtraction left to right.

void advance(Parser *parser) {
    parser->current = next_token(parser->text, &parser->index);
}

long long parse_integer(Parser *parser) {
    if (parser->current.type != TOKEN_NUMBER) {
        parser->error_position = parser->current.position;
        return 0;
    }

    long long value = parser->current.integer_value;
    advance(parser);
    return value;
}
// Help with multiplication/division precedence and error positions.
// A term completes multiplication and division before addition/subtraction.
// AI assistance: OpenAI ChatGPT/Codex, prompt "façamos isso".
// Help with multiplication/division precedence and error positions.
long long parse_term(Parser *parser) {
    long long result = parse_integer(parser);

    while (parser->error_position == 0 &&
           (parser->current.type == TOKEN_MULTIPLY ||
            parser->current.type == TOKEN_DIVIDE)) {
        TokenType operation = parser->current.type;
        advance(parser);

        size_t right_position = parser->current.position;
        long long right = parse_integer(parser);

        if (parser->error_position != 0) {
            return 0;
        }

        if (operation == TOKEN_MULTIPLY) {
            result *= right;
        } else {
            // Division-by-zero errors point to the right operand.
            if (right == 0) {
                parser->error_position = right_position;
                return 1;
            }

            result /= right;
        }
    }

    return result;
}
long long parse_expression(Parser *parser) {
    
    long long result = parse_term(parser);

    while (parser->error_position == 0 &&
           (parser->current.type == TOKEN_PLUS ||
            parser->current.type == TOKEN_MINUS)) {
        TokenType operation = parser->current.type;
        advance(parser);

        long long right = parse_term(parser);

        if (parser->error_position != 0) {
            return 0;
        }

        if (operation == TOKEN_PLUS) {
            result += right;
        } else {
            result -= right;
        }
    }

    return result;
}
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
        if (output_dir != NULL) {
        size_t path_size = strlen(output_dir) + strlen(output_name) + 2;
        char *output_path = malloc(path_size);

        if (output_path == NULL) {
            fprintf(stderr, "Error: Memory allocation failed.\n");
            free(output_name);
            return 1;
        }

        snprintf(output_path, path_size, "%s/%s",
                 output_dir, output_name);

        printf("Output path: %s\n", output_path);
        free(output_path);
    }
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

    // Help with a growing input buffer, realloc failure handling,
    // and preserving characters for error positions.

        size_t capacity = 256;
    size_t length = 0;
    char *text = malloc(capacity);

    if (text == NULL) {
        fprintf(stderr, "Error: Memory allocation failed.\n");
        fclose(input);
        return 1;
    }

    int ch;
    while ((ch = fgetc(input)) != EOF) {
        // Keep one extra byte available for the string terminator.
        if (length == capacity - 1) {
            if (capacity > (size_t)-1 / 2) {
                fprintf(stderr, "Error: Input is too large.\n");
                free(text);
                fclose(input);
                return 1;
            }

            size_t new_capacity = capacity * 2;
            char *temporary = realloc(text, new_capacity);

            if (temporary == NULL) {
                fprintf(stderr, "Error: Memory allocation failed.\n");
                free(text);
                fclose(input);
                return 1;
            }

            text = temporary;
            capacity = new_capacity;
        }

        text[length++] = (char)ch;
    }

    if (ferror(input)) {
        fprintf(stderr, "Error: Failed to read input file.\n");
        free(text);
        fclose(input);
        return 1;
    }

    if (fclose(input) != 0) {
        perror(input_file);
        free(text);
        return 1;
    }

    text[length] = '\0';

        text[length] = '\0';

    printf("Characters read: %zu\n", length);

    Parser parser = {0};
    parser.text = text;
    advance(&parser);

    while (parser.current.type != TOKEN_END) {
        if (parser.current.type == TOKEN_NEWLINE) {
            advance(&parser);
            continue;
        }

        long long result = parse_expression(&parser);

        if (parser.error_position == 0 &&
            parser.current.type != TOKEN_NEWLINE &&
            parser.current.type != TOKEN_END) {
            parser.error_position = parser.current.position;
        }

        if (parser.error_position != 0) {
            printf("ERROR:%zu\n", parser.error_position);
            break;
        }

        printf("Result: %lld\n", result);
    }

    free(text);
    return 0;
}