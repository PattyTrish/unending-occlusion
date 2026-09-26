#include "PowerPC_EABI_Support/Msl/MSL_C/MSL_Common/locale.h"
#include "PowerPC_EABI_Support/Msl/MSL_C/MSL_Common/limits.h"
#include "string.h"

struct lconv __lconv = {
	".", "", "", "", "", "", "", "", "",
	CHAR_MAX, CHAR_MAX, CHAR_MAX, CHAR_MAX, CHAR_MAX, CHAR_MAX, CHAR_MAX,
	"",
	CHAR_MAX, CHAR_MAX, CHAR_MAX, CHAR_MAX, CHAR_MAX, CHAR_MAX, CHAR_MAX,
};

/* Not linked, but kept: its "C" survives in __lconv's string pool */
char* setlocale(int category, const char* locale)
{
	if (locale == 0 || strcmp(locale, "C") == 0 || strcmp(locale, "") == 0)
		return "C";

	return 0;
}

/*
struct lconv* localeconv(void)
{
	return &__lconv;
}
*/
