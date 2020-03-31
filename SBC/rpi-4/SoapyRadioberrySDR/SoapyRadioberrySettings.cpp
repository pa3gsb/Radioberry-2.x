#include "SoapyRadioberry.hpp"


/***********************************************************************
 * Device interface
 **********************************************************************/
 
SoapyRadioberry::SoapyRadioberry( const SoapySDR::Kwargs &args ){
	
	SoapySDR_log(SOAPY_SDR_INFO, "SoapyRadioberry::SoapyRadioberry  constructor called");
	
	fd_rb = open("/dev/radioberry", O_RDWR);
}

SoapyRadioberry::~SoapyRadioberry(void){
	
	SoapySDR_log(SOAPY_SDR_INFO, "SoapyRadioberry::SoapyRadioberry  destructor called");
	
	if (fd_rb != 0) close(fd_rb);
}

void SoapyRadioberry::controlRadioberry(uint32_t command, uint32_t command_data) {
	
	SoapySDR_log(SOAPY_SDR_INFO, "SoapyRadioberry::controlRadioberry called");
	
	uint32_t CWX =0;
	uint32_t running = 1;
	
	rb_control.rb_command = (((CWX << 1) & 0x02) | (running & 0x01));
	rb_control.command = command;
	rb_control.command_data = command_data;
	
	fprintf(stderr, "Command = %02X  command_data = %08X\n", command, command_data);
	
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
	
	return "v2.0-beta4";
}

SoapySDR::Kwargs SoapyRadioberry::getHardwareInfo( void ) const
{
	
	SoapySDR_log(SOAPY_SDR_INFO, "SoapyRadioberry::getHardwareInfo called");
	
	SoapySDR::Kwargs info;
	
	struct rb_info_arg_t rb_info;
	
	if (ioctl(fd_rb, RADIOBERRY_IOC_COMMAND, &rb_info) == -1) {
		rb_info.major = 0;
		rb_info.minor = 0;
	}		

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
	
	SoapySDR_log(SOAPY_SDR_INFO, "SoapyRadioberry::listBandwidths called");
	
	
	std::vector<double> options;
	
	if (direction == SOAPY_SDR_RX) {
		options.push_back(0.048e6);  
		options.push_back(0.096e6);
		options.push_back(0.192e6);
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

SoapySDR::Range SoapyRadioberry::getGainRange( const int direction, const size_t channel, const std::string &name ) const
{
	SoapySDR_log(SOAPY_SDR_INFO, "SoapyRadioberry::getGainRange called");
	
	if(direction==SOAPY_SDR_RX)
		return(SoapySDR::Range(0, 60));
	return(SoapySDR::Range(0,18));

}

void SoapyRadioberry::setGain( const int direction, const size_t channel, const double value ) {
	
	SoapySDR_log(SOAPY_SDR_INFO, "SoapyRadioberry::setGain called");
	
	uint32_t command = 0;
	uint32_t command_data = 0x40100000 + (0x40 | (((uint32_t) (value + 12.0))  & 0x3F));
	
	if(direction==SOAPY_SDR_RX)	 command = 0x14; 
	if(direction==SOAPY_SDR_TX) { command = 2; }
	
	this->SoapyRadioberry::controlRadioberry(command, command_data);
}

/*******************************************************************
 * Frequency API 
 ******************************************************************/
void SoapyRadioberry::setFrequency( const int direction, const size_t channel,  const double frequency, const SoapySDR::Kwargs &args ) {
	
	SoapySDR_log(SOAPY_SDR_INFO, "SoapyRadioberry::setFrequency called");
	
	uint32_t command = 0;
	
	if(direction==SOAPY_SDR_RX)	command = 4;
	if(direction==SOAPY_SDR_TX)	command = 2;
	
	uint32_t command_data = (uint32_t) frequency;
	
	this->SoapyRadioberry::controlRadioberry(command, command_data);
}

// end of source.