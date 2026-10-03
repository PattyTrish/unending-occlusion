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

 function: libvorbis codec headers
 last mod: $Id: codec_internal.h,v 1.12 2001/12/20 01:00:26 segher Exp $

 ********************************************************************/

#ifndef _V_CODECI_H_
#define _V_CODECI_H_

#include "envelope.h"
#include "codebook.h"

#define BLOCKTYPE_IMPULSE    0
#define BLOCKTYPE_PADDING    1
#define BLOCKTYPE_TRANSITION 0 
#define BLOCKTYPE_LONG       1

typedef struct vorbis_block_internal{
  float  **pcmdelay;  /* this is a pointer into local storage */ 
  float  ampmax;
  int    blocktype;

  ogg_uint32_t *packet_markers;
} vorbis_block_internal;

typedef void vorbis_look_time;
typedef void vorbis_look_mapping;
typedef void vorbis_look_floor;
typedef void vorbis_look_residue;
typedef void vorbis_look_transform;

/* mode ************************************************************/
typedef struct {
  int blockflag;
  int windowtype;
  int transformtype;
  int mapping;
} vorbis_info_mode;

typedef void vorbis_info_time;
typedef void vorbis_info_floor;
typedef void vorbis_info_residue;
typedef void vorbis_info_mapping;

#include "psy.h"
#include "bitrate.h"

typedef struct backend_lookup_state {
  /* local lookup storage */
  envelope_lookup        *ve; /* envelope lookup */    
  /* Silicon Knights: no window[2][2][2] here and nothing after mode (sizeof 0x1C); ve and psy_g_look
     keep their slots (inferred: only the offsets of transform, fullbooks, modebits, mode are proven) */
  vorbis_look_transform **transform[2];    /* block, type */
  codebook               *fullbooks;
  vorbis_look_psy_global *psy_g_look;

  /* backend lookups are tied to the mode, not the backend or naked mapping */
  int                     modebits;
  vorbis_look_mapping   **mode;

} backend_lookup_state;

/* high level configuration information for setting things up
   step-by-step with the detaile vorbis_encode_ctl interface */

typedef struct highlevel_block {
  double tone_mask_quality;
  double tone_peaklimit_quality;

  double noise_bias_quality;
  double noise_compand_quality;

  double ath_quality;

} highlevel_block;

typedef struct highlevel_encode_setup {
  double base_quality;       /* these have to be tracked by the ctl */
  double base_quality_short; /* interface so that the right books get */
  double base_quality_long;  /* chosen... */

  int short_block_p;
  int long_block_p;
  int impulse_block_p;

  int stereo_couple_p;
  int stereo_backfill_p;
  int residue_backfill_p;

  int    stereo_point_dB;
  double stereo_point_kHz[2];
  double lowpass_kHz[2];

  double ath_floating_dB;
  double ath_absolute_dB;

  double amplitude_track_dBpersec;
  double trigger_quality;

  highlevel_block blocktype[4]; /* impulse, padding, trans, long */
  
} highlevel_encode_setup;

/* codec_setup_info contains all the setup information specific to the
   specific compression/decompression mode in progress (eg,
   psychoacoustic settings, channel setup, options, codebook
   etc).  
*********************************************************************/

typedef struct codec_setup_info {

  /* Vorbis supports only short and long blocks, but allows the
     encoder to choose the sizes */

  long blocksizes[2];

  /* modes are the primary means of supporting on-the-fly different
     blocksizes, different channel mappings (LR or M/A),
     different residue backends, etc.  Each mode consists of a
     blocksize flag and a mapping (along with the mapping setup */

  int        modes;
  int        maps;
  int        times;
  int        floors;
  int        residues;
  int        books;
  /* SK: no psys (encode only): mode_param is at 0x20 */

  vorbis_info_mode       *mode_param[64];
  int                     map_type[64];
  vorbis_info_mapping    *map_param[64];
  int                     time_type[64];
  vorbis_info_time       *time_param[64];
  int                     floor_type[64];
  vorbis_info_floor      *floor_param[64];
  int                     residue_type[64];
  vorbis_info_residue    *residue_param[64];
  static_codebook        *book_param[256];

  /* SK: 0xB4 bytes after book_param (sizeof 0xDD4 in vorbis_info_init); keeping psy_g_param, passlimit and
     coupling_passes is the only set of whole fields that fits (inferred); psy_param, bi and hi are gone */
  vorbis_info_psy_global psy_g_param;

  int    passlimit[32];     /* iteration limit per couple/quant pass */
  int    coupling_passes;
} codec_setup_info;

extern vorbis_look_psy_global *_vp_global_look(vorbis_info *vi);
extern void _vp_global_free(vorbis_look_psy_global *look);

#endif
