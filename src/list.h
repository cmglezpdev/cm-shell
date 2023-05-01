#ifndef CMSH_LIST_H
#define CMSH_LIST_H

typedef struct List {
    int len;
    unsigned capacity;
    int *array;
} List;

struct List *createList();

struct List *cloneList(struct List *original);

void append(struct List *list, int value);

void clear(struct List *list);

int getValueAtIndex(struct List *list, int index);

int setValueAtIndex(struct List *list, int index, int value);

int removeAtIndex(struct List *list, int index);

int addAtIndex(struct List *list, int index, int value);

#endif //CMSH_LIST_H
