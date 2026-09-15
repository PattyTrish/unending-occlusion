#ifndef MSL_CTYPE_H
#define MSL_CTYPE_H

void *luaM_realloc (lua_State *L, void *oldblock, lint32 size, const char *file, int line);

#define luaM_free(L, b)         luaM_realloc(L, (b), 0, __FILE__, __LINE__)
#define luaM_malloc(L, t)       luaM_realloc(L, NULL, (t), __FILE__, __LINE__)

#endif
