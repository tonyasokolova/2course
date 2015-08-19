#include <stdio.h>
#include <stdlib.h> 
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/stat.h>
#include <fcntl.h>  
#include <sys/wait.h>
#include <signal.h>

#define REWRITE 1
#define APPEND  1
#define LEN 10
#define VAR_SIZE 1024

#define PASS    0x0   //00000000
#define ADDWORD 0x1   //00000001
#define ADDCHAR 0x2   //00000010
#define ADDVAR  0x4   //00000100
#define CHANGECHAR 0x6//00001000
#define NEWLINE 0x40  //01000000

int lastflag = 0;
pid_t sigpid = 0;

char **line = NULL;

typedef struct program
{
    uid_t uid;
    pid_t pid;
    int prog_status;
	char* name;
	int number_of_arguments;
	char** arguments;
	char *input_file, *output_file;
	int output_type; /* REWRITE/ APPEND*/
} Program;

struct job
{
	int background;  //если наша задача выполняется в фоновом режиме, то значение 1
    char **program;  //все элементы программы, включающие символы конвейера, перенаправления и т.д.
    char ***conv;    //тут массив подзадач, разбитый из program по признаку конвейера ('|')
	int number_of_programs;  //количество элементов конвейера
} Job;

struct job *shell_jobs;
int jobs_count;

struct program program;
struct job job;
/*
void handler(int s)
{
    free_all();
    kill(sigpid,SIGINT);
}
*/

//flags
//0 - standart
//1 - "
//2 - '
//3 - &  or | or < or > 
/*4 - \ */
//5 - $

