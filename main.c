#include <stdio.h>
#include <string.h>
#include <math.h>
#define HEAP_SIZE 70
#define TYPE *(memory-2)
#define NEXT_P(p) ((p) + TYPE)
#define PREVIOUS_P(p) ((p) + 2*TYPE)
#define CURR_FOOTER(p, size) writeToArr((p + abs(size) + TYPE), readFromArr(p))
#define NEXT_BLOCK(p) ((int*)((char*)(p) + sizeof(int) + *((int*)(p)) + sizeof(int)))
#define PREVIOUS_BLOCK(p) ((int*)((char*)(p) - (int)abs(*((int*)((char*)(p) - sizeof(int)))) - sizeof(int) - sizeof(int)))
#define NEW_SIZE(p, size) (readFromArr(p) - 2* TYPE - size)



//smernik na zaciatok vyhradenej pamate
char* memory = NULL;

{
    if (*(memory - 2) == 1)
    {
        *(memory + paOffset) = (char)paVal;
    }
    else
    {
        if (*(memory - 2) == 2)
        {
            *((short*)(memory + paOffset)) = (short)paVal;
        }
        else
        {
            *((int*)(memory + paOffset)) = (int)paVal;
        }
    }
}
int readFromArr(int paOffset)
{
    if (*(memory - 2) == 1)
    {
        return *((char*)(memory + paOffset));
    }
    else
    {
        if (*(memory - 2) == 2)
        {
            return *((short*)(memory + paOffset));
        }
        else
        {
            return *((char*)(memory + paOffset));
        }
    }
}

int numOfBlocks(int paSize)
{
    if (((int)(floor(log2 (paSize)))-3) < 0)
        return 0;
    else
        return ((int)(round(log2 (paSize)))-3);
}
void deleteBlock(int act)
{
    if (readFromArr(PREVIOUS_P(act)) < *(memory - 1) * TYPE)
        writeToArr(readFromArr(PREVIOUS_P(act)), readFromArr(NEXT_P(act)));
    else
        writeToArr(NEXT_P(readFromArr(PREVIOUS_P(act))), NEXT_P(act));

    if (readFromArr(NEXT_P(act)) != 1)
        writeToArr(PREVIOUS_P(readFromArr(NEXT_P(act))), readFromArr(PREVIOUS_P(act)));
    memset(memory + act + TYPE, -1, readFromArr(act));
}
void insertBlock(int act)
{
    int rankOfList = numOfBlocks(readFromArr(act));
    writeToArr(NEXT_P(act), rankOfList* TYPE);
    writeToArr(rankOfList* TYPE, act);
    if (readFromArr(NEXT_P(act)) != -1)
    {
        writeToArr(PREVIOUS_P(readFromArr(NEXT_P(act))), act);
    }
    writeToArr(PREVIOUS_P(act), readFromArr(rankOfList*TYPE));
}
int* mergeBlocks(int* first, int* second)
{
    //if (*((int*)NEXT_POINTER(first)) != -1)
    deleteBlock(first);
    *(first) += 2*sizeof(int) + *second;
    /**(CURR_FOOTER(first)) = *first;*/
    memset(first+1, -1, *first);
    return first;
}
int bestFit(int act, int size)
{

    int best = act;
    while (readFromArr(NEXT_P(act)) != -1)
    {
        if (readFromArr(act) == size) return act;
        if (readFromArr(act) - size < readFromArr(best) - size) best = act;
        act = readFromArr(NEXT_P(act));
    }
    return best;
}
void *split(int act, unsigned int size)
{
    int new = act + TYPE + size + TYPE;
    writeToArr(new, NEW_SIZE(act, size));
    CURR_FOOTER(new, readFromArr(new));
    writeToArr(act, -size);
    CURR_FOOTER(act, size);
    deleteBlock(act);

    insertBlock(new);

    return act;
}


void *memory_alloc(unsigned int size)
{
    int act = -1;
    if (size < 8) size = 8;
    int rankOfList = numOfBlocks(size);
    while (readFromArr(rankOfList* TYPE) == -1)
        rankOfList++;
    if (rankOfList >= *(memory - 1)) return NULL;
    act = readFromArr(rankOfList* TYPE);
    act = bestFit(act, size);


    if (readFromArr(act) >= size + 4 * TYPE)
    {
        return (char*)split(act, size)+sizeof(int);
    } else
    {
        writeToArr(act, -readFromArr(act));
        CURR_FOOTER(act, size);
        //deleteBlock(act);
        return act+1;
    }
}
int memory_check(void *ptr)
{
    if (ptr == NULL) return 0;
    int* act = (int*)ptr - 1;
   /* if (*act == *(CURR_FOOTER(act))) return 1;*/
    return 0;
}
int memory_free(void *valid_ptr)
{
    int* act = NULL, *next = NULL;
    if (!(memory_check(valid_ptr))) return 1;

    act = (int*)valid_ptr- 1;
    *act = -*act;
    /**(CURR_FOOTER(act)) = *act;*/
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
    int act = NULL;
    for (int i = 0; i < HEAP_SIZE; i++)
    {
        *((char*)ptr+ i) = -1;
    }
    if (size < 128)
        *((char*)ptr) = sizeof(char);
    else if (size < 32767)
            *((char*)ptr) = sizeof(short);
        else
            *((char*)ptr) = sizeof(int);
    *((char*)ptr+1) = numOfLists;
    memory = (int*)((char*)ptr+2);


    for (int i = 0; i < numOfLists; i++)
    {
        writeToArr(i * TYPE, -1);
    }
    writeToArr((numOfLists-1) * TYPE,  numOfLists * TYPE);
    writeToArr(size - 2 -TYPE,  0);

    //vytvorenie prveho bloku
    act = numOfLists * TYPE;
    writeToArr(act, size - 2 - 3* TYPE - numOfLists* TYPE);
    CURR_FOOTER(act, readFromArr(act));
    writeToArr(act + TYPE, -1);
    writeToArr(act + 2 * TYPE, (numOfLists-1)*TYPE);
}

int main()
{/*
    char region[HEAP_SIZE];

    memory_init(region, HEAP_SIZE);
    char* list[30];*/
/*
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
    }*//*
    char region[100];
    memory_init(region, 100);
    int a;
    char *pointer = (char *) memory_alloc(8);
    char *pointer2 = (char *) memory_alloc(16);
    char *pointer3 = (char *) memory_alloc(8);
    char *pointer4 = (char *) memory_alloc(8);
    char *pointer5 = (char *) memory_alloc(8);
    a=memory_free(pointer2);
    a=memory_free(pointer4);
    char *pointer6 = (char * ) memory_alloc(8);
    *pointer6 = 'a';*/
    char region[70];
    memory_init(region, 70);
    int a;
    char *pointer = (char *) memory_alloc(10);
    char *pointer2 = (char *) memory_alloc(10);
    char *pointer3 = (char *) memory_alloc(10);
    char *pointer4 = (char *) memory_alloc(10);
    char *pointer5 = (char *) memory_alloc(9);
    a=memory_free(pointer);
    a=memory_free(pointer5);
    a=memory_free(pointer2);
    a=memory_free(pointer4);
    a=memory_free(pointer3);
    return 0;
}