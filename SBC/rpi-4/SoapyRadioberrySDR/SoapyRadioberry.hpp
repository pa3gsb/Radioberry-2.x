#include <SoapySDR/Device.hpp>
#include <SoapySDR/Logger.hpp>
#include <SoapySDR/Formats.hpp>

#include <stdio.h>	
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <memory>
#include <string.h> 
#include "radioberry_ioctl.h"
#include "i2c.h"

#define TX_MAX 4800
#define TX_MAX_BUFFER (TX_MAX * 8)
const int npackages = 4;

typedef enum radioberrysdrStreamFormat {
	RADIOBERRY_SDR_CF32,
	RADIOBERRY_SDR_CS16
} radioberrysdrStreamFormat;

class SoapyRadioberry : public SoapySDR::Device{

	public:
		
		SoapyRadioberry( const SoapySDR::Kwargs & args );
		~SoapyRadioberry();

		/*******************************************************************
		 * Identification API
		 ******************************************************************/

		std::string getDriverKey( void ) const;

		std::string getHardwareKey( void ) const;
		
		SoapySDR::Kwargs getHardwareInfo( void ) const;
		
		/*******************************************************************
		 * Channels API
		 ******************************************************************/

		size_t getNumChannels( const int direction ) const;

		bool getFullDuplex( const int direction, const size_t channel ) const;
		
		/*******************************************************************
		 * Stream API
		 ******************************************************************/
		SoapySDR::RangeList getSampleRateRange(const int direction, const size_t channel) const;

		std::vector<std::string> getStreamFormats(const int direction, const size_t channel) const;

		std::string getNativeStreamFormat(const int direction, const size_t channel, double &fullScale) const;

		SoapySDR::ArgInfoList getStreamArgsInfo(const int direction, const size_t channel) const;


		SoapySDR::Stream *setupStream(
				const int direction,
				const std::string &format,
				const std::vector<size_t> &channels = std::vector<size_t>(),
				const SoapySDR::Kwargs &args = SoapySDR::Kwargs() );
				
		int readStream(
				SoapySDR::Stream *stream,
				void * const *buffs,
				const size_t numElems,
				int &flags,
				long long &timeNs,
				const long timeoutUs = 100000 );				
				
		int writeStream(
				SoapySDR::Stream *stream,
				const void * const *buffs,
				const size_t numElems,
				int &flags,
				const long long timeNs = 0,
				const long timeoutUs = 100000);


		/*******************************************************************
		 * Sample Rate API
		 ******************************************************************/

		void setSampleRate( const int direction, const size_t channel, const double rate );

		double getBandwidth( const int direction, const size_t channel ) const;


		std::vector<double> listBandwidths( const int direction, const size_t channel ) const;
		std::vector<double> listSampleRates( const int direction, const size_t channel ) const;
		
		
		/*******************************************************************
		 * Frequency API
		 ******************************************************************/
		
		void setFrequency(
				const int direction,
				const size_t channel,
				const double frequency,
				const SoapySDR::Kwargs &args = SoapySDR::Kwargs());


		SoapySDR::RangeList getFrequencyRange( const int direction, const size_t channel) const;
 

		/*******************************************************************
		 * Antenna API
		 ******************************************************************/

		std::vector<std::string> listAntennas( const int direction, const size_t channel ) const;

		
		/*******************************************************************
		 * Gain API
		 ******************************************************************/

		std::vector<std::string> listGains( const int direction, const size_t channel ) const;

		void setGain( const int direction, const size_t channel, const double value );

		SoapySDR::Range getGainRange( const int direction, const size_t channel ) const;


		void controlRadioberry(uint32_t command, uint32_t command_data);
		
	/*******************************************************************
	 * I2C API
	 ******************************************************************/
	
		std::string readI2C(const int addr, const size_t numBytes);
		void writeI2C(const int addr, const std::string &data);
	
	private:
	
	int		fd_rb;	
	int		sample_rate;
	int		rx_frequency;
	int		no_channels;	
	struct rb_info_arg_t rb_control;
	std::unique_ptr<rpihw::driver::i2c> i2c_ptr;
	bool							i2c_available = false;
	radioberrysdrStreamFormat		streamFormat;
	uint32_t		m_count;
	uint32_t		m_highwater, m_lowwater;
	__useconds_t	m_sleep;
};
