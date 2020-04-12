// Thanks to:
// https://github.com/xuhdev/Quine-McCluskey-minimizer

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
#include <iostream>
#include <sstream>
#include <vector>
using namespace std;
#include <stdio.h>
#include <math.h>

//全局依赖：异常
namespace exceptions {
    struct SyntaxError{string msg;};
    void syntax_error(const string& msg){
        printf("%s\n", msg.c_str());
        throw SyntaxError{msg};
    }
    void syntax_error(const string& expr, int position){
        printf("Syntax Error at col %d\n", position);
        printf("%s\n", expr.c_str());
        for(int i=0; i<position; ++i) {printf("_");}
        printf("^ Error Here");

        stringstream errstr;
        errstr << "Syntax Error at col " << position;
        throw SyntaxError{errstr.str()};
    }
    void syntax_error(const string& expr, int position, const string& errmsg){
        printf("%s\n", errmsg.c_str());
        printf("Syntax Error at col %d\n", position);
        printf("%s\n", expr.c_str());
        for(int i=0; i<position; ++i) {printf("_");}
        printf("^ Error Here");
        throw SyntaxError{errmsg};
    }

    struct StackError{string msg;};

    struct ValueError{string msg;};
    void value_error(const string& msg){
        printf("%s\n", msg.c_str());
        throw ValueError{msg};
    }
}
using namespace exceptions;

// Adjust MAXVARS and MAX if you have more input to handle.
// #define MAXVARS 7
// #define MAX 2048
// #define MAXVARS 10
// #define MAX 12288L

// //Global fields: / Globale Felder:
// static int minterm[MAX][MAX];
// static int mask[MAX][MAX];      // mask of minterm  /  Maske des Minterm
// static int used[MAX][MAX];      // minterm used  /  Minterm wurde verwendet
// static int result[MAX];     // results  /  Ergebnisse
// static int primmask[MAX];       // mask for prime implicants  /  Maske f�r Primimplikant
// static int prim[MAX];           // prime implicant  /  Primimplikant
// static bool wprim[MAX];          // essential prime implicant (true/false)  /  wesentlicher Primimplikant (true/false)
// static int nwprim[MAX];     // needed not essential prime implicant  /  ben�tigter unwesentlicher Primimplikant

class TruthtableParser{
public:
    int MAXVARS = 7;
    int MAX = 2048;

    int num_of_variables;
    int length_of_truthtable;
    vector<bool> truthtable;
    // result of simplifying the truthtable
    int prim_count;
    vector<int> out;
    bool all1=false, all0=false;

    vector<vector<int> > minterm;  // 中间项
    vector<vector<int> > mask;  // 中间项掩码
    vector<vector<int> > used;  // 使用过的中间项
    vector<int> result;
    vector<int> prim;  //质蕴含  // prime implicant
    vector<int> primmask;  //质蕴含掩码
    vector<bool> wprim;  //基本质蕴含
    vector<int> nwprim;  // 需要的基本质蕴含

    inline bool is_power_of_2(int input_num){
        if((input_num & (input_num-1)) == 0) {return true;}
        else {return false;}
    }

    TruthtableParser(const std::string& input_truthtable){
        length_of_truthtable = input_truthtable.size();
        if(length_of_truthtable==0){  //cin输入时应该不会触发这个
            value_error("No input acquired!");
        }

        if(not is_power_of_2(length_of_truthtable)){
            stringstream fmt;
            fmt << "Your input length of truthtable is " << length_of_truthtable << ", which is not a power of 2.\n";
            // value_error(fmt.str());
            printf("%s", fmt.str().c_str());
            printf("Ignored the error without warranty of correct result.\n");
        }
        truthtable.assign(length_of_truthtable,false);

        num_of_variables = int(log(length_of_truthtable)/log(2)+0.5);
        if(num_of_variables > MAXVARS){
            printf("Too many (%d) variables needed. Increase MAX and MAXVARS before calculating.\n", num_of_variables);
            printf("Currently MAXVARS == %d\n", MAXVARS);
        }

        /* the truth truthtable -- 3 inputs 1 output
         *
         * C  B  A  o
         * -------
         * 0  0  0  1
         * 0  0  1  1
         * 0  1  0  1
         * 0  1  1  1
         * 1  0  0  1
         * 1  0  1  1
         * 1  1  0  0
         * 1  1  1  0
         */

        // truthtable[0] = true;
        // truthtable[1] = true;
        // truthtable[2] = true;
        // truthtable[3] = true;
        // truthtable[4] = true;
        // truthtable[5] = true;
        // truthtable[6] = false;
        // truthtable[7] = false;

        int i, j;
        all1=true; all0=true;
        for(i=0; i<length_of_truthtable; ++i){
            switch(input_truthtable[i]){
                case '0': all1=false; truthtable[i]=false; continue;
                case '1': all0=false; truthtable[i]=true; continue;
                default: syntax_error(input_truthtable, i, "unexpected char");
            }
        }
    }

