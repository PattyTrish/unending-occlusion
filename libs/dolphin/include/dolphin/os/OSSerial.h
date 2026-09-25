#ifndef _DOLPHIN_OSSERIAL_H
#define _DOLPHIN_OSSERIAL_H

#include <dolphin/hw_regs.h>
#include <dolphin/si.h>

typedef struct SIPacket {
    s32 chan;
    void *output;
    u32 outputBytes;
    void *input;
    u32 inputBytes;
    SICallback callback;
    OSTime fire;
} SIPacket;

void SIInit(void);
u32 SIGetStatus(s32 chan);
BOOL SIIsChanBusy(s32 chan);
u32 SISync(void);
void SISetCommand(s32 chan, u32 command);
BOOL SIGetResponse(s32 chan, void *data);
BOOL SIRegisterPollingHandler(__OSInterruptHandler handler);
BOOL SIUnregisterPollingHandler(__OSInterruptHandler handler);
u32 SIGetType(s32 chan);
u32 SIGetTypeAsync(s32 chan, SITypeAndStatusCallback callback);
u32 SIDecodeType(u32 type);
void SISetSamplingRate(u32 msec);
void SIRefreshSamplingRate(void);

#endif // _DOLPHIN_OSSERIAL_H
