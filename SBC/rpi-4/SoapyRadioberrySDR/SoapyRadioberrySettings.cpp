#include "SoapyRadioberry.hpp"
#include <chrono>
#include <thread>
#include <string>

#define RADIOBERRY_BUFFER_SIZE	4096	

/***********************************************************************
 * Device interface
 **********************************************************************/
 
SoapyRadioberry::SoapyRadioberry( const SoapySDR::Kwargs &args ){

	SoapySDR_setLogLevel(SOAPY_SDR_INFO);
	SoapySDR_log(SOAPY_SDR_INFO, "SoapyRadioberry::SoapyRadioberry  constructor called");
	mox = false;
	no_channels = 1;
	fd_rb = open("/dev/radioberry", O_RDWR);
	try
	{
		i2c_ptr = std::make_unique<rpihw::driver::i2c>(rpihw::driver::i2c("/dev/i2c-1"));
		i2c_available = true;
	}
	catch (std::runtime_error e)
	{
		printf("I2c not found %s", e.what());
		i2c_available = false;
	}
}

SoapyRadioberry::~SoapyRadioberry(void)
{
	SoapySDR_log(SOAPY_SDR_INFO, "SoapyRadioberry::SoapyRadioberry  destructor called");
	for (auto con : streams)
		delete (con);
	if (fd_rb != 0) close(fd_rb);
}

void SoapyRadioberry::controlRadioberry(uint32_t command, uint32_t command_data) {

	std::unique_lock<std::mutex> soapy_lock(send_command);
	SoapySDR_log(SOAPY_SDR_INFO, "SoapyRadioberry::controlRadioberry called");
	
	uint32_t CWX =0;
	uint32_t running = 1;
	
	rb_control.rb_command = 0x04 | (((CWX << 1) & 0x02) | (running & 0x01));
	rb_control.command = command;
	rb_control.command_data = command_data;

	fprintf(stderr, "RB-Command = %02X Command = %02X  command_data = %08X Mox %d\n", rb_control.rb_command, command >> 1, command_data, command & 0x01);
	
	if (ioctl(fd_rb, RADIOBERRY_IOC_COMMAND, &rb_control) == -1) {
		SoapySDR_log(SOAPY_SDR_INFO, "Could not sent command to radioberry device.");
	} else SoapySDR_log(SOAPY_SDR_INFO, "Command sent succesfull to radioberry device.");
}

std::string SoapyRadioberry::getDriverKey( void ) const
{
	SoapySDR_log(SOAPY_SDR_INFO, "SoapyRadioberry::getDriverKey called");
	
	return "radioberry";
}

std::string SoapyRadioberry::getHardwareKey( void ) const
{
	SoapySDR_log(SOAPY_SDR_INFO, "SoapyRadioberry::getHardwareKey called");
	
	return "v2.0-beta5";
}

SoapySDR::Kwargs SoapyRadioberry::getHardwareInfo( void ) const
{
	
	SoapySDR_log(SOAPY_SDR_INFO, "SoapyRadioberry::getHardwareInfo called");
	
	SoapySDR::Kwargs info;
	int count = 0;
	struct rb_info_arg_t rb_info;
	
	do
	{
		std::memset(&rb_info, 0, sizeof(rb_info));
		if (ioctl(fd_rb, RADIOBERRY_IOC_COMMAND, &rb_info) == -1)
		{
			rb_info.major = 0;
			rb_info.minor = 0;
		}
		std::this_thread::sleep_for(std::chrono::seconds(1));
		count++;
	} while ((rb_info.major == 0 || rb_info.major > 128) && count < 20);

	unsigned int major, minor;
	major = rb_info.major;
	minor = rb_info.minor;
	
	char firmware_version[100];
	snprintf(firmware_version, 100, "%u.%u", 0, 1); //0.1
	info["firmwareVersion"] = firmware_version;

	char gateware_version[100];
	snprintf(gateware_version, 100, "%u.%u ", major, minor);
	info["gatewareVersion"] = gateware_version;
	
	char hardware_version[100];
	snprintf(hardware_version, 100, "%u.%u", 2, 4); //2.4 beta
	info["hardwareVersion"] = hardware_version;

	char protocol_version[100];
	snprintf(protocol_version, 100, "%u.%u ", 1, 58); //1.58 protocol 1
	info["protocolVersion"] = protocol_version;

	return info;
}

size_t SoapyRadioberry::getNumChannels( const int direction ) const
{
	SoapySDR_log(SOAPY_SDR_INFO, "SoapyRadioberry::getNumChannels called");
	
	//if (direction == SOAPY_SDR_RX) return(4);
	
	return(1); //1 RX and 1 TX channel; making this for standalone radioberry!
}

bool SoapyRadioberry::getFullDuplex( const int direction, const size_t channel ) const
{
	SoapySDR_log(SOAPY_SDR_INFO, "SoapyRadioberry::getFullDuplex called");
	
	return(true);
}

std::vector<double> SoapyRadioberry::listBandwidths( const int direction, const size_t channel ) const
{
	// radioberry does nor support bandwidth
	SoapySDR_log(SOAPY_SDR_INFO, "SoapyRadioberry::listBandwidths called");
		
	std::vector<double> options;
	return(options);
}

