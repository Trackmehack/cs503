#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#define BUFFER_SZ 50

// Function prototypes
void usage(char *exename);
int setup_buff(char *buff, const char *user_str, int len);
int count_words(char *buff, int len, int str_len);
void reverse_string(char *buff, int str_len);
void print_words(char *buff, int len, int str_len);
int replace_word(char *buff, int len, const char *find, const char *replace);

// Main function
int main(int argc, char *argv[]) {
    if (argc < 3) {
        usage(argv[0]);
        exit(1);
    }

    char *buff = malloc(BUFFER_SZ);
    if (!buff) {
        fprintf(stderr, "Error: Memory allocation failed.\n");
        exit(99);
    }

    char *option = argv[1];
    char *user_str = argv[2];
    int user_len = setup_buff(buff, user_str, BUFFER_SZ);

    if (user_len < 0) {
        fprintf(stderr, "Error: Input string too large.\n");
        free(buff);
        exit(2);
    }

    if (strcmp(option, "-h") == 0) {
        usage(argv[0]);
    } else if (strcmp(option, "-c") == 0) {
        int count = count_words(buff, BUFFER_SZ, user_len);
        if (count >= 0) {
            printf("Word Count: %d\n", count);
        } else {
            fprintf(stderr, "Error counting words.\n");
        }
    } else if (strcmp(option, "-r") == 0) {
        reverse_string(buff, user_len);
        printf("Reversed String: %.*s\n", user_len, buff);
    } else if (strcmp(option, "-w") == 0) {
        print_words(buff, BUFFER_SZ, user_len);
    } else if (strcmp(option, "-x") == 0) {
        if (argc != 5) {
            fprintf(stderr, "Error: -x requires two additional arguments.\n");
            free(buff);
            exit(1);
        }
        int rc = replace_word(buff, BUFFER_SZ, argv[3], argv[4]);
        if (rc == 0) {
            printf("Modified String: %.*s\n", BUFFER_SZ, buff);
        } else {
            fprintf(stderr, "Error: %s\n", rc == -1 ? "Buffer overflow" : "Word not found");
        }
    } else {
        fprintf(stderr, "Invalid option. Use -h for help.\n");
    }

    free(buff);
    return 0;
}

// Function to display usage instructions
void usage(char *exename) {
    printf("usage: %s [-h|c|r|w|x] \"string\" [other args]\n", exename);
}
int setup_buff(char *buff, const char *user_str, int len) {
    char *buff_ptr = buff;
    const char *user_ptr = user_str;
    int count = 0;

    while (*user_ptr) {
        if (count >= len) return -1;
        if (!isspace(*user_ptr)) {
            *buff_ptr++ = *user_ptr;
            count++;
        } else if (buff_ptr != buff && *(buff_ptr - 1) != ' ') {
            *buff_ptr++ = ' ';
            count++;
        }
        user_ptr++;
    }

    if (buff_ptr != buff && *(buff_ptr - 1) == ' ') {
        buff_ptr--;
        count--;
    }

    while (count < len) {
        *buff_ptr++ = '.';
        count++;
    }
    return count;
}
int count_words(char *buff, int len, int str_len) {
    int word_count = 0, in_word = 0;

    for (int i = 0; i < str_len; i++) {
        if (buff[i] != ' ' && buff[i] != '.') {
            if (!in_word) {
                word_count++;
                in_word = 1;
            }
        } else {
            in_word = 0;
        }
    }
    return word_count;
}
void reverse_string(char *buff, int str_len) {
    char *start = buff;
    char *end = buff + str_len - 1;

    while (start < end) {
        char temp = *start;
        *start = *end;
        *end = temp;
        start++;
        end--;
    }
}
void print_words(char *buff, int len, int str_len) {
    char *ptr = buff;
    int word_index = 1;

    printf("Word Print\n----------\n");

    while (ptr < buff + str_len) {
        int word_len = 0;
        char *word_start = ptr;

        while (ptr < buff + str_len && *ptr != ' ' && *ptr != '.') {
            word_len++;
            ptr++;
        }

        if (word_len > 0) {
            printf("%d. %.*s (%d)\n", word_index++, word_len, word_start, word_len);
        }

        while (ptr < buff + str_len && (*ptr == ' ' || *ptr == '.')) {
            ptr++;
        }
    }
}
int replace_word(char *buff, int len, const char *find, const char *replace) {
    char *pos = buff;
    int find_len = 0, replace_len = 0;

    while (find[find_len]) find_len++;
    while (replace[replace_len]) replace_len++;

    while (*pos) {
        char *start = pos;
        const char *f = find;

        while (*pos && *f && *pos == *f) {
            pos++;
            f++;
        }

        if (!*f) {
            int shift = replace_len - find_len;
            if (shift > 0 && len - (pos - buff) < shift) return -1;

            memmove(pos + shift, pos, len - (pos - buff));
            for (int i = 0; i < replace_len; i++) *(start + i) = replace[i];
            return 0;
        }
        pos++;
    }
    return -2;
}
