#include "picOps.h"
#include <math.h>
#include <string.h> // memset

// =============================================================
// 최근접 이웃 보간 (Nearest Neighbor Interpolation)
// Nearest Neighbor interpolation function
// 입력된 부동소수 좌표(fx, fy)를 가장 가까운 정수 좌표로 반올림하여 샘플링함
// Given fractional coordinates (fx, fy), round to the nearest integer
// and fetch the corresponding pixel value.
// =============================================================

static inline unsigned char nearest(const unsigned char* RESTRICT s, // 원본 영상 데이터 포인터 / pointer to source image data
	int Sw, int Sh,                                                 // 영상의 너비(Width), 높이(Height) / image width and height
	double fx, double fy)                                           // 역매핑된 부동소수 좌표 / mapped fractional coordinates
{
	// 1) 최근접 정수 좌표 계산 (반올림)
	int ix = clampi((int)(fx + 0.5), 0, Sw - 1);  // x좌표를 0~Sw-1 범위로 클램프 / clamp x within valid width range
	int iy = clampi((int)(fy + 0.5), 0, Sh - 1);  // y좌표를 0~Sh-1 범위로 클램프 / clamp y within valid height range

	// 2) 인덱스 계산 및 픽셀 접근
	// (iy * Sw + ix)는 2차원 좌표 → 1차원 메모리 인덱스 변환 / convert (x,y) to linear memory index
	return s[(size_t)iy * (size_t)Sw + (size_t)ix]; // 최종 픽셀 값 반환 / return the sampled pixel
}

// =============================================================
// Bilinear 보간 함수 (Bilinear Interpolation Function)
// 입력된 실수 좌표 (fx, fy)를 이용하여 주변 2x2 화소 값을 선형 가중합으로 계산
// =============================================================

static inline unsigned char Bilinear(const unsigned char* RESTRICT s,  // 원본 영상 포인터 / pointer to source image
	int Sw, int Sh,                                                   // 영상의 가로, 세로 크기 / image width and height
	double fx, double fy)                                             // 역매핑된 실수 좌표 / mapped floating-point coordinates
{
	//////최적화 안봄
	// 2) 경계 클램프 (하한) : 좌표가 영상 밖으로 벗어나는 것을 방지
	fx = fx < 0 ? 0 : (fx > Sw - 1 ? Sw - 1 : fx);
	fy = fy < 0 ? 0 : (fy > Sh - 1 ? Sh - 1 : fy);

	// 1) 기준 정수 좌표 (왼쪽 위)
	int x0 = (int)fx;
	int y0 = (int)fy;

	// 3) 이웃 픽셀 (오른쪽 / 아래) - 상한 클램프 : 오른쪽(x+1), 아래(y+1) 방향 픽셀 좌표를 계산하되, 영상 경계 밖으로 나가지 않도록 제한
	x0 = clampi(x0, 0, Sw - 2);
	y0 = clampi(y0, 0, Sh - 2);

	// 4) 소수부 및 가중치 계산 : 실수 좌표(fx, fy)에서 정수 좌표(x0, y0)까지의 거리로 보간 가중치를 계산
			// x 방향 거리 / fractional distance along x
			// y 방향 거리 / fractional distance along y
			// x 방향 가중치 / weights along x
			// y 방향 가중치 / weights along y
	double dx = fx - (double)x0;
	double dy = fy - (double)y0;

	// 5) 2x2 샘플 픽셀 불러오기 : 주변 4개의 픽셀(p00, p01, p10, p11)을 불러와 가중합 계산에 사용
			 // 좌상단(top-left)
			 // 우상단(top-right)
			 // 좌하단(bottom-left)
			 // 우하단(bottom-right)
	int cur = y0 * Sw + x0;
	int p00 = s[cur];
	int p01 = s[cur + 1];
	int p10 = s[cur + Sw];
	int p11 = s[cur + Sw + 1];

	// 6) 가중합 (행 보간 → 열 보간) : 먼저 행 방향으로 보간한 후, 그 결과를 다시 열 방향으로 보간
			 // 상단 행 보간 / interpolate along top row
			 // 하단 행 보간 / interpolate along bottom row
			 // 최종 보간 + 반올림 / final interpolated value with rounding
	double topr = p00 * (1 - dx) + p01 * dx;
	double botr = p10 * (1 - dx) + p11 * dx;
	int val = (int)(topr * (1 - dy) + botr * dy + 0.5);

	// 7) 출력 클램프 (0~255) : 계산 결과를 8비트 유효 범위로 제한
	return clip3_u8_int(val); // 이걸로 리턴해야함
}

