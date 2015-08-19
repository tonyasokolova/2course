#include <cstring>

class mystring
{
    char *str;
public:

    mystring()
    {
        str = (char *) calloc(1, 1);
    }

    mystring(const char *s)
    {
        if (s)
        {
            str = (char *) malloc(strlen(s) + 1);
            strcpy(str, s);
        }
    }

    mystring(const mystring &s)
    {
        str = (char *) malloc(strlen(s.ptr()) + 1);
        strcpy(str, s.ptr());
    }

    mystring &operator=(const mystring &s)
    {
        free(str);
        str = (char *) malloc(strlen(s.ptr()) + 1);
        strcpy(str, s.ptr());
        return *this;
    }

    int operator==(const char *s)
    {
        if (!strcmp(str, s))
            return 1;
        else 
            return 0;
    }

    int operator!=(const char *s)
    {
        if (strcmp(str, s))
            return 1;
        else 
            return 0;
    }

    mystring &operator+=(const char *s)
    {
        str = (char *) realloc(str, strlen(s) + strlen(str) + 1);
        strcat(str, s);

        return *this;
    }

    mystring &operator+=(const mystring &s)
    {
        str = (char *) realloc(str, strlen(s.ptr()) + strlen(str) + 1);
        strcat(str, s.ptr());

        return *this;
    }

    int len() const { return strlen(str); }

    char *ptr() const { return str; };

    ~mystring() { free(str); }
};
