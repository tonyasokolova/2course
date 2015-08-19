enum typeoflex
{
    LEX_WORD,
    LEX_NEW,
    LEX_TAB,
    LEX_NULL,
    LEX_COLON,
    LEX_EQ,
    LEX_SLASH,
    LEX_RPAREN,
    LEX_LPAREN,
    LEX_INCLUDE,
    LEX_LRULE,
    LEX_RRULE,
//    LEX_CR,
    LEX_PHONY  
};

class Token
{
    mystring inf;
    typeoflex type;
    int cur_str;
    int cur_num;
public:
    Token(typeoflex t, const char *s, int cns = 0, int cn = 0): type(t), inf(s), cur_str(cns), cur_num(cn) {}

    void print() const { std::cout << inf.ptr() << std::endl; };
    int get_cstr() const { return cur_str; }
    int get_cnum() const { return cur_num; }
    mystring  getdata() const { return inf; }
    typeoflex gettype() const { return type; }
};

class Token_LRule: public Token
{
public:
    Token_LRule(const char *w, int cns, int cn): Token(LEX_LRULE, w, cns, cn) {}
};

class Token_RRule: public Token
{
public:
    Token_RRule(const char *w, int cns, int cn): Token(LEX_RRULE, w, cns, cn) {}
};

class Token_Word: public Token
{
public:
    Token_Word(const char *w, int cns, int cn): Token(LEX_WORD, w, cns, cn) {}
};

class Token_Include: public Token
{
public:
    Token_Include(const char *w, int cns, int cn): Token(LEX_INCLUDE, w, cns, cn) {}
};

class Token_New: public Token
{
public:
    Token_New(const char *w, int cns, int cn): Token(LEX_NEW, w, cns, cn) {}
};

class Token_Tab: public Token
{
public:
	Token_Tab(const char *w, int cns, int cn): Token(LEX_TAB, w, cns, cn) {}
};

class Token_Null: public Token
{
public:
    Token_Null(): Token(LEX_NULL, "") {}
};

class Token_Colon: public Token
{
public:
	Token_Colon(const char *w, int cns, int cn): Token(LEX_COLON, w, cns, cn) {}
};

class Token_Eq: public Token
{
public:
	Token_Eq(const char *w, int cns, int cn): Token(LEX_EQ, w, cns, cn) {}
};

class Token_Slash: public Token
{
public:	
	Token_Slash(const char *w, int cns, int cn): Token(LEX_SLASH, w, cns, cn) {}
};

class Token_LParen: public Token
{
public:
    Token_LParen(const char *w, int cns, int cn): Token(LEX_LPAREN, w, cns, cn) {}
};

class Token_RParen: public Token
{
public:
    Token_RParen(const char *w, int cns, int cn): Token(LEX_RPAREN, w, cns, cn) {}
};
/*
class Token_CReturn: public Token
{
public:
	Token_CReturn(const char *w): Token(LEX_CR, w) {}
};
*/
class Token_Phony: public Token
{
public:
    Token_Phony(const char *w, int cns, int cn): Token(LEX_PHONY, w, cns, cn) {}
};

class Scanner
{
    enum state
    {
    	H, 
    	NEW_LINE,
    	WORD,
    	COLON,
    	DOLLAR,
    	SLASH,
    	EQ,
        LEFT_PAREN,
        RIGHT_PAREN,
        LEFT_RULE,
        RIGHT_RULE,
        ERR,
        TAB,
        INCLUDE,
    //  CARRIER_RETURN,
        PHONY
    };

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

    FILE *fp;
    int c;
    int cnum;
    int cstr;

    int gc()
    {
		cnum++;
        c = fgetc(fp);

        return c;
    }

public:
    int words;
    Token *get_lex();

