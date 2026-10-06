/* Silicon Knights' Ogg Vorbis streamer (0x802364B8..0x80237EE0): a decode thread built on libvorbis's
   examples/decoder_example.c, reading the file from DVD and writing 16-bit PCM into two MusyX streams (left,
   right). No strings name this file or its functions: the file name is a placeholder and the symbols keep their
   addresses. */

#include <string.h>
#include <stdio.h>
#include <dolphin/os.h>
#include <dolphin/dvd.h>
#include <dolphin/vi.h>
#include <ogg/ogg.h>
#include "vorbis/codec.h"

typedef struct OSMessageQueue OSMessageQueue;
typedef void* OSMessage;
BOOL OSCreateThread(OSThread* thread, void* (*func)(void*), void* param, void* stack, u32 stackSize,
                    OSPriority priority, u16 attr);
BOOL OSSetThreadPriority(OSThread* thread, OSPriority priority);

typedef u32 SND_STREAMID;
SND_STREAMID sndStreamAllocEx(u8 prio, void* buffer, u32 samples, u32 frq, u8 vol, u8 pan, u8 span, u8 auxa,
                              u8 auxb, u8 studio, u32 flags,
                              u32 (*updateFunction)(void* buffer1, u32 len1, void* buffer2, u32 len2, u32 user),
                              u32 user, void* adpcmInfo);
void sndStreamFree(SND_STREAMID stid);
void sndStreamMixParameterEx(SND_STREAMID stid, u8 vol, u8 pan, u8 span, u8 auxa, u8 auxb);
int sndStreamActivate(SND_STREAMID stid);
void sndStreamDeactivate(SND_STREAMID stid);
void sndStreamARAMUpdate(SND_STREAMID stid, u32 off1, u32 len1, u32 off2, u32 len2);

void fn_8022F700(void* base, long size);
void fn_8022F75C(void);
void* fn_8022F780(long size);
void fn_8022F854(void* base, long size);
void fn_8022F8F8(void);
void fn_8022F8FC(void);

u8 fn_801A9B94(u8 vol, int);
void fn_801ADC50(void);
void fn_801ADCF8(void);
extern int lbl_8064C64C;
extern int lbl_8064D294;

typedef struct StreamChannel {
  SND_STREAMID stid; // 0x00
  s16* buffer;       // 0x04
  int flags;         // 0x08
  u32 half;          // 0x0C
  int lastHalf;      // 0x10
  int fadeDelay;     // 0x14
  u8 curVol;         // 0x18
  u8 vol;            // 0x19
  u8 newVol;         // 0x1A
  u8 pan;            // 0x1B
} StreamChannel;

typedef struct StreamRequest {
  int type;                                 // 0x00
  u32 size;                                 // 0x04
  u32 half;                                 // 0x08
  SND_STREAMID stid;                        // 0x0C
  s16* dest;                                // 0x10
  void (*callback)(SND_STREAMID stid, u32 half); // 0x14
  OSMessageQueue* queue;                    // 0x18
} StreamRequest;

typedef struct StreamCommand {
  int type;              // 0x00
  char name[0x50];       // 0x04
  void* buffer;          // 0x54
  OSMessageQueue* queue; // 0x58
} StreamCommand;

static volatile StreamChannel lbl_8064A548[2];
static OSMessageQueue lbl_8064A528;
static OSMessage lbl_8064A508[8];
static OSThread lbl_8064A1F8;
static u8 lbl_806481F8[0x2000];
static OSMessageQueue lbl_806481D8;
static OSMessage lbl_806481B8[8];
static OSMessageQueue lbl_80648198;
static OSMessageQueue lbl_80648178;
static OSMessage lbl_80648158[8];
static u8 lbl_80648128[0x30];
static StreamRequest lbl_8064810C;
static StreamRequest lbl_806480F0;
static StreamRequest lbl_806480B8[2];
static StreamCommand lbl_8064805C;
static s16 lbl_80647C5C[512];
static DVDFileInfo lbl_80647C20;
static u8 lbl_80643C20[0x4000] ATTRIBUTE_ALIGN(32);

