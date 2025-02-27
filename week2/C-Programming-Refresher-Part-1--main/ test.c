#include <stdio.h>
#include <string.h>

void print_help();
void count_words(const char *str);
void reverse_string(char *str);
void word_print(const char *str);

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: stringfun -[h|c|r|w] \"sample string\"\n");
        return 1;
    }

    char *option = argv[1];
    char *input = argv[2];

    if (strcmp(option, "-h") == 0) {
        print_help();
    } else if (strcmp(option, "-c") == 0) {
        count_words(input);
    } else if (strcmp(option, "-r") == 0) {
        reverse_string(input);
        printf("Reversed string: %s\n", input);
    } else if (strcmp(option, "-w") == 0) {
        word_print(input);
    } else {
        printf("Invalid option. Use -h for help.\n");
    }

    return 0;
}

void print_help() {
    printf("Usage: stringfun -[h|c|r|w] \"sample string\"\n");
    printf("-h: Print this help message.\n");
    printf("-c: Count the number of words in the string.\n");
    printf("-r: Reverse the string.\n");
    printf("-w: Print each word and its length.\n");
}

void count_words(const char *str) {
    int count = 0;
    int in_word = 0;

    for (int i = 0; str[i] != '\0'; i++) {
        if (str[i] != ' ' && str[i] != '\t') {
            if (!in_word) {
                count++;
                in_word = 1;
            }
        } else {
            in_word = 0;
        }
    }
    printf("Word count: %d\n", count);
}

void reverse_string(char *str) {
    int length = strlen(str);
    for (int i = 0, j = length - 1; i < j; i++, j--) {
        char temp = str[i];
        str[i] = str[j];
        str[j] = temp;
    }
}

void word_print(const char *str) {
    int start = 0, end = 0;

    while (str[start] != '\0') {
        if (str[start] != ' ' && str[start] != '\t') {
            end = start;
            while (str[end] != ' ' && str[end] != '\t' && str[end] != '\0') {
                end++;
            }
            printf("Word: ");
            for (int i = start; i < end; i++) {
                putchar(str[i]);
            }
            printf(", Length: %d\n", end - start);
            start = end;
        } else {
            start++;
        }
    }
}
