#include <vector>


class Target
{
public:
	mystring target;
	std::vector<mystring>starget;
	std::vector<mystring>command;

	Target(mystring t): target(t) {} 
};

class Const
{
public:
	mystring name;
	std::vector<mystring> words;

	Const(mystring n): name(n) {} 
};


class Parser
{
	Token *curr_lex;
	typeoflex c_type;
	Scanner scan;
	std::vector<Target *>targets;
	std::vector<Const *> consts;

	void P();
	void I();
	void R(mystring);
	void PH();
	void END();
	void C();
	void SG();
	void EQ();
	void RLS1();
	void RL1();
	void RLS2();
	void RL2();
	void LP(int);
	
	void gl()
	{
		delete curr_lex;
		curr_lex = scan.get_lex();
		c_type = curr_lex->gettype();
	//	curr_lex->print();
	}

public:
	Parser(const char *program) : scan(program) 
	{ 
		curr_lex = scan.get_lex(); 
		c_type = curr_lex->gettype();
	//	curr_lex->print();
	}

	void execute(Target*, mystring);

	void analyze();

	~Parser()
	{
		delete curr_lex;
		for (auto it: targets) delete it;
		for (auto it: consts) delete it;	
	}
};

int matches(mystring s1, mystring s2)
{
	if (s1[0] == '%')
	{
		if (!strcmp(*s1+1, s2.find('.')))
			return 1;
		else
			return 0;
	}

	if (!strcmp(*s1, *s2))
		return 1;
	else
		return 0;
}

void Parser::execute(Target *initial = NULL, mystring tgt = "")
{
	if (!initial) initial = targets[0];

	for (auto &it: initial->starget)
	{
		for (auto &elem: targets)
		{
			if (matches(elem->target, it))
			{
				execute(elem, it);
			}
		}
	}


	char *args[1024];
	int arg_count = 0;

	for (auto &command: initial->command)
	{
		if (command == "$@")
		{
			args[arg_count++] = tgt.ptr();
		}
		else if (command == "$^")
		{
			for (auto &elem: initial->starget)
				args[arg_count++] = *elem;
		}
		else
			args[arg_count++] = *command;
	}

	args[arg_count] = NULL;

	for (arg_count = 0; args[arg_count]; ++arg_count)
		std::cout << args[arg_count] << ' ';

	std::cout << std::endl;

	int status = 0;

	if (fork())
	{
		wait(&status);
	}
	else
	{
		if (*args) execvp(*args, args);
	}
}


void Parser::analyze()
{
		P();
}

void Parser::P()
{
	if (c_type == LEX_INCLUDE)
	{
		gl(); I();
	}
	else if (c_type == LEX_WORD)
	{
		mystring tmp = curr_lex->getdata();
		gl(); R(tmp);
	}
	else if ((c_type == LEX_NEW)
		   ||(c_type == LEX_TAB)
		   ||(c_type == LEX_SLASH))
	{
		gl(); P();
	}
	else if (c_type == LEX_PHONY)
	{
		gl(); PH();
	}
	else if (c_type == LEX_NULL)
	{
		END();
	}
	else throw Exception("Error",curr_lex->get_cstr(),curr_lex->get_cnum());
}

void Parser::I()
{
	if (c_type == LEX_WORD)
	{
//		Parser newfile(curr_lex->getdata().ptr());
//		newfile.analyze();

		gl();

		if (c_type == LEX_NEW)
		{
			gl(); P();
		}
		else throw Exception("Error",curr_lex->get_cstr(),curr_lex->get_cnum());;
	}
	else throw Exception("*** missing separator. Stop.",curr_lex->get_cstr(),curr_lex->get_cnum());
}

void Parser::R(mystring targ)
{
	if (c_type == LEX_EQ)
	{
		consts.push_back(new Const(targ));
		gl(); EQ();
	}
	else if (c_type == LEX_COLON)
	{
		targets.push_back(new Target(targ));
		gl(); C();
	}
	else throw Exception("*** missing separator. Stop.",curr_lex->get_cstr(),curr_lex->get_cnum());
}

void Parser::EQ()
{
	if (c_type == LEX_WORD)
	{
		consts.back()->words.push_back(curr_lex->getdata());
		gl();

		if (c_type == LEX_NEW)
		{
			gl(); P();
		}
		else if (c_type == LEX_WORD)
		{
			gl(); EQ();
		}
		else Exception("Error",curr_lex->get_cstr(),curr_lex->get_cnum());
	}
	else if (c_type == LEX_SLASH)
	{
		while (c_type == LEX_SLASH)
		{
			gl();
		}

		if (c_type == LEX_NEW)
		{
			gl();

			if (c_type != LEX_WORD)
				throw Exception("Error",curr_lex->get_cstr(),curr_lex->get_cnum());
		}

		EQ();
	}
/*	else if (c_type == LEX_TAB)
	{
		gl(); EQ();
	}
	else if (c_type == LEX_NEW)
	{
		gl(); P();
	}
*/	else throw Exception("*** missing separator. Stop.",curr_lex->get_cstr(),curr_lex->get_cnum());
}

void Parser::PH()
{
	if (c_type != LEX_COLON)
		throw Exception("*** missing separator. Stop.",curr_lex->get_cstr(),curr_lex->get_cnum());

	gl(); C();
}