std::vector<double> SoapyRadioberry::listSampleRates( const int direction, const size_t channel ) const
{
	
	SoapySDR_log(SOAPY_SDR_INFO, "SoapyRadioberry::listSampleRates called");
	
	
	std::vector<double> options;
	
	if (direction == SOAPY_SDR_RX) {
		options.push_back(0.048e6);  
		options.push_back(0.096e6);
		options.push_back(0.192e6);
		options.push_back(0.384e6);
	}
	if (direction == SOAPY_SDR_TX) {
		options.push_back(0.048e6);  
	}
	return(options);
}

double SoapyRadioberry::getBandwidth( const int direction, const size_t channel ) const
{
	
	SoapySDR_log(SOAPY_SDR_INFO, "SoapyRadioberry::getBandwidth called");
	
    long long bandwidth = 48000.0;

	if(direction==SOAPY_SDR_RX){
      
	  //depends on settings.. TODO

	}

	else if(direction==SOAPY_SDR_TX){
       bandwidth = 48000.0;
	}

	return double(bandwidth);
}

SoapySDR::RangeList SoapyRadioberry::getFrequencyRange( const int direction, const size_t channel)  const
{
	SoapySDR_log(SOAPY_SDR_INFO, "SoapyRadioberry::getFrequencyRange called");
	
	SoapySDR::RangeList rangeList;
	
	rangeList.push_back(SoapySDR::Range(10000.0, 30000000.0, 1.0));
	
	return rangeList;
}

std::vector<std::string> SoapyRadioberry::listAntennas( const int direction, const size_t channel ) const
{
	
	SoapySDR_log(SOAPY_SDR_INFO, "SoapyRadioberry::listAntennas called");
	
	std::vector<std::string> options;
	if(direction == SOAPY_SDR_RX) options.push_back( "ANTENNA RX" );
	if(direction == SOAPY_SDR_TX) options.push_back( "ANTENNA TX" );
	return(options);
}


/*******************************************************************
 * Gain API
 ******************************************************************/

std::vector<std::string> SoapyRadioberry::listGains( const int direction, const size_t channel ) const
{
	SoapySDR_log(SOAPY_SDR_INFO, "SoapyRadioberry::listGains called");
	
	std::vector<std::string> options;
	//options.push_back("PGA"); in pihpsdr no additional gain settings.
	return(options);
}

SoapySDR::Range SoapyRadioberry::getGainRange( const int direction, const size_t channel) const
{
	SoapySDR_log(SOAPY_SDR_INFO, "SoapyRadioberry::getGainRange called");
	
	if(direction==SOAPY_SDR_RX)
		return(SoapySDR::Range(-12, 48));
	return(SoapySDR::Range(0,15));
}

void SoapyRadioberry::setGain( const int direction, const size_t channel, const double value ) {
	
	SoapySDR_log(SOAPY_SDR_INFO, "SoapyRadioberry::setGain called");
	
	uint32_t command = 0;
	uint32_t command_data = (0x40 | (((uint32_t)value)  & 0x3F));
	
	if (direction == SOAPY_SDR_RX)	 
	{
		if (mox)
			command = 0x15;
		else
			command = 0x14;
		command_data = (0x40 | (((uint32_t)value + 12) & 0x3F));
	}
	
	if(direction==SOAPY_SDR_TX) 
	{ // 0 -7 TX RF gain
		if (!mox)
			return;

		uint32_t	z = (uint32_t)value;
		if (value > 15) z = 15;
		if (value < 0.0) z = 0;
		z = z << 28;
		command = 0x13; 
		command_data = z;
	}
	
	this->SoapyRadioberry::controlRadioberry(command, command_data);
}

/*******************************************************************
 * Frequency API 
 ******************************************************************/
void SoapyRadioberry::setFrequency( const int direction, const size_t channel,  const double frequency, const SoapySDR::Kwargs &args ) {
	
	SoapySDR_log(SOAPY_SDR_INFO, "SoapyRadioberry::setFrequency called");
	
	uint32_t command = 0;

	if (direction == SOAPY_SDR_RX)
	{
		if (mox)
			command = 5;
		else
			command = 4;
	}

	if (direction == SOAPY_SDR_TX)
	{
		if (!mox)
			return;
		command = 3;
	}

	uint32_t command_data = (uint32_t) frequency;
	
	this->SoapyRadioberry::controlRadioberry(command, command_data);
}

void SoapyRadioberry::writeI2C(const int addr, const std::string &data)
{
	SoapySDR_log(SOAPY_SDR_INFO, "SoapyRadioberry::writeI2C called");

	if (!i2c_available)
		return;
	i2c_ptr->addr(addr);
	try
	{
		i2c_ptr->write((uint8_t *)data.c_str(), data.size());
	}
	catch (std::runtime_error e)
	{
		printf("%s", e.what());
	}
}

std::string SoapyRadioberry::readI2C(const int addr, const size_t numBytes)
{
	SoapySDR_log(SOAPY_SDR_INFO, "SoapyRadioberry::readI2C called");

	std::string data;

	if (!i2c_available)
		return std::string("");
	i2c_ptr->addr(addr);
	data.reserve(numBytes);
	try
	{
		i2c_ptr->read((uint8_t *)data.c_str(), numBytes);
		data.resize(numBytes);
	}
	catch (std::runtime_error e)
	{
		printf("%s", e.what());
	}
	return data;
}
// end of source.

