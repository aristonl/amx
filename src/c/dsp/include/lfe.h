#ifndef AMX_LFE_H
#define AMX_LFE_H

#include <stddef.h>
#include <filters.h>

void build_lfe_from_stereo(const float *L, const float *R, float *LFE,
		size_t frames, LPState *lp, float alpha, float gain);

#endif