// 0) 커널: 3차 B-Spline
static inline float BSplineKernelF(float x)
{
	// |x| 기준의 구간 3차 다항식 (pow 미사용: 연산 축소)
	x = (float)fabs((double)x);
	/////////////////////////////////////////////////////////////////////////////
	if(x>=0.0 && x<1.0) return 0.5*x*x*x - x*x + (2.0/3.0);
	else if(x>=1 && x<2) return -(1.0/6.0)*x*x*x + x*x -2.0*x + (4.0/3.0);
	else return 0.0;
	/////////////////////////////////////////////////////////////////////////////
}
// =============================================================
// 3차 B-스플라인 보간 (Cubic B-Spline Interpolation)
// 입력 실수 좌표 (fx, fy) 주위의 4x4 픽셀을 사용하여 부드럽게 보간합니다.
//   s      : 입력(Y8) 버퍼 / source Y8 buffer
//   Sw, Sh : 입력 너비/높이 / source width & height
//   fx, fy : 역매핑된 실수 좌표 / mapped floating-point coordinates
// 반환     : 보간된 8-bit 값(0~255) / interpolated 8-bit value (0–255)
// =============================================================
static inline unsigned char BSpline(const unsigned char* RESTRICT s,
	int Sw, int Sh,
	double fx, double fy)
{
	// 1) 기준 정수 좌표 (바닥값; floor)
	// 실수 좌표의 정수부를 기준점으로 사용
	int x0 = floor(fx);
	int y0 = floor(fy);

	// 2) 소수부 계산 (0 <= frac < 1)
	// 정수 기준점으로부터의 거리
	double dx = fx - (double)x0;
	double dy = fy - (double)y0;

	// 3) 4점 인덱스(좌/우/상/하) — 경계 클램프 *** 카피 패딩 사용
	// x: {x0-1, x0, x0+1, x0+2}, y: {y0-1, y0, y0+1, y0+2}
	int xs[4] = { x0 - 1, x0, x0 + 1, x0 + 2 };
    int ys[4] = { y0 - 1, y0, y0 + 1, y0 + 2 };

    for(int i = 0; i < 4; i++) { // 카피패딩
        if(xs[i] < 0)      xs[i] = 0;
        if(xs[i] >= Sw)    xs[i] = Sw - 1;
        if(ys[i] < 0)      ys[i] = 0;
        if(ys[i] >= Sh)    ys[i] = Sh - 1;
    }

	// 4) B-스플라인 커널 가중치 (x, y 각각 4개)
	// 4) Cubic B-spline kernel weights (4 for x, 4 for y)
	// m ∈ {-1, 0, 1, 2}, frac ∈ [0,1) → {-1-ax, -ax, 1-ax, 2-ax} 등으로 평가
	double wx[4];
    wx[0] = BSplineKernelF(-1.0 - dx);
	wx[1] = BSplineKernelF(0.0 - dx);
	wx[2] = BSplineKernelF(1.0 - dx);
	wx[3] = BSplineKernelF(2.0 - dx);

    double wy[4];
    wy[0] = BSplineKernelF(-1.0 - dy);
	wy[1] = BSplineKernelF(0.0 - dy);
	wy[2] = BSplineKernelF(1.0 - dy);
	wy[3] = BSplineKernelF(2.0 - dy);

	// 5) 행 오프셋 미리 계산 — 반복 연산 제거
	const unsigned char* row0 = s + ys[0] * Sw;
    const unsigned char* row1 = s + ys[1] * Sw;
    const unsigned char* row2 = s + ys[2] * Sw;
    const unsigned char* row3 = s + ys[3] * Sw;

	// 6) x방향 1D 보간(각 행에서 4픽셀 가중합) → r0..r3
    double r[4];
    r[0] = row0[xs[0]]*wx[0] + row0[xs[1]]*wx[1] + row0[xs[2]]*wx[2] + row0[xs[3]]*wx[3];
    r[1] = row1[xs[0]]*wx[0] + row1[xs[1]]*wx[1] + row1[xs[2]]*wx[2] + row1[xs[3]]*wx[3];
    r[2] = row2[xs[0]]*wx[0] + row2[xs[1]]*wx[1] + row2[xs[2]]*wx[2] + row2[xs[3]]*wx[3];
    r[3] = row3[xs[0]]*wx[0] + row3[xs[1]]*wx[1] + row3[xs[2]]*wx[2] + row3[xs[3]]*wx[3];	

	// 7) y방향 1D 보간(행 부분합 r0..r3 결합)
	double v =
        r[0]*wy[0] +
        r[1]*wy[1] +
        r[2]*wy[2] +
        r[3]*wy[3];

	// 8) 반올림 후 0~255 범위로 포화
	int vv = (int)(v + 0.5);
	if(vv < 0)   vv = 0;
	if(vv > 255) vv = 255;
	return (unsigned char)vv;
}



