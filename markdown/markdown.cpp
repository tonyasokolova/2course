#include <iostream>
#include <termios.h>
#include <sys/ioctl.h>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <cerrno>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdexcept>
#include "my_string.h"
#include "Scanner.h"
#include "Parser.h"

class Text_viewer
{
    Text_Storage &storage;

public:
    Text_viewer(Text_Storage &ts): storage(ts) {}

    void show()
    {
        for (int i = 0; i < storage.size(); ++i)
        {
            storage[i]->print();
            std::cout << std::endl;
        }

        int words = 0, symbols = 0;
        for (int i = 0; i < storage.size(); ++i)
            words += storage[i]->countWords();
        for (int i = 0; i < storage.size(); ++i)
            symbols += storage[i]->countSymbols();  

        std::cout << "words: " << words << std::endl;
        std::cout << "symbols: " << symbols << std::endl;
    }
};

int main(int argc, char **argv)
{
    struct winsize ws;
    ioctl(0, TIOCGWINSZ, &ws);
    width = ws.ws_col;

    int c;

    char *path = NULL; 

    while ((c = getopt (argc, argv, "f:w:t:m:r:hv")) != -1)
        switch (c)
        {
        case 'f':
            path = optarg;
        break;

        case 'w':
            width = atoi(optarg); 
        break;

        case 't':
            tab = atoi(optarg); 
        break;

        case 'm':
            ul_marker = *optarg; 
        break;

        case 'r':
            red = atoi(optarg); 
        break;

        case 'h':
            std::cout << "NO HELP\n";
        break;

        case 'v':
            std::cout << 100500 << std::endl;
        break;

        default:
            std::cerr << "error with arguments" << std::endl;
            return 1;
        }

    try
    {
        if (!path) throw "Error: no input file specified.";
        
        Scanner S(path);
        Text_Storage Text;

        Parser P(S, Text);
        P.run();

        Text_viewer V(Text);
        V.show();
    }
    catch (char const *s)
    {
        std::cerr << s << std::endl;
    }
    return 0;
}