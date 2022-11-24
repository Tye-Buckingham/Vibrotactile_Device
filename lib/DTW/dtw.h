#ifndef DTW_H
#define DTW_H

#include <float.h>

#include "../Misc/tl_includes.h"
#include "../MFCCs/mfccs.h"
#include "../Misc/mfcc_vars.h"


long double dtw_frame_result(float* signal, int signal_length, struct Phoneme* phoneme, int p);

#endif

