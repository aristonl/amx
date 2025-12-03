#include <centre.h>

void
build_centre_from_stereo(L, R, C, frames, gain, subtract)
	float *L;
	float *R;
	float *C;
	size_t frames;
	float gain;
	float subtract;
{
	size_t i;

	for (i = 0; i < frames; i++) {
		float l = L[i];
		float r = R[i];

		float mid = 0.5f * (l + r);

		float sub = subtract * mid;

		L[i] = l - sub;
		R[i] = r - sub;
		C[i] = gain * mid;
	}
}
