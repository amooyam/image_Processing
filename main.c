#include "common.h"
#include "picIO.h"
#include "picOps.h" //Picture Operations

// ---- ����� ���� ----
#ifndef IN_PATH
#define IN_PATH   "lena_512x512.raw"
#endif
#ifndef OUT_PATH
#define OUT_PATH  "rPlus.raw"
#endif
#ifndef WIDTH
#define WIDTH     512
#endif
#ifndef HEIGHT
#define HEIGHT    512
#endif
// ---------------------

int main(void) {
	// 1) �Է� ���� �ε�
	ImageY8 src = { 0 };
	image_load_raw_y8(IN_PATH, WIDTH, HEIGHT, &src);

	// 2) ��� ���� �Ҵ�
	ImageY8 dst_Plus = { 0 };
	image_alloc(&dst_Plus, WIDTH, HEIGHT);
	ImageY8 dst_Minus = { 0 };
	image_alloc(&dst_Minus, WIDTH, HEIGHT);
	ImageY8 dst_Multi = { 0 };
	image_alloc(&dst_Multi, WIDTH, HEIGHT);
	ImageY8 dst_Divi = { 0 };
	image_alloc(&dst_Divi, WIDTH, HEIGHT);

	ImageY8 dst_Hist = { 0 };
	image_alloc(&dst_Hist, 256, 256);

	//������� ������׷�
	ImageY8 dst_HistPlus = { 0 };
	image_alloc(&dst_HistPlus, 256, 256);
	ImageY8 dst_HistMinus = { 0 };
	image_alloc(&dst_HistMinus, 256, 256);
	ImageY8 dst_HistMulti = { 0 };
	image_alloc(&dst_HistMulti, 256, 256);
	ImageY8 dst_HistDivi = { 0 };
	image_alloc(&dst_HistDivi, 256, 256);

	//Equalization ������׷�
	ImageY8 dst_Equal = { 0 };
	image_alloc(&dst_Equal, WIDTH, HEIGHT);
	ImageY8 dst_HistEqual = { 0 };
	image_alloc(&dst_HistEqual, 256, 256);
	//


	//Histogram Matching(specification) �Ҵ�
	//

	// 3) ����
	image_Arithmetic_PLUS(&src, &dst_Plus);
	image_Arithmetic_MINUS(&src, &dst_Minus);
	image_Arithmetic_Multi(&src, &dst_Multi);
	image_Arithmetic_Divi(&src, &dst_Divi);

	image_histogram_256(&src, &dst_Hist);

	image_histogram_256(&dst_Plus, &dst_HistPlus);
	image_histogram_256(&dst_Minus, &dst_HistMinus);
	image_histogram_256(&dst_Multi, &dst_HistMulti);
	image_histogram_256(&dst_Divi, &dst_HistDivi);
	
	
	//Equalization ȣ��
	// ���� ����
	image_equalization(&src, &dst_Equal);
	image_histogram_256(&dst_Equal, &dst_HistEqual);
	// 
	
	//Histogram Matching(specification) ȣ��
	// ���� ����
	//

	// 4) ����
	image_save_raw_y8(OUT_PATH, &dst_Plus);
	image_save_raw_y8("rMinus.raw", &dst_Minus);
	image_save_raw_y8("rMulti.raw", &dst_Multi);
	image_save_raw_y8("rDivi.raw", &dst_Divi);

	image_save_raw_y8("src_hist_256x256.raw", &dst_Hist);

	image_save_raw_y8("rplus_hist_256x256.raw", &dst_HistPlus);
	image_save_raw_y8("rminus_hist_256x256.raw", &dst_HistMinus);
	image_save_raw_y8("rmulti_hist_256x256.raw", &dst_HistMulti);
	image_save_raw_y8("rdivi_hist_256x256.raw", &dst_HistDivi);

	image_save_raw_y8("equalized_hist.raw", &dst_Equal);
	image_save_raw_y8("equalized_hist_256x256.raw", &dst_HistEqual);


	// 5) ����
	image_free(&src);
	image_free(&dst_Plus);
	image_free(&dst_Minus);
	image_free(&dst_Divi);
	image_free(&dst_Multi);

	image_free(&dst_Hist);

	image_free(&dst_HistPlus);
	image_free(&dst_HistMinus);
	image_free(&dst_HistMulti);
	image_free(&dst_HistDivi);

	image_free(&dst_Equal);
	image_free(&dst_HistEqual);


	return 0;
}
