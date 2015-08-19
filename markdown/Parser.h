int width = 40, 
    tab = 2, 
    red = 0;

char ul_marker = '*';

enum typeoftext
{
    TEXT_RAW,
    TEXT_ULIST,
    TEXT_OLIST,
    TEXT_HEADER, 
    TEXT_NULL,  
};

int cnt_digits(int num){ return ( num /= 10 ) ? 1 + cnt_digits(num) : 1; }

class Abstract_text
{
    typeoftext type;
    mystring text;
public:
    Abstract_text(typeoftext t = TEXT_NULL): type(t) {}

    Abstract_text(const mystring txt, typeoftext t): text(txt), type(t) {}

    mystring gets() const { return text; }
    typeoftext gettype() const { return type; }

    virtual unsigned long int countSymbols() const = 0;
    virtual unsigned long int countWords() const = 0;
    virtual void print() const = 0;

    virtual ~Abstract_text() {}
};

class Text_Storage
{
    Abstract_text **storage;
    int _size;
public:

    Text_Storage(): storage(NULL), _size(0) {}
    ~Text_Storage() 
    { 
        for (int i = 0; i < _size; delete storage[i++]); 
        free(storage);
    }

    void push(Abstract_text *T)
    {
        storage = (Abstract_text **) realloc(storage, ++_size * sizeof(Abstract_text *));
        storage[_size-1] = T;
    }

    Abstract_text *top() const { return _size ? storage[_size-1] : NULL; }

    Abstract_text * operator[](int pos) const { return storage[pos]; }

    int size() const { return _size; }

};

class Raw_text: public Abstract_text
{
    int indent;
public:
    Raw_text(const mystring txt, int ind): Abstract_text(txt, TEXT_RAW), indent(ind) {}

    unsigned long int countSymbols() const
    {
        int symbols = 0;

        mystring str = gets();
        for (char *word = strtok(str.ptr(), " "); word; word = strtok(NULL, " ")) 
            symbols += strlen(word);
        return symbols;
    }

    unsigned long int countWords() const
    {
        int words = 0;

        mystring str = gets();
        for (char *word = strtok(str.ptr(), " "); word; word = strtok(NULL, " ")) 
            ++words;
        return words;
    }

    void set_indent(int ind) { indent = ind; }

    virtual void print() const
    {
        int cur_width = width - indent;
        bool newline = indent ? false : true;

        mystring str = gets();
        char *word = strtok(str.ptr(), " ");
        while (word)
        {
            if (strlen(word) > width - indent) throw "Error: too long word.";

            if (newline)
            {
                newline = false;
                for (int i = 0; i++ < indent; std::cout << ' ');
                std::cout << word;
                cur_width -= strlen(word);
            }
            else
            {
                if (strlen(word) >= cur_width)
                {
                    std::cout << std::endl;
                    newline = true;
                    cur_width = width - indent;
                    continue;
                }
                else
                {
                    std::cout << ' ' << word;
                    cur_width -= strlen(word) + 1;
                }
            }
            word = strtok(NULL, " ");
        }
        std::cout << std::endl;
    }
};

class Header: public Abstract_text
{
    int level; 

public:
    Header(int l, const mystring txt): Abstract_text(txt, TEXT_HEADER), level(l) {}

    unsigned long int countSymbols() const
    {
        int symbols = 0;

        mystring str = gets();
        for (char *word = strtok(str.ptr(), " "); word; word = strtok(NULL, " ")) 
            symbols += strlen(word);
        return symbols;
    }

    unsigned long int countWords() const
    {
        int words = 0;

        mystring str = gets();
        for (char *word = strtok(str.ptr(), " "); word; word = strtok(NULL, " ")) 
            ++words;
        return words;
    }

    virtual void print() const
    {
        if (gets().len() + 2*level > width)
            throw "Error: Can't fit header in a line.";

        for (int i = 0; i++ < width; std::cout << '#');
        std::cout << std::endl;

        for (int i = 0; i++ < level; std::cout << '#');

        int spaces = (width - 2*level - gets().len())/2;
        for (int i = 0; i++ < spaces; std::cout << ' ');

        std::cout << gets().ptr();

        spaces += (width - 2*level - gets().len())%2;
        for (int i = 0; i++ < spaces; std::cout << ' ');

        for (int i = 0; i++ < level; std::cout << '#');
        std::cout << std::endl;

        for (int i = 0; i++ < width; std::cout << '#');
        std::cout << std::endl;
    }
};

class List: public Abstract_text
{
    Text_Storage elems;
    int level;

public:
    List(typeoftext t, int l): Abstract_text(t), level(l) {}

