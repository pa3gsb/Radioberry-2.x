//#ifndef _AUDIO_H
//#define _AUDIO_H

extern int audio_open_output(char* hw);
extern void audio_close_output();
extern int audio_write(unsigned char* buffer);
extern void audio_get_cards();
//#endif