int lbl_8064C4D8 = 512;

static volatile int lbl_8064DC78;
static int lbl_8064DC74;
static int lbl_8064DC70;
static int lbl_8064DC6C;
static int lbl_8064DC68;
static OSMessage lbl_8064DC60[2];
static volatile int lbl_8064DC5C;
static volatile int lbl_8064DC58;
static int lbl_8064DC54;
static int lbl_8064DC50;
static int lbl_8064DC4C;
static int lbl_8064DC48;
static int lbl_8064DC44;
static int lbl_8064DC40;
static int lbl_8064DC3C;
static volatile int lbl_8064DC38;
static volatile int lbl_8064DC34;
static int lbl_8064DC30;

static void SetVolume(volatile StreamChannel* c) {
  c->newVol = c->vol;
  if (c->curVol != c->newVol) {
    sndStreamMixParameterEx(c->stid, fn_801A9B94(c->newVol, 2), c->pan, 0, 0, 0);
  }
  c->curVol = c->newVol;
}

void fn_80237EAC(SND_STREAMID stid, u32 half) { sndStreamARAMUpdate(stid, half << 13, 0x2000, 0, 0); }

void fn_80237E7C(s32 result, DVDFileInfo* fileInfo) { OSSendMessage(&lbl_80648178, (OSMessage)result, 1); }

#pragma dont_inline on
void fn_80237DB4(char* name) {
  OSInitMessageQueue(&lbl_80648178, lbl_80648158, 8);
  lbl_8064DC3C = 0;
  DVDOpen(name, &lbl_80647C20);
  lbl_8064DC54 = lbl_80647C20.length;
  lbl_8064DC50 = 0;
  while (!DVDReadAsyncPrio(&lbl_80647C20, lbl_80643C20, 0x2000, lbl_8064DC50, fn_80237E7C, 2)) {
  }
  lbl_8064DC30 = 0x2000;
  lbl_8064DC50 += 0x2000;
  lbl_8064DC54 -= 0x2000;
  lbl_8064DC48 = 0;
  lbl_8064DC74 = 0;
}
#pragma dont_inline reset

void fn_80237D84(void) {
  if (lbl_8064DC38) {
    lbl_8064A548[0].flags |= 8;
    lbl_8064A548[1].flags |= 8;
  }
}

void fn_80237D2C(int fast) {
  if (fast) {
    OSSetThreadPriority(&lbl_8064A1F8, 8);
    lbl_8064DC6C = 8;
  } else {
    OSSetThreadPriority(&lbl_8064A1F8, 0x12);
    lbl_8064DC6C = 0x12;
  }
}

void fn_80237C28(void) {
  int i;
  volatile StreamChannel* c;

  OSSetThreadPriority(&lbl_8064A1F8, 0x12);
  fn_80237D2C(0);
  if (lbl_8064DC38) {
    lbl_8064A548[0].flags |= 8;
    lbl_8064A548[1].flags |= 8;
  } else {
    for (i = 0; i < 2; i++) {
      if (lbl_8064A548[i].flags) {
        sndStreamDeactivate(lbl_8064A548[i].stid);
        sndStreamFree(lbl_8064A548[i].stid);
        lbl_8064A548[i].flags = 0;
      }
    }
  }
  c = lbl_8064A548;
  while (c[0].flags || c[1].flags || lbl_8064DC38) {
    VIWaitForRetrace();
  }
  fn_801ADC50();
}

