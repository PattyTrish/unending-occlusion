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

 function: window functions
 last mod: $Id: window.c,v 1.15 2001/12/20 01:00:30 segher Exp $

 ********************************************************************/

#include <stdlib.h>
#include <math.h>
#include "os.h"
#include "misc.h"

/* Silicon Knights: one sample per call instead of a calloc'd table (mapping0_inverse multiplies
   each sample by it) */
float _vorbis_window(int type,int i,int window,int left,int right){
  float ret=0.f;

  switch(type){
  case 0:
    /* The 'vorbis window' (window 0) is sin(sin(x)*sin(x)*2pi) */
    {
      int leftbegin=window/4-left/2;
      int rightbegin=window-window/4-right/2;

      if(i>=leftbegin && i<left+leftbegin){
	float x=(i-leftbegin+.5f)/left*M_PI/2.;
	x=sin(x);
	x*=x;
	x*=M_PI/2.f;
	x=sin(x);
	ret=x;
      }else if(i>=rightbegin && i<rightbegin+right){
	float x=(right-(i-rightbegin)-.5f)/right*M_PI/2.;
	x=sin(x);
	x*=x;
	x*=M_PI/2.f;
	x=sin(x);
	ret=x;
      }else if(i>=leftbegin+left && i<rightbegin)
	ret=1.f;
    }
    break;
  }
  return(ret);
}
