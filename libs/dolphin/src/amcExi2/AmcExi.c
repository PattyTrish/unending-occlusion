#include <dolphin.h>
#include <dolphin/hw_regs.h>
#include <string.h>

#include "__amcExi2.h"

static ECB Ecb;
static ECB *exi = &Ecb;

static void CompleteTransfer(void) {
    u8 *dst;
    u32 data;
    s32 i;
    s32 len;

    if (exi->state & 3) {
        if (exi->state & 2) {
            len = exi->length;
            if (len != 0) {
                dst = exi->buffer;
                data = __EXIRegs[14];
                for (i = 0; i < len; i++) {
                    *dst++ = data >> ((3 - i) * 8);
                }
            }
        }
        exi->state &= ~3;
    }
}

s32 AmcEXIImm(void *buf, s32 len, u32 type, EXICallback cb) {
    BOOL enabled;
    int data;
    int i;
    int chan = 2;

    enabled = OSDisableInterrupts();
    if ((exi->state & 3) || !(exi->state & 4)) {
        OSRestoreInterrupts(enabled);
        return 0;
    }

    exi->exiCallback2 = cb;
    if ((u32)exi->exiCallback2 != 0) {
        AmcEXIClearInterrupts(0, 1);
        __OSUnmaskInterrupts(OS_INTERRUPTMASK_EXI_0_TC >> (chan * 3));
    }

    exi->state |= 2;
    if (type != 0) {
        data = 0;
        for (i = 0; i < len; i++) {
            data |= (int)((u8 *)buf)[i] << ((3 - i) * 8);
        }
        __EXIRegs[14] = data;
    }

    exi->buffer = buf;
    exi->length = (type != 1) ? len : 0;
    __EXIRegs[13] = (type * 4) | 1 | ((len - 1) * 0x10);

    OSRestoreInterrupts(enabled);
    return 1;
}

// stripped, size 0xDC
// void AmcEXIDma(void);

void AmcEXISync(void) {
    while (exi->state & 4) {
        if (!(__EXIRegs[13] & 1)) {
            BOOL enabled = OSDisableInterrupts();
            CompleteTransfer();
            OSRestoreInterrupts(enabled);
            break;
        }
    }
}

void AmcEXIClearInterrupts(s32 pi, s32 ext) {
    u32 stat;
    u32 cause;

    if (pi != 0) {
        cause = __PIRegs[0];
        cause &= 0x1000;
        cause |= 0x1000;
        __PIRegs[0] = cause;
    }
    if (ext != 0) {
        stat = __EXIRegs[10];
        stat &= 0x7F5;
        stat |= 8;
        __EXIRegs[10] = stat;
    }
}

EXICallback AmcEXISetExiCallback(EXICallback cb) {
    EXICallback prev;
    BOOL enabled;

    enabled = OSDisableInterrupts();
    if (cb) {
        __OSUnmaskInterrupts(OS_INTERRUPTMASK_PI_DEBUG);
    } else {
        __OSMaskInterrupts(OS_INTERRUPTMASK_PI_DEBUG);
    }

    prev = exi->exiCallback;
    exi->exiCallback = cb;
    OSRestoreInterrupts(enabled);
    return prev;
}

// stripped, size 0x10 each
// void AmcEXIEnable32MHz(void);
// void AmcEXIDisable32MHz(void);

s32 AmcEXISelect(u32 freq) {
    BOOL enabled;
    int stat;

    enabled = OSDisableInterrupts();
    if (exi->state & 4) {
        OSRestoreInterrupts(enabled);
        return 0;
    }
    exi->state |= 4;
    stat = __EXIRegs[10];
    stat &= 0x405;
    stat |= (freq * 0x10) | 0x80;
    __EXIRegs[10] = stat;
    OSRestoreInterrupts(enabled);
    return 1;
}

s32 AmcEXIDeselect(void) {
    BOOL enabled = OSDisableInterrupts();
    int stat;

    if (!(exi->state & 4)) {
        OSRestoreInterrupts(enabled);
        return 0;
    }
    exi->state &= ~4;
    stat = __EXIRegs[10];
    stat &= 0x405;
    __EXIRegs[10] = stat;
    OSRestoreInterrupts(enabled);
    return 1;
}

// stripped, size 0x1DC
// void AmcTCIntrruptHandler(void);

static void AmcDebugIntHandler(__OSInterrupt interrupt, OSContext *context) {
    EXICallback callback;

    AmcEXIClearInterrupts(TRUE, FALSE);
    callback = exi->exiCallback;
    if (callback) {
        callback(2, context);
    }
}

void AmcEXIEnableInterrupts(void) {
    __OSMaskInterrupts(OS_INTERRUPTMASK_EXI_2_TC);
    __OSSetInterruptHandler(__OS_INTERRUPT_PI_DEBUG, AmcDebugIntHandler);
    __OSUnmaskInterrupts(OS_INTERRUPTMASK_PI_DEBUG);
}

void AmcEXIInit(void) {
    memset(&Ecb, 0, sizeof(Ecb));
    __EXIRegs[10] = 0;
    __SIRegs[15] = 0;
}