int fn_80237A04(char* buffer, u32 size) {
  int ret;
  OSMessage msg;
  int blk;
  u32 n;

  ret = size;
  if ((lbl_8064DC74 >= 0x2000) == lbl_8064DC48) {
    blk = 0x2000;
    if (lbl_8064DC54 > 0) {
      do {
        OSReceiveMessage(&lbl_80648178, &msg, 1);
        if (lbl_8064DC30 != (int)msg) {
          OSReport("Error detected. Resumbitting job!\n");
          while (!DVDReadAsyncPrio(&lbl_80647C20, lbl_80643C20 + (lbl_8064DC48 << 13), lbl_8064DC30,
                                   lbl_8064DC50 - lbl_8064DC30, fn_80237E7C, 2)) {
          }
        }
      } while (lbl_8064DC30 != (int)msg);
    }
    lbl_8064DC48 ^= 1;
    if (lbl_8064DC54 < 0x2000) {
      blk = (lbl_8064DC54 + 31) & ~31;
      lbl_8064DC54 = 0;
    } else {
      lbl_8064DC54 -= 0x2000;
    }
    memset(lbl_80643C20 + (lbl_8064DC48 << 13), 0, 0x2000);
    DCFlushRange(lbl_80643C20 + (lbl_8064DC48 << 13), 0x2000);
    if (blk != 0) {
      while (!DVDReadAsyncPrio(&lbl_80647C20, lbl_80643C20 + (lbl_8064DC48 << 13), blk, lbl_8064DC50, fn_80237E7C,
                               2)) {
      }
      lbl_8064DC30 = blk;
      lbl_8064DC50 += blk;
    }
  }
  n = 0x4000 - lbl_8064DC74;
  if (n <= size) {
    memcpy(buffer, lbl_80643C20 + lbl_8064DC74, n);
    if (size == 0 && lbl_8064DC74 <= 0 && lbl_8064DC54 == 0) {
      ret = 0;
    }
    lbl_8064DC74 = 0;
    size -= n;
  }
  if (size != 0) {
    memcpy(buffer, lbl_80643C20 + lbl_8064DC74, size);
    lbl_8064DC74 += size;
  }
  return ret;
}

u8* fn_80236AF8(u8* buffer, OSMessageQueue* queue);

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

int fn_80237704(s16* pcm, int samples, int channels) {
  StreamRequest* msg;
  StreamRequest* left;
  StreamRequest* right;
  int n;
  int i;
  s16* l;
  s16* r;

  if (lbl_8064DC4C == 2) {
    printf("Aborted already....\n");
    return 1;
  }
  left = &lbl_8064810C;
  right = &lbl_806480F0;
  samples += lbl_8064DC78;
  do {
    if (lbl_8064DC3C == 0) {
      left->dest = NULL;
      right->dest = NULL;
      while (left->dest == NULL || right->dest == NULL) {
        if (!OSReceiveMessage(&lbl_8064A528, (OSMessage*)&msg, 0)) {
          OSReceiveMessage(&lbl_8064A528, (OSMessage*)&msg, 1);
        }
        switch (msg->type) {
        case 0:
          lbl_8064810C = *msg;
          break;
        case 1:
          lbl_806480F0 = *msg;
          break;
        case 2:
          lbl_8064DC54 = 0;
          if (++lbl_8064DC4C == 2) {
            return 1;
          }
          break;
        }
        lbl_8064DC3C = MAX((u32)lbl_8064DC3C, msg->size);
      }
    }
    n = MIN(lbl_8064DC3C, samples);
    l = left->dest;
    r = right->dest;
    if (channels == 2) {
      for (i = 0; i < n; i++) {
        if (i < lbl_8064DC78) {
          *l++ = 0;
          *r++ = 0;
        } else {
          *l++ = pcm[0];
          *r++ = pcm[1];
          pcm += 2;
        }
      }
      DCFlushRange(left->dest, n * 2);
      DCFlushRange(right->dest, n * 2);
    }
    samples -= n;
    lbl_8064DC78 -= n;
    left->dest = l;
    right->dest = r;
    lbl_8064DC3C -= n;
    if (lbl_8064DC78 < 0) {
      lbl_8064DC78 = 0;
    }
    if (lbl_8064DC3C == 0) {
      if (right->callback) {
        right->callback(right->stid, right->half);
      }
      if (left->callback) {
        left->callback(left->stid, left->half);
      }
      if (left->queue) {
        OSSendMessage(left->queue, NULL, 1);
      }
      if (right->queue) {
        OSSendMessage(right->queue, NULL, 1);
      }
    }
    if (lbl_8064DC34 == 2) {
      lbl_8064DC34 = 1;
    }
  } while (samples != 0);
  return 0;
}

