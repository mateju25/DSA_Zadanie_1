#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include<time.h>

#define TYPE *(memory-2)    //velkost modu (char, short alebo int)
#define NEXT_P(p) ((p) + TYPE)  //offset dalsieho volneho bloku
#define PREVIOUS_P(p) ((p) + 2*TYPE)    //offset predchadzajuceho volneho bloku
#define CURR_FOOTER(p, size) writeToArr((p + abs(size) + TYPE), readFromArr(p)) //nastavi patu podla toho aka je hlavicka
#define NEXT_BLOCK(p) ((p) + 2*TYPE + abs(readFromArr(p)))  //blok vpravo od daneho bloku
#define PREVIOUS_BLOCK(p) ((p) - abs(readFromArr(p-TYPE)) - 2* TYPE)   //blok vlavo od daneho bloku

//smernik na zaciatok vyhradenej pamate
char* memory = NULL;



//funkcia poskytuje zapis do pamate na zaklade modu(char, short a int)
void writeToArr(long paOffset, long paVal)
{
    switch (TYPE)
    {
        case 1:{*((char*)(memory + paOffset))= (char)paVal; break;}
        case 2:{*((short*)(memory + paOffset)) = (short)paVal; break;}
        case 4:{*((int*)(memory + paOffset)) = (int)paVal; break;}
        case 8:{*((long*)(memory + paOffset)) = (long)paVal; break;}
    }
}
//funkcia poskytuje citanie z pamate na zaklade modu(char, short a int)
long readFromArr(long paOffset)
{
    switch (TYPE)
    {
        case 1:{return *((char*)(memory + paOffset));}
        case 2:{return *((short*)(memory + paOffset));}
        case 4:{return *((int*)(memory + paOffset));}
        case 8:{return *((long*)(memory + paOffset));}
    }
}
//funkcia vrati cislo konkretneho segregovaneho zoznamu na zaklade velkosti
char blockNumber(long paSize)
{
    if (((char)(floor(log2 (paSize)))-3) < 0)
        return 0;
    else
        return ((char)(round(log2 (paSize)))-3);
}
//funkcia vymaze blok pamate z jeho priradeneho zoznamu
void deleteBlock(long act)
{
    if (readFromArr(PREVIOUS_P(act)) < *(memory - 1) * TYPE)
        writeToArr(readFromArr(PREVIOUS_P(act)), readFromArr(NEXT_P(act)));
    else
        writeToArr(NEXT_P(readFromArr(PREVIOUS_P(act))), readFromArr(NEXT_P(act)));

    if (readFromArr(NEXT_P(act)) != -1)
        writeToArr(PREVIOUS_P(readFromArr(NEXT_P(act))), readFromArr(PREVIOUS_P(act)));
    //vymazanie smernikov bloku
    writeToArr(NEXT_P(act), -1);
    writeToArr(PREVIOUS_P(act), -1);
}
//funkcia zaradi dany blok na zaciatok jeho priradeneho zoznamu
void insertBlock(long act)
{
    int rankOfList = blockNumber(readFromArr(act));
    writeToArr(NEXT_P(act), readFromArr(rankOfList* TYPE));
    writeToArr(rankOfList* TYPE, act);
    if (readFromArr(NEXT_P(act)) != -1)
    {
        writeToArr(PREVIOUS_P(readFromArr(NEXT_P(act))), act);
    }
    writeToArr(PREVIOUS_P(act), rankOfList*TYPE);
}
//funckia spoji dva bloky, ktore idu za sebou v pamati
long mergeBlocks(long first, long second)
{
    //ak este nebol vymazany zo zoznamu, tak ho vymaz
    if ((readFromArr(NEXT_P(first)) != -1) || (readFromArr(PREVIOUS_P(first)) != -1))
        deleteBlock(first);
    //spoj bloky
    writeToArr(first, readFromArr(first) + 2*TYPE + readFromArr(second));
    CURR_FOOTER(first, readFromArr(first));
    //vymaz obsah oboch blokov
    memset(memory + first + TYPE, -1, abs(readFromArr(first)));
    return first;
}
//funkcia prejde prideleny zoznam(na zaklade prveho volneho bloku v nom) a najde blok, ktory ma najefektivnejsiu velkost
long bestFit(long first, long size)
{
    long best = first, act = first;
    while (readFromArr(NEXT_P(act)) != -1)
    {
        if (readFromArr(act) == size) return act;
        if (readFromArr(act) - size < readFromArr(best) - size) best = act;
        //chod na dalsi blok v zozname
        act = readFromArr(NEXT_P(act));
    }
    return best;
}
//funkcia rozdeli konkretny blok na blok velkosti size a na blok zvysnej pamate
long split(long act, unsigned int size)
{
    long new = act + 2* TYPE + size;
    //nastav hlavicku a patu novemu bloku
    writeToArr(new, readFromArr(act) - 2* TYPE - size);
    CURR_FOOTER(new, readFromArr(new));
    //nastav hlavicku a patu staremu bloku
    writeToArr(act, -size);
    CURR_FOOTER(act, size);
    //vymaz stary z konkretneho zoznamu
    deleteBlock(act);
    //pridaj novy do spravneho zoznamu
    insertBlock(new);
    return act;
}