int analyze_symbol(int c, int *flag)
{
    switch (c)
    {
        case '"':   if (*flag == 0)      { *flag = 1; return PASS; }
                    if (*flag == 1)      { *flag = 0; return PASS; }
                    if (*flag == 2)                   return ADDCHAR;
                    if (*flag == 3)      { *flag = 1; return ADDWORD | ADDCHAR; }
                    if (*flag == 4)		 { *flag = lastflag; return CHANGECHAR; }
                    if (*flag == 5)      { *flag = 0; return ADDVAR | ADDWORD;}

        case '\'':  if (*flag == 0)      { *flag = 2; return PASS; }
                    if (*flag == 1)                   return ADDCHAR;
                    if (*flag == 2)      { *flag = 0; return PASS; }
                    if (*flag == 3)      { *flag = 2; return ADDWORD | ADDCHAR; }
                    if ((*flag == 4) && (lastflag == 0))   { *flag = lastflag; return CHANGECHAR; }
                    if ((*flag == 4) && (lastflag == 1))   { *flag = lastflag; return ADDCHAR; }
                    if ((*flag == 4) && (lastflag == 2))   { *flag = 0;        return PASS; }
                    if ((*flag == 4) && (lastflag  > 2))   { *flag = 0;        return CHANGECHAR; }
                    if (*flag == 5)      { *flag = 1; return ADDVAR | ADDCHAR; }

        case '#':   if (*flag == 0)		 { return ADDWORD | NEWLINE; }
        			if (*flag == 1)		   return ADDCHAR;
        			if (*flag == 2)		   return ADDCHAR;
        			if (*flag == 3)		 { return ADDWORD | NEWLINE; }
        			if ((*flag == 4) && (lastflag == 0))   { *flag = lastflag; return CHANGECHAR; }
                    if ((*flag == 4) && (lastflag == 1))   { *flag = lastflag; return ADDCHAR; }
                    if ((*flag == 4) && (lastflag == 2))   { *flag = lastflag; return ADDCHAR; }
                    if ((*flag == 4) && (lastflag  > 2))   { *flag = 0;        return CHANGECHAR; }
                    if (*flag == 5)      { return PASS; }

        case ' ':   if (*flag == 0)                          return ADDWORD;
                    if (*flag == 1)                          return ADDCHAR;
                    if (*flag == 2)                          return ADDCHAR;
                    if (*flag == 3)                          return ADDWORD;
                    if (*flag == 4)		 { *flag = lastflag; return ADDCHAR; }
                    if (*flag == 5)      { *flag = 1;        return ADDVAR | ADDCHAR; }          

        case '\\':  if (*flag == 0) 	 { *flag = 4; lastflag = 0; return PASS; }
        			if (*flag == 1)		 { *flag = 4; lastflag = 1; return ADDCHAR; }
        			if (*flag == 2)		 { *flag = 4; lastflag = 2; return ADDCHAR; }  
        			if (*flag == 3)		 { *flag = 4; lastflag = 3; return ADDWORD; }
        			if ((*flag == 4) && (lastflag == 0))   { *flag = lastflag; return CHANGECHAR; }
                    if ((*flag == 4) && (lastflag == 1))   { *flag = lastflag; return CHANGECHAR; }
                    if ((*flag == 4) && (lastflag >= 2))   { *flag = lastflag; return ADDCHAR; }
                    if (*flag == 5)      { return PASS; }

        case '$':   if (*flag == 0)      { return ADDCHAR; }
                    if (*flag == 1)      { *flag = 5; return PASS; }
                    if (*flag == 2)      { return ADDCHAR; }
                    if (*flag == 3)      { *flag = 0; return ADDWORD | ADDCHAR; }
                    if ((*flag == 4) && (lastflag == 0))   { *flag = lastflag; return CHANGECHAR; }
                    if ((*flag == 4) && (lastflag == 1))   { *flag = lastflag; return CHANGECHAR; }
                    if ((*flag == 4) && (lastflag >= 2))   { *flag = lastflag; return ADDCHAR; }
                    if (*flag == 5)      { return ADDVAR; }

        case ';':
        case '&':
        case '|':
        case '<':  
        case '>':   if (*flag == 0)      { *flag = 3; return ADDWORD | ADDCHAR; }
                    if (*flag == 1)      return ADDCHAR;
                    if (*flag == 2)      return ADDCHAR;
                    if (*flag == 3)      return ADDWORD | ADDCHAR; 
                    if ((*flag == 4) && (lastflag == 0))   { *flag = lastflag; return CHANGECHAR; }
                    if ((*flag == 4) && (lastflag >= 1))   { *flag = lastflag; return ADDCHAR; }
                    if (*flag == 5)      { *flag = 1; return ADDVAR | ADDCHAR; }

        case ':':   if (*flag == 3)      { *flag = 0; return ADDWORD | ADDCHAR; }  
                    if ((*flag == 4) && (lastflag == 0))   { *flag = lastflag; return CHANGECHAR; }
                    if ((*flag == 4) && (lastflag == 1))   { *flag = lastflag; return ADDCHAR; }
                    if ((*flag == 4) && (lastflag == 2))   { *flag = lastflag; return ADDCHAR; }
                    if ((*flag == 4) && (lastflag  > 2))   { *flag = 0;        return CHANGECHAR; }
                    if (*flag == 5)   {*flag = 1; return ADDVAR | ADDCHAR; }

        case '\n':  if (*flag == 0)      { return ADDWORD | NEWLINE; }
                    if (*flag == 1)        return ADDCHAR;
                    if (*flag == 2)        return ADDCHAR;
                    if (*flag == 3)        return ADDWORD | NEWLINE; 
                    if (*flag == 4)      { *flag = lastflag; return PASS; }
                    if (*flag == 5)        return ADDCHAR; 

        case EOF:   return ADDWORD | NEWLINE; 
        
        default:    if (*flag == 3)      { *flag = 0; return ADDWORD | ADDCHAR; }  
        			if ((*flag == 4) && (lastflag == 0))   { *flag = lastflag; return CHANGECHAR; }
                    if ((*flag == 4) && (lastflag == 1))   { *flag = lastflag; return ADDCHAR; }
                    if ((*flag == 4) && (lastflag == 2))   { *flag = lastflag; return ADDCHAR; }
                    if ((*flag == 4) && (lastflag  > 2))   { *flag = 0;        return CHANGECHAR; }
                    else return ADDCHAR;
    }
}