void fn_80237224(char* name) {
  ogg_sync_state oy;
  ogg_stream_state os;
  ogg_page og;
  ogg_packet op;
  vorbis_info vi;
  vorbis_comment vc;
  vorbis_dsp_state vd;
  vorbis_block vb;
  struct OSStopwatch sw;
  char* buffer;
  int bytes;

  OSCheckHeap(__OSCurrHeap);
  OSInitStopwatch(&sw, "decompress");
  OSResetStopwatch(&sw);
  OSStartStopwatch(&sw);
  ogg_sync_init(&oy);
  fn_80237DB4(name);

  while (1) {
    int eos = 0;
    int i;

    buffer = ogg_sync_buffer(&oy, 4096);
    bytes = fn_80237A04(buffer, 4096);
    ogg_sync_wrote(&oy, bytes);
    if (ogg_sync_pageout(&oy, &og) != 1) {
      if (bytes < 4096) break;
      break;
    }
    ogg_stream_init(&os, ogg_page_serialno(&og));
    vorbis_info_init(&vi);
    vorbis_comment_init(&vc);
    if (ogg_stream_pagein(&os, &og) < 0) {
      break;
    }
    if (ogg_stream_packetout(&os, &op) != 1) {
      break;
    }
    fn_8022F8FC();
    if (vorbis_synthesis_headerin(&vi, &vc, &op) < 0) {
      break;
    }
    fn_8022F8FC();

    i = 0;
    while (i < 2) {
      while (i < 2) {
        int result = ogg_sync_pageout(&oy, &og);
        if (result == 0) break;
        if (result == 1) {
          ogg_stream_pagein(&os, &og);
          while (i < 2) {
            result = ogg_stream_packetout(&os, &op);
            if (result == 0) break;
            if (result < 0) {
              break;
            }
            vorbis_synthesis_headerin(&vi, &vc, &op);
            i++;
          }
        }
      }
      buffer = ogg_sync_buffer(&oy, 4096);
      bytes = fn_80237A04(buffer, 4096);
      if (bytes == 0 && i < 2) {
        break;
      }
      ogg_sync_wrote(&oy, bytes);
    }

    {
      char** ptr = vc.user_comments;
      while (*ptr) {
        ++ptr;
      }
    }

    lbl_8064C4D8 = 512 / vi.channels;
    fn_8022F8FC();
    vorbis_synthesis_init(&vd, &vi);
    fn_8022F8FC();
    vorbis_block_init(&vd, &vb);
    fn_8022F8FC();

    while (!eos) {
      while (!eos) {
        int result = ogg_sync_pageout(&oy, &og);
        if (result == 0) break;
        if (result < 0) {
          eos = 1;
          break;
        } else {
          ogg_stream_pagein(&os, &og);
          while (1) {
            result = ogg_stream_packetout(&os, &op);
            if (result == 0) break;
            if (lbl_8064DC4C) break;
            if (result < 0) {
            } else {
              s16** pcm;
              int samples;
              if (vorbis_synthesis(&vb, &op) == 0) vorbis_synthesis_blockin(&vd, &vb);
              while ((samples = vorbis_synthesis_pcmout(&vd, &pcm)) > 0) {
                int j;
                int bout = (samples < lbl_8064C4D8 ? samples : lbl_8064C4D8);
                for (i = 0; i < vi.channels; i++) {
                  s16* ptr = lbl_80647C5C + i;
                  s16* mono = pcm[i];
                  for (j = 0; j < bout; j++) {
                    *ptr = mono[j];
                    ptr += vi.channels;
                  }
                }
                if (fn_80237704(lbl_80647C5C, bout, vi.channels)) {
                  lbl_8064DC74 = 0;
                  lbl_8064DC54 = 0;
                  break;
                }
                fn_8022F8FC();
                vorbis_synthesis_read(&vd, bout);
              }
            }
            if (lbl_8064DC4C) break;
          }
          if (ogg_page_eos(&og) || lbl_8064DC4C) eos = 1;
        }
      }
      if (!eos) {
        buffer = ogg_sync_buffer(&oy, 4096);
        bytes = fn_80237A04(buffer, 4096);
        ogg_sync_wrote(&oy, bytes);
        if (bytes == 0) eos = 1;
      }
    }
    fn_8022F8F8();
    break;
  }

  OSCheckStopwatch(&sw);
  ogg_sync_clear(&oy);
  lbl_8064DC34 = 0;
  lbl_8064DC38 = 0;
  if (lbl_8064A548[0].flags) {
    lbl_8064A548[0].flags = (lbl_8064A548[0].flags & ~1) | 2;
  }
  if (lbl_8064A548[1].flags) {
    lbl_8064A548[1].flags = (lbl_8064A548[1].flags & ~1) | 2;
  }
}

