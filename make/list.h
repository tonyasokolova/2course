typedef struct node_t
{
    int elem;
    struct node_t *next;
}   TList;

extern int pushl(TList**, int );
extern int popl(TList** );
extern void clearl(TList** );
extern void printl(TList* );
extern TList* scanl(size_t );
extern TList* insertl(TList*, int, int, int );
extern int ldelete(TList**, int );
extern int searchl(TList*, int );

