enum typeoflex
{
    LEX_WORD,
    LEX_ULIST,
    LEX_OLIST,
    LEX_SPACE,
    LEX_HEADER,
    LEX_BLANK,
    LEX_NULL   
};

class Token
{
    mystring inf;
    typeoflex type;
public:
    Token(typeoflex t, const char *s): type(t), inf(s) {}

    void print() const { std::cout << inf.ptr() << std::endl; };
    mystring  getdata() const { return inf; }
    typeoflex gettype() const { return type; }
};

class Token_Word: public Token
{
public:

    Token_Word(const char *w): Token(LEX_WORD, w) {}
};

class Token_Blank: public Token
{
public:

    Token_Blank(): Token(LEX_BLANK, "") {}
};

class Token_Null: public Token
{
public:

    Token_Null(): Token(LEX_NULL, "") {}
};

class Token_Space: public Token
{
public:

    Token_Space(const char *s): Token(LEX_SPACE, s) {}
};

class Token_Ulist: public Token
{
public:

    Token_Ulist(): Token(LEX_ULIST, "*") {}
};

class Token_Olist: public Token
{
public:

    Token_Olist(const char *s): Token(LEX_OLIST, s) {}
};

class Token_Header: public Token
{
public:

    Token_Header(const char *s): Token(LEX_HEADER, s) {}
};

class Scanner
{
    enum state{H, SPACES, GRIDS, ULIST, OLIST, RAW, HEADER, LIST};

    state CS;
    char buf[1024];
    int top;

    void clear()
    {
        buf[(top=0)] = 0;
    }

    void add()
    {
        buf[top++] = c;
        buf[ top ] = 0;
    }

    void add(int ch)
    {
        buf[top++] = ch;
        buf[ top ] = 0;
    }

    void add_space()
    {
        if ((top == 0) || (buf[top-1] != ' ')) add(' ');
    }

    FILE *fp;
    int c;

    int gc()
    {
        return (c = fgetc(fp));
    }
public:

    int words;
    Token *get_lex();

    Scanner (const char *md)
    {
        if ((fp = fopen(md, "r")) == NULL) throw "Error: No such file found.";
        CS = H;
        words = 0;
        clear();
        gc();
    }

    ~Scanner()
    {
        fclose(fp);
    }
};

Token *Scanner::get_lex()
{
    clear();
    for(;;)
    {
        switch(CS)
        {
            case H:
                if (c == ' ')
                {
                    CS = SPACES;
                }
                else if (c == '#')
                {
                    CS = GRIDS;
                }
                else if (c == '*')
                {
                    add();
                    CS = ULIST;
                    gc();
                }
                else if (isdigit(c))
                {
                    add();
                    CS = OLIST;
                    gc();
                }
                else if (c == '\n')
                {
                    while (gc() == '\n');
                    return new Token_Blank();
                }
                else if (c == EOF)
                {
                    return new Token_Null;
                }
                else
                {
                    CS = RAW;
                }
                
            break;

            case OLIST:
                while (isdigit(c))
                {
                    add();
                    gc();
                }
                if (c == '.')
                {
                    add();
                    gc();
                    if (c == ' ')
                    {
                        while (gc() == ' ');

                        if ((c == '\n') || (c == EOF))
                            CS = RAW;
                        else
                        {
                            CS = LIST;
                            return new Token_Olist(buf);
                        }

                    }
                    else CS = RAW;
                }
                else CS = RAW;

            break;

            case RAW:
                while ((c != '\n') && (c != EOF))
                {
                    if (c == ' ')
                    {
                        add_space();
                        gc();
                    }   
                    else if (c == '\\')
                    {
                        add();
                        gc();

                        if ((c == '\\') || (c == '#') || (c == '*'))
                        {
                            buf[strlen(buf) - 1] = c;
                            gc();
                            continue;
                        }
                    }
                    else
                    {
                        add();
                        gc();
                    }
                }

                if (c == '\n') gc();

                CS = H;
                return new Token_Word(buf);

            break;

            case SPACES:
                while (c == ' ')
                {
                    add();
                    gc();
                }

                if ((c == '\n') || (c == EOF))
                {
                    CS = H;
                    return new Token_Blank();
                }

                CS = H;
                return new Token_Space(buf);

            break;

            case GRIDS:
                while (c == '#')
                {
                    add();
                    gc();
                }

                if (c == ' ')
                {
                    while (gc() == ' ');

                    if ((c == '\n') || (c == EOF))
                        CS = RAW;
                    else 
                    {
                        CS = HEADER;
                        return new Token_Header(buf);
                    }
                }            
                else CS = RAW;

            break;

            case HEADER:
                while ((c != '\n') && (c != EOF))
                {
                    if (c == ' ')
                    {
                        words++;
                        add();
                        while (gc() == ' ');
                    }
                    else if (c == '\\')
                    {
                        add();
                        gc();

                        if ((c == '\\') || (c == '#') || (c == '*'))
                        {
                            buf[strlen(buf) - 1] = c;
                            gc();
                            continue;
                        }
                    }
                    else
                    {
                        add();
                        gc();
                    }
                }

                if (c == '\n') gc();

                CS = H;
                return new Token_Word(buf);

            break;

            case LIST:
                while ((c != '\n') && (c != EOF))
                {
                    if (c == ' ')
                    {
                        add();
                        while (gc() == ' ');
                    }
                    else if (c == '\\')
                    {
                        add();
                        gc();

                        if ((c == '\\') || (c == '#') || (c == '*'))
                        {
                            buf[strlen(buf) - 1] = c;
                            gc();
                            continue;
                        }
                    }
                    else
                    {
                        add();
                        gc();
                    }
                }

                if (c == '\n') gc();

                CS = H;
                return new Token_Word(buf);

            break;

            case ULIST:
                if (c == ' ')
                {
                    while (gc() == ' ');

                    if ((c == '\n') || (c == EOF))
                    {
                        CS = RAW;
                    }
                    else
                    {
                        CS = LIST;
                        return new Token_Ulist;
                    }

                }
                else CS = RAW;

            break;
        }
    }
}