    Text_Storage &get() { return elems; }

    unsigned long int countSymbols() const
    {
        int symbols = 0;

        for (int i = 0; i < elems.size(); ++i)
            symbols += elems[i]->countSymbols();
        return symbols;
    }

    unsigned long int countWords() const
    {
        int words = 0;

        for (int i = 0; i < elems.size(); ++i)
            words += elems[i]->countWords();
        return words;
    }

    virtual void print() const
    {
        for (int i = 0; i < elems.size(); ++i)
        {
            if ((elems[i]->gettype() == TEXT_OLIST) || (elems[i]->gettype() == TEXT_ULIST))
            {
                elems[i]->print();
            }
            else if (gettype() == TEXT_OLIST)
            {
                for (int j = 0; j++ < level*tab; std::cout << ' ');
                std::cout << i+1 << '.';
                dynamic_cast<Raw_text *>(elems[i])->set_indent(level*tab + cnt_digits(i+1) + 2);
                elems[i]->print();
            }
            else
            {
                for (int j = 0; j++ < level*tab; std::cout << ' ');
                std::cout << ul_marker;
                elems[i]->print();
            }
        }
    }
};

class Parser
{
    Scanner &S;
    Token *cur_lex;
    Text_Storage &storage;

public:
    Parser(Scanner &s, Text_Storage &ts): S(s), storage(ts) { cur_lex = S.get_lex(); }
    ~Parser() { delete cur_lex; }

    int scan_list(Abstract_text *, int, int);
    void run();

    Token *gl() { delete cur_lex; return cur_lex = S.get_lex(); }
};

int Parser::scan_list(Abstract_text *L, int s, int level)
{
    int spaces = s, elem_num = 0;
    mystring tmp;

    while (true) switch (cur_lex->gettype())
    {
        case LEX_BLANK:
            gl();
        case LEX_NULL:
            dynamic_cast<List *>(L)->get().push(new Raw_text(tmp, level*tab+2));
            return -1;
        break;
        
        case LEX_SPACE:
            spaces = cur_lex->getdata().len();
            gl();
        break;

        case LEX_ULIST:
        case LEX_OLIST:
            if (tmp.len())
            {
                dynamic_cast<List *>(L)->get().push(new Raw_text(tmp, level*tab+2));
                tmp = "";
            }

            if      (spaces == s) 
            {
                gl();   
            }
            else if (spaces <  s) 
            {
                return spaces; 
            }
            else if (spaces >  s) 
            { 
                if (cur_lex->gettype() == LEX_OLIST)
                    dynamic_cast<List *>(L)->get().push(new List(TEXT_OLIST, level+1));
                else 
                    dynamic_cast<List *>(L)->get().push(new List(TEXT_ULIST, level+1));

                spaces = scan_list(dynamic_cast<List *>(L)->get().top(), spaces, level+1);
                break;
            }

            spaces = 0;
        break;

        case LEX_HEADER:
            tmp += cur_lex->getdata() += " ";
            gl();

        case LEX_WORD:
            tmp += cur_lex->getdata() += " ";
            gl();
            spaces = 0;
        break;
    }
}

void Parser::run()
{
    int spaces = 0;
    mystring tmp;

    while (true) switch (cur_lex->gettype())
    {
        case LEX_BLANK:
            if (tmp.len()) storage.push(new Raw_text(tmp, 0));

            spaces = 0;
            tmp = "";
            gl();
        break;

        case LEX_WORD:
            spaces = 0;
            tmp += cur_lex->getdata() += " ";
            gl();
        break;

        case LEX_HEADER:
            if (tmp.len())
                tmp += cur_lex->getdata() += " ";
            else
                storage.push(new Header(cur_lex->getdata().len(), gl()->getdata()));

            gl();
        break;

        case LEX_SPACE:
            spaces = cur_lex->getdata().len();
            gl();
        break;

        case LEX_OLIST:
            if (tmp.len())
                tmp += cur_lex->getdata() += " ";
            else
            {
                gl();
                storage.push(new List(TEXT_OLIST, 1));
                scan_list(storage.top(), spaces, 1);
            }
        break;

        case LEX_ULIST:
            if (tmp.len())
                tmp += cur_lex->getdata() += " ";
            else
            {
                gl();
                storage.push(new List(TEXT_ULIST, 1));
                scan_list(storage.top(), spaces, 1);
            }
        break;

        default:  
            if (tmp.len()) storage.push(new Raw_text(tmp, 0));
            return;
    }
}