#ifndef PICINOUTPUT_H
#define PICINOUTPUT_H

#include "common.h"
#include <string.h>  // memcpy

// RAW Y8 로드
static inline int image_load_raw_y8(const char* path, I32 width, I32 height, ImageY8* img) {
	FILE* f = fopen(path, "rb");

	image_alloc(img, width, height);

	size_t need = (size_t)width * (size_t)height;
	size_t got = fread(img->data, 1, need, f);
	fclose(f);
	return got == need;
}

// RAW Y8 저장
static inline int image_save_raw_y8(const char* path, const ImageY8* img) {
	FILE* f = fopen(path, "wb");
	size_t want = (size_t)img->info.width * (size_t)img->info.height;
	size_t put = fwrite(img->data, 1, want, f);
	printf("%s: %d * %d\n", path, img->info.width, img->info.height);
	fclose(f);
	return put == want;
}
#endif // PICINOUTPUT_H
