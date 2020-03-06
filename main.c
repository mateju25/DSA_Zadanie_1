#include <stdio.h>
#include <string.h>
#include <math.h>
#define TYPE *(memory-2)
#define NEXT_P(p) ((p) + TYPE)
#define PREVIOUS_P(p) ((p) + 2*TYPE)
#define CURR_FOOTER(p, size) (p + abs(size) + TYPE)
#define NEXT_BLOCK(p) ((p) + 2*TYPE + abs(readFromArr(p)))
#define PREVIOUS_BLOCK(p) ((p) - abs(readFromArr(p-TYPE)) - 2* TYPE)
#define NEW_SIZE(p, size) (readFromArr(p) - 2* TYPE - size)



//smernik na zaciatok vyhradenej pamate
char* memory = NULL;

void writeToArr(int paOffset, int paVal)
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
        writeToArr(NEXT_P(readFromArr(PREVIOUS_P(act))), readFromArr(NEXT_P(act)));

    if (readFromArr(NEXT_P(act)) != -1) {
        writeToArr(PREVIOUS_P(readFromArr(NEXT_P(act))), readFromArr(PREVIOUS_P(act)));
    }
    memset(memory + act + TYPE, -1, abs(readFromArr(act)));
}
void insertBlock(int act)
{
    int rankOfList = numOfBlocks(readFromArr(act));
    writeToArr(NEXT_P(act), readFromArr(rankOfList* TYPE));
    writeToArr(rankOfList* TYPE, act);
    if (readFromArr(NEXT_P(act)) != -1)
    {
        writeToArr(PREVIOUS_P(readFromArr(NEXT_P(act))), act);
    }
    writeToArr(PREVIOUS_P(act), rankOfList*TYPE);
}
int mergeBlocks(int first, int second)
{
    if ((readFromArr(NEXT_P(first)) != -1) || (readFromArr(PREVIOUS_P(first)) != -1))
        deleteBlock(first);
    writeToArr(first, readFromArr(first) + 2*TYPE + readFromArr(second));
    writeToArr(CURR_FOOTER(first, readFromArr(first)), readFromArr(first));
    //CURR_FOOTER(first, readFromArr(first));
    memset(memory + first + TYPE, -1, abs(readFromArr(first)));
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
int split(int act, unsigned int size)
{
    int new = act + TYPE + size + TYPE;
    writeToArr(new, NEW_SIZE(act, size));
    writeToArr(CURR_FOOTER(new, readFromArr(new)), readFromArr(new));
    //CURR_FOOTER(new, readFromArr(new));
    writeToArr(act, -size);
    writeToArr(CURR_FOOTER(act, size), readFromArr(new));
    //CURR_FOOTER(act, size);
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
        return (void*)(memory + split(act, size) + TYPE);
    } else
    {
        writeToArr(act, -readFromArr(act));
        writeToArr(CURR_FOOTER(act, size), readFromArr(act));
        //CURR_FOOTER(act, size);
        deleteBlock(act);
        return (void*)(memory + act + TYPE);
    }
}
int memory_check(void *ptr)
{
    if (ptr == NULL) return 0;
    int act = ((char*)ptr - TYPE) - memory;
    if (readFromArr(act) == readFromArr(CURR_FOOTER(act, readFromArr(act)))) return 1;
    return 0;
}
int memory_free(void *valid_ptr)
{
    int act = 0, next = 0;
    if (!(memory_check(valid_ptr))) return 1;

    act = ((char*)valid_ptr - TYPE) - memory;
    writeToArr(act, -readFromArr(act));
    writeToArr(CURR_FOOTER(act, readFromArr(act)), -readFromArr(act));
    //CURR_FOOTER(act, readFromArr(act));
    next = NEXT_BLOCK(act);
    if ((readFromArr(act - TYPE) != 0)&&(act > *(memory-1)* TYPE))
    {
        if (readFromArr(PREVIOUS_BLOCK(act)) > 0)
            act = mergeBlocks(PREVIOUS_BLOCK(act), act);
    }

    if (readFromArr(next) != 0)
    {
        if (readFromArr(next) > 0){
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
    for (int i = 0; i < size; i++)
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
{
    char region[900];

    memory_init(region, 900);
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
    /*char region[100];
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
    /*char region[70];
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
    return 0;*/
}