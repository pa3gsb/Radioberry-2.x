#include <SoapySDR/Device.hpp>
#include <SoapySDR/Registry.hpp>

#include "SoapyRadioberry.hpp"

/***********************************************************************
 * Find available devices
 **********************************************************************/

 
SoapySDR::KwargsList findMyRadioberry(const SoapySDR::Kwargs &args)
{
	SoapySDR::Kwargs options;
	
	static std::vector<SoapySDR::Kwargs> results;
	
	options["driver"] = "radioberry";
	results.push_back(options);
	
	return results;
}

/***********************************************************************
 * Make device instance
 **********************************************************************/
SoapySDR::Device *makeMyRadioberry(const SoapySDR::Kwargs &args)
{
    return new SoapyRadioberry(args);
}

/***********************************************************************
 * Registration
 **********************************************************************/
static SoapySDR::Registry registerRadioberry("radioberry", &findMyRadioberry, &makeMyRadioberry, SOAPY_SDR_ABI_VERSION);
