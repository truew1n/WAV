#ifndef LIB_WAVE_H
#define LIB_WAVE_H

#ifdef _cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

// Main Chunks
#define BE_RIFF_CHUNK_ID 0x52494646
#define BE_WAVE_CHUNK_ID 0x57415645
#define BE_FMT_CHUNK_ID 0x666d7420
#define BE_DATA_CHUNK_ID 0x64617461

#define LE_RIFF_CHUNK_ID 0x46464952
#define LE_WAVE_CHUNK_ID 0x45564157
#define LE_FMT_CHUNK_ID 0x20746d66
#define LE_DATA_CHUNK_ID 0x61746164

// Skippable chunks
#define BE_LIST_CHUNK_ID 0x4C495354
#define LE_LIST_CHUNK_ID 0x5453494C

// Offsets
#define RIFF_CHUNK_OFFSET 4
#define NORMAL_CHUNK_OFFSET 8

// Audio Formats
#define PCM_AUDIO_FORMAT 1

// PCM Specific
#define PCM_FMT_CHUNK_SIZE 16

typedef enum sample_type_t {
    suint8,
    suint16,
    sfloat32
} sample_type_t;

typedef char cint8_t;

typedef struct wave_riff_chunk_t {
    int32_t chunk_id;
    int32_t chunk_size;
    int32_t format;
} wave_riff_chunk_t;

typedef struct wave_fmt_chunk_t {
    int32_t subchunk_id;
    int32_t subchunk_size;
    int16_t audio_format;
    int16_t num_channels;
    int32_t sample_rate;
    int32_t byte_rate;
    int16_t block_align;
    int16_t bits_per_sample;
} wave_fmt_chunk_t;

typedef struct wave_data_chunk_t {
    int32_t subchunk_id;
    int32_t subchunk_size;
    cint8_t *data;
} wave_data_chunk_t;

typedef struct wave_t {
    wave_riff_chunk_t riff_chunk;
    wave_fmt_chunk_t fmt_chunk;
    wave_data_chunk_t data_chunk;
    uint8_t is_loaded;
} wave_t;


inline int32_t get_file_size(FILE *file)
{
    fseek(file, 0L, SEEK_END);
    int32_t size = ftell(file);
    rewind(file);
    return size;
}

inline int32_t wave_calculate_byte_rate(int32_t sample_rate, int16_t num_channels, int16_t bits_per_sample)
{
    return sample_rate * num_channels * (bits_per_sample / 8);
}

inline int16_t wave_calculate_block_align(int16_t num_channels, int16_t bits_per_sample)
{
    return num_channels * (bits_per_sample / 8);
}

inline int16_t wave_get_bits_per_sample(sample_type_t sample_type)
{
    switch (sample_type) {
        case suint8: {
            return 8;
        }
        case suint16: {
            return 16;
        }
        case sfloat32: {
            return 32;
        }
        default: {
            return 8;
        }
    }
}