char* var_handler(char* varbuffer)
{
    int digit = varbuffer[1] - '0';
    char* tmp = (char *) calloc(VAR_SIZE, 1);
    strcpy(tmp, varbuffer+2);
    program.name = (char *) calloc(VAR_SIZE, 1);

    if (!strcmp(varbuffer, "$HOME"))  strcpy(varbuffer, getenv("HOME"));
    if (!strcmp(varbuffer, "$USER"))  strcpy(varbuffer, getenv("USER"));
    if (!strcmp(varbuffer, "$SHELL")) strcpy(varbuffer, getenv("SHELL"));
    if (!strcmp(varbuffer, "$PWD"))   strcpy(varbuffer, getenv("PWD"));

    if (!strcmp(varbuffer, "$HOSTNAME"))  
    {
        gethostname(program.name,VAR_SIZE);
        strcpy(varbuffer,program.name);
    }

    if ((digit >= 0) && (digit <= 9))
    {
        strcpy(varbuffer, program.arguments[digit]);
        strcat(varbuffer, tmp);
    } 

    if (!strcmp(varbuffer, "$UID"))  sprintf(varbuffer, "%d", program.uid);
    if (!strcmp(varbuffer, "$PID"))  sprintf(varbuffer, "%d", program.pid);
    if (!strcmp(varbuffer, "$#"))    sprintf(varbuffer, "%d", program.number_of_arguments);
    if (!strcmp(varbuffer, "$?"))    sprintf(varbuffer, "%d", program.prog_status);
    
    free(program.name);
    free(tmp);
    return varbuffer;
}

char **command_string()
{
	char *word = calloc(1, 1);
	char **line = NULL;
	int line_len = 0, linebuf_len = sizeof(char*);
	int charbuf_len = 1, varbuf_len = 0;
    char* tmp = NULL;
    char *varbuffer = (char *) calloc(VAR_SIZE, 1);
    char** linetmp = NULL;
    int c = 0;
	int words = 0, chars = 0;
	int flag1 = 0, flag2 = 0;/* флаги */ 

	while (!(flag2 & NEWLINE))
	{
        flag2 = analyze_symbol(c = getchar(), &flag1);

        if (chars && (!strcmp(word, "|") || !strcmp(word, "||") || !strcmp(word, "&&")) && (c=='\n')) 
            flag2 = ADDWORD;

        if ((c=='#') && words && line[words-1] && (!strcmp(line[words-1], "||") || !strcmp(line[words-1], "&&") || !strcmp(line[words-1], "|")))
        {
            while(getchar() != '\n');
            flag2 = PASS;
        }
        //if ((flag1==3) && (c == '#'))
        //{
        //    while(getchar() != '\n');
        //    flag2 = NEWLINE;
       // }

        if (chars && (c == '>') && !strcmp(word, ">")) flag2 = ADDCHAR;

        if (chars && (c == '|') && !strcmp(word, "|")) flag2 = ADDCHAR;

        if (chars && (c == '&') && !strcmp(word, "&")) flag2 = ADDCHAR;

        if (flag2 & ADDVAR)
        {
            charbuf_len += 1024;
            word = (char *) realloc(word, charbuf_len);
            strcpy(word+chars, var_handler(varbuffer));
            chars = strlen(word);
            free(varbuffer);
            varbuffer = (char *) calloc(VAR_SIZE, 1);
            varbuf_len = 0;
        }

        if (flag1 == 5)
        {
            varbuffer[varbuf_len++] = c;
            continue;
        }

		if ((flag2 & ADDWORD) && *word)
		{
            line_len += sizeof(char*);

			if (line_len >= linebuf_len) 
            {
                linebuf_len *= LEN;
                line = (char **) realloc(line, linebuf_len);
            } 

            tmp = (char *) malloc(chars+1);
            memcpy(tmp, word, chars+1);
			line[++words-1] = tmp;
            chars = 0;
            charbuf_len = 1;
            free(word);
            word = (char *) calloc(1,1);
		}

		if ((flag2 & ADDCHAR))
		{
            if (++chars >= charbuf_len)
            {
                charbuf_len *= LEN;
                word = (char *) realloc(word, charbuf_len);
            }

            word[chars-1] = c;
            word[chars] = '\0';
		}

        if ((flag2 & CHANGECHAR))
            word[chars-1] = c;
	}

    free(varbuffer);

    if (c == '#')
        while(getchar() != '\n');

    if (!line)
    {
        free(word); 
        return NULL;
    }

    linetmp = (char **) malloc((words+1)*sizeof(char *));
    memcpy(linetmp, line, words*sizeof(char *));
    linetmp[words] = NULL;

    if (word != NULL) 
        free(word);

    free(line);

    return linetmp;
}

