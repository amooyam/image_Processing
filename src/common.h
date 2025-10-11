#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

// 타입 별칭
typedef uint8_t  U8;   // 픽셀(0~255)
typedef int32_t  I32;  // 정수 (폭, 높이 등)

// 영상 정보
typedef struct Information {
	I32 width;
	I32 height;
} Information;

// 단일 프레임 8bit 흑백 영상
typedef struct ImageY8 {
	U8* data;         // 픽셀 버퍼
	Information info; // width, height
	I32 stride;       // 행 보폭 (여기서는 width와 동일)
} ImageY8;

// 메모리 관리
static inline int image_alloc(ImageY8* img, I32 width, I32 height) 
{
	img->info.width = width;
	img->info.height = height;
	img->stride = width;
	size_t bytes = (size_t)width * (size_t)height;
	img->data = (U8*)malloc(bytes);
	return img->data != NULL;
}

static inline void image_free(ImageY8* img) 
{
	free(img->data);
	img->data = NULL;
	img->info.width = 0;
	img->info.height = 0;
	img->stride = 0;
}

#endif // COMMON_H
