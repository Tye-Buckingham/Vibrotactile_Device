#ifndef KNN_H
#define KNN_H

#include <limits.h>

#include "../Misc/tl_includes.h"
#include "../DTW/dtw.h"
#include "../MFCCs/mfccs.h"

struct Guess {
	int guess;
	long double diff;
	int ref_indx;
	struct Phoneme* ref;
};

int knn_mfccs_size(float* test, int test_length, int k);

#endif
