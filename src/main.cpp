/*
 *
 * @file main.cpp
 * @brief The setup and control functins for each of the threds; me recordng and one processing.
 * 
 * @author T. Buckingham
 * 
 * As these methods mostly re-implentations of the functions found in the dekstop application the same
 * documentation and comments apply.
 * 
 */
#ifdef DEBUG //Remove compiler optimizations for hardware debugging
#pragma GCC optimize ("O0") 
#endif

#include <Arduino.h>

#include <Audio.h>
#include "TeensyDebug.h"
#include <Wire.h>
#include <SD.h>
#include <SPI.h>
#include <SerialFlash.h>
#include <TeensyThreads.h>

#include "../Misc/tl_includes.h"
#include "../Test/test.h"

#define AUDIO_SAMPLE_RATE_EXACT 16000.0f // Redefine so its the same as the data-set we used
#define WAIT_FOR_DEBUGGER_SECONDS 120
#define WAIT_FOR_SERIAL_SECONDS 20

// EXTMEM uint8_t buffer3[3 * 1024 * 1024];

extern "C" uint8_t external_psram_size;

extern "C"
{

	int _open(void)
	{
		return 0;
	}
}

ThreadWrap(Serial, SerialXtra);
#define Serial ThreadClone(SerialXtra)

// GUItool: begin automatically generated code
AudioInputI2S i2s1;		// xy=253,288
AudioRecordQueue queue; // xy=484,292
AudioConnection patchCord1(i2s1, 0, queue, 0);
AudioControlSGTL5000 sgtl5000_1; // xy=369,427
// GUItool: end automatically generated code

#define SDCARD_CS_PIN 10
#define SDCARD_MOSI_PIN 7
#define SDCARD_SCK_PIN 14

void record(void);
int check_in(void);

void process_thread();
void record_thread();
void update_arrays(int in, int pr);
void boundary(void);

void setup()
{
	Serial.begin(9600);
	while (!Serial)
		;
	delay(50);

	bool debug_avail=false;
    bool serial_avail=false;

	#ifdef DEBUG
    {   
        SerialUSB1.begin(19200);
        for (uint8_t i = 0; i < (WAIT_FOR_DEBUGGER_SECONDS); i++)
        {
			Serial.println("Trying to connect debugger...");
            digitalWrite(LED_BUILTIN,true);
            delay(800);
            digitalWrite(LED_BUILTIN,false);
            delay(200);
            if (SerialUSB1) {
                debug_avail=true;
                break;
            }
        }
        if (debug_avail)
        {
            debug.begin(SerialUSB1);
            delay(100);
            halt_cpu();  
            delay(100);
            debug.printf("Debugger is connected. Connect USB serial monitor now ...\n");
        }  
    }
	#endif

	Sd2Card card;
	SdVolume volume;
	File f1, f2, f3, f4;
	boolean status;
	elapsedMicros usecTotal, usecSingle;
	float size;


	SPI.setMOSI(SDCARD_MOSI_PIN);
	SPI.setSCK(SDCARD_SCK_PIN);

	Serial.begin(9600);

	status = card.init(SPI_FULL_SPEED, SDCARD_CS_PIN) ;
	
	status = volume.init(card);
	if (!status)
	{
		Serial.println("Failed to access the file system");
		return;
	}

	size = volume.blocksPerCluster() * volume.clusterCount();
	size = size * (512.0 / 1e6); 

	status = SD.begin(SDCARD_CS_PIN);

	dtw_init();

	AudioMemory(8);
	sgtl5000_1.enable();
	sgtl5000_1.volume(0.5);
	sgtl5000_1.inputSelect(AUDIO_INPUT_MIC);
	sgtl5000_1.micGain(36);

	threads.addThread(record_thread);
	threads.addThread(process_thread);
	delay(1000);
}

Threads::Mutex lock;

void loop()
{

}

short in_array[32000] = {0}; /* Recorded samples */
short pr_array[32000] = {0}; /* Recorded samples to be processed*/

int in_size = 0;
int pr_size = 0;

int STOP = 0;


void record_thread()
{
	while (1)
	{
		record();
	}

	return;

}

void process_thread()
{

	while (1)
	{
		check_in();
	}

	return;
}

int check_in(void)
{
	if (in_size > 128)
	{
		update_arrays(1, 0);
		return 0;
	}

	return -1;
}

void update_arrays(int in, int pr)
{
	while(lock.try_lock() == 0) {
		delay(1);
	}
	
	if (pr == 0)
	{
		if (pr_size += in_size >= 32000)
		{
			pr_size = 0;
		}
		// move the in_buffer to the end of the process_buffer
		for (int i = 0; i < in_size; i++)
		{
			pr_array[i + pr_size] = in_array[i];
		}

		// zero out the entire buffer
		for (int i = 0; i < in_size; i++)
		{
			in_array[i] = 0;
		}

		// reset the size to allow appending to the start
		pr_size += in_size;
		in_size = 0;

		boundary(); //  { do_seperation -> knn&&dtw }
	}
	else
	{
		if (in_size + 10 >= 32000)
		{
			in_size = 0;
		}
		in_array[in_size] = in;
		in_size++;
	}
	lock.unlock();

	return;
}

void record(void)
{
	queue.begin();
	elapsedMicros rectime;

	while (rectime < 100000)
	{
	}
	queue.end();

	if (queue.available() >= 2)
	{
		byte buffer[512];
		memcpy(buffer, queue.readBuffer(), 256);
		queue.freeBuffer();
		memcpy(buffer + 256, queue.readBuffer(), 256);
		queue.freeBuffer();
		queue.clear();

		for (int i = 1; i < 512; i++)
		{
			update_arrays(buffer[i], 1);
		}
	}
	return;
}

void boundary(void)
{
	int bound = 0;
	while (1)
	{
		bound = next_boundary(pr_array, pr_size);
		if (bound == 0)
		{
			pr_size = shift_and_reduce(pr_array, pr_size, 32);
			continue;
		}
		if (bound == -1)
		{
			break;
		}
		short *h = (short *)calloc(bound, sizeof(short));
		for (int i = 0; i < bound; i++)
		{
			h[i] = pr_array[i];
		}
		int result = test_phoneme_utterance(h, bound);
		if(result < 0) {
			Serial.println(F("Sil")); 
		} else {
			Serial.println(phones[result]->index->name);
		}
		pr_size = shift_and_reduce(pr_array, pr_size, bound);
	}

	return;
}
