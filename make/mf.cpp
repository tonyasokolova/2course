#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <cerrno>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdexcept>
#include "exception.h"
#include "my_string.h"
#include "mf_lex.h"
#include "mf_syn.h"




int main()
{
    try
    {        
        Parser S("Makefile.txt");

        S.analyze();
        S.execute();
	
        return 0;
    }
    catch(const Exception &ex)
    {
        printf("%d:%d: %s\n",ex.get_str(),ex.get_num(),ex.get_comment());
    }
}
