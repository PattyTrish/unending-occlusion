#ifndef MSL_STDLIB_H_
#define MSL_STDLIB_H_

#include "PowerPC_EABI_Support/Msl/MSL_C/MSL_Common/abort_exit.h"
#include "PowerPC_EABI_Support/Msl/MSL_C/MSL_Common/arith.h"
#include "PowerPC_EABI_Support/Msl/MSL_C/MSL_Common/mbstring.h"
#include "PowerPC_EABI_Support/Msl/MSL_C/MSL_Common/strtoul.h"
#include "stddef.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef int (*_compare_function)(const void*, const void*);

void qsort(void* table_base, size_t num_members, size_t member_size,
           _compare_function compare_members);

int rand(void);
void srand(unsigned int seed);

#ifdef __cplusplus
}
#endif

#endif
