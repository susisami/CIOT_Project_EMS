#ifndef UTILS_H
#define UTILS_H
/* UTILITY FUNCTIONS */

    // STRING + STDIN CHECKS
    void trim_string(char *str);
    void clear_stdin(const char *str);
    void modify_string(char *str);
    void remove_char(char *str, char c);

    // NUMBER VALIDATION
    bool is_integer(const char *str);


#endif //UTILS_H