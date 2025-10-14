#include "picOps.h"
#include <string.h> // memcpy


void image_Arithmetic_MINUS(const ImageY8* src, ImageY8* dst)
{
	const size_t N = (size_t)src->info.width * (size_t)src->info.height;
	for (size_t i = 0; i < N; ++i) {
		{
			int v = (int)src->data[i] - 30;
			dst->data[i] = clip3_u8_int(v);
		}
	}
}

void image_Arithmetic_Multi(const ImageY8* src, ImageY8* dst)
{
	const size_t N = (size_t)src->info.width * (size_t)src->info.height;
	for(size_t i = 0; i < N; ++i) {
		int v = (int)src->data[i] * 1.2 + 0.5;
		dst->data[i] = clip3_u8_int(v);
	}
}

void image_Arithmetic_Divi(const ImageY8* src, ImageY8* dst)
{
	const size_t N = (size_t)src->info.width * (size_t)src->info.height;
	for(size_t i = 0; i < N; ++i) {
		int v = (int)src->data[i] / 1.2 + 0.5;
		dst->data[i] = clip3_u8_int(v);
	}
}


void image_histogram_256(const ImageY8* src, ImageY8* hist)
{
	const int W = src->info.width;
	const int H = src->info.height;

	// 1) 히스토그램 카운트 (0~255)
	int count[256] = { 0 };
	const size_t N = (size_t)W * (size_t)H;
	const unsigned char* s = src->data;
	for (size_t i = 0; i < N; ++i) {
		++count[(unsigned int)s[i]];
	}


	// 2) 최대값 (스케일링에 사용)
	int maxVal = 0;
	//직접	구현
	for(size_t i = 0; i < 256; ++i) {
		if(maxVal < count[i]) maxVal = count[i];
	}
	//printf("maxVal : %d\n", maxVal);
	//

	// 3) 출력 이미지 초기화(검정)
	const int outW = 256, outH = 256;
	memset(hist->data, 0, (size_t)outW * (size_t)outH);

	if (maxVal <= 0) {
		// 모든 픽셀이 0개인 경우: 그냥 검정 화면 유지
		return;
	}

	// 4) 세로 막대 그리기 (흰색=255)
	//    x축: 밝기(0~255), y축: 빈도(상단=최대)
	for (int x = 0; x < 256; ++x) {
		// 높이: 0..(outH-1), 정수 스케일 (반올림 없이 비율)
		int height = 0;
		//직접	구현
		height = count[x] * 255 / maxVal;
		//printf("lumi:%d, cnt:%d, scaling:%d\n", x, count[x], height);
		// 
		// 

		// 아래에서 위로 채우기
		for (int y = 0; y < height; ++y) {
			size_t idx = (size_t)(outH - 1 - y) * (size_t)outW + (size_t)x;
			hist->data[idx] = 255u;
		}
	}

}

void image_equalization(const ImageY8* src, ImageY8* dst)
{
	const int W = src->info.width;
	const int H = src->info.height;
	const size_t N = (size_t)W * (size_t)H;

	// (1) 히스토그램(0~255)
	int hist[256] = { 0 };
	const unsigned char* s = src->data;
	for (size_t i = 0; i < N; ++i) {
		++hist[(unsigned)s[i]];
	}

	// (2) CDF (누적분포)
	unsigned int cdf[256];
	cdf[0] = (unsigned int)hist[0];
	//직접 구현
	for(int i = 1; i < 256; ++i) {
		cdf[i] = cdf[i-1] + (unsigned int)hist[i];
	}
	//printf("end cdf : %d", cdf[255]); //262144개 나와야하는디
	//


	// (3) LUT 작성
	// round(255.0 * cdf[val] / N) val : 0~255
	// 정수 반올림: (cdf*255 + N/2) / N
	unsigned char LUT[256];
	//직접 구현
	for(int i = 0; i < 256; ++i) {
		LUT[i] = clip3_u8_int((int)((cdf[i] * 255 + N / 2) / N));
		//평활화는 누적합을 0~1 사이 값으로 만들고(: / 총 개수), 가장 높은 밝기 값을 곱해 적당히 구간을 나눔(: * 255)
	}
	//

	// (4) 적용
	unsigned char* d = dst->data;
	for (size_t i = 0; i < N; ++i) {
		d[i] = LUT[s[i]];
	}
}


void image_histmatch(const ImageY8* src, ImageY8* dst)
{
	if (!src || !dst || !src->data || !dst->data) return;

	const int  W = src->info.width;
	const int  H = src->info.height;
	const size_t N = (size_t)W * (size_t)H;

	// --- 0) 준비 버퍼 ---
	int    histInput[256] = { 0 };   // 입력 히스토그램
	int    histTarget[256] = { 0 };   // 목표 히스토그램(선형 증가형)
	double cdfInput[256] = { 0.0 }; // 입력 CDF (0~1)
	double cdfTarget[256] = { 0.0 }; // 목표 CDF (0~1)
	unsigned char mapLUT[256];      // 매핑 LUT: 입력 계조 -> 출력 계조

	const unsigned char* s = src->data;
	unsigned char* d = dst->data;

	// --- 1) 입력 히스토그램 계산 ---
	for (size_t i = 0; i < N; ++i) {
		++histInput[(unsigned int)s[i]];
	}

	// --- 2) 목표 히스토그램 구성(선형 증가형) ---
	// sumOfSequence = 0 + 1 + ... + 255 = (255*256)/2
	const int sumOfSequence = (255 * 256) / 2; // = 32640
	const double factor = (double)N / (double)sumOfSequence;

	for (int i = 0; i < 256; ++i) {
		// (int)(i * factor + 0.5)  : 반올림
		histTarget[i] = (int)((double)i * factor + 0.5);
	}

	// 필요 시 커스텀 타깃의 예(주석):
	//for (int i = 128; i < 256; ++i) histTarget[i] = 2048;

	// --- 3) CDF 계산(정규화: 총합으로 나눠 0~1 범위) ---
	// 입력 CDF
	// 직접	구현
	cdfInput[0] = (double)histInput[0] / N;
	for(int i = 1; i < 256; ++i) {
		cdfInput[i] = cdfInput[i-1] + (double)histInput[i] / N;
	}
	// 
	
	// 목표 CDF (타깃 합이 N이 되도록 설계한 상태이므로 동일하게 N으로 나눔)
	// 직접	구현
	cdfTarget[0] = (double)histTarget[0] / N;
	for(int i = 1; i < 256; ++i) {
		cdfTarget[i] = cdfTarget[i-1] + (double)histTarget[i] / N;
	}
	//

	// --- 4) 매핑 LUT 생성 ---
	for (int i = 0; i < 256; ++i) {
		int j = 0;
		// cdfTarget[j] < cdfInput[i] 동안 전진 → 처음으로 cdfTarget[j]가
		// cdfInput[i] 이상이 되는 최소 j를 선택 (단조성 보장)
		// 직접	구현
		while(cdfTarget[j] < cdfInput[i]) ++j;
		//
		//
		if (j > 255) j = 255; // 경계 방어
		mapLUT[i] = (unsigned char)j;
	}

	// --- 5) 적용(픽셀 치환) ---
	for (size_t i = 0; i < N; ++i) {
		d[i] = mapLUT[s[i]];
	}
}

