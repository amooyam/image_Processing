#ifndef PICOPS_H
#define PICOPS_H

#include "common.h"

// 0~255 범위 클램프
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


// 입력 Y8 영상(src)의 히스토그램(0~255)을 256x256 이미지로 시각화하여 hist에 출력(흰 막대)
void image_histogram_256(const ImageY8* src, ImageY8* hist);

// src(Y8)의 히스토그램 평활화 결과를 dst에 출력 (dst는 W×H로 미리 할당)
void image_equalization(const ImageY8* src, ImageY8* dst);

// 입력(src) → 목표(선형 증가형 타깃 히스토그램)으로 Histogram Matching 수행
// dst는 src와 동일 크기(W×H)로 미리 할당되어 있어야 합니다.
void image_histmatch(const ImageY8* src, ImageY8* dst);

//void image_cdf_graph(const ImageY8* src, ImageY8* dst);

void image_stretching(const ImageY8* src, ImageY8* dst, int low, int high);

#endif // PICOPS_H
