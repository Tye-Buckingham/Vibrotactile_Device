#ifndef TEST_H
#define TEST_H

#include <float.h>

#include "../Misc/tl_includes.h"
#include "../Boundary/bounds.h"
#include "../Feature/mfcc.h"
#include "../MFCCs/mfccs.h"
#include "../KNN/knn.h"

int test_phoneme_utterance(short* h, int signal_length);

#endif
