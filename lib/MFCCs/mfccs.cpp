#include "mfccs.h"
#include <Arduino.h>
#include <SPI.h>
#include <SD.h>
#include <Audio.h>

struct Phoneme** phones;

int truncation = 0;

extern unsigned long _heap_start;
extern unsigned long _heap_end;
extern char *__brkval;

// const char* p_codes[] = {"b", "d", "k", "p", "t", "g",                                                       // Stops [1 - 6] - OBSTRUENT
// 	                     "jh", "ch",                                                                         // Affri [7 - 8] - OBSTRUENT
// 			             "s", "sh", "th", "v", "f", "dh", "z",                                               // Frics [9 - 15] - OBSTRUENT
// 			             "m", "n", "ng",                                                                     // Nasal [16 - 18] - SONORANT
// 			             "l", "r", "hh", "w", "y",                                                           // Semiv [19 - 23] - SONORANT
// 	                     "aa", "ae", "ah", "aw", "er", "ay", "eh", "ey", "ih", "iy", "ow", "oy", "uh", "uw", // Vowel [24 - 37] - SONORANT
// 			             "sil",                                                                              // Other [38] - OTHER
// 	                "\0"};
const char* p_codes[] = { "b", "jh", "s", "l", "aa", "\0" };


int freeram() {
  return (char *)&_heap_end - __brkval;
}


void load_from_file(struct Phoneme* phone)
{
	
	// char base[16];
	// // Serial.print(F("Loading :: "));
	// // Serial.println(phone->index->name);
	// sprintf(base, "/device/%s", phone->index->name);

	char filepath[32];

	sprintf(filepath, "/device/%s/%s.conf", phone->index->name, phone->index->name);
	File mfcc_file = SD.open(filepath, FILE_READ);
	if(!mfcc_file) {
		return;
	}
	short line_num = 0, cnt = 0;
	char c;
	String buff;
	while(mfcc_file.available()) {
		c = mfcc_file.read();
		if(c == '\n') {
			line_num++;
			cnt = 0;
			buff = "";
			continue;
		}
		if(c != ' ') {
			buff += c;
			continue;
		}
		if(buff == "") {
			continue;
		}
		if(line_num == 0) {
			phone->size_count = (short)buff.toInt();
			phone->count = (short*)malloc(phone->size_count * sizeof(short));
			phone->size = (short*)malloc(phone->size_count * sizeof(short));
		} else if(line_num == 1) {
				phone->size[cnt] = (short)buff.toInt();
				cnt++;
		} else {
				phone->count[cnt] = (short)buff.toInt();
				cnt++;
		}
		buff = "";
	}
	mfcc_file.close();
	return;
}

struct Phoneme* get_mfcc(struct Phoneme* phone, int test_length)
{
	
	char base[32];
	
	clear_mfccs(phone);
	
	sprintf(base, "/device/%s/", phone->index->name);

	char filename[64];
	int count = 0;
	for(int i = 0; i < phone->size_count; i++) {
		if(phone->size[i] == test_length) {
			count++;
		}
	}
	if(count == 0) {
		return phone;
	}
	phone->mfcc = (float**)malloc(count * sizeof(float*));
	if(phone->mfcc == NULL) {
			// Serial.println(F("Error making mfcc"));
	}
	count = 0;
	File mfcc_file;
	for(int i = 0; i < phone->size_count; i++) {
		if(phone->size[i] != test_length) {
			continue;
		}
		sprintf(filename, "/device/%s/%d_%d.phn", phone->index->name, phone->size[i], count);
		phone->mfcc[phone->use_count] = (float*)calloc(phone->size[i], sizeof(float));
		if(phone->mfcc[phone->use_count] == NULL) {
			// Serial.println(F("Error making mfcc"));
		}
		mfcc_file = SD.open(filename, FILE_READ); 
		
		if(!mfcc_file) {
				// Serial.print(F("Failed to open "));
				// Serial.println(filename);
				return NULL;
		}
		int n = 0, t = 0;
		char* end_ptr;
		char c;
		char buff[32] = {'\0'};
		while(mfcc_file.available()) {
			int s = mfcc_file.read();
			if(s == -1) {
				break;
			}
			c = (char)s;
			if(c == '\n') {
				break;
			}
			if(c != ' ') {
				buff[t] = c;
				t++;
				continue;
			} else {
				for(int i = 0; i < t; i++) {
					buff[i] = '\0';
				}
				t = 0;
				continue;
			}
			phone->mfcc[phone->use_count][n] = strtof(buff, &end_ptr);
			for(int i = 0; i < t; i++) {
				buff[i] = '\0';
			}
			t = 0;
			n++;
		}
		n = 0;
		phone->use_count++;
		count++;
		mfcc_file.close();
		
	}
	return phone;
}

 
void dtw_init(void)
{
	int j = 0;
	while(strcmp(p_codes[j], "\0") != 0) {
		j++;	
	}
	num_ph = j;
	phones = (struct Phoneme**)malloc(j * sizeof(struct Phoneme*));
	for(int i = 0; i < j; i++) {
		phones[i] = (struct Phoneme*)malloc(sizeof(struct Phoneme));
		phones[i]->mfcc = (float**)malloc(sizeof(float*));
		phones[i]->index = (struct Ph_index*)malloc(sizeof(struct Ph_index));
		phones[i]->index->i = i; 
		phones[i]->index->name = p_codes[i];
		phones[i]->size_count = 0;
		phones[i]->use_count = 0;
		load_from_file(phones[i]);
	}
	return;
}

void clear_mfccs(struct Phoneme* phone)
{
	for(int i = 0; i < phone->use_count; i++) {
		free(phone->mfcc[i]);
	}
	free(phone->mfcc);
	phone->use_count = 0;

	return;
}

