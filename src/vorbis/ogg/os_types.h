/********************************************************************
 *                                                                  *
 * THIS FILE IS PART OF THE OggVorbis SOFTWARE CODEC SOURCE CODE.   *
 * USE, DISTRIBUTION AND REPRODUCTION OF THIS LIBRARY SOURCE IS     *
 * GOVERNED BY A BSD-STYLE SOURCE LICENSE INCLUDED WITH THIS SOURCE *
 * IN 'COPYING'. PLEASE READ THESE TERMS BEFORE DISTRIBUTING.       *
 *                                                                  *
 * THE OggVorbis SOURCE CODE IS (C) COPYRIGHT 1994-2001             *
 * by the XIPHOPHORUS Company http://www.xiph.org/                  *
 *                                                                  *
 ********************************************************************

 function: #ifdef jail to whip a few platforms into the UNIX ideal.
 last mod: $Id: os_types.h,v 1.9 2001/12/20 00:58:44 segher Exp $

 ********************************************************************/
#ifndef _OS_TYPES_H
#define _OS_TYPES_H

#ifdef __MWERKS__
/* Silicon Knights: an arena allocator (0x8022F780, name unknown) that can neither grow nor free a block.
   Inferred from framing.c: ogg_sync_buffer's realloc branch and ogg_sync_clear's free compile to nothing */
void *fn_8022F780(long size);
#define _ogg_malloc  fn_8022F780
/* _ogg_calloc: one call, no memset (sharedbook.c), and SK's macro is unparenthesized: info.c's
   _ogg_calloc(vc->comments+1, sizeof(...)) allocates comments + 4 bytes in ED */
#define _ogg_calloc(n, s) fn_8022F780(n * s)
#define _ogg_realloc(p, n) (p)
#define _ogg_free(p)
#else
/* make it easy on the folks that want to compile the libs with a
   different malloc than stdlib */
#define _ogg_malloc  malloc
#define _ogg_calloc  calloc
#define _ogg_realloc realloc
#define _ogg_free    free
#endif

#ifdef __MWERKS__

   /* Silicon Knights' GameCube build (inferred: none of the upstream platforms apply) */
   typedef short ogg_int16_t;
   typedef int ogg_int32_t;
   typedef unsigned int ogg_uint32_t;
   /* 32 bits in ED: ogg_stream_init allocates granule_vals as 4-byte elements and the struct ends at 0x40 */
   typedef long ogg_int64_t;

#elif defined(_WIN32)

#  ifndef __GNUC__
   /* MSVC/Borland */
   typedef __int64 ogg_int64_t;
   typedef __int32 ogg_int32_t;
   typedef unsigned __int32 ogg_uint32_t;
   typedef __int16 ogg_int16_t;
#  else
   /* Cygwin */
   #include <_G_config.h>
   typedef _G_int64_t ogg_int64_t;
   typedef _G_int32_t ogg_int32_t;
   typedef _G_uint32_t ogg_uint32_t;
   typedef _G_int16_t ogg_int16_t;
#  endif

#elif defined(__MACOS__)

#  include <sys/types.h>
   typedef SInt16 ogg_int16_t;
   typedef SInt32 ogg_int32_t;
   typedef UInt32 ogg_uint32_t;
   typedef SInt64 ogg_int64_t;

#elif defined(__MACOSX__) /* MacOS X Framework build */

#  include <sys/types.h>
   typedef int16_t ogg_int16_t;
   typedef int32_t ogg_int32_t;
   typedef u_int32_t ogg_uint32_t;
   typedef int64_t ogg_int64_t;

#elif defined(__BEOS__)

   /* Be */
#  include <inttypes.h>

#elif defined (__EMX__)

   /* OS/2 GCC */
   typedef short ogg_int16_t;
   typedef int ogg_int32_t;
   typedef unsigned int ogg_uint32_t;
   typedef long long ogg_int64_t;

#else

#  include <sys/types.h>
#  include <ogg/config_types.h>

#endif

#endif  /* _OS_TYPES_H */
