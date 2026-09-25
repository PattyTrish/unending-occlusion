#ifndef _DOLPHIN_AMCEXI2_INTERNAL_H_
#define _DOLPHIN_AMCEXI2_INTERNAL_H_

#include <dolphin/amc/AmcExi2Comm.h>

typedef struct ECB {
    EXICallback exiCallback;
    EXICallback exiCallback2;
    u8 unk8[4];
    volatile u32 state;
    u32 length;
    u8 *buffer;
} ECB;

s32 AmcEXIImm(void *buf, s32 len, u32 type, EXICallback cb);
void AmcEXISync(void);
void AmcEXIClearInterrupts(s32 pi, s32 ext);
EXICallback AmcEXISetExiCallback(EXICallback cb);
s32 AmcEXISelect(u32 freq);
s32 AmcEXIDeselect(void);
void AmcEXIEnableInterrupts(void);
void AmcEXIInit(void);

#endif
