#ifndef MFCC_H
#define MFCC_H

#include <complex.h>
#include <float.h>

#include "../Misc/tl_includes.h"
#include "../Misc/mfcc_vars.h"

int frame_amount(int signal_length);
int mfcc_size(int signal_length);
float* mfcc_quick(float* sequence, int width, int incr, int banks, int paa);

#endif