u32 fn_80236DC4(void* buffer1, u32 len1, void* buffer2, u32 len2, u32 user) {
  int half;
  int prev;
  StreamRequest* r;
  volatile StreamChannel* c;

  c = &lbl_8064A548[user];
  r = &lbl_806480B8[user];
  half = len1 >= 0x2000;
  prev = c->half;
  if (c->flags & 2) {
    sndStreamDeactivate(c->stid);
    sndStreamFree(c->stid);
    c->flags = 0;
    return 0;
  }
  if (user == 0) {
    if (!(c->flags & 0x10)) {
      lbl_8064DC58 = len1 - (half << 13);
      if (c->lastHalf != half) {
        lbl_8064DC5C += 0x2000;
        c->lastHalf = half;
      }
    } else if (c->lastHalf != half) {
      c->lastHalf = half;
    }
  }
  if (c->flags & 4) {
    if (c->vol != 100) {
      if (c->flags & 0x80) {
        c->vol = MIN(c->vol + 20, 100);
      } else {
        c->vol = MIN(c->vol + 50, 100);
      }
    } else {
      c->flags &= ~0x84;
    }
  } else if ((c->flags & 8) || (c->flags & 0x10) || (c->flags & 0x20)) {
    if (c->vol != 0) {
      c->vol = c->vol - 20 < 0 ? 0 : c->vol - 20;
    } else if (!(c->flags & 0x10) && !(c->flags & 0x20)) {
      r->type = 2;
      r->size = 0;
      r->dest = NULL;
      OSSendMessage(&lbl_8064A528, r, 1);
      c->flags |= 2;
    }
  }
  lbl_8064A548[1].vol = lbl_8064A548[0].vol;
  SetVolume(c);
  if (half != prev) {
    if (c->flags & 0x20) {
      c->flags |= 0x10;
      c->flags &= ~0x20;
    }
    if (c->flags & 0x40) {
      if (c->fadeDelay > 0) {
        c->fadeDelay--;
      } else {
        c->flags &= ~0x70;
        lbl_8064A548[0].flags |= 0x84;
      }
    } else {
      c->fadeDelay = 0;
    }
    if (!(c->flags & 2)) {
      r->dest = c->buffer + (c->half << 13);
      if (lbl_8064A528.usedCount >= 4) {
        lbl_8064DC58 = 0;
        lbl_8064DC5C -= 0x1000;
        memset(r->dest, 0, 0x4000);
        DCFlushRange(r->dest, 0x4000);
        fn_80237EAC(c->stid, c->half);
        OSSetThreadPriority(&lbl_8064A1F8, 8);
      } else if (((c->flags == 0) & 1) || (c->flags & 0x10)) {
        if (!(c->flags & 0x10)) {
          c->flags |= 2;
        }
        memset(r->dest, 0, 0x4000);
        DCFlushRange(r->dest, 0x4000);
        fn_80237EAC(c->stid, c->half);
      } else {
        r->type = user;
        r->size = 0x2000;
        r->queue = NULL;
        r->half = c->half;
        r->stid = c->stid;
        r->callback = fn_80237EAC;
        memset(r->dest, 0, 0x4000);
        DCFlushRange(r->dest, 0x4000);
        if (lbl_8064A528.usedCount > 1) {
          OSSetThreadPriority(&lbl_8064A1F8, 8);
        } else {
          OSSetThreadPriority(&lbl_8064A1F8, lbl_8064DC6C);
        }
        OSSendMessage(&lbl_8064A528, r, 1);
      }
      c->half ^= 1;
    }
  }
  return 0;
}

