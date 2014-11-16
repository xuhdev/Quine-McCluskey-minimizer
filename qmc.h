/*
 * This file is added by Hong Xu <hong@topbug.net> to the original
 * Quine-McCluskey minimizer (http://sourceforge.net/projects/mini-qmc/) in
 * order for it to be used as a library. This file is under public domain so
 * you can use it without any restriction, but no warranty as well.
 */

#ifndef QMC_H_
#define QMC_H_

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Simplify a truth table.
 *
 * @param num is the number of literals (input) in your truth table.
 *
 * @param table is the truth table you specified. It is a boolean array, with
 * each bit of the index specifying the input. e.g. for a truth table which has
 * 2 literals, table[0] and table[3] indicate the output when both of them are
 * false and true respectively, while table[1] and table[2] indicates the
 * output when one of them is true and the other is false.
 *
 * @param out is the output simplified boolean expression. *out has a the
 * number of elements of @num * @out_size, which means *out contains @out_size
 * groups of @num elements. Each group represents a clause. For each group, the
 * nth element indicates the status of nth literal in the clause -- 0 means
 * false, 1 means true, -1 means this literal is not in this clause. *out must
 * be released by using qmc_free() when it's no longer been used.
 *
 * @param out_size is the output size. See @out for details.
 *
 * @return 0 if succeeds, otherwise fails.
 */
int qmc_simplify(const bool * table, size_t num, int ** out, size_t * out_size);

/**
 * The function used to free *out in qmc_simplify.
 *
 * @param p the pointer needs to be freed.
 */
void qmc_free(void * p);

#ifdef __cplusplus
}
#endif

#endif /* QMC_H_ */