inline void wave_save(const wchar_t *filepath, cint8_t *data, int32_t data_size, int16_t num_channels, int32_t sample_rate, sample_type_t sample_type)
{
    FILE *file = _wfopen(filepath, L"wb");

    if (!file) {
        fprintf(stderr, "FILE_STREAM: Cannot open a file!\nFile: %ls\n", filepath);
        exit(-1);
    }

    int16_t bits_per_sample = wave_get_bits_per_sample(sample_type);

    wave_data_chunk_t data_chunk = {
        .subchunk_id = LE_DATA_CHUNK_ID,
        .subchunk_size = data_size,
        .data = data
    };
    
    wave_fmt_chunk_t fmt_chunk = {
        .subchunk_id = LE_FMT_CHUNK_ID,
        .subchunk_size = PCM_FMT_CHUNK_SIZE,
        .audio_format = PCM_AUDIO_FORMAT,
        .num_channels = num_channels,
        .sample_rate = sample_rate,
        .byte_rate = wave_calculate_byte_rate(sample_rate, num_channels, bits_per_sample),
        .block_align = wave_calculate_block_align(num_channels, bits_per_sample),
        .bits_per_sample = bits_per_sample
    };

    wave_riff_chunk_t riff_chunk = {
        .chunk_id = LE_RIFF_CHUNK_ID,
        .chunk_size = RIFF_CHUNK_OFFSET + (NORMAL_CHUNK_OFFSET + PCM_FMT_CHUNK_SIZE) + (NORMAL_CHUNK_OFFSET + data_size),
        .format = LE_WAVE_CHUNK_ID
    };

    fwrite(&riff_chunk, sizeof(riff_chunk), 1, file);
    
    fwrite(&fmt_chunk.subchunk_id, sizeof(fmt_chunk.subchunk_id), 1, file);
    fwrite(&fmt_chunk.subchunk_size, sizeof(fmt_chunk.subchunk_size), 1, file);
    fwrite(&fmt_chunk.audio_format, sizeof(fmt_chunk.audio_format), 1, file);
    fwrite(&fmt_chunk.num_channels, sizeof(fmt_chunk.num_channels), 1, file);
    fwrite(&fmt_chunk.sample_rate, sizeof(fmt_chunk.sample_rate), 1, file);
    fwrite(&fmt_chunk.byte_rate, sizeof(fmt_chunk.byte_rate), 1, file);
    fwrite(&fmt_chunk.block_align, sizeof(fmt_chunk.block_align), 1, file);
    fwrite(&fmt_chunk.bits_per_sample, sizeof(fmt_chunk.bits_per_sample), 1, file);

    fwrite(&data_chunk.subchunk_id, sizeof(data_chunk.subchunk_id), 1, file);
    fwrite(&data_chunk.subchunk_size, sizeof(data_chunk.subchunk_size), 1, file);
    fwrite(data_chunk.data, data_size, 1, file);

    fclose(file);
}

