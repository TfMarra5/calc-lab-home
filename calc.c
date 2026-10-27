// Thomas Fernandes Araujo 251ADB193
// Repository: https://github.com/TfMarra5/calc-lab-home
// Compile with: gcc -O2 -Wall -Wextra -std=c17 -o calc calc.c -lm
// Resources used: course slides by Professor Evalds Masalskis, W3Schools C Tutorial, previous C labs, OpenAI ChatGPT/Codex (guided explanations).
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <dirent.h>

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

    // A # starts a comment only when preceded by spaces/tabs
    // since the beginning of the line.
    if (text[*index] == '#') {
        size_t start = *index;

        while (start > 0 &&
               (text[start - 1] == ' ' || text[start - 1] == '\t')) {
            start--;
        }

        if (start == 0 || text[start - 1] == '\n') {
            while (text[*index] != '\n' && text[*index] != '\0') {
                (*index)++;
            }
        }
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

long long parse_expression(Parser *parser);

// AI assistance: OpenAI ChatGPT/Codex
// Help with recursive parsing of grouped expressions and error positions.
long long parse_primary(Parser *parser) { // here
    if (parser->current.type == TOKEN_NUMBER) {
        long long value = parser->current.integer_value;
        advance(parser);
        return value;
    }

    if (parser->current.type == TOKEN_LEFT_PAREN) {
        advance(parser);

        long long value = parse_expression(parser);

        if (parser->error_position != 0) {
            return 0;
        }

        // A grouped expression must end with a closing parenthesis.
        if (parser->current.type != TOKEN_RIGHT_PAREN) {
            parser->error_position = parser->current.position;
            return 0;
        }

        advance(parser);
        return value;
    }

    parser->error_position = parser->current.position;
    return 0;
}

// Help with right-associative powers and integer exponentiation.

long long parse_power(Parser *parser) {
    long long base = parse_primary(parser);

    if (parser->error_position != 0) {
        return 0;
    }

    if (parser->current.type == TOKEN_POWER) {
        advance(parser);
        size_t exponent_position = parser->current.position;

        // Recursion groups a ** b ** c as a ** (b ** c).
        long long exponent = parse_power(parser);

        if (parser->error_position != 0) {
            return 0;
        }

        if (exponent < 0) {
            parser->error_position = exponent_position;
            return 0;
        }

        long long result = 1;

        // Repeated squaring avoids one multiplication per exponent unit.
        while (exponent > 0) {
            if (exponent % 2 != 0) {
                result *= base;
            }

            exponent /= 2;

            // Avoid an unnecessary final square that could overflow.
            if (exponent > 0) {
                base *= base;
            }
        }

        return result;
    }

    return base;
}

// Help with multiplication/division precedence and error positions.
// A term completes multiplication and division before addition/subtraction.

long long parse_term(Parser *parser) { // here
    long long result = parse_power(parser);

    while (parser->error_position == 0 &&
           (parser->current.type == TOKEN_MULTIPLY ||
            parser->current.type == TOKEN_DIVIDE)) {
        TokenType operation = parser->current.type;
        advance(parser);

        size_t right_position = parser->current.position;
        long long right = parse_power(parser);

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

// Validate the whole input before writing any results.
// Two passes avoid storing a separate array of results.
int write_results(const char *text, const char *output_path) {
    Parser parser = {0};
    parser.text = text;
    advance(&parser);

    // First pass: find the first error without writing partial results.
    while (parser.current.type != TOKEN_END) {
        if (parser.current.type == TOKEN_NEWLINE) {
            advance(&parser);
            continue;
        }

        parse_expression(&parser);

        if (parser.error_position == 0 &&
            parser.current.type != TOKEN_NEWLINE &&
            parser.current.type != TOKEN_END) {
            parser.error_position = parser.current.position;
        }

        if (parser.error_position != 0) {
            break;
        }
    }

    FILE *output = fopen(output_path, "w");
    if (output == NULL) {
        perror(output_path);
        return 1;
    }

    int write_failed = 0;

    if (parser.error_position != 0) {
        if (fprintf(output, "ERROR:%zu\n",
                    parser.error_position) < 0) {
            write_failed = 1;
        }
    } else {
        // Second pass: the input is valid, so write each result.
        parser = (Parser){0};
        parser.text = text;
        advance(&parser);

        while (parser.current.type != TOKEN_END) {
            if (parser.current.type == TOKEN_NEWLINE) {
                advance(&parser);
                continue;
            }

            long long result = parse_expression(&parser);

            if (fprintf(output, "%lld\n", result) < 0) {
                write_failed = 1;
                break;
            }
        }
    }

    if (fclose(output) != 0) {
        write_failed = 1;
    }

    if (write_failed) {
        fprintf(stderr, "Error: Failed to write output file.\n");
        return 1;
    }

    return 0;
}

// Argument handling based on lecture 5, slide 27.
// AI assistance: OpenAI ChatGPT/Codex
// code review messages. Help with variable scope, braces and -o handling.
// Conversation history: to be supplied before submission.

// Argv - Argument Vector. An array of strings that contain each argument.
int run_calculator(int argc, char *argv[], char **default_directory) {
    const char *output_dir = NULL;
    const char *input_file = NULL;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-o") == 0 ||
        strcmp(argv[i], "--output-dir") == 0) {
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

    // AI assistance: OpenAI ChatGPT/Codex, prompt
    // "implementar a pasta padrão quando -o não for informado".
    // main owns this allocation so every return path releases it.
    if (output_dir == NULL) {
        const char directory_suffix[] = "_thomas_251adb193";

        *default_directory =
            malloc(base_length + sizeof(directory_suffix));

        if (*default_directory == NULL) {
            fprintf(stderr, "Error: Memory allocation failed.\n");
            return 1;
        }

        memcpy(*default_directory, input_name, base_length);
        memcpy(*default_directory + base_length,
               directory_suffix, sizeof(directory_suffix));

        output_dir = *default_directory;
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

    // AI assistance: OpenAI ChatGPT/Codex
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


    // Rebuild the output name: the earlier diagnostic code freed it.
    output_name = malloc(base_length + sizeof(suffix));

    if (output_name == NULL) {
        fprintf(stderr, "Error: Memory allocation failed.\n");
        free(text);
        return 1;
    }

    memcpy(output_name, input_name, base_length);
    memcpy(output_name + base_length, suffix, sizeof(suffix));

    size_t path_size = strlen(output_dir) + strlen(output_name) + 2;
    char *output_path = malloc(path_size);

    if (output_path == NULL) {
        fprintf(stderr, "Error: Memory allocation failed.\n");
        free(output_name);
        free(text);
        return 1;
    }

    snprintf(output_path, path_size, "%s/%s",
             output_dir, output_name);
    free(output_name);

    int status = write_results(text, output_path);

    free(output_path);
    free(text);
    return status;
}

// AI assistance: OpenAI ChatGPT/Codex.
// Prompt: "completar a linha de comando: --output-dir, -d e --dir".
// Directory traversal uses the concepts from lecture 5.
// Only regular files ending in .txt are processed; no recursion.
int process_directory(const char *input_dir, const char *output_dir) {
    DIR *directory = opendir(input_dir);

    if (directory == NULL) {
        perror(input_dir);
        return 1;
    }

    char *owned_output = NULL;

    if (output_dir == NULL) {
        const char suffix[] = "_thomas_251adb193";
        size_t end = strlen(input_dir);

        // Ignore trailing slashes when extracting the folder name.
        while (end > 1 && input_dir[end - 1] == '/') {
            end--;
        }

        size_t start = end;
        while (start > 0 && input_dir[start - 1] != '/') {
            start--;
        }

        size_t name_length = end - start;
        owned_output = malloc(name_length + sizeof(suffix));

        if (owned_output == NULL) {
            fprintf(stderr, "Error: Memory allocation failed.\n");
            closedir(directory);
            return 1;
        }

        memcpy(owned_output, input_dir + start, name_length);
        memcpy(owned_output + name_length, suffix, sizeof(suffix));
        output_dir = owned_output;
    }

    // Create the output folder even when the input folder is empty.
    if (mkdir(output_dir, 0755) != 0 && errno != EEXIST) {
        perror(output_dir);
        free(owned_output);
        closedir(directory);
        return 1;
    }

    struct stat output_info;
    if (stat(output_dir, &output_info) != 0) {
        perror(output_dir);
        free(owned_output);
        closedir(directory);
        return 1;
    }

    if (!S_ISDIR(output_info.st_mode)) {
        fprintf(stderr, "Error: Output path is not a directory.\n");
        free(owned_output);
        closedir(directory);
        return 1;
    }

    int status = 0;

    while (1) {
        // Reset errno so end-of-directory can be distinguished from failure.
        errno = 0;
        struct dirent *entry = readdir(directory);

        if (entry == NULL) {
            if (errno != 0) {
                perror(input_dir);
                status = 1;
            }
            break;
        }

        size_t length = strlen(entry->d_name);

        if (length < 4 ||
            strcmp(entry->d_name + length - 4, ".txt") != 0) {
            continue;
        }

        size_t path_size = strlen(input_dir) + length + 2;
        char *input_path = malloc(path_size);

        if (input_path == NULL) {
            fprintf(stderr, "Error: Memory allocation failed.\n");
            status = 1;
            break;
        }

        snprintf(input_path, path_size, "%s/%s",
                 input_dir, entry->d_name);

        struct stat info;
        if (stat(input_path, &info) != 0) {
            perror(input_path);
            free(input_path);
            status = 1;
            break;
        }

        // A folder may also have a name ending in .txt.
        if (!S_ISREG(info.st_mode)) {
            free(input_path);
            continue;
        }

        // Reuse the existing single-file processor with an explicit output.
        char *file_arguments[] = {
            "calc", "-o", (char *)output_dir, input_path
        };
        char *unused_default = NULL;

        int file_status =
            run_calculator(4, file_arguments, &unused_default);

        free(unused_default);
        free(input_path);

        if (file_status != 0) {
            status = 1;
            break;
        }
    }

    if (closedir(directory) != 0) {
        perror(input_dir);
        status = 1;
    }

    free(owned_output);
    return status;
}

int main(int argc, char *argv[]) {
    const char *input_dir = NULL;
    const char *input_file = NULL;
    const char *output_dir = NULL;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-o") == 0 ||
            strcmp(argv[i], "--output-dir") == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, "Error: Missing output directory.\n");
                return 1;
            }
            output_dir = argv[++i];
        } else if (strcmp(argv[i], "-d") == 0 ||
                   strcmp(argv[i], "--dir") == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, "Error: Missing input directory.\n");
                return 1;
            }
            if (input_dir != NULL) {
                fprintf(stderr, "Error: Multiple input directories.\n");
                return 1;
            }
            input_dir = argv[++i];
        } else if (argv[i][0] == '-') {
            fprintf(stderr, "Error: Unknown option: %s\n", argv[i]);
            return 1;
        } else {
            if (input_file != NULL) {
                fprintf(stderr, "Error: Too many input files.\n");
                return 1;
            }
            input_file = argv[i];
        }
    }

    if (input_dir != NULL && input_file != NULL) {
        fprintf(stderr, "Error: Choose an input file or a directory.\n");
        return 1;
    }

    if (input_dir != NULL) {
        return process_directory(input_dir, output_dir);
    }

    if (input_file == NULL) {
        fprintf(stderr, "Error: No input file or directory provided.\n");
        return 1;
    }

    char *default_directory = NULL;
    int status = run_calculator(argc, argv, &default_directory);
    free(default_directory);
    return status;
}