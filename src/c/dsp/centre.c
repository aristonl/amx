#include <centre.h>

void
build_centre_from_stereo(L, R, C, frames, gain)
	const float *L;
	const float *R;
	float *C;
	size_t frames;
	float gain;
{
	size_t i;

	for (i = 0; i < frames; i++) {
		float l = L[i];
		float r = R[i];

		float mid = 0.5f * (l + r);

		C[i] = gain * mid;
	}
}
