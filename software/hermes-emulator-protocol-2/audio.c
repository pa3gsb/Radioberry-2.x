
#include <alsa/asoundlib.h>
#include "audio.h"
#include "hermeslite.h"

static int audio_buffer_size = 256; 	// samples (both left and right)
static int audioseq=-1;

#define AUDIO_SAMPLES 63
#define AUDIO_SAMPLE_SIZE 2
#define AUDIO_CHANNELS 2
#define AUDIO_BUFFERS 10
#define OUTPUT_BUFFER_SIZE (AUDIO_SAMPLE_SIZE*AUDIO_CHANNELS*audio_buffer_size)

snd_pcm_t *playback_handle=NULL;
unsigned char *playback_buffer;
int playback_offset;
int low_latency;

int audio_write(unsigned char* buffer) {
	snd_pcm_sframes_t delay;
	int error;
	long trim;
	
	if (use_local_audio_out==0) return 0;
	
	int seq = buffer[0] << 24 | buffer[1] << 16  | buffer[2]<< 8 | buffer[3] ;
	if (audioseq == seq) return 0;
	audioseq = seq;

	int sample = 4;
	for (sample; sample < 260; sample+= 4) {
	  if(playback_handle!=NULL && playback_buffer!=NULL) {
		playback_buffer[playback_offset++]=buffer[sample + 1];
		playback_buffer[playback_offset++]=buffer[sample];
		playback_buffer[playback_offset++]=buffer[sample + 3];
		playback_buffer[playback_offset++]=buffer[sample + 2];

		if(playback_offset==OUTPUT_BUFFER_SIZE) {
		  trim=0;
		  if(snd_pcm_delay(playback_handle,&delay)==0) {
			if(delay>2048) {
			  trim=delay-2048;
			}
		  }
		  if ((error = snd_pcm_writei (playback_handle, playback_buffer, audio_buffer_size-trim)) != audio_buffer_size-trim) {
			if(error==-EPIPE) {
			  if ((error = snd_pcm_prepare (playback_handle)) < 0) {
				fprintf (stderr, "audio_write: cannot prepare audio interface for use (%s)\n",
						snd_strerror (error));
				return -1;
			  }
			  if ((error = snd_pcm_writei (playback_handle, playback_buffer, audio_buffer_size-trim)) != audio_buffer_size) {
				fprintf (stderr, "audio_write: write to audio interface failed (%s)\n",
						snd_strerror (error));
				return -1;
			  }
			}
		  }
		  
		  playback_offset=0;
		}
	  }
	}
	return 0;
}

int audio_open_output(char* selected) {
	int err;
	snd_pcm_hw_params_t *hw_params;
	int rate=48000;
	int dir=0;
	char hw[64];
	
	if (use_local_audio_out==0) return 0;
	
	int i=0;
	while(selected[i]!=' ') {
		hw[i]=selected[i];
		i++;
	}
	hw[i]='\0';
	
	if ((err = snd_pcm_open (&playback_handle, hw, SND_PCM_STREAM_PLAYBACK, 0)) < 0) {
	fprintf (stderr, "audio_open_output: cannot open audio device %s (%s)\n", 
			hw,
			snd_strerror (err));
	return -1;
	}

	//fprintf(stderr,"audio_open_output: handle=%p\n",playback_handle);

	if ((err = snd_pcm_hw_params_malloc (&hw_params)) < 0) {
	fprintf (stderr, "audio_open_output: cannot allocate hardware parameter structure (%s)\n",
			snd_strerror (err));
	audio_close_output();
	return -1;
	}

	if ((err = snd_pcm_hw_params_any (playback_handle, hw_params)) < 0) {
	fprintf (stderr, "audio_open_output: cannot initialize hardware parameter structure (%s)\n",
			snd_strerror (err));
	audio_close_output();
	return -1;
	}

	if ((err = snd_pcm_hw_params_set_access (playback_handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {
	fprintf (stderr, "audio_open_output: cannot set access type (%s)\n",
			snd_strerror (err));
	audio_close_output();
	return -1;
	}

	if ((err = snd_pcm_hw_params_set_format (playback_handle, hw_params, SND_PCM_FORMAT_S16_LE)) < 0) {
	fprintf (stderr, "audio_open_output: cannot set sample format (%s)\n",
			snd_strerror (err));
	audio_close_output();
	return -1;
	}


	if ((err = snd_pcm_hw_params_set_rate_near (playback_handle, hw_params, &rate, &dir)) < 0) {
	fprintf (stderr, "audio_open_output: cannot set sample rate (%s)\n",
			snd_strerror (err));
	audio_close_output();
	return -1;
	}

	if ((err = snd_pcm_hw_params_set_channels (playback_handle, hw_params, 2)) < 0) {
	fprintf (stderr, "audio_open_output: cannot set channel count (%s)\n",
			snd_strerror (err));
	audio_close_output();
	return -1;
	}

	if ((err = snd_pcm_hw_params (playback_handle, hw_params)) < 0) {
	fprintf (stderr, "audio_open_output: cannot set parameters (%s)\n",
			snd_strerror (err));
	audio_close_output();
	return -1;
	}

	snd_pcm_hw_params_free (hw_params);

	playback_offset=0;
	playback_buffer=(unsigned char *)malloc(OUTPUT_BUFFER_SIZE);

	return 0;
}

void audio_close_output() {
	if(playback_handle!=NULL) {
		snd_pcm_close (playback_handle);
		playback_handle=NULL;
	}
	if(playback_buffer!=NULL) {
		free(playback_buffer);
		playback_buffer=NULL;
	}
}