void Parser::C()
{
	if (c_type == LEX_NEW)
	{
		gl(); RLS1();
	}
	else if (c_type == LEX_SLASH)
	{
		while (c_type == LEX_SLASH)
		{
			gl();
		}

		if (c_type != LEX_NEW)
			throw Exception("Error",curr_lex->get_cstr(),curr_lex->get_cnum());

		gl(); C();
	}
	else if (c_type == LEX_WORD)
	{
		targets.back()->starget.push_back(curr_lex->getdata());
		gl(); SG();
	}
	else if (c_type == LEX_LPAREN)
	{
		gl(); LP(0); SG();
	}
	else throw Exception("*** missing separator. Stop.",curr_lex->get_cstr(),curr_lex->get_cnum());
}

void Parser::RLS1()
{
	if ((c_type == LEX_SLASH) || (c_type == LEX_NEW))
	{
		gl(); RLS1();
	}
	else if (c_type == LEX_TAB)
	{
		gl(); RL1();
		
		if (c_type == LEX_WORD)
		{
			mystring tmp = curr_lex->getdata();
		    gl(); R(tmp);
		}
	}
	else throw Exception("*** missing separator. Stop.",curr_lex->get_cstr(),curr_lex->get_cnum());
}

void Parser::RL1()
{
	if (c_type == LEX_WORD)
	{
		targets.back()->command.push_back(curr_lex->getdata());
		gl(); RL1();
	}
	else if (c_type == LEX_LRULE)
	{
		gl(); RL1();
	}
	else if (c_type == LEX_LPAREN)
	{
		gl(); LP(1); RL1();
	}
	else if (c_type == LEX_SLASH)
	{
		while (c_type == LEX_SLASH)
		{
			gl();
		}

		if (c_type == LEX_NEW)
		{
			gl();

			if ((c_type != LEX_WORD)
			  ||(c_type != LEX_LRULE)
			  ||(c_type != LEX_LPAREN))
				throw Exception("Error",curr_lex->get_cstr(),curr_lex->get_cnum());
		}

		RL1();
	}
	else if (c_type == LEX_NEW)
	{
		gl(); RLS1();
	}
	else if (c_type == LEX_NULL)
	{
		P();
	}
	else throw Exception("*** missing separator. Stop.",curr_lex->get_cstr(),curr_lex->get_cnum());
}

void Parser::LP(int n)
{
	if (c_type != LEX_WORD)
		throw Exception("*** missing separator. Stop.",curr_lex->get_cstr(),curr_lex->get_cnum());

	for (auto c: consts)
		if (matches(c->name, curr_lex->getdata()))
			for (auto w: c->words)
			{
				if (n == 0)
					targets.back()->starget.push_back(w);
				else
					targets.back()->command.push_back(w);
			}


	gl();

	if (c_type != LEX_RPAREN)
		throw Exception("*** missing ')'. Stop.",curr_lex->get_cstr(),curr_lex->get_cnum());

	gl();
}

void Parser::SG()
{
	if (c_type == LEX_WORD)
	{
		targets.back()->starget.push_back(curr_lex->getdata());
		gl(); SG();
	}
	else if (c_type == LEX_LPAREN)
	{
		gl(); LP(0); SG();
	}
	else if (c_type == LEX_SLASH)
	{
		while (c_type == LEX_SLASH)
		{
			gl();
		}

		if (c_type == LEX_NEW)
		{
			gl();

			if ((c_type != LEX_WORD)
			  ||(c_type != LEX_LPAREN))
				throw Exception("Error",curr_lex->get_cstr(),curr_lex->get_cnum());
		}

		SG();
	}
	else if (c_type == LEX_NEW)
	{
		gl(); RLS2();
	}
	else throw Exception("*** missing separator. Stop.",curr_lex->get_cstr(),curr_lex->get_cnum());
}

void Parser::RLS2()
{
	if ((c_type == LEX_SLASH) || (c_type == LEX_NEW))
	{
		gl(); RLS2();
	}
	else if (c_type == LEX_TAB)
	{
		gl(); RL2();
	}
	else if (c_type == LEX_WORD)
	{
		mystring tmp = curr_lex->getdata();
		gl(); R(tmp);
	}
	else throw Exception("*** missing separator. Stop.",curr_lex->get_cstr(),curr_lex->get_cnum());
}

void Parser::RL2()
{
	if (c_type == LEX_WORD)
	{
		targets.back()->command.push_back(curr_lex->getdata());
		gl(); RL2();
	}
	else if (c_type == LEX_LRULE)
	{
		targets.back()->command.push_back(curr_lex->getdata());
		gl(); RL2();
	}
	else if (c_type == LEX_RRULE)
	{
		targets.back()->command.push_back(curr_lex->getdata());
		gl(); RL2();
	}
	else if (c_type == LEX_LPAREN)
	{
		gl(); LP(1); RL2();
	}
	else if (c_type == LEX_SLASH)
	{
		while (c_type == LEX_SLASH)
		{
			gl();
		}

		if (c_type == LEX_NEW)
		{
			gl();

			if ((c_type != LEX_WORD)
			  ||(c_type != LEX_RRULE)
			  ||(c_type != LEX_LRULE)
			  ||(c_type != LEX_LPAREN))
				throw Exception("Error",curr_lex->get_cstr(),curr_lex->get_cnum());
		}

		RL2();
	}
	else if (c_type == LEX_NEW)
	{
		gl(); RLS2();
	}
	else if (c_type == LEX_NULL)
	{
		P();
	}
	else throw Exception("*** missing separator. Stop.",curr_lex->get_cstr(),curr_lex->get_cnum());
}

void Parser::END()
{
	printf("The end of file. \n");
}
