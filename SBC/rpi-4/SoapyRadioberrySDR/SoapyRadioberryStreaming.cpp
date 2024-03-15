#include <time.h>
#include <sys/time.h>
#include <stdint.h>
#include <cstdio>
#include <cstdlib>
#include <chrono>
#include <iostream>
#include <unistd.h>
#include <cmath>
#include "SoapyRadioberry.hpp"

 void SoapyRadioberry::setSampleRate( const int direction, const size_t channel, const double rate ) {
	SoapySDR_log(SOAPY_SDR_INFO, "SoapyRadioberry::setSampleRate called");
	
	int irate = floor(rate);
	uint32_t	ucom =0x4;
	uint32_t	command = 0;

	if (direction == SOAPY_SDR_TX)
	{
		command = 0x1;
		if (!mox)
			return;
	}
	if (direction == SOAPY_SDR_RX)
	{
		sample_rate = rate;
		if (mox)
			command = 1;
	}

	// incase of transmit still the receive samplerate need to be send
	if (sample_rate < 48001.0)
		ucom = 0x00000004;
	if (sample_rate > 48000.0 && sample_rate < 96001.0)
		ucom = 0x01000004;
	if (sample_rate > 96000.0 && sample_rate < 192001.0)
		ucom = 0x02000004;
	if (sample_rate > 192000.0)
		ucom = 0x03000004;
	this->SoapyRadioberry::controlRadioberry(command, ucom);
}

SoapySDR::RangeList SoapyRadioberry::getSampleRateRange(const int direction, const size_t channel) const 
{
	
	SoapySDR_log(SOAPY_SDR_INFO, "SoapyRadioberry::getSampleRateRange called");
	
	SoapySDR::RangeList rangeList;
	
	if (direction == SOAPY_SDR_RX) 	rangeList.push_back(SoapySDR::Range(48000.0, 384000.0, 48000.0));
	if (direction == SOAPY_SDR_TX) 	rangeList.push_back(SoapySDR::Range(48000.0, 48000.0, 48000.0));
	
	return rangeList;
}

std::vector<std::string> SoapyRadioberry::getStreamFormats(const int direction, const size_t channel) const
{
	SoapySDR_log(SOAPY_SDR_INFO, "SoapyRadioberry::getStreamFormats called");
	
	std::vector<std::string> formats;

	formats.push_back(SOAPY_SDR_CF32);
	formats.push_back(SOAPY_SDR_CS16);

	return formats;
}

std::string SoapyRadioberry::getNativeStreamFormat(const int direction, const size_t channel, double &fullScale) const
{
	SoapySDR_log(SOAPY_SDR_INFO, "SoapyRadioberry::getNativeStreamFormat called");
	
	if (direction == SOAPY_SDR_RX) {
		fullScale = 2048; // RX expects 12 bit samples LSB aligned
	}
	else if (direction == SOAPY_SDR_TX) {
		fullScale = 32768; // TX expects 12 bit samples MSB aligned
	}
	return SOAPY_SDR_CF32;
}

SoapySDR::ArgInfoList SoapyRadioberry::getStreamArgsInfo(const int direction, const size_t channel) const
{
	SoapySDR::ArgInfoList streamArgs;
	
	SoapySDR::ArgInfo bufflenArg;
    bufflenArg.key = "bufflen";
    bufflenArg.value = "64"; 
    bufflenArg.name = "Buffer Size";
    bufflenArg.description = "Number of bytes per buffer, multiples of 512 only.";
    bufflenArg.units = "bytes";
    bufflenArg.type = SoapySDR::ArgInfo::INT;

    streamArgs.push_back(bufflenArg);

	return streamArgs;
}

auto startTime = std::chrono::high_resolution_clock::now();


SoapySDR::Stream *SoapyRadioberry::setupStream(
		const int direction,
		const std::string &format,
		const std::vector<size_t> &channels,
		const SoapySDR::Kwargs &args )
{
	SoapySDR_log(SOAPY_SDR_INFO, "SoapyRadioberry::setupStream called");
	startTime = std::chrono::high_resolution_clock::now();
	//check the format
	sdr_stream *ptr;
	ptr = new sdr_stream(direction);

	if (format == SOAPY_SDR_CF32) {
		SoapySDR_log(SOAPY_SDR_INFO, "Using format CF32.");
		ptr->set_stream_format(RADIOBERRY_SDR_CF32);
		mox = true;
	}
	else if(format == SOAPY_SDR_CS16 && direction == SOAPY_SDR_TX)
	{
		SoapySDR_log(SOAPY_SDR_INFO, "Using format CS16.");
		ptr->set_stream_format(RADIOBERRY_SDR_CS16);
		mox = true;
	}
	else
	{
		throw std::runtime_error(
			"setupStream invalid format '" + format + "' -- Only CF32 is supported by SoapyRadioberrySDR module.");
	}

	streams.push_back(ptr);
	return (SoapySDR::Stream *)ptr;
}

void SoapyRadioberry::closeStream(SoapySDR::Stream *stream)
{
	int i = 0;
	for (auto con : streams)
	{
		if ((sdr_stream *)stream == con)
		{
			if (((sdr_stream *)stream)->get_direction() == SOAPY_SDR_TX)
			{	// switch off TX stream
				mox = false;
				setSampleRate(SOAPY_SDR_RX, 0, sample_rate);	
			}
			delete ((sdr_stream *)stream);
			streams.erase(streams.begin() + i);
		}
		i++;
	}
}

