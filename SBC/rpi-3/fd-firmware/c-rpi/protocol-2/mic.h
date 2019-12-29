//#ifndef _MIC_H
//#define _MIC_H


#define MIC_SAMPLES 64

extern void start_mic_thread();
extern int audio_open_input(char* hw);
extern void audio_close_input();
//static void *mic_read_thread(void *arg);

//#endif