void* fn_80236D60(void* arg) {
  OSMessage msg;
  StreamCommand* cmd;
  void* buffer;

  for (;;) {
    OSReceiveMessage(&lbl_806481D8, &msg, 1);
    cmd = msg;
    switch (cmd->type) {
    case 3:
      buffer = cmd->buffer;
      lbl_8064DC4C = 0;
      fn_80236AF8(buffer, cmd->queue);
      fn_80237224(cmd->name);
      break;
    }
  }
}

int fn_80236D30(void) {
  if (lbl_8064A548[0].flags || lbl_8064A548[1].flags) {
    return 1;
  }
  return 0;
}

u8* fn_80236AF8(u8* buffer, OSMessageQueue* queue) {
  int i;
  BOOL enabled;

  fn_801ADCF8();
  VIWaitForRetrace();
  while (OSReceiveMessage(&lbl_8064A528, NULL, 0)) {
  }
  for (i = 0; i < 2; i++) {
    lbl_8064A548[i].buffer = (s16*)buffer;
    buffer += 0x8000;
    lbl_8064A548[i].half = 0;
    lbl_8064A548[i].lastHalf = 0;
    lbl_8064A548[i].vol = 0;
    lbl_8064A548[i].curVol = 0;
    lbl_8064A548[i].flags = 1;
    memset(lbl_8064A548[i].buffer, 0, 0x8000);
    DCFlushRange(lbl_8064A548[i].buffer, 0x8000);
  }
  enabled = OSDisableInterrupts();
  if (lbl_8064D294 == 0) {
    lbl_8064A548[0].pan = 0x40;
    lbl_8064A548[1].pan = 0x40;
  } else {
    lbl_8064A548[0].pan = 0;
    lbl_8064A548[1].pan = 0x7f;
  }
  lbl_8064A548[0].stid = sndStreamAllocEx(0xff, lbl_8064A548[0].buffer, 0x4000, 44100, 0, lbl_8064A548[0].pan, 0, 0,
                                          0, 0, 0x30000, fn_80236DC4, 0, NULL);
  lbl_8064A548[1].stid = sndStreamAllocEx(0xff, lbl_8064A548[1].buffer, 0x4000, 44100, 0, lbl_8064A548[1].pan, 0, 0,
                                          0, 0, 0x30000, fn_80236DC4, 1, NULL);
  OSRestoreInterrupts(enabled);
  lbl_8064DC78 = 4000;
  for (i = 0; i < 2; i++) {
    lbl_806480B8[i].type = i;
    lbl_806480B8[i].size = 0x2000;
    lbl_806480B8[i].dest = lbl_8064A548[i].buffer;
    lbl_806480B8[i].queue = queue;
    lbl_806480B8[i].half = lbl_8064A548[i].half;
    lbl_806480B8[i].stid = lbl_8064A548[i].stid;
    lbl_806480B8[i].callback = fn_80237EAC;
    OSSendMessage(&lbl_8064A528, &lbl_806480B8[i], 1);
    lbl_8064A548[i].half ^= 1;
    lbl_8064A548[i].vol = 0;
    lbl_8064A548[i].curVol = 0;
  }
  return buffer;
}