void *memory_alloc(unsigned int size)
{
    long act = -1;
    //ak si uzivatel vypyta menej ako 8
    if (size < 8) size = 8;
    printf("CHCEM %d    ", size);
    //zisti spravny zoznam
        int rankOfList = blockNumber(size);
        while (readFromArr(rankOfList* TYPE) == -1)
            rankOfList++;
        if (rankOfList >= *(memory - 1)) {printf("Nealokovane - nedostatok miesta %d\n", size); return NULL;}
        act = readFromArr(rankOfList* TYPE);
    //********************
    act = bestFit(act, size);

    //ak sa blok da rozdelit, resp. aby novy vzniknuty blok nemal menej ako 8 bajtov
    if (readFromArr(act) >= size + 2* TYPE + 16)
    {
        printf("Alokovane %d\n", size);
        return (void *) (memory + split(act, size) + TYPE);
    }
    else
    {
        //ak ma presnu velkost tak ho len prirad a nerozdeluj ho
        if (size > readFromArr(act)) {printf("Nealokovane - nedostatok miesta %d\n", size); return NULL;}
        printf("Alokovane %d *******************************************\n", readFromArr(act));
        writeToArr(act, -readFromArr(act));
        CURR_FOOTER(act, readFromArr(act));
        deleteBlock(act);
        return (void*)(memory + act + TYPE);
    }
}
int memory_check(void *ptr)
{
    if (ptr == NULL) return 0;
    int act = ((char*)ptr - TYPE) - memory;
    //porovna hlavu a patu, ak sa cisla rovnaju, je obrovska pravdepodobnost, ze ide prave o moj spracovavany blok + este nebol uvolneny
    if ((readFromArr(act) == readFromArr(act + TYPE + abs(readFromArr(act)))) && (readFromArr(act) > 0))  return 1;
    return 0;
}
int memory_free(void *valid_ptr)
{
    //nemusim testovat lebo v zadani bolo ze pride vzdy platny, ale inak keby trebalo, tak takto
    //if (!(memory_check(valid_ptr))) return 1;

    long act = ((char*)valid_ptr - TYPE) - memory;
    long next = NEXT_BLOCK(act);
    //priprav blok na uvolnenie
    writeToArr(act, -readFromArr(act));
    CURR_FOOTER(act, readFromArr(act));
    printf("Uvolnene %d    ", readFromArr(act));
    //ak je blok, PRED uvolnovanym blokom, volny, tak ich mergni
    if ((readFromArr(act - TYPE) != 0)&&(act > *(memory-1)* TYPE))
    {
        if (readFromArr(PREVIOUS_BLOCK(act)) > 0)
            act = mergeBlocks(PREVIOUS_BLOCK(act), act);
    }
    //ak je blok, ZA uvolnovanym blokom, volny, tak ich mergni
    if (readFromArr(next) != 0)
    {
        if (readFromArr(next) > 0){
            deleteBlock(next);
            act = mergeBlocks(act, next);
        }

    }
    //pridaj mergnuty blok
    insertBlock(act);
    printf("Spojene na %d\n", readFromArr(act));
    //*((char*)valid_ptr) = NULL;
    return 0;
}
void memory_init(void *ptr, unsigned int size)
{
    char numOfLists = blockNumber(size)+1;
    for (long i = 0; i < size; i++)
    {
        *((char*)ptr+ i) = -1;
    }
    //zisti s akym modom sa bude pracovat a zapis ho na prve miesto pamate
        if (size < 128)
            *((char*)ptr) = sizeof(char);
        else if (size < 32767)
                *((char*)ptr) = sizeof(short);
            else if(size < 2147483647)
                    *((char*)ptr) = sizeof(int);
                  else
                    *((char*)ptr) = sizeof(long);
    //***********************************
    //na druhe miesto pamate zapis pocet samostatnych zoznamov
    *((char*)ptr+1) = numOfLists;
    //globalny smernik bude ukazovat az na tretie miesto pamate
    memory = (char*)ptr+2;

    //resetuj zoznamy na "NULL", resp. -1
    for (int i = 0; i < numOfLists; i++)
    {
        writeToArr(i * TYPE, -1);
    }
    //nastav jeden zoznam aby ukazoval na prvy blok
    writeToArr((numOfLists-1) * TYPE,  numOfLists * TYPE);
    //na koniec pamate zapisem nulu, aby som vedel pri free, ze som na konci
    writeToArr(size - 2 -TYPE,  0);

    //vytvorenie prveho bloku
    //blok obsahuje velkost, offset na dalsi volny blok a offset na predchadzajuci volny blok
    long act = numOfLists * TYPE;
    writeToArr(act, size - 2 - 3* TYPE - numOfLists* TYPE);
    CURR_FOOTER(act, readFromArr(act));
    writeToArr(act + TYPE, -1);
    writeToArr(act + 2 * TYPE, (numOfLists-1)*TYPE);
}

