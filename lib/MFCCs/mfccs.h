#ifndef MFCCS_H
#define MFCCS_H

#include "../Misc/tl_includes.h"
#include "../Feature/mfcc.h"

struct Ph_index {
	unsigned char i;
	char group[5];
	const char* name;
};

struct Phoneme {
	struct Ph_index* index;
	double score;
	float** mfcc;
	short* size;
	short* use_size;
	short* count;
	short size_count;
	short use_count;
};

struct Phoneme* get_mfcc(struct Phoneme* phone, int length);
void clear_mfccs(struct Phoneme* phone);
void dtw_init(void);

extern int truncation;
extern struct Phoneme** phones;
extern const char* p_codes[];

#endif