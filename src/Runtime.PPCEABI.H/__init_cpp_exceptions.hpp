#ifndef RUNTIME___INIT_CPP_EXCEPTIONS_H
#define RUNTIME___INIT_CPP_EXCEPTIONS_H

#ifdef __cplusplus
extern "C" {
#endif

void __fini_cpp_exceptions(void);
void __init_cpp_exceptions(void);
void __destroy_global_chain(void);

#ifdef __cplusplus
}
#endif

#endif