void print(long size)
{
    long shift = 0;
    for (long i = 0; i < size - (blockNumber(size)+1)*TYPE - 2; i = i + 2*TYPE + abs(shift))
    {
        shift = readFromArr((blockNumber(size)+1)*TYPE + i);
        printf("%ld =>", shift);
    }
    printf("\n");
}

/*TEST1 - PAMAT 50 - ROVNAKE BLOKY 8 A ICH UVOLNENIE,
 *                   POTOM ALOKOVANIE 24, 8, 8, UVOLNENIE 24 A ALOKOVANIE ZASE 8*/
void test1(void)
{
    char region[50];
    memory_init(region, 50);

    char* list[5];
    int x;
    for (int i = 0; i < 5; i++) {
        list[i] = (char*)memory_alloc(8);//tu
    }
    for (int i = 0; i < 5; i++) {
        if (list[i] != NULL)
            x=memory_free(list[i]);
        print(50);
    }

    char *pointer = (char *) memory_alloc(24);
    char *pointer2 = (char *) memory_alloc(8);
    char *pointer3 = (char *) memory_alloc(8);
    char *pointer4 = (char *) memory_alloc(8);
    char *pointer5 = (char *) memory_alloc(8);
    print(50);
    x =memory_free(pointer);
    pointer = (char *) memory_alloc(8);
    char *pointer6 = (char *) memory_alloc(8);
    print(50);
}

/*TEST2 - PAMAT 100 - BLOKY NAHODNEJ VELKOSTI(8-24) A ICH UVOLNENIE,
 *                   POTOM ALOKOVANIE 80*/
void test2(void){
    char region[100];
    memory_init(region, 100);
    char* list[10];
    srand(time(0));
    int x ;
    for (int i = 0; i < 10; i++) {
        x = rand() % 16 + 8;
        list[i] = (char*)memory_alloc(x);
    }

    for (int i = 0; i < 10; i++) {
        if (list[i] != NULL)
            x = memory_free(list[i]);
    }

    char* pointer = (char*)memory_alloc(80);
}

/*TEST3 - PAMAT 20000 - BLOKY NAHODNEJ VELKOSTI(500-5000) A ICH UVOLNENIE,
 *                   POTOM ALOKOVANIE 15000*/
void test3(void)
{
    char region[20000];
    memory_init(region, 20000);
    char* list[10];
    srand(time(0));
    int x;
    for (int i = 0; i < 10; i++) {
        x = rand() % 5001 + 500;
        list[i] = (char*)memory_alloc(x);//tu
    }

    for (int i = 0; i < 10; i++) {
        if (list[i] != NULL)
            x = memory_free(list[i]);
    }

    char* pointer = (char*)memory_alloc(15000);
}

/*TEST4 - PAMAT 300_000 - BLOKY NAHODNEJ VELKOSTI(8-50000) A ICH UVOLNENIE,
 *                   POTOM ALOKOVANIE 150_000*/
void test4(void)
{
    char region[300000];
    memory_init(region, 300000);
    char* list[15];
    srand(time(0));
    int x ;
    for (int i = 0; i < 15; i++) {
        x = (rand() + rand()) % 50001 + 8;
        list[i] = (char*)memory_alloc(x);//tu
    }

    for (int i = 0; i < 15; i++) {
        if (list[i] != NULL)
            x = memory_free(list[i]);
    }

    char* pointer = (char*)memory_alloc(200000);
}

/*TEST5 - PAMAT 300_000 - NAHODNE PRIDELOVANIE BLOKOv (8-50000) A ICH NAHODNE UVLONOVANIE*/
void test5(void)
{
    char region[300000];
    memory_init(region, 300000);
    char* list[30];
    for (int i =0;i<30;i++)
    {
        list[i] = NULL;
    }
    srand(time(0));
    int x = (rand() + rand()) % 50001 + 8;
    int y, res;
    print(300000);
    for (int i = 0; i < 100; i++)
    {
        y = rand() % 2;
        if (y == 0)
        {
            y = (rand()) % 30;
            if (list[y] == NULL) {
                list[y] = (char *) memory_alloc(x);
                print(300000);
            }
        }
        else
        {
            y = (rand()) % 30;
            if (list[y] != NULL){
                res = memory_free(list[y]);
                list[y] = NULL;
                print(300000);
            }
        }
        x = (rand() + rand()) % 50001 + 8;
    }

}

int main()
{
    test5();
    return 0;
}