//определяет, есть ли аргумент key в массиве аргументов prog 
int find(char **prog, char *key)
{
    int i = 0;

    while (prog[i])
        if (!strcmp(prog[i++], key)) return 1;

    return 0;
}

//освобождаем char ***
void free_line_pr(char ***line_pr)
{
    int i = 0;

    while(line_pr[i])
        free(line_pr[i++]);

    free(line_pr);
}

//освобождаем char **
void free_line(char **line)
{
    int i = 0;

    while(line[i])
        free(line[i++]);

    free(line);
}

void free_jobs()
{
    int i=0;

    for(i = 0; i < jobs_count; i++)
    {
        free(shell_jobs[i].program);
        free_line_pr(shell_jobs[i].conv);
    }

    free(shell_jobs);

}

void free_all()
{
    if (line)       free_line(line);
    if (jobs_count) free_jobs();
}


//separator - разделитель, например ";" или "||"
char ***massiv_prog(char **line, char *separator)
{
    int i = 0;

    //это наш массив
    char ***str = (char ***) malloc(sizeof(char **)); //сначала там будет единственный элемент NULL
    *str = NULL; // NULL у нас всегда будет последним элементом str

    //это аргументы текущей программы
    char **tmpprog = (char **) malloc(sizeof(char *)); //сначала там будет единственный элемент NULL
    *tmpprog = NULL; // NULL у нас всегда будет последним элементом tmpprog

    //текущее количество аргументов, считанных в tmpprog
    int args = 0;

    //текущее количество программ, считанных в массив
    int progs = 0;

    do
    {
        //если дошли до конца или до разделителя, и при этом что-то было считано
        if ((!line[i] && args) || (line[i] && !strcmp(line[i], separator)))
        {
            //выделяем в str место под указатель на программу
            str = (char ***) realloc(str, (++progs + 1)*sizeof(char **));
            str[progs]   = NULL;    //последний элемент - NULL
            str[progs-1] = tmpprog; //заменяем предыдущий последний NULL на tmpprog

            //обнуляем tmpprog
            tmpprog = (char **) malloc(sizeof(char *));
            *tmpprog = NULL;
            args = 0;
        }
        //иначе просто считываем в tmpprog новый аргумент из line
        else
        {
            //выделяем место под новый аргумент
            tmpprog = (char **) realloc(tmpprog, (++args + 1)*sizeof(char *));
            tmpprog[args]   = NULL;    //последний элемент - NULL
            tmpprog[args-1] = line[i]; //заменяем предыдущий последний NULL на line[i]
        }    
    } while (line[i++]);

    free(tmpprog);
    return str;
}

//проверяем программу на ввод или вывод. если есть, то возвращаем соответствующий файловы дескриптор. Иначе 0
int in_out(char **prog, char mode)
{
    int fd, elem = 0;

    while (prog[elem])
    {
        if ((mode == 'r') && !strcmp(prog[elem], "<"))
        {
            if (!prog[elem+1])
                fprintf(stderr, "syntax error near '<'\n");
            else if ((fd = open(prog[elem + 1], O_RDONLY)) == -1)
                perror("INPUT");
            else
                return fd;
        }
        else  if ((mode == 'w') && !strcmp(prog[elem], ">"))
        {
            if (!prog[elem+1])
                fprintf(stderr, "syntax error near '>'\n");
            else if ((fd = open(prog[elem + 1], O_TRUNC | O_WRONLY | O_CREAT, 0666)) == -1)
                perror("OUTPUT");
            else
                return fd;
        }
        else  if ((mode == 'w') && !strcmp(prog[elem], ">>"))
        {
            if (!prog[elem+1])
                fprintf(stderr, "syntax error near '>>'\n");
            else if ((fd = open(prog[elem+1], O_APPEND | O_WRONLY | O_CREAT, 0666)) == -1)
                perror("OUTPUT");
            else
                return fd;
        }
        elem++;
    }

    return 0;
}