void fn_80236A1C(void) {
  while (lbl_8064DC34 == 2) {
    VIWaitForRetrace();
  }
  if (lbl_8064DC38 == 0 && lbl_8064DC34 == 1) {
    OSReceiveMessage(&lbl_80648198, NULL, 1);
    OSReceiveMessage(&lbl_80648198, NULL, 1);
    if (lbl_8064DC38 == 0) {
      lbl_8064A548[0].vol = 0;
      lbl_8064A548[1].vol = 0;
      lbl_8064A548[0].curVol = 0;
      lbl_8064A548[1].curVol = 0;
      lbl_8064A548[0].flags |= 4;
      lbl_8064A548[1].flags |= 4;
      lbl_8064DC58 = 0;
      lbl_8064DC5C = -4000;
      sndStreamActivate(lbl_8064A548[0].stid);
      sndStreamActivate(lbl_8064A548[1].stid);
      lbl_8064DC38 = 1;
    }
  }
}

void fn_80236914(void) {
  if (lbl_8064DC38) {
    sndStreamDeactivate(lbl_8064A548[0].stid);
    sndStreamDeactivate(lbl_8064A548[1].stid);
    lbl_8064A548[0].vol = 0;
    lbl_8064A548[1].vol = 0;
    SetVolume(&lbl_8064A548[0]);
    SetVolume(&lbl_8064A548[1]);
  }
}

void fn_802368A8(void) {
  if (lbl_8064DC38) {
    lbl_8064A548[0].flags |= 0x40;
    lbl_8064A548[1].flags |= 0x40;
    lbl_8064A548[0].fadeDelay = 20;
    lbl_8064A548[1].fadeDelay = 20;
    sndStreamActivate(lbl_8064A548[0].stid);
    sndStreamActivate(lbl_8064A548[1].stid);
  }
}

void fn_802367B0(char* name, void* mem) {
  void* buffer;

  lbl_8064DC44 = 0;
  lbl_8064DC40 = lbl_8064C64C;
  if (lbl_8064DC38 == 0 && lbl_8064DC34 == 0) {
    OSInitMessageQueue(&lbl_8064A528, lbl_8064A508, 8);
    fn_8022F700(mem, 0x3C000);
    fn_8022F75C();
    buffer = fn_8022F780(0x10000);
    fn_8022F854(fn_8022F780(0x4C00), 0x4C00);
    lbl_8064805C.type = 3;
    strcpy(lbl_8064805C.name, name);
    lbl_8064805C.queue = &lbl_80648198;
    lbl_8064805C.buffer = buffer;
    OSSendMessage(&lbl_806481D8, &lbl_8064805C, 1);
    lbl_8064DC34 = 2;
    lbl_8064DC70 = 0;
  } else {
    lbl_8064DC38;
  }
}

int fn_802365C0(void) { return lbl_8064DC38 == 1; }

int fn_802365D4(void) {
  int pos;
  int frame;

  pos = lbl_8064DC58 + lbl_8064DC5C;
  frame = pos / 735;
  if (lbl_8064DC44 == pos && fn_802365C0() != 0 && lbl_8064DC40 + 30 < lbl_8064C64C) {
    fn_80236914();
    fn_802368A8();
  }
  if (lbl_8064DC44 != pos) {
    lbl_8064DC40 = lbl_8064C64C;
  }
  lbl_8064DC44 = pos;
  return frame;
}

int fn_8023657C(void) {
  if (fn_802365C0() != 0 && (lbl_8064A548[0].fadeDelay != 0 || lbl_8064A548[1].fadeDelay != 0)) {
    return 1;
  }
  return 0;
}

void fn_802364B8(int restart) {
  fn_80237D84();
  lbl_8064DC68 = 1;
  if (restart == 0) {
    OSInitMessageQueue(&lbl_8064A528, lbl_8064A508, 8);
    OSInitMessageQueue(&lbl_806481D8, lbl_806481B8, 8);
    OSInitMessageQueue(&lbl_80648198, lbl_8064DC60, 2);
    OSCreateThread(&lbl_8064A1F8, fn_80236D60, NULL, lbl_806481F8 + 0x2000, 0x2000, 0x12, 1);
    OSResumeThread(&lbl_8064A1F8);
  }
}
