#ifndef METROTRK_MEM_TRK_H
#define METROTRK_MEM_TRK_H
#include "TRK_MINNOW_DOLPHIN/dstypes.h"
#include "TRK_MINNOW_DOLPHIN/trk.h"

typedef struct memRange {
	u8* start;
	u8* end;
	BOOL readable;
	BOOL writeable;
} memRange;

const memRange gTRKMemMap[1] = { { (u8*)0, (u8*)-1, TRUE, TRUE } };

#endif
