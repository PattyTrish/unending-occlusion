#include "platform.h"

typedef void (*Destructor)(void*, int);
typedef struct DestructorChain {
    struct DestructorChain* next;
    Destructor destructor;
    void* object;
} DestructorChain;

DestructorChain* __global_destructor_chain;

void __destroy_global_chain(void)
{
    while (__global_destructor_chain) {
        DestructorChain* current = __global_destructor_chain;
        __global_destructor_chain = current->next;
        current->destructor(current->object, -1);
    }
}

SECTION_DTORS static void* const __destroy_global_chain_reference = __destroy_global_chain;
