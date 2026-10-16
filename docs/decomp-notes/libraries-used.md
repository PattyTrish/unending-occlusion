# Libraries Used

## Lua

The [Lua 4.0.1 source](https://www.lua.org/source/4.0/) was used as a basis when it was clear that Lua file names were found in the binary. Upon further matching, it may be a tweaked version of 4.0.0.

One key thing is that SK seems to have their own implementation of `luaM_realloc` from `lmem.c`. It's not 100% clear on how or why this has happened yet.

## Dolphin/Runtime/MSL/MetroTRK

Since existing decomps in the community have already achieved 100% matches, I decided not to reinvent the wheel here, basing this particularly on the Melee and MP4 decomps, given the release date locality.

Dolphin SDK seems to be revision 52.

Deviations currently need a pass for understanding.
