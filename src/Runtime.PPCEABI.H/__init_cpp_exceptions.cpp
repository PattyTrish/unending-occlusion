#include "__init_cpp_exceptions.hpp"
#include "global_destructor_chain.h"
#include "Gecko_ExceptionPPC.h"
#include "platform.h"

static int fragmentID = -2;

SECTION_INIT extern __eti_init_info _eti_init_info[];

static asm char* GetR2(void) {
#ifdef __MWERKS__ // clang-format off
    nofralloc
    mr r3, r2
    blr
#endif // clang-format on
}

void __fini_cpp_exceptions(void) {
    if (fragmentID != -2) {
        __unregister_fragment(fragmentID);
        fragmentID = -2;
    }
}

void __init_cpp_exceptions(void) {
    if (fragmentID == -2) {
        fragmentID = __register_fragment(_eti_init_info, GetR2());
    }
}

SECTION_CTORS extern void* const __init_cpp_exceptions_reference = __init_cpp_exceptions;
SECTION_DTORS extern void* const __destroy_global_chain_reference = __destroy_global_chain;
SECTION_DTORS extern void* const __fini_cpp_exceptions_reference = __fini_cpp_exceptions;
