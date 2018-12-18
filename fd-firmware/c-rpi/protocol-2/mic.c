
#include <alsa/asoundlib.h>
#include "mic.h"
#include "radioberry.h"

static int mic_buffer_size = 64;
static snd_pcm_t *record_handle=NULL;
static unsigned char *mic_buffer=NULL;

#define AUDIO_SAMPLE_SIZE 2
#define AUDIO_CHANNELS 2
#define MIC_BUFFER_SIZE (AUDIO_SAMPLE_SIZE*AUDIO_CHANNELS*mic_buffer_size)

void *mic_dummy_thread(void *arg) {
	
	int i=0;
	unsigned char l_mic_buffer[132];
	
	while(1) {
		
		sem_wait(&mic_sample);
		i++;
		if ((i % sampleSpeed[0])==0) {
			
			i = 0;
		
			l_mic_buffer[mic_count++] = 0; //msb
			l_mic_buffer[mic_count++] = 0;
				
			if (mic_count == 132) {
				l_mic_buffer[0]=mic_sequence>>24;
				l_mic_buffer[1]=mic_sequence>>16;
				l_mic_buffer[2]=mic_sequence>>8;
				l_mic_buffer[3]=mic_sequence;
				send_udp_packet(radioberry_socket, mic_line_addr, dst_addr, l_mic_buffer, 132);
				mic_sequence++;
				mic_count = 4;
			}
		}
	}
	fprintf(stderr,"mic_dummy_thread: exiting\n");
	return NULL;
}

void start_dummy_mic_thread() {
	pthread_t pid1; 
	pthread_create(&pid1, NULL, mic_dummy_thread, NULL);
}

unsigned char l_mic_buffer[132]; 
void process_local_mic(unsigned char *mic_buffer){
	int i=0;
	
	if (running) {
		for(i=0;i<MIC_SAMPLES;i++) {
			l_mic_buffer[mic_count++] = mic_buffer[i*2+1]; //msb
			l_mic_buffer[mic_count++] = mic_buffer[i*2];
			
			/* making verbose function.....
			lcounta ++;
			if (lcounta == 48000) {
				lcounta = 0;
				gettimeofday(&t2a, 0);
				float elapsd = timedifference_msec(t1a, t2a);
				printf("Audio line-in executed in %f milliseconds.\n", elapsd);
				gettimeofday(&t1a, 0);
			}
			*/
			
		}
		l_mic_buffer[0]=mic_sequence>>24;
		l_mic_buffer[1]=mic_sequence>>16;
		l_mic_buffer[2]=mic_sequence>>8;
		l_mic_buffer[3]=mic_sequence;
		send_udp_packet(radioberry_socket, mic_line_addr, dst_addr, l_mic_buffer, 132);
		mic_sequence++;
		mic_count = 4;
	}
}

static void *mic_read_thread(void *arg) {
	int rc;
	int mic_running = 1;
	
	if ((rc = snd_pcm_prepare (record_handle)) < 0) {
		fprintf (stderr, "mic_read_thread: cannot prepare audio interface for use (%s)\n",
				snd_strerror (rc));
		return NULL;
	}
	
	while(mic_running) {
		if ((rc = snd_pcm_readi (record_handle, mic_buffer, mic_buffer_size)) != mic_buffer_size) {
		  if(mic_running) {
			if(rc<0) {
			  fprintf (stderr, "mic_read_thread: read from audio interface failed (%s)\n",
					  snd_strerror (rc));
			  mic_running=0;
			} else {
			  fprintf(stderr,"mic_read_thread: read %d\n",rc);
			}
		  }
		} else process_local_mic(mic_buffer);
	}
	fprintf(stderr,"mic_read_thread: exiting\n");
	return NULL;
}

void start_mic_thread() {
	pthread_t pid1; 
	
	if (use_local_audio_in==0) start_dummy_mic_thread(); else pthread_create(&pid1, NULL, mic_read_thread, NULL);
}

int audio_open_input(char* selected) {
	int err;
	snd_pcm_hw_params_t *hw_params;
	int rate=48000;
	int dir=0;
	char hw[64];
	
	int i=0;
	while(selected[i]!=' ') {
		hw[i]=selected[i];
		i++;
	}
	hw[i]='\0';

	if ((err = snd_pcm_open (&record_handle, hw, SND_PCM_STREAM_CAPTURE, 0)) < 0) {
	fprintf (stderr, "audio_open_input: cannot open audio device %s (%s)\n",
			hw,
			snd_strerror (err));
	return -1;
	}

	if ((err = snd_pcm_hw_params_malloc (&hw_params)) < 0) {
	fprintf (stderr, "audio_open_input: cannot allocate hardware parameter structure (%s)\n",
			snd_strerror (err));
	audio_close_input();
	return -1;
	}

	if ((err = snd_pcm_hw_params_any (record_handle, hw_params)) < 0) {
	fprintf (stderr, "audio_open_input: cannot initialize hardware parameter structure (%s)\n",
			snd_strerror (err));
	audio_close_input();
	return -1;
	}

	if ((err = snd_pcm_hw_params_set_access (record_handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {
	fprintf (stderr, "audio_open_input: cannot set access type (%s)\n",
			snd_strerror (err));
	audio_close_input();
	return -1;
	}

	if ((err = snd_pcm_hw_params_set_format (record_handle, hw_params, SND_PCM_FORMAT_S16_LE)) < 0) {
	fprintf (stderr, "audio_open_input: cannot set sample format (%s)\n",
			snd_strerror (err));
	audio_close_input();
	return -1;
	}

	if ((err = snd_pcm_hw_params_set_rate_near (record_handle, hw_params, &rate, &dir)) < 0) {
	fprintf (stderr, "audio_open_input: cannot set sample rate (%s)\n",
			snd_strerror (err));
	audio_close_input();
	return -1;
	}

	if ((err = snd_pcm_hw_params_set_channels (record_handle, hw_params, 1)) < 0) {
	fprintf (stderr, "audio_open_input: cannot set channel count (%s)\n",
			snd_strerror (err));
	audio_close_input();
	return -1;
	}

	if ((err = snd_pcm_hw_params (record_handle, hw_params)) < 0) {
	fprintf (stderr, "audio_open_input: cannot set parameters (%s)\n",
			snd_strerror (err));
	audio_close_input();
	return -1;
	}

	snd_pcm_hw_params_free (hw_params);

	mic_buffer=(unsigned char *)malloc(MIC_BUFFER_SIZE);

	return 0;
}

void audio_close_input() {
	if(record_handle!=NULL) {
		snd_pcm_close (record_handle);
		record_handle=NULL;
	}
	if(mic_buffer!=NULL) {
		free(mic_buffer);
		mic_buffer=NULL;
	}
}