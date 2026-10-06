/* Silicon Knights' arena allocator for Ogg Vorbis (0x8022F700..0x8022F93C). No strings name this file or its
   functions: the file name is a placeholder and the symbols keep their addresses.

   Pool 1 (_ogg_malloc/_ogg_calloc): each block has an 8-byte header in front of it, a magic word and the block's
   size including the header. Allocated blocks are marked 0x0A1BEEF0; the block after the last one is marked
   0xBAADF00D. Pool 2 (alloca/_vorbis_block_alloc) is a bump allocator, reset per packet by vorbis_synthesis.
   Both track the lowest free count seen. */

#include <string.h>

char *lbl_8064DC2C;
long lbl_8064DC28;
char *lbl_8064DC24;
long lbl_8064DC20;
char *lbl_8064DC1C;
long lbl_8064DC18;
char *lbl_8064DC14;
long lbl_8064DC10;
long lbl_8064DC0C;
long lbl_8064DC08;

void fn_8022F8FC(void) {
  unsigned long *p = (unsigned long *)(lbl_8064DC2C + 0x18);

  while (p[0] == 0xBAADF00D || p[0] == 0x0A1BEEF0) {
    if (p[0] == 0xBAADF00D) return;
    p = (unsigned long *)((char *)p + p[1]);
  }
}

void fn_8022F8F8(void) {
}

void *fn_8022F8C4(long size) {
  char *ret;

  size = (size + 3) & ~3;
  ret = lbl_8064DC14;
  lbl_8064DC10 -= size;
  lbl_8064DC14 += size;
  if (lbl_8064DC10 < lbl_8064DC08) lbl_8064DC08 = lbl_8064DC10;
  return ret;
}

void fn_8022F890(void) {
  lbl_8064DC14 = lbl_8064DC1C;
  lbl_8064DC10 = lbl_8064DC18;
  memset(lbl_8064DC1C, 0, lbl_8064DC18);
}

void fn_8022F854(void *base, long size) {
  lbl_8064DC1C = base;
  lbl_8064DC18 = size;
  lbl_8064DC14 = base;
  lbl_8064DC10 = size;
  lbl_8064DC08 = size;
  memset(base, 0, size);
}

void *fn_8022F780(long size) {
  char *ret;
  unsigned long *end;

  fn_8022F8FC();
  size = (size + 8 + 3) & ~3;
  ret = lbl_8064DC24;
  lbl_8064DC20 -= size;
  lbl_8064DC24 += size;
  if (lbl_8064DC20 < lbl_8064DC0C) lbl_8064DC0C = lbl_8064DC20;
  ((unsigned long *)ret)[-1] = size;
  ((unsigned long *)ret)[-2] = 0x0A1BEEF0;
  end = (unsigned long *)lbl_8064DC24;
  end[-2] = 0xBAADF00D;
  end[-1] = 0;
  fn_8022F8FC();
  return ret;
}

void fn_8022F75C(void) {
  lbl_8064DC24 = lbl_8064DC2C + 0x20;
  lbl_8064DC20 = lbl_8064DC28;
  ((unsigned long *)lbl_8064DC2C)[6] = 0xBAADF00D;
}

void fn_8022F700(void *base, long size) {
  lbl_8064DC2C = base;
  lbl_8064DC28 = size;
  memset(base, 0, size);
  lbl_8064DC20 = size;
  lbl_8064DC24 = lbl_8064DC2C + 0x20;
  lbl_8064DC0C = size;
  ((unsigned long *)lbl_8064DC2C)[6] = 0xBAADF00D;
}
