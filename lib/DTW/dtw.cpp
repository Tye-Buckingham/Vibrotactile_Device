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

double** init_dtw_matrix(int signal_length, int phone_length, int w)
{
	double** dtw_matrix = (double**)malloc(signal_length *  sizeof(double*));
	if(dtw_matrix == NULL) {
		return NULL;
	}
	for(int i = 0; i < signal_length; i++) {
		dtw_matrix[i] = (double*)malloc(phone_length * sizeof(double));
	}
	for(int i = 0; i < signal_length; i++) {
		for(int j = 0; j < phone_length; j++) {
			dtw_matrix[i][j] = DBL_MAX;
		}
	}
	dtw_matrix[0][0] = 0;
	for(int i = 0; i < signal_length; i++) {
		for(int j = 0; j < phone_length; j++) {
			dtw_matrix[i][j] = 0;
		}
	}
	return dtw_matrix;
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
	double** dtw_matrix = init_dtw_matrix(signal_length, phone_length, w);
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
	for(int i = 0; i < signal_length; i++) {
		free(dtw_matrix[i]);
	}
	free(dtw_matrix);
	return final_score;
}