//просматриваем все элементы prog. если видим "<" или ">" или ">>" то спокойно пропускаем его и следующий за ним элемент
//всё, что не пропустили, выводим
char **remove_inout(char **prog)
{
    int args;
    for (args = 0; prog[args]; args++);

    int i = 0;
    char **tmp = malloc(sizeof(char *));
    int size = 0;
    *tmp = NULL;

    while (i < args)
    {
        if (!strcmp(prog[i], "<") || !strcmp(prog[i], ">") || !strcmp(prog[i], ">>"))
            i++;
        else
        {  
            tmp = (char **) realloc(tmp, (++size + 1)*sizeof(char *));
            tmp[size] = NULL;
            tmp[size-1] = prog[i];
        }
        i++;
    }

    return tmp;
}

//реализация && и ||
int run_and_or(char **tmp)
{
    int i, j, status;
    char ***OR, ***AND;

    //разбиваем программу по признаку || и начинаем проходиться по получившимся элементам
    OR = massiv_prog(tmp, "||");
    for (i = 0; OR[i]; i++)
    {
        //каждый элемент OR разбиваем по признаку && и проходимся уже по этим элементам
        AND = massiv_prog(OR[i], "&&");
        for (j = 0; AND[j]; j++)
        {
            switch((fork()))
            {
                case -1: 
                    status = 1;
                    break;

                case  0:
                    signal(SIGINT, SIG_DFL);     
                    execvp(AND[j][0], AND[j]);
                    perror("EXEC");
                    free_line_pr(AND);
                    free_line_pr(OR);
                    free(tmp);
                    free_all();
                    exit(1);

                default: 
                    wait(&status);
                    status = WEXITSTATUS(status);
            }

            if (status) break; //если хоть один элемент завершился неудачно, то дальше элементы AND не смотрим
        }

        if (!AND[j]) //если дошли до конца AND, значит всё было завершено удачно и дальше элементы OR не смотрим
        {    
            free_line_pr(AND);
            break;
        }
        free_line_pr(AND);
    }
    if (OR) free_line_pr(OR);
    return status;
}


int run(char **prog, int *lpipe, int *rpipe)
{
    int fd;
    int status = 0;
    char **tmp;

    if (rpipe) pipe(rpipe);

    switch (fork())
    {
        case -1: 
            perror("fork");
            return 1;

        case  0:
            //---------ПЕРЕНАПРАВЛЕНИЯ-------------
            fd = in_out(prog, 'r');

            if (lpipe)
            {
                dup2(lpipe[0], 0);
                close(lpipe[0]);
                close(lpipe[1]);
            }

            if (fd)
            { 
                dup2(fd, 0);
                close(fd);
            }

            fd = in_out(prog, 'w');
                 
            if (rpipe)
            {
                if (!fd)
                    dup2(rpipe[1], 1);
                close(rpipe[0]);
                close(rpipe[1]);
            }

            if (fd) 
            {
                dup2(fd, 1);
                close(fd);
            }
            //--------КОНЕЦ ПЕРЕНАПРАВЛЕНИЙ----------
            
            //удаляем лишние аргументы перенаправления из задачи
            tmp = remove_inout(prog);

            if (*tmp) status = run_and_or(tmp);
            free(tmp);
            free_all();
            exit(status);

        default: 
            if (lpipe)
            {
                close(lpipe[0]);
                close(lpipe[1]);
            }
            return 0;
    }
}

//для нас любая задача - конвейер, даже если состоит из 1 элемента
int conveyor(char ***conv, int conv_elems)
{
    int i, fd1[2], fd2[2], status, error = 0;

    if (!conv_elems)
        return 1;

    if (conv_elems == 1) 
    {
        run(conv[0], NULL, NULL);
    }
    else
    {
        run(conv[0], NULL, fd2);

        for (i = 1; i < conv_elems-1; ++i)
        {
            fd1[0] = fd2[0];
            fd1[1] = fd2[1];

            if (run(conv[i], fd1, fd2) != 0)
                break;
        }  

        run(conv[i], fd2, NULL);
        
    }
    while(wait(&status) != -1)
        if (WEXITSTATUS(status)) error = 1;

    return error; 
}


