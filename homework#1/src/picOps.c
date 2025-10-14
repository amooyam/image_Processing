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

	// 1) ������׷� ī��Ʈ (0~255)
	int count[256] = { 0 };
	const size_t N = (size_t)W * (size_t)H;
	const unsigned char* s = src->data;
	for (size_t i = 0; i < N; ++i) {
		++count[(unsigned int)s[i]];
	}


	// 2) �ִ밪 (�����ϸ��� ���)
	int maxVal = 0;
	//����	����
	for(size_t i = 0; i < 256; ++i) {
		if(maxVal < count[i]) maxVal = count[i];
	}
	//printf("maxVal : %d\n", maxVal);
	//

	// 3) ��� �̹��� �ʱ�ȭ(����)
	const int outW = 256, outH = 256;
	memset(hist->data, 0, (size_t)outW * (size_t)outH);

	if (maxVal <= 0) {
		// ��� �ȼ��� 0���� ���: �׳� ���� ȭ�� ����
		return;
	}

	// 4) ���� ���� �׸��� (���=255)
	//    x��: ���(0~255), y��: ��(���=�ִ�)
	for (int x = 0; x < 256; ++x) {
		// ����: 0..(outH-1), ���� ������ (�ݿø� ���� ����)
		int height = 0;
		//����	����
		height = count[x] * 255 / maxVal;
		//printf("lumi:%d, cnt:%d, scaling:%d\n", x, count[x], height);
		// 
		// 

		// �Ʒ����� ���� ä���
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

	// (1) ������׷�(0~255)
	int hist[256] = { 0 };
	const unsigned char* s = src->data;
	for (size_t i = 0; i < N; ++i) {
		++hist[(unsigned)s[i]];
	}

	// (2) CDF (��������)
	unsigned int cdf[256];
	cdf[0] = (unsigned int)hist[0];
	//���� ����
	for(int i = 1; i < 256; ++i) {
		cdf[i] = cdf[i-1] + (unsigned int)hist[i];
	}
	//printf("end cdf : %d", cdf[255]); //262144�� ���;��ϴµ�
	//


	// (3) LUT �ۼ�
	// round(255.0 * cdf[val] / N) val : 0~255
	// ���� �ݿø�: (cdf*255 + N/2) / N
	unsigned char LUT[256];
	//���� ����
	for(int i = 0; i < 256; ++i) {
		LUT[i] = clip3_u8_int((int)((cdf[i] * 255 + N / 2) / N));
		//��Ȱȭ�� �������� 0~1 ���� ������ �����(: / �� ����), ���� ���� ��� ���� ���� ������ ������ ����(: * 255)
	}
	//

	// (4) ����
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

	// --- 0) �غ� ���� ---
	int    histInput[256] = { 0 };   // �Է� ������׷�
	int    histTarget[256] = { 0 };   // ��ǥ ������׷�(���� ������)
	double cdfInput[256] = { 0.0 }; // �Է� CDF (0~1)
	double cdfTarget[256] = { 0.0 }; // ��ǥ CDF (0~1)
	unsigned char mapLUT[256];      // ���� LUT: �Է� ���� -> ��� ����

	const unsigned char* s = src->data;
	unsigned char* d = dst->data;

	// --- 1) �Է� ������׷� ��� ---
	for (size_t i = 0; i < N; ++i) {
		++histInput[(unsigned int)s[i]];
	}

	// --- 2) ��ǥ ������׷� ����(���� ������) ---
	// sumOfSequence = 0 + 1 + ... + 255 = (255*256)/2
	const int sumOfSequence = (255 * 256) / 2; // = 32640
	const double factor = (double)N / (double)sumOfSequence;

	for (int i = 0; i < 256; ++i) {
		// (int)(i * factor + 0.5)  : �ݿø�
		histTarget[i] = (int)((double)i * factor + 0.5);
	}

	// �ʿ� �� Ŀ���� Ÿ���� ��(�ּ�):
	//for (int i = 128; i < 256; ++i) histTarget[i] = 2048;

	// --- 3) CDF ���(����ȭ: �������� ���� 0~1 ����) ---
	// �Է� CDF
	// ����	����
	cdfInput[0] = (double)histInput[0] / N;
	for(int i = 1; i < 256; ++i) {
		cdfInput[i] = cdfInput[i-1] + (double)histInput[i] / N;
	}
	// 
	
	// ��ǥ CDF (Ÿ�� ���� N�� �ǵ��� ������ �����̹Ƿ� �����ϰ� N���� ����)
	// ����	����
	cdfTarget[0] = (double)histTarget[0] / N;
	for(int i = 1; i < 256; ++i) {
		cdfTarget[i] = cdfTarget[i-1] + (double)histTarget[i] / N;
	}
	//

	// --- 4) ���� LUT ���� ---
	for (int i = 0; i < 256; ++i) {
		int j = 0;
		// cdfTarget[j] < cdfInput[i] ���� ���� �� ó������ cdfTarget[j]��
		// cdfInput[i] �̻��� �Ǵ� �ּ� j�� ���� (������ ����)
		// ����	����
		while(cdfTarget[j] < cdfInput[i]) ++j;
		//
		//
		if (j > 255) j = 255; // ��� ���
		mapLUT[i] = (unsigned char)j;
	}

	// --- 5) ����(�ȼ� ġȯ) ---
	for (size_t i = 0; i < N; ++i) {
		d[i] = mapLUT[s[i]];
	}
}

