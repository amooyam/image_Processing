#include "common.h"
#include "picIO.h"
#include "picOps.h" //Picture Operations

// ---- 사용자 설정 ----
#ifndef IN_PATH
#define IN_PATH   "lena_512x512.raw"
#endif
#ifndef WIDTH
#define WIDTH     512
#endif
#ifndef HEIGHT
#define HEIGHT    512
#endif
// ---------------------

int main(void) 
{	
	const int Rotation_factor = 30;                // 필요 시 조정
	const double theta = Rotation_factor * (3.14159265358979323846 / 180.0);

	const double scale_factor = 0.47;                // 필요 시 조정
	const int outW = (int)(WIDTH * scale_factor + 0.5);
	const int outH = (int)(HEIGHT * scale_factor + 0.5);
	
	// 1) 입력 영상 로드
	ImageY8 src = { 0 };
	image_load_raw_y8(IN_PATH, WIDTH, HEIGHT, &src);

	// 2) 출력 영상 할당
	ImageY8 dstScale[INTERPOL_COUNT] = { 0 };
	ImageY8 dstRotate[INTERPOL_COUNT] = { 0 };
	for (InterpolationMode mode = (InterpolationMode)0; mode < INTERPOL_COUNT; mode = (InterpolationMode)(mode + 1))
	{
		image_alloc(&dstScale[mode], outW, outH);
		image_alloc(&dstRotate[mode], src.info.width, src.info.height);
	}

	ImageY8 dstfilter[FILTER_COUNT] = { 0 };
	for (FilterMode mode = (FilterMode)0; mode < FILTER_COUNT; mode = (FilterMode)(mode + 1))
		image_alloc(&dstfilter[mode], src.info.width, src.info.height);
	
	// 3) 연산
	/////////////////////////////////////////////////////////////////////
	image_scale(&src, dstScale, scale_factor);
	image_rotate(&src, dstRotate, theta);
	image_filter(&src, dstfilter, FILTER_COUNT);
	////////////////////////////////////////////////////////////////////
	
	// 4) 저장
	image_save_raw_y8("rScale_nearest.raw", &dstScale[INTERPOL_NEARESTNEIGHBOR]);
	image_save_raw_y8("rScale_Bilinear.raw", &dstScale[INTERPOL_Bilinear]);
	image_save_raw_y8("rScale_BSpline.raw", &dstScale[INTERPOL_BSpline]);

	image_save_raw_y8("rRotate_nearest.raw", &dstRotate[INTERPOL_NEARESTNEIGHBOR]);
	image_save_raw_y8("rRotate_Bilinear.raw", &dstRotate[INTERPOL_Bilinear]);
	image_save_raw_y8("rRotate_BSpline.raw", &dstRotate[INTERPOL_BSpline]);

	image_save_raw_y8("rFilter_embo.raw", &dstfilter[FILTER_EMBOSS]);
	image_save_raw_y8("rFilter_sharp.raw", &dstfilter[FILTER_SHARPEN]);
	image_save_raw_y8("rFilter_DoG.raw", &dstfilter[FILTER_DOG7x7]);
	image_save_raw_y8("rFilter_Blur.raw", &dstfilter[FILTER_BLUR]);
	image_save_raw_y8("rFilter_Median.raw", &dstfilter[FILTER_MEDIAN]);
	image_save_raw_y8("rFilter_Homo.raw",   &dstfilter[FILTER_HOMO]);


	// 5) 정리
	image_free(&src);

	for (InterpolationMode mode = (InterpolationMode)0; mode < INTERPOL_COUNT; mode = (InterpolationMode)(mode + 1))
	{
		image_free(&dstScale[mode]);
		image_free(&dstRotate[mode]);
	}

	for (FilterMode mode = (FilterMode)0; mode < FILTER_COUNT; mode = (FilterMode)(mode + 1))
		image_free(&dstfilter[mode]);

	return 0;
}
