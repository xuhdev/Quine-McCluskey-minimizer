//-------------------------------------------------------------------------------------------------------------
// Quine-McCluskey Algorithm
// =========================
//-------------------------------------------------------------------------------------------------------------
// English:
//-------------------------------------------------------------------------------------------------------------
// Description: Application to simplify boolean functions with Quine-McCluskey algorithm
// Date: 05/16/2012
// Author: Stefan Moebius (mail@stefanmoebius.de)
// Modified by Hong Xu (hong@topbug.net) to use as a library on 11/16/2014
// Licence: Can be used freely (Public Domain)
//-------------------------------------------------------------------------------------------------------------
// German:
//-------------------------------------------------------------------------------------------------------------
// Beschreibung: Programm zur Vereinfachung von Booleschen Funktionen mit hilfe des Quine�McCluskey Verfahrens.
// Datum: 16.05.2012
// Author: Stefan Moebius (mail@stefanmoebius.de)
// Modified by Hong Xu (hong@topbug.net) to use as a library
// Lizenz: darf frei verwendet werden (Public Domain)
//-------------------------------------------------------------------------------------------------------------
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

#define TRUE 1
#define FALSE 0

// Adjust MAXVARS and MAX if you have more input to handle.
#define MAXVARS 7
#define MAX 2048
// #define MAXVARS 10
// #define MAX 12288L

//Global fields: / Globale Felder:
static int minterm[MAX][MAX];
static int32_t mask[MAX][MAX];      // mask of minterm  /  Maske des Minterm
static int used[MAX][MAX];      // minterm used  /  Minterm wurde verwendet
static int result[MAX];     // results  /  Ergebnisse
static int primmask[MAX];       // mask for prime implicants  /  Maske f�r Primimplikant
static int prim[MAX];           // prime implicant  /  Primimplikant
static int wprim[MAX];          // essential prime implicant (TRUE/FALSE)  /  wesentlicher Primimplikant (TRUE/FALSE)
static int nwprim[MAX];     // needed not essential prime implicant  /  ben�tigter unwesentlicher Primimplikant

//Count all set bits of the integer number  /  Z�hlen der gesetzen Bits in einer Integerzahl
static int popCount(uint32_t x) { // Taken from book "Hackers Delight"  / Aus dem Buch "Hackers Delight"
    x = x - ((x >> 1) & 0x55555555);
    x = (x & 0x33333333) + ((x >> 2) & 0x33333333);
    x = (x + (x >> 4)) & 0x0F0F0F0F;
    x = x + (x >> 8);
    x = x + (x >> 16);
    return x & 0x0000003F;
}

//Calculate hamming weight/distance of two integer numbers  /  Berechnung der Hammingdistanz von 2 Integerzahlen
static int hammingWeight(uint32_t v1, uint32_t v2) {
    return popCount(v1 ^ v2);
}

//Output upper part of term in console  /  Oberer Teil des Terms in der Konsole ausgeben
static void upperTerm(int bitfield, int mask, int num) {
    if (mask) {
        int z;
        for ( z = 0; z < num; z++) {
            if (mask & (1 << z)) {
                if (bitfield & (1 << z))
                    printf("_");
                else
                    printf(" ");
            }
        }
    }
}

//Output lower part of term in console  /  Unterer Teil des Terms in der Konsole ausgeben
static void lowerTerm(int mask, int num) {
    if (mask) {
        int z;
        for (z = 0; z < num; z++) {
            if (mask & (1 << z)) {
                printf("%c", 'z' - (num - 1) + z);
            }
        }
    }
}

static void writeOutput(int bitfield, int mask, size_t num, int * out) {
    if (mask) {
        int z;
        for ( z = 0; z < num; z++) {
            if (mask & (1 << z)) {
                if (bitfield & (1 << z))
                    out[z] = 0;
                else
                    out[z] = 1;
            } else {
                out[z] = -1;
            }
        }
    }
}

//Output a term to console  /  Ausgabe eines Terms in der Konsole
static void outputTerm(int bitfield, int mask, int num) {
    upperTerm(bitfield, mask, num);
    if (mask) printf("\n");
    lowerTerm(mask, num);
}

//Determines whether "value" contains "part"  /  Bestimmt, ob "value" "part" beinhaltet
static int contains(int value, int32_t mask, int part, int partmask) {
    if ((value & partmask) == (part & partmask)) {
        if ((mask & partmask) ==  partmask)
            return TRUE;
    }
    return FALSE;
}

// free memory alocated by qmc_simplify
void qmc_free(void * p)
{
    free(p);
}

