/*
 * wav.c - WAV codec library for matrix. Can also be used as a standalone.
 */

/*
 * Copyright (c) 2025 Aris Lorenzo.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 * 3.  Neither the name of Ariston Lorenzo nor the names of its contributors
 *     may be used to endorse or promote products derived from this software
 *     without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY ARISTON LORENZO AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL ARISTON LORENZO OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "wav.h"

struct wav_reader {
	FILE *fp;
	struct wav_info info;
	long data_offset;
	uint32_t data_size;
	uint64_t frames_read;
};

static int
read_u16_le(fp, out)
	FILE *fp;
	uint16_t *out;
{
	unsigned char buf[2];

	if (fread(buf, 1, 2, fp) != 2)
		return -1;

	*out = (uint16_t)(buf[0] | (buf[1] << 8));
	return 0;
}

static int
read_u32_le(fp, out)
	FILE *fp;
	uint32_t *out;
{
	unsigned char buf[4];

	if (fread(buf, 1, 4, fp) != 4)
		return -1;

	*out = (uint32_t)(buf[0] | (buf[1] << 8) | (buf[2] << 16) | (buf[3] << 24));
	return 0;
}

static int
parse_header(fp, info, data_offset, data_size)
	FILE *fp;
	struct wav_info *info;
	long *data_offset;
	uint32_t *data_size;
{
	char riff_id[4];
	char wave_id[4];
	uint32_t riff_size;
	int have_fmt = 0;
	int have_data = 0;

	memset(info, 0, sizeof(*info));
	*data_offset = 0;
	*data_size = 0;

	if (fread(riff_id, 1, 4, fp) != 4)
		return -1;
	if (read_u32_le(fp, &riff_size) != 0)
		return -1;
	if (fread(wave_id, 1, 4, fp) != 4)
		return -1;

	if (memcmp(riff_id, RIFF_SIGNATURE, 4) != 0)
		return -1;
	if (memcmp(wave_id, WAVE_SIGNATURE, 4) != 0)
		return -1;

	for (;;) {
		char chunk_id[4];
		uint32_t chunk_size;
		long next_chunk_pos;

		if (fread(chunk_id, 1, 4, fp) != 4)
			break;

		if (read_u32_le(fp, &chunk_size) != 0)
			return -1;

		next_chunk_pos = ftell(fp);
		if (next_chunk_pos < 0)
			return -1;

		if (memcmp(chunk_id, FMT_SIGNATURE, 4) == 0) {
			uint16_t audio_format;
			uint16_t channels;
			uint32_t sample_rate;
			uint32_t byte_rate;
			uint16_t block_align;
			uint16_t bits_per_sample;
			
			if (chunk_size < 16)
				return -1;

			if (read_u16_le(fp, &audio_format) != 0)
				return -1;
			if (read_u16_le(fp, &channels) != 0)
				return -1;
			if (read_u32_le(fp, &sample_rate) != 0)
				return -1;
			if (read_u32_le(fp, &byte_rate) != 0)
				return -1;
			if (read_u16_le(fp, &block_align) != 0)
				return -1;
			if (read_u16_le(fp, &bits_per_sample) != 0)
				return -1;

			if (chunk_size > 16) {
				long skip = (long)(chunk_size - 16);
				if (fseek(fp, skip, SEEK_CUR) != 0)
					return -1;
			}

			info->audio_format = audio_format;
			info->channels = channels;
			info->sample_rate = sample_rate;
			info->byte_rate = byte_rate;
			info->block_align = block_align;
			info->bitdepth = bits_per_sample;

			have_fmt = 1;
		} else if (memcmp(chunk_id, DATA_SIGNATURE, 4) == 0) {
			*data_offset = ftell(fp);
			*data_size = chunk_size;

			if (fseek(fp, (long)chunk_size, SEEK_CUR) != 0)
				return -1;

			have_data = 1;
		} else {
			if (fseek(fp, (long)chunk_size, SEEK_CUR) != 0)
				return -1;
		}

		if (chunk_size & 1) {
			if (fseek(fp, 1L, SEEK_CUR) != 0)
				return -1;
		}
	}

	if (!have_fmt || !have_data)
		return -1;

	if (info->block_align == 0)
		return -1;

	info->total_frames = (uint64_t)(*data_size / info->block_align);

	return 0;
}

int
wav_read_header(path, info)
	const char *path;
	struct wav_info *info;
{
	FILE *fp;
	long data_offset;
	uint32_t data_size;
	int ret;

	if (path == NULL || info == NULL)
		return -1;

	fp = fopen(path, "rb"); // note to self: always include the b
	if (fp == NULL)
		return -1;

	ret = parse_header(fp, info, &data_offset, &data_size);

	fclose(fp);
	return ret;
}

size_t
wav_read_frames_f32(wr, out_channels, max_frames)
	struct wav_reader *wr;
	float **out_channels;
	size_t max_frames;
{
	uint16_t ch;
	uint16_t bps;
	uint16_t bytes_per_sample;
	size_t block_align;
	uint64_t frames_left;
	size_t frames_to_read;
	size_t total_bytes;
	unsigned char *buf;
	size_t read_bytes;
	size_t n;
	uint16_t c;

	if (wr == NULL || out_channels == NULL || max_frames == 0)
		return 0;

	ch = wr->info.channels;
	bps = wr->info.bitdepth;
	bytes_per_sample = (uint16_t)(bps / 8);
	block_align = wr->info.block_align;

	if (bps != 16 || wr->info.audio_format != 1) // TODO: only 16-bit is supported, get 24-bit working
		return 0;

	if (block_align != (size_t)(ch * bytes_per_sample))
		return 0;

	frames_left = wr->info.total_frames - wr->frames_read;
	if (frames_left == 0)
		return 0;

	frames_to_read = max_frames;
	if (frames_to_read > frames_left)
		frames_to_read = (size_t)(frames_left);

	total_bytes = frames_to_read * block_align;
	buf = malloc(total_bytes);
	if (buf == NULL)
		return 0;

	read_bytes = fread(buf, 1, total_bytes, wr->fp);
	if (read_bytes != total_bytes) {
		free(buf);
		return 0;
	}

	for (n = 0; n < frames_to_read; ++n) {
		size_t frame_offset = n * block_align;
		for (c = 0; c < ch; ++c) {
			size_t byte_index = frame_offset + c * bytes_per_sample;
			int16_t s;

			s = (int16_t)(buf[byte_index] | (buf[byte_index + 1] << 8));

			out_channels[c][n] = (float)s / 32768.0f;
		}
	}

	wr->frames_read += frames_to_read;
	free(buf);

	return frames_to_read;
}

struct wav_reader *
wav_open_read(path, info)
	const char *path;
	struct wav_info *info;
{
	FILE *fp;
	struct wav_reader *wr;
	long data_offset;
	uint32_t data_size;

	if (path == NULL || info == NULL)
		return NULL;

	fp = fopen(path, "rb");
	if (fp == NULL)
		return NULL;

	if (parse_header(fp, info, &data_offset, &data_size) != 0) {
		fclose(fp);
		return NULL;
	}

	wr = malloc(sizeof(*wr));
	if (wr == NULL) {
		fclose(fp);
		return NULL;
	}

	wr->fp = fp;
	wr->info = *info;
	wr->data_offset = data_offset;
	wr->data_size = data_size;
	wr->frames_read = 0;

	if (fseek(fp, data_offset, SEEK_SET) != 0) {
		fclose(fp);
		free(wr);
		return NULL;
	}

	return wr;
}

void
wav_close_read(wr)
	struct wav_reader *wr;
{
	if (wr == NULL)
		return;

	if (wr->fp != NULL)
		fclose(wr->fp);

	free(wr);
}

struct wav_writer {
	FILE *fp;
	struct wav_info info;
	long riff_size_pos;
	long data_size_pos;
	uint64_t frames_written;
};

static void
write_u16_le(fp, value)
	FILE *fp;
	uint16_t value;
{
	unsigned char buf[2];

	buf[0] = (unsigned char)(value & 0xFF);
	buf[1] = (unsigned char)((value >> 8) & 0xFF);
	fwrite(buf, 1, 2, fp);
}

static void
write_u32_le(fp, value)
	FILE *fp;
	uint32_t value;
{
	unsigned char buf[4];

	buf[0] = (unsigned char)(value & 0xFF);
	buf[1] = (unsigned char)((value >> 8) & 0xFF);
	buf[2] = (unsigned char)((value >> 16) & 0xFF);
	buf[3] = (unsigned char)((value >> 24) & 0xFF);
	fwrite(buf, 1, 4, fp);
}

struct wav_writer *
wav_open_write(path, info)
	const char *path;
	const struct wav_info *info;
{
	struct wav_writer *ww;
	FILE *fp;
	uint16_t bytes_per_sample;
	uint32_t byte_rate;
	uint16_t block_align;
	long pos;

	if (path == NULL || info == NULL)
		return NULL;

	if (info->audio_format != 1)
		return NULL;

	if (info->channels == 0)
		return NULL;

	if (info->bitdepth != 16 &&
	    info->bitdepth != 24)
		return NULL;

	bytes_per_sample = (uint16_t)(info->bitdepth / 8);
	block_align = (uint16_t)(info->channels * bytes_per_sample);
	byte_rate = info->sample_rate * block_align;

	fp = fopen(path, "wb");
	if (fp == NULL)
		return NULL;

	ww = malloc(sizeof(*ww));
	if (ww == NULL) {
		fclose(fp);
		return NULL;
	}

	memset(ww, 0, sizeof(*ww));
	ww->fp = fp;
	ww->info = *info;
	ww->info.block_align = block_align;
	ww->info.byte_rate = byte_rate;
	ww->frames_written = 0;

	fwrite(RIFF_SIGNATURE, 1, 4, fp);
	
	pos = ftell(fp);
	if (pos < 0) {
		fclose(fp);
		free(ww);
		return NULL;
	}
	ww->riff_size_pos = pos;

	write_u32_le(fp, 0); /* placeholder, will be file_size - 8 */

	fwrite(WAVE_SIGNATURE, 1, 4, fp);

	fwrite(FMT_SIGNATURE, 1, 4, fp);
	write_u32_le(fp, 16); /* PCM fmt chunk size */
	write_u16_le(fp, 1);  /* PCM */
	write_u16_le(fp, ww->info.channels);
	write_u32_le(fp, ww->info.sample_rate);
	write_u32_le(fp, ww->info.byte_rate);
	write_u16_le(fp, ww->info.block_align);
	write_u16_le(fp, ww->info.bitdepth);

	fwrite(DATA_SIGNATURE, 1, 4, fp);

	pos = ftell(fp);
	if (pos < 0) {
		fclose(fp);
		free(ww);
		return NULL;
	}
	ww->data_size_pos = pos;

	write_u32_le(fp, 0); /* placeholder for data size */

	return ww;
}

