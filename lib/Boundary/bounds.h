#ifndef BOUNDS_H
#define BOUNDS_H

#include <math.h>

#include "../Misc/tl_includes.h"
#include "../Misc/mfcc_vars.h"

int shift_and_reduce(short* sequence, int length, int shift);
int next_boundary(short* sequence, int length);
float get_entropy(short* sequence, int inc);
int next_boundary(short* sequence, int length);
int is_positive(short num);
int f_is_positive(float num);

#endif
