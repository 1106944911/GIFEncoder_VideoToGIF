#ifndef GIF_ENCODER_H
#define GIF_ENCODER_H

#include <stdio.h>
#include <stdint.h>
#include "bit_writing_block.h"

#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

typedef struct _GifInfo {
	uint16_t width;
	uint16_t height;
	uint16_t left;
	uint16_t top;
	int useDither;
	const char* gifTitle;
	FILE * gifFile;
} GifInfo;

int init(GifInfo * gifInfo, uint16_t width, uint16_t height, const char* gifTitle);
int header(GifInfo * gifInfo);
int logicalScreenDescriptor(GifInfo * gifInfo);
int globalColorTable(GifInfo * gifInfo);
int graphicsControlExtension(GifInfo * gifInfo, uint16_t delay);
int imageDescriptor(GifInfo * gifInfo);
int imageData(GifInfo * gifInfo, uint8_t * indexStream);
int finish(GifInfo * gifInfo);
int reduceColor(GifInfo * gifInfo, uint32_t* pixels);
int writeNetscapeExt(GifInfo * gifInfo);

#endif GIF_ENCODER_H