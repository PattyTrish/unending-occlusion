#include "PowerPC_EABI_Support/Msl/MSL_C/MSL_Common/math.h"

float powf(float x, float y) {
    return (float) pow(x, y);
}

float ceilf(float x) {
    return (float) ceil(x);
}

float floorf(float x) {
    return (float) floor(x);
}
