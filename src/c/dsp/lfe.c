#include <lfe.h>

void build_lfe_from_stereo(L, R, LFE, frames, st, alpha, gain)
	const float *L;
	const float *R;
	float *LFE;
	size_t frames;
	LPState *st;
	float alpha;
	float gain;
{
	size_t i;

	for (i = 0; i < frames; i++) {
		float mono = 0.5f * (L[i] + R[i]);
		LFE[i] = mono;
	}

	lfe_lowpass_block(LFE, LFE, frames, st, alpha);

	for (i = 0; i < frames; i++) {
		LFE[i] *= gain;
	}
}
