#include <alsa/asoundlib.h>
#include "audio.h"
#include "hermeslite.h"

char *input_devices[32];
int n_input_devices=0;

char *output_devices[32];
int n_output_devices=0;


char* selectAudioOutputDevice(int deviceId) { 
	int i;
	char hw[64];
	char *selected=output_devices[deviceId];
	
	if(deviceId <0 || deviceId >= n_output_devices) return NULL;

	//i=0;
	//while(selected[i]!=' ') {
	//	hw[i]=selected[i];
	//	i++;
	//}
	//hw[i]='\0';
	
	return selected;
}

char* selectAudioInputDevice(int deviceId) { 
	int i;
	char hw[64];
	//char *selected=input_devices[deviceId];
	return input_devices[deviceId];
  
//	if(deviceId<0 || deviceId>=n_input_devices) return NULL;
 
//	i=0;
//	while(selected[i]!=' ') {
//		hw[i]=selected[i];
//		i++;
//	}
//	hw[i]='\0';
	
//	return hw;
}

void discover_audio_cards() {
	
	//audio_get_cards();
	
	fprintf(stderr, "\n\n List of Output devices\n");
	int d=0;
	for (d=0; d < n_output_devices; d++) {
		fprintf(stderr,"%d - output_device: %s\n", d, output_devices[d]);
	}
	fprintf(stderr, "\n\n List of Input devices\n");
	for (d=0; d < n_input_devices; d++) {
		fprintf(stderr,"%d - input_device: %s\n", d, input_devices[d]);
	}
}

void audio_get_cards() {
	snd_ctl_card_info_t *info;
	snd_pcm_info_t *pcminfo;
	snd_ctl_card_info_alloca(&info);
	snd_pcm_info_alloca(&pcminfo);
	int i;
	char *device_id;
	int card = -1;

	for(i=0;i<n_input_devices;i++) {
		free(input_devices[i]);
	}
	n_input_devices=0;

	for(i=0;i<n_output_devices;i++) {
		free(output_devices[i]);
	}
	n_output_devices=0;

	while (snd_card_next(&card) >= 0 && card >= 0) {
		int err = 0;
		snd_ctl_t *handle;
		char name[20];
		snprintf(name, sizeof(name), "hw:%d", card);
		if ((err = snd_ctl_open(&handle, name, 0)) < 0) {
		  continue;
		}
		if ((err = snd_ctl_card_info(handle, info)) < 0) {
		  snd_ctl_close(handle);
		  continue;
		}

		int dev = -1;

		while (snd_ctl_pcm_next_device(handle, &dev) >= 0 && dev >= 0) {
			snd_pcm_info_set_device(pcminfo, dev);
			snd_pcm_info_set_subdevice(pcminfo, 0);

			// input devices
			snd_pcm_info_set_stream(pcminfo, SND_PCM_STREAM_CAPTURE);
			if ((err = snd_ctl_pcm_info(handle, pcminfo)) == 0) {
				device_id=malloc(64);
				snprintf(device_id, 64, "plughw:%d,%d %s", card, dev, snd_ctl_card_info_get_name(info));
				input_devices[n_input_devices++]=device_id;
				//fprintf(stderr,"input_device: %s\n",device_id);
			}

			// ouput devices
			snd_pcm_info_set_stream(pcminfo, SND_PCM_STREAM_PLAYBACK);
			if ((err = snd_ctl_pcm_info(handle, pcminfo)) == 0) {
				device_id=malloc(64);
				snprintf(device_id, 64, "plughw:%d,%d %s", card, dev, snd_ctl_card_info_get_name(info));
				output_devices[n_output_devices++]=device_id;
				//fprintf(stderr,"output_device: %s\n",device_id);
			}
		}
		snd_ctl_close(handle);
	}

	// look for dmix
	void **hints, **n;
	char *name, *descr, *io;

	if (snd_device_name_hint(-1, "pcm", &hints) < 0) return;
	n = hints;
	while (*n != NULL) {
		name = snd_device_name_get_hint(*n, "NAME");
		descr = snd_device_name_get_hint(*n, "DESC");
		io = snd_device_name_get_hint(*n, "IOID");

		if(strncmp("dmix:", name, 5)==0/* || strncmp("pulse", name, 5)==0*/) {
			//fprintf(stderr,"name=%s descr=%s io=%s\n",name, descr, io);
			device_id=malloc(64);
		  
			snprintf(device_id, 64, "%s", name);
			output_devices[n_output_devices++]=device_id;
			//fprintf(stderr,"output_device: %s\n",device_id);
		}

		if (name != NULL)
		  free(name);
		if (descr != NULL)
		  free(descr);
		if (io != NULL)
		  free(io);
		n++;
	}
	snd_device_name_free_hint(hints);
	

	
}

