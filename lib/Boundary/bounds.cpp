#include "bounds.h"
#include <Arduino.h>

int is_positive(short num)
{
	if(num > 0) return 1;
	else return 0;
	
	return -1;
}

int f_is_positive(float num)
{
	if(num > 0) return 1;
	else return 0;
	
	return -1;
}

float get_entropy(short* sequence, int inc)
{
	float entropy = 0;
	for(int m = 0; m < inc; m++) {
		if(sequence[m] == 0) {
			sequence[m] = 1;
		}
		sequence[m] = abs(sequence[m]);
		if(sequence[m] == -32768) {
			sequence[m] = 32767;
		}
		entropy += (float)(sequence[m]) * (log(sequence[m]) / log(2));
	}
	return entropy;
}


float short_time_energy(short* signal, int signal_length, int window_length)
{
	float result = 0;

	for(int i = 0; i < signal_length; i++) {
		result += (signal[i] * signal[i]);
	}
	return result;
}

int cross_rate(short* signal, int signal_length)
{

	int result = 0;
	for(int i = 0; i < signal_length - 1; i++) {
		if(is_positive(signal[i]) != is_positive(signal[i+1])) {
			result++;	
		}
	}
	
	return result;
}

int next_boundary(short* sequence, int length)
{
	if(length <= 32)
		return -1;
	int inc = 32;
	float change = 0, last_zc = 0, ste_change = 0, last_ste = 0; 
	float entropy = 0, prev_entropy = 0, entropy_change = 0;
	int gap = 0;
	for(int i = inc; i < length - inc; i+=inc) {
		entropy = 0;
		float zc = cross_rate(&sequence[i], inc);
		change = abs(last_zc - zc);

		float ste = short_time_energy(&sequence[i], inc, inc);
		if(last_ste == 0) {
			last_ste = 1;
		}
		ste_change = ((ste-last_ste) / fabs(last_ste)) * 100;
		
		entropy = get_entropy(&sequence[i], inc); 
		if(prev_entropy == 0) {
			entropy_change = fabs(entropy - prev_entropy);
		} else {
			entropy_change = ((entropy-prev_entropy) / fabs(prev_entropy)) * 100;
		}
		
		if((i != 32) && (gap > 6) && 
		   ((change >= 4 && ste_change <= -75 && entropy_change <= -80) ||
		    (ste_change > 425 && entropy_change > 1050))) {
			return i;
		}
		gap++;
		prev_entropy = entropy;
		last_zc = zc;
		last_ste = ste;
		change = 0;
	}
	return -1;
}

int shift_and_reduce(short* sequence, int length, int shift)
{
	for(int i = 0; i < shift; i++) {
		sequence[i] = 0;			  
	}
	for(int i = shift; i < length; i++) {
		sequence[i - shift] = sequence[i];
		sequence[i] = 0;
	}
	return (length - shift);
}
