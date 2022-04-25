#include <time.h>
#include <sys/time.h>
#include <stdint.h>
#include <cstdio>
#include <cstdlib>
#include <chrono>
#include <iostream>
#include <unistd.h>
#include "SoapyRadioberry.hpp"

 void SoapyRadioberry::setSampleRate( const int direction, const size_t channel, const double rate ) {
	//SoapySDR_log(SOAPY_SDR_INFO, "SoapyRadioberry::setSampleRate called");
	
	int irate = floor(rate);
	uint32_t	ucom =0x00000004;
	uint32_t	command = 0;
	 
	 if (direction == SOAPY_SDR_TX)
		 command = 0x1;
	 if (direction == SOAPY_SDR_RX)
		 sample_rate = rate;

	 // incase of transmit still the receive samplerate need to be send
	 if (rate < 48001.0)
		 ucom = 0x00000004;
	 if (rate > 48000.0 && rate < 96001.0)
		 ucom = 0x01000004;
	 if (rate > 96000.0 && rate < 192001.0)
		 ucom = 0x02000004;
	 if (rate > 192000.0)
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
	if (format == SOAPY_SDR_CF32) {
		SoapySDR_log(SOAPY_SDR_INFO, "Using format CF32.");
		streamFormat = RADIOBERRY_SDR_CF32;
	}
	else if(format == SOAPY_SDR_CS16 && direction == SOAPY_SDR_TX)
	{
		SoapySDR_log(SOAPY_SDR_INFO, "Using format CS16.");
		streamFormat = RADIOBERRY_SDR_CS16;
	}
	else
	{
		throw std::runtime_error(
			"setupStream invalid format '" + format + "' -- Only CF32 is supported by SoapyRadioberrySDR module.");
	}

	return (SoapySDR::Stream *) this;

}

int SoapyRadioberry::readStream(
		SoapySDR::Stream *handle,
		void * const *buffs,
		const size_t numElems,
		int &flags,
		long long &timeNs,
		const long timeoutUs )
{
	int i;
	int iq = 0;
	int nr_samples;
	 
	void *buff_base = buffs[0];
	float	*target_buffer = (float *) buff_base;
	int16_t *itarget_buffer = (int16_t *) buff_base;
	
	char rx_buffer[512];
	//setSampleRate(SOAPY_SDR_RX, 0, sample_rate);
	for(int ii = 0 ; ii < npackages ; ii++)
	{
		nr_samples = read(fd_rb, rx_buffer, sizeof(rx_buffer));
		//printf("nr_samples %d sample: %d %d %d %d %d %d\n",nr_samples, (int)rx_buffer[0],(int)rx_buffer[1],(int)rx_buffer[2],(int)rx_buffer[3],(int)rx_buffer[4],(int)rx_buffer[5]);
		if(streamFormat == RADIOBERRY_SDR_CF32)
		{
			int32_t left_sample;
			int32_t right_sample;
			
			for (i = 0; i < nr_samples; i += 6) {
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
			//printf("nr_samples %d sample: %d %d \n", nr_samples, left_sample, right_sample);
		}
		if (streamFormat == RADIOBERRY_SDR_CS16)
		{
			int32_t left_sample;
			int32_t right_sample;
			
			for (i = 0; i < nr_samples; i += 6) {
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
	}
	return (npackages * nr_samples / 6); //return the number of IQ samples
}

union uTxBuffer
{
	std::uint16_t	i16TxBuffer[2];
	unsigned char	i8TxBuffer[4];
};

int SoapyRadioberry::writeStream(SoapySDR::Stream *stream, const void * const *buffs, const size_t numElems, int &flags, const long long timeNs, const long timeoutUs)
{
	int iq = 0; 
	size_t ret;
	int left_sample;
	int right_sample;
	int nr_samples;
	 
	void const		*buff_base = buffs[0];
	float			*target_buffer = (float *) buff_base;
	int16_t			*itarget_buffer = (int16_t *) buff_base;
	
	
	uTxBuffer	tx;

	//setSampleRate(SOAPY_SDR_TX, 0, sample_rate);
	if (streamFormat == RADIOBERRY_SDR_CF32)
	{
		for (int ii = 0; ii < numElems; ii++)
		{
			float		i, q;
			int16_t		di, dq;
		
			tx.i16TxBuffer[0] = (int16_t)(target_buffer[iq++] * 16384.0f);
			tx.i16TxBuffer[1] = (int16_t)(target_buffer[iq++] * -16384.0f);
			ret = write(fd_rb, &tx, 4 * sizeof(uint8_t));
		}
	}
	if (streamFormat == RADIOBERRY_SDR_CS16)
	{
		int j = 0;
	
		//printf("SoapySDR send %d elements \n", numElems);
		for (int ii = 0; ii < numElems; ii++)
		{
			//printf("%x %x %x %x\n", (itarget_buffer[j] & 0xFF00) >> 8, (itarget_buffer[j] & 0x00FF), (itarget_buffer[j+1] & 0xFF00) >> 8, (itarget_buffer[j+1] & 0x00FF));
			
			tx.i8TxBuffer[0] = (unsigned char)((itarget_buffer[j] & 0xff00) >> 8);
			tx.i8TxBuffer[1] = (unsigned char)(itarget_buffer[j] & 0xff);
			tx.i8TxBuffer[2] = (unsigned char)(((-1 * itarget_buffer[j + 1]) & 0xff00) >> 8);
			tx.i8TxBuffer[3] = (unsigned char)((-1 * itarget_buffer[j + 1]) & 0xff); 
				
			ret = write(fd_rb, &tx, sizeof(uint32_t));
			j += 2;
		}
		
	}
	return numElems;
}