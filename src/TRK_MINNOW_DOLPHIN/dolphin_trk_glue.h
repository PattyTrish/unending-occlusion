#ifndef _METROTRK_DOLPHIN_TRK_GLUE_H
#define _METROTRK_DOLPHIN_TRK_GLUE_H

#include <dolphin/os.h>
#include "TRK_MINNOW_DOLPHIN/trk.h"

#ifdef __cplusplus
extern "C" {
#endif // ifdef __cplusplus

// Function types for DB communications.
typedef int (*DBCommFunc)();
typedef void (*DBCommInitFunc)(volatile u8**, __OSInterruptHandler);
typedef int (*DBCommReadFunc)(void*, u32);
typedef int (*DBCommWriteFunc)(const void*, u32);

// Struct for storing DB communication functions (size 0x1C).
typedef struct DBCommTable {
	DBCommInitFunc initialize_func;  // _00
	DBCommFunc init_interrupts_func; // _04
	DBCommFunc peek_func;            // _08
	DBCommReadFunc read_func;        // _0C
	DBCommWriteFunc write_func;      // _10
	DBCommFunc open_func;            // _14
	DBCommFunc close_func;           // _18
} DBCommTable;

void TRKEXICallBack(__OSInterrupt, OSContext*);

#ifdef __cplusplus
};
#endif // ifdef __cplusplus

#endif
