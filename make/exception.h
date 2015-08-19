class Exception
{
    char *comment;
    int cur_str;
    int cur_num;

public:
    Exception(const char *cmt, int cstr, int cnum);
    Exception(const Exception& other);
    ~Exception();
    const char *get_comment() const { return comment; }
    int get_str() const { return cur_str; }
    int get_num() const { return cur_num; }

private:
    static char *strdup(const char *str);
};

Exception::Exception(const char *cmt, int cstr, int cnum)
{
    comment = strdup(cmt);
    cur_num = cnum;
    cur_str = cstr;
}

Exception::Exception(const Exception& other)
{
    comment = strdup(other.comment);
    cur_num = other.cur_num;
    cur_str = other.cur_str;
}

Exception::~Exception()
{
    delete [] comment;
}  

char* Exception::strdup(const char *str)
{
    char *res = new char[strlen(str) + 1];
    strcpy(res, str);
    return res;
}