    //Count all set bits of the integer number  /  Z�hlen der gesetzen Bits in einer Integerzahl
    inline int popCount(unsigned int x) { // Taken from book "Hackers Delight"  / Aus dem Buch "Hackers Delight"
        x = x - ((x >> 1) & 0x5555555555555555);
        x = (x & 0x3333333333333333) + ((x >> 2) & 0x3333333333333333);
        x = (x + (x >> 4)) & 0x0F0F0F0F0F0F0F0F;
        x = x + (x >> 8);
        x = x + (x >> 16);
        return x & 0x0000003F;
    }

    //Calculate hamming weight/distance of two integer numbers  /  Berechnung der Hammingdistanz von 2 Integerzahlen
    inline int hammingWeight(unsigned int v1, unsigned int v2) {
        return popCount(v1 ^ v2);
    }

    //Determines whether "value" contains "part"  /  Bestimmt, ob "value" "part" beinhaltet
    inline int contains(int value, int mask, int part, int partmask) {
        if ((value & partmask) == (part & partmask)) {
            if ((mask & partmask) ==  partmask)
                return true;
        }
        return false;
    }

    inline void reduce(){
        int cur = 0;
        int x = 0;
        int y = 0;
        int z = 0;

        for ( x=0; x < length_of_truthtable; x++) {
            // int value = 0;
            // outputTerm(x, length_of_truthtable - 1, num_of_variables);
            // printf(" = ");
            // scanf(" %d", &value);
            if (truthtable[~x & (length_of_truthtable - 1)]) {
                mask[cur][0] = ((1 << num_of_variables)- 1);
                minterm[cur][0] = x;
                cur++;
                result[x] = 1;
            }
            // printf("\n");
        }

        int reduction = 0; //reduction step  / Reduktionsschritt
        int maderedction = false;
        int term = 0;
        int termmask = 0;
        int found = 0;

        for (reduction = 0; reduction < MAX; reduction++) {
            cur = 0;
            maderedction = false;
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

                                found = false;
                                for ( z=0; z<cur; z++) {
                                    if ((minterm[z][reduction+1] == term) && (mask[z][reduction+1] == termmask) ) {
                                        found = true;
                                    }
                                }

                                if (found == false) {
                                    minterm[cur][reduction+1] = term;
                                    mask[cur][reduction+1] = termmask;
                                    cur++;
                                }
                                used[x][reduction] = true;
                                used[y][reduction] = true;
                                maderedction = true;
                            }
                        }
                    }
                }
            }
            if (maderedction == false)
                break; //exit loop early (speed optimisation)  /  Vorzeitig abbrechen (Geschwindigkeitsoptimierung)
        }

        prim_count = 0;
        //printf("\nprime implicants:\n");
        for ( reduction = 0 ; reduction < MAX; reduction++) {
            for ( x=0 ;x < MAX; x++) {
                //Determine all not used minterms  /  Alle nicht verwendeten Minterme bestimmen
                if ((used[x][reduction] == false) && (mask[x][reduction]) ) {
                    //Check if the same prime implicant is already in the list  /  �berpr�fen, ob gleicher Primimplikant bereits in der Liste
                    found = false;
                    for ( z=0; z < prim_count; z++) {
                        if (((prim[z] & primmask[z]) == (minterm[x][reduction] & mask[x][reduction])) &&  (primmask[z] == mask[x][reduction]) )
                            found = true;
                    }
                    if (found == false) {
                        //outputTerm(minterm[x][reduction], mask[x][reduction], num_of_variables);
                        //printf("\n");
                        primmask[prim_count] = mask[x][reduction];
                        prim[prim_count] = minterm[x][reduction];
                        prim_count++;
                    }
                }
            }
        }
    }

    inline void gen_prim(){
        int x = 0;
        int y = 0;
        int z = 0;

        int count = 0;
        int lastprim = 0;
        int res = 0; // actual result  /  Ist-Ausgabe

        //find essential and not essential prime implicants  /  wesentliche und unwesentliche Primimplikanten finden
        //all alle prime implicants are set to "not essential" so far  /  Primimplikanten sind bisher auf "nicht wesentlich" gesetzt
        for (y=0; y < length_of_truthtable; y++) { //for all minterms  /  alle Minterme durchgehen
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
                    wprim[lastprim] = true;
                }
            }
        }

        // successively testing if it is possible to remove prime implicants from the rest matrix  /  Nacheinander testen, ob es m�gich ist, Primimplikaten der Restmatrix zu entfernen
        for ( z=0; z < prim_count; z++) {
            if (primmask[z] ) {
                if (wprim[z] == false) { // && (rwprim[z] == true))
                    nwprim[z] = false; // mark as "not essential" /  als "nicht ben�tigt" markiert
                    for ( y=0; y < length_of_truthtable; y++) { // for all possibilities  /  alle M�glichkeiten durchgehen
                        res = 0;
                        for ( x=0; x < prim_count; x++) {
                            if ( (wprim[x] == true) || (nwprim[x] == true)) {  //essential prime implicant or marked as required  /  wesentlicher Primimplikant oder als ben�tigt markiert
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
                            nwprim[z] = true; //prime implicant required  /  Primimplikant wird doch ben�tigt
                        }
                    }
                }
            }
        }
        // printf("\nResult:\n\n");
    }

    inline void writeOutput(int bitfield, int mask, int num, vector<int>::iterator out) {
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

    inline void gen_out(){
        // *out = (int *) calloc(sizeof(int), prim_count * num_of_variables);
        out.assign(prim_count * num_of_variables, 0);
        vector<int>::iterator out_begin = out.begin();

        // Output of essential and required prime implicants / Ausgabe der wesentlichen und restlichen ben�tigten Primimplikanten:
        int count = 0;
        for (int x = 0 ; x < prim_count; x++) {
            if (wprim[x] == true) {
                // if (count > 0) printf("   ");
                writeOutput(prim[x], primmask[x], num_of_variables, out_begin + x * num_of_variables);
                count++;
            }
            else if ((wprim[x] == false) && (nwprim[x] == true)) {
                // if (count > 0) printf("   ");
                writeOutput(prim[x], primmask[x], num_of_variables, out_begin + x * num_of_variables);
                count++;
            }
        }
    }

    // @num count of variables
    // @out_size will point to the number clauses
    // *out is in the form of DNF. (*out) is an array which has @num * @out_size elements.
    // Each consecutive @num elements is a clause.
    vector<int> qmc_simplify() {
        // int num = 0; // Number of Variables  /  Anzahl Eing�nge
        // int length_of_truthtable = truthtable.size();
        prim_count = 0;

        minterm.assign(MAX, vector<int>(MAX,0));  // 中间项
        mask.assign(MAX, vector<int>(MAX,0));  // 中间项掩码
        used.assign(MAX, vector<int>(MAX,0));  // 使用过的中间项
        result.assign(MAX,0);
        prim.assign(MAX,0);  //质蕴含  // prime implicant
        primmask.assign(MAX, 0);  //质蕴含掩码
        wprim.assign(MAX,false);  //基本质蕴含
        nwprim.assign(MAX,1);  // 需要的基本质蕴含
        //TODO: control the cost of memory

        if (all1 or all0) // All entries are equal, return now
        {
            return vector<int>{};
        }

        // printf("Enter number of variables: ");
        // scanf(" %d", &num_of_variables);
        if (num_of_variables > MAXVARS) {
            // fprintf(stderr, "ERROR: Number of variables too big!\n");
            return vector<int> {};
        }
        if (num_of_variables < 1) {
            // fprintf(stderr, "ERROR: Number of variables must be at least 1!\n");
            return vector<int> {};
        }

        reduce();
        gen_prim();
        gen_out();

        return out;
    }

    string gen_output_str(){
        if (prim_count == 0) /* All truthtable entries are true or false */
            if (all1)
                return("true");
            else
                return("false");

        stringstream output_str;

        int i, j;
        char tmpstr[2]={'\0','\0',};
        for (i = 0; i < prim_count; ++ i)
        {
            if (i > 0)
                output_str << " + ";

            for (j = 0; j < num_of_variables; ++ j)
            {
                switch (out[i * num_of_variables + j])
                {
                    case 0:    /* the literal is false */
                        output_str << "~";
                        // do not break
                    case 1:    /* the literal is true */
                        tmpstr[0] = 'A'+j;
                        output_str << tmpstr;
                        break;
                }
            }
        }
        return output_str.str();
    }

    void print_output(){
        int i, j;
        for (i = 0; i < prim_count; ++ i)
        {
            if (i > 0)
                printf(" + ");

            for (j = 0; j < num_of_variables; ++ j)
            {
                switch (out[i * num_of_variables + j])
                {
                    case 0:    /* the literal is false */
                        printf("~");
                    case 1:    /* the literal is true */
                        printf("%c", 'A'+j);
                        break;
                }
            }
        }

        if (prim_count == 0) /* All truthtable entries are true or false */
            if (all1)
                printf("true");
            else
                printf("false");

        printf("\n");
    }
};

std::string truthtable_to_expr(const std::string& truth_table){
    try{
        TruthtableParser t = TruthtableParser(truth_table);
        t.qmc_simplify();
        return t.gen_output_str();
    }catch(SyntaxError){
        // do nothing
    }catch(StackError){
        // do nothing
    }catch(ValueError){
        // do nothing
    }
}

int main(){
    printf("输入真值表字符串，输出逻辑表达式字符串\n\n");
    printf("示例：01 -> A\n");
    int effective_operands;
    string truthtable;
    while(1){
        printf(">>> ");
        cin >> truthtable;
        while(cin.fail()) {
            cin.clear();
            cin.ignore();
            printf("输入错误。请输入一个仅含0或1的真值表字符串\n");
            cin >> truthtable;
        }
        printf("%s\n", truthtable_to_expr(truthtable).c_str());
        printf("\n");
    }
    return 0;
}
