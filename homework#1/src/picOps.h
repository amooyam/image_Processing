#ifndef PICOPS_H
#define PICOPS_H

#include "common.h"

// 0~255 ���� Ŭ����
static inline unsigned char clip3_u8_int(int v) {
	if (v < 0)   return 0u;
	if (v > 255) return 255u;
	return (unsigned char)v;
}

static inline void image_Arithmetic_PLUS(const ImageY8* src, ImageY8* dst)
{
	const size_t N = (size_t)src->info.width * (size_t)src->info.height;
	for (size_t i = 0; i < N; ++i) {
		{
			int v = (int)src->data[i] + 70;
			dst->data[i] = clip3_u8_int(v);
		}
	}
}

void image_Arithmetic_MINUS(const ImageY8* src, ImageY8* dst);
void image_Arithmetic_Multi(const ImageY8* src, ImageY8* dst);
void image_Arithmetic_Divi(const ImageY8* src, ImageY8* dst);


// �Է� Y8 ����(src)�� ������׷�(0~255)�� 256x256 �̹����� �ð�ȭ�Ͽ� hist�� ���(�� ����)
void image_histogram_256(const ImageY8* src, ImageY8* hist);

// src(Y8)�� ������׷� ��Ȱȭ ����� dst�� ��� (dst�� W��H�� �̸� �Ҵ�)
void image_equalization(const ImageY8* src, ImageY8* dst);

// �Է�(src) �� ��ǥ(���� ������ Ÿ�� ������׷�)���� Histogram Matching ����
// dst�� src�� ���� ũ��(W��H)�� �̸� �Ҵ�Ǿ� �־�� �մϴ�.
void image_histmatch(const ImageY8* src, ImageY8* dst);

//void image_cdf_graph(const ImageY8* src, ImageY8* dst);

void image_stretching(const ImageY8* src, ImageY8* dst, int low, int high);

#endif // PICOPS_H
