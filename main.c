#include <stdio.h>
#include <string.h>
#include <math.h>
#define HEAP_SIZE 901
#define NEXT_POINTER(p) (((int*)((p) + 1)))
#define PREVIOUS_POINTER(p) (((int*)((p) + 2)))
#define CURR_FOOTER(p) ((int*)((char*)(p) + sizeof(int) + abs(*((int*)(p)))))
#define POINTER_BASEDON_POINTER(p) ((int*)(memory + abs(*((int*)(p)))))
#define OFFSET(p) ((int)((char*)(p) - memory));
#define NEXT_BLOCK(p) ((int*)((char*)(p) + sizeof(int) + *((int*)(p)) + sizeof(int)))
#define PREVIOUS_BLOCK(p) ((int*)((char*)(p) - (int)abs(*((int*)((char*)(p) - sizeof(int)))) - sizeof(int) - sizeof(int)))
#define NEW_SIZE(p, size) ((int)(*((int*)(p)) - 2* sizeof(int*) - size))

//smernik na zaciatok vyhradenej pamate
char* memory = NULL;

int numOfBlocks(int paSize)
{
    if (((int)(floor(log2 (paSize)))-3) < 0)
        return 0;
    else
        return ((int)(round(log2 (paSize)))-3);
}
void deleteBlock(int* act)
{
    if ((char*)POINTER_BASEDON_POINTER(PREVIOUS_POINTER(act)) < memory + *(memory - 1)*sizeof(int))
        *(POINTER_BASEDON_POINTER(PREVIOUS_POINTER(act))) = *(NEXT_POINTER(act));
    else
        *(NEXT_POINTER(POINTER_BASEDON_POINTER(PREVIOUS_POINTER(act)))) = *(NEXT_POINTER(act));

    if (*(NEXT_POINTER(act)) != -1)
    {
        *(PREVIOUS_POINTER(POINTER_BASEDON_POINTER(NEXT_POINTER(act)))) = *(PREVIOUS_POINTER(act));
    }
    memset(act+1, -1,abs(*(act)));
}
void insertBlock(int* act)
{
    int rankOfList = numOfBlocks(*act);
    *(NEXT_POINTER(act)) = *((int*)(memory + rankOfList*sizeof(int)));
    *((int*)(memory + rankOfList*sizeof(int))) = OFFSET(act);
    if (*(NEXT_POINTER(act)) != -1)
    {
        *(PREVIOUS_POINTER(POINTER_BASEDON_POINTER(NEXT_POINTER(act)))) = OFFSET(act);
    }
    *(PREVIOUS_POINTER(act)) = OFFSET(((int*)(memory + rankOfList*sizeof(act))));
}
int* mergeBlocks(int* first, int* second)
{
    //if (*((int*)NEXT_POINTER(first)) != -1)
    deleteBlock(first);
    *(first) += 2*sizeof(int) + *second;
    *(CURR_FOOTER(first)) = *first;
    memset(first+1, -1, *first);
    return first;
}
int* bestFit(int* act, int size)
{

    int* best = act;
    while (*(NEXT_POINTER(act)) != -1)
    {
        if (*act == size) return act;
        if (*act- size < *best - size) best = act;
        act = (int*)(POINTER_BASEDON_POINTER(NEXT_POINTER(act)));
    }
    return best;
}
void *split(int* act, unsigned int size)
{
    int *new = (int*)((char*)(act) + sizeof(int) + size + sizeof(int));
    *new = NEW_SIZE(act, size);
    *(CURR_FOOTER(new)) = *new;
    *act= -size;
    *(CURR_FOOTER(act)) = -size;
    deleteBlock(act);

    insertBlock(new);

    return act;
}

void *memory_alloc(unsigned int size)
{
    int* act = NULL, *new = NULL;
    int rankOfList = numOfBlocks(size);
    while (*((int*)(memory + rankOfList*sizeof(int))) == -1)
        rankOfList++;
    if (rankOfList >= *(memory - 1)) return NULL;
    act = (int*)(POINTER_BASEDON_POINTER(memory + rankOfList*sizeof(int)));
    act = bestFit(act, size);


    if (*act >= size + 4*sizeof(int))
    {
        return (char*)split(act, size)+sizeof(int);
    } else
    {

        *act = -*act;
        *(CURR_FOOTER(act)) = *act;
        deleteBlock(act);
        return act+1;
    }

}
int memory_check(void *ptr)
{
    int* act = (int*)ptr - 1;
    if (*act == *(CURR_FOOTER(act))) return 1;
    return 0;
}
int memory_free(void *valid_ptr)
{
    int* act = NULL, *next = NULL;
    if (!(memory_check(valid_ptr))) return 1;

    act = (int*)valid_ptr- 1;
    *act = -*act;
    *(CURR_FOOTER(act)) = *act;
    next = NEXT_BLOCK(act);
    if ((*((int*)(act - 1)) != 0)&&(act > (int*)(memory + *(memory-1)*sizeof(int))))
    {
        if (*(PREVIOUS_BLOCK(act)) > 0)
            act = mergeBlocks(PREVIOUS_BLOCK(act), act);
    }

    if (*((int*)next) != 0)
    {
        if (*next > 0){
            deleteBlock(next);
            act = mergeBlocks(act, next);
        }

    }
    insertBlock(act);//nefunguje spravne insert pravdepodobne

    return 0;

}
void memory_init(void *ptr, unsigned int size)
{
    char numOfLists = numOfBlocks(size)+1;
    int* act = NULL;
    for (int i = 0; i < HEAP_SIZE; i++)
    {
        *((char*)ptr+ i) = -1;
    }
    //vyrobenie hlavnej hlavicky
    *((char*)ptr) = numOfLists;
    memory = (char*)ptr+1;
    for (int i = 0; i < numOfLists; i++)
    {
        *((int*)(memory + i*sizeof(int))) = -1;
    }
    *((int*)(memory + (numOfLists-1)*sizeof(int))) = (numOfLists)*sizeof(int);
    *((int*)(memory + size - 1 - sizeof(int))) = 0;

    //vytvorenie prveho bloku
    act = (int*)(memory + numOfLists*sizeof(int));
    *act = size - 1 - 3*(sizeof(int)) - numOfLists*(sizeof(int));
    *(CURR_FOOTER(act)) = *act;
    *(NEXT_POINTER(act)) = -1;
    *(PREVIOUS_POINTER(act)) = (numOfLists-1)*sizeof(int);
}

int main()
{
    char region[HEAP_SIZE];

    memory_init(region, HEAP_SIZE);
    char* list[30];

    for (int i = 0; i < 30; i++) {
        list[i] = (char*)memory_alloc(8);//tu
        if (list[i] != 0) {
            printf("ALOKOVANE %d\n", i);
        }
        else  printf("NEALOKOVANE %d\n", i);
    }

    for (int i = 0; i < 30; i++) {
        if (list[i] != 0) {
            if (memory_free(list[i]) == 1) {
                printf("PROBLEM\n");
            }
        }
    }

    char* pointer = (char*)memory_alloc(800);
    if (pointer != 0) {
        printf("HURA\n");
    }
    else {
        printf("SHIT\n");
    }

    return 0;
}