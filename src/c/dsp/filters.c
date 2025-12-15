#include <filters.h>

void
lfe_lowpass_block(in, out, frames, st, alpha)
	float *in;
	float *out;
	size_t frames;
	LPState *st;
	float alpha;
{
	size_t i;

	float y_prev = st->y_prev;

	for (i = 0; i < frames; i++) {
		float x = in[i];
		float y = y_prev + alpha * (x - y_prev);
		out[i] = y;
		y_prev = y;
	}

	st->y_prev = y_prev;
}

void
highpass_block(in, out, frames, st, alpha)
	float *in;
	float *out;
	size_t frames;
	HPState *st;
	float alpha;
{
	size_t i;
	float y_prev = st->y_prev;

	for (i = 0; i < frames; i++) {
		float x = in[i];
		float y = y_prev + alpha * (x - y_prev);
		out[i] = x - y;
		y_prev = y;
	}

	st->y_prev = y_prev;
}
