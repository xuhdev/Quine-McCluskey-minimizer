/*
 * Example for 3 inputs
 *
 * This file is written by Hong Xu <hong@topbug.net> as an example for
 * the Quine-McCluskey-minimizer. This file is in public domain.
 *
 * To compile this example, simply run
 *
 *     gcc -o example example.c quine_mc_cluskey.c
 *
 * You can replace gcc with other compilers if you are not using gcc.
 */

#include <stddef.h>
#include <stdio.h>
#include <stdbool.h>

#include "qmc.h"

int main()
{
    bool table[8];
    int* out;
    size_t out_size;
    size_t i, j;

    /* the truth table -- 2 inputs 1 output
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

    table[0] = true;
    table[1] = true;
    table[2] = true;
    table[3] = true;
    table[4] = true;
    table[5] = true;
    table[6] = false;
    table[7] = false;


    /* simplify the table */
    if (qmc_simplify(table, 3, &out, &out_size))
    {
        fprintf(stderr, "Simpification failed!\n");
        /* No need to call qmc_free(out) if failed */
        return 1;
    }

    /* print the output */
    for (i = 0; i < out_size; ++ i)
    {
        if (i > 0)
            printf(" + ");

        for (j = 0; j < 3u; ++ j)
        {
            switch (out[i * 3u + j])
            {
            case 0:    /* the literal is false */
                printf("~");
            case 1:    /* the literal is true */
                switch (j)
                {
                case 0:
                    printf("A");
                    break;
                case 1:
                    printf("B");
                    break;
                case 2:
                    printf("C");
                    break;
                }
                break;
            }
        }
    }

    printf("\n");

    qmc_free(out);

    return 0;
}