//здесь мы создаём отдельный процесс для нашей задачи и соответственно выполняем её
void run_job(struct job cur_job)
{
    int conv_er;

    switch ((sigpid = fork()))
    {
        case -1: 
            perror("fork");
            free_all(); 
            exit(-1);

        case 0: 
            if (cur_job.background)
                printf("Process with ID {%d} started\n",getpid());

            conv_er = conveyor(cur_job.conv, cur_job.number_of_programs);
            free_all();
            exit(conv_er);

        default: 
            if (!cur_job.background)
            {
                //signal(SIGTSTP,handler);
                wait(&program.prog_status);
            }
    }
}

//создаём элементы shell_jobs.conv
void make_convs()
{
    int i, elems;

    for (i = 0; i < jobs_count; i++){
        shell_jobs[i].conv = massiv_prog(shell_jobs[i].program, "|");
        for(elems = 0; shell_jobs[i].conv[elems]; elems++);
        shell_jobs[i].number_of_programs = elems;
    }

}

//создаём массив shell_jobs
void massiv_jobs()
{
    int i = 0;

    //это наш массив
    shell_jobs = NULL;
    jobs_count = 0;

    char **tmpprog = (char **) malloc(sizeof(char *));
    *tmpprog = NULL; 

    int args = 0;

    do
    {
        if ((!line[i] && args) || (line[i] && !strcmp(line[i], ";")))
        {
            shell_jobs = (struct job *) realloc(shell_jobs, (++jobs_count)*sizeof(struct job));
            shell_jobs[jobs_count-1].background = 0;
            shell_jobs[jobs_count-1].program = tmpprog;

            tmpprog = (char **) malloc(sizeof(char *));
            *tmpprog = NULL;
            args = 0;
        }
        else if (line[i] && !strcmp(line[i], "&"))
        {
            shell_jobs = (struct job *) realloc(shell_jobs, (++jobs_count)*sizeof(Job));
            shell_jobs[jobs_count-1].background = 1;
            shell_jobs[jobs_count-1].program = tmpprog;

            tmpprog = (char **) malloc(sizeof(char *));
            *tmpprog = NULL;
            args = 0;
        }
        else
        {
            tmpprog = (char **) realloc(tmpprog, (++args + 1)*sizeof(char *));
            tmpprog[args]   = NULL;   
            tmpprog[args-1] = line[i]; 
        }    
    } while (line[i++]);

    free(tmpprog);
}

int commands()
{
    int i;
    pid_t pid;
    int status;
    
    printf("shell:~$ ");

    while ((pid = waitpid(0,&status,WNOHANG)) > 0)
        printf("Process with ID {%d} stopped with status %d\n",pid,WEXITSTATUS(status));
    
    line = command_string();

    if (!line) 
        return 0;

    //делим нашу строку по признакам & и ; на отдельные задачи(jobs)
    //эти задачи будут храниться в массиве структур struct job * shell_jobs
    //
    massiv_jobs();
    make_convs();

    //проходимся по всем задачам и поочерёдно их выполняем
    for (i = 0; i < jobs_count; i++)
    {

        if (shell_jobs[i].program[0] && !strcmp(shell_jobs[i].program[0], "cd"))
        {
            if (chdir(shell_jobs[i].program[1]))
                printf("shell: cd: No such file or directory\n");
            continue;
        }

        if (shell_jobs[i].program[0] && !strcmp(shell_jobs[i].program[0], "exit"))
        {
            free_all();
            exit(0);
        }

        run_job(shell_jobs[i]);

    }

    free_all();
    return 0;
}

int main(int argc, char **argv)
{
    program.number_of_arguments = argc;
    program.prog_status = 0;
    program.arguments = argv;
    program.pid = getpid();
    program.uid = getuid();

    signal(SIGINT,SIG_IGN);    

    while (!commands());

    return 0;
}
