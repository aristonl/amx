/*
 * Copyright 2025 Aris Lorenzo. All rights reserved.
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <wav.h>
#include <filters.h>
#include <lfe.h>

static float
compute_alpha(fc, fs)
	float fc;
	float fs;
{
    // α = 1 - exp(-2π fc / fs)
    float x = -2.0f * (float)M_PI * fc / fs;
    return 1.0f - expf(x);
}

int
main(argc, argv)
	int argc;
	char *argv[];
{
	int i;

	const char *in_path = (argc > 1) ? argv[1] : "input.wav";
	const char *out_path = (argc > 2) ? argv[2] : "output_2_1.wav";

	struct wav_info in_info;
	struct wav_reader *wr = wav_open_read(in_path, &in_info);
	if (!wr) {
		fprintf(stderr, "Failed to open input WAV: %s\n", in_path);
		return 1;
	}

	if (in_info.channels != 2) {
		fprintf(stderr, "This only works with stereo input, not %u\n",
				in_info.channels);
		wav_close_read(wr);
		return 1;
	}

	struct wav_info out_info = in_info;
	out_info.channels = 3;
	out_info.total_frames = 0;

	struct wav_writer *ww = wav_open_write(out_path, &out_info);

	if (!ww) {
		fprintf(stderr, "Failed to open output WAV: %s\n", out_path);
		wav_close_read(wr);
		return 1;
	}

	size_t max_frames = 1024;
	float *buf[3];

	buf[0] = malloc(sizeof(float) * max_frames);
	buf[1] = malloc(sizeof(float) * max_frames);
	buf[2] = malloc(sizeof(float) * max_frames);

	if (!buf[0] || !buf[1] || !buf[2]) {
		fprintf(stderr, "Allocation failed\n");
		for (i = 0; i < 3; i++)
			free(buf[i]);
		wav_close_read(wr);
		wav_close_write(ww);
		return 1;
	}

	float *L = buf[0];
	float *R = buf[1];
	float *LFE = buf[2];

	LPState lfe_state = {0};
	float fc = 120.f;
	float fs = (float)in_info.sample_rate;
	float alpha = compute_alpha(fc, fs);
	float gain = 1.0f;

	for (;;) {
		size_t frames = wav_read_frames_f32(wr, buf, max_frames);
		if (frames == 0)
			break;

		build_lfe_from_stereo(L, R, LFE, frames, &lfe_state, alpha, gain);
		wav_write_frames_f32(ww, buf, frames);
	}

	for (i = 0; i < 3; i++)
		free(buf[i]);

	wav_close_read(wr);
	wav_close_write(ww);

	return 0;
}
