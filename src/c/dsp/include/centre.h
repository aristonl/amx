#ifndef AMX_CENTRE_H
#define AMX_CENTRE_H

#include <stddef.h>

void build_centre_from_stereo(const float *L, const float *R, float *C, size_t frames, float gain);

#endif
