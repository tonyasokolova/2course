#include <stdio.h>
#include <stdlib.h>

 
typedef struct node_t
{
    int elem;
    struct node_t *next;
}   TList;
 
/*unsigned int list_len = 0;
*/

/*
 * Добавление звена в конец
 */
int pushl(TList** list, int elem)
{
    	TList *node = NULL;

	node = (TList*)malloc(sizeof(TList));

    	node->next = NULL;
    	node->elem = elem;

    	if (*list == NULL)
    	{
        	*list = node;
    	}
	else
	{
        	TList* nov = *list;

        	while (nov->next!=NULL)
 	        	nov = nov->next;
        	
		nov->next = node;
    	}
    
	return -1;
}


/*
 * Удалить последнее звено из списка
 */
int popl(TList** list)
{
	int elem = 0;
 
	if (*list != NULL)
	{
		TList* node = *list;
		*list = (*list)->next;
		elem = node->elem;

		free(node);
	}
 
	return elem;
}


/*
 * Удаление списка
 */
void clearl(TList** list)
{
	while (*list != NULL)
	{
		popl(list);
	}
}


/*
 * Вывод списка
 */
void printl(TList* list)
{
	for (; list != NULL; list = list->next)
	{
		printf("%d ", list->elem);
	}
	
	printf("\n");
}


/*
 * Ввод списка
 */
TList* scanl(size_t count)
{
	TList* list = NULL;
	int c = 0;
 
	while (count--)
	{
		scanf("%d",&c); 
		pushl(&list, c );
	}
 
	return list;
}


/*
 * Добавление нового звена в позицию n 
 */
TList* insertl(TList* list, int list_len, int pos, int elem)
{
	int i;
	TList* node = list;
	TList* nov = (TList*) malloc(sizeof(TList));	

	if (list == NULL) return list;
	if (list_len < pos) return list;

	for (i = 1; i < pos; i++)
	{
		node = node->next;
	}
    
    if (pos)
    {
        nov->elem = elem;
        nov->next = node->next;
        node->next = nov;
    }

	if (pos == 0)
	{
		nov = (TList*) malloc(sizeof(TList));
        nov->elem = elem;
		nov->next = list;
		list = nov;
	}


	return list;
}


/*
 * Удаление звена из списка по номеру
 */
int ldelete(TList** list, int n)
{
        TList *node = *list;
        TList *nov = NULL;
        int i = 0;

        if (node == NULL)
        {
                return 2;
        }

        if (n == 0)
        {
                nov = (*list)->next;
                free(*list);
                *list = nov;

                return 0;
        }

        while (1)
        {
                if (node->next == NULL)
                {
                        return 2;
                }

                if (i == n-1)
                {
                        break;
                }

                node = node->next;
                i++;
        }

        nov = node->next->next;
        free(node->next);
        node->next = nov;

        return 0;
}


/*
 * Поиск звена, у которого поле равно key
 */
int searchl(TList* list, int key)
{
	int i = 0;
        TList* node = list;

        if (node == NULL)
                return -1;

        while ((node != NULL) && (node->elem != key))
        {
                i++;
                node = node->next;
        }

        if (node == NULL)
        {
                return -1;
        }
        else
        {
                return i;
        }
}
