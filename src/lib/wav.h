#ifndef LIBWAV_WAV_H
#define LIBWAV_WAV_H

#include <stdint.h>
#include <stddef.h>

#define RIFF_SIGNATURE "RIFF"
#define WAVE_SIGNATURE "WAVE"
#define FMT_SIGNATURE "fmt "
#define DATA_SIGNATURE "data"

#define WAV_MAX_CHANNELS 12 // only supporting up to 7.1.4 (enough for atmos)

struct wav_info {
	uint16_t audio_format;
	uint16_t channels;
	uint32_t sample_rate;
	uint32_t byte_rate;
	uint16_t block_align;
	uint16_t bitdepth;
	uint64_t total_frames;
};

struct wav_reader;
struct wav_writer;

int wav_read_header(const char* path, struct wav_info *info);

struct wav_reader *wav_open_read(const char *path, struct wav_info *info);

size_t wav_read_frames_f32(struct wav_reader *wr, float **out_channels, size_t max_frames);

void wav_close_read(struct wav_reader *wr);

struct wav_writer *wav_open_write(const char *path, const struct wav_info *info);

size_t wav_write_frames_f32(struct wav_writer *ww, float **in_channels, size_t frames);

void wav_close_write(struct wav_writer *ww);

int wav_verify_header(char *file);

#endif /* LIBWAV_WAV_H */