void image_scale(const ImageY8* src, ImageY8 dst[INTERPOL_COUNT], double sf) //sf: scale factor
{
	const int Sw = src->info.width;
	const int Sh = src->info.height;
	const int Dw = dst[0].info.width;
	const int Dh = dst[0].info.height;


	// 1) 역매핑 실수 좌표 LUT (보간 공통 사용)
	double* mapXf = (double*)malloc((size_t)Dw * sizeof(double));
	double* mapYf = (double*)malloc((size_t)Dh * sizeof(double));
	/////////////////////////////////////////////////////////////////////////////////
	//역매핑 LUT에 저장
	for(int x = 0; x < Dw; ++x) mapXf[x] = (double)x / sf;
	for(int y = 0; y < Dh; ++y) mapYf[y] = (double)y / sf;
	//
	////////////////////////////////////////////////////////////////////////////////
	// 2) 최종 루프: 여기서는 보간 함수만 바뀐다
	const unsigned char* RESTRICT sp = src->data; //source pointer
		  unsigned char* RESTRICT dN  = dst[0].data;
		  unsigned char* RESTRICT dB  = dst[1].data;
		  unsigned char* RESTRICT dBS = dst[2].data;

	for (int y = 0; y < Dh; ++y) 
	{
		const size_t drow = (size_t)y * (size_t)Dw;
		const double fy = mapYf[y];
		for (int x = 0; x < Dw; ++x) 
		{
			const double fx = mapXf[x];
			dN [drow + (size_t)x] = nearest(sp, Sw, Sh, fx, fy);
			dB [drow + (size_t)x] = Bilinear(sp, Sw, Sh, fx, fy);
			dBS[drow + (size_t)x] = BSpline(sp, Sw, Sh, fx, fy);
		}
	}

	free(mapXf);
	free(mapYf);
}

