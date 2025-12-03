#include <math.h>

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

	const float eps = 1e-6f;
	const float steer_f = 1.5f;

	for (i = 0; i < frames; i++) {
		float l = L[i];
		float r = R[i];

		float mid = 0.5f * (l + r);
		float side = 0.5f * (l + r);

		float mid_abs = fabsf(mid);
		float side_abs = fabsf(side);

		float ratio = mid_abs / (side_abs + eps);
		float steer = ratio * steer_f;
		if (steer > 1.0f) steer = 1.0f;

		float sub = subtract * steer * mid;

		L[i] = l - sub;
		R[i] = r - sub;
		C[i] = gain * mid;
	}
}
