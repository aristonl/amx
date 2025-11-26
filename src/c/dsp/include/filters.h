#ifndef AMX_FILTERS_H
#define AMX_FILTERS_H

#include <stddef.h>

typedef struct {
	float y_prev;
} LPState;

void lfe_lowpass_block(float *in, float *out, size_t frames, LPState *st, float alpha);

#endif