void image_rotate(const ImageY8* src, ImageY8 dst[INTERPOL_COUNT], double theta)
{
	// [0] 기초 파라미터
	const int Sw = src->info.width;
	const int Sh = src->info.height;
	const int Dw = dst[0].info.width;   // main에서 원본과 동일 크기로 할당됨
	const int Dh = dst[0].info.height;


	// [1] 중심 좌표
	const double cx = (double)(Sw - 1) * 0.5;
	const double cy = (double)(Sh - 1) * 0.5;

	// [2] 각도/삼각값 준비 (deg → rad)
	const double c = cos(theta);
	const double s = sin(theta);

	// [3] 포인터 별칭 제거 힌트
	const unsigned char* RESTRICT sp = src->data;
	unsigned char* RESTRICT dN = dst[0].data; // Nearest
	unsigned char* RESTRICT dB = dst[1].data; // Bilinear
	unsigned char* RESTRICT dS = dst[2].data; // BSpline

	// [4] 역매핑(Backward) + 행별 증분 갱신
	for (int y = 0; y < Dh; ++y) 
	{
		const size_t drow = (size_t)y * (size_t)Dw;
		const double dy = (double)y - cy;   // 중심 기준 y 좌표

		double fx =  c*(0 - cx) + s*(y - cy) + cx;
		double fy = -s*(0 - cx) + c*(y - cy) + cy;

		// 최적화용
		const double dfx =  c;   // x += 1 → fx += c
		const double dfy = -s;   // x += 1 → fy += -s

		for (int x = 0; x < Dw; ++x) // 최적화 필요
		{
			const size_t idx = drow + (size_t)x;

			if (fx < 0.0 || fx >(double)(Sw - 1) || fy < 0.0 || fy >(double)(Sh - 1))
			{
				dN[idx] = dB[idx] = dS[idx] = 0; // 빈공간을 검정색으로
			}
			else {
				dN[idx] = nearest(sp, Sw, Sh, fx, fy);
				dB[idx] = Bilinear(sp, Sw, Sh, fx, fy);
				dS[idx] = BSpline(sp, Sw, Sh, fx, fy);
			}
			fx += dfx;
			fy += dfy;
			//
		}
	}
}

// 경계 클램프 샘플(인라인)
static inline int sample_clamp_idx(const unsigned char* RESTRICT s, // 안쓰는게 더 간단함
	int w, int h,
	int x, int y)
{
	x = clampi(x, 0, w - 1);
	y = clampi(y, 0, h - 1);
	return (int)s[(size_t)y * (size_t)w + (size_t)x];
}


static const int MaskSize[FILTER_COUNT] = { 3, 3, 7, 3 }; // 이 아래로 알아서 추가
static const int bias_list[FILTER_COUNT] = { 128, 0, 0, 0 }; // 바이어스 안쓰는 애들은 0
static const int shift_list[FILTER_COUNT] = { 0, 0, 0, 4 }; // 비트 시프트 쓰는 용도
// --- 커널(마스크) 정의 ---
static const int K_EMBOSS[9] = { 
	-1,  0,  0,  
	 0,  0,  0,  
	 0,  0,  1 };
static const int K_SHARP[9] = {
	 0, -1,  0,
	-1,  5, -1,
	 0, -1,  0
};
static const int K_DOG7x7[49] = {
	 0,  0, -1, -1, -1,  0,  0,
	 0, -2, -3, -3, -3, -2,  0,
	-1, -3,  5,  5,  5, -3, -1,
	-1, -3,  5, 16,  5, -3, -1,
	-1, -3,  5,  5,  5, -3, -1,
	 0, -2, -3, -3, -3, -2,  0,
	 0,  0, -1, -1, -1,  0,  0
};
static const int K_BLUR[9] = {
	 1, 2, 1,
	 2, 4, 2,
	 1, 2, 1
};