// @num count of variables
// @out_size will point to the number clauses
// *out is in the form of DNF. (*out) is an array which has @num * @out_size elements.
// Each consecutive @num elements is a clause.
int qmc_simplify(const bool * table, size_t num, int ** out, size_t* out_size) {
    // int num = 0; // Number of Variables  /  Anzahl Eing�nge
    int pos = 0;
    int cur = 0;
    int reduction = 0; //reduction step  / Reduktionsschritt
    int maderedction = FALSE;
    size_t prim_count = 0;
    int term = 0;
    int termmask = 0;
    int found = 0;
    size_t x = 0;
    size_t y = 0;
    size_t z = 0;
    int count = 0;
    int lastprim = 0;
    int res = 0; // actual result  /  Ist-Ausgabe

    // Fill all arrays with default values / Alle Arrays mit Standardwert auff�llen
    for (x = 0; x < MAX; x++) {
        primmask[x] = 0;
        prim[x] = 0;
        wprim[x] = FALSE;
        nwprim[x] = FALSE;
        result[x] = FALSE;
        nwprim[x] = TRUE; //unwesentliche Primimplikaten als ben�tigt markieren
        for (y = 0; y < MAX; y++) {
            mask[x][y] = 0;
            minterm[x][y] = 0;
            used[x][y] = FALSE;
        }
    }

    // printf("Enter number of variables: ");
    // scanf(" %d", &num);
    if (num > MAXVARS) {
        // fprintf(stderr, "ERROR: Number of variables too big!\n");
        return 1;
    }
    if (num < 1) {
        // fprintf(stderr, "ERROR: Number of variables must be at least 1!\n");
        return 2;
    }

    pos = (1 << num);  // 2 ^ num
    // printf("Please enter desired results: ( 0 or 1)\n\n");

    cur = 0;
    for ( x=0; x < pos; x++) {
        // int value = 0;
        // outputTerm(x, pos - 1, num);
        // printf(" = ");
        // scanf(" %d", &value);
        if (table[~x & (pos - 1)]) {
            mask[cur][0] = ((1 << num)- 1);
            minterm[cur][0] = x;
            cur++;
            result[x] = 1;
        }
        // printf("\n");
    }

    for (reduction = 0; reduction < MAX; reduction++) {
        cur = 0;
        maderedction = FALSE;
        for (y=0; y < MAX; y++) {
            for (x=0; x < MAX; x++) {
                if ((mask[x][reduction]) && (mask[y][reduction])) {
                    if (popCount(mask[x][reduction]) > 1) { // Do not allow complete removal (problem if all terms are 1)  /  Komplette aufhebung nicht zulassen (sonst problem, wenn alle Terme = 1)
                        if ((hammingWeight(minterm[x][reduction] & mask[x][reduction], minterm[y][reduction] & mask[y][reduction]) == 1) && (mask[x][reduction] == mask[y][reduction])) { // Simplification only possible if 1 bit differs  /  Vereinfachung nur m�glich, wenn 1 anderst ist
                            term = minterm[x][reduction]; // could be mintern x or y /  egal ob mintern x oder minterm y
                            //e.g.:
                            //1110
                            //1111
                            //Should result in mask of 1110  /  Soll Maske von 1110 ergeben
                            termmask = mask[x][reduction]  ^ (minterm[x][reduction] ^ minterm[y][reduction]);
                            term  &= termmask;

                            found = FALSE;
                            for ( z=0; z<cur; z++) {
                                if ((minterm[z][reduction+1] == term) && (mask[z][reduction+1] == termmask) ) {
                                    found = TRUE;
                                }
                            }

                            if (found == FALSE) {
                                minterm[cur][reduction+1] = term;
                                mask[cur][reduction+1] = termmask;
                                cur++;
                            }
                            used[x][reduction] = TRUE;
                            used[y][reduction] = TRUE;
                            maderedction = TRUE;
                        }
                    }
                }
            }
        }
        if (maderedction == FALSE)
            break; //exit loop early (speed optimisation)  /  Vorzeitig abbrechen (Geschwindigkeitsoptimierung)
    }

    prim_count = 0;
    //printf("\nprime implicants:\n");
    for ( reduction = 0 ; reduction < MAX; reduction++) {
        for ( x=0 ;x < MAX; x++) {
            //Determine all not used minterms  /  Alle nicht verwendeten Minterme bestimmen
            if ((used[x][reduction] == FALSE) && (mask[x][reduction]) ) {
                //Check if the same prime implicant is already in the list  /  �berpr�fen, ob gleicher Primimplikant bereits in der Liste
                found = FALSE;
                for ( z=0; z < prim_count; z++) {
                    if (((prim[z] & primmask[z]) == (minterm[x][reduction] & mask[x][reduction])) &&  (primmask[z] == mask[x][reduction]) )
                        found = TRUE;
                }
                if (found == FALSE) {
                    //outputTerm(minterm[x][reduction], mask[x][reduction], num);
                    //printf("\n");
                    primmask[prim_count] = mask[x][reduction];
                    prim[prim_count] = minterm[x][reduction];
                    prim_count++;
                }
            }
        }
    }

    //find essential and not essential prime implicants  /  wesentliche und unwesentliche Primimplikanten finden
    //all alle prime implicants are set to "not essential" so far  /  Primimplikanten sind bisher auf "nicht wesentlich" gesetzt
    for (y=0; y < pos; y++) { //for all minterms  /  alle Minterme durchgehen
        count = 0;
        lastprim = 0;
        if (mask[y][0]) {
            for (x=0; x < prim_count; x++ ) { //for all prime implicants  /  alle Primimplikanten durchgehen
                if (primmask[x]) {
                    // Check if the minterm contains prime implicant  /  the �berpr�fen, ob der Minterm den Primimplikanten beinhaltet
                    if (contains(minterm[y][0], mask[y][0], prim[x], primmask[x])) {
                        count++;
                        lastprim = x;
                    }
                }
            }
            // If count = 1 then it is a essential prime implicant /  Wenn Anzahl = 1, dann wesentlicher Primimplikant
            if (count == 1) {
                wprim[lastprim] = TRUE;
            }
        }
    }

    // successively testing if it is possible to remove prime implicants from the rest matrix  /  Nacheinander testen, ob es m�gich ist, Primimplikaten der Restmatrix zu entfernen
    for ( z=0; z < prim_count; z++) {
        if (primmask[z] ) {
            if (wprim[z] == FALSE) { // && (rwprim[z] == TRUE))
                nwprim[z] = FALSE; // mark as "not essential" /  als "nicht ben�tigt" markiert
                for ( y=0; y < pos; y++) { // for all possibilities  /  alle M�glichkeiten durchgehen
                    res = 0;
                    for ( x=0; x < prim_count; x++) {
                        if ( (wprim[x] == TRUE) || (nwprim[x] == TRUE)) {  //essential prime implicant or marked as required  /  wesentlicher Primimplikant oder als ben�tigt markiert
                            if ((y & primmask[x]) == (prim[x] & primmask[x])) { //All bits must be 1  /  Es m�ssen alle Bits auf einmal auf 1 sein (da And-Verkn�pfung)
                                res = 1;
                                break;
                            }
                        }
                    }
                    //printf(" %d\t%d\n", result, result[y]);
                    if (res == result[y]) {  // compare calculated result with input value /  Berechnetes ergebnis mit sollwert vergleichen
                        //printf("not needed\n"); //prime implicant not required  /  Primimplikant wird nicht ben�tigt
                    }
                    else {
                        //printf("needed\n");
                        nwprim[z] = TRUE; //prime implicant required  /  Primimplikant wird doch ben�tigt
                    }
                }
            }
        }
    }
    // printf("\nResult:\n\n");

    *out = (int *) calloc(sizeof(int), prim_count * num);

    // Output of essential and required prime implicants / Ausgabe der wesentlichen und restlichen ben�tigten Primimplikanten:
    count = 0;
    for ( x = 0 ; x < prim_count; x++) {
        if (wprim[x] == TRUE) {
            // if (count > 0) printf("   ");
            writeOutput(prim[x], primmask[x], num, (*out) + x * num);
            count++;
        }
        else if ((wprim[x] == FALSE) && (nwprim[x] == TRUE)) {
            // if (count > 0) printf("   ");
            writeOutput(prim[x], primmask[x], num, (*out) + x * num);
            count++;
        }
    }

    *out_size = prim_count;

#if 0
    // Output of essential and required prime implicants / Ausgabe der wesentlichen und restlichen ben�tigten Primimplikanten:
    count = 0;
    for ( x = 0 ; x < prim_count; x++) {
        if (wprim[x] == TRUE) {
            // if (count > 0) printf("   ");
            upperTerm(prim[x], primmask[x], num);
            count++;
        }
        else if ((wprim[x] == FALSE) && (nwprim[x] == TRUE)) {
            // if (count > 0) printf("   ");
            upperTerm(prim[x], primmask[x], num);
            count++;
        }
    }
    // printf("\n");
    count = 0;
    for ( x = 0 ; x < prim_count; x++) {
        if (wprim[x] == TRUE) {
            // if (count > 0) printf(" + ");
            lowerTerm(primmask[x], num);
            count++;
        }
        else if ((wprim[x] == FALSE) && (nwprim[x] == TRUE)) {
            // if (count > 0) printf(" + ");
            lowerTerm(primmask[x], num);
            count++;
        }
    }
    printf("\n");
#endif
    return 0;
}
