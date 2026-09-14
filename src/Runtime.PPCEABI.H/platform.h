#ifndef SECTION_INIT
#if defined(__MWERKS__) && !defined(M2CTX)
#define SECTION_INIT __declspec(section ".init")
#else
#define SECTION_INIT
#endif
#endif

#ifndef SECTION_CTORS
#if defined(__MWERKS__) && !defined(M2CTX)
#define SECTION_CTORS __declspec(section ".ctors")
#else
#define SECTION_CTORS
#endif
#endif

#ifndef SECTION_DTORS
#if defined(__MWERKS__) && !defined(M2CTX)
#define SECTION_DTORS __declspec(section ".dtors")
#else
#define SECTION_DTORS
#endif
#endif