    Scanner (const char *md)
    {
		cnum = 1;
		cstr = 1;
        fp = fopen(md, "r");
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
                    while (c == ' ') gc();
                }
                else if (c == '#')
                {
                    while ((c != '\n') && (c != EOF)) gc();
                }
                else if (c == ':')
                {
                	CS = COLON;
                }
                else if (c == '\n')
                {
                	CS = NEW_LINE;
                }
                else if (c == '$')
                {
                	CS = DOLLAR;
                }
                else if ((isalpha(c))
                        || (isdigit(c)) 
                        || (c == '.') 
                        || (c == '-') 
                        || (c == '*') 
                        || (c == '%')
                        || (c == '_')
                        || (c == '+'))
        		{
        			add();
        			CS = WORD;
        			gc();
        		}
                else if (c == '\\')
                {
                	CS = SLASH;
                }
                else if (c == EOF)
                {
					clear();
                    return new Token_Null();
                }
                else if (c == '\t')
                {
                	CS = TAB;
                }
                else if (c == '=')
                {
                	CS = EQ;
                }
                else if (c == '(')
                {
                    CS = ERR;
                }
                else if (c == ')')
                {
                	CS = RIGHT_PAREN;
                }
  /*              else if (c == '\r')
                {
					CS = CARRIER_RETURN;
				}
	*/			else
				{
					clear();
					throw Exception("*** Unknown symbols.",cstr,cnum);
				}
        	break;

        	case COLON:
        		add();
        		CS = H;
        		gc();
        		return new Token_Colon(buf,cstr,cnum);
        	break;

        	case NEW_LINE:
				cnum = 0;
				cstr++;
        		add();
        		CS = H;
        		gc();
        		return new Token_New(buf,cstr,cnum);
        	break;

        	case WORD:
        		if ((isalpha(c))
                   || (isdigit(c))  
				   || (c == '.') 
				   || (c == '-') 
				   || (c == '*') 
				   || (c == '%')
                   || (c == '_')
                   || (c == '+'))
        		{
        			add();
        			CS = WORD;
        			gc();
        		}
        		else
        		{
                    if (!strcmp(buf,".PHONY"))
                    {
                        CS = PHONY;
                    }
                    else if (!strcmp(buf,"include"))
                    {
                        CS = INCLUDE;
                    }
                    else
                    {
                        CS = H;
                        return new Token_Word(buf,cstr,cnum);
                    }
        		}
        	break;

        	case SLASH:
        		add();
        		CS = H;
        		gc();
        		return new Token_Slash(buf,cstr,cnum);
        	break;

        	case TAB:
        		add();
        		CS = H;
        		gc();
        		return new Token_Tab(buf,cstr,cnum);
        	break;

        	case DOLLAR:
        		add();
        		gc();

                if (c == '^')
                {
                    CS = RIGHT_RULE;
                }    
        		else if (c == '@')
                {
                    CS = LEFT_RULE;
                }
                else if (c == '(')
                {
                    CS = LEFT_PAREN;
                }
                else CS = ERR;
        	break;

        	case EQ:
        		add();
        		CS = H;
        		gc();
        		return new Token_Eq(buf,cstr,cnum);
        	break;

        	case LEFT_PAREN:
        		add();
                CS = H;
                gc();
                return new Token_LParen(buf,cstr,cnum);
        	break;

            case RIGHT_PAREN:
                add();
                CS = H;
                gc();
                return new Token_RParen(buf,cstr,cnum);
            break;

            case INCLUDE:
                CS = H;
                return new Token_Include(buf,cstr,cnum);
            break;

            case PHONY:
                CS = H;
                return new Token_Phony(buf,cstr,cnum);
            break;

            case LEFT_RULE:
                add();
                gc();
                CS = H;
                return new Token_LRule(buf,cstr,cnum);
            break;

            case RIGHT_RULE:
                add();
                gc();
                CS = H;
                return new Token_RRule(buf,cstr,cnum);
            break;
            /*
            case CARRIER_RETURN:
				add();
				CS = H;
				return new Token_CReturn(buf);
            break;
*/
            case ERR:
            	clear();
            	throw Exception("*** missing separator. Stop.",cstr,cnum);
            break;
        }
    }
}