int SoapyRadioberry::readStream(
		SoapySDR::Stream *handle,
		void * const *buffs,
		const size_t numElems,
		int &flags,
		long long &timeNs,
		const long timeoutUs )
{
	int nr_samples, no_bytes, iq=0;
	int npackages = numElems / 63;

	void *buff_base = buffs[0];
	float	*target_buffer = (float *) buff_base;
	int16_t *itarget_buffer = (int16_t *) buff_base;
	int32_t left_sample;
	int32_t right_sample;
	sdr_stream *ptr = (sdr_stream *)handle;

	char rx_buffer[512];
	for(int ii = 0 ; ii < npackages ; ii++)
	{
		no_bytes = read(fd_rb, rx_buffer, sizeof(rx_buffer));
		nr_samples = no_bytes / 6;
		//printf("nr_samples %d sample: %d %d %d %d %d %d\n",nr_samples, (int)rx_buffer[0],(int)rx_buffer[1],(int)rx_buffer[2],(int)rx_buffer[3],(int)rx_buffer[4],(int)rx_buffer[5]);
		if (ptr->get_stream_format() == RADIOBERRY_SDR_CF32)
		{
			for (int i = 0; i < no_bytes; i += 6) {
				left_sample = (int32_t)((signed char) rx_buffer[i]) << 16;
				left_sample |= (int32_t)((((unsigned char)rx_buffer[i + 1]) << 8) & 0xFF00);
				left_sample |= (int32_t)((unsigned char)rx_buffer[i + 2] & 0xFF);
				right_sample = (int32_t)((signed char)rx_buffer[i + 3]) << 16;
				right_sample |= (int32_t)((((unsigned char)rx_buffer[i + 4]) << 8) & 0xFF00);
				right_sample |= (int32_t)((unsigned char)rx_buffer[i + 5] & 0xFF);
				right_sample =  right_sample * -1;

				target_buffer[iq++] = (float)left_sample / 8388608.0;      // 24 bit sample
				target_buffer[iq++] = (float)right_sample / 8388608.0;      // 24 bit sample
			}
		}
		if (ptr->get_stream_format() == RADIOBERRY_SDR_CS16)
		{
			for (int i = 0; i < no_bytes; i += 6) {
				left_sample   = (int32_t)((signed char) rx_buffer[i]) << 16;
				left_sample  |= (int32_t)((((unsigned char)rx_buffer[i + 1]) << 8) & 0xFF00);
				left_sample  |= (int32_t)((unsigned char)rx_buffer[i + 2] & 0xFF);
				right_sample  = (int32_t)((signed char)rx_buffer[i + 3]) << 16;
				right_sample |= (int32_t)((((unsigned char)rx_buffer[i + 4]) << 8) & 0xFF00);
				right_sample |= (int32_t)((unsigned char)rx_buffer[i + 5] & 0xFF);
				right_sample =  right_sample * -1;
			
				itarget_buffer[iq++] = (int16_t)(left_sample >> 8);     // 16 bit sample
				itarget_buffer[iq++] = (int16_t)(right_sample >> 8);    // 16 bit sample
			}
		}
		//printf("nr_samples %d sample: %f %f \n", nr_samples, (float)left_sample / 8388608.0, (float)right_sample / 8388608.0);
	}
	return (npackages * nr_samples); //return the number of IQ samples
}


int SoapyRadioberry::writeStream(SoapySDR::Stream *stream, const void * const *buffs, const size_t numElems, int &flags, const long long timeNs, const long timeoutUs)
{
	int iq = 0; 
	size_t ret;
	int nr_samples;
	 
	void const		*buff_base = buffs[0];
	float			*target_buffer = (float *) buff_base;
	int16_t			*itarget_buffer = (int16_t *) buff_base;
	sdr_stream *ptr = (sdr_stream *)stream;
	unsigned char i8TxBuffer[6];

	if (ptr->get_stream_format() == RADIOBERRY_SDR_CF32)
	{
		for (int ii = 0; ii < numElems; ii++)
		{
			const float gain = 8388608.0f;
			int isample = target_buffer[iq] >= 0.0 ? (long)floor(target_buffer[iq] * gain + 0.5) : (long)ceil(target_buffer[iq] * gain - 0.5);
			int qsample = target_buffer[iq + 1] >= 0.0 ? (long)floor(target_buffer[iq + 1] * gain + 0.5) : (long)ceil(target_buffer[iq + 1] * gain - 0.5);

			//printf("nr_samples %d sample: %d %d \n", iq, isample,qsample );
			
			i8TxBuffer[0] = isample >> 16;
			i8TxBuffer[1] = isample >> 8;
			i8TxBuffer[2] = qsample >> 16;
			i8TxBuffer[3] = qsample >> 8;
			ret = write(fd_rb, i8TxBuffer, 4);
			iq++;
			iq++;
		}
	}
	if (ptr->get_stream_format() == RADIOBERRY_SDR_CS16)
	{
		int j = 0;
	
		for (int ii = 0; ii < numElems; ii++)
		{
			i8TxBuffer[0] = (unsigned char)((itarget_buffer[j] & 0xff00) >> 8);
			i8TxBuffer[1] = (unsigned char)(itarget_buffer[j] & 0xff);
			i8TxBuffer[2] = (unsigned char)(((itarget_buffer[j + 1]) & 0xff00) >> 8);
			i8TxBuffer[3] = (unsigned char)((itarget_buffer[j + 1]) & 0xff);
			ret = write(fd_rb, i8TxBuffer, 4);
			j += 2;
		}
	}
	if (ret < 0)
		SoapySDR_log(SOAPY_SDR_ERROR, "Write error to radioberry driver");
	return numElems;
}