void image_filter(const ImageY8* src, ImageY8 dst[FILTER_COUNT], FilterMode targetMode)
{
	const int w = src->info.width;
	const int h = src->info.height;
	const unsigned char* RESTRICT s = src->data;

	const int* kernel_list[] = { K_EMBOSS, K_SHARP, K_DOG7x7, K_BLUR };

	for (int mode = targetMode == FILTER_COUNT ? 0 : targetMode;
		mode < (targetMode == FILTER_COUNT ? FILTER_COUNT : targetMode + 1);
		mode++)
	{
		ImageY8* out = (targetMode == FILTER_COUNT) ? &dst[mode] : &dst[0];
		unsigned char* RESTRICT d = out->data;

		const int* K = kernel_list[mode];
		const int  N = MaskSize[mode];
		const int  R = N >> 1; // 패딩 개수 체크
		const int  bias = bias_list[mode];
		const int  shift = shift_list[mode];
		for (int y = 0; y < h; ++y) 
		{
			const size_t drow = (size_t)y * (size_t)w;
			for (int x = 0; x < w; ++x) 
			{
				// -------------------------------------------------------
				// 1) MEDIAN
				if (mode == FILTER_MEDIAN)
				{
					unsigned char buf[9];
					int idx = 0;

					for (int ky = -1; ky <= 1; ky++) {
						int sy = y + ky;
						if (sy < 0) sy = 0;
						if (sy >= h) sy = h - 1;

						for (int kx = -1; kx <= 1; kx++) {
							int sx = x + kx;
							if (sx < 0) sx = 0;
							if (sx >= w) sx = w - 1;

							buf[idx++] = s[(size_t)sy * w + (size_t)sx];
						}
					}

					// 9개 정렬 (선택정렬)
					for (int i = 0; i < 9; i++) {
						for (int j = i + 1; j < 9; j++) {
							if (buf[i] > buf[j]) {
								unsigned char t = buf[i];
								buf[i] = buf[j];
								buf[j] = t;
							}
						}
					}

					d[drow + x] = buf[4];   // median
					continue;
				}

				// -------------------------------------------------------
				// 2) HOMO
				if (mode == FILTER_HOMO)
				{
					int center = s[drow + x];
					int maxv = 0;

					for (int ky = -1; ky <= 1; ky++) {
						int sy = y + ky;
						if (sy < 0) sy = 0;
						if (sy >= h) sy = h - 1;

						for (int kx = -1; kx <= 1; kx++) {
							int sx = x + kx;
							if (sx < 0) sx = 0;
							if (sx >= w) sx = w - 1;

							if (kx == 0 && ky == 0) continue;  // 중심 제외

							int v = s[(size_t)sy * w + (size_t)sx];
							int diff = center - v;
							if (diff < 0) diff = -diff;
							if (diff > maxv) maxv = diff;
						}
					}

					int vv = maxv + 60;
					if (vv > 255) vv = 255;   // clip
					if (vv < 0)   vv = 0;

					d[drow + x] = (unsigned char)vv;
					continue;
				}

				//PixelNode+ head = NULL;
				//PixelNode* tail = NULL;
				int sum = 0;
				// -------------------------------------------------------
				// 3) MASK
				for (int ky = -R; ky <= R; ++ky)
				{
					const int sy = y + ky;
					// 경계 처리: sy를 0~h-1로 클램프
					const int cy = (sy < 0) ? 0 : (sy >= h ? h - 1 : sy);

					for (int kx = -R; kx <= R; ++kx)
					{
						const int sx = x + kx;
						// 경계 처리: sx를 0~w-1로 클램프
						const int cx = (sx < 0) ? 0 : (sx >= w ? w - 1 : sx);

						// 커널 인덱스(N*N 크기)
						const int kidx = (ky + R) * N + (kx + R);

						// 누적합
						sum += (int)s[(size_t)cy * (size_t)w + (size_t)cx] * K[kidx];
					}
				}

				// 바이어스 적용
				sum += bias;

				// 시프트 적용
				if (shift > 0) sum >>= shift;
				//free_pixel_list(head);
				////////////////////////////////////////////////////
				d[drow + (size_t)x] = clip3_u8_int((int)sum);
			}
		}
	}

	
}
