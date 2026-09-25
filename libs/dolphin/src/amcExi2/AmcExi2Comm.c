#include <dolphin.h>
#include <dolphin/hw_regs.h>
#include <stdio.h>

#include "__amcExi2.h"

static u8 ucEXI2InputPending;
static u8 *pucEXI2InputPending = &ucEXI2InputPending;
static s32 fExi2Selected;
static EXICallback TRK_Callback;

static void EXI2_SetTRKCallback(EXICallback monitorCallback) {
    BOOL enabled = OSDisableInterrupts();
    TRK_Callback = monitorCallback;
    OSRestoreInterrupts(enabled);
}

static void EXI2_CallBack(s32 chan, OSContext *context) {
    *pucEXI2InputPending = 1;
    if (TRK_Callback) {
        TRK_Callback(0, context);
    }
}

static s32 EXI2_Select(s32 freq, s32 unused) {
    s32 status = 0;

    while (status == 0) {
        status = AmcEXISelect(freq);
    }
    return status;
}

static void EXI2_ToWriteMode(void) {
    u32 buf = 0x80000000;

    AmcEXIImm(&buf, 2, 1, NULL);
    AmcEXISync();
}

static void EXI2_ToReadMode(u16 addr) {
    u32 buf = addr << 16;

    AmcEXIImm(&buf, 2, 1, NULL);
    AmcEXISync();
}

static s32 EXI2_StartRead(void) {
    u32 buf;
    s32 status;

    if (fExi2Selected != 0) {
        status = 1;
    } else {
        status = EXI2_Select(5, 0);
        if (status == 0) {
            printf("Can't select EXI2 port!\n");
            return 0;
        }
        fExi2Selected = 1;
        EXI2_ToReadMode(0);
        AmcEXIImm(&buf, 2, 0, NULL);
        AmcEXISync();
    }
    return status;
}

static void EXI2_FinishRead(void) {
    AmcEXIDeselect();
    fExi2Selected = 0;
    *pucEXI2InputPending = 0;
}

static s32 EXI2_SendCmd(u8 cmd, u32 addr, s32 data, u32 unused) {
    s32 status;
    s32 buf1 = (cmd << 24) | (addr & 0xFFFFFF);
    s32 buf2 = data;

    status = EXI2_Select(5, 0);
    EXI2_ToWriteMode();

    AmcEXIImm(&buf1, 4, 1, NULL);
    AmcEXISync();

    AmcEXIImm(&buf2, 4, 1, NULL);
    AmcEXISync();

    AmcEXIDeselect();
    return status;
}

void EXI2_Init(volatile unsigned char **inputPendingPtrRef, EXICallback monitorCallback) {
    *inputPendingPtrRef = pucEXI2InputPending;
    EXI2_SetTRKCallback(monitorCallback);
    AmcEXIInit();
    EXI2_SendCmd(1, 3, 0, 0);
}

void EXI2_EnableInterrupts(void) {
    AmcEXISetExiCallback(EXI2_CallBack);
    AmcEXIEnableInterrupts();
}

int EXI2_Poll(void) {
    u32 data;
    u32 buf;
    u32 unused;

    if (!(__PIRegs[0] & 0x1000) && *pucEXI2InputPending == 0) {
        data = 0;
    } else {
        // EXI2_StartRead, inlined without its early return
        s32 status;

        if (fExi2Selected != 0) {
            status = 1;
        } else {
            status = EXI2_Select(5, 0);
            if (status == 0) {
                printf("Can't select EXI2 port!\n");
            } else {
                fExi2Selected = 1;
                EXI2_ToReadMode(0);
                AmcEXIImm(&buf, 2, 0, NULL);
                AmcEXISync();
            }
        }
        AmcEXIImm(&data, 4, 0, NULL);
        AmcEXISync();
        if (*pucEXI2InputPending == 0) {
            __PIRegs[0] &= 0x1000;
            *pucEXI2InputPending = 1;
        }
    }
    return data;
}

AmcExiError EXI2_ReadN(void *bytes, unsigned long length) {
    u8 *out = (u8 *)bytes;
    u32 len = length;
    s32 status;
    int i, j;
    s32 words;
    int count;
    u32 data;

    status = EXI2_StartRead();
    if (status == 0) {
        return AMC_EXI_UNSELECTED;
    }

    words = (len / 4) + ((len % 4) ? 1 : 0);

    for (i = 0; i < len; i += 4) {
        AmcEXIImm(&data, ((u32)(i / 4) < (words - 1)) ? 4 : ((len & 3) + (len & 1)) == 2 ? 2 : 4, 0, NULL);
        AmcEXISync();

        if ((len - i) >= 4) {
            count = 4;
        } else {
            count = (s32)len % 4;
        }

        for (j = 0; j < count; j++) {
            out[i + j] = data >> ((3 - j) * 8);
        }
    }

    EXI2_FinishRead();
    return AMC_EXI_NO_ERROR;
}

AmcExiError EXI2_WriteN(const void *bytes, unsigned long length) {
    u32 size;
    u32 data;
    s32 status;
    u32 i;
    u32 unused;
    u32 *words = (u32 *)bytes;
    u32 count;

    size = length;
    if ((status = EXI2_Select(5, 0)) == 0) {
        return AMC_EXI_UNSELECTED;
    }

    EXI2_ToWriteMode();
    AmcEXIImm(&size, 4, 1, NULL);
    AmcEXISync();

    count = (size >> 2) + ((size & 3) ? 1 : 0);

    for (i = 0; i < count; i++) {
        AmcEXIImm(words++, (i < (count - 1)) ? 4 : ((u32)((size & 3) + (size & 1)) == 2 ? 2 : 4), 1, NULL);
        AmcEXISync();
    }
    AmcEXIDeselect();

    status = EXI2_Select(5, 0);
    if (status == 0) {
        return AMC_EXI_UNSELECTED;
    }

    EXI2_ToReadMode(1);
    AmcEXIImm(&data, 2, 0, NULL);
    AmcEXISync();

    do {
        AmcEXIImm(&data, 2, 0, NULL);
        AmcEXISync();
    } while (!((data >> 16) & 1));

    AmcEXIDeselect();
    return AMC_EXI_NO_ERROR;
}

void EXI2_Reserve(void) {}

void EXI2_Unreserve(void) {}

// stripped, size 0xB0
// AmcExiError EXI2_GetStatusReg(u16 *pu16StatusReg);
