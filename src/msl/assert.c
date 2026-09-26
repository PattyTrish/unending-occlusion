#include "PowerPC_EABI_Support/Msl/MSL_C/MSL_Common/stdio.h"
#include "PowerPC_EABI_Support/Msl/MSL_C/MSL_Common/abort_exit.h"

void __assertion_failed(char const* condition, char const* filename, int lineno)
{
	fprintf(stderr, "Assertion (%s) failed in \"%s\" on line %d\n", condition, filename, lineno);
	abort();
}
