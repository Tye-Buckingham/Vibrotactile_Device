#include <Arduino.h>
#include "dtw.h"

int LOOP_TEST = 0;

int max(int a, int b)
{
	if(a > b) { return a; } else { return b; }
}

int min(int a, int b)
{
	if(a < b) { return a; } else { return b; }
}

void init_dtw_matrix(double dtw_matrix[glbl_frame_limit][glbl_frame_limit])
{

	for(int i = 0; i < glbl_frame_limit; i++) {
		for(int j = 0; j < glbl_frame_limit; j++) {
			dtw_matrix[i][j] = DBL_MAX;
		}
	}
	dtw_matrix[0][0] = 0;
	for(int i = 0; i < glbl_frame_limit; i++) {
		for(int j = 0; j < glbl_frame_limit; j++) {
			dtw_matrix[i][j] = 0;
		}
	}
	return;
}

long double dtw_frame_result(float* signal, int signal_length, struct Phoneme* phoneme, int p)
{
	phoneme->score = 0;
	long double temp_last_min = 0, last_min = 0;
	int phone_length = 0;
	int w = 0;
	long double final_score = 0;
	int trunc = floor((glbl_banks) * glbl_test_trunc);
	signal_length = frame_amount(signal_length);
	phone_length = signal_length;
	
	long double score = 0;
	long double cost = 0;
	int largest = max(signal_length, phone_length);
	float win = glbl_dtw_window;
	w = floor(win * (float)largest);
	if(signal_length == 0) {
		return -10;
	}
	if(phone_length == 0) {
		return -11;
	}
	double dtw_matrix[glbl_frame_limit][glbl_frame_limit] = {0.0f};
	init_dtw_matrix(dtw_matrix);
	for(int i = 1; i <= signal_length; i++) {
		for(int j = max(1, i-w); j <= min(phone_length, i+w); j++) {
			for(int m = 0; m < trunc; m++) {
				cost += fabsl(signal[(i * trunc) +  m] - phoneme->mfcc[p][(j * trunc) +  m]);
			}
			temp_last_min = fminl(dtw_matrix[i-1][j], dtw_matrix[i][j-1]);
			last_min = fminl(temp_last_min, dtw_matrix[i-1][j-1]);
			dtw_matrix[i][j] = cost + last_min;
			score = dtw_matrix[i][j];
			cost = 0;
		}
	}
	final_score = score;	

	return final_score;
}