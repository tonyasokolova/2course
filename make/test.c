#include <stdio.h>

#include "list.h"


int main()
{
        int c, key;
        unsigned int pos;
	unsigned int list_len;
        TList* list;

        printf("Введите длину списка: ");
        scanf("%u", &list_len);

        list  = scanl(list_len);
        printf("\n");

        printf("Добавление нового звена\n");
        printf(">Введите число:");
        scanf("%d",&c);
        printf(">Введите позицию:");
        scanf("%u", &pos);
        list = insertl(list,list_len,pos,c);
        printf("list:  ");
        printl(list);
        printf("\n");

        printf("Удаление звена из списка по номеру\n");
        printf(">Введите позицию:");
        scanf("%u", &pos);
        printf("\n");
        ldelete(&list, pos);
        printf("list:  ");
        printl(list);
        printf("\n");

        printf("Поиск звена, у которого поле равно key\n");
        printf(">Введите key: ");
        scanf("%d", &key);
        printf("Номер звена: %d\n", searchl(list, key));
        printf("\n");

        printf("list:  ");
        printl(list);
        printf("\n");

        clearl(&list);

        return 0;
}

