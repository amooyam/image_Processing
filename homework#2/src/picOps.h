#ifndef PICOPS_H
#define PICOPS_H

#include "common.h"

// 크로스플랫폼 restrict
#if defined(_MSC_VER)
#define RESTRICT __restrict
#elif defined(__GNUC__) || defined(__clang__)
#define RESTRICT __restrict__
#else
#define RESTRICT restrict
#endif


// 0~255 범위 클램프
static inline unsigned char clip3_u8_int(int v) {
	if (v < 0)   return 0u;
	if (v > 255) return 255u;
	return (unsigned char)v;
}


static inline int clampi(int x, int lo, int hi) {
	return x < lo ? lo : (x > hi ? hi : x);
}


typedef enum {
	INTERPOL_NEARESTNEIGHBOR = 0,
	INTERPOL_Bilinear = 1,
	INTERPOL_BSpline = 2,
	//뭔가 넣을거면 이 사이에 넣자
	INTERPOL_COUNT
} InterpolationMode;

typedef enum {
	FILTER_EMBOSS = 0,
	FILTER_SHARPEN = 1,
	FILTER_DOG7x7 = 2,
	FILTER_BLUR = 3, //Gaussian Filter
	FILTER_MEDIAN = 4,    // 추가
    FILTER_HOMO = 5,
	FILTER_COUNT
} FilterMode;


void image_scale(const ImageY8* src, ImageY8 dst[INTERPOL_COUNT], double sf); //sf: scale factor
void image_rotate(const ImageY8* src, ImageY8 dst[INTERPOL_COUNT], double theta);
void image_filter(const ImageY8* src, ImageY8 dst[FILTER_COUNT], FilterMode targetMode);
#endif // PICOPS_H
