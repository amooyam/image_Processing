#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

// Ÿ�� ��Ī
typedef uint8_t  U8;   // �ȼ�(0~255)
typedef int32_t  I32;  // ���� (��, ���� ��)

// ���� ����
typedef struct Information {
	I32 width;
	I32 height;
} Information;

// ���� ������ 8bit ��� ����
typedef struct ImageY8 {
	U8* data;         // �ȼ� ����
	Information info; // width, height
	I32 stride;       // �� ���� (���⼭�� width�� ����)
} ImageY8;

// �޸� ����
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