inline wave_t wave_open(const wchar_t *filepath)
{
    FILE *file = _wfopen(filepath, L"rb");

    if(!file) {
        fprintf(stderr, "FILE_STREAM: Cannot open a file!\nFile: %ls\n", filepath);
        exit(-1);
    }

    int32_t calculated_chunk_size = get_file_size(file) - NORMAL_CHUNK_OFFSET;

    wave_t wave_file = {0};
    wave_riff_chunk_t riff_chunk = {0};
    wave_fmt_chunk_t fmt_chunk = {0};
    wave_data_chunk_t data_chunk = {0};

    int32_t chunk_id = 0;

    fread(&riff_chunk, sizeof(riff_chunk), 1, file);

    if(riff_chunk.chunk_id != LE_RIFF_CHUNK_ID) {
        fprintf(stderr, "RIFF_CHUNK_ID:\nGot: 0x%04x\nExpected: 0x%04x\n", riff_chunk.chunk_id, BE_RIFF_CHUNK_ID);
        exit(-1);
    }

    if(riff_chunk.chunk_size != calculated_chunk_size) {
        fprintf(stderr, "RIFF_CHUNK_SIZE: Chunk size is not equal to calculated chunk size\nGot: 0x%04x\nExpected: 0x%04x\n", riff_chunk.chunk_size, calculated_chunk_size);
        exit(-1);
    }

    if(riff_chunk.format != LE_WAVE_CHUNK_ID) {
        fprintf(stderr, "WAVE_CHUNK_ID:\nGot: 0x%04x\nExpected: 0x%04x\n", riff_chunk.format, BE_WAVE_CHUNK_ID);
        exit(-1);
    }

    fread(&fmt_chunk, sizeof(fmt_chunk), 1, file);

    if(fmt_chunk.subchunk_id != LE_FMT_CHUNK_ID) {
        fprintf(stderr, "FMT_SUBCHUNK_ID:\nGot: 0x%04x\nExpected: 0x%04x\n", fmt_chunk.subchunk_id, BE_FMT_CHUNK_ID);
        exit(-1);
    }

    if(fmt_chunk.audio_format != PCM_AUDIO_FORMAT)
    {
        fprintf(stderr, "AUDIO_FORMAT:\nUnsupported audio format\nSupported audio formats: PCM\n");
        exit(-1);
    }

    
    int32_t calculated_byte_rate = wave_calculate_byte_rate(fmt_chunk.sample_rate, fmt_chunk.num_channels, fmt_chunk.bits_per_sample);
    if(fmt_chunk.byte_rate != calculated_byte_rate) {
        fprintf(stderr, "BYTE_RATE:\nByte rate is not equal to calculated byte rate\nGot: %i\nExpected: %i\n", fmt_chunk.byte_rate, calculated_byte_rate);
        exit(-1);
    }

    
    int32_t calculated_block_align = wave_calculate_block_align(fmt_chunk.num_channels, fmt_chunk.bits_per_sample);
    if(fmt_chunk.block_align != calculated_block_align) {
        fprintf(stderr, "BLOCK_ALIGN:\nBlock align is not equal to calculated block align\nGot: %i\nExpected: %i\n", fmt_chunk.block_align, calculated_block_align);
        exit(-1);
    }

    fread(&chunk_id, sizeof(chunk_id), 1, file);
    switch(chunk_id) {
        case LE_LIST_CHUNK_ID: {
            int32_t list_subchunk_size = 0;
            fread(&list_subchunk_size, sizeof(list_subchunk_size), 1, file);
            calculated_chunk_size -= (list_subchunk_size + NORMAL_CHUNK_OFFSET);
            fseek(file, list_subchunk_size, SEEK_CUR);
            break;
        }
        case LE_DATA_CHUNK_ID: {
            int32_t current_file_pointer_position = ftell(file);
            fseek(file, current_file_pointer_position - sizeof(chunk_id), SEEK_SET);
            break;
        }
    }

    fread(&data_chunk.subchunk_id, sizeof(data_chunk.subchunk_id), 1, file);

    if(data_chunk.subchunk_id != LE_DATA_CHUNK_ID) {
        fprintf(stderr, "DATA_SUBCHUNK_ID:\nGot: 0x%04x\nExpected: 0x%04x\n", data_chunk.subchunk_id, BE_DATA_CHUNK_ID);
        exit(-1);
    }

    fread(&data_chunk.subchunk_size, sizeof(data_chunk.subchunk_size), 1, file);

    int32_t calculated_data_subchunk_size = calculated_chunk_size - sizeof(fmt_chunk) - sizeof(riff_chunk);
    if(data_chunk.subchunk_size > calculated_data_subchunk_size) {
        fprintf(stderr, "DATA_SUBCHUNK_SIZE:\nGot: 0x%04x\nExpected: 0x%04x\n", data_chunk.subchunk_size, calculated_data_subchunk_size);
        exit(-1);
    }

    data_chunk.data = (cint8_t *) malloc(sizeof(data_chunk.data) * data_chunk.subchunk_size);
    if (!data_chunk.data) {
        fprintf(stderr, "MALLOC:\nFailed to allocate memory of size: %lli\n", sizeof(data_chunk.data) * data_chunk.subchunk_size);
        exit(-1);
    }
    fread(data_chunk.data, data_chunk.subchunk_size, 1, file);

    fclose(file);

    wave_file.riff_chunk = riff_chunk;
    wave_file.fmt_chunk = fmt_chunk;
    wave_file.data_chunk = data_chunk;

    wave_file.is_loaded = 1;

    return wave_file;
}

void wave_free(wave_t *wave)
{
    if (wave->is_loaded) {
        free(wave->data_chunk.data);
        wave->is_loaded = 0;
    }
}

#ifdef _cplusplus
}
#endif

#endif