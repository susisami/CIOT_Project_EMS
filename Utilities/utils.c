/* LIBRARIES */
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>

/* FUNCTIONS */

    // TRIM STRING GIVEN AS PARAMETER OF SPECIFIC SYMBOLS
    void trim_string(char *str)
    {
        const size_t len = strlen(str);

        for (int i = 0; i < len; i++)
        {
            if (str[i] == '\n' || str[i] == '\r')
            {
                str[i] = '\0';
            }
        }
    }

    // CLEAR STDIN IF BUFFER IS EXCEEDED
    void clear_stdin(const char *str)
    {
        if (!strchr(str, '\n'))
        {
            int c;
            while ((c = getchar()) != '\n' && c != EOF) { }
        }
    }

    // MODIFIES / FORMATS STRING GIVEN AS A PARAMETER
    void modify_string(char *str)
    {
        const size_t len = strlen(str);

        for (int i = 0; i < len; i++)
        {
            if (isupper(str[i]))
            {
                str[i] = tolower(str[i]);
            }
        }
    }

    // REMOVE SPECIFIC CHARACTER FROM STRING
    void remove_char(char *str, char c)
    {
        int j = 0;
        for (int i = 0; str[i] != '\0'; i++)
        {
            if (str[i] != c)
            {
                str[j++] = str[i];
            }
        }
        str[j] = '\0';
    }

    // VALIDATES INPUT AS A VALID LONG INTEGER = TRUE, OR FALSE
    bool is_integer(const char *str)
    {
        bool is_Valid = true;

        char *overflow;

        strtol(str, &overflow, 10);

        if (overflow == str || *overflow != '\0') is_Valid = false; // if there is overflow after a number has been found or if the overflow is the same as the str

        if (errno == ERANGE) is_Valid = false; // checks whether the input overflows accepted length of a long integer

        return is_Valid;
    }