size_t
wav_write_frames_f32(ww, in_channels, frames)
	struct wav_writer *ww;
	float **in_channels;
	size_t frames;
{
	FILE *fp;
	uint16_t ch;
	uint16_t bits;
	uint16_t bytes_per_sample;
	size_t n, c;
	size_t frames_written = 0;

	if (ww == NULL || in_channels == NULL || frames == 0)
		return 0;

	fp = ww->fp;
	ch = ww->info.channels;
	bits = ww->info.bitdepth;
	bytes_per_sample = (uint16_t)(bits / 8);

	if (bits != 16 && bits != 24)
		return 0;

	for (n = 0; n < frames; ++n) {
		for (c = 0; c < ch; ++c) {
			float x = in_channels[c][n];

			if (x > 1.0f)
				x = 1.0f;
			else if (x < -1.0f)
				x = -1.0f;

			if (bits == 16) {
				int32_t s = (int32_t)(x * 32767.0f);
				uint16_t u = (uint16_t)(s & 0xFFFF);
				write_u16_le(fp, u);
			} else { /* 24-bit PCM */
				int32_t s = (int32_t)(x * 8388607.0f); /* 2^23 - 1 */
				unsigned char b[3];

				b[0] = (unsigned char)( s        & 0xFF);
				b[1] = (unsigned char)((s >> 8)  & 0xFF);
				b[2] = (unsigned char)((s >> 16) & 0xFF);
				fwrite(b, 1, 3, fp);
			}
		}
		frames_written++;
	}

	ww->frames_written += frames_written;
	return frames_written;
}

void
wav_close_write(ww)
	struct wav_writer *ww;
{
	uint32_t data_bytes;
	uint32_t riff_size;
	long file_end;

	if (ww == NULL)
		return;

	if (ww->fp != NULL) {
		if (fseek(ww->fp, 0, SEEK_END) == 0) {
			file_end = ftell(ww->fp);
			if (file_end >= 0) {
				data_bytes = (uint32_t)(file_end - (ww->data_size_pos + 4));
				riff_size  = (uint32_t)(file_end - 8);

				if (fseek(ww->fp, ww->data_size_pos, SEEK_SET) == 0)
					write_u32_le(ww->fp, data_bytes);

				if (fseek(ww->fp, ww->riff_size_pos, SEEK_SET) == 0)
					write_u32_le(ww->fp, riff_size);
			}
		}

		fclose(ww->fp);
		ww->fp = NULL;
	}

	free(ww);
}

int
wav_verify_header(file)
	char *file;
{
	struct wav_info info;
	
	if (file == NULL)
		return -1;

	if (wav_read_header(file, &info) == 0)
		return 0;

	